/******************************************************************************
 * File:             irv_node.hpp
 *
 * Author:           Floyd Everest <me@floydeverest.com>
 * Created:          02/22/22
 * Description:      This file declares an IRV TreeNode representing the
 *                   internal states of a stochastic process which produces
 *                   valid, partially specified IRV ballots with some minimum
 *                   number of candidates selected.
 *****************************************************************************/
#ifndef IRV_NODE_H
#define IRV_NODE_H

#include "distributions.h"
#include "irv_ballot.h"
#include "tree_node.h"

#include <list>
#include <random>
#include <vector>

class IRVParameters : Parameters {
private:
  // The number of candidates participating in the IRV election.
  unsigned nCandidates;
  // The minimum number of ballots that must be specified for an IRV election.
  unsigned minDepth = 0;
  // The maximum number of ballots that must be specified for an IRV election.
  unsigned maxDepth = 0;
  // The prior parameter for a uniform Dirichlet-tree.
  float a0 = 1.;
  // A flag indicating whether or not the parameter structure reduces to a
  // vanilla Dirichlet distribution.
  bool vd = false;
  // For storing factor calculations for each depth level in the tree.
  std::vector<float> depthFactors = std::vector<float>(0);

public:
  // Canonical constructor
  IRVParameters(unsigned nCandidates_, unsigned minDepth_ = 0,
                unsigned maxDepth_ = 0, float a0_ = 1., bool vd_ = false)
      : nCandidates(nCandidates_), minDepth(minDepth_), maxDepth(maxDepth_),
        a0(a0_), vd(vd_) {
    calculateDepthFactors();
  }

  // Copy constructor is removed.
  IRVParameters(const IRVParameters &) = delete;

  // Copy assignment via `=` operator is removed.
  IRVParameters &operator=(const IRVParameters &) = delete;

  /*! \brief Returns the factor with which to multiply a0 for the prior to
   * reduce to a vanilla Dirichlet distribution.
   *
   * \param depth The depth in the tree.
   *
   * \return The factor with which to multiply a0 by for a Dirichlet
   * distribution.
   */
  float depthFactor(unsigned depth) { return depthFactors[depth]; };

  /*! \brief Calculates the factors with which to multiple a0 at each depth.
   *
   *  For a tree prior to reduce to a vanilla Dirichlet distribution, the
   * interior parameters at each node must add to the sum of the parameters at
   * its children. Hence, when we update minDepth we need to recalculate this
   * value, as the number of children at each node are prone to change.
   */
  void calculateDepthFactors();

  // Getters

  /*! \brief Returns the default path for traversing an IRV tree.
   *
   * With 5 candidates, this will be {1, 2, 3, 4, 5}.
   *
   * \return A vector representing the default path.
   */
  std::vector<unsigned> defaultPath() {
    std::vector<unsigned> out{};
    for (unsigned i = 0; i < nCandidates; ++i)
      out.emplace_back(i);
    return out;
  };

  /*! \brief Gets the number of participating candidates.
   *
   * \return Returns the number of candidates participating in the IRV election.
   */
  unsigned getNCandidates() { return nCandidates; }

  /*! \brief Gets the minimum depth.
   *
   * \return Returns the minimum number of candidates which must be specified
   * for a valid IRV Ballot.
   */
  unsigned getMinDepth() { return minDepth; }

  /*! \brief Gets the maximum depth.
   *
   * \return Returns the maximum number of candidates which can be specified
   * for a valid IRV Ballot.
   */
  unsigned getMaxDepth() { return maxDepth; }

  /*! \brief Gets the prior uniform-Dirichlet-tree parameter a0.
   *
   * \return a0, the prior parameter of the uniform Dirichlet-tree.
   */
  float getA0() { return a0; }

  /*! \brief Indicates whether the tree reduces to a Dirichlet distribution.
   *
   * \return vd, true if the tree reduces to a vanilla Dirichlet distribution.
   */
  float getVD() { return vd; }

  // Setters
  /*! \brief Sets the minimum depth for the election.
   *
   * \param minDepth_ The new minimum number of candidates to be specified for a
   * valid IRV ballot.
   */
  void setMinDepth(unsigned minDepth_) { minDepth = minDepth_; }

  /*! \brief Sets the maximum depth for the election.
   *
   * \param minDepth_ The new maximum number of candidates to be specified for a
   * valid IRV ballot.
   */
  void setMaxDepth(unsigned maxDepth_) {
    maxDepth = maxDepth_;
    calculateDepthFactors();
  }

  /*! \brief Sets the uniform Dirichlet-tree prior parameter a0.
   *
   * \param a0_ The new prior parameter for the uniform Dirichlet-tree.
   */
  void setA0(float a0_) { a0 = a0_; }

  /*! \brief Change the parameter structure of the prior.
   *
   *  Changes the prior either to a uniform Dirichlet-tree with a0 on
   *  every branch, or scale the parameters such that it is reducible to a
   *  vanilla Dirichlet distribution.
   *
   * \param a0_ The new prior parameter for the uniform Dirichlet-tree.
   */
  void setVD(bool vd_) { vd = vd_; };
};

/*! \brief Simulate random ballots from a uniform Dirichlet-tree starting from
 * an incomplete ballot.
 *
 *  Simulates random ballots, starting from an internal state in the IRV
 * stochastic process.
 *
 * \param params The IRVParameters for the election.
 *
 * \param count The number of ballots to sample.
 *
 * \param path The path to the internal node representing the incomplete
 * ballot.
 *
 * \param depth The current depth in the Dirichlet-tree.
 *
 * \param engine A PRNG for sampling.
 *
 * \return A list of valid IRV ballots from the sub-tree uniquely specified by
 * the arguments.
 */
std::list<IRVBallotCount> lazyIRVBallots(IRVParameters params, unsigned count,
                                         std::vector<unsigned> path,
                                         unsigned depth, std::mt19937 *engine);

class IRVNode : public TreeNode<IRVBallot, IRVNode, IRVParameters> {
public:
  using NodeP = IRVNode *;

  /*! \brief Constructs a new IRVNode.
   *
   *  Constructs an IRVNode representing an internal state of the
   * stochastic process which yields valid IRV ballots by selecting candidates
   * one-by-one.
   *
   * \param nChildren_ The number of possible child states (not including the
   * terminal state). In IRV, this is the number of remaining candidates to
   * choose from.
   *
   * \param depth_ The depth of this node in the tree.
   *
   * \param parameters A pointer to the object containing the IRV
   * distribution parameters.
   *
   * \return Returns a new IRV node.
   */
  IRVNode(unsigned depth_, IRVParameters *parameters_);

  /*! \brief Destroys the node and its' sub-tree.
   */
  ~IRVNode();

  /*! \brief Samples valid ballots from the sub-tree.
   *
   *  An IRVNode represents an incompleted ballot. This method provides an
   * interface for sampling completed ballots from the starting point
   * represented by this node.
   *
   * \param count The number of ballots to sample.
   *
   * \param path The path to this node, represented by a permutation on the
   * candidates.
   *
   * \param engine A PRNG for random sampling.
   *
   * \return A list of (ballot, count) pairs sampled from the subtree.
   */
  std::list<IRVBallotCount> sample(unsigned count, std::vector<unsigned> path,
                                   std::mt19937 *engine);

  /*! \brief Updates the parameters in the sub-tree to obtain a posterior.
   *
   *  Given the path to a valid IRV ballot starting from this node, this method
   * updates the parameters along the path to obtain the posterior distribution
   * having observed this ballot.
   *
   * \param b The ballot to observe.
   *
   * \param path The path to this node.
   *
   * \param count The number of times to observe the ballot.
   */
  void update(const IRVBallot &b, std::vector<unsigned> path,
              unsigned count = 1);
};

#endif /* IRV_NODE_H */

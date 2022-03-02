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

#include "distributions.hpp"
#include "irv_ballot.hpp"
#include "tree_node.hpp"

#include <list>
#include <random>
#include <vector>

class IRVParameters : Parameters {
private:
  // The number of candidates participating in the IRV election.
  int nCandidates;
  // The minimum number of ballots that must be specified for an IRV election.
  int minDepth;
  // The prior parameter for a uniform Dirichlet Tree.
  float alpha0;

public:
  // Canonical constructor
  IRVParameters(int nCandidates_, int minDepth_, float alpha0_ = 1.)
      : nCandidates(nCandidates_), minDepth(minDepth_), alpha0(alpha0_) {}

  // Getters

  /*! \brief Returns the default path for traversing an IRV tree.
   *
   * With 5 candidates, this will be {1, 2, 3, 4, 5}.
   *
   * \return A vector representing the default path.
   */
  std::vector<int> defaultPath() {
    std::vector<int> out(nCandidates);
    for (auto i = 0; i < nCandidates; ++i)
      out[i] = i + 1;
    return out;
  };

  /*! \brief Gets the number of participating candidates.
   *
   * \return Returns the number of candidates participating in the IRV election.
   */
  int getNCandidates() { return nCandidates; }

  /*! \brief Gets the minimum depth.
   *
   * \return Returns the minimum number of candidates which must be specified
   * for a valid IRV Ballot.
   */
  int getMinDepth() { return minDepth; }

  /*! \brief Gets the prior uniform-Dirichlet-Tree parameter alpha0.
   *
   * \return alpha0, the prior parameter of the uniform Dirichlet Tree.
   */
  float getAlpha0() { return alpha0; }

  // Setters
  /*! \brief Sets the minimum depth for the election.
   *
   *  Detailed description of the function
   *
   * \param minDepth_ The new minimum number of candidates to be specified for a
   * valid IRV ballot.
   */
  void setMinDepth(int minDepth_) { minDepth = minDepth_; }

  /*! \brief Sets the uniform Dirichlet Tree prior parameter alpha0.
   *
   * \param alpha0_ The new prior parameter for the uniform Dirichlet Tree.
   */
  void setAlpha0(float alpha0_) { alpha0 = alpha0_; }
};

/*! \brief Simulate random ballots from a uniform Dirichlet Tree starting from
 * an incomplete ballot.
 *
 *  Simulates random ballots, starting from an internal state in the IRV
 * stochastic process.
 *
 * \param params The IRVParameters for the election.
 *
 * \param count The number of ballots to sample.
 *
 * \param path The path to the internal node representing the incomplete ballot.
 *
 * \param depth The current depth in the Dirichlet Tree.
 *
 * \param engine A PRNG for sampling.
 *
 * \return A list of valid IRV ballots from the sub-tree uniquely specified by
 * the arguments.
 */
std::list<IRVBallot> lazyIRVBallots(IRVParameters params, int count,
                                    std::vector<int> path, int depth,
                                    std::mt19937 *engine);

class IRVNode : public TreeNode<IRVBallot, IRVNode, IRVParameters> {
public:
  using NodeP = IRVNode *;

  /*! \brief Constructs a new IRVNode.
   *
   *  Constructs an IRVNode representing an internal state of the stochastic
   * process which yields valid IRV ballots by selecting candidates one-by-one.
   *
   * \param nChildren_ The number of possible child states (not including the
   * terminal state). In IRV, this is the number of remaining candidates to
   * choose from.
   *
   * \param depth_ The depth of this node in the tree.
   *
   * \param parameters A pointer to the object containing the IRV distribution
   * parameters.
   *
   * \return Returns a new IRV node.
   */
  IRVNode(int depth_, IRVParameters *parameters_);

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
   * \return A list of completed ballots sampled from the subtree.
   */
  std::list<IRVBallot> sample(int count, std::vector<int> path,
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
  void update(const IRVBallot &b, std::vector<int> path, int count = 1);

  /*! \brief Samples a probability of observing a ballot from the posterior.
   *
   *  Given the ballot below the current node in the tree, this method samples a
   * probability for observing this ballot given the current state, from the
   * posterior Dirichlet Tree.
   *
   * \param b See `IRVNode::update`.
   *
   * \param path The path to this node.
   *
   * \param engine A PRNG for sampling.
   *
   * \return Return parameter description
   */
  float marginalProbability(const IRVBallot &b, std::vector<int> path,
                            std::mt19937 *engine);
};

#endif /* IRV_NODE_H */

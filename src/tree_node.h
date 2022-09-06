/******************************************************************************
 * File:             tree_node.hpp
 *
 * Author:           Floyd Everest (me@floydeverest.com)
 * Created:          02/26/22
 * Description:      This file declares the abstract class `TreeNode` for use
 *                   in Dirichlet-trees. A node in a Dirichlet-tree represents
 *                   a non-terminal state in an incomplete stochastic process.
 *                   For example, a node might represent an IRV ballot that is
 *                   not yet completely specified.
 *
 *                   A TreeNode should provide an interface to its' parent
 *                   (or the `DirichletTree` class if it is the root) which
 *                   allows for sampling count data from the sub-tree, updating
 *                   interior Dirichlet parameters, and sampling leaf-
 *                   probabilities of specific outcomes.
 *****************************************************************************/
#ifndef NODE_H
#define NODE_H

#include <list>
#include <random>

class Parameters {
public:
  /*! \brief Returns the default path for traversing a tree described by these
   * parameters.
   *
   * \return A vector representing the default path.
   */
  std::vector<unsigned> defaultPath();
};

template <typename Outcome, typename ChildNode, class Parameters>
class TreeNode {
protected:
  // The distribution parameters for the tree.
  Parameters *parameters;

  // The depth of the node in the tree.
  unsigned depth;

  // The number of child nodes below. For example, in IRV this can represent the
  // selection of a candidate for next preference. The leaves in a tree will
  // have 2 children, representing one of two remaining candidates. If
  // partial ballots are allowed, then the number of children is still the same
  // as a ballot termination does require a child node.
  unsigned nChildren;

  // The a parameters for the dirichlet distribution on the possible
  // next-preferences. Considering the case of IRV ballots allowing for partial
  // specification, then it has size nCandidates+1.
  float *as;

  // An array of ChildNode pointers corresponding to each of the child states.
  // These will be null pointers if the corresponding child has not yet been
  // initialized.
  ChildNode **children;

public:
  // Destructor.
  virtual ~TreeNode(){};

  /*! \brief Samples count data from the sub-tree.
   *
   *  A TreeNode represents a non-terminal state of a stochastic process.
   * This method provides an interface for sampling completed outcomes of of
   * the underlying stochastic process for which this node represents an
   * internal state.
   *
   * \param count The number of outcomes to sample starting from the current
   * node.
   *
   * \param path The path to this node. This can vary between different
   * types of TreeNodes. For example, if we consider an IRV tree with
   * complete ballots, a path could be a partial permutation which (at a
   * leaf) will realize a complete IRV ballot.
   *
   * \param engine A PRNG used for sampling.
   *
   * \return A list of (outcome, count) pairs corresponding to realizations of
   * the underlying stochastic process possible from the starting point that
   * this node represents.
   */
  virtual std::list<std::pair<Outcome, unsigned>>
  sample(unsigned count, std::vector<unsigned> path, std::mt19937 *engine) = 0;

  /*! \brief Updates sub-tree parameters to obtain a posterior.
   *
   *  Given an outcome of the underlying stochastic process, this
   * method updates the parameters along the path to the outome in order to
   * obtain the posterior Dirichlet-tree having observed the outcome.
   *
   * \param o The outcome to observe.
   *
   * \param path The path to the current node.
   *
   * \param count The number of times to observe o.
   */
  virtual void update(const Outcome &o, std::vector<unsigned> path,
                      unsigned count) = 0;
};

#endif /* NODE_H */

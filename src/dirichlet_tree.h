/******************************************************************************
 * File:             dirichlet-tree.hpp
 *
 * Author:           Floyd Everest <me@floydeverest.com>
 * Created:          02/22/22
 * Description:      This file implements the Dirichlet-tree distribution class.
 *                   This class provides an interface to the interior nodes of
 *                   the tree, and provides methods to sample ballots, update
 *                   the posterior distribution, and alter internal distribution
 *                   functionalities such as the choice of prior parameter or
 *                   style of the tree (Dirichlet vs Dirichlet-tree sampling).
 *****************************************************************************/

#ifndef DIRICHLET_TREE_H
#define DIRICHLET_TREE_H

#include "irv_ballot.h"
#include "tree_node.h"

#include <list>
#include <map>
#include <random>

template <typename NodeType, typename Outcome, class Parameters>
class DirichletTree {
private:
  // The interior root node for the Dirichlet-tree.
  NodeType *root;

  // The tree parameters. This object defines both the structure and sampling
  // parameters for the Dirichlet-tree. Some parameters will be immutable, for
  // example the tree structure cannot be changed dynamically while the prior
  // parameter scheme at each level might be possible to alter dynamically. The
  // parameters might also indicate some outcome filtering which can often be
  // changed dynamically.
  Parameters *parameters;

  // The number of outcomes observed to obtain the posterior.
  unsigned nObserved = 0;
  // A map of unique observations to the number of times it has been observed.
  std::map<Outcome, unsigned> observed{};

  // A default PRNG for sampling.
  std::mt19937 engine;

public:
  /*! \brief The DirichletTree constructor.
   *
   *  The constructor returns a new Dirichlet-tree according to the specified
   * characteristics. There is no copy constructor for DirichletTrees.
   *
   * \param parameters_ The Dirichlet-tree parameters object.
   *
   * \param seed A string representing the mt19937 initial seed.
   *
   * \return A DirichletTree object with the corresponding attributes.
   */
  DirichletTree(Parameters *parameters_, std::string seed = "12345");

  // No copy constructor
  DirichletTree(const DirichletTree &dirichletTree) = delete;

  ~DirichletTree();

  /*! \brief Resets the distribution to its' prior.
   *
   *  This function will clear the internal state of the distribution. All
   * observed ballots will be erased, along with all interior parameters and
   * nodes.
   *
   * \return void
   */
  void reset();

  /*! \brief Update a Dirichlet-tree with an observed outcome.
   *
   *  This function will update the internal parameters and nodes of the tree,
   * realising a new posterior distribution having observed the provided
   * outcome.
   *
   * \param oc A pair, the first element being the outcome of the stochastic
   * process, and the second being the count for that outcome.
   *
   * \return void
   */
  void update(const std::pair<Outcome, unsigned> &oc);

  /*! \brief Sample outcomes from the posterior predictive distribution.
   *
   *  Samples a specified number of outcomes from one realisation of the
   * Dirichlet-tree.
   *
   * \param n The number of outcomes to sample from a single realisation of the
   * Dirichlet-tree.
   *
   * \param engine An optional warmed-up mt19937 PRNG for randomness.
   *
   * \return A list of (outcome, count) pairs observed from the resulting
   * stochastic process.
   */
  std::list<std::pair<Outcome, unsigned>>
  sample(unsigned n, std::mt19937 *engine = nullptr);

  /*! \brief Sample possible full sets from the posterior.
   *
   *  Assuming we have been updating the Dirichlet-tree with observations
   * without replacement, this method samples possible complete outcome sets of
   * size N from the posterior. Each complete set contains the already observed
   * outcomes which determine the posterior Dirichlet-tree used in generating
   * the output. For example, if we observe a set of outcomes {o1, o2, o2}, then
   * `posteriorSets(nSets=2, N=4)` may return [{o1, o2, o2, o3},
   * {o1, o2, o2, o1}].
   *
   * \param nSets The number of complete observation sets to sample.
   *
   * \param N The number of observations in each complete set (must be >=
   * than the number of observed outcomes).
   *
   * \return Returns `nSets` complete outcome sets sampled from the posterior
   * Dirichlet-tree distribution, using the already observed data.
   */
  std::list<std::list<std::pair<Outcome, unsigned>>>
  posteriorSets(unsigned nSets, unsigned N, std::mt19937 *engine = nullptr);

  // Getters

  /*! \brief Get the PRNG engine.
   *
   *  Gets a pointer to the mt19937 PRNG.
   *
   * \return A pointer to the base mt19937 engine.
   */
  std::mt19937 *getEnginePtr() { return &engine; }

  /*! \brief Gets the tree parameters.
   *
   * \return Returns a pointer to the Dirichlet-tree parameters.
   */
  Parameters *getParameters() { return parameters; }

  // Setters

  /*! \brief Sets the seed of the internal mt19937 PRNG.
   *
   *  Resets the mt19937 seed and warms up the PRNG.
   *
   * \param seed A string representing the new seed for the PRNG engine.
   *
   * \return void
   */
  void setSeed(std::string seed) {
    std::seed_seq ss(seed.begin(), seed.end());
    engine.seed(ss);
    // warmup. TODO: treeGen->discard(treeGen->state_size * 100);
    for (unsigned i = 1000; i; --i)
      engine();
  }
};

template <typename NodeType, typename Outcome, typename Parameters>
DirichletTree<NodeType, Outcome, Parameters>::DirichletTree(
    Parameters *parameters_, std::string seed) {

  parameters = parameters_;

  // Initialize the root node of the tree.
  root = new NodeType(0, parameters);

  // Initialize a default PRNG, seed it and warm it up.
  std::mt19937 engine{};
  setSeed(seed);
}

template <typename NodeType, typename Outcome, typename Parameters>
void DirichletTree<NodeType, Outcome, Parameters>::reset() {
  // Replace the root node, calling the destructor of the old root after call.
  delete root;
  root = new NodeType(0, parameters);
  // Destroy the observations list
  observed.clear();
  nObserved = 0;
}

template <typename NodeType, typename Outcome, typename Parameters>
void DirichletTree<NodeType, Outcome, Parameters>::update(
    const std::pair<Outcome, unsigned> &oc) {
  if (observed.count(oc.first) == 0) {
    observed[oc.first] = oc.second;
  } else {
    observed[oc.first] = observed[oc.first] + oc.second;
  }
  nObserved += oc.second;
  std::vector<unsigned> path = parameters->defaultPath();
  root->update(oc.first, path, oc.second);
}

template <typename NodeType, typename Outcome, typename Parameters>
std::list<std::pair<Outcome, unsigned>>
DirichletTree<NodeType, Outcome, Parameters>::sample(unsigned n,
                                                     std::mt19937 *engine_) {
  // Use the default engine unless one is passed to the method.
  if (engine_ == nullptr) {
    engine_ = &engine;
  }

  // Initialize output
  std::vector<unsigned> path = parameters->defaultPath();
  std::list<std::pair<Outcome, unsigned>> out = root->sample(n, path, engine_);

  return out;
}

template <typename NodeType, typename Outcome, typename Parameters>
DirichletTree<NodeType, Outcome, Parameters>::~DirichletTree() {
  delete root;
}

template <typename NodeType, typename Outcome, typename Parameters>
std::list<std::list<std::pair<Outcome, unsigned>>>
DirichletTree<NodeType, Outcome, Parameters>::posteriorSets(
    unsigned nSets, unsigned N, std::mt19937 *engine) {

  // Initialize list of outcomes.
  std::list<std::list<std::pair<Outcome, unsigned>>> out{};
  std::list<std::pair<Outcome, unsigned>> old_outcomes, new_outcomes;

  // Handle invalid case by returning empty list.
  if (nObserved > N || nSets <= 0)
    return {};

  for (unsigned i = 0; i < nSets; ++i) {
    // Add a new list to the list, first by copying the observed outcomes.
    out.push_back({});

    // Copy the observed outcomes
    old_outcomes = std::list<std::pair<Outcome, unsigned>>(observed.begin(),
                                                           observed.end());

    // Then sample new outcomes.
    new_outcomes = sample(N - nObserved, engine);

    // Combine the two, by appending to the new list.
    out.back().splice(out.back().end(), old_outcomes);
    out.back().splice(out.back().end(), new_outcomes);
  }

  return out;
}

#endif /* DIRICHLET_TREE_H */

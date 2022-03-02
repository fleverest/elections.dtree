/******************************************************************************
 * File:             dirichlet_tree.cpp
 *
 * Author:           Floyd Everest <me@floydeverest.com>
 * Created:          02/27/22
 * Description:      This file implements the Dirichlet Tree class as outlined
 *                   in `dirichlet_tree.hpp`.
 *****************************************************************************/

#include "dirichlet_tree.hpp"
#include <random>

template <typename NodeType, typename Outcome, typename Parameters>
DirichletTree<NodeType, Outcome, Parameters>::DirichletTree(
    Parameters parameters_, std::string seed) {
  // Set the parameters.
  parameters = parameters_;

  // Initialize the root node of the tree.
  root = TreeNode<Outcome, NodeType, Parameters>(0, &parameters);

  // Initialize a default PRNG, seed it and warm it up.
  engine = std::mt19937();
  setSeed(seed);
}

template <typename NodeType, typename Outcome, typename Parameters>
void DirichletTree<NodeType, Outcome, Parameters>::reset() {
  // Replace the root node, calling the destructor of the old root after call.
  root = TreeNode<Outcome, NodeType, Parameters>(0, &parameters);
}

template <typename NodeType, typename Outcome, typename Parameters>
void DirichletTree<NodeType, Outcome, Parameters>::update(Outcome o,
                                                          int count) {
  observed.push_back(o);
  std::vector<int> path(o.nPreferences);
  for (auto i = 1; i <= o.nPreferences; ++i) {
    path[i] = i;
  }
  root.update(o, path, count);
}

template <typename NodeType, typename Outcome, typename Parameters>
float DirichletTree<NodeType, Outcome, Parameters>::marginalProbability(
    Outcome o, std::mt19937 *engine) {

  // Use the default engine unless one is passed to the method.
  if (engine == nullptr) {
    engine = this->engine;
  }

  // Pass straight to the root node.
  return root.marginalProbability(o, engine);
}

template <typename NodeType, typename Outcome, typename Parameters>
std::vector<Outcome>
DirichletTree<NodeType, Outcome, Parameters>::sample(int n,
                                                     std::mt19937 *engine) {
  // Use the default engine unless one is passed to the method.
  if (engine == nullptr) {
    engine = this->engine;
  }

  // Initialize output
  std::vector<Outcome> out{};
  std::vector<int> path = parameters.defaultPath();
  std::list<Outcome> sample = root.sample(n, path, engine);

  // Vectorize the list of outputs and return.
  for (auto o : sample) {
    out.push_back(0);
  }

  return out;
}

template <typename NodeType, typename Outcome, typename Parameters>
std::list<std::list<Outcome>>
DirichletTree<NodeType, Outcome, Parameters>::posteriorSets(
    int nSets, int N, std::mt19937 *engine) {
  // Initialize list of outcomes.
  std::list<std::vector<Outcome>> out(nSets, {});
  std::list<Outcome> old_outcomes, new_outcomes;

  // The number of observed outcomes.
  int n = observed.size();

  for (auto i = 0; i < N; ++i) {
    // Add a new set to the list, first by copying the observed outcomes.
    old_outcomes = observed;

    // Then sample new outcomes.
    new_outcomes = sample(N - n, engine);

    // Combine the two, by appending to the output.
    out[i].splice(out[i].end(), old_outcomes);
    out[i].splice(out[i].end(), new_outcomes);
  }

  return out;
}

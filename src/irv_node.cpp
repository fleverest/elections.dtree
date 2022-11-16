/******************************************************************************
 * File:             irv_node.cpp
 *
 * Author:           Floyd Everest <me@floydeverest.com>
 * Created:          02/26/22
 * Description:      This file implements the IRVNode class as outlined in
 *                   `irv_node.hpp`.
 *****************************************************************************/
#include "irv_node.h"

// Calculates the factors with which to multiply a0 in order to obtain the
// interior parameters which reduce to a Dirichlet distribution.
void IRVParameters::calculateDepthFactors() {
  depthFactors = std::vector<double>(maxDepth);
  // The number of children to a node for a given depth in the tree.
  unsigned nChildren;
  // For each depth, maxDepth-1 through 0, we calculate the factors.
  double f = 1.;
  for (int depth = maxDepth - 1; depth >= 0; --depth) {
    nChildren = nCandidates - depth;
    if (depth >= minDepth) ++nChildren;
    depthFactors[depth] = f;
    f = f * nChildren;
  }
}

std::list<IRVBallotCount> lazyIRVBallots(IRVParameters *params, unsigned count,
                                         std::vector<unsigned> path,
                                         unsigned depth, std::mt19937 *engine) {
  // Get parameters
  unsigned nCandidates = params->getNCandidates();
  double minDepth = params->getMinDepth();
  double maxDepth = params->getMaxDepth();
  double a0 = params->getA0();
  if (params->getVD()) a0 = a0 * params->depthFactor(depth);

  std::list<IRVBallotCount> out = {};

  std::vector<unsigned> mnomCounts;

  unsigned nChildren = nCandidates - depth;
  unsigned nOutcomes = nChildren + (depth >= minDepth);

  std::vector<double> a(nOutcomes);

  if (depth == nCandidates - 1 || depth == maxDepth) {
    // If the ballot is completely specified, return count * the specified
    // ballot.
    IRVBallot b(std::list<unsigned>(path.begin(), path.begin() + depth));
    out.emplace_back(std::move(b), count);
    return out;
  }

  // Otherwise we sample from a Dirichlet-Multinomial distribution to
  // determine how many ballots we sample from each sub-tree (or how many
  // ballots terminate).

  // We start by initializing a to the appropriate values.
  for (unsigned i = 0; i < nOutcomes; ++i) a[i] = a0;
  mnomCounts = rDirichletMultinomial(count, a, engine);

  // Add the ballots which terminate at this node.
  if (depth >= minDepth && mnomCounts[nOutcomes - 1] > 0) {
    // Create the ballot.
    IRVBallot b(std::list<unsigned>(path.begin(), path.begin() + depth));
    // Add ballots to output.
    out.emplace_back(std::move(b), mnomCounts[nOutcomes - 1]);
  }

  for (unsigned i = 0; i < nChildren; ++i) {
    // Skip if there the sampled count for the subtree is zero.
    if (mnomCounts[i] == 0) continue;

    // Update path for recursive sampling.
    std::swap(path[depth], path[depth + i]);
    // Combine results with output.
    out.splice(out.end(),
               lazyIRVBallots(params, mnomCounts[i], path, depth + 1, engine));
    // Change the path back for further sampling.
    std::swap(path[depth], path[depth + i]);
  }

  return out;
}

IRVNode::IRVNode(unsigned depth_, IRVParameters *parameters_) {
  parameters = parameters_;
  nChildren = parameters->getNCandidates() - depth_;
  depth = depth_;

  as = new double[nChildren + 1];  // +1 for incomplete ballots
  for (unsigned i = 0; i < nChildren + 1; ++i) as[i] = 0.;

  children = new NodeP[nChildren]{nullptr};
}

IRVNode::~IRVNode() {
  // Destructor must delete the entire sub-tree. Hence, we need to delete any
  // initialized nodes in the sub-tree before removing the array.
  delete[] as;
  for (unsigned i = 0; i < nChildren; ++i) {
    if (children[i] != nullptr) delete children[i];
  }
  delete[] children;
}

std::list<IRVBallotCount> IRVNode::sample(unsigned count,
                                          std::vector<unsigned> path,
                                          std::mt19937 *engine) {
  std::list<IRVBallotCount> out = {};

  unsigned minDepth = parameters->getMinDepth();
  unsigned maxDepth = parameters->getMaxDepth();
  double a0 = parameters->getA0();
  if (parameters->getVD()) a0 = a0 * parameters->depthFactor(depth);

  unsigned nOutcomes = nChildren + (depth >= minDepth);

  std::vector<double> asPost(nOutcomes);
  for (unsigned i = 0; i < nOutcomes; ++i) asPost[i] = as[i] + a0;

  // Get Dirichlet-multinomial counts for next-preference selections below
  // current node.
  std::vector<unsigned> mnomCounts =
      rDirichletMultinomial(count, asPost, engine);

  // Add terminal node ballots
  if (depth >= minDepth && mnomCounts[nChildren] > 0) {
    IRVBallot b(std::list<unsigned>(path.begin(), path.begin() + depth));

    out.emplace_back(std::move(b), mnomCounts[nChildren]);
  }

  // If the ballot is one preference from being completely specified, add the
  // completed ballots to the output.
  if (depth == maxDepth - 1) {
    for (unsigned i = 0; i < nChildren; ++i) {
      // Skip if there the sampled count for the ballot is zero.
      if (mnomCounts[i] == 0) continue;

      std::swap(path[depth], path[depth + i]);

      IRVBallot b(std::list<unsigned>(path.begin(), path.begin() + depth + 1));

      out.emplace_back(std::move(b), mnomCounts[i]);

      std::swap(path[depth], path[depth + i]);
    }
    // Return early since there are no child nodes to sample from.
    return out;
  }

  // Otherwise we continue recursively sampling from subtrees. If a subtree is
  // not specified, then we lazily generate samples from a uniform dirichlet
  // tree.
  for (unsigned i = 0; i < nChildren; ++i) {
    // Skip if there the sampled count for the subtree is zero.
    if (mnomCounts[i] == 0) continue;

    // Sample from the next subtree.
    std::swap(path[depth], path[depth + i]);

    // Add the samples to the output.
    if (children[i] == nullptr) {
      out.splice(out.end(), lazyIRVBallots(parameters, mnomCounts[i], path,
                                           depth + 1, engine));
    } else {
      out.splice(out.end(), children[i]->sample(mnomCounts[i], path, engine));
    }
    std::swap(path[depth], path[depth + i]);
  }

  return out;
}

void IRVNode::update(const IRVBallot &b, std::vector<unsigned> path,
                     unsigned count) {
  /* We traverse the tree such that at each step, b.preferences and
   * path vectors are exactly equal up to the next index.
   *
   * For example, at depth 0, if b.preferences is {4, 2, 1} and path is {0, 1,
   * 2, 3, 4}, then we swap indices d=0 and i=4 to obtain the next path of {4,
   * 1, 2, 3, 0} and then proceed to children[i-d]. Then, at depth 1, we will
   * swap indices d=1 and i=2 to obtain {4, 2, 1, 3, 0} and proceed to
   * children[i-d]. Hence we match the two vectors exactly on the specified
   * preferences, and so we stop traversing.
   */

  // If the next preference is not defined, then we increment the halting
  // parameter and stop traversing.
  if (depth == b.nPreferences()) {
    as[nChildren] += count;
    return;
  }

  // Determine the next candidate preference.
  auto it = b.preferences.begin();
  for (unsigned i = 0; i < depth; ++i) ++it;
  unsigned nextCandidate = *it;

  // Find the index of the next candidate, and increment the corresponding
  // parameter.
  unsigned i = depth;
  while (path[i] != nextCandidate) ++i;
  unsigned next_idx = i - depth;
  as[next_idx] += count;

  // Stop traversing if the number of children is 2, since we don't need to
  // access the leaves.
  if (nChildren == 2) return;

  // If the next node is uninitialized, we create a new one with one less
  // candidate to choose from.
  if (children[next_idx] == nullptr)
    children[next_idx] = new IRVNode(depth + 1, parameters);

  // Recursively update the following children down the path, updating the
  // path as we go.
  std::swap(path[depth], path[i]);
  children[next_idx]->update(b, path);
}

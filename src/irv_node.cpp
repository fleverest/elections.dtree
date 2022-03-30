/******************************************************************************
 * File:             irv_node.cpp
 *
 * Author:           Floyd Everest <me@floydeverest.com>
 * Created:          02/26/22
 * Description:      This file implements the IRVNode class as outlined in
 *                   `irv_node.hpp`.
 *****************************************************************************/
#include "irv_node.hpp"

std::list<IRVBallot> lazyIRVBallots(IRVParameters *params, int count,
                                    std::vector<int> path, int depth,
                                    std::mt19937 *engine) {
  // Get parameters
  int nCandidates = params->getNCandidates();
  float alpha0 = params->getAlpha0();
  float minDepth = params->getMinDepth();

  std::list<IRVBallot> temp = {};
  std::list<IRVBallot> out = {};

  float *alpha;
  int *mnomCounts;

  int nChildren = nCandidates - depth;
  int nOutcomes = nChildren + (depth >= minDepth);

  if (depth == nCandidates - 1) {
    // If the ballot is completely specified, return count * the specified
    // ballot.
    IRVBallot b(std::vector<int>(path.begin(), path.begin() + depth));
    for (auto i = 0; i < count; ++i)
      out.push_back(b);
    return out;
  }

  // Otherwise we sample from a Dirichlet-Multinomial distribution to
  // determine how many ballots we sample from each sub-tree (or how many
  // ballots terminate).

  // We start by initializing alpha to the appropriate values.
  alpha = new float[nOutcomes]{alpha0};
  mnomCounts = rDirichletMultinomial(count, alpha, nOutcomes, engine);

  // Add the ballots which terminate at this node.
  if (depth >= minDepth && mnomCounts[nOutcomes - 1] > 0) {
    // Create the ballot.
    IRVBallot b(std::vector<int>(path.begin(), path.begin() + depth));
    // Add ballots to output.
    for (auto i = 0; i < mnomCounts[nOutcomes - 1]; ++i)
      out.push_back(b);
  }

  for (auto i = 0; i < nChildren; ++i) {
    // Skip if there the sampled count for the subtree is zero.
    if (mnomCounts[i] == 0)
      continue;

    // Update path for recursive sampling.
    std::swap(path[depth], path[depth + i]);
    temp = lazyIRVBallots(params, mnomCounts[i], path, depth + 1, engine);
    // Combine temp with output.
    out.splice(out.end(), temp);
    // Change the path back for further sampling.
    std::swap(path[depth], path[depth + i]);
  }

  delete[] mnomCounts;
  delete[] alpha;

  return out;
}

IRVNode::IRVNode(int depth_, IRVParameters *parameters_) {
  parameters = parameters_;
  nChildren = parameters->getNCandidates() - depth_;
  depth = depth_;
  alphas = new float[nChildren + 1]{0.}; // +1 for incomplete ballots
  children = new NodeP[nChildren]{nullptr};
}

IRVNode::~IRVNode() {
  // Destructor must delete the entire sub-tree. Hence, we need to delete any
  // initialized nodes in the sub-tree before removing the array.
  delete[] alphas;
  for (auto i = 0; i < nChildren; ++i) {
    if (children[i])
      delete children[i];
  }
  delete[] children;
}

std::list<IRVBallot> IRVNode::sample(int count, std::vector<int> path,
                                     std::mt19937 *engine) {
  std::list<IRVBallot> temp = {};
  std::list<IRVBallot> out = {};

  int minDepth = parameters->getMinDepth();
  float alpha0 = parameters->getAlpha0();

  int nOutcomes = nChildren + (depth >= minDepth);

  float *alphasPost = new float[nOutcomes];
  for (auto i = 0; i < nOutcomes; ++i) {
    alphasPost[i] = alphas[i] + alpha0;
  }
  // Get Dirichlet-multinomial counts for next-preference selections below
  // current node.
  int *mnomCounts = rDirichletMultinomial(count, alphasPost, nOutcomes, engine);
  delete[] alphasPost;

  // Add any terminated ballots to the output.
  if (depth >= minDepth && mnomCounts[nChildren] > 0) {

    IRVBallot b(std::vector<int>(path.begin(), path.begin() + depth));

    for (auto i = 0; i < mnomCounts[nChildren]; ++i)
      out.push_back(b);
  }

  // If nChildren is 2, stop recursing and add the completely specified ballots
  // to the output.
  if (nChildren == 2) {
    for (auto i = 0; i < nChildren; ++i) {
      // Skip if there the sampled count for the ballot is zero.
      if (mnomCounts[i] == 0)
        continue;

      std::swap(path[depth], path[depth + i]);

      IRVBallot b(std::vector<int>(path.begin(), path.begin() + depth));

      for (auto j = 0; j < mnomCounts[j]; ++j)
        out.push_back(b);

      std::swap(path[depth], path[depth + i]);
    }
    // Return early since there are no child nodes to sample from.
    delete[] mnomCounts;
    return out;
  }

  // Otherwise we continue recursively sampling from subtrees. If a subtree is
  // not specified, then we lazily generate samples from a uniform dirichlet
  // tree.
  for (auto i = 0; i < nChildren; ++i) {

    // Skip if there the sampled count for the subtree is zero.
    if (mnomCounts[i] == 0)
      continue;

    // Sample from the next subtree.
    std::swap(path[depth], path[depth + i]);
    if (children[i] == nullptr) {
      temp = lazyIRVBallots(parameters, count, path, depth + 1, engine);
    } else {
      temp = children[i]->sample(count, path, engine);
    }
    std::swap(path[depth], path[depth + i]);
    // Add the samples to the output.
    out.splice(out.end(), temp);
  }

  delete[] mnomCounts;

  return out;
}

void IRVNode::update(const IRVBallot &b, std::vector<int> path, int count) {
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
    alphas[nChildren] += count;
    return;
  }

  // Determine the next candidate preference.
  int nextCandidate = b.preferences[depth];

  // Find the index of the next candidate, and increment the corresponding
  // parameter.
  int i = 0;
  while (b.preferences[i + depth] != nextCandidate)
    ++i;
  int next_idx = i;
  alphas[next_idx] += count;

  // Stop traversing if the number of children is 2, since we don't need to
  // access the leaves.
  if (nChildren == 2)
    return;

  // If the next node is uninitialized, we create a new one with one less
  // candidate to choose from.
  if (children[next_idx] == nullptr)
    children[next_idx] = new IRVNode(depth + 1, parameters);

  // Recursively update the following children down the path, updating the
  // path as we go.
  std::swap(path[depth], path[i]);
  children[next_idx]->update(b, path);
}

float IRVNode::marginalProbability(const IRVBallot &b, std::vector<int> path,
                                   std::mt19937 *engine) {
  float alpha0 = parameters->getAlpha0();
  int minDepth = parameters->getMinDepth();
  int nOutcomes = nChildren + (depth >= minDepth);
  int nCandidates = parameters->getNCandidates();
  float a_beta, b_beta, branchProb;

  // See update method for traversal.
  int nextCandidate = b.preferences[depth];

  // Otherwise determine the next candidate index.
  int i = depth;
  while (i < nCandidates && path[i] != nextCandidate)
    ++i;
  int next_idx = i - depth;

  // Evaluate the alpha and beta parameters, and sample the next
  // marginal branch probability.
  a_beta = alphas[next_idx] + alpha0;
  b_beta = 0.;
  for (auto j = 0; j < nOutcomes; ++j)
    if (j != next_idx)
      b_beta += alphas[j] + alpha0;
  branchProb = rBeta(a_beta, b_beta, engine);

  // Stop if the number of children is 2 or if no further preferences were
  // specified.
  if (nChildren == 2 || next_idx == nChildren)
    return branchProb;

  // If the next node is uninitialized, calculate beta parameters lazily for
  // sampling.
  if (children[next_idx] == nullptr) {
    // For each remaining preference, since we have not initialized the
    // corresponding nodes we know that alpha'=(alpha0,...,alpha0). Hence,
    // the following branch probabilities will necessarily be distributed as
    // Beta(alpha0, alpha0*(nCandidates-i')), where i ranges from depth+1 to
    // b.nPreferences(), and i'= i - 1(i>=minDepth).
    for (auto i = depth + 1; i < b.nPreferences(); ++i)
      branchProb *=
          rBeta(alpha0, alpha0 * (nCandidates - i + (i >= minDepth)), engine);
    return branchProb;
  }
  // Otherwise continue recursively evaluating branch probabilities.
  std::swap(path[depth], path[i]);
  return branchProb * children[next_idx]->marginalProbability(b, path, engine);
}

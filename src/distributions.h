#ifndef DISTRIBUTIONS_H
#define DISTRIBUTIONS_H

#include "ballot.h"
#include <algorithm>
#include <random>

/* Draws random Dirichlet Multinomial samples as specified by alpha.
 *
 * Returns an array of dirichlet multinomial samples, where each of the
 * samples is an array of category counts.
 */
int **rDirichletMultinomial(int n,        // Number of repetitions.
                            int *draws,   // number of draws per sample.
                            float *alpha, // alpha parameter.
                            int d,        // dimension of alpha parameter.
                            std::mt19937 *engine // RNG engine.
) {

  int **out = new int *[n]; // output array:
                            // categorical counts for each repetition.
  float **gammas =
      new float *[d]; // Array for the gamma(alphaj,1) samples for each
                      // repetition. Note the dimension-first index.
  float *gamma_sums =
      new float[n]{0.}; // Array of sum of gamma variates for each repetition.
  float sum_ps, p, a, gam;
  int remaining, count;

  // Sample gamma variates for each alpha. Additionally we calculate a sum of
  // the alpha parameters here.
  for (int i = 0; i < d; ++i) {
    gammas[i] = new float[n];
    a = alpha[i]; // Single alpha float (category i+1) for gamma parameter.
    std::gamma_distribution<float> g(a, 1.0);
    // Sample the gamma variate for category i+1 for the jth repetition.
    for (int j = 0; j < n; ++j) {
      gam = g(*engine);
      gammas[i][j] = gam;
      gamma_sums[j] += gam;
    }
  }

  // Sample from Multinomial(ps = gamma/sum_gamma, draws) for each repetition.
  for (int j = 0; j < n; ++j) {
    out[j] = new int[d];
    sum_ps = 1.0;
    remaining = draws[j];
    // We calculate a multinomial sample by obtaining binomial samples at each
    // border
    for (int i = 0; i < d - 1; ++i) {
      p = gammas[i][j] / gamma_sums[j];
      std::binomial_distribution<int> b(remaining, p / sum_ps);
      count = b(*engine);
      remaining -= count;
      sum_ps -= p;
      out[j][i] = count;
    }
    out[j][d - 1] = remaining;
  }

  // Cleanup
  for (int i = 0; i < n; ++i) {
    delete[] gammas[i];
  }
  delete[] gammas;
  delete[] gamma_sums;

  return out;
}

/* Simulates a random election from a uniform dirichlet-tree, starting from a
 * specified partial-ballot.
 *
 * Updates an election with sampled ballots which prepended with the first
 * nChosen choices of permutationArray.
 */
void rElections(float scale, int *nBallots, int nElections, int nCandidates,
                int *permutationArray, int nChosen, std::mt19937 *engine,
                bool isVanilla, int *factorials, election *out) {
  BallotCount bc;
  bool atLeastOne;
  int *nextNBallots;
  int **countsForChildren;
  float *alpha;
  if (nChosen == nCandidates - 1) {
    // If ballot is completely specified, return.
    for (int i = 0; i < nElections; ++i) {
      if (nBallots[i] == 0)
        continue;
      bc = *(new BallotCount);
      bc.count = nBallots[i];
      bc.ballotPermutation = new int[nCandidates];
      for (int j = 0; j < nCandidates; ++j) {
        bc.ballotPermutation[j] = permutationArray[j];
      }
      out[i].push_back(bc);
    }
    return;
  }

  // Else we sample from DirichletMulinomial and recursively sample next
  // candidates.
  alpha = new float[nCandidates - nChosen];
  // Initalize alpha to appropriate uniform scale.
  for (int i = 0; i < nCandidates - nChosen; ++i) {
    alpha[i] = scale;
    if (isVanilla)
      alpha[i] = alpha[i] * factorials[nCandidates - nChosen];
  }
  countsForChildren = rDirichletMultinomial(nElections, nBallots, alpha,
                                            nCandidates - nChosen, engine);

  for (int i = 0; i < nCandidates - nChosen; ++i) {
    nextNBallots = new int[nElections];
    atLeastOne = false;
    // construct next nBallots array for samples.
    for (int j = 0; j < nElections; ++j) {
      nextNBallots[j] = countsForChildren[j][i];
      atLeastOne = (atLeastOne || nextNBallots[j]);
    }
    // If there are no non-zero ballot counts for this candidate,
    // we simply skip to return an empty result set for this index.
    if (!atLeastOne) {
      delete[] nextNBallots;
      continue;
    }
    // Update next candidate.
    std::swap(permutationArray[i + nChosen], permutationArray[nChosen]);
    rElections(scale, nextNBallots, nElections, nCandidates, permutationArray,
               nChosen + 1, engine, isVanilla, factorials, out);
    std::swap(permutationArray[i + nChosen], permutationArray[nChosen]);
  }
  for (int i = 0; i < nElections; ++i) {
    delete[] countsForChildren[i];
  }
  delete[] countsForChildren;
  delete[] alpha;
}

#endif

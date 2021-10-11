// distributions.hpp

#ifndef DISTRIBUTIONS_HPP
#define DISTRIBUTIONS_HPP

#include "ballot.hpp"
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
);

/* Simulates a random election from a uniform dirichlet-tree, starting from a
 * specified partial-ballot.
 *
 * Updates an election with sampled ballots which prepended with the first
 * nChosen choices of permutationArray.
 */
void rElections(float scale, int *nBallots, int nElections, int nCandidates,
                int *permutationArray, int nChosen, std::mt19937 *engine,
                bool isVanilla, int *factorials, election *out);

/* Beta distribution for sampling a marginal branch probability.
 */
float rBeta(float alpha, float beta, std::mt19937 *engine);

#endif

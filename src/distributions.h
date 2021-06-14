#ifndef DISTRIBUTIONS_H
#define DISTRIBUTIONS_H

#include <random>
#include <algorithm>
#include "ballot.h"

/* Draws random Dirichlet Multinomial samples as specified by alpha.
 *
 * Returns an array of dirichlet multinomial samples, where each of the
 * samples is an array of category counts.
 */
int** rDirichletMultinomial(
        int n, // Number of repetitions.
        int* draws,  // number of draws per sample.
        float* alpha, // alpha parameter.
        int d, // dimension of alpha parameter.
        std::mt19937 engine // RNG engine.
) {

    int** out = new int*[n];        // output array:
                                    // categorical counts for each repetition.
    float** gammas = new float*[d]; // Array for the gamma(alphaj,1) samples for each repetition.
                                    // Note the dimension-first index.
    int* gamma_sums = new int[n];   // Array of sum of gamma variates for each repetition.
    float sum_ps, p, sum_alpha, a, gam;
    int remaining, count;

    // Sample gamma variates for each alpha. Additionally we calculate a sum of the alpha
    // parameters here.
    sum_alpha = 0.0;
    for( int i = 0; i < d; ++i ){
        gammas[i] = new float[n];
        a = alpha[i]; // Single alpha float (category i+1) for gamma parameter.
        sum_alpha += a;
        std::gamma_distribution<float> g(a,1.0);
        // Sample the gamma variate for category i+1 for the jth repetition.
        for( int j = 0; j < n; ++j ){
            gam = g(engine);
            gammas[i][j] = gam;
            gamma_sums[j] += gam;
        }
    }

    // Sample from Multinomial(ps = gamma/sum_gamma, draws) for each repetition.
    for( int j = 0; j < n; ++j ){
        out[j] = new int[d];
        sum_ps = 1.0;
        remaining = draws[j];
        // We calculate a multinomial sample by obtaining binomial samples at each border
        for( int i = 0; i < d - 1; ++i ){
            p = gammas[i][j]/gamma_sums[j];
            std::binomial_distribution<int> b(remaining, p/sum_ps);
            count = b(engine);
            remaining -= count;
            sum_ps -= p;
            out[j][i] = count;
        }
        out[j][d-1] = remaining;
    }

    return out;
}

/* Simulates a random election from a uniform dirichlet-tree, starting from a specified
 * partial-ballot.
 *
 * Returns an array of elections of length nElections, each with nBallots[i]
 * sampled ballots which are prepended with the first nCompleted choices of indexArray.
 */
election* rElections(
        int* nBallots,
        int nElections,
        int nCandidates,
        int* indexArray,
        int* permutationArray,
        int nChosen,
        std::mt19937 engine
){
    BallotCount* bc;
    bool atLeastOne;
    int* nextNBallots;
    int** countsForChildren;
    election* childBallotSets;
    election* out = new election[nElections];
    float* alpha = new float[nCandidates]; // Initalize alpha to uniform parameter of ones.
    for(int i = 0; i < nChosen - nCandidates; ++i ){
        alpha[i] = 1.;
    }
    if( nChosen == nCandidates - 1 ){ // If ballot is completely specified, return.
        for( int i = 0; i < nElections; ++i ){
            if( nBallots[i] == 0 ) continue;
            bc = new BallotCount;
            bc->count = nBallots[i];
            bc->ballotIndices = new int[nChosen];
            bc->ballotPermutation = new int[nChosen];
            for( int j = nChosen; j >= 0; --j ){
                bc->ballotIndices[j] = indexArray[j];
                bc->ballotPermutation[j] = permutationArray[j];
            }
            out[i].push_back(*bc);
        }
        return out;
    }

    // Else we sample from DirichletMulinomial and recursively sample next candidates.
    countsForChildren = rDirichletMultinomial(
            nElections, nBallots, alpha, nCandidates - nChosen, engine
    );

    for( int i = 0; i < nCandidates - nChosen; ++i ){
        nextNBallots = new int[nElections];
        atLeastOne = 0;
        for( int j = 0; j < nElections; ++j ){ // construct next nBallots array for samples.
            nextNBallots[j] = countsForChildren[j][i];
            atLeastOne = (atLeastOne || nextNBallots[j]);
        }
        // If there are no non-zero ballot counts for this candidate,
        // we simply skip to return an empty result set for this index.
        if( !atLeastOne ) continue;
        // Get candidate number of corresponding index.
        indexArray[nChosen] = i;
        std::swap(permutationArray[i+nChosen], permutationArray[nChosen]);
        childBallotSets = rElections(
            nextNBallots, nElections, nCandidates, indexArray, permutationArray, nChosen + 1, engine
        );
        std::swap(permutationArray[i+nChosen], permutationArray[nChosen]);
        for( int j = 0; j < nElections; ++j ){
            out[j].insert(
                    out[j].end(),
                    std::make_move_iterator(childBallotSets[j].begin()),
                    std::make_move_iterator(childBallotSets[j].end())
            );
        }
    }
    return out;
}

#endif


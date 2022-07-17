/******************************************************************************
 * File:             distributions.cpp
 *
 * Author:           Floyd Everest <me@floydeverest.com>
 * Created:          02/27/22
 * Description:      This file implements the required distributions as
 *                   outlined in `distributions.hpp`.
 *****************************************************************************/

#include "distributions.h"
#include <random>

unsigned *rDirichletMultinomial(unsigned count, float *a, unsigned d,
                                std::mt19937 *engine) {
  unsigned *out = new unsigned[d];

  float *p = rDirichlet(a, d, engine);

  // Sample from Multinomial distribution with p.
  float sum_ps = 1.0;
  for (unsigned i = 0; i < d - 1; ++i) {
    // Calculate marginal probability p.
    // Draw from marginal binomial distribution.
    std::binomial_distribution<unsigned> b(count, p[i]);
    out[i] = b(*engine);
    count -= out[i];
    // Renormalise ps for next categories.
    sum_ps -= p[i];
  }
  out[d - 1] = count;

  delete[] p;

  return out;
}

float *rDirichlet(float *a, unsigned d, std::mt19937 *engine) {
  float sum_ps, p;
  float *gamma = new float[d];
  float gamma_sum = 0.;

  // Sample the gamma variates for category i.
  for (unsigned i = 0; i < d; ++i) {
    std::gamma_distribution<float> g(a[i]);
    gamma[i] = g(*engine);
    gamma_sum += gamma[i];
  }

  // Edge case where all gammas are zero.
  if (gamma_sum == 0.) {
    // Choose index i uniformly at random to have p_i=1, and set all others to
    // p_j=0.
    std::uniform_int_distribution<unsigned> rint(0, d - 1);
    unsigned idx = rint(*engine);
    for (auto i = 0; i < d; ++i)
      gamma[i] = 0.;
    gamma[idx] = 1.;
    return gamma;
  }

  // Otherwise normalize the gamma variates and return.
  for (auto i = 0; i < d; ++i) {
    gamma[i] = gamma[i] / gamma_sum;
  }
  return gamma;
}

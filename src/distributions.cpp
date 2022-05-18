/******************************************************************************
 * File:             distributions.cpp
 *
 * Author:           Floyd Everest <me@floydeverest.com>
 * Created:          02/27/22
 * Description:      This file implements the required distributions as
 *                   outlined in `distributions.hpp`.
 *****************************************************************************/

#include "distributions.h"

unsigned *rDirichletMultinomial(unsigned count, float *a, unsigned d,
                                std::mt19937 *engine) {
  unsigned *out = new unsigned[d];

  float sum_ps, p;
  float *gamma = new float[d];
  float gamma_sum = 0.;

  // Sample the gamma variates for category i.
  for (unsigned i = 0; i < d; ++i) {
    std::gamma_distribution<float> g(a[i], 1.0);
    gamma[i] = g(*engine);
    gamma_sum += gamma[i];
  }

  // Sample from Multinomial distribution with pi=gamma/gamma_sum.
  sum_ps = 1.0;
  for (unsigned i = 0; i < d; ++i) {
    // Calculate marginal probability p.
    if (gamma_sum == 0) {
      p = 1.;
    } else {
      p = gamma[i] / gamma_sum;
    }
    // Draw from marginal binomial distribution.
    std::binomial_distribution<unsigned> b(count, p / sum_ps);
    out[i] = b(*engine);
    count -= out[i];
    // Renormalise ps for next categories.
    sum_ps -= p;
  }

  delete[] gamma;

  return out;
}

float rBeta(float a, float b, std::mt19937 *engine) {
  std::gamma_distribution<float> gx(a);
  std::gamma_distribution<float> gy(b);
  // Avoid zero denominator by adding epsilon.
  float x = gx(*engine) + std::numeric_limits<float>::epsilon();
  float y = gy(*engine) + std::numeric_limits<float>::epsilon();
  return x / (x + y);
}

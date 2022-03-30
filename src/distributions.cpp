/******************************************************************************
 * File:             distributions.cpp
 *
 * Author:           Floyd Everest <me@floydeverest.com>
 * Created:          02/27/22
 * Description:      This file implements the required distributions as
 *                   outlined in `distributions.hpp`.
 *****************************************************************************/

#include "distributions.hpp"

int *rDirichletMultinomial(int count, float *alpha, int d,
                           std::mt19937 *engine) {
  int *out = new int[d];

  float gam, sum_ps, p;
  float *gamma = new float[d];
  float gamma_sum = 0.;

  // Sample the gamma variates for category i.
  for (auto i = 0; i < d; ++i) {
    std::gamma_distribution<float> g(alpha[i], 1.0);
    gam = g(*engine);
    gamma[i] = gam;
    gamma_sum += gam;
  }

  // Sample from Multinomial distribution with pi=gamma/gamma_sum.
  sum_ps = 1.0;
  for (auto i = 0; i < d - 1; ++i) {
    // Calculate marginal probability p.
    if (gamma_sum == 0) {
      p = 1.;
    } else {
      p = gamma[i] / gamma_sum;
    }
    // Draw from marginal binomial distribution.
    std::binomial_distribution<int> b(count, p / sum_ps);
    out[i] = b(*engine);
    count -= out[i];
    // Renormalise ps for next categories.
    sum_ps -= p;
  }
  // Remainder goes to last category.
  out[d - 1] = count;

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

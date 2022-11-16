/******************************************************************************
 * File:             distributions.cpp
 *
 * Author:           Floyd Everest <me@floydeverest.com>
 * Created:          02/27/22
 * Description:      This file implements the required distributions as
 *                   outlined in `distributions.hpp`.
 *****************************************************************************/

#include "distributions.h"

std::vector<unsigned> rDirichletMultinomial(const unsigned &N,
                                            const std::vector<double> &a,
                                            std::mt19937 *engine) {
  // Draw p ~ Dirichlet(a)
  std::vector<double> p = rDirichlet(a, engine);
  // Draw out ~ Multinomial(p)
  return rMultinomial(N, p, engine);
}

std::vector<unsigned> rMultinomial(const unsigned &N,
                                   const std::vector<double> &p,
                                   std::mt19937 *engine) {
  size_t d = p.size();
  std::vector<unsigned> out(p.size());

  // norm is necessary because floating point precision does not often allow
  // the probabilities p to sum to exactly 1.0f.
  double norm = 0.0;
  for (size_t i = 0; i < d; ++i) norm += p[i];

  // Draw from Multinomial(N, p)
  double sum_ps = 0.0;
  double pnorm;
  unsigned n = N;
  for (size_t i = 0; i < d; ++i) {
    if (norm - (sum_ps + p[i]) == 0.0) {
      // First check if this is the last positive p.
      out[i] = n;
      for (size_t j = i + 1; j < d; ++j) out[j] = 0;
      break;
    } else {
      // Otherwise continue to draw using binomial marginals
      pnorm = p[i] / (norm - sum_ps);
      std::binomial_distribution<unsigned> b(n, pnorm);
      out[i] = b(*engine);
      n -= out[i];
      // Normalise remaining ps.
      sum_ps += p[i];
    }
  }
  return out;
}

std::vector<double> rDirichlet(const std::vector<double> &a,
                               std::mt19937 *engine) {
  unsigned d = a.size();
  std::vector<double> gamma(d);
  double gamma_sum = 0.;

  // Sample the gamma variates for category i.
  for (size_t i = 0; i < d; ++i) {
    std::gamma_distribution<double> g(a[i]);
    gamma[i] = g(*engine);
    gamma_sum += gamma[i];
  }

  // Edge case where all gammas are zero.
  if (gamma_sum == 0.) {
    // Choose index i uniformly at random to have p_i=1, and set all others to
    // p_j=0.
    std::uniform_int_distribution<unsigned> rint(0, d - 1);
    unsigned idx = rint(*engine);
    for (size_t i = 0; i < d; ++i) gamma[i] = 0.;
    gamma[idx] = 1.;
    return gamma;
  }

  // Otherwise normalize the gamma variates and return.
  for (size_t i = 0; i < d; ++i) {
    gamma[i] = gamma[i] / gamma_sum;
  }
  return gamma;
}

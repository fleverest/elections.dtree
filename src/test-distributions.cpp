/*
 * This file tests the required distributions.
 */

#include <testthat.h>

#include "distributions.h"

context("Test Dirichlet-Multinomial samples sum to count.") {

  unsigned *result, sum;
  float *a;
  std::mt19937 mte;
  mte.seed(time(NULL));
  // We draw each a parameter from gamma(2,2)
  std::gamma_distribution<float> g(2.0, 2.0);

  // Draw 1000 multinomial samples.
  unsigned count = 1000;

  bool always_sums_to_count = true;

  for (auto z = 0; z < 10; ++z) {
    for (unsigned d = 1; d < 1000; ++d) {
      // Initialize a new a vector.
      a = new float[d];
      for (unsigned i = 0; i < d; ++i)
        a[i] = g(mte);

      result = rDirichletMultinomial(count, a, d, &mte);

      // Sum the result
      sum = 0;
      for (unsigned i = 0; i < d; ++i)
        sum += result[i];

      always_sums_to_count = always_sums_to_count && (sum == count);

      // Delete a and mnom counts
      delete[] a;
      delete[] result;
    }
  }
  test_that("Dirichlet-Multinomial sample sums to count.") {
    expect_true(always_sums_to_count);
  }
}

context("Test dirichlet marginal distributions.") {

  std::mt19937 mte;
  mte.seed(time(NULL));

  unsigned n = 100;
  unsigned n_trials = 1000;

  float *alpha = new float[n];
  for (auto i = 0; i < n; ++i)
    alpha[i] = 1.;

  float *p;
  float sum_p_n = 0.;
  for (auto i = 0; i < n_trials; ++i) {
    p = rDirichlet(alpha, n, &mte);
    sum_p_n += p[n - 1];
    delete[] p;
  }

  delete[] alpha;

  test_that("Last Dirichlet probability has mean approximately 1/n.") {
    expect_true(sum_p_n <
                1.1 * static_cast<float>(n_trials) / static_cast<float>(n));
    expect_true(sum_p_n >
                0.9 * static_cast<float>(n_trials) / static_cast<float>(n));
  }
}

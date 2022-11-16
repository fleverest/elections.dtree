/*
 * This file tests the required distributions.
 */

#include <testthat.h>

#include <vector>

#include "distributions.h"

context("Test Dirichlet-Multinomial samples sum to count.") {
  std::vector<unsigned> result;
  unsigned sum;
  std::vector<double> a;
  std::mt19937 mte;
  mte.seed(time(NULL));
  // We draw each a parameter from gamma(2,2)
  std::gamma_distribution<double> g(2.0, 2.0);

  // Draw 1000 multinomial samples.
  unsigned count = 1000;

  bool always_sums_to_count = true;

  for (auto z = 0; z < 10; ++z) {
    for (unsigned d = 1; d < 1000; ++d) {
      // Initialize a new a vector.
      a = std::vector<double>(d);
      for (unsigned i = 0; i < d; ++i) a[i] = g(mte);

      result = rDirichletMultinomial(count, a, &mte);

      // Sum the result
      sum = 0;
      for (unsigned i = 0; i < d; ++i) sum += result[i];

      always_sums_to_count = always_sums_to_count && (sum == count);
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

  std::vector<double> alpha(n);
  for (unsigned i = 0; i < n; ++i) alpha[i] = 1.;

  std::vector<double> p;
  double sum_p_n = 0.;
  for (unsigned i = 0; i < n_trials; ++i) {
    p = rDirichlet(alpha, &mte);
    sum_p_n += p[n - 1];
  }

  test_that("Last Dirichlet probability has mean approximately 1/n.") {
    expect_true(sum_p_n <
                1.1 * static_cast<double>(n_trials) / static_cast<double>(n));
    expect_true(sum_p_n >
                0.9 * static_cast<double>(n_trials) / static_cast<double>(n));
  }
}

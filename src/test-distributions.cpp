/*
 * This file tests the required distributions.
 */

#include <testthat.h>

#include "distributions.hpp"

context("Test Dirichlet-Multinomial samples sum to count.") {

  unsigned *result, sum;
  float *alpha;
  std::mt19937 mte;
  mte.seed(time(NULL));
  // We draw each alpha parameter from gamma(2,2)
  std::gamma_distribution<float> g(2.0, 2.0);

  // Draw 1000 multinomial samples.
  unsigned count = 1000;

  bool always_sums_to_count = true;

  for (unsigned d = 1; d < 100; ++d) {
    // Initialize a new alpha vector.
    alpha = new float[d];
    for (auto i = 0; i < d; ++i)
      alpha[i] = g(mte);

    result = rDirichletMultinomial(count, alpha, d, &mte);

    // Sum the result
    sum = 0;
    for (auto i = 0; i < d; ++i)
      sum += result[i];

    if (sum != count)
      always_sums_to_count = false;

    // Delete alpha and mnom counts
    delete[] alpha;
    delete[] result;
  }
  test_that("Dirichlet-Multinomial sample sums to count.") {
    expect_true(always_sums_to_count);
  }
}

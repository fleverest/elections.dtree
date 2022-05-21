test_that("Prior marginal probabilities are approximately `1/N!` with fully specified ballots", {

  for (i in 1:10) {
    # Initialize a new tree
    nCandidates <- floor(runif(1, 3, 10))
    dtree <- dirtree.pirv(candidates = LETTERS[1:nCandidates], a0 = 1., minDepth = nCandidates - 1)

    # The ballot to analyse
    ballot <- structure(
      list(LETTERS[1:nCandidates]),
      class = "PIRVBallots",
      candidates = LETTERS[1:nCandidates]
    )

    # The true mean probability for all ballots under a uniform prior.
    ps.mean <- 1 / factorial(nCandidates)

    # Sample 1000 marginal probabilities for a single ballot under the tree prior
    ps.DT <- sampleMPP(dtree, n = 5000, ballot)

    # Sample 1000 marginal probabilities for a single ballot under the dirichlet prior
    dtree$vd <- T
    ps.D <- sampleMPP(dtree, n = 5000, ballot)

    expect_equal(mean(abs(ps.DT / ps.mean)), 1, tolerance = 0.33)
    expect_equal(mean(abs(ps.D / ps.mean)), 1, tolerance = 0.33)
  }
})

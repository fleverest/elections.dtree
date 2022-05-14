test_that("Update and reset have an effect.", {
  for (i in 1:10) {
    # Create a new tree.
    nCandidates = floor(runif(1, 3, 10))
    dtree <- dirtree.pirv(candidates=LETTERS[1:nCandidates], alpha0=1., minDepth=nCandidates-1)

    prior.mean.expected = 1/factorial(nCandidates)

    # The ballot we'll be assessing probabilities for.
    b <- LETTERS[1:nCandidates]

    # Get the mean posterior marginal probability.
    update(dtree, list(b))
    ps <- sampleMPP(dtree, n=5000, b)
    post.mean <- mean(ps)

    expect_gt(post.mean, prior.mean.expected)

    # Get the mean marginal probability of the ballot from the supposed prior.
    reset(dtree)
    ps <- sampleMPP(dtree, n=5000, b)
    prior.mean <- mean(ps)

    expect_equal(prior.mean/prior.mean.expected, 1, tolerance=0.25)
  }
})
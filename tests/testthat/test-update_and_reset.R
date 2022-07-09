test_that("Update and reset have an effect.", {
  for (i in 1:10) {
    # Create a new tree.
    nCandidates <- floor(runif(1, 3, 10))
    dtree <- dirtree.pirv(
      candidates = LETTERS[1:nCandidates],
      a0 = 1.,
      minDepth = nCandidates - 1
    )

    # The ballot we'll be assessing probabilities for.
    b <- structure(
      list(LETTERS[1:nCandidates]),
      class = "PIRVBallots",
      candidates = LETTERS[1:nCandidates]
    )

    # Estimate the posterior probability for candidate 1 winning.
    update(dtree, b)
    p1 <- samplePosterior(dtree, 100, 5)[1]

    # Estimate the prior probability for candidate 1 winning.
    reset(dtree)
    p0 <- samplePosterior(dtree, 100, 5)[1]

    expect_gt(p1, p0)
  }
})

test_that("Update fails with invalid ballot.", {
  dtree <- dirtree.pirv(candidates = LETTERS[1:3])
  # Invalid ballot.
  expect_error({
    update(dtree, structure(
      list(LETTERS[4]),
      candidates = LETTERS[1:3],
      class = "PIRVBallots"
    ))
  })
})

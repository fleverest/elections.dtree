test_that("Update and reset have an effect.", {
  for (i in 1:10) {
    # Create a new tree.
    n_candidates <- floor(runif(1, 3, 10))

    dtree <- dirtree.pirv(
      candidates = LETTERS[1:n_candidates],
      a0 = 1.,
      minDepth = n_candidates - 1,
      maxDepth = n_candidates - 1
    )

    # The ballot we'll be assessing probabilities for.
    b <- PIRVBallots(LETTERS[1:n_candidates])

    # Estimate the posterior probability for candidate 1 winning.
    update(dtree, b)
    p1 <- samplePosterior(dtree, 100, 5)[1]

    # Estimate the prior probability for candidate 1 winning.
    reset(dtree)
    p0 <- samplePosterior(dtree, 100, 5)[1]

    expect_gt(p1, p0)
  }
})

test_that(paste0("Presented with warnings when observing ballots",
                " with fewer preferences than minDepth, but greater",
                " than zero."), {
  dtree <- dirtree.pirv(
    candidates = LETTERS,
    minDepth = 3,
    maxDepth = 10,
    a0 = 0,
    vd = FALSE
  )
  expect_warning({
    update(
      dtree,
      PIRVBallots(LETTERS[1:2])
    )
  })
  dtree$minDepth <- 2
  expect_warning({
    dtree$minDepth <- 3
  })
})

test_that("Update fails with invalid ballot.", {
  dtree <- dirtree.pirv(candidates = LETTERS[1:3])
  # Invalid ballot.
  expect_error({
    update(dtree, PIRVBallots(LETTERS[4]))
  })
})

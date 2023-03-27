test_that("Update and reset have an effect.", {
  for (i in 1:10) {
    # Create a new tree.
    n_candidates <- floor(runif(1, 3, 10))

    dtree <- dirtree(
      candidates = LETTERS[1:n_candidates],
      a0 = 1.,
      min_depth = n_candidates - 1,
      max_depth = n_candidates - 1
    )

    # The ballot we'll be assessing probabilities for.
    b <- prefio::preferences(
      t(1:n_candidates),
      format = "ranking",
      item_names = LETTERS[1:n_candidates]
    )

    # Estimate the posterior probability for candidate 1 winning.
    update(dtree, b)
    p1 <- sample_posterior(dtree, 100, 5)[1]

    # Estimate the prior probability for candidate 1 winning.
    reset(dtree)
    p0 <- sample_posterior(dtree, 100, 5)[1]

    expect_gt(p1, p0)
  }
})

test_that(paste0(
  "Presented with warnings when observing ballots",
  " with fewer preferences than min_depth, but greater",
  " than zero."
), {
  dtree <- dirtree(
    candidates = LETTERS,
    min_depth = 3,
    max_depth = 10,
    a0 = 0,
    vd = FALSE
  )
  expect_warning({
    update(
      dtree,
      prefio::preferences(
        t(c(1:2, rep(NA, 24))),
        format = "ranking",
        item_names = LETTERS
      )
    )
  })
  dtree$min_depth <- 2
  expect_warning({
    dtree$min_depth <- 3
  })
})

test_that("Update fails with invalid ballot.", {
  dtree <- dirtree(candidates = LETTERS[1:3])
  # Invalid ballot.
  expect_error({
    update(
      dtree,
      prefio::preferences(
        t(c(NA, NA, NA, 1)),
        format = "ranking",
        item_names = LETTERS[1:4]
      )
    )
  })
})

test_that("Updating with incorrect structure results in error", {
  dtree <- dirtree(candidates = LETTERS[1:3])
  expect_error({
    update(dtree, list(c("A"), c("B", "A")))
  })
  expect_error({
    dtree$update(list(c("A"), c("B", "A")))
  })
})

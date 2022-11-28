test_that("Posterior distribution shifts after observing data", {
  dtree <- dirtree(candidates = LETTERS[1:4])

  prior_probs <- sample_posterior(dtree, 1000, 10)

  ballot <- ranked_ballots(LETTERS[1:4])

  for (i in 1:5) update(dtree, ballot)

  post_probs_dt <- sample_posterior(dtree, 1000, 10)
  dtree$vd <- TRUE
  post_probs_d <- sample_posterior(dtree, 1000, 10)

  expect_true(post_probs_d[1] > prior_probs[1])
  expect_true(post_probs_dt[1] > prior_probs[1])
  expect_true(all(post_probs_d[2:4] < prior_probs[2:4]))
  expect_true(all(post_probs_dt[2:4] < prior_probs[2:4]))
})

test_that("Posterior is relatively uniform when a0=0.", {
  dtree <- dirtree(candidates = LETTERS[1:10], a0 = 0)
  probs <- sample_posterior(dtree, 1000, 10)
  # Test that they are all around the same with low variance
  expect_gt(1e-2, var(probs))
})

test_that("Posterior cannot be calculated when n_ballots is too low", {
  dtree <- dirtree(candidates = LETTERS[1:10])
  update(dtree, ranked_ballots(LETTERS[1:10]))
  update(dtree, ranked_ballots(LETTERS[10:1]))
  expect_error({
    sample_posterior(dtree, 1, 1)
  })
})

test_that("Posterior cannot be calculated with invalid `n_elections`", {
  dtree <- dirtree(candidates = LETTERS[1:3])
  expect_error({
    sample_posterior(dtree, 0, 1)
  })
})

test_that("Exception is thrown with invalid `n_threads`", {
  dtree <- dirtree(candidates = LETTERS[1:3])
  expect_error({
    sample_posterior(dtree, 2, 2, n_threads = 0)
  })
})

test_that("No exception is thrown with `n_threads` > maximum available", {
  skip_on_cran() # Exceeds maximum number of cores
  dtree <- dirtree(candidates = LETTERS[1:3])
  expect_true(any(sample_posterior(dtree, 2, 2, n_threads = Inf) > 0))
  expect_true(
    any(sample_posterior(dtree, 2, 2,
      n_threads = parallel::detectCores() + 1
    ) > 0)
  )
})

test_that(paste0(
  "Posterior yields alternative winners ",
  "when sampling with replacement"
), {
  dtree <- dirtree(candidates = LETTERS[1:3])
  ballots <- ranked_ballots(list(
    LETTERS[1:3],
    LETTERS[1:3],
    LETTERS[3:1]
  ))
  dtree$update(ballots)
  res <- dtree$sample_posterior(1000, 3, replace = TRUE)
  # We expect more than one outcome in the support of the posterior.
  expect_true(sum(res > 0) > 1)
})

test_that("Posterior distribution shifts after observing data", {
  dtree <- dirtree.pirv(candidates = LETTERS[1:4])

  prior_probs <- samplePosterior(dtree, 1000, 10)

  ballot <- PIRVBallots(LETTERS[1:4])

  for (i in 1:5) update(dtree, ballot)

  post_probs_dt <- samplePosterior(dtree, 1000, 10)
  dtree$vd <- TRUE
  post_probs_d <- samplePosterior(dtree, 1000, 10)

  expect_true(post_probs_d[1] > prior_probs[1])
  expect_true(post_probs_dt[1] > prior_probs[1])
  expect_true(all(post_probs_d[2:4] < prior_probs[2:4]))
  expect_true(all(post_probs_dt[2:4] < prior_probs[2:4]))
})

test_that("Posterior is relatively uniform when a0=0.", {
  dtree <- dirtree.pirv(candidates = LETTERS[1:10], a0 = 0)
  probs <- samplePosterior(dtree, 1000, 10)
  # Test that they are all around the same with low variance
  expect_gt(1e-2, var(probs))
})

test_that("Posterior cannot be calculated when nBallots is too low", {
  dtree <- dirtree.pirv(candidates = LETTERS[1:10])
  update(dtree, PIRVBallots(LETTERS[1:10]))
  update(dtree, PIRVBallots(LETTERS[10:1]))
  expect_error({
    samplePosterior(dtree, 1, 1)
  })
})
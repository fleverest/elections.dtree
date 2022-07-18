test_that(paste0("Bayesian bootstrap posterior can sample truncated ballots ",
                "with maxDepth preferences"), {
  dtree <- dirtree.pirv(
    candidates = LETTERS[1:5],
    a0 = 0,
    minDepth = 0,
    maxDepth = 3,
    vd = FALSE
  )
  bs <- PIRVBallots(c("C", "B", "A", "D", "E"))

  update(dtree, bs)

  test_sample <- samplePredictive(dtree, 1)

  expect_true(all(test_sample[[1]][1:3] == bs[[1]][1:3]))
})

test_that("samplePredictive produces valid ballots", {
  dtree <- dirtree.pirv(
    candidates = LETTERS[1:10],
    a0 = 1.,
    minDepth = 2,
    maxDepth = 8,
    vd = FALSE
  )

  test_samples <- samplePredictive(dtree, 1000)

  expect_true(all(lapply(test_samples, length) >= 2))
  expect_true(all(lapply(test_samples, length) <= 8))
})

test_that("samplePredictive produces the correct number of ballots when a0=0", {
  dtree <- dirtree.pirv(
    candidates = LETTERS[1:10],
    a0 = 1.,
    minDepth = 3,
    vd = FALSE
  )

  initial_samples <- samplePredictive(dtree, 1000)

  update(dtree, initial_samples)
  dtree$a0 <- 0
  dtree$maxDepth <- 7

  expect_equal(length(samplePredictive(dtree, 1000)), 1000)
})

test_that("minDepth cannot be less than maxDepth.", {
  expect_error({
    dtree <- dirtree.pirv(candidates = LETTERS, minDepth = 4, maxDepth = 3)
  })

  dtree_min3 <- dirtree.pirv(candidates = LETTERS, minDepth = 3)
  expect_error({
    dtree_min3$maxDepth <- 2
  })

  dtree_max3 <- dirtree.pirv(candidates = LETTERS, maxDepth = 3)
  expect_error({
    dtree_max3$minDepth <- 4
  })
})

test_that(paste0(
  "Bayesian bootstrap posterior can sample truncated ballots ",
  "with max_depth preferences"
), {
  dtree <- dirtree(
    candidates = LETTERS[1:5],
    a0 = 0,
    min_depth = 0,
    max_depth = 3,
    vd = FALSE
  )
  bs <- ranked_ballots(c("C", "B", "A", "D", "E"))

  update(dtree, bs)

  test_sample <- sample_predictive(dtree, 1)

  expect_true(all(test_sample[[1]][1:3] == bs[[1]][1:3]))
})

test_that("sample_predictive produces valid ballots", {
  dtree <- dirtree(
    candidates = LETTERS[1:10],
    a0 = 1.,
    min_depth = 2,
    max_depth = 8,
    vd = FALSE
  )

  test_samples <- sample_predictive(dtree, 1000)

  expect_true(all(lapply(test_samples, length) >= 2))
  expect_true(all(lapply(test_samples, length) <= 8))
})

test_that("sample_predictive produces correct number of ballots when a0=0", {
  dtree <- dirtree(
    candidates = LETTERS[1:10],
    a0 = 1.,
    min_depth = 3,
    vd = FALSE
  )

  initial_samples <- sample_predictive(dtree, 1000)

  update(dtree, initial_samples)
  dtree$a0 <- 0
  dtree$max_depth <- 7

  expect_equal(length(sample_predictive(dtree, 1000)), 1000)
})

test_that("min_depth cannot be less than max_depth.", {
  expect_error({
    dtree <- dirtree(candidates = LETTERS, min_depth = 4, max_depth = 3)
  })

  dtree_min3 <- dirtree(candidates = LETTERS, min_depth = 3)
  expect_error({
    dtree_min3$max_depth <- 2
  })

  dtree_max3 <- dirtree(candidates = LETTERS, max_depth = 3)
  expect_error({
    dtree_max3$min_depth <- 4
  })
})

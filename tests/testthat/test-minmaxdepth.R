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
  bs <- prefio::preferences(
    matrix(
      c(3, 2, 1, 4, 5),
      ncol = 5,
      byrow = TRUE
    ),
    format = "ranking",
    item_names = LETTERS[1:5]
  )

  update(dtree, bs)

  test_sample <- sample_predictive(dtree, 1)

  expect_true(test_sample$preferences == bs[, 1:3, by.rank = TRUE])
})

test_that("sample_predictive produces valid-length ballots", {
  dtree <- dirtree(
    candidates = LETTERS[1:10],
    a0 = 1.,
    min_depth = 2,
    max_depth = 8,
    vd = FALSE
  )

  test_samples <- sample_predictive(dtree, 1000)$preferences

  expect_true(all(rowSums(!is.na(as.matrix(test_samples))) >= 2))
  expect_true(all(rowSums(!is.na(as.matrix(test_samples))) <= 8))
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

  expect_equal(sum(sample_predictive(dtree, 1000)$frequencies), 1000)
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

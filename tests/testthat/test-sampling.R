test_that("Predictive distribution sampling adheres to depth restrictions", {
  min_depth <- 3L
  max_depth <- 7L
  dtree <- dirtree(
    candidates = LETTERS[1L:10L],
    min_depth = min_depth,
    max_depth = max_depth,
    a0 = 10L
  )

  ballots <- sample_predictive(dtree, 100L)

  expect_true(
    all(rowSums(!is.na(as.matrix(ballots$preferences))) >= min_depth)
  )
  expect_true(
    all(rowSums(!is.na(as.matrix(ballots$preferences))) <= max_depth)
  )
})

test_that("Predictive sampling fails when arguments don't make sense.", {
  dtree <- dirtree(
    candidates = LETTERS[1L:10L],
    a0 = 1L
  )

  expect_error({
    sample_predictive(dtree, -1L)
  })
})

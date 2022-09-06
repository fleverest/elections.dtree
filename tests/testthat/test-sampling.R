test_that("Predictive distribution sampling adheres to depth restrictions", {
  min_depth <- 3
  max_depth <- 7
  dtree <- dirtree(
    candidates = LETTERS[1:10],
    min_depth = min_depth,
    max_depth = max_depth,
    a0 = 1
  )

  ballots <- sample_predictive(dtree, 100)

  min_len <- min_depth
  max_len <- max_depth
  for (b in ballots) {
    len <- length(b)
    if (len < min_len) min_len <- len
    if (len > max_len) max_len <- len
  }

  expect_true(min_len >= min_depth)
  expect_true(max_len <= max_depth)
})

test_that("Predictive sampling fails when arguments don't make sense.", {
  dtree <- dirtree(
    candidates = LETTERS[1:10],
    a0 = 1
  )

  expect_error({
    sample_predictive(dtree, -1)
  })
})

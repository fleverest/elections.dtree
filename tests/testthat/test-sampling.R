test_that("Predictive distribution sampling adheres to depth restrictions", {
  min_depth <- 3
  max_depth <- 7
  dtree <- dirtree.pirv(
    candidates = LETTERS[1:10],
    minDepth = min_depth,
    maxDepth = max_depth,
    a0 = 1
  )

  ballots <- samplePredictive(dtree, 100)

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
  dtree <- dirtree.pirv(
    candidates = LETTERS[1:10],
    a0 = 1
  )

  expect_error({
    samplePredictive(dtree, -1)
  })
})

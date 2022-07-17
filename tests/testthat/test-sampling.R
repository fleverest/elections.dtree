test_that("Predictive distribution sampling adheres to depth restrictions", {
  min.depth <- 3
  max.depth <- 7
  dtree <- dirtree.pirv(
    candidates = LETTERS[1:10],
    minDepth = min.depth,
    maxDepth = max.depth,
    a0=1
  )

  ballots <- samplePredictive(dtree, 100)

  min.len <- min.depth
  max.len <- max.depth
  for (b in ballots) {
    len <- length(b)
    if (len < min.len) min.len <- len
    if (len > max.len) max.len <- len
  }

  expect_true(min.len >= min.depth)
  expect_true(max.len <= max.depth)
})

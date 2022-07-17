test_that("Bayesian bootstrap posterior can sample truncated ballots with maxDepth preferences", {
  dtree <- dirtree.pirv(
    candidates = LETTERS[1:5],
    a0 = 0,
    minDepth = 0,
    maxDepth = 3,
    vd = F
  )
  bs <- PIRVBallots(c("C", "B", "A", "D", "E"))

  update(dtree, bs)

  test.sample <- samplePredictive(dtree, 1)

  expect_true(all(test.sample[[1]][1:3] == bs[[1]][1:3]))
})

test_that("samplePredictive produces valid ballots", {
  dtree <- dirtree.pirv(
    candidates = LETTERS[1:10],
    a0 = 1.,
    minDepth = 2,
    maxDepth = 8,
    vd = F
  )

  test.samples <- samplePredictive(dtree, 1000)

  expect_true(all(lapply(test.samples, length)>=2))
  expect_true(all(lapply(test.samples, length)<=8))
})

test_that("samplePredictive produces the correct number of ballots when a0=0", {
  dtree <- dirtree.pirv(
    candidates = LETTERS[1:10],
    a0 = 1.,
    minDepth = 3,
    vd = F
  )

  initial.samples <- samplePredictive(dtree, 1000)

  update(dtree, initial.samples)
  dtree$a0 <- 0
  dtree$maxDepth <- 7

  expect_equal(length(samplePredictive(dtree, 1000)), 1000)
})

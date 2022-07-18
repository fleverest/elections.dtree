dtree <- dirtree.pirv(
  candidates = LETTERS[1:3],
  a0 = 1,
  minDepth = 0,
  vd = FALSE
)

test_that("Subsets of ballots have same attributes.", {
  ballots <- samplePredictive(dtree, 100)
  subset <- ballots[1:10]
  expect_true(
    identical(
      attributes(ballots),
      attributes(subset)
    )
  )
})

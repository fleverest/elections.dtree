dtree <- dirtree(
  candidates = LETTERS[1:3],
  a0 = 1,
  min_depth = 0,
  vd = FALSE
)

test_that("Subsets of ballots have same attributes.", {
  ballots <- sample_predictive(dtree, 100)
  subset <- ballots[1:10]
  expect_true(
    identical(
      attributes(ballots),
      attributes(subset)
    )
  )
})

dtree <- dirtree(
  candidates = LETTERS[1:3],
  a0 = 1,
  min_depth = 0,
  vd = FALSE
)

test_that("ranked_ballots raises error on invalid input", {
  expect_error({
    ranked_ballots(list(c("A", "B"), c("C", "D")), candidates = LETTERS[1:3])
  })
  expect_error({
    ranked_ballots(c("A", "A"), candidates = c("A"))
  })
})


test_that("ranked_ballots can construct empty sets.", {
  expect_equal(
    ranked_ballots(list(), candidates = LETTERS),
    ranked_ballots(c(), candidates = LETTERS)
  )
})

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

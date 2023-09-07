dtree <- dirtree(
  candidates = LETTERS[1:3],
  a0 = 1,
  min_depth = 0,
  vd = FALSE
)

test_that("ranked_ballots raises error on invalid input", {
  expect_error({
    suppressWarnings(
      ranked_ballots(list(c("A", "B"), c("C", "D")), candidates = LETTERS[1:3])
    )
  })
  expect_error({
    suppressWarnings(
      ranked_ballots(c("A", "A"), candidates = c("A"))
    )
  })
})


test_that("ranked_ballots can construct empty sets.", {
  expect_warning({
    ranked_ballots(c(), candidates = LETTERS)
  })
  expect_warning({
    ranked_ballots(list(), candidates = LETTERS)
  })
  expect_equal(
    suppressWarnings(ranked_ballots(list(), candidates = LETTERS)),
    suppressWarnings(ranked_ballots(c(), candidates = LETTERS))
  )
})

test_that("Subsets of ballots have same attributes.", {
  ballots <- suppressWarnings(
    ranked_ballots(list(LETTERS[1:5], LETTERS[2:4], LETTERS[2:4]))
  )
  subset <- ballots[1:2]
  expect_true(
    identical(
      attributes(ballots),
      attributes(subset)
    )
  )
})

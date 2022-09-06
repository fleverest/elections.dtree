test_that("Social choice works on one basic example", {
  ballots <- ranked_ballots(list(
    c("A"), c("A"), c("A"),
    c("B"), c("B"), c("B"),
    c("C", "A"),
    c()
  ))

  result <- social_choice(ballots)
  expect_equal(result$elimination_order, c("C", "B"))
  expect_equal(result$winner, "A")
})

test_that("Social choice raises error when an unknown candidate is passed.", {
  ballots <- ranked_ballots(list(
    c("A"), c("A"), c("A"),
    c("B"), c("B"), c("B"),
    c("C", "A"),
    c()
  ))

  ballots[[1]] <- c("D")

  expect_error({
    social_choice(ballots)
  })
})

test_that("Social choice gives error when conditions don't make sense.", {
  # n_winners is invalid
  ballots <- ranked_ballots(c("A", "B"))
  expect_error({
    social_choice(ballots, n_winners = 0)
  })

  # All ballots are null
  ballots[[1]] <- c()
  expect_error({
    social_choice(ballots)
  })
})

test_that("Calling with undefined social choice function raises error", {
  ballots <- ranked_ballots(c("A", "B"))
  expect_error({
    social_choice(ballots, n_winners = 1, fn = "undefined")
  })
})

test_that("Social choice works on one basic example", {
  ballots <- PIRVBallots(list(
    c("A"), c("A"), c("A"),
    c("B"), c("B"), c("B"),
    c("C", "A"),
    c()
  ))

  result <- social.choice(ballots)
  expect_equal(result$elimination_order, c("C", "B"))
  expect_equal(result$winner, "A")
})

test_that("Social choice raises error when an unknown candidate is passed.", {
  ballots <- PIRVBallots(list(
    c("A"), c("A"), c("A"),
    c("B"), c("B"), c("B"),
    c("C", "A"),
    c()
  ))

  ballots[[1]] <- c("D")

  expect_error({
    social.choice(ballots)
  })
})

test_that("Social choice gives error when conditions don't make sense.", {
  # nWinners is invalid
  ballots <- PIRVBallots(c("A", "B"))
  expect_error({
    social.choice(ballots, nWinners = 0)
  })

  # All ballots are null
  ballots[[1]] <- c()
  expect_error({
    social.choice(ballots)
  })
})

test_that("Social choice works on one basic example", {
  ballots <- list(
    c("A"), c("A"), c("A"),
    c("B"), c("B"), c("B"),
    c("C", "A")
  )

  ballots <- structure(ballots, class = "PIRVBallots", candidates = LETTERS[1:3])

  result <- social.choice(ballots)
  expect_equal(result$elimination_order, c("C", "B"))
  expect_equal(result$winner, "A")
})

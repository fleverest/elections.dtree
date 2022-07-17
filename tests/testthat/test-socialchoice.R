test_that("Social choice works on one basic example", {
  ballots <- PIRVBallots(list(
    c("A"), c("A"), c("A"),
    c("B"), c("B"), c("B"),
    c("C", "A")
  ))

  result <- social.choice(ballots)
  expect_equal(result$elimination_order, c("C", "B"))
  expect_equal(result$winner, "A")
})

test_that("read.ballots and write.ballots are inverse.", {
  bs <- PIRVBallots(list(LETTERS[1:5], LETTERS[2:4], LETTERS[2:4]))
  expect_true(identical(
    bs,
    read.ballots(write.ballots(bs, returnLines = TRUE, suppress = TRUE))
  ))

  lines <- c(
    "A, B, C, D, E",
    "A, B, C, D, E",
    "-+-+-+-+-",
    "(A, B, C, D, E) : 1",
    "(B, C, D) : 2"
  )
  expect_true(all(
    lines == write.ballots(
      read.ballots(lines),
      returnLines = TRUE,
      suppress = TRUE
    )
  ))
})

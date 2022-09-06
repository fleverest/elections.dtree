test_that("read_ballots and write_ballots are inverse.", {
  bs <- ranked_ballots(list(LETTERS[1:5], LETTERS[2:4], LETTERS[2:4]))
  expect_true(identical(
    bs,
    read_ballots(write_ballots(bs, return_lines = TRUE, suppress = TRUE))
  ))

  # Ensure all candidates show in the output
  for (l in LETTERS[1:5]) {
    expect_output(
      write_ballots(bs),
      l
    )
  }

  lines <- c(
    "A, B, C, D, E",
    "A, B, C, D, E",
    "-+-+-+-+-",
    "(A, B, C, D, E) : 1",
    "(B, C, D) : 2"
  )
  expect_true(all(
    lines == write_ballots(
      read_ballots(lines),
      return_lines = TRUE,
      suppress = TRUE
    )
  ))
})

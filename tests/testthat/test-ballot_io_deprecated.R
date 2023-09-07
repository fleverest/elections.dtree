test_that("read_ballots and write_ballots are inverse.", {
  bs <- suppressWarnings(
    ranked_ballots(list(LETTERS[1:5], LETTERS[2:4], LETTERS[2:4]))
  )
  expect_warning({
    out <- write_ballots(bs, return_lines = TRUE, suppress = TRUE)
  })
  expect_warning({
    read_ballots(out)
  })
  expect_true(suppressWarnings(identical(
    bs,
    read_ballots(write_ballots(bs, return_lines = TRUE, suppress = TRUE))
  )))

  # Ensure all candidates show in the output
  for (l in LETTERS[1:5]) {
    expect_output(
      suppressWarnings(write_ballots(bs)),
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
    lines == suppressWarnings(write_ballots(
      read_ballots(lines),
      return_lines = TRUE,
      suppress = TRUE
    ))
  ))
})

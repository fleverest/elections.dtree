test_that("Print shows all candidates.", {
  dtree <- dirtree(candidates = LETTERS)
  for (l in LETTERS) {
    expect_output(
      dtree$print(),
      l
    )
  }
})

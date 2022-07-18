test_that("Constructor shows errors for invalid input.", {
  expect_error({
    dirtree.pirv(candidates = c(1, 2, 3)) # Candidates must be character vecs
  })
  expect_error({
    dirtree.pirv(candidates = c("A", "A", "B")) # Candidates must be unique
  })
  expect_error({
    dirtree.pirv(candidates = LETTERS, minDepth = 5, maxDepth = 4)
  })
  expect_error({
    dirtree.pirv(candidates = LETTERS, a0 = -1) # invalid a0 (< 0)
  })
  expect_error({
    dirtree.pirv(candidates = LETTERS, vd = "dirichlet") # invalid vd flag
  })
})

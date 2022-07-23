dtree <- dirtree(
  candidates = LETTERS[1:3],
  a0 = 1,
  min_depth = 0,
  vd = FALSE
)

test_that("Can update a0", {
  dtree$a0 <- 0
  expect_equal(dtree$a0, 0)
  dtree$a0 <- 1000
  expect_equal(dtree$a0, 1000)
})

test_that("Can update min_depth", {
  dtree$min_depth <- 2
  expect_equal(dtree$min_depth, 2)
  dtree$min_depth <- 1
  expect_equal(dtree$min_depth, 1)
})

test_that("Can update max_depth", {
  dtree$max_depth <- 1
  expect_equal(dtree$max_depth, 1)
  dtree$max_depth <- 2
  expect_equal(dtree$max_depth, 2)
})

test_that("Can update vd", {
  expect_equal(dtree$vd, FALSE)
  dtree$vd <- TRUE
  expect_equal(dtree$vd, TRUE)
  dtree$vd <- FALSE
  expect_equal(dtree$vd, FALSE)
})

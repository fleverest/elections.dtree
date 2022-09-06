dtree <- dirtree(
  candidates = LETTERS[1:4],
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

test_that("Invalid a0 raises error", {
  expect_error({
    dtree$a0 <- -1
  })
  expect_error({
    dtree$a0 <- "test"
  })
})

test_that("Invalid min_depth raises error", {
  expect_error({
    dtree$min_depth <- -1
  })
  expect_error({
    dtree$min_depth <- "test"
  })
  expect_error({
    dtree$max_depth <- 2
    dtree$min_depth <- 3
  })
  dtree$max_depth <- 3
})

test_that("Invalid max_depth raises error", {
  expect_error({
    dtree$max_depth <- -1
  })
  expect_error({
    dtree$max_depth <- "test"
  })
  expect_error({
    dtree$min_depth <- 2
    dtree$max_depth <- 1
  })
  dtree$min_depth <- 0
})

test_that("Invalid vd raises error", {
  expect_error({
    dtree$vd <- 15
  })
  expect_error({
    dtree$vd <- "test"
  })
})

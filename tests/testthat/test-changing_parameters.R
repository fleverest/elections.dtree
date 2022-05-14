dtree <- dirtree.pirv(candidates=LETTERS[1:3], alpha0=1, minDepth=0, vd=F)

test_that("Can update alpha0", {
  dtree$alpha0 <- 0
  expect_equal(dtree$alpha0, 0)
  dtree$alpha0 <- 1000
  expect_equal(dtree$alpha0, 1000)
})

test_that("Can update minDepth", {
  dtree$minDepth <- 2
  expect_equal(dtree$minDepth, 2)
  dtree$minDepth <- 1
  expect_equal(dtree$minDepth, 1)
})

test_that("Can update vd", {
  expect_equal(dtree$vd, F)
  dtree$vd <- T
  expect_equal(dtree$vd, T)
  dtree$vd <- F
  expect_equal(dtree$vd, F)
})
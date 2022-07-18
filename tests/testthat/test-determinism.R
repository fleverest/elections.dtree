seed <- runif(1, 0, 1000)

dtree <- dirtree.pirv(candidates = LETTERS[1:10], a0 = 1., minDepth = 0)

test_that("`samplePredictive` is deterministic with specified seed", {
  set.seed(seed)
  bs_1 <- samplePredictive(dtree, 1000)
  set.seed(seed)
  bs_2 <- samplePredictive(dtree, 1000)
  expect_true(
    identical(bs_1, bs_2),
    "`samplePredictive` is not deterministic with constant seed."
  )
})

test_that(paste0("`samplePosterior` (nElections=1) ",
                 "is deterministic with specified seed"), {
  set.seed(seed)
  ps_1 <- samplePosterior(dtree, 1, 1000)
  set.seed(seed)
  ps_2 <- samplePosterior(dtree, 1, 1000)
  expect_true(
    identical(ps_1, ps_2),
    "`samplePosterior` not deterministic on a single thread."
  )
})

test_that(paste0("`samplePosterior` (nElections=100) is deterministic with ",
                 "specified seed"), {
  set.seed(seed)
  ps_1 <- samplePosterior(dtree, 100, 1000)
  set.seed(seed)
  ps_2 <- samplePosterior(dtree, 100, 1000)
  expect_true(
    identical(ps_1, ps_2),
    "`samplePosterior` not deterministic on multiple threads."
  )
})

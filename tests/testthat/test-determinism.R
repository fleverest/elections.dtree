seed <- runif(1, 0, 1000)

dtree <- dirtree.pirv(candidates=LETTERS[1:10], alpha0=1., minDepth=0)

test_that("`samplePredictive` is deterministic with specified seed", {
  set.seed(seed)
  bs.1 <- samplePredictive(dtree, 1000)
  set.seed(seed)
  bs.2 <- samplePredictive(dtree, 1000)
  expect_true(
    identical(bs.1, bs.2),
    "`samplePredictive` is not deterministic with constant seed."
  )
})

test_that("`samplePosterior` (nElections=1) is deterministic with specified seed", {
  set.seed(seed)
  ps.1 <- samplePosterior(dtree, 1, 1000)
  set.seed(seed)
  ps.2 <- samplePosterior(dtree, 1, 1000)
  expect_true(
    identical(ps.1, ps.2),
    "`samplePosterior` is not deterministic with constant seed when nElections=1."
  )
})

test_that("`samplePosterior` (nElections=100) is deterministic with specified seed", {
  set.seed(seed)
  ps.1 <- samplePosterior(dtree, 100, 1000)
  set.seed(seed)
  ps.2 <- samplePosterior(dtree, 100, 1000)
  expect_true(
    identical(ps.1, ps.2),
    "`samplePosterior` is not deterministic with constant seed when nElections=100."
  )
})

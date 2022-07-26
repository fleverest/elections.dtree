seed <- runif(1, 0, 1000)

dtree <- dirtree(candidates = LETTERS[1:10], a0 = 1., min_depth = 0)

test_that("`sample_predictive` is deterministic with specified seed", {
  set.seed(seed)
  bs_1 <- sample_predictive(dtree, 1000)
  set.seed(seed)
  bs_2 <- sample_predictive(dtree, 1000)
  expect_true(
    identical(bs_1, bs_2),
    "`sample_predictive` is not deterministic with constant seed."
  )
})

test_that(paste0(
  "`sample_posterior` (n_elections=1) ",
  "is deterministic with specified seed"
), {
  set.seed(seed)
  ps_1 <- sample_posterior(dtree, 1, 1000)
  set.seed(seed)
  ps_2 <- sample_posterior(dtree, 1, 1000)
  expect_true(
    identical(ps_1, ps_2),
    "`sample_posterior` not deterministic on a single thread."
  )
})

test_that(paste0(
  "`sample_posterior` (n_elections=100) is deterministic with ",
  "specified seed"
), {
  set.seed(seed)
  ps_1 <- sample_posterior(dtree, 100, 1000)
  set.seed(seed)
  ps_2 <- sample_posterior(dtree, 100, 1000)
  expect_true(
    identical(ps_1, ps_2),
    "`sample_posterior` not deterministic on multiple threads."
  )
})

test_that("Posterior distribution shifts after observing data", {
  dtree <- dirtree.pirv(candidates = LETTERS[1:4])

  prior.probs <- samplePosterior(dtree, 1000, 10)

  ballot <- structure(
    list(LETTERS[1:4]),
    class = "PIRVBallots",
    candidates = LETTERS[1:4]
  )

  for (i in 1:5) update(dtree, ballot)

  post.probs.DT <- samplePosterior(dtree, 1000, 10)
  dtree$vd <- T
  post.probs.D <- samplePosterior(dtree, 1000, 10)

  expect_true(post.probs.D[1] > prior.probs[1])
  expect_true(post.probs.DT[1] > prior.probs[1])
  expect_true(all(post.probs.D[2:4] < prior.probs[2:4]))
  expect_true(all(post.probs.DT[2:4] < prior.probs[2:4]))
})

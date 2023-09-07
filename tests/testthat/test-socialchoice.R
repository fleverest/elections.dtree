wakehurst2023 <- prefio::as.preferences(
  prefio::read_preflib("../data/wakehurst2023.soi")
)

test_that("social_choice works for plurality (Wakehurst 2023)", {
  ballots_pref1 <- wakehurst2023[, 1, by.rank = TRUE]
  winners <- social_choice(ballots_pref1, sc_function = "plurality")
  expect_true("WILLIAMS Toby" == winners)
})

test_that("social_choice works for irv (Wakehurst 2023)", {
  outcome <- social_choice(wakehurst2023, sc_function = "irv")
  expect_true(outcome$winners == "REGAN Michael")
  expect_true(
    all(outcome$elimination_order == c(
      "MAWSON Greg",
      "SORENSEN Susan",
      "HRNJAK Ethan",
      "WRIGHT Sue",
      "WILLIAMS Toby"
    ))
  )
})

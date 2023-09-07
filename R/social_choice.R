social_choice <- function(ballots,
                          sc_function = c("plurality", "irv", "stv"),
                          n_winners = 1,
                          ...) {
  fn <- try(match.arg(sc_function), silent = TRUE)
  if (inherits(fmt, "try-error")) {
    stop(
      "Social choice function '", fn, "' not implemented: Must be one ",
      "of 'plurality', 'irv' or 'stv'."
    )
  }

  if (!"preferences" %in% class(ballots)) {
    stop("`ballots` must be of class `prefio::preferences`.")
  }

  if (fn == "plurality") {
    # Aggregate the preferences
    ag_ballots <- aggregate(ballots)
    # Project ballots onto their first preferences only.
    ag_ballots <- ag_ballots[, 1, by.rank = TRUE]
    # Find winners by index
    windex <- which(ag_ballots$frequencies == max(ag_ballots$frequencies))
    winners <- unname(
      unlist(ag_ballots$preferences[windex, as.ordering = TRUE])
    )
    return(winners)
  } else if (fn == "irv") {
    bs <- lapply(
      seq_along(prefs),
      function(i) unname(unlist(prefs[i, as.ordering = TRUE]))
    )
    return(social_choice_irv(bs,
      nWinners = 1,
      candidates = names(ballots),
      seed = gseed()
    ))
  } else if (fn == "stv") {
    stop("'stv' social choice not implemented.")
  }
}

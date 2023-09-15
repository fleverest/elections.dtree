#' Election Social Choice Functions
#'
#' @description
#' Compute election outcomes on ranked ballots with a variety of social choice
#' functions.
#'
#' @param ballots
#' A `prefio::preferences` object containing the ballots cast in the election.
#'
#' @param sc_function
#' One of "plurality", "irv" or "stv", corresponding to the social choice
#' function you wish to evaluate.
#'
#' @param n_winners
#' Refers to the number of seats available when `sc_function` is "stv".
#'
#' @param ...
#' Unused.
#'
#' @keywords social choice election irv stv plurality
#'
#' @return
#' The output depends on the chosen `sc_function`:
#' \describe{
#'    \item{"plurality"}{A character vector with the candidate(s) who received
#'                       the most votes.}
#'    \item{"irv"}{A named `list` with two objects. First, "elimination_order"
#'                 is a vector with each eliminated candidate in the order of
#'                 elimination. Second, "winners" is the vector containing the
#'                 winning candidate(s).}
#'    \item{"stv"}{Not yet implemented.}
#' }
#' @export
#' @importFrom stats aggregate na.omit
social_choice <- function(ballots,
                          sc_function = c("plurality", "irv", "stv"),
                          n_winners = 1,
                          ...) {
  fn <- try(match.arg(sc_function), silent = TRUE)
  if (inherits(fn, "try-error")) {
    stop(
      "Social choice function '", fn, "' not implemented: Must be one ",
      "of 'plurality', 'irv' or 'stv'."
    )
  }

  if (!"preferences" %in% class(ballots)) {
    stop("`ballots` must be of class `prefio::preferences`.")
  }

  if (suppressWarnings(max(ballots, na.rm = TRUE)) == -Inf) {
    warning(paste0(
      "`ballots` does not contain any entry with at least one ",
      "preference. Any social choice is meaningless."
    ))
  }

  if (fn == "plurality") {
    # Show a warning if any ballots specify more than one candidate.
    if (suppressWarnings(max(ballots, na.rm = TRUE)) > 1) {
      warning(paste0(
        "`ballots` contains entries with more than one ",
        "preference. To compute the plurality social choice ",
        "outcome, they will be truncated to their first ",
        "preferences only."
      ))
    }
    # Aggregate the preferences
    ag_ballots <- aggregate(ballots)
    # Project ballots onto their first preferences only.
    ag_ballots <- ag_ballots[, 1, by.rank = TRUE]
    # Remove blank ballots
    ag_ballots <- ag_ballots[apply(
      ag_ballots$preferences,
      1L,
      function(b) length(na.omit(b))
    ) > 0, ]
    # Find winners by index
    windex <- which(ag_ballots$frequencies == max(ag_ballots$frequencies))
    winners <- unname(
      unlist(ag_ballots$preferences[windex, as.ordering = TRUE])
    )
    return(winners)
  } else if (fn == "irv") {
    bs <- lapply(
      seq_along(ballots),
      function(i) unname(unlist(ballots[i, as.ordering = TRUE]))
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

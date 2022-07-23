#' @name dirtree
#'
#' @aliases dirichlet_tree dtree
#'
#' @title
#' Create a Dirichlet Tree model
#'
#' @description
#' \code{dirtree} is used to create a Dirichlet Tree for modelling ballots,
#' as described by \insertCite{dtree_evoteid;textual}{dirtree.elections}.
#'
#' @keywords dirichlet tree irv election ballot
#'
#' @param candidates
#' A character vector, with each element (must be unique) representing a single
#' candidate.
#'
#' @param min_depth
#' The minimum number of candidates which must be specified for a valid ballot.
#'
#' @param max_depth
#' The maximum number of candidates which can be specified for a valid ballot.
#'
#' @param a0
#' The prior parameter for the distribution.
#'
#' @param vd
#' A flag which, when \code{TRUE}, employs a parameter structure which reduces
#' to a regular Dirichlet distribution as described by
#' \insertCite{dtree_evoteid;textual}{dirtree.elections}.
#'
#' @docType class
#'
#' @import methods
#'
#' @return
#' A Dirichlet Tree representing partial IRV ballots, as an object of class
#' \code{dirichlet_tree}.
#'
#' @references
#' \insertAllCited{}
#'
#' @export
dirtree <- function(
  candidates,
  min_depth = 0,
  max_depth = length(candidates),
  a0 = 1.,
  vd = FALSE
) {
  # Ensure n_candidates > 1
  if (class(candidates) != "character") {
    stop(paste0("`candidates` must be a character vector, with each element",
                " representing a single candidate."))
  }
  if (length(unique(candidates)) != length(candidates)) {
    stop("All `candidates` must be unique.")
  }
  # Ensure 0 <= min_depth <= max_depth <= n_candidates.
  if (!(
    min_depth >= 0
    && max_depth >= min_depth
    && length(candidates) >= max_depth
  )) {
    stop(paste0("min_depth and max_depth must satisfy: ",
                "0 <= min_depth <= max_depth <= n_candidates"))
  }
  # Ensure a0 >= 0
  if (a0 < 0) {
    stop("`a0` must be >= 0.")
  }
  # Ensure vd is a logical
  if (!is.logical(vd)) {
    stop("`vd` must be a logical.")
  }
  # Return Dirichlet Tree
  return(new(
    RDirichletTree,
    candidates = candidates,
    min_depth = min_depth,
    max_depth = max_depth,
    a0 = a0,
    vd = vd,
    seed = gseed()
  ))
}

#' @name sample_predictive
#'
#' @title
#' Draw PIRV ballots from the posterior predictive distribution.
#'
#' @description
#' \code{sample_predictive} draws ballots from a multinomial distribution with
#' probabilities obtained from a single realization of the Dirichlet-Tree
#' posterior on the ranked ballots. See
#' \insertCite{dtree_evoteid;textual}{dirtree.elections} for details.
#'
#' @param dtree
#' A PIRV Dirichlet Tree object.
#'
#' @param n_ballots
#' An integer representing the number of ballots to draw.
#'
#' @return A \code{ranked_ballots} object containing \code{n_ballots} ballots
#' drawn from a single realisation of the posterior Dirichlet-Tree.
#'
#' @references
#' \insertAllCited{}
#'
#' @export
sample_predictive <- function(dtree, n_ballots) {
  stopifnot(class(dtree) %in% .dtree_classes)
  # Ensure n_ballots > 0.
  if (n_ballots <= 0 || !is.numeric(n_ballots)) {
    stop("n_ballots must be an integer > 0")
  }
  ballots <- dtree$sample_predictive(as.integer(n_ballots), gseed())
  class(ballots) <- "ranked_ballots"
  attr(ballots, "candidates") <- dtree$candidates
  return(ballots)
}

#' @name sample_posterior
#'
#' @title
#' Draw election outcomes from the posterior distribution.
#'
#' @description
#' \code{sample_posterior} draws sets of ballots from independent realizations
#' of the Dirichlet Tree posterior, then determines the probability for each
#' candidate being elected by aggregating the results of the social choice
#' function. See \insertCite{dtree_evoteid;textual}{dirtree.elections} for
#' details.
#'
#' @param dtree
#' A \code{dirichlet_tree} object.
#'
#' @param n_elections
#' An integer representing the number of elections to generate. A higher
#' number yields higher precision in the output probabilities.
#'
#' @param n_ballots
#' An integer representing the total number of ballots cast in the election.
#'
#' @param n_winners
#' The number of candidates elected in each election.
#'
#' @return A numeric vector containing the probabilities for each candidate
#' being elected.
#'
#' @references
#' \insertAllCited{}
#'
#' @export
sample_posterior <- function(dtree, n_elections, n_ballots, n_winners = 1) {
  stopifnot(class(dtree) %in% .dtree_classes)
  return(
    dtree$sample_posterior(
      nElections = n_elections,
      nBallots = n_ballots,
      nWinners = n_winners,
      nBatches = n_elections / 2,
      gseed()
    )
  )
}

#' @name update
#'
#' @title
#' Update a \code{dirichlet_tree} model by observing some ranked ballots.
#'
#' @description
#' \code{update} updates a Dirichlet-Tree model with observations to obtain
#' a posterior distribution on the ranked ballots. See
#' \insertCite{dtree_evoteid;textual}{dirtree.elections} for implementation
#' details.
#'
#' @param object A PIRV Dirichlet Tree object.
#'
#' @param ballots a list of PIRV ballots.
#'
#' @param \\dots Unused.
#'
#' @references
#' \insertAllCited{}
#'
#' @export
update.Rcpp_RDirichletTree <- function(object, ballots, ...) {
  stopifnot(class(object) %in% .dtree_classes)
  stopifnot(class(ballots) %in% .ballot_types)
  object$update(ballots)
}

#' @name reset
#'
#' @title
#' Clear the internal state of a PIRV Dirichlet Tree
#'
#' @description
#' Destroy the Tree's internal state and revert back to the prior.
#'
#' @param dtree
#' A \code{dirichlet_tree} object.
#'
#' @export
reset <- function(dtree) {
  stopifnot(class(dtree) %in% .dtree_classes)
  dtree$reset()
}

# Helper function to get a random seed string to pass to CPP methods
gseed <- function() {
  return(paste(sample(LETTERS, 10), collapse = ""))
}

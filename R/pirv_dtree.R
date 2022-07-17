#' @name dirtree.pirv
#' @aliases PIRVDirichletTree
#' @title Dirichlet Tree for Partial IRV ballots
#' @description \code{dirtree.pirv} is used to create a Dirichlet Tree for modelling partially ordered IRV ballots.
#' @keywords dirichlet tree irv election ballot
#' @section References:
#' \itemize{
#'   \item F. Everest, M. Blom, P.B. Stark, P.J. Stuckey, V. Teague and D. Vukcevic. (2022). Auditing Ranked Voting Elections with Dirichlet-Tree Models: First Steps.
#' }
#' @param candidates A character vector, with each element (must be unique) representing a single candidate.
#' @param minDepth the minimum number of candidates which must be specified for a valid ballot.
#' @param maxDepth the maximum number of candidates which can be specified for a valid ballot.
#' @param a0 the prior parameter for the distribution.
#' @param vd a boolean value representing whether or not the prior should reduce to a vanilla Dirichlet distribution.
#' @docType class
#' @author Floyd Everest
#' @import methods
#' @return A Dirichlet Tree representing partial IRV ballots, as an Rcpp module of class `PIRVDirichletTree`.
#' @export
dirtree.pirv <- function(candidates, minDepth = 0, maxDepth = length(candidates), a0 = 1., vd = FALSE) {
  # Ensure nCandidates > 1
  if (class(candidates) != "character") {
    stop("`candidates` must be a character vector, with each element representing a single candidate.")
  }
  if (length(unique(candidates)) != length(candidates)) {
    stop("All `candidates` must be unique.")
  }
  # Ensure 0 <= minDepth <= maxDepth <= nCandidates.
  if (!(
    minDepth >= 0
    && maxDepth >= minDepth
    && length(candidates) >= maxDepth
  )) {
    stop("minDepth and maxDepth must satisfy: 0 <= minDepth <= maxDepth <= nCandidates")
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
    PIRVDirichletTree,
    candidates = candidates,
    minDepth = minDepth,
    maxDepth = maxDepth,
    a0 = a0,
    vd = vd,
    seed = gseed()
  ))
}

#' @name samplePredictive
#' @title Draw PIRV ballots from the posterior predictive distribution.
#' @description \code{samplePredictive} draws ballots from a multinomial distribution with probabilities obtained from a single realization of the Dirichlet Tree posterior.
#' @section References:
#' \itemize{
#'   \item F. Everest, M. Blom, P.B. Stark, P.J. Stuckey, V. Teague and D. Vukcevic. (2022). Auditing Ranked Voting Elections with Dirichlet-Tree Models: First Steps.
#' }
#' @param dtree a PIRV Dirichlet Tree object.
#' @param nBallots an integer representing the number of ballots to draw.
#' @return A list with each element corresponding to a drawn ballot.
#' @export
samplePredictive <- function(dtree, nBallots) {
  stopifnot(class(dtree) %in% .dtree_classes)
  # Ensure nBallots > 0.
  if (nBallots <= 0 || !is.numeric(nBallots)) {
    stop("nBallots must be an integer > 0")
  }
  ballots <- dtree$samplePredictive(as.integer(nBallots), gseed())
  class(ballots) <- "PIRVBallots"
  attr(ballots, "candidates") <- dtree$candidates
  return(ballots)
}

#' @name samplePosterior
#' @title Draw election outcomes from the posterior distribution.
#' @description \code{samplePosterior} draws sets of ballots from independent realizations of the Dirichlet Tree posterior, then determines the probability for each candidate being elected by aggregating the results of the social choice function.
#' @section References:
#' \itemize{
#'   \item F. Everest, M. Blom, P.B. Stark, P.J. Stuckey, V. Teague and D. Vukcevic. (2022). Auditing Ranked Voting Elections with Dirichlet-Tree Models: First Steps.
#' }
#' @param dtree A PIRV Dirichlet Tree object.
#' @param nElections An integer representing the number of elections to generate. A higher number yields higher precision in the output probabilities.
#' @param nBallots An integer representing the number of ballots cast in total for each election.
#' @param nWinners The number of candidates elected in each election.
#' @return A NumericVector containing the probabilities for each candidate being elected.
#' @export
samplePosterior <- function(dtree, nElections, nBallots, nWinners = 1) {
  stopifnot(class(dtree) %in% .dtree_classes)
  return(
    dtree$samplePosterior(
      nElections = nElections,
      nBallots = nBallots,
      nWinners = nWinners,
      nBatches = nElections / 2,
      gseed()
    )
  )
}

#' @name update
#' @title Update a Dirichlet Tree with PIRV ballot data.
#' @description \code{update} updates a Dirichlet Tree with observations to obtain a posterior distribution.
#' @section References:
#' \itemize{
#'   \item F. Everest, M. Blom, P.B. Stark, P.J. Stuckey, V. Teague and D. Vukcevic. (2022). Auditing Ranked Voting Elections with Dirichlet-Tree Models: First Steps.
#' }
#' @param object A PIRV Dirichlet Tree object.
#' @param ballots a list of PIRV ballots.
#' @param \\dots Unused.
#' @export
update.Rcpp_PIRVDirichletTree <- function(object, ballots, ...) {
  stopifnot(class(object) %in% .dtree_classes)
  stopifnot(class(ballots) %in% .ballot.types)
  object$update(ballots)
}

#' @name reset
#' @title Clear the internal state of a PIRV Dirichlet Tree
#' @description Destroys the Tree's internal state to revert it back to the prior.
#' @param dtree a PIRV Dirichlet Tree object.
#' @export
reset <- function(dtree) {
  stopifnot(class(dtree) %in% .dtree_classes)
  dtree$reset()
}

# Helper function to get a random seed string to pass to CPP methods
gseed <- function() {
  return(paste(sample(LETTERS, 10), collapse = ""))
}

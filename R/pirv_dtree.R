#' @name dirtree.pirv
#' @aliases PIRVDirichletTree
#' @title Dirichlet Tree for Partial IRV ballots
#' @description A Dirichlet Tree for modelling partially ordered IRV ballots.
#' @param candidates A character vector, with each element (must be unique) representing a single candidate.
#' @param minDepth the minimum number of candidates to be specified for a valid ballot.
#' @param alpha0 the prior parameter for the distribution.
#' @param vd a boolean value representing whether or not the prior should reduce to a vanilla Dirichlet distribution.
#' @docType class
#' @author Floyd Everest
#' @import methods
#' @return A Dirichlet Tree representing partial IRV ballots, as an Rcpp module of class `PIRVDirichletTree`.
#' @export
dirtree.pirv <- function(candidates, minDepth = 0, alpha0 = 1., vd = FALSE) {
  # Ensure nCandidates > 1
  if (class(candidates) != "character") {
    stop("`candidates` must be a character vector, with each element representing a single candidate.")
  }
  if (length(unique(candidates)) != length(candidates)) {
    stop("All `candidates` must be unique.")
  }
  # Ensure 0 <= minDepth <= nCandidates.
  if (minDepth > length(candidates) || minDepth < 0) {
    stop("`minDepth` must be >= 0 and <= length(candidates).")
  }
  # Ensure alpha0 >= 0
  if (alpha0 < 0) {
    stop("`alpha0` must be >= 0.")
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
    alpha0 = alpha0,
    vd= vd,
    seed = gseed()
  ))
}

#' @name samplePredictive
#' @title Draw PIRV ballots from the posterior predictive distribution.
#' @description Draws ballots from a single realization of the Dirichlet Tree posterior.
#' @param dtree a PIRV Dirichlet Tree object.
#' @param nBallots an integer representing the number of ballots to draw.
#' @return A list with each element corresponding to a drawn ballot.
#' @export
samplePredictive <- function(dtree, nBallots) {
  stopifnot(class(dtree) %in% .dtree_classes)
  # Ensure nBallots > 0.
  if (nBallots <=0 || !is.numeric(nBallots)) {
    stop("nBallots must be an integer > 0")
  }
  ballots = dtree$samplePredictive(as.integer(nBallots), gseed())
  class(ballots) <- "PIRVBallots"
  attr(ballots, "candidates") <- dtree$candidates
  return(ballots)
}

#' @name samplePosterior
#' @title Draw election outcomes from the posterior distribution.
#' @description Draws ballots from a realizations of the Dirichlet Tree posterior, and determines the probability for each candidate being elected by aggregating the results.
#' @param dtree A PIRV Dirichlet Tree object.
#' @param nElections An integer representing the number of elections to generate. A higher number yields higher precision in the output probabilities.
#' @param nBallots An integer representing the number of ballots cast in total for each election.
#' @return A NumericVector containing the probabilities for each candidate being elected.
#' @export
samplePosterior <- function(dtree, nElections, nBallots, nWinners=1) {
  stopifnot(class(dtree) %in% .dtree_classes)
  return(
    dtree$samplePosterior(
      nElections=nElections,
      nBallots=nBallots,
      nWinners=nWinners,
      nBatches=nElections/2,
      gseed()
    )
  )
}

#' @name sampleMPP
#' @title Sample marginal posterior probabilities for a ballot.
#' @description Draws marginal probabilities for observing a given ballot under the posterior distribution.
#' @param dtree A PIRV Dirichlet Tree object.
#' @param n The number of samples to draw from the posterior.
#' @param ballot The ballot to sample posterior probabilities for.
#' @return A NumericVector of \code{n} probabilities, each corresponding to a probability of observing \code{b} under an independent realisation of the posterior distribution.
#' @export
sampleMPP <- function(dtree, n, ballot) {
  stopifnot(class(dtree) %in% .dtree_classes)
  return(dtree$sampleMarginalProbability(n, ballot, gseed()))
}

#' @name update
#' @title Update a Dirichlet Tree with PIRV ballot data.
#' @description Updates a Dirichlet Tree with observed ballots to obtain a new posterior.
#' @param dtree a PIRV Dirichlet Tree object.
#' @param ballots a list of PIRV ballots.
#' @export
update.Rcpp_PIRVDirichletTree <- function(dtree, ballots) {
  stopifnot(class(dtree) %in% .dtree_classes)
  # TODO: Throw a warning?
  #stopifnot(class(ballots) == 'PIRVBallots')
  dtree$update(ballots)
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
  return( paste(sample(LETTERS, 10), collapse="") )
}

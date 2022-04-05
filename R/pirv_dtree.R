#' @name dirtree.pirv
#' @aliases PIRVDirichletTree
#' @title Dirichlet Tree for Partial IRV ballots
#' @description A Dirichlet Tree for modelling partially ordered IRV ballots.
#' @param candidates A character vector, with each element (must be unique) representing a single candidate.
#' @param minDepth the minimum number of candidates to be specified for a valid ballot.
#' @param alpha0 the prior parameter for the distribution.
#' @docType class
#'
#' @author Floyd Everest
#' @import methods
#' @return A Dirichlet Tree representing partial IRV ballots, as an Rcpp module of class `PIRVDirichletTree`.
#' @export
dirtree.pirv <- function(candidates, minDepth, alpha0 = 1.) {
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
    stop("`alpha0` must be >=0")
  }
  # Return Dirichlet Tree
  return(new(
    PIRVDirichletTree,
    candidates = candidates,
    minDepth = minDepth,
    alpha0 = alpha0,
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
  return(ballots)
}

#' @name update
#' @title Update a Dirichlet Tree with PIRV ballot data.
#' @description Updates a Dirichlet Tree with observed ballots to obtain a new posterior.
#' @param dtree a PIRV Dirichlet Tree object.
#' @param ballots a list of PIRV ballots.
#' @export
update.Rcpp_PIRVDirichletTree <- function(dtree, ballots) {
  stopifnot(class(dtree) %in% .dtree_classes)
  stopifnot(class(ballots) == 'PIRVBallots')
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

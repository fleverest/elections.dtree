#' @name dirtree.irv
#' @aliases DirichletTreeIRV
#' @title Dirichlet Tree for IRV
#' @description A Dirichlet Tree structure for IRV ballot types.
#' @param nCandidates the number of candidates in the IRV election.
#' @param scale the prior scale of the distribution.
#' @param dirichlet a boolean indicating whether or not the prior is to be
#'        a regular Dirichlet distribution as opposed to a Dirichlet Tree.
#' @param seed a string seeding the initial random state of the distribution.
#' @docType class
#'
#' @author Floyd Everest
#' @import methods
#' @return A Dirichlet Tree representing IRV ballots, as an Rcpp module of class `DirichletTreeIRV`.
#' @export
dirtree.irv <- function(nCandidates, scale = 1., dirichlet = F, seed = NULL) {
  # If seed is null, seed from R's internal random state:
  if (is.null(seed)) {
    seed <- "default_seed"
  }
  # Ensure nCandidates > 1
  if (nCandidates <= 1) {
    stop("nCandidates must be >1.")
  }
  # Ensure scale > 0
  if (scale <=0) {
    stop ("scale must be >0")
  }
  # Return Dirichlet Tree
  return(new(
    DirichletTreeIRV,
    nCandidates = nCandidates,
    scale = scale,
    treeType = dirichlet,
    seed = seed
  ))
}

# clear

#' @name clear
#' @title Clear the internal state of a Dirichlet Tree
#' @description Destroys the Tree's internal state to revert it back to the original prior.
#' @param dtree a Dirichlet Tree object.
#' @export
clear <- function(dtree) {
  stopifnot(class(dtree) %in% .dtree_classes)
  dtree$clear()
}

# draw

#' @name draw
#' @title Draw ballots from the posterior distribution.
#' @description Draws ballots from one realization of the posterior distribution.
#' @param dtree a Dirichlet Tree object.
#' @param nBallots an integer representing the number of ballots to draw.
#' @return A dataframe with rows corresponding to the drawn ballots.
#' @export
draw <- function(dtree, nBallots) {
  stopifnot(class(dtree) %in% .dtree_classes)
  # Ensure nBallots > 0.
  if (nBallots <=0) {
    stop("nBallots must be an integer > 0")
  }
  return(dtree$sampleBallots(as.integer(nBallots)))
}

# update

#' @name update
#' @title Update a Dirichlet Tree with ballot data.
#' @description Updates a Dirichlet Tree with observed ballots to obtain a new posterior.
#' @param dtree a Dirichlet Tree object.
#' @param ballots a dataframe with rows representing the ballots to observe.
#' @export
update.Rcpp_DirichletTreeIRV <- function(dtree, ballots){
  stopifnot(class(dtree) %in% .dtree_classes)
  stopifnot(class(ballots) == 'data.frame')
  dtree$update(ballots)
}

# samplePosterior

#' @name samplePosterior
#' @title Sample from the posterior distribution on candidate victory outcomes.
#' @description Samples elections from the posterior distribution and determines the outcomes.
#' @param dtree a Dirichlet Tree object.
#' @param nElections the number of election outcomes to determine.
#' @param nBallots the number of ballots per election.
#' @param useObserved a logical indicating whether or not the already observed ballots should
#'        be taken into account to determine outcomes. If false, we resample all ballots.
#' @param nBatches a performance metric which determines how many batches the elections should
#'        be split into for tuning multithreading performance.
#' @return A dataframe with rows corresponding to the drawn ballots.
#' @export
samplePosterior <- function(dtree, nElections, nBallots, useObserved = T, nBatches = nElections/2) {
  stopifnot(class(dtree) %in% .dtree_classes)
  # Ensure nElections is > 0
  if (nElections <= 0){
    stop("nElections must be an integer > 0")
  }
  # Ensure nBallots is > 0
  if (nBallots <= 0){
    stop("nBallots must be an integer > 0")
  }
  # Ensure useObserved is a logical
  if (!is.logical(useObserved)){
    stop("useObserved must be a logical")
  }
  # Ensure nBatches < nElections.
  if (nBatches > nElections) {
    print(paste("Warning: nBatches (", nBatches,") > nElections (", nElections, "). Setting nBatches to ",nElections/2, ".", sep=""))
    nBatches <- nElections/2
  }
  result <- dtree$samplePosterior(as.integer(nElections),as.integer(nBallots),useObserved,as.integer(nBatches))
  return(result)
}

# evaluateElection

#' @name evaluate.election
#' @title Evaluates the outcome of an election
#' @description Given a DataFrame input representing the ballots of an
#'              election, outputs the winner.
#' @param ballots a DataFrame representing the complete set of election ballots>
#' @return an integer representing the index of the victorious candidate.
#' @export
evaluate.election <- function(ballots) {
  return(evaluateElection(ballots))
}

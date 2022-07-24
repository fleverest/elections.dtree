#' Create a Dirichlet-Tree for modelling ranked ballots
#'
#' @description
#' A \code{dirichlet_tree} object represents a Dirichlet-Tree distribution
#' on ballots. By specifying the tree structure for the ranked ballots,
#' the Dirichlet Tree is initialized as the prior as described by
#' \insertCite{dtree_evoteid;textual}{dirtree.elections}. There are
#' methods available for observing data to obtain a posterior distribution
#' along with methods to sample election outcomes and sets of ballots from
#' the posterior predictive distribution.
#'
#' @param candidates
#' A character vector, with each element (must be unique) representing a
#' single candidate.
#'
#' @param min_depth
#' The minimum number of candidates which must be specified for a valid
#' ballot in the election.
#'
#' @param max_depth
#' The maximum number of candidates which can be specified for a valid
#' ballot in the election.
#'
#' @param a0
#' The prior parameter for the distribution.
#'
#' @param vd
#' A flag which, when \code{TRUE}, employs a parameter structure which
#' reduces to a regular Dirichlet distribution as described by
#' \insertCite{dtree_evoteid;textual}{dirtree.elections}.
#'
#' @usage dtree <- dirichlet_tree$new(candidates = LETTERS)
#'
#' @keywords dirichlet tree irv election ballot
#'
#' @format An \code{\link{R6Class}} generator object.
#'
#' @docType class
#'
#' @importFrom R6 R6Class
#'
#' @references
#' \insertAllCited{}
#'
#' @export
dirichlet_tree <- R6::R6Class("dirichlet_tree",
  class = TRUE,
  cloneable = FALSE,

  private = list(
    .Rcpp_tree = NULL
  ),

  active = list(
    #' @field a0
    #' Gets or sets the \code{a0} parameter for the Dirichlet-Tree.
    a0 = function(a0) {
      if (missing(a0)) {
        return(private$.Rcpp_tree$a0)
      } else {
        if (!is.numeric(a0) || a0 < 0)
          stop("The `a0` parameter must be a numeric >= 0.")
        private$.Rcpp_tree$a0 <- a0
        invisible(self)
      }
    },

    #' @field min_depth
    #' Gets or sets the \code{min_depth} parameter for the Dirichlet-Tree.
    min_depth = function(min_depth) {
      if (missing(min_depth)) {
        return(private$.Rcpp_tree$min_depth)
      } else {
        if (!is.numeric(min_depth) || min_depth < 0)
          stop("`min_depth` must be an integer >= 0.")
        if (min_depth > private$.Rcpp_tree$max_depth)
          stop("`min_depth` must be <= `max_depth`.")
        private$.Rcpp_tree$min_depth <- min_depth
        invisible(self)
      }
    },

    #' @field max_depth
    #' Gets or sets the \code{max_depth} parameter for the
    #' Dirichlet-Tree.
    max_depth = function(max_depth) {
      if (missing(max_depth)) {
        return(private$.Rcpp_tree$max_depth)
      } else {
        if (!is.numeric(max_depth)
          || max_depth < 0
          || max_depth >= length(private$.Rcpp_tree$candidates)
        )
          stop("`max_depth` must be an integer >= 0 and <= #candidates.")
        if (max_depth < private$.Rcpp_tree$min_depth)
          stop("`max_depth` must be >= `min_depth`.")
        private$.Rcpp_tree$max_depth <- max_depth
        invisible(self)
      }
    },

    #' @field vd
    #' Gets or sets the \code{vd} parameter for the Dirichlet-Tree.
    vd = function(vd) {
      if (missing(vd)) {
        return(private$.Rcpp_tree$vd)
      } else {
        if (!is.logical(vd))
          stop("`vd` must be a logical value.")
        private$.Rcpp_tree$vd <- vd
        invisible(self)
      }
    }
  ),

  public = list(
    #' @description
    #' Create a new \code{dirichlet_tree} prior distribution with the specified
    #' tree structure. See \insertCite{dtree_evoteid;textual}{dirtree.elections}
    #' for further details.
    #'
    #' @examples
    #' dtree <- dirichlet_tree$new(candidates = LETTERS, a0 = 1., min_depth = 1)
    #'
    #' @return
    #' A new \code{dirichlet_tree} prior.
    initialize = function(
      candidates,
      min_depth = 0,
      max_depth = length(candidates) - 1,
      a0 = 1.,
      vd = FALSE
    ) {
      # Ensure n_candidates > 1
      if (class(candidates) != "character") {
        stop(paste0("`candidates` must be a character vector, with each",
                    " element representing a single candidate."))
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
      private$.Rcpp_tree <- new(
        RDirichletTree,
        candidates = candidates,
        min_depth = min_depth,
        max_depth = max_depth,
        a0 = a0,
        vd = vd,
        seed = gseed()
      )
      invisible(self)
    },

    #' @description
    #' Updates the \code{dirichlet_tree} object with observations of ballots.
    #' This updates the parameter structure of the tree to yield the posterior
    #' Dirichlet-Tree, as described in
    #' \insertCite{dtree_evoteid;textual}{dirtree.elections}.
    #'
    #' @param ballots
    #' A set of ballots to observe - must be of class \code{ranked_ballots}.
    #'
    #' @usage
    #' dtree$update(ballots)
    #'
    #' @examples
    #' dirichlet_tree$new(
    #'   candidates = LETTERS
    #' )$update(
    #'   ranked_ballots(c("A", "B", "C"))
    #' )
    #'
    #' @return The \code{dirichlet_tree} object.
    update = function(ballots) {
      if (!any(class(ballots) %in% .ballot_types))
        stop("`ballots` must be an object of class `ranked_ballots`.")
      private$.Rcpp_tree$update(ballots = ballots)
      invisible(self)
    },

    #' @description
    #' Resets the \code{dirichlet_tree} observations to revert the
    #' parameter structure back to the originally specified prior.
    #'
    #' @usage
    #' dtree$reset()
    #'
    #' @examples
    #' dirichlet_tree$new(
    #'   candidates = LETTERS
    #' )$update(
    #'   ranked_ballots(c("A", "B", "C"))
    #' )$reset()
    #'
    #' @return The \code{dirichlet_tree} object.
    reset = function() {
      private$.Rcpp_tree$reset()
      invisible(self)
    },

    #' @description
    #' Draws sets of ballots from independent realizations of the Dirichlet-Tree
    #' posterior, then determines the probability for each candidate being
    #' elected by aggregating the results of the social choice function. See
    #' \insertCite{dtree_evoteid;textual}{dirtree.elections} for details.
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
    #' @usage
    #' dtree$sample_posterior(n_elections = 100, n_ballots = 1000)
    #'
    #' @examples
    #' dirichlet_tree$new(
    #'   candidates = LETTERS,
    #'   a0 = 1.,
    #'   min_depth = 3,
    #'   max_depth = 6,
    #'   vd = FALSE
    #' )$update(
    #'   ranked_ballots(c("A","B","C"))
    #' )$sample_posterior(
    #'   n_elections = 100,
    #'   n_ballots = 1000
    #' )
    #'
    #' @return A numeric vector containing the probabilities for each candidate
    #' being elected.
    sample_posterior = function(n_elections, n_ballots, n_winners = 1) {
      private$.Rcpp_tree$sample_posterior(
        nElections = n_elections,
        nBallots = n_ballots,
        nWinners = n_winners,
        nBatches = n_elections / 2,
        gseed()
      )
    },

    #' @description
    #' \code{sample_predictive} draws ballots from a multinomial distribution
    #' with ballot probabilities obtained from a single realization of the
    #' Dirichlet-Tree posterior on the ranked ballots. See
    #' \insertCite{dtree_evoteid;textual}{dirtree.elections} for details.
    #'
    #' @param n_ballots
    #' An integer representing the number of ballots to draw per election.
    #'
    #' @param n_elections
    #' An integer representing the number of elections to draw.
    #'
    #' @usage
    #' dtree$sample_predictive(n_ballots = 10)
    #'
    #' @examples
    #' dirichlet_tree$new(
    #'   candidates = LETTERS,
    #'   a0 = 1.,
    #'   min_depth = 3,
    #'   max_depth = 6,
    #'   vd = FALSE
    #' )$update(
    #'   ranked_ballots(c("A","B","C"))
    #' )$sample_predictive(
    #'   n_ballots = 10
    #' )
    #'
    #' @return A \code{ranked_ballots} object containing \code{n_ballots}
    #' ballots drawn from a single realisation of the posterior Dirichlet-Tree.
    sample_predictive = function(n_ballots) {
      # Ensure n_ballots > 0.
      if (n_ballots <= 0 || !is.numeric(n_ballots))
        stop("n_ballots must be an integer > 0")
      ballots <- private$.Rcpp_tree$sample_predictive(
                              as.integer(n_ballots), gseed())
      class(ballots) <- "ranked_ballots"
      attr(ballots, "candidates") <- private$.Rcpp_tree$candidates
      return(ballots)
    }
  )
)

#' @name dirtree
#'
#' @aliases dtree
#'
#' @title
#' Create a Dirichlet Tree object
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
  dirichlet_tree$new(
    candidates = candidates,
    min_depth = min_depth,
    max_depth = max_depth,
    a0 = a0,
    vd = vd
  )
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
  stopifnot(any(class(dtree) %in% .dtree_classes))
  # Ensure n_ballots > 0.
  return(dtree$sample_predictive(n_ballots))
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
  stopifnot(any(class(dtree) %in% .dtree_classes))
  return(
    dtree$sample_posterior(
      n_elections = n_elections,
      n_ballots = n_ballots,
      n_winners = n_winners
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
#' @return
#' The \code{dirichlet_tree} object.
#'
#' @references
#' \insertAllCited{}
#'
#' @export
update.dirichlet_tree <- function(object, ballots, ...) {
  stopifnot(any((class(object) %in% .dtree_classes)))
  stopifnot(any(class(ballots) %in% .ballot_types))
  return(object$update(ballots = ballots))
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
#' @return
#' The \code{dirichlet_tree} object.
#'
#' @export
reset <- function(dtree) {
  stopifnot(any(class(dtree) %in% .dtree_classes))
  return(dtree$reset())
}

# Helper function to get a random seed string to pass to CPP methods
gseed <- function() {
  return(paste(sample(LETTERS, 10), collapse = ""))
}

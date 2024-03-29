# nolint start

#' Create a Dirichlet-tree for modelling ranked ballots
#'
#' @description
#' A \code{dirichlet_tree} object represents a Dirichlet-tree distribution
#' on ballots. By specifying the tree structure for the ranked ballots,
#' the Dirichlet-tree is initialized with the same prior structure described by
#' \insertCite{dtree_evoteid;textual}{elections.dtree}. There are
#' methods provided for observing data (to obtain a posterior distribution)
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
#' \insertCite{dtree_evoteid;textual}{elections.dtree}.
#'
#' @param ballots
#' A set of ballots of class `prefio::preferences` or
#' `prefio::aggregated_preferences` to observe. The ballots should not contain
#' any ties, but they may be incomplete.
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
#' @param replace
#' A boolean indicating whether or not we should replace our sample in the
#' monte-carlo step, drawing the full set of election ballots from the posterior
#'
#' @param n_threads
#' The maximum number of threads for the process. The default value of
#' \code{NULL} will default to 2 threads. \code{Inf} will default to the maximum
#' available, and any value greater than or equal to the maximum available will
#' result in the maximum available.
#'
#' @keywords dirichlet tree dirichlet-tree irv election ballot
#'
#' @format An \code{\link{R6Class}} generator object.
#'
#' @docType class
#'
#' @importFrom R6 R6Class
#' @importFrom prefio preferences
#'
#' @references
#' \insertRef{dtree_eis}{elections.dtree}.
#'
#' \insertRef{dtree_evoteid}{elections.dtree}.
#'
#' @export
dirichlet_tree <- R6::R6Class("dirichlet_tree",
  class = TRUE,
  cloneable = FALSE,
  private = list(
    .Rcpp_tree = NULL,
    observations = NULL
  ),
  active = list(
    #' @field a0
    #' Gets or sets the \code{a0} parameter for the Dirichlet-tree.
    a0 = function(a0) {
      if (missing(a0)) {
        return(private$.Rcpp_tree$a0)
      } else {
        if (!is.numeric(a0) || a0 < 0) {
          stop("The `a0` parameter must be a numeric >= 0.")
        }
        private$.Rcpp_tree$a0 <- a0
        invisible(self)
      }
    },

    #' @field min_depth
    #' Gets or sets the \code{min_depth} parameter for the Dirichlet-tree.
    min_depth = function(min_depth) {
      if (missing(min_depth)) {
        return(private$.Rcpp_tree$min_depth)
      } else {
        if (!is.numeric(min_depth) || min_depth < 0) {
          stop("`min_depth` must be an integer >= 0.")
        }
        if (min_depth > private$.Rcpp_tree$max_depth) {
          stop("`min_depth` must be <= `max_depth`.")
        }
        private$.Rcpp_tree$min_depth <- min_depth
        invisible(self)
      }
    },

    #' @field max_depth
    #' Gets or sets the \code{max_depth} parameter for the
    #' Dirichlet-tree.
    max_depth = function(max_depth) {
      if (missing(max_depth)) {
        return(private$.Rcpp_tree$max_depth)
      } else {
        if (!is.numeric(max_depth) ||
          max_depth < 0 ||
          max_depth >= length(private$.Rcpp_tree$candidates)
        ) {
          stop("`max_depth` must be an integer >= 0 and <= #candidates.")
        }
        if (max_depth < private$.Rcpp_tree$min_depth) {
          stop("`max_depth` must be >= `min_depth`.")
        }
        private$.Rcpp_tree$max_depth <- max_depth
        invisible(self)
      }
    },

    #' @field vd
    #' Gets or sets the \code{vd} parameter for the Dirichlet-tree.
    vd = function(vd) {
      if (missing(vd)) {
        return(private$.Rcpp_tree$vd)
      } else {
        if (!is.logical(vd)) {
          stop("`vd` must be a logical value.")
        }
        private$.Rcpp_tree$vd <- vd
        invisible(self)
      }
    }
  ),
  public = list(
    #' @description
    #' Create a new \code{dirichlet_tree} prior distribution with the specified
    #' tree structure. See \insertCite{dtree_evoteid;textual}{elections.dtree}
    #' for further details.
    #'
    #' @examples
    #' dtree <- dirichlet_tree$new(candidates = LETTERS, a0 = 1., min_depth = 1)
    #'
    #' @return
    #' A new \code{dirichlet_tree} prior.
    initialize = function(candidates,
                          min_depth = 0,
                          max_depth = length(candidates) - 1,
                          a0 = 1.,
                          vd = FALSE) {
      # Ensure n_candidates > 1
      if (!is(candidates, "character")) {
        stop(paste0(
          "`candidates` must be a character vector, with each",
          " element representing a single candidate."
        ))
      }
      if (length(unique(candidates)) != length(candidates)) {
        stop("All `candidates` must be unique.")
      }
      # Ensure 0 <= min_depth <= max_depth <= n_candidates.
      if (!(
        min_depth >= 0 &&
          max_depth >= min_depth &&
          length(candidates) >= max_depth
      )) {
        stop(paste0(
          "min_depth and max_depth must satisfy: ",
          "0 <= min_depth <= max_depth <= n_candidates"
        ))
      }
      # Ensure a0 >= 0
      if (a0 < 0) {
        stop("`a0` must be >= 0.")
      }
      # Ensure vd is a logical
      if (!is.logical(vd)) {
        stop("`vd` must be a logical.")
      }
      # Set the observations attribute to an empty set.
      private$observations <- prefio::preferences(
        matrix(
          ncol = length(candidates),
          nrow = 0L
        ),
        format = "ranking",
        item_names = candidates,
        aggregate = TRUE
      )
      # Return Dirichlet-tree
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
    #' \code{print} shows some details of the distribution and its parameters.
    #'
    #' @return The \code{dirichlet_tree} object.
    print = function() {
      cat(
        "Dirichlet-tree (",
        "a0=", private$.Rcpp_tree$a0, ", ",
        "min_depth=", private$.Rcpp_tree$min_depth, ", ",
        "max_depth=", private$.Rcpp_tree$max_depth, ", ",
        "vd=", private$.Rcpp_tree$vd,
        ")\n",
        sep = ""
      )
      cat(
        "Candidates: ",
        paste(
          private$.Rcpp_tree$candidates,
          collapse = " "
        ),
        "\n",
        sep = ""
      )
      # Summarize observations
      cat("Observations:\n")
      print(private$observations, row.names = FALSE)
      # Return self
      invisible(self)
    },

    #' @description
    #' Updates the \code{dirichlet_tree} object with observations of ballots.
    #' This updates the parameter structure of the tree to yield the posterior
    #' Dirichlet-tree, as described in
    #' \insertCite{dtree_evoteid;textual}{elections.dtree}.
    #'
    #' @examples
    #' ballots <- prefio::preferences(
    #'   t(c(1, 2, 3)),
    #'   format = "ranking",
    #'   item_names = LETTERS[1:3]
    #' )
    #' dirichlet_tree$new(
    #'   candidates = LETTERS[1:3]
    #' )$update(ballots)
    #'
    #' @return The \code{dirichlet_tree} object.
    update = function(ballots) {
      if (!inherits(ballots, .ballot_types)) {
        stop(
          "`ballots` must be a `prefio::preferences` or",
          "`prefio::aggregated_preferences` object."
        )
      }
      if (inherits(ballots, "ranked_ballots")) {
        warning(
          "\"ranked_ballots\" is now deprecated and should be replaced ",
          "by \"prefio::preferences\" or ",
          "\"prefio::aggregated_preferences\"."
        )
        ballots <- prefio::preferences(
          as.data.frame(
            do.call(
              rbind,
              lapply(ballots, as.list)
            )
          ),
          format = "ordering",
          aggregate = TRUE
        )
      }
      if (inherits(ballots, "preferences") ||
        inherits(ballots, "aggregated_preferences")) {
        prefs <- prefio::as.preferences(ballots)

        if (!attr(prefs, "preftype") %in% c("soc", "soi")) {
          stop("`ballots` must not feature ties between candidates.")
        }

        bs <- lapply(
          seq_along(prefs),
          function(i) unlist(prefs[i, as.ordering = TRUE])
        )
      }
      private$.Rcpp_tree$update(ballots = bs)
      private$observations <- rbind(private$observations, aggregate(ballots))
      invisible(self)
    },

    #' @description
    #' Resets the \code{dirichlet_tree} observations to revert the
    #' parameter structure back to the originally specified prior.
    #'
    #' @examples
    #' ballots <- prefio::preferences(
    #'   t(c(1, 2, 3)),
    #'   format = "ranking",
    #'   item_names = LETTERS[1:3]
    #' )
    #' dtree <- dirichlet_tree$new(
    #'   candidates = LETTERS
    #' )$update(ballots)
    #' print(dtree)
    #' dtree$reset()
    #' print(dtree)
    #'
    #' @return The \code{dirichlet_tree} object.
    reset = function() {
      candidates <- private$.Rcpp_tree$candidates
      private$.Rcpp_tree$reset()
      private$observations <- prefio::preferences(
        matrix(
          ncol = length(candidates),
          nrow = 0L
        ),
        format = "ranking",
        item_names = candidates,
        aggregate = TRUE
      )
      invisible(self)
    },

    #' @description
    #' Draws sets of ballots from independent realizations of the Dirichlet-tree
    #' posterior, then determines the probability for each candidate being
    #' elected by aggregating the results of the social choice function. See
    #' \insertCite{dtree_evoteid;textual}{elections.dtree} for details.
    #'
    #' @examples
    #' ballots <- prefio::preferences(
    #'   t(c(1, 2, 3)),
    #'   format = "ranking",
    #'   item_names = LETTERS[1:3]
    #' )
    #' dirichlet_tree$new(
    #'   candidates = LETTERS,
    #'   a0 = 1.,
    #'   min_depth = 3,
    #'   max_depth = 6,
    #'   vd = FALSE
    #' )$update(
    #'   ballots
    #' )$sample_posterior(
    #'   n_elections = 10,
    #'   n_ballots = 10
    #' )
    #'
    #' @return A numeric vector containing the probabilities for each candidate
    #' being elected.
    sample_posterior = function(n_elections,
                                n_ballots,
                                n_winners = 1,
                                replace = FALSE,
                                n_threads = NULL) {
      if (n_elections <= 0) {
        stop("`n_elections` must be an integer > 0.")
      }
      if (n_ballots < length(private$observations) && !replace) {
        stop(paste0(
          "`n_ballots` must be an integer >= the number of ",
          "observed ballots unless sampling with replacement."
        ))
      }
      # Validate n_threads input
      if (is.null(n_threads)) {
        # NULL is mapped to the default of 2.
        n_threads <- 2
      }
      if (n_threads > parallel::detectCores()) {
        # Any value greater than the maximum available is set to the number of
        #  available cores.
        n_threads <- parallel::detectCores()
      }
      if (n_threads < 1) {
        # Invalid inputs raise an exception.
        stop("`n_threads` must be >= 1.")
      }
      private$.Rcpp_tree$sample_posterior(
        nElections = n_elections,
        nBallots = n_ballots,
        nWinners = n_winners,
        replace = replace,
        nThreads = n_threads,
        gseed()
      )
    },

    #' @description
    #' \code{sample_predictive} draws ballots from a multinomial distribution
    #' with ballot probabilities obtained from a single realization of the
    #' Dirichlet-tree posterior on the ranked ballots. See
    #' \insertCite{dtree_evoteid;textual}{elections.dtree} for details.
    #'
    #' @examples
    #' ballots <- prefio::preferences(
    #'   t(c(1, 2, 3)),
    #'   format = "ranking",
    #'   item_names = LETTERS[1:3]
    #' )
    #' dirichlet_tree$new(
    #'   candidates = LETTERS,
    #'   a0 = 1.,
    #'   min_depth = 3,
    #'   max_depth = 6,
    #'   vd = FALSE
    #' )$update(
    #'   ballots
    #' )$sample_predictive(
    #'   n_ballots = 10
    #' )
    #'
    #' @return A \code{prefio::preferences} object containing \code{n_ballots}
    #' ballots drawn from a single realisation of the posterior Dirichlet-tree.
    sample_predictive = function(n_ballots) {
      # Ensure n_ballots > 0.
      if (n_ballots <= 0 || !is.numeric(n_ballots)) {
        stop("n_ballots must be an integer > 0")
      }
      ballots <- private$.Rcpp_tree$sample_predictive(
        as.integer(n_ballots), gseed()
      )
      # Replace empty ballots with NA
      ballots <- lapply(ballots, function(x) if (length(x) == 0) NA else x)
      # Make length equal to number of candidates
      candidates <- private$.Rcpp_tree$candidates
      n_candidates <- length(candidates)
      # Coerce into a data frame in ordering format
      ballots <- as.data.frame(
        do.call(
          rbind,
          lapply(
            ballots,
            function(b) {
              b <- as.list(b)
              length(b) <- n_candidates
              b
            }
          )
        )
      )
      # Convert NAs back into blank ballots
      ballots <- rapply(
        ballots,
        function(x) if (is.na(x)) character() else x,
        how = "replace"
      )
      return(
        prefio::preferences(
          ballots,
          item_names = candidates,
          format = "ordering",
          aggregate = TRUE
        )
      )
    }
  )
)

# nolint end

#' @name dirtree
#'
#' @aliases dtree
#'
#' @title
#' Create a Dirichlet-tree object
#'
#' @description
#' \code{dirtree} is used to create a Dirichlet-tree for modelling ballots,
#' as described by \insertCite{dtree_evoteid;textual}{elections.dtree}.
#'
#' @keywords dirichlet tree dirichlet-tree irv election ballot
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
#' \insertCite{dtree_evoteid;textual}{elections.dtree}.
#'
#' @docType class
#'
#' @import methods
#'
#' @return
#' A Dirichlet-tree representing ranked ballots, as an object of class
#' \code{dirichlet_tree}.
#'
#' @references
#' \insertRef{dtree_eis}{elections.dtree}.
#'
#' \insertRef{dtree_evoteid}{elections.dtree}.
#'
#' @export
dirtree <- function(candidates,
                    min_depth = 0,
                    max_depth = length(candidates),
                    a0 = 1.,
                    vd = FALSE) {
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
#' Draw ballots from the posterior predictive distribution.
#'
#' @description
#' \code{sample_predictive} draws ballots from a multinomial distribution with
#' probabilities obtained from a single realization of the Dirichlet-tree
#' posterior on the ranked ballots. See
#' \insertCite{dtree_evoteid;textual}{elections.dtree} for details.
#'
#' @param dtree
#' A \code{dirichlet_tree} object.
#'
#' @param n_ballots
#' An integer representing the number of ballots to draw.
#'
#' @return A \code{prefio::preferences} object containing \code{n_ballots}
#' ballots drawn from a single realisation of the posterior Dirichlet-tree.
#'
#' @references
#' \insertRef{dtree_eis}{elections.dtree}.
#'
#' \insertRef{dtree_evoteid}{elections.dtree}.
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
#' of the Dirichlet-tree posterior, then determines the probability for each
#' candidate being elected by aggregating the results of the social choice
#' function. See \insertCite{dtree_evoteid;textual}{elections.dtree} for
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
#' @param replace
#' A boolean indicating whether or not we should re-use the observed ballots
#' in the monte-carlo integration step to determine the posterior probabilities.
#'
#' @param n_threads
#' The maximum number of threads for the process. The default value of
#' \code{NULL} will default to 2 threads. \code{Inf} will default to the maximum
#' available, and any value greater than or equal to the maximum available will
#' result in the maximum available.
#'
#' @return A numeric vector containing the probabilities for each candidate
#' being elected.
#'
#' @references
#' \insertRef{dtree_eis}{elections.dtree}.
#'
#' \insertRef{dtree_evoteid}{elections.dtree}.
#'
#' @export
sample_posterior <- function(dtree,
                             n_elections,
                             n_ballots,
                             n_winners = 1,
                             replace = FALSE,
                             n_threads = NULL) {
  stopifnot(any(class(dtree) %in% .dtree_classes))
  return(
    dtree$sample_posterior(
      n_elections = n_elections,
      n_ballots = n_ballots,
      n_winners = n_winners,
      replace = replace,
      n_threads = n_threads
    )
  )
}

#' @name update
#'
#' @title
#' Update a \code{dirichlet_tree} model by observing some ranked ballots.
#'
#' @description
#' \code{update} updates a Dirichlet-tree model with observations to obtain
#' a posterior distribution on the ranked ballots. See
#' \insertCite{dtree_evoteid;textual}{elections.dtree} for implementation
#' details.
#'
#' @param object A \code{dirichlet_tree} object.
#'
#' @param ballots A set of ballots - must be of type \code{prefio::preferences}.
#'
#' @param \\dots Unused.
#'
#' @return
#' The \code{dirichlet_tree} object.
#'
#' @references
#' \insertRef{dtree_eis}{elections.dtree}.
#'
#' \insertRef{dtree_evoteid}{elections.dtree}.
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
#' Clear the internal state of a \code{dirichlet_tree} object.
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

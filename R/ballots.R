# Helper ensures a set of ranked_ballots are all valid
validate_rankedballots <- function(ballots, candidates = NULL, ...) {
  for (b in ballots) {
    # No Repetitions
    if (length(b) != length(unique(b))) {
      stop(paste0(
        "Ballot ",
        paste(b, collapse = ","),
        " contains duplicate entries."
      ))
    }
    if (!is.null(candidates) && any(!b %in% candidates)) {
      stop(paste0(
        "Ballot ",
        paste(b, collapse = ","),
        " contains a candidate not in `candidates`."
      ))
    }
  }
}

#' @name `[.ranked_ballots`
#'
#' @title
#' Access Subsets of Ballots.
#'
#' @description
#' Extract subsets of ballots by index.
#'
#' @param x
#' Some \code{ranked_ballots}.
#'
#' @param i
#' The index, or vector of indices corresponding to each ballot to be extracted.
#'
#' @export
`[.ranked_ballots` <- function(x, i = NULL) {
  subset <- unclass(x)[i]
  attr(subset, "class") <- attr(x, "class")
  attr(subset, "candidates") <- attr(x, "candidates")
  subset
}

#' @name ranked_ballots
#'
#' @title
#' Construct a set of ranked ballots.
#'
#' @description
#' \code{ranked_ballots} is used to easily construct a set of ranked ballots.
#'
#' @examples
#' ranked_ballots(LETTERS[1:5])
#' ranked_ballots(list(LETTERS[1:5], LETTERS[6:1]))
#'
#' @param x
#' A character vector representing a single ballot, or a list of
#' character vectors representing multiple ballots.
#'
#' @param candidates
#' A character vector of names corresponding to the candidates running in
#' the election.
#'
#' @param \\dots
#' Additional parameters to pass to \code{ranked_ballots}.
#'
#' @return A \code{ranked_ballots} object representing the ballot(s).
#'
#' @export
ranked_ballots <- function(x, candidates = NULL, ...) {
  # If a single vector is passed, add it to a singleton list.
  if (typeof(x) == "character") {
    x <- list(x)
  } else if (typeof(x) == "NULL") {
    x <- list()
  }

  # Check ballots are valid
  validate_rankedballots(x, candidates)

  if (is.null(candidates)) {
    candidates <- sort(unique(unlist(x)))
  }

  # Return the ranked_ballots object
  return(structure(
    x,
    class = "ranked_ballots",
    candidates = candidates
  ))
}

#' @name write_ballots
#'
#' @title
#' Write \code{ranked_ballots} to a file.
#'
#' @description
#' Writes a set of ballots to a new file. This follows the
#' ballot:count standard, with a header describing the candidates.
#'
#' @examples
#' \donttest{
#' write_ballots(ranked_ballots(c(LETTERS)), "test.txt") # Output to test.txt
#' }
#' write_ballots(ranked_ballots(c(LETTERS))) # Output to stdout
#'
#' @param ballots
#' The \code{ranked_ballots} to write to a file.
#'
#' @param filename
#' The name of the file to write to, or \code{""} to write output to stdout.
#'
#' @param return_lines
#' A flag which determines whether or not the output should be returned as a
#' character vector
#'
#' @param suppress
#' A flag which, when True, suppresses any output to stdout.
#'
#' @export
write_ballots <- function(ballots,
                          filename = "",
                          return_lines = FALSE,
                          suppress = FALSE) {
  stopifnot(class(ballots) %in% .ballot_types)

  if (filename == "") {
    cout <- TRUE
  } else {
    cout <- FALSE
    f <- file(filename)
  }
  lines <- c()

  candidates <- attr(ballots, "candidates")
  ballot_counts <- count_ballots(ballots, candidates = candidates)

  candidates <- attr(ballot_counts, "candidates")

  # The file header contains candidate names
  lines <- c(lines, paste(collapse = ", ", candidates))
  lines <- c(lines, paste(collapse = ", ", candidates))
  lines <- c(lines, paste(collapse = "+", rep("-", length(candidates))))

  # The rest of the file contains the ballot:count pairs.
  for (bc in ballot_counts) {
    ballot <- paste(collapse = ", ", bc$ballot)
    lines <- c(lines, paste("(", ballot, ") : ", bc$count, sep = ""))
  }

  if (cout) {
    if (!suppress) {
      cat(lines, sep = "\n")
    }
    if (return_lines) {
      return(lines)
    }
  } else {
    writeLines(lines, f)
    close(f)
    if (return_lines) {
      return(lines)
    }
  }
}

# Helper function to count ballots by type.
count_ballots <- function(ballots, candidates) {
  candidates <- sort(unique(unlist(ballots)))

  # Count number of occurances for each ballot using the stackoverflow user
  # 2414948/alexis-laz answer to ttps://stackoverflow.com/questions/39372372
  ballots_unq <- unique(ballots)
  counts <- tabulate(match(ballots, ballots_unq))
  ballot_counts <- lapply(
    seq_along(counts),
    function(i) list(ballot = ballots_unq[[i]], count = counts[i])
  )

  attr(ballot_counts, "candidates") <- candidates
  return(ballot_counts)
}


#' @name read_ballots
#'
#' @title
#' Read \code{ranked_ballots} from a file.
#'
#' @description
#' Reads a set of partial IRV ballots from a file. The file is expected to
#' follow the ballot:count standard, with a header describing all participating
#' candidates.
#'
#' @param file
#' The name of the file to read from, or a character vector of file lines.
#'
#' @export
read_ballots <- function(file) {
  ballots <- list()

  # Read the file.
  if (length(file) == 1 && file.exists(file)) {
    lines <- readLines(file)
  } else {
    lines <- file
  }

  ballots <- tryCatch(
    {
      # First 2 or 3 lines are the header, we only use the first of those.
      candidates <- strsplit(gsub(" ", "", lines[1]), ",")[[1]]

      # Check if the header contains the affiliated parties or not.
      if (gsub("[-+]*", "", lines[2]) == "") {
        final_header_line <- 2
      } else {
        final_header_line <- 3
      }

      # Process the ballots.
      lines_body <- gsub("[() ]", "", lines[-(1:final_header_line)])
      lines_body <- strsplit(lines_body, ":")
      ballot_types <- strsplit(sapply(lines_body, "[", 1), ",")
      counts <- strtoi(sapply(lines_body, "[", 2))
      ballots <- rep(ballot_types, counts)

      # Package them up and return.
      class(ballots) <- "ranked_ballots"
      attr(ballots, "candidates") <- candidates
      ballots
    },
    error = function(msg) {
      stop(paste0(
        "An error was encountered while reading ballots from ",
        "file or data:\n", msg
      ))
    }
  )

  return(ballots)
}

#' @name social_choice
#'
#' @title
#' Compute the outcome of an election.
#'
#' @description
#' \code{social_choice} reads a set of ballots, and computes the
#' outcome of the election. The outcome is described by a vector of winning
#' candidates, along with the elimination order of the losing candidates.
#'
#' @param x
#' The set of ballots for which to compute the outcome of the social
#' choice function.
#'
#' @param \\dots
#' Additional parameters to pass to \code{social_choice}.
#'
#' @examples
#' social_choice(ranked_ballots(list(LETTERS[1], LETTERS[1], LETTERS[2])))
#'
#' @export
social_choice <- function(x, ...) UseMethod("social_choice", x)

#' @name social_choice.ranked_ballots
#'
#' @title
#' Evaluate a social choice function on a set of \code{ranked_ballots}.
#'
#' @description
#' \code{social_choice.ranked_ballots} reads a set of ranked ballots and
#' computes an election outcome. The outcome depends on the chosen
#' social choice function, along with any its associated parameters.
#'
#' @param x
#' The set of ballots for which to compute the outcome of the IRV
#' social choice function.
#'
#' @param n_winners
#' The number of candidates to elect.
#'
#' @param fn
#' The social choice function to use. \describe{
#'   \item{\code{"irv"}}{The IRV social choice function.}
#' }
#'
#' @param \\dots Unused.
#'
#' @export
social_choice.ranked_ballots <- function(x,
                                         n_winners = 1,
                                         fn = "irv",
                                         ...) {
  stopifnot(class(x) %in% .ballot_types)
  if (fn == "irv") {
    return(social_choice_irv(x, n_winners, attr(x, "candidates"), gseed()))
  } else {
    stop(paste0("Social choice function `", fn, "` not implemented."))
  }
}

# Helper function to get a random seed string to pass to CPP methods
gseed <- function() {
  return(paste(sample(LETTERS, 10), collapse = ""))
}

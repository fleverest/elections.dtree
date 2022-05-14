.ballot.types <- c("PIRVBallots")

#' @name write.ballots
#' @title Write PIRV ballots to a file.
#' @description Writes a set of ballots to a new file. This follows the ballot:count standard, with a header describing candidates.
#' @param ballots The IRVBallots to write to a file.
#' @param filename The name of the file to write to, or \code{""} to print to stdout.
#' @export
write.ballots <- function(ballots, filename="") {

  # TODO: Warning once we fully implement ballot classes
  #stopifnot(class(ballots) %in% .ballot.types)

  if (filename=="") {
    cout = T
  } else {
    cout = F
    f <- file(filename)
  }
  lines <- c()

  ballot.counts <- count.ballots(ballots, candidates=candidates)

  candidates <- attr(ballot.counts, "candidates")

  # The file header contains candidate names
  lines <- c(lines, paste(collapse=", ", candidates))
  lines <- c(lines, paste(collapse=", ", candidates))
  lines <- c(lines, paste(collapse="+", rep('-', length(candidates))))

  # The rest of the file contains the ballot:count pairs.
  for (bc in ballot.counts) {
    ballot <- paste(collapse=", ", bc$ballot)
    lines <- c(lines, paste("(",ballot,") : ", bc$count, sep=""))
  }

  if (cout) {
    cat(lines, sep="\n")
  } else {
    writeLines(lines, f)
    close(f)
  }
}

# Helper function to count ballots by type.
count.ballots <- function(ballots, candidates) {

  ballot.counts <- list()
  if (is.null(candidates)) {
    candidates <- c()
  }

  for (b in ballots) {
    # Check for any unseen candidates
    candidates <- unique(c(candidates, b))

    l <- length(ballot.counts)

    # Check if it has been seen before.
    seen <- F
    for (i in 1:l) {
      if (l > 0
        && length(ballot.counts[[i]]$ballot)==length(b)
        && all(ballot.counts[[i]]$ballot==b)
      ) {
        # If it is seen, increment the counter
        seen <- T
        ballot.counts[[i]]$count = ballot.counts[[i]]$count + 1
        break
      }
    }
    # If it was not seen, add it to our collection.
    if (!seen) {
      ballot.counts <- c(ballot.counts, list(list(ballot=b, count=1)))
    }
  }
  attr(ballot.counts, "candidates") <- candidates
  return(ballot.counts)
}


#' @name read.ballots
#' @title Read PIRV ballots from a file.
#' @description Reads a set of partial IRV ballots from a file. The file is expected to follow the ballot:count standard, with a header describing all participating candidates.
#' @param filename The name of the file to read from.
#' @export
read.ballots <- function(filename) {
  ballots <- list()

  # Read the file.
  lines = readLines(filename)

  # First 2 or 3 lines are the header, we only use the first of those.
  candidates <- strsplit(gsub(" ", "", lines[1]), ",")[[1]]

  # Check if the header contains the affiliated parties or not.
  if (gsub("[-+]*", "", lines[2]) == "")
    final.header.line <- 2
  else
    final.header.line <- 3

  # Process the ballots.
  lines.body   <- gsub("[() ]", "", lines[-(1:final.header.line)])
  lines.body   <- strsplit(lines.body, ":")
  ballot.types <- strsplit(sapply(lines.body, "[", 1), ",")
  counts       <- strtoi(sapply(lines.body, "[", 2))
  ballots      <- rep(ballot.types, counts)

  # Package them up and return.
  class(ballots) <- "PIRVBallots"
  attr(ballot, "candidates") <- candidates
  return(ballots)
}

#' @name social.choice
#' @title Computes the outcome of the appropriate social choice function.
#' @description Reads a set of ballots, and computes the outcome of the election. The outcome is described by a vector of winning candidates, along with the elimination order of the losing candidates.
#' @param ballots The set of ballots for which to compute the outcome of the social choice function.
#' @param nWinners The number of candidates to elect.
#' @export
social.choice <- function(x, ...) UseMethod("social.choice", x)

#' @export
social.choice.PIRVBallots <- function(x, nWinners = 1, ...) {
  RSocialChoiceIRV(x, nWinners, gseed())
}

# Helper function to get a random seed string to pass to CPP methods
gseed <- function() {
  return( paste(sample(LETTERS, 10), collapse="") )
}

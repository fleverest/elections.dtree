#' @name write.ballots
#' @title Write PIRV ballots to a file.
#' @description Writes a set of partial IRV ballots to a new file. This follows the ballot:count standard, with a header describing candidates.
#' @param filename The name of the file to write to.
#' @param ballots The IRVBallots to write to a file.
#' @param candidates Optionally, supply the candidate names in advance.
#' @export
write.ballots <- function(filename, ballots, candidates=NULL) {

  stopifnot(class(ballots)=="PIRVBallots")

  f <- file(filename)
  lines <- c()

  temp <- count.ballots(ballots, candidates=candidates)

  ballot.counts <- temp$ballot.counts
  candidates <- temp$candidates

  # The file header contains candidate names
  lines <- c(lines, paste(collapse=", ", candidates))
  lines <- c(lines, paste("IND", paste(collapse=", IND", candidates), sep=""))
  lines <- c(lines, paste(collapse="+", rep('-', length(candidates))))

  # The rest of the file contains the ballot:count pairs.
  for (bc in ballot.counts) {
    ballot <- paste(collapse=", ", bc$ballot)
    lines <- c(lines, paste("(",ballot,") : ", bc$count, sep=""))
  }

  writeLines(lines, f)
  close(f)
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
      if (l > 0 && all(ballot.counts[[i]]$ballot==b)) {
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
  return(list(ballot.counts=ballot.counts, candidates=candidates))
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

  # First 3 lines are the header, we only use the first of the three.
  candidates <- strsplit(gsub(" ", "", lines[1]), ",")[[1]]

  # Then the ballots follow:
  for (i in 4:length(lines)) {
    line <- gsub("[() ]","",lines[i])
    ballot.count.str <- strsplit(line, ":")[[1]]
    count <- strtoi(ballot.count.str[2])
    ballot <- strsplit(ballot.count.str[1], ",")[[1]]
    if (length(ballot) > 0) {
      for (j in 1:count) {
        ballots <- c(ballots, list(ballot))
      }
    } else {
      warning("Ballot of length 0 encountered.")
    }
  }

  class(ballots) <- "PIRVBallots"
  return(list(candidates=candidates, ballots=ballots))
}

#' @name social.choice
#' @title Computes the outcome of the appropriate social choice function.
#' @description Reads a set of ballots, and computes the outcome of the election. The outcome is described by a vector of winning candidates, along with the elimination order of the losing candidates.
#' @param ballots The set of ballots for which to compute the outcome of the social choice function.
#' @param nWinners The number of candidates to elect.
#' @export
social.choice.PIRVBallots <- function(ballots, nWinners=1) {
  return(RSocialChoiceIRV(ballots, nWinners))
}

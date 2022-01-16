#!/usr/bin/Rscript

require(ggplot2)
require(dirtree.elections)

set.seed(12345)

nCandidates = 5    # Number of candidates participating
nc = nCandidates
nBallots    = 1000 # Number of ballots cast per election
nb = nBallots
nElections  = 1000  # Number of elections for sampling precision
ne = nElections
n.audits = 100     # Number of ballot orderings considered for each election
little.m = 100     # Maximum number of ballots counted before automatically halting
halt.thresh = 0.90 # The probability threshold which represents "enough evidence"

nRepetitions = 100

dtree <- dirtree.irv(nCandidates=nc, scale=1.)

cert.rates <- c()
margins <- c()
mean.sample.sizes <- c()

for (k in 1:nRepetitions) {
  # Sample an election
  full.election <- draw(dtree, nBallots=nb)

  # Evaluate the true outcome
  true.winner <- evaluate.election(full.election)

  # TODO: calculate exact margin
  first.prefs <- table(full.election[,1])
  ordered.first.prefs <- first.prefs[order(first.prefs, decreasing=TRUE)]
  print(ordered.first.prefs)
  margin.approx <- ceiling((ordered.first.prefs[1] - ordered.first.prefs[2])/2)

  c.margins <- rep(0, nCandidates)
  # Calculate the "margin" for each candidate.
  # This is an estimate for the number of ballots required to make each candidate win.
  for (i in 1:nCandidates) {
    if (i == true.winner) {
      c.margins[i] <- margin.approx
    }
    if (i != true.winner) {
      c.margins[i] <- floor((first.prefs[i] - max(first.prefs))/2)
    }
  }

  cat("Approximate margins: ", c.margins, "\n")
  cat("True winner: ", true.winner, "\n")

  cert.rate <- rep(0, nCandidates)
  mean.sample.size <- rep(0, nCandidates)

  for (i in 1:n.audits) {

    cat(i, " ")

    # Shuffle election
    full.election <- full.election[sample(1:nb,nb),]

    # Conduct an audit for all candidates
    certified = rep(FALSE, nCandidates)
    sample.size = rep(little.m, nCandidates) # number of ballots counted before certification (m by default)
    post.probs <- rep(0, nCandidates)
    for (j in 1:little.m) {

      if (all(certified)) next # Stop auditing if all are certified

      update(dtree, full.election[j,]) # Update with next ballot
      # Eval posterior probs
      post.probs <- samplePosterior(dtree, nElections=ne, nBallots=nb)/ne
      for (n in 1:nc) {
        if (certified[n]) next # Skip if already certified

        if (post.probs[n] > halt.thresh) { # certify candidate
          certified[n] <- TRUE
          sample.size[n] <- j
        }
      }
    }

    mean.sample.size <- mean.sample.size + sample.size/n.audits
    cert.rate <- cert.rate + certified/n.audits

    clear(dtree)
  }

  cat("\n")

  cert.rates <- c(cert.rates, cert.rate)
  margins <- c(margins, c.margins)
  mean.sample.sizes <- c(mean.sample.sizes, mean.sample.size)
}


outdf <- data.frame(
  margin=margins,
  cert.rate=cert.rates,
  mean.sample.size=mean.sample.sizes
)

write.csv(outdf, "election_halting.csv")

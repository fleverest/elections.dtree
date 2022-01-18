#!/usr/bin/Rscript

# Command line arguments
library('getopt')

spec <- matrix(c(
  'alpha0', 'a', 1, "numeric", "The prior parameter to consider (required)",
  'repetitions', 'r', 1, "numeric", "The number of elections to audit (optional, default 50)",
  'nCandidates', 'c', 1, "integer", "The number of candidates in the election (optional, default 4)" ,
  'nBallots', 'n', 1, "integer", "The number of ballots cast in the election (optional, default 1000)",
  'nElections', 'e', 1, "integer", "The number of monte carlo election samples to draw for posterior calculation (optional, default 250)",
  'nAudits', 'k', 1, "integer", "The number of audits per election (optional, default 100)",
  'm', 'm', 1, "integer", "Maximum number of ballots observed before proceeding to full recount (optional, default 100)",
  'seed', 's', 1, "integer", "The seed for the experiment (optional, default 12345)",
  'help', 'h', 0, "logical", "Display this help menu"
  ), ncol=5, byrow=T)

opt = getopt(spec)
if (!is.null(opt$help)) {
    cat(paste(getopt(spec, usage=T),"\n"))
    q()
}

# Ensure required parameters are passed.
if (any(mapply(is.null,
  list(opt$alpha0)
))) {
  cat("Error. Missing required parameters.\n")
  cat(paste(getopt(spec, usage=T), "\n"))
  q()
} else {
  alpha0 <- as.numeric(opt$alpha0)
}


require(ggplot2)
require(dirtree.elections)

set.seed(12345)

if (is.null(opt$nCandidates)) {
  nCandidates = 5    # Number of candidates participating
} else {
  nCandidates = opt$nCandidates
}
nc = nCandidates

if (is.null(opt$nBallots)) {
  nBallots = 1000
} else {
  nBallots = opt$nBallots
}
nb = nBallots

if (is.null(opt$nElections)) {
  nElections = 250
} else {
  nElections = opt$nElections
}
ne = nElections

if (is.null(opt$m)) {
  little.m = 100
} else {
  little.m = opt$m
}

if (is.null(opt$nAudits)) {
  n.audits = 100
} else {
  n.audits = opt$nAudits
}

if (is.null(opt$halt.thresh)) {
  halt.thresh = 0.90
} else {
  halt.thresh = opt$halt.thresh
}

if (is.null(opt$repetitions)) {
  nRepetitions = 100
} else {
  nRepetitions = opt$repetitions
}

election.tree <- dirtree.irv(nCandidates=nc, 1.)

dtree <- dirtree.irv(nCandidates=nc, alpha0)

cert.rates <- c()
margins <- c()
mean.sample.sizes <- c()

for (k in 1:nRepetitions) {
  # Sample an election
  full.election <- draw(election.tree, nBallots=nb)

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

write.csv(
  outdf,
  paste(
    "certrate",
    "_a", alpha0,
    "_c", nc,
    "_n", nb,
    "_r", nRepetitions,
    "_k", n.audits,
    ".csv", sep="")
)

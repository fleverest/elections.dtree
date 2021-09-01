#!/usr/bin/Rscript

# Command line arguments
library('getopt')

spec <- matrix(c(
  'nCandidates', 'n', 1, "integer", "The number of candidates in the election (required)" ,
  'nBallots', 'm', 1, "integer", "The number of ballots cast in the election (required)",
  'scales', 's', 1, "character", "The prior scales to consider (required)",
  'eScale', 'k', 1, "numeric", "The scale of the initial election, determines the margin (required)",
  'nElections', 'e', 1, "integer", "The number of elections to draw during sampling (optional, default 100)",
  'nRepetitions', 'r', 1, "integer", "The number of repetitions of the experiment (optional, default 10)",
  'nSteps', 'p', 1, "integer", "The number of points to evaluate the posterior at (optional, default 10)",
  'seed', 'c', 1, "character", "The seed for the experiment (optional, default \"seed\")",
  'help', 'h', 0, "logical", "Display this help menu"
  ), ncol=5, byrow=T)

opt = getopt(spec)
if (!is.null(opt$help)) {
    cat(paste(getopt(spec, usage=T),"\n"))
    q()
}

# Ensure required parameters are passed.
if (any(mapply(is.null,
  list(opt$nCandidates, opt$nBallots, opt$scales, opt$eScale)
))) {
  cat("Error. Missing required parameters.\n")
  cat(paste(getopt(spec, usage=T), "\n"))
  q()
} else {
  nCandidates <- opt$nCandidates
  nBallots <- opt$nBallots
  eScale <- opt$eScale
  scales <- as.vector(mapply(as.numeric,strsplit(opt$scales,',')[[1]]))
}

# Set defaults for missing parameters.
if (is.null(opt$seed)) {
  seed <- "seed"
} else {
  seed <- opt$seed
}
if (is.null(opt$nSteps)) {
  nSteps <- 10
} else {
  nSteps <- opt$nSteps
}
if (is.null(opt$nRepetitions)) {
  nRepetitions <- 10
} else {
  nRepetitions <- opt$nRepetitions
}
if (is.null(opt$nElections)) {
  nElections <- 100
} else {
  nElections <- opt$nElections
}
stepSize     = nBallots/nSteps

require('dirtree.elections')
require('ggplot2')
dir  = paste("nCandidates",nCandidates,"/","eScale",eScale,"/",sep="")
name = paste(
  dir,
  paste("nBallots", nBallots, sep=""),
  sep = ""
)

# Simulate an election from a dirichlet tree with scale `eScale` to audit.
dtree <- dirtree.irv(nCandidates, scale=eScale)
election.full <- draw(dtree, nBallots)
cat("Election simulated.\n")
winner <- evaluate.election(election.full)
cat("First preferences:")
print(table(election.full[,1]))
cat(paste(winner, "wins the election.\n"))

# Prepare output dataframe columns
df.results = data.frame(
  usingObserved=logical(),
  treeType=character(),
  scale=numeric(),
  counted=numeric(),
  numWins=integer(),
  stringsAsFactors=F
)

addRow <- function(df.results, newvals) {
  rbind(
    df.results,
    setNames(
      data.frame(newvals),
      colnames(df.results)
    )
  )
}

# Conduct `nRepetitions` audits for each of the priors.
for (i in 1:nRepetitions) {
  cat(paste("\nRepetition",i,"\n",sep=" "))

  # Reset the dtree to the initial prior.
  clear(dtree)

  # Determine a ballot ordering, here we're sampling with replacement from
  # the set of possible ballet orderings.
  election.i <- election.full[sample(1:nBallots,nBallots),]

  # Proceeding in steps, determine the posterior after updating with next batch and add to df.
  counted = 0.
  pb <- txtProgressBar(min = 1, max = nSteps, style = 3)
  for (stepNum in 1:nSteps) {
    setTxtProgressBar(pb, stepNum)
    election.batch <- election.i[(stepSize*(stepNum-1)+1):(stepSize*stepNum),]
    counted = counted + stepSize
    update(dtree, election.batch)

    # Dirichlet Trees
    dtree$isDirichlet <- F
    for (s in scales) {
      dtree$scale <- s
      type <- "tree"
      df.results <- addRow(
        df.results,
        list(
          F,
          type,
          s,
          counted,
          samplePosterior(
            dtree,
            nElections,
            nBallots,
            F
          )[winner]
        )
      )
      df.results <- addRow(
        df.results,
        list(
          T,
          type,
          s,
          counted,
          samplePosterior(
            dtree,
            nElections,
            nBallots,
            T
          )[winner]
        )
      )
    }
    # Dirichlets
    dtree$isDirichlet <- T
    for (s in scales) {
      dtree$scale <- s
      type <- "dirichlet"
      df.results <- addRow(
      df.results,
        list(
          F,
          type,
          s,
          counted,
          samplePosterior(
            dtree,
            nElections,
            nBallots,
            F
          )[winner]
        )
      )
      df.results <- addRow(
        df.results,
        list(
          T,
          type,
          s,
          counted,
          samplePosterior(
            dtree,
            nElections,
            nBallots,
            T
          )[winner]
        )
      )
    }
  }
}

# construct a new skeleton dataframe with the summary statistics for
# each distribution / audit count step.
df.out <- unique(df.results[,1:4])
nrows <- dim(df.out)[1]
df.out$mean <- rep(0,nrows)
df.out$pi.lower <- rep(0,nrows)
df.out$pi.upper <- rep(1,nrows)

# The negative log-likelihood of the beta-binomial distribution
negll.betabin <- function(par, k, n) {
  return(
    sum(
      lbeta(par[1],par[2]) - lbeta(k+par[1],n-k+par[2])
    )
  )
}

comb.res <- interaction(df.results[,1:4])
comb.out <- interaction(df.out[,1:4])
for (i in 1:nrows) {
  # We want the results which correspond to the same step of the
  # same distribution for each repetition.
  df.rows <- df.results[comb.res == comb.out[i],]
  # the beta-binomial samples
  ks <- df.rows$numWins
  par <- optim(c(10,10),negll.betabin, k = ks, n = nElections, lower = 0.01, method = "L-BFGS-B")$par
  # Calculate mean, 0.05 and 0.95 quantiles for beta probability with these parameters.
  df.out$mean[i] <- par[1]/sum(par)
  df.out$pi.lower[i] <- qbeta(0.05,par[1],par[2])
  df.out$pi.upper[i] <- qbeta(0.95,par[1],par[2])
}

dir.create(dir, recursive=T)

write.csv(df.results, paste(name,'raw.csv',sep='.'))
write.csv(df.out, paste(name,'csv',sep='.'))
png(paste(name,'png',sep='.'), width=1920, height=1080)

ggplot(
  df.out,
  aes(
    x=counted,
    y=mean,
    color=as.factor(scale),
    group=as.factor(scale)
  )
) +
  geom_line() +
  geom_ribbon(aes(y=mean, ymin=pi.lower, ymax=pi.upper), alpha=0.1) +
  facet_wrap(~interaction(treeType,usingObserved))

dev.off()


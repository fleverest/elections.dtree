#!/usr/bin/Rscript

# Command line arguments
library('getopt')

spec <- matrix(c(
  'nCandidates', 'n', 1, "integer", "The number of candidates in the election (required)" ,
  'nBallots', 'm', 1, "integer", "The number of ballots cast in the election (required)",
  'scale', 's', 1, "numeric", "The prior scale (alpha_0) to consider (required)",
  'eScale', 'k', 1, "numeric", "The scale of the initial election, determines the margin (required)",
  'nElections', 'e', 1, "integer", "The number of elections to draw during sampling (optional, default 100)",
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
  list(opt$nCandidates, opt$nBallots, opt$scale, opt$eScale)
))) {
  cat("Error. Missing required parameters.\n")
  cat(paste(getopt(spec, usage=T), "\n"))
  q()
} else {
  nCandidates <- opt$nCandidates
  nBallots <- opt$nBallots
  eScale <- opt$eScale
  scale <- as.numeric(opt$scale)
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
if (is.null(opt$nElections)) {
  nElections <- 100
} else {
  nElections <- opt$nElections
}
stepSize     = nBallots/nSteps

require('dirtree.elections')
require('ggplot2')
require('reshape2')

dir  = paste("singleaudit_nCandidates",nCandidates,"/","eScale",eScale,"/",sep="")
name = paste(
  dir,
  paste("nBallots", nBallots, "_", "priorScale", scale, sep=""),
  sep = ""
)

# Find the equivalent dirichlet prior parameter
dir.equivparam <- function(a, n) {
  A <- 1
  for (k in 2:n) {
    A <- A * (1+a)/(1+k*a)
  }
  alpha.dir <- (A-1)/(1-A*factorial(n))
  return(alpha.dir)
}
dirScale <- dir.equivparam(scale,nCandidates)

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
  treeType=character(),
  counted=numeric(),
  stringsAsFactors=F
)
for (i in 1:nCandidates) {
  df.results[[paste('propWins',i,sep='.')]] <- integer()
}

addRow <- function(df.results, newvals) {
  rbind(
    df.results,
    setNames(
      data.frame(newvals),
      colnames(df.results)
    )
  )
}

# Reset the dtree to the initial prior.
clear(dtree)

# Determine a ballot ordering for the election
election.i <- election.full[sample(1:nBallots,nBallots),]

# Proceeding in steps, determine the posterior after updating with next batch and add to df.
counted = 0.
for (stepNum in 1:nSteps) {
  election.batch <- election.i[(stepSize*(stepNum-1)+1):(stepSize*stepNum),]
  counted = counted + stepSize
  update(dtree, election.batch)

  # Dirichlet Trees
  dtree$isDirichlet <- F
  dtree$scale <- scale
  type <- "Dirichlet Tree"
  df.results <- addRow(
    df.results,
    c(
      list(
        type,
        counted
      ),
      as.list(
        samplePosterior(
          dtree,
          nElections,
          nBallots,
          T
        )/nElections
      )
    )
  )
  # Dirichlets
  dtree$isDirichlet <- T
  dtree$scale <- dirScale
  type <- "Dirichlet"
  df.results <- addRow(
    df.results,
    c(
      list(
        type,
        counted
      ),
      as.list(
        samplePosterior(
          dtree,
          nElections,
          nBallots,
          T
        )/nElections
      )
    )
  )
}

# Reshaped data for plotting.
df.long <- melt(df.results, id.vars=c("treeType","counted"))

levels(df.long$variable) <- 1:nCandidates
df.long$Candidate <- df.long$variable

dir.create(dir, recursive=T)

write.csv(df.results, paste(name,'raw.csv',sep='.'))

png(paste(name,'png',sep='.'), width=1920, height=1080)

ggplot(
  df.long,
  aes(
    x=counted,
    y=value,
    color=Candidate,
    group=Candidate
  )
) +
  geom_line() +
  facet_wrap(~treeType, nrow=2) +
  labs(x="# of ballots counted") +
  labs(y="Posterior P(Candidate wins)") +
  theme(text = element_text(size = 30))

dev.off()


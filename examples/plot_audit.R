require('dirtree.elections')
require('ggplot2')

seed         = "seed12345"

eScale       = 5.

nCandidates  = 10
nElections   = 500
nBallots     = 1000
scales       = c(0.01, 0.1, 1., 10., 100)

nRepetitions = 10
nSteps       = 10
stepSize     = nBallots/nSteps

name         = paste(
  paste("nCandidates",nCandidates,"/", sep=""),
  paste("eScale", eScale, "/", sep=""),
  paste("nBallots", nBallots, sep=""),
  sep = ""
)

# Simulate an election from a dirichlet tree with scale `eScale` to audit.
dtree <- dirtree.irv(10)
election.full <- sample(dtree, nBallots)
winner <- evaluateElection(election.full)
rm(electionTree)
gc()

# Prepare output dataframe columns
df.results = data.frame(
  repno=integer(),
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
  print(paste("Repetition",i,sep=" "))

  # Reset the prior distributions to their original form.
  rm(dtrees)
  rm(dirichlets)
  gc()
  dtrees <- c()
  dirichlets <- c()
  for (s in scales) {
    dtrees <- c(
      dtrees,
      new(
        RcppDirichletTreeIRV,
        nCandidates=nCandidates,
        scale=s,
        treeType="dirichlettree",
        seed=seed
      )
    )
    dirichlets <- c(
      dirichlets,
      new(
        RcppDirichletTreeIRV,
        nCandidates=nCandidates,
        scale=s,
        treeType="dirichlet",
        seed=seed
      )
    )
  }

  # Determine a ballot ordering, here we're sampling with replacement from
  # the set of possible ballet orderings.
  election.i <- election.full[sample(1:nBallots,nBallots),]

  # Proceeding in steps, determine the posterior after updating with next batch and add to df.
  counted = 0.
  for (stepNum in 1:nSteps) {
    print(paste("Step",stepNum,sep=" "))
    election.batch <- election.i[(stepSize*(stepNum-1)+1):(stepSize*stepNum),]
    counted = counted + stepSize

    # Dirichlet Trees
    for (distr in dtrees) {
      distr$update(election.batch)
      type <- "dtree"
      s <- distr$getScale()
      df.results <- addRow(
        df.results,
        list(
          i,
          F,
          type,
          s,
          counted,
          distr$samplePosterior(
            nElections,
            nBallots,
            F,
            64
          )[winner]
        )
      )
      df.results <- addRow(
        df.results,
        list(
          i,
          T,
          type,
          s,
          counted,
          distr$samplePosterior(
            nElections,
            nBallots,
            F,
            64
          )[winner]
        )
      )
    }
    # Dirichlets
    for (distr in dirichlets) {
      distr$update(election.batch)
      type <- "dirichlet"
      s <- distr$getScale()
      df.results <- addRow(
      df.results,
        list(
          i,
          F,
          type,
          s,
          counted,
          distr$samplePosterior(
            nElections,
            nBallots,
            F,
            64
          )[winner]
        )
      )
      df.results <- addRow(
        df.results,
        list(
          i,
          T,
          type,
          s,
          counted,
          distr$samplePosterior(
            nElections,
            nBallots,
            F,
            64
          )[winner]
        )
      )
    }
  }
}

# construct a new skeleton dataframe with the summary statistics for
# each distribution / audit count step.
df.out <- unique(df.results[,2:5])
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

comb.res <- interaction(df.results[,2:5])
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

write.csv(df.results, paste(name,'raw.csv',sep='.'))
write.csv(df.out, paste(name,'csv',sep='.'))
png(paste(name,'png',sep='.'), width=1920, height=1080)

ggplot(
  df.out,
  aes(
    x=jitter(counted),
    y=mean,
    color=as.factor(scale),
    group=as.factor(scale)
  )
) +
  geom_line() +
  geom_ribbon(aes(y=mean, ymin=pi.lower, ymax=pi.upper), alpha=0.1) +
  facet_wrap(~interaction(treeType,usingObserved))

dev.off()


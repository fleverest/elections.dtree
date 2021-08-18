require('dirtree.elections')
require('ggplot2')

seed         = "seed12345"
name         = "10c_10kb_medium_accurate"
eScale       = 5.

nCandidates  = 10
nElections   = 1000
nBallots     = 1000
scales       = c(0.01, 0.1, 1., 10.)

nRepetitions = 2
nSteps       = 100
stepSize     = nBallots/nSteps

# Simulate an election from a dirichlet tree with scale `eScale` to audit.
electionTree <- new(
  RcppDirichletTreeIRV,
  nCandidates=nCandidates,
  scale=eScale,
  treeType="dirichlettree",
  seed=seed
)
election.full <- electionTree$sample(nBallots=nBallots)
winner <- electionTree$evaluate(election.full)


# Prepare the prior distributions.
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

# Prepare output dataframe columns
outdf = data.frame(
                 repno=integer(),
                 usingObserved=logical(),
                 treeType=character(),
                 scale=numeric(),
                 counted=numeric(),
                 stringsAsFactors=F
)
for (i in 1:nCandidates) {
  colname = paste("wincount",as.character(i),sep='.')
  outdf[[colname]] <- integer()
}

addRow <- function(outdf, newvals) {
  rbind(
    outdf,
    setNames(
      data.frame(newvals),
      colnames(outdf)
    )
  )
}

# Conduct `nRepetitions` audits for each of the priors.
for (i in 1:nRepetitions) {
  print(paste("Repetition",i,sep=" "))
  # Reset the prior distributions to their original form.
  for (p in dtrees) {p$reset()}
  for (p in dirichlets) {p$reset()}

  # Determine a ballot ordering
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
      outdf <- addRow(
        outdf,
        append(
          list(
            i,
            F,
            type,
            s,
            counted
          ),
          as.list(
            distr$samplePosterior(
              nElections,
              nBallots,
              F,
              64
            )
          )
        )
      )
      outdf <- addRow(
        outdf,
        append(
          list(
            i,
            T,
            type,
            s,
            counted
          ),
          as.list(
            distr$samplePosterior(
              nElections,
              nBallots,
              T,
              64
            )
          )
        )
      )
    }
    # Dirichlets
    for (distr in dirichlets) {
      distr$update(election.batch)
      type <- "dirichlet"
      s <- distr$getScale()
      outdf <- addRow(
        outdf,
        append(
          list(
            i,
            F,
            type,
            s,
            counted
          ),
          as.list(
            distr$samplePosterior(
              nElections,
              nBallots,
              F,
              64
            )
          )
        )
      )
      outdf <- addRow(
        outdf,
        append(
          list(
            i,
            T,
            type,
            s,
            counted
          ),
          as.list(
            distr$samplePosterior(
              nElections,
              nBallots,
              T,
              64
            )
          )
        )
      )
    }
  }
}

write.csv(outdf, paste(name,'csv',sep='.'))
png(paste(name,'png',sep='.'), width=1920, height=1080)

ggplot(
  outdf,
  aes(
    x=jitter(counted),
    y=outdf[[paste("wincount",as.character(winner),sep='.')]],
    color=treeType,
    group=interaction(usingObserved,treeType,repno)
  )
) +
  geom_point(aes(shape=usingObserved)) +
  geom_line(aes(linetype=usingObserved)) +
  facet_wrap(~scale)

dev.off()


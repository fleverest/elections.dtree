#!/usr/bin/Rscript

require(ggplot2)
require(dirtree.elections)

nc = 10
nb = 1000
ne = 100

dtree <- dirtree.irv(nCandidates=10, scale=1.)

full.election <- draw(dtree, nBallots=nb)

print(table(full.election[,1]))
print(evaluateElection(full.election))

dat <- list()

for (i in 1:100) {

  print(i)

  # Shuffle election
  full.election <- full.election[sample(1:nb,nb),]

  probpaths = list()
  for (i in 1:nc) {
    probpaths[[i]] <- numeric()
  }

  for (j in 1:dim(full.election)[1]) {
    update(dtree, full.election[j,]) # Update with next ballot
    # Eval posterior probs
    posteriors <- samplePosterior(dtree, nElections=ne, nBallots=nb)
    for (i in 1:nc) {
      probpaths[[i]] <- c(probpaths[[i]], posteriors[i])
    }
  }

  dat[[i]] <- probpaths

  clear(dtree)
}

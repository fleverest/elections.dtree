#!/usr/bin/Rscript
require(dirtree.elections)
require(ggplot2)

# Find the equivalent dirichlet prior parameter
dir.equivparam <- function(a, n) {
  A <- 1
  for (k in 2:n) {
    A <- A * (1+a)/(1+k*a)
  }
  alpha.dir <- (A-1)/(1-A*factorial(n))
  return(alpha.dir)
}

ballot <- c(1,2,3,4)
n <- 4
s <- 10.
reps <- 10000

# Dirichlet Tree prior
dtree <- dirtree.irv(nCandidates = n, scale = s)

ps.dtree <- c()
for (i in 1:reps) {
  ps.dtree <- c(ps.dtree, sampleLeafProbability(dtree, ballot))
}

# Equivalent Dirichlet prior

dtree$isDirichlet <- T
dtree$scale <- dir.equivparam(s, n)

ps.dir <- c()
for (i in 1:reps) {
  ps.dir <- c(ps.dir, sampleLeafProbability(dtree, ballot))
}

# Filter zeros
ps.dtree <- ps.dtree[ps.dtree>0]
ps.dir <- ps.dir[ps.dir>0]

dataf <- data.frame(
  prior=c(
    rep("Dirichlet-Tree",length(ps.dtree)),
    rep("Dirichlet",length(ps.dir))
  ),
  prob=c(
    ps.dtree,
    ps.dir
  )
)

mean(ps.dtree)
var(ps.dtree)

mean(ps.dir)
var(ps.dir)

png("log_p_prior_densities.png", width=1920, height=1080)
ggplot(dataf, aes(x=log(prob), grouping=prior, color=prior)) +
  geom_density() +
  geom_vline(xintercept=log(1/factorial(n)))
dev.off()

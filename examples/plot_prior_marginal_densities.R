#!/usr/bin/Rscript
require(dirtree.elections)
require(ggplot2)

# Dirichlet scale parameter with equivalent variance
getDirScale <- function(s, n){
  n.fac <- factorial(n)
  # Calculate f(s.dt,n)
  f <- 1
  for (l in 2:n) {
    f <- f * (1+s)/(1+l*s)
  }
  s.dir <- (1 - f)/(f * n.fac - 1)
  return(s.dir)
}

ballot <- c(1,2,3,4)
n <- 4
s <- 10.
reps <- 100000

# Dirichlet Tree prior
dtree <- dirtree.irv(nCandidates = n, scale = s)

ps.dtree <- c()
for (i in 1:reps) {
  ps.dtree <- c(ps.dtree, sampleLeafProbability(dtree, ballot))
}

# Equivalent Dirichlet prior

dtree$isDirichlet <- T
dtree$scale <- getDirScale(s, n)

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

summary(ps.dtree)
summary(ps.dir)

png("log_p_prior_densities.png", width=1920, height=1080)
ggplot(dataf, aes(x=log(prob), grouping=prior, color=prior)) +
  geom_density() +
  geom_vline(xintercept=log(1/factorial(n)))
dev.off()

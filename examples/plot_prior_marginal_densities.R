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

# Plotting parameters
n <- 3
a.dtree <- 10.
reps <- 100000


ballot <- 1:n
a.dir <- dir.equivparam(a.dtree, n)

# Dirichlet Tree prior
dtree <- dirtree.irv(nCandidates = n, scale = a.dtree)

ps.dtree <- c()
for (i in 1:reps) {
  ps.dtree <- c(ps.dtree, sampleLeafProbability(dtree, ballot))
}

# Equivalent Dirichlet prior
dtree$isDirichlet <- T
dtree$scale <- a.dir

ps.dir <- c()
for (i in 1:reps) {
  ps.dir <- c(ps.dir, sampleLeafProbability(dtree, ballot))
}

dataf <- data.frame(
  prior=c(
    rep("Dirichlet-Tree",length(ps.dtree)),
    rep("Equivalent Dirichlet",length(ps.dir))
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
  geom_vline(xintercept=log(1/factorial(n))) +
  labs(x = "log p_b", y = "Density", color="Prior") +
  theme(text = element_text(size = 25))
dev.off()


#!/usr/bin/Rscript

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

out.dtree.alpha <- c()
out.dir.alpha <- c()
out.ns <- c()

ns <- c(5,10,20)
dtree.alpha <- seq(1,10,1/100)

for (n in ns) {
  for (a in dtree.alpha) {
    out.dtree.alpha <- c(out.dtree.alpha, a)
    out.ns <- c(out.ns, n)
    out.dir.alpha <- c(out.dir.alpha, dir.equivparam(a, n))
  }
}

outdf <- data.frame(N=as.factor(out.ns), dtree.alpha=out.dtree.alpha, dir.alpha=out.dir.alpha)
outdf[1:5,]
png('alphas.png', height=600, width=800)
ggplot(outdf, aes(x=dtree.alpha, y=dir.alpha, col=N, group=N)) +
  geom_line() +
  labs(x = "Dirichlet Tree parameter", y = "Dirichlet Parameter") +
  theme(text = element_text(size = 20))
dev.off()

png('log-alphas.png', height=600, width=800)
ggplot(outdf, aes(x=log(dtree.alpha), y=log(dir.alpha), col=N, group=N)) +
  geom_line() +
  labs(x = "Log Dirichlet Tree parameter", y = "Log Dirichlet Parameter") +
  theme(text = element_text(size = 20))
dev.off()

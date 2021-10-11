#!/usr/bin/Rscript

require(ggplot2)

# Dirichlet scale parameter with equivalent variance
getDirScale <- function(s, n){
  n.fac <- factorial(n)
  # Calculate f(s.dt, 1, n)
  f <- 1
  for (l in 2:n) {
    f <- f * (1+s)/(1+l*s)
  }
  s.dir <- (1 - f)/(f * n.fac - 1)
  return(s.dir)
}

out.dtree.alpha <- c()
out.dir.alpha <- c()
out.ns <- c()

ns <- c(5,10,20,40,80)
dtree.alpha <- 1:1000/100

for (n in ns) {
  for (a in dtree.alpha) {
    out.dtree.alpha <- c(out.dtree.alpha, a)
    out.ns <- c(out.ns, n)
    out.dir.alpha <- c(out.dir.alpha, getDirScale(a, n))
  }
}

outdf <- data.frame(N=out.ns, dtree.alpha=out.dtree.alpha, dir.alpha=out.dir.alpha)
png('alphas.png', height=600, width=800)
ggplot(outdf, aes(x=dtree.alpha, y=dir.alpha, col=N, group=N)) +
  geom_line() +
  labs(x = "Dirichlet Tree parameter", y = "Dirichlet Parameter")
dev.off()

png('log-alphas.png', height=600, width=800)
ggplot(outdf, aes(x=log(dtree.alpha), y=log(dir.alpha), col=N, group=N)) +
  geom_line() +
  labs(x = "Log Dirichlet Tree parameter", y = "Log Dirichlet Parameter") +
  theme(text = element_text(size = 20))
dev.off()

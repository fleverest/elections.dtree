#!/usr/bin/Rscript

require(reshape2)
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

# Moment functions
dir.postprob.moment1 <- function(s,k,n) {
    if (k==n-1) {
        return((s+1)/(factorial(n)*s + 1))
    } else {
        return(s/(factorial(n)*s + 1))
    }
}

dtree.postprob.moment1 <- function(s,k,n) {
    sigma <- 1
    if (k>0) { # If the ballots have any candidates in common...
        for (l in (n-k+1):n) {
            sigma <- sigma * (s+1)/(l*s+1)
        }
    }
    if (k < (n-1)) { # If it ever deviates...
        delta <- s/((n-k)*s + 1)
    } else {
        delta <- 1
    }
    gamma <- 1/factorial(n-k-1)
    return(sigma * delta * gamma)
}

dir.postprob.moment2 <- function(s,k,n) {
    if (k==n-1) {
        return((s+1)/(factorial(n)*s + 1) * (s+2)/(factorial(n)*s+2))
    } else {
        return(s/(factorial(n)*s + 1) * (s+1)/(factorial(n)*s + 2))
    }
}

dtree.postprob.moment2 <- function(s,k,n) {
    sigma.2 <- 1
    if (k>0) { # If the ballots have any candidates in common...
        for (l in (n-k+1):n) {
            sigma.2 <- sigma.2 * (s+1)/(l*s+1) * (s+2)/(l*s+2)
        }
    }
    delta.2 <- 1
    if (k < (n-1)) { # If it ever deviates...
        delta.2 <- s/((n-k)*s + 1) * (s+1)/((n-k)*s + 2)
    }
    gamma.2 <- 1
    if (k > (n-1)) { # If it ever deviates...
        for (l in 2:(n-k-1)){
            gamma.2 <- gamma.2 * (s+1)/(l*(l*s + 1))
        }
    }
    return(sigma.2 * delta.2 * gamma.2)
}

# Plotting
n <- 10
s <- 1 # Dtree prior parameter
s.dir <- getDirScale(s, n) # Equivalent (by variance) dirichlet parameter
ks <- 0:(n-1)
p.dir.moment1 <- c()
p.dtree.moment1 <- c()
p.dir.moment2 <- c()
p.dtree.moment2 <- c()
for (k in ks) {
    p.dir.moment1 <- c(p.dir.moment1, dir.postprob.moment1(s.dir,k,n))
    p.dtree.moment1 <- c(p.dtree.moment1, dtree.postprob.moment1(s,k,n))
    p.dir.moment2 <- c(p.dir.moment2, dir.postprob.moment2(s.dir,k,n))
    p.dtree.moment2 <- c(p.dtree.moment2, dtree.postprob.moment2(s,k,n))
}

outdf <- data.frame(k=rep(n-ks-1,2))
outdf$Prior <- c(rep("Dirichlet Tree",n), rep("Dirichlet",n))
outdf$mom.1 <- log(c(p.dtree.moment1,p.dir.moment1))
outdf$mom.1.weighted <- log(c(
     unlist(Map(function(x) max(1,x)*factorial(x),(n-1):0))
         * p.dtree.moment1,
     unlist(Map(function(x) max(1,x)*factorial(x),(n-1):0))
         * p.dir.moment1
  ))
outdf$mom.2=log(c(
    p.dtree.moment2,
    p.dir.moment2
  ))
outdf$var=log(c(
    p.dtree.moment2 - p.dtree.moment1^2,
    p.dir.moment2 - p.dir.moment1^2
  ))
outdf.long <- melt(outdf, id.vars = c("Prior","k"))
levels(outdf.long$variable) <- c("1st Moment", "Weighted 1st Moment", "2nd Moment", "Variance")
png('moments.png', height = 600, width=800)
ggplot(outdf.long, aes(x=k, y=value, col=Prior)) +
  geom_point() +
  geom_line() +
  facet_wrap(~variable) +
  theme(text = element_text(size = 20))
dev.off()

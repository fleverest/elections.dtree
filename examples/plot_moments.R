#!/usr/bin/Rscript

dir.dtree.postprob.moment1 <- function(s,t,k,n) {
    sigma <- 1
    if (k>0) { # If the ballots have any candidates in common...
        for (l in (n-k+1):n) {
            sigma <- sigma * (factorial(l)*s+t)/(l*factorial(l)*s+t)
        }
    }
    if (k < (n-1)) { # If it ever deviates...
        delta <- s/((n-k)*factorial(n-k)*s + t)
    } else {
        delta <- 1
    }
    gamma <- 1/factorial(n-k-1)^2
    return(sigma * delta * gamma)
}
dir.dtree.postprob.moment2 <- function(s,t,k,n) {
    sigma.2 <- 1
    if (k>0) { # If the ballots have any candidates in common...
        for (l in (n-k+1):n) {
            sigma.2 <- sigma.2 * (factorial(l)*s+t)/(l*factorial(l)*s+t) * (factorial(l)*s+t+1)/(factorial(l)*l*s+t+1)
        }
    }
    delta.2 <- 1
    if (k < (n-1)) { # If it ever deviates...
        delta.2 <- factorial(n-k)*s/((n-k)*factorial(n-k)*s + t) * (factorial(n-k)*s+1)/((n-k)*factorial(n-k)*s + t+1)
    }
    gamma.2 <- 1
    if (k > (n-1)) { # If it ever deviates...
        for (l in 2:(n-k-1)){
            gamma.2 <- gamma.2 * (factorial(l)*s+1)/(l*(l*factorial(l)*s + 1))
        }
    }
    return(sigma.2 * delta.2 * gamma.2)
}

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

# Moment functions
dir.postprob.moment1 <- function(s,t,k,n) {
    if (k==n-1) {
        return((s+t)/(factorial(n)*s + t))
    } else {
        return(s/(factorial(n)*s + t))
    }
}

dtree.postprob.moment1 <- function(s,t,k,n) {
    sigma <- 1
    if (k>0) { # If the ballots have any candidates in common...
        for (l in (n-k+1):n) {
            sigma <- sigma * (s+t)/(l*s+t)
        }
    }
    if (k < (n-1)) { # If it ever deviates...
        delta <- s/((n-k)*s + t)
    } else {
        delta <- 1
    }
    gamma <- 1/factorial(n-k-1)
    return(sigma * delta * gamma)
}

dir.postprob.moment2 <- function(s,t,k,n) {
    if (k==n-1) {
        return((s+t)/(factorial(n)*s + t) * (s+t+1)/(factorial(n)*s+t+1))
    } else {
        return(s/(factorial(n)*s + t) * (s+1)/(factorial(n)*s + t + 1))
    }
}

dtree.postprob.moment2 <- function(s,t,k,n) {
    sigma.2 <- 1
    if (k>0) { # If the ballots have any candidates in common...
        for (l in (n-k+1):n) {
            sigma.2 <- sigma.2 * (s+t)/(l*s+t) * (s+t+1)/(l*s+t+1)
        }
    }
    delta.2 <- 1
    if (k < (n-1)) { # If it ever deviates...
        delta.2 <- s/((n-k)*s + t) * (s+1)/((n-k)*s + t+1)
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
t <- 1
ks <- 0:(n-1)
p.dir.moment1 <- c()
p.dtree.moment1 <- c()
p.dir.moment2 <- c()
p.dtree.moment2 <- c()
for (k in ks) {
    p.dir.moment1 <- c(p.dir.moment1, dir.postprob.moment1(s.dir,t,k,n))
    p.dtree.moment1 <- c(p.dtree.moment1, dtree.postprob.moment1(s,t,k,n))
    p.dir.moment2 <- c(p.dir.moment2, dir.postprob.moment2(s.dir,t,k,n))
    p.dtree.moment2 <- c(p.dtree.moment2, dtree.postprob.moment2(s,t,k,n))
}
png('moments.png', height = 1080, width=1920)
par(mfrow=c(2,2))
plot(
  x = rep(ks, 2),
  y = c(p.dtree.moment1,p.dir.moment1),
  col = c(rep(1,n),rep(2,n)),
  xlab = "k (# of candidates in common with observed ballot)",
  ylab = "log(1st moment of posterior probability, unweighted)"
)
plot(
  x = rep(ks, 2),
  y = c(
     unlist(Map(function(x) max(1,x)*factorial(x),(n-1):0))
         * p.dtree.moment1,
     unlist(Map(function(x) max(1,x)*factorial(x),(n-1):0))
         * p.dir.moment1
  ),
  col = c(rep(1,n),rep(2,n)),
  xlab = "k (# of candidates in common with observed ballot)",
  ylab = "log(1st moment of posterior probability, weighted)"
)
plot(
  x = rep(ks, 2),
  y = c(p.dtree.moment2,p.dir.moment2),
  col = c(rep(1,n),rep(2,n)),
  xlab = "k (# of candidates in common with observed ballot)",
  ylab = "log(2nd moment of posterior probabilities)"
)
plot(
  x = rep(ks, 2),
  y = c(
         p.dtree.moment2 - p.dtree.moment1^2,
         p.dir.moment2 - p.dir.moment1^2
  ),
  col = c(rep(1,n),rep(2,n)),
  xlab = "k (# of candidates in common with observed ballot)",
  ylab = "Log-Variance of posterior probability"
)
dev.off()

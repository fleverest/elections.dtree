#!/usr/bin/Rscript

require(reshape2)
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

# Moment functions
dir.postprob.moment1 <- function(s,k,n,t) {
    if (k==0) {
        return((s+t)/(factorial(n)*s + t))
    } else {
        return(s/(factorial(n)*s + t))
    }
}

dtree.postprob.moment1 <- function(s,k,n,t) {
    sigma <- 1
    if (k<(n-1)) { # If the ballots have any candidates in common...
        for (l in (k+2):n) {
            sigma <- sigma * (s+t)/(l*s+t)
        }
    }
    if (k > 0) { # If it ever deviates...
        delta <- s/((k+1)*s + t)
    } else {
        delta <- 1
    }
    gamma <- 1/factorial(k)
    return(sigma * delta * gamma)
}

dir.postprob.moment2 <- function(s,k,n) {
    if (k==0) {
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
t <- 1
s.dir <- dir.equivparam(s, n) # Equivalent (by variance) dirichlet parameter
ks <- 0:(n-1)
p.dir.moment1 <- c()
p.dtree.moment1 <- c()
p.dir.moment2 <- c()
p.dtree.moment2 <- c()
for (k in ks) {
    p.dir.moment1 <- c(p.dir.moment1, dir.postprob.moment1(s.dir,k,n,t))
    p.dtree.moment1 <- c(p.dtree.moment1, dtree.postprob.moment1(s,k,n,t))
    p.dir.moment2 <- c(p.dir.moment2, dir.postprob.moment2(s.dir,k,n))
    p.dtree.moment2 <- c(p.dtree.moment2, dtree.postprob.moment2(s,k,n))
}

outdf <- data.frame(k=rep(ks,2))
outdf$Prior <- c(rep("Dirichlet Tree",n), rep("Dirichlet",n))
# Regular scale
outdf$logmom.1 <- c(log(p.dtree.moment1),log(p.dir.moment1))
outdf$mom.1.weighted <- c(
     unlist(Map(function(x) max(factorial(x+1)-factorial(x),1),0:(n-1)))
         * p.dtree.moment1,
     unlist(Map(function(x) max(factorial(x+1)-factorial(x),1),0:(n-1)))
         * p.dir.moment1
  )

print(outdf$mom.1)

print(outdf$mom.1.weighted)
print(sum(outdf$mom.1.weighted))
# Log scale
#outdf$logmom.1 <- log(c(p.dtree.moment1,p.dir.moment1))
#outdf$logmom.1.weighted <- log(c(
#     unlist(Map(function(x) max(1,x)*factorial(x),(n-1):0))
#         * p.dtree.moment1,
#     unlist(Map(function(x) max(1,x)*factorial(x),(n-1):0))
#         * p.dir.moment1
#  ))
outdf.long <- melt(outdf, id.vars = c("Prior","k"))
levels(outdf.long$variable) <- c("1st Moment", "Weighted 1st Moment")
png('moments.png', height = 700, width=1000)
ggplot(outdf.long, aes(x=k, y=value, col=Prior)) +
  geom_point() +
  geom_line() +
  facet_wrap(~variable,scales="free") +
  theme(text = element_text(size = 20)) +
  labs(y="Expected log-probability")
dev.off()

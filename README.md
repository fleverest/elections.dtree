# Bayesian IRV Election Auditing with the Dirichlet-Tree Prior.

The work so far in Bayesian auditing literature has employed the Dirichlet prior to the multinomial ballot types. In the case of Ranked-choice voting systems, the ballot-space can grow factorially; in the case of the IRV election, which is used for the House of Representatives in Australia, the number of possible ballots is `n!`, with `n` candidates.

Though the Dirichlet distribution is a simple and effective choice for fewer Ballot types, when this grows it is incredibly difficult to choose appropriate prior parameters. As `n` grows, you would need to select an initial `alpha` parameter at the `1/n!` scale, which is at best significantly limited numerically by floating-point precision.

The Dirichlet-Tree prior distribution introduces a hierarchical Dirichlet structure, consisting of nested Dirichlet distributions over the conditional probabilities on the next-preferences given the prior candidate selections. This improves the prior such that initial parameters need only be chosen at the scale of `1/n`.

In this repository, I have implemented the IRV Dirichlet-Tree structure in a lazy way such that the memory-complexity is bound by `O(n*m)`, where `m` is the number of ballots observed in the audit process, and `n` is the number of participating candidates.


## WIP: Usage


```R
# Initialize a new Dirichlet Tree prior.
dtree <- dirtree.irv(nCandidates = 10, scale = 5., dirichlet = F, seed="seed")

# Sample 1000 ballots from the prior.
ballots <- draw(dtree, 1000)

# Observe the 1000 ballots to obtain a posterior.
update(dtree, ballots)

# Evaluate 100 random elections (2000 ballots each) from the prior without using the initial ballots.
samplePosterior(dtree, nElections = 100, nBallots = 2000, useObserved = F)

# Evaluate again but starting with the initial ballots.
samplePosterior(dtree, nElections = 100, nBallots = 2000, useObserved = T)

# Change the prior scale.
dtree$scale <- 1.

# Switch to a Dirichlet Prior.
dtree$isDirichlet <- T

# Reset to the prior, removing observed data.
clear(dtree)
```


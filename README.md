# Bayesian IRV Election Auditing with the Dirichlet-Tree Prior.

The work so far in Bayesian auditing literature has employed the Dirichlet prior to the multinomial ballot types. In the case of Ranked-choice voting systems, the ballot-space can grow factorially; in the case of the IRV election, which is used for the House of Representatives in Australia, the number of possible ballots is `n!`, with `n` candidates.

Though the Dirichlet distribution is a simple and effective choice for fewer Ballot types, when this grows it is incredibly difficult to choose appropriate prior parameters. As `n` grows, you would need to select an initial `alpha` parameter at the `1/n!` scale, which is at best significantly limited numerically by floating-point precision.

The Dirichlet-Tree prior distribution introduces a hierarchical Dirichlet structure, consisting of nested Dirichlet distributions over the conditional probabilities on the next-preferences given the prior candidate selections. This improves the prior such that initial parameters need only be chosen at the scale of `1/n`.

In this repository, I have implemented the IRV Dirichlet-Tree structure in a lazy way such that the memory-complexity is bound by `O(n*m)`, where `m` is the number of ballots observed in the audit process, and `n` is the number of participating candidates.


## WIP: Usage

Eventually this will have a complete S4 interface, along with several methods for tasks such as evaluating election outcomes.


```R
dtree <- new(RcppDirichletTreeIRV, nCandidates=5, scale=1, treeType="dirichlettree", seed="seed1234")
s10 <- dtree$sample(nBallots=10)
dtree$update(s10)
dtree$samplePosterior(100,100,s10)
```


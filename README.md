# Bayesian IRV Election Auditing with the Dirichlet-Tree Prior.
  <!-- badges: start -->
  [![R-CMD-check](https://github.com/fleverest/dirtree-elections/actions/workflows/R-CMD-check.yaml/badge.svg)](https://github.com/fleverest/dirtree-elections/actions/workflows/R-CMD-check.yaml)
  [![codecov](https://codecov.io/gh/fleverest/dirtree-elections/branch/master/graph/badge.svg?token=V36WLNA1BY)](https://codecov.io/gh/fleverest/dirtree-elections)
  <!-- badges: end -->

## About the project.

#### Why?

The work so far in Bayesian auditing literature has employed the Dirichlet prior to the multinomial ballot types. In the case of Ranked-choice voting systems, the ballot-space can grow factorially; in the case of the IRV election, which is used for the House of Representatives in Australia, the number of possible ballot types is `n!` with `n` candidates.

Though the Dirichlet distribution is a simple and effective choice for fewer Ballot types, when this grows it is incredibly difficult to choose appropriate prior parameters. As `n` grows, you would need to select an initial `a0` parameter at the `1/n!` scale, which is _at best_ significantly limited numerically by floating-point precision and _on average_ an incredibly over-informative prior distribution.

The Dirichlet-Tree prior distribution introduces a hierarchical Dirichlet structure, consisting of nested Dirichlet distributions over the conditional probabilities on the next-preferences given the prior candidate selections. This improves the prior such that initial parameters need only be chosen at the scale of `1/n` in order to perform well.

#### How?

In this repository, I have implemented the IRV Tree structure in a lazy way such that the memory-complexity is bounded by `O(n*m)`, where `m` is the number of ballots observed in the audit process, and `n` is the number of participating candidates.

To avoid loading all tree nodes into memory for sampling, I chose to implement a recursive sampling strategy which can generate Dirichlet-Tree samples starting from any uninitialized point in the tree. This works well for IRV ballot structures, since the tree structure is easily navigated given a ballot. In order to support different tree structures or other elections, this should be implemented - and anyone designing a new tree structure for this should consider it carefully - this way of sampling is what makes the method computationally feasible for elections with more candidates.


## Usage

#### S3 interface
```R
# Initialize a new Dirichlet Tree for IRV elections with
# 26 candidates (named A through Z), requiring exactly 3 preferences
# specified for a valid ballot, and using a prior parameter of 1.5.
dtree <- dirtree(
  candidates = LETTERS,
  min_depth = 3,
  max_depth = 3,
  a0 = 1.5
)

# Create some generic ballots
ballots <- ranked_ballots(list(c("A", "B"), c("C", "D")), candidates = LETTERS)

# Sample 1000 random ballots from the tree.
ballots <- sample_predictive(dtree, 1000)

# Check which candidate wins the simulated election:
social_choice(ballots)

# Shuffle the ballots
ballots <- sample(ballots)

# Observe the first 100 ballots to obtain a posterior Dirichlet Tree.
update(dtree, ballots[1:100])

# Evaluate 100 random election outcomes by:
#  1. sampling 900 ballots from the posterior predictive distribution, and
#  2. evaluating the outcome of the 900 total sampled ballots, plus the 100 observed.
sample_posterior(dtree, n_elections = 100, n_ballots = 1000)

# Change the prior parameter and compare the posterior winning probabilities.
dtree$a0 <- 1.
sample_posterior(dtree, n_elections = 100, n_ballots = 1000)

# Do it again, with a Dirichlet prior this time!
dtree$vd <- TRUE
sample_posterior(dtree, n_elections = 100, n_ballots = 1000)

# Reset the distribution to the prior, removing observed data. This is equivalent to
# creating a new tree with the same parameters.
reset(dtree)
```

#### R6 interface
```R
# Initialize a new Dirichlet Tree for IRV elections with
# 26 candidates (named A through Z), requiring exactly 3 preferences
# specified for a valid ballot, and using a prior parameter of 1.5.
dtree <- dirichlet_tree$new(
  candidates = LETTERS,
  min_depth = 3,
  max_depth = 3,
  a0 = 1.5
)

# Sample 1000 random ballots from the tree.
ballots <- dtree$sample_predictive(1000)

# Check which candidate wins the simulated election:
social_choice(ballots)

# Shuffle the ballots
ballots <- sample(ballots)

# Observe the first 100 ballots to obtain a posterior Dirichlet Tree.
dtree$update(ballots[1:100])

# Evaluate 100 random election outcomes by:
#  1. sampling 900 ballots from the posterior predictive distribution, and
#  2. evaluating the outcome of the 900 total sampled ballots, plus the 100 observed.
dtree$sample_posterior(n_elections = 100, n_ballots = 1000)

# Change the prior parameter and compare the posterior winning probabilities.
dtree$a0 <- 1.
dtree$sample_posterior(n_elections = 100, n_ballots = 1000)

# Do it again, with a Dirichlet prior this time!
dtree$vd <- TRUE
dtree$sample_posterior(n_elections = 100, n_ballots = 1000)

# Reset the distribution to the prior, removing observed data. This is equivalent to
# creating a new tree with the same parameters.
dtree$reset()

# Additionally, the R6 interface allows you to chain commands:
dtree <- dirichlet_tree$new(
  candidates = LETTERS
)$update(
  ranked_ballots(
    list(
      c("A", "B", "C"),
      c("D", "E", "F")
    ),
    candidates = LETTERS
  )
)
```

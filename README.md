# Bayesian IRV Election Auditing with the Dirichlet-tree Prior.
  <!-- badges: start -->
  [![R-CMD-check](https://github.com/fleverest/elections.dtree/actions/workflows/R-CMD-check.yaml/badge.svg)](https://github.com/fleverest/elections.dtree/actions/workflows/R-CMD-check.yaml)
  [![codecov](https://codecov.io/gh/fleverest/elections.dtree/branch/master/graph/badge.svg?token=V36WLNA1BY)](https://codecov.io/gh/fleverest/elections.dtree)
  <!-- badges: end -->

## About the project.

#### Why?

The work so far in Bayesian auditing literature has employed the Dirichlet prior to the multinomial ballot types. In the case of Ranked-choice voting systems, the ballot-space can grow factorially; in the case of the IRV election, which is used for the House of Representatives in Australia, the number of possible ballot types is `n!` with `n` candidates.

The Dirichlet distribution is a simple and effective choice if the number of ballot types is small, but it becomes problematic when this number gets large. As `n` grows, the prior concentration parameters (defined by `a0` in our implementation) would need to be on the order of `1/n!` to ensure the prior is not overly informative. If `n` is large enough, we can even push the limits of floating-point precision. Also, the fact that this varies by `n` at all is inconvenient. A more practical alternative is given by the Dirichlet-tree distribution, which we implement in this package.

The Dirichlet-tree prior distribution introduces a hierarchical Dirichlet structure, consisting of nested Dirichlet distributions over the conditional probabilities on the next-preferences given the previous candidate selections. This improves on the Dirichlet prior such that initial parameters need not depend on `n` to perform well.

#### How?

In this repository, the IRV Tree structure is implemented such that the nodes in the Dirichlet-tree are only initialised when data has been observed below. This allows the memory-complexity to be `O(n*m)`, where `m` is the number of ballots observed in the audit process, and `n` is the number of participating candidates. Without such lazy evaluation, the memory-complexity is necessarily `O(n!)`. Hence, our implementation of the Dirichlet distribution (based on a reducible Dirichlet-tree structure) enables a larger candidate pool than would be possible using traditional methods.

To sample unseen ballots without loading all nodes into memory, this repository implements a recursive strategy which generates samples starting from any uninitialized point in the tree. This works well for IRV ballot structures, since the permutation-tree structure is easily navigated given a target ballot. In order to support different tree structures or other elections which deal with high cardinality, this should be implemented - and anyone designing a new tree structure for this should consider it carefully.


## Usage

#### S3 interface
```R
# Initialize a new Dirichlet-tree for IRV elections with
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

# Observe the first 100 ballots to obtain a posterior Dirichlet-tree.
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
# Initialize a new Dirichlet-tree for IRV elections with
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

# Observe the first 100 ballots to obtain a posterior Dirichlet-tree.
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

## Installation

#### CRAN

TBD

#### Development

To install the development version of `elections.dtree` from GitHub:
```R
# install.packages("remotes")
remotes::install_github("fleverest/elections.dtree")
```

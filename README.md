# Ranked Voting Election Audits with Dirichlet-Trees

Perform ballot-polling Bayesian audits for ranked voting elections using a Dirichlet-tree prior distribution.

  <!-- badges: start -->
  [![CRAN_Status_Badge](https://www.r-pkg.org/badges/version/elections.dtree)](https://cran.r-project.org/package=elections.dtree)
  [![pkgdown](https://github.com/fleverest/elections.dtree/workflows/pkgdown/badge.svg)](https://fleverest.github.io/elections.dtree/)
  [![R-CMD-check](https://github.com/fleverest/elections.dtree/actions/workflows/R-CMD-check.yaml/badge.svg)](https://github.com/fleverest/elections.dtree/actions/workflows/R-CMD-check.yaml)
  [![Codecov test coverage](https://codecov.io/gh/fleverest/elections.dtree/branch/main/graph/badge.svg)](https://app.codecov.io/gh/fleverest/elections.dtree?branch=main)
  <!-- badges: end -->


## Installation

#### CRAN

To install the latest minor release of `elections.dtree` from CRAN:
```R
install.packages("elections.dtree")
```

#### Development

To install the development release of `elections.dtree` from GitHub:
```R
# install.packages("remotes")
remotes::install_github("fleverest/elections.dtree")
```

## About the project

#### Why?

Bayesian audits of elections typically employ a Dirichlet prior, conjugate to
a multinomial distribution for the observed ballots.  For ranked voting systems,
the number of ballot types can grow factorially.  In the case of instant-runoff
voting (IRV), a popular type of ranked voting system, the number of possible
ballot types is `n!` with `n` candidates (assuming all candidates are ranked;
it is even greater if partially completed ballots are permitted).

The Dirichlet distribution is a simple and effective choice if the number of
ballot types is small, but becomes problematic when this number gets large.  As
`n` grows, the prior concentration parameters (defined by `a0` in our
implementation) need to be on the order of `1 / n!` to ensure the prior is not
overly informative.  If `n` is large enough, this may be smaller than the
available precision.  Also, the fact that this varies by `n` is inconvenient.
A more practical alternative is given by the Dirichlet-tree distribution,
which we implement in this package.

The Dirichlet-tree distribution consists of nested Dirichlet distributions,
arranged hierarchically in a tree structure.  The structure represents the
preference ordering of the candidates.  Branches coming out of each node
correspond to choices for which candidate to select as the next preferred, and
nodes represent a ranking of candidates (a complete ranking for leaf nodes,
and an incomplete ranking for internal nodes).  We place a Dirichlet
distribution at each node, to model the conditional split of preferences at
that node.  The structure as a whole then defines a Dirichlet-tree
distribution.  Just like the Dirichlet, it is conjugate to a multinomial
distribution.  Also, the Dirichlet-tree is a generalisation, including
a Dirichlet as a special case.

Using the Dirichlet-tree as a prior distribution allows it to scale efficiently
to large `n`, and does not require setting the concentration parameters (`a0`)
to values that depend on `n` to perform well.
depend on `n` to perform well.

#### How?

In this package, the tree structure is implemented such that the nodes are only
initialised when they appear in the observed ballot data (i.e. when at least
one of the ballots includes a preference sequence that is represented by that
node).  This allows the memory-complexity to be `O(n*m)`, where `m` is the
number of ballots observed in the audit process.

Without such lazy evaluation, the memory-complexity is necessarily `O(n!)`.
Hence, our implementation of the Dirichlet distribution (based on a reducible
Dirichlet-tree structure) enables a larger set of candidates than would be
possible using traditional methods.

To sample unseen ballots without loading all nodes into memory, this repository
implements a recursive strategy that generates samples starting from any
uninitialized point in the tree.  This works well for IRV ballot structures,
since the permutation-tree structure is easily navigated given a target ballot.

Currently, only IRV elections have been implemented, but other ranked voting
systems could be supported by implementing the corresponding social choice
function.

In order to support different tree structures or other elections which deal
with high cardinality, a similar recursive strategy for sampling should be
developed (and anyone designing a new tree structure should consider this
carefully).


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

# Create some generic ballots.
ballots <- ranked_ballots(list(c("A", "B"), c("C", "D")), candidates = LETTERS)

# Sample 1000 random ballots from the tree.
ballots <- sample_predictive(dtree, 1000)

# Check which candidate wins the simulated election.
social_choice(ballots)

# Shuffle the ballots.
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

# Do it again, with a Dirichlet prior using all available threads.
dtree$vd <- TRUE
sample_posterior(dtree, n_elections = 100, n_ballots = 1000, n_threads = Inf)

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

# Check which candidate wins the simulated election.
social_choice(ballots)

# Shuffle the ballots.
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

# Do it again, with a Dirichlet prior using all available threads.
dtree$vd <- TRUE
dtree$sample_posterior(n_elections = 100, n_ballots = 1000, n_threads = Inf)

# Reset the distribution to the prior, removing observed data. This is equivalent to
# creating a new tree with the same parameters.
dtree$reset()

# Additionally, the R6 interface allows you to chain commands.
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

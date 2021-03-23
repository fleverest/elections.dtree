require('dirtree.elections')
require('readr')

NUM_ELECTIONS <- 25

num_candidates <- 6

# In this script, we load two elections, one close between two candidates,
# and the other uniform across all ballots.
close.election <- read_lines('simulations/close_election_6candidates_6wins.txt')
uniform.election <- read_lines('simulations/uniform_election_6candidates_4wins.txt')

# We wish to evaluate the effects of altering the initial 'scale' of the
# dirichlet and dirichlet-tree distributions. We expect that as the scale
# varies, the priors will be more or less informative at the beginning of the
# audit. This hyperparameter can be used to tune the models, and it will be
# easier to compare the two.

# To compare these, we wish to observe the progress of the audit at 5 stages,
# Plots will be generated at 1, 10, 100, 500 and 900 ballot counts, and we will
# plot the determined probability masses for each candidate winning. We complete
# these plots using scale factors ranging across the same values.

outcomes <- 1:num_candidates

genPlot <- function(ballots, scale) {
    # initialize trees
    tree.constant <- dirtree.irv(candidates=num_candidates, ptype='constant', scale=scale)
    tree.dirichlet <- dirtree.irv(candidates=num_candidates, ptype='dirichlet', scale=scale)

    # Update
    print('Updating with first 100 ballots')
    dirtree.update(tree.constant,ballots[1:100])
    dirtree.update(tree.dirichlet,ballots[1:100])
    
    # Calculate probabilities
    outcomes.constant100 <- table(factor(
        dirtree.montecarlo(
            tree.constant,
            900,
            NUM_ELECTIONS
        ),
        levels=outcomes
    ))
    outcomes.dirichlet100 <- table(factor(
        dirtree.montecarlo(
            tree.dirichlet,
            900,
            NUM_ELECTIONS
        ),
        levels=outcomes
    ))

    # Update
    print('Updating with first 500 ballots')
    dirtree.update(tree.constant,ballots[101:500])
    dirtree.update(tree.dirichlet,ballots[101:500])
    
    # Calculate probabilities
    outcomes.constant500 <- table(factor(
        dirtree.montecarlo(
            tree.constant,
            500,
            NUM_ELECTIONS
        ),
        levels=outcomes
    ))
    outcomes.dirichlet500 <- table(factor(
        dirtree.montecarlo(
            tree.dirichlet,
            500,
            NUM_ELECTIONS
        ),
        levels=outcomes
    ))

    # Update
    print('Updating with first 900 ballots')
    dirtree.update(tree.constant,ballots[501:900])
    dirtree.update(tree.dirichlet,ballots[501:900])
    
    # Calculate probabilities
    outcomes.constant900 <- table(factor(
        dirtree.montecarlo(
            tree.constant,
            100,
            NUM_ELECTIONS
        ),
        levels=outcomes
    ))
    outcomes.dirichlet900 <- table(factor(
        dirtree.montecarlo(
            tree.dirichlet,
            100,
            NUM_ELECTIONS
        ),
        levels=outcomes
    ))

    # make plots
    barplot(
        rbind(outcomes.dirichlet100, outcomes.constant100),
        beside=T,
        legend=c("dirichlet100","constant100")
    )
    barplot(
        rbind(outcomes.dirichlet500, outcomes.constant500),
        beside=T,
        legend=c("dirichlet500","constant500")
    )
    barplot(
        rbind(outcomes.dirichlet900, outcomes.constant900),
        beside=T,
        legend=c("dirichlet900","constant900")
    )

}

# The ground truth: candidate 4 wins the uniform election, and candidate 1 wins
# the close election.

# First we sample the 500 ballots to be counted:
ballots.close <- sample(close.election,900)
ballots.uniform <- sample(uniform.election,900)

png('close6.png', width=3000, height=3000)
par(mfrow=c(4,3))

for (scale in c(1,100,1000,10000)) {
    print(scale)
    genPlot(ballots.close,scale)
}

dev.off()

png('uniform6.png', width=3000, height=3000)
par(mfrow=c(4,3))

for (scale in c(1,100,1000,10000)) {
    print(scale)
    genPlot(ballots.uniform,scale)
}
dev.off()

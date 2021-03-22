require('dirtree.elections')

# Consider an election with 4 candidates and a total of 1000 ballots cast,
# 100 of which have been counted so far:
root.ones <- dirtree(type="irv", ptype="ones", candidates=4)

# Simulate in 4 steps for non-uniform ballots, saving the same ballots for later
election.ballots <- c()
for(i in 1:4){
    # simulate 25 ballots and update
    new.ballots <- dirtree.simulate(root.ones, 25)
    election.ballots <- c(election.ballots, new.ballots)
    dirtree.update(root.ones, new.ballots)
}

# Now, we have 900 remaining ballots to count, and we will simulate 1000
# complete elections to determine the distribution of the candidate victories
outcomes_ones <- table(dirtree.montecarlo(root.ones,900,1000))


# Do it all again but with the Dirichlet prior
root.dir <- dirtree(type="irv", ptype="dirichlet", candidates=4)
dirtree.update(root.dir, election.ballots)

outcomes_dir <- table(dirtree.montecarlo(root.dir,900,1000))


# Generate the bar plot for the victory distributions
png(file="barplots_100.png")
barplot(rbind(outcomes_dir, outcomes_ones), beside=T, legend=c("dirichlet","ones"))
dev.off()


# Do it again but with 1 initial ballot
root.ones <- dirtree(type="irv", ptype="ones", candidates=4)

ballot <- dirtree.simulate(root.ones,1)
dirtree.update(root.ones, ballot)
outcomes_ones <- table(dirtree.montecarlo(root.ones,999,1000))
root.dir <- dirtree(type="irv",ptype="dirichlet", candidates=4)
dirtree.update(root.dir,ballot)
outcomes_dir <- table(dirtree.montecarlo(root.dir,999,1000))

# Generate plot:
png(file="barplots_1.png")
barplot(rbind(outcomes_dir, outcomes_ones), beside=T, legend=c("dirichlet","ones"))
dev.off()

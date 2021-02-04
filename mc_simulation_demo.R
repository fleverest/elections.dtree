require('dirtree.elections')

# Consider an election with 4 candidates and a total of 1000 ballots cast,
# 100 of which have been counted so far:
root <- dirtree(type="irv", ptype="ones", candidates=4)

# Simulate in 4 steps for non-uniform ballots:
for(i in 1:4){
    # simulate 25 ballots and update
    dirtree.update(root, dirtree.simulate(root, 25))
}

# Now, we have 900 remaining ballots to count, and we will simulate 1000
# complete elections to determine the distribution of the candidate victories
outcomes_ones <- table(dirtree.montecarlo(root,900,1000))


# Do it all again but with the Dirichlet prior
root <- dirtree(type="irv", ptype="dirichlet", candidates=4)
for(i in 1:4){
    dirtree.update(root, dirtree.simulate(root, 25))
}
outcomes_dir <- table(dirtree.montecarlo(root,900,1000))


# Generate the bar plots for the victory distributions
png(file="barplots.png")
barplot(rbind(outcomes_dir, outcomes_ones), beside=T, legend=c("dirichlet","ones"))
dev.off()

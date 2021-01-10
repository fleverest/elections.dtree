electionsize <- 1000
counted <- 100
numelections <- 1000

tree <- dirtree(candidates=3)

dirtree.update(tree, dirtree.simulate(tree, counted))

print(tree, "ballots")

uncounted <- electionsize-counted

outcomes <- c()
for( i in 1:numelections ){
    treeclone <- Clone(tree)
    dirtree.update(treeclone,dirtree.simulate(treeclone,uncounted))
    outcomes <- c(outcomes, dirtree.irv.socialchoice(treeclone))
}

png("election_distribution.png")

barplot(c(sum(outcomes==1),sum(outcomes==2),sum(outcomes==3)))

dev.off()
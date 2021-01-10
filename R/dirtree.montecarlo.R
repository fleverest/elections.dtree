dirtree.montecarlo <- function(
    tree, # Election tree to complete
    n, # Number of ballots left to complete
    m # Number of elections to simulate
){
    outcomes <- c()
    for( i in 1:m ){
        treeclone <- Clone(tree)
        dirtree.update(treeclone,dirtree.simulate(treeclone,n))
        outcomes <- c(outcomes, dirtree.irv.socialchoice(treeclone))
    }

}

# In this function I apply an analytical solution (potentially inefficient) to 
# find the probability that a candidate receives the least next-preference votes
dirtree.montecarlo.analytical <- function(
    tree, # Election tree to complete
    n, # Number of ballots left
    m, # Number of elections to simulate
    eliminated # Candidates which have been eliminated
){

}
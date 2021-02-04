# Simulate completion of entire elections from an updated dirichlet tree
dirtree.montecarlo <- function(
    tree, # Election tree to complete
    n, # Number of ballots left to sample
    m # Number of elections to simulate
){
    outcomes <- c()
    for( i in 1:m ){
        tree.clone <- Clone(tree)
        dirtree.update(tree.clone,dirtree.simulate(tree.clone,n))
        outcomes <- c(outcomes, dirtree.irv.socialchoice(tree.clone))
    }
    return(outcomes)
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
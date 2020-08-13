# This Function creates a new ballot tree for IRV elections.
irvElectionTree = function(
    candidates = 5, # the number of candidates in the election
    ballots    = 1000 # the number of ballots to be cast
){
    #ensure required packages are available
    if(!require(data.tree)){
        stop("data.tree package is required")
    }
    
}
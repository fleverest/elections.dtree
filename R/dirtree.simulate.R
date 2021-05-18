# Simulate from a dirichlet tree
# Recursively splits the remaining samples according to a dirichlet multinomial
# sample at each node.
dirtree.simulate <- function(
    node,           # Node to simulate from. This should be 'root'
                    # if you want to sample across all possible ballots.
    n,              # Number of ballots to simulate
    format='counts' # Indicates whether unique ballots have a ':count'
                    # appended, or if it is a list with duplicated ballots.
                    # Can be 'counts' or 'duplicated'.
){
    # Requires the extraDistr package for dirichlet-multinomial sampling
    if(!require('extraDistr')){
        stop("Sampling requires the `extraDistr` package!")
    }
    # terminate at leaf nodes or when n is 0
    if(n==0 || length(node$children)==0){
        if (format=='counts' && n!=0){
            return(paste(node$name,':',n))
        } else if (format=='duplicated') {
            return(rep(node$name,n))
        }
        return()
    }

    # get alphas of the children
    alphas <- sapply(node$children, function(child) child$alpha)
    # sample n times, with new dirichlet sample each time (otherwise use 1,n)
    dat <- colSums(rdirmnom(n, 1, alphas))

    # sample at each child node, corresponding to the number of samples which
    # continue down that subtree
    results <- c()

    for(i in 1:length(node$children)){
        child.result <- dirtree.simulate(n = dat[i], node = node$children[[i]])
        results <- c(results, child.result)
    }

    return(results)
}

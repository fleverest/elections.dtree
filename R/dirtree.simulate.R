# Simulate from a dirichlet tree
# Recursively splits the remaining samples according to a dirichlet multinomial
# sample at each node.
dirtree.simulate <- function(
    node, # Node to simulate from. This should be 'root' if you want to sample 
          # across all possible ballots.
    n    # Number of ballots to simulate
){
    # Requires the extraDistr package for dirichlet-multinomial sampling
    if(!require('extraDistr')){
        stop("Sampling requires the `extraDistr` package!")
    }
    # terminate at leaf nodes
    if(length(node$children)==0){
        return(paste(node$name,n,sep=':'))
    }

    # sample n times, with new dirichlet sample each time (otherwise use 1,n)
    dat <- colSums(rdirmnom(n, 1, node$alpha))

    # sample at each child node, corresponding to the number of samples which
    # continue down that subtree
    results <- c()
    if( length(node$children)>0 ){
        for(i in 1:length(node$children)){
            result <- dirtree.simulate(n = dat[i], node = node$children[[i]])
            results <- c(results, result)
        }
    }
    return(results)
}
# this function takes an IRV election tree and a number of ballots as arguments
# and simulates that number of ballots, currently uniformly.
irvSimulate <- function(
    root, # the election root node
    n = 1000 # number of ballots to simulate on tree
){

    #ensure extraDistr is available
    if(!require(extraDistr)) {
        stop("this function requires the extraDistr package")
    }

    # randomLeaf function takes a tree arg and selects a random leaf
    randomSamples <- function(n, node) {

        node$ballots = node$ballot + n

        if(length(node$children)==0){
            return()
        }

        params <- node$alpha

        # Get n samples from dirichlet multinomial
        dat <- rdirmnom(n,1,params)

        # continue sampling at each subtree
        i=1
        for(m in dat){
            randomLeaf(m,node$children[i])
            i = i + 1
        }
    }

    return(
        randomSamples(n, root)
    )
}
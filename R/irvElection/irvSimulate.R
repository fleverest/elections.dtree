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
    randomLeaf <- function(node) {
        height <- node$candidates
        # Specifies a random path (by child indices)
        path <- mapply(function(x) rdunif(1,1,x), height:1)

        # traverses tree to the leaf
        for( num in path ){
            node <- node$children[[num]]
        }

        # returns the leaf
        return(node)
    }


    # Repeat n times
    for( i in 1:n ) {

        leaf <- randomLeaf(root)
        leaf$ballots = leaf$ballots + 1

    }
}
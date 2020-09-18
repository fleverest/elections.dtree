# This Function creates a new ballot tree for IRV elections.
irvElectionTree = function(
    candidates = 5
){
    #ensure required packages are available
    if(!require(data.tree)){
        stop("data.tree package is required")
    }

    # Builds the a subtree under node with all possible orderings of candidates
    buildSubTree <- function(node, vecCandidates){

        node$Set(ballots = 0)

        # stop building nodes at leaf
        if( length(vecCandidates) == 1 ){
            return(1)
        }

        params = c()
        for( candidate in vecCandidates ){
            # add a child to the parent for the candidate
            child <- node$AddChild(paste(node$name, candidate, sep=''))

            # build the subtree using only the remaining candidates
            params = c(
                params, 
                buildSubTree(
                    child, 
                    vecCandidates[vecCandidates != candidate]
                )
            )
        }

        node$Set(alpha = params)

        return(sum(params))

    }

    # Create the root node
    root <- Node$new()
    # Store number of candidates in root node for efficient access
    root$Set(candidates = candidates)
    vecCandidates = 1:candidates

    # Build the tree
    buildSubTree(root, vecCandidates)
    
    return(root)
}
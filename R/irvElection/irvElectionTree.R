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
        # do nothing at leaf
        if( length(vecCandidates) == 0 ){
            node$Set(ballots = 0)
            return()
        }
        
        for( candidate in vecCandidates ){
            # add a child to the parent for the candidate
            child <- node$AddChild(paste(node$name, candidate))
            # build the subtree using only the remaining candidates
            buildSubTree(child, vecCandidates[vecCandidates != candidate])
        }
    }

    # Create the root node
    root <- Node$new()
    vecCandidates = 1:candidates

    # Build the tree
    buildSubTree(root, vecCandidates)
    
    return(root)
}
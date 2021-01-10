# Builds the a subtree under node with all possible orderings of candidates
buildSubTree <- function(node, vecCandidates, ptype){

    node$Set(ballots = 0)

    if(ptype == "ones"){
        node$alpha = 1
    } else if(ptype == "dirichlet"){
        node$alpha = sum(sapply(node$children, function(child) child$alpha))
    } else{
        stop("Invalid value for `ptype`: expected either 'ones' or 'dirichlet'")
    }

    # stop building nodes at leaf
    if( length(vecCandidates) == 1 ){
        return(1)
    }

    params = c()
    for( candidate in vecCandidates ){
        # add a child to the parent for the candidate
        name <- paste(node$name, candidate, sep='')
        child <- node$AddChild(name)
        child$name <- name

        # build the subtree using only the remaining candidates
        buildSubTree(
            child, 
            vecCandidates[vecCandidates != candidate],
            ptype
        )
    }
}

# This Function creates a new ballot tree for IRV elections.
dirtree.irv = function(
    candidates = 5,
    ptype = "ones"
){
    #ensure required packages are available
    if(!require(data.tree)){
        stop("data.tree package is required")
    }

    # Create the root node
    root <- Node$new('.')
    root$type <- 'irv'

    # Store number of candidates in root node for efficient access
    root$Set(candidates = candidates)
    vecCandidates = 1:candidates

    # Build the tree
    buildSubTree(root, vecCandidates, ptype)
    
    return(root)
}
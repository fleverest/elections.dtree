addtrees <- function(
    election.tree,  # the Election Tree
    sub.tree   # subtree of ballots to be split
) {
    tree <- Node$new() # New tree to be the sum of the others
    
    # Ballots at root equal to sum of subtree and election tree root ballots
    if( !is.null(sub.tree$ballots) ){
        tree$ballots <- election.tree$ballots + sub.tree$ballots
    } else {
        tree$ballots <- election.tree$ballots
    }

    # Name follows election tree format
    tree$name <- election.tree$name

    # Add the tree ballots at the children recursively
    for( child in election.tree$children ){
        tree$AddChild(addtrees( # Add the subtree to the rest of the election tree
            sub.tree$Climb(name=paste(
                str_sub(sub.tree$name,1,1), # first char, corresponding to losing party
                str_sub(child$name,2), # rest of path
                sep=""
                )),
            child
        ))
    }
    return(tree)
}

majorityCandidate <- function(tree) {
    total <- tree$ballots
    
    # Temporary fix to approximately determine majority candidate based on
    # incorrect social choice procedure (smaller subtrees ignored)
    if( length(tree$children)==2 ){
        if( tree$children[[1]]$ballots >= tree$children[[2]]$ballots ){
            return( tree$children[[1]]$name )
        } else {
            return( tree$children[[2]]$name )
        }
    }


    for( child in tree$children ){
        if( child$ballots >= ceiling(total/2) ){
            return( child$name )
        }
    }
    return( "" )
}

dirtree.irv.socialchoice <- function(
    tree # completed tree of ballots to decide the election winner
) {
    if(!require("stringr")){
        stop("The social choice function requires `stringr` to evaluate election results.")
    }

    # Proceed with social choice function until majority candidate is identified
    while( (cand <- majorityCandidate(tree))=="" ){
        minindex <- which.min(
            Map(function(n) n$ballots, tree$children)
        )[[1]]
        # Get subtree
        sub.tree <- tree$children[[minindex]]
        # prune the current losing candidate
        Prune(tree, function(node) str_sub(node$name,-1)!=minindex)
        # prune the redundant leaves
        Prune(tree, function(n) !isLeaf(n))
    }

    # Return the majority candidate
    return(cand)
}
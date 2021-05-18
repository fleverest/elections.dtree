last.candidate <- function( str ){
    cands <- strsplit(str, split = "\\.")[[1]]
    last <- cands[length(cands)]

    if( is.na(last) ){
        return( '' )
    } else {
        return( last )
    }

}

eliminate.candidate <- function(
    dtree, # dirtree to eliminate from
    candidate # candidate to eliminate
){
    # Prune the candidate branches and distribute
#    dtree$Do(
#        function(n) distribute.nextpref(n),
#        filterFun = function(n) candidate==last.candidate(n$name),
#        traversal="post-order"
#    )
    for (child in dtree$children) {
        if (last.candidate(child$name)==candidate) {
            distribute.nextpref(child)
            break
        }
    }
    # Prune the leaves and candidate branches
    Prune(
        dtree,
        function(n) {
            if( isLeaf(n) || candidate==last.candidate(n$name) ){
                return(FALSE)
            } else {
                return(TRUE)
            }
        }
    )
}

# Function to distribute ballots at a node to their next preferences
distribute.nextpref <- function(
    node # node to eliminate at, or to do nothing-
){

    # Node last character is the candidate to be pruned
    candidate = last.candidate(node$name)

    # Remove the candidate from the subtree names
    node$Do(
        function(n){
            n$name <- gsub(paste('.',candidate),'',n$name)
        }
    )

    parent <- node$parent
    # Send params and ballots to next preferences
    for( child in node$children ){
        addTrees(child, parent$children[[child$name]])
    }

}
# Helper for distribute: Adds the alpha parameter and ballot count of the first
# tree to those values on the second.
addTrees <- function(tree1,tree2){
    tree2$ballots <- tree1$ballots + tree2$ballots
    tree2$alpha <- tree1$alpha + tree2$alpha
    for( child in tree1$children ){
        addTrees(child, tree2$children[[child$name]])#add subtree with same name
    }
}

dirtree.irv.socialchoice <- function(
    tree, # completed tree of ballots to decide the election winner
    duplicate = TRUE # Whether or not the tree should be duplicated
                     # in order to preserve the data.
) {

    if (duplicate) {
        # Clone the tree so we can evaluate social choice without mutating it
        tree = Clone(tree)
    }

    # while more than two candidates remain:
    while( length(tree$children) > 2 ){
        # Eliminate minimum candidate:

        # Get the ballot counts for each child (by index)
        ballots = c()
        children = tree$children
        for( i in 1:length(children) ){
            ballots = c(ballots, children[[i]]$ballots)
        }

        eliminate.index = which.min(ballots)
        eliminated.candidate = last.candidate(children[[eliminate.index]]$name)

        eliminate.candidate(
            tree,
            eliminated.candidate
        )

    }

    # Return the winning candidate
    ballots = c()
    children = tree$children
    for( i in 1:length(children) ){
        ballots = c(ballots, children[[i]]$ballots)
    }

    victor.index = which.max(ballots)
    victorious.candidate = last.candidate(children[[victor.index]]$name)


    return(
        victorious.candidate
    )
}

last.candidate <- function( str ){
    out = sub(
        ".",
        "",
        str_extract(str, "\\.[:digit:]+$")
    )

    if( is.na(out) ){
        return( '' )
    } else {
        return( out )
    }
    
}

eliminate.candidate <- function(
    dtree, # dirtree to eliminate from
    candidate # candidate to eliminate
){
    # Prune the candidate branches and distribute
    dtree$Do(
        function(n) distribute.nextpref(n),
        filterFun = function(n) candidate==last.candidate(n$name),
        traversal="post-order"
    )
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

    # clone subtree
    clone <- Clone(node)
    # Remove the candidate from the subtree names
    clone$Do(
        function(n){
            n$name <- gsub(paste('.',candidate),'',n$name)
        }
    )

    parent <- node$parent
    # Send params and ballots to next preferences
    for( child in clone$children ){
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
    tree # completed tree of ballots to decide the election winner
) {

    # Clone the tree so we can evaluate social choice without mutating it
    tree.clone = Clone(tree)

    # while more than two candidates remain:
    while( length(tree.clone$children) > 2 ){
        # Eliminate minimum candidate:

        # Get the ballot counts for each child (by index)
        ballots = c()
        children = tree.clone$children
        for( i in 1:length(children) ){
            ballots = c(ballots, children[[i]]$ballots)
        }

        eliminate.index = which.min(ballots)
        eliminated.candidate = last.candidate(children[[eliminate.index]]$name)


        eliminate.candidate(
            tree.clone,
            eliminated.candidate
        )

    }

    # Return the winning candidate
    ballots = c()
    children = tree.clone$children
    for( i in 1:length(children) ){
        ballots = c(ballots, children[[i]]$ballots)
    }

    victor.index = which.max(ballots)
    victorious.candidate = last.candidate(children[[victor.index]]$name)


    return(
        victorious.candidate
    )
}

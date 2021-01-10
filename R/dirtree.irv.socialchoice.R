dirtree.eliminate.candidate <- function(
    dtree, # dirtree to eliminate from
    candidate # candidate to eliminate
){
    # Prune the candidate branches and distribute
    dtree$Do(
        function(n) distribute.nextpref(n),
        filterFun = function(n) candidate==lastchar(n$name),
        traversal="post-order"
    )
    # Prune the leaves and candidate branched
    Prune(
        dtree,
        function(n) {
            if( isLeaf(n) || candidate==lastchar(n$name) ){
                return(FALSE)
            } else {
                return(TRUE)
            }
        }
    )
}

# This function takes a string input and returns the last character
lastchar <- function(str) {
    return(
        substr(str,nchar(str),nchar(str))
    )
}

# Function to distribute ballots at a node to their next preferences
distribute.nextpref <- function(
    node # node to eliminate at, or to do nothing
){
    print(node$name)

    # Node last character is the candidate to be pruned
    candidate = lastchar(node$name)

    # clone subtree
    clone <- Clone(node)
    # Remove the candidate from the subtree names
    clone$Do(
        function(n){
            n$name <- gsub(candidate,'',n$name)
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

}
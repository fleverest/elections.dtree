# Format ballot count
formatCount <- function(ballotStr){
    split <- strsplit(ballotStr, ":")[[1]]
    ballot <- split[1]
    count <- split[2]

    return(c(ballot, count))
}

# Update the parameters of a tree given a ballot and a count
update <- function(node, ballot, count){
    if(!require('stringr')){
        stop("Requires `stringr` package to navigate dirtrees.")
    }

    node$ballots <- node$ballots + count

    n_children <- length(node$children)


    if(n_children>0){
        for( i in 1:n_children){
            # Check that the ballot path matches
            child <- node$children[[i]]
            if( startsWith(ballot, child$name) ){
                # update appropriate alpha parameter
                node$alpha[i] <- node$alpha[i] + count
                # Continue down the subtree
                update(child, ballot, count)
            }
        }

    }
}

dirtree.update <- function(
    tree, # Tree to update
    data  # ballot data, as a list of "ballot:count" strings
){
    for( ballotStr in data ){
        bc <- formatCount(ballotStr)
        ballot <- bc[1]
        count <- strtoi(bc[2])

        update(tree, ballot, count)

    }
}
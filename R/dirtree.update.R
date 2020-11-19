# Format ballot count
formatCount <- function(ballotStr){
    split <- strsplit(ballotStr, ":")
    ballot <- split[1]
    count <- strtoi(split[2])

    return(c(ballot, count))
}

# Update the parameters of a tree given a ballot and a count
update <- function(node, ballot, count){
    if(!require('stringr')){
        stop("Requires `stringr` package to navigate dirtrees.")
    }

    node$ballots <- node$ballots + count

    if(length(node$children)>0){
        
        for( i in 1:length(node$chilren)){
            # Check that the ballot path matches
            if( str_locate(node$children[i]$name, ballot)[1]==1 ){
                # update appropriate alpha parameter
                node$alpha[i] <- node$alpha[i] + count
                # Continue down the subtree
                update(node$children[i], ballot, count)
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
        count <- bc[2]

        
    }
}
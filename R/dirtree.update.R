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
                child$alpha <- child$alpha + count
                # Continue down the subtree
                update(child, ballot, count)
            }
        }

    }
}

dirtree.update <- function(
    tree, # Tree to update
    data  # ballot data, as a vector of ".1st.2nd.3rd...last" strings
){
    ballot.counts <- list()
    for( ballot in data ){
        if( length(ballot.counts[[ballot]])>0 ){
            ballot.counts[[ballot]] <- ballot.counts[[ballot]] + 1
        } else {
            ballot.counts[[ballot]] <- 1
        }
    }
    
    for( ballot in names(ballot.counts) ){
        update(
            node=tree,
            ballot=ballot,
            count=ballot.counts[[ballot]]
        )
    }
}

# Simulate completion of entire elections from an updated dirichlet tree
dirtree.montecarlo <- function(
    tree, # Election tree to complete
    n, # Number of ballots left to sample
    m # Number of elections to simulate
){
    outcomes <- c()
    for( i in 1:m ){
        tree.clone <- Clone(tree)
        dirtree.update(tree.clone,dirtree.simulate(tree.clone,n))
        outcomes <- c(outcomes, dirtree.irv.socialchoice(tree.clone))
    }
    return(outcomes)
}

dirtree.montecarlo.parallel <- function(
    tree, # Election tree to complete
    n, # Number of ballots left
    m # Number of elections to simulate
){ ###############WIP############################
    if(!require(Rmpi)){
        stop("Need to be in MPI.")
    }

    tree <- mpi.bcast(tree, rank=0)

    n <- mpi.comm.size()

    outcomes <- c()

    for( i in 1:m ) {
        if ( i%%n == mpi.comm.rank() )  {

            tree.clone <- Clone(tree)
            dirtree.update(tree.clone,dirtree.simulate(tree.clone,n))
            outcomes <- c(outcomes, dirtree.irv.socialchoice(tree.clone))

        }
    }

    all_outcomes <- mpi.gather(outcomes, root=0)
    return(all_outcomes)

}

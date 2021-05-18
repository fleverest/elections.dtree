require('dirtree.elections')
require('readr')
if (!is.loaded("mpi_initialize")) {
    library("Rmpi")
    }

print(mpi.universe.size())
ns <- mpi.universe.size() - 1
mpi.spawn.Rslaves(nslaves=ns)

print("Setting simulation variables")

candidates=4
ballots = 10000
unfair.ballots = c('.4.3.2','.1.2.3')
out.close = 'close_election_10.txt'
out.uniform = 'uniform_election_10.txt'

print("Initializing dirichlet trees")

simulate <- function(){
    root <- dirtree( type='irv', candidates=candidates )
    if(mpi.comm.rank()==0){
        outfile <- out.close
        dirtree.update(tree=root,unfair.ballots)
    } else {
        outfile <- out.uniform
    }
    write_lines(
        dirtree.simulate(
            node=root,
            n=ballots
        ),
        outfile
    )
}


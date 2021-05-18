library("Rmpi")
require('dirtree.elections')
require('readr')

candidates=10
ballots = 100000
unfair.ballots <- c(
    '.1.2.3.4.5.6.7.8.9.',
    '.10.9.8.7.6.5.4.3.2'
)

root <- dirtree( type='irv', candidates=candidates )

outfile <- paste(candidates,'candidates_',ballots,'ballots_')

if ( mpi.comm.rank(comm = 0) == 0 ) {
    outfile <- paste(outfile,'uniform_')
} else if ( mpi.comm.rank(comm = 0) == 1 ) {
    outfile <- paste(outfile,'10v1_')
    dirtree.update(root,unfair.ballots)
}

ballots <- dirtree.simulate(
    node=root,
    n=ballots
)

victor <- dirtree.irv.socialchoice(root)

write_lines(
    ballots,
    paste(outfile,victor,'.txt')
)

mpi.barrier(comm = 0)
mpi.finalize()

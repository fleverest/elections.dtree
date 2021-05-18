require('dirtree.elections')
require('readr')
if (!is.loaded("mpi_initialize")) {
    library("Rmpi")
    }

print("Setting simulation variables")

candidates=10
ballots = 1000000

# Simulate an election which is equally dominated by two candidates (1 and 10)
unfair.ballots = c('.10.9.8.7.6.5.4.3.2','.1.2.3.4.5.6.7.8.9')
outfile = '10cand_10v1_'

# Simulate an entirely uniform election
#unfair.ballots = c()
#outfile = '10cand_uniform_'

# Simulate an election with different outcomes depending on the order of
# candidate elimination
unfair.ballots = c('.10.9.8.7.6.5.4.3.2','.1.2.3.4.5.6.7.8.9',
                    '.9.10.8.7.6.5.4.3.2', '.2.1.3.4.5.6.7.8.9')

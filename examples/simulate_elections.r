require('dirtree.elections')
require('readr')

# Helper function to format samples as list of ballots

# We generate two elections with 1000 ballots, one which is close between
# candidates 1 and 2,
close.irv <- dirtree(
    type='irv',
    candidates=6,
    ptype='constant',
    scale=1
)
# and the other which in which ballots are entirely uniform
uniform.irv <- dirtree(
    type='irv',
    candidates=6
)

# we generate the first by updating with two ballots, and then sampling
dirtree.update(
    tree=close.irv,
    data=c(
        '.1.2.3.4.5',
        '.6.5.4.3.2'
    )
)
write_lines(
    dirtree.simulate(
        node=close.irv,
        n=1000
    ),
    'close_election_6.txt'
)

# Then we generate the second election just by sampling the ballots directly
write_lines(
    dirtree.simulate(
        node=uniform.irv,
        n=1000
    ),
    'uniform_election_6.txt'
)
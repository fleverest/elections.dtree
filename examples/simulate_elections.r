require('dirtree.elections')
require('readr')

print("Setting simulation variables")

candidates=10
ballots = 10000
unfair.ballots = c('.10.9.8.7.6.5.4.3.2','.1.2.3.4.5.6.7.8.9')
out.close = 'close_election_10.txt'
out.uniform = 'uniform_election_10.txt'

print("Initializing dirichlet trees")

# We generate two elections with 1000 ballots, one which is close between
# candidates 1 and 2,
close.irv <- dirtree(
    type='irv',
    candidates=candidates,
    ptype='constant',
    scale=1
)
# and the other which in which ballots are entirely uniform
uniform.irv <- dirtree(
    type='irv',
    candidates=candidates
)

# we generate the first by updating with two ballots, and then sampling
print("Updating close election tree with unfair ballots")
dirtree.update(
    tree=close.irv,
    unfair.ballots
)

print("Simulating close election")
write_lines(
    dirtree.simulate(
        node=close.irv,
        n=ballots
    ),
    out.close
)

# Then we generate the second election just by sampling the ballots directly
print("Simulating uniform election")
write_lines(
    dirtree.simulate(
        node=uniform.irv,
        n=ballots
    ),
    out.uniform
)

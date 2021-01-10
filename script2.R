# Dirtree with 3 candidates
root <- dirtree(type="irv", ptype="ones", candidates=3)

# Simulate in 2 steps to sample non-uniform ballots:
# simulate 10 ballots and update
dirtree.update(root, dirtree.simulate(root, 10))
# simulate 10 more and update
dirtree.update(root, dirtree.simulate(root, 10))

# Show tree parameters:
print(root, "alpha")

# candidate 3 will be eliminated:
dirtree.eliminate.candidate(root,'3')
print(root, "alpha")

# we are left with the reduced tree after 3 is eliminated.
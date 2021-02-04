require('dirtree.elections')

# Dirtree with 3 candidates
root <- dirtree(type="irv", ptype="ones", candidates=3)

# Simulate in 2 steps for non-uniform ballots:
# simulate 10 ballots and update
dirtree.update(root, dirtree.simulate(root, 10))
# simulate 10 more and update
dirtree.update(root, dirtree.simulate(root, 10))

# Show tree parameters:
print(root, "alpha")

# Calculate winner via social choice function
dirtree.irv.socialchoice(root)
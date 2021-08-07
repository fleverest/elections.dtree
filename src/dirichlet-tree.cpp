// dirichlet-tree.cpp

#include "dirichlet-tree.hpp"

/* Node class constructor.
 *
 * To initialize a new node, we require the number of
 * remaining candidates to initialize appropriately
 * sized arrays for the child nodes and the corresponding
 * alpha parameter for the distribution over them.
 */
Node::Node(int nChildren_, DirichletTreeIRV *baseTree_)
    : nChildren(nChildren_), baseTree(baseTree_) {

  children = new NodePtr[nChildren]{nullptr};
  alphas = new float[nChildren];
  if (baseTree->getTreeType() == TREE_TYPE_VANILLA_DIRICHLET) {
    for (int i = 0; i < nChildren; ++i) {
      alphas[i] = baseTree->getScale() * baseTree->getFactorials()[nChildren];
    }
  } else if (baseTree->getTreeType() == TREE_TYPE_DIRICHLET_TREE) {
    for (int i = 0; i < nChildren; ++i) {
      alphas[i] = baseTree->getScale();
    }
  }
}

/* Node class destructor.
 *
 * We must delete all child nodes to avoid memory leaks.
 */
Node::~Node() {
  for (int i = 0; i < nChildren; ++i) {
    if (children[i] != nullptr) {
      delete children[i]; // If child is initialized, call its' destructor.
    }
  }
  delete[] children;
  delete[] alphas;
}

// Update the subtree with the corresponding ballot and count.
// The ballot must be in index array form.
void Node::update(int *ballotPermutation, int *permutationArray) {
  int nextCandidate = ballotPermutation[0];
  int i = 0;
  while (permutationArray[i] != nextCandidate) {
    ++i;
  }
  // Update alpha parameter.
  alphas[i] += 1;
  // Stop if the number of children is 2, since we don't need to access the
  // leaves.
  if (nChildren == 2)
    return;
  // If the next node is uninitialized, initialize it.
  if (children[i] == nullptr) {
    // Each child node has one less candidate available to choose from.
    children[i] = new Node(nChildren - 1, baseTree);
  }
  // Update the corresponding child, passing it the array starting at next
  // index.
  std::swap(permutationArray[0], permutationArray[i]);
  children[i]->update(ballotPermutation + 1, permutationArray + 1);
}

/* Sample from the dirichlet tree distribution.
 *
 * Append sampled ballots to the output vector.
 */
void Node::sample(int *nBallots, int nElections, int *permutationArray,
                  int nChosen, election *out) {
  Ballot *b;
  int *nextNBallots;
  bool atLeastOne;
  int **countsForChildren = rDirichletMultinomial(
      nElections, nBallots, alphas, nChildren,
      baseTree->getEnginePtr()); // nElections arrays of length nCandidates,
                                 // each containing an array of counts which
                                 // correspond to the number of ballots which
                                 // choose child i as the next preference.

  // If nCandidates is 2, we stop recursing and return the array of elections.
  // TODO: STV elections
  if (nChildren == 2) {
    // Choose last 2 candidates.
    for (int j = 0; j < nChildren; ++j) {
      std::swap(permutationArray[nChosen + j], permutationArray[nChosen]);
      for (int i = 0; i < nElections; ++i) {
        if (countsForChildren[i][j] == 0)
          continue;
        b = new Ballot(nChosen + 2);
        // Convert start to a candidate permutation.
        for (int k = 0; k < nChosen + 2; ++k) {
          b->ballotPermutation[k] = permutationArray[k];
        }
        // Push appropriate number of copies to election.
        for (int k = 0; k < countsForChildren[i][j]; ++k) {
          out[i].push_back(*b);
        }
      }
      std::swap(permutationArray[nChosen + j], permutationArray[nChosen]);
    }
    for (int i = 0; i < nElections; ++i) {
      delete[] countsForChildren[i];
    }
    delete[] countsForChildren;
    return;
  }

  // Otherwise, we continue recursively distributing the ballots via
  // dirmultinomial sampling at each parent node, or if we reach a nullptr
  // child, we sample random permutations instead.
  for (int i = 0; i < nChildren; ++i) {
    // For each candidate we determine the number of ballots in each election.
    nextNBallots = new int[nElections];
    atLeastOne = 0;
    for (int j = 0; j < nElections; ++j) {
      // construct next nBallots array for samples.
      nextNBallots[j] = countsForChildren[j][i];
      atLeastOne = (atLeastOne || nextNBallots[j]);
    }
    // If there are no non-zero ballot counts for this candidate,
    // we simply skip to return an empty result set for this index.
    if (!atLeastOne) {
      delete[] nextNBallots;
      continue;
    }
    // Update next candidate.
    std::swap(permutationArray[nChosen + i], permutationArray[nChosen]);
    if (children[i] == nullptr) { // Sample random ballots indices.
      rElections(baseTree->getScale(), nextNBallots, nElections,
                 baseTree->getNCandidates(), permutationArray, nChosen + 1,
                 baseTree->getEnginePtr(), baseTree->getTreeType(),
                 baseTree->getFactorials(), out);
    } else {
      children[i]->sample(nextNBallots, nElections, permutationArray,
                          nChosen + 1, out);
      // Concat the results for each election.
    }
    std::swap(permutationArray[nChosen + i], permutationArray[nChosen]);
    delete[] nextNBallots;
  }
  for (int i = 0; i < nElections; ++i) {
    delete[] countsForChildren[i];
  }
  delete[] countsForChildren;
}

/* DirichletTreeIRV class constructor.
 *
 * We initialize a new node to act as the root of the tree,
 * and we initialize the factorials array for vanilla dirichlet distributions.
 */
DirichletTreeIRV::DirichletTreeIRV(int nCandidates, float scale, bool treeType,
                                   std::string seed) {

  this->nCandidates = nCandidates;
  this->scale = scale;
  this->treeType = treeType;
  this->observedBallots = {};

  std::seed_seq s(seed.begin(), seed.end());
  std::mt19937 e(s);
  for (int i = 0; i < 100; ++i) {
    e(); // Warming up prng
  }
  engine = e;

  if (treeType == TREE_TYPE_VANILLA_DIRICHLET) {
    // Initialize factorials for initial alpha calculations on each node.
    factorials = new int[nCandidates + 1];
    factorials[0] = 1;
    for (int i = 1; i <= nCandidates; ++i) {
      factorials[i] = factorials[i - 1] * i;
    }
  }
  root = new Node(nCandidates, this);
}

// Custom destructor.
DirichletTreeIRV::~DirichletTreeIRV() {
  delete root;
  if (treeType == TREE_TYPE_VANILLA_DIRICHLET) {
    delete[] factorials;
  }
}

// Update a dirichlet tree with a ballot and count in permutation form.
void DirichletTreeIRV::update(Ballot b) {
  int *permutationArray = new int[nCandidates];
  for (int i = 0; i < nCandidates; ++i) {
    permutationArray[i] = i + 1;
  }

  root->update(b.ballotPermutation, permutationArray);

  delete[] permutationArray;
}

/* Sample elections (i.e. distinct sets of ballots) from the Dirichlet Tree.
 */
election *DirichletTreeIRV::sample(
    int nElections, // Number of elections to sample.
    int nBallots    // Number of ballots to sample per election.
) {
  election *out = new election[nElections];
  int *permutationArray = new int[nCandidates];
  for (int i = 0; i < nCandidates; ++i) {
    permutationArray[i] = i + 1;
  }
  int nChosen = 0;
  int *ballots = new int[nElections];
  for (int i = 0; i < nElections; ++i) {
    ballots[i] = nBallots;
  }

  root->sample(ballots, nElections, permutationArray, nChosen, out);

  delete[] permutationArray;
  delete[] ballots;

  return out;
}

/* Sample elections and tabulate election winners in an integer array.
 */
int *DirichletTreeIRV::samplePosterior(int nElections, int nBallotsRemaining,
                                       bool useObserved) {
  int *candidateWins = new int[nCandidates]{0};
  int winner;
  election *e;

  // A copy of the incomplete election
  election incompleteCopy{};
  incompleteCopy.insert(incompleteCopy.end(), observedBallots.begin(),
                        observedBallots.end());

  e = sample(nElections, nBallotsRemaining);

  for (int i = 0; i < nElections; ++i) {
    incompleteCopy.erase(incompleteCopy.begin() + observedBallots.size(),
                         incompleteCopy.end());
    incompleteCopy.insert(incompleteCopy.begin() + observedBallots.size(),
                          e[i].begin(), e[i].end());

    winner = evaluateElection(incompleteCopy);

    ++candidateWins[winner - 1];
  }

  delete[] e;

  return candidateWins;
}

// Reset to prior without replacing rng.
void DirichletTreeIRV::reset() {
  delete root;
  root = new Node(nCandidates, this);
}

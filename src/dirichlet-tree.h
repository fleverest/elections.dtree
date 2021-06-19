#ifndef DIRICHLET_TREE_H
#define DIRICHLET_TREE_H

#include "ballot.h"
#include "distributions.h"
#include <algorithm>
#include <array>
#include <iterator>
#include <random>
#include <sstream>
#include <string>
#include <vector>

const bool TREE_TYPE_VANILLA_DIRICHLET = 1;
const bool TREE_TYPE_DIRICHLET_TREE = 0;

class DirichletTreeIRV;
class Node;

typedef Node *NodePtr;

// A class to represent a dirichlet tree prior for multinomial IRV ballots.
class DirichletTreeIRV {
private:
  int nCandidates; // The number of candidates in the election.
  float scale = 1; // The scale multiplier for the alpha paramters in the tree.
  NodePtr root;    // The dirichlet tree root node.
  int *factorials; // An array to store the factorials
                   // for a vanilla dirichlet distribution.
  bool treeType =
      TREE_TYPE_DIRICHLET_TREE; // A boolean indicating whether the tree is a
                                // vanilla dirichlet distribution or an
                                // arbitrary dirichlet tree.
  std::mt19937 engine;

public:
  // Custom constructor to calculate factorials, initialize a root node and RNG
  // generator.
  DirichletTreeIRV(int nCandidates_, float scale_, bool treeType_,
                   std::string seed);
  // Custom destructor to delete the nodes.
  ~DirichletTreeIRV();

  // For updating prior to obtain a posterior.
  void update(BallotCount bc);

  // For sampling from the posterior.
  election *sample(int nElections, int nBallots);

  // Getters.
  float getScale() { return scale; }

  int getNCandidates() { return nCandidates; }

  bool getTreeType() { return treeType; }

  std::mt19937 *getEnginePtr() { return &engine; }

  int *getFactorials() { return factorials; }
};

// A class for the internal nodes of a dirichlet tree.
class Node {
private:
  int nCandidates; // Number of children below node
  float *alphas;   // Array of doubles corresponding to the alpha paramter
  bool treeType;   // Boolean indicating whether the tree is a vanilla dirichlet
                   // distribution or an arbitrary dirichlet tree.
  NodePtr *children; // Array of child node pointers, initalized to NULL.
  DirichletTreeIRV *baseTree; // pointer to the tree which this node belongs to.

public:
  /* Node class constructor.
   *
   * To initialize a new node, we require the number of
   * remaining candidates to initialize appropriately
   * sized arrays for the child nodes and the corresponding
   * alpha parameter for the distribution over them.
   */
  Node(int nCandidates_, DirichletTreeIRV *baseTree_)
      : nCandidates(nCandidates_), baseTree(baseTree_) {

    if (nCandidates > 2) {
      children = new NodePtr[nCandidates]{NULL};
    }
    alphas = new float[nCandidates];
    if (baseTree->getTreeType() == TREE_TYPE_VANILLA_DIRICHLET) {
      std::fill(alphas, alphas + nCandidates,
                baseTree->getScale() * baseTree->getFactorials()[nCandidates]);
    } else if (baseTree->getTreeType() == TREE_TYPE_DIRICHLET_TREE) {
      std::fill(alphas, alphas + nCandidates, baseTree->getScale());
    }
  }

  /* Node class destructor.
   *
   * We must delete all child nodes to avoid memory leaks.
   */
  ~Node() {
    for (int i = 0; i < nCandidates; ++i) {
      if (children[i] != NULL) {
        delete children[i]; // If child is initialized, call its' destructor.
      }
    }
  }

  // Update the subtree with the corresponding ballot and count.
  // The ballot must be in index array form.
  void update(int *ballotPermutation, int count, int *permutationArray) {
    int nextCandidate = ballotPermutation[0];
    int i = 0;
    while (permutationArray[i] != nextCandidate) {
      ++i;
    }
    // Update alpha parameter.
    alphas[i] += count;
    // Stop if the number of children is 2, since we don't need to access the
    // leaves.
    if (nCandidates == 2)
      return;
    // If the next node is uninitialized, initialize it.
    if (children[i] == NULL) {
      // Each child node has one less candidate available to choose from.
      children[i] = new Node(nCandidates - 1, baseTree);
    }
    // Update the corresponding child, passing it the array starting at next
    // index.
    std::swap(permutationArray[0], permutationArray[i]);
    children[i]->update(ballotPermutation + 1, count, permutationArray + 1);
  }

  /* Sample from the dirichlet tree distribution.
   *
   * Append sampled ballots to the output vector.
   */
  void sample(int *nBallots, int nElections, int *permutationArray, int nChosen,
              election *out) {
    BallotCount bc;
    int *nextNBallots;
    bool atLeastOne;
    int **countsForChildren = rDirichletMultinomial(
        nElections, nBallots, alphas, nCandidates,
        baseTree->getEnginePtr()); // nElections arrays of length nCandidates,
                                   // each containing an array of counts which
                                   // correspond to the number of ballots which
                                   // choose child i as the next preference.

    // If nCandidates is 2, we stop recursing and return the array of elections.
    if (nCandidates == 2) {
      for (int j = 0; j < nCandidates; ++j) {
        std::swap(permutationArray[nChosen + j], permutationArray[nChosen]);
        for (int i = 0; i < nElections; ++i) {
          if (countsForChildren[i][j] == 0)
            continue;
          bc = *(new BallotCount);
          bc.count = countsForChildren[i][j];
          bc.ballotPermutation = new int[nChosen + 2];
          // Convert start to a candidate permutation.
          for (int k = 0; k <= nChosen + 1; ++k) {
            bc.ballotPermutation[k] = permutationArray[k];
          }
          out[i].push_back(bc);
        }
        std::swap(permutationArray[nChosen + j], permutationArray[nChosen]);
      }
      return;
    }

    // Otherwise, we continue recursively distributing the ballots via
    // dirmultinomial sampling at each parent node, or if we reach a NULL child,
    // we sample random permutations instead.
    for (int i = 0; i < nCandidates; ++i) {
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
      if (children[i] == NULL) { // Sample random ballots indices.
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
};

/* DirichletTreeIRV class constructor.
 *
 * We initialize a new node to act as the root of the tree,
 * and we initialize the factorials array for vanilla dirichlet distributions.
 */
DirichletTreeIRV::DirichletTreeIRV(int nCandidates_, float scale_,
                                   bool treeType_, std::string seed)
    : nCandidates(nCandidates_), scale(scale_), treeType(treeType_) {

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
DirichletTreeIRV::~DirichletTreeIRV() { delete root; }

// Update a dirichlet tree with a ballot and count in permutation form.
void DirichletTreeIRV::update(BallotCount bc) {
  int *permutationArray = new int[nCandidates];
  for (int i = 0; i < nCandidates; ++i) {
    permutationArray[i] = i + 1;
  }

  root->update(bc.ballotPermutation, bc.count, permutationArray);
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

#endif

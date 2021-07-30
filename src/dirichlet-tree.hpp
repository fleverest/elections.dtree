// dirichlet-tree.hpp

#ifndef DIRICHLET_TREE_HPP
#define DIRICHLET_TREE_HPP

#include "ballot.hpp"
#include "distributions.hpp"
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
  DirichletTreeIRV(int nCandidates, float scale, bool treeType,
                   std::string seed);

  // Custom destructor to delete the nodes.
  ~DirichletTreeIRV();

  // Reset the tree to the prior.
  void reset();

  // For updating prior to obtain a posterior.
  void update(Ballot b);

  // For sampling ballots from the posterior.
  election *sample(int nElections, int nBallots);

  // For determining posterior probabilities of each candidate winning.
  int *samplePosterior(int nElections, int nBallots, election incomplete);

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
  int nChildren; // Number of children representing candidates below node
  float *alphas; // Array of doubles corresponding to the alpha paramter
  bool treeType; // Boolean indicating whether the tree is a vanilla dirichlet
                 // distribution or an arbitrary dirichlet tree.
  NodePtr *children; // Array of child node pointers, initalized to nullptr.
  DirichletTreeIRV *baseTree; // pointer to the tree which this node belongs to.

public:
  /* Node class constructor.
   *
   * To initialize a new node, we require the number of
   * remaining candidates to initialize appropriately
   * sized arrays for the child nodes and the corresponding
   * alpha parameter for the distribution over them.
   */
  Node(int nCandidates_, DirichletTreeIRV *baseTree_);

  /* Node class destructor.
   *
   * We must delete all child nodes to avoid memory leaks.
   */
  ~Node();

  // Update the subtree with the corresponding ballot and count.
  // The ballot must be in index array form.
  void update(int *ballotPermutation, int *permutationArray);

  /* Sample from the dirichlet tree distribution.
   *
   * Append sampled ballots to the output vector.
   */
  void sample(int *nBallots, int nElections, int *permutationArray, int nChosen,
              election *out);
};

#endif

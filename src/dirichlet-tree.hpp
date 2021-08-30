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

#include <iostream>

const bool TREE_TYPE_VANILLA_DIRICHLET = 1;
const bool TREE_TYPE_DIRICHLET_TREE = 0;

// A class to represent a dirichlet tree prior for multinomial IRV ballots.
class DirichletTreeIRV {

private:
  struct node {
    /* An interior node of the Dirichlet Tree. */
    using nodeptr = node *;

    int nChildren; // The number of chlidren representing candidates below this
                   // node.
    float *alphas; // The corresponding alpha parameters for the dirichlet,
                   // floating point array with size = nChlidren. distribution
                   // across the child candidates.
    nodeptr *children; // Array of pointers to children below this node with
                       // size = nChildren.

    // Constructor initializes child array and alphas.
    node(int nChildren_) : nChildren(nChildren_) {
      // Nodes have an internal default alpha parameter of 0. This corresponds
      // to the additional alpha values added to the scale by ballot
      // observations. Initially there are no observations, so this is zero.
      alphas = new float[nChildren]{0.};
      // No children are initialized from the start, they are loaded lazily.
      children = new nodeptr[nChildren]{nullptr};
    }

    node(const node &node) = delete; // No copy constructor.

    // Destructor must delete the alpha parameters and also recursively destroy
    // children.
    ~node() {
      // Destroy alphas.
      delete alphas;
      // Destroy children
      for (auto i = 0; i < nChildren; ++i) {
        delete children[i];
      }
    }

    // An update method to observe ballots. Must update alpha parameters and add
    // children as necessary. To observe the ballots and hence update the
    // corresponding alpha parameters, we traverse the tree and update along the
    // path through a permutation tree.
    void update(int *ballotPermutation, int *permutationArray) {
      int nextCandidate = ballotPermutation[0];
      int i = 0;
      while (permutationArray[i] != nextCandidate) {
        ++i;
      }
      // Update alpha parameter.
      alphas[i] += 1;
      // Stop if the number of children is 2, since we don't need to access the
      // leaves when the last permutation choice is fixed.
      if (nChildren == 2)
        return;
      // If the next node is uninitialized, create it.
      if (children[i] == nullptr)
        children[i] = new node(nChildren - 1); // One less candidate to choose.
      // Recursively update next child, passing it an updated permutation array,
      // and the ballot permutation starting from the next index.
      std::swap(permutationArray[0], permutationArray[i]);
      children[i]->update(ballotPermutation + 1, permutationArray + 1);
    }

    /** A sampling method to sample ballots from below this node.
     *
     * Parameters:
     *  int nElections: the number sample elections to obtain from the posterior
     *                  distribution.
     *  int *nBallots:  an array of size nElections corresponding
     *                  to the number of ballots left to sample below this node
     *                  for each election.
     *  int *permutationArray: The permutation array used to navigate the tree.
     *  int nChosen:    the current depth in the permutation tree.
     *  float scale:    The prior scale of the underlying distribution.
     *  bool treeType:  The type oy posterior, either Dirichlet or Dirichlet
     *                  Tree.
     *  int *factorials: Array of factorials for parameter calculation in
     *                   vanilla Dirichlet.
     *  election *out:  An array of size nElections, which we add ballots
     *                  to for each sample.
     *  std::mt19937 engine: A PRNG to use for random sampling.
     */
    void sample(int *nBallots, int nElections, int *permutationArray,
                int nChosen, float scale, bool treeType, int *factorials,
                election *out, std::mt19937 *engine) {
      Ballot *b;
      int *nextNBallots;
      bool atLeastOne;
      // Construct an array of parameters which take into account the initial
      // scale.
      float *effectiveAlphas = new float[nChildren];
      for (auto i = 0; i < nChildren; ++i) {
        if (treeType == TREE_TYPE_DIRICHLET_TREE) {
          effectiveAlphas[i] = alphas[i] + scale;
        } else { // treeType == TREE_TYPE_VANILLA_DIRICHLET
          effectiveAlphas[i] = alphas[i] + scale * factorials[nChildren];
        }
      }
      // Get Dirichlet-multinomial counts for next-preference selections below
      // current node.
      int **countsForChildren = rDirichletMultinomial(
          nElections, nBallots, effectiveAlphas, nChildren, engine);
      delete[] effectiveAlphas;

      // if nCandidates is 2, stop recursing and add outputs to the vector.
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
        // For each candidate we determine the number of ballots in each
        // election.
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
          rElections(scale, nextNBallots, nElections, nChildren + nChosen,
                     permutationArray, nChosen + 1, engine, treeType,
                     factorials, out);
        } else {
          children[i]->sample(nextNBallots, nElections, permutationArray,
                              nChosen + 1, scale, treeType, factorials, out,
                              engine);
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

  node::nodeptr root; // The root node of the Dirichlet Tree.
  int nCandidates; // The number of candidates in the running for the election.
  float scale;     // Parameter which controlls how informative the prior is.
  int *factorials; // An array of pre-calculated factorials of size =
                   // nCandidates;
  bool treeType;   // Indicates the type of the prior. For the space-efficient
                   // vanilla Dirichlet distribution prior implementation it is
  // `TREE_TYPE_VANILLA_DIRICHLET`, for the Dirichlet Tree prior it
  // is `TREE_TYPE_DIRICHLET_TREE`.
  std::vector<Ballot> observedBallots; // A vector of the set of ballots
                                       // observed thus far in the audit.
  std::mt19937 engine; // A default PRNG to use in random sampling. This is
                       // necessary for multithreading, and  in practise it will
                       // be seeded from another PRNG.

public:
  // Constructor
  DirichletTreeIRV(int nCandidates_, float scale_, bool treeType_,
                   std::string seed)
      : nCandidates(nCandidates_), scale(scale_), treeType(treeType_),
        root(new node(nCandidates_)) {
    // Initialize factorials.
    factorials = new int[nCandidates + 1]{1};
    for (auto i = 1; i <= nCandidates; ++i) {
      factorials[i] = factorials[i - 1] * i;
    }
  }

  // Destructor must delete factorials array.
  ~DirichletTreeIRV() { delete factorials; }

  // Reset the prior to original state before observing any data.
  void reset() {
    delete root;
    root = new node(nCandidates);
  }

  // Core methods for inference implemented in `dirichlet-tree.cpp`.

  // Update the prior with a ballot.
  void update(Ballot b) {
    int *permutationArray = new int[nCandidates];
    for (int i = 0; i < nCandidates; ++i) {
      permutationArray[i] = i + 1;
    }
    // Update observed ballots.
    observedBallots.push_back(b);

    // Update the alpha parameters by traversing the tree.
    root->update(b.ballotPermutation, permutationArray);

    delete[] permutationArray;
  }

  // Sample an election from the posterior.
  election *sample(int nElections, int nBallots,
                   std::mt19937 *engine = nullptr) {
    // Apply default engine if not provided.
    if (engine == nullptr) {
      engine = &this->engine;
    }

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

    root->sample(ballots, nElections, permutationArray, nChosen, scale,
                 treeType, factorials, out, engine);

    delete[] permutationArray;
    delete[] ballots;

    return out;
  }

  // Sample from the posterior ballot distribution to determine election
  // victory proportions for each candidate.
  int *samplePosterior(int nElections, int nBallots, bool useObserved,
                       std::mt19937 *engine = nullptr) {
    int *candidateWins = new int[nCandidates]{0};
    int winner, size;
    election *e;
    election incomplete{};

    if (useObserved) {
      // Insert a copy of the incomplete election
      incomplete.insert(incomplete.end(), observedBallots.begin(),
                        observedBallots.end());
      size = observedBallots.size();
      nBallots = nBallots - size;
    } else {
      size = 0;
    }

    if (nBallots < 0) {
      return candidateWins;
    }

    e = sample(nElections, nBallots, engine);

    for (int i = 0; i < nElections; ++i) {
      // Clear old sampled ballots and copy next sample in place.
      incomplete.erase(incomplete.begin() + size, incomplete.end());
      incomplete.insert(incomplete.begin() + size, e[i].begin(), e[i].end());

      // Evaluate completed election
      winner = evaluateElection(incomplete);

      ++candidateWins[winner - 1];
    }

    delete[] e;

    return candidateWins;
  }

  // Getters
  float getScale() { return scale; }

  int getNCandidates() { return nCandidates; }

  bool getTreeType() { return treeType; }

  int *getFactorials() { return factorials; }

  std::mt19937 *getEnginePtr() { return &engine; }

  // Setters
  void setScale(float scale) { scale = scale; }

  void setTreeType(bool treeType) { treeType = treeType; }
};

#endif

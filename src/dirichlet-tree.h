#ifndef DIRICHLET_TREE_H
#define DIRICHLET_TREE_H

#include <array>
#include <random>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include "distributions.h"
#include "ballot.h"

const bool TREE_TYPE_VANILLA_DIRICHLET = 1;
const bool TREE_TYPE_DIRICHLET_TREE = 0;

class DirichletTreeIRV;
class Node;

typedef Node* NodePtr;

// A class to represent a dirichlet tree prior for multinomial IRV ballots.
class DirichletTreeIRV {
private:
    int nCandidates;   // The number of candidates in the election.
    float scale = 1;   // The scale multiplier for the alpha paramters in the tree.
    NodePtr root; // The dirichlet tree root node.
    int* factorials;                          // An array to store the factorials
                                              // for a vanilla dirichlet distribution.
    bool treeType = TREE_TYPE_DIRICHLET_TREE; // A boolean indicating whether the tree is a vanilla
                                              // dirichlet distribution or an arbitrary dirichlet tree.
    std::mt19937 engine;

public:

    // Custom constructor to calculate factorials, initialize a root node and RNG generator.
    DirichletTreeIRV(int nCandidates_, float scale_, bool treeType_, unsigned seed);
    // Custom destructor to delete the nodes.
    ~DirichletTreeIRV();

    // For updating prior to obtain a posterior.
    void update(int* ballot, int count);

    // For sampling from the posterior.
    std::string* sample(int nElections, int nBallots);

    // Getters.
    float getScale(){
        return scale;
    }

    int getNCandidates(){
        return nCandidates;
    }

    bool getTreeType(){
        return treeType;
    }

    std::mt19937 getEngine(){
        return engine;
    }

    int factorial(int i){
        return factorials[i];
    }

};

// A class for the internal nodes of a dirichlet tree.
class Node {
private:
    int nCandidates;            // Number of children below node
    float* alphas;              // Array of floats corresponding to the alpha paramter
    bool treeType;              // Boolean indicating whether the tree is a vanilla dirichlet distribution or an arbitrary dirichlet tree.
    NodePtr* children;          // Array of child node pointers, initalized to NULL.
    DirichletTreeIRV* baseTree; // pointer to the tree which this node belongs to.

public:
    /* Node class constructor.
     *
     * To initialize a new node, we require the number of
     * remaining candidates to initialize appropriately
     * sized arrays for the child nodes and the corresponding
     * alpha parameter for the distribution over them.
     */
    Node(
            int nCandidates_,
            DirichletTreeIRV* baseTree_
    ): nCandidates(nCandidates_), baseTree(baseTree_) {
        if( nCandidates > 2 ){
            children = new NodePtr[nCandidates];
        }
        alphas = new float[nCandidates];
        if( baseTree->getTreeType() == TREE_TYPE_VANILLA_DIRICHLET ){
            std::fill(alphas, alphas + nCandidates, baseTree->getScale() * baseTree->factorial(nCandidates));
        } else if (baseTree->getTreeType() == TREE_TYPE_DIRICHLET_TREE) {
            std::fill(alphas, alphas + nCandidates, baseTree->getScale());
        }
    }

    /* Node class destructor.
     *
     * We must delete all child nodes to avoid memory leaks.
     */
    ~Node(){
        for( int i = 0; i < nCandidates; ++i ){
            if( children[i] != NULL ){
                children[i]->~Node(); // If node is initialized, call its' destructor.
            }
        }
    }

    // Update the subtree with the corresponding ballot and count.
    // The ballot must be in index array form.
    void update(int* ballot, int count){
        int next = ballot[0];
        // Update alpha parameter.
        alphas[next] += count;
        // Stop if the number of children is 2, since we don't need to access the leaves.
        if( nCandidates == 2) return;
        // If the next node is uninitiated, initiate it.
        if( children[next] == NULL ){
            // Each child node has one less candidate available to choose from.
            children[next] = new Node(nCandidates - 1, baseTree);
        }
        // Update the corresponding child, passing it the array starting at next index.
        children[next]->update(&ballot[1], count);
    }

    /* Sample from the dirichlet tree distribution.
     *
     * Given an int array (number of ballots to sample for each set), and
     * the number of elections, we return an array of vectors of ballot
     * counts (i.e. an array of elections).
     */
    election* sample(int* nBallots, int nElections, int* start, int nChosen){
        int n;
        BallotCount* bc;
        int* nextNBallots;
        bool atLeastOne;
        election* childBallotSets;
        election* results = new election[nElections];
        int** countsForChildren = rDirichletMultinomial(
                nElections, nBallots, alphas, nCandidates, baseTree->getEngine()
        ); // nElections arrays of length nCandidates, each containing an array of counts which
        // correspond to the number of ballots which choose child i as the next preference.

        // If nCandidates is 2, we stop recursing and return the array of elections.
        if( nCandidates == 2 ){
            for( int i = 0; i < nElections; ++i ){
                for( int j = 0; j < nCandidates; ++j ){
                    if( countsForChildren[i][j] == 0 ) continue;
                    bc = new BallotCount;
                    bc->count = countsForChildren[i][j];
                    bc->ballot = new int[nChosen];
                    // Convert start to a candidate permutation.
                    start[nChosen] = j;
                    for( int k = nChosen; k >= 0; --k ){
                        n = start[k];
                        for( int l = k; l >= 0; --l ){
                            if( start[l] <= start[k] ) ++n;
                        }
                        bc->ballot[k] = n;
                    }
                    results[i].push_back(*bc);
                }
            }
            return results;
        }

        // Otherwise, we continue recursively distributing the ballots via dirmultinomial sampling
        // at each parent node, or if we reach a NULL child, we sample random permutations instead.
        for( int i = 0; i < nCandidates; ++i ){
            // For each candidate we determine the number of ballots in each election.
            nextNBallots = new int[nElections];
            atLeastOne = 0;
            for( int j = 0; j < nElections; ++j ){ // construct next nBallots array for samples.
                nextNBallots[j] = countsForChildren[j][i];
                atLeastOne = (atLeastOne || nextNBallots[j]);
            }
            // If there are no non-zero ballot counts for this candidate,
            // we simply skip to return an empty result set for this index.
            if( !atLeastOne ) continue;
            // Get candidate number of corresponding index.
            start[nChosen] = i;
            if( children[i] == NULL ){ // Sample random ballots indices.
                // Calculate candidates remaining based on index trace
                childBallotSets = rElections(
                        nextNBallots,
                        nElections,
                        baseTree->getNCandidates(),
                        start,
                        nChosen + 1,
                        baseTree->getEngine()
                );
                for( int j = 0; j < nElections; ++j ){
                    results[j].insert(
                        results[j].end(),
                        std::make_move_iterator(childBallotSets[j].begin()),
                        std::make_move_iterator(childBallotSets[j].end())
                    );
                }
                delete[] childBallotSets;
            } else {
                childBallotSets = children[i]->sample(nextNBallots, nElections, start, nChosen + 1);
                // Concat the results for each election.
                for( int j = 0; j < nElections; ++j ){
                    results[j].insert(
                        results[j].end(),
                        std::make_move_iterator(childBallotSets[j].begin()),
                        std::make_move_iterator(childBallotSets[j].end())
                    );
                }
                delete[] childBallotSets;
            }
        }
        return results;
    }
};

/* DirichletTreeIRV class constructor.
 *
 * We initialize a new node to act as the root of the tree,
 * and we initialize the factorials array for vanilla dirichlet distributions.
 */
DirichletTreeIRV::DirichletTreeIRV(
        int nCandidates_, float scale_, bool treeType_, unsigned seed
): nCandidates(nCandidates_), scale(scale_), treeType(treeType_), engine(std::mt19937(seed)) {

    if( treeType == TREE_TYPE_VANILLA_DIRICHLET ){
        // Initialize factorials for initial alpha calculations on each node.
        factorials = new int[nCandidates+1];
        factorials[0] = 1;
        for( int i = 1; i <= nCandidates; ++i){
            factorials[i] = factorials[i-1] * i;
        }
    }
    root = new Node(nCandidates, this);
}

// Update a dirichlet tree with a ballot and count in index form.
void DirichletTreeIRV::update(int* ballot, int count){
    for( int i = 0; i < count; ++i ){
            root->update(ballot, count);
    }
}

/* Sample elections (i.e. distinct sets of ballots) from the Dirichlet Tree.
 *
 * Outputs an array of arrays of strings.
 */
std::string* DirichletTreeIRV::sample(
        int nElections, // Number of elections to sample.
        int nBallots // Number of ballots to sample per election.
){
    std::string* out = new std::string[nElections];
    election* es;
    int* start = new int[nCandidates-1];
    int nChosen = 0;
    int* ballots = new int[nElections];
    for( int i = 0; i < nElections; ++i ){
        ballots[i] = nBallots;
    }

    es = root->sample(ballots, nElections, start, nChosen);

    // Convert ballot counts to string format.
    for( int i = 0; i < nElections; ++i ){
        out[i] = electionToStr(es[i], nCandidates);
    }
    return out;
}

#endif


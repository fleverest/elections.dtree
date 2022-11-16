/******************************************************************************
 * File:             R_tree.h
 *
 * Author:           Floyd Everest <me@floydeverest.com>
 * Created:          05/14/22
 * Description:      This file defines the Rcpp interface for all of the
 *                   Dirichlet-tree methods, and for the social choice
 *                   functions.
 *****************************************************************************/

#ifndef R_TREE_H
#define R_TREE_H

#include <Rcpp.h>
#include <RcppThread.h>

#include <random>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "dirichlet_tree.h"
#include "irv_ballot.h"
#include "irv_node.h"

/*! \brief An Rcpp object which implements the `dtree` R object interface.
 *
 *  This class exposes all interfaces for the partially-ordered, IRV-ballot
 * Dirichlet-tree.
 */
class RDirichletTree {
 private:
  // The underlying Dirichlet-tree.
  DirichletTree<IRVNode, IRVBallot, IRVParameters> *tree;

  // A vector of candidate names.
  Rcpp::CharacterVector candidateVector{};

  // A map of candidate names to their ballot index.
  std::unordered_map<std::string, size_t> candidateMap{};

  // A record of the number of observed ballots.
  size_t nObserved = 0;

  // Records the depths which have been observed, so that we can check whether
  // the posterior can reduce to a Dirichlet distribution or not.
  std::unordered_set<unsigned> observedDepths{};

  /*! \brief Converts an R list of valid IRV ballot vectors to a
   * std::list<IRVBallotCount> format.
   *
   *  In R, we consider a matrix of ballots to be that with columns
   * corresponding to each preference choice, and elements corresponding to the
   * index of the candidate.
   *
   * \param bs An Rcpp::List of ballots (assumed to be in Rcpp::CharacterVector
   * representation).
   *
   * \return A list of IRVBallotCount objects.
   */
  std::list<IRVBallotCount> parseBallotList(Rcpp::List bs);

 public:
  // Constructor
  RDirichletTree(Rcpp::CharacterVector candidates, unsigned minDepth_,
                 unsigned maxDepth_, double a0_, bool vd_, std::string seed_);

  // Destructor.
  ~RDirichletTree();

  // TODO: Document methods.

  // Getters
  unsigned getNCandidates();
  unsigned getMinDepth();
  unsigned getMaxDepth();
  double getA0();
  bool getVD();
  Rcpp::CharacterVector getCandidates();

  // Setters
  void setMinDepth(unsigned minDepth_);
  void setMaxDepth(unsigned maxDepth_);
  void setA0(double a0_);
  void setSeed(std::string seed_);
  void setVD(bool vd_);

  // Other methods
  void reset();
  void update(Rcpp::List ballots);
  Rcpp::List samplePredictive(unsigned nSamples, std::string seed);
  Rcpp::NumericVector samplePosterior(unsigned nElections, unsigned nBallots,
                                      unsigned nWinners, unsigned nThreads,
                                      std::string seed);
};

#endif /* R_TREE_H */

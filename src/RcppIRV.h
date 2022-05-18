/******************************************************************************
 * File:             RcppIRV.hpp
 *
 * Author:           Floyd Everest <me@floydeverest.com>
 * Created:          05/14/22
 * Description:      This file defines an Rcpp interface for the IRV
 *                   Dirichlet Tree methods, and for the IRV social choice
 *                   function.
 *****************************************************************************/

#ifndef RCPP_TREE_H
#define RCPP_TREE_H

#include "dirichlet_tree.h"
#include "irv_ballot.h"
#include "irv_node.h"

#include <Rcpp.h>
#include <RcppThread.h>
#include <random>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

/*! \brief The IRV social choice function.
 *
 *  This function calculates an election outcome using the standard IRV social
 * choice function.
 *
 * \param  bs An Rcpp::List of ballots in CharacterVector representation.
 *
 * \param nWinners An integer indicating the number of winners to elect.
 *
 * \param candidates A vector of strings corresponding to candidate names.
 *
 * \param seed A seed for the PRNG for tie-breaking.
 *
 * \return The winning candidate.
 */
Rcpp::List RSocialChoiceIRV(Rcpp::List bs, unsigned nWinners,
                            Rcpp::CharacterVector candidates, std::string seed);

/*! \brief An Rcpp object which implements the `dtree` R object interface.
 *
 *  This class exposes all interfaces for the partially-ordered, IRV-ballot
 * Dirichlet Tree.
 */
class PIRVDirichletTree {
private:
  // The underlying Dirichlet Tree.
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
  PIRVDirichletTree(Rcpp::CharacterVector candidates, unsigned minDepth_,
                    float a0_, bool vd_, std::string seed_);

  // Destructor.
  ~PIRVDirichletTree();

  // TODO: Document methods.

  // Getters
  unsigned getNCandidates();
  unsigned getMinDepth();
  float getA0();
  bool getVD();
  Rcpp::CharacterVector getCandidates();

  // Setters
  void setMinDepth(unsigned minDepth_);
  void setA0(float a0_);
  void setSeed(std::string seed_);
  void setVD(bool vd_);

  // Other methods
  void reset();
  void update(Rcpp::List ballots);
  Rcpp::List samplePredictive(unsigned nSamples, std::string seed);
  Rcpp::NumericVector samplePosterior(unsigned nElections, unsigned nBallots,
                                      unsigned nWinners, unsigned nBatches,
                                      std::string seed);
  Rcpp::NumericVector sampleMarginalProbability(unsigned nSamples,
                                                Rcpp::CharacterVector ballot,
                                                std::string seed);
};

#endif /* RCPP_TREE_H */

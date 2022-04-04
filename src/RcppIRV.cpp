/******************************************************************************
 * File:             RcppIRV.cpp
 *
 * Author:           Floyd Everest <me@floydeverest.com>
 * Created:          03/01/22
 * Description:      This file implements an Rcpp interface for the IRV
 *                   Dirichlet Tree methods, and for the IRV social choice
 *function.
 *****************************************************************************/

// [[Rcpp::plugins("cpp17")]]
// [[Rcpp::depends(RcppThread)]]

#include "dirichlet_tree.hpp"
#include "irv_ballot.hpp"
#include "irv_node.hpp"

#include <Rcpp.h>
#include <RcppThread.h>
#include <unordered_map>

/*! \brief The IRV social choice function.
 *
 *  This function calculates an election outcome using the standard IRV social
 * choice function.
 *
 * \param  bs An Rcpp::List of ballots in CharacterVector representation.
 *
 * \return The winning candidate.
 */
// [[Rcpp::export]]
Rcpp::CharacterVector RSocialChoiceIRV(Rcpp::List bs) {
  std::list<IRVBallot> scInput{};

  std::unordered_map<std::string, size_t> c2Index{};
  std::vector<std::string> cNames{};

  Rcpp::CharacterVector bNames;
  std::string cName;
  std::vector<int> bIndices;

  for (auto i = 0; i < bs.size(); ++i) {
    bNames = bs[i];
    bIndices = {};
    for (auto j = 0; j < bNames.size(); ++j) {
      cName = bNames[j];
      // If candidate has not yet been seen, add it to our map and vector.
      if (c2Index.count(cName) == 0) {
        c2Index[cName] = cNames.size();
        cNames.push_back(cName);
      }
      bIndices.push_back(c2Index[cName]);
    }
    scInput.emplace_back(bIndices);
  }

  return cNames[socialChoiceIRV(scInput, cNames.size())];
}

/*! \brief An Rcpp object which implements the `dtree` R object interface.
 *
 *  This class exposes all interfaces for the partially-ordered, IRV-ballot
 * Dirichlet Tree.
 */
class PIRVDirichletTree {
private:
  // The underlying Dirichlet Tree.
  DirichletTree<IRVNode, IRVBallot, IRVParameters> *tree;

  // A vector (in order) of seen candidates.
  std::vector<std::string> candidateVector{};
  // A map of candidate names to their index.
  std::unordered_map<std::string, size_t> candidateMap{};

  /*! \brief Converts an R list of valid IRV ballot vectors to a
   * std::list<IRVBallot> format.
   *
   *  In R, we consider a matrix of ballots to be that with columns
   * corresponding to each preference choice, and elements corresponding to the
   * index of the candidate.
   *
   * \param bs An Rcpp::List of ballots (assumed to be in Rcpp::CharacterVector
   * representation).
   *
   * \return A list of IRVBallot objects.
   */
  std::list<IRVBallot> parseBallotList(Rcpp::List bs) {
    Rcpp::CharacterVector namePrefs;
    std::string cName;
    std::vector<int> indexPrefs;
    int cIndex;

    int nCandidates = getNCandidates();

    std::list<IRVBallot> out;

    // We iterate over each ballot, and convert it into an IRVBallot using the
    // "candidate index" for each seen candidate.
    for (auto i = 0; i < bs.size(); ++i) {
      namePrefs = bs[i];
      indexPrefs = {};
      for (auto j = 0; j < namePrefs.size(); ++j) {
        cName = namePrefs[j];

        // Find index for the candidate. Add it to our set if it doesn't exist.
        if (candidateMap.count(cName) == 0) {
          Rcpp::stop("Unknown candidate encountered in ballot!");
        } else {
          cIndex = candidateMap[cName];
        }

        indexPrefs.push_back(cIndex);
      }
      out.emplace_back(indexPrefs);
    }

    return out;
  }

public:
  // Constructor/destructor
  PIRVDirichletTree(Rcpp::CharacterVector candidates, int minDepth_,
                    float alpha0_, std::string seed_) {
    // Parse the candidate strings.
    std::string cName;
    int cIndex = 0;
    for (auto i = 0; i < candidates.size(); ++i) {
      cName = candidates[i];
      candidateVector.push_back(cName);
      candidateMap[cName] = cIndex;
      ++cIndex;
    }
    // Initialize tree.
    IRVParameters params(candidates.size(), minDepth_, alpha0_);
    tree = new DirichletTree<IRVNode, IRVBallot, IRVParameters>(params, seed_);
  }
  ~PIRVDirichletTree() { delete tree; }

  // Getters
  int getNCandidates() { return tree->getParameters().getNCandidates(); }
  int getMinDepth() { return tree->getParameters().getMinDepth(); }
  float getAlpha0() { return tree->getParameters().getAlpha0(); }

  // Setters
  void setMinDepth(int minDepth_) {
    tree->getParameters().setMinDepth(minDepth_);
  }
  void setAlpha0(float alpha0_) { tree->getParameters().setAlpha0(alpha0_); }
  void setSeed(std::string seed_) { tree->setSeed(seed_); }

  // Other methods
  void reset() { tree->reset(); }

  void update(Rcpp::List ballots) {
    std::list<IRVBallot> bs = parseBallotList(ballots);
    for (auto b : bs)
      tree->update(b, 1);
  }

  Rcpp::List samplePredictive(int nSamples, std::string seed) {

    tree->setSeed(seed);

    Rcpp::List out;
    Rcpp::CharacterVector rBallot;

    std::list<IRVBallot> samples = tree->sample(nSamples);
    for (auto b : samples) {
      rBallot = Rcpp::CharacterVector::create();
      for (auto cIndex : b.preferences) {
        rBallot.push_back(candidateVector[cIndex]);
      }
      out.push_back(rBallot);
    }

    return out;
  }

  Rcpp::NumericVector sampleCandidatePosterior(int nElections, int nBallots,
                                               std::string seed) {

    Rcpp::NumericVector out = {};
    int nCandidates = getNCandidates();
    int winner;

    for (auto i = 0; i < nCandidates; ++i)
      out.push_back(0);

    std::list<std::list<IRVBallot>> elections =
        tree->posteriorSets(nElections, nBallots);

    for (auto e : elections) {
      winner = socialChoiceIRV(e, nCandidates);
      out[winner] = out[winner] + 1;
    }

    out = out / nElections;
    return out;
  }

  Rcpp::NumericVector sampleMarginalProbability(int nSamples,
                                                Rcpp::CharacterVector ballot,
                                                std::string seed) {
    float prob;
    Rcpp::NumericVector out = {};
    std::string name;

    std::vector<int> preferences = {};
    for (auto i = 0; i < ballot.size(); ++i) {
      name = ballot[i];
      preferences.push_back(candidateMap[name]);
    }

    IRVBallot b(preferences);

    for (auto i = 0; i < nSamples; ++i) {
      prob = tree->marginalProbability(b, nullptr);
      out.push_back(prob);
    }

    return out;
  }
};

// The Rcpp module interface.
RCPP_MODULE(pirv_dirichlet_tree_module) {
  Rcpp::class_<PIRVDirichletTree>("PIRVDirichletTree")
      // Constructor needs nCandidates, minDepth, alpha0 and seed.
      .constructor<Rcpp::CharacterVector, int, float, std::string>()
      // Getter/Setter interface
      .property("nCandidates", &PIRVDirichletTree::getNCandidates)
      .property("alpha0", &PIRVDirichletTree::getAlpha0,
                &PIRVDirichletTree::setAlpha0)
      .property("minDepth", &PIRVDirichletTree::getMinDepth,
                &PIRVDirichletTree::setMinDepth)
      .method("setSeed", &PIRVDirichletTree::setSeed)
      // Methods
      .method("reset", &PIRVDirichletTree::reset)
      .method("update", &PIRVDirichletTree::update)
      .method("samplePredictive", &PIRVDirichletTree::samplePredictive)
      .method("sampleCandidatePosterior",
              &PIRVDirichletTree::sampleCandidatePosterior)
      .method("sampleMarginalProbability",
              &PIRVDirichletTree::sampleMarginalProbability);
}

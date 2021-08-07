// Rexports.cpp

#include "ballot.hpp"
#include "dirichlet-tree.hpp"
#include "distributions.hpp"

#include <Rcpp.h>
#include <string.h>

using namespace Rcpp;

// Converts an R DataFrame representation to an election.
election dfToElection(DataFrame df) {
  election e = {};
  int nCandidates = df.length();
  IntegerVector col;

  if (df.length() < nCandidates) {
    Rcout << "Insufficient columns (" << df.length()
          << ") to represent ballots cast in an "
             "election with "
          << nCandidates << " candidates." << std::endl;
  }

  for (auto i = 0; i < nCandidates; ++i) {
    col = df[i];
    for (auto j = 0; j < df.nrows(); ++j) {
      if (i == 0) // One ballot for each row.
        e.push_back(*new Ballot(nCandidates));
      e[j].ballotPermutation[i] = col[j];
    }
  }

  return e;
}

DataFrame electionToDF(election e, int nCandidates) {
  DataFrame out = DataFrame::create();
  IntegerVector col;
  int nBallots = e.size();

  for (int i = 0; i < nCandidates; ++i) {
    col = IntegerVector::create();
    for (int j = 0; j < nBallots; ++j) {
      if (e[j].nPreferences < i) {
        col.push_back(NA_INTEGER);
      } else {
        col.push_back(e[j].ballotPermutation[i]);
      }
    }

    out.push_back(col, "preference" + std::to_string(i + 1));
  }

  return out;
}

// R Interface for the DirichletTreeIRV class.
class RcppDirichletTreeIRV {
private:
  DirichletTreeIRV dtree; // The internal Dirichlet Tree to interface with.
  int nCandidates;

public: // Methods to be exposed to R
  // Constructor
  RcppDirichletTreeIRV(int nCandidates, float scale, std::string treeType,
                       std::string seed)
      : nCandidates(nCandidates),
        dtree(*new DirichletTreeIRV(nCandidates, scale, treeType == "dirichlet",
                                    seed)) {}

  void reset() { dtree.reset(); }

  void update(DataFrame ballotCounts) {
    election e = dfToElection(ballotCounts);

    for (auto b : e) {
      dtree.update(b);
    }
  }

  DataFrame sample(int nBallots) {
    DataFrame out = DataFrame::create();
    election e = dtree.sample(1, nBallots)[0];
    return electionToDF(e, nCandidates);
  }

  IntegerVector samplePosterior(int nElections, int nBallots,
                                bool useObserved) {
    int *results = dtree.samplePosterior(nElections, nBallots, useObserved);

    return IntegerVector(results, results + nCandidates);
  }

  int evaluate(DataFrame df) {
    election e = dfToElection(df);
    return evaluateElection(e);
  }

  int evaluateSelf() { return evaluateElection(dtree.getObservedBallots()); };
};

RCPP_MODULE(RcppDirichletTreeIRV) {
  class_<RcppDirichletTreeIRV>("RcppDirichletTreeIRV")
      .constructor<int, float, std::string, std::string>()
      .method("reset", &RcppDirichletTreeIRV::reset)
      .method("update", &RcppDirichletTreeIRV::update)
      .method("evaluate", &RcppDirichletTreeIRV::evaluate)
      .method("evaluateSelf", &RcppDirichletTreeIRV::evaluateSelf)
      .method("sample", &RcppDirichletTreeIRV::sample)
      .method("samplePosterior", &RcppDirichletTreeIRV::samplePosterior);
};

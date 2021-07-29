// Rexports.cpp

#include "ballot.hpp"
#include "dirichlet-tree.hpp"
#include "distributions.hpp"

#include <Rcpp.h>
#include <string.h>

using namespace Rcpp;

// Converts an R DataFrame representation to an election.
election dfToElection(DataFrame df, int nCandidates) {
  election e = {};
  Ballot *b;
  IntegerVector col;

  if (df.length() < nCandidates + 1) {
    Rcout << "Insufficient columns to represent ballots cast in an "
             "election with "
          << nCandidates << " candidates." << std::endl;
  }

  for (int i = 0; i < df.nrows(); ++i) {
    b = new Ballot;
    b->nPreferences = nCandidates;
    e.push_back(*b);
  }

  for (int i = 0; i < nCandidates; ++i) {
    col = df[i];
    for (int j = 0; j < df.nrows(); ++j) {
      e[j].ballotPermutation[i] = col[j];
    }
  }

  return e;
}

DataFrame electionToDF(election e, int nCandidates) {
  DataFrame out = DataFrame::create();
  IntegerVector col;

  for (int i = 0; i < nCandidates; ++i) {
    col = {};
    for (int j = 0; j < e.size(); ++j) {
      if (e[j].nPreferences < i) {
        col.push_back(NA_INTEGER);
        continue;
      }
      col.push_back(e[j].ballotPermutation[i]);
    }
    out.push_back(col);
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
    election e = dfToElection(ballotCounts, nCandidates);

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
                                DataFrame incompleteElection) {
    election incomplete = dfToElection(incompleteElection, nCandidates);

    int *results = dtree.samplePosterior(nElections, nBallots, incomplete);

    return IntegerVector(results, results + nCandidates);
  }
};

RCPP_MODULE(RcppDirichletTreeIRV) {
  class_<RcppDirichletTreeIRV>("RcppDirichletTreeIRV")
      .constructor<int, float, std::string, std::string>()
      .method("reset", &RcppDirichletTreeIRV::reset)
      .method("update", &RcppDirichletTreeIRV::update)
      .method("sample", &RcppDirichletTreeIRV::sample)
      .method("samplePosterior", &RcppDirichletTreeIRV::samplePosterior);
};

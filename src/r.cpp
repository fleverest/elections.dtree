// Rexports.cpp
//
// [[Rcpp::depends(RcppThread)]]

#include "ballot.hpp"
#include "dirichlet-tree.hpp"
#include "distributions.hpp"

#include <Rcpp.h>
#include <RcppThread.h>
#include <random>
#include <string.h>
#include <thread>

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
  int nCandidates = 1;

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

  IntegerVector samplePosterior(int nElections, int nBallots, bool useObserved,
                                int nBatches = 1) {
    int *output = new int[nCandidates];
    for (int i = 0; i < nCandidates; ++i) {
      output[i] = 0;
    }
    int **results = new int *[nBatches + 1];
    int electionBatchSize = nElections / nBatches;
    int electionBatchRemainder = nElections % nBatches;
    // Seed one RNG for each thread.
    std::mt19937 treeGen = *dtree.getEnginePtr();
    unsigned seed[nBatches];
    for (int i = 0; i <= nBatches; ++i) {
      seed[i] = treeGen();
    }

    // Use RcppThreads to compute the posterior in batches.
    RcppThread::ThreadPool pool(std::thread::hardware_concurrency());

    auto getBatchResult = [&](size_t i) -> void {
      RcppThread::checkUserInterrupt();
      // Seed a new PRNG, and warm it up.
      std::mt19937 e(seed[i]);
      e.discard(e.state_size * 100);
      // Sample posterior
      results[i] =
          dtree.samplePosterior(electionBatchSize, nBallots, useObserved, &e);
    };

    pool.parallelFor(0, nBatches, getBatchResult, nBatches);
    pool.join();

    for (int i = 0; i <= nBatches; ++i) {
      // Sample remainder for the remainder.
      if (i == nBatches) {
        std::mt19937 e(seed[i]);
        e.discard(e.state_size * 100);
        results[i] = dtree.samplePosterior(electionBatchRemainder, nBallots,
                                           useObserved, &e);
      }
      for (int j = 0; j < nCandidates; ++j) {
        output[j] += results[i][j];
      }
      delete[] results[i];
    }
    delete[] results;

    return IntegerVector(output, output + nCandidates);
  }

  int evaluate(DataFrame df) {
    election e = dfToElection(df);
    return evaluateElection(e);
  }

  float getScale() { return dtree.getScale(); }
};

RCPP_MODULE(RcppDirichletTreeIRV) {
  class_<RcppDirichletTreeIRV>("RcppDirichletTreeIRV")
      .constructor<int, float, std::string, std::string>()
      .method("reset", &RcppDirichletTreeIRV::reset)
      .method("update", &RcppDirichletTreeIRV::update)
      .method("evaluate", &RcppDirichletTreeIRV::evaluate)
      .method("sample", &RcppDirichletTreeIRV::sample)
      .method("samplePosterior", &RcppDirichletTreeIRV::samplePosterior)
      .method("getScale", &RcppDirichletTreeIRV::getScale);
};

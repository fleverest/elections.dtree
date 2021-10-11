// Rexports.cpp
//
// [[Rcpp::plugins("cpp17")
// [[Rcpp::depends(RcppThread)]]

#include "Rcpp/algo.h"
#include "ballot.hpp"
#include "dirichlet-tree.hpp"
#include "distributions.hpp"

#include <Rcpp.h>
#include <RcppThread.h>
#include <random>
#include <string.h>
#include <thread>

// Converts an R DataFrame representation to an election.
election dfToElection(Rcpp::DataFrame df) {
  election e = {};
  int nCandidates = df.length();
  Rcpp::IntegerVector col;

  if (df.length() < nCandidates) {
    Rcpp::Rcout << "Insufficient columns (" << df.length()
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

Rcpp::DataFrame electionToDF(election e, int nCandidates) {
  Rcpp::DataFrame out = Rcpp::DataFrame::create();
  Rcpp::IntegerVector col;
  int nBallots = e.size();

  for (int i = 0; i < nCandidates; ++i) {
    col = Rcpp::IntegerVector::create();
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

// Rcpp interface to evaluate an election outcome.
// [[Rcpp::export]]
int evaluateElection(Rcpp::DataFrame df) {
  election e = dfToElection(df);
  return evaluateElection(e);
}

// Rcpp interface to update must convert from DataFrame to election.
void update(DirichletTreeIRV *dtree, Rcpp::DataFrame ballots) {
  election bs = dfToElection(ballots);
  for (auto b : bs) {
    dtree->update(b);
  }
}

// Rcpp interface to sample leaf probabilities for a single ballot.
float sampleLeafProbability(DirichletTreeIRV *dtree,
                            Rcpp::IntegerVector ballot) {
  Ballot b(ballot.size());
  for (int i = 0; i < b.nPreferences; ++i) {
    b.ballotPermutation[i] = ballot.at(i);
  }
  float p = dtree->sampleLeafProbability(b);
  return p;
}

// R interface to sample will sample one election from the distribution.
Rcpp::DataFrame sampleBallots(DirichletTreeIRV *dtree, int nBallots) {
  Rcpp::DataFrame out = Rcpp::DataFrame::create();
  election *e = dtree->sample(1, nBallots);
  out = electionToDF(e[0], dtree->getNCandidates());
  delete[] e;
  return out;
}

// Rcpp interface to samplePosterior.
Rcpp::IntegerVector samplePosterior(DirichletTreeIRV *dtree, int nElections,
                                    int nBallots, bool useObserved,
                                    int nBatches) {
  int nCandidates = dtree->getNCandidates();
  int *output = new int[nCandidates];
  for (int i = 0; i < nCandidates; ++i) {
    output[i] = 0;
  }
  int *results[nBatches + 1];
  int electionBatchSize = nElections / nBatches;
  int electionBatchRemainder = nElections % nBatches;
  // Seed one RNG for each thread.
  std::mt19937 *treeGen = dtree->getEnginePtr();
  unsigned seed[nBatches + 1];
  for (int i = 0; i <= nBatches; ++i) {
    seed[i] = (*treeGen)();
  }
  treeGen->discard(treeGen->state_size * 100);

  // Use RcppThreads to compute the posterior in batches.
  RcppThread::ThreadPool pool(std::thread::hardware_concurrency());

  auto getBatchResult = [&](size_t i) -> void {
    RcppThread::checkUserInterrupt();
    // Seed a new PRNG, and warm it up.
    std::mt19937 e(seed[i]);
    e.discard(e.state_size * 100);
    // Sample posterior
    results[i] =
        dtree->samplePosterior(electionBatchSize, nBallots, useObserved, &e);
  };

  pool.parallelFor(0, nBatches, getBatchResult);
  pool.join();

  for (int i = 0; i <= nBatches; ++i) {
    // Sample remainder for the remainder.
    if (i == nBatches) {
      if (electionBatchRemainder == 0)
        continue;
      std::mt19937 e(seed[i]);
      e.discard(e.state_size * 100);
      results[i] = dtree->samplePosterior(electionBatchRemainder, nBallots,
                                          useObserved, &e);
    }
    for (int j = 0; j < nCandidates; ++j) {
      output[j] += results[i][j];
    }
    delete[] results[i];
  }

  Rcpp::IntegerVector out(output, output + nCandidates);
  delete[] output;
  return out;
}

RCPP_MODULE(dirichlet_tree_irv_module) {
  Rcpp::class_<DirichletTreeIRV>("DirichletTreeIRV")
      .constructor<int, float, bool, std::string>()
      .property("nCandidates", &DirichletTreeIRV::getNCandidates)
      .property("scale", &DirichletTreeIRV::getScale,
                &DirichletTreeIRV::setScale)
      .property("isDirichlet", &DirichletTreeIRV::getTreeType,
                &DirichletTreeIRV::setTreeType)
      .property("topLevelAlphas", &DirichletTreeIRV::getTopLevelAlphas)
      .method("clear", &DirichletTreeIRV::clear)
      .method("update", &update)
      .method("sampleLeafProbability", &sampleLeafProbability)
      .method("sampleBallots", &sampleBallots)
      .method("samplePosterior", &samplePosterior);
};

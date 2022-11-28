/******************************************************************************
 * File:             R_tree.cpp
 *
 * Author:           Floyd Everest <me@floydeverest.com>
 * Created:          03/01/22
 * Description:      This file implements the Rcpp interface for all of the
 *                   Dirichlet-tree methods, and for the social choice
 *                   functions.
 *****************************************************************************/

#include "R_tree.h"

std::list<IRVBallotCount> RDirichletTree::parseBallotList(Rcpp::List bs) {
  Rcpp::CharacterVector namePrefs;
  std::string cName;
  std::list<unsigned> indexPrefs;
  size_t cIndex;

  std::list<IRVBallotCount> out;

  // We iterate over each ballot, and convert it into an IRVBallotCount using
  // the "candidate index" for each seen candidate.
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
    out.emplace_back(std::move(indexPrefs), 1);
  }

  return out;
}

RDirichletTree::RDirichletTree(Rcpp::CharacterVector candidates,
                               unsigned minDepth_, unsigned maxDepth_,
                               double a0_, bool vd_, std::string seed_) {
  // Parse the candidate strings.
  std::string cName;
  size_t cIndex = 0;
  for (auto i = 0; i < candidates.size(); ++i) {
    cName = candidates[i];
    candidateVector.push_back(cName);
    candidateMap[cName] = cIndex;
    ++cIndex;
  }
  // Initialize tree.
  IRVParameters *params =
      new IRVParameters(candidates.size(), minDepth_, maxDepth_, a0_, vd_);
  tree = new DirichletTree<IRVNode, IRVBallot, IRVParameters>(params, seed_);
}

// Destructor.
RDirichletTree::~RDirichletTree() {
  delete tree->getParameters();
  delete tree;
}

// Getters
unsigned RDirichletTree::getNCandidates() {
  return tree->getParameters()->getNCandidates();
}
unsigned RDirichletTree::getMinDepth() {
  return tree->getParameters()->getMinDepth();
}
unsigned RDirichletTree::getMaxDepth() {
  return tree->getParameters()->getMaxDepth();
}
double RDirichletTree::getA0() { return tree->getParameters()->getA0(); }
bool RDirichletTree::getVD() { return tree->getParameters()->getVD(); }
Rcpp::CharacterVector RDirichletTree::getCandidates() {
  Rcpp::CharacterVector out{};
  for (const auto &[candidate, idx] : candidateMap) out.push_back(candidate);
  return out;
}

// Setters
void RDirichletTree::setMinDepth(unsigned minDepth_) {
  if (minDepth_ > tree->getParameters()->getMaxDepth())
    Rcpp::stop("Cannot set `minDepth` to a value larger than `maxDepth`.");
  tree->getParameters()->setMinDepth(minDepth_);
  // If the tree is reducible to a Dirichlet distribution,
  // we need to check that the ballots observed so far do not
  // violate len(ballot) < minDepth - otherwise the resulting
  // posterior will not be Dirichlet.
  for (const auto &d : observedDepths) {
    if (d < minDepth_ && d > 0) {
      Rcpp::warning(
          "Ballots with fewer than `minDepth` preferences specified "
          "have been observed. Some sampling techniques could now exhibit "
          "undefined behaviour. A Dirichlet Posterior can no longer reduce to "
          "a tree of height 1. Consider setting `minDepth` to a value lower "
          "than the length of the smallest ballot.");
      break;
    }
  }
}

void RDirichletTree::setMaxDepth(unsigned maxDepth_) {
  if (maxDepth_ < tree->getParameters()->getMinDepth())
    Rcpp::stop("Cannot set `maxDepth` to a value less than `minDepth`.");
  tree->getParameters()->setMaxDepth(maxDepth_);
}

void RDirichletTree::setA0(double a0_) { tree->getParameters()->setA0(a0_); }

void RDirichletTree::setVD(bool vd_) { tree->getParameters()->setVD(vd_); }

// Other methods
void RDirichletTree::reset() {
  tree->reset();
  nObserved = 0;
  observedDepths.clear();
}

void RDirichletTree::update(Rcpp::List ballots) {
  // For checking validitity of inputs.
  unsigned minDepth = tree->getParameters()->getMinDepth();
  unsigned depth;
  // Parse the ballots.
  std::list<IRVBallotCount> bcs = parseBallotList(ballots);
  for (IRVBallotCount &bc : bcs) {
    // If the tree is reducible to a Dirichlet distribution,
    // we need to check that the observed ballot length is >=
    // the minDepth of the tree, otherwise the posterior tree
    // will no longer be reducible to a Dirichlet distribution.
    // This does not apply if the ballot has length zero, since
    // it will be essentially ignored whenever minDepth > 0.
    depth = bc.first.nPreferences();
    if (depth < minDepth && depth > 0)
      Rcpp::warning(
          "Updating a Dirichlet-tree distribution with a ballot "
          "specifying fewer than `minDepth` preferences. This introduces "
          "undefined behaviour to the sampling methods, and the "
          "resulting posterior can no longer reduce to a Dirichlet "
          "distribution when using the `vd` option. Consider setting "
          "`minDepth` to a value lower than the length of the smallest "
          "ballot.");
    // Update the tree with count * the ballot.
    nObserved += bc.second;
    tree->update(bc);
    observedDepths.insert(depth);
  }
}

Rcpp::List RDirichletTree::samplePredictive(unsigned nSamples,
                                            std::string seed) {
  tree->setSeed(seed);

  Rcpp::List out;
  Rcpp::CharacterVector rBallot;

  std::list<IRVBallotCount> samples = tree->sample(nSamples);
  for (auto &[b, count] : samples) {
    // Push count * b to the list.
    for (unsigned i = 0; i < count; ++i) {
      rBallot = Rcpp::CharacterVector::create();
      for (auto cIndex : b.preferences) {
        rBallot.push_back(candidateVector[cIndex]);
      }
      out.push_back(rBallot);
    }
  }

  return out;
}

Rcpp::NumericVector RDirichletTree::samplePosterior(unsigned nElections,
                                                    unsigned nBallots,
                                                    unsigned nWinners,
                                                    bool replace,
                                                    unsigned nThreads,
                                                    std::string seed) {
  if (nBallots < nObserved)
    Rcpp::stop(
        "`nBallots` must be larger than the number of ballots "
        "observed to obtain the posterior.");

  tree->setSeed(seed);

  size_t nCandidates = getNCandidates();

  // Generate PRNG seeds.
  std::mt19937 *treeGen = tree->getEnginePtr();
  std::vector<unsigned> seeds{};
  for (unsigned i = 0; i <= nThreads; ++i) {
    seeds.push_back((*treeGen)());
  }

  // The number of elections to sample per batch.
  unsigned batchSize, batchRemainder;
  if (nElections <= 1) {
    batchSize = 0;
    batchRemainder = nElections;
  } else {
    batchSize = nElections / nThreads;
    batchRemainder = nElections % nThreads;
  }

  // The results vector for each thread.
  std::vector<std::vector<unsigned>> results(nElections);

  // Use multiple threads to compute the posterior in batches.
  auto processBatch = [&](size_t thread_idx, size_t size) -> void {
    // Seed a new PRNG, and warm it up.
    std::mt19937 e(seeds[thread_idx]);
    e.discard(e.state_size * 100);

    for (unsigned j = 0; j < size; ++j) {
      // Check for interrupt.
      RcppThread::checkUserInterrupt();
      // Simulate election.
      std::list<IRVBallotCount> election = tree->posteriorSet(nBallots, replace, &e);
      // Evaluate social choice function.
      results[thread_idx * batchSize + j] =
          socialChoiceIRV(election, nCandidates, &e);
    }
  };

  // Dispatch the jobs
  std::vector<std::thread> pool(nThreads);
  for (unsigned i = 0; i < nThreads; ++i) {
    pool[i] = std::thread(std::bind(processBatch, i, batchSize));
  }

  // Process remainder on main thread.
  processBatch(nThreads, batchRemainder);

  // Join the threads
  std::for_each(pool.begin(), pool.end(), [](std::thread &t) { t.join(); });

  // Aggregate the results
  Rcpp::NumericVector out(nCandidates);
  out.names() = candidateVector;
  for (unsigned j = 0; j < nElections; ++j) {
    for (auto i = nCandidates - nWinners; i < nCandidates; ++i)
      out[results[j][i]] = out[results[j][i]] + 1;
  }

  out = out / nElections;
  return out;
}

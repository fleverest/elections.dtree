/******************************************************************************
 * File:             RcppIRV.cpp
 *
 * Author:           Floyd Everest <me@floydeverest.com>
 * Created:          03/01/22
 * Description:      This file implements an Rcpp interface for the IRV
 *                   Dirichlet Tree methods, and for the IRV social choice
 *                   function.
 *****************************************************************************/

#include "RcppIRV.h"
#include "R.h"

// [[Rcpp::plugins("cpp17")]]
// [[Rcpp::depends(RcppThread)]]

// [[Rcpp::export]]
Rcpp::List RSocialChoiceIRV(Rcpp::List bs, unsigned nWinners,
                            Rcpp::CharacterVector candidates,
                            std::string seed) {

  Rcpp::List out{};

  std::list<IRVBallotCount> scInput{};

  std::unordered_map<std::string, size_t> c2Index{};
  std::vector<std::string> cNames{};
  // If candidates vector is not null, initialze some indices.
  std::string cName;
  for (const auto &candidate : candidates) {
    cName = candidate;
    if (c2Index.count(cName) == 0) {
      c2Index[cName] = cNames.size();
      cNames.push_back(cName);
    }
  }

  Rcpp::CharacterVector bNames;
  std::list<unsigned> bIndices;
  bool newBallot = true;

  for (auto i = 0; i < bs.size(); ++i) {
    if (bs[i] == R_NilValue) // Skip empty ballots
      continue;
    bNames = bs[i];
    bIndices = {};
    for (auto j = 0; j < bNames.size(); ++j) {
      cName = bNames[j];
      // If candidate has not yet been seen, raise an error.
      if (c2Index.count(cName) == 0)
        Rcpp::stop("Invalid candidate found during social-choice evaluation.");
      bIndices.push_back(c2Index[cName]);
    }

    // Search for the same ballot already in the social choice input.
    IRVBallot b(bIndices);
    newBallot = true;
    for (auto &bc : scInput) {
      if (b == bc.first) {
        bc.second = bc.second + 1;
        newBallot = false;
      }
    }
    // If it's not already there, add it to the back of the list with count 1.
    if (newBallot)
      scInput.emplace_back(std::move(b), 1);
  }

  if (nWinners < 1 || nWinners >= cNames.size())
    Rcpp::stop("`nWinners` must be >= 1 and <= the number of candidates.");

  if (scInput.size() == 0)
    Rcpp::stop("No valid ballots for the IRV social choice function.");

  // Seed the PRNG.
  std::seed_seq ss(seed.begin(), seed.end());
  std::mt19937 e(ss);
  e.discard(e.state_size * 100);

  // Group all equal ballots

  std::vector<unsigned> elimination_order_idx =
      socialChoiceIRV(scInput, cNames.size(), &e);

  Rcpp::CharacterVector elimination_order{};
  Rcpp::CharacterVector winners{};

  for (size_t i = 0; i < cNames.size() - nWinners; ++i) {
    elimination_order.push_back(cNames[elimination_order_idx[i]]);
  }
  for (size_t i = cNames.size() - nWinners; i < cNames.size(); ++i) {
    winners.push_back(cNames[elimination_order_idx[i]]);
  }

  out("elimination_order") = elimination_order;
  out("winners") = winners;

  return out;
}

std::list<IRVBallotCount> PIRVDirichletTree::parseBallotList(Rcpp::List bs) {
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

PIRVDirichletTree::PIRVDirichletTree(Rcpp::CharacterVector candidates,
                                     unsigned minDepth_, unsigned maxDepth_,
                                     float a0_, bool vd_, std::string seed_) {
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
PIRVDirichletTree::~PIRVDirichletTree() {
  delete tree->getParameters();
  delete tree;
}

// Getters
unsigned PIRVDirichletTree::getNCandidates() {
  return tree->getParameters()->getNCandidates();
}
unsigned PIRVDirichletTree::getMinDepth() {
  return tree->getParameters()->getMinDepth();
}
unsigned PIRVDirichletTree::getMaxDepth() {
  return tree->getParameters()->getMaxDepth();
}
float PIRVDirichletTree::getA0() { return tree->getParameters()->getA0(); }
bool PIRVDirichletTree::getVD() { return tree->getParameters()->getVD(); }
Rcpp::CharacterVector PIRVDirichletTree::getCandidates() {
  Rcpp::CharacterVector out{};
  for (const auto &[candidate, idx] : candidateMap)
    out.push_back(candidate);
  return out;
}

// Setters
void PIRVDirichletTree::setMinDepth(unsigned minDepth_) {
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

void PIRVDirichletTree::setMaxDepth(unsigned maxDepth_) {
  if (maxDepth_ < tree->getParameters()->getMinDepth())
    Rcpp::stop("Cannot set `maxDepth` to a value less than `minDepth`.");
  tree->getParameters()->setMaxDepth(maxDepth_);
}

void PIRVDirichletTree::setA0(float a0_) { tree->getParameters()->setA0(a0_); }

void PIRVDirichletTree::setVD(bool vd_) { tree->getParameters()->setVD(vd_); }

// Other methods
void PIRVDirichletTree::reset() {
  tree->reset();
  nObserved = 0;
  observedDepths.clear();
}

void PIRVDirichletTree::update(Rcpp::List ballots) {
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
          "Updating a Dirichlet-Tree distribution with a ballot "
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

Rcpp::List PIRVDirichletTree::samplePredictive(unsigned nSamples,
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

Rcpp::NumericVector PIRVDirichletTree::samplePosterior(unsigned nElections,
                                                       unsigned nBallots,
                                                       unsigned nWinners,
                                                       unsigned nBatches,
                                                       std::string seed) {

  if (nBallots < nObserved)
    Rcpp::stop("`nBallots` must be larger than the number of ballots "
               "observed to obtain the posterior.");

  tree->setSeed(seed);

  size_t nCandidates = getNCandidates();

  // Generate nBatches PRNGs.
  std::mt19937 *treeGen = tree->getEnginePtr();
  std::vector<unsigned> seeds{};
  for (unsigned i = 0; i <= nBatches; ++i) {
    seeds.push_back((*treeGen)());
  }
  // TODO: Remove this?
  treeGen->discard(treeGen->state_size * 100);

  // The number of elections to sample per thread.
  unsigned batchSize, batchRemainder;
  if (nElections <= 1) {
    batchSize = 0;
    batchRemainder = nElections;
  } else {
    batchSize = nElections / nBatches;
    batchRemainder = nElections % nBatches;
  }

  // The results vector for each thread.
  std::vector<std::vector<std::vector<unsigned>>> results(nBatches + 1);

  // Use RcppThreads to compute the posterior in batches.
  auto getBatchResult = [&](size_t i, size_t batchSize) -> void {
    // Check for interrupt.
    RcppThread::checkUserInterrupt();

    // Seed a new PRNG, and warm it up.
    std::mt19937 e(seeds[i]);
    e.discard(e.state_size * 100);

    // Simulate elections.
    std::list<std::list<IRVBallotCount>> elections =
        tree->posteriorSets(batchSize, nBallots, &e);

    for (auto &el : elections)
      results[i].push_back(socialChoiceIRV(el, nCandidates, &e));
  };

  // Dispatch the jobs.
  RcppThread::ThreadPool pool(std::thread::hardware_concurrency());

  // Process batches on workers
  pool.parallelFor(0, nBatches,
                   [&](size_t i) { getBatchResult(i, batchSize); });

  // Process remainder on main thread.
  if (batchRemainder > 0)
    getBatchResult(nBatches, batchRemainder);

  pool.join();

  // Aggregate the results
  Rcpp::NumericVector out(nCandidates);
  out.names() = candidateVector;

  for (unsigned j = 0; j <= nBatches; ++j) {
    for (auto elimination_order_idx : results[j]) {
      for (auto i = nCandidates - nWinners; i < nCandidates; ++i)
        out[elimination_order_idx[i]] = out[elimination_order_idx[i]] + 1;
    }
  }

  out = out / nElections;
  return out;
}

// The Rcpp module interface.
RCPP_MODULE(pirv_dirichlet_tree_module) {
  Rcpp::class_<PIRVDirichletTree>("PIRVDirichletTree")
      // Constructor needs nCandidates, minDepth, a0 and seed.
      .constructor<Rcpp::CharacterVector, unsigned, unsigned, float, bool,
                   std::string>()
      // Getter/Setter interface
      .property("nCandidates", &PIRVDirichletTree::getNCandidates)
      .property("a0", &PIRVDirichletTree::getA0, &PIRVDirichletTree::setA0)
      .property("minDepth", &PIRVDirichletTree::getMinDepth,
                &PIRVDirichletTree::setMinDepth)
      .property("maxDepth", &PIRVDirichletTree::getMaxDepth,
                &PIRVDirichletTree::setMaxDepth)
      .property("vd", &PIRVDirichletTree::getVD, &PIRVDirichletTree::setVD)
      .property("candidates", &PIRVDirichletTree::getCandidates)
      // Methods
      .method("reset", &PIRVDirichletTree::reset)
      .method("update", &PIRVDirichletTree::update)
      .method("samplePredictive", &PIRVDirichletTree::samplePredictive)
      .method("samplePosterior", &PIRVDirichletTree::samplePosterior);
}

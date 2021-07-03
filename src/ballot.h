#ifndef BALLOT_H
#define BALLOT_H

#include <algorithm>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

struct BallotCount {
  int *ballotPermutation; // Ballot represented as a permutation.
  int count;              // The number of occurances of that ballot in the set.
};

typedef std::vector<BallotCount> election;

// Formats a BallotCount as a string.
std::string bcToStr(BallotCount bc, int nCandidates) {
  std::ostringstream out;
  for (int i = 0; i < nCandidates; ++i) {
    out << bc.ballotPermutation[i] << ",";
  }
  out << bc.count;
  return out.str();
}

// Formats an election as a stringe
std::string electionToStr(election e, int nCandidates) {
  std::ostringstream out;
  // Format header
  for (int i = 0; i < nCandidates; ++i) {
    out << "choice" << i + 1 << ",";
  }
  out << "count" << std::endl;

  // Add each ballot count line
  for (BallotCount bc : e) {
    out << bcToStr(bc, nCandidates) << std::endl;
  }

  return out.str();
}

// Evaluates an election outcome, returns the winning candidate.
int evaluateElection(election e, int nCandidates) {
  std::vector<std::vector<BallotCount>> candidateBallotCounts = {};
  int idx;
  int *counts = new int[nCandidates];
  bool *isEliminated = new bool[nCandidates];
  int nEliminated = 0;
  for (int i = 0; i < nCandidates; ++i) {
    isEliminated[i] = false;
    candidateBallotCounts.push_back(std::vector<BallotCount>());
    counts[i] = 0;
  }

  for (long unsigned int i = 0; i < e.size(); ++i) {
    idx = e[i].ballotPermutation[0] - 1;
    candidateBallotCounts[idx].push_back(e[i]);
  }

  while (nEliminated < nCandidates - 1) {
    // Calculate count sums for each candidate.
    for (int i = 0; i < nCandidates; ++i) {
      counts[i] = 0;
      for (long unsigned int j = 0; j < candidateBallotCounts[i].size(); ++j) {
        counts[i] += candidateBallotCounts[i][j].count;
      }
      if (isEliminated[i]) {
        counts[i] = std::numeric_limits<int>::max();
      }
    }
    // Find non-eliminated candidate with lowest next-preference count.
    idx = std::distance(counts, std::min_element(counts, counts + nCandidates));
    // Distribute the ballots of this candidate.
    // Mark as eliminated.
    isEliminated[idx] = true;
    ++nEliminated;
    for (BallotCount bc : candidateBallotCounts[idx]) {
      // Iterate ballot to next preference.
      bc.ballotPermutation = bc.ballotPermutation + 1;
      // Add ballot to the appropriate candidate.
      candidateBallotCounts[bc.ballotPermutation[0] - 1].push_back(bc);
    }
  }

  delete[] counts;

  idx = std::distance(
      isEliminated, std::min_element(isEliminated, isEliminated + nCandidates));
  delete[] isEliminated;
  return idx + 1;
}

#endif

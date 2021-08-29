// ballot.cpp

#include "ballot.hpp"

// Default constructor
Ballot::Ballot(int nPreferences) : nPreferences(nPreferences) {
  ballotPermutation = new int[nPreferences];
}
// Copy constructor
Ballot::Ballot(const Ballot &obj)
    : nPreferences(obj.nPreferences),
      ballotPermutation(new int[obj.nPreferences]) {
  for (auto i = 0; i < nPreferences; ++i) {
    ballotPermutation[i] = obj.ballotPermutation[i];
  }
}
// Destructor
Ballot::~Ballot() { delete[] ballotPermutation; }
// Nextpref method to move ballot on to its next preference
void Ballot::nextPref() {
  int *newBallotPermutation = new int[nPreferences - 1];
  for (auto i = 0; i < nPreferences - 1; ++i) {
    newBallotPermutation[i] = ballotPermutation[i + 1];
  }
  delete[] ballotPermutation;
  this->ballotPermutation = newBallotPermutation;
  this->nPreferences = nPreferences - 1;
}

// Formats a BallotCount as a string.
std::string bToStr(Ballot b) {
  std::ostringstream out;
  for (int i = 0; i < b.nPreferences; ++i) {
    out << b.ballotPermutation[i];
    if (i < b.nPreferences - 1)
      out << ",";
  }
  return out.str();
}

// Formats an election as a string
void electionToCSV(election e, int nCandidates, std::string out) {
  std::ofstream outfile;
  outfile.open(out, std::ofstream::out);
  if (!outfile.is_open()) {
    throw std::invalid_argument("CSV output file cannot be opened");
  }

  // Format header
  for (int i = 0; i < nCandidates; ++i) {
    outfile << "choice" << i + 1 << ",";
  }
  outfile << "count" << std::endl;

  // Add each ballot count line
  for (Ballot b : e) {
    outfile << bToStr(b);
    // Fill empty columns with ','
    for (int i = 0; i < nCandidates - b.nPreferences; ++i) {
      outfile << ",";
    }
    outfile << std::endl;
  }
}

// Evaluates an election outcome, returns the winning candidate.
int evaluateElection(election e) {
  // TODO: Implement STV
  int nCandidates = e[0].nPreferences;
  std::vector<election> candidateBallots = {};
  int idx;
  int *counts = new int[nCandidates];
  bool *isEliminated = new bool[nCandidates];
  int nEliminated = 0;
  for (int i = 0; i < nCandidates; ++i) {
    isEliminated[i] = false;
    candidateBallots.push_back(election());
    counts[i] = 0;
  }

  for (long unsigned int i = 0; i < e.size(); ++i) {
    idx = e[i].ballotPermutation[0] - 1;
    candidateBallots[idx].push_back(e[i]);
  }

  while (nEliminated < nCandidates - 1) {
    // Calculate count sums for each candidate.
    for (int i = 0; i < nCandidates; ++i) {
      if (isEliminated[i]) {
        counts[i] = std::numeric_limits<int>::max();
      } else {
        counts[i] = candidateBallots[i].size();
      }
    }
    // Find non-eliminated candidate with lowest next-preference count.
    idx = std::distance(counts, std::min_element(counts, counts + nCandidates));
    // Distribute the ballots of this candidate.
    // Mark as eliminated.
    isEliminated[idx] = true;
    ++nEliminated;
    for (Ballot b : candidateBallots[idx]) {
      // Move ballot to next preference.
      b.nextPref();
      // Add ballot to the appropriate candidate.
      candidateBallots[b.ballotPermutation[0] - 1].push_back(b);
    }
  }

  delete[] counts;

  idx = std::distance(
      isEliminated, std::min_element(isEliminated, isEliminated + nCandidates));
  delete[] isEliminated;
  return idx + 1;
}

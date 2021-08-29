// ballot.hpp

#ifndef BALLOT_HPP
#define BALLOT_HPP

#include <algorithm>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

struct Ballot {
  int nPreferences; // The number of preferences selected. In IRV elections this
                    // must be equal to the number of candidates.
  int *ballotPermutation; // Ballot represented as a permutation.
  // Default constructor
  Ballot(int nPreferences);
  // Copy Constructor
  Ballot(const Ballot &b);
  // Destructor
  ~Ballot();
  // Move ballot on to next preference
  void nextPref();
};

typedef std::vector<Ballot> election;

// Formats a BallotCount as a string.
std::string bToStr(Ballot b);

// Formats an election as a string
void electionToCSV(election e, int nCandidates, std::string out);

// Evaluates an election outcome, returns the winning candidate.
int evaluateElection(election e);

#endif

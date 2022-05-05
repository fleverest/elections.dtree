/******************************************************************************
 * File:             irv_ballot.cpp
 *
 * Author:           Floyd Everest <me@floydeverest.com>
 * Created:          02/27/22
 * Description:      This file implements the IRVBallot methods as outlined
 *                   in `irv_ballot.hpp`.
 *****************************************************************************/

#include "irv_ballot.hpp"

IRVBallot::IRVBallot(std::vector<unsigned> preferences_)
    : preferences(preferences_) {}

IRVBallot::IRVBallot(const IRVBallot &obj) : preferences(obj.preferences) {}

bool IRVBallot::eliminate(unsigned candidate) {
  // Find the first occurance of the candidate in the ballot.
  auto it = std::find(preferences.begin(), preferences.end(), candidate);
  // If it occurs then erase it.
  if (it != preferences.end()) {
    preferences.erase(it);
  }
  // Return whether or not the ballot is empty.
  if (nPreferences() == 0) {
    return true;
  } else {
    return false;
  }
}

bool IRVBallot::operator==(const IRVBallot &b) {
  // First check the number of specified candidates is equal.
  if (!(nPreferences() == b.nPreferences())) {
    return false;
  }
  // Then check each path element to ensure they are equal.
  bool equal = true;
  for (auto i = 0; i < nPreferences(); ++i) {
    equal = equal && (preferences[i] == b.preferences[i]);
  }
  return equal;
}

std::vector<unsigned> socialChoiceIRV(std::list<IRVBallot> ballots,
                                      unsigned nCandidates) {

  std::vector<unsigned> out{};

  // A copy of the ballots which will be altered during eliminations.
  std::list<IRVBallot> altered_ballots = ballots;

  // Filter out the empty ballots, as these are not useless to the
  // social choice function.
  altered_ballots.remove_if([](IRVBallot b) { return b.nPreferences() == 0; });

  unsigned nEliminations = 0;

  // An array of booleans representing whether or not the candidate index has
  // been eliminated.
  std::vector<bool> eliminated(nCandidates, false);

  // The minimum tally among standing candidates.
  unsigned min_tally;

  // The index of the next candidate to be eliminated.
  unsigned elim;

  // The current tally of first-preferences for each candidate.
  std::vector<unsigned> tally;

  // While more than one candidate stands.
  while (nEliminations < nCandidates) {

    // Reset the tally.
    tally = std::vector<unsigned>(nCandidates, 0);

    // Tally the first preferences of each ballot.
    for (auto b : altered_ballots)
      tally[b.firstPreference()] += 1;

    // Determine which candidate is to be eliminated this round.
    elim = 0;
    min_tally = std::numeric_limits<unsigned>::max();
    for (auto i = 0; i < nCandidates; ++i) {
      if (!eliminated[i] && min_tally > tally[i]) {
        elim = i;
        min_tally = tally[i];
      }
    }

    // Eliminate the standing candidate with the minimum tally.
    eliminated[elim] = true;
    out.push_back(elim);

    altered_ballots.remove_if(
        [elim](IRVBallot b) mutable { return b.eliminate(elim); });

    ++nEliminations;
  }

  return out;
}

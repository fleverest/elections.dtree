/******************************************************************************
 * File:             irv_ballot.cpp
 *
 * Author:           Floyd Everest <me@floydeverest.com>
 * Created:          02/27/22
 * Description:      This file implements the IRVBallot methods as outlined
 *                   in `irv_ballot.hpp`.
 *****************************************************************************/

#include "irv_ballot.hpp"

IRVBallot::IRVBallot(std::vector<int> preferences_)
    : preferences(preferences_) {}

IRVBallot::IRVBallot(const IRVBallot &obj) : preferences(obj.preferences) {}

bool IRVBallot::eliminate(int candidate) {
  // Find the first occurance of the candidate in the ballot.
  auto it = std::find(preferences.begin(), preferences.end(), candidate);
  // If it occurs then erase it.
  if (it != preferences.end()) {
    preferences.erase(it);
  }
  // Return whether or not the ballot remains valid.
  if (nPreferences() == 0) {
    return false;
  } else {
    return true;
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

int socialChoiceIRV(std::list<IRVBallot> ballots, int nCandidates) {

  // A copy of the ballots which will be altered during eliminations.
  std::list<IRVBallot> altered_ballots = ballots;

  bool valid = true;

  int nEliminations = 0;

  // An array of booleans representing whether or not the candidate index has
  // been eliminated.
  std::vector<bool> eliminated(nCandidates, false);

  // The minimum tally among standing candidates.
  int min_tally;
  // The maximum tally
  int max_tally;

  // The index of the next candidate to be eliminated.
  int elim;

  // The index of the victor
  int victor;

  // The current tally of first-preferences for each candidate.
  std::vector<int> tally;

  // While more than one candidate stands.
  while (nEliminations < nCandidates - 2) {

    // Reset the tally.
    tally = std::vector<int>(nCandidates, 0);

    // Tally the first preferences of each ballot.
    for (auto b : altered_ballots) {
      tally[b.firstPreference()] += 1;
    }

    // Determine which candidate is to be eliminated this round.
    elim = 0;
    min_tally = std::numeric_limits<int>::max();
    for (auto i = 0; i < nCandidates; ++i) {
      if (!eliminated[i] && min_tally > tally[i]) {
        elim = i;
        min_tally = tally[i];
      }
    }

    // Eliminate the standing candidate with the minimum tally.
    eliminated[elim] = true;
    altered_ballots.erase(std::remove_if(altered_ballots.begin(),
                                         altered_ballots.end(),
                                         [valid, elim](IRVBallot b) mutable {
                                           valid = b.eliminate(elim);
                                           return !valid;
                                         }));

    ++nEliminations;
  }

  // Find which candidate has won.

  // Reset the tally one last time.
  tally = std::vector<int>(nCandidates, 0);

  // Tally the first preferences of each ballot.
  for (auto b : altered_ballots) {
    tally[b.firstPreference()] += 1;
  }

  // Determine the winner.
  victor = 0;
  max_tally = 0;
  for (auto i = 0; i < nCandidates; ++i) {
    if (max_tally < tally[i]) {
      victor = i;
      max_tally = tally[i];
    }
  }

  return victor;
}

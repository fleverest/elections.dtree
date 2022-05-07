/******************************************************************************
 * File:             irv_ballot.cpp
 *
 * Author:           Floyd Everest <me@floydeverest.com>
 * Created:          02/27/22
 * Description:      This file implements the IRVBallot methods as outlined
 *                   in `irv_ballot.hpp`.
 *****************************************************************************/

#include "irv_ballot.hpp"

#include <algorithm>

IRVBallot::IRVBallot(std::list<unsigned> preferences_) {
  preferences = std::move(preferences_);
}

IRVBallot::IRVBallot(const IRVBallot &b) { preferences = b.preferences; }

bool IRVBallot::eliminateFirstPref() {
  preferences.pop_front();
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
  // Then check each preference to ensure they are equal.
  return std::equal(preferences.begin(), preferences.end(),
                    b.preferences.begin());
}

std::vector<unsigned> socialChoiceIRV(std::list<IRVBallotCount> ballots,
                                      unsigned nCandidates) {

  unsigned firstPref;
  bool isEmpty;

  std::vector<unsigned> out{};

  // Filter out the empty ballots, as these are useless to the
  // social choice function.
  ballots.remove_if(
      [](IRVBallotCount &b) { return b.first.nPreferences() == 0; });

  unsigned nEliminations = 0;

  // An array of booleans representing whether or not the candidate index has
  // been eliminated.
  std::vector<bool> eliminated(nCandidates, false);

  // The minimum tally among standing candidates.
  unsigned min_tally;

  // The index of the next candidate to be eliminated.
  unsigned elim;

  // Vector of lists of iterators to the ballotcounts which contribute to the
  // tally for each candidate.
  std::vector<std::list<std::list<IRVBallotCount>::iterator>> tally_groups(
      nCandidates);

  // Tally the initial first preferences for each ballot.
  for (auto it = ballots.begin(); it != ballots.end(); ++it) {
    firstPref = it->first.firstPreference();
    tally_groups[firstPref].push_back(it);
  }

  // While more than one candidate stands.
  while (nEliminations < nCandidates - 1) {

    // Determine which candidate is to be eliminated this round.
    elim = 0;
    min_tally = std::numeric_limits<unsigned>::max();
    for (unsigned i = 0; i < nCandidates; ++i) {
      if (!eliminated[i] && min_tally > tally_groups[i].size()) {
        elim = i;
        min_tally = tally_groups[i].size();
      }
    }

    // Eliminate the standing candidate with the minimum tally.
    eliminated[elim] = true;
    out.push_back(elim);

    // Redistribute the ballots attributed to the losing candidate.
    auto list_start = tally_groups[elim].begin();
    auto list_end = tally_groups[elim].end();
    while (list_start != list_end) {
      // Delete all eliminated candidates from the start of the ballot.
      firstPref = (*list_start)->first.firstPreference();
      while (eliminated[firstPref]) {
        // Check if the ballot was emptied. If so, we break now.
        isEmpty = (*list_start)->first.eliminateFirstPref();
        if (isEmpty)
          break;
        // Otherwise, continue looking for a standing next-preference.
        firstPref = (*list_start)->first.firstPreference();
      }
      if (isEmpty) {
        // If the resulting ballot was emptied, then we delete it from
        // the full set of ballots, and we don't redistribute it.
        ballots.erase(*list_start);
      } else {
        // If it is not empty, we add the ballotcount to the next *standing*
        // candidates' tally.
        tally_groups[firstPref].push_back(*list_start);
      }
      // Now that the ballot has been redistributed, continue
      list_start = tally_groups[elim].erase(list_start);
    }
    ++nEliminations;
  }

  // Push the last standing to the back.
  for (unsigned i = 0; i < nCandidates; ++i) {
    if (!eliminated[i]) {
      out.push_back(i);
      break;
    }
  }

  return out;
}

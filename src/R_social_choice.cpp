/******************************************************************************
 * File:             R_social_choice.cpp
 *
 * Author:           Floyd Everest <me@floydeverest.com>
 * Created:          07/23/22
 * Description:      This file defined the R interfaces to the social choice
 *                   functions.
 *****************************************************************************/

#include "R_social_choice.h"

// [[Rcpp::plugins("cpp17")]]
// [[Rcpp::depends(RcppThread)]]

// [[Rcpp::export]]
Rcpp::List social_choice_irv(Rcpp::List bs, unsigned nWinners,
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

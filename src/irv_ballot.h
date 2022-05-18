/******************************************************************************
 * File:             ballot.hpp
 *
 * Author:           Floyd Everest <me@floydeverest.com>
 * Created:          02/26/22
 * Description:      This file declares the IRVBallot type. A complete IRV
 *                   ballot is a permutation on N candidates. A partial IRV
 *                   ballot is one which gives a partial ordering of the N
 *                   candidates.
 *****************************************************************************/

#ifndef IRV_BALLOT_H
#define IRV_BALLOT_H

#include <algorithm>
#include <limits>
#include <list>
#include <random>
#include <sstream>
#include <string>
#include <vector>

class IRVBallot {
public:
  // The IRV Ballot in list representation with candidate indices in order of
  // preference as elements, e.g. {0, 1, 2, 3, 4} or {4, 3, 2}.
  std::list<unsigned> preferences;

  /*! \brief The IRVBallot constructor.
   *
   * \param preferences A list representation of an IRV ballot consisting of
   * candidate indices in order of preference.
   *
   * \return A ballot with the specified preferences.
   */
  IRVBallot(std::list<unsigned> preferences);

  /*! \brief Returns the number of preferences specified by the ballot
   *
   *  Returns the number of preferences specified in the ballot. For example, in
   * an IRV election with 5 candidates, the ballot {0, 1, 2, 3, 4} specifies 5
   * preferences, and the ballot {0, 1, 2} specifies 3.
   *
   * \return The number of specified preferences.
   */
  unsigned nPreferences() const { return preferences.size(); }

  /*! \brief Returns the first preference of the ballot.
   *
   *  Returns the candidate index corresponding to the first preference of the
   * ballot.
   *
   * \return The first preference of the ballot.
   */
  unsigned firstPreference() const { return preferences.front(); }

  /*! \brief Eliminate the first candidate.
   *
   * Eliminates the first preference from the ballot e.g.
   * if the ballot is {1, 2, 3, 4}, then after eliminate it will be
   * {2, 3, 4}.
   * If the resulting ballot is empty, this returns true.
   */
  bool eliminateFirstPref();

  /*! \brief Returns whether the provided ballot is equal to this one.
   *
   *  Checks whether another instance of IRVBallot represents the same ballot.
   *
   * \param b Another IRVBallot to compare with this one.
   *
   * \return A boolean representing whether or not the two ballots are equal.
   */
  bool operator==(const IRVBallot &b);

  /*! \breif Defines a comparison < on IRV ballots.
   *
   * An IRVBallot b is convered less than b2 if its starting preferences
   * are all less than b2s' equivalent preferences.
   *
   * \param b Another IRVBallot to compare with this one.
   *
   * \return A boolean representing whether this ballot is less than b2.
   */
  bool operator<(const IRVBallot &b) const;
};

typedef std::pair<IRVBallot, unsigned> IRVBallotCount;

/*! \brief Evaluates the outcome of an IRV election.
 *
 *  Given a set of ballots, this applies the social choice function to determine
 * the elimination order.
 *
 * \param ballotcounts A reference to a set of ballot counts to conduct the
 * social choice function with. The reference object will be deleted.
 *
 * \param engine A pointer to a mt19937 PRNG for tie-breaking.
 *
 * \return A list of candidate indices in order of elimination.
 */
std::vector<unsigned> socialChoiceIRV(std::list<IRVBallotCount> &ballotcounts,
                                      unsigned nCandidates,
                                      std::mt19937 *engine);

#endif /* IRV_BALLOT_H */

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
#include <sstream>
#include <string>
#include <vector>

class IRVBallot {
public:
  // The IRV Ballot in vector representation with candidate indices in order of
  // preference as elements, e.g. {0, 1, 2, 3, 4} or {4, 3, 2}.
  std::vector<int> preferences;

  /*! \brief The IRVBallot constructor.
   *
   * \param preferences A vector representation of an IRV ballot consisting of
   * candidate indices in order of preference.
   *
   * \return A ballot with the specified preferences.
   */
  IRVBallot(std::vector<int> preferences);

  /*! \brief The copy constructor for IRVBallot.
   *
   * \param b An IRVBallot.
   *
   * \return A new ballot which is exactly the same as b.
   */
  IRVBallot(const IRVBallot &b);

  /*! \brief The IRVBallot destructor.
   */
  ~IRVBallot();

  /*! \brief Returns the number of preferences specified by the ballot
   *
   *  Returns the number of preferences specified in the ballot. For example, in
   * an IRV election with 5 candidates, the ballot {0, 1, 2, 3, 4} specifies 5
   * preferences, and the ballot {0, 1, 2} specifies 3.
   *
   * \return The number of specified preferences.
   */
  int nPreferences() const { return preferences.size(); }

  /*! \brief Returns the first preference of the ballot.
   *
   *  Returns the candidate index corresponding to the first preference of the
   * ballot.
   *
   * \return The first preference of the ballot.
   */
  int firstPreference() const { return preferences[0]; }

  /*! \brief Eliminates a specified candidate from the ballot.
   *
   *  For example, if preferences is {1, 2, 3, 4} and we call
   * `ballot.eliminate(2)`, then the resulting preferences will be {1, 3, 4}.
   * This method returns a boolean representing whether or not after elimination
   * of the candidate, the ballot remains valid. For example, if preferences is
   * {4} and we call `ballot.eliminate(4)`, then the resulting preferences is an
   * empty vector which cannot possibly represent a valid ballot.
   */
  bool eliminate(int candidate);

  /*! \brief Returns whether the provided ballot is equal to this one.
   *
   *  Checks whether another instance of IRVBallot represents the same ballot.
   *
   * \param b Another IRVBallot to compare with this one.
   *
   * \return A boolean representing whether or not the two ballots are equal.
   */
  bool operator==(const IRVBallot &b);
};

/*! \brief Evaluates the outcome of an election.
 *
 *  Given a set of ballots, this applies the social choice function to determine
 * the victorious candidate.
 *
 * \param election A set of ballots to conduct the social choice function with.
 *
 * \return The index of the victorious candidate.
 */
int socialChoiceIRV(std::list<IRVBallot> election, int nCandidates);

#endif /* IRV_BALLOT_H */

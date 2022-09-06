/******************************************************************************
 * File:             R_social_choice.h
 *
 * Author:           Floyd Everest <me@floydeverest.com>
 * Created:          07/23/22
 * Description:      This file declares the R interface to the social choice
 *                   functions.
 *****************************************************************************/

#ifndef R_SOCIAL_CHOICE_H
#define R_SOCIAL_CHOICE_H

#include <R.h>
#include <Rcpp.h>

#include "irv_ballot.h"

/*! \brief The IRV social choice function.
 *
 *  This function calculates an election outcome using the standard IRV social
 * choice function.
 *
 * \param  bs An Rcpp::List of ballots in CharacterVector representation.
 *
 * \param nWinners An integer indicating the number of winners to elect.
 *
 * \param candidates A vector of strings corresponding to candidate names.
 *
 * \param seed A seed for the PRNG for tie-breaking.
 *
 * \return The winning candidate.
 */
Rcpp::List social_choice_irv(Rcpp::List bs, unsigned nWinners,
                             Rcpp::CharacterVector candidates,
                             std::string seed);

#endif /* R_SOCIAL_CHOICE_H */

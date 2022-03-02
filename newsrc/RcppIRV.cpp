/******************************************************************************
 * File:             RcppIRV.cpp
 *
 * Author:           Floyd Everest <me@floydeverest.com>
 * Created:          03/01/22
 * Description:      This file implements an Rcpp interface for the IRV
 *                   Dirichlet Tree methods, and for the IRV social choice
 *function.
 *****************************************************************************/

// [[Rcpp::plugins("cpp17")]]
// [[Rcpp::depends(RcppThread)]]

#include "dirichlet_tree.hpp"
#include "irv_ballot.hpp"
#include "irv_node.hpp"

/*! \brief Converts an R list of valid IRV ballot vectors to a
 * std::list<IRVBallot> format.
 *
 *  In R, we consider a matrix of ballots to be that with columns corresponding
 * to each preference choice, and elements corresponding to the index of the
 * candidate.
 *
 * \param l An Rcpp::List
 *
 * \return A list of IRVBallot objects.
 */
void function_name(Type Parameter) {}

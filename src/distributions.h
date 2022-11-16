/******************************************************************************
 * File:             distributions.hpp
 *
 * Author:           Floyd Everest <me@floydeverest.com>
 * Created:          02/27/22
 * Description:      This file declares the distributions which we use in our
 *                   Dirichlet-tree implementation.
 *****************************************************************************/

#ifndef DISTRIBUTIONS_H
#define DISTRIBUTIONS_H

#include <algorithm>
#include <random>
#include <vector>

/*! \brief Draws a sample from a Dirichlet Multinomial distribution.
 *
 *  Given the count, `a` parameters and dimension of the distribution, this
 * method samples from the Dirichlet Multinomial distribution to obtain
 * appropriately distributed multinomial counts.
 *
 * \param N The total number of multinomial samples.
 *
 * \param a The `a` parameter to the Dirichlet distribution.
 *
 * \param engine A PRNG for sampling.
 *
 * \return A vector containing the sampled counts.
 */
std::vector<unsigned> rDirichletMultinomial(const unsigned &N,
                                            const std::vector<double> &a,
                                            std::mt19937 *engine);

/*! \brief Draws a sample from a Multinomial distribution.
 *
 *  Given the multinomial count, category probabilities `p`, and the number of
 * categories `d`, this method draws a single sample from the corresponding
 * Multinomial distribution.
 *
 * \param N The total number of Multinomial samples.
 *
 * \param p A vector of category probabilities.
 *
 * \param engine A PRNG for sampling.
 *
 * \return A vector containing sampled counts.
 */
std::vector<unsigned> rMultinomial(const unsigned &N,
                                   const std::vector<double> &p,
                                   std::mt19937 *engine);

/*! \brief Draws a sample from a Dirichlet distribution.
 *
 *  Given the parameter vector a, this function will draw a sample from a
 * Dirichlet(a) random variable.
 *
 * \param a The a parameter to the Dirichlet distribution
 *
 * \param d The dimension of a.
 *
 * \param *engine A PRNG for sampling.
 *
 * \return A single sample from a Dirichlet(a) random variable.
 */
std::vector<double> rDirichlet(const std::vector<double> &a,
                               std::mt19937 *engine);

#endif /* DISTRIBUTIONS_H */

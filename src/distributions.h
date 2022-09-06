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

/*! \brief Draws a sample from a Dirichlet Multinomial distribution.
 *
 *  Given the count, `a` parameters and dimension of the distribution, this
 * method samples from the Dirichlet Multinomial distribution to obtain
 * appropriately distributed multinomial counts.
 *
 * \param count The total number of multinomial samples.
 *
 * \param a An array of `a` parameters to the Dirichlet distribution.
 *
 * \param d The dimension of a, which is equal to the number of categories.
 *
 * \param engine A PRNG for sampling.
 *
 * \return An array of multinomial counts of dimension d.
 */
unsigned *rDirichletMultinomial(const unsigned count, const float *a,
                                const unsigned d, std::mt19937 *engine);

/*! \brief Draws a sample from a Multinomial distribution.
 *
 *  Given the multinomial count, category probabilities `p`, and the number of
 * categories `d`, this method draws a single sample from the corresponding
 * Multinomial distribution.
 *
 * \param count The total number of Multinomial samples.
 *
 * \param p An array of category probabilities.
 *
 * \param d The dimension of p, which is equal to the number of categories.
 *
 * \param engine A PRNG for sampling.
 *
 * \return An array of multinomial counts of dimension d.
 */
unsigned *rMultinomial(unsigned count, const float *p, const unsigned d,
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
float *rDirichlet(const float *a, const unsigned d, std::mt19937 *engine);

#endif /* DISTRIBUTIONS_H */

/******************************************************************************
 * File:             distributions.hpp
 *
 * Author:           Floyd Everest <me@floydeverest.com>
 * Created:          02/27/22
 * Description:      This file declares the distributions which we use in our
 *                   Dirichlet Tree implementation.
 *****************************************************************************/

#ifndef DISTRIBUTIONS_H
#define DISTRIBUTIONS_H

#include <algorithm>
#include <random>

/*! \brief Draws a sample from a Dirichlet Multinomial distribution.
 *
 *  Given the count, alpha parameters and dimension of the distribution, this
 * method samples from the Dirichlet Multinomial distribution to obtain
 * appropriately distributed multinomial counts.
 *
 * \param count The total number of multinomial samples.
 *
 * \param alpha An array of alpha parameters to the Dirichlet distribution.
 *
 * \param d The dimension of alpha, which is equal to the number of categories.
 *
 * \param engine A PRNG for sampling.
 *
 * \return An array of multinomial counts of dimension d.
 */
unsigned *rDirichletMultinomial(unsigned count, float *alpha, unsigned d,
                                std::mt19937 *engine);

/*! \brief Draws samples from a Beta distribution.
 *
 *  Given the parameters alpha and beta, this function will draw a sample from a
 * Beta(a, b) distributed random variable.
 *
 * \param a The a parameter to the Beta distribution
 * \param b The b parameter to the Beta distribution
 * \param *engine A PRNG for sampling.
 * \return A single sample from a Beta-distributed random variable.
 */
float rBeta(float alpha, float beta, std::mt19937 *engine);

#endif /* DISTRIBUTIONS_H */

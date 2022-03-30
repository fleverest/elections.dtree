/******************************************************************************
 * File:             dirichlet-tree.hpp
 *
 * Author:           Floyd Everest <me@floydeverest.com>
 * Created:          02/22/22
 * Description:      This file implements the Dirichlet Tree distribution class.
 *                   This class provides an interface to the interior nodes of
 *                   the tree, and provides methods to sample ballots, update
 *                   the posterior distribution, and alter internal distribution
 *                   functionalities such as the choice of prior parameter or
 *                   style of the tree (Dirichlet vs Dirichlet-Tree sampling).
 *****************************************************************************/

#ifndef DIRICHLET_TREE_H
#define DIRICHLET_TREE_H

#include "irv_ballot.hpp"
#include "tree_node.hpp"

#include <list>
#include <random>

template <typename NodeType, typename Outcome, class Parameters>
class DirichletTree {
private:
  // The interior root node for the Dirichlet Tree.
  NodeType root;

  // The tree parameters. This object defines both the structure and sampling
  // parameters for the Dirichlet Tree. Some parameters will be immutable, for
  // example the tree structure cannot be changed dynamically while the prior
  // parameter scheme at each level might be possible to alter dynamically. The
  // parameters might also indicate some outcome filtering which can often be
  // changed dynamically.
  Parameters parameters;

  // A vector of observations determining the posterior.
  std::list<Outcome> observed;

  // A default PRNG for sampling.
  std::mt19937 engine;

public:
  /*! \brief The DirichletTree constructor.
   *
   *  The constructor returns a new Dirichlet Tree according to the specified
   * characteristics. There is no copy constructor for DirichletTrees.
   *
   * \param parameters_ The Dirichlet Tree parameters object.
   *
   * \param seed A string representing the mt19937 initial seed.
   *
   * \return A DirichletTree object with the corresponding attributes.
   */
  DirichletTree(Parameters parameters_, std::string seed = "12345");

  // No copy constructor
  DirichletTree(const DirichletTree &dirichletTree) = delete;

  /*! \brief Resets the distribution to its' prior.
   *
   *  This function will clear the internal state of the distribution. All
   * observed ballots will be erased, along with all interior parameters and
   * nodes.
   *
   * \return void
   */
  void reset();

  /*! \brief Update a dirichlet tree with an observed outcome.
   *
   *  This function will update the internal parameters and nodes of the tree,
   * realising a new posterior distribution having observed the provided
   * outcome.
   *
   * \param o The outcome of the stochastic process to observe.
   *
   * \param count The number of times to observe this outcome.
   *
   * \return void
   */
  void update(Outcome o, int count);

  /*! \brief Sample from the marginal posterior distribution for a specific
   * outcome.
   *
   *  This function samples marginal probabilities for observing a specific
   * outcome under the posterior Dirichlet Tree.
   *
   * \param o The outcome for which we sample posterior observation
   * probabilities.
   *
   * \param engine Optionally, a (warmed-up) mt19937 PRNG for
   * randomness.
   *
   * \return A probability of observing the outcome, drawn from the posterior
   * Dirichlet Tree.
   */
  float marginalProbability(Outcome o, std::mt19937 *engine = nullptr);

  /*! \brief Sample outcomes from the posterior predictive distribution.
   *
   *  Samples a specified number of outcomes from one realisation of the
   * Dirichlet Tree.
   *
   * \param n The number of outcomes to sample from a single realisation of the
   * Dirichlet Tree.
   *
   * \param engine An optional warmed-up mt19937 PRNG for randomness.
   *
   * \return A list of outcomes observed from the resulting stochastic
   * process.
   */
  std::list<Outcome> sample(int n, std::mt19937 *engine = nullptr);

  /*! \brief Sample possible full sets from the posterior.
   *
   *  Assuming we have been updating the Dirichlet Tree with observations
   * without replacement, this method samples possible complete outcome sets of
   * size N from the posterior. Each complete set contains the already observed
   * outcomes which determine the posterior Dirichlet Tree used in generating
   * the output. For example, if we observe a set of outcomes {o1, o2, o2}, then
   * `posteriorSets(nSets=2, N=4)` may return [{o1, o2, o2, o3},
   * {o1, o2, o2, o1}].
   *
   * \param nSets The number of complete observation sets to sample.
   *
   * \param N The number of observations in each complete set (must be >=
   * than the number of observed outcomes).
   *
   * \return Returns `nSets` complete outcome sets sampled from the posterior
   * Dirichlet Tree distribution, using the already observed data.
   */
  std::list<std::list<Outcome>> posteriorSets(int nSets, int N,
                                              std::mt19937 *engine = nullptr);

  // Getters

  /*! \brief Get the PRNG engine.
   *
   *  Gets a pointer to the mt19937 PRNG.
   *
   * \return A pointer to the base mt19937 engine.
   */
  std::mt19937 *getEnginePtr() { return &engine; }

  /*! \brief Gets the tree parameters.
   *
   * \return Returns a pointer to the Dirichlet Tree parameters.
   */
  Parameters *getParameters() { return &parameters; }

  // Setters

  /*! \brief Sets the seed of the internal mt19937 PRNG.
   *
   *  Resets the mt19937 seed and warms up the PRNG.
   *
   * \param seed A string representing the new seed for the PRNG engine.
   *
   * \return void
   */
  void setSeed(std::string seed) {
    std::seed_seq ss(seed.begin(), seed.end());
    engine.seed(ss);
    for (auto i = 1000; i; --i) // warmup
      engine();
  }
};

#endif /* DIRICHLET_TREE_H */

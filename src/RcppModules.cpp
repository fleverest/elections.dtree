/******************************************************************************
 * File:             RcppModules.cpp
 *
 * Author:           Floyd Everest <me@floydeverest.com>
 * Created:          07/23/22
 * Description:      This file contains the Rcpp module declarations, defining
 *                   the Dirichlet-tree interface to R.
 *****************************************************************************/

#include "R_tree.h"
#include <Rcpp.h>

// The Rcpp Dirichlet-tree interface to R.
RCPP_MODULE(dirichlet_tree_module) {
  Rcpp::class_<RDirichletTree>("RDirichletTree")
      // candidates, minDepth, maxDepth, a0, vd and seed.
      .constructor<Rcpp::CharacterVector, unsigned, unsigned, float, bool,
                   std::string>()
      // Getter and Setter interfaces
      .property("n_candidates", &RDirichletTree::getNCandidates)
      .property("a0", &RDirichletTree::getA0, &RDirichletTree::setA0)
      .property("min_depth", &RDirichletTree::getMinDepth,
                &RDirichletTree::setMinDepth)
      .property("max_depth", &RDirichletTree::getMaxDepth,
                &RDirichletTree::setMaxDepth)
      .property("vd", &RDirichletTree::getVD, &RDirichletTree::setVD)
      .property("candidates", &RDirichletTree::getCandidates)
      // Other methods
      .method("reset", &RDirichletTree::reset)
      .method("update", &RDirichletTree::update)
      .method("sample_predictive", &RDirichletTree::samplePredictive)
      .method("sample_posterior", &RDirichletTree::samplePosterior);
}

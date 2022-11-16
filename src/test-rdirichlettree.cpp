/*
 * This file tests the RDirichletTree interface class.
 */

#include <testthat.h>

#include "R_tree.h"

void createAndDeleteTree(Rcpp::CharacterVector candidates, unsigned minDepth,
                         unsigned maxDepth, double a0, bool vd,
                         std::string seed) {
  RDirichletTree *tree;
  tree = new RDirichletTree(candidates, minDepth, maxDepth, a0, vd, seed);
  delete tree;
}

context("Test RDirichletTree constructor and destructor.") {
  Rcpp::CharacterVector candidates{"A", "B", "C", "D"};
  unsigned minDepth = 3;
  unsigned maxDepth = 3;
  double a0 = 1.;
  bool vd = true;
  std::string seed = "123";
  RDirichletTree *tree;

  test_that("We can create and destroy tree.") {
    CATCH_CHECK_NOTHROW(
        createAndDeleteTree(candidates, minDepth, maxDepth, a0, vd, seed));
  }
}

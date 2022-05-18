/*
 * This file tests the PIRVDirichletTree interface class.
 */

#include <testthat.h>

#include "RcppIRV.hpp"

void createAndDeleteTree(Rcpp::CharacterVector candidates, unsigned minDepth,
                         float a0, bool vd, std::string seed) {
  PIRVDirichletTree *tree;
  tree = new PIRVDirichletTree(candidates, minDepth, a0, vd, seed);
  delete tree;
}

context("Test PIRVDirichletTree constructor and destructor.") {

  Rcpp::CharacterVector candidates{"A", "B", "C", "D"};
  unsigned minDepth = 3;
  float a0 = 1.;
  bool vd = true;
  std::string seed = "123";
  PIRVDirichletTree *tree;

  test_that("We can create and destroy tree.") {
    CATCH_CHECK_NOTHROW(
        createAndDeleteTree(candidates, minDepth, a0, vd, seed));
  }
}

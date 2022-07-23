# dirtree.elections-package.R

## usethis namespace: start
#' @import Rcpp
#' @useDynLib dirtree.elections, .registration = TRUE
#' @export RDirichletTree
## usethis namespace: end

Rcpp::loadModule(module = "dirichlet_tree_module", TRUE)

.dtree_classes <- c("RDirichletTree", "Rcpp_RDirichletTree")

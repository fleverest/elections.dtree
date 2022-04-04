# dirtree.elections-package.R

## usethis namespace: start
#' @import Rcpp RcppThread
#' @useDynLib dirtree.elections, .registration = TRUE
#' @export PIRVDirichletTree
## usethis namespace: end

Rcpp::loadModule(module = "pirv_dirichlet_tree_module", TRUE)

.dtree_classes <- c("PIRVDirichletTree", "Rcpp_PIRVDirichletTree")

# dirtree.elections-package.R

## usethis namespace: start
#' @import Rcpp RcppThread
#' @useDynLib dirtree.elections, .registration = TRUE
#' @export DirichletTreeIRV
## usethis namespace: end

Rcpp::loadModule(module = "dirichlet_tree_irv_module", TRUE)

.dtree_classes <- c("Rcpp_DirichletTreeIRV")

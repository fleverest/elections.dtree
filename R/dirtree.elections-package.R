# dirtree.elections-package.R

## usethis namespace: start
#' @import Rcpp RcppThread
#' @useDynLib dirtree.elections, .registration = TRUE
#' @export RcppDirichletTreeIRV
## usethis namespace: end

Rcpp::loadModule(module = "RcppDirichletTreeIRV", TRUE)

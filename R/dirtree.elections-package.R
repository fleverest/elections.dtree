# dirtree.elections-package.R

## usethis namespace: start
#' @import Rcpp
#' @useDynLib dirtree.elections, .registration = TRUE
#' @export RcppDirichletTreeIRV
## usethis namespace: end

Rcpp::loadModule(module = "RcppDirichletTreeIRV", TRUE)

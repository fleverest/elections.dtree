# dirtree.elections-package.R

## usethis namespace: start
#' @import Rcpp
#' @useDynLib dirtree.elections, .registration = TRUE
#' @importFrom Rcpp sourceCpp
## usethis namespace: end

Rcpp::loadModule(module = "RcppDirichletTreeIRV", TRUE)

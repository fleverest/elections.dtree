# dirtree.elections-package.R

## usethis namespace: start
#' @import Rcpp
#' @importFrom Rdpack reprompt
#' @useDynLib dirtree.elections, .registration = TRUE
#' @export RDirichletTree
## usethis namespace: end

Rcpp::loadModule(module = "dirichlet_tree_module", TRUE)

.dtree_classes <- c("dirichlet_tree")
.ballot_types <- c("ranked_ballots")

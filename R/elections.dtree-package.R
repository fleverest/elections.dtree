# elections.dtree-package.R

## usethis namespace: start
#' @import Rcpp
#' @import parallel
#' @importFrom Rdpack reprompt
#' @useDynLib elections.dtree, .registration = TRUE
## usethis namespace: end

Rcpp::loadModule(module = "dirichlet_tree_module", TRUE)

.dtree_classes <- c("dirichlet_tree")
.ballot_types <- c("preferences", "aggregated_preferences", "ranked_ballots")

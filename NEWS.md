# elections.dtree 2.0.0

* Rewrote the package to use `prefio` for handling ballots.
* Added the function `social_choice` for computing election results explicitly.
* Improved documentation in README.

## 1.1.2

* Improved the `sample_posterior` multithreading so that the remainder is spread
out among the worker threads instead of the entire workload being run on the
head process.
* Fixed an issue where sampling from a tree with the `n_ballots` parameter set
to a value fewer than the number of observed ballots replacement raises an
error.

## 1.1.1

Fixed a bug in the `dirichlet_tree$sample_posterior` method where specifying
`n_threads=x` would result in `x+1` threads spawning.

## 1.1.0

Added a logical parameter `replace` to the `sample_posterior` methods. This flag
allows you to sample from the posterior distribution without reusing the ballots
in the sample.

## 1.0.3

* Fixed Multinomial overflow issue on `libc++`, improved documentation.

## 1.0.2

* Patched the Dirichlet-Multinomial sampling for systems built on `libc++`.

## 1.0.1

* Patched concurrency functionality for `sample_posterior` which prevented
compilation on `clang` systems.

## 1.0.0

* Initial release

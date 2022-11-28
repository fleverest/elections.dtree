# elections.dtree 1.1.0

This release adds a new parameter to the `sample_posterior` methods. This flag
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

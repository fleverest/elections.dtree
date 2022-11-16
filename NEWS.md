# elections.dtree

* Fixed Multinomial overflow issue on `libc++`, improved documentation.

## 1.0.2

* Patched the Dirichlet-Multinomial sampling for systems built on `libc++`.

## 1.0.1

* Patched concurrency functionality for `sample_posterior` which prevented
compilation on `clang` systems.

## 1.0.0

* Initial release

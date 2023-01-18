# `elections.dtree` v1.1.1

Fix a bug in the multithreading functionality in the
`dirichlet_tree$sample_posterior` method. When specifying `n_threads=x`, `x+1`
threads were spawned. Now we only spawn `x` including the main thread.

#### `R CMD check` results

GH Actions (via `usethis::use_github_action_check_standard()`):

* ✅ macOS (12.6.2 21G320), R==4.2.2
* ✅ Windows Server (2022 10.0.20348), R==4.2.2-win
* ✅ Ubuntu (20.04.1), R==4.1.3
* ✅ Ubuntu (20.04.1), R==4.2.2
* ✅ Ubuntu (20.04.1), R==devel

`0 errors ✔ | 0 warnings ✔ | 0 notes ✔`

Other Rhub checks:

* ✅ debian-clang-devel

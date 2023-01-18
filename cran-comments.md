# `elections.dtree` v1.1.0 (submission 2)

Add a `replace` flag to `sample_posterior` method which will not re-use the
observed ballots in the monte-carlo integration step.

#### `R CMD check` results

GH Actions (via `usethis::use_github_action_check_standard()`):

* ✅ macOS (12.6.1 21G217), R==4.2.2
* ✅ Windows Server (2022 10.0.20348), R==4.2.2-win
* ✅ Ubuntu (20.04.5), R==4.1.3
* ✅ Ubuntu (20.04.5), R==4.2.2
* ✅ Ubuntu (20.04.5), R==devel

`0 errors ✔ | 0 warnings ✔ | 0 notes ✔`

Other Rhub checks:

* ✅ debian-clang-devel

#### Other comments

Removed the `R_tree.o.tmp` build artifact from the repo.

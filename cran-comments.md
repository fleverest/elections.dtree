# `elections.dtree` v1.0.3

This patch includes a confirmed fix for the underlying Multinomial distribution
sampling under `libc++`.

#### `R CMD check` results

GH Actions (via `usethis::use_github_action_check_standard()`):

* ✅ macOS (11.6.8 20G730), R==4.2.1
* ✅ Windows Server (2022 10.0.20348), R==4.2.1-win
* ✅ Ubuntu (20.04.5), R==4.1.3
* ✅ Ubuntu (20.04.5), R==4.2.1
* ✅ Ubuntu (20.04.5), R==devel

```0 errors ✔ | 0 warnings ✔ | 0 notes ✔```

Other Rhub checks:

* ✅ fedora-clang-devel
* ✅ debian-clang-devel

# `elections.dtree` v1.0

This is the initial release

#### `R CMD check` results

GH Actions (via `usethis::use_github_action_check_standard()`):
* ✅ macOS (11.6.8 20G730), R==4.2.1
* ✅ Windows Server (2022 10.0.20348), R==4.2.1-win
* ✅ Ubuntu (20.04.5), R==4.1.3
* ✅ Ubuntu (20.04.5), R==4.2.1
* ✅ Ubuntu (20.04.5), R==devel

```
0 errors ✔ | 0 warnings ✔ | 1 note ✖

❯ checking installed package size ... NOTE
    installed size is 10.8Mb
    sub-directories of 1Mb or more:
      libs  10.6Mb
```

A large directory `libs` is common among packages which depend on `Rcpp`: (source)[https://stackoverflow.com/questions/53819970#comment94489093_53819970].

#### `devtools::check_win_devel` results

Dependency `bioconductor` isn't building for 4.3 yet.

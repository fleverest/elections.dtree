# Usage

```R
dtree <- new(RcppDirichletTreeIRV, nCandidates=5, scale=1, treeType="dirichlettree", seed="seed1234")
s10 <- dtree$sample(nBallots=10)
dtree$update(s10)
dtree$samplePosterior(100,100,s10)
```


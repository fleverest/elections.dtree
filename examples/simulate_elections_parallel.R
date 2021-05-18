#!/usr/bin/env Rscript
#
# simulate_elections_parallel.R

library("dirtree.elections")
library("BatchJobs")
library(parallel)
library(readr)

conf = BatchJobs:::getBatchJobsConf()
conf$cluster.functions = makeClusterFunctionsSlurm("../batch.tpl")

job.function <- function(i) {

    library(parallel)
    library(Rmpi)

    slave.n <- mpi.universe.size() - 1
    if (slave.n<1) {
        slave.n <- 1
    }

    parallel.function <- function(j) {
        return(c(
            paste("I am",mpi.comm.rank(),"of",mpi.comm.size()),
            paste("i",i,"j",j)
        ))
    }

    cl <- makeCluster(slaveno, type = "MPI")
    results <- clusterApply(cl, 1:4, fun=parallel.function)

    saveRDS(results, file = "result.rds")
    stopCluster(cl)
    results
}


reg <- makeRegistry("floyd-test")
ids <- batchMap(reg, fun=job.function, 1:4)
print(ids)
print(reg)

start.time <- Sys.time()
done <- submitJobs(reg, np=7)
waitForJobs(reg)

end.time <- Sys.time()
time.taken <- end.time - start.time
print(time.taken)

final_result <- readRDS("result.rds")
print(final_result)

removeRegistry(reg,"no")

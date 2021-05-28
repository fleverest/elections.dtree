args <- commandArgs(trailingOnly=TRUE)

if (length(args)<3){
    stop("Must supply number of candidates, number of Monte-Carlo elections, input file and output target as arguments")
} else {
    num_candidates <- strtoi(args[1])
    num_elections <- strtoi(args[2])
    output <- args[4]
    input <- args[3]
}

if (!file.access(output, mode=2)) {
    stop("Destination supplied must be writeable")
}

library("Rmpi")
require('dirtree.elections')
require('readr')

num.ballots <- 0
ballots.1 <- c()
ballots.2 <- c()
ballots.3 <- c()
ballots.4 <- c()
ballots.5 <- c()

ballot_proportions = c(0.9,0.5,0.1,ballot_proportions[5],0.001)

if (mpi.comm.rank(comm=0)==0) {
	ballot.counts <- read_lines(input)

	ballots <- c()

	for (bcount in ballot.counts){
		split <- strsplit(bcount, " : ")[[1]]
		ballots <- c(ballots, rep(split[1],strtoi(split[2])))
	}

	# For evaluation intervals at 1%, 25%, 50%, 75% and 99%
	num.ballots <- length(ballots)
	ballots.1 <- sample(ballots,floor(ballot_proportions[1]*num.ballots))
	temp <- ballots.1
	ballots.2 <- sample(ballots,floor(ballot_proportions[2]*num.ballots))
	ballots.3 <- sample(ballots,floor(ballot_proportions[3]*num.ballots))
	ballots.4 <- sample(ballots,floor(ballot_proportions[4]*num.ballots))
	ballots.5 <- sample(ballots,floor(ballot_proportions[5]*num.ballots)) # 0.0001 for testing.
}

mpi.barrier(comm=0)

num.ballots <- mpi.bcast(num.ballots, type=1, rank=0, comm=0)
ballots.1 <- mpi.bcast.Robj(ballots.1, rank=0, comm=0)
if(mpi.comm.rank(comm=0)==0){
	ballots.1 <- temp
}
ballots.2 <- mpi.bcast.Robj(ballots.2, rank=0, comm=0)
ballots.3 <- mpi.bcast.Robj(ballots.3, rank=0, comm=0)
ballots.4 <- mpi.bcast.Robj(ballots.4, rank=0, comm=0)
ballots.5 <- mpi.bcast.Robj(ballots.5, rank=0, comm=0)

mpi.barrier(comm=0)

# We look at scales 0.1,1,10,100
if ( mpi.comm.rank(comm=0)<5 ){
	tree.type = "irv"
	tree = dirtree.irv(candidates = num_candidates, ptype='constant', scale=0.1)
} else {
	tree = dirtree.irv(candidates = num_candidates, ptype='dirichlet', scale=0.1)
	tree.type = "dirichlet"
}

outcomes <- 1:num_candidates

# Helper function to scale up/down a tree
scale.tree <- function(tree,factor){
	tree$Do(function(node) node$alpha <- (node$alpha - node$ballots)*factor + node$ballots)
}

# helper function for monte-carlo simulation with varying scale but constant ballots
montecarlo <- function(tree, remaining) {
	df <- data.frame(
		tree_type=character(),
		ballots=numeric(),
		scale=numeric(),
		p1=numeric(),
		p2=numeric(),
		p3=numeric(),
		p4=numeric(),
		p5=numeric(),
		p6=numeric(),
		stringsAsFactors = FALSE
	)
	# scale=0.1
	res <- as.list(table(factor(
		dirtree.montecarlo(tree,remaining,num_elections),
		levels=outcomes
	))/num_elections)
	df[1,] <- append(list(tree.type, tree$ballots, 0.1), res)

	# scale=1
	scale.tree(tree,10)
	res <- as.list(table(factor(
		dirtree.montecarlo(tree,remaining,num_elections),
		levels=outcomes
	))/num_elections)
	df[2,] <- append(list(tree.type, tree$ballots, 1), res)

	# scale=10
	scale.tree(tree,10)
	res <- as.list(table(factor(
		dirtree.montecarlo(tree,remaining,num_elections),
		levels=outcomes
	))/num_elections)
	df[3,] <- append(list(tree.type, tree$ballots, 10), res)

	# scale=100
	scale.tree(tree,10)
	res <- as.list(table(factor(
		dirtree.montecarlo(tree,remaining,num_elections),
		levels=outcomes
	))/num_elections)
	df[4,] <- append(list(tree.type, tree$ballots, 100), res)

	# scale=1000
	scale.tree(tree,10)
	res <- as.list(table(factor(
		dirtree.montecarlo(tree,remaining,num_elections),
		levels=outcomes
	))/num_elections)
	df[5,] <- append(list(tree.type, tree$ballots, 100), res)

	return(df)

}

mpi.barrier(comm=0)


# 99% simulation:
if (mpi.comm.rank(comm=0)==0 || mpi.comm.rank(comm=0)==5) {
	ballots <- ballots.1
	remaining <- floor(ballot_proportions[1]*num.ballots) - length(ballots)
	dirtree.update(tree,ballots,format='duplicated')
	out_df <- montecarlo(tree, remaining)
	print(out_df)
}

# 75% simulation:
if (mpi.comm.rank(comm=0)==1 || mpi.comm.rank(comm=0)==6) {
	ballots <- ballots.2
	remaining <- floor(ballot_proportions[2]*num.ballots) - length(ballots)
	dirtree.update(tree,ballots,format='duplicated')
	out_df <- montecarlo(tree, remaining)
}

# 50% simulation:
if (mpi.comm.rank(comm=0)==2 || mpi.comm.rank(comm=0)==7) {
	ballots <- ballots.3
	remaining <- floor(ballot_proportions[3]*num.ballots) - length(ballots)
	dirtree.update(tree,ballots,format='duplicated')
	out_df <- montecarlo(tree, remaining)
}

# 25% simulation:
if (mpi.comm.rank(comm=0)==3 || mpi.comm.rank(comm=0)==8) {
	ballots <- ballots.4
	remaining <- floor(ballot_proportions[4]*num.ballots) - length(ballots)
	dirtree.update(tree,ballots,format='duplicated')
	out_df <- montecarlo(tree, remaining)
}

# 1% simulation:
if (mpi.comm.rank(comm=0)==4 || mpi.comm.rank(comm=0)==9) {
	ballots <- ballots.5
	remaining <- floor(ballot_proportions[5]*num.ballots) - length(ballots)
	dirtree.update(tree,ballots,format='duplicated')
	out_df <- montecarlo(tree, remaining)
}

mpi.barrier(comm=0)


dfs <- mpi.gather.Robj(obj=out_df, root=0, comm=0)

if( mpi.comm.rank(comm=0)==0 ){
	df = data.frame()

	for (f in dfs) {
		df <- cbind(df,f)
	}
	write.csv(df, output)
}




mpi.barrier(comm=0)
mpi.finalize()

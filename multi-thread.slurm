#!/bin/bash
# Created by the University of Melbourne job script generator for SLURM
# Thu Mar 25 2021 10:33:55 GMT+1100 (Australian Eastern Daylight Time)

# Partition for the job:
#SBATCH --partition=physical

# Multithreaded (SMP) job: must run on one node 
#SBATCH --nodes=1

# The name of the job:
#SBATCH --job-name="Single Thread"

# The project ID which this job should run under:
#SBATCH --account="COMP90024"

# Maximum number of tasks/CPU cores used by the job:
#SBATCH --ntasks=8

#SBATCH -e mpi_test_%j.err
#SBATCH -o mpi_test_%j.out

# The maximum running time of the job in days-hours:mins:sec
#SBATCH --time=0-02:00:00

# check that the script is launched with sbatch
if [ "x$SLURM_JOB_ID" == "x" ]; then
   echo "You need to submit your job to the queuing system with sbatch"
   exit 1
fi

# The modules to load:
module load gcc/8.3.0 r/4.0.0

echo 'R_LIBS_USER="/home/feverest/R_libs"' >  $HOME/.Renviron

# The job command(s):
mpirun -np 1 --mca mpi_warn_on_fork 0 R CMD BATCH --no-save --no-restore /home/feverest/simulate_elections_test.r mpi_test_${SLURM_JOB_ID}.out

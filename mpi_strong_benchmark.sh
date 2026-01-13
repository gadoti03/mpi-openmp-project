#!/bin/bash
#SBATCH --account=tra25_IngInfBo
#SBATCH --partition=g100_usr_prod
#SBATCH -t 00:20:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1 # Run a single task per node
#SBATCH -c 48 # number of CPU cores i.e. OpenMP threads per task
#SBATCH -o job.out
#SBATCH -e job.err

EXEC="./bin/binarize_mpi"
N_FIXED=10000
RESULTS_DIR="results"

mkdir -p $RESULTS_DIR

echo "Run,P,N,Time" > $RESULTS_DIR/strong_mpi_results.csv

for P in 1 2 4 8 12 16 20 24 48; do
    for i in {1..3}; do
        RESULT=$(srun -n $P $EXEC $N_FIXED)
        echo "$i,$P,$N_FIXED,$RESULT" >> $RESULTS_DIR/strong_mpi_results.csv
    done
done
#!/bin/bash
#SBATCH --account=tra24_IngInfBo
#SBATCH --partition=g100_usr_prod
#SBATCH -t 00:20:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1 # Run a single task per node
#SBATCH -c 24 # number of CPU cores i.e. OpenMP threads per task
#SBATCH -o job.out
#SBATCH -e job.err

EXEC="./bin/binarize_openMP"
RESULTS_DIR="results"

mkdir -p $RESULTS_DIR

TASKS=(1     2     4     8     12    16    24)
# SIZES=(2000 2828 4000 5656 6928 8000 9798)
SIZES=(5000 7071 10000 14142 17320 20000 24494)

echo "Run,P,N,Time" > $RESULTS_DIR/weak_openMP_results.csv

for i in "${!TASKS[@]}"; do
    P=${TASKS[$i]}
    N=${SIZES[$i]}
    for run in {1..3}; do
        RESULT=$(srun $EXEC $N $P)
        echo "$run,$P,$N,$RESULT" >> $RESULTS_DIR/weak_openMP_results.csv
    done
done
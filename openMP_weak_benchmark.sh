#!/bin/bash
#SBATCH --account=tra25_IngInfBo
#SBATCH --partition=g100_usr_prod
#SBATCH -t 00:20:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=48
#SBATCH -o job.out
#SBATCH -e job.err

EXEC="./bin/binarize_openMP"
RESULTS_DIR="results"

mkdir -p $RESULTS_DIR

TASKS=(1     2     4     8     12    16    20    24   48)
SIZES=(2000 2828 4000 5656 6924 8000 8940 9792 13872)
# SIZES=(5000 7070 10000 14144 17316 20000 22360 24480 34608)

echo "Run,P,N,Time" > $RESULTS_DIR/weak_openMP_results.csv

for i in "${!TASKS[@]}"; do
    P=${TASKS[$i]}
    N=${SIZES[$i]}
    for run in {1..3}; do
        RESULT=$(srun $EXEC $N $P)
        echo "$run,$P,$N,$RESULT" >> $RESULTS_DIR/weak_openMP_results.csv
    done
done
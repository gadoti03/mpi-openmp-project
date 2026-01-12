#!/bin/bash
#SBATCH --account=tra25_IngInfBo
#SBATCH --partition=g100_usr_prod
#SBATCH -t 00:20:00
#SBATCH --nodes=1
#SBATCH --ntasks=24
#SBATCH -o weak_scaling.out

EXEC="./bin/binarize_sr_mpi"
RESULTS_DIR="results"

mkdir -p $RESULTS_DIR

TASKS=(1     2     4     8     12    16    20    24)
# SIZES=(2000 2828 4000 5656 6928 8000 8944 9798)
SIZES=(5000 7071 10000 14142 17320 20000 22360 24494)

MODES=(0 1 2)   # exchange_mode: SSend/Recv, Isend/Irecv, Sendrecv

for mode in "${MODES[@]}"; do
    OUTFILE="$RESULTS_DIR/weak_mpi_results_mode${mode}.csv"
    
    # intestazione CSV
    echo "Run,P,N,Time" > $OUTFILE

    for i in "${!TASKS[@]}"; do
        P=${TASKS[$i]}
        N=${SIZES[$i]}
        for run in {1..3}; do
            RESULT=$(srun -n $P $EXEC $N $mode)
            echo "$run,$P,$N,$RESULT" >> $OUTFILE
        done
    done
done

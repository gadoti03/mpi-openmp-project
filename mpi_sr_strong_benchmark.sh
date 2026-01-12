#!/bin/bash
#SBATCH --account=tra25_IngInfBo
#SBATCH --partition=g100_usr_prod
#SBATCH -t 00:20:00
#SBATCH --nodes=1
#SBATCH --ntasks=24
#SBATCH -o strong_scaling.out

EXEC="./bin/binarize_sr_mpi"
N_FIXED=10000
RESULTS_DIR="results"

mkdir -p $RESULTS_DIR

MODES=(0 1 2)  # exchange_mode: SSend/Recv, Isend/Irecv, Sendrecv

for mode in "${MODES[@]}"; do
    OUTFILE="$RESULTS_DIR/strong_mpi_results_mode${mode}.csv"

    # intestazione CSV
    echo "Run,P,N,Time" > $OUTFILE

    for P in 1 2 4 8 12 16 20 24; do
        for i in {1..3}; do
            RESULT=$(srun -n $P $EXEC $N_FIXED $mode)
            # RESULT=$(srun -n $P $EXEC $N_FIXED $mode)
            echo "$i,$P,$N_FIXED,$RESULT" >> $OUTFILE
        done
    done
done

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

# ------------------------------------------------------------
# Prendi il mode come argomento
if [ -z "$1" ]; then
    echo "Uso: $0 <mode>"
    echo "mode: 0=Ssend/Recv, 1=Isend/Irecv, 2=Sendrecv"
    exit 1
fi
MODE=$1
# ------------------------------------------------------------

OUTFILE="$RESULTS_DIR/strong_mpi_results_mode${MODE}.csv"

# intestazione CSV
echo "Run,P,N,Time" > $OUTFILE

for P in 1 2 4 8 12 16 20 24; do
    for i in {1..3}; do
        RESULT=$(srun -n $P $EXEC $N_FIXED $MODE)
        echo "$i,$P,$N_FIXED,$RESULT" >> $OUTFILE
    done
done

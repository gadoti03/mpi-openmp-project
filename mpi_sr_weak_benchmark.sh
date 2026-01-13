#!/bin/bash
#SBATCH --account=tra25_IngInfBo
#SBATCH --partition=g100_usr_prod
#SBATCH -t 00:20:00
#SBATCH --nodes=1
#SBATCH -c 48
#SBATCH -o job.out
#SBATCH -e job.err

EXEC="./bin/binarize_sr_mpi"
RESULTS_DIR="results"

mkdir -p $RESULTS_DIR

TASKS=(1     2     4     8     12    16    20    24   48)
# SIZES=(2000 2828 4000 5656 6928 8000 8944 9798)
SIZES=(5000 7070 10000 14144 17316 20000 22360 24480 34608)

# ------------------------------------------------------------
# Prendi il mode come argomento
if [ -z "$1" ]; then
    echo "Uso: $0 <mode>"
    echo "mode: 0=Ssend/Recv, 1=Isend/Irecv, 2=Sendrecv"
    exit 1
fi
MODE=$1
# ------------------------------------------------------------

OUTFILE="$RESULTS_DIR/weak_mpi_results_mode${MODE}.csv"

# intestazione CSV
echo "Run,P,N,Time" > $OUTFILE

for i in "${!TASKS[@]}"; do
    P=${TASKS[$i]}
    N=${SIZES[$i]}
    for run in {1..3}; do
        RESULT=$(srun -n $P $EXEC $N $MODE)
        echo "$run,$P,$N,$RESULT" >> $OUTFILE
    done
done

sbatch mpi_strong_benchmark.sh
sbatch mpi_weak_benchmark.sh
sbatch openMP_strong_benchmark.sh
sbatch openMP_weak_benchmark.sh

python3 plot.py results/strong_mpi_results.csv results/weak_mpi_results.csv
python3 plot.py results/strong_openMP_results.csv results/weak_openMP_results.csv
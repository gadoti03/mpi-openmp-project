# COMPILA mpi
mpicc -o bin/binarize_mpi scripts/mpi.c

# COMPILA openMP
gcc -fopenmp -o bin/binarize_openMP scripts/openMP.c
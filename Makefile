# =================== CONFIG ===================
BIN_DIR     = bin
SRC_DIR     = scripts
RESULTS_DIR = results
OUTPUT_DIR  = output

MPI_SRC = $(SRC_DIR)/mpi.c
OMP_SRC = $(SRC_DIR)/openMP.c

MPI_BIN = $(BIN_DIR)/binarize_mpi
OMP_BIN = $(BIN_DIR)/binarize_openMP

MPI_STRONG_JOB = mpi_strong_benchmark.sh
MPI_WEAK_JOB   = mpi_weak_benchmark.sh
OMP_STRONG_JOB = openMP_strong_benchmark.sh
OMP_WEAK_JOB   = openMP_weak_benchmark.sh

MPI_STRONG_RES = $(RESULTS_DIR)/strong_mpi_results.csv
MPI_WEAK_RES   = $(RESULTS_DIR)/weak_mpi_results.csv
OMP_STRONG_RES = $(RESULTS_DIR)/strong_openMP_results.csv
OMP_WEAK_RES   = $(RESULTS_DIR)/weak_openMP_results.csv

# =================== DEFAULT ===================
all: build venv plot

# =================== BUILD ===================
build: $(MPI_BIN) $(OMP_BIN)

$(MPI_BIN): $(MPI_SRC)
  	mkdir -p $(BIN_DIR)
	mpicc -o $@ $<

$(OMP_BIN): $(OMP_SRC)
 	mkdir -p $(BIN_DIR)
	gcc -fopenmp -o $@ $<

# =================== RUN ===================
run: mpi openmp

mpi: build $(MPI_STRONG_RES) $(MPI_WEAK_RES)

openmp: build $(OMP_STRONG_RES) $(OMP_WEAK_RES)

$(MPI_STRONG_RES):
	sbatch $(MPI_STRONG_JOB)

$(MPI_WEAK_RES):
	sbatch $(MPI_WEAK_JOB)

$(OMP_STRONG_RES):
	sbatch $(OMP_STRONG_JOB)

$(OMP_WEAK_RES):
	sbatch $(OMP_WEAK_JOB)

# =================== PLOT ====================
plot: plot_mpi plot_openmp

plot_mpi: $(MPI_STRONG_RES) $(MPI_WEAK_RES)
	python3 plot.py $(MPI_STRONG_RES) $(MPI_WEAK_RES)

plot_openmp: $(OMP_STRONG_RES) $(OMP_WEAK_RES)
	python3 plot.py $(OMP_STRONG_RES) $(OMP_WEAK_RES)

# =================== PYTHON DEPENDENCY =======
venv:
	python3 -m venv venv
	venv/bin/pip install --upgrade pip
	venv/bin/pip install pandas matplotlib numpy

# =================== CLEAN ===================
clean:
	rm -f $(MPI_BIN) $(OMP_BIN) $(RESULTS_DIR)/*.csv

# =================== CLEANALL ===================
cleanall:
	rm -f $(MPI_BIN) $(OMP_BIN) $(RESULTS_DIR)/*.csv $(OUTPUT_DIR)/*.png

# =================== CONFIG ===================
BIN_DIR     = bin
SRC_DIR     = scripts
RESULTS_DIR = results
OUTPUT_DIR  = output

MPI_SRC = $(SRC_DIR)/mpi.c
MPI_SR_SRC = $(SRC_DIR)/mpi_sr.c
OMP_SRC = $(SRC_DIR)/openMP.c

MPI_BIN = $(BIN_DIR)/binarize_mpi
MPI_SR_BIN = $(BIN_DIR)/binarize_sr_mpi
OMP_BIN = $(BIN_DIR)/binarize_openMP

MPI_STRONG_JOB = mpi_strong_benchmark.sh
MPI_STRONG_SR_JOB = mpi_sr_strong_benchmark.sh
MPI_WEAK_JOB = mpi_weak_benchmark.sh
MPI_WEAK_SR_JOB = mpi_sr_weak_benchmark.sh
OMP_STRONG_JOB = openMP_strong_benchmark.sh
OMP_WEAK_JOB   = openMP_weak_benchmark.sh

MPI_STRONG_RES = $(RESULTS_DIR)/strong_mpi_results.csv
MPI_STRONG_RES_MODE0 = $(RESULTS_DIR)/strong_mpi_results_mode0.csv
MPI_STRONG_RES_MODE1 = $(RESULTS_DIR)/strong_mpi_results_mode1.csv
MPI_STRONG_RES_MODE2 = $(RESULTS_DIR)/strong_mpi_results_mode2.csv
MPI_WEAK_RES   = $(RESULTS_DIR)/weak_mpi_results.csv
MPI_WEAK_RES_MODE0   = $(RESULTS_DIR)/weak_mpi_results_mode0.csv
MPI_WEAK_RES_MODE1   = $(RESULTS_DIR)/weak_mpi_results_mode1.csv
MPI_WEAK_RES_MODE2   = $(RESULTS_DIR)/weak_mpi_results_mode2.csv
OMP_STRONG_RES = $(RESULTS_DIR)/strong_openMP_results.csv
OMP_WEAK_RES   = $(RESULTS_DIR)/weak_openMP_results.csv

# =================== DEFAULT ===================
all: build venv plot

# =================== BUILD ===================
build: $(MPI_BIN) $(MPI_SR_BIN) $(OMP_BIN)

$(MPI_SR_BIN): $(MPI_SR_SRC)
	mkdir -p $(BIN_DIR)
	mpicc -O3 -o $@ $<

$(MPI_BIN): $(MPI_SRC)
	mkdir -p $(BIN_DIR)
	mpicc -O3 -o $@ $<

$(OMP_BIN): $(OMP_SRC)
	mkdir -p $(BIN_DIR)
	gcc -fopenmp -O3 -o $@ $<

# =================== RUN ===================
run: mpi mpi_sr openmp

mpi: build $(MPI_STRONG_RES) $(MPI_WEAK_RES)

mpi_sr: build $(MPI_STRONG_RES_MODE0) $(MPI_WEAK_RES_MODE0) $(MPI_STRONG_RES_MODE1) $(MPI_WEAK_RES_MODE1) $(MPI_STRONG_RES_MODE2) $(MPI_WEAK_RES_MODE2)

openmp: build $(OMP_STRONG_RES) $(OMP_WEAK_RES)

$(MPI_STRONG_RES):
	sbatch --wait $(MPI_STRONG_JOB)

$(MPI_WEAK_RES):
	sbatch --wait $(MPI_WEAK_JOB)

$(MPI_STRONG_RES_MODE0):
	sbatch --wait $(MPI_STRONG_SR_JOB) 0

$(MPI_STRONG_RES_MODE1):
	sbatch --wait $(MPI_STRONG_SR_JOB) 1

$(MPI_STRONG_RES_MODE2):
	sbatch --wait $(MPI_STRONG_SR_JOB) 2

$(MPI_WEAK_RES_MODE0):
	sbatch --wait $(MPI_WEAK_SR_JOB) 0

$(MPI_WEAK_RES_MODE1):
	sbatch --wait $(MPI_WEAK_SR_JOB) 1

$(MPI_WEAK_RES_MODE2):
	sbatch --wait $(MPI_WEAK_SR_JOB) 2

$(OMP_STRONG_RES):
	sbatch --wait $(OMP_STRONG_JOB)

$(OMP_WEAK_RES):
	sbatch --wait $(OMP_WEAK_JOB)

# =================== PLOT ====================
plot: plot_mpi plot_mpi_sr_mode0 plot_mpi_sr_mode1 plot_mpi_sr_mode2 plot_openmp plot_mpi_groups

plot_mpi: $(MPI_STRONG_RES) $(MPI_WEAK_RES)
	python3 plot.py $(MPI_STRONG_RES) $(MPI_WEAK_RES)

plot_mpi_sr_mode0: $(MPI_STRONG_RES_MODE0) $(MPI_WEAK_RES_MODE0)
	python3 plot.py $(MPI_STRONG_RES_MODE0) $(MPI_WEAK_RES_MODE0)

plot_mpi_sr_mode1: $(MPI_STRONG_RES_MODE1) $(MPI_WEAK_RES_MODE1)
	python3 plot.py $(MPI_STRONG_RES_MODE1) $(MPI_WEAK_RES_MODE1)

plot_mpi_sr_mode2: $(MPI_STRONG_RES_MODE2) $(MPI_WEAK_RES_MODE2)
	python3 plot.py $(MPI_STRONG_RES_MODE2) $(MPI_WEAK_RES_MODE2)

plot_openmp: $(OMP_STRONG_RES) $(OMP_WEAK_RES)
	python3 plot.py $(OMP_STRONG_RES) $(OMP_WEAK_RES)

plot_mpi_groups: $(MPI_STRONG_RES_MODE0) $(MPI_STRONG_RES_MODE1) $(MPI_STRONG_RES_MODE2) $(MPI_STRONG_RES) $(MPI_WEAK_RES_MODE0) $(MPI_WEAK_RES_MODE1) $(MPI_WEAK_RES_MODE2) $(MPI_WEAK_RES)
	python3 plot_comparison.py $(MPI_STRONG_RES_MODE0) $(MPI_STRONG_RES_MODE1) $(MPI_STRONG_RES_MODE2) $(MPI_STRONG_RES) $(MPI_WEAK_RES_MODE0) $(MPI_WEAK_RES_MODE1) $(MPI_WEAK_RES_MODE2) $(MPI_WEAK_RES)

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

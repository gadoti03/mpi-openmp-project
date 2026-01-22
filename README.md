# HPC Matrix Binarization

This project performs **matrix binarization** on an HPC system using **MPI** and **OpenMP**. The workflow supports testing the program on multiple process counts across different nodes.

## Supported Process Counts

The program can be executed with the following number of processes:

```
1, 2, 4, 8, 12, 16, 20, 24, 48
```

## Requirements

* Python 3.x
* MPI (e.g., OpenMPI or MPICH)
* OpenMP support in your compiler
* `requirements.txt` for Python dependencies

---

## Setup

1. **Create Python virtual environment:**

```bash
make venv
```

2. **Activate the virtual environment:**

```bash
source venv/bin/activate
```

---

## Usage

### Generate CSVs (run binarization only)

```bash
make run
```

This will execute the program for all specified process counts and save the output CSVs in `results`.

### Generate CSVs + Plots

```bash
make
```

This will execute the program and also generate plots from the CSV results, saved in `results/plots/`.

### Clean CSVs and Executables

```bash
make clean
```

Removes all generated CSV files and executable binaries.

### Clean Everything (CSV, Executables, Plots)

```bash
make cleanall
```

Removes all CSVs, executables, and plots.

---

## Directory Structure

```
project_root/
├── bin
│   ├── binarize_mpi
│   └── binarize_openMP
├── config.txt
├── Makefile
├── mpi_sr_strong_benchmark.sh
├── mpi_sr_weak_benchmark.sh
├── mpi_strong_benchmark.sh
├── mpi_weak_benchmark.sh
├── openMP_strong_benchmark.sh
├── openMP_weak_benchmark.sh
├── output
├── plot_comparison.py
├── plot.py
├── presentation.pdf
├── README.md
├── requirements.txt
├── results
├── scripts
│   ├── mpi.c
│   ├── mpi_sr.c
│   ├── openMP.c
│   └── serial.c
```

---

## Notes

* The Makefile is configured to run all specified process counts sequentially.
* Plots are generated using the Python script `scripts/plot.py` and `scripts/plot_comparison.py`, which expects CSVs in `results/`.
* Ensure MPI and OpenMP are correctly installed on your HPC system.

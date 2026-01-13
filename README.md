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

This will execute the program for all specified process counts and save the output CSVs in `results/csv/`.

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

## Sending Results to a Server

A batch script `send_file.sh` can be used to send results to a remote server using `scp`.

Example usage:

```bash
./send_file.sh
```

Make sure to edit the script to set your server address and remote directory. Activate your Python virtual environment before sending the files.

---

## Directory Structure

```
project_root/
│
├─ bin/                  # Compiled binaries
├─ scripts/              # Python scripts for plotting or processing
├─ results/
│  ├─ csv/               # CSV outputs from runs
│  └─ plots/             # Generated plots
├─ venv/                 # Python virtual environment
├─ Makefile
├─ send_file.sh
└─ README.md
```

---

## Notes

* The Makefile is configured to run all specified process counts sequentially.
* Modify the `PROCS` variable in the Makefile if you want to test different process counts.
* Plots are generated using the Python script `scripts/plot_results.py`, which expects CSVs in `results/csv/`.
* Ensure MPI and OpenMP are correctly installed on your HPC system.

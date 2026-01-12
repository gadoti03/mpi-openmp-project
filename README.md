# MPI & OpenMP Benchmark Project

This project provides a framework to **compile, run, and plot benchmarks** for MPI and OpenMP implementations. The workflow is fully managed through a `Makefile`, making it easy to reproduce results and generate plots from benchmark data.

---

## Table of Contents

1. [Requirements](#requirements)
2. [Setup](#setup)
3. [Build](#build)
4. [Run Benchmarks](#run-benchmarks)
5. [Generate Plots](#generate-plots)
6. [Full Pipeline](#full-pipeline)
7. [Clean](#clean)
8. [Project Structure](#project-structure)
9. [Outputs](#outputs)

---

## Requirements

* **C compiler**: `gcc` for OpenMP
* **MPI compiler**: `mpicc` for MPI
* **SLURM** for submitting benchmark jobs (`sbatch`)
* **Python 3** with packages:

  * `numpy`
  * `pandas`
  * `matplotlib`

---

## Setup

### 1. Create a Python virtual environment (recommended)

```bash
python3 -m venv venv
```

Activate it:

* On Linux/macOS:

```bash
source venv/bin/activate
```

* On Windows (PowerShell):

```powershell
.\venv\Scripts\Activate.ps1
```

### 2. Install Python dependencies

```bash
pip install -r requirements.txt
```

This is required for generating plots.

---

## Build

Compile the MPI and OpenMP executables:

```bash
make build
```

This generates:

* `bin/binarize_mpi`
* `bin/binarize_openMP`

---

## Run Benchmarks

Submit strong and weak scaling benchmarks for both MPI and OpenMP using SLURM:

```bash
make run
```

* The Makefile submits jobs asynchronously.
* Each job script generates a `.csv` file in `results/`.

---

## Generate Plots

After benchmark results are available, generate plots:

```bash
make plots
```

This runs `plot.py` on the CSV results for both MPI and OpenMP.

---

## Full Pipeline

To execute the full workflow (build + venv + run + plot):

```bash
make
```

---

## Clean

Remove binaries and benchmark results:

```bash
make clean
```

---

## Project Structure

```
bin/                # Compiled binaries
scripts/            # Source code for MPI/OpenMP
results/            # Benchmark CSV output
output/             # Generated plots
plot.py             # Python script for generating plots
Makefile            # Workflow automation
requirements.txt    # Python dependencies
README.md           # Project documentation
```

---

## Outputs

* **Benchmark results**: `results/*.csv`
* **Generated plots**: `output/` (created by the plotting script)

---

Enjoy benchmarking and analyzing performance!

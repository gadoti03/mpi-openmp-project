# Project Instructions

This project uses a Makefile to manage compilation, benchmark execution on SLURM,
and generation of performance plots.

The entire workflow is driven through `make`.

---

## 1. Create and activate a Python virtual environment (optional but recommended)

Using a virtual environment helps avoid conflicts with system Python packages.

```bash
python3 -m venv venv
```

Activate it:

- On Linux/macOS:
```bash
source venv/bin/activate
```

- On Windows (PowerShell):
```powershell
.\venv\Scripts\Activate.ps1
```

---

## 2. Install Python dependencies

Ensure that a `requirements.txt` file is present, then run:

```bash
pip install -r requirements.txt
```

This step is required for generating plots.

---

## 3. Build the executables

Compile both MPI and OpenMP versions:

```bash
make build
```

This generates:
- `bin/binarize_mpi`
- `bin/binarize_openMP`

---

## 4. Run benchmarks on SLURM

Submit all strong and weak scaling benchmarks (MPI + OpenMP):

```bash
make run
```

This submits SLURM jobs using `sbatch`.
Each benchmark script is expected to generate a `.csv` file inside the `results/` directory.

> ⚠️ Note:
> SLURM jobs are submitted asynchronously.
> The Makefile assumes benchmark scripts correctly generate the expected output files.

---

## 5. Generate plots

After benchmark results are available, generate plots with:

```bash
make plots
```

This runs the Python plotting script on the generated CSV files.

---

## 6. Full pipeline

To execute the full workflow (build + run + plots):

```bash
make
```

---

## 7. Clean generated files

Remove binaries and benchmark results:

```bash
make clean
```

---

## 8. Output

- Benchmark results: `results/*.csv`
- Generated plots: `output/` (created by the plotting script)

Enjoy the results.

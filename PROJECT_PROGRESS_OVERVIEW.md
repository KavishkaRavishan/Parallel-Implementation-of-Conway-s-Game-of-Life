# Conway's Game of Life HPC Project - Complete Overview

## 1) Project Goal
This project aims to implement **Conway's Game of Life** using High Performance Computing approaches and compare performance across implementations.

Target implementation set (as described in project docs):
- Serial (baseline)
- OpenMP (shared memory)
- MPI (distributed memory)
- CUDA (GPU)
- Hybrid (MPI + OpenMP)

---

## 2) Current Repository Status (What is actually implemented now)
From the current workspace and commit history, the code that is present and implemented is:

### ✅ Implemented in codebase
1. **MPI implementation**
   - File: `mpi/game_of_life_mpi.c`
   - Build file: `mpi/Makefile`
2. **OpenMP implementation**
   - File: `openmp/game_of_life_openmp.c`
   - Build file: `openmp/Makefile`
3. **Top-level orchestration**
   - `Makefile` (calls subdirectory builds)
   - `benchmark.sh` (performance benchmark automation)

### ⚠️ Referenced but not present as source directories in current branch
- `serial/`
- `cuda/`
- `hybrid/`

These are mentioned in `README.md`, top-level `Makefile`, `.gitignore`, and `benchmark.sh`, but corresponding source folders/files are not currently available in this workspace snapshot.

---

## 3) Detailed Implementation Summary

## 3.1 MPI Implementation (`mpi/game_of_life_mpi.c`)
Main design and features:
- **Row-wise domain decomposition** across MPI processes.
- Handles uneven row distribution using `remainder` logic.
- Uses **ghost rows** (top and bottom) per rank for boundary communication.
- Performs halo exchange with `MPI_Sendrecv` every generation.
- Uses toroidal wrap for columns; row edges are handled via exchanged ghost rows.
- Supports optional terminal visualization (`--visual` / `-v`).
- Measures runtime using `MPI_Wtime()`.
- Computes global live-cell count with `MPI_Reduce`.
- Uses deterministic initialization (`srand(42)`) and rank-aware random-state advancement for reproducibility.

What this means for evaluation:
- Correct distributed-memory parallelization strategy.
- Communication pattern is explicit and explainable.
- Suitable for scalability testing with varying MPI ranks.

---

## 3.2 OpenMP Implementation (`openmp/game_of_life_openmp.c`)
Main design and features:
- Shared-memory parallelization using `#pragma omp parallel for collapse(2)` for grid update.
- OpenMP reduction for final live-cell counting.
- Accepts thread count from CLI and sets via `omp_set_num_threads()`.
- Includes both:
  - a serial compute path (`compute_next_gen_serial`) for baseline timing,
  - an OpenMP compute path (`compute_next_gen_omp`) for accelerated run.
- Uses deterministic initialization (`srand(42)`) for fair comparison.
- Prints serial time, OpenMP time, and speedup.
- Supports optional visualization mode.

What this means for evaluation:
- Clear comparison methodology (sequential baseline vs parallel).
- Direct thread-level speedup reporting.
- Demonstrates practical OpenMP usage patterns (parallel loops + reduction).

---

## 3.3 Build and Benchmark Workflow

### Top-level `Makefile`
- Declares build targets for: `serial`, `openmp`, `mpi`, `cuda`, `hybrid`.
- Runs `make -C <dir>` for each implementation.
- Includes global `clean` target for all modules.

### `benchmark.sh`
- Builds project with `make all`.
- Runs configured benchmark (`1000 x 1000`, `500 generations`).
- Executes multiple OpenMP and MPI configurations.
- Includes CUDA and Hybrid commands in benchmark flow.
- Extracts timing output and prints a summary table with speedups.

Important note:
- Since `serial/`, `cuda/`, and `hybrid/` folders are currently absent, those benchmark steps will fail unless those folders are added.

---

## 4) Progress Timeline (from Git history)
High-level milestones in current repository history:
- Initial repository and README setup.
- MPI implementation committed (`mpi` commit).
- Project-wide build/benchmark structure added (`Makefile`, `benchmark.sh`, `.gitignore`).
- OpenMP implementation committed (`adding the openmp file`).

This indicates your team has completed core CPU parallel implementations and shared benchmark scaffolding.

---

## 5) CUDA Status (Based on current workspace)
You mentioned CUDA was used in the project. In the **current branch snapshot**, CUDA appears in documentation and scripts, but **CUDA source files are not present**.

So for presentation/evaluation, state it this way:
- **Project design includes CUDA path** (documented and wired in scripts).
- **Current checked-in code in this branch contains MPI + OpenMP implementations**.
- CUDA implementation may exist in another branch/local folder and should be added to this repository for full reproducibility.

---

## 6) Suggested Evaluation Talking Points
Use these points during progress evaluation:
1. We implemented two full parallel strategies:
   - OpenMP for shared-memory multicore CPUs,
   - MPI for distributed-memory process-based execution.
2. We built a benchmark framework to compare multiple configurations and compute speedups.
3. We ensured deterministic initialization for fair result comparisons.
4. We included optional visualization mode for functional validation.
5. We prepared the repository structure for Serial/CUDA/Hybrid integration, with MPI/OpenMP currently available in code.

---

## 7) Current Deliverables Checklist
- [x] MPI implementation
- [x] OpenMP implementation
- [x] Per-module Makefiles
- [x] Top-level orchestration Makefile
- [x] Benchmark automation script
- [ ] Serial source directory in current branch
- [ ] CUDA source directory in current branch
- [ ] Hybrid source directory in current branch

---

## 8) Short One-Paragraph Summary (for quick reporting)
This project implements Conway's Game of Life using HPC methods, with completed and committed **MPI** and **OpenMP** versions, including runtime measurement, reproducible initialization, optional visualization, and automated benchmarking support. The repository also includes top-level build and benchmark infrastructure designed to compare Serial, OpenMP, MPI, CUDA, and Hybrid variants; however, in the current branch snapshot, source code directories for Serial/CUDA/Hybrid are not present, while MPI/OpenMP are fully available for demonstration and evaluation.

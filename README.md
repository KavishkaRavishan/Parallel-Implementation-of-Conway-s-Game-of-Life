# Conway's Game of Life: HPC Implementations

This project implements Conway's Game of Life utilizing various High Performance Computing (HPC) technologies.

## Project Structure

*   `serial/`: Contains the baseline serial (single-threaded) implementation.
*   `openmp/`: Contains the OpenMP shared-memory parallel implementation.
*   `mpi/`: Contains the MPI distributed-memory parallel implementation.
*   `cuda/`: Contains the CUDA GPU-accelerated implementation.
*   `hybrid/`: Contains a hybrid implementation (MPI + OpenMP).

## Features

- **Multiple HPC Implementations:** Serial, OpenMP, MPI, CUDA, and Hybrid (MPI+OpenMP).
- **Periodic Boundary Conditions:** The grid wraps around (toroidal topology).
- **Real-time Terminal Visualization:** Interactive terminal-based UI to watch the simulation.
- **Performance Benchmarking:** Automated script to compare execution times and speedups.

## Building the Project

A top-level `Makefile` is provided to orchestrate the build process.

- `make all`: Compiles all implementations.
- `make serial`, `make openmp`, `make mpi`, `make cuda`, `make hybrid`: Compiles specific versions.
- `make clean`: Removes all compiled binaries.

## Running the Implementations

All implementations follow a similar command-line structure:
`./executable [rows] [cols] [generations] [threads (if applicable)] [flags]`

### Terminal Visualization (UI)

To watch the simulation live in your terminal, add the `-v` or `--visual` flag. 
**Note:** For the best experience, use a smaller grid size (e.g., 30x80) that fits your terminal window.

```bash
# Example: Serial with visualization
cd serial
./gol_serial 30 80 100 -v

# Example: OpenMP with 4 threads and visualization
cd ../openmp
./gol_openmp 30 80 100 4 -v
```

### Execution Examples (High Performance)

For performance testing, use larger grid sizes without the visualization flag.

#### 1. Serial (Baseline)
```bash
cd serial
./gol_serial 1000 1000 500
```

#### 2. OpenMP (Shared Memory)
```bash
cd ../openmp
./gol_openmp 1000 1000 500 8
```

#### 3. MPI (Distributed Memory)
```bash
cd ../mpi
mpiexec -n 4 ./gol_mpi 1000 1000 500
```

#### 4. CUDA (GPU Accelerated)
```bash
cd ../cuda
./gol_cuda 1000 1000 500
```

#### 5. Hybrid (MPI + OpenMP)
```bash
cd ../hybrid
mpiexec -n 4 ./gol_hybrid 1000 1000 500 4
```

### Benchmarking
To automatically compare all implementations:
```bash
bash benchmark.sh
```
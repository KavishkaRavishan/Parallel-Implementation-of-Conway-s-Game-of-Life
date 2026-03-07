# Conway's Game of Life: HPC Implementations

This project implements Conway's Game of Life utilizing various High Performance Computing (HPC) technologies.

**Course:** EC7207
**Group:** 32

## Project Structure

*   `serial/`: Contains the baseline serial (single-threaded) implementation.
*   `openmp/`: Contains the OpenMP shared-memory parallel implementation.
*   `mpi/`: Contains the MPI distributed-memory parallel implementation.
*   `cuda/`: Contains the CUDA GPU-accelerated implementation.
*   `hybrid/`: Contains a hybrid implementation (MPI + OpenMP).

## Building the Project

A top-level `Makefile` is provided to orchestrate the build process for the different implementations. It expects each subdirectory to contain its own `Makefile`.

*   `make all`: Compiles all implementations.
*   `make serial`, `make openmp`, `make mpi`, `make cuda`, `make hybrid`: Compiles specific implementations.
*   `make clean`: Cleans build artifacts in all subdirectories.

Alternatively, you can navigate into each folder and run `make`. Note: Compiling CUDA requires `nvcc`, compiling MPI requires `mpicc`, and compiling OpenMP requires a compiler with OpenMP support (e.g., `gcc -fopenmp`).

## Running the Implementations

All executables accept command-line arguments to specify the grid dimensions and number of generations. The default grid size is 1000x1000 running for 1000 generations.

### 1. Serial
**Arguments:** `[rows] [cols] [generations]`
```bash
cd serial
./gol_serial 1000 1000 500
```

### 2. OpenMP
**Arguments:** `[rows] [cols] [generations] [threads]`
```bash
cd openmp
./gol_openmp 1000 1000 500 4
```

### 3. MPI
**Arguments:** `[rows] [cols] [generations]`
```bash
cd mpi
mpiexec -n 4 ./gol_mpi 1000 1000 500
```

### 4. CUDA
**Arguments:** `[rows] [cols] [generations]`
```bash
cd cuda
./gol_cuda 1000 1000 500
```

### 5. Hybrid (MPI + OpenMP)
**Arguments:** `[rows] [cols] [generations] [threads]`
```bash
cd hybrid
mpiexec -n 4 ./gol_hybrid 1000 1000 500 4
```

### Benchmark Script
A `benchmark.sh` script is provided in the root directory to automatically build and run all implementations with various configurations (e.g., 2/4/8 threads and processes) and print a performance comparison table.

```bash
bash benchmark.sh
```
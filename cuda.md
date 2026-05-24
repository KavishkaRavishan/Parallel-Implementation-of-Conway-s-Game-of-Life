# CUDA for Conway's Game of Life

## Table of Contents

1. [What CUDA Is](#what-cuda-is)
2. [Why We Need CUDA](#why-we-need-cuda)
3. [CUDA vs CPU, OpenMP, and MPI](#cuda-vs-cpu-openmp-and-mpi)
4. [Core CUDA Concepts](#core-cuda-concepts)
5. [Important CUDA Keywords and Functions](#important-cuda-keywords-and-functions)
6. [How Game of Life Fits CUDA](#how-game-of-life-fits-cuda)
7. [Understanding the MPI Code First](#understanding-the-mpi-code-first)
8. [How This MPI Design Would Translate to CUDA](#how-this-mpi-design-would-translate-to-cuda)
9. [Why Each Major Function Is Used in the MPI Code](#why-each-major-function-is-used-in-the-mpi-code)
10. [Important Performance Ideas](#important-performance-ideas)
11. [When CUDA Is Better and When It Is Not](#when-cuda-is-better-and-when-it-is-not)
12. [Simple Teaching Summary](#simple-teaching-summary)

## What CUDA Is

CUDA stands for Compute Unified Device Architecture. It is NVIDIA's platform for general-purpose parallel programming on NVIDIA GPUs.

Normally, we use a CPU to run programs. A CPU is very powerful for:

- running a few complex tasks very fast
- making decisions
- handling operating systems and general applications

A GPU is different. A GPU is built to run a very large number of small operations at the same time. This makes it useful for workloads where:

- the same operation must be repeated many times
- each data element can be processed mostly independently
- the problem is large enough to keep thousands of GPU threads busy

Conway's Game of Life is a very good example of that type of problem because each cell updates using the same rule.

## Why We Need CUDA

If we update a very large Game of Life grid on a CPU, one core or a few cores may become a bottleneck. Every generation requires checking neighbors for every cell.

CUDA helps because:

- one GPU can launch thousands of threads together
- one thread can handle one cell or a small group of cells
- the same update rule is applied everywhere
- the workload is regular and repetitive

So the Game of Life is a classic data-parallel problem, and CUDA is designed for data-parallel computing.

## CUDA vs CPU, OpenMP, and MPI

### CPU only

CPU-only execution is easiest to write and debug, but it may be slow for large grids.

### OpenMP

OpenMP uses shared-memory parallelism on one machine. Multiple CPU threads work together in the same memory space.

Good for:

- simple parallelization on one computer
- moderate acceleration
- easier learning compared with MPI or CUDA

### MPI

MPI uses distributed-memory parallelism. Multiple processes communicate by sending messages.

Good for:

- clusters
- multiple machines
- very large simulations that do not fit well in one process

### CUDA

CUDA uses the GPU for massively parallel execution.

Good for:

- large regular computations
- stencil-like operations
- image processing
- matrix operations
- cellular automata such as Game of Life

### Main difference in one sentence

- OpenMP: many CPU threads, shared memory
- MPI: many processes, message passing
- CUDA: many GPU threads, device memory

## Core CUDA Concepts

To teach CUDA well, these are the most important ideas.

### 1. Host and Device

In CUDA, the CPU is called the host and the GPU is called the device.

- Host code runs on the CPU
- Device code runs on the GPU

This is important because memory is also separated:

- host memory is CPU memory
- device memory is GPU memory

If data starts on the CPU and must be processed on the GPU, it usually has to be copied.

### 2. Kernel

A kernel is a function that runs on the GPU.

If you write a normal C function, the CPU executes it once when you call it.

If you write a CUDA kernel, the GPU executes many copies of that function in parallel.

For Game of Life, a common idea is:

- one kernel launch per generation
- one GPU thread per cell

### 3. Threads

A thread is the smallest execution unit.

In CUDA, many threads run the same kernel code, but each thread works on different data. For example:

- thread 0 updates cell `(0,0)`
- thread 1 updates cell `(0,1)`
- thread 2 updates cell `(0,2)`

and so on.

### 4. Blocks

CUDA groups threads into blocks.

Why blocks matter:

- threads in the same block can cooperate
- they can use shared memory
- they can synchronize with `__syncthreads()`

### 5. Grid

A grid is a collection of blocks.

So the execution hierarchy is:

- grid
- block
- thread

For a 2D Game of Life board, we often use:

- a 2D grid of blocks
- a 2D block of threads

That matches the 2D nature of the problem nicely.

### 6. Thread Indexing

Each thread must know which cell it should update.

CUDA provides built-in variables such as:

- `threadIdx.x`, `threadIdx.y`
- `blockIdx.x`, `blockIdx.y`
- `blockDim.x`, `blockDim.y`

These are used to compute the global row and column of the thread.

Typical idea:

```cpp
int col = blockIdx.x * blockDim.x + threadIdx.x;
int row = blockIdx.y * blockDim.y + threadIdx.y;
```

This is one of the most important CUDA ideas, because this mapping tells each thread which data element it owns.

### 7. Device Memory

CUDA provides GPU memory allocation using functions like:

- `cudaMalloc()`
- `cudaFree()`

This memory is on the GPU, not on the CPU.

### 8. Memory Copy

If your data is created on the CPU, then you often copy it to the GPU with:

- `cudaMemcpy()`

And after computation, you may copy results back.

This is important because memory transfers can be expensive. A GPU can be very fast, but if we copy too much data too often, performance suffers.

### 9. Shared Memory

Shared memory is a small, fast memory visible to threads in the same block.

For stencil-like problems such as Game of Life, shared memory can reduce repeated global memory reads. Neighboring cells are reused often, so loading a tile into shared memory can improve performance.

This is one of CUDA's most valuable optimization features.

### 10. Synchronization

Threads in the same block can synchronize using:

- `__syncthreads()`

This ensures all threads in that block have reached the same point before moving on.

Important note:

- `__syncthreads()` only works inside one block
- it does not synchronize all blocks in the whole grid

### 11. Warp

A warp is a small group of threads that the GPU schedules together, usually 32 threads on NVIDIA GPUs.

Why this matters:

- if threads in a warp follow different branches, performance can drop
- this is called warp divergence

In Game of Life, branching exists because alive and dead cells follow different rules. That branching is usually acceptable, but it is still useful to mention as a performance topic.

## Important CUDA Keywords and Functions

These are the names students usually see first.

### `__global__`

Used to define a kernel function that is called from the CPU and runs on the GPU.

Why important:

- it marks the main parallel function

### `__device__`

Used for a function that runs on the GPU and can be called from other GPU code.

Why important:

- helps organize device-side helper logic

### `__host__`

Used for a function that runs on the CPU.

Why important:

- makes host/device roles explicit

### `cudaMalloc()`

Allocates memory on the GPU.

Why important:

- without it, the GPU has nowhere to store the board

### `cudaMemcpy()`

Copies data between CPU and GPU.

Why important:

- the host and device usually have separate memory spaces

### `cudaFree()`

Frees GPU memory.

Why important:

- prevents memory leaks on the device

### `cudaDeviceSynchronize()`

Waits until the GPU finishes previous work.

Why important:

- useful for correctness checks and timing
- often used while learning and debugging

### Kernel launch syntax

CUDA kernel launch looks different from normal C:

```cpp
update_kernel<<<gridDim, blockDim>>>(...);
```

Why important:

- `gridDim` tells how many blocks
- `blockDim` tells how many threads per block

This launch configuration is central to CUDA programming.

## How Game of Life Fits CUDA

The Game of Life update for one cell depends on:

- the current state of that cell
- the 8 neighboring cells

This type of computation is often called a stencil computation.

Why CUDA is suitable:

- every cell performs similar work
- updates for one generation are independent if all threads read from the old board and write to a separate new board
- the computation repeats many times

This means a common CUDA solution uses:

- `current` board on the GPU
- `next` board on the GPU
- one kernel launch per generation
- pointer swap after each generation

That design is very similar in spirit to the MPI code's `current` and `next` arrays.

## Understanding the MPI Code First

The file [game_of_life_mpi.c](/home/nimesha/MP/Parallel-Implementation-of-Conway-s-Game-of-Life/mpi/game_of_life_mpi.c) is not CUDA code. It is MPI code. But it already teaches several parallel programming ideas that are also useful before learning CUDA.

### What this MPI program does

It simulates Conway's Game of Life by dividing the global grid among multiple MPI processes.

Each MPI process:

- owns a subset of rows
- stores two extra ghost rows
- exchanges boundary rows with neighbor processes
- computes the next generation for its own rows

### Parallel idea used here

This file uses domain decomposition.

That means:

- split the whole problem into pieces
- assign each piece to a worker
- workers exchange only the data needed at boundaries

In MPI:

- the workers are MPI processes

In CUDA:

- the workers are GPU threads

So the worker type changes, but the parallel thinking is related.

## How This MPI Design Would Translate to CUDA

Here is the conceptual mapping.

### MPI concept -> CUDA idea

- MPI process -> many GPU threads
- process-local chunk of rows -> thread-owned cell or thread-owned tile
- halo exchange -> neighbor loads from surrounding cells in memory
- `MPI_Gatherv()` for visualization -> optional copy from device to host for display
- `MPI_Reduce()` for total live cell count -> parallel reduction on GPU or copy back and count on CPU

### Big conceptual difference

MPI workers do not share memory directly, so they send messages.

CUDA threads on the same GPU mostly read from the same device memory space, so they usually do not need message passing for neighbor access. They just compute addresses and read nearby cells.

That is one of the biggest teaching points:

- MPI solves communication explicitly
- CUDA solves massive local parallelism inside one accelerator

## Why Each Major Function Is Used in the MPI Code

This section explains the code in [game_of_life_mpi.c](/home/nimesha/MP/Parallel-Implementation-of-Conway-s-Game-of-Life/mpi/game_of_life_mpi.c) and why each part matters.

### `initialize_grid(...)`

Purpose:

- initializes each process's part of the board

Why this function exists:

- every MPI rank owns only part of the board
- the code still wants reproducible initialization
- the fixed seed `srand(42)` ensures repeatable results

Why it advances `rand()` for earlier rows:

- rank 0 owns the first rows
- rank 1 owns later rows
- rank 2 owns even later rows
- to match a serial run exactly, each rank must generate the same values its rows would have received in the full global sequence

Why this is important:

- fair comparison between serial and parallel versions
- deterministic testing

### `print_grid(...)`

Purpose:

- prints the full board for visualization

Why only rank 0 should print:

- if every MPI process printed its own piece, terminal output would be mixed and unreadable

Why it counts live cells:

- helps the user observe simulation progress

Why escape sequences are used:

- `\033[H` moves the cursor to the top-left
- `\033[J` clears from cursor to the end of the screen

This creates an animation effect in the terminal.

### `compute_next_gen(...)`

Purpose:

- applies Conway's Game of Life rules to the local rows

Why it uses `current` and `next`:

- all updates for one generation must read the old state
- if we overwrite the current board immediately, neighbor calculations become incorrect

So we use:

- `current` for reading
- `next` for writing

This double-buffering idea is also very common in CUDA.

Why loop range is from `1` to `local_rows`:

- row `0` is the top ghost row
- row `local_rows + 1` is the bottom ghost row
- only the real owned rows should be updated

Why column index wraps with:

```c
int nj = (j + dj + cols) % cols;
```

Purpose:

- creates wrap-around behavior in the horizontal direction

Why important:

- avoids out-of-bounds access
- models a toroidal left-right boundary

Why rows do not wrap inside this function:

- row neighbors come from ghost rows
- those ghost rows are filled during MPI communication

This is a nice example of separating:

- computation
- communication

### `MPI_Init(&argc, &argv)`

Purpose:

- starts the MPI environment

Why important:

- without this call, MPI functions cannot be used safely

### `MPI_Comm_rank(MPI_COMM_WORLD, &rank)`

Purpose:

- gives the current process its ID

Why important:

- each process must know which part of the grid it owns

### `MPI_Comm_size(MPI_COMM_WORLD, &size)`

Purpose:

- tells how many processes are participating

Why important:

- needed for data partitioning and neighbor calculation

### Command-line parsing

Purpose:

- allows the user to choose rows, columns, generations, and visualization mode

Why important:

- makes the program flexible without recompilation

### Row-wise domain decomposition

The code computes:

- `remainder`
- `local_rows`
- `global_start_row`

Purpose:

- divide the global board among MPI processes

Why `remainder` is needed:

- when rows are not evenly divisible by process count, some ranks must receive one extra row

Why this matters:

- keeps the load balanced as much as possible

### `calloc((local_rows + 2) * cols, sizeof(unsigned char))`

Purpose:

- allocates the local board plus 2 ghost rows

Why `+ 2`:

- one ghost row for data from the process above
- one ghost row for data from the process below

Why `calloc` instead of `malloc`:

- `calloc` initializes memory to zero
- this gives clean starting values for ghost rows before communication

### Visualization buffers on rank 0

The code allocates:

- `full_grid`
- `recvcounts`
- `displs`

Purpose:

- gather distributed data back to rank 0 for display

Why `recvcounts` is needed:

- each rank may own a different number of rows

Why `displs` is needed:

- rank 0 must know where each received chunk belongs in the global board

### Neighbor calculation

```c
int top_neighbor = (rank - 1 + size) % size;
int bottom_neighbor = (rank + 1) % size;
```

Purpose:

- identify the neighboring ranks

Why modulo is used:

- creates circular neighbor relationships

This is called ring topology.

### `MPI_Barrier(MPI_COMM_WORLD)`

Purpose:

- synchronize all processes at the same point

Why important before timing:

- makes timing fair
- avoids one process starting much earlier than others

### `MPI_Wtime()`

Purpose:

- measure execution time

Why important:

- parallel programming is not only about correctness
- performance measurement is a key learning goal

### `MPI_Gatherv(...)`

Purpose:

- collect variable-sized local grid portions onto rank 0

Why not simple `MPI_Gather(...)`:

- not all ranks necessarily own the same number of rows

Why important:

- this is the correct MPI routine for uneven chunk sizes

### `MPI_Sendrecv(...)`

Purpose:

- exchange halo rows safely between neighboring ranks

Why this function is especially important:

- sending and receiving boundaries is required before computing the next generation
- `MPI_Sendrecv` combines send and receive in one call
- it reduces the chance of deadlock compared with separate blocking send/receive patterns

This function is one of the most important MPI concepts in this code.

The first `MPI_Sendrecv(...)`:

- sends the first real row upward
- receives the bottom ghost row from the bottom neighbor

The second `MPI_Sendrecv(...)`:

- sends the last real row downward
- receives the top ghost row from the top neighbor

Why this matters:

- after halo exchange, each process has the boundary data needed for correct neighbor counting

### Pointer swap

```c
unsigned char *temp = current;
current = next;
next = temp;
```

Purpose:

- avoid copying the entire board after every generation

Why important:

- swapping pointers is much cheaper than copying all elements
- this is a standard optimization in serial, MPI, OpenMP, and CUDA versions

### Local live-cell count

Purpose:

- each process counts the live cells in its own rows

Why important:

- counting locally is parallel
- only the final total requires communication

### `MPI_Reduce(...)`

Purpose:

- combine all local counts into one global count on rank 0

Why important:

- reduction is a core parallel programming pattern
- this is much more efficient than manually sending counts one by one

### `MPI_Finalize()`

Purpose:

- cleanly shut down the MPI environment

Why important:

- ends MPI usage correctly

## Important Performance Ideas

Even before writing CUDA code, students should learn these performance principles.

### 1. Parallelism alone is not enough

A parallel program becomes fast only if:

- enough work exists
- communication is controlled
- memory access is efficient

### 2. Memory access matters a lot

In MPI:

- halo exchange introduces communication overhead

In CUDA:

- global memory traffic can become the bottleneck

So both models teach the same lesson:

- moving data efficiently is as important as computing

### 3. Double buffering is essential

Using `current` and `next` is not just a programming style. It is necessary for correctness in Game of Life because all cells in the next generation must be based on the old generation.

### 4. Boundary handling affects design

In this MPI code:

- columns wrap directly in computation
- rows depend on ghost-row communication

In CUDA:

- both directions are usually handled through index calculations in device memory

### 5. Load balance matters

The MPI code carefully distributes extra rows when `rows % size != 0`.

In CUDA, similar thinking appears when:

- choosing block size
- covering problem sizes not divisible by block dimensions

## When CUDA Is Better and When It Is Not

### CUDA is often better when

- the grid is large
- many generations are computed
- the same rule is applied everywhere
- data transfer to the GPU is not dominating execution time

### CUDA may not be better when

- the problem is very small
- GPU transfer overhead is larger than the saved compute time
- the system has no NVIDIA GPU
- the algorithm has too much irregular branching or too much sequential dependency

This is an important teaching point: CUDA is powerful, but it is not automatically the best choice for every problem.

## Simple Teaching Summary

If you want to explain this to beginners, a very simple storyline is:

1. Conway's Game of Life updates many cells using the same rule.
2. That makes it a natural parallel problem.
3. MPI solves it by splitting rows across processes and exchanging boundary rows.
4. CUDA solves similar problems by launching many GPU threads, often one thread per cell.
5. In both approaches, we still need:
   - a current board
   - a next board
   - careful boundary handling
   - performance-aware memory use

The MPI file already teaches valuable foundations for CUDA:

- decomposition of work
- boundary dependency
- synchronization
- double buffering
- performance measurement

So even though [game_of_life_mpi.c](/home/nimesha/MP/Parallel-Implementation-of-Conway-s-Game-of-Life/mpi/game_of_life_mpi.c) is not CUDA code, it is a very good stepping stone for teaching CUDA concepts.

# Presentation-Ready Results Table

## Benchmark Setup

- Grid size: `400 x 400`
- Generations: `200`
- Execution style: commands were run individually during analysis

## Results Table

| Implementation | Configuration | Time (s) | Speedup vs Serial | Final Live Cells | Status |
|---|---|---:|---:|---:|---|
| Serial | 1 thread | 0.920111 | 1.00x | 11955 | Verified baseline |
| OpenMP | 2 threads | 0.638632 | 1.44x | 11955 | Verified |
| OpenMP | 4 threads | 0.468743 | 1.96x | 11955 | Verified |
| OpenMP | 8 threads | 0.537897 | 1.71x | 11955 | Verified |
| MPI | 2 processes | 0.369244 | 2.49x | 11955 | Verified |
| MPI | 4 processes | 0.298854 | 3.08x | 11955 | Verified |
| Hybrid | 2 MPI x 2 OMP | 0.251631 | 3.66x | 11599 | Fastest CPU result, but initialization differs |
| CUDA | GPU | 0.000000 | Not reliable | 79903 | Environment-limited, not valid for final comparison |

## Slide Summary

### Main Performance Takeaways

- Best fully consistent CPU result: `MPI (4 processes)` with `0.298854 s`
- Best measured CPU runtime overall: `Hybrid (2 MPI x 2 OMP)` with `0.251631 s`
- Best OpenMP result: `4 threads` with `0.468743 s`
- CUDA result should not be used as a final performance claim in this environment

### Correctness Takeaways

- `Serial`, `OpenMP`, and `MPI` all produced the same final live-cell count: `11955`
- `Hybrid` and `CUDA` produced different final counts because their initialization logic differs from the main baseline group

## Commands Used

```bash
./serial/gol_serial 400 400 200
./openmp/gol_openmp 400 400 200 2
./openmp/gol_openmp 400 400 200 4
./openmp/gol_openmp 400 400 200 8
mpiexec --allow-run-as-root --oversubscribe -n 2 ./mpi/gol_mpi 400 400 200
mpiexec --allow-run-as-root --oversubscribe -n 4 ./mpi/gol_mpi 400 400 200
mpiexec --allow-run-as-root --oversubscribe -n 2 ./hybrid/gol_hybrid 400 400 200 2
./cuda/gol_cuda 400 400 200
```

## Presenter Note

For the presentation, it is safest to present:

- Serial, OpenMP, and MPI as fully validated comparison results
- Hybrid as a promising best CPU result with one remaining consistency issue
- CUDA as implemented, but requiring a proper GPU-enabled environment for valid final benchmarking

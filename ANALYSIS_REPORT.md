# Conway's Game of Life Analysis Report

## 1. Introduction

This report analyzes the performance of five implementations of Conway's Game of Life:

- Serial
- OpenMP
- MPI
- CUDA
- Hybrid (MPI + OpenMP)

The analysis is based on the benchmark outputs you collected from the terminal. The goal is to study each technology separately under three common scenarios where possible:

1. Changing grid size
2. Changing number of iterations (generations)
3. Changing the level of parallelism

This report also includes a final comparison across implementations, with clear validation notes so that only fair comparisons are treated as strict conclusions.

## 2. Validation Notes

Before comparing runtimes, correctness consistency must be checked.

### 2.1 Serial, OpenMP, and MPI

These three implementations use the same initialization rule:

```c
(rand() % 10 < 3) ? 1 : 0
```

This is confirmed in:

- [serial/game_of_life_serial.c](/home/nimesha/MP/Parallel-Implementation-of-Conway-s-Game-of-Life/serial/game_of_life_serial.c:28)
- [openmp/game_of_life_openmp.c](/home/nimesha/MP/Parallel-Implementation-of-Conway-s-Game-of-Life/openmp/game_of_life_openmp.c:26)
- [mpi/game_of_life_mpi.c](/home/nimesha/MP/Parallel-Implementation-of-Conway-s-Game-of-Life/mpi/game_of_life_mpi.c:31)

Their final live-cell counts match for corresponding test cases, so they form one validated comparison group.

### 2.2 CUDA and Hybrid

CUDA and Hybrid use a different initialization rule:

```c
rand() % 2
```

This is confirmed in:

- [cuda/game_of_life_cuda.cu](/home/nimesha/MP/Parallel-Implementation-of-Conway-s-Game-of-Life/cuda/game_of_life_cuda.cu:22)
- [hybrid/game_of_life_hybrid.c](/home/nimesha/MP/Parallel-Implementation-of-Conway-s-Game-of-Life/hybrid/game_of_life_hybrid.c:31)

Their final live-cell counts match each other for the same problem settings, so CUDA and Hybrid form a second internally consistent comparison group.

### 2.3 Important Comparison Rule

Because the initial population is different:

- Serial vs OpenMP vs MPI can be compared directly
- CUDA vs Hybrid can be compared directly
- CUDA/Hybrid should not be used for strict speedup claims against Serial/OpenMP/MPI unless all implementations are rerun with the same initialization rule

## 3. Serial Baseline

There is no separate serial benchmark block in your provided data, but the OpenMP program prints serial execution time before the OpenMP run. Therefore, the serial timings embedded inside the OpenMP outputs are used as the baseline.

This is valid because the OpenMP program runs a serial version first, then the threaded version, on the same input size and generation count.

## 4. OpenMP Analysis

OpenMP was tested under three scenarios:

1. Grid size variation with constant threads and constant generations
2. Generation variation with constant grid size and constant threads
3. Thread variation with constant grid size and constant generations

### 4.1 Scenario 1: Changing Grid Size

Constants:

- Threads = `2`
- Generations = `200`

| Grid Size | Serial Time (s) | OpenMP Time (s) | Speedup | Final Live Cell Count |
|---|---:|---:|---:|---:|
| 200 x 200 | 1.470169 | 0.783940 | 1.88x | 2743 |
| 400 x 400 | 1.931382 | 0.972736 | 1.99x | 11955 |
| 800 x 800 | 28.330368 | 8.096674 | 3.50x | 46665 |
| 1000 x 1000 | 32.605148 | 8.328918 | 3.91x | 73698 |

Discussion:

- OpenMP benefits more as the grid becomes larger.
- For small grids, threading overhead limits the gain.
- For larger grids, the workload is large enough to keep threads busy, so speedup improves.
- The jump from `400 x 400` to `800 x 800` shows a strong increase in parallel benefit.

### 4.2 Scenario 2: Changing Generations

Constants:

- Grid Size = `1000 x 1000`
- Threads = `2`

| Generations | Serial Time (s) | OpenMP Time (s) | Speedup | Final Live Cell Count |
|---|---:|---:|---:|---:|
| 100 | 13.342451 | 5.834036 | 2.29x | 95907 |
| 200 | 23.738229 | 13.385529 | 1.77x | 73698 |
| 400 | 35.883830 | 55.764819 | 0.64x | 57357 |
| 800 | 91.449456 | 68.785429 | 1.33x | 45637 |

Discussion:

- The `100` and `200` generation results show clear OpenMP benefit.
- The `400` generation result is an outlier and should be treated carefully.
- A separate OpenMP thread-scaling run for the same configuration `1000 x 1000 x 400, 2 threads` produced `11.112589 s`, not `55.764819 s`.
- Because of that conflict, the `400` generation OpenMP point is not reliable enough for a strong trend conclusion and should ideally be rerun before plotting a final graph.
- Excluding the outlier, runtime increases with generations as expected.

### 4.3 Scenario 3: Changing Thread Count

Constants:

- Grid Size = `1000 x 1000`
- Generations = `400`

| Threads | Serial Time (s) | OpenMP Time (s) | Speedup | Final Live Cell Count |
|---|---:|---:|---:|---:|
| 1 | 23.851962 | 22.976353 | 1.04x | 57357 |
| 2 | 21.303171 | 11.112589 | 1.92x | 57357 |
| 4 | 30.796969 | 9.593352 | 3.21x | 57357 |
| 8 | 21.254482 | 8.405809 | 2.53x | 57357 |

Discussion:

- Increasing threads from `1` to `4` improves performance significantly.
- `8` threads gives the lowest execution time, but its reported speedup is lower than the `4`-thread case because the serial baseline printed in that run was also lower.
- The runtime trend itself is still useful: `22.98 s -> 11.11 s -> 9.59 s -> 8.41 s`.
- OpenMP scales well up to moderate thread counts, but the improvement is not perfectly linear.
- This behavior is expected because synchronization and memory bandwidth overheads increase with more threads.

### 4.4 OpenMP Summary

OpenMP performs well for large grids and moderate thread counts. Its strongest behavior appears when the problem size is large enough to offset thread-management overhead. The main caution is that one iteration-scaling result at `400` generations is inconsistent with the separate thread-scaling experiment and should be treated as an outlier.

## 5. MPI Analysis

MPI was tested under three scenarios:

1. Grid size variation with constant processes and constant generations
2. Generation variation with constant grid size and constant processes
3. Process variation with constant grid size and constant generations

### 5.1 Scenario 1: Changing Grid Size

Constants:

- Processes = `2`
- Generations = `200`

| Grid Size | MPI Time (s) | Final Live Cell Count |
|---|---:|---:|
| 200 x 200 | 0.840566 | 2743 |
| 400 x 400 | 2.368836 | 11955 |
| 800 x 800 | 12.804574 | 46665 |
| 1000 x 1000 | 17.304117 | 73698 |

Discussion:

- MPI runtime increases with grid size as expected.
- The final live-cell counts match the Serial/OpenMP group exactly.
- For small grids, communication overhead is a larger fraction of total runtime.
- For larger grids, computation dominates more strongly, making MPI more worthwhile.

### 5.2 Scenario 2: Changing Generations

Constants:

- Grid Size = `1000 x 1000`
- Processes = `2`

| Generations | MPI Time (s) | Final Live Cell Count |
|---|---:|---:|
| 100 | 4.882268 | 95907 |
| 200 | 6.494852 | 73698 |
| 400 | 10.219770 | 57357 |
| 800 | 29.604758 | 45637 |

Discussion:

- MPI runtime rises with the number of generations.
- The growth is broadly expected because each generation requires both local computation and halo exchange.
- Even though communication repeats every generation, the results show that MPI remains efficient for this problem size.

### 5.3 Scenario 3: Changing Process Count

Constants:

- Grid Size = `1000 x 1000`
- Generations = `400`

| Processes | MPI Time (s) | Speedup vs 1 Process | Parallel Efficiency | Final Live Cell Count |
|---|---:|---:|---:|---:|
| 1 | 38.205562 | 1.00x | 1.00 | 57357 |
| 2 | 22.801708 | 1.68x | 0.84 | 57357 |
| 4 | 12.386404 | 3.08x | 0.77 | 57357 |
| 8 | 8.793422 | 4.34x | 0.54 | 57357 |

Discussion:

- MPI clearly benefits from increasing process count.
- The best measured runtime is at `8` processes.
- Efficiency decreases as process count increases, which is normal because communication overhead grows.
- The `1`-process MPI runtime is slower than the serial baseline because MPI still pays process-management and communication-framework overhead even without real parallel distribution.

### 5.4 MPI Summary

MPI shows stable and believable scaling across all three scenarios. Compared with OpenMP, MPI is more expensive at small sizes but becomes very strong for larger workloads and higher process counts. Among the validated CPU implementations, MPI provides the clearest scaling trend.

## 6. CUDA Analysis

CUDA was tested mainly with:

1. Grid size variation
2. Generation variation at `1000 x 1000`
3. Generation variation at `10000 x 10000`

### 6.1 Scenario 1: Changing Grid Size

Constants:

- Generations = `400`

| Grid Size | CUDA Time (s) | Final Live Cell Count |
|---|---:|---:|
| 100 x 100 | 0.016933 | 521 |
| 200 x 200 | 0.049681 | 1849 |
| 400 x 400 | 0.057920 | 9256 |
| 800 x 800 | 0.161731 | 37484 |
| 1000 x 1000 | 0.237015 | 59017 |
| 10000 x 10000 | 18.638279 | 5852476 |

Discussion:

- CUDA is extremely fast at all tested problem sizes.
- The execution time remains very low even as the grid increases to `1000 x 1000`.
- Only when the problem reaches `10000 x 10000` does runtime become large enough to compare more comfortably on a seconds scale.
- This is the strongest evidence that the Game of Life update step maps very well to GPU parallelism.

### 6.2 Scenario 2: Changing Generations at 1000 x 1000

Constants:

- Grid Size = `1000 x 1000`

| Generations | CUDA Time (s) | Final Live Cell Count |
|---|---:|---:|
| 100 | 0.092205 | 94170 |
| 200 | 0.112058 | 73223 |
| 400 | 0.180239 | 59017 |
| 800 | 0.320820 | 48447 |
| 1000 | 0.436539 | 44507 |
| 8000 | 2.818582 | 28328 |

Discussion:

- Runtime increases smoothly with the number of generations.
- The scaling trend is much more regular than the inconsistent OpenMP outlier.
- Even at `8000` generations, CUDA remains very fast for a `1000 x 1000` grid.

### 6.3 Scenario 3: Changing Generations at 10000 x 10000

Constants:

- Grid Size = `10000 x 10000`

| Generations | CUDA Time (s) | Final Live Cell Count |
|---|---:|---:|
| 100 | 3.522898 | 9452874 |
| 200 | 7.349660 | 7400384 |
| 300 | 11.428271 | 6460541 |
| 400 | 18.638279 | 5852476 |

Discussion:

- At a much larger grid size, CUDA runtime scales more visibly with generations.
- This larger test makes GPU performance easier to present in graphs because the times are no longer near zero.
- The results confirm that CUDA remains practical even for extremely large problem sizes.

### 6.4 CUDA Summary

CUDA is the fastest implementation by reported runtime. Its scaling behavior is smooth and its results are internally consistent with Hybrid. However, strict direct comparison against Serial/OpenMP/MPI is not fully fair yet because CUDA uses a different initial live-cell distribution.

## 7. Hybrid Analysis

Hybrid was tested under three useful scenarios:

1. Grid size variation with fixed MPI processes and OpenMP threads
2. Process-thread configuration variation at fixed problem size
3. Generation variation at fixed problem size and fixed process-thread setting

### 7.1 Scenario 1: Changing Grid Size

Constants:

- MPI Processes = `4`
- OpenMP Threads per Process = `4`
- Generations = `400`

| Grid Size | Hybrid Time (s) | Final Live Cell Count |
|---|---:|---:|
| 100 x 100 | 9.889167 | 521 |
| 200 x 200 | 13.255563 | 1849 |
| 400 x 400 | 14.960785 | 9256 |
| 800 x 800 | 25.763168 | 37484 |
| 1000 x 1000 | 34.053781 | 59017 |

Discussion:

- The final live-cell counts match the CUDA group exactly.
- Runtime increases with grid size, but the absolute time is much higher than CUDA.
- For small grids especially, the hybrid method suffers from high process and thread overhead relative to the work done.

### 7.2 Scenario 2: Changing MPI/OpenMP Configuration

Constants:

- Grid Size = `1000 x 1000`
- Generations = `400`

| MPI Processes | Threads per Process | Hybrid Time (s) | Final Live Cell Count |
|---|---:|---:|---:|
| 4 | 4 | 17.945950 | 59017 |
| 2 | 2 | 11.358330 | 59017 |
| 2 | 4 | 11.119223 | 59017 |
| 4 | 2 | 10.188373 | 59017 |

Discussion:

- The best measured hybrid runtime in this configuration sweep is `4 MPI x 2 OpenMP = 10.188373 s`.
- `2 x 4` and `2 x 2` are very close to each other.
- `4 x 4` is noticeably slower than the other tested combinations.
- This suggests that too many total CPU workers can introduce overhead rather than speedup in this environment.

### 7.3 Scenario 3: Changing Generations

Constants:

- Grid Size = `1000 x 1000`
- MPI Processes = `4`
- OpenMP Threads per Process = `4`

| Generations | Hybrid Time (s) | Final Live Cell Count |
|---|---:|---:|
| 100 | 5.930832 | 94170 |
| 200 | 10.045002 | 73223 |
| 300 | 26.531438 | 65302 |
| 400 | 16.091894 | 59017 |
| 800 | 34.282858 | 48447 |

Discussion:

- The final live-cell counts are consistent with the CUDA group where matching generation counts were provided.
- The runtimes do not follow a perfectly smooth trend.
- The `300` generation result is unexpectedly higher than the `400` generation result, which indicates measurement noise or system variability.
- Because the same `1000 x 1000 x 400, 4 x 4` configuration was recorded multiple times as `34.053781 s`, `17.945950 s`, and `16.091894 s`, hybrid timing clearly has significant run-to-run variation in this environment.

### 7.4 Hybrid Summary

Hybrid works correctly relative to CUDA, but its timing is less stable than MPI and CUDA. In your measurements, the best hybrid configuration was not the largest one. Instead, a moderate balance of processes and threads performed better, which is a common result when oversubscription or CPU contention becomes significant.

## 8. Cross-Implementation Comparison

### 8.1 Validation Summary

| Comparison Group | Implementations | Initial Population Rule | Final Count Agreement | Strict Runtime Comparison Valid? |
|---|---|---|---|---|
| CPU Validated Group | Serial, OpenMP, MPI | `rand() % 10 < 3` | Yes | Yes |
| GPU/Hybrid Validated Group | CUDA, Hybrid | `rand() % 2` | Yes | Yes |
| Full Five-Way Group | Serial, OpenMP, MPI, CUDA, Hybrid | Mixed | No | Not fully |

### 8.2 Validated CPU Comparison at 1000 x 1000, 400 Generations

This is the cleanest direct comparison because Serial, OpenMP, and MPI agree on the final live-cell count `57357`.

| Implementation | Configuration | Time (s) | Final Live Cell Count |
|---|---|---:|---:|
| Serial | Baseline from OpenMP serial run | 23.851962 | 57357 |
| OpenMP | 1 thread | 22.976353 | 57357 |
| OpenMP | 2 threads | 11.112589 | 57357 |
| OpenMP | 4 threads | 9.593352 | 57357 |
| OpenMP | 8 threads | 8.405809 | 57357 |
| MPI | 1 process | 38.205562 | 57357 |
| MPI | 2 processes | 22.801708 | 57357 |
| MPI | 4 processes | 12.386404 | 57357 |
| MPI | 8 processes | 8.793422 | 57357 |

Discussion:

- Serial is the baseline.
- OpenMP with `8` threads gave the lowest validated CPU runtime in your data: `8.405809 s`.
- MPI with `8` processes was very close at `8.793422 s`.
- MPI at `1` process is slower than serial because MPI overhead dominates when there is no parallel scaling benefit.
- Both OpenMP and MPI demonstrate clear CPU-side acceleration.

### 8.3 CUDA and Hybrid Comparison at 1000 x 1000, 400 Generations

This comparison is valid internally because both implementations produce the same final live-cell count `59017`.

| Implementation | Configuration | Time (s) | Final Live Cell Count |
|---|---|---:|---:|
| CUDA | GPU | 0.180239 | 59017 |
| Hybrid | 2 MPI x 2 OMP | 11.358330 | 59017 |
| Hybrid | 2 MPI x 4 OMP | 11.119223 | 59017 |
| Hybrid | 4 MPI x 2 OMP | 10.188373 | 59017 |
| Hybrid | 4 MPI x 4 OMP | 17.945950 | 59017 |

Discussion:

- CUDA is dramatically faster than Hybrid in the reported data.
- Hybrid still benefits from mixed parallelism, but it does not approach GPU performance.
- Among the hybrid configurations, `4 x 2` was the best recorded balance.

### 8.4 Overall Performance Interpretation

Based on the measured runtimes:

- Best validated CPU result: OpenMP with `8` threads at `8.405809 s`
- Best validated MPI result: `8` processes at `8.793422 s`
- Best hybrid result: `4 MPI x 2 OpenMP` at `10.188373 s`
- Best reported CUDA result at `1000 x 1000, 400`: `0.180239 s`

Interpretation:

- For CPU-only execution, OpenMP and MPI are both effective.
- OpenMP is slightly better than MPI in your `1000 x 1000 x 400` validated comparison.
- MPI shows cleaner scaling behavior across process counts.
- Hybrid did not outperform the best pure OpenMP or MPI results in your environment.
- CUDA is the clear raw-performance winner, especially for large grids, but it should be presented as a separately validated group unless initialization is unified.

## 9. Final Discussion

This project demonstrates that Conway's Game of Life is highly suitable for parallel execution, but the best technology depends on the hardware model and the fairness criteria used for comparison.

For shared-memory CPU execution, OpenMP gives strong performance improvements and is relatively simple to implement. Its behavior improves as the grid becomes larger, although scaling is not perfectly linear at higher thread counts.

MPI provides the most stable CPU-side scaling trend. It is especially effective when the workload is large enough to justify communication overhead. The process-scaling results are believable and consistent, which makes MPI one of the strongest parts of the study.

CUDA is by far the fastest implementation in raw runtime. The GPU handles this workload extremely efficiently, and the large-grid experiments make that advantage very clear. Hybrid is correct relative to CUDA, but in your environment it did not outperform the best pure CPU implementations consistently, and its timings varied significantly across repeated runs.

The biggest methodological issue in the current benchmark set is that Serial/OpenMP/MPI use one initialization rule while CUDA/Hybrid use another. This does not invalidate the per-technology analysis, but it means the final five-way comparison must be presented carefully.

## 10. Recommendations for the Final Version

To make the report even stronger:

1. Re-run OpenMP for `1000 x 1000, 400 generations, 2 threads` because one measurement is clearly inconsistent.
2. Re-run the hybrid `4 MPI x 4 OpenMP` case several times and report the average runtime.
3. Unify the initialization rule across all five implementations.
4. After unifying initialization, produce one final strict comparison table for all technologies under the same settings.
5. Use the tables in this report directly for plotting graphs, but mark the OpenMP `400`-generation point and the repeated Hybrid `4 x 4` timings as unstable measurements.

## 11. Final Conclusion

The benchmark results support the following main conclusions:

- OpenMP is an effective shared-memory solution and performs best on the validated CPU comparison.
- MPI scales well and remains one of the most reliable implementations in the study.
- CUDA delivers the best raw performance by a very large margin.
- Hybrid is functional and correct relative to CUDA, but its measured performance is more variable and not always better than pure OpenMP or MPI.
- A fully fair five-way comparison still requires unified initialization and a few repeated measurements for the unstable cases.

Even with those cautions, the data already gives a strong and useful report structure for graphs, discussion, and presentation.

#!/bin/bash

# Configuration
ROWS=1000
COLS=1000
GENS=500

echo "Building all implementations..."
make all

echo "==========================================================="
echo "Running Benchmarks (${ROWS}x${COLS}, $GENS generations)"
echo "==========================================================="

# Function to extract time from output
extract_time() {
    local output="$1"
    local prefix="$2"
    echo "$output" | grep -E "$prefix" | awk '{print $(NF-1)}'
}

# Run Serial
echo "Running Serial..."
OUT_SERIAL=$(./serial/gol_serial $ROWS $COLS $GENS)
TIME_SERIAL=$(extract_time "$OUT_SERIAL" "Total Execution Time:")

# Run OpenMP
echo "Running OpenMP (2 threads)..."
OUT_OMP2=$(./openmp/gol_openmp $ROWS $COLS $GENS 2)
TIME_OMP2=$(extract_time "$OUT_OMP2" "OpenMP Execution Time:")

echo "Running OpenMP (4 threads)..."
OUT_OMP4=$(./openmp/gol_openmp $ROWS $COLS $GENS 4)
TIME_OMP4=$(extract_time "$OUT_OMP4" "OpenMP Execution Time:")

echo "Running OpenMP (8 threads)..."
OUT_OMP8=$(./openmp/gol_openmp $ROWS $COLS $GENS 8)
TIME_OMP8=$(extract_time "$OUT_OMP8" "OpenMP Execution Time:")

# Run MPI
echo "Running MPI (2 processes)..."
OUT_MPI2=$(mpiexec --allow-run-as-root --oversubscribe -n 2 ./mpi/gol_mpi $ROWS $COLS $GENS)
TIME_MPI2=$(extract_time "$OUT_MPI2" "Total Execution Time:")

echo "Running MPI (4 processes)..."
OUT_MPI4=$(mpiexec --allow-run-as-root --oversubscribe -n 4 ./mpi/gol_mpi $ROWS $COLS $GENS)
TIME_MPI4=$(extract_time "$OUT_MPI4" "Total Execution Time:")

echo "Running MPI (8 processes)..."
OUT_MPI8=$(mpiexec --allow-run-as-root --oversubscribe -n 8 ./mpi/gol_mpi $ROWS $COLS $GENS)
TIME_MPI8=$(extract_time "$OUT_MPI8" "Total Execution Time:")

# Run CUDA
echo "Running CUDA..."
OUT_CUDA=$(./cuda/gol_cuda $ROWS $COLS $GENS)
TIME_CUDA=$(extract_time "$OUT_CUDA" "Total Execution Time \(GPU\):")

# Run Hybrid
echo "Running Hybrid (4 MPI procs, 4 OMP threads)..."
OUT_HYBRID=$(mpiexec --allow-run-as-root --oversubscribe -n 4 ./hybrid/gol_hybrid $ROWS $COLS $GENS 4)
TIME_HYBRID=$(extract_time "$OUT_HYBRID" "Total Execution Time:")

# Print formatted results table
echo ""
echo "=========================================================================="
printf "%-20s | %-16s | %-12s | %-10s
" "Implementation" "Configuration" "Time (s)" "Speedup"
echo "--------------------------------------------------------------------------"

print_row() {
    local impl="$1"
    local config="$2"
    local time_val="$3"
    
    if [ -z "$time_val" ] || [ "$time_val" == "0" ] || [ -z "$TIME_SERIAL" ]; then
        printf "%-20s | %-16s | %-12s | %-10s
" "$impl" "$config" "N/A" "N/A"
    else
        local speedup=$(awk -v t1="$TIME_SERIAL" -v t2="$time_val" 'BEGIN { printf "%.2f", t1/t2 }')
        printf "%-20s | %-16s | %-12.6f | %-10.2fx
" "$impl" "$config" "$time_val" "$speedup"
    fi
}

if [ -n "$TIME_SERIAL" ]; then
    printf "%-20s | %-16s | %-12.6f | %-10.2fx
" "Serial" "1 Thread" "$TIME_SERIAL" "1.00"
else
    printf "%-20s | %-16s | %-12s | %-10s
" "Serial" "1 Thread" "N/A" "N/A"
fi

print_row "OpenMP" "2 Threads" "$TIME_OMP2"
print_row "OpenMP" "4 Threads" "$TIME_OMP4"
print_row "OpenMP" "8 Threads" "$TIME_OMP8"
print_row "MPI" "2 Processes" "$TIME_MPI2"
print_row "MPI" "4 Processes" "$TIME_MPI4"
print_row "MPI" "8 Processes" "$TIME_MPI8"
print_row "CUDA" "GPU Blocks" "$TIME_CUDA"
print_row "Hybrid" "4 Procs x 4 Thds" "$TIME_HYBRID"
echo "=========================================================================="

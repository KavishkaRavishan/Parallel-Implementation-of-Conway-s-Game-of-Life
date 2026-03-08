/*
 * Course: EC7207
 * Group: 32
 * Description: Conway's Game of Life - MPI Implementation
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <mpi.h>
#include <unistd.h>

#define DEFAULT_ROWS 1000
#define DEFAULT_COLS 1000
#define DEFAULT_GENS 1000

// Function to initialize the grid reproducibly across MPI ranks (matches serial exactly)
void initialize_grid(unsigned char *grid, int global_start_row, int local_rows, int cols, int total_rows) {
    srand(42); // Fixed seed for reproducibility
    
    // Advance rand() state for previous rows so each rank generates the correct sequence
    for (int r = 0; r < global_start_row; r++) {
        for (int c = 0; c < cols; c++) {
            rand();
        }
    }
    
    // Initialize local rows (index 1 to local_rows)
    for (int i = 1; i <= local_rows; i++) {
        for (int j = 0; j < cols; j++) {
            grid[i * cols + j] = (rand() % 10 < 3) ? 1 : 0;
        }
    }
}

// Function to print the grid to the terminal (only called by rank 0)
void print_grid(unsigned char *grid, int rows, int cols, int gen) {
    // Move cursor to home (top-left)
    printf("\033[H");
    
    int live_count = 0;
    for (int i = 0; i < rows * cols; i++) {
        if (grid[i] == 1) live_count++;
    }

    printf("Generation: %d | Live Cells: %d (MPI)          \n", gen, live_count);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fputs(grid[i * cols + j] ? "█" : " ", stdout);
        }
        putchar('\n');
    }
    // Clear from cursor to end of screen
    printf("\033[J");
    fflush(stdout);
}

// Function to compute the next generation
void compute_next_gen(unsigned char *current, unsigned char *next, int local_rows, int cols) {
    for (int i = 1; i <= local_rows; i++) {
        for (int j = 0; j < cols; j++) {
            int count = 0;
            
            for (int di = -1; di <= 1; di++) {
                for (int dj = -1; dj <= 1; dj++) {
                    if (di == 0 && dj == 0) continue;
                    
                    // No wrap for rows; relying on ghost rows
                    int ni = i + di;
                    
                    // Wrap for columns
                    int nj = (j + dj + cols) % cols; 
                    
                    count += current[ni * cols + nj];
                }
            }
            
            int idx = i * cols + j;
            if (current[idx] == 1) {
                next[idx] = (count == 2 || count == 3) ? 1 : 0;
            } else {
                next[idx] = (count == 3) ? 1 : 0;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int rows = DEFAULT_ROWS;
    int cols = DEFAULT_COLS;
    int gens = DEFAULT_GENS;
    int visual = 0;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--visual") == 0 || strcmp(argv[i], "-v") == 0) {
            visual = 1;
        } else if (i == 1 && argv[i][0] != '-') {
            rows = atoi(argv[i]);
        } else if (i == 2 && argv[i][0] != '-') {
            cols = atoi(argv[i]);
        } else if (i == 3 && argv[i][0] != '-') {
            gens = atoi(argv[i]);
        }
    }

    if (rows < size) {
        if (rank == 0) {
            fprintf(stderr, "Error: Number of rows (%d) must be >= number of processes (%d)\n", rows, size);
        }
        MPI_Finalize();
        return 1;
    }

    if (rank == 0 && !visual) {
        printf("Grid Size: %dx%d\n", rows, cols);
        printf("Generations: %d\n", gens);
        printf("MPI Processes: %d\n", size);
    }

    // Domain decomposition (row-wise block distribution)
    int remainder = rows % size;
    int local_rows = rows / size + ((rank < remainder) ? 1 : 0);
    int global_start_row = 0;
    
    if (rank < remainder) {
        global_start_row = rank * (rows / size + 1);
    } else {
        global_start_row = remainder * (rows / size + 1) + (rank - remainder) * (rows / size);
    }

    // Allocate memory including 2 ghost rows (top and bottom)
    unsigned char *current = (unsigned char *)calloc((local_rows + 2) * cols, sizeof(unsigned char));
    unsigned char *next = (unsigned char *)calloc((local_rows + 2) * cols, sizeof(unsigned char));

    if (current == NULL || next == NULL) {
        fprintf(stderr, "Error allocating memory on rank %d\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    initialize_grid(current, global_start_row, local_rows, cols, rows);

    // Memory for gathering the full grid on rank 0 for visualization
    unsigned char *full_grid = NULL;
    int *recvcounts = NULL;
    int *displs = NULL;
    if (visual && rank == 0) {
        full_grid = (unsigned char *)malloc(rows * cols * sizeof(unsigned char));
        recvcounts = (int *)malloc(size * sizeof(int));
        displs = (int *)malloc(size * sizeof(int));
        
        for (int i = 0; i < size; i++) {
            int r_local_rows = rows / size + ((i < remainder) ? 1 : 0);
            recvcounts[i] = r_local_rows * cols;
            if (i == 0) {
                displs[i] = 0;
            } else {
                displs[i] = displs[i-1] + recvcounts[i-1];
            }
        }
    }

    // Compute neighbors for halo exchange (ring topology)
    int top_neighbor = (rank - 1 + size) % size;
    int bottom_neighbor = (rank + 1) % size;

    // Start timing
    MPI_Barrier(MPI_COMM_WORLD);

    if (visual && rank == 0) {
        printf("\033[2J"); // Initial screen clear
    }

    double start_time = MPI_Wtime();

    for (int g = 0; g < gens; g++) {
        if (visual) {
            // Gather current state to rank 0 for printing
            MPI_Gatherv(current + cols, local_rows * cols, MPI_UNSIGNED_CHAR,
                        full_grid, recvcounts, displs, MPI_UNSIGNED_CHAR,
                        0, MPI_COMM_WORLD);
            if (rank == 0) {
                print_grid(full_grid, rows, cols, g);
                usleep(200000);
            }
        }

        // Exchange boundaries (Halo Exchange)
        MPI_Sendrecv(current + 1 * cols, cols, MPI_UNSIGNED_CHAR, top_neighbor, 0,
                     current + (local_rows + 1) * cols, cols, MPI_UNSIGNED_CHAR, bottom_neighbor, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        MPI_Sendrecv(current + local_rows * cols, cols, MPI_UNSIGNED_CHAR, bottom_neighbor, 1,
                     current + 0 * cols, cols, MPI_UNSIGNED_CHAR, top_neighbor, 1,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Compute next generation based on current + ghost rows
        compute_next_gen(current, next, local_rows, cols);

        // Swap pointers for next generation
        unsigned char *temp = current;
        current = next;
        next = temp;
    }

    // End timing
    MPI_Barrier(MPI_COMM_WORLD);
    double end_time = MPI_Wtime();
    double exec_time = end_time - start_time;

    // Count local live cells
    int local_count = 0;
    for (int i = 1; i <= local_rows; i++) {
        for (int j = 0; j < cols; j++) {
            local_count += current[i * cols + j];
        }
    }

    // Reduce counts to rank 0
    int global_count = 0;
    MPI_Reduce(&local_count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        if (!visual) {
            printf("Total Execution Time: %f seconds\n", exec_time);
            printf("Final Live Cell Count: %d\n", global_count);
        } else {
            // Gather final state for final print
            MPI_Gatherv(current + cols, local_rows * cols, MPI_UNSIGNED_CHAR,
                        full_grid, recvcounts, displs, MPI_UNSIGNED_CHAR,
                        0, MPI_COMM_WORLD);
            print_grid(full_grid, rows, cols, gens);
            printf("\nSimulation complete. Final Live Cells: %d\n", global_count);
        }
    }

    if (full_grid) free(full_grid);
    if (recvcounts) free(recvcounts);
    if (displs) free(displs);
    free(current);
    free(next);

    MPI_Finalize();
    return 0;
}
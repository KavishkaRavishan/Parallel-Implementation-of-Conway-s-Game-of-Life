/*
 * Course: EC7207
 * Group: 32
 * Description: Conway's Game of Life - OpenMP Implementation
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <omp.h>
#include <unistd.h> // For usleep

#define DEFAULT_ROWS 1000
#define DEFAULT_COLS 1000
#define DEFAULT_GENS 1000

// Function to allocate a 1D array to represent the 2D grid
unsigned char* allocate_grid(int rows, int cols) {
    unsigned char *grid = (unsigned char*)calloc(rows * cols, sizeof(unsigned char));
    if (grid == NULL) {
        fprintf(stderr, "Error allocating memory for grid.\n");
        exit(1);
    }
    return grid;
}

// Function to initialize the grid randomly
void initialize_grid(unsigned char *grid, int rows, int cols) {
    srand(42); // Fixed seed for reproducibility
    for (int i = 0; i < rows * cols; i++) {
        grid[i] = rand() % 2;
    }
}

// Function to print the grid to the terminal
void print_grid(unsigned char *grid, int rows, int cols, int gen) {
    printf("\033[H\033[J");
    printf("Generation: %d\n", gen);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%s", grid[i * cols + j] ? "█" : " ");
        }
        printf("\n");
    }
}

// Function to count live neighbors with periodic boundary conditions
int count_neighbors(unsigned char *grid, int r, int c, int rows, int cols) {
    int count = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) continue;
            
            // Handle wrap-around boundaries
            int ni = (r + i + rows) % rows;
            int nj = (c + j + cols) % cols;
            
            count += grid[ni * cols + nj];
        }
    }
    return count;
}

// Serial version for computing the next generation (baseline)
void compute_next_gen_serial(unsigned char *current, unsigned char *next, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int neighbors = count_neighbors(current, i, j, rows, cols);
            int idx = i * cols + j;
            
            if (current[idx] == 1) {
                next[idx] = (neighbors == 2 || neighbors == 3) ? 1 : 0;
            } else {
                next[idx] = (neighbors == 3) ? 1 : 0;
            }
        }
    }
}

// OpenMP version for computing the next generation
void compute_next_gen_omp(unsigned char *current, unsigned char *next, int rows, int cols) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int neighbors = count_neighbors(current, i, j, rows, cols);
            int idx = i * cols + j;
            
            if (current[idx] == 1) {
                next[idx] = (neighbors == 2 || neighbors == 3) ? 1 : 0;
            } else {
                next[idx] = (neighbors == 3) ? 1 : 0;
            }
        }
    }
}

// OpenMP version to count the total number of live cells
int count_live_cells_omp(unsigned char *grid, int rows, int cols) {
    int count = 0;
    #pragma omp parallel for reduction(+:count)
    for (int i = 0; i < rows * cols; i++) {
        count += grid[i];
    }
    return count;
}

int main(int argc, char *argv[]) {
    int rows = DEFAULT_ROWS;
    int cols = DEFAULT_COLS;
    int gens = DEFAULT_GENS;
    int threads = omp_get_max_threads();
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
        } else if (i == 4 && argv[i][0] != '-') {
            threads = atoi(argv[i]);
        }
    }

    omp_set_num_threads(threads);

    if (!visual) {
        printf("Grid Size: %dx%d\n", rows, cols);
        printf("Generations: %d\n", gens);
        printf("OpenMP Threads: %d\n", threads);
    }

    // --- Serial Run for Speedup Baseline ---
    double time_seq = 0.0;
    if (!visual) {
        unsigned char *current_seq = allocate_grid(rows, cols);
        unsigned char *next_seq = allocate_grid(rows, cols);
        initialize_grid(current_seq, rows, cols);

        double start_seq = omp_get_wtime();
        for (int g = 0; g < gens; g++) {
            compute_next_gen_serial(current_seq, next_seq, rows, cols);
            unsigned char *temp = current_seq;
            current_seq = next_seq;
            next_seq = temp;
        }
        double end_seq = omp_get_wtime();
        time_seq = end_seq - start_seq;
        free(current_seq);
        free(next_seq);
    }
    
    // --- OpenMP Run ---
    unsigned char *current_omp = allocate_grid(rows, cols);
    unsigned char *next_omp = allocate_grid(rows, cols);
    initialize_grid(current_omp, rows, cols); // Same seed for fair comparison

    double start_omp = omp_get_wtime();
    for (int g = 0; g < gens; g++) {
        if (visual) {
            print_grid(current_omp, rows, cols, g);
            usleep(100000);
        }
        compute_next_gen_omp(current_omp, next_omp, rows, cols);
        unsigned char *temp = current_omp;
        current_omp = next_omp;
        next_omp = temp;
    }
    double end_omp = omp_get_wtime();
    double time_omp = end_omp - start_omp;
    
    int live_cells = count_live_cells_omp(current_omp, rows, cols);

    if (!visual) {
        printf("Serial Execution Time: %f seconds\n", time_seq);
        printf("OpenMP Execution Time: %f seconds\n", time_omp);
        if (time_omp > 0) {
            printf("Speedup: %.2fx\n", time_seq / time_omp);
        }
        printf("Final Live Cell Count: %d\n", live_cells);
    } else {
        print_grid(current_omp, rows, cols, gens);
        printf("\nSimulation complete. Final Live Cells: %d\n", live_cells);
    }

    free(current_omp);
    free(next_omp);

    return 0;
}
<<<<<<< HEAD
/*
 * Course: EC7207
 * Group: 32
 * Description: Conway's Game of Life - Serial Implementation
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
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
        // 30% probability for a cell to be alive
        grid[i] = (rand() % 10 < 3) ? 1 : 0;
    }
}

// Function to print the grid to the terminal
void print_grid(unsigned char *grid, int rows, int cols, int gen) {
    // Move cursor to home (top-left) and clear from cursor to end of screen
    // This is smoother than clearing the whole screen
    printf("\033[H");
    
    int live_count = 0;
    // Calculate live cells
    for (int i = 0; i < rows * cols; i++) {
        if (grid[i] == 1) live_count++;
    }

    printf("Generation: %d | Live Cells: %d          \n", gen, live_count);
    
    // Build the grid output in a buffer to minimize syscalls and flickering
    // Each cell is roughly 3 bytes in UTF-8 (█) or 1 byte (space) + newline
    // We'll just print row by row to keep it simple but efficient
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fputs(grid[i * cols + j] ? "█" : " ", stdout);
        }
        putchar('\n');
    }
    // Clear the rest of the screen if the terminal was larger
    printf("\033[J");
    fflush(stdout);
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

// Function to compute the next generation
void compute_next_gen(unsigned char *current, unsigned char *next, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int neighbors = count_neighbors(current, i, j, rows, cols);
            int idx = i * cols + j;
            
            if (current[idx] == 1) {
                if (neighbors < 2 || neighbors > 3) {
                    next[idx] = 0; // Underpopulation or overpopulation
                } else {
                    next[idx] = 1; // Survival
                }
            } else {
                if (neighbors == 3) {
                    next[idx] = 1; // Reproduction
                } else {
                    next[idx] = 0; // Remains dead
                }
            }
        }
    }
}

// Function to count the total number of live cells
int count_live_cells(unsigned char *grid, int rows, int cols) {
    int count = 0;
    for (int i = 0; i < rows * cols; i++) {
        count += grid[i];
    }
    return count;
}

int main(int argc, char *argv[]) {
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

    if (!visual) {
        printf("Grid Size: %dx%d\n", rows, cols);
        printf("Generations: %d\n", gens);
    }

    unsigned char *current = allocate_grid(rows, cols);
    unsigned char *next = allocate_grid(rows, cols);

    initialize_grid(current, rows, cols);

    if (visual) {
        printf("\033[2J"); // Complete clear once
    }

    struct timespec start, end;
    
    // Start timing
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int g = 0; g < gens; g++) {
        if (visual) {
            print_grid(current, rows, cols, g);
            usleep(200000); // 200ms delay for better visibility
        }

        compute_next_gen(current, next, rows, cols);
        
        // Swap pointers for double buffering
        unsigned char *temp = current;
        current = next;
        next = temp;
    }

    // End timing
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    
    int live_cells = count_live_cells(current, rows, cols);

    if (!visual) {
        printf("Total Execution Time: %f seconds\n", time_taken);
        printf("Final Live Cell Count: %d\n", live_cells);
    } else {
        print_grid(current, rows, cols, gens);
        printf("\nSimulation complete. Final Live Cells: %d\n", live_cells);
    }

    free(current);
    free(next);

    return 0;
=======
/*
 * Course: EC7207
 * Group: 32
 * Description: Conway's Game of Life - Serial Implementation
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
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
    // Clear screen and move cursor to top-left
    printf("\033[H\033[J");
    printf("Generation: %d | Live Cells: %d\n", gen, 0); // Placeholder for live count
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

// Function to compute the next generation
void compute_next_gen(unsigned char *current, unsigned char *next, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int neighbors = count_neighbors(current, i, j, rows, cols);
            int idx = i * cols + j;
            
            if (current[idx] == 1) {
                if (neighbors < 2 || neighbors > 3) {
                    next[idx] = 0; // Underpopulation or overpopulation
                } else {
                    next[idx] = 1; // Survival
                }
            } else {
                if (neighbors == 3) {
                    next[idx] = 1; // Reproduction
                } else {
                    next[idx] = 0; // Remains dead
                }
            }
        }
    }
}

// Function to count the total number of live cells
int count_live_cells(unsigned char *grid, int rows, int cols) {
    int count = 0;
    for (int i = 0; i < rows * cols; i++) {
        count += grid[i];
    }
    return count;
}

int main(int argc, char *argv[]) {
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

    if (!visual) {
        printf("Grid Size: %dx%d\n", rows, cols);
        printf("Generations: %d\n", gens);
    }

    unsigned char *current = allocate_grid(rows, cols);
    unsigned char *next = allocate_grid(rows, cols);

    initialize_grid(current, rows, cols);

    struct timespec start, end;
    
    // Start timing
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int g = 0; g < gens; g++) {
        if (visual) {
            print_grid(current, rows, cols, g);
            usleep(100000); // 100ms delay
        }

        compute_next_gen(current, next, rows, cols);
        
        // Swap pointers for double buffering
        unsigned char *temp = current;
        current = next;
        next = temp;
    }

    // End timing
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    
    int live_cells = count_live_cells(current, rows, cols);

    if (!visual) {
        printf("Total Execution Time: %f seconds\n", time_taken);
        printf("Final Live Cell Count: %d\n", live_cells);
    } else {
        print_grid(current, rows, cols, gens);
        printf("\nSimulation complete. Final Live Cells: %d\n", live_cells);
    }

    free(current);
    free(next);

    return 0;
>>>>>>> 96aff32808efcc97c71ba406c274c3827faa3559
}
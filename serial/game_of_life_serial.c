/*
 * Course: EC7207
 * Group: 32
 * Description: Conway's Game of Life - Serial Implementation
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#define DEFAULT_ROWS 1000
#define DEFAULT_COLS 1000
#define DEFAULT_GENS 1000

unsigned char* allocate_grid(int rows, int cols) {
    unsigned char *grid = (unsigned char*)calloc(rows * cols, sizeof(unsigned char));
    if (grid == NULL) {
        fprintf(stderr, "Error allocating memory for grid.\n");
        exit(1);
    }
    return grid;
}

void initialize_grid(unsigned char *grid, int rows, int cols) {
    srand(42);
    for (int i = 0; i < rows * cols; i++) {
        grid[i] = (rand() % 10 < 3) ? 1 : 0;
    }
}

void print_grid(unsigned char *grid, int rows, int cols, int gen) {
    printf("\033[H");

    int live_count = 0;
    for (int i = 0; i < rows * cols; i++) {
        if (grid[i] == 1) live_count++;
    }

    printf("Generation: %d | Live Cells: %d          \n", gen, live_count);

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fputs(grid[i * cols + j] ? "█" : " ", stdout);
        }
        putchar('\n');
    }
    printf("\033[J");
    fflush(stdout);
}

int count_neighbors(unsigned char *grid, int r, int c, int rows, int cols) {
    int count = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) continue;

            int ni = (r + i + rows) % rows;
            int nj = (c + j + cols) % cols;

            count += grid[ni * cols + nj];
        }
    }
    return count;
}

void compute_next_gen(unsigned char *current, unsigned char *next, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int neighbors = count_neighbors(current, i, j, rows, cols);
            int idx = i * cols + j;

            if (current[idx] == 1) {
                if (neighbors < 2 || neighbors > 3) {
                    next[idx] = 0;
                } else {
                    next[idx] = 1;
                }
            } else {
                if (neighbors == 3) {
                    next[idx] = 1;
                } else {
                    next[idx] = 0;
                }
            }
        }
    }
}

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
        printf("\033[2J");
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int g = 0; g < gens; g++) {
        if (visual) {
            print_grid(current, rows, cols, g);
            usleep(200000);
        }

        compute_next_gen(current, next, rows, cols);

        unsigned char *temp = current;
        current = next;
        next = temp;
    }

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
}

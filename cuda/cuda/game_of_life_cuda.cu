/*
 * Course: EC7207
 * Group: 32
 * Description: Conway's Game of Life - CUDA Implementation
 */
#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>
#include <string.h>
#include <unistd.h>

#define DEFAULT_ROWS 1000
#define DEFAULT_COLS 1000
#define DEFAULT_GENS 1000
#define BLOCK_SIZE_X 16
#define BLOCK_SIZE_Y 16

// Function to initialize the grid randomly on the host
void initialize_grid(unsigned char *grid, int rows, int cols) {
    srand(42); // Fixed seed for reproducibility
    for (int i = 0; i < rows * cols; i++) {
        // 30% probability for a cell to be alive
        grid[i] = (rand() % 10 < 3) ? 1 : 0;
    }
}

// Function to print the grid to the terminal
void print_grid(unsigned char *grid, int rows, int cols, int gen) {
    // Move cursor to home (top-left)
    printf("\033[H");
    
    int live_count = 0;
    for (int i = 0; i < rows * cols; i++) {
        if (grid[i] == 1) live_count++;
    }

    printf("Generation: %d | Live Cells: %d (CUDA)          \n", gen, live_count);
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

// CUDA Kernel to compute the next generation
__global__ void compute_next_gen_kernel(const unsigned char *current, unsigned char *next, int rows, int cols) {
    int c = blockIdx.x * blockDim.x + threadIdx.x;
    int r = blockIdx.y * blockDim.y + threadIdx.y;

    if (r < rows && c < cols) {
        int count = 0;
        
        // Count live neighbors
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                if (i == 0 && j == 0) continue;
                
                int nr = r + i;
                int nc = c + j;
                
                // Wrap around boundaries
                nr = (nr + rows) % rows;
                nc = (nc + cols) % cols;
                
                count += current[nr * cols + nc];
            }
        }
        
        int idx = r * cols + c;
        if (current[idx] == 1) {
            next[idx] = (count == 2 || count == 3) ? 1 : 0;
        } else {
            next[idx] = (count == 3) ? 1 : 0;
        }
    }
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

    size_t grid_bytes = rows * cols * sizeof(unsigned char);

    // Host memory allocation
    unsigned char *h_grid = (unsigned char *)malloc(grid_bytes);
    if (h_grid == NULL) {
        fprintf(stderr, "Host memory allocation failed.\n");
        return 1;
    }

    initialize_grid(h_grid, rows, cols);

    if (visual) {
        printf("\033[2J"); // Initial screen clear
    }

    // Device memory allocation
    unsigned char *d_current;
    unsigned char *d_next;
    cudaMalloc((void **)&d_current, grid_bytes);
    cudaMalloc((void **)&d_next, grid_bytes);

    // Copy initial state to device
    cudaMemcpy(d_current, h_grid, grid_bytes, cudaMemcpyHostToDevice);

    // Setup execution configuration
    dim3 threads(BLOCK_SIZE_X, BLOCK_SIZE_Y);
    dim3 blocks((cols + BLOCK_SIZE_X - 1) / BLOCK_SIZE_X, (rows + BLOCK_SIZE_Y - 1) / BLOCK_SIZE_Y);

    // Setup CUDA events for timing
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);

    // Start timing
    cudaEventRecord(start);

    for (int g = 0; g < gens; g++) {
        if (visual) {
            cudaMemcpy(h_grid, d_current, grid_bytes, cudaMemcpyDeviceToHost);
            print_grid(h_grid, rows, cols, g);
            usleep(200000);
        }

        compute_next_gen_kernel<<<blocks, threads>>>(d_current, d_next, rows, cols);
        
        // Swap device pointers
        unsigned char *temp = d_current;
        d_current = d_next;
        d_next = temp;
    }

    // Stop timing
    cudaEventRecord(stop);
    cudaEventSynchronize(stop);

    float milliseconds = 0;
    cudaEventElapsedTime(&milliseconds, start, stop);
    double exec_time = milliseconds / 1000.0;

    // Copy final result back to host
    cudaMemcpy(h_grid, d_current, grid_bytes, cudaMemcpyDeviceToHost);

    // Count live cells
    int live_cells = 0;
    for (int i = 0; i < rows * cols; i++) {
        live_cells += h_grid[i];
    }

    if (!visual) {
        printf("Total Execution Time (GPU): %f seconds\n", exec_time);
        printf("Final Live Cell Count: %d\n", live_cells);
    } else {
        print_grid(h_grid, rows, cols, gens);
        printf("\nSimulation complete (GPU). Final Live Cells: %d\n", live_cells);
    }

    // Cleanup
    cudaEventDestroy(start);
    cudaEventDestroy(stop);
    cudaFree(d_current);
    cudaFree(d_next);
    free(h_grid);

    return 0;
}
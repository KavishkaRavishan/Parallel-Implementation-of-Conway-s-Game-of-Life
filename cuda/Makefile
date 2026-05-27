CC = nvcc
CFLAGS = -O2

all: gol_cuda

gol_cuda: game_of_life_cuda.cu
	$(CC) $(CFLAGS) -o gol_cuda game_of_life_cuda.cu

clean:
	rm -f gol_cuda gol_cuda.exe

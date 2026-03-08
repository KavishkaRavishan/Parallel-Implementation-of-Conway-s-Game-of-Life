CC = gcc
CFLAGS = -O2 -fopenmp

all: gol_openmp

gol_openmp: game_of_life_openmp.c
	$(CC) $(CFLAGS) -o gol_openmp game_of_life_openmp.c

clean:
	rm -f gol_openmp gol_openmp.exe

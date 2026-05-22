CC = mpicc
CFLAGS = -O2 -fopenmp

all: gol_hybrid

gol_hybrid: game_of_life_hybrid.c
	$(CC) $(CFLAGS) -o gol_hybrid game_of_life_hybrid.c

clean:
	rm -f gol_hybrid gol_hybrid.exe

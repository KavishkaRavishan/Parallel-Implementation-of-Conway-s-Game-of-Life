CC = mpicc
CFLAGS = -O2

all: gol_mpi

gol_mpi: game_of_life_mpi.c
	$(CC) $(CFLAGS) -o gol_mpi game_of_life_mpi.c

clean:
	rm -f gol_mpi gol_mpi.exe

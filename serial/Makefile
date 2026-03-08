CC = gcc
CFLAGS = -O2

all: gol_serial

gol_serial: game_of_life_serial.c
	$(CC) $(CFLAGS) -o gol_serial game_of_life_serial.c

clean:
	rm -f gol_serial gol_serial.exe

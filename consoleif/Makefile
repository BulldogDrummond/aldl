# compiler flags
CFLAGS= -g -Wall

all: consoleif.o

consoleif.o: consoleif.c consoleif.h 
	gcc -lncurses $(CFLAGS) -c consoleif.c -o consoleif.o

clean:
	rm -f *.o

stats:
	wc -l *.c *.h */*.c */*.h

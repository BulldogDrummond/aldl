# compiler flags
CFLAGS= -O2 -Wall

all: consoleif.o datalogger.o debugif.o

consoleif.o: consoleif.c modules.h
	gcc -lncurses $(CFLAGS) -c consoleif.c -o consoleif.o

datalogger.o: datalogger.c modules.h
	gcc $(CFLAGS) -c datalogger.c -o datalogger.o

debugif.o: debugif.c modules.h
	gcc $(CFLAGS) -c debugif.c -o debugif.o

clean:
	rm -f *.o

stats:
	wc -l *.c *.h */*.c */*.h

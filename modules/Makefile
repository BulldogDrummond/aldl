# compiler flags
CFLAGS= -O2 -Wall

all: consoleif.o datalogger.o dataserver.o remote.o

consoleif.o: consoleif.c modules.h
	gcc -lncurses $(CFLAGS) -c consoleif.c -o consoleif.o

datalogger.o: datalogger.c modules.h
	gcc $(CFLAGS) -c datalogger.c -o datalogger.o

dataserver.o: dataserver.c modules.h
	gcc $(CFLAGS) -c dataserver.c -o dataserver.o

remote.o: remote.c modules.h
	gcc $(CFLAGS) -c remote.c -o remote.o

clean:
	rm -f *.o

stats:
	wc -l *.c *.h */*.c */*.h

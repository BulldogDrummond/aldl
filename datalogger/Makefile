# compiler flags
CFLAGS= -O2 -Wall

all: datalogger.o

datalogger.o: datalogger.c datalogger.h
	gcc $(CFLAGS) -c datalogger.c -o datalogger.o

clean:
	rm -f *.o

stats:
	wc -l *.c *.h */*.c */*.h

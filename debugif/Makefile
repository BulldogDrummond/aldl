# compiler flags
CFLAGS= -O2 -Wall

all: debugif.o

debugif.o: debugif.c debugif.h
	gcc $(CFLAGS) -c debugif.c -o debugif.o

clean:
	rm -f *.o

stats:
	wc -l *.c *.h */*.c */*.h

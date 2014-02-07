# compiler flags
CFLAGS= -O2 -Wall

all: aldl-analyzer

aldl-analyzer: analyzer.c csv.o
	gcc $(CFLAGS) -o aldl-analyzer analyzer.c csv.o

csv.o: csv.c csv.h
	gcc $(CFLAGS) -c csv.c

clean:
	rm -f aldl-analyzer *.o

stats:
	wc -l *.c *.h */*.c */*.h

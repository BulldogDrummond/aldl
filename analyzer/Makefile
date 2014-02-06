# compiler flags
CFLAGS= -O2 -Wall

all: aldl-analyzer

aldl-analyzer: analyzer.c
	gcc $(CFLAGS) analyzer.c

clean:
	rm -f aldl-analyzer

stats:
	wc -l *.c *.h */*.c */*.h

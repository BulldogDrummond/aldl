#ifndef FILEBREAKER_H
#define FILEBREAKER_H

# compiler flags
CFLAGS= -O0 -g -Wall
LIBS= -lftdi
OBJS= serio.o aldlcomm.o aldldata.o

all: test filebreaker.o

test: test.c
	gcc $(CFLAGS) test.c -o test

filebreaker.o: filebreaker.c filebreaker.h
	gcc $(CFLAGS) -c filebreaker.c -o filebreaker.o

clean:
	rm -f *.o test

stats:
	wc -l *.c *.h

#endif

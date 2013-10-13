# compiler flags
CFLAGS= -O0 -g -Wall
LIBS= -lftdi

all: ftdi

serio-ftdi.o: serio-ftdi.c aldl-io.h
	gcc $(CFLAGS) -c serio-ftdi.c -o serio-ftdi.o

aldlcomm.o: serio-ftdi.o serio-tty.o
	gcc $(CFLAGS) -c aldlcomm.c -o aldlcomm.o

aldldata.o: aldldata.c
	gcc $(CFLAGS) -c aldldata.c -o aldldata.o

ftdi: serio-ftdi.o aldlcomm.o aldldata.o
	ar rcs aldl-io-ftdi.a serio-ftdi.o aldlcomm.o aldldata.o

clean:
	rm -f *.o *.a

stats:
	wc -l *.c *.h

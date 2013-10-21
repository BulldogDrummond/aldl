# compiler flags
CFLAGS= -g -Wall
OBJS= debugif/debugif.o acquire.o error.o dfiler.o loadconfig.o
FTDI= /usr/lib/arm-linux-gnueabihf/libftdi.a

all: aldl-ftdi

aldl-ftdi: main.c aldl-io-ftdi debugif_ acquire.o error.o dfiler.o loadconfig.o
	gcc $(CFLAGS) -lftdi -lpthread main.c -o aldl-ftdi $(OBJS) aldl-io/aldl-io-ftdi.a

loadconfig.o: loadconfig.c loadconfig.h
	gcc $(CFLAGS) -c loadconfig.c -o loadconfig.o

dfiler.o: dfiler.c dfiler.h
	gcc $(CFLAGS) -c dfiler.c -o dfiler.o

acquire.o: acquire.h
	gcc $(CFLAGS) -c acquire.c -o acquire.o

error.o: error.c error.h
	gcc $(CFLAGS) -c error.c -o error.o

configfile_:
	cd configfile ; make ; cd ..

aldl-io-ftdi:
	cd aldl-io ; make ; cd ..

debugif_:
	cd debugif ; make ; cd ..

clean:
	rm -f *.o aldl-ftdi
	cd aldl-io ; make clean ; cd ..
	cd configfile ; make clean ; cd ..
	cd debugif ; make clean ; cd ..

stats:
	wc -l *.c *.h */*.c */*.h

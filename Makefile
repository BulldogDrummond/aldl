# compiler flags
CFLAGS= -g -Wall
OBJS= configfile/lex.yy.o configfile/y.tab.o configfile/varstore.o debugif/debugif.o acquire.o error.o
FTDI= /usr/lib/arm-linux-gnueabihf/libftdi.a

all: aldl-ftdi

aldl-ftdi: main.c aldl-io-ftdi configfile_ debugif_ acquire.o error.o
	gcc $(CFLAGS) -lftdi -lpthread main.c -o aldl-ftdi $(OBJS) aldl-io/aldl-io-ftdi.a

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

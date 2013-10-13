# compiler flags
CFLAGS= -g -Wall
OBJS= configfile/lex.yy.o configfile/y.tab.o configfile/varstore.o debugif/debugif.o
FTDI= /usr/lib/arm-linux-gnueabihf/libftdi.a

all: aldl-ftdi

aldl-ftdi: main.c aldl-io-ftdi configfile_ debugif_
	gcc $(CFLAGS) -lftdi main.c -o aldl-ftdi $(OBJS) aldl-io/aldl-io-ftdi.a

configfile_:
	cd configfile ; make ; cd ..

aldl-io-ftdi:
	cd aldl-io ; make ftdi ; cd ..

debugif_:
	cd debugif ; make ; cd ..

clean:
	rm -f *.o aldl-ftdi
	cd aldl-io ; make clean ; cd ..
	cd configfile ; make clean ; cd ..

stats:
	wc -l *.c *.h */*.c */*.h

# compiler flags
CFLAGS= -g -Wall
OBJS= configfile/lex.yy.o configfile/y.tab.o configfile/varstore.o debugif/debugif.o
FTDI= /usr/lib/arm-linux-gnueabihf/libftdi.a

all: aldl-ftdi

aldl-ftdi: main.c aldl-io-ftdi configfile_ debugif_
	gcc $(CFLAGS) -lftdi main.c -o aldl-ftdi $(OBJS) aldl-io/aldl-io-ftdi.a

aldl-tty: main.c aldl-io-tty configfile_
	gcc $(CFLAGS) main.c -o aldl-tty $(OBJS) aldl-io/aldl-io-tty.a

configfile_:
	cd configfile ; make ; cd ..

aldl-io-ftdi:
	cd aldl-io ; make ftdi ; cd ..

aldl-io-tty:
	cd aldl-io ; make tty ; cd ..

debugif_:
	cd debugif ; make ; cd ..

clean:
	rm -f *.o aldl-ftdi aldl-tty
	cd aldl-io ; make clean ; cd ..
	cd configfile ; make clean ; cd ..

stats:
	wc -l *.c *.h */*.c */*.h

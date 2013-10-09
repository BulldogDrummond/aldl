# compiler flags
CFLAGS= -g -Wall
OBJS= configfile/lex.yy.o configfile/y.tab.o configfile/varstore.o
FTDI= /usr/lib/arm-linux-gnueabihf/libftdi.a

all: aldl-ftdi

aldl-ftdi: main.c aldl-io-ftdi configfile clean
	gcc $(CFLAGS) -lftdi main.c -o aldl-ftdi $(OBJS) aldl-io/aldl-io-ftdi.a

aldl-tty: main.c aldl-io-tty configfile clean
	gcc $(CFLAGS) main.c -o aldl-tty $(OBJS) aldl-io/aldl-io-tty.a

configfile:
	cd configfile ; make ; cd ..

aldl-io-ftdi:
	cd aldl-io ; make clean ; make ftdi ; cd ..

aldl-io-tty:
	cd aldl-io ; make clean ; make tty ; cd ..

clean:
	rm -f *.o aldl-ftdi aldl-tty

stats:
	wc -l *.c *.h

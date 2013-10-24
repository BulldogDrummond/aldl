# compiler flags
CFLAGS= -g -Wall
#CFLAGS= -O2 -Wall
OBJS= debugif/debugif.o consoleif/consoleif.o acquire.o error.o loadconfig.o useful.o
FTDI= /usr/lib/arm-linux-gnueabihf/libftdi.a

all: clean aldl-ftdi aldl-dummy

aldl-ftdi: main.c aldl-io-ftdi.a config.h aldl-io.h aldl-types.h debugif_ $(OBJS)
	gcc $(CFLAGS) -lftdi -lpthread -lncurses main.c -o aldl-ftdi $(OBJS) aldl-io-ftdi.a

aldl-dummy: main.c aldl-io-dummy.a config.h aldl-io.h aldl-types.h debugif_ $(OBJS)
	gcc $(CFLAGS) -lftdi -lpthread -lncurses main.c -o aldl-dummy $(OBJS) aldl-io-dummy.a

useful.o: useful.c useful.h config.h aldl-types.h
	gcc $(CFLAGS) -c useful.c -o useful.o

loadconfig.o: loadconfig.c loadconfig.h config.h aldl-types.h
	gcc $(CFLAGS) -c loadconfig.c -o loadconfig.o

acquire.o: acquire.c acquire.h config.h aldl-io.h aldl-types.h
	gcc $(CFLAGS) -c acquire.c -o acquire.o

error.o: error.c error.h config.h aldl-types.h
	gcc $(CFLAGS) -c error.c -o error.o

debugif_:
	cd debugif ; make ; cd ..

consoleif_:
	cd consoleif ; make ; cd ..

serio-ftdi.o: serio-ftdi.c aldl-io.h aldl-types.h config.h
	gcc $(CFLAGS) -c serio-ftdi.c -o serio-ftdi.o

serio-dummy.o: serio-dummy.c aldl-io.h aldl-types.h config.h
	gcc $(CFLAGS) -c serio-dummy.c -o serio-dummy.o

aldlcomm.o: aldl-io.h aldl-types.h serio-ftdi.o config.h
	gcc $(CFLAGS) -c aldlcomm.c -o aldlcomm.o

aldldata.o: aldl-io.h aldl-types.h aldldata.c aldlcomm.o config.h
	gcc $(CFLAGS) -c aldldata.c -o aldldata.o

aldl-io-ftdi.a: serio-ftdi.o aldlcomm.o aldldata.o config.h
	ar rcs aldl-io-ftdi.a serio-ftdi.o aldlcomm.o aldldata.o

aldl-io-dummy.a: serio-dummy.o aldlcomm.o aldldata.o config.h
	ar rcs aldl-io-dummy.a serio-dummy.o aldlcomm.o aldldata.o

clean:
	rm -fv *.o *.a aldl-ftdi aldl-dummy
	cd debugif ; make clean ; cd ..
	cd consoleif ; make clean ; cd ..

stats:
	wc -l *.c *.h

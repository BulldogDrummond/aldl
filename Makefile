# compiler flags
CFLAGS= -g -Wall
#CFLAGS= -O2 -Wall
OBJS= debugif/debugif.o acquire.o error.o loadconfig.o useful.o
FTDI= /usr/lib/arm-linux-gnueabihf/libftdi.a

all: aldl-ftdi

aldl-ftdi: main.c aldl-io-ftdi.a config.h aldl-io.h aldl-types.h debugif_ $(OBJS)
	gcc $(CFLAGS) -lftdi -lpthread main.c -o aldl-ftdi $(OBJS) aldl-io-ftdi.a

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

serio-ftdi.o: serio-ftdi.c aldl-io.h aldl-types.h config.h
	gcc $(CFLAGS) -c serio-ftdi.c -o serio-ftdi.o

aldlcomm.o: aldl-io.h aldl-types.h serio-ftdi.o config.h
	gcc $(CFLAGS) -c aldlcomm.c -o aldlcomm.o

aldldata.o: aldl-io.h aldl-types.h aldldata.c aldlcomm.o config.h
	gcc $(CFLAGS) -c aldldata.c -o aldldata.o

aldl-io-ftdi.a: serio-ftdi.o aldlcomm.o aldldata.o config.h
	ar rcs aldl-io-ftdi.a serio-ftdi.o aldlcomm.o aldldata.o

clean:
	rm -f *.o *.a aldl-ftdi
	cd debugif ; make clean ; cd ..

stats:
	wc -l *.c *.h

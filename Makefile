# compiler flags
CFLAGS= -g -Wall
OBJS= acquire.o error.o loadconfig.o useful.o aldlcomm.o aldldata.o
MODULES= modules/*.o
LIBS= -lpthread -lrt -lncurses

all: aldl-ftdi aldl-dummy

aldl-ftdi: main.c serio-ftdi.o config.h aldl-io.h aldl-types.h modules_ $(OBJS)
	gcc $(CFLAGS) -lftdi $(LIBS) main.c -o aldl-ftdi $(OBJS) $(MODULES) serio-ftdi.o

aldl-dummy: main.c serio-dummy.o config.h aldl-io.h aldl-types.h modules_ $(OBJS)
	gcc $(CFLAGS) $(LIBS) main.c -o aldl-dummy $(OBJS) $(MODULES) serio-dummy.o

useful.o: useful.c useful.h config.h aldl-types.h
	gcc $(CFLAGS) -c useful.c -o useful.o

loadconfig.o: loadconfig.c loadconfig.h config.h aldl-types.h
	gcc $(CFLAGS) -c loadconfig.c -o loadconfig.o

acquire.o: acquire.c acquire.h config.h aldl-io.h aldl-types.h
	gcc $(CFLAGS) -c acquire.c -o acquire.o

error.o: error.c error.h config.h aldl-types.h
	gcc $(CFLAGS) -c error.c -o error.o

modules_:
	cd modules ; make ; cd ..

serio-ftdi.o: serio-ftdi.c aldl-io.h aldl-types.h config.h
	gcc $(CFLAGS) -c serio-ftdi.c -o serio-ftdi.o

serio-dummy.o: serio-dummy.c aldl-io.h aldl-types.h config.h
	gcc $(CFLAGS) -c serio-dummy.c -o serio-dummy.o

aldlcomm.o: aldl-io.h aldl-types.h serio-ftdi.o config.h
	gcc $(CFLAGS) -c aldlcomm.c -o aldlcomm.o

aldldata.o: aldl-io.h aldl-types.h aldldata.c aldlcomm.o config.h
	gcc $(CFLAGS) -c aldldata.c -o aldldata.o

clean:
	rm -fv *.o *.a aldl-ftdi aldl-dummy
	cd modules ; make clean ; cd ..

stats:
	wc -l *.c *.h */*.c */*.h

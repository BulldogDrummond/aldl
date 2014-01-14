# compiler flags
CFLAGS= -O2 -Wall
OBJS= acquire.o error.o loadconfig.o useful.o aldlcomm.o aldldata.o
MODULES= modules/*.o
LIBS= -lpthread -lrt -lncurses

# install configuration
CONFIGDIR= /etc/aldl
LOGDIR= /var/log/aldl
BINDIR= /usr/local/bin
BINARIES= aldl-ftdi aldl-dummy

.PHONY: clean install stats _modules

all: aldl-ftdi aldl-dummy
	@echo
	@echo '*********************************************************'
	@echo ' Run the following as root to install the binaries and'
	@echo ' config files:  make install'
	@echo '*********************************************************'
	@echo

install: aldl-ftdi aldl-dummy
	@echo Installing to $(BINDIR)
	cp -fv $(BINARIES) $(BINDIR)/
	ln -sf $(BINDIR)/aldl-ftdi $(BINDIR)/aldl
	@echo 'Creating directory structure'
	mkdir -pv $(CONFIGDIR)
	mkdir -pv $(LOGDIR)
	@echo 'Copying example configs, will not overwrite...'
	cp -nv ./examples/* $(CONFIGDIR)/
	@echo '******* WARNING ********'
	@echo no automatic updates of configs are done.  please see
	@echo examples/ if this was an existing installation, and
	@echo attempt to merge these changes manually...
	@echo Install complete, see configs in $(CONFIGDIR) before running

aldl-ftdi: main.c serio-ftdi.o config.h aldl-io.h aldl-types.h modules_ $(OBJS)
	gcc $(CFLAGS) $(LIBS) -lftdi main.c -o aldl-ftdi $(OBJS) $(MODULES) serio-ftdi.o
	@echo
	@echo '***************************************************'
	@echo ' You must blacklist or rmmod the ftdi_sio driver!!'
	@echo ' Debian users can try the script in extras/ '
	@echo '***************************************************'
	@echo

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

/* WARNING !!!!!! THIS IS BROKEN DONT USE IT YET */
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <time.h>

#include "serio.h"
#include "aldl-io.h"
#include "config.h"

/****************GLOBALSn'STRUCTURES*****************************/

/* use a global file descriptor for the serial port.  it should never
   change .. */
int fd;

/* a structure containing runtime-configurable timing specifications */
struct timing_t {
  int sleepy; /* the delay between most other polls, and the timeout divisor */
  int adder; /* an adder to help calculate some instructional time */
  int chatterwait;
};
struct timing_t timing;

/***************FUNCTION DEFS************************************/

/* configures the timing structure */
void serial_set_timing();

/* this sets misc. attribs for the serial port. */
int serial_config_attrib(int fd);

/* sets read blocking for the serial port */
void serial_set_block(int fd, int s_block);

/* placeholder error handler. */
void fatalerror(int errno, int errloc, char *errnotes);

/* temporary codes:
99 - generic error, see notes
100 - port could not be opened.
101 - failed to retrieve serial port attributes (not a serial port?)
102 - failed to set attributes, perhaps specified an invalid attribute
104 - unsupported connection method
299 - ftdi error
*/

/* sleep for ms milliseconds */
inline void msleep(int ms);

/****************FUNCTIONS**************************************/

inline void msleep(int ms) {
  usleep(ms * 1000); /* just use usleep and convert from ms in unix */
}

void serial_set_timing() {
  /* FIXME should actually import some stuff here. */
  /* these are just some sane defaults */
  timing.sleepy = 3;
  timing.adder = 9;
  timing.chatterwait = 650;
};

int serial_init(char *port) {
  #ifdef SERIAL_VERBOSE
  printf("verbosity in the serial handling routine is enabled.\n");
  printf("serial_init opening port @ %s with method tty\n",port);
  #endif

  serial_set_timing();

  fd = open(port,O_RDWR|O_NOCTTY|O_SYNC);
  if(fd < 0) {
    fatalerror(100,1,"failed to get a FD, probably INVALID DEVICE");
  };
  serial_config_attrib(fd);
  serial_set_block(fd,0); /* no blocking */
  return fd;
}

void serial_close() {
  close(fd);
}

void serial_chatterwait() {
  /* FIXME this sucks, it's just a placeholder.  it waits for  single
     byte ... */
  while(serial_skip_bytes(1,50) == 0) {
    msleep(timing.chatterwait);
  };
  return;
}

int serial_config_attrib(int fd) {
  #ifdef SERIAL_DEBUG
    if(fd <=0) fatalerror(99,1,"passed a null file descriptor to sca");
  #endif
  #ifdef SERIAL_VERBOSE
    printf("serial_config_attrib configuring descriptor %i\n",fd);
  #endif

  /* allocate tty */
  struct termios tty;
  memset(&tty,0,sizeof(tty));

  /* get the existing attributes of the terminal. */
  if(tcgetattr(fd,&tty) != 0) {
    fatalerror(101,1,"failed get attrib");
  };

  /* configs (need to fuck with this more, they're just for testing) */
  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; /* charset */
  tty.c_iflag &= ~IGNBRK; /* ignore break */
  tty.c_lflag = 0; /* no signaling chars, echo */
  tty.c_oflag = 0; /* no remapping, no delay */
  tty.c_cc[VMIN] = 0; /* don't block on read */
  tty.c_cc[VTIME] = 5; /* read timeout (10=1second) */
  tty.c_iflag &= ~(IXON | IXOFF | IXANY); /* no xon/xoff ctrl */
  tty.c_cflag |= (CLOCAL | CREAD); /* ignore modem control codes */
  tty.c_cflag &= ~(PARENB | PARODD); /* disable parity */
  tty.c_cflag |= 0; /* set parity */
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CRTSCTS;

  if(tcsetattr(fd, TCSANOW, &tty) != 0) {
    fatalerror(102,1,"failed to set attributes");
    return -1;
  };

  return 0;
}

void serial_set_block(int fd, int s_block) {
  #ifdef SERIAL_VERBOSE
    printf("set block @ fd %i, blocking %i\n",fd,s_block);
  #endif
  struct termios tty;
  memset (&tty, 0, sizeof tty);
  if(tcgetattr(fd,&tty) != 0) {
    fatalerror(101,1,"failed get attrib at set block");
  };
  tty.c_cc[VMIN] = s_block ? 1 : 0;
  tty.c_cc[VTIME] = 5; /* read timeout 10=1sec */
  if(tcsetattr(fd,TCSANOW,&tty) != 0) {
    fatalerror(102,1,"failed set attrib in block");
    return;
  };
}

int serial_purge() {
  tcflush(fd,TCIOFLUSH);
  return 0;
}

int serial_write(char *str) {
  serial_f_write(str,strlen(str));
  return 0;
}

inline int serial_f_write(char *str, int len) {
  /* check for 0 length or null string */
  if(str == NULL || len == 0) {
    #ifdef SERIAL_VERBOSE
      printf("non-fatal, attempted serial write of 0 len or null string\n");
    #endif
    return 1;
  };

  write(fd,str,len); /* standard serial tty write */
  return 0;
}

inline int serial_read_bytes(char *str, int bytes, int timeout) {
  int bytes_read = 0;
  int timespent = 0;
  do {
    bytes_read += serial_read(str + bytes_read, bytes - bytes_read);
    if(bytes_read >= bytes) return 1;
    msleep(timing.sleepy);
    timespent += timing.sleepy + timing.adder; 
  } while (timespent <= timeout);
  return 0;
}

inline int serial_skip_bytes(int bytes, int timeout) {
  int bytes_read = 0;
  int timespent = 0;
  /* it would seem the only way to do this in a driver-independant way is
     to just read into a buffer, and discard it.  basically just costs us
     a malloc. */
  char *buf = malloc(sizeof(char) * bytes);
  do {
    bytes_read += serial_read(buf, bytes - bytes_read);
    if(bytes_read >= bytes) {
      free(buf);
      return 1;
    };
    msleep(timing.sleepy);
    timespent += timing.sleepy + timing.adder;
  } while (timespent <= timeout);
  free(buf);
  return 0;
}

inline int serial_read(char *str, int len) {
  /* check for null string or 0 length */
  if(str == NULL || len == 0) {
    #ifdef SERIAL_VERBOSE
      printf("non-fatal, attempted serial read to NULL buffer or 0 len\n");
    #endif
    return 0;
  };
  int resp = 0; /* to store response from whatever read */
  resp = read(fd,str,len);
  if(resp < 0) { /* there was an error */
    fatalerror(128,1,"read failed ... ");
  };
  return resp; /* return number of chars read (incl. 0) */
}

/* FIXME this peice of shit needs to be synchronized with the superior routine
   in ftdi's lib when it's done, for now it just bails. */
int serial_listen(char *str, int len, int max, int timeout) {
  return 0;
}

void fatalerror(int errno, int errloc, char *errnotes) {
  fprintf(stderr,"FATAL ERROR: errno %i errloc %i\n",errno,errloc);
  #ifdef SERIAL_DEBUG
    if(errnotes != NULL) fprintf(stderr,"DEBUG NOTE: %s\n",errnotes);
  #endif
  fprintf(stderr,"-- this used the depreciated error handler\n");
  fprintf(stderr,"exiting.\n");
  /* error 104 is the bad method error.  don't try to close anything if the
     method is bad, it'll probably cause an infinite loop, since close will
     generate another bad method error. */
  if(errno != 104) serial_close();
  exit(0);
}


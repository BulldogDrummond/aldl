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

#include <ftdi.h>

#include "serio.h"
#include "aldl-io.h"

#include "config.h"

/****************GLOBALSn'STRUCTURES*****************************/

/* a connection flag.  useage depends on method. */
int fcon;

/* global ftdi context pointer */
struct ftdi_context *ftdi;

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

/* inits the ftdi driver by a special port description:
   d:devicenode (usually at /proc/bus/usb)
   i:vendor:product
   i:vendor:product:index
   s:vendor:product:serial
*/
int serial_init_ftdi(char *port, int baud);

/* special ftdi error handler, bails if errno<0.  could be macro. */
void ftdierror(int loc,int errno);

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
  printf("serial_init opening port @ %s with method ftdi\n",port);
  #endif

  fcon = 0;

  serial_set_timing();
  serial_init_ftdi(port,8192);
  return 0;
}

void serial_close() {
  if(fcon != 0) ftdi_usb_close(ftdi);
  ftdi_free(ftdi);
}

void serial_chatterwait() {
  /* FIXME this sucks, it's just a placeholder.  it waits for  single
     byte ... */
  while(serial_skip_bytes(1,50) == 0) {
    msleep(timing.chatterwait);
  };
  return;
}

int serial_init_ftdi(char *port, int baud) {
  /* new ftdi instance */
  if((ftdi = ftdi_new()) == NULL) {
    fatalerror(299,1,"ftdi_new failed");
  };
  ftdierror(2,ftdi_usb_open_string(ftdi,port)); /* trap error */

  #ifdef SERIAL_VERBOSE
  printf("init ftdi userland driver appears sucessful...\n");
  #endif

  ftdierror(3,ftdi_set_baudrate(ftdi,baud));
  fcon = 1; /* set only on successful connection */
  return 0;
};

int serial_purge() {
  ftdierror(88,ftdi_usb_purge_buffers(ftdi));
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
  #ifdef SERIAL_VERBOSE
  printf("WRITE: ");
  printhexstring(str,len);
  #endif

  ftdierror(6,ftdi_write_data(ftdi,(unsigned char *)str,len));
  return 0;
}

inline int serial_read_bytes(char *str, int bytes, int timeout) {
  int bytes_read = 0;
  int timespent = 0;
  do {
    bytes_read += serial_read(str + bytes_read, bytes - bytes_read);
    if(bytes_read >= bytes) {
      #ifdef SERIAL_VERBOSE
      printf("READ: ");
      printhexstring(str,bytes);
      #endif
      return 1;
    }
    msleep(timing.sleepy);
    timespent += timing.sleepy + timing.adder; 
  } while (timespent <= timeout);
  #ifdef SERIAL_VERBOSE
  printf("TIMEOUT TRYING TO READ %i BYTES, GOT: ",bytes);
  printhexstring(str,bytes_read);
  #endif
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
      #ifdef SERIAL_VERBOSE
      printf("SKIP: ");
      printhexstring(buf,bytes);
      #endif
      free(buf);
      return 1;
    };
    msleep(timing.sleepy);
    timespent += timing.sleepy + timing.adder;
  } while (timespent <= timeout);
  free(buf);
  #ifdef SERIAL_VERBOSE
  printf("TIMEOUT TRYING TO SKIP %i BYTES\n",bytes);
  #endif
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
  resp = ftdi_read_data(ftdi,(unsigned char *)str,len);
  ftdierror(22,resp); /* this will break if resp<0 */
  #ifdef SERIAL_VERBOSE
  printf("READ: ");
  printhexstring(str,len);
  #endif

  return resp; /* return number of bytes read, or zero */
}

int serial_listen(char *str, int len, int max, int timeout) {
  int chars_read = 0; /* functions as cursor */
  int chars_in = 0;
  int timespent = 0; /* estimation of time spent */
  char *buf = malloc(max); /* buffer for incoming data */
  memset(buf,0,max);
  #ifdef SERIAL_VERBOSE
  printf("LISTEN: ");
  printhexstring(str,len);
  #endif
  while(chars_read < max) {
    chars_in = serial_read(buf + chars_read,max - chars_read);
    if(chars_in > 0) {
      if(strstr(buf,str) != NULL) {
        free(buf);
        return 1;
      };
      chars_read += chars_in; /* mv cursor */
    } else { /* no chars were read */
      buf[chars_read] = 0; /* might not be necessary */
    };
    /* timeout and throttling routine */
    msleep(timing.sleepy); /* timing delay */
    if(timeout > 0) { /* timeout is enabled, we arent waiting forever */
      timespent += timing.sleepy + timing.adder; /* increment est. time */
      if(timespent >= timeout) { /* timeout exceeded */
        free(buf);
        return 0;
      };
    };
  };
  free(buf);
  return 0; /* got max chars with no result */
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

void ftdierror(int loc,int errno) {
  if(errno>=0) return;
  fprintf(stderr,"FTDI ERROR: %i, %s\n",errno,ftdi_get_error_string(ftdi)); 
  fatalerror(299,loc,NULL);
};


#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <ftdi.h>

#include "serio.h"
#include "aldl-io.h"

#include "config.h"

/****************GLOBALSn'STRUCTURES*****************************/

/* global ftdi context pointer */
struct ftdi_context *ftdi;

/***************FUNCTION DEFS************************************/

/* placeholder error handler. */
void fatalerror(int errno, int errloc, char *errnotes);

/* temporary codes:
99 - generic error, see notes
100 - port could not be opened.
101 - failed to retrieve serial port attributes (not a serial port?)
102 - failed to set attributes, perhaps specified an invalid attribute
104 - unsupported connection method
299 - ftdi error
301 - autodetection failed no devices found
302 - autodetection failed for some reason
*/

/* init the ftdi driver by a special port description:
   d:devicenode (usually at /proc/bus/usb)
   i:vendor:product
   i:vendor:product:index
   s:vendor:product:serial
*/

/* special ftdi error handler, bails if errno<0.  could be macro. */
void ftdierror(int loc,int errno);

/****************FUNCTIONS**************************************/

void serial_close() {
  ftdi_usb_close(ftdi);
  ftdi_free(ftdi);
}

int serial_init(char *port) {
  #ifdef SERIAL_VERBOSE
  printf("verbosity in the serial handling routine is enabled.\n");
  printf("serial_init opening port @ %s with method ftdi\n",port);
  #endif

  /* new ftdi instance */
  if((ftdi = ftdi_new()) == NULL) {
    fatalerror(299,1,"ftdi_new failed");
  };

  if(port != NULL) { /* static device config */
    ftdierror(2,ftdi_usb_open_string(ftdi,port)); /* trap error */
  } else { /* autodetect mode */
    struct ftdi_device_list **devlist = NULL;
    int n_devices = ftdi_usb_find_all(ftdi,devlist,0,0);
    if(n_devices > 0) { /* device found */
      /* right now this just grabs the first available ftdi device .. */
      ftdierror(2,ftdi_usb_open_dev(ftdi,devlist[0]->dev));
      ftdi_list_free(devlist);
    } else if(n_devices == 0) { /* nothing found */
      fatalerror(301,1,"no devices found, please connect your serial cable");
    } else {
      fatalerror(302,1,"ftdi detection failed for some reason.");
    };
  };

  #ifdef SERIAL_VERBOSE
  printf("init ftdi userland driver appears sucessful...\n");
  #endif

  /* set baud rate */
  ftdierror(3,ftdi_set_baudrate(ftdi,FTDI_BAUD));

  /* set latency timer */
  ftdierror(3,ftdi_set_latency_timer(ftdi,2));

  /* put connection state tracking here .*/

  return 0;
};

void serial_purge() {
  ftdierror(88,ftdi_usb_purge_buffers(ftdi));
  #ifdef SERIAL_VERBOSE
  printf("SERIAL PURGE RX/TX\n");
  #endif
}

void serial_purge_rx() {
  ftdierror(88,ftdi_usb_purge_rx_buffer(ftdi));
  #ifdef SERIAL_VERBOSE
  printf("SERIAL PURGE RX\n");
  #endif
}

void serial_purge_tx() {
  ftdierror(88,ftdi_usb_purge_tx_buffer(ftdi));
  #ifdef SERIAL_VERBOSE
  printf("SERIAL PURGE TX\n");
  #endif
}

int serial_write(byte *str, int len) {
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

inline int serial_read(byte *str, int len) {
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
  if(resp > 0) {
    printf("READ %i of %i bytes: ",resp,len);
    printhexstring(str,resp);
  } else {
    printf("EMPTY\n");
  };
  #endif

  return resp; /* return number of bytes read, or zero */
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


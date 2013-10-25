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
#include "error.h"
#include "config.h"

/****************GLOBALSn'STRUCTURES*****************************/

/* global ftdi context pointer */
struct ftdi_context *ftdi;

/* simple connection state status bit */
byte ftdistatus;

/***************FUNCTION DEFS************************************/

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
  if(ftdistatus > 0) {
    ftdi_usb_close(ftdi);
    ftdi_free(ftdi);
  };
}

int serial_init(char *port) {
  #ifdef SERIAL_VERBOSE
  printf("serial_init opening port @ %s with method ftdi\n",port);
  #endif

  ftdistatus = 0;
  int res = -1;

  /* new ftdi instance */
  if((ftdi = ftdi_new()) == NULL) {
    fatalerror(ERROR_FTDI,"ftdi_new failed");
  };

  if(port != NULL) { /* static device config */
    res = ftdi_usb_open_string(ftdi,port);
    ftdierror(2,res); /* trap error */
  } else { /* autodetect mode */
    struct ftdi_device_list **devlist = malloc(sizeof(
                          struct ftdi_device_list *) * FTDI_AUTO_MAXDEVS);
    int n_devices = ftdi_usb_find_all(ftdi,devlist,0,0);
    if(n_devices > 0) { /* device found */
      /* right now this just grabs the first available ftdi device .. */
      res = ftdi_usb_open_dev(ftdi,devlist[0]->dev);
      ftdierror(2,res);
      ftdi_list_free(devlist);
    } else if(n_devices == 0) { /* nothing found */
      fatalerror(ERROR_FTDI,"no ftdi devices found");
    } else {
      fatalerror(ERROR_FTDI,"detection failed for some reason.");
    };
  };

  if(res < 0) return 0;

  #ifdef SERIAL_VERBOSE
  printf("init ftdi userland driver appears sucessful...\n");
  #endif

  /* set baud rate */
  ftdierror(3,ftdi_set_baudrate(ftdi,FTDI_BAUD));

  /* set latency timer */
  ftdierror(3,ftdi_set_latency_timer(ftdi,2));

  ftdistatus = 1;
  return 1;
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
  #ifdef SERIAL_SUPERVERBOSE
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
  #ifdef SERIAL_SUPERVERBOSE
  if(resp > 0) {
    printf("READ %i of %i bytes: ",resp,len);
    printhexstring(str,resp);
  } else {
    printf("EMPTY\n");
  };
  #endif

  return resp; /* return number of bytes read, or zero */
}

void ftdierror(int loc,int errno) {
  if(errno>=0) return;
  fprintf(stderr,"FTDI DRIVER: %i, %s\n",errno,ftdi_get_error_string(ftdi)); 
  fatalerror(ERROR_FTDI,"*** See above FTDI DRIVER error message @ stderr");
};


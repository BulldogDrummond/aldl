#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "serio.h"
#include "aldl-io.h"
#include "error.h"
#include "config.h"

/****************GLOBALSn'STRUCTURES*****************************/

char *databuff;
char txmode;

/****************FUNCTIONS**************************************/

void serial_close() {
  return;
}

int serial_init(char *port) {
  printf("serial dummy driver active\n");
  txmode=0;
  return 1;
};

void serial_purge() {
  return;
}

void serial_purge_rx() {
  return;
}

void serial_purge_tx() {
  return;
}

int serial_write(byte *str, int len) {
  txmode++;
  return 0;
}

inline int serial_read(byte *str, int len) {
  /* initial idle traffic */
  if(txmode == 0) {
    str[0] = 0x33;
    return 1;
  };
}


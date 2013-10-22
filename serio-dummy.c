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

/****************FUNCTIONS**************************************/

void serial_close() {
  return;
}

int serial_init(char *port) {
  printf("serial dummy driver active\n");

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
  return 0;
}

inline int serial_read(byte *str, int len) {

}


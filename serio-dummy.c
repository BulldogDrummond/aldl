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

unsigned char *databuff;
char txmode;

void gen_pkt();

/****************FUNCTIONS**************************************/

void serial_close() {
  return;
}

void gen_pkt() {
  int x;
  databuff[0]=0xF4;
  databuff[1]=0x92;
  databuff[2]=0x01;
  for(x=3;x<63;x++) databuff[x] = (byte)rand() % 254;
  databuff[63] = checksum_generate(databuff,63);
};

int serial_init(char *port) {
  printf("serial dummy driver active\n");
  txmode=0;
  databuff=malloc(64);
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
  usleep(2000);
  if(txmode == 0) { /* idle traffic req */
    usleep(500000);
    str[0] = 0x33;
    txmode++;
    return 1;
  } if(txmode == 1) { /* shutup req */
    str[0] = 0xF4;
    str[1] = 0x56;
    str[2] = 0x08;
    str[3] = 0xAE;
    txmode++;
    return 4;
  } if(txmode == 2) { /* data request reply */
    txmode = 3; 
    str[0] = 0xF4;
    str[1] = 0x57;
    str[2] = 0x01;
    str[3] = 0x00;
    str[4] = 0xB4;
    return 5;
  } if(txmode == 3) { /* data send */
    txmode = 2;
    gen_pkt();
    int x;
    for(x=0;x<len;x++) {
      str[x] = databuff[x]; 
    };
    return len;
  };
  return 0;
}


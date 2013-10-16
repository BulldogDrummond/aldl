#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "error.h"

char errstr[7][24] = {
"GENERAL",
"NULL",
"OUT OF MEMORY",
"FTDI DRIVER",
"OUT OF RANGE",
"TIMING",
"CONFIG"
};

void fatalerror(error_t code, char *str) {
  fprintf(stderr,"FATAL: %s ERROR (%i)\n",errstr[code],code);
  if(str != NULL) {
    fprintf(stderr,"NOTES: %s\n",str);
  };
  exit(1);
};

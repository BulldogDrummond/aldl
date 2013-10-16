#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "error.h"

void fatalerror(error_t code, char *str) {
  fprintf(stderr,"FATAL ERROR:  CODE %i:\n",code);
  fprintf(stderr,"%s\n",str);
  exit(1);
};

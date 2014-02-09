#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"

void err(char *str, ...) {
  va_list arg;
  if(str != NULL) {
    fprintf(stderr,"ERROR: ");
    va_start(arg,str);
    vfprintf(stderr,str,arg);
    va_end(arg);
    fprintf(stderr,"\n");
  };
  exit(1);
};


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "error.h"

/* list of error code to string, array index matches enumerated values in
   error_t.  these will have ERROR printed AFTER them, so GENERAL will come
   out as GENERAL ERROR. */

char errstr[N_ERRORCODES][24] = {
"GENERAL",
"NULL",
"OUT OF MEMORY",
"FTDI DRIVER",
"OUT OF RANGE",
"TIMING",
"CONFIG",
"BUFFER",
"CONFIG OPTION MISSING"
};

void fatalerror(error_t code, char *str) {
  fprintf(stderr,"FATAL: %s ERROR (%i)\n",errstr[code],code);
  if(str != NULL) {
    fprintf(stderr,"NOTES: %s\n",str);
  };
  exit(1);
};

void nonfatalerror(error_t code, char *str) {
  fprintf(stderr,"ERROR: %s ERROR (%i)\n",errstr[code],code);
  if(str != NULL) {
    fprintf(stderr,"NOTES: %s\n",str);
  };
};


#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "error.h"
#include "aldl-io.h"

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
"CONFIG OPTION MISSING",
"PLUGIN LOADING",
"THREADLOCKING"
};

void fatalerror(error_t code, char *str, ...) {
  va_list arg;
  fprintf(stderr,"FATAL: %s ERROR (%i)\n",errstr[code],code);
  if(str != NULL) {
    fprintf(stderr,"NOTES: ");
    va_start(arg,str);
    vfprintf(stderr,str,arg);
    va_end(arg);
    fprintf(stderr,"\n");
  };
  main_exit();
};

void nonfatalerror(error_t code, char *str, ...) {
  va_list arg;
  fprintf(stderr,"ERROR: %s ERROR (%i)\n",errstr[code],code);
  if(str != NULL) {
    fprintf(stderr,"NOTES: ");
    va_start(arg,str);
    vfprintf(stderr,str,arg);
    va_end(arg);
    fprintf(stderr,"\n");
  };
};


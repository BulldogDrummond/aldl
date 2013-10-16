#ifndef _ERROR_H
#define _ERROR_H

typedef enum _error {
  ERROR_GENERAL,
  ERROR_NULL,
  ERROR_MEMORY,
  ERROR_FTDI,
  ERROR_RANGE,
  ERROR_TIMING
} error_t;

void fatalerror(error_t code, char *str);

#endif

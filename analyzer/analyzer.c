#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>

/* local objects */
#include "../error.h"
#include "../aldl-types.h"
#include "../loadconfig.h"
#include "../useful.h"

/* structures */

typedef struct _an_conf_t {
  int min_temp; /* minimum temperature to start 'trusting' values */
  int min_timestamp; /* minimum timestamp to start trusting values */
} an_conf_t;

int main(int argc, char **argv) {

  return 1;
};


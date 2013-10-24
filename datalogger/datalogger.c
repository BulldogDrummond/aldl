#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

/* local objects */
#include "../aldl-io.h"

void *datalogger_init(void *aldl_in) {
  aldl_conf_t *aldl = (aldl_conf_t *)aldl_in;



  return aldl;
};


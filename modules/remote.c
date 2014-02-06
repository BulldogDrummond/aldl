#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/* local objects */
#include "../error.h"
#include "../aldl-io.h"
#include "../loadconfig.h"
#include "../useful.h"

void *remote_init(void *aldl_in) {
  aldl_conf_t *aldl = aldl_in;
  while(1) {
    sleep(1);
    if(access("/etc/aldl/aldl-stop",F_OK) != -1) {
      exit(1);
    };
  };
  return NULL;  
};


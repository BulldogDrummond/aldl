#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <limits.h>

/* local objects */
#include "loadconfig.h"
#include "config.h"
#include "acquire.h"
#include "error.h"
#include "aldl-io.h"

/* ------- GLOBAL----------------------- */

aldl_conf_t *aldl; /* aldl data structure */
aldl_commdef_t *comm; /* comm specs */

/* ------- LOCAL FUNCTIONS ------------- */

/* run cleanup rountines for aldl and serial crap */
int aldl_finish();

int main() {
  /* initialize locking mechanisms */
  init_locks();

  /* parse config file ... never free this structure */
  loadconfig("lt1.conf");

  set_connstate(ALDL_LOADING,aldl); /* initial connection state */

  /* FIXME this needs to come from load_config or switch to autodetct */
  char *serialport = "d:002/002";
  serial_init(serialport); /* init serial port */

  aldl_acq(aldl); /* start main event loop */

  aldl_finish(comm);

  return 0;
}

int aldl_finish() {
  serial_close();
  return 0;
}


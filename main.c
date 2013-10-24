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
#include "debugif/debugif.h"
#include "consoleif/consoleif.h"
#include "useful.h"

/* ------ local functions ------------- */

/* run cleanup rountines for aldl and serial crap */
int aldl_finish();

int main() { /*-------------------------------------------------- */

  /*------ INITIALIZE ----------------------------------------------*/

  /* initialize locking mechanisms */
  init_locks();

  /* allocate everything and parse config */
  /* FIXME this static config file thing has to go */
  aldl_conf_t *aldl = aldl_setup();

  /* initial record so linked list isnt broken */
  aldl_init_record(aldl);

  /* set initial connection state */
  set_connstate(ALDL_LOADING,aldl);

  /* configure port and initialize */
  serial_init(aldl->serialstr);

  /* ------ THREAD SPAWNING -------------------------------------*/

  /* error checking */
  if(aldl->debugif_enable == 1 && aldl->consoleif_enable == 1) {
    fatalerror(ERROR_PLUGIN,"consoleif and debugif plugins are incompat");
  };

  /* configure threading */
  pthread_t thread_acq; 
  pthread_t thread_debugif;
  pthread_t thread_consoleif;

  /* spawn acq thread */
  pthread_create(&thread_acq,NULL,aldl_acq,(void *)aldl);

  if(aldl->debugif_enable == 1) {
    pthread_create(&thread_debugif,NULL,debugif_loop,(void *)aldl);
  };

  if(aldl->consoleif_enable == 1) {
    pthread_create(&thread_consoleif,NULL,consoleif,(void *) aldl);
  };

  /* wait for acq thread to finish ... */
  pthread_join(thread_acq,NULL);

  /* ----- CLEANUP ---------------------------------------------*/

  aldl_finish();
  return 0;

} /*-----------------------------------------------------------*/

int aldl_finish() {
  serial_close();
  return 0;
}


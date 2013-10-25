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
#include "useful.h"

#include "modules/modules.h"

/* ------ local functions ------------- */

/* run cleanup rountines for aldl and serial crap */
int aldl_finish();

int main(int argc, char **argv) { /*--------------------------- */

  /*------ INITIALIZE ----------------------------------------------*/

  /* initialize locking mechanisms */
  init_locks();

  /* allocate everything and parse config */
  aldl_conf_t *aldl = aldl_setup();

  /* ----- PROC CMDLINE OPTS ---------------------------------------*/

  int n_arg = 0;
  for(n_arg=0;n_arg<argc;n_arg++) {
    if(faststrcmp(argv[n_arg],"configtest") == 1) {
      printf("Loaded config OK.  Exiting...\n");
      exit(0);
    } else if(faststrcmp(argv[n_arg],"debugif") == 1) {
      aldl->debugif_enable = 1;
    } else if(faststrcmp(argv[n_arg],"consoleif") == 1) {
      aldl->consoleif_enable = 1;
    } else if(faststrcmp(argv[n_arg],"datalogger") == 1) {
      aldl->datalogger_enable = 1;
    };
  };

  /* compatibility checking */
  if(aldl->debugif_enable == 1 && aldl->consoleif_enable == 1) {
    fatalerror(ERROR_PLUGIN,"consoleif and debugif plugins are incompat");
  };
  if(aldl->debugif_enable == 0 && aldl->consoleif_enable == 0 &&
     aldl->datalogger_enable == 0) {
    fatalerror(ERROR_PLUGIN,"no plugins are enabled");
  };

  /* ---- MORE INIT -----------------------------------------------*/

  /* initial record so linked list isnt broken */
  aldl_init_record(aldl);

  /* set initial connection state */
  set_connstate(ALDL_LOADING,aldl);

  /* configure port and initialize */
  serial_init(aldl->serialstr);

  /* ------ THREAD SPAWNING -------------------------------------*/

  /* configure threading */
  pthread_t thread_acq; 
  pthread_t thread_debugif;
  pthread_t thread_consoleif;
  pthread_t thread_datalogger;

  /* spawn acq thread */
  pthread_create(&thread_acq,NULL,aldl_acq,(void *)aldl);

  if(aldl->debugif_enable == 1) {
    pthread_create(&thread_debugif,NULL,debugif_init,(void *)aldl);
  };

  if(aldl->consoleif_enable == 1) {
    pthread_create(&thread_consoleif,NULL,consoleif_init,(void *) aldl);
  };

  if(aldl->datalogger_enable == 1) {
    pthread_create(&thread_datalogger,NULL,datalogger_init,(void *) aldl);
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


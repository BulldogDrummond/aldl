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
#include "serio.h"

#include "modules/modules.h"

/* ------ local functions ------------- */

/* run some post-config loading sanity checks */
void aldl_sanity_check(aldl_conf_t *aldl);

/* run cleanup rountines for aldl and serial crap */
int aldl_finish();

int main(int argc, char **argv) { /*--------------------------- */

  /*------ INITIALIZE ----------------------------------------------*/

  /* initialize locking mechanisms */
  init_locks();

  /* allocate everything and parse config */
  aldl_conf_t *aldl = aldl_setup();
  aldl_sanity_check(aldl); /* sanity checks */

  /* ----- PROC CMDLINE OPTS ---------------------------------------*/

  int n_arg = 0;
  for(n_arg=1;n_arg<argc;n_arg++) {
    if(faststrcmp(argv[n_arg],"configtest") == 1) {
      printf("Loaded config OK.  Exiting...\n");
      exit(0);
    } else if(faststrcmp(argv[n_arg],"devices") == 1) {
      serial_help_devs();
      exit(0);
    } else if(faststrcmp(argv[n_arg],"consoleif") == 1) {
      aldl->consoleif_enable = 1;
    } else if(faststrcmp(argv[n_arg],"datalogger") == 1) {
      aldl->datalogger_enable = 1;
    } else {
      fatalerror(ERROR_NULL,"Option %s not recognized",argv[n_arg]);
    };
  };

  /* compatibility checking */
  if(aldl->consoleif_enable == 0 &&
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
  pthread_t thread_consoleif;
  pthread_t thread_datalogger;

  /* spawn acq thread */
  pthread_create(&thread_acq,NULL,aldl_acq,(void *)aldl);

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

void main_exit() {
  consoleif_exit();
  serial_close();
  aldl_finish();
}

int aldl_finish() {
  exit(1);
  return 0;
}

void aldl_sanity_check(aldl_conf_t *aldl) {
  int x;
  aldl_define_t *def;
  aldl_packetdef_t *pkt;
  int id = 0;

  /* find related pkt number */
  for(x=0;x<aldl->n_defs;x++) {
    def = &aldl->def[x];
    pkt = NULL;
    for(id=0; id < aldl->comm->n_packets; id++) {
      if(aldl->comm->packet[id].id == def->packet) {
        pkt = &aldl->comm->packet[id]; 
        break;
      };
    };
    if(pkt == NULL) fatalerror(ERROR_RANGE,"invalid packet specified");

    /* check range */
    if(pkt->offset + def->offset > pkt->length) {
      fatalerror(ERROR_RANGE,"definition out of packet range");
    };
  };
};

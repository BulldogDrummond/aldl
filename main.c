#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

/* local objects */
#include "config.h"
#include "aldl-io/aldl-io.h"
#include "configfile/varstore.h"
#include "configfile/configfile.h"

/* ------- GLOBAL----------------------- */

aldl_conf_t *aldl; /* aldl data structure */
aldl_commdef_t *comm; /* communication config structure */

/* ------- LOCAL FUNCTIONS ------------- */

/* run main aldl aquisition event loop */
int aldl_acq();

/* run cleanup rountines for aldl and serial crap */
int aldl_finish();

/* load all config data */
int load_config(char *filename);

#ifdef PREPRODUCTION
/* debug-only fallback config */
void fallback_config();

/* placeholder error handler */
void tmperror(char *str);
#endif

int main() {
  #ifdef VERBLOSITY
  printf("verblosity routine enabled in main\n");
  printf("base aldl structure: %i bytes\n",(int)sizeof(aldl_conf_t));
  printf("base comm structure: %i bytes\n",(int)sizeof(aldl_commdef_t));
  #endif

  /* ------- INITIALIZE ------------------------------*/
  char *port = "d:001/004"; /* FIXME need to get from config */
  serial_init(port);

  /* allocation ------*/
  aldl = malloc(sizeof(aldl_conf_t));
  comm = malloc(sizeof(aldl_commdef_t));
  if(aldl == NULL || comm == NULL) tmperror("out of memory 1055"); /* FIXME */

  /* configuration ---*/
  //load_config("/project/lt1.conf");
  fallback_config();

  /* ------- EVENT LOOP STUFF ------------------------*/

  /* start main event loop -- this should be threaded out */
  aldl_acq();

  /* program dies here ... */
  aldl_finish(comm);
  return 0;
}

int load_config(char *filename) {

  return 0;
}

int aldl_acq() {
  aldl_reconnect(comm); /* this shouldn't return without a connection .. */
  printf("connection successful, bailing !\n");
  exit(1);
  /* PERFORM ACQ ROUTINE HERE */
  return 0;
}

int aldl_finish() {
  serial_close();
  return 0;
}

void tmperror(char *str) {
  printf("FATAL ERROR: %s",str);
  exit(1);
}

#ifdef PREPRODUCTION
void fallback_config() {
  sprintf(comm->ecmstring, "EE");
  comm->checksum_enable = 1;
  comm->pcm_address = 0xF4;
  comm->idletraffic = "00";
  comm->idledelay = 10;
  comm->chatterwait = 1;
  comm->shutupcommand = "\xF4\x56\x08\xAE";
  comm->shutuplength = 4;
  comm->shutuptime = 3000;
  comm->shutupfailwait = 500;
  comm->shutupcharlimit = 20;
  comm->shutuprepeat = 3;
  comm->shutuprepeatdelay = 75;
  comm->n_packets = 0;
  comm->packet = NULL;
}
#endif


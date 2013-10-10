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
aldl_commdef_t *comm; /* comm specs */
char *config_file; /* path to config file */

/* ------- LOCAL FUNCTIONS ------------- */

/* run main aldl aquisition event loop */
int aldl_acq();

/* run cleanup rountines for aldl and serial crap */
int aldl_finish();

/* allocate all base data structures */
int aldl_alloc();

/* load all base config data, but no packet definitions. */
int load_config_a(char *filename);

/* load all packet definitions, and allocate as needed */
int load_config_b(char *filename);

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

  /* ------- SETUP AND LOAD CONFIG -------------------*/

  aldl_alloc(); /* perform initial allocations */

  load_config_a("/project/lt1.conf"); /* load 1st stage config */

  fallback_config(); /* REMOVE ME */

  load_config_b("/project/lt1.conf"); /* allocate and load stage b conf */

  comm->serialport = "d:001/004"; /* REMOVE ME needs to come from load_config */

  serial_init(comm->serialport); /* init serial port */

  /* ------- EVENT LOOP STUFF ------------------------*/

  aldl_acq(); /* start main event loop */

  /* ------- CLEANUP ----------------------------------*/

  aldl_finish(comm);

  return 0;
}

int aldl_alloc() {
#ifdef VERBLOSITY
  printf("performing initial allocation\n");
#endif
  aldl = malloc(sizeof(aldl_conf_t));
  if(aldl == NULL) tmperror("out of memory 1055"); /* FIXME */
  comm = malloc(sizeof(aldl_commdef_t));
  if(comm == NULL) tmperror("out of memory 1055"); /* FIXME */
  aldl->comm = comm; /* link */
  return 0;
}

int load_config_a(char *filename) {
#ifdef VERBLOSITY
  printf("load stage a config\n");
#endif

  return 0;
}

int load_config_b(char *filename) {
#ifdef VERBLOSITY
  printf("load stage b config\n");
#endif
  /* FIXME this mallocs a bunch of shit without checking ret val */
  /* allocate space to store packet definitions */
  comm->packet = malloc(sizeof(aldl_packetdef_t) * comm->n_packets);
  /* !! get packet definitions here, or this flunks */
  int x = 0;
  for(x=0;x<comm->n_packets;x++) {
    comm->packet[x].data = malloc(comm->packet[x].length);
  };
  aldl->def = malloc(sizeof(aldl_define_t) * aldl->n);
  /* get data definitions here !! */

  /* allocate space for records */
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


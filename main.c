#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>

/* local objects */
#include "config.h"
#include "acquire.h"
#include "error.h"
#include "aldl-io/config.h"
#include "aldl-io/aldl-io.h"
#include "configfile/varstore.h"
#include "configfile/configfile.h"

/* ------- GLOBAL----------------------- */

aldl_conf_t *aldl; /* aldl data structure */
aldl_commdef_t *comm; /* comm specs */
char *config_file; /* path to config file */

/* ------- LOCAL FUNCTIONS ------------- */

/* run cleanup rountines for aldl and serial crap */
int aldl_finish();

/* allocate all base data structures */
int aldl_alloc();

/* load all base config data, but no packet definitions. */
int load_config_a(char *filename);

/* load all packet definitions, and allocate as needed */
int load_config_b(char *filename);

int main() {
  /* ------- SETUP AND LOAD CONFIG -------------------*/

  aldl_alloc(); /* perform initial allocations */

  aldl->state = ALDL_CONNECTING; /* initial connection state */

  load_config_a("/project/lt1.conf"); /* load 1st stage config */

  load_config_b("/project/lt1.conf"); /* allocate and load stage b conf */

  /* FIXME this needs to come from load_config or switch to autodetct */
  char *serialport = "d:002/002";

  serial_init(serialport); /* init serial port */

  /* ------- EVENT LOOP STUFF ------------------------*/

  aldl_acq(aldl); /* start main event loop */

  /* ------- CLEANUP ----------------------------------*/

  aldl_finish(comm);

  return 0;
}

int aldl_alloc() {
  aldl = malloc(sizeof(aldl_conf_t));
  if(aldl == NULL) fatalerror(ERROR_MEMORY,"conf_t alloc"); /* FIXME */
  memset(aldl,0,sizeof(aldl_conf_t));
  comm = malloc(sizeof(aldl_commdef_t));
  if(comm == NULL) fatalerror(ERROR_MEMORY,"commdef alloc"); /* FIXME */
  memset(comm,0,sizeof(aldl_commdef_t));
  aldl->comm = comm; /* link */
  aldl->stats = malloc(sizeof(aldl_stats_t));
  if(aldl->stats == NULL) fatalerror(ERROR_MEMORY,"stats alloc");
  memset(aldl->stats,0,sizeof(aldl_stats_t));
  return 0;
}

int load_config_a(char *filename) {
  sprintf(comm->ecmstring, "EE");
  comm->checksum_enable = 1;
  comm->pcm_address = 0xF4;
  comm->idledelay = 10;
  comm->chatterwait = 1;
  comm->shutupcommand = generate_shutup(0x56,0x08,comm);
  comm->returncommand = generate_shutup(0x56,0x09,comm);
  comm->shutuprepeat = 3;
  comm->shutuprepeatdelay = 75;
  comm->n_packets = 3;
  return 1;
}

int load_config_b(char *filename) {
  /* allocate space to store packet definitions */
  comm->packet = malloc(sizeof(aldl_packetdef_t) * comm->n_packets);
  if(comm->packet == NULL) fatalerror(ERROR_MEMORY,"packet mem");

  /* !! get packet definitions here, or this flunks due to missing length */

  /* a placeholder packet, lt1 msg 0 */
  comm->packet[0].length = 64;
  comm->packet[0].enable = 0;
  comm->packet[0].id = 0x00;
  comm->packet[0].msg_len = 0x57;
  comm->packet[0].msg_mode = 0x01;
  comm->packet[0].commandlength = 5;
  comm->packet[0].offset = 3;
  comm->packet[0].retry = 1;
  generate_pktcommand(&comm->packet[0],comm);
  
  /* a placeholder packet, lt1 msg 2 */
  comm->packet[1].length = 57;
  comm->packet[1].enable = 0;
  comm->packet[1].id = 0x02;
  comm->packet[1].msg_len = 0x57;
  comm->packet[1].msg_mode = 0x01;
  comm->packet[1].commandlength = 5;
  comm->packet[1].offset = 3;
  comm->packet[1].retry = 1;
  generate_pktcommand(&comm->packet[1],comm);

  /* a placeholder packet, lt1 msg 4 */
  comm->packet[2].length = 49;
  comm->packet[2].enable = 0;
  comm->packet[2].id = 0x04;
  comm->packet[2].msg_len = 0x57;
  comm->packet[2].msg_mode = 0x01;
  comm->packet[2].commandlength = 5;
  comm->packet[2].offset = 3;
  comm->packet[2].retry = 1;
  generate_pktcommand(&comm->packet[2],comm);

  int x = 0;
  for(x=0;x<comm->n_packets;x++) { /* allocate data storage */
    comm->packet[x].data = malloc(comm->packet[x].length);
    if(comm->packet[x].data == NULL) fatalerror(ERROR_MEMORY,"pkt data");
  };

  aldl->def = malloc(sizeof(aldl_define_t) * aldl->n);
  if(aldl->def == NULL) fatalerror(ERROR_MEMORY,"definition");
  /* get data definitions here !! */

  /* allocate space for records */
  return 0;
}

int aldl_finish() {
  serial_close();
  return 0;
}


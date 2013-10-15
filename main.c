#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

/* local objects */
#include "config.h"
#include "aldl-io/config.h"
#include "aldl-io/aldl-io.h"
#include "configfile/varstore.h"
#include "configfile/configfile.h"

/* plugins */
#include "debugif/debugif.h"

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
  /* ------- SETUP AND LOAD CONFIG -------------------*/

  aldl_alloc(); /* perform initial allocations */

  aldl->state = ALDL_CONNECTING; /* initial connection state */

  load_config_a("/project/lt1.conf"); /* load 1st stage config */

  fallback_config(); /* REMOVE ME */

  load_config_b("/project/lt1.conf"); /* allocate and load stage b conf */

  /* FIXME this needs to come from load_config or switch to autodetct */
  char *serialport = "d:002/002";

  serial_init(serialport); /* init serial port */

  /* ------- EVENT LOOP STUFF ------------------------*/

  aldl_acq(); /* start main event loop */

  /* ------- CLEANUP ----------------------------------*/

  aldl_finish(comm);

  return 0;
}

int aldl_alloc() {
  aldl = malloc(sizeof(aldl_conf_t));
  if(aldl == NULL) tmperror("out of memory 1055"); /* FIXME */
  memset(aldl,0,sizeof(aldl_conf_t));
  comm = malloc(sizeof(aldl_commdef_t));
  if(comm == NULL) tmperror("out of memory 1055"); /* FIXME */
  memset(comm,0,sizeof(aldl_commdef_t));
  aldl->comm = comm; /* link */
  aldl->stats = malloc(sizeof(aldl_stats_t));
  if(aldl->stats == NULL) tmperror("out of memory 1056");
  memset(aldl->stats,0,sizeof(aldl_stats_t));
  return 0;
}

int load_config_a(char *filename) {
  return 0;
}

int load_config_b(char *filename) {
  /* allocate space to store packet definitions */
  comm->packet = malloc(sizeof(aldl_packetdef_t) * comm->n_packets);
  if(comm->packet == NULL) tmperror("out of memory 1055"); /* FIXME */

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
    if(comm->packet[x].data == NULL) tmperror("out of memory 1055"); /* FIXME */
  };

  aldl->def = malloc(sizeof(aldl_define_t) * aldl->n);
  if(aldl->def == NULL) tmperror("out of memory 1055"); /* FIXME */
  /* get data definitions here !! */

  /* allocate space for records */
  return 0;
}

int aldl_acq() {
  int npkt = 0;
  aldl_packetdef_t *pkt = NULL;
  ALDL_RECON:
  aldl_reconnect(comm); /* this shouldn't return without a connection .. */
  aldl->state = ALDL_CONNECTED;
  #ifdef VERBLOSITY
  printf("----- CONNECTION SUCCESSFUL -----\n");
  #endif
  while(1) {
    for(npkt=0;npkt < comm->n_packets;npkt++) {
      if(aldl->stats->failcounter > MAX_FAIL_DISCONNECT) {
        aldl->state = ALDL_DESYNC;
        goto ALDL_RECON; 
      };
      #ifdef VERBLOSITY
      printf("acquiring packet %i\n",npkt);
      #endif
      /* FIXME need some retry logic here, for both failed checksums and
         dropped packets, that tie into connection state so we can tell when
         sync is lost, and try to reconnect */
      pkt = &comm->packet[npkt];
      if(aldl_get_packet(pkt) == NULL) { /* packet timeout or fail */
        pkt->enable = 0;
        aldl->stats->packetrecvtimeout++;
        aldl->stats->failcounter++;
        #ifdef VERBLOSITY
        printf("packet %i failed due to timeout...\n",npkt);
        #endif
        if(pkt->retry == 1) npkt--;
        continue;
      };
      if(checksum_test(pkt->data, pkt->length) == 0) { /* fail chksum */
        aldl->stats->packetchecksumfail++;
        aldl->stats->failcounter++;
        pkt->enable = 0;
        #ifdef VERBLOSITY
        printf("checksum failed @ pkt %i...\n",npkt);
        #endif
        if(pkt->retry == 1) npkt--;
        continue;
      };
      pkt->enable = 1; /* data is good, and can be used */
      aldl->stats->failcounter = 0; /* reset failcounter */
    };
    /* process packets here */
    debugif_iterate(aldl);
  };
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
  comm->idledelay = 10;
  comm->chatterwait = 1;
  comm->shutupcommand = generate_shutup(0x56,0x08,comm);
  comm->shutuprepeat = 3;
  comm->shutuprepeatdelay = 75;
  comm->n_packets = 3;
}
#endif


#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

/* local objects */
#include "../aldl-io/aldl-io.h"

void debugif_loop(aldl_conf_t *c) {
  printf("-------DEBUG DISPLAY INTERFACE ACTIVE--------\n"); 
};

void debugif_iterate(aldl_conf_t *c) {
  /* this displays raw data for now. this isnt what we want. */
  int x;
  aldl_commdef_t *comm = c->comm;
  aldl_packetdef_t *pkt = NULL;
  printf("STATS: %f pkt/sec, %i timeouts, %i bad header, %i checksum fail\n",
            c->stats->packetspersecond, c->stats->packetrecvtimeout,
            c->stats->packetheaderfail, c->stats->packetchecksumfail);
  for(x=0;x<comm->n_packets;x++) {
    pkt = &comm->packet[x];
    if(pkt->clean == 1) {
      printf("debugif.c PKT id=%i length=%i: ",pkt->id,pkt->length);
      printhexstring(pkt->data,pkt->length);
      /* print raw data here */
    } else {
      printf("skipping packet %i, not clean\n",x);
    };
  };
};


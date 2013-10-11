#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

/* local objects */
#include "../aldl-io/aldl-io.h"

void debugif_loop(aldl_conf_t *c) {
  printf("-------DEBUG DISPLAY INTERFACE ACTIVE--------\n"); 
  printhexstring("this is a test string",12);
};

void debugif_iterate(aldl_conf_t *c) {
  /* this displays raw data for now. this isnt what we want. */
  int x;
  aldl_commdef_t *comm = c->comm;
  aldl_packetdef_t *pkt = NULL;
  for(x=0;x<comm->n_packets;x++) {
    pkt = &comm->packet[x];
    if(pkt->enable == 1) {
      printf("packet id=%i length=%i -----------\n",pkt->id,pkt->length);
      printhexstring(pkt->data,pkt->length);
      /* print raw data here */
    } else {
      printf("skipping packet %i, enable bit not set\n",x);
    };
  };
};


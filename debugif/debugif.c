#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

/* local objects */
#include "../aldl-io.h"

void *debugif_loop(void *aldl_in) {
  aldl_conf_t *aldl = (aldl_conf_t *)aldl_in;
  printf("-------DEBUG DISPLAY INTERFACE ACTIVE--------\n"); 
  int x;
  pause_until_connected(aldl);
  printf("got connected state\n");
  pause_until_buffered(aldl);
  printf("got buffered state\n");
  
  aldl_record_t *rec = newest_record(aldl); /* ptr to the most current record */
  while(1) {
    rec = next_record_wait(rec);
    printf("--- record @ time %u ---\n",(unsigned int)rec->t);
    for(x=0;x<aldl->n_defs;x++) {
      printf("%s: ",aldl->def[x].name);
      switch(aldl->def[x].type) {
        case ALDL_FLOAT:
          printf("%f ",rec->data[x].f); 
          break;
        case ALDL_INT:
        case ALDL_BOOL:
          printf("%i ",rec->data[x].i);
          break;
        default:
          printf("WTF ");
      }
      printf("\n");
    };
  };
  return aldl;
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
    printf("debugif.c PKT id=%i length=%i: ",pkt->id,pkt->length);
    printhexstring(pkt->data,pkt->length);
  };
};


#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

/* local objects */
#include "../aldl-io.h"

void *debugif_loop(void *aldl_in) {
  aldl_conf_t *aldl = (aldl_conf_t *)aldl_in;

  printf("-------DEBUG DISPLAY INTERFACE ACTIVE--------\n"); 

  /* get the index and store it, to avoid repeated lookups */
  int rpmindex = get_index_by_id(aldl,0);
  aldl_record_t *rec = newest_record(aldl); /* ptr to the most current record */

  pause_until_connected(aldl);

  while(1) {
    /* pause until new data is available, then retrieve it. */
    rec = next_record_wait(rec);

    /* in that record, get data field rpmindex, and the floating point value
       contained within ... also get the short name from the definition. */
    printf("%s: %f\n",aldl->def[rpmindex].name, rec->data[rpmindex].f);
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


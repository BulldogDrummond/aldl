#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>

/* local objects */
#include "config.h"
#include "aldl-io/config.h"
#include "aldl-io/aldl-io.h"
#include "acquire.h"

/* plugins */
#include "debugif/debugif.h"

/* ------- LOCAL FUNCTIONS ------------- */

int aldl_acq(aldl_conf_t *aldl) {
  aldl_commdef_t *comm = aldl->comm;
  #ifdef TRACK_PKTRATE
  time_t timestamp = time(NULL);
  int pktcounter = 0; /* how many packets between timestamps */
  #endif
  int pktfail = 0; /* marker for a failed packet in event loop */
  int npkt = 0; /* array index of packet to operate on */
  aldl_packetdef_t *pkt = NULL; /* temporary pointer to the packet def */
  aldl->state = ALDL_CONNECTING; /* initial disconnected state */
  /* this should be fine as an infinite loop ... */
  while(1) {
    for(npkt=0;npkt < comm->n_packets;npkt++) { /* iterate through all pkts */
      if(aldl->state != ALDL_CONNECTED) {
        aldl_reconnect(comm); /* main connection happens here */
        aldl->state = ALDL_CONNECTED;
        #ifdef VERBLOSITY
        printf("----- RECONNECTED ----------------\n");
        #endif
      };
      #ifdef TRACK_PKTRATE
      if(time(NULL) - timestamp >= 5) {
        aldl->stats->packetspersecond = pktcounter / 5;
        timestamp = time(NULL);
        pktcounter = 0;
      };
      #endif
      #ifdef VERBLOSITY
      printf("ACQUIRE pkt# %i @ rate %f/sec\n",npkt,
                    aldl->stats->packetspersecond);
      #endif
      pkt = &comm->packet[npkt];
      pktfail = 0; /* assume no failure ... */
      if(aldl_get_packet(pkt) == NULL) { /* packet timeout or fail */
        aldl->stats->packetrecvtimeout++;
        pktfail = 1;
        #ifdef VERBLOSITY
        printf("packet %i failed due to timeout...\n",npkt);
        #endif
      };
      if(pkt->data[0] != comm->pcm_address) { /* fail header */
        pktfail = 1;
        aldl->stats->packetheaderfail++;
        #ifdef VERBLOSITY
        printf("header failed @ pkt %i...\n",npkt);
        #endif
      };
      if(checksum_test(pkt->data, pkt->length) == 0) { /* fail chksum */
        pktfail = 1;
        aldl->stats->packetchecksumfail++;
        #ifdef VERBLOSITY
        printf("checksum failed @ pkt %i...\n",npkt);
        #endif
      };
      if(pktfail == 1) {
        aldl->stats->failcounter++;
        #ifdef VERBLOSITY
        printf("packet fail counter: %i\n",aldl->stats->failcounter);
        #endif
        pkt->enable = 0;
        if(pkt->retry == 1) npkt--;
        /* if we aren't over our fail limit, just go on as normal .. */
        if(aldl->stats->failcounter > MAX_FAIL_DISCONNECT) {
          aldl->state = ALDL_DESYNC;
        };
      } else {
        pkt->enable = 1; /* data is good, and can be used */
        #ifdef TRACK_PKTRATE
        pktcounter++; /* increment packet counter */
        #endif
        aldl->stats->failcounter = 0; /* reset failcounter */
      };
    };
    /* process packets here */
    debugif_iterate(aldl);
  };
  return 0;
}


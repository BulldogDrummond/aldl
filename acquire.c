#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/* local objects */
#include "error.h"
#include "config.h"
#include "aldl-io/config.h"
#include "aldl-io/aldl-io.h"
#include "acquire.h"

/* plugins */
#include "debugif/debugif.h"

/* primary data acqusition event loop */

int aldl_acq(aldl_conf_t *aldl) {
  /* ---- main variables --------------- */
  aldl_commdef_t *comm = aldl->comm; /* direct reference to commdef */
  aldl_packetdef_t *pkt = NULL; /* temporary pointer to the packet def */
  int pktfail = 0; /* marker for a failed packet in event loop */
  int npkt = 0; /* array index of packet to operate on */

  /* sanity checks */
  if(aldl->rate * 1000 > 2000) fatalerror(ERROR_TIMING,"acq delay too high");
  if(comm->n_packets < 1) fatalerror(ERROR_RANGE,"no packets in acq");

  #ifdef ALDL_MULTIPACKET
  /* prepare array for packet retrieval frequency tracking */
  int *freq_counter = malloc(sizeof(int) * comm->n_packets);
  int freq_init;
  for(freq_init=0;freq_init < comm->n_packets; freq_init++) {
    /* if we init the frequency with freq max, that will ensure that each
       packet is iterated once at the beginning of the acq. routine */
    freq_counter[freq_init] = comm->packet[freq_init].frequency;
  };
  #endif

  /* config vars and get initial stamp if packet rate tracking is enabled */
  #ifdef TRACK_PKTRATE
  time_t timestamp = time(NULL);
  int pktcounter = 0; /* how many packets between timestamps */
  #endif

  /* intial connection state */
  aldl->state = ALDL_CONNECTING;

  /* if the connection state gets set to quit, end the infinite loop.  not
     sure where a good place to set this would be anyway ... */

  /* ---- event loop header -----  until ALDL_QUIT connection state is set,
     iterate through all packets in sequence. */
  while(aldl->state != ALDL_QUIT) for(npkt=0;npkt < comm->n_packets;npkt++) {

    /* detect a necessary retry, and step back iterator */
    if(pktfail == 1) {
      if(npkt == 0) {
        npkt = comm->n_packets - 1;
      } else {
        npkt--;
      };
      pktfail = 0; /* reset fail marker */
      /* note that *pkt will have persisted from last iteration */
    } else {  /* retry skips the frequency selector */
    
    #ifdef ALDL_MULTIPACKET
    /* ---- frequency select routine ---- */
      /* skip packet if frequency is 0 to match spec */
      if(comm->packet[npkt].frequency == 0) continue;
      if(freq_counter[npkt] < comm->packet[npkt].frequency) {
        /* frequency requirement not met */
        freq_counter[npkt]++;
        continue; /* go to next pkt */
      } else {
        /* reached frequency, reset to 1 */
        freq_counter[npkt] = 1;
      };
    #endif
    };

    pkt = &comm->packet[npkt]; /* pointer to the correct packet */

    /* this would seem an appropriate time to maintain the connection if it
       drops, or if it never existed ... if not, time for a delay */
    if(aldl->state != ALDL_CONNECTED) {
      aldl_reconnect(comm); /* main connection happens here */
      aldl->state = ALDL_CONNECTED;
    } else {
      /* delay between data collection iterations */
      usleep(aldl->rate);
    };

    /* check if we're @ 5 seconds, and average the number of packets for
       statistical purposes */
    #ifdef TRACK_PKTRATE
    if(time(NULL) - timestamp >= PKTRATE_DURATION) {
      aldl->stats->packetspersecond = pktcounter / PKTRATE_DURATION;
      timestamp = time(NULL);
      pktcounter = 0;
    };
    #endif

    /* print debugging info */
    #ifdef VERBLOSITY
    printf("ACQUIRE pkt# %i\n",npkt);
    #endif

    /* send request and get packet data (from aldlcomm.c); if NULL is
       returned, it's because it timed out waiting for data. */
    if(aldl_get_packet(pkt) == NULL) {
      aldl->stats->packetrecvtimeout++;
      pktfail = 1;
      #ifdef VERBLOSITY
      printf("packet %i failed due to timeout...\n",npkt);
      #endif
    };

    /* optional check for pcm address bit in the header, to see if we're
       even in the ballpark of a legit packet.  this may avoid an expensive
       checksumming run if the packet is total garbage. */
    #ifdef CHECK_HEADER_SANITY
    if(pkt->data[0] != comm->pcm_address) { /* fail header */
      pktfail = 1;
      aldl->stats->packetheaderfail++;
      #ifdef VERBLOSITY
      printf("header failed @ pkt %i...\n",npkt);
      #endif
    };
    #endif

    /* verify checksum if that option is enabled in the commdef. */
    if(comm->checksum_enable == 1 &&
       checksum_test(pkt->data, pkt->length) == 0) {
      pktfail = 1;
      aldl->stats->packetchecksumfail++;
      #ifdef VERBLOSITY
      printf("checksum failed @ pkt %i...\n",npkt);
      #endif
    };

    /* handle condition of a bad packet */
    if(pktfail == 1) {
      aldl->stats->failcounter++; /* increment failed pkt counter */
      #ifdef VERBLOSITY
      printf("packet fail counter: %i\n",aldl->stats->failcounter);
      #endif
      pkt->clean = 0; /* mark packet unclean temporarily */

      /* --- set a desync state if we're getting lots of fails in a row */
      if(aldl->stats->failcounter > MAX_FAIL_DISCONNECT) {
        aldl->state = ALDL_DESYNC;
      };

      /* if a retry is not required, do not persist the fail bit.. */
      if(pkt->retry != 1) pktfail = 0;

    /* packet is good to go */
    } else {
      pkt->clean = 1; /* data is good, and can be used */
      #ifdef TRACK_PKTRATE
      pktcounter++; /* increment packet counter */
      #endif
      aldl->stats->failcounter = 0; /* reset failcounter */
    };
    /* process packets here maybe ? */
    debugif_iterate(aldl);
  };
  return 0;
}



/* enable debug level verbosity in the main program.  probably disable this if
   you're using a display plugin ... */

#define VERBLOSITY

/* build in preproduction code */

#define PREPRODUCTION

/* the maximum failed packets before the connection is marked failed */
#define MAX_FAIL_DISCONNECT 6

/* track packet retrieval rate */
#define TRACK_PKTRATE

/* number of seconds to average retrieval rate.  reccommend at least 5. */
#define PKTRATE_DURATION 5

/* extra check for bad message header.  checksum should be sufficient.. */
#define CHECK_HEADER_SANITY

/* support multiple packets */
#define ALDL_MULTIPACKET

/* track connection state for shutup lag */
#define LAGCHECK

/* seconds of acceptable lag time */
#define LAGTIME 2

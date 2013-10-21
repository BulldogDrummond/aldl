
/* enable debug level verbosity in the main program.  probably disable this if
   you're using a display plugin ... */

//#define VERBLOSITY

/* debug structures and buffering */
//#define DEBUGSTRUCT

/* the maximum failed packets before the connection is marked failed */
#define MAX_FAIL_DISCONNECT 6

/* track packet retrieval rate */
#define TRACK_PKTRATE

/* number of seconds to average retrieval rate.  reccommend at least 5. */
#define PKTRATE_DURATION 5

/* extra check for bad message header.  checksum should be sufficient.. */
#define CHECK_HEADER_SANITY

/* support multiple packets */
//#define ALDL_MULTIPACKET

/* track connection state for shutup lag */
#define LAGCHECK

/* seconds of acceptable lag time */
#define LAGTIME 2

/* spits a bunch of random shit out on stdout.  probably disable when using
   an actual display interface */
//#define SERIAL_VERBOSE
//#define SERIAL_SUPERVERBOSE

/* verbose aldl comm routines on stdout */
//#define ALDL_VERBOSE

/* a static delay in milliseconds.  used for waiting in between grabbing
   serial chunks, and for delay timers.  decreasing this value makes timeouts
   more accurate.  increasing improves cpu usage. */
#define SLEEPYTIME 2

/* the baud rate to set for the ftdi usb userland driver.  reccommend 8192. */
#define FTDI_BAUD 8192

/* a constant theoretical amount of bytes per millisecond that can be
   moved at the baud rate; generally 1 / baud * 1000 */
#define SERIAL_BYTES_PER_MS 0.98

/* a theoretical maximum multiplier per byte that the ECM may take to generate
   data under any circumstance ... */
#define ECMLAGTIME 0.35

/* length of an aldl mode change request string */
/* FIXME this might not be good for other ECMS */
#define SHUTUP_LENGTH 4

/* the maximum number of ftdi devices attached to a system, it'll puke if more
   than this number of devices is found ... */
#define FTDI_AUTO_MAXDEVS 100


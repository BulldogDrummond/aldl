#define VERSION "ALDL-IO v0.3"
/* ----------- FILE CONFIG ----------------------------*/

/* path to the root config file */
#define ROOT_CONFIG_FILE "./main.conf"

/* ----------- DEBUG OUTPUT --------------------------*/

/* generally verbose behavior */
//#define VERBLOSITY

/* debug structural functions, such as record link list management */
//#define DEBUGSTRUCT

/* debug configuration file loading */
//#define DEBUGCONFIG

/* print debugging info for memory */
//#define DEBUGMEM

/* verbosity levels in raw serial handlers */
//#define SERIAL_VERBOSE
//#define SERIAL_SUPERVERBOSE

/* verbose aldl protocol level comms on stdout */
//#define ALDL_VERBOSE

/* --------- GLOBAL FEATURE CONFIG -----------------*/

/* support multiple packets, required by some definition files */
//#define ALDL_MULTIPACKET

/* --------- TIMING CONSTANTS ------------------------*/

/* define for more aggressive timing behavior in an attempt to increase packet
   rate, at a higher risk of dropped packets and increased cpu usage */
//#define AGGRESSIVE

/* a static delay in microseconds.  used for waiting in between grabbing
   serial chunks, and other throttling.  if AGGRESSIVE is defined, this is
   generally ignored ... */
#define SLEEPYTIME 200

/* a theoretical maximum multiplier per byte that the ECM may take to generate
   data under any circumstance ... */
#define ECMLAGTIME 0.35

/* a constant theoretical amount of bytes per millisecond that can be
   moved at the baud rate; generally 1 / baud * 1000 */
#define SERIAL_BYTES_PER_MS 0.98

/* --------- DATA ACQ. CONFIG ----------------------*/

/* track packet retrieval rate */
#define TRACK_PKTRATE

/* number of seconds to average retrieval rate.  reccommend at least 5. */
#define PKTRATE_DURATION 5

/* extra check for bad message header.  checksum should be sufficient.. */
#define CHECK_HEADER_SANITY

/* track connection state for expiry of disable comms mode */
#define LAGCHECK

/* attempt to handle cases where runtime may exceed ULONG_MAX by wrapping back
   to zero.  this is at least 49 days of runtime or more depending on host
   system; so the check should not be necessary ... and results are undefined */
#define TIMESTAMP_WRAPAROUND

/* ------- FTDI DRIVER CONFIG ------------------------*/

/* the baud rate to set for the ftdi usb userland driver.  reccommend 8192. */
#define FTDI_BAUD 8192

/* the maximum number of ftdi devices attached to a system, it'll puke if more
   than this number of devices is found ... */
#define FTDI_AUTO_MAXDEVS 100


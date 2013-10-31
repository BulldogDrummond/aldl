#define VERSION "ALDL-IO v0.4"
/* ----------- FILE CONFIG ----------------------------*/

/* path to the root config file */
#define ROOT_CONFIG_FILE "./examples/aldl.conf"

/* ----------- DEBUG OUTPUT --------------------------*/

/* generally verbose behavior */
#undef VERBLOSITY

/* debug structural functions, such as record link list management */
#undef DEBUGSTRUCT

/* debug configuration file loading */
#undef DEBUGCONFIG

/* print debugging info for memory */
#undef DEBUGMEM

/* verbosity levels in raw serial handlers */
#undef SERIAL_VERBOSE
#undef SERIAL_SUPERVERBOSE

/* verbose aldl protocol level comms on stdout */
#undef ALDL_VERBOSE

/* --------- GLOBAL FEATURE CONFIG -----------------*/

/* support multiple packets, required by some definition files */
#undef ALDL_MULTIPACKET

/* --------- TIMING CONSTANTS ------------------------*/

/* define for more aggressive timing behavior in an attempt to increase packet
   rate, at a higher risk of dropped packets and increased cpu usage */
#undef AGGRESSIVE

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

/* defining this provides a linear decrease in the frequency of reconnect
   attempts.  this is for 'always-on' dashboard systems that might just sit
   there for hours at a time with no connection available.  this only works
   with 'chatterwait' mode enabled ... */
#define NICE_RECONNECT

/* max delay in milliseconds */
#define NICE_RECON_MAXDELAY 500

/* this is added to actual message length, incl. header and checksum, to
   determine packet length byte (byte 2 of most aldl messages).  so far, no
   known ecms use a constant other than 0x52 */
#define MSGLENGTH_MAGICNUMBER 0x52

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


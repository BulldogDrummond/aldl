#ifndef _ALDL_CONFIG_H
#define _ALDL_CONFIG_H

/*------ DEBUG STUFF --------------------------------------*/

/* extra debugging routines that can probably be disabed in production */
#define SERIAL_DEBUG

/* spits a bunch of random shit out on stdout.  probably disable when using
   an actual display interface */
#define SERIAL_VERBOSE
//#define SERIAL_SUPERVERBOSE

/* verbose aldl comm routines on stdout */
#define ALDL_VERBOSE

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

#endif

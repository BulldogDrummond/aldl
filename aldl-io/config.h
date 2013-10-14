#ifndef _ALDL_CONFIG_H
#define _ALDL_CONFIG_H

/*------ DEBUG STUFF --------------------------------------*/

/* extra debugging routines that can probably be disabed in production */
#define SERIAL_DEBUG

/* spits a bunch of random shit out on stdout.  probably disable when using
   an actual display interface */
#define SERIAL_VERBOSE

/* verbose aldl comm routines on stdout */
#define ALDL_VERBOSE

/* a static delay in milliseconds.  used for waiting in between grabbing
   serial chunks, and for delay timers.  decreasing this value makes timeouts
   more accurate.  increasing improves cpu usage. */
#define SLEEPYTIME 2

/* the baud rate to set for the ftdi usb userland driver.  reccommend 8192. */
#define FTDI_BAUD 8192

/* length of an aldl mode change request string */
/* FIXME this might not be good for other ECMS */
#define SHUTUP_LENGTH 4

#endif

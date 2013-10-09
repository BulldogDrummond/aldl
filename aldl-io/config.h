
/*------ DEBUG STUFF --------------------------------------*/

/* extra debugging routines that can probably be disabed in production */
#define SERIAL_DEBUG

/* spits a bunch of random shit out on stdout.  probably disable when using
   an actual display interface */
#define SERIAL_VERBOSE

/*------ TIMING CONSTANTS ---------------------------------*/

/* a microsecond sleep time to be used for cpu control in certain functions,
   mostly related to serial buffer polling.  might make this runtime config
   later. */
#define ALDL_SLEEPYTIME 200

/* space out aldl requests a bit more than usual to ensure serialization of
   request and reply.  unused at this point */
#define ALDL_SLOW

#ifndef ALDLIO_H
#define ALDLIO_H

#include "aldl-types.h"

/* diagnostic comms ------------------------------*/

int aldl_reconnect(); /* go into diagnostic mode, returns 1 on success */

int aldl_waitforchatter(); /* returns when idle traffic is detected */

byte *aldl_get_packet(aldl_packetdef_t *p); /* get packet data */

/* serial comms-----------------------------------*/

int serial_init(char *port); /* initalize the serial handler */

void serial_close(); /* close the serial port */

/* data mgmt ----------------------------------*/

/* get an array index for a definition or data chunk by id.  returns -1 if it
   doesn't exist. */
int get_definition_by_id(aldl_conf_t *c, char *id);

/* misc. useful functions ----------------------*/

/* compare a byte string n(eedle) in h(aystack), nonzero if found */
int cmp_bytestring(byte *h, int hsize, byte *n, int nsize);

/* print a string of bytes in hex format */
void printhexstring(byte *str, int length);

#endif

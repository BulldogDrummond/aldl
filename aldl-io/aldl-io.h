#ifndef ALDLIO_H
#define ALDLIO_H

#include "aldl-types.h"

/* diagnostic comms ------------------------------*/

int aldl_reconnect(); /* go into diagnostic mode, returns 1 on success */

byte *aldl_get_packet(aldl_packetdef_t *p); /* get packet data */

/* generate request strings */
byte *generate_pktcommand(aldl_packetdef_t *packet, aldl_commdef_t *comm);
byte *generate_shutup(byte len, byte mode, aldl_commdef_t *comm);

/* serial comms-----------------------------------*/

int serial_init(char *port); /* initalize the serial handler */

void serial_close(); /* close the serial port */

/* data mgmt ----------------------------------*/

/* process data from all packets, create a record, and link it to the list */
aldl_record_t *process_data(aldl_conf_t *aldl);

/* remove a record from the linked list and deallocate */
void remove_record(aldl_record_t *rec);

/* return a pointer to the oldest record in the linked list */
aldl_record_t *oldest_record(aldl_conf_t *aldl);


/* misc. useful functions ----------------------*/

/* generate a checksum byte */
byte checksum_generate(byte *buf, int len);

/* test checksum byte of buf, 1 if ok */
int checksum_test(byte *buf, int len);

/* compare a byte string n(eedle) in h(aystack), nonzero if found */
int cmp_bytestring(byte *h, int hsize, byte *n, int nsize);

/* print a string of bytes in hex format */
void printhexstring(byte *str, int length);

#endif

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>

#include "serio.h"
#include "config.h"

#include "../error.h"
#include "aldl-io.h"
#include "../configfile/configfile.h"

/* fill data pointer from pkt using definition */
aldl_data_t *aldl_parse_def(aldl_data_t *out, aldl_packetdef_t *pkt,
                    aldl_define_t *def);

/* where size is the number of bits and p is a pointer to the beginning of the
   data, output the result as an int */
int inputsizeconvert(int size, byte *p);

/* get a single bit from a byte */
int getbit(byte *p, int byte);

aldl_data_t *aldl_parse_def(aldl_data_t *out, aldl_packetdef_t *pkt,
                    aldl_define_t *def) {
  if(def->packet != pkt->id) fatalerror(ERROR_RANGE,"parse id mismatch");
  byte *p = ( pkt->data + pkt->offset + def->offset );
  long int data = inputsizeconvert(def->size,p);
  printf("GOT DATA %li\n",data);
  return NULL;
};

int getbit(byte *p, int byte) {
  if(byte < 0 || byte > 7) fatalerror(ERROR_RANGE,"byte field number");
  return( ( *p >> ( byte + 1 ) & 0x01 ));
};

int inputsizeconvert(int size, byte *p) {
    switch(size) {
    case 8:
      return (long int)*p;
      break;
    case 16:
      return (long int)((*p<<8)|*(p+1));
      break;
    default:
      fatalerror(ERROR_RANGE,"bad input size in definition"); 
  };
  return 0;
};

/* a debug output function ... */
void printhexstring(byte *str, int length) {
  int x;
  for(x=0;x<length;x++) printf("%X ",(unsigned int)str[x]);
  printf("\n");
};


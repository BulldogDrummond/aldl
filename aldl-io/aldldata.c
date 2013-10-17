#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "serio.h"
#include "config.h"

#include "../error.h"
#include "../config.h"
#include "aldl-io.h"
#include "../configfile/configfile.h"

/* update the value in the record from definition n ... */
aldl_data_t *aldl_parse_def(aldl_conf_t *aldl, aldl_record_t *r, int n);

/* remove a record */
void remove_record(aldl_record_t *rec);

/* where size is the number of bits and p is a pointer to the beginning of the
   data, output the result as an int */
int inputsizeconvert(int size, byte *p);

/* get a single bit from a byte */
int getbit(byte *p, int bpos, int flip);

void remove_record(aldl_record_t *rec) {
  free(rec->data);
  free(rec);
};

aldl_record_t *aldl_create_record(aldl_conf_t *aldl) {
  /* allocate record memory */
  aldl_record_t *rec = malloc(sizeof(aldl_record_t));
  if(rec == NULL) fatalerror(ERROR_MEMORY,"record creation");

  /* allocate data memory */
  rec->data = malloc(sizeof(aldl_data_t) * aldl->n_defs);
  if(rec == NULL) fatalerror(ERROR_MEMORY,"data str in record");

  /* start with old data here, that way if any data isn't filled, at least the
     stale data is there ... if there's no old data, just fill zeros. */
  if(aldl->r == NULL) {
    memset(rec->data,0,sizeof(aldl_data_t) * aldl->n_defs);
  } else {
    memcpy(rec->data,aldl->r->data,sizeof(aldl_data_t) * aldl->n_defs);
  };

  /* process packet data */
  int def_n;
  for(def_n=0;def_n<aldl->n_defs;def_n++) {
    aldl_parse_def(aldl,rec,def_n); 
  };

  return rec;
};

aldl_data_t *aldl_parse_def(aldl_conf_t *aldl, aldl_record_t *r, int n) {
  /* check for out of range */
  if(n < 0 || n > aldl->n_defs - 1) fatalerror(ERROR_RANGE,"def number"); 

  aldl_define_t *def = &aldl->def[n]; /* shortcut to definition */

  /* find associated packet number as 'id' */
  int id = 0; /* array index, not actual id ..... */
  #ifdef ALDL_MULTIPACKET
  /* we'll assume the packet exists; this should be checked during config
     load time ... */
  for(id=0; id < aldl->comm->n_packets - 1; id++) {
    if(aldl->comm->packet[id].id == def->id) break;
  };
  #endif

  aldl_packetdef_t *pkt = &aldl->comm->packet[id]; /* ptr to packet */

  /* FIXME need to check for bad or stale packet here */

  /* check to ensure byte location is in range */
  if(pkt->offset + def->offset > pkt->length) {
    fatalerror(ERROR_RANGE,"definition out of packet range");
  };

  /* location of actual data byte */
  byte *data = pkt->data + def->offset + pkt->offset;

  /* location for output of data, matches definition array index ... */
  aldl_data_t *out = &r->data[n];

  /* FIXME this doesnt deal with fields wider than 1 byte ... */
  /* FIXME doesn't deal with signed input */
  /* FIXME doesn't deal with min/max */

  /* get value, this does need more work ... */
  switch(def->type) {
    case ALDL_INT:
      out->i = (int)*data + def->adder * def->multiplier;
      break;
    case ALDL_UINT:
      out->u = (unsigned int)*data + def->adder * def->multiplier;
      break;
    case ALDL_FLOAT:
      out->f = (float)*data + def->adder * def->multiplier;
      break;
    case ALDL_BOOL:
      out->i = getbit(data,def->binary,def->invert) ;
      break;
    /* raw or invalid bit just transfers the raw byte */
    case ALDL_RAW:
    default:
      out->raw = *data;
  };

  return out;
};

int getbit(byte *p, int bpos, int flip) {
  if(bpos < 0 || bpos > 7) fatalerror(ERROR_RANGE,"bit field number");
  int bit = ( *p >> ( bpos + 1 ) & 0x01 );
  /* implement XOR */
  return bit;
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


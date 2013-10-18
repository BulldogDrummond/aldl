#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include "serio.h"
#include "config.h"

#include "../error.h"
#include "../config.h"
#include "aldl-io.h"
#include "../configfile/configfile.h"

/* -------- globalstuffs ------------------ */

typedef struct aldl_lock {
  pthread_mutex_t *connstate; /* locks connection state */
  pthread_mutex_t *recordptr; /* locks linked list links in records */
} aldl_lock_t;

aldl_lock_t lock;

/* --------- local functions ---------------- */

/* update the value in the record from definition n */
aldl_data_t *aldl_parse_def(aldl_conf_t *aldl, aldl_record_t *r, int n);

/* allocate record and timestamp it */
aldl_record_t *aldl_create_record(aldl_conf_t *aldl);

/* link a prepared record to the linked list */
void link_record(aldl_record_t *rec, aldl_conf_t *aldl);

/* fill a prepared record with data */
aldl_record_t *aldl_fill_record(aldl_conf_t *aldl, aldl_record_t *rec);

/* turn two 8 bit bytes into a 16 bit int */
unsigned int sixteenbit(byte *p);

/* get a single bit from a byte */
int getbit(byte p, int bpos, int flip);

void init_locks() {
  lock.connstate = malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(lock.connstate,NULL);
  lock.recordptr = malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(lock.recordptr,NULL);
};

aldl_record_t *process_data(aldl_conf_t *aldl) {
  aldl_record_t *rec = aldl_create_record(aldl);
  aldl_fill_record(aldl,rec);
  link_record(rec,aldl);
  return rec;
};

aldl_record_t *oldest_record(aldl_conf_t *aldl) {
  aldl_record_t *last = aldl->r;
  while(last->prev != NULL) last++;
  return last;
};

void remove_record(aldl_record_t *rec) {
  if(rec->next == NULL) return; /* dont remove the only record */
  if(rec->prev != NULL) fatalerror(ERROR_NULL,"remove wrong record");
  rec->next->prev = NULL; /* delink from linked list */
  free(rec->data);
  free(rec);
};

void link_record(aldl_record_t *rec, aldl_conf_t *aldl) {
  /* prepare links in new record */
  rec->next = NULL; /* terminate linked list */
  rec->prev = aldl->r; /* previous link */
  pthread_mutex_lock(lock.recordptr);
  aldl->r->next = rec; /* attach to linked list */
  aldl->r = rec; /* fix master link */
  pthread_mutex_unlock(lock.recordptr);
};

aldl_record_t *aldl_create_record(aldl_conf_t *aldl) {
  /* allocate record memory */
  aldl_record_t *rec = malloc(sizeof(aldl_record_t));
  if(rec == NULL) fatalerror(ERROR_MEMORY,"record creation");

  /* handle no definition case, which creates records with no data,
     for testing purposes */
  if(aldl->n_defs < 1) return rec;

  /* timestamp record */
  rec->t = time(NULL);

  /* allocate data memory */
  rec->data = malloc(sizeof(aldl_data_t) * aldl->n_defs);
  if(rec == NULL) fatalerror(ERROR_MEMORY,"data str in record");

  return rec;
};

aldl_record_t *aldl_fill_record(aldl_conf_t *aldl, aldl_record_t *rec) {
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

  /* check to ensure byte location is in range */
  /* FIXME this could be checked while loading definitions ... */
  if(pkt->offset + def->offset > pkt->length) {
    fatalerror(ERROR_RANGE,"definition out of packet range");
  };

  /* location of actual data byte */
  byte *data = pkt->data + def->offset + pkt->offset;

  /* location for output of data, matches definition array index ... */
  aldl_data_t *out = &r->data[n];

  /* FIXME doesn't deal with signed input */
  /* FIXME doesn't deal with min/max */

  unsigned int x; /* converted value */
  switch(def->size) {
    case 16:
      x = sixteenbit(data);
      break;
    case 8: /* direct conversion */
    default:
      x = (unsigned int)*data;
  };

  /* get value, this does need more work ... */
  switch(def->type) {
    case ALDL_INT:
      out->i = (int)x + def->adder * def->multiplier;
      break;
    case ALDL_UINT:
      out->u = (unsigned int)x + def->adder * def->multiplier;
      break;
    case ALDL_FLOAT:
      out->f = (float)x + def->adder * def->multiplier;
      break;
    case ALDL_BOOL:
      out->i = getbit(x,def->binary,def->invert) ;
      break;
    /* raw or invalid bit just transfers the raw byte */
    case ALDL_RAW:
    default:
      out->raw = *data;
  };

  return out;
};

int getbit(byte p, int bpos, int flip) {
  if(bpos < 0 || bpos > 7) fatalerror(ERROR_RANGE,"bit field number");
  int bit = ( p >> ( bpos + 1 ) & 0x01 );
  /* implement XOR */
  return bit;
};

unsigned int sixteenbit(byte *p) {
  return (unsigned int)((*p<<8)|*(p+1));
};

aldl_state_t get_connstate(aldl_conf_t *aldl) {
  pthread_mutex_lock(lock.connstate);
  return aldl->state;
  pthread_mutex_unlock(lock.connstate);
};

void set_connstate(aldl_state_t s, aldl_conf_t *aldl) {
  pthread_mutex_lock(lock.connstate);
  aldl->state = s;
  pthread_mutex_unlock(lock.connstate);
};

/* a debug output function ... */
void printhexstring(byte *str, int length) {
  int x;
  for(x=0;x<length;x++) printf("%X ",(unsigned int)str[x]);
  printf("\n");
};


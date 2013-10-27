#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <limits.h>

#include "serio.h"
#include "config.h"

#include "error.h"
#include "config.h"
#include "aldl-io.h"
#include "useful.h"

/* -------- globalstuffs ------------------ */

/* locking */
typedef enum aldl_lock {
  LOCK_CONNSTATE = 0,
  LOCK_RECORDPTR = 1,
  LOCK_STATS = 2,
  N_LOCKS = 3
} aldl_lock_t;
pthread_mutex_t *aldllock;

timespec_t firstrecordtime; /* timestamp used to calc. relative time */

/* --------- local function decl. ---------------- */

/* update the value in the record from definition n */
aldl_data_t *aldl_parse_def(aldl_conf_t *aldl, aldl_record_t *r, int n);

/* allocate record and timestamp it */
aldl_record_t *aldl_create_record(aldl_conf_t *aldl);

/* link a prepared record to the linked list */
void link_record(aldl_record_t *rec, aldl_conf_t *aldl);

/* fill a prepared record with data */
aldl_record_t *aldl_fill_record(aldl_conf_t *aldl, aldl_record_t *rec);

/* set and unset locks, wrapper with error checking for pthread funcs */
inline void set_lock(aldl_lock_t lock_number);
inline void unset_lock(aldl_lock_t lock_number);

/* --------------------------------------------------------- */

void init_locks() {
  int x;
  int pthreaderr;
  aldllock = smalloc(sizeof(pthread_mutex_t) * N_LOCKS); 
  for(x=0;x<N_LOCKS;x++) {
    pthreaderr = pthread_mutex_init(&aldllock[x],NULL); 
    if(pthreaderr != 0) fatalerror(ERROR_LOCK,
         "error initializing lock %i, pthread error %i",x,pthreaderr);
  };
};

inline void set_lock(aldl_lock_t lock_number) {
  int rtval;
  rtval = pthread_mutex_lock(&aldllock[lock_number]);
  if(rtval != 0) fatalerror(ERROR_LOCK,
          "error setting lock %i, pthread error code %i",lock_number,rtval);
};

inline void unset_lock(aldl_lock_t lock_number) {
  int rtval;
  rtval = pthread_mutex_unlock(&aldllock[lock_number]);
  if(rtval != 0) fatalerror(ERROR_LOCK,
          "error unsetting lock %i, pthread error code %i",lock_number,rtval);
};

void lock_stats() {
  set_lock(LOCK_STATS);
};

void unlock_stats() {
  unset_lock(LOCK_STATS);
};

aldl_record_t *process_data(aldl_conf_t *aldl) {
  aldl_record_t *rec = aldl_create_record(aldl);
  aldl_fill_record(aldl,rec);
  link_record(rec,aldl);
  return rec;
};

aldl_record_t *oldest_record(aldl_conf_t *aldl) {
  aldl_record_t *last = aldl->r;
  while(last->prev != NULL) last = last->prev;
  return last;
};

void remove_record(aldl_record_t *rec) {
  #ifdef DEBUGSTRUCT
  if(rec->next == NULL) fatalerror(ERROR_NULL,"remove only record");
  if(rec->prev != NULL) fatalerror(ERROR_NULL,"remove wrong record");
  #endif
  rec->next->prev = NULL; /* delink from linked list */
  free(rec->data);
  free(rec);
};

void link_record(aldl_record_t *rec, aldl_conf_t *aldl) {
  rec->next = NULL; /* terminate linked list */
  rec->prev = aldl->r; /* previous link */
  set_lock(LOCK_RECORDPTR);
  aldl->r->next = rec; /* attach to linked list */
  aldl->r = rec; /* fix master link */
  unset_lock(LOCK_RECORDPTR);
};

void aldl_init_record(aldl_conf_t *aldl) {
  aldl_record_t *rec = aldl_create_record(aldl);
  set_lock(LOCK_RECORDPTR);
  rec->next = NULL;
  rec->prev = NULL;
  aldl->r = rec;
  unset_lock(LOCK_RECORDPTR);
  firstrecordtime = get_time();
};

aldl_record_t *aldl_create_record(aldl_conf_t *aldl) {
  /* allocate record memory */
  aldl_record_t *rec = smalloc(sizeof(aldl_record_t));

  /* timestamp record */
  rec->t = get_elapsed_ms(firstrecordtime);

  #ifdef TIMESTAMP_WRAPAROUND
  /* handle wraparound if we're 100 seconds before time limit */
  if(rec->t > ULONG_MAX - 100000) firstrecordtime = get_time();
  #endif

  /* allocate data memory */
  rec->data = smalloc(sizeof(aldl_data_t) * aldl->n_defs);

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
  if(n < 0 || n > aldl->n_defs - 1) fatalerror(ERROR_RANGE,
                                    "def number %i is out of range",n); 

  aldl_define_t *def = &aldl->def[n]; /* shortcut to definition */

  /* find associated packet number as 'id' */
  int id = 0; /* array index, not actual id ..... */
  #ifdef ALDL_MULTIPACKET
  /* we'll assume the packet exists; this should be checked during config
     load time ... */
  for(id=0; id < aldl->comm->n_packets; id++) {
    if(aldl->comm->packet[id].id == def->packet) break;
  };
  #endif

  aldl_packetdef_t *pkt = &aldl->comm->packet[id]; /* ptr to packet */

  /* location of actual data byte */
  byte *data = pkt->data + def->offset + pkt->offset;

  /* location for output of data, matches definition array index ... */
  aldl_data_t *out = &r->data[n];

  /* FIXME doesn't deal with signed input */

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
      out->i = ( (int)x * def->multiplier.i ) + def->adder.i;
      if(aldl->minmax == 1) {
        if(out->i < def->min.i) out->i = def->min.i;
        if(out->i > def->max.i) out->i = def->max.i;
      };
      break;
    case ALDL_FLOAT:
      out->f = ( (float)x * def->multiplier.f ) + def->adder.f;
      if(aldl->minmax == 1) {
        if(out->f < def->min.f) out->f = def->min.f;
        if(out->f > def->max.f) out->f = def->max.f;
      };
      break;
    case ALDL_BOOL:
      out->i = getbit(x,def->binary,def->invert);
      break;
    default:
      fatalerror(ERROR_RANGE,"invalid type spec: %i",def->type);
  };

  return out;
};

aldl_state_t get_connstate(aldl_conf_t *aldl) {
  set_lock(LOCK_CONNSTATE);
  aldl_state_t st = aldl->state;
  unset_lock(LOCK_CONNSTATE);
  return st;
};

void set_connstate(aldl_state_t s, aldl_conf_t *aldl) {
  set_lock(LOCK_CONNSTATE);
  #ifdef DEBUGSTRUCT
  printf("set connection state to %i\n",s);
  #endif
  aldl->state = s;
  unset_lock(LOCK_CONNSTATE);
};

aldl_record_t *newest_record(aldl_conf_t *aldl) {
  aldl_record_t *rec = NULL;
  set_lock(LOCK_RECORDPTR);
  rec = aldl->r; 
  unset_lock(LOCK_RECORDPTR);
  return rec;
};

aldl_record_t *newest_record_wait(aldl_conf_t *aldl, aldl_record_t *rec) {
  aldl_record_t *next = NULL;
  while(1) {
    next = newest_record(aldl);
    if(next != rec) {
      return next;
    } else if(get_connstate(aldl) > 10) {
      return NULL;
    } else {
      #ifndef AGGRESSIVE
      usleep(500);
      #endif
    };
  }; 
};

aldl_record_t *next_record_wait(aldl_conf_t *aldl, aldl_record_t *rec) {
  aldl_record_t *next = NULL;
  while(next == NULL) {
    next = next_record(rec);
    if(get_connstate(aldl) > 10) return NULL;
    #ifndef AGGRESSIVE
    usleep(500); /* throttling ... */
    #endif
  };
  return next;
};

aldl_record_t *next_record_waitf(aldl_conf_t *aldl, aldl_record_t *rec) {
  aldl_record_t *next = NULL;
  while((next = next_record_wait(aldl,rec)) == NULL) usleep(500);
  return next;
};

aldl_record_t *newest_record_waitf(aldl_conf_t *aldl, aldl_record_t *rec) {
  aldl_record_t *next = NULL;
  while((next = newest_record_wait(aldl,rec)) == NULL) usleep(500);
  return next;
};

aldl_record_t *next_record(aldl_record_t *rec) {
  #ifdef DEBUGSTRUCT
  /* check for underrun ... */
  if(rec->prev == NULL) {
     fatalerror(ERROR_BUFFER,"underrun in record retrieve %p",rec);
  };
  #endif
  aldl_record_t *next;
  set_lock(LOCK_RECORDPTR);
  next = rec->next;
  unset_lock(LOCK_RECORDPTR);
  return next;
};

void pause_until_connected(aldl_conf_t *aldl) {
  while(get_connstate(aldl) > 10) usleep(100);
};

void pause_until_buffered(aldl_conf_t *aldl) {
  while(aldl->ready ==0) usleep(50);
};

int get_index_by_id(aldl_conf_t *aldl, int id) {
  int x;
  for(x=0;x<aldl->n_defs;x++) {
    if(id == aldl->def[x].id) return x;
  };
  return -1; /* not found */
};

int get_index_by_name(aldl_conf_t *aldl, char *name) {
  int x;
  for(x=0;x<aldl->n_defs;x++) {
    if(faststrcmp(name,aldl->def[x].name) == 1) return x;
  };
  return -1; /* not found */
};

char *get_state_string(aldl_state_t s) {
  switch(s) {
    case ALDL_CONNECTED:
      return "Connected";
    case ALDL_CONNECTING:
      return "Connecting";
    case ALDL_LOADING:
      return "Loading";
    case ALDL_DESYNC:
      return "Lost Sync";
    case ALDL_ERROR:
      return "Error";
    case ALDL_LAGGY:
      return "Laggy";
    case ALDL_QUIT:
      return "Quit";
    case ALDL_PAUSE:
      return "Paused";
    default:
      return "Undefined";
  };
};


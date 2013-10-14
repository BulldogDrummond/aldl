#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "serio.h"
#include "config.h"

#include "aldl-io.h"
#include "../configfile/configfile.h"

aldl_record_t *get_next_record(aldl_record_t *r) {
  while(r->next == NULL);
  return r->next;
}

int get_definition_by_id(aldl_conf_t *c, char *id) {
  int x;
  for(x=0;x<c->n;x++) { /* simple ordered search */
    if(strcmp(c->def[x].id,id) == 0) return x;
  };
  return -1;
}

/* get type casted and formatted output from a peice of data by its array
   index.  this is the "safe" way to get data, but if you pick the wrong
   type, a conversion is applied.  alternatively, typecast on the actual
   data pointer. */
int get_int_data(aldl_record_t *r, int i);
unsigned int get_uint_data(aldl_record_t *r, int i);
float get_float_data(aldl_record_t *r, int i);
int get_bool_data(aldl_record_t *r, int i);

/* per-record lock management.  this should be unnecessary if you're only
   reading the newest record, unless your plugin is rediculously slow.. */
void set_record_lock(aldl_record_t *r);
void unset_record_lock(aldl_record_t *r);

void link_record(aldl_conf_t *c, aldl_record_t *r) {
  /* prepare record for linking */
  //set_record_lock(r);
  r->next = NULL; /* ensure record is terminated properly */
  r->prev = c->r; /* old latest record is new previous record */
  /* link record */
  if(c->r->next != NULL) return; /* FIXME this is a fatal error condit. ... */
  c->r->next = r; /* link to record */
  c->r = r; /* link to stream */
  //unset_record_lock(r);
}

/* prepare an unlinked record to later be filled with parsed data */

aldl_record_t *prepare_record(aldl_conf_t *c) {
  /* allocate record header */
  aldl_record_t *r = malloc(sizeof(aldl_record_t)); 
  //size_t data_size = 0;
  /* allocate data space */
  return r;
};

/* a debug output function ... */
void printhexstring(byte *str, int length) {
  int x;
  for(x=0;x<length;x++) {
    printf("%X.2 ",(unsigned int)str[x]);
  };
  printf("\n");
};


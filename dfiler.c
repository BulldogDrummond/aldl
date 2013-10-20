#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dfiler.h"

#define MAX_PARAMETERS 500

inline int is_whitespace(char ch);

dfile_t *dfile_load(char *filename) {
  char *data = load_file(filename);
  if(data == NULL) return NULL;
  dfile_t *d = dfile(data);
  dfile_shrink(d);
  free(data);
  return d; 
};

dfile_t *dfile(char *data) {
  /* allocate base structure */
  dfile_t *out = malloc(sizeof(dfile_t));

  /* initial allocation of pointer array */
  out->p = malloc(sizeof(char*) * MAX_PARAMETERS);
  out->v = malloc(sizeof(char*) * MAX_PARAMETERS);
  out->n = 0;

  /* more useful variables */
  char *c; /* operating cursor within data */
  char *cx; /* auxiliary operating cursor */
  char *cz;
  int len = strlen(data);

  for(c=data;c<data+len;c++) { /* iterate through entire data set */
    if(c[0] == '=') {
      if(c == data || c == data + len) continue; /* handle first or last char */
      /* set value starting point */
      out->v[out->n] = c + 1;
      /* null the delimter */
      c[0] = 0;
      /* find ending point */
      cx = c + 1; /* initialize aux cursor */ 
      while(is_whitespace(*cx) != 1) {
        if(*cx == '"') { /* skip quoted string */
          cx++;
          while(cx[0] != '"') { /* until there's another quote or EOF */
            if(cx == data + len) continue; /* end of file */
            cx++;
          };
        };
        cx++;
      };
      cx[0] = 0; /* null end */
      cz = cx; /* rememeber end point */
      /* find starting point */
      cx = c - 1;
      while(is_whitespace(*cx) != 1) {
        if(*cx == '"') { /* skip quoted string */
          cx--;
          while(cx[0] != '"') { /* until there's another quote or EOF */
            if(cx == data + len) continue; /* end of file */
            cx--;
          };
        };
        cx--;
        if(cx == data) { /* handle case of beginning of file */
          cx--;
          break;
        };
      };
      out->p[out->n] = cx + 1;
      out->n++;
      if(out->n == MAX_PARAMETERS) return out; /* out of space */
      c = cz;
    };
  };
  return out;
};

/* reduce memory footprint */
char *dfile_shrink(dfile_t *d) {
  /* shrink arrays */
  d->p = realloc(d->p,sizeof(char*) * d->n);
  d->v = realloc(d->v,sizeof(char*) * d->n);
  /* calculate total size of new data storage */
  size_t ttl = 0;
  int x;
  for(x=0;x<d->n;x++) {
    ttl += strlen(d->p[x]);
    ttl += strlen(d->v[x]);
    ttl += 2; /* for null terminators */
  };
  ttl++;
  /* move everything and update pointers */
  char *newdata = malloc(ttl); /* new storage */
  char *c = newdata; /* cursor*/
  for(x=0;x<d->n;x++) {
    /* copy parameter */
    strcpy(c,d->p[x]);
    d->p[x] = c;
    c += (strlen(c) + 1);
    /* copy value */
    strcpy(c,d->v[x]);
    d->v[x] = c;
    c += (strlen(c) + 1);
  };
  return newdata;
};


inline int is_whitespace(char ch) {
  if(ch == 0 || ch == ' ' || ch == '\n') return 1;
  return 0;
};

/* read file into memory */
char *load_file(char *filename) {
  FILE *fdesc;
  if(filename == NULL) return NULL;
  fdesc = fopen(filename, "r");
  if(fdesc == NULL) return NULL;
  fseek(fdesc, 0L, SEEK_END);
  int flength = ftell(fdesc);
  if(flength == -1) return NULL;
  rewind(fdesc);
  char *buf = malloc(sizeof(char) * ( flength + 1));
  if(buf == NULL) return NULL;
  if(fread(buf,1,flength,fdesc) != flength) return NULL;
  fclose(fdesc);
  buf[flength] = 0;
  return buf;
};

char *value_by_parameter(char *str, dfile_t *d) {
  int x;
  int y;
  for(x=0;x<d->n;x++) {
    if(faststrcmp(str,d->p[x]) == 1) return d->v[x];
  };
  return NULL;
};

inline int faststrcmp(char *a, char *b) {
  int x = 0;
  while(a[x] == b[x]) {
    x++;
    if(a[x] == 0 || b[x] == 0) {
      if(a[x] == 0 && b[x] == 0) {
        return 1;
      } else {
        return 0;
      };
    };
  };
  return 0;
};

void print_config(dfile_t *d) {
  printf("config list has %i parsed options:-------------\n",d->n);
  int x;
  for(x=0;x<d->n;x++) printf("p(arameter):%s v(alue):%s\n",d->p[x],d->v[x]);
  printf("----------end config\n");
};

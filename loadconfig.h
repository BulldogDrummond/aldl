#ifndef _LOADCONF_H
#define _LOADCONF_H

#include "aldl-types.h"

#define MAX_PARAMETERS 65535

/* a struture with array index matched parameters and values from a cfg file */
typedef struct _dfile_t {
  unsigned int n; /* number of parameters */
  char **p;  /* parameter */
  char **v;  /* value */
} dfile_t;

/* configure all aldl structures and load config according to config file. */
aldl_conf_t *aldl_setup();

/* loads file, strips quotes, shrinks, parses in one step.. */
dfile_t *dfile_load(char *filename);

/* read file into memory */
char *load_file(char *filename);

/* split up data into parameters and values */
dfile_t *dfile(char *data);

/* reduce the data section and pointer arrays to reduce mem usage, returns
   pointer to new data to be freed later. */
char *dfile_shrink(dfile_t *d);

/* get a value by parameter string */
char *value_by_parameter(char *str, dfile_t *d);

/* strip all starting and ending quotes from quoted 'value' fields. */
void dfile_strip_quotes(dfile_t *d);

/* for debugging, print a list of all config pairs extracted. */
void print_config(dfile_t *d);

#endif

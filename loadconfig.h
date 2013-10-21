#ifndef _LOADCONF_H
#define _LOADCONF_H

#include "aldl-types.h"

/* configure all aldl structures and load config according to config file. */
aldl_conf_t *aldl_setup(char *configfile);

/* a fast string compare function, returns 1 on match 0 on fail, bails instantly
   on mismatch. */
inline int faststrcmp(char *a, char *b);

#endif

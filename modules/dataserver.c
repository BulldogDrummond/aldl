#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>

/* local objects */
#include "../error.h"
#include "../aldl-io.h"
#include "../loadconfig.h"
#include "../useful.h"

typedef struct _ds_conf {
  int max_clients; /* maximum clients that can be connected */
  dfile_t *dconf;
} ds_conf_t;

ds_conf_t *ds_load_config(aldl_conf_t *aldl);

void *dataserver_init(void *aldl_in) {
  aldl_conf_t *aldl = (aldl_conf_t *)aldl_in;
  ds_conf_t *conf = ds_load_config(aldl);

  return NULL;
};

ds_conf_t *ds_load_config(aldl_conf_t *aldl) {
  ds_conf_t *conf = smalloc(sizeof(ds_conf_t));

  if(aldl->dataserver_config == NULL) fatalerror(ERROR_CONFIG,
                               "no dataserver config file specified");
  conf->dconf = dfile_load(aldl->dataserver_config);
  if(conf->dconf == NULL) fatalerror(ERROR_CONFIG,
                                  "dataserver config file missing");
  dfile_t *config = conf->dconf;
  conf->max_clients = configopt_int(config,"MAX_CLIENTS",1,65535,1000);

  return conf;
};

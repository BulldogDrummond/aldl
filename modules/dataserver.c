#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>

/* networking*/
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

/* local objects */
#include "../error.h"
#include "../aldl-io.h"
#include "../loadconfig.h"
#include "../useful.h"

#define DS_LISTENQ 1024

typedef struct _ds_conf {
  int max_clients; /* maximum clients that can be connected */
  int listen_port;
  dfile_t *dconf;
  struct sockaddr_in addr; /* server address */
} ds_conf_t;

ds_conf_t *ds_load_config(aldl_conf_t *aldl);

void *dataserver_init(void *aldl_in) {
  aldl_conf_t *aldl = (aldl_conf_t *)aldl_in;
  int list_s; /* listen socket */
  int conn_s; /* connection socket */

  /* load configuration */
  ds_conf_t *conf = ds_load_config(aldl);

  /* configure address structure */
  memset(&conf->addr,0,sizeof(struct sockaddr_in));
  conf->addr.sin_family = AF_INET;
  conf->addr.sin_addr.s_addr = htonl(INADDR_ANY);
  conf->addr.sin_port = conf->listen_port;

  /* create, bind, and listen ...  */
  if((list_s = socket(AF_INET,SOCK_STREAM,0)) < 0) {
    fatalerror(ERROR_NET,"can't create listen socket");
  };
  if(bind(list_s,(struct sockaddr *) &conf->addr,
           sizeof(struct sockaddr_in)) < 0 ) {
    fatalerror(ERROR_NET,"can't bind to socket");
  };
  if(listen(list_s,DS_LISTENQ) < 0) {
    fatalerror(ERROR_NET,"can't listen on socket");
  };
  fatalerror(ERROR_NET,"server not written yet...");

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
  conf->listen_port = configopt_int(config,"LISTEN_PORT",2,65534,42012);

  return conf;
};

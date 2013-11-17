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
#include "../config.h"

#define DS_LISTENQ 1024

typedef struct _ds_conf {
  int max_clients; /* maximum clients that can be connected */
  int listen_port; /* the listen port as an integer */
  int listen_all; /* listen on all interfaces */
  char *listen_addr;  /* address to listen to */
  dfile_t *dconf; /* the configuration */
  struct sockaddr_in addr; /* server address */
} ds_conf_t;

/* local functions */
ds_conf_t *ds_load_config(aldl_conf_t *aldl);
void ds_get_addrstruct(ds_conf_t *conf);

void *dataserver_init(void *aldl_in) {
  aldl_conf_t *aldl = (aldl_conf_t *)aldl_in;
  int list_s; /* listen socket */
  int conn_s; /* connection socket */

  /* load configuration */
  ds_conf_t *conf = ds_load_config(aldl);

  /* configure port and address */
  ds_get_addrstruct(conf);

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

  #ifdef NET_VERBOSE
  printf("bound to socket\n");
  #endif

  /* event loop */
  while(1) {
    #ifdef NET_VERBOSE
    printf("listening...\n");
    #endif
    if((conn_s = accept(list_s,NULL,NULL))<0) { /* FIXME */
      fatalerror(ERROR_NET,"error accepting connection");
    };

    /* do stuff here ... */

    if(close(conn_s) <0) {
      fatalerror(ERROR_NET,"close error");
    };
  };

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
  conf->listen_all = configopt_int(config,"LISTEN_ALL",0,1,0);
  conf->listen_addr = configopt(config,"LISTEN_ADDR","127.0.0.1");

  return conf;
};

void ds_get_addrstruct(ds_conf_t *conf) {
  /* configure address structure */
  memset(&conf->addr,0,sizeof(struct sockaddr_in));
  conf->addr.sin_family = AF_INET;
  if(conf->listen_all == 1) {
    conf->addr.sin_addr.s_addr = htonl(INADDR_ANY);
  } else {
    conf->addr.sin_addr.s_addr = inet_addr(conf->listen_addr);
    if(conf->addr.sin_addr.s_addr == INADDR_NONE) {
      fatalerror(ERROR_NET,"specified address %s seems invalid",
                conf->listen_addr);
    };
  };
  /* configure port */
  conf->addr.sin_port = htons(conf->listen_port);
  #ifdef NET_VERBOSE
  printf("configured to listen on %s:%i\n",
               inet_ntoa(conf->addr.sin_addr),
               conf->listen_port);
  #endif
};

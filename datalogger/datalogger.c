#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/* local objects */
#include "../error.h"
#include "../aldl-io.h"
#include "../loadconfig.h"

typedef struct _datalogger_conf {
  dfile_t *dconf; /* raw config data */
  int autostart;
  char *log_filename;
  int log_all;
  FILE *fdesc;
} datalogger_conf_t;

datalogger_conf_t *datalogger_load_config(aldl_conf_t *aldl);

void *datalogger_init(void *aldl_in) {
  /* grab config data */
  aldl_conf_t *aldl = (aldl_conf_t *)aldl_in;
  datalogger_conf_t *conf = datalogger_load_config(aldl);

  /* alloc and fill filename buffer */
  int maxfnlength = strlen(conf->log_filename) * 2 + 20;
  struct tm *tm;
  time_t t;
  t = time(NULL);
  tm = localtime(&t);
  char *filename = malloc(maxfnlength);
  strftime(filename,maxfnlength,conf->log_filename,tm);

  /* open file */
  conf->fdesc = fopen(filename, "a");
  if(conf->fdesc == NULL) fatalerror(ERROR_PLUGIN,"cannot append to log");
  free(filename); /* shouldn't need to re-open it */

  /* wait for buffered connection */
  pause_until_buffered(aldl);

  /* FIXME this just logs all the time.  that's stupid. */

  /* bigass buffer, should calculate this better */
  char *linebuf = malloc(aldl->n_defs * 128);
  char *cursor = linebuf;

  int x;
  cursor += sprintf(cursor,"TIMESTAMP");
  for(x=0;x<aldl->n_defs;x++) {
    if(conf->log_all == 0) {
      if(aldl->def[x].log == 0) continue;
    };
    cursor += sprintf(cursor,",%s",aldl->def[x].name);
  };
  cursor += sprintf(cursor,"\n");
  fwrite(linebuf,cursor - linebuf,1,conf->fdesc);

  aldl_record_t *rec = aldl->r;
  /* event loop */
  while(1) {
    rec = next_record_wait(rec);
    cursor=linebuf; /* reset cursor */
    cursor += sprintf(cursor,"%lu",rec->t);
    for(x=0;x<aldl->n_defs;x++) {
      if(conf->log_all == 0) {
        if(aldl->def[x].log == 0) continue;
      };
      switch(aldl->def[x].type) {
        case ALDL_FLOAT:
          cursor += sprintf(cursor,",%.2f",rec->data[x].f);
          break;
        case ALDL_INT:
          cursor += sprintf(cursor,",%i",rec->data[x].i);
          break;
        default:
          cursor += sprintf(cursor,",");
      };
    };
    cursor += sprintf(cursor,"\n");
    fwrite(linebuf,cursor - linebuf,1,conf->fdesc);
  };

  fclose(conf->fdesc);
  /* end ... */
  return NULL;
};

datalogger_conf_t *datalogger_load_config(aldl_conf_t *aldl) {
  datalogger_conf_t *conf = malloc(sizeof(datalogger_conf_t));
  if(aldl->datalogger_config == NULL) fatalerror(ERROR_CONFIG,
                               "no datalogger config file specified");
  conf->dconf = dfile_load(aldl->datalogger_config);
  if(conf->dconf == NULL) fatalerror(ERROR_CONFIG,
                                  "datalogger config file missing");
  dfile_t *config = conf->dconf;
  conf->autostart = configopt_int(config,"AUTOSTART",0,1,1);
  conf->log_all = configopt_int(config,"LOG_ALL",0,1,0);
  conf->log_filename = configopt_fatal(config,"LOG_FILENAME");
  return conf;
};

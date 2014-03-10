#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>
#include <pthread.h>
#include <string.h>

#include "../error.h"
#include "../aldl-io.h"
#include "../config.h"
#include "../loadconfig.h"
#include "../useful.h"

enum {
  RED_ON_BLACK = 1,
  BLACK_ON_RED = 2,
  GREEN_ON_BLACK = 3,
  CYAN_ON_BLACK = 4,
  WHITE_ON_BLACK = 5,
  WHITE_ON_RED = 6
};

typedef enum _gaugetype {
  GAUGE_HBAR,
  GAUGE_TEXT,
  GAUGE_BIN,
  GAUGE_ERRSTR,
  GAUGE_SHIFT
} gaugetype_t;

typedef struct _gauge {
  int x, y; /* coords */
  int width, height; /* size */
  int data_a, data_b; /* assoc. data index */
  aldl_data_t prev_a, prev_b; /* prev. value */
  float bottom, top; /* bottom and top of a graph */
  int smoothing; /* averaging */
  int weight;  /* smoothing weight */
  gaugetype_t gaugetype;
} gauge_t;

/* some global cached indexes */
int index_rpm, index_map, index_speed;

typedef struct _consoleif_conf {
  int n_gauges;
  gauge_t *gauge; 
  dfile_t *dconf;
  int statusbar;
  int delay;
} consoleif_conf_t;

#define COLOR_STATUSSCREEN RED_ON_BLACK

/* --- variables ---------------------------- */

int w_height, w_width; /* width and height of window */

aldl_conf_t *aldl; /* global pointer to aldl conf struct */

char *bigbuf; /* a large temporary string construction buffer */

aldl_record_t *rec; /* current record */

#define N_GEARS 6
float geartable[N_GEARS];

/* --- local functions ------------------------*/

consoleif_conf_t *consoleif_load_config(aldl_conf_t *aldl);

/* center half-width of an element on the screen */
int xcenter(int width);
int ycenter(int height);
 
/* print a centered string */
void print_centered_string(char *str);
void statusmessage(char *str);

/* clear screen and display waiting for connection messages */
void cons_wait_for_connection();

/* get a config string for a particular gauge */
char *gconfig(char *parameter, int n);

/* "smooth" a float */
float smooth_float(gauge_t *g);

/* check if a value is past alarm range */
int alarm_range(gauge_t *g);

/* gauges -------------------*/
void draw_h_progressbar(gauge_t *g);
void draw_simpletext_a(gauge_t *g);
void draw_bin(gauge_t *g);
void draw_errstr(gauge_t *g);
void draw_shift(gauge_t *g);
void gauge_blank(gauge_t *g);
void draw_statusbar();

/* --------------------------------------------*/

void *consoleif_init(void *aldl_in) {
  aldl = (aldl_conf_t *)aldl_in;

  bigbuf = malloc(512);

  /* load config file */
  consoleif_conf_t *conf = consoleif_load_config(aldl);

  /* initialize root window */
  WINDOW *root;
  if((root = initscr()) == NULL) {
    fatalerror(ERROR_NULL,"could not init ncurses");
  };

  curs_set(0);

  start_color();
  init_pair(RED_ON_BLACK,COLOR_RED,COLOR_BLACK);
  init_pair(BLACK_ON_RED,COLOR_BLACK,COLOR_RED);
  init_pair(GREEN_ON_BLACK,COLOR_GREEN,COLOR_BLACK);
  init_pair(CYAN_ON_BLACK,COLOR_CYAN,COLOR_BLACK);
  init_pair(WHITE_ON_BLACK,COLOR_WHITE,COLOR_BLACK);
  init_pair(WHITE_ON_RED,COLOR_WHITE,COLOR_RED);


  /* get initial screen size */
  getmaxyx(stdscr,w_height,w_width);

  /* some globals */
  index_rpm = get_index_by_name(aldl,"RPM");
  index_map = get_index_by_name(aldl,"MAP");
  index_speed = get_index_by_name(aldl,"SPEED");

  cons_wait_for_connection();

  int x;
  gauge_t *gauge;

  while(1) {
    rec = newest_record_wait(aldl,rec);
    if(rec == NULL) { /* disconnected */
      cons_wait_for_connection();
      continue;
    };
    for(x=0;x<conf->n_gauges;x++) {
      gauge = &conf->gauge[x];
      switch(gauge->gaugetype) {
        case GAUGE_HBAR:
          draw_h_progressbar(gauge);
          break;
        case GAUGE_TEXT:
          draw_simpletext_a(gauge);
          break;
        case GAUGE_BIN:
          draw_bin(gauge);
          break;
        case GAUGE_ERRSTR:
          draw_errstr(gauge);
          break;
        case GAUGE_SHIFT:
          draw_shift(gauge);
          break;
        default:
          break;
      };
    };
    if(conf->statusbar == 1) {
      draw_statusbar();
    };
    refresh();
    usleep(conf->delay);
  };

  sleep(4);
  delwin(root);
  endwin();
  refresh();

  pthread_exit(NULL);
  return NULL;
};

int xcenter(int width) {
  return ( w_width / 2 ) - ( width / 2 );
}

int ycenter(int height) {
  return ( w_height / 2 ) - ( height / 2 );
}

void print_centered_string(char *str) {
  mvaddstr(ycenter(0),xcenter(strlen(str)),str);
};

void draw_statusbar() {
  lock_stats();
  float pps = aldl->stats->packetspersecond;
  unsigned int failcounter = aldl->stats->failcounter;
  unlock_stats();
  if(w_width < 40) { /* small statusbar */
    mvprintw(w_height - 1,0,"%u R=%.1f ERR=%u  ",
             rec->t / 1000, pps, failcounter);
  } else { /* lg statusbar */
    mvprintw(w_height - 1,1,"%s  TIMESTAMP: %i  PKT/S: %.1f  FAILED: %u  ",
             VERSION, rec->t, pps, failcounter);
  };
};

void statusmessage(char *str) {
  clear();
  attron(COLOR_PAIR(COLOR_STATUSSCREEN));
  print_centered_string(str);
  mvaddstr(1,1,VERSION);
  attroff(COLOR_PAIR(COLOR_STATUSSCREEN));
  refresh();
  usleep(5000);
};

void cons_wait_for_connection() {
  aldl_state_t s = ALDL_LOADING;
  aldl_state_t s_cache = ALDL_LOADING; /* cache to avoid redraws */
  while(s > 10) {
    s = get_connstate(aldl);
    if(s != s_cache) statusmessage(get_state_string(s));
    s_cache = s;
    usleep(100000);
  };

  statusmessage("Buffering...");
  pause_until_buffered(aldl);

  clear();
}

/* --- GAUGES ---------------------------------- */

void draw_bin(gauge_t *g) {
  aldl_define_t *def = &aldl->def[g->data_a];
  gauge_blank(g);
  aldl_data_t *data = &rec->data[g->data_a];
  if(data->i == 0) return;
  attron(COLOR_PAIR(GREEN_ON_BLACK));
  mvprintw(g->y,g->x,"%s",def->name);
  attroff(COLOR_PAIR(GREEN_ON_BLACK));
};

void draw_errstr(gauge_t *g) {
  int errfound = 0; 
  int x = 0;
  gauge_blank(g);
  for(x=0;x<=aldl->n_defs - 1;x++) {
    if(aldl->def[x].err == 1) { /* is an err flag */
      if(rec->data[x].i == 1) { /* err flag is set */
        if(errfound > 4) return; /* display max 4 codes */
        attron(COLOR_PAIR(RED_ON_BLACK));
        errfound++;
        if(errfound == 1) mvprintw(g->y,g->x,"ERROR:");
        printw(" %s",aldl->def[x].name);
        attroff(COLOR_PAIR(RED_ON_BLACK));
      };
    };
  };
  if(errfound == 0) mvprintw(g->y,g->x,"NO ERRORS");
};

void draw_shift(gauge_t *g) {
  aldl_define_t *rpmdef = &aldl->def[index_rpm];
  aldl_define_t *speeddef = &aldl->def[index_speed];
  aldl_data_t *rpmdata = &rec->data[index_rpm];
  aldl_data_t *speeddata = &rec->data[index_speed];
  gauge_blank(g);

  int gear;
  float drive_ratio = speeddata / rpmdata;
  float diff = 99999;
  float t;
  int closest_gear = 0;
  for(gear=1;gear<=N_GEARS;gear++) {
    /* get unsigned difference */
    if(drive_ratio <= geartable[gear]) {
      t = geartable[gear] - drive_ratio;
    } else {
      t = drive_ratio - geartable[gear];
    };
    if(t < diff) {
      diff = t;
      closest_gear = gear;
    };
  };
};

void draw_simpletext_a(gauge_t *g) {
  aldl_define_t *def = &aldl->def[g->data_a];
  gauge_blank(g);
  aldl_data_t *data = &rec->data[g->data_a];
  if(alarm_range(g) == 1) attron(COLOR_PAIR(RED_ON_BLACK));
  switch(def->type) {
    case ALDL_FLOAT:
      mvprintw(g->y,g->x,"%s: %.1f",
            def->name,smooth_float(g));
      #ifdef CONSOLEIF_UOM
      if(def->uom != NULL) printw(" %s",def->uom);
      #endif
      break;
    case ALDL_INT:
    case ALDL_BOOL:
      mvprintw(g->y,g->x,"%s: %i",
          def->name,data->i);
      #ifdef CONSOLEIF_UOM
      if(def->uom != NULL) printw(" %s",def->uom);
      #endif
      break;
    default:
      return;
  };
  if(alarm_range(g) == 1) attroff(COLOR_PAIR(RED_ON_BLACK));
};

int alarm_range(gauge_t *g) {
  aldl_define_t *def = &aldl->def[g->data_a];
  aldl_data_t *data = &rec->data[g->data_a];
  switch(def->type) {
    case ALDL_FLOAT:
      if( ( def->alarm_low_enable == 1 && data->f < def->alarm_low.f ) ||
      ( def->alarm_high_enable == 1 && data->f > def->alarm_high.f) ) return 1;
      return 0;
      break;
    case ALDL_INT:
      if( ( def->alarm_low_enable == 1 && data->i < def->alarm_low.i ) ||
      ( def->alarm_high_enable == 1 && data->i > def->alarm_high.i) ) return 1;
      return 0;
    default:
      return 0;
  };
};

void draw_h_progressbar(gauge_t *g) {
  aldl_define_t *def = &aldl->def[g->data_a];
  float data = 0;
  switch(def->type) {
    case ALDL_INT:
    case ALDL_BOOL:
      data = clamp_int(g->bottom,g->top,rec->data[g->data_a].i);
      break;
    case ALDL_FLOAT:
      data = clamp_float(g->bottom,g->top,smooth_float(g));
      break;
    default:
      break;
  };

  int x;
  char *curs;

  /* get rh text width */
  int width_rhtext = sprintf(bigbuf,"] %.0f",g->top);
  #ifdef CONSOLEIF_UOM
  width_rhtext += sprintf(bigbuf,"%s",def->uom);
  #endif

  /* print LH text */
  int width_lhtext = sprintf(bigbuf,"%s [",def->name);
  
  curs = bigbuf + width_lhtext; /* set cursor after initial text */
  int pbwidth = g->width - width_lhtext - width_rhtext;
  int filled = data / ( g->top / pbwidth );
  int remainder = pbwidth - filled;

  /* draw progress bar content */
  for(x=0;x<filled;x++) { /* filled section */
    curs[0] = '*';
    curs++;
  };
  for(x=0;x<remainder;x++) { /* unfilled section */
    curs[0] = ' ';
    curs++;
  };
  #ifdef CONSOLEIF_UOM
  sprintf(curs,"] %.0f%s",data,def->uom);
  #else
  sprintf(curs,"] %.0f",data);
  #endif
  move(g->y,g->x); 
  gauge_blank(g);

  if(alarm_range(g) == 1) attron(COLOR_PAIR(RED_ON_BLACK));
  mvaddstr(g->y,g->x,bigbuf);
  if(alarm_range(g) == 1) attroff(COLOR_PAIR(RED_ON_BLACK));
};

void gauge_blank(gauge_t *g) {
  move(g->y,g->x);
  int x;
  for(x=0;x<g->width;x++) addch(' ');
};

/*---- * LOAD CONFIG *--------------------------- */

consoleif_conf_t *consoleif_load_config(aldl_conf_t *aldl) {
  consoleif_conf_t *conf = malloc(sizeof(consoleif_conf_t));
  if(aldl->consoleif_config == NULL) fatalerror(ERROR_CONFIG,
                       "no consoleif config specified");
  conf->dconf = dfile_load(aldl->consoleif_config);
  if(conf->dconf == NULL) fatalerror(ERROR_CONFIG,
                       "consoleif config file missing");
  dfile_t *config = conf->dconf;
  /* GLOBAL OPTIONS */
  conf->n_gauges = configopt_int_fatal(config,"N_GAUGES",1,99999);
  conf->statusbar = configopt_int(config,"STATUSBAR",0,1,0);
  conf->delay = configopt_int(config,"DELAY",0,65535,0);
  /* PER GAUGE OPTIONS */
  conf->gauge = malloc(sizeof(gauge_t) * conf->n_gauges);
  gauge_t *gauge;
  char *idstring = NULL;
  int n;
  for(n=0;n<conf->n_gauges;n++) {
    gauge = &conf->gauge[n]; 
    idstring = configopt(config,gconfig("A_NAME",n),NULL);
    if(idstring != NULL) { /* A_NAME is present */
      gauge->data_a = get_index_by_name(aldl,idstring);
      if(gauge->data_a == -1) fatalerror(ERROR_CONFIG,
                         "consoleif: gauge %i invalid name %s",n,idstring);
    } else {
      fatalerror(ERROR_CONFIG,"consoleif: name missing from %i",n);
    };
    gauge->x = configopt_int_fatal(config,gconfig("X",n),0,10000);
    gauge->y = configopt_int_fatal(config,gconfig("Y",n),0,10000);
    gauge->width = configopt_int(config,gconfig("WIDTH",n),0,10000,30);
    gauge->height = configopt_int(config,gconfig("HEIGHT",n),0,10000,1);
    gauge->bottom = configopt_float(config,gconfig("MIN",n),0);
    gauge->top = configopt_float(config,gconfig("MAX",n),65535);
    gauge->smoothing = configopt_int(config,gconfig("SMOOTHING",n),0,1000,0);
    if(gauge->smoothing > aldl->bufstart - 1) {
      fatalerror(ERROR_BUFFER,"gauge %i has its smoothing setting too high\n\
                  gauge smoothing %i, but prebuffer is %i\n\
              please decrease smoothing or increase prebuffer.",
                 n,gauge->smoothing,aldl->bufstart);
    };
    gauge->weight = configopt_int(config,gconfig("WEIGHT",n),0,500,0);
    /* TYPE SELECTOR */
    char *gtypestr = configopt_fatal(config,gconfig("TYPE",n));
    if(faststrcmp(gtypestr,"HBAR") == 1) {
      gauge->gaugetype = GAUGE_HBAR;
    } else if(faststrcmp(gtypestr,"TEXT") == 1) {
      gauge->gaugetype = GAUGE_TEXT;
    } else if(faststrcmp(gtypestr,"BIN") == 1) {
      gauge->gaugetype = GAUGE_BIN;
    } else if(faststrcmp(gtypestr,"ERRSTR") == 1) {
      gauge->gaugetype = GAUGE_ERRSTR;
    } else if(faststrcmp(gtypestr,"SHIFT") == 1) {
      gauge->gaugetype = GAUGE_SHIFT;
      int gear = 1;
      char *gearstring = malloc(20);
      for(gear=1;gear<=N_GEARS;gear++) {
        sprintf(gearstring,"GEAR%i",gear);
        geartable[gear] = configopt_float_fatal(config,gearstring);
      };
      free(gearstring);
    } else {
      fatalerror(ERROR_CONFIG,"consoleif: gauge %i bad type %s",n,gtypestr);
    };
  };
  return conf;
};

char *gconfig(char *parameter, int n) {
  sprintf(bigbuf,"G%i.%s",n,parameter);
  return bigbuf;
};

float smooth_float(gauge_t *g) {
  if(g->smoothing == 0) return rec->data[g->data_a].f;
  int x;
  aldl_record_t *r = rec;
  float avg = 0;
  for(x=0;x<=g->smoothing;x++) {
    avg += r->data[g->data_a].f; 
    if(r->prev == NULL) { /* attempt to trap underrun (might not work) */
      fatalerror(ERROR_BUFFER,"buffer underrun caught in %s gauge\n\
           %i smoothing w/ %i total buffer, and %i prebuffer.\n\
            please decrease smoothing or increase prebuffer settings!!",
         aldl->def[g->data_a].name,g->smoothing,aldl->bufsize,aldl->bufstart);
    };
    r = r->prev;
  };
  avg += rec->data[g->data_a].f * g->weight;
  return avg / ( g->smoothing + g->weight + 1 );
};

void consoleif_exit() {
  endwin();
}

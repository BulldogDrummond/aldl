#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>
#include <pthread.h>
#include <string.h>

#include "../error.h"
#include "../aldl-io.h"
#include "../config.h"

enum {
  RED_ON_BLACK = 1,
  BLACK_ON_RED = 2,
  GREEN_ON_BLACK = 3,
  CYAN_ON_BLACK = 4,
  WHITE_ON_BLACK = 5
};

typedef struct _gauge {
  int x, y; /* coords */
  int width, height; /* size */
  int data_a, data_b; /* assoc. data index */
  aldl_data_t prev_a, prev_b; /* prev. value */
  float bottom, top; /* bottom and top of a graph */
} gauge_t;

#define COLOR_STATUSSCREEN RED_ON_BLACK
#define COLOR_PROGRESSBAR RED_ON_BLACK

/* --- variables ---------------------------- */

int w_height, w_width; /* width and height of window */

aldl_conf_t *aldl; /* global pointer to aldl conf struct */

char *bigbuf; /* a large temporary string construction buffer */

aldl_record_t *rec; /* current record */

/* --- local functions ------------------------*/

/* center half-width of an element on the screen */
int xcenter(int width);
int ycenter(int height);
 
/* print a centered string */
void print_centered_string(char *str);
void statusmessage(char *str);

/* clear screen and display waiting for connection messages */
void cons_wait_for_connection();

void draw_h_progressbar(gauge_t *g);

/* --------------------------------------------*/

void *consoleif(void *aldl_in) {
  aldl = (aldl_conf_t *)aldl_in;
  bigbuf = malloc(128);

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

  /* get initial screen size */
  getmaxyx(stdscr,w_height,w_width);

  cons_wait_for_connection();

  rec = newest_record(aldl);

  int rpmid = get_index_by_name(aldl,"NEWRFPRT");
  if(rpmid == -1) fatalerror(ERROR_NULL,"rpm not found");

  gauge_t *demogauge = malloc(sizeof(gauge_t));
  demogauge->x = 1;
  demogauge->y = 1;
  demogauge->width=30;
  demogauge->data_a = get_index_by_name(aldl,"ISESDD");
  demogauge->bottom = 0;
  demogauge->top = 3187.50;

  gauge_t *rpmgauge = malloc(sizeof(gauge_t));
  rpmgauge->x = 1;
  rpmgauge->y = 4;
  rpmgauge->width = 30;
  rpmgauge->data_a = get_index_by_name(aldl,"NTRPMX");
  rpmgauge->bottom = 0;
  rpmgauge->top = 6375;

  while(1) {
    rec = newest_record_wait(aldl,rec);
    if(rec == NULL) { /* disconnected */
      cons_wait_for_connection();
      continue;
    };
    sprintf(bigbuf,"TIMESTAMP: %i",(int)rec->t);
    mvaddstr(ycenter(0)+1,xcenter(strlen(bigbuf)),bigbuf);
    draw_h_progressbar(demogauge);
    draw_h_progressbar(rpmgauge);
    refresh();
    usleep(10000);
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

void statusmessage(char *str) {
  clear();
  attron(COLOR_PAIR(COLOR_STATUSSCREEN));
  print_centered_string(str);
  mvaddstr(1,1,VERSION);
  attroff(COLOR_PAIR(COLOR_STATUSSCREEN));
  refresh();
  usleep(400);
};

void cons_wait_for_connection() {
  statusmessage("Connecting to ECM...");
  pause_until_connected(aldl);

  statusmessage("Buffering...");
  pause_until_buffered(aldl);

  clear();
}

void draw_h_progressbar(gauge_t *g) {
  aldl_define_t *def = &aldl->def[g->data_a];
  float data = rec->data[g->data_a].f;
  /* draw title text */
  mvaddstr(g->y,g->x,def->name);
  /* draw output text */
  sprintf(bigbuf,"%.1f %s",data,def->uom);
  mvaddstr(g->y,g->x + g->width - strlen(bigbuf),bigbuf);
  /* draw progress bar */
  move(g->y + 1, g->x);
  int filled = data / ( g->top / g->width );
  int remainder = g->width - filled;
  int x;
  attron(COLOR_PAIR(COLOR_PROGRESSBAR)); 
  for(x=0;x<filled;x++) { /* draw filled section */
    addch(' '|A_REVERSE);
  }; 
  for(x=0;x<remainder;x++) { /* draw unfilled section */
    addch('-');
  };
  attroff(COLOR_PAIR(COLOR_PROGRESSBAR));
};

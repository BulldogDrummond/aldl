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

#define COLOR_STATUSSCREEN RED_ON_BLACK

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

/* --------------------------------------------*/

void *consoleif(void *aldl_in) {
  aldl = (aldl_conf_t *)aldl_in;
  bigbuf = malloc(128);

  /* initialize root window */
  WINDOW *root;
  if((root = initscr()) == NULL) {
    fatalerror(ERROR_NULL,"could not init ncurses");
  };

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

  while(1) {
    rec = next_record_wait(rec);
    sprintf(bigbuf,"RPM: %f",rec->data[rpmid].f);
    mvaddstr(ycenter(0),xcenter(strlen(bigbuf)),bigbuf);
    refresh();

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

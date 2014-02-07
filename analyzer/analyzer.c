#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>

#include "csv.h"
#include "config.h"

typedef struct _anl_t {
  float low_rpm,high_rpm,avg_rpm;
  float low_map,high_map,avg_map;
  float low_maf,high_maf,avg_maf;
  int low_blm,high_blm;
  float avg_blm;
  int counts;
} anl_t;
anl_t *anl;

int badlines;
int goodlines;
int skiplines;

void err(char *str, ...);
void parse_file(char *data);
char *load_file(char *filename);
void parse_line(char *line);
int verify_line(char *line);

void log_blm(char *line);

int csvint(char *line, int f);
float csvfloat(char *line, int f);

void prep_anl();

void post_calc();

void print_results();

int main(int argc, char **argv) {
  printf("-- aldl log analyzer --\n");
  /* load files ... */
  if(argc < 2) err("no files specified...");
  int x;
  char **log = malloc(sizeof(char) * argc);
  printf("Loading files...");
  for(x=1;x<argc;x++) {
    printf("Loading file %s\n",argv[x]);
    log[x-1] = load_file(argv[x]);
    if(log[x-1] == NULL) err("File %s could not be loaded!",argv[x]);
  };

  prep_anl();

  /* parse */
  for(x=0;x<argc-1;x++) {
    parse_file(log[x]);
  };

  post_calc();

  print_results();

  return 1;
};

void prep_anl() {
  /* alloc anl struct */
  anl = malloc(sizeof(anl_t) * N_CELLS);
  memset(anl,0,sizeof(anl_t) * N_CELLS);

  int cell;
  anl_t *cdata;
  for(cell=0;cell<N_CELLS;cell++) {
    cdata = &anl[cell];    
    cdata->low_map = 999999;
    cdata->low_rpm = 999999;
    cdata->low_blm = 999999;
    cdata->low_maf = 999999;
  };
  goodlines = 0;
  badlines = 0;
  skiplines = 0;
};

void parse_file(char *data) {
  printf("Parsing data...\n");
  int ln = 1; /* line number (skip header) */
  char *line = line_start(data,ln); /* initial line pointer */
  while(line != NULL) { /* loop for all lines */
    parse_line(line);
    ln++;
    line = line_start(data,ln);
  };
};

void parse_line(char *line) {
  /* verify line integrity */
  if(verify_line(line) == 0) return;

  /* check timestamp minimum */
  if(csvint(line,COL_TIMESTAMP) < MIN_TIME) {
    skiplines++;
    return;
  };

  /* bring in blm stuff */
  log_blm(line);
};

void log_blm(char *line) {
  /* get cell number and confirm it's in range */
  int cell = csvint(line,COL_CELL);
  if(cell < 0 || cell >= N_CELLS) {
    skiplines++;
    return;
  };

  /* check temperature minimum */
  if(csvfloat(line,COL_TEMP) < MIN_TEMP) {
    skiplines++;
    return;
  };

  /* check CL/PE op */
  if(csvint(line,COL_CL) != 1) return;
  if(csvint(line,COL_BLM) != 1) return;
  if(csvint(line,COL_WOT) == 1) return;

  /* point to cell index */
  anl_t *cdata = &anl[cell];

  /* update counts */
  cdata->counts++;

  /* update blm */
  int blm = (csvint(line,COL_LBLM) + csvint(line,COL_RBLM)) / 2;
  cdata->avg_blm += blm; /* will div for avg later */
  if(blm < cdata->low_blm) cdata->low_blm = blm;
  if(blm > cdata->high_blm) cdata->high_blm = blm;

  /* update map */
  float map = csvfloat(line,COL_MAP);
  if(map < cdata->low_map) cdata->low_map = map;
  if(map > cdata->high_map) cdata->high_map = map;
  cdata->avg_map += map;

  /* update maf */
  float maf = csvfloat(line,COL_MAF);
  if(maf < cdata->low_maf) cdata->low_maf = maf;
  if(maf > cdata->high_maf) cdata->high_maf = maf;
  cdata->avg_maf += maf;

  /* update rpm */
  float rpm = csvfloat(line,COL_RPM);
  if(rpm < cdata->low_rpm) cdata->low_rpm = rpm;
  if(rpm > cdata->high_rpm) cdata->high_rpm = rpm;
  cdata->avg_rpm += rpm;
};

void post_calc() {
  int cell;
  anl_t *cdata;
  for(cell=0;cell<N_CELLS;cell++) {
    cdata = &anl[cell]; /* ptr to cell */
    cdata->avg_rpm = cdata->avg_rpm / cdata->counts;  
    cdata->avg_maf = cdata->avg_maf / cdata->counts;
    cdata->avg_map = cdata->avg_map / cdata->counts;
    cdata->avg_rpm = cdata->avg_rpm / cdata->counts;
  };
};

void print_results() {
  int x;
  anl_t *cdata;
  printf("Parsed %i/%i lines.\n",goodlines-badlines,goodlines+badlines);
  printf("Skipped %i/%i unreliable records.\n",skiplines,goodlines);
  for(x=0;x<N_CELLS;x++) {
    cdata = &anl[x]; 
    if(cdata->counts > MIN_COUNTS) {
      printf("\n* Cell %i (%i Hits)\n",x,cdata->counts);
      printf("\tBLM: %i - %i (Avg %.1f)\n",
              cdata->low_blm,cdata->high_blm,cdata->avg_blm);
      printf("\tRPM: %.1f - %.1f RPM (Avg %.1f)\n",
              cdata->low_rpm,cdata->high_rpm,cdata->avg_rpm);
      printf("\tMAP: %.1f - %.1f KPA, (Avg %.1f)\n",
              cdata->low_map,cdata->high_map,cdata->avg_map);
      printf("\tMAF: %.1f - %.1f AFGS, (Avg %.1f)\n",
              cdata->low_maf,cdata->high_maf,cdata->avg_maf);
    };
  };
};

int verify_line(char *line) {
  if(line == NULL) return 0;

  /* check for too many columns */
  if(field_start(line,LAST_COL) == NULL) {
    badlines++;
    return 0;
  };

  /* check for not enough columns */
  if(field_start(line,LAST_COL + 1) != NULL) {
    badlines++;
    return 0;
  };

  goodlines++;
  return 1;
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
  if(fread(buf,1,flength,fdesc) != flength) return NULL;
  fclose(fdesc);
  buf[flength] = 0;
  return buf;
};

void err(char *str, ...) {
  va_list arg;
  if(str != NULL) {
    fprintf(stderr,"ERROR: ");
    va_start(arg,str);
    vfprintf(stderr,str,arg);
    va_end(arg);
    fprintf(stderr,"\n");
  };
  exit(1);
};

int csvint(char *line, int f) {
  return csv_get_int(field_start(line,f));
};

float csvfloat(char *line, int f) {
  return csv_get_float(field_start(line,f));
};


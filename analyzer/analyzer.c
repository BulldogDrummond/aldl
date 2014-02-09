#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>

#include "csv.h"
#include "config.h"
#include "loadconfig.h"
#include "error.h"

typedef struct _anl_t {
  float low_rpm,high_rpm,avg_rpm;
  float low_map,high_map,avg_map;
  float low_maf,high_maf,avg_maf;
  float low_blm,high_blm,avg_blm;
  int counts;
} anl_t;
anl_t *anl;

typedef struct _anl_conf_t {
  int n_cols; /* number of columns in a log file */
  /* valid row specifier */
  int valid_min_time; /* minimum timestamp */
  int valid_min_temp; /* minimum temperature */
  int valid_cl,valid_blm,valid_wot;
  /* blm analyzer */
  int blm_on; /* activate the blm analyzer */
  int blm_n_cells; /* number of blm cells */
  int blm_min_count; /* minimum number of counts for a valid cell */
  /* column identifiers */ 
  int col_timestamp, col_rpm, col_temp, col_lblm, col_rblm, col_cell;
  int col_map, col_maf, col_cl, col_blm, col_wot;
} anl_conf_t;
anl_conf_t *anl_conf;

int badlines;
int goodlines;
int skiplines;

void parse_file(char *data);
void parse_line(char *line);
int verify_line(char *line);

void log_blm(char *line);

int csvint(char *line, int f);
float csvfloat(char *line, int f);

void prep_anl();

void anl_load_conf(char *filename);

void post_calc();

void print_results();

int main(int argc, char **argv) {
  printf("-- aldl log analyzer --\n");
  /* load config */
  anl_load_conf("analyzer.conf");
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
  float blm = (csvfloat(line,COL_LBLM) + csvfloat(line,COL_RBLM)) / 2;
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
    cdata->avg_blm = cdata->avg_blm / (float)cdata->counts;  
    cdata->avg_maf = cdata->avg_maf / (float)cdata->counts;
    cdata->avg_map = cdata->avg_map / (float)cdata->counts;
    cdata->avg_rpm = cdata->avg_rpm / (float)cdata->counts;
  };
};

void print_results() {
  int x;
  anl_t *cdata;
  printf("Parsed %i/%i lines.\n",goodlines-badlines,goodlines+badlines);
  printf("Skipped %i/%i unreliable records.\n",skiplines,goodlines);
  float overall_blm_avg = 0;
  float overall_blm_count = 0;
  for(x=0;x<N_CELLS;x++) {
    cdata = &anl[x]; 
    if(cdata->counts > MIN_COUNTS) {
      overall_blm_count++;
      overall_blm_avg += cdata->avg_blm;
      printf("\n* Cell %i (%i Hits)\n",x,cdata->counts);
      printf("\tBLM: %.1f - %.1f (Avg %.1f)\n",
              cdata->low_blm,cdata->high_blm,cdata->avg_blm);
      printf("\tRPM: %.1f - %.1f RPM (Avg %.1f)\n",
              cdata->low_rpm,cdata->high_rpm,cdata->avg_rpm);
      printf("\tMAP: %.1f - %.1f KPA, (Avg %.1f)\n",
              cdata->low_map,cdata->high_map,cdata->avg_map);
      printf("\tMAF: %.1f - %.1f AFGS, (Avg %.1f)\n",
              cdata->low_maf,cdata->high_maf,cdata->avg_maf);
    };
  };

  printf("\nOverall useful BLM Average: %f (%.3f percent)\n",
        overall_blm_avg / overall_blm_count,
        ( overall_blm_avg / overall_blm_count) / 128);
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

int csvint(char *line, int f) {
  return csv_get_int(field_start(line,f));
};

float csvfloat(char *line, int f) {
  return csv_get_float(field_start(line,f));
};

void anl_load_conf(char *filename) {
  anl_conf = malloc(sizeof(anl_conf_t));
  dfile_t *dconf;
  dconf = dfile_load(filename);
  if(dconf == NULL) err("Couldn't load config %s",filename);
  anl_conf->n_cols = configopt_int_fatal(dconf,"N_COLS",1,500);
  anl_conf->valid_min_time = configopt_int_fatal(dconf,"MIN_TIME",0,999999);
  anl_conf->valid_min_temp  = configopt_int_fatal(dconf,"MIN_TEMP",-20,99999);
  anl_conf->valid_cl  = configopt_int_fatal(dconf,"VALID_CL",0,1);
  anl_conf->valid_blm  = configopt_int_fatal(dconf,"VALID_BLM",0,1);
  anl_conf->valid_wot = configopt_int_fatal(dconf,"VALID_WOT",0,1);
  anl_conf->blm_on = configopt_int_fatal(dconf,"BLM_ON",0,1);
  anl_conf->blm_n_cells = configopt_int_fatal(dconf,"N_CELLS",1,255);
  anl_conf->blm_min_count = configopt_int_fatal(dconf,"BLM_MIN_COUNTS",0,10000);
  anl_conf->col_timestamp = configopt_int_fatal(dconf,"COL_TIMESTAMP",0,500);
  anl_conf->col_rpm = configopt_int_fatal(dconf,"COL_RPM",0,500);
  anl_conf->col_temp = configopt_int_fatal(dconf,"COL_TEMP",0,500);
  anl_conf->col_lblm = configopt_int_fatal(dconf,"COL_LBLM",0,500);
  anl_conf->col_cell = configopt_int_fatal(dconf,"COL_CELL",0,500);
  anl_conf->col_rblm = configopt_int_fatal(dconf,"COL_RBLM",0,500);
  anl_conf->col_map = configopt_int_fatal(dconf,"COL_MAP",0,500);
  anl_conf->col_maf = configopt_int_fatal(dconf,"COL_MAF",0,500);
  anl_conf->col_cl = configopt_int_fatal(dconf,"COL_CL",0,500);
  anl_conf->col_blm = configopt_int_fatal(dconf,"COL_BLM",0,500);
  anl_conf->col_wot = configopt_int_fatal(dconf,"COL_WOT",0,500);
};

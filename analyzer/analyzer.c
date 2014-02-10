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

#define RPM_GRIDSIZE ( GRID_RPM_RANGE / GRID_RPM_INTERVAL )
#define MAP_GRIDSIZE ( GRID_MAP_RANGE / GRID_MAP_INTERVAL )

/* grid cell, float */
typedef struct _anl_fcell_t {
  float low,high;
  float avg;
  int count;
} anl_fcell_t;

/* grid cell, int */
typedef struct anl_icell_t {
  int low,high;
  float avg;
  int count;
} anl_icell_t;

/* blm analysis storage */
typedef struct _anl_t {
  anl_fcell_t rpm,map,maf;
  anl_icell_t blm; 
  int counts;
} anl_t;
anl_t *anl_blm;

/* knock table */
typedef struct _anl_knock_t {
  int t[RPM_GRIDSIZE][MAP_GRIDSIZE];
  int ttl;
  int prev;
} anl_knock_t;
anl_knock_t *anl_knock;

/* wideband analysis table */
typedef struct _anl_wb_t {
  anl_fcell_t t[RPM_GRIDSIZE][MAP_GRIDSIZE];
} anl_wb_t;
anl_wb_t *anl_wb;

/* configuration storage */
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
  /* knock analyzer */
  int knock_on; /* activate knock counter */
  /* wb analyzer */
  int wb_on,wb_wot,wb_counts;
  /* column identifiers */ 
  int col_timestamp, col_rpm, col_temp, col_lblm, col_rblm, col_cell;
  int col_map, col_maf, col_cl, col_blm, col_wot, col_knock, col_wb;
} anl_conf_t;
anl_conf_t *anl_conf;

typedef struct _anl_stats_t {
  int badlines,goodlines,skiplines;
} anl_stats_t;
anl_stats_t *stats;

void parse_file(char *data);
void parse_line(char *line);
int verify_line(char *line);

int csvint(char *line, int f);
float csvfloat(char *line, int f);

void prep_anl();

void anl_load_conf(char *filename);

void log_blm(char *line);
void log_knock(char *line);
void log_wb(char *line);

void post_calc_blm();
void post_calc_wb();
void post_calc();

void print_results();
void print_results_blm();
void print_results_knock();
void print_results_wb();

int rpm_cell_offset(int value);
int map_cell_offset(int value);

int main(int argc, char **argv) {
  printf("**** aldlio offline log analyzer %s ****\n\n",ANL_VERSION);
  printf("(c)2014 Steve Haslin\n");

  /* load config */
  anl_load_conf(ANL_CONFIGFILE);

  /* load files ... */
  if(argc < 2) err("no files specified...");
  int x;
  char **log = malloc(sizeof(char) * argc);
  printf("Loading files...\n");
  for(x=1;x<argc;x++) {
    printf("Loading file %s\n",argv[x]);
    log[x-1] = load_file(argv[x]);
    if(log[x-1] == NULL) err("File %s could not be loaded!",argv[x]);
  };

  prep_anl(); /* prepare data structures */

  /* parse */
  for(x=0;x<argc-1;x++) {
    parse_file(log[x]);
  };

  post_calc(); /* post-calculations (averaging mostly) */

  print_results();

  return 1;
};

void prep_anl() {
  /* alloc blm anl struct */
  anl_blm = malloc(sizeof(anl_t) * anl_conf->blm_n_cells);
  memset(anl_blm,0,sizeof(anl_t) * anl_conf->blm_n_cells);

  /* config blm struct */
  if(anl_conf->blm_on) {
    int cell;
    anl_t *cdata;
    for(cell=0;cell<anl_conf->blm_n_cells;cell++) {
      cdata = &anl_blm[cell];    
      cdata->map.low = 999999;
      cdata->rpm.low = 999999;
      cdata->blm.low = 999999;
      cdata->maf.low = 999999;
    };
  };

  /* alloc and config stats struct */
  stats = malloc(sizeof(anl_stats_t));
  stats->goodlines = 0;
  stats->badlines = 0;
  stats->skiplines = 0;

  /* config knock struct */
  if(anl_conf->knock_on == 1) {
    anl_knock = malloc(sizeof(anl_knock_t));
    memset(anl_knock,0,sizeof(anl_knock_t));
  };

  /* config wb struct */
  if(anl_conf->wb_on == 1) {
    anl_wb = malloc(sizeof(anl_wb_t));
    memset(anl_wb,0,sizeof(anl_wb_t));
  };
};

void parse_file(char *data) {
  printf("Parsing data...\n");
  char *line = line_start(data,1); /* initial line pointer */
  while(line != NULL) { /* loop for all lines */
    parse_line(line);
    line = line_start(line,1);
  };
};

void parse_line(char *line) {
  /* verify line integrity */
  if(verify_line(line) == 0) return;

  /* check timestamp minimum */
  if(csvint(line,anl_conf->col_timestamp) < anl_conf->valid_min_time) {
    stats->skiplines++;
    return;
  };

  /* bring in blm stuff */
  if(anl_conf->blm_on == 1) log_blm(line);

  /* bring in knock stuff */
  if(anl_conf->knock_on == 1) log_knock(line);

  /* wb */
  if(anl_conf->wb_on == 1) log_wb(line);
};

void log_wb(char *line) {
  /* check temperature minimum */
  if(csvfloat(line,anl_conf->col_temp) < anl_conf->valid_min_temp) {
    stats->skiplines++;
    return;
  };
  int rpmcell = rpm_cell_offset(csvfloat(line,anl_conf->col_rpm));
  int mapcell = map_cell_offset(csvfloat(line,anl_conf->col_map));
  anl_wb->t[rpmcell][mapcell].avg += (csvfloat(line,anl_conf->col_wb));
  anl_wb->t[rpmcell][mapcell].count++;
};

void log_knock(char *line) {
  int kcount = csvint(line,anl_conf->col_knock);
  if(kcount == anl_knock->prev) return; /* no new knock */
  if(kcount < anl_knock->prev) {
    /* either knock counter has rolled, or this is the initial value. */
    /* FIXME we are discarding knock events during rollover ... */
    anl_knock->prev = kcount; 
    return;
  };
  /* at this point a knock event HAS been encountered */
  int rpmcell = rpm_cell_offset(csvfloat(line,anl_conf->col_rpm));
  int mapcell = map_cell_offset(csvfloat(line,anl_conf->col_map));
  anl_knock->t[rpmcell][mapcell] += kcount; /* incr kcount */
  anl_knock->prev = kcount; /* reset prev */
  anl_knock->ttl += kcount; /* incr. ttl */
};

void print_results_knock() {
  printf("\n**** KNOCK Analysis ****\n\n");
  int maprow = 0;
  int rpmrow = 0;
  for(rpmrow=0;rpmrow<RPM_GRIDSIZE;rpmrow++) {
    for(maprow=0;maprow<MAP_GRIDSIZE;maprow++) {
      if(anl_knock->t[rpmrow][maprow] != 0) {
        printf("RPM %i-%i @ MAP%i-%i: %i Counts\n",
          rpmrow * GRID_RPM_INTERVAL, ( rpmrow + 1) * GRID_RPM_INTERVAL,
          maprow * GRID_MAP_INTERVAL, ( maprow + 1) * GRID_MAP_INTERVAL,
          anl_knock->t[rpmrow][maprow]);
      };
    };
  };
  printf("Total knock events: %.0f k\n",(float)anl_knock->ttl / 1000);
};

void log_blm(char *line) {
  /* get cell number and confirm it's in range */
  int cell = csvint(line,anl_conf->col_cell);
  if(cell < 0 || cell >= anl_conf->blm_n_cells) {
    stats->skiplines++;
    return;
  };

  /* check temperature minimum */
  if(csvfloat(line,anl_conf->col_temp) < anl_conf->valid_min_temp) {
    stats->skiplines++;
    return;
  };

  /* check CL/PE op */
  if(csvint(line,anl_conf->col_cl) != 1) return;
  if(csvint(line,anl_conf->col_blm) != 1) return;

  /* point to cell index */
  anl_t *cdata = &anl_blm[cell];

  /* update counts */
  cdata->counts++;

  /* update blm */
  float blm = (csvfloat(line,anl_conf->col_lblm) +
             csvfloat(line,anl_conf->col_rblm)) / 2;
  cdata->blm.avg += blm; /* will div for avg later */
  if(blm < cdata->blm.low) cdata->blm.low = blm;
  if(blm > cdata->blm.high) cdata->blm.high = blm;

  /* update map */
  float map = csvfloat(line,anl_conf->col_map);
  if(map < cdata->map.low) cdata->map.low = map;
  if(map > cdata->map.high) cdata->map.high = map;
  cdata->map.avg += map;

  /* update maf */
  float maf = csvfloat(line,anl_conf->col_maf);
  if(maf < cdata->maf.low) cdata->maf.low = maf;
  if(maf > cdata->maf.high) cdata->maf.high = maf;
  cdata->maf.avg += maf;

  /* update rpm */
  float rpm = csvfloat(line,anl_conf->col_rpm);
  if(rpm < cdata->rpm.low) cdata->rpm.low = rpm;
  if(rpm > cdata->rpm.high) cdata->rpm.high = rpm;
  cdata->rpm.avg += rpm;
};

void post_calc() {
  if(anl_conf->blm_on == 1) post_calc_blm();
  if(anl_conf->wb_on == 1) post_calc_wb();
};

void post_calc_wb() {
  int maprow = 0;
  int rpmrow = 0;
  for(rpmrow=0;rpmrow<RPM_GRIDSIZE;rpmrow++) {
    for(maprow=0;maprow<MAP_GRIDSIZE;maprow++) {
      anl_wb->t[rpmrow][maprow].avg = anl_wb->t[rpmrow][maprow].avg /
                        anl_wb->t[rpmrow][maprow].count;
    };
  };
};

void print_results_wb() {
  int maprow = 0;
  int rpmrow = 0;
  printf("\n**** WB Analysis ****\n");
  for(rpmrow=0;rpmrow<RPM_GRIDSIZE;rpmrow++) {
    for(maprow=0;maprow<MAP_GRIDSIZE;maprow++) {
      if(anl_wb->t[rpmrow][maprow].count > 50) {
        printf("RPM %i-%i @ MAP%i-%i: %.2f   (%i Counts)\n",
        rpmrow * GRID_RPM_INTERVAL, ( rpmrow + 1) * GRID_RPM_INTERVAL,
        maprow * GRID_MAP_INTERVAL, ( maprow + 1) * GRID_MAP_INTERVAL,
        anl_wb->t[rpmrow][maprow].avg, anl_wb->t[rpmrow][maprow].count);
      };
    };
  };
};

void post_calc_blm() {
  int cell;
  anl_t *cdata;
  for(cell=0;cell<anl_conf->blm_n_cells;cell++) {
    cdata = &anl_blm[cell]; /* ptr to cell */
    cdata->blm.avg = cdata->blm.avg / (float)cdata->counts;  
    cdata->maf.avg = cdata->maf.avg / (float)cdata->counts;
    cdata->map.avg = cdata->map.avg / (float)cdata->counts;
    cdata->rpm.avg = cdata->rpm.avg / (float)cdata->counts;
  };
};

void print_results() {
  printf("Parsed %i/%i lines.\n",
      stats->goodlines - stats->badlines,
      stats->goodlines + stats->badlines);
  printf("Skipped %i/%i unreliable records.\n",
      stats->skiplines,stats->goodlines);
  if(anl_conf->blm_on == 1) print_results_blm();
  if(anl_conf->knock_on == 1) print_results_knock();
  if(anl_conf->wb_on == 1) print_results_wb();
};

void print_results_blm() {
  int x;
  anl_t *cdata;
  float overall_blm_avg = 0;
  float overall_blm_count = 0;

  printf("\n**** BLM Analysis ****\n");

  for(x=0;x<anl_conf->blm_n_cells;x++) {
    cdata = &anl_blm[x];
    if(cdata->counts > anl_conf->blm_min_count) {
      overall_blm_count++;
      overall_blm_avg += cdata->blm.avg;
      printf("\n* Cell %i (%i Hits)\n",x,cdata->counts);
      printf("\tBLM: %i - %i (Avg %.1f)\n",
              cdata->blm.low,cdata->blm.high,cdata->blm.avg);
      printf("\tRPM: %.1f - %.1f RPM (Avg %.1f)\n",
              cdata->rpm.low,cdata->rpm.high,cdata->rpm.avg);
      printf("\tMAP: %.1f - %.1f KPA, (Avg %.1f)\n",
              cdata->map.low,cdata->map.high,cdata->map.avg);
      printf("\tMAF: %.1f - %.1f AFGS, (Avg %.1f)\n",
              cdata->maf.low,cdata->maf.high,cdata->maf.avg);
      if(cdata->blm.avg > 138) {
        printf("\t!!!! This cell is tuned too lean !!!!\n");
      };
      if(cdata->blm.avg < 110) {
        printf("\t!!!! This cell is tuned too rich !!!!\n");
      };
    } else {
      printf("\n* Cell %i (%i Hits) - Not reliable.\n",x,cdata->counts);
    };
  };
  printf("\nOverall useful BLM Average: %.2f (%.3f percent)\n",
        overall_blm_avg / overall_blm_count,
        ( overall_blm_avg / overall_blm_count) / 128);
  if(overall_blm_avg / overall_blm_count < 118) {
    printf("!!!! Overall tune too rich !!!!\n");
  };
  if(overall_blm_avg / overall_blm_count > 138) {
    printf("!!!! Overall tune too lean !!!!\n");
  };
};

int verify_line(char *line) {
  if(line == NULL) return 0;

  /* check for too many columns */
  if(field_start(line,anl_conf->n_cols) == NULL) {
    stats->badlines++;
    return 0;
  };

  /* check for not enough columns */
  if(field_start(line,anl_conf->n_cols + 1) != NULL) {
    stats->badlines++;
    return 0;
  };

  stats->goodlines++;
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
  anl_conf->blm_on = configopt_int_fatal(dconf,"BLM_ON",0,1);
  anl_conf->knock_on = configopt_int_fatal(dconf,"KNOCK_ON",0,1);
  if(anl_conf->blm_on == 1) {
    anl_conf->blm_n_cells = configopt_int_fatal(dconf,"N_CELLS",1,255);
    anl_conf->blm_min_count = configopt_int_fatal(dconf,"BLM_MIN_COUNTS",
                                  1,10000);
    anl_conf->col_lblm = configopt_int_fatal(dconf,"COL_LBLM",0,500);
    anl_conf->col_cell = configopt_int_fatal(dconf,"COL_CELL",0,500);
    anl_conf->col_rblm = configopt_int_fatal(dconf,"COL_RBLM",0,500);
  };
  if(anl_conf->knock_on == 1) {
    anl_conf->col_knock = configopt_int_fatal(dconf,"COL_KNOCK",0,500);
  };
  anl_conf->wb_on = configopt_int_fatal(dconf,"WB_ON",0,1);
  if(anl_conf->wb_on == 1) {
    anl_conf->col_wb = configopt_int_fatal(dconf,"COL_WB",0,500);
    anl_conf->wb_wot = configopt_int_fatal(dconf,"WB_WOT",0,1);
    anl_conf->wb_counts = configopt_int_fatal(dconf,"WB_MIN_COUNTS",1,65535);
  };
  anl_conf->col_timestamp = configopt_int_fatal(dconf,"COL_TIMESTAMP",0,500);
  anl_conf->col_rpm = configopt_int_fatal(dconf,"COL_RPM",0,500);
  anl_conf->col_temp = configopt_int_fatal(dconf,"COL_TEMP",0,500);
  anl_conf->col_map = configopt_int_fatal(dconf,"COL_MAP",0,500);
  anl_conf->col_maf = configopt_int_fatal(dconf,"COL_MAF",0,500);
  anl_conf->col_cl = configopt_int_fatal(dconf,"COL_CL",0,500);
  anl_conf->col_blm = configopt_int_fatal(dconf,"COL_BLM",0,500);
  anl_conf->col_wot = configopt_int_fatal(dconf,"COL_WOT",0,500);
};

int rpm_cell_offset(int value) {
  if(value > GRID_RPM_RANGE) return GRID_RPM_RANGE / GRID_RPM_INTERVAL;
  if(value < 0) return 0;
  if(value < GRID_RPM_INTERVAL) return 0;
  int cell = ((float)value/GRID_RPM_RANGE)*(GRID_RPM_RANGE/GRID_RPM_INTERVAL);
  return cell;
};

int map_cell_offset(int value) {
  if(value > GRID_MAP_RANGE) return GRID_MAP_RANGE / GRID_MAP_INTERVAL;
  if(value < 0) return value;
  if(value < GRID_MAP_INTERVAL) return 0;
  int cell = ((float)value/GRID_MAP_RANGE)*(GRID_MAP_RANGE/GRID_MAP_INTERVAL);
  return cell;
};


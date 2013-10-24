#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <limits.h>

/* local objects */
#include "loadconfig.h"
#include "config.h"
#include "error.h"
#include "aldl-io.h"
#include "useful.h"

/* ------- GLOBAL----------------------- */

aldl_conf_t *aldl; /* aldl data structure */
aldl_commdef_t *comm; /* comm specs */
dfile_t *config; /* configuration */

/* ------- LOCAL FUNCTIONS ------------- */

/* is a char whitespace ...? */
inline int is_whitespace(char ch);

/* copy data contained in field f of d to dst, delimted by start and end */
char *brk_field(char *dst, int f, char *in);

/* following functions use internally stored config and should never be
   exported ... */

/* get various config options by name */
int configopt_int_fatal(char *str, int min, int max);
int configopt_int(char *str, int min, int max, int def);
byte configopt_byte(char *str, byte def);
byte configopt_byte_fatal(char *str);
float configopt_float(char *str, float def);
float configopt_float_fatal(char *str);
char *configopt_fatal(char *str);
char *configopt(char *str,char *def);

/* get a packet config string */
char *pktconfig(char *buf, char *parameter, int n);
char *dconfig(char *buf, char *parameter, int n);

/* initial memory allocation routines */
void aldl_alloc_a(); /* fixed structures */
void aldl_alloc_b(); /* definition arrays */
void aldl_alloc_c(); /* more data space */

/* config file loading */
void load_config_a(); /* load data to alloc_a structures */
void load_config_b(); /* load data to alloc_b structures */
void load_config_c();
char *load_config_root(); /* returns path to sub config */

aldl_conf_t *aldl_setup() {
  /* load root config file ... */
  config = dfile_load(ROOT_CONFIG_FILE);
  if(config == NULL) fatalerror(ERROR_CONFIG,"cant load config file");
  #ifdef DEBUGCONFIG
  print_config(config);
  #endif

  /* allocate main (predictable) structures */
  aldl_alloc_a(); /* creates aldl_conf_t structure ... */
  #ifdef DEBUGCONFIG
  printf("loading root config...\n");
  #endif

  char *configfile = load_config_root();

  /* load def config file ... */
  config = dfile_load(configfile);
  if(config == NULL) fatalerror(ERROR_CONFIG,"cant load definition file");
  #ifdef DEBUGCONFIG
  print_config(config);
  printf("configuration, stage A...\n");
  #endif
  load_config_a();

  #ifdef DEBUGCONFIG
  printf("configuration, stage B...\n");
  #endif
  aldl_alloc_b();
  load_config_b();
  #ifdef DEBUGCONFIG
  printf("configuration, stage C...\n");
  #endif
  aldl_alloc_c();
  load_config_c();
  #ifdef DEBUGCONFIG
  printf("configuration complete.\n");
  #endif
  return aldl;
}

void aldl_alloc_a() {
  /* primary aldl configuration structure */
  aldl = malloc(sizeof(aldl_conf_t));
  if(aldl == NULL) fatalerror(ERROR_MEMORY,"conf_t alloc");
  memset(aldl,0,sizeof(aldl_conf_t));

  #ifdef DEBUGMEM
  printf("aldl_conf_t: %i bytes\n",(int)sizeof(aldl_conf_t));
  #endif

  /* communication definition */
  comm = malloc(sizeof(aldl_commdef_t));
  if(comm == NULL) fatalerror(ERROR_MEMORY,"commdef alloc");
  memset(comm,0,sizeof(aldl_commdef_t));
  aldl->comm = comm; /* link to conf */

  #ifdef DEBUGMEM
  printf("aldl_commdef_t: %i bytes\n",(int)sizeof(aldl_commdef_t));
  #endif

  /* stats tracking structure */
  aldl->stats = malloc(sizeof(aldl_stats_t));
  if(aldl->stats == NULL) fatalerror(ERROR_MEMORY,"stats alloc");
  memset(aldl->stats,0,sizeof(aldl_stats_t));

  #ifdef DEBUGMEM
  printf("aldl_stats_t: %i bytes\n",(int)sizeof(aldl_stats_t));
  #endif

}

char *load_config_root() {
  aldl->serialstr = configopt("PORT",NULL);
  aldl->bufsize = configopt_int("BUFFER",10,10000,200);
  aldl->bufstart = configopt_int("START",10,10000,aldl->bufsize / 2);
  aldl->minmax = configopt_int("MINMAX",0,1,1);
  aldl->maxfail = configopt_int("MAXFAIL",1,1000,6);
  return configopt_fatal("DEFINITION"); /* path not stored ... */
};

void load_config_a() {
  comm->checksum_enable = configopt_int("CHECKSUM_ENABLE",0,1,1);;
  comm->pcm_address = configopt_byte_fatal("PCM_ADDRESS");
  comm->idledelay = configopt_int("IDLE_DELAY",0,5000,10);
  comm->chatterwait = configopt_int("IDLE_ENABLE",0,1,1);
  comm->shutupcommand = generate_mode(configopt_byte_fatal("SHUTUP_MODE"),comm);
  comm->returncommand = generate_mode(configopt_byte_fatal("RETURN_MODE"),comm);
  comm->shutuprepeat = configopt_int("SHUTUP_REPEAT",0,5000,1);
  comm->shutuprepeatdelay = configopt_int("SHUTUP_DELAY",0,5000,75);
  comm->n_packets = configopt_int("N_PACKETS",1,99,1);
  aldl->shutup_time = configopt_int("SHUTUP_TIME",10,50000,2500);
  aldl->n_defs = configopt_int_fatal("N_DEFS",1,512);
}

void aldl_alloc_b() {
  /* allocate space to store packet definitions */
  comm->packet = malloc(sizeof(aldl_packetdef_t) * comm->n_packets);
  if(comm->packet == NULL) fatalerror(ERROR_MEMORY,"packet mem");

  #ifdef DEBUGMEM
  printf("aldl_commdef_t: %i bytes\n",(int)sizeof(aldl_commdef_t));
  #endif
}

void load_config_b() {
  int x;
  char *pktname = malloc(50);
  for(x=0;x<comm->n_packets;x++) {
    comm->packet[x].id = configopt_byte_fatal(pktconfig(pktname,"ID",x));
    comm->packet[x].length = configopt_int_fatal(pktconfig(pktname,
                                                "SIZE",x),1,255);
    comm->packet[x].offset = configopt_int(pktconfig(pktname,
                                                 "OFFSET",x),0,254,3);
    comm->packet[x].frequency = configopt_int(pktconfig(pktname,
                                                 "FREQUENCY",x),0,1000,1);
    generate_pktcommand(&comm->packet[x],comm);
    #ifdef DEBUGCONFIG
    printf("loaded packet %i\n",x);
    #endif
  };
  free(pktname);

  /* sanity checks for single packet mode */
  #ifndef ALDL_MULTIPACKET
  if(comm->packet[0].frequency == 0) {
    fatalerror(ERROR_CONFIG,"the only packet is disabled");
  };
  if(comm->n_packets != 1) {
    fatalerror(ERROR_CONFIG,"this config requires multipacket capabilities");
  };
  #endif
}

void aldl_alloc_c() {
  /* storage for raw packet data */
  int x = 0;
  for(x=0;x<comm->n_packets;x++) {
    comm->packet[x].data = malloc(comm->packet[x].length);
    #ifdef DEBUGMEM
    printf("packet %i raw storage: %i bytes\n",x,comm->packet[x].length);
    #endif
    if(comm->packet[x].data == NULL) fatalerror(ERROR_MEMORY,"pkt data");
  };

  /* storage for data definitions */
  aldl->def = malloc(sizeof(aldl_define_t) * aldl->n_defs);
  if(aldl->def == NULL) fatalerror(ERROR_MEMORY,"definition");
  #ifdef DEBUGMEM
  printf("aldl_define_t definition storage: %i bytes\n",
              (int)sizeof(aldl_define_t) * aldl->n_defs);
  #endif
};

void load_config_c() {
  int x=0;
  char *configstr = malloc(50);
  char *tmp;
  aldl_define_t *d;
  int z;

  for(x=0;x<aldl->n_defs;x++) {
    d = &aldl->def[x]; /* shortcut to def */
    tmp=configopt_fatal(dconfig(configstr,"TYPE",x));
    if(faststrcmp(tmp,"BINARY") == 1) {
      d->type=ALDL_BOOL;
      d->binary=configopt_int_fatal(dconfig(configstr,"BINARY",x),0,7);
      d->invert=configopt_int(dconfig(configstr,"INVERT",x),0,1,0);
    } else {
      if(faststrcmp(tmp,"FLOAT") == 1) {
        d->type=ALDL_FLOAT;
        d->precision=configopt_int(dconfig(configstr,"PRECISION",x),0,1000,0);
        d->min.f=configopt_float(dconfig(configstr,"MIN",x),0);
        d->max.f=configopt_float(dconfig(configstr,"MAX",x),9999999);
        d->adder.f=configopt_float(dconfig(configstr,"ADDER",x),0);
        d->multiplier.f=configopt_float(dconfig(configstr,"MULTIPLIER",x),1);
      } else if(faststrcmp(tmp,"INT") == 1) {
        d->type=ALDL_INT; 
        d->min.i=configopt_int(dconfig(configstr,"MIN",x),-32678,32767,0);
        d->max.i=configopt_int(dconfig(configstr,"MAX",x),-32678,32767,65535);
        d->adder.i=configopt_int(dconfig(configstr,"ADDER",x),-32678,32767,0);
        d->multiplier.i=configopt_int(dconfig(configstr,"MULTIPLIER",x),
                                         -32678,32767,1);
      } else {
        fatalerror(ERROR_CONFIG,"invalid data type in def");
      };
      d->uom=configopt(dconfig(configstr,"UOM",x),NULL);
      d->size=configopt_int(dconfig(configstr,"SIZE",x),1,32,8);     
      /* FIXME no support for signed input type */
    };
    d->id=configopt_int(dconfig(configstr,"ID",x),0,32767,x);
    for(z=x-1;z>=0;z--) { /* check for duplicate unique id */
      if(aldl->def[z].id == d->id) fatalerror(ERROR_CONFIG,"duplicate id");
    };
    d->offset=configopt_byte_fatal(dconfig(configstr,"OFFSET",x));
    d->packet=configopt_byte(dconfig(configstr,"PACKET",x),0x00);
    if(d->packet > comm->n_packets - 1) fatalerror(ERROR_CONFIG,"pkt range");
    d->name=configopt_fatal(dconfig(configstr,"NAME",x));
    d->description=configopt_fatal(dconfig(configstr,"DESC",x));
    #ifdef DEBUGCONFIG
    printf("loaded definition %i\n",x);
    #endif
  };
  free(configstr);
}

char *configopt_fatal(char *str) {
  char *val = configopt(str,NULL);
  if(val == NULL) fatalerror(ERROR_CONFIG_MISSING,str);
  return val;
};

char *configopt(char *str,char *def) {
  char *val = value_by_parameter(str, config);
  if(val == NULL) return def;
  return val;
};

float configopt_float(char *str, float def) {
  char *in = configopt(str,NULL);
  #ifdef DEBUGCONFIG
  if(in == NULL) {
    printf("caught default value for %s: %f\n",str,def);
    return def;
  };
  #else
  if(in == NULL) return def;
  #endif
  float x = atof(in);
  return x;
};

float configopt_float_fatal(char *str) {
  float x = atof(configopt_fatal(str));
  return x;
};

int configopt_int(char *str, int min, int max, int def) {
  char *in = configopt(str,NULL);
  #ifdef DEBUGCONFIG
  if(in == NULL) {
    printf("caught default value for %s: %i\n",str,def);
    return def;
  };
  #else
  if(in == NULL) return def;
  #endif
  int x = atoi(in);
  if(x < min || x > max) fatalerror(ERROR_RANGE,str);
  return x;
};

int configopt_int_fatal(char *str, int min, int max) {
  int x = atoi(configopt_fatal(str));
  if(x < min || x > max) fatalerror(ERROR_RANGE,str);
  return x;
};

byte configopt_byte(char *str, byte def) {
  char *in = configopt(str,NULL);
  #ifdef DEBUGCONFIG
  if(in == NULL) {
    printf("caught default value for %s: ",str);
    printhexstring(&def,1);
    return def;
  };
  #else
  if(in == NULL) return def;
  #endif
  return hextobyte(in);
};

byte configopt_byte_fatal(char *str) {
  char *in = configopt_fatal(str);
  return hextobyte(in);
};

char *pktconfig(char *buf, char *parameter, int n) {
  sprintf(buf,"P%i.%s",n,parameter);
  return buf;
};

char *dconfig(char *buf, char *parameter, int n) {
  sprintf(buf,"D%i.%s",n,parameter);
  return buf;
};

dfile_t *dfile_load(char *filename) {
  char *data = load_file(filename);
  if(data == NULL) return NULL;
  dfile_t *d = dfile(data);
  dfile_strip_quotes(d);
  dfile_shrink(d);
  free(data);
  return d; 
};

void dfile_strip_quotes(dfile_t *d) {
  int x;
  char *c; /* cursor*/
  for(x=0;x<d->n;x++) {
    if(d->v[x][0] == '"') {
      d->v[x]++;
      c = d->v[x];
      while(*c != '"') c++;
      c[0] = 0;
    };
  };
};

dfile_t *dfile(char *data) {
  /* allocate base structure */
  dfile_t *out = malloc(sizeof(dfile_t));

  /* initial allocation of pointer array */
  out->p = malloc(sizeof(char*) * MAX_PARAMETERS);
  out->v = malloc(sizeof(char*) * MAX_PARAMETERS);
  out->n = 0;

  /* more useful variables */
  char *c; /* operating cursor within data */
  char *cx; /* auxiliary operating cursor */
  char *cz;
  int len = strlen(data);

  for(c=data;c<data+len;c++) { /* iterate through entire data set */
    if(c[0] == '=') {
      if(c == data || c == data + len) continue; /* handle first or last char */
      out->v[out->n] = c + 1;
      c[0] = 0;
      cx = c + 1;
      while(is_whitespace(*cx) != 1) {
        if(*cx == '"') { /* skip quoted string */
          cx++;
          while(cx[0] != '"') {
            if(cx == data + len) continue;
            cx++;
          };
        };
        cx++;
      };
      cx[0] = 0; /* null end */
      cz = cx; /* rememeber end point */
      /* find starting point */
      cx = c - 1;
      while(is_whitespace(*cx) != 1) {
        if(*cx == '"') { /* skip quoted string */
          cx--;
          while(cx[0] != '"') {
            if(cx == data + len) continue;
            cx--;
          };
        };
        cx--;
        if(cx == data) { /* handle case of beginning of file */
          cx--;
          break;
        };
      };
      out->p[out->n] = cx + 1;
      out->n++;
      if(out->n == MAX_PARAMETERS) return out; /* out of space */
      c = cz;
    };
  };
  return out;
};

/* reduce memory footprint */
char *dfile_shrink(dfile_t *d) {
  /* shrink arrays */
  d->p = realloc(d->p,sizeof(char*) * d->n);
  d->v = realloc(d->v,sizeof(char*) * d->n);
  /* calculate total size of new data storage */
  size_t ttl = 0;
  int x;
  for(x=0;x<d->n;x++) {
    ttl += strlen(d->p[x]);
    ttl += strlen(d->v[x]);
    ttl += 2; /* for null terminators */
  };
  ttl++;
  /* move everything and update pointers */
  char *newdata = malloc(ttl); /* new storage */
  char *c = newdata; /* cursor*/
  for(x=0;x<d->n;x++) {
    /* copy parameter */
    strcpy(c,d->p[x]);
    d->p[x] = c;
    c += (strlen(c) + 1);
    /* copy value */
    strcpy(c,d->v[x]);
    d->v[x] = c;
    c += (strlen(c) + 1);
  };
  return newdata;
};


inline int is_whitespace(char ch) {
  if(ch == 0 || ch == ' ' || ch == '\n') return 1;
  return 0;
};

char *brk_field(char *dst, int f, char *in) {
  if(dst == NULL || in == NULL) return NULL;
  char *start = in;
  int x = 0;
  if(f != 0) { /* not first field */
    for(x=0;x<f;x++) {
      while(start[0] != ',') {
        start++;
        if(start[0] == 0 && x < f) return NULL;
      };
      start++;
    };
  };
  strcpy(dst,start);
  /* just terminate the end */
  char *end = dst;
  end++;
  while(end[0] != ',' && end[0] != 0) end++;
  end[0] = 0;
  return dst;
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
  if(buf == NULL) return NULL;
  if(fread(buf,1,flength,fdesc) != flength) return NULL;
  fclose(fdesc);
  buf[flength] = 0;
  return buf;
};

char *value_by_parameter(char *str, dfile_t *d) {
  int x;
  for(x=0;x<d->n;x++) {
    if(faststrcmp(str,d->p[x]) == 1) return d->v[x];
  };
  return NULL;
};

void print_config(dfile_t *d) {
  printf("config list has %i parsed options:-------------\n",d->n);
  int x;
  for(x=0;x<d->n;x++) printf("p(arameter):%s v(alue):%s\n",d->p[x],d->v[x]);
  printf("----------end config\n");
};

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

/* local objects */
#include "config.h"
#include "acquire.h"
#include "error.h"
#include "dfiler.h"
#include "aldl-io/config.h"
#include "aldl-io/aldl-io.h"
#include "configfile/varstore.h"
#include "configfile/configfile.h"

/* TEMP remove me */
/* end of line delimters */
#define EOL '\n'
#define DOSEOL '\r'

/* copy the contents of field number f in *d to dst.  returns NULL if the
   field cannot be located.  start and end define the field delimters,
   or 0 for start/end of line. */
char *brk_get_field(char *dst, char start, char end, int f, char *d);

/* case sensitive string match the first field of every line until a
   match is found, return a pointer to it, or NULL if can't be found. */
char *brk_get_line(char end, char *d, char *str);

/* ------- GLOBAL----------------------- */

aldl_conf_t *aldl; /* aldl data structure */
aldl_commdef_t *comm; /* comm specs */
char *config_file; /* path to config file */

/* ------- LOCAL FUNCTIONS ------------- */

/* run cleanup rountines for aldl and serial crap */
int aldl_finish();

/* allocate all major structures and load config routines */
void aldl_setup(dfile_t *config);

/* initial memory allocation routines */
void aldl_alloc_a(); /* fixed structures */
void aldl_alloc_b(); /* definition arrays */
void aldl_alloc_c(); /* more data space */

/* config file loading */
void load_config_a(dfile_t *config); /* load data to alloc_a structures */
void load_config_b(dfile_t *config); /* load data to alloc_b structures */

int main() {
  /* initialize locking mechanisms */
  init_locks();

  /* parse config file ... never free this structure */
  dfile_t *config = dfile_load("lt1.conf");

  #ifdef VERBLOSITY
  print_config(config);
  #endif

  char *val = value_by_parameter("ECMID", config);
  if(val == NULL) fatalerror(ERROR_CONFIG,"not found");
  printf("%s\n",val);
  exit(1);

  /* allocate structures and parse config data */
  aldl_setup(config);

  set_connstate(ALDL_LOADING,aldl); /* initial connection state */

  /* FIXME this needs to come from load_config or switch to autodetct */
  char *serialport = "d:002/002";
  serial_init(serialport); /* init serial port */

  aldl_acq(aldl); /* start main event loop */

  aldl_finish(comm);

  return 0;
}

void aldl_setup(dfile_t *config) {
  aldl_alloc_a();
  load_config_a(config);
  aldl_alloc_b();
  load_config_b(config);
  aldl_alloc_c();
}

void aldl_alloc_a() {
  /* primary aldl configuration structure */
  aldl = malloc(sizeof(aldl_conf_t));
  if(aldl == NULL) fatalerror(ERROR_MEMORY,"conf_t alloc");
  memset(aldl,0,sizeof(aldl_conf_t));

  /* communication definition */
  comm = malloc(sizeof(aldl_commdef_t));
  if(comm == NULL) fatalerror(ERROR_MEMORY,"commdef alloc");
  memset(comm,0,sizeof(aldl_commdef_t));
  aldl->comm = comm; /* link to conf */

  /* stats tracking structure */
  aldl->stats = malloc(sizeof(aldl_stats_t));
  if(aldl->stats == NULL) fatalerror(ERROR_MEMORY,"stats alloc");
  memset(aldl->stats,0,sizeof(aldl_stats_t));
}

void load_config_a(dfile_t *config) {
  sprintf(comm->ecmstring, "EE");
  comm->checksum_enable = 1;
  comm->pcm_address = 0xF4;
  comm->idledelay = 10;
  comm->chatterwait = 1;
  comm->shutupcommand = generate_shutup(0x56,0x08,comm);
  comm->returncommand = generate_shutup(0x56,0x09,comm);
  comm->shutuprepeat = 3;
  comm->shutuprepeatdelay = 75;
  comm->n_packets = 1;
}

void aldl_alloc_b() {
  /* allocate space to store packet definitions */
  comm->packet = malloc(sizeof(aldl_packetdef_t) * comm->n_packets);
  if(comm->packet == NULL) fatalerror(ERROR_MEMORY,"packet mem");
}

void load_config_b(dfile_t *config) {
  /* a placeholder packet, lt1 msg 0 */
  comm->packet[0].length = 64;
  comm->packet[0].id = 0x00;
  comm->packet[0].msg_len = 0x57;
  comm->packet[0].msg_mode = 0x01;
  comm->packet[0].commandlength = 5;
  comm->packet[0].offset = 3;
  comm->packet[0].frequency = 1;
  generate_pktcommand(&comm->packet[0],comm);

  /* sanity checks for single packet mode */
  #ifndef ALDL_MULTIPACKET
  if(comm->packet[0].frequency == 0) {
    fatalerror(ERROR_CONFIG,"the only packet is disabled");
  };
  if(comm->n_packets != 1) {
    fatalerror(ERROR_CONFIG,"this config requires multipacket capabilities");
  };
  #endif
  
  /* a placeholder packet, lt1 msg 2 */
//  comm->packet[1].length = 57;
//  comm->packet[1].id = 0x02;
//  comm->packet[1].msg_len = 0x57;
//  comm->packet[1].msg_mode = 0x01;
//  comm->packet[1].commandlength = 5;
//  comm->packet[1].offset = 3;
//  comm->packet[1].frequency = 50;
//  generate_pktcommand(&comm->packet[1],comm);

}

void aldl_alloc_c() {
  /* storage for raw packet data */
  int x = 0;
  for(x=0;x<comm->n_packets;x++) {
    comm->packet[x].data = malloc(comm->packet[x].length);
    if(comm->packet[x].data == NULL) fatalerror(ERROR_MEMORY,"pkt data");
  };

  /* storage for data definitions */
  aldl->def = malloc(sizeof(aldl_define_t) * aldl->n_defs);
  if(aldl->def == NULL) fatalerror(ERROR_MEMORY,"definition");

  /* get data definitions here !! */

  /* allocate space for records here ~ */
}

int aldl_finish() {
  serial_close();
  return 0;
}


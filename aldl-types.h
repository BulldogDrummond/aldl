#ifndef ALDLTYPES_H
#define ALDLTYPES_H

/* --------- datatypes ----------------------------------------*/

/* output data type enumeration */

typedef enum aldl_datatype {
  ALDL_INT, ALDL_FLOAT, ALDL_BOOL
} aldl_datatype_t;

/* connection state tracking */

typedef enum aldl_state {
  ALDL_CONNECTED = 0,
  /* states < 11 reserved for connected states */
  ALDL_CONNECTING = 11,
  ALDL_LOADING = 12,
  ALDL_DESYNC = 13,
  ALDL_ERROR = 14,
  ALDL_LAGGY = 15,
  /* states > 50 reserved for special states */
  ALDL_QUIT = 51,
  ALDL_PAUSE = 52
} aldl_state_t;

/* 8-bit chunk of data */

typedef unsigned char byte;

/* definition of a single multi-type data array member. */

typedef union aldl_data {
  float f;
  int i;
} aldl_data_t;

/* each data record will be associated with a static definition.  this will
   contain whatever is necessary to convert a raw stream, as well as interpret
   the converted data. */

typedef struct aldl_define {
  int id;             /* unique identifier for each definition */
  char *name;         /* short name */
  char *description;  /* description of each definition */
  /* ----- output definition -------------------------- */
  aldl_datatype_t type; /* the OUTPUT type */
  char *uom;     /* unit of measure */
  byte precision;       /* floating point display precision.  not used in
                           conversion; only for display plugins. */
  aldl_data_t min, max;  /* the low and high range of OUTPUT value */
  /* ----- conversion ----------------------------------*/
  aldl_data_t adder;          /*  ... */
  aldl_data_t multiplier;     /*  ... */
  /* ----- input definition --------------------------- */
  byte packet; /* selects which packet unique id the data comes from */
  byte offset; /* offset within packet in bytes */
  byte size;   /* size in bits.  8,16,32 only... */
  byte sig;    /* 1 if signed */
  /* binary stuff only */
  byte binary; /* offset in bits.  only works for 1 bit fields */
  byte invert; /* invert (0 means no) */
} aldl_define_t;

/* definition of a record, which is a sequential linked-list type structure,
   used as a container for a snapshot of data. */

typedef struct aldl_record {
  /* WARNING! never traverse the linked list manually, as that would not
     necessarily be thread-safe ... there are functions for that. */
  struct aldl_record *next; /* linked list traversal, newer record or NULL */
  struct aldl_record *prev; /* linked list traversal, older record or NULL */
  unsigned long t;            /* timestamp of the record */
  aldl_data_t *data;   /* pointer to the first data record. */
} aldl_record_t;

/* defines each packet of data and how to retrieve it */

typedef struct aldl_packetdef {
  byte id;         /* message number */
  int length;     /* how long the packet is, overall, including the header */
  byte *command;  /* the command string sent to retrieve the packet */
  int offset;        /* the offset of the data in bytes, aka header size */
  int frequency;    /* retrieval frequency, or 0 to disable packet */
  byte *data;     /* pointer to the raw data buffer */
} aldl_packetdef_t;

/* master definition of a communication spec for an ECM. */

typedef struct aldl_commdef {
  /* ------- config stuff ---------------- */
  int checksum_enable:1;    /* set to 1 to enable checksum verification of
                              packet data. */
  byte pcm_address;        /* the address of the PCM */
  /* ------- idle traffic stuff ---------- */
  int chatterwait;        /* 1 enables chatter checking.  if this is
                              disabled, it'll immediately and constanty
                              send shutup requests. */
  int idledelay;           /* a ms delay at the end of idle traffic, before
                              sending shutup requests, or 0 */
  /* ------- shutup related stuff -------- */
  byte *shutupcommand;     /* the shutup (disable comms) command */
  int shutuprepeat;        /* how many times to repeat a shutup request */
  int shutuprepeatdelay;   /* the delay, in ms, to delay in between requests */
  byte *returncommand;     /* the "return to normal" command */
  /* ------- data packet requests -------- */
  int n_packets;             /* the number of packets of data */
  aldl_packetdef_t *packet;  /* the actual packet definitions */
} aldl_commdef_t;

typedef struct aldl_stats {
  unsigned int packetchecksumfail;
  unsigned int packetheaderfail;
  unsigned int packetrecvtimeout;
  unsigned int failcounter;
  float packetspersecond;
} aldl_stats_t;

/* an info structure defining aldl communications and data mgmt */

typedef struct aldl_conf {
  char ecmtype; /* the type of ecm being read */
  /* settings ------------ */
  char *serialstr; /* string to init serial port */
  int n_defs;   /* number of definitions */
  int bufsize;  /* the minimum number of records to maintain */
  int bufstart; /* start plugins when this many records are present */
  int rate;    /* slow down data collection, in microseconds.  never exceed
                  one second, or connection quality may suffer.. */
  int maxfail; /* maximum packet retrieve fails before it's assumed that the
                  connection is no longer synchronized */
  int minmax;  /* enforce min/max values during conversion */
  int shutup_time; /* time in ms that a shutup state lasts */
  /* structures -----------*/
  aldl_state_t state; /* connection state, do not touch */
  aldl_define_t *def; /* link to the definition set */
  aldl_record_t *r; /* link to the latest record */
  aldl_commdef_t *comm; /* link back to the communication spec */
  aldl_stats_t *stats;   /* statistics */
  time_t uptime;   /* time stamp for acq loop */
  int ready; /* mark this flag when the buffer is full enough */
} aldl_conf_t;

#endif

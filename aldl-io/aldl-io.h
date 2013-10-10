#ifndef ALDLIO_H
#define ALDLIO_H

/* --------- datatypes ----------------------------------------*/

/* datatypes for OUTPUT conversion only. */

typedef enum aldl_datatype {
  ALDL_INT, ALDL_UINT, ALDL_FLOAT, ALDL_BOOL
} aldl_datatype_t;

/* 8-bit chunk of data */

typedef char octet;

/* type for sequence numbers of records, etc */

typedef unsigned long int aldl_seq;

/* each data record will be associated with a static definition.  this will
   contain whatever is necessary to convert a raw stream, as well as interpret
   the converted data. */

typedef struct aldl_define {
  char id[12];           /* unique identifier for each definition.  spaces are
                            forbidden. */
  char description[32];  /* description of each definition */
  int enable;        /* any plugin using the item must set enable=1.
                        under no circumstances should a plugin set this
                        to 0. */
  /* ----- output definition -------------------------- */
  char name[35];        /* name of the definition */
  aldl_datatype_t type; /* the OUTPUT type */
  unsigned int uom;     /* unit of measure */
  octet precision;       /* floating point display precision */
  float min, max;       /* the low and high range of OUTPUT value */
  /* ----- conversion ----------------------------------*/
  float adder;          /*  ... */
  float multiplier;     /*  ... */
  /* ----- input definition --------------------------- */
  octet packet; /* selects which packet unique id the data comes from */
  octet offset; /* offset within packet in bytes */
  octet size;   /* size in bits.  8,16,32 only... */
  octet sig;    /* 1 if signed */
  /* binary stuff only */
  octet binary; /* offset in bits.  only works for 1 bit fields */
  octet invert; /* invert (0 means set) */
} aldl_define_t;

/* definition of a single multi-type data array member. */

typedef union aldl_data {
  float f;
  int i;
  unsigned int u;
} aldl_data_t;

/* definition of a record, which is a sequential linked-list type structure,
   used as a container for a snapshot of data. */

typedef struct aldl_record {
  int lock;            /* semaphore-stype lock for garbage collection */
  struct aldl_record *next; /* linked list traversal, newer record or NULL */
  struct aldl_record *prev; /* linked list traversal, older record or NULL */
  time_t t;            /* timestamp of the record */
  aldl_data_t *data;   /* pointer to the first data record. */
} aldl_record_t;

/* defines each packet of data and how to retrieve it */

typedef struct aldl_packetdef {
  int enable;     /* actually get data for the packet.  if disabled, it's still
                     placed into the dataset, but filled with NULL. */
  int id;         /* a unique id for the packet of data, used for associations
                     with data definitions. */
  int length;     /* how long the packet is, overall, not including the first
                     three bytes, which are header. */
  char *command;  /* the command string sent to retrieve the packet */
  int commandlength; /* length of the command string in bytes */
  int offset;        /* the offset of the actual data in bytes */
  int delay_send; /* wait this long after sending a retrieve request before
                     attempting to get the data.  or 0 works too.. */
  int delay_recv; /* wait this long after recieving data to send a new request,
                     this allows the pcm time to 'get ready'. */
  char *data;     /* pointer to the raw data buffer */
} aldl_packetdef_t;

/* master definition of a communication spec for an ECM. */

typedef struct aldl_commdef {
  char *serialport;        /* serial port init string */
  /* ------- config stuff ---------------- */
  char ecmstring[4];       /* a unique identifying string for the platform */
  int checksum_enable;     /* set to 1 to enable checksum verification */
  char pcm_address;        /* the address of the PCM */
  /* ------- idle traffic stuff ---------- */
  int chatterwait;         /* 1 enables chatter checking.  if this is
                              disabled, it'll immediately and constanty
                              send shutup requests. */
  char *idletraffic;       /* a known portion of idle traffic, or NULL */
  int idledelay;           /* a ms delay at the end of idle traffic, before
                              sending shutup requests, or 0 */
  /* ------- shutup related stuff -------- */
  char *shutupcommand;     /* the shutup (disable comms) command */
  int shutuplength;        /* length of the shutup string */
  int shutupfailwait;      /* how long to wait for shutup reply before fail */
  int shutuptime;          /* time that a shutup lasts, in seconds.  if a
                              request has not been sent in this time, the
                              connection is considered back @ idle */
  int shutupcharlimit;     /* after recieving this many chars, the shutup
                              request has obviously failed. */
  int shutuprepeat;        /* how many times to repeat a shutup request */
  int shutuprepeatdelay;   /* the delay, in ms, to delay in between requests */
  /* ------- data packet requests -------- */
  int n_packets;             /* the number of packets of data */
  aldl_packetdef_t *packet;  /* the actual packet definitions */
} aldl_commdef_t;

/* an info structure defining aldl communications and data mgmt */

typedef struct aldl_conf {
  char ecmtype; /* the type of ecm being read */
  int n;        /* static number of definitions */
  int bufsize;  /* the minimum number of records to maintain */
  aldl_define_t *def; /* link to the definition set */
  aldl_record_t *r; /* link to the latest record */
  aldl_commdef_t *comm; /* link back to the communication spec */
} aldl_conf_t;

/* functions ------------------------------------------- */

/* diagnostic comms ------------------------------*/

int aldl_reconnect(); /* go into diagnostic mode, returns 1 on success */
int aldl_waitforchatter(); /* waits forever for a byte, then bails */

/* serial comms-----------------------------------*/

int serial_init(char *port); /* initalize the serial if by port spec */
void serial_close(); /* close the serial port */

/*------- COMPLETE THESE FUNCTIONS ------------*/

/* get the next record, waiting until one is available.  takes a pointer to the
   current record.  if an error occurs while waiting for a record, returns NULL
   ... */
aldl_record_t *get_next_record(aldl_record_t *r);

/* get an array index for a definition or data chunk by id.  returns -1 if it
   doesn't exist.  this would be a good method to use for interfaces to index
   data, in case the index is re-ordered in between config loads. */
int get_definition_by_id(aldl_conf_t *c, char *id);

/* get type casted and formatted output from a peice of data by its array
   index. */
int get_int_data(aldl_record_t *r, int i);
unsigned int get_uint_data(aldl_record_t *r, int i);
float get_float_data(aldl_record_t *r, int i);
int get_bool_data(aldl_record_t *r, int i);

/* per-record locking */
void set_record_lock(aldl_record_t *r);
void unset_record_lock(aldl_record_t *r);
int get_record_lock(aldl_record_t *r);

/* link a detached record to the stream.  includes maintainance of the
   linked list within the records themselves, and counters. */
void link_record(aldl_conf_t *c, aldl_record_t *r);


#endif

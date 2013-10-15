#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "serio.h"
#include "config.h"
#include "aldl-io.h"

typedef unsigned int sum_t;

/* local functions -----*/
int aldl_shutup(); /* repeatedly attempt to make the ecm shut up */

int aldl_waitforchatter(); /* waits forever for a byte, then bails */

int aldl_timeout(int len); /* figure out a timeout period */

/* sends a request, delays for a calculated time, and waits for an echo.  if
   the request is successful, returns 1, otherwise 0. */
int aldl_request(byte *pkt, int len);

/* read the number of bytes specified, into str.  waits until the correct
   number of bytes were read, and returns 1, or the timeout (in ms)
   has expired, and returns 0. */
inline int read_bytes(byte *str, int bytes, int timeout);

/* the same as serial_read_bytes, but discards the bytes.  useful for
   ignoring a known-length string of bytes. */
inline int skip_bytes(int bytes, int timeout);

/* listen for something.  it must match str exactly, if str is non-null.
   len is the length of str if it is known, or not null terminated. */
int listen_bytes(byte *str, int len, int max, int timeout);

/* sleep for ms milliseconds */
inline void msleep(int ms);

/************ FUNCTIONS **********************/

int aldl_reconnect(aldl_commdef_t *c) {
  #ifdef ALDL_VERBOSE
    printf("attempting to place ecm in diagnostic mode.\n");
  #endif
  /* reconnect runs in an infinite loop for now. */
  while(1) {
    if(c->chatterwait == 1) {
      aldl_waitforchatter(c);
    } else {
      msleep(c->idledelay);
    };
    if(aldl_shutup(c) == 1) return 1;
  };
  return 0;
}

int aldl_waitforchatter(aldl_commdef_t *c) {
  #ifdef ALDL_VERBOSE
    printf("waiting for idle chatter to confirm key is on..\n");
  #endif
  while(skip_bytes(1,50) == 0) msleep(500);
  #ifdef ALDL_VERBOSE
    printf("got idle chatter or something.\n");
  #endif
  msleep(c->idledelay);
  return 1;
}

int aldl_request(byte *pkt, int len) {
  serial_purge();
  serial_write(pkt,len);
  msleep(aldl_timeout(len));
  int result = listen_bytes(pkt,len,len*2,aldl_timeout(len));
  #ifdef ALDL_VERBOSE
    printf("response success: %i\n",result);
  #endif
  return result;
}

int aldl_timeout(int len) {
  int timeout = len + 5;
  if(timeout < SLEEPYTIME) timeout = SLEEPYTIME * 2;
  return(timeout);
}

int aldl_shutup(aldl_commdef_t *c) {
  if(c->shutuprepeat == 0) return 1; /* no shutup necessary */
  int x;
  for(x=1;x<=c->shutuprepeat;x++) {
    if(aldl_request(c->shutupcommand,SHUTUP_LENGTH) == 1) return 1;
  }
  return 0;
}

byte *aldl_get_packet(aldl_packetdef_t *p) {
  if(aldl_request(p->command, p->commandlength) == 0) return NULL;
  /* get actual data */
  if(read_bytes(p->data, p->length, aldl_timeout(p->length)) == 0) {
    /* failed to get data */
    memset(p->data,0,p->length);
    return NULL;
  }
  return p->data;
}

int cmp_bytestring(byte *h, int hsize, byte *n, int nsize) {
  if(nsize > hsize) return 0; /* needle is larger than haystack */
  if(hsize < 1 || nsize < 1) return 0;
  int cursor = 0; /* haystack compare cursor */
  int matched = 0; /* needle compare cursor */
  while(cursor <= hsize) {
    if(nsize == matched) return 1;
    if(h[cursor] != n[matched]) { /* reset match */
      matched = 0;
    } else {
      matched++;
    };
    cursor++;
  };
  return 0;
};

inline int read_bytes(byte *str, int bytes, int timeout) {
  int bytes_read = 0;
  int timespent = 0;
  #ifdef SERIAL_VERBOSE
  printf("**START READ_BYTES %i bytes %i timeout ",bytes,timeout);
  printhexstring(str,bytes);
  #endif
  do {
    bytes_read += serial_read(str + bytes_read, bytes - bytes_read);
    if(bytes_read >= bytes) {
      #ifdef SERIAL_VERBOSE
      printf("**END READ_BYTES: ");
      printhexstring(str,bytes);
      #endif
      return 1;
    }
    msleep(SLEEPYTIME);
    timespent += SLEEPYTIME;
  } while (timespent <= timeout);
  #ifdef SERIAL_VERBOSE
  printf("TIMEOUT TRYING TO READ %i BYTES, GOT: ",bytes);
  printhexstring(str,bytes_read);
  #endif
  return 0;
}

inline int skip_bytes(int bytes, int timeout) {
  byte *buf = malloc(bytes);
  int bytes_read = read_bytes(buf,bytes,timeout);
  #ifdef SERIAL_VERBOSE
  printf("SKIP_BYTES: Discarded %i bytes.\n",bytes_read);
  #endif
  free(buf);
  return bytes_read;
}

int listen_bytes(byte *str, int len, int max, int timeout) {
  int chars_read = 0; /* total chars read into buffer */
  int chars_in = 0; /* chars added to buffer */
  int timespent = 0; /* estimation of time spent */
  byte *buf = malloc(max); /* buffer for incoming data */
  memset(buf,0,max);
  #ifdef SERIAL_VERBOSE
  printf("LISTEN: ");
  printhexstring(str,len);
  #endif
  while(chars_read < max) {
    chars_in = serial_read(buf + chars_read,max - chars_read);
    if(chars_in > 0) {
      chars_read += chars_in; /* mv cursor */
      if(cmp_bytestring(buf,chars_read,str,len) == 1) {
        #ifdef SERIAL_VERBOSE
        printf(" FOUND.\n");
        printhexstring(str,len);
        #endif
        free(buf);
        return 1;
      };
    };
    /* timeout and throttling routine */
    msleep(SLEEPYTIME); /* timing delay */
    if(timeout > 0) { /* timeout is enabled, we arent waiting forever */
      timespent += SLEEPYTIME; /* increment est. time */
      if(timespent >= timeout) { /* timeout exceeded */
        #ifdef SERIAL_VERBOSE
        printf("LISTEN TIMEOUT\n");
        #endif
        free(buf);
        return 0;
      };
    };
  };
  #ifdef SERIAL_VERBOSE
  printf("STRING NOT FOUND, GOT: ");
  printhexstring(buf,chars_read);
  #endif
  free(buf);
  return 0; /* got max chars with no result */
}

byte checksum_generate(byte *buf, int len) {
  int x = 0;
  sum_t sum = 0;
  for(x=0;x<len;x++) sum += buf[x];
  return ( 256 - ( sum % 256 ) );
};

int checksum_test(byte *buf, int len) {
  int x = 0;
  sum_t sum = 0;
  for(x=0;x<len;x++) sum += buf[x];
  if(( sum & 0xFF ) == 0) return 1;
  return 0;
};

byte *generate_pktcommand(aldl_packetdef_t *packet, aldl_commdef_t *comm) {
  packet->command = malloc(5);
  packet->command[0] = comm->pcm_address;
  packet->command[1] = packet->msg_len;
  packet->command[2] = packet->msg_mode;
  packet->command[3] = packet->id;
  packet->command[4] = checksum_generate(packet->command,4);
  return packet->command;
}

byte *generate_shutup(byte len, byte mode, aldl_commdef_t *comm) {
  byte *tmp = malloc(SHUTUP_LENGTH);
  tmp[0] = comm->pcm_address;
  tmp[1] = len;
  tmp[2] = mode;
  tmp[3] = checksum_generate(tmp,SHUTUP_LENGTH - 1);
  return tmp;
};

inline void msleep(int ms) {
  usleep(ms * 1000); /* just use usleep and convert from ms in unix */
};


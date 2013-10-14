#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include "serio.h"
#include "config.h"
#include "aldl-io.h"

/* number of attempts to shut up the ecm */
int shutup_attempts;

/* local functions -----*/
int aldl_shutup(); /* repeatedly attempt to make the ecm shut up */
int aldl_send_shutup(); /* send shutup requests, returns 1 all the time */
int aldl_recv_shutup(); /* wait for shutup response, returns 1 on success */
int aldl_waitforchatter(); /* waits forever for a byte, then bails */

/* serial wrappers -----------------------*/
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
  if(c->chatterwait == 1) {
    /* FIXME chatter wait spec is not completely implemented.. */
    aldl_waitforchatter(c);
  } else {
    /* without chatter wait mode, just wait 10x chatter delay */
    msleep(c->idledelay * 10);
  };
  if(aldl_shutup(c) == 1) return 1;
  return 0;
}

int aldl_waitforchatter(aldl_commdef_t *c) {
  #ifdef ALDL_VERBOSE
    printf("waiting for idle chatter to confirm key is on..\n");
  #endif
  /* FIXME this should be tuneable */
  while(skip_bytes(1,50) == 0) msleep(500);
  #ifdef ALDL_VERBOSE
    printf("got idle chatter or something.\n");
  #endif
  msleep(c->idledelay);
  return 1;
}

int aldl_shutup(aldl_commdef_t *c) {
  serial_purge_rx(); /* pre-clear read buffer */
  shutup_attempts = 1;
  while(1) {
    aldl_send_shutup(c);
    shutup_attempts++;
    if(shutup_attempts > c->shutuprepeat) return 0;
    if(aldl_recv_shutup(c) == 1) return 1;
  };
}

int aldl_send_shutup(aldl_commdef_t *c) {
  int x;
  /* cram the shutup string into the stream.  this could be optimized, but
     it does work. */
  for(x=0;x<c->shutuprepeat;x++) {
    #ifdef ALDL_VERBOSE
      printf("sending shutup request %i\n",x + 1);
    #endif
    serial_write(c->shutupcommand,c->shutuplength);
    msleep(c->shutuprepeatdelay);
  };
  return 1;
}

int aldl_recv_shutup(aldl_commdef_t *c){
  /* the pcm should echo the string on success, then wait. */
  #ifdef ALDL_VERBOSE
    printf("waiting for response...\n");
  #endif
  int result = listen_bytes(c->shutupcommand,c->shutuplength,
               c->shutupcharlimit,
               c->shutupfailwait);
  #ifdef ALDL_VERBOSE
    printf("response success: %i\n",result);
  #endif
  return result;
}

byte *aldl_get_packet(aldl_packetdef_t *p) {
  serial_purge_rx();
  serial_write(p->command, p->commandlength);
  msleep(p->timer); /* wait for packet generation */
  /* get actual data */
  /* note that this may theoretically take timer * 2 msec to actually fail,
     if the packet isn't actually complete.  that's ok, though. */
  if(read_bytes(p->data, p->length, p->timer) == 0) {
    /* failed to get data */
    memset(p->data,0,p->length);
    return NULL;
  }
  /* got data */
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
      printf("matched %i chars\n",matched);
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
      /* this could be improved, it keeps going back and comparing the
         entire buffer .. */
      if(cmp_bytestring(buf,chars_read,str,len) == 1) {
        #ifdef SERIAL_VERBOSE
        printf("BYTES MATCHED!\n");
        #endif
        free(buf);
        return 1;
      };
      chars_read += chars_in; /* mv cursor */
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
  unsigned int sum;
  for(x=0;x<len;x++) sum += buf[x];
  return ( 256 - ( sum % 256 ) );
};

int checksum_test(byte *buf, int len) {
  int x = 0;
  unsigned int sum;
  for(x=0;x<len;x++) sum += buf[x];
  if((sum & 0xFF ) == 0) return 1;
  return 0;
};

inline void msleep(int ms) {
  usleep(ms * 1000); /* just use usleep and convert from ms in unix */
};


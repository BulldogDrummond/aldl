#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "error.h"
#include "serio.h"
#include "config.h"
#include "aldl-io.h"
#include "useful.h"

/* length of an aldl mode change request string */
/* FIXME this might not be good for other ECMS */
#define SHUTUP_LENGTH 4

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

/* look for str in the serial stream up to a length of max, or a time of
   timeout. */
int listen_bytes(byte *str, int len, int max, int timeout);

/* sleep for ms milliseconds */
inline void msleep(int ms);

/************ FUNCTIONS **********************/

int aldl_reconnect(aldl_commdef_t *c) {
  #ifdef ALDL_VERBOSE
    printf("attempting to place ecm in diagnostic mode.\n");
  #endif
  /* wait forever.  bail some other way if you want to stop waiting. */
  while(1) {
    /* send a 'return to normal mode' command first ... */
    serial_write(c->returncommand,SHUTUP_LENGTH);
    msleep(50);
    serial_purge();
    if(c->chatterwait == 1) {
      aldl_waitforchatter(c);
    } else {
      msleep(c->idledelay);
    };
    if(aldl_shutup(c) == 1) {
      /* a delay here seems necessary ... */
      msleep(50);
      serial_purge();
      return 1;
    } else { /* shutup request failed */
      msleep(50);
      serial_purge();
    };
  };
  return 0;
}

int aldl_waitforchatter(aldl_commdef_t *c) {
  #ifdef ALDL_VERBOSE
    printf("waiting for idle chatter to confirm key is on..\n");
  #endif
  int c_delay = 10;
  while(skip_bytes(1,c_delay) == 0) {
    msleep(c_delay);
    #ifdef NICE_RECONNECT
    if(c_delay < NICE_RECON_MAXDELAY) c_delay += 50;
    #endif
  };
  #ifdef ALDL_VERBOSE
    printf("got idle chatter or something.\n");
  #endif
  msleep(c->idledelay);
  return 1;
}

int aldl_request(byte *pkt, int len) {
  serial_purge();
  serial_write(pkt,len);
  #ifndef AGGRESSIVE
  msleep(aldl_timeout(len));
  #endif
  int result = listen_bytes(pkt,len,len,aldl_timeout(len));
  return result;
}

int aldl_timeout(int len) {
  int timeout = ( len * SERIAL_BYTES_PER_MS ) + ( len * ECMLAGTIME );
  /* if the timeout is too short, set it higher */
  if(timeout < SLEEPYTIME / 1000) timeout = SLEEPYTIME * 2 / 1000;
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
  if(aldl_request(p->command, 5) == 0) return NULL;
  /* get actual data */
  if(read_bytes(p->data, p->length, aldl_timeout(p->length)) == 0) {
    /* failed to get data */
    memset(p->data,0,p->length);
    return NULL;
  }
  return p->data;
}

inline int read_bytes(byte *str, int bytes, int timeout) {
  int bytes_read = 0;
  timespec_t timestamp = get_time();
  #ifdef SERIAL_VERBOSE
  printf("**READ_BYTES %i bytes %i timeout : ",bytes,timeout);
  #endif
  do {
    bytes_read += serial_read(str + bytes_read, bytes - bytes_read);
    if(bytes_read >= bytes) {
      #ifdef SERIAL_VERBOSE
      printhexstring(str,bytes);
      #endif
      return 1;
    }
    #ifndef AGGRESSIVE
    usleep(SLEEPYTIME);
    #endif
  } while (get_elapsed_ms(timestamp) <= timeout);
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
  timespec_t timestamp = get_time(); /* timestamp beginning of op */
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
        free(buf);
        return 1;
      };
    };
    /* timeout and throttling routine */
    #ifndef AGGRESSIVE
    usleep(SLEEPYTIME); /* timing delay */
    #endif
    if(timeout > 0) { /* timeout is enabled, we arent waiting forever */
      if(get_elapsed_ms(timestamp) >= timeout) { /* timeout exceeded */
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

byte *generate_pktcommand(aldl_packetdef_t *packet, aldl_commdef_t *comm) {
  packet->command = malloc(5);
  packet->command[0] = comm->pcm_address;
  packet->command[1] = 0x57; /* msg length */
  packet->command[2] = 0x01; /* msg mode */
  packet->command[3] = packet->id;
  packet->command[4] = checksum_generate(packet->command,4);
  return packet->command;
}

byte *generate_mode(byte mode, aldl_commdef_t *comm) {
  byte *tmp = malloc(SHUTUP_LENGTH);
  tmp[0] = comm->pcm_address;
  tmp[1] = 0x56;
  tmp[2] = mode;
  tmp[3] = checksum_generate(tmp,SHUTUP_LENGTH - 1);
  return tmp;
};


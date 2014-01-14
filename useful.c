#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "aldl-types.h"
#include "useful.h"
#include "error.h"

timespec_t get_time() {
  timespec_t currenttime;
  #ifdef USEFUL_BETTERCLOCK
  clock_gettime(CLOCK_MONOTONIC,&currenttime);
  #else
  gettimeofday(&currenttime,NULL);
  #endif
  return currenttime;
};

unsigned long get_elapsed_ms(timespec_t timestamp) {
  timespec_t currenttime = get_time();
  unsigned long seconds = currenttime.tv_sec - timestamp.tv_sec;
  #ifdef USEFUL_BETTERCLOCK
  unsigned long milliseconds =(currenttime.tv_nsec-timestamp.tv_nsec) / 1000000;
  #else
  unsigned long milliseconds =(currenttime.tv_usec-timestamp.tv_usec) / 1000;
  #endif
  return ( seconds * 1000 ) + milliseconds;
};

inline int faststrcmp(char *a, char *b) {
  #ifdef RETARDED
  retardptr(a,"faststrcmp a");
  retardptr(b,"faststrcmp b");
  #endif
  int x = 0;
  while(a[x] == b[x]) {
    x++;
    if(a[x] == 0 || b[x] == 0) {
      if(a[x] == 0 && b[x] == 0) {
        return 1;
      } else {
        return 0;
      };
    };
  };
  return 0;
};

char faststrcmp_list(char *str, char *list) {
  #ifdef RETARDED
  retardptr(str,"faststrcmp str");
  retardptr(list,"faststrcmp list");
  #endif
  int x = 0;
  int y = 0;
  while(list[x] != 0) {
    y = 0;
    while(str[y] != 0) {
      if(str[y] == list[x]) return list[x];
      y++;
    };
    x++;
  };
  return 0;
};

byte hextobyte(char *str) {
  #ifdef RETARDED
  retardptr(str,"hextobyte");
  #endif
  return (int)strtol(str,NULL,16);
};

int getbit(byte p, int bpos, int flip) {
  return flip ^ ( p >> ( bpos + 1 ) & 0x01 );
};

unsigned int sixteenbit(byte *p) {
  return (unsigned int)((*p<<8)|*(p+1));
};

byte checksum_generate(byte *buf, int len) {
  #ifdef RETARDED
  retardptr(buf,"checksum buf");
  #endif
  int x = 0;
  unsigned int sum = 0;
  for(x=0;x<len;x++) sum += buf[x];
  return ( 256 - ( sum % 256 ) );
};

int checksum_test(byte *buf, int len) {
  int x = 0;
  unsigned int sum = 0;
  for(x=0;x<len;x++) sum += buf[x];
  if(( sum & 0xFF ) == 0) return 1;
  return 0;
};

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

void msleep(int ms) {
  usleep(ms * 1000); /* just use usleep and convert from ms in unix */
};

void printhexstring(byte *str, int length) {
  int x;
  for(x=0;x<length;x++) printf("%X ",(unsigned int)str[x]);
  printf("\n");
};

void *smalloc(size_t size) {
  void *m = malloc(size); 
  if(m == NULL) {
   fprintf(stderr, "Out of memory trying to alloc %u bytes",(unsigned int)size);
   exit(1);
  };
  return m;
};

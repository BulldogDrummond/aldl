#ifndef _USEFUL_H
#define _USEFUL_H

/* this uses clock_gettime instead of gettimeofday, which rounds nanosecond
   clock instead of microsecond, and isnt affected by time jumps.  i think that
   it might introduce more overhead, though ... */
#define USEFUL_BETTERCLOCK

/* clock source for USEFUL_BETTERCLOCK mode. */
#define _CLOCKSOURCE CLOCK_MONOTONIC

#ifdef USEFUL_BETTERCLOCK
#include <time.h>
typedef struct timespec timespec_t;
#else
#include <sys/time.h>
typedef struct timeval timespec_t;
#endif

/* a safe malloc that checks the return value and bails */
void *smalloc(size_t size);

/* get current time */
timespec_t get_time();

/* get the difference between the current time and the timestamp */
unsigned long get_elapsed_ms(timespec_t timestamp);

/* a fast string compare function, returns 1 on match.  this is much faster
   than using strcmp for searches, as it doesn't count chars, and bails
   instantly on mismatch */
inline int faststrcmp(char *a, char *b);

/* a fast string compare function.  returns 0 if none of the chars in list are
   found in str.  otherwise returns the char found. useful for filtering */
char faststrcmp_list(char *str, char *list);

/* convert a 0xFF format string to a 'byte'... */
byte hextobyte(char *str);

/* turn two 8 bit bytes into a 16 bit int */
unsigned int sixteenbit(byte *p);

/* get a single bit from a byte.  xor with flip. */
int getbit(byte p, int bpos, int flip);

/* generate a checksum byte */
byte checksum_generate(byte *buf, int len);

/* test checksum byte of buf, 1 if ok */
int checksum_test(byte *buf, int len);

/* compare a byte string n(eedle) in h(aystack), nonzero if found */
int cmp_bytestring(byte *h, int hsize, byte *n, int nsize);

/* print a string of bytes in hex format */
void printhexstring(byte *str, int length);

/* sleep for ms milliseconds */
void msleep(int ms);

/* ensure number remains within a boundary */
int clamp_int(int min, int max, int in);
float clamp_float(float min, float max, float in);

#endif

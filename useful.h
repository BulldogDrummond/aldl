#ifndef _USEFUL_H
#define _USEFUL_H

/* a fast string compare function, returns 1 on match.  this is much faster
   than using strcmp for searches, as it doesn't count chars, and bails
   instantly on mismatch */
inline int faststrcmp(char *a, char *b);

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

#endif

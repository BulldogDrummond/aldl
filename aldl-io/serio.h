
#ifndef _SERIO_H
#define _SERIO_H

/* write functions.  _f is faster if you know the length in advance. */
int serial_write(char *str);
inline int serial_f_write(char *str, int len);

/* read data from the serial port to buf, returns number of bytes read.
   only reads UP TO len, doesn't stick around waiting for more data if it
   isn't there. */
int serial_read(char *str, int len);

/* read the number of bytes specified, into str.  waits until the correct
   number of bytes were read, and returns 1, or the timeout (in ms)
   has expired, and returns 0. */
inline int serial_read_bytes(char *str, int bytes, int timeout);

/* the same as serial_read_bytes, but discards the bytes.  useful for
   ignoring a known-length string of bytes. */
inline int serial_skip_bytes(int bytes, int timeout);

/* listen for something.  it must match str exactly, if str is non-null.
   len is the length of str if it is known, or not null terminated. */
int serial_listen(char *str, int len, int max, int timeout);

/* wait forever with delay scaling for any packet */
void serial_chatterwait();

/* clears any i/o buffers */
int serial_purge();

/* sleep for ms milliseconds */
inline void msleep(int ms);

#endif


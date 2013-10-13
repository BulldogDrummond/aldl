
#ifndef _SERIO_H
#define _SERIO_H

typedef unsigned char byte;

/* write buffer *str to the serial port, up to len bytes */
int serial_write(byte *str, int len);

/* read data from the serial port to buf, returns number of bytes read.
   only reads UP TO len, doesn't stick around waiting for more data if it
   isn't there. */
int serial_read(byte *str, int len);

/* read the number of bytes specified, into str.  waits until the correct
   number of bytes were read, and returns 1, or the timeout (in ms)
   has expired, and returns 0. */
inline int serial_read_bytes(byte *str, int bytes, int timeout);

/* the same as serial_read_bytes, but discards the bytes.  useful for
   ignoring a known-length string of bytes. */
inline int serial_skip_bytes(int bytes, int timeout);

/* listen for something.  it must match str exactly, if str is non-null.
   len is the length of str if it is known, or not null terminated. */
int serial_listen(byte *str, int len, int max, int timeout);

/* clears any i/o buffers ... */
void serial_purge(); /* both buffers */
void serial_purge_rx(); /* rx only */
void serial_purge_tx(); /* tx only */

/* sleep for ms milliseconds */
inline void msleep(int ms);

#endif


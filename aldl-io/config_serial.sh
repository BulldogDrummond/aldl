#!/bin/sh

# this is for the dumb serial driver, attempts to set a baud around 9600
# using divisors or whatever.  totally untested.  use the ftdi driver !!

if [ -z "${1}" ] ; then
  echo "no interface specified."
  exit 1
fi

if [ ! -f "${1}" ] ; then
  echo "bad interface specified."
  exit 1
fi

# this is untested.  test it ......

setserial -v "$1" baud_base 38400
setserial -v "$1" spd_cust
setserial -v "$1" divisor 4.6875

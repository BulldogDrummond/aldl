#!/bin/sh

echo "adding usb group"
groupadd usb

echo "setting permissions on usb devices"
echo 'SUBSYSTEMS=="usb", ACTION=="add", MODE="0664", GROUP="usb"' >> /etc/udev/rules.d/30-usb.rules

echo "blacklisting ftdi_sio driver"
echo 'blacklist ftdi_sio' > /etc/modprobe.d/ftdi.conf


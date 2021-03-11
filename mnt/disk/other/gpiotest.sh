#!/bin/sh
#
#
# Script to test GPIO output speed
#

while [ 1 ]
do
  # Port 0, HIGH
  gpioctl -i 0 -s 1
  
  # Port 0, LOW
  gpioctl -i 0 -s 0
done


#!/bin/bash

#for lighttpd 
ln -s /disk/usr/local/ /usr/
ln -s /disk/usr/share/ /usr/


#libraries for PHP
ln -s /disk/lib/librt.so.1 /lib/librt.so.1


#libraries for powercalc
ln -s /disk/lib/sqlite3.so /lib/sqlite3.so


#libraries for C++
ln -s /disk/lib/libstdc++.so.6 /lib/libstdc++.so.6
ln -s /disk/lib/libgcc_s.so.1 /lib/libgcc_s.so.1


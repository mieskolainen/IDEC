#
# deltaBox - Algorithmic Non-Intrusive Load Monitoring
#
# Copyright (c) 2008-2009 Mikael Mieskolainen.
# Licensed under the MIT License <http://opensource.org/licenses/MIT>.
#
# Permission is hereby  granted, free of charge, to any  person obtaining a copy
# of this software and associated  documentation files (the "Software"), to deal
# in the Software  without restriction, including without  limitation the rights
# to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
# copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
# furnished to do so, subject to the following conditions:
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
# IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
# FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
# AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
# LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

# --------------------------------------------------
# Makefile for deltaBox
# --------------------------------------------------

CC      = arm-linux-g++
CC2		= arm-linux-gcc
STRIP   = arm-linux-strip
CFLAGS  = -O2 -Wall -I./userial
LDFLAGS = -ldl -lpthread
RM      = /bin/rm -f

# Object files
OBJS1 = powercalc.o io.o ow.o boxfunctions.o sqlite3.so
OBJS2 = socket_client.o boxfunctions.o sqlite3.so
OBJS3 = get_file_archive.o boxfunctions.o sqlite3.so
OBJS4 = get_statistics.o boxfunctions.o sqlite3.so
OBJS5 = cluster.o boxfunctions.o sqlite3.so



# Common userial header/source
ONEWIREHDRS	= userial/ownet.h userial/owproto.h userial/ad26.h
			
ONEWIREOBJS	= userial/crcutil.o userial/ioutil.o userial/swt1f.o \
		      userial/owerr.o userial/cnt1d.o userial/ad26.o

# DS9097 passive adapter support source
DS9097OBJS = userial/ds9097/ownet.o userial/ds9097/linuxlnk.o \
			 userial/ds9097/linuxses.o userial/ds9097/owtran.o
			
# DS9097-U adapter support source
#DS9097UOBJS = userial/ds9097u/ds2480ut.o userial/ds9097u/ownetu.o \
#			   userial/ds9097u/owllu.o userial/ds9097u/owsesu.o \
#			   userial/ds9097u/owtrnu.o userial/ds9097u/linuxlnk.o \
#              src/ds9097u.o


CFLAGS += -DLINUX
#LIBS   += -static -static-libgcc

# Set LOCK to yes for serial port locking support
LOCK = no

ifeq ($(LOCK), yes)
  CFLAGS += -DLOCKDEV
  LIBS   += -llockdev
endif


# --------------------------------------------------
# Linking
# --------------------------------------------------

all: onewire_compile powercalc socket_client.cgi get_file_archive.cgi get_statistics.cgi cluster

powercalc: $(ONEWIREOBJS) $(DS9097OBJS) $(OBJS1) 
	$(CC) $(ONEWIREOBJS) $(DS9097OBJS) $(OBJS1) $(LDFLAGS) -o powercalc $(LIBS)
	$(STRIP) powercalc
	
socket_client.cgi: $(OBJS2)
	$(CC) $(OBJS2) $(LDFLAGS) -o socket_client.cgi
	$(STRIP) socket_client.cgi

get_file_archive.cgi: $(OBJS3)
	$(CC) $(OBJS3) $(LDFLAGS) -o get_file_archive.cgi
	$(STRIP) get_file_archive.cgi

get_statistics.cgi: $(OBJS4)
	$(CC) $(OBJS4) $(LDFLAGS) -o get_statistics.cgi
	$(STRIP) get_statistics.cgi
	
cluster: $(OBJS5)
	$(CC) $(OBJS5) $(LDFLAGS) -o cluster
	$(STRIP) cluster

# --------------------------------------------------	
# Compiling object files
# --------------------------------------------------

onewire_compile: $(ONEWIREHDRS)
	$(CC2) $(CFLAGS) -c -o userial/crcutil.o userial/crcutil.c
	$(CC2) $(CFLAGS) -c -o userial/ioutil.o userial/ioutil.c
	$(CC2) $(CFLAGS) -c -o userial/swt1f.o userial/swt1f.c	
	$(CC2) $(CFLAGS) -c -o userial/owerr.o userial/owerr.c
	$(CC2) $(CFLAGS) -c -o userial/cnt1d.o userial/cnt1d.c
	$(CC2) $(CFLAGS) -c -o userial/ad26.o userial/ad26.c
	$(CC2) $(CFLAGS) -c -o userial/ds9097/ownet.o userial/ds9097/ownet.c
	$(CC2) $(CFLAGS) -c -o userial/ds9097/linuxlnk.o userial/ds9097/linuxlnk.c
	$(CC2) $(CFLAGS) -c -o userial/ds9097/linuxses.o userial/ds9097/linuxses.c
	$(CC2) $(CFLAGS) -c -o userial/ds9097/owtran.o userial/ds9097/owtran.c
	
powercalc.o: powercalc.c io.h ow.h boxfunctions.h sqlite3.h
	$(CC) $(CFLAGS) -c powercalc.c

boxfunctions.o: boxfunctions.c boxfunctions.h sqlite3.h
	$(CC) $(CFLAGS) -c boxfunctions.c
	
io.o: io.c io.h boxfunctions.h sqlite3.h
	$(CC) $(CFLAGS) -c io.c

ow.o: ow.c ow.h boxfunctions.h sqlite3.h $(ONEWIREHDRS)
	$(CC) $(CFLAGS) -c ow.c

get_statistics.o: get_statistics.c boxfunctions.h sqlite3.h
	$(CC) $(CFLAGS) -c get_statistics.c

get_file_archive.o: get_file_archive.c boxfunctions.h sqlite3.h
	$(CC) $(CFLAGS) -c get_file_archive.c

socket_client.o: socket_client.c boxfunctions.h sqlite3.h
	$(CC) $(CFLAGS) -c socket_client.c
	
cluster.o: cluster.c boxfunctions.h sqlite3.h
	$(CC) -O2 -c cluster.c

# Dynamic shared library
sqlite3.so: sqlite3.c sqlite3.h 
	$(CC2) $(CFLAGS) -c sqlite3.c
	$(CC2) -shared -o sqlite3.so sqlite3.o
	$(STRIP) sqlite3.so
	
# --------------------------------------------------
# Clean
# --------------------------------------------------

clean:
	$(RM) *.o
	$(RM) *.so
	$(RM) userial/*.o
	$(RM) userial/ds9097/*.o
	

#	$Id: Makefile.in,v 1.22 2012/09/25 11:17:17 plunky Exp $
#
# Makefile.in for the f77 frontend of pcc.
#

top_srcdir=../..
top_builddir=../..
prefix = /usr/local
exec_prefix = /usr/local
bindir = ${exec_prefix}/bin
libdir = ${exec_prefix}/lib
libexecdir = ${exec_prefix}/libexec
includedir = ${prefix}/include
CC = gcc
TARGOS = litebsd
TARGOSVER = 0
TARGMACH = mips
TARGET = mipsel-unknown-litebsd0.1
VERSION = 1.2.0.DEVEL
PCCLIBDIR = $(libdir)/pcc/$(TARGET)/$(VERSION)/lib
PCCINCDIR = $(libdir)/pcc/$(TARGET)/$(VERSION)/include
F77LIBDIR=-L$(prefix)/lib
CFLAGS = -g -O2  -Wall -Wmissing-prototypes -Wshadow -Wsign-compare
CPPFLAGS =   -D_BSD_SOURCE -DGCC_COMPAT -DNATIVE_FLOATING_POINT -DLANG_F77 \
	-DLIBEXECDIR=\"$(libexecdir)\" -DINCLUDEDIR=\"$(includedir)\" \
	-DPCCINCDIR=\"$(PCCINCDIR)/\" -DPCCLIBDIR=\"$(PCCLIBDIR)/\" \
	-DLIBDIR=\"$(F77LIBDIR)\" -Dmach_$(TARGMACH) -Dos_$(TARGOS) \
	-I$(FCOMDIR) -I$(top_builddir) -I$(top_srcdir)/os/$(TARGOS) -I$(MDIR)
LIBS = 
LDFLAGS = 
INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL} -s
INSTALL_DATA = ${INSTALL} -m 644
OBJS=f77.o
DEST=mipsel-unknown-litebsd0.1-f77

MIPDIR=$(top_srcdir)/mip
MDIR=$(top_srcdir)/arch/$(TARGMACH)
FCOMDIR=$(top_srcdir)/f77/fcom

all: $(DEST)

$(DEST): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

install:
	$(INSTALL_PROGRAM) $(DEST) $(bindir)

clean:
	rm -f  $(OBJS) $(DEST)

distclean: clean
	rm -f  Makefile

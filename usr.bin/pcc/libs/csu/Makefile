#	$Id: Makefile.in,v 1.12 2011/07/26 06:55:43 plunky Exp $
#
# Makefile.in for pcc-libs/csu
#



prefix = /usr/local
exec_prefix = ${prefix}
libdir = ${exec_prefix}/lib
srcdir = .

TARGOS = litebsd
TARGMACH = mips*
TARGET = mipsel-unknown-litebsd0.1
VERSION = 1.2.0.DEVEL
PCCLIBDIR = $(libdir)/pcc/$(TARGET)/$(VERSION)/lib

CC = pcc
CFLAGS = 

MFLAGS += CC="$(CC)"
MFLAGS += MACH="$(TARGMACH)"
MFLAGS += PCCLIBDIR="$(PCCLIBDIR)"

all clean install:
	if test -f $(srcdir)/$(TARGOS)/Makefile; then			\
		(cd $(srcdir)/$(TARGOS) && exec $(MAKE) $(MFLAGS) $@)	\
		    || exit $$?;					\
	fi

distclean: clean
	/bin/rm -f Makefile

#	$Id: Makefile.in,v 1.21 2014/07/01 13:01:48 ragge Exp $
#
# Makefile.in for libpcc.a
#


srcdir = .
top_srcdir = ..
top_builddir = ..
prefix = /usr/local
exec_prefix = ${prefix}
libdir = ${exec_prefix}/lib

CC = pcc
TARGMACH = mips*
TARGOS = litebsd
TARGET = mipsel-unknown-litebsd0.1
VERSION = 1.2.0.DEVEL
PCCLIBDIR = $(libdir)/pcc/$(TARGET)/$(VERSION)/lib
PCCINCDIR = $(libdir)/pcc/$(TARGET)/$(VERSION)/include

CPPFLAGS +=  -I$(srcdir) -DTARGET_LITTLE_ENDIAN=1 -Dos_$(TARGOS) \
	-Dmach_$(TARGMACH)
CPPFLAGS += -isystem $(top_srcdir)/libpcc/include

LIBS = 
AR = ar
RANLIB = ranlib
INSTALL = /usr/bin/install -c

HEADERS +=	float.h limits.h stdarg.h stdbool.h stddef.h \
		iso646.h \
		libpcc_float.h libpcc_limits.h libpcc_stdarg.h \
		libpcc_stdbool.h libpcc_stddef.h

OBJS +=		cmpdi2.o divdi3.o fixdfdi.o fixsfdi.o fixunsdfdi.o \
		fixunssfdi.o floatdidf.o floatdisf.o floatunsdidf.o \
		moddi3.o muldi3.o negdi2.o qdivrem.o \
		ucmpdi2.o udivdi3.o umoddi3.o cxmuldiv.o

OBJS +=		ashldi3.o ashrdi3.o lshrdi3.o

OBJS +=		_alloca.o unwind.o

OBJS +=		ssp.o

DEST = libpcc.a

all: $(DEST)

$(DEST): $(OBJS)
	$(AR) r $@ $?
	$(RANLIB) $@

.c.o :
	$(CC) -O $(CPPFLAGS) $(CFLAGS) -c $<

install: install-headers install-lib

install-headers:
	test -z "${DESTDIR}$(PCCINCDIR)" || mkdir -p "${DESTDIR}$(PCCINCDIR)"
	headers='$(HEADERS)'; for i in $$headers; do \
		$(INSTALL) -m 644 $(srcdir)/include/$$i \
			${DESTDIR}$(PCCINCDIR)/$$i; \
	done

install-lib:
	test -z "${DESTDIR}$(PCCLIBDIR)" || mkdir -p "${DESTDIR}$(PCCLIBDIR)"
	$(INSTALL) -m 644 $(DEST) ${DESTDIR}$(PCCLIBDIR)

clean:
	/bin/rm -f $(OBJS) $(DEST)

distclean:
	/bin/rm -f Makefile

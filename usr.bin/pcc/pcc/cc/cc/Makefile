#	$Id: Makefile.in,v 1.36 2015/11/17 22:34:17 plunky Exp $
#
# Makefile.in for the cc part of pcc.
#

srcdir=.
top_srcdir=../..
top_builddir=../..
prefix = /usr
exec_prefix = ${prefix}
bindir = ${exec_prefix}/bin
libdir = ${exec_prefix}/lib
libexecdir = ${exec_prefix}/libexec
includedir = ${prefix}/include
datarootdir = ${prefix}/share
mandir = ${datarootdir}/man
EXEEXT = 
BINPREFIX = 
TARGOS = litebsd
TARGOSVER = 0
TARGMACH = mips
TARGET = mipsel-unknown-litebsd0.1
VERSION = 1.2.0.DEVEL
PCCLIBDIR = $(libdir)/pcc/$(TARGET)/$(VERSION)/lib
PCCINCDIR = $(libdir)/pcc/$(TARGET)/$(VERSION)/include
CFLAGS += -Wall -Wmissing-prototypes -Wshadow -Wsign-compare -Wno-error \
	-ffunction-sections -fdata-sections
CPPFLAGS =  -DLIBEXECDIR=\"$(libexecdir)/\" \
	 -DGCC_COMPAT -DINCLUDEDIR=\"$(includedir)/\" \
	-DPCCINCDIR=\"$(PCCINCDIR)/\" -DPCCLIBDIR=\"$(PCCLIBDIR)/\" \
	-Dos_$(TARGOS) -Dmach_$(TARGMACH) -DTARGOSVER=$(TARGOSVER) \
	-DCXXPROGNAME=\"$(BINPREFIX)p++$(EXEEXT)\" \
	-DCPPROGNAME=\"$(BINPREFIX)pcpp$(EXEEXT)\" \
	-I$(top_srcdir)/cc/driver -I$(top_builddir) \
	-I$(top_srcdir)/os/$(TARGOS) -I$(MIPDIR) -I$(MDIR)
LIBS =
LDFLAGS += -Wl,--gc-sections
INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644

MIPDIR=$(top_srcdir)/mip
MDIR=$(top_srcdir)/arch/$(TARGMACH)

DEST=cc$(EXEEXT)
DRIVERS=pcc pcpp p++ cc cpp c++

all: $(DEST)

OBJS=	cc.o compat.o strlist.o xalloc.o

cc.o:	$(srcdir)/cc.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(srcdir)/cc.c

compat.o: $(MIPDIR)/compat.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(MIPDIR)/compat.c

strlist.o: $(top_srcdir)/cc/driver/strlist.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(top_srcdir)/cc/driver/strlist.c

xalloc.o: $(top_srcdir)/cc/driver/xalloc.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(top_srcdir)/cc/driver/xalloc.c

$(DEST): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ $(LIBS) -lc -lgcc

install:
	test -z "$(DESTDIR)$(bindir)" || mkdir -p "$(DESTDIR)$(bindir)"
	test -z "$(DESTDIR)$(mandir)/cat1" || mkdir -p "$(DESTDIR)$(mandir)/cat1"
	@for driver in $(DRIVERS); do \
		$(INSTALL_PROGRAM) $(DEST) $(DESTDIR)$(bindir)/$(BINPREFIX)$$driver$(EXEEXT); \
		$(INSTALL_DATA) $(srcdir)/cc.1 $(DESTDIR)$(mandir)/cat1/$$driver.1; \
	done
	test -z "$(DESTDIR)$(PCCINCDIR)" || mkdir -p "$(DESTDIR)$(PCCINCDIR)"
	test -z "$(DESTDIR)$(PCCLIBDIR)" || mkdir -p "$(DESTDIR)$(PCCLIBDIR)"

clean:
	rm -f  $(OBJS) $(DEST)

distclean: clean
	rm -f  Makefile

.include <bsd.prog.mk>

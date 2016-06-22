#	$Id: Makefile.in,v 1.54 2016/03/08 18:42:13 ragge Exp $
#
# Makefile.in for cpp
#

srcdir=.
top_srcdir=../..
builddir=.
top_builddir=../..
prefix = /usr
exec_prefix = ${prefix}
bindir = ${exec_prefix}/bin
libexecdir = ${exec_prefix}/libexec
datarootdir = ${prefix}/share
mandir = ${datarootdir}/man
EXEEXT = 
CFLAGS += -ffunction-sections -fdata-sections
CPPFLAGS =   -D_BSD_SOURCE -DGCC_COMPAT -DNATIVE_FLOATING_POINT \
	-I$(srcdir) -I$(top_builddir) -I$(builddir) -I$(MIPDIR) -I$(MDIR) \
	-I$(COMMONDIR)
LIBS = -lc -lgcc
LDFLAGS += -Wl,--gc-sections -s
YACC = byacc
YFLAGS = 
INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644
TARGMACH = mips

MIPDIR=$(top_srcdir)/mip
MDIR=$(top_srcdir)/arch/$(TARGMACH)
COMMONDIR=$(top_srcdir)/common

DEST=cpp$(EXEEXT)
MANPAGE=cpp

all: $(DEST)

OBJS=	compat.o cpp.o cpc.o token.o
HDRS=	cpp.h

$(OBJS): $(HDRS)

compat.o: $(COMMONDIR)/compat.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(COMMONDIR)/compat.c

cpp.o: $(srcdir)/cpp.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(srcdir)/cpp.c

cpc.o: $(srcdir)/cpc.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(srcdir)/cpc.c

token.o: $(srcdir)/token.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(srcdir)/token.c

$(DEST): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ $(LIBS)

test: $(DEST)
	@for n in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 ; do	\
		echo -n "test$${n} " ;					\
		./$(DEST) < tests/test$${n} > tests/run$${n} &&		\
		cmp tests/run$${n} tests/res$${n} && echo ;		\
		if test -f tests/res$${n}C ; then			\
			echo -n "test$${n}C " ;				\
			./$(DEST) -C < tests/test$${n} > tests/run$${n}C && \
			cmp tests/run$${n}C tests/res$${n}C && echo ;	\
		fi ;							\
	done

install:
	test -z "$(DESTDIR)$(libexecdir)" || mkdir -p "$(DESTDIR)$(libexecdir)"
	$(INSTALL_PROGRAM) $(DEST) $(DESTDIR)$(libexecdir)
	test -z "$(DESTDIR)$(mandir)/cat1" || mkdir -p "$(DESTDIR)$(mandir)/cat1"
	$(INSTALL_DATA) $(srcdir)/cpp.1 $(DESTDIR)$(mandir)/cat1/$(MANPAGE).1

clean:
	rm -f $(OBJS) $(DEST) tests/run*

distclean: clean
	rm -f Makefile

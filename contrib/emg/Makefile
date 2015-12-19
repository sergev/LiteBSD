#	@(#)Makefile	8.56 (Berkeley) 12/3/94
#
# This Makefile is ONLY correct for 4.4BSD style source trees.  If you're
# not using a system with that setup, it's not going to work.  Read the
# files ../README and ../PORT/README for information on building nvi for
# other systems.

PROG=	emg

DPADD+= ${LIBTERMCAP}
LDADD+=	-ltermcap

CLEANFILES+=${PROG}

CFLAGS+=-ffunction-sections -fdata-sections
LDFLAGS+=-Wl,--gc-sections

# emg sources.
SRCS=	basic.c buffer.c display.c file.c fileio.c line.c main.c random.c \
	region.c search.c strtonum.c tcap.c ttyio.c word.c window.c

afterinstall:
	install -d ${DESTDIR}/usr/share/doc/emg
	install -c emg.keys ${DESTDIR}/usr/share/doc/emg

.include <bsd.prog.mk>

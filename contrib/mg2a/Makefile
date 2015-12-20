#	@(#)Makefile	8.56 (Berkeley) 12/3/94
#
# This Makefile is ONLY correct for 4.4BSD style source trees.  If you're
# not using a system with that setup, it's not going to work.  Read the
# files ../README and ../PORT/README for information on building nvi for
# other systems.

PROG=	mg
MAN1=   mg.0

CFLAGS+=-DLOCAL_VARARGS -DDO_METAKEY -DPREFIXREGION -DFKEYS -DXKEYS
CFLAGS+=-DNO_RESIZE
DPADD+= ${LIBTERMLIB}
LDADD+=	-ltermlib

CLEANFILES+=${PROG}

# Mg sources.
SRCS=	basic.c buffer.c cinfo.c dir.c dired.c display.c echo.c \
	extend.c file.c help.c kbd.c keymap.c line.c macro.c main.c \
	match. modes.c paragraph.c random.c region.c search.c \
	version.c window.c word.c

# Originally from sys/bsd.
SRCS+=	fileio.c spawn.c ttyio.c

# Originally from sys/default.
SRCS+=	tty.c ttykbd.c

.include <bsd.prog.mk>

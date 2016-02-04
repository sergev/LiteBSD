# $OpenBSD: Makefile,v 1.31 2015/09/29 03:50:58 tedu Exp $

PROG=	mg

LDADD+=	-lncurses -lutil
DPADD+=	${LIBNCURSES} ${LIBUTIL}

# (Common) compile-time options:
#
#	REGEX		-- create regular expression functions.
#	STARTUPFILE	-- look for and handle initialization file.
#
CFLAGS+=-DREGEX -Wno-error=maybe-uninitialized
CFLAGS+=-ffunction-sections -fdata-sections
LDFLAGS+=-Wl,--gc-sections

SRCS=	autoexec.c basic.c bell.c buffer.c cinfo.c dir.c display.c \
	echo.c extend.c file.c fileio.c funmap.c help.c kbd.c keymap.c \
	line.c macro.c main.c match.c modes.c paragraph.c \
	re_search.c region.c search.c spawn.c tty.c ttyio.c ttykbd.c \
	undo.c util.c version.c window.c word.c yank.c

#
# More or less standalone extensions.
#
SRCS+=	cmode.c dired.c grep.c

beforeinstall:
	install -d ${DESTDIR}/usr/share/doc/mg
	install -m 444 -c ${.CURDIR}/tutorial \
	    ${DESTDIR}/usr/share/doc/mg

.include <bsd.prog.mk>

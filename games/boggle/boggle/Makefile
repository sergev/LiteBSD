#	@(#)Makefile	8.1 (Berkeley) 6/11/93

PROG=	boggle
SRCS=	bog.c help.c mach.c prtable.c timer.c word.c
DPADD=	${LIBOCURSES} ${LIBTERM}
LDADD=	-locurses -ltermlib
HIDEGAME=hidegame
MAN6=	boggle.0
CFLAGS+=-DOLD_CURSES

beforeinstall:
	install -c -o ${BINOWN} -g ${BINGRP} -m 444 ${.CURDIR}/helpfile \
	    ${DESTDIR}/usr/share/games/boggle/helpfile

.include "../../Makefile.inc"
.include <bsd.prog.mk>

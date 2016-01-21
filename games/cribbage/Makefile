#	@(#)Makefile	8.1 (Berkeley) 5/31/93

PROG=	cribbage
DPADD=	${LIBOCURSES} ${LIBTERM} ${LIBCOMPAT}
LDADD=	-locurses -ltermlib -lcompat
SRCS=	extern.c crib.c cards.c instr.c io.c score.c support.c
MAN6=	cribbage.0
HIDEGAME=hidegame
CFLAGS+=-DOLD_CURSES

beforeinstall: 
	install -c -o ${BINOWN} -g ${BINGRP} -m 444 ${.CURDIR}/cribbage.n \
	    ${DESTDIR}/usr/share/games/cribbage.instr

.include <bsd.prog.mk>

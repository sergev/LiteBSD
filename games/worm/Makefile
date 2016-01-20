#	@(#)Makefile	8.1 (Berkeley) 5/31/93

PROG=	worm
MAN6=	worm.0
DPADD=	${LIBOCURSES} ${LIBTERM} ${LIBCOMPAT}
LDADD=	-locurses -ltermcap -lcompat
HIDEGAME=hidegame
CFLAGS+=-DOLD_CURSES

.include <bsd.prog.mk>

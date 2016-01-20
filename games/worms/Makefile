#	@(#)Makefile	8.1 (Berkeley) 5/31/93

PROG=	worms
MAN6=	worms.0
DPADD=	${LIBOCURSES} ${LIBTERM} ${LIBCOMPAT}
LDADD=	-locurses -ltermcap -lcompat
CFLAGS+=-DOLD_CURSES

.include <bsd.prog.mk>

#	@(#)Makefile	8.1 (Berkeley) 5/31/93

PROG=	canfield
MAN6=	canfield.0
DPADD=	${LIBOCURSES} ${LIBTERM} ${LIBCOMPAT}
LDADD=	-locurses -ltermcap -lcompat
HIDEGAME=hidegame
CFLAGS+=-DOLD_CURSES

.include "../../Makefile.inc"
.include <bsd.prog.mk>

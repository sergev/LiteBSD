#	@(#)Makefile	8.1 (Berkeley) 5/31/93

PROG=	mille
SRCS=	comp.c end.c extern.c init.c mille.c misc.c move.c print.c \
	roll.c save.c types.c varpush.c
DPADD=	${LIBOCURSES} ${LIBTERM} ${LIBCOMPAT}
LDADD=	-locurses -ltermlib -lcompat
MAN6=	mille.0
HIDEGAME=hidegame
CFLAGS+=-DOLD_CURSES

.include <bsd.prog.mk>


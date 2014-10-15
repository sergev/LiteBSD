#	@(#)Makefile	8.1 (Berkeley) 7/19/93

PROG=	login
SRCS=	klogin.c login.c
DPADD=	${LIBUTIL}
LDADD=	-lutil
#CFLAGS+= -DKERBEROS
#DPADD+= ${LIBKRB} ${LIBDES}
#LDADD+= -lkrb -ldes
BINOWN=	root
BINMODE=4555
#INSTALLFLAGS=-fschg

.include <bsd.prog.mk>

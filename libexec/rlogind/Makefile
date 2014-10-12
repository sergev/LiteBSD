#	@(#)Makefile	8.1 (Berkeley) 6/4/93

PROG=	rlogind
SRCS=	rlogind.c des_rw.c
MAN8=	rlogind.0
DPADD=	${LIBUTIL}
LDADD=	-lutil
#CFLAGS+=-DKERBEROS -DCRYPT
#DPADD=	${LIBKRB} ${LIBDES}
#LDADD+=	-lkrb -ldes
.PATH: ${.CURDIR}/../../usr.bin/rlogin

.include <bsd.prog.mk>

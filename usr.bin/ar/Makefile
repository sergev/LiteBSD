PROG=	ar
SRCS=	ar.c read.c util.c write.c
LSRC=	acplex.l
YSRC=	acpyacc.y

LDADD=	-larchive -lelftc -lelf
DPADD=	ranlib

CFLAGS+=-I.

LINKS=	${BINDIR}/ar ${BINDIR}/ranlib

CLEANFILES+= ranlib

MAN1=	ar.0 ranlib.0

ranlib:
	ln -s -f ${PROG} $@

.include <bsd.prog.mk>

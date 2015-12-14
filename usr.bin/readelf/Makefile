PROG=	readelf
SRCS=	readelf.c

WARNS?=	6

DPADD=	${LIBDWARF} ${LIBELF}
LDADD=	-ldwarf -lelftc -lelf

MAN1=	readelf.1

.include <bsd.prog.mk>

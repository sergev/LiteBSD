PROG=	readelf
SRCS=	readelf.c

WARNS?=	6

DPADD=	${LIBDWARF} ${LIBELF}
LDADD=	-ldwarf -lelftc -lelf

MAN1=	readelf.0

.include <bsd.prog.mk>

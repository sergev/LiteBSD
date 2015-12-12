# $Id$

TOP=	..

PROG=	readelf
SRCS=	readelf.c

WARNS?=	6

DPADD=	${LIBDWARF} ${LIBELF}
LDADD=	-ldwarf -lelftc -lelf

MAN1=	readelf.1

.include "${TOP}/mk/elftoolchain.prog.mk"

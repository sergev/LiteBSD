#       $OpenBSD: Makefile,v 1.3 1997/09/21 11:48:17 deraadt Exp $

PROG=	apropos
SRCS=	apropos.c config.c
.PATH:	${.CURDIR}/../man

CFLAGS+= -ffunction-sections -fdata-sections
LDFLAGS+= -Wl,--gc-sections

.include <bsd.prog.mk>

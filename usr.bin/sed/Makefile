#	$OpenBSD: Makefile,v 1.4 2010/01/04 17:50:39 deraadt Exp $

PROG=	sed
SRCS=	compile.c main.c misc.c process.c reallocarray.c

CFLAGS+=-ffunction-sections -fdata-sections
LDFLAGS+=-Wl,--gc-sections

.include <bsd.prog.mk>

#	$OpenBSD: Makefile,v 1.5 2012/12/21 06:17:29 deraadt Exp $

PROG=	sort
SRCS=	append.c fields.c files.c fsort.c init.c msort.c sort.c tmp.c

CFLAGS+= -ffunction-sections -fdata-sections
LDFLAGS+= -Wl,--gc-sections

.include <bsd.prog.mk>

#	$OpenBSD: Makefile,v 1.2 2003/06/25 02:42:50 deraadt Exp $

PROG=	diff
SRCS=	diff.c diffdir.c diffreg.c

CFLAGS+=-ffunction-sections -fdata-sections
LDFLAGS+=-Wl,--gc-sections

.include <bsd.prog.mk>
.include "../../Makefile.inc"

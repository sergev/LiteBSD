#	$NetBSD: Makefile,v 1.11 2009/04/14 22:15:17 lukem Exp $

PROG=		at
SRCS=		at.c panic.c parsetime.c perm.c stime.c raise_default_signal.c
CFLAGS+=	-ffunction-sections -fdata-sections
LDFLAGS+=	-Wl,--gc-sections
LINKS=		${BINDIR}/at ${BINDIR}/atq \
		${BINDIR}/at ${BINDIR}/atrm \
		${BINDIR}/at ${BINDIR}/batch
MLINKS=		at.1 batch.1 \
		at.1 atq.1 \
		at.1 atrm.1

.include <bsd.prog.mk>

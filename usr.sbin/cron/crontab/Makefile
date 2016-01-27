#	$NetBSD: Makefile,v 1.2 2010/05/07 21:54:07 christos Exp $

PROG=	crontab
SRCS=	crontab.c misc.c entry.c env.c pw_dup.c
NOMAN=	We delay man page generation until install time
CFLAGS+= -I${.CURDIR} -ffunction-sections -fdata-sections
LDFLAGS+= -Wl,--gc-sections
BINDIR=	${DESTDIR}/usr/bin
MANDIR= ${DESTDIR}/usr/share/man
CLEANFILES+= crontab.01 crontab.05

install:
	install -c -m 4555 crontab ${BINDIR}
	nroff -man crontab.1 > crontab.01
	install -c -m 444 crontab.01 ${MANDIR}/cat1/crontab.0
	nroff -man crontab.5 > crontab.05
	install -c -m 444 crontab.05 ${MANDIR}/cat5/crontab.0

.include <bsd.prog.mk>

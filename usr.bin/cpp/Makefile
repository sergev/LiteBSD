#	$OpenBSD: Makefile,v 1.12 2013/12/09 02:35:09 guenther Exp $

NOMAN=noman
NOOBJ=noobj

all nologin clean cleandir depend lint tags:

install:
	install -c -m ${BINMODE} ${.CURDIR}/cpp.sh ${DESTDIR}/usr/bin/cpp

.include <bsd.prog.mk>

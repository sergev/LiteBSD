#	$FreeBSD$
#	$OpenBSD: Makefile,v 1.4 2006/06/30 19:02:28 otto Exp $

PROG=	bc
SRCS=	bc.y scan.l
CFLAGS+= -I. -I${.CURDIR} -ffunction-sections -fdata-sections
LDFLAGS+=-Wl,--gc-sections
CLEANFILES+= y.tab.c y.tab.h

DPADD=	${LIBEDIT} ${LIBTERMCAP}
LDADD=	-ledit -ltermcap

NO_WMISSING_VARIABLE_DECLARATIONS=

FILES+=	bc.library
FILESDIR=${DESTDIR}/usr/share/misc

beforeinstall:
	install -m 444 ${FILES} ${FILESDIR}

depend:

.include <bsd.prog.mk>

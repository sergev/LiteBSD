#	$OpenBSD: Makefile,v 1.2 2002/03/02 16:20:33 millert Exp $

PROG=	spellprog
MAN1=	spell.0
SRCS=	spellprog.c look.c
CFLAGS+= -ffunction-sections -fdata-sections
LDFLAGS+= -Wl,--gc-sections
BINDIR=	/usr/libexec
INSTALL= install
INSTALL_COPY= -c
BINMODE= 555
NONBINMODE= 444

beforeinstall:
	${INSTALL} -d ${DESTDIR}/usr/share/dict/special
	${INSTALL} ${INSTALL_COPY} -m ${BINMODE} \
		${.CURDIR}/spell.ksh ${DESTDIR}/usr/bin/spell
	${INSTALL} ${INSTALL_COPY} -m ${NONBINMODE} \
		${.CURDIR}/stop ${DESTDIR}/usr/share/dict
	${INSTALL} ${INSTALL_COPY} -m ${NONBINMODE} \
		${.CURDIR}/american ${DESTDIR}/usr/share/dict
	${INSTALL} ${INSTALL_COPY} -m ${NONBINMODE} \
		${.CURDIR}/british ${DESTDIR}/usr/share/dict
	${INSTALL} ${INSTALL_COPY} -m ${NONBINMODE} \
		${.CURDIR}/special.4bsd  ${DESTDIR}/usr/share/dict/special/4bsd
	${INSTALL} ${INSTALL_COPY} -m ${NONBINMODE} \
		${.CURDIR}/special.math ${DESTDIR}/usr/share/dict/special/math

.include <bsd.prog.mk>

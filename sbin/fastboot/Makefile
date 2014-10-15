#	@(#)Makefile	8.1 (Berkeley) 6/5/93

MAN8=	fastboot.0
MLINKS=	fastboot.8 fasthalt.8

beforeinstall:
	install -c -m ${BINMODE} \
	    ${.CURDIR}/fastboot.sh ${DESTDIR}${BINDIR}/fastboot
	install -c -m ${BINMODE} \
	    ${.CURDIR}/fasthalt.sh ${DESTDIR}${BINDIR}/fasthalt

.include <bsd.prog.mk>

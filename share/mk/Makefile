#	@(#)Makefile	8.1 (Berkeley) 6/8/93

FILES=	bsd.doc.mk bsd.lib.mk bsd.man.mk bsd.prog.mk bsd.subdir.mk
NOOBJ=	noobj

all clean cleandir depend lint tags:

install:
	install -d ${DESTDIR}${BINDIR}/mk
	install -c -m 444 ${FILES} ${DESTDIR}${BINDIR}/mk
	install -c -m 444 nsys.mk ${DESTDIR}${BINDIR}/mk/sys.mk

.include <bsd.prog.mk>

#	@(#)Makefile	8.1 (Berkeley) 6/8/93

FILES=	dot.cshrc dot.login dot.mailrc dot.mg dot.profile dot.rhosts
NOOBJ=	noobj

all clean cleandir depend lint tags:

install:
	install -d ${DESTDIR}${BINDIR}/skel
	install -c -m 444 ${FILES} ${DESTDIR}${BINDIR}/skel

.include <bsd.prog.mk>

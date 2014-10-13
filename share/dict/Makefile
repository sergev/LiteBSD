#	@(#)Makefile	8.1 (Berkeley) 6/5/93

FILES=	README connectives propernames web2 web2a words
NOOBJ=	noobj

all clean cleandir depend lint tags:

install:
	install -d ${DESTDIR}${BINDIR}/dict
	install -c -m 444 ${FILES} ${DESTDIR}${BINDIR}/dict

.include <bsd.prog.mk>

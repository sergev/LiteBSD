#	@(#)Makefile	8.1 (Berkeley) 6/8/93

FILES=	airport ascii birthtoken eqnchar flowers inter.phone man.template \
	mdoc.template na.phone operator zipcodes
NOOBJ=	noobj

all clean cleandir depend lint tags:

install:
	install -d ${DESTDIR}${BINDIR}/misc
	install -c -m 444 ${FILES} ${DESTDIR}${BINDIR}/misc

.include <bsd.prog.mk>

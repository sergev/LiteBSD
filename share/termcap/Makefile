#	@(#)Makefile	8.1 (Berkeley) 6/8/93

# reorder gives an editor command for most common terminals
# (in reverse order from n'th to 1'st most commonly used)
# to move them to the front of termcap
#
MAN5=	termcap.0

all: termcap.small termcap.full ${MAN5} #termcap.db

termcap.full: reorder termcap.src
	ex - ${.CURDIR}/termcap.src < ${.CURDIR}/reorder > /dev/null

termcap.db: termcap.full
	cap_mkdb termcap.full

clean:
	rm -f termcap.full termcap.db

cleandir: clean
	rm -f ${MAN5}

beforeinstall:
	install -d ${DESTDIR}${BINDIR}/misc
	install -m 444 termcap.small ${DESTDIR}${BINDIR}/misc/termcap
	install -m 444 termcap.full ${DESTDIR}${BINDIR}/misc/termcap.full
#	install -m 444 termcap.db ${DESTDIR}${BINDIR}/misc
	install -m 444 map3270 ${DESTDIR}${BINDIR}/misc

.include <bsd.prog.mk>

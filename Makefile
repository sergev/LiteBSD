#	@(#)Makefile	8.1 (Berkeley) 6/19/93

SUBDIR=	bin contrib games include lib libexec old sbin \
	share usr.bin usr.sbin

afterinstall:
	(cd share/man && ${MAKE} makedb)

build:
	(cd include && ${MAKE} install)
	${MAKE} cleandir
	(cd lib && ${MAKE} depend && ${MAKE} && ${MAKE} install)
	${MAKE} depend && ${MAKE} && ${MAKE} install

.include <bsd.subdir.mk>

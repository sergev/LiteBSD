#	@(#)Makefile	8.1 (Berkeley) 6/19/93

SUBDIR=	bin contrib games include lib libexec old sbin \
	share usr.bin usr.sbin

afterinstall:
	(cd share/man && ${MAKE} makedb)

build:
	(cd etc && ${MAKE} distribution)
	(cd include && ${MAKE} install)
	${MAKE} cleandir
	${MAKE} -C contrib/elf2aout all
	(cd lib && ${MAKE} depend && ${MAKE} && ${MAKE} install)
	${MAKE} depend && ${MAKE} && ${MAKE} install

.include <bsd.subdir.mk>

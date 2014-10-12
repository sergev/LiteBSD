#	@(#)Makefile	8.1 (Berkeley) 6/4/93

PROG=	bugfiler
CFLAGS+=-I${.CURDIR}
SRCS=	bugfiler.c error.c gethead.c process.c redist.c reply.c
BINOWN=	root
BINMODE=4555
MAN1=	sendbug.0
MAN8=	bugfiler.0

beforeinstall:
	install -c -D -m 555 \
	    ${.CURDIR}/sendbug.sh ${DESTDIR}/usr/bin/sendbug
	install -c -D -m 444 ${.CURDIR}/bugformat \
	    ${DESTDIR}/usr/share/misc/bugformat

.include <bsd.prog.mk>

#	@(#)Makefile	8.1 (Berkeley) 6/6/93

PROG=	mset
CFLAGS+=-I${.CURDIR} -I.
SRCS+=	astosc.c map3270.c mset.c
MAN1=	mset.0
MAN5=	map3270.0
.PATH:	${.CURDIR}/../api ${.CURDIR}/../ascii

CLEANFILES+=	astosc.OUT astosc.out

# This and the dependency hacks below to make 'depend' target
# work right...

DEPSRCS+=	astosc.OUT map3270.c mset.c

astosc.o: astosc.OUT
astosc.OUT: ${.CURDIR}/../ctlr/hostctlr.h ${.CURDIR}/../ctlr/function.h
astosc.OUT: ${.CURDIR}/../ctlr/${KBD} ${.CURDIR}/../tools/mkastosc/mkastosc
	${.CURDIR}/../tools/mkastosc/mkastosc \
	    ${.CURDIR}/../ctlr/hostctlr.h ${.CURDIR}/../ctlr/function.h \
	    < ${.CURDIR}/../ctlr/${KBD} > ${.TARGET}
	rm -f astosc.out; ln -s astosc.OUT astosc.out

# astosc.out
${.CURDIR}/../tools/mkastosc/mkastosc:
	cd ${.CURDIR}/../tools/mkastosc; ${MAKE}

depend:	.depend
.depend:${DEPSRCS}
	mkdep ${MKDEP} ${CFLAGS:M-[ID]*} ${.ALLSRC:M*.c}

.include <../../Makefile.inc>
.include <bsd.prog.mk>

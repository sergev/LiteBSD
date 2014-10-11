#	@(#)Makefile	8.1 (Berkeley) 6/1/93

CFLAGS=	-O -DLIBC_SCCS
OBJS=	crt0.o gcrt0.o
CLEANFILES+=	core a.out

ASCROSS+= -I${DESTDIR}/usr/include

all: ${OBJS}

crt0.o: crt0.s
	${MIPS_GCC_PREFIX}${CPP} ${ASCROSS} -DCRT0 ${.ALLSRC} | \
            ${MIPS_GCC_PREFIX}${AS} ${ASCROSS} -o $@

gcrt0.o: crt0.s
	${MIPS_GCC_PREFIX}${CPP} ${ASCROSS} -DMCRT0 ${.ALLSRC} | \
            ${MIPS_GCC_PREFIX}${AS} ${ASCROSS} -o $@

install:
	install -m 444 ${OBJS} ${DESTDIR}/usr/lib

depend lint tags:

.include <bsd.prog.mk>

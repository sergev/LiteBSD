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

install: elf32-mips-le.ld
	install -m 444 ${OBJS} ${DESTDIR}/usr/lib
	install -m 444 elf32-mips-le.ld ${DESTDIR}/usr/lib
	if [ -d ${MIPS_GCC_ROOT}/lib/gcc/mips-sde-elf ]; then\
		install -m 444 ${MIPS_GCC_ROOT}/lib/gcc/mips-sde-elf/*/el/sof/libgcc.a ${DESTDIR}/usr/lib;\
        fi
	if [ -d ${MIPS_GCC_ROOT}/lib/gcc/mips-elf ]; then\
		install -m 444 ${MIPS_GCC_ROOT}/lib/gcc/mips-elf/*/soft-float/el/libgcc.a ${DESTDIR}/usr/lib;\
        fi

depend lint tags:

.include <bsd.prog.mk>

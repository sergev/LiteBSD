#	@(#)sys.mk	8.2 (Berkeley) 3/21/94

unix		?= We run UNIX.
MACHINE_ARCH    =  mips
MACHINE         =  mips
NOPROFILE       =  True

.SUFFIXES: .out .a .ln .o .c .F .f .e .r .y .l .s .cl .p .h

.LIBS:		.a

#
# Default destination directory.
#
DESTDIR         ?= ${BSDSRC}/DESTDIR

#
# Use gcc cross compiler installed at $MIPS_GCC_ROOT directory.
#
.if exists(${MIPS_GCC_ROOT}/bin/mips-sde-elf-gcc)
GCC_PREFIX      ?= ${MIPS_GCC_ROOT}/bin/mips-sde-elf-
.endif
.if exists(${MIPS_GCC_ROOT}/bin/mips-elf-gcc)
GCC_PREFIX      ?= ${MIPS_GCC_ROOT}/bin/mips-elf-
.endif

AR		?= ${GCC_PREFIX}ar
ARFLAGS		?= rl
RANLIB		?= ${GCC_PREFIX}ranlib

AS		?= ${GCC_PREFIX}as -mips32r2 -EL
AFLAGS		?=

CC		?= ${GCC_PREFIX}gcc -mips32r2 -EL
USRINCLUDE      ?= -I${DESTDIR}/usr/include
CFLAGS		?= -msoft-float -nostdinc -Werror -Os ${USRINCLUDE}

CPP		?= ${GCC_PREFIX}cpp

FC		?= f77
FFLAGS		?= -O
EFLAGS		?=

LEX		?= flex
LFLAGS		?=

LD		?= ${GCC_PREFIX}ld -mips32r2 -EL
LDFLAGS		?= -nostdlib -nostartfiles -L${DESTDIR}/usr/lib \
                   -T ${DESTDIR}/usr/lib/elf32-mips-le.ld \
                   -Wl,-z,max-page-size=4096 ${DESTDIR}/usr/lib/crt0.o
.if exists(${MIPS_GCC_ROOT}/bin/mips-elf-gcc)
LDFLAGS		+= -Wl,--oformat=elf32-littlemips
.endif

LINT		?=	lint
LINTFLAGS	?=	-chapbx

MAKE		?=	${MAKE}

PC		?=	pc
PFLAGS		?=

RC		?=	f77
RFLAGS		?=

SHELL		?=	sh

YACC		?=	byacc
YFLAGS		?=	-d

.c:
	${CC} ${CFLAGS} ${.IMPSRC} -o ${.TARGET}

.c.o:
	${CC} ${CFLAGS} -c ${.IMPSRC}

.p.o:
	${PC} ${PFLAGS} -c ${.IMPSRC}

.e.o .r.o .F.o .f.o:
	${FC} ${RFLAGS} ${EFLAGS} ${FFLAGS} -c ${.IMPSRC}

.s.o:
	${AS} ${AFLAGS} -o ${.TARGET} ${.IMPSRC}

.y.o:
	${YACC} ${YFLAGS} ${.IMPSRC}
	${CC} ${CFLAGS} -c y.tab.c -o ${.TARGET}
	rm -f y.tab.c

.l.o:
	${LEX} ${LFLAGS} ${.IMPSRC}
	${CC} ${CFLAGS} -c lex.yy.c -o ${.TARGET}
	rm -f lex.yy.c

.y.c:
	${YACC} ${YFLAGS} ${.IMPSRC}
	mv y.tab.c ${.TARGET}

.l.c:
	${LEX} ${LFLAGS} ${.IMPSRC}
	mv lex.yy.c ${.TARGET}

.s.out .c.out .o.out:
	${CC} ${CFLAGS} ${.IMPSRC} ${LDLIBS} -o ${.TARGET}

.f.out .F.out .r.out .e.out:
	${FC} ${EFLAGS} ${RFLAGS} ${FFLAGS} ${.IMPSRC} \
	    ${LDLIBS} -o ${.TARGET}
	rm -f ${.PREFIX}.o

.y.out:
	${YACC} ${YFLAGS} ${.IMPSRC}
	${CC} ${CFLAGS} y.tab.c ${LDLIBS} -ly -o ${.TARGET}
	rm -f y.tab.c

.l.out:
	${LEX} ${LFLAGS} ${.IMPSRC}
	${CC} ${CFLAGS} lex.yy.c ${LDLIBS} -ll -o ${.TARGET}
	rm -f lex.yy.c

#	@(#)bsd.prog.mk	8.2 (Berkeley) 4/2/94

.if !defined(NOINCLUDE) && exists(${.CURDIR}/../Makefile.inc)
.include "${.CURDIR}/../Makefile.inc"
.endif

.SUFFIXES: .out .o .c .y .l .s .8 .7 .6 .5 .4 .3 .2 .1 .0

.8.0 .7.0 .6.0 .5.0 .4.0 .3.0 .2.0 .1.0:
	nroff -man ${.IMPSRC} > ${.TARGET}

CFLAGS=         ${COPTS} -Werror

STRIP?=	-s

BINGRP?=	bin
BINOWN?=	bin
BINMODE?=	555

CC=             gcc
LDFLAGS=        ${LDOPTS}

LIBCOMPAT?=	/usr/lib/libcompat.a
LIBCURSES?=	/usr/lib/libcurses.a
LIBDBM?=	/usr/lib/libdbm.a
LIBDES?=	/usr/lib/libdes.a
LIBKDB?=	/usr/lib/libkdb.a
LIBKRB?=	/usr/lib/libkrb.a
LIBKVM?=	/usr/lib/libkvm.a
LIBM?=		/usr/lib/libm.a
LIBMP?=		/usr/lib/libmp.a
LIBNCURSES?=	/usr/lib/libncurses.a
LIBOCURSES?=	/usr/lib/libocurses.a
LIBPC?=		/usr/lib/libpc.a
LIBPLOT?=	/usr/lib/libplot.a
LIBRESOLV?=	/usr/lib/libresolv.a
LIBRPC?=	/usr/lib/sunrpc.a
LIBTERM?=	/usr/lib/libterm.a
LIBUTIL?=	/usr/lib/libutil.a

.if defined(SHAREDSTRINGS)
CLEANFILES+=strings
.c.o:
	${CC} -E ${CFLAGS} ${.IMPSRC} | xstr -c -
	@${CC} ${CFLAGS} -c x.c -o ${.TARGET}
	@rm -f x.c
.endif

.if defined(PROG)
.if defined(SRCS)

OBJS+=  ${SRCS:R:S/$/.o/g}

${PROG}: ${OBJS} ${DPADD}
	${CC} ${LDFLAGS} -o ${.TARGET} ${OBJS} ${LDADD}

.else defined(SRCS)

SRCS= ${PROG}.c

${PROG}: ${SRCS} ${DPADD}
	${CC} ${CFLAGS} -o ${.TARGET} ${.CURDIR}/${SRCS} ${LDADD}

MKDEP=	-p

.endif

.if	!defined(MAN1) && !defined(MAN2) && !defined(MAN3) && \
	!defined(MAN4) && !defined(MAN5) && !defined(MAN6) && \
	!defined(MAN7) && !defined(MAN8) && !defined(NOMAN)
MAN1=	${PROG}.0
.endif
.endif
.if !defined(NOMAN)
MANALL=	${MAN1} ${MAN2} ${MAN3} ${MAN4} ${MAN5} ${MAN6} ${MAN7} ${MAN8}
.else
MANALL=
.endif
manpages: ${MANALL}

_PROGSUBDIR: .USE
.if defined(SUBDIR) && !empty(SUBDIR)
	@for entry in ${SUBDIR}; do \
		(echo "===> $$entry"; \
		if test -d ${.CURDIR}/$${entry}.${MACHINE}; then \
			${MAKE} -C ${.CURDIR}/$${entry}.${MACHINE} ${.TARGET:S/realinstall/install/:S/.depend/depend/}; \
		else \
			${MAKE} -C ${.CURDIR}/$${entry} ${.TARGET:S/realinstall/install/:S/.depend/depend/}; \
		fi); \
	done
.endif

.if !target(all)
.MAIN: all
all: ${PROG} ${MANALL} _PROGSUBDIR
.endif

.if !target(clean)
clean: _PROGSUBDIR
	rm -f a.out [Ee]rrs mklog ${PROG}.core ${PROG} ${OBJS} ${CLEANFILES}
.endif

.if !target(cleandir)
cleandir: _PROGSUBDIR
	rm -f a.out [Ee]rrs mklog ${PROG}.core ${PROG} ${OBJS} ${CLEANFILES}
	rm -f .depend ${MANALL}
.endif

# some of the rules involve .h sources, so remove them from mkdep line
.if !target(depend)
depend: .depend _PROGSUBDIR
.depend: ${SRCS}
.if defined(PROG)
	${BSDSRC}/admin/build/mkdep ${MKDEP} ${CFLAGS:M-[ID]*} ${.ALLSRC:M*.c}
.endif
.endif

.if !target(install)
.if !target(beforeinstall)
beforeinstall:
.endif
.if !target(afterinstall)
afterinstall:
.endif

realinstall: _PROGSUBDIR

install: afterinstall maninstall
afterinstall: realinstall
realinstall: beforeinstall
.endif

.if !target(lint)
lint: ${SRCS} _PROGSUBDIR
.if defined(PROG)
	@${LINT} ${LINTFLAGS} ${CFLAGS} ${.ALLSRC} | more 2>&1
.endif
.endif

.if !target(tags)
tags: ${SRCS} _PROGSUBDIR
.if defined(PROG)
	-ctags -f /dev/stdout ${.ALLSRC} | \
	    sed "s;${.CURDIR}/;;" > ${.CURDIR}/tags
.endif
.endif

.if !defined(NOMAN)
.include <bsd.man.mk>
.else
maninstall:
.endif

#	@(#)bsd.lib.mk	8.3 (Berkeley) 4/22/94

.if exists(${.CURDIR}/../Makefile.inc)
.include "${.CURDIR}/../Makefile.inc"
.endif

LIBDIR?=	/usr/lib
LINTLIBDIR?=	/usr/libdata/lint
LIBGRP?=	bin
LIBOWN?=	bin
LIBMODE?=	444

STRIP?=	-s

BINGRP?=	bin
BINOWN?=	bin
BINMODE?=	555

#CCROSS?=        -mips32r2 -EL -msoft-float -nostdinc -Werror -I${DESTDIR}/usr/include
#LDCROSS?=       -mips32r2 -EL -nostdlib
#ASCROSS?=       -mips32r2 -EL

.MAIN: all

# prefer .s to a .c, add .po, remove stuff not used in the BSD libraries
.SUFFIXES:
.SUFFIXES: .out .o .po .s .c .f .y .l .8 .7 .6 .5 .4 .3 .2 .1 .0 .m4

.8.0 .7.0 .6.0 .5.0 .4.0 .3.0 .2.0 .1.0:
	nroff -man ${.IMPSRC} > ${.TARGET}

.c.o:
	${MIPS_GCC_PREFIX}${CC} ${CCROSS} ${CFLAGS} -c ${.IMPSRC}

.c.po:
	${MIPS_GCC_PREFIX}${CC} ${CCROSS} -p ${CFLAGS} -c ${.IMPSRC} -o ${.TARGET}

.s.o:
	${MIPS_GCC_PREFIX}${CPP} -E ${CFLAGS:M-[ID]*} ${AINC} ${ASCROSS} ${.IMPSRC} | \
	    ${MIPS_GCC_PREFIX}${AS} ${ASCROSS} -o ${.TARGET}

.s.po:
	${MIPS_GCC_PREFIX}${CPP} -E -DPROF ${CFLAGS:M-[ID]*} ${AINC} ${ASCROSS} ${.IMPSRC} | \
	    ${MIPS_GCC_PREFIX}${AS} ${ASCROSS} -o ${.TARGET}

MANALL=	${MAN1} ${MAN2} ${MAN3} ${MAN4} ${MAN5} ${MAN6} ${MAN7} ${MAN8}
manpages: ${MANALL}

.if !defined(NOPROFILE)
_LIBS=lib${LIB}.a lib${LIB}_p.a
.else
_LIBS=lib${LIB}.a
.endif

all: ${_LIBS} # llib-l${LIB}.ln
.if !defined(NOMAN)
all: ${MANALL}
.endif

OBJS+=	${SRCS:R:S/$/.o/g}

lib${LIB}.a:: ${OBJS}
	@echo building standard ${LIB} library
	@rm -f lib${LIB}.a
	@${MIPS_GCC_PREFIX}${AR} cq lib${LIB}.a `lorder ${OBJS} | tsort` ${LDADD}
	${MIPS_GCC_PREFIX}ranlib lib${LIB}.a

POBJS+=	${OBJS:.o=.po}
lib${LIB}_p.a:: ${POBJS}
	@echo building profiled ${LIB} library
	@rm -f lib${LIB}_p.a
	@${MIPS_GCC_PREFIX}${AR} cq lib${LIB}_p.a `lorder ${POBJS} | tsort` ${LDADD}
	${MIPS_GCC_PREFIX}ranlib lib${LIB}_p.a

llib-l${LIB}.ln: ${SRCS}
	${LINT} -C${LIB} ${CFLAGS} ${.ALLSRC:M*.c}

.if !target(clean)
clean:
	rm -f ${OBJS}
	rm -f ${POBJS}
	rm -f a.out [Ee]rrs mklog ${CLEANFILES} \
	    profiled/*.o lib${LIB}.a lib${LIB}_p.a llib-l${LIB}.ln
.endif

.if !target(cleandir)
cleandir:
	rm -f ${OBJS}
	rm -f ${POBJS}
	rm -f a.out [Ee]rrs mklog ${CLEANFILES} \
	    profiled/*.o lib${LIB}.a lib${LIB}_p.a llib-l${LIB}.ln
	rm -f ${MANALL} .depend
.endif

.if !target(depend)
depend: .depend
.depend: ${SRCS}
	mkdep ${CFLAGS:M-[ID]*} ${AINC} ${.ALLSRC}
	@(TMP=/tmp/_depend$$$$; \
	    sed -e 's/^\([^\.]*\).o *:/\1.o \1.po:/' < .depend > $$TMP; \
	    mv $$TMP .depend)
.endif

.if !target(install)
.if !target(beforeinstall)
beforeinstall:
.endif

realinstall: beforeinstall
	${MIPS_GCC_PREFIX}ranlib lib${LIB}.a
	install -D lib${LIB}.a ${DESTDIR}${LIBDIR}/lib${LIB}.a
	${MIPS_GCC_PREFIX}${RANLIB} -t ${DESTDIR}${LIBDIR}/lib${LIB}.a
.if !defined(NOPROFILE)
	${MIPS_GCC_PREFIX}ranlib lib${LIB}_p.a
	install -D lib${LIB}_p.a ${DESTDIR}${LIBDIR}/lib${LIB}_p.a
	${MIPS_GCC_PREFIX}${RANLIB} -t ${DESTDIR}${LIBDIR}/lib${LIB}_p.a
.endif
#	install -c -o ${LIBOWN} -g ${LIBGRP} -m ${LIBMODE} \
#	    llib-l${LIB}.ln ${DESTDIR}${LINTLIBDIR}
.if defined(LINKS) && !empty(LINKS)
	@set ${LINKS}; \
	while test $$# -ge 2; do \
		l=${DESTDIR}$$1; \
		shift; \
		t=${DESTDIR}$$1; \
		shift; \
		echo $$t -\> $$l; \
		rm -f $$t; \
		ln $$l $$t; \
	done; true
.endif

install: afterinstall
afterinstall: realinstall
.if !defined(NOMAN)
afterinstall: maninstall
.endif
.endif

.if !target(lint)
lint:
.endif

.if !target(tags)
tags: ${SRCS}
	-ctags -f /dev/stdout ${.ALLSRC:M*.c} | \
	    sed "s;\${.CURDIR}/;;" > ${.CURDIR}/tags
.endif

.include <bsd.man.mk>
.if !target(obj)
.if defined(NOOBJ)
obj:
.else
obj:
	@cd ${.CURDIR}; rm -rf obj; \
	here=`pwd`; dest=/usr/obj/`echo $$here | sed 's,/usr/src/,,'`; \
	echo "$$here -> $$dest"; ln -s $$dest obj; \
	if test -d /usr/obj -a ! -d $$dest; then \
		mkdir -p $$dest; \
	else \
		true; \
	fi;
.endif
.endif

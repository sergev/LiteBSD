PROG=	elfcopy

SRCS=	archive.c ascii.c binary.c main.c sections.c segments.c symbols.c

WARNS?=	5

LDADD=	-lelf -lelftc -larchive

MAN1=	elfcopy.0 mcs.0 strip.0

NO_SHARED?=	yes

LINKS=	${BINDIR}/elfcopy ${BINDIR}/strip 	\
	${BINDIR}/elfcopy ${BINDIR}/mcs

EXTRA_TARGETS=	strip mcs
DPADD=	${EXTRA_TARGETS}

CLEANFILES+=	${EXTRA_TARGETS}

# Create in-place symbolic links to "elfcopy" at build time.
${EXTRA_TARGETS}:
	ln -s -f ${PROG} $@

.include <bsd.prog.mk>

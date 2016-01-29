#
# bzip2 Makefile
#

PROG= bzip2

CFLAGS+= -I${.CURDIR}/../../lib/libbzip2 -ffunction-sections -fdata-sections
LDFLAGS+= -Wl,--gc-sections

DPADD= ${LIBBZIP2}
LDADD+= -lbzip2

LINKS+=	${BINDIR}/bzip2 ${BINDIR}/bunzip2 \
	${BINDIR}/bzip2 ${BINDIR}/bzcat

MLINKS+= bzip2.1 bunzip2.1 bzip2.1 bzcat.1

.include <bsd.prog.mk>

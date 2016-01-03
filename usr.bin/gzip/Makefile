#	$NetBSD: Makefile,v 1.18 2013/11/13 11:12:24 pettai Exp $

PROG=		gzip
MAN=		gzip.1

CFLAGS+=	-DNO_BZIP2_SUPPORT -DNO_COMPRESS_SUPPORT -DNO_PACK_SUPPORT
CFLAGS+=	-DNO_XZ_SUPPORT -DSMALL
CFLAGS+=	-ffunction-sections -fdata-sections

LDFLAGS+=	-Wl,--gc-sections

DPADD=		${LIBZ}
LDADD=		-lz

MLINKS+=	gzip.1 gunzip.1 \
		gzip.1 gzcat.1

LINKS+=		${BINDIR}/gzip ${BINDIR}/gunzip \
		${BINDIR}/gzip ${BINDIR}/gzcat

.include <bsd.prog.mk>

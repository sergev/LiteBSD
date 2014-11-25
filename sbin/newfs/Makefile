#	@(#)Makefile	8.2 (Berkeley) 3/27/94

PROG=	newfs
SRCS=	newfs.c mkfs.c getmntopts.c scan_scaled.c
MAN8=	newfs.0

MOUNT=	${.CURDIR}/../mount
CFLAGS+=-DMFS -I${MOUNT}
#.PATH:	${MOUNT} ${.CURDIR}/../disklabel

LINKS=	${BINDIR}/newfs ${BINDIR}/mount_mfs
MLINKS=	newfs.8 mount_mfs.8 newfs.8 mfs.8

.include <bsd.prog.mk>

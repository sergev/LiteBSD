#	@(#)Makefile	8.2 (Berkeley) 4/27/95

PROG=	fsck_ffs
MAN8=	fsck_ffs.0
SRCS=	dir.c inode.c main.c pass1.c pass1b.c pass2.c pass3.c pass4.c \
	pass5.c setup.c utilities.c ffs_subr.c ffs_tables.c fsutil.c
LIBS+=  -lutil
.PATH:	${.CURDIR}/../../sys/ufs/ffs

.include <bsd.prog.mk>

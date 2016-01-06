#	@(#)Makefile	8.1 (Berkeley) 6/4/93

SUBDIR=	libarchive libc libcompat libcurses libcrypto libdwarf libedit \
        libelf libelftc libjson libkvm libm libresolv librpc/rpc libtelnet \
        libterm libutil liby libz

.if	${MACHINE} == "tahoe"
SUBDIR+=csu/tahoe.pcc
.elif	${MACHINE} == "vax"
SUBDIR+=csu/vax.pcc
.else
SUBDIR+=csu/${MACHINE}
.endif

.include <bsd.subdir.mk>

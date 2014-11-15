#	@(#)Makefile	8.1 (Berkeley) 6/12/93

PROG=	netstat
SRCS=	if.c inet.c main.c mbuf.c mroute.c route.c unix.c
#SRCS+= tp_astring.c iso.c ns.c
CFLAGS+=-I/sys
.PATH:	${.CURDIR}/../../sys/netiso
BINGRP=	kmem
BINMODE=2555
LDADD=	-lkvm
DPADD=	${LIBKVM}

.include <bsd.prog.mk>

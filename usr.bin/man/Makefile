#	@(#)Makefile	8.1 (Berkeley) 6/6/93

PROG=	man
SRCS=	config.c man.c
MAN1=	man.0 help.0
MAN5=	man.conf.0

CFLAGS+= -ffunction-sections -fdata-sections
LDFLAGS+= -Wl,--gc-sections

.include <bsd.prog.mk>

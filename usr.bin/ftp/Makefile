#	$OpenBSD: Makefile,v 1.25 2009/05/05 19:35:30 martynas Exp $

# Uncomment the following to provide defaults for gate-ftp operation
#CFLAGS+=-DGATE_SERVER=\"ftp-gw.host\" # -DGATE_PORT=21

PROG=	ftp
SRCS=	cmds.c cmdtab.c complete.c cookie.c domacro.c fetch.c ftp.c \
	list.c main.c ruserpass.c small.c stringlist.c util.c fparseln.c \
	base64.c

CFLAGS+= -ffunction-sections -fdata-sections
LDFLAGS+= -Wl,--gc-sections

LDADD+=	-lssl -lcrypto
DPADD+=	${LIBSSL} ${LIBCRYPTO}

.include <bsd.prog.mk>

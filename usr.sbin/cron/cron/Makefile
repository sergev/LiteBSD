#	$NetBSD: Makefile,v 1.3 2012/06/22 20:32:34 abs Exp $

PROG=	cron
SRCS=	cron.c database.c do_command.c entry.c env.c job.c \
	misc.c popen.c pw_dup.c user.c setproctitle.c
CFLAGS+= -I${.CURDIR} -ffunction-sections -fdata-sections
LDFLAGS+= -Wl,--gc-sections
MAN8=	cron.0

.include <bsd.prog.mk>
.include "../../Makefile.inc"

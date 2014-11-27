#	@(#)Makefile	8.3 (Berkeley) 4/20/95

PROG=	mail
SRCS=	version.c aux.c cmd1.c cmd2.c cmd3.c cmdtab.c collect.c edit.c fio.c \
	getname.c head.c v7.local.c lex.c list.c main.c names.c popen.c \
	quit.c send.c strings.c temp.c tty.c vars.c
SFILES=	mail.help mail.tildehelp
#LINKS=	${BINDIR}/mail ${BINDIR}/Mail
#MLINKS=	mail.1 Mail.1

beforeinstall:
	install -d ${DESTDIR}/usr/share/misc
	install -d ${DESTDIR}/etc
	cd ${.CURDIR}/misc; install -c ${SFILES} ${DESTDIR}/usr/share/misc
	cd ${.CURDIR}/misc; install -c mail.rc ${DESTDIR}/etc/mail.rc

.include <bsd.prog.mk>

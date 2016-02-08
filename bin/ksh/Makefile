#	$OpenBSD: Makefile,v 1.23 2004/12/31 16:24:09 millert Exp $

PROG=	ksh
SRCS=	alloc.c c_ksh.c c_sh.c c_test.c c_ulimit.c edit.c \
	emacs.c eval.c exec.c expr.c history.c io.c jobs.c lex.c \
	mail.c main.c misc.c path.c shf.c syn.c table.c trap.c \
	tree.c tty.c var.c version.c vi.c

DEFS=	-Wall
CFLAGS+=${DEFS} -I. -I${.CURDIR} -ffunction-sections -fdata-sections
LDFLAGS+=-Wl,--gc-sections
MAN1=	ksh.0

CLEANFILES+=	emacs.out

.depend emacs.o: emacs.out

emacs.out: emacs.c
	/bin/sh ${.CURDIR}/emacs-gen.sh ${.CURDIR}/emacs.c > emacs.out

check test:
	/usr/bin/perl ${.CURDIR}/tests/th -s ${.CURDIR}/tests -p ./ksh -C pdksh,sh,ksh,posix,posix-upu

.include <bsd.prog.mk>

#	@(#)Makefile	8.56 (Berkeley) 12/3/94
#
# This Makefile is ONLY correct for 4.4BSD style source trees.  If you're
# not using a system with that setup, it's not going to work.  Read the
# files ../README and ../PORT/README for information on building nvi for
# other systems.

PROG=	mksh
BINDIR= /bin

CFLAGS+=-DMKSH_NO_SIGSETJMP -DMKSH_SMALL
CFLAGS+=-DMKSH_DISABLE_DEPRECATED -DMKSH_DISABLE_EXPERIMENTAL
CFLAGS+=-DMKSH_NOPWNAM

CLEANFILES+=${PROG}

# mksh sources.
SRCS=	conftest.c edit.c eval.c exec.c expr.c funcs.c histrap.c \
	jobs.c lalloc.c lex.c main.c misc.c shf.c strlcpy.c syn.c \
	tree.c var.c

afterinstall:
	install -c dot.mkshrc ${DESTDIR}/root/.mkshrc

.include <bsd.prog.mk>

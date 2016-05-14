# flex Makefile

PROG=	flex
SRCS=	buf.c ccl.c dfa.c ecs.c filter.c gen.c main.c misc.c nfa.c options.c \
	parse.c regex.c scan.c scanflags.c scanopt.c skel.c sym.c tables.c \
	tables_shared.c tblcmp.c yylex.c

CFLAGS+= -I. -DHAVE_CONFIG_H -ffunction-sections -fdata-sections
LDFLAGS+= -Wl,--gc-sections -s

DPADD=	${LIBM}
LDADD+=	-lm

LINKS=	${BINDIR}/flex ${BINDIR}/lex
MLINKS+=flex.1 lex.1

.include <bsd.prog.mk>

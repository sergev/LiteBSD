#	@(#)Makefile	5.3 (Berkeley) 5/12/90

PROG=	yacc
SRCS=	closure.c error.c graph.c lalr.c lr0.c main.c mkpar.c mstring.c \
	output.c reader.c symtab.c verbose.c warshall.c yaccpar.c
MAN1=	yacc.0 yyfix.0

CFLAGS+=-I. -DHAVE_CONFIG_H -DYYPATCH=20160324 -ffunction-sections -fdata-sections
LDFLAGS+=-Wl,--gc-sections -s

beforeinstall:
	install -c -m ${BINMODE} \
	    ${.CURDIR}/yyfix.sh ${DESTDIR}${BINDIR}/yyfix

.include <bsd.prog.mk>

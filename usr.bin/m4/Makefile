#	$NetBSD: Makefile,v 1.19 2015/01/29 19:26:20 christos Exp $
#
#	@(#)Makefile	8.1 (Berkeley) 6/6/93

# -DEXTENDED 
# 	if you want the paste & spaste macros.

PROG=		m4
CFLAGS+=	-DEXTENDED -I${.CURDIR}/lib -ffunction-sections -fdata-sections
LDFLAGS+=	-Wl,--gc-sections -s
SRCS=	parser.y tokenizer.l eval.c expr.c look.c main.c misc.c gnum4.c trace.c
.PATH: ${.CURDIR}/lib
SRCS+=	ohash_create_entry.c ohash_delete.c ohash_do.c ohash_entries.c \
	ohash_enum.c ohash_init.c ohash_interval.c \
	ohash_lookup_interval.c ohash_lookup_memory.c ohash_qlookup.c \
	ohash_qlookupi.c
YHEADER=1
.if (${HOSTPROG:U} == "")
DPADD+=		${LIBUTIL} ${LIBL}
LDADD+=		-lutil -ll
.endif

tokenizer.o: y.tab.h

CLEANFILES+=parser.c y.tab.h tokenizer.o

.include <bsd.prog.mk>

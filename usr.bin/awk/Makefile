# /****************************************************************
# Copyright (C) Lucent Technologies 1997
# All Rights Reserved
# 
# Permission to use, copy, modify, and distribute this software and
# its documentation for any purpose and without fee is hereby
# granted, provided that the above copyright notice appear in all
# copies and that both that the copyright notice and this
# permission notice and warranty disclaimer appear in supporting
# documentation, and that the name Lucent Technologies or any of
# its entities not be used in advertising or publicity pertaining
# to distribution of the software without specific, written prior
# permission.
# 
# LUCENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
# IN NO EVENT SHALL LUCENT OR ANY OF ITS ENTITIES BE LIABLE FOR ANY
# SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
# IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
# ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
# THIS SOFTWARE.
# ****************************************************************/

PROG=	awk
HOSTCC=	gcc
LDADD=	-lm
DPADD=	${LIBM}
CLEANFILES+=proctab.c maketab.o maketab ytab.c ytab.h
CFLAGS+=-I. -I${.CURDIR} -DNDEBUG -Os \
	 -ffunction-sections -fdata-sections
LDFLAGS+=-Wl,--gc-sections
HOSTCFLAGS+=-I. -DNDEBUG

SRCS=	ytab.c lex.c b.c main.c parse.c lib.c run.c tran.c proctab.c

ytab.c ytab.h: awkgram.y
	$(YACC) -d ${.CURDIR}/awkgram.y
	mv y.tab.c ytab.c
	mv y.tab.h ytab.h

proctab.c: maketab
	./maketab >proctab.c

maketab: ytab.h maketab.c
	${HOSTCC} ${HOSTCFLAGS} ${.CURDIR}/maketab.c -o $@

.include <bsd.prog.mk>

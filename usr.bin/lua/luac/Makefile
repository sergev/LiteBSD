#
# luac Makefile
#
PROG= luac

CFLAGS+= -Wall -Wextra -ffunction-sections -fdata-sections -I${.CURDIR}/../../../lib/liblua
LDFLAGS+=-Wl,--gc-sections

DPADD= ${LIBLUA} ${LIBM}
LDADD+= -llua -lm

.include <bsd.prog.mk>
.include "../../Makefile.inc"

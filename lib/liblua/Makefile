#
# liblua.a Makefile
#
LIB=	lua
HDRS=	lauxlib.h lua.h lua.hpp luaconf.h lualib.h
SRCS=	lapi.c lcode.c lctype.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c \
	lmem.c lobject.c lopcodes.c lparser.c lstate.c lstring.c ltable.c \
	ltm.c lundump.c lvm.c lzio.c lauxlib.c lbaselib.c lbitlib.c \
	lcorolib.c ldblib.c liolib.c lmathlib.c loslib.c lstrlib.c \
	ltablib.c lutf8lib.c loadlib.c linit.c

ARFLAGS= rcu
CFLAGS+= -Wall -Wextra -DLUA_COMPAT_5_2

beforeinstall:
	-cd ${.CURDIR}; for i in ${HDRS}; do \
		j="cmp -s $$i ${DESTDIR}/usr/include/$$i || \
		    install -c -m 444 \
		    $$i ${DESTDIR}/usr/include"; \
		echo $$j; \
		eval "$$j"; \
	done

.include <bsd.lib.mk>

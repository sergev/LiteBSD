#
# libsqlite3.a Makefile
#
LIB=	sqlite3
HDRS=	sqlite3.h sqlite3ext.h
SRCS=	sqlite3.c

CFLAGS+=-DPACKAGE_NAME=\"sqlite\" -DPACKAGE_TARNAME=\"sqlite\" -DPACKAGE_VERSION=\"3.10.1\" -DPACKAGE_STRING=\"sqlite\ 3.10.1\" -DPACKAGE_BUGREPORT=\"http://www.sqlite.org\" -DPACKAGE_URL=\"\" -DPACKAGE=\"sqlite\" -DVERSION=\"3.10.1\" -D_FILE_OFFSET_BITS=64 -DHAVE_USLEEP=1 -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -D_BSD_SOURCE=1 -I. -DSQLITE_THREADSAFE=0 -DSQLITE_OMIT_LOAD_EXTENSION=1 -DSQLITE_ENABLE_FTS3 -DSQLITE_ENABLE_RTREE

beforeinstall:
	-cd ${.CURDIR}; for i in $(HDRS); do \
		j="cmp -s $$i ${DESTDIR}/usr/include/$$i || \
		    install -c -m 444 \
		    $$i ${DESTDIR}/usr/include"; \
		echo $$j; \
		eval "$$j"; \
	done

.include <bsd.lib.mk>

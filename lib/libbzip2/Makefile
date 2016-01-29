#
# libbzip2 Makefile
#

LIB= bzip2
HDRS= bzlib.h
SRCS= blocksort.c huffman.c crctable.c randtable.c compress.c \
	decompress.c bzlib.c

CFLAGS+= -Wall -D_FILE_OFFSET_BITS=64

beforeinstall:
	-cd ${.CURDIR}; for i in $(HDRS); do \
		j="cmp -s $$i ${DESTDIR}/usr/include/$$i || \
		    install -c -m 444 \
		    $$i ${DESTDIR}/usr/include"; \
		echo $$j; \
		eval "$$j"; \
	done

.include <bsd.lib.mk>

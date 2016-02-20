# linenoise Makefile

LIB=	linenoise
SRCS=	linenoise.c
HDRS=	linenoise.h

beforeinstall:
	-cd ${.CURDIR}; for i in $(HDRS); do \
		j="cmp -s $$i ${DESTDIR}/usr/include/$$i || \
		    install -c -m 444 \
		    $$i ${DESTDIR}/usr/include"; \
		echo $$j; \
		eval "$$j"; \
	done

.include <bsd.lib.mk>

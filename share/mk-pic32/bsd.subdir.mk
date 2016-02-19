#	@(#)bsd.subdir.mk	8.1 (Berkeley) 6/8/93

.MAIN: all

#STRIP?=	-s

BINGRP?=	bin
BINOWN?=	bin
BINMODE?=	555

_SUBDIRUSE: .USE
	@for entry in ${SUBDIR}; do \
		if test -d ${.CURDIR}/$${entry}.${MACHINE}; then \
			echo "===> $${entry}.${MACHINE}"; \
			${MAKE} -C ${.CURDIR}/$${entry}.${MACHINE} ${.TARGET:realinstall=install}; \
		else \
			echo "===> $$entry"; \
			${MAKE} -C ${.CURDIR}/$${entry} ${.TARGET:realinstall=install}; \
		fi; \
	done

${SUBDIR}::
	@if test -d ${.TARGET}.${MACHINE}; then \
		${MAKE} -C ${.CURDIR}/${.TARGET}.${MACHINE} all; \
	else \
		${MAKE} -C ${.CURDIR}/${.TARGET} all; \
	fi

.if !target(all)
all: _SUBDIRUSE
.endif

.if !target(clean)
clean: _SUBDIRUSE
.endif

.if !target(cleandir)
cleandir: _SUBDIRUSE
.endif

.if !target(depend)
depend: _SUBDIRUSE
.endif

.if !target(manpages)
manpages: _SUBDIRUSE
.endif

.if !target(install)
.if !target(beforeinstall)
beforeinstall:
.endif
.if !target(afterinstall)
afterinstall:
.endif
install: afterinstall
afterinstall: realinstall
realinstall: beforeinstall _SUBDIRUSE
.endif
.if !target(maninstall)
maninstall: _SUBDIRUSE
.endif

.if !target(lint)
lint: _SUBDIRUSE
.endif

.if !target(obj)
obj: _SUBDIRUSE
.endif

.if !target(objdir)
objdir: _SUBDIRUSE
.endif

.if !target(tags)
tags: _SUBDIRUSE
.endif

.if !target(package)
package: _SUBDIRUSE
.endif

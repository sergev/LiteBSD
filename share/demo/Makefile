#	@(#)Makefile	8.1 (Berkeley) 6/5/93

FILES_C= c/Makefile c/hello.c c/q8.c c/skeleton.c c/rain.c
FILES_ASM= asm/Makefile asm/echo.S asm/ashello.S
NOOBJ=	noobj

all clean cleandir depend lint tags:

install:
	install -d ${DESTDIR}${BINDIR}/demo/c
	install -c -m 444 ${FILES_C} ${DESTDIR}${BINDIR}/demo/c
	install -d ${DESTDIR}${BINDIR}/demo/asm
	install -c -m 444 ${FILES_ASM} ${DESTDIR}${BINDIR}/demo/asm

.include <bsd.prog.mk>

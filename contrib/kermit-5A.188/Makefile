#	@(#)Makefile	8.1 (Berkeley) 5/31/93

COPTS= -DBSD44 -DCK_CURSES -DDYNAMIC -DTCPSOCKET -DOLD_CURSES
LDADD= -locurses -ltermcap

PROG=	kermit
SRCS=   ckcmai.c ckucmd.c ckuusr.c ckuus2.c ckuus3.c \
        ckuus4.c ckuus5.c ckuus6.c ckuus7.c \
        ckuusx.c ckuusy.c ckcpro.c ckcfns.c \
        ckcfn2.c ckcfn3.c ckuxla.c ckucon.c \
        ckutio.c ckufio.c ckudia.c ckuscr.c \
        ckcnet.c

.include <bsd.prog.mk>

ckcpro.c: ckcpro.w wart ckcdeb.h ckcasc.h ckcker.h
	./wart ckcpro.w ckcpro.c

wart: ckwart.c
	gcc -o wart ckwart.c

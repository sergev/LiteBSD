PROG=	kermit
SRCS=	ckcfn2.c ckcfn3.c ckcfns.c ckclib.c \
	ckcmai.c ckcnet.c ckcpro.c ckctel.c \
	ckcuni.c ckuath.c ckucmd.c ckucns.c \
	ckudia.c ckufio.c ckupty.c ckusig.c \
	ckutio.c ckuus2.c ckuus3.c ckuus4.c \
	ckuus5.c ckuus7.c ckuusr.c ckuusx.c \
	ckuusy.c ckuxla.c

CFLAGS+= -DKTARGET=\"bsd44\" -DBPS_14K -DBPS_19K -DBPS_28K -DBPS_38K -DBPS_57K -DBPS_115K -DBSD44 -DDYNAMIC -DNOCSETS -DNODEBUG -DNODIAL -DNOFLOAT -DNOFTP -DNOHTTP -DNOICP -DNORLOGIN -DNOSEXP -DNOSPL -DNOSSH -DNOUNICODE -DNOUUCP -DTCPSOCKET -ffunction-sections -fdata-sections
LDFLAGS+= -Wl,--gc-sections -s

BUILDID=20110821
CKVER= "9.0.302"

.include <bsd.prog.mk>

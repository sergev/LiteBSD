#
# OpenSSL/crypto/engine/Makefile
#

DIR=	engine
TOP=	../..
CC=	cc
INCLUDES= -I.. -I$(TOP) -I../../include
CFLAG=-g
MAKEFILE=	Makefile
AR=		ar r

CFLAGS= $(INCLUDES) $(CFLAG)

GENERAL=Makefile
TEST= enginetest.c
APPS=

LIB=$(TOP)/libcrypto.a
LIBSRC= eng_err.c eng_lib.c eng_list.c eng_init.c eng_ctrl.c \
	eng_table.c eng_pkey.c eng_fat.c eng_all.c \
	tb_rsa.c tb_dsa.c tb_ecdsa.c tb_dh.c tb_ecdh.c tb_rand.c tb_store.c \
	tb_cipher.c tb_digest.c tb_pkmeth.c tb_asnmth.c \
	eng_openssl.c eng_cnf.c eng_dyn.c eng_cryptodev.c \
	eng_rdrand.c
LIBOBJ= eng_err.o eng_lib.o eng_list.o eng_init.o eng_ctrl.o \
	eng_table.o eng_pkey.o eng_fat.o eng_all.o \
	tb_rsa.o tb_dsa.o tb_ecdsa.o tb_dh.o tb_ecdh.o tb_rand.o tb_store.o \
	tb_cipher.o tb_digest.o tb_pkmeth.o tb_asnmth.o \
	eng_openssl.o eng_cnf.o eng_dyn.o eng_cryptodev.o \
	eng_rdrand.o

SRC= $(LIBSRC)

EXHEADER= engine.h
HEADER=	$(EXHEADER)

ALL=    $(GENERAL) $(SRC) $(HEADER)

top:
	(cd ../..; $(MAKE) DIRS=crypto SDIRS=$(DIR) sub_all)

all:	lib

lib:	$(LIBOBJ)
	$(AR) $(LIB) $(LIBOBJ)
	$(RANLIB) $(LIB) || echo Never mind.
	@touch lib

files:
	$(PERL) $(TOP)/util/files.pl Makefile >> $(TOP)/MINFO

links:
	@$(PERL) $(TOP)/util/mklink.pl ../../include/openssl $(EXHEADER)
	@$(PERL) $(TOP)/util/mklink.pl ../../test $(TEST)
	@$(PERL) $(TOP)/util/mklink.pl ../../apps $(APPS)

install:
	@[ -n "$(INSTALLTOP)" ] # should be set by top Makefile...
	@headerlist="$(EXHEADER)"; for i in $$headerlist ; \
	do  \
	(cp $$i $(INSTALL_PREFIX)$(INSTALLTOP)/include/openssl/$$i; \
	chmod 644 $(INSTALL_PREFIX)$(INSTALLTOP)/include/openssl/$$i ); \
	done;

tags:
	ctags $(SRC)

tests:

lint:
	lint -DLINT $(INCLUDES) $(SRC)>fluff

update: depend

depend:
	@[ -n "$(MAKEDEPEND)" ] # should be set by upper Makefile...
	$(MAKEDEPEND) -- $(CFLAG) $(INCLUDES) $(DEPFLAG) -- $(PROGS) $(LIBSRC)

dclean:
	$(PERL) -pe 'if (/^# DO NOT DELETE THIS LINE/) {print; exit(0);}' $(MAKEFILE) >Makefile.new
	mv -f Makefile.new $(MAKEFILE)

clean:
	rm -f *.o */*.o *.obj lib tags core .pure .nfs* *.old *.bak fluff

# DO NOT DELETE THIS LINE -- make depend depends on it.

eng_all.o: ../../e_os.h ../cryptlib.h eng_all.c eng_int.h
eng_cnf.o: ../../e_os.h ../cryptlib.h eng_cnf.c eng_int.h
eng_cryptodev.o: eng_cryptodev.c
eng_ctrl.o: ../../e_os.h ../cryptlib.h eng_ctrl.c eng_int.h
eng_dyn.o: ../../e_os.h ../cryptlib.h eng_dyn.c eng_int.h
eng_err.o: eng_err.c
eng_fat.o: ../../e_os.h ../cryptlib.h eng_fat.c eng_int.h
eng_init.o: ../../e_os.h ../cryptlib.h eng_init.c eng_int.h
eng_lib.o: ../../e_os.h ../cryptlib.h eng_int.h eng_lib.c
eng_list.o: ../../e_os.h ../cryptlib.h eng_int.h eng_list.c
eng_openssl.o: ../../e_os.h ../cryptlib.h eng_openssl.c
eng_pkey.o: ../../e_os.h ../cryptlib.h eng_int.h eng_pkey.c
eng_rdrand.o: eng_rdrand.c
eng_table.o: ../../e_os.h ../cryptlib.h eng_int.h eng_table.c
tb_asnmth.o: ../../e_os.h ../asn1/asn1_locl.h ../cryptlib.h eng_int.h
tb_asnmth.o: tb_asnmth.c
tb_cipher.o: ../../e_os.h ../cryptlib.h eng_int.h tb_cipher.c
tb_dh.o: ../../e_os.h ../cryptlib.h eng_int.h tb_dh.c
tb_digest.o: ../../e_os.h ../cryptlib.h eng_int.h tb_digest.c
tb_dsa.o: ../../e_os.h ../cryptlib.h eng_int.h tb_dsa.c
tb_ecdh.o: ../../e_os.h ../cryptlib.h eng_int.h tb_ecdh.c
tb_ecdsa.o: ../../e_os.h ../cryptlib.h eng_int.h tb_ecdsa.c
tb_pkmeth.o: ../../e_os.h ../cryptlib.h eng_int.h tb_pkmeth.c
tb_rand.o: ../../e_os.h ../cryptlib.h eng_int.h tb_rand.c
tb_rsa.o: ../../e_os.h ../cryptlib.h eng_int.h tb_rsa.c
tb_store.o: ../../e_os.h ../cryptlib.h eng_int.h tb_store.c

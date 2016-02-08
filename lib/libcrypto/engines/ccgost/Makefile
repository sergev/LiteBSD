DIR=ccgost
TOP=../..
CC=cc
INCLUDES= -I../../include
CFLAG=-g
MAKEFILE= Makefile
AR= ar r
CFLAGS= $(INCLUDES) $(CFLAG)
LIB=$(TOP)/libcrypto.a

LIBSRC= gost2001.c gost2001_keyx.c gost89.c gost94_keyx.c gost_ameth.c gost_asn1.c gost_crypt.c gost_ctl.c gost_eng.c gosthash.c gost_keywrap.c gost_md.c gost_params.c gost_pmeth.c gost_sign.c

LIBOBJ= e_gost_err.o gost2001_keyx.o gost2001.o gost89.o gost94_keyx.o gost_ameth.o gost_asn1.o gost_crypt.o gost_ctl.o gost_eng.o gosthash.o gost_keywrap.o gost_md.o gost_params.o gost_pmeth.o gost_sign.o

SRC=$(LIBSRC)

LIBNAME=gost

top: 
	(cd $(TOP); $(MAKE) DIRS=engines EDIRS=$(DIR) sub_all)

all: lib

tags:
	ctags $(SRC)

errors:
	$(PERL) ../../util/mkerr.pl -conf gost.ec -nostatic -write $(SRC)

lib: $(LIBOBJ)
	if [ -n "$(SHARED_LIBS)" ]; then \
		$(MAKE) -f $(TOP)/Makefile.shared -e \
			LIBNAME=$(LIBNAME) \
			LIBEXTRAS='$(LIBOBJ)' \
			LIBDEPS='-L$(TOP) -lcrypto' \
			link_o.$(SHLIB_TARGET); \
	else \
		$(AR) $(LIB) $(LIBOBJ); \
	fi
	@touch lib

install:
	[ -n "$(INSTALLTOP)" ] # should be set by top Makefile...
	if [ -n "$(SHARED_LIBS)" ]; then \
		set -e; \
		echo installing $(LIBNAME); \
		pfx=lib; \
		if expr "$(PLATFORM)" : "Cygwin" >/dev/null; then \
			sfx=".so"; \
			cp cyg$(LIBNAME).dll $(INSTALL_PREFIX)$(INSTALLTOP)/$(LIBDIR)/engines/$${pfx}$(LIBNAME)$$sfx.new; \
		else \
			case "$(CFLAGS)" in \
			*DSO_BEOS*) sfx=".so";; \
			*DSO_DLFCN*) sfx=`expr "$(SHLIB_EXT)" : '.*\(\.[a-z][a-z]*\)' \| ".so"`;; \
			*DSO_DL*) sfx=".sl";; \
			*DSO_WIN32*) sfx="eay32.dll"; pfx=;; \
			*) sfx=".bad";; \
			esac; \
			cp $${pfx}$(LIBNAME)$$sfx $(INSTALL_PREFIX)$(INSTALLTOP)/$(LIBDIR)/engines/$${pfx}$(LIBNAME)$$sfx.new; \
		fi; \
		chmod 555 $(INSTALL_PREFIX)$(INSTALLTOP)/$(LIBDIR)/engines/$${pfx}$(LIBNAME)$$sfx.new; \
		mv -f $(INSTALL_PREFIX)$(INSTALLTOP)/$(LIBDIR)/engines/$${pfx}$(LIBNAME)$$sfx.new $(INSTALL_PREFIX)$(INSTALLTOP)/$(LIBDIR)/engines/$${pfx}$(LIBNAME)$$sfx; \
	fi

links:

tests:

update: local_depend
	@if [ -z "$(THIS)" ]; then $(MAKE) -f $(TOP)/Makefile reflect THIS=$@; fi

depend: local_depend
	@if [ -z "$(THIS)" ]; then $(MAKE) -f $(TOP)/Makefile reflect THIS=$@; fi
local_depend:
	@[ -z "$(THIS)" ] || $(MAKEDEPEND) -- $(CFLAG) $(INCLUDES) $(DEPFLAG) -- $(PROGS) $(LIBSRC)

files:
	$(PERL) $(TOP)/util/files.pl Makefile >> $(TOP)/MINFO

lint:
	lint -DLINT $(INCLUDES) $(SRC)>fluff

dclean:
	$(PERL) -pe 'if (/^# DO NOT DELETE THIS LINE/) {print; exit(0);}' $(MAKEFILE) >Makefile.new
	mv -f Makefile.new $(MAKEFILE)

clean:
	rm -f *.o *.obj lib tags core .pure .nfs* *.old *.bak fluff *.so *.sl *.dll

# DO NOT DELETE THIS LINE -- make depend depends on it.

gost2001.o: e_gost_err.h gost2001.c gost89.h gost_lcl.h gost_params.h
gost2001.o: gosthash.h
gost2001_keyx.o: e_gost_err.h gost2001_keyx.c gost2001_keyx.h gost89.h
gost2001_keyx.o: gost_keywrap.h gost_lcl.h gosthash.h
gost89.o: gost89.c gost89.h
gost94_keyx.o: e_gost_err.h gost89.h gost94_keyx.c gost_keywrap.h gost_lcl.h
gost94_keyx.o: gosthash.h
gost_ameth.o: e_gost_err.h gost89.h gost_ameth.c gost_lcl.h gost_params.h
gost_ameth.o: gosthash.h
gost_asn1.o: gost89.h gost_asn1.c gost_lcl.h gosthash.h
gost_crypt.o: e_gost_err.h gost89.h gost_crypt.c gost_lcl.h gosthash.h
gost_ctl.o: gost89.h gost_ctl.c gost_lcl.h gosthash.h
gost_eng.o: e_gost_err.h gost89.h gost_eng.c gost_lcl.h gosthash.h
gost_keywrap.o: gost89.h gost_keywrap.c gost_keywrap.h
gost_md.o: e_gost_err.h gost89.h gost_lcl.h gost_md.c gosthash.h
gost_params.o: gost_params.c gost_params.h
gost_pmeth.o: e_gost_err.h gost89.h gost_lcl.h gost_params.h gost_pmeth.c
gost_pmeth.o: gosthash.h
gost_sign.o: e_gost_err.h gost89.h gost_lcl.h gost_params.h gost_sign.c
gost_sign.o: gosthash.h
gosthash.o: gost89.h gosthash.c gosthash.h

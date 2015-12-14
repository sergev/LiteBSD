LIB=	elf

SRCS=   elf.c elf_begin.c elf_cntl.c elf_end.c elf_errmsg.c \
        elf_errno.c elf_data.c elf_fill.c elf_flag.c \
        elf_getarhdr.c elf_getarsym.c elf_getbase.c \
        elf_getident.c elf_hash.c elf_kind.c elf_memory.c \
        elf_next.c elf_open.c elf_rand.c elf_rawfile.c \
        elf_phnum.c elf_shnum.c elf_shstrndx.c elf_scn.c \
        elf_strptr.c elf_update.c elf_version.c gelf_cap.c \
        gelf_checksum.c gelf_dyn.c gelf_ehdr.c gelf_getclass.c \
        gelf_fsize.c gelf_move.c gelf_phdr.c gelf_rel.c \
        gelf_rela.c gelf_shdr.c gelf_sym.c gelf_syminfo.c \
        gelf_symshndx.c gelf_xlate.c libelf_align.c \
        libelf_allocate.c libelf_ar.c libelf_ar_util.c \
        libelf_checksum.c libelf_data.c libelf_ehdr.c \
        libelf_extended.c libelf_memory.c libelf_open.c \
        libelf_phdr.c libelf_shdr.c libelf_xlate.c ${GENSRCS}

INCS=		libelf.h gelf.h

GENSRCS=	libelf_fsize.c libelf_msize.c libelf_convert.c
CLEANFILES=	${GENSRCS}

SHLIB_MAJOR=	1

WARNS?=	6

MAN3=   elf.0 elf_begin.0 elf_cntl.0 elf_end.0 \
        elf_errmsg.0 elf_fill.0 elf_flagdata.0 elf_getarhdr.0 \
        elf_getarsym.0 elf_getbase.0 elf_getdata.0 elf_getident.0 \
        elf_getscn.0 elf_getphdrnum.0 elf_getphnum.0 \
        elf_getshdrnum.0 elf_getshnum.0 elf_getshdrstrndx.0 \
        elf_getshstrndx.0 elf_hash.0 elf_kind.0 elf_memory.0 \
        elf_next.0 elf_open.0 elf_rawfile.0 elf_rand.0 \
        elf_strptr.0 elf_update.0 elf_version.0 gelf.0 \
        gelf_checksum.0 gelf_fsize.0 gelf_getcap.0 \
        gelf_getclass.0 gelf_getdyn.0 gelf_getehdr.0 \
        gelf_getmove.0 gelf_getphdr.0 gelf_getrel.0 \
        gelf_getrela.0 gelf_getshdr.0 gelf_getsym.0 \
        gelf_getsyminfo.0 gelf_getsymshndx.0 gelf_newehdr.0 \
        gelf_newphdr.0 gelf_update_ehdr.0 gelf_xlatetof.0

MLINKS+= \
	elf_errmsg.3 elf_errno.3 \
	elf_flagdata.3 elf_flagarhdr.3 \
	elf_flagdata.3 elf_flagehdr.3 \
	elf_flagdata.3 elf_flagelf.3 \
	elf_flagdata.3 elf_flagphdr.3 \
	elf_flagdata.3 elf_flagscn.3 \
	elf_flagdata.3 elf_flagshdr.3 \
	elf_getdata.3 elf_newdata.3 \
	elf_getdata.3 elf_rawdata.3 \
	elf_getscn.3 elf_ndxscn.3 \
	elf_getscn.3 elf_newscn.3 \
	elf_getscn.3 elf_nextscn.3 \
	elf_getshstrndx.3 elf_setshstrndx.3 \
	elf_open.3 elf_openmemory.3 \
	gelf_getcap.3 gelf_update_cap.3 \
	gelf_getdyn.3 gelf_update_dyn.3 \
	gelf_getmove.3 gelf_update_move.3 \
	gelf_getrel.3 gelf_update_rel.3 \
	gelf_getrela.3 gelf_update_rela.3 \
	gelf_getsym.3 gelf_update_sym.3 \
	gelf_getsyminfo.3 gelf_update_syminfo.3 \
	gelf_getsymshndx.3 gelf_update_symshndx.3 \
	gelf_update_ehdr.3 gelf_update_phdr.3 \
	gelf_update_ehdr.3 gelf_update_shdr.3 \
	gelf_xlatetof.3 gelf_xlatetom.3

.for E in 32 64
MLINKS+= \
	gelf_checksum.3	elf${E}_checksum.3 \
	gelf_fsize.3	elf${E}_fsize.3 \
	gelf_getehdr.3	elf${E}_getehdr.3 \
	gelf_getphdr.3	elf${E}_getphdr.3 \
	gelf_getshdr.3	elf${E}_getshdr.3 \
	gelf_newehdr.3	elf${E}_newehdr.3 \
	gelf_newphdr.3	elf${E}_newphdr.3 \
	gelf_xlatetof.3	elf${E}_xlatetof.3 \
	gelf_xlatetof.3	elf${E}_xlatetom.3
.endfor

libelf_convert.c:	elf_types.m4 libelf_convert.m4
	m4 -D SRCDIR=$(.CURDIR) libelf_convert.m4 > $@

libelf_fsize.c:		elf_types.m4 libelf_fsize.m4
	m4 -D SRCDIR=$(.CURDIR) libelf_fsize.m4 > $@

libelf_msize.c:		elf_types.m4 libelf_msize.m4
	m4 -D SRCDIR=$(.CURDIR) libelf_msize.m4 > $@

.include <bsd.lib.mk>

LIB=	elftc

SRCS=	elftc_bfdtarget.c			\
	elftc_copyfile.c			\
	elftc_demangle.c			\
	elftc_set_timestamps.c			\
	elftc_string_table.c			\
	elftc_version.c				\
	libelftc_bfdtarget.c			\
	libelftc_dem_arm.c			\
	libelftc_dem_gnu2.c			\
	libelftc_dem_gnu3.c			\
	libelftc_hash.c				\
	libelftc_vstr.c

INCS=		libelftc.h

SHLIB_MAJOR=	1

WARNS?=	6

MAN3=	elftc.0 \
	elftc_bfd_find_target.0 \
	elftc_copyfile.0 \
	elftc_demangle.0 \
	elftc_set_timestamps.0 \
	elftc_string_table_create.0 \
	elftc_symbol_table_create.0 \
	elftc_version.0

MLINKS=	elftc_bfd_find_target.3 elftc_bfd_target_byteorder.3 \
	elftc_bfd_find_target.3 elftc_bfd_target_class.3 \
	elftc_bfd_find_target.3 elftc_bfd_target_flavor.3 \
	elftc_string_table_create.3 elftc_string_table_from_section.3 \
	elftc_string_table_create.3 elftc_string_table_destroy.3 \
	elftc_string_table_create.3 elftc_string_table_image.3 \
	elftc_string_table_create.3 elftc_string_table_insert.3 \
	elftc_string_table_create.3 elftc_string_table_lookup.3 \
	elftc_symbol_table_create.3 elftc_symbol_table_create_nested.3 \
	elftc_symbol_table_create.3 elftc_symbol_table_delete_name.3 \
	elftc_symbol_table_create.3 elftc_symbol_table_delete_entry.3 \
	elftc_symbol_table_create.3 elftc_symbol_table_destroy.3 \
	elftc_symbol_table_create.3 elftc_symbol_table_from_section.3 \
	elftc_symbol_table_create.3 elftc_symbol_table_insert.3 \
	elftc_symbol_table_create.3 elftc_symbol_table_iterate.3 \
	elftc_symbol_table_create.3 elftc_symbol_table_lookup.3 \
	elftc_symbol_table_create.3 elftc_symbol_table_to_image.3

GITCOUNT!= git rev-list HEAD --count
CFLAGS+= -DGITCOUNT='"$(GITCOUNT)"'

.include <bsd.lib.mk>

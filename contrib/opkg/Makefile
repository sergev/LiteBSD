PROG    = opkg
CFLAGS  += -DHOST_CPU_STR='"mipsel"'
CFLAGS  += -DDATADIR='"/usr/lib"'
CFLAGS  += -DVERSION='"0.3.2-LiteBSD"'
CFLAGS  += -DHAVE_SOLVER_INTERNAL
#CFLAGS  += -DHAVE_STATVFS
NOGCCERROR =
LDADD   = -larchive -lz
SRCS    = opkg.c
SRCS    += opkg_cmd.c opkg_configure.c opkg_download.c \
	   opkg_install.c opkg_remove.c opkg_conf.c release.c \
  	   release_parse.c opkg_utils.c pkg.c pkg_depends.c pkg_extract.c \
  	   hash_table.c pkg_hash.c pkg_parse.c pkg_vec.c conffile.c \
	   conffile_list.c nv_pair.c nv_pair_list.c pkg_dest.c pkg_dest_list.c \
	   pkg_src.c pkg_src_list.c str_list.c void_list.c active_list.c \
	   file_util.c opkg_message.c md5.c parse_util.c cksum_list.c \
	   sprintf_alloc.c xregex.c xsystem.c xfuncs.c opkg_archive.c \
	   opkg_verify.c string_util.c
SRCS    += opkg_download_http.c
SRCS    += opkg_action.c opkg_upgrade.c opkg_solver_internal.c

afterinstall:
	install -d ${DESTDIR}/etc/opkg
	install -c options.conf ${DESTDIR}/etc/opkg/
	install -c sources.conf ${DESTDIR}/etc/opkg/

.include <bsd.prog.mk>

#	@(#)Makefile	8.1 (Berkeley) 6/19/93

SUBDIR=	bin contrib games include lib libexec old sbin \
	share usr.bin usr.sbin

afterinstall:
	(cd share/man && ${MAKE} makedb)

build:
	(cd etc && ${MAKE} distribution)
	(cd include && ${MAKE} install)
	${MAKE} cleandir
	${MAKE} -C contrib/elf2aout all
	(cd lib && ${MAKE} depend && ${MAKE} && ${MAKE} install)
	${MAKE} depend && ${MAKE} && ${MAKE} install

fsimage: sdcard.img

# Filesystem and swap sizes.
ROOT_MBYTES = 200
SWAP_MBYTES = 32
U_MBYTES    = 100
UFSTOOL     = contrib/ufstool/ufstool

.PHONY: sdcard.img
sdcard.img: ${UFSTOOL} etc/rootfs.manifest
	rm -f $@
	${UFSTOOL} --repartition=fs=${ROOT_MBYTES}M:swap=${SWAP_MBYTES}M:fs=${U_MBYTES}M $@
	${UFSTOOL} --new --partition=1 --manifest=etc/rootfs.manifest $@ ${DESTDIR}
	${UFSTOOL} --new --partition=3 $@

.include <bsd.subdir.mk>

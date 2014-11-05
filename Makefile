#	@(#)Makefile	8.1 (Berkeley) 6/19/93

SUBDIR=	bin contrib games include lib libexec old sbin \
	share usr.bin usr.sbin

afterinstall:
	(cd share/man && ${MAKE} makedb)

#
# Build the whole DESTDIR tree.
#
build:
	${MAKE} -C etc distribution
	${MAKE} -C include install
	${MAKE} cleandir
	${MAKE} -C contrib/elf2aout all
	${MAKE} -C lib depend all install
	${MAKE} depend all install

# Filesystem and swap sizes.
ROOT_MBYTES = 200
SWAP_MBYTES = 32
U_MBYTES    = 100
UFSTOOL     = contrib/ufstool/ufstool

#
# Create disk image from DESTDIR directory contents.
#
fs:     sdcard.img

.PHONY: sdcard.img
sdcard.img: ${UFSTOOL} etc/rootfs.manifest
	rm -f $@
	${UFSTOOL} --repartition=fs=${ROOT_MBYTES}M:swap=${SWAP_MBYTES}M:fs=${U_MBYTES}M $@
	${UFSTOOL} --new --partition=1 --manifest=etc/rootfs.manifest $@ ${DESTDIR}
	${UFSTOOL} --new --partition=3 $@

#
# Write disk image to SD card.
#
installfs:
.if defined(SDCARD)
	@[ -f sdcard.img ] || $(MAKE) sdcard.img
	sudo dd bs=32k if=sdcard.img of=$(SDCARD)
.else
	@echo "Error: No SDCARD defined."
.endif

.include <bsd.subdir.mk>

#
# Top level makefile for the LiteBSD project.
# Usage:
#       make all        -- Compile binaries and create SD image
#       make clean      -- Delete build results
#       make build      -- Compile binaries
#       make kernel     -- Compile kernels for all board types
#       make fs         -- Create SD image
#       make installfs  -- Write the image to SD card
#

SUBDIR=	bin contrib games include lib libexec old sbin \
	share usr.bin usr.sbin

#
# Build binaries and create a filesystem image.
#
all:    build fs

#
# Delete all results of the build.
#
clean:  cleandir
	rm -rf sdcard.img ${DESTDIR}

#
# Create the DESTDIR tree.
# Install /etc and /usr/include files.
# Build and install the libraries.
#
${DESTDIR}:
	${MAKE} -Cetc install
	${MAKE} -Cinclude install
	${MAKE} cleandir
	${MAKE} -Clib depend
	${MAKE} -Clib all install
	${MAKE} depend

#
# Build all the binaries.
#
build:  ${DESTDIR}
	@for d in ${SUBDIR}; do \
		if test $${d} != lib -a $${d} != include -a $${d} != etc; then \
			echo "===> $$d"; \
			${MAKE} -C${.CURDIR}/$${d} all install; \
		fi; \
	done
	${MAKE} -Cshare/man makedb

#
# Build kernels for all boards.
#
kernel:
	${MAKE} -Csys/compile all

#
# Filesystem and swap sizes.
#
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
	test -d ${DESTDIR} || ${MAKE} build
	rm -f $@
	${UFSTOOL} --repartition=fs=${ROOT_MBYTES}M:swap=${SWAP_MBYTES}M:fs=${U_MBYTES}M $@
	${UFSTOOL} --new --partition=1 --manifest=etc/rootfs.manifest $@ ${DESTDIR}
#	${UFSTOOL} --new --partition=3 $@

${UFSTOOL}:
	make -C`dirname ${UFSTOOL}`

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

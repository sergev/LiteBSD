#
# Let us do special work with ports.
#

THISISAPORT="Yes"
.include <bsd.prog.mk>

FAKEDIR=./work
PREFIX=/usr/local
BINDIR=${PREFIX}/bin
LOCALBASE=${PREFIX}
MANDIR=${PREFIX}/man/man
PKGBASE=${LOCALBASE}/pkg
PLISTBASE=${PKGBASE}/plists/
PKGNAME=${PROG}-${V}
TRUEDESTDIR=${DESTDIR}
ARCH?=mipsel

build: all

fake: build
	DESTDIR=${FAKEDIR} ${MAKE} afterinstall maninstall
.if exists(${.CURDIR}/plist)
	install -d ${FAKEDIR}${PLISTBASE}
	install -c -m 444 plist ${FAKEDIR}${PLISTBASE}${PKGNAME}
.endif

plist: fake
.if !exists(${.CURDIR}/plist)
	@echo -n "Creating ${PKGNAME} plist... "
	@echo "Put a one-line description of the package here." >${FAKEDIR}/${PKGNAME}-tempplist
	@echo "Maintainer: Your Name <your@email.com>" >>${FAKEDIR}/${PKGNAME}-tempplist
	@echo "License: BSD, MIT, ISC, GPLv2, GPLv2+, GPLv3, GPLv3+, LGPLv2.1+, etc." >>${FAKEDIR}/${PKGNAME}-tempplist
	@echo "###" >>${FAKEDIR}/${PKGNAME}-tempplist
	@find ${FAKEDIR}/${PREFIX} -type f | xargs ls -1 | \
		sed 's,${FAKEDIR}/${PREFIX}/,,' >>${FAKEDIR}/${PKGNAME}-tempplist
	@mkdir -p ${FAKEDIR}${PLISTBASE}
	@mv ${FAKEDIR}/${PKGNAME}-tempplist "${FAKEDIR}${PLISTBASE}${PKGNAME}"
	@echo "done!"
.endif

package: plist
	@echo -n "Creating ${PKGNAME} package... "
	@(cd ${FAKEDIR} && tar -cz -f "${PKGNAME}".tgz usr/)
	@echo "done!"

fakeroot: build
	DESTDIR=${FAKEDIR} ${MAKE} afterinstall maninstall

control: fakeroot
.if !exists(${.CURDIR}/control)
	@echo -n "Creating ${PKGNAME} control file... "
	@echo "Package: ${PROG}" >control
	@echo "Version: ${V}" >>control
	@echo "Description: Put a description of the package here." >>control
	@echo "    Multiple lines allowed." >>control
	@echo "Maintainer: Your Name <your@email.com>" >>control
	@echo "License: BSD, MIT, ISC, GPLv2, GPLv2+, GPLv3, GPLv3+, LGPLv2.1+, etc." >>control
	@echo "Architecture: ${ARCH}" >>control
	@echo "Installed-Size: nbytes" >>control
	@echo "Depends: list of dependencies" >>control
	@echo "done!"
.endif

CONTROLFILES=control
.if exists(${.CURDIR}/conffiles)
    CONTROLFILES+=conffiles
.endif
.if exists(${.CURDIR}/preinst)
    CONTROLFILES+=preinst
.endif
.if exists(${.CURDIR}/postinst)
    CONTROLFILES+=postinst
.endif
.if exists(${.CURDIR}/prerm)
    CONTROLFILES+=prerm
.endif
.if exists(${.CURDIR}/postrm)
    CONTROLFILES+=postrm
.endif

#
# Build a package in Debian format, compatible with opkg utility
#
pkg:    control
	@echo -n "Creating ${PKGNAME}_${ARCH} package... "
	@(cd ${FAKEDIR} && tar -cz -f data.tar.gz usr/)
	@tar -cz -f ${FAKEDIR}/control.tar.gz ${CONTROLFILES}
	@echo 2.0 > ${FAKEDIR}/debian-binary
	@(cd ${FAKEDIR} && ar cr ${PKGNAME}_${ARCH}.ar debian-binary control.tar.gz data.tar.gz)
	@rm -f ${FAKEDIR}/debian-binary ${FAKEDIR}/control.tar.gz ${FAKEDIR}/data.tar.gz
	@echo "done!"

install: package
	@echo -n "Installing ${PKGNAME}... "
	@tar xzf ${FAKEDIR}/${PKGNAME}.tgz -C "${TRUEDESTDIR}"
	@echo "done!"
	@echo "Remember to edit etc/rootfs.manifest to include the port's files."
	@echo "Use ${TRUEDESTDIR}${PLISTBASE}${PKGNAME} as a guide."

uninstall:
	@echo -n "Deleting ${PKGNAME}... "
	@tail -n +5 ${TRUEDESTDIR}${PLISTBASE}${PKGNAME} | \
		while read x; do rm -rf ${TRUEDESTDIR}${LOCALBASE}/"$x"; done
	@rm -f ${TRUEDESTDIR}${PLISTBASE}${PKGNAME}
	@echo "done!"

clean-package: cleandir
	rm -rf ${FAKEDIR}

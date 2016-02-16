#
# Let us do special work with ports.
#

.include <bsd.prog.mk>

FAKEDIR=${.CURDIR}/work
PREFIX=/usr/local
BINDIR=${PREFIX}/bin
LOCALBASE=${PREFIX}
MANDIR=${PREFIX}/man/man
PKGNAME=${PORT}-${V}
ARCH?=mipsel

build: all

fake: build
.ifdef "${PROG}"
	DESTDIR=${FAKEDIR} ${MAKE} install
.else
	DESTDIR=${FAKEDIR} ${MAKE} do-install
.endif

control: fake
.if !exists(${.CURDIR}/control)
	@echo -n "Creating ${PKGNAME} control file... "
	@echo "Package: ${PORT}" >control
	@echo "Version: ${V}" >>control
	@echo "Description: Put a description of the package here." >>control
	@echo "    Multiple lines allowed." >>control
	@echo "Maintainer: Your Name <your@email.com>" >>control
	@echo "License: BSD, MIT, ISC, GPLv2, GPLv2+, GPLv3, GPLv3+, LGPLv2.1+, etc." >>control
	@echo "Architecture: ${ARCH}" >>control
	@echo "Installed-Size: nbytes" >>control
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
package: control
	@echo -n "Creating ${PKGNAME}_${ARCH} package... "
	@${MAKE} update-control INSTALLED_SIZE="`du -b -s ${FAKEDIR}/usr | (read b x; echo $$b)`"
	@(cd ${FAKEDIR} && tar -cz -f data.tar.gz usr/)
	@tar -cz -f ${FAKEDIR}/control.tar.gz ${CONTROLFILES}
	@echo 2.0 > ${FAKEDIR}/debian-binary
	@(cd ${FAKEDIR} && ar cr ${PKGNAME}_${ARCH}.ar debian-binary control.tar.gz data.tar.gz)
	@rm -f ${FAKEDIR}/debian-binary ${FAKEDIR}/control.tar.gz
	@echo "done!"

# Update Installed-Size field in control file
update-control: control
	@sed "/Installed-Size:/s/:.*/: ${INSTALLED_SIZE}/" -i control

clean-package: cleandir
	rm -rf ${FAKEDIR}

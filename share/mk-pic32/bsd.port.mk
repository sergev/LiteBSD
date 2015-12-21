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

build: all

fake: build
	DESTDIR=${FAKEDIR} ${MAKE} afterinstall maninstall

plist: fake
.if !exists(${FAKEDIR}${PLISTBASE}${PKGNAME})
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

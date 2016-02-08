#	@(#)Makefile	8.7 (Berkeley) 5/25/95

NOOBJ=	oobj

# -rw-r--r--
BIN1=	aliases amd-home amd-n changelist csh.cshrc csh.login csh.logout \
	crontab crontab.local daily dm.conf exports ftpusers ftpwelcome \
	gettytab group hosts hosts.lpd inetd.conf man.conf \
	monthly motd netgroup networks phones printcap protocols rc \
	rc.local remote security services shells syslog.conf ttys weekly

# -rw-rw-rw-
BIN2=	motd

KRB=	README krb.conf krb.realms
NAMEDB=	localhost.rev named.boot root.cache
PCS=	pcs750.bin
WCS1=	wcs fppwcs poc poc1 poc2 fppoc
WCS2=	fpevent fppwcs fppwcs_dual hdcwcs load_diags start_fpp wcs wcs_dual
RBOOTD=	rbootd.conf

all clean cleandir depend etc lint:

install:
	# Hand-craft the password file first.
	-mkdir -p ${DESTDIR}/etc/namedb ${DESTDIR}/dev ${DESTDIR}/tmp
	-mkdir -p ${DESTDIR}/usr/lib ${DESTDIR}/usr/src/sys ${DESTDIR}/root
	-mkdir -p ${DESTDIR}/var/log ${DESTDIR}/var/run ${DESTDIR}/var/spool/uucp
	-mkdir -p ${DESTDIR}/etc/ssl
	cp master.passwd ${DESTDIR}/etc/master.passwd
	cp group ${DESTDIR}/etc/group
	# install devices
.if ${MACHINE} == "hp300"
	(cd etc.hp300; install -c -m 444 MAKEDEV.hpux ${DESTDIR}/dev)
.endif
	install -c -m 555 MAKEDEV.local etc.${MACHINE}/MAKEDEV ${DESTDIR}/dev
	install -c -m 444 COPYRIGHT ${DESTDIR}/
	install -c -m 600 master.passwd ${DESTDIR}/etc
	install -c -m 600 hosts.equiv ${DESTDIR}/etc
	install -c -m 744 netstart ${DESTDIR}/etc
	install -c -m 644 resolv.conf ${DESTDIR}/etc
	install -c -m 644 ${BIN1} ${DESTDIR}/etc
	install -c -m 666 ${BIN2} ${DESTDIR}/etc
	(cd root; \
		install -c -m 644 dot.cshrc ${DESTDIR}/root/.cshrc; \
		install -c -m 600 dot.klogin ${DESTDIR}/root/.klogin; \
		install -c -m 644 dot.login ${DESTDIR}/root/.login; \
		install -c -m 644 dot.mg ${DESTDIR}/root/.mg; \
		install -c -m 644 dot.profile ${DESTDIR}/root/.profile; \
		install -c -m 600 dot.rhosts ${DESTDIR}/root/.rhosts; \
		rm -f ${DESTDIR}/.cshrc ${DESTDIR}/.profile; \
		ln ${DESTDIR}/root/.cshrc ${DESTDIR}/.cshrc; \
		ln ${DESTDIR}/root/.profile ${DESTDIR}/.profile)
	(cd mtree; \
		install -c -m 600 special ${DESTDIR}/etc/mtree; \
		install -c -m 444 4.4BSD.dist ${DESTDIR}/etc/mtree)
	(cd namedb; install -c -m 644 ${NAMEDB} \
	    ${DESTDIR}/etc/namedb)
	(cd ssl; install -c -m 600 cert.pem ${DESTDIR}/etc/ssl)
	install -c -m 664 /dev/null ${DESTDIR}/etc/dumpdates
	install -c -m 644 /dev/null ${DESTDIR}/var/log/lastlog
	install -c -m 644 /dev/null ${DESTDIR}/var/log/messages
	install -c -m 644 /dev/null ${DESTDIR}/var/log/maillog
	install -c -m 644 /dev/null ${DESTDIR}/var/log/lpd-errs
	install -c -m 664 /dev/null ${DESTDIR}/var/run/utmp
	install -c -m 664 /dev/null ${DESTDIR}/var/log/wtmp
	install -c -m 664 /dev/null ${DESTDIR}/var/log/secure
	install -c -m 664 /dev/null ${DESTDIR}/var/log/ftp.log
	install -c -m 664 /dev/null ${DESTDIR}/var/spool/uucp/ERRORS
	(cd etc.${MACHINE}; install -c -m 444 fstab ${DESTDIR}/etc)
.if ${MACHINE} == "hp300"
	(cd etc.hp300; install -c -m 444 ${RBOOTD} ${DESTDIR}/etc)
.endif
.if ${MACHINE} == "tahoe"
	(cd etc.tahoe; install -c -m 444 ${WCS1} ${DESTDIR}/)
.endif
.if ${MACHINE} == "vax"
	(cd etc.vax; install -c -m 444 ${PCS} ${DESTDIR}/)
.endif
	# There are certain symbolic links we have to have, and
	# no other place to have install them.
	(cd ${DESTDIR}/ && rm -f sys && ln -s usr/src/sys sys)
	(cd ${DESTDIR}/etc && rm -f rmt && ln -s ../usr/sbin/rmt rmt)
	(cd ${DESTDIR}/etc && \
	    rm -f termcap && ln -s ../usr/share/misc/termcap termcap)
.if ${MACHINE} == "hp300"
	(cd ${DESTDIR}/usr/lib && rm -f grmd && ln -s ../hpux/lib/grmd grmd)
.endif
	(cd ${DESTDIR}/usr/lib && rm -f tmac && ln -s ../share/tmac tmac)

hcx9-distribution:
	(cd etc.tahoe; install -c -m 444 ${WCS2} \
	    ${DESTDIR}/)

.include <bsd.prog.mk>

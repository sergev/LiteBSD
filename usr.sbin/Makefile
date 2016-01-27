#	@(#)Makefile	5.20 (Berkeley) 6/12/93

SUBDIR=	accton amd arp chown chroot cron dev_mkdb edquota \
	inetd kgmon lpr mtree portmap pstat pwd_mkdb \
	quotaon repquota rmt rwhod sliplogin sysctl \
	syslogd traceroute trpt update vipw

# Missing on 4.4bsd tape: cron mkproto sa
# From the above missing list:
# cron: ported from NetBSD (with tweaks from FreeBSD)

# TODO: ac quot sendmail trsp

.if   make(clean) || make(cleandir)
SUBDIR+=bad144 config config.new eeprom iostat timed
.elif ${MACHINE} == "hp300"
SUBDIR+=config iostat timed
.elif ${MACHINE} == "i386"
SUBDIR+=bad144 config iostat
.elif ${MACHINE} == "luna68k"
SUBDIR+=config iostat timed
.elif ${MACHINE} == "mips"
SUBDIR+=config iostat timed
.elif ${MACHINE} == "sparc"
SUBDIR+=config.new eeprom timed
.elif ${MACHINE} == "tahoe"
SUBDIR+=config iostat timed
.elif ${MACHINE} == "vax"
SUBDIR+=bad144 config iostat timed
.endif

.include <bsd.subdir.mk>

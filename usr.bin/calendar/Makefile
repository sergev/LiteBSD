#	@(#)Makefile	8.1 (Berkeley) 6/6/93

PROG=	calendar

beforeinstall:
	install -d ${DESTDIR}/usr/share/calendar
	install -c -m 444 \
	    ${.CURDIR}/calendars/calendar.* ${DESTDIR}/usr/share/calendar

.include <bsd.prog.mk>

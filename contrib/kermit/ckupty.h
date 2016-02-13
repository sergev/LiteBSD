/* C K U P T Y . H  --  Includes and definitions for ckupty.c  */

/*
  Copyright 1995 by the Massachusetts Institute of Technology.

  Modified for use in C-Kermit by:

  Jeffrey E Altman <jaltman@secure-endpoints.com>
    Secure Endpoints Inc., New York City
  November 1999
*/
#ifndef __PTY_INT_H__
#include <sys/types.h>

/* #define WANT_UTMP */
/* We don't want all the utmp/wtmp stuff */

#ifdef WANT_UTMP
#ifdef HAVE_UTMP_H
#include <utmp.h>
#endif /* HAVE_UTMP_H */
#ifdef HAVE_UTMPX_H
#include <utmpx.h>
#endif /* HAVE_UTMPX_H */
#endif /* WANT_UTMP */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#ifdef __SCO__
#include <sys/unistd.h>
#endif /* __SCO__ */
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#include <stdio.h>

#include <sys/stat.h>
#ifndef SUNOS41
#include <sys/ioctl.h>
#endif	/* SUNOS41 */
#include <sys/file.h>
#include <sys/time.h>
#include <ctype.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <errno.h>
#include <pwd.h>

#ifdef HAVE_SYS_LABEL_H
/* only SunOS 4? */
#include <sys/label.h>
#include <sys/audit.h>
#include <pwdadj.h>
#endif /* HAVE_SYS_LABEL_H */

#include <signal.h>

#ifdef HPUX
#include <sys/ptyio.h>
#endif /* HPUX */
#ifdef sysvimp
#include <compat.h>
#endif /* sysvimp */

#ifdef COMMENT
/* I don't think we actually use this for anything */
/* and it kills Slackware builds, where there is no select.h. */
#ifndef NO_SYS_SELECT_H
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif /* HAVE_SYS_SELECT_H */
#endif /* NO_SYS_SELECT_H */
#endif /* COMMENT */

#ifdef HAVE_STREAMS
#include <sys/stream.h>
#include <sys/stropts.h>
#endif /* HAVE_STREAMS */

#ifdef POSIX_TERMIOS
#ifndef ultrix
#include <termios.h>
#else
#include <sgtty.h>
#endif /* ultrix */
#else /* POSIX_TERMIOS */
#include <sgtty.h>
#endif /* POSIX_TERMIOS */

#include <netdb.h>
/* #include <syslog.h> */
#ifndef ultrix
#include <string.h>
#endif /* ultrix */
/* #include <sys/param.h> */		/* (now done in ckcdeb.h) */

#ifdef HAVE_STREAMS
/* krlogin doesn't test sys/tty... */
#ifdef HAVE_SYS_TTY_H
#include <sys/tty.h>
#endif /* HAVE_SYS_TTY_H */

#ifdef HAVE_SYS_PTYVAR_H
/* Solaris actually uses packet mode, so the real macros are needed too */
#include <sys/ptyvar.h>
#endif /* HAVE_SYS_PTYVAR_H */
#endif /* HAVE_STREAMS */

#ifdef COMMENT
/* This block moved to ckcdeb.h */
#ifndef NO_OPENPTY
/* For NetBSD, see makefile */
#ifndef HAVE_OPENPTY
#ifdef __FreeBSD__
#define HAVE_OPENPTY
#else
#ifdef MACOSX10
#define HAVE_OPENPTY
#endif	/* MACOSX10 */
#endif	/* __FreeBSD__ */
#endif	/* HAVE_OPENPTY */
#endif	/* NO_OPENPTY */
#endif	/* COMMENT */

#ifdef HAVE_VHANGUP
#ifndef OPEN_CTTY_ONLY_ONCE
/*
  Breaks under Ultrix and others where you cannot get controlling
  terminal twice.
*/
#define VHANG_first
#define VHANG_LAST
#endif /* OPEN_CTTY_ONLY_ONCE */
#endif /* HAVE_VHANGUP */

/* Internal functions */
_PROTOTYP(long ptyint_void_association,(void));
_PROTOTYP(long ptyint_open_ctty ,(char *, int *));
_PROTOTYP(VOID ptyint_vhangup, (void));

#ifdef WANT_UTMP
_PROTOTYP(long ptyint_update_wtmp, (struct utmp *, char *, char *));
#endif /* WANT_UTMP */

#define __PTY_INT_H__
#endif /* __PTY_INT_H__ */

#ifndef __LIBPTY_H__

#ifdef WANT_UTMP
/* Constants for pty_update_utmp */
#define PTY_LOGIN_PROCESS 0
#define PTY_USER_PROCESS 1
#define PTY_DEAD_PROCESS 2
#define PTY_TTYSLOT_USABLE (0x1)	/* flags to update_utmp*/
#define PTY_UTMP_USERNAME_VALID (0x2)
#endif /* WANT_UTMP */

_PROTOTYP(long pty_init,(void));
_PROTOTYP(long pty_getpty, ( int *, char *, int));
_PROTOTYP(long pty_open_slave, (char *, int *, int));
_PROTOTYP(long pty_open_ctty, (char *, int *, int));
_PROTOTYP(long pty_initialize_slave, (int));
#ifdef WANT_UTMP
_PROTOTYP(long pty_update_utmp, (int, int, char *, char *, char *, int));
_PROTOTYP(long pty_logwtmp, (char *, char *, char *));
#endif /* WANT_UTMP */
_PROTOTYP(long pty_cleanup, (char *, int, int));

#define PTY_GETPTY_STREAMS               (44806912L)
#define PTY_GETPTY_FSTAT                 (44806913L)
#define PTY_GETPTY_NOPTY                 (44806914L)
#define PTY_GETPTY_SLAVE_TOOLONG         (44806915L)
#define PTY_OPEN_SLAVE_OPENFAIL          (44806916L)
#define PTY_OPEN_SLAVE_CHMODFAIL         (44806917L)
#define PTY_OPEN_SLAVE_NOCTTY            (44806918L)
#define PTY_OPEN_SLAVE_CHOWNFAIL         (44806919L)
#define PTY_OPEN_SLAVE_LINE_PUSHFAIL     (44806920L)
#define PTY_OPEN_SLAVE_PUSH_FAIL         (44806921L)
#define PTY_OPEN_SLAVE_REVOKEFAIL        (44806922L)
#ifdef WANT_UTMP
#define PTY_UPDATE_UTMP_PROCTYPE_INVALID (44806923L)
#endif /* WANT_UTMP */
#define PTY_OPEN_SLAVE_TOOSHORT          (44806924L)
#define ERROR_TABLE_BASE_pty             (44806912L)

extern struct error_table et_pty_error_table;

#define __LIBPTY_H__
#endif /* __LIBPTY_H__ */


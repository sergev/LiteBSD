char *ckptyv = "Pseudoterminal support, 9.0.101, 13 Jun 2011";

/*  C K U P T Y  --  C-Kermit pseudoterminal control functions for UNIX  */

/* Last update: Mon Jun 13 11:32:52 2011 */

/*
  Copyright 1995 by the Massachusetts Institute of Technology.

  Permission to use, copy, modify, and distribute this software and its
  documentation for any purpose and without fee is hereby granted, provided
  that the above copyright notice appear in all copies and that both that
  copyright notice and this permission notice appear in supporting
  documentation, and that the name of M.I.T. not be used in advertising or
  publicity pertaining to distribution of the software without specific,
  written prior permission.  Furthermore if you modify this software you must
  label your software as modified software and not distribute it in such a
  fashion that it might be confused with the original M.I.T. software.
  M.I.T. makes no representations about the suitability of this software for
  any purpose.  It is provided "as is" without express or implied warranty.

  Modified for use in C-Kermit, and new material added, by:

  Jeffrey Altman <jaltman@secure-endpoints.com>
  Secure Endpoints Inc., New York City
  November 1999

  Parameterized for pty file descriptor and function code,
  Frank da Cruz, Columbia University, New York City
  Dec 2006 - Sep 2009
*/

/*
  Built and tested successully on:
   . 4.4BSD, including BSDI/OS, NetBSD, FreeBSD, OpenBSD, Mac OS X
   . AIX 4.1 and later
   . DG/UX 5.4R4.11
   . Digital UNIX 3.2 and 4.0
   . HP-UX 9.00 and later
   . IRIX 6.0 and later
   . Linux
   . Mac OS X 10.4
   . NeXTSTEP 3.x
   . OpenBSD
   . QNX 4.25 (except PTY process termination not detected)
   . SCO OSR5.0.5
   . SCO Unixware 7
   . SINIX 5.42
   . Solaris 2.x and 7
   . SunOS 4.1.3

  Failures include:
   . SCO UNIX 3.2v4.2 (compile fails with syntax error in <memory.h>)
   . HP-UX 8.00 and earlier (no vhangup or ptsname routines)
*/

#include "ckcsym.h"
#include "ckcdeb.h"			/* To pick up NETPTY definition */

#ifndef NETPTY				/* Selector for PTY support */

char * ptyver = "No PTY support";

#else  /* (rest of this module...) */

char * ptyver = "PTY support 8.0.016, 22 Aug 2007";

/* These will no doubt need adjustment... */

#ifndef NEXT
#define HAVE_SETSID
#endif /* NEXT */
#define HAVE_KILLPG
#define HAVE_TTYNAME
#define HAVE_WAITPID

#ifdef SUNOS41
#define BSD44ORPOSIX
#endif	/* SUNOS41 */

#ifndef USE_TERMIO
#ifdef LINUX
#define USE_TERMIO
#else
#ifdef ATTSV
#define USE_TERMIO
#else
#ifdef HPUX
#define USE_TERMIO
#else
#ifdef AIX
#define USE_TERMIO
#else
#ifdef BSD44ORPOSIX
#define USE_TERMIO
#else
#ifdef IRIX60
#define USE_TERMIO
#else
#ifdef QNX
#define USE_TERMIO
#endif /* QNX */
#endif /* IRIX60 */
#endif /* BSD44ORPOSIX */
#endif /* AIX */
#endif /* HPUX */
#endif /* ATTSV */
#endif /* LINUX */
#endif /* USE_TERMIO */

#ifdef QNX
#include <fcntl.h>
#endif /* QNX */

#ifdef USE_TERMIO
#define POSIX_TERMIOS			/* Seems to be a misnomer */
#endif /* USE_TERMIO */

#ifdef NEXT
#ifndef GETPGRP_ONEARG
#define GETPGRP_ONEARG
#endif /* GETPGRP_ONEARG */
#endif /* NEXT */

#ifdef WANT_UTMP			/* See ckupty.h */
/*
  WANT_UTMP is not defined because (a) the utmp/wtmp junk is the most
  nonportable part of this module, and (b) we're not logging anybody
  in, we're just running a process, and don't need to write utmp/wtmp records.
*/
#ifndef HAVE_SETUTXENT			/* Who has <utmpx.h> */
#ifdef SOLARIS
#define HAVE_SETUTXENT
#else
#ifdef IRIX60
#define HAVE_SETUTXENT
#else
#ifdef CK_SCOV5
#define HAVE_SETUTXENT
#else
#ifdef HPUX10
#define HAVE_SETUTXENT
#else
#ifdef UNIXWARE
#define HAVE_SETUTXENT
#else
#ifdef IRIX60
#define HAVE_SETUTXENT
#endif /* IRIX60 */
#endif /* UNIXWARE */
#endif /* HPUX10 */
#endif /* CK_SCOV5 */
#endif /* IRIX60 */
#endif /* SOLARIS */
#endif /* HAVE_SETUTXENT */

#ifndef HAVE_UTHOST			/* Does utmp include ut_host[]? */
#ifdef HAVE_SETUTXENT			/* utmpx always does */
#define HAVE_UTHOST
#else
#ifdef LINUX				/* Linux does */
#define HAVE_UTHOST
#else
#ifdef SUNOS4				/* SunOS does */
#define HAVE_UTHOST
#else
#ifdef AIX41				/* AIX 4.1 and later do */
#define HAVE_UTHOST
#endif /* AIX41 */
#endif /* SUNOS4 */
#endif /* LINUX */
#endif /* HAVE_SETUTXENT */
#endif /* HAVE_UTHOST */

#ifndef HAVE_UT_HOST
#ifndef NO_UT_HOST
#define NO_UT_HOST
#endif /* NO_UT_HOST */
#endif /* HAVE_UT_HOST */

#endif /* WANT_UTMP */

#ifdef LINUX
#define CK_VHANGUP
#define HAVE_SYS_SELECT_H
#define HAVE_GETUTENT
#define HAVE_SETUTENT
#define HAVE_UPDWTMP
#endif /* LINUX */

#ifdef HPUX10
#define CK_VHANGUP
#define VHANG_FIRST
#define HAVE_PTSNAME
#ifndef HAVE_PTYTRAP
#define HAVE_PTYTRAP
#endif /* HAVE_PTYTRAP */
#else
#ifdef HPUX9
#define CK_VHANGUP
#define VHANG_FIRST
#define HAVE_PTSNAME
#ifndef HAVE_PTYTRAP
#define HAVE_PTYTRAP
#endif /* HAVE_PTYTRAP */
#endif /* HPUX9 */
#endif /* HPUX10 */

#ifdef SUNOS4
#define CK_VHANGUP
#define NO_UT_PID
#define VHANG_FIRST
#endif /* SUNOS4 */

#ifdef IRIX60
#define CK_VHANGUP
#define HAVE__GETPTY
#endif /* IRIX60 */

#ifdef SINIX
#define HAVE_STREAMS
#define HAVE_GRANTPT
#define HAVE_PTSNAME
#define PUSH_PTEM
#define PUSH_LDTERM
#define PUSH_TTCOMPAT
#endif /* SINIX */

#ifdef ultrix
#define MUST_SETPGRP
#endif /* ultrix */

#ifdef QNX
#define MUST_SETPGRP
#define NO_DEVTTY
#define INIT_SPTY
#endif /* QNX */

#ifdef LINUX
#ifdef HAVE_PTMX
#define HAVE_GRANTPT
#define HAVE_PTSNAME
#endif /* HAVE_PTMX */
#else
#ifdef HAVE_STREAMS
#define HAVE_PTMX
#endif /* HAVE_STREAMS */
#endif /* LINUX */

#include "ckupty.h"

#ifdef PTYNOBLOCK
#ifndef O_NDELAY
#ifdef O_NONBLOCK
#define O_NDELAY O_NONBLOCK
#endif /* O_NONBLOCK */
#endif /* O_NDELAY */
#else /* PTYNOBLOCK */
#ifdef O_NDELAY
#undef O_NDELAY
#endif /* O_NDELAY */
#define O_NDELAY 0
#endif /* PTYNOBLOCK */

#ifndef ONLCR
#define ONLCR 0
#endif /* ONLCR */

#ifdef CK_WAIT_H
#include <sys/wait.h>
#endif /* CK_WAIT_H */

#ifdef STREAMSPTY
#ifndef INIT_SPTY
#define INIT_SPTY
#endif /* INIT_SPTY */

#include <sys/stream.h>
#include <stropts.h>
#include <termio.h>

/* Make sure we don't get the BSD version */

#ifdef HAVE_SYS_TTY_H
#include "/usr/include/sys/tty.h"
#endif /* HAVE_SYS_TTY_H */

#ifdef HAS_PTYVAR			/* Where is this set? */

#include <sys/ptyvar.h>

#else /* HAS_PTYVAR */

#ifndef TIOCPKT_FLUSHWRITE
#define TIOCPKT_FLUSHWRITE 0x02
#define TIOCPKT_NOSTOP     0x10
#define TIOCPKT_DOSTOP     0x20
#define TIOCPKT_IOCTL      0x40
#endif /* TIOCPKT_FLUSHWRITE */

#endif /* HAS_PTYVAR */

#ifdef HAVE_TTY_H
#include <tty.h>
#endif /* HAVE_TTY_H */

/*
  Because of the way ptyibuf is used with streams messages, we need
  ptyibuf+1 to be on a full-word boundary.  The following weirdness
  is simply to make that happen.
*/
long ptyibufbuf[BUFSIZ/sizeof(long)+1];
char *ptyibuf = ((char *)&ptyibufbuf[1])-1;
char *ptyip = ((char *)&ptyibufbuf[1])-1;
char ptyibuf2[BUFSIZ];
unsigned char ctlbuf[BUFSIZ];
struct strbuf strbufc, strbufd;

int readstream();

#else  /* ! STREAMSPTY */

/* I/O data buffers, pointers, and counters. */

char ptyibuf[BUFSIZ], *ptyip = ptyibuf;
char ptyibuf2[BUFSIZ];

#endif /* ! STREAMSPTY */

#ifndef USE_TERMIO
struct termbuf {
    struct sgttyb sg;
    struct tchars tc;
    struct ltchars ltc;
    int state;
    int lflags;
} termbuf, termbuf2;

#define cfsetospeed(tp,val) (tp)->sg.sg_ospeed = (val)
#define cfsetispeed(tp,val) (tp)->sg.sg_ispeed = (val)
#define cfgetospeed(tp)     (tp)->sg.sg_ospeed
#define cfgetispeed(tp)     (tp)->sg.sg_ispeed

#else  /* USE_TERMIO */

#ifdef SYSV_TERMIO
#define termios termio
#endif /* SYSV_TERMIO */

#ifndef TCSANOW

#ifdef TCSETS

#define TCSANOW TCSETS
#define TCSADRAIN TCSETSW
#define tcgetattr(f, t) ioctl(f, TCGETS, (char *)t)

#else /* TCSETS */

#ifdef TCSETA
#define TCSANOW TCSETA
#define TCSADRAIN TCSETAW
#define tcgetattr(f,t) ioctl(f,TCGETA,(char *)t)
#else /* TCSETA */
#define TCSANOW TIOCSETA
#define TCSADRAIN TIOCSETAW
#define tcgetattr(f,t) ioctl(f,TIOCGETA,(char *)t)
#endif /* TCSETA */

#endif /* TCSETS */

#define tcsetattr(f,a,t) ioctl(f,a,t)
#define cfsetospeed(tp,val) (tp)->c_cflag &= ~CBAUD;(tp)->c_cflag|=(val)
#define cfgetospeed(tp) ((tp)->c_cflag & CBAUD)

#ifdef CIBAUD
#define cfsetispeed(tp,val) \
 (tp)->c_cflag &= ~CIBAUD; (tp)->c_cflag |= ((val)<<IBSHIFT)
#define cfgetispeed(tp) (((tp)->c_cflag & CIBAUD)>>IBSHIFT)
#else /* CIBAUD */
#define cfsetispeed(tp,val) (tp)->c_cflag &= ~CBAUD; (tp)->c_cflag|=(val)
#define cfgetispeed(tp) ((tp)->c_cflag & CBAUD)
#endif /* CIBAUD */

#endif /* TCSANOW */

struct termios termbuf, termbuf2;       /* pty control structure */

#ifdef INIT_SPTY
static int spty = -1;
#endif /* INIT_SPTY */

#endif /* USE_TERMIO */

#ifdef QNX				/* 299 */
#ifndef IXANY
#define IXANY 0
#endif	/* IXANY */
#endif	/* QNX */

static int msg = 0;

/* Variables available to other modules */

int pty_fork_active = 0;		/* pty fork is active */
PID_T pty_fork_pid = -1;		/* pty fork pid */
int pty_slave_fd = -1;			/* pty slave file descriptor */
int pty_master_fd = -1;			/* pty master file descriptor */

/* termbuf routines (begin) */
/*
  init_termbuf()
  copy_termbuf(cp)
  set_termbuf()

  These three routines are used to get and set the "termbuf" structure
  to and from the kernel.  init_termbuf() gets the current settings.
  copy_termbuf() hands in a new "termbuf" to write to the kernel, and
  set_termbuf() writes the structure into the kernel.
*/
VOID
init_termbuf(fd) int fd; {
    int ttyfd;
    int rc = 0;

    ttyfd = fd;

#ifdef HAVE_STREAMS
    debug(F100,"init_termbuf HAVE_STREAMS","",0);
#else
    debug(F100,"init_termbuf HAVE_STREAMS NOT DEFINED","",0);
#endif	/* HAVE_STREAMS */
#ifdef STREAMSPTY
    debug(F100,"init_termbuf STREAMSPTY","",0);
#else
    debug(F100,"init_termbuf STREAMSPTY NOT DEFINED","",0);
#endif	/* STREAMSPTY */
#ifdef INIT_SPTY
    debug(F100,"init_termbuf INIT_SPTY","",0);
#else
    debug(F100,"init_termbuf INIT_SPTY NOT DEFINED","",0);
#endif	/* INIT_SPTY */

    debug(F101,"init_termbuf ttyfd","",ttyfd);
#ifdef INIT_SPTY
    debug(F101,"init_termbuf spty","",spty);
#endif	/* INIT_SPTY */

    memset(&termbuf,0,sizeof(termbuf));
    memset(&termbuf2,0,sizeof(termbuf2));
#ifndef	USE_TERMIO
    rc = ioctl(ttyfd, TIOCGETP, (char *)&termbuf.sg);
    rc |= ioctl(ttyfd, TIOCGETC, (char *)&termbuf.tc);
    rc |= ioctl(ttyfd, TIOCGLTC, (char *)&termbuf.ltc);
#ifdef TIOCGSTATE
    rc |= ioctl(ttyfd, TIOCGSTATE, (char *)&termbuf.state);
#endif /* TIOCGSTATE */
#else /* USE_TERMIO */
    errno = 0;
#ifdef INIT_SPTY
    rc = tcgetattr(spty, &termbuf);
    debug(F111,"init_termbuf() tcgetattr(spty)",ckitoa(rc),errno);
#else
    rc = tcgetattr(ttyfd, &termbuf);
    debug(F111,"init_termbuf() tcgetattr(ttyfd)",ckitoa(rc),errno);
#endif /* INIT_SPTY */
#endif /* USE_TERMIO */
    if (!rc)
      termbuf2 = termbuf;
}

#ifdef TIOCPKT_IOCTL
VOID
copy_termbuf(cp, len) char *cp; int len; {
    if (len > sizeof(termbuf))
      len = sizeof(termbuf);
    memcpy((char *)&termbuf, cp, len);
    termbuf2 = termbuf;
}
#endif /* TIOCPKT_IOCTL */

VOID
set_termbuf(fd) int fd; {		/* Only make the necessary changes. */
    int x;
    int ttyfd;
    ttyfd = fd;

    debug(F101,"set_termbuf ttyfd","",ttyfd);
#ifdef INIT_SPTY
    debug(F101,"set_termbuf spty","",spty);
#endif	/* INIT_SPTY */

#ifndef	USE_TERMIO
    debug(F100,"set_termbuf USE_TERMIO","",0);
    if (memcmp((char *)&termbuf.sg, (char *)&termbuf2.sg, sizeof(termbuf.sg)))
      ioctl(ttyfd, TIOCSETN, (char *)&termbuf.sg);
    if (memcmp((char *)&termbuf.tc, (char *)&termbuf2.tc, sizeof(termbuf.tc)))
      ioctl(ttyfd, TIOCSETC, (char *)&termbuf.tc);
    if (memcmp((char *)&termbuf.ltc, (char *)&termbuf2.ltc,
	       sizeof(termbuf.ltc)))
      ioctl(ttyfd, TIOCSLTC, (char *)&termbuf.ltc);
    if (termbuf.lflags != termbuf2.lflags)
      ioctl(ttyfd, TIOCLSET, (char *)&termbuf.lflags);
#else  /* USE_TERMIO */
    x = memcmp((char *)&termbuf, (char *)&termbuf2, sizeof(termbuf));
    debug(F101,"set_termbuf !USE_TERMIO memcmp","",x);
    x = 1;				/* Force this */
    if (x) {
	int x;
	errno = 0;
#ifdef INIT_SPTY
	debug(F100,"set_termbuf INIT_SPTY","",0);
	x = tcsetattr(spty, TCSANOW, &termbuf);
	debug(F111,"set_termbuf tcsetattr(spty)",ckitoa(x),errno);
#else
	debug(F100,"set_termbuf !INIT_SPTY","",0);
	x = tcsetattr(ttyfd, TCSANOW, &termbuf);
	debug(F111,"set_termbuf tcsetattr(ttyfd)",ckitoa(x),errno);
#endif /* INIT_SPTY */
    }
#endif /* USE_TERMIO */
}
/* termbuf routines (end) */

VOID
ptyint_vhangup() {
#ifdef CK_VHANGUP
#ifdef CK_POSIX_SIG
    struct sigaction sa;
    /* Initialize "sa" structure. */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = SIG_IGN;
    sigaction(SIGHUP, &sa, (struct sigaction *)0);
    vhangup();
    sa.sa_handler = SIG_DFL;
    sigaction(SIGHUP, &sa, (struct sigaction *)0);
#else /* CK_POSIX_SIG */
    signal(SIGHUP,SIG_IGN);
    vhangup();
    signal(SIGHUP,SIG_DFL);
#endif /* CK_POSIX_SIG */
#endif /* CK_VHANGUP */
}

/*
  This routine is called twice.  It's not particularly important that the
  setsid() or TIOCSCTTY ioctls succeed (they may not the second time), but
  rather that we have a controlling terminal at the end.  It is assumed that
  vhangup doesn't exist and confuse the process's notion of controlling
  terminal on any system without TIOCNOTTY.  That is, either vhangup() leaves
  the controlling terminal in tact, breaks the association completely, or the
  system provides TIOCNOTTY to get things back into a reasonable state.  In
  practice, vhangup() either breaks the association completely or doesn't
  effect controlling terminals, so this condition is met.
*/
long
ptyint_void_association() {
    int con_fd;
#ifdef HAVE_SETSID
    debug(F110,
	  "ptyint_void_association()",
	  "setsid()",
	  0
	  );
    setsid();
#endif /* HAVE_SETSID */

#ifndef NO_DEVTTY
    /* Void tty association first */
#ifdef TIOCNOTTY
    con_fd = open("/dev/tty", O_RDWR);
    debug(F111,
	  "ptyint_void_association() open(/dev/tty,O_RDWR)",
	  "/dev/tty",
	  con_fd);
    if (con_fd >= 0) {
        ioctl(con_fd, TIOCNOTTY, 0);
        close(con_fd);
    }
#ifdef DEBUG
    else debug(F101, "ptyint_void_association() open() errno","",errno);
#endif /* DEBUG */
#endif /* TIOCNOTTY */
#endif /* NO_DEVTTY */
    return(0);
}

/* PID may be zero for unknown.*/

long
pty_cleanup(slave, pid, update_utmp) char *slave; int pid; int update_utmp; {
#ifdef VHANG_LAST
    int retval, fd;
#endif /* VHANG_LAST */

    debug(F111,"pty_cleanup()",slave,pid);
#ifdef WANT_UTMP
    if (update_utmp)
      pty_update_utmp(PTY_DEAD_PROCESS,
		      0,
		      "",
		      slave,
		      (char *)0,
		      PTY_UTMP_USERNAME_VALID
		      );
#endif /* WANT_UTMP */

#ifdef SETUID
    chmod(slave, 0666);
    chown(slave, 0, 0);
#endif /* SETUID */

#ifdef HAVE_REVOKE
    revoke(slave);
    /*
       Revoke isn't guaranteed to send a SIGHUP to the processes it
       dissociates from the terminal.  The best solution without a Posix
       mechanism for forcing a hangup is to killpg() the process group of the
       pty.  This will at least kill the shell and hopefully, the child
       processes.  This is not always the case, however.  If the shell puts
       each job in a process group and doesn't pass along SIGHUP, all
       processes may not die.
    */
    if (pid > 0) {
#ifdef HAVE_KILLPG
	killpg(pid, SIGHUP);
#else
	kill(-(pid), SIGHUP);
#endif /*HAVE_KILLPG*/
    }
#else /* HAVE_REVOKE*/
#ifdef VHANG_LAST
    {
        int status;
#ifdef CK_POSIX_SIG
        sigset_t old, new;
        sigemptyset(&new);
        sigaddset(&new, SIGCHLD);
        sigprocmask(SIG_BLOCK, &new, &old);
#else /*CK_POSIX_SIG*/
        int mask = sigblock(sigmask(SIGCHLD));
#endif /*CK_POSIX_SIG*/
        switch (retval = fork()) {
	  case -1:
#ifdef CK_POSIX_SIG
            sigprocmask(SIG_SETMASK, &old, 0);
#else /*CK_POSIX_SIG*/
            sigsetmask(mask);
#endif /*CK_POSIX_SIG*/
            return errno;
	  case 0:
            ptyint_void_association();
            if (retval = (pty_open_ctty(slave, &fd, -1)))
	      exit(retval);
            ptyint_vhangup();
            exit(0);
	    break;
	  default:
#ifdef HAVE_WAITPID
            waitpid(retval, &status, 0);
#else /*HAVE_WAITPID*/
            wait(&status);
#endif /* HAVE_WAITPID */
#ifdef CK_POSIX_SIG
            sigprocmask(SIG_SETMASK, &old, 0);
#else /*CK_POSIX_SIG*/
            sigsetmask(mask);
#endif /*CK_POSIX_SIG*/
            break;
        }
    }
#endif /*VHANG_LAST*/
#endif /* HAVE_REVOKE*/
#ifndef HAVE_STREAMS
    slave[strlen("/dev/")] = 'p';
#ifdef SETUID
    chmod(slave, 0666);
    chown(slave, 0, 0);
#endif /* SETUID */
#endif /* HAVE_STREAMS */
    return(0);
}

long
pty_getpty(fd, slave, slavelength) int slavelength; int *fd; char *slave; {
    char *cp;
    char *p;
    int i, ptynum;
    struct stat stb;
#ifndef HAVE_OPENPTY
#ifndef HAVE__GETPTY
    char slavebuf[1024];
#endif /* HAVE__GETPTY */
#endif /* HAVE_OPENPTY */
#ifdef HAVE__GETPTY
    char *slaveret;			/* Temp to hold pointer to slave */
#endif /*HAVE__GETPTY*/

#ifdef HAVE_OPENPTY
    int slavefd;

    pty_master_fd = -1;
    debug(F100,"HAVE_OPENPTY","",0);
    if (openpty(fd,
		&slavefd,
		slave,
		(struct termios *)0,
		(struct winsize *)0
		)
	) {
	pty_master_fd = *fd;
	return(1);
    }
    close(slavefd);
    return(0);

#else /* HAVE_OPENPTY */

#ifdef HAVE__GETPTY
/*
  This code is included for Irix; as of version 5.3, Irix has /dev/ptmx, but
  it fails to work properly; even after calling unlockpt, root gets permission
  denied opening the pty.  The code to support _getpty should be removed if
  Irix gets working streams ptys in favor of maintaining the least needed code
  paths.
*/
    debug(F100,"HAVE__GETPTY","",0);
    if ((slaveret = _getpty(fd, O_RDWR | O_NDELAY, 0600, 0)) == 0) {
	*fd = -1;
	return(PTY_GETPTY_NOPTY);
    }
    if (strlen(slaveret) > slavelength - 1) {
	close(*fd);
	*fd = -1;
	return(PTY_GETPTY_SLAVE_TOOLONG);
    } else {
	ckstrncpy(slave, slaveret, slavelength);
    }
    return(0);

#else /* HAVE__GETPTY */

    *fd = open("/dev/ptym/clone", O_RDWR|O_NDELAY); /* HPUX */
    if (*fd >= 0) {
        debug(F110,"pty_getpty()","open(/dev/ptym/clone) success",0);
        goto have_fd;
    }

#ifdef HAVE_PTMX
    debug(F100,"HAVE_PTMX","",0);
    *fd = open("/dev/ptmx",O_RDWR|O_NDELAY);
    if (*fd >= 0) {
        debug(F110,"pty_getpty()","open(/dev/ptmx) success",0);
        goto have_fd;
    }
#endif /* HAVE_PTMX */

    *fd = open("/dev/ptc", O_RDWR|O_NDELAY); /* AIX */
    if (*fd >= 0) {
        debug(F110,"pty_getpty()","open(/dev/ptc) success",0);
        goto have_fd;
    }
    *fd = open("/dev/pty", O_RDWR|O_NDELAY); /* sysvimp */
    if (*fd >= 0)
        debug(F110,"pty_getpty()","open(/dev/pty) success",0);

  have_fd:
    /* This would be the pty master */
    debug(F101,"pty_getpty fd(A)","",*fd);
    if (*fd >= 0) {
	pty_master_fd = *fd;

#ifdef HAVE_GRANTPT
#ifdef HAVE_PTMX
        debug(F100,"HAVE_GRANTPT","",0);
	if (grantpt(*fd) || unlockpt(*fd))
	  return(PTY_GETPTY_STREAMS);
#endif /* HAVE_PTMX */
#endif /* HAVE_GRANTPT */

#ifdef HAVE_PTSNAME
        debug(F100,"HAVE_PTSNAME","",0);
	p = (char *)ptsname(*fd);
        debug(F110,"pty_getpty() ptsname()",p,0);
#else
#ifdef HAVE_TTYNAME
        debug(F100,"HAVE_TTYNAME","",0);
	p = ttyname(*fd);
        debug(F110,"pty_getpty() ttyname()",p,0);
#else
	/* If we don't have either what do we do? */
  	return(PTY_GETPTY_NOPTY);	/* punt */
#endif /* HAVE_TTYNAME */
#endif /* HAVE_PTSNAME */
	if (p) {
	    if (strlen(p) > slavelength - 1) {
                close (*fd);
                *fd = -1;
                return(PTY_GETPTY_SLAVE_TOOLONG);
	    }
	    ckstrncpy(slave, p, slavelength);
	    return(0);
	}
	if (fstat(*fd, &stb) < 0) {
	    close(*fd);
	    return(PTY_GETPTY_FSTAT);
	}
	ptynum = (int)(stb.st_rdev&0xFF);
	sprintf(slavebuf, "/dev/ttyp%x", ptynum); /* safe */
	if (strlen(slavebuf) > slavelength - 1) {
	    close(*fd);
	    *fd = -1;
	    return(PTY_GETPTY_SLAVE_TOOLONG);
	}
        debug(F110,"pty_getpty() slavebuf",slavebuf,0);
	ckstrncpy(slave, slavebuf, slavelength);
	return(0);
    } else {
    	for (cp = "pqrstuvwxyzPQRST";*cp; cp++) {
	    sprintf(slavebuf,"/dev/ptyXX"); /* safe */
	    slavebuf[sizeof("/dev/pty") - 1] = *cp;
	    slavebuf[sizeof("/dev/ptyp") - 1] = '0';
	    if (stat(slavebuf, &stb) < 0)
	      break;
	    for (i = 0; i < 16; i++) {
		slavebuf[sizeof("/dev/ptyp") - 1] = "0123456789abcdef"[i];
		errno = 0;
		*fd = open(slavebuf, O_RDWR|O_NDELAY);
		if (*fd < 0) {
		    debug(F111,"pty_getpty() pty master open error",
			  slavebuf,errno);
		    continue;
		}
                debug(F111,"pty_getpty() found pty master",slavebuf,*fd);
		slavebuf[sizeof("/dev/") - 1] = 't'; /* got pty */
		if (strlen(slavebuf) > slavelength -1) {
		    close(*fd);
		    *fd = -1;
		    return(PTY_GETPTY_SLAVE_TOOLONG);
		}
		ckstrncpy(slave, slavebuf, slavelength);
                debug(F110,"pty_getpty slave name",slave,0);
		pty_master_fd = *fd;
		return(0);
	    }
	}
	return(PTY_GETPTY_NOPTY);
    }
#endif /*HAVE__GETPTY*/
#endif /* HAVE_OPENPTY */
}

long
pty_init() {
#ifdef HAVE_PTYM
    static char dummy;
    debug(F100,"HAVE_PTYM","",0);
    tty_bank =  &master_name[strlen("/dev/ptym/pty")];
    tty_num  =  &master_name[strlen("/dev/ptym/ptyX")];
    slave_bank = &slave_name[strlen("/dev/pty/tty")];
    slave_num  = &slave_name[strlen("/dev/pty/ttyX")];
#endif
    return(0L);
}

/*
  The following is an array of modules that should be pushed on the stream.
  See configure.in for caviats and notes about when this array is used and not
  used.
*/
#ifdef HAVE_STREAMS
#ifndef HAVE_LINE_PUSH
static char *push_list[] = {
#ifdef PUSH_PTEM
    "ptem",
#endif
#ifdef PUSH_LDTERM
    "ldterm",
#endif
#ifdef PUSH_TTCOMPAT
    "ttcompat",
#endif
    0
};
#endif /* HAVE_LINE_PUSH */
#endif /* HAVE_STREAMS */

long
pty_initialize_slave (fd) int fd; {
#ifdef POSIX_TERMIOS
#ifndef ultrix
    struct termios new_termio;
#else
    struct sgttyb b;
#endif /* ultrix */
#else
    struct sgttyb b;
#endif /* POSIX_TERMIOS */
    int pid;
#ifdef POSIX_TERMIOS
#ifndef ultrix
    int rc;
#endif /* ultrix */
#endif /* POSIX_TERMIOS */

    debug(F111,"pty_initialize_slave()","fd",fd);

#ifdef HAVE_STREAMS
#ifdef HAVE_LINE_PUSH
    while (ioctl(fd,I_POP,0) == 0) ;	/* Clear out any old lined's */

    if (line_push(fd) < 0) {
        debug(F110,"pty_initialize_slave()","line_push() failed",0);
	close(fd);
        fd = -1;
        return(PTY_OPEN_SLAVE_LINE_PUSHFAIL);
    }
#else /*No line_push */
    {
        char **module = &push_list[0];
        while (*module) {
	    if (ioctl(fd, I_PUSH, *(module++)) < 0) {
                debug(F110,"pty_initialize_slave()","ioctl(I_PUSH) failed",0);
		return(PTY_OPEN_SLAVE_PUSH_FAIL);
	    }
	}
    }
#endif /*LINE_PUSH*/
#endif /*HAVE_STREAMS*/
/*
  Under Ultrix 3.0, the pgrp of the slave pty terminal needs to be set
  explicitly.  Why rlogind works at all without this on 4.3BSD is a mystery.
*/
#ifdef GETPGRP_ONEARG
    pid = getpgrp(getpid());
#else
    pid = getpgrp();
#endif /* GETPGRP_ONEARG */

    debug(F111,"pty_initialize_slave()","pid",pid);

#ifdef TIOCSPGRP
    ioctl(fd, TIOCSPGRP, &pid);
#endif /* TIOCSPGRP */

#ifdef POSIX_TERMIOS
#ifndef ultrix
    tcsetpgrp(fd, pid);
    errno = 0;
    rc = tcgetattr(fd,&new_termio);
    debug(F111,"pty_initialize_slave tcgetattr(fd)",ckitoa(rc),errno);
    if (rc == 0) {
	new_termio.c_cc[VMIN] = 1;
	new_termio.c_cc[VTIME] = 0;
	rc = tcsetattr(fd,TCSANOW,&new_termio);
	debug(F111,"pty_initialize_slave tcsetattr(fd)",ckitoa(rc),errno);
    }
#endif /* ultrix */
#endif /* POSIX_TERMIOS */
    return(0L);
}

#ifdef WANT_UTMP
long
pty_logwtmp (tty, user, host) char *user, *tty, *host; {
#ifdef HAVE_LOGWTMP
    logwtmp(tty,user,host);
    return(0);
#else
    struct utmp ut;
    char *tmpx;
    char utmp_id[5];
    int loggingin = user[0];		/* Will be empty for logout */

#ifndef NO_UT_HOST
    strncpy(ut.ut_host, host, sizeof(ut.ut_host));
#endif /* NO_UT_HOST */

    strncpy(ut.ut_line, tty, sizeof(ut.ut_line));
    ut.ut_time = time(0);

#ifndef NO_UT_PID
    ut.ut_pid = getpid();
    strncpy(ut.ut_user, user, sizeof(ut.ut_user));

    tmpx = tty + strlen(tty) - 2;
    ckmakmsg(utmp_id,5,"kr",tmpx,NULL,NULL);
    strncpy(ut.ut_id, utmp_id, sizeof(ut.ut_id));
    ut.ut_pid = (loggingin ? getpid() : 0);
    ut.ut_type = (loggingin ? USER_PROCESS : DEAD_PROCESS);
#else
    strncpy(ut.ut_name, user, sizeof(ut.ut_name));
#endif /* NO_UT_PID */

    return(ptyint_update_wtmp(&ut, host, user));

#endif /* HAVE_LOGWTMP */
}
#endif /* WANT_UTMP */

/*
  This routine is called twice.  It's not particularly important that the
  setsid() or TIOCSCTTY ioctls succeed (they may not the second time), but
  rather that we have a controlling terminal at the end.  It is assumed that
  vhangup doesn't exist and confuse the process's notion of controlling
  terminal on any system without TIOCNOTTY.  That is, either vhangup() leaves
  the controlling terminal in tact, breaks the association completely, or the
  system provides TIOCNOTTY to get things back into a reasonable state.  In
  practice, vhangup() either breaks the association completely or doesn't
  effect controlling terminals, so this condition is met.
*/
long
pty_open_ctty(slave, fd, fc) char * slave; int *fd; int fc; {
    int retval;

    debug(F110,"pty_open_ctty() slave",slave,0);

/* First, dissociate from previous terminal */

    if ((retval = ptyint_void_association()) != 0) {
        debug(F111,
	      "pty_open_ctty()",
	      "ptyint_void_association() failed",
	      retval
	      );
	return(retval);
    }
#ifdef MUST_SETPGRP
/*
  The Ultrix (and other BSD tty drivers) require the process group
  to be zero in order to acquire the new tty as a controlling tty.
*/
    setpgrp(0,0);
    debug(F101,"pty_open_ctty MUST_SETPGRP setpgrp(0,0)","",errno);
#endif /* MUST_SETPGRP */

    errno = 0;
    *fd = open(slave, O_RDWR);
    debug(F111,"pty_open_ctty open(slave) fd",slave,*fd);
    if (*fd < 0) {
	debug(F111,"pty_open_ctty() open failure", slave, errno);
	return(PTY_OPEN_SLAVE_OPENFAIL);
    }
#ifdef SOLARIS
    /* This forces the job to have a controlling terminal. */
    close(*fd);
    *fd = open(slave, O_RDWR);
    debug(F111,"pty_open_ctty close/open(slave) fd",slave,*fd);
#ifdef DEBUG
    /* This shows that /dev/tty exists */
if (deblog) {
	int x;
	x = open("/dev/tty", O_RDWR);
	debug(F111,"pty_open_ctty open(/dev/tty) fd",slave,x);
	if (x < 0) debug(F111,"pty_open_ctty open(/dev/tty) errno","",errno);
	debug(F110,"pty_open_ctty ttyname(/dev/tty)",ttyname(x),0);
	if (x > -1) close(x);
    }
#endif	/* DEBUG */
#endif /* SOLARIS */

#ifdef MUST_SETPGRP
    setpgrp(0, getpid());
#endif /* MUST_SETPGRP */

#ifdef TIOCSCTTY
    if (
#ifdef COMMENT
	fc == 0
#else
	1
#endif	/* COMMENT */
	) {
	/* TIOCSCTTY = Make this the job's controlling terminal */
	errno = 0;
	retval = ioctl(*fd, TIOCSCTTY, 0); /* Don't check return.*/
	debug(F111,"pty_open_ctty() ioctl TIOCSCTTY",ckitoa(retval),errno);
    }
#endif /* TIOCSCTTY */
    return(0L);
}

long
pty_open_slave(slave, fd, fc) char *slave; int *fd; int fc; {
    int vfd, testfd;
    long retval;
#ifdef CK_POSIX_SIG
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);		/* Initialize "sa" structure. */
    sa.sa_flags = 0;
#endif /* CK_POSIX_SIG */

/*
  First, chmod and chown the slave.  If we have vhangup then we really need
  pty_open_ctty to make sure our controlling terminal is the pty we're
  opening.  However, if we are using revoke or nothing then we just need a
  file descriiptor for the pty.  Considering some OSes in this category break
  on the second call to open_ctty (currently OSF but others may), we simply
  use a descriptor if we can.
*/
#ifdef VHANG_FIRST
    if ((retval = pty_open_ctty(slave, &vfd, fc)) != 0) {
        debug(F111,
	      "pty_open_slave() VHANG_FIRST",
	      "pty_open_ctty() failed",
	      retval
	      );
        return(retval);
    }
    if (vfd < 0) {
        debug(F111,
	      "pty_open_slave() VHANG_FIRST",
	      "PTY_OPEN_SLAVE_OPENFAIL",
	      vfd
	      );
	return(PTY_OPEN_SLAVE_OPENFAIL);
    }
#endif /* VHANG_FIRST */

    if (slave == NULL || *slave == '\0') {
        debug(F110,"pty_open_slave()","PTY_OPEN_SLAVE_TOOSHORT",0);
        return(PTY_OPEN_SLAVE_TOOSHORT);
    }

#ifdef SETUID
    if (chmod(slave, 0)) {
        debug(F110,"pty_open_slave()","PTY_OPEN_SLAVE_CHMODFAIL",0);
        return(PTY_OPEN_SLAVE_CHMODFAIL);
    }
    if (chown(slave, 0, 0 ) == -1 ) {
        debug(F110,"pty_open_slave()","PTY_OPEN_SLAVE_CHOWNFAIL",0);
        return(PTY_OPEN_SLAVE_CHOWNFAIL);
    }
#endif /* SETUID */
#ifdef VHANG_FIRST
    ptyint_vhangup();
    close(vfd);
#endif /* VHANG_FIRST */

    if ((retval = ptyint_void_association()) != 0) {
        debug(F111,
	      "pty_open_slave()",
	      "ptyint_void_association() failed",
	      retval
	      );
        return(retval);
    }

#ifdef HAVE_REVOKE
    if (revoke (slave) < 0 ) {
        debug(F110,"pty_open_slave()","PTY_OPEN_SLAVE_REVOKEFAIL",0);
	return(PTY_OPEN_SLAVE_REVOKEFAIL);
    }
#endif /* HAVE_REVOKE */

/* Open the pty for real. */

    retval = pty_open_ctty(slave, fd, fc);
    debug(F111,"pty_open_slave retval",slave,retval);
    debug(F111,"pty_open_slave fd",slave,*fd);
    if (retval != 0) {
        debug(F111,"pty_open_slave()","pty_open_ctty() failed",retval);
	return(PTY_OPEN_SLAVE_OPENFAIL);
    }
    pty_slave_fd = *fd;		   /* This is not visible to the upper fork */
    debug(F111,"pty_open_slave fd ctty'd",slave,pty_slave_fd);
    retval = pty_initialize_slave(*fd);
    debug(F111,"pty_open_slave fd init'd",slave,pty_slave_fd);
    if (retval) {
        debug(F111,"pty_open_slave()","pty_initialize_slave() failed",retval);
        return(retval);
    }
    /* (VOID)pty_make_raw(*fd); */

    debug(F100,"pty_open_slave OK","",*fd);
    return(0L);
}

#ifdef WANT_UTMP

#ifndef UTMP_FILE
#ifdef _PATH_UTMP
#define UTMP_FILE _PATH_UTMP
#endif /* _PATH_UTMP */
#endif /*  UTMP_FILE */

/* If it is *still* missing, assume /etc/utmp */

#ifndef UTMP_FILE
#define	UTMP_FILE "/etc/utmp"
#endif /* UTMP_FILE */

#ifndef NO_UT_PID
#define WTMP_REQUIRES_USERNAME
#endif /* NO_UT_PID */

long
pty_update_utmp(process_type, pid, username, line, host, flags)
    int process_type;
    int pid;
    char *username, *line, *host;
    int flags;
/* pty_update_utmp */ {
    struct utmp ent, ut;
#ifndef HAVE_SETUTENT
    struct stat statb;
    int tty;
#endif /* HAVE_SETUTENT */
#ifdef HAVE_SETUTXENT
    struct utmpx utx;
#endif /* HAVE_SETUTXENT */
#ifndef NO_UT_PID
    char *tmpx;
    char utmp_id[5];
#endif /* NO_UT_PID */
    char userbuf[32];
    int fd;

    debug(F100,"pty_update_utmp()","",0);
    strncpy(ent.ut_line, line+sizeof("/dev/")-1, sizeof(ent.ut_line));
    ent.ut_time = time(0);

#ifdef NO_UT_PID
    if (process_type == PTY_LOGIN_PROCESS)
      return(0L);
#else /* NO_UT_PID */

    ent.ut_pid = pid;

    switch (process_type) {
      case PTY_LOGIN_PROCESS:
	ent.ut_type = LOGIN_PROCESS;
	break;
      case PTY_USER_PROCESS:
	ent.ut_type = USER_PROCESS;
	break;
      case PTY_DEAD_PROCESS:
	ent.ut_type = DEAD_PROCESS;
	break;
      default:
	return(PTY_UPDATE_UTMP_PROCTYPE_INVALID);
    }
#endif /*NO_UT_PID*/

#ifndef NO_UT_HOST
    if (host)
      strncpy(ent.ut_host, host, sizeof(ent.ut_host));
    else
      ent.ut_host[0] = '\0';
#endif /* NO_UT_HOST */

#ifndef NO_UT_PID
    if (!strcmp (line, "/dev/console")) {
	char * s = NULL;

#ifdef sun
#ifdef __SVR4
	s = "co";
#else
	s = "cons";
#endif /* __SVR4 */
#else
	s = "cons";
#endif /* sun */

	strncpy(ent.ut_id, s, 4);

    } else {

	tmpx = line + strlen(line)-1;
	if (*(tmpx-1) != '/') tmpx--;	/* last 2 chars unless it's a '/' */
#ifdef __hpux
	ckstrncpy(utmp_id, tmpx, 5);
#else
	ckmakmsg(utmp_id,5,"kl",tmpx,NULL,NULL);
#endif /* __hpux */
	strncpy(ent.ut_id, utmp_id, sizeof(ent.ut_id));
    }
    strncpy(ent.ut_user, username, sizeof(ent.ut_user));

#else

    strncpy(ent.ut_name, username, sizeof(ent.ut_name));

#endif /* NO_UT_PID */

    if (username[0])
      strncpy(userbuf, username, sizeof(userbuf));
    else
      userbuf[0] = '\0';

#ifdef HAVE_SETUTENT

    utmpname(UTMP_FILE);
    setutent();
/*
  If we need to preserve the user name in the wtmp structure and Our flags
  tell us we can obtain it from the utmp and we succeed in obtaining it, we
  then save the utmp structure we obtain, write out the utmp structure and
  change the username pointer so it is used by update_wtmp.
*/

#ifdef WTMP_REQUIRES_USERNAME
    if ((!username[0]) && (flags&PTY_UTMP_USERNAME_VALID) &&line) {
	struct utmp *utptr;
	strncpy(ut.ut_line, line, sizeof(ut.ut_line));
	utptr = getutline(&ut);
	if (utptr)
	  strncpy(userbuf,utptr->ut_user,sizeof(ut.ut_user));
    }
#endif /* WTMP_REQUIRES_USERNAME */

    pututline(&ent);
    endutent();

#ifdef HAVE_SETUTXENT
    setutxent();
#ifdef HAVE_GETUTMPX
    getutmpx(&ent, &utx);
#else /* HAVE_GETUTMPX */
    /* For platforms like HPUX and Dec Unix which don't have getutmpx */
    strncpy(utx.ut_user, ent.ut_user, sizeof(ent.ut_user));
    strncpy(utx.ut_id, ent.ut_id, sizeof(ent.ut_id));
    strncpy(utx.ut_line, ent.ut_line, sizeof(ent.ut_line));
    utx.ut_pid = pid;		/* kludge for Irix, etc. to avoid trunc. */
    utx.ut_type = ent.ut_type;
#ifdef UT_EXIT_STRUCTURE_DIFFER
    utx.ut_exit.ut_exit = ent.ut_exit.e_exit;
#else /* UT_EXIT_STRUCTURE_DIFFER */
/* KLUDGE for now; eventually this will be a feature test... See PR#[40] */
#ifdef __hpux
    utx.ut_exit.__e_termination = ent.ut_exit.e_termination;
    utx.ut_exit.__e_exit = ent.ut_exit.e_exit;
#else /* __hpux */
    /* XXX do nothing for now; we don't even know the struct member exists */
#endif /* __hpux */
#endif /* UT_EXIT_STRUCTURE_DIFFER */
    utx.ut_tv.tv_sec = ent.ut_time;
    utx.ut_tv.tv_usec = 0;
#endif /* HAVE_GETUTMPX */
    if (host)
      strncpy(utx.ut_host, host, sizeof(utx.ut_host));
    else
      utx.ut_host[0] = 0;
    pututxline(&utx);
    endutxent();
#endif /* HAVE_SETUTXENT */

#else /* HAVE_SETUTENT */
    if (flags&PTY_TTYSLOT_USABLE) {
	tty = ttyslot();
    } else {
	int lc;
	tty = -1;
	if ((fd = open(UTMP_FILE, O_RDWR)) < 0)
	  return(errno);
	for (lc = 0;
	     lseek(fd, (off_t)(lc * sizeof(struct utmp)), SEEK_SET) != -1;
	     lc++
	     ) {
	    if (read(fd,
		     (char *)&ut,
		     sizeof(struct utmp)
		     ) != sizeof(struct utmp)
		)
	      break;
	    if (strncmp(ut.ut_line, ent.ut_line, sizeof(ut.ut_line)) == 0) {
		tty = lc;
#ifdef WTMP_REQUIRES_USERNAME
		if (!username&&(flags&PTY_UTMP_USERNAME_VALID))
		  strncpy(userbuf, ut.ut_user, sizeof(ut.ut_user));
#endif /* WTMP_REQUIRES_USERNAME */
		break;
	    }
	}
	close(fd);
    }
    if (tty > 0 && (fd = open(UTMP_FILE, O_WRONLY, 0)) >= 0) {
	lseek(fd, (off_t)(tty * sizeof(struct utmp)), SEEK_SET);
	write(fd, (char *)&ent, sizeof(struct utmp));
	close(fd);
    }
#endif /* HAVE_SETUTENT */

    /* Don't record LOGIN_PROCESS entries. */
    if (process_type == PTY_LOGIN_PROCESS)
      return(0);
    else
      return(ptyint_update_wtmp(&ent, host, userbuf));
}
#ifndef WTMP_FILE
#ifdef _PATH_WTMP
#define WTMP_FILE _PATH_WTMP
#endif /* _PATH_WTMP */
#endif /* WTMP_FILE */

#ifndef WTMPX_FILE
#ifdef _PATH_WTMPX
#ifdef HAVE_UPDWTMPX
#define WTMPX_FILE _PATH_WTMPX
#endif /* HAVE_UPDWTMPX */
#endif /* _PATH_WTMPX */
#endif /* WTMPX_FILE */

/* If it is *still* missing, assume /usr/adm/wtmp */

#ifndef WTMP_FILE
#define	WTMP_FILE "/usr/adm/wtmp"
#endif /* WTMP_FILE */

#ifdef COMMENT
/* The following test can not be made portably */

/* #if defined(__GLIBC__) && (__GLIBC__ >= 2) && (__GLIBC_MINOR__ >= 1) */
/*
  This is ugly, but the lack of standardization in the utmp/utmpx space, and
  what glibc implements and doesn't make available, is even worse.
*/
/* #undef HAVE_UPDWTMPX */	/* Don't use updwtmpx for glibc 2.1 */
/* #endif */ /* __GLIBC__ etc */

#else  /* COMMENT */

#ifdef __GLIBC__
#undef HAVE_UPDWTMPX		/* Don't use updwtmpx for glibc period */
#endif /* __GLIBC__ */
#endif /* COMMENT */

long
ptyint_update_wtmp(ent,host,user) struct utmp *ent; char *host; char *user; {
    struct utmp ut;
    struct stat statb;
    int fd;
    time_t uttime;
#ifdef HAVE_UPDWTMPX
    struct utmpx utx;

    getutmpx(ent, &utx);
    if (host)
      strncpy(utx.ut_host, host, sizeof(utx.ut_host) );
    else
      utx.ut_host[0] = 0;
    if (user)
      strncpy(utx.ut_user, user, sizeof(utx.ut_user));
    updwtmpx(WTMPX_FILE, &utx);
#endif /* HAVE_UPDWTMPX */

#ifdef HAVE_UPDWTMP
#ifndef HAVE_UPDWTMPX
    /* This is already performed byupdwtmpx if present.*/
    updwtmp(WTMP_FILE, ent);
#endif /* HAVE_UPDWTMPX*/
#else /* HAVE_UPDWTMP */

    if ((fd = open(WTMP_FILE, O_WRONLY|O_APPEND, 0)) >= 0) {
	if (!fstat(fd, &statb)) {
	    memset((char *)&ut, 0, sizeof(ut));
#ifdef __hpux
	    strncpy(ut.ut_id, ent->ut_id, sizeof (ut.ut_id));
#endif /* __hpux */
	    strncpy(ut.ut_line, ent->ut_line, sizeof(ut.ut_line));
	    strncpy(ut.ut_name, ent->ut_name, sizeof(ut.ut_name));
#ifndef NO_UT_HOST
	    strncpy(ut.ut_host, ent->ut_host, sizeof(ut.ut_host));
#endif /* NO_UT_HOST */

	    time(&uttime);
	    ut.ut_time = uttime;

#ifdef HAVE_GETUTENT
#ifdef USER_PROCESS
	    if (ent->ut_name) {
		if (!ut.ut_pid)
		  ut.ut_pid = getpid();
#ifndef __hpux
		ut.ut_type = USER_PROCESS;
#else  /* __hpux */
		ut.ut_type = ent->ut_type;
#endif /* __hpux */

	    } else {

#ifdef EMPTY
		ut.ut_type = EMPTY;
#else
		ut.ut_type = DEAD_PROCESS; /* For Linux brokenness*/
#endif /* EMPTY */

	    }
#endif /* USER_PROCESS */
#endif /* HAVE_GETUTENT */

	    if (write(fd, (char *)&ut, sizeof(struct utmp)) !=
		sizeof(struct utmp))
#ifndef COHERENT
	      ftruncate(fd, statb.st_size);
#else
	      chsize(fd, statb.st_size);
#endif /* COHERENT */
	}
	close(fd);
    }
#endif /* HAVE_UPDWTMP */
    return(0); /* no current failure cases; file not found is not failure! */
}
#endif /* WANT_UTMP */

/* This is for ancient Unixes that don't have these tty symbols defined. */

#ifndef PENDIN
#define PENDIN ICANON
#endif /* PENDIN */
#ifndef FLUSHO
#define FLUSHO ICANON
#endif /* FLUSHO */
#ifndef IMAXBEL
#define IMAXBEL ICANON
#endif /* IMAXBEL */
#ifndef EXTPROC
#define EXTPROC ICANON
#endif /* EXTPROC */

static char Xline[17] = { 0, 0 };
/*
  getptyslave()
  Open the slave side of the pty, and do any initialization that is necessary.
  The return value fd is a file descriptor for the slave side.
  fc = function code from do_pty() (q.v.)
*/
int
getptyslave(fd, fc) int * fd, fc; {
    int ttyfd;
    int t = -1;
    long retval;
#ifdef TIOCGWINSZ
    struct winsize ws;
    extern int cmd_rows, cmd_cols;
#endif /* TIOCGWINSZ */

    ttyfd = *fd;
    debug(F111,"getptyslave()","ttyfd",ttyfd);
    /*
     * Opening the slave side may cause initilization of the
     * kernel tty structure.  We need remember the state of:
     *      if linemode was turned on
     *      terminal window size
     *      terminal speed
     * so that we can reset them if we need to.
     */
    if ((retval = pty_open_slave(Xline, &t, fc)) != 0) {
	perror(Xline);
	msg++;
        debug(F111,"getptyslave()","Unable to open slave",retval);
        return(-1);
    }
    debug(F111,"getptyslave","t",t);
#ifdef INIT_SPTY
    spty = t;
    debug(F111,"getptyslave","spty",spty);
#endif /* INIT_SPTY */
#ifdef STREAMSPTY
    if (ioctl(t,I_PUSH,"pckt") < 0) {
        debug(F111,"getptyslave()","ioctl(I_PUSH) failed",errno);
#ifndef _AIX
        fatal("I_PUSH pckt");
#endif /* _AIX */
    }
#endif /* STREAMSPTY */

    /* Set up the tty modes as we like them to be. */
#ifdef COMMENT
    /* Originally like this... But this is the master - we want the slave */
    /* Anyway, this fails on Solaris and probably other System V OS's */
    init_termbuf(ttyfd);
#else
    init_termbuf(t);
#endif	/* COMMENT */
#ifdef TIOCGWINSZ
    if (cmd_rows || cmd_cols) {
        memset((char *)&ws, 0, sizeof(ws));
        ws.ws_col = cmd_cols;
        ws.ws_row = cmd_rows;
	debug(F101,"getptyslave() doing TIOCSWINSZ...","",t);
        ioctl(t, TIOCSWINSZ, (char *)&ws);
    }
#endif /* TIOCGWINSZ */

    /* For external protocols, put the pty in no-echo mode */
    if (fc == 1) {
	debug(F100,"getptyslave() setting rawmode","",0);
	/* iflags */
	termbuf.c_iflag &= ~(PARMRK|ISTRIP|BRKINT|INLCR|IGNCR|ICRNL);
	termbuf.c_iflag &= ~(INPCK|IGNPAR|IMAXBEL|IXANY|IXON|IXOFF);
	termbuf.c_iflag |= IGNBRK;
#ifdef IUCLC
	termbuf.c_iflag &= ~IUCLC;
#endif /* IUCLC */

	/* oflags */
	termbuf.c_oflag &= ~OPOST;
#ifdef OXTABS
	termbuf.c_oflag &= ~OXTABS;
#endif /* OXTABS */
#ifdef ONOCR
	termbuf.c_oflag &= ~ONOCR;
#endif /* ONOCR */
#ifdef ONLRET
	termbuf.c_oflag &= ~ONLRET;
#endif /* ONLRET */
#ifdef ONLCR
	termbuf.c_oflag &= ~ONLCR;
#endif /* ONLCR */

	/* lflags */
	termbuf.c_lflag &= ~ECHO;
#ifdef ECHOE
	termbuf.c_lflag &= ~ECHOE;
#endif /* ECHOE */
#ifdef ECHONL
	termbuf.c_lflag &= ~ECHONL;
#endif /* ECHONL */
#ifdef ECHOPRT
	termbuf.c_lflag &= ~ECHOPRT;
#endif /* ECHOPRT */
#ifdef ECHOKE
	termbuf.c_lflag &= ~ECHOKE;
#endif /* ECHOKE */
#ifdef ECHOCTL
	termbuf.c_lflag &= ~ECHOCTL;
#endif /* ECHOCTL */
#ifdef ALTWERASE
	termbuf.c_lflag &= ~ALTWERASE;
#endif /* ALTWERASE */
#ifdef EXTPROC
	termbuf.c_lflag &= ~EXTPROC;
#endif /* EXTPROC */
	termbuf.c_lflag &= ~(ICANON|ISIG|IEXTEN|TOSTOP|FLUSHO|PENDIN);

#ifdef NOKERNINFO
	termbuf.c_lflag |= NOKERNINFO;
#endif	/* NOKERNINFO */
	/* termbuf.c_lflag |= NOFLSH; */
	termbuf.c_lflag &= ~NOFLSH;

	/* cflags */
	termbuf.c_cflag &= ~(CSIZE|PARENB|PARODD);
	termbuf.c_cflag |= CS8|CREAD;
#ifdef VMIN
	termbuf.c_cc[VMIN] = 1;
#endif	/* VMIN */
    } else {				/* Regular interactive use */
	debug(F100,"getptyslave() setting cooked mode","",0);

	/* Settings for sgtty based systems */

#ifndef USE_TERMIO
	termbuf.sg.sg_flags |= CRMOD|ANYP|ECHO|XTABS;
#endif /* USE_TERMIO */

#ifndef OXTABS
#define OXTABS 0
#endif /* OXTABS */

	/* Settings for UNICOS and HPUX */

#ifdef CRAY
	termbuf.c_oflag = OPOST|ONLCR|TAB3;
	termbuf.c_iflag = IGNPAR|ISTRIP|ICRNL|IXON;
	termbuf.c_lflag = ISIG|ICANON|ECHO|ECHOE|ECHOK;
	termbuf.c_cflag = EXTB|HUPCL|CS8;
#else /* CRAY */
#ifdef HPUX
	termbuf.c_oflag = OPOST|ONLCR|TAB3;
	termbuf.c_iflag = IGNPAR|ISTRIP|ICRNL|IXON;
	termbuf.c_lflag = ISIG|ICANON|ECHO|ECHOE|ECHOK;
	termbuf.c_cflag = EXTB|HUPCL|CS8;
#else /* HPUX */
#ifdef USE_TERMIO
	/*
	  Settings for all other termios/termio based systems, other than 
	  4.4BSD.  In 4.4BSD the kernel does the initial terminal setup.
	*/
#ifdef BSD42
#ifndef BSD44
	termbuf.c_lflag |= ECHO|ICANON|IEXTEN|ISIG;
	termbuf.c_oflag |= ONLCR|OXTABS|OPOST;
	termbuf.c_iflag |= ICRNL|IGNPAR;
	termbuf.c_cflag |= HUPCL;
	termbuf.c_iflag &= ~IXOFF;
#endif /* BSD44 */
#else /* BSD42 */
	termbuf.c_lflag |= ECHO|ICANON|IEXTEN|ISIG;
	termbuf.c_oflag |= ONLCR|OXTABS|OPOST;
	termbuf.c_iflag |= ICRNL|IGNPAR;
	termbuf.c_cflag |= HUPCL;
	termbuf.c_iflag &= ~IXOFF;
#endif /* BSD42 */
#endif /* USE_TERMIO */
#endif /* HPUX */
#endif /* CRAY */
    }

    /* Set the tty modes, and make this our controlling tty. */
#ifdef COMMENT
    /* But this is the master - we want the slave */
    set_termbuf(ttyfd);
#else
    set_termbuf(t);
#endif	/* COMMENT */

    if (t != 0)
      dup2(t, 0);
    if (t != 1)
      dup2(t, 1);
    if (t != 2) {
	if (fc == 0) {
	    dup2(t, 2);
	} else if (fc == 1) {
	    /* For external protocols, send stderr to /dev/null */
#ifdef COMMENT
	    int xx;
#ifndef COMMENT
	    char * s = "/dev/null";
	    errno = 0;
	    xx = open(s, O_WRONLY);
#else
	    char * s = "pty.log";
	    errno = 0;
	    xx = open(s, O_CREAT, 0644);
#endif /* COMMENT */
	    debug(F111,"getptyslave redirect stderr",s,errno);
	    dup2(xx,2);
#endif	/* COMMENT */
	}
    }
    if (t > 2)
      close(t);

    if (ttyfd > 2) {
	close(ttyfd);
        ttyfd = -1;
	*fd = ttyfd;
    }
    return(0);
}

#ifdef HAVE_PTYTRAP
/*
  To be called to determine if a trap is pending on a pty
  if and only if select() cannot be used.
*/
int
pty_trap_pending(fd) int fd; {
    int pending;
    int rc;

    rc = ioctl(fd, TIOCTRAPSTATUS, (char *)&pending, sizeof(pending));
    if (rc == 0) {
        debug(F101,"pty_trap_pending()","",pending);
        return(pending);
    } else {
        debug(F111,"pty_trap_pending()","ioctl() failed",rc);
        return(-1);
    }
}

/*
  To be called after select() has returned indicating that an exception is
  waiting on a pty.  It should be called with the file descriptor of the pty.
  Returns -1 on error; 0 if pty is still open; 1 if pty has closed.
*/
int
pty_trap_handler(fd) int fd; {
    struct request_info ri;

    memset(&ri,0,sizeof(ri));
    if (ioctl(fd,TIOCREQCHECK,(char *)&ri, sizeof(ri)) != 0) {
        debug(F111,"pty_trap_handler()","ioctl(TIOCREQCHECK) failed",errno);
        return(-1);
    }
    switch (ri.request) {
      case TIOCOPEN:
        debug(F110,"pty_trap_handler()","an open() call",0);
        break;
      case TIOCCLOSE:
        debug(F110,"pty_trap_handler()","a close() call",0);
        break;
      default:
        debug(F110,"pty_trap_handler()","an ioctl() call",0);
        ri.errno_error = EINVAL;
    }
    if (ioctl(fd, TIOCREQSET, (char *)&ri,sizeof(ri)) != 0) {
        debug(F111,"pty_trap_handler()","ioctl(TIOCREQSET) failed",errno);
        return(-1);
    }
    if (ri.request == TIOCCLOSE)
      return(1);
    else
      return(0);
}
#endif /* HAVE_PTYTRAP */

VOID
exec_cmd(s) char * s; {
    struct stringarray * q;
    char ** args = NULL;

    if (!s) return;
    if (!*s) return;

    q = cksplit(1,0,s,NULL,"\\%[]&$+-/=*^_@!{}/<>|.#~'`:;?",7,0,0);
    if (!q) return;

    args = q->a_head + 1;

#ifdef DEBUG    
    {
	int i, n;
	n = q->a_size;
	for (i = 0; i <= n; i++) {
	    if (!args[i]) {
		debug(F111,"exec_cmd arg","NULL",i);
		break;
	    } else {
		debug(F111,"exec_cmd arg",args[i],i);
		if (i == n && args[i]) {
		    debug(F101,"exec_cmd SUBSTITUTING NULL","",i);
		    if (strlen(args[i]) == 0)
		      makestr(&(args[i]),NULL);
		}

	    }
	}	    
    }
#endif	/* DEBUG */

    execvp(args[0],args);
}

/* Get a pty, scan input lines. */
/* fc = 0 for interactive access; fc = 1 for running external protocols */

static int pty_fc = -1;			/* Global copy of fc */

int
do_pty(fd, cmd, fc) int * fd; char * cmd; int fc; {
    long retval;
    int syncpipe[2];
    int i, ttyfd;
#ifdef HAVE_PTYTRAP
    int x;
#endif /* HAVE_PTYTRAP */

    debug(F101,"CKUPTY.C do_pty fc","",fc);

    ttyfd = *fd;

    pty_master_fd = -2;
    pty_slave_fd = -2;
    pty_fork_pid = -2;

    msg = 0;				/* Message counter */
    pty_init();				/* Find an available pty to use. */
    errno = 0;

    if ((retval = pty_getpty(&ttyfd, Xline, 20)) != 0) {
	if (msg++ == 0)
	  perror(Xline);
        debug(F111,"do_pty()","pty_getpty() fails",retval);
	*fd = ttyfd;
        return(-1);
    }
    *fd = ttyfd;
    debug(F111,"do_pty() Xline",Xline,ttyfd);

#ifdef SIGTTOU
/*
  Ignoring SIGTTOU keeps the kernel from blocking us.  we tweak the tty with
  an ioctl() (in ttioct() in /sys/tty.c in a BSD kernel)
*/
     signal(SIGTTOU, SIG_IGN);
#endif /* SIGTTOU */

/* Start up the command on the slave side of the terminal */

    if (pipe(syncpipe) < 0) {
        debug(F110,"do_pty()","pipe() fails",0);
	perror("pipe() failed");
	msg++;
        debug(F111,"do_pty()","pipe fails",errno);
        return(-1);
    }
    if ((i = fork()) < 0) {
        /* XXX - need to clean up the allocated pty */
        perror("fork() failed");
	msg++;
        debug(F111,"do_pty()","fork fails",errno);
        return(-1);
    }
    if (i) {  /* Wait for child before writing to parent side of pty. */
        char c;
#ifdef HAVE_PTYTRAP
        int on = 1;
#endif /* HAVE_PTYTRAP */
	close(syncpipe[1]);
	errno = 0;
        if (read(syncpipe[0], &c, 1) == 0) { /* Slave side died */
	    perror("Pipe read() failed");
	    msg++;
            debug(F110,"do_pty()","Slave fails to initialize",0);
            close(syncpipe[0]);
            return(-1);
        }
        pty_fork_pid = i;		/* So we can clean it up later */
	pty_fork_active = 1;
	debug(F101,"do_pty pty_fork_pid","",pty_fork_pid);
#ifdef HAVE_PTYTRAP
        /* HPUX does not allow the master to read end of file.  */
        /* Therefore, we must determine that the slave has been */
        /* closed by trapping the call to close().              */
	errno = 0;
	x = ioctl(ttyfd, TIOCTRAP, (char *)&on);
	debug(F111,"do_pty ioctl(TIOCTRAP)",ckitoa(x),errno);
#endif /* HAVE_PTYTRAP */
        debug(F111,"do_pty()","synchronized - pty_fork_pid",pty_fork_pid);
        close(syncpipe[0]);
    } else {
	int x;
	debug(F101,"do_pty getptyslave ttyfd A","",ttyfd);
        debug(F110,"do_pty()","Slave starts",0);
	x = getptyslave(&ttyfd,fc);
	debug(F101,"do_pty getptyslave","",x);
        if (x == 0) {
	    debug(F101,"do_pty getptyslave ttyfd B","",ttyfd);
#ifdef WANT_UTMP
            pty_update_utmp(PTY_USER_PROCESS,
			    getpid(),
			    "KERMIT",
			    Xline,
			    cmd,
			    PTY_TTYSLOT_USABLE
			    );
#endif /* WANT_UTMP */
            /* Notify our parent we're ready to continue.*/
            debug(F110,"do_pty()","slave synchronizing",0);
            write(syncpipe[1],"y",1);
            close(syncpipe[0]);
            close(syncpipe[1]);

	    debug(F110,"do_pty cmd",cmd,"");
            exec_cmd(cmd);
            debug(F111,"do_pty()","exec_cmd() returns - why?",errno);
        }
	*fd = ttyfd;
        debug(F110,"do_pty()","getptyslave() fails - exiting",0);
        exit(1);
    }
    *fd = ttyfd;
    pty_fc = fc;
    return(getpid());
} /* end of do_pty() */


VOID
end_pty() {
    msg = 0;				/* Message counter */
    debug(F101,"end_pty pty_fork_pid","",pty_fork_pid);
    if (Xline[0] && pty_fork_pid >= 0) {
        pty_cleanup(Xline,pty_fork_pid,1);
        Xline[0] = '\0';
        pty_fork_pid = -1;
	pty_fork_active = 0;
	debug(F101,"end_pty pty_fork_active","",pty_fork_active);
    }
    pty_fc = -1;
}
#endif /* NETPTY */

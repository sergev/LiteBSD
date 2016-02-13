#define CKUTIO_C

#ifdef aegis
char *ckxv = "Aegis Communications support, 9.0.326, 20 August 2011";
#else
#ifdef Plan9
char *ckxv = "Plan 9 Communications support, 9.0.326, 20 August 2011";
#else
char *ckxv = "UNIX Communications support, 9.0.326, 20 August 2011";
#endif /* Plan9 */
#endif /* aegis */

/*  C K U T I O  */

/* C-Kermit interrupt, communications control and I/O functions for UNIX */

/*
  Author: Frank da Cruz (fdc@columbia.edu),
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 2011,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/

/*
  NOTE TO CONTRIBUTORS: This file, and all the other C-Kermit files, must be
  compatible with C preprocessors that support only #ifdef, #else, #endif,
  #define, and #undef.  Please do not use #if, logical operators, or other
  preprocessor features in any of the portable C-Kermit modules.  You can,
  of course, use these constructions in platform-specific modules when they
  are supported by all compilers/preprocessors that could be used on that
  platform.
*/

extern int nettype;			/* Defined in ckcmai.c */
extern int duplex;

/* Includes */

#include "ckcsym.h"			/* This must go first   */
#include "ckcdeb.h"			/* This must go second  */

#ifdef OSF13
#ifdef CK_ANSIC
#ifdef _NO_PROTO
#undef _NO_PROTO
#endif /* _NO_PROTO */
#endif /* CK_ANSIC */
#endif /* OSF13 */

#ifndef HPUXPRE65
#include <errno.h>			/* Error number symbols */
#else
#ifndef ERRNO_INCLUDED
#include <errno.h>			/* Error number symbols */
#endif	/* ERRNO_INCLUDED */
#endif	/* HPUXPRE65 */

#ifdef __386BSD__
#define ENOTCONN 57
#else
#ifdef __bsdi__
#define ENOTCONN 57
#else
#ifdef __FreeBSD__
#define ENOTCONN 57
#endif /* __FreeBSD__ */
#endif /* __bsdi__ */
#endif /* __386BSD__ */

#ifdef SCO_OSR504
#define NBBY 8
#endif /* SCO_OSR504 */

#ifdef Plan9
#define SELECT
#include <sys/time.h>
#include <select.h>
#define FD_SETSIZE (3 * sizeof(long) * 8)
static struct timeval tv;
#endif /* Plan9 */

#ifdef CLIX
#include <sys/time.h>
#endif /* CLIX */

#include "ckcnet.h"			/* Symbols for network types. */
#ifdef CK_SSL
#include "ck_ssl.h"
#endif /* CK_SSL */

/*
  The directory-related includes are here because we need to test some
  file-system-related symbols to find out which system we're being compiled
  under.  For example, MAXNAMLEN is defined in BSD4.2 but not 4.1.
*/
#ifdef SDIRENT				/* Directory bits... */
#define DIRENT
#endif /* SDIRENT */

#ifdef XNDIR
#include <sys/ndir.h>
#else /* !XNDIR */
#ifdef NDIR
#include <ndir.h>
#else /* !NDIR, !XNDIR */
#ifdef RTU
#include "/usr/lib/ndir.h"
#else /* !RTU, !NDIR, !XNDIR */
#ifdef DIRENT
#ifdef SDIRENT
#include <sys/dirent.h>
#else
#include <dirent.h>
#endif /* SDIRENT */
#else /* !RTU, !NDIR, !XNDIR, !DIRENT, i.e. all others */
#include <sys/dir.h>
#endif /* DIRENT */
#endif /* RTU */
#endif /* NDIR */
#endif /* XNDIR */

#ifdef QNX
#include <sys/dev.h>
#endif /* QNX */

#ifdef HPUX5
#ifndef TCPSOCKET
/* I don't know why this is needed here since we never reference bzero(). */
/* But without it C-Kermit won't link in an HP-UX 5.xx non-TCP build. */
void
bzero(s,n) char *s; int n; {
    extern char * memset();
    memset(s,0,n);
}
#endif /* TCPSOCKET */
#endif /* HPUX5 */

/* Definition of HZ, used in msleep() */

#ifdef MIPS
#define HZ ( 1000 / CLOCK_TICK )
#else  /* MIPS */
#ifdef ATTSV
#ifndef NAP
#ifdef TRS16
#define HZ ( 1000 / CLOCK_TICK )
#endif /* TRS16 */
#ifdef NAPHACK
#define nap(x) (void)syscall(3112, (x))
#define NAP
#endif /* NAPHACK */
#endif /* NAP */
#endif /* ATTSV */
#endif /* MIPS */

#ifdef M_UNIX
#undef NGROUPS_MAX		/* Prevent multiple definition warnings */
#endif /* M_UNIX */

/*
  NOTE: HP-UX 8.0 has a <sys/poll.h>, but there is no corresponding
  library routine, so _poll comes up undefined at link time.
*/
#ifdef CK_POLL
#ifndef AIXRS			/* IBM AIX needs special handling */
#include <poll.h>		/* "standard" (SVID) i/o multiplexing, etc */
#else /* AIXRS */
#ifdef SVR4			/* AIX 3.2 is like SVID... */
#include <poll.h>
#else				/* But AIX 3.1 is not ... */
#include <sys/poll.h>		/* The include file is in include/sys */
#define events reqevents	/* And it does not map IBM-specific member */
#define revents rtnevents	/* names to the System V equivalents */
#endif /* SVR4 */
#endif /* AIXRS */
#endif /* CK_POLL */

#include <signal.h>                     /* Signals */

/* For setjmp and longjmp */

#ifndef ZILOG
#include <setjmp.h>
#else
#include <setret.h>
#endif /* ZILOG */

/*
  The following test differentiates between 4.1 BSD and 4.2 & later.
  If you have a 4.1BSD system with the DIRENT library, this test could
  mistakenly diagnose 4.2BSD and then later enable the use of system calls
  that aren't defined.  If indeed there are such systems, we can use some
  other way of testing for 4.1BSD, or add yet another compile-time switch.
*/
#ifdef BSD4
#ifdef MAXNAMLEN
#ifndef FT21				/* Except for Fortune. */
#ifndef FT18
#ifndef BELLV10				/* And Bell Labs Research UNIX V10 */
#define BSD42
#endif /* BELLV10 */
#endif /* FT18 */
#endif /* FT21 */
#endif /* MAXNAMLEN */
#endif /* BSD4 */

#ifdef SUNOS41				/* From Christian Corti */
#define BSD44ORPOSIX			/* Uni Stuttgart */
#define SVORPOSIX			/* February 2010 */
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <limits.h>
#endif	/* SUNOS41 */

#ifdef SNI542
#include <sys/filio.h>			/* 299 for FIONREAD */
#endif	/* SNI542 */

/*
  Minix 2.0 support added by Terry McConnell,
  Syracuse University <tmc@barnyard.syr.edu>
  No more sgtty interface, posix compliant.
*/
#ifdef MINIX2
#define _MINIX   /* Needed for some Minix header files */
#define BSD44ORPOSIX
#define SVORPOSIX
#ifndef MINIX3
#define DCLTIMEVAL
#endif	/* MINIX3 */
#define NOFILEH
#include <sys/types.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <limits.h>
#undef TIOCGETC    /* defined in sys/ioctl.h, but not really supported */
#define TANDEM 0
#endif /* MINIX2 */

/*
 MINIX 1.0 support added by Charles Hedrick,
 Rutgers University <hedrick@aramis.rutgers.edu>.
 MINIX also has V7 enabled.
*/
#ifdef MINIX
#define TANDEM 0
#define MYREAD
#define NOSYSIOCTLH
#include <limits.h>
#endif /* MINIX */

#ifdef CK_REDIR		/* <sys/wait.h> needed only for REDIRECT command. */
/*
  If anybody can figure out how to make this work with NeXTSTEP, be
  my guest!  (NeXTBlah/NeXTBlah/bsd/sys/wait.h does not define WEXITSTATUS)
*/
#ifndef CK_WAIT_H			/* If wait.h not already included... */
#ifdef OSF				/* force OSF to select POSIX wait */
#ifdef _BSD				/* instead of BSD (see ckcdeb.h) */
#define CK_OSF_BSD
#undef _BSD
#endif /* _BSD */
#endif /* OSF */
#include <sys/wait.h>			/* Include it */
#ifdef OSF
#ifdef CK_OSF_BSD
#define _BSD				/* Restore it */
#undef CK_OSF_BSD
#endif /* CK_OSF_BSD */
#endif /* OSF */
#endif /* CK_WAIT_H */
#endif /* CK_REDIR */

#include "ckuver.h"			/* Version herald */
char *ckxsys = HERALD;

#ifdef CK_UTSNAME
#include <sys/utsname.h>

#ifdef TRU64				/* Tru64 UNIX 4.0 and later */
/* Verified on Tru64 4.0F - might break on 4.0E or earlier */
#include <sys/sysinfo.h>		/* (don't know about OSF/1 or DU) */
#include <machine/hal_sysinfo.h>
#endif /* TRU64 */

#ifdef SOLARIS25			/* Solaris 2.5 and later */
#include <sys/systeminfo.h>		/* (don't know about earlier ones) */
#endif /* SOLARIS25 */

#ifdef UW7
#ifndef SYS_NMLN
#define SYS_NMLN 257
#endif /* NMLN */
#endif /* UW7 */
#ifdef HPUX9PLUS
static int hpis800 = 0;
#endif /* HPUX9PLUS */
#ifdef SYS_NMLN
#define CK_SYSNMLN SYS_NMLN
#else
#ifdef _SYS_NMLN
#define CK_SYSNMLN _SYS_NMLN
#else
#ifdef UTSLEN
#define CK_SYSNMLN UTSLEN
#else
#define CK_SYSNMLN 31
#endif /* UTSLEN */
#endif /* _SYS_NMLN */
#endif /* SYS_NMLN */
char unm_mch[CK_SYSNMLN+1] = { '\0', '\0' };
char unm_mod[CK_SYSNMLN+1] = { '\0', '\0' };
char unm_nam[CK_SYSNMLN+1] = { '\0', '\0' };
char unm_rel[CK_SYSNMLN+1] = { '\0', '\0' };
char unm_ver[CK_SYSNMLN+1] = { '\0', '\0' };
#endif /* CK_UTSNAME */

#ifdef CIE
#include <stat.h>			/* For chasing symlinks, etc. */
#else
#include <sys/stat.h>
#endif /* CIE */

#ifdef QNX				/* 299 */
#ifndef IXANY
#define IXANY 0
#endif	/* IXANY */
#endif	/* QNX */

/* UUCP lockfile material... */

#ifndef NOUUCP
#ifdef USETTYLOCK
#ifdef HAVE_LOCKDEV			/* Red Hat baudboy/lockdev */
/*
  Watch out: baudboy.h references open() without making sure it has been
  declared, resulting in warnings on at least Red Hat 7.3.  It's declared in
  fcntl.h, but we don't include that until later.  In this case only, we
  include it here, and then the second include is harmless because in Red Hat
  Linux (the only place where you find baudboy.h) fcntl.h is protected from
  multiple inclusion by _FCNTL_H.   - fdc, 10 May 2004.

  NOTE: Although Linux /usr/sbin/lockdev obviates the need for setuid or
  setgid bits to access the lockfile, C-Kermit will still need them to access
  the serial port itself unless the port is open for world read/write.
  Normally setgid uucp does the trick.

  Extra: HAVE_LOCKDEV has been added als openSuSE >= 11.3 doesn't use baudboy
  but ttylock.  - jb, 26 Jul 2010
*/
#include <fcntl.h>			/* This has to come before baudboy */
#ifdef HAVE_BAUDBOY			/* Red Hat baudboy/lockdev */
#include <baudboy.h>
#else  /* !HAVE_BAUDBOY */		/* openSuSE lock via ttylock */
#include <ttylock.h>
#endif  /* HAVE_BAUDBOY */
#define LOCK_DIR "/var/lock"		/* (even though we don't care) */

#else  /* !HAVE_LOCKDEV */

#ifdef USE_UU_LOCK
#ifdef __FreeBSD__
#include <libutil.h>			/* FreeBSD */
#else
#include <util.h>			/* OpenBSD */
#endif /* HAVE_LOCKDEV */
#endif /* __FreeBSD */
#endif /* USE_UU_LOCK */
#else  /* USETTYLOCK */

/* Name of UUCP tty device lockfile */

#ifdef LINUXFSSTND
#ifndef HDBUUCP
#define HDBUUCP
#endif /* HDBUUCP */
#endif /* LINUXFSSTND */

#ifdef ACUCNTRL
#define LCKDIR
#endif /* ACUCNTRL */

/*
  PIDSTRING means use ASCII string to represent pid in lockfile.
*/
#ifndef PIDSTRING
#ifdef HDBUUCP
#define PIDSTRING
#else
#ifdef BSD44
#define PIDSTRING
#else
#ifdef RTAIX
#define PIDSTRING
#else
#ifdef AIXRS
#define PIDSTRING
#else
#ifdef COHERENT
#define PIDSTRING
#endif /* COHERENT */
#endif /* AIXRS */
#endif /* RTAIX */
#endif /* BSD44 */
#endif /* HDBUUCP */
#endif /* PIDSTRING */

/* Now the PIDSTRING exceptions... */

#ifdef PIDSTRING
#ifdef HPUX
#undef PIDSTRING
#endif /* HPUX */
#endif /* PIDSTRING */

#ifdef __bsdi__				/* BSDI (at least thru 1.1) */
#ifdef PIDSTRING
#undef PIDSTRING
#endif /* PIDSTRING */
#endif /* __bsdi__ */

#ifdef OSF32				/* Digital UNIX (OSF/1) 3.2 */
#ifdef PIDSTRING
#undef PIDSTRING
#endif /* PIDSTRING */
#endif /* OSF32 */

/*
  LOCK_DIR is the name of the lockfile directory.
  If LOCK_DIR is already defined (e.g. on command line), we don't change it.
*/

#ifndef LOCK_DIR
#ifdef MACOSX
#define LOCK_DIR "/var/spool/lock"
#endif /* MACOSX */
#endif/* LOCK_DIR */

#ifndef LOCK_DIR
#ifdef BSD44
#ifdef __386BSD__
#define LOCK_DIR "/var/spool/lock"
#else
#ifdef __FreeBSD__
#define LOCK_DIR "/var/spool/lock"
#else
#ifdef __NetBSD__
#define LOCK_DIR "/var/spool/lock"
#else
#ifdef __OpenBSD__
#define LOCK_DIR "/var/spool/lock"
#else
/* So which ones is this for? */
/* Probably original 4.4BSD on Vangogh */
/* Plus who knows about Mac OS X... It doesn't even have a cu program */
#define LOCK_DIR "/var/spool/uucp"
#endif /* __OpenBSD__ */
#endif /* __NetBSD__ */
#endif /* __FreeBSD__ */
#endif /* __386BSD__ */
#else
#ifdef DGUX430
#define LOCK_DIR "/var/spool/locks"
#else
#ifdef HPUX10
#define LOCK_DIR "/var/spool/locks"
#else
#ifdef RTAIX				/* IBM RT PC AIX 2.2.1 */
#define LOCK_DIR "/etc/locks"
#else
#ifdef AIXRS
#define LOCK_DIR "/etc/locks"
#else
#ifdef ISIII
#define LOCK_DIR "/etc/locks"
#else
#ifdef HDBUUCP
#ifdef M_SYS5
#define LOCK_DIR "/usr/spool/uucp"
#else
#ifdef M_UNIX
#define LOCK_DIR "/usr/spool/uucp"
#else
#ifdef SVR4
#define LOCK_DIR "/var/spool/locks"
#else
#ifdef SUNOS4
#define LOCK_DIR "/var/spool/locks"
#else
#ifdef LINUXFSSTND
#define LOCK_DIR "/var/lock";
#else
#define LOCK_DIR "/usr/spool/locks"
#endif /* LINUXFSSTND */
#endif /* SUNOS4 */
#endif /* SVR4 */
#endif /* M_UNIX */
#endif /* M_SYS5 */
#else
#ifdef LCKDIR
#define LOCK_DIR "/usr/spool/uucp/LCK"
#else
#ifdef COHERENT
#define LOCK_DIR "/usr/spool/uucp"
#else
#define LOCK_DIR "/usr/spool/uucp"
#endif /* COHERENT */
#endif /* LCKDIR */
#endif /* HDBUUCP */
#endif /* ISIII */
#endif /* AIXRS */
#endif /* RTAIX */
#endif /* HPUX10 */
#endif /* DGUX430 */
#endif /* BSD44 */
#endif /* !LOCK_DIR (outside ifndef) */

#ifdef OSF2				/* OSF/1 2.0 or later */
#ifdef LOCK_DIR				/* (maybe 1.x too, who knows...) */
#undef LOCK_DIR
#define LOCK_DIR "/var/spool/locks"
#endif /* LOCK_DIR */
#endif /* OSF2 */

#ifdef COMMENT
/* Sorry no more lockf() -- we lock first and THEN open the device. */
#ifdef SVR4
#ifndef BSD44
#ifndef LOCKF
#define LOCKF				/* Use lockf() on tty device in SVR4 */
#endif /* LOCKF */
#endif /* BSD44 */
#endif /* SVR4 */
#endif /* COMMENT */

#ifdef NOLOCKF				/* But NOLOCKF cancels LOCKF */
#ifdef LOCKF
#undef LOCKF
#endif /* LOCKF */
#endif /* NOLOCKF */

/* More about this below... */

#endif /* USETTYLOCK */
#endif /* NOUUCP */

/*
  MYREAD means use our internally defined nonblocking buffered read routine.
*/
#ifdef ATTSV
#define MYREAD
#endif /* ATTSV */

#ifdef ATT7300
#ifndef MYREAD
#define MYREAD
#endif /* MYREAD */
/* bits for attmodem: internal modem in use, restart getty */
#define ISMODEM 1
#define DOGETY 512
#endif  /* ATT7300 */

#ifdef BSD42
#define MYREAD
#endif /* BSD42 */

#ifdef POSIX
#define MYREAD
#endif /* POSIX */
#ifdef __bsdi__
#ifndef O_NDELAY
#define O_NDELAY O_NONBLOCK
#endif /* O_NDELAY */
#endif /* __bsdi__ */

/*
 Variables available to outside world:

   dftty  -- Pointer to default tty name string, like "/dev/tty".
   dfloc  -- 0 if dftty is console, 1 if external line.
   dfprty -- Default parity
   dfflow -- Default flow control
   ckxech -- Flag for who echoes console typein:
     1 - The program (system echo is turned off)
     0 - The system (or front end, or terminal).
   functions that want to do their own echoing should check this flag
   before doing so.

   flfnam  -- Name of lock file, including its path, e.g.,
                "/usr/spool/uucp/LCK..cul0" or "/etc/locks/tty77"
   lkflfn  -- Name of link to lock file, including its paths
   haslock -- Flag set if this kermit established a uucp lock.
   lockpid -- PID of other process that has desired line open, as string.
   backgrd -- Flag indicating program executing in background ( & on
                end of shell command). Used to ignore INT and QUIT signals.
   rtu_bug -- Set by stptrap().  RTU treats ^Z as EOF (but only when we handle
                SIGTSTP)

 Functions for assigned communication line (either external or console tty):

   sysinit()               -- System dependent program initialization
   syscleanup()            -- System dependent program shutdown
   ttopen(ttname,local,mdmtyp,timo) -- Open the named tty for exclusive access.
   ttclos()                -- Close & reset the tty, releasing any access lock.
   ttsspd(cps)             -- Set the transmission speed of the tty.
   ttgspd()                -- Get (read) the the transmission speed of the tty.
   ttpkt(speed,flow,parity) -- Put the tty in packet mode and set the speed.
   ttvt(speed,flow)        -- Put the tty in virtual terminal mode.
                                or in DIALING or CONNECTED modem control state.
   ttres()                 -- Restore original tty modes.
   ttscarr(carrier)        -- Set carrier control mode, on/off/auto.
   ttinl(dest,max,timo)    -- Timed read line from the tty.
   ttinc(timo)             -- Timed read character from tty.
   myread()                -- Raw mode bulk buffer read, gives subsequent
                                chars one at a time and simulates FIONREAD.
   myunrd(c)               -- Places c back in buffer to be read (one only)
   ttchk()                 -- See how many characters in tty input buffer.
   ttxin(n,buf)            -- Read n characters from tty (untimed).
   ttol(string,length)     -- Write a string to the tty.
   ttoc(c)                 -- Write a character to the tty.
   ttflui()                -- Flush tty input buffer.
   ttsndb()                -- Send BREAK signal.
   ttsndlb()               -- Send Long BREAK signal.

   ttlock(ttname)          -- "Lock" tty device against uucp collisions.
   ttunlck()               -- Unlock tty device.

                              For ATT7300/Unix PC, System V:
   attdial(ttname,speed,telnbr) -- dials ATT7300/Unix PC internal modem
   offgetty(ttname)        -- Turns off getty(1m) for comms line
   ongetty(ttname)         -- Restores getty() to comms line
*/

/*
Functions for console terminal:

   congm()   -- Get console terminal modes.
   concb(esc) -- Put the console in single-character wakeup mode with no echo.
   conbin(esc) -- Put the console in binary (raw) mode.
   conres()  -- Restore the console to mode obtained by congm().
   conoc(c)  -- Unbuffered output, one character to console.
   conol(s)  -- Unbuffered output, null-terminated string to the console.
   conola(s) -- Unbuffered output, array of strings to the console.
   conxo(n,s) -- Unbuffered output, n characters to the console.
   conchk()  -- Check if characters available at console (bsd 4.2).
                Check if escape char (^\) typed at console (System III/V).
   coninc(timo)  -- Timed get a character from the console.
   congks(timo)  -- Timed get keyboard scan code.
   conint()  -- Enable terminal interrupts on the console if not background.
   connoi()  -- Disable terminal interrupts on the console if not background.

Time functions

   msleep(m) -- Millisecond sleep
   ztime(&s) -- Return pointer to date/time string
   rtimer() --  Reset timer
   gtimer()  -- Get elapsed time since last call to rtimer()
*/

/* Conditional Includes */

/* Whether to include <sys/file.h> */

#ifdef RTU				/* RTU doesn't */
#define NOFILEH
#endif /* RTU */

#ifdef CIE				/* CIE does. */
#undef NOFILEH
#endif /* CIE */

#ifdef BSD41				/* 4.1 BSD doesn't */
#define NOFILEH
#endif /* BSD41 */

#ifdef is68k				/* Integrated Solutions 68000 UNIX  */
#define NOFILEH				/* e.g. on Plexux P60 and Sun-1 */
#endif /* is68k */

#ifdef MINIX				/* MINIX */
#define NOFILEH
#endif /* MINIX */

#ifdef COHERENT				/* Coherent */
#define NOFILEH
#endif /* COHERENT */

#ifndef NOFILEH				/* Now include if selected. */
#include <sys/file.h>
#endif /* NOFILEH */

/* POSIX */

#ifdef BSD44ORPOSIX			/* POSIX uses termios.h */
#define TERMIOS
#ifdef __bsdi__
#ifdef POSIX
#undef _POSIX_SOURCE			/* Get extra stuff from termios.h */
#endif /* POSIX */
#endif /* __bsdi__ */
#include <termios.h>
#ifdef LINUX
#include <sys/ioctl.h>
#endif /* LINUX */
#ifdef QNX16
#include <ioctl.h>
#else
#ifdef QNX6
#include <ioctl.h>
#endif /* QNX6 */
#endif /* QNX16 */
#ifdef __bsdi__
#ifdef POSIX
#define _POSIX_SOURCE
#endif /* POSIX */
#endif /* __bsdi__ */
#ifndef BSD44				/* Really POSIX */
#ifndef CK_QNX32			/* was CK_QNX32 */
#define NOSYSIOCTLH			/* No ioctl's allowed. */
#undef ultrix				/* Turn off any ultrix features. */
#endif /* CK_QNX32 */
#endif /* BSD44 */
#endif /* POSIX */

/* System III, System V */

#ifdef ATTSV
#ifndef BSD44
#ifndef POSIX
#include <termio.h>
#endif /* POSIX */
#endif /* BSD44 */
#ifdef TERMIOX
/* Need this for termiox structure, RTS/CTS and DTR/CD flow control */
#include <termiox.h>
  struct termiox rctsx;
#else
#ifdef STERMIOX
#ifdef SCO_OSR504
/* Sorry, this is truly disgusting but it's SCO's fault. */
#ifndef _SVID3
#define _CK_SVID3_X
#define _SVID3
#endif /* _SVID3 */
#endif /* SCO_OSR504 */
#include <sys/termiox.h>
  struct termiox rctsx;
#ifdef CK_SVID3_X
#undef _SVID3
#undef CK_SVID3_X
#endif /* CK_SVID3_X */
#endif /* STERMIOX */
#endif /* TERMIOX */
#endif /* ATTSV */

#ifdef COHERENT			/* Use termio.h, not sgtty.h for Coherent */
#include <termio.h>
#endif /* COHERENT */

#ifdef MINIX				/* MINIX uses ioctl's */
#define NOSYSIOCTLH			/* but has no <sys/ioctl.h> */
#endif /* MINIX */

/* Others */

#ifndef NOSYSIOCTLH			/* Others use ioctl() */
#ifdef SUN4S5
/*
  This is to get rid of cpp warning messages that occur because all of
  these symbols are defined by both termios.h and ioctl.h on the SUN.
*/
#undef ECHO
#undef NL0
#undef NL1
#undef TAB0
#undef TAB1
#undef TAB2
#undef XTABS
#undef CR0
#undef CR1
#undef CR2
#undef CR3
#undef FF0
#undef FF1
#undef BS0
#undef BS1
#undef TOSTOP
#undef FLUSHO
#undef PENDIN
#undef NOFLSH
#endif /* SUN4S5 */
#include <sys/ioctl.h>
#endif /* NOSYSIOCTLH */
/*
  We really, really, REALLY want FIONREAD, because it is the only way to find
  out not just *if* stuff is waiting to be read, but how much, which is
  critical to our sliding-window and streaming procedures, not to mention
  efficiency of CONNECT, etc.
*/
#ifdef BELLV10
#include <sys/filio.h>			/* For FIONREAD */
#ifdef FIONREAD
#define MYREAD
#endif /* MYREAD */
#endif /* BELLV10 */

#ifndef FIONREAD
/* It wasn't found in ioctl.h or term*.h - try these places: */
#ifdef UNIXWARE
#include <sys/filio.h>
#else
#ifdef SOLARIS
#include <sys/filio.h>
#endif /* SOLARIS */
#endif /* UNIXWARE */
#endif /* FIONREAD */

#ifdef XENIX /* Was M_UNIX but XENIX implies M_UNIX and applies to XENIX too */
/*
  <sys/socket.h> included above via "ckcnet.h" defines FIONREAD as
  something.  Due to this, in_chk() uses the FIONREAD instead of RDCHK
  and the hot keys during file transfer (X to cancel file etc) do not
  work because FIONREAD doesn't work even though it is defined.

  NOTE: This might also be true elsewhere.
*/
#ifdef FIONREAD
#undef FIONREAD
#endif /* FIONREAD */
#endif /* XENIX */

#ifdef CK_SCOV5				/* Ditto for SCO OpenServer 5.0 */
#ifndef SCO_OSR507			/* 299 */
#ifdef FIONREAD
#undef FIONREAD
#endif /* FIONREAD */
#endif	/* SCO_OSR507 */
#endif /* CK_SCOV5 */

#ifdef SCO_OSR507			/* 299 */
#ifdef RDCHK
#undef RDCHK
#endif	/* RDCHK */
#endif	/* SCO_OSR507 */

/* Whether to include <fcntl.h> */

#ifndef is68k				/* Only a few don't have this one. */
#ifndef BSD41
#ifndef FT21
#ifndef FT18
#ifndef COHERENT
#include <fcntl.h>
#endif /* COHERENT */
#endif /* FT18 */
#endif /* FT21 */
#endif /* BSD41 */
#endif /* not is68k */

#ifdef COHERENT
#ifdef _I386
#include <fcntl.h>
#else
#include <sys/fcntl.h>
#endif /* _I386 */
#endif /* COHERENT */

#ifdef ATT7300				/* Unix PC, internal modem dialer */
#include <sys/phone.h>
#endif /* ATT7300 */

#ifdef HPUX				/* HP-UX variations. */
#define HPUXJOBCTL
#include <sys/modem.h>			/* HP-UX modem signals */
#ifdef hp9000s500			/* Model 500 */
#undef HPUXJOBCTL
#endif /* hp9000s500 */
#ifdef HPUXPRE65
#undef HPUXJOBCTL
typedef long mflag;
#endif /* HPUXPRE65 */
#ifdef HPUXJOBCTL
#include <sys/bsdtty.h>			/* HP-UX Berkeley tty support */
#endif /* HPUXJOBCTL */
#endif /* HPUX */

/*
  Which time.h files to include... See ckcdeb.h for defaults.
  Note that 0, 1, 2, or all 3 of these can be included according to
  the symbol definitions.
*/
#ifndef NOTIMEH
#ifdef TIMEH
#include <time.h>
#endif /* TIMEH */
#endif /* NOTIMEH */

#ifndef NOSYSTIMEH
#ifdef SYSTIMEH
#include <sys/time.h>
#endif /* SYSTIMEH */
#endif /* NOSYSTIMEH */

#ifndef NOSYSTIMEBH
#ifdef SYSTIMEBH
#include <sys/timeb.h>
#endif /* SYSTIMEBH */
#endif /* NOSYSTIMEBH */

#ifndef NODCLTIMEVAL
#ifdef DCLTIMEVAL
/*
  In certain POSIX builds (like Unixware 7), <[sys/]time.h> refuses to
  define the structs we need to access the higher speeds, so we have to
  do it ourselves.
*/
struct timeval {
    long tv_sec;
    long tv_usec;
};
struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};
#endif /* DCLTIMEVAL */
#endif /* NODCLTIMEVAL */

#ifdef __linux__
/* THIS IS OBSOLETE since about Linux 0.92 */
#ifdef OLINUXHISPEED
#include <linux/serial.h>
#endif /* OLINUXHISPEED */
#ifdef __alpha__			/* Linux on DEC Alpha */
#ifndef __GLIBC__			/* But not with glibc */
#include <asm/termios.h>
#endif /* __GLIBC__ */
#endif /* __alpha__ */
#endif /* __linux__ */

#ifdef NOIEXTEN				/* This is broken on some systems */
#undef IEXTEN				/* like Convex/OS 9.1 */
#endif /* NOIEXTEN */
#ifndef IEXTEN				/* Turn off ^O/^V processing. */
#define IEXTEN 0			/* Needed, at least, on BSDI. */
#endif /* IEXTEN */
/*
  Pick up definitions needed for select() if we don't have them already.
  Normally they come from <sys/types.h> but some systems get them from
  <sys/select.h>...  Rather than hardwire all of them into the source, we
  include it if SELECT_H is defined in compile-time CFLAGS.
*/
#ifndef SCO_OSR504
#ifdef SELECT_H
#include <sys/select.h>
#endif /* SELECT_H */
#endif /* SCO_OSR504 */

#ifdef aegis
#include "/sys/ins/base.ins.c"
#include "/sys/ins/error.ins.c"
#include "/sys/ins/ios.ins.c"
#include "/sys/ins/sio.ins.c"
#include "/sys/ins/pad.ins.c"
#include "/sys/ins/time.ins.c"
#include "/sys/ins/pfm.ins.c"
#include "/sys/ins/pgm.ins.c"
#include "/sys/ins/ec2.ins.c"
#include "/sys/ins/type_uids.ins.c"
#include <default_acl.h>
#undef TIOCEXCL
#undef FIONREAD
#endif /* aegis */

#ifdef sxaE50				/* PFU Compact A SX/A TISP V10/L50 */
#undef FIONREAD
#endif /* sxaE50 */

/* The following #defines are catch-alls for those systems */
/* that didn't have or couldn't find <file.h>... */

#ifndef FREAD
#define FREAD 0x01
#endif /* FREAD */

#ifndef FWRITE
#define FWRITE 0x10
#endif /* FWRITE */

#ifndef O_RDONLY
#define O_RDONLY 000
#endif /* O_RDONLY */

/* This is for ancient Unixes that don't have these tty symbols defined. */

#ifndef PENDIN
#define PENDIN ICANON
#endif /* PENDIN */
#ifndef FLUSHO
#define FLUSHO ICANON
#endif /* FLUSHO */
#ifndef EXTPROC
#define EXTPROC ICANON
#endif /* EXTPROC */

#ifdef SVORPOSIX
/*
  Modem signals are also forbidden in the POSIX world.  But some POSIX-based
  platforms let us at them anyway if we know where to look.
*/
#ifndef NEEDMDMDEFS
/* Doesn't work for Linux */
#ifdef UNIXWARE7
#define NEEDMDMDEFS
#endif /* UNIXWARE7 */
#endif /* NEEDMDMDEFS */

#ifdef NEEDMDMDEFS
#ifndef TIOCMGET
#define TIOCMGET (('t'<<8)|29)
#endif /* TIOCMGET */

#ifndef TIOCM_DTR
#define TIOCM_DTR 0x0002
#endif /* TIOCM_DTR */
#ifndef TIOCM_RTS
#define TIOCM_RTS 0x0004
#endif /* TIOCM_RTS */
#ifndef TIOCM_CTS
#define TIOCM_CTS 0x0020
#endif /* TIOCM_CTS */
#ifndef TIOCM_CAR
#define TIOCM_CAR 0x0040
#endif /* TIOCM_CAR */
#ifndef TIOCM_RNG
#define TIOCM_RNG 0x0080
#endif /* TIOCM_RNG */
#ifndef TIOCM_DSR
#define TIOCM_DSR 0x0100
#endif /* TIOCM_DSR */
#endif /* NEEDMDMDEFS */
#endif /* SVORPOSIX */

/* Declarations */

#ifdef OXOS
#undef TCGETA
#undef TCSETA
#undef TCSETAW
#undef TCSETAF
#define TCGETA TCGETS
#define TCSETA TCSETS
#define TCSETAW TCSETSW
#define TCSETAF TCSETSF
#define termio termios
#endif /* OXOS */

#ifdef SVORPOSIX			/* AT&T Sys V or POSIX */
#ifdef UNIXWAREPOSIX			/* UnixWare 7 POSIX build */
/*
  In Unixware POSIX builds, <[sys/]time.h> refuses to define the
  structs we need to access the higher speeds, so we have to do it
  ourselves.
*/
struct timeval {
    long tv_sec;
    long tv_usec;
};
struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};
#endif /* UNIXWAREPOSIX */
#endif /* SVORPOSIX */

#ifdef __GNUC__
#ifdef XENIX
/*
  Because Xenix <time.h> doesn't declare time() if we're using gcc.
*/
time_t time();
#endif /* XENIX */
#endif /* __GNUC__ */

/* Special stuff for V7 input buffer peeking */

#ifdef  V7
int kmem[2] = { -1, -1};
char *initrawq(), *qaddr[2]={0,0};
#define CON 0
#define TTY 1
#endif /* V7 */

/* dftty is the device name of the default device for file transfer */
/* dfloc is 0 if dftty is the user's console terminal, 1 if an external line */

#ifdef BEOS
    char * dftty = NULL;
    char * dfmdm = "none";
    int dfloc = 0;                  /* that goes in local mode by default */
#else
#ifndef DFTTY
#ifdef PROVX1
    char *dftty = "/dev/com1.dout"; /* Only example so far of a system */
    char *dfmdm = "none";
    int dfloc = 1;                  /* that goes in local mode by default */
#else
    char *dftty = CTTNAM;               /* Remote by default, use normal */
    char *dfmdm = "none";
    int dfloc = 0;                      /* controlling terminal name. */
#endif /* PROVX1 */
#else
    char *dftty = DFTTY;		/* Default location specified on */
    char *dfmdm = "none";		/* command line. */
    int dfloc = 1;                      /* controlling terminal name. */
#endif /* DFTTY */
#endif /* BEOS */

#define CON_RES 0			/* Console state is "reset" */
#define CON_CB  1			/* Console state is CBREAK */
#define CON_BIN 2			/* Console state is binary */
    static int constate = CON_RES;

#define CONI_RES 0			/* Console interrupts are "reset" */
#define CONI_INT 1			/* Console intterupts are set */
#define CONI_NOI 2			/* Console intterupts are disabled */
    static int conistate = CONI_RES;

#ifdef CK_SMALL
#define CONBUFSIZ 15
#else
#define CONBUFSIZ 255
#endif /* CK_SMALL */
    static char conbuf[CONBUFSIZ];	/* Console readahead buffer */
    static int  conbufn = 0;		/* Chars in readahead buffer */
    static char *conbufp = conbuf;	/* Next char in readahead buffer */

    char cttnam[DEVNAMLEN+1] = { '\0', '\0' }; /* Determined at runtime */

#ifdef RTU
    int rtu_bug = 0;		    /* set to 1 when returning from SIGTSTP */
#endif /* RTU */

    int dfprty = DEFPAR;                /* Default parity (0 = none) */
    int ttprty = 0;                     /* The parity that is in use. */
    static int ttpmsk = 0xff;		/* Parity stripping mask. */
    int ttmdm = 0;                      /* Modem in use. */
    int ttcarr = CAR_AUT;		/* Carrier handling mode. */
    int dfflow = FLO_NONE;		/* Default flow control is NONE */
    int backgrd = 0;                    /* Assume in foreground (no '&' ) */
#ifdef F_SETFL
    int iniflags = -1;			/* fcntl flags for ttyfd */
#endif /* F_SETFL */
    int fdflag = 0;			/* Flag for redirected stdio */
    int ttfdflg = 0;			/* Open File descriptor was given */
    int tvtflg = 0;			/* Flag that ttvt has been called */
    long ttspeed = -1L;			/* For saving speed */
    int ttflow = -9;			/* For saving flow */
    int ttld = -1;			/* Line discipline */

#ifdef sony_news
    static int km_con = -1;		/* Kanji mode for console tty */
    static int km_ext = -1;		/* Kanji mode for external device */
#endif /* sony_news */

#ifdef PARSENSE
    static int needpchk = 1;		/* Need parity check */
#else
    static int needpchk = 0;
#endif /* PARSENSE */

    extern int stopbits;		/* Stop bits */
#ifdef HWPARITY
/*
  Unfortunately we must do this with global variables rather than through the
  tt...() APIs to avoid changing the APIs and the many modules that use them.
  If hwparity != 0, this indicates 8 data bits + parity, rather than 7 data
  bits + parity or 8 data bits and no parity, and overrides the regular parity
  variable, which is communicated to this module thru ttpkt(), and represented
  locally by the ttprty variable.
*/
    extern int hwparity;		/* Hardware parity */
#endif /* HWPARITY */

#ifdef TCPSOCKET
#ifdef TCP_NODELAY
static int nodelay_sav = -1;
#endif /* TCP_NODELAY */
#endif /* TCPSOCKET */

static int sigint_ign = 0;		/* SIGINT is ignored */

/*
  Having this module rely on external globals is bad, but fixing this
  requires overhaul of the ck*tio.c modules for all the different operating
  systems supported by C-Kermit.  Left for a future release.
*/
extern int ttnproto;			/* Defined in ckcnet.c */
extern int ttnet;			/* Defined in ckcnet.c */
extern int nopush, xfrcan, xfrchr, xfrnum; /* Defined in ckcmai.c */
extern int xsuspend, wasclosed;
extern int inserver, local;

int ckxech = 0; /* 0 if system normally echoes console characters, else 1 */

int ckmaxfiles = 0;			/* Max number of open files */

#ifdef CK_ENCRYPTION			/* Kerberos */
#include "ckuath.h"
extern int me_encrypt, u_encrypt;
#endif /* CK_ENCRYPTION */

/* Declarations of variables global within this module */

#ifdef TTLEBUF				/* See ckcnet.h */
int ttpush = -1;
#define LEBUFSIZ 4096
static CHAR le_buf[LEBUFSIZ];
static int le_start = 0, le_end = 0, le_data = 0;
#endif /* TTLEBUF */

#define MSGBUF_SIZE 1024		/* For debugging */
static char msgbuf[MSGBUF_SIZE];

static int gotsigs = 0;

static time_t tcount = (time_t)0;	/* Elapsed time counter */

static SIGTYP (*saval)()     = NULL;	/* For saving alarm() handler */
static SIGTYP (*savquit)()   = NULL;	/* and other signal handlers */
#ifdef SIGUSR1
static SIGTYP (*savusr1)()   = NULL;
#endif /* SIGUSR1 */
#ifdef SIGUSR2
static SIGTYP (*savusr2)()   = NULL;
#endif /* SIGUSR2 */
#ifdef SIGPIPE
static SIGTYP (*savpipe)()   = NULL;
#endif /* SIGPIPE */
#ifdef SIGDANGER
static SIGTYP (*savdanger)() = NULL;
#endif /* SIGDANGER */

#ifndef NOJC
static SIGTYP (*jchdlr)()    = NULL;	/* For checking suspend handler */
#endif /* NOJC */
static int jcshell = -1;		/* And flag for result */

/*
  BREAKNULS is defined for systems that simulate sending a BREAK signal
  by sending a bunch of NUL characters at low speed.
*/
#ifdef PROVX1
#ifndef BREAKNULS
#define BREAKNULS
#endif /* BREAKNULS */
#endif /* PROVX1 */

#ifdef V7
#ifndef BREAKNULS
#define BREAKNULS
#endif /* BREAKNULS */
#endif /* V7 */

#ifdef BREAKNULS
static char				/* A string of nulls */
*brnuls = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
#endif /* BREAKNULS */

#ifdef CK_POSIX_SIG			/* Longjump buffers */
static sigjmp_buf sjbuf;		/* POSIX signal handling */
#else
static jmp_buf sjbuf;
#endif /* CK_POSIX_SIG */

#ifdef V7
static jmp_buf jjbuf;
#endif /* V7 */

/* static */				/* (Not static any more) */
int ttyfd = -1;				/* TTY file descriptor */

int ttpipe = 0;				/* NETCMD: Use pipe instead of ttyfd */
int ttpty  = 0;                         /* NETPTY: Use pty instead of ttfyd */

#ifdef NETPTY				/* These are in ckupty.c */
extern PID_T pty_fork_pid;
extern int pty_master_fd, pty_slave_fd;
#endif	/* NETPTY */

#ifdef NETCMD
#ifdef NETCONN
static int pipe0[2], pipe1[2];		/* Pipes for net i/o */
#endif /* NETCONN */
static PID_T ttpid = 0;			/* Process ID for fork */
static int fdin, fdout;			/* File descriptors for pipe */
static FILE * ttout = NULL;		/* File pointer for output pipe */
#ifdef DCLFDOPEN
/* fdopen() needs declaring because it's not declared in <stdio.h> */
_PROTOTYP( FILE * fdopen, (int, char *) );
#endif /* DCLFDOPEN */
#endif /* NETCMD */

extern int pexitstat, quiet;

#ifdef Plan9
int ttyctlfd  = -1;   /* TTY control channel - What? UNIX doesn't have one? */
int consctlfd = -1;			/* Console control channel */
int noisefd = -1;			/* tone channel */
static int ttylastspeed = -1;		/* So we can lie about the speed */
#endif /* Plan9 */

int telnetfd = 0;			/* File descriptor is for telnet */
#ifdef NETCONN
int x25fd = 0;				/* File descriptor is for X.25 */
#endif /* NETCONN */

char lockpid[16] = { '\0', '\0' };	/* PID stored in lockfile, as string */

static int lkf = 0,                     /* Line lock flag */
    cgmf = 0,                           /* Flag that console modes saved */
    xlocal = 0,                         /* Flag for tty local or remote */
    curcarr = 0;			/* Carrier mode: require/ignore. */

static int netconn = 0;			/* 1 if network connection active */

static char escchr;                     /* Escape or attn character */

#ifdef CK_SCO32V4
#include <sys/time.h>
#endif /* CK_SCO32V4 */

#ifdef HAVE_TV
    static struct timeval tv;		/* For getting time, from sys/time.h */
#endif /* HAVE_TV */
#ifdef HAVE_TZ
    static struct timezone tz;
#endif /* HAVE_TZ */

#ifdef OSF
    static struct timeb ftp;            /* And from sys/timeb.h */
#endif /* OSF */

#ifdef BSD29
    static long xclock;			/* For getting time from sys/time.h */
    static struct timeb ftp;            /* And from sys/timeb.h */
#endif /* BSD29 */

#ifdef BSD41
    static long xclock;			/* For getting time from sys/time.h */
    static struct timeb ftp;            /* And from sys/timeb.h */
#endif /* BSD41 */

#ifdef BELLV10
    static long xclock;			/* For getting time from sys/time.h */
    static struct timeb ftp;            /* And from sys/timeb.h */
#endif /* BELLV10 */

#ifdef FT21
    static long xclock;			/* For getting time from sys/time.h */
    static struct timeb ftp;            /* And from sys/timeb.h */
#endif /* FT21 */

#ifdef TOWER1
    static long xclock;			/* For getting time from sys/time.h */
    static struct timeb ftp;		/* And from sys/timeb.h */
#endif /* TOWER1 */

#ifdef COHERENT
    static long xclock;			/* For getting time from sys/time.h */
    static struct timeb ftp;		/* And from sys/timeb.h */
#endif /* COHERENT */

#ifdef V7
    static long xclock;
#endif /* V7 */

/* sgtty/termio information... */

#ifdef BSD44ORPOSIX			/* POSIX or BSD44 */
  static struct termios
    ttold, ttraw, tttvt, ttcur,
    ccold, ccraw, cccbrk;
#else					/* BSD, V7, etc */

#ifdef COHERENT				/* Hack alert... */
#define ATTSV
#endif /* COHERENT */

#ifdef ATTSV
  static struct termio ttold = {0};	/* Init'd for word alignment, */
  static struct termio ttraw = {0};	/* which is important for some */
  static struct termio tttvt = {0};	/* systems, like Zilog... */
  static struct termio ttcur = {0};
  static struct termio ccold = {0};
  static struct termio ccraw = {0};
  static struct termio cccbrk = {0};
#else
  static struct sgttyb                  /* sgtty info... */
    ttold, ttraw, tttvt, ttcur, 	/* for communication line */
    ccold, ccraw, cccbrk;		/* and for console */
#ifdef BELLV10
  static struct ttydevb			/* Device info... */
    tdold, tdcur;			/* for communication device */
#endif /* BELLV10 */
#ifdef TIOCGETC
  static struct tchars tchold, tchnoi;

  static int tcharf;
#endif /* TIOCGETC */
#ifdef TIOCGLTC
  static struct ltchars ltchold, ltchnoi;
  static int ltcharf;
#endif /* TIOCGLTC */
  int lmodef = 0;			/* Local modes */
  int lmode = 0;
#endif /* ATTSV */
#endif /* BSD44ORPOSIX */

#ifdef COMMENT
/* It picks up the speeds but they don't work */
#ifdef UNIXWARE				/* For higher serial speeds */
#ifdef UW7				/* in Unixware 7.0 */
#include <sys/asyc.h>			/* This picks up 57600 and 115200 */
#endif /* UW7 */
#endif /* UNIXWARE */
#endif /* COMMENT */

#ifdef PROVX1
  static struct sgttyb ttbuf;
#endif /* PROVX1 */

#ifdef ultrix
/* do we really need this? */
  static struct sgttyb vanilla;
#endif /* ultrix */

#ifdef ATT7300
static int attmodem = 0;                /* ATT7300 internal-modem status */
struct updata dialer = {0};		/* Condition dialer for data call */
#endif /* ATT7300 */

#ifndef NOUUCP
#define FLFNAML 128
#ifndef USETTYLOCK
#ifdef RTAIX
char lkflfn[FLFNAML] = { '\0', '\0' };	/* and possible link to it */
#endif /* RTAIX */
char lock2[FLFNAML] =  { '\0', '\0' };	/* Name of second lockfile */
#endif /* USETTYLOCK */
#else
#define FLFNAML 7
#endif /* NOUUCP */
char flfnam[FLFNAML+1] = { '\0', '\0' }; /* UUCP lock file path name */

int haslock = 0;			/* =1 if this kermit locked uucp */

#ifndef OXOS
#ifdef SVORPOSIX
static int conesc = 0;                  /* set to 1 if esc char (^\) typed */
#else
#ifdef V7
static int conesc = 0;
#else
#ifdef C70
static int conesc = 0;
#endif /* C70 */
#endif /* V7 */
#endif /* SVORPOSIX */
#endif /* OXOS */

/* Local copy of comm device name or network host */
static char ttnmsv[DEVNAMLEN+1] = { '\0', '\0' };
#ifdef USETTYLOCK
static char lockname[DEVNAMLEN+1];	/* Ditto, the part after "/dev/". */
#endif /* USETTYLOCK */

#ifdef aegis
static status_$t st;                    /* error status return value */
static short concrp = 0;                /* true if console is CRP pad */
static uid_$t ttyuid;                   /* tty type uid */
static uid_$t conuid;                   /* stdout type uid */

/* APOLLO Aegis main()
 * establish acl usage and cleanup handling
 *    this makes sure that CRP pads
 *    get restored to a usable mode
 */
main(argc,argv) int argc; char **argv; {
        status_$t status;
        pfm_$cleanup_rec dirty;

        PID_T pid = getpid();

        /* acl usage according to invoking environment */
        default_acl(USE_DEFENV);

        /* establish a cleanup continuation */
        status = pfm_$cleanup(dirty);
        if (status.all != pfm_$cleanup_set) {
                /* only handle faults for the original process */
                if (pid == getpid() && status.all > pgm_$max_severity) {
		    /* blew up in main process */
		    status_$t quo;
		    pfm_$cleanup_rec clean;

		    /* restore the console in any case */
		    conres();

		    /* attempt a clean exit */
		    debug(F101, "cleanup fault status", "", status.all);

		    /* doexit(), then send status to continuation */
		    quo = pfm_$cleanup(clean);
		    if (quo.all == pfm_$cleanup_set)
		      doexit(pgm_$program_faulted,-1);
		    else if (quo.all > pgm_$max_severity)
		      pfm_$signal(quo); /* blew up in doexit() */
                }
                /* send to the original continuation */
                pfm_$signal(status);
                /*NOTREACHED*/
	    }
        return(ckcmai(argc, argv));
}
#endif /* aegis */

/* ANSI-style prototypes for internal functions. */
/* Functions used outside this module are prototyped in ckcker.h. */

#ifdef apollo
_PROTOTYP( SIGTYP timerh, () );
_PROTOTYP( SIGTYP cctrap, () );
_PROTOTYP( SIGTYP esctrp, () );
_PROTOTYP( SIGTYP sig_ign, () );
#else
_PROTOTYP( SIGTYP timerh, (int) );
_PROTOTYP( SIGTYP cctrap, (int) );
_PROTOTYP( SIGTYP esctrp, (int) );
#endif /* apollo */
_PROTOTYP( int do_open, (char *) );
_PROTOTYP( static int in_chk, (int, int) );
_PROTOTYP( static int ttrpid, (char *) );
_PROTOTYP( static int ttchkpid, (char *) );
_PROTOTYP( static int ttlock, (char *) );
_PROTOTYP( static int ttunlck, (void) );
_PROTOTYP( static VOID sigchld_handler, (int) );
_PROTOTYP( int mygetbuf, (void) );
_PROTOTYP( int myfillbuf, (void) );
_PROTOTYP( VOID conbgt, (int) );
#ifdef ACUCNTRL
_PROTOTYP( VOID acucntrl, (char *, char *) );
#endif /* ACUCNTRL */

#ifdef BSD44ORPOSIX
_PROTOTYP( int carrctl, (struct termios *, int) );
#else
#ifdef ATTSV
_PROTOTYP( int carrctl, (struct termio *, int) );
#else
_PROTOTYP( int carrctl, (struct sgttyb *, int) );
#endif /* ATTSV */
#endif /* BSD44ORPOSIX */

#ifdef ATT7300
_PROTOTYP( int attdial, (char *, long, char *) );
_PROTOTYP( int offgetty, (char *) );
_PROTOTYP( int ongetty, (char *) );
#endif /* ATT7300 */

#ifdef BEOSORBEBOX
#ifdef SELECT
    /* BeOS is not capable of using SELECT on anything but sockets */
#undef SELECT
#endif /* SELECT */
#include <kernel/OS.h>
/* #ifdef BE_DR_7 */
static double time_started = 0.0;
struct ALARM_STRUCT {
    thread_id thread;
    int time;
};
static thread_id alarm_thread = -1;
static struct ALARM_STRUCT alarm_struct;
_PROTOTYP( long do_alarm, (void *) );
_PROTOTYP( unsigned int alarm, (unsigned int) );
_PROTOTYP( void alarm_expired, (void) );
/* #endif */ /* BE_DR_7 */
#endif /* BEOSORBEBOX */

#ifndef xunchar
#define xunchar(ch) (((ch) - 32 ) & 0xFF )	/* Character to number */
#endif /* xunchar */

#ifdef CK_ANSIC
static char *
xxlast(char *s, char c)
#else
static char *
xxlast(s,c) char *s; char c;
#endif /* CK_ANSIC */
/* xxlast */ {		/*  Last occurrence of character c in string s. */
    int i;
    for (i = (int)strlen(s); i > 0; i--)
      if (s[i-1] == c ) return(s + (i - 1));
    return(NULL);
}

/* Timeout handler for communication line input functions */

/*ARGSUSED*/
SIGTYP
timerh(foo) int foo; {
    ttimoff();
#ifdef BEOSORBEBOX
/* #ifdef BE_DR_7 */
    alarm_expired();
/* #endif */ /* BE_DR_7 */
#endif /* BEOSORBEBOX */
#ifdef CK_POSIX_SIG
    siglongjmp(sjbuf,1);
#else
    longjmp(sjbuf,1);
#endif /* CK_POSIX_SIG */
}

/*ARGSUSED*/
SIGTYP
xtimerh(foo) int foo; {			/* Like timerh() but does */
#ifdef BEOSORBEBOX			/* not reset the timer itself */
/* #ifdef BE_DR_7 */
    alarm_expired();
/* #endif */ /* BE_DR_7 */
#endif /* BEOSORBEBOX */
#ifdef CK_POSIX_SIG
    siglongjmp(sjbuf,1);
#else
    longjmp(sjbuf,1);
#endif /* CK_POSIX_SIG */
}


/* Control-C trap for communication line input functions */

int cc_int;				/* Flag */
SIGTYP (* occt)();			/* For saving old SIGINT handler */

/*ARGSUSED*/
SIGTYP
cctrap(foo) int foo; {			/* Needs arg for ANSI C */
  cc_int = 1;				/* signal() prototype. */
  return;
}

/*  S Y S I N I T  --  System-dependent program initialization.  */

/*
 * ttgwsiz() returns:
 *	1    tt_rows and tt_cols are known, both altered, both > 0
 *	0    tt_rows and/or tt_cols are known, both altered, one or both <= 0
 *	-1   tt_rows and tt_cols are unknown and unaltered
 */

extern int tt_rows, tt_cols;

static int
xttgwsiz() {
    char *p;
    int rows = 0, cols = 0;
    p = getenv("LINES");
    debug(F110,"xttgwsiz LINES",p,0);
    if (p) {
	rows = atol(p);
	if (rows > 0) {
	    p = getenv("COLUMNS");
	    debug(F110,"xttgwsiz COLUMNS",p,0);
	    if (p) {
		cols = atol(p);
		if (cols > 0) {
		    tt_rows = rows;
		    tt_cols = cols;
		    return(1);
		}
		return(0);
	    }
	}
    }
    return(-1);
}

#ifdef TTLEBUF
VOID
le_init() {				/* LocalEchoInit() */
    int i;
    for (i = 0; i < LEBUFSIZ; i++)
      le_buf[i] = '\0';
    le_start = 0;
    le_end = 0;
    le_data = 0;
}

VOID
le_clean() {				/* LocalEchoCleanup() */
    le_init();
    return;
}

int
le_inbuf() {
    int rc = 0;
    if (le_start != le_end) {
	rc = (le_end -
	      le_start +
	      LEBUFSIZ) % LEBUFSIZ;
    }
    debug(F111,"le_inbuf","chars waiting",rc);
    return(rc);
}

int
#ifdef CK_ANSIC
le_putchar(CHAR ch)
#else
le_putchar(ch) CHAR ch;
#endif /* CK_ANSIC */
/* le_putchar */ {
#ifdef COMMENT
    /* In UNIX we do not have another thread taking chars out of the buffer */
    while ((le_start - le_end == 1) ||
            (le_start == 0 && le_end == LEBUFSIZ - 1)) {
	/* Buffer is full */
        debug(F111,"le_putchar","Buffer is Full",ch);
        ReleaseLocalEchoMutex() ;
        msleep(250);
        RequestLocalEchoMutex( SEM_INDEFINITE_WAIT ) ;
    }
#else
    if ((le_start - le_end + LEBUFSIZ)%LEBUFSIZ == 1) {
        debug(F110,"le_putchar","buffer is full",0);
        return(-1);
    }
#endif /* COMMENT */
    le_buf[le_end++] = ch;
    if (le_end == LEBUFSIZ)
      le_end = 0;
    le_data = 1;
    return(0);
}

int
#ifdef CK_ANSIC
le_puts(CHAR * s, int n)
#else
le_puts(s,n) CHAR * s; int n;
#endif /* CK_ANSIC */
/* le_puts */ {
    int rc = 0;
    int i = 0;
    CHAR * p = (CHAR *)"le_puts";
    ckhexdump(p,s,n);
    for (i = 0; i < n; i++)
      rc = le_putchar((char)s[i]);
    debug(F101,"le_puts","",rc);
    return(rc);
}

int
#ifdef CK_ANSIC
le_putstr(CHAR * s)
#else
le_putstr(s) CHAR * s;
#endif /* CK_ANSIC */
/* le_puts */ {
    CHAR * p;
    int rc = 0;
    p = (CHAR *)"le_putstr";
    ckhexdump(p,s,(int)strlen((char *)s));
    for (p = s; *p && !rc; p++)
      rc = le_putchar(*p);
    return(rc);
}

int
#ifdef CK_ANSIC
le_getchar(CHAR * pch)
#else /* CK_ANSIC */
le_getchar(pch) CHAR * pch;
#endif /* CK_ANSIC */
/* le_gatchar */ {
    int rc = 0;
    if (le_start != le_end) {
        *pch = le_buf[le_start];
        le_buf[le_start] = 0;
        le_start++;

        if (le_start == LEBUFSIZ)
          le_start = 0;

        if (le_start == le_end) {
            le_data = 0;
        }
        rc++;
    } else {
        *pch = 0;
    }
    return(rc);
}
#endif /* TTLEBUF */

#ifdef COMMENT
/*
  Some systems like OSF/1 use TIOCGSIZE instead of TIOCGWINSZ.
  But as far as I know, whenever TIOCGSIZE is defined, it is
  equated to TIOCGWINSZ.  For cases where this is not done, try this:
*/
#ifndef TIOCGWINSZ
#ifdef TIOCGSIZE
#define TIOCGWINSZ TIOCGSIZE
#endif /* TIOCGSIZE */
#endif /* TIOCGWINSZ */
#endif /* COMMENT */

static int tt_xpixel = 0, tt_ypixel = 0;

int
ttgwsiz() {
    int x = 0;
#ifndef NONAWS
#ifdef QNX
/*
  NOTE: TIOCGWSIZ works here too, but only in the 32-bit version.
  This code works for both the 16- and 32-bit versions.
*/
    extern int dev_size(int, int, int, int *, int *);
    int r, c;

    if (dev_size(0, -1, -1, &r, &c) == 0) {
	debug(F101,"ttgwsiz QNX r","",r);
	debug(F101,"ttgwsiz QNX c","",c);
	tt_rows = r;
	tt_cols = c;
	return ((r > 0 && c > 0) ? 1 : 0);
    } else return(xttgwsiz());
#else /* QNX */
#ifdef TIOCGWINSZ

/* Note, this was M_UNIX, changed to XENIX to allow cross compilation... */
#ifdef XENIX				/* SCO UNIX 3.2v4.0 */
#include <sys/stream.h>			/* typedef mblk_t needed by ptem.h */
#include <sys/ptem.h>			/* for ttgwsiz() */
#endif /* XENIX */

#ifdef I386IX				/* Ditto for Interactive */
#include <sys/stream.h>
#include <sys/ptem.h>
#endif /* I386IX */

/* Note, the above might be needed for some other older SVR3 Intel makes... */

    struct winsize w;
    tt_xpixel = 0;
    tt_ypixel = 0;

#ifdef IKSD
    if (inserver)
      return(xttgwsiz());
#endif /* IKSD */
    x = ioctl(0, (int)TIOCGWINSZ, (char *)&w);
    debug(F101,"ttgwsiz TIOCGWINSZ","",x);
    if (x < 0) {
	return(xttgwsiz());
    } else if (w.ws_row > 0 && w.ws_col > 0) {
	tt_rows = w.ws_row;
	tt_cols = w.ws_col;
	tt_xpixel = w.ws_xpixel;
	tt_ypixel = w.ws_ypixel;
	debug(F101,"ttgwsiz tt_rows","",tt_rows);
	debug(F101,"ttgwsiz tt_cols","",tt_cols);
	return(1);
    } else {
	debug(F100,"ttgwsiz TIOCGWINSZ 00","",0);
	return(xttgwsiz());
    }
#else
    return(xttgwsiz());
#endif /* TIOCGWINSZ */
#endif /* QNX */
#endif /* NONAWS */
}


#ifdef RLOGCODE
_PROTOTYP( int rlog_naws, (void) );
#endif	/* RLOGCODE */

#ifndef NOSIGWINCH
#ifdef SIGWINCH
SIGTYP
winchh(foo) int foo; {			/* SIGWINCH handler */
    int x = 0;
#ifdef CK_TTYFD
#ifndef VMS
    extern int ttyfd;
#endif /* VMS */
#endif /* CK_TTYFD */
    extern int tt_rows, tt_cols, cmd_rows, cmd_cols;
#ifdef DEBUG
    if (deblog) {
	debug(F100,"***************","",0);
	debug(F100,"SIGWINCH caught","",0);
	debug(F100,"***************","",0);
#ifdef NETPTY
	debug(F101,"SIGWINCH pty_fork_pid","",pty_fork_pid);
#endif /* NETPTY */
    }
#endif /* DEUB */
    signal(SIGWINCH,winchh);            /* Re-arm the signal */
    x = ttgwsiz();                      /* Get new window size */
    cmd_rows = tt_rows;			/* Adjust command screen too */
    cmd_cols = tt_cols;

#ifdef CK_TTYFD
    if					/* If we don't have a connection */
#ifdef VMS				/* we're done. */
      (vmsttyfd() == -1)
#else
      (ttyfd == -1)
#endif /* VMS */
#else
      (!local)
#endif /* CK_TTYFD */
        return;

#ifdef NETPTY
    if (pty_fork_pid > -1) {		/* "set host" to a PTY? */
	int x;

#ifdef TIOCSWINSZ
	struct winsize w;		/* Resize the PTY */
	errno = 0;
	w.ws_col = tt_cols;
	w.ws_row = tt_rows;
	w.ws_xpixel = tt_xpixel;
	w.ws_ypixel = tt_ypixel;
	x = ioctl(ttyfd,TIOCSWINSZ,&w);
	debug(F101,"winchh TIOCSWINSZ","",x);
	debug(F101,"winchh TIOCSWINSZ errno","",errno);
#endif /* TIOCSWINSZ */

	errno = 0;
	x = kill(pty_fork_pid,SIGWINCH);
	debug(F101,"winchh kill","",x);
	debug(F101,"winchh kill errno","",errno);
    }
#endif /* NETPTY */

/*
  This should be OK.  It might seem that sending this from
  interrupt level could interfere with another TELNET IAC string
  that was in the process of being sent.  But we always send
  TELNET strings with a single write(), which should prevent mixups.
  blah_snaws() should protect themselves from being called on the
  wrong kind of connection.
*/
#ifdef TCPSOCKET
#ifndef NOTTGWSIZ
    if (x > 0 && tt_rows > 0 && tt_cols > 0) {
        tn_snaws();
#ifdef RLOGCODE
        rlog_naws();
#endif /* RLOGCODE */
    }
#endif /* NOTTGWSIZ */
#endif /* TCPSOCKET */
    SIGRETURN;
}
#endif /* SIGWINCH */
#endif /* NOSIGWINCH */

SIGTYP
sighup(foo) int foo; {			/* SIGHUP handler */
    backgrd = 1;
    debug(F100,"***************","",0);
    debug(F100,"SIGHUP received","",0);
    debug(F100,"***************","",0);
    doexit(BAD_EXIT,-1);
    /*NOTREACHED*/
    SIGRETURN;				/* Shut picky compilers up... */
}

#ifdef CK_SCO32V4
/* Exists but there is no prototype in the header files */
_PROTOTYP( char * ttyname, (int) );
#else
#ifdef SV68R3V6
_PROTOTYP( char * ttyname, (int) );
#else
#ifdef ultrix
_PROTOTYP( char * ttyname, (int) );
#else
#ifdef HPUX6
_PROTOTYP( char * ttyname, (int) );
#else
#ifdef HPUX5
_PROTOTYP( char * ttyname, (int) );
#else
#ifdef PS2AIX10
_PROTOTYP( char * ttyname, (int) );
#else
#ifdef BSD42
_PROTOTYP( char * ttyname, (int) );
#endif /* BSD42 */
#endif /* PS2AIX10 */
#endif /* HPUX5 */
#endif /* HPUX6 */
#endif /* ultrix */
#endif /* SV68R3V6 */
#endif /* CK_SCO32V4 */

#ifndef SIGUSR1				/* User-defined signals */
#define SIGUSR1 30
#endif /* SIGUSR1 */

#ifndef SIGUSR2
#define SIGUSR2 31
#endif /* SIGUSR2 */

/*
  ignorsigs() sets certain signals to SIG_IGN.  But when a signal is
  ignored, it remains ignored across exec(), so we have to restore these
  signals before exec(), which is the purpose of restorsigs().
*/
static VOID
ignorsigs() {				/* Ignore these signals */
    savquit = signal(SIGQUIT,SIG_IGN);	/* Ignore Quit signal */

#ifdef SIGDANGER			/* Ignore danger signals */
/*
  This signal is sent when the system is low on swap space.  Processes
  that don't handle it are candidates for termination.  If swap space doesn't
  clear out enough, we still might be terminated via kill() -- nothing we can
  do about that!  Conceivably, this could be improved by installing a real
  signal handler that warns the user, but that would be pretty complicated,
  since we are not always in control of the screen -- e.g. during remote-mode
  file transfer.
*/
    savdanger = signal(SIGDANGER,SIG_IGN); /* e.g. in AIX */
#endif /* SIGDANGER */
#ifdef SIGPIPE
/*
  This one comes when a TCP/IP connection is broken by the remote.
  We prefer to catch this situation by examining error codes from write().
*/
    savpipe = signal(SIGPIPE,SIG_IGN);
#endif /* SIGPIPE */
    savusr1 = signal(SIGUSR1,SIG_IGN);	/* Ignore user-defined signals */
    savusr2 = signal(SIGUSR2,SIG_IGN);
}

VOID
restorsigs() {				/* Restore these signals */
    (VOID) signal(SIGQUIT,savquit);	/* (used in ckufio.c) */
#ifdef SIGDANGER
    (VOID) signal(SIGDANGER,savdanger);
#endif /* SIGDANGER */
#ifdef SIGPIPE
    (VOID) signal(SIGPIPE,savpipe);
#endif /* SIGPIPE */
    (VOID) signal(SIGUSR1,savusr1);
    (VOID) signal(SIGUSR2,savusr2);
}

int
sysinit() {
    int x;
    char * s;
#ifdef CK_UTSNAME
    struct utsname name;
#endif /* CK_UTSNAME */

    extern char startupdir[];
/*
  BEFORE ANYTHING ELSE: Initialize the setuid package.
  Change to the user's real user and group ID.
  If this can't be done, don't run at all.
*/
    x = priv_ini();
#ifdef SUIDDEBUG
    fprintf(stderr,"PRIV_INI=%d\n",x);
#endif /* SUIDDEBUG */
    if (x) {
	if (x & 1) fprintf(stderr,"Fatal: setuid failure.\n");
	if (x & 2) fprintf(stderr,"Fatal: setgid failure.\n");
	if (x & 4) fprintf(stderr,"Fatal: C-Kermit setuid to root!\n");
	exit(1);
    }
    signal(SIGINT,SIG_IGN);		/* Ignore interrupts at first */
    signal(SIGFPE,SIG_IGN);		/* Ignore floating-point exceptions */
    signal(SIGHUP,sighup);		/* Catch SIGHUP */
#ifndef NOSIGWINCH
#ifdef SIGWINCH
    signal(SIGWINCH,winchh);		/* Catch window-size change */
#endif /* SIGWINCH */
#endif /* NOSIGWINCH */

#ifdef SIGXFSZ
    signal(SIGXFSZ,SIG_IGN);		/* Ignore writing past file limit */ 
#endif	/* SIGXFSZ */

#ifndef NOJC
/*
  Get the initial job control state.
  If it is SIG_IGN, that means the shell does not support job control,
  and so we'd better not suspend ourselves.
*/
#ifdef SIGTSTP
    jchdlr = signal(SIGTSTP,SIG_IGN);
    if (jchdlr == SIG_IGN) {
	jcshell = 0;
	debug(F100,"sysinit jchdlr: SIG_IGN","",0);
    } else if (jchdlr == SIG_DFL) {
	debug(F100,"sysinit jchdlr: SIG_DFL","",0);
	jcshell = 1;
    } else {
	debug(F100,"sysinit jchdlr: other","",0);
	jcshell = 3;
    }
    (VOID) signal(SIGTSTP,jchdlr);	/* Put it back... */
#endif /* SIGTSTP */
#endif /* NOJC */

    conbgt(0);				/* See if we're in the background */
    congm();				/* Get console modes */

    (VOID) signal(SIGALRM,SIG_IGN);	/* Ignore alarms */

    ignorsigs();			/* Ignore some other signals */

#ifdef F_SETFL
    iniflags = fcntl(0,F_GETFL,0);	/* Get stdin flags */
#endif /* F_SETFL */

#ifdef ultrix
    gtty(0,&vanilla);			/* Get sgtty info */
#else
#ifdef AUX
    set42sig();				/* Don't ask! (hakanson@cs.orst.edu) */
#endif /* AUX */
#endif /* ultrix */
/*
  Warning: on some UNIX systems (SVR4?), ttyname() reportedly opens /dev but
  never closes it.  If it is called often enough, we run out of file
  descriptors and subsequent open()'s of other devices or files can fail.
*/
    s = NULL;
#ifndef MINIX
    if (isatty(0))			/* Name of controlling terminal */
      s = ttyname(0);
    else if (isatty(1))
      s = ttyname(1);
    else if (isatty(2))
      s = ttyname(2);
    debug(F110,"sysinit ttyname(0)",s,0);
#endif /* MINIX */

#ifdef BEOS
    if (!dftty)
      makestr(&dftty,s);
#endif /* BEOS */

    if (s)
      ckstrncpy((char *)cttnam,s,DEVNAMLEN+1);
#ifdef SVORPOSIX
    if (!cttnam[0])
      ctermid(cttnam);
#endif /* SVORPOSIX */
    if (!cttnam[0])
      ckstrncpy((char *)cttnam,dftty,DEVNAMLEN+1);
    debug(F110,"sysinit CTTNAM",CTTNAM,0);
    debug(F110,"sysinit cttnam",cttnam,0);

    ttgwsiz();				/* Get window (screen) dimensions. */

#ifndef NOSYSCONF
#ifdef _SC_OPEN_MAX
    ckmaxfiles = sysconf(_SC_OPEN_MAX);
#endif /* _SC_OPEN_MAX */
#endif /* NOSYSCONF */

#ifdef Plan9
    if (!backgrd) {
    	consctlfd = open("/dev/consctl", O_WRONLY);
    	/*noisefd = open("/dev/noise", O_WRONLY)*/
    }
    ckxech = 1;
#endif /* Plan9 */

#ifdef CK_UTSNAME
    if (uname(&name) > -1) {
	ckstrncpy(unm_mch,name.machine,CK_SYSNMLN);
	ckstrncpy(unm_nam,name.sysname,CK_SYSNMLN);
	ckstrncpy(unm_rel,name.release,CK_SYSNMLN);
	ckstrncpy(unm_ver,name.version,CK_SYSNMLN);
#ifdef DEBUG
	if (deblog) {
	    debug(F110,"sysinit uname machine",unm_mch,0);
	    debug(F110,"sysinit uname sysname",unm_nam,0);
	    debug(F110,"sysinit uname release",unm_rel,0);
	    debug(F110,"sysinit uname version",unm_ver,0);
	}
#endif /* DEBUG */

#ifdef HPUX9PLUS
	if (name.machine[5] == '8')
	  hpis800 = 1;
	else
	  hpis800 = 0;
	debug(F101,"sysinit hpis800","",hpis800);
#endif /* HPUX9PLUS */
#ifdef TRU64
        getsysinfo(GSI_PLATFORM_NAME, unm_mod, CK_SYSNMLN, 0, 0);
        debug(F110,"sysinit getsysinfo model",unm_mod,0);
#endif /* TRU64 */
#ifdef SOLARIS25
        sysinfo(SI_PLATFORM, unm_mod, CK_SYSNMLN);
        debug(F110,"sysinit sysinfo model",unm_mod,0);
#endif /* SOLARIS25 */
    }
#endif /* CK_UTSNAME */

#ifdef CK_ENVIRONMENT
    {
#ifdef TNCODE
	extern char tn_env_acct[], tn_env_disp[], tn_env_job[],
	tn_env_prnt[], tn_env_sys[];
#endif /* TNCODE */
	extern char uidbuf[];
        extern char * whoami();
	char *p;
#ifdef CKSENDUID
        uidbuf[0] = '\0';
#ifdef IKSD
        if (!inserver) {
#endif /* IKSD */
            p = getenv("USER");
            debug(F110,"sysinit uidbuf from USER",uidbuf,0);
	    if (!p) p = "";
            if (!*p) {
                p = getenv("LOGNAME");
                debug(F110,"sysinit uidbuf from LOGNAME",uidbuf,0);
            }
	    if (!p) p = "";
            if (!*p) {
                p = whoami();
                debug(F110,"sysinit uidbuf from whoami()",uidbuf,0);
            }
	    if (!p) p = "";
	    ckstrncpy(uidbuf, *p ? p : "UNKNOWN", UIDBUFLEN);
#ifdef IKSD
        }
#endif /* IKSD */
	debug(F110,"sysinit final uidbuf",uidbuf,0);
#endif /* CKSENDUID */

#ifdef TNCODE
	if ((p = getenv("JOB"))) ckstrncpy(tn_env_job,p,63);
	if ((p = getenv("ACCT"))) ckstrncpy(tn_env_acct,p,63);
	if ((p = getenv("PRINTER"))) ckstrncpy(tn_env_prnt,p,63);
	if ((p = getenv("DISPLAY"))) ckstrncpy(tn_env_disp,p,63);
#ifdef aegis
	ckstrncpy(tn_env_sys,"Aegis",64);
#else
#ifdef Plan9
	ckstrncpy(tn_env_sys,"Plan9",64);
#else
	ckstrncpy(tn_env_sys,"UNIX",64);
#endif /* Plan9 */
#endif /* aegis */
#endif /* TNCODE */
    }
#endif /* CK_ENVIRONMENT */
#ifdef CK_SNDLOC
    {
	extern char * tn_loc;
	char *p;
	if (p = getenv("LOCATION"))
	  if (tn_loc = (char *)malloc((int)strlen(p)+1))
	    strcpy(tn_loc,p);		/* safe */
    }
#endif /* CK_SNDLOC */

    ckstrncpy(startupdir, zgtdir(), CKMAXPATH);
    startupdir[CKMAXPATH] = '\0';
    x = strlen(startupdir);
    if (x <= 0) {
	startupdir[0] = '/';
	startupdir[1] = '\0';
    } else if (startupdir[x-1] != '/') {
	startupdir[x] = '/';
	startupdir[x+1] = '\0';
    }
    debug(F110,"sysinit startupdir",startupdir,0);
#ifdef TTLEBUF
    le_init();
#endif /* TTLEBUF */
#ifdef BSD44ORPOSIX
    /* This should catch the ncurses platforms */
    /* Some platforms don't have putenv(), like NeXTSTEP */
    putenv("NCURSES_NO_SETBUF=1");
#endif /* BSD44ORPOSIX */
    return(0);
}

/*  S Y S C L E A N U P  --  System-dependent program cleanup.  */

int
syscleanup() {
#ifdef F_SETFL
    if (iniflags > -1)
      fcntl(0,F_SETFL,iniflags);	/* Restore stdin flags */
#endif /* F_SETFL */
#ifdef ultrix
    stty(0,&vanilla);                   /* Get sgtty info */
#endif /* ultrix */
#ifdef NETCMD
    if (ttpid) kill(ttpid,9);
#endif /* NETCMD */
    return(0);
}

/*  T T O P E N  --  Open a tty for exclusive access.  */

/*
  Call with:
    ttname: character string - device name or network host name.
    lcl:
  If called with lcl < 0, sets value of lcl as follows:
  0: the terminal named by ttname is the job's controlling terminal.
  1: the terminal named by ttname is not the job's controlling terminal.
  But watch out: if a line is already open, or if requested line can't
  be opened, then lcl remains (and is returned as) -1.
    modem:
  Less than zero: ttname is a network host name.
  Zero or greater: ttname is a terminal device name.
  Zero means a local connection (don't use modem signals).
  Positive means use modem signals.
   timo:
  0 = no timer.
  nonzero = number of seconds to wait for open() to return before timing out.

  Returns:
    0 on success
   -5 if device is in use
   -4 if access to device is denied
   -3 if access to lock directory denied
   -2 upon timeout waiting for device to open
   -1 on other error
*/
static int ttotmo = 0;			/* Timeout flag */
/* Flag kept here to avoid being clobbered by longjmp.  */

int
ttopen(ttname,lcl,modem,timo) char *ttname; int *lcl, modem, timo; {

#ifdef BSD44
#define ctermid(x) strcpy(x,"")
#else
#ifdef SVORPOSIX
#ifndef CIE
    extern char *ctermid();		/* Wish they all had this! */
#else					/* CIE Regulus */
#define ctermid(x) strcpy(x,"")
#endif /* CIE */
#endif /* SVORPOSIX */
#endif /* BSD44 */

#ifdef ultrix
    int temp = 0;
#endif /* ultrix */

#ifndef OPENFIRST
    char fullname[DEVNAMLEN+1];
#endif /* OPENFIRST */

    char * fnam;			/* Full name after expansion */

    int y;

#ifndef pdp11
#define NAMEFD	 /* Feature to allow name to be an open file descriptor */
#endif /* pdp11 */

#ifdef NAMEFD
    char *p;
    debug(F101,"ttopen telnetfd","",telnetfd);
#endif /* NAMEFD */

    debug(F110,"ttopen ttname",ttname,0);
    debug(F110,"ttopen ttnmsv",ttnmsv,0);
    debug(F101,"ttopen modem","",modem);
    debug(F101,"ttopen netconn","",netconn);
    debug(F101,"ttopen ttyfd","",ttyfd);
    debug(F101,"ttopen *lcl","",*lcl);
    debug(F101,"ttopen ttmdm","",ttmdm);
    debug(F101,"ttopen ttnet","",ttnet);

    ttpmsk = 0xff;
    lockpid[0] = '\0';

    if (ttyfd > -1) {			/* If device already opened */
        if (!strncmp(ttname,ttnmsv,DEVNAMLEN)) /* are new & old names equal? */
	  return(0);			/* Yes, nothing to do - just return */
	ttnmsv[0] = '\0';		/* No, clear out old name */
	ttclos(ttyfd);			/* close old connection.  */
    }
    wasclosed = 0;			/* New connection, not closed yet. */
    ttpipe = 0;				/* Assume it's not a pipe */
    ttpty = 0;				/* or a pty... */

#ifdef NETCONN
/*
  This is a bit tricky...  Suppose that previously Kermit had dialed a telnet
  modem server ("set host xxx:2001, set modem type usr, dial ...").  Then the
  connection was closed (ttyfd = -1), and then a REDIAL command was given.  At
  this point we've obliterated the negative modem type hack, and so would
  treat the IP hostname as a device name, and would then fail because of "No
  such device or directory".  But the previous connection has left behind some
  clues, so let's use them...
*/
    if (ttyfd < 0) {			/* Connection is not open */
	if (!strcmp(ttname,ttnmsv)) {	/* Old and new names the same? */
	    if (((netconn > 0) && (ttmdm < 0)) ||
		((ttnet > 0) &&
		 (!ckstrchr(ttname,'/')) && (ckstrchr(ttname,':')))
		) {
		int x, rc;
		x = (ttmdm < 0) ? -ttmdm : ttnet;
		rc = netopen(ttname, lcl, x);
		debug(F111,"ttopen REOPEN netopen",ttname,rc);
		if (rc > -1) {
		    netconn = 1;
		    xlocal = *lcl = 1;
		} else {
		    netconn = 0;
		}
		gotsigs = 0;
		return(rc);
	    }
	}
    }
#endif /* NETCONN */

#ifdef MAXNAMLEN
    debug(F100,"ttopen MAXNAMLEN defined","",0);
#else
    debug(F100,"ttopen MAXNAMLEN *NOT* defined","",0);
#endif

#ifdef BSD4
    debug(F100,"ttopen BSD4 defined","",0);
#else
    debug(F100,"ttopen BSD4 *NOT* defined","",0);
#endif /* BSD4 */

#ifdef BSD42
    debug(F100,"ttopen BSD42 defined","",0);
#else
    debug(F100,"ttopen BSD42 *NOT* defined","",0);
#endif /* BSD42 */

#ifdef MYREAD
    debug(F100,"ttopen MYREAD defined","",0);
#else
    debug(F100,"ttopen MYREAD *NOT* defined","",0);
#endif /* MYREAD */

#ifdef	NETCONN
    if (modem < 0) {			/* modem < 0 = code for network */
	int x;
	ttmdm = modem;
	modem = -modem;			/* Positive network type number */
	fdflag = 0;			/* Stdio not redirected. */
	netconn = 1;			/* And it's a network connection */
	debug(F111,"ttopen net",ttname,modem);
#ifdef NAMEFD
	for (p = ttname; isdigit(*p); p++) ; /* Check for all digits */
 	if (*p == '\0' && (telnetfd || x25fd)) { /* Avoid X.121 addresses */
	    ttyfd = atoi(ttname);	/* Is there a way to test it's open? */
	    ttfdflg = 1;		/* We got an open file descriptor */
	    debug(F111,"ttopen net ttfdflg",ttname,ttfdflg);
	    debug(F101,"ttopen net ttyfd","",ttyfd);
	    ckstrncpy(ttnmsv,ttname,DEVNAMLEN); /* Remember the "name". */
	    x = 1;			/* Return code is "good". */
	    if (telnetfd) {
		ttnet = NET_TCPB;
		if (ttnproto != NP_TCPRAW)
		  ttnproto = NP_TELNET;
#ifdef SUNX25
	    } else if (x25fd) {
		ttnet = NET_SX25;
		ttnproto = NP_NONE;
#endif /* SUNX25 */
	    }
	} else {			/* Host name or address given */
#ifdef NETPTY
	    if (modem == NET_PTY) {
		int x;
		if (nopush) {
		    debug(F100,"ttopen PTY: nopush","",0);
		    return(-1);
		}
                ttnet = NET_PTY;
		ttnproto = NP_NONE;
                netconn = 1;            /* but we don't use network i/o */
                ttpty = 1;
                debug(F110,"ttopen PTY",ttname,0);
		x = do_pty(&ttyfd,ttname,0);
		if (x > -1) {
		    ckstrncpy(ttnmsv,ttname,DEVNAMLEN);
		    xlocal = *lcl = 1;	/* It's local */
		} else {
		    ttpty = 0;
		    netconn = 0;
		}
		gotsigs = 0;
		return(x);
	    }
#endif /* NETPTY */
#ifdef NETCMD
/*
  dup2() is not available on older System V platforms like AT&T 3Bx.  For
  those systems we punt by not defining NETCMD, but we might be able to do
  better -- see workarounds for this problem in ckufio.c (search for dup2).
*/
	    if (modem == NET_CMD) {
		if (nopush) {
		    debug(F100,"ttopen pipe: nopush","",0);
		    return(-1);
		}
		if (pipe(pipe0) || pipe(pipe1)) {
		    perror("Pipe error");
		    return(-1);
		}
		ttpid = fork();		/* Make a fork */

		switch (ttpid) {
		  case -1:		/* Error making fork */
		    close(pipe0[0]);
		    close(pipe0[1]);
		    close(pipe1[0]);
		    close(pipe1[1]);
		    perror("Fork error");
		    return(-1);
		  case 0:		/* Child. */
		    close(pipe0[0]);
		    close(pipe1[1]);
		    dup2(pipe0[1], 1);
		    close(pipe0[1]);
		    dup2(pipe1[0], 0);
		    close(pipe1[0]);
		    system(ttname);
		    _exit(0);
		  default:		/* Parent */
		    close(pipe0[1]);
		    close(pipe1[0]);
		    fdin = pipe0[0];	/* Read from pipe */
		    fdout = pipe1[1];	/* Write to pipe */
		    ttout = fdopen(fdout,"w"); /* Get stream so we can */
		    if (!ttout) {	/* make it unbuffered. */
			perror("fdopen failure");
			return(-1);
		    }
		    setbuf(ttout,NULL);
		    ckstrncpy(ttnmsv,ttname,DEVNAMLEN);
		    xlocal = *lcl = 1;	/* It's local */
		    netconn = 1;	/* Call it a network connection */
		    ttmdm = modem;	/* Remember network type */
		    ttyfd = fdin;
		    ttpipe = 1;
		    gotsigs = 0;
		    return(0);
		}
	    }
#endif /* NETCMD */
#endif /* NAMEFD */
	    x = netopen(ttname, lcl, modem); /* (see ckcnet.h) */
	    if (x > -1) {
		ckstrncpy(ttnmsv,ttname,DEVNAMLEN);
	    } else netconn = 0;
#ifdef NAMEFD
	}
#endif /* NAMEFD */

#ifdef sony_news			/* Sony NEWS */
	if (ioctl(ttyfd,TIOCKGET,&km_ext) < 0) { /* Get Kanji mode */
	    perror("ttopen error getting Kanji mode (network)");
	    debug(F111,"ttopen error getting Kanji mode","network",0);
	    km_ext = -1;		/* Make sure this stays undefined. */
	}
#endif /* sony_news */

	xlocal = *lcl = 1;		/* Network connections are local. */
	debug(F101,"ttopen net x","",x);
#ifdef COMMENT
/* Let netopen() do this */
	if (x > -1 && !x25fd)
	  x = tn_ini();			/* Initialize TELNET protocol */
#endif /* COMMENT */
	gotsigs = 0;
	return(x);
    } else {				/* Terminal device */
#endif	/* NETCONN */

#ifdef NAMEFD
/*
  This code lets you give Kermit an open file descriptor for a serial
  communication device, rather than a device name.  Kermit assumes that the
  line is already open, locked, conditioned with the right parameters, etc.
*/
	for (p = ttname; isdigit(*p); p++) ; /* Check for all-digits */
	if (*p == '\0') {
	    ttyfd = atoi(ttname);	/* Is there a way to test it's open? */
	    debug(F111,"ttopen got open fd",ttname,ttyfd);
	    ckstrncpy(ttnmsv,ttname,DEVNAMLEN); /* Remember the "name". */
	    if (ttyfd >= 0 && ttyfd < 3) /* If it's stdio... */
	      xlocal = *lcl = 0;	/* we're in remote mode */
	    else			/* otherwise */
	      xlocal = *lcl = 1;	/* local mode. */
	    netconn = 0;		/* Assume it's not a network. */
	    tvtflg = 0;			/* Might need to initialize modes. */
	    ttmdm = modem;		/* Remember modem type. */
	    fdflag = 0;			/* Stdio not redirected. */
	    ttfdflg = 1;		/* Flag we were opened this way. */
	    debug(F111,"ttopen non-net ttfdflg",ttname,ttfdflg);
	    debug(F101,"ttopen non-net ttyfd","",ttyfd);

#ifdef sony_news			/* Sony NEWS */
	    /* Get device Kanji mode */
	    if (ioctl(ttyfd,TIOCKGET,&km_ext) < 0) {
		perror("ttopen error getting Kanji mode");
		debug(F101,"ttopen error getting Kanji mode","",0);
		km_ext = -1;		/* Make sure this stays undefined. */
	    }
#endif /* sony_news */
	    gotsigs = 0;
	    return(0);			/* Return success */
	}
#endif /* NAMEFD */
#ifdef NETCONN
    }
#endif /* NETCONN */

/* Here we have to open a serial device of the given name. */

    netconn = 0;			/* So it's not a network connection */
    occt = signal(SIGINT, cctrap);	/* Set Control-C trap, save old one */
    sigint_ign = 0;

    tvtflg = 0;			/* Flag for use by ttvt(). */
				/* 0 = ttvt not called yet for this device */

    fdflag = (!isatty(0) || !isatty(1)); /* Flag for stdio redirected */
    debug(F101,"ttopen fdflag","",fdflag);

    ttmdm = modem;                      /* Make this available to other fns */
    xlocal = *lcl;                      /* Make this available to other fns */

/* Code for handling bidirectional tty lines goes here. */
/* Use specified method for turning off logins and suppressing getty. */

#ifdef ACUCNTRL
    /* Should put call to priv_on() here, but that would be very risky! */
    acucntrl("disable",ttname);         /* acucntrl() program. */
    /* and priv_off() here... */
#else
#ifdef ATT7300
    if ((attmodem & DOGETY) == 0)       /* offgetty() program. */
      attmodem |= offgetty(ttname);	/* Remember response.  */
#endif /* ATT7300 */
#endif /* ACUCNTRL */

#ifdef OPENFIRST
/*
 1985-2001: opens device first then gets lock; reason:
 Kermit usually has to run setuid or setgid in order to create a lockfile.
 If you give a SET LINE command for a device that happens to be your job's
 controlling terminal, Kermit doesn't have to create a lockfile, and in fact
 should not create one, and would fail if it tried to if it did not have the
 required privileges.  But you can't find out if two tty device names are
 equivalent until you have a file descriptor that you can give to ttyname().
 But this can cause a race condition between Kermit and [m]getty.  So see
 the [#]else part...
*/ 

/*
 In the following section, we open the tty device for read/write.
 If a modem has been specified via "set modem" prior to "set line"
 then the O_NDELAY parameter is used in the open, provided this symbol
 is defined (e.g. in fcntl.h), so that the program does not hang waiting
 for carrier (which in most cases won't be present because a connection
 has not been dialed yet).  O_NDELAY is removed later on in ttopen().  It
 would make more sense to first determine if the line is local before
 doing this, but because ttyname() requires a file descriptor, we have
 to open it first.  See do_open().

 Now open the device using the desired treatment of carrier.
 If carrier is REQUIRED, then open could hang forever, so an optional
 timer is provided.  If carrier is not required, the timer should never
 go off, and should do no harm...
*/
    ttotmo = 0;				/* Flag no timeout */
    debug(F101,"ttopen timo","",timo);
    debug(F101,"ttopen xlocal","",xlocal);
    if (timo > 0) {
	int xx;
	saval = signal(SIGALRM,timerh);	/* Timed, set up timer. */
	xx = alarm(timo);		/* Timed open() */
	debug(F101,"ttopen alarm","",xx);
	if (
#ifdef CK_POSIX_SIG
	    sigsetjmp(sjbuf,1)
#else
	    setjmp(sjbuf)
#endif /* CK_POSIX_SIG */
	    ) {
	    ttotmo = 1;			/* Flag timeout. */
	} else ttyfd = do_open(ttname);
	ttimoff();
	debug(F111,"ttopen","modem",modem);
	debug(F101,"ttopen ttyfd","",ttyfd);
	debug(F101,"ttopen alarm return","",ttotmo);
    } else {
	errno = 0;
	ttyfd = do_open(ttname);
    }
    debug(F111,"ttopen ttyfd",ttname,ttyfd);
    if (ttyfd < 0) {			/* If couldn't open, fail. */
	debug(F101,"ttopen errno","",errno);
	if (errno > 0 && !quiet)
	  perror(ttname);		/* Print message */

#ifdef ATT7300
	if (attmodem & DOGETY)		/* was getty(1m) running before us? */
	  ongetty(ttnmsv);		/* yes, restart on tty line */
	attmodem &= ~DOGETY;		/* no phone in use, getty restored */
#else
#ifdef ACUCNTRL
        /* Should put call to priv_on() here, but that would be risky! */
	acucntrl("enable",ttname);	/* acucntrl() program. */
	/* and priv_off() here... */
#endif /* ACUNTRL */
#endif /* ATT7300 */

	signal(SIGINT,occt);		/* Put old Ctrl-C trap back. */
	if (errno == EACCES) {		/* Device is protected against user */
	    debug(F110,"ttopen EACCESS",ttname,0); /* Return -4 */
	    return(-4);
	} else return(ttotmo ? -2 : -1); /* Otherwise -2 if timeout, or -1 */
    }

#ifdef QNX
    {
	extern int qnxportlock;
	x = qnxopencount();
	debug(F101,"ttopen qnxopencount","",x);
	debug(F101,"ttopen qnxportlock","",qnxportlock);
	if (x < 0 && qnxportlock) {
	    ttclos(0);
	    printf("?Can't get port open count\n");
	    printf("(Try again with SET QNX-PORT-LOCK OFF)\n");
	    return(-1);			/* Indicate device is in use */
	}
	if (x > 1) {			/* 1 == me */
	    if (qnxportlock)
	      ttclos(0);
	      return(-2);		/* Indicate device is in use */
	    else if (!quiet)
	      printf("WARNING: \"%s\" looks busy...\n",ttdev);
	}
    }
#endif /* QNX */

#ifdef Plan9
    /* take this opportunity to open the control channel */
    if (p9openttyctl(ttname) < 0)
#else
    /* Make sure it's a real tty. */
    if (!ttfdflg && !isatty(ttyfd) && strcmp(ttname,"/dev/null"))
#endif /* Plan9 */
      {
	fprintf(stderr,"%s is not a terminal device\n",ttname);
	debug(F111,"ttopen not a tty",ttname,errno);
	close(ttyfd);
	ttyfd = -1;
	wasclosed = 1;
	signal(SIGINT,occt);
	return(-1);
    }

#ifdef aegis
	/* Apollo C runtime claims that console pads are tty devices, which
	 * is reasonable, but they aren't any good for packet transfer. */
	ios_$inq_type_uid((short)ttyfd, ttyuid, st);
	if (st.all != status_$ok) {
	    fprintf(stderr, "problem getting tty object type: ");
	    error_$print(st);
	} else if (ttyuid != sio_$uid) { /* reject non-SIO lines */
	    close(ttyfd); ttyfd = -1;
	    wasclosed = 1;
	    errno = ENOTTY; perror(ttname);
	    signal(SIGINT,occt);
	    return(-1);
	}
#endif /* aegis */

    sigint_ign = (occt == SIG_IGN) ? 1 : 0;

    ckstrncpy(ttnmsv,ttname,DEVNAMLEN);	/* Keep copy of name locally. */

/* Caller wants us to figure out if line is controlling tty */

    if (*lcl < 0) {
        if (strcmp(ttname,CTTNAM) == 0) { /* "/dev/tty" always remote */
            xlocal = 0;
	    debug(F111,"ttopen ttname=CTTNAM",ttname,xlocal);
        } else if (strcmp(ttname,cttnam) == 0) {
            xlocal = 0;
	    debug(F111,"ttopen ttname=cttnam",ttname,xlocal);
	} else if (cttnam[0]) {
#ifdef BEBOX_DR7
            x = ttnmsv;			/* ttyname() is broken */
#else
            x = ttyname(ttyfd);         /* Get real name of ttname. */
#endif /* BEBOX_DR7 */
	    if (!x) x = "";
	    if (*x)
	      xlocal = ((strncmp(x,cttnam,DEVNAMLEN) == 0) ? 0 : 1);
	    else
	      xlocal = 1;
            debug(F111,"ttopen ttyname(ttyfd) xlocal",x,xlocal);
        }
    }

#ifndef NOFDZERO
/* Note, the following code was added so that Unix "idle-line" snoopers */
/* would not think Kermit was idle when it was transferring files, and */
/* maybe log people out. */
    if (xlocal == 0) {			/* Remote mode */
	if (fdflag == 0) {		/* Standard i/o is not redirected */
	    debug(F100,"ttopen setting ttyfd = 0","",0);
#ifdef LYNXOS
	    /* On Lynx OS, fd 0 is open for read only. */
	    dup2(ttyfd,0);
#endif /* LYNXOS */
	    close(ttyfd);		/* Use file descriptor 0 */
	    ttyfd = 0;
	} else {			/* Standard i/o is redirected */
	    debug(F101,"ttopen stdio redirected","",ttyfd);
	}
    }
#endif /* NOFDZERO */

/* Now check if line is locked -- if so fail, else lock for ourselves */
/* Note: After having done this, don't forget to delete the lock if you */
/* leave ttopen() with an error condition. */

    lkf = 0;                            /* Check lock */
    if (xlocal > 0) {
	int xx; int xpid;
        if ((xx = ttlock(ttname)) < 0) { /* Can't lock it. */
            debug(F111,"ttopen ttlock fails",ttname,xx);
	    /* WARNING - This close() can hang if tty is an empty socket... */
            close(ttyfd);		/* Close the device. */
	    ttyfd = -1;			/* Erase its file descriptor. */
	    wasclosed = 1;
	    signal(SIGINT,occt);	/* Put old SIGINT back. */
	    sigint_ign = (occt == SIG_IGN) ? 1 : 0;
	    if (xx == -2) {		/* If lockfile says device in use, */
#ifndef NOUUCP
		debug(F111,"ttopen reading lockfile pid",flfnam,xx);
		xpid = ttrpid(flfnam);	/* Try to read pid from lockfile */
		if (xpid > -1) {	/* If we got a pid */
                    if (!quiet)
		      printf("Locked by process %d\n",xpid); /* tell them. */
		    sprintf(lockpid,"%d",xpid);	/* Record it too */
		    debug(F110,"ttopen lockpid",lockpid,0);
		} else if (*flfnam) {
		    extern char *DIRCMD;
		    char *p = NULL;
		    int x;
		    x = (int)strlen(flfnam) + (int)strlen(DIRCMD) + 2;
		    p = malloc(x);	/* Print a directory listing. */
/*
  Note: priv_on() won't help here, because we do not pass privs along to
  to inferior processes, in this case ls.  So if the real user does not have
  directory-listing access to the lockfile directory, this will result in
  something like "not found".  That's why we try this only as a last resort.
*/
		    if (p) {		/* If we got the space... */
			ckmakmsg(p,x,DIRCMD," ",flfnam,NULL);
			zsyscmd(p);	/* Get listing. */
			if (p) {	/* free the space */
			    free(p);
			    p = NULL;
			}
		    }
		}
#endif /* NOUUCP */
		return(-5);		/* Code for device in use */
	    } else return(-3);		/* Access denied */
        } else lkf = 1;
    }
#else  /* OPENFIRST */

/*
  27 Oct 2001: New simpler code that gets the lock first and then opens the
  device, which eliminates the race condition.  The downside is you can no
  longer say "set line /dev/ttyp0" or whatever, where /dev/ttyp0 is your login
  terminal, without trying to create a lockfile, which fails if C-Kermit lacks
  privs, and if it succeeds, it has created a lockfile where it didn't create
  one before.
*/
    xlocal = *lcl;			/* Is the device my login terminal? */
    debug(F111,"ttopen xlocal","A",xlocal);
    fnam = ttname;
    if (strcmp(ttname,CTTNAM) && netconn == 0) {
	if (zfnqfp(ttname,DEVNAMLEN+1,fullname)) {
	    if ((int)strlen(fullname) > 0)
	      fnam = fullname;
	}
    }
    debug(F110,"ttopen fnam",fnam,0);
    if (xlocal < 0) {
	xlocal = (strcmp(fnam,CTTNAM) != 0);
    }
    debug(F111,"ttopen xlocal","B",xlocal);

    lkf = 0;                            /* No lock yet */
    if (xlocal > 0) {			/* If not... */
	int xx; int xpid;
	xx = ttlock(fnam);		/* Try to lock it. */
	debug(F101,"ttopen ttlock","",xx);
        if (xx < 0) {			/* Can't lock it. */
            debug(F111,"ttopen ttlock fails",fnam,xx);
	    if (xx == -2) {		/* If lockfile says device in use, */
#ifndef NOUUCP
		debug(F111,"ttopen reading lockfile pid",flfnam,xx);
		xpid = ttrpid(flfnam);	/* Try to read pid from lockfile */
		if (xpid > -1) {	/* If we got a pid */
                    if (!quiet)
		      printf("Locked by process %d\n",xpid); /* tell them. */
		    ckstrncpy(lockpid,ckitoa(xpid),16);
		    debug(F110,"ttopen lockpid",lockpid,0);
#ifndef NOPUSH
		} else if (flfnam[0] && !nopush) {
		    extern char *DIRCMD;
		    char *p = NULL;
		    int x;
		    x = (int)strlen(flfnam) + (int)strlen(DIRCMD) + 2;
		    p = malloc(x);	/* Print a directory listing. */
/*
  Note: priv_on() won't help here, because we do not pass privs along to
  to inferior processes, in this case ls.  So if the real user does not have
  directory-listing access to the lockfile directory, this will result in
  something like "not found".  That's why we try this only as a last resort.
*/
		    if (p) {		/* If we got the space... */
			ckmakmsg(p,x,DIRCMD," ",flfnam,NULL);
			zsyscmd(p);	/* Get listing. */
			if (p) {	/* free the space */
			    free(p);
			    p = NULL;
			}
		    }
#endif /* NOPUSH */
		}
#endif /* NOUUCP */
		return(-5);		/* Code for device in use */
	    } else return(-3);		/* Access denied */
        } else lkf = 1;
    }
    /* Have lock -- now it's safe to open the device */

    debug(F101,"ttopen lkf","",lkf);
    debug(F101,"ttopen timo","",timo);

    ttotmo = 0;				/* Flag no timeout */
    if (timo > 0) {
	int xx;
	saval = signal(SIGALRM,timerh);	/* Timed, set up timer. */
	xx = alarm(timo);		/* Timed open() */
	debug(F101,"ttopen alarm","",xx);
	if (
#ifdef CK_POSIX_SIG
	    sigsetjmp(sjbuf,1)
#else
	    setjmp(sjbuf)
#endif /* CK_POSIX_SIG */
	    ) {
	    ttotmo = 1;			/* Flag timeout. */
	} else {
	    ttyfd = do_open(fnam);
	}
	ttimoff();
	debug(F111,"ttopen timed ttyfd",fnam,ttyfd);
    } else {
	errno = 0;
	ttyfd = do_open(fnam);
	debug(F111,"ttopen untimed ttyfd",fnam,ttyfd);
    }
    if (ttyfd < 0) {			/* If couldn't open, fail. */
	debug(F111,"ttopen errno",fnam,errno);
	debug(F111,"ttopen xlocal","C",xlocal);
	if (xlocal == 0) {
	    debug(F100,"ttopen substituting 0","",0);
	    ttyfd = 0;
	} else {
	    if (errno > 0 && !quiet) {
	        debug(F111,"ttopen perror",fnam,errno);
		perror(fnam);		/* Print message */
	    }
	    if (ttunlck())                  /* Release the lock file */
	      fprintf(stderr,"Warning, problem releasing lock\r\n");
	}
    }

    if (ttyfd < 0) {			/* ttyfd is still < 0? */
#ifdef ATT7300
	if (attmodem & DOGETY)		/* was getty(1m) running before us? */
	  ongetty(ttnmsv);		/* yes, restart on tty line */
	attmodem &= ~DOGETY;		/* no phone in use, getty restored */
#else
#ifdef ACUCNTRL
        /* Should put call to priv_on() here, but that would be risky! */
	acucntrl("enable",fnam);	/* acucntrl() program. */
	/* and priv_off() here... */
#endif /* ACUNTRL */
#endif /* ATT7300 */

	signal(SIGINT,occt);		/* Put old Ctrl-C trap back. */
	if (errno == EACCES) {		/* Device is protected against user */
	    debug(F110,"ttopen EACCESS",fnam,0); /* Return -4 */
	    return(-4);
	} else return(ttotmo ? -2 : -1); /* Otherwise -2 if timeout, or -1 */
    }

/* Make sure it's a real tty. */

#ifdef Plan9
    /* take this opportunity to open the control channel */
    if (p9openttyctl(fnam) < 0)       
#else
      if (!ttfdflg && !isatty(ttyfd) && strcmp(fnam,"/dev/null"))
#endif /* Plan9 */
	{
	    fprintf(stderr,"%s is not a terminal device\n",fnam);
	    debug(F111,"ttopen not a tty",fnam,errno);
	    if (ttunlck())		/* Release the lock file */
	      fprintf(stderr,"Warning, problem releasing lock\r\n");
	    close(ttyfd);
	    ttyfd = -1;
	    wasclosed = 1;
	    signal(SIGINT,occt);
	    return(-1);
	}

#ifdef aegis
    /*
      Apollo C runtime claims that console pads are tty devices, which
      is reasonable, but they aren't any good for packet transfer.
    */
    ios_$inq_type_uid((short)ttyfd, ttyuid, st);
    if (st.all != status_$ok) {
	fprintf(stderr, "problem getting tty object type: ");
	error_$print(st);
    } else if (ttyuid != sio_$uid) {	/* Reject non-SIO lines */
	close(ttyfd); ttyfd = -1;
	wasclosed = 1;
	errno = ENOTTY; perror(fnam);
	signal(SIGINT,occt);
	return(-1);
    }
#endif /* aegis */

    sigint_ign = (occt == SIG_IGN) ? 1 : 0;

    ckstrncpy(ttnmsv,ttname,DEVNAMLEN);	/* Keep copy of name locally. */

/* Caller wants us to figure out if line is controlling tty */

    if (*lcl < 0) {
	char * s;
        if (strcmp(fnam,CTTNAM) == 0) { /* "/dev/tty" always remote */
            xlocal = 0;
	    debug(F111,"ttopen fnam=CTTNAM",fnam,xlocal);
        } else if (strcmp(fnam,cttnam) == 0) {
            xlocal = 0;
	    debug(F111,"ttopen fnam=cttnam",fnam,xlocal);
	} else if (cttnam[0]) {
#ifdef BEBOX_DR7
            s = ttnmsv;			/* ttyname() is broken */
#else
            s = ttyname(ttyfd);         /* Get real name of ttname. */
#endif /* BEBOX_DR7 */
	    if (!s) s = "";
	    if (*s)
	      xlocal = ((strncmp(s,cttnam,DEVNAMLEN) == 0) ? 0 : 1);
	    else
	      xlocal = 1;
            debug(F111,"ttopen ttyname(ttyfd) xlocal",s,xlocal);
        }
    }

#ifndef NOFDZERO
/* Note, the following code was added so that Unix "idle-line" snoopers */
/* would not think Kermit was idle when it was transferring files, and */
/* maybe log people out. */
    if (xlocal == 0) {			/* Remote mode */
	if (fdflag == 0) {		/* Standard i/o is not redirected */
	    debug(F100,"ttopen setting ttyfd = 0","",0);
#ifdef LYNXOS
	    /* On Lynx OS, fd 0 is open for read only. */
	    dup2(ttyfd,0);
#endif /* LYNXOS */
	    close(ttyfd);		/* Use file descriptor 0 */
	    ttyfd = 0;
	} else {			/* Standard i/o is redirected */
	    debug(F101,"ttopen stdio redirected","",ttyfd);
	}
    }
#endif /* NOFDZERO */
#endif /* OPENFIRST */

/* Got the line, now set the desired value for local. */

    if (*lcl != 0) *lcl = xlocal;

/* Some special stuff for v7... */

#ifdef  V7
#ifndef MINIX
    if (kmem[TTY] < 0) {		/*  If open, then skip this.  */
	qaddr[TTY] = initrawq(ttyfd);   /* Init the queue. */
	if ((kmem[TTY] = open("/dev/kmem", 0)) < 0) {
	    fprintf(stderr, "Can't read /dev/kmem in ttopen.\n");
	    perror("/dev/kmem");
	    exit(1);
	}
    }
#endif /* !MINIX */
#endif /* V7 */

/* No failure returns after this point */

#ifdef ultrix
    ioctl(ttyfd, TIOCMODEM, &temp);
#ifdef TIOCSINUSE
    if (xlocal && ioctl(ttyfd, TIOCSINUSE, NULL) < 0) {
	if (!quiet)
	  perror(fnam);
    }
#endif /* TIOCSINUSE */
#endif /* ultrix */

/* Get tty device settings  */

#ifdef BSD44ORPOSIX			/* POSIX */
    tcgetattr(ttyfd,&ttold);
    debug(F101,"ttopen tcgetattr ttold.c_lflag","",ttold.c_lflag);
    tcgetattr(ttyfd,&ttraw);
    debug(F101,"ttopen tcgetattr ttraw.c_lflag","",ttraw.c_lflag);
    tcgetattr(ttyfd,&tttvt);
    debug(F101,"ttopen tcgetattr tttvt.c_lflag","",tttvt.c_lflag);
#else					/* BSD, V7, and all others */
#ifdef ATTSV				/* AT&T UNIX */
    ioctl(ttyfd,TCGETA,&ttold);
    debug(F101,"ttopen ioctl TCGETA ttold.c_lflag","",ttold.c_lflag);
    ioctl(ttyfd,TCGETA,&ttraw);
    ioctl(ttyfd,TCGETA,&tttvt);
#else
#ifdef BELLV10
    ioctl(ttyfd,TIOCGETP,&ttold);
    debug(F101,"ttopen BELLV10 ttold.sg_flags","",ttold.sg_flags);
    ioctl(ttyfd,TIOCGDEV,&tdold);
    debug(F101,"ttopen BELLV10 tdold.flags","",tdold.flags);
#else
    gtty(ttyfd,&ttold);
    debug(F101,"ttopen gtty ttold.sg_flags","",ttold.sg_flags);
#endif /* BELLV10 */

#ifdef sony_news			/* Sony NEWS */
    if (ioctl(ttyfd,TIOCKGET,&km_ext) < 0) { /* Get console Kanji mode */
	perror("ttopen error getting Kanji mode");
	debug(F101,"ttopen error getting Kanji mode","",0);
	km_ext = -1;			/* Make sure this stays undefined. */
    }
#endif /* sony_news */

#ifdef TIOCGETC
    debug(F100,"ttopen TIOCGETC","",0);
    tcharf = 0;				/* In remote mode, also get */
    if (xlocal == 0) {			/* special characters */
	if (ioctl(ttyfd,TIOCGETC,&tchold) < 0) {
	    debug(F100,"ttopen TIOCGETC failed","",0);
	} else {
	    tcharf = 1;			/* It worked. */
	    ioctl(ttyfd,TIOCGETC,&tchnoi); /* Get another copy */
	    debug(F100,"ttopen TIOCGETC ok","",0);
	}
    }
#else
    debug(F100,"ttopen TIOCGETC not defined","",0);
#endif /* TIOCGETC */

#ifdef TIOCGLTC
    debug(F100,"ttopen TIOCGLTC","",0);
    ltcharf = 0;			/* In remote mode, also get */
    if (xlocal == 0) {			/* local special characters */
	if (ioctl(ttyfd,TIOCGLTC,&ltchold) < 0) {
	    debug(F100,"ttopen TIOCGLTC failed","",0);
	} else {
	    ltcharf = 1;		/* It worked. */
	    ioctl(ttyfd,TIOCGLTC,&ltchnoi); /* Get another copy */
	    debug(F100,"ttopen TIOCGLTC ok","",0);
	}
    }
#else
    debug(F100,"ttopen TIOCGLTC not defined","",0);
#endif /* TIOCGLTC */

#ifdef TIOCLGET
    debug(F100,"ttopen TIOCLGET","",0);
    lmodef = 0;
    if (ioctl(ttyfd,TIOCLGET,&lmode) < 0) {
	debug(F100,"ttopen TIOCLGET failed","",0);
    } else {
	lmodef = 1;
	debug(F100,"ttopen TIOCLGET ok","",0);
    }
#endif /* TIOCLGET */

#ifdef BELLV10
    ioctl(ttyfd,TIOCGETP,&ttraw);
    ioctl(ttyfd,TIOCGETP,&tttvt);
#else
    gtty(ttyfd,&ttraw);                 /* And a copy of it for packets*/
    gtty(ttyfd,&tttvt);                 /* And one for virtual tty service */
#endif /* BELLV10 */

#endif /* ATTSV */
#endif /* BSD44ORPOSIX */

/* Section for changing line discipline.  It's restored in ttres(). */

#ifdef AIXRS
#ifndef AIX41
    { union txname ld_name; int ld_idx = 0;
      ttld = 0;
        do {
  	  ld_name.tx_which = ld_idx++;
  	  ioctl(ttyfd, TXGETCD, &ld_name);
	  if (!strncmp(ld_name.tx_name, "rts", 3))
  	    ttld |= 1;
        } while (*ld_name.tx_name);
        debug(F101,"AIX line discipline","",ttld);
      }
#endif /* AIX41 */
#endif /* AIXRS */

#ifdef BSD41
/* For 4.1BSD only, force "old" tty driver, new one botches TANDEM. */
    { int k;
      ioctl(ttyfd, TIOCGETD, &ttld);	/* Get and save line discipline */
      debug(F101,"4.1bsd line discipline","",ttld);
      k = OTTYDISC;			/* Switch to "old" discipline */
      k = ioctl(ttyfd, TIOCSETD, &k);
      debug(F101,"4.1bsd tiocsetd","",k);
    }
#endif /* BSD41 */

#ifdef aegis
    /* This was previously done before the last two TCGETA or gtty above,
     * in both the ATTSV and not-ATTSV case.  If it is not okay to have only
     * one copy if it here instead, give us a shout!
     */
    sio_$control((short)ttyfd, sio_$raw_nl, false, st);
    if (xlocal) {       /* ignore breaks from local line */
        sio_$control((short)ttyfd, sio_$int_enable, false, st);
        sio_$control((short)ttyfd, sio_$quit_enable, false, st);
    }
#endif /* aegis */

#ifdef VXVE
    ttraw.c_line = 0;                   /* STTY line 0 for VX/VE */
    tttvt.c_line = 0;                   /* STTY line 0 for VX/VE */
    ioctl(ttyfd,TCSETA,&ttraw);
#endif /* vxve */

/* If O_NDELAY was used during open(), then remove it now. */

#ifdef O_NDELAY
    debug(F100,"ttopen O_NDELAY","",0);
    if (xlocal > 0) {
      if (fcntl(ttyfd, F_GETFL, 0) & O_NDELAY) {
	debug(F100,"ttopen fcntl O_NDELAY","",0);
#ifndef aegis
	if (fcntl(ttyfd,F_SETFL, fcntl(ttyfd, F_GETFL, 0) & ~O_NDELAY) < 0) {
	    debug(F100,"ttopen fcntl failure to unset O_NDELAY","",0);
	    perror("Can't unset O_NDELAY");
	}
#endif /* aegis */
	/* Some systems, notably Xenix (don't know how common this is in
	 * other systems), need special treatment to get rid of the O_NDELAY
	 * behaviour on read() with respect to carrier presence (i.e. read()
	 * returning 0 when carrier absent), even though the above fcntl()
	 * is enough to make read() wait for input when carrier is present.
	 * This magic, in turn, requires CLOCAL for working when the carrier
	 * is absent. But if xlocal == 0, presumably you already have CLOCAL
	 * or you have a carrier, otherwise you wouldn't be running this.
	 */
	debug(F101,"ttopen xlocal","",xlocal);
#ifdef ATTSV
#ifdef BSD44ORPOSIX
#ifdef COMMENT				/* 12 Aug 1997 */
#ifdef __bsdi__
	if (xlocal)
	  ttraw.c_cflag |= CLOCAL;
#else
#ifdef __FreeBSD__
	if (xlocal)
	  ttraw.c_cflag |= CLOCAL;
#endif /* __FreeBSD__ */
#endif /* __bsdi__ */
#else /* Not COMMENT */
#ifdef CLOCAL
	if (xlocal)			/* Unset this if it's defined. */
	  ttraw.c_cflag |= CLOCAL;
#endif /* CLOCAL */
#endif /* COMMENT */
	debug(F101,"ttopen BSD44ORPOSIX calling tcsetattr","",TCSADRAIN);
	if (tcsetattr(ttyfd, TCSADRAIN, &ttraw) < 0) {
	    debug(F100,"ttopen POSIX tcseattr fails","",0);
	    perror("tcsetattr");
	}
#else /* !BSD44ORPOSIX */
	if (xlocal) {
	    ttraw.c_cflag |= CLOCAL;
	    debug(F100,"ttopen calling ioctl(TCSETA)","",0);
	    errno = 0;
	    if (ioctl(ttyfd, TCSETA, &ttraw) < 0) {
                debug(F101,"ttopen ioctl(TCSETA) fails","",errno);
                perror("ioctl(TCSETA)");
            }
	}
#endif /* BSD44ORPOSIX */
#endif /* ATTSV */
#ifndef NOCOTFMC /* = NO Close(Open()) To Force Mode Change */
/* Reportedly lets uugetty grab the device in SCO UNIX 3.2 / XENIX 2.3 */
	debug(F100,"ttopen executing close/open","",0);
	close( priv_opn(fnam, O_RDWR) ); /* Magic to force change. */
#endif /* NOCOTFMC */
      }
    }
#endif /* O_NDELAY */

/* Instruct the system how to treat the carrier, and set a few other tty
 * parameters.
 *
 * This also undoes the temporary setting of CLOCAL that may have been done
 * for the close(open()) above (except in Xenix).  Also throw in ~ECHO, to
 * prevent the other end of the line from sitting there talking to itself,
 * producing garbage when the user performs a connect.
 *
 * SCO Xenix unfortunately seems to ignore the actual state of CLOCAL.
 * Now it thinks CLOCAL is always on. It seems the only real solution for
 * Xenix is to switch between the lower and upper case device names.
 *
 * This section may at some future time expand into setting a complete
 * collection of tty parameters, or call a function shared with ttpkt()/
 * ttvt() that does so.  On the other hand, the initial parameters are not
 * that important, since ttpkt() or ttvt() should always fix that before
 * any communication is done.  Well, we'll see...
 */
    if (xlocal) {
    	curcarr = -2;
	debug(F100,"ttopen calling carrctl","",0);
	carrctl(&ttraw, ttcarr == CAR_ON);
	debug(F100,"ttopen carrctl ok","",0);

#ifdef COHERENT
#define SVORPOSIX
#endif /* COHERENT */

#ifdef SVORPOSIX
	ttraw.c_lflag &= ~ECHO;
	ttold.c_lflag &= ~ECHO;
#ifdef BSD44ORPOSIX
	y = tcsetattr(ttyfd, TCSADRAIN, &ttraw);
	debug(F101,"ttopen tcsetattr","",y);
#else
	y = ioctl(ttyfd, TCSETA, &ttraw);
	debug(F100,"ttopen ioctl","",y);
#endif /* BSD44ORPOSIX */

#else /* BSD, etc */
	ttraw.sg_flags &= ~ECHO;
	ttold.sg_flags &= ~ECHO;
#ifdef BELLV10
	y = ioctl(ttyfd,TIOCSETP,&ttraw);
	debug(F100,"ttopen ioctl","",y);
#else
	y = stty(ttyfd,&ttraw);
	debug(F100,"ttopen stty","",y);
#endif /* BELLV10 */
#endif /* SVORPOSIX */

#ifdef COHERENT
#undef SVORPOSIX
#endif /* COHERENT */

	/* ttflui(); */  /*  This fails for some reason.  */
    }

    /* Get current speed */

#ifndef BEBOX
    ttspeed = ttgspd();
#else
    ttspeed = 19200;
#endif /* !BEBOX */
    debug(F101,"ttopen ttspeed","",ttspeed);

    /* Done, make entries in debug log, restore Ctrl-C trap, and return. */

    debug(F101,"ttopen ttyfd","",ttyfd);
    debug(F101,"ttopen *lcl","",*lcl);
    debug(F111,"ttopen lock file",flfnam,lkf);
    signal(SIGINT,occt);
    sigint_ign = (occt == SIG_IGN) ? 1 : 0;
    gotsigs = 0;
    return(0);
}


/*  D O _ O P E N  --  Do the right kind of open() call for the tty. */

int
do_open(ttname) char *ttname; {
    int flags;

#ifdef QNX6
    /* O_NONBLOCK on /dev/tty makes open() fail */
    return(priv_opn(ttname, O_RDWR |
		    (
		     ((int)strcmp(ttname,"/dev/tty") == 0) ?
		     0 :
		     (ttcarr != CAR_ON) ? O_NONBLOCK : 0)
		    )
	   ); 
#else  /* !QNX6 */

#ifndef	O_NDELAY			/* O_NDELAY not defined */
    return(priv_opn(ttname,2));
#else					/* O_NDELAY defined */

#ifdef ATT7300
/*
 Open comms line without waiting for carrier so initial call does not hang
 because state of "modem" is likely unknown at the initial call  -jrd.
 If this is needed for the getty stuff to work, and the open would not work
 without O_NDELAY when getty is still on, then this special case is ok.
 Otherwise, get rid of it. -ske
*/
    return(priv_opn(ttname, O_RDWR | O_NDELAY));

#else	/* !ATT7300 */

    /* Normal case. Use O_NDELAY according to SET CARRIER. See ttscarr(). */
    flags = O_RDWR;
    debug(F101,"do_open xlocal","",xlocal);
    debug(F111,"do_open flags A",ttname,flags);
    if (xlocal && (ttcarr != CAR_ON))
      flags |= O_NDELAY;
    debug(F111,"do_open flags B",ttname,flags);
    return(priv_opn(ttname, flags));
#endif /* !ATT7300 */
#endif /* O_NDELAY */
#endif /* QNX6 */
}

/*  T T C L O S  --  Close the TTY, releasing any lock.  */

static int ttc_state = 0;		/* ttclose() state */
static char * ttc_nam[] = { "setup", "hangup", "reset", "close" };

int
ttclos(foo) int foo; {			/* Arg req'd for signal() prototype */
    int xx, x = 0;
    extern int exithangup;

    debug(F101,"ttclos ttyfd","",ttyfd);
    debug(F101,"ttclos netconn","",netconn);
    debug(F101,"ttclos xlocal","",xlocal);
#ifdef NOFDZERO
    debug(F100,"ttclos NOFDZERO","",0);
#endif /* NOFDZERO */

#ifdef COMMENT
#ifdef TTLEBUF
    le_init();				/* No need for any of this */
#endif /* TTLEBUF */
#endif /* COMMENT */

    if (ttyfd < 0)			/* Wasn't open. */
      return(0);

    if (ttfdflg)			/* If we inherited ttyfd from */
      return(0);			/* another process, don't close it. */

    tvtflg = 0;				/* (some day get rid of this...) */
    gotsigs = 0;

#ifdef IKSD
    if (inserver) {
#ifdef TNCODE
          tn_push();                    /* Place any waiting data into input*/
          tn_sopt(DO,TELOPT_LOGOUT);    /* Send LOGOUT option before close */
          TELOPT_UNANSWERED_DO(TELOPT_LOGOUT) = 1;
          tn_reset();                   /* The Reset Telnet Option table.  */
#endif /* TNCODE */
#ifdef CK_SSL
	  if (ssl_active_flag) {
	      if (ssl_debug_flag)
		BIO_printf(bio_err,"calling SSL_shutdown(ssl)\n");
	      SSL_shutdown(ssl_con);
	      SSL_free(ssl_con);
	      ssl_con = NULL;
	      ssl_active_flag = 0;
	  }
	  if (tls_active_flag) {
	      if (ssl_debug_flag)
		BIO_printf(bio_err,"calling SSL_shutdown(tls)\n");
	      SSL_shutdown(tls_con);
	      SSL_free(tls_con);
	      tls_con = NULL;
	      tls_active_flag = 0;
	  }
#endif /* CK_SSL */
    }
#endif /* IKSD */
#ifdef NETCMD
    if (ttpipe) {			/* We've been using a pipe */
	/* ttpipe = 0; */
	if (ttpid > 0) {
	    int wstat;
	    int statusp;
	    close(fdin);		/* Close these. */
	    close(fdout);
	    fdin = fdout = -1;
	    kill(ttpid,1);		/* Kill fork with SIGHUP */
	    while (1) {
		wstat = wait(&statusp);
		if (wstat == ttpid || wstat == -1)
		  break;
		pexitstat = (statusp & 0xff) ? statusp : statusp >> 8;
	    }
	    ttpid = 0;
	}
	netconn = 0;
	wasclosed = 1;
	ttyfd = -1;
	return(0);
    }
#endif /* NETCMD */
#ifdef NETPTY
    if (ttpty) {
#ifndef NODOPTY
        end_pty();
#endif /* NODOPTY */
        close(ttyfd);
	netconn = 0;
	wasclosed = 1;
        ttpty = 0;
        ttyfd = -1;
        return(0);
    }
#endif /* NETPTY */

#ifdef	NETCONN
    if (netconn) {			/* If it's a network connection. */
	debug(F100,"ttclos closing net","",0);
	netclos();			/* Let the network module close it. */
	netconn = 0;			/* No more network connection. */
	debug(F101,"ttclos ttyfd after netclos","",ttyfd); /* Should be -1 */
	return(0);
    }
#endif	/* NETCONN */

    if (xlocal) {			/* We're closing a SET LINE device */
#ifdef FT21				/* Fortune 2.1-specific items ... */
	ioctl(ttyfd,TIOCHPCL, NULL);
#endif /* FT21 */
#ifdef ultrix				/* Ultrix-specific items ... */
#ifdef TIOCSINUSE
	/* Unset the INUSE flag that we set in ttopen() */
	ioctl(ttyfd, TIOCSINUSE, NULL);
#endif /* TIOCSINUSE */
	ioctl(ttyfd, TIOCNMODEM, &x);
#ifdef COMMENT
	/* What was this? */
	ioctl(ttyfd, TIOCNCAR, NULL);
#endif /* COMMENT */
#endif /* ultrix */
    }

    /* This is to prevent us from sticking in tthang() or close(). */

#ifdef O_NDELAY
#ifndef aegis
    if (ttyfd > 0) {			/* But skip it on stdin. */
	debug(F100,"ttclos setting O_NDELAY","",0);
	x = fcntl(ttyfd,F_SETFL,fcntl(ttyfd,F_GETFL, 0)|O_NDELAY);
#ifdef DEBUG
	if (deblog && x == -1) {
	    perror("Warning - Can't set O_NDELAY");
	    debug(F101,"ttclos fcntl failure to set O_NDELAY","",x);
	}
#endif /* DEBUG */
    }
#endif /* aegis */
#endif /* O_NDELAY */

    x = 0;
    ttc_state = 0;
    if (xlocal
#ifdef NOFDZERO
	|| ttyfd > 0
#endif /* NOFDZERO */
	) {
	saval = signal(SIGALRM,xtimerh); /* Enable timer interrupt. */
	xx = alarm(8);			/* Allow 8 seconds. */
	debug(F101,"ttclos alarm","",xx);
	if (
#ifdef CK_POSIX_SIG
	    sigsetjmp(sjbuf,1)
#else
	    setjmp(sjbuf)
#endif /* CK_POSIX_SIG */
	    ) {				/* Timer went off? */
	    x = -1;
#ifdef DEBUG
	    debug(F111,"ttclos ALARM TRAP errno",ckitoa(ttc_state),errno);
	    printf("ttclos() timeout: %s\n", ttc_nam[ttc_state]);
#endif /* DEBUG */
	}
	/* Hang up the device (drop DTR) */

	errno = 0;
	debug(F111,"ttclos A",ckitoa(x),ttc_state);
	if (ttc_state < 1) {
	    ttc_state = 1;
	    debug(F101,"ttclos exithangup","",exithangup);
	    if (exithangup) {
		alarm(8);		/* Re-arm the timer */
		debug(F101,"ttclos calling tthang()","",x);
		x = tthang();		/* Hang up first, then... */
		debug(F101,"ttclos tthang()","",x);
	    }
#ifndef CK_NOHUPCL
/*
  Oct 2006 - Leave DTR on if SET EXIT HANGUP OFF.
  Suggested by Soewono Effendi.
*/
#ifdef HUPCL
	    else {
		ttold.c_cflag &= ~HUPCL; /* Let's see how this travels */
#ifdef BSD44ORPOSIX
		tcsetattr(ttyfd,TCSANOW,&ttold);
#else /* !BSD44ORPOSIX */
#ifdef ATTSV
		ioctl(ttyfd,TCSETAW,&ttold);		
#else  /* !ATTSV */
		stty(ttyfd,&ttold);
#endif	/* ATTSV */
#endif	/* BSD44ORPOSIX */
	    }
#endif	/* HUPCL */
#endif	/* CK_NOHUPCL */
	}
	/* Put back device modes as we found them */

	errno = 0;
	debug(F111,"ttclos B",ckitoa(x),ttc_state);
	if (ttc_state < 2) {
	    ttc_state = 2;
	    /* Don't try to mess with tty modes if tthang failed() */
	    /* since it probably won't work. */
	    if (x > -1) {
		debug(F101,"ttclos calling ttres()","",x);
		signal(SIGALRM,xtimerh); /* Re-enable the alarm. */
		alarm(8);		/* Re-arm the timer */
		x = ttres();		/* Reset device modes. */
		debug(F101,"ttclos ttres()","",x);
		alarm(0);
	    }
	}
	/* Close the device */

	errno = 0;
	debug(F101,"ttclos C","",ttc_state);
	if (ttc_state < 3) {
	    ttc_state = 3;
	    errno = 0;
	    debug(F101,"ttclos calling close","",x);
	    signal(SIGALRM,xtimerh);	/* Re-enable alarm. */
	    alarm(8);			/* Re-arm the timer */
	    x = close(ttyfd);		/* Close the device. */
	    debug(F101,"ttclos close()","",x);
	    if (x > -1)
	      ttc_state = 3;
	}
	debug(F101,"ttclos D","",ttc_state);
	ttimoff();			/* Turn off timer. */
	if (x < 0) {
	    printf("?WARNING - close failed: %s\n",ttnmsv);
#ifdef DEBUG
	    if (deblog) {
		printf("errno = %d\n", errno);
		debug(F101,"ttclos failed","",errno);
	    }
#endif /* DEBUG */
	}
	/* Unlock after closing but before any getty mumbo jumbo */

	debug(F100,"ttclos about to call ttunlck","",0);
        if (ttunlck())                  /* Release uucp-style lock */
	  fprintf(stderr,"Warning, problem releasing lock\r\n");
    }

/* For bidirectional lines, restore getty if it was there before. */

#ifdef ACUCNTRL				/* 4.3BSD acucntrl() method. */
    if (xlocal) {
	debug(F100,"ttclos ACUCNTRL","",0);
	acucntrl("enable",ttnmsv);	/* Enable getty on the device. */
    }
#else
#ifdef ATT7300				/* ATT UNIX PC (3B1, 7300) method. */
    if (xlocal) {
	debug(F100,"ttclos ATT7300 ongetty","",0);
	if (attmodem & DOGETY)		/* Was getty(1m) running before us? */
	  ongetty(ttnmsv);		/* Yes, restart getty on tty line */
	attmodem &= ~DOGETY;		/* No phone in use, getty restored */
    }
#endif /* ATT7300 */
#endif /* System-dependent getty-restoring methods */

#ifdef sony_news
    km_ext = -1;			/* Invalidate device's Kanji-mode */
#endif /* sony_news */

    ttyfd = -1;                         /* Invalidate the file descriptor. */
    wasclosed = 1;
    debug(F100,"ttclos done","",0);
    return(0);
}

/*  T T H A N G  --  Hangup phone line or network connection.  */
/*
  Returns:
  0 if it does nothing.
  1 if it believes that it hung up successfully.
 -1 if it believes that the hangup attempt failed.
*/

#define HUPTIME 500			/* Milliseconds for hangup */

#ifdef COMMENT
/* The following didn't work but TIOCSDTR does work */
#ifdef UNIXWARE
/* Define HUP_POSIX to force non-POSIX builds to use the POSIX hangup method */
#ifndef POSIX				/* Such as Unixware 1.x, 2.x */
#ifndef HUP_POSIX
#define HUP_POSIX
#endif /* HUP_POSIX */
#endif /* POSIX */
#endif /* UNIXWARE */
#endif /* COMMENT */

#ifndef USE_TIOCSDTR
#ifdef __NetBSD__
/* Because the POSIX method (set output speed to 0) doesn't work in NetBSD */
#ifdef TIOCSDTR
#ifdef TIOCCDTR
#define USE_TIOCSDTR
#endif /* TIOCCDTR */
#endif /* TIOCSDTR */
#endif /* __NetBSD__ */
#endif /* USE_TIOCSDTR */

#ifndef HUP_CLOSE_POSIX
#ifdef OU8
#define HUP_CLOSE_POSIX
#else
#ifdef CK_SCOV5
#define HUP_CLOSE_POSIX
#endif /* CK_SCOV5 */
#endif /* OU8 */
#endif /* HUP_CLOSE_POSIX */

#ifdef NO_HUP_CLOSE_POSIX
#ifdef HUP_CLOSE_POSIX
#undef HUP_CLOSE_POSIX
#endif /* HUP_CLOSE_POSIX */
#endif /* NO_HUP_CLOSE_POSIX */

int
tthang() {
#ifdef NOLOCAL
    return(0);
#else
    int x = 0;				/* Sometimes used as return code. */
#ifndef POSIX
    int z;				/* worker */
#endif /* POSIX */

#ifdef COHERENT
#define SVORPOSIX
#endif /* COHERENT */

#ifdef SVORPOSIX			/* AT&T, POSIX, HPUX declarations. */
    int spdsav;				/* for saving speed */
#ifdef HUP_POSIX
    int spdsavi;
#else
#ifdef BSD44ORPOSIX
    int spdsavi;
#endif /* BSD44ORPOSIX */
#endif /* HUP_POSIX */
#ifdef HPUX
/*
  Early versions of HP-UX omitted the mflag typedef.  If you get complaints
  about it, just change it to long (or better still, unsigned long).
*/
    mflag
      dtr_down = 00000000000,
      modem_rtn,
      modem_sav;
    char modem_state[64];
#endif /* HPUX */
    int flags;				/* fcntl flags */
    unsigned short ttc_save;
#endif /* SVORPOSIX */

    if (ttyfd < 0) return(0);           /* Don't do this if not open  */
    if (xlocal < 1) return(0);		/* Don't do this if not local */

#ifdef NETCMD
    if (ttpipe)
      return((ttclos(0) < 0) ? -1 : 1);
#endif /* NETCMD */
#ifdef NETPTY
    if (ttpty)
      return((ttclos(0) < 0) ? -1 : 1);
#endif /* NETPTY */
#ifdef NETCONN
    if (netconn) {			/* Network connection. */
#ifdef TN_COMPORT
        if (istncomport()) {
            int rc = tnc_set_dtr_state(0);
            if (rc >= 0) {
                msleep(HUPTIME);
                rc = tnc_set_dtr_state(1);
            }
            return(rc >= 0 ? 1 : -1);
        } else
#endif /* TN_COMPORT */
	  return((netclos() < 0) ? -1 : 1); /* Just close it. */
  }
#endif /* NETCONN */

/* From here down, we handle real tty devices. */
#ifdef HUP_POSIX
/*
  e.g. for Unixware 2, where we don't have a full POSIX build, we
  still have to use POSIX-style hangup.  Thus the duplication of this
  and the next case, the only difference being we use a local termios
  struct here, since a different model is used elsewhere.

  NO LONGER USED as of C-Kermit 8.0 -- it turns out that this method,
  even though it compiles and executes without error, doesn't actually
  work (i.e. DTR does not drop), whereas the TIOCSDTR method works just fine,
*/
    {
	struct termios ttcur;
	int x;
	debug(F100,"tthang HUP_POSIX style","",0);
	x = tcgetattr(ttyfd, &ttcur);	/* Get current attributes */
	debug(F111,"tthang tcgetattr",ckitoa(errno),x);
	if (x < 0) return(-1);
	spdsav = cfgetospeed(&ttcur);	/* Get current speed */
	debug(F111,"tthang cfgetospeed",ckitoa(errno),spdsav);
	spdsavi = cfgetispeed(&ttcur);	/* Get current speed */
	debug(F111,"tthang cfgetispeed",ckitoa(errno),spdsavi);
	x = cfsetospeed(&ttcur,B0);	/* Replace by 0 */
	debug(F111,"tthang cfsetospeed",ckitoa(errno),x);
	if (x < 0) return(-1);
	x = cfsetispeed(&ttcur,B0);
	debug(F111,"tthang cfsetispeed",ckitoa(errno),x);
	if (x < 0) return(-1);
	x = tcsetattr(ttyfd,TCSADRAIN,&ttcur);
	debug(F111,"tthang tcsetattr B0",ckitoa(errno),x);
	if (x < 0) return(-1);
	msleep(HUPTIME);		/* Sleep 0.5 sec */
	x = cfsetospeed(&ttcur,spdsav); /* Restore prev speed */
	if (x < 0) return(-1);
	debug(F111,"tthang cfsetospeed prev",ckitoa(errno),x);
	x = cfsetispeed(&ttcur,spdsavi);
	debug(F111,"tthang cfsetispeed prev",ckitoa(errno),x);
	if (x < 0) return(-1);
	x = tcsetattr(ttyfd,TCSADRAIN,&ttcur);
	debug(F111,"tthang tcsetattr restore",ckitoa(errno),x);
	if (x < 0) return(-1);
	return(1);
    }
#else
#ifdef BSD44ORPOSIX
#ifdef QNX
    {
	int x;
	x = tcdropline(ttyfd,500);
	debug(F101,"tthang QNX tcdropline","",x);
	ttcur.c_cflag |= CLOCAL;
	x = tcsetattr(ttyfd,TCSADRAIN,&ttcur);
	debug(F101,"tthang QNX tcsetattr restore","",x);
	if (x < 0) {
	    debug(F101,"tthang QNX tcsetattr restore errno","",errno);
	    return(-1);
	}
	/* Fix flags - ensure O_NONBLOCK is off */

	errno = 0;
	debug(F101,"tthang QNX iniflags","",iniflags);
	if (fcntl(ttyfd, F_SETFL, iniflags) == -1) {
	    debug(F101,"tthang QNX F_SETFL errno","",errno);
	    return(-1);
	}
	return(x);
    }
#else  /* QNX */
    {
	int x;
#ifdef USE_TIOCSDTR
	debug(F100,"tthang BSD44ORPOSIX USE_TIOCSDTR","",0);
	errno = 0;
	x = ioctl(ttyfd, TIOCCDTR, NULL);
	debug(F111,"tthang BSD44ORPOSIX ioctl TIOCCDTR",ckitoa(errno),x);
	if (x < 0) return(-1);
	msleep(HUPTIME);		/* Sleep 0.5 sec */
	errno = 0;
	x = ioctl(ttyfd, TIOCSDTR, NULL);
	debug(F111,"tthang BSD44ORPOSIX ioctl TIOCSDTR",ckitoa(errno),x);
	if (x < 0) return(-1);
#else  /* USE_TIOCSDTR */

#ifdef HUP_CLOSE_POSIX
/*
  In OSR5 versions where TIOCSDTR is not defined (up to and including at
  least 5.0.6a) the POSIX APIs in the "#else" part below are available but
  don't work, and no other APIs are available that do work.  In this case
  we have to drop DTR by brute force: close and reopen the port.  This
  code actually works, but all the steps are crucial: setting CLOCAL, the
  O_NDELAY manipulations, etc.
*/
	debug(F100,"tthang HUP_CLOSE_POSIX close/open","",0);
	debug(F101,"tthang HUP_CLOSE_POSIX O_NONBLOCK","",O_NONBLOCK);
	debug(F101,"tthang HUP_CLOSE_POSIX O_NDELAY","",O_NDELAY);
	errno = 0;
	x = tcgetattr(ttyfd, &ttcur);	/* Get current attributes */
	debug(F101,"tthang HUP_CLOSE_POSIX tcgetattr","",x);
	if (x < 0) {
	    debug(F101,"tthang HUP_CLOSE_POSIX tcgetattr errno","",errno);
	    return(-1);
	}
	errno = 0;

	x = close(ttyfd);		/* Close without releasing lock */
	if (x < 0) {
	    debug(F101,"tthang HUP_CLOSE_POSIX close errno","",errno);
	    return(-1);
	}
	errno = 0;
	x = msleep(500);		/* Pause half a second */
	if (x < 0) {			/* Or if that doesn't work, 1 sec */
	    debug(F101,"tthang HUP_CLOSE_POSIX msleep errno","",errno);
	    sleep(1);
	}
	errno = 0;
	ttyfd = priv_opn(ttnmsv, (O_RDWR|O_NDELAY)); /* Reopen the device */
	debug(F111,"tthang HUP_CLOSE_POSIX reopen",ttnmsv,ttyfd);
	if (ttyfd < 0) {
	    debug(F101,"tthang HUP_CLOSE_POSIX reopen errno","",errno);
	    return(-1);
	}
	debug(F101,"tthang HUP_CLOSE_POSIX re-ttopen ttyfd","",ttyfd);

	/* Restore previous attributes */

	errno = 0;
	tvtflg = 0;
	ttcur.c_cflag |= CLOCAL;
	x = tcsetattr(ttyfd,TCSADRAIN,&ttcur);
	debug(F101,"tthang HUP_CLOSE_POSIX tcsetattr restore","",x);
	if (x < 0) {
	    debug(F101,"tthang HUP_CLOSE_POSIX tcsetattr restore errno",
		  "",errno);
	    return(-1);
	}
	/* Fix flags - ensure O_NDELAY and O_NONBLOCK are off */

	errno = 0;
        if ((x = fcntl(ttyfd, F_GETFL, 0)) == -1) {
	    debug(F101,"tthang HUP_CLOSE_POSIX F_GETFL errno","",errno);
	    return(-1);
	}
	debug(F101,"tthang HUP_CLOSE_POSIX flags","",x);
	errno = 0;
        x &= ~(O_NONBLOCK|O_NDELAY);
	debug(F101,"tthang HUP_CLOSE_POSIX flags to set","",x);
	debug(F101,"tthang HUP_CLOSE_POSIX iniflags","",iniflags);
	if (fcntl(ttyfd, F_SETFL, x) == -1) {
	    debug(F101,"tthang HUP_CLOSE_POSIX F_SETFL errno","",errno);
	    return(-1);
	}
#ifdef DEBUG
	if (deblog) {
	    if ((x = fcntl(ttyfd, F_GETFL, 0)) > -1) {
		debug(F101,"tthang HUP_CLOSE_POSIX flags","",x);
		debug(F101,"tthang HUP_CLOSE_POSIX flags & O_NONBLOCK",
		      "",x&O_NONBLOCK);
		debug(F101,"tthang HUP_CLOSE_POSIX flags & O_NDELAY",
		      "",x&O_NDELAY);
	    }
	}
#endif /* DEBUG */

#else  /* HUP_CLOSE_POSIX */
	
	/* General BSD44ORPOSIX case (Linux, BSDI, FreeBSD, etc) */

	debug(F100,"tthang BSD44ORPOSIX B0","",0);
	x = tcgetattr(ttyfd, &ttcur);	/* Get current attributes */
	debug(F111,"tthang BSD44ORPOSIX tcgetattr",ckitoa(errno),x);
	if (x < 0) return(-1);
	spdsav = cfgetospeed(&ttcur);	/* Get current speed */
	debug(F111,"tthang BSD44ORPOSIX cfgetospeed",ckitoa(errno),spdsav);
	spdsavi = cfgetispeed(&ttcur);	/* Get current speed */
	debug(F111,"tthang BSD44ORPOSIX cfgetispeed",ckitoa(errno),spdsavi);
	x = cfsetospeed(&ttcur,B0);	/* Replace by 0 */
	debug(F111,"tthang BSD44ORPOSIX cfsetospeed",ckitoa(errno),x);
	if (x < 0) return(-1);
	x = cfsetispeed(&ttcur,B0);
	debug(F111,"tthang BSD44ORPOSIX cfsetispeed",ckitoa(errno),x);
	if (x < 0) return(-1);
	/* This gets EINVAL on NetBSD 1.4.1 because of B0... */
	x = tcsetattr(ttyfd,TCSADRAIN,&ttcur);
	debug(F111,"tthang BSD44ORPOSIX tcsetattr B0",ckitoa(errno),x);
	if (x < 0) return(-1);
	msleep(HUPTIME);		/* Sleep 0.5 sec */
	debug(F101,"tthang BSD44ORPOSIX restore output speed","",spdsav);
	x = cfsetospeed(&ttcur,spdsav); /* Restore prev speed */
	debug(F111,"tthang BSD44ORPOSIX cfsetospeed prev",ckitoa(errno),x);
	if (x < 0) return(-1);
	debug(F101,"tthang BSD44ORPOSIX restore input speed","",spdsavi);
	x = cfsetispeed(&ttcur,spdsavi);
	debug(F111,"tthang BSD44ORPOSIX cfsetispeed prev",ckitoa(errno),x);
	if (x < 0) return(-1);
	ttcur.c_cflag |= CLOCAL;	/* Don't expect CD after hangup */
	x = tcsetattr(ttyfd,TCSADRAIN,&ttcur);
	debug(F111,"tthang BSD44ORPOSIX tcsetattr restore",ckitoa(errno),x);
	if (x < 0) return(-1);

#endif /* HUP_CLOSE_POSIX */
#endif /* USE_TIOCSDTR */

	return(1);
    }

#endif /* QNX */
#else /* BSD44ORPOSIX */

#ifdef aegis				/* Apollo Aegis */
    sio_$control((short)ttyfd, sio_$dtr, false, st);    /* DTR down */
    msleep(HUPTIME);					/* pause */
    sio_$control((short)ttyfd, sio_$dtr, true,  st);    /* DTR up */
    return(1);
#endif /* aegis */

#ifdef ANYBSD				/* Any BSD version. */
#ifdef TIOCCDTR				/* Except those that don't have this */
    debug(F100,"tthang BSD style","",0);
    if (ioctl(ttyfd,TIOCCDTR,0) < 0) {	/* Clear DTR. */
	debug(F101,"tthang TIOCCDTR fails","",errno);
	return(-1);
    }
    msleep(HUPTIME);			/* For about 1/2 sec */
    errno = 0;
    x = ioctl(ttyfd,TIOCSDTR,0);	/* Restore DTR */
    if (x < 0) {
	/*
	  For some reason, this tends to fail with "no such device or address"
	  but the operation still works, probably because of the close/open
	  later on.  So let's not scare the user unnecessarily here.
	*/
	debug(F101,"tthang TIOCSDTR errno","",errno); /* Log the error */
	x = 1;				/* Pretend we succeeded */
    } else if (x == 0) x = 1;		/* Success */
#ifdef COMMENT
#ifdef FT21
    ioctl(ttyfd, TIOCSAVEMODES, 0);
    ioctl(ttyfd, TIOCHPCL, 0);
    close(ttyfd);			/* Yes, must do this twice */
    if ((ttyfd = open(ttnmsv,2)) < 0)	/* on Fortune computers... */
      return(-1);			/* (but why?) */
    else x = 1;
#endif /* FT21 */
#endif /* COMMENT */
#endif /* TIOCCDTR */
    close(do_open(ttnmsv));		/* Clear i/o error condition */
    errno = 0;
#ifdef COMMENT
/* This is definitely dangerous.  Why was it here? */
    z = ttvt(ttspeed,ttflow);		/* Restore modes. */
    debug(F101,"tthang ttvt returns","",z);
    return(z < 0 ? -1 : 1);
#else
    return(x);
#endif /* COMMENT */
#endif /* ANYBSD */

#ifdef ATTSV
/* AT&T UNIX section, includes HP-UX and generic AT&T System III/V... */

#ifdef HPUX
/* Hewlett Packard allows explicit manipulation of modem signals. */

#ifdef COMMENT
/* Old way... */
    debug(F100,"tthang HP-UX style","",0);
    if (ioctl(ttyfd,MCSETAF,&dtr_down) < 0)        /* lower DTR */
      return(-1);		    	           /* oops, can't. */
    msleep(HUPTIME);			           /* Pause half a second. */
    x = 1;				           /* Set return code */
    if (ioctl(ttyfd,MCGETA,&modem_rtn) > -1) {     /* Get line status. */
	if ((modem_rtn & MDCD) != 0)      	   /* Check if CD is low. */
	  x = -1;                                  /* CD didn't drop, fail. */
    } else x = -1;

    /* Even if above calls fail, RTS & DTR should be turned back on. */
    modem_rtn = MRTS | MDTR;
    if (ioctl(ttyfd,MCSETAF,&modem_rtn) < 0) x = -1;
    return(x);
#else
/* New way, from Hellmuth Michaelis */
    debug(F100,"tthang HP-UX style, HPUXDEBUG","",0);
    if (ioctl(ttyfd,MCGETA,&modem_rtn) == -1) { /* Get current status. */
	debug(F100,"tthang HP-UX: can't get modem lines, NO HANGUP!","",0);
	return(-1);
    }
    sprintf(modem_state,"%#lx",modem_rtn);
    debug(F110,"tthang HP-UX: modem lines = ",modem_state,0);
    modem_sav = modem_rtn;		/* Save current modem signals */
    modem_rtn &= ~MDTR;			/* Turn DTR bit off */
    sprintf(modem_state,"%#lx",modem_rtn);
    debug(F110,"tthang HP-UX: DTR down = ",modem_state,0);
    if (ioctl(ttyfd,MCSETAF,&modem_rtn) < 0) { /* lower DTR */
	debug(F100,"tthang HP-UX: can't lower DTR!","",0);
	return(-1);			/* oops, can't. */
    }
    msleep(HUPTIME);			/* Pause half a second. */
    x = 1;				/* Set return code */
    if (ioctl(ttyfd,MCGETA,&modem_rtn) > -1) { /* Get line status. */
	sprintf(modem_state,"%#lx",modem_rtn);
	debug(F110,"tthang HP-UX: modem lines got = ",modem_state,0);
	if ((modem_rtn & MDCD) != 0) {	/* Check if CD is low. */
	    debug(F100,"tthang HP-UX: DCD not down","",0);
	    x = -1;			/* CD didn't drop, fail. */
	} else {
	    debug(F100,"tthang HP-UX: DCD down","",0);
	}
    } else {
	x = -1;
	debug(F100,"tthang HP-UX: can't get DCD status !","",0);
    }

    /* Even if above calls fail, DTR should be turned back on. */

    modem_sav |= MDTR;
    if (ioctl(ttyfd,MCSETAF,&modem_sav) < 0) {
	x = -1;
	debug(F100,"tthang HP-UX: can't set saved state","",0);
    } else {
	sprintf(modem_state,"%#lx",modem_sav);
	debug(F110,"tthang HP-UX: final modem lines = ",modem_state,0);
    }
    return(x);
#endif /* COMMENT */

#else /* AT&T but not HP-UX */

/* SVID for AT&T System V R3 defines ioctl's for handling modem signals. */
/* It is not known how many, if any, systems actually implement them, */
/* so we include them here in ifdef's. */

/*
  Unixware has the TIOCMxxx symbols defined, but calling ioctl() with them
  gives error 22 (invalid argument).
*/
#ifndef _IBMR2
/*
  No modem-signal twiddling for IBM RT PC or RS/6000.
  In AIX 3.1 and earlier, the ioctl() call is broken.
  This code could be activated for AIX 3.1 with PTF 2006 or later
  (e.g. AIX 3.2), but close/open does the job too, so why bother.
*/
#ifdef TIOCMBIS				/* Bit Set */
#ifdef TIOCMBIC				/* Bit Clear */
#ifdef TIOCM_DTR			/* DTR */

/* Clear DTR, sleep 300 msec, turn it back on. */
/* If any of the ioctl's return failure, go on to the next section. */

    z = TIOCM_DTR;			/* Code for DTR. */
#ifdef COMMENT
/*
  This was the cause of the troubles with the Solaris Port Monitor.
  The problem is: RTS never comes back on.  Moral: Don't do it!
  (But why doesn't it come back on?  See the TIOCMBIS call...)
*/
#ifdef TIOCM_RTS			/* Lower RTS too if symbol is known. */
    z |= TIOCM_RTS;
#endif /* TIOCM_RTS */
#endif /* COMMENT */

    debug(F101,"tthang TIOCM signal mask","",z);
    if (ioctl(ttyfd,TIOCMBIC,&z) > -1) {   /* Try to lower DTR. */
	debug(F100,"tthang TIOCMBIC ok","",0);
	msleep(HUPTIME);		   /* Pause half a second. */
	if (ioctl(ttyfd,TIOCMBIS,&z) > -1) { /* Try to turn it back on. */
	    debug(F100,"tthang TIOCMBIS ok","",0);
#ifndef CLSOPN
	    return(1);			/* Success, done. */
#endif /* CLSOPN */
	} else {			/* Couldn't raise, continue. */
	    debug(F101,"tthang TIOCMBIS errno","",errno);
	}
    } else {				/* Couldn't lower, continue. */
 	debug(F101,"tthang TIOCMBIC errno","",errno);
    }
#endif /* TIOCM_DTR */
#endif /* TIOCMBIC */
#endif /* TIOCMBIS */
#endif /* _IBMR2 */

/*
  General AT&T UNIX case, not HPUX.  The following code is highly suspect.  No
  two AT&T-based systems seem to do this the same way.  The object is simply
  to turn off DTR and then turn it back on.  SVID says the universal method
  for turning off DTR is to set the speed to zero, and this does seem to do
  the trick in all cases.  But neither SVID nor any known man pages say how to
  turn DTR back on again.  Some variants, like most Xenix implementations,
  raise DTR again when the speed is restored to a nonzero value.  Others
  require the device to be closed and opened again, but this is risky because
  getty could seize the device during the instant it is closed.
*/

/* Return code for ioctl failures... */
#ifdef ATT6300
    x = 1;				/* ATT6300 doesn't want to fail... */
#else
    x = -1;
#endif /* ATT6300 */

    debug(F100,"tthang get settings","",0);
    if (ioctl(ttyfd,TCGETA,&ttcur) < 0) /* Get current settings. */
      return(x);			/* Fail if this doesn't work. */
    if ((flags = fcntl(ttyfd,F_GETFL,0)) < 0) /* Get device flags. */
      return(x);
    ttc_save = ttcur.c_cflag;		/* Remember current speed. */
    spdsav = ttc_save & CBAUD;
    debug(F101,"tthang speed","",spdsav);

#ifdef O_NDELAY
    debug(F100,"tthang turning O_NDELAY on","",0);
    fcntl(ttyfd, F_SETFL, flags | O_NDELAY); /* Activate O_NDELAY */
#endif /* O_NDELAY */

#ifdef ATT7300 /* This is the way it is SUPPOSED to work */
    ttcur.c_cflag &= ~CBAUD;		/* Change the speed to zero.  */
#else
#ifdef RTAIX
    ttcur.c_cflag &= ~CBAUD;		/* Change the speed to zero.  */
#else          /* This way really works but may be dangerous */
#ifdef u3b2
    ttcur.c_cflag = ~(CBAUD|CLOCAL);	/* Special for AT&T 3B2s */
					/* (CLOCAL must be OFF) */
#else
#ifdef SCO3R2				/* SCO UNIX 3.2 */
/*
  This is complete nonsense, but an SCO user claimed this change made
  hanging up work.  Comments from other SCO UNIX 3.2 users would be
  appreciated.
*/
    ttcur.c_cflag = CBAUD|B0;
#else
#ifdef AIXRS				/* AIX on RS/6000 */
/*
  Can't set speed to zero on AIX 3.1 on RS/6000 64-port adapter,
  even though you can do it on the built-in port and the 8- and 16-port
  adapters.  (Untested on 128-port adapter.)
*/
    ttcur.c_cflag = CLOCAL|HUPCL|spdsav; /* Speed 0 causes EINVAL */
#else					/* None of the above */
/*
  Set everything, including the speed, to zero, except for the CLOCAL
  and HUPCL bits.
*/
    ttcur.c_cflag = CLOCAL|HUPCL;
#endif /* AIXRS */
#endif /* SCO3R2 */
#endif /* u3b2 */
#endif /* RTAIX */
#endif /* ATT7300 */

#ifdef COMMENT
    /* and if none of those work, try one of these... */
    ttcur.c_cflag = 0;
    ttcur.c_cflag = CLOCAL;
    ttcur.c_cflag &= ~(CBAUD|HUPCL);
    ttcur.c_cflag &= ~(CBAUD|CREAD);
    ttcur.c_cflag &= ~(CBAUD|CREAD|HUPCL);
    /* or other combinations */
#endif /* COMMENT */

#ifdef TCXONC
    debug(F100,"tthang TCXONC","",0);
    if (ioctl(ttyfd, TCXONC, 1) < 0) {
	debug(F101,"tthang TCXONC failed","",errno);
    }
#endif /* TCXONC */

#ifdef TIOCSTART
    debug(F100,"tthang TIOCSTART","",0);
    if (ioctl(ttyfd, TIOCSTART, 0) < 0) {
	debug(F101,"tthang TIOCSTART failed","",errno);
    }
#endif /* TIOCSTART */

    if (ioctl(ttyfd,TCSETAF,&ttcur) < 0) { /* Fail if we can't. */
	debug(F101,"tthang TCSETAF failed","",errno);
	fcntl(ttyfd, F_SETFL, flags);	/* Restore flags */
	return(-1);			/* before returning. */
    }
    msleep(300);			/* Give modem time to notice. */

#ifndef NOCOTFMC

/* Now, even though it doesn't say this in SVID or any man page, we have */
/* to close and reopen the device.  This is not necessary for all systems, */
/* but it's impossible to predict which ones need it and which ones don't. */

#ifdef ATT7300
/*
  Special handling for ATT 7300 UNIX PC and 3B1, which have "phone"
  related ioctl's for their internal modems.  attmodem has getty status and
  modem-in-use bit.  Reportedly the ATT7300/3B1 PIOCDISC call is necessary,
  but also ruins the file descriptor, and no other phone(7) ioctl call can fix
  it.  Whatever it does, it seems to escape detection with PIOCGETA and TCGETA.
  The only way to undo the damage is to close the fd and then reopen it.
*/
    if (attmodem & ISMODEM) {
	debug(F100,"tthang attmodem close/open","",0);
	ioctl(ttyfd,PIOCUNHOLD,&dialer); /* Return call to handset. */
	ioctl(ttyfd,PIOCDISC,&dialer);	/* Disconnect phone. */
	close(ttyfd);			/* Close and reopen the fd. */
	ttyfd = priv_opn(ttnmsv, O_RDWR | O_NDELAY);
	attmodem &= ~ISMODEM;		/* Phone no longer in use. */
    }
#else /* !ATT7300 */
/* It seems we have to close and open the device for other AT&T systems */
/* too, and this is the place to do it.  The following code does the */
/* famous close(open(...)) magic by default.  If that doesn't work for you, */
/* then try uncommenting the following statement or putting -DCLSOPN in */
/* the makefile CFLAGS. */

/* #define CLSOPN */

#ifndef SCO32 /* Not needed by, and harmful to, SCO UNIX 3.2 / Xenix 2.3 */

#ifdef O_NDELAY
#define OPENFLGS O_RDWR | O_NDELAY
#else
#define OPENFLGS O_RDWR
#endif

#ifndef CLSOPN
/* This method is used by default, i.e. unless CLSOPN is defined. */
/* It is thought to be safer because there is no window where getty */
/* can seize control of the device.  The drawback is that it might not work. */

    debug(F101,"tthang close(open()), OPENFLGS","",OPENFLGS);
    close(priv_opn(ttnmsv, OPENFLGS));

#else
/* This method is used if you #define CLSOPN.  It is more likely to work */
/* than the previous method, but it's also more dangerous. */

    debug(F101,"tthang close/open, OPENFLGS","",OPENFLGS);
    close(ttyfd);
    msleep(10);
    ttyfd = priv_opn(ttnmsv, OPENFLGS);	/* Open it again */
#endif /* CLSOPN */
#undef OPENFLGS

#endif /* SCO32 */
#endif /* ATT7300 */

#endif /* NOCOTFMC */

/* Now put all flags & modes back the way we found them. */
/* (Does the order of ioctl & fcntl matter ? ) */

    debug(F100,"tthang restore settings","",0);
    ttcur.c_cflag = ttc_save;		/* Get old speed back. */
    if (ioctl(ttyfd,TCSETAF,&ttcur) < 0) /* ioctl parameters. */
      return(-1);
#ifdef O_NDELAY
/*
  This is required for IBM RT and RS/6000, probably helps elsewhere too (?).
  After closing a modem line, the modem will probably not be asserting
  carrier any more, so we should not require carrier any more.  If this
  causes trouble on non-IBM UNIXes, change the #ifdef to use _IBMR2 rather
  than O_NDELAY.
*/
    flags &= ~O_NDELAY;			/* Don't require carrier on reopen */
#endif /* O_NDELAY */
    if (fcntl(ttyfd,F_SETFL,flags) < 0)	/* fcntl parameters */
      return(-1);

    return(1);
#endif /* not HPUX */
#endif /* ATTSV */
#endif /* BSD44ORPOSIX */
#endif /* HUP_POSIX */
#endif /* NOLOCAL */
}

/*
  Major change in 5A(174).  We used to use LPASS8, if it was defined, to
  allow 8-bit data and Xon/Xoff flow control at the same time.  But this
  LPASS8 business seems to have been causing trouble for everybody but me!
  For example, Annex terminal servers, commonly used with Encore computers,
  do not support LPASS8 even though the Encore itself does.  Ditto for many
  other terminal servers, TELNET connections, rlogin connections, etc etc.
  Now, reportedly, even vanilla 4.3 BSD systems can't do this right on their
  serial lines, even though LPASS8 is a feature of 4.3BSD.  So let's turn it
  off for everybody.  That means we goes back to using raw mode, with no
  flow control.  Phooey.

  NOTE: This must be done before the first reference to LPASS8 in this file,
  and after the last #include statment.
*/
#ifdef LPASS8
#undef LPASS8
#endif /* LPASS8 */

/*  T T R E S  --  Restore terminal to "normal" mode.  */

/* ske@pkmab.se: There are two choices for what this function should do.
 * (1) Restore the tty to current "normal" mode, with carrier treatment
 * according to ttcarr, to be used after every kermit command. (2) Restore
 * the tty to the state it was in before kermit opened it. These choices
 * conflict, since ttold can't hold both choices of tty parameters.  ttres()
 * is currently being called as in choice (1), but ttold basically holds
 * the initial parameters, as in (2), and the description at the beginning
 * of this file says (2).
 *
 * I don't think restoring tty parameters after all kermit commands makes
 * much of a difference.  Restoring them upon exit from kermit may be of
 * some use in some cases (when the line is not restored automatically on
 * close, by the operating system).
 *
 * I can't choose which one it should be, so I haven't changed it. It
 * probably works as it is, too. It would probably even work even with
 * ttres() entirely deleted...
 *
 * (from fdc: Actually, this function operates in remote mode too, so
 * it restores the console (command) terminal to whatever mode it was
 * in before packet operations began, so that commands work right again.)
 */
int
ttres() {                               /* Restore the tty to normal. */
    int x;

    if (ttyfd < 0) return(-1);          /* Not open. */

    if (ttfdflg) return(0);		/* Don't mess with terminal modes if */
					/* we got ttyfd from another process */
#ifdef	NETCONN
    if (netconn) {			/* Network connection */
        tvtflg = 0;
#ifdef TCPSOCKET
#ifdef TCP_NODELAY
        {
	    extern int tcp_nodelay;	/* Just put this back if necessary */
	    if (ttnet == NET_TCPB) {
		if (nodelay_sav > -1) {
		    no_delay(ttyfd,nodelay_sav);
		    nodelay_sav = -1;
		}
	    }
        }
#endif /* TCP_NODELAY */
#ifdef TN_COMPORT
        if (istncomport()) {
            int rc = -1;
            if ((rc = tnsetflow(ttflow)) < 0)
	      return(rc);
            if (ttspeed <= 0) 
	      ttspeed = tnc_get_baud();
            else if ((rc = tnc_set_baud(ttspeed)) < 0)
	      return(rc);
            tnc_set_datasize(8);
	    tnc_set_stopsize(stopbits);

#ifdef HWPARITY
            if (hwparity) {
                switch (hwparity) {
		  case 'e':			/* Even */
                    debug(F100,"ttres 8 bits + even parity","",0);
                    tnc_set_parity(3);
                    break;
		  case 'o':			/* Odd */
                    debug(F100,"ttres 8 bits + odd parity","",0);
                    tnc_set_parity(2);
                    break;
		  case 'm':			/* Mark */
                    debug(F100,"ttres 8 bits + invalid parity: mark","",0);
                    tnc_set_parity(4);
                    break;
		  case 's':			/* Space */
                    debug(F100,"ttres 8 bits + invalid parity: space","",0);
                    tnc_set_parity(5);
                    break;
                }
            } else
#endif /* HWPARITY */
	    {
                tnc_set_parity(1);              /* None */
            }
            tvtflg = 0;
            return(0);
        }
#endif /* TN_COMPORT */
#endif /* TCPSOCKET */
	return(0);
    }
#endif	/* NETCONN */
#ifdef NETCMD
    if (ttpipe) return(0);
#endif /* NETCMD */
#ifdef NETPTY
    if (ttpty) return(0);
#endif /* NETPTY */

/* Real terminal device, so restore its original modes */

#ifdef BSD44ORPOSIX			/* For POSIX like this */
    debug(F100,"ttres BSD44ORPOSIX","",0);
    x = tcsetattr(ttyfd,TCSADRAIN,&ttold);
#else					/* For all others... */
#ifdef ATTSV                            /* For AT&T versions... */
    debug(F100,"ttres ATTSV","",0);
    x = ioctl(ttyfd,TCSETAW,&ttold);	/* Restore tty modes this way. */
#else
/* Here we restore the modes for BSD */

#ifdef LPASS8				/* Undo "pass8" if it were done */
    if (lmodef) {
	if (ioctl(ttyfd,TIOCLSET,&lmode) < 0)
	  debug(F100,"ttres TIOCLSET failed","",0);
	else
	  debug(F100,"ttres TIOCLSET ok","",0);
    }
#endif /* LPASS8 */

#ifdef CK_DTRCTS		   /* Undo hardware flow if it were done */
    if (lmodef) {
 	if (ioctl(ttyfd,TIOCLSET,&lmode) < 0)
 	  debug(F100,"ttres TIOCLSET failed","",0);
 	else
 	  debug(F100,"ttres TIOCLSET ok","",0);
    }
#endif /* CK_DTRCTS */

#ifdef TIOCGETC				/* Put back special characters */
    if (tcharf && (xlocal == 0)) {
	if (ioctl(ttyfd,TIOCSETC,&tchold) < 0)
	  debug(F100,"ttres TIOCSETC failed","",0);
	else
	  debug(F100,"ttres TIOCSETC ok","",0);
    }
#endif /* TIOCGETC */

#ifdef TIOCGLTC				/* Put back local special characters */
    if (ltcharf && (xlocal == 0)) {
	if (ioctl(ttyfd,TIOCSLTC,&ltchold) < 0)
	  debug(F100,"ttres TIOCSLTC failed","",0);
	else
	  debug(F100,"ttres TIOCSLTC ok","",0);
    }
#endif /* TIOCGLTC */

#ifdef BELLV10
    debug(F100,"ttres BELLV10","",0);
    x = ioctl(ttyfd,TIOCSETP,&ttold);	/* Restore both structs */
    x = ioctl(ttyfd,TIOCSDEV,&tdold);
#else
    debug(F100,"ttres stty","",0);
    x = stty(ttyfd,&ttold);             /* Restore tty modes the old way. */
#endif /* BELLV10 */

    if (!xlocal)
      msleep(100);			/* This replaces sleep(1)... */
					/* Put back sleep(1) if tty is */
					/* messed up after close. */
#endif /* ATTSV */
#endif /* BSD44ORPOSIX */

    debug(F101,"ttres result","",x);
#ifndef QNX
    if (x < 0) debug(F101,"ttres errno","",errno);
#endif /* QNX */

#ifdef AIXRS
#ifndef AIX41
    x = ioctl(ttyfd, ttld & 1 ? TXADDCD : TXDELCD, "rts");
    debug(F101,"ttres AIX line discipline rts restore","",x);
#endif /* AIX41 */
#endif /* AIXRS */

#ifdef BSD41
    if (ttld > -1) {			/* Put back line discipline */
	x = ioctl(ttyfd, TIOCSETD, &ttld);
	debug(F101,"ttres BSD41 line discipline restore","",x);
	if (x < 0) debug(F101,"...ioctl errno","",errno);
	ttld = -1;
    }
#endif /* BSD41 */

#ifdef sony_news
    x = xlocal ? km_ext : km_con;	/* Restore Kanji mode. */
    if (x != -1) {			/* Make sure we know original modes. */
	if (ioctl(ttyfd,TIOCKSET, &x) < 0) {
	    perror("ttres can't set Kanji mode");
	    debug(F101,"ttres error setting Kanji mode","",x);
	    return(-1);
	}
    }
    debug(F100,"ttres set Kanji mode ok","",0);
#endif /* sony_news */

    tvtflg = 0;				/* Invalidate terminal mode settings */
    debug(F101,"ttres return code","",x);
    return(x);
}

#ifndef NOUUCP

/*  T T C H K P I D  --  Check lockfile pid  */
/*
  Read pid from lockfile named f, check that it's still valid.
  If so, return 1.
  On failure to read pid, return 1.
  Otherwise, try to delete lockfile f and return 0 if successful, else 1.
*/
static int
ttchkpid(f) char *f; {
    int pid, mypid, x;
    pid = ttrpid(f);			/* Read pid from file. */
    if (pid > -1) {			/* If we were able to read the pid.. */
	debug(F101,"ttchkpid lock pid","",pid);
	errno = 0;			/* See if process still exists. */
	mypid = (int)getpid();		/* Get my own pid. */
	debug(F101,"ttchkpid my pid","",mypid);
	if (pid == mypid) {		/* It's me! */
	    x = -1;			/* So I can delete it */
	    errno = ESRCH;		/* pretend it's invalid */
	} else {			/* It's not me */
	    x = kill((PID_T)pid, 0);	/* See if it's a live process */
	    debug(F101,"ttchkpid kill errno","",errno);
	}
	debug(F101,"ttchkpid pid test","",x);
	if (x < 0 && errno == ESRCH) { /* pid is invalid */
	    debug(F111,"removing stale lock",f,pid);
	    if (!backgrd)
	      printf("Removing stale lock %s (pid %d terminated)\n", f, pid);
	    priv_on();
	    x = unlink(f);		/* Remove the lockfile. */
	    priv_off();
	    debug(F111,"ttchkpid unlink",f,x);
	    if (x > -1)
	      return(0);		/* Device is not locked after all */
	    else if (!backgrd)
	      perror(f);
	}
	return(1);
    }
    return(1);				/* Failure to read pid */
}

#ifdef HPUX

/* Aliases (different drivers) for HP-UX dialout devices: */

static char *devprefix[] = { "tty", "ttyd", "cul", "cua", "cuad", "culd", "" };
static int ttydexists = 0;

#endif /* HPUX */

/*  T T R P I D  --  Read pid from lockfile "name" */

static int
ttrpid(name) char *name; {
    long len;
    int x, fd, pid;
    short spid;
    char buf[32];

    debug(F110,"ttrpid",name,0);
    if (!name) return(-1);
    if (!*name) return(-1);
    priv_on();
    len = zchki(name);			/* Get file length */
    priv_off();
    debug(F101,"ttrpid zchki","",len);
    if (len < 0)
      return(-1);
    if (len > 31)
      return(-1);
    priv_on();
    fd = open(name,O_RDONLY);		/* Try to open lockfile. */
    priv_off();
    debug(F101,"ttrpid fd","",fd);
    if (fd <= 0)
      return(-1);
/*
  Here we try to be flexible and allow for all different binary and string
  formats at runtime, rather than a specific format for each configuration
  hardwired at compile time.
*/
    pid = -1;
#ifndef COHERENT
/*
  COHERENT uses a string PID but without leading spaces or 0's, so there is
  no way to tell from the file's length whether it contains a string or binary
  pid.  So for COHERENT only, we only allow string pids.  For all others, we
  decide based on the size of the lockfile.
*/
    if (len > 4) {			/* If file > 4 bytes it's a string */
#endif /* COHERENT */
	x = read(fd,buf,(int)len);
	debug(F111,"ttrpid string read",buf,x);
	if (x < 0) {
	    pid = -1;
	} else {
	    buf[31] = '\0';
	    x = sscanf(buf,"%d",&pid);	/* Get the integer pid from it. */
	}
#ifndef COHERENT
    } else if (len == 4) {		/* 4 bytes so binary */
	x = read(fd, (char *)&pid, 4);	/* Read the bytes into an int */
	debug(F101,"ttrpid integer read","",x);
	if (x < 4)
	  pid = -1;
    } else if (len == 2) {		/* 2 bytes binary */
	x = read(fd, (char *)&spid, 2);	/* Read the bytes into a short */
	debug(F101,"ttrpid short read","",x);
	if (x < 2)
	  pid = -1;
	else
	  pid = spid;
    } else
      pid = -1;
#endif /* COHERENT */
    close(fd);				/* Close the lockfile */
    debug(F101,"ttrpid pid","",pid);
    return(pid);
}
#endif /* NOUUCP */

/*  T T L O C K  */

/*
  This function attempts to coordinate use of the communication device with
  other copies of Kermit and any other program that follows the UUCP
  device-locking conventions, which, unfortunately, vary among different UNIX
  implementations.  The idea is to look for a file of a certain name, the
  "lockfile", in a certain directory.  If such a file is found, then the line
  is presumed to be in use, and Kermit should not use it.  If no such file is
  found, Kermit attempts to create one so that other programs will not use the
  same line at the same time.  Because the lockfile and/or the directory it's
  in might lack write permission for the person running Kermit, Kermit could
  find itself running setuid to uucp or other user that does have the
  necessary permissions.  At startup, Kermit has changed its effective uid to
  the user's real uid, and so ttlock() must switch back to the original
  effective uid in order to create the lockfile, and then back again to the
  real uid to prevent unauthorized access to other directories or files owned
  by the user the program is setuid to.

  Totally rewritten for C-Kermit 5A to eliminate windows of vulnerability,
  based on suggestions from Warren Tucker.  Call with pointer to name of
  tty device.  Returns:

   0 on success
  -1 on failure

  Note: Once privileges are turned on using priv_on(), it is essential that
  they are turned off again before this function returns.
*/
#ifdef SVR4				/* Lockfile uses device numbers. */
/*
  Although I can't find this in writing anywhere (e.g. in SVID for SVR4),
  it is the behavior of the "reference version" of SVR4, i.e. the Intel
  port from UNIX Systems Laboratories, then called Univel UnixWare,
  then called Novell UnixWare, then called SCO Unixware, then called Caldera
  Open UNIX...  It also makes much more sense than device-name-based lockfiles
  since there can be multiple names for the same device, symlinks, etc.
*/
#ifndef NOLFDEVNO
#ifndef LFDEVNO				/* Define this for SVR4 */
#ifndef AIXRS				/* But not for RS/6000 AIX 3.2, etc. */
#ifndef BSD44				/* If anybody else needs it... */
#ifndef __386BSD__
#ifndef __FreeBSD__
#ifndef HPUX10
#ifndef IRIX51				/* SGI IRIX 5.1 or later */
#ifndef CK_SCOV5			/* SCO Open Server 5.0 */
#define LFDEVNO
#endif /* CK_SCOV5 */
#endif /* IRIX51 */
#endif /* HPUX10 */
#endif /* __FreeBSD__ */
#endif /* __386BSD__ */
#endif /* BSD44 */
#endif /* AIXRS */
#endif /* LFDEVNO */			/* ... define it here or on CC */
#endif /* NOLFDEVNO */
#endif /* SVR4 */			/* command line. */

#ifdef COHERENT
#define LFDEVNO
#endif /* COHERENT */

/*
  For platforms where the lockfile name is made from device/major/minor
  device number, as in SVR4.  Which, if we must have lockfiles at all, is
  by far the best format, since it eliminates all the confusion that stems
  from multiple names (or drivers) for the same port, not to mention
  symlinks.  It might even be a good idea to start using this form even
  on platforms where it's not supported, alongside the normal forms for those
  platforms, in order to get people used to it...
*/
#ifdef LFDEVNO
#ifndef major				/* If we didn't find it */
#ifdef SVR4				/* then for Sys V R4 */
#include <sys/mkdev.h>			/* look here */
#else					/* or for SunOS versions */
#ifdef SUNOS4				/* ... */
#include <sys/sysmacros.h>		/* look here */
#else					/* Otherwise take a chance: */
#define	major(dev) ( (int) ( ((unsigned)(dev) >> 8) & 0xff))
#define	minor(dev) ( (int) ( (dev) & 0xff))
#endif /* SUNOS4 */
#endif /* SVR4 */
#endif /* major */
#endif /* LFDEVNO */

/* No advisory locks if F_TLOCK and F_ULOCK are not defined at this point */

#ifdef LOCKF
#ifndef F_TLOCK
#undef LOCKF
#ifndef NOLOCKF
#define NOLOCKF
#endif /* NOLOCKF */
#endif /* F_TLOCK */
#endif /* LOCKF */

#ifdef LOCKF
#ifndef F_ULOCK
#undef LOCKF
#ifndef NOLOCKF
#define NOLOCKF
#endif /* NOLOCKF */
#endif /* F_ULOCK */
#endif /* LOCKF */

static char linkto[DEVNAMLEN+1];
static char * linkdev = NULL;

#ifndef NOUUCP
#ifdef USETTYLOCK
#ifdef LOCK_DIR
char * uucplockdir = LOCK_DIR;
#else
char * uucplockdir = "";
#endif /* LOCK_DIR */
#else
#ifdef LOCK_DIR
char * uucplockdir = LOCK_DIR;
#else
char * uucplockdir = "";
#endif /* LOCK_DIR */
#endif /* USETTYLOCK */
#else
char * uucplockdir = "";
#endif /* NOUUCP */

#ifdef QNX				/* Only for QNX4 */
int					/* Visible to outside world */
qnxopencount() {			/* Get QNX device open count */
    struct _dev_info_entry info;
    int x;

    x = -1;				/* Unknown */
    if (ttyfd > -1) {
	if (!dev_info(ttyfd, &info)) {
	    debug(F101,"ttlock QNX open_count","",info.open_count);
	    x = info.open_count;
	}
    }
    return(x);
}
#endif /* QNX */

char *
ttglckdir() {				/* Get Lockfile directory name */
#ifdef __OpenBSD__
    return("/var/spool/lock");
#else /* __OpenBSD__ */
#ifdef __FreeBSD__
    return("/var/spool/lock");
#else  /* __FreeBSD__ */
#ifdef LOCK_DIR
    char * s = LOCK_DIR;
#endif /* LOCK_DIR */
#ifdef NOUUCP
    return("");
#else  /* NOUUCP */
#ifdef LOCK_DIR
    return(s);
#else  /* LOCK_DIR */
    return("");
#endif /* LOCK_DIR */
#endif /* NOUUCP */
#endif /* __FreeBSD__ */
#endif /* __OpenBSD__ */
}

static int
ttlock(ttdev) char *ttdev; {

    int x, n;
    int islink = 0;
#ifdef __FreeBSD__
    char *devname;
#endif	/* __FreeBSD__ */

#ifdef NOUUCP
    debug(F100,"ttlock NOUUCP","",0);
    ckstrncpy(flfnam,"NOLOCK",FLFNAML);
    haslock = 1;
    return(0);
#else /* !NOUUCP */

#ifdef USETTYLOCK
    haslock = 0;                        /* Not locked yet. */
    *flfnam = '\0';			/* Lockfile name is empty. */
#ifdef __FreeBSD__
    if ((devname = xxlast(ttdev,'/')) != NULL)
#ifdef FREEBSD8
      ckstrncat(lockname,devname+1,DEVNAMLEN-ckstrncpy(lockname,"pts",4));
#else
      ckstrncpy(lockname,devname+1,DEVNAMLEN);
#endif	/* FREEBSD8 */
#else
    if (!strncmp(ttdev,"/dev/",5) && ttdev[5])
      ckstrncpy(lockname,ttdev+5,DEVNAMLEN);
#endif	/* __FreeBSD__ */
    else
      ckstrncpy(lockname,ttdev,DEVNAMLEN);
/*
  This might be overkill, but it's not clear from the man pages whether
  ttylock() can be called without calling ttylocked() first, since the doc
  says that ttylocked() removes any stale lockfiles, but it does not say this
  about ttylock().  Also the docs don't say what ttylocked() returns in the
  case when it finds and removes a stale lockfile.  So one or both calls to
  to ttylocked() might be superfluous, but they should do no harm.  Also I'm
  assuming that we have to do all the same ID swapping, etc, with these
  routines as we do without them.  Thus the priv_on/off() sandwich.
*/
#ifdef USE_UU_LOCK
    priv_on();				/* Turn on privs */
    x = uu_lock(lockname);		/* Try to set the lock */
    priv_off();				/* Turn privs off */
    debug(F111,"ttlock uu_lock",lockname,x);
    switch (x) {
      case UU_LOCK_INUSE:
	return(-2);
      case UU_LOCK_OK:
#ifdef BSD44
	ckmakmsg(flfnam,FLFNAML,"/var/spool/lock/LCK..",lockname,NULL,NULL);
#endif /* BSD44 */
	haslock = 1;
	return(0);
      default:
	return(-1);
    }
#else  /* USE_UU_LOCK */
    priv_on();				/* Turn on privs */
    if (ttylocked(lockname)) {		/* This should remove any stale lock */
	if (ttylocked(lockname)) {	/* so check again. */
	    priv_off();
	    return(-5);			/* Still locked, fail. */
	}
    }
    x = ttylock(lockname);		/* Lock it. */
    priv_off();				/* Turn off privs */

    debug(F111,"ttlock lockname",lockname,x);
    if (x > -1) {
	/*
	  We don't really know the name of the lockfile, but
	  this is what the man page says it is.  In USETTYLOCK
          builds, it is used only for display by SHOW COMM.
	*/
	ckmakmsg(flfnam,FLFNAML,"/etc/locks/LCK..",lockname,NULL,NULL);
	haslock = 1;
    }
    return(x);
#endif /* USE_UU_LOCK */
#else  /* Systems that don't have ttylock()... */

#ifndef HPUX

    int lockfd;				/* File descriptor for lock file. */
    PID_T pid;				/* Process id of this process. */
    int tries;				/* How many times we've tried... */
    struct stat devbuf;			/* For device numbers (SVR4). */

#ifdef PIDSTRING
    char pid_str[32];			/* My pid in string format. */
#endif /* PIDSTRING */

    char *device, *devname;

#define LFNAML 256			/* Max length for lock file name. */
    char lockfil[LFNAML];		/* Lock file name */
#ifdef RTAIX
    char lklockf[LFNAML];		/* Name for link to lock file  */
#endif /* RTAIX */
#ifdef CKSYMLINK
    char symlock[LFNAML];		/* Name for symlink lockfile name */
#endif /* CKSYMLINK */
    char tmpnam[LFNAML+30];		/* Temporary lockfile name. */
    char *lockdir = LOCK_DIR;		/* Defined near top of this file, */
					/* or on cc command line. */
    haslock = 0;                        /* Not locked yet. */
    *flfnam = '\0';			/* Lockfile name is empty. */
    lock2[0] = '\0';			/* Clear secondary lockfile name. */
    pid = getpid();			/* Get id of this process. */

/*  Construct name of lockfile and temporary file */

/*  device  = name of tty device without the path, e.g. "ttyh8" */
/*  lockfil = name of lock file, without path, e.g. "LCK..ttyh8" */

    device = ((devname = xxlast(ttdev,'/')) != NULL ? devname+1 : ttdev);

    if (stat(ttdev,&devbuf) < 0)
      return(-1);

#ifdef CKSYMLINK
    islink = 1;				/* Assume it's a symlink */
    linkto[0] = '\0';			/* But we don't know to what */
#ifdef COMMENT
/*
  This is undependable.  If it worked it would save the readlink call if
  we knew the device name was not a link.
*/
#ifdef S_ISLNK
    islink = S_ISLNK(devbuf.st_mode);
    debug(F101,"ttlock stat S_ISLNK","",islink);
#endif /* S_ISLNK */
#endif /* COMMENT */
    if (islink) {
	n = readlink(ttdev,linkto,DEVNAMLEN); /* See if it's a link */
	debug(F111,"ttlock readlink",ttdev,n);
	if (n > -1)			/* It is */
	  linkto[n] = '\0';
	else				/* It's not */
	  islink = 0;
	debug(F111,"ttlock link",linkto,islink);
    }
    if (islink) {
	linkdev = (devname = xxlast(linkto,'/')) ? devname + 1 : linkto;
	debug(F110,"ttlock linkdev",linkdev,0);
    }
#endif /* CKSYMLINK */

/*
  On SCO platforms, if we don't have a symlink, then let's pretend the
  name given for the device is a symlink, because later we will change
  the name if it contains any uppercase characters.
*/
#ifdef CK_SCOV5				/* SCO Open Server 5.0 */
    if (!islink) {
	islink = 1;
	ckstrncpy(linkto,ttdev,DEVNAMLEN);
	linkdev = (devname = xxlast(linkto,'/')) ? devname + 1 : linkto;
	debug(F110,"ttlock linkdev",linkdev,0);
    }
#else
#ifdef M_XENIX				/* SCO Xenix or UNIX */
    if (!islink) {
	islink = 1;
	ckstrncpy(linkto,ttdev,DEVNAMLEN);
	linkdev = (devname = xxlast(linkto,'/')) ? devname + 1 : linkto;
	debug(F110,"ttlock linkdev",linkdev,0);
    }
#endif /* M_XENIX */
#endif /* CK_SCOV5 */

#ifdef ISIII				/* Interactive System III, PC/IX */
    ckstrncpy(lockfil, device, DEVNAMLEN);
#else  /* not ISIII */
#ifdef LFDEVNO				/* Lockfilename has device numbers. */
#ifdef COHERENT
    sprintf(lockfil,"LCK..%d.%d",	/* SAFE */
	    major(devbuf.st_rdev),	   /* major device number */
	    0x1f & minor(devbuf.st_rdev)); /* minor device number */
#else
    /* Note: %d changed to %u in 8.0 -- %u is part of SVID for SVR4 */
    /* Lockfile name format verified to agree with Solaris cu, Dec 2001 */
    sprintf(lockfil,"LK.%03u.%03u.%03u", /* SAFE */
	    major(devbuf.st_dev),	/* device */
	    major(devbuf.st_rdev),	/* major device number */
	    minor(devbuf.st_rdev));	/* minor device number */
#endif /* COHERENT */
#else  /* Not LFDEVNO */
#ifdef PTX				/* Dynix PTX */
    if ((device != &ttdev[5]) && (strncmp(ttdev,"/dev/",5) == 0)) {
	if ((int)strlen(device) + 8 < LFNAML)
	  sprintf(lockfil,"LCK..%.3s%s", &ttdev[5], device);
	else
	  ckstrncpy(lockfil,"LOCKFILE_NAME_TOO_LONG",LFNAML);
    } else
#endif /* PTX */
      if ((int)strlen(device) + 5 < LFNAML)
	sprintf(lockfil,"LCK..%s", device);
      else
	ckstrncpy(lockfil,"LOCKFILE_NAME_TOO_LONG",LFNAML);
#ifdef RTAIX
    ckstrncpy(lklockf,device,DEVNAMLEN);
#endif /* RTAIX */
#ifdef CKSYMLINK
    symlock[0] = '\0';
    if (islink)
      ckmakmsg(symlock,LFNAML, "LCK..", linkdev, NULL, NULL);
#endif /* CKSYMLINK */
#endif /* LFDEVNO */
#endif /* ISIII */

#ifdef CK_SCOV5				/* SCO Open Server 5.0 */
    {
	/* Lowercase the entire filename. */
        /* SCO says we must do this in V5.0 and later. */
	/* BUT... watch out for devices -- like Digiboard Portserver */
	/* That can have hundreds of ports... */
	char *p = (char *)(lockfil + 5);
	while (*p) { if (isupper(*p)) *p = (char) tolower(*p); p++; }
    }
#ifdef CKSYMLINK
    if (islink) {			/* If no change */
	if (!strcmp(lockfil,symlock)) {	/* then no second lockfile needed */
	    islink = 0;
	    symlock[0] = '\0';
	}
    }
#endif /* CKSYMLINK */
#else
#ifdef M_XENIX				/* SCO Xenix or UNIX */
    {
	int x; char c;
	x = (int)strlen(lockfil) - 1;	/* Get last letter of device name. */
	if (x > 0) {			/* If it's uppercase, lower it. */
	    c = lockfil[x];
	    if (c >= 'A' && c <= 'Z') lockfil[x] += ('a' - 'A');
	}
    }
#ifdef CKSYMLINK
    if (islink) {
	if (!strcmp(lockfil,symlock)) {	/* No change */
	    islink = 0;			/* so no second lockfile */
	    symlock[0] = '\0';
	}
    }
#endif /* CKSYMLINK */
#endif /* M_XENIX */
#endif /* CK_SCOV5 */

/*  flfnam = full lockfile pathname, e.g. "/usr/spool/uucp/LCK..ttyh8" */
/*  tmpnam = temporary unique, e.g. "/usr/spool/uucp/LTMP..pid" */

    ckmakmsg(flfnam,LFNAML,lockdir,"/",lockfil,NULL);

#ifdef RTAIX
    ckmakmsg(lkflfn,FLFNAML,lockdir,"/",lklockf,NULL);
#endif /* RTAIX */

#ifndef LFDEVNO
#ifdef CKSYMLINK
    /* If it's a link then also make a lockfile for the real name */
    debug(F111,"ttlock link symlock",symlock,islink);
    if (islink && symlock[0]) {
	/* But only if the lockfile names would be different. */
	/* WARNING: They won't be, e.g. for /dev/ttyd2 => /hw/ttys/ttyd2 */
	ckmakmsg(lock2,FLFNAML,lockdir,"/",symlock,NULL);
	debug(F110,"ttlock lock2",lock2,0);
	if (!strcmp(lock2,flfnam)) {	/* Are lockfile names the same? */
	    debug(F100,"ttlock lock2 cleared","",0);
	    lock2[0] = '\0';		/* Clear secondary lockfile name. */
	}
    }
#endif /* CKSYMLINK */
#endif /* LFDEVNO */

    sprintf(tmpnam,"%s/LTMP.%05d",lockdir,(int) pid); /* safe */
    debug(F110,"ttlock flfnam",flfnam,0);
    debug(F110,"ttlock tmpnam",tmpnam,0);

    priv_on();				/* Turn on privileges if possible. */
    lockfd = creat(tmpnam, 0444);	/* Try to create temp lock file. */
    if (lockfd < 0) {			/* Create failed. */
	debug(F111,"ttlock creat failed",tmpnam,errno);
	if (errno == ENOENT) {
	    perror(lockdir);
	    printf("UUCP not installed or Kermit misconfigured\n");
	} else {
	    if (!quiet)
	      perror(lockdir);
	    unlink(tmpnam);		/* Get rid of the temporary file. */
	}
	priv_off();			/* Turn off privileges!!! */
	return(-1);			/* Return failure code. */
    }
/* Now write the pid into the temp lockfile in the appropriate format */

#ifdef PIDSTRING			/* For Honey DanBer UUCP, */
    sprintf(				/* write PID as decimal string */
	    pid_str,
#ifdef LINUXFSSTND			/* The "Linux File System Standard" */
#ifdef FSSTND10				/* Version 1.0 calls for */
	    "%010d\n",			/* leading zeros */
#else					/* while version 1.2 calls for */
	    "%10d\n",			/* leading spaces */
#endif /* FSSTND10 */
#else
#ifdef COHERENT
	    "%d\n",			/* with leading nothing */
#else
	    "%10d\n",			/* with leading blanks */
#endif /* COHERENT */
#endif /* LINUXFSSTND */
	    (int) pid
	    );				/* safe */
    write(lockfd, pid_str, 11);
    debug(F111,"ttlock hdb pid string",pid_str,(int) pid);

#else /* Not PIDSTRING, use integer PID */

    write(lockfd, (char *)&pid, sizeof(pid) );
    debug(F101,"ttlock pid","",(int) pid);

#endif /* PIDSTRING */

/* Now try to rename the temp file to the real lock file name. */
/* This will fail if a lock file of that name already exists.  */

    close(lockfd);			/* Close the temp lockfile. */
    chmod(tmpnam,0444);			/* Permission for a valid lock. */
    tries = 0;
    while (!haslock && tries++ < 2) {
	haslock = (link(tmpnam,flfnam) == 0); /* Create a link to it. */
	if (haslock) {			      /* If we got the lockfile */
#ifdef RTAIX
	    link(flfnam,lkflfn);
#endif /* RTAIX */
#ifdef CKSYMLINK
#ifndef LFDEVNO
	    if (islink && lock2[0])
	      link(flfnam,lock2);
#endif /* LFDEVNO */
#endif /* CKSYMLINK */

#ifdef COMMENT
/* Can't do this any more because device is not open yet so no ttyfd. */
#ifdef LOCKF
/*
  Advisory file locking works on SVR4, so we use it.  In fact, it is
  necessary in some cases, e.g. when SLIP is involved.  But it still doesn't
  seem to prevent multiple users accessing the same device by different names.
*/
            while (lockf(ttyfd, F_TLOCK, 0L) != 0) {
                debug(F111, "ttlock lockf returns errno", "", errno);
                if ((++tries >= 3) || (errno != EAGAIN)) {
                    x = unlink(flfnam); /* remove the lockfile */
#ifdef RTAIX
		    unlink(lkflfn);	/* And any links to it... */
#endif /* RTAIX */
#ifdef CKSYMLINK
#ifndef LFDEVNO
		    if (islink && lock2[0])
		      unlink(lock2);	/* ditto... */
#endif /* LFDEVNO */
#endif /* CKSYMLINK */
                    debug(F111,"ttlock unlink",flfnam,x);
                    haslock = 0;
		    break;
		}
                sleep(2);
	    }
	    if (haslock)		/* If we got an advisory lock */
#endif /* LOCKF */
#endif /* COMMENT */
	      break;			/* We're done. */

	} else {			/* We didn't create a new lockfile. */
	    priv_off();
	    if (ttchkpid(flfnam)) {	/* Check existing lockfile */
		priv_on();		/* cause ttchkpid turns priv_off... */
		unlink(tmpnam);		/* Delete the tempfile */
		debug(F100,"ttlock found tty locked","",0);
		priv_off();		/* Turn off privs */
		return(-2);		/* Code for device is in use. */
	    }
	    priv_on();
	}
    }
    unlink(tmpnam);			/* Unlink (remove) the temp file. */
    priv_off();				/* Turn off privs */
    return(haslock ? 0 : -1);		/* Return link's return code. */

#else /* HPUX */

/*
  HP-UX gets its own copy of this routine, modeled after the observed behavior
  of the HP-UX 'cu' program.  HP-UX serial device names consist of a base name
  such as "tty", "ttyd", "cua", "cul", "cuad", or "culd", followed by a unit
  designator which is a string of digits, possibly containing an imbedded
  letter "p".  Examples (for base name "tty"):

     /dev/tty0, /dev/tty00, dev/ttyd00, /dev/tty0p0

  According to the HP-UX UUCP manual of 1988, the "0p0" notation has been
  used on Series 800 since HP-UX 2.00, and the "non-p" notation was used
  on other models.  In HP-UX 10.00, "0p0" notation was adopted for all models.
  However, we make and enforce no such distinctions; either notation is
  accepted on any model or HP-UX version as a valid unit designator.

  If a valid unit is specified (as opposed to a designer name or symlink), we
  check for all aliases of the given unit according to the devprefix[] array.
  If no lockfiles are found for the given unit, we can have the device; we
  create a lockfile LCK..name in the lockfile directory appropriate for the
  HP-UX version (/var/spool/locks for 10.00 and later, /usr/spool/uucp for
  9.xx and earlier).  If it is a "cua" or "cul" device, a second lockfile is
  created with the "ttyd" prefix.  This is exactly what cu does.

  If the "set line" device does not have a valid unit designator, then it is
  used literally and no synomyms are searched for and only one lockfile is
  created.

  -fdc, March 1998.
*/
#define LFNAML 80			/* Max length for lock file name. */

    int lockfd;				/* File descriptor for lock file. */
    PID_T pid;				/* Process ID of this process. */
    int fpid;				/* pid found in existing lockfile. */
    int tries;				/* How many times we've tried... */
    int i, k;				/* Workers */

    char *device, *devname;		/* "/dev/xxx", "xxx" */
    char *unit, *p;			/* <instance>p<port> part of xxx */

    char lockfil[LFNAML];		/* Lockfile name (no path) */
    char tmpnam[LFNAML];		/* Temporary lockfile name. */

#ifdef HPUX10				/* Lockfile directory */
    char *lockdir = "/var/spool/locks";	/* Always this for 10.00 and higher */
#else  /* HP-UX 9.xx and below */
#ifdef LOCK_DIR
    char *lockdir = LOCK_DIR;		/* Defined near top of this file */
#else
    char *lockdir = "/usr/spool/uucp";	/* or not... */
#endif /* LOCK_DIR */
#endif /* HPUX10 */

    haslock = 0;                        /* Not locked yet. */
    *flfnam = '\0';			/* Lockfile name is empty. */
    lock2[0] = '\0';			/* Second one too. */
    pid = getpid();			/* Get my process ID */
/*
  Construct name of lockfile and temporary file...
  device  = name of tty device without the path, e.g. "tty0p0"
  lockfil = name of lock file, without path, e.g. "LCK..tty0p0"
*/
    device = ((devname = xxlast(ttdev,'/')) != NULL ? devname+1 : ttdev);
    debug(F110,"TTLOCK device",device,0);
    ckmakmsg(lockfil,LFNAML,"LCK..",device,NULL,NULL);

    k = 0;				/* Assume device is not locked */
    n = 0;				/* Digit counter */
    unit = device;			/* Unit = <instance>p<port> */
    while (*unit && !isdigit(*unit))	/* Search for digit... */
      unit++;
    p = unit;				/* Verify <num>p<num> format... */
    debug(F110,"TTLOCK unit 1",unit,0);
/*
  The unit number is recognized as:
  (a) any sequence of digits that runs to the end of the string.
  (b) any (a) that includes one and only one letter "p", with at least
      one digit before and after it.
*/
    while (isdigit(*p)) p++, n++;	/* Get a run of digits */
    if (*p && n > 0) {			/* Have a "p"? */
	if (*p == 'p' && isdigit(*(p+1))) {
	    p++;
	    n = 0;
	    while (isdigit(*p)) p++, n++;
	}
    }
    if (n == 0 || *p) unit = "";
    debug(F110,"TTLOCK unit 2",unit,0);

    if (*unit) {			/* Device name has unit number. */
	/* The following loop not only searches for the various lockfile    */
	/* synonyms, but also removes all -- not just one -- stale lockfile */
	/* for the device, should there be more than one.  See ttchkpid().  */
	ttydexists = 0;
	for (i = 0; *devprefix[i]; i++) { /* For each driver... */
	    /* Make device name */
	    ckmakmsg(lock2,FLFNAML,"/dev/",devprefix[i],unit,NULL);
	    priv_on();			/* Privs on */
	    k = zchki(lock2) != -1;	/* See if device exists */
	    priv_off();			/* Privs off */
	    debug(F111,"TTLOCK exist",lock2,k);
            if (k) {
		if (!strcmp(devprefix[i],"ttyd")) /* ttyd device exists */
		  ttydexists = 1;
		/* Make lockfile name */
		ckmakmsg(lock2,FLFNAML,lockdir,"/LCK..",devprefix[i],unit);
		debug(F110,"TTLOCK checking",lock2,0);
		priv_on();		/* Privs on */
		k = zchki(lock2) != -1;	/* See if lockfile exists */
		priv_off();		/* Privs off */
		debug(F111,"TTLOCK check for lock A",lock2,k);
		if (k) if (ttchkpid(lock2)) { /* If pid still active, fail. */
		    ckstrncpy(flfnam,lock2,FLFNAML);
		    return(-2);
		}
	    }
	}
    } else {				/* Some other device-name format */
	/* This takes care of symbolic links, etc... */
	/* But does not chase them down! */
	ckmakmsg(lock2,FLFNAML,lockdir,"/LCK..",device,NULL);
	priv_on();
	k = zchki(lock2) != -1;		/* Check for existing lockfile */
	priv_off();
	debug(F111,"TTLOCK check for lock B",lock2,k);
	if (k) if (ttchkpid(lock2)) {	/* Check pid from lockfile */
	    ckstrncpy(flfnam,lock2,FLFNAML);
	    debug(F110,"TTLOCK in use",device,0);
	    debug(F101,"TTLOCK returns","",-2);
	    return(-2);
	}
    }
/*
  Get here only if there is no (more) lockfile, so now we make one (or two)...
  flfnam = full lockfile pathname, e.g. "/usr/spool/uucp/LCK..cul0p0".
  tmpnam = unique temporary filname, e.g. "/usr/spool/uucp/LTMP..pid".
*/
    ckmakmsg(flfnam,FLFNAML,lockdir,"/",lockfil,NULL); /* SET LINE device */

    /* If dialout device, also make one for corresponding dialin device */
    lock2[0] = '\0';
    if (!strncmp(device,"cu",2) && *unit && ttydexists)
      ckmakmsg(lock2,FLFNAML,lockdir,"/LCK..ttyd",unit,NULL);

    if ((int)strlen(lockdir)+12 < LFNAML)
      sprintf(tmpnam,"%s/LTMP.%05d",lockdir,(int) pid); /* Make temp name */
#ifdef DEBUG
    if (deblog) {
	debug(F110,"TTLOCK flfnam",flfnam,0);
	debug(F110,"TTLOCK lock2",lock2,0);
	debug(F110,"TTLOCK tmpnam",tmpnam,0);
    }
#endif /* DEBUG */
/*
   Lockfile permissions...
   444 is standard, HP-UX 10.00 uses 664.  It doesn't matter.
   Kermit uses 444; the difference lets us tell whether Kermit created
   the lock file.
*/
    priv_on();				/* Turn on privileges. */
    lockfd = creat(tmpnam, 0444);	/* Try to create temporary file. */
    if (lockfd < 0) {			/* Create failed. */
	debug(F111,"TTLOCK creat failed",tmpnam,errno);
	if (errno == ENOENT) {
	    perror(lockdir);
	    printf("UUCP not installed or Kermit misconfigured\n");
	} else {
	    if (!quiet)
	      perror(lockdir);
	    unlink(tmpnam);		/* Get rid of the temporary file. */
	}
	priv_off();			/* Turn off privileges!!! */
	debug(F101,"TTLOCK returns","",-1);
	return(-1);			/* Return failure code. */
    }
    debug(F110,"TTLOCK temp ok",tmpnam,0);

/* Now write our pid into the temp lockfile in integer format. */

    i = write(lockfd, (char *)&pid, sizeof(pid));

#ifdef DEBUG
    if (deblog) {
	debug(F101,"TTLOCK pid","",pid);
	debug(F101,"TTLOCK sizeof pid","",sizeof(pid));
	debug(F101,"TTLOCK write pid returns","",i);
    }
#endif /* DEBUG */

/*
  Now try to rename the temporary file to the real lockfile name.
  This will fail if a lock file of that name already exists, which
  will catch race conditions with other users.
*/
    close(lockfd);			/* Close the temp lockfile. */
    chmod(tmpnam,0444);

    tries = 0;
    while (!haslock && tries++ < 2) {
	haslock = (link(tmpnam,flfnam) == 0); /* Create a link to it. */
	debug(F101,"TTLOCK link","",haslock);
	if (haslock) {			/* If we made the lockfile... */

#ifdef COMMENT
/* We can't do this any more because we don't have a file descriptor yet. */
#ifdef LOCKF				/* Can be canceled with -DNOLOCKF */
/*
  Create an advisory lock on the device through its file descriptor.
  This code actually seems to work.  If it is executed, and then another
  process tries to open the same device under a different name to circumvent
  the lockfile, they get a "device busy" error.
*/
	    debug(F100,"TTLOCK LOCKF code...","",0);
            while ( lockf(ttyfd, F_TLOCK, 0L) != 0 ) {
                debug(F111, "TTLOCK lockf error", "", errno);
                if ((++tries >= 3) || (errno != EAGAIN)) {
                    x = unlink(flfnam); /* Remove the lockfile */
		    if (errno == EACCES && !quiet)
		      printf("Device already locked by another process\n");
                    haslock = 0;
		    break;
		}
                sleep(2);
	    }
#endif /* LOCKF */
#endif /* COMMENT */

	    if (haslock) {		/* If we made the lockfile ... */
		if (lock2[0]) {		/* if there is to be a 2nd lockfile */
		    lockfd = creat(lock2, 0444); /* Create it */
		    debug(F111,"TTLOCK lock2 creat", lock2, lockfd);
		    if (lockfd > -1) {	/* Created OK, write pid. */
			write(lockfd, (char *)&pid, sizeof(pid) );
			close(lockfd);	/* Close and */
			chmod(lock2, 0444); /* set permissions. */
		    } else {		 /* Not OK, but don't fail. */
			lock2[0] = '\0'; /* Just remember it's not there. */
		    }
		}
		break;			/* and we're done. */
	    }
	}
    }
    unlink(tmpnam);			/* Unlink (remove) the temp file. */
    priv_off();				/* Turn off privs */
    i = haslock ? 0 : -1;		/* Our return value */
    debug(F101,"TTLOCK returns","",i);
    return(i);
#endif /* HPUX */
#endif /* USETTYLOCK */
#endif /* !NOUUCP */
}

/*  T T U N L O C K  */

static int
ttunlck() {                             /* Remove UUCP lockfile(s). */
#ifndef NOUUCP
    int x;

    debug(F111,"ttunlck",flfnam,haslock);

#ifdef USETTYLOCK

    if (haslock && *flfnam) {
	int x;
	priv_on();			/* Turn on privs */
#ifdef USE_UU_LOCK
	x = uu_unlock(lockname);
#else  /* USE_UU_LOCK */
	x = ttyunlock(lockname);	/* Try to unlock */
#endif /* USE_UU_LOCK */
	priv_off();			/* Turn off privs */
	if (x < 0 && !quiet)
	  printf("Warning - Can't remove lockfile: %s\n", flfnam);

	*flfnam = '\0';			/* Erase the name. */
	haslock = 0;
	return(0);
    }

#else  /* No ttylock()... */

    if (haslock && *flfnam) {
	/* Don't remove lockfile if we didn't make it ourselves */
	if ((x = ttrpid(flfnam)) != (int)getpid()) {
	    debug(F111,"ttunlck lockfile seized",flfnam,x);
	    printf("Warning - Lockfile %s seized by pid %d\n",
		   flfnam,
		   x
		   );
	    return(0);
	}
	priv_on();			/* Turn privileges on.  */
	errno = 0;
	x = unlink(flfnam);		/* Remove the lockfile. */
	debug(F111,"ttunlck unlink",flfnam,x);
	if (x < 0) {
	    if (errno && !quiet)
	      perror(ttnmsv);
	    printf("Warning - Can't remove lockfile: %s\n", flfnam);
	}
	haslock = 0;
	*flfnam = '\0';			/* Erase the name. */

#ifdef RTAIX
	errno = 0;
	x = unlink(lkflfn);		/* Remove link to lockfile */
	debug(F111,"ttunlck AIX link unlink",lkflfn,x);
	if (x < 0) {
	    if (errno && !quiet)
	      perror(ttnmsv);
	    printf("Warning - Can't remove link to lockfile: %s\n", lkflfn);
	}
	*lkflfn = '\0';
#else
	if (lock2[0]) {			/* If there is a second lockfile, */
	    errno = 0;
	    x = unlink(lock2);		/*  remove it too. */
	    debug(F111,"ttunlck lock2 unlink",lock2,x);
	    if (x < 0) {
		if (errno && !quiet)
		  perror(ttnmsv);
		printf("Warning - Can't remove secondary lockfile: %s\n",
		       lock2
		       );
	    }
	    lock2[0] = '\0';		/* Forget its name. */
	}
#endif /* RTAIX */

#ifdef COMMENT
#ifdef LOCKF
        (VOID) lockf(ttyfd, F_ULOCK, 0L); /* Remove advisory lock */
#endif /* LOCKF */
#endif /* COMMENT */

	priv_off();			/* Turn privileges off. */
    }
#endif /* USETTYLOCK */
#endif /* !NOUUCP */
    return(0);
}

/*
  4.3BSD-style UUCP line direction control.
  (Stan Barber, Rice U, 1980-something...)
*/
#ifndef NOUUCP
#ifdef ACUCNTRL
VOID
acucntrl(flag,ttname) char *flag, *ttname; {
    char x[DEVNAMLEN+32], *device, *devname;

    if (strcmp(ttname,CTTNAM) == 0 || xlocal == 0) /* If not local, */
      return;				/* just return. */
    device = ((devname = xxlast(ttname,'/')) != NULL ? devname+1 : ttname);
    if (strncmp(device,"LCK..",4) == 0) device += 5;
    ckmakmsg(x,DEVNAMLEN+32,"/usr/lib/uucp/acucntrl ",flag," ",device);
    debug(F110,"called ",x,0);
    zsyscmd(x);
}
#endif /* ACUCNTRL */
#endif /* NOUUCP */

/*
  T T H F L O W  --  Set or Reset hardware flow control.

  This is an attempt to collect all hardware-flow-control related code
  into a single module.  Thanks to Rick Sladkey and John Kohl for lots of
  help here.  Overview:

  Hardware flow control is not supported in many UNIX implementions.  Even
  when it is supported, there is no (ha ha) "standard" for the programming
  interface.  In general, 4.3BSD and earlier (sometimes), 4.4BSD, System V,
  SunOS, AIX, etc, have totally different methods.  (And, not strictly
  relevant here, the programming interface often brings one only to a no-op
  in the device driver!)

  Among all these, we have two major types of APIs: those in which hardware
  flow control is determined by bits in the same termio/termios/sgtty mode
  word(s) that are used for controlling such items as CBREAK vs RAW mode, and
  which are also used by the ttvt(), ttpkt(), conbin(), and concb() routines
  for changing terminal modes.  And those that use entirely different
  mechanisms.

  In the first category, it is important that any change in the mode bits be
  reflected in the relevant termio(s)/sgtty structure, so that subsequent
  changes to that structure do not wipe out the effects of this routine.  That
  is why a pointer, attrs, to the appropriate structure is passed as a
  parameter to this routine.

  The second category should give us no worries, since any changes to hardware
  flow control accomplished by this routine should not affect the termio(s)/
  sgtty structures, and therefore will not be undone by later changes to them.

  The second argument, status, means to turn on hardware flow control if
  nonzero, and to turn it off if zero.

  Returns: 0 on apparent success, -1 on probable failure.
*/

/*
  The following business is for BSDI, where it was discovered that two
  separate bits, CCTS_OFLOW and CRTS_IFLOW, are used in hardware flow control,
  but CTRSCTS is defined (in <termios.h>) to be just CCTS_OFLOW rather both
  bits, so hwfc only works in one direction if you use CRTSCTS to control it.
  Other 4.4BSD-based Unixes such as FreeBSD 4.1, which use these two bits,
  define CRTSCTS correctly.
*/
#ifdef FIXCRTSCTS
#ifdef CRTSCTS
#ifdef CCTS_OFLOW
#ifdef CRTS_IFLOW
#undef CRTSCTS
#define CRTSCTS (CRTS_IFLOW|CCTS_OFLOW)
#endif /* CRTS_IFLOW */
#endif /* CCTS_OFLOW */
#endif /* CRTSCTS */
#endif /* FIXCRTSCTS */

static int
tthflow(flow, status, attrs)
    int flow,				/* Type of flow control (ckcdeb.h) */
    status;				/* Nonzero = turn it on */
					/* Zero = turn it off */
#ifdef BSD44ORPOSIX			/* POSIX or BSD44 */
    struct termios *attrs;
#else					/* System V */
#ifdef ATTSV
#ifdef ATT7300
#ifdef UNIX351M
/* AT&T UNIX 3.51m can set but not test for hardware flow control */
#define RTSFLOW CTSCD
#define CTSFLOW CTSCD
#endif /* ATT7300 */
#endif /* UNIX351M */
    struct termio *attrs;
#else					/* BSD, V7, etc */
    struct sgttyb *attrs;		/* sgtty info... */
#endif /* ATTSV */
#endif /* BSD44ORPOSIX */
/* tthflow */ {

    int x = 0;				/* tthflow() return code */

#ifdef Plan9
    return p9tthflow(flow, status);
#else

#ifndef OXOS				/* NOT Olivetti X/OS... */
/*
  For SunOS 4.0 and later in the BSD environment ...

  The declarations are copied and interpreted from the System V header files,
  so we don't actually have to pull in all the System V junk when building
  C-Kermit for SunOS in the BSD environment, which would be dangerous because
  having those symbols defined would cause us to take the wrong paths through
  the code.  The code in this section is used in both the BSD and Sys V SunOS
  versions.
*/
#ifdef SUNOS41
/*
  In SunOS 4.1 and later, we use the POSIX calls rather than ioctl calls
  because GNU CC uses different formats for the _IOxxx macros than regular CC;
  the POSIX forms work for both.  But the POSIX calls are not available in
  SunOS 4.0.
*/
#define CRTSCTS 0x80000000		/* RTS/CTS flow control */
#define TCSANOW 0			/* Do it now */

    struct termios {
	unsigned long c_iflag;		/* Input modes */
	unsigned long c_oflag;		/* Output modes */
	unsigned long c_cflag;		/* Control modes */
	unsigned long c_lflag;		/* Line discipline modes */
	char c_line;
	CHAR c_cc[17];
    };
    struct termios temp;

_PROTOTYP( int tcgetattr, (int, struct termios *) );
_PROTOTYP( int tcsetattr, (int, int, struct termios *) );
/*
  When CRTSCTS is set, SunOS won't do output unless both CTS and CD are
  asserted.  So we don't set CRTSCTS unless CD is up.  This should be OK,
  since we don't need RTS/CTS during dialing, and after dialing is complete,
  we should have CD.  If not, we still communicate, but without RTS/CTS.
*/
    int mflags;				/* Modem signal flags */

#ifdef NETCMD
    if (ttpipe) return(0);
#endif /* NETCMD */
#ifdef NETPTY
    if (ttpty) return(0);
#endif /* NETPTY */

    debug(F101,"tthflow SUNOS41 entry status","",status);
    if (!status) {			/* Turn hard flow off */
	if (tcgetattr(ttyfd, &temp) > -1 && /* Get device attributes */
	    (temp.c_cflag & CRTSCTS)) { /* Check for RTS/CTS */
	    temp.c_cflag &= ~CRTSCTS;	/* It's there, remove it */
	    x = tcsetattr(ttyfd,TCSANOW,&temp);
	}
    } else {				/* Turn hard flow on */
	if (ioctl(ttyfd,TIOCMGET,&mflags) > -1 && /* Get modem signals */
	    (mflags & TIOCM_CAR)) {		/* Check for CD */
	    debug(F100,"tthflow SunOS has CD","",0);
	    if (tcgetattr(ttyfd, &temp) > -1 && /* Get device attributes */
		!(temp.c_cflag & CRTSCTS)) { /* Check for RTS/CTS */
		temp.c_cflag |= CRTSCTS;	/* Not there, add it */
		x = tcsetattr(ttyfd,TCSANOW,&temp);
	    }
	} else {
	    x = -1;
	    debug(F100,"tthflow SunOS no CD","",0);
	}
    }
#else
#ifdef QNX
    struct termios temp;
#ifdef NETCMD
    if (ttpipe) return(0);
#endif /* NETCMD */
#ifdef NETPTY
    if (ttpty) return(0);
#endif /* NETPTY */
    debug(F101,"tthflow QNX entry status","",status);
    if (tcgetattr(ttyfd, &temp) > -1) {	/* Get device attributes */
	if (!status) {			/* Turn hard flow off */
	    if ((temp.c_cflag & (IHFLOW|OHFLOW)) == (IHFLOW|OHFLOW)) {
		temp.c_cflag &= ~(IHFLOW|OHFLOW); /* It's there, remove it */
		attrs->c_cflag &= ~(IHFLOW|OHFLOW);
		x = tcsetattr(ttyfd,TCSANOW,&temp);
	    }
	} else {			/* Turn hard flow on */
	    if ((temp.c_cflag & (IHFLOW|OHFLOW)) != (IHFLOW|OHFLOW)) {
		temp.c_cflag |= (IHFLOW|OHFLOW); /* Not there, add it */
		temp.c_iflag &= ~(IXON|IXOFF);   /* Bye to IXON/IXOFF */
		ttraw.c_lflag |= IEXTEN;         /* Must be on */
		x = tcsetattr(ttyfd,TCSANOW,&temp);
		attrs->c_cflag |= (IHFLOW|OHFLOW);
		attrs->c_iflag &= ~(IXON|IXOFF);
	    }
	}
    } else {
	x = -1;
	debug(F100, "tthflow QNX getattr fails", "", 0);
    }
#else
#ifdef POSIX_CRTSCTS
/*
  POSIX_CRTSCTS is defined in ckcdeb.h or on CC command line.
  Note: Do not assume CRTSCTS is a one-bit field!
*/
    struct termios temp;
#ifdef NETCMD
    if (ttpipe) return(0);
#endif /* NETCMD */
#ifdef NETPTY
    if (ttpty) return(0);
#endif /* NETPTY */
    debug(F101,"tthflow POSIX_CRTSCTS entry status","",status);
    errno = 0;
    x = tcgetattr(ttyfd, &temp);
    debug(F111,"tthflow POSIX_CRTSCTS tcgetattr",ckitoa(x),errno);
    errno = 0;
    if (x < 0) {
	x = -1;
    } else {
	if (!status) {			/* Turn hard flow off */
	    if (
#ifdef COMMENT
		/* This can fail because of sign extension */
		/* e.g. in Linux where it's Bit 31 */
		(temp.c_cflag & CRTSCTS) == CRTSCTS
#else
		(temp.c_cflag & CRTSCTS) != 0
#endif /* COMMENT */
		) {
		temp.c_cflag &= ~CRTSCTS; /* It's there, remove it */
		attrs->c_cflag &= ~CRTSCTS;
		x = tcsetattr(ttyfd,TCSANOW,&temp);
		debug(F111,"tthflow POSIX_CRTSCTS OFF tcsetattr",
		      ckitoa(x),errno);
	    } else {			/* John Dunlap 2010-01-26 */
		debug(F001,
		      "tthflow before forcing off attrs CRTSCTS",
		      "",
		      attrs->c_cflag&CRTSCTS
		      );
		attrs->c_cflag &= ~CRTSCTS; /* force it off if !status */
		debug(F001,
		      "tthflow after forcing off attrs CRTSCTS",
		      "",
		      attrs->c_cflag&CRTSCTS
		      );
		}
	} else {			/* Turn hard flow on */
	    if (
#ifdef COMMENT
		/* This can fail because of sign extension */
		(temp.c_cflag & CRTSCTS) != CRTSCTS
#else
		(temp.c_cflag & CRTSCTS) == 0
#endif /* COMMENT */
		) {
		temp.c_cflag |= CRTSCTS; /* Not there, add it */
		temp.c_iflag &= ~(IXON|IXOFF|IXANY); /* Bye to IXON/IXOFF */
		x = tcsetattr(ttyfd,TCSANOW,&temp);
		debug(F111,"tthflow POSIX_CRTSCTS ON tcsetattr",
		      ckitoa(x),errno);
		attrs->c_cflag |= CRTSCTS;
		attrs->c_iflag &= ~(IXON|IXOFF|IXANY);
	    }
	}
    }
#else
#ifdef SUNOS4
/*
  SunOS 4.0 (and maybe earlier?).  This code is dangerous because it
  prevents compilation with GNU gcc, which uses different formats for the
  _IORxxx macros than regular cc.  SunOS 4.1 and later can use the POSIX
  routines above, which work for both cc and gcc.
*/
#define TCGETS _IOR(T, 8, struct termios) /* Get modes into termios struct */
#define TCSETS _IOW(T, 9, struct termios) /* Set modes from termios struct */
#define CRTSCTS 0x80000000		  /* RTS/CTS flow control */

    struct termios {
	unsigned long c_iflag;		/* Input modes */
	unsigned long c_oflag;		/* Output modes */
	unsigned long c_cflag;		/* Control modes */
	unsigned long c_lflag;		/* Line discipline modes */
	char c_line;
	CHAR c_cc[17];
    };
    struct termios temp;
#ifdef NETCMD
    if (ttpipe) return(0);
#endif /* NETCMD */
#ifdef NETPTY
    if (ttpty) return(0);
#endif /* NETPTY */
    debug(F101,"tthflow entry status","",status);
    if (ioctl(ttyfd,TCGETS,&temp) > -1) { /* Get terminal modes. */
	if (status) {			/* Turn hard flow on */
	    temp.c_cflag |= CRTSCTS;	/* Add RTS/CTS to them. */
	    x = ioctl(ttyfd,TCSETS,&temp); /* Set them again. */
	    attrs->c_cflag |= CRTSCTS;	/* Add to global info. */
	} else {			/* Turn hard flow off */
	    temp.c_cflag &= ~CRTSCTS;
	    x = ioctl(ttyfd,TCSETS,&temp);
	    attrs->c_cflag &= ~CRTSCTS;
	}
    }
#else					/* Not SunOS 4.0 or later */
#ifdef AIXRS				/* IBM AIX RS/6000 */
#ifndef AIX41				/* But only pre-4.x == SVR4 */
#ifdef NETCMD
    if (ttpipe) return(0);
#endif /* NETCMD */
#ifdef NETPTY
    if (ttpty) return(0);
#endif /* NETPTY */
    if (status) {
	if ((x = ioctl(ttyfd, TXADDCD, "rts")) < 0 && errno != EBUSY)
	  debug(F100,"hardflow TXADDCD (rts) error", "", 0);
    } else {
	if ((x = ioctl(ttyfd, TXDELCD, "rts")) < 0 && errno != EINVAL)
	  debug(F100,"hardflow TXDELCD (rts) error", "", 0);
    }
#endif /* AIX41 */
#else					/* Not AIX RS/6000 */

#ifdef ATTSV				/* System V... */

#ifdef CK_SCOV5				/* SCO Open Server 5.0 */
#define CK_SCOUNIX
#else
#ifdef M_UNIX				/* SCO UNIX 3.2v4.x or earlier */
#define CK_SCOUNIX
#endif /* M_UNIX */
#endif /* CK_SCOV5 */

#ifdef SCO_FORCE_RTSXOFF
#ifdef CK_SCOUNIX			/* But not SCO OpenServer 5.0.4 */
#ifdef SCO_OSR504			/* or later... */
#undef CK_SCOUNIX
#endif /* SCO_OSR504 */
#endif /* CK_SCOUNIX */
#endif /* SCO_FORCE_RTSXOFF */

#ifdef CK_SCOUNIX
#ifdef POSIX
    struct termios temp;
#ifdef NETCMD
    if (ttpipe) return(0);
#endif /* NETCMD */
#ifdef NETPTY
    if (ttpty) return(0);
#endif /* NETPTY */
    debug(F101,"tthflow SCOUNIX POSIX entry status","",status);
    errno = 0;
    x = tcgetattr(ttyfd, &temp);
    debug(F111,"tthflow SCO UNIX POSIX tcgetattr",ckitoa(x),errno);
#else /* POSIX */
    struct termio temp;
#ifdef NETCMD
    if (ttpipe) return(0);
#endif /* NETCMD */
#ifdef NETPTY
    if (ttpty) return(0);
#endif /* NETPTY */
    debug(F101,"tthflow SCOUNIX non-POSIX entry status","",status);
    x = ioctl(ttyfd, TCGETA, &temp);
    debug(F111,"tthflow SCO UNIX non-POSIX TCGETA",ckitoa(x),errno);
#endif /* POSIX */
/*
  This is not really POSIX, since POSIX does not deal with hardware flow
  control, but we are using the POSIX APIs.  In fact, RTSFLOW and CTSFLOW
  are defined in termio.h, but within #ifndef _POSIX_SOURCE..#endif.  So
  let's try forcing their definitions here.
*/
#ifndef CTSFLOW
#define CTSFLOW 0020000
    debug(F101,"tthflow SCO defining CTSFLOW","",CTSFLOW);
#else
    debug(F101,"tthflow SCO CTSFLOW","",CTSFLOW);
#endif /* CTSFLOW */
#ifndef RTSFLOW
#define RTSFLOW 0040000
    debug(F101,"tthflow SCO defining RTSFLOW","",RTSFLOW);
#else
    debug(F101,"tthflow SCO RTSFLOW","",RTSFLOW);
#endif /* RTSFLOW */
#ifndef ORTSFL
#define ORTSFL 0100000
    debug(F101,"tthflow SCO defining ORTSFL","",ORTSFL);
#else
    debug(F101,"tthflow SCO ORTSFL","",ORTSFL);
#endif /* ORTSFL */

    if (x != -1) {
	if (status) {			/* Turn it ON */
	    temp.c_cflag |= RTSFLOW|CTSFLOW;
	    attrs->c_cflag |= RTSFLOW|CTSFLOW;
#ifdef ORTSFL
	    temp.c_cflag &= ~ORTSFL;
	    attrs->c_cflag &= ~ORTSFL;
#endif /* ORTSFL */
	    temp.c_iflag &= ~(IXON|IXOFF|IXANY);
	    attrs->c_iflag &= ~(IXON|IXOFF|IXANY);
	} else {			/* Turn it OFF */
#ifdef ORTSFL
	    temp.c_cflag &= ~(RTSFLOW|CTSFLOW|ORTSFL);
	    attrs->c_cflag &= ~(RTSFLOW|CTSFLOW|ORTSFL);
#else  /* ORTSFL */
	    temp.c_cflag &= ~(RTSFLOW|CTSFLOW);
	    attrs->c_cflag &= ~(RTSFLOW|CTSFLOW);
#endif /* ORTSFL */
	}
#ifdef POSIX
	x = tcsetattr(ttyfd, TCSADRAIN, &temp);
#else
	x = ioctl(ttyfd, TCSETA, &temp);
#endif /* POSIX */
	debug(F101,"tthflow SCO set modes","",x);
    }
#else /* Not SCO UNIX */
#ifdef NETCMD
    if (ttpipe) return(0);
#endif /* NETCMD */
#ifdef NETPTY
    if (ttpty) return(0);
#endif /* NETPTY */
    if (!status) {			/* Turn it OFF */
#ifdef RTSXOFF
	debug(F100,"tthflow ATTSV RTS/CTS OFF","",0);
	rctsx.x_hflag &= ~(RTSXOFF|CTSXON);
#ifdef TCSETX
	x = ioctl(ttyfd,TCSETX,&rctsx);
	debug(F101,"tthflow ATTSV TCSETX OFF","",x);
#else
	x = -1
	debug(F100,"tthflow TCSETX not defined","",0);
#endif /* TCSETX */
#else
	debug(F100,"tthflow ATTSV RTSXOFF not defined","",0);
#endif /* RTSXOFF */
#ifdef DTRXOFF
	debug(F100,"tthflow ATTSV DTR/CD OFF","",0);
	rctsx.x_hflag &= ~(DTRXOFF|CDXON);
	x = ioctl(ttyfd,TCSETX,&rctsx);
	debug(F101,"tthflow ATTSV DTRXOFF OFF","",x);
#else
	debug(F100,"tthflow ATTSV DTRXOFF not defined","",0);
#endif /* DTRXOFF */
    } else {				/* Turn it ON. */
	if (flow == FLO_RTSC) {	/* RTS/CTS Flow control... */
	    debug(F100,"tthflow ATTSV RTS/CTS ON","",0);
#ifdef RTSXOFF
	    /* This is the preferred way, according to SVID3 */
#ifdef TCGETX
	    x = ioctl(ttyfd,TCGETX,&rctsx);
	    debug(F101,"tthflow TCGETX","",x);
	    if (x > -1) {
		rctsx.x_hflag |= RTSXOFF | CTSXON;
		x = ioctl(ttyfd,TCSETX,&rctsx);
		debug(F100,"tthflow ATTSV ioctl","",x);
	    }
#else
	    debug(F100,"tthflow TCGETX not defined","",0);
	    x = -1
#endif /* TCGETX */
#else
	    debug(F100,"tthflow RTSXOFF not defined","",0);
	    x = -1;
#endif /* RTSXOFF */
	} else if (flow == FLO_DTRC) {	/* DTR/CD Flow control... */
	    debug(F100,"tthflow ATTSV DTR/CD ON","",0);
#ifdef DTRXOFF
	    /* This is straight out of SVID R4 */
	    if (ioctl(ttyfd,TCGETX,&rctsx) > -1) {
		rctsx.x_hflag &= ~(DTRXOFF|CDXON);
		x = ioctl(ttyfd,TCSETX,&rctsx);
	    }
#else
	    debug(F100,"tthflow ATTSV DTRXOFF not defined","",0);
	    x = -1;
#endif /* DTRXOFF */
	}
    }
#endif /* CK_SCOUNIX */

#else /* not System V... */

#ifdef CK_DTRCTS
#ifdef LDODTR
#ifdef LDOCTS
#ifdef NETCMD
    if (ttpipe) return(0);
#endif /* NETCMD */
#ifdef NETPTY
    if (ttpty) return(0);
#endif /* NETPTY */
    x = LDODTR | LDOCTS;		/* Found only on UTEK? */
    if (flow == FLO_DTRT && status) {	/* Use hardware flow control */
	if (lmodef) {
	    x = ioctl(ttyfd,TIOCLBIS,&x);
	    if (x < 0) {
	        debug(F100,"hardflow TIOCLBIS error","",0);
	    } else {
		lmodef++;
		debug(F100,"hardflow TIOCLBIS ok","",0);
	    }
	}
    } else {
	if (lmodef) {
	    x = ioctl(ttyfd,TIOCLBIC,&x);
	    if (x < 0) {
	        debug(F100,"hardflow TIOCLBIC error","",0);
	    } else {
		lmodef++;
		debug(F100,"hardflow TIOCLBIC ok","",0);
	    }
	}
    }
#endif /* LDODTR */
#endif /* LDOCTS */
#endif /* CK_DTRCTS */
#endif /* ATTSV */
#endif /* AIXRS */
#endif /* SUNOS4 */
#endif /* QNX */
#endif /* POSIX_CRTSCTS */
#endif /* SUNOS41 */

#else /* OXOS */

    struct termios temp;		/* Olivetti X/OS ... */

#ifdef NETCMD
    if (ttpipe) return(0);
#endif /* NETCMD */
#ifdef NETPTY
    if (ttpty) return(0);
#endif /* NETPTY */
    x = ioctl(ttyfd,TCGETS,&temp);
    if (x == 0) {
	temp.c_cflag &= ~(CRTSCTS|CDTRCTS|CBRKFLOW|CDTRDSR|CRTSDSR);
	if (status) {
	    switch (flow) {
	      case FLO_RTSC: temp.c_cflag |= CRTSCTS; /* RTS/CTS (hard) */
		break;
	      case FLO_DTRT: temp.c_cflag |= CDTRCTS; /* DTR/CTS (hard) */
		break;
	    }
	}
	x = ioctl(ttyfd,TCSETS,&temp);
    }
#endif /* OXOS */
    return(x);

#endif /* Plan9 */
}

/*  T T P K T  --  Condition the communication line for packets */
/*                 or for modem dialing */

/*
  If called with speed > -1, also set the speed.
  Returns 0 on success, -1 on failure.

  NOTE: the "xflow" parameter is supposed to be the currently selected
  type of flow control, but for historical reasons, this parameter is also
  used to indicate that we are dialing.  Therefore, when the true flow
  control setting is needed, we access the external variable "flow", rather
  than trusting our "xflow" argument.
*/
int
#ifdef CK_ANSIC
ttpkt(long speed, int xflow, int parity)
#else
ttpkt(speed,xflow,parity) long speed; int xflow, parity;
#endif /* CK_ANSIC */
/* ttpkt */ {
#ifndef NOLOCAL
    int s2;
    int s = -1;
#endif /* NOLOCAL */
    int x;
    extern int flow;			/* REAL flow-control setting */

    if (ttyfd < 0) return(-1);          /* Not open. */

    debug(F101,"ttpkt parity","",parity);
    debug(F101,"ttpkt xflow","",xflow);
    debug(F101,"ttpkt speed","",(int) speed);

    ttprty = parity;                    /* Let other tt functions see these. */
    ttspeed = speed;			/* Make global copy for this module */
    ttpmsk = ttprty ? 0177 : 0377;	/* Parity stripping mask */
#ifdef PARSENSE
    needpchk = ttprty ? 0 : 1;		/* Parity check needed? */
#else
    needpchk = 0;
#endif /* PARSENSE */

    debug(F101,"ttpkt ttpmsk","",ttpmsk);
    debug(F101,"ttpkt netconn","",netconn);

#ifdef NETCONN				/* No mode-changing for telnet */
    if (netconn) {
#ifdef TCPSOCKET
#ifdef TCP_NODELAY
        if (ttnet == NET_TCPB) {	/* But turn off Nagle */
            extern int tcp_nodelay;
            nodelay_sav = tcp_nodelay;
            no_delay(ttyfd,1);
        }
#endif /* TCP_NODELAY */
#ifdef TN_COMPORT
        if (istncomport()) {
            int rc = -1;
            if (tvtflg == 0 && speed == ttspeed && flow == ttflow
                 /* && ttcarr == curcarr */ ) {
                debug(F100,"ttpkt modes already set, skipping...","",0);
                return(0);		/* Already been called. */
            }
            if (flow != ttflow) {
                if ((rc = tnsetflow(flow)) < 0)
		  return(rc);
                ttflow = flow;
            }
            if (speed != ttspeed) {
                if (speed <= 0) 
		  speed = tnc_get_baud();
                else if ((rc = tnc_set_baud(speed)) < 0)
		  return(rc);
                ttspeed = speed;
            }
            tnc_set_datasize(8);
	    tnc_set_stopsize(stopbits);

#ifdef HWPARITY
            if (hwparity) {
                switch (hwparity) {
		  case 'e':			/* Even */
                    debug(F100,"ttres 8 bits + even parity","",0);
                    tnc_set_parity(3);
                    break;
		  case 'o':			/* Odd */
                    debug(F100,"ttres 8 bits + odd parity","",0);
                    tnc_set_parity(2);
                    break;
		  case 'm':			/* Mark */
                    debug(F100,"ttres 8 bits + invalid parity: mark","",0);
                    tnc_set_parity(4);
                    break;
		  case 's':			/* Space */
                    debug(F100,"ttres 8 bits + invalid parity: space","",0);
                    tnc_set_parity(5);
                    break;
                }
            } else 
#endif /* HWPARITY */
	    {
                tnc_set_parity(1);              /* None */
            }
            tvtflg = 0;
            return(0);
        }
#endif /* TN_COMPORT */
#endif /* TCPSOCKET */
        tvtflg = 0;
        return(0);
    }
#endif /* NETCONN */
#ifdef NETCMD
    if (ttpipe) return(0);
#endif /* NETCMD */
#ifdef NETPTY
    if (ttpty) return(0);
#endif /* NETPTY */

#ifndef Plan9
    if (ttfdflg && !isatty(ttyfd)) return(0);
#endif /* Plan9 */

#ifdef COHERENT
#define SVORPOSIX
#endif /* COHERENT */

#ifndef SVORPOSIX			/* Berkeley, V7, etc. */
#ifdef LPASS8
/*
 For some reason, with BSD terminal drivers, you can't set FLOW to XON/XOFF
 after having previously set it to NONE without closing and reopening the
 device.  Unless there's something I overlooked below...
*/
    if (ttflow == FLO_NONE && flow == FLO_XONX && xlocal == 0) {
	debug(F101,"ttpkt executing horrible flow kludge","",0);
	ttclos(0);			/* Close it */
	x = 0;
	ttopen(ttnmsv,&x,ttmdm,0);	/* Open it again */
    }
#endif /* LPASS8 */
#endif /* SVORPOSIX */

#ifdef COHERENT				/* This must be vestigial since we */
#undef SVORPOSIX			/* reverse it a few lines below... */
#endif /* COHERENT */

    if (xflow != FLO_DIAL && xflow != FLO_DIAX)
      ttflow = xflow;			/* Now make this available too. */

#ifndef NOLOCAL
    if (xlocal) {
	s2 = (int) (speed / 10L);	/* Convert bps to cps */
	debug(F101,"ttpkt calling ttsspd","",s2);
	s = ttsspd(s2);			/* Check and set the speed */
	debug(F101,"ttpkt ttsspd result","",s);
 	carrctl(&ttraw, xflow != FLO_DIAL /* Carrier control */
		&& (ttcarr == CAR_ON || (ttcarr == CAR_AUT && ttmdm != 0)));
	tvtflg = 0;			/* So ttvt() will work next time */
    }
#endif /* NOLOCAL */

#ifdef COHERENT
#define SVORPOSIX
#endif /* COHERENT */

#ifndef SVORPOSIX			/* BSD section */
    if (flow == FLO_RTSC ||		/* Hardware flow control */
	flow == FLO_DTRC ||
	flow == FLO_DTRT) {
	tthflow(flow, 1, &ttraw);
	debug(F100,"ttpkt hard flow, TANDEM off, RAW on","",0);
	ttraw.sg_flags &= ~TANDEM;	/* Turn off software flow control */
	ttraw.sg_flags |= RAW;		/* Enter raw mode */
    } else if (flow == FLO_NONE) {	/* No flow control */
	debug(F100,"ttpkt no flow, TANDEM off, RAW on","",0);
	ttraw.sg_flags &= ~TANDEM;	/* Turn off software flow control */
	tthflow(flow, 0, &ttraw);	/* Turn off any hardware f/c too */
	ttraw.sg_flags |= RAW;		/* Enter raw mode */
    } else if (flow == FLO_KEEP) {	/* Keep device's original setting */
	debug(F100,"ttpkt keeping original TANDEM","",0);
	ttraw.sg_flags &= ~TANDEM;
	ttraw.sg_flags |= (ttold.sg_flags & TANDEM);
	/* NOTE: We should also handle hardware flow control here! */
    }

/* SET FLOW XON/XOFF is in effect, or SET FLOW KEEP resulted in Xon/Xoff */

    if ((flow == FLO_XONX) || (ttraw.sg_flags & TANDEM)) {
	debug(F100,"ttpkt turning on TANDEM","",0);
	ttraw.sg_flags |= TANDEM;	/* So ask for it. */

#ifdef LPASS8				/* Can pass 8-bit data through? */
/* If the LPASS8 local mode is available, then flow control can always  */
/* be used, even if parity is none and we are transferring 8-bit data.  */
/* But we only need to do all this if Xon/Xoff is requested. */
/* BUT... this tends not to work through IP or LAT connections, terminal */
/* servers, telnet, rlogin, etc, so it is currently disabled. */
	x = LPASS8;			/* If LPASS8 defined, then */
	debug(F100,"ttpkt executing LPASS8 code","",0);
	if (lmodef) {			/* TIOCLBIS must be too. */
	    x = ioctl(ttyfd,TIOCLBIS,&x); /* Try to set LPASS8. */
	    if (x < 0) {
		debug(F100,"ttpkt TIOCLBIS error","",0);
	    } else {
		lmodef++;
		debug(F100,"ttpkt TIOCLBIS ok","",0);
	    }
	}
/*
 But if we use LPASS8 mode, we must explicitly turn off
 terminal interrupts of all kinds.
*/
#ifdef TIOCGETC				/* Not rawmode, */
	if (tcharf && (xlocal == 0)) {	/* must turn off */
	    tchnoi.t_intrc = -1;	/* interrupt character */
	    tchnoi.t_quitc = -1;	/* and quit character. */
	    tchnoi.t_startc = 17;	/* Make sure xon */
	    tchnoi.t_stopc = 19;	/* and xoff not ignored. */
#ifndef NOBRKC
	    tchnoi.t_eofc = -1;		/* eof character. */
	    tchnoi.t_brkc = -1;		/* brk character. */
#endif /* NOBRKC */
	    if (ioctl(ttyfd,TIOCSETC,&tchnoi) < 0) {
		debug(F100,"ttpkt TIOCSETC failed","",0);
	    } else {
		tcharf = 1;
		debug(F100,"ttpkt TIOCSETC ok","",0);
	    }
#ifdef COMMENT
/* only for paranoid debugging */
	    if (tcharf) {
		struct tchars foo;
		char tchbuf[100];
		ioctl(0,TIOCGETC,&foo);
		sprintf(tchbuf,
		    "intr=%d,quit=%d, start=%d, stop=%d, eof=%d, brk=%d",
		    foo.t_intrc, foo.t_quitc, foo.t_startc,
		    foo.t_stopc, foo.t_eofc,  foo.t_brkc);
		debug(F110,"ttpkt chars",tchbuf,0);
	    }
#endif /* COMMENT */
	}
	ttraw.sg_flags |= CBREAK;	/* Needed for unknown reason */
#endif /* TIOCGETC */

/* Prevent suspend during packet mode */
#ifdef TIOCGLTC				/* Not rawmode, */
	if (ltcharf && (xlocal == 0)) {	/* must turn off */
	    ltchnoi.t_suspc = -1;	/* suspend character */
	    ltchnoi.t_dsuspc = -1;	/* and delayed suspend character */
	    if (ioctl(ttyfd,TIOCSLTC,&tchnoi) < 0) {
		debug(F100,"ttpkt TIOCSLTC failed","",0);
	    } else {
		ltcharf = 1;
		debug(F100,"ttpkt TIOCSLTC ok","",0);
	    }
	}
#endif /* TIOCGLTC */

#else /* LPASS8 not defined */

/* Previously, BSD-based implementations always */
/* used rawmode for packets.  Now, we use rawmode only if parity is NONE. */
/* This allows the flow control requested above to actually work, but only */
/* if the user asks for parity (which also means they get 8th-bit quoting). */

	if (parity) {			/* If parity, */
	    ttraw.sg_flags &= ~RAW;	/* use cooked mode */
#ifdef COMMENT
/* WHY??? */
	    if (xlocal)
#endif /* COMMENT */
	      ttraw.sg_flags |= CBREAK;
	    debug(F101,"ttpkt cooked, cbreak, parity","",parity);
#ifdef TIOCGETC				/* Not rawmode, */
	    if (tcharf && (xlocal == 0)) { /* must turn off */
		tchnoi.t_intrc = -1;	/* interrupt character */
		tchnoi.t_quitc = -1;	/* and quit character. */
		tchnoi.t_startc = 17;	/* Make sure xon */
		tchnoi.t_stopc = 19;	/* and xoff not ignored. */
#ifndef NOBRKC
		tchnoi.t_eofc = -1;	/* eof character. */
		tchnoi.t_brkc = -1;	/* brk character. */
#endif /* NOBRKC */
		if (ioctl(ttyfd,TIOCSETC,&tchnoi) < 0) {
		    debug(F100,"ttpkt TIOCSETC failed","",0);
		} else {
		    tcharf = 1;
		    debug(F100,"ttpkt TIOCSETC ok","",0);
		}
	    }
#endif /* TIOCGETC */
#ifdef TIOCGLTC				/* Not rawmode, */
/* Prevent suspend during packet mode */
	    if (ltcharf && (xlocal == 0)) { /* must turn off */
		ltchnoi.t_suspc = -1;	/* suspend character */
		ltchnoi.t_dsuspc = -1;	/* and delayed suspend character */
		if (ioctl(ttyfd,TIOCSLTC,&tchnoi) < 0) {
		    debug(F100,"ttpkt TIOCSLTC failed","",0);
		} else {
		    ltcharf = 1;
		    debug(F100,"ttpkt TIOCSLTC ok","",0);
		}
	    }
#endif /* TIOCGLTC */
	} else {			/* If no parity, */
	    ttraw.sg_flags |= RAW;	/* must use 8-bit raw mode. */
	    debug(F101,"ttpkt setting rawmode, parity","",parity);
	}
#endif /* LPASS8 */
    } /* End of Xon/Xoff section */

    /* Don't echo, don't map CR to CRLF on output, don't fool with case */
#ifdef LCASE
    ttraw.sg_flags &= ~(ECHO|CRMOD|LCASE);
#else
    ttraw.sg_flags &= ~(ECHO|CRMOD);
#endif /* LCASE */

#ifdef TOWER1
    ttraw.sg_flags &= ~ANYP;            /* Must set this on old Towers */
#endif /* TOWER1 */

#ifdef BELLV10
    if (ioctl(ttyfd,TIOCSETP,&ttraw) < 0) /* Set the new modes. */
      return(-1);
#else
    errno = 0;
    if (stty(ttyfd,&ttraw) < 0) {       /* Set the new modes. */
        debug(F101,"ttpkt stty failed","",errno);
        return(-1);
    }
#endif /* BELLV10 */
    debug(F100,"ttpkt stty ok","",0);

#ifdef sony_news
    x = xlocal ? km_ext : km_con;	/* Put line in ASCII mode. */
    if (x != -1) {			/* Make sure we know original modes. */
	x &= ~KM_TTYPE;
	x |= KM_ASCII;
	if (ioctl(ttyfd,TIOCKSET, &x) < 0) {
	    perror("ttpkt can't set ASCII mode");
	    debug(F101,"ttpkt error setting ASCII mode","",x);
	    return(-1);
	}
    }
    debug(F100,"ttpkt set ASCII mode ok","",0);
#endif /* sony_news */

    if (xlocal == 0) {			/* Turn this off so we can read */
	signal(SIGINT,SIG_IGN);		/* Ctrl-C chars typed at console */
	sigint_ign = 1;
    }
    tvtflg = 0;				/* So ttvt() will work next time */
    debug(F100,"ttpkt success","",0);
    return(0);

#endif /* Not ATTSV or POSIX */

/* AT&T UNIX and POSIX */

#ifdef COHERENT
#define SVORPOSIX
#endif /* COHERENT */

#ifdef SVORPOSIX
    if (flow == FLO_XONX) {		/* Xon/Xoff */
	ttraw.c_iflag |= (IXON|IXOFF);
	tthflow(flow, 0, &ttraw);
    } else if (flow == FLO_NONE) {	/* None */
	/* NOTE: We should also turn off hardware flow control here! */
	ttraw.c_iflag &= ~(IXON|IXOFF);
	tthflow(flow, 0, &ttraw);
    } else if (flow == FLO_KEEP) {	/* Keep */
	ttraw.c_iflag &= ~(IXON|IXOFF);	/* Turn off Xon/Xoff flags */
	ttraw.c_iflag |= (ttold.c_iflag & (IXON|IXOFF)); /* OR in old ones */
	/* NOTE: We should also handle hardware flow control here! */
#ifdef POSIX_CRTSCTS
/* In Linux case, we do this, which is unlikely to be portable */
        ttraw.c_cflag &= ~CRTSCTS;	/* Turn off RTS/CTS flag */
        ttraw.c_cflag |= (ttold.c_cflag & CRTSCTS); /* OR in old one */
#endif /* POSIX_CRTSCTS */
    } else if (flow == FLO_RTSC ||	/* Hardware */
	       flow == FLO_DTRC ||
	       flow == FLO_DTRT) {
	ttraw.c_iflag &= ~(IXON|IXOFF);	/* (190) */
	tthflow(flow, 1, &ttraw);
    }
    ttraw.c_lflag &= ~(ICANON|ECHO);
    ttraw.c_lflag &= ~ISIG;		/* Do NOT check for interrupt chars */

#ifndef OXOS
#ifdef QNX
    if (flow != FLO_RTSC && flow != FLO_DTRC && flow != FLO_DTRT)
#endif /* QNX */
#ifndef COHERENT
      ttraw.c_lflag &= ~IEXTEN;		/* Turn off ^O/^V processing */
#endif /* COHERENT */
#else /* OXOS */
    ttraw.c_cc[VDISCARD] = ttraw.c_cc[VLNEXT] = CDISABLE;
#endif /* OXOS */
    ttraw.c_lflag |= NOFLSH;		/* Don't flush */
    ttraw.c_iflag |= IGNPAR;		/* Ignore parity errors */
#ifdef ATTSV
#ifdef BSD44
    ttraw.c_iflag &= ~(IGNBRK|INLCR|IGNCR|ICRNL|INPCK|ISTRIP|IXANY);
#else
    ttraw.c_iflag &= ~(IGNBRK|INLCR|IGNCR|ICRNL|IUCLC|INPCK|ISTRIP|IXANY);
#endif /* BSD44 */
#else /* POSIX */
    ttraw.c_iflag &= ~(IGNBRK|INLCR|IGNCR|ICRNL|INPCK|ISTRIP);
#endif /* ATTSV */
    ttraw.c_oflag &= ~OPOST;
    ttraw.c_cflag &= ~(CSIZE);
    ttraw.c_cflag |= (CS8|CREAD|HUPCL);

#ifdef CSTOPB
    if (xlocal) {
	if (stopbits == 2) {
	    ttraw.c_cflag |= CSTOPB;	/* 2 stop bits */
	    debug(F100,"ttpkt 2 stopbits","",0);
	} else if (stopbits == 1) {
	    ttraw.c_cflag &= ~(CSTOPB);	/* 1 stop bit */
	    debug(F100,"ttpkt 1 stopbit","",0);
	}
    }
#endif /* CSTOPB */

#ifdef HWPARITY
    if (hwparity && xlocal) {		/* Hardware parity */
	ttraw.c_cflag |= PARENB;	/* Enable parity */
#ifdef COMMENT
/* Uncomment this only if needed -- I don't think it is */
	ttraw.c_cflag &= ~(CSIZE);	/* Clear out character-size mask */
	ttraw.c_cflag |= CS8;		/* And set it to 8 */
#endif /* COMMENT */
#ifdef IGNPAR
	ttraw.c_iflag |= IGNPAR;	/* Don't discard incoming bytes */
	debug(F100,"ttpkt IGNPAR","",0); /* that have parity errors */
#endif /* IGNPAR */
	switch (hwparity) {
	  case 'e':			/* Even */
	    ttraw.c_cflag &= ~(PARODD);
	    debug(F100,"ttpkt 8 bits + even parity","",0);
	    break;
	  case 'o':			/* Odd */
	    ttraw.c_cflag |= PARODD;
	    debug(F100,"ttpkt 8 bits + odd parity","",0);
	    break;
	  case 'm':			/* Mark */
	  case 's':			/* Space */
	    /* PAREXT is mentioned in SVID but the details are not given. */
	    /* PAREXT is not included in POSIX ISO/IEC 9945-1. */
	    debug(F100,"ttpkt 8 bits + invalid parity","",0);
	    break;
	}
    } else {				/* We handle parity ourselves */
#endif /* HWPARITY */
	ttraw.c_cflag &= ~(PARENB);	/* Don't enable parity */
#ifdef HWPARITY
    }
#endif /* HWPARITY */

#ifdef IX370
    ttraw.c_cc[4] = 48;  /* So Series/1 doesn't interrupt on every char */
    ttraw.c_cc[5] = 1;
#else
#ifndef VEOF				/* for DGUX this is VEOF, not VMIN */
    ttraw.c_cc[4] = 1;   /* [VMIN]  return max of this many characters or */
#else
#ifndef OXOS
#ifdef VMIN
    ttraw.c_cc[VMIN] = 1;
#endif /* VMIN */
#else /* OXOS */
    ttraw.c_min = 1;
#endif /* OXOS */
#endif /* VEOF */
#ifndef VEOL				/* for DGUX this is VEOL, not VTIME */
    ttraw.c_cc[5] = 0;	 /* [VTIME] when this many secs/10 expire w/no input */
#else
#ifndef OXOS
#ifdef VTIME
    ttraw.c_cc[VTIME] = 0;
#endif /* VTIME */
#else /* OXOS */
    ttraw.c_time = 0;
#endif /* OXOS */
#endif /* VEOL */
#endif /* IX370 */

#ifdef VINTR				/* Turn off interrupt character */
    if (xlocal == 0)			/* so ^C^C can break us out of */
      ttraw.c_cc[VINTR] = 0;		/* packet mode. */
#endif /* VINTR */

#ifdef Plan9
    if (p9ttyparity('n') < 0)
	return -1;
#else
#ifdef BSD44ORPOSIX
    errno = 0;
#ifdef BEOSORBEBOX
    ttraw.c_cc[VMIN] = 0;		/* DR7 can only poll. */
#endif /* BEOSORBEBOX */

#define TESTING234
#ifdef TESTING234
    if (1) {
	debug(F100,"ttpkt TESTING234 rawmode","",0);

	/* iflags */
	ttraw.c_iflag &= ~(PARMRK|ISTRIP|BRKINT|INLCR|IGNCR|ICRNL);
	ttraw.c_iflag &= ~(INPCK|IGNPAR|IXON|IXOFF);
	ttraw.c_iflag |= IGNBRK;
#ifdef IMAXBEL
	ttraw.c_iflag &= ~IMAXBEL;
#endif	/* IMAXBEL */
#ifdef IXANY
	ttraw.c_iflag &= ~IXANY;
#endif	/* IXANY */
#ifdef IUCLC
	ttraw.c_iflag &= ~IUCLC;
#endif /* IUCLC */

	/* oflags */
	ttraw.c_oflag &= ~OPOST;
#ifdef OXTABS
	ttraw.c_oflag &= ~OXTABS;
#endif /* OXTABS */
#ifdef ONOCR
	ttraw.c_oflag &= ~ONOCR;
#endif /* ONOCR */
#ifdef ONLRET
	ttraw.c_oflag &= ~ONLRET;
#endif /* ONLRET */
#ifdef ONLCR
	ttraw.c_oflag &= ~ONLCR;
#endif /* ONLCR */

	/* lflags */
	ttraw.c_lflag &= ~ECHO;
#ifdef ECHOE
	ttraw.c_lflag &= ~ECHOE;
#endif /* ECHOE */
#ifdef ECHONL
	ttraw.c_lflag &= ~ECHONL;
#endif /* ECHONL */
#ifdef ECHOPRT
	ttraw.c_lflag &= ~ECHOPRT;
#endif /* ECHOPRT */
#ifdef ECHOKE
	ttraw.c_lflag &= ~ECHOKE;
#endif /* ECHOKE */
#ifdef ECHOCTL
	ttraw.c_lflag &= ~ECHOCTL;
#endif /* ECHOCTL */
#ifdef ALTWERASE
	ttraw.c_lflag &= ~ALTWERASE;
#endif /* ALTWERASE */
#ifdef EXTPROC
	ttraw.c_lflag &= ~EXTPROC;
#endif /* EXTPROC */
	ttraw.c_lflag &= ~(ICANON|ISIG|IEXTEN|TOSTOP|FLUSHO|PENDIN);
#ifdef NOKERNINFO
	ttraw.c_lflag |= NOKERNINFO;
#endif	/* NOKERNINFO */
	/* ttraw.c_lflag |= NOFLSH; */
	ttraw.c_lflag &= ~NOFLSH;

	/* cflags */
	ttraw.c_cflag &= ~(CSIZE|PARENB|PARODD);
	ttraw.c_cflag |= CS8|CREAD;
#ifdef VMIN
	ttraw.c_cc[VMIN] = 1;		/* Supposedly needed for AIX */
#endif	/* VMIN */

    }
#endif /* TESTING234 */

    debug(F100,"ttpkt calling tcsetattr(TCSETAW)","",0);
    x = tcsetattr(ttyfd,TCSADRAIN,&ttraw);
    debug(F101,"ttpkt BSD44ORPOSIX tcsetattr","",x);
    if (x < 0) {
	debug(F101,"ttpkt BSD44ORPOSIX tcsetattr errno","",errno);
        return(-1);
    }
#else /* BSD44ORPOSIX */
    x = ioctl(ttyfd,TCSETAW,&ttraw);
    debug(F101,"ttpkt ATTSV ioctl TCSETAW","",x);
    if (x < 0) {  /* set new modes . */
	debug(F101,"ttpkt ATTSV ioctl TCSETAW errno","",errno);
        return(-1);
    }
#endif /* BSD44ORPOSIX */
#endif /* Plan9 */
    tvtflg = 0;
    debug(F100,"ttpkt ok","",0);
    return(0);
#endif /* ATTSV */

#ifdef COHERENT
#undef SVORPOSIX
#endif /* COHERENT */

}

/*  T T S E T F L O W  --  Set flow control immediately.  */

#ifdef COHERENT
#define SVORPOSIX
#endif /* COHERENT */

int
ttsetflow(flow) int flow; {
    if (ttyfd < 0)			/* A channel must be open */
      return(-1);

    debug(F101,"ttsetflow flow","",flow);

#ifdef TN_COMPORT
    if (netconn && istncomport()) {
	debug(F101,"ttsetflow net modem","",ttmdm);
	return(tnsetflow(flow));
    }
#endif /* TN_COMPORT */
#ifdef NETCMD
    if (ttpipe) return(0);
#endif /* NETCMD */
#ifdef NETPTY
    if (ttpty) return(0);
#endif /* NETPTY */

#ifdef COMMENT
    /* This seems to hurt... */
    if (flow == FLO_KEEP)
      return(0);
#endif /* COMMENT */

    if (flow == FLO_RTSC ||		/* Hardware flow control... */
	flow == FLO_DTRC ||
	flow == FLO_DTRT) {
	tthflow(flow, 1, &ttraw);
#ifndef SVORPOSIX
	ttraw.sg_flags &= ~TANDEM;	/* Turn off software flow control */
#else
	ttraw.c_iflag &= ~(IXON|IXOFF);
#endif /* SVORPOSIX */

    } else if (flow == FLO_XONX) {	/* Xon/Xoff... */

#ifndef SVORPOSIX
	ttraw.sg_flags |= TANDEM;
#else
	ttraw.c_iflag |= (IXON|IXOFF);
#endif /* SVORPOSIX */
	tthflow(FLO_RTSC, 0, &ttraw);	/* Turn off hardware flow control */

    } else if (flow == FLO_NONE) {	/* No flow control */

#ifndef SVORPOSIX
	ttraw.sg_flags &= ~TANDEM;	/* Turn off software flow control */
#else
	ttraw.c_iflag &= ~(IXON|IXOFF);
#endif /* SVORPOSIX */
	tthflow(FLO_RTSC, 0, &ttraw);	/* Turn off any hardware f/c too */
    }

/* Set the new modes... */

#ifndef SVORPOSIX			/* BSD and friends */
#ifdef BELLV10
    if (ioctl(ttyfd,TIOCSETP,&ttraw) < 0)
      return(-1);
#else
#ifndef MINIX2
    if (stty(ttyfd,&ttraw) < 0)
      return(-1);
#endif /* MINIX2 */
#endif /* BELLV10 */
#else
#ifdef BSD44ORPOSIX			/* POSIX */
    if (tcsetattr(ttyfd,TCSADRAIN,&ttraw) < 0)
      return(-1);
#else					/* System V */
    if (ioctl(ttyfd,TCSETAW,&ttraw) < 0)
      return(-1);
#endif /* BSD44ORPOSIX */
#endif /* SVORPOSIX */
    return(0);
}
#ifdef COHERENT
#undef SVORPOSIX
#endif /* COHERENT */

/*  T T V T -- Condition communication device for use as virtual terminal. */

int
#ifdef CK_ANSIC
ttvt(long speed, int flow)
#else
ttvt(speed,flow) long speed; int flow;
#endif /* CK_ANSIC */
/* ttvt */ {
    int s, s2, x;

    debug(F101,"ttvt ttyfd","",ttyfd);
    debug(F101,"ttvt tvtflg","",tvtflg);
    debug(F111,"ttvt speed",ckitoa(ttspeed),speed);
    debug(F111,"ttvt flow",ckitoa(ttflow),flow);
    debug(F111,"ttvt curcarr",ckitoa(ttcarr),curcarr);

/* Note: NetBSD and maybe other BSD44s have cfmakeraw() */
/* Maybe it would be simpler to use it... */

    ttpmsk = 0xff;
#ifdef NOLOCAL
    return(conbin((char)escchr));
#else
    if (ttyfd < 0) {			/* Not open. */
	if (ttchk() < 0)
	  return(-1);
	else				/* But maybe something buffered. */
	  return(0);
    }
#ifdef NETCMD
    if (ttpipe) return(0);
#endif /* NETCMD */
#ifdef NETPTY
    if (ttpty) return(0);
#endif /* NETPTY */
#ifdef NETCONN
    if (netconn) {
#ifdef TCPSOCKET
#ifdef TCP_NODELAY
        {
	    extern int tcp_nodelay;
	    if (ttnet == NET_TCPB) {
		if (nodelay_sav > -1) {
		    no_delay(ttyfd,nodelay_sav);
		    nodelay_sav = -1;
		}
	    }
        }
#endif /* TCP_NODELAY */
#ifdef TN_COMPORT
        if (istncomport()) {
            int rc = -1;
            if (tvtflg != 0 && speed == ttspeed && flow == ttflow
                 /* && ttcarr == curcarr */ ) {
                debug(F100,"ttvt modes already set, skipping...","",0);
                return(0);			/* Already been called. */
            }
            if (flow != ttflow) {
                if ((rc = tnsetflow(flow)) < 0)
		  return(rc);
                ttflow = flow;
            }
            if (speed != ttspeed) {
                if (speed <= 0) 
		  speed = tnc_get_baud();
                else if ((rc = tnc_set_baud(speed)) < 0)
		  return(rc);
                ttspeed = speed;
            }
            tnc_set_datasize(8);
	    tnc_set_stopsize(stopbits);

#ifdef HWPARITY
            if (hwparity) {
                switch (hwparity) {
		  case 'e':		/* Even */
                    debug(F100,"ttres 8 bits + even parity","",0);
                    tnc_set_parity(3);
                    break;
		  case 'o':		/* Odd */
                    debug(F100,"ttres 8 bits + odd parity","",0);
                    tnc_set_parity(2);
                    break;
		  case 'm':		/* Mark */
                    debug(F100,"ttres 8 bits + invalid parity: mark","",0);
                    tnc_set_parity(4);
                    break;
		  case 's':		/* Space */
                    debug(F100,"ttres 8 bits + invalid parity: space","",0);
                    tnc_set_parity(5);
                    break;
                }
            } else
#endif /* HWPARITY */
            {
                tnc_set_parity(1);	/* None */
            }
            tvtflg = 1;
            return(0);
        }
#endif /* TN_COMPORT */
#endif /* TCPSOCKET */
	tvtflg = 1;			/* Network connections... */
	debug(F100,"ttvt network connection, skipping...","",0);
	return(0);			/* ... require no special setup */
    }
#endif /* NETCONN */

    if (tvtflg != 0 && speed == ttspeed && flow == ttflow
	/* && ttcarr == curcarr */ )
      {
	  debug(F100,"ttvt modes already set, skipping...","",0);
	  return(0);			/* Already been called. */
      }

    if (ttfdflg
#ifndef Plan9
	&& !isatty(ttyfd)
#endif /* Plan9 */
	) {
	debug(F100,"ttvt using external fd, skipping...","",0);
	return(0);
    }

    debug(F100,"ttvt setting modes...","",0);

    if (xlocal) {			/* For external lines... */
	s2 = (int) (speed / 10L);
	s = ttsspd(s2);			/* Check/set the speed */
	carrctl(&tttvt, flow != FLO_DIAL /* Do carrier control */
		&& (ttcarr == CAR_ON || (ttcarr == CAR_AUT && ttmdm != 0)));
    } else
      s = s2 = -1;

#ifdef COHERENT
#define SVORPOSIX
#endif /* COHERENT */

#ifndef SVORPOSIX
    /* Berkeley, V7, etc */
    if (flow == FLO_RTSC ||		/* Hardware flow control */
	flow == FLO_DTRC ||
	flow == FLO_DTRT) {
	tthflow(flow, 1, &tttvt);
	debug(F100,"ttvt hard flow, TANDEM off","",0);
	tttvt.sg_flags &= ~TANDEM;	/* Turn off software flow control */
    } else if (flow == FLO_XONX) {	/* Xon/Xoff flow control */
	debug(F100,"ttvt TANDEM on","",0);
	tttvt.sg_flags |= TANDEM;	/* Ask for it. */
	tthflow(flow, 0, &tttvt);	/* Turn off hardware f/c */
    } else if (flow == FLO_NONE) {
	debug(F100,"ttvt no flow, TANDEM off, RAW on","",0);
	tttvt.sg_flags &= ~TANDEM;	/* Turn off software flow control */
	tthflow(flow, 0, &tttvt);	/* Turn off any hardware f/c too */
	tttvt.sg_flags |= RAW;		/* Enter raw mode */
    } else if (flow == FLO_KEEP) {	/* Keep device's original setting */
	debug(F100,"ttvt keeping original TANDEM","",0);
	tttvt.sg_flags &= ~TANDEM;
	tttvt.sg_flags |= (ttold.sg_flags & TANDEM);
	/* NOTE: We should also handle hardware flow control here! */
    }
    tttvt.sg_flags |= RAW;              /* Raw mode in all cases */
#ifdef TOWER1
    tttvt.sg_flags &= ~(ECHO|ANYP);     /* No echo or parity */
#else
    tttvt.sg_flags &= ~ECHO;            /* No echo */
#endif /* TOWER1 */

#ifdef BELLV10
    if (ioctl(ttyfd,TIOCSETP,&tttvt) < 0) /* Set the new modes */
      return(-1);
#else
    if (stty(ttyfd,&tttvt) < 0)		/* Set the new modes */
      return(-1);
#endif /* BELLV10 */

#else /* It is ATTSV or POSIX */

    if (flow == FLO_XONX) {		/* Software flow control */
	tttvt.c_iflag |= (IXON|IXOFF);	/* On if requested. */
	tthflow(flow, 0, &tttvt);	/* Turn off hardware f/c */
	debug(F100,"ttvt SVORPOSIX flow XON/XOFF","",0);
    } else if (flow == FLO_NONE) {	/* NONE */
	tttvt.c_iflag &= ~(IXON|IXOFF);	/* Turn off Xon/Xoff */
	tthflow(flow, 0, &tttvt);	/* Turn off hardware f/c */
	debug(F100,"ttvt SVORPOSIX flow NONE","",0);
    } else if (flow == FLO_KEEP) {
	tttvt.c_iflag &= ~(IXON|IXOFF);	/* Turn off Xon/Xoff flags */
	tttvt.c_iflag |= (ttold.c_iflag & (IXON|IXOFF)); /* OR in old ones */
#ifdef POSIX_CRTSCTS
        tttvt.c_cflag &= ~CRTSCTS;	/* Turn off RTS/CTS flag */
        tttvt.c_cflag |= (ttold.c_cflag & CRTSCTS); /* OR in old one */
#endif /* POSIX_CRTSCTS */
	debug(F100,"ttvt SVORPOSIX flow KEEP","",0);
    } else if (flow == FLO_RTSC ||	/* Hardware flow control */
	       flow == FLO_DTRC ||
	       flow == FLO_DTRT) {
	tttvt.c_iflag &= ~(IXON|IXOFF);	/* (196) */
	tthflow(flow, 1, &tttvt);
	debug(F100,"ttvt SVORPOSIX flow HARD","",0);
    }
#ifndef OXOS
#ifdef COHERENT
    tttvt.c_lflag &= ~(ISIG|ICANON|ECHO);
#else
    tttvt.c_lflag &= ~(ISIG|ICANON|ECHO|IEXTEN);
#endif /* COHERENT */
#ifdef QNX
    /* Needed for hwfc */
    if (flow == FLO_RTSC || flow == FLO_DTRC || flow == FLO_DTRT)
      tttvt.c_lflag |= IEXTEN;
#endif /* QNX */
#else /* OXOS */
    tttvt.c_lflag &= ~(ISIG|ICANON|ECHO);
    tttvt.c_cc[VDISCARD] = tttvt.c_cc[VLNEXT] = CDISABLE;
#endif /* OXOS */

    tttvt.c_iflag |= (IGNBRK|IGNPAR);

/* Stop bits */

#ifdef CSTOPB
    if (xlocal) {
	if (stopbits == 2) {
	    tttvt.c_cflag |= CSTOPB;	/* 2 stop bits */
	    debug(F100,"ttvt 2 stopbits","",0);
	} else if (stopbits == 1) {
	    tttvt.c_cflag &= ~(CSTOPB);	/* 1 stop bit */
	    debug(F100,"ttvt 1 stopbit","",0);
	}
    }
#endif /* CSTOPB */

/* Parity */

#ifdef HWPARITY
    if (hwparity && xlocal) {		/* Hardware parity */
#ifdef COMMENT
/* Uncomment this only if needed -- I don't think it is */
	ttraw.c_cflag &= ~(CSIZE);	/* Clear out character-size mask */
	ttraw.c_cflag |= CS8;		/* And set it to 8 */
#endif /* COMMENT */
#ifdef IGNPAR
	debug(F101,"ttvt hwparity IGNPAR","",IGNPAR);
	tttvt.c_iflag |= IGNPAR;	/* Don't discard incoming bytes */
#endif /* IGNPAR */
	tttvt.c_cflag |= PARENB;	/* Enable parity */

	switch (hwparity) {
	  case 'e':			/* Even */
	    tttvt.c_cflag &= ~(PARODD);
	    debug(F100,"ttvt 8 bits + even parity","",0);
	    break;
	  case 'o':			/* Odd */
	    tttvt.c_cflag |= PARODD;
	    debug(F100,"ttvt 8 bits + odd parity","",0);
	    break;
	  case 'm':			/* Mark */
	  case 's':			/* Space */
	    /* PAREXT is mentioned in SVID but the details are not given. */
	    /* PAREXT is not included in POSIX ISO/IEC 9945-1. */
	    debug(F100,"ttvt 8 bits + invalid parity","",0);
	    break;
	}
    } else {				/* We handle parity ourselves */
#endif /* HWPARITY */
	tttvt.c_cflag &= ~(PARENB);	/* Don't enable parity */
#ifdef HWPARITY
    }
#endif /* HWPARITY */

#ifdef ATTSV
#ifdef BSD44
    /* Things not to do... */
    tttvt.c_iflag &= ~(INLCR|IGNCR|ICRNL|INPCK|ISTRIP|IXANY);
#else
    tttvt.c_iflag &= ~(INLCR|IGNCR|ICRNL|IUCLC|INPCK|ISTRIP|IXANY);
#endif /* BSD44 */
#else /* POSIX */
    tttvt.c_iflag &= ~(INLCR|IGNCR|ICRNL|INPCK|ISTRIP);
#endif /* ATTSV */
    tttvt.c_cflag &= ~(CSIZE);		/* Zero out the char size field */
    tttvt.c_cflag |= (CS8|CREAD|HUPCL);	/* Char size 8, enable receiver, hup */
    tttvt.c_oflag &= ~OPOST;		/* Don't postprocess output */
#ifndef VEOF /* DGUX termio has VEOF at entry 4, see comment above */
    tttvt.c_cc[4] = 1;
#else
#ifndef OXOS
#ifdef VMIN
    tttvt.c_cc[VMIN] = 1;
#endif /* VMIN */
#else /* OXOS */
    tttvt.c_min = 1;
#endif /* OXOS */
#endif /* VEOF */
#ifndef VEOL	/* DGUX termio has VEOL at entry 5, see comment above */
    tttvt.c_cc[5] = 0;
#else
#ifndef OXOS
#ifdef VTIME
    tttvt.c_cc[VTIME] = 0;
#endif /* VTIME */
#else /* OXOS */
    tttvt.c_time = 0;
#endif /* OXOS */
#endif /* VEOL */

#ifdef Plan9
    if (p9ttyparity('n') < 0)
      return -1;
#else
#ifdef BSD44ORPOSIX
    errno = 0;
#ifdef BEOSORBEBOX
    tttvt.c_cc[VMIN] = 0;		/* DR7 can only poll. */
#endif /* BEOSORBEBOX */

    x = tcsetattr(ttyfd,TCSADRAIN,&tttvt);
    debug(F101,"ttvt BSD44ORPOSIX tcsetattr","",x);
    if (x < 0) {
	debug(F101,"ttvt BSD44ORPOSIX tcsetattr errno","",errno);
	return(-1);
    }
#else /* ATTSV */
    x = ioctl(ttyfd,TCSETAW,&tttvt);
    debug(F101,"ttvt ATTSV ioctl TCSETAW","",x);
    if (x < 0) {			/* set new modes . */
	debug(F101,"ttvt ATTSV ioctl TCSETAW errno","",errno);
	return(-1);	
    }
#endif /* BSD44ORPOSIX */
#endif /* Plan9 */
#endif /* ATTSV */

    ttspeed = speed;			/* Done, remember how we were */
    ttflow = flow;			/* called, so we can decide how to */
    tvtflg = 1;				/* respond next time. */
    debug(F100,"ttvt ok","",0);
    return(0);

#ifdef COHERENT
#undef SVORPOSIX
#endif /* COHERENT */

#endif /* NOLOCAL */
}

#ifndef NOLOCAL

/* Serial speed department . . . */

/*
  SCO OSR5.0.x might or might not support high speeds.  Sometimes they are not
  defined in the header files but they are supported (e.g. when building with
  UDK compiler rather than /bin/cc), sometimes vice versa.  Even though 5.0.4
  was the first release that came with high serial speeds standard, releases
  back to 5.0.0 could use them if certain patches (or "supplements") were
  applied to the SIO driver.  Plus a lot of SCO installations run third-party
  drivers.
*/
#ifdef CK_SCOV5
#ifndef B38400
#define	B38400	0000017
#endif /* B38400 */
#ifndef B57600
#define	B57600	0000021
#endif /* B57600 */
#ifndef B76800
#define	B76800	0000022
#endif /* B76800 */
#ifndef B115200
#define	B115200	0000023
#endif /* B115200 */
#ifndef B230400
#define	B230400	0000024
#endif /* B230400 */
#ifndef B460800
#define	B460800	0000025
#endif /* B460800 */
#ifndef B921600
#define	B921600	0000026
#endif /* B921600 */
#endif /* CK_SCOV5 */
/*
  Plan 9's native speed setting interface lets you set anything you like,
  but will fail if the hardware doesn't like it, so we allow all the common
  speeds.
*/
#ifdef Plan9
#ifndef B50
#define B50 50
#endif /* B50 */
#ifndef B75
#define B75 75
#endif /* B75 */
#ifndef B110
#define B110 110
#endif /* B110 */
#ifndef B134
#define B134 134
#endif /* B134 */
#ifndef B200
#define B200 200
#endif /* B200 */
#ifndef B300
#define B300 300
#endif /* B300 */
#ifndef B1200
#define B1200 1200
#endif /* B1200 */
#ifndef B1800
#define B1800 1800
#endif /* B1800 */
#ifndef B2400
#define B2400 2400
#endif /* B2400 */
#ifndef B4800
#define B4800 4800
#endif /* B4800 */
#ifndef B9600
#define B9600 9600
#endif /* B9600 */
#ifndef B14400
#define B14400 14400
#endif /* B14400 */
#ifndef B19200
#define B19200 19200
#endif /* B19200 */
#ifndef B28800
#define B28800 28800
#endif /* B28800 */
#ifndef B38400
#define B38400 38400
#endif /* B38400 */
#ifndef B57600
#define B57600 57600
#endif /* B57600 */
#ifndef B76800
#define B76800 76800
#endif /* B76800 */
#ifndef B115200
#define B115200 115200
#endif /* B115200 */
#ifndef B230400
#define B230400 230400
#endif /* B230400 */
#ifndef B460800
#define B460800 460800
#endif /* B460800 */
#ifndef B921600
#define B921600 921600
#endif /* B921600 */
#endif /* Plan9 */

/*  T T S S P D  --  Checks and sets transmission rate.  */

/*  Call with speed in characters (not bits!) per second. */
/*  Returns -1 on failure, 0 if it did nothing, 1 if it changed the speed. */

#ifdef USETCSETSPEED
/*
  The tcsetspeed() / tcgetspeed() interface lets you pass any number at all
  to be used as a speed to be set, rather than forcing a choice from a
  predefined list.  It seems to be peculiar to UnixWare 7.

  These are the function codes to be passed to tc[gs]etspeed(),
  but for some reason they don't seem to be picked up from termios.h.
*/
#ifndef TCS_ALL
#define TCS_ALL 0
#endif /* TCS_ALL */
#ifndef TCS_IN
#define TCS_IN 1
#endif /* TCS_IN */
#ifndef TCS_OUT
#define TCS_OUT 2
#endif /* TCS_OUT */
#endif /* USETCSETSPEED */

int
ttsspd(cps) int cps; {
    int x;
#ifdef POSIX
/* Watch out, speed_t should be unsigned, so don't compare with -1, etc... */
    speed_t
#else
    int
#endif /* POSIX */
      s, s2;
    int ok = 1;				/* Speed check result, assume ok */

#ifdef OLINUXHISPEED
    unsigned int spd_flags = 0;
    struct serial_struct serinfo;
#endif /* OLINUXHISPEED */

    debug(F101,"ttsspd cps","",cps);
    debug(F101,"ttsspd ttyfd","",ttyfd);
    debug(F101,"ttsspd xlocal","",xlocal);

    if (ttyfd < 0 || xlocal == 0)	/* Don't set speed on console */
      return(0);

#ifdef	NETCONN
    if (netconn) {
#ifdef TN_COMPORT
        if (istncomport())
	  return(tnc_set_baud(cps * 10));
        else
#endif /* TN_COMPORT */
	return(0);
  }
#endif	/* NETCONN */
#ifdef NETCMD
    if (ttpipe) return(0);
#endif /* NETCMD */
#ifdef NETPTY
    if (ttpty) return(0);
#endif /* NETPTY */

    if (cps < 0) return(-1);
    s = s2 = 0;				/* NB: s and s2 might be unsigned */

#ifdef USETCSETSPEED

    s = cps * 10L;

    x = tcgetattr(ttyfd,&ttcur);	/* Get current speed */
    debug(F101,"ttsspd tcgetattr","",x);
    if (x < 0)
      return(-1);
    debug(F101,"ttsspd TCSETSPEED speed","",s);

    errno = 0;
    if (s == 8880L) {			/* 75/1200 split speed requested */
	tcsetspeed(TCS_IN, &ttcur, 1200L);
	tcsetspeed(TCS_OUT, &ttcur, 75L);
    } else
      tcsetspeed(TCS_ALL, &ttcur, s);	/* Put new speed in structs */
#ifdef DEBUG
    if (errno & deblog) {
	debug(F101,"ttsspd TCSETSPEED errno","",errno);
    }
#endif /* DEBUG */

#ifdef COMMENT
    tcsetspeed(TCS_ALL, &ttraw, s);
    tcsetspeed(TCS_ALL, &tttvt, s);
    tcsetspeed(TCS_ALL, &ttold, s);
#else
    if (s == 8880L) {			/* 75/1200 split speed requested */
	tcsetspeed(TCS_IN, &ttraw, 1200L);
	tcsetspeed(TCS_OUT, &ttraw, 75L);
	tcsetspeed(TCS_IN, &tttvt, 1200L);
	tcsetspeed(TCS_OUT, &tttvt, 75L);
	tcsetspeed(TCS_IN, &ttold, 1200L);
	tcsetspeed(TCS_OUT, &ttold, 75L);
    } else {
	tcsetspeed(TCS_ALL, &ttraw, s);
	tcsetspeed(TCS_ALL, &tttvt, s);
	tcsetspeed(TCS_ALL, &ttold, s);
    }
#endif /* COMMENT */

    x = tcsetattr(ttyfd,TCSADRAIN,&ttcur); /* Set the speed */
    debug(F101,"ttsspd tcsetattr","",x);
    if (x < 0)
      return(-1);

#else  /* Not USETCSETSPEED */

    /* First check that the given speed is valid. */

    switch (cps) {
#ifndef MINIX
      case 0:   s = B0;    break;
      case 5:   s = B50;   break;
      case 7:   s = B75;   break;
#endif /* MINIX */
      case 11:  s = B110;  break;
#ifndef MINIX
      case 13:  s = B134;  break;
      case 15:  s = B150;  break;
      case 20:  s = B200;  break;
#endif /* MINIX */
      case 30:  s = B300;  break;
#ifndef MINIX
      case 60:  s = B600;  break;
#endif /* MINIX */
      case 120: s = B1200; break;
#ifndef MINIX
      case 180: s = B1800; break;
#endif /* MINIX */
      case 240: s = B2400; break;
      case 480: s = B4800; break;
#ifndef MINIX
      case 888: s = B75; s2 = B1200; break; /* 888 = 75/1200 split speed */
#endif /* MINIX */
#ifdef B7200
      case 720: s = B7200; break;
#endif /* B7200 */
      case 960: s = B9600; break;
#ifdef B14400
      case 1440: s = B14400; break;
#endif /* B14400 */
#ifdef B19200
      case 1920: s = B19200; break;
#else
#ifdef EXTA
      case 1920: s = EXTA; break;
#endif /* EXTA */
#endif /* B19200 */
#ifdef B28800
      case 2880: s = B28800; break;
#endif /* B28800 */
#ifdef B38400
      case 3840: s = B38400;
#ifdef OLINUXHISPEED
        spd_flags = ~ASYNC_SPD_MASK;	/* Nonzero, but zero flags */
#endif /* OLINUXHISPEED */
	break;
#else /* B38400 not defined... */
#ifdef EXTB
      case 3840: s = EXTB; break;
#endif /* EXTB */
#endif /* B38400 */

#ifdef HPUX
#ifdef _B57600
      case 5760: s = _B57600; break;
#endif /* _B57600 */
#ifdef _B115200
      case 11520: s = _B115200; break;
#endif /* _B115200 */
#else
#ifdef OLINUXHISPEED
/*
  This bit from <carlo@sg.tn.tudelft.nl>:
  "Only note to make is maybe this: When the ASYNC_SPD_CUST flags are set then
  setting the speed to 38400 will set the custom speed (and ttgspd returns
  38400), but speeds 57600 and 115200 won't work any more because I didn't
  want to mess up the speed flags when someone is doing sophisticated stuff
  like custom speeds..."
*/
      case 5760: s = B38400; spd_flags = ASYNC_SPD_HI; break;
      case 11520: s = B38400; spd_flags = ASYNC_SPD_VHI; break;
#else
#ifdef B57600
      case 5760: s = B57600; break;
#endif /* B57600 */
#ifdef B76800
      case 7680: s = B76800; break;
#endif /* B76800 */
#ifdef B115200
      case 11520: s = B115200; break;
#endif /* B115200 */
#endif /* OLINUXHISPEED */
#ifdef B153600
      case 15360: s = B153600; break;
#endif /* B153600 */
#ifdef B230400
      case 23040: s = B230400; break;
#endif /* B230400 */
#ifdef B307200
      case 30720: s = B307200; break;
#endif /* B307200 */
#ifdef B460800
      case 46080: s = B460800; break;
#endif /* 460800 */
#ifdef B921600
      case 92160: s = B921600; break;
#endif /* B921600 */
#endif /* HPUX */
      default:
	ok = 0;				/* Good speed not found, so not ok */
	break;
    }
    debug(F101,"ttsspd ok","",ok);
    debug(F101,"ttsspd s","",s);

    if (!ok) {
	debug(F100,"ttsspd fails","",0);
	return(-1);
    } else {
	if (!s2) s2 = s;		/* Set input speed */
#ifdef Plan9
	if (p9ttsspd(cps) < 0)
	  return(-1);
#else
#ifdef BSD44ORPOSIX
	x = tcgetattr(ttyfd,&ttcur);	/* Get current speed */
	debug(F101,"ttsspd tcgetattr","",x);
	if (x < 0)
	  return(-1);
#ifdef OLINUXHISPEED
	debug(F101,"ttsspd spd_flags","",spd_flags);
	if (spd_flags && spd_flags != ASYNC_SPD_CUST) {
	    if (ioctl(ttyfd, TIOCGSERIAL, &serinfo) < 0) {
		debug(F100,"ttsspd: TIOCGSERIAL failed","",0);
		return(-1);
	    } else debug(F100,"ttsspd: TIOCGSERIAL ok","",0);
	    serinfo.flags &= ~ASYNC_SPD_MASK;
	    serinfo.flags |= (spd_flags & ASYNC_SPD_MASK);
	    if (ioctl(ttyfd, TIOCSSERIAL, &serinfo) < 0)
	      return(-1);
	}
#endif /* OLINUXHISPEED */
	cfsetospeed(&ttcur,s);
	cfsetispeed(&ttcur,s2);
	cfsetospeed(&ttraw,s);
	cfsetispeed(&ttraw,s2);
	cfsetospeed(&tttvt,s);
	cfsetispeed(&tttvt,s2);
	cfsetospeed(&ttold,s);
	cfsetispeed(&ttold,s2);
	x = tcsetattr(ttyfd,TCSADRAIN,&ttcur);
	debug(F101,"ttsspd tcsetattr","",x);
	if (x < 0) return(-1);
#else
#ifdef ATTSV
	if (cps == 888) return(-1);	/* No split speeds, sorry. */
	x = ioctl(ttyfd,TCGETA,&ttcur);
	debug(F101,"ttsspd TCGETA ioctl","",x);
	if (x < 0) return(-1);
	ttcur.c_cflag &= ~CBAUD;
	ttcur.c_cflag |= s;
	tttvt.c_cflag &= ~CBAUD;
	tttvt.c_cflag |= s;
	ttraw.c_cflag &= ~CBAUD;
	ttraw.c_cflag |= s;
	ttold.c_cflag &= ~CBAUD;
	ttold.c_cflag |= s;
	x = ioctl(ttyfd,TCSETAW,&ttcur);
	debug(F101,"ttsspd TCSETAW ioctl","",x);
	if (x < 0) return(-1);
#else
#ifdef BELLV10
	x = ioctl(ttyfd,TIOCGDEV,&tdcur);
	debug(F101,"ttsspd TIOCGDEV ioctl","",x);
	if (x < 0) return(-1);
	tdcur.ispeed = s2;
	tdcur.ospeed = s;
	errno = 0;
	ok = ioctl(ttyfd,TIOCSDEV,&tdcur);
	debug(F101,"ttsspd BELLV10 ioctl","",ok);
	if (ok < 0) {
	    perror(ttnmsv);
	    debug(F101,"ttsspd BELLV10 errno","",ok);
	    return(-1);
	}
#else
	x = gtty(ttyfd,&ttcur);
	debug(F101,"ttsspd gtty","",x);
	if (x < 0) return(-1);
	ttcur.sg_ospeed = s; ttcur.sg_ispeed = s2;
	tttvt.sg_ospeed = s; tttvt.sg_ispeed = s2;
	ttraw.sg_ospeed = s; ttraw.sg_ispeed = s2;
	ttold.sg_ospeed = s; ttold.sg_ispeed = s2;
	x = stty(ttyfd,&ttcur);
	debug(F101,"ttsspd stty","",x);
	if (x < 0) return(-1);
#endif /* BELLV10 */
#endif /* ATTSV */
#endif /* BSD44ORPOSIX */
#endif /* Plan9 */
    }
    return(1);				/* Return 1 = success. */
#endif /* USETCSETSPEED */
}

#endif /* NOLOCAL */

/* C O N G S P D  -  Get speed of console terminal  */

long
congspd() {
/*
  This is a disgusting hack.  The right way to do this would be to pass an
  argument to ttgspd(), but then we'd need to change the Kermit API and
  all of the ck?tio.c modules.  (Currently used only for rlogin.)
*/
    int t1;
    long spd;
#ifdef NETCONN
    int t2 = netconn;
    netconn = 0;
#endif /* NETCONN */
    t1 = ttyfd;
    ttyfd = -1;
    spd = ttgspd();
    debug(F101,"congspd","",spd);
#ifdef NETCONN
    netconn = t2;
#endif /* NETCONN */
    ttyfd = t1;
    return(spd);
}

/*  T T S P D L I S T  -- Get list of serial speeds allowed on this platform */

#define NSPDLIST 64
static long spdlist[NSPDLIST];
/*
  As written, this picks up the speeds known at compile time, and thus
  apply to the system where C-Kermit was built, rather than to the one where
  it is running.  Suggestions for improvement are always welcome.
*/
long *
ttspdlist() {
    int i;
    for (i = 0; i < NSPDLIST; i++)	/* Initialize the list */
      spdlist[i] = -1L;
    i = 1;

#ifdef USETCSETSPEED			/* No way to find out what's legal */
    debug(F100,"ttspdlist USETCSETSPEED","",0);
    spdlist[i++] = 50L;
#ifndef UW7
    spdlist[i++] = 75L;
#endif /* UW7 */
    spdlist[i++] = 110L;
#ifndef UW7
    spdlist[i++] = 134L;
#endif /* UW7 */
    spdlist[i++] = 150L;
    spdlist[i++] = 200L;
    spdlist[i++] = 300L;
    spdlist[i++] = 600L;
    spdlist[i++] = 1200L;
    spdlist[i++] = 1800L;
    spdlist[i++] = 2400L;
    spdlist[i++] = 4800L;
    spdlist[i++] = 8880L;
    spdlist[i++] = 9600L;
    spdlist[i++] = 14400L;
    spdlist[i++] = 19200L;
    spdlist[i++] = 28800L;
#ifndef UW7
    spdlist[i++] = 33600L;
#endif /* UW7 */
    spdlist[i++] = 38400L;
    spdlist[i++] = 57600L;
    spdlist[i++] = 76800L;
    spdlist[i++] = 115200L;
#ifndef UW7
    spdlist[i++] = 153600L;
    spdlist[i++] = 230400L;
    spdlist[i++] = 307200L;
    spdlist[i++] = 460800L;
    spdlist[i++] = 921600L;
#endif /* UW7 */

#else  /* USETCSETSPEED */

    debug(F100,"ttspdlist no USETCSETSPEED","",0);

#ifdef B50
    debug(F101,"ttspdlist B50","",B50);
    spdlist[i++] = 50L;
#endif /* B50 */
#ifdef B75
    debug(F101,"ttspdlist B75","",B75);
    spdlist[i++] = 75L;
#endif /* B75 */
#ifdef B110
    debug(F101,"ttspdlist B110","",B110);
    spdlist[i++] = 110L;
#endif /* B110 */
#ifdef B134
    debug(F101,"ttspdlist B134","",B134);
    spdlist[i++] = 134L;
#endif /* B134 */
#ifdef B150
    debug(F101,"ttspdlist B150","",B150);
    spdlist[i++] = 150L;
#endif /* B150 */
#ifdef B200
    debug(F101,"ttspdlist B200","",B200);
    spdlist[i++] = 200L;
#endif /* B200 */
#ifdef B300
    debug(F101,"ttspdlist B300","",B300);
    spdlist[i++] = 300L;
#endif /* B300 */
#ifdef B600
    debug(F101,"ttspdlist B600","",B600);
    spdlist[i++] = 600L;
#endif /* B600 */
#ifdef B1200
    debug(F101,"ttspdlist B1200","",B1200);
    spdlist[i++] = 1200L;
#endif /* B1200 */
#ifdef B1800
    debug(F101,"ttspdlist B1800","",B1800);
    spdlist[i++] = 1800L;
#endif /* B1800 */
#ifdef B2400
    debug(F101,"ttspdlist B2400","",B2400);
    spdlist[i++] = 2400L;
#endif /* B2400 */
#ifdef B4800
    debug(F101,"ttspdlist B4800","",B4800);
    spdlist[i++] = 4800L;
#endif /* B4800 */
#ifdef B9600
    debug(F101,"ttspdlist B9600","",B9600);
    spdlist[i++] = 9600L;
#endif /* B9600 */
#ifdef B14400
    debug(F101,"ttspdlist B14400","",B14400);
    spdlist[i++] = 14400L;
#endif /* B14400 */
#ifdef B19200
    debug(F101,"ttspdlist B19200","",B19200);
    spdlist[i++] = 19200L;
#else
#ifdef EXTA
    debug(F101,"ttspdlist EXTA","",EXTA);
    spdlist[i++] = 19200L;
#endif /* EXTA */
#endif /* B19200 */
#ifdef B28800
    debug(F101,"ttspdlist B28800","",B28800);
    spdlist[i++] = 28800L;
#endif /* B28800 */
#ifdef B33600
    debug(F101,"ttspdlist B33600","",B33600);
    spdlist[i++] = 33600L;
#endif /* B33600 */
#ifdef B38400
    debug(F101,"ttspdlist B38400","",B38400);
    spdlist[i++] = 38400L;
#else
#ifdef EXTB
    debug(F101,"ttspdlist EXTB","",EXTB);
    spdlist[i++] = 38400L;
#endif /* EXTB */
#endif /* B38400 */
#ifdef _B57600
    debug(F101,"ttspdlist _B57600","",_B57600);
    spdlist[i++] = 57600L;
#else
#ifdef B57600
    debug(F101,"ttspdlist B57600","",B57600);
    spdlist[i++] = 57600L;
#endif /* B57600 */
#endif /* _B57600 */
#ifdef B76800
    debug(F101,"ttspdlist B76800","",B76800);
    spdlist[i++] = 76800L;
#endif /* B76800 */
#ifdef _B115200
    debug(F101,"ttspdlist _B115200","",_B115200);
    spdlist[i++] = 115200L;
#else
#ifdef B115200
    debug(F101,"ttspdlist B115200","",B115200);
    spdlist[i++] = 115200L;
#endif /* B115200 */
#endif /* _B115200 */
#ifdef B153600
    debug(F101,"ttspdlist B153600","",B153600);
    spdlist[i++] = 153600L;
#endif /* B153600 */
#ifdef B230400
    debug(F101,"ttspdlist B230400","",B230400);
    spdlist[i++] = 230400L;
#endif /* B230400 */
#ifdef B307200
    debug(F101,"ttspdlist B307200","",B307200);
    spdlist[i++] = 307200L;
#endif /* B307200 */
#ifdef B460800
    debug(F101,"ttspdlist B460800","",B460800);
    spdlist[i++] = 460800L;
#endif /* B460800 */
#ifdef B921600
    debug(F101,"ttspdlist B921600","",B921600);
    spdlist[i++] = 921600L;
#endif /* B921600 */
#endif /* USETCSETSPEED */
    spdlist[0] = i - 1;			/* Return count in 0th element */
    debug(F111,"ttspdlist spdlist","0",spdlist[0]);
    return((long *)spdlist);
}

/* T T G S P D  -  Get speed of currently selected tty line  */

/*
  Unreliable.  After SET LINE, it returns an actual speed, but not necessarily
  the real speed.  On some systems, it returns the line's nominal speed, from
  /etc/ttytab.  Even if you SET SPEED to something else, this function might
  not notice.
*/
long
ttgspd() {				/* Get current serial device speed */
#ifdef NOLOCAL
    return(-1L);
#else
#ifdef POSIX
    speed_t				/* Should be unsigned */
#else
    int					/* Isn't unsigned */
#endif /* POSIX */
      s;
    int x;
    long ss;
#ifdef OLINUXHISPEED
    unsigned int spd_flags = 0;
    struct serial_struct serinfo;
#endif /* OLINUXHISPEED */

#ifdef NETCONN
    if (netconn) {
#ifdef TN_COMPORT
	if (istncomport())
	  return(tnc_get_baud());
	else
#endif /* TN_COMPORT */
	  return(-1);			/* -1 if network connection */
    }
#endif /* NETCONN */
#ifdef NETCMD
    if (ttpipe) return(-1);
#endif /* NETCMD */
#ifdef NETPTY
    if (ttpty) return(-1);
#endif /* NETPTY */

    debug(F101,"ttgspd ttyfd","",ttyfd);

#ifdef USETCSETSPEED

    x = tcgetattr(ttyfd,&ttcur);	/* Get current speed */
    debug(F101,"ttgspd tcgetattr","",x);
    if (x < 0)
      return(-1);
    errno = 0;
    s = tcgetspeed(TCS_ALL, &ttcur);
    debug(F101,"ttsspd TCGETSPEED speed","",s);
    if (s == 0) {
	long s1, s2;
	s1 = tcgetspeed(TCS_IN, &ttcur);
	s2 = tcgetspeed(TCS_OUT, &ttcur);
	if (s1 == 1200L && s2 == 75L)
	  return(8880L);
    }
#ifdef DEBUG
    if (errno & deblog) {
	debug(F101,"ttsspd TCGETSPEED errno","",errno);
    }
#endif /* DEBUG */
    return(s);

#else  /* Not USETCSETSPEED */

#ifdef Plan9
    if (ttyfd < 0)
      ss = -1;
    else
      ss = ttylastspeed;
#else
#ifdef OLINUXHISPEED
    debug(F100,"ttgspd Linux OLINUXHISPEED","",0);
#endif /* OLINUXHISPEED */

    if (ttyfd < 0) {
#ifdef BSD44ORPOSIX
	s = cfgetospeed(&ccold);
	debug(F101,"ttgspd cfgetospeed 1 POSIX","",s);
#else
#ifdef ATTSV
	s = ccold.c_cflag & CBAUD;
	debug(F101,"ttgspd c_cflag CBAUD 1 ATTSV","",s);
#else
	s = ccold.sg_ospeed;		/* (obtained by congm()) */
	debug(F101,"ttgspd sg_ospeed 1","",s);
#endif /* ATTSV */
#endif /* BSD44POSIX */

    } else {
#ifdef BSD44ORPOSIX
	if (tcgetattr(ttyfd,&ttcur) < 0) return(-1);
	s = cfgetospeed(&ttcur);
	debug(F101,"ttgspd cfgetospeed 2 BSDORPOSIX","",s);
#ifdef OLINUXHISPEED
	if (ioctl(ttyfd,TIOCGSERIAL,&serinfo) > -1)
	  spd_flags = serinfo.flags & ASYNC_SPD_MASK;
	debug(F101,"ttgspd spd_flags","",spd_flags);
#endif /* OLINUXHISPEED */
#else
#ifdef ATTSV
	x = ioctl(ttyfd,TCGETA,&ttcur);
	debug(F101,"ttgspd ioctl 2 ATTSV x","",x);
	debug(F101,"ttgspd ioctl 2 ATTSV errno","",errno);
	if (x < 0) return(-1);
	s = ttcur.c_cflag & CBAUD;
	debug(F101,"ttgspd ioctl 2 ATTSV speed","",s);
#else
#ifdef BELLV10
	x = ioctl(ttyfd,TIOCGDEV,&tdcur);
	debug(F101,"ttgspd ioctl 2 BELLV10 x","",x);
	if (x < 0) return(-1);
	s = tdcur.ospeed;
	debug(F101,"ttgspd ioctl 2 BELLV10 speed","",s);
#else
	x = gtty(ttyfd,&ttcur);
	debug(F101,"ttgspd gtty 2 x","",x);
	debug(F101,"ttgspd gtty 2 errno","",errno);
	if (x < 0) return(-1);
	s = ttcur.sg_ospeed;
	debug(F101,"ttgspd gtty 2 speed","",s);
#endif /* BELLV10 */
#endif /* ATTSV */
#endif /* BSD44ORPOSIX */
    }
    debug(F101,"ttgspd code","",s);
#ifdef OLINUXHISPEED
    debug(F101,"ttgspd spd_flags","",spd_flags);
#endif /* OLINUXHISPEED */
    switch (s) {
#ifdef B0
      case B0:    ss = 0L; break;
#endif /* B0 */

#ifndef MINIX
/*
 MINIX defines the Bxx symbols to be bps/100, so B50==B75, B110==B134==B150,
 etc, making for many "duplicate case in switch" errors, which are fatal.
*/
#ifdef B50
      case B50:   ss = 50L; break;
#endif /* B50 */
#ifdef B75
      case B75:   ss = 75L; break;
#endif /* B75 */
#endif /* MINIX */

#ifdef B110
      case B110:  ss = 110L; break;
#endif /* B110 */

#ifndef MINIX
#ifdef B134
      case B134:  ss = 134L; break;
#endif /* B134 */
#ifdef B150
      case B150:  ss = 150L; break;
#endif /* B150 */
#endif /* MINIX */

#ifdef B200
      case B200:  ss = 200L; break;
#endif /* B200 */

#ifdef B300
      case B300:  ss = 300L; break;
#endif /* B300 */

#ifdef B600
      case B600:  ss = 600L; break;
#endif /* B600 */

#ifdef B1200
      case B1200: ss = 1200L; break;
#endif /* B1200 */

#ifdef B1800
      case B1800: ss = 1800L; break;
#endif /* B1800 */

#ifdef B2400
      case B2400: ss = 2400L; break;
#endif /* B2400 */

#ifdef B4800
      case B4800: ss = 4800L; break;
#endif /* B4800 */

#ifdef B7200
      case B7200: ss = 7200L; break;
#endif /* B7200 */

#ifdef B9600
      case B9600: ss = 9600L; break;
#endif /* B9600 */

#ifdef B19200
      case B19200: ss = 19200L; break;
#else
#ifdef EXTA
      case EXTA: ss = 19200L; break;
#endif /* EXTA */
#endif /* B19200 */

#ifndef MINIX
#ifdef B38400
      case B38400:
        ss = 38400L;
#ifdef OLINUXHISPEED
        switch(spd_flags) {
          case ASYNC_SPD_HI:  ss =  57600L; break;
          case ASYNC_SPD_VHI: ss = 115200L; break;
	}
#endif /* OLINUXHISPEED */
        break;
#else
#ifdef EXTB
      case EXTB: ss = 38400L; break;
#endif /* EXTB */
#endif /* B38400 */
#endif /* MINIX */

#ifdef HPUX
#ifdef _B57600
      case _B57600: ss = 57600L; break;
#endif /* _B57600 */
#ifdef _B115200
      case _B115200: ss = 115200L; break;
#endif /* _B115200 */
#else
#ifdef B57600
      case B57600: ss = 57600L; break;
#endif /* B57600 */
#ifdef B76800
      case B76800: ss = 76800L; break;
#endif /* B76800 */
#ifdef B115200
      case B115200: ss = 115200L; break;
#endif /* B115200 */
#ifdef B153600
      case B153600: ss = 153600L; break;
#endif /* B153600 */
#ifdef B230400
      case B230400: ss = 230400L; break;
#endif /* B230400 */
#ifdef B307200
      case B307200: ss = 307200L; break;
#endif /* B307200 */
#ifdef B460800
      case B460800: ss = 460800L; break;
#endif /* B460800 */
#endif /* HPUX */
#ifdef B921600
      case B921600: ss = 921600L; break;
#endif /* B921600 */
      default:
	ss = -1; break;
    }
#endif /* Plan9 */
    debug(F101,"ttgspd speed","",ss);
    return(ss);

#endif /* USETCSETSPEED */
#endif /* NOLOCAL */
}
#ifdef MINIX2				/* Another hack alert */
#define MINIX
#endif /* MINIX2 */

/*
  FIONREAD data type...  This has been defined as "long" for many, many
  years, and it worked OK until 64-bit platforms appeared.  Thus we use
  int for 64-bit platforms, but keep long for the others.  If we changed
  the default PEEKTYPE to int, this would probably break 16-bit builds
  (note that sizeof(long) == sizeof(int) on most 32-bit platforms), many
  of which we have no way of testing any more.  Therefore, do not change
  the default definition of PEEKTYPE -- only add exceptions to it as needed.
*/
#ifdef COHERENT
#ifdef FIONREAD
#undef FIONREAD
#endif /* FIONREAD */
/* #define FIONREAD TIOCQUERY */
/* #define PEEKTYPE int */
#else  /* Not COHERENT... */

#ifdef OSF32				/* Digital UNIX 3.2 or higher */
#define PEEKTYPE int
#else
#define PEEKTYPE long			/* Elsewhere (see notes above) */
#endif /* OSF32 */
#endif /* COHERENT */

/* ckumyr.c by Kristoffer Eriksson, ske@pkmab.se, 15 Mar 1990. */

#ifdef MYREAD

/* Private buffer for myread() and its companions.  Not for use by anything
 * else.  ttflui() is allowed to reset them to initial values.  ttchk() is
 * allowed to read my_count.
 *
 * my_item is an index into mybuf[].  Increment it *before* reading mybuf[].
 *
 * A global parity mask variable could be useful too.  We could use it to
 * let myread() strip the parity on its own, instead of stripping sign
 * bits as it does now.
 */
#ifdef BIGBUFOK
#define MYBUFLEN 32768
#else
#ifdef pdp11
#define MYBUFLEN 256
#else
#define MYBUFLEN 1024
#endif /* pdp11 */
#endif /* BIGBUFOK */

#ifdef ANYX25
#undef MYBUFLEN
#define MYBUFLEN 256
/*
  On X.25 connections, there is an extra control byte at the beginning.
*/
static CHAR x25buf[MYBUFLEN+1];		/* Communication device input buffer */
static CHAR  *mybuf = x25buf+1;
#else
static CHAR mybuf[MYBUFLEN];
#endif /* ANYX25 */

static int my_count = 0;		/* Number of chars still in mybuf */
static int my_item = -1;		/* Last index read from mybuf[]   */

/*  T T P E E K  --  Peek into our internal communications input buffers. */

/*
  NOTE: This routine is peculiar to UNIX, and is used only by the
  select()-based CONNECT module, ckucns.c.  It need not be replicated in
  the ck?tio.c of other platforms.
*/
int
ttpeek() {
#ifdef TTLEBUF
    int rc = 0;
    if (ttpush >= 0)
      rc++;
    rc += le_inbuf();
    if (rc > 0)
      return(rc);
    else
#endif /* TTLEBUF */

#ifdef MYREAD
    return(my_count);
#else
    return(0);
#endif /* MYREAD */
}

/* myread() -- Efficient read of one character from communications line.
 *
 * NOTE: myread() and its helpers mygetbuf() and myfillbuf() return raw
 * bytes from connection, so when the connection is encrypted, these bytes
 * must be decrypted.
 *
 * Uses a private buffer to minimize the number of expensive read() system
 * calls.  Essentially performs the equivalent of read() of 1 character, which
 * is then returned.  By reading all available input from the system buffers
 * to the private buffer in one chunk, and then working from this buffer, the
 * number of system calls is reduced in any case where more than one character
 * arrives during the processing of the previous chunk, for instance high
 * baud rates or network type connections where input arrives in packets.
 * If the time needed for a read() system call approaches the time for more
 * than one character to arrive, then this mechanism automatically compensates
 * for that by performing bigger read()s less frequently.  If the system load
 * is high, the same mechanism compensates for that too.
 *
 * myread() is a macro that returns the next character from the buffer.  If the
 * buffer is empty, mygetbuf() is called.  See mygetbuf() for possible error
 * returns.
 *
 * This should be efficient enough for any one-character-at-a-time loops.
 * For even better efficiency you might use memcpy()/bcopy() or such between
 * buffers (since they are often better optimized for copying), but it may not
 * be worth it if you have to take an extra pass over the buffer to strip
 * parity and check for CTRL-C anyway.
 *
 * Note that if you have been using myread() from another program module, you
 * may have some trouble accessing this macro version and the private variables
 * it uses.  In that case, just add a function in this module, that invokes the
 * macro.
 */
#define myread() (--my_count < 0 ? mygetbuf() : 255 & (int)mybuf[++my_item])

/* Specification: Push back up to one character onto myread()'s queue.
 *
 * This implementation: Push back characters into mybuf. At least one character
 * must have been read through myread() before myunrd() may be used.  After
 * EOF or read error, again, myunrd() can not be used.  Sometimes more than
 * one character can be pushed back, but only one character is guaranteed.
 * Since a previous myread() must have read its character out of mybuf[],
 * that guarantees that there is space for at least one character.  If push
 * back was really needed after EOF, a small addition could provide that.
 *
 * As of 02/2007 myunrd() is used by ttinl().
 */
VOID
#ifdef CK_ANSIC
myunrd(CHAR ch)
#else
myunrd(ch) CHAR ch;
#endif	/* CK_ANSIC */
{
    if (my_item >= 0) {
	mybuf[my_item--] = ch;
	++my_count;
    }
}

/*  T T P U S H B A C K  --  Put n bytes back into the myread buffer */

static CHAR * pushbuf = NULL;
/* static int pushed = 0; */

int
ttpushback(s,n) CHAR * s; int n; {
    debug(F101,"ttpushback n","",n);
    if (pushbuf || n > MYBUFLEN || n < 1)
      return(-1);
    debug(F101,"ttpushback my_count","",my_count);
    if (my_count > 0) {
	if (!(pushbuf = (CHAR *)malloc(n+1)))
	  return(-1);
	memcpy(pushbuf,mybuf,my_count);
	/* pushed = my_count; */ /* (set but never used) */
    }
    memcpy(mybuf,s,n);
    my_count = n;
    my_item = -1;
    return(0);
}

/* mygetbuf() -- Fill buffer for myread() and return first character.
 *
 * This function is what myread() uses when it can't get the next character
 * directly from its buffer.  First, it calls a system dependent myfillbuf()
 * to read at least one new character into the buffer, and then it returns
 * the first character just as myread() would have done.  This function also
 * is responsible for all error conditions that myread() can indicate.
 *
 * Returns: When OK	=> a positive character, 0 or greater.
 *	    When EOF	=> -2.
 *	    When error	=> -3, error code in errno.
 *
 * Older myread()s additionally returned -1 to indicate that there was nothing
 * to read, upon which the caller would call myread() again until it got
 * something.  The new myread()/mygetbuf() always gets something.  If it
 * doesn't, then make it do so!  Any program that actually depends on the old
 * behaviour will break.
 *
 * The older version also used to return -2 both for EOF and other errors,
 * and used to set errno to 9999 on EOF.  The errno stuff is gone, EOF and
 * other errors now return different results, although Kermit currently never
 * checks to see which it was.  It just disconnects in both cases.
 *
 * Kermit lets the user use the quit key to perform some special commands
 * during file transfer.  This causes read(), and thus also mygetbuf(), to
 * finish without reading anything and return the EINTR error.  This should
 * be checked by the caller.  Mygetbuf() could retry the read() on EINTR,
 * but if there is nothing to read, this could delay Kermit's reaction to
 * the command, and make Kermit appear unresponsive.
 *
 * The debug() call should be removed for optimum performance.
 */
int
mygetbuf() {
    int x;
    errno = 0;
#ifdef DEBUG
    if (deblog && my_count > 0)
      debug(F101,"mygetbuf IMPROPERLY CALLED with my_count","",my_count);
#endif /* DEBUG */
    if (my_count <= 0)
      my_count = myfillbuf();

#ifdef DEBUG
#ifdef COMMENT
    if (deblog) debug(F101, "mygetbuf read", "", my_count);
#else /* COMMENT */
    ckhexdump("mygetbuf read", mybuf, my_count);
#endif /* COMMENT */
#endif /* DEBUG */
    x = my_count;
    if (my_count <= 0) {
	my_count = 0;
	my_item = -1;
	debug(F101,"mygetbuf errno","",errno);
#ifdef TCPSOCKET
	if (netconn && ttnet == NET_TCPB && errno != 0) {
	    if (errno != EINTR) {
		debug(F101,"mygetbuf TCP error","",errno);
		ttclos(0);		/* Close the connection. */
	    }
	    return(-3);
	}
#endif /* TCPSOCKET */
	if (!netconn && xlocal && errno) {
	    if (errno != EINTR) {
		debug(F101,"mygetbuf SERIAL error","",errno);
		x = -3;
		ttclos(0);		/* Close the connection. */
	    }
	}
	return((x < 0) ? -3 : -2);
    }
    --my_count;
    return((unsigned)(0xff & mybuf[my_item = 0]));
}

/* myfillbuf():
 * System-dependent read() into mybuf[], as many characters as possible.
 *
 * Returns: OK => number of characters read, always more than zero.
 *          EOF => 0
 *          Error => -1, error code in errno.
 *
 * If there is input available in the system's buffers, all of it should be
 * read into mybuf[] and the function return immediately.  If no input is
 * available, it should wait for a character to arrive, and return with that
 * one in mybuf[] as soon as possible.  It may wait somewhat past the first
 * character, but be aware that any such delay lengthens the packet turnaround
 * time during kermit file transfers.  Should never return with zero characters
 * unless EOF or irrecoverable read error.
 *
 * Correct functioning depends on the correct tty parameters being used.
 * Better control of current parameters is required than may have been the
 * case in older Kermit releases.  For instance, O_NDELAY (or equivalent) can
 * no longer be sometimes off and sometimes on like it used to, unless a
 * special myfillbuf() is written to handle that.  Otherwise the ordinary
 * myfillbuf()s may think they have come to EOF.
 *
 * If your system has a facility to directly perform the functioning of
 * myfillbuf(), then use it.  If the system can tell you how many characters
 * are available in its buffers, then read that amount (but not less than 1).
 * If the system can return a special indication when you try to read without
 * anything to read, while allowing you to read all there is when there is
 * something, you may loop until there is something to read, but probably that
 * is not good for the system load.
 */

#ifdef SVORPOSIX
	/* This is for System III/V with VMIN>0, VTIME=0 and O_NDELAY off,
	 * and CLOCAL set any way you like.  This way, read() will do exactly
	 * what is required by myfillbuf(): If there is data in the buffers
	 * of the O.S., all available data is read into mybuf, up to the size
	 * of mybuf.  If there is none, the first character to arrive is
	 * awaited and returned.
	 */
int
myfillbuf() {
    int fd, n;
#ifdef NETCMD
    if (ttpipe)
      fd = fdin;
    else
#endif /* NETCMD */
      fd = ttyfd;

#ifdef sxaE50
    /* From S. Dezawa at Fujifilm in Japan.  I don't know why this is */
    /* necessary for the sxa E50, but it is. */
    return read(fd, mybuf, 255);
#else
#ifdef BEOSORBEBOX
    while (1) {
#ifdef NETCONN
        if (netconn) {
            n = netxin(sizeof(mybuf), (char *)mybuf);
            debug(F101,"BEBOX SVORPOSIX network myfillbuf","",n);
	}
        else
#endif /* NETCONN */
	  n = read(fd, mybuf, sizeof(mybuf));
	debug(F101,"BEBOX SVORPOSIX notnet myfillbuf","",n);
        if (n > 0)
	  return(n);
        snooze(1000.0);
    }
#else /* BEOSORBEBOX */
    errno = 0;
    /* debug(F101,"SVORPOSIX myfillbuf calling read() fd","",fd); */
#ifdef IBMX25
    if (netconn && (nettype == NET_IX25)) {
	/* can't use sizeof because mybuf is a pointer, and not an array! */
	n = x25xin( MYBUFLEN, mybuf );
    } else
#endif /* IBMX25 */

#ifdef CK_SSL
      if (ssl_active_flag || tls_active_flag) {
	  int error, n = 0;
	  debug(F100,"myfillbuf calling SSL_read() fd","",0);
	  while (n == 0) {
	      if (ssl_active_flag)
                n = SSL_read(ssl_con, (char *)mybuf, sizeof(mybuf));
	      else if (tls_active_flag)
                n = SSL_read(tls_con, (char *)mybuf, sizeof(mybuf));
              else
		break;
	      switch (SSL_get_error(ssl_active_flag?ssl_con:tls_con,n)) {
		case SSL_ERROR_NONE:
		  if (n > 0)
                    return(n);
		  if (n < 0)
                    return(-2);
		  msleep(50);
		  break;
		case SSL_ERROR_WANT_WRITE:
		case SSL_ERROR_WANT_READ:
		  return(-1);
		case SSL_ERROR_SYSCALL:
		  if (n != 0)
		    return(-1);
		case SSL_ERROR_WANT_X509_LOOKUP:
		case SSL_ERROR_SSL:
		case SSL_ERROR_ZERO_RETURN:
		default:
		  ttclos(0);
		  return(-3);
            }
        }
    }
#endif /* CK_SSL */
#ifdef CK_KERBEROS
#ifdef KRB4
#ifdef RLOGCODE
    if (ttnproto == NP_EK4LOGIN) {
	debug(F101,"myfillbuf calling krb4_des_read() fd","",ttyfd);
        if ((n = krb4_des_read(ttyfd,(char *)mybuf,sizeof(mybuf))) < 0)
	  return(-3);
        else
	  return(n);
    }
#endif /* RLOGCODE */
#endif /* KRB4 */
#ifdef KRB5
#ifdef RLOGCODE
    if (ttnproto == NP_EK5LOGIN) {
	debug(F101,"myfillbuf calling krb5_des_read() fd","",ttyfd);
        if ((n = krb5_des_read(ttyfd,(char *)mybuf,sizeof(mybuf),0)) < 0)
	  return(-3);
        else
	  return(n);
    }
#endif /* RLOGCODE */
#ifdef KRB5_U2U
    if (ttnproto == NP_K5U2U) {
	debug(F101,"myfillbuf calling krb5_u2u_read() fd","",ttyfd);
        if ((n = krb5_u2u_read(ttyfd,(char *)mybuf,sizeof(mybuf))) < 0)
	  return(-3);
        else
	  return(n);
    }
#endif /* KRB5_U2U */
#endif /* KRB5 */
#endif /* CK_KERBEROS */

#ifdef NETPTY
#ifdef HAVE_PTYTRAP
    /* Special handling for HP-UX pty i/o */
  ptyread:
    if (ttpty && pty_trap_pending(ttyfd) > 0) {
	debug(F101,"myfillbuf calling pty_trap_handler() fd","",ttyfd);
        if (pty_trap_handler(ttyfd) > 0) {
            ttclos(0);
            return(-3);
        }
    }
#endif /* HAVE_PTYTRAP */
#endif /* NETPTY */
    debug(F101,"myfillbuf calling read() fd","",ttyfd);
    n = read(fd, mybuf, sizeof(mybuf));
    debug(F101,"SVORPOSIX myfillbuf read","",n);
    debug(F101,"SVORPOSIX myfillbuf errno","",errno);
    debug(F101,"SVORPOSIX myfillbuf ttcarr","",ttcarr);
    if (n < 1) {
#ifdef NETPTY
#ifdef HAVE_PTYTRAP
        /* When we have a PTY trap in place the connection cannot */
        /* be closed until the trap receives a close indication.  */
        if (n == 0 && ttpty)
            goto ptyread;
#endif /* HAVE_PTYTRAP */
#endif /* NETPTY */
        return(-3);
    }
    return(n);
#endif /* BEOSORBEBOX */
#endif /* sxaE50 */
}

#else /* not AT&T or POSIX */

#ifdef aegis
	/* This is quoted from the old myread().  The semantics seem to be
	 * alright, but maybe errno would not need to be set even when
	 * there is no error?  I don't know aegis.
	 */
int
myfillbuf() {
    int count;
#ifdef NETCMD
    if (ttpipe)
      fd = fdin;
    else
#endif /* NETCMD */
      fd = ttyfd;

    count = ios_$get((short)fd, ios_$cond_opt, mybuf, 256L, st);
    errno = EIO;
    if (st.all == ios_$get_conditional_failed) /* get at least one */
      count = ios_$get((short)fd, 0, mybuf, 1L, st);
    if (st.all == ios_$end_of_file)
      return(-3);
    else if (st.all != status_$ok) {
	errno = EIO;
	return(-1);
    }
    return(count > 0 ? count : -3);
}
#else /* !aegis */

#ifdef FIONREAD
	/* This is for systems with FIONREAD.  FIONREAD returns the number
	 * of characters available for reading. If none are available, wait
	 * until something arrives, otherwise return all there is.
	 */
int
myfillbuf() {
    PEEKTYPE avail = 0;
    int x, fd;
#ifdef NETCMD
    if (ttpipe)
      fd = fdin;
    else
#endif /* NETCMD */
      fd = ttyfd;

#ifdef SUNX25
/*
  SunLink X.25 support in this routine from Stefaan A. Eeckels, Eurostat (CEC).
  Depends on SunOS having FIONREAD, not because we use it, but just so this
  code is grouped correctly within the #ifdefs.  Let's hope Solaris keeps it.

  We call x25xin() instead of read() so that Q-Bit packets, which contain
  X.25 service-level information (e.g. PAD parameter changes), can be processed
  transparently to the upper-level code.  This is a blocking read, and so
  we depend on higher-level code (such as ttinc()) to set any necessary alarms.
*/
    extern int nettype;
    if (netconn && nettype == NET_SX25) {
	while ((x = x25xin(sizeof(x25buf), x25buf)) < 1) ;
	return(x - 1);	        /* "-1" compensates for extra status byte */
    }
#endif /* SUNX25 */

#ifdef CK_SSL
    if (ssl_active_flag || tls_active_flag) {
        int error, n = 0;
        while (n == 0) {
            if (ssl_active_flag)
	      n = SSL_read(ssl_con, (char *)mybuf, sizeof(mybuf));
            else
	      n = SSL_read(tls_con, (char *)mybuf, sizeof(mybuf));
            switch (SSL_get_error(ssl_active_flag?ssl_con:tls_con,n)) {
	      case SSL_ERROR_NONE:
                if (n > 0)
		  return(n);
                if (n < 0)
		  return(-2);
                msleep(50);
                break;
	      case SSL_ERROR_WANT_WRITE:
	      case SSL_ERROR_WANT_READ:
                return(-1);
	      case SSL_ERROR_SYSCALL:
		if (n != 0)
		  return(-1);
	      case SSL_ERROR_WANT_X509_LOOKUP:
	      case SSL_ERROR_SSL:
	      case SSL_ERROR_ZERO_RETURN:
	      default:
                ttclos(0);
                return(-2);
            }
        }
    }
#endif /* CK_SSL */
#ifdef CK_KERBEROS
#ifdef KRB4
#ifdef RLOGCODE
    if (ttnproto == NP_EK4LOGIN) {
        if ((x = krb4_des_read(ttyfd,mybuf,sizeof(mybuf))) < 0)
	  return(-1);
        else
	  return(x);
    }
#endif /* RLOGCODE */
#endif /* KRB4 */
#ifdef KRB5
#ifdef RLOGCODE
    if (ttnproto == NP_EK5LOGIN) {
        if ((x = krb5_des_read(ttyfd,mybuf,sizeof(mybuf),0)) < 0)
	  return(-1);
        else
	  return(x);
    }
#endif /* RLOGCODE */
#ifdef KRB5_U2U
    if (ttnproto == NP_K5U2U) {
        if ((x = krb5_u2u_read(ttyfd,mybuf,sizeof(mybuf))) < 0)
	  return(-1);
        else
	  return(x);
    }
#endif /* KRB5_U2U */
#endif /* KRB5 */
#endif /* CK_KERBEROS */

    errno = 0;
    debug(F101,"myfillbuf calling FIONREAD ioctl","",xlocal);
    x = ioctl(fd, FIONREAD, &avail);
#ifdef DEBUG
    if (deblog) {
	debug(F101,"myfillbuf FIONREAD","",x);
	debug(F101,"myfillbuf FIONREAD avail","",avail);
	debug(F101,"myfillbuf FIONREAD errno","",errno);
    }
#endif /* DEBUG */
    if (x < 0 || avail == 0)
      avail = 1;

    if (avail > MYBUFLEN)
      avail = MYBUFLEN;

    errno = 0;

    x = read(fd, mybuf, (int) avail);
#ifdef DEBUG
    if (deblog) {
	debug(F101,"myfillbuf avail","",avail);
	debug(F101,"myfillbuf read","",x);
	debug(F101,"myfillbuf read errno","",errno);
        if (x > 0)
	  ckhexdump("myfillbuf mybuf",mybuf,x);
    }
#endif /* DEBUG */
    if (x < 1) x = -3;			/* read 0 == connection loss */
    return(x);
}

#else /* !FIONREAD */
/* Add other systems here, between #ifdef and #else, e.g. NETCONN. */
/* When there is no other possibility, read 1 character at a time. */
int
myfillbuf() {
    int x;

#ifdef CK_SSL
    if (ssl_active_flag || tls_active_flag) {
        int error, n = 0;
        while (n == 0) {
            if (ssl_active_flag)
	      n = SSL_read(ssl_con, (char *)mybuf, sizeof(mybuf));
            else
	      count = SSL_read(tls_con, (char *)mybuf, sizeof(mybuf));
            switch (SSL_get_error(ssl_active_flag?ssl_con:tls_con,n)) {
	      case SSL_ERROR_NONE:
                if (n > 0)
		  return(n);
                if (n < 0)
		  return(-2);
                msleep(50);
                break;
	      case SSL_ERROR_WANT_WRITE:
	      case SSL_ERROR_WANT_READ:
                return(-1);
	      case SSL_ERROR_SYSCALL:
		if (n != 0)
		  return(-1);
	      case SSL_ERROR_WANT_X509_LOOKUP:
	      case SSL_ERROR_SSL:
	      case SSL_ERROR_ZERO_RETURN:
	      default:
                ttclos(0);
                return(-2);
            }
        }
    }
#endif /* CK_SSL */
#ifdef CK_KERBEROS
#ifdef KRB4
#ifdef RLOGCODE
    if (ttnproto == NP_EK4LOGIN) {
        if ((len = krb4_des_read(ttyfd,mybuf,sizeof(mybuf))) < 0)
	  return(-1);
        else
	  return(len);
    }
#endif /* RLOGCODE */
#endif /* KRB4 */
#ifdef KRB5
#ifdef RLOGCODE
    if (ttnproto == NP_EK5LOGIN) {
        if ((len = krb5_des_read(ttyfd,mybuf,sizeof(mybuf),0)) < 0)
	  return(-1);
        else
	  return(len);
    }
#endif /* RLOGCODE */
#ifdef KRB5_U2U
    if (ttnproto == NP_K5U2U) {
        if ((len = krb5_u2u_read(ttyfd,mybuf,sizeof(mybuf))) < 0)
	  return(-1);
        else
	  return(len);
    }
#endif /* KRB5_U2U */
#endif /* KRB5 */
#endif /* CK_KERBEROS */

#ifdef NETCMD
    if (ttpipe)
      fd = fdin;
    else
#endif /* NETCMD */
      fd = ttyfd;
    x = read(fd, mybuf, 1);
    return(x > 0 ? x : -3);
}

#endif /* !FIONREAD */
#endif /* !aegis */
#endif /* !ATTSV */

#endif /* MYREAD */

/*  T T _ T N O P T  --  Handle Telnet negotions in incoming data */
/*
  Call with the IAC that was encountered.
  Returns:
   -3: If connection has dropped or gone bad.
   -2: On Telnet protocol error resulting in inconsistent states.
    0: If negotiation OK and caller has nothing to do.
    1: If packet start character has changed (new value is in global stchr).
  255: If there was a quoted IAC as data.
   or: Not at all if we got a legitimate Telnet Logout request.
*/
#ifdef TCPSOCKET
static int
tt_tnopt(n) int n; {			/* Handle Telnet options */
    /* In case caller did not already check these conditions...  */
    if (n == IAC &&
	((xlocal && netconn && IS_TELNET()) ||
	 (!xlocal && sstelnet))) {
	extern int server;
	int tx = 0;
	debug(F100,"ttinl calling tn_doop()","",0);
	tx = tn_doop((CHAR)(n & 0xff),duplex,ttinc);
	debug(F111,"ttinl tn_doop() returned","tx",tx);
	switch (tx) {
	  case 0:
	    return(0);
	  case -1:			/* I/O error */
	    ttimoff();			/* Turn off timer */
	    return(-3);
          case -2:			/* Connection failed. */
          case -3:
	    ttimoff();			/* Turn off timer */
	    ttclos(0);
	    return(-3);
	  case 1:			/* ECHO change */
	    duplex = 1;
	    return(0);
	  case 2:			/* ECHO change */
	    duplex = 0;
	    return(0);
	  case 3:			/* Quoted IAC */
	    n = 255;
	    return((unsigned)255);
#ifdef IKS_OPTION
	  case 4: {
	      if (TELOPT_SB(TELOPT_KERMIT).kermit.u_start && server
#ifdef IKSD
		  && !inserver
#endif /* IKSD */
		  ) {			/* Remote in Server mode */
		  ttimoff();		/* Turn off timer */
		  debug(F100,"u_start and !inserver","",0);
		  return(-2);		/* End server mode */
	      } else if (!TELOPT_SB(TELOPT_KERMIT).kermit.me_start &&
			 server
			 ) {		/* I'm no longer in Server Mode */
		  debug(F100,"me_start and server","",0);
		  ttimoff();
		  return(-2);
	      }
	      return(0);
	  }
	  case 5: {			/* Start character change */
	      /* extern CHAR stchr; */
	      /* start = stchr; */
	      return(1);
	  }
#endif /* IKS_OPTION */
	  case 6:			/* Remote Logout */
	    ttimoff();
	    ttclos(0);
#ifdef IKSD
	    if (inserver && !local)
	      doexit(GOOD_EXIT,0);
	    else
#endif /* IKSD */
	      return(-2);
	  default:
	    return(0);
	}
    } else
      return(0);
}
#endif /* TCPSOCKET */

/*  T T F L U I  --  Flush tty input buffer */

void
ttflux() {				/* But first... */
#ifdef MYREAD
/*
  Flush internal MYREAD buffer.
*/
#ifdef TCPSOCKET
    int dotnopts, x;
    dotnopts = (((xlocal && netconn && IS_TELNET()) ||
		 (!xlocal && sstelnet)));
#endif /* TCPSOCKET */
    debug(F101,"ttflux my_count","",my_count);
#ifdef TCPSOCKET
    if (dotnopts) {
	CHAR ch = '\0';
        while (my_count > 0) {
	    ch = myread();
#ifdef CK_ENCRYPTION
            if (TELOPT_U(TELOPT_ENCRYPTION))
	      ck_tn_decrypt((char *)&ch,1);
#endif /* CK_ENCRYPTION */
            if (ch == IAC)
	      x = tt_tnopt(ch);
        }
    } else
#endif /* TCPSOCKET */
#ifdef COMMENT
#ifdef CK_ENCRYPTION
    if (TELOPT_U(TELOPT_ENCRYPTION) && my_count > 0)
      ck_tn_decrypt(&mybuf[my_item+1],my_count);
#endif /* CK_ENCRYPTION */
#endif /* COMMENT */
    my_count = 0;			/* Reset count to zero */
    my_item = -1;			/* And buffer index to -1 */
#endif /* MYREAD */
}

int
ttflui() {
    int n, fd;
#ifdef TCPSOCKET
    int dotnopts;
    dotnopts = (((xlocal && netconn && IS_TELNET()) ||
		 (!xlocal && sstelnet)));
#endif /* TCPSOCKET */

#ifdef NETCMD
    if (ttpipe)
      fd = fdin;
    else
#endif /* NETCMD */
      fd = ttyfd;

#ifdef TTLEBUF
    ttpush = -1;			/* Clear the peek-ahead char */
    while (le_data && (le_inbuf() > 0)) {
        CHAR ch = '\0';
        if (le_getchar(&ch) > 0) {	/* Clear any more... */
            debug(F101,"ttflui le_inbuf ch","",ch);
        }
    }
#endif /* TTLEBUF */
    debug(F101,"ttflui ttpipe","",ttpipe);

#ifdef MYREAD
/*
  Flush internal MYREAD buffer *NEXT*, in all cases.
*/
    ttflux();
#endif /* MYREAD */

#ifdef NETCONN
/* Network flush is done specially, in the network support module. */
    if ((netconn || sstelnet) && !ttpipe && !ttpty) {
	debug(F100,"ttflui netflui","",0);
#ifdef COMMENT
#ifdef TN_COMPORT
	if (istncomport())
            tnc_send_purge_data(TNC_PURGE_RECEIVE);
#endif /* TN_COMPORT */
#endif /* COMMENT */
	return(netflui());
    }
#endif /* NETCONN */

    debug(F101,"ttflui ttyfd","",ttyfd); /* Not network */
    if (ttyfd < 0)
      return(-1);

#ifdef aegis
    sio_$control((short)yfd, sio_$flush_in, true, st);
    if (st.all != status_$ok) {
	fprintf(stderr, "flush failed: "); error_$print(st);
    } else {      /* sometimes the flush doesn't work */
        for (;;) {
	    char buf[256];
            /* eat all the characters that shouldn't be available */
            ios_$get((short)fd, ios_$cond_opt, buf, 256L, st); /* (void) */
            if (st.all == ios_$get_conditional_failed) break;
            fprintf(stderr, "flush failed(2): "); error_$print(st);
        }
    }
#else
#ifdef BSD44				/* 4.4 BSD */
    n = FREAD;                          /* Specify read queue */
    debug(F100,"ttflui BSD44","",0);
    ioctl(fd,TIOCFLUSH,&n);
#else
#ifdef Plan9
#undef POSIX				/* Uh oh... */
#endif /* Plan9 */
#ifdef POSIX				/* POSIX */
    debug(F100,"ttflui POSIX","",0);
    tcflush(fd,TCIFLUSH);
#else
#ifdef ATTSV				/* System V */
#ifndef VXVE
    debug(F100,"ttflui ATTSV","",0);
    ioctl(fd,TCFLSH,0);
#endif /* VXVE */
#else					/* Not BSD44, POSIX, or Sys V */
#ifdef TIOCFLUSH			/* Those with TIOCFLUSH defined */
#ifdef ANYBSD				/* Berkeley */
    n = FREAD;                          /* Specify read queue */
    debug(F100,"ttflui TIOCFLUSH ANYBSD","",0);
    ioctl(fd,TIOCFLUSH,&n);
#else					/* Others (V7, etc) */
    debug(F100,"ttflui TIOCFLUSH","",0);
    ioctl(fd,TIOCFLUSH,0);
#endif /* ANYBSD */
#else					/* All others... */
/*
  No system call (that we know about) for input buffer flushing.
  So see how many there are and read them in a loop, using ttinc().
  ttinc() is buffered, so we're not getting charged with a system call
  per character, just a function call.
*/
    if ((n = ttchk()) > 0) {
	debug(F101,"ttflui read loop","",n);
	while ((n--) && ttinc(0) > 0) ;
    }
#endif /* TIOCFLUSH */
#endif /* ATTSV */
#endif /* POSIX */
#ifdef Plan9
#define POSIX
#endif /* Plan9 */
#endif /* BSD44 */
#endif /* aegis */
    return(0);
}

int
ttfluo() {				/* Flush output buffer */
    int fd;
#ifdef NETCMD
    if (ttpipe)
      fd = fdout;
    else
#endif /* NETCMD */
      fd = ttyfd;

#ifdef Plan9
    return 0;
#else
#ifdef POSIX
    return(tcflush(fd,TCOFLUSH));
#else
#ifdef OXOS
    return(ioctl(fd,TCFLSH,1));
#else
    return(0);				/* All others, nothing */
#endif /* OXOS */
#endif /* POSIX */
#endif /* Plan9 */
}

/* Interrupt Functions */

/* Set up terminal interrupts on console terminal */

#ifndef FIONREAD			/* We don't need esctrp() */
#ifndef SELECT				/* if we have any of these... */
#ifndef CK_POLL
#ifndef RDCHK

#ifndef OXOS
#ifdef SVORPOSIX
SIGTYP
esctrp(foo) int foo; {			/* trap console escapes (^\) */
    signal(SIGQUIT,SIG_IGN);            /* ignore until trapped */
    conesc = 1;
    debug(F101,"esctrp caught SIGQUIT","",conesc);
}
#endif /* SVORPOSIX */
#endif /* OXOS */

#ifdef V7
#ifndef MINIX2
SIGTYP
esctrp(foo) int foo; {			/* trap console escapes (^\) */
    signal(SIGQUIT,SIG_IGN);            /* ignore until trapped */
    conesc = 1;
    debug(F101,"esctrp caught SIGQUIT","",conesc);
}
#endif /* MINIX2 */
#endif /* V7 */

#ifdef C70
SIGTYP
esctrp(foo) int foo; {			/* trap console escapes (^\) */
    conesc = 1;
    signal(SIGQUIT,SIG_IGN);            /* ignore until trapped */
}
#endif /* C70 */

#endif /* RDCHK */
#endif /* CK_POLL */
#endif /* SELECT */
#endif /* FIONREAD */

/*  C O N B G T  --  Background Test  */

static int jc = 0;			/* 0 = no job control */

/*
  Call with flag == 1 to prevent signal test, which can not be expected
  to work during file transfer, when SIGINT probably *is* set to SIG_IGN.

  Call with flag == 0 to use the signal test, but only if the process-group
  test fails, as it does on some UNIX systems, where getpgrp() is buggy,
  requires an argument when the man page says it doesn't, or vice versa.

  If flag == 0 and the process-group test fails, then we determine background
  status simply (but not necessarily reliably) from isatty().

  conbgt() sets the global backgrd = 1 if we appear to be in the background,
  and to 0 if we seem to be in the foreground.  conbgt() is highly prone to
  misbehavior.
*/
VOID
conbgt(flag) int flag; {
    int x = -1,				/* process group or SIGINT test */
        y = 0;				/* isatty() test */
/*
  Check for background operation, even if not running on real tty, so that
  background flag can be set correctly.  If background status is detected,
  then Kermit will not issue its interactive prompt or most messages.
  If your prompt goes away, you can blame (and fix?) this function.
*/

/* Use process-group test if possible. */

#ifdef POSIX				/* We can do it in POSIX */
#define PGROUP_T
#else
#ifdef BSD4				/* and in BSD 4.x. */
#define PGROUP_T
#else
#ifdef HPUXJOBCTL			/* and in most HP-UX's */
#define PGROUP_T
#else
#ifdef TIOCGPGRP			/* and anyplace that has this ioctl. */
#define PGROUP_T
#endif /* TIOCGPGRP */
#endif /* HPUXJOBCTL */
#endif /* BSD4 */
#endif /* POSIX */

#ifdef MIPS				/* Except if it doesn't work... */
#undef PGROUP_T
#endif /* MIPS */

#ifdef MINIX
#undef PGROUP_T
#endif	/* MINIX */

#ifdef PGROUP_T
/*
  Semi-reliable process-group test.  Check whether this process's group is
  the same as the controlling terminal's process group.  This works if the
  getpgrp() call doesn't lie (as it does in the SUNOS System V environment).
*/
    PID_T mypgrp = (PID_T)0;		/* Kermit's process group */
    PID_T ctpgrp = (PID_T)0;		/* The terminal's process group */
#ifndef _POSIX_SOURCE
/*
  The getpgrp() prototype is obtained from system header files for POSIX
  and Sys V R4 compilations.  Other systems, who knows.  Some complain about
  a duplicate declaration here, others don't, so it's safer to leave it in
  if we don't know for certain.
*/
#ifndef SVR4
#ifndef PS2AIX10
#ifndef HPUX9
    extern PID_T getpgrp();
#endif /* HPUX9 */
#endif /* PS2AIX10 */
#endif /* SVR4 */
#endif /* _POSIX_SOURCE */

/* Get my process group. */

#ifdef SVR3 /* Maybe this should be ATTSV? */
/* This function is not described in SVID R2 */
    mypgrp = getpgrp();
    /* debug(F101,"ATTSV conbgt process group","",(int) mypgrp); */
#else
#ifdef POSIX
    mypgrp = getpgrp();
    /* debug(F101,"POSIX conbgt process group","",(int) mypgrp); */
#else
#ifdef OSFPC
    mypgrp = getpgrp();
    /* debug(F101,"OSF conbgt process group","",(int) mypgrp); */
#else
#ifdef QNX
    mypgrp = getpgrp();
    /* debug(F101,"QNX conbgt process group","",(int) mypgrp); */
#else
#ifdef OSF32				/* (was OSF40) */
    mypgrp = getpgrp();
    /* debug(F101,"Digital UNIX conbgt process group","",(int) mypgrp); */
#else /* BSD, V7, etc */
#ifdef MINIX2
    mypgrp = getpgrp();
#else
    mypgrp = getpgrp(0);
#endif /* MINIX2 */
    /* debug(F101,"BSD conbgt process group","",(int) mypgrp); */
#endif /* OSF32 */
#endif /* QNX */
#endif /* OSFPC */
#endif /* POSIX */
#endif /* SVR3 */

#ifdef MINIX
    /* MINIX does not support job control so Kermit is always in foreground */
    x = 0;

#else  /* Not MINIX */

/* Now get controlling tty's process group */
#ifdef BSD44ORPOSIX
    ctpgrp = tcgetpgrp(1);		/* The POSIX way */
    /* debug(F101,"POSIX conbgt terminal process group","",(int) ctpgrp); */
#else
    ioctl(1, TIOCGPGRP, &ctpgrp);	/* Or the BSD way */
   /* debug(F101,"non-POSIX conbgt terminal process group","",(int) ctpgrp); */
#endif /* BSD44ORPOSIX */

    if ((mypgrp > (PID_T) 0) && (ctpgrp > (PID_T) 0))
      x = (mypgrp == ctpgrp) ? 0 : 1;	/* If they differ, then background. */
    else x = -1;			/* If error, remember. */
    debug(F101,"conbgt process group test","",x);
#endif /* PGROUP_T */
#endif	/* MINIX */

/* Try to see if job control is available */

#ifdef NOJC				/* User override */
    jc = 0;				/* No job control allowed */
    debug(F111,"NOJC","jc",jc);
#else
#ifdef BSD44
    jc = 1;
#else
#ifdef SVR4ORPOSIX			/* POSIX actually tells us */
    debug(F100,"SVR4ORPOSIX jc test...","",0);
#ifdef _SC_JOB_CONTROL
#ifdef __bsdi__
    jc = 1;
#else
#ifdef __386BSD__
    jc = 1;
#else
    jc = sysconf(_SC_JOB_CONTROL);	/* Whatever system says */
    if (jc < 0) {
	debug(F101,"sysconf fails, jcshell","",jcshell);
	jc = (jchdlr == SIG_DFL) ? 1 : 0;
    } else
      debug(F111,"sysconf(_SC_JOB_CONTROL)","jc",jc);
#endif /* __386BSD__ */
#endif /* __bsdi__ */
#else
#ifdef _POSIX_JOB_CONTROL
    jc = 1;				/* By definition */
    debug(F111,"_POSIX_JOB_CONTROL is defined","jc",jc);
#else
    jc = 0;				/* Assume job control not allowed */
    debug(F111,"SVR4ORPOSIX _SC/POSIX_JOB_CONTROL not defined","jc",jc);
#endif /* _POSIX_JOB_CONTROL */
#endif /* _SC_JOB_CONTROL */
#else
#ifdef BSD4
    jc = 1;				/* Job control allowed */
    debug(F111,"BSD job control","jc",jc);
#else
#ifdef SVR3JC
    jc = 1;				/* JC allowed */
    debug(F111,"SVR3 job control","jc",jc);
#else
#ifdef OXOS
    jc = 1;				/* JC allowed */
    debug(F111,"X/OS job control","jc",jc);
#else
#ifdef HPUX9
    jc = 1;				/* JC allowed */
    debug(F111,"HP-UX 9.0 job control","jc",jc);
#else
#ifdef HPUX10
    jc = 1;				/* JC allowed */
    debug(F111,"HP-UX 10.0 job control","jc",jc);
#else
    jc = 0;				/* JC not allowed */
    debug(F111,"job control catch-all","jc",jc);
#endif /* HPUX10 */
#endif /* HPUX9 */
#endif /* OXOS */
#endif /* SVR3JC */
#endif /* BSD4 */
#endif /* SVR4ORPOSIX */
#endif /* BSD44 */
#endif /* NOJC */
    debug(F101,"conbgt jc","",jc);
#ifndef NOJC
    debug(F101,"conbgt jcshell","",jcshell);
/*
  At this point, if jc == 1 but jcshell == 0, it means that the OS supports
  job control, but the shell or other process we are running under does not
  (jcshell is set in sysinit()) and so if we suspend ourselves, nothing good
  will come of it.  So...
*/
    if (jc < 0) jc = 0;
    if (jc > 0 && jcshell == 0) jc = 0;
#endif /* NOJC */

/*
  Another background test.
  Test if SIGINT (terminal interrupt) is set to SIG_IGN (ignore),
  which is done by the shell (sh) if the program is started with '&'.
  Unfortunately, this is NOT done by csh or ksh so watch out!
  Note, it's safe to set SIGINT to SIG_IGN here, because further down
  we always set it to something else.
  Note: as of 16 Jul 1999, we also skip this test if we set SIGINT to
  SIG_IGN ourselves.
*/
    if (x < 0 && !flag && !sigint_ign) { /* Didn't get good results above... */

	SIGTYP (*osigint)();

	osigint = signal(SIGINT,SIG_IGN);	/* What is SIGINT set to? */
	sigint_ign = 1;
	x = (osigint == SIG_IGN) ? 1 : 0;	/* SIG_IGN? */
	/* debug(F101,"conbgt osigint","",osigint); */
	/* debug(F101,"conbgt signal test","",x); */
    }

/* Also check to see if we're running with redirected stdio. */
/* This is not really background operation, but we want to act as though */
/* it were. */

#ifdef IKSD
    if (inserver) {			/* Internet Kermit Server */
	backgrd = 0;			/* is not in the background */
	return;
    }
#endif /* IKSD */

    y = (isatty(0) && isatty(1)) ? 1 : 0;
    debug(F101,"conbgt isatty test","",y);

#ifdef BSD29
/* The process group and/or signal test doesn't work under these... */
    backgrd = !y;
#else
#ifdef sxaE50
    backgrd = !y;
#else
#ifdef MINIX
    backgrd = !y;
#else
#ifdef MINIX2
    backgrd = !y;
#else
    if (x > -1)
      backgrd = (x || !y) ? 1 : 0;
    else backgrd = !y;
#endif /* BSD29 */
#endif /* sxaE50 */
#endif /* MINIX */
#endif /* MINIX2 */
    debug(F101,"conbgt backgrd","",backgrd);
}

/*  C O N I N T  --  Console Interrupt setter  */

/*
  First arg is pointer to function to handle SIGTERM & SIGINT (like Ctrl-C).
  Second arg is pointer to function to handle SIGTSTP (suspend).
*/

VOID					/* Set terminal interrupt traps. */
#ifdef CK_ANSIC
#ifdef apollo
conint(f,s) SIGTYP (*f)(), (*s)();
#else
conint(SIGTYP (*f)(int), SIGTYP (*s)(int))
#endif /* apollo */
#else
conint(f,s) SIGTYP (*f)(), (*s)();
#endif /* CK_ANSIC */
/* conint */ {

    debug(F101,"conint conistate","",conistate);

    conbgt(0);				/* Do background test. */

/* Set the desired handlers for hangup and software termination. */

#ifdef SIGTERM
    signal(SIGTERM,f);                  /* Software termination */
#endif /* SIGTERM */

/*
  Prior to July 1999 we used to call sighup() here but now it's called in
  sysinit() so SIGHUP can be caught during execution of the init file or
  a kerbang script.
*/

/* Now handle keyboard stop, quit, and interrupt signals. */
/* Check if invoked in background -- if so signals set to be ignored. */
/* However, if running under a job control shell, don't ignore them. */
/* We won't be getting any, as we aren't in the terminal's process group. */

    debug(F101,"conint backgrd","",backgrd);
    debug(F101,"conint jc","",jc);

    if (backgrd && !jc) {		/* In background, ignore signals */
	debug(F101,"conint background ignoring signals, jc","",jc);
#ifdef SIGTSTP
        signal(SIGTSTP,SIG_IGN);        /* Keyboard stop */
#endif /* SIGTSTP */
        signal(SIGQUIT,SIG_IGN);        /* Keyboard quit */
        signal(SIGINT,SIG_IGN);         /* Keyboard interrupt */
	sigint_ign = 1;
	conistate = CONI_NOI;
    } else {				/* Else in foreground or suspended */
	debug(F101,"conint foreground catching signals, jc","",jc);
        signal(SIGINT,f);               /* Catch terminal interrupt */
	sigint_ign = (f == SIG_IGN) ? 1 : 0;

#ifdef SIGTSTP				/* Keyboard stop (suspend) */
	/* debug(F101,"conint SIGSTSTP","",s); */
	if (s == NULL) s = SIG_DFL;
#ifdef NOJC				/* No job control allowed. */
	signal(SIGTSTP,SIG_IGN);
#else					/* Job control allowed */
	if (jc)				/* if available. */
	  signal(SIGTSTP,s);
	else
	  signal(SIGTSTP,SIG_IGN);
#endif /* NOJC */
#endif /* SIGTSTP */

#ifndef OXOS
#ifdef SVORPOSIX
#ifndef FIONREAD			/* Watch out, we don't know this... */
#ifndef SELECT
#ifndef CK_POLL
#ifndef RDCHK
        signal(SIGQUIT,esctrp);         /* Quit signal, Sys III/V. */
#endif /* RDCHK */
#endif /* CK_POLL */
#endif /* SELECT */
#endif /* FIONREAD */
        if (conesc) conesc = 0;         /* Clear out pending escapes */
#else
#ifdef V7
        signal(SIGQUIT,esctrp);         /* V7 like Sys III/V */
        if (conesc) conesc = 0;
#else
#ifdef aegis
        signal(SIGQUIT,f);              /* Apollo, catch it like others. */
#else
        signal(SIGQUIT,SIG_IGN);        /* Others, ignore like 4D & earlier. */
#endif /* aegis */
#endif /* V7 */
#endif /* SVORPOSIX */
#endif /* OXOS */
	conistate = CONI_INT;
    }
}


/*  C O N N O I  --  Reset console terminal interrupts */

VOID
connoi() {                              /* Console-no-interrupts */

    debug(F101,"connoi conistate","",conistate);
#ifdef SIGTSTP
    signal(SIGTSTP,SIG_IGN);		/* Suspend */
#endif /* SIGTSTP */
    conint(SIG_IGN,SIG_IGN);		/* Interrupt */
    sigint_ign = 1;			/* Remember we did this ourselves */
#ifdef SIGQUIT
    signal(SIGQUIT,SIG_IGN);		/* Quit */
#endif /* SIGQUIT */
#ifdef SIGTERM
    signal(SIGTERM,SIG_IGN);		/* Term */
#endif /* SIGTERM */
    conistate = CONI_NOI;
}

/*  I N I T R A W Q  --  Set up to read /dev/kmem for character count.  */

#ifdef  V7
/*
 Used in Version 7 to simulate Berkeley's FIONREAD ioctl call.  This
 eliminates blocking on a read, because we can read /dev/kmem to get the
 number of characters available for raw input.  If your system can't
 or you won't let the world read /dev/kmem then you must figure out a
 different way to do the counting of characters available, or else replace
 this by a dummy function that always returns 0.
*/
/*
 * Call this routine as: initrawq(tty)
 * where tty is the file descriptor of a terminal.  It will return
 * (as a char *) the kernel-mode memory address of the rawq character
 * count, which may then be read.  It has the side-effect of flushing
 * input on the terminal.
 */
/*
 * John Mackin, Physiology Dept., University of Sydney (Australia)
 * ...!decvax!mulga!physiol.su.oz!john
 *
 * Permission is hereby granted to do anything with this code, as
 * long as this comment is retained unmodified and no commercial
 * advantage is gained.
 */
#ifndef MINIX
#ifndef MINIX2
#ifndef COHERENT
#include <a.out.h>
#include <sys/proc.h>
#endif /* COHERENT */
#endif /* MINIX2 */
#endif /* MINIX */

#ifdef COHERENT
#include <l.out.h>
#include <sys/proc.h>
#endif /* COHERENT */

char *
initrawq(tty) int tty; {
#ifdef MINIX
    return(0);
#else
#ifdef MINIX2
    return(0);
#else
#ifdef UTS24
    return(0);
#else
#ifdef BSD29
    return(0);
#else
    long lseek();
    static struct nlist nl[] = {
        {PROCNAME},
        {NPROCNAME},
        {""}
    };
    static struct proc *pp;
    char *qaddr, *p, c;
    int m;
    PID_T pid, me;
    NPTYPE xproc;                       /* Its type is defined in makefile. */
    int catch();

    me = getpid();
    if ((m = open("/dev/kmem", 0)) < 0) err("kmem");
    nlist(BOOTNAME, nl);
    if (nl[0].n_type == 0) err("proc array");

    if (nl[1].n_type == 0) err("nproc");

    lseek(m, (long)(nl[1].n_value), 0);
    read (m, &xproc, sizeof(xproc));
    saval = signal(SIGALRM, catch);
    if ((pid = fork()) == 0) {
        while(1)
            read(tty, &c, 1);
    }
    alarm(2);

    if(setjmp(jjbuf) == 0) {
        while(1)
	  read(tty, &c, 1);
    }
    signal(SIGALRM, SIG_DFL);

#ifdef DIRECT
    pp = (struct proc *) nl[0].n_value;
#else
    if (lseek(m, (long)(nl[0].n_value), 0) < 0L) err("seek");
    if (read(m, &pp, sizeof(pp)) != sizeof(pp))  err("no read of proc ptr");
#endif
    lseek(m, (long)(nl[1].n_value), 0);
    read(m, &xproc, sizeof(xproc));

    if (lseek(m, (long)pp, 0) < 0L) err("Can't seek to proc");
    if ((p = malloc(xproc * sizeof(struct proc))) == NULL) err("malloc");
    if (read(m,p,xproc * sizeof(struct proc)) != xproc*sizeof(struct proc))
        err("read proc table");
    for (pp = (struct proc *)p; xproc > 0; --xproc, ++pp) {
        if (pp -> p_pid == (short) pid) goto iout;
    }
    err("no such proc");

iout:
    close(m);
    qaddr = (char *)(pp -> p_wchan);
    free (p);
    kill(pid, SIGKILL);
    wait((WAIT_T *)0);
    return (qaddr);
#endif
#endif
#endif
#endif
}

/*  More V7-support functions...  */

static VOID
err(s) char *s; {
    char buf[200];

    ckmakmsg(buf,200,"fatal error in initrawq: ", s, NULL, NULL);
    perror(buf);
    doexit(1,-1);
}

static VOID
catch(foo) int foo; {
    longjmp(jjbuf, -1);
}


/*  G E N B R K  --  Simulate a modem break.  */

#ifdef MINIX
#define BSPEED B110
#else
#ifdef MINIX2
#define BSPEED B110
#else
#define BSPEED B150
#endif /* MINIX2 */
#endif /* MINIX */

#ifndef MINIX2
VOID
genbrk(fn,msec) int fn, msec; {
    struct sgttyb ttbuf;
    int ret, sospeed, x, y;

    ret = ioctl(fn, TIOCGETP, &ttbuf);
    sospeed = ttbuf.sg_ospeed;
    ttbuf.sg_ospeed = BSPEED;
    ret = ioctl(fn, TIOCSETP, &ttbuf);
    y = (int)strlen(brnuls);
    x = ( BSPEED * 100 ) / msec;
    if (x > y) x = y;
    ret = write(fn, brnuls, (( BSPEED * 100 ) / msec ));
    ttbuf.sg_ospeed = sospeed;
    ret = ioctl(fn, TIOCSETP, &ttbuf);
    ret = write(fn, "@", 1);
    return;
}
#endif /* MINIX2 */

#ifdef MINIX2
int
genbrk(fn,msec) int fn, msec; {
    struct termios ttbuf;
    int ret, x, y;
    speed_t sospeed;

    ret = tcgetattr(fn, &ttbuf);
    sospeed = ttbuf.c_ospeed;
    ttbuf.c_ospeed = BSPEED;
    ret = tcsetattr(fn,TCSADRAIN, &ttbuf);
    y = (int)strlen(brnuls);
    x = ( BSPEED * 100 ) / msec;
    if (x > y) x = y;
    ret = write(fn, brnuls, (( BSPEED * 100 ) / msec ));
    ttbuf.c_ospeed = sospeed;
    ret = tcsetattr(fn, TCSADRAIN, &ttbuf);
    ret = write(fn, "@", 1);
    return ret;
}
#endif /* MINIX2 */
#endif /* V7 */

/*
  I N C H K  --  Check if chars waiting to be read on given file descriptor.

  This routine is a merger of ttchk() and conchk().
  Call with:
    channel == 0 to check console.
    channel == 1 to check communications connection.
  and:
    fd = file descriptor.
  Returns:
   >= 0: number of characters waiting, 0 or greater,
     -1: on any kind of error,
     -2: if there is (definitely) no connection.
  Note: In UNIX we don't have to call nettchk() because a socket
  file descriptor works just like in serial i/o, ioctls and all.
  (But this will change if we add non-file-descriptor channels,
  such as IBM X.25 for AIX...)
*/
static int
in_chk(channel, fd) int channel, fd; {
    int x, n = 0;			/* Workers, n = return value */
    extern int clsondisc;		/* Close on disconnect */
/*
  The first section checks to make sure we have a connection,
  but only if we're in local mode.
*/
#ifdef DEBUG
    if (deblog) {
	debug(F111,"in_chk entry",ckitoa(fd),channel);
	debug(F101,"in_chk ttyfd","",ttyfd);
	debug(F101,"in_chk ttpty","",ttpty);
    }
#endif /* DEBUG */
/*
  But don't say connection is gone if we have any buffered-stuff.
*/
#ifdef TTLEBUF
    debug(F101,"in_chk ttpush","",ttpush);
    if (channel == 1) {
	if (ttpush >= 0)
	  n++;
	n += le_inbuf();
	if (n > 0)
	  return(n);
    }
#endif /* TTLEBUF */

#ifdef NETPTY
#ifdef HAVE_PTYTRAP
    /* Special handling for HP-UX pty i/o */
    if (ttpty && pty_trap_pending(ttyfd) > 0) {
        if (pty_trap_handler(ttyfd) > 0) {
            ttclos(0);
            return(-2);
        }
    }
#endif /* HAVE_PTYTRAP */
#endif /* NETPTY */

    if (channel) {			/* Checking communications channel */
	if (ttyfd < 0) {		/* No connection */
	  return(-2);			/* That's what this means */
	} else if (xlocal &&		/* In local mode */
		   (!netconn		/* Serial connection or */
#ifdef TN_COMPORT
		    || istncomport()    /* Telnet Com Port */
#endif /* TN_COMPORT */
		   ) && ttcarr != CAR_OFF /* with CARRIER WATCH ON (or AUTO) */
#ifdef COMMENT
#ifdef MYREAD
/*
  Seems like this would be a good idea but it prevents C-Kermit from
  popping back to the prompt automatically when carrier drops.  However,
  commenting this out prevents us from seeing the NO CARRIER message.
  Needs more work...
*/
		   && my_count < 1	/* Nothing in our internal buffer */
#endif /* MYREAD */
#endif /* COMMENT */
		   ) {
	    int x;
	    x = ttgmdm();		/* So get modem signals */
	    debug(F101,"in_chk close-on-disconnect","",clsondisc);
	    if (x > -1) {		/* Check for carrier */
		if (!(x & BM_DCD)) {	/* No carrier */
		    debug(F101,"in_chk carrier lost","",x);
		    if (clsondisc)	/* If "close-on-disconnect" */
		      ttclos(0);	/* close device & release lock. */
		    return(-2);		/* This means "disconnected" */
		}
	    /* In case I/O to device after CD dropped always fails */
	    /* as in Debian Linux 2.1 and Unixware 2.1... */
	    } else {
	        debug(F101,"in_chk ttgmdm I/O error","",errno);
	        debug(F101,"in_chk ttgmdm gotsigs","",gotsigs);
	        if (gotsigs) {		/* If we got signals before... */
		    if (errno == 5 || errno == 6) { /* I/O error etc */
		        if (clsondisc)	/* like when modem hangs up */
			  ttclos(0);
			return(-2);
		    }
		}
		/* If we never got modem signals successfully on this */
		/* connection before, we can't conclude that THIS failure */
		/* means the connection was lost. */
		return(0);
	    }
	}
    }

/* We seem to have a connection so now see if any bytes are waiting on it */

#ifdef CK_SSL
    if (ssl_active_flag || tls_active_flag) {
        n += SSL_pending(ssl_active_flag?ssl_con:tls_con);
        debug(F101,"in_chk SSL_pending","",n);
        if (n < 0) {
            ttclos(0);
            return(-1);
        } else if (n > 0) {
            return(n);
        }
    }
#endif /* CK_SSL */
#ifdef RLOGCODE
#ifdef CK_KERBEROS
    /* It is not safe to read any data when using encrypted Klogin */
    if (ttnproto == NP_EK4LOGIN || ttnproto == NP_EK5LOGIN) {
#ifdef KRB4
        if (ttnproto == NP_EK4LOGIN) {
            n += krb4_des_avail(ttyfd);
            debug(F101,"in_chk krb4_des_avail","",n);
        }
#endif /* KRB4 */
#ifdef KRB5
        if (ttnproto == NP_EK5LOGIN) {
            n += krb5_des_avail(ttyfd);
            debug(F101,"in_chk krb5_des_avail","",n);
        }
#ifdef KRB5_U2U
        if (ttnproto == NP_K5U2U) {
            n += krb5_u2u_avail(ttyfd);
            debug(F101,"in_chk krb5_des_avail","",n);
        }
#endif /* KRB5_U2U */
#endif /* KRB5 */
        if (n < 0)			/* Is this right? */
	  return(-1);
        else
	  return(n);
    }
#endif /* CK_KERBEROS */
#endif /* RLOGCODE */

    errno = 0;				/* Reset this so we log good info */
#ifdef FIONREAD
    x = ioctl(fd, FIONREAD, &n);	/* BSD and lots of others */
#ifdef DEBUG				/* (the more the better) */
    if (deblog) {
	debug(F101,"in_chk FIONREAD return code","",x);
	debug(F101,"in_chk FIONREAD count","",n);
	debug(F101,"in_chk FIONREAD errno","",errno);
    }
#endif /* DEBUG */
#else /* FIONREAD not defined */
/*
  Here, if (netconn && ttnet == NET_TCPB), we might try calling recvmsg()
  with flags MSG_PEEK|MSG_DONTWAIT on the socket (ttyfd), except this is not
  portable (MSG_DONTWAIT isn't defined in any of the <sys/socket.h> files
  that I looked at, but it is needed to prevent the call from blocking), and
  the msghdr struct differs from place to place, so we would need another
  avalanche of ifdefs.  Still, when FIONREAD is not available, this is the
  only other known method of asking the OS for the *number* of characters
  available for reading.
*/
#ifdef V7				/* UNIX V7: look in kernel memory */
#ifdef MINIX
    n = 0;				/* But not in MINIX */
#else
#ifdef MINIX2
    n = 0;
#else
    lseek(kmem[TTY], (long) qaddr[TTY], 0); /* 7th Edition Unix */
    x = read(kmem[TTY], &n, sizeof(int));
    if (x != sizeof(int))
      n = 0;
#endif /* MINIX2 */
#endif /* MINIX */
#else /* Not V7 */
#ifdef PROVX1
    x = ioctl(fd, TIOCQCNT, &ttbuf);	/* DEC Pro/3xx Venix V.1 */
    n = ttbuf.sg_ispeed & 0377;		/* Circa 1984 */
    if (x < 0) n = 0;
#else
#ifdef MYREAD
/*
  Here we skip all the undependable and expensive calls below if we
  already have something in our internal buffer.  This tends to work quite
  nicely, so the only really bad case remaining is the one in which neither
  FIONREAD or MYREAD are defined, which is increasingly rare these days.
*/
    if (channel != 0 && my_count > 0) {
	debug(F101,"in_chk buf my_count","",my_count);
	n = my_count;			/* n was 0 before we got here */
	return(n);
    }
#endif /* MYREAD */
/*
  rdchk(), select(), and poll() tell us *if* data is available to be read, but
  not how much, so these should be used only as a final resort.  Especially
  since these calls tend to add a lot overhead.
*/
#ifdef RDCHK				/* This mostly SCO-specific */
    n = rdchk(fd);
    debug(F101,"in_chk rdchk","",n);
#else /* No RDCHK */
#ifdef SELECT
#ifdef Plan9
    /* Only allows select on the console ... don't ask */
    if (channel == 0)
#endif /* Plan9 */
      {
	fd_set rfds;			/* Read file descriptors */
#ifdef BELLV10
	FD_ZERO(rfds);			/* Initialize them */
	FD_SET(fd,rfds);		/* We want to look at this fd */
#else
	FD_ZERO(&rfds);			/* Initialize them */
	FD_SET(fd,&rfds);		/* We want to look at this fd */
	tv.tv_sec = tv.tv_usec = 0L;	/* A 0-valued timeval structure */
#endif /* BELLV10 */
#ifdef Plan9
	n = select( FD_SETSIZE, &rfds, (fd_set *)0, (fd_set *)0, &tv );
	debug(F101,"in_chk Plan 9 select","",n);
#else
#ifdef BELLV10
	n = select( 128, rfds, (fd_set *)0, (fd_set *)0, 0 );
	debug(F101,"in_chk BELLV10 select","",n);
#else
#ifdef BSD44
	n = select( FD_SETSIZE, &rfds, (fd_set *)0, (fd_set *)0, &tv );
	debug(F101,"in_chk BSD44 select","",n);
#else
#ifdef BSD43
	n = select( FD_SETSIZE, &rfds, (fd_set *)0, (fd_set *)0, &tv );
	debug(F101,"in_chk BSD43 select","",n);
#else
#ifdef SOLARIS
	n = select( FD_SETSIZE, &rfds, (fd_set *)0, (fd_set *)0, &tv );
	debug(F101,"in_chk SOLARIS select","",n);
#else
#ifdef QNX6
	n = select( FD_SETSIZE, &rfds, (fd_set *)0, (fd_set *)0, &tv );
	debug(F101,"in_chk QNX6 select","",n);
#else
#ifdef QNX
	n = select( FD_SETSIZE, &rfds, (fd_set *)0, (fd_set *)0, &tv );
	debug(F101,"in_chk QNX select","",n);
#else
#ifdef COHERENT
	n = select( FD_SETSIZE, &rfds, (fd_set *)0, (fd_set *)0, &tv );
	debug(F101,"in_chk COHERENT select","",n);
#else
#ifdef SVR4
	n = select( FD_SETSIZE, &rfds, (fd_set *)0, (fd_set *)0, &tv );
	debug(F101,"in_chk SVR4 select","",n);
#else
#ifdef __linux__
	n = select( FD_SETSIZE, &rfds, (fd_set *)0, (fd_set *)0, &tv );
	debug(F101,"in_chk LINUX select","",n);
#ifdef OSF
	n = select( FD_SETSIZE, &rfds, (fd_set *)0, (fd_set *)0, &tv );
	debug(F101,"in_chk OSF select","",n);
#else
	n = select( FD_SETSIZE, &rfds, (int *)0, (int *)0, &tv );
	debug(F101,"in_chk catchall select","",n);
#endif /* OSF */
#endif /* __linux__ */
#endif /* SVR4 */
#endif /* COHERENT */
#endif /* QNX */
#endif /* QNX6 */
#endif /* SOLARIS */
#endif /* BSD43 */
#endif /* BSD44 */
#endif /* BELLV10 */
#endif /* Plan9 */
    }
#else  /* Not SELECT */
#ifdef CK_POLL
    {
      struct pollfd pfd;

      pfd.fd = fd;
      pfd.events = POLLIN;
      pfd.revents = 0;
      n = poll(&pfd, 1, 0);
      debug(F101,"in_chk poll","",n);
      if ((n > 0) && (pfd.revents & POLLIN))
	n = 1;
    }
#endif /* CK_POLL */
#endif /* SELECT */
#endif /* RDCHK */
#endif /* PROVX1 */
#endif /* V7 */
#endif /* FIONREAD */

/* From here down, treat console and communication device differently... */

    if (channel == 0) {			/* Console */

#ifdef SVORPOSIX
#ifndef FIONREAD
#ifndef SELECT
#ifndef CK_POLL
#ifndef RDCHK
/*
  This is the hideous hack used in System V and POSIX systems that don't
  support FIONREAD, rdchk(), select(), poll(), etc, in which the user's
  CONNECT-mode escape character is attached to SIGQUIT.  Used, obviously,
  only on the console.
*/
	if (conesc) {			/* Escape character typed == SIGQUIT */
	    debug(F100,"in_chk conesc","",conesc);
	    conesc = 0;
	    signal(SIGQUIT,esctrp);	/* Restore signal */
	    n += 1;
	}
#endif /* RDCHK */
#endif /* CK_POLL */
#endif /* SELECT */
#endif /* FIONREAD */
#endif /* SVORPOSIX */

	return(n);			/* Done with console */
    }

    if (channel != 0) {			/* Communications connection */

#ifdef MYREAD
#ifndef FIONREAD
/*
  select() or rdchk(), etc, has told us that something is waiting, but we
  don't know how much.  So we do a read to get it and then we know.  Note:
  This read is NOT nonblocking if nothing is there (because of VMIN=1), but
  it should be safe in this case since the OS tells us at least one byte is
  waiting to be read, and MYREAD reads return as much as is there without
  waiting for any more.  Controlled tests on Solaris and Unixware (with
  FIONREAD deliberately undefined) show this to be true.
*/
	debug(F101,"in_chk read my_count","",my_count);
	debug(F101,"in_chk read n","",n);
	if (n > 0 && my_count == 0) {
	    /* This also catches disconnects etc */
	    /* Do what mygetbuf does except don't grab a character */
	    my_count = myfillbuf();
	    my_item = -1;		/* ^^^ */
	    debug(F101,"in_chk myfillbuf my_count","",my_count);
	    if (my_count < 0)
	      return(-1);
	    else
	      n = 0;			/* NB: n is replaced by my_count */
	}
#endif /* FIONREAD */
/*
  Here we add whatever we think is unread to what is still in our
  our internal buffer.  Thus the importance of setting n to 0 just above.
*/
	debug(F101,"in_chk my_count","",my_count);
	debug(F101,"in_chk n","",n);
	if (my_count > 0)
	  n += my_count;
#endif /* MYREAD */
    }
    debug(F101,"in_chk result","",n);

    /* Errors here don't prove the connection has dropped so just say 0 */

    return(n < 0 ? 0 : n);
}


/*  T T C H K  --  Tell how many characters are waiting in tty input buffer  */

int
ttchk() {
    int fd;
#ifdef NETCMD
    if (ttpipe)
      fd = fdin;
    else
#endif /* NETCMD */
      fd = ttyfd;
    return(in_chk(1,fd));
}

/*  T T X I N  --  Get n characters from tty input buffer  */

/*  Returns number of characters actually gotten, or -1 on failure  */

/*  Intended for use only when it is known that n characters are actually */
/*  Available in the input buffer.  */

int
ttxin(n,buf) int n; CHAR *buf; {
    register int x = 0, c = -2;
#ifdef TTLEBUF
    register int i = 0;
#endif /* TTLEBUF */
    int fd;

    if (n < 1)				/* Nothing to do */
      return(0);

#ifdef TTLEBUF
    if (ttpush >= 0) {
        buf[0] = ttpush;		/* Put pushed char in buffer*/
        ttpush = -1;			/* Clear the push buffer */
        if (ttchk() > 0)
	  return(ttxin(n-1, &buf[1]) + 1);
        else
	  return(1);
    }
    if (le_data) {
        while (le_inbuf() > 0) {
	    if (le_getchar(&buf[i])) {
                i++;
                n--;
            }
        }
        if (ttchk() > 0)
	  return(ttxin(n,&buf[i])+i);
        else
	  return(i);
    }
#endif /* TTLEBUF */

#ifdef NETCMD
    if (ttpipe)
      fd = fdin;
    else
#endif /* NETCMD */
      fd = ttyfd;

#ifdef SUNX25
    if (netconn && (ttnet == NET_SX25))	/* X.25 connection */
      return(x25xin(n,buf));
#endif /* SUNX25 */

#ifdef IBMX25
    /* riehm: possibly not needed. Test worked with normal reads and writes */
    if (netconn && (ttnet == NET_IX25))	{ /* X.25 connection */
	x = x25xin(n,buf);
	if (x > 0) buf[x] = '\0';
	return(x);
    }
#endif /* IBMX25 */

#ifdef MYREAD
    debug(F101,"ttxin MYREAD","",n);
    while (x < n) {
	c = myread();
	if (c < 0) {
	    debug(F101,"ttxin myread returns","",c);
	    if (c == -3) x = -1;
	    break;
        }
	buf[x++] = c & ttpmsk;
#ifdef RLOGCODE
#ifdef CK_KERBEROS
        /* It is impossible to know how many characters are waiting */
        /* to be read when you are using Encrypted Rlogin or SSL    */
        /* as the transport since the number of real data bytes     */
        /* can be greater or less than the number of bytes on the   */
        /* wire which is what ttchk() returns.                      */
        if (netconn && (ttnproto == NP_EK4LOGIN || ttnproto == NP_EK5LOGIN))
	  if (ttchk() <= 0)
	    break;
#endif /* CK_KERBEROS */
#endif /* RLOGCODE */
#ifdef CK_SSL
        if (ssl_active_flag || tls_active_flag)
	  if (ttchk() <= 0)
	    break;
#endif /* CK_SSL */
    }
#else
    debug(F101,"ttxin READ","",n);
    x = read(fd,buf,n);
    for (c = 0; c < n; c++)		/* Strip any parity */
      buf[c] &= ttpmsk;
#endif /* MYREAD */

    debug(F101,"ttxin x","",x);		/* Done */
    if (x > 0) buf[x] = '\0';
    if (x < 0) x = -1;
    return(x);
}

/*  T T O L  --  Write string s, length n, to communication device.  */
/*
  Returns:
   >= 0 on success, number of characters actually written.
   -1 on failure.
*/
#ifdef CK_ENCRYPTION
CHAR * xpacket = NULL;
int nxpacket = 0;
#endif /* CK_ENCRYPTION */

#define TTOLMAXT 5
int
ttol(s,n) int n; CHAR *s; {
    int x, len, tries, fd;
#ifdef CKXXCHAR
    extern int dblflag;			/* For SET SEND DOUBLE-CHARACTER */
    extern short dblt[];
    CHAR *p = NULL, *p2, *s2, c;
    int n2 = 0;
#endif /* CKXXCHAR */

    if (ttyfd < 0)			/* Not open? */
      return(-3);
#ifdef DEBUG
    if (deblog) {
	/* debug(F101,"ttol ttyfd","",ttyfd); */
	ckhexdump("ttol s",s,n);
    }
#endif /* DEBUG */

#ifdef NETCMD
    if (ttpipe)
      fd = fdout;
    else
#endif /* NETCMD */
      fd = ttyfd;

#ifdef CKXXCHAR
/*  Double any characters that must be doubled.  */
    debug(F101,"ttol dblflag","",dblflag);
    if (dblflag) {
	p = (CHAR *) malloc(n + n + 1);
	if (p) {
	    s2 = s;
	    p2 = p;
	    n2 = 0;
	    while (*s2) {
		c = *s2++;
		*p2++ = c;
		n2++;
		if (dblt[(unsigned) c] & 2) {
		    *p2++ = c;
		    n2++;
		}
	    }
	    s = p;
	    n = n2;
	    s[n] = '\0';
	}
#ifdef DEBUG
        ckhexdump("ttol doubled s",s,n);
#endif /* DEBUG */
    }
#endif /* CKXXCHAR */

    tries = TTOLMAXT;			/* Allow up to this many tries */
    len = n;				/* Remember original length */

#ifdef CK_ENCRYPTION
/*
  This is to avoid encrypting a packet that is already encrypted, e.g.
  when we resend a packet directly out of the packet buffer, and also to
  avoid encrypting a constant (literal) string, which can cause a memory
  fault.
*/
    if (TELOPT_ME(TELOPT_ENCRYPTION)) {
	int x;
	if (nxpacket < n) {
	    if (xpacket) {
		free(xpacket);
		xpacket = NULL;
		nxpacket = 0;
	    }
	    x = n > 10240 ? n : 10240;
	    xpacket = (CHAR *)malloc(x);
	    if (!xpacket) {
		fprintf(stderr,"ttol malloc failure\n");
		return(-1);
	    } else
	      nxpacket = x;
	}
	memcpy((char *)xpacket,(char *)s,n);
	s = xpacket;
	ck_tn_encrypt((char *)s,n);
    }
#endif /* CK_ENCRYPTION */

    while (n > 0 &&
	   (tries-- > 0
#ifdef CK_ENCRYPTION
	    /* keep trying if we are encrypting */
	    || TELOPT_ME(TELOPT_ENCRYPTION)
#endif /* CK_ENCRYPTION */
            )) {			/* Be persistent */
	debug(F101,"ttol try","",TTOLMAXT - tries);
#ifdef BEOSORBEBOX
        if (netconn && !ttpipe && !ttpty)
	  x = nettol((char *)s,n);	/* Write string to device */
        else
#endif /* BEOSORBEBOX */
#ifdef IBMX25
	  if (ttnet == NET_IX25)
	    /*
	     * this is a more controlled way of writing to X25
	     * STREAMS, however write should also work!
	     */
	    x = x25write(ttyfd, s, n);
	  else
#endif /* IBMX25 */
#ifdef CK_SSL
	    if (ssl_active_flag || tls_active_flag) {
		int error;
		/* Write using SSL */
                ssl_retry:
		if (ssl_active_flag)
                  x = SSL_write(ssl_con, s, n);
		else
                  x = SSL_write(tls_con, s, n);
		switch (SSL_get_error(ssl_active_flag?ssl_con:tls_con,x)) {
                case SSL_ERROR_NONE:
                    if (x == n)
		      return(len);
                    s += x;
                    n -= x;
                    goto ssl_retry;
		  case SSL_ERROR_WANT_WRITE:
		  case SSL_ERROR_WANT_READ:
		    x = 0;
		    break;
		  case SSL_ERROR_SYSCALL:
                    if (x != 0)
		      return(-1);
		  case SSL_ERROR_WANT_X509_LOOKUP:
		  case SSL_ERROR_SSL:
		  case SSL_ERROR_ZERO_RETURN:
		  default:
		    ttclos(0);
		    return(-3);
		}
	    } else
#endif /* CK_SSL */
#ifdef CK_KERBEROS
#ifdef KRB4
#ifdef RLOGCODE
	    if (ttnproto == NP_EK4LOGIN) {
		return(krb4_des_write(ttyfd,s,n));
	    } else
#endif /* RLOGCODE */
#endif /* KRB4 */
#ifdef KRB5
#ifdef RLOGCODE
            if (ttnproto == NP_EK5LOGIN) {
                return(krb5_des_write(ttyfd,(char *)s,n,0));
            } else
#endif /* RLOGCODE */
#ifdef KRB5_U2U
            if (ttnproto == NP_K5U2U) {
                return(krb5_u2u_write(ttyfd,(char *)s,n));
            } else
#endif /* KRB5_U2U */
#endif /* KRB5 */
#endif /* CK_KERBEROS */
	      x = write(fd,s,n);	/* Write string to device */

	if (x == n) {			/* Worked? */
	    debug(F101,"ttol ok","",x);	/* OK */
#ifdef CKXXCHAR
	    if (p) free(p);
#endif /* CKXXCHAR */
	    return(len);		/* Done */
	} else if (x < 0) {		/* No, got error? */
	    debug(F101,"ttol write error","",errno);
#ifdef EWOULDBLOCK
	    if (errno == EWOULDBLOCK) {
		msleep(10);
		continue;
	    } else
#endif /* EWOULDBLOCK */
#ifdef TCPSOCKET
	    if (netconn && ttnet == NET_TCPB) {
		debug(F101,"ttol TCP error","",errno);
		ttclos(0);		/* Close the connection. */
		x = -3;
	    }
#endif /* TCPSOCKET */
#ifdef CKXXCHAR
	    if (p) free(p);
#endif /* CKXXCHAR */
	    return(x);
	} else {			/* No error, so partial success */
	    debug(F101,"ttol partial","",x); /* This never happens */
	    s += x;			/* Point to part not written yet */
	    n -= x;			/* Adjust length */
	    if (x > 0) msleep(10);	/* Wait 10 msec */
	}				/* Go back and try again */
    }
#ifdef CKXXCHAR
    if (p) free(p);
#endif /* CKXXCHAR */
    return(n < 1 ? len : -1);		/* Return the results */
}

/*  T T O C  --  Output a character to the communication line  */

/*
 This function should only be used for interactive, character-mode operations,
 like terminal connection, script execution, dialer i/o, where the overhead
 of the signals and alarms does not create a bottleneck.
*/
int
#ifdef CK_ANSIC
ttoc(char c)
#else
ttoc(c) char c;
#endif /* CK_ANSIC */
/* ttoc */ {
#define TTOC_TMO 15			/* Timeout in case we get stuck */
    int xx, fd;

    if (ttyfd < 0)			/* Check for not open. */
      return(-1);

#ifdef NETCMD
    if (ttpipe)
      fd = fdout;
    else
#endif /* NETCMD */
      fd = ttyfd;

    c &= 0xff;
    /* debug(F101,"ttoc","",(CHAR) c); */
    saval = signal(SIGALRM,timerh);	/* Enable timer interrupt */
    xx = alarm(TTOC_TMO);		/* for this many seconds. */
    if (xx < 0) xx = 0;			/* Save old alarm value. */
    /* debug(F101,"ttoc alarm","",xx); */
    if (
#ifdef CK_POSIX_SIG
	sigsetjmp(sjbuf,1)
#else
	setjmp(sjbuf)
#endif /* CK_POSIX_SIG */
	) {		/* Timer went off? */
	ttimoff();			/* Yes, cancel this alarm. */
	if (xx - TTOC_TMO > 0) alarm(xx - TTOC_TMO); /* Restore previous one */
        /* debug(F100,"ttoc timeout","",0); */
#ifdef NETCONN
	if (!netconn) {
#endif /* NETCONN */
	    debug(F101,"ttoc timeout","",c);
	    if (ttflow == FLO_XONX) {
		debug(F101,"ttoc flow","",ttflow); /* Maybe we're xoff'd */
#ifndef Plan9
#ifdef POSIX
		/* POSIX way to unstick. */
		debug(F100,"ttoc tcflow","",tcflow(ttyfd,TCOON));
#else
#ifdef BSD4				/* Berkeley way to do it. */
#ifdef TIOCSTART
/* .... Used to be "ioctl(ttyfd, TIOCSTART, 0);".  Who knows? */
		{
		  int x = 0;
		  debug(F101,"ttoc TIOCSTART","",ioctl(ttyfd, TIOCSTART, &x));
		}
#endif /* TIOCSTART */
#endif /* BSD4 */
					/* Is there a Sys V way to do this? */
#endif /* POSIX */
#endif /* Plan9 */
	    }
#ifdef NETCONN
        }
#endif /* NETCONN */
	return(-1);			/* Return failure code. */
    } else {
        int rc;
#ifdef BEOSORBEBOX
#ifdef NETCONN
        if (netconn && !ttpipe && !ttpty)
	  rc = nettoc(c);
        else
#endif /*  BEOSORBEBOX */
#endif /* NETCONN */
#ifdef CK_ENCRYPTION
	  if (TELOPT_ME(TELOPT_ENCRYPTION))
	    ck_tn_encrypt(&c,1);
#endif /* CK_ENCRYPTION */
#ifdef IBMX25
	/* riehm: maybe this isn't necessary after all. Test program
	 * worked fine with data being sent and retrieved with normal
	 * read's and writes!
	 */
	if (ttnet == NET_IX25)
	  rc = x25write(ttyfd,&c,1); /* as above for X25 streams */
	else
#endif /* IBMX25 */
#ifdef CK_SSL
	  if (ssl_active_flag || tls_active_flag) {
	      int error;
	      /* Write using SSL */
	      if (ssl_active_flag)
                rc = SSL_write(ssl_con, &c, 1);
	      else
                rc = SSL_write(tls_con, &c, 1);
	      switch (SSL_get_error(ssl_active_flag?ssl_con:tls_con,rc)){
		case SSL_ERROR_NONE:
		  break;
		case SSL_ERROR_WANT_WRITE:
		case SSL_ERROR_WANT_READ:
		  rc = 0;
		  break;
		case SSL_ERROR_SYSCALL:
		  if (rc != 0)
		    return(-1);
		case SSL_ERROR_WANT_X509_LOOKUP:
		case SSL_ERROR_SSL:
		case SSL_ERROR_ZERO_RETURN:
		default:
		  ttclos(0);
		  return(-1);
	      }
	  } else
#endif /* CK_SSL */
#ifdef CK_KERBEROS
#ifdef KRB4
#ifdef RLOGCODE
	  if (ttnproto == NP_EK4LOGIN) {
	      rc = (krb4_des_write(ttyfd,(char *)&c,1) == 1);
	  } else
#endif /* RLOGCODE */
#endif /* KRB4 */
#ifdef KRB5
#ifdef RLOGCODE
          if (ttnproto == NP_EK5LOGIN) {
              rc = (krb5_des_write(ttyfd,&c,1,0) == 1);
          } else
#endif /* RLOGCODE */
#ifdef KRB5_U2U
          if (ttnproto == NP_K5U2U) {
              rc = (krb5_u2u_write(ttyfd,&c,1) == 1);
          } else
#endif /* KRB5_U2U */
#endif /* KRB5 */
#endif /* CK_KERBEROS */
	    rc = write(fd,&c,1);	/* Try to write the character. */
	if (rc < 1) {			/* Failed */
	    ttimoff();			/* Turn off the alarm. */
	    alarm(xx);			/* Restore previous alarm. */
	    debug(F101,"ttoc errno","",errno); /* Log the error, */
	    return(-1);			/* and return the error code. */
	}
    }
    ttimoff();				/* Success, turn off the alarm. */
    alarm(xx);				/* Restore previous alarm. */
    return(0);				/* Return good code. */
}

/*  T T I N L  --  Read a record (up to break character) from comm line.  */
/*
  Reads up to "max" characters from the connection, terminating on:
    (a) the packet length field if the "turn" argument is zero, or
    (b) on the packet-end character (eol) if the "turn" argument is nonzero
    (c) a certain number of Ctrl-C's in a row

  Returns:
    >= 0, the number of characters read upon success;
    -1 if "max" exceeded, timeout, or other correctable error;
    -2 on user interruption (c);
    -3 on fatal error like connection lost.

  The name of this routine dates from the early days when Kermit packets
  were, indeed, always lines of text.  That was before control-character
  unprefixing and length-driven packet framing were introduced, which this
  version handle.  NB: this routine is ONLY for reading incoming Kermit
  packets, nothing else.  To read other kinds of incoming material, use
  ttinc() or ttxin().

  The bytes that were input are copied into "dest" with their parity bits
  stripped if parity was selected.  Returns the number of bytes read.
  Bytes after the eol are available upon the next call to this function.

  The idea is to minimize the number of system calls per packet, and also to
  minimize timeouts.  This function is the inner loop of the protocol and must
  be as efficient as possible.  The current strategy is to use myread(), a
  macro to manage buffered (and generally nonblocking) reads.

  WARNING: This function calls parchk(), which is defined in another module.
  Normally, ckutio.c does not depend on code from any other module, but there
  is an exception in this case because all the other ck?tio.c modules also
  need to call parchk(), so it's better to have it defined in a common place.
*/
#ifdef CTRLC
#undef CTRLC
#endif /* CTRLC */
#define CTRLC '\03'
/*
  We have four different declarations here because:
  (a) to allow Kermit to be built without the automatic parity sensing feature
  (b) one of each type for ANSI C, one for non-ANSI.
*/
#ifndef NOXFER

static int pushedback = 0;

int
#ifdef PARSENSE
#ifdef CK_ANSIC
ttinl(CHAR *dest, int max,int timo, CHAR eol, CHAR start, int turn)
#else
ttinl(dest,max,timo,eol,start,turn) int max,timo,turn; CHAR *dest, eol, start;
#endif /* CK_ANSIC */
#else /* not PARSENSE */
#ifdef CK_ANSIC
ttinl(CHAR *dest, int max,int timo, CHAR eol)
#else
ttinl(dest,max,timo,eol) int max,timo; CHAR *dest, eol;
#endif /* CK_ANSIC */
#endif /* PARSENSE */
/* ttinl */ {

#ifndef MYREAD
    CHAR ch, dum;
#endif /* MYREAD */
#ifdef PARSENSE
    int pktlen = -1;
    int lplen = 0;
    int havelen = 0;
#endif /* PARSENSE */
    int fd;
    int sopmask = 0xff;			/* Start-Of-Packet mask */
#ifdef CKXXCHAR
    extern short dblt[];		/* Ignore-character table */
    extern int ignflag;
#endif /* CKXXCHAR */
#ifdef TCPSOCKET
    extern CHAR stchr;
#endif /* TCPSOCKET */
    int x;
#ifdef STREAMING
    extern int streaming;
    extern int sndtyp;
#endif /* STREAMING */

    if (ttyfd < 0) return(-3);          /* Not open. */
/*
  In February 2007 I fixed ttinl() to work better under the truly awful
  conditions encountered by the AM-APEX oceanographic floats that gather
  hurricane data and phone home using Iridium satellite modems, which under
  certain conditions, can send two packets back to back after a long pause.
  In this case the second packet would be ignored because the SOH was skipped
  due to the ttflui() call.  But the reworked lookahead/pushback logic broke
  Kermit transfers on encrypted connections.  This was fixed 12-13 August
  2007.  All of this happened after 8.0.212 Dev.27 was released and before
  Dev.28, so no harm done other than the delay.
*/
    debug(F101,"ttinl max","",max);
    debug(F101,"ttinl timo","",timo);

#ifdef NETCMD
    if (ttpipe)
      fd = fdin;
    else
#endif /* NETCMD */
      fd = ttyfd;

#ifdef COMMENT
    if (xlocal && conchk() > 0)		/* Allow for console interruptions */
      return(-1);
#endif /* COMMENT */

    *dest = '\0';                       /* Clear destination buffer */
    if (timo < 0) timo = 0;		/* Safety */
    if (timo) {				/* Don't time out if timo == 0 */
	int xx;
	saval = signal(SIGALRM,timerh);	/* Enable timer interrupt */
	xx = alarm(timo);		/* Set it. */
	debug(F101,"ttinl alarm","",xx);
    }
    if (
#ifdef CK_POSIX_SIG
	sigsetjmp(sjbuf,1)
#else
	setjmp(sjbuf)
#endif /* CK_POSIX_SIG */
	) {				/* Timer went off? */
	debug(F100,"ttinl timout","",0); /* Get here on timeout. */
	/* debug(F110," with",(char *) dest,0); */
	ttimoff();			/* Turn off timer */
	return(-1);			/* and return error code. */
    } else {
	register int i, n = -1;		/* local variables */
	int ccn = 0;
#ifdef PARSENSE
	register int flag = 0;
	debug(F000,"ttinl start","",start);
#endif /* PARSENSE */

	ttpmsk = ttprty ? 0177 : 0377;	/* Set parity stripping mask. */
	sopmask = needpchk ? 0177 : ttpmsk; /* And SOP matching mask. */

/* Now read into destination, stripping parity and looking for the */
/* the packet terminator, and also for several Ctrl-C's typed in a row. */

	i = 0;				/* Destination index */
	debug(F101,"ttinl eol","",eol);

	while (i < max-1) {
#ifdef MYREAD
	    errno = 0;
	    /* On encrypted connections myread returns encrypted bytes */
	    n = myread();
	    debug(F000,"TTINL myread char","",n);
	    if (n < 0) {	/* Timeout or i/o error? */
#ifdef DEBUG
		if (deblog) {
		    debug(F101,"ttinl myread failure, n","",n);
		    debug(F101,"ttinl myread errno","",errno);
		}
#endif /* DEBUG */
		/* Don't let EINTR break packets. */
		if (n == -3) {
		    if (errno == EINTR && i > 0) {
			debug(F111,"ttinl EINTR myread i","continuing",i);
			continue;
		    } else {
			debug(F110,"ttinl non-EINTR -3","closing",0);
			wasclosed = 1;
			ttimoff();	/* Turn off timer */
			ttclos(0);
			return(n);
		    }
		} else if (n == -2 && netconn /* && timo == 0 */ ) {
		    /* Here we try to catch broken network connections */
		    /* even when ioctl() and read() do not catch them */
		    debug(F111,"ttinl network myread failure","closing",n);
		    wasclosed = 1;
		    ttimoff();
		    ttclos(0);
		    return(-3);
		}
#ifdef STREAMING
		/* Streaming and no data to read */
		else if (n == 0 && streaming && sndtyp == 'D')
		  return(0);
#endif /* STREAMING */
		break;			/* Break out of while loop */
	    }

#else /* not MYREAD (is this code used anywhere any more?) */
/*
  The non-MYREAD code dates from the 1980s and was needed on certain platforms
  where there were no nonblocking reads.  -fdc, 2007/02/22.
*/
	    if ((n = read(fd, &n, 1)) < 1)
	      break;			/* Error - break out of while loop */

#endif /* MYREAD */

	    /* Get here with char in n */

#ifdef CK_ENCRYPTION
	    if (TELOPT_U(TELOPT_ENCRYPTION) && !pushedback) {
		CHAR ch = n;
		ck_tn_decrypt((char *)&ch,1);
		n = ch;
		debug(F000,"TTINL decryp char","",n);
	    }
	    pushedback = 0;
#endif /* CK_ENCRYPTION */

#ifdef TCPSOCKET
	    if (n == IAC &&		/* Handle Telnet options */
		((xlocal && netconn && IS_TELNET()) ||
		(!xlocal && sstelnet))) {
		n = tt_tnopt(n);
		if (n < 0)
		  return(n);
#ifndef NOPARSEN
		else if (n == 1)
		  start = stchr;
#endif /* NOPARSEN */
		if (n != 255)		/* No data - go back for next char */
		  continue;
	    }				/* Quoted IAC - keep going */
#endif /* TCPSOCKET */

#ifdef CKXXCHAR
	    if (ignflag)
	      if (dblt[(unsigned) n] & 1) /* Character to ignore? */
		continue;
#endif /* CKXXCHAR */
/*
  Use parity mask, rather than always stripping parity, to check for
  cancellation.  Otherwise, runs like \x03\x83\x03 in a packet could cancel
  the transfer when parity is NONE.  (Note that \x03\x03\x03 is extremely
  unlikely due to run-length encoding.)
*/
	    /* Check cancellation */
	    if (!xlocal && xfrcan && ((n & ttpmsk) == xfrchr)) {
		if (++ccn >= xfrnum) {	/* If xfrnum in a row, bail out. */
		    if (timo) {		/* Clear timer. */
			ttimoff();
		    }
		    if (xfrchr < 32)
		      printf("^%c...\r\n",(char)(xfrchr+64));
		    else
		      printf("Canceled...\r\n");
		    return(-2);
		}
	    } else ccn = 0;		/* No cancellation, reset counter, */

#ifdef PARSENSE
/*
  Restructured code allows for a new packet to appear somewhere in the
  middle of a previous one.  -fdc, 24 Feb 2007.
*/
	    if ((n & sopmask) == start) { /* Start of Packet */
		debug(F101,"ttinl SOP i","",i);
		flag = 1;		/* Flag that we are in a packet */
		havelen = 0;		/* Invalidate previous length */
		pktlen = -1;		/* (if any) in case we were */
		lplen = 0;		/* alread processand a packet */
		i = 0;			/* and reset the dest buffer pointer */
	    }
	    if (flag == 0) {		/* No SOP yet... */
		debug(F000,"ttinl skipping","",n);
		continue;
	    }
	    dest[i++] = n & ttpmsk;
/*
  If we have not been instructed to wait for a turnaround character, we can go
  by the packet length field.  If turn != 0, we must wait for the end of line
  (eol) character before returning.  This is an egregious violation of all
  principles of layering...  (Less egregious in C-Kermit 9.0, in which we go
  by the length field but also look for the eol in case it arrives early,
  e.g. if the length field was corrupted upwards.)
*/
	    if (!havelen) {
		if (i == 2) {
		    if ((dest[1] & 0x7f) < 32) /* Garbage in length field */
		      return(-1);	/* fdc - 13 Apr 2010 */
		    pktlen = xunchar(dest[1] & 0x7f);
                    if (pktlen > 94)	/* Rubout in length field */
		      return(-1);	/* fdc - 13 Apr 2010 */
		    if (pktlen > 1) {
			havelen = 1;
			debug(F101,"ttinl pktlen value","",pktlen);
		    }
		} else if (i == 5 && pktlen == 0) {
		    lplen = xunchar(dest[4] & 0x7f);
		} else if (i == 6 && pktlen == 0) {
		    pktlen = lplen * 95 + xunchar(dest[5] & 0x7f) + 5;
		    havelen = 1;
		    debug(F101,"ttinl extended length","",pktlen);
		}
	    }

/*
  Suppose we looked at the sequence number here and found it was out of
  range?  This would mean either (a) incoming packets had SOP unprefixed
  and we are out of sync, or (b) the packet is damaged.  Since (a) is bad
  practice, let's ignore it.  So what should we do here if we know the
  packet is damaged?

   1. Nothing -- keep trying to read the packet till we find what we think
      is the end, or we time out, and let the upper layer decide what to
      do.  But since either the packet is corrupt or we are out of sync,
      our criterion for finding the end does not apply and we are likely
      to time out (or swallow a piece of the next packet) if our assumed
      length is too long.  (This was the behavior prior to version 7.0.)

   2. set flag = 0 and continue?  This would force us to wait for the
      next packet to come in, and therefore (in the nonwindowing case),
      would force a timeout in the other Kermit.

   3. set flag = 0 and continue, but only if the window size is > 1 and
      the window is not blocked?  Talk about cheating!

   4. Return a failure code and let the upper layer decide what to do.
      This should be equivalent to 3, but without the cheating.  So let's
      do it that way...  But note that we must ignore the parity bit
      in case this is the first packet and we have not yet run parchk().
*/
	    if (i == 3) {		/* Peek at sequence number */
		x = xunchar((dest[i-1] & 0x7f)); /* If it's not in range... */
		if (x < 0 || x > 63) {
		    debug(F111,"ttinl bad seq",dest,x);
		    if (timo) ttimoff();
		    return(-1);		/* return a nonfatal error */
		}
	    }

#else /* PARSENSE */
	    dest[i++] = n & ttpmsk;
#endif /* PARSENSE */

    /* Check for end of packet */

	    if (
		((n & ttpmsk) == eol)	/* Always break on the eol char */
#ifdef PARSENSE
		 ||			/* fdc - see notes of 13 Apr 2010 */
/*
  Purely length-driven if SET HANDSHAKE NONE (i.e. turn == 0).
  This allows packet terminators and handshake characters to appear
  literally inside a packet data field.
*/
		(havelen && (i > pktlen+1) &&
		 (!turn || (turn && (n & 0x7f) == turn))) /* (turn, not eol) */

#endif /* PARSENSE */
		) {
/*
  Here we have either read the last byte of the packet based on its length
  field, or else we have read the packet terminator (eol) or the half-duplex
  line-turnaround char (turn).
*/
#ifndef PARSENSE
		debug(F101,"ttinl got eol","",eol); /* (or turn) */
		dest[i] = '\0';		/* Yes, terminate the string, */
		/* debug(F101,"ttinl i","",i); */

#else  /* PARSENSE */

#ifdef DEBUG
		if (deblog) {
		    if ((n & ttpmsk) != eol) {
			debug(F101,"ttinl EOP length","",pktlen);
			debug(F000,"ttinl EOP current char","",n);
			debug(F101,"ttinl EOP packet buf index","",i);
		    } else debug(F101,"ttinl got eol","",eol);
		}
#endif /* DEBUG */

#ifdef MYREAD
/*
  The packet was read based on its length.  This leaves the packet terminator
  unread, and so ttchk() will always return at least 1 because of this,
  possibly giving a false positive to the "is there another packet waiting?"
  test.  But if we know the terminator (or any other interpacket junk) is
  there, we can safely get rid of it.

  NOTE: This code reworked to (a) execute even if the debug log isn't active;
  and (b) actually work.  -fdc, 2007/02/22.  And again 2007/08/12-13 to also
  work on encrypted connections.
*/	
		debug(F101,"TTINL my_count","",my_count);
		if ((n & ttpmsk) != eol) { /* Not the packet terminator */
		    int x;
		    while (my_count > 0) {
			x = myread();	   /* (was ttinc(0) */
			debug(F000,"TTINL lkread char","",x);
#ifdef CK_ENCRYPTION
			if (TELOPT_U(TELOPT_ENCRYPTION)) {
			    CHAR ch = x;
			    ck_tn_decrypt((char *)&ch,1);
			    x = ch;
			    debug(F000,"TTINL lkdecr char","",x); 
			}
#endif	/* CK_ENCRYPTION */
			/*
			  Note: while it might seem more elegant to simply
			  push back the encrypted byte, that desynchronizes
			  the decryption stream; the flag is necessary so we
			  don't try to decrypt the same byte twice.
			*/
			if ((x & ttpmsk) == start) { /* Start of next packet */
			    myunrd(x);	/* Push back the decrypted byte */
			    pushedback = 1; /* And set flag */
			    debug(F000,"TTINL lkpush char","",x);
			    break;
			}
		    }
		}
#endif /* MYREAD */

		dest[i] = '\0';		/* Terminate the string, */
	        if (needpchk) {		/* Parity checked yet? */
		    if (ttprty == 0) {	/* No, check. */
			if ((ttprty = parchk(dest,start,i)) > 0) {
			    int j;
			    debug(F101,"ttinl senses parity","",ttprty);
			    debug(F110,"ttinl packet before",dest,0);
			    ttpmsk = 0x7f;
			    for (j = 0; j < i; j++)
			      dest[j] &= 0x7f;	/* Strip parity from packet */
			    debug(F110,"ttinl packet after ",dest,0);
			} else ttprty = 0; /* Restore if parchk error */
		    }
		    sopmask = ttpmsk;
		    needpchk = 0;
		}
#endif /* PARSENSE */

		if (timo)		/* Turn off timer if it was on */
		  ttimoff();
                ckhexdump("ttinl got",dest,i);

#ifdef STREAMING
		/* ttinl() was called because there was non-packet */
		/* data sitting in the back channel.  Ignore it.   */
		if (streaming && sndtyp == 'D')
		  return(-1);
#endif /* STREAMING */
		return(i);
	    }
	} /* End of while() */
	ttimoff();
	return(n);
    }
}
#endif /* NOXFER */

/*  T T I N C --  Read a character from the communication line  */
/*
 On success, returns the character that was read, >= 0.
 On failure, returns -1 or other negative myread error code,
   or -2 if connection is broken or ttyfd < 0.
   or -3 if session limit has expired,
   or -4 if something or other...
 NOTE: The API does not provide for ttinc() returning a special code
 upon timeout, but we need it.  So for this we have a global variable,
 ttinctimo.
*/
static int ttinctimo = 0;		/* Yuk */

int
ttinc(timo) int timo; {

    int n = 0, fd;
    int is_tn = 0;
    CHAR ch = 0;

    ttinctimo = 0;

    if (ttyfd < 0) return(-2);          /* Not open. */

    is_tn = (xlocal && netconn && IS_TELNET()) ||
	    (!xlocal && sstelnet);

#ifdef TTLEBUF
    if (ttpush >= 0) {
        debug(F111,"ttinc","ttpush",ttpush);
        ch = ttpush;
        ttpush = -1;
        return(ch);
    }
    if (le_data) {
        if (le_getchar(&ch) > 0) {
            debug(F111,"ttinc le_getchar","ch",ch);
            return(ch);
        }
    }
#endif /* TTLEBUF */

#ifdef NETCMD
    if (ttpipe)
      fd = fdin;
    else
#endif /* NETCMD */
      fd = ttyfd;

    if ((timo <= 0)			/* Untimed. */
#ifdef MYREAD
	|| (my_count > 0)		/* Buffered char already waiting. */
#endif /* MYREAD */
	) {
#ifdef MYREAD
        /* Comm line failure returns -1 thru myread, so no &= 0377 */
	n = myread();			/* Wait for a character... */
	/* debug(F000,"ttinc MYREAD n","",n); */
#ifdef CK_ENCRYPTION
	/* debug(F101,"ttinc u_encrypt","",TELOPT_U(TELOPT_ENCRYPTION)); */
	if (TELOPT_U(TELOPT_ENCRYPTION) && n >= 0) {
	    ch = n;
	    ck_tn_decrypt((char *)&ch,1);
	    n = ch;
	}
#endif /* CK_ENCRYPTION */

#ifdef NETPTY
	if (ttpty && n < 0) {
	    debug(F101,"ttinc error on pty","",n);
	    ttclos(0);
	    return(n);
	}
#endif /* NETPTY */

#ifdef TNCODE
	if ((n > -1) && is_tn)
	  return((unsigned)(n & 0xff));
	else
#endif /* TNCODE */
	  return(n < 0 ? n : (unsigned)(n & ttpmsk));

#else  /* MYREAD */

        while ((n = read(fd,&ch,1)) == 0) /* Wait for a character. */
        /* Shouldn't have to loop in ver 5A. */
#ifdef NETCONN
	  if (netconn) {		/* Special handling for net */
	      netclos();		/* If read() returns 0 it means */
	      netconn = 0;		/* the connection has dropped. */
	      errno = ENOTCONN;
	      return(-2);
	  }
#endif /* NETCONN */
	  ;
	/* debug(F101,"ttinc","",ch); */
#ifdef TNCODE
	if ((n > 0) && is_tn) {
#ifdef CK_ENCRYPTION
	    if (TELOPT_U(TELOPT_ENCRYPTION)) {
		ck_tn_decrypt(&ch,1);
		n = ch;
	    }
#endif /* CK_ENCRYPTION */
	    return((unsigned)(ch & 0xff));
	} else
#endif /* TNCODE */
        return((n < 0) ? -4 : ((n == 0) ? -1 : (unsigned)(ch & ttpmsk)));
#endif /* MYREAD */

    } else {				/* Timed read */

	int oldalarm;
	saval = signal(SIGALRM,timerh);	/* Set up handler, save old one. */
	oldalarm = alarm(timo);		/* Set alarm, save old one. */
	if (
#ifdef CK_POSIX_SIG
	    sigsetjmp(sjbuf,1)
#else
	    setjmp(sjbuf)
#endif /* CK_POSIX_SIG */
	    ) {				/* Timer expired */
	    ttinctimo = 1;
	    n = -1;			/* set flag */
	} else {
#ifdef MYREAD
	    n = myread();		/* If managing own buffer... */
	    debug(F101,"ttinc myread","",n);
	    ch = n;
#else
	    n = read(fd,&ch,1);		/* Otherwise call the system. */
	    if (n == 0) n = -1;
	    debug(F101,"ttinc read","",n);
#endif /* MYREAD */

#ifdef CK_ENCRYPTION
	    if (TELOPT_U(TELOPT_ENCRYPTION) && n >= 0) {
		ck_tn_decrypt((char *)&ch,1);
	    }
#endif /* CK_ENCRYPTION */
	    if (n >= 0)
	      n = (unsigned) (ch & 0xff);
	    else
	      n = (n < 0) ? -4 : -2;	/* Special return codes. */
	}
	ttimoff();			/* Turn off the timer */
	if (oldalarm > 0) {
	    if (n == -1)		/* and restore any previous alarm */
	      oldalarm -= timo;
	    if (oldalarm < 0)		/* adjusted by our timeout interval */
	      oldalarm = 0;
	    if (oldalarm) {
	        debug(F101,"ttinc restoring oldalarm","",oldalarm);
		alarm(oldalarm);
	    }
	}
#ifdef NETCONN
	if (netconn) {
	    if (n == -2) {		/* read() returns 0 */
		netclos();		/* on network read failure */
		netconn = 0;
		errno = ENOTCONN;
	    }
	}
#endif	/* NETCONN */
#ifdef TNCODE
	if ((n > -1) && is_tn)
	  return((unsigned)(n & 0xff));
	else
#endif /* TNCODE */
	  /* Return masked char or neg. */
	  return( (n < 0) ? n : (unsigned)(n & ttpmsk) );
    }
}

/*  S N D B R K  --  Send a BREAK signal of the given duration  */

static int
#ifdef CK_ANSIC
sndbrk(int msec) {			/* Argument is milliseconds */
#else
sndbrk(msec) int msec; {
#endif /* CK_ANSIC */
#ifndef POSIX
    int x, n;
#endif /* POSIX */

#ifdef OXOS
#define BSDBREAK
#endif /* OXOS */

#ifdef ANYBSD
#define BSDBREAK
#endif /* ANYBSD */

#ifdef BSD44
#define BSDBREAK
#endif /* BSD44 */

#ifdef COHERENT
#ifdef BSDBREAK
#undef BSDBREAK
#endif /* BSDBREAK */
#endif /* COHERENT */

#ifdef BELLV10
#ifdef BSDBREAK
#undef BSDBREAK
#endif /* BSDBREAK */
#endif /* BELLV10 */

#ifdef PROVX1
    char spd;
#endif /* PROVX1 */

    debug(F101,"ttsndb ttyfd","",ttyfd);
    if (ttyfd < 0) return(-1);          /* Not open. */

#ifdef Plan9
    return p9sndbrk(msec);
#else
#ifdef NETCONN
#ifdef NETCMD
    if (ttpipe)				/* Pipe */
      return(ttoc('\0'));
#endif /* NETCMD */
#ifdef NETPTY
    if (ttpty)
      return(ttoc('\0'));
#endif /* NETPTY */
    if (netconn) 			/* Send network BREAK */
      return(netbreak());
#endif /* NETCONN */

    if (msec < 1 || msec > 5000) return(-1); /* Bad argument */

#ifdef POSIX				/* Easy in POSIX */
    {
	int x;
	debug(F111,"sndbrk POSIX",ckitoa(msec),(msec/375));
	errno = 0;
	x = tcsendbreak(ttyfd,msec / 375);
	debug(F111,"sndbrk tcsendbreak",ckitoa(errno),x);
	return(x);
    }
#else
#ifdef PROVX1
    gtty(ttyfd,&ttbuf);                 /* Get current tty flags */
    spd = ttbuf.sg_ospeed;              /* Save speed */
    ttbuf.sg_ospeed = B50;              /* Change to 50 baud */
    stty(ttyfd,&ttbuf);                 /*  ... */
    n = (int)strlen(brnuls);		/* Send the right number of nulls */
    x = msec / 91;
    if (x > n) x = n;
    write(ttyfd,brnuls,n);
    ttbuf.sg_ospeed = spd;              /* Restore speed */
    stty(ttyfd,&ttbuf);                 /*  ... */
    return(0);
#else
#ifdef aegis
    sio_$control((short)ttyfd, sio_$send_break, msec, st);
    return(0);
#else
#ifdef BSDBREAK
    n = FWRITE;                         /* Flush output queue. */
/* Watch out for int vs long problems in &n arg! */
    debug(F101,"sndbrk BSDBREAK","",msec);
    ioctl(ttyfd,TIOCFLUSH,&n);          /* Ignore any errors.. */
    if (ioctl(ttyfd,TIOCSBRK,(char *)0) < 0) {  /* Turn on BREAK */
        perror("Can't send BREAK");
        return(-1);
    }
    x = msleep(msec);                    /* Sleep for so many milliseconds */
    if (ioctl(ttyfd,TIOCCBRK,(char *)0) < 0) {  /* Turn off BREAK */
        perror("BREAK stuck!!!");
        doexit(BAD_EXIT,-1);		/* Get out, closing the line. */
                                        /*   with bad exit status */
    }
    return(x);
#else
#ifdef ATTSV
/*
  No way to send a long BREAK in Sys V, so send a bunch of regular ones.
  (Actually, Sys V R4 is *supposed* to have the POSIX tcsendbreak() function,
  but there's no way for this code to know for sure.)
*/
    debug(F101,"sndbrk ATTSV","",msec);
    x = msec / 275;
    for (n = 0; n < x; n++) {
	/* Reportedly the cast breaks this function on some systems */
	/* But then why was it here in the first place? */
	if (ioctl(ttyfd,TCSBRK, /* (char *) */ 0) < 0) {
	    perror("Can't send BREAK");
	    return(-1);
	}
    }
    return(0);
#else
#ifdef  V7
    debug(F101,"sndbrk V7","",msec);
    return(genbrk(ttyfd,250));		/* Simulate a BREAK */
#else
    debug(F101,"sndbrk catchall","",msec);
    ttoc(0);ttoc(0);ttoc(0);ttoc(0);
    return(0);
#endif /* V7 */
#endif /* BSDBREAK */
#endif /* ATTSV */
#endif /* aegis */
#endif /* PROVX1 */
#endif /* POSIX */
#endif /* Plan9 */
}

/*  T T S N D B  --  Send a BREAK signal  */

int
ttsndb() {
#ifdef TN_COMPORT
    if (netconn && istncomport())
      return((tnsndb(275L) >= 0) ? 0 : -1);
    else
#endif /* TN_COMPORT */
      return(sndbrk(275));
}

/*  T T S N D L B  --  Send a Long BREAK signal  */

int
ttsndlb() {
#ifdef TN_COMPORT
    if (netconn && istncomport())
      return((tnsndb(1800L) >= 0) ? 0 : -1);
    else
#endif /* TN_COMPORT */
    return(sndbrk(1500));
}

/*  M S L E E P  --  Millisecond version of sleep().  */

/*
  Call with number of milliseconds (thousandths of seconds) to sleep.
  Intended only for small intervals.  For big ones, just use sleep().
  Highly system-dependent.
  Returns 0 always, even if it didn't work.
*/

/* Define MSLFTIME for systems that must use an ftime() loop. */
#ifdef ANYBSD				/* For pre-4.2 BSD versions */
#ifndef BSD4
#define MSLFTIME
#endif /* BSD4 */
#endif /* ANYBSD */

#ifdef TOWER1				/* NCR Tower OS 1.0 */
#define MSLFTIME
#endif /* TOWER1 */

#ifdef COHERENT         /* Coherent... */
#ifndef _I386           /* Maybe Coherent/386 should get this, too */
#define MSLFTIME        /* Opinions are divided */
#endif /* _I386 */
#endif /* COHERENT */

#ifdef COMMENT
#ifdef GETMSEC

/* Millisecond timer */

static long msecbase = 0L;		/* Unsigned long not portable */

long
getmsec() {				/* Milliseconds since base time */
    struct timeval xv;
    struct timezone xz;
    long secs, msecs;
    if (
#ifdef GTODONEARG
	gettimeofday(&tv)
#else
#ifdef PTX
	gettimeofday(&tv, NULL)
#else
	gettimeofday(&tv, &tz)
#endif /* PTX */
#endif /* GTODONEARG */
	< 0)
      return(-1);
    if (msecbase == 0L) {		/* First call, set base time. */
	msecbase = tv.tv_sec;
	debug(F101,"getmsec base","",msecbase);
    }
    return(((tv.tv_sec - msecbase) * 1000L) + (tv.tv_usec / 1000L));
}
#endif /* GETMSEC */
#endif /* COMMENT */

#ifdef SELECT
int
ttwait(fd, secs) int fd, secs; {
    int x;
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(fd,&rfds);
    tv.tv_sec = secs;
    tv.tv_usec = 0L;
    errno = 0;
    if ((x = select(FD_SETSIZE,
#ifdef HPUX9
		    (int *)
#else
#ifdef HPUX1000
		    (int *)
#endif /* HPUX1000 */
#endif /* HPUX9 */
		    &rfds,
		    0, 0, &tv)) < 0) {
	debug(F101,"ttwait select errno","",errno);
	return(0);
    } else {
	debug(F101,"ttwait OK","",errno);
	x = FD_ISSET(fd, &rfds);
	debug(F101,"ttwait select x","",x);
	return(x ? 1 : 0);
    }
}
#endif /* SELECT */

int
msleep(m) int m; {
/*
  Other possibilities here are:
   nanosleep(), reportedly defined in POSIX.4.
   sginap(), IRIX only (back to what IRIX version I don't know).
*/
#ifdef Plan9
    return _SLEEP(m);
#else
#ifdef BEOSORBEBOX
    snooze(m*1000);
#else /* BEOSORBEBOX */
#ifdef SELECT
    int t1, x;
    debug(F101,"msleep SELECT 1","",m);
    if (m <= 0) return(0);
    if (m >= 1000) {			/* Catch big arguments. */
	sleep(m/1000);
	m = m % 1000;
	if (m < 10) return(0);
    }
    debug(F101,"msleep SELECT 2","",m);
#ifdef BELLV10
    x = select( 0, (fd_set *)0, (fd_set *)0, (fd_set *)0, m );
    debug(F101,"msleep BELLV10 select","",x);
#else /* BELLV10 */
#ifdef HPUX9
    gettimeofday(&tv, &tz);
#else

#ifndef COHERENT
#ifdef GTODONEARG
    if (gettimeofday(&tv) < 0)
#else
#ifdef PTX
    if (gettimeofday(&tv,NULL) < 0)
#else
#ifdef NOTIMEZONE
    if (gettimeofday(&tv, NULL) < 0)	/* wonder what this does... */
#else
    if (gettimeofday(&tv, &tz) < 0)
#endif /* NOTIMEZONE */
#endif /* PTX */
#endif /* GTODONEARG */
      return(-1);
    t1 = tv.tv_sec;                     /* Seconds */
#endif /* COHERENT */
#endif /* HPUX9 */
    tv.tv_sec = 0;                      /* Use select() */
    tv.tv_usec = m * 1000L;
#ifdef BSD44
    x = select( 0, (fd_set *)0, (fd_set *)0, (fd_set *)0, &tv );
    debug(F101,"msleep BSD44 select","",x);
#else /* BSD44 */
#ifdef __linux__
    x = select( 0, (fd_set *)0, (fd_set *)0, (fd_set *)0, &tv );
    debug(F101,"msleep __linux__ select","",x);
#else /* __linux__ */
#ifdef BSD43
    x = select( 0, (fd_set *)0, (fd_set *)0, (fd_set *)0, &tv );
    debug(F101,"msleep BSD43 select","",x);
#else /* BSD43 */
#ifdef QNX6
    x = select( 0, (fd_set *)0, (fd_set *)0, (fd_set *)0, &tv );
    debug(F101,"msleep QNX6 select","",x);
#else /* QNX6 */
#ifdef QNX
    x = select( 0, (fd_set *)0, (fd_set *)0, (fd_set *)0, &tv );
    debug(F101,"msleep QNX select","",x);
#else /* QNX */
#ifdef COHERENT
    x = select( 0, (fd_set *)0, (fd_set *)0, (fd_set *)0, &tv );
    debug(F101,"msleep COHERENT select","",x);
#else /* COHERENT */
#ifdef HPUX1000				/* 10.00 only, not 10.10 or later */
    x = select( 0, (int *)0, (int *)0, (int *)0, &tv );
    debug(F101,"msleep HP-UX 10.00 select","",x);
#else /* HPUX1000 */
#ifdef SVR4
    x = select( 0, (fd_set *)0, (fd_set *)0, (fd_set *)0, &tv );
    debug(F101,"msleep SVR4 select","",x);
#else /* SVR4 */
#ifdef OSF40
    x = select( 0, (fd_set *)0, (fd_set *)0, (fd_set *)0, &tv );
    debug(F101,"msleep OSF40 select","",x);
#else /* OSF40 */
#ifdef PTX
    x = select( 0, (fd_set *)0, (fd_set *)0, (fd_set *)0, &tv );
    debug(F101,"msleep OSF40 select","",x);
#else
    x = select( 0, (int *)0, (int *)0, (int *)0, &tv );
    debug(F101,"msleep catch-all select","",x);
#endif /* PTX */
#endif /* OSF40 */
#endif /* HP1000 */
#endif /* SVR4 */
#endif /* COHERENT */
#endif /* QNX */
#endif /* QNX6 */
#endif /* BSD43 */
#endif /* __linux__ */
#endif /* BSD44 */
#endif /* BELLV10 */
    return(0);

#else					/* Not SELECT */
#ifdef CK_POLL				/* We have poll() */
    struct pollfd pfd;			/* Supply a valid address for poll() */

#ifdef ODT30				/* But in SCO ODT 3.0 */
#ifdef NAP				/* we should use nap() instead */
    debug(F101,"msleep ODT 3.0 NAP","",m); /* because using poll() here */
    nap((long)m);			   /* seems to break dialing. */
    return(0);
#else
    debug(F101,"msleep ODT 3.0 POLL","",m);
    poll(&pfd, 0, m);
    return(0);
#endif /* NAP */
#else
    debug(F101,"msleep POLL","",m);
    poll(&pfd, 0, m);
    return(0);
#endif /* ODT30 */

/*
  We could handle the above more cleanly by just letting nap() always
  take precedence over poll() in this routine, but there is no way to know
  whether that would break something else.
*/

#else					/* Not POLL */
#ifdef USLEEP
/*
  "This routine is implemented using setitimer(2); it requires eight
  system calls...".  In other words, it might take 5 minutes to sleep
  10 milliseconds...
*/
    debug(F101,"msleep USLEEP","",m);
    if (m >= 1000) {			/* Catch big arguments. */
	sleep(m/1000);
	m = m % 1000;
	if (m < 10) return(0);
    }
    usleep((unsigned int)(m * 1000));
    return(0);
#else
#ifdef aegis
    time_$clock_t dur;
    debug(F101,"msleep aegis","",m);
    dur.c2.high16 = 0;
    dur.c2.low32  = 250 * m; /* one millisecond = 250 four microsecond ticks */
    time_$wait(time_$relative, dur, st);
    return(0);
#else
#ifdef PROVX1
    debug(F101,"msleep Venix","",m);
    if (m <= 0) return(0);
    sleep(-((m * 60 + 500) / 1000));
    return(0);
#else
#ifdef NAP
    debug(F101,"msleep NAP","",m);
    nap((long)m);
    return(0);
#else
#ifdef ATTSV
#ifndef BSD44
    extern long times();		/* Or #include <times.h> ? */
#endif /* BSD44 */
    long t1, t2, tarray[4];
    int t3;
    char *cp = getenv("HZ");
    int CLOCK_TICK;
    int hertz;

    if (cp && (hertz = atoi(cp))) {
        CLOCK_TICK  = 1000 / hertz;
    } else {				/* probably single user mode */
#ifdef HZ
        CLOCK_TICK  = 1000 / HZ;
#else
	static warned = 0;
	/* HZ always exists in, for instance, SCO Xenix, so you don't have to
	 * make special #ifdefs for XENIX here, like in ver 4F. Also, if you
	 * have Xenix, you have should have nap(), so the best is to use -DNAP
	 * in the makefile. Most systems have HZ.
	 */
	CLOCK_TICK = 17;		/* 1/60 sec */
	if (!warned) {
          printf("warning: environment variable HZ bad... using HZ=%d\r\n",
		 1000 / CLOCK_TICK);
          warned = 1;
	}
#endif /* !HZ */
    }
    debug(F101,"msleep ATTSV","",m);
    if (m <= 0) return(0);
    if (m >= 1000) {			/* Catch big arguments. */
	sleep(m/1000);
	m = m % 1000;
	if (m < 10) return(0);
    }
    if ((t1 = times(tarray)) < 0) return(-1);
    while (1) {
        if ((t2 = times(tarray)) < 0) return(-1);
        t3 = ((int)(t2 - t1)) * CLOCK_TICK;
        if (t3 > m) return(t3);
    }
#else /* Not ATTSV */
#ifdef MSLFTIME				/* Use ftime() loop... */
    int t1, t3 = 0;
    debug(F101,"msleep MSLFTIME","",m);
    if (m <= 0) return(0);
    if (m >= 1000) {			/* Catch big arguments. */
	sleep(m/1000);
	m = m % 1000;
	if (m < 10) return(0);
    }
#ifdef QNX
    ftime(&ftp);			/* void ftime() in QNX */
#else
    if (ftime(&ftp) < 0) return(-1);	/* Get base time. */
#endif /* QNX */
    t1 = ((ftp.time & 0xff) * 1000) + ftp.millitm;
    while (1) {
        ftime(&ftp);			/* Get current time and compare. */
        t3 = (((ftp.time & 0xff) * 1000) + ftp.millitm) - t1;
        if (t3 > m) return(0);
    }
#else
/* This includes true POSIX, which has no way to do this. */
    debug(F101,"msleep busy loop","",m);
    if (m >= 1000) {			/* Catch big arguments. */
	sleep(m/1000);
	m = m % 1000;
	if (m < 10) return(0);
    }
    if (m > 0) while (m > 0) m--;	/* Just a dumb busy loop */
    return(0);
#endif /* MSLFTIME */
#endif /* ATTSV */
#endif /* NAP */
#endif /* PROVX1 */
#endif /* aegis */
#endif /* CK_POLL */
#endif /* SELECT */
#endif /* BEOSORBEBOX */
#endif /* USLEEP */
#endif /* Plan9 */
}

/*  R T I M E R --  Reset elapsed time counter  */

VOID
rtimer() {
    tcount = time( (time_t *) 0 );
}


/*  G T I M E R --  Get current value of elapsed time counter in seconds  */

int
gtimer() {
    int x;
    x = (int) (time( (time_t *) 0 ) - tcount);
    debug(F101,"gtimer","",x);
    return( (x < 0) ? 0 : x );
}

#ifdef GFTIMER
/*
  Floating-point timers.  Require not only floating point support, but
  also gettimeofday().
*/
static struct timeval tzero;

VOID
rftimer() {
#ifdef GTODONEARG			/* Account for Mot's definition */
    (VOID) gettimeofday(&tzero);
#else
    (VOID) gettimeofday(&tzero, (struct timezone *)0);
#endif /* GTODONEARG */
}

CKFLOAT
gftimer() {
    struct timeval tnow, tdelta;
    CKFLOAT s;
#ifdef DEBUG
    char fpbuf[64];
#endif /* DEBUG */
#ifdef GTODONEARG			/* Account for Mot's definition */
    (VOID) gettimeofday(&tnow);
#else
    (VOID) gettimeofday(&tnow, (struct timezone *)0);
#endif /* GTODONEARG */

    tdelta.tv_sec = tnow.tv_sec - tzero.tv_sec;
    tdelta.tv_usec = tnow.tv_usec - tzero.tv_usec;

    if (tdelta.tv_usec < 0) {
	tdelta.tv_sec--;
	tdelta.tv_usec += 1000000;
    }
    s = (CKFLOAT) tdelta.tv_sec + ((CKFLOAT) tdelta.tv_usec / 1000000.0);
    if (s < GFMINTIME)
      s = GFMINTIME;
#ifdef DEBUG
    if (deblog) {
	sprintf(fpbuf,"%f",s);
	debug(F110,"gftimer",fpbuf,0);
    }
#endif /* DEBUG */
    return(s);
}
#endif /* GFTIMER */

/*  Z T I M E  --  Return asctime()-format date/time string  */
/*
  NOTE: as a side effect of calling this routine, we can also set the
  following two variables, giving the micro- and milliseconds (fractions of
  seconds) of the clock time.  Currently this is done only in BSD-based builds
  that use gettimeofday().  When these variables are not filled in, they are
  left with a value of -1L.
*/
static char asctmbuf[64];

VOID
ztime(s) char **s; {

#ifdef GFTIMER
/*
  The gettimeofday() method, which also sets ztmsec and ztusec, works for
  all GFTIMER builds.  NOTE: ztmsec and ztusec are defined in ckcmai.c,
  and extern declarations for them are in ckcdeb.h; thus they are
  declared in this file by inclusion of ckcdeb.h.
*/
    char *asctime();
    struct tm *localtime();
    struct tm *tp;
    ztmsec = -1L;
    ztusec = -1L;

    if (!s)
      debug(F100,"ztime s==NULL","",0);

#ifdef GTODONEARG
    /* No 2nd arg in Motorola SV88 and some others */
    if (gettimeofday(&tv) > -1)
#else
#ifndef COHERENT
#ifdef PTX
    if (gettimeofday(&tv,NULL) > -1)
#else
#ifdef NOTIMEZONE
    if (gettimeofday(&tv, NULL) > -1)	/* wonder what this does... */
#else
    if (gettimeofday(&tv, &tz) > -1)
#endif /* NOTIMEZONE */
#endif /* PTX */
#endif /* COHERENT */
#endif /* GTODONEARG */
      {					/* Fill in tm struct */
	ztusec = tv.tv_usec;		/* Microseconds */
	ztmsec = ztusec / 1000L;	/* Milliseconds */
#ifdef HPUX9
	{
	    time_t zz;
	    zz = tv.tv_sec;
	    tp = localtime(&zz);	/* Convert to local time */
	}
#else
#ifdef HPUX1000
	{
	    time_t zz;
	    zz = tv.tv_sec;
	    tp = localtime(&zz);
	}
#else
#ifdef LINUX
	{   /* avoid unaligned access trap on 64-bit platforms */
	    time_t zz;
	    zz = tv.tv_sec;
	    tp = localtime(&zz);
	}
#else
#ifdef MACOSX
	tp = localtime((time_t *)&tv.tv_sec); /* Convert to local time */
#else
	tp = localtime(&tv.tv_sec);
#endif /* MACOSX */
#endif /* LINUX */
#endif /* HPUX1000 */
#endif /* HPUX9 */
	if (s) {
	    char * s2;
	    s2 = asctime(tp);		/* Convert result to ASCII string */
	    asctmbuf[0] = '\0';
	    if (s2) ckstrncpy(asctmbuf,s2,64);
	    *s = asctmbuf;
	    debug(F111,"ztime GFTIMER gettimeofday",*s,ztusec);
	}
    }
#else  /* Not GFTIMER */

#undef ZTIMEV7				/* Which systems need to use */
#ifdef COHERENT				/* old UNIX Version 7 way... */
#define ZTIMEV7
#endif /* COHERENT */
#ifdef TOWER1
#define ZTIMEV7
#endif /* TOWER1 */
#ifdef ANYBSD
#ifndef BSD42
#define ZTIMEV7
#endif /* BSD42 */
#endif /* ANYBSD */
#ifdef V7
#ifndef MINIX
#define ZTIMEV7
#endif /* MINIX */
#endif /* V7 */
#ifdef POSIX
#define ZTIMEV7
#endif /* POSIX */

#ifdef HPUX1020
/*
  Prototypes are in <time.h>, included above.
*/
    time_t clock_storage;
    clock_storage = time((void *) 0);
    if (s) {
	*s = ctime(&clock_storage);
	debug(F110,"ztime: HPUX 10.20",*s,0);
    }
#else
#ifdef ATTSV				/* AT&T way */
/*  extern long time(); */		/* Theoretically these should */
    char *ctime();			/* already been dcl'd in <time.h> */
    time_t clock_storage;
    clock_storage = time(
#ifdef IRIX60
			 (time_t *)
#else
#ifdef BSD44
			 (time_t *)
#else
			 (long *)
#endif /* BSD44 */
#endif /* IRIX60 */
			 0 );
    if (s) {
	*s = ctime( &clock_storage );
	debug(F110,"ztime: ATTSV",*s,0);
    }
#else
#ifdef PROVX1				/* Venix 1.0 way */
    int utime[2];
    time(utime);
    if (s) {
	*s = ctime(utime);
	debug(F110,"ztime: PROVX1",*s,0);
    }
#else
#ifdef BSD42				/* 4.2BSD way */
    char *asctime();
    struct tm *localtime();
    struct tm *tp;
    gettimeofday(&tv, &tz);
    ztusec = tv.tv_usec;
    ztmsec = tv.tv_usec / 1000L;
    tp = localtime(&tv.tv_sec);
    if (s) {
	*s = asctime(tp);
	debug(F111,"ztime: BSD42",*s,ztusec);
    }
#else
#ifdef MINIX				/* MINIX way */
#ifdef COMMENT
    extern long time();			/* Already got these from <time.h> */
    extern char *ctime();
#endif /* COMMENT */
    time_t utime[2];
    time(utime);
    if (s) {
	*s = ctime(utime);
	debug(F110,"ztime: MINIX",*s,0);
    }
#else
#ifdef ZTIMEV7				/* The regular way */
    char *asctime();
    struct tm *localtime();
    struct tm *tp;
    long xclock;			/* or unsigned long for BeBox? */
    time(&xclock);
    tp = localtime(&xclock);
    if (s) {
	*s = asctime(tp);
	debug(F110,"ztime: ZTIMEV7",*s,0);
    }
#else					/* Catch-all for others... */
    if (s) {
	*s = "Day Mon 00 00:00:00 0000\n"; /* Dummy in asctime() format */
	debug(F110,"ztime: catch-all",*s,0);
    }
#endif /* ZTIMEV7 */
#endif /* MINIX */
#endif /* BSD42 */
#endif /* PROVX1 */
#endif /* ATTSV */
#endif /* HPUX1020 */
#endif /* GFTIMER */
}

/*  C O N G M  --  Get console terminal modes.  */

/*
  Saves initial console mode, and establishes variables for switching
  between current (presumably normal) mode and other modes.
  Should be called when program starts, but only after establishing
  whether program is in the foreground or background.
  Returns 1 if it got the modes OK, 0 if it did nothing, -1 on error.
*/
int
congm() {
    int fd;
    if (backgrd || !isatty(0)) {	/* If in background. */
	cgmf = -1;			/* Don't bother, modes are garbage. */
	return(-1);
    }
    if (cgmf > 0) return(0);		/* Already did this. */
    debug(F100,"congm getting modes","",0); /* Need to do it. */
#ifdef aegis
    ios_$inq_type_uid(ios_$stdin, conuid, st);
    if (st.all != status_$ok) {
	fprintf(stderr, "problem getting stdin objtype: ");
	error_$print(st);
    }
    concrp = (conuid == mbx_$uid);
    conbufn = 0;
#endif /* aegis */

#ifndef BEBOX
    if ((fd = open(CTTNAM,2)) < 0) {	/* Open controlling terminal */
#ifdef COMMENT
	fprintf(stderr,"Error opening %s\n", CTTNAM);
	perror("congm");
	return(-1);
#else
	fd = 0;
#endif /* COMMENT */
    }
#else
    fd = 0;
#endif /* !BEBOX */
#ifdef BSD44ORPOSIX
    if (tcgetattr(fd,&ccold) < 0) return(-1);
    if (tcgetattr(fd,&cccbrk) < 0) return(-1);
    if (tcgetattr(fd,&ccraw) < 0) return(-1);
#else
#ifdef ATTSV
    if (ioctl(fd,TCGETA,&ccold)  < 0) return(-1);
    if (ioctl(fd,TCGETA,&cccbrk) < 0) return(-1);
    if (ioctl(fd,TCGETA,&ccraw)  < 0) return(-1);
#ifdef VXVE
    cccbrk.c_line = 0;			/* STTY line 0 for CDC VX/VE */
    if (ioctl(fd,TCSETA,&cccbrk) < 0) return(-1);
    ccraw.c_line = 0;			/* STTY line 0 for CDC VX/VE */
    if (ioctl(fd,TCSETA,&ccraw) < 0) return(-1);
#endif /* VXVE */
#else
#ifdef BELLV10
    if (ioctl(fd,TIOCGETP,&ccold) < 0) return(-1);
    if (ioctl(fd,TIOCGETP,&cccbrk) < 0) return(-1);
    if (ioctl(fd,TIOCGETP,&ccraw) < 0) return(-1);
    debug(F101,"cccbrk.sg_flags orig","", cccbrk.sg_flags);
#else
    if (gtty(fd,&ccold) < 0) return(-1);
    if (gtty(fd,&cccbrk) < 0) return(-1);
    if (gtty(fd,&ccraw) < 0) return(-1);
#endif /* BELLV10 */
#endif /* ATTSV */
#endif /* BSD44ORPOSIX */
#ifdef sony_news			/* Sony NEWS */
    if (ioctl(fd,TIOCKGET,&km_con) < 0) { /* Get console Kanji mode */
	perror("congm error getting Kanji mode");
	debug(F101,"congm error getting Kanji mode","",0);
	km_con = -1;			/* Make sure this stays undefined. */
	return(-1);
    }
#endif /* sony_news */
    if (fd > 0)
      close(fd);
    cgmf = 1;				/* Flag that we got them. */
    return(1);
}


static VOID
congetbuf(x) int x; {
    int n;
    n = CONBUFSIZ - (conbufp - conbuf);	/* How much room left in buffer? */
    if (x > n) {
	debug(F101,"congetbuf char loss","",x-n);
	x = n;
    }
    x = read(0,conbufp,x);
    conbufn += x;
    debug(F111,"congetbuf readahead",conbuf,x);
}


/*  C O N C B --  Put console in cbreak mode.  */

/*  Returns 0 if ok, -1 if not  */

int
#ifdef CK_ANSIC
concb(char esc)
#else
concb(esc) char esc;
#endif /* CK_ANSIC */
/* concb */ {
    int x, y = 0;
    debug(F101,"concb constate","",constate);
    debug(F101,"concb cgmf","",cgmf);
    debug(F101,"concb backgrd","",backgrd);

    if (constate == CON_CB)
      return(0);

    if (cgmf < 1)			/* Did we get console modes yet? */
      if (!backgrd)			/* No, in background? */
	congm();			/* No, try to get them now. */
    if (cgmf < 1)			/* Still don't have them? */
      return(0);			/* Give up. */
    debug(F101,"concb ttyfd","",ttyfd);
    debug(F101,"concb ttfdflg","",ttfdflg);
#ifdef COMMENT
    /* This breaks returning to prompt after protocol with "-l 0" */
    /* Commented out July 1998 */
    if (ttfdflg && ttyfd >= 0 && ttyfd < 3)
      return(0);
#endif /* COMMENT */
    x = isatty(0);
    debug(F101,"concb isatty","",x);
    if (!x) return(0);			/* Only when running on real ttys */
    debug(F101,"concb xsuspend","",xsuspend);
    if (backgrd)			/* Do nothing if in background. */
      return(0);
    escchr = esc;                       /* Make this available to other fns */
    ckxech = 1;                         /* Program can echo characters */
#ifdef aegis
    conbufn = 0;
    if (concrp) return(write(1, "\035\002", 2));
    if (conuid == input_pad_$uid) {pad_$raw(ios_$stdin, st); return(0);}
#endif /* aegis */

#ifdef COHERENT
#define SVORPOSIX
#endif /* COHERENT */

#ifdef Plan9
    x = p9concb();
#else
#ifndef SVORPOSIX			/* BSD, V7, etc */
    debug(F101,"cccbrk.sg_flags concb 1","", cccbrk.sg_flags);
    debug(F101,"concb stty CBREAK","",0);
    cccbrk.sg_flags |= (CBREAK|CRMOD);	/* Set to character wakeup, */
    cccbrk.sg_flags &= ~ECHO;           /* no echo. */
    debug(F101,"cccbrk.sg_flags concb 2","", cccbrk.sg_flags);
    errno = 0;
/*
  BSD stty() clears the console buffer.  So if anything is waiting in it,
  we have to read it now to avoid losing it.
*/
    x = conchk();
    if (x > 0)
      congetbuf(x);

#ifdef BELLV10
    x = ioctl(0,TIOCSETP,&cccbrk);
#else
    x = stty(0,&cccbrk);
    debug(F101,"cccbrk.sg_flags concb x","", x);
#endif /* BELLV10 */
#else					/* Sys V and POSIX */
#ifndef OXOS
    debug(F101,"concb cccbrk.c_flag","",cccbrk.c_lflag);
#ifdef QNX
    /* Don't mess with IEXTEN */
    cccbrk.c_lflag &= ~(ICANON|ECHO);
#else
#ifdef COHERENT
    cccbrk.c_lflag &= ~(ICANON|ECHO);
#else
    cccbrk.c_lflag &= ~(ICANON|ECHO|IEXTEN);
#endif /* COHERENT */
#endif /* QNX */
    cccbrk.c_lflag |= ISIG;		/* Allow signals in command mode. */
    cccbrk.c_iflag |= IGNBRK;		/* But ignore BREAK signal */
    cccbrk.c_iflag &= ~BRKINT;

#else /* OXOS */
    debug(F100,"concb OXOS is defined","",0);
    cccbrk.c_lflag &= ~(ICANON|ECHO);
    cccbrk.c_cc[VDISCARD] = cccbrk.c_cc[VLNEXT] = CDISABLE;
#endif /* OXOS */
#ifdef COMMENT
/*
  Believe it or not, in SCO UNIX, VSUSP is greater than NCC, and so this
  array reference is out of bounds.  It's only a debug() call so who needs it.
*/
#ifdef VSUSP
    debug(F101,"concb c_cc[VSUSP]","",cccbrk.c_cc[VSUSP]);
#endif /* VSUSP */
#endif /* COMMENT */
#ifndef VINTR
    debug(F101,"concb c_cc[0]","",cccbrk.c_cc[0]);
    cccbrk.c_cc[0] = 003;               /* Interrupt char is Control-C */
#else
    debug(F101,"concb c_cc[VINTR]","",cccbrk.c_cc[0]);
    cccbrk.c_cc[VINTR] = 003;
#endif /* VINTR */
#ifndef VQUIT
    cccbrk.c_cc[1] = escchr;            /* escape during packet modes */
#else
    cccbrk.c_cc[VQUIT] = escchr;
#endif /* VQUIT */
#ifndef VEOF
    cccbrk.c_cc[4] = 1;
#else
#ifndef OXOS
#ifdef VMIN
    cccbrk.c_cc[VMIN] = 1;
#endif /* VMIN */
#else /* OXOS */
    cccbrk.c_min = 1;
#endif /* OXOS */
#endif /* VEOF */
#ifdef ZILOG
    cccbrk.c_cc[5] = 0;
#else
#ifndef VEOL
    cccbrk.c_cc[5] = 1;
#else
#ifndef OXOS
#ifdef VTIME
    cccbrk.c_cc[VTIME] = 1;
#endif /* VTIME */
#else /* OXOS */
    cccbrk.c_time = 1;
#endif /* OXOS */
#endif /* VEOL */
#endif /* ZILOG */
    errno = 0;
#ifdef BSD44ORPOSIX			/* Set new modes */
    x = tcsetattr(0,TCSADRAIN,&cccbrk);
#else /* ATTSV */      			/* or the POSIX way */
    x = ioctl(0,TCSETAW,&cccbrk);	/* the Sys V way */
#endif /* BSD44ORPOSIX */
#endif /* SVORPOSIX */

#ifdef COHERENT
#undef SVORPOSIX
#endif /* COHERENT */

    debug(F101,"concb x","",x);
    debug(F101,"concb errno","",errno);

#ifdef  V7
#ifndef MINIX
    if (kmem[CON] < 0) {
        qaddr[CON] = initrawq(0);
        if((kmem[CON] = open("/dev/kmem", 0)) < 0) {
            fprintf(stderr, "Can't read /dev/kmem in concb.\n");
            perror("/dev/kmem");
            exit(1);
        }
    }
#endif /* MINIX */
#endif /* V7 */
#endif /* Plan9 */

    if (x > -1)
      constate = CON_CB;

    debug(F101,"concb returns","",x);
    return(x);
}

/*  C O N B I N  --  Put console in binary mode  */

/*  Returns 0 if ok, -1 if not  */

int
#ifdef CK_ANSIC
conbin(char esc)
#else
conbin(esc) char esc;
#endif /* CK_ANSIC */
/* conbin */  {

    int x;

    debug(F101,"conbin constate","",constate);

    if (constate == CON_BIN)
      return(0);

    if (!isatty(0)) return(0);          /* only for real ttys */
    congm();				/* Get modes if necessary. */
    debug(F100,"conbin","",0);
    escchr = esc;                       /* Make this available to other fns */
    ckxech = 1;                         /* Program can echo characters */
#ifdef aegis
    conbufn = 0;
    if (concrp) return(write(1, "\035\002", 2));
    if (conuid == input_pad_$uid) {
	pad_$raw(ios_$stdin, st);
	return(0);
      }
#endif /* aegis */

#ifdef COHERENT
#define SVORPOSIX
#endif /* COHERENT */

#ifdef Plan9
    return p9conbin();
#else
#ifdef SVORPOSIX
#ifndef OXOS
#ifdef QNX
    ccraw.c_lflag &= ~(ISIG|ICANON|ECHO);
#else
#ifdef COHERENT
    ccraw.c_lflag &= ~(ISIG|ICANON|ECHO);
#else
    ccraw.c_lflag &= ~(ISIG|ICANON|ECHO|IEXTEN);
#endif /* COHERENT */
#endif /* QNX */
#else /* OXOS */
    ccraw.c_lflag &= ~(ISIG|ICANON|ECHO);
    ccraw.c_cc[VDISCARD] = ccraw.c_cc[VLNEXT] = CDISABLE;
#endif /* OXOS */
    ccraw.c_iflag |= IGNPAR;
/*
  Note that for terminal sessions we disable Xon/Xoff flow control to allow
  the passage ^Q and ^S as data characters for EMACS, and to allow XMODEM
  transfers to work when C-Kermit is in the middle, etc.  Hardware flow
  control, if in use, is not affected.
*/
#ifdef ATTSV
#ifdef BSD44
    ccraw.c_iflag &= ~(IGNBRK|INLCR|IGNCR|ICRNL|IXON|IXANY|IXOFF
                        |INPCK|ISTRIP);
#else
    ccraw.c_iflag &= ~(IGNBRK|INLCR|IGNCR|ICRNL|IUCLC|IXON|IXANY|IXOFF
                        |INPCK|ISTRIP);
#endif /* BSD44 */
#else /* POSIX */
    ccraw.c_iflag &= ~(IGNBRK|INLCR|IGNCR|ICRNL|IXON|IXOFF|INPCK|ISTRIP);
#endif /* ATTSV */
    ccraw.c_oflag &= ~OPOST;
#ifdef COMMENT
/*
  WHAT THE HECK WAS THIS FOR?
  The B9600 setting (obviously) prevents CONNECT from working at any
  speed other than 9600 when you are logged in to the 7300 on a serial
  line.  Maybe some of the other flags are necessary -- if so, put back
  the ones that are needed.  This code is supposed to work the same, no
  matter whether you are logged in to the 7300 on the real console device,
  or through a serial port.
*/
#ifdef ATT7300
    ccraw.c_cflag = CLOCAL | B9600 | CS8 | CREAD | HUPCL;
#endif /* ATT7300 */
#endif /* COMMENT */

/*** Kermit used to put the console in 8-bit raw mode, but some users have
 *** pointed out that this should not be done, since some sites actually
 *** use terminals with parity settings on their Unix systems, and if we
 *** override the current settings and stop doing parity, then their terminals
 *** will display blotches for characters whose parity is wrong.  Therefore,
 *** the following two lines are commented out (Larry Afrin, Clemson U):
 ***
 ***   ccraw.c_cflag &= ~(PARENB|CSIZE);
 ***   ccraw.c_cflag |= (CS8|CREAD);
 ***
 *** Sys III/V sites that have trouble with this can restore these lines.
 ***/
#ifndef VINTR
    ccraw.c_cc[0] = 003;		/* Interrupt char is Ctrl-C */
#else
    ccraw.c_cc[VINTR] = 003;
#endif /* VINTR */
#ifndef VQUIT
    ccraw.c_cc[1] = escchr;		/* Escape during packet mode */
#else
    ccraw.c_cc[VQUIT] = escchr;
#endif /* VQUIT */
#ifndef VEOF
    ccraw.c_cc[4] = 1;
#else
#ifndef OXOS
#ifdef VMIN
    ccraw.c_cc[VMIN] = 1;
#endif /* VMIN */
#else /* OXOS */
    ccraw.c_min = 1;
#endif /* OXOS */
#endif /* VEOF */

#ifdef ZILOG
    ccraw.c_cc[5] = 0;
#else
#ifndef VEOL
    ccraw.c_cc[5] = 1;
#else
#ifndef OXOS
#ifdef VTIME
    ccraw.c_cc[VTIME] = 1;
#endif /* VTIME */
#else /* OXOS */
    ccraw.c_time = 1;
#endif /* OXOS */
#endif /* VEOL */
#endif /* ZILOG */

#ifdef BSD44ORPOSIX
    x = tcsetattr(0,TCSADRAIN,&ccraw);	/* Set new modes. */
#else
    x = ioctl(0,TCSETAW,&ccraw);
#endif /* BSD44ORPOSIX */
#else /* Berkeley, etc. */
    x = conchk();			/* Because stty() is destructive */
    if (x > 0)
      congetbuf(x);
    ccraw.sg_flags |= (RAW|TANDEM);     /* Set rawmode, XON/XOFF (ha) */
    ccraw.sg_flags &= ~(ECHO|CRMOD);    /* Set char wakeup, no echo */
#ifdef BELLV10
    x = ioctl(0,TIOCSETP,&ccraw);
#else
    x = stty(0,&ccraw);
#endif /* BELLV10 */
#endif /* SVORPOSIX */
#endif /* Plan9 */

    if (x > -1)
      constate = CON_BIN;

    debug(F101,"conbin returns","",x);
    return(x);

#ifdef COHERENT
#undef SVORPOSIX
#endif /* COHERENT */

}


/*  C O N R E S  --  Restore the console terminal  */

int
conres() {
    int x;
    debug(F101,"conres cgmf","",cgmf);
    debug(F101,"conres constate","",constate);

    if (cgmf < 1)			/* Do nothing if modes unchanged */
      return(0);
    if (constate == CON_RES)
      return(0);

    if (!isatty(0)) return(0);          /* only for real ttys */
    debug(F100,"conres isatty ok","",0);
    ckxech = 0;                         /* System should echo chars */

#ifdef aegis
    conbufn = 0;
    if (concrp) return(write(1, "\035\001", 2));
    if (conuid == input_pad_$uid) {
	pad_$cooked(ios_$stdin, st);
	constate = CON_RES;
	return(0);
    }
#endif /* aegis */

#ifdef Plan9
    p9conres();
#else
#ifdef BSD44ORPOSIX
    debug(F100,"conres restoring tcsetattr","",0);
    x = tcsetattr(0,TCSADRAIN,&ccold);
#else
#ifdef ATTSV
    debug(F100,"conres restoring ioctl","",0);
    x = ioctl(0,TCSETAW,&ccold);
#else /* BSD, V7, and friends */
#ifdef sony_news			/* Sony NEWS */
    if (km_con != -1)
      ioctl(0,TIOCKSET,&km_con);	/* Restore console Kanji mode */
#endif /* sony_news */
    msleep(100);
    debug(F100,"conres restoring stty","",0);
    x = conchk();			/* Because stty() is destructive */
    if (x > 0)
      congetbuf(x);
#ifdef BELLV10
    x = ioctl(0,TIOCSETP,&ccold);
#else
    x = stty(0,&ccold);
#endif /* BELLV10 */
#endif /* ATTSV */
#endif /* BSD44ORPOSIX */
#endif /* Plan9 */
    if (x > -1)
      constate = CON_RES;

    debug(F101,"conres returns","",x);
    return(x);
}

/*  C O N O C  --  Output a character to the console terminal  */

int
#ifdef CK_ANSIC
conoc(char c)
#else
conoc(c) char c;
#endif /* CK_ANSIC */
/* conoc */ {

#ifdef IKSD
    if (inserver && !local)
      return(ttoc(c));

#ifdef CK_ENCRYPTION
    if (inserver && TELOPT_ME(TELOPT_ENCRYPTION))
        ck_tn_encrypt(&c,1);
#endif /* CK_ENCRYPTION */
#endif /* IKSD */

#ifdef Plan9
    return conwrite(&c,1);
#else
    return(write(1,&c,1));
#endif /* Plan9 */
}

/*  C O N X O  --  Write x characters to the console terminal  */

int
conxo(x,s) int x; char *s; {

#ifdef IKSD
    if (inserver && !local)
      return(ttol((CHAR *)s,x));

#ifdef CK_ENCRYPTION
    if (inserver && TELOPT_ME(TELOPT_ENCRYPTION))
        ck_tn_encrypt(s,x);
#endif /* CK_ENCRYPTION */
#endif /* IKSD */

#ifdef Plan9
    return(conwrite(s,x));
#else
    return(write(1,s,x));
#endif /* Plan9 */
}

/*  C O N O L  --  Write a line to the console terminal  */

int
conol(s) char *s; {
    int len;
    if (!s) s = "";			/* Always do this! */
    len = strlen(s);
    if (len == 0)
      return(0);

#ifdef IKSD
    if (inserver && !local)
      return(ttol((CHAR *)s,len));

#ifdef CK_ENCRYPTION
    if (inserver && TELOPT_ME(TELOPT_ENCRYPTION)) {
	if (nxpacket < len) {
	    if (xpacket) {
		free(xpacket);
		xpacket = NULL;
		nxpacket = 0;
	    }
	    len = len > 10240 ? len : 10240;
	    xpacket = (CHAR *)malloc(len);
	    if (!xpacket) {
		fprintf(stderr,"ttol malloc failure\n");
		return(-1);
	    } else
	      nxpacket = len;
	}
	memcpy(xpacket,s,len);
	s = (char *)xpacket;
	ck_tn_encrypt(s,len);
    }
#endif /* CK_ENCRYPTION */
#endif /* IKSD */

#ifdef Plan9
    return(conwrite(s,len));
#else
    return(write(1,s,len));
#endif /* Plan9 */
}

/*  C O N O L A  --  Write an array of lines to the console terminal */

int
conola(s) char *s[]; {
    char * p;
    int i, x;


    if (!s) return(0);
    for (i = 0; ; i++) {
	p = s[i];
	if (!p) p = "";			/* Let's not dump core shall we? */
	if (!*p)
	  break;
#ifdef IKSD
	if (inserver && !local)
	  x = ttol((CHAR *)p,(int)strlen(p));
	else
#endif /* IKSD */
	  x = conol(p);
	if (x < 0)
	  return(-1);
    }
    return(0);
}

/*  C O N O L L  --  Output a string followed by CRLF  */

int
conoll(s) char *s; {
    CHAR buf[3];
    buf[0] = '\r';
    buf[1] = '\n';
    buf[2] = '\0';
    if (!s) s = "";

#ifdef IKSD
    if (inserver && !local) {
	if (*s) ttol((CHAR *)s,(int)strlen(s));
	return(ttol(buf,2));
    }
#endif /* IKSD */

    if (*s) conol(s);
#ifdef IKSD
#ifdef CK_ENCRYPTION
    if (inserver && TELOPT_ME(TELOPT_ENCRYPTION))
      ck_tn_encrypt((char *)buf,2);
#endif /* CK_ENCRYPTION */
#endif /* IKSD */

#ifdef Plan9
    return(conwrite(buf, 2));
#else
    return(write(1,buf,2));
#endif /* Plan9 */
}

/*  C O N C H K  --  Return how many characters available at console  */
/*
  We could also use select() here to cover a few more systems that are not
  covered by any of the following, e.g. HP-UX 9.0x on the model 800.
*/
int
conchk() {
    static int contyp = 0;		/* +1 for isatty, -1 otherwise */

    if (contyp == 0)			/* This prevents unnecessary */
      contyp = (isatty(0) ? 1 : -1);	/* duplicated calls to isatty() */
    debug(F101,"conchk contyp","",contyp);
    if (backgrd || (contyp < 0))
      return(0);

#ifdef aegis
    if (conbufn > 0) return(conbufn);   /* use old count if nonzero */

    /* read in more characters */
    conbufn = ios_$get(ios_$stdin,
              ios_$cond_opt, conbuf, (long)sizeof(conbuf), st);
    if (st.all != status_$ok) conbufn = 0;
    conbufp = conbuf;
    return(conbufn);
#else
#ifdef IKSD
    if (inserver && !local)
      return(in_chk(1,ttyfd));
    else
#endif /* IKSD */
      return(in_chk(0,0));
#endif /* aegis */
}

/*  C O N I N C  --  Get a character from the console  */
/*
  Call with timo > 0 to do a timed read, timo == 0 to do an untimed blocking
  read.  Upon success, returns the character.  Upon failure, returns -1.
  A timed read that does not complete within the timeout period returns -2.
*/
int
coninc(timo) int timo; {
    int n = 0; CHAR ch;
    int xx;

    if (conbufn > 0) {			/* If something already buffered */
	--conbufn;
	return((unsigned)(*conbufp++ & 0xff));
    }

    errno = 0;				/* Clear this */
#ifdef IKSD
    if (inserver && !local) {
	xx = ttinc(timo);
	if (xx < 0)
	  return(ttinctimo ? -2 : -1);
	else
	  return(xx);
    }
#endif /* IKSD */

#ifdef aegis				/* Apollo Aegis only... */
    debug(F101,"coninc timo","",timo);
    fflush(stdout);
    if (conchk() > 0) {
	--conbufn;
	return((unsigned)(*conbufp++ & 0xff));
    }
#endif /* aegis */

#ifdef TTLEBUF
    if (
#ifdef IKSD
	inserver &&
#endif /* IKSD */
	!xlocal
	) {
	if (ttpush >= 0) {
	    debug(F111,"ttinc","ttpush",ttpush);
	    ch = ttpush;
	    ttpush = -1;
	    return(ch);
	}
	if (le_data) {
	    if (le_getchar(&ch) > 0) {
		debug(F111,"ttinc LocalEchoInBuf","ch",ch);
		return(ch);
	    }
	}
    }
#endif /* TTLEBUF */

    if (timo <= 0) {			/* Untimed, blocking read. */
	while (1) {			/* Keep trying till we get one. */
	    n = read(0, &ch, 1);	/* Read a character. */
	    if (n == 0) continue;	/* Shouldn't happen. */
	    if (n > 0) {		/* If read was successful, */
#ifdef IKSD
#ifdef CK_ENCRYPTION
                debug(F100,"coninc decrypt 1","",0);
                if (inserver && !local && TELOPT_U(TELOPT_ENCRYPTION))
		  ck_tn_decrypt((char *)&ch,1);
#endif /* CK_ENCRYPTION */
#endif /* IKSD */
		return((unsigned)(ch & 0xff)); /* return the character. */
            }

/* Come here if read() returned an error. */

	    debug(F101, "coninc(0) errno","",errno); /* Log the error. */
#ifndef OXOS
#ifdef SVORPOSIX
#ifdef CIE                             /* CIE Regulus has no EINTR symbol? */
#ifndef EINTR
#define EINTR 4
#endif /* EINTR */
#endif /* CIE */
/*
  This routine is used for several different purposes.  In CONNECT mode, it is
  used to do an untimed, blocking read from the keyboard in the lower CONNECT
  fork.  During local-mode file transfer, it reads a character from the
  console to interrupt the file transfer (like A for a status report, X to
  cancel a file, etc).  Obviously, we don't want the reads in the latter case
  to be blocking, or the file transfer would stop until the user typed
  something.  Unfortunately, System V does not allow the console device input
  buffer to be sampled nondestructively (e.g. by conchk()), so a kludge is
  used instead.  During local-mode file transfer, the SIGQUIT signal is armed
  and trapped by esctrp(), and this routine pretends to have read the quit
  character from the keyboard normally.  But, kludge or no kludge, the read()
  issued by this command, under System V only, can fail if a signal -- ANY
  signal -- is caught while the read is pending.  This can occur not only when
  the user types the quit character, but also during telnet negotiations, when
  the lower CONNECT fork signals the upper one about an echoing mode change.
  When this happens, we have to post the read() again.  This is apparently not
  a problem in BSD-based UNIX versions.
*/
	    if (errno == EINTR)		/* Read interrupted. */
	      if (conesc)  {		/* If by SIGQUIT, */
 		 conesc = 0;		/* the conesc variable is set, */
 		 return(escchr);	/* so return the escape character. */
	     } else continue;		/* By other signal, try again. */
#else
/*
  This might be dangerous, but let's do this on non-System V versions too,
  since at least one SunOS 4.1.2 user complains of immediate disconnections
  upon first making a TELNET connection.
*/
	    if (errno == EINTR)		/* Read interrupted. */
	      continue;
#endif /* SVORPOSIX */
#else /* OXOS */
	    if (errno == EINTR)		/* Read interrupted. */
	      continue;
#endif /* OXOS */
	    return(-1);			/* Error */
	}
    }
#ifdef DEBUG
    if (deblog && timo <= 0) {
	debug(F100,"coninc timeout logic error","",0);
	timo = 1;
    }
#endif /* DEBUG */

/* Timed read... */

    saval = signal(SIGALRM,timerh);	/* Set up timeout handler. */
    xx = alarm(timo);			/* Set the alarm. */
    debug(F101,"coninc alarm set","",timo);
    if (
#ifdef CK_POSIX_SIG
	sigsetjmp(sjbuf,1)
#else
	setjmp(sjbuf)
#endif /* CK_POSIX_SIG */
	)				/* The read() timed out. */
      n = -2;				/* Code for timeout. */
    else
      n = read(0, &ch, 1);
    ttimoff();				/* Turn off timer */
    if (n > 0) {			/* Got character OK. */
#ifdef IKSD
#ifdef CK_ENCRYPTION
        debug(F100,"coninc decrypt 2","",0);
        if (inserver && !local && TELOPT_U(TELOPT_ENCRYPTION))
	  ck_tn_decrypt((char *)&ch,1);
#endif /* CK_ENCRYPTION */
#endif /* IKSD */
	return((unsigned)(ch & 0xff));	/* Return it. */
    }
/*
  read() returned an error.  Same deal as above, but without the loop.
*/
    debug(F101, "coninc(timo) n","",n);
    debug(F101, "coninc(timo) errno","",errno);
#ifndef OXOS
#ifdef SVORPOSIX
    if (n == -1 && errno == EINTR && conesc != 0) {
	conesc = 0;
	return(escchr);			/* User entered escape character. */
    }
#endif /* SVORPOSIX */
    if (n == 0 && errno > 0) {		/* It's an error */
	return(-1);
    }
#endif /* ! OXOS */
    return(n);
}

/*  C O N G K S  --  Console Get Keyboard Scancode  */

#ifndef congks
/*
  This function needs to be filled in with the various system-dependent
  system calls used by SUNOS, NeXT OS, Xenix, Aviion, etc, to read a full
  keyboard scan code.  Unfortunately there aren't any.
*/
int
congks(timo) int timo; {

#ifdef IKSD
    if (inserver && !local)
      return(ttinc(timo));
#endif /* IKSD */

    return(coninc(timo));
}
#endif /* congks */

#ifdef ATT7300

/*  A T T D I A L  --  Dial up the remote system using internal modem
 * Purpose: to open and dial a number on the internal modem available on the
 * ATT7300 UNIX PC.  Written by Joe Doupnik. Superceeds version written by
 * Richard E. Hill, Dickinson, TX. which employed dial(3c).
 * Uses information in <sys/phone.h> and our status int attmodem.
 */
attdial(ttname,speed,telnbr) char *ttname,*telnbr; long speed; {
    char *telnum;

    attmodem &= ~ISMODEM;                       /* modem not in use yet */
                    /* Ensure O_NDELAY is set, else i/o traffic hangs */
                    /* We turn this flag off once the dial is complete */
    fcntl(ttyfd, F_SETFL, fcntl(ttyfd, F_GETFL, 0) | O_NDELAY);

    /* Condition line, check availability & DATA mode, turn on speaker */
    if (ioctl(ttyfd,PIOCOFFHOOK, &dialer) == -1) {
        printf("cannot access phone\n");
        ttclos(0);
        return (-2);
    }
    ioctl(ttyfd,PIOCGETP,&dialer);      /* get phone dialer parameters */

    if (dialer.c_lineparam & VOICE) {	/* phone must be in DATA mode */
        printf(" Should not dial with modem in VOICE mode.\n");
        printf(" Exit Kermit, switch to DATA and retry call.\n");
        ttclos(0);
        return (-2);
    }
#ifdef ATTTONED				/* Old way, tone dialing only. */
    dialer.c_lineparam = DATA | DTMF;	/* Dial with tones, */
    dialer.c_lineparam &= ~PULSE;	/* not with pulses. */
#else
    /* Leave current pulse/tone state alone. */
    /* But what about DATA?  Add it back if you have trouble. */
    /* sys/phone says you get DATA automatically by opening device RDWR */
#endif
    dialer.c_waitdialtone = 5;                  /* wait 5 sec for dialtone */
#ifdef COMMENT
    dialer.c_feedback = SPEAKERON|NORMSPK|RINGON;  /* control speaker */
#else
    /* sys/phone says RINGON used only for incoming voice calls */
    dialer.c_feedback &= ~(SOFTSPK|LOUDSPK);
    dialer.c_feedback |= SPEAKERON|NORMSPK;
#endif
    dialer.c_waitflash = 500;                   /* 0.5 sec flash hook */
    if(ioctl(ttyfd,PIOCSETP,&dialer) == -1) {   /* set phone parameters */
        printf("Cannot set modem characteristics\n");
        ttclos(0);
        return (-2);
    }
    ioctl(ttyfd,PIOCRECONN,0);		/* Turns on speaker for pulse */

#ifdef COMMENT
    fprintf(stderr,"Phone line status. line_par:%o dialtone_wait:%o \
line_status:%o feedback:%o\n",
    dialer.c_lineparam, dialer.c_waitdialtone,
    dialer.c_linestatus, dialer.c_feedback);
#endif

    attmodem |= ISMODEM;                        /* modem is now in-use */
    sleep(1);
    for (telnum = telnbr; *telnum != '\0'; telnum++)    /* dial number */
#ifdef ATTTONED
      /* Tone dialing only */
      if (ioctl(ttyfd,PIOCDIAL,telnum) != 0) {
	  perror("Error in dialing");
	  ttclos(0);
	  return(-2);
      }
#else /* Allow Pulse or Tone dialing */
    switch (*telnum) {
      case 't': case 'T': case '%':	/* Tone dialing requested */
	dialer.c_lineparam |= DTMF;
	dialer.c_lineparam &= ~PULSE;
	if (ioctl(ttyfd,PIOCSETP,&dialer) == -1) {
	    printf("Cannot set modem to tone dialing\n");
	    ttclos(0);
	    return(-2);
	}
	break;
      case 'd': case 'D': case 'p': case 'P': case '^':
	dialer.c_lineparam |= PULSE;
	dialer.c_lineparam &= ~DTMF;
	if (ioctl(ttyfd,PIOCSETP,&dialer) == -1) {
	    printf("Cannot set modem to pulse dialing\n");
	    ttclos(0);
	    return(-2);
	}
	break;
      default:
        if (ioctl(ttyfd,PIOCDIAL,telnum) != 0) {
	    perror("Dialing error");
	    ttclos(0);
	    return(-2);
	}
	break;
    }
#endif

    ioctl(ttyfd,PIOCDIAL,"@");		/* terminator for data call */
    do {				/* wait for modems to Connect */
        if (ioctl(ttyfd,PIOCGETP,&dialer) != 0)	{ /* get params */
	    perror("Cannot get modems to connect");
	    ttclos(0);
	    return(-2);
	}
    } while ((dialer.c_linestatus & MODEMCONNECTED) == 0);
    /* Turn off O_NDELAY flag now. */
    fcntl(ttyfd, F_SETFL, fcntl(ttyfd, F_GETFL, 0) & ~O_NDELAY);
    signal(SIGHUP, sighup);             /* hangup on loss of carrier */
    return(0);                          /* return success */
}

/*
  Offgetty, ongetty functions. These function get the 'getty(1m)' off
  and restore it to the indicated line.  Shell's return codes are:
    0: Can't do it.  Probably a user logged on.
    1: No need.  No getty on that line.
    2: Done, you should restore the getty when you're done.
  DOGETY System(3), however, returns them as 0, 256, 512, respectively.
  Thanks to Kevin O'Gorman, Anarm Software Systems.

   getoff.sh looks like:   geton.sh looks like:
     setgetty $1 0           setgetty $1 1
     err=$?                  exit $?
     sleep 2
     exit $err
*/

/*  O F F G E T T Y  --  Turn off getty(1m) for the communications tty line
 * and get status so it can be restarted after the line is hung up.
 */
int
offgetty(ttname) char *ttname; {
    char temp[30];
    while (*ttname != '\0') ttname++;       /* seek terminator of path */
    ttname -= 3;                            /* get last 3 chars of name */
    sprintf(temp,"/usr/bin/getoff.sh %s",ttname);
    return(zsyscmd(temp));
}

/*  O N G E T T Y  --  Turn on getty(1m) for the communications tty line */

int
ongetty(ttname) char *ttname; {
    char temp[30];
    while (*ttname != '\0') ttname++;       /* comms tty path name */
    ttname -= 3;
    sprintf(temp,"/usr/bin/geton.sh %s",ttname);
    return(zsyscmd(temp));
}
#endif /* ATT7300 */

/*  T T S C A R R  --  Set ttcarr variable, controlling carrier handling.
 *
 *  0 = Off: Always ignore carrier. E.g. you can connect without carrier.
 *  1 = On: Heed carrier, except during dialing. Carrier loss gives disconnect.
 *  2 = Auto: For "modem direct": The same as "Off".
 *            For real modem types: Heed carrier during connect, but ignore
 *                it anytime else.  Compatible with pre-5A C-Kermit versions.
 *
 * As you can see, this setting does not affect dialing, which always ignores
 * carrier (unless there is some special exception for some modem type).  It
 * does affect ttopen() if it is set before ttopen() is used.  This setting
 * takes effect on the next call to ttopen()/ttpkt()/ttvt().  And they are
 * (or should be) always called before any communications is tried, which
 * means that, practically speaking, the effect is immediate.
 *
 * Of course, nothing of this applies to remote mode (xlocal = 0).
 *
 * Someone has yet to uncover how to manipulate the carrier in the BSD
 * environment (or any non-termio using environment).  Until that time, this
 * will simply be a no-op for BSD.
 *
 * Note that in previous versions, the carrier was most often left unchanged
 * in ttpkt()/ttvt() unless they were called with FLO_DIAL or FLO_DIAX.  This
 * has changed.  Now it is controlled by ttcarr in conjunction with these
 * modes.
 */
int
ttscarr(carrier) int carrier; {
    ttcarr = carrier;
    debug(F101, "ttscarr","",ttcarr);
    return(ttcarr);
}

/* C A R R C T L  --  Set tty modes for carrier treatment.
 *
 * Sets the appropriate bits in a termio or sgttyb struct for carrier control
 * (actually, there are no bits in sgttyb for that), or performs any other
 * operations needed to control this on the current system.  The function does
 * not do the actual TCSETA or stty, since often we want to set other bits too
 * first.  Don't call this function when xlocal is 0, or the tty is not opened.
 *
 * We don't know how to do anything like carrier control on non-ATTSV systems,
 * except, apparently, ultrix.  See above.  It is also known that this doesn't
 * have much effect on a Xenix system.  For Xenix, one should switch back and
 * forth between the upper and lower case device files.  Maybe later.
 * Presently, Xenix will stick to the mode it was opened with.
 *
 * carrier: 0 = ignore carrier, 1 = require carrier.
 * The current state is saved in curcarr, and checked to save labour.
 */
#ifdef SVORPOSIX
int
#ifdef BSD44ORPOSIX
carrctl(ttpar, carrier)	struct termios *ttpar; int carrier;
#else /* ATTSV */
carrctl(ttpar, carrier)	struct termio *ttpar; int carrier;
#endif /* BSD44ORPOSIX */
/* carrctl */ {
    debug(F101, "carrctl","",carrier);
    if (carrier)
      ttpar->c_cflag &= ~CLOCAL;
    else
      ttpar->c_cflag |= CLOCAL;
    return(0);
}
#else /* Berkeley, V7, et al... */
int
carrctl(ttpar, carrier) struct sgttyb *ttpar; int carrier; {
    debug(F101, "carrctl","",carrier);
    if (carrier == curcarr)
      return(0);
    curcarr = carrier;
#ifdef ultrix
#ifdef COMMENT
/*
  Old code from somebody at DEC that tends to get stuck, time out, etc.
*/
    if (carrier) {
	ioctl(ttyfd, TIOCMODEM, &temp);
	ioctl(ttyfd, TIOCHPCL, 0);
    } else {
	/* (According to the manuals, TIOCNCAR should be preferred */
	/* over TIOCNMODEM...) */
	ioctl(ttyfd, TIOCNMODEM, &temp);
    }
#else
/*
  New code from Jamie Watson that, he says, eliminates the problems.
*/
    if (carrier) {
	ioctl(ttyfd, TIOCCAR);
	ioctl(ttyfd, TIOCHPCL);
    } else {
	ioctl(ttyfd, TIOCNCAR);
    }
#endif /* COMMENT */
#endif /* ultrix */
    return(0);
}
#endif /* SVORPOSIX */


/*  T T G M D M  --  Get modem signals  */
/*
 Looks for RS-232 modem signals, and returns those that are on in as its
 return value, in a bit mask composed of the BM_xxx values defined in ckcdeb.h.
 Returns:
 -3 Not implemented
 -2 if the communication device does not have modem control (e.g. telnet)
 -1 on error.
 >= 0 on success, with a bit mask containing the modem signals that are on.
*/

/*
  Define the symbol K_MDMCTL if we have Sys V R3 / 4.3 BSD style
  modem control, namely the TIOCMGET ioctl.
*/

#ifdef BSD43
#define K_MDMCTL
#endif /* BSD43 */

#ifdef SUNOS4
#define K_MDMCTL
#endif /* SUNOS4 */

/*
  SCO OpenServer R5.0.4.  The TIOCMGET definition is hardwired in because it
  is skipped in termio.h when _POSIX_SOURCE is defined.  But _POSIX_SOURCE
  must be defined in order to get the high serial speeds that are new to
  5.0.4.  However, the regular SCO drivers do not implement TIOCMGET, so the
  ioctl() returns -1 with errno 22 (invalid function).  But third-party
  drivers, e.g. for Digiboard, do implement it, and so it should work on ports
  driven by those drivers.
*/
#ifdef SCO_OSR504
#ifndef TIOCMGET
#define TIOCMGET (('t'<<8)|29)
#endif /* TIOCMGET */
#endif /* SCO_OSR504 */

#ifdef CK_SCOV5
/* Because POSIX strictness in <sys/termio.h> won't let us see these. */
#ifndef TIOCM_DTR
#define TIOCM_DTR	0x0002		/* data terminal ready */
#define TIOCM_RTS	0x0004		/* request to send */
#define TIOCM_CTS	0x0020		/* clear to send */
#define TIOCM_CAR	0x0040		/* carrier detect */
#define TIOCM_RNG	0x0080		/* ring */
#define TIOCM_DSR	0x0100		/* data set ready */
#define TIOCM_CD	TIOCM_CAR
#define TIOCM_RI	TIOCM_RNG
#endif /* TIOCM_DTR */
#endif /* CK_SCOV5 */

#ifdef QNX
#define K_MDMCTL
#else
#ifdef TIOCMGET
#define K_MDMCTL
#endif /* TIOCMGET */
#endif /* QNX */
/*
  "A serial communication program that can't read modem signals
   is like a car without windows."
*/
int
ttgmdm() {

#ifdef QNX
#include <sys/qioctl.h>

    unsigned long y, mdmbits[2];
    int x, z = 0;

    if (xlocal && ttyfd < 0)
      return(-1);

#ifdef NETCONN
    if (netconn) {			/* Network connection */
#ifdef TN_COMPORT
        if (istncomport()) {
	    gotsigs = 1;
	    return(tngmdm());
	} else
#endif /* TN_COMPORT */
	  return(-2);			/* No modem signals */
    }
#endif /* NETCONN */

#ifdef NETCMD
    if (ttpipe) return(-2);
#endif /* NETCMD */
#ifdef NETPTY
    if (ttpty) return(-2);
#endif /* NETPTY */

    mdmbits[0] = 0L;
    mdmbits[1] = 0L;
/*
 * From <sys/qioctl.h>:
 *
 * SERIAL devices   (all Dev.ser versions)
 * 0 : DTR           8 = Data Bits 0  16 - reserved     24 - reserved
 * 1 : RTS           9 = Data Bits 1  17 - reserved     25 - reserved
 * 2 = Out 1        10 = Stop Bits    18 - reserved     26 - reserved
 * 3 = Int Enable   11 = Par Enable   19 - reserved     27 - reserved
 * 4 = Loop         12 = Par Even     20 = CTS          28 - reserved
 * 5 - reserved     13 = Par Stick    21 = DSR          29 - reserved
 * 6 - reserved     14 : Break        22 = RI           30 - reserved
 * 7 - reserved     15 = 0            23 = CD           31 - reserved
 */
    errno = 0;
    x = qnx_ioctl(ttyfd, QCTL_DEV_CTL, &mdmbits[0], 8, &mdmbits[0], 4);
    debug(F101,"ttgmdm qnx_ioctl","",x);
    debug(F101,"ttgmdm qnx_ioctl errno","",errno);
    if (!x) {
	debug(F101,"ttgmdm qnx_ioctl mdmbits[0]","",mdmbits[0]);
	debug(F101,"ttgmdm qnx_ioctl mdmbits[1]","",mdmbits[1]);
	y = mdmbits[0];
	if (y & 0x000001L) z |= BM_DTR;	/* Bit  0 */
	if (y & 0x000002L) z |= BM_RTS;	/* Bit  1 */
	if (y & 0x100000L) z |= BM_CTS;	/* Bit 20 */
	if (y & 0x200000L) z |= BM_DSR;	/* Bit 21 */
	if (y & 0x400000L) z |= BM_RNG;	/* Bit 22 */
	if (y & 0x800000L) z |= BM_DCD;	/* Bit 23 */
	debug(F101,"ttgmdm qnx result","",z);
	debug(F110,"ttgmdm qnx CD = ",(z & BM_DCD) ? "On" : "Off", 0);
	gotsigs = 1;
	return(z);
    } else return(-1);
#else /* QNX */
#ifdef HPUX				/* HPUX has its own way */
    int x, z;

#ifdef HPUX10				/* Modem flag word */
    mflag y;				/* mflag typedef'd in <sys/modem.h> */
#else
#ifdef HPUX9
    mflag y;
#else
#ifdef HPUX8
    mflag y;
#else
    unsigned long y;			/* Not sure about pre-8.0... */
#endif /* HPUX8 */
#endif /* HPUX9 */
#endif /* HPUX10 */

    if (xlocal && ttyfd < 0)
      return(-1);

#ifdef NETCONN
    if (netconn) {			/* Network connection */
#ifdef TN_COMPORT
        if (istncomport()) {
	    gotsigs = 1;
	    return(tngmdm());
	} else
#endif /* TN_COMPORT */
	  return(-2);			/* No modem signals */
    }
#endif /* NETCONN */

#ifdef NETCMD
    if (ttpipe) return(-2);
#endif /* NETCMD */
#ifdef NETPTY
    if (ttpty) return(-2);
#endif /* NETPTY */

    if (xlocal)				/* Get modem signals */
      x = ioctl(ttyfd,MCGETA,&y);
    else
      x = ioctl(0,MCGETA,&y);
    if (x < 0) return(-1);
    debug(F101,"ttgmdm","",y);

    z = 0;				/* Initialize return value */

/* Now set bits for each modem signal that is reported to be on. */

#ifdef MCTS
    /* Clear To Send */
    debug(F101,"ttgmdm HPUX CTS","",y & MCTS);
    if (y & MCTS) z |= BM_CTS;
#endif
#ifdef MDSR
    /* Data Set Ready */
    debug(F101,"ttgmdm HPUX DSR","",y & MDSR);
    if (y & MDSR) z |= BM_DSR;
#endif
#ifdef MDCD
    /* Carrier */
    debug(F101,"ttgmdm HPUX DCD","",y & MDCD);
    if (y & MDCD) z |= BM_DCD;
#endif
#ifdef MRI
    /* Ring Indicate */
    debug(F101,"ttgmdm HPUX RI","",y & MRI);
    if (y & MRI) z |= BM_RNG;
#endif
#ifdef MDTR
    /* Data Terminal Ready */
    debug(F101,"ttgmdm HPUX DTR","",y & MDTR);
    if (y & MDTR) z |= BM_DTR;
#endif
#ifdef MRTS
    /* Request To Send */
    debug(F101,"ttgmdm HPUX RTS","",y & MRTS);
    if (y & MRTS) z |= BM_RTS;
#endif
    gotsigs = 1;
    return(z);

#else /* ! HPUX */

#ifdef K_MDMCTL
/*
  Note, TIOCMGET might already have been defined in <sys/ioctl.h> or elsewhere.
  If not, we try including <sys/ttycom.h> -- if this blows up then more ifdefs
  are needed.
*/
#ifndef TIOCMGET
#include <sys/ttycom.h>
#endif /* TIOCMGET */

    int x, y, z;

    debug(F100,"ttgmdm K_MDMCTL defined","",0);

#ifdef NETCONN
    if (netconn) {			/* Network connection */
#ifdef TN_COMPORT
        if (istncomport()) {
	    gotsigs = 1;
	    return(tngmdm());
	} else
#endif /* TN_COMPORT */
	  return(-2);			/* No modem signals */
    }
#endif /* NETCONN */

#ifdef NETCMD
    if (ttpipe) return(-2);
#endif /* NETCMD */
#ifdef NETPTY
    if (ttpty) return(-2);
#endif /* NETPTY */

    if (xlocal && ttyfd < 0)
      return(-1);

    if (xlocal)
      x = ioctl(ttyfd,TIOCMGET,&y);	/* Get modem signals. */
    else
      x = ioctl(0,TIOCMGET,&y);
    debug(F101,"ttgmdm TIOCMGET ioctl","",x);
    if (x < 0) {
	debug(F101,"ttgmdm errno","",errno);
	return(-1);
    }
    debug(F101,"ttgmdm bits","",y);

    z = 0;				/* Initialize return value. */
#ifdef TIOCM_CTS
    /* Clear To Send */
    if (y & TIOCM_CTS) z |= BM_CTS;
    debug(F101,"ttgmdm TIOCM_CTS defined","",TIOCM_CTS); 
#else
    debug(F100,"ttgmdm TIOCM_CTS not defined","",0);
#endif
#ifdef TIOCM_DSR
    /* Data Set Ready */
    if (y & TIOCM_DSR) z |= BM_DSR;
    debug(F101,"ttgmdm TIOCM_DSR defined","",TIOCM_DSR); 
#else
    debug(F100,"ttgmdm TIOCM_DSR not defined","",0);
#endif
#ifdef TIOCM_CAR
    /* Carrier */
    if (y & TIOCM_CAR) z |= BM_DCD;
    debug(F101,"ttgmdm TIOCM_CAR defined","",TIOCM_CAR); 
#else
    debug(F100,"ttgmdm TIOCM_CAR not defined","",0);
#endif
#ifdef TIOCM_RNG
    /* Ring Indicate */
    if (y & TIOCM_RNG) z |= BM_RNG;
    debug(F101,"ttgmdm TIOCM_RNG defined","",TIOCM_RNG); 
#else
    debug(F100,"ttgmdm TIOCM_RNG not defined","",0);
#endif
#ifdef TIOCM_DTR
    /* Data Terminal Ready */
    if (y & TIOCM_DTR) z |= BM_DTR;
    debug(F101,"ttgmdm TIOCM_DTR defined","",TIOCM_DTR); 
#else
    debug(F100,"ttgmdm TIOCM_DTR not defined","",0);
#endif
#ifdef TIOCM_RTS
    /* Request To Send */
    if (y & TIOCM_RTS) z |= BM_RTS;
    debug(F101,"ttgmdm TIOCM_RTS defined","",TIOCM_RTS); 
#else
    debug(F100,"ttgmdm TIOCM_RTS not defined","",0);
#endif
    gotsigs = 1;
    return(z);

#else /* !K_MDMCTL catch-All */

    debug(F100,"ttgmdm K_MDMCTL not defined","",0);
#ifdef TIOCMGET
    debug(F100,"ttgmdm TIOCMGET defined","",0);
#else
    debug(F100,"ttgmdm TIOCMGET not defined","",0);
#endif /* TIOCMGET */
#ifdef _SVID3
    debug(F100,"ttgmdm _SVID3 defined","",0);
#else
    debug(F100,"ttgmdm _SVID3 not defined","",0);
#endif /* _SVID3 */

#ifdef NETCONN
    if (netconn) {			/* Network connection */
#ifdef TN_COMPORT
        if (istncomport()) {
	    gotsigs = 1;
	    return(tngmdm());
	} else
#endif /* TN_COMPORT */
	  return(-2);			/* No modem signals */
    }
#endif /* NETCONN */

#ifdef NETCMD
    if (ttpipe) return(-2);
#endif /* NETCMD */
#ifdef NETPTY
    if (ttpty) return(-2);
#endif /* NETPTY */

    return(-3);				/* Sorry, I don't know how... */

#endif /* K_MDMCTL */
#endif /* HPUX */
#endif /* QNX */
}

/*  P S U S P E N D  --  Put this process in the background.  */

/*
  Call with flag nonzero if suspending is allowed, zero if not allowed.
  Returns 0 on apparent success, -1 on failure (flag was zero, or
  kill() returned an error code.
*/
int
psuspend(flag) int flag; {

#ifdef RTU
    extern int rtu_bug;
#endif /* RTU */

    if (flag == 0) return(-1);

#ifdef NOJC
    return(-1);
#else
#ifdef SIGTSTP
/*
  The big question here is whether job control is *really* supported.
  There's no way Kermit can know for sure.  The fact that SIGTSTP is
  defined does not guarantee the Unix kernel supports it, and the fact
  that the Unix kernel supports it doesn't guarantee that the user's
  shell (or other process that invoked Kermit) supports it.
*/
#ifdef RTU
    rtu_bug = 1;
#endif /* RTU */
    if (kill(0,SIGSTOP) < 0
#ifdef MIPS
/* Let's try this for MIPS too. */
	&& kill(getpid(),SIGSTOP) < 0
#endif /* MIPS */
	) {				/* If job control, suspend the job */
	perror("suspend");
	debug(F101,"psuspend error","",errno);
	return(-1);
    }
    debug(F100,"psuspend ok","",0);
    return(0);
#else
    return(-1);
#endif /* SIGTSTP */
#endif /* NOJC */
}

/*
  setuid package, by Kristoffer Eriksson, with contributions from Dean
  Long and fdc.
*/

/* The following is for SCO when CK_ANSILIBS is defined... */
#ifdef M_UNIX
#ifdef CK_ANSILIBS
#ifndef NOGETID_PROTOS
#define NOGETID_PROTOS
#endif /* NOGETID_PROTOS */
#endif /* CK_ANSILIBS */
#endif /* M_UNIX */

#ifndef _POSIX_SOURCE
#ifndef SUNOS4
#ifndef NEXT
#ifndef PS2AIX10
#ifndef sequent
#ifndef HPUX9
#ifndef HPUX10
#ifndef COHERENT
#ifndef NOGETID_PROTOS
_PROTOTYP( UID_T getuid, (void) );
_PROTOTYP( UID_T geteuid, (void) );
_PROTOTYP( UID_T getreuid, (void) );
_PROTOTYP( UID_T getgid, (void) );
_PROTOTYP( UID_T getegid, (void) );
_PROTOTYP( UID_T getregid, (void) );
#endif /* NOGETID_PROTOS */
#else
_PROTOTYP( UID_T getreuid, (void) );
_PROTOTYP( UID_T getregid, (void) );
#endif /* COHERENT */
#endif /* HPUX10 */
#endif /* HPUX9 */
#endif /* sequent */
#endif /* PS2AIX10 */
#endif /* NEXT */
#endif /* SUNOS4 */
#endif /* _POSIX_SOURCE */

/*
Subject: Set-user-id
To: fdc@watsun.cc.columbia.edu (Frank da Cruz)
Date: Sat, 21 Apr 90 4:48:25 MES
From: Kristoffer Eriksson <ske@pkmab.se>

This is a set of functions to be used in programs that may be run set-user-id
and/or set-group-id. They handle both the case where the program is not run
with such privileges (nothing special happens then), and the case where one
or both of these set-id modes are used.  The program is made to run with the
user's real user and group ids most of the time, except for when more
privileges are needed.  Don't set-user-id to "root".

This works on System V and POSIX.  In BSD, it depends on the
"saved-set-user-id" feature.
*/

#define UID_ROOT 0			/* Root user and group ids */
#define GID_ROOT 0

/*
  The following code defines the symbol SETEUID for UNIX systems based
  on BSD4.4 (either -Encumbered or -Lite).  This program will then use
  seteuid() and setegid() instead of setuid() and setgid(), which still
  don't allow arbitrary switching.  It also avoids setreuid() and
  setregid(), which are included in BSD4.4 for compatibility only, are
  insecure, and print warnings to stderr under at least one system (NetBSD
  1.0).  Note that POSIX systems should still use setuid() and setgid();
  the seteuid() and setegid() functions are BSD4.4 extensions to the
  POSIX model.  Mike Long <mike.long@analog.com>, 8/94.
*/
#ifdef BSD44
#define SETEUID
#endif /* BSD44 */

/*
  The following construction automatically defines the symbol SETREUID for
  UNIX versions based on Berkeley Unix 4.2 and 4.3.  If this symbol is
  defined, then this program will use getreuid() and getregid() calls in
  preference to getuid() and getgid(), which in Berkeley-based Unixes do
  not allow arbitrary switching back and forth of real & effective uid.
  This construction also allows -DSETREUID to be put on the cc command line
  for any system that has and wants to use setre[ug]id().  It also prevents
  automatic definition of SETREUID if -DNOSETREU is included on the cc
  command line (or otherwise defined).
*/
#ifdef FT18				/* None of this for Fortune. */
#define NOSETREU
#endif /* FT18 */

#ifdef ANYBSD
#ifndef BSD29
#ifndef BSD41
#ifndef SETREUID
#ifndef NOSETREU
#ifndef SETEUID
#define SETREUID
#endif /* SETEUID */
#endif /* NOSETREU */
#endif /* SETREUID */
#endif /* !BSD41 */
#endif /* !BSD29 */
#endif /* ANYBSD */

/* Variables for user and group IDs. */

static UID_T realuid = (UID_T) -1, privuid = (UID_T) -1;
static GID_T realgid = (GID_T) -1, privgid = (GID_T) -1;


/* P R I V _ I N I  --  Initialize privileges package  */

/* Called as early as possible in a set-uid or set-gid program to store the
 * set-to uid and/or gid and step down to the users real uid and gid. The
 * stored id's can be temporarily restored (allowed in System V) during
 * operations that require the privilege.  Most of the time, the program
 * should execute in unpriviliged state, to not impose any security threat.
 *
 * Note: Don't forget that access() always uses the real id:s to determine
 * file access, even with privileges restored.
 *
 * Returns an error mask, with error values or:ed together:
 *   1 if setuid() fails,
 *   2 if setgid() fails, and
 *   4 if the program is set-user-id to "root", which can't be handled.
 *
 * Only the return value 0 indicates real success. In case of failure,
 * those privileges that could be reduced have been, at least, but the
 * program should be aborted none-the-less.
 *
 * Also note that these functions do not expect the uid or gid to change
 * without their knowing. It may work if it is only done temporarily, but
 * you're on your own.
 */
int
priv_ini() {
    int err = 0;

#ifndef HAVE_LOCKDEV

    /* Save real ID:s. */
    realuid = getuid();
    realgid = getgid();

    /* Save current effective ID:s, those set to at program exec. */
    privuid = geteuid();
    privgid = getegid();

    /* If running set-uid, go down to real uid, otherwise remember that
     * no privileged uid is available.
     *
     * Exceptions:
     *
     * 1) If the real uid is already "root" and the set-uid uid (the
     * initial effective uid) is not "root", then we would have trouble
     * if we went "down" to "root" here, and then temporarily back to the
     * set-uid uid (not "root") and then again tried to become "root". I
     * think the "saved set-uid" is lost when changing uid from effective
     * uid "root", which changes all uid, not only the effective uid. But
     * in this situation, we can simply go to "root" and stay there all
     * the time. That should give sufficient privilege (understatement!),
     * and give the right uids for subprocesses.
     *
     * 2) If the set-uid (the initial effective uid) is "root", and we
     * change uid to the real uid, we can't change it back to "root" when
     * we need the privilege, for the same reason as in 1). Thus, we can't
     * handle programs that are set-user-id to "root" at all. The program
     * should be stopped.  Use some other uid.  "root" is probably too
     * privileged for such things, anyway. (The uid is reverted to the
     * real uid until termination.)
     *
     * These two exceptions have the effect that the "root" uid will never
     * be one of the two uids that are being switched between, which also
     * means we don't have to check for such cases in the switching
     * functions.
     *
     * Note that exception 1) is handled by these routines (by constantly
     * running with uid "root", while exception 2) is a serious error, and
     * is not provided for at all in the switching functions.
     */
    if (realuid == privuid)
	privuid = (UID_T) -1;		/* Not running set-user-id. */

    /* If running set-gid, go down to real gid, otherwise remember that
     * no privileged gid is available.
     *
     * There are no exception like there is for the user id, since there
     * is no group id that is privileged in the manner of uid "root".
     * There could be equivalent problems for group changing if the
     * program sometimes ran with uid "root" and sometimes not, but
     * that is already avoided as explained above.
     *
     * Thus we can expect always to be able to switch to the "saved set-
     * gid" when we want, and back to the real gid again. You may also
     * draw the conclusion that set-gid provides for fewer hassles than
     * set-uid.
     */

#ifdef SUIDDEBUG
    fprintf(stderr,"UID_ROOT=%d\n",UID_ROOT);
    fprintf(stderr,"realuid=%d\n",realuid);
    fprintf(stderr,"privuid=%d\n",privuid);
#endif /* SUIDDEBUG */

    if (realgid == privgid)		/* If not running set-user-id, */
      privgid = (GID_T) -1;		/*  remember it this way. */

    err = priv_off();			/* Turn off setuid privilege. */

    if (privuid == UID_ROOT)		/* If setuid to root, */
      err |= 4;				/* return this error. */

    if (realuid == UID_ROOT) {		/* If real id is root, */
	privuid = (UID_T) -1;		/* stay root at all times. */
#ifdef ATT7300
	/* If Kermit installed SUID uucp and user is running as root */
	err &= ~1;			/* System V R0 does not save UID */
#endif /* ATT7300 */
    }
#endif /* HAVE_LOCKDEV */
    return(err);
}


/* Macros for hiding the differences in UID/GID setting between various Unix
 * systems. These macros should always be called with both the privileged ID
 * and the non-privileged ID. The one in the second argument, will become the
 * effective ID. The one in the first argument will be retained for later
 * retrieval.
 */
#ifdef SETREUID
#ifdef SAVEDUID
/* On BSD systems with the saved-UID feature, we just juggle the effective
 * UID back and forth, and leave the real UID at its true value.  The kernel
 * allows switching to both the current real UID, the effective UID, and the
 * UID which the program is set-UID to.  The saved set-UID always holds the
 * privileged UID for us, and the real UID will always be the non-privileged,
 * and we can freely choose one of them for the effective UID at any time.
 */
#define switchuid(hidden,active) setreuid( (UID_T) -1, active)
#define switchgid(hidden,active) setregid( (GID_T) -1, active)

#else   /* SETREUID,!SAVEDUID */

/* On systems with setreXid() but without the saved-UID feature, notably
 * BSD 4.2, we swap the real and effective UIDs each time.  It's
 * the effective UID that we are interested in, but we have to retain the
 * unused UID somewhere to enable us to restore it later, and we do this
 * in the real UID.  The kernel only allows switching to either the current
 * real or the effective UID, unless you're "root".
 */
#define switchuid(hidden,active)	setreuid(hidden,active)
#define switchgid(hidden,active)	setregid(hidden,active)
#endif

#else /* !SETREUID, !SAVEDUID */

#ifdef SETEUID
/*
  BSD 4.4 works similarly to System V and POSIX (see below), but uses
  seteXid() instead of setXid() to change effective IDs.  In addition, the
  seteXid() functions work the same for "root" as for other users.
*/
#define switchuid(hidden,active)	seteuid(active)
#define switchgid(hidden,active)	setegid(active)

#else /* !SETEUID */

/* On System V and POSIX, the only thing we can change is the effective UID
 * (unless the current effective UID is "root", but initsuid() avoids that for
 * us).  The kernel allows switching to the current real UID or to the saved
 * set-UID.  These are always set to the non-privileged UID and the privileged
 * UID, respectively, and we only change the effective UID.  This breaks if
 * the current effective UID is "root", though, because for "root" setuid/gid
 * becomes more powerful, which is why initsuid() treats "root" specially.
 * Note: That special treatment maybe could be ignored for BSD?  Note: For
 * systems that don't fit any of these four cases, we simply can't support
 * set-UID.
 */
#define switchuid(hidden,active)	setuid(active)
#define switchgid(hidden,active)	setgid(active)

#endif /* SETEUID */
#endif /* SETREUID */


/* P R I V _ O N  --  Turn on the setuid and/or setgid */

/* Go to the privileged uid (gid) that the program is set-user-id
 * (set-group-id) to, unless the program is running unprivileged.
 * If setuid() fails, return value will be 1. If getuid() fails it
 * will be 2.  Return immediately after first failure, and the function
 * tries to restore any partial work done.  Returns 0 on success.
 * Group id is changed first, since it is less serious than user id.
 */
int
priv_on() {
#ifndef HAVE_LOCKDEV
    if (privgid != (GID_T) -1)
      if (switchgid(realgid,privgid))
        return(2);

    if (privuid != (UID_T) -1)
      if (switchuid(realuid,privuid)) {
	  if (privgid != (GID_T) -1)
	    switchgid(privgid,realgid);
	  return(1);
      }
#endif /* HAVE_LOCKDEV */
    return(0);
}

/* P R I V _ O F F  --  Turn on the real uid and gid */

/* Return to the unprivileged uid (gid) after an temporary visit to
 * privileged status, unless the program is running without set-user-id
 * (set-group-id). Returns 1 for failure in setuid() and 2 for failure
 * in setgid() or:ed together. The functions tries to return both uid
 * and gid to unprivileged state, regardless of errors. Returns 0 on
 * success.
 */
int
priv_off() {
    int err = 0;
#ifndef HAVE_LOCKDEV
    if (privuid != (UID_T) -1)
       if (switchuid(privuid,realuid))
	  err |= 1;

    if (privgid != (GID_T) -1)
       if (switchgid(privgid,realgid))
	err |= 2;
#endif /* HAVE_LOCKDEV */
    return(err);
}

/* Turn off privilege permanently.  No going back.  This is necessary before
 * a fork() on BSD43 machines that don't save the setUID or setGID, because
 * we swap the real and effective ids, and we don't want to let the forked
 * process swap them again and get the privilege back. It will work on other
 * machines too, such that you can rely on its effect always being the same,
 * for instance, even when you're in priv_on() state when this is called.
 * (Well, that part about "permanent" is on System V only true if you follow
 * this with a call to exec(), but that's what we want it for anyway.)
 * Added by Dean Long -- dlong@midgard.ucsc.edu
 */
int
priv_can() {
#ifndef HAVE_LOCKDEV
#ifdef SETREUID
    int err = 0;
    if (privuid != (UID_T) -1)
       if (setreuid(realuid,realuid))
	  err |= 1;

    if (privgid != (GID_T) -1)
        if (setregid(realgid,realgid))
 	  err |= 2;

    return(err);

#else
#ifdef SETEUID
    int err = 0;
    if (privuid != (UID_T) -1)
	if (setuid(realuid)) {
	    debug(F101,"setuid failed","",errno);
	    err |= 1;
	    debug(F101,"ruid","",getuid());
	    debug(F101,"euid","",geteuid());
	}
    debug(F101,"setuid","",realuid);
    if (privgid != (GID_T) -1)
        if (setgid(realgid)) {
	    debug(F101,"setgid failed","",errno);
	    err |= 2;
	    debug(F101,"rgid","",getgid());
	    debug(F101,"egid","",getegid());
	}
    debug(F101,"setgid","",realgid);
    return(err);
#else
    /* Easy way of using setuid()/setgid() instead of setreuid()/setregid().*/
    return(priv_off());
#endif /* SETEUID */
#endif /* SETREUID */
#else
    return(0);
#endif /* HAVE_LOCKDEV */
}

/* P R I V _ O P N  --  For opening protected files or devices. */

int
priv_opn(name, modes) char *name; int modes; {
    int x;
    priv_on();				/* Turn privileges on */
    debug(F111,"priv_opn",name,modes);
    errno = 0;
    x = open(name, modes);		/* Try to open the device */
    debug(F101,"priv_opn result","",x);
    debug(F101,"priv_opn errno","",errno);
    priv_off();				/* Turn privileges off */
    return(x);				/* Return open's return code */
}

/*  P R I V _ C H K  --  Check privileges.  */

/*  Try to turn them off.  If turning them off did not succeed, cancel them */

int
priv_chk() {
    int x, y = 0;
    x = priv_off();			/* Turn off privs. */
    if (x != 0 || getuid() == privuid || geteuid() == privuid)
      y = priv_can();
    if (x != 0 || getgid() == privgid || getegid() == privgid)
      y = y | priv_can();
    return(y);
}

UID_T
real_uid() {
    return(realuid);
}

VOID
ttimoff() {				/* Turn off any timer interrupts */
    /* int xx; */
/*
  As of 5A(183), we set SIGALRM to SIG_IGN (to ignore alarms) rather than to
  SIG_DFL (to catch alarms, or if there is no handler, to exit).  This is to
  cure (mask, really) a deeper problem with stray alarms that occurs on some
  systems, possibly having to do with sleep(), that caused core dumps.  It
  should be OK to do this, because no code in this module uses nested alarms.
  (But we still have to watch out for SCRIPT and DIAL...)
*/
    /* xx = */ alarm(0);
    /* debug(F101,"ttimoff alarm","",xx); */
    if (saval) {			/* Restore any previous */
	signal(SIGALRM,saval);		/* alarm handler. */
	/* debug(F101,"ttimoff alarm restoring saval","",saval); */
	saval = NULL;
    } else {
	signal(SIGALRM,SIG_IGN);	/* Used to be SIG_DFL */
	/* debug(F100,"ttimoff alarm SIG_IGN","",0); */
    }
}


int
tt_is_secure() {	  /* Tells whether the current connection is secure */

    if (ttyfd == -1)
      return(0);

    if (0
#ifdef SSHBUILTIN
	|| IS_SSH()
#endif /* SSHBUILTIN */
#ifdef CK_ENCRYPTION
	|| ck_tn_encrypting() && ck_tn_decrypting()
#endif /* CK_ENCRYPTION */
#ifdef CK_SSL
	|| tls_active_flag || ssl_active_flag
#endif /* CK_SSL */
#ifdef RLOGCODE
#ifdef CK_KERBEROS
#ifdef CK_ENCRYPTION
	|| ttnproto == NP_EK4LOGIN || ttnproto == NP_EK5LOGIN
#endif /* CK_ENCRYPTION */
#endif /* CK_KERBEROS */
#endif /* RLOGCODE */
	)
      return(1);
    return(0);
}

#ifdef CK_REDIR
  
/* External protocol handler parameters from ckuus3.c */
extern int exp_handler, exp_stderr, exp_timo;

#ifdef SELECT
#ifdef NETPTY

/* The right size is 24576 */

#ifndef PTY_PBUF_SIZE			/* Size of buffer to read from pty */
#define PTY_PBUF_SIZE 24576		/* and write to net. */
#endif	/* PTY_PBUF_SIZE */

#ifndef PTY_TBUF_SIZE			/* Size of buffer to read from net */
#define PTY_TBUF_SIZE 24576		/* and write to pty. */
#endif	/* PTY_TBUF_SIZE */

#ifdef O_NDELAY				/* Whether to use nonblocking */
#ifndef PTY_NO_NDELAY			/* reads on the pseudoterminal */
#ifndef PTY_USE_NDELAY
#define PTY_USE_NDELAY
#endif	/* PTY_USE_NDELAY */
#endif	/* PTY_NO_NDELAY */
#endif	/* O_NDELAY */

#ifndef HAVE_OPENPTY
#ifndef USE_CKUPTY_C
#define USE_CKUPTY_C
#endif /* USE_CKUPTY_C */
#endif /* HAVE_OPENPTY */

VOID
pty_make_raw(fd) int fd; {
    int x = -23, i;

#ifdef BSD44ORPOSIX			/* POSIX */
    struct termios tp;
#else
#ifdef ATTSV				/* AT&T UNIX */
#ifdef CK_ANSIC
    struct termio tp = {0};
#else
    struct termio tp;
#endif	/* CK_ANSIC */
#else
    struct sgttyb tp;			/* Traditional */
#endif /* ATTSV */
#endif /* BSD44ORPOSIX */

    debug(F101,"pty_make_raw fd","",fd);
    errno = 0;

#ifdef BSD44ORPOSIX			/* POSIX */
    x = tcgetattr(fd,&tp);
    debug(F101,"pty_make_raw tcgetattr","",x);
#else
#ifdef ATTSV				/* AT&T UNIX */
    x = ioctl(fd,TCGETA,&tp);
    debug(F101,"pty_make_raw TCGETA ioctl","",x);
#else
    x = gtty(fd,&tp);
    debug(F101,"pty_make_raw ttty","",x);
#endif /* ATTSV */
#endif /* BSD44ORPOSIX */
    debug(F101,"pty_make_raw GET errno","",errno);

#ifdef USE_CFMAKERAW
    errno = 0;
    cfmakeraw(&tp);
    debug(F101,"pty_make_raw cfmakeraw errno","",errno);
#else  /* USE_CFMAKERAW */

#ifdef COMMENT

/* This very simple version recommended by Serg Iakolev doesn't work */

    tp.c_lflag &= ~(ECHO|ICANON|IEXTEN|ISIG);
    tp.c_iflag &= ~(BRKINT|ICRNL|INPCK|ISTRIP|IXON);
    tp.c_cflag &= ~(CSIZE|PARENB);
    tp.c_cflag |= CS8;
    tp.c_oflag &= ~(OPOST);
    tp.c_cc[VMIN] = 1;
    tp.c_cc[VTIME] = 0;

    debug(F101,"pty_make_raw 1 c_cc[] NCCS","",NCCS);
    debug(F101,"pty_make_raw 1 iflags","",tp.c_iflag);
    debug(F101,"pty_make_raw 1 oflags","",tp.c_oflag);
    debug(F101,"pty_make_raw 1 lflags","",tp.c_lflag);
    debug(F101,"pty_make_raw 1 cflags","",tp.c_cflag);

#else
#ifdef COMMENT
/*
  In this version we unset everything and then set only the
  bits we know we need.
*/
    /* iflags */
    tp.c_iflag = 0L;
    tp.c_iflag |= IGNBRK;
#ifdef IMAXBEL
    tp.c_iflag |= IMAXBEL;
#endif /* IMAXBEL */

    /* oflags */
    tp.c_oflag = 0L;

    /* lflags */
    tp.c_lflag = 0L;
#ifdef NOKERNINFO
    tp.c_lflag |= NOKERNINFO;
#endif	/* NOKERNINFO */

    /* cflags */
    tp.c_cflag = 0L;
    tp.c_cflag |= CS8|CREAD;

    for (i = 0; i < NCCS; i++) {	/* No special characters */
	tp.c_cc[i] = 0;
    }
#ifdef VMIN
    tp.c_cc[VMIN] = 1;			/* But always wait for input */
#endif	/* VMIN */
    debug(F101,"pty_make_raw 2 c_cc[] NCCS","",NCCS);
    debug(F101,"pty_make_raw 2 iflags","",tp.c_iflag);
    debug(F101,"pty_make_raw 2 oflags","",tp.c_oflag);
    debug(F101,"pty_make_raw 2 lflags","",tp.c_lflag);
    debug(F101,"pty_make_raw 2 cflags","",tp.c_cflag);

#else  /* COMMENT */
/*
  In this version we set or unset every single flag explicitly.  It works a
  bit better than the simple version just above, but it's still far from
  adequate.
*/
    /* iflags */
    tp.c_iflag &= ~(PARMRK|ISTRIP|BRKINT|INLCR|IGNCR|ICRNL);
    tp.c_iflag &= ~(INPCK|IGNPAR|IXANY|IXON|IXOFF);
    tp.c_iflag |= IGNBRK;
#ifdef IMAXBEL
#ifdef COMMENT
    tp.c_iflag |= IMAXBEL;
#else
    tp.c_iflag &= ~IMAXBEL;
#endif /* COMMENT */
#endif /* IMAXBEL */
#ifdef IUCLC
    tp.c_iflag &= ~IUCLC;
#endif /* IUCLC */

    /* oflags */
#ifdef BSDLY
    tp.c_oflag &= ~BSDLY;
#endif /* BSDLY */
#ifdef CRDLY
    tp.c_oflag &= ~CRDLY;
#endif /* CRDLY */
#ifdef FFDLY
    tp.c_oflag &= ~FFDLY;
#endif /* FFDLY */
#ifdef NLDLY
    tp.c_oflag &= ~NLDLY;
#endif /* NLDLY */
#ifdef TABDLY
    tp.c_oflag &= ~TABDLY;
#endif /* TABDLY */
#ifdef VTDLY
    tp.c_oflag &= ~VTDLY;
#endif /* VTDLY */
#ifdef OFDEL
    tp.c_oflag &= ~OFDEL;
#endif /* OFDEL */
#ifdef OFILL
    tp.c_oflag &= ~OFILL;
#endif /* OFILL */
#ifdef OLCUC
    tp.c_oflag &= ~OLCUC;
#endif /* OLCUC */
#ifdef CMSPAR
    tp.c_oflag &= ~CMSPAR;
#endif /* CMSPAR */
    tp.c_oflag &= ~OPOST;
#ifdef OXTABS
    tp.c_oflag &= ~OXTABS;
#endif /* OXTABS */
#ifdef COMMENT
#ifdef ONOCR
    tp.c_oflag &= ~ONOCR;		/* Maybe should be |=? */
    tp.c_oflag |= ONOCR;		/* makes no difference either way */
#endif /* ONOCR */
#endif /* COMMENT */
#ifdef ONOEOT
    tp.c_oflag &= ~ONOEOT;
#endif /* ONOEOT */
#ifdef ONLRET
    tp.c_oflag &= ~ONLRET;
#endif /* ONLRET */
#ifdef ONLCR
    tp.c_oflag &= ~ONLCR;
#endif /* ONLCR */
#ifdef OCRNL
    tp.c_oflag &= ~OCRNL;
#endif /* OCRNL */

    /* lflags */
    tp.c_lflag &= ~ECHO;
#ifdef ECHOE
    tp.c_lflag &= ~ECHOE;
#endif /* ECHOE */
#ifdef ECHONL
    tp.c_lflag &= ~ECHONL;
#endif /* ECHONL */
#ifdef ECHOPRT
    tp.c_lflag &= ~ECHOPRT;
#endif /* ECHOPRT */
#ifdef ECHOKE
    tp.c_lflag &= ~ECHOKE;
#endif /* ECHOKE */
#ifdef ECHOCTL
    tp.c_lflag &= ~ECHOCTL;
#endif /* ECHOCTL */
#ifdef XCASE
    tp.c_lflag &= ~XCASE;
#endif /* XCASE */
#ifdef ALTWERASE
    tp.c_lflag &= ~ALTWERASE;
#endif /* ALTWERASE */
#ifdef EXTPROC
    tp.c_lflag &= ~(ICANON|ISIG|IEXTEN|TOSTOP|FLUSHO|PENDIN|EXTPROC);
#else
    tp.c_lflag &= ~(ICANON|ISIG|IEXTEN|TOSTOP|FLUSHO|PENDIN);
#endif	/* EXTPROC */
#ifdef NOKERNINFO
    tp.c_lflag |= NOKERNINFO;
#endif	/* NOKERNINFO */
#ifndef COMMENT
    tp.c_lflag &= ~NOFLSH;		/* TRY IT THE OTHER WAY? */
#else
    tp.c_lflag |= NOFLSH;		/* No, this way is worse */
#endif /* COMMENT */

    /* cflags */
    tp.c_cflag &= ~(CSIZE|PARENB|PARODD);
    tp.c_cflag |= CS8|CREAD;

#ifdef MDMBUF
    tp.c_cflag &= ~(MDMBUF);
#else
#ifdef CCAR_OFLOW
    tp.c_cflag &= ~(CCAR_OFLOW);	/* two names for the same thing */
#endif /* CCAR_OFLOW */
#endif /* MDMBUF */

#ifdef CCTS_OFLOW
    tp.c_cflag &= ~(CCTS_OFLOW);
#endif /* CCTS_OFLOW */
#ifdef CDSR_OFLOW
    tp.c_cflag &= ~(CDSR_OFLOW);
#endif /* CDSR_OFLOW */
#ifdef CDTR_IFLOW
    tp.c_cflag &= ~(CDTR_IFLOW);
#endif /* CDTR_IFLOW */
#ifdef CRTS_IFLOW
    tp.c_cflag &= ~(CRTS_IFLOW);
#endif /* CRTS_IFLOW */
#ifdef CRTSXOFF
    tp.c_cflag &= ~(CRTSXOFF);
#endif /* CRTSXOFF */
#ifdef CRTSCTS
    tp.c_cflag &= ~(CRTSCTS);
#endif /* CRTSCTS */
#ifdef CLOCAL
    tp.c_cflag &= ~(CLOCAL);
#endif /* CLOCAL */
#ifdef CSTOPB
    tp.c_cflag &= ~(CSTOPB);
#endif /* CSTOPB */
#ifdef HUPCL
    tp.c_cflag &= ~(HUPCL);
#endif /* HUPCL */

    for (i = 0; i < NCCS; i++) {	/* No special characters */
	tp.c_cc[i] = 0;
    }
#ifdef VMIN
    tp.c_cc[VMIN] = 1;			/* But always wait for input */
#endif	/* VMIN */
    debug(F101,"pty_make_raw 3 c_cc[] NCCS","",NCCS);
    debug(F101,"pty_make_raw 3 iflags","",tp.c_iflag);
    debug(F101,"pty_make_raw 3 oflags","",tp.c_oflag);
    debug(F101,"pty_make_raw 3 lflags","",tp.c_lflag);
    debug(F101,"pty_make_raw 3 cflags","",tp.c_cflag);
#endif /* COMMENT */
#endif /* COMMENT */

    errno = 0;
#ifdef BSD44ORPOSIX			/* POSIX */
    x = tcsetattr(fd,TCSANOW,&tp);
    debug(F101,"pty_make_raw tcsetattr","",x);
#else
#ifdef ATTSV				/* AT&T UNIX */
    x = ioctl(fd,TCSETA,&tp);
    debug(F101,"pty_make_raw ioctl","",x);
#else
    x = stty(fd,&tp);			/* Traditional */
    debug(F101,"pty_make_raw stty","",x);
#endif /* ATTSV */
#endif /* BSD44ORPOSIX */
    debug(F101,"pty_make_raw errno","",errno);

#endif /* __NetBSD__ */
}

static int
pty_chk(fd) int fd; {
    int x, n = 0;
    errno = 0;
#ifdef FIONREAD
    x = ioctl(fd, FIONREAD, &n);	/* BSD and most others */
    ckmakmsg(msgbuf,500,
	     "pty_chk ioctl FIONREAD errno=",
	     ckitoa(errno),
	     " count=",
	     ckitoa(n));
    debug(F100,msgbuf,"",0);
#else
#ifdef RDCHK
    n = rdchk(fd);
    debug(F101,"pty_chk rdchk","",n);
#else
    n = 1;
#endif	/* RDCHK */
#endif	/* FIONREAD */
    return((n > -1) ? n : 0);
}

static int
pty_get_status(fd,pid) int fd; PID_T pid; {
    int x, status = -1;
    PID_T w;

    debug(F101,"pty_get_status fd","",fd);
    debug(F101,"pty_get_status pid","",pid);

    if (pexitstat > -1)
      return(pexitstat);

#ifdef COMMENT
    /* Not only unnecessary but harmful */
    errno = 0;
    x = kill(pty_fork_pid,0);
    debug(F101,"pty_get_status kill value","",x);
    debug(F101,"pty_get_status kill errno","",errno);
    if (x > -1 && errno != ESRCH)
      return(-1);			/* Fork still there */
    /* Fork seems to be gone */
#endif	/* COMMENT */

    errno = 0;
    x = waitpid(pty_fork_pid,&status,WNOHANG);
    debug(F111,"pty_get_status waitpid",ckitoa(errno),x);
    if (x <= 0 && errno == 0) {
	debug(F101,"pty_get_status waitpid return","",-1);
	return(-1);
    }
    if (x > 0) {
	if (x != pty_fork_pid)
	  debug(F101,
		"pty_get_status waitpid pid doesn't match","",pty_fork_pid); 
	debug(F101,"pty_get_status waitpid status","",status);
	debug(F101,"pty_get_status waitpid errno","",errno);
	if (WIFEXITED(status)) {
	    debug(F100,"pty_get_status WIFEXITED","",0);
	    status = WEXITSTATUS(status);
	    debug(F101,"pty_get_status fork exit status","",status);
#ifdef COMMENT
	    end_pty();
#endif	/* COMMENT */
	    close(fd);
	    pexitstat = status;
	} else {
	    debug(F100,"pty_get_status waitpid unexpected status","",0);
	}
    }
    debug(F101,"pty_get_status return status","",status);
    return(status);
}

/* t t p t y c m d  --  Run command on pty and forward to net */

/*
  Needed for running external protocols on secure connections.
  For example, if C-Kermit has made an SSL/TLS or Kerberos Telnet
  connection, and then needs to transfer a file with Zmodem, which is
  an external program, this routine reads Zmodem's output, encrypts it,
  and then forwards it out the connection, and reads the encrypted data
  stream coming in from the connection, decrypts it, and forwards it to
  Zmodem.

  Works like a TCP/IP port forwarder except one end is a pty rather
  than a socket, which introduces some complications:

   . On most platforms, select() always indicates the output side of
     the pty has characters waiting to be read, even when it doesn't,
     even when the pty process has already exited.

   . Nonblocking reads must be used on the pty, because there is no
     way on certain platforms (e.g. NetBSD) to find out how many characters
     are available to be read (the FIONREAD ioctl always says 0).  The code
     also allows for blocking reads (if O_NDELAY and O_NONBLOCK are not
     defined, or if PTY_NO_NDELAY is defined), but on some platforms this can
     result in single-byte reads and writes (NetBSD again).

   . Testing for "EOF" on the pty is problematic.  select() never gives
     any indication.  After the pty process has exited and the fork has
     disappeared, read() can still return with 0 bytes read but without an
     error (NetBSD); no known test on the pty file descriptor will indicate
     that it is no longer valid.  The process ID of the pty fork can be
     tested on some platforms (NetBSD, luckily) but not others (Solaris,
     Linux).

  On the network side, we use ttinc() and ttoc(), which, for network 
  connections, handle any active security methods.

  Call with s = command.
  Returns 0 on failure, 1 on success.
  fdc - December 2006 - August 2007.

  NOTE: This code defaults to nonblocking reads if O_NDELAY or O_NONBLOCK are
  defined in the header files, which should be true of every recent Unix
  platform.  If this causes trouble somewhere, define PTY_NO_NDELAY, e.g. when
  building C-Kermit:

    touch ckutio.c
    make platformname KFLAGS=-DPTY_NO_NODELAY
*/
static int have_pty = 0;		/* Do we have a pty? */

static SIGTYP (*save_sigchld)() = NULL;	/* For catching SIGCHLD */

static VOID
sigchld_handler(sig) int sig; {
    have_pty = 0;			/* We don't have a pty */
#ifdef DEBUG
    if (save_sigchld) {
	(VOID) signal(SIGCHLD,save_sigchld);
	save_sigchld = NULL;
    }
    if (deblog) {
	debug(F100,"**************","",0);
	debug(F100,"SIGCHLD caught","",0);
	debug(F100,"**************","",0);
    }
#endif	/* DEBUG */
}
#define HAVE_IAC 1
#define HAVE_CR  2

int
ttptycmd(s) char *s; {
    CHAR tbuf[PTY_TBUF_SIZE];		/* Read from net, write to pty */
    int tbuf_avail = 0;			/* Pointers for tbuf */
    int tbuf_written = 0;
    static int in_state = 0;		/* For TELNET IAC and NVT in */
    static int out_prev = 0;		/* Simpler scheme for out */

    CHAR pbuf[PTY_PBUF_SIZE];		/* Read from pty, write to net */
    CHAR dbuf[PTY_PBUF_SIZE + PTY_PBUF_SIZE + 1]; /* Double-size buffer */
    int pbuf_avail = 0;			/* Pointers for pbuf */
    int pbuf_written = 0;

    int ptyfd = -1;			/* Pty file descriptor */
    int have_net = 0;			/* We have a network connection */
    int pty_err = 0;			/* Got error on pty */
    int net_err = 0;			/* Got error on net */
    int status = -1;			/* Pty process exit status */
    int rc = 0;				/* Our return code */

    int x1 = 0, x2 = 0;			/* Workers... */
    int c, n, m, t, x;			/* Workers */

    long seconds_to_wait = 0L;		/* select() timeout */
    struct timeval tv, *tv2;		/* For select() */
#ifdef INTSELECT
    int in, out, err;			/* For select() */
#else
    fd_set in, out, err;
#endif /* INTSELECT */
    int nfds = 0;			/* For select() */

    int pset = 0, tset = 0, pnotset = 0, tnotset = 0; /* stats/debuggin only */
    int read_net_bytes = 0;		/* Stats */
    int write_net_bytes = 0;		/* Stats */
    int read_pty_bytes = 0;		/* Stats */
    int write_pty_bytes = 0;		/* Stats */
    int is_tn = 0;			/* TELNET protocol is active */

    int masterfd = -1;
    int slavefd = -1;
#ifndef USE_CKUPTY_C
    struct termios term;
    struct winsize twin;
    struct stringarray * q;
    char ** args = NULL;
#endif /* USE_CKUPTY_C */

    in_state = 0;			/* No previous character yet */

    if (ttyfd == -1) {
	printf("?Sorry, communication channel is not open\n");
	return(0);
    } else {
	have_net = 1;
    }
    if (nopush) {
	debug(F100,"ttptycmd fail: nopush","",0);
	return(0);
    }
    if (!s) s = "";			/* Defense de bogus arguments */
    if (!*s) return(0);
    pexitstat = -1;			/* Fork process exit status */

#ifdef TNCODE
    is_tn = (xlocal && netconn && IS_TELNET()) || /* Telnet protocol active */
	    (!xlocal && sstelnet);
#endif /* TNCODE */

    debug(F110,"ttptycmd command",s,0);
    debug(F101,"ttptycmd ttyfd","",ttyfd);
    debug(F101,"ttptycmd is_tn","",is_tn);
    debug(F101,"ttptycmd ckermit pid","",getpid());

#ifdef USE_CKUPTY_C
    /* Call ckupty.c module to get and set up the pty fork */
    /* fc 1 == "run an external protocol" */
    debug(F100,"ttptycmd using ckupty.c","",0);
    if (do_pty(&ptyfd,s,1) < 0) {	/* Start the command on a pty */
	debug(F100,"ttptycmd do_pty fails","",0);
	return(0);
    }
    masterfd = ptyfd;
    pty_master_fd = ptyfd;
#ifdef COMMENT
    slavefd = pty_slave_fd;		/* This is not visible to us */
#endif /* COMMENT */
    debug(F111,"ttptycmd ptyfd","USE_CKUPTY_C",ptyfd);
    debug(F111,"ttptycmd masterfd","USE_CKUPTY_C",masterfd);
    debug(F111,"ttptycmd fork pid","USE_CKUPTY_C",pty_fork_pid);
#ifndef SOLARIS
    /* "ioctl inappropriate on device" for pty master */
    pty_make_raw(masterfd);
#endif /* SOLARIS */

#else /* USE_CKUPTY_C */

    debug(F100,"ttptycmd OPENPTY","",0);
    if (tcgetattr(0, &term) == -1) {	/* Get controlling terminal's modes */
	perror("tcgetattr");
	return(0);
    }
    if (ioctl(0, TIOCGWINSZ, (char *) &twin) == -1) { /* and window size */
	perror("ioctl TIOCGWINSZ");
	return(0);
    }
    if (openpty(&masterfd, &slavefd, NULL, NULL, NULL) == -1) {
	debug(F101,"ttptycmd openpty failed errno","",errno);
	perror("opentpy");
	return(0);
    }
    debug(F101,"ttptycmd openpty masterfd","",masterfd);
    debug(F101,"ttptycmd openpty slavefd","",slavefd);
    pty_master_fd = masterfd;
    pty_slave_fd = slavefd;
    debug(F101,"ttptycmd openpty pty_master_fd","",pty_master_fd);

    /* Put pty master in raw mode but let forked app control the slave */
    pty_make_raw(masterfd);

#ifdef COMMENT
#ifdef TIOCREMOTE
    /* TIOCREMOTE,0 = disable all termio processing */
    x = ioctl(masterfd, TIOCREMOTE, 1);
    debug(F111,"ttptycmd ioctl TIOCREMOTE",ckitoa(x),errno);
#endif	/* TIOCREMOTE */
#ifdef TIOCTTY
    /* TIOCTTY,0 = disable all termio processing */
    x = ioctl(masterfd, TIOCTTY, 0);
    debug(F111,"ttptycmd ioctl TIOCTTY",ckitoa(x),errno);
#endif	/* TIOCTTY */
#endif /* COMMENT */

    have_pty = 1;			/* We have an open pty */
    save_sigchld = signal(SIGCHLD, sigchld_handler); /* Catch fork quit */

    pty_fork_pid = fork();		/* Make fork for external protocol */
    debug(F101,"ttptycmd pty_fork_pid","",pty_fork_pid);
    if (pty_fork_pid == -1) {
	perror("fork");
	return(0);
    } else if (pty_fork_pid == 0) {	/* In new fork */
	int x;
	debug(F101,"ttptycmd new fork pid","",getpid());
	close(masterfd);		/* Slave quarters no masters allowed */
	x = setsid();
	debug(F101,"ttptycmd new fork setsid","",x);
	if (x == -1) {
	    perror("ttptycmd setsid");
	    exit(1);
	}
	signal(SIGINT,SIG_IGN);		/* Let upper fork catch this */
	
#ifdef COMMENT
#ifdef TIOCSCTTY
	/* Make pty the controlling terminal for the process */
	/* THIS CAUSES AN INFINITE SIGWINCH INTERRUPT LOOP */
	x = ioctl(slavefd, TIOCSCTTY, NULL);
	debug(F101,"ttptycmd TIOCSCTTY","",x);
#endif	/* TIOCSCTTY */
#endif	/* COMMENT */

	/* Initialize slave pty modes and size to those of our terminal */
	if (tcsetattr(slavefd, TCSANOW, &term) == -1) {
	    perror("ttptycmd tcsetattr");
	    exit(1);
	}
	if (ioctl(slavefd, TIOCSWINSZ, &twin) == -1) {
	    perror("ttptycmd ioctl");
	    exit(1);
	}
#ifdef COMMENT
#ifdef TIOCNOTTY
	/* Disassociate this process from its terminal */
	/* THIS HAS NO EFFECT */
	x = ioctl(slavefd, TIOCNOTTY, NULL);
	debug(F101,"ttptycmd TIOCNOTTY","",x);
#endif	/* TIOCNOTTY */
#endif	/* COMMENT */

#ifdef COMMENT
#ifdef SIGTTOU	
	/* Ignore terminal output interrupts */
	/* THIS HAS NO EFFECT */
	debug(F100,"ttptycmd ignoring SIGTTOU","",0);
	signal(SIGTTOU, SIG_IGN);
#endif	/* SIGTTOU */
#ifdef SIGTSTP	
	/* Ignore terminal output interrupts */
	/* THIS HAS NO EFFECT */
	debug(F100,"ttptycmd ignoring SIGTSTP","",0);
	signal(SIGTSTP, SIG_IGN);
#endif	/* SIGTSTP */
#endif	/* COMMENT */

	pty_make_raw(slavefd);		/* Put it in rawmode */

	errno = 0;
	if (dup2(slavefd, STDIN_FILENO) != STDIN_FILENO ||
	    dup2(slavefd, STDOUT_FILENO) != STDOUT_FILENO) {
	    debug(F101,"ttptycmd new fork dup2 error","",errno);
	    perror("ttptycmd dup2");
	    exit(1);
	}
	debug(F100,"ttptycmd new fork dup2 ok","",0);

	/* Parse external protocol command line */
	q = cksplit(1,0,s,NULL,"\\%[]&$+-/=*^_@!{}/<>|.#~'`:;?",7,0,0);
	if (!q) {
	    debug(F100,"ttptycmd cksplit failed","",0);
	    exit(1);
	} else {
	    int i, n;
	    debug(F100,"ttptycmd cksplit ok","",0);
	    n = q->a_size;
	    args = q->a_head + 1;
	    for (i = 0; i <= n; i++) {
		if (!args[i]) {
		    break;
		} else {
		    /* sometimes cksplit() doesn't terminate the list */
		    if ((i == n) && args[i]) {
			if ((int)strlen(args[i]) == 0)
			  makestr(&(args[i]),NULL);
		    }
		}
	    }	    
	}
#ifdef COMMENT
/*
  Putting the slave pty in rawmode should not be necessary because the
  external protocol program is supposed to do that itself.  Yet doing this
  here cuts down on Zmodem binary-file transmission errors by 30-50% but
  still doesn't eliminate them.
*/
	pty_make_raw(STDIN_FILENO);
	pty_make_raw(STDOUT_FILENO);
#endif /* COMMENT */

	debug(F100,"ttptycmd execvp'ing external protocol","",0);
	execvp(args[0],args);
	perror("execvp failed");
	debug(F101,"ttptycmd execvp failed","",errno);
	close(slavefd);
	exit(1);
    } 
    /* (there are better ways to do this...) */
    msleep(1000);		  /* Make parent wait for child to be ready */
    ptyfd = masterfd;			/* We talk to the master */

#endif /* USE_CKUPTY_C */

    debug(F101,"ttptycmd ptyfd","",ptyfd);
    if (ptyfd < 0) {
	printf("?Failure to get pty\n");
	return(-9);
    }
    have_pty = 1;	      /* We have an open pty or we wouldn't he here */

    debug(F101,"ttptycmd PTY_PBUF_SIZE","",PTY_PBUF_SIZE);
    debug(F101,"ttptycmd PTY_TBUF_SIZE","",PTY_TBUF_SIZE);

#ifdef PTY_USE_NDELAY
    /* 
       NOTE: If select() and ioctl(ptyfd,FIONREAD,&n) return true indications
       on the pty, we don't need nonblocking reads.  Performance of either
       method seems to be about the same, so use whatever works.
    */
    errno = 0;
    x = fcntl(ptyfd,F_SETFL,fcntl(ptyfd,F_GETFL, 0)|O_NDELAY);
    ckmakmsg(msgbuf,500,
	     "ttptycmd set O_NDELAY errno=",
	     ckitoa(errno),
	     " fcntl=",
	     ckitoa(x));
    debug(F100,msgbuf,"",0);
#endif /* PTY_USE_NDELAY */

#ifdef COMMENT
/* Not necessary, the protocol module already did this */

#ifdef USE_CFMAKERAW
    if (tcgetattr(ttyfd, &term) > -1) {
	cfmakeraw(&term);
	debug(F101,"ttptycmd net cfmakeraw errno","",errno);
	x tcsetattr(ttyfd, TCSANOW, &term);
	debug(F101,"ttptycmd net tcsetattr","",x);
	debug(F101,"ttptycmd net tcsetattr","",errno);
    }
#else
    if (local)				/* Put network connection in */
      ttpkt(ttspeed,ttflow,ttprty);	/* "packet mode". */
    else
      conbin((char)escchr);		/* OR... pty_make_raw(0) */
#endif /* USE_CFMAKERAW */
#endif /* COMMENT */

#ifdef TNCODE
    if (is_tn) {
      debug(F101,"<<< ttptycmd TELOPT_ME_BINARY","",TELOPT_ME(TELOPT_BINARY));
      debug(F101,"<<< ttptycmd TELOPT_U_BINARY","",TELOPT_U(TELOPT_BINARY));
    }
#endif /* TNCODE */

    debug(F101,"ttptycmd entering loop - seconds_to_wait","",seconds_to_wait);

    while (have_pty || have_net) {
	FD_ZERO(&in);			/* Initialize select() structs */
	FD_ZERO(&out);
	FD_ZERO(&err);			/* (not used because useless) */
	nfds = -1;

	debug(F101,"ttptycmd loop top have_pty","",have_pty);
	debug(F101,"ttptycmd loop top have_net","",have_net);

	/* Pty is open and we have room to read from it? */
	if (have_pty && pbuf_avail < PTY_PBUF_SIZE) {
	    debug(F100,"ttptycmd FD_SET ptyfd in","",0);
            FD_SET(ptyfd, &in);
	    nfds = ptyfd;
        }
	/* Network is open and we have room to read from it? */
        if (have_net && have_pty && tbuf_avail < PTY_TBUF_SIZE) {
	    debug(F100,"ttptycmd FD_SET ttyfd in","",0);
            FD_SET(ttyfd, &in);
	    if (ttyfd > nfds) nfds = ttyfd;
        }
	/* Pty is open and we have stuff to write to it? */
        if (have_pty && tbuf_avail - tbuf_written > 0) {
	    debug(F100,"ttptycmd FD_SET ptyfd out","",0);
            FD_SET (ptyfd, &out);
	    if (ptyfd > nfds) nfds = ptyfd;
        }
	/* Net is open and we have stuff to write to it? */
	debug(F101,"ttptycmd pbuf_avail-pbuf_written","",
	      pbuf_avail - pbuf_written);
        if (have_net && pbuf_avail - pbuf_written > 0) {
	    debug(F100,"ttptycmd FD_SET ttyfd out","",0);
            FD_SET (ttyfd, &out);
	    if (ttyfd > nfds) nfds = ttyfd;
        }
	/* We don't use err because it's not really for errors, */
	/* but for out of band data on the TCP socket, which, if it is */
	/* to be handled at all, is handled in the tt*() routines */

	nfds++;				/* 0-based to 1-based */
	debug(F101,"ttptycmd nfds","",nfds);
	if (!nfds) {
	    debug(F100,"ttptycmd NO FDs set for select","",0);
	    if (have_pty) {
		/* This is not right -- sleeping won't accomplish anything */
		debug(F101,"ttptycmd msleep","",100);
		msleep(100);	    
	    } else {
		debug(F100,"ttptycmd no pty - quitting loop","",0);
		break;
	    }
	}
	errno = 0;

	if (seconds_to_wait > 0L) {	/* Timeout in case nothing happens */
	    tv.tv_sec = seconds_to_wait; /* for a long time */
	    tv.tv_usec = 0L;		
	    tv2 = &tv;
        } else {
            tv2 = NULL;
	}
	x = select(nfds, &in, &out, NULL, tv2);
	debug(F101,"ttptycmd select","",x);
	if (x < 0) {
	    if (errno == EINTR)
	      continue;
	    debug(F101,"ttptycmd select error","",errno);
	    break;
	}
	if (x == 0) {
	    debug(F101,"ttptycmd +++ select timeout","",seconds_to_wait); 
	    if (have_pty) {
		status = pty_get_status(ptyfd,pty_fork_pid);
		debug(F101,"ttptycmd pty_get_status A","",status);
		if (status > -1) pexitstat = status;
		have_pty = 0;
	    }
	    break;
	}
	/* We want to handle any pending writes first to make room */
	/* for new incoming. */

	if (FD_ISSET(ttyfd, &out)) {	/* Can write to net? */
	    CHAR * s;
	    s = pbuf + pbuf_written;	/* Current spot for sending */
#ifdef TNCODE
	    if (is_tn) {		/* ttol() doesn't double IACs */
		CHAR c;			/* Rewrite string with IACs doubled */
		int i;
		s = pbuf + pbuf_written; /* Source */
		x = 0;		 	 /* Count */
		for (i = 0; i < pbuf_avail - pbuf_written; i++) {
		    c = s[i];		/* Next character */
		    if (c == IAC) {	/* If it's IAC */
			dbuf[x++] = c;	/* put another one */
			debug(F000,">>> QUOTED IAC","",c);
		    } else if (c != 0x0a && out_prev == 0x0d) {	/* Bare CR */
			if (!TELOPT_ME(TELOPT_BINARY)) { /* NVT rule */
			    c = 0x00;
			    dbuf[x++] = c;
			    debug(F000,">>> CR-NUL","",c);
			}			
		    }
		    dbuf[x++] = c;	/* Copy and count it */
		    debug(F000,">>> char",ckitoa(in_state),c);
		    out_prev = c;
		}
		s = dbuf;		/* New source */
	    } else
#endif /* TNCODE */
	      x = pbuf_avail - pbuf_written; /* How much to send */

	    debug(F101,"ttptycmd bytes to send","",x);
	    x = ttol(s, x);
	    debug(F101,">>> ttol","",x);
	    if (x < 0) {
		net_err++;
		debug(F111,"ttptycmd ttol error",ckitoa(x),errno);
		x = 0;
	    }
	    write_net_bytes += x;
	    pbuf_written += x;
	}
	if (FD_ISSET(ptyfd, &out)) {	/* Can write to pty? */
	    debug(F100,"ttptycmd FD_ISSET ptyfd out","",0);
	    errno = 0;
#ifndef COMMENT
	    x = write(ptyfd,tbuf + tbuf_written,tbuf_avail - tbuf_written);
#else
	    /* Byte loop to rule out data overruns in the pty */
	    /* (it makes no difference) */
	    {
		char *p = tbuf+tbuf_written;
		int n = tbuf_avail - tbuf_written;
		for (x = 0; x < n; x++) {
		    msleep(10);
		    if (write(ptyfd,&(p[x]),1) < 0)
		      break;
		}
	    }
#endif /* COMMENT */
	    debug(F111,"ttptycmd ptyfd write",ckitoa(errno),x);
	    if (x > 0) {
		tbuf_written += x;
		write_pty_bytes += x;
	    } else {
		x = 0;
		pty_err++;
		if (pexitstat < 0) {
		    status = pty_get_status(ptyfd,pty_fork_pid);
		    debug(F101,"ttptycmd pty_get_status B","",status);
		    if (status > -1) pexitstat = status;
		    have_pty = 0;
		}
		debug(F100,"ttptycmd +++ ptyfd write error","",0);
	    }
	}
	if (FD_ISSET(ttyfd, &in)) {	/* Can read from net? */
	    tset++;
	    debug(F100,"ttptycmd FD_ISSET ttyfd in","",0);
	    n = in_chk(1,ttyfd);
	    debug(F101,"ttptycmd in_chk(ttyfd)","",n); 
	    if (n < 0 || ttyfd == -1) {
		debug(F101,"ttptycmd +++ ttyfd errno","",errno);
		net_err++;
	    } else if (n > 0) {
		if (n > PTY_TBUF_SIZE - tbuf_avail)
		  n = PTY_TBUF_SIZE - tbuf_avail;
		debug(F101,"ttptycmd net read size adjusted","",n); 
		if (xlocal && netconn) {
		    /*
		      We have to use a byte loop here because ttxin()
		      does not decrypt or, for that matter, handle Telnet.
		    */
		    int c;
		    CHAR * p;
		    p = tbuf + tbuf_avail;
		    for (x = 0; x < n; x++) {
			if ((c = ttinc(0)) < 0)
			  break;
			if (!is_tn) {	/* Not Telnet - keep all bytes */
			    *p++ = (CHAR)c;
			    debug(F000,"<<< char","",c);
#ifdef TNCODE
			} else {	/* Telnet - must handle IAC and NVT */
			    debug(F000,"<<< char",ckitoa(in_state),c);
			    switch (c) {
			      case 0x00: /* NUL */
				if (in_state == HAVE_CR) {
				    debug(F000,"<<< SKIP","",c);
				} else {
				    *p++ = c;
				    debug(F000,"<<< Keep","",c);
				}
				in_state = 0;
				break;
			      case 0x0d: /* CR */
				if (!TELOPT_U(TELOPT_BINARY))
				  in_state = HAVE_CR;
				*p++ = c;
				debug(F000,"<<< Keep","",c);
				break;
#ifdef COMMENT
			      case 0x0f: /* Ctrl-O */
			      case 0x16: /* Ctrl-V */
				*p++ = 0x16;
				*p++ = c;
				debug(F000,"<<< QUOT","",c);
				break;
#endif /* COMMENT */
			      case 0xff: /* IAC */
				if (in_state == HAVE_IAC) {
				    debug(F000,"<<< KEEP","",c);
				    *p++ = c;
				    in_state = 0;
				} else {
				    debug(F000,"<<< SKIP","",c);
				    in_state = HAVE_IAC;
				}
				break;
			      default:	/* All others */
				if (in_state == HAVE_IAC) {
#ifdef COMMENT
/*
  tn_doop() will consume an unknown number of bytes and we'll overshoot
  the for-loop.  The only Telnet command I've ever seen arrive here is
  a Data Mark, which comes when the remote protocol exits and the remote
  job returns to its shell prompt.  On the assumption it's a 1-byte command,
  we don't write out the IAC or the command, and we clear the state.  If
  we called tn_doop() we'd have no way of knowing how many bytes it took
  from the input stream.
*/
				    int xx;
				    xx = tn_doop((CHAR)c,duplex,ttinc);
				    debug(F111,"<<< DOOP",ckctoa(c),xx);
#else
				    debug(F101,"<<< DOOP","",c);
#endif	/* COMMENT */
				    in_state = 0;
				} else {
				    *p++ = c;
				    debug(F000,"<<< keep","",c);
				    in_state = 0;
				}
			    }
#endif	/* TNCODE */
			}
		    }
		    ckmakmsg(msgbuf,500,
			     "ttptycmd read net [ttinc loop] errno=",
			     ckitoa(errno),
			     " count=",
			     ckitoa(x));
		    debug(F100,msgbuf,"",0);
		} else {
		    x = ttxin(n,tbuf+tbuf_avail);
		    debug(F101,"ttptycmd ttxin x","",x); 
		}

		if (x < 0) {
		    debug(F101,"ttptycmd read net error","",x);
		    net_err++;
		}
		tbuf_avail += x;
		read_net_bytes += x;
	    }

	} else
	  tnotset++;

	if (FD_ISSET(ptyfd, &in)) {	/* Read from pty? */
	    pset++;
	    debug(F100,"ttptycmd FD_ISSET ptyfd in","",0);
#ifdef PTY_USE_NDELAY
	    n = PTY_PBUF_SIZE;
#else
	    /*
	      This does not work on nonblocking channels
	      on certain platforms such as NetBSD.
	    */
	    n = pty_chk(ptyfd);
#endif /* PTY_USE_NDELAY */
	    debug(F101,"ttptycmd pty_chk() n","",n); 

	    if (n < 0)
	      n = 0;
	    if (n > 0) {
		if (n > PTY_PBUF_SIZE - pbuf_avail)
		  n = PTY_PBUF_SIZE - pbuf_avail;
		debug(F101,"ttptycmd pty read size adjusted","",n); 
		errno = 0;
		x = read(ptyfd,pbuf+pbuf_avail,n);
#ifdef DEBUG
		if (deblog) {
		    ckmakmsg(msgbuf,500,
			     "ttptycmd read pty errno=",
			     ckitoa(errno),
			     " count=",
			     ckitoa(x));
		    debug(F100,msgbuf,"",0);
		}
#endif	/* DEBUG */

		if (x < 0 && errno == EAGAIN)
		  x = 0;

		if (x < 0) {		/* This works on Solaris and Linux */
		    pty_err++;		/* but not NetBSD */
		    debug(F100,"TERMINATION TEST A","",0);
#ifdef COMMENT
		    if (errno == EIO)
		      rc = 1;
#endif	/* COMMENT */
		    if (pexitstat < 0) {
			status = pty_get_status(ptyfd,pty_fork_pid);
			debug(F101,"ttptycmd pty_get_status C","",status);
			if (status > -1) pexitstat = status;
		    }
		    have_pty = 0;
		    x = 0;
		}
		if (x == 0 && !pty_err) { /* This works on NetBSD but */
		    debug(F100,"TERMINATION TEST B","",0);
		    status = pexitstat > -1 ? pexitstat :
			pty_get_status(ptyfd,pty_fork_pid);
		    debug(F101,"ttptycmd pty_get_status D","",status);
		    if (status > -1) {
			pexitstat = status;
			pty_err++;
			have_pty = 0;
		    } else {		/* Select() lied */
			pty_err = 0;	/* pty still there but has nothing */
			msleep(100);	/* sleep a bit */
		    }
		    x = 0;
		} 
		/* Hopefully the next two are no longer needed... */
		if (!pty_err && (
#ifndef PTY_USE_NDELAY
		    x < 1 || errno
#else
		    errno != 0 && errno != EAGAIN
#endif /* PTY_USE_NDELAY */
		    )) {
		    debug(F100,"TERMINATION TEST C","",0);
		    pty_err++;
		    debug(F101,"ttptycmd SET pty_err","",pty_err);
		    if (errno == EIO)	/* errno == EIO is like EOF */
		      rc = 1;
		    if (x < 0)
		      x = 0;
		}
#ifdef COMMENT
#ifdef DEBUG
		if (deblog) {
		    pbuf[pbuf_avail + x] = '\0';
		    debug(F111,"ttptycmd added to pty buffer",
			  pbuf+pbuf_avail,x);
		}
#endif	/* DEBUG */
#endif	/* COMMENT */
		pbuf_avail += x;
		read_pty_bytes += x;
	    } else {			/* n == 0 with blocking reads */
		debug(F100,
		      "PTY READ RETURNED ZERO BYTES - SHOULD NOT HAPPEN",
		      "",0);
	    }
	} else
	  pnotset++;

	/* If writes have caught up to reads, reset the buffers */

	if (pbuf_written == pbuf_avail)
	  pbuf_written = pbuf_avail = 0;
	if (tbuf_written == tbuf_avail)
	  tbuf_written = tbuf_avail = 0;

	/* See if we can exit */

	x1 = pbuf_avail - pbuf_written; 
	x2 = tbuf_avail - tbuf_written;

	debug(F101,"ttptycmd pty_err LOOP EXIT TEST pty_err","",pty_err);
	debug(F101,"ttptycmd pty_err LOOP EXIT TEST x1 [write to net]","",x1);
	debug(F101,"ttptycmd pty_err LOOP EXIT TEST x2 [write to pty]","",x2);
	debug(F101,"ttptycmd pty_err LOOP EXIT TEST rc","",rc);
	debug(F101,"ttptycmd pty_err LOOP EXIT TEST status","",status);
	debug(F101,"ttptycmd pty_err LOOP EXIT TEST pexitstat","",pexitstat);

	if (net_err) {			/* Net error? */
	    debug(F101,"ttptycmd net_err LOOP EXIT TEST net_err","",net_err);
	    if (have_net) {
		if (local) {
		    ttclos(0);
		    printf("?Connection closed\n");
		}
		have_net = 0;
	    }
	    debug(F101,"ttptycmd net_err LOOP EXIT TEST x1","",x1);
	    if (x1 == 0)
	      break;
	}
	if (pty_err) {			/* Pty error? */
	    if (have_pty) {
		if (pexitstat < 0) {		
		    status = pty_get_status(ptyfd,pty_fork_pid);
		    debug(F101,"ttptycmd pty_get_status E","",status);
		    if (status > -1) pexitstat = status;
		}
		have_pty = 0;
	    }
	    if (x1 == 0 && x2 == 0) {	/* If buffers are caught up */
		rc = 1;			/* set preliminary return to success */
		debug(F101,"ttptycmd pty_err LOOP EXIT TEST rc 2","",rc);
		break;			/* and exit the loop */
	    }
	}
    }
    debug(F101,"ttptycmd +++ have_pty","",have_pty);
    if (have_pty) {			/* In case select() failed */
#ifdef USE_CKUPTY_C
	end_pty();
	close(ptyfd);
#else
	close(slavefd);
	close(masterfd);
#endif /* USE_CKUPTY_C */
    }
    pty_master_fd = -1;
    debug(F101,"ttptycmd +++ pexitstat","",pexitstat);
    if (pexitstat < 0) {		/* Try one last time to get status */
	status = pty_get_status(ptyfd,pty_fork_pid);
	debug(F101,"ttptycmd pty_get_status F","",status);
	if (status > -1) pexitstat = status;
    }
    debug(F101,"ttptycmd +++ final pexitstat","",pexitstat);
    if (deblog) {			/* Stats for debug log */
	debug(F101,"ttptycmd +++ pset	","",pset);
	debug(F101,"ttptycmd +++ pnotset","",pnotset);
	debug(F101,"ttptycmd +++ tset	","",tset);
	debug(F101,"ttptycmd +++ tnotset","",tnotset);

	debug(F101,"ttptycmd +++  read_pty_bytes","",read_pty_bytes);
	debug(F101,"ttptycmd +++ write_net_bytes","",write_net_bytes);
	debug(F101,"ttptycmd +++  read_net_bytes","",read_net_bytes);
	debug(F101,"ttptycmd +++ write_pty_bytes","",write_pty_bytes);
    }
/*
  If we got the external protocol's exit status from waitpid(), we use that
  to set our return code.  If not, we fall back on whatever rc was previously
  set to, namely 1 (success) if the pty fork seemed to terminate, 0 otherwise.
*/
    if (save_sigchld) {			/* Restore this if we changed it */
	(VOID) signal(SIGCHLD,save_sigchld);
	save_sigchld = NULL;
    }
    msleep(500);
    x = kill(pty_fork_pid,SIGHUP);	/* In case it's still there */
    pty_fork_pid = -1;
    debug(F101,"ttptycmd fork kill SIGHUP","",x);
    if (pexitstat > -1)
      rc = (pexitstat == 0 ? 1 : 0);
    debug(F101,"ttptycmd +++ rc","",rc);
    if (!local) {			/* If in remote mode */
	conres();			/* restore console to CBREAK mode */
	concb((char)escchr);
    }
    return(rc);
}
#endif	/* NETPTY */
#endif	/* SELECT */

/* T T R U N C M D  --  Redirect an external command over the connection. */

/*
  TTRUNCMD is the routine that was originally used for running external
  protocols.  It is very simple and works fine provided (a) the connection
  is not encrypted, and (b) the external protocol uses standard i/o
  (file descriptors 0 and 1) for file transfer.
*/

int
ttruncmd(s) char *s; {
    PID_T pid;				/* pid of lower fork */
    int wstat;				/* for wait() */
    int x;
    int statusp;

    if (ttyfd == -1) {
	printf("?Sorry, device is not open\n");
	return(0);
    }
    if (nopush) {
	debug(F100,"ttruncmd fail: nopush","",0);
	return(0);
    }

#ifdef NETPTY
/***************
  It might also be necessary to use the pty routine for other reasons,
  e.g. because the external program does not use stdio.
*/
#ifdef NETCONN
/*
  If we have a network connection we use a different routine because
  (a) if the connection is encrypted, the mechanism used here can't deal
  with it; and (b) it won't handle any network protocols either, e.g.
  Telnet, Rlogin, K5 U-to-U, etc.  However, this routine works much
  better (faster, more transparent) on serial connections and when
  C-Kermit is in remote mode (i.e. is on the far end).
*/
    /* For testing always use this */
    if (netconn)
      return(ttptycmd(s));
#endif /* NETCONN */

/***************/
#else  /* NETPTY */
    if (tt_is_secure()) {
	printf("?Sorry, \
external protocols over secure connections not supported in this OS.\n"
              );
        return(0);
    }
#endif	/* NETPTY */

    conres();				/* Make console normal  */
    pexitstat = -4;
    if ((pid = fork()) == 0) {		/* Make a child fork */
	if (priv_can())			/* Child: turn off privs. */
	  exit(1);
	dup2(ttyfd, 0);			/* Give stdin/out to the line */
	dup2(ttyfd, 1);
	x = system(s);
	debug(F101,"ttruncmd system",s,x);
	_exit(x ? BAD_EXIT : 0);
    } else {
	SIGTYP (*istat)(), (*qstat)();
	if (pid == (PID_T) -1)		/* fork() failed? */
	  return(0);
	istat = signal(SIGINT,SIG_IGN); /* Let the fork handle keyboard */
	qstat = signal(SIGQUIT,SIG_IGN); /* interrupts itself... */

#ifdef COMMENT
    	while (((wstat = wait(&statusp)) != pid) && (wstat != -1)) ;
#else  /* Not COMMENT */
    	while (1) {
	    wstat = wait(&statusp);
	    debug(F101,"ttruncmd wait","",wstat);
	    if (wstat == pid || wstat == -1)
	      break;
	}
#endif /* COMMENT */

	pexitstat = (statusp & 0xff) ? statusp : statusp >> 8;
	debug(F101,"ttruncmd wait statusp","",statusp);
	debug(F101,"ttruncmd wait pexitstat","",pexitstat);
	signal(SIGINT,istat);		/* Restore interrupts */
	signal(SIGQUIT,qstat);
    }
    concb((char)escchr);		/* Restore console to CBREAK mode */
    return(statusp == 0 ? 1 : 0);
}
#endif	/* CK_REDIR */

struct tm *
#ifdef CK_ANSIC
cmdate2tm(char * date, int gmt)         /* date as "yyyymmdd hh:mm:ss" */
#else
cmdate2tm(date,gmt) char * date; int gmt;
#endif
{
    /* date as "yyyymmdd hh:mm:ss" */
    static struct tm _tm;
    time_t now;

    if (strlen(date) != 17 ||
	date[8] != ' ' ||
	date[11] != ':' ||
	date[14] != ':')
      return(NULL);

    time(&now);
    if (gmt)
      _tm = *gmtime(&now);
    else
      _tm = *localtime(&now);
    _tm.tm_year = (date[0]-'0')*1000 + (date[1]-'0')*100 +
                  (date[2]-'0')*10   + (date[3]-'0')-1900;
    _tm.tm_mon  = (date[4]-'0')*10   + (date[5]-'0')-1;
    _tm.tm_mday = (date[6]-'0')*10   + (date[7]-'0');
    _tm.tm_hour = (date[9]-'0')*10   + (date[10]-'0');
    _tm.tm_min  = (date[12]-'0')*10  + (date[13]-'0');
    _tm.tm_sec  = (date[15]-'0')*10  + (date[16]-'0');

    /* Should we set _tm.tm_isdst to -1 here? */

    _tm.tm_wday = 0;
    _tm.tm_yday = 0;

    return(&_tm);
}

#ifdef OXOS
#undef kill
#endif /* OXOS */

#ifdef OXOS
int
priv_kill(pid, sig) int pid, sig; {
    int	i;

    if (priv_on())
	debug(F100,"priv_kill priv_on failed","",0);
    i = kill(pid, sig);
    if (priv_off())
	debug(F100,"priv_kill priv_off failed","",0);
    return(i);
}
#endif /* OXOS */

#ifdef BEOSORBEBOX
/* #ifdef BE_DR_7 */
/*
  alarm() function not supplied with Be OS DR7 - this one contributed by
  Neal P. Murphy.
*/

/*
  This should mimic the UNIX/POSIX alarm() function well enough, with the
  caveat that one's SIGALRM handler must call alarm_expired() to clean up vars
  and wait for the alarm thread to finish.
*/
unsigned int
alarm(unsigned int seconds) {
    long time_left = 0;

/* If an alarm is active, turn it off, saving the unused time */
    if (alarm_thread != -1) {
        /* We'll be generous and count partial seconds as whole seconds. */
        time_left = alarm_struct.time -
	  ((system_time() - time_started) / 1000000.0);

        /* Kill the alarm thread */
        kill_thread (alarm_thread);

        /* We need to clean up as though the alarm occured. */
        time_started = 0;
        alarm_struct.thread = -1;
        alarm_struct.time = 0;
        alarm_expired();
    }

/* Set a new alarm clock, if requested. */
    if (seconds > 0) {
        alarm_struct.thread = find_thread(NULL);
        alarm_struct.time = seconds;
        time_started = system_time();
        alarm_thread = spawn_thread (do_alarm,
                                     "alarm_thread",
                                     B_NORMAL_PRIORITY,
                                     (void *) &alarm_struct
				     );
        resume_thread (alarm_thread);
    }

/* Now return [unused time | 0] */
    return ((unsigned int) time_left);
}

/*
  This function is the departure from UNIX/POSIX alarm handling. In the case
  of Be's missing alarm() function, this stuff needs to be done in the SIGALRM
  handler. When Be implements alarm(), this function call can be eliminated
  from user's SIGALRM signal handlers.
*/

void
alarm_expired(void) {
    long ret_val;

    if (alarm_thread != -1) {
        wait_for_thread (alarm_thread, &ret_val);
        alarm_thread = -1;
    }
}

/*
  This is the function that snoozes the requisite number of seconds and then
  SIGALRMs the calling thread. Note that kill() wants a pid_t arg, whilst Be
  uses thread_id; currently they are both typdef'ed as long, but I'll do the
  cast anyway. This function is run in a separate thread.
*/

long
do_alarm (void *alarm_struct) {
    snooze ((double) ((struct ALARM_STRUCT *) alarm_struct)->time * 1000000.0);
    kill ((pid_t)((struct ALARM_STRUCT *) alarm_struct)->thread, SIGALRM);
    time_started = 0;
    ((struct ALARM_STRUCT *) alarm_struct)->thread = -1;
    ((struct ALARM_STRUCT *) alarm_struct)->time = 0;
}
/* #endif */ /* BE_DR_7 */
#endif /* BEOSORBEBOX */

#ifdef Plan9

int
p9ttyctl(char letter, int num, int param) {
    char cmd[20];
    int len;

    if (ttyctlfd < 0)
      return -1;

    cmd[0] = letter;
    if (num)
      len = sprintf(cmd + 1, "%d", param) + 1;
    else {
	cmd[1] = param;
	len = 2;
    }
    if (write(ttyctlfd, cmd, len) == len) {
	cmd[len] = 0;
	/* fprintf(stdout, "wrote '%s'\n", cmd); */
	return 0;
    }
    return -1;
}

int
p9ttyparity(char l) {
    return p9ttyctl('p', 0, l);
}

int
p9tthflow(int flow, int status) {
    return p9ttyctl('m', 1, status);
}

int
p9ttsspd(int cps) {
    if (p9ttyctl('b', 1, cps * 10) < 0)
      return -1;
    ttylastspeed = cps * 10;
    return 0;
}

int
p9openttyctl(char *ttname) {
    char name[100];

    if (ttyctlfd >= 0) {
	close(ttyctlfd);
	ttyctlfd = -1;
	ttylastspeed = -1;
    }
    sprintf(name, "%sctl", ttname);
    ttyctlfd = open(name, 1);
    return ttyctlfd;
}

int
p9concb() {
    if (consctlfd >= 0) {
	if (write(consctlfd, "rawon", 5) == 5)
	  return 0;
    }
    return -1;
}

int
p9conbin() {
    return p9concb();
}

int
p9conres() {
    if (consctlfd >= 0) {
	if (write(consctlfd, "rawoff", 6) == 6)
	  return 0;
    }
    return -1;
}

int
p9sndbrk(int msec) {
    if (ttyctlfd >= 0) {
	char cmd[20];
	int i = sprintf(cmd, "k%d", msec);
	if (write(ttyctlfd, cmd, i) == i)
	  return 0;
    }
    return -1;
}

int
conwrite(char *buf, int n) {
    int x;
    static int length = 0;
    static int holdingcr = 0;
    int normal = 0;
    for (x = 0; x < n; x++) {
	char c = buf[x];
	if (c == 007) {
	    if (normal) {
		write(1, buf + (x - normal), normal);
		length += normal;
		normal = 0;
	    }
	    /* write(noisefd, "1000 300", 8); */
	    holdingcr = 0;
	} else if (c == '\r') {
	    if (normal) {
		write(1, buf + (x - normal), normal);
		length += normal;
		normal = 0;
	    }
	    holdingcr = 1;
	} else if (c == '\n') {
	    write(1, buf + (x - normal), normal + 1);
	    normal = 0;
	    length = 0;
	    holdingcr = 0;
	} else if (c == '\b') {
	    if (normal) {
		write(1, buf + (x - normal), normal);
		length += normal;
		normal = 0;
	    }
	    if (length) {
		write(1, &c, 1);
		length--;
	    }
	    holdingcr = 0;
	} else {
	    if (holdingcr) {
		char b = '\b';
		while (length-- > 0)
		  write(1, &b, 1);
		length = 0;	/* compiler bug */
	    }
	    holdingcr = 0;
	    normal++;
	}
    }
    if (normal) {
	write(1, buf + (x - normal), normal);
	length += normal;
    }
    return n;
}

void
conprint(char *fmt, ...) {
    static char buf[1000];		/* not safe if on the stack */

    va_list ap;
    int i;

    va_start(ap, fmt);
    i = vsprintf(buf, fmt, ap);
    conwrite(buf, i);
}
#endif /* Plan9 */

/* fprintf, printf, perror replacements... */

/* f p r i n t f */

#ifdef UNIX
#ifdef CK_ANSIC
#include <stdarg.h>
#else /* CK_ANSIC */
#ifdef __GNUC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif	/* __GNUC__ */
#endif /* CK_ANSIC */
#ifdef fprintf
#undef fprintf
static char str1[4096];
static char str2[4096];
int
#ifdef CK_ANSIC
ckxfprintf(FILE * file, const char * format, ...)
#else /* CK_ANSIC */
ckxfprintf(va_alist) va_dcl
#endif /* CK_ANSIC */
/* ckxfprintf */ {
    int i, j, len, got_cr;
    va_list args;
    int rc = 0;

#ifdef CK_ANSIC
    va_start(args, format);
#else /* CK_ANSIC */
    char * format;
    FILE * file;
    va_start(args);
    file = va_arg(args,FILE *);
    format = va_arg(args,char *);
#endif /* CK_ANSIC */

    if (!inserver || (file != stdout && file != stderr && file != stdin)) {
	rc = vfprintf(file,format,args);
    } else {
	unsigned int c;
        rc = vsprintf(str1, format, args);
        len = strlen(str1);
        if (len >= sizeof(str1)) {
            debug(F101,"ckxfprintf() buffer overflow","",len);
            doexit(BAD_EXIT,1);
        }
        for (i = 0, j = 0, got_cr = 0;
	     i < len && j < sizeof(str1)-2;
	     i++, j++ ) {
	    /* We can't use 255 as a case label because of signed chars */
	    c = (unsigned)(str1[i] & 0xff);
#ifdef TNCODE
	    if (c == 255) {
		if (got_cr && !TELOPT_ME(TELOPT_BINARY))
		  str2[j++] = '\0';
		str2[j++] = IAC;
		str2[j] = IAC;
		got_cr = 0;
	    } else
#endif /* TNCODE */
	    switch (c) {
	      case '\r':
                if (got_cr
#ifdef TNCODE
		    && !TELOPT_ME(TELOPT_BINARY)
#endif /* TNCODE */
		    )
		  str2[j++] = '\0';
                str2[j] = str1[i];
                got_cr = 1;
                break;
	      case '\n':
                if (!got_cr)
		  str2[j++] = '\r';
                str2[j] = str1[i];
                got_cr = 0;
                break;
	      default:
                if (got_cr
#ifdef TNCODE
		    && !TELOPT_ME(TELOPT_BINARY)
#endif /* TNCODE */
		    )
		  str2[j++] = '\0';
                str2[j] = str1[i];
                got_cr = 0;
            }
        }
        if (got_cr
#ifdef TNCODE
             && !TELOPT_ME(TELOPT_BINARY)
#endif /* TNCODE */
             )
            str2[j++] = '\0';
#ifdef CK_ENCRYPTION
#ifdef TNCODE
        if (TELOPT_ME(TELOPT_ENCRYPTION))
	  ck_tn_encrypt(str2,j);
#endif /* TNCODE */
#endif /* CK_ENCRYPTION */
#ifdef CK_SSL
	if (inserver && (ssl_active_flag || tls_active_flag)) {
	    /* Write using SSL */
            char * p = str2;
          ssl_retry:
            if (ssl_active_flag)
	      rc = SSL_write(ssl_con, p, j);
            else
	      rc = SSL_write(tls_con, p, j);
	    debug(F111,"ckxfprintf","SSL_write",rc);
            switch (SSL_get_error(ssl_active_flag?ssl_con:tls_con,rc)) {
	      case SSL_ERROR_NONE:
                if (rc == j)
		  break;
                p += rc;
                j -= rc;
                goto ssl_retry;
	      case SSL_ERROR_WANT_WRITE:
	      case SSL_ERROR_WANT_READ:
	      case SSL_ERROR_SYSCALL:
                if (rc != 0)
		  return(-1);
	      case SSL_ERROR_WANT_X509_LOOKUP:
	      case SSL_ERROR_SSL:
	      case SSL_ERROR_ZERO_RETURN:
	      default:
                rc = 0;
            }
	} else
#endif /* CK_SSL */
        fwrite(str2,sizeof(char),j,stdout);
    }
    va_end(args);
    return(rc);
}
#endif /* fprintf */

/* p r i n t f */

#ifdef printf
#undef printf
int
#ifdef CK_ANSIC
ckxprintf(const char * format, ...)
#else /* CK_ANSIC */
ckxprintf(va_alist) va_dcl
#endif /* CK_ANSIC */
/* ckxprintf */ {
    int i, j, len, got_cr;
    va_list args;
    int rc = 0;

#ifdef CK_ANSIC
    va_start(args, format);
#else /* CK_ANSIC */
    char * format;
    va_start(args);
    format = va_arg(args,char *);
#endif /* CK_ANSIC */

    if (!inserver) {
	rc = vprintf(format, args);
    } else {
	unsigned int c;
        rc = vsprintf(str1, format, args);
        len = strlen(str1);
        if (len >= sizeof(str1)) {
            debug(F101,"ckxprintf() buffer overflow","",len);
            doexit(BAD_EXIT,1);
        }
        for (i = 0, j = 0, got_cr=0;
	     i < len && j < sizeof(str1)-2;
	     i++, j++ ) {
	    c = (unsigned)(str1[i] & 0xff);
#ifdef TNCODE
	    if (c == 255) {
		if (got_cr && !TELOPT_ME(TELOPT_BINARY))
		  str2[j++] = '\0';
		str2[j++] = IAC;
		str2[j] = IAC;
		got_cr = 0;
	    } else
#endif /* TNCODE */
	    switch (c) {
	      case '\r':
                if (got_cr
#ifdef TNCODE
		    && !TELOPT_ME(TELOPT_BINARY)
#endif /* TNCODE */
		    )
		  str2[j++] = '\0';
                str2[j] = str1[i];
                got_cr = 1;
                break;
	      case '\n':
                if (!got_cr)
		  str2[j++] = '\r';
                str2[j] = str1[i];
                got_cr = 0;
                break;
	      default:
                if (got_cr
#ifdef TNCODE
		    && !TELOPT_ME(TELOPT_BINARY)
#endif /* TNCODE */
		    )
		  str2[j++] = '\0';
                str2[j] = str1[i];
                got_cr = 0;
                break;
	    }
        }
        if (got_cr
#ifdef TNCODE
             && !TELOPT_ME(TELOPT_BINARY)
#endif /* TNCODE */
             )
            str2[j++] = '\0';
#ifdef CK_ENCRYPTION
#ifdef TNCODE
        if (TELOPT_ME(TELOPT_ENCRYPTION))
	  ck_tn_encrypt(str2,j);
#endif /* TNCODE */
#endif /* CK_ENCRYPTION */
#ifdef CK_SSL
	if (inserver && (ssl_active_flag || tls_active_flag)) {
            char * p = str2;
	    /* Write using SSL */
          ssl_retry:
            if (ssl_active_flag)
	      rc = SSL_write(ssl_con, p, j);
            else
	      rc = SSL_write(tls_con, p, j);
	    debug(F111,"ckxprintf","SSL_write",rc);
            switch (SSL_get_error(ssl_active_flag?ssl_con:tls_con,rc)) {
	      case SSL_ERROR_NONE:
                if (rc == j)
		  break;
                p += rc;
                j -= rc;
                goto ssl_retry;
	      case SSL_ERROR_WANT_WRITE:
	      case SSL_ERROR_WANT_READ:
	      case SSL_ERROR_SYSCALL:
                if (rc != 0)
		  return(-1);
	      case SSL_ERROR_WANT_X509_LOOKUP:
	      case SSL_ERROR_SSL:
	      case SSL_ERROR_ZERO_RETURN:
	      default:
                rc = 0;
            }
	} else
#endif /* CK_SSL */
	  rc = fwrite(str2,sizeof(char),j,stdout);
    }
    va_end(args);
    return(rc);
}
#endif /* printf */

/*  p e r r o r  */

#ifdef perror
#undef perror
_PROTOTYP(char * ck_errstr,(VOID));
#ifdef NEXT
void
#else
#ifdef CK_SCOV5
void
#else
int
#endif /* CK_SCOV5 */
#endif /* NEXT */
#ifdef CK_ANSIC
ckxperror(const char * str)
#else /* CK_ANSIC */
ckxperror(str) char * str;
#endif /* CK_ANSIC */
/* ckxperror */ {
    char * errstr = ck_errstr();
#ifndef NEXT
#ifndef CK_SCOV5
    return
#endif /* CK_SCOV5 */
#endif /* NEXT */
      ckxprintf("%s%s %s\n",str,*errstr?":":"",errstr);
}
#endif /* perror */
#endif /* UNIX */

#ifdef MINIX2

/* Minix doesn't have a gettimeofday call (but MINIX3 does).
 * We fake one here using time(2)
 */

#ifndef MINIX3
int
gettimeofday(struct timeval *tp, struct timezone *tzp) {
    tp->tv_usec = 0L;			/* Close enough for horseshoes */
    if(time(&(tp->tv_sec))==-1)
      return(-1);
    return(0);
}
#endif	/* MINIX3 */

#ifndef MINIX3
int
readlink(const char *path, void *buf, size_t bufsiz) {
    errno = ENOSYS;
    return(-1);
}
#endif	/* MINIX3 */

#endif /* MINIX2 */

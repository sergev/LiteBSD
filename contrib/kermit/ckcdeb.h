/*  C K C D E B . H  */

/*
Mon Aug 23 09:22:05 2010

  NOTE TO CONTRIBUTORS: This file, and all the other C-Kermit files, must be
  compatible with C preprocessors that support only #ifdef, #else, #endif,
  #define, and #undef.  Please do not use #if, logical operators, or other
  later-model preprocessor features in any of the portable C-Kermit modules.
  You can, of course, use these constructions in platform-specific modules
  when you know they are supported.
*/

/*
  This file is included by all C-Kermit modules, including the modules
  that aren't specific to Kermit (like the command parser and the ck?tio and
  ck?fio modules).  It should be included BEFORE any other C-Kermit header
  files.  It specifies format codes for debug(), tlog(), and similar
  functions, and includes any necessary definitions to be used by all C-Kermit
  modules, and also includes some feature selection compile-time switches, and
  also system- or compiler-dependent definitions, plus #includes and prototypes
  required by all C-Kermit modules.
*/

/*
  Author: Frank da Cruz <fdc@columbia.edu>,
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 2010,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/

/*
  Etymology: The name of this file means "C-Kermit Common-C-Language Debugging
  Header", because originally it contained only the formats (F000-F111) for
  the debug() and tlog() functions.  Since then it has grown to include all
  material required by all other C-Kermit modules, including the non-Kermit
  specific ones.  In other words, this is the one header file that is
  guaranteed to be included by all C-Kermit source modules.
*/

#ifndef CKCDEB_H			/* Don't include me more than once. */
#define CKCDEB_H

#ifdef OS2
#include "ckoker.h"
#else /* OS2 */
/* Unsigned numbers */

#ifndef USHORT
#define USHORT unsigned short
#endif /* USHORT */

#ifndef UINT
#define UINT unsigned int
#endif /* UINT */

#ifndef ULONG
#define ULONG unsigned long
#endif /* ULONG */
#endif /* OS2 */

#ifdef MACOSX10				/* Mac OS X 1.0 */
#ifndef MACOSX				/* implies Mac OS X */
#define MACOSX
#endif /* MACOSX */
#endif /* MACOSX10 */

#ifdef MACOSX				/* Mac OS X */
#ifndef BSD44				/* implies 4.4 BSD */
#define BSD44
#endif /* BSD44 */
#endif /* MACOSX */

#ifdef SCO_OSR505			/* SCO 3.2v5.0.5 */
#ifndef SCO_OSR504			/* implies SCO 3.2v5.0.4 */
#define SCO_OSR504
#endif /* SCO_OSR504 */
#endif /* SCO_OSR505 */

#ifdef SCO_OSR504			/* SCO 3.2v5.0.4 */
#ifndef CK_SCOV5			/* implies SCO 3.2v5.0 */
#define CK_SCOV5
#endif /* CK_SCOV5 */
#include <sys/types.h>			/* To sidestep header-file mess */
#endif /* SCO_OSR504 */

#ifdef CK_SCOV5
#ifndef ANYSCO
#define ANYSCO
#endif /* ANYSCO */
#endif /* CK_SCOV5 */

#ifdef UNIXWARE
#ifndef ANYSCO
#define ANYSCO
#endif /* ANYSCO */
#endif /* UNIXWARE */

#ifndef MINIX				/* Minix versions */
#ifdef MINIX315
#define MINIX
#else
#ifdef MINIX3
#define MINIX
#else
#ifdef MINIX2
#define MINIX
#endif	/* MINIX2 */
#endif	/* MINIX3 */
#endif	/* MINIX315 */
#endif	/* MINIX */

#ifdef CK_SCO32V4			/* SCO 3.2v4 */
#ifndef ANYSCO
#define ANYSCO
#endif /* ANYSCO */
#ifndef XENIX
#define XENIX
#endif /* XENIX */
#ifndef SVR3
#define SVR3
#endif /* SVR3 */
#ifndef DIRENT
#define DIRENT
#endif /* DIRENT */
#ifndef RENAME
#define RENAME
#endif /* RENAME */
#ifndef SVR3JC
#define SVR3JC
#endif /* SVR3JC */
#ifndef CK_RTSCTS
#define CK_RTSCTS
#endif /* CK_RTSCTS */
#ifndef PID_T
#define PID_T pid_t
#endif /* PID_T */
#ifndef PWID_T
#define PWID_T int
#endif /* PWID_T */
#endif /* CK_SCO32V4 */

#ifdef NOICP				/* If no command parser */
#ifndef NOSPL				/* Then no script language either */
#define NOSPL
#endif /* NOSPL */
#ifndef NOCSETS				/* Or characer sets */
#define NOCSETS
#endif /* NOCSETS */
#ifndef NOFTP				/* Or FTP client */
#define NOFTP
#endif /* NOFTP */
#endif /* NOICP */

/* Built-in makefile entries */

#ifdef SOLARIS11			/* Solaris 11 implies 10 */
#ifndef SOLARIS10
#define SOLARIS10
#endif /* SOLARIS10 */
#endif /* SOLARIS11 */

#ifdef SOLARIS10			/* Solaris 10 implies 9 */
#ifndef SOLARIS9
#define SOLARIS9
#endif /* SOLARIS9 */
#endif /* SOLARIS10 */

#ifdef SOLARIS9				/* Solaris 9 implies 8 */
#ifndef SOLARIS8
#define SOLARIS8
#endif /* SOLARIS8 */
#endif /* SOLARIS9 */

#ifdef SOLARIS8				/* Solaris 8 implies 7 */
#ifndef SOLARIS7
#define SOLARIS7
#endif /* SOLARIS7 */
#endif /* SOLARIS8 */

#ifdef SOLARIS7				/* Solaris 7 implies 2.6 */
#ifndef SOLARIS26
#define SOLARIS26
#endif /* SOLARIS26 */
#endif /* SOLARIS7 */

#ifdef SOLARIS26			/* Solaris 2.6 implies 2.5 */
#ifndef SOLARIS25
#define SOLARIS25
#endif /* SOLARIS25 */
#endif /* SOLARIS26 */

#ifdef SOLARIS25			/* Solaris 2.5 implies Solaris */
#ifndef SOLARIS
#define SOLARIS
#endif /* SOLARIS */
#ifndef POSIX				/* And POSIX */
#define POSIX
#endif /* POSIX */
#ifndef CK_WREFRESH			/* And this (curses) */
#define CK_WREFRESH
#endif /* CK_WREFRESH */
#endif /* SOLARIS25 */

#ifdef SOLARIS24			/* Solaris 2.4 implies Solaris */
#ifndef SOLARIS
#define SOLARIS
#endif /* SOLARIS */
#endif /* SOLARIS24 */

#ifdef SOLARIS				/* Solaris gets "POSIX" RTS/CTS API */
#ifdef POSIX
#ifndef POSIX_CRTSCTS
#define POSIX_CRTSCTS
#endif /* POSIX_CRTSCTS */
#endif /* POSIX */
#ifndef SVR4
#define SVR4
#endif	/* SVR4 */
#ifndef STERMIOX
#define STERMIOX
#endif	/* STERMIOX */
#ifndef SELECT
#define SELECT
#endif	/* SELECT */
#ifndef FNFLOAT
#define FNFLOAT
#endif	/* FNFLOAT */
#ifndef DIRENT
#define DIRENT
#endif	/* DIRENT */
#ifndef BIGBUFOK
#define BIGBUFOK
#endif	/* BIGBUFOK */
#ifndef CK_NEWTERM
#define CK_NEWTERM
#endif	/* CK_NEWTERM */
#endif /* SOLARIS */

#ifdef SUN4S5				/* Sun-4 System V environment */
#ifndef SVR3				/* implies System V R3 or later */
#define SVR3
#endif /* SVR3 */
#endif /* SUN4S5 */
#ifdef SUNOS41				/* SUNOS41 implies SUNOS4 */
#ifndef SUNOS4
#define SUNOS4
#endif /* SUNOS4 */
#endif /* SUNOS41 */

#ifdef SUN4S5				/* Sun-4 System V environment */
#ifndef SVR3				/* implies System V R3 or later */
#define SVR3
#endif /* SVR3 */
#endif /* SUN4S5 */

#ifdef SUNOS41				/* SUNOS41 implies SUNOS4 */
#ifndef SUNOS4
#define SUNOS4
#endif /* SUNOS4 */
#endif /* SUNOS41 */

#ifdef SUNOS4				/* Built-in SUNOS4 makefile entry */
#ifndef UNIX
#define UNIX
#endif /* UNIX */
#ifndef BSD4
#define BSD4
#endif /* BSD4 */
#ifndef NOSETBUF
#define NOSETBUF
#endif /* NOSETBUF */
#ifndef DIRENT
#define DIRENT
#endif /* DIRENT */
#ifndef NONET
#ifndef TCPSOCKET
#define TCPSOCKET
#endif /* TCPSOCKET */
#endif /* NONET */
#ifndef SAVEDUID
#define SAVEDUID
#endif /* SAVEDUID */
#ifndef DYNAMIC
#define DYNAMIC
#endif /* DYNAMIC */
#endif /* SUNOS4 */

#ifdef SOLARIS				/* Built in makefile entry */
#ifndef NOSETBUF			/* for Solaris 2.x */
#define NOSETBUF
#endif /* NOSETBUF */
#ifndef NOCURSES
#ifndef CK_CURSES
#define CK_CURSES
#endif /* CK_CURSES */
#endif /* NOCURSES */
#ifndef CK_NEWTERM
#define CK_NEWTERM
#endif /* CK_NEWTERM */
#ifndef DIRENT
#define DIRENT
#endif /* DIRENT */
#ifndef NONET
#ifndef TCPSOCKET
#define TCPSOCKET
#endif /* TCPSOCKET */
#endif /* NONET */
#ifndef UNIX
#define UNIX
#endif /* UNIX */
#ifndef SVR4
#define SVR4
#endif /* SVR4 */
#ifndef HADDRLIST
#define HADDRLIST
#endif /* HADDRLIST */
#ifndef STERMIOX
#define STERMIOX
#endif /* STERMIOX */
#ifndef SELECT
#define SELECT
#endif /* SELECT */
#ifndef DYNAMIC
#define DYNAMIC
#endif /* DYNAMIC */
#ifndef NOUUCP
#ifndef HDBUUCP
#define HDBUUCP
#endif /* HDBUUCP */
#endif /* NOUUCP */
#endif /* SOLARIS */

/* Features that can be eliminated from a no-file-transfer version */

#ifdef NOXFER
#ifndef NOFTP
#define NOFTP
#endif /* NOFTP */
#ifndef OS2
#ifndef NOCURSES			/* Fullscreen file-transfer display */
#define NOCURSES
#endif /* NOCURSES */
#endif /* OS2 */
#ifndef NOCKXYZ				/* XYZMODEM support */
#define NOCKXYZ
#endif /* NOCKXYZ */
#ifndef NOCKSPEED			/* Ctrl-char unprefixing */
#define NOCKSPEED
#endif /* NOCKSPEED */
#ifndef NOSERVER			/* Server mode */
#define NOSERVER
#endif /* NOSERVER */
#ifndef NOCKTIMERS			/* Dynamic packet timers */
#define NOCKTIMERS
#endif /* NOCKTIMERS */
#ifndef NOPATTERNS			/* File-type patterns */
#define NOPATTERNS
#endif /* NOPATTERNS */
#ifndef NOSTREAMING			/* Streaming */
#define NOSTREAMING
#endif /* NOSTREAMING */
#ifndef NOIKSD				/* Internet Kermit Service */
#define NOIKSD
#endif /* NOIKSD */
#ifndef NOPIPESEND			/* Sending from pipes */
#define NOPIPESEND
#endif /* NOPIPESEND */
#ifndef NOAUTODL			/* Autodownload */
#define NOAUTODL
#endif /* NOAUTODL */
#ifndef NOMSEND				/* MSEND */
#define NOMSEND
#endif /* NOMSEND */
#ifndef NOTLOG				/* Transaction logging */
#define NOTLOG
#endif /* NOTLOG */
#ifndef NOCKXXCHAR			/* Packet character doubling */
#define NOCKXXCHAR
#endif /* NOCKXXCHAR */
#endif /* NOXFER */

#ifdef NOICP				/* No Interactive Command Parser */
#ifndef NODIAL				/* Implies No DIAL command */
#define NODIAL
#endif /* NODIAL */
#ifndef NOCKXYZ				/* and no external protocols */
#define NOCKXYZ
#endif /* NOCKXYZ */
#endif /* NOICP */

#ifndef NOIKSD
#ifdef IKSDONLY
#ifndef IKSD
#define IKSD
#endif /* IKSD */
#ifndef NOLOCAL
#define NOLOCAL
#endif /* NOLOCAL */
#ifndef NOPUSH
#define NOPUSH
#endif /* NOPUSH */
#ifndef TNCODE
#define TNCODE
#endif /* TNCODE */
#ifndef TCPSOCKET
#define TCPSOCKET
#endif /* TCPSOCKET */
#ifndef NETCONN
#define NETCONN
#endif /* NETCONN */
#ifdef SUNX25
#undef SUNX25
#endif /* SUNX25 */
#ifdef IBMX25
#undef IBMX25
#endif /* IBMX25 */
#ifdef STRATUSX25
#undef STRATUSX25
#endif /* STRATUSX25 */
#ifdef CK_NETBIOS
#undef CK_NETBIOS
#endif /* CK_NETBIOS */
#ifdef SUPERLAT
#undef SUPERLAT
#endif /* SUPERLAT */
#ifdef NPIPE
#undef NPIPE
#endif /* NPIPE */
#ifdef NETFILE
#undef NETFILE
#endif /* NETFILE */
#ifdef NETCMD
#undef NETCMD
#endif /* NETCMD */
#ifdef NETPTY
#undef NETPTY
#endif /* NETPTY */
#ifdef RLOGCODE
#undef RLOGCODE
#endif /* RLOGCODE */
#ifdef NETDLL
#undef NETDLL
#endif /* NETDLL */
#ifndef NOSSH
#undef NOSSH
#endif /* NOSSH */
#ifndef NOFORWARDX
#define NOFORWARDX
#endif /* NOFORWARDX */
#ifndef NOBROWSER
#define NOBROWSER
#endif /* NOBROWSER */
#ifndef NOHTTP
#define NOHTTP
#endif /* NOHTTP */
#ifndef NOFTP
#define NOFTP
#endif /* NOFTP */
#ifndef NO_COMPORT
#define NO_COMPORT
#endif /* NO_COMPORT */
#endif /* IKSDONLY */
#endif /* NOIKSD */

/* Features that can be eliminated from a remote-only version */

#ifdef NOLOCAL
#ifndef NOFTP
#define NOFTP
#endif /* NOFTP */
#ifndef NOHTTP
#define NOHTTP
#endif /* NOHTTP */
#ifndef NOSSH
#define NOSSH
#endif /* NOSSH */
#ifndef NOTERM
#define NOTERM
#endif /* NOTERM */
#ifndef NOCURSES			/* Fullscreen file-transfer display */
#define NOCURSES
#endif /* NOCURSES */
#ifndef NODIAL
#define NODIAL
#endif /* NODIAL */
#ifndef NOSCRIPT
#define NOSCRIPT
#endif /* NOSCRIPT */
#ifndef NOSETKEY
#define NOSETKEY
#endif /* NOSETKEY */
#ifndef NOKVERBS
#define NOKVERBS
#endif /* NOKVERBS */
#ifndef NOXMIT
#define NOXMIT
#endif /* NOXMIT */
#ifdef CK_CURSES
#undef CK_CURSES
#endif /* CK_CURSES */
#ifndef IKSDONLY
#ifndef NOAPC
#define NOAPC
#endif /* NOAPC */
#ifndef NONET
#define NONET
#endif /* NONET */
#endif /* IKSDONLY */
#endif /* NOLOCAL */

#ifdef NONET
#ifdef NETCONN
#undef NETCONN
#endif /* NETCONN */
#ifdef TCPSOCKET
#undef TCPSOCKET
#endif /* TCPSOCKET */
#ifndef NOTCPOPTS
#define NOTCPOPTS
#endif /* NOTCPOPTS */
#ifdef SUNX25
#undef SUNX25
#endif /* SUNX25 */
#ifdef IBMX25
#undef IBMX25
#endif /* IBMX25 */
#ifdef STRATUSX25
#undef STRATUSX25
#endif /* STRATUSX25 */
#ifdef CK_NETBIOS
#undef CK_NETBIOS
#endif /* CK_NETBIOS */
#ifdef SUPERLAT
#undef SUPERLAT
#endif /* SUPERLAT */
#ifdef NPIPE
#undef NPIPE
#endif /* NPIPE */
#ifdef NETFILE
#undef NETFILE
#endif /* NETFILE */
#ifdef NETCMD
#undef NETCMD
#endif /* NETCMD */
#ifdef NETPTY
#undef NETPTY
#endif /* NETPTY */
#ifdef RLOGCODE
#undef RLOGCODE
#endif /* RLOGCODE */
#ifdef NETDLL
#undef NETDLL
#endif /* NETDLL */
#ifndef NOSSH
#define NOSSH
#endif /* NOSSH */
#ifndef NOFTP
#define NOFTP
#endif /* NOFTP */
#ifndef NOHTTP
#define NOHTTP
#endif /* NOHTTP */
#ifndef NOBROWSER
#define NOBROWSER
#endif /* NOBROWSER */
#ifndef NOFORWARDX
#define NOFORWARDX
#endif /* NOFORWARDX */
#endif /* NONET */

#ifdef IKSDONLY
#ifdef SUNX25
#undef SUNX25
#endif /* SUNX25 */
#ifdef IBMX25
#undef IBMX25
#endif /* IBMX25 */
#ifdef STRATUSX25
#undef STRATUSX25
#endif /* STRATUSX25 */
#ifdef CK_NETBIOS
#undef CK_NETBIOS
#endif /* CK_NETBIOS */
#ifdef SUPERLAT
#undef SUPERLAT
#endif /* SUPERLAT */
#ifdef NPIPE
#undef NPIPE
#endif /* NPIPE */
#ifdef NETFILE
#undef NETFILE
#endif /* NETFILE */
#ifdef NETCMD
#undef NETCMD
#endif /* NETCMD */
#ifdef NETPTY
#undef NETPTY
#endif /* NETPTY */
#ifdef RLOGCODE
#undef RLOGCODE
#endif /* RLOGCODE */
#ifdef NETDLL
#undef NETDLL
#endif /* NETDLL */
#ifndef NOSSH
#define NOSSH
#endif /* NOSSH */
#ifndef NOHTTP
#define NOHTTP
#endif /* NOHTTP */
#ifndef NOBROWSER
#define NOBROWSER
#endif /* NOBROWSER */
#endif /* IKSDONLY */
/*
  Note that none of the above precludes TNCODE, which can be defined in
  the absence of TCPSOCKET, etc, to enable server-side Telnet negotation.
*/
#ifndef TNCODE				/* This is for the benefit of */
#ifdef TCPSOCKET			/* modules that might need TNCODE */
#define TNCODE				/* not all of ckcnet.h... */
#endif /* TCPSOCKET */
#endif /* TNCODE */

#ifndef NETCONN
#ifdef TCPSOCKET
#define NETCONN
#endif /* TCPSOCKET */
#endif /* NETCONN */

#ifndef DEFPAR				/* Default parity */
#define DEFPAR 0			/* Must be here because it is used */
#endif /* DEFPAR */			/* by all classes of modules */

#ifdef NT
#ifndef OS2ORWIN32
#define OS2ORWIN32
#endif /* OS2ORWIN32 */
#ifndef OS2
#define WIN32ONLY
#endif /* OS2 */
#endif /* NT */

#ifdef OS2				/* For OS/2 debugging */
#ifndef OS2ORWIN32
#define OS2ORWIN32
#endif /* OS2ORWIN32 */
#ifdef NT
#define NOCRYPT
#include <windows.h>
#define NTSIG
#else /* NT */
#define OS2ONLY
#include <os2def.h>
#endif /* NT */
#ifndef OS2ORUNIX
#define OS2ORUNIX
#endif /* OS2ORUNIX */
#ifndef OS2ORVMS
#define OS2ORVMS
#endif /* OS2ORVMS */
#endif /* OS2 */

#include <stdio.h>			/* Begin by including this. */
#include <ctype.h>			/* and this. */

#ifdef VMS
#include <types.h>			/* Ensure off_t. */
#include "ckvrms.h"			/* Get NAMDEF NAMX_C_MAXRSS. */
#endif /* def VMS */

/* System-type compilation switches */

#ifdef FT21				/* Fortune For:Pro 2.1 implies 1.8 */
#ifndef FT18
#define FT18
#endif /* FT18 */
#endif /* FT21 */

#ifdef __bsdi__
#ifndef BSDI
#define BSDI
#endif /* BSDI */
#endif /* __bsdi__ */

#ifdef AIXPS2				/* AIXPS2 implies AIX370 */
#ifndef AIX370
#define AIX370
#endif /* AIX370 */
#endif /* AIXPS2 */

#ifdef AIX370				/* AIX PS/2 or 370 implies BSD4 */
#ifndef BSD4
#define BSD4
#endif /* BSD4 */
#endif /* AIX370 */

#ifdef AIXESA				/* AIX/ESA implies BSD4.4 */
#ifndef BSD44
#define BSD44
#endif /* BSD44 */
#endif /* AIXESA */

#ifdef AIX53				/* AIX53 implies AIX52 */
#ifndef AIX52
#define AIX52
#endif /* AIX52 */
#endif /* AIX53 */

#ifdef AIX52				/* AIX52 implies AIX51 */
#ifndef AIX51
#define AIX51
#endif /* AIX51 */
#endif /* AIX52 */

#ifdef AIX51				/* AIX51 implies AIX50 */
#ifndef AIX50
#define AIX50
#endif /* AIX50 */
#endif /* AIX51 */

#ifdef AIX50				/* AIX50 implies AIX45 */
#ifndef AIX45
#define AIX45
#endif /* AIX45 */
#endif /* AIX50 */

#ifdef AIX45				/* AIX45 implies AIX44 */
#ifndef AIX44
#define AIX44
#endif /* AIX44 */
#endif /* AIX45 */

#ifdef AIX44				/* AIX44 implies AIX43 */
#ifndef AIX43
#define AIX43
#endif /* AIX43 */
#endif /* AIX44 */

#ifdef AIX43				/* AIX43 implies AIX42 */
#ifndef AIX42
#define AIX42
#endif /* AIX42 */
#endif /* AIX43 */

#ifdef AIX42				/* AIX42 implies AIX41 */
#ifndef AIX41
#define AIX41
#endif /* AIX41 */
#endif /* AIX42 */

#ifdef SV68R3V6				/* System V/68 R32V6 implies SVR3 */
#ifndef SVR3
#define SVR3
#endif /* SVR3 */
#endif /* SV68R3V6 */

#ifdef SV88R32				/* System V/88 R32 implies SVR3 */
#ifndef SVR3
#define SVR3
#endif /* SVR3 */
#endif /* SV88R32 */

#ifdef DGUX540				/* DG UX 5.40 implies Sys V R 4 */
#ifndef SVR4
#define SVR4
#endif /* SVR4 */
#endif /* DGUX540 */

#ifndef DGUX
#ifdef DGUX540				/* DG/UX 5.40 implies DGUX */
#define DGUX
#else
#ifdef DGUX430				/* So does DG/UX 4.30 */
#define DGUX
#endif /* DGUX430 */
#endif /* DGUX540 */
#endif /* DGUX */

#ifdef IRIX65				/* IRIX 6.5 implies IRIX 6.4 */
#ifndef IRIX64
#define IRIX64
#endif /* IRIX64 */
#endif /* IRIX65 */

#ifdef IRIX64				/* IRIX 6.4 implies IRIX 6.2 */
#ifndef BSD44ORPOSIX
#define BSD44ORPOSIX			/* for ckutio's benefit */
#endif /* BSD44ORPOSIX */
#ifndef IRIX62
#define IRIX62
#endif /* IRIX62 */
#endif /* IRIX64 */

#ifdef IRIX62				/* IRIX 6.2 implies IRIX 6.0 */
#ifndef IRIX60
#define IRIX60
#endif /* IRIX60 */
#endif /* IRIX62 */

#ifdef IRIX60				/* IRIX 6.0 implies IRIX 5.1 */
#ifndef IRIX51
#define IRIX51
#endif /* IRIX51 */
#ifndef IRIX52				/* And IRIX 5.2 (for hwfc) */
#define IRIX52
#endif /* IRIX52 */
#endif /* IRIX60 */

#ifndef IRIX				/* IRIX 4.0 or greater implies IRIX */
#ifdef IRIX64
#define IRIX
#else
#ifdef IRIX62
#define IRIX
#else
#ifdef IRIX60
#define IRIX
#else
#ifdef IRIX51
#define IRIX
#else
#ifdef IRIX40
#define IRIX
#endif /* IRIX40 */
#endif /* IRIX51 */
#endif /* IRIX60 */
#endif /* IRIX62 */
#endif /* IRIX64 */
#endif /* IRIX */

#ifdef MIPS				/* MIPS System V environment */
#ifndef SVR3				/* implies System V R3 or later */
#define SVR3
#endif /* SVR3 */
#endif /* MIPS */

#ifdef HPUX9				/* HP-UX 9.x */
#ifndef SVR3
#define SVR3
#endif /* SVR3 */
#ifndef HPUX
#define HPUX
#endif /* HPUX */
#ifndef HPUX9PLUS
#define HPUX9PLUS
#endif /* HPUX9PLUS */
#endif /* HPUX9 */

#ifdef HPUX10				/* HP-UX 10.x */
#ifndef HPUX1010			/* If anything higher is defined */
#ifdef HPUX1020				/* define HPUX1010 too. */
#define HPUX1010
#endif /* HPUX1020 */
#ifdef HPUX1030
#define HPUX1010
#endif /* HPUX1030 */
#endif /* HPUX1010 */

#ifdef HPUX1100				/* HP-UX 11.00 implies 10.10 */
#ifndef HPUX1010
#define HPUX1010
#endif /* HPUX1010 */
#endif /* HPUX1100 */

#ifndef SVR4
#define SVR4
#endif /* SVR4 */
#ifndef HPUX
#define HPUX
#endif /* HPUX */
#ifndef HPUX9PLUS
#define HPUX9PLUS
#endif /* HPUX9PLUS */
#endif /* HPUX10 */

#ifdef QNX				/* QNX Software Systems Inc */
#ifndef POSIX				/* QNX 4.0 or later is POSIX */
#define POSIX
#endif /* POSIX */
#ifndef __386__				/* Comes in 16-bit and 32-bit */
#define __16BIT__
#define CK_QNX16
#else
#define __32BIT__
#define CK_QNX32
#endif /* __386__ */
#endif /* QNX */

/*
  4.4BSD is a mixture of System V R4, POSIX, and 4.3BSD.
*/
#ifdef BSD44				/* 4.4 BSD */
#ifndef SVR4				/* BSD44 implies SVR4 */
#define SVR4
#endif /* SVR4 */
#ifndef NOSETBUF			/* NOSETBUF is safe */
#define NOSETBUF
#endif /* NOSETBUF */
#ifndef DIRENT				/* Uses <dirent.h> */
#define DIRENT
#endif /* DIRENT */
#endif /* BSD44 */

#ifdef OPENBSD				/* OpenBSD might or might not */
#ifndef __OpenBSD__			/* have this defined... */
#define __OpenBSD__
#endif /* __OpenBSD__ */
#endif /* OPENBSD */

#ifdef SVR3				/* SVR3 implies ATTSV */
#ifndef ATTSV
#define ATTSV
#endif /* ATTSV */
#endif /* SVR3 */

#ifdef SVR4				/* SVR4 implies ATTSV */
#ifndef ATTSV
#define ATTSV
#endif /* ATTSV */
#ifndef SVR3				/* ...as well as SVR3 */
#define SVR3
#endif /* SVR3 */
#endif /* SVR4 */

#ifdef OXOS
#ifndef ATTSV
#define ATTSV				/* OXOS implies ATTSV */
#endif /* ! ATTSV */
#define SW_ACC_ID			/* access() wants privs on */
#define kill priv_kill			/* kill() wants privs on */
#ifndef NOSETBUF
#define NOSETBUF			/* NOSETBUF is safe */
#endif /* ! NOSETBUF */
#endif /* OXOS */

#ifdef UTSV				/* UTSV implies ATTSV */
#ifndef ATTSV
#define ATTSV
#endif /* ATTSV */
#endif /* UTSV */

#ifdef XENIX				/* XENIX implies ATTSV */
#ifndef ATTSV
#define ATTSV
#endif /* ATTSV */
#endif /* XENIX */

#ifdef AUX				/* AUX implies ATTSV */
#ifndef ATTSV
#define ATTSV
#endif /* ATTSV */
#endif /* AUX */

#ifdef ATT7300				/* ATT7300 implies ATTSV */
#ifndef ATTSV
#define ATTSV
#endif /* ATTSV */
#endif /* ATT7300 */

#ifdef ATT6300				/* ATT6300 implies ATTSV */
#ifndef ATTSV
#define ATTSV
#endif /* ATTSV */
#endif /* ATT6300 */

#ifdef HPUX				/* HPUX implies ATTSV */
#ifndef ATTSV
#define ATTSV
#endif /* ATTSV */
#endif /* HPUX */

#ifdef ISIII				/* ISIII implies ATTSV */
#ifndef ATTSV
#define ATTSV
#endif /* ATTSV */
#endif /* ISIII */

#ifdef NEXT33				/* NEXT33 implies NEXT */
#ifndef NEXT
#define NEXT
#endif /* NEXT */
#endif /* NEXT33 */

#ifdef NEXT				/* NEXT implies BSD4 */
#ifndef BSD4
#define BSD4
#endif /* BSD4 */
#endif /* NEXT */

#ifdef BSD41				/* BSD41 implies BSD4 */
#ifndef BSD4
#define BSD4
#endif /* BSD4 */
#endif /* BSD41 */

#ifdef BSD43				/* BSD43 implies BSD4 */
#ifndef BSD4
#define BSD4
#endif /* BSD4 */
#endif /* BSD43 */

#ifdef BSD4				/* BSD4 implies ANYBSD */
#ifndef ANYBSD
#define ANYBSD
#endif /* ANYBSD */
#endif /* BSD4 */

#ifdef BSD29				/* BSD29 implies ANYBSD */
#ifndef ANYBSD
#define ANYBSD
#endif /* ANYBSD */
#endif /* BSD29 */

#ifdef ATTSV				/* ATTSV implies UNIX */
#ifndef UNIX
#define UNIX
#endif /* UNIX */
#endif /* ATTSV */

#ifdef ANYBSD				/* ANYBSD implies UNIX */
#ifndef UNIX
#define UNIX
#endif /* UNIX */
#endif /* ANYBSD */

#ifdef POSIX				/* POSIX implies UNIX */
#ifndef UNIX
#define UNIX
#endif /* UNIX */
#ifndef DIRENT				/* and DIRENT, i.e. <dirent.h> */
#ifndef SDIRENT
#define DIRENT
#endif /* SDIRENT */
#endif /* DIRENT */
#ifndef NOFILEH				/* POSIX doesn't use <sys/file.h> */
#define NOFILEH
#endif /* NOFILEH */
#endif /* POSIX */

#ifdef V7
#ifndef UNIX
#define UNIX
#endif /* UNIX */
#endif /* V7 */

#ifdef COHERENT
#ifndef UNIX
#define UNIX
#endif /* UNIX */
#ifdef COMMENT
#ifndef NOCURSES
#define NOCURSES
#endif /* NOCURSES */
#endif /* COMMENT */
#endif /* COHERENT */

#ifdef MINIX
#ifndef UNIX
#define UNIX
#endif /* UNIX */
#endif /* MINIX */
/*
  The symbol SVORPOSIX is defined for both AT&T and POSIX compilations
  to make it easier to select items that System V and POSIX have in common,
  but which BSD, V7, etc, do not have.
*/
#ifdef ATTSV
#ifndef SVORPOSIX
#define SVORPOSIX
#endif /* SVORPOSIX */
#endif /* ATTSV */

#ifdef POSIX
#ifndef SVORPOSIX
#define SVORPOSIX
#endif /* SVORPOSIX */
#endif /* POSIX */

/*
  The symbol SVR4ORPOSIX is defined for both AT&T System V R4 and POSIX
  compilations to make it easier to select items that System V R4 and POSIX
  have in common, but which BSD, V7, and System V R3 and earlier, etc, do
  not have.
*/
#ifdef POSIX
#ifndef SVR4ORPOSIX
#define SVR4ORPOSIX
#endif /* SVR4ORPOSIX */
#endif /* POSIX */
#ifdef SVR4
#ifndef SVR4ORPOSIX
#define SVR4ORPOSIX
#endif /* SVR4ORPOSIX */
#endif /* SVR4 */

/*
  The symbol BSD44ORPOSIX is defined for both 4.4BSD and POSIX compilations
  to make it easier to select items that 4.4BSD and POSIX have in common,
  but which System V, BSD, V7, etc, do not have.
*/
#ifdef BSD44
#ifndef BSD44ORPOSIX
#define BSD44ORPOSIX
#endif /* BSD44ORPOSIX */
#endif /* BSD44 */

#ifdef POSIX
#ifndef BSD44ORPOSIX
#define BSD44ORPOSIX
#endif /* BSD44ORPOSIX */
#endif /* POSIX */

#ifdef UNIX				/* For items common to OS/2 and UNIX */
#ifndef OS2ORUNIX
#define OS2ORUNIX
#endif /* OS2ORUNIX */
#endif /* UNIX */

#ifdef UNIX				/* For items common to VMS and UNIX */
#define VMSORUNIX
#else
#ifdef VMS
#define VMSORUNIX
#ifndef OS2ORVMS
#define OS2ORVMS
#endif /* OS2ORVMS */
#endif /* VMS */
#endif /* UNIX */

#ifndef UNIXOROSK			/* UNIX or OS-9 (or OS-9000) */
#ifdef UNIX
#define UNIXOROSK
#else
#ifdef OSK
#define UNIXOROSK
#endif /* OSK */
#endif /* UNIX */
#endif /* UNIXOROSK */

#ifndef OSKORUNIX
#ifdef UNIXOROSK
#define OSKORUNIX
#endif /* UNIXOROSK */
#endif /* OSKORUNIX */

#ifdef OS2
#define CK_ANSIC		 /* OS/2 supports ANSIC and more extensions */
#endif /* OS2 */

#ifdef OSF50			   /* Newer OSF/1 versions imply older ones */
#ifndef OSF40
#define OSF40
#endif /* OSF40 */
#endif /* OSF50 */

#ifdef OSF40
#ifndef OSF32
#define OSF32
#endif /* OSF32 */
#endif /* OSF40 */

#ifdef OSF32
#ifndef OSF30
#define OSF30
#endif /* OSF30 */
#endif /* OSF32 */

#ifdef OSF30
#ifndef OSF20
#define OSF20
#endif /* OSF20 */
#endif /* OSF30 */

#ifdef OSF20
#ifndef OSF10
#define OSF10
#endif /* OSF10 */
#endif /* OSF20 */

#ifdef __DECC				/* For DEC Alpha VMS or OSF/1 */
#ifndef CK_ANSIC
#define CK_ANSIC			/* Even with /stand=vaxc, need ansi */
#endif /* CKANSIC */
#ifndef SIG_V
#define SIG_V				/* and signal type is VOID */
#endif /* SIG_V */
#ifndef CK_ANSILIBS
#define CK_ANSILIBS			/* (Martin Zinser, Feb 1995) */
#endif /* CK_ANSILIBS */
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 1
#endif /* _POSIX_C_SOURCE */
#endif	/* __DECC */

#ifdef VMS
#ifdef __ia64				/* VMS on Itanium */
#ifndef VMSI64
#define VMSI64
#endif	/* VMSI64 */
#endif	/* __ia64 */
#ifndef VMS64BIT			/* 64-bit VMS on Itanium or Alpha */
#ifdef __ia64
#define VMS64BIT
#else
#ifdef __ALPHA
#define VMS64BIT
#endif	/* __ia64 */
#endif	/* __ALPHA */
#endif	/* VMS64BIT */
#endif	/* VMS */

#ifdef apollo				/* May be ANSI-C, check further */
#ifdef __STDCPP__
#define CK_ANSIC			/* Yes, this is real ANSI-C */
#define SIG_V
#else
#define NOANSI				/* Nope, not ANSI */
#undef __STDC__				/* Even though it say it is! */
#define SIG_I
#endif /* __STDCPP__ */
#endif /* apollo */

#ifdef POSIX				/* -DPOSIX on cc command line */
#ifndef _POSIX_SOURCE			/* Implies _POSIX_SOURCE */
#define _POSIX_SOURCE
#endif /* _POSIX_SOURCE */
#endif /* POSIX */

/*
  ANSI C?  That is, do we have function prototypes, new-style
  function declarations, and parameter type checking and coercion?
*/
#ifdef MAC				/* MPW C is ANSI */
#ifndef NOANSI
#ifndef CK_ANSIC
#define CK_ANSIC
#endif /* CK_ANSIC */
#endif /* NOANSI */
#endif /* MAC */

#ifdef STRATUS				/* Stratus VOS */
#ifndef CK_ANSIC
#define CK_ANSIC
#endif /* CK_ANSIC */
#ifndef NOSTAT
#define NOSTAT
#endif /* NOSTAT */
#endif /* STRATUS */

#ifndef NOANSI
#ifdef __STDC__				/* __STDC__ means ANSI C */
#ifndef CK_ANSIC
#define CK_ANSIC
#endif /* CK_ANSIC */
#endif /* __STDC__ */
#endif /* NOANSI */
/*
  _PROTOTYP() is used for forward declarations of functions so we can have
  parameter and return value type checking if the compiler offers it.
  __STDC__ should be defined by the compiler only if function prototypes are
  allowed.  Otherwise, we get old-style forward declarations.  Our own private
  CK_ANSIC symbol tells whether we use ANSI C prototypes.  To force use of
  ANSI prototypes, include -DCK_ANSIC on the cc command line.  To disable the
  use of ANSI prototypes, include -DNOANSI.
*/
#ifdef CK_ANSIC
#define _PROTOTYP( func, parms ) func parms
#else /* Not ANSI C */
#define _PROTOTYP( func, parms ) func()
#endif /* CK_ANSIC */

#ifndef OS2
#ifdef NOLOGIN				/* NOLOGIN implies NOIKSD */
#ifndef NOIKSD
#define NOIKSD
#endif /* NOIKSD */
#endif /* NOLOGIN */
#endif /* OS2 */

#ifdef NOIKSD				/* Internet Kermit Service Daemon */
#ifndef OS2
#ifndef NOPRINTFSUBST
#define NOPRINTFSUBST
#endif /* NOPRINTFSUBST */
#endif /* OS2 */
#ifndef NOLOGIN
#define NOLOGIN
#endif /* NOLOGIN */
#ifndef NOSYSLOG
#define NOSYSLOG
#endif /* NOSYSLOG */
#ifndef NOWTMP
#define NOWTMP
#endif /* NOWTMP */
#else
#ifndef IKSD
#ifdef OS2ORUNIX			/* Platforms where IKSD is supported */
#define IKSD
#endif /* OS2ORUNIX */
#endif /* IKSD */
#endif /* NOIKSD */

#ifdef IKSD				/* IKSD options... */
#ifndef IKSDCONF			/* IKSD configuration file */
#ifdef UNIX
#define IKSDCONF "/etc/iksd.conf"
#else
#ifdef OS2
#define IKSDCONF "iksd.ksc"
#endif /* OS2 */
#endif /* UNIX */
#endif /* IKSDCONF */
#ifndef NOIKSDB
#ifndef IKSDB				/* IKSD database */
#ifdef UNIX
#define IKSDB
#define IK_LCKTRIES 16			/* How many times to try to get lock */
#define IK_LCKSLEEP 1			/* How long to sleep between tries */
#define IK_LOCKFILE "iksd.lck"		/* Database lockfilename */
#define IK_DBASEDIR "/var/log/"		/* Database directory */
#define IK_DBASEFIL "iksd.db"		/* Database filename */
#else /* UNIX */
#ifdef OS2
#define IKSDB
#ifndef NOFTRUNCATE			/* ftruncate() not available */
#define NOFTRUNCATE
#endif /* NOFTRUNCATE */
#define IK_LCKTRIES 16			/* How many times to try to get lock */
#define IK_LCKSLEEP 1			/* How long to sleep between tries */
#define IK_LOCKFILE "iksd.lck"		/* DB lockfilename (in systemroot) */
#define IK_DBASEFIL "iksd.db"		/* Database filename */
#endif /* OS2 */
#endif /* UNIX */
#endif /* IKSDB */
#endif /* NOIKSDB */
#endif /* IKSD */
/*
  Substitutes for printf() and friends used in IKS to compensate for
  lack of a terminal driver, mainly to supply CR after LF.
*/
#ifndef NOPRINTFSUBST
#ifdef MAC
/*
 * The MAC doesn't use standard stdio routines.
 */
#undef getchar
#define getchar()   mac_getchar()
#undef putchar
#define putchar(c)	mac_putchar(c)
#define printf		mac_printf
#define perror		mac_perror
#define puts		mac_puts
extern int mac_putchar (int c);
extern int mac_puts (const char *string);
extern int mac_printf(const char *, ...);
extern int mac_getchar (void);
#endif /* MAC */

#ifdef OS2
#define printf Vscrnprintf
#define fprintf Vscrnfprintf
extern int Vscrnprintf(const char *, ...);
extern int Vscrnprintw(const char *, ...);
extern int Vscrnfprintf(FILE *, const char *, ...);
#ifdef putchar
#undef putchar
#endif /* putchar */
#define putchar(x) Vscrnprintf("%c",x)
#define perror(x)  Vscrnperror(x)
#endif /* OS2 */

#ifndef CKWART_C
#ifdef UNIX
#ifndef pdp11
#ifndef CKXPRINTF
#define CKXPRINTF
#endif /* CKXPRINTF */
#endif /* pdp11 */
#endif /* UNIX */
#endif /* CKWART_C */
#endif /* NOPRINTFSUBST */

#ifdef CKXPRINTF
#define printf ckxprintf
#define fprintf ckxfprintf
#ifdef CK_ANSIC
_PROTOTYP(int ckxprintf,(const char *, ...));
#ifdef NEXT
_PROTOTYP(void ckxperror,(const char *));
#else
#ifdef CK_SCOV5
_PROTOTYP(void ckxperror,(const char *));
#else
_PROTOTYP(int ckxperror,(const char *));
#endif /* CK_SCOV5 */
#endif /* NEXT */
_PROTOTYP(int ckxfprintf,(FILE *, const char *, ...));
#endif /* CK_ANSIC */
#ifdef putchar
#undef putchar
#endif /* putchar */
#define putchar(x) ckxprintf("%c",x)
#ifdef putc
#undef putc
#endif /* putc */
#define putc(a,b) ckxfprintf(b,"%c",a)
#define perror(x)  ckxperror(x)
#endif /* CKXPRINTF */

/*
  Altos-specific items: 486, 586, 986 models...
*/
#ifdef A986
#define M_VOID
#define void int
#define CHAR char
#define SIG_I
#endif /* A986 */

/* Signal handling */

#ifdef QNX
#ifndef CK_POSIX_SIG
#define CK_POSIX_SIG
#endif /* CK_POSIX_SIG */
#endif /* QNX */

/* 
  void type, normally available only in ANSI compilers.
  The HP-UX exception (for its "bundled" non-ANSI C compiler)
  is known to be valid back to HP-UX 6.5.
  Adjustments might be needed for earlier HP-UX versions.
*/
#ifndef VOID				/* Used throughout all C-Kermit */
#ifdef CK_ANSIC				/* modules... */
#define VOID void
#else
#ifdef HPUX
#define VOID void
#else
#define VOID int
#endif /* HPUX */
#endif /* CK_ANSIC */
#endif /* VOID */
/*
  Exactly the same as VOID but for use in contexts where the VOID symbol
  conflicts some header-file definition.  This is needed for the section
  of ckuusx.c that provides C-Kermit's curses interface, roughly the
  second half of ckuusx.c.
*/
#ifndef CKVOID
#ifdef CK_ANSIC
#define CKVOID void
#else
#ifdef HPUX
#define CKVOID void
#else
#define CKVOID int
#endif /* HPUX */
#endif /* CK_ANSIC */
#endif /* CKVOID */

/* Const type */

#ifndef CONST
#ifdef OSK
#ifdef _UCC
#define CONST const
#else
#define CONST
#endif /* _UCC */
#else  /* !OSK */
#ifdef CK_SCO32V4
#define CONST
#else
#ifdef CK_ANSIC
#define CONST const
#else
#define CONST
#endif /* CK_ANSIC */
#endif /* CK_SCO32V4 */
#endif /* OSK */
#endif /* CONST */

/* Signal type */

#ifndef SIG_V				/* signal() type, if not def'd yet */
#ifndef SIG_I
#ifdef OS2
#define SIG_V
#else
#ifdef POSIX
#define SIG_V
#else
#ifdef SVR3				/* System V R3 and later */
#define SIG_V
#else
#ifdef SUNOS4				/* SUNOS V 4.0 and later */
#ifndef sun386
#define SIG_V
#else
#define SIG_I
#endif /* sun386 */
#else
#ifdef NEXT				/* NeXT */
#define SIG_V
#else
#ifdef AIX370
#include <signal.h>
#define SIG_V
#define SIGTYP __SIGVOID		/* AIX370 */
#else
#ifdef STRATUS				/* Stratus VOS */
#define SIG_V
#else
#ifdef MAC
#define SIGTYP long
#define SIG_I
#ifndef MPW33
#define SIG_IGN 0
#endif /* MPW33 */
#define SIGALRM 1
#ifndef MPW33
#define SIGINT  2
#endif /* MPW33 */
#else /* Everything else */
#define SIG_I
#endif /* MAC */
#endif /* STRATUS */
#endif /* AIX370 */
#endif /* NEXT */
#endif /* SUNOS4 */
#endif /* SVR3 */
#endif /* POSIX */
#endif /* OS2 */
#endif /* SIG_I */
#endif /* SIG_V */

#ifdef SIG_I
#define SIGRETURN return(0)
#ifndef SIGTYP
#define SIGTYP int
#endif /* SIGTYP */
#endif /* SIG_I */

#ifdef SIG_V
#define SIGRETURN return
#ifndef SIGTYP
#define SIGTYP void
#endif /* SIGTYP */
#endif /* SIG_V */

#ifdef NT
#ifndef SIGTYP
#define SIGTYP void
#endif /* SIGTYP */
#endif /* NT */

#ifndef SIGTYP
#define SIGTYP int
#endif /* SIGTYP */

#ifndef SIGRETURN
#define SIGRETURN return(0)
#endif /* SIGRETURN */

#ifdef CKNTSIG
/* This does not work, so don't use it. */
#define signal ckntsignal
SIGTYP (*ckntsignal(int type, SIGTYP (*)(int)))(int);
#endif /* CKNTSIG */

/* We want all characters to be unsigned if the compiler supports it */

#ifdef KUI
#ifdef CHAR
#undef CHAR
#endif /* CHAR */
#define CHAR unsigned char
#else
#ifdef PROVX1
typedef char CHAR;
/* typedef long LONG; */
typedef int void;
#else
#ifdef MINIX
typedef unsigned char CHAR;
#else
#ifdef V7
typedef char CHAR;
#else
#ifdef C70
typedef char CHAR;
/* typedef long LONG; */
#else
#ifdef BSD29
typedef char CHAR;
/* typedef long LONG; */
#else
#ifdef datageneral
#define CHAR unsigned char			/* 3.22 compiler */
#else
#ifdef HPUX
#define CHAR unsigned char
#else
#ifdef OS2
#ifdef NT
#define CHAR unsigned char
#else /* NT */
#ifdef CHAR
#undef CHAR
#endif /* CHAR */
typedef unsigned char CHAR;
#endif /* NT */
#else /* OS2 */
#ifdef VMS
typedef unsigned char CHAR;
#else
#ifdef CHAR
#undef CHAR
#endif /* CHAR */
typedef unsigned char CHAR;
#endif /* VMS */
#endif /* OS2 */
#endif /* HPUX */
#endif /* datageneral */
#endif /* BSD29 */
#endif /* C70 */
#endif /* V7 */
#endif /* MINIX */
#endif /* PROVX1 */
#endif /* KUI */

union ck_short {			/* Mainly for Unicode */
    USHORT x_short;
    CHAR x_char[2];
};

#ifdef MAC				/* Macintosh file routines */
#ifndef CKWART_C			/* But not in "wart"... */
#ifdef feof
#undef feof
#endif /* feof */
#define feof mac_feof
#define rewind mac_rewind
#define fgets mac_fgets
#define fopen mac_fopen
#define fclose mac_fclose
int mac_feof();
void mac_rewind();
char *mac_fgets();
FILE *mac_fopen();
int mac_fclose();
#endif /* CKCPRO_W */
#endif /* MAC */
/*
   Systems whose mainline modules have access to the communication-line
   file descriptor, ttyfd.
*/
#ifndef CK_TTYFD
#ifdef UNIX
#define CK_TTYFD
#else
#ifdef OS2
#define CK_TTYFD
#else
#ifdef VMS
#define CK_TTYFD
#endif /* VMS */
#endif /* OS2 */
#endif /* UNIX */
#endif /* CK_TTYFD */

/* Systems where we can get our own process ID */

#ifndef CK_PID
#ifdef UNIX
#define CK_PID
#endif /* UNIX */
#ifdef OS2
#define CK_PID
#endif /* OS2 */
#ifdef VMS
#define CK_PID
#endif /* VMS */
#endif /* CK_PID */

/* Systems that support the Microsoft Telephony API (TAPI) */

#ifndef NODIAL
#ifndef CK_TAPI
#ifdef NT
#define CK_TAPI
#endif /* NT */
#endif /* CK_TAPI */
#endif /* NODIAL */

#ifndef NONZXPAND
#ifndef NZXPAND
#ifdef OS2ORUNIX
#define NZXPAND
#else
#ifdef VMS
#define NZXPAND
#else
#ifdef datageneral
#define NZXPAND
#else
#ifdef OSK
#define NZXPAND
#endif /* OSK */
#endif /* datageneral */
#endif /* VMS */
#endif /* OS2ORUNIX */
#endif /* NZXPAND */
#else
#ifdef NZXPAND
#undef NZXPAND
#endif /* NZXPAND */
#endif /* NONZXPAND */

/* nzxpand() option flags */

#define ZX_FILONLY   1			/* Match only regular files */
#define ZX_DIRONLY   2			/* Match only directories */
#define ZX_RECURSE   4			/* Descend through directory tree */
#define ZX_MATCHDOT  8			/* Match "dot files" */
#define ZX_NOBACKUP 16			/* Don't match "backup files" */
#define ZX_NOLINKS  32			/* Don't follow symlinks */

#ifndef NZXPAND
#define nzxpand(a,b) zxpand(a)
#endif /* NZXPAND */

#ifndef NOZXREWIND
#ifndef ZXREWIND			/* Platforms that have zxrewind() */
#ifdef OS2ORUNIX
#define ZXREWIND
#else
#ifdef VMS
#define ZXREWIND
#else
#ifdef datageneral
#define ZXREWIND
#else
#ifdef OSK
#define ZXREWIND
#else
#ifdef STRATUS
#define ZXREWIND
#endif /* STRATUS */
#endif /* OSK */
#endif /* datageneral */
#endif /* VMS */
#endif /* OS2ORUNIX */
#endif /* ZXREWIND */
#else
#ifdef ZXREWIND
#undef ZXREWIND
#endif /* ZXREWIND */
#endif /* NOZXREWIND */

/* Temporary-directory-for-RECEIVE feature ... */
/* This says whether we have the isdir() function defined. */

#ifdef UNIX				/* UNIX has it */
#ifndef CK_TMPDIR
#ifndef pdp11
#define CK_TMPDIR
#define TMPDIRLEN 256
#endif /* pdp11 */
#endif /* CK_TMPDIR */
#endif /* UNIX */

#ifdef VMS				/* VMS too */
#ifndef CK_TMPDIR
#define CK_TMPDIR
#define TMPDIRLEN 256
#endif /* CK_TMPDIR */
#endif /* VMS */

#ifdef OS2				/* OS two too */
#ifndef CK_TMPDIR
#define CK_TMPDIR
#define TMPDIRLEN 129
#endif /* CK_TMPDIR */
#endif /* OS2 */

#ifdef STRATUS				/* Stratus VOS too. */
#ifndef CK_TMPDIR
#define CK_TMPDIR
#define TMPDIRLEN 256
#endif /* CK_TMPDIR */
#endif /* STRATUS */

#ifdef OSK				/* OS-9 too */
#ifndef CK_TMPDIR
#define CK_TMPDIR
#define TMPDIRLEN 256
#endif /* CK_TMPDIR */
#endif /* OSK */

#ifdef datageneral			/* AOS/VS too */
#ifndef CK_TMPDIR
#define CK_TMPDIR
#define TMPDIRLEN 256
#endif /* CK_TMPDIR */
#endif /* datageneral */

#ifdef CK_TMPDIR			/* Needs command parser */
#ifdef NOICP
#undef CK_TMPDIR
#endif /* NOICP */
#endif /* CK_TMPDIR */

/* Whether to include <sys/time.h> */

#ifndef NOTIMEH				/* <time.h> */
#ifndef TIMEH
#define TIMEH
#endif /* TIMEH */
#endif /* NOTIMEH */

#ifndef NOSYSTIMEH			/* <sys/time.h> */
#ifndef SYSTIMEH
#ifdef UNIX				/* UNIX */
#ifdef SVORPOSIX			/* System V or POSIX... */
#ifdef M_UNIX
#define SYSTIMEH
#else
#ifdef SCO_32V4
#define SYSTIMEH
#else
#ifdef OXOS
#define SYSTIMEH
#else
#ifdef BSD44
#define SYSTIMEH
#else
#ifdef __linux__
#define SYSTIMEH
#else
#ifdef AIXRS
#ifndef AIX41
#define SYSTIMEH
#endif /* AIX41 */
#else
#ifdef IRIX60
#define SYSTIMEH
#else
#ifdef I386IX
#define SYSTIMEH
#else
#ifdef SV68R3V6
#define SYSTIMEH
#endif /* SV68R3V6 */
#endif /* I386IX */
#endif /* IRIX60 */
#endif /* AIXRS */
#endif /* __linux__ */
#endif /* BSD44 */
#endif /* OXOS */
#endif /* SCO_32V4 */
#endif /* M_UNIX */

#else  /* Not SVORPOSIX */

#ifndef BELLV10				/* All but these... */
#ifndef PROVX1
#ifndef V7
#ifndef BSD41
#ifndef COHERENT
#define SYSTIMEH
#endif /* COHERENT */
#endif /* BSD41 */
#endif /* V7 */
#endif /* PROVX1 */
#endif /* BELLV10 */
#endif /* SVORPOSIX */
#endif /* UNIX */
#endif /* SYSTIMEH */
#endif /* NOSYSTIMEH */

#ifndef NOSYSTIMEBH			/* <sys/timeb.h> */
#ifndef SYSTIMEBH
#ifdef OSF
#define SYSTIMEBH
#else
#ifdef COHERENT
#define SYSTIMEBH
#else
#ifdef BSD41
#define SYSTIMEBH
#else
#ifdef BSD29
#define SYSTIMEBH
#else
#ifdef TOWER1
#define SYSTIMEBH
#else
#ifdef FT21
#define SYSTIMEBH
#else
#ifdef BELLV10
#define SYSTIMEBH
#endif /* BELLV10 */
#endif /* FT21 */
#endif /* TOWER1 */
#endif /* BSD29 */
#endif /* BSD41 */
#endif /* COHERENT */
#endif /* OSF */
#endif /* SYSTIMEBH */
#endif /* NOSYSTIMEBH */

/*
 Debug and transaction logging is included automatically unless you define
 NODEBUG or NOTLOG.  Do this if you want to save the space and overhead.
 (Note, in version 4F these definitions changed from "{}" to the null string
 to avoid problems with semicolons after braces, as in: "if (x) tlog(this);
 else tlog(that);"
*/
#ifndef NODEBUG
#ifndef DEBUG
#define DEBUG
#endif /* DEBUG */
#else
#ifdef DEBUG
#undef DEBUG
#endif /* DEBUG */
#endif /* NODEBUG */

#ifdef NOTLOG
#ifdef TLOG
#undef TLOG
#endif /* TLOG */
#else  /* NOTLOG */
#ifndef TLOG
#define TLOG
#endif /* TLOG */
#endif /* NOTLOG */

/* debug() macro style selection. */

#ifdef VMS
#ifndef IFDEBUG
#define IFDEBUG
#endif /* IFDEBUG */
#endif /* VMS */

#ifdef MAC
#ifndef IFDEBUG
#define IFDEBUG
#endif /* IFDEBUG */
#endif /* MAC */

#ifdef OS2
#ifndef IFDEBUG
#define IFDEBUG
#endif /* IFDEBUG */
#endif /* OS2 */

#ifdef OXOS				/* tst is faster than jsr */
#ifndef IFDEBUG
#define IFDEBUG
#endif /* IFDEBUG */
#endif /* OXOS */

#ifndef CKCMAI
extern int deblog;
extern int debok;
extern int debxlen;
extern int matchdot;
extern int tt_bell;
#endif /* CKCMAI */

#ifdef OS2
_PROTOTYP( void bleep, (short) );
#else /* OS2 */
#define bleep(x) if(tt_bell)putchar('\07')
#endif /* OS2 */

#ifndef BEOSORBEBOX
#ifdef BEBOX				/* This was used only for DR7 */
#define BEOSORBEBOX
#else
#ifdef BEOS				/* This is used for BeOS 4.x */
#define BEOSORBEBOX
#endif /* BEOS */
#endif /* BEBOX */
#endif /* BEOSORBEBOX */

#ifdef NOICP
#ifdef TLOG
#undef TLOG
#endif /* TLOG */
#endif /* NOICP */

/* Formats for debug() and tlog() */

#define F000 0
#define F001 1
#define F010 2
#define F011 3
#define F100 4
#define F101 5
#define F110 6
#define F111 7

#ifdef __linux__
#ifndef LINUX
#define LINUX
#endif /* LINUX */
#endif /* __linux__ */

/* Platforms where small size is needed */

#ifdef pdp11
#define CK_SMALL
#endif /* pdp11 */

/* Can we use realpath()? */

#ifndef NOREALPATH
#ifdef pdp11
#define NOREALPATH
#endif /* pdp11 */
#endif /* NOREALPATH */

#ifndef NOREALPATH
#ifdef UNIX
#ifdef HPUX5
#define NOREALPATH
#else
#ifdef HPUX6
#define NOREALPATH
#else
#ifdef HPUX7
#define NOREALPATH
#else
#ifdef HPUX8
#define NOREALPATH
#else
#ifdef SV68R3V6
#define NOREALPATH
#else
#ifdef XENIX
#define NOREALPATH
#else
#ifdef CK_SCO32V4
#define NOREALPATH
#else
#ifdef CK_SCOV5
#define NOREALPATH
#else
#ifdef OSF32
#define NOREALPATH
#else
#ifdef OSF30
#define NOREALPATH
#else
#ifdef ultrix
#define NOREALPATH
#else
#ifdef COHERENT
#define NOREALPATH
#endif /* COHERENT */
#endif /* ultrix */
#endif /* OSF30 */
#endif /* OSF32 */
#endif /* CK_SCOV5 */
#endif /* CK_SCO32V4 */
#endif /* XENIX */
#endif /* SV68R3V6 */
#endif /* HPUX8 */
#endif /* HPUX7 */
#endif /* HPUX6 */
#endif /* HPUX5 */
#endif /* NOREALPATH */

#ifndef NOREALPATH
#ifndef CKREALPATH
#define CKREALPATH
#endif /* NOREALPATH */
#endif /* CKREALPATH */
#endif /* UNIX */

#ifdef CKREALPATH
#ifdef OS2ORUNIX
#ifndef CKROOT
#define CKROOT
#endif /* CKROOT */
#endif /* OS2ORUNIX */
#endif /* CKREALPATH */

/* CKSYMLINK should be set only if we can use readlink() */

#ifdef UNIX
#ifndef NOSYMLINK
#ifndef CKSYMLINK
#define CKSYMLINK
#endif /* NOSYMLINK */
#endif /* CKSYMLINK */
#endif /* UNIX */

/* Platforms where we can use lstat() instead of stat() (for symlinks) */
/* This should be set only if both lstat() and readlink() are available */

#ifndef NOLSTAT
#ifndef NOSYMLINK
#ifndef USE_LSTAT
#ifdef UNIX
#ifdef CKSYMLINK
#ifdef SVR4				/* SVR4 has lstat() */
#define USE_LSTAT
#else
#ifdef BSD42				/* 4.2BSD and 4.3BSD have it */
#define USE_LSTAT			/* This should include old HPUXs */
#else
#ifdef BSD44				/* 4.4BSD has it */
#define USE_LSTAT
#else
#ifdef LINUX				/* LINUX has it */
#define USE_LSTAT
#else
#ifdef SUNOS4				/* SunOS has it */
#define USE_LSTAT
#endif /* SUNOS4 */
#endif /* LINUX */
#endif /* BSD44 */
#endif /* BSD42 */
#endif /* SVR4 */
#endif /* CKSYMLINK */
#endif /* UNIX */
#endif /* USE_LSTAT */
#endif /* NOSYMLINK */
#endif /* NOLSTAT */

#ifdef NOLSTAT
#ifdef USE_LSTAT
#undef USE_LSTAT
#endif /* USE_LSTAT */
#endif /* NOLSTAT */

#ifndef NOTTYLOCK			/* UNIX systems that have ttylock() */
#ifndef USETTYLOCK
#ifdef AIXRS				/* AIX 3.1 and later */
#define USETTYLOCK
#else
#ifdef USE_UU_LOCK			/* FreeBSD or other with uu_lock() */
#define USETTYLOCK
#else
/*
  Prior to 8.0.299 Alpha.08 this was HAVE_BAUDBOY which was added for
  Red Hat 7.2 in May 2003 but which is no longer supported in Debian and
  OpenSuse (at least).
*/
#ifdef HAVE_LOCKDEV
#define USETTYLOCK
#endif /* HAVE_LOCKDEV */
#endif /* USE_UU_LOCK */
#endif /* AIXRS */
#endif /* USETTYLOCK */
#endif /* NOTTYLOCK */

#ifndef NO_OPENPTY			/* Can use openpty() */
#ifndef HAVE_OPENPTY
#ifdef __linux__
#define HAVE_OPENPTY
#else
#ifdef __FreeBSD__
#define HAVE_OPENPTY
#else
#ifdef __OpenBSD__
#define HAVE_OPENPTY
#else
#ifdef __NetBSD__
#define HAVE_OPENPTY
#else
#ifdef MACOSX10
#define HAVE_OPENPTY
#endif	/* MACOSX10 */
#endif	/* __NetBSD__ */
#endif	/* __OpenBSD__ */
#endif	/* __FreeBSD__ */
#endif	/* __linux__ */
#endif	/* HAVE_OPENPTY */
#endif	/* NO_OPENPTY */

/* Kermit feature selection */

#ifndef NOSPL
#ifndef NOCHANNELIO			/* Channel-based file i/o package */
#ifndef CKCHANNELIO
#ifdef UNIX
#define CKCHANNELIO
#else
#ifdef OS2
#define CKCHANNELIO
#else
#ifdef VMS
#define CKCHANNELIO
#else
#ifdef STRATUS
#define CKCHANNELIO
#endif /* STRATUS */
#endif /* VMS */
#endif /* OS2 */
#endif /* UNIX */
#endif /* CKCHANNELIO */
#endif /* NOCHANNELIO */
#endif /* NOSPL */

#ifndef NOCKEXEC			/* EXEC command */
#ifndef NOPUSH
#ifndef CKEXEC
#ifdef UNIX				/* UNIX can do it */
#define CKEXEC
#endif /* UNIX */
#endif /* CKEXEC */
#endif /* NOPUSH */
#endif /* NOCKEXEC */

#ifndef NOFAST				/* Fast Kermit protocol by default */
#ifndef CK_FAST
#ifdef UNIX
#define CK_FAST
#else
#ifdef VMS
#define CK_FAST
#else
#ifdef OS2
#define CK_FAST
#endif /* OS2 */
#endif /* VMS */
#endif /* UNIX */
#endif /* CK_FAST */
#endif /* NOFAST */

#ifdef UNIX				/* Transparent print */
#ifndef NOXPRINT
#ifndef XPRINT
#define XPRINT
#endif /* XPRINT */
#endif /* NOXPRINT */
#endif /* UNIX */

#ifndef NOHWPARITY			/* Hardware parity */
#ifndef HWPARITY
#ifdef SVORPOSIX			/* System V or POSIX can have it */
#define HWPARITY
#else
#ifdef SUNOS41				/* SunOS 4.1 can have it */
#define HWPARITY
#else
#ifdef OS2				/* K95 can have it */
#define HWPARITY
#endif /* OS2 */
#endif /* SUNOS41 */
#endif /* SVORPOSIX */
#endif /* HWPARITY */
#endif /* NOHWPARITY */

#ifndef NOSTOPBITS			/* Stop-bit selection */
#ifndef STOPBITS
#ifdef OS2ORUNIX
/* In Unix really this should only be if CSTOPB is defined. */
/* But we don't know that yet. */
#define STOPBITS
#else
#ifdef TN_COMPORT
#define STOPBITS
#endif /* TN_COMPORT */
#endif /* OS2ORUNIX */
#endif /* STOPBITS */
#endif /* NOSTOPBITS */

#ifdef UNIX
#ifndef NETCMD				/* Can SET NETWORK TYPE COMMAND */
#define NETCMD
#endif /* NETCMD */
#endif /* UNIX */

/* Pty support, nonportable, available on a case-by-case basis */

#ifndef NOPTY
#ifdef NEXT				/* NeXTSTEP (tested on 3.1)*/
#define NETPTY
#else
#ifdef CK_SCOV5				/* SCO OSR5 (tested on 5.0.5)*/
#define NETPTY
#else
#ifdef QNX				/* QNX (tested on 4.25) */
#define NETPTY
#else
#ifdef SINIX                            /* Sinix (tested on 5.42) */
#define NETPTY
#else
#ifdef DGUX540				/* DG/UX 5.4++ (tested on 5.4R4.11) */
#define NETPTY
#else
#ifdef OSF32				/* Digital Unix 3.2 */
#define NETPTY
#else
#ifdef OSF40				/* Digital Unix 4.0 / Tru64 */
#define NETPTY
#else
#ifdef IRIX60				/* IRIX 6.0 (not earlier) */
#define NETPTY
#else
#ifdef HPUX10				/* HPUX 10.00 or later */
#define NETPTY
#ifndef HAVE_PTYTRAP
#define HAVE_PTYTRAP
#endif /* HAVE_PTYTRAP */
#else
#ifdef HPUX9				/* HPUX 9.00 (not earlier) */
#define NETPTY
#ifndef HAVE_PTYTRAP
#define HAVE_PTYTRAP
#endif /* HAVE_PTYTRAP */
#else
#ifdef BSD44				/* BSD44, {Net,Free,Open}BSD */
#define NETPTY
#else
#ifdef BSDI				/* BSDI/OS (tested in 4) */
#define NETPTY
#else
#ifdef SOLARIS				/* Solaris (tested in 2.5) */
#define NETPTY
#else
#ifdef UW7				/* Unixware 7 */
#define NETPTY
#else
#ifdef SUNOS41				/* SunOS (tested in 4.1.3) */
#define NETPTY
#else
#ifdef AIX41				/* AIX 4.1 and later */
#define NETPTY
#else
#ifdef LINUX				/* Linux */
#define NETPTY
#endif /* LINUX */
#endif /* AIX41 */
#endif /* SUNOS41 */
#endif /* UW7 */
#endif /* SOLARIS */
#endif /* BSDI */
#endif /* BSD44 */
#endif /* HPUX9 */
#endif /* HPUX10 */
#endif /* IRIX60 */
#endif /* OSF40 */
#endif /* OSF32 */
#endif /* DGUX540 */
#endif /* SINIX */
#endif /* QNX */
#endif /* CK_SCOV5 */
#endif /* NEXT */

#else /* NOPTY */

#ifdef NETPTY
#undef NETPTY
#endif /* NETPTY */
#endif /* NOPTY */

#ifdef NETPTY                           /* NETCMD required for NETPTY */
#ifndef NETCMD
#define NETCMD
#endif /* NETCMD */
#endif /* NETPTY */

#ifndef CK_UTSNAME			/* Can we call uname()? */
#ifdef VMS
#define CK_UTSNAME
#else
#ifdef OS2
#define CK_UTSNAME
#else
#ifdef POSIX				/* It's in POSIX.1 */
#define CK_UTSNAME
#else
#ifdef SUNOS41				/* It's in SunOS 4.1 */
#define CK_UTSNAME
#else
#ifdef AIXRS				/* It's in AIX */
#define CK_UTSNAME
#else
#ifdef SVR4				/* It's in SVR4 (but not SVR3) */
#define CK_UTSNAME
#else
#ifdef HPUX				/* It's in HP-UX 5.00 and later */
#define CK_UTSNAME
#else
#ifdef OSF				/* It's in OSF/1 / Digital UNIX */
#define CK_UTSNAME
#else
#ifdef CK_SCOV5
#define CK_UTSNAME
#endif /* CK_SCOV5 */
#endif /* OSF */
#endif /* HPUX */
#endif /* SVR4 */
#endif /* AIXRS */
#endif /* SUNOS41 */
#endif /* POSIX */
#endif /* OS2 */
#endif /* VMS */
#endif /* CK_UTSNAME */

/* This section for anything that might use floating-point */

/* If the following causes trouble use -DFLOAT=float on the command line */

#ifdef NOSPL
#ifdef FNFLOAT
#undef FNFLOAT
#endif /* FNFLOAT */
#ifdef CKFLOAT
#undef CKFLOAT
#endif /* CKFLOAT */
#endif /* NOSPL */

#ifndef NOFLOAT

#ifndef CKFLOAT
#ifdef __alpha
/* Don't use double on 64-bit platforms -- bad things happen */
#define CKFLOAT float
#define CKFLOAT_S "float"
#else
#define CKFLOAT double
#define CKFLOAT_S "double"
#endif /* __alpha */
#endif /* CKFLOAT */

#ifndef NOGFTIMER			/* Floating-point timers */
#ifndef GFTIMER
#ifdef UNIX				/* For UNIX */
#define GFTIMER
#endif /* UNIX */
#ifdef VMS				/* VMS */
#ifndef OLD_VMS				/* 5.0 and later */
#define GFTIMER
#endif /* OLD_VMS */
#endif /* VMS */
#ifdef OS2				/* And K95 */
#define GFTIMER
#endif /* OS2 */
#ifdef STRATUS				/* And Stratus VOS */
#define GFTIMER
#endif /* STRATUS */
#endif /* GFTIMER */
#endif /* NOGFTIMER */

#ifndef NOSPL
#ifndef FNFLOAT				/* Floating-point math functions */
#ifdef VMS				/* defined by default in VMS */
#define FNFLOAT
#else
#ifdef OS2				/* and K95 */
#define FNFLOAT
#endif /* OS2 */
#endif /* VMS */
#endif /* FNFLOAT */
#endif /* NOSPL */

#else  /* NOFLOAT is defined */

#ifdef CKFLOAT
#undef CKFLOAT
#endif /* CKFLOAT */

#ifdef GFTIMER
#undef GFTIMER
#endif /* GFTIMER */

#ifdef FNFLOAT
#undef FNFLOAT
#endif /* FNFLOAT */

#endif /* NOFLOAT */

#ifdef GFTIMER				/* Fraction of second to use when */
#ifndef GFMINTIME			/* elapsed time is <= 0 */
#define GFMINTIME 0.005
#endif /* GFMINTIME */
#endif /* GFTIMER */

#ifndef CKCMAI
extern long ztmsec, ztusec;		/* Fraction of sec of current time */
#endif /* CKCMAI */

#ifndef NOUNPREFIXZERO			/* Allow unprefixing of NUL (0) */
#ifndef UNPREFIXZERO			/* in file-transfer packets */
#define UNPREFIXZERO
#endif /* UNPREFIXZERO */
#endif /* NOUNPREFIXZERO */

#ifdef CK_SMALL
#define NOCAL				/* Calibrate */
#endif /* CK_SMALL */

#ifndef NOPATTERNS			/* Filetype matching patterns */
#ifndef PATTERNS
#ifndef VMS
#ifndef CK_SMALL
#define PATTERNS
#endif /* CK_SMALL */
#endif /* VMS */
#endif /* PATTERNS */
#endif /* NOPATTERNS */

#ifndef NOCAL
#ifndef CALIBRATE
#define CALIBRATE
#endif /* CALIBRATE */
#else
#ifdef CALIBRATE
#undef CALIBRATE
#endif /* CALIBRATE */
#endif /* NOCAL */

#ifndef NORECURSE			/* Recursive directory traversal */
#ifndef RECURSIVE
#ifdef VMS
#define RECURSIVE
#else
#ifdef OS2ORUNIX
#ifndef CK_SMALL
#define RECURSIVE
#endif /* CK_SMALL */
#else
#ifdef STRATUS
#define RECURSIVE
#else
#ifdef OSK
#define RECURSIVE
#endif /* OSK */
#endif /* STRATUS */
#endif /* OS2ORUNIX */
#endif /* VMS */
#endif /* RECURSIVE */
#endif /* NORECURSE */

#ifndef CK_SMALL			/* Enable file-transfer tuning code */
#ifndef CKTUNING			/* in which more code is added */
#ifndef NOTUNING			/* to avoid function calls, etc */
#define CKTUNING
#endif /* NOTUNING */
#endif /* CKTUNING */
#endif /* CK_SMALL */

#ifndef NOURL				/* Parse URLs in SET HOST, etc */
#define CK_URL
#define NO_FTP_AUTH                     /* No auth "ftp" / "anonymous" */
#endif /* NOURL */

#ifndef NOTRIGGER
#ifndef CK_TRIGGER			/* Trigger string to exit CONNECT */
#ifdef OS2ORUNIX			/* OK for UNIX and K95 */
#define CK_TRIGGER
#else
#ifdef VMS				/* and VMS */
#define CK_TRIGGER
#else
#ifdef datageneral			/* and AOS/VS */
#define CK_TRIGGER
#endif /* datageneral */
#endif /* OS2ORUNIX */
#endif /* VMS */
#endif /* CK_TRIGGER */
#endif /* NOTRIGGER */

#ifdef CK_TRIGGER
#define TRIGGERS 8			/* How many triggers allowed */
#endif /* CK_TRIGGER */

#ifndef XLIMITS				/* CONNECT limits */
#ifdef OS2
#define XLIMITS
#endif /* OS2 */
#endif /* XLIMITS */

#ifdef NOFRILLS
#ifndef NOBROWSER
#define NOBROWSER
#endif /* NOBROWSER */
#ifndef NOFTP
#define NOFTP
#endif /* NOFTP */
#endif /* NOFRILLS */

#ifndef NOHTTP				/* HTTP features need... */
#ifdef NOICP				/* an interactive command parser */
#define NOHTTP
#endif /* NOICP */
#ifndef VMS
#ifndef OS2ORUNIX			/* K95 or UNIX (because of */
#define NOHTTP				/* time functions, time_t, etc) */
#endif /* OS2ORUNIX */
#endif /* VMS */
#endif /* NOHTTP */


#ifndef NONET
#ifdef TCPSOCKET

/* The HTTP code is not very portable, so it must be asked for with -DCKHTTP */

#ifndef NOHTTP
#ifndef CKHTTP
#ifdef SUNOS4				/* We can use it in SunOS */
#define CKHTTP
#endif /* SUNOS4 */
#ifdef SOLARIS				/* And in Solaris */
#define CKHTTP
#endif /* SOLARIS */
#ifdef LINUX				/* And Linux */
#define CKHTTP
#endif /* LINUX */
#ifdef HPUX10				/* And HP-UX 10 and above */
#define CKHTTP
#endif /* HPUX10 */
#ifdef OS2				/* And in K-95 */
#define CKHTTP
#endif /* OS2 */
#ifdef AIX41				/* In AIX 4.1 and higher */
#define CKHTTP
#endif /* AIX41 */
#ifdef UNIXWARE				/* In Unixware 2.1 and higher */
#define CKHTTP				/* and probably also in 1.x and 2.0 */
#endif /* UNIXWARE */
#ifdef CK_SCOV5
#define CKHTTP
#endif /* CK_SCOV5 */
#ifdef OSF                              /* And in OSF Digital UNIX/True 64 */
#define CKHTTP
#endif /* OSF */
#ifdef ultrix                           /* And in Ultrix Mips */
#ifdef mips
#define CKHTTP
#endif /* mips */
#endif /* ultrix */
#ifdef __NetBSD__			/* NetBSD */
#define CKHTTP
#endif	/* __NetBSD__ */
#ifdef __FreeBSD__
#define CKHTTP
#endif	/* __FreeBSD__ */
#ifdef __OpenBSD__
#define CKHTTP
#endif	/* __OpenBSD__ */
/* Add more here... */
#endif /* CKHTTP */
#ifndef CKHTTP				/* If CKHTTP not defined yet */
#define NOHTTP				/* then define NOHTTP */
#endif /* CKHTTP */
#endif /* NOHTTP */

#ifdef NETCONN				/* Special "network" types... */
#ifndef NOLOCAL
#ifdef OS2
#ifndef NETFILE
#define NETFILE
#endif /* NETFILE */
#ifndef NOPUSH
#ifndef NETCMD
#define NETCMD
#endif /* NETCMD */
#endif /* NOPUSH */
#ifdef NT
#ifndef NETDLL
#define NETDLL
#endif /* NETDLL */
#endif /* NT */
#endif /* OS2 */
#endif /* NOLOCAL */
#endif /* NETCONN */

#ifndef NOFTP
#ifndef SYSFTP
#ifndef NEWFTP
#ifdef OS2ORUNIX
#define NEWFTP
#endif /* OS2ORUNIX */
#endif /* NEWFTP */
#endif /* SYSFTP */
#endif /* NOFTP */

#ifndef NOFTP
#ifdef NEWFTP
#ifdef SYSFTP
#undef SYSFTP
#endif /* SYSFTP */
#else /* NEWFTP */
#ifndef SYSFTP
#define SYSFTP
#endif /* SYSFTP */
#endif /* NEWFTP */
#else /* NOFTP */
#ifdef NEWFTP
#undef NEWFTP
#endif /* NEWFTP */
#ifdef SYSFTP
#undef SYSFTP
#endif /* SYSFTP */
#endif /* NOFTP */

#ifndef NOBROWSER
#ifdef UNIX
#ifndef BROWSER
#ifndef NOPUSH
#define BROWSER
#endif /* NOPUSH */
#endif /* BROWSER */
#endif /* UNIX */
#ifdef OS2
#ifndef BROWSER
#ifndef NOPUSH
#define BROWSER
#endif /* NOPUSH */
#endif /* BROWSER */
#endif /* OS2 */
#else
#ifdef BROWSER
#undef BROWSER
#endif /* BROWSER */
#endif /* NOBROWSER */

#else /* TCPSOCKET */
#ifndef NOHTTP                          /* HTTP requires TCPSOCKET */
#define NOHTTP
#endif /* NOHTTP */
#endif /* TCPSOCKET */
#endif /* NONET */

#ifdef TCPSOCKET
#ifndef NOCKGETFQHOST
#ifdef __ia64__
#define NOCKGETFQHOST
#else  /* __ia64__ */
#ifdef SV68
#define NOCKGETFQHOST
#else
#ifdef HPUXPRE65
#define NOCKGETFQHOST
#endif /* HPUXPRE65 */
#endif /* SV68 */
#endif /* __ia64 */
#endif /* NOCKGETFQHOST */
/*
  Regarding System V/68 (SV68) (from Gerry Belanger, Oct 2002):

    1) The gethostbyname() appears to return the actual host IP
       address in the hostent struct, instead of the expected pointer
       to the address. Hence the bogus address in the bcopy/memcopy.
       This is despite the header agreeing with our expectations.

    2) the expected argument swap between bcopy and memcopy
       did not happen.  What grief this might cause, I know not.
*/
#endif /* TCPSOCKET */

#ifdef TCPSOCKET
#ifdef OS2ONLY
#ifndef NOSOCKS
#define NOSOCKS
#endif /* NOSOCKS */
#endif /* OS2ONLY */
#ifdef NOSOCKS
#ifdef CK_SOCKS
#undef CK_SOCKS
#endif /* CK_SOCKS */
#ifdef CK_SOCKS5
#undef CK_SOCKS5
#endif /* CK_SOCKS5 */
#else /* NOSOCKS */
#ifdef NT
#ifndef CK_SOCKS
#define CK_SOCKS
#endif /* CK_SOCKS */
#endif /* NT */
#ifdef CK_SOCKS5			/* CK_SOCKS5 implies CK_SOCKS */
#ifndef CK_SOCKS
#define CK_SOCKS
#endif /* CK_SOCKS */
#endif /* CK_SOCKS5 */
#endif /* NOSOCKS */
#endif /* TCPSOCKET */

#ifdef TNCODE
#ifndef CK_AUTHENTICATION
#ifdef OS2
#ifdef _M_PPC
#define NO_KERBEROS
#define NO_SRP
#else /* _M_PPC */
#ifndef NO_SSL
#define CK_SSL
#define SSLDLL
#endif /* NO_SSL */
#endif /* _M_PPC */
#ifndef NO_KERBEROS
#define CK_KERBEROS
#define KRB4
#define KRB5
#define KRB524
#define KRB524_CONV
#ifdef NT
#ifndef _M_PPC
#ifndef _M_ALPHA
#ifndef NO_SSL_KRB5
#define SSL_KRB5
#endif /* NO_SSL_KRB5 */
#endif /* _M_ALPHA */
#endif /* _M_PPC */
#endif /* NT */
#endif /* NO_KERBEROS */
#ifndef NO_SRP
#define CK_SRP
#endif /* NO_SRP */
#define CK_AUTHENTICATION
#endif /* OS2 */
#endif /* CK_AUTHENTICATION */

#ifdef CK_AUTHENTICATION		/* Encryption must have Auth */
#ifndef CK_ENCRYPTION
#ifndef NO_ENCRYPTION
#ifdef OS2
#define CK_ENCRYPTION
#define CK_DES
#define CK_CAST
#endif /* OS2 */
#endif /* NO_ENCRYPTION */
#endif /* CK_ENCRYPTION */
#endif /* CK_AUTHENTICATION */

#ifdef NO_AUTHENTICATION                /* Allow authentication to be */
#ifdef CK_AUTHENTICATION                /* disabled in NT and OS/2    */
#undef CK_AUTHENTICATION
#endif /* CK_AUTHENTICATION */
#ifdef CK_KERBEROS
#undef CK_KERBEROS
#endif /* CK_KERBEROS */
#ifdef CK_SRP
#undef CK_SRP
#endif /* CK_SRP */
#ifdef CK_ENCRYPTION
#undef CK_ENCRYPTION
#endif /* CK_ENCRYPTION */
#endif /* NO_AUTHENTICATION */

#ifdef NO_ENCRYPTION                    /* Allow encryption to be */
#ifdef CK_ENCRYPTION                    /* disabled in NT and OS/2 */
#undef CK_ENCRYPTION
#endif /* CK_ENCRYPTION */
#endif /* NO_ENCRYPTION */

#ifdef CK_KERBEROS      /* Disable funcs not yet supported with Heimdal */
#ifdef KRB5
#ifndef HEIMDAL
#define KRB5_U2U
#endif /* HEIMDAL */
#endif /* KRB5 */
#endif /* CK_KERBEROS */

/*
  SSH section.  NOSSH disables any form of SSH support.
  If NOSSH is not defined (or implied by NONET, NOLOCAL, etc)
  then SSHBUILTIN is defined for K95 and SSHCMD is defined for UNIX.
  Then, if either SSHBUILTIN or SSHCMD is defined, ANYSSH is also defined.
*/

#ifndef NOSSH
#ifndef NO_SSL
#ifdef OS2ONLY
#define NOSSH
#endif /* OS2ONLY */
#ifdef NT
#ifndef CK_SSL
#define NOSSH
#endif /* CK_SSL */
#endif /* NT */
#else /* NO_SSL */
#define NOSSH
#endif /* NO_SSL */
#endif /* NOSSH */

#ifdef NOSSH				/* NOSSH */
#ifdef SSHBUILTIN			/* undefines any SSH selctors */
#undef SSHBUILTIN
#endif /* SSHBUILTIN */
#ifdef SFTP_BUILTIN
#undef SFTP_BUILTIN
#endif /* SFTP_BUILTIN */
#ifdef SSHCMD
#undef SSHCMD
#endif /* SSHCMD */
#ifdef ANYSSH
#undef ANYSSH
#endif /* ANYSSH */
#else  /* Not NOSSH */
#ifndef NOLOCAL
#ifdef OS2
#ifndef SSHBUILTIN
#define SSHBUILTIN
#endif /* SSHBUILTIN */
#else  /* Not OS2 */
#ifdef UNIX
#ifndef SSHCMD
#ifdef NETPTY
#ifndef NOPUSH
#define SSHCMD
#endif /* NOPUSH */
#endif /* NETPTY */
#endif /* SSHCMD */
#endif /* UNIX */
#endif /* OS2 */
#ifndef ANYSSH
#ifdef SSHBUILTIN
#define ANYSSH
#ifdef SSHCMD
#undef SSHCMD
#endif /* SSHCMD */
#else  /* SSHBUILTIN */
#ifdef SSHCMD
#define ANYSSH
#endif /* SSHCMD */
#endif /* SSHBUILTIN */
#endif /* ANYSSH */
#endif /* NOLOCAL */
#endif /* NOSSH */

/* This is in case #ifdef SSH is used anywhere in the K95 modules */

#ifdef OS2
#ifdef SSHBUILTIN
#ifndef SSH
#define SSH
#endif /* SSH */
#endif /* SSHBUILTIN */
#endif /* OS2 */

#ifdef CK_AUTHENTICATION
#define CK_SECURITY
#else
#ifdef CK_SSL
#define CK_AUTHENTICATION
#define CK_SECURITY
#endif /* CK_SSL */
#endif /* CK_AUTHENTICATION */

/* Environment stuff */

#ifndef OS2ORUNIX
#ifndef NOPUTENV
#define NOPUTENV
#endif /* NOPUTENV */
#endif /* OS2ORUNIX */

#ifndef CK_ENVIRONMENT
#ifdef OS2
#define CK_ENVIRONMENT
#else
#ifdef UNIX
#define CK_ENVIRONMENT
#else
#ifdef STRATUS
#define CK_ENVIRONMENT
#else
#ifdef VMS
#define CK_ENVIRONMENT
#endif /* VMS */
#endif /* STRATUS */
#endif /* UNIX */
#endif /* OS2 */
#endif /* CK_ENVIRONMENT */
#ifndef NOSNDLOC			/* RFC 779 SEND LOCATION */
#ifndef CK_SNDLOC
#define CK_SNDLOC
#endif /* CK_SNDLOC */
#endif /* NOSNDLOC */
#ifndef NOXDISPLOC			/* RFC 1096 XDISPLOC */
#ifndef CK_XDISPLOC
#define CK_XDISPLOC
#endif /* CK_XDISPLOC */
#endif /* NOXDISPLOC */
#ifndef NOFORWARDX
#ifndef NOPUTENV
#ifndef NOSELECT
#ifndef CK_FORWARD_X
#ifdef CK_AUTHENTICATION
#ifndef OS2ONLY
#define CK_FORWARD_X
#endif /* OS2ONLY */
#endif /* CK_AUTHENTICATION */
#endif /* CK_FORWARD_X */
#endif /* NOSELECT */
#endif /* NOPUTENV */
#endif /* NOFORWARDX */
#ifndef NO_COMPORT
#ifdef TCPSOCKET
#ifndef TN_COMPORT
#define TN_COMPORT
#endif /* TN_COMPORT */
#endif /* TCPSOCKET */
#endif /* NO_COMPORT */
#endif /* TNCODE */

#ifndef NOXFER
#ifndef NOCTRLZ				/* Allow SET FILE EOF CTRL-Z */
#ifndef CK_CTRLZ
#ifdef OS2ORUNIX
#define CK_CTRLZ
#endif /* OS2ORUNIX */
#endif /* CK_CTRLZ */
#endif /* NOCTRLZ */
#endif /* NOXFER */

#ifndef NOPERMS				/* File permissions in A packets */
#ifndef CK_PERMS
#ifdef UNIX
#define CK_PERMS
#else
#ifdef VMS
#define CK_PERMS
#endif /* VMS */
#endif /* UNIX */
#endif /* CK_PERMS */
#endif /* NOPERMS */
#ifdef CK_PERMS
#define CK_PERMLEN 24			/* Max length of sys-dependent perms */
#endif /* CK_PERMS */

#ifdef UNIX				/* NOSETBUF for everybody */
#ifndef NOSETBUF
#ifndef USE_SETBUF			/* This is the escape clause */
#define NOSETBUF
#endif /* USE_SETBUF */
#endif /* NOSETBUF */
#endif /* UNIX */

#ifndef USE_STRERROR			/* Whether to use strerror() */
#ifdef pdp11
#define USE_STRERROR
#endif /* pdp11 */
#endif /* USE_STRERROR */

#ifdef VMS				/* Features for all VMS builds */
#ifndef NOJC
#define NOJC
#endif /* NOJC */
#ifndef NOSETBUF
#define NOSETBUF
#endif /* NOSETBUF */
#ifndef DYNAMIC
#define DYNAMIC
#endif /* DYNAMIC */
#ifndef NOCURSES
#ifndef CK_CURSES
#define CK_CURSES
#endif /* CK_CURSES */
#endif /* NOCURSES */
#endif /* VMS */

#ifndef NOCKTIMERS			/* Dynamic timeouts */
#ifndef CK_TIMERS
#define CK_TIMERS
#endif /* CK_TIMERS */
#endif /* NOCKTIMERS */

#define CK_SPEED			/* Control-prefix removal */
#ifdef NOCKSPEED
#undef CK_SPEED
#endif /* NOCKSPEED */

#ifndef NOCKXXCHAR
#ifndef CKXXCHAR
#ifdef UNIX
#define CKXXCHAR
#else
#ifdef OS2
#define CKXXCHAR
#endif /* OS2 */
#endif /* UNIX */
#endif /* CKXXCHAR */
#endif /* NOCKXXCHAR */

#ifdef MAC				/* For Macintosh, no escape */
#define NOPUSH				/* to operating system */
#endif /* MAC */

/* Systems where we can call zmkdir() to create directories. */

#ifndef CK_MKDIR
#ifndef NOMKDIR

#ifdef UNIX
#ifndef pdp11
#define CK_MKDIR
#endif /* pdp11 */
#endif /* UNIX */

#ifdef OS2
#define CK_MKDIR
#endif /* OS2 */

#ifdef VMS
#define CK_MKDIR
#endif /* VMS */

#ifdef STRATUS
#define CK_MKDIR
#endif /* STRATUS */

#ifdef OSK
#define CK_MKDIR
#endif /* OSK */

#ifdef datageneral
#define CK_MKDIR
#endif /* datageneral */

#endif /* CK_MKDIR */
#endif /* NOMKDIR */

#ifdef NOMKDIR				/* Allow for command-line override */
#ifdef CK_MKDIR
#undef CK_MKDIR
#endif /* CK_MKDIR */
#endif /* NOMKDIR */

/* Systems for which we can enable the REDIRECT command automatically */
/*   As of 6.0.193, it should work for all UNIX... */

#ifndef NOREDIRECT
#ifndef CK_REDIR
#ifdef UNIX
#define CK_REDIR
#endif /* UNIX */
#ifdef OS2				/* As well as OS/2 and friends... */
#define CK_REDIR
#endif /* OS2 */
#endif /* CK_REDIR */
#endif /* NOREDIRECT */

#ifdef NOPUSH				/* But... REDIRECT command is not */
#ifdef CK_REDIR				/*  allowed if NOPUSH is defined. */
#undef CK_REDIR
#endif /* CK_REDIR */
#ifdef NETCMD				/* Nor is SET NET COMMAND */
#undef NETCMD
#endif /* NETCMD */
#ifdef NETPTY
#undef NETPTY
#endif /* NETPTY */
#endif /* NOPUSH */

#ifndef PEXITSTAT			/* \v(pexitstat) variable defined */
#ifdef OS2ORUNIX
#define PEXITSTAT
#else
#ifdef VMS
#define PEXITSTAT
#endif /* VMS */
#endif /* OS2ORUNIX */
#endif /* PEXITSTAT */

/* The following allows automatic enabling of REDIRECT to be overridden... */

#ifdef NOREDIRECT
#ifdef NETCMD
#undef NETCMD
#endif /* NETCMD */
#ifdef NETPTY
#undef NETPTY
#endif /* NETPTY */
#ifdef CK_REDIR
#undef CK_REDIR
#endif /* CK_REDIR */
#endif /* NOREDIRECT */

#ifdef NONETCMD
#ifdef NETCMD
#undef NETCMD
#endif /* NETCMD */
#ifdef NETPTY
#undef NETPTY
#endif /* NETPTY */
#endif /* NONETCMD */

#ifdef CK_REDIR
_PROTOTYP( int ttruncmd, (char *) );
#endif /* CK_REDIR */

/* Use built-in DIRECTORY command */

#ifndef NOMYDIR
#ifndef DOMYDIR
#ifdef UNIXOROSK
#define DOMYDIR
#else
#ifdef OS2
#define DOMYDIR
#else
#ifdef VMS
#define DOMYDIR
#endif /* VMS */
#endif /* OS2 */
#endif /* UNIXOROSK */
#endif /* DOMYDIR */
#endif /* NOMYDIR */

/* Sending from and receiving to commands/pipes */

#ifndef PIPESEND
#ifdef UNIX
#define PIPESEND
#endif /* UNIX */
#ifdef OS2
#define PIPESEND
#endif /* OS2 */
#endif /* PIPESEND */

#ifdef PIPESEND
#ifdef NOPIPESEND
#undef PIPESEND
#endif /* NOPIPESEND */
#ifdef NOPUSH
#undef PIPESEND
#endif /* NOPUSH */
#endif /* PIPESEND */

#ifdef NOPUSH
#ifdef BROWSER
#undef BROWSER
#endif /* BROWSER */
#endif /* NOPUSH */

/* Versions where we support the RESEND command */

#ifndef NOXFER
#ifndef NORESEND
#ifndef CK_RESEND
#ifdef UNIX
#ifndef pdp11
#define CK_RESEND
#endif /* pdp11 */
#endif /* UNIX */

#ifdef VMS
#define CK_RESEND
#endif /* VMS */

#ifdef OS2
#define CK_RESEND
#endif /* OS2 */

#ifdef AMIGA
#define CK_RESEND
#endif /* AMIGA */

#ifdef datageneral
#define CK_RESEND
#endif /* datageneral */

#ifdef STRATUS
#define CK_RESEND
#endif /* STRATUS */

#ifdef OSK
#define CK_RESEND
#endif /* OSK */

#endif /* CK_RESEND */
#endif /* NORESEND */
#endif /* NOXFER */

/* Systems implementing "Doomsday Kermit" protocol ... */

#ifndef DOOMSDAY
#ifdef UNIX
#define DOOMSDAY
#else
#ifdef VMS
#define DOOMSDAY
#else
#ifdef OS2
#define DOOMSDAY
#else
#ifdef STRATUS
#define DOOMSDAY
#endif /* STRATUS */
#endif /* OS2 */
#endif /* VMS */
#endif /* UNIX */
#endif /* DOOMSDAY */

/* Systems where we want the Thermometer to be used for fullscreen */

#ifdef OS2
#ifndef CK_PCT_BAR
#define CK_PCT_BAR
#endif /* CK_PCT_BAR */
#endif /* OS2 */

/* Systems where we have a REXX command */

#ifdef OS2
#ifdef __32BIT__
#ifndef NOREXX
#define CK_REXX
#endif /* NOREXX */
#endif /* __32BIT__ */
#endif /* OS2 */

/* Platforms that have a ZCHKPID function */

#ifdef OS2ORUNIX
#define ZCHKPID
#endif /* OS2ORUNIX */

#ifndef ZCHKPID
/* If we can't check pids then we have treat all pids as active & valid. */
#define zchkpid(x) 1
#endif /* ZCHKPID */

/* Systems that have a ZRENAME function */

#define ZRENAME				/* They all do */

/* Systems that have a ZCOPY function */

#ifndef ZCOPY
#ifdef VMS
#define ZCOPY
#else
#ifdef OS2
#define ZCOPY
#else
#ifdef UNIX
#define ZCOPY
#else
#ifdef STRATUS
#define ZCOPY
#endif /* STRATUS */
#endif /* UNIX */
#endif /* OS2 */
#endif /* VMS */
#endif /* ZCOPY */

/* Systems that have ttgwsiz() (they all should but they don't) */

#ifndef NOTTGWSIZ
#ifndef CK_TTGWSIZ
#ifdef UNIX
#define CK_TTGWSIZ
#else
#ifdef VMS
#define CK_TTGWSIZ
#else
#ifdef OS2
#define CK_TTGWSIZ
#else
#ifdef OSK
#define CK_TTGWSIZ
#endif /* OSK */
#endif /* OS2 */
#endif /* VMS */
#endif /* UNIX */
#endif /* CK_TTGWSIZ */
#endif /* NOTTGWSIZ */

#ifdef NOTTGWSIZ
#ifdef CK_TTGWSIZ
#undef CK_TTGWSIZ
#endif /* CK_TTGWSIZ */
#endif /* NOTTGWSIZ */

#ifdef OS2
/* OS/2 C-Kermit features not available in 16-bit version... */

#ifdef OS2ONLY
#ifndef __32BIT__
#ifndef NOLOCAL
#ifdef PCFONTS				/* PC Font support */
#undef PCFONTS
#endif /* PCFONTS */
#ifdef NPIPE				/* Named Pipes communication */
#undef NPIPE
#endif /* NPIPE */
#ifdef CK_NETBIOS			/* NETBIOS communication */
#undef CK_NETBIOS
#endif /* CK_NETBIOS */
#ifdef OS2MOUSE				/* Mouse */
#undef OS2MOUSE
#endif /* OS2MOUSE */
#ifdef OS2PM				/* Presentation Manager */
#undef OS2PM
#endif /* OS2PM */
#endif /* NOLOCAL */
#ifdef CK_REXX				/* Rexx */
#undef CK_REXX
#endif /* CK_REXX */
#endif /* __32BIT__ */
#endif /* OS2ONLY */

/* OS/2 C-Kermit features not available in Windows NT version... */

#ifdef NT
#ifdef PCFONTS				/* PC Font support */
#undef PCFONTS
#endif /* PCFONTS */
#ifdef OS2PM				/* Presentation Manager */
#undef OS2PM
#endif /* OS2PM */
#ifdef CK_REXX				/* Rexx */
#undef CK_REXX
#endif /* CK_REXX */
#endif /* NT */
#endif /* OS2 */

/*
  Systems that have select().
  This is used for both msleep() and for read-buffer checking in in_chk().
*/
#define CK_SLEEPINT 250 /* milliseconds - set this to something that
                           divides evenly into 1000 */
#ifndef SELECT
#ifndef NOSELECT
#ifdef __linux__
#define SELECT
#else
#ifdef SUNOS4
#define SELECT
#else
#ifdef NEXT
#define SELECT
#else
#ifdef RTAIX
#define SELECT
#else
#ifdef HPUX
/*
  Not really.  I think it's only in HP-UX 7.0 and later, except it's also
  in earlier versions that have TCP/IP installed.  Override this default
  in particular HP-UX makefile entries by adding -DNOSELECT, as in (e.g.)
  the HP-UX 6.5 ones.
*/
#define SELECT
#else
#ifdef AIXRS
#define SELECT
#else
#ifdef BSD44
#define SELECT
#else
#ifdef BSD4
#define SELECT
#else
#ifdef OXOS
#define SELECT
#else
#ifdef OS2
#define SELECT
#else
#ifdef BEBOX
#define SELECT
#endif /* BEBOX */
#endif /* OS2 */
#endif /* OXOS */
#endif /* BSD4 */
#endif /* BSD44 */
#endif /* AIXRS */
#endif /* HPUX */
#endif /* RTAIX */
#endif /* NEXT */
#endif /* __linux__ */
#endif /* SUNOS4 */
#endif /* NOSELECT */
#endif /* SELECT */

/*
  The following section moved here from ckcnet.h in 6.1 because select()
  is now used for non-networking purposes.
*/

/* On HP-9000/500 HP-UX 5.21 this stuff is not defined in any header file */

#ifdef hp9000s500
#ifndef NEEDSELECTDEFS
#define NEEDSELECTDEFS
#endif /* NEEDSELECTDEFS */
#endif /* hp9000s500 */

#ifdef NEEDSELECTDEFS
typedef long fd_mask;
#ifndef NBBY
#define NBBY 8
#endif /* NBBY */
#ifndef FD_SETSIZE
#define FD_SETSIZE 32
#endif /* FD_SETSIZE */
#ifndef NFDBITS
#define NFDBITS (sizeof(fd_mask) * NBBY)
#endif /* NFDBITS */
#ifndef howmany
#define howmany(x,y) (((x)+((y)-1))/(y))
#endif /* howmany */
typedef struct fd_set {
    fd_mask fds_bits[howmany(FD_SETSIZE, NFDBITS)];
} fd_set;
#ifndef FD_SET
#define FD_SET(n,p) ((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#endif /* FD_SET */
#ifndef FD_CLR
#define FD_CLR(n,p) ((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#endif /* FD_CLR */
#ifndef FD_ISSET
#define FD_ISSET(n,p) ((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#endif /* FD_ISSET */
#ifndef FD_COPY
#define FD_COPY(f,t) (bcopy(f,t,sizeof(*(f)))
#endif /* FD_COPY */
#ifndef FD_ZERO
#define FD_ZERO(p) bzero((char *)(p),sizeof(*(p)))
#endif /* FD_ZERO */
#endif /* NEEDSELECTDEFS */

/*
  CK_NEED_SIG is defined if the system cannot check the console to
  to see if characters are waiting.  This is used during local-mode file
  transfer to interrupt the transfer, refresh the screen display, etc.
  If CK_NEED_SIG is defined, then file-transfer interruption characters
  have to be preceded a special character, e.g. the SIGQUIT character.
  CK_NEED_SIG should be defined if the conchk() function is not operational.
*/
#ifdef NOPOLL				/* For overriding CK_POLL definition */
#ifdef CK_POLL
#undef CK_POLL
#endif /* CK_POLL */
#endif /* NOPOLL */

#ifndef CK_POLL				/* If we don't have poll() */
#ifndef RDCHK				/* And we don't have rdchk() */
#ifndef SELECT				/* And we don't have select() */
#ifdef ATTSV
#ifndef aegis
#ifndef datageneral
#ifndef OXOS
#define CK_NEED_SIG
#endif /* OXOS */
#endif /* datageneral */
#endif /* aegis */
#endif /* ATTSV */
#ifdef POSIX
#ifndef CK_NEED_SIG
#define CK_NEED_SIG
#endif /* CK_NEED_SIG */
#endif /* POSIX */
#endif /* SELECT */
#endif /* RDCHK */
#endif /* CK_POLL */

#ifdef HPUX				/* HP-UX has select() */
#ifdef CK_NEED_SIG
#undef CK_NEED_SIG
#endif /* CK_NEED_SIG */
#endif /* HPUX */

#ifdef AIXRS				/* AIX has select() */
#ifdef CK_NEED_SIG
#undef CK_NEED_SIG
#endif /* CK_NEED_SIG */
#endif /* AIXRS */

#ifdef BSD44				/* 4.4BSD has FIONREAD */
#ifdef CK_NEED_SIG
#undef CK_NEED_SIG
#endif /* CK_NEED_SIG */
#endif /* BSD44 */

#ifdef QNX				/* QNX has FIONREAD and select() */
#ifdef CK_NEED_SIG
#undef CK_NEED_SIG
#endif /* CK_NEED_SIG */
#endif /* QNX */

#ifdef COHERENT
#ifndef NOTIMEZONE
#define NOTIMEZONE
#endif /* NOTIMEZONE */
#endif /* COHERENT */

#ifdef UNIX
#ifndef HAVE_TZ				/* Can we use struct timezone? */
#ifndef NOTIMEZONE
#ifdef PTX
#define NOTIMEZONE
#else
#ifndef SELECT
#ifdef COHERENT
#define NOTIMEZONE
#else
#ifdef BELLV10
#define NOTIMEZONE
#endif /* BELLV10 */
#endif /* COHERENT */
#endif /* SELECT */
#endif /* PTX */
#endif /* NOTIMEZONE */
#endif /* HAVE_TZ */
#ifndef NOTIMEVAL			/* Can we use struct timeval? */
#ifndef HAVE_TV
#define HAVE_TV
#endif /* HAVE_TV */
#endif /* NOTIMEVAL */
#ifndef NOTIMEZONE
#ifndef HAVE_TZ
#define HAVE_TZ
#endif /* HAVE_TZ */
#endif /* NOTIMEZONE */
#endif /* UNIX */

#ifdef SCO32
#ifdef HAVE_TV
#undef HAVE_TV
#endif /* HAVE_TV */
#ifdef HAVE_TZ
#undef HAVE_TZ
#endif /* HAVE_TZ */
#ifndef NOTIMEVAL
#define NOTIMEVAL
#endif /* NOTIMEVAL */
#ifndef NOTIMEZONE
#define NOTIMEZONE
#endif /* NOTIMEZONE */
#endif /* SCO32 */

#ifdef ATT7300
#ifdef HAVE_TV
#undef HAVE_TV
#endif /* HAVE_TV */
#ifdef HAVE_TZ
#undef HAVE_TZ
#endif /* HAVE_TZ */
#ifndef NOTIMEVAL
#define NOTIMEVAL
#endif /* NOTIMEVAL */
#ifndef NOTIMEZONE
#define NOTIMEZONE
#endif /* NOTIMEZONE */
#endif /* ATT7300 */

/*
  Automatic parity detection.
  This actually implies a lot more now: length-driven packet reading,
  "Doomsday Kermit" IBM Mainframe file transfer through 3270 data streams, etc.
*/
#ifdef UNIX				/* For Unix */
#ifndef NOPARSEN
#define PARSENSE
#endif /* NOPARSEN */
#endif /* UNIX */

#ifdef VMS				/* ... and VMS */
#ifndef NOPARSEN
#define PARSENSE
#endif /* NOPARSEN */
#ifdef __GNUC__
#define VMSGCC
#endif /* __GNUC__ */
#endif /* VMS */

#ifdef MAC				/* and Macintosh */
#ifndef NOPARSEN
#define PARSENSE
#endif /* NOPARSEN */
#endif /* MAC */

#ifdef STRATUS				/* and Stratus VOS */
#ifndef NOPARSEN
#define PARSENSE
#endif /* NOPARSEN */
#endif /* STRATUS */

#ifdef OS2				/* and OS/2, finally */
#ifndef NOPARSEN
#define PARSENSE
#endif /* NOPARSEN */
#endif /* OS2 */

#ifndef NODYNAMIC			/* DYNAMIC is default for UNIX */
#ifndef DYNAMIC				/* as of C-Kermit 7.0 */
#ifdef UNIX
#define DYNAMIC
#endif /* UNIX */
#endif /* DYNAMIC */
#endif /* NODYNAMIC */

#ifdef DYNAMIC				/* If DYNAMIC is defined */
#define DCMDBUF				/* then also define this. */
#endif /* DYNAMIC */

#ifndef CK_LBRK				/* Can send Long BREAK */

#ifdef UNIX				/* (everybody but OS-9) */
#define CK_LBRK
#endif /* UNIX */
#ifdef VMS
#define CK_LBRK
#endif /* VMS */
#ifdef datageneral
#define CK_LBRK
#endif /* datageneral */
#ifdef GEMDOS
#define CK_LBRK
#endif /* GEMDOS */
#ifdef OS2
#define CK_LBRK
#endif /* OS2 */
#ifdef AMIGA
#define CK_LBRK
#endif /* AMIGA */
#ifdef STRATUS
#define CK_LBRK
#endif /* STRATUS */

#endif /* CK_LBRK */

/* Carrier treatment */
/* These are defined here because they are shared by the system dependent */
/* and the system independent modules. */

#define  CAR_OFF 0	/* Off: ignore carrier always. */
#define  CAR_ON  1      /* On: heed carrier always, except during DIAL. */
#define  CAR_AUT 2      /* Auto: heed carrier, but only if line is declared */
			/* to be a modem line, and only during CONNECT. */

/* And more generically (for use with any ON/OFF/AUTO feature) */
#define  CK_OFF  0
#define  CK_ON   1
#define  CK_AUTO 2

#ifndef NOLOCAL
/*
  Serial interface speeds available.

  As of C-Kermit 6.1 there is a new method to get the supported
  speeds, which obviates the need for all the craziness below.  At runtime,
  just call the new ttspdlist() routine to get a list of supported speeds.
  Then the user interface module can build a keyword table or menu from it.
*/
#ifndef TTSPDLIST
#ifdef UNIX				/* For now, only for UNIX */
#ifndef OLINUXHISPEED			/* But not systems with hacks for */
#ifndef MINIX				/* high speeds, like 110 = 115200 */
#define TTSPDLIST
#endif /* MINIX */
#endif /* OLINUXHISPEED */
#else
#ifdef VMS
#define TTSPDLIST			/* VMS gets it too */
#endif /* VMS */
#endif /* UNIX */
#endif /* TTSPDLIST */

#ifndef NODIAL				/* Hangup by modem command */
#ifndef NOMDMHUP
#ifndef MDMHUP
#define MDMHUP
#endif /* MDMHUP */
#endif /* NOMDMHUP */
#endif /* NODIAL */

#ifdef NOSPL
#ifndef NOLOGDIAL			/* Connection log needs mjd(), etc. */
#define NOLOGDIAL
#endif /* NOLOGDIAL */
#endif /* NOSPL */

#ifdef pdp11
#define NOLOGDIAL
#endif /* pdp11 */

#ifndef NOLOGDIAL			/* Connection log */
#ifndef CXLOGFILE
#define CXLOGFILE "CX.LOG"		/* Default connection log file name */
#endif /* CXLOGFILE */
#ifndef CKLOGDIAL
#ifndef CK_SMALL
#define CKLOGDIAL
#define CXLOGBUFL 1024			/* Connection log record buffer size */
#endif /* CK_SMALL */
#endif /* NOLOGDIAL */
#endif /* CKLOGDIAL */

#endif /* NOLOCAL */

#ifdef NOTTSPDLIST			/* Except if NOTTSPDLIST is defined */
#ifdef TTSPDLIST
#undef TTSPDLIST
#endif /* TTSPDLIST */
#endif /* NOTTSPDLIST */

#ifdef TTSPDLIST

_PROTOTYP( long * ttspdlist, (void) );

#else /* TTSPDLIST not defined */
/*
  We must use a long and convoluted series of #ifdefs that have to be kept in
  sync with the code in the ck?tio.c module.

  We assume that everybody supports: 0, 110, 300, 600, 1200, 2400, 4800, and
  9600 bps.  Symbols for other speeds are defined here.  You can also add
  definitions on the CC command lines.  These definitions affect the SET SPEED
  keyword table, and are not necessarily usable in the system-dependent
  speed-setting code in the ck?tio.c modules, which depends on system-specific
  symbols like (in UNIX) B19200.  In other words, just defining it doesn't
  mean it'll work -- you also have to supply the supporting code in ttsspd()
  and ttgspd() in ck?tio.c.

  The symbols have the form BPS_xxxx, where xxxx is the speed in bits per
  second, or (for bps values larger than 9999) thousands of bps followed by K.
  The total symbol length should be 8 characters or less.  Some values are
  enabled automatically below.  You can disable a particular value by defining
  NOB_xxxx on the CC command line.

*/

#ifndef NOB_50
#define BPS_50				/* 50 bps */
#endif

#ifndef NOB_75
#define BPS_75				/* 75 bps */
#endif

#ifndef NOB7512
#ifdef ANYBSD
#define BPS_7512			/* 75/1200 Split Speed */
#endif /* ANYBSD */
#endif /* NOB7512 */

#ifndef NOB134
#ifdef SOLARIS25
#define BPS_134
#else
#undef BPS_134				/* 134.5 bps (IBM 2741) */
#endif /* BPS_134 */
#endif /* NOB134 */

#ifndef NOB_150
#define BPS_150				/* 150 bps */
#endif

#ifndef NOB_200
#define BPS_200				/* 200 bps */
#endif

#ifndef NOB_1800
#ifdef MAC
#define BPS_1800			/* 1800 bps */
#else
#ifdef SOLARIS25
#define BPS_1800
#endif
#endif
#endif

#ifndef NOB_3600
#ifndef SOLARIS25
#define BPS_3600			/* 3600 bps */
#endif
#endif

#ifndef NOB_7200
#ifndef SOLARIS25
#define BPS_7200			/* 7200 bps */
#endif /* SOLARIS25 */
#endif

#ifndef NOB_14K
#ifdef BSD44
#define BPS_14K				/* 14400 bps */
#else
#ifdef OS2
#define BPS_14K
#else
#ifdef NEXT
#define BPS_14K
#else
#ifdef MAC
#define BPS_14K
#else
#ifdef AMIGA
#define BPS_14K
#endif /* AMIGA */
#endif /* MAC */
#endif /* NEXT */
#endif /* OS2 */
#endif /* BSD44 */
#endif /* NOB_14K */

#ifndef NOB_19K
#define BPS_19K				/* 19200 bps */
#endif

#ifndef NOB_28K
#ifdef BSD44
#define BPS_28K
#else
#ifdef OS2
#define BPS_28K
#else
#ifdef NEXT
#define BPS_28K				/* 28800 bps */
#else
#ifdef MAC
#define BPS_28K				/* 28800 bps */
#endif /* MAC */
#endif /* NEXT */
#endif /* OS2 */
#endif /* BSD44 */
#endif /* NOB_28K */

#ifndef NOB_38K
#define BPS_38K				/* 38400 bps */
#endif

#ifndef NOB_57K
#ifdef Plan9
#define BPS_57K
#else
#ifdef SOLARIS25
#define BPS_57K
#else
#ifdef VMS
#define BPS_57K				/* 57600 bps */
#else
#ifdef OS2
#define BPS_57K
#else
#ifdef __linux__
#define BPS_57K
#else
#ifdef HPUX
#define BPS_57K
#else
#ifdef NEXT
#define BPS_57K
#else
#ifdef __386BSD__
#define BPS_57K
#else
#ifdef __FreeBSD__
#define BPS_57K
#else
#ifdef __NetBSD__
#define BPS_57K
#else
#ifdef MAC
#define BPS_57K
#else
#ifdef QNX
#define BPS_57K
#else
#ifdef BEOSORBEBOX
#define BPS_57K
#else
#ifdef IRIX62
#define BPS_57K
#else
#ifdef SCO_OSR504
#define BPS_57K
#else
#ifdef BSDI2
#define BPS_57K
#endif /* BSDI2 */
#endif /* SCO_OSR504 */
#endif /* IRIX62 */
#endif /* BEOSORBEBOX */
#endif /* QNX */
#endif /* MAC */
#endif /* __NetBSD__ */
#endif /* __FreeBSD__ */
#endif /* __386BSD__ */
#endif /* NEXT */
#endif /* HPUX */
#endif /* __linux__ */
#endif /* OS2 */
#endif /* VMS */
#endif /* SOLARIS25 */
#endif /* Plan9 */
#endif /* NOB_57K */

#ifndef NOB_76K
#ifdef BSDI2
#define BPS_76K
#endif /* BSDI2 */
#ifdef Plan9
#define BPS_76K
#endif /* Plan9 */
#ifdef SOLARIS25
#define BPS_76K
#endif /* SOLARIS25 */
#ifdef VMS
#define BPS_76K				/* 76800 bps */
#endif /* VMS */
#ifdef OS2
#ifdef __32BIT__
#define BPS_76K
#endif /* __32BIT__ */
#endif /* OS2 */
#ifdef QNX
#define BPS_76K
#endif /* QNX */
#ifdef IRIX62
#define BPS_76K
#endif /* IRIX62 */
#ifdef SCO_OSR504
#define BPS_76K
#endif /* SCO_OSR504 */
#endif /* NOB_76K */

#ifndef NOB_115K
#ifdef BSDI2
#define BPS_115K
#endif /* BSDI2 */
#ifdef Plan9
#define BPS_115K
#endif /* Plan9 */
#ifdef SOLARIS25
#define BPS_115K
#endif /* SOLARIS25 */
#ifdef VMS
#define BPS_115K			/* 115200 bps */
#else
#ifdef QNX
#define BPS_115K
#else
#ifdef HPUX
#define BPS_115K
#else
#ifdef __linux__
#define BPS_115K
#else
#ifdef __386BSD__
#define BPS_115K
#else
#ifdef __FreeBSD__
#define BPS_115K
#else
#ifdef __NetBSD__
#define BPS_115K
#else
#ifdef OS2
#ifdef __32BIT__
#define BPS_115K
#endif /* __32BIT__ */
#else
#ifdef BEOSORBEBOX
#define BPS_115K
#else
#ifdef IRIX62
#define BPS_115K
#else
#ifdef SCO_OSR504
#define BPS_115K
#endif /* SCO_OSR504 */
#endif /* IRIX62 */
#endif /* BEOSORBEBOX */
#endif /* OS2 */
#endif /* __NetBSD__ */
#endif /* __FreeBSD__ */
#endif /* __386BSD__ */
#endif /* __linux__ */
#endif /* HPUX */
#endif /* QNX */
#endif /* VMS */
#endif /* NOB_115K */

#ifndef NOB_230K			/* 230400 bps */
#ifdef BSDI2
#define BPS_230K
#else
#ifdef SCO_OSR504
#define BPS_230K
#else
#ifdef __linux__
#define BPS_230K
#else
#ifdef SOLARIS25
#define BPS_230K
#else
#ifdef OS2
#ifdef __32BIT__
#define BPS_230K
#endif /* __32BIT__ */
#else
#undef BPS_230K
#endif /* OS2 */
#endif /* SOLARIS25 */
#endif /* __linux__ */
#endif /* SCO_OSR504 */
#endif /* BSDI2 */
#endif /* NOB_230K */

#ifndef NOB_460K			/* 460800 bps */
#ifdef SCO_OSR504
#define BPS_460K
#else
#ifdef __linux__
#define BPS_460K
#else
#ifdef OS2
#ifdef __32BIT__
#define BPS_460K
#endif /* __32BIT__ */
#else
#undef BPS_460K
#endif /* __linux__ */
#endif /* SCO_OSR504 */
#endif /* OS2 */
#endif /* NOB_460K */

#ifndef NOB_921K			/* 921600 bps */
#ifdef SCO_OSR504
#define BPS_921K
#endif /* SCO_OSR504 */
#endif /* NOB_921K */

#ifdef BPS_921K				/* Maximum speed defined */
#define MAX_SPD 921600L
#else
#ifdef BPS_460K
#define MAX_SPD 460800L
#else
#ifdef BPS_230K
#define MAX_SPD 230400L
#else
#ifdef BPS_115K
#define MAX_SPD 115200L
#else
#ifdef BPS_76K
#define MAX_SPD 76800L
#else
#ifdef BPS_57K
#define MAX_SPD 57600L
#else
#ifdef BPS_38K
#define MAX_SPD 38400L
#else
#ifdef BPS_28K
#define MAX_SPD 28800L
#else
#ifdef BPS_19K
#define MAX_SPD 19200L
#else
#ifdef BPS_14K
#define MAX_SPD 14400L
#else
#define MAX_SPD 9600L
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif /* TTSPDLIST */

#ifndef CONGSPD				/* Systems that can call congspd() */
#ifdef UNIX
#define CONGSPD
#endif /* UNIX */
#ifdef VMS
#define CONGSPD
#endif /* VMS */
#ifdef STRATUS
#define CONGSPD
#endif /* STRATUS */
#endif /* CONGSPD */

/* Types of flow control available */

#define CK_XONXOFF			/* Everybody can do this, right? */

#ifdef AMIGA				/* Commodore Amiga */
#define CK_RTSCTS			/* has RTS/CTS */
#endif /* AMIGA */

#ifdef SUN4S5				/* SunOS in System V environment */
#define CK_RTSCTS
#else					/* SunOS 4.0/4.1 in BSD environment */
#ifdef SUNOS4				/* SunOS 4.0+later supports RTS/CTS */
#ifdef SUNOS41				/* Easy in 4.1 and later */
#define CK_RTSCTS
#else					/* Harder in 4.0 */
#ifndef __GNUC__			/* (see tthflow() in ckutio.c) */
#ifndef GNUC
#define CK_RTSCTS			/* Only if not using GNU gcc */
#endif /* __GNUC__ */
#endif /* GNUC */
#endif /* SUNOS41 */
#endif /* SUNOS4 */
#endif /* SUN4S5 */

#ifdef BSD44				/* And in 4.4 BSD, including BSDI */
#define CK_RTSCTS
#endif /* BSD44 */

#ifdef TERMIOX				/* Sys V R4 <termiox.h> */
#ifndef CK_RTSCTS
#define CK_RTSCTS
#endif /* CK_RTSCTS */
#ifndef CK_DTRCD
#define CK_DTRCD
#endif /* CK_DTRCD */
#else
#ifdef STERMIOX				/* Sys V R4 <sys/termiox.h> */
#ifndef CK_RTSCTS
#define CK_RTSCTS
#endif /* CK_RTSCTS */
#ifndef CK_DTRCD
#define CK_DTRCD
#endif /* CK_DTRCD */
#endif /* STERMIOX */
#endif /* TERMIOX */

#ifdef OXOS				/* Olivetti X/OS R2 struct termios */
#define CK_RTSCTS			/* Ditto. */
#define CK_DTRCD
#endif /* OXOS */

#ifdef AIXRS				/* RS/6000 with AIX 3.x */
#define CK_RTSCTS			/* Has its own peculiar method... */
#endif /* AIXRS */

#ifdef __linux__			/* Linux */
#define CK_RTSCTS
#endif /* __linux__ */
/*
  Hardware flow control is not defined in POSIX.1.  Nevertheless, a certain
  style API for hardware flow control, using tcsetattr() and the CRTSCTS
  bit(s), seems to be gaining currency on POSIX-based UNIX systems.  The
  following code defines the symbol POSIX_CRTSCTS for such systems.
*/
#ifdef CK_RTSCTS
#ifdef __bsdi__				/* BSDI, a.k.a. BSD/386 */
#define POSIX_CRTSCTS
#endif /* __bsdi__ */
#ifdef __linux__			/* Linux */
#define POSIX_CRTSCTS
#endif /* __linux__ */
#ifdef __NetBSD__			/* NetBSD */
#define POSIX_CRTSCTS
#endif /* __NetBSD__ */
#ifdef __OpenBSD__
#define POSIX_CRTSCTS
#endif /* __OpenBSD__ */
#ifdef BEOSORBEBOX			/* BeBOX */
#define POSIX_CRTSCTS
/* BEBOX defines CRTSFL as (CTSFLOW & RTSFLOW) */
#define CRTSCTS CRTSFL
#endif /* BEOSORBEBOX */
#ifdef IRIX52				/* IRIX 5.2 and later */
#define POSIX_CRTSCTS
#define CRTSCTS CNEW_RTSCTS		/* See <sys/termios.h> */
#endif /* IRIX52 */
#endif /* CK_RTSCTS */

/* Implementations that have implemented the ttsetflow() function. */

#ifndef CK_TTSETFLOW
#ifdef UNIX
#define CK_TTSETFLOW
#endif /* UNIX */
#ifdef OS2
#define CK_TTSETFLOW
#endif /* OS2 */
#endif /* CK_TTSETFLOW */

#ifdef CK_TTSETFLOW
_PROTOTYP( int ttsetflow, (int) );
#endif /* CK_TTSETFLOW */
/*
 Systems where we can expand tilde at the beginning of file or directory names
*/
#ifdef POSIX
#ifndef DTILDE
#define DTILDE
#endif /* DTILDE */
#endif /* POSIX */
#ifdef BSD4
#ifndef DTILDE
#define DTILDE
#endif /* DTILDE */
#endif /* BSD4 */
#ifdef ATTSV
#ifndef DTILDE
#define DTILDE
#endif /* DTILDE */
#endif /* ATTSV */
#ifdef OSK
#ifndef DTILDE
#define DTILDE
#endif /* DTILDE */
#endif /* OSK */
#ifdef HPUX				/* I don't know why this is */
#ifndef DTILDE				/* necessary, since -DHPUX */
#define DTILDE				/* automatically defines ATTSV */
#endif /* DTILDE */			/* (see above) ... */
#endif /* HPUX */

/*
  This is mainly for the benefit of ckufio.c (UNIX and OS/2 file support).
  Systems that have an atomic rename() function, so we don't have to use
  link() and unlink().
*/
#ifdef POSIX
#ifndef RENAME
#define RENAME
#endif /* RENAME */
#endif /* POSIX */

#ifdef OS2
#ifndef RENAME
#define RENAME
#endif /* RENAME */
#endif /* OS2 */

#ifdef SUNOS41
#ifndef RENAME
#define RENAME
#endif /* RENAME */
#endif /* SUNOS41 */

#ifdef SVR4
#ifndef RENAME
#define RENAME
#endif /* RENAME */
#endif /* SVR4 */

#ifdef AIXRS
#ifndef RENAME
#define RENAME
#endif /* RENAME */
#endif /* AIXRS */

#ifdef BSD44
#ifndef RENAME
#define RENAME
#endif /* RENAME */
#endif /* BSD44 */

#ifdef NORENAME				/* Allow for compile-time override */
#ifdef RENAME
#undef RENAME
#endif /* RENAME */
#endif /* NORENAME */

#ifdef STRATUS				/* Stratus VOS */
#ifndef RENAME
#define RENAME
#endif /* RENAME */
#endif /* STRATUS */

/* Line delimiter for text files */

/*
 If the system uses a single character for text file line delimitation,
 define NLCHAR to the value of that character.  For text files, that
 character will be converted to CRLF upon output, and CRLF will be converted
 to that character on input during text-mode (default) packet operations.
*/
#ifdef MAC                              /* Macintosh */
#define NLCHAR 015
#else
#ifdef OSK				/* OS-9/68K */
#define NLCHAR 015
#else                                   /* All Unix-like systems */
#define NLCHAR 012
#endif /* OSK */
#endif /* MAC */

/*
 At this point, if there's a system that uses ordinary CRLF line
 delimitation AND the C compiler actually returns both the CR and
 the LF when doing input from a file, then #undef NLCHAR.
*/
#ifdef OS2				/* OS/2 */
#undef NLCHAR
#endif /* OS2 */

#ifdef GEMDOS				/* Atari ST */
#undef NLCHAR
#endif /* GEMDOS */

/*
  VMS file formats are so complicated we need to do all the conversion
  work in the CKVFIO module, so we tell the rest of C-Kermit not to fiddle
  with the bytes.
*/

#ifdef vms
#undef NLCHAR
#endif /* vms */

/* The device name of a job's controlling terminal */
/* Special for VMS, same for all Unixes (?), not used by Macintosh */

#ifdef BEOS
#define CTTNAM dftty
#else
#ifdef vms
#define CTTNAM "SYS$INPUT:"		/* (4 Jan 2002) Was TT: */
#else
#ifdef datageneral
#define CTTNAM "@output"
#else
#ifdef OSK
extern char myttystr[];
#define CTTNAM myttystr
#else
#ifdef OS2
#define CTTNAM "con"
#else
#ifdef UNIX
#define CTTNAM "/dev/tty"
#else
#ifdef GEMDOS
#define CTTNAM "aux:"
#else
#ifdef STRATUS
extern char myttystr[];
#define CTTNAM myttystr
#else /* Anyone else... */
#define CTTNAM "stdout"			/* This is a kludge used by Mac */
#endif /* STRATUS */
#endif /* GEMDOS */
#endif /* UNIX */
#endif /* OS2 */
#endif /* OSK */
#endif /* datageneral */
#endif /* vms */
#endif /* BEOS */

#ifndef HAVECTTNAM
#ifdef UNIX
#define HAVECTTNAM
#else
#ifdef VMS
#define HAVECTTNAM
#endif /* VMS */
#endif /* UNIX */
#endif /* HAVECTTNAM */

#ifndef ZFCDAT				/* zfcdat() function available? */
#ifdef UNIX
#define  ZFCDAT
#else
#ifdef STRATUS
#define  ZFCDAT
#else
#ifdef GEMDOS
#define  ZFCDAT
#else
#ifdef AMIGA
#define  ZFCDAT
#else
#ifdef OS2
#define  ZFCDAT
#else
#ifdef datageneral
#define  ZFCDAT
#else
#ifdef VMS
#define  ZFCDAT
#endif /* VMS */
#endif /* datageneral */
#endif /* OS2 */
#endif /* AMIGA */
#endif /* GEMDOS */
#endif /* STRATUS */
#endif /* UNIX */
#endif /* ZFCDAT */

#ifdef SUNS4S5
#define tolower _tolower
#define toupper _toupper
#endif /* SUNS4S5 */

/* Error number */

#ifdef _CRAY
#ifdef _CRAYCOM				/* Cray Computer Corp. */
extern int errno;
#else /* _CRAYCOM */
#include <errno.h>			/* Cray Research UNICOS defines */
					/* errno as a function. */
#endif /* _CRAYCOM */			/* OK for UNICOS 6.1 and 7.0. */
#else /* _CRAY */
#ifdef STRATUS				/* Stratus VOS */
#include <errno.h>
#else /* not STRATUS */
#ifndef VMS
#ifndef OS2
#ifdef __GLIBC__
/*
  "glibc uses threads, kermit uses glibc; errno access is in Thread Local
  Storage (TLS) from glibc-3.2.2.  ...a thread specific errno is being run in
  thread local storage relative to the %gs segment register, so some means to
  revector gets/puts needs to be done." - Jeff Johnson, Red Hat, Feb 2003.
*/
#include <errno.h>
#else
/*
  The following declaration would cause problems for VMS and OS/2, in which
  errno is an "extern volatile int noshare"...  NOTE: by now (2007) the
  following is an anachronism and should be the execption rather than the
  rule.
*/
extern int errno;
#endif /* __GLIBC__ */
#endif /* OS2 */
#endif /* VMS */
#endif /* STRATUS */
#endif /* _CRAY */

#ifdef UNIX				/* Catch-all so we can have */
#ifndef ESRCH				/* access to error mnemonics */
#include <errno.h>			/* in all modules - 2007/08/25 */
#endif	/* ESRCH */
#endif	/* UNIX */

#ifdef pdp11				/* Try to make some space on PDP-11 */
#ifndef NODIAL
#define NODIAL
#endif /* NODIAL */
#ifndef NOCURSES
#define NOCURSES
#endif /* NOCURSES */
#ifndef NOBIGBUF
#define NOBIGBUF
#endif /* NOBIGBUF */
#endif /* pdp11 */

#ifndef NOBIGBUF
#ifndef BIGBUFOK			/* Platforms with lots of memory */

#ifdef QNX				/* QNX */
#ifndef QNX16				/* But not 16-bit versions */
#define BIGBUFOK
#endif /* QNX16 */
#endif /* QNX */

#ifdef BSD44
#define BIGBUFOK
#endif /* BSD44 */

#ifdef STRATUS				/* Stratus VOS */
#define BIGBUFOK
#endif /* STRATUS */

#ifdef sparc				/* SPARC processors */
#define BIGBUFOK
#endif /* sparc */

#ifdef mips				/* MIPS processors */
#define BIGBUFOK
#endif /* mips */

#ifdef HPUX9				/* HP-UX 9.x */
#define BIGBUFOK
#endif /* HPUX9 */

#ifdef HPUX10				/* HP-UX 10.0 PA-RISC */
#define BIGBUFOK
#endif /* HPUX10 */

#ifdef NEXT				/* NeXTSTEP */
#ifdef mc68000				/* on NEXT platforms... */
#define BIGBUFOK
#endif /* mc68000 */
#endif /* NEXT */

#ifdef LINUX				/* Linux in 1998 should be OK */
#ifndef BIGBUFOK
#define BIGBUFOK
#endif /* BIGBUFOK */
#endif /* LINUX */

#ifdef OS2				/* 32-bit OS/2 2.x and above */
#ifdef __32BIT__
#define BIGBUFOK
#endif /* __32BIT__ */
#ifdef NT
#define BIGBUFOK
#endif /* NT */
#endif /* OS2 */

#ifdef Plan9				/* Plan 9 is OK */
#define BIGBUFOK
#endif /* Plan9 */

#ifdef VMS				/* Any VMS is OK */
#ifndef BIGBUFOK
#define BIGBUFOK
#endif /* BIGBUFOK */
#endif /* VMS */

#ifdef __alpha				/* DEC 64-bit Alpha, e.g. OSF/1 */
#ifndef BIGBUFOK			/* Might already be defined for VMS */
#define BIGBUFOK
#endif /* BIGBUFOK */
#endif /* __alpha */

#ifdef sgi				/* SGI with IRIX 4.0 or later */
#ifndef BIGBUFOK
#define BIGBUFOK
#endif /* BIGBUFOK */
#endif /* sgi */

#ifdef AIXRS				/* AIX on RISC */
#define BIGBUFOK
#endif /* AIXRS */

#ifdef CK_SCOV5				/* SCO OSR5 */
#ifndef BIGBUFOK
#define BIGBUFOK
#endif /* BIGBUFOK */
#endif /* CK_SCOV5 */

#ifdef SOLARIS				/* Solaris x86 */
#ifndef BIGBUFOK
#define BIGBUFOK
#endif /* BIGBUFOK */
#endif /* SOLARIS */

#endif /* BIGBUFOK */
#endif /* NOBIGBUF */

#ifdef CK_SMALL
#ifdef BIGBUFOK
#undef BIGBUFOK
#endif /* BIGBUFOK */
#endif /* CK_SMALL */

/* If "memory is no problem" then this improves performance */

#ifdef DEBUG
#ifdef BIGBUFOK
#ifndef IFDEBUG
#define IFDEBUG
#endif /* IFDEBUG */
#endif /* BIGBUFOK */
#endif /* DEBUG */

/* File System Defaults */

#ifndef UIDBUFLEN			/* Length of User ID */
#ifdef OS2
#define UIDBUFLEN 256
#else /* OS2 */
#ifdef BIGBUFOK
#define UIDBUFLEN 256
#else
#define UIDBUFLEN 64
#endif /* BIGBUFOK */
#endif /* OS2 */
#endif /* UIDBUFLEN */

#ifdef UNIX
#ifdef PROVX1
#define MAXWLD 50
#else
#ifdef pdp11
#define MAXWLD 50
#else
#ifdef BIGBUFOK
#define MAXWLD 102400
#else
#define MAXWLD 1024
#endif /* BIGBUFOK */
#endif /* pdp11 */
#endif /* PROVX1 */
#else
#ifdef VMS
#define MAXWLD 102400			/* Maximum wildcard filenames */
#else
#ifdef datageneral
#define MAXWLD 500
#else
#ifdef STRATUS
#define MAXWLD 5000
#endif /* STRATUS */
#endif /* datageneral */
#endif /* VMS */
#endif /* UNIX */

#ifdef VMS
#define DBLKSIZ 512
#define DLRECL 512
#else
#define DBLKSIZ 0
#define DLRECL 0
#endif /* VMS */

/* Communication device / network host name length */

#ifdef BIGBUFOK
#define TTNAMLEN 512
#else
#ifdef MAC
#define TTNAMLEN 256
#else
#ifndef CK_SMALL
#define TTNAMLEN 128
#else
#define TTNAMLEN 80
#endif /* CK_SMALL */
#endif /* MAC */
#endif /* BIGBUFOK */

/* Program return codes for DECUS C and UNIX (VMS uses UNIX codes) */

#ifdef decus
#define GOOD_EXIT   IO_NORMAL
#define BAD_EXIT    IO_ERROR
#else
#define GOOD_EXIT   0
#define BAD_EXIT    1
#endif /* decus */

/* Special hack for Fortune, which doesn't have <sys/file.h>... */

#ifdef FT18
#define FREAD 0x01
#define FWRITE 0x10
#endif /* FT18 */

/* Special hack for OS-9/68k */
#ifdef OSK
#ifndef _UCC
#define SIGALRM 30			/* May always cancel I/O */
#endif /* _UCC */
#define SIGARB	1234			/* Arbitrary for I/O */
SIGTYP (*signal())();
#endif /* OSK */

#ifdef MINIX
#ifdef putchar
#undef putchar
#endif /* putchar */
#define putchar(c) (putc(c,stdout)!=EOF)&&fflush(stdout)
#endif /* MINIX */

#ifdef datageneral			/* Data General AOS/VS */
#ifdef putchar
#undef putchar
#endif /* putchar */
#define putchar(c) conoc(c)
#endif /* datageneral */

/* Escape/quote character used by the command parser */

#define CMDQ '\\'

/* Symbols for RS-232 modem signals */

#define KM_FG    1			/* Frame ground */
#define KM_TXD   2			/* Transmit */
#define KM_RXD   3			/* Receive */
#define KM_RTS   4			/* Request to Send */
#define KM_CTS   5			/* Clear to Send */
#define KM_DSR   6			/* Data Set Ready */
#define KM_SG    7			/* Signal ground */
#define KM_DCD   8			/* Carrier Detect */
#define KM_DTR  20			/* Data Terminal Ready */
#define KM_RI   22			/* Ring Indication */

/* Bit mask values for modem signals */

#define BM_CTS   0001			/* Clear to send       (From DCE) */
#define BM_DSR   0002			/* Dataset ready       (From DCE) */
#define BM_DCD   0004			/* Carrier             (From DCE) */
#define BM_RNG   0010			/* Ring Indicator      (From DCE) */
#define BM_DTR   0020			/* Data Terminal Ready (From DTE) */
#define BM_RTS   0040			/* Request to Send     (From DTE) */

/* Codes for full duplex flow control */

#define FLO_NONE 0			/* None */
#define FLO_XONX 1			/* Xon/Xoff (soft) */
#define FLO_RTSC 2			/* RTS/CTS (hard) */
#define FLO_DTRC 3			/* DTR/CD (hard) */
#define FLO_ETXA 4			/* ETX/ACK (soft) */
#define FLO_STRG 5			/* String-based (soft) */
#define FLO_DIAL 6			/* DIALing kludge */
#define FLO_DIAX 7			/* Cancel dialing kludge */
#define FLO_DTRT 8			/* DTR/CTS (hard) */
#define FLO_KEEP 9			/* Keep, i.e. don't touch or change */
#define FLO_AUTO 10			/* Figure out automatically */

/* Types of connections */

#define CXT_REMOTE  0			/* Remote mode - no connection */
#define CXT_DIRECT  1			/* Direct serial connection */
#define CXT_MODEM   2			/* Modem dialout */
#define CXT_TCPIP   3			/* TCP/IP - Telnet, Rlogin, etc */
#define CXT_X25     4			/* X.25 peer-to-peer */
#define CXT_DECNET  5			/* DECnet (CTERM, etc) */
#define CXT_LAT     6			/* LAT */
#define CXT_NETBIOS 7			/* NETBIOS */
#define CXT_NPIPE   8			/* Named Pipe */
#define CXT_PIPE    9			/* Pipe, Command, PTY, DLL, etc */
#define CXT_SSH     10                  /* SSH */
#define CXT_MAX     10			/* Highest connection type */

/* Autodownload Detection Options */

#define ADL_PACK 0			/* Auto-Download detect packet */
#define ADL_STR  1			/* Auto-Download detect string */

/* And finally... */

#ifdef COMMENT				/* Make sure this is NOT defined! */
#undef COMMENT
#endif /* COMMENT */

/* zstr zattr filinfo were here (moved to top for DECC 5 Jun 2000) */

#ifndef ZFNQFP				/* Versions that have zfnqfp() */
#ifdef UNIX
#define ZFNQFP
#else
#ifdef VMS
#define ZFNQFP
#else
#ifdef OS2
#define ZFNQFP
#else
#ifdef datageneral
#define ZFNQFP
#else
#ifdef STRATUS
#define ZFNQFP
#endif /* STRATUS */
#endif /* datageneral */
#endif /* OS2 */
#endif /* VMS */
#endif /* UNIX */
struct zfnfp {
   int len;				/* Length of full pathname */
   char * fpath;			/* Pointer to full pathname */
   char * fname;			/* Pointer to name part */
};
#endif /* ZFNQFP */

/* Systems that support FILE TYPE LABELED */

#ifdef VMS
#define CK_LABELED
#else
#ifdef OS2
#ifdef __32BIT__
#ifndef NT
#define CK_LABELED
#endif /* NT */
#endif /* __32BIT__ */
#endif /* OS2 */
#endif /* VMS */

/* LABELED FILE options bitmask */

#ifdef VMS				/* For VMS */
#define LBL_NAM  1			/* Ignore incoming name if set */
#define LBL_PTH  2			/* Use complete path if set */
#define LBL_ACL  4			/* Preserve ACLs if set */
#define LBL_BCK  8			/* Preserve backup date if set */
#define LBL_OWN 16			/* Preserve ownership if set */

#else

#ifdef OS2				/* Ditto for OS/2 */
#define LBL_NOR  0x0000			/* Normal file */
#define LBL_ARC  0x0020			/* Archive */
#define LBL_DIR  0x0010			/* Directory */
#define LBL_HID  0x0002			/* Hidden file */
#define LBL_RO   0x0001			/* Read only file */
#define LBL_SYS  0x0004			/* System file */
#define LBL_EXT  0x0040			/* Extended */
#endif /* OS2 */
#endif /* VMS */

/*
  Data types.  First the header file for data types so we can pick up the
  types used for pids, uids, and gids.  Override this section by putting
  -DCKTYP_H=xxx on the command line to specify the header file where your
  system defines these types.
*/
#ifndef STRATUS
#ifdef __ALPHA
#ifdef MULTINET
#define CK_TGV_AXP
#endif /* MULTINET */
#endif /* __ALPHA */

#ifdef CK_TGV_AXP			/* Alpha, VMS, MultiNet */
/*
  Starting in DECC 5.0, <stdlib.h> no longer includes <types.h>.
  But before that an elaborate workaround is required, which results in
  including <types.h> sometimes but not others, evidently depending on whether
  <types.h> protects itself against multiple inclusion, which in turn probably
  differentiates between DECC <types.h> and TGV <types.h>.  Unfortunately I
  don't remember the details.  (fdc, 25 Oct 96)
*/
#ifdef COMMENT
/*
  Previously the test here was for DEC version prior to 4.0, but since the
  test involved an "#if" statement, it was not portable and broke some non-VMS
  builds.  In any case, condition was never satisfied, so the result of
  commenting this section out is the same as the previous "#if" condition.
*/
#ifndef __TYPES_LOADED
#define __TYPES_LOADED			/* Work around bug in .h files */
#endif /* __TYPES_LOADED */
#endif /* COMMENT */
#include <sys/types.h>
#ifdef IF_DOT_H
#ifndef MULTINET
#include <if.h>				/* Needed to put up u_int typedef */
#endif /* MULTINET */
#else /* IF_DOT_H */
#ifdef NEEDUINT
typedef unsigned int u_int;
#endif /* NEEDUINT */
#endif /* IF_DOT_H */
#else					/* !CK_TGV_AXP */
#ifdef OSK				/* OS-9 */
#include <types.h>
#else					/* General case, not OS-9 */
#ifndef CKTYP_H
#ifndef VMS
#ifndef MAC
#ifndef AMIGA
#define CKTYP_H <sys/types.h>
#endif /* AMIGA */
#endif /* MAC */
#endif /* VMS */
#endif /* CKTYP_H */

#ifdef GEMDOS
#undef CKTYP_H
#include <types.h>
#endif /* GEMDOS */

#ifdef OS2
#undef CKTYP_H
#include <sys/types.h>
#endif /* OS2 */

#ifdef CKTYP_H				/* Include it. */
#ifdef COHERENT				/* Except for COHERENT */
#include <unistd.h>
#include <sys/types.h>
#else
#ifdef datageneral			/* AOS/VS */
#include <sys/types.h>
#else  /* All others */
#ifdef __bsdi__				/* BSDI */
#ifdef POSIX
#undef _POSIX_SOURCE
#endif /* POSIX */
#endif /* __bsdi__ */
#include CKTYP_H
#ifdef __bsdi__
#ifdef POSIX
#define _POSIX_SOURCE
#endif /* POSIX */
#endif /* __bsdi__ */
#endif /* datageneral */
#endif /* COHERENT */
#endif /* CKTYP_H */

#endif /* OSK */
#endif /* CK_TGV_AXP */
#endif /* STRATUS */			/* End of types.h section */

/*
  File lengths and offsets.  This section is expected to grow as we
  support long files on 32-bit platforms.  We want this data type to be
  signed because so many functions return either a file size or a negative
  value to indicate an error.
*/
#ifndef CK_OFF_T
#ifdef OS2
#ifdef NT
#define CK_OFF_T __int64
#else
#define CK_OFF_T long
#endif  /* NT */
#endif	/* OS2 */
#endif	/* CK_OFF_T */

/* FreeBSD and OpenBSD set off_t to the appropriate size unconditionally */

#ifndef CK_OFF_T
#ifdef __FreeBSD__
#define CK_OFF_T off_t
#else
#ifdef __OpenBSD__
#define CK_OFF_T off_t
#endif	/* __OpenBSD__ */
#endif	/* __FreeBSD__ */
#endif	/* CK_OFF_T */

/* 32-bit platforms that support long files thru "transitional interface" */
/* These include Linux, Solaris, NetBSD... */

#ifdef AIXRS
#ifdef _LARGE_FILES
#ifndef CK_OFF_T
#define CK_OFF_T off_t
#endif	/* CK_OFF_T */
#endif	/* _LARGE_FILES */
#endif	/* AIXRS */

#ifdef _LARGEFILE_SOURCE
#ifndef CK_OFF_T
#define CK_OFF_T off_t
#endif	/* CK_OFF_T */
#ifdef IRIX
#define CKFSEEK(a,b,c) fseek64(a,b,c)
#define CKFTELL(a) ftell64(a)
#else /* IRIX */
#define CKFSEEK(a,b,c) fseeko(a,b,c)
#define CKFTELL(a) ftello(a)
#endif	/* IRIX */
#else  /* Not  _LARGEFILE_SOURCE */
#define CKFSEEK(a,b,c) fseek(a,b,c)
#define CKFTELL(a) ftell(a)
/* See below the next section for the catch-all case */
#endif	/* _LARGEFILE_SOURCE */

/* 32-bit or 64-bit platforms */

/* CK_64BIT is a compile-time symbol indicating a true 64-bit build */
/* meaning that longs and pointers are 64 bits */

#ifndef VMS				/* VMS Alpha and IA64 are 32-bit! */
#ifndef CK_64BIT
#ifdef _LP64				/* Solaris */
#define CK_64BIT
#else
#ifdef __LP64__				/* MacOS X 10.4 (or _LP64,__ppc64__) */
#define CK_64BIT
#else
#ifdef __arch64__			/* gcc alpha, sparc */
#define CK_64BIT
#else
#ifdef __alpha				/* Alpha decc (or __ALPHA) */
#define CK_64BIT
#else
#ifdef __amd64				/* AMD x86_64 (or __x86_64) */
#define CK_64BIT
#else
#ifdef __ia64				/* Intel IA64 */
#ifndef HPUX
#define CK_64BIT
#endif	/* HPUX */
#endif	/* __ia64 */
#endif	/* __amd64 */
#endif	/* __alpha */
#endif	/* __arch64__ */
#endif	/* __LP64__ */
#endif	/* _LP64 */
#endif	/* CK_64BIT */
#endif	/* VMS */

#ifndef CK_OFF_T
#ifdef CK_64BIT
#define CK_OFF_T off_t			/* This has to be signed */
#else  /* CK_64BIT */
#define CK_OFF_T long			/* Signed */
#endif	/* CK_64BIT */
#endif	/* CK_OFF_T */

#ifndef TLOG
#define tlog(a,b,c,d)
#else
#ifndef CKCMAI
/* Debugging included.  Declare debug log flag in main program only. */
extern int tralog, tlogfmt;
#endif /* CKCMAI */
_PROTOTYP(VOID dotlog,(int, char *, char *, CK_OFF_T));
#define tlog(a,b,c,d) if (tralog && tlogfmt) dotlog(a,b,c,(CK_OFF_T)d)
_PROTOTYP(VOID doxlog,(int, char *, CK_OFF_T, int, int, char *));
#endif /* TLOG */

#ifndef DEBUG
/* Compile all the debug() statements away.  Saves a lot of space and time. */
#define debug(a,b,c,d)
#define ckhexdump(a,b,c)
/* Now define the debug() macro. */
#else /* DEBUG */
_PROTOTYP(int dodebug,(int,char *,char *,CK_OFF_T));
_PROTOTYP(int dohexdump,(CHAR *,CHAR *,int));
#ifdef IFDEBUG
/* Use this form to avoid function calls: */
#ifdef COMMENT
#define debug(a,b,c,d) if (deblog) dodebug(a,b,(char *)(c),(CK_OFF_T)(d))
#define ckhexdump(a,b,c) if (deblog) dohexdump((CHAR *)(a),(CHAR *)(b),c)
#else
#ifdef CK_ANSIC
#define debug(a,b,c,d) \
((void)(deblog?dodebug(a,b,(char *)(c),(CK_OFF_T)(d)):0))
#define ckhexdump(a,b,c) \
((void)(deblog?dohexdump((CHAR *)(a),(CHAR *)(b),c):0))
#else
#define debug(a,b,c,d) (deblog?dodebug(a,b,(char *)(c),(CK_OFF_T)(d)):0)
#define ckhexdump(a,b,c) (deblog?dohexdump((CHAR *)(a),(CHAR *)(b),c):0)
#endif /* CK_ANSIC */
#endif /* COMMENT */
#else /* IFDEBUG */
/* Use this form to save space: */
#define debug(a,b,c,d) dodebug(a,b,(char *)(c),(CK_OFF_T)(d))
#define ckhexdump(a,b,c) dohexdump((CHAR *)(a),(CHAR *)(b),c)
#endif /* IFDEBUG */
#endif /* DEBUG */


/* Structure definitions for Kermit file attributes */
/* All strings come as pointer and length combinations */
/* Empty string (or for numeric variables, -1) = unused attribute. */

struct zstr {             /* string format */
    int len;	          /* length */
    char *val;            /* value */
};

struct zattr {            /* Kermit File Attribute structure */
    CK_OFF_T lengthk;	  /* (!) file length in K */
    struct zstr type;     /* (") file type (text or binary) */
    struct zstr date;     /* (#) file creation date yyyymmdd[ hh:mm[:ss]] */
    struct zstr creator;  /* ($) file creator id */
    struct zstr account;  /* (%) file account */
    struct zstr area;     /* (&) area (e.g. directory) for file */
    struct zstr password; /* (') password for area */
    long blksize;         /* (() file blocksize */
    struct zstr xaccess;  /* ()) file access: new, supersede, append, warn */
    struct zstr encoding; /* (*) encoding (transfer syntax) */
    struct zstr disp;     /* (+) disposition (mail, message, print, etc) */
    struct zstr lprotect; /* (,) protection (local syntax) */
    struct zstr gprotect; /* (-) protection (generic syntax) */
    struct zstr systemid; /* (.) ID for system of origin */
    struct zstr recfm;    /* (/) record format */
    struct zstr sysparam; /* (0) system-dependent parameter string */
    CK_OFF_T length;      /* (1) exact length on system of origin */
    struct zstr charset;  /* (2) transfer syntax character set */
#ifdef OS2
    struct zstr longname; /* OS/2 longname if applicable */
#endif /* OS2 */
    struct zstr reply;    /* This goes last, used for attribute reply */
};

/* Kermit file information structure */

struct filinfo {
  int bs;				/* Blocksize */
  int cs;				/* Character set */
  long rl;				/* Record length */
  int org;				/* Organization */
  int fmt;				/* Record format */
  int cc;				/* Carriage control */
  int typ;				/* Type (text/binary) */
  int dsp;				/* Disposition */
  char *os_specific;			/* OS-specific attributes */
#ifdef OS2
  unsigned long int lblopts;		/* LABELED FILE options bitmask */
#else
  int lblopts;
#endif /* OS2 */
};


/*
  Data type for pids.  If your system uses a different type, put something
  like -DPID_T=pid_t on command line, or override here.
*/
#ifndef PID_T
#define PID_T int
#endif /* PID_T */
/*
  Data types for uids and gids.  Same deal as for pids.
  Wouldn't be nice if there was a preprocessor test to find out if a
  typedef existed?
*/
#ifdef VMS
/* Not used in VMS so who cares */
#define UID_T int
#define GID_T int
#endif /* VMS */

#ifdef POSIX
/* Or would it be better (or worse?) to use _POSIX_SOURCE here? */
#ifndef UID_T
#define UID_T uid_t
#endif /* UID_T */
#ifndef GID_T
#define GID_T gid_t
#endif /* GID_T */
#else /* Not POSIX */
#ifdef SVR4
/* SVR4 and later have uid_t and gid_t. */
/* SVR3 and earlier use int, or unsigned short, or.... */
#ifndef UID_T
#define UID_T uid_t
#endif /* UID_T */
#ifndef GID_T
#define GID_T gid_t
#endif /* GID_T */
#else /* Not SVR4 */
#ifdef BSD43
#ifndef UID_T
#define UID_T uid_t
#endif /* UID_T */
#ifndef GID_T
#define GID_T gid_t
#endif /* GID_T */
#else /* Not BSD43 */
/* Default these to int for older UNIX versions */
#ifndef UID_T
#define UID_T int
#endif /* UID_T */
#ifndef GID_T
#define GID_T int
#endif /* GID_T */
#endif /* BSD43 */
#endif /* SVR4  */
#endif /* POSIX */

/*
  getpwuid() arg type, which is not necessarily the same as UID_T,
  e.g. in SCO UNIX SVR3, it's int.
*/
#ifndef PWID_T
#define PWID_T UID_T
#endif /* PWID_T */

#ifdef CK_REDIR
#ifdef NEXT
#define MACHWAIT
#else
#ifdef MACH
#define MACHWAIT
#endif /* MACH */
#endif /* NEXT */

#ifdef MACHWAIT				/* WAIT_T argument for wait() */
#include <sys/wait.h>
#define CK_WAIT_H
typedef union wait WAIT_T;
#else
#ifdef POSIX
#ifdef OSF
/* OSF wait.h defines BSD wait if _BSD is defined so  hide _BSD from wait.h */
#ifdef _BSD
#define CK_OSF_BSD
#undef  _BSD
#endif /* _BSD */
#endif /* OSF */
#include <sys/wait.h>
#define CK_WAIT_H
#ifndef WAIT_T
typedef int WAIT_T;
#endif /* WAIT_T */
#ifdef CK_OSF_BSD			/* OSF/1: Restore  _BSD definition */
#define _BSD
#undef CK_OSF_BSD
#endif /* CK_OSF_BSD */
#else /* !POSIX */
typedef int WAIT_T;
#endif /* POSIX */
#endif /* MACHWAIT */
#else
typedef int WAIT_T;
#endif /* CK_REDIR */

/* Assorted other blah_t's handled here... */

#ifndef SIZE_T
#define SIZE_T size_t
#endif /* SIZE_T */

/* Forward declarations of system-dependent functions callable from all */
/* C-Kermit modules. */

/* File-related functions from system-dependent file i/o module */

#ifndef CKVFIO_C
/* For some reason, this does not agree with DEC C */
_PROTOTYP( int zkself, (void) );
#endif /* CKVFIO_C */
_PROTOTYP( int zopeni, (int, char *) );
_PROTOTYP( int zopeno, (int, char *, struct zattr *, struct filinfo *) );
_PROTOTYP( int zclose, (int) );
#ifndef MAC
_PROTOTYP( int zchin, (int, int *) );
#endif /* MAC */
_PROTOTYP( int zxin, (int, char *, int) );
_PROTOTYP( int zsinl, (int, char *, int) );
_PROTOTYP( int zinfill, (void) );
_PROTOTYP( int zsout, (int, char*) );
_PROTOTYP( int zsoutl, (int, char*) );
_PROTOTYP( int zsoutx, (int, char*, int) );
_PROTOTYP( int zchout, (int, char) );
_PROTOTYP( int zoutdump, (void) );
_PROTOTYP( int zsyscmd, (char *) );
_PROTOTYP( int zshcmd, (char *) );
#ifdef UNIX
_PROTOTYP( int zsetfil, (int, int) );
_PROTOTYP( int zchkpid, (unsigned long) );
#endif	/* UNIX */
#ifdef CKEXEC
_PROTOTYP( VOID z_exec, (char *, char **, int) );
#endif /* CKEXEC */
_PROTOTYP( int chkfn, (int) );
_PROTOTYP( CK_OFF_T zchki, (char *) );
#ifdef VMSORUNIX
_PROTOTYP( CK_OFF_T zgetfs, (char *) );
#else
#ifdef OS2
_PROTOTYP( CK_OFF_T zgetfs, (char *) );
#else
#define zgetfs(a) zchki(a)
#endif /* OS2 */
#endif /* VMSORUNIX */
_PROTOTYP( int iswild, (char *) );
_PROTOTYP( int isdir, (char *) );
_PROTOTYP( int zchko, (char *) );
_PROTOTYP( int zdelet, (char *) );
_PROTOTYP( VOID zrtol, (char *,char *) );
_PROTOTYP( VOID zltor, (char *,char *) );
_PROTOTYP( VOID zstrip, (char *,char **) );
#ifdef VMS
_PROTOTYP( char * zrelname, (char *, char *) );
#endif /* VMS */
_PROTOTYP( int zchdir, (char *) );
_PROTOTYP( char * zhome, (void) );
_PROTOTYP( char * zgtdir, (void) );
_PROTOTYP( int zxcmd, (int, char *) );
#ifndef MAC
_PROTOTYP( int zclosf, (int) );
#endif /* MAC */
#ifdef NZXPAND
_PROTOTYP( int nzxpand, (char *, int) );
#else /* NZXPAND */
_PROTOTYP( int zxpand, (char *) );
#endif /* NZXPAND */
_PROTOTYP( int znext, (char *) );
#ifdef ZXREWIND
_PROTOTYP( int zxrewind, (void) );
#endif /* ZXREWIND */
_PROTOTYP( int zchkspa, (char *, CK_OFF_T) );
_PROTOTYP( VOID znewn, (char *, char **) );
_PROTOTYP( int zrename, (char *, char *) );
_PROTOTYP( int zcopy, (char *, char *) );
_PROTOTYP( int zsattr, (struct zattr *) );
_PROTOTYP( int zfree, (char *) );
_PROTOTYP( char * zfcdat, (char *) );
_PROTOTYP( int zstime, (char *, struct zattr *, int) );
#ifdef CK_PERMS
_PROTOTYP( char * zgperm, (char *) );
_PROTOTYP( char * ziperm, (char *) );
#endif /* CK_PERMS */
_PROTOTYP( int zmail, (char *, char *) );
_PROTOTYP( int zprint, (char *, char *) );
_PROTOTYP( char * tilde_expand, (char *) );
_PROTOTYP( int zmkdir, (char *) ) ;
_PROTOTYP( int zfseek, (CK_OFF_T) ) ;
#ifdef ZFNQFP
_PROTOTYP( struct zfnfp * zfnqfp, (char *, int, char * ) ) ;
#else
#define zfnqfp(a,b,c) ckstrncpy(c,a,b)
#endif /* ZFNQFP */
_PROTOTYP( int zvuser, (char *) ) ;
_PROTOTYP( int zvpass, (char *) ) ;
_PROTOTYP( VOID zvlogout, (void) ) ;
#ifdef OS2
_PROTOTYP( int os2setlongname, ( char * fn, char * ln ) ) ;
_PROTOTYP( int os2getlongname, ( char * fn, char ** ln ) ) ;
_PROTOTYP( int os2rexx, ( char *, char *, int ) ) ;
_PROTOTYP( int os2rexxfile, ( char *, char *, char *, int) ) ;
_PROTOTYP( int os2geteas, (char *) ) ;
_PROTOTYP( int os2seteas, (char *) ) ;
_PROTOTYP( char * get_os2_vers, (void) ) ;
_PROTOTYP( int do_label_send, (char *) ) ;
_PROTOTYP( int do_label_recv, (void) ) ;
#ifdef OS2MOUSE
_PROTOTYP( unsigned long os2_mouseon, (void) );
_PROTOTYP( unsigned long os2_mousehide, (void) );
_PROTOTYP( unsigned long os2_mouseshow, (void) );
_PROTOTYP( unsigned long os2_mouseoff, (void) );
_PROTOTYP( void os2_mouseevt, (void *) );
_PROTOTYP( int mousebuttoncount, (void));
#endif /* OS2MOUSE */
#endif /* OS2 */

/* Functions from system-dependent terminal i/o module */

_PROTOTYP( int ttopen, (char *, int *, int, int) );  /* tty functions */
#ifndef MAC
_PROTOTYP( int ttclos, (int) );
#endif /* MAC */
_PROTOTYP( int tthang, (void) );
_PROTOTYP( int ttres, (void) );
_PROTOTYP( int ttpkt, (long, int, int) );
#ifndef MAC
_PROTOTYP( int ttvt, (long, int) );
#endif /* MAC */
_PROTOTYP( int ttsspd, (int) );
_PROTOTYP( long ttgspd, (void) );
_PROTOTYP( int ttflui, (void) );
_PROTOTYP( int ttfluo, (void) );
_PROTOTYP( int ttpushback, (CHAR *, int) );
_PROTOTYP( int ttpeek, (void) );
_PROTOTYP( int ttgwsiz, (void) );
_PROTOTYP( int ttchk, (void) );
_PROTOTYP( int ttxin, (int, CHAR *) );
_PROTOTYP( int ttxout, (CHAR *, int) );
_PROTOTYP( int ttol, (CHAR *, int) );
_PROTOTYP( int ttoc, (char) );
_PROTOTYP( int ttinc, (int) );
_PROTOTYP( int ttscarr, (int) );
_PROTOTYP( int ttgmdm, (void) );
_PROTOTYP( int ttsndb, (void) );
_PROTOTYP( int ttsndlb, (void) );
#ifdef UNIX
_PROTOTYP( char * ttglckdir, (void) );
#endif /* UNIX */
#ifdef PARSENSE
#ifdef UNIX
_PROTOTYP( int ttinl, (CHAR *, int, int, CHAR, CHAR, int) );
#else
#ifdef VMS
_PROTOTYP( int ttinl, (CHAR *, int, int, CHAR, CHAR, int) );
#else
#ifdef STRATUS
_PROTOTYP( int ttinl, (CHAR *, int, int, CHAR, CHAR, int) );
#else
#ifdef OS2
_PROTOTYP( int ttinl, (CHAR *, int, int, CHAR, CHAR, int) );
#else
#ifdef OSK
_PROTOTYP( int ttinl, (CHAR *, int, int, CHAR, CHAR, int) );
#else
_PROTOTYP( int ttinl, (CHAR *, int, int, CHAR, CHAR) );
#endif /* OSK */
#endif /* OS2 */
#endif /* STRATUS */
#endif /* VMS */
#endif /* UNIX */
#else /* ! PARSENSE */
_PROTOTYP( int ttinl, (CHAR *, int, int, CHAR) );
#endif /* PARSENSE */

/* XYZMODEM support */

/*
  CK_XYZ enables the various commands and data structures.
  XYZ_INTERNAL means these protocols are built-in; if not defined,
  then they are external.  XYZ_DLL is used to indicate a separate
  loadable library containing the XYZmodem protocol code.
*/
#ifdef pdp11				/* No room for this in PDP-11 */
#define NOCKXYZ
#endif /* pdp11 */

#ifndef NOCKXYZ				/* Alternative protocols */
#ifndef CK_XYZ
#ifdef UNIX
#define CK_XYZ
#else
#ifdef OS2
#define CK_XYZ
#ifndef NOXYZDLL
#define XYZ_INTERNAL			/* Internal and DLL */
#define XYZ_DLL
#endif /* NOXYZDLL */
#endif /* OS2 */
#endif /* UNIX */
#endif /* CK_XYZ */
#endif /* NOCKXYZ */

#ifdef XYZ_INTERNAL			/* This ensures that XYZ_INTERNAL */
#ifndef CK_XYZ				/* is defined only if CK_XYZ is too */
#undef XYZ_INTERNAL
#endif /* CK_XYZ */
#endif /* XYZ_INTERNAL */
#ifdef XYZ_DLL				/* This ensures XYZ_DLL is defined */
#ifndef XYZ_INTERNAL			/* only if XYZ_INTERNAL is too */
#undef XYZ_DLL
#endif /* XYZ_INTERNAL */
#endif /* XYZ_DLL */

/* Console functions */

_PROTOTYP( int congm, (void) );
#ifdef COMMENT
_PROTOTYP( VOID conint, (SIGTYP (*)(int, int), SIGTYP (*)(int, int)) );
#else
_PROTOTYP( VOID conint, (SIGTYP (*)(int), SIGTYP (*)(int)) );
#endif /* COMMENT */
_PROTOTYP( VOID connoi, (void) );
_PROTOTYP( int concb, (char) );
#ifdef CONGSPD
_PROTOTYP( long congspd, (void) );
#endif /* CONGSPD */
_PROTOTYP( int conbin, (char) );
_PROTOTYP( int conres, (void) );
_PROTOTYP( int conoc, (char) );
_PROTOTYP( int conxo, (int, char *) );
_PROTOTYP( int conol, (char *) );
_PROTOTYP( int conola, (char *[]) );
_PROTOTYP( int conoll, (char *) );
_PROTOTYP( int conchk, (void) );
_PROTOTYP( int coninc, (int) );
_PROTOTYP( char * conkbg, (void) );
_PROTOTYP( int psuspend, (int) );
_PROTOTYP( int priv_ini, (void) );
_PROTOTYP( int priv_on, (void) );
_PROTOTYP( int priv_off, (void) );
_PROTOTYP( int priv_can, (void) );
_PROTOTYP( int priv_chk, (void) );
_PROTOTYP( int priv_opn, (char *, int) );

_PROTOTYP( int sysinit, (void) );	/* Misc Kermit functions */
_PROTOTYP( int syscleanup, (void) );
_PROTOTYP( int msleep, (int) );
_PROTOTYP( VOID rtimer, (void) );
_PROTOTYP( int gtimer, (void) );
#ifdef GFTIMER
_PROTOTYP( VOID rftimer, (void) );
_PROTOTYP( CKFLOAT gftimer, (void) );
#endif /* GFTIMER */
_PROTOTYP( VOID ttimoff, (void) );
_PROTOTYP( VOID ztime, (char **) );
_PROTOTYP( int parchk, (CHAR *, CHAR, int) );
_PROTOTYP( VOID doexit, (int, int) );
_PROTOTYP( int askmore, (void) );
_PROTOTYP( VOID fatal, (char *) );
_PROTOTYP( VOID fatal2, (char *, char *) );
#ifdef VMS
_PROTOTYP( int ck_cancio, (void) );
#endif /* VMS */

/* Key mapping support */

#ifdef NOICP
#ifndef NOSETKEY
#define NOSETKEY
#endif /* NOSETKEY */
#endif /* NOICP */

#ifdef MAC
#ifndef NOSETKEY
#define NOSETKEY
#endif /* NOSETKEY */
#endif /* MAC */

_PROTOTYP( int congks, (int) );
#ifdef OS2
/* OS2 requires these definitions even if SET KEY is not being supported */
#define KMSIZE 8916
typedef ULONG KEY;
typedef CHAR *MACRO;
extern int wideresult;
#else /* Not OS2 */
#ifndef NOSETKEY
/*
  Catch-all for systems where we don't know how to read keyboard scan
  codes > 255.
*/
#define KMSIZE 256
/* Note: CHAR (i.e. unsigned char) is very important here. */
typedef CHAR KEY;
typedef CHAR * MACRO;
#define congks coninc
#endif /* NOSETKEY */
#endif /* OS2 */

#ifndef OS2
#ifndef NOKVERBS			/* No \Kverbs unless... */
#define NOKVERBS
#endif /* NOKVERBS */
#endif /* OS2 */

#ifndef NOKVERBS
#ifdef OS2
/*
  Note: this value chosen to be bigger than PC BIOS key modifier bits,
  but still fit in 16 bits without affecting sign.

  As of K95 1.1.5, this no longer fits in 16 bits, good thing we are 32 bit.
*/
#define F_MACRO 0x2000          /* Bit indicating a macro indice */
#define IS_MACRO(x) (x & F_MACRO)
#define F_KVERB 0x4000			/* Bit indicating a keyboard verb */
#define IS_KVERB(x) (x & F_KVERB)	/* Test this bit */
#endif /* OS2 */
#endif /* NOKVERBS */

#define F_ESC   0x8000		/* Bit indicating ESC char combination */
#define IS_ESC(x) (x & F_ESC)
#define F_CSI   0x10000		/* Bit indicating CSI char combination */
#define IS_CSI(x) (x & F_CSI)

#ifdef NOSPL				/* This might be overkill.. */
#ifndef NOKVERBS			/* Not all \Kverbs require */
#define NOKVERBS			/* the script programming language. */
#endif /* NOKVERBS */
#ifndef NOTAKEARGS
#define NOTAKEARGS
#endif /* NOTAKEARGS */
#endif /* NOSPL */

/*
  Function prototypes for system and library functions.
*/
#ifdef _POSIX_SOURCE
#ifndef VMS
#ifndef MAC
#define CK_ANSILIBS
#endif /* MAC */
#endif /* VMS */
#endif /* _POSIX_SOURCE */

#ifdef NEXT
#define CK_ANSILIBS
#endif /* NEXT */

#ifdef SVR4
#define CK_ANSILIBS
#endif /* SVR4 */

#ifdef STRATUS				/* Stratus VOS uses ANSI libraries */
#define CK_ANSILIBS
#endif /* STRATUS */

#ifdef OS2
#define CK_ANSILIBS
#ifndef NOCURSES
#define MYCURSES
#endif /* NOCURSES */
#define CK_RTSCTS
#ifdef __IBMC__
#define S_IFMT 0xF000
#define timezone _timezone
#endif /* __IBMC__ */
#include <fcntl.h>
#include <io.h>
#ifdef __EMX__
#ifndef __32BIT__
#define __32BIT__
#endif /* __32BIT__ */
#include <sys/timeb.h>
#else /* __EMX__ */
#include <direct.h>
#undef SIGALRM
#ifndef SIGUSR1
#define SIGUSR1 7
#endif /* SIGUSR1 */
#define SIGALRM SIGUSR1
_PROTOTYP( unsigned alarm, (unsigned) );
_PROTOTYP( unsigned sleep, (unsigned) );
#endif /* __EMX__ */
_PROTOTYP( unsigned long zdskspace, (int) );
_PROTOTYP( int zchdsk, (int) );
_PROTOTYP( int conincraw, (int) );
_PROTOTYP( int ttiscom, (int f) );
_PROTOTYP( int IsFileNameValid, (char *) );
_PROTOTYP( void ChangeNameForFAT, (char *) );
_PROTOTYP( char *GetLoadPath, (void) );
#endif /* OS2 */

/* Fullscreen file transfer display items... */

#ifndef NOCURSES
#ifdef CK_NCURSES			/* CK_NCURSES implies CK_CURSES */
#ifndef CK_CURSES
#define CK_CURSES
#endif /* CK_CURSES */
#endif /* CK_NCURSES */

#ifdef MYCURSES				/* MYCURSES implies CK_CURSES */
#ifndef CK_CURSES
#define CK_CURSES
#endif /* CK_CURSES */
#endif /* MYCURSES */
#endif /* NOCURSES */

#ifdef NOCURSES
#ifdef CK_CURSES
#undef CK_CURSES
#endif /* CK_CURSES */
#ifndef NODISPLAY
#define NODISPLAY
#endif /* NODISPLAY */
#endif /* NOCURSES */

#ifdef CK_CURSES
/*
  The CK_WREFRESH symbol is defined if the curses library provides
  clearok() and wrefresh() functions, which are used in repainting
  the screen.
*/
#ifdef NOWREFRESH			/* Override CK_WREFRESH */

#ifdef CK_WREFRESH			/* If this is defined, */
#undef CK_WREFRESH			/* undefine it. */
#endif /* CK_WREFRESH */

#else /* !NOWREFRESH */			/* No override... */

#ifndef CK_WREFRESH			/* If CK_WREFRESH not defined */
/*
  Automatically define it for systems known to have it ...
*/
#ifdef VMS				/* DEC (Open)VMS has it */
#define CK_WREFRESH
#else
#ifdef ultrix				/* DEC ULTRIX has it */
#else
#ifdef SVR3				/* System V has it */
#define CK_WREFRESH
#else
#ifdef BSD44				/* 4.4 BSD has it */
#define CK_WREFRESH
#else
#ifdef NEXT				/* Define it for NeXTSTEP */
#define CK_WREFRESH
#else
#ifdef SUNOS4				/* SunOS 4.x... */
#define CK_WREFRESH
#else
#ifdef SOLARIS25			/* Solaris 2.5 and later */
#define CK_WREFRESH
#else
#ifdef AIXRS				/* RS/6000 AIX ... */
#define CK_WREFRESH
#else
#ifdef RTAIX				/* RT PC AIX ... */
#define CK_WREFRESH
#else
#ifdef OSF				/* DEC OSF/1 ... */
#define CK_WREFRESH

/* Add more here, or just define CK_WREFRESH on the CC command line... */

#endif /* OSF */
#endif /* RTAIX */
#endif /* AIXRS */
#endif /* SOLARIS25 */
#endif /* SUNOS4 */
#endif /* NEXT */
#endif /* BSD44 */
#endif /* SVR3 */
#endif /* ultrix */
#endif /* VMS */

#else /* CK_WREFRESH is defined */

/* This is within an ifdef CK_CURSES block.  The following is not needed */

#ifndef CK_CURSES			/* CK_WREFRESH implies CK_CURSES */
#define CK_CURSES
#endif /* CK_CURSES */

#endif /* CK_WREFRESH */
#endif /* NOWREFRESH */

#ifndef TRMBUFL
#ifdef BIGBUFOK
#define TRMBUFL 16384
#else
#ifdef DYNAMIC
#define TRMBUFL 8192
#else
#define TRMBUFL 1024
#endif /* BIGBUFOK */
#endif /* DYNAMIC */
#endif /* TRMBUFL */
#endif /* CK_CURSES */

/*
  Whether to use ckmatch() in all its glory for C-Shell-like patterns.
  If CKREGEX is NOT defined, all but * and ? matching are removed from
  ckmatch().  NOTE: Defining CKREGEX does not necessarily mean that ckmatch()
  regexes are used for filename matching.  That depends on whether zxpand()
  in ck?fio.c calls ckmatch().  NOTE 2: REGEX is a misnomer -- these are not
  regular expressions in the computer-science sense (in which, e.g. "a*b"
  matches 0 or more 'a' characters followed by 'b') but patterns (in which
  "a*b" matches 'a' followed by 0 or more non-b characters, followed by b).
*/
#ifndef NOCKREGEX
#ifndef CKREGEX
#define CKREGEX
#endif /* CKREGEX */
#endif /* NOCKREGEX */

/* Learned-script feature */

#ifndef NOLEARN
#ifdef NOSPL
#define NOLEARN
#else
#ifdef NOLOCAL
#define NOLEARN
#endif /* NOLOCAL */
#endif /* NOSPL */
#endif /* NOLEARN */

#ifdef NOLEARN
#ifdef CKLEARN
#undef CKLEARN
#endif /* CKLEARN */
#else  /* !NOLEARN */
#ifndef CKLEARN
#ifdef OS2ORUNIX
/* In UNIX this can work only with ckucns.c builds */
#define CKLEARN
#else
#ifdef VMS
#define CKLEARN
#endif /* VMS */
#endif /* OS2ORUNIX */
#endif /* CKLEARN */
#endif /* NOLEARN */

#ifdef CKLEARN
#ifndef LEARNBUFSIZ
#define LEARNBUFSIZ 128
#endif /* LEARNBUFSIZ */
#endif /* CKLEARN */

#ifndef IKSDONLY
#ifndef CKTIDLE				/* Pseudo-keepalive in CONNECT */
#ifdef OS2				/* In K95 */
#define CKTIDLE
#else
#ifdef UNIX				/* In UNIX but only ckucns versions */
#ifndef NOLEARN
#ifndef NOSELECT
#define CKTIDLE
#endif /* NOSELECT */
#endif /* NOLEARN */
#endif /* UNIX */
#endif /* OS2 */
#endif /* CKTIDLE */
#endif /* IKSDONLY */

#ifdef CK_ANSILIBS
/*
  String library functions.
  For ANSI C, get prototypes from <string.h>.
  Otherwise, skip the prototypes.
*/
#include <string.h>

/*
  Prototypes for other commonly used library functions, such as
  malloc, free, getenv, atol, atoi, and exit.  Otherwise, no prototypes.
*/
#include <stdlib.h>
#ifdef DIAB /* DIAB DS90 */
/* #include <commonC.h>  */
#include <sys/wait.h>
#define CK_WAIT_H
#ifdef COMMENT
extern void exit(int status);
extern void _exit(int status);
extern int uname(struct utsname *name);
#endif /* COMMENT */
extern int chmod(char *path, int mode);
extern int ioctl(int fildes, int request, ...);
extern int rdchk(int ttyfd);
extern int nap(int m);
#ifdef COMMENT
extern int getppid(void);
#endif /* COMMENT */
extern int _filbuf(FILE *stream);
extern int _flsbuf(char c,FILE *stream);
#endif /* DIAB */

/*
  Prototypes for UNIX functions like access, alarm, chdir, sleep, fork,
  and pause.  Otherwise, no prototypes.
*/
#ifdef VMS
#include <signal.h>  /* SMS: sleep() for old (V4.0-000) DEC C. */
#include <unixio.h>
#include <unixlib.h> /* SMS: getpid() for old (V4.0-000) DEC C. */
#endif /* VMS */

#ifdef NEXT
#ifndef NEXT33
#include <libc.h>
#endif /* NEXT33 */
#else  /* NoT NeXT */
#ifndef AMIGA
#ifndef OS2
#ifdef STRATUS
#include <c_utilities.h>
#else /* !STRATUS */
#ifndef OSKXXC
#include <unistd.h>
#endif /* OSKXXC */
#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif /* HAVE_CRYPT_H */
#endif /* STRATUS */
#endif /* OS2 */
#endif /* AMIGA */
#endif /* NEXT */

#else /* Not ANSI libs... */

#ifdef MAC
#include <String.h>
#include <StdLib.h>
#endif /* MAC */

#ifdef HPUX
#ifndef HPUXPRE65
#include <unistd.h>
#endif /* HPUXPRE65 */
#endif /* HPUX */

#ifdef SUNOS41
#include <unistd.h>
#include <stdlib.h>
#else
#ifndef MAC
/*
  It is essential that these are declared correctly!
  Which is not always easy.  Take malloc() for instance ...
*/
#ifdef PYRAMID
#ifdef SVR4
#ifdef __STDC__
#define SIZE_T_MALLOC
#endif /* __STDC__ */
#endif /* SVR4 */
#endif /* PYRAMID */
/*
  Maybe some other environments need the same treatment for malloc.
  If so, define SIZE_T_MALLOC for them here or in compiler CFLAGS.
*/
#ifdef SIZE_T_MALLOC
_PROTOTYP( void * malloc, (size_t) );
#else
_PROTOTYP( char * malloc, (unsigned int) );
#endif /* SIZE_T_MALLOC */

_PROTOTYP( char * getenv, (char *) );
_PROTOTYP( long atol, (char *) );
#endif /* !MAC */
#endif /* SUNOS41 */
#endif /* CK_ANSILIBS */

/*
  <sys/param.h> generally picks up NULL, MAXPATHLEN, and MAXNAMLEN
  and seems to present on all Unixes going back at least to SCO Xenix
  with the exception(s) noted.
*/
#ifndef NO_PARAM_H			/* 2001-11-03 */
#ifndef UNIX				/* Non-Unixes don't have it */
#define NO_PARAM_H
#else
#ifdef TRS16				/* Tandy Xenix doesn't have it */
#define NO_PARAM_H
#endif /* TRS16 */
#endif /* UNIX */
#endif /* NO_PARAM_H */

#ifndef NO_PARAM_H
#ifndef INCL_PARAM_H
#define INCL_PARAM_H
#endif /* INCL_PARAM_H */
#include <sys/param.h>
#endif /* NO_PARAM_H */

#ifndef NULL				/* In case NULL is still not defined */
#define NULL 0L
/* or #define NULL 0 */
/* or #define NULL ((char *) 0) */
/* or #define NULL ((void *) 0) */
#endif /* NULL */

/* Macro to differentiate "" from NULL (to avoid comparisons with literals) */

#ifndef isemptystring
#define isemptystring(s) ((s?(*s?0:1):0))
#endif	/* isemptystring */

/* Maximum length for a fully qualified filename, not counting \0 at end. */
/*
  This is a rough cut, and errs on the side of being too big.  We don't
  want to pull in hundreds of header files looking for many and varied
  symbols, for fear of introducing unnecessary conflicts.
*/
#ifndef CKMAXPATH
#ifdef VMS				/* VMS may have bad (small, ODS2) */
#define CKMAXPATH NAMX_C_MAXRSS		/* PATH_MAX, so use NAMX_C_MAXRSS. */
#else /* def VMS */
#ifdef MAXPATHLEN			/* (it probably isn't) */
#define CKMAXPATH MAXPATHLEN
#else
#ifdef PATH_MAX				/* POSIX */
#define CKMAXPATH PATH_MAX
#else /* def PATH_MAX */
#ifdef MAC
#define CKMAXPATH 63
#else /* def MAC */
#ifdef pdp11
#define CKMAXPATH 255
#else /* def pdp11 */
#ifdef UNIX				/* Even though some are way less... */
#define CKMAXPATH 1024
#else /* def UNIX */
#ifdef STRATUS
#define CKMAXPATH 256			/* == $MXPL from PARU.H */
#else /* def STRATUS */
#ifdef datageneral
#define CKMAXPATH 256			/* == $MXPL from PARU.H */
#else /* def datageneral */
#define CKMAXPATH 255
#endif /* def STRATUS [else] */
#endif /* def datageneral [else] */
#endif /* def UNIX [else] */
#endif /* def pdp11 [else] */
#endif /* def MAC [else] */
#endif /* def PATH_MAX [else] */
#endif /* def MAXPATHLEN [else] */
#endif /* def VMS [else] */
#endif /* ndef CKMAXPATH */

/* Maximum length for the name of a tty device */

#ifndef DEVNAMLEN
#define DEVNAMLEN CKMAXPATH
#endif /* DEVNAMLEN */

/* Directory (path segment) separator */
/* Not fully general - Tricky for VMS, Amiga, ... */

#ifndef DIRSEP
#ifdef UNIX
#define DIRSEP '/'
#define ISDIRSEP(c) ((c)=='/')
#else
#ifdef OS2
#define DIRSEP '/'
#define ISDIRSEP(c) ((c)=='/'||(c)=='\\')
#else
#ifdef datageneral
#define DIRSEP ':'
#define ISDIRSEP(c) (((c)==':')||((c)=='^')||((c)=='='))
#else
#ifdef STRATUS
#define DIRSEP '>'
#define ISDIRSEP(c) ((c)=='>')
#else
#ifdef VMS
#define DIRSEP ']'			/* (not really) */
#define ISDIRSEP(c) ((c)==']'||(c)==':')
#else
#ifdef MAC
#define DIRSEP ':'
#define ISDIRSEP(c) ((c)==':')
#else
#ifdef AMIGA
#define DIRSEP '/'
#define ISDIRSEP(c) ((c)=='/'||(c)==':')
#else
#ifdef GEMDOS
#define DIRSEP '\\'
#define ISDIRSEP(c) ((c)=='\\'||(c)==':')
#else
#define DIRSEP '/'
#define ISDIRSEP(c) ((c)=='/')
#endif /* GEMDOS */
#endif /* AMIGA */
#endif /* MAC */
#endif /* VMS */
#endif /* STRATUS */
#endif /* datageneral */
#endif /* OS2 */
#endif /* UNIX */
#endif /* DIRSEP */

/* FILE package parameters */

#ifdef pdp11
#define NOCHANNELIO
#else

#ifndef CKMAXOPEN
#ifdef QNX
#define CKMAXOPEN 390
#else
#ifdef VMS
#define CKMAXOPEN 64
#else
#ifdef OPEN_MAX
#define CKMAXOPEN OPEN_MAX
#else
#ifdef FOPEN_MAX
#define CKMAXOPEN FOPEN_MAX
#else
#define CKMAXOPEN 64
#endif /* FOPEN_MAX */
#endif /* OPEN_MAX */
#endif /* VMS */
#endif /* QNX */
#endif /* CKMAXOPEN */

/* Maximum channels for FOPEN = CKMAXOPEN minus logs, stdio, etc */

#ifndef Z_MINCHAN
#define Z_MINCHAN 16
#endif /* Z_MINCHAN */

#ifndef Z_MAXCHAN
#define Z_MAXCHAN (CKMAXOPEN-ZNFILS-5)
#endif /* Z_MAXCHAN */
#endif /* pdp11 */

/* New-format nzltor() and nzrtol() functions that handle pathnames */

#ifndef NZLTOR
#ifdef UNIX
#define NZLTOR
#else
#ifdef VMS
#define NZLTOR
#else
#ifdef OS2
#define NZLTOR
#else
#ifdef STRATUS
#define NZLTOR
#endif /* STRATUS */
#endif /* OS2 */
#endif /* VMS */
#endif /* UNIX */
#endif /* NZLTOR */

#ifdef NZLTOR
_PROTOTYP( VOID nzltor, (char *, char *, int, int, int) );
_PROTOTYP( VOID nzrtol, (char *, char *, int, int, int) );
#endif /* NZLTOR */

/* Implementations with a zrmdir() function */

#ifndef ZRMDIR
#ifdef OS2
#define ZRMDIR
#else /* OS2 */
#ifdef UNIX
#define ZRMDIR
#else
#ifdef VMS
#define ZRMDIR
#else /* VMS */
#ifdef STRATUS
#define ZRMDIR
#endif /* STRATUS */
#endif /* VMS */
#endif /* UNIX */
#endif /* OS2 */
#endif /* ZRMDIR */

#ifdef ZRMDIR
_PROTOTYP( int zrmdir, (char *) );
#endif /* ZRMDIR */

#ifndef FILECASE
#ifdef UNIXOROSK
#define FILECASE 1
#else
#define FILECASE 0
#endif /* UNIXOROSK */
#ifndef CKCMAI
extern int filecase;
#endif /* CKCMAI */
#endif /* FILECASE */

/* Funny names for library functions department... */

#ifdef ZILOG
#define setjmp setret
#define longjmp longret
#define jmp_buf ret_buf
#define getcwd curdir
#endif /* ZILOG */

#ifdef STRATUS
/* The C-runtime conflicts with things we do in Stratus VOS ckltio.c ... */
#define printf vosprtf
_PROTOTYP( int vosprtf, (char *fmt, ...) );
#define perror(txt) printf("%s\n", txt)
/* char_varying is a string type from PL/I that VOS uses extensively */
#define CV char_varying
#endif /* STRATUS */

#ifdef NT
extern int OSVer;
#define isWin95() (OSVer==VER_PLATFORM_WIN32_WINDOWS)
#else
#define isWin95() (0)
#endif /* NT */

#ifndef BPRINT
#ifdef OS2
#define BPRINT
#endif /* OS2 */
#endif /* BPRINT */

#ifndef SESLIMIT
#ifdef OS2
#define SESLIMIT
#endif /* OS2 */
#endif /* SESLIMIT */

#ifndef NOTERM
#ifndef PCTERM
#ifdef NT
#define PCTERM
#endif /* NT */
#endif /* PCTERM */
#endif /* NOTERM */

#ifdef BEOSORBEBOX
#define query ckquery
#endif /* BEOSORBEBOX */

#ifndef PTYORPIPE			/* NETCMD and/or NETPTY defined */
#ifdef NETCMD
#define PTYORPIPE
#else
#ifdef NETPTY
#define PTYORPIPE
#endif /* NETPTY */
#endif /* NETCMD */
#endif /* PTYORPIPE */

/* mktemp() and mkstemp() */

#ifndef NOMKTEMP
#ifndef MKTEMP
#ifdef OS2ORUNIX
#define MKTEMP
#endif /* OS2ORUNIX */
#endif /* MKTEMP */

#ifdef MKTEMP
#ifndef NOMKSTEMP
#ifndef MKSTEMP
#ifdef BSD44
#define MKSTEMP
#else
#ifdef __linux__
#define MKSTEMP
#endif /* __linux__ */
#endif /* BSD44 */
#endif /* MKSTEMP */
#endif /* NOMKSTEMP */
#endif /* MKTEMP */
#endif /* NOMKTEMP */

/* Platforms that have memcpy() -- only after all headers included */

#ifndef USE_MEMCPY
#ifdef VMS
#define USE_MEMCPY
#else
#ifdef NEXT
#define USE_MEMCPY
#else
#ifdef OS2
#define USE_MEMCPY
#else
#ifdef __linux__
#define USE_MEMCPY
#else
#ifdef SOLARIS
#define USE_MEMCPY
#else
#ifdef SUNOS4
#define USE_MEMCPY
#else
#ifdef AIXRS
#define USE_MEMCPY
#else
#ifdef HPUX
#define USE_MEMCPY
#else
#ifdef POSIX
#define USE_MEMCPY
#else
#ifdef SVR4
#define USE_MEMCPY
#else
#ifdef OSF
#define USE_MEMCPY
#else
#ifdef datageneral
#define USE_MEMCPY
#else
#ifdef STRATUS
#define USE_MEMCPY
#endif /* STRATUS */
#endif /* datageneral */
#endif /* OSF */
#endif /* SVR4 */
#endif /* POSIX */
#endif /* HPUX */
#endif /* AIXRS */
#endif /* SUNOS4 */
#endif /* SOLARIS */
#endif /* __linux__ */
#endif /* OS2 */
#endif /* NEXT */
#endif /* VMS */
#endif /* USE_MEMCPY */

#ifndef USE_MEMCPY
#define memcpy(a,b,c) ckmemcpy((a),(b),(c))
#else
#ifdef CK_SCO32V4
/* Because the prototype isn't picked up in the normal header files */
_PROTOTYP( void *memcpy, (void *, const void *, size_t));
#endif /* CK_SCO32V4 */
#endif /* USE_MEMCPY */

/* User authentication for IKS -- So far K95 and UNIX only */

#ifdef NOICP
#ifndef NOLOGIN
#define NOLOGIN
#endif /* NOLOGIN */
#endif /* NOICP */

#ifndef NOLOGIN
#ifdef OS2ORUNIX
#ifndef CK_LOGIN
#define CK_LOGIN
#ifndef NOSHADOW
#ifdef CK_SCOV5
#define CK_SHADOW
#endif /* CK_SCOV5 */
#endif /* NOSHADOW */
#endif /* CK_LOGIN */
#ifdef NT
#define NTCREATETOKEN
#endif /* NT */
#endif /* OS2ORUNIX */
#else /* NOLOGIN */
#ifdef CK_LOGIN
#undef CK_LOGIN
#endif /* CK_LOGIN */
#endif /* NOLOGIN */

#ifdef OS2
#define CKSPINNER
#endif /* OS2 */

#ifdef CK_LOGIN				/* Telnet protocol required */
#ifndef TNCODE				/* for login to IKSD. */
#define TNCODE
#endif /* TNCODE */
#endif /* CK_LOGIN */

#ifdef CK_AUTHENTICATION
#ifdef NOSENDUID
#undef NOSENDUID
#endif /* NOSENDUID */
#endif /* CK_AUTHENTICATION */

#ifdef TNCODE				/* Should TELNET send user ID? */
#ifndef NOSENDUID
#ifndef CKSENDUID
#define CKSENDUID
#endif /* CKSENDUID */
#endif /* NOSENDUID */
#endif /* TNCODE */

/* UNIX platforms that don't have getusershell() */

#ifdef UNIX
#ifndef NOGETUSERSHELL
#ifdef IRIX
#define NOGETUSERSHELL
#else
#ifdef PTX
#define NOGETUSERSHELL
#else
#ifdef AIXRS
#define NOGETUSERSHELL
#else
#ifdef SINIX
#define NOGETUSERSHELL
#else
#ifdef UNIXWARE
#define NOGETUSERSHELL
#else
#ifdef COHERENT
#define NOGETUSERSHELL
#endif /* COHERENT */
#endif /* UNIXWARE */
#endif /* SINIX */
#endif /* AIXRS */
#endif /* PTX */
#endif /* IRIX */
#endif /* NOGETUSERSHELL */
#endif /* UNIX */

#ifdef CK_LOGIN
#ifdef NT
#ifndef NOSYSLOG
#ifndef CKSYSLOG
#define CKSYSLOG
#endif /* CKSYSLOG */
#endif /* NOSYSLOG */
#endif /* NT */
#ifdef UNIX
#ifndef NOSYSLOG
#ifndef CKSYSLOG
#define CKSYSLOG
#endif /* CKSYSLOG */
#endif /* NOSYSLOG */
#ifndef NOWTMP
#ifndef CKWTMP
#define CKWTMP
#endif /* CKWTMP */
#endif /* NOWTMP */
#ifndef NOGETUSERSHELL
#ifndef GETUSERSHELL
#define GETUSERSHELL
#endif /* GETUSERSHELL */
#endif /* NOGETUSERSHELL */
#endif /* UNIX */
_PROTOTYP( int ckxlogin, (CHAR *, CHAR *, CHAR *, int));
_PROTOTYP( int ckxlogout, (VOID));
#endif /* CK_LOGIN */

#ifndef NOZLOCALTIME			/* zlocaltime() available. */
#ifdef OS2ORUNIX
#define ZLOCALTIME
_PROTOTYP( char * zlocaltime, (char *) );
#endif /* OS2ORUNIX */
#endif /* NOZLOCALTIME */

#ifdef CKSYSLOG				/* Syslogging levels */
#define SYSLG_NO 0			/* No logging */
#define SYSLG_LI 1			/* Login/out */
#define SYSLG_DI 2			/* Dialing out */
#define SYSLG_AC 3			/* Making any kind of connection */
#define SYSLG_PR 4                      /* Protocol Operations */
#define SYSLG_FC 5			/* File creation */
#define SYSLG_FA 6			/* File reading */
#define SYSLG_CM 7			/* Top-level commands */
#define SYSLG_CX 8			/* All commands */
#define SYSLG_DB 9			/* Debug */
#define SYSLGMAX 9			/* Highest level */
#define SYSLG_DF SYSLG_FA		/* Default level */
/* Logging function */
_PROTOTYP(VOID cksyslog,(int, int, char *, char *, char *));
#endif /* CKSYSLOG */
#ifndef CKCMAI
extern int ckxlogging, ckxsyslog, ikdbopen;
#endif /* CKCMAI */

#ifndef CK_KEYTAB
#define CK_KEYTAB
/* Keyword Table Template */

/* Note: formerly defined in ckucmd.h but now more widely used */

struct keytab {				/* Keyword table */
    char *kwd;				/* Pointer to keyword string */
    int kwval;				/* Associated value */
    int flgs;				/* Flags (as defined above) */
};
#endif /* CK_KEYTAB */

#ifdef UNIX
_PROTOTYP( int isalink, (char *));
#endif	/* UNIX */

#ifdef NETPTY
_PROTOTYP( int do_pty, (int *, char *, int));
_PROTOTYP( VOID end_pty, (void));
#endif /* NETPTY */

#ifdef CKROOT
_PROTOTYP( int zsetroot, (char *) );
_PROTOTYP( char * zgetroot, (void) );
_PROTOTYP( int zinroot, (char *) );
#endif /* CKROOT */

/* Local Echo Buffer prototypes */
_PROTOTYP( VOID le_init, (void) );
_PROTOTYP( VOID le_clean, (void));
_PROTOTYP( int le_inbuf, (void));
_PROTOTYP( int le_putstr, (CHAR *));
_PROTOTYP( int le_puts, (CHAR *, int));
_PROTOTYP( int le_putchar, (CHAR));
_PROTOTYP( int le_getchar, (CHAR *));

/* #ifndef NOHTTP */
#ifndef NOCMDATE2TM
#ifndef CMDATE2TM
#ifdef OS2ORUNIX
#define CMDATE2TM
#endif /* OS2ORUNIX */
#ifdef VMS
#define CMDATE2TM
#endif /* VMS */
#endif /* CMDATE2TM */
#endif /* NOCMDATE2TM */

#ifdef CMDATE2TM
_PROTOTYP( struct tm * cmdate2tm, (char *,int));
#endif /* CMDATE2TM */
/* #endif */ /* NOHTTP */

#ifndef NOSETTIME			/* This would be set in CFLAGS */
#ifdef SVR4ORPOSIX			/* Defined in IEEE 1003.1-1996 */
#ifndef UTIMEH				/* and in SVID for SVR4 */
#define UTIMEH
#endif /* UTIMEH */
#else  /* SVR4ORPOSIX */
#ifdef OSF				/* Verified by Lucas Hart */
#ifndef UTIMEH
#define UTIMEH
#endif /* UTIMEH */
#else  /* OSF */
#ifdef SUNOS41				/* Verified by Lucas Hart */
#ifndef UTIMEH
#define UTIMEH
#endif /* UTIMEH */
#else  /* SUNOS41 */
#ifdef OS2
#ifndef SYSUTIMEH
#define SYSUTIMEH
#endif /* SYSUTIMEH */
#else /* OS2 */
#ifdef VMS
#ifndef UTIMEH
#define UTIMEH
#endif /* UTIMEH */
#endif /* VMS */
#endif /* OS2 */
#endif /* SUNOS41 */
#endif /* OSF */
#endif /* SVR4ORPOSIX */
#endif /* NOSETTIME */

#ifdef NEWFTP
_PROTOTYP( int ftpisconnected, (void));
_PROTOTYP( int ftpisloggedin, (void));
_PROTOTYP( int ftpissecure, (void));
#endif /* NEWFTP */

_PROTOTYP( int readpass, (char *, char *, int));
_PROTOTYP( int readtext, (char *, char *, int));

#ifdef OS2
_PROTOTYP(int ck_auth_loaddll, (VOID));
_PROTOTYP(int ck_auth_unloaddll, (VOID));
#endif /* OS2 */

#ifdef NT
_PROTOTYP(DWORD ckGetLongPathname,(LPCSTR lpFileName, 
                                   LPSTR lpBuffer, DWORD cchBuffer));
#endif /* NT */


#include "ckclib.h"

/* End of ckcdeb.h */
#endif /* CKCDEB_H */

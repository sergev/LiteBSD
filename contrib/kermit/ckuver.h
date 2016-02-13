/* ckuver.h -- C-Kermit UNIX Version heralds */
/*
  Author: Frank da Cruz <fdc@columbia.edu>,
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 2010,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/

#ifndef CKUVER_H
#define CKUVER_H

/* Arranged more or less alphabetically by compiler symbol */
/* Must be included AFTER ckcdeb.h. */

#ifdef BEOS
#ifdef BEOS45
#define HERALD " BeOS 4.5"
#else
#define HERALD " BeOS"
#endif /* BEOS45 */
#else
#ifdef BEBOX
#ifdef BE_DR_7
#define HERALD " BeBox DR7"
#else
#define HERALD " BeBox"
#endif /* BE_DR_7 */
#endif /* BEBOX */
#endif /* BEOS */

#ifdef BELLV10
#define HERALD " Bell Labs Research UNIX V10"
#endif /* BELLV10 */

#ifdef APOLLOSR10
#define HERALD " Apollo SR10"
#endif /* APOLLOSR10 */

#ifdef MAC
#define HERALD " Apple Macintosh"
#endif /* MAC */

#ifdef A986
#define HERALD " Altos 986 / Xenix 3.0"
#endif /* A986 */

#ifdef AS400
#define HERALD " AS/400"
#endif /* AS400 */

#ifdef aegis
#ifdef BSD4
#define HERALD " Apollo DOMAIN/IX 4.2 BSD"
#else
#ifdef ATTSV
#define HERALD " Apollo DOMAIN/IX System V"
#else
#define HERALD " Apollo Aegis"
#endif /* BSD4  */
#endif /* ATTSV */
#endif /* aegis */

#ifndef HERALD

#ifdef AIXRS

#ifdef AIX53
#define HERALD " IBM AIX 5.3"
#else
#ifdef AIX52
#define HERALD " IBM AIX 5.2"
#else
#ifdef AIX51
#define HERALD " IBM AIX 5.1"
#else
#ifdef AIX45
#define HERALD " IBM AIX 5.0"
#else
#ifdef AIX45
#define HERALD " IBM AIX 4.5"
#else
#ifdef AIX44
#define HERALD " IBM AIX 4.4"
#else
#ifdef AIX43
#define HERALD " IBM AIX 4.3"
#else
#ifdef AIX42
#define HERALD " IBM AIX 4.2"
#else
#ifdef SVR4
#ifdef AIX41
#define HERALD " IBM AIX 4.1"
#else
#define HERALD " IBM RS/6000 AIX 3.2"
#endif /* AIX41 */
#else
#define HERALD " IBM RS/6000 AIX 3.0/3.1"
#endif /* SVR4 */
#endif /* AIX42 */
#endif /* AIX43 */
#endif /* AIX44 */
#endif /* AIX45 */
#endif /* AIX50 */
#endif /* AIX51 */
#endif /* AIX52 */
#endif /* AIX53 */
#endif /* AIXRS */

#ifdef PS2AIX10
#define HERALD " IBM PS/2 AIX 1.x"
#endif /* PS2AIX10 */

#ifdef AIXPS2
#define HERALD " IBM PS/2 AIX 3.x"
#endif /* AIXPS2 */

#ifdef AIX370
#ifndef HERALD
#define HERALD " IBM System/370 AIX/370"
#endif
#endif /* AIX370 */

#ifdef AIXESA
#ifndef HERALD
#define HERALD " IBM AIX/ESA version 2.1"
#endif
#endif /* AIXESA */

#ifdef ATT6300
#define HERALD " AT&T 6300"
#endif /* ATT6300 */

#ifdef ATT7300
#ifdef UNIX351M
#define HERALD " AT&T 7300 UNIX PC UNIX 3.51m"
#else
#define HERALD " AT&T 7300 UNIX PC"
#endif /* UNIX351M */
#endif /* ATT7300 */

#ifdef AUX
#define HERALD " Apple Macintosh AUX"
#endif /* AUX */

#ifdef BSD44
#ifdef MACOSX
#define HERALD " Mac OS X"
#else
#ifdef __OpenBSD__
#define HERALD " OpenBSD"
#else
#ifdef __bsdi__
#ifdef BSDI4
#define HERALD " BSDI BSD/OS 4.0"
#else
#ifdef BSDI3
#define HERALD " BSDI BSD/OS 3.0"
#else
#ifdef BSDI2
#define HERALD " BSDI BSD/OS 2.0"	/* 1.1++ name... */
#else
#define HERALD " BSDI BSD/386"		/* Original 1.0 name */
#endif /* BSDI2 */
#endif /* BSDI3 */
#endif /* BSDI4 */
#else  /* __bsdi__ */
#ifdef __NetBSD__
#ifndef HERALD
#ifdef NETBSD16
#define HERALD " NetBSD 1.6"
#else
#ifdef NETBSD15
#define HERALD " NetBSD 1.5"
#else
#define HERALD " NetBSD"
#endif /* NETBSD15 */
#endif /* NETBSD16 */
#endif /* HERALD */
#else  /* __NetBSD__ */
#ifdef __FreeBSD__
#ifdef FREEBSD51
#define HERALD " FreeBSD 5.1"
#else
#ifdef FREEBSD50
#define HERALD " FreeBSD 5.0"
#else
#ifdef FREEBSD49
#define HERALD " FreeBSD 4.9"
#else
#ifdef FREEBSD48
#define HERALD " FreeBSD 4.8"
#else
#ifdef FREEBSD47
#define HERALD " FreeBSD 4.7"
#else
#ifdef FREEBSD46
#define HERALD " FreeBSD 4.6"
#else
#ifdef FREEBSD45
#define HERALD " FreeBSD 4.5"
#else
#ifdef FREEBSD44
#define HERALD " FreeBSD 4.4"
#else
#ifdef FREEBSD43
#define HERALD " FreeBSD 4.3"
#else
#ifdef FREEBSD42
#define HERALD " FreeBSD 4.2"
#else
#ifdef FREEBSD41
#define HERALD " FreeBSD 4.1"
#else
#ifdef FREEBSD4
#define HERALD " FreeBSD 4.0"
#else
#ifdef FREEBSD3
#define HERALD " FreeBSD 3.0"
#else
#ifdef FREEBSD2
#define HERALD " FreeBSD 2.0"
#else
#define HERALD " FreeBSD"
#endif /* FREEBSD2 */
#endif /* FREEBSD3 */
#endif /* FREEBSD4 */
#endif /* FREEBSD41 */
#endif /* FREEBSD42 */
#endif /* FREEBSD43 */
#endif /* FREEBSD44 */
#endif /* FREEBSD45 */
#endif /* FREEBSD46 */
#endif /* FREEBSD47 */
#endif /* FREEBSD48 */
#endif /* FREEBSD49 */
#endif /* FREEBSD50 */
#endif /* FREEBSD51 */
#else
#ifdef __386BSD__
#define HERALD " 386BSD"
#else
#define HERALD " 4.4BSD"
#endif /* __386BSD__ */
#endif /* __FreeBSD__ */
#endif /* __NetBSD__ */
#endif /* __bsdi__ */
#endif /* __OpenBSD__ */
#endif /* MACOSX */
#endif /* BSD44 */

#ifdef ENCORE
#ifdef BSD43
#define HERALD " Encore Multimax UMAX 4.3"
#else
#define HERALD " Encore Multimax UMAX 4.2"
#endif
#endif /* ENCORE */

#ifdef BSD29
#define HERALD " 2.9 BSD"
#endif /* BSD29 */

#ifdef BSD41
#define HERALD " 4.1 BSD"
#endif /* BSD41 */

#ifdef C70
#define HERALD " BBN C/70"
#endif /* c70 */

#ifdef CIE
#define HERALD " CIE Systems 680/20 Regulus"
#endif /* CIE */

#ifdef COHERENT
#ifdef _I386
#define HERALD " MWC Coherent 386 4.x"
#ifndef i386
#define i386
#endif /* i386 */
#else
#define HERALD " PC/AT MWC Coherent 286 3.x"
#ifndef i286
#define i286
#endif /* i286 */
#endif /* _I386 */
#endif /* COHERENT */

#ifdef CONVEX9
#define HERALD " Convex/OS"
#endif /* CONVEX9 */

#ifdef CONVEX10
#define HERALD " Convex/OS 10.1"
#endif /* CONVEX10 */

#ifdef _CRAY
#ifdef _CRAYCOM
#define HERALD " Cray CSOS"
#else /* _CRAYCOM */
#define HERALD " Cray UNICOS"
#endif /* _CRAYCOM */
#endif /* _CRAY */

#ifdef DGUX
#ifdef DGUX54420
#define HERALD " Data General DG/UX R4.20"
#else
#ifdef DGUX54411
#define HERALD " Data General DG/UX R4.11"
#else
#ifdef DGUX54410
#define HERALD " Data General DG/UX R4.10"
#else
#ifdef DGUX54310
#define HERALD " Data General DG/UX 5.4R3.10"
#else
#ifdef DGUX543
#define HERALD " Data General DG/UX 5.4R3.00"
#else
#ifdef DGUX540
#define HERALD " Data General DG/UX 5.4"
#else
#ifdef DGUX430
#define HERALD " Data General DG/UX 4.30"
#else
#define HERALD " Data General DG/UX"
#endif /* DGUX430 */
#endif /* DGUX540 */
#endif /* DGUX543 */
#endif /* DGUX54310 */
#endif /* DGUX54410 */
#endif /* DGUX54411 */
#endif /* DGUX54420 */
#endif /* DGUX */

#ifdef datageneral
#ifndef HERALD
#define HERALD " Data General AOS/VS"
#endif /* HERALD */
#endif /* datageneral */

#ifdef SINIX
#ifdef SNI544
#define HERALD " Siemens Nixdorf Reliant UNIX V5.44"
#else
#ifdef SNI543
#define HERALD " Siemens Nixdorf Reliant UNIX V5.43"
#else
#ifdef SNI541
#define HERALD " Siemens Nixdorf SINIX V5.41"
#else
#define HERALD " Siemens Nixdorf SINIX V5.42"
#endif /* SNI541 */
#endif /* SNI543 */
#endif /* SNI544 */
#endif /* SINIX */

#ifdef POWERMAX
#define HERALD " Concurrent PowerMAX OS"
#endif /* POWERMAX */

#ifdef DELL_SVR4
#define HERALD " Dell System V R4"
#endif /* DELL_SVR4 */

#ifdef NCRMPRAS
#define HERALD " NCR MP-RAS"
#endif /* NCRMPRAS */

#ifdef UNIXWARE
#define HERALD " UnixWare"
#else
#ifdef OLD_UNIXWARE
#define HERALD " UnixWare"
#endif /* OLD_UNIXWARE */
#endif /* UNIXWARE */

#ifdef ICL_SVR4
#define HERALD " ICL System V R4 DRS N/X"
#endif /* ICL_SVR4 */

#ifdef FT18
#ifdef FT21
#define HERALD " Fortune For:Pro 2.1"
#else
#define HERALD " Fortune For:Pro 1.8"
#endif /* FT21 */
#endif /* FT18 */

#ifdef GEMDOS
#define HERALD " Atari ST GEM 1.0"
#endif /* GEMDOS */

#ifdef XF68R3V6
#define HERALD " Motorola UNIX System V/68 R3V6"
#endif /* XF68R3V6 */

#ifdef XF88R32
#define HERALD " Motorola UNIX System V/88 R32"
#endif /* XF88R32 */

#ifdef I386IX
#ifdef SVR3JC
#define HERALD " Interactive UNIX System V/386 R3.2"
#else
#define HERALD " Interactive Systems Corp 386/ix"
#endif /* SVR3JC */
#endif /* I386IX */

#ifdef IRIX65
#define HERALD " Silicon Graphics IRIX 6.5"
#else
#ifdef IRIX64
#define HERALD " Silicon Graphics IRIX 6.4"
#else
#ifdef IRIX63
#define HERALD " Silicon Graphics IRIX 6.3"
#else
#ifdef IRIX62
#define HERALD " Silicon Graphics IRIX 6.2"
#else
#ifdef IRIX60
#define HERALD " Silicon Graphics IRIX 6.0"
#else
#ifdef IRIX53
#define HERALD " Silicon Graphics IRIX 5.3"
#else
#ifdef IRIX52
#define HERALD " Silicon Graphics IRIX 5.2"
#else
#ifdef IRIX51
#define HERALD " Silicon Graphics IRIX 5.1"
#else
#ifdef IRIX40
#define HERALD " Silicon Graphics IRIX 4.0"
#endif /* IRIX40 */
#endif /* IRIX51 */
#endif /* IRIX52 */
#endif /* IRIX53 */
#endif /* IRIX60 */
#endif /* IRIX62 */
#endif /* IRIX63 */
#endif /* IRIX64 */
#endif /* IRIX65 */

#ifdef ISIII
#define HERALD " Interactive Systems Corp System III"
#endif /* ISIII */

#ifdef IX370
#define HERALD " IBM IX/370"
#endif /* IX370 */

#ifdef HPUX
#ifdef HPUX5
#define HERALD " HP-UX 5.00"
#else
#ifdef HPUX6
#define HERALD " HP-UX 6.00"
#else
#ifdef HPUX7
#define HERALD " HP-UX 7.00"
#else
#ifdef HPUX8
#define HERALD " HP-UX 8.00"
#else
#ifdef HPUX9
#define HERALD " HP-UX 9.00"
#else
#ifdef HPUX1100
#define HERALD " HP-UX 11.00"
#else
#ifdef HPUX10
#ifdef HPUX1030
#define HERALD " HP-UX 10.30"
#else
#ifdef HPUX1020
#define HERALD " HP-UX 10.20"
#else
#ifdef HPUX1010
#define HERALD " HP-UX 10.10"
#else
#ifdef HPUX10xx
#define HERALD " HP-UX 10.xx"
#else
#define HERALD " HP-UX 10.00"
#endif /* HPUX10XX */
#endif /* HPUX1010 */
#endif /* HPUX1020 */
#endif /* HPUX1030 */
#else
#define HERALD " HP-UX"
#endif /* HPUX10 */
#endif /* HPUX1100 */
#endif /* HPUX9  */
#endif /* HPUX8  */
#endif /* HPUX7  */
#endif /* HPUX6  */
#endif /* HPUX5  */
#endif /* HPUX   */

#ifndef MINIX
#ifdef MINIX315
#define MINIX
#endif	/* MINIX315 */
#endif	/* MINIX */

#ifndef MINIX
#ifdef MINIX3
#define MINIX
#endif	/* MINIX3 */
#endif	/* MINIX */

#ifdef MINIX
#ifdef MINIX315
#define HERALD " Minix 3.1.5"
#ifndef MINIX3
#define MINIX3
#endif	/* MINIX3 */
#endif	/* MINIX315 */
#ifdef MINIX3
#ifndef MINIX2
#define MINIX2
#endif	/* MINIX2 */
#ifndef HERALD
#define HERALD " Minix 3.0"
#endif	/* HERALD */
#else
#ifdef MINIX2
#define HERALD " Minix 2.0"
#else
#define HERALD " Minix 1.0"
#endif /* MINIX3 */
#endif /* MINIX2 */
#endif /* MINIX */

#ifdef MIPS
#define HERALD " MIPS RISC/OS SVR3"
#endif /* MIPS */

#ifdef NEXT
#ifdef OPENSTEP42
#define HERALD " OPENSTEP 4.2"
#else
#ifdef NEXT33
#define HERALD " NeXTSTEP 3.3"
#else
#define HERALD " NeXTSTEP"
#endif /* NEXT33 */
#endif /* OPENSTEP42 */
#endif /* NEXT */

#ifdef OSF
#ifdef i386
#define HERALD " DECpc OSF/1"
#ifdef __GNUC
#define OSFPC
#endif /* __GNUC */
#else  /* Not i386 so Alpha */

#ifdef TRU64

#ifdef OSF51B
#define HERALD " Compaq Tru64 UNIX 5.1B"
#else
#ifdef OSF51A
#define HERALD " Compaq Tru64 UNIX 5.1A"
#else
#ifdef OSF50
#define HERALD " Compaq Tru64 UNIX 5.0A"
#else
#ifdef OSF40G
#define HERALD " Compaq Tru64 UNIX 4.0G"
#else
#ifdef OSF40F
#define HERALD " Compaq Tru64 UNIX 4.0F"
#else
#ifdef OSF40E
#define HERALD " Compaq Tru64 UNIX 4.0E"
#endif /* OSF40E */
#endif /* OSF40F */
#endif /* OSF40G */
#endif /* OSF50 */
#endif /* OSF51A */
#endif /* OSF51B */

#else  /* Not TRU64 */

#ifdef OSF40
#define HERALD " Digital UNIX 4.0"
#else
#ifdef OSF32
#define HERALD " Digital UNIX 3.2"
#else
#define HERALD " DEC OSF/1 Alpha"
#endif /* OSF40 */
#endif /* OSF32 */

#endif /* TRU64 */
#endif /* i386 */
#endif /* OSF */

#ifdef PCIX
#define HERALD " PC/IX"
#endif /* PCIX */

#ifdef sxaE50
#define HERALD " PFU SX/A V10/L50"
#endif /* sxaE50 */

#ifdef PROVX1
#define HERALD " DEC Professional 300 (Venix 1.0)"
#endif /* PROVX1 */

#ifdef PYRAMID
#ifdef SVR4
#define HERALD " Pyramid DC/OSx"
#else
#define HERALD " Pyramid Dual Port OSx"
#endif /* SVR4 */
#endif /* PYRAMID */

#ifdef RTAIX
#define HERALD " IBM RT PC (AIX 2.2)"
#endif /* RTAIX */

#ifdef RTU
#define HERALD " Masscomp/Concurrent RTU"
#endif /* RTU */

#ifdef sony_news
#define HERALD " SONY NEWS"
#endif /* sony_news */

#ifdef SOLARIS24
#define HERALD " Solaris 2.4"
#else
#ifdef SOLARIS23
#define HERALD " Solaris 2.3"
#else
#ifdef SOLARIS
#define HERALD " Solaris 2.x"
#endif /* SOLARIS */
#endif /* SOLARIS23 */
#endif /* SOLARIS24 */

#ifdef SUNOS4
#ifdef BSD4
#ifdef SUNOS41
#define HERALD " SunOS 4.1"
#else
#define HERALD " SunOS 4.0"
#endif /* SUNOS41 */
#endif /* BSD4 */
#endif /* SUNOS4 */

#ifdef SUN4S5
#ifdef HDBUUCP
#define HERALD " SunOS 4.1 (SVR3)"
#else
#define HERALD " SunOS 4.0 (SVR3)"
#endif /* HDBUUCP */
#endif /* SUN4S5 */

#ifdef STRATUS
#define HERALD " Stratus VOS"
#endif /* STRATUS */

#ifdef TOWER1
#define HERALD " NCR Tower 1632 OS 1.02"
#endif /* TOWER1 */

#ifdef TRS16
#define HERALD " Tandy 16/6000 Xenix 3.0"
#ifndef CKCPU
#define CKCPU "mc68000"
#endif /* CKCPU */
#endif /* TRS16 */

#ifdef u3b2
#ifndef HERALD
#ifdef SVR3
#define HERALD " AT&T 3B2 System V R3"
#else
#define HERALD " AT&T 3B2 System V"
#endif /* SVR3 */
#endif /* HERALD */
#endif /* u3b2 */

#ifdef ultrix
#ifdef vax
#ifdef ULTRIX3
#define HERALD " VAX/ULTRIX 3.0"
#else
#define HERALD " VAX/ULTRIX"
#endif /* ULTRIX3 */
#else
#ifdef mips
#ifdef ULTRIX43
#define HERALD " DECstation/ULTRIX 4.3"
#else
#ifdef ULTRIX44
#define HERALD " DECstation/ULTRIX 4.4"
#else
#ifdef ULTRIX45
#define HERALD " DECstation/ULTRIX 4.5"
#else
#define HERALD " DECstation/ULTRIX"
#endif /* ULTRIX45 */
#endif /* ULTRIX44 */
#endif /* ULTRIX43 */
#else
#define HERALD " ULTRIX"
#endif /* mips */
#endif /* vax */
#endif /* ultrix */

#ifdef OXOS
#define HERALD " Olivetti X/OS"
#endif /* OXOS */

#ifdef _386BSD
#define HERALD " 386BSD"
#endif /* _386BSD */

#ifdef POSIX
#ifdef PTX
#ifdef PTX4
#define HERALD " DYNIX/ptx V4"
#else
#define HERALD " DYNIX/ptx"
#endif /* PTX4 */
#else  /* PTX */
#ifndef OSF		/* Let OSF -DPOSIX keep previously defined HERALD */
#ifdef HERALD
#undef HERALD
#endif /* HERALD */
#endif /* OSF */
#ifdef OU8
#define HERALD " OpenUNIX 8"
#else
#ifdef UW7
#define HERALD " Unixware 7"
#else
#ifdef QNX
#ifdef QNX16
#define HERALD " QNX 16-bit"
#else
#define HERALD " QNX 32-bit"
#endif /* QNX16 */
#else
#ifdef NEUTRINO
#define HERALD " QNX Neutrino 2"
#else  /* NEUTRINO */
#ifdef QNX6
#define HERALD " QNX6"
#else  /* QNX6 */
#ifdef __linux__
#ifdef ZSL5500
#define HERALD " Sharp Zaurus SL-5500"
#else
#ifdef RH90
#define HERALD " Red Hat Linux 9.0"
#else
#ifdef RH80
#define HERALD " Red Hat Linux 8.0"
#else
#ifdef RH73
#define HERALD " Red Hat Linux 7.3"
#else
#ifdef RH72
#define HERALD " Red Hat Linux 7.2"
#else
#ifdef RH71
#define HERALD " Red Hat Linux 7.1"
#else
#define HERALD " Linux"
#endif /* RH71 */
#endif /* RH72 */
#endif /* RH73 */
#endif /* RH80 */
#endif /* RH90 */
#endif /* ZSL5500 */
#else  /* __linux__ */
#ifdef _386BSD				/* 386BSD Jolix */
#define HERALD " 386BSD"
#else
#ifdef LYNXOS				/* Lynx OS 2.2 */
#define HERALD " Lynx OS"
#else
#ifdef Plan9
#define HERALD " Plan 9 from Bell Labs"
#else
#ifdef SOLARIS11
#define HERALD " Solaris 11"
#else
#ifdef SOLARIS10
#define HERALD " Solaris 10"
#else
#ifdef SOLARIS9
#define HERALD " Solaris 9"
#else
#ifdef SOLARIS8
#define HERALD " Solaris 8"
#else
#ifdef SOLARIS7
#define HERALD " Solaris 7"
#else
#ifdef SOLARIS26
#define HERALD " Solaris 2.6"
#else
#ifdef SOLARIS25
#define HERALD " Solaris 2.5"
#else
#ifdef SOLARIS24
#define HERALD " Solaris 2.4"
#else
#ifdef SOLARIS
#define HERALD " Solaris 2.x"
#endif /* SOLARIS */
#endif /* SOLARIS24 */
#endif /* SOLARIS25 */
#endif /* SOLARIS26 */
#endif /* SOLARIS7 */
#endif /* SOLARIS8 */
#endif /* SOLARIS9 */
#endif /* SOLARIS10 */
#endif /* SOLARIS11 */
#endif /* Plan9 */
#endif /* LYNXOS */
#endif /* _386BSD */
#endif /* __linux__ */
#endif /* QNX6 */
#endif /* NEUTRINO */
#endif /* QNX */
#endif /* UW7 */
#endif /* OU8 */
#endif /* PTX */
#endif /* POSIX */

#ifdef UTS24
#define HERALD " Amdahl UTS 2.4"
#endif /* UTS24 */

#ifdef UTSV
#define HERALD " Amdahl UTS V"
#endif /* UTSV */

#ifdef VXVE
#define HERALD " CDC VX/VE 5.2.1 System V"
#endif /* VXVE */

#ifdef SCO234
#ifdef HERALD
#undef HERALD
#endif /* HERALD */
#define HERALD " SCO XENIX 2.3.4"
#else
#ifdef CK_SCO32V4
#ifdef HERALD
#undef HERALD
#endif /* HERALD */
#ifdef ODT30
#define HERALD " SCO ODT 3.0"
#else
#define HERALD " SCO UNIX/386 V4"
#endif /* ODT30 */
#else
#ifdef CK_SCOV5
#ifdef HERALD
#undef HERALD
#endif /* HERALD */
#ifdef SCO_OSR507
#define HERALD " SCO OpenServer R5.0.7"
#else
#ifdef SCO_OSR506A
#define HERALD " SCO OpenServer R5.0.6a"
#else
#ifdef SCO_OSR506
#define HERALD " SCO OpenServer R5.0.6"
#else
#ifdef SCO_OSR505
#define HERALD " SCO OpenServer R5.0.5"
#else
#ifdef SCO_OSR504
#define HERALD " SCO OpenServer R5.0.4"
#else
#ifdef SCO_OSR502
#define HERALD " SCO OpenServer R5.0.2"
#else
#define HERALD " SCO OpenServer R5.0"
#endif /* SCO_OSR502 */
#endif /* SCO_OSR504 */
#endif /* SCO_OSR505 */
#endif /* SCO_OSR506 */
#endif /* SCO_OSR506A */
#endif /* SCO_OSR507 */
#else
#ifdef XENIX
#ifdef HERALD
#undef HERALD
#endif /* HERALD */
#ifdef M_UNIX
#define HERALD " SCO UNIX/386"
#else
#ifdef M_I386
#define HERALD " Xenix/386"
#else
#ifdef M_I286
#define HERALD " Xenix/286"
#else
#define HERALD " Xenix"
#endif /* M_I286 */
#endif /* M_I386 */
#endif /* M_UNIX */
#endif /* XENIX  */
#endif /* CK_SCOV5 */
#endif /* CK_SCOV32V4 */
#endif /* SCO234 */

#ifdef ZILOG
#define HERALD " Zilog S8000 Zeus 3.21+"
#endif /* ZILOG */

#ifdef UTEK
#define HERALD " UTek"
#endif /* UTEK */

/* Catch-alls for anything not defined explicitly above */

#ifndef HERALD
#ifdef SVR4
#ifdef i386
#define HERALD " AT&T System V/386 R4"
#else
#ifdef AMIX
#define HERALD " Commodore Amiga System V/m68k R4"
#else
#define HERALD " AT&T System V R4"
#endif /* AMIX */
#endif /* i386 */
#else
#ifdef SVR3
#define HERALD " AT&T System V R3"
#else
#ifdef ATTSV
#define HERALD " AT&T System III / System V"
#else
#ifdef BSD43
#ifdef pdp11
#define HERALD " 2.10 BSD PDP-11"
#else
#ifdef vax
#define HERALD " 4.3 BSD VAX"
#else
#define HERALD " 4.3 BSD"
#endif /* vax */
#endif /* pdp11 */
#else
#ifdef BSD4
#ifdef vax
#define HERALD " 4.2 BSD VAX"
#else
#define HERALD " 4.2 BSD"
#endif /* vax */
#else
#ifdef V7
#define HERALD " UNIX Version 7"
#endif /* V7 */
#endif /* BSD4 */
#endif /* BSD43 */
#endif /* ATTSV */
#endif /* SVR3 */
#endif /* SVR4 */
#endif /* HERALD */
#endif /* HERALD */

#ifdef OS2
#ifdef HERALD
#undef HERALD
#endif /* HERALD */
#ifdef NT
#define HERALD " 32-bit Windows"
#else /* NT */
#define HERALD " 32-bit OS/2"
#endif /* NT */
#endif /* OS/2 */

#ifndef HERALD
#define HERALD " Unknown Version"
#endif /* HERALD */

/* Hardware type */

#ifdef vax				/* DEC VAX */
#ifndef CKCPU
#define CKCPU "vax"
#endif /* CKCPU */
#endif /*  vax */
#ifdef pdp11				/* DEC PDP-11 */
#ifndef CKCPU
#define CKCPU "pdp11"
#endif /* CKCPU */
#endif /* pdp11 */

#ifdef __ALPHA				/* DEC Alpha */
#ifndef CKCPU
#define CKCPU "Alpha"
#endif /* CKCPU */
#endif /* __ALPHA */

#ifdef __alpha				/* OSF/1 uses lowercase... */
#ifndef CKCPU
#define CKCPU "Alpha"
#endif /* CKCPU */
#endif /* __alpha */

#ifdef DGUX				/* Override Motorola 88k assumption */
#ifndef CKCPU				/* New AViiONs are Intel based... */
#ifdef i586
#define CKCPU "i586"
#else
#ifdef i486
#define CKCPU "i486"
#else
#ifdef i386
#define CKCPU "i386"
#endif /* i386 */
#endif /* i486 */
#endif /* i586 */
#endif /* CKCPU */
#endif /* DGUX */

/* HP 9000 */

#ifdef __hp9000s700
#ifndef CKCPU
#define CKCPU "hp9000s700"
#endif /* CKCPU */
#endif /* __hp9000s700 */

#ifdef __hp9000s800
#ifndef CKCPU
#define CKCPU "hp9000s800"
#endif /* CKCPU */
#endif /* __hp9000s800 */

#ifdef __hp9000s500
#ifndef CKCPU
#define CKCPU "hp9000s500"
#endif /* CKCPU */
#endif /* __hp9000s500 */

#ifdef __hp9000s400
#ifndef CKCPU
#define CKCPU "hp9000s400"
#endif /* CKCPU */
#endif /* __hp9000s400 */

#ifdef __hp9000s300
#ifndef CKCPU
#define CKCPU "hp9000s300"
#endif /* CKCPU */
#endif /* __hp9000s300 */

#ifdef __hp9000s200
#ifndef CKCPU
#define CKCPU "hp9000s200"
#endif /* CKCPU */
#endif /* __hp9000s200 */

#ifdef m88000				/* Motorola 88000 */
#ifndef CKCPU
#define CKCPU "mc88000"
#endif /* CKCPU */
#endif /* m88000 */
#ifdef __using_M88KBCS			/* DG symbol for Motorola 88000 */
#ifndef CKCPU
#define CKCPU "mc88000"
#endif /* CKCPU */
#endif /* __using_M88KBCS */
#ifdef m88k				/* Motorola symbol for 88000 */
#ifndef CKCPU
#define CKCPU "mc88000"
#endif /* CKCPU */
#endif /* m88k */
#ifdef mc68040				/* Motorola 68040 */
#ifndef CKCPU
#define CKCPU "mc68040"
#endif /* CKCPU */
#endif /* mc68040 */
#ifdef mc68030				/* Motorola 68030 */
#ifndef CKCPU
#define CKCPU "mc68030"
#endif /* CKCPU */
#endif /* mc68030 */
#ifdef mc68020				/* Motorola 68020 */
#ifndef CKCPU
#define CKCPU "mc68020"
#endif /* CKCPU */
#endif /* mc68020 */
#ifdef mc68010				/* Motorola 68010 */
#ifndef CKCPU
#define CKCPU "mc68010"
#endif /* CKCPU */
#endif /* mc68010 */
#ifdef mc68000				/* Motorola 68000 */
#ifndef CKCPU
#define CKCPU "mc68000"
#endif /* CKCPU */
#endif /* mc68000 */
#ifdef mc68k				/* Ditto (used by DIAB DS90) */
#ifndef CKCPU
#define CKCPU "mc68000"
#endif /* CKCPU */
#endif /* mc68k */
#ifdef m68				/* Ditto */
#ifndef CKCPU
#define CKCPU "mc68000"
#endif /* CKCPU */
#endif /* m68 */
#ifdef m68k				/* Ditto */
#ifndef CKCPU
#define CKCPU "mc68000"
#endif /* CKCPU */
#endif /* m68k */

#ifdef ia64				/* IA64 / Itanium */
#ifndef CKCPU
#define CKCPU "ia64"
#endif /* CKCPU */
#endif /* i686 */

#ifdef i686				/* Intel 80686 */
#ifndef CKCPU
#define CKCPU "i686"
#endif /* CKCPU */
#endif /* i686 */

#ifdef i586				/* Intel 80586 */
#ifndef CKCPU
#define CKCPU "i586"
#endif /* CKCPU */
#endif /* i586 */

#ifdef i486				/* Intel 80486 */
#ifndef CKCPU
#define CKCPU "i486"
#endif /* CKCPU */
#endif /* i80486 */
#ifdef i386				/* Intel 80386 */
#ifndef CKCPU
#define CKCPU "i386"
#endif /* CKCPU */
#endif /* i80386 */
#ifdef i286				/* Intel 80286 */
#ifndef CKCPU
#define CKCPU "i286"
#endif /* CKCPU */
#endif /* i286 */
#ifdef i186				/* Intel 80186 */
#ifndef CKCPU
#define CKCPU "i186"
#endif /* CKCPU */
#endif /* i186 */
#ifdef M_I586				/* Intel 80586 */
#ifndef CKCPU
#define CKCPU "i586"
#endif /* CKCPU */
#endif /* M_I586 */
#ifdef M_I486				/* Intel 80486 */
#ifndef CKCPU
#define CKCPU "i486"
#endif /* CKCPU */
#endif /* M_I486 */
#ifdef _M_I386				/* Intel 80386 */
#ifndef CKCPU
#define CKCPU "i386"
#endif /* CKCPU */
#endif /* _M_I386 */
#ifdef M_I286				/* Intel 80286 */
#ifndef CKCPU
#define CKCPU "i286"
#endif /* CKCPU */
#endif /* M_I286 */
#ifdef M_I86				/* Intel 80x86 */
#ifndef CKCPU
#define CKCPU "ix86"
#endif /* CKCPU */
#endif /* M_I86 */
#ifdef sparc				/* SUN SPARC */
#ifndef CKCPU
#define CKCPU "sparc"
#endif /* CKCPU */
#endif /* sparc */
#ifdef mips				/* MIPS RISC processor */
#ifndef CKCPU
#define CKCPU "mips"
#endif /* CKCPU */
#endif /* mips */
#ifdef _IBMR2				/* IBM RS/6000 */
#ifndef CKCPU				/* (what do they call the chip?) */
#define CKCPU "rs6000"
#endif /* CKCPU */
#endif /* rs6000 */
#ifdef u3b5				/* WE32000 MAC-32, AT&T 3Bx */
#ifndef CKCPU
#define CKCPU "u3b5"
#endif /* CKCPU */
#endif /* u3b5 */
#ifdef n3b
#ifndef CKCPU
#define CKCPU "n3b"
#endif /* CKCPU */
#endif /* n3b */
#ifdef u3b
#ifndef CKCPU
#define CKCPU "u3b"
#endif /* CKCPU */
#endif /* u3b */
#ifdef n16				/* Encore Multimax */
#ifndef CKCPU
#define CKCPU "n16"
#endif /* CKCPU */
#endif /* n16 */
#ifdef u370				/* IBM 370 */
#ifndef CKCPU
#define CKCPU "u370"
#endif /* CKCPU */
#endif /* u370 */
#ifdef MAC				/* Macintosh catch-all */
#ifndef CKCPU
#define CKCPU "mc68000"
#endif /* CKCPU */
#endif /* MAC */

#ifdef STRATUS
#ifndef CKCPU
#ifdef __I860__
#define CKCPU "I860 Family"
#else
#ifdef __MC68K__
#define CKCPU "MC680x0 Family"
#else
#define CKCPU "Stratus unknown processor"
#endif /* __MC68K__ */
#endif /* __I860__ */
#endif /* CKCPU */
#endif /* STRATUS */

#ifdef COMMENT
#ifndef CKCPU				/* All others */
#define CKCPU "unknown"
#endif /* CKCPU */
#endif /* COMMENT */

#endif /* CKUVER_H */

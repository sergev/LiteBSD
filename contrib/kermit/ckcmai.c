#define EDITDATE  "20 Aug 2011"		/* Last edit date dd mmm yyyy */
#define EDITNDATE "20110820"		/* Keep them in sync */
/* Sat Aug 20 17:20:17 2011 */

/* ckcmai.c - Main program for C-Kermit plus some miscellaneous functions */

/*
  ckcsym.h is used for for defining symbols that normally would be defined
  using -D or -d on the cc command line, for use with compilers that don't
  support this feature.  Must come before any tests for preprocessor symbols.
*/
#include "ckcsym.h"
/*
  Consolidated program C-Kermit version information for all platforms
  (but for UNIX also see ckuver.h).  See makever() below for how they are used.
*/
#ifdef COMMENT                    /* Uncomment this for real K95 version */
#ifndef OS2				/* OS2 actually means Kermit 95. */
#ifndef BETATEST			/* It's because Kermit 95 started */
#define BETATEST			/* out as C-Kermit for OS/2. */
#endif /* BETATEST */
#endif /* OS2 */
#endif /* COMMENT */

#ifdef BETATEST
#ifdef OS2
#ifdef __DATE__
#define BETADATE
#endif /* __DATE__ */
#endif /* OS2 */
#endif /* BETATEST */

char * ck_cryear = "2011"; 		/* C-Kermit copyright year */

#ifndef MAC				/* MAC = Kermit for MAC OS 6, 7, ... */
/*
  Note: initialize ck_s_test to "" if this is not a test version.
  Use (*ck_s_test != '\0') to decide whether to print test-related messages.
*/

#ifndef BETATEST
#ifndef OS2                             /* UNIX, VMS, etc... (i.e. C-Kermit) */
char *ck_s_test = "";			/* "Dev","Alpha","Beta","RC", or "" */
char *ck_s_tver = "";			/* Test version number or "" */
#else  /* OS2 */
char *ck_s_test = "";			/* (i.e. K95) */
char *ck_s_tver = "";
#endif /* OS2 */
#else
char *ck_s_test = "";			/* Development */
char *ck_s_tver = "";
#endif /* BETATEST */
#else /* MAC */
char *ck_s_test = "Pre-Alpha";          /* Mac Kermit is always a test... */
char *ck_s_tver = "";			/* (pre Mac OS X 10, that is!) */
#endif /* MAC */

#ifdef BETADATE                         /* Date of this version or edit */
char *ck_s_date = __DATE__;             /* Compilation date */
#else
char *ck_s_date = EDITDATE;		/* See top */

#endif /* BETADATE */
char *buildid = EDITNDATE;		/* See top */

#ifdef UNIX
static char sccsid[] = "@(#)C-Kermit 9.0.302";
#endif /* UNIX */

/*
  The C-Kermit Version number is major.minor.edit (integers).
  Major version always goes up.
  Minor version is historical, hasn't been used since C-Kermit 7.1.
  Edit is sequential, always goes up, but there can be gaps.
  For example there might be many edits between releases.
  If the major goes to 10, some version-number-based feature tests
  could fail.  It might be better to use the minor version field
  for future releases.
*/

char *ck_s_ver = "9.0.302";             /* C-Kermit version string */
long  ck_l_ver =  900302L;              /* C-Kermit version number */

#ifdef OS2
char *ck_s_xver = "3.0.0";		/* Product-specific version string */
long  ck_l_xver = 3000L;                /* Product-specific version number */
#else
#ifdef MAC
char *ck_s_xver = "0.995";              /* Product-specific version string */
long  ck_l_xver = 995L;                 /* Product-specific version number */
#else
char *ck_s_xver = "";                   /* Don't touch these... */
long  ck_l_xver = 0L;                   /* they are computed at runtime */
#endif /* MAC */
#endif /* OS2 */

#ifdef OS2
#ifdef IKSDONLY
#ifdef NT
char *ck_s_name = "IKS-NT";
#else /* NT */
char *ck_s_name = "IKS-OS/2";
#endif /* NT */
#else /* IKSDONLY */
char *ck_s_name = "Kermit 95";          /* Program name */
#endif /* IKSDONLY */
#else
#ifdef MAC
char *ck_s_name = "Mac Kermit";
#else
char *ck_s_name = "C-Kermit";
#endif /* MAC */
#endif /* OS2 */

char *ck_s_who = "";                    /* Where customized, "" = not. */
char *ck_patch = "";                    /* Patch info, if any. */

#define CKVERLEN 128
char versiox[CKVERLEN];                 /* Version string buffer  */
char *versio = versiox;                 /* These are filled in at */
long vernum, xvernum;                   /* runtime from above.    */

#define CKCMAI

#include "ckcasc.h"                     /* ASCII character symbols */
#include "ckcdeb.h"                     /* Debug & other symbols */

char * myname = NULL;                   /* The name I am called by */
#ifndef OS2
char * exedir = NULL;                   /* Directory I was executed from */
#endif /* OS2 */
char * myhome = NULL;			/* Home directory override */

/*  C K C M A I  --  C-Kermit Main program  */

/*
  Author: Frank da Cruz (fdc@columbia.edu),
  Columbia University in the city of New York,
  Computer Center / Center for Computing Activities / Information Technology.
  I am no longer at Columbia U as of 1 July 2011, but the email address
  should still work.  The Kermit website http://kermit.columbia.edu should
  still be available and under my control, as well as the Kermit FTP site,
  ftp://kermit.columbia.edu/kermit/.

COPYRIGHT NOTICE:
*/

char *copyright[] = {

#ifdef pdp11
"Copyright (C) 1985, %s, Trustees of Columbia University, NYC.",
"All rights reserved.",
" ",
#else
"Copyright (C) 1985, %s,",
"  The Trustees of Columbia University in the City of New York.",
"  All rights reserved.",
" ",
"Redistribution and use in source and binary forms, with or without",
"modification, are permitted provided that the following conditions",
"are met:",
" ",
" + Redistributions of source code must retain the above copyright",
"   notice, this list of conditions and the following disclaimer.",
" ",
" + Redistributions in binary form must reproduce the above copyright",
"   notice, this list of conditions and the following disclaimer in",
"   the documentation and/or other materials provided with the",
"   distribution.",
" ",
" + Neither the name of Columbia University nor the names of its",
"   contributors may be used to endorse or promote products derived",
"   from this software without specific prior written permission.",
" ",
"THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS",
"\"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT",
"LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR",
"A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT",
"HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,",
"SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT",
"LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,",
"DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY",
"THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT",
"(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE",
"OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.",
#endif /* pdp11 */

#ifdef OS2
"Portions Copyright (C) 2002-2005, Secure Endpoints Inc, New York NY USA.",
"Portions Copyright (C) 1995, Oy Online Solutions Ltd., Jyvaskyla, Finland.",
#endif /* OS2 */

#ifdef CK_AUTHENTICATION
"Portions Copyright (C) 1990, Massachusetts Institute of Technology.",
#ifdef CK_ENCRYPTION
"Portions Copyright (C) 1991, 1993 Regents of the University of California.",
"Portions Copyright (C) 1991, 1992, 1993, 1994, 1995 by AT&T.",
"Portions Copyright (C) 1995, 1997, Eric Young <eay@cryptosoft.com>.",
#endif /* CK_ENCRYPTION */
#ifdef CK_SRP
"Portions Copyright (C) 1997, Stanford University.",
#endif /* CK_SRP */
#endif /* CK_AUTHENTICATION */

#ifndef pdp11
" ",
"For further information, visit the Kermit Project website:",
"http://www.columbia.edu/kermit/ .",
#endif /* pdp11 */
""};

/* Windows IKSD copyright used to be separate */
char *wiksdcpr = (char *) copyright;


/*
DOCUMENTATION:

 "Using C-Kermit" by Frank da Cruz and Christine M. Gianone,
  Digital Press / Butterworth-Heinemann, Woburn MA, USA.
  Second edition (1997), ISBN 1-55558-164-1.
  Order from Digital Press:    +1 (800) 366-2665
  Or from Columbia University: +1 (212) 854-3703

For Kermit 95, also:

  "Kermit 95" by Christine M. Gianone and Frank da Cruz,
  Manning Publications, Greenwich CT, USA (1998) - Online.

ACKNOWLEDGMENTS:

  The Kermit file transfer protocol was developed at the Columbia University
  Center for Computing Activities (CUCCA), which was since renamed to Columbia
  University Academic Information Systems (AcIS) and after that Columbia
  University Information Technology (CUIT).  Kermit is named after Kermit the
  Frog, star of the television series THE MUPPET SHOW; the name is used by
  permission of Henson Associates, Inc.

  Thanks to at least the following people for their contributions to this
  program over the years, and apologies to anyone who was inadvertantly
  omitted:

   Chris Adie, Edinburgh U, Scotland (OS/2)
   Robert Adsett, University of Waterloo, Canada
   Larry Afrin, Clemson U
   Russ Allbery, Stanford U
   Jeffrey Altman, Columbia University
   Greg Andrews, Telebit Corp
   Barry Archer, U of Missouri
   Robert Andersson, International Systems A/S, Oslo, Norway
   Chris Armstrong, Brookhaven National Lab (OS/2)
   William Bader, Software Consulting Services, Nazareth, PA
   Fuat Baran, Columbia U
   Stan Barber, Rice U
   Jim Barbour, U of Colorado
   Donn Baumgartner, Dell
   Ian Beckwith, Debian Project
   Nelson Beebe, U of Utah
   Gerry Belanger, Cognitronics
   Karl Berry, UMB
   Mark Berryman, SAIC
   Dean W Bettinger, SUNY
   Gary Bilkus
   Peter Binderup, Denmark
   David Bolen, Advanced Networks and Services, Inc.
   Joop Bonen
   Marc Boucher, U of Montreal
   Charles Brooks, EDN
   Bob Brown
   Mike Brown, Purdue U
   Rob Brown
   Jack Bryans, California State U at Long Beach
   Mark Buda, DEC (VMS)
   Fernando Cabral, Padrao iX, Brasilia
   Bjorn Carlsson, Stockholm University Computer Centre QZ, Sweden
   Bill Catchings, (formerly of) Columbia U
   Bob Cattani, Columbia U CS Dept
   Davide Cervone, Rochester U
   Seth Chaiklin, Denmark
   John Chandler, Harvard U / Smithsonian Astronomical Observatory
   Bernard Chen, UCLA
   Andrew A Chernov, RELCOM Team, Moscow
   John L Chmielewski, AT&T, Lisle, IL
   Howard Chu, U of Michigan
   Bill Coalson, McDonnell Douglas
   Bertie Coopersmith, London
   Christian Corti
   Chet Creider, U of Western Ontario
   Alan Crosswell, Columbia U
   Jeff Damens, (formerly of) Columbia U
   Mark Davies, Bath U, UK
   Sin-itirou Dezawa, Fujifilm, Japan
   Joe R. Doupnik, Utah State U
   Frank Dreano, Honeywell
   John Dunlap, U of Washington
   Alex Dupuy, SMART.COM
   David Dyck, John Fluke Mfg Co.
   Stefaan A. Eeckels, Eurokom, Luxembourg
   Nick Efthymiou
   Paul Eggert, Twin Sun, Inc., El Segundo, CA
   Bernie Eiben, DEC
   Peter Eichhorn, Assyst International
   Kristoffer Eriksson, Peridot Konsult AB, Oerebro, Sweden
   John R. Evans, IRS, Kansas City
   Glenn Everhart, RCA Labs
   Charlie Finan, Cray Research
   Herm Fischer, Encino, CA (extensive contributions to version 4.0)
   Carl Fongheiser, CWRU
   Mike Freeman, Bonneville Power Authority
   Carl Friedberg
   Marcello Frutig, Catholic University, Sao Paulo, Brazil (X.25 support)
   Hirofumi Fujii, Japan Nat'l Lab for High Energy Physics, Tokyo (Kanji)
   Chuck Fuller, Westinghouse Corporate Computer Services
   Andy Fyfe, Caltech
   Christine M. Gianone, Columbia U
   John Gilmore, UC Berkeley
   Madhusudan Giyyarpuram, HP
   Rainer Glaschick, Siemens AG, Paderborn
   William H. Glass
   German Goldszmidt, IBM
   Chuck Goodhart, NASA
   Alistair Gorman, New Zealand
   Richard Gration, ADFA, Australia
   Chris Green, Essex U, UK
   Alan Grieg, Dundee Tech, Scotland
   Yekta Gursel, MIT
   Jim Guyton, Rand Corp
   Michael Haertel
   Bruno Haible
   Bob Hain, UMN
   Marion Hakanson, ORST
   Richard Hamilton
   John Hamilston, Iowa State U
   Simon Hania, Netherlands
   Stan Hanks, Rice U.
   Ken Harrenstein, SRI
   Eugenia Harris, Data General (AOS/VS)
   David Harrison, Kingston Warren Corp
   Lucas Hart, Oregon State University
   James Harvey, Indiana/Purdue U (VMS)
   Rob Healey
   Chuck Hedrick, Rutgers U
   Ron Heiby, Technical Systems Division, Motorola Computer Group
   Steve Hemminger, Tektronix
   Christian Hemsing, RWTH Aachen, Germany (OS-9)
   Randolph Herber, US DOE,
   Andrew Herbert, Monash Univ, Australia
   Marcus Herbert, Germany
   Mike Hickey, ITI
   Dan Hildebrand, QNX Software Systems Inc, Kanata, ON (QNX)
   R E Hill
   Stephan Hoffman-Emden
   Sven Holmstrom, ABB Utilities AB, Sweden
   Bill Homer, Cray Research
   Ray Hunter, The Wollongong Group
   Randy Huntziger, National Library of Medicine
   Larry Jacobs, Transarc
   Steve Jenkins, Lancaster University, UK
   Dave Johnson, Gradient Technologies
   Mark B Johnson, Apple Computer
   Jyke Jokinen, Tampere University of Technology, Finland (QNX)
   Eric F Jones, AT&T
   Luke Jones, AT&T
   Peter Jones, U of Quebec Montreal
   Phil Julian, SAS Institute
   Peter Kabal, U of Quebec
   Mic Kaczmarczik, U of Texas at Austin
   Sergey Kartashoff, Inst. of Precise Mechanics & Computer Equipment, Moscow
   Howie Kaye, Columbia U
   Rob Kedoin, Linotype Co, Hauppauge, NY (OS/2)
   Phil Keegstra
   Mark Kennedy, IBM
   Terry Kennedy, St Peter's College, Jersey City, NJ (VMS and more)
   "Carlo Kid", Technical University of Delft, Netherlands
   Tim Kientzle
   Paul Kimoto, Cornell U
   Douglas Kingston, morgan.com
   Lawrence Kirby, Wiltshire, UK
   Tom Kloos, Sequent Computer Systems
   Guenter Knauf
   Jim Knutson, U of Texas at Austin
   John T. Kohl (BSDI)
   Scott Kramer, SRI International, Menlo Park, CA
   John Kraynack, US Postal Service
   David Kricker, Encore Computer
   Thomas Krueger, UWM
   Bo Kullmar, ABC Klubben, Stockholm, and Central Bank of Sweden, Kista
   R. Brad Kummer, AT&T Bell Labs, Atlanta, GA
   John Kunze, UC Berkeley
   David Lane, BSSI / BellSouth (Stratus VOS, X.25)
   Bob Larson, USC (OS-9)
   Bert Laverman, Groningen U, Netherlands
   Steve Layton
   David Lawyer, UC Irvine
   Jason Lehr
   David LeVine, National Semiconductor Corporation
   Daniel S. Lewart, UIUC
   S.O. Lidie, Lehigh U
   Tor Lillqvist, Helsinki U, Finland
   David-Michael Lincke, U of St Gallen, Switzerland
   Robert Lipe (for SCO makefile entries & advice)
   Dean Long
   Mike Long, Analog Devices, Norwood MA
   Kevin Lowey, U of Saskatchewan (OS/2)
   Andy Lowry, Columbia U
   James Lummel, Caprica Telecomputing Resources (QNX)
   Lewis McCarthy
   David MacKenzie, Environmental Defense Fund, U of Maryland
   John Mackin, University of Sidney, Australia
   Martin Maclaren, Bath U, UK
   Chris Maio, Columbia U CS Dept
   Montserrat Mane, HP, Grenoble, France
   Fulvio Marino, Olivetti, Ivrea, Italy
   Arthur Marsh, dircsa.org.au
   Peter Mauzey, Lucent Technologies
   Tye McQueen, Utah State U
   Ted Medin
   Hellmuth Michaelis, Hanseatischer Computerservice GmbH, Hamburg, Germany
   Leslie Mikesell, American Farm Bureau
   Todd Miller, Courtesan Consulting
   Gary Mills
   Martin Minow, DEC (VMS)
   Pawan Misra, Bellcore
   Ken Mizialko, IBM, Manassas, VA
   Wolfgang Moeller, DECUS Germany
   Ray Moody, Purdue U
   Bruce J Moore, Allen-Bradley Co, Highland Heights, OH (Atari ST)
   Steve Morley, Convex
   Peter Mossel, Columbia U
   Tony Movshon, NYU
   Lou Muccioli, Swanson Analysis Systems
   Dan Murphy
   Neal P. Murphy, Harsof Systems, Wonder Lake IL
   Gary Mussar
   John Nall, FSU
   Jack Nelson, U of Pittsburgh
   Jim Noble, Planning Research Corporation (Macintosh)
   Ian O'Brien, Bath U, UK
   Melissa O'Neill, SFU
   John Owens
   Thomas Pinkl, Health Business Systems Inc.
   Michael Pins, Iowa Computer Aided Engineering Network
   Andre' Pirard, University of Liege, Belgium
   Paul Placeway, Ohio State U
   Piet W. Plomp, ICCE, Groningen University, Netherlands
   Ken Poulton, HP Labs
   Manfred Prange, Oakland U
   Christopher Pratt, APV Baker, UK
   Frank Prindle, NADC
   Tony Querubin, U of Hawaii
   Jean-Pierre Radley
   Anton Rang
   Mike Rechtman
   Scott Ribe
   Alan Robiette, Oxford University, UK
   Michel Robitaille, U of Montreal (Mac)
   Huw Rogers, Schweizerische Kreditanstalt, Zuerich
   Nigel Roles, Cambridge, England
   Kai Uwe Rommel, Technische Universitaet Muenchen (OS/2)
   Larry Rosenman (Amiga)
   Jay Rouman, U of Michigan
   Jack Rouse, SAS Institute (Data General and/or Apollo)
   Stew Rubenstein, Harvard U (VMS)
   Gerhard Rueckle, FH Darmstadt, Fb. E/Automatisierungstechnik
   John Santos, EG&H
   Bill Schilit, Columbia U
   Ulli Schlueter, RWTH Aachen, Germany (OS-9, etc)
   Michael Schmidt, U of Paderborn, Germany
   Eric Schnoebelen, Convex
   Benn Schreiber, DEC
   Dan Schullman, DEC (modems, DIAL command, etc)
   John Schultz, 3M
   Steven Schultz, Contel (PDP-11)
   Steven Schweda
   APPP Scorer, Leeds Polytechnic, UK
   Gordon Scott, Micro Focus, Newbury UK
   Gisbert W. Selke, WIdO, Bonn, Germany
   Kijal Shah
   David Singer, IBM Almaden Research Labs
   David Sizeland, U of London Medical School
   Fridrik Skulason, Iceland
   Rick Sladkey (Linux)
   Dave Slate
   Bradley Smith, UCLA
   Fred Smith, Merk / Computrition
   Richard S Smith, Cal State
   Ryan Stanisfer, UNT
   Bertil Stenstroem, Stockholm University Computer Centre (QZ), Sweden
   James Sturdevant, CAP GEMENI AMERICA, Minneapolis
   Peter Svanberg, Royal Techn. HS, Sweden
   James R. Swenson, Accu-Weather, Inc.
   Ted T'so, MIT (Linux)
   Andy Tanenbaum, Vrije U, Amsterdam, Netherlands
   Seth Theriault, Columbia U
   Glen Thobe
   Markku Toijala, Helsinki U of Technology
   Teemu Torma, Helsinki U of Technology
   Linus Torvalds, Helsinki
   Rick Troxel, NIH
   Warren Tucker, Tridom Corp, Mountain Park, GA
   Dave Tweten, AMES-NAS
   G Uddeborg, Sweden
   Walter Underwood, Ford Aerospace
   Pieter Van Der Linden, Centre Mondial, Paris
   Ge van Geldorp, Netherlands
   Fred van Kempen, MINIX User Group, Voorhout, Netherlands
   Wayne Van Pelt, GE/CRD
   Mark Vasoll, Oklahoma State U (V7 UNIX)
   Konstantin Vinogradov, ICSTI, Moscow
   Paul Vixie, DEC
   Bernie Volz, Process Software
   Eduard Vopicka, Prague University of Economics, Czech Republic
   Martin Vorlaender
   Dimitri Vulis, CUNY
   Roger Wallace, Raytheon
   Stephen Walton, Calif State U, Northridge (Amiga)
   Jamie Watson, Adasoft, Switzerland (AIX)
   Rick Watson, U of Texas (Macintosh)
   Eric Weaver, Columbia U
   Scott Weikart (Association for Progressive Communications)
   Robert Weiner, Programming Plus, New York City
   Lauren Weinstein, Vortex Technlogy
   David Wexelblat, AT&T
   Clark Wierda, Illuminati Online
   Joachim Wiesel, U of Karlsruhe
   Lon Willett, U of Utah
   Michael Williams, UCLA
   Nate Williams, U of Montana
   David Wilson
   Joellen Windsor, U of Arizona
   Patrick Wolfe, Kuck & Associates, Inc.
   Gregg Wonderly, Oklahoma State U (V7 UNIX)
   Farrell Woods, Concurrent (formerly Masscomp)
   Dave Woolley, CAP Communication Systems, London
   Jack Woolley, SCT Corp
   Frank Wortner
   Ken Yap, formerly of U of Rochester
   John Zeeff, Ann Arbor, MI
*/

#include "ckcker.h"                     /* Kermit symbols */
#include "ckcnet.h"                     /* Network symbols */

#ifdef CK_SSL
#include "ck_ssl.h"
#endif /* CK_SSL */

#ifndef NOSPL
#include "ckuusr.h"
#endif /* NOSPL */

#ifdef OS2ONLY
#define INCL_VIO                        /* Needed for ckocon.h */
#include <os2.h>
#undef COMMENT
#endif /* OS2ONLY */

#ifdef NT
#include <windows.h>
#include <tapi.h>
#include "ckntap.h"
#endif /* NT */

#ifndef NOSERVER
/* Text message definitions.. each should be 256 chars long, or less. */
#ifdef MINIX
char *srvtxt = "\r\n\
Entering server mode.\r\n\0";
#else
#ifdef OLDMSG
/*
  It seems there was a large installation that was using C-Kermit 5A(165)
  or thereabouts, which had deployed thousands of MS-DOS Kermit scripts in
  scattered locations that looked for strings in the old server message,
  which changed in 5A(183), August 1992.
*/
char *srvtxt = "\r\n\
C-Kermit server starting.  Return to your local machine by typing\r\n\
its escape sequence for closing the connection, and issue further\r\n\
commands from there.  To shut down the C-Kermit server, issue the\r\n\
FINISH or BYE command and then reconnect.\n\
\r\n\0";
#else
#ifdef OSK
char *srvtxt = "\r\012\
Entering server mode.  If your local Kermit software is menu driven, use\r\012\
the menus to send commands to the server.  Otherwise, enter the escape\r\012\
sequence to return to your local Kermit prompt and issue commands from\r\012\
there. Use SEND and GET for file transfer. Use REMOTE HELP for a list of\r\012\
other available services.  Use BYE or FINISH to end server mode.\r\012\0";
#else /* UNIX, VMS, AOS/VS, and all others */
char *srvtxt = "\r\n\
Entering server mode.  If your local Kermit software is menu driven, use\r\n\
the menus to send commands to the server.  Otherwise, enter the escape\r\n\
sequence to return to your local Kermit prompt and issue commands from\r\n\
there.  Use SEND and GET for file transfer.  Use REMOTE HELP for a list of\r\n\
other available services.  Use BYE or FINISH to end server mode.\r\n\0";
#endif /* OSK */
#endif /* OLDMSG */
#endif /* MINIX */
#else  /* server mode disabled */
char *srvtxt = "";
#endif /* NOSERVER */

int initflg = 0;                        /* sysinit() has executed... */
int howcalled = I_AM_KERMIT;            /* How I was called */
int hmtopline = 0;
int quitting = 0;			/* I'm in the act of quitting */

#ifdef IKSDCONF
char * iksdconf = IKSDCONF;             /* IKSD configuration file */
int    iksdcf   = 0;                    /* Has IKSD c.f. been processed? */
#endif /* IKSDCONF */

int srvcdmsg = 0;                       /* [Server] CD message */
char * cdmsgfile[8] = { NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL };
char * cdmsgstr = NULL;
char * ckcdpath = NULL;

#ifdef NLCHAR                           /* Text-file line terminator */
CHAR feol = NLCHAR;
#else
CHAR feol = 0;
#endif /* NLCHAR */

int fblksiz = DBLKSIZ;          /* File blocksize */
int frecl = DLRECL;             /* File record length */
int frecfm = XYFF_S;            /* File record format (default = stream) */
int forg = XYFO_S;              /* File organization (sequential) */
int fcctrl = XYFP_N;            /* File carriage control (ctrl chars) */
int filecase = FILECASE;        /* Case matters in filenames */
int stathack = 1;               /* Fast directory lookups by default */

char uidbuf[UIDBUFLEN] = { NUL, NUL };  /* User ID buffer */
int cfilef = 0;                         /* Application ("kerbang") file flag */
char cmdfil[CKMAXPATH + 1] = { NUL, NUL }; /* Application file name */
int haveurl = 0;                        /* URL given on command line */

#ifndef NOXFER
/* Multi-protocol support */

struct ck_p ptab[NPROTOS] = {           /* Initialize the Kermit part ... */
  { "Kermit",
    DRPSIZ,                             /* Receive packet size */
    DSPSIZ,                             /* Send packet size */
    0,                                  /* Send-packet-size-set flag */
    DFWSIZ,                             /* Window size */

#ifdef NEWDEFAULTS
    PX_CAU,                             /* Control char unprefixing... */
#else
    PX_ALL,
#endif /* NEWDEFAULTS */

#ifdef VMS                              /* Default filename collision action */
    XYFX_X,                             /* REPLACE for VAX/VMS */
#else
    XYFX_B,                             /* BACKUP for everybody else */
#endif /* VMS */

#ifdef OS2                              /* Flag for file name conversion */
    XYFN_L,                             /* Literal for OS2 */
#else
    XYFN_C,                             /* Converted for others */
#endif /* OS2 */

    PATH_OFF,                   /* Send pathnames OFF */
    PATH_AUTO,                  /* Receive pathnames AUTO */
    NULL,                       /* Host receive initiation string (binary) */
    NULL,                       /* Host receive initiation string (text)   */
    NULL,                       /* Host server string */
    NULL,                       /* External protocol send command (binary) */
    NULL,                       /* External protocol send command (text)   */
    NULL,                       /* External protocol receive command (bin) */
    NULL }                      /* External protocol receive command (txt) */
#ifdef CK_XYZ
,
{"XMODEM",    128,128,-1,-1,   1,-1,-1,0,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL},
{"XMODEM-CRC",128,128,-1,-1,  -1,-1,-1,0,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL},
{"YMODEM",   -1, -1,-1,-1,    -1,-1,-1,0,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL},
{"YMODEM-g", -1, -1,-1,-1,    -1,-1,-1,0,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL},
{"ZMODEM",   -1, -1,-1,-1,PX_WIL,-1,-1,0,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL},
{"Other",    -1, -1,-1,-1,    -1,-1,-1,0,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL}
#endif /* CK_XYZ */
};

/* Declarations for Send-Init Parameters */

int spsiz = DSPSIZ,                     /* Current packet size to send */
    spmax = DSPSIZ,                     /* Biggest packet size we can send */
    lastspmax = DSPSIZ,                 /* Send-packet size last used */
    spsizr = DSPSIZ,                    /* Send-packet size requested */
    spsizf = 0,                         /* Flag to override size negotiation */
    rpsiz = DRPSIZ,                     /* Biggest we want to receive */
    urpsiz = DRPSIZ,                    /* User-requested receive pkt size */
    maxrps = MAXRP,                     /* Maximum incoming long packet size */
    maxsps = MAXSP,                     /* Maximum outbound l.p. size */
    maxtry = MAXTRY,                    /* Maximum retries per packet */
    wslots = 1,                         /* Window size currently in use */
    wslotr = DFWSIZ,                    /* Window size from SET WINDOW */
    wslotn = 1,                         /* Window size negotiated in S-pkt */
    timeouts = 0,                       /* For statistics reporting */
    spackets = 0,                       /*  ... */
    rpackets = 0,                       /*  ... */
    retrans = 0,                        /*  ... */
    crunched = 0,                       /*  ... */
    wmax = 0,                           /*  ... */
    wcur = 0,                           /*  ... */
    srvidl = 0,                         /* Server idle timeout */
    srvdis = 1,                         /* Server file xfer display */
    srvtim = DSRVTIM,                   /* Server command wait timeout */
    srvping = 1,                        /* Server keepalive */
/*
  timint is the timeout interval I use when waiting for a packet.
  pkttim is the SET RECEIVE TIMEOUT value, sent to the other Kermit.
  rtimo is the SET SEND TIMEOUT value.  rtimo is the initial value of
  timint.  timint is changed by the value in the incoming negotiation
  packet unless a SET SEND TIMEOUT command was given.
*/
    timint = DMYTIM,                    /* Timeout interval I use */
    pkttim = URTIME,                    /* Timeout I want you to use */
    rtimo = DMYTIM,                     /* Normal packet wait timeout */
    timef = 0,                          /* Flag to override what you ask */
#ifdef CK_TIMERS
    rttflg = 1,                         /* Use dynamic round-trip timers */
#else
    rttflg = 0,                         /* Use fixed timer */
#endif /* CK_TIMERS */
    mintime = 1,                        /* Minimum timeout */
    maxtime = 0,                        /* Maximum timeout */

    npad = MYPADN,                      /* How much padding to send */
    mypadn = MYPADN,                    /* How much padding to ask for */
    bctr = DFBCT,                       /* Block check type requested */
    bctu = 1,                           /* Block check type used */
    bctl = 1,                           /* Block check length */
    bctf = 0,				/* Block check type 3 forced on all */
    c_save = -1,                        /* Block check saving and restoring */
    ss_save = -1,                       /* Slow-start saving and restoring */
    ebq =  MYEBQ,                       /* 8th bit prefix */
    ebqflg = 0,                         /* 8th-bit quoting flag */
    rqf = -1,                           /* Flag used in 8bq negotiation */
    rq = 0,                             /* Received 8bq bid */
    sq = 'Y',                           /* Sent 8bq bid */
    rpt = 0,                            /* Repeat count */
    rptq = MYRPTQ,                      /* Repeat prefix */
    rptflg = 0,                         /* Repeat processing flag */
    rptena = 1,                         /* Repeat processing enabled */
    xfrcan = 1,                         /* Transfer cancellation enabled */
    xfrint = 1,                         /* Transfer interruption enabled */
    xfrchr = 3,                         /* Transfer cancel char = Ctrl-C */
    xfrnum = 3,                         /* Need three of them by default */
    g_xfrxla = -1;
    char * xfrmsg = NULL;               /* Message for f.t. display screen */
#endif /* NOXFER */

#ifdef NOCSETS
int xfrxla = 0;                         /* Character-set translation */
#else
int xfrxla = 1;                         /* enabled or disabled */
#endif /* NOCSETS */

int havelfs = 0;			/* Large file support available */

#ifndef NOXFER
int epktflg = 0;                        /* E-PACKET command active */

int capas  = 9,                         /* Position of Capabilities */
    lpcapb = 2,                         /* Long Packet capability */
    lpcapr = 1,                         /*  requested */
    lpcapu = 0,                         /*  used */
    swcapb = 4,                         /* Sliding Window capability */
    swcapr = 1,                         /*  requested (allowed) */
    swcapu = 0,                         /*  used */
    atcapb = 8,                         /* Attribute capability */
    atcapr = 1,                         /*  requested */
    atcapu = 0,                         /*  used */
    rscapb = 16,                        /* RESEND capability */
    rscapr = 1,                         /*  requested by default */
    rscapu = 0,                         /*  used */
    lscapb = 32,                        /* Locking Shift capability */
    lscapr = 1,                         /*  requested by default */
    lscapu = 0;                         /*  used */

/* Flags for whether to use particular attributes */

int atenci = 1,                         /* Encoding in */
    atenco = 1,                         /* Encoding out */
    atdati = 1,                         /* Date in */
    atdato = 1,                         /* Date out */
    atdisi = 1,                         /* Disposition in/out */
    atdiso = 1,
    atleni = 1,                         /* Length in/out (both kinds) */
    atleno = 1,
    atblki = 1,                         /* Blocksize in/out */
    atblko = 1,
    attypi = 1,                         /* File type in/out */
    attypo = 1,
    atsidi = 1,                         /* System ID in/out */
    atsido = 1,
    atsysi = 1,                        /* System-dependent parameters in/out */
    atsyso = 1;

int dispos = 0;                         /* Disposition */

#ifdef CK_PERMS
int atlpri = 1,
    atlpro = 1,
    atgpri = 1,
    atgpro = 1;
#endif /* CK_PERMS */

int atfrmi = 1,                         /* Record Format in/out */
    atfrmo = 1;

#ifdef STRATUS
int atcrei = 1,                         /* Creator ID in/out */
    atcreo = 1,
    atacti = 1,                         /* Account in/out */
    atacto = 1;
#endif /* STRATUS */

int sprmlen = -1;                       /* Send/Receive protocol parameter */
int rprmlen = -1;                       /* string length limits */
int sendipkts = 1;                      /* Send I packets */

CHAR padch = MYPADC,                    /* Padding character to send */
    mypadc = MYPADC,                    /* Padding character to ask for */
    seol = MYEOL,                       /* End-Of-Line character to send */
    eol = MYEOL,                        /* End-Of-Line character to look for */
    ctlq = CTLQ,                        /* Control prefix in incoming data */
    myctlq = CTLQ,                      /* Outbound control character prefix */
    myrptq = MYRPTQ;                    /* Repeat prefix I want to use */

int rptmin = 3;                         /* Repeat-count minimum */

int usepipes = 0,                       /* Used for xfer to/from pipes */
    g_usepipes = -1;

char * filefile = NULL;                 /* File containing list of filenames */
/* CD message filename list */

char whoareu[16] = { NUL, NUL };        /* System ID of other Kermit */
int sysindex = -1;                      /* and index to its system ID struct */
int myindex  = -1;
int wearealike = 0;                     /* 2 Kermits have compatible sysids */
char * cksysid =                        /* My system ID */
#ifdef UNIX
    "U1"
#else
#ifdef VMS
    "D7"
#else
#ifdef OSK
    "UD"
#else
#ifdef AMIGA
    "L3"
#else
#ifdef MAC
    "A3"
#else
#ifdef OS2
#ifdef NT
    "UN"
#else /* NT */
    "UO"
#endif /* NT */
#else /* OS2 */
#ifdef datageneral
    "F3"
#else
#ifdef GEMDOS
    "K2"
#else
#ifdef STRATUS
    "MV"
#else
    ""
#endif /* STRATUS */
#endif /* GEMDOS */
#endif /* datageneral */
#endif /* OS2 */
#endif /* MAC */
#endif /* AMIGA */
#endif /* OSK */
#endif /* VMS */
#endif /* UNIX */
    ;

int oopts = -1;                         /* O-Packet Options */
int omode = -1;                         /* O-Packet Transfer Mode */
int oname = -1;                         /* O-Packet Filename Options */
int opath = -1;                         /* O-Packet Pathname Options */

struct zattr iattr;                     /* Incoming file attributes */

#ifdef VMS
/* VMS labeled file default options - name only. */
int lf_opts = LBL_NAM;
#else
#ifdef OS2
/* OS/2 labeled file default options, all attributes but archived. */
unsigned long int lf_opts = LBL_EXT|LBL_HID|LBL_RO|LBL_SYS;
#else
int lf_opts = 0;
#endif /* OS2 */
#endif /* VMS */

/* Packet-related variables */

int pktnum = 0,                         /* Current packet number */
    sndtyp = 0,                         /* Type of packet just sent */
    rcvtyp = 0,                         /* Type of packet just received */
    rsn,                                /* Received packet sequence number */
    rln,                                /* Received packet length */
    size,                               /* Current size of output pkt data */
    osize,                              /* Previous output packet data size */
    maxsize,                            /* Max size for building data field */
    spktl = 0,                          /* Length packet being sent */
    rpktl = 0,                          /* Length of packet just received */
    pktpaus = 0,                        /* Interpacket pause interval, msec */
    rprintf,                            /* REMOTE PRINT flag */
    rmailf,                             /* MAIL flag */
    xferstat = -1,                      /* Status of last transaction */
    filestatus = 0;                     /* Status of last file transfer */

CHAR pktmsgbuf[PKTMSGLEN+1];
CHAR *epktmsg = pktmsgbuf;

#ifdef pdp11
int srvcmdlen = MAXRP;                  /* srvcmd buffer length */
#else
#ifdef DYNAMIC
int srvcmdlen = MAXRP;
#else
int srvcmdlen = 0;
#endif /* DYNAMIC */
#endif /* pdp11 */

CHAR
#ifdef pdp11
    srvcmdbuf[MAXRP+4],
    *srvcmd = srvcmdbuf,
#else
#ifdef DYNAMIC
    *srvcmd = (CHAR *)0,                /* Where to decode server command */
#else
    srvcmdbuf[MAXRP+4],
    *srvcmd = srvcmdbuf,
#endif /* DYNAMIC */
#endif /* pdp11 */
    padbuf[96],                         /* Buffer for send-padding */
    *recpkt,
    *rdatap,                            /* Pointer to received packet data */
    *data = (CHAR *)0,                  /* Pointer to send-packet data */
    *srvptr,                            /* Pointer to srvcmd */
    mystch = SOH,                       /* Outbound packet-start character */
    stchr = SOH;                        /* Incoming packet-start character */

/* File-related variables */

#ifndef NOMSEND                         /* Multiple SEND */
struct filelist * filehead = NULL;      /* SEND list */
struct filelist * filetail = NULL;
struct filelist * filenext = NULL;
int addlist = 0;
#endif /* NOMSEND */

char filnam[CKMAXPATH + 1];             /* Name of current file. */
char ofilnam[CKMAXPATH + 1];            /* Original name. */

int pipesend = 0;                       /* Nonzero if sending from pipe */
#ifdef PIPESEND
char * sndfilter = NULL;                /* Send and receive filters */
char * rcvfilter = NULL;
#endif /* PIPESEND */

char ** sndarray = NULL;                /* SEND /ARRAY pointer and range */
#ifndef NOSPL
int sndxlo = -1, sndxhi = -1, sndxin = -1;
#endif /* NOSPL */
#endif /* NOXFER */

#ifndef NOSERVER
int ngetpath = 0;                       /* GET search path */
int fromgetpath = 0;
char * getpath[MAXGETPATH];
char * x_user = NULL;                   /* Server login information */
char * x_passwd = NULL;
char * x_acct = NULL;
#endif /* NOSERVER */

int x_login = 0;                        /* Login required */
int x_logged = 0;                       /* User is logged in */

extern int timelimit;

#ifdef CK_LOGIN
int logintimo = 300;                    /* Login timeout */
char * userfile = NULL;                 /* Forbidden user file */
#endif /* CK_LOGIN */
#ifdef IKSD
char * anonfile = NULL;                 /* Anonymous login init file */
char * anonroot = NULL;                 /* Anonymous file-system root */
int iks_timo  = 300;                    /* 5 minutes idle timo */
int iks_retry = 3;                      /* 3 attempts at login */
#endif /* IKSD */

#ifdef CKSYSLOG
extern VOID zsyslog();
extern int ckxlogging, ckxsyslog;
#endif /* CKSYSLOG */

CK_OFF_T fsize = (CK_OFF_T)0,		/* Size of current file */
 sendstart = (CK_OFF_T)0,		/* SEND start position */
 calibrate = (CK_OFF_T)0;		/* Nonzero if calibration run */

int nzxopts = 0;                        /* Options for nzxpand() */
int nfils = 0;                          /* Number of files in file group */
int wildena = 1;			/* Wildcard expansion enabled */
#ifdef UNIX
int wildxpand = 0;                      /* Who expands wildcards, 0=Kermit.. */
#else /* UNIX */
#ifdef STRATUS
int wildxpand = 1;			/* 1=Shell. */
#endif /* STRATUS */
#endif /* UNIX */
#ifdef UNIXOROSK
int matchdot = 0;                       /* Whether to match dot files */
#else
int matchdot = 1;
#endif /* UNIXOROSK */
int matchfifo = 0;			/* Whether to match FIFO "files" */
int clfils = 0;                         /* Flag for command-line files */
int stayflg = 0;                        /* Flag for "stay", i.e. "-S" */
int xfinish = 0;                        /* Flag for FINISH = EXIT */
long ztusec = -1L;                      /* Used with ztime() */
long ztmsec = -1L;                      /* Ditto */

/* Communication device / connection variables */

char ttname[TTNAMLEN+1];                /* Name of communication device */

#ifdef MAC
int connected = 0;                      /* True if connected */
int startconnected;                     /* initial state of connected */
#endif /* MAC */

long speed = -1L;                       /* Communication device speed */
int wasclosed = 0;                      /* Connection was just closed */
int whyclosed = WC_REMO;                /* why it was closed */
int qnxportlock = 0;                    /* QNX port locking on/off */

#ifndef CLSONDISC
#define CLSONDISC 0
#endif /* CLSONDISC */

int cxflow[CXT_MAX+1];                  /* See initflow() */

#ifndef NOSHOW
char * floname[] = {                    /* Flow control names */
  "none", "xon/xoff", "rts/cts", "dtr/cd", "etx/ack", "string",
  "xxx1", "xxx2", "dtr/cts", "keep", "auto"
};
int nfloname = (sizeof(floname) / sizeof(char *));

char * cxname[] = {                     /* Connection type names */
  "remote", "direct-serial", "modem", "tcp/ip", "x.25", "decnet",
  "lat", "netbios", "named-pipe", "ssh", "pipe"
};
int ncxname = (sizeof(cxname) / sizeof(char *));
#endif /* NOSHOW */

int parity = DEFPAR,                    /* Parity specified, 0,'e','o',etc */
    hwparity = 0,                       /* Hardware parity for serial port */
    stopbits = -1,                      /* Stop bits for serial port */
    clsondisc = CLSONDISC,              /* Serial port close on disconnect */
    autopar = 0,                        /* Automatic parity change flag */
    sosi = 0,                           /* Shift-In/Out flag */
    flow = 0,                           /* Flow control (see initflow()) */
    autoflow = 1,                       /* Automatic flow control */
    turn = 0,                           /* Line turnaround handshake flag */
    turnch = XON,                       /* Line turnaround character */
    duplex = 0,                         /* Duplex, full by default */
    escape = DFESC,                     /* Escape character for connect */
    ckdelay = DDELAY,                   /* Initial delay before sending */
    tnlm = 0;                           /* Terminal newline mode */

/* Networks for SET HOST */

#ifdef BIGBUFOK
#define MYHOSTL 1024
#else
#define MYHOSTL 100
#endif /* BIGBUFOK */

char myhost[MYHOSTL];                   /* Local host name */
int network = 0;                        /* Network vs serial connection */
int inserver = 0;                       /* Running as an Internet server */
int isguest = 0;                        /* User is anonymous */
char * clienthost = NULL;               /* Peer host name or address */
int tcp_incoming = 0;                   /* Incoming TCP connection? */

#ifdef NETCONN
#ifdef TCPSOCKET
int nettype = NET_TCPB;                 /* Default network type */
#else
#ifdef SUNX25
int nettype = NET_SX25;
#else
#ifdef IBMX25
int nettype = NET_IX25;
#else
#ifdef HPX25
int nettype = NET_HX25;
#else
#ifdef STRATUSX25
int nettype = NET_VX25;
#else
#ifdef DECNET
int nettype = NET_DEC;
#else
#ifdef SUPERLAT
int nettype = NET_SLAT;
#else
int nettype = NET_NONE;
#endif /* SUPERLAT */
#endif /* DECNET */
#endif /* STRATUSX25 */
#endif /* HPX25 */
#endif /* IBMX25 */
#endif /* SUNX25 */
#endif /* TCPSOCKET */
#else  /* NETCONN */
int nettype = NET_NONE;
#endif /* NETCONN */

#ifdef ANYX25
int revcall = 0;                        /* X.25 reverse call not selected */
int closgr  = -1;                       /* X.25 closed user group  */
int cudata = 0;                         /* X.25 call user data not specified */
char udata[MAXCUDATA];                  /* X.25 call user data */

#ifdef IBMX25
/*
  I was unable to find any pre-defined MAX values for x25 addresses - the
  addresses that I've seen have been around 10-12 characters 32 is probably
  enough, 64 is hopefully safe for everyone.
*/
    x25addr_t local_nua = {'\0'};       /* local x.25 address */
    x25addr_t remote_nua = {'\0'};      /* remote x.25 address */
    char x25name[32] = {'\0'};          /* x25 device name, sx25a0 or sx25a1 */
    char x25dev[64] =  "/dev/x25pkt";   /* x25 device in /dev */
    int x25port = 0;                    /* port used for X.25 - AIX only */
#endif /* IBMX25 */

#ifndef IBMX25
/*
  This condition is unrelated to the above IBMX25 condition.
  IBM X.25 doesn't have PAD support.
*/
    CHAR padparms[MAXPADPARMS+1]; /* X.3 parameters */
#endif /* IBMX25 */
#endif /* ANYX25 */

/* Other items */

int isinterrupted = 0;                  /* Used in exception handling */
int what = W_INIT;                      /* What I am doing */
int lastxfer = 0;                       /* Last transfer (send or receive) */

extern int mdmtyp;                      /* Modem (/network) type */

#ifdef NT
extern int StartedFromDialer;
#ifdef NTSIG
extern int TlsIndex;
#endif /* NTSIG */
#ifdef NTASM
unsigned long ESPToRestore;             /* Ditto */
#endif /* NTASM */
#endif /* NT */

#ifdef OS2PM
int os2pm = 0;                          /* OS/2 Presentation Manager flag */
#endif /* OS2PM */

/* Terminal screen size, if known, -1 means unknown. */

#ifdef OS2
#include "ckocon.h"
#ifdef KUI
int tt_rows[VNUM] = {24,24,25,1};       /* Rows (height) */
int tt_cols[VNUM] = {80,80,80,80};      /* Columns (width) */
int cmd_rows = 24, cmd_cols = 80;       /* Command/console screen dimensions */
#else /* KUI */
int tt_rows[VNUM] = {-1,24,25,1};       /* Rows (height) */
int tt_cols[VNUM] = {-1,80,80,80};      /* Columns (width) */
int cmd_rows = -1, cmd_cols = -1;       /* Command/console screen dimensions */
#endif /* KUI */
int k95stdio = 0;                       /* Stdio threads */
int tt_bell = XYB_AUD | XYB_SYS;        /* BELL AUDIBLE (system sounds) */
#else /* OS2 */
int tt_rows = -1;                       /* Rows (height) */
int tt_cols = -1;                       /* Columns (width) */
int cmd_rows = 24, cmd_cols = 80;       /* Command/console screen dimensions */
int tt_bell = XYB_AUD;                  /* BELL ON */
#endif /* OS2 */

int tt_print = 0;                       /* Transparent print disabled */
int tt_escape = 1;                      /* Escaping back is enabled */
int tt_scroll = 1;                      /* Scrolling operations are enabled */

int tn_exit = 0;                        /* Exit on disconnect */

int exitonclose = 0;                    /* Exit on close */
int exithangup = 1;                     /* Hangup on exit */
int haveline = 0;                       /* SET LINE or SET HOST in effect */
int tlevel = -1;                        /* Take-file command level */
int hints = 1;                          /* Whether to give hints */

#ifdef NOLOCAL
int remonly = 1;                        /* Remote-mode-only advisory (-R) */
int nolocal = 1;                        /* Remote-only strictly enforced */
#else
int remonly = 0;
int nolocal = 0;
int cx_status = 0;                      /* CONNECT return status */
#endif /* NOLOCAL */

#ifndef NOSPL
extern int cmdlvl;                      /* Command level */
extern int maclvl;                      /* Macro invocation level */
#endif /* NOSPL */

int protocol  = PROTO_K;                /* File transfer protocol = Kermit */

#ifdef NEWDEFAULTS
int prefixing = PX_CAU;
#else
int prefixing = PX_ALL;
#endif /* NEWDEFAULTS */

extern short ctlp[];                    /* Control-prefix table */

int carrier = CAR_AUT;                  /* Pay attention to carrier signal */
int cdtimo = 0;                         /* Carrier wait timeout */
int xitsta = GOOD_EXIT;                 /* Program exit status */

#ifdef VMS                              /* Default filename collision action */
int fncact = XYFX_X;                    /* REPLACE for VMS */
#else
int fncact = XYFX_B;                    /* BACKUP for everybody else */
#endif /* VMS */

int fncsav = -1;                        /* For saving & restoring the above */
int bgset = -1;                         /* BACKGROUND mode set explicitly */

int cmdint = 1;                         /* Interrupts are allowed */
#ifdef UNIX
int xsuspend = DFSUSP;			/* Whether SUSPEND command, etc, */
#else                                   /* is to be allowed. */
int xsuspend = 0;
#endif /* UNIX */

/* Statistics variables */

CK_OFF_T
    flci,                       /* Characters from line, current file */
    flco,                       /* Chars to line, current file  */
    tlci,                       /* Chars from line in transaction */
    tlco,                       /* Chars to line in transaction */
    ffc,                        /* Chars to/from current file */
    tfc;                        /* Chars to/from files in transaction */

long filcnt,                    /* Number of files in transaction */
    filrej,                     /* Number of files rejected in transaction */
    cps = 0L,                   /* Chars/sec last transfer */
    peakcps = 0L,               /* Peak chars/sec last transfer */
    ccu,                        /* Control chars unprefixed in transaction */
    ccp,                        /* Control chars prefixed in transaction */
    rptn;                       /* Repeated characters compressed */

int tsecs = 0;                          /* Seconds for transaction */
int fsecs = 0;                          /* Per-file timer */

#ifdef GFTIMER
CKFLOAT
  fpfsecs = 0.0,                        /* Floating point per-file timer */
  fptsecs = 0.0;                        /* and per-transaction timer */
#endif /* GFTIMER */

/* Flags */

int deblog = 0,                         /* Debug log is open */
    debok = 1,                          /* Debug log is not disabled */
    debxlen = 54,                       /* Default length for debug strings */
    debses = 0,                         /* Flag for DEBUG SESSION */
    debtim = 0,                         /* Include timestamp in debug log */
    debmsg = 0,                         /* Debug messages on/off */
    pktlog = 0,                         /* Flag for packet logging */
    seslog = 0,                         /* Session logging */
    dialog = 0,                         /* DIAL logging */
    tralog = 0,                         /* Transaction logging */
    tlogfmt = 1,                        /* Transaction log format (verbose) */
    tlogsep = (int)',',                 /* Transaction log field separator */
    displa = 0,                         /* File transfer display on/off */
    stdouf = 0,                         /* Flag for output to stdout */
    stdinf = 0,                         /* Flag for input from stdin */
    xflg   = 0,                         /* Flag for X instead of F packet */
    hcflg  = 0,                         /* Doing Host command */
    dest   = DEST_D,                    /* Destination for packet data */
    zchkod = 0,                         /* zchko() should work for dirs too? */
    zchkid = 0,                         /* zchki() should work for dirs too? */

/* If you change this, also see struct ptab above... */

#ifdef OS2                              /* Flag for file name conversion */
    fncnv  = XYFN_L,                    /* Default is Literal in OS/2, */
    f_save = XYFN_L,                    /* (saved copy of same) */
#else
    fncnv  = XYFN_C,                    /* elsewhere Convert them */
    f_save = XYFN_C,                    /* (ditto) */
#endif /* OS2 */

    fnspath = PATH_OFF,                 /* Send file path */
    fnrpath = PATH_AUTO,                /* Receive file path */
    fackpath = 1,                       /* Send back path in ACK to F */
    binary = XYFT_B,                    /* Default file transfer mode */
    b_save = XYFT_B,                    /* Saved file mode */
    eofmethod = 0,                      /* EOF detection method (length) */

#ifdef OS2
    cursor_save = -1,                   /* Cursor state */
#endif /* OS2 */

    xfermode = XMODE_A,                 /* Transfer mode, manual or auto */
    xfiletype = -1,                     /* Transfer only text (or binary) */
    recursive = 0,                      /* Recursive directory traversal */
    nolinks   = 2,                      /* Don't follow symbolic links */
    skipbup   = 0,                      /* Skip backup files when sending */
    sendmode = SM_SEND,                 /* Which type of SEND operation */
    slostart  = 1,                      /* Slow start (grow packet lengths) */
    cmask  = 0377,                      /* CONNECT (terminal) byte mask */
    fmask  = 0377,                      /* File byte mask */
    ckwarn = 0,                         /* Flag for file warning */
    quiet  = 0,                         /* Be quiet during file transfer */
    local  = 0,                         /* 1 = local mode, 0 = remote mode */
    cxtype = CXT_REMOTE,                /* Connection type */
    server = 0,                         /* Flag for I Am Server */
    query = 0,                          /* Flag for Query active */
    justone = 0,                        /* Server should do Just One command */
    urserver = 0,                       /* Flag for You Are Server */
    bye_active = 0,                     /* Flag for BYE command active */
    diractive = 0,                      /* Flag for DIRECTORY command active */
    cdactive = 0,                       /* Flag for CD command active */
    cflg   = 0,                         /* Connect before transaction */
    cnflg  = 0,                         /* Connect after transaction */
    cxseen = 0,                         /* Flag for cancelling a file */
    czseen = 0,                         /* Flag for cancelling file group */
    fatalio = 0,                        /* Flag for fatal i/o error */
    discard = 0,                        /* Flag for file to be discarded */
    keep = SET_AUTO,                    /* Keep incomplete files = AUTO */
    unkcs = 1,                          /* Keep file w/unknown character set */
#ifdef VMS
    filepeek = 0,                       /* Inspection of files */
#else
#ifdef datgeneral
    filepeek = 0,
#else
    filepeek = 1,
#endif /* datageneral */
#endif /* VMS */
    nakstate = 0,                       /* In a state where we can send NAKs */
    dblchar = -1,                       /* Character to double when sending */
    moving = 0,                         /* MOVE = send, then delete */
    reliable = SET_AUTO,                /* Nonzero if transport is reliable */
    xreliable = -1,
    setreliable = 0,
    urclear = 0,                        /* Nonzero for clear channel to you */
    clearrq = SET_AUTO,                 /* SET CLEARCHANEL value */
    cleared = 0,
    streaming = 0,                      /* Nonzero if streaming is active */
    streamok = 0,                       /* Nonzero if streaming negotiated */
    streamrq = SET_AUTO,                /* SET STREAMING value */
    streamed = -1;                      /* Whether we streamed last time */

char * snd_move = NULL;                 /* Move file after sending it */
char * snd_rename = NULL;               /* Rename file after sending it */
char * rcv_move = NULL;                 /* Move file after receiving it */
char * rcv_rename = NULL;               /* Rename file after receiving it */

char * g_snd_move = NULL;
char * g_snd_rename = NULL;
char * g_rcv_move = NULL;
char * g_rcv_rename = NULL;

#ifdef CK_TRIGGER
char *tt_trigger[TRIGGERS] = { NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL };
CHAR *tt_trmatch[TRIGGERS] = { NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL };
char *triggerval = NULL;
#endif /* CK_TRIGGER */

int ckxlogging = 0;                     /* Flag for syslogging active */
int ikdbopen = 0;                       /* Flag for IKSD database active */
int dbinited = 0;                       /* Flag for IKSDB record init'd */
#ifndef CKSYSLOG
int ckxsyslog = 0;                      /* Logging level 0 */
#else
#ifdef SYSLOGLEVEL
int ckxsyslog = SYSLOGLEVEL;            /* Logging level specified */
#else
int ckxsyslog = SYSLG_DF;               /* Default logging level */
#endif /* SYSLOGLEVEL */
#endif /* CKSYSLOG */

#ifndef NOHELP
#ifndef NOCMDL
_PROTOTYP( VOID iniopthlp, (void) );    /* Command-line help initializer */
#endif /* NOCMDL */
#endif /* NOHELP */

_PROTOTYP( VOID getexedir, (void) );
_PROTOTYP( int putnothing, (char) );

#ifdef IKSD
_PROTOTYP( VOID doiksdinit, (void) );
_PROTOTYP( VOID iksdinit, (void) );
_PROTOTYP( VOID doiklog, (void) );
_PROTOTYP( int dbinit, (void) );
#endif /* IKSD */

/* Variables passed from command parser to protocol module */

#ifndef NOSPL
#ifndef NOICP
#ifdef CK_APC
_PROTOTYP( VOID apconect, (void) );
#endif /* CK_APC */
#ifdef OS2
extern int initvik;
#endif /* OS2 */
#endif /* NOICP */
#endif /* NOSPL */
char *clcmds = NULL;                    /* Pointer to command-line commands */

#ifndef NOSETKEY
extern KEY *keymap;
extern MACRO *macrotab;
#endif /* NOSETKEY */

#ifndef NOPUSH
int nopush = 0;                         /* PUSH enabled */
#else
int nopush = 1;                         /* PUSH disabled */
#endif /* NOPUSH */

CHAR sstate  = (CHAR) 0;                /* Starting state for automaton */
CHAR zstate  = (CHAR) 0;                /* For remembering sstate */
char * printername = NULL;              /* NULL if printer not redirected */
int printpipe = 0;                      /* For SET PRINTER */
int noprinter = 0;

#ifndef NOXFER
char *cmarg  = "";                      /* Pointer to command data */
char *cmarg2 = "";                      /* Pointer to 2nd command data */
char **cmlist;                          /* Pointer to file list in argv */

#ifdef CK_AUTODL                        /* Autodownload */
int autodl = 1;                         /* Enabled by default */
#else
int autodl = 0;                         /* (or if not implemented). */
#endif /* CK_AUTODL */
int adl_err = 1;                        /* 1 = stop on error */
#ifdef KUI
int adl_ask = 1;			/* 1 = file dialog on autodownload */
#else
int adl_ask = 0;			/* 0 = no file dialog */
#endif /* KUI */
#ifdef OS2                              /* AUTODOWNLOAD parameters */
int adl_kmode = ADL_PACK,               /* Match Packet to signal download */
    adl_zmode = ADL_PACK;
char * adl_kstr = NULL;                 /* KERMIT Download String */
char * adl_zstr = NULL;                 /* ZMODEM Download String */
#endif /* OS2 */

int remfile = 0, rempipe = 0, remappd = 0; /* REMOTE output redirection */
char * remdest = NULL;

#ifndef NOSERVER
/*
  Server services:
   0 = disabled
   1 = enabled in local mode
   2 = enabled in remote mode
   3 = enabled in both local and remote modes
  only as initial (default) values.
*/
int en_xit = 2;                         /* EXIT */
int en_cwd = 3;                         /* CD/CWD */
int en_cpy = 3;                         /* COPY   */
int en_del = 2;                         /* DELETE */
int en_mkd = 3;                         /* MKDIR */
int en_rmd = 2;                         /* RMDIR */
int en_dir = 3;                         /* DIRECTORY */
int en_fin = 3;                         /* FINISH */
int en_get = 3;                         /* GET */
#ifndef NOPUSH
int en_hos = 2;                         /* HOST enabled */
#else
int en_hos = 0;                         /* HOST disabled */
#endif /* NOPUSH */
int en_ren = 3;                         /* RENAME */
int en_sen = 3;                         /* SEND */
int en_set = 3;                         /* SET */
int en_spa = 3;                         /* SPACE */
int en_typ = 3;                         /* TYPE */
int en_who = 3;                         /* WHO */
#ifdef datageneral
/* Data General AOS/VS can't do this */
int en_bye = 0;                         /* BYE */
#else
int en_bye = 2;                         /* PCs in local mode... */
#endif /* datageneral */
int en_asg = 3;                         /* ASSIGN */
int en_que = 3;                         /* QUERY */
int en_ret = 2;                         /* RETRIEVE */
int en_mai = 3;                         /* MAIL */
int en_pri = 3;                         /* PRINT */
int en_ena = 3;                         /* ENABLE */
#else
int en_xit = 0, en_cwd = 0, en_cpy = 0, en_del = 0, en_mkd = 0, en_rmd = 0,
    en_dir = 0, en_fin = 0, en_get = 0, en_hos = 0, en_ren = 0, en_sen = 0,
    en_set = 0, en_spa = 0, en_typ = 0, en_who = 0, en_bye = 0, en_asg = 0,
    en_que = 0, en_ret = 0, en_mai = 0, en_pri = 0, en_ena = 0;
#endif /* NOSERVER */
#endif /* NOXFER */

/* Miscellaneous */

char **xargv;                           /* Global copies of argv */
int xargc;                              /* and argc  */
int xargs;                              /* an immutable copy of argc */
char *xarg0;                            /* and of argv[0] */
char *pipedata;                         /* Pointer to -P (pipe) data */

extern char *dftty;                     /* Default tty name from ck?tio.c */
extern int dfloc;                       /* Default location: remote/local */
extern int dfprty;                      /* Default parity */
extern int dfflow;                      /* Default flow control */

#ifdef TNCODE
extern int tn_deb;
#endif /* TNCODE */
/*
  Buffered file input and output buffers.  See getpkt() in ckcfns.c
  and zoutdump() in the system-dependent file i/o module (usually ck?fio.c).
*/
#ifndef DYNAMIC
/* Now we allocate them dynamically, see getiobs() below. */
char zinbuffer[INBUFSIZE], zoutbuffer[OBUFSIZE];
#endif /* DYNAMIC */
char *zinptr, *zoutptr;
int zincnt, zoutcnt;
int zobufsize = OBUFSIZE;
int zofbuffer = 1;
int zofblock  = 1;

#ifdef SESLIMIT
int seslimit = 0;
#endif /* SESLIMIT */

#ifdef CK_AUTHENTICATION
#include "ckuath.h"
#endif /* CK_AUTHENTICATION */

_PROTOTYP( int getiobs, (VOID) );

/*  M A I N  --  C-Kermit main program  */

#include <signal.h>

#ifndef NOCCTRAP
#include <setjmp.h>
#include "ckcsig.h"
ckjmpbuf cmjbuf;
#ifdef GEMDOS                           /* Special for Atari ST */
cc_clean();                             /* This can't be right? */
#endif /* GEMDOS */
#endif /* NOCCTRAP */

#ifndef NOXFER
/* Info associated with a system ID */

struct sysdata sysidlist[] = {          /* Add others as needed... */
  { "0",  "anonymous",    0, NUL,  0, 0, 0 },
  { "A1", "Apple II",     0, NUL,  0, 0, 3 }, /* fix this */
  { "A3", "Macintosh",    1, ':',  0, 2, 1 },
  { "D7", "VMS",          0, ']',  1, 0, 0 },
  { "DA", "RSTS/E",       0, ']',  1, 0, 3 }, /* (i think...) */
  { "DB", "RT11",         0, NUL,  1, 0, 3 }, /* (maybe...) */
  { "F3", "AOS/VS",       1, ':',  0, 0, 2 },
  { "I1", "VM/CMS",       0, NUL,  0, 0, 0 },
  { "I2", "MVS/TSO",      0, NUL,  0, 0, 0 },
  { "I4", "MUSIC",        0, NUL,  0, 0, 0 },
  { "I7", "CICS",         0, NUL,  0, 0, 0 },
  { "I9", "MVS/ROSCOE",   0, NUL,  0, 0, 0 },
  { "K2", "Atari ST",     1, '\\', 1, 0, 3 },
  { "L3", "Amiga",        1, '/',  1, 0, 2 },
  { "MV", "Stratus VOS",  1, '>',  0, 1, 0 },
  { "N3", "Apollo Aegis", 1, '/',  0, 3, 2 },
  { "U1", "UNIX",         1, '/',  0, 3, 2 },
  { "U8", "MS-DOS",       1, '\\', 1, 0, 3 },
  { "UD", "OS-9",         1, '/',  0, 3, 2 },
  { "UN", "Windows-32",   1, '\\', 1, 2, 3 },
  { "UO", "OS/2",         1, '\\', 1, 2, 3 }
};
static int nxxsysids = (sizeof(sysidlist) / sizeof(struct sysdata));

/* Given a Kermit system ID code, return the associated name string */
/* and some properties of the filenames... */

char *
getsysid(s) char * s; {                 /* Get system-type name */
    int i;
    if (!s) return("");
    for (i = 0; i < nxxsysids; i++)
      if (!strcmp(sysidlist[i].sid_code,s))
        return(sysidlist[i].sid_name);
    return(s);
}

int
getsysix(s) char *s; {                  /* Get system-type index */
    int i;
    if (!s) return(-1);
    for (i = 0; i < nxxsysids; i++)
      if (!strcmp(sysidlist[i].sid_code,s))
        return(i);
    return(-1);
}
#endif /* NOXFER */

/* Tell if a pathname is absolute (versus relative) */
/* This should be parceled out to each of the ck*fio.c modules... */
/* VMS isabsolute() is now in ckvfio.c. */
#ifndef VMS
int
isabsolute(path) char * path; {
    int rc = 0;
    int x;
    if (!path)
      return(0);
    if (!*path)
      return(0);
    x = (int) strlen(path);
    debug(F111,"isabsolute",path,x);
#ifdef UNIX
    if (*path == '/'
#ifdef DTILDE
        || *path == '~'
#endif /* DTILDE */
        )
      rc = 1;
#else /* def UNIX */
#ifdef OS2
    if (*path == '/' || *path == '\\')
      rc = 1;
    else if (isalpha(*path) && x > 2)
      if (*(path+1) == ':' && (*(path +2) == '/' || *(path+2) == '\\'))
        rc = 1;
#else /* def OS2 */
#ifdef AMIGA
    if (*path == '/'
#ifdef DTILDE
        || *path == '~'
#endif /* DTILDE */
        )
      rc = 1;
#else /* def AMIGA */
#ifdef OSK
    if (*path == '/'
#ifdef DTILDE
        || *path == '~'
#endif /* DTILDE */
        )
      rc = 1;
#else /* def OSK */
#ifdef datageneral
    if (*path == ':')
      rc = 1;
#else /* def datageneral */
#ifdef MAC
    rc = 0;                             /* Fill in later... */
#else /* def MAC */
#ifdef STRATUS
    rc = 0;                             /* Fill in later... */
#else /* def STRATUS */
#ifdef GEMDOS
    if (*path == '/' || *path == '\\')
      rc = 1;
    else if (isalpha(*path) && x > 1)
      if (*(path+1) == ':')
        rc = 1;
#endif /* GEMDOS */
#endif /* STRATUS */
#endif /* MAC */
#endif /* datageneral */
#endif /* OSK */
#endif /* AMIGA */
#endif /* OS2 */
#endif /* UNIX */
    debug(F101,"isabsolute rc","",rc);
    return(rc);
}
#endif /* ndef VMS */

/*  See if I have direct access to the keyboard  */

int
is_a_tty(n) int n; {
#ifdef UNIX
    extern int ttfdflg;
    if (ttfdflg > 0)
      return(1);
#endif /* UNIX */
#ifdef KUI
    return 1;
#else /* KUI */
#ifdef NT
    if (isWin95())
      return(1);
    else
      return(_isatty(n));
#else
#ifdef IKSD
   if (inserver)
     return(1);
   else
#endif /* IKSD */
     return(isatty(n));
#endif /* NT */
#endif /* KUI */
}

#ifndef NOXFER
VOID
initxlist() {
    extern char * sndexcept[], * rcvexcept[];
    int i;
    for (i = 0; i < NSNDEXCEPT; i++) {
	sndexcept[i] = NULL;
	rcvexcept[i] = NULL;
    }
}
#endif /* NOXFER */

/* Initialize flow control table */

VOID
initflow() {                            /* Default values for flow control */
#ifdef VMS                              /* for each kind of connection. */
    /* The VMS telnet terminal driver treats "none" as request to lose chars */
    cxflow[CXT_REMOTE]  = FLO_XONX;     /* Remote mode... */
#else
#ifdef HPUX
    /* Ditto for HP-UX */
    cxflow[CXT_REMOTE]  = FLO_XONX;     /* Remote mode... */
#else
    /* The temptation is to make this one FLO_KEEP but don't!!! */
    /* It totally wrecks binary-file transfer when coming in via Telnet. */
    /* In UNIX at least... */
    cxflow[CXT_REMOTE]  = FLO_NONE;
#endif /* HPUX */
#endif /* VMS */

#ifdef VMS
    cxflow[CXT_DIRECT]  = FLO_XONX;     /* Direct serial connections... */
#else
    cxflow[CXT_DIRECT]  = FLO_NONE;
#endif /* VMS */

#ifdef CK_RTSCTS
    cxflow[CXT_MODEM]   = FLO_RTSC;     /* Modem connections... */
#else
#ifdef VMS
    cxflow[CXT_MODEM]   = FLO_XONX;
#else
    cxflow[CXT_MODEM]   = FLO_NONE;
#endif /* VMS */
#endif /* CK_RTSCTS */

#ifdef VMS
    cxflow[CXT_TCPIP]   = FLO_XONX;     /* TCP/IP connections... */
#else
    cxflow[CXT_TCPIP]   = FLO_NONE;
#endif /* VMS */

    cxflow[CXT_SSH]     = FLO_NONE;
    cxflow[CXT_X25]     = FLO_NONE;     /* Other kinds of networks... */
    cxflow[CXT_DECNET]  = FLO_XONX;
    cxflow[CXT_LAT]     = FLO_XONX;
    cxflow[CXT_NETBIOS] = FLO_NONE;
    cxflow[CXT_NPIPE]   = FLO_NONE;
    cxflow[CXT_PIPE]    = FLO_NONE;
    flow = cxflow[cxtype];              /* Initial flow setting. */
    debug(F101,"initflow","",flow);
}

#ifndef NOXFER
/* Initialize file transfer protocols */

VOID
initproto(y, upbstr, uptstr, srvstr, sndbstr, sndtstr, rcvbstr, rcvtstr)
    int y;
    char * upbstr, * uptstr, * srvstr, * sndbstr, * sndtstr, * rcvbstr,
    * rcvtstr;
/* initproto */ {

    if (upbstr)                         /* Convert null strings */
      if (!*upbstr)                     /* to null pointers */
        upbstr = NULL;

    if (uptstr)                         /* Convert null strings */
      if (!*uptstr)                     /* to null pointers */
        uptstr = NULL;

    if (sndbstr)
      if (!*sndbstr)
        sndbstr = NULL;

    if (sndtstr)
      if (!*sndtstr)
        sndtstr = NULL;

    if (rcvbstr)
      if (!*rcvbstr)
        rcvbstr = NULL;

    if (rcvtstr)
      if (!*rcvtstr)
        rcvtstr = NULL;

    if (srvstr)
      if (!*srvstr)
        srvstr = NULL;

    protocol = y;                       /* Set protocol */

    if (ptab[protocol].rpktlen > -1)
      urpsiz = ptab[protocol].rpktlen;
    if (ptab[protocol].spktflg > -1)
      spsizf = ptab[protocol].spktflg;
    if (ptab[protocol].spktlen > -1) {
        spsiz = ptab[protocol].spktlen;
        debug(F101,"initproto spsiz","",spsiz);
        if (spsizf) {
            spsizr = spmax = spsiz;
            debug(F101,"initproto spsizr","",spsizr);
        }
    }
    if (ptab[protocol].winsize > -1)
      wslotr = ptab[protocol].winsize;
    if (ptab[protocol].prefix > -1)
      prefixing = ptab[protocol].prefix;
    if (ptab[protocol].fnca > -1)
      fncact  = ptab[protocol].fnca;
    if (ptab[protocol].fncn > -1)
      fncnv   = ptab[protocol].fncn;
    if (ptab[protocol].fnsp > -1)
      fnspath = ptab[protocol].fnsp;
    if (ptab[protocol].fnrp > -1)
      fnrpath = ptab[protocol].fnrp;

    makestr(&(ptab[protocol].h_b_init),upbstr);
    makestr(&(ptab[protocol].h_t_init),uptstr);
    makestr(&(ptab[protocol].h_x_init),srvstr);
    makestr(&(ptab[protocol].p_b_scmd),sndbstr);
    makestr(&(ptab[protocol].p_t_scmd),sndtstr);
    makestr(&(ptab[protocol].p_b_rcmd),rcvbstr);
    makestr(&(ptab[protocol].p_t_rcmd),rcvtstr);
}
#endif /* NOXFER */

#ifndef NOCMDL
VOID
#ifdef CK_ANSIC
docmdline(void * threadinfo)
#else /* CK_ANSIC */
docmdline(threadinfo) VOID * threadinfo;
#endif /* CK_ANSIC */
{
#ifdef NTSIG
    setint();
    if (threadinfo) {                   /* Thread local storage... */
       TlsSetValue(TlsIndex,threadinfo);
       debug( F100, "docmdline called with threadinfo block", "", 0 );
    } else
      debug( F100, "docmdline threadinfo is NULL","",0);
#endif /* NTSIG */
#ifdef CK_LOGIN
#ifdef NT
#ifdef IKSD
    if (inserver)
      setntcreds();
#endif /* IKSD */
#endif /* NT */
#endif /* CK_LOGIN */
    proto();                            /* Take any requested action, then */
    if (!quiet)                         /* put cursor back at left margin, */
      conoll("");
#ifndef NOLOCAL
    if (cnflg) {                        /* Re-connect if requested */
        cnflg = 0;
        doconect(0,0);
        if (ttchk() < 0)
          dologend();
    }
#endif /* NOLOCAL */

#ifdef NTSIG
     ckThreadEnd(threadinfo);
#endif /* NTSIG */
   return;
}

void
ikslogin() {
    if (sstelnet
#ifdef IKSD
        || inserver                     /* Internet server */
#endif /* IKSD */
        ) {
        char *s;
        extern int fdispla;             /* File-transfer display format */
        extern char * ikprompt;         /* IKSD prompt */

#ifdef IKSD
#ifdef CK_LOGIN
        if (inserver) {
            x_login = 1;                /* Login required */
            x_logged = 0;               /* Not logged in yet */
            cmsetp(ikprompt);           /* Set up IKSD's prompt */
#ifndef NOSERVER
            en_mai = 0;                 /* MAIL is disabled */
            en_who = 0;                 /* REMOTE WHO is disabled */
            en_hos = 0;                 /* REMOTE HOST is disabled */
            en_pri = 0;                 /* PRINT is disabled */
#endif /* NOSERVER */
        } else {
            x_login = 0;                /* Login not required */
            x_logged = 1;               /* Already logged in */
        }
#endif /* CK_LOGIN */
#endif /* IKSD */
        nolocal = 1;                    /* SET LINE/HOST not allowed */
        fdispla = XYFD_N;               /* No file-transfer display */
#ifdef NETCONN
        clienthost = ckgetpeer();       /* Get client's hostname */
        debug(F110,"ikslogin clienthost",clienthost,0);
#endif /* NETCONN */
        ztime(&s);                      /* Get current date and time */

#ifdef CK_LOGIN
#ifdef CK_AUTHENTICATION
        if (x_login) {
            x_logged = ck_tn_auth_valid(); /* Did Telnet Auth succeed? */
            debug(F111,"ikslogin","x_logged",x_logged);

#ifdef NT
            /* On Windows 9x, we do not have the ability in  */
            /* zvuser() at present to determine if the name  */
            /* approved in a Kerberos principal is really a  */
            /* an account in the Windows Access Control List */
            if (isWin95() && x_logged == AUTH_VALID
                 && (ck_tn_authenticated() != AUTHTYPE_NTLM)
#ifdef CK_SRP
                 && (ck_tn_authenticated() != AUTHTYPE_SRP)
#endif /* CK_SRP */
                 ) {
                auth_finished(AUTH_USER);
                x_logged = AUTH_USER;
                printf("WARNING:\r\n");
                printf(
" The Telnet authentication method used cannot provide for automated\r\n");
                printf(
" login to Windows 95 or Windows 98.  A password must be entered\r\n");
                printf(
" locally to validate your userid.  Telnet authentication (and encryption)\r\n"
                );
                printf(
" can be used to validate the host (and protect the privacy of your password.)\
\r\n"
                );
            }
#endif /* NT */

            if (x_logged == AUTH_VALID) {
#ifdef CK_SSL
                if ((ssl_active_flag || tls_active_flag) &&
                    (!TELOPT_U(TELOPT_AUTHENTICATION) ||
                     ck_tn_authenticated() == AUTHTYPE_NULL ||
                     ck_tn_authenticated() == AUTHTYPE_AUTO)
                    ) {
#ifdef SSL_KRB5
                    if (tls_is_krb5(0)) {
                        printf("Authenticated using Kerberos 5\r\n");
#ifdef CKSYSLOG
                        if (ckxsyslog >= SYSLG_LI && ckxlogging) {
                            extern char szUserNameAuthenticated[];
                            cksyslog(SYSLG_LI, 1, "AUTH_VALID",
                                     "Kerberos 5",
                                     szUserNameAuthenticated
                                     );
                        }
#endif /* CKSYSLOG */
                    } else
#endif /* SSL_KRB5 */
                    {
                        printf("Authenticated using X.509 certificate\r\n");
#ifdef CKSYSLOG
                        if (ckxsyslog >= SYSLG_LI && ckxlogging) {
                            extern char szUserNameAuthenticated[];
                            cksyslog(SYSLG_LI, 1, "AUTH_VALID",
                                     "X.509 certificate",
                                     szUserNameAuthenticated
                                     );
                        }
#endif /* CKSYSLOG */
                    }
                } else
#endif /* CK_SSL */
                  {
                      printf("Authenticated using %s\r\n",
                             AUTHTYPE_NAME(ck_tn_authenticated()));
#ifdef CKSYSLOG
                      if (ckxsyslog >= SYSLG_LI && ckxlogging) {
                          extern char szUserNameAuthenticated[];
                          cksyslog(SYSLG_LI, 1, "AUTH_VALID",
                                   AUTHTYPE_NAME(ck_tn_authenticated()),
                                   szUserNameAuthenticated
                                   );
                      }
#endif /* CKSYSLOG */
                  }
                zvuser(uidbuf);
                if (zvpass("") == 0)
                  x_logged = 0;
            } else if (x_logged == AUTH_USER && !strcmp(uidbuf,"anonymous")) {
                extern char szUserNameAuthenticated[];
                zvuser(uidbuf);
                debug(F110,"szUserNameAuthenticated",
                      szUserNameAuthenticated,0);
                if (zvpass(szUserNameAuthenticated) == 0) {
                  /* Anonymous login failed.  Force a username prompt. */
                  x_logged = 0;
                  uidbuf[0] = '\0';
                } else {
#ifdef CK_SSL
                    if ((ssl_active_flag || tls_active_flag) &&
                        (!TELOPT_U(TELOPT_AUTHENTICATION) ||
                         ck_tn_authenticated() == AUTHTYPE_NULL ||
                         ck_tn_authenticated() == AUTHTYPE_AUTO)) {
                        printf("Authenticated using X.509 certificate\r\n");
#ifdef CKSYSLOG
                        if (ckxsyslog >= SYSLG_LI && ckxlogging) {
                            extern char szUserNameAuthenticated[];
                            cksyslog(SYSLG_LI, 1, "AUTH_USER",
                                     "X.509 certificate",
                                     szUserNameAuthenticated
                                     );
                        }
#endif /* CKSYSLOG */
                    } else
#endif /* CK_SSL */
                      {
                          printf("Authenticated using %s\r\n",
                                 AUTHTYPE_NAME(ck_tn_authenticated())
                                 );
#ifdef CKSYSLOG
                          if (ckxsyslog >= SYSLG_LI && ckxlogging) {
                              cksyslog(SYSLG_LI, 1, "AUTH_USER",
                                       AUTHTYPE_NAME(ck_tn_authenticated()),
                                       szUserNameAuthenticated
                                       );
                          }
#endif /* CKSYSLOG */
                      }
                }
            } else {
#ifdef CKSYSLOG
                if (ckxsyslog >= SYSLG_LI && ckxlogging &&
                    x_logged == AUTH_USER) {
                    extern char szUserNameAuthenticated[];
                    cksyslog(SYSLG_LI, 1, "AUTH_USER",
                             AUTHTYPE_NAME(ck_tn_authenticated()),
                             szUserNameAuthenticated
                             );
                }
#endif /* CKSYSLOG */
                x_logged = 0;
                if (!strcmp("(unknown)",uidbuf)
#ifdef NT
                    || !stricmp("administrator",uidbuf)
#ifdef UNIX
                    || !strcmp("root",uidbuf)
#else
#ifdef Plan9
                    || !strcmp("root",uidbuf)
#else
#ifdef OSK
                    || !strcmp("root",uidbuf)
#endif /* OSK */
#endif /* Plan9 */
#endif /* UNIX */
#endif /* NT */
                    )
                  uidbuf[0] = '\0';
            }
        }
#endif /* CK_AUTHENTICATION */
#endif /* CK_LOGIN */

#ifdef IKSD
        if (inserver)
          printf("\r\nInternet Kermit Service ready at %s%s\r\n",s,versio);
        else
#endif /* IKSD */
          printf("\r\nC-Kermit ready at %s%s\r\n",s,versio);
        if (*myhost)
          printf("%s\r\n", myhost);
        printf("\r\n");
    }
#ifdef CK_LOGIN
#ifdef IKSD
    if (inserver) {
        int i;
        extern int arg_x;               /* Flag for '-x' on command line */
#ifndef NOSPL
        extern struct mtab *mactab;         /* For ON_LOGIN macro. */
        extern int nmac;
#endif /* NOSPL */

        debug(F110,"MAIN clienthost",clienthost,0);
        srvidl = timelimit = logintimo; /* For interactive login */
        rtimer();                       /* Reset timer */
        for (i = 0; i < iks_retry && !x_logged; i++) { /* Count retries */
            if (gtimer() > logintimo)
              break;
#ifdef TNCODE
            tn_wait("login loop");
            tn_push();
#endif /* TNCODE */
            debug(F101,"MAIN LOGIN try","",i);
            what = W_NOTHING;           /* Because proto() changes this */

#ifdef IKS_OPTION
            debug(F111,"MAIN LOGIN",
                  "TELOPT_SB(TELOPT_KERMIT).kermit.me_start",
                  TELOPT_SB(TELOPT_KERMIT).kermit.me_start
                  );
            /* Kermit server negotiated */
            if (TELOPT_SB(TELOPT_KERMIT).kermit.me_start) {
                debug(F101,"IKSD starting in server mode","",0);
                arg_x = 1;              /* Enter server mode */
                sstate = 'x';
#ifdef IKSDPOPBACK
                justone = 1;            /* Execute one command at a time. */
#endif /* IKSDPOPBACK */
                proto();                /* Enter protocol if requested. */
#ifdef NTSIG
                ck_ih();
#endif /* NTSIG */
                if (x_logged)           /* Logged in */
                  break;
            } else {                    /* Not in client/server mode */
#endif /* IKS_OPTION */
                debug(F101,"IKSD starting with Username prompt","",0);
                x_logged = ckxlogin((CHAR *)uidbuf,NULL,NULL,1);
                if (sstate) {           /* Received a packet at prompt */
#ifdef IKSDPOPBACK
                    justone = 1;        /* Go handle it */
#endif /* IKSDPOPBACK */
                    proto();
                }
                if (!x_logged) {        /* In case we are at the prompt... */
                    printf("Access denied.\n");
                    uidbuf[0] = '\0';   /* Forget the name if we have one */
                }
#ifdef IKS_OPTION
            }
#endif /* IKS_OPTION */
        }
        srvidl = timelimit = iks_timo;  /* Reset command timelimit */
        debug(F101,"MAIN LOGIN","",x_logged);
        if (!x_logged) {                /* Logins failed. */
            if (TELOPT_SB(TELOPT_KERMIT).kermit.me_start)
              errpkt((CHAR *)"Login Timeout");
            msleep(500);
            doexit(BAD_EXIT,0);
        }
        what = W_NOTHING;               /* Stay in known state */
#ifndef NOSERVER
        if (isguest) {
            en_pri = 0;                 /* No printing for anonymous users */
            en_mai = 0;                 /* No email for anonymous users */
            en_mkd = 0;                 /* Or directory creation */
            en_rmd = 0;                 /* Or directory removal */
            en_ena = 0;                 /* Or ENABLing DISABLEd items */
        }
#endif /* NOSERVER */

#ifndef NOSPL
/*
  If a macro named "on_login" is defined, execute it.  Also remove it from the
  macro table so the user cannot see what it does.  Execute it as part of the
  iksd.conf file.
*/
        if (nmac) {                     /* Any macros defined? */
            int k;                      /* Yes */
            char * cmd = "on_login";    /* MSVC 2.x compiler error */
            k = mlook(mactab,cmd,nmac); /* Look up "on_login" */
            if (k >= 0) {               /* If found, */
#ifdef IKSDCONF
                int saved = iksdcf;
                iksdcf = 0;
#endif /* IKSDCONF */
                if (dodo(k,"",0) > -1)  /* set it up, */
                  parser(1);            /* execute it */
#ifdef IKSDCONF
                iksdcf = saved;
#endif /* IKSDCONF */
                delmac(cmd,1);          /* and delete it */
            }
        }
#endif /* NOSPL */
    } /* if (inserver) */
#else /* CK_LOGIN */
    if (inserver)
        srvidl = timelimit = iks_timo;  /* Set idle limits for IKS */
#endif /* CK_LOGIN */
#endif /* IKSD */
}

VOID
#ifdef CK_ANSIC
failcmdline(void * foo)
#else /* CK_ANSIC */
failcmdline(foo) VOID * foo;
#endif /* CK_ANSIC */
{
#ifdef GEMDOS
    cc_clean();
#endif /* GEMDOS */
#ifndef NOLOCAL
    if (cnflg) doconect(0,0);           /* connect again if requested. */
    if (ttchk() < 0)
      dologend();
#endif /* NOLOCAL */
}
#endif /* NOCMDL */

#ifndef NOICP
VOID
#ifdef CK_ANSIC
dotakeini(void * threadinfo)            /* Execute init file. */
#else  /* CK_ANSIC */
dotakeini(threadinfo) VOID * threadinfo; /* Execute init file. */
#endif /* CK_ANSIC */
/* dotakeini */ {
#ifdef NTSIG
    setint();
    if (threadinfo) {                   /* Thread local storage... */
       TlsSetValue(TlsIndex,threadinfo);
       debug(F100, "dotakeini called with threadinfo block","", 0);
    } else
      debug(F100, "dotakeini - threadinfo is NULL", "", 0);
#endif /* NTSIG */
#ifdef CK_LOGIN
#ifdef NT
#ifdef IKSD
    if (inserver)
      setntcreds();
#endif /* IKSD */
#endif /* NT */
#endif /* CK_LOGIN */
    cmdini();                           /* Sets tlevel */

    debug(F111,"dotakeini","inserver",inserver);
    debug(F111,"dotakeini","sstelnet",sstelnet);

#ifdef COMMENT
/* Wrong place for this... */
#ifndef NOXFER
#ifdef CK_FAST
    dofast();                           /* By now FAST defaults should be OK */
#endif /* CK_FAST */
#endif /* NOXFER */
#endif /* COMMENT */

    doinit();                           /* Now do the initialization file */
    debug(F101,"main executing init file","",tlevel);
    while (tlevel > -1) {
        sstate = (CHAR) parser(1);      /* Execute one command at a time. */
        if (sstate) proto();            /* Enter protocol if requested. */
#ifdef NTSIG
        ck_ih();
#endif /* NTSIG */
    }
    debug(F101,"main exits init file","",tlevel);

#ifdef NTSIG
    ckThreadEnd(threadinfo);
#endif /* NTSIG */
    return;
}

VOID
#ifdef CK_ANSIC
failtakeini(void * threadinfo)
#else /* CK_ANSIC */
failtakeini(threadinfo) VOID * threadinfo;
#endif /* CK_ANSIC */
/* failtakeini */ {
#ifdef GEMDOS
    cc_clean();                         /* Atari: Clean up after ^C-trap. */
#endif /* GEMDOS */
    fixcmd();
    if (!cfilef) {
        conoll("Interrupted during initialization or \
command-line processing.");
        conoll("C-Kermit quitting...");
    }
    doexit(BAD_EXIT,-1);                /* Exit with bad status. */
}

VOID
#ifdef CK_ANSIC
doicp(void * threadinfo)
#else /* CK_ANSIC */
doicp(threadinfo) VOID * threadinfo;
#endif /* CK_ANSIC */
/* doicp */ {
#ifdef NTSIG
    setint();
    if (threadinfo) {                   /* Thread local storage... */
       if (!TlsSetValue(TlsIndex,threadinfo))
          debug(F101,"doicp TlsSetValue failed","",GetLastError());
       debug(F101, "doicp a threadinfo block - TlsIndex", "", TlsIndex);
    } else {
        debug(F100, "doicp received a null threadinfo", "", 0);
    }
#endif /* NTSIG */
#ifdef CK_LOGIN
#ifdef NT
#ifdef IKSD
    if (inserver)
      setntcreds();
#endif /* IKSD */
#endif /* NT */
#endif /* CK_LOGIN */
#ifdef MAC
    while (1) {
        extern char *lfiles;            /* Fake pointer cast */

        if (connected) {
            debug(F100, "doicp: calling macparser", "", 0);
            sstate = newparser(1, 1, 0L);

            /* ignore null command state */
            if (sstate == 'n')
              sstate = '\0';

            if (sstate)
              proto();
        } else {
            /*
             * process take files the finder gave us.
             */
            if ((tlevel == -1) && lfiles)
              startlfile();

            debug(F100, "doicp: calling parser", "", 0);
            sstate = (CHAR) parser(0);
            if (sstate == 'c')          /* if MAC connect */
              sstate = 0;
            if (sstate)
              proto();
        }
    }
#else /* Not MAC */

#ifndef NOSPL
/*
  If interactive commands were given on the command line (using the
  -C "command, command, ..." option), assign them to a macro called
  "cl_commands", then execute the macro and leave it defined for
  subsequent re-execution if desired.
*/
    if (clcmds) {                       /* Check for -C commands */
        int x;
        x = addmac("cl_commands",clcmds); /* Put macro in table */
        if (x > -1) {                   /* If successful, */
            dodo(x,NULL,CF_CMDL);       /* set up for macro execution */
            while (maclvl > -1) {       /* Loop getting macro commands. */
                sstate = (CHAR) parser(1);
                if (sstate) proto();    /* Enter protocol if requested. */
#ifdef NTSIG
                ck_ih();
#endif /* NTSIG */
            }
        }
        debug(F100,"doicp calling herald","",0);
        herald();
    }
#endif /* NOSPL */
    while(1) {                          /* Loop getting commands. */
        sstate = (CHAR) parser(0);
        if (sstate) proto();            /* Enter protocol if requested. */
#ifdef NTSIG
       ck_ih();
#endif /* NTSIG */
    }
#ifdef NTSIG
    ckThreadEnd(threadinfo);
#endif /* NTSIG */
#endif /* MAC */
}

VOID
#ifdef CK_ANSIC
failicp(void * threadinfo)
#else /* CK_ANSIC */
failicp(threadinfo) VOID * threadinfo;
#endif /* CK_ANSIC */
{
#ifdef GEMDOS
    cc_clean();
#endif /* GEMDOS */
    fixcmd();                           /* Pop command stacks, etc. */
    clcmds = NULL;
    debug(F100,"ckcmai got interrupt","",0);
}
#endif /* NOICP */

#ifndef NOICP
VOID
#ifdef CK_ANSIC
docmdfile(void * threadinfo)            /* Execute application file */
#else /* CK_ANSIC */
docmdfile(threadinfo) VOID * threadinfo;
#endif /* CK_ANSIC */
/* docmdfile */ {
#ifdef NTSIG
    concb((char)escape);
    setint();
    if (threadinfo) {                   /* Thread local storage... */
        TlsSetValue(TlsIndex,threadinfo);
        debug(F100, "docmdfile called with threadinfo block","", 0);
    } else debug(F100, "docmdfile - threadinfo is NULL", "", 0);
#endif /* NTSIG */
#ifdef CK_LOGIN
#ifdef IKSD
#ifdef NT
    if (inserver)
      setntcreds();
#endif /* NT */
#endif /* IKSD */
#endif /* CK_LOGIN */
    debug(F110,"main cmdfil",cmdfil,0);
#ifndef NOSPL
    addmac("\\%0",cmdfil);
#endif /* NOSPL */
    dotake(cmdfil);			/* Set up the command file */
    if (tlevel > -1)			/* Remember we did this */
      cfilef = 1;
    while (tlevel > -1) {               /* Execute it until it runs out. */
        sstate = parser(1);             /* Loop getting commands. */
        if (sstate) proto();            /* Enter protocol if requested. */
#ifdef NTSIG
        ck_ih();
#endif /* NTSIG */
    }

#ifdef NTSIG
    ckThreadEnd(threadinfo);
#endif /* NTSIG */
    return;
}

VOID
#ifdef CK_ANSIC
failcmdfile(void * threadinfo)
#else /* CK_ANSIC */
failcmdfile(threadinfo) VOID * threadinfo;
#endif /* CK_ANSIC */
/* failcmdfile */ {
#ifdef GEMDOS
    cc_clean();                         /* Atari: Clean up after ^C-trap. */
#endif /* GEMDOS */
    fixcmd();
    if (!cfilef) {
	conoll("Interrupted during initialization or \
command-line processing.");
	conoll("C-Kermit quitting...");
    }
    doexit(BAD_EXIT,-1);                /* Exit with bad status. */
}
#endif /* NOICP */

#ifndef NOXFER
VOID
setprefix(z) int z; {                   /* Initial control-char prefixing */
#ifdef CK_SPEED
    int i, val;

    prefixing = z;
    ptab[protocol].prefix = prefixing;
    debug(F101,"setprefix","",prefixing);
    switch (z) {
      case PX_ALL:                      /* All */
#ifdef COMMENT
        /* Don't let Clear-Channel be dependent on prefixing */
        clearrq = 0;                    /* Turn off clearchannel, fall thru */
#endif /* COMMENT */
      case PX_NON:                      /* None */
        val = (z == PX_ALL) ? 1 : 0;
        for (i =
#ifdef UNPREFIXZERO
             0
#else
             1
#endif /* UNPREFIXZERO */
             ; i < 32; i++)
          ctlp[i] = val;
        for (i = 127; i < 160; i++) ctlp[i] = val;
        ctlp[(unsigned)255] = val;
        if (z == PX_NON) {              /* These are never safe */
            if (network) {              /* Assume network = telnet or rlogin */
                ctlp[CR] = 1;           /* Prefix CR because of NVT rules */
                ctlp[XON] = ctlp[XOFF] = 1; /* Because of Telnet server */
                ctlp[127] = ctlp[255] = 1;  /* Telnet IAC */
                ctlp[mystch] = ctlp[mystch+128] = 1; /* Kermit packet start */
            } else {
                ctlp[CR] = ctlp[255] = ctlp[mystch] = ctlp[mystch+128] = 1;
                if (flow == FLO_XONX)       /* Xon/Xoff forces prefixing */
                  ctlp[XON] = ctlp[XOFF] = ctlp[XON+128] = ctlp[XOFF+128] = 1;
            }
        }
        break;

      case PX_CAU:                      /* Cautious or Minimal */
#ifdef COMMENT
        /* Don't let CLEAR-CHANNEL be dependent on Prefixing */
        clearrq = 0;                    /* Turn off clearchannel */
#endif /* COMMENT */
      case PX_WIL:                      /* Minimal ("wild") */
        ctlp[0] = 1;                    /* Does not include 0 */
        for (i = 1; i < 32; i++)
          ctlp[i] = 0;
        for (i = 127; i < 160; i++)
          ctlp[i] = 0;
        ctlp[mystch] = ctlp[mystch+128] = 1; /* Kermit start of packet */
        if (seol != 13)
          ctlp[seol] = ctlp[seol+128] = 1; /* Kermit end */
        ctlp[13] = ctlp[141] = 1;       /* In case of TELNET (NVT rules) */
        ctlp[(unsigned)255] = 1;        /* Ditto */

        /* ^D, ^J, ^M, or ^U followed by tilde trigger Rlogin escape */

        ctlp[4]  = ctlp[4+128]  = 1;    /* In case of RLOGIN */
        ctlp[10] = ctlp[10+128] = 1;    /* In case of RLOGIN */
        ctlp[21] = ctlp[21+128] = 1;    /* In case of RLOGIN */

        if (flow == FLO_XONX ||         /* Xon/Xoff forces prefixing these */
            prefixing == PX_CAU ||      /* So does CAUTIOUS */
            network)                    /* Networks too... */
          ctlp[XON] = ctlp[XOFF] = ctlp[XON+128] = ctlp[XOFF+128] = 1;
        if (prefixing == PX_CAU) {      /* Cautious - add some more */
#ifdef UNPREFIXZERO
            ctlp[0] = 1;
#endif /* UNPREFIXZERO */
            ctlp[3]   = ctlp[16]  = 1;             /* ^C, DLE */
            ctlp[14]  = ctlp[15]  = 1;             /* SO/SI */
            ctlp[24]  = ctlp[25]  = 1;             /* VMS might need these */
            ctlp[26]  = ctlp[26+128] = 1;          /* UNIX suspend */
            ctlp[28]  = ctlp[29]  = ctlp[30]  = 1; /* Assorted esc chars */
            ctlp[131] = ctlp[141] = ctlp[144] = 1; /* and 8-bit versions */
            ctlp[(unsigned)255] = ctlp[156] = ctlp[157] = ctlp[158] = 1;
        }
        break;
    }
#endif /* CK_SPEED */
}
#endif /* NOXFER */

VOID
makever() {                             /* Make version string from pieces */
    int x, y;
    char * s;
#ifndef OS2
#ifndef MAC
    ck_s_xver = ck_s_ver;               /* Fill in C-Kermit version number */
    ck_l_xver = ck_l_ver;               /* for UNIX, VMS, etc. */
#endif /* MAC */
#endif /* OS2 */
    x = strlen(ck_s_name);
    y = strlen(ck_s_xver);
    if (y + x + 1 < CKVERLEN) {
        ckmakmsg(versio,CKVERLEN,ck_s_name," ",ck_s_xver,NULL);
    } else {
        ckstrncpy(versio,"C-Kermit",CKVERLEN);
        return;
    }
    x += y + 1;
  
    s = " OPEN SOURCE:";		/* C-Kermit 9.0 and later */
    y = strlen(s);
    if (CKVERLEN < x + y + 1)
      return;
    ckstrncat(versio,s,CKVERLEN);

    x += y + 1;
    if (*ck_s_who) {
        y = strlen(ck_s_who);
        if (CKVERLEN < x + y + 1)
          return;
        ckstrncat(versio,"-",CKVERLEN);
        ckstrncat(versio,ck_s_who,CKVERLEN);
    }
    x += y + 1;
    y = strlen(ck_s_test);
    if (y > 0 && y + x + 1 < CKVERLEN) {
        ckstrncat(versio," ",CKVERLEN);
        ckstrncat(versio,ck_s_test,CKVERLEN);
        x += y + 1;
        y = strlen(ck_s_tver);
        if (y > 0 && y + x + 1 < CKVERLEN) {
            ckstrncat(versio,".",CKVERLEN);
            ckstrncat(versio,ck_s_tver,CKVERLEN);
            x += y + 1;
        }
    }
    y = strlen(ck_s_date);
    if (y > 0 && y + x + 2 < CKVERLEN) {
        ckstrncat(versio,", ",CKVERLEN);
        ckstrncat(versio,ck_s_date,CKVERLEN);
    }
    vernum = ck_l_ver;
    xvernum = ck_l_xver;
    debug(F110,"Kermit version",versio,0);
}

union ck_short shortbytes;              /* For determining byte order */
int byteorder = 0;                      /* 0 = Big Endian; 1 = Little Endian */
int bigendian = 1;
/* NOTE: MUST BE 0 or 1 - nothing else */

#ifndef NOSPL
#define SCRIPTLEN 10240
#endif	/* NOSPL */

#ifdef NETCONN
#ifndef NOCMDL
#ifndef NOURL
VOID
dourl() {
    int rc = 0;
    char * port = NULL;
    extern int ttnproto;
    extern struct urldata g_url;

#ifdef COMMENT
    /* NOTE: debug() doesn't work yet - must use printf's */
    printf("URL:  %s\n",g_url.sav ? g_url.sav : "(none)");
    printf("Type: %s\n",g_url.svc ? g_url.svc : "(none)");
    printf("User: %s\n",g_url.usr ? g_url.usr : "(none)");
    printf("Pass: %s\n",g_url.psw ? g_url.psw : "(none)");
    printf("Host: %s\n",g_url.hos ? g_url.hos : "(none)");
/*  printf("Port: %s\n",g_url.por ? g_url.por : "(none)"); */
    printf("Path: %s\n",g_url.pth ? g_url.pth : "(none)");
#endif /* COMMENT */

    if (!ckstrcmp(g_url.svc,"iksd",-1,0) ||
        !ckstrcmp(g_url.svc,"kermit",-1,0)) {
        extern char pwbuf[];
        extern int pwflg;
#ifdef OS2
        extern int pwcrypt;
#endif /* OS2 */

        if (!g_url.hos) {
            printf("?Incomplete IKSD URL\n");
            doexit(BAD_EXIT,1);
        }
        if (!g_url.usr)
            makestr(&g_url.usr,"anonymous");
        if (!g_url.psw) {
            char * tmpbuf = NULL;
            if (!(tmpbuf = (char *)malloc(1024)))
                fatal("dourl: out of memory");
            if (!ckstrcmp(g_url.usr,"anonymous",-1,0)) {
                ckmakmsg(tmpbuf,1024,uidbuf,"@",myhost,NULL);
                makestr(&g_url.psw,tmpbuf);
            } else {
                readpass(" Password:",tmpbuf,1024);
                makestr(&g_url.psw,tmpbuf);
            }
            free(tmpbuf);
        }
        port = "kermit";
        ttnproto = NP_TELNET;
        nettype = NET_TCPB;
        mdmtyp = -nettype;
        local = -1;
        ckstrncpy(uidbuf,g_url.usr,UIDBUFLEN);
        if (g_url.psw) {
            ckstrncpy(pwbuf,g_url.psw,PWBUFL);
            pwflg = 1;
#ifdef OS2
            pwcrypt = 0;
#endif /* OS2 */
        }
        ckmakmsg(ttname,
                 TTNAMLEN,
                 g_url.hos,
                 ":",
                 g_url.por ? g_url.por : port,
                 NULL
                 );
        rc = ttopen(ttname,&local,mdmtyp,0);
        if (rc > -1) {
            network = 1;
            exitonclose = 1;
#ifdef CKLOGDIAL
            dolognet();
#endif /* CKLOGDIAL */
        } else {
            printf("?Connection failed: %s\n",g_url.sav);
            doexit(BAD_EXIT,1);
        }
        /* Also need to check here for secure authentication already done */

#ifdef NOSPL
        cflg = 1;
#else
	{
            char * script = NULL;
            if (!(script = (char *)malloc(SCRIPTLEN)))
              fatal("dourl: out of memory");
            if (!g_url.pth) {           /* Write the appropriate script */
		cflg = 1;
                ckmakxmsg(script,SCRIPTLEN,
			  "if not eq {\\v(authstate)} {user} ",
			  "if not eq {\\v(authstate)} {valid} { ",
                          "remote login ", /* No path */
                          g_url.usr,       /* Just log in and CONNECT */
                          " ",
                          g_url.psw,
                          ", if fail exit 1 {IKSD login failed} }",
                          ", connect",
                          NULL,NULL,NULL,NULL);
		/* printf("CLCMDS 1: %s\n",script); */
            } else {
                /* does the path specify a file or a directory? */
                int len = strlen(g_url.pth);
                if (ISDIRSEP(g_url.pth[len-1])) {
		    ckmakxmsg(script,SCRIPTLEN, /* Directory name given */
			      "if not eq {\\v(authstate)} {user} \
if not eq {\\v(authstate)} {valid} { remote login ",
			      g_url.usr,
			      " ",
			      g_url.psw,
			      ", if fail exit 1 {IKSD login failed} }",
			      ", set macro error on",
			      ", set xfer displ brief",
			      ", set xfer bell off",
			      ", remote cd ",
			      g_url.pth,
			      ", lineout directory",
			      ", connect"
			      );
		    /* printf("CLCMDS 2: %s\n",script); */
		} else {
		    ckmakxmsg(script,SCRIPTLEN, /* Path given, try to GET */
			      "if not eq {\\v(authstate)} {user} \
if not eq {\\v(authstate)} {valid} { remote login ",
			      g_url.usr,
			      " ",
			      g_url.psw,
			      ", if fail exit 1 {IKSD login failed} }",
			      ", set xfer displ brief",
			      ", set xfer bell off",
			      ", get ",
			      g_url.pth,
			      ", .rc := \\v(status)",
			      ", if open connection bye",
			      ", exit \\m(rc)"
			      );
		    /* printf("CLCMDS 2: %s\n",script); */
		}
            }
            clcmds = script;		/* Make this our -C cmdline macro */
	    /* printf("HAVEURL=%d\n",haveurl); */
        }
#endif /* NOSPL */
    } else {
        if (ckstrcmp(g_url.svc,"telnet",-1,0) &&
#ifdef SSHBUILTIN
            ckstrcmp(g_url.svc,"ssh",-1,0) &&
#endif /* SSHBUILTIN */
            ckstrcmp(g_url.svc,"ftp",-1,0)) {
            printf("?Sorry, %s URLs not supported\n",
                   g_url.svc ? g_url.svc : "");
            doexit(BAD_EXIT,1);
        }
    }
}
#endif /* NOCMDL */
#endif /* NETCONN */
#endif /* NOURL */

/*
  main()...

  If you get complaints about "main: return type is not blah",
  define MAINTYPE on the CC command line, e.g. "CFLAGS=-DMAINTYPE=blah"
  (where "blah" is int, long, or whatever).

  If the complaint is "Attempt to return a value from a function of type void"
  then add -DMAINISVOID.
*/
#ifndef MAINTYPE
#ifndef MAINISVOID
#define MAINTYPE int
#endif /* MAINISVOID */
#endif /* MAINTYPE */

#ifdef MAINISVOID
#ifndef MAINTYPE
#define MAINTYPE void
#endif /* MAINTYPE */
#endif /* MAINISVOID */

#ifdef aegis
/* On the Apollo, intercept main to insert a cleanup handler */
int
ckcmai(argc,argv) int argc; char **argv;
#else
#ifdef MAC                              /* Macintosh */
int
main (void)
#else
#ifdef __GNUC__                         /* GCC compiler */
int
main(argc,argv) int argc; char **argv;
#else
#ifdef __DECC                           /* DEC Alpha with DEC C compiler */
#ifdef __ALPHA
int
main(argc,argv) int argc; char **argv;
#else                                   /* DEC C compiler, not Alpha */
#define MAINISVOID
VOID
main(argc,argv) int argc; char **argv;
#endif  /* __ALPHA */
#else
#ifdef STRATUS                          /* Stratus VOS */
int
main(argc,argv) int argc; char **argv;
#else                                   /* K-95 */
#ifdef OS2
#ifdef KUI
#define MAINISVOID
void
Main( int argc, char ** argv )
#else /* KUI */
#define MAINISVOID
VOID
main(argc,argv) int argc; char **argv;
#endif /* KUI */
#else  /* Not K95 */
MAINTYPE                                /* All others... */
main(argc,argv) int argc; char **argv;
#endif /* OS2 */
#endif /* STRATUS */
#endif /* __DECC */
#endif /* __GNUC__ */
#endif /* MAC */
#endif /* aegis */

/* main */ {

    char *p;

#ifndef NOSETKEY
    int i;
#endif /* NOSETKEY */

#ifdef datageneral
    short *pfha = 016000000036;         /* Get around LANG_RT problem */
    *pfha = (short) 0;                  /* No user protection fault handler */
#endif /* datageneral */

    int unbuf = 0;			/* nonzero for unbuffered stdout */

/* setbuf has to be called on the file descriptor before it is used */

#ifdef UNIX
#ifdef NONOSETBUF			/* Unbuffered console i/o */
    unbuf++;				/* as a compile-time option */
#endif	/* NONOSETBUF */
    if (!unbuf) {			/* Or as a command-line selection */
	int i, n;			/* We have to pre-pre-scan for */
	char * s;			/* this one. */
	for (i = 1; i < argc; i++) {
	    s = argv[i];
	    if (!s) n = 0; else n = (int)strlen(s);
	    if (n > 4) {
		if (!ckstrcmp("--unbuffered",s,n,0)) {
		    unbuf++;
		    break;
		}
	    }
	}
    }
    if (unbuf)
      setbuf(stdout,NULL);
#endif	/* UNIX */

/* Do some initialization */

#ifdef VMS
#ifdef __DECC
    /* Get some RMS default settings. */
    get_rms_defaults();
#endif /* def __DECC */
#endif /* def VMS */

#ifndef MAC
    xargc = xargs = argc;               /* Make global copies of argc */
    xargv = argv;                       /* ...and argv. */
    xarg0 = argv[0];
#ifdef NT
    setOSVer();
#endif /* NT */
    zstrip(argv[0],&p);                 /* Get name we were invoked with */
    makestr(&myname,p);
    if (!ckstrcmp(myname,"telnet",-1,0))       howcalled = I_AM_TELNET;
#ifdef CK_KERBEROS
    else if (!ckstrcmp(myname,"ktelnet",-1,0)) howcalled = I_AM_TELNET;
#endif /* CK_KERBEROS */
    else if (!ckstrcmp(myname,"rlogin",-1,0))  howcalled = I_AM_RLOGIN;
    else if (!ckstrcmp(myname,"iksd",-1,0))    howcalled = I_AM_IKSD;
#ifdef NEWFTP
    else if (!ckstrcmp(myname,"ftp",-1,0))     howcalled = I_AM_FTP;
#endif /* NEWFTP */
#ifndef NOHTTP
    else if (!ckstrcmp(myname,"http",-1,0))    howcalled = I_AM_HTTP;
#endif /* NOHTTP */
#ifdef OS2
    else if (!ckstrcmp(myname,"telnet.exe",-1,0))  howcalled = I_AM_TELNET;
#ifdef SSHBUILTIN
    else if (!ckstrcmp(myname,"ssh",-1,0))  howcalled = I_AM_SSH;
    else if (!ckstrcmp(myname,"ssh.exe",-1,0))  howcalled = I_AM_SSH;
#endif /* SSHBUILTIN */
#ifdef CK_KERBEROS
    else if (!ckstrcmp(myname,"ktelnet.exe",-1,0)) howcalled = I_AM_TELNET;
#endif /* CK_KERBEROS */
    else if (!ckstrcmp(myname,"rlogin.exe",-1,0))  howcalled = I_AM_RLOGIN;
#ifdef NT
    else if (!ckstrcmp(myname,"iksdnt",-1,0))    howcalled = I_AM_IKSD;
    else if (!ckstrcmp(myname,"iksdnt.exe",-1,0))    howcalled = I_AM_IKSD;
#endif /* NT */
#ifdef NEWFTP
    else if (!ckstrcmp(myname,"ftp.exe",-1,0))     howcalled = I_AM_FTP;
#endif /* NEWFTP */
#ifndef NOHTTP
    else if (!ckstrcmp(myname,"http.exe",-1,0))    howcalled = I_AM_HTTP;
#endif /* NOHTTP */
#endif /* OS2 */
    else if (!ckstrcmp(myname,"kermit-sshsub",-1,0)) howcalled = I_AM_SSHSUB;

#ifndef NOICP
    cmdini();                           /* Must come before prescan */
    debug(F100,"main cmdini() done","",0);
#endif /* NOICP */
    prescan(0);                         /* Pre-Check for debugging, etc */
#endif /* MAC */
    debug(F101,"MAIN feol","",feol);
    makever();                          /* Put together version strings */
#ifndef NOSETKEY                        /* Allocate & initialize the keymap */
    /* This code has been moved to before sysinit() for K95G */
    if (!(keymap = (KEY *) malloc(sizeof(KEY)*KMSIZE)))
      fatal("main: no memory for keymap");
    if (!(macrotab = (MACRO *) malloc(sizeof(MACRO)*KMSIZE)))
      fatal("main: no memory for macrotab");
    for (i = 0; i < KMSIZE; i++) {
       keymap[i] = (KEY) i;
       macrotab[i] = NULL;
    }
#endif /* NOSETKEY */

    shortbytes.x_short = 0xABCD;        /* Get Endianness */
    if (shortbytes.x_char[0] == 0xCD) { /* 0 = Big Endian */
        byteorder = 1;                  /* 1 = Little Endian */
        bigendian = 0;                  /* (for clarity in programming) */
    } else {
        byteorder = 0;                  /* Big Endian */
        bigendian = 1;
    }
    if (sizeof(CK_OFF_T) == 8)		/* Large files and ints? */
      havelfs = 1;

    if (sysinit() < 0)                  /* System-dependent initialization. */
      fatal("Can't initialize!");
    else
      initflg = 1;                      /* Remember we did. */
    debug(F111,"ckcmai myname",myname,howcalled);

#ifdef UNIX
    getexedir();                        /* Compute exedir variable */
#endif /* UNIX */

#ifdef CKSYSLOG
#ifdef SYSLOGLEVEL
/*
  If built with -DSYSLOGLEVEL on cc command line, this means we always
  do syslogging at the indicated level.
*/
    zsyslog();                          /* Open syslog */
#else /* SYSLOGLEVEL */
#ifdef IKSD
    if (inserver)
      zsyslog();                        /* Open syslog */
#endif /* IKSD */
#endif /* SYSLOGLEVEL */
#endif /* CKSYSLOG */

#ifdef CK_KERBEROS
    ini_kerb();                         /* Initialize Kerberos data */
#endif /* CK_KERBEROS */
#ifdef CK_SSL
    ssl_once_init();
#endif /* CK_SSL */
#ifdef TNCODE
    tn_set_modes();                     /* Init Telnet Option tables */
#endif /* TNCODE */

#ifdef CK_TTGWSIZ                       /* Initialize screen dimensions */
#ifdef OS2
    ttgcwsz();
#else /* OS2 */
    if (ttgwsiz() > 0) {
        if (tt_rows > 0 && tt_cols > 0) {
            cmd_rows = tt_rows;
            cmd_cols = tt_cols;
        }
    }
#endif /* OS2 */
#endif /* CK_TTGWSIZ */

#ifndef OS2
#ifdef TCPSOCKET
#ifdef CK_SOCKS
    SOCKSinit(argv[0]);                 /* Internet relay package... */
#endif /* CK_SOCKS */
#endif /* TCPSOCKET */
#endif /* OS2 */

    initflow();                         /* Initialize flow-control table */

#ifndef NOICP
#ifdef CKFLOAT
    initfloat();                        /* Deduce floating-point precision */
#endif /* CKFLOAT */
#endif /* NOICP */

#ifndef NOXFER
    initxlist();			/* Init exception lists */

#ifdef CK_XYZ                           /* Initialize protocols...  */

#ifdef XYZ_INTERNAL /* XYZMODEM are internal ... */

#ifdef COMMENT
    /* Can't do this for XMODEM because if filename contains a "C" etc... */
    initproto(PROTO_X, "rx %s","rx %s", NULL, NULL, NULL, NULL, NULL);
    initproto(PROTO_XC,"rc %s","rc %s", NULL, NULL, NULL, NULL, NULL);
#else /* COMMENT */
    initproto(PROTO_X, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    initproto(PROTO_XC,NULL, NULL, NULL, NULL, NULL, NULL, NULL);
#endif /* COMMENT */
    initproto(PROTO_Y, "rb","rb", NULL, NULL, NULL, NULL, NULL);
    initproto(PROTO_G, "rb","rb", NULL, NULL, NULL, NULL, NULL);
    initproto(PROTO_Z, "rz","rz", NULL, NULL, NULL, NULL, NULL);
   initproto(PROTO_K,"kermit -ir","kermit -r","kermit -x",NULL,NULL,NULL,NULL);
    /* Kermit Must be last */

#else /* XYZMODEM are external protocols ... */

 /*                  s1      s2     s3    s4      s5         s6      s7     */
 initproto(PROTO_X, "rx %s","rx %s",NULL,"sx %s","sx -a %s","rx %s", "rx %s");
 initproto(PROTO_XC,"rc %s","rc %s",NULL,"sx %s","sx -a %s","rc %s", "rc %s");
 initproto(PROTO_Y, "rb",   "rb",   NULL,"sb %s","sb -a %s","rb",    "rb"   );
 initproto(PROTO_G, "rb",   "rb",   NULL,"sb %s","sb -a %s","rb",    "rb"   );
 initproto(PROTO_Z, "rz",   "rz",   NULL,"sz %s","sz -a %s","rz",    "rz"   );
 initproto(PROTO_K, "kermit -ir","kermit -r","kermit -x",NULL,NULL,NULL,NULL);
 /* Kermit must be last */

#endif /* XYZ_INTERNAL */

#else  /* No XYZMODEM support */

   initproto(PROTO_K,"kermit -ir","kermit -r","kermit -x",NULL,NULL,NULL,NULL);

#endif /* CK_XYZ */
#endif /* NOXFER */

    connoi();                           /* Console interrupts off */

#ifndef NOXFER
#ifdef OS2
    /* Initialize Kermit and Zmodem Auto-Download Strings */
    adl_kstr = strdup("KERMIT READY TO SEND...");
    adl_zstr = strdup("rz\r");
#endif /* OS2 */

#ifdef PATTERNS
    initpat();                          /* Initialize filename patterns */
#endif /* PATTERNS */
#endif /* NOXFER */

#ifndef NOCSETS
    initcsets();                        /* Initialize character sets */
#endif /* NOCSETS */

#ifndef NOICP
#ifdef DFCDMSG
    makestr(&cdmsgstr,DFCDMSG);
    makelist(cdmsgstr,cdmsgfile,8);     /* Initialize CD message filenames */
#endif /* DFCDMSG */
#endif /* NOICP */

    sstate = 0;                         /* No default start state. */
#ifdef DYNAMIC
    if (getiobs() < 0)
      fatal("Can't allocate i/o buffers!");
#endif /* DYNAMIC */

#ifndef NOSPL
#ifndef NORANDOM
    {
        char stackdata[256];
        unsigned int c = 1234, n;
        /* try to make a random unsigned int to feed srand() */
#ifndef VMS
	/* time.h and MultiNet do not get along */
        c = time(NULL);
#endif /* VMS */
        c *= getpid();
	/* Referenced before set... DELIBERATELY */
        for (n = 0; n < sizeof(stackdata); n++) /* IGNORE WARNING */
	  c += stackdata[n];		/* DELIBERATELY USED BEFORE SET */
        srand((unsigned int)c);
    }
#endif /* NORANDOM */
#endif /* NOSPL */

    ckhost(myhost,MYHOSTL);             /* Name of local host */
    debug(F110,"main ckhost",myhost,0);
#ifdef IKSD
    if (!inserver) {
#endif /* IKSD */
        ckstrncpy(ttname,dftty,TTNAMLEN); /* Set up default tty name. */
        local = nolocal ? 0 : dfloc;    /* And whether it's local or remote. */
        parity = dfprty;                /* Set initial parity, */
#ifndef NOXFER
        myindex = getsysix(cksysid);    /* System index */
#endif /* NOXFER */
        if (local) if (ttopen(ttname,&local,0,0) < 0) {
#ifndef OS2
            conol("Can't open device: ");
            conoll(ttname);
#endif /* OS2 */
            local = 0;
            ckstrncpy(ttname,CTTNAM,TTNAMLEN);
        }
        setflow();                      /* Set appropriate flow control */
        speed = ttgspd();               /* Get transmission speed. */
#ifdef IKSD
    }
#endif /* IKSD */

#ifdef ANYX25                           /* All X.25 implementations */
#ifndef IBMX25                          /* except IBM have PAD support */
    initpad();                          /* Initialize X.25 PAD */
#endif /* IBMX25 */
#endif /* ANYX25 */

#ifndef NOXFER
    if (inibufs(SBSIZ,RBSIZ) < 0)       /* Allocate packet buffers */
      fatal("Can't allocate packet buffers!");
#ifndef NOCKSPEED
    setprefix(prefixing);               /* Set up control char prefixing */
#endif /* NOCKSPEED */
#endif /* NOXFER */

#ifndef NOICP
    if (sstelnet
#ifdef IKSD
        || inserver
#endif /* IKSD */
        ) {
        int on = 1, x = 0;
        extern int ckxech, ttnet, ttnproto, cmdmsk;
#ifdef SO_SNDBUF
        extern int tcp_sendbuf;
#endif
#ifdef SO_RCVBUF
        extern int tcp_recvbuf;
#endif
#ifdef SO_KEEPALIVE
        extern int tcp_keepalive;
#endif
#ifdef SO_LINGER
        extern int tcp_linger, tcp_linger_tmo;
#endif /* SO_LINGER */
#ifdef SO_DONTROUTE
        extern int tcp_dontroute;
#endif /* SO_DONTROUTE */
#ifdef TCP_NODELAY
        extern int tcp_nodelay;
#endif /* TCP_NODELAY */
#ifdef IKSD
        extern int iklogopen;
#endif /* IKSD */
        extern int ttmdm;

#ifdef UNIX
        if (isatty(0))
          fatal("Internet Kermit Service cannot be started at a terminal.");
#endif /* UNIX */

        reliable = xreliable = SET_ON;  /* IKSD has reliable connection */
#ifndef VMS
        flow = 0;                       /* No flow control needed */
#endif /* VMS */
        bgset = 0;                      /* Not in background */
        nopush = 1;                     /* No external processes */
        parity = 0;                     /* 8 bits ... */
        cmdmsk = 0xff;                  /* all the way */
        cmask = 0xff;

#ifdef IKSD
        if (inserver) {                 /* If IKSD */
            doiksdinit();               /* Execute IKSD configuration file */
            while (tlevel > -1)
              parser(1);                /* (Ignore any file-xfer commands) */
            iksdcf = 1;                 /* IKSD c.f. has been processed */
        }
        if (!iklogopen) (VOID) doiklog(); /* Open Kermit-specific log */
#endif /* IKSD */

#ifdef UNIX
        setbuf(stdout,NULL);            /* Don't buffer the output */
        ckstrncpy(ttname,"0",TTNAMLEN); /* not "/dev/tty"... */
#endif /* UNIX */
        local = 0;                      /* We are in remote mode */
        ckxech = 1;                     /* We will echo */
#ifdef OS2
        nettype = NET_TCPB;             /* So ttopen() treats the connection */
        mdmtyp = -nettype;              /* as a network */
#endif /* OS2 */
        debug(F100,"main about to call ttopen() inserver","",0);
        if (ttopen(ttname,&local,mdmtyp,0) < 0) { /* Open comm channel */
            fatal("can't initialize i/o");
        }
#ifdef OS2
        local = 0;
        network = 1;                    /* Does use networking code */
#else  /* OS2 */
        network = 0;                    /* Does not use networking code */
#endif /* OS2 */
        ttmdm = -1;                     /* Does not use a modem */
        sstelnet = 1;                   /* Do server-side Telnet negotations */
        debug(F111,"MAIN","sstelnet",sstelnet);
        ttnet = NET_TCPB;               /* Network type is TCP sockets */
        ttnproto = NP_TELNET;           /* Netword protocol is Telnet */
#ifdef IKSDB
        dbinit();                       /* Initialize database record */
#endif /* IKSDB */
#ifndef OS2
#ifdef CK_AUTHENTICATION
        /* Before initializating Telnet/Rlogin negotiations, init Kerberos */
        ck_auth_init(ckgetpeer(),"","",0);
#endif /* CK_AUTHENTICATION */

#ifdef NON_BLOCK_IO
        on = 1;
        x = socket_ioctl(0,FIONBIO,&on);
        debug(F101,"main FIONBIO","",x);
#endif /* NON_BLOCK_IO */
#ifdef SO_OOBINLINE
        on = 1;
        x = setsockopt(0, SOL_SOCKET, SO_OOBINLINE, (char *)&on, sizeof(on));
        debug(F101,"main SO_OOBINLINE","",x);
#endif /* SO_OOBINLINE */

#ifndef NOTCPOPTS
#ifndef datageneral
#ifdef SOL_SOCKET
#ifdef TCP_NODELAY
        no_delay(0,tcp_nodelay);
#endif /* TCP_NODELAY */
#ifdef SO_KEEPALIVE
        keepalive(0,tcp_keepalive);
#endif /* SO_KEEPALIVE */
#ifdef SO_LINGER
        ck_linger(0,tcp_linger, tcp_linger_tmo);
#endif /* SO_LINGER */
#ifdef SO_DONTROUTE
        dontroute(0,tcp_dontroute);
#endif /* SO_DONTROUTE */
#ifdef SO_SNDBUF
        sendbuf(0,tcp_sendbuf);
#endif /* SO_SNDBUF */
#ifdef SO_RCVBUF
        recvbuf(0,tcp_recvbuf);
#endif /* SO_RCVBUF */
#endif /* SOL_SOCKET */
#endif /* datageneral */
#endif /* NOTCPOPTS */

#ifdef CK_SSL
        if (ck_ssleay_is_installed()) {
            if (!ssl_tn_init(SSL_SERVER)) {
                if (bio_err != NULL) {
                    BIO_printf(bio_err,"do_ssleay_init() failed\r\n");
                    ERR_print_errors(bio_err);
                } else {
                    fflush(stderr);
                    fprintf(stderr,"do_ssleay_init() failed\r\n");
                    ERR_print_errors_fp(stderr);
                }
                switch (ttnproto) {
		  case NP_SSL:
		  case NP_TLS:
  		  case NP_SSL_RAW:
		  case NP_TLS_RAW:
		  case NP_SSL_TELNET:
		  case NP_TLS_TELNET:
                    doexit(BAD_EXIT,1);
                }
                /* otherwise we will continue to accept the connection   */
                /* without SSL or TLS support unless required. */
                if ( TELOPT_DEF_S_ME_MODE(TELOPT_START_TLS) != TN_NG_MU )
                    TELOPT_DEF_S_ME_MODE(TELOPT_START_TLS) = TN_NG_RF;
                if ( TELOPT_DEF_S_U_MODE(TELOPT_START_TLS) != TN_NG_MU )
                    TELOPT_DEF_S_U_MODE(TELOPT_START_TLS) = TN_NG_RF;
                if ( TELOPT_DEF_C_ME_MODE(TELOPT_START_TLS) != TN_NG_MU )
                    TELOPT_DEF_C_ME_MODE(TELOPT_START_TLS) = TN_NG_RF;
                if ( TELOPT_DEF_C_U_MODE(TELOPT_START_TLS) != TN_NG_MU )
                    TELOPT_DEF_C_U_MODE(TELOPT_START_TLS) = TN_NG_RF;
            } else {
                if ( ck_ssl_incoming(0) < 0 ) {
                    doexit(BAD_EXIT,1);
                }
            }
        }
#endif /* CK_SSL */

#ifdef TNCODE
        tn_ini();                       /* Start Telnet negotiation now */
#endif /* TNCODE */
#endif /* OS2 */
    }
    debug(F101,"main argc after prescan()","",argc);

    /* Now process any relevant environment variables */

#ifndef NODIAL
    getdialenv();                       /* Dialing */
#ifdef NETCONN
    ndinit();                           /* Initialize network directory info */
    getnetenv();                        /* Network directories */
#endif /* NETCONN */
#endif /* NODIAL */

#ifndef NOXFER
#ifdef CK_FAST
    dofast();                           /* By now FAST defaults should be OK */
#endif /* CK_FAST */
#endif /* NOXFER */

#ifndef NOCMDL
    ikslogin();                          /* IKSD Login and other stuff */
#ifdef IKSD
#ifdef NT
    if ( inserver )
      setntcreds();
#endif /* NT */
#endif /* IKSD */
#endif /* NOCMDL */

    if (howcalled == I_AM_SSHSUB) {
        reliable = 1;			/* We say the connection is reliable */
        xreliable = 1;			/* And that we said it was */
        setreliable = 1;		/* And pretend the "user" did too */
        xfinish = 1;			/* For REMOTE HELP response */
        mdmtyp = 0;			/* For ttopen() */
        ckstrncpy(ttname,"0",TTNAMLEN+1);  /* Use file descriptor 0 */
        local = 0;                         /* And force remote mode */
        ttopen(ttname,&local,mdmtyp,0); /* Open the "connection" */
        sstate = 'x';			/* Initial state is Server */
        proto();			/* Enter protocol */
        doexit(GOOD_EXIT,xitsta);	/* Exit when done */
    }
    debug(F111,"howcalled",myname,howcalled);

#ifdef NOCCTRAP
    dotakeini(0);
#else /* NOCCTRAP */
    debug(F100,"main about to cc_execute","",0);
    setint();
    cc_execute( ckjaddr(cmjbuf), dotakeini, failtakeini );
#endif /* NOCCTRAP */

    debug(F111,"main 2 cfilef",cmdfil,cfilef);
    if (cmdfil[0]) {                    /* If we got one (see prescan())... */
#ifdef NOCCTRAP
        docmdfile(0);                   /* execute it. */
#else /* NOCCTRAP */
        setint();
        cc_execute( ckjaddr(cmjbuf), docmdfile, failcmdfile );
#endif /* NOCCTRAP */
    }
#ifndef OS2                             /* Preserve name so we can delete it */
    *cmdfil = '\0';                     /* Done, nullify the file name */
#endif /* OS2 */
#endif /* NOICP */

#ifndef NOCMDL
/* Look for a UNIX-style command line... */

    what = W_NOTHING;

    debug(F101,"main argc","",argc);
#ifndef NOHELP
    iniopthlp();                        /* Initialize cmdline arg help */
#endif /* NOHELP */
    if (
#ifdef COMMENT
        !cfilef &&
#endif /* COMMENT */
        argc > 1) {                     /* Command line arguments? */
        sstate = (CHAR) cmdlin();       /* Yes, parse. */
#ifdef NETCONN
#ifndef NOURL
        if (haveurl) {                  /* Was a URL given? */
            dourl();                    /* if so, do it. */
        }
#endif /* NOURL */
#endif /* NETCONN */
#ifndef NOXFER
        zstate = sstate;                /* Remember sstate around protocol */
        debug(F101,"main zstate","",zstate);
#endif /* NOXFER */

#ifndef NOLOCAL
        if (cflg) {                     /* Connect first if requested */
            doconect(0,0);
            if (ttchk() < 0)
              dologend();
            cflg = 0;
        }
#endif /* NOLOCAL */

#ifndef NOXFER
        if (sstate) {
#ifndef NOLOCAL
            if (displa) concb((char)escape); /* (for console "interrupts") */
#endif /* NOLOCAL */
#ifdef NOCCTRAP
            docmdline(1);
#else /* NOCCTRAP */
            setint();
            cc_execute( ckjaddr(cmjbuf), docmdline, failcmdline );
#endif /* NOCCTRAP */
        }
#endif /* NOXFER */

#ifndef NOICP
/*
  If a command-line action argument was given and -S ("stay") was not given,
  exit now.
*/
        if ((cflg || cnflg || zstate) && !stayflg)
#endif /* NOICP */
          doexit(GOOD_EXIT,xitsta);     /* Exit with good status */

#ifndef NOLOCAL
#ifndef NOICP
        if (local) {
#ifdef NETCONN
            if ((cflg || cnflg) && tn_exit && ttchk() < 0)
              doexit(GOOD_EXIT,xitsta); /* Exit with good status */
#endif /* NETCONN */
            if (exitonclose && !network &&
                (carrier != CAR_OFF && (ttgmdm() & BM_DCD) == 0))
              doexit(GOOD_EXIT,xitsta); /* Exit with good status */
            if (exitonclose && network && ttchk() < 0)
              doexit(GOOD_EXIT,xitsta); /* Exit with good status */
        }
#endif /* NOICP */
#endif /* NOLOCAL */
    }
#endif /* NOCMDL */

#ifdef NOICP                            /* No interactive command parser */
#ifndef NOCMDL
    else {

        /* Command-line-only version */
        fatal("?No command-line options given - type 'kermit -h' for help");
    }
#else                                   /* Neither one! */
        sstate = 'x';
        justone = 0;
        proto();                        /* So go into server mode */
        doexit(GOOD_EXIT,xitsta);       /* exit with good status */

#endif /* NOCMDL */
#else /* not NOICP */
/*
  If no action requested on command line, or if -S ("stay") was included,
  enter the interactive command parser.
*/
    if (!clcmds)
      herald();                         /* Display program herald. */

#ifdef NOCCTRAP
    debug(F100,"main NOCCTRAP setting interrupt trap","",0);
    setint();                           /* Set up command interrupt traps */
    doicp(NULL);
#else /* NOCCTRAP */
    while (1) {
        debug(F100,"main setting interrupt trap","",0);
        setint();                       /* Set up command interrupt traps */
        if (!cc_execute(ckjaddr(cmjbuf), doicp, failicp))
          break;
    }
#endif /* NOCCTRAP */
#endif /* NOICP */
#ifndef MAINISVOID
    return(1);
#endif /* MAINISVOID */
}

#ifdef DYNAMIC
/* Allocate file i/o buffers */

char *zinbuffer = NULL, *zoutbuffer = NULL;

int
getiobs() {
    zinbuffer = (char *)malloc(INBUFSIZE);
    if (!zinbuffer) return(-1);
    zoutbuffer = (char *)malloc(zobufsize);
    debug(F101,"zoutbuffer malloc","",zobufsize);
    if (!zoutbuffer) return(-1);
    debug(F100,"getiobs ok","",0);
    return(0);
}
#endif /* DYNAMIC */

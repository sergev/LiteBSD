#include "ckcsym.h"
#ifndef NOICP

/*
  Authors:
    Frank da Cruz <fdc@columbia.edu>,
      The Kermit Project, Columbia University, New York City
    Jeffrey E Altman <jaltman@secure-endpoints.com>
      Secure Endpoints Inc., New York City

  Copyright (C) 1985, 2011,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/

/* Includes */

#include "ckcdeb.h"
#include "ckcasc.h"
#include "ckcker.h"
#include "ckuusr.h"
#include "ckcxla.h"
#include "ckcnet.h"                     /* Network symbols */
#include <signal.h>

#ifndef NOSTAT
#ifdef VMS
/* 2010-03-09 SMS.  VAX C needs help to find "sys".  It's easier not to try. */
#include <stat.h>
#else /* def VMS */
#include <sys/stat.h>
#endif /* def VMS [else] */
#endif /* NOSTAT */

#ifdef VMS
#ifndef TCPSOCKET
#include <errno.h>
#endif /* TCPSOCKET */
#endif /* VMS */

#ifdef datageneral
#define fgets(stringbuf,max,fd) dg_fgets(stringbuf,max,fd)
#endif /* datageneral */

#ifdef QNX6
#define readblock kreadblock
#endif /* QNX6 */

/* External Kermit Variables, see ckmain.c for description. */

extern xx_strp xxstring;

extern int local, xitsta, binary, parity, escape, flow, cmd_rows, turn,
  turnch, duplex, ckxech, seslog, dfloc, cnflg, tlevel, pflag, msgflg, mdmtyp,
  zincnt, quiet, repars, techo, network, nzxopts, what, filepeek, recursive;

extern int xaskmore, tt_rows, tt_cols, cmd_cols, g_matchdot, diractive,
  xcmdsrc, nscanfile, reliable, nolinks, cmflgs;

#ifdef VMSORUNIX
extern int zgfs_dir, zgfs_link;
#endif /* VMSORUNIX */

#ifdef CK_IFRO
  extern int remonly;
#endif /* CK_IFRO */

#ifdef OS2
extern int StartedFromDialer ;
extern int vmode;
extern int k95stdout;
#ifndef NT
#define INCL_NOPM
#define INCL_VIO                        /* Needed for ckocon.h */
#include <os2.h>
#undef COMMENT
#else
#define APIRET ULONG
#include <windows.h>
#include <tapi.h>
#include "ckntap.h"
#endif /* NT */
#include "ckocon.h"
#endif /* OS2 */

extern long vernum, speed;
extern char *versio, *protv, *ckxv, *ckzv, *fnsv, *connv, *dftty, *cmdv;
extern char *dialv, *loginv, *for_def[], *whil_def[], *xif_def[], *sw_def[];
extern char *foz_def[];
extern char *ckxsys, *ckzsys;
#ifndef OS2
extern char *DIRCMD;
#ifndef UNIX
extern char *DELCMD;
#endif /* UNIX */
#endif /* OS2 */
extern char ttname[], filnam[];
extern CHAR sstate, feol;
extern char *zinptr;

#ifdef UNIX
extern char ** mtchs;                   /* zxpand() file list */
#endif /* UNIX */

#ifndef NOXFER
extern int oopts, omode, oname, opath;  /* O-Packet options */

extern int stdinf, sndsrc, size, rpsiz, urpsiz, fncnv, fnrpath, displa,
  stdouf, isguest, pktlog, nfils, keep, maxrps, fblksiz, frecl, frecfm,
  atcapr, atdiso, spsizf, spsiz, spsizr, spmax, wslotr, prefixing,
  fncact, fnspath, nprotos, g_proto, g_urpsiz, g_spsizf,
  g_spsiz, g_spsizr, g_spmax, g_wslotr, g_prefixing, g_fncact, g_fncnv,
  g_fnspath, g_fnrpath, xfrxla, g_xfrxla;

extern char *cmarg, *cmarg2;

#ifndef NOMSEND                         /* Multiple SEND */
extern char *msfiles[];
#endif /* NOMSEND */
extern char fspec[];                    /* Most recent filespec */
extern int fspeclen;

#ifdef CK_TMPDIR
extern int f_tmpdir;                    /* Directory changed temporarily */
extern char savdir[];                   /* For saving current directory */
#endif /* CK_TMPDIR */

extern struct keytab protos[];          /* File transfer protocols */
extern struct ck_p ptab[NPROTOS];
#endif /* NOXFER */

#ifdef DCMDBUF                          /* Declarations from cmd package */
extern char *cmdbuf, *atmbuf;           /* Command buffers */
#else
extern char cmdbuf[], atmbuf[];         /* Command buffers */
#endif /* DCMDBUF */

extern int nopush;

#ifndef NOSPL
int askflag = 0;                        /* ASK-class command active */
int echostars = 0;			/* ASKQ should echo asterisks */
extern char **a_ptr[];
extern int a_dim[];
extern char **m_xarg[];
extern int n_xarg[];
extern struct mtab *mactab;
extern int nmac;
extern long ck_alarm;
extern char alrm_date[], alrm_time[];
extern int x_ifnum;
#endif /* NOSPL */

extern int inserver;                    /* I am IKSD */
extern int backgrd;                     /* Kermit executing in background */
extern char psave[];                    /* For saving & restoring prompt */
extern char *tp;                        /* Temporary buffer */

int readblock = 4096;                   /* READ buffer size */
CHAR * readbuf = NULL;                  /* Pointer to read buffer */
int readsize = 0;                       /* Number of chars actually read */
int getcmd = 0;                         /* GET-class command was given */

extern int zchkod, zchkid;

/*  C K U U S 6 --  "User Interface" for Unix Kermit (Part 6)  */

struct keytab deltab[] = {              /* DELETE Command Options */
    { "/all",           DEL_ALL,  CM_INV },
    { "/after",         DEL_AFT,  CM_ARG },
    { "/ask",           DEL_ASK,  0 },
    { "/before",        DEL_BEF,  CM_ARG },
    { "/directories",   DEL_DIR,  0 },
    { "/dotfiles",      DEL_DOT,  0 },
    { "/except",        DEL_EXC,  CM_ARG },
    { "/heading",       DEL_HDG,  0 },
    { "/l",             DEL_LIS,  CM_INV|CM_ABR },
    { "/larger-than",   DEL_LAR,  CM_ARG },
    { "/list",          DEL_LIS,  0 },
    { "/log",           DEL_LIS,  CM_INV },
    { "/noask",         DEL_NAS,  0 },
    { "/nodotfiles",    DEL_NOD,  0 },
    { "/noheading",     DEL_NOH,  0 },
    { "/nol",           DEL_NOL,  CM_INV|CM_ABR },
    { "/nolist",        DEL_NOL,  0 },
    { "/nolog",         DEL_NOL,  CM_INV },
    { "/nopage",        DEL_NOP,  0 },
    { "/not-after",     DEL_NAF,  CM_ARG },
    { "/not-before",    DEL_NBF,  CM_ARG },
    { "/not-since",     DEL_NAF,  CM_INV|CM_ARG },
    { "/page",          DEL_PAG,  0 },
    { "/quiet",         DEL_QUI,  CM_INV },
    { "/recursive",     DEL_REC,  0 },
    { "/simulate",      DEL_SIM,  0 },
    { "/since",         DEL_AFT,  CM_ARG|CM_INV },
    { "/smaller-than",  DEL_SMA,  CM_ARG },
    { "/summary",       DEL_SUM,  0 },
    { "/tree",          DEL_ALL,  0 },
    { "/type",          DEL_TYP,  CM_ARG },
    { "/verbose",       DEL_VRB,  CM_INV }
};
int ndeltab = sizeof(deltab)/sizeof(struct keytab);

/* /QUIET-/VERBOSE (/LIST-/NOLIST) (/LOG-/NOLOG) table */

struct keytab qvswtab[] = {
    { "/l",           DEL_LIS,  CM_INV|CM_ABR },
    { "/list",        DEL_LIS,  0 },
    { "/log",         DEL_LIS,  CM_INV },
    { "/nol",         DEL_NOL,  CM_INV|CM_ABR },
    { "/nolist",      DEL_NOL,  0 },
    { "/nolog",       DEL_NOL,  CM_INV },
    { "/quiet",       DEL_QUI,  CM_INV },
    { "/verbose",     DEL_VRB,  CM_INV }
};
int nqvswtab = sizeof(qvswtab)/sizeof(struct keytab);

static struct keytab renamsw[] = {
    { "/collision",   REN_OVW,  CM_ARG },
#ifndef NOUNICODE
    { "/convert",     REN_XLA,  CM_ARG },
#endif	/* NOUNICODE */
    { "/fixspaces",   REN_SPA,  CM_ARG },
    { "/l",           DEL_LIS,  CM_INV|CM_ABR },
    { "/list",        DEL_LIS,  0      },
    { "/log",         DEL_LIS,  CM_INV },
    { "/lower",       REN_LOW,  CM_ARG },
    { "/nol",         DEL_NOL,  CM_INV|CM_ABR },
    { "/nolist",      DEL_NOL,  0      },
    { "/nolog",       DEL_NOL,  CM_INV },
    { "/quiet",       DEL_QUI,  CM_INV },
    { "/replace",     REN_RPL,  CM_ARG },
    { "/simulate",    DEL_SIM,  0      },
    { "/upper",       REN_UPP,  CM_ARG },
    { "/verbose",     DEL_VRB,  CM_INV }
};
static int nrenamsw = sizeof(renamsw)/sizeof(struct keytab);

static struct keytab renamset[] = {
    { "collision",    REN_OVW,  0 },
    { "list",         DEL_LIS,  0 }
};
static int nrenamset = sizeof(renamset)/sizeof(struct keytab);

/* Args for RENAME /LOWER: and /UPPER: */

static struct keytab r_upper[] = {
    { "all",   1, 0 },
    { "lower", 0, 0 }
};

static struct keytab r_lower[] = {
    { "all",   1, 0 },
    { "upper", 0, 0 }
};

/* Args for RENAME /COLLISION... */

#define RENX_FAIL 0
#define RENX_OVWR 1
#define RENX_SKIP 2

static struct keytab r_collision[] = {
    { "fail",      RENX_FAIL, 0 },
    { "overwrite", RENX_OVWR, 0 },
    { "proceed",   RENX_SKIP, CM_INV },
    { "skip",      RENX_SKIP, 0 }
};
static int nr_collision = sizeof(r_collision)/sizeof(struct keytab);

struct keytab copytab[] = {
    { "/append",      998,      0 },
#ifndef NOSPL
    { "/fromb64",     997,      0 },
#endif /* NOSPL */
    { "/l",           DEL_LIS,  CM_INV|CM_ABR },
    { "/list",        DEL_LIS,  0 },
    { "/log",         DEL_LIS,  CM_INV },
    { "/nol",         DEL_NOL,  CM_INV|CM_ABR },
    { "/nolist",      DEL_NOL,  0 },
    { "/nolog",       DEL_NOL,  CM_INV },
    { "/overwrite",   994,      CM_ARG },
#ifndef NOXFER
    { "/preserve",    995,      0 },
#endif	/* NOXFER */
    { "/quiet",       DEL_QUI,  CM_INV },
    { "/swap-bytes",  999,      0 },
#ifndef NOSPL
    { "/tob64",       996,      0 },
#endif /* NOSPL */
    { "/verbose",     DEL_VRB,  CM_INV }
};
int ncopytab = sizeof(copytab)/sizeof(struct keytab);

#define OVW_ALWAYS 0
#define OVW_NEVER  1
#define OVW_OLDER  2
#define OVW_NEWER  3

static struct keytab ovwtab[] = {
    { "always", OVW_ALWAYS, 0 },
    { "never",  OVW_NEVER, 0 },
    { "newer",  OVW_NEWER, 0 },
    { "older",  OVW_OLDER, 0 }
};
static int novwtab = 4;

#ifndef NOXFER
static struct keytab gettab[] = {       /* GET options */
    { "/as-name",         SND_ASN, CM_ARG },
    { "/binary",          SND_BIN, 0 },
#ifdef CALIBRATE
    { "/calibrate",       SND_CAL, CM_INV },
#endif /* CALIBRATE */
#ifdef PIPESEND
    { "/command",         SND_CMD, CM_PSH },
#endif /* PIPESEND */
    { "/delete",          SND_DEL, 0 },
    { "/except",          SND_EXC, CM_ARG },
    { "/filenames",       SND_NAM, CM_ARG },
#ifdef PIPESEND
    { "/filter",          SND_FLT, CM_ARG|CM_PSH },
#endif /* PIPESEND */
#ifdef VMS
    { "/image",           SND_IMG, 0 },
    { "/labeled",         SND_LBL, 0 },
#else
    { "/image",           SND_BIN, CM_INV },
#endif /* VMS */
#ifdef CK_TMPDIR
    { "/move-to",         SND_MOV, CM_ARG },
#endif /* CK_TMPDIR */
    { "/pathnames",       SND_PTH, CM_ARG },
    { "/pipes",           SND_PIP, CM_ARG|CM_PSH },
    { "/quiet",           SND_SHH, 0 },
#ifdef CK_RESEND
    { "/recover",         SND_RES, 0 },
#endif /* CK_RESEND */
    { "/recursive",       SND_REC, 0 },
    { "/rename-to",       SND_REN, CM_ARG },
#ifdef COMMENT
    { "/smaller-than",    SND_SMA, CM_ARG },
#endif /* COMMENT */
    { "/subdirectories",  SND_REC, CM_INV },
    { "/text",            SND_TXT, 0 },
    { "/transparent",     SND_XPA, 0 }
};
#define NGETTAB sizeof(gettab)/sizeof(struct keytab)
static int ngettab = NGETTAB;

static struct keytab rcvtab[] = {       /* RECEIVE options */
    { "/as-name",         SND_ASN, CM_ARG },
    { "/binary",          SND_BIN, 0 },
#ifdef CALIBRATE
    { "/calibrate",       SND_CAL, CM_INV },
#endif /* CALIBRATE */
#ifdef PIPESEND
    { "/command",         SND_CMD, CM_PSH },
#endif /* PIPESEND */
    { "/except",          SND_EXC, CM_ARG },
    { "/filenames",       SND_NAM, CM_ARG },
#ifdef PIPESEND
    { "/filter",          SND_FLT, CM_ARG|CM_PSH },
#endif /* PIPESEND */
#ifdef VMS
    { "/image",           SND_IMG, 0 },
    { "/labeled",         SND_LBL, 0 },
#else
    { "/image",           SND_BIN, CM_INV },
#endif /* VMS */
#ifdef CK_TMPDIR
    { "/move-to",         SND_MOV, CM_ARG },
#endif /* CK_TMPDIR */
    { "/pathnames",       SND_PTH, CM_ARG },
    { "/pipes",           SND_PIP, CM_ARG|CM_PSH },
#ifdef CK_XYZ
    { "/protocol",        SND_PRO, CM_ARG },
#else
    { "/protocol",        SND_PRO, CM_ARG|CM_INV },
#endif /* CK_XYZ */
    { "/quiet",           SND_SHH, 0 },
    { "/recursive",       SND_REC, 0 },
    { "/rename-to",       SND_REN, CM_ARG },
    { "/text",            SND_TXT, 0 },
    { "/transparent",     SND_XPA, 0 }
};
#define NRCVTAB sizeof(rcvtab)/sizeof(struct keytab)
static int nrcvtab = NRCVTAB;
#endif /* NOXFER */

/* WAIT table */

#define WAIT_FIL 997
#define WAIT_MDM 998

struct keytab waittab[] = {
    { "cd",            BM_DCD,   CM_INV }, /* (Carrier Detect) */
    { "cts",           BM_CTS,   CM_INV }, /* (Clear To Send)  */
    { "dsr",           BM_DSR,   CM_INV }, /* (Data Set Ready) */
    { "file",          WAIT_FIL, 0 },      /* New category selector keywords */
    { "modem-signals", WAIT_MDM, 0 },      /* ... */
    { "ri",            BM_RNG,   CM_INV }  /* (Ring Indicator) */
};
int nwaittab = (sizeof(waittab) / sizeof(struct keytab));

/* Modem signal table */

struct keytab mstab[] = {
    { "cd",    BM_DCD, 0 },             /* Carrier Detect */
    { "cts",   BM_CTS, 0 },             /* Clear To Send  */
    { "dsr",   BM_DSR, 0 },             /* Data Set Ready */
    { "ri",    BM_RNG, 0 }              /* Ring Indicator */
};
int nms = (sizeof(mstab) / sizeof(struct keytab));

#define WF_MOD 1
#define WF_DEL 2
#define WF_CRE 3

struct keytab wfswi[] = {               /* WAIT FILE switches */
    { "creation",     WF_CRE, 0 },      /* Wait for file to be created */
    { "deletion",     WF_DEL, 0 },      /* Wait for file to be deleted */
    { "modification", WF_MOD, 0 }       /* Wait for file to be modified */
};
int nwfswi = (sizeof(wfswi) / sizeof(struct keytab));

#ifndef NOSPL
struct keytab asgtab[] = {              /* Assignment operators for "." */
    { "::=", 2, 0 },                    /* ASSIGN and EVALUATE */
    { ":=",  1, 0 },                    /* ASSIGN */
    { "=",   0, 0 }                     /* DEFINE */
};
int nasgtab = (sizeof(asgtab) / sizeof(struct keytab));

struct keytab opntab[] = {
#ifndef NOPUSH
    { "!read",  OPN_PI_R, CM_INV },
    { "!write", OPN_PI_W, CM_INV },
#endif /* NOPUSH */
    { "append", OPN_FI_A, 0 },
    { "host",   OPN_NET,  0 },
#ifdef OS2
    { "line",   OPN_SER,  CM_INV },
    { "port",   OPN_SER,  0 },
#else
    { "line",   OPN_SER,  0 },
    { "port",   OPN_SER,  CM_INV },
#endif /* OS2 */
    { "read",   OPN_FI_R, 0 },
    { "write",  OPN_FI_W, 0 }
};
int nopn = (sizeof(opntab) / sizeof(struct keytab));

/* IF conditions */

#define  XXIFCO 0       /* IF COUNT */
#define  XXIFER 1       /* IF ERRORLEVEL */
#define  XXIFEX 2       /* IF EXIST */
#define  XXIFFA 3       /* IF FAILURE */
#define  XXIFSU 4       /* IF SUCCESS */
#define  XXIFNO 5       /* IF NOT */
#define  XXIFDE 6       /* IF DEFINED */
#define  XXIFEQ 7       /* IF EQUAL (strings) */
#define  XXIFAE 8       /* IF = (numbers) */
#define  XXIFLT 9       /* IF < (numbers) */
#define  XXIFGT 10      /* IF > (numbers) */
#define  XXIFLL 11      /* IF Lexically Less Than (strings) */
#define  XXIFLG 12      /* IF Lexically Greater Than (strings) */
#define  XXIFEO 13      /* IF EOF (READ file) */
#define  XXIFBG 14      /* IF BACKGROUND */
#define  XXIFNU 15      /* IF NUMERIC */
#define  XXIFFG 16      /* IF FOREGROUND */
#define  XXIFDI 17      /* IF DIRECTORY */
#define  XXIFNE 18      /* IF NEWER */
#define  XXIFRO 19      /* IF REMOTE-ONLY */
#define  XXIFAL 20      /* IF ALARM */
#define  XXIFSD 21      /* IF STARTED-FROM-DIALER */
#define  XXIFTR 22      /* IF TRUE */
#define  XXIFNT 23      /* IF FALSE */
#define  XXIFTM 24      /* IF TERMINAL-MACRO */
#define  XXIFEM 25      /* IF EMULATION */
#define  XXIFOP 26      /* IF OPEN */
#define  XXIFLE 27      /* IF <= */
#define  XXIFGE 28      /* IF >= */
#define  XXIFIP 29      /* IF INPATH */
#define  XXIFTA 30      /* IF TAPI */
#define  XXIFMA 31      /* IF MATCH */
#define  XXIFFL 32      /* IF FLAG */
#define  XXIFAB 33      /* IF ABSOLUTE */
#define  XXIFAV 34      /* IF AVAILABLE */
#define  XXIFAT 35      /* IF ASKTIMEOUT */
#define  XXIFRD 36      /* IF READABLE */
#define  XXIFWR 37      /* IF WRITEABLE */
#define  XXIFAN 38      /* IF ... AND ... */
#define  XXIFOR 39      /* IF ... OR ... */
#define  XXIFLP 40      /* IF left parenthesis */
#define  XXIFRP 41      /* IF right parenthesis */
#define  XXIFNQ 42      /* IF != (== "NOT =") */
#define  XXIFQU 43      /* IF QUIET */
#define  XXIFCK 44      /* IF C-KERMIT */
#define  XXIFK9 45      /* IF K-95 */
#define  XXIFMS 46      /* IF MS-KERMIT */
#define  XXIFWI 47      /* IF WILD */
#define  XXIFLO 48      /* IF LOCAL */
#define  XXIFCM 49      /* IF COMMAND */
#define  XXIFFP 50      /* IF FLOAT */
#define  XXIFIK 51      /* IF IKS */
#define  XXIFKB 52      /* IF KBHIT */
#define  XXIFKG 53      /* IF KERBANG */
#define  XXIFVE 54      /* IF VERSION */
#define  XXIFDC 55      /* IF DECLARED */
#define  XXIFGU 56      /* IF GUI */
#define  XXIFLN 57	/* IF LINK */
#define  XXIFDB 58	/* IF DEBUG */

struct keytab iftab[] = {               /* IF commands */
    { "!",          XXIFNO, 0 },
    { "!=",         XXIFNQ, 0 },
    { "&&",         XXIFAN, 0 },
    { "(",          XXIFLP, 0 },
    { ")",          XXIFRP, 0 },
    { "<",          XXIFLT, 0 },
    { "<=",         XXIFLE, 0 },
    { "=",          XXIFAE, 0 },
    { "==",         XXIFAE, CM_INV },
    { ">",          XXIFGT, 0 },
    { ">=",         XXIFGE, 0 },
    { "absolute",   XXIFAB, 0 },
    { "alarm",      XXIFAL, 0 },
    { "and",        XXIFAN, 0 },
    { "asktimeout", XXIFAT, 0 },
    { "available",  XXIFAV, 0 },
    { "background", XXIFBG, 0 },
    { "c-kermit",   XXIFCK, 0 },
    { "command",    XXIFCM, 0 },
    { "count",      XXIFCO, 0 },
    { "dcl",        XXIFDC, CM_INV },
    { "debug",      XXIFDB, 0 },
    { "declared",   XXIFDC, 0 },
    { "defined",    XXIFDE, 0 },
#ifdef CK_TMPDIR
    { "directory",  XXIFDI, 0 },
#endif /* CK_TMPDIR */
    { "emulation",  XXIFEM, 0 },
#ifdef COMMENT
    { "eof",        XXIFEO, 0 },
#endif /* COMMENT */
    { "equal",      XXIFEQ, 0 },
    { "error",      XXIFFA, CM_INV },
    { "exist",      XXIFEX, 0 },
    { "failure",    XXIFFA, 0 },
    { "false",      XXIFNT, 0 },
    { "flag",       XXIFFL, 0 },
#ifdef CKFLOAT
    { "float",      XXIFFP, 0 },
#endif /* CKFLOAT */
    { "foreground", XXIFFG, 0 },
#ifdef OS2
    { "gui",        XXIFGU, 0 },
#else
    { "gui",        XXIFGU, CM_INV },
#endif /* OS2 */
#ifdef IKSD
    { "iksd",       XXIFIK, 0 },
#else
    { "iksd",       XXIFIK, CM_INV },
#endif /* IKSD */
    { "integer",    XXIFNU, CM_INV },
    { "k-95",       XXIFK9, 0 },
    { "kbhit",      XXIFKB, 0 },
#ifdef UNIX
    { "kerbang",    XXIFKG, 0 },
#else
    { "kerbang",    XXIFKG, CM_INV },
#endif /* UNIX */
    { "lgt",        XXIFLG, 0 },
#ifdef UNIX
    { "link",       XXIFLN, 0 },
#endif /* UNIX */
    { "llt",        XXIFLL, 0 },
    { "local",      XXIFLO, 0 },
    { "match",      XXIFMA, 0 },
    { "ms-kermit",  XXIFMS, CM_INV },
#ifdef ZFCDAT
    { "newer",      XXIFNE, 0 },
#endif /* ZFCDAT */
    { "not",        XXIFNO, 0 },
    { "numeric",    XXIFNU, 0 },
    { "ok",         XXIFSU, CM_INV },
    { "open",       XXIFOP, 0 },
    { "or",         XXIFOR, 0 },
    { "quiet",      XXIFQU, 0 },
    { "readable",   XXIFRD, 0 },
    { "remote-only",XXIFRO, 0 },
    { "started-from-dialer",XXIFSD, CM_INV },
    { "success",    XXIFSU, 0 },
    { "tapi",       XXIFTA, 0 },
#ifdef OS2
    { "terminal-macro", XXIFTM, 0 },
#else
    { "terminal-macro", XXIFTM, CM_INV },
#endif /* OS2 */
    { "true",       XXIFTR, 0 },
    { "version",    XXIFVE, 0 },
    { "wild",       XXIFWI, 0 },
    { "writeable",  XXIFWR, 0 },
    { "||",         XXIFOR, 0 },
    { "", 0, 0 }
};
int nif = (sizeof(iftab) / sizeof(struct keytab)) - 1;

struct keytab iotab[] = {               /* Keywords for IF OPEN */
    { "!read-file",      ZRFILE, CM_INV },
    { "!write-file",     ZWFILE, CM_INV },
    { "append-file",     ZWFILE, CM_INV },
    { "connection",      8888,   0 },
#ifdef CKLOGDIAL
    { "cx-log",          7777,   0 },
#endif /* CKLOGDIAL */
    { "debug-log",       ZDFILE, 0 },
    { "error",           9999,   0 },
    { "packet-log",      ZPFILE, 0 },
    { "read-file",       ZRFILE, 0 },
    { "screen",          ZSTDIO, 0 },
    { "session-log",     ZSFILE, 0 },
    { "transaction-log", ZTFILE, 0 },
    { "write-file",      ZWFILE, 0 }
};
int niot = (sizeof(iotab) / sizeof(struct keytab));
#endif /* NOSPL */

/* Variables and prototypes */

_PROTOTYP(static int doymdir,(int));
_PROTOTYP(static int renameone,(char *,char *,
				int,int,int,int,int,int,int,int,int,int,int));

#ifdef NETCONN
extern int nnetdir;                     /* How many network directories */
#endif /* NETCONN */
#ifdef CK_SECURITY
_PROTOTYP(int ck_krb4_is_installed,(void));
_PROTOTYP(int ck_krb5_is_installed,(void));
_PROTOTYP(int ck_ntlm_is_installed,(void));
_PROTOTYP(int ck_srp_is_installed,(void));
_PROTOTYP(int ck_ssleay_is_installed,(void));
_PROTOTYP(int ck_ssh_is_installed,(void));
_PROTOTYP(int ck_crypt_is_installed,(void));
#else
#define ck_krb4_is_installed() (0)
#define ck_krb5_is_installed() (0)
#define ck_ntlm_is_installed() (0)
#define ck_srp_is_installed() (0)
#define ck_ssleay_is_installed() (0)
#define ck_ssh_is_installed() (0)
#define ck_crypt_is_installed() (0)
#endif /* CK_SECURITY */

#define AV_KRB4   1
#define AV_KRB5   2
#define AV_NTLM   3
#define AV_SRP    4
#define AV_SSL    5
#define AV_CRYPTO 6
#define AV_SSH    7

struct keytab availtab[] = {             /* Available authentication types */
    { "crypto",     AV_CRYPTO, CM_INV }, /* and encryption */
    { "encryption", AV_CRYPTO, 0 },
    { "k4",         AV_KRB4,   CM_INV },
    { "k5",         AV_KRB5,   CM_INV },
    { "kerberos4",  AV_KRB4,   0 },
    { "kerberos5",  AV_KRB5,   0 },
    { "krb4",       AV_KRB4,   CM_INV },
    { "krb5",       AV_KRB5,   CM_INV },
    { "ntlm",       AV_NTLM,   0 },
    { "srp",        AV_SRP,    0 },
    { "ssh",        AV_SSH,    0 },
    { "ssl",        AV_SSL,    0 },
    { "tls",        AV_SSL,    0 },
    { "",           0,         0 }
};
int availtabn = sizeof(availtab)/sizeof(struct keytab)-1;

#ifndef NODIAL
_PROTOTYP(static int ddcvt, (char *, FILE *, int) );
_PROTOTYP(static int dncvt, (int, int, int, int) );
_PROTOTYP(char * getdname, (void) );

static int partial  = 0;                /* For partial dial */
static char *dscopy = NULL;
int dialtype = -1;

char *dialnum = (char *)0;              /* Remember DIAL number for REDIAL */
int dirline = 0;                        /* Dial directory line number */
extern char * dialdir[];                /* Dial directory file names */
extern int dialdpy;                     /* DIAL DISPLAY on/off */
extern int ndialdir;                    /* How many dial directories */
extern int ntollfree;                   /* Toll-free call info */
extern int ndialpxx;                    /* List of PBX exchanges */
extern char *dialtfc[];
char * matchpxx = NULL;                 /* PBX exchange that matched */
extern int nlocalac;                    /* Local area-code list */
extern char * diallcac[];
extern int tttapi;
#ifdef CK_TAPI
extern int tapiconv;                    /* TAPI Conversions */
extern int tapipass;                    /* TAPI Passthrough */
#endif /* CK_TAPI */
extern int dialatmo;
extern char * dialnpr, * dialsfx;
extern char * dialixp, * dialixs, * dialmac;
extern char * dialldp, * diallds, * dialtfp;
extern char * dialpxi, * dialpxo, * diallac;
extern char * diallcp, * diallcs, * diallcc;
extern char * dialpxx[];

extern int dialcnf;                     /* DIAL CONFIRMATION */
int dialfld = 0;                        /* DIAL FORCE-LONG-DISTANCE */
int dialsrt = 1;                        /* DIAL SORT ON */
int dialrstr = 6;                       /* DIAL RESTRICTION */
int dialtest = 0;                       /* DIAL TEST */
int dialcount = 0;                      /* \v(dialcount) */

extern int dialsta;                     /* Dial status */
int dialrtr = -1,                       /* Dial retries */
    dialint = 10;                       /* Dial retry interval */
extern long dialcapas;                  /* Modem capabilities */
extern int dialcvt;                     /* DIAL CONVERT-DIRECTORY */
#endif /* NODIAL */

#ifndef NOSPL
#define IFCONDLEN 256
int ifc,                                /* IF case */
    not = 0,                            /* Flag for IF NOT */
    ifargs = 0;                         /* Count of IF condition words */
char ifcond[IFCONDLEN];                 /* IF condition text */
char *ifcp;                             /* Pointer to IF condition text */
extern int vareval;
#ifdef DCMDBUF
extern int
 *ifcmd,  *count,  *iftest, *intime,
    *inpcas, *takerr, *merror, *xquiet, *xvarev;
#else
extern int ifcmd[];                     /* Last command was IF */
extern int iftest[];                    /* Last IF was true */
extern int count[];                     /* For IF COUNT, one for each cmdlvl */
extern int intime[];                    /* Ditto for other stackables... */
extern int inpcas[];
extern int takerr[];
extern int merror[];
extern int xquiet[];
extern int xvarev[];
#endif /* DCMDBUF */
#else
extern int takerr[];
#endif /* NOSPL */

#ifdef DCMDBUF
extern char *line;                      /* Character buffer for anything */
extern char *tmpbuf;
#else
extern char line[], tmpbuf[];
#endif /* DCMDBUF */
extern char *lp;                        /* Pointer to line buffer */

int cwdf = 0;                           /* CWD has been done */

/* Flags for ENABLE/DISABLE */
extern int en_cwd, en_cpy, en_del, en_dir, en_fin,
   en_get, en_hos, en_ren, en_sen, en_set, en_spa, en_typ, en_who, en_bye,
   en_asg, en_que, en_ret, en_mai, en_pri, en_mkd, en_rmd, en_xit, en_ena;

extern FILE *tfile[];                   /* File pointers for TAKE command */
extern char *tfnam[];                   /* Names of TAKE files */
extern int tfline[];                    /* TAKE-file line number */

extern int success;                     /* Command success/failure flag */
extern int cmdlvl;                      /* Current position in command stack */

#ifndef NOSPL
extern int maclvl;                      /* Macro to execute */
extern char *macx[];                    /* Index of current macro */
extern char *mrval[];                   /* Macro return value */
extern char *macp[];                    /* Pointer to macro */
extern int macargc[];                   /* ARGC from macro invocation */

#ifdef COMMENT
extern char *m_line[];
#endif /* COMMENT */

extern char *m_arg[MACLEVEL][NARGS];    /* Stack of macro arguments */
extern char *g_var[];                   /* Global variables %a, %b, etc */

#ifdef DCMDBUF
extern struct cmdptr *cmdstk;           /* The command stack itself */
#else
extern struct cmdptr cmdstk[];          /* The command stack itself */
#endif /* DCMDBUF */
#endif /* NOSPL */

#define xsystem(s) zsyscmd(s)

static int x, y, z = 0;
static char *s, *p;

#ifdef OS2
_PROTOTYP( int os2settitle, (char *, int) );
#endif /* OS2 */

extern struct keytab yesno[], onoff[], fntab[];
extern int nyesno, nfntab;

#ifndef NOSPL

/* Do the ASK, ASKQ, GETOK, and READ commands */

int asktimedout = 0;

#define ASK_PUP 1
#define ASK_TMO 2
#define ASK_GUI 3
#define ASK_QUI 4
#define ASK_DEF 5
#define ASK_ECH 6

static struct keytab asktab[] = {
    {  "/default", ASK_DEF, CM_ARG },
    {  "/gui",     ASK_GUI,      
#ifdef KUI
           0
#else /* KUI */
           CM_INV
#endif /* KUI */
    },
    { "/popup",    ASK_PUP,   
#ifdef OS2
           0
#else /* OS2 */
           CM_INV
#endif /* OS2 */
    },
    { "/quiet",    ASK_QUI, 0 },
    { "/timeout",  ASK_TMO, CM_ARG },
    { "", 0, 0 }
};
static int nasktab = sizeof(asktab)/sizeof(struct keytab)-1;

static struct keytab askqtab[] = {
    { "/default",  ASK_DEF, CM_ARG },
    { "/echo",     ASK_ECH, CM_ARG },
    { "/gui",      ASK_GUI,      
#ifdef KUI
           0
#else /* KUI */
           CM_INV
#endif /* KUI */
    },
    { "/noecho",   ASK_QUI, CM_INV },
    { "/popup",    ASK_PUP,   
#ifdef OS2
           0
#else /* OS2 */
           CM_INV
#endif /* OS2 */
    },
    { "/quiet",    ASK_QUI, 0 },
    { "/timeout",  ASK_TMO, CM_ARG },
    { "", 0, 0 }
};
static int naskqtab = sizeof(askqtab)/sizeof(struct keytab)-1;

int
doask(cx) int cx; {
    extern int asktimer, timelimit;
#ifdef CK_RECALL
    extern int on_recall;
#endif /* CK_RECALL */
    int echochar = 0;
    int popupflg = 0;
    int guiflg = 0;
    int nomsg = 0;
    int mytimer = 0;
#ifdef CK_APC
    extern int apcactive, apcstatus;
#endif /* CK_APC */

    char dfbuf[1024];			/* Buffer for default answer */
    char * dfanswer = NULL;		/* Pointer to it */

    char vnambuf[VNAML+1];              /* Buffer for variable names */
    char *vnp = NULL;                   /* Pointer to same */
    
    dfbuf[0] = NUL;
    vnambuf[0] = NUL;

#ifdef CK_APC
    if ( apcactive != APC_INACTIVE && (apcstatus & APC_NOINP) ) {
        return(success = 0);
    }
#endif /* CK_APC */

    mytimer = asktimer;                 /* Inherit global ASK timer */
    echostars = 0;			/* For ASKQ */

    if (cx == XXASK || cx == XXASKQ) {
        struct FDB sw, fl;
        int getval;
        char c;
        if (cx == XXASKQ)               /* Don't log ASKQ response */
          debok = 0;
        cmfdbi(&sw,                     /* First FDB - command switches */
               _CMKEY,                  /* fcode */
               "Variable name or switch",
               "",                      /* default */
               "",                      /* addtl string data */
	       ((cx == XXASK) ? nasktab : naskqtab), /* Table size */
               4,                       /* addtl numeric data 2: 4 = cmswi */
               xxstring,                /* Processing function */
	       ((cx == XXASK) ? asktab : askqtab), /* Keyword table */
               &fl                      /* Pointer to next FDB */
               );
        cmfdbi(&fl,                     /* Anything that doesn't match */
               _CMFLD,                  /* fcode */
               "",                      /* hlpmsg */
               "",                      /* default */
               "",                      /* addtl string data */
               0,                       /* addtl numeric data 1 */
               0,                       /* addtl numeric data 2 */
               NULL,
               NULL,
               NULL
               );
        while (1) {                     /* Parse 0 or more switches */
            x = cmfdb(&sw);             /* Parse something */
            if (x < 0)
              return(x);
            if (cmresult.fcode != _CMKEY) /* Break out if not a switch */
              break;
            c = cmgbrk();
            if ((getval = (c == ':' || c == '=')) && !(cmgkwflgs() & CM_ARG)) {
                printf("?This switch does not take an argument\n");
                return(-9);
            }
            if (!getval && (cmgkwflgs() & CM_ARG)) {
                printf("?This switch requires an argument\n");
                return(-9);
            }
            switch (cmresult.nresult) {
	      case ASK_QUI:
		nomsg = 1;
		if (cx == XXASKQ)
		  echostars = 0;
		break;
              case ASK_PUP:
                popupflg = 1;
                break;
	      case ASK_GUI:
		guiflg = 1;
		break;
              case ASK_TMO: {
                  if ((y = cmnum("seconds","1",10,&x,xxstring)) < 0)
                    return(y);
                  if (x < 0)
                    x = 0;
                  mytimer = x;
                  break;
              }
              case ASK_ECH: {
                  if ((y = cmfld("Character to echo","*",&s,xxstring)) < 0)
                    return(y);
		  echochar = *s;
                  break;
              }
              case ASK_DEF: {
                  if ((y = cmfld("Text to supply if reply is empty",
				 "",&s,xxstring)) < 0)
                    return(y);
		  ckstrncpy(dfbuf,s,1024);
		  dfanswer = dfbuf;
                  break;
              }
              default: return(-2);
            }
        }
        /* Have variable name, make copy. */
        ckstrncpy(vnambuf,cmresult.sresult,VNAML);
        vnp = vnambuf;
        if (vnambuf[0] == CMDQ &&
            (vnambuf[1] == '%' || vnambuf[1] == '&'))
          vnp++;
        y = 0;
        if (*vnp == '%' || *vnp == '&') {
            if ((y = parsevar(vnp,&x,&z)) < 0)
	      return(y);
        }
    } else if (cx != XXGOK && cx != XXRDBL) { /* Get variable name */
        if ((y = cmfld("Variable name","",&s,NULL)) < 0) {
            if (y == -3) {
                printf("?Variable name required\n");
                return(-9);
            } else return(y);
        }
        ckstrncpy(vnambuf,s,VNAML);     /* Make a copy. */
        vnp = vnambuf;
        if (vnambuf[0] == CMDQ &&
            (vnambuf[1] == '%' || vnambuf[1] == '&'))
          vnp++;
        y = 0;
        if (*vnp == '%' || *vnp == '&') {
            if ((y = parsevar(vnp,&x,&z)) < 0)
              return(y);
        }
    }
    if (cx == XXREA || cx == XXRDBL) {  /* READ or READBLOCK command */
        if ((y = cmcfm()) < 0)          /* Get confirmation */
          return(y);
        if (chkfn(ZRFILE) < 1) {        /* File open? */
            printf("?Read file not open\n");
            return(success = 0);
        }
        if (!(s = (char *)readbuf)) {           /* Where to read into. */
            printf("?Oops, no READ buffer!\n");
            return(success = 0);
        }
        y = zsinl(ZRFILE, s, readblock); /* Read a line. */
        debug(F111,"read zsinl",s,y);
        if (y < 0) {                    /* On EOF or other error, */
            zclose(ZRFILE);             /* close the file, */
            delmac(vnp,0);              /* delete the variable, */
            return(success = 0);        /* and return failure. */
        } else {                        /* Read was OK. */
            readsize = (int) strlen(s);
            success = (addmac(vnp,s) < 0 ? 0 : 1); /* Define variable */
            debug(F111,"read addmac",vnp,success);
            return(success);            /* Return success. */
        }
    }

    /* ASK, ASKQ, GETOK, or GETC */

    if (cx == XXGOK) {			/* GETOK can take switches */
        struct FDB sw, fl;
        int getval;
        char c;
        cmfdbi(&sw,                     /* First FDB - command switches */
               _CMKEY,                  /* fcode */
               "Variable name or question prompt",
               "",                      /* default */
               "",                      /* addtl string data */
               nasktab,                 /* addtl numeric data 1: tbl size */
               4,                       /* addtl numeric data 2: 4 = cmswi */
               xxstring,                /* Processing function */
               asktab,                  /* Keyword table */
               &fl                      /* Pointer to next FDB */
               );
        cmfdbi(&fl,                     /* Anything that doesn't match */
               _CMTXT,                  /* fcode */
               "",                      /* hlpmsg */
               "",                      /* default */
               "",                      /* addtl string data */
               0,                       /* addtl numeric data 1 */
               0,                       /* addtl numeric data 2 */
               NULL,
               NULL,
               NULL
               );
        while (1) {                     /* Parse 0 or more switches */
            x = cmfdb(&sw);             /* Parse something */
            if (x < 0)
              return(x);
            if (cmresult.fcode != _CMKEY) /* Break out if not a switch */
              break;
            c = cmgbrk();
            if ((getval = (c == ':' || c == '=')) && !(cmgkwflgs() & CM_ARG)) {
                printf("?This switch does not take an argument\n");
                return(-9);
            }
            if (!getval && (cmgkwflgs() & CM_ARG)) {
                printf("?This switch requires an argument\n");
                return(-9);
            }
            switch (cmresult.nresult) {
              case ASK_PUP:
                popupflg = 1;
                break;
	      case ASK_GUI:
		guiflg = 1;
		break;
              case ASK_TMO: {
                  if ((y = cmnum("seconds","1",10,&x,xxstring)) < 0)
                    return(y);
                  if (x < 0)
                    x = 0;
                  mytimer = x;
                  break;
              }
              case ASK_DEF: {
                  if ((y = cmfld("Text to supply if reply is empty",
				 "",&s,xxstring)) < 0)
                    return(y);
		  ckstrncpy(dfbuf,s,1024);
		  dfanswer = dfbuf;
                  break;
              }
	      case ASK_QUI:
		nomsg = 1;
		break;
              default: return(-2);
            }
	}
	p = cmresult.sresult;
    } else
      if ((y = cmtxt(
"Prompt,\n\
 enclose in { braces } or \" quotes \" to preserve leading and trailing\n\
 spaces, precede question mark with backslash (\\).",
                   "",&p,xxstring)) < 0)
        return(y);

    if (!p) p = "";
#ifndef NOLOCAL
#ifdef OS2
    if (popupflg) {                     /* Popup requested */
        int len = -1;
        ckstrncpy(tmpbuf,brstrip(p),TMPBUFSIZ);
        p = tmpbuf;
        if (cx == XXASK || cx == XXASKQ) {
            if (cx == XXASK)
              len = popup_readtext(vmode,NULL,p,line,LINBUFSIZ,mytimer);
            else
              len = popup_readpass(vmode,NULL,p,line,LINBUFSIZ,mytimer);
            asktimedout = ( len < 0 && mytimer );
        } else if (cx == XXGOK) {
	    printf("?Sorry, GETOK /POPUP not implemented yet\n");
	    timelimit = 0;
	    return(-9);
	}
        if (len >= 0) {
	    y = addmac(vnp,(char *)line); /* Add it to the macro table. */
        } else if ( asktimedout && dfanswer ) {
            y = addmac(vnp,dfanswer);	    /* Add it to the macro table. */
            asktimedout = 0;
            len = 0;
        }
        timelimit = 0;
        return(success = ((len >= 0) && (y >= 0)) && !asktimedout);
    }
#ifdef KUI
    if (guiflg) {                       /* GUI requested */
        int rc, n;
	char * s1;
	s1 = tmpbuf;
	n = TMPBUFSIZ-1;
	zzstring(brstrip(p),&s1,&n);
        p = tmpbuf;
        if (cx == XXASK || cx == XXASKQ) {
            rc = gui_txt_dialog(NULL,p,(cx == XXASK),
                                line,LINBUFSIZ,dfanswer,mytimer);
            asktimedout = (rc == -1 && mytimer);
            if (rc == 1) {
                y = addmac(vnp,(char *)line); /* Add it to the macro table. */
            } else if ( asktimedout && dfanswer ) {
                y = addmac(vnp,dfanswer); /* Add default to macro table. */
                asktimedout = 0;
                rc = 1;
            }
	    timelimit = 0;
	    return(success = (rc == 1 && (y >= 0)) && !asktimedout);
	} else if (cx == XXGOK) {
	    int x;
	    x = lookup(yesno,dfanswer,nyesno,NULL);
	    if (x != 1) x = 2;
	    rc = uq_ok(NULL, p, 3, NULL, x);
	    return(success = (rc == 1));
	}
    }
#endif /* KUI */
#endif /* OS2 */
#endif /* NOLOCAL */

    concb((char)escape);                /* Enter CBREAK mode */
    cmsavp(psave,PROMPTL);              /* Save old prompt */
    cmsetp(brstrip(p));                 /* Make new prompt */

reprompt:
    if (cx == XXASKQ) {                 /* For ASKQ, */
        cmini(0);                       /* no-echo mode. */
	if (echochar)
	  echostars = echochar;
    } else {                            /* For others, regular echoing. */
        cmini(ckxech);
	echostars = 0;
    }
    askflag = 1;
    x = -1;                             /* This means to reparse. */
    cmflgs = 0;
    if (pflag)
      prompt(xxstring);                 /* Issue prompt. */

    asktimedout = 0;                    /* Handle timed responses. */
    timelimit = mytimer;
reparse:
    cmres();
    if (cx == XXGOK) {                  /* GETOK */
#ifdef CK_RECALL
        on_recall = 0;
#endif /* CK_RECALL */
        askflag = 0;
	/* GETOK uses keyword table */
        x = cmkey(yesno,nyesno,"",dfanswer,xxstring);
        if (x < 0) {                    /* Parse error */
            if (x == -10) {
		char * ds;
		ds = dfanswer ? dfanswer : "No";
		if (!nomsg)
		  printf("?Timed out, assuming \"%s\"",ds);
		printf("\n");
                asktimedout = 1;
		x = lookup(yesno,ds,nyesno,NULL);
		if (x != 1) x = 0;
                goto gokdone;
            } else if (x == -3) {       /* No answer? */
                printf("Please respond Yes or No\n"); /* Make them answer */
                cmini(ckxech);
                goto reprompt;
            } else if (x == -1) {
                goto reparse;
            } else
              goto reprompt;
        }
        if (cmcfm() < 0)                /* Get confirmation */
          goto reparse;
  gokdone:
        askflag = 0;
        cmsetp(psave);                  /* Restore prompt */
#ifdef VMS
        if (cmdlvl > 0)                 /* In VMS and not at top level, */
          conres();                     /*  restore console again. */
#endif /* VMS */
        timelimit = 0;
        return(x);                      /* Return success or failure */
    } else if (cx == XXGETC             /* GETC */
#ifdef OS2
               || cx == XXGETK          /* or GETKEYCODE */
#endif /* OS2 */
               ) {                      /* GETC */
        char tmp[16];
        conbin((char)escape);           /* Put keyboard in raw mode */
#ifndef NOSETKEY
#ifdef OS2
        if (cx == XXGETK) {             /* GETKEYCODE */
            extern int os2gks;
            int t;
            t = os2gks;                 /* Turn off kverb recognition */
            os2gks = 0;
            x = congks(timelimit);      /* Read a key event, blocking */
            os2gks = t;                 /* Put back kverb recognition */
        } else                          /* GETC */
#endif /* OS2 */
#endif /* NOSETKEY */
        {
            debug(F101,"GETC conchk","",conchk());
            x = coninc(timelimit);      /* Just read one character */
            debug(F101,"GETC coninc","",x);
        }
        concb((char)escape);            /* Put keyboard back in cbreak mode */
        if (x > -1) {
            if (xcmdsrc == 0)
              printf("\r\n");
#ifdef OS2
            if (cx == XXGETK) {         /* GETKEYCODE */
                sprintf(tmp,"%d",x);    /* SAFE */
            } else {
#endif /* OS2 */
                tmp[0] = (char) (x & 0xff);
                tmp[1] = NUL;
#ifdef OS2
            }
#endif /* OS2 */
            y = addmac(vnp,tmp);        /* Add it to the macro table. */
            debug(F111,"getc/getk addmac",vnp,y);
        } else y = -1;
        cmsetp(psave);                  /* Restore old prompt. */
        if (x < -1) {
            asktimedout = 1;
            if (!quiet && !nomsg)
              printf("?Timed out");
	    printf("\n");
        }
        timelimit = 0;
        return(success = ((y < 0 ? 0 : 1) && (asktimedout == 0)));
    } else {                            /* ASK or ASKQ */
#ifdef CK_RECALL
        on_recall = 0;			/* Don't put response in recall buf */
#endif /* CK_RECALL */
	askflag = 1;			/* ASK[Q] always goes to terminal */
        y = cmdgquo();                  /* Get current quoting */
        cmdsquo(0);                     /* Turn off quoting */
        while (x == -1) {               /* Prompt till they answer */
            x = cmtxt("Please respond.",dfanswer,&s,NULL);
            debug(F111,"ASK cmtxt",s,x);
            cmres();
        }
        cmdsquo(y);                     /* Restore previous quoting */
        if (cx == XXASKQ)               /* ASKQ must echo CRLF here */
          printf("\r\n");
	if (x == -10 && dfanswer) {	/* Don't fail on timeout if */
	    s = dfanswer;		/* a default was specified */
	    asktimedout = 0;		/* and don't fail */
	    x = 0;
	}
        if (x < 0) {                    /* If cmtxt parse error, */
            cmsetp(psave);              /* restore original prompt */
#ifdef VMS
            if (cmdlvl > 0)             /* In VMS and not at top level, */
              conres();                 /*  restore console again. */
#endif /* VMS */
            if (x == -10) {		/* Timed out with no response */
		if (!nomsg)
		  printf("?Timed out");
		printf("\n");
                asktimedout = 1;
		if (dfanswer)		/* Supply default answer if any */
		  s = dfanswer;
                success = x = 0;	/* (was "x = -9;") */
            }
            timelimit = 0;
            return(x);                  /* and return cmtxt's error code. */
        }
        if (!s || *s == NUL) {		/* If user typed a bare CR, */
            cmsetp(psave);              /* Restore old prompt, */
            delmac(vnp,0);              /* delete variable if it exists, */
#ifdef VMS
            if (cmdlvl > 0)             /* In VMS and not at top level, */
              conres();                 /*  restore console again. */
#endif /* VMS */
            timelimit = 0;
            return(success = 1);        /* and return. */
        }
        y = addmac(vnp,s);              /* Add it to the macro table. */
        debug(F111,"ask addmac",vnp,y);
        cmsetp(psave);                  /* Restore old prompt. */
#ifdef VMS
        if (cmdlvl > 0)                 /* In VMS and not at top level, */
          conres();                     /*  restore console again. */
#endif /* VMS */
        timelimit = 0;
        return(success = (y < 0 ? 0 : 1) && (asktimedout == 0));
    }
}
#endif /* NOSPL */

#ifndef NOSPL
int
doincr(cx) int cx; {                    /* INCREMENT, DECREMENT */
    char vnambuf[VNAML+1];              /* Buffer for variable names */
    int eval = 0;
    CK_OFF_T x;
    eval = (cx == XX_DECR || cx == XX_INCR);

    if ((y = cmfld("Variable name","",&s, eval ? xxstring : NULL)) < 0) {
        if (y == -3) {
            printf("?Variable name required\n");
            return(-9);
        } else return(y);
    }
    ckstrncpy(vnambuf,s,VNAML);
    if ((y = cmnumw("by amount","1",10,&x,xxstring)) < 0)
      return(y);
    if ((y = cmcfm()) < 0)
      return(y);

    z = (cx == XX_INCR || cx == XXINC) ? 1 : 0; /* Increment or decrement? */

    if (incvar(vnambuf,x,z) < 0) {
        printf("?Variable %s not defined or not numeric\n",vnambuf);
        return(success = 0);
    }
    return(success = 1);
}

/* Used by doundef() */
static int
xxundef(s,verbose,simulate) char * s; int verbose, simulate; {
    int rc = 0;
    if (!s) return(0);
    if (*s == CMDQ && *(s+1) == '%') {
        char c = *(s+2), * p = NULL;
        if (c >= '0' && c <= '9') {
            if (maclvl < 0)
              p = g_var[c];
            else
              p = m_arg[maclvl][c - '0'];
        } else {
            if (isupper(c)) c += ('a'-'A');
            if (c >= 'a' && c <= 'z')
              p = g_var[c];
        }
        if (!p) return(-1);
    }
    if (verbose)
      printf(" %s ",s);
    if (simulate) {
        printf("(SELECTED)\n");
    } else if ((x = delmac(s,1)) > -1) { /* Full name required */
        rc = 1;
        if (verbose) printf("(OK)\n");
    } else if (verbose)
      printf("(FAILED)\n");
    return(rc);
}

/* Do the (_)DEFINE, (_)ASSIGN, and UNDEFINE commands */

#define UND_MAT 1
#define UND_VRB 2
#define UND_EXC 3
#define UND_SIM 3

static struct keytab undefswi[] = {
    { "/list",     UND_VRB, 0 },
#ifdef COMMENT
    { "/except",   UND_EXC, CM_ARG },
#endif /* COMMENT */
    { "/matching", UND_MAT, 0 },
    { "/simulate", UND_SIM, 0 },
    { "/verbose",  UND_VRB, CM_INV }
};
static int nundefswi = sizeof(undefswi) / sizeof(struct keytab);

#define UNDEFMAX 64
static char ** undeflist = NULL;
int
doundef(cx) int cx; {                   /* UNDEF, _UNDEF */
    int i, j, n, rc = 0, arraymsg = 0;
    int domatch = 0, verbose = 0, errors = 0, simulate = 0, flag = 0;
    char *vnp, vnbuf[4];
#ifdef COMMENT
    char *except = NULL;
#endif /* COMMENT */
    struct FDB sw, fl;
    int getval;
    char c;

    if (!undeflist) {                   /* Allocate list if necessary */
        undeflist = (char **)malloc(UNDEFMAX * sizeof(char *));
        if (!undeflist) {
            printf("?Memory allocation failure\n");
            return(-9);
        }
        for (i = 0; i < UNDEFMAX; i++)
          undeflist[i] = NULL;
    }
    cmfdbi(&sw,                         /* First FDB - command switches */
           _CMKEY,                      /* fcode */
           "Variable name or switch",
           "",                          /* default */
           "",                          /* addtl string data */
           nundefswi,                   /* addtl numeric data 1: tbl size */
           4,                           /* addtl numeric data 2: 4 = cmswi */
           xxstring,                    /* Processing function */
           undefswi,                    /* Keyword table */
           &fl                          /* Pointer to next FDB */
           );
    cmfdbi(&fl,                         /* Anything that doesn't match */
           _CMFLD,                      /* fcode */
           "",                          /* hlpmsg */
           "",                          /* default */
           "",                          /* addtl string data */
           0,                           /* addtl numeric data 1 */
           0,                           /* addtl numeric data 2 */
           (cx == XXUNDEF) ? NULL : xxstring,
           NULL,
           NULL
           );
    while (1) {                         /* Parse 0 or more switches */
        x = cmfdb(&sw);                 /* Parse something */
        if (x < 0)
          return(x);
        if (cmresult.fcode != _CMKEY)   /* Break out if not a switch */
          break;
        c = cmgbrk();
        if ((getval = (c == ':' || c == '=')) && !(cmgkwflgs() & CM_ARG)) {
            printf("?This switch does not take an argument\n");
            return(-9);
        }
        switch (cmresult.nresult) {
          case UND_MAT: domatch  = 1; break;
          case UND_SIM: simulate = 1; /* fall thru on purpose */
          case UND_VRB: verbose  = 1; break;

#ifdef COMMENT
          case UND_EXC:
            if (!getval) break;
            if ((x = cmfld("Pattern","",&s,xxstring)) < 0) {
                if (x == -3) {
                    printf("?Pattern required\n");
                    x = -9;
                }
                goto xgetx;
            }
            makestr(&except,cmresult.sresult);
            break;
#endif /* COMMENT */

          default:
            return(-2);
        }
    }
    n = 0;
    makestr(&(undeflist[n++]),cmresult.sresult);
    for (i = 1; i < UNDEFMAX; i++) {
        x = cmfld("Macro or variable name","",&s,
                  ((cx == XXUNDEF) ? NULL : xxstring)
                  );
        if (x == -3) {
            if ((y = cmcfm()) < 0)
              return(y);
            break;
        } else if (y < 0) {
            return(y);
        }
        makestr(&(undeflist[n++]),s);
    }
    /* Now we have a list of n variables or patterns to undefine */

    for (i = 0; i < n; i++) {
        flag = 0;
        if (!(vnp = undeflist[i]))
          continue;
        if (vnp[0] == CMDQ && (vnp[1] == '%' || vnp[1] == '&')) {
            flag++;
            vnp++;
        }
        if (!domatch) {                 /* Pattern match not requested */
            if (flag) {
                if ((y = parsevar(vnp,&x,&z)) < 0) {
                    vnp--;
                    if (verbose) printf(" %s...error\n",vnp);
                    continue;
                }
                vnp--;
            }
            x = xxundef(vnp,verbose,simulate);
            if (x > -1) {
                if (!x && !simulate) errors++;
                rc += x;
            }
            continue;
        }
        /* Pattern match requested */

        if (!flag) {                    /* It's a macro */
            for (j = 0; j < nmac; j++) {
                if (ckmatch(vnp,mactab[j].kwd,0,1)) {
                    x = xxundef(mactab[j].kwd,verbose,simulate);
                    if (x > -1) {
                        rc += x;
                        if (!x) errors++;
                    }
                    if (!simulate)
                      j--;              /* Because mactab shifted up */
                }
            }
        } else if (vnp[0] == '%') {     /* It's a \%x variable */
            vnbuf[0] = CMDQ;
            vnbuf[1] = '%';
            vnbuf[3] = NUL;
            for (j = '0'; j <= 'z'; j++) { /* 0..9 a..z */
                vnbuf[2] = j;
                if (ckmatch(vnp,&vnbuf[1],0,1)) {
                    x = xxundef(vnbuf,verbose,simulate);
                    if (x > -1) {
                        if (!x) errors++;
                        rc += x;
                    }
                }
                if (j == '9') j = (int)'a' - 1; /* 9 -> a */
            }
        } else if (vnp[0] == '&') {
            if (!arraymsg && !quiet) {
                printf("?UNDEFINE /MATCH can't be used with arrays.\n");
                printf("(Type HELP ARRAY to see other methods.)\n");
            }
            arraymsg++;
            errors++;
        }
    }
    if (verbose)
      printf("undefined: %d, errors: %d\n",rc,errors);

    for (i = 0; i < UNDEFMAX; i++) {    /* Check them all */
        if (undeflist[i]) {             /* in case we were interrupted */
            free(undeflist[i]);         /* previously... */
            undeflist[i] = NULL;
        }
    }
    return(success = (errors == 0) ? 1 : 0);
}

int
dodef(cx) int cx; {
    extern int xxdot;
    extern char ppvnambuf[];
    int doeval = 0;
    char vnambuf[VNAML+1];              /* Buffer for variable names */
    char *vnp;                          /* Pointer to same */
    int k, mydot;
    mydot = xxdot;                      /* Copy */
    xxdot = 0;                          /* and reset */
/*
  In case we got here from a command that begins like ".\%a", cmkey() has
  already evaluated \%a, but we don't want that, so we retrieve the variable
  name from a special pre-evaluation buffer in the command module, and we
  undo the "unget word" that would be done because of the token, because if
  the variable was defined, it will unget its value rather than its name.
*/
    s = NULL;

    if (mydot && ppvnambuf[0] == '.' && ppvnambuf[1]) {
        s = ppvnambuf+1;
        unungw();
    }
    if (!s) {
        if (cx == XXDFX || cx == XXASX)
          /* Evaluate variable name */
          y = cmfld("Macro or variable name","",&s,xxstring);
        else
          /* Don't evaluate the variable name */
          y = cmfld("Macro or variable name","",&s,NULL);
        if (y < 0) {
            if (y == -3) {
                printf("?Variable name required\n");
                return(-9);
            } else return(y);
        }
    }
    k = strlen(s);
    if (k > VNAML) {
        printf("?Name too long: \"%s\"\n",s);
        return(-9);
    }
    ckstrncpy(vnambuf,s,VNAML);
    vnambuf[VNAML] = NUL;
    vnp = vnambuf;
    if (vnambuf[0] == CMDQ && (vnambuf[1] == '%' || vnambuf[1] == '&')) vnp++;
    if (*vnp == '%' || *vnp == '&') {
        if ((y = parsevar(vnp,&x,&z)) < 0) return(y);
#ifdef COMMENT
        if (cx == XXUNDEF) {            /* Undefine */
            if ((y = cmtxt("Text to be ignored","",&s,NULL)) < 0) return(y);
            delmac(vnp,0);
            return(success = 1);
        }
#endif /* COMMENT */
        debug(F101,"dodef parsevar x","",x);
        if (mydot) {
            if ((doeval = cmkey(asgtab,nasgtab,"operator","=",NULL)) < 0)
              return(doeval);
            if (doeval > 0)             /* Type of assignment */
              cx = XXASS;
        }
        if (y == 1) {                   /* Simple variable */
            if ((y = cmtxt("Definition of variable","",&s,NULL)) < 0)
              return(y);
            s = brstrip(s);
            debug(F110,"xxdef var name",vnp,0);
            debug(F110,"xxdef var def",s,0);
        } else if (y == 2) {            /* Array element */
            if ((y = arraynam(vnp,&x,&z)) < 0) return(y);
            if (x == 96) {
                printf("?Argument vector array is read-only\n");
                return(-9);
            }
            if (chkarray(x,z) < 0) return(-2);
            if ((y = cmtxt("Definition of array element","",&s,NULL)) < 0)
              return(y);
            debug(F110,"xxdef array ref",vnp,0);
            debug(F110,"xxdef array def",s,0);
        }
    } else {                            /* Macro */
#ifdef COMMENT
        if (cx == XXUNDEF) {            /* Undefine */
            if ((y = cmtxt("Text to be ignored","",&s,NULL)) < 0) return(y);
            delmac(vnp,0);
            return(success = 1);
        }
#endif /* COMMENT */
        if (mydot) {
            if ((doeval = cmkey(asgtab,nasgtab,"operator","=",NULL)) < 0)
              return(doeval);
            if (doeval > 0)
              cx = XXASS;
        }
        if ((y = cmtxt("Definition of macro","",&s,NULL)) < 0) return(y);
#ifdef DEBUG
        if (deblog) {
            debug(F110,"xxdef macro name",vnp,0);
            debug(F010,"xxdef macro def",s,0);
        }
#endif /* DEBUG */
        s = brstrip(s);
    }
    if (*s == NUL) {                    /* No arg given, undefine */
        delmac(vnp,1);                  /* silently... */
        return(success = 1);            /* even if it doesn't exist... */
    }
    /* Defining a new macro or variable */

    if (cx == XXASS || cx == XXASX) {   /* ASSIGN rather than DEFINE? */
        int t;
        t = LINBUFSIZ-1;
        lp = line;                      /* If so, expand its value now */
        zzstring(s,&lp,&t);
        s = line;
    }
    if (doeval == 2) {                  /* Arithmetic evaluation wanted too? */
        ckstrncpy(line,evala(s),LINBUFSIZ);
        line[LINBUFSIZ] = NUL;
    }
    /* debug(F111,"calling addmac",s,(int)strlen(s)); */

    y = addmac(vnp,s);                  /* Add it to the appropriate table. */
    if (y < 0) {
        printf("?%s failed\n",(cx == XXASS || cx == XXASX) ?
               "ASSIGN" : "DEFINE");
        return(success = 0);
    } else if (cx == XXASX || cx == XXDFX) /* For _ASG or _DEF, */
      return(1);                           /* don't change success variable */
    else
      return(success = 1);
}
#endif /* NOSPL */


#ifndef NODIAL
/*
   L U D I A L  --  Lookup up dialing directory entry.

   Call with string to look up and file descriptor of open dialing directory
   file.  On success, returns number of matches found, with numbers stored
   in an array accessible via getdnum().
*/
static char *dn_p[MAXDNUMS + 1];        /* Dial Number pointers */
static char *dn_p2[MAXDNUMS + 1];       /* Converted dial number pointers */
static int dn_x[MAXDNUMS + 1];          /* Type of call */
static int dncount = 0;
char * d_name = NULL;                   /* Dial name pointer */

char *                                  /* Get dial directory entry name */
getdname() {
    return(d_name ? d_name : "");
}

char *
getdnum(n) int n; {                     /* Get dial number n from directory */
    if (n < 0 || n > dncount || n > MAXDNUMS)
      return("");
    else
      return(dn_p[n]);
}

char *                  /* Check area code for spurious leading digit */
chk_ac(i,buf) int i; char buf[]; {
    char *p;
    if (!buf)
      return("");
    p = (char *) buf;                   /* Country we are calling: */
    if (i ==  44 ||                     /* UK */
        i ==  49 ||                     /* Germany */
        i ==  39 ||                     /* Italy */
        i ==  31 ||                     /* Netherlands */
        i == 351 ||                     /* Portugal */
        i ==  55 ||                     /* Brazil */
        i == 972 ||                     /* Israel */
        i ==  41 ||                     /* Switzerland */
        i ==  43 ||                     /* Austria */
        i ==  42 ||                     /* Czech Republic */
        i ==  36 ||                     /* Hungary */
        i ==  30 ||                     /* Greece */
        i == 352 ||                     /* Luxembourg */
        i ==  48 ||                     /* Poland */
        i ==  27 ||                     /* South Africa */
        i ==  33 ||                     /* France (as of 1997) */
        i ==  358                       /* Finland (ditto) */
        ) {
        if (buf[0] == '0')
          p++;
    }
    return(p);
}

/* Call Is Long Distance -- Expand this to cover 10-digit local dialing etc */
/*
   src  = area code of caller
   dest = area code of callee
   Returns:
     0 if call is local
     1 if call is long distance
     2 if call is local but area code must be dialed anyway
*/
static int
callisld(src, dest) char * src, * dest; {
    int i;
    if (dialfld)                        /* Force long distance? */
      return(1);
    if (!strcmp(src,dest)) {            /* Area codes are the same */
        for (i = 0; i < nlocalac; i++)  /* Is AC in the lc-area-codes list? */
          if (!strcmp(src,diallcac[i]))
            return(2);                  /* Yes so must be dialed */
        return(0);                      /* No so don't dial it. */
    }
    for (i = 0; i < nlocalac; i++)      /* ACs not the same so look in list */
      if (!strcmp(dest,diallcac[i]))    /* Match */
        return(2);                      /* So local call with area code */
    return(1);                          /* Not local so long-distance */
}

char pdsfx[64] = { NUL, NUL };

#ifndef NOSPL
static char *
xdial(s) char *s; {                     /* Run dial string thru macro */
    int x, m;
    char * s2;
    s2 = NULL;
    makestr(&s2,s);			/* Copy the argument */
    if (!dialmac)                       /* Dial macro name given? */
      return(NULL);
    if ((x = mxlook(mactab,dialmac,nmac)) < 0) /* Is the macro defined? */
      return(NULL);
    m = maclvl;
    x = dodo(x,s2,0);			/* Set up the macro */
    if (s2) free(s2);
    if (x > 0) {
        while (maclvl > m)              /* Execute the parser */
          parser(1);
        return(mrval[maclvl+1]);        /* Return the result */
    }
    return(NULL);
}
#endif /* NOSPL */

static int
dncvt(k,cx, prefix, suffix)
    int k, cx, prefix, suffix; {        /* Dial Number Convert */
    int i, j, n, what;                  /* cx is top-level command index */
    char *ss;                           /* prefix - add prefixes? */
    char *p, *p2, *pxo;                 /* suffix - add suffixes? */
    char *lac;
    char *npr;
    char *sfx;
    /* char *psfx; */
    char ccbuf[128];
    int cc;
    char acbuf[24];
    char *acptr;
    char outbuf[256];
/*
  First pass for strict (punctuation-based) interpretation.
  If it fails, we try the looser (length-based) one.
*/
    dialtype = -1;
    what = 0;                           /* Type of call */
    s = dn_p[k];                        /* Number to be converted. */
    debug(F111,"dncvt",s,k);
    if (dn_p2[k]) {
        free(dn_p2[k]);
        dn_p2[k] = NULL;
    }
    if (!s) {
        printf("Error - No phone number to convert\n");
        return(-1);
    }
    if ((int)strlen(s) > 200) {
        ckstrncpy(outbuf,s,40);
        printf("?Too long: \"%s...\"\n",outbuf);
        return(-1);
    }
    npr = (prefix && dialnpr) ? dialnpr : "";
    sfx = (suffix && dialsfx) ? dialsfx : "";
    /* if (partial) psfx = dialsfx ? dialsfx : ""; */
    pxo = (prefix && dialpxo) ? dialpxo : "";
    lac = diallac ? diallac : "";       /* Local area code */

    outbuf[0] = NUL;                    /* Initialize conversion buffer */
    ss = s;                             /* Remember original string */

    if (*s != '+') {                    /* Literal number */
        dn_x[k] = DN_UNK;               /* Sort key is "unknown". */
        ckmakmsg(outbuf,256,            /* Sandwich it between */
                 pxo,npr,s,sfx          /* DIAL PREFIX and SUFFIX */
                );
#ifdef CK_TAPI
        if (tttapi &&                   /* TAPI does its own conversions */
            !tapipass &&                /* if not in passthru mode */
            tapiconv == CK_AUTO ||      /* and TAPI conversions are AUTO */
            tapiconv == CK_ON           /* OR if TAPI conversions are ON */
            ) {
            char * p = NULL;
            dialtype = -2;
            if (!cktapiConvertPhoneNumber(dn_p[k], &p))
              return(-1);
            makestr(&dn_p2[k], p);
            if (p) free(p);
            return(0);
        } else
#endif /* CK_TAPI */
          makestr(&dn_p2[k], outbuf);   /* Not TAPI */
        dialtype = what;
        return(0);                      /* Done. */
    }
    i = 0;                              /* Portable number */
    s++;                                /* Tiptoe past the plus sign */
    ccbuf[0] = NUL;                     /* Do country code first */

    if (!diallcc) {                     /* Do we know our own? */
        if (cx != XXLOOK)
          printf("Error - prior SET DIAL COUNTRY-CODE command required\n");
        return(-1);
    }

    /* Parse the number */

    while (1) {                         /* Get the country code */
        while (*s == HT || *s == SP)
          s++;
        if (!s)                         /* Not in standard format */
          break;
        if (*s == '(') {                /* Beginning of area code  */
            s++;                        /* Skip past parenthesis   */
            ccbuf[i] = NUL;             /* End of country code */
            if (!s) {                   /* Check for end of string */
                printf("Error - phone number ends prematurely: \"%s\"\n",ss);
                return(-1);
            }
            break;
        } else {                        /* Collect country code */
            if (isdigit(*s))
              ccbuf[i++] = *s;          /* copy this character */
            s++;
            if (!*s || i > 127)         /* watch out for memory leak */
              break;
        }
    }
    cc = atoi(ccbuf);                   /* Numeric version of country code */

    i = 0;                              /* Now get area code */
    acbuf[0] = NUL;                     /* Initialize area-code buffer */
    acptr = acbuf;                      /* and pointer. */
    while (1) {
        while (*s == HT || *s == SP)    /* Ignore whitespace */
          s++;
        if (!s)                         /* String finished */
          break;
        if (*s == ')') {                /* End of area code  */
            s++;                        /* Skip past parenthesis   */
            acbuf[i] = NUL;             /* Terminate area-code buffer */
            break;
        } else {                        /* Part of area code */
            if (isdigit(*s))            /* If it's a digit, */
              acbuf[i++] = *s;          /* copy this character */
            s++;                        /* Point to next */
            if (!*s || i > 23)          /* Watch out for overflow */
              break;
        }
    }

/*
   Here we strip any leading 0 for countries that we know have
   0 as a long-distance prefix and do not have any area codes that
   start with 0 (formerly also ditto for "9" in Finland...)
*/
    i = atoi(ccbuf);
    acptr = chk_ac(i,acbuf);

    while (*s == HT || *s == SP)        /* Skip whitespace */
      s++;

/* printf("S=[%s], ACPTR=[%s]\n",s,acptr); */

    if (*s && *acptr) {                 /* Area code was delimited */

        while (*s == '-' || *s == '.')  /* Skip past gratuitious punctuation */
          s++;
        if (!*s) s--;                   /* But not to end of string */

        if (strcmp(diallcc,ccbuf)) {    /* Out of country? */
            if (!dialixp) {             /* Need intl-prefix */
                if (cx != XXLOOK)
                  printf("Error - No international dialing prefix defined\n");
                return(-1);
            }
            what = dn_x[k] = DN_INTL;
            p  = (prefix && dialixp) ? dialixp : ""; /* Intl-prefix */
            p2 = (suffix && dialixs) ? dialixs : ""; /* Intl-suffix */

            /* Form the final phone number */
#ifdef COMMENT
            sprintf(pdsfx,"%s%s",p2,sfx); /* UNSAFE */
            sprintf(outbuf,
                    "%s%s%s%s%s%s%s%s",
                    pxo,npr,p,ccbuf,acptr,s,p2,sfx
                    );
#else
            ckmakmsg(pdsfx,64,p2,sfx,NULL,NULL);
            ckmakxmsg(outbuf,256,pxo,npr,p,ccbuf,acptr,s,p2,sfx,
                      NULL,NULL,NULL,NULL);
#endif /* COMMENT */

        } else if ((x = callisld(lac,acptr)) >= 1) { /* In-country LD */
            if (!diallac && cx != XXLOOK) { /* Don't know my own area code */
                if (cc == 1)
                  printf("WARNING - Prior SET DIAL AREA-CODE needed\n");
            }
            if (x == 2) {               /* Local call with area code */
                what = dn_x[k] = DN_LOCAL;      /* Local-call */
                p  = (prefix && diallcp) ? diallcp : ""; /* local-prefix */
                p2 = (suffix && diallcs) ? diallcs : ""; /* local-suffix */
            } else {
                what = dn_x[k] = DN_LONG;       /* Long-distance */
                for (i = 0; i < ntollfree; i++) { /* But toll-free too? */
                    if (!strcmp(acptr,dialtfc[i])) {
                        what = dn_x[k] = DN_FREE;
                        break;
                    }
                }
                if (what == DN_FREE) {  /* Toll-free call */
                    p = (prefix && dialtfp) ? dialtfp :
                        ((prefix && dialldp) ? dialldp : "");
                    p2 = "";            /* no suffix */
                } else {                        /* normal long distance */
                    p  = (prefix && dialldp) ? dialldp : ""; /* ld-prefix */
                    p2 = (suffix && diallds) ? diallds : ""; /* ld-suffix */
                }
            }
            /* Form the number to be dialed */
#ifdef COMMENT
            sprintf(outbuf,"%s%s%s%s%s%s%s",
                    pxo,npr,p,acptr,s,p2,sfx
                    );
            sprintf(pdsfx,"%s%s",p2,sfx);
#else
            ckmakxmsg(outbuf,256,
                      pxo,npr,p,acptr,s,p2,sfx,
                      NULL,NULL,NULL,NULL,NULL);
            ckmakmsg(pdsfx,64,p2,sfx,NULL,NULL);
#endif /* COMMENT */
        } else {                        /* Same country, same area code */
            what = dn_x[k] = DN_LOCAL;  /* So it's a local call. */
            if (!prefix || !(dialpxo || ndialpxx)) { /* Not dialing from PBX */
                p  = (prefix && diallcp) ? diallcp : ""; /* local-prefix */
                p2 = (suffix && diallcs) ? diallcs : ""; /* local-suffix */
#ifdef COMMENT
                if (x == 2)
                  sprintf(outbuf,"%s%s%s%s%s%s",npr,p,acptr,s,p2,sfx);
                else
                  sprintf(outbuf,"%s%s%s%s%s",npr,p,s,p2,sfx);
                sprintf(pdsfx,"%s%s",p2,sfx);
#else
                if (x == 2)
                  ckmakxmsg(outbuf,256,
                            npr,p,acptr,s,p2,sfx,
                            NULL,NULL,NULL,NULL,NULL,NULL);
                else
                  ckmakxmsg(outbuf,256,
                            npr,p,s,p2,sfx,
                            NULL,NULL,NULL,NULL,NULL,NULL,NULL);
                ckmakmsg(pdsfx,64,p2,sfx,NULL,NULL);
#endif /* COMMENT */

            } else {                    /* Dialing from a PBX and not TAPI */
                if (ndialpxx) {         /* Is it internal? */
#ifdef COMMENT
                    i = (int) strlen(dialpxx);
                    j = (int) strlen(s);
                    x = -1;
                    if (j > i)
                      x = ckstrcmp(dialpxx,s,i,0);
#else
                    int kx;
                    x = -1;
                    j = (int) strlen(s);
                    for (kx = 0; kx < ndialpxx; kx++) {
                        i = (int) strlen(dialpxx[kx]);
                        if (j > i)
                          if (!(x = ckstrcmp(dialpxx[kx],s,i,0)))
                            break;
                    }
#endif /* COMMENT */
                    if (!x) {
                        char * icp, buf[32];
                        makestr(&matchpxx,dialpxx[kx]);
                        debug(F111,"dncvt matchpxx",matchpxx,kx);
                        what = dn_x[kx] = DN_INTERN;   /* Internal call. */
                        s += i;
                        /* Internal-call prefix */
                        icp = dialpxi;
#ifndef NOSPL
                        if (icp) {
                            if (*icp == '\\') {
                                char c, *bp;
                                int n;
                                c = *(icp+1);
                                if (isupper(c)) c = tolower(c);
                                if (c == 'v' || c == 'f') {
                                    n = 32;
                                    bp = buf;
                                    zzstring(icp,&bp,&n);
                                    icp = buf;
                                }
                            }
                        }
#endif /* NOSPL */
                        p = (prefix && icp) ? icp : "";
#ifdef COMMENT
                        sprintf(outbuf,"%s%s%s%s",npr,p,s,sfx);
#else
                        ckmakmsg(outbuf,256,npr,p,s,sfx);
#endif /* COMMENT */
                    } else {            /* External local call */
                        /* local-prefix */
                        p  = (prefix && diallcp) ? diallcp : "";
                        /* local-suffix */
                        p2 = (prefix && diallcs) ? diallcs : "";
#ifdef COMMENT
                        if (x == 2)
                          sprintf(outbuf,"%s%s%s%s%s%s%s",
                                  dialpxo ? dialpxo : "",
                                  npr,p,acptr,s,p2,sfx);
                        else
                          sprintf(outbuf,
                                  "%s%s%s%s%s%s",
                                  dialpxo ? dialpxo : "",
                                  npr,p,s,p2,sfx
                                  );
#else
                        if (x == 2)
                          ckmakxmsg(outbuf, 256,
                                   dialpxo ? dialpxo : "",
                                   npr,p,acptr,s,p2,sfx,
                                   NULL,NULL,NULL,NULL,NULL);
                        else
                          ckmakxmsg(outbuf, 256,
                                    dialpxo ? dialpxo : "",
                                    npr,p,s,p2,sfx,
                                    NULL,NULL,NULL,NULL,NULL,NULL);
#endif /* COMMENT */
                    }
                }
            }
        }

    } else {                            /* Area code was not delimited */

        char xbuf[256];                 /* Comparison based only on length */
        char ybuf[256];
        int x, j;

        s = ss;

        for (i = 0; i < 255; i++) {
            if (!*s) break;
            while (!isdigit(*s)) {      /* Pay attention only to digits */
                s++;
                if (!*s) break;
            }
            xbuf[i] = *s++;
        }
        xbuf[i] = NUL;

        x = 1;                          /* Assume LD */
        n = 0;
        if (!dialfld) {                 /* If LD not forced */
            for (j = 0; j < nlocalac; j++) { /* check local AC list? */
                ckmakmsg(ybuf,256,diallcc,diallcac[j],NULL,NULL);
                n = (int) strlen(ybuf);
                if (n > 0 && !ckstrcmp(xbuf,ybuf,n,0)) {
                    x = 2;
                    break;
                }
            }
            if (x == 1) {               /* Or exact match with local CC+AC? */
                ckmakmsg(ybuf,256,diallcc,lac,NULL,NULL);
                n = (int) strlen(ybuf);
                if (n > 0 && !ckstrcmp(xbuf,ybuf,n,0))
                  x = 0;
            }
        }
        if (x == 0 || x == 2) {         /* Local call */
            int xx,kx;                  /* Begin 1 Dec 2001... */
            /* Account for PBX internal calls */
            if (ndialpxx) {
                xx = -1;
                j = (int) strlen(ybuf);
                for (kx = 0; kx < ndialpxx; kx++) {
                    i = (int) strlen(dialpxx[kx]);
                    if (j >= i)
                      if (!(xx = ckstrcmp(dialpxx[kx],&xbuf[j],i,0)))
                        break;
                }
            }
            if (!xx) {
                char * icp, buf[32];
                makestr(&matchpxx,dialpxx[kx]);
                debug(F111,"dncvt matchpxx",matchpxx,kx);
                what = dn_x[kx] = DN_INTERN; /* Internal call. */
                s = xbuf + j + i;
                icp = dialpxi;          /* Internal-call prefix */
#ifndef NOSPL
                if (icp) {
                    if (*icp == '\\') {
                        char c, *bp;
                        int n;
                        c = *(icp+1);
                        if (isupper(c)) c = tolower(c);
                        if (c == 'v' || c == 'f') {
                            n = 32;
                            bp = buf;
                            zzstring(icp,&bp,&n);
                            icp = buf;
                        }
                    }
                }
#endif /* NOSPL */
                p = (prefix && icp) ? icp : "";
                ckmakmsg(outbuf,256,npr,p,s,sfx);
                /* End 1 Dec 2001... */

            } else {                    /* Not PBX internal */

                dn_x[k] = DN_LOCAL;
                p = (prefix && diallcp) ? diallcp : "";
                p2 = (suffix && diallcs) ? diallcs : "";
                s = (char *) (xbuf + ((x == 0) ? n : (int)strlen(diallcc)));
                ckmakxmsg(outbuf,256,
                          pxo,npr,p,s,p2,sfx,
                          NULL,NULL,NULL,NULL,NULL,NULL);
                ckmakmsg(pdsfx,64,p2,sfx,NULL,NULL);
            }
        } else {                        /* Not local */
            n = ckstrncpy(ybuf,diallcc,256);
            if (n > 0 && !ckstrcmp(xbuf,ybuf,n,0)) { /* Long distance */
                dn_x[k] = DN_LONG;
                p = (prefix && dialldp) ? dialldp : "";
                p2 = (suffix && diallds) ? diallds : "";
                s = xbuf + n;
                while (*s == '-' || *s == '.')
                  s++;
#ifdef COMMENT
                sprintf(outbuf,"%s%s%s%s%s%s",pxo,npr,p,s,p2,sfx);
                sprintf(pdsfx,"%s%s",p2,sfx);
#else
                ckmakxmsg(outbuf,256,
                          pxo,npr,p,s,p2,sfx,
                         NULL,NULL,NULL,NULL,NULL,NULL);
                ckmakmsg(pdsfx,64,p2,sfx,NULL,NULL);
#endif /* COMMENT */
            } else {
                dn_x[k] = DN_INTL;      /* International */
                if (!dialixp) {
                    if (cx != XXLOOK) {
                        printf(
                          "Error - No international dialing prefix defined\n"
                               );
                        return(-1);
                    }
                }
                p = (prefix && dialixp) ? dialixp : "";
                p2 = (suffix && dialixs) ? dialixs : "";
#ifdef COMMENT
                sprintf(outbuf,"%s%s%s%s%s%s",pxo,npr,p,xbuf,p2,sfx);
                sprintf(pdsfx,"%s%s",p2,sfx);
#else
                ckmakxmsg(outbuf,256,
                          pxo,npr,p,xbuf,p2,sfx,
                          NULL,NULL,NULL,NULL,NULL,NULL);
                ckmakmsg(pdsfx,64,p2,sfx,NULL,NULL);
#endif /* COMMENT */
            }
        }
    }
#ifdef CK_TAPI
    if (tttapi &&                       /* TAPI performs the conversions */
        !tapipass &&
        tapiconv == CK_AUTO ||
        tapiconv == CK_ON
        ) {
        p = NULL;
        dialtype = -2;
        if (!cktapiConvertPhoneNumber(dn_p[k],&p))
          return(-1);
        makestr(&dn_p2[k], p);
        if (p) free(p);
        return(0);
    } else {
#endif /* CK_TAPI */
        makestr(&dn_p2[k], outbuf);
#ifdef CK_TAPI
    }
#endif /* CK_TAPI */
    dialtype = what;
    return(0);
}

static int
ddcvt(s, f, n) char * s; FILE * f; int n; { /* Dial Directory Convert */
    char linebuf[1024], *s2;            /* Buffers and pointers */
#ifdef VMS
    char * temp = NULL;
#endif /* VMS */
    char *info[8];                      /* Pointers to words from entry */
    FILE * f2 = NULL;
    int x, rc;
    rc = -1;

    debug(F110,"ddcvt file",s,0);

    if (!s || !f)                       /* No filename or file */
      return(-1);
    if (!*s)

    fclose(f);
    znewn(s,&s2);                       /* s2 = address of static buffer */
    debug(F110,"ddcvt newname",s2,0);

#ifdef VMS
    /* In VMS, znewn() returns the same file name with a new version number */
    makestr(&temp,s);                   /* Swap - otherwise the new */
    s = s2;                             /* version has the older version */
    s2 = temp;                          /* number... */
    debug(F110,"ddcvt after swap s",s,0);
    debug(F110,"ddcvt after swap s2",s2,0);
    makestr(&(dialdir[n]),s);           /* New file gets new version number */
    debug(F110,"ddcvt after makestr s2",s2,0);
    debug(F111,"ddcvt dialdir[n]",dialdir[n],n);
#else
    if (zrename(s,s2) < 0) {            /* Not VMS - rename old file */
        perror(s2);                     /* to new (wierd) name. */
        goto ddexit;
    }
#endif /* VMS */
    debug(F110,"ddcvt s2 (old)",s2,0);
    if ((f = fopen(s2,"r")) == NULL) {  /* Reopen old file with wierd name */
        debug(F110,"ddcvt s2 open error",ck_errstr(),0);
        dirline = 0;                    /* (or in VMS, old version) */
        perror(s2);
        goto ddexit;
    }
    debug(F110,"ddcvt fopen(s2) OK",s2,0);

    debug(F110,"ddcvt s (new)",s,0);
    if ((f2 = fopen(s,"w")) == NULL) {  /* Create new file with old name */
        debug(F110,"ddcvt s open error",ck_errstr(),0);
        perror(s);                      /* (or in VMS, new version) */
        goto ddexit;
    }
    debug(F110,"ddcvt fopen(s) OK",s,0);

    printf("\nSaving old directory as %s.\nConverting %s...",s2,s);
    fprintf(f2,"; %s - Kermit dialing directory\n", s);
    fprintf(f2,"%-16s %-20s ; %5s %-6s ; %s\n",
               "; Name","Number","Speed","Parity","Comment"
               );

    while (1) {
        linebuf[0] = NUL;               /* Read a line */
        if (fgets(linebuf,1023,f) == NULL)
          break;
        debug(F110,"ddcvt linebuf",linebuf,0);
        if (!linebuf[0]) {              /* Empty line */
            fprintf(f2,"\n");
            continue;
        }
        x = (int) strlen(linebuf);      /* Strip line terminator, */
        while (x-- > 0) {               /* if any. */
            if (linebuf[x] <= SP)
              linebuf[x] = NUL;
            else
              break;
        }
        xwords(linebuf,5,info,1);       /* Parse it the old way */
        for (x = 1; x < 6; x++)
          if (!info[x]) info[x] = "";
        fprintf(f2,"%-16s %-20s ; %5s %-6s %s\n",
               info[1],info[2],info[3],info[4],info[5]
               );
    }
    printf(" OK\n\n");
    rc = 0;                             /* Success */
  ddexit:
    if (f) fclose(f);
    if (f2) fclose(f2);
#ifdef VMS
    if (temp) free(temp);
#endif /* VMS */
    return(rc);
}

int                                     /* s = name to look up   */
#ifdef CK_ANSIC                         /* cx = index of command */
ludial(char *s, int cx)                 /* (DIAL, LOOKUP, etc)   */
#else
ludial(s, cx) char *s; int cx;
#endif /* CK_ANSIC */
/* ludial */ {

    int dd, n1, n2, n3, i, j, t;        /* Workers */
    int olddir, newdir, oldentry, newentry;
    int pass = 0;
    int oldflg = 0;
    int ambiguous = 0;                  /* Flag for lookup was ambiguous */
    char *info[7];                      /* Pointers to words from entry */
    char *pp;                           /* Pointer to element of array */
    FILE * f;
    char *line;                         /* File input buffer */

/* #define LUDEBUG */

#ifdef LUDEBUG
int zz = 1;
#endif /* LUDEBUG */

    if (!s || ndialdir < 1)             /* Validate arguments */
      return(-1);

    if ((n1 = (int) strlen(s)) < 1)     /* Length of string to look up */
      return(-1);

    if (!(line = malloc(1024)))         /* Allocate input buffer */
      return(-1);

#ifdef LUDEBUG
if (zz) printf("LUDIAL 1 s[%s], n1=%d\n",s,n1);
#endif /* LUDEBUG */

    pass = 0;
  lu_again:
    f = NULL;                           /* Dial directory file descriptor */
    t = dncount = 0;                    /* Dial-number match count */
    dd = 0;                             /* Directory counter */
    olddir = 0;
    newdir = 0;
/*
  We need to recognize both old- and new-style directories.
  But we can't allow old-style and new-style entries in the same
  directory because there is no way to tell for sure the difference between
  an old-style entry like this:

    foo  5551212  9600

  and a new-style literal entry like this:

    foo  555 9600

  I.e. is the "9600" a speed, or part of the phone number?
*/
    while (1) {                         /* We make one pass */
        if (!f) {                       /* Directory not open */
            if (dd >= ndialdir)         /* No directories left? */
              break;                    /* Done. */
            debug(F111,"ludial dialdir[dd]",dialdir[dd],dd);
            if ((f = fopen(dialdir[dd],"r")) == NULL) { /* Open it */
                perror(dialdir[dd]);    /* Can't, print message saying why */
                if (line) {
                    free(line);
                    line = NULL;
                }
                dd++;                   /* Go on to next one, if any... */
                continue;
            }
            dirline = 0;                /* Directory file line number */
            if (dialdpy && !pass)
              printf("Opening: %s...\n",dialdir[dd]);
            dd++;
            if (!oldflg) olddir = 0;
            newdir = 0;
        }
        oldentry = 0;
        newentry = 0;
        line[0] = NUL;
        if (getnct(line,1023,f,1) < 0) { /* Read a line */
            if (f) {                    /* f can be clobbered! */
                fclose(f);              /* Close the file */
                f = NULL;               /* Indicate next one needs opening */
                oldflg = 0;
            }
            continue;
        }
        if (!line[0])                   /* Empty line */
          continue;
#ifdef LUDEBUG
if (zz) printf("LUDIAL 2 s[%s]\n",s);
#endif /* LUDEBUG */

        /* Make a copy and parse it the old way */
        /* A copy is needed because xwords() pokes NULs into the string */

        if ((pp = malloc((int)strlen(line) + 1))) {
            strcpy(pp,line);            /* safe */
            xwords(pp,5,info,0);        /* Parse it the old way */

#ifdef LUDEBUG
if (zz) printf("LUDIAL 3 s[%s]\n",s);
#endif /* LUDEBUG */

            if (!info[1])
              continue;
            if (*info[1] == ';') {      /* If full-line comment, */
                newdir = 1;             /* (only new directories have them) */
                continue;               /* keep reading. */
            }
            if (!info[2])
              continue;
            if (*info[2] == '+')
              newentry = 1;
            if (info[4]) {
                if ((*info[4] == '=') ||
                    !ckstrcmp(info[4],"none", 4,0) ||
                    !ckstrcmp(info[4],"even", 4,0) ||
                    !ckstrcmp(info[4],"space",5,0) ||
                    !ckstrcmp(info[4],"mark", 4,0) ||
                    !ckstrcmp(info[4],"odd",  3,0)
                    )
                  oldentry = 1;
            }
        }
        if (pp) {
            free(pp);
            pp = NULL;
        }

        /* Check consistency */

        if ((oldentry || olddir) && (newentry || newdir)) {
            printf(
"\nERROR: You seem to have old- and new-format entries mixed in your\n");
            printf(
"dialing directory.  You'll have to edit it by hand to convert it to the\n");
#ifndef NOHELP
            printf("new format.  Type HELP DIAL for further information.\n\n");
#else
            printf("new format.\n\n");
#endif /* NOHELP */
            if (line) {
                free(line);
                line = NULL;
            }
            return(-1);
        }
        if (!olddir && oldentry) {
            int convert = 0;
            olddir = 1;
            if (dialcvt == 2) {         /* 2 == ASK */
                sprintf(tmpbuf,
"WARNING: Old-style dialing directory detected:\n%s", line);
		convert = uq_ok(tmpbuf,
				"Shall I convert it for you? ",3,NULL,0);
            } else
              convert = dialcvt;
            if (convert) {
                debug(F111,"ludial calling ddcvt",dialdir[dd-1],dd);
                if (ddcvt(dialdir[dd-1],f,dd-1) < 0) {
                    debug(F111,"ludial ddcvt failed",dialdir[dd-1],dd);
                    oldflg = 1;
                    printf(
"  Sorry, can't convert.");
                    printf(
"  Will ignore speed and parity fields, continuing...\n\n");
                } else {
                    olddir = newdir = 0;
                    debug(F111,"ludial ddcvt ok",dialdir[dd-1],dd);
                }
                dd--;
                f = NULL;
                continue;
            } else {
                if (dialcvt == 2)
                  printf(
"  OK, will ignore speed and parity fields, continuing...\n\n");
                olddir = 1;
            }
        }

#ifdef LUDEBUG
if (zz) printf("LUDIAL XX s[%s], n1=%d\n",s,n1);
#endif /* LUDEBUG */

        /* Now parse again for real */

        if (oldentry)                   /* Parse it the old way */
          xwords(line,5,info,0);
        else                            /* Parse it the new way */
          xwords(line,2,info,1);

#ifdef LUDEBUG
if (zz) printf("LUDIAL YY s[%s], n1=%d\n",s,n1);
if (zz) printf("%s [%s]\n",info[1],info[2]);
#endif /* LUDEBUG */

        if (info[1]) {                  /* First word is entry name */
            if ((n3 = (int) strlen(info[1])) < 1) /* Its length */
              continue;                 /* If no first word, keep reading. */
            if (n3 < n1)                /* Search name is longer */
              continue;                 /* Can't possibly match */
            if (ambiguous && n3 != n1)
              continue;

#ifdef LUDEBUG
if (zz) printf("MATCHING: [%s] [%s], n1=%d\n",s,info[1],n1);
#endif /* LUDEBUG */

            if (ckstrcmp(s,info[1],n1,0)) /* Caseless string comparison */
              continue;

#ifdef LUDEBUG
if (zz) printf("MATCH OK: [%s] [%s], n1=%d\n",s,info[1],n1);
#endif /* LUDEBUG */

            if (!info[2])               /* No phone number given */
              continue;
            if ((n2 = (int) strlen(info[2])) < 1) /* Length of phone number */
              continue;                 /* Ignore empty phone numbers */

            /* Got one */

            if (!(pp = (char *)malloc(n2 + 1))) { /* Allocate storage for it */
                printf("?internal error - ludial malloc 1\n");
                if (line) {
                    free(line);
                    line = NULL;
                }
                dncount = 0;
                return(-1);
            }
            strcpy(pp,info[2]);         /* safe */

            if (dncount > MAXDNUMS) {
                printf("Warning: %d matches found, %d max\n",
                       dncount,
                       MAXDNUMS
                       );
                dncount = MAXDNUMS;
                break;
            }
            dn_p[dncount++] = pp;       /* Add pointer to array. */
            if (dncount == 1) {         /* First one... */
                if (d_name) free(d_name);
                if (!(d_name = (char *)malloc(n3 + 1))) { /* Save its name */
                    printf("?internal error - ludial malloc 2\n");
                    if (line) {
                        free(line);
                        line = NULL;
                    }
                    dncount = 0;
                    return(-1);
                }
                t = n3;                 /* And its length */
                strcpy(d_name,info[1]); /* safe */
            } else {                    /* Second or subsequent one */

#ifdef LUDEBUG
                if (zz)
                  printf("d_name=[%s],info[1]=%s,t=[%d]\n",d_name,info[1],t);
#endif /* LUDEBUG */

                if ((int) strlen(info[1]) == t) /* Lengths compare */
                  if (!ckstrcmp(d_name,info[1],t,0)) /* Caseless compare OK */
                    continue;

                /* Name given by user matches entries with different names */

                if (ambiguous)          /* Been here before */
                  break;

                ambiguous = 1;          /* Now an exact match is required */
                for (j = 0; j < dncount; j++) { /* Clean out previous list */
                    if (dn_p[j]) {
                        free(dn_p[j]);
                        dn_p[j] = NULL;
                    }
                }
                pass++;                 /* Second pass... */
                goto lu_again;          /* Do it all over again. */
            }
        }
    }
    if (line) free(line);
    if (dncount == 0 && ambiguous) {
        printf(" Lookup: \"%s\" - ambiguous%s\n",
               s,
               cx == XXLOOK ? "" : " - dialing skipped"
               );
        return(-2);
    }
    return(dncount);
}

char *
pncvt(s) char *s; {                     /* Phone number conversion */
    char *p = NULL;                     /* (just a wrapper for dncvt() */
    char *q = NULL;
    static char pnbuf[128];
    makestr(&p,dn_p[0]);                /* Save these in case they are */
    makestr(&q,dn_p2[0]);               /* being used */
    makestr(&dn_p[0],s);                /* Copy the argument string to here */
    dncvt(0,XXLOOK,1,1);                /* Convert it */
    if (!dn_p2[0])                      /* Put result where can return it */
      pnbuf[0] = NUL;
    else
      ckstrncpy(pnbuf,dn_p2[0],127);
    makestr(&dn_p[0],p);                /* Restore these */
    makestr(&dn_p2[0],q);
    makestr(&p,NULL);                   /* Free these */
    makestr(&q,NULL);
    return((char *)pnbuf);
}

int
dodial(cx) int cx; {                    /* DIAL or REDIAL */
    int i = 0, x = 0;                   /* Workers */
    int sparity = -1;                   /* For saving global parity value */
    int previous = 0;
    int len = 0;
    int literal = 0;
    int flowsave;
    int lufound = 0;                    /* Did any lookup succeed? */
    int prefix = 1;
    int postfix = 1;
    int wasalpha = 0;
    int xredial = 0;
    int braces = 0;

    char *p = NULL, *s3 = NULL, * sav = NULL;
    int j = 0, t = 0, n = 0;
    int xretries, xlcc;

#ifdef COMMENT
    debug(F101,"dodial cx","",cx);
    debug(F111,"dodial diallcc",diallcc,diallcc);
#endif	/* COMMENT */

    xretries = dialrtr;                 /* If retries not set, */
    if (diallcc) {                      /* choose default based on */
        xlcc = atoi(diallcc);           /* local country code. */
        if (xretries < 0) {
            switch (xlcc) {
              case 1: xretries = 10; break; /* No restrictions in NANP */
                /* Add other country codes here */
                /* that are known to have no restrictions on redialing. */
              default: xretries = 1;
            }
        }
    }
    if (cx == XXPDIA) {                 /* Shortcut... */
        cx = XXDIAL;
        partial = 1;
        debug(F100,"PDIAL sets partial=1","",0);
        postfix = 0;                    /* Do not add postfix */
    } else {
        partial = 0;
        debug(F100,"DIAL sets partial=0","",0);
    }
    previous = dialsta;                 /* Status of previous call, if any */
    if (previous == DIA_PART) {
        prefix = 0;                     /* do not add prefix */
    }
    s = NULL;                           /* Initialize user's dial string */
    if (cx == XXRED) {                  /* REDIAL or... */
        if ((y = cmcfm()) < 0)
          return(y);
    } else if (cx == XXANSW) {          /* ANSWER or ... */
        if ((y = cmnum("timeout (seconds)","0",10,&x,xxstring)) < 0)
          return(y);
        dialatmo = x;
        if ((y = cmcfm()) < 0)
          return(y);
    } else {                            /* DIAL or LOOKUP */
        if (ndialdir > 0)
          s3 = "Number to dial or entry from dial directory";
        else
          s3 = "Number to dial";
        if ((x = cmtxt(s3, dialnum ? dialnum : "",&s,xxstring)) < 0)
          return(x);
        if (s) {
            len = (int) strlen(s);
            ckstrncpy(tmpbuf,s,TMPBUFSIZ); /* Save literal copy */
#ifdef COMMENT
            if (len > 1) {              /* Strip outer braces if given */
                if (*s == '{') {
                    if (s[len-1] == '}') {
                        s[len-1] = NUL;
                        s++;
                        len -= 2;
                    }
                }
            }
#else
            s = brstrip(s);             /* Strip outer braces or quotes */
#endif /* COMMENT */
        }
    }

    if (cx != XXLOOK) {                 /* Not LOOKUP */
#ifdef IKSD
        if (inserver) {
            printf("Sorry, dialing is disabled.\r\n");
            return(success = 0);
        }
#endif /* IKSD */
#ifdef CK_TAPI
        if (tttapi && !tapipass) {
          ;                             /* Skip the modem test if TAPI */
        } else
#endif /* CK_TAPI */
        if (mdmtyp < 1 && !dialtest) {
            if (network
#ifdef TN_COMPORT
                 && !istncomport()
#endif /* TN_COMPORT */
                 )
              printf("Please SET HOST first, and then SET MODEM TYPE\n");
            else
              printf("Sorry, you must SET MODEM TYPE first\n");
            dialsta = DIA_NOMO;
            return(success = 0);
        }
        if (!local && !dialtest) {
            printf("Sorry, you must SET %s or SET HOST first\n",
#ifdef OS2
                   "PORT"
#else
                   "LINE"
#endif /* OS2 */
                   );
            dialsta = DIA_NOLI;
            return(success = 0);
        }
        if ((!network 
#ifdef TN_COMPORT
              || istncomport()
#endif /* TN_COMPORT */
              ) && !dialtest &&
#ifdef CK_TAPI
             !tttapi &&
#endif /* CK_TAPI */
            (speed < 0L)
#ifdef UNIX
            && (strcmp(ttname,"/dev/null"))
#else
#ifdef OSK
            && (strcmp(ttname,"/nil"))
#endif /* OSK */
#endif /* UNIX */
            ) {
            printf("\nSorry, you must SET SPEED first\n");
            dialsta = DIA_NOSP;
            return(success = 0);
        }
    }
    if (cx != XXANSW) {
        for (j = 0; j < MAXDNUMS; j++) { /* Initialize dial-number list */
            if (!dialnum) {             /* First time dialing */
                dn_p[j] = NULL;         /* initialize all pointers. */
                dn_p2[j] = NULL;
            } else if (dn_p[j]) {       /* Not the first time, */
                free(dn_p[j]);          /* free previous, if any, */
                dn_p[j] = NULL;         /* then set to NULL. */
                if (dn_p2[j])
                  free(dn_p2[j]);
                dn_p2[j] = NULL;
            } else break;               /* Already NULL */
        }
        if (len == 0)
          s = NULL;
        if (!s)
          s = dialnum;
        if (!s) {
            if (cx == XXLOOK)
              printf("?Lookup what?\n");
            else
              printf("%s\n", (cx == XXRED) ?
                   "?No DIAL command given yet" :
                   "?You must specify a number to dial"
                   );
            return(-9);
        }

    /* Now we have the "raw" dial or lookup string and s is not NULL */

        makestr(&dscopy,s);             /* Put it in a safe place */
        s = dscopy;
        n = 0;

        debug(F111,"dodial",s,ndialdir);

        wasalpha = 0;
        if (isalpha(*s)) {
            wasalpha = 1;
            if (ndialdir > 0) {         /* Do we have a dialing directory? */
                n = ludial(s,cx);       /* Look up what the user typed */
                if (n == 0)
                  printf(" Lookup: \"%s\" - not found%s\n",
                         s,
                         cx == XXLOOK ? "" : " - dialing as given\n"
                         );
            }
            debug(F101,"dodial",s,n);
            if (n < 0 && cx != XXLOOK) { /* Error out if they wanted to dial */
                if (n == -1)            /* -2 means ludial already gave msg */
                  printf(" Lookup: fatal error - dialing skipped\n");
                dialsta = DIA_DIR;
                return(-9);
            }
            if (n > 0)                  /* A successful lookup */
              lufound = 1;
        } else if (*s == '=') {         /* If number starts with = sign */
            s++;                        /* strip it */
            literal = 1;                /* remember this */
            while (*s == SP) s++;       /* and then also any leading spaces */
        } else if (tmpbuf[0] == '{' && tmpbuf[1] == '{') {
            makelist(tmpbuf,dn_p,MAXDNUMS);
            makestr(&dscopy,tmpbuf);
            s = tmpbuf;
            for (n = 0; n < MAXDNUMS; n++) /* (have to count how many) */
              if (!dn_p[n]) break;
            braces = 1;
        }
        if (cx == XXLOOK && !wasalpha && !braces) {
            /* We've been told to lookup a number or a quoted name */
            char *p;
            n = 0;
            p = literal ? s : pncvt(dscopy);
            if (!p) p = "";
            if (*p) {
                printf("%s  => %s\n", dscopy, p);
                return(success = 1);
            } else {
                printf("?Bad phone number\n");
                return(success = 0);
            }
        }
        /* Save DIAL or successful LOOKUP string for future DIAL or REDIAL */
        /* But don't save pieces of partial dial ... */

        debug(F101,"DIAL save dialnum partial","",partial);
        debug(F101,"DIAL save dialnum previous","",previous);
        if ((cx == XXDIAL && partial == 0 && previous != DIA_PART) ||
            (cx == XXLOOK && n > 0)) {
            makestr(&dialnum,dscopy);
            if (!quiet && dscopy && !dialnum)
              printf("WARNING - memory allocation failure: redial number\n");
        }
        if (n > 0) {
            if (!quiet && !backgrd && !braces /* && dialdpy */ ) {
                if (!strcmp(d_name,s))
                  printf(" Lookup: \"%s\" - exact match\n",s);
                else
                  printf(" Lookup: \"%s\" - uniquely matches \"%s\"\n",
                         s,
                         d_name
                         );
            }
            if ((cx == XXLOOK) ||
                ((n > 1) && !quiet && !backgrd /* && dialdpy */ )) {
                printf(" %d telephone number%sfound for \"%s\"%s\n",
                       n,
                       (n == 1) ? " " : "s ",
                       s,
                       (n > 0) ? ":" : "."
                       );
                s3 = getdname();
            }
            for (i = 0; i < n; i++) {   /* Convert */
                dn_x[i] = -1;
                if (dncvt(i,cx,prefix,postfix) < 0) {
                    if (cx != XXLOOK) {
                        dialsta = DIA_DIR;
                        return(-9);
                    }
                }
            }
            if (dialsrt && n > 1) {     /* Sort into optimal order */
                for (i = 0; i < n-1; i++) {
                    for (j = i+1; j < n; j++) {
                        if (dn_x[j] < dn_x[i]) {
                            t = dn_x[j];
                            dn_x[j] = dn_x[i];
                            dn_x[i] = t;
                            p = dn_p[j];
                            dn_p[j] = dn_p[i];
                            dn_p[i] = p;
                            p = dn_p2[j];
                            dn_p2[j] = dn_p2[i];
                            dn_p2[i] = p;
                        }
                    }
                }
            }
            if ((cx == XXLOOK) ||
                ((n > 1) && !quiet && !backgrd /* && dialdpy */ )) {
                int nn = n;
#ifndef NOSPL
                char * p;
#endif /* NOSPL */
                if (cx != XXLOOK)
                  if (n > 12) nn = 12;
                for (i = 0; i < nn; i++) {
                    printf("%3d. %-12s  %-20s =>  %-20s  (%d)\n",i+1,
                           s3, dn_p[i],
                           dn_p2[i] ? dn_p2[i] : "(processing failed)",
                           dn_x[i]
                           );
                }
                if (cx != XXLOOK && n != nn)
                  printf("And %d more...\n", n - nn);
            }
        } else if (n == 0) {            /* Not found in directory */
            makestr(&(dn_p[0]),literal ? s : dscopy);
            makestr(&d_name,literal ? s : dscopy);
            dncount = 1;
            n = 1;
            if (dncvt(0,cx,prefix,postfix) < 0) { /* In case they typed a */
                dialsta = DIA_DIR;      /* portable-format number ... */
                return(-9);
            }
        }

#ifndef NONET
#ifdef NETCONN
        /* It's not good that the networks directory depends on NOT-NODIAL.. */
        if (cx == XXLOOK && dscopy) {   /* Networks here too... */
            extern char *nh_p[], *nh_p2[], *n_name;
            extern char *nh_px[4][MAXDNUMS+1];
            n = -1;
            if (nnetdir > 0) {          /* Do we have a network directory? */
                dirline = 0;
                n = lunet(dscopy);      /* Look up what the user typed */
            }
            if (n > -1) {
                int k;
                if (n > 0)              /* A successful lookup */
                  lufound = 1;
                if (cx == XXLOOK && n == 0)
                  printf(" Lookup: \"%s\" - not found\n",dscopy);
                else
                  printf("%s %d network entr%s found for \"%s\"%s\n",
                         cx == XXLOOK ? " Lookup:" : "",
                         n,
                         (n == 1) ? "y" : "ies",
                         dscopy,
                         (n > 0) ? ":" : "."
                         );

                for (i = 0; i < n; i++) {

                    printf("%3d. %-12s => %-9s %s",
                           i+1,n_name,nh_p2[i],nh_p[i]);
                    for (k = 0; k < 4; k++) {
                        if (nh_px[k][i]) {
                            printf(" %s",nh_px[k][i]);
                        } else
                          break;
                    }
                    printf("\n");
                }
            }
        }
#endif /* NETCONN */
#endif /* NONET */
        if (cx == XXLOOK)
          return(success = lufound);
    } /* cx != XXANSW */

#ifdef VMS
    conres();                   /* So Ctrl-C/Y will work */
#endif /* VMS */
/*
  Some modems do not react well to parity.  Also, if we are dialing through a
  TCP/IP TELNET modem server, parity can be fatally misinterpreted as TELNET
  negotiations.

  This should work even if the user interrupts the DIAL command, because the
  DIAL module has its own interrupt handler.  BUT... if, for some reason, a
  dialing device actually *requires* parity (e.g. CCITT V.25bis says that even
  parity should be used), this might prevent successful dialing.  For that
  reason, we don't do this for V.25bis modems.
*/
    sparity = parity;                   /* Save current parity */
    if ((dialcapas & CKD_V25) == 0)     /* If not V.25bis...  */
      parity = 0;                       /* Set parity to NONE */

    flowsave = flow;
/*
  These modems use some kind of screwy flow control while in command mode,
  and do not present CTS as they should.  So if RTS/CTS is set (or even if
  it isn't) disable flow control during dialing.
*/
#ifndef MINIDIAL
    if (mdmtyp == n_ATT1910 || mdmtyp == n_ATT1900) {
        flow = FLO_NONE;                /* This is not enough */
#ifdef CK_TTSETFLOW
        ttsetflow(FLO_NONE);            /* Really turn it off */
#endif /* CK_TTSETFLOW */
    }
#endif /* MINIDIAL */
    if (!network
#ifdef TN_COMPORT
        || istncomport()
#endif /* TN_COMPORT */
         ) {
        int x;
        if ((x = ttgmdm()) > -1) {
            if (!x && msgflg) {
                printf(
"WARNING - No modem signals detected.  Is your modem turned on?  If not,\n\
use Ctrl-C to interrupt dialing, turn on your modem, then %s.\n",
                       cx == XXANSW ?
                       "ANSWER again" :
                       "REDIAL"
                       );
            }
            if (flow == FLO_RTSC) {
                if (!(x & BM_CTS)) {
                    if (msgflg)
                      printf(
"WARNING - SET FLOW RTS/CTS is in effect but modem's CTS signal is off.\n\
Disabling flow control temporarily %s...\n",
                             cx == XXANSW ?
                             "while waiting for call" :
                             "during dialing"
                             );
                    flow = FLO_NONE;
                }
            }
        }
    }
    if (cx == XXANSW) {                 /* ANSWER */
        success = ckdial("",0,0,1,0);
        goto dialfin;
    }

/* Edit 192 adds the ability to dial repeatedly. */

    i = 0;
    dialcount = 0;
    do {
        if (i > 0) printf("\nDial attempt %d of %d...\n", i+1, xretries);
        dialcount = i+1;
        success = 0;
        /* And the ability to dial alternate numbers. */
        /* Loop to dial each in a list of numbers for the same name... */
        for (j = 0; j < n && !success; j++) { /* until one answers. */
            s = dn_p2[j];               /* Next number in list */
            if (dn_x[j] >= dialrstr) {  /* Dial restriction */
                printf("Restricted: %s, skipping...\n",dn_p[j]);
                continue;
            }
            xredial = (i == 0 && j == 0) ? 0 : 1;
            if (!s) s = dn_p[j];

#ifndef NOSPL
            sav = s;
            p = xdial(s);               /* Apply DIAL macro now */
            if (p) if (*p) s = p;
#endif /* NOSPL */

	    /* Dial confirmation */
	    /* NOTE: the uq_xxx() calls allow for a GUI dialog */

            if (i == 0 && dialcnf) {
		char msgbuf[128];
		ckmakmsg(msgbuf,128,"Dialing ",s,NULL,NULL);
		x = uq_ok(msgbuf,"Is this number correct? ",3,NULL,0);
                if (!x) {

#ifndef COMMENT
		    x = uq_txt(		/* Allow GUI dialog */
#ifdef OS2
" Please enter the correct number,\r\n or press Enter to skip.",
#else
" Please enter the correct number,\r\n or press Return to skip.",
#endif /* OS2 */
                              "Corrected phone number: ",
                               1,
			       NULL,
			       atmbuf,
			       ATMBL,
                               s,
                               DEFAULT_UQ_TIMEOUT
                               );
		    if (x && atmbuf[0]) { /* They gave a new one */
			s = atmbuf;
			makestr(&(dn_p2[j]), s);
		    }			

#else  /* COMMENT */

#ifdef CK_RECALL
                    extern int on_recall;
#endif /* CK_RECALL */
                    cmsavp(psave,PROMPTL);
                    cmsetp(
#ifdef OS2
" Please enter the correct number,\r\n or press Enter to skip: "
#else
" Please enter the correct number,\r\n or press Return to skip: "
#endif /* OS2 */
                           );
                    cmini(ckxech);
                    x = -1;
                    if (pflag) prompt(NULL);
#ifdef CK_RECALL
                    on_recall = 0;
#endif /* CK_RECALL */
                    y = cmdgquo();
                    cmdsquo(0);
                    while (x < 0) {
                        x = cmtxt("Corrected phone number","",&s,NULL);
                        cmres();
                    }
                    if ((int) strlen(s) < 1) {
                        cmsetp(psave);
                        continue;
                    }
                    makestr(&(dn_p2[j]), s);
                    cmdsquo(y);
                    cmsetp(psave);
#endif /* COMMENT */
                }
            }
            if (dialtest) {             /* Just testing */
                if (i + j == 0)
                  printf("\nTESTING...\n");
                if (dialmac)
                  printf(" Number: \"%s\" => \"%s\"\n",sav,s);
                else
                  printf(" Number: \"%s\"\n",s);
                dialsta = DIA_BUSY;
                success = 0;
            } else {
                what |= W_DIALING;
                success = ckdial(s,i,j,partial ? 3 : 0, xredial); /* Dial it */
                what &= ~(W_DIALING);
                if (!success) {
                    if (dialsta < 8 ||  /* Break out if unrecoverable error */
                        dialsta  == DIA_INTR ||
                        dialsta  == DIA_ERR  ||
                        previous == DIA_PART
                        )
                      break;
                }
            }
        }
        if (success)                    /* Succeeded, leave the outer loop */
          break;
        if (dialsta < 8 ||              /* Break out if unrecoverable error */
            dialsta == DIA_INTR ||      /* Interrupted */
            dialsta == DIA_NODT ||      /* No dialtone */
            dialsta == DIA_NOAC ||      /* Access forbidden */
            dialsta == DIA_BLCK ||      /* Blacklisted */
            dialsta == DIA_DIR  ||      /* Dialing directory error */
            dialsta == DIA_ERR  ||      /* Modem command error */
            previous == DIA_PART)
          break;
        if (++i >= xretries)            /* Break out if too many tries */
          break;
        if (!backgrd && !quiet) {
            if (dialint > 5)
              printf(
"\nWill redial in %d second%s- press any key to redial immediately.\n",
                     dialint,
                     dialint == 1 ? " " : "s "
                     );
            printf("Ctrl-C to cancel...\n");
        }
        x = dialint;                    /* Redial interval */
        while (x-- > 0) {
            if ((y = conchk()) > 0) {   /* Did they type something? */
                while (y--) coninc(0);  /* Yes, absorb it */
                break;                  /* And wake up */
            }
            sleep(1);                   /* No interrupt, sleep a sec */
        }
    } while (!success);

  dialfin:

    if (cx != XXLOOK) {
        if (!success)
          bleep((short) BP_FAIL);
        else if (!quiet)
          bleep((short) BP_NOTE);
#ifdef OS2
        setint();                       /* Fix OS/2 interrupts */
#endif /* OS2 */
        if (sparity > -1)
          parity = sparity;             /* Restore parity if we saved it */
        flow = flowsave;
#ifdef OS2
        ttres();                        /* Restore DIAL device */
#endif /* OS2 */
#ifdef VMS
        concb((char)escape);            /* Restore console */
#endif /* VMS */
#ifdef OS2
        {                               /* Set session title */
            char * p, name[72];         /* in window list. */
            char * q;
            if (cx == XXANSW) {
                q = "Incoming call";
            } else {
                if (d_name)
                  q = d_name;
                else if (dialnum)
                  q = dialnum;
                else if (ttname[0])
                  q = ttname;
                else q = "";
            }
            p = name;
            if (success) {
                strncpy(name,q,48);
                while (*p) {            /* Uppercase it for emphasis. */
                    if (islower(*p))
                      *p = toupper(*p);
                    p++;
                }
            } else
              name[0] = NUL ;
            os2settitle((char *) name, TRUE);
        }
#endif /* OS2 */
    }
    if (cx != XXLOOK) {
        if (success) {
            if (reliable == SET_AUTO) { /* It's not a reliable connection. */
                reliable = SET_OFF;
                debug(F101,"dodial reliable","",reliable);
            }
        } else {
#ifndef NOHINTS
            extern int hints;
            if (hints && !quiet && dialsta != 9) { /* 9 == User interrupted */
                extern int dialmhu, dialhng, dialdpy;
                extern char * dialmsg[];
                printf("\n*************************\n");
                printf("DIAL-class command failed.\n");
                printf("Modem type:  %s\n", gmdmtyp());
                printf("Device:      %s\n", ttname);
                printf("Speed:       %ld\n", speed);
                printf("Dial status: %d",dialsta);
                if (dialsta < 35 && dialmsg[dialsta])
                  printf(" [%s]",dialmsg[dialsta]);
                printf("\n");
                if (dialsta == DIA_TIMO ||
                    dialsta == DIA_NRDY ||
                   (dialsta > 13 && dialsta != DIA_BUSY && dialsta != DIA_NOAN)
                    ) {
                    switch (dialsta) {
                      case DIA_TIMO:
                        printf(
" . SET DIAL TIMEOUT to a greater value and try again.\n"
                               );
                        break;
                      case DIA_NRSP:
                      case DIA_NRDY:
                      case DIA_NOIN:
                        printf(
" . Is the modem turned on?\n"
                               );
                        printf(
" . Are you using the right communication port?\n"
                               );
                        break;
                      case DIA_NODT:
                        printf(
" . Is the modem connected to the telephone line?\n"
                               );
                    }
                    if (mdmtyp == n_GENERIC) {
                        printf(
" . Please choose a specific modem type with SET MODEM TYPE and try again.\n"
                               );
                        printf(
"    SET MODEM TYPE ? to see the list of known modem types.\n"
                               );
                    } else {
                        printf(
" . Are you sure you have chosen the appropriate modem type?\n"
                               );
                    }
                    if (speed > 19200L) {
                        printf(
" . Maybe the interface speed (%ld) is too fast:\n", speed
                               );
                        printf(
"    SET SPEED to a lower speed and try again.\n"
                               );
                        printf(
"    SET SPEED ? to see the list of valid speeds.\n"
                               );
                    }
                    if (dialhng) {
                        if (dialmhu)
                          printf(
" . SET MODEM HANGUP-METHOD RS232 and try again.\n"
                                 );
                        else
                          printf(
" . SET MODEM HANGUP-METHOD MODEM-COMMAND and try again.\n"
                                 );
                        printf(
" . If that doesn't work, try again with SET DIAL HANGUP OFF.\n"
                               );
                    } else {
                        printf(
" . Give a HANGUP or SET DIAL HANGUP ON command and try again.\n"
                               );
                    }
                    if (!dialdpy)
                      printf(
" . Use SET DIAL DISPLAY ON to watch the dialog between Kermit and modem.\n"
                             );
                }
#ifndef NOSHOW
                printf(
" . SHOW COMMUNICATIONS, SHOW MODEM, SHOW DIAL to see current settings.\n"
                       );
#endif /* NOSHOW */

#ifndef NOHELP
                printf(
" . HELP SET MODEM, HELP SET DIAL, and HELP DIAL for more information.\n"
                       );
#endif /* NOHELP */
                printf("(Use SET HINTS OFF to suppress future hints.)\n");
                printf("*************************\n\n");
            }
#endif /* NOHINTS */
        }
    }
    return(success);
}
#endif /* NODIAL */

/*  D O T Y P E  --  Type (display) a file with various options...  */

#ifdef BIGBUFOK
#define TYPBUFL 16384
#else
#define TYPBUFL 256
#endif /* BIGBUFOK */

int typ_lines = 0;                      /* \v(ty_ln) */
int typ_mtchs = 0;                      /* \v(ty_lm) */
static int typ_int = 0;                 /* Flag if TYPE interrupted */

#ifdef UNICODE
extern int fcharset, fileorder, byteorder, ucsorder;
#define TYPXBUFL TYPBUFL+TYPBUFL+TYPBUFL+4
static char * mp = NULL;
static char * mbuf = NULL;
static long xn = 0L;

static int
#ifdef CK_ANSIC
storechar(char c)
#else
storechar(c) char c;
#endif /* CK_ANSIC */
{
    if (!mp) return(-1);
    if (++xn > TYPXBUFL)
      return(-1);
    debug(F111,"storechar xn",ckitoa((int)c),xn);
    *mp++ = c;
    return(0);
}
#endif /* UNICODE */

static FILE * ofp = NULL;               /* For /OUTPUT: file */

static int
typeline(buf,len,outcs,ofp) char * buf; int len, outcs; FILE * ofp; {
    register int i;

    debug(F011,"typeline buf",buf,len);
    /* debug(F101,"typeline outcs","",outcs); */

#ifdef OS2
#ifndef NOLOCAL
#ifdef UNICODE
    /* In K95 only, the buffer is guaranteed to be in UCS-2 if outcs >= 0. */
    /* Len is its length in bytes.  There is no line terminator. */
    /* outcs is the file character-set number (FC_xxx) of the target set */
    /* that was requested by the user. */
    if (!inserver && !k95stdout) {
        extern int wherex[], wherey[];
        extern unsigned char colorcmd;

        VscrnWrtUCS2StrAtt( VCMD, (unsigned short *)buf, len/2,
                           wherey[VCMD], wherex[VCMD], &colorcmd);
        printf("\r\n");
        return(0);
    }
#endif /* UNICODE */
#endif /* NOLOCAL */
#endif /* OS2 */

/* In Unix, VMS, etc, the line has already been converted to the desired  */
/* character-set, if one was given.  OR... on all platforms, including in */
/* K95, we don't know the character set.  In either case we dump the line */
/* byte by byte in case it contains NULs (printf() would truncate). */

#ifdef COMMENT
    for (i = 0; i < len; i++)
      putchar(buf[i]);
#else
    for (i = 0; i < len; i++) {
        if (ofp == stdout) {
            putchar(buf[i]);
        } else {
            putc(buf[i],ofp);
        }
    }
#endif /* COMMENT */

#ifdef IKSD
    if (inserver) {
#ifdef UNICODE
        if (outcs == FC_UCS2) {
            if (ofp == stdout) {
                putchar(NUL);
            } else {
                putc(NUL,ofp);
            }
        }
#endif /* UNICODE */
        if (ofp == stdout) {
            putchar('\r');
        } else {
            putc('\r',ofp);
        }
    }
#endif /* IKSD */
#ifdef UNICODE
    if (outcs == FC_UCS2) {
        if (ofp == stdout) {
            putchar(NUL);
        } else {
            putc(NUL,ofp);
        }
    }
#endif /* UNICODE */
    if (ofp == stdout) {
        putchar('\n');
    } else {
        putc('\n',ofp);
    }
    fflush(stdout);
    return(0);
}

static int                              /* Get translated line */
typegetline(incs, outcs, buf, n) int incs, outcs, n; char * buf; {
    int x = 0, c0, c1, len = 0, count = 0, eof = 0, xlate = 0;
#ifdef UNICODE
    int xxn = -1;
    int yyn = -9;
    xn = 0L;

#ifdef DEBUG
    if (deblog && typ_lines == 0) {
        debug(F101,"typegetline incs","",incs);
        debug(F101,"typegetline outcs","",outcs);
        debug(F101,"typegetline feol","",feol);
        debug(F101,"typegetline byteorder","",byteorder);
        debug(F101,"typegetline ucsorder ","",ucsorder);
        debug(F111,"typegetline fileorder","1",fileorder);
    }
#endif /* DEBUG */

    if (incs < 0)                       /* Shouldn't happen */
      return(-2);

    if (outcs == -1)                    /* Can happen */
      outcs = incs;

    if (incs != outcs || incs == FC_UCS2) { /* See if we should translate */
        xlate = 1;
        if (!mbuf) {                    /* Allocate buffer if not allocated */
            mbuf = (char *)malloc(TYPXBUFL+1); /* yet */
            if (!mbuf) {
                printf("WARNING: Translation buffer allocation failure.\n");
                printf("Translation will be skipped...\n");
                xlate = 0;
            }
        }
    }
    if (xlate) {                        /* Translating... */
        mp = mbuf;                      /* Reset working buffer pointer */
/*
  Here we call xgnbyte() in a loop, having it return UCS-2 bytes.  In K95, we
  use UCS-2 directly.  Elsewhere, we feed the UCS-2 bytes into xpnbyte() to
  convert them to the desired target character set.  But since we are using
  UCS-2, we have several sources for confusion: (1) xgnbyte() might return in
  LE or BE byte order, with no explicit indication of what the order is; but
  (2) xpnbyte() wants BE; but (3) Windows wants LE.
*/
        while (1) {
            if (typ_int)                /* Quit if interrupted */
              return(0);
            c0 = xgnbyte(FC_UCS2,incs,NULL); /* Convert to UCS-2 */
            debug(F000,"typegetline c0","",c0);
            if (c0 < 0) {               /* EOF */
                eof++;
                break;
            }
            c1 = xgnbyte(FC_UCS2,incs,NULL); /* Convert to UCS-2 */
            debug(F000,"typegetline c1","",c1);
            if (c1 < 0) {               /* EOF */
                eof++;
                break;
            }
#ifdef DEBUG
            if (deblog && typ_lines == 0) {
                if (count == 0) /* Check fileorder after BOM */
                  debug(F111,"typegetline fileorder","2",fileorder);
            }
#endif /* DEBUG */

#ifdef COMMENT
/* Now we have the two UCS-2 bytes.  Which order are they in? */

            if (fileorder > 0) {        /* Little Endian */
                int t;                  /* So swap them */
                debug(F100,"typegetline swapping","",0);
                t = c1;
                c1 = c0;
                c0 = t;
            }
#endif /* COMMENT */
            if (c0 == 0 && c1 == 0x0D)  /* Now see if we have EOL */
              yyn = xn;

            if (c0 == 0 && c1 == 0x0A)  /* Now see if we have EOL */
              xxn = xn;

            count++;                    /* Count byte */

/* Give the two bytes to xpnbyte() in BE order */

            if ((x = xpnbyte(c0,TC_UCS2,outcs,storechar)) < 0) return(-1);
            if ((x = xpnbyte(c1,TC_UCS2,outcs,storechar)) < 0) return(-1);

            if (xxn > -1) {             /* Have end of line? */
                xn = xxn;
                if (yyn == xxn - 2)     /* Adjust for CRLF */
                  xn = yyn;
                break;                  /* And break out of loop. */
            }
        }
        mbuf[xn] = NUL;
        if (xn > n)                     /* Can truncate here... */
          xn = n;
        memcpy(buf,mbuf,xn);
        debug(F011,"typegetline xlate",buf,xn);
        return((eof && (xn == 0)) ? -1 : xn);
    }
#endif /* UNICODE */
#ifdef COMMENT
    /* We can't use this because, stupidly, zsinl() doesn't return a length. */
    /* It could be changed but then we'd have to change all ck?fio.c modules */
    x = zsinl(ZIFILE,buf,n);
#else
    /* So instead, we copy zsinl() to here... */
    /* But note: This does not necessarily handle UCS-2 alignment properly;  */
    /* that's what the code in the first section of this routine is for. */
    /* But it does tolerate files that contain NULs. */
    {
        int a;
        char *s;

        s = buf;
        a = -1;                         /* Current character, none yet. */
        debug(F101,"typegetline zsinl simulation","",n);
        while (n--) {                   /* Up to given length */
#ifdef COMMENT
            int old = 0;
            if (feol)                   /* Previous character */
              old = a;
#endif /* COMMENT */
            if (zchin(ZIFILE,&a) < 0) { /* Read a character from the file */
                debug(F101,"typegetline zchin fail","",count);
                if (count == 0)
                  x = -1;               /* EOF or other error */
                break;
            } else
              count++;
            if (feol) {                 /* Single-character line terminator */
                if (a == feol)
                  break;
            } else {                    /* CRLF line terminator */
#ifdef COMMENT
/* Debug log shows that in Windows, <CR><LF> is returned as <LF>. */
/* Apparently we're not reading the file in binary mode. */

                if (a == '\015')        /* CR, get next character */
                  continue;
                if (old == '\015') {    /* Previous character was CR */
                    if (a == '\012') {  /* This one is LF, so we have a line */
                        break;
                    } else {            /* Not LF, deposit CR */
                        *s++ = '\015';
                        n--;
                        len++;
                    }
                }
#else
                if (a == LF) {
                    if (s[len] == CR) { /* This probably won't happen */
                        s[len] = NUL;
                        s--;
                        len--;
                    }
                    break;
                }
#endif /* COMMENT */
            }
            *s = a;                     /* Deposit character */
            s++;
            len++;
        }
        *s = '\0';                      /* Terminate the string */
    }
#endif /* COMMENT */
    return(x < 0 ? -1 : len);
}


#ifndef MAC
SIGTYP
#ifdef CK_ANSIC
tytrap(int foo)                         /* TYPE interrupt trap */
#else
tytrap(foo) int foo;
#endif /* CK_ANSIC */
/* tytrap */ {
#ifdef __EMX__
    signal(SIGINT, SIG_ACK);
#endif
    debug(F100,"type tytrap SIGINT","",0);
    typ_int = 1;                        /* (Need arg for ANSI C) */
    SIGRETURN;
}
#endif /* MAC */

int
dotype(file, paging, first, head, pat, width, prefix, incs, outcs, outfile, z)
    char * file, * pat, * prefix; int paging, first, head, width, incs, outcs;
    char * outfile; int z;
/* dotype */ {
    extern CK_OFF_T ffc;
    char buf[TYPBUFL+2];
    char * s = NULL;
    int rc = 1, lines = 0, ucs2 = 0;
    char ** tail = NULL;
    int * tlen = NULL;
    int tailing = 0, counting = 0;
    int x, c, n, i, j, k = 0;
    int number = 0, save, len, pfxlen = 0, evalpfx = 1;
#ifdef UNICODE
    int ucsbom_sav;
    extern int ucsbom;
#endif /* UNICODE */
#ifdef NT
    int gui = 0;
#endif /* NT */

#ifndef MAC
#ifdef OS2
#ifdef NT
    SIGTYP (* oldsig)(int);             /* For saving old interrupt trap. */
#else /* NT */
    SIGTYP (* volatile oldsig)(int);
#endif /* NT */
#else /* OS2 */
    SIGTYP (* oldsig)();
#endif /* OS2 */
#endif /* MAC */

#ifdef KUI
    if (outfile == (char *)1) {
        gui = 1;
        outfile = "";
    }
#endif /* KUI */

    if (!file) file = "";
    if (!*file) return(-2);

    if (ofp != stdout) {                /* In case of previous interruption */
        if (ofp) fclose(ofp);
        ofp = stdout;
    }
    if (!outfile) outfile = "";
    if (outfile[0]) {
        ofp = fopen(outfile,"w");       /* Open output file */
        if (!ofp) {
            printf("?Can't open output file %s: %s\n",outfile,ck_errstr());
            ofp = stdout;
            return(-9);
        }
    }
    number = z;
    if (number && prefix) prefix = NULL;

#ifdef UNICODE
    ucsbom_sav = ucsbom;                /* We are not creating a file */
    ucsbom = 0;                         /* Do not use BOM bytes */
#endif /* UNICODE */

    typ_int = 0;

    save = binary;                      /* Save file type */

    debug(F101,"dotype incs","",incs);
    debug(F101,"dotype outcs","",outcs);

#ifdef UNICODE
    debug(F111,"dotype fileorder","A",fileorder);
#ifdef OS2
    if (!inserver && !k95stdout)
      outcs = FC_UCS2;
#endif /* OS2 */

    if (outcs == FC_UCS2)               /* Output is UCS-2? */
      ucs2 = 1;
    if (fileorder < 0)
      fileorder = ucsorder;
    debug(F111,"dotype fileorder","B",fileorder);
#endif /* UNICODE */

#ifdef CK_TTGWSIZ
#ifdef OS2
    ttgcwsz();
#else /* OS2 */
    /* Check whether window size changed */
    if (ttgwsiz() > 0) {
        if (tt_rows > 0 && tt_cols > 0) {
            cmd_rows = tt_rows;
            cmd_cols = tt_cols;
            debug(F101,"dotype cmd_rows","",cmd_rows);
            debug(F101,"dotype cmd_cols","",cmd_cols);
        }
    }
#endif /* OS2 */
#endif /* CK_TTGWSIZ */

    if (prefix)
      pfxlen = strlen(prefix);

    if (paging < 0) {                   /* Count only, don't print */
        counting = 1;
        prefix = NULL;
        width = 0;
        paging = 0;
    }
    if (ucs2)                           /* Crude... */
      width *= 2;

#ifdef OS2
    if (*file) {
        ckstrncpy(buf, file, TYPBUFL);  /* Change / to \. */
        p = buf;
        while (*p) {
            if (*p == '/') *p = '\\';
            p++;
        }
        file = buf;
    } else {
        rc = 0;
        goto xdotype;
    }
#endif /* OS2 */

    if (zchki(file) == -2) {            /* It's a directory */
        debug(F111,"dotype zchki failure",file,-2);
        if (xcmdsrc == 0) {
            printf("?Not a regular file: \"%s\"\n",file);
            rc = -9;
        } else
          rc = 0;
        goto xdotype;
    }
    if (!zopeni(ZIFILE, file)) {        /* Not a directory, open it */
        debug(F111,"dotype zopeni failure",file,0);
        if (xcmdsrc == 0) {
            printf("?Can't open file: \"%s\"\n",file);
            rc = -9;
        } else
          rc = 0;
        goto xdotype;
    }

#ifndef AMIGA
#ifndef MAC
    errno = 0;
    oldsig = signal(SIGINT, tytrap);    /* Save current interrupt trap. */
    /* debug(F111,"type SIGINT trap set",ckitoa(errno),oldsig); */
#endif /* MAC */
#endif /* AMIGA */

    if (paging > -1)                    /* More-prompting */
      xaskmore = paging;

    binary = 0;

    if (head < 0) {                     /* "tail" was requested */
        tailing = 1;                    /* Set flag */
        head = 0 - head;                /* Get absolute number of lines */
        if (!counting) {
            tail = (char **) malloc(head * sizeof(char *)); /* Allocate list */
            if (!tail) {
                printf("?Memory allocation failure\n");
                goto xdotype;

            }
            tlen = (int *) malloc(head * sizeof(int));
            if (!tlen) {
                printf("?Memory allocation failure\n");
                goto xdotype;

            }
            for (i = 0; i < head; i++) { /* Initialize each pointer in list. */
                tail[i] = NULL;
                tlen[i] = 0;
            }
        }
    }
    typ_lines = 0;
    typ_mtchs = 0;

#ifdef UNICODE
    if (outcs > -1 && (incs != outcs || incs == FC_UCS2)) { /* Translating? */
        ffc = (CK_OFF_T)0;
        initxlate(incs,outcs);          /* Set up translation functions */
    } else
#endif /* UNICODE */
      outcs = -1;                       /* Means we don't know the charset */

    debug(F101,"dotype ffc","",ffc);
    debug(F101,"dotype outcs 2","",outcs);
#ifdef UNICODE
    debug(F111,"dotype fileorder","C",fileorder);
#endif /* UNICODE */

    /* Allow the buffer to contain NULs */

    for (n = first;
         (len = typegetline(incs,outcs,buf,TYPBUFL)) > -1;
         lines++
         ) {
        debug(F011,"dotype line",buf,len);
#ifndef MAC
        if (typ_int) {                  /* Interrupted? */
            typ_int = 0;
            debug(F101,"type interrupted line","",lines);
            printf("^C...\n");          /* Print message */
            if (ofp != stdout) {        /* Close any output file */
                if (ofp) fclose(ofp);
                ofp = stdout;
            }
            goto xxdotype;
        }
#endif /* MAC */
        typ_lines++;                    /* For \v(ty_ln) */
        if (pat)                        /* Matching? */
          if (!ckmatch(pat,buf,1,1+4))  /* Line matches pattern? */
            continue;                   /* No, skip it */
        typ_mtchs++;

        if (head > 0 && !tailing && lines == head) /* Handle /HEAD:n */
          break;

        buf[TYPBUFL+1] = NUL;           /* Just in case... */
        if (prefix) {                   /* Add specified prefix to each line */
            char pbuf[64];
            char * pp;
            pp = prefix;
#ifndef NOSPL
            if (evalpfx) {              /* Prefix is a variable? */
                int n = 63;             /* Maybe - evaluate it and see */
                char * p = pbuf;
                zzstring(prefix,&p,&n); /* If there is no change */
                if (!strcmp(prefix,pbuf)) { /* it's not a variable */
                    evalpfx = 0;        /* So don't do this again. */
                } else {                /* It was a variable */
                    pp = pbuf;          /* So substitute its value */
                    pfxlen = 63 - n;    /* and get its new length */
                }
            }
#endif /* NOSPL */
            if (len + pfxlen + 2 < TYPBUFL) {
                /* Shift right to make room for prefix */
                memcpy((char *)line+pfxlen,(char *)buf,len);
                lset((char *)line,pp,pfxlen,SP);
                debug(F110,"dotype prefix",line,pfxlen);
                len += pfxlen;
                memcpy((char *)buf,(char *)line,len);
            }
        } else if (number) {            /* Line numbers */
            int x;
            sprintf(line,"%4d. ",typ_lines);
            x = strlen(line);
            len += x;
            if (len < LINBUFSIZ) {
                memcpy((char *)&line[x],(char *)buf,len);
                memcpy((char *)buf,(char *)line,len);
            }
        }
        if (width > 0 && width <= TYPBUFL) { /* Truncate at given width. */
            char * obuf = line;         /* But to do that first we must */
            int i,k,z;                  /* expand tabs; assume every 8 cols. */
            line[0] = NUL;
            for (i = 0, k = 0; i < width; k++) { /* Character loop... */
                if (!buf[k])            /* No more chars in this line, done. */
                  break;
                if (buf[k] != '\t') {   /* If it's not a tab */
                    if (i >= LINBUFSIZ) /* Check for overflow */
                      break;
                    obuf[i++] = buf[k]; /* and then deposit it. */
                    obuf[i] = NUL;      /* Keep it null-terminated */
                    continue;
                }
                z = 8 - (i % 8);        /* It's a tab, expand it. */
                if (z == 0) z = 8;
                for (j = 0; j < z && i < LINBUFSIZ; j++) {
#ifdef UNICODE
                    if (ucs2 && !ucsorder)
                      obuf[i++] = NUL;
#endif /* UNICODE */
                    obuf[i++] = ' ';
#ifdef UNICODE
                    if (ucs2 && ucsorder)
                      obuf[i++] = NUL;
#endif /* UNICODE */
                }
                obuf[i++] = NUL;
                obuf[i] = NUL;
            }
            obuf[width] = NUL;          /* Now truncate at given width. */
#ifdef COMMENT
            /* This doesn't work for UCS-2 because it contains NULs */
            ckstrncpy(buf,obuf,TYPBUFL); /* and copy it back (again?) */
#else
            memcpy((char *)buf,(char *)obuf,i); /* Copy it back */
#endif /* COMMENT */
            len = (i > width) ? width : i; /* Spare us another strlen()... */
        }
        if (tailing) {                  /* If /TAIL:n... */
            k = lines % head;           /* save this line in circular buffer */
            if (!counting) {
                if (tail[k]) free(tail[k]);
                tail[k] = malloc(len+2);
                if (!tail[k]) {
                    printf("?Memory allocation failure\n");
                    goto xdotype;
                }
                memcpy(tail[k],buf,len);
                tlen[k] = len;
                continue;
            }
        }
        if (counting)                   /* If only counting */
          continue;                     /* we're done with this line */

        if (paging) {                   /* Displaying this line... */
            int u;
            u = len;                    /* Length in BYTES */
            if (ucs2)                   /* If outputting in UCS-2 */
              u /= 2;                   /* convert length to CHARACTERS */
            x = (u / cmd_cols) + 1;     /* Crudely allow for wrap */
            if (cmd_rows > 0 && cmd_cols > 0)
              n += x;                   /* This assumes terminal will wrap */
        }
#ifdef KUI
        if ( gui ) {
            int i;
            unsigned short * uch = (unsigned short *)buf;
            for ( i=0; i<len/2; i++)
                gui_text_popup_append(uch[i]);
			gui_text_popup_append(CR);
			gui_text_popup_append(LF);
        } 
        else
#endif /* KUI */
        typeline(buf,len,outcs,ofp);    /* Print line, length based */
#ifdef CK_TTGWSIZ
        debug(F101,"dotype n","",n);
        if (paging > 0 && ofp == stdout) { /* Pause at end of screen */
            if (cmd_rows > 0 && cmd_cols > 0) {
                if (n > cmd_rows - 3) {
                    if (!askmore())
                      goto xdotype;
                    else
                      n = 0;
                }
            }
        }
#endif /* CK_TTGWSIZ */
    }

  xdotype:
    if (counting) {
        fprintf(ofp,
                "%s: %d line%s\n",file,typ_lines,typ_lines == 1 ? "" : "s");
        if (pat)
          fprintf(ofp,
                  "%s: %d match%s\n",pat,typ_mtchs,typ_mtchs == 1 ? "" : "es");
        goto xxdotype;
    }
    if (tailing && tail) {              /* Typing tail of file? */
        if (lines < head) {             /* Yes, show the lines we saved */
            k = 0;                      /* Show all lines */
        } else {                        /* More lines than tail number */
            lines = k;                  /* Last line to show */
            k++;                        /* First line to show */
            if (k >= head)
              k = 0;
        }
        n = first;                      /* Output line counter */
        for (i = k ;; i++) {            /* Loop thru circular buffer */
#ifndef MAC
            if (typ_int) {              /* Interrupted? */
                printf("^C...\n");      /* Print message */
                goto xxdotype;
            }
#endif /* MAC */
            j = i % head;               /* Index of this line */
            s = tail[j];                /* Point to line to display */
            if (!s)                     /* (shouldn't happen...) */
              break;
            if (paging) {               /* Crudely allow for line wrap */
                x = tlen[j];
                if (ucs2) x /= 2;
                x = x / cmd_cols + 1;
                if (cmd_rows > 0 && cmd_cols > 0)
                  n += x;
            }
            typeline(s,tlen[j],outcs,ofp); /* Display this line */
            if (paging && ofp == stdout) { /* Pause at end of screen */
                if (cmd_rows > 0 && cmd_cols > 0) {
                    if (n > cmd_rows - 3) {
                        if (!askmore())
                          break;
                        else
                          n = 0;
                    }
                }
            }
            tail[j] = NULL;
            free(s);                    /* Free the line */
            if (i % head == lines)      /* When to stop */
              break;
        }
        free((char *)tail);             /* Free the list */
        tail = NULL;
        if (tlen) free((char *)tlen);
        tlen = NULL;
    }

/* Come here when finished or on SIGINT */

  xxdotype:
#ifndef AMIGA
#ifndef MAC
    signal(SIGINT,oldsig);              /* Put old signal action back. */
#endif /* MAC */
#endif /* AMIGA */
    if (tailing && tail) {
        for (i = 0; i < head; i++) {    /* Free each line. */
            if (tail[i])
              free(tail[i]);
        }
        free((char *)tail);             /* Free list pointer */
        if (tlen)
          free((char *)tlen);
    }
    x = zclose(ZIFILE);                 /* Done, close the input file */
    if (ofp != stdout) {                /* Close any output file */
        if (ofp) fclose(ofp);
        ofp = stdout;
    }
    binary = save;                      /* Restore text/binary mode */
#ifdef UNICODE
    ucsbom = ucsbom_sav;                /* Restore BOM usage */
#endif /* UNICODE */

#ifdef KUI
    if ( gui )
        gui_text_popup_wait(-1);        /* Wait for user to close the dialog */
#endif /* KUI */
    return(rc);
}

/* GREP command */

#define GREP_CASE  0                    /* /CASE */
#define GREP_COUN  1                    /* /COUNT */
#define GREP_DOTF  2                    /* /DOTFILES */
#define GREP_NAME  3                    /* /NAMEONLY */
#define GREP_NOBK  4                    /* /NOBACKUP */
#define GREP_NODO  5                    /* /NODOTFILES */
#define GREP_NOLI  6                    /* /NOLIST */
#define GREP_NOMA  7                    /* /INVERT = /NOMATCH */
#define GREP_NOPA  8                    /* /NOPAGE */
#define GREP_NUMS  9                    /* /LINENUMBERS */
#define GREP_PAGE 10                    /* /PAGE */
#define GREP_RECU 11                    /* /RECURSIVE */
#define GREP_TYPE 12                    /* /TYPE: */
#define GREP_OUTP 13                    /* /OUTPUTFILE: */
#define GREP_EXCP 14			/* /EXCEPT: */

static struct keytab greptab[] = {
    { "/count",        GREP_COUN, CM_ARG },
    { "/dotfiles",     GREP_DOTF, 0 },
    { "/except",       GREP_EXCP, CM_ARG },
    { "/linenumbers",  GREP_NUMS, 0 },
    { "/nameonly",     GREP_NAME, 0 },
    { "/nobackupfiles",GREP_NOBK, 0 },
    { "/nocase",       GREP_CASE, 0 },
    { "/nodotfiles",   GREP_NODO, 0 },
    { "/nolist",       GREP_NOLI, 0 },
    { "/nomatch",      GREP_NOMA, 0 },
    { "/nopage",       GREP_NOPA, 0 },
    { "/output",       GREP_OUTP, CM_ARG },
    { "/page",         GREP_PAGE, 0 },
    { "/quiet",        GREP_NOLI, CM_INV },
#ifdef RECURSIVE
    { "/recursive",    GREP_RECU, 0 },
#endif /* RECURSIVE */
    { "/type",         GREP_TYPE, CM_ARG },
    { "", 0, 0 }
};
static int ngreptab =  sizeof(greptab)/sizeof(struct keytab)-1;

static char * grep_except = NULL;

int
dogrep() {
    int match, x, y, fc, getval, mc = 0, count = 0, bigcount = 0;
    int fline = 0, sline = 0, wild = 0, len = 0;
    int xmode = -1, scan = 0;
    char c, name[CKMAXPATH+1], outfile[CKMAXPATH+1], *p, *s, *cv = NULL;
    FILE * fp = NULL;

    int                                 /* Switch values and defaults */
      gr_coun = 0,
      gr_name = 0,
      gr_nobk = 0,
      gr_case = 1,
      gr_noli = 0,
      gr_noma = 0,
      gr_nums = 0,
      gr_excp = 0,
      gr_page = xaskmore;

    struct FDB sw, fl;

    g_matchdot = matchdot;              /* Save global matchdot setting */
    outfile[0] = NUL;
    makestr(&grep_except,NULL);

    if (ofp != stdout) {                /* In case of previous interruption */
        if (ofp) fclose(ofp);
        ofp = stdout;
    }
    cmfdbi(&sw,                         /* First FDB - command switches */
           _CMKEY,                      /* fcode */
           "String or pattern to search for, or switch",
           "",                          /* default */
           "",                          /* addtl string data */
           ngreptab,                    /* addtl numeric data 1: tbl size */
           4,                           /* addtl numeric data 2: 4 = cmswi */
           xxstring,                    /* Processing function */
           greptab,                     /* Keyword table */
           &fl                          /* Pointer to next FDB */
           );
    cmfdbi(&fl,                         /* Anything that doesn't match */
           _CMFLD,                      /* fcode */
           "",                          /* hlpmsg */
           "",                          /* default */
           "",                          /* addtl string data */
           0,                           /* addtl numeric data 1 */
           0,                           /* addtl numeric data 2 */
           xxstring,			/* xxstring */
           NULL,
           NULL
           );
    while (1) {                         /* Parse 0 or more switches */
        x = cmfdb(&sw);                 /* Parse something */
        if (x < 0)
          return(x);
        if (cmresult.fcode != _CMKEY)   /* Break out if not a switch */
          break;
        c = cmgbrk();
        if ((getval = (c == ':' || c == '=')) && !(cmgkwflgs() & CM_ARG)) {
            printf("?This switch does not take an argument\n");
            return(-9);
        }
        if ((cmresult.nresult != GREP_COUN) && !getval &&
            (cmgkwflgs() & CM_ARG)) {
            printf("?This switch requires an argument\n");
            return(-9);
        }
        switch (cmresult.nresult) {
          case GREP_COUN: {
              gr_coun++;
              if (getval) {
                  if ((x = cmfld("Variable for result","",&s,NULL)) < 0)
                    return(x);
                  makestr(&cv,s);
              }
              break;
          }
          case GREP_CASE: gr_case=0; break;
          case GREP_NAME: gr_name++; gr_noli=0; break;
          case GREP_NOBK: gr_nobk++; break;
          case GREP_NOLI: gr_noli++; gr_name=0; gr_nums=0; break;
          case GREP_NOMA: gr_noma++; break;
          case GREP_NOPA: gr_page=0; break;
          case GREP_NUMS: gr_nums++; gr_noli=0; break;
          case GREP_PAGE: gr_page++; gr_noli=0; break;
          case GREP_NODO:
            matchdot = 0;
            break;
          case GREP_DOTF:
            matchdot = 1;
            break;
#ifdef RECURSIVE
          case GREP_RECU:
            recursive = 1;
            break;
#endif /* RECURSIVE */
          case GREP_TYPE: {
              extern struct keytab txtbin[];
              if ((x = cmkey(txtbin,3,"","",xxstring)) < 0)
                return(x);
              if (x == 2) {             /* ALL */
                  xmode = -1;
              } else {                  /* TEXT or BINARY only */
                  xmode = x;
                  scan = 1;
              }
              break;
          }
          case GREP_OUTP:               /* Send output to file */
            if ((x = cmofi("File for GREP'd lines","",&s,xxstring)) < 0)
              return(x);
            ckstrncpy(outfile,s,CKMAXPATH);
            break;
	  case GREP_EXCP:		/* Exception pattern */
	    if (getval) {
		if ((x = cmfld("Exception pattern",
			       "",
			       &s,
			       xxstring
			       )) < 0)
		  return(x);
		gr_excp++;
		makestr(&grep_except,s);
	    }
        }
    }
    if (outfile[0]) {
        ofp = fopen(outfile,"w");       /* Open output file */
        if (!ofp) {
            printf("?Can't open output file %s: %s\n",outfile,ck_errstr());
            ofp = stdout;
            return(-9);
        }
        gr_page = 0;
    }
    s = cmresult.sresult;
    s = brstrip(s);                     /* Strip braces from pattern */
    if (!*s) {
        printf("?Pattern required\n");
        return(-9);
    }
    ckstrncpy(tmpbuf,s,TMPBUFSIZ);      /* Save pattern */
    if ((x = cmifi("File(s) to search","",&s,&wild,xxstring)) < 0) {
        if (x == -3) {
            printf("?File specification required\n");
            x = -9;
        }
        return(x);
    }
    s = brstrip(s);                     /* Strip braces from filename */
#ifndef ZXREWIND
    ckstrncpy(line,s,LINBUFSIZ);
#endif /* ZXREWIND */
    if ((y = cmcfm()) < 0)
      return(y);

    if (gr_page > -1)
      xaskmore = gr_page;               /* Paging... */

    p = tmpbuf;                         /* Point to pattern */
#ifdef COMMENT
/* Now this is done in ckmatch */
    if (*p == '^') {                    /* '^' anchors pattern to beginning */
        p++;
    } else if (*p != '*') {             /* Otherwise prepend implied '*' */
        tmpbuf[0] = '*';
        p = tmpbuf;
    }
    x = strlen(p);                      /* Get length of result */
    if (x > 0 && x < TMPBUFSIZ) {       /* '$' at end anchors pattern to end */
        if (p[x-1] == '$') {
            p[x-1] = NUL;
        } else if (p[x-1] != '*') {
            p[x] = '*';
            p[x+1] = NUL;
        }
    }
#endif /* COMMENT */
    debug(F111,"grep pat",p,x);

#ifdef ZXREWIND
    fc = zxrewind();                    /* Rewind the file list */
#else
    {
        int flags = ZX_FILONLY;         /* Expand file list */
        if (matchdot)  flags |= ZX_MATCHDOT;
        if (recursive) flags |= ZX_RECURSE;
        fc = nzxpand(line,flags);
    }
#endif /* ZXREWIND */
#ifdef UNIX
    sh_sort(mtchs,NULL,fc,0,0,filecase);
#endif /* UNIX */

    debug(F101,"grep cmd_rows","",cmd_rows);
    debug(F101,"grep cmd_cols","",cmd_cols);

    while (1) {                         /* Loop for each file */
        znext(name);                    /* Get next file */
        if (!name[0])                   /* No more, done */
          break;
        if (gr_nobk)                    /* Skipping backup files? */
          if (ckmatch("*.~[1-9]*~",name,1,1)) /* Backup file? */
            continue;                   /* Yes, skip */
        if (scan) {                     /* /TYPE: given? */
            switch (scanfile(name,&y,nscanfile)) { /* Yes, scan the file */
              case FT_BIN:
                if (xmode != 1)
                  continue;
                break;
              case FT_TEXT:
              case FT_7BIT:
              case FT_8BIT:
#ifdef UNICODE
              case FT_UTF8:
              case FT_UCS2:
#endif /* UNICODE */
                if (xmode != 0)
                  continue;
            }
        }
        fp = fopen(name,"r");           /* Open */
        if (!fp)                        /* Can't */
          continue;                     /* Skip */
        count = 0;                      /* Match count, this file */
        fline = 0;                      /* Line count, this file */
        while (1) {                     /* Loop for each line */
            if (fgets(line,LINBUFSIZ,fp) == NULL) { /* Get next line */
                fclose(fp);
                fp = NULL;
		debug(F100,"GREP EOF","",0);
                break;
            }
            fline++;                    /* Count this line */
            line[LINBUFSIZ] = NUL;      /* Make sure it's terminated */
	    debug(F111,"GREP",line,fline);
            len = (int)strlen(line);    /* Get length */
            while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
              line[--len] = NUL;        /* Chop off terminators */
            match = ckmatch(p,line,gr_case,1+4); /* Match against pattern */
	    if (match && gr_excp) {
		if (ckmatch(grep_except,line,gr_case,1+4))
		    match = 0;
	    }
            if (gr_noma)                /* Invert match sense if requested */
              match = !match;
            if (match) {                /* Have a matching line */
                mc++;                   /* Total match count */
                count++;                /* Match count this file */
                if (gr_name) {          /* Don't care how many lines match */
                    fclose(fp);         /* Close the file */
                    fp = NULL;          /* and quit the line-reading loop. */
                    break;
                }
                if (gr_coun || gr_noli) /* Not listing each line */
                  continue;             /* so don't print anything now. */
                if (wild) {		/* If searching multiple files */
                    fprintf(ofp,"%s:",name); /* print filename. */
                    len += (int)strlen(name) + 1;
                }
                if (gr_nums) {          /* If line numbers wanted */
                    char nbuf[32];
                    len += ckmakmsg(nbuf,32,ckitoa(fline),":",NULL,NULL);
                    fprintf(ofp,"%s",nbuf);
                }
                if (cmd_rows > 0 && cmd_cols > 0)
                  sline += (len / cmd_cols) + 1;
                fprintf(ofp,"%s\n",line); /* Print the line. */
                if (sline > cmd_rows - 3) {
                    if (!askmore()) goto xgrep; else sline = 0;
                }
            }
        }
        if (!gr_noli) {			/* If not not listing... */
            x = 0;
            if (gr_coun) {              /* Show match count only */
                fprintf(ofp,"%s:%d\n",name,count);
                x++;
            } else if (gr_name && count > 0) { /* Show name only */
                fprintf(ofp,"%s\n",name);
                x++;
            }
            if (x > 0) {
                if (++sline > cmd_rows - 3) {
                    if (!askmore()) goto xgrep; else sline = 0;
                }
            }
        }
        bigcount += count;              /* Overall count */
    }
  xgrep:
#ifndef NOSPL
    if (gr_coun && cv) {                /* /COUNT:blah */
        addmac(cv,ckitoa(bigcount));    /* set the variable */
        makestr(&cv,NULL);              /* free this */
    }
#endif /* NOSPL */
    if (fp) fclose(fp);                 /* close input file if still open */
    if (ofp != stdout) {                /* Close any output file */
        if (ofp) fclose(ofp);
        ofp = stdout;
    }
    return(success = mc ? 1 : 0);
}

/* System-independent directory */

static char ** dirlist = NULL;
static int ndirlist = 0;

static VOID
freedirlist() {
    if (dirlist) {
        int i;
        for (i = 0; i < ndirlist; i++) {
            if (dirlist[i])
              free(dirlist[i]);
        }
        free((char *)dirlist);
        dirlist = NULL;
    }
    ndirlist = 0;
}

static struct keytab dirswtab[] = {     /* DIRECTORY command switches */
    { "/after",       DIR_AFT, CM_ARG },
    { "/all",         DIR_ALL, 0 },
#ifndef NOSPL
    { "/array",       DIR_ARR, CM_ARG },
#endif /* NOSPL */
    { "/ascending",   DIR_ASC, 0 },
    { "/backup",      DIR_BUP, 0 },
    { "/before",      DIR_BEF, CM_ARG },
    { "/brief",       DIR_BRF, 0 },
    { "/count",       DIR_COU, CM_ARG },
    { "/descending",  DIR_DSC, CM_INV },
    { "/directories", DIR_DIR, 0 },
    { "/dotfiles",    DIR_DOT, 0 },
    { "/englishdate", DIR_DAT, 0 },
    { "/except",      DIR_EXC, CM_ARG },
    { "/files",       DIR_FIL, 0 },
    { "/heading",     DIR_HDG, 0 },
    { "/isodate",     DIR_ISO, 0 },
    { "/larger-than", DIR_LAR, CM_ARG },
#ifdef CKSYMLINK
    { "/followlinks", DIR_LNK, 0 },
#endif /* CKSYMLINK */
    { "/message",     DIR_MSG, CM_ARG },
    { "/nobackupfiles",DIR_NOB, 0 },
    { "/nodotfiles",  DIR_NOD, 0 },
#ifdef CKSYMLINK
    { "/nofollowlinks",DIR_NLK, 0 },
#endif /* CKSYMLINK */
    { "/noheading",   DIR_NOH, 0 },
#ifdef CKSYMLINK
    { "/nolinks",     DIR_NOL, 0 },
#endif /* CKSYMLINK */
    { "/nomessage",   DIR_NOM, 0 },
#ifdef CK_TTGWSIZ
    { "/nopage",      DIR_NOP, 0 },
#endif /* CK_TTGWSIZ */
#ifdef RECURSIVE
    { "/norecursive", DIR_NOR, 0 },
#else
#ifdef VMS
    { "/norecursive", DIR_NOR, 0 },
#else
#ifdef datageneral
    { "/norecursive", DIR_NOR, 0 },
#endif /* datageneral */
#endif /* VMS */
#endif /* RECURSIVE */
    { "/nosort",      DIR_NOS, 0 },
    { "/not-after",   DIR_NAF, CM_ARG },
    { "/not-before",  DIR_NBF, CM_ARG },
    { "/not-since",   DIR_NAF, CM_INV|CM_ARG },
    { "/noxfermode",  DIR_NOT, 0 },
    { "/output",      DIR_OUT, CM_ARG },
#ifdef CK_TTGWSIZ
    { "/page",        DIR_PAG, 0 },
#endif /* CK_TTGWSIZ */
#ifdef RECURSIVE
    { "/recursive",   DIR_REC, 0 },
#else
#ifdef VMS
    { "/recursive",   DIR_REC, 0 },
#else
#ifdef datageneral
    { "/recursive",   DIR_REC, 0 },
#endif /* datageneral */
#endif /* VMS */
#endif /* RECURSIVE */
    { "/reverse",     DIR_DSC, 0 },
    { "/since",       DIR_AFT, CM_ARG|CM_INV },
    { "/smaller-than",DIR_SMA, CM_ARG },
    { "/sort",        DIR_SRT, CM_ARG },
    { "/summary",     DIR_SUM, 0 },
    { "/top",         DIR_TOP, CM_ARG },
    { "/type",        DIR_BIN, CM_ARG },
    { "/xfermode",    DIR_TYP, 0 },
    { "/verbose",     DIR_VRB, 0 },
    { "",0,0 }
};
static int ndirswtab = (sizeof(dirswtab) / sizeof(struct keytab)) - 1;

static struct keytab dirsort[] = {      /* DIRECTORY /SORT: options */
    { "date",         DIRS_DT, 0 },
    { "name",         DIRS_NM, 0 },
    { "size",         DIRS_SZ, 0 }
};
static int ndirsort = (sizeof(dirsort) / sizeof(struct keytab));

static int dir_date = -1;               /* Option defaults (-1 means none) */
static int dir_page = -1;
static int dir_verb =  1;
static int dir_msg  = -1;
#ifdef VMS
static int dir_sort = -1;               /* Names are already sorted in VMS */
static int dir_rvrs = -1;
#else
static int dir_sort =  1;               /* Sort by default */
static int dir_rvrs =  0;               /* Not in reverse */
#endif /* VMS */
static int dir_skey = DIRS_NM;          /* By name */
#ifdef RECURSIVE
static int dir_recu = -1;
#endif /* RECURSIVE */
static int dir_mode = -1;
static int dir_show = -1;               /* Show all files by default */
int dir_dots =  -1;			/* Except dot files */
int dir_back =  1;
int dir_head =  0;
static char * dirmsg = NULL;
static int dirmsglen = 0;

#ifndef NOSHOW
VOID
showdiropts() {
    int x = 0;
    extern int optlines;
    prtopt(&optlines,"DIRECTORY");
    if (dir_show > 0) {
        prtopt(&optlines,(dir_show == 1) ? "/FILES" :
               ((dir_show == 2) ? "/DIRECTORIES" : "/ALL"));
        x++;
    } else {
        prtopt(&optlines,"/ALL");
        x++;
    }
    if (dir_verb > -1) {
        prtopt(&optlines,dir_verb ? "/VERBOSE" : "/BRIEF");
        x++;
    }
    if (dir_page > -1) {
        prtopt(&optlines,dir_page ? "/PAGE" : "/NOPAGE");
        x++;
    }
    if (dir_date > -1) {
        prtopt(&optlines,dir_date ? "/ENGLISHDATE" : "/ISODATE");
        x++;
    }
    if (dir_dots > -1) {
        prtopt(&optlines,dir_dots ? "/DOTFILES" : "/NODOTFILES");
        x++;
    }
    if (dir_back > -1) {
        prtopt(&optlines,dir_back ? "/BACKUP" : "/NOBACKUP");
        x++;
    }
    if (dir_head > -1) {
        prtopt(&optlines,dir_head ? "/HEADING" : "/NOHEADING");
        x++;
    }
#ifdef RECURSIVE
    if (dir_recu > -1) {
        prtopt(&optlines,dir_recu ? "/RECURSIVE" : "/NORECURSIVE");
        x++;
    }
#endif /* RECURSIVE */
    if (dir_mode > -1) {
        prtopt(&optlines,dir_mode ? "/XFERMODE" : "/NOXFERMODE");
        x++;
    }
    if (dir_sort == 0) {
        x++;
        prtopt(&optlines,"/NOSORT ");
    } else if (dir_sort > 0) {
        x++;
        if (dir_skey == DIRS_NM) s = "/SORT:NAME";
        else if (dir_skey == DIRS_SZ) s = "/SORT:SIZE";
        else if (dir_skey == DIRS_DT) s = "/SORT:DATE";
        prtopt(&optlines,s);
    }
    if (dir_rvrs > -1) {
        prtopt(&optlines,dir_rvrs ? "/REVERSE" : "/ASCENDING");
        x++;
    }
    if (dir_msg > -1) {
        if (dir_msg == 0) {
            prtopt(&optlines,"/NOMESSAGE");
        } else {
            ckmakmsg(tmpbuf,TMPBUFSIZ,"/MESSAGE:{",dirmsg,"}",NULL);
            prtopt(&optlines,tmpbuf);
        }
        x++;
    }
    if (!x) prtopt(&optlines,"(no options set)");
    prtopt(&optlines,"");
}
#endif /* NOSHOW */

int
setdiropts() {                          /* Set DIRECTORY option defaults */
    int xb = -1, xv = -1, xp = -1, xd = -1, xh = -1, xf = -1;
    int xk = -1, xr = -1, xs = -1, xx = -1, xm = -1, xa = -1, xg = -1;
    int getval;
    char c;
    while (1) {
        if ((y = cmswi(dirswtab,ndirswtab,"Switch","",xxstring)) < 0) {
            if (y == -3)
              break;
            else
              return(y);
        }
        c = cmgbrk();
        if ((getval = (c == ':' || c == '=')) && !(cmgkwflgs() & CM_ARG)) {
            printf("?This switch does not take an argument\n");
            return(-9);
        }
        if (!getval && (cmgkwflgs() & CM_ARG)) {
            printf("?This switch requires an argument\n");
            return(-9);
        }
        switch (y) {
          case DIR_BRF: xv = 0; break;
          case DIR_VRB: xv = 1; break;
          case DIR_PAG: xp = 1; break;
          case DIR_NOP: xp = 0; break;
          case DIR_ISO: xd = 0; break;
          case DIR_DAT: xd = 1; break;
          case DIR_HDG: xh = 1; break;
          case DIR_NOH: xh = 0; break;
          case DIR_DOT: xf = 1; break;
          case DIR_NOD: xf = 0; break;
          case DIR_ALL: xa = 3; break;
          case DIR_DIR: xa = 2; break;
          case DIR_FIL: xa = 1; break;
          case DIR_SRT:
            x = DIRS_NM;
            if (getval)
              if ((x = cmkey(dirsort,ndirsort,"Sort key","name",xxstring)) < 0)
                return(x);
            xk = x;
            xs = 1;
            break;
          case DIR_NOS: xs = 0; break;
          case DIR_ASC: xx = 0; break;
          case DIR_DSC: xx = 1; break;
          case DIR_REC: xr = 1; break;
          case DIR_NOR: xr = 0; break;
          case DIR_TYP: xm = 1; break;
          case DIR_NOT: xm = 0; break;
          case DIR_BUP: xb = 1; break;
          case DIR_NOB: xb = 0; break;
          case DIR_NOM: xg = 0; break;
          case DIR_MSG:
            if (getval)
              if ((x = cmfld("Message to append to each line",
                             "",
                             &s,
                             xxstring
                             )) < 0)
                return(x);
            xg = 1;
            ckstrncpy(tmpbuf,brstrip(s),TMPBUFSIZ);
            break;
          default:
            printf("?This option can not be set\n");
            return(-9);
        }
    }
    if ((x = cmcfm()) < 0)              /* Get confirmation */
      return(x);
    if (xv > -1) dir_verb = xv;         /* Confirmed, save defaults */
    if (xp > -1) dir_page = xp;
    if (xd > -1) dir_date = xd;
    if (xh > -1) dir_head = xh;
    if (xs > -1) dir_sort = xs;
    if (xk > -1) dir_skey = xk;
    if (xx > -1) dir_rvrs = xx;
    if (xf > -1) dir_dots = xf;
    if (xa > -1) dir_show = xa;
    if (xm > -1) dir_mode = xm;
    if (xb > -1) dir_back = xb;
#ifdef RECURSIVE
    if (xr > -1) dir_recu = xr;
#endif /* RECURSIVE */
    if (xg > -1) dir_msg  = xg;
    if (xg > 0)
      makestr(&dirmsg,tmpbuf);
    return(success = 1);
}

int
domydir(cx) int cx; {			/* Internal DIRECTORY command */
    extern char *months[];
#ifdef VMS
    _PROTOTYP( char * zrelname, (char *,char *) );
    char * cdp = NULL;
#endif /* VMS */

    char name[CKMAXPATH+1], outfile[CKMAXPATH+1], *p = NULL, c = NUL;
    char linebuf[CKMAXPATH+256];
    char * mstr = NULL, * dstr = NULL, * s2 = NULL, * cv = NULL;
    CK_OFF_T len = (CK_OFF_T)0, nbytes = (CK_OFF_T)0;
    CK_OFF_T minsize = (CK_OFF_T)-1, maxsize = (CK_OFF_T)-1;
    long ndirs = 0, nfiles = 0, nmatches = 0;
    int verbose = 0, wild = 0, page = 0, n = 0, engdate = 0, summary = 0;
    int heading = 0, xsort = 0, reverse = 0, sortby = 0, msg = 0;
    int k, i = 0, x = 0, nx = 0, skey = 0, dlen = 0, itsadir = 0;
    int show = 3, xfermod = 0, backup = 1, rc = 0, getval = 0;
    int touch = 0;
    int fs = 0;
    int multiple = 0;
    int cmifn1 = 1, cmifn2 = 0;
    int dir_top = 0, dir_cou = 0;
    int dontshowlinks = 0;
    int dontfollowlinks = 0;
    int arrayindex = -1;
    struct FDB sw, fi, fl;
    char dbuf[32], xbuf[32];

#ifndef NOSPL
    char array = NUL;
    char ** ap = NULL;
#endif /* NOSPL */
    char
      * dir_aft = NULL,
      * dir_bef = NULL,
      * dir_naf = NULL,
      * dir_nbf = NULL,
      * dir_exc = NULL;
    char * xlist[16];

    debug(F101,"domydir cx","",cx);

    g_matchdot = matchdot;              /* Save global matchdot setting */
#ifdef COMMENT
    nolinks = 2;                        /* (it should already be 2) */
#endif	/* COMMENT */
    outfile[0] = NUL;                   /* No output file yet */

    if (ofp != stdout) {                /* In case of previous interruption */
        if (ofp) fclose(ofp);
        ofp = stdout;
    }
    for (i = 0; i < 16; i++) xlist[i] = NULL;

    dir_top = 0;
    name[0] = NUL;
    freedirlist();                      /* In case not freed last time */
    page      = dir_page > -1 ? dir_page : xaskmore; /* Set option defaults */
    engdate   = dir_date > -1 ? dir_date : 0;
    verbose   = dir_verb > -1 ? dir_verb : 1;
    heading   = dir_head > -1 ? dir_head : 0;
    xsort     = dir_sort > -1 ? dir_sort : 0;
    sortby    = dir_skey > -1 ? dir_skey : 0;
    reverse   = dir_rvrs > -1 ? dir_rvrs : 0;
    msg       = dir_msg  > -1 ? dir_msg  : 0;
#ifdef UNIXOROSK
    if (dir_dots > -1) matchdot = dir_dots;
#endif /* UNIXOROSK */
    xfermod   = dir_mode > -1 ? dir_mode : 0;
    backup    = dir_back > -1 ? dir_back : 1;
#ifdef RECURSIVE
    recursive = dir_recu > -1 ? dir_recu : 0;
#endif /* RECURSIVE */
    show      = dir_show > -1 ? dir_show : 3;

    if (cx == XXWDIR) {			/* WDIRECTORY */
	debug(F100,"domydir WDIRECTORY","",0);
	reverse = 1;			/* Reverse chronological order */
	xsort = 1;
	sortby = DIRS_DT;
    } else if (cx == XXHDIR) {		/* HDIRECTORY */
	debug(F100,"domydir HDIRECTORY","",0);
	reverse = 1;			/* Reverse order by size */
	xsort = 1;
	sortby = DIRS_SZ;
    } else if (cx == XXTOUC) {
	touch = 1;
	verbose = 0;
    }	

#ifdef CK_TTGWSIZ
#ifdef OS2
    ttgcwsz();                          /* Screen length for more-prompting */
#else /* OS2 */
    /* Check whether window size changed */
    if (ttgwsiz() > 0) {
        if (tt_rows > 0 && tt_cols > 0) {
            cmd_rows = tt_rows;
            cmd_cols = tt_cols;
        }
    }
#endif /* OS2 */
#endif /* CK_TTGWSIZ */

    diractive = 1;
    cmifn1 = nolinks | 1;               /* 1 = files or directories */
    cmifn2 = 0;                         /* 0 = not directories only */

  again:

    cmfdbi(&sw,                         /* First FDB - command switches */
           _CMKEY,                      /* fcode */
           "Enter or Return to confirm the command, or\n\
 file specification, or switch",
           "",                          /* default */
           "",                          /* addtl string data */
           ndirswtab,                   /* addtl numeric data 1: tbl size */
           4,                           /* addtl numeric data 2: 4 = cmswi */
           xxstring,                    /* Processing function */
           dirswtab,                    /* Keyword table */
           &fi                          /* Pointer to next FDB */
           );
    cmfdbi(&fi,                         /* 2nd FDB - filespec to match */
           _CMIFI,                      /* fcode */
           "File specification",        /* hlpmsg */
#ifdef datageneral
           "+",                         /* Default filespec is wildcard */
#else                                   /* that matches all files... */
#ifdef VMS
           "*.*",
#else
           "*",
#endif /* VMS */
#endif /* datageneral */
           "",                          /* addtl string data */
           cmifn1,
           cmifn2,                      /* 1 = only dirs; 0 files or dirs */
           xxstring,
           NULL,
           &fl
           );
    cmfdbi(&fl,                         /* Anything that doesn't match */
           _CMFLD,                      /* fcode */
           "",                          /* hlpmsg */
           "",                          /* default */
           "",                          /* addtl string data */
           0,                           /* addtl numeric data 1 */
           0,                           /* addtl numeric data 2 */
           xxstring,
           NULL,
           NULL
           );
    while (1) {                         /* Parse 0 or more switches */
        x = cmfdb(&sw);                 /* Parse something */
        debug(F101,"domydir cmfdb","",x);
        if (x < 0)
          return(x); 
        if (cmresult.fcode != _CMKEY)   /* Break out if not a switch */
          break;
        c = cmgbrk();
        if ((getval = (c == ':' || c == '=')) && !(cmgkwflgs() & CM_ARG)) {
            printf("?This switch does not take an argument\n");
            return(-9);
        }
	k = cmresult.nresult;
        if (!getval &&
	    (cmgkwflgs() & CM_ARG) && k != DIR_TOP && k != DIR_COU) {
            printf("?This switch requires an argument\n");
            return(-9);
        }
        switch (k) {
          case DIR_COU: {
              dir_cou++;
              if (getval) {
                  if ((x = cmfld("Variable for result","",&s,NULL)) < 0)
                    return(x);
                  makestr(&cv,s);
              }
              break;
          }
          case DIR_BRF: verbose = 0; break;
          case DIR_VRB: verbose = 1; break;
#ifdef CK_TTGWSIZ
          case DIR_PAG: page = 1;    break;
          case DIR_NOP: page = 0;    break;
#endif /* CK_TTGWSIZ */
          case DIR_ISO: engdate = 0; break;
          case DIR_DAT: engdate = 1; break;
          case DIR_HDG: heading = 1; break;
          case DIR_NOH: heading = 0; break;
#ifdef UNIXOROSK
          case DIR_DOT: matchdot = 1; break;
          case DIR_NOD: matchdot = 0; break;
#endif /* UNIXOROSK */
          case DIR_ALL:
            show = 3;
            cmifn1 |= 1;
            cmifn2 = 0;
            goto again;
          case DIR_DIR:
            show = 2;
            cmifn1 |= 1;
            cmifn2 = 1;
            goto again;
          case DIR_FIL:
            show = 1;
            cmifn1 &= ~(1);
            cmifn2 = 0;
            goto again;
          case DIR_SRT:
            x = DIRS_NM;
            if (c == ':' || c == '=')
              if ((x = cmkey(dirsort,ndirsort,"Sort key","name",xxstring)) < 0)
                return(x);
            xsort = 1;
            sortby = x;
            break;

          case DIR_BUP: backup  = 1; fs++;   break;
          case DIR_NOB: backup  = 0; fs++;   break;

          case DIR_NOS: xsort = 0;     break;
          case DIR_ASC: reverse = 0;   break;
          case DIR_DSC: reverse = 1;   break;
#ifdef RECURSIVE
          case DIR_REC: recursive = 1; diractive = 1; break;
          case DIR_NOR: recursive = 0; diractive = 0; break;
#endif /* RECURSIVE */
          case DIR_TYP: xfermod = 1;   break;
          case DIR_NOT: xfermod = 0;   break;

#ifdef CKSYMLINK
          case DIR_LNK:                 /* Follow links */
#ifdef COMMENT
	    /* A command switch shouldn't be setting a global value! */
            nolinks = 0;
#endif	/* COMMENT */
            cmifn1 &= ~(2);
	    dontfollowlinks = 0;
            goto again;
          case DIR_NLK:                 /* Don't follow links */
#ifdef COMMENT
            nolinks = 2;
#endif	/* COMMENT */
            cmifn1 &= ~(2);
	    dontfollowlinks = 1;
            goto again;
	  case DIR_NOL:			/* Don't show links at all */
	    dontshowlinks = 1;
	    goto again;
#endif /* CKSYMLINK */

          case DIR_NOM: msg     = 0;   break;
          case DIR_MSG:
            if (c == ':' || c == '=')
              if ((x = cmfld("Message to append to each line",
                             "",
                             &s,
                             xxstring
                             )) < 0)
                return(x);
            msg = 1;
            ckstrncpy(tmpbuf,brstrip(s),TMPBUFSIZ);
            break;

          case DIR_SMA:
          case DIR_LAR: {
	      CK_OFF_T y;
	      if (!getval) break;
	      if ((x = cmnumw("File size in bytes","0",10,&y,xxstring)) < 0)
		return(x);
	      fs++;
	      show = 1;
	      switch (cmresult.nresult) {
		case DIR_SMA: minsize = y; break;
		case DIR_LAR: maxsize = y; break;
	      }
	      break;
	  }
	  case DIR_TOP:
	    dir_top = 10;
	    if (!getval) break;
	      if ((x = cmnum("How many lines to show","10",10,&y,xxstring))< 0)
		return(x);
	      dir_top = y;
	      break;

#ifndef NOSPL
          case DIR_ARR:
            if (c != ':' && c != '=') {
                printf("?Array name required\n");
                return(-9);
            }
            if ((x = cmfld("Array name (a single letter will do)",
                           "",
                           &s,
                           NULL
                           )) < 0) {
                if (x == -3) {
                    printf("?Array name required\n");
                    return(-9);
                } else
                  return(x);
            }
            if (!*s) {
                printf("?Array name required\n");
                return(-9);
            }
            s2 = s;
            if (*s == CMDQ) s++;
            if (*s == '&') s++;
            if (!isalpha(*s)) {
                printf("?Bad array name - \"%s\"\n",s2);
                return(-9);
            }
            array = *s++;
            if (isupper(array)) array = tolower(array);
            if (*s && (*s != '[' || *(s+1) != ']')) {
                printf("?Bad array name - \"%s\"\n",s2);
                return(-9);
            }
            break;
#endif /* NOSPL */
          case DIR_AFT:
          case DIR_BEF:
          case DIR_NAF:
          case DIR_NBF:
            if (!getval) break;
            if ((x = cmdate("File-time","",&s,0,xxstring)) < 0) {
                if (x == -3) {
                    printf("?Date-time required\n");
                    rc = -9;
                } else
                  rc = x;
                goto xdomydir;
            }
            fs++;
            switch (k) {
              case DIR_AFT: makestr(&dir_aft,s); break;
              case DIR_BEF: makestr(&dir_bef,s); break;
              case DIR_NAF: makestr(&dir_naf,s); break;
              case DIR_NBF: makestr(&dir_nbf,s); break;
            }
            break;
          case DIR_EXC:
            if (!getval) break;
            if ((x = cmfld("Pattern","",&s,xxstring)) < 0) {
                if (x == -3) {
                    printf("?Pattern required\n");
                    rc = -9;
                } else
                  rc = x;
                goto xdomydir;
            }
            fs++;
            makestr(&dir_exc,s);
            break;

          case DIR_SUM:
            summary = 1; break;

          case DIR_BIN: {
              extern struct keytab txtbin[];
              extern int xfiletype;
              if (!getval) break;
              if ((x = cmkey(txtbin,3,"","all",xxstring)) < 0) {
                  rc = x;
                  goto xdomydir;
              }
              if (x == 2) {
                  xfiletype = -1;
              } else {
                  xfiletype = x;
                  fs = 1;
              }
              break;
          }
          case DIR_OUT:
            if ((x = cmofi("File for directory listing","",&s,xxstring)) < 0)
              return(x);
            ckstrncpy(outfile,s,CKMAXPATH+1);
            break;

          default:
            printf("?Sorry, not implemented yet - \"%s\"\n", atmbuf);
            goto xdomydir;
        }
    }
    ckstrncpy(line,cmresult.sresult,LINBUFSIZ); /* Safe copy of filespec */

/* ^^^ START MULTIPLE */
    
    while (!touch) {
	x = cmfld("Another filespec or Enter","",&s,xxstring);
	if (x == -3)
	  break;
	if (x < 0)
	  return(x);
	ckstrncat(line,",",LINBUFSIZ);
	ckstrncat(line,s,LINBUFSIZ);
	multiple++;
    }
    ckmakmsg(tmpbuf,TMPBUFSIZ,"{",line,"}",NULL);
    ckstrncpy(line,tmpbuf,LINBUFSIZ);
    cmresult.nresult = 1;
    cmresult.fcode = _CMIFI;

/* ^^^ END MULTIPLE */

    s = line;

    if ((x = cmcfm()) < 0)              /* Get confirmation */
      return(x);

/*
  Command is TOUCH and file doesn't exist.
*/
    if (touch) {			/* TOUCH */
	if ((cmresult.fcode == _CMIFI && zchki(s) == (CK_OFF_T)-1)) {
	    FILE * fp;
	    s = brstrip(s);
	    if (!iswild(s)) {
#ifdef UNIX
		if (s[0] == '~')
		  s = tilde_expand(s);
#endif	/* UNIX */
		fp = fopen(s,"w");	/* Create file */
		if (!fp) {
		    printf("?TOUCH %s: %s\n",s,ck_errstr());
		    rc = -9;
		    goto xdomydir;
		}
		fclose(fp);
		cx = XXDIR;		/* Now maybe list it. */
		multiple++;		/* Force new directory scan */
	    }
	}
    } else

    if (cmresult.fcode != _CMIFI) {     /* Nothing matched */
	/*
	  Note - this never gets executed because after the "begin
	  multiple" hack above, the result is always _CMIFI).
	*/
        char * m;
	if (*s == '/')
#ifdef UNIXOROSK
	  m = "does not match switch or name of accessible file";
#else
#ifdef OS2
	m = "does not match switch or name of accessible file";
#else
	m = "no switches match";
#endif /* OS2 */
#endif /* UNIXOROSX */
	else
	  m = "not found or not accessible";
	printf("\"%s\" - %s\n",s,m);
	rc = -9;
	goto xdomydir;
    }
#ifdef COMMENT
    /* This can't be right because it's based on _CMCFM */
    wild = cmresult.nresult;            /* Wildcard was given? */
    debug(F111,"domydir cmifi2",s,wild);
#else
    wild = 0;
#endif	/* COMMENT */

    if (outfile[0]) {			/* If an output file was specified */
        ofp = fopen(outfile,"w");       /* open it */
        if (!ofp) {
            printf("?Can't open output file %s: %s\n",outfile,ck_errstr());
            ofp = stdout;
            return(-9);
        }
        page = 0;
    }

#ifdef OS2
    if (!wild) {
        if (zchki(s) == -2) {           /* Found a directory */
            p = s + (int)strlen(s) - 1; /* Yes */
            if (*p == '\\' || *p == '/')
              strcat(s, "*");
            else if (*p == ':')
              strcat(s, "./*");
            else
              strcat(s, "/*");
            wild = 1;                   /* Now it's wild */
        }
    }
#else
    if (!wild) if (isdir(s)) {          /* Is it a directory? */
        p = s + (int)strlen(s) - 1;     /* Yes */
#ifdef VMS
        {
            /* Convert from FOO.DIR;1 to [x.FOO] if necessary */
            char buf[CKMAXPATH+1];
            debug(F000,"domydir directory 0",s,*p);
            if (cvtdir(s,buf,CKMAXPATH) > 0)
              ckstrncpy(line,buf,LINBUFSIZ);
        }
#endif /* VMS */
        debug(F000,"domydir directory 1",s,*p);
#ifdef VMS
        if (*p == ']' || *p == '>' || *p == ':')
          strcat(s, "*.*");
#else
#ifdef datageneral
        if (*p == ':')
          strcat(s, "+");
        else
          strcat(s, ":+");
#else
#ifdef STRATUS
        if (*p == '>')
          strcat(s, "*");
        else
          strcat(s, ">*");
#endif /* STRATUS */
#endif /* datageneral */
#endif /* VMS */
        wild = 1;                       /* Now it's wild */
        debug(F000,"domydir directory 2",s,*p);
    }
#endif /* OS2 */

#ifdef ZXREWIND
/* cmifi() already called nzxpand so we can just re-use the same list. */
    if (!multiple) {
	x = zxrewind();			/* Rewind the list */
	debug(F111,"domydir zxrewind",s,x);
    } else {
#endif /* ZXREWIND */
/*
  In case we gave multiple filespecs they are now in {a,b,c} list format.
  Which is a valid wildcard.  We pass it to nzxpand() to get back the list
  of files that match.  This is fine for DIRECTORY but it's not find for
  TOUCH because we want TOUCH to see those names so it can create the files.
  So for now at least, if TOUCH is to be used to create files -- as opposed
  to changing the timestamps of existing files -- it can only do one file
  at a time.
*/
	nzxopts = (show == ZX_DIRONLY) ? ZX_DIRONLY :
	  (show == ZX_FILONLY ? ZX_FILONLY : 0);
	if (matchdot)  nzxopts |= ZX_MATCHDOT;
	if (recursive) nzxopts |= ZX_RECURSE;
	x = nzxpand(s,nzxopts);             /* Expand file list */
	debug(F111,"domydir nzxpand",s,x);
#ifdef ZXREWIND
    }
#endif /* ZXREWIND */

#ifndef NOSPL
    if (array) {
        int n, xx;
        n = (x < 0) ? 0 : x;
        if ((xx = dclarray(array,n)) < 0) {
            printf("?Array declaration failure\n");
            rc = -9;
            goto xdomydir;
        }
	arrayindex = xx;
        ap = a_ptr[xx];			/* Pointer to list of elements */
        if (ap)				/* Set element 0 to dimension */
          makestr(&(ap[0]),"0");	/* which so far is zero */
        if (n < 1) {			/* No files matched, done. */
            rc = 0;
            goto xdomydir;
        }
    } else
#endif /* NOSPL */
      if (!touch && x < 1) {
#ifdef CKROOT
          extern int ckrooterr;
          if (ckrooterr)
            printf("?Off limits: %s\n",s);
          else
#endif /* CKROOT */
            if (x == 0 && isdir(s))
              printf("?Empty directory - \"%s\"\n", s);
            else
              printf("?%s %s match - \"%s\"\n",
                     (x == 0) ? "No" : "Too many",
                     (show == 2) ? "directories" : "files",
                     brstrip(s)
                     );
          rc = -9;
          goto xdomydir;
    }
    nx = x;                             /* Remember how many files */
    if (nx < 2) xsort = 0;		/* Skip sorting if none or one */

    if (msg) {
        makestr(&dirmsg,tmpbuf);
        dirmsglen = strlen(tmpbuf);
    }

#ifdef VMS
    cdp = zgtdir();                     /* Get current directory */
    debug(F110,"domydir VMS zgtdir",cdp,0);
#endif /* VMS */

    if (xsort && verbose) {             /* If sorting, allocate space */
        if (!(dirlist = (char **) malloc((x + 1) * sizeof(char **)))) {
            if (!quiet) {
                printf("* Warning: Failure to allocate memory for sorting.\n");
                printf("* Will proceed without sorting...\n");
            }
            xsort = 0;
        }
        debug(F101,"domydir sort malloc","",xsort);
    }

    /* Display the listing */

#ifndef NOSPL
    if (array)                          /* Storing instead of printing */
      heading = 0;
#endif /* NOSPL */

    if (heading) {                      /* If /HEADING print heading */
        zfnqfp(s,TMPBUFSIZ,tmpbuf);
        fprintf(ofp,"\nDirectory of %s\n\n",tmpbuf);
        n += 3;
    }
    if (page > -1)                      /* Paging */
      xaskmore = page;

    if (!verbose && !touch) {		/* /BRIEF */
        if (outfile[0]) {               /* To file  */
            int k = 0;
            znext(name);
            while (name[0]) {           /* One line per file */
                k++;
                if (fs) if (fileselect(name,
                                       dir_aft,dir_bef,dir_naf,dir_nbf,
                                       minsize,maxsize,!backup,16,xlist) < 1) {
                    znext(name);
                    continue;
                }
                fprintf(ofp,"%s\n",name);
                znext(name);
            }
            if (heading)
              fprintf(ofp,"Files: %d\n\n",k);
            rc = 1;
            goto xdomydir;
        } else {
            rc = xfilhelp(x,"","",n,0,1,
			  dir_aft,dir_bef,dir_naf,dir_nbf,
			  minsize,maxsize,!backup,16,xlist);
            if (rc < 0)
              goto xdomydir;
            if (heading && rc > 0)
              fprintf(ofp,"Files: %d\n\n",x); /* (Might scroll a line or 2) */
            rc = 1;
            goto xdomydir;
        }
    }
    ndirs = nfiles = 0L;		/* Initialize counters */
    nbytes = (CK_OFF_T)0;

    if (dir_exc)                        /* Have exception list? */
      makelist(dir_exc,xlist,16);	/* Yes, convert to array */

    diractive = 1;
    znext(name);                        /* Get next file */
    while (name[0]) {                   /* Loop for each file */
        if (fs) if (fileselect(name,
                       dir_aft,dir_bef,dir_naf,dir_nbf,
                       minsize,maxsize,!backup,16,xlist) < 1) {
            znext(name);
            continue;
        }
        len = zgetfs(name);             /* Get file length */
        debug(F111,"domydir zgetfs",name,len);
#ifdef VMSORUNIX
        itsadir = zgfs_dir;             /* See if it's a directory */
#else
        itsadir = (len == (CK_OFF_T)-2 || isdir(name));
#endif /* VMSOUNIX */
        debug(F111,"domydir itsadir",name,itsadir);
        if ((itsadir && (show == 1)) || (!itsadir && (show == 2))) {
            znext(name);
            continue;
        }
        /* Get here when we know we have selected this file */

        nmatches++;
        if (itsadir) {                  /* Accumulate totals for summary */
            ndirs++;
        } else {
            nfiles++;
            nbytes += len;
        }
	dstr = NULL;
	if (cx == XXTOUC) {		/* Command was TOUCH, not DIRECTORY */
	    char * filename;
	    struct zattr xx;
	    dstr = ckcvtdate("",0);
	    xx.date.val = dstr;
	    xx.date.len = (int)strlen(xx.date.val);
	    xx.lprotect.len = 0;
	    debug(F110,"domydir touch",name,0);
	    debug(F110,"domydir touch",dstr,0);
	    if (zstime(name,&xx,0) < 0) {
		printf("?TOUCH %s: %s\n",name,ck_errstr());
		rc = -9;
		goto xdomydir;
	    }
	    if (!verbose) {		/* No listing so skip detail */
		znext(name);
		continue;
	    }
	}
        if (summary) {			/* Summary only, no detail */
            znext(name);
            continue;
        }

#ifndef NOSPL
        if (array) {
            debug(F111,"domydir array",name,nfiles);
            if (ap)
              makestr(&(ap[nmatches]),name);
            znext(name);
            continue;
        }
#endif /* NOSPL */

/*
  NOTE: The sprintf's in this routine should be safe.  They involve
  permission strings, date/time strings, and filenames, all of which have
  known maximum lengths; none of these items is input from users.  The
  destination buffers are large enough to hold maximum sizes for any and
  all items.  NOTE 2: If command was TOUCH, dstr was already set just
  above.
*/
	if (!dstr) {			/* Get file's modification date/time */
	    dstr = zfcdat(name);
	    debug(F111,"domydir zcfdat",dstr,0);
	}
        if (!dstr) dstr = "";
        {
/*
  Note that zfcdat() always returns "" or yyyymmdd hh:mm:ss, so any warnings
  about possible out-of-bounds dstr[] array refs do not apply.  This block of
  code is to stifle the warnings and also allows for any out-of-spec
  zfcdat() implementations.
*/
            int x;
            char * p = "00000000 00:00:00";
            x = ckstrncpy(xbuf,dstr,32);
            if (x < 17) ckstrncpy(&xbuf[x],p+x,32-x);
            dstr = xbuf;
        }
        if (engdate) {                  /* English date requested? */
            short month, day, year, hour, minute, seconds;
            month = (dstr[4]-48)*10 + (dstr[5]-48);
            mstr  = (month > 0 && month <= 12) ? months[month-1] : "xxx";
            day   = (dstr[6]-48)*10 + (dstr[7]-48);
            year  = (((dstr[0]-48)*10 +
                      (dstr[1]-48))*10 +
                      (dstr[2]-48))*10 +
                      (dstr[3]-48);
            hour  = (dstr[9]-48)*10 + (dstr[10]-48);
            minute = (dstr[12]-48)*10 + (dstr[13]-48);
            seconds = (dstr[15]-48)*10 + (dstr[16]-48);
            sprintf(dbuf,               /* SAFE */
                    "%2d-%s-%4d %02d:%02d:%02d",
                    day,mstr,year,hour,minute,seconds
                    );
            dstr = dbuf;
        } else {                        /* ISO date */
            dbuf[0] = dstr[0];          /* yyyy */
            dbuf[1] = dstr[1];
            dbuf[2] = dstr[2];
            dbuf[3] = dstr[3];
            dbuf[4] = '-';
            dbuf[5] = dstr[4];          /* mm (numeric) */
            dbuf[6] = dstr[5];
            dbuf[7] = '-';
            dbuf[8] = dstr[6];          /* dd */
            dbuf[9] = dstr[7];
            strcpy(dbuf+10,dstr+8);     /* hh:mm:ss */
            dstr = dbuf;
        }
        dlen = strlen(dbuf);            /* Length of date */
        name[CKMAXPATH] = NUL;
#ifdef CK_PERMS
#ifdef VMSORUNIX
        p = ziperm(name);               /* Get permissions */
        debug(F110,"ziperm perms",p,0);
#else
        p = zgperm(name);
        debug(F110,"zgperm perms",p,0);
#endif /* VMSORUNIX */
#else
        p = NULL;
        debug(F110,"NULL perms",p,0);
#endif /* CK_PERMS */

#ifdef VMS
        /* Get relative name to save space -- VMS fullnames are long... */
        ckstrncpy(name,zrelname(name,cdp),CKMAXPATH);
#endif /* VMS */

        if (itsadir && len < (CK_OFF_T)0) { /* Directory */
#ifdef VMS
            sprintf(linebuf,"%-22s%-10s  %s  %s",p,"<DIR>",dstr,name);
#else
            if (p)
              sprintf(linebuf,"%10s%-10s  %s  %s",p,"<DIR>",dstr,name);
            else
              sprintf(linebuf,"%-10s  %s  %s", "<DIR>", dstr, name);
#endif /* VMS */
        } else {                        /* Regular file */
#ifdef VMS
            sprintf(linebuf,"%-22s%10s  %s  %s", p, ckfstoa(len), dstr, name);
#else
            if (p)
              sprintf(linebuf,"%10s%10s  %s  %s", p, ckfstoa(len), dstr, name);
            else
              sprintf(linebuf,"%10s  %s  %s", ckfstoa(len), dstr, name);
#endif /* VMS */
        }
#ifdef UNIX
#ifdef CKSYMLINK
        if (zgfs_link) {		/* If it's a symlink */
	    if (dontshowlinks) {	/* If /NOLINKS don't show it */
		znext(name);
		continue;
	    }
	}
        if (zgfs_link && !dontfollowlinks) { /* Symlink and following links */
            int n, m;			/* Show what the link points to */
            extern char linkname[];
            n = strlen(linebuf);
            m = strlen(linkname) + n;
            if (m < CKMAXPATH + 58)
              strcpy(linebuf+n, " -> "); /* safe (checked) */
            if (m + 4 < CKMAXPATH - 58)
              strcpy(linebuf+n+4, linkname); /* safe (checked) */
        } else
#endif /* CKSYMLINK */
#endif /* UNIX */
        if (xfermod) {                  /* Show transfer mode */
            int i, x, y;
            char * s = "";
            y = -1;
            x = scanfile(name,&y,nscanfile);
            switch (x) {
              case FT_TEXT: s = " (T)"; break;
              case FT_7BIT: s = " (T)(7BIT)"; break;
              case FT_8BIT: s = " (T)(8BIT)"; break;
#ifdef UNICODE
              case FT_UTF8: s = " (T)(UTF8)"; break;
              case FT_UCS2:
                s = y ? " (T)(UCS2LE)" : " (T)(UCS2BE)";
                break;
#endif /* UNICODE */
              case FT_BIN:  s = " (B)"; break;
            }
            if (!*s) {
                s = binary ? " (B)" : " (T)";
            }
            if (*s) {
                int n;
                n = strlen(linebuf);
                if (n + 4 < CKMAXPATH - 58)
                  strcpy(linebuf+n, s); /* safe (checked) */
            }
        }
        if (msg && dirmsg) {
            int n;
            n = strlen(linebuf);
            if (n + dirmsglen + 2 < CKMAXPATH)
              sprintf((char *)(linebuf+n)," %s", dirmsg); /* SAFE */
        }
        if (xsort) {			/* Sorting - save line */
            i = strlen(linebuf);
            if ((ndirlist >= nx) ||
                !(dirlist[ndirlist] = (char *)malloc(i+1))) {
                printf("?Memory allocation error - try /NOSORT\n");
                rc = -9;
                goto xdomydir;
            }
            strcpy(dirlist[ndirlist],linebuf); /* safe */
            ndirlist++;
        }
        znext(name);                    /* Peek ahead to next file */
        if (!xsort) {
	    if (!touch || (touch && verbose))
	      fprintf(ofp,"%s\n",linebuf);
            if (page && (name[0] || heading)) { /* If /PAGE */
                if (cmd_cols > 0) {
                    int x = strlen(linebuf);
                    int y;
                    y = (x % cmd_cols) ? 1 : 0;
                    n += x / cmd_cols + y;
                } else {
                    n++;
                }
#ifdef CK_TTGWSIZ
                if (n > (cmd_rows - 3)) { /* Do more-prompting */
                    if (!askmore()) {
                        rc = 0;
                        goto xdomydir;
                    } else
                      n = 0;
                }
#endif /* CK_TTGWSIZ */
            }
        }
    }
#ifndef NOSPL
    if (array) {
        if (ap)
          makestr(&(ap[0]),ckitoa(nmatches));
        rc = 1;
        goto xdomydir;
    }
#endif /* NOSPL */
    if (xsort) {
	int namepos;
        skey = 0;
#ifdef VMS
	namepos = dlen + 35;
        switch (sortby) {
          case DIRS_NM: skey = namepos; break;
          case DIRS_DT: skey = 33; break;
          case DIRS_SZ: skey = 21;
        }
#else
        if (p) {
	    namepos = dlen + 24;
            switch (sortby) {
              case DIRS_NM: skey = namepos; break;
              case DIRS_DT: skey = 22; break;
              case DIRS_SZ: skey = 10;
            }
        } else {
	    namepos = dlen + 14;
            switch (sortby) {
              case DIRS_NM: skey = namepos; break;
              case DIRS_DT: skey = 12; break;
              case DIRS_SZ: skey = 0;
            }
        }
#endif /* VMS */
        sh_sort(dirlist,NULL,ndirlist,skey,reverse,filecase);
	if (dir_top > 0 && dir_top < ndirlist)
	  ndirlist = dir_top;
        for (i = 0; i < ndirlist; i++) {
            fprintf(ofp,"%s\n",dirlist[i]);
            if (page && (i < ndirlist -1 || heading)) { /* If /PAGE */
                if (cmd_cols > 0) {
                    int x = strlen(dirlist[i]);
                    int y;
                    y = (x % cmd_cols) ? 1 : 0;
                    n += ((int)strlen(dirlist[i]) / cmd_cols) + y;
                } else {
                    n++;
                }
#ifdef CK_TTGWSIZ
                if (n > (cmd_rows - 3)) { /* Do more-prompting */
                    if (!askmore()) {
                        rc = 0;
                        goto xdomydir;
                    } else
                      n = 0;
                }
#endif /* CK_TTGWSIZ */
            }
        }
    }

    if (heading || summary) {
#ifdef CKFLOAT
        CKFLOAT gm;
#endif /* CKFLOAT */
        fprintf(ofp,"\n%ld director%s, %ld file%s, %s byte%s",
               ndirs,
               (ndirs == 1) ? "y" : "ies",
               nfiles,
               (nfiles == 1) ? "" : "s",
	       ckfstoa(nbytes),
               (nbytes == 1) ? "" : "s"
               );
#ifdef CKFLOAT
        gm = ((CKFLOAT) nbytes ) / 1000000.0;
        if (gm > 1000.0)
          fprintf(ofp," (%0.2fGB)",(gm / 1000.0));
        else if (gm >= 0.01)
          fprintf(ofp," (%0.2fMB)",gm);
#endif /* CKFLOAD */
        fprintf(ofp,"\n\n");
    } else if (dir_cou && !cv) {
	fprintf(ofp,"\n Files: %ld\n\n",nfiles);
    }
  xdomydir:
#ifndef NOSPL
    if (dir_cou && cv) {                /* /COUNT:var */
        addmac(cv,ckitoa(nfiles));	/* set the variable */
        makestr(&cv,NULL);              /* free this */
    }
    if (ap) {				/* If we have a result array */
	if (a_dim[arrayindex] > nmatches) /* but it was not filled */
	  a_dim[arrayindex] = nmatches;   /* adjust dimension */
    }
#endif	/* NOSPL */
    if (g_matchdot > -1) {
        matchdot = g_matchdot;          /* Restore these... */
        g_matchdot = -1;
    }
    freedirlist();
    if (ofp != stdout) {                /* Close any output file */
        if (ofp) fclose(ofp);
        ofp = stdout;
    }
    if (rc > 0)
      success = 1;
    return(rc);
}

int
dodir(cx) int cx; {                     /* Do the DIRECTORY command */
    char *dc , *msg;

#ifdef OS2
    return(domydir(cx));
#else /* OS2 */
    if (nopush
#ifdef DOMYDIR                          /* Builds that domydir() by default */
        || (cx == XXDIR  || cx == XXLDIR || cx == XXWDIR ||
	    cx == XXHDIR || cx == XXTOUC)
#endif /* DOMYDIR */
        )
      return(domydir(cx));		/* Built-in directory command */

    /* Use the system's directory command. */

    msg = (cx == XXLS) ?
      "Arguments for ls" :
        "Directory and/or file specification";
    if ((x = cmtxt(msg,"",&s,xxstring)) < 0)
      return(x);

    ckstrncpy(tmpbuf,s,TMPBUFSIZ);      /* Copy the filespec */
    s = tmpbuf;

    if ((y = cmcfm()) < 0) return(y);

    lp = line;
    if (!(dc = getenv("CK_DIR")))
      dc = DIRCMD;
    ckmakmsg(lp,LINBUFSIZ,dc," ",s,NULL);
    debug(F110,"DIR",line,0);
#ifdef VMS
    conres();
#endif /* VMS */
    x = zshcmd(line);
#ifdef VMS
    concb((char)escape);
#endif /* VMS */
    return(success = (x < 1) ? 0 : 1);
#endif /* OS2 */
}

#ifndef NOSERVER
#ifndef NOFRILLS
/* Do the ENABLE and DISABLE commands */

int
doenable(y,x) int y, x; {
#ifdef CK_LOGIN
    if (isguest)                        /* IKSD: Don't let guests */
      return(0);                        /* enable anything that's disabled */
#endif /* CK_LOGIN */
    switch (x) {
      case EN_ALL:
        en_cwd = en_cpy = en_del = en_dir = en_fin = en_get = y;
        en_ren = en_sen = en_set = en_spa = en_typ = en_ret = y;
        if (!inserver)
          en_who = en_mai = en_pri = y;
        en_mkd = en_rmd = y;
        en_xit = y;
#ifndef datageneral
        en_bye = y;
#endif /* datageneral */
#ifndef NOPUSH
        if (!nopush && !inserver)
          en_hos = y;
#endif /* NOPUSH */
#ifndef NOSPL
        en_asg = en_que = y;
#endif /* NOSPL */
        break;

      case EN_BYE:
#ifndef datageneral
/*
  In Data General AOS/VS Kermit can't log out its superior process.
*/
        en_bye = y;
#endif /* datageneral */
        break;
      case EN_CPY:
        en_cpy = y;
        break;
      case EN_CWD:
        en_cwd = y;
#ifdef IKSD
        if (inserver && y == 0) {
            fnrpath = PATH_OFF;
            fnspath = PATH_OFF;
        }
#endif /* IKSD */
        break;
      case EN_DEL:                      /* Deleting of files */
        en_del = y;
        break;
      case EN_DIR:
        en_dir = y;
        break;
      case EN_FIN:
        en_fin = y;
        break;
      case EN_GET:
        en_get = y;
        break;
#ifndef NOPUSH
      case EN_HOS:
        if (!nopush)
         en_hos = y;
        break;
#endif /* NOPUSH */
      case EN_REN:
        en_ren = y;
        break;
      case EN_SEN:
        en_sen = y;
        break;
      case EN_SET:
        en_set = y;
        break;
      case EN_SPA:
        en_spa = y;
        break;
      case EN_TYP:
        en_typ = y;
        break;
      case EN_WHO:
        en_who = y;
        break;
#ifndef NOSPL
      case EN_ASG:
        en_asg = y;
        break;
      case EN_QUE:
        en_que = y;
        break;
#endif /* NOSPL */
      case EN_RET:
        en_del = y;
        break;
      case EN_MAI:
#ifdef CK_LOGIN
        if (isguest && y) {
            printf("?Sorry, not valid for guests\n");
            return(-9);
        }
#endif /* CK_LOGIN */
        en_mai = y;
        break;
      case EN_PRI:
#ifdef CK_LOGIN
        if (isguest && y) {
            printf("?Sorry, not valid for guests\n");
            return(-9);
        }
#endif /* CK_LOGIN */
        en_pri = y;
        break;
      case EN_MKD:
        en_mkd = y;
        break;
      case EN_RMD:
        en_rmd = y;
        break;
      case EN_XIT:
        en_xit = y;
        break;
      case EN_ENA:
        if (((y & 1) && !(en_ena & 1)) ||
            ((y & 2) && !(en_ena & 2))) {
            printf("?Sorry, DISABLE ENABLE can not be undone\n");
            return(-9);
        } else {
            en_ena = y;
            break;
        }
      default:
        return(-2);
    }
    return(1);
}
#endif /* NOFRILLS */
#endif /* NOSERVER */

#ifndef NOFRILLS

static int del_lis = 0;
static int del_dot = 0;
static int del_hdg = 0;
static int del_pag = -1;
static int del_ask = 0;

#ifndef NOSHOW
VOID
showdelopts() {
    int x = 0;
    extern int optlines;
    prtopt(&optlines,"");
    prtopt(&optlines,"DELETE");
    if (del_ask > -1) {
        prtopt(&optlines, del_ask ? "/ASK" : "/NOASK");
        x++;
    }
#ifdef UNIXOROSK
    if (del_dot > -1) {
        prtopt(&optlines, del_dot ? "/DOTFILES" : "/NODOTFILES");
        x++;
    }
#endif /* UNIXOROSK */
    if (del_lis > -1) {
        prtopt(&optlines, del_lis ? "/LIST" : "/NOLIST");
        x++;
    }
    if (del_hdg > -1) {
        prtopt(&optlines, del_hdg ? "/HEADING" : "/NOHEADING");
        x++;
    }
#ifndef CK_TTGWSIZ
    if (del_pag > -1) {
        prtopt(&optlines, del_pag ? "/PAGE" : "/NOPAGE");
        x++;
    }
#endif /* CK_TTGWSIZ */
    if (!x) prtopt(&optlines,"(no options set)");
    prtopt(&optlines,"");
}
#endif /* NOSHOW */


int
setdelopts() {
    int x_lis = -1, x_pag = -1, x_dot = -1, x_hdg = -1, x_ask = -1;
    int getval = 0;
    char c;
    while (1) {
        if ((y = cmswi(deltab,ndeltab,"Switch","",xxstring)) < 0) {
            if (y == -3)
              break;
            else
              return(y);
        }
        c = cmgbrk();
        if ((getval = (c == ':' || c == '=')) && !(cmgkwflgs() & CM_ARG)) {
            printf("?This switch does not take an argument\n");
            return(-9);
        }
        if (!getval && (cmgkwflgs() & CM_ARG)) {
            printf("?This switch requires an argument\n");
            return(-9);
        }
        switch (y) {
          case DEL_DOT:
            x_dot = 1;
            break;
          case DEL_NOD:
            x_dot = 0;
            break;
          case DEL_HDG:
            x_hdg = 1;
            break;
          case DEL_LIS:
            x_lis = 1;
            break;
          case DEL_NOL:
            x_lis = 0;
            break;
#ifndef CK_TTGWSIZ
          case DEL_PAG:
            x_pag = 1;
            break;
          case DEL_NOP:
            x_pag = 0;
            break;
#endif /* CK_TTGWSIZ */
          case DEL_QUI:
            x_lis = 0;
            break;
          case DEL_VRB:
            x_lis = 1;
            break;
          case DEL_ASK:
            x_ask = 1;
            break;
          case DEL_NAS:
            x_ask = 0;
            break;
          default:
            printf("?Sorry, this option can not be set\n");
            return(-9);
        }
    }
    if ((x = cmcfm()) < 0)              /* Get confirmation */
      return(x);
    if (x_pag > -1) del_pag = x_pag;
    if (x_dot > -1) del_dot = x_dot;
    if (x_hdg > -1) del_hdg = x_hdg;
    if (x_lis > -1) del_lis = x_lis;
    if (x_ask > -1) del_ask = x_ask;
    return(success = 1);
}

#ifdef OS2
static char ** xmtchs = NULL;
static int xmtchn = 0;
#endif /* OS2 */

int
dodel() {                               /* DELETE */
    int i, j, k, x;
    int fs = 0;                         /* Need to call fileselect() */
    int len = 0;
    int bad = 0;
    int getval = 0, asking = 0;
    int simulate = 0, rc = 0;
    CK_OFF_T minsize = -1L, maxsize = -1L;
    int havename = 0, confirmed = 0;
    int qflag = 0;
    int summary = 0;
    int deldirs = 0;
    int deltree = 0;
    int itsadir = 0;
    int argisdir = 0;
    int xmode = -1, scan = 0, skip = 0;
#ifdef COMMENT
    int pass = 0;
#endif /* COMMENT */
    char c;
    char * deldef = "";
    char safebuf[CKMAXPATH+1];
    struct FDB sw, fi, fl;
    char
      * del_aft = NULL,
      * del_bef = NULL,
      * del_naf = NULL,
      * del_nbf = NULL,
      * del_exc = NULL;
    int
      x_lis = 0,
      /* x_dot = -1, */
      x_hdg = 0;

    char * dxlist[8];

    for (i = 0; i < 8; i++) dxlist[i] = NULL;

    g_matchdot = matchdot;

    if (del_lis > -1) x_lis    = del_lis;
    if (del_dot > -1) matchdot = del_dot;
    if (del_hdg > -1) x_hdg    = del_hdg;
    if (del_pag > -1) xaskmore = del_pag;
    if (del_ask > -1) asking   = del_ask;

    diractive = 1;
    nolinks = 2;                        /* By default don't follow links */

    cmfdbi(&sw,                         /* First FDB - command switches */
           _CMKEY,                      /* fcode */
           "File specification;\n or switch",
           "",                          /* default */
           "",                          /* addtl string data */
           ndeltab,                     /* addtl numeric data 1: tbl size */
           4,                           /* addtl numeric data 2: 4 = cmswi */
           xxstring,                    /* Processing function */
           deltab,                      /* Keyword table */
           &fi                          /* Pointer to next FDB */
           );
    cmfdbi(&fl,                         /* Anything that doesn't match */
           _CMFLD,                      /* fcode */
           "",                          /* hlpmsg */
           "",                          /* default */
           "",                          /* addtl string data */
           0,                           /* addtl numeric data 1 */
           0,                           /* addtl numeric data 2 */
           xxstring,
           NULL,
           NULL
           );
  again:
    cmfdbi(&fi,                         /* 2nd FDB - file to delete */
           _CMIFI,                      /* fcode */
           "File(s) to delete",         /* hlpmsg */
           deldef,                      /* default */
           "",                          /* addtl string data */
           nolinks | deldirs,           /* 0 = files, 1 = files or dirs */
           0,                           /* 1 = dirs only */
           xxstring,
           NULL,
           &fl
           );
    while (!havename && !confirmed) {
        x = cmfdb(&sw);                 /* Parse something */
        if (x < 0) {                    /* Error */
            if (x == -3)
              break;
            if (x == -2 || x == -9)
              printf("?Does not match switch or filename: \"%s\"\n",atmbuf);
            return(x);
        }
        if (cmresult.fcode != _CMKEY)   /* Break out if not a switch */
          break;
        c = cmgbrk();                   /* Get break character */
        if ((getval = (c == ':' || c == '=')) && !(cmgkwflgs() & CM_ARG)) {
            printf("?This switch does not take an argument\n");
            rc = -9;
            goto xdelete;
        }
        if (!getval && (cmgkwflgs() & CM_ARG)) {
            printf("?This switch requires an argument\n");
            rc = -9;
            goto xdelete;
        }
        switch (k = cmresult.nresult) {
          case DEL_AFT:
          case DEL_BEF:
          case DEL_NAF:
          case DEL_NBF:
            if (!getval) break;
            if ((x = cmdate("File-time","",&s,0,xxstring)) < 0) {
                if (x == -3) {
                    printf("?Date-time required\n");
                    x = -9;
                } else
                  rc = x;
                goto xdelete;
            }
            fs++;
            deltree = 0;
            switch (k) {
              case DEL_AFT: makestr(&del_aft,s); break;
              case DEL_BEF: makestr(&del_bef,s); break;
              case DEL_NAF: makestr(&del_naf,s); break;
              case DEL_NBF: makestr(&del_nbf,s); break;
            }
            break;
          case DEL_DOT:
            matchdot = 1;
            break;
          case DEL_NOD:
            matchdot = 0;
            break;
          case DEL_ALL:
            fs = 0;
#ifdef VMS
            deldef = "*.*";             /* UNIX, Windows, OS/2 */
#else
#ifdef datageneral
            deldef = "+";               /* AOS/VS */
#else
            deldef = "*";               /* UNIX, Windows, OS/2, VMS... */
#endif /* datageneral */
#endif /* VMS */
            deltree = 1;
            nolinks = 2;
            matchdot = 1;
            recursive = 1;              /* Fall through purposely... */
          case DEL_DIR:
            deldirs = 1;
            goto again;
          case DEL_EXC:
            if (!getval) break;
            if ((x = cmfld("Pattern","",&s,xxstring)) < 0) {
                if (x == -3) {
                    printf("?Pattern required\n");
                    x = -9;
                } else
                  rc = x;
                goto xdelete;
            }
            fs++;
            deltree = 0;
            makestr(&del_exc,s);
            break;
          case DEL_HDG:
            x_hdg = 1;
            break;
#ifdef RECURSIVE
          case DEL_REC:
            recursive = 1;
            break;
#endif /* RECURSIVE */
          case DEL_LIS:
            x_lis = 1;
            break;
          case DEL_SUM:
            summary = 1;
            x_lis = 0;
            x_hdg = 1;
            break;
          case DEL_NOL:
            x_lis = 0;
            break;
#ifndef CK_TTGWSIZ
          case DEL_PAG:
            xaskmore = 1;
            break;
          case DEL_NOP:
            xaskmore = 0;
            break;
#endif /* CK_TTGWSIZ */
          case DEL_QUI:
            qflag = 1;
            x_lis = 0;
            break;
          case DEL_VRB:
            x_lis = 1;
            break;

          case DEL_SMA:
          case DEL_LAR:
            if (!getval) break;
            if ((x = cmnum("File size in bytes","0",10,&y,xxstring)) < 0)
              return(x);
            fs++;
            deltree = 0;
            switch (cmresult.nresult) {
              case DEL_SMA: minsize = y; break;
              case DEL_LAR: maxsize = y; break;
            }
            break;

          case DEL_SIM:
            simulate = 1;
            x_lis = 1;
            break;
          case DEL_ASK:
            asking = 1;
            break;
          case DEL_NAS:
            asking = 0;
            break;
          case DEL_TYP: {
              extern struct keytab txtbin[];
              if (!getval) break;
              if ((x = cmkey(txtbin,3,"","",xxstring)) < 0)
                return(x);
              if (x == 2) {             /* ALL */
                  xmode = -1;
              } else {                  /* TEXT or BINARY only */
                  xmode = x;
                  scan = 1;
              }
              break;
          }
          default:
            printf("?Not implemented yet - \"%s\"\n",atmbuf);
            return(-9);
        }
    }
    if (qflag && (cmresult.fcode == _CMFLD)) {
        if ((x = cmcfm()) < 0)
          return(x);
        else
          return(success = 0);
    }
    if (cmresult.fcode != _CMIFI) {
        if (*atmbuf) {
	    int x;
            if (iswild(atmbuf) && nzxpand(atmbuf,nzxopts) == 0)
              printf("?No files match: %s\n",brstrip(atmbuf));
            else if ((x = zchki(atmbuf)) == -1)
	      printf("?File not found: %s\n",brstrip(atmbuf));
	    else if (x == -2)
	      printf("?Not a regular file: %s\n",atmbuf);
	    else
              /* printf("?Not a deletable file: %s\n",atmbuf); */
              goto tryanyway;
        } else {
            printf("?A file specification is required\n");
        }
        return(-9);
    }
  tryanyway:
    ckstrncpy(tmpbuf,cmresult.sresult,TMPBUFSIZ); /* Safe copy of filespec */
    if (deldirs) {
        ckstrncpy(safebuf,cmresult.sresult,CKMAXPATH);
#ifdef VMSORUNIX
        len = zgetfs(tmpbuf);           /* Is it a directory name? */
        argisdir = zgfs_dir;            /* Then because of how zxpand() */
        if (argisdir && zgfs_link)      /* works, we have to add it to */
          argisdir = 0;                 /* the list. */
        if (itsadir)
          len = -2;
#else
        len = zchki(tmpbuf);
        if (len < 0)
          argisdir = isdir(tmpbuf);
#endif /* VMSORUNIX */
    }
    debug(F110,"DELETE file",tmpbuf,0);
    if ((x = cmcfm()) < 0)
      return(x);

#ifdef IKSD
#ifdef CK_LOGIN
    if (inserver && isguest) {
        printf("?Sorry, DELETE unavailable to guests\n");
        return(-9);
    }
#endif /* CK_LOGIN */
#endif /* IKSD */

#ifndef OS2ORUNIX
    if (simulate) {
        printf("?Sorry, /SIMULATE not implemented on this platform\n");
        return(-9);
    }
#endif /* OS2ORUNIX */

#ifdef COMMENT
    /* (not needed) */
    if (!iswild(tmpbuf)) {
        char *m;
        errno = 0;
        x = zchki(tmpbuf);
        if (x < 0) {
            switch (x) {
              case -2: m = "Not a regular file"; break;
              case -1: m = "File not found or not accessible"; break;
              default: m = errno ? ck_errstr() : "Can't delete";
            }
            printf("?%s: \"%s\"\n",m,tmpbuf);
            return(-9);
        }
    }
#endif /* COMMENT */

    makelist(del_exc,dxlist,8);

/* tmpbuf[] has the name - now do any needed conversions on it */

#ifdef OS2
    {   /* Lower level functions change / to \, not good for CMD.EXE. */
        char *p = tmpbuf;
        while (*p) {                    /* Change them back to \ */
            if (*p == '/') *p = '\\';
            p++;
        }
    }
#endif /* OS2 */

#ifdef VMS
    if (iswild(tmpbuf)) {
#ifdef COMMENT
        /* Does not handle '.' as version separator */
        char *p = tmpbuf;
        x = 0;
        while (*p) {
            if (*p == ';') {
                x = 1;
                break;
            } else
              p++;
        }
        if (!x) ckstrncat(tmpbuf,";*",TMPBUFSIZ);
#else
        j = 0; x = 0;                   /* for end_dot and number of dots */
        i = strlen(tmpbuf);
        if (tmpbuf[i] == ';') {
            ckstrncat(tmpbuf,"0",TMPBUFSIZ);
        } else {
            if (tmpbuf[i--] == '.')
              j++;
            for (; i >= 0; i--) {
                if (tmpbuf[i] == ';' || tmpbuf[i] == ':' ||
                    tmpbuf[i] == ']' || tmpbuf[i] == '>')
                  break;
                else if (tmpbuf[i] == '.')
                  x++;
            }
            if (tmpbuf[i] != ';') {     /* dot may have been used */
                if (j) {                /* last char is dot */
                  if (x)                /* second is version separator */
                    ckstrncat(tmpbuf,"0",TMPBUFSIZ);
                  else                  /* 'foo.' */
                    ckstrncat(tmpbuf,";0",TMPBUFSIZ);
                } else if (x == 1)      /* lacking a version separator */
                  ckstrncat(tmpbuf,";0",TMPBUFSIZ);
                else if (x == 0)        /* x == 2 has a version */
                  ckstrncat(tmpbuf,".*;0",TMPBUFSIZ);
            }
        }
#endif /* COMMENT */
    }
#endif /* VMS */

    debug(F110,"dodel tmpbuf",tmpbuf,0); /* Filename */

#ifndef OS2ORUNIX                       /* No built-in DELETE code... */
    /* Construct system command. */
    ckmakmsg(line,LINBUFSIZ,DELCMD," ",tmpbuf,NULL);
#else
#ifdef VMS
    if (asking) {                       /* Maybe overwrite in VMS */
        if (x_lis)                      /* if options are needed... */
          ckmakmsg(line,LINBUFSIZ,DELCMD," /confirm/log ",tmpbuf,NULL);
        else
          ckmakmsg(line,LINBUFSIZ,DELCMD," /confirm ",tmpbuf,NULL);
    } else if (x_lis)
      ckmakmsg(line,LINBUFSIZ,DELCMD," /log ",tmpbuf,NULL);
    conres();
#endif /* VMS */

    debug(F110,"dodel line",line,0);
#endif /* OS2ORUNIX */

#ifdef MAC
    success = (zdelet(tmpbuf) == 0);

#else  /* !MAC ... */

#ifdef OS2ORUNIX
    {
        int filespace = 0;
        int count = 0;
        int lines = 0;
        int n = 0;

        s = tmpbuf;

#ifdef CK_TTGWSIZ
#ifdef OS2
        ttgcwsz();
#else /* OS2 */
        /* Check whether window size changed */
        if (ttgwsiz() > 0) {
            if (tt_rows > 0 && tt_cols > 0) {
                cmd_rows = tt_rows;
                cmd_cols = tt_cols;
            }
        }
#endif /* OS2 */
#endif /* CK_TTGWSIZ */

        if (x_hdg > 0 && !summary) {
            printf("Deleting %s...%s\n", s, simulate ? " (SIMULATION)" : "");
            n += 2;
        }
#ifdef ZXREWIND
        z = zxrewind();                 /* Rewind file list */
#else
        if (!deldirs)
          nzxopts = ZX_FILONLY;
        if (recursive) nzxopts |= ZX_RECURSE;
        if (matchdot)  nzxopts |= ZX_MATCHDOT;
        errno = 0;
        z = nzxpand(s,nzxopts);         /* Expand file list */
#endif /* ZXREWIND */
        debug(F111,"dodel",s,z);

        /* If deleting directories, sort in reverse order */
        /* so we delete the files first, then the directory. */

#ifdef OS2
        /* In K95, we have no mtchs array, nor any control over */
        /* the order in which znext() returns filenames, so we  */
        /* must copy the array and sort it. */
        {
            int i;
            if (xmtchs) {               /* Free previous list in case */
                debug(F101,"dodel freeing previous list","",xmtchn);
                for (i = 0; i < xmtchn; i++) /* it wasn't freed last time. */
                  if (xmtchs[i])
                    free(xmtchs[i]);
                free(xmtchs);
            }
            xmtchn = 0;
            xmtchs = (char **)malloc(z * (sizeof(char **))); /* Make new one */
            if (!xmtchs) {
                printf("?Memory allocation failure\n");
                return(-9);
            }
            for (i = 0; i < z; i++) {
                xmtchs[i] = NULL;
                znext(tmpbuf);
                if (!*tmpbuf)
                  break;
                makestr(&(xmtchs[i]),tmpbuf);
                if (!xmtchs[i]) {
                    printf("?Memory allocation failure\n");
                    xmtchn = i - 1;
                    rc = -9;
                    goto xdelete;
                }
                /* debug(F111,"dodel add",xmtchs[i],i); */
            }
            xmtchn = i;
            debug(F101,"dodel xmtchn","",xmtchn);
            sh_sort(xmtchs,NULL,z,0,deldirs,0);
        }
#else
#ifdef UNIX
        sh_sort(mtchs,NULL,z,0,deldirs,filecase);
#endif /* UNIX */
#endif /* OS2 */

        if (z > 0) {
            int i;
#ifdef OS2
            int ix = 0;
#endif /* OS2 */
            success = 1;
            if (x_hdg > 0)
              printf("\n");

            while (
#ifdef OS2
                   ix < xmtchn
#else
                   1
#endif /* OS2 */
                   ) {                  /* Loop for all files */
#ifdef OS2
                ckstrncpy(tmpbuf,xmtchs[ix++],TMPBUFSIZ);
#else
                znext(tmpbuf);          /* Get next file */
#endif /* OS2 */
                if (!*tmpbuf) {         /* No more */
                    if (deldirs && recursive && argisdir) {
                        ckstrncpy(tmpbuf,safebuf,TMPBUFSIZ);
                        argisdir = 0;   /* (only do this once) */
                    } else
                      break;
                }
                skip = 0;
                if (!deltree) {
                    if (fs)
                      if (fileselect(tmpbuf,
                                     del_aft,del_bef,del_naf,del_nbf,
                                     minsize,maxsize,0,8,dxlist) < 1) {
                          skip++;
                      }
                }
                if (!skip && scan && itsadir) {
                    skip++;
                }
                if (!skip && scan) {
                      switch (scanfile(tmpbuf,&y,nscanfile)) {
                      case FT_BIN:
                        if (xmode != 1)
                          skip++;
                        break;
                      case FT_TEXT:
                      case FT_7BIT:
                      case FT_8BIT:
#ifdef UNICODE
                      case FT_UTF8:
                      case FT_UCS2:
#endif /* UNICODE */
                        if (xmode != 0)
                          skip++;
                    }
                }
                if (!skip && asking) {
                    int x;
                    ckmakmsg(line,LINBUFSIZ," Delete ",tmpbuf,"? ",NULL);
                    x = getyesno(line,3);
                    switch (x) {
                      case 0: continue;           /* no */
                      case 1: break;              /* yes */
                      case 2: goto xdelete;       /* quit */
                      case 3: asking = 0; break;  /* go */
                    }
                }
#ifdef VMSORUNIX
                len = zgetfs(tmpbuf);   /* Get length and accessibility */
                itsadir = zgfs_dir;
                if (itsadir && zgfs_link) { /* Treat links to directories */
                    itsadir = 0;        /* as regular files */
                    if (scan)           /* But not if /TYPE: was given */
                      skip++;
                }
                if (itsadir)            /* (emulate non-Unix code) */
                  len = -2;
#else
                len = zchki(tmpbuf);    /* Get accessibility */
                if (len < 0)            /* See if it's a directory */
                  itsadir = isdir(tmpbuf);
#endif /* VMSORUNIX */

                if (skip) {
#ifdef COMMENT                          /* Too verbose */
                    if (x_lis > 0) {
                        lines++;
                        printf(" %s (SKIPPED)\n",tmpbuf);
#ifdef CK_TTGWSIZ
                        if (++n > cmd_rows - 3)
                          if (!askmore()) goto xdelete; else n = 0;
#endif /* CK_TTGWSIZ */
                    }
#endif /* COMMENT */
                    continue;
                }

                debug(F111,"DELETE len",tmpbuf,len);
                if (simulate) {
                    filespace += len;
                    count++;
                    if (x_lis > 0) {
                        lines++;
                        printf(" %s (SELECTED)\n",tmpbuf);
                        if (++n > cmd_rows - 3) {
                            int xx;
                            xx = askmore();
                            if (!xx) goto xdelete; else n = 0;
                        }
                    }
                } else if (len >= 0 || !itsadir) { /* Regular file */
                    zdelet(tmpbuf);                /* or symlink, etc... */
                    if (zchki(tmpbuf) < 0) {
                        filespace += len;
                        count++;
                        if (x_lis > 0) {
                            lines++;
                            printf(" %s (OK)\n",tmpbuf);
                            if (++n > cmd_rows - 3)
                              if (!askmore()) goto xdelete; else n = 0;
                        }
                    } else {
                        bad++;
                        success = 0;
                        if (x_lis > 0) {
                            lines++;
                            printf(" %s (FAILED: %s)\n",tmpbuf,ck_errstr());
                            if (++n > cmd_rows - 3)
                              if (!askmore()) goto xdelete; else n = 0;
                        }
                    }
                } else if (/* pass > 0 && */ deldirs && itsadir) {
                    /* It's a directory */
                    if (zrmdir(tmpbuf) > -1) { /* Only works if empty */
                        count++;
                        if (x_lis > 0) {
                            lines++;
                            printf(" %s (OK)\n",tmpbuf);
                            if (++n > cmd_rows - 3)
                              if (!askmore()) goto xdelete; else n = 0;
                        }
                    } else {
                        success = 0;
                        if (x_lis > 0) {
                            lines++;
                            printf(" %s (FAILED: %s)\n",
                                   tmpbuf,
                                   ck_errstr());
                            if (++n > cmd_rows - 3)
                              if (!askmore()) goto xdelete; else n = 0;
                        }
                    }
                } else if (x_lis > 0) {
                    lines++;
                    if (isdir(tmpbuf))
                      printf(" %s (FAILED: directory)\n",tmpbuf);
                    else
                      printf(" %s (FAILED: not a regular file)\n",tmpbuf);
                    if (++n > cmd_rows - 3)
                      if (!askmore()) goto xdelete; else n = 0;
                }
            }
            if (x_hdg > 0) {
                if (lines > 0)
                  printf("\n");
                if (++n > cmd_rows - 3)
                  if (!askmore()) goto xdelete; else n = 0;
                printf("%d file%s %sdeleted, %d byte%s %sfreed%s\n",
                       count,
                       count != 1 ? "s" : "",
                       simulate ? "would be " : "",
                       filespace,
                       filespace != 1 ? "s" : "",
                       simulate ? "would be " : "",
                       simulate ? " (maybe)" : ""
                       );
            }
            if (!x_lis && !success && !quiet) {
                printf("?DELETE failed for %d file%s \
(use DELETE /LIST to see details)\n",
                       bad, bad == 1 ? "" : "s"
                       );
            }
        } else if (x_lis > 0) {
            if (errno)
              printf("?%s: %s\n",ck_errstr(), tmpbuf);
            else
              printf("?Can't delete: %s\n",tmpbuf);
        }
    }
#else /* OS2ORUNIX */
#ifndef VMS                             /* Others - let the system do it. */
    xsystem(line);
    x = nzxpand(tmpbuf,nzxopts);
    success = (x > 0) ? 0 : 1;
    if (x_hdg > 0)
      printf("%s - %sdeleted\n", tmpbuf, success ? "" : "not ");
#else
    if (asking)
      printf("\n");
    x = xsystem(line);                  /* zshcmd returns 1 for success */
    success = (x > 0) ? 1 : 0;
    if (x_hdg > 0 && !asking)
      printf("%s - %sdeleted\n", tmpbuf, success ? "" : "not ");
    concb((char)escape);
#endif /* VMS */
#endif /* OS2ORUNIX */
#endif /* MAC */
  xdelete:
    if (g_matchdot > -1) {
        matchdot = g_matchdot;          /* Restore these... */
        g_matchdot = -1;
    }
#ifdef OS2
    if (xmtchs) {
        int i;
        debug(F101,"dodel freeing list","",xmtchn);
        for (i = 0; i < xmtchn; i++)
          if (xmtchs[i]) free(xmtchs[i]);
        free(xmtchs);
        xmtchs = NULL;
        xmtchn = 0;
    }
#endif /* OS2 */
    debug(F101,"dodel result","",rc);
    return((rc < 0) ? rc : success);
}
#endif /* NOFRILLS */

#ifndef NOSPL                           /* The ELSE command */
_PROTOTYP( VOID pushqcmd, (char *) );

int
doelse() {
    if (!ifcmd[cmdlvl]) {
        printf("?ELSE doesn't follow IF\n");
        return(-2);
    }
#ifdef COMMENT
/*
  Wrong.  This prevents IF..ELSE IF...ELSE IF...ELSE IF...ELSE...
  from working.
*/
    ifcmd[cmdlvl] = 0;
#endif /* COMMENT */
    if (!iftest[cmdlvl]) {              /* If IF was false do ELSE part */
        if (maclvl > -1 || tlevel > -1) { /* In macro or command file */
            debug(F100,"doelse pushing","",0);
#ifndef COMMENT
            pushcmd(NULL);              /* save rest of command. */
#else
            /* This fixes certain obscure problems */
            /* but breaks many other constructions that must work. */
            pushqcmd(NULL);
#endif /* COMMENT */
        } else {                        /* If interactive, */
            cmini(ckxech);              /* just start a new command */
            printf("\n");               /* (like in MS-DOS Kermit) */
            if (pflag) prompt(xxstring);
        }
    } else {                            /* Condition is false */
        if ((y = cmtxt("command to be ignored","",&s,NULL)) < 0)
          return(y);                    /* Gobble up rest of line */
    }
    return(0);
}
#endif /* NOSPL */

#ifndef NOSPL
int
doswitch() {
    char *lp, *ap;                      /* Macro argument pointer */
    int len = 0, x, y, pp = 0;
    char brbuf[3];

    /* Get variable name */

    tmpbuf[0] = NUL;
    brbuf[0] = '{';
    brbuf[1] = '}';
    brbuf[2] = NUL;

    y = cmfld("Variable name","",&s,xxstring);
    debug(F111,"doswitch cmfld",s,y);
    if (y < 0) {
        if (y == -3)			/* Because brstrip() writes */
          s = brbuf;			/* into its argument. */
        else
          return(y);
    }
    debug(F110,"doswitch A",s,0);
    if (!strcmp(s,"(")) {
        pp++;
        if ((y = cmfld("Variable name","",&s,xxstring)) < 0) {
            if (y == -3)
              s = brbuf;
            else
              return(y);
	    debug(F110,"doswitch B",s,0);
        }
    }
    len = ckstrncpy(tmpbuf,brstrip(s),TMPBUFSIZ);
    if (tmpbuf[0] == CMDQ) {
        if (chkvar(s) < 1) {
            printf("?Variable name required\n");
            return(-9);
        }
    }
    if (pp > 0) {                       /* If open paren given parse closing */
        if ((y = cmfld("Closing parenthesis","",&s,NULL)) < 0)
          return(y);
        if (strcmp(atmbuf,")")) {
            printf("?Closing parenthesis required\n");
            return(-9);
        }
    }
    lp = line;
    x = ckstrncpy(lp,"_switx ",LINBUFSIZ); /* _switx + space */
    lp += x;
    ap = lp;
    debug(F010,"SWITCH a",line,0);

#ifdef COMMENT
    x = ckmakmsg(lp,LINBUFSIZ-x,tmpbuf," ",NULL,NULL); /* variable name + SP */
#else
    {                                   /* variable name + SP */
        char * p = tmpbuf;
	if (len > 0) {
	    if (tmpbuf[0] == '(' && tmpbuf[len-1] == ')') {
		tmpbuf[len-1] = NUL;
		p++;
	    }
        }
        x = ckmakmsg(lp,LINBUFSIZ-x,"{",brstrip(p),"}"," ");
    }
#endif /* COMMENT */
    debug(F010,"SWITCH b",line,0);
    lp += x;

    /* Get body */

    if ((y = cmtxt("series of cases","",&s,NULL)) < 0) return(y);
    if ((y = (int)strlen(s)) < 1) return(-2);
    if (s[0] != '{' && s[y-1] != '}') { /* Supply braces if missing */
        ckmakmsg(tmpbuf,TMPBUFSIZ,"{ ",s," }",NULL);
        s = tmpbuf;
    }
    if (litcmd(&s,&lp,(LINBUFSIZ - (lp - (char *)line) - 2)) < 0) {
        printf("?Unbalanced braces\n");
        return(0);
    }
    debug(F010,"SWITCH c",line,0);

    x = mlook(mactab,"_switx",nmac);    /* Look up SWITCH macro definition */
    if (x < 0) {                        /* Not there? */
        addmmac("_switx",sw_def);       /* Put it back. */
        if ((x = mlook(mactab,"_switx",nmac)) < 0) { /* Look it up again. */
            printf("?SWITCH macro definition gone!\n"); /* Shouldn't happen. */
            return(success = 0);
        }
    }
    debug(F010,"SWITCH command",line,0); /* Execute the SWITCH macro. */
    success = dodo(x,ap,cmdstk[cmdlvl].ccflgs | CF_IMAC);
    debug(F101,"SWITCH status","",success);
    return(success);
}

int
dofor() {                               /* The FOR command. */
    int i, fx, fy, fz;                  /* loop variables */
    char *ap, *di;                      /* macro argument pointer */
    int pp = 0;                         /* Paren level */
    int mustquote = 0;

    for (i = 0; i < 2; i++) {
        if ((y = cmfld("Variable name","",&s,NULL)) < 0) {
            if (y == -3) {
                printf("?Variable name required\n");
                return(-9);
            } else
              return(y);
        }
        if (strcmp(s,"("))
          break;
        pp++;
    }
#ifdef COMMENT
    if ((y = parsevar(s,&x,&z)) < 0)    /* Check variable. */
      return(y);
#else
    if (*s == CMDQ)                     /* If loop variable starts with */
      mustquote++;                      /* backslash, mustquote is > 0. */
#endif /* COMMENT */

    lp = line;                          /* Build a copy of the command */
    ckstrncpy(lp,"_forx ",LINBUFSIZ);
    lp += (int)strlen(line);            /* "_for" macro. */
    ap = lp;                            /* Save pointer to macro args. */

    if (*s == CMDQ) s++;                /* Skip past backslash if any. */
    while ((*lp++ = *s++)) ;            /* copy it */
    lp--; *lp++ = SP;                   /* add a space */

    if ((y = cmnum("initial value","",10,&fx,xxstring)) < 0) {
        if (y == -3) return(-2);
        else return(y);
    }
    debug(F101,"dofor fx","",fx);
    s = atmbuf;                         /* Copy the atom buffer */

    if ((int)strlen(s) < 1) goto badfor;
/*
  In edit 192, we change the loop variables to be evaluated at loop entry,
  not each time through the loop.  This was required in order to allow
  \v(argc) to be used as a loop variable, or in a loop-variable expression.
  Thus, we can't have FOR loops that modify their own exit conditions by
  changing the final value or the increment.  The problem with \v(argc) was
  that it is on the macro stack; after entry into the _forx macro, it is at
  the wrong place.
*/
    sprintf(tmpbuf,"%d",fx);            /* (SAFE) Substitute actual value */
    s = tmpbuf;
    while ((*lp++ = *s++)) ;            /* (what they actually typed) */
    lp--; *lp++ = SP;
#ifdef DEBUG
    *lp = NUL;
    debug(F110,"FOR A",line,0);
#endif /* DEBUG */

    if ((y = cmnum("final value","",10,&fy,xxstring)) < 0) {
        if (y == -3) return(-2);
        else return(y);
    }
    debug(F101,"dofor fy","",fy);
    s = atmbuf;                         /* Same deal */
    if ((int)strlen(s) < 1)
      goto badfor;

    sprintf(tmpbuf,"%d",fy);            /* SAFE */
    s = tmpbuf;
    while ((*lp++ = *s++)) ;
    lp--;
    *lp++ = SP;
#ifdef DEBUG
    *lp = NUL;
    debug(F110,"FOR B",line,0);
#endif /* DEBUG */

    x_ifnum = 1;                        /* Increment or parenthesis */
    di = (fx < fy) ? "1" : "-1";        /* Default increment */
    if ((y = cmnum("increment",di,10,&fz,xxstring)) < 0) {
        debug(F111,"dofor increment",atmbuf,y);
        x_ifnum = 0;
        if (y == -3) {                  /* Premature termination */
            return(-2);
        } else if (y == -2) {           /* Maybe closing paren */
            if (!strcmp(atmbuf,")")) {
                pp--;                   /* Count it */
                s = di;                 /* supply default interval */
                fz = atoi(s);
            } else                      /* Not closing paren, invalid */
              return(y);
        } else                          /* Other error */
          return(y);
    } else {                            /* Number */
        x_ifnum = 0;
        debug(F101,"dofor fz","",fz);
        s = atmbuf;                     /* Use it */
    }
    if ((int)strlen(s) < 1)
      goto badfor;

    sprintf(tmpbuf,"%d",fz);            /* (SAFE) Same deal */
    s = tmpbuf;
    while ((*lp++ = *s++)) ;
    lp--; *lp++ = SP;

#ifdef DEBUG
    *lp = NUL;
    debug(F110,"FOR C",line,0);
#endif /* DEBUG */

    /* Insert the appropriate comparison operator */
    if (fz < 0)
      *lp++ = '<';
    else
      *lp++ = '>';
    *lp++ = SP;

#ifdef DEBUG
    *lp = NUL;
    debug(F110,"FOR D",line,0);
#endif /* DEBUG */

    if (pp > 0) {                       /* If open paren given parse closing */
        if ((y = cmfld("Closing parenthesis","",&s,NULL)) < 0)
          return(y);
        if (strcmp(atmbuf,")")) {
            printf("?Closing parenthesis required\n");
            return(-9);
        }
    }
    if ((y = cmtxt("Command to execute","",&s,NULL)) < 0) return(y);
    if ((y = (int)strlen(s)) < 1) return(-2);
    if (s[0] != '{' && s[y-1] != '}') { /* Supply braces if missing */
        ckmakmsg(tmpbuf,TMPBUFSIZ,"{ ",s," }",NULL);
        s = tmpbuf;
    }
    if (litcmd(&s,&lp,(LINBUFSIZ - (lp - (char *)line) - 2)) < 0) {
        printf("?Unbalanced braces\n");
        return(0);
    }
#ifdef DEBUG
    *lp = NUL;
    debug(F110,"FOR E",line,0);
#endif /* DEBUG */

#ifdef COMMENT
/* Too strict */
    if (fz == 0) {
        printf("?Zero increment not allowed\n");
        return(0);
    }
#endif /* COMMENT */
/*
  In version 8.0 we decided to allow macro names anyplace a numeric-valed
  variable could appear.  But this caused trouble for the FOR loops because
  the quoting in for_def[] assumed a \%i-style loop variable.  We account
  for this here in the if (mustquote)...else logic by invoking separate
  FOR macro definitions in the two cases.
*/
    if (mustquote) {                    /* \%i-style loop variable */
        x = mlook(mactab,"_forx",nmac); /* Look up FOR macro definition */
        if (x < 0) {                    /* Not there? */
            addmmac("_forx",for_def);   /* Put it back. */
            if ((x = mlook(mactab,"_forx",nmac)) < 0) { /* Look it up again. */
                printf("?FOR macro definition gone!\n");
                return(success = 0);
            }
        }
    } else {                            /* Loop variable is a macro */
        x = mlook(mactab,"_forz",nmac);
        if (x < 0) {
            addmmac("_forz",foz_def);
            if ((x = mlook(mactab,"_forz",nmac)) < 0) {
                printf("?FOR macro definition gone!\n");
                return(success = 0);
            }
        }
    }
    debug(F010,"FOR command",line,0);   /* Execute the FOR macro. */
    return(success = dodo(x,ap,cmdstk[cmdlvl].ccflgs | CF_IMAC));

badfor:
    printf("?Incomplete FOR command\n");
    return(-2);
}
#endif /* NOSPL */


#ifndef NOSPL

/*  T O D 2 S E C  --  Convert time of day as hh:mm:ss to secs since midnite */
/*
  Call with a string hh:mm or hh:mm:ss.
  Returns a 0 to 86400 on success, or a negative number on failure.
*/
long
tod2sec(t) char * t; {
    long t2;
    long hh = 0L, mm = 0L, ss = 0L;

    if (!t) t = "";
    if (!*t)
      return(-3L);
    debug(F110,"tod2sec",t,0);

    if (isdigit(*t))                    /* Get hours from argument */
      hh = *t++ - '0';
    else
      return(-1L);
    if (isdigit(*t))
      hh = hh * 10 + *t++ - '0';
#ifdef COMMENT
    if (hh > 24L)
      return(-1L);
#endif /* COMMENT */
    if (*t == ':')
      t++;
    else if (!*t)
      goto xtod2sec;
    else
      return(-1L);

    if (isdigit(*t))                    /* Minutes */
      mm = *t++ - '0';
    else
      return(-1L);
    if (isdigit(*t))
      mm = mm * 10 + *t++ - '0';
    if (mm > 60L)
      return(-1L);
    if (*t == ':')
      t++;
    else if (!*t)
      goto xtod2sec;
    else
      return(-1L);

    if (isdigit(*t))                    /* Seconds */
      ss = *t++ - '0';
    else
      return(-1L);
    if (isdigit(*t))
      ss = ss * 10 + *t++ - '0';
    if (ss > 60L)
      return(-1L);

    if (*t > 32)                        /* No trailing junk allowed */
      return(-1L);

  xtod2sec:

    t2 = hh * 3600L + mm * 60L + ss;    /* Seconds since midnight from arg */
    debug(F101,"tod2sec t2","",t2);

    return(t2);
}

int waitinterval = 1;

#ifdef OLDWAIT
#undef OLDWAIT
#endif /* OLDWAIT */

int kbchar = NUL;

int
dopaus(cx) int cx; {
    long zz;
    extern int sleepcan;

#ifdef OLDWAIT
    zz = -1L;
    x_ifnum = 1;                        /* Turn off internal complaints */
    if (cx == XXWAI)
      y = cmnum("seconds to wait, or time of day hh:mm:ss","1",10,&x,xxstring);
    else if (cx == XXPAU)
      y = cmnum("seconds to pause, or time of day hh:mm:ss",
                "1",10,&x,xxstring);
    else
      y = cmnum("milliseconds to sleep, or time of day hh:mm:ss",
                "100",10,&x,xxstring);
    x_ifnum = 0;
    if (y < 0) {
        if (y == -2) {                  /* Invalid number or expression */
            char *p = tmpbuf;           /* Retrieve string from atmbuf */
            int n = TMPBUFSIZ;
            *p = NUL;
            zzstring(atmbuf,&p,&n);     /* Evaluate in case it's a variable */
            zz = tod2sec(tmpbuf);       /* Convert to secs since midnight */
            if (zz < 0L) {
                printf("?Number, expression, or time of day required\n");
                return(-9);
            } else {
                char now[32];           /* Current time */
                char *p;
                long tnow;
                p = now;
                ztime(&p);
                tnow = atol(p+11) * 3600L + atol(p+14) * 60L + atol(p+17);
                if (zz < tnow)          /* User's time before now */
                  zz += 86400L;         /* So make it tomorrow */
                zz -= tnow;             /* Seconds from now. */
            }
        } else
          return(y);
    }
    if (x < 0) x = 0;
    switch (cx) {
      case XXPAU:                       /* PAUSE */
      case XXMSL:                       /* MSLEEP */
        if ((y = cmcfm()) < 0) return(y);
        break;
      case XXWAI:                       /* WAIT */
        z = 0;                          /* Modem signal mask */
        while (1) {                     /* Read zero or more signal names */
            y = cmkey(mstab,nms,"modem signal","",xxstring);
            if (y == -3) break;         /* -3 means they typed CR */
            if (y < 0) return(y);       /* Other negatives are errors */
            z |= y;                     /* OR the bit into the signal mask */
        }
        if ((y = cmcfm()) < 0) return(y);
        break;

      default:                          /* Shouldn't happen */
        return(-2);
    }

/* Command is entered, now do it. */

    if (zz > -1L) {                     /* Time of day given? */
        x = zz;
        if (zz != (long) x) {
            printf(
"Sorry, arithmetic overflow - hh:mm:ss not usable on this platform.\n"
                   );
            return(-9);
        }
    }
    if (cx == XXMSL) {                  /* Millisecond sleep */
        msleep(zz < 0 ? x : x * 1000);
        return(success = 1);
    }
    if (cx == XXPAU && !sleepcan) {     /* SLEEP CANCELLATION is OFF */
        sleep(x);
        return(success = 1);
    }

    /* WAIT, or else SLEEP with cancellation allowed... */

    do {                                /* Sleep loop */
        int mdmsig;
        if (sleepcan) {                 /* Keyboard cancellation allowed? */
            if (y = conchk()) {         /* Did they type something? */
#ifdef COMMENT
                while (y--) coninc(0);  /* Yes, gobble it all up */
#else
                /* There is a debate over whether PAUSE should absorb    */
                /* its cancelling character(s).  There are several       */
                /* reasons why it should gobble at least one character:  */
                /* (1) MS-DOS Kermit does it                             */
                /* (2) if not, subsequent PAUSE commands will terminate  */
                /*     immediately                                       */
                /* (3) if not, subsequent ASK commands will use it as    */
                /*     valid input.  If \13, then it will get no input   */
                /* (4) if not, then the character appears on the command */
                /*     line after all enclosing macros are complete.     */
                kbchar = coninc(0);     /* Gobble one up */
#endif /* COMMENT */
                break;                  /* And quit PAUSing or WAITing */
            }
        }
        if (cx == XXWAI) {              /* WAIT (z == modem signal mask) */
            debug(F101,"WAIT x","",x);
            if (z > 0) {                /* Looking for any modem signals? */
                mdmsig = ttgmdm();      /* Yes, get them */
                if (mdmsig < 0)         /* Failed */
                  return(success = 0);
                if ((mdmsig & z) == z)  /* Got what we wanted? */
                  return(success = 1);  /* Succeed */
            }
            if (x == 0)                 /* WAIT 0 and didn't get our signals */
              break;
        }
        sleep(1);                       /* No interrupt, sleep one second */
    } while (--x > 0);

    if (cx == XXWAI)                    /* If WAIT and loop exhausted */
      success = (z == 0);               /* Fail. */
    else                                /*  */
      success = (x == 0);               /* Set SUCCESS/FAILURE for PAUSE. */
    return(success);

#else  /* New code uses chained FDBs and allows FILE waits... */

    char * m = "";                      /* Help message */
    struct FDB nu, fl;                  /* Parse function descriptor blocks */
    int filewait = 0;
    int mdmsig = 0, fs = 0;
    char filedate[32];

    kbchar = 0;

    switch (cx) {
      case XXWAI: m = "seconds to wait, or time of day hh:mm:ss"; break;
      case XXPAU: m = "seconds to pause, or time of day hh:mm:ss"; break;
      case XXMSL: m = "milliseconds to sleep, or time of day hh:mm:ss"; break;
    }
    zz = -1L;
    cmfdbi(&nu,
           _CMNUM,                      /* Number */
           m,                           /* Help message */
           (cx == XXMSL) ? "100" : "1", /* Default */
           "",                          /* N/A */
           0,                           /* N/A */
           0,                           /* N/A */
           xxstring,                    /* Processing function */
           NULL,                        /* N/A */
           &fl                          /* Next */
           );
    cmfdbi(&fl,                         /* Time of day */
           _CMFLD,                      /* Field */
           "",                          /* hlpmsg */
           "",                          /* default */
           "",                          /* addtl string data */
           0,                           /* addtl numeric data 1 */
           0,                           /* addtl numeric data 2 */
           xxstring,                    /* processing func */
           NULL,                        /* N/A */
           NULL                         /* No next */
           );
    x = cmfdb(&nu);                     /* Parse a number or a field */
    if (x < 0) {
        if (x == -3)
          x = -2;
        return(x);
    }
    switch (cmresult.fcode) {
      case _CMNUM:                      /* Number */
        x = cmresult.nresult;
        break;
      case _CMFLD:                      /* Field */
        zz = tod2sec(cmresult.sresult); /* Convert to secs since midnight */
        if (zz < 0L) {
            printf("?Number, expression, or time of day required\n");
            return(-9);
        } else {
            char now[32];               /* Current time */
            char *p;
            long tnow;
            p = now;
            ztime(&p);
            tnow = atol(p+11) * 3600L + atol(p+14) * 60L + atol(p+17);
            if (zz < tnow)              /* User's time before now */
              zz += 86400L;             /* So make it tomorrow */
            zz -= tnow;         /* Seconds from now. */
        }
    }
    debug(F101,"PAUSE/WAIT/MSLEEP zz","",zz);
    switch (cx) {
      case XXPAU:                       /* PAUSE */
      case XXMSL:                       /* MSLEEP */
        if ((y = cmcfm()) < 0) return(y);
        break;
      case XXWAI:                       /* WAIT */
        z = 0;                          /* Modem signal mask */
        y = cmkey(waittab,nwaittab,"","",xxstring);
        if (y < 0) {
            if (y == -3) {
                if ((y = cmcfm()) < 0)
                  return(y);
                break;
            } else
              return(y);
        }
        if (y == WAIT_FIL) {            /* FILE */
            int wild = 0;
            if ((z = cmkey(wfswi,nwfswi,"event","",xxstring)) < 0)
              return(z);
            filewait = z;
            if (filewait == WF_MOD || filewait == WF_DEL)
              z = cmifi("Filename","",&s,&wild,xxstring);
            else
              z = cmfld("Filename","",&s,xxstring);
            if (z < 0)
              return(z);
            if (wild || ((filewait == WF_CRE) && iswild(s))) {
                printf("?Wildcards not valid here\n");
                return(-9);
            }
            ckstrncpy(tmpbuf,s,TMPBUFSIZ);
            if ((z = cmcfm()) < 0)
              return(z);
            break;
        } else if (y != WAIT_MDM) {     /* A modem signal */
            z |= y;                     /* OR the bit into the signal mask */
        }
        if (!filewait) {                /* Modem signals... */
            while (1) {                 /* Get zero or more signal names */
                y = cmkey(mstab,nms,"modem signal","",xxstring);
                if (y == -3) break;     /* -3 means they typed CR */
                if (y < 0) return(y);   /* Other negatives are errors */
                z |= y;                 /* OR the bit into the signal mask */
            }
            if ((y = cmcfm()) < 0) return(y);
            break;
        }

      default:                          /* Shouldn't happen */
        return(-2);
    } /* switch (cx) */

/* Command is entered, now do it. */

    if (zz > -1L) {                     /* Time of day given? */
        x = zz;
        if (zz != (long) x) {
            printf(
"Sorry, arithmetic overflow - hh:mm:ss not usable on this platform.\n"
                   );
            return(-9);
        }
    }
    if (sleepcan)
      concb((char)escape);              /* Ensure single-char wakeup */

    if (cx == XXMSL) {                  /* Millisecond sleep */
        msleep(zz < 0 ? x : x * 1000);
        return(success = 1);
    }
    if (cx == XXPAU && !sleepcan) {     /* SLEEP CANCELLATION is OFF */
        sleep(x);
        return(success = 1);
    }
    if (filewait) {                     /* FILE... */
        fs = zchki(tmpbuf);             /* Check if file exists */
        switch (filewait) {
          case WF_DEL:
            if (fs == -1)
              return(success = 1);
            break;
          case WF_MOD:
            if (fs == -1) {
                printf("?File does not exit: %s\n",tmpbuf);
                return(-9);
            }
            s = zfcdat(tmpbuf);         /* Get current modification date */
            if (!s) s = "";
            if (ckstrncpy(filedate,s,32) != 17) {
                printf("?Can't get modification time: %s\n",tmpbuf);
                return(-9);
            }
            break;
          case WF_CRE:
            if (fs > -1)
              return(success = 1);
            break;
        }
    }
    do {                                /* Polling loop */
        if (sleepcan) {                 /* Keyboard cancellation allowed? */
            if ((y = conchk()) > 0) {   /* Did they type something? */
                kbchar = coninc(0);     /* Yes, get first char they typed */
                debug(F000,"WAIT kbchar","",kbchar);
#ifdef COMMENT
                while (--y > 0)         /* Gobble the rest up */
                  coninc(0);
#endif /* COMMENT */
                return(success = 0);    /* And quit PAUSing or WAITing */
            }
        }
        if (filewait == 0) {
            if (cx == XXWAI) {          /* WAIT for modem signals */
                if (z != 0) {
                    mdmsig = ttgmdm();  /* Get them. */
                    debug(F101,"WAIT ttgmdm","",mdmsig);
                    if (mdmsig < 0)     /* Failure to get them? */
                      return(success = 0); /* Fail. */
                    if ((mdmsig & z) == z) /* Got desired ones? */
                      return(success = 1); /* Succeed. */
                } else if (x == 0)
                  return(success = 0);
            }
        } else {                        /* FILE... */
            fs = zchki(tmpbuf);         /* Get file status */
            if (filewait == WF_MOD) {   /* Wait for modification */
                if (fs == -1)           /* Failure to get status */
                  return(success = 0);  /* so WAIT fails. */
                s = zfcdat(tmpbuf);     /* Get current modification time */
                if (!s) s = "";         /* And compare with the time */
                if (strcmp(s,filedate)) /* when the WAIT started */
                  return(success = 1);
            } else if (filewait == WF_DEL) { /* Wait for deletion */
                if (fs == -1)           /* If file doesn't exist, */
                  return(success = 1);  /* succeed. */
            } else if (filewait == WF_CRE) { /* Wait for creation */
                if (fs != -1)           /* If file exists */
                  return(success = 1);  /* succeed. */
            }
        }
        if (x < 1)                      /* SLEEP/WAIT/PAUSE 0 */
          break;
        sleep(waitinterval);            /* No interrupt, sleep */
        x -= waitinterval;              /* Deduct sleep time */
    } while (x > 0);

    if (cx == XXWAI)                    /* WAIT time expired */
      success = (z == 0);               /* Succeed if no modem signals */
    else                                /* For SLEEP or PAUSE, success */
      success = (x == 0);               /* depends on whether it was */
    return(success);                    /* interrupted from the keyboard. */
#endif /* OLDWAIT */
}
#endif /* NOSPL */

#ifdef OS2ORUNIX
_PROTOTYP(int zcmpfn,(char *, char *));
#endif /* OS2ORUNIX */

#ifndef NOFRILLS
#ifdef NT
int 
dolink() {
    /* Parse a file or a directory name */
    int i, x, z, listing = 0, havename = 0, wild = 0, rc = 1;
    struct FDB sw, fi;

    cmfdbi(&sw,                         /* 2nd FDB - optional /PAGE switch */
           _CMKEY,                      /* fcode */
           "Filename or switch",        /* hlpmsg */
           "",                          /* default */
           "",                          /* addtl string data */
           nqvswtab,                    /* addtl numeric data 1: tbl size */
           4,                           /* addtl numeric data 2: 4 = cmswi */
           xxstring,                    /* Processing function */
           qvswtab,                     /* Keyword table */
           &fi                          /* Pointer to next FDB */
           );

    cmfdbi(&fi,                         /* 1st FDB - file to type */
           _CMIFI,                      /* fcode */
           "",                          /* hlpmsg */
           "",                          /* default */
           "",                          /* addtl string data */
           3,                           /* addtl numeric data 1 */
           0,                           /* addtl numeric data 2 */
           xxstring,
           NULL,
           NULL
           );

    while (!havename) {
        x = cmfdb(&sw);                 /* Parse something */
        if (x < 0)                      /* Error */
          return(x);
        switch (cmresult.fcode) {
          case _CMKEY:
            switch (cmresult.nresult) {
              case DEL_LIS:
              case DEL_VRB:
                listing = 1;
                break;
              case DEL_NOL:
              case DEL_QUI:
                listing = 0;
                break;
            }
            break;
          case _CMIFI:
            s = cmresult.sresult;
            havename = 1;
            break;
          default:
            return(-2);
        }
    }
    wild = cmresult.nresult;            /* Source specification wild? */

    ckstrncpy(line,s,LINBUFSIZ);        /* Make a safe copy of source name */
    s = line;

    if (!wild)
      wild = iswild(line);

    p = tmpbuf;                         /* Place for new name */
    if ((x = cmofi(wild ? "Target directory" : "New name",
                   "",&s,xxstring)) < 0) { /* Get new name */
        if (x == -3) {
            printf("?%s required\n", wild ? "Target directory" : "New name");
            return(-9);
        } else return(x);
    }
    ckstrncpy(p,s,TMPBUFSIZ);           /* Make a safe copy of the new name */
    if ((y = cmcfm()) < 0) return(y);

    if (!wild) {                        /* Just one */
        if (listing) printf("%s => %s ",line,p);
        if (zlink(line,p) < 0) {
            if (listing) printf("(FAILED: %s\n",ck_errstr());
            rc = 0;
        } else {
            if (listing) printf("(OK)\n");
        }
        return(success = rc);
    }
    if (!isdir(p)) {                    /* Multiple */
        printf(                         /* if target is not a directory */
"?Multiple source files not allowed if target is not a directory.\n");
        return(-9);
    }
#ifdef COMMENT
    else {                              /* Show full path of target */
        char buf[CKMAXPATH];            /* (too much) */
        if (zfnqfp(p,CKMAXPATH,buf))
          ckstrncpy(tmpbuf,buf,TMPBUFSIZ);
    }
#endif /* COMMENT */

#ifdef VMS
    conres();                           /* Let Ctrl-C work. */
#endif /* VMS */
    debug(F110,"dolink line",line,0);

#ifdef ZXREWIND
    z = zxrewind();                     /* Rewind file list */
#else
    z = nzxpand(s,0);                   /* Expand file list */
#endif /* ZXREWIND */
    debug(F111,"dolink p",p,z);

#ifdef UNIX
    if (wild && z > 1)
      sh_sort(mtchs,NULL,z,0,0,filecase); /* Alphabetize the filename list */
#endif /* UNIX */

    while (z-- > 0) {
        if (!(z == 0 && !wild))
          znext(line);
        if (!line[0])
          break;
        if (listing) printf("%s => %s ",line,p);
        if (zlink(line,p) < 0) {
            if (listing) printf("(FAILED: %s\n",ck_errstr());
            rc = 0;
        } else {
            if (listing) printf("(OK)\n");
        }
    }
#ifdef VMS
    concb((char)escape);
#endif /* VMS */
    return(success = rc);
}
#endif /* NT */

#ifdef ZCOPY
int
docopy() {
    int i, x, listing = 0, nolist = 0, havename = 0, getval;
    char c;
    struct FDB sw, fi;
    int overwrite = OVW_ALWAYS;
    int targetisdir = 0;
    int targetlen = 0;
    int appending = 0;
    int preserve = 0;
    int swapping = 0;
    int fromb64 = 0;
    int tob64 = 0;
    int wild = 0;
    int rc = 1;

    char newname[CKMAXPATH], * nm;
    nm = newname;

    cmfdbi(&sw,                         /* 1st FDB - switches */
           _CMKEY,                      /* fcode */
           "Filename or switch",        /* hlpmsg */
           "",                          /* default */
           "",                          /* addtl string data */
           ncopytab,                    /* addtl numeric data 1: tbl size */
           4,                           /* addtl numeric data 2: 4 = cmswi */
           xxstring,                    /* Processing function */
           copytab,                     /* Keyword table */
           &fi                          /* Pointer to next FDB */
           );
    cmfdbi(&fi,                         /* 2nd FDB - file to copy */
           _CMIFI,                      /* fcode */
           "",                          /* hlpmsg */
           "",                          /* default */
           "",                          /* addtl string data */
           0,                           /* addtl numeric data 1 */
           0,                           /* addtl numeric data 2 */
           xxstring,
           NULL,
           NULL
           );

    while (!havename) {
        x = cmfdb(&sw);                 /* Parse something */
        if (x < 0)                      /* Error */
          return(x);
        switch (cmresult.fcode) {
          case _CMKEY:
	    c = cmgbrk();                   /* Get break character */
	    if ((getval = (c == ':' || c == '=')) && !(cmgkwflgs() & CM_ARG)) {
		printf("?This switch does not take an argument\n");
		rc = -9;
		return(rc);
	    }
	    if (!getval && (cmgkwflgs() & CM_ARG)) {
		printf("?This switch requires an argument\n");
		rc = -9;
		return(rc);
	    }
            switch (cmresult.nresult) {
              case DEL_LIS:
              case DEL_VRB:
		nolist = 0;
                listing = 1;
                break;
              case DEL_NOL:
              case DEL_QUI:
		nolist = 1;
                listing = 0;
                break;
              case 999:
                swapping = 1;
                break;
              case 998:
                appending = 1;
                break;
	      case 995:
		preserve = 1;
                break;
	      case 994:
		if ((x = cmkey(ovwtab,novwtab,
			       "When to overwrite existing destination file",
			       "",xxstring)) < 0)
		  return(x);
		overwrite = x;
                break;
#ifndef NOSPL
              case 997:
                fromb64 = 1;
                break;
              case 996:
                tob64 = 1;
                break;
#endif /* NOSPL */
            }
            break;
          case _CMIFI:
            s = cmresult.sresult;
            havename = 1;
            break;
          default:
            return(-2);
        }
    }
    wild = cmresult.nresult;
    ckstrncpy(line,s,LINBUFSIZ);        /* Make a safe copy of source name */
    s = line;
    p = tmpbuf;                         /* Place for new name */

    /* Get destination name */
    if ((x = cmofi("destination name and/or directory",
#ifdef UNIX
                   "."
#else
                   ""
#endif /* UNIX */
                   ,&s,xxstring)) < 0) {
        if (x == -3) {
            printf("?Name for destination file required\n");
            return(-9);
        } else return(x);
    }
    ckstrncpy(p,s,TMPBUFSIZ);           /* Safe copy of destination name */
    if ((y = cmcfm()) < 0) return(y);
    if (appending && swapping) {
        printf("?Sorry, /APPEND and /SWAP conflict\n");
        return(-9);
    }
#ifdef COMMENT
/*
  This unreasonably prevented "COPY /APPEND *.* bigfile" from concatenating
  a bunch of files into one big file.
*/
    if (appending && wild) {
        printf("?Sorry, /APPEND can be used only with single files\n");
        return(-9);
    }
#endif /* COMMENT */
    targetisdir = isdir(p);
    x = strlen(p);
    if (targetisdir) {
#ifdef UNIXOROSK
        if (p[x-1] != '/') {
            ckstrncat(p,"/",TMPBUFSIZ);
            x++;
        }
#else
#ifdef OS2
        if (p[x-1] != '/') {
            ckstrncat(p,"/",TMPBUFSIZ);
            x++;
        }
#else
#ifdef STRATUS
        if (p[x-1] != '>') {
            ckstrncat(p,">",TMPBUFSIZ);
            x++;
        }
#else
#ifdef datageneral
        if (p[x-1] != ':') {
            ckstrncat(p,":",TMPBUFSIZ);
            x++;
        }
#else
        if (p[x-1] != '/') {
            ckstrncat(p,"/",TMPBUFSIZ);
            x++;
        }
#endif /* datageneral */
#endif /* STRATUS */
#endif /* OS2 */
#endif /* UNIXOROSK */
    }
    targetlen = x;

    if (!appending) {			/* If /APPEND not given */
	if (wild && !targetisdir) {	/* No wildcards allowed */
	    printf(			/* if target is not a directory */
"?Multiple source files not allowed if target is not a directory.\n");
	    return(-9);
	}
    }

#ifdef VMS
    conres();                           /* Let Ctrl-C work. */
#endif /* VMS */
    debug(F110,"docopy line",line,0);
    debug(F110,"docopy p",p,0);
    debug(F110,"docopy nm",nm,0);

#ifdef ZXREWIND
    z = zxrewind();                     /* Rewind file list */
#else
    z = nzxpand(s,0);                   /* Expand file list */
#endif /* ZXREWIND */

#ifdef UNIX
    if (wild)
      sh_sort(mtchs,NULL,z,0,0,filecase); /* Alphabetize the filename list */
#endif /* UNIX */

#ifdef IKSD
    if (!targetisdir && zchki(p) > -1) { /* Destination file exists? */
        if (inserver && (!ENABLED(en_del)
#ifdef CK_LOGIN
                         || isguest
#endif /* CK_LOGIN */
                         )) {
            printf("?Sorry, overwriting existing files is disabled\n");
            return(-9);
        }
    }
#endif /* IKSD */

    if (tob64 && fromb64) {             /* To and from B64 = no conversion */
        tob64 = 0;
        fromb64 = 0;
    }
    debug(F110,"COPY dest",p,0);

    while (z > 0) {

        znext(line);
        if (!line[0])
          break;

        errno = 0;                      /* Reset errno */

	if (targetisdir) {
	    zstrip(line,&nm);
	    ckmakmsg(newname,CKMAXPATH,p,nm,NULL,NULL);
	    nm = newname;
	} else {
	    nm = p;
	}
	if (overwrite) {		/* Overwrite checking? */
	    if (zchki(nm) >= (CK_OFF_T)0) { /* Destination file exists? */

		char d1[20], * d2;
		char * n1, * n2;
		int i, skip = 0;

		i = strlen(line);	/* Isolate source filename */
		for (; i >= 0; i--) {
		    if (ISDIRSEP(line[i])) {
			n1 = &line[i+1];
			break;
		    }
		}
		debug(F110,"COPY n1", n1, 0);
		i = strlen(nm);		/* And destination filename */
		for (; i >= 0; i--) {
		    if (ISDIRSEP(nm[i])) {
			n2 = &nm[i+1];
			break;
		    }
		}
		debug(F110,"COPY n2", n2, 0);
		if (!strcmp(n1,n2)) {          	  /* Same name? */
		    if (overwrite == OVW_NEVER) { /* Never overwrite? */
			if (listing)	          /* Skip */
			  if (listing) printf("%s => %s (SKIPPED)\n",line,nm);
			continue;
		    }
		    ckstrncpy(d1,zfcdat(line),20); /* Source file timestamp */
		    d2 = zfcdat(nm);	       /* Timestamp of dest file */
		    x = strcmp(d1,d2);	       /* Compare them */
		    if (((overwrite == OVW_NEWER) && (x < 0)) ||
			((overwrite == OVW_OLDER) && (x > 0))) {
			if (listing)
			  if (listing) printf("%s => %s (SKIPPED)\n",line,nm);
			continue;
		    }
		}
	    }
	}
        if (listing) printf("%s => %s ",line,nm);

        /* Straight copy */
        if (!swapping && !appending && !fromb64 && !tob64) {
            debug(F110,"COPY zcopy",line,0);

            if ((x = zcopy(line,p)) < 0) { /* Let zcopy() do it. */
		debug(F111,"COPY not OK",line,x);
                switch (x) {
                  case -2:
		    if (listing)
		      printf("(FAILED: Not a regular file)\n");
		    else if (!nolist)
		      printf("?Not a regular file - %s\n",line);
                    rc = 0;
                    break;
                  case -3:
		    if (listing)
		      printf("(FAILED: Not found or not accessible)\n");
		    else if (!nolist)
		      printf("?Not found or not accessible - %s\n",line);
                    rc = 0;
                    break;
                  case -4:
		    if (listing)
		      printf("(FAILED: Permission denied)\n");
		    else if (!nolist)
		      printf("?Permission denied - %s\n",line);
                    rc = 0;
                    break;
                  case -5:
		    if (listing)
		      printf("(Source and destination are the same file)\n");
		    else if (!nolist)
		      printf(
			  "?Source and destination are the same file - %s\n",
			  line
			  );
                    break;
                  case -6:
		    if (listing)
		      printf("(FAILED: Input/Output error)\n");
		    else if (!nolist)
		      printf("?Input/Output error - %s\n",line);
                    rc = 0;
                    break;
                  case -7:
		    if (listing)
		      printf("(FAILED: %s - %s)\n",p,ck_errstr());
		    else if (!nolist)
		      printf("?%s - %s\n",ck_errstr(),p);
                    rc = 0;
                    break;
                  default:
		    if (listing)
		      printf("(FAILED: %s)\n",ck_errstr());
		    else if (!nolist)
		      printf("?%s\n",ck_errstr());
                    rc = 0;
                }
            } else {			/* Regular copy succeeded */
		debug(F110,"COPY OK..",newname,0);
#ifndef NOXFER
		if (preserve) {		/* Handle /PRESERVE */
		    char * pstr = "";	/* File permissions string */
		    struct zattr xx;	/* File attribute structure */
		    extern char * cksysid;

		    initattr(&xx);	/* Initialize the struct */

		    xx.systemid.val = cksysid; /* Set our system ID */
		    xx.systemid.len = (int)strlen(cksysid);
#ifdef CK_PERMS
		    pstr = zgperm(line); /* Get source file's permissions */
#endif /* CK_PERMS */
		    xx.lprotect.val = pstr;
		    xx.lprotect.len = (int)strlen(pstr);
		    xx.date.val = zfcdat(line);	/* Source file's timestamp */
		    xx.date.len = (int)strlen(xx.date.val);
		    if (zstime(nm,&xx,0) < 0) {
			printf("?COPY /PRESERVE %s: %s\n",nm,ck_errstr());
			rc = -9;
		    }
		}
#endif	/* NOXFER */
		if (listing && rc > -1)
		  printf("(OK)\n");
            }

        } else {                        /* Special options */

            int prev, y, x = 0;         /* Variables needed for them */
            int i, t;
            char ibuf[100];
            char obuf[200];
            FILE * in = NULL;
            FILE * out = NULL;

            if ((in = fopen(line,"r")) == NULL) { /* Open input file */
                if (listing)
		  printf("(FAILED: %s)\n",ck_errstr());
		else if (!nolist)
		  printf("?%s - %s)\n",ck_errstr(),line);
                rc = 0;
                continue;
            }
            if (targetisdir) {          /* Target is directory */
                char * buf = NULL;      /* so append this filename to it */
                zstrip(line,&buf);
                p[targetlen] = NUL;
                if (buf)
                  ckstrncat(p,buf,TMPBUFSIZ);
            }
#ifdef OS2ORUNIX
            if (zcmpfn(line,p)) {       /* Input and output are same file? */
                if (listing)
                  printf("(FAILED: Source and destination identical)\n");
		else if (!nolist)
                  printf("?Source and destination identical - %s\n", line); 
                rc = 0;
                continue;
            }
#endif /* OS2ORUNIX */
            if ((out = fopen(p, (appending ? "a" : "w"))) == NULL) {
                fclose(in);
                if (listing)
		  printf("(FAILED: %s - %s)\n",p,ck_errstr());
		else if (!nolist)
		  printf("?%s - %s\n",p,ck_errstr());
                rc = 0;
                continue;
            }
#ifndef NOSPL
            if (tob64) {                /* Converting to Base-64 */

                debug(F110,"COPY tob64",line,0);

                while (1) {             /* Loop... */
                    prev = x;
                    if ((x = fread(ibuf,1,54,in)) < 1) { /* EOF */
                        if (listing)
                          printf("(OK)\n");
                        break;
                    }
                    if (prev % 3) {
                        if (listing)
                          printf("(FAILED: Phase error at %d)\n",prev);
			else if (!nolist)
                          printf("?Phase error at %d\n",prev);
                        rc = 0;
                        break;
                    }
                    if (swapping) {
                        if (x & 1) {
                            if (listing)
                              printf("(FAILED: Swap error)\n");
			    else if (!nolist)
			      printf("?Swap error\n");
                            rc = 0;
                            break;
                        }
                        for (i = 0; i < x; i+=2) {
                            t = ibuf[i];
                            ibuf[i] = ibuf[i+1];
                            ibuf[i+1] = t;
                        }
                    }
                    if ((y = b8tob64(ibuf,x,obuf,180)) < 0) {
                        if (listing)
                          printf("(FAILED: Encoding error)\n");
			else if (!nolist)
			  printf("?Encoding error\n");
                        rc = 0;
                        break;
                    }
                    fprintf(out,"%s\n",obuf);
                }

            } else if (fromb64) {       /* Converting from Base 64 */

                debug(F110,"COPY fromb64",line,0);

                if ((out = fopen(p,appending ? "a" : "w")) == NULL) {
                    fclose(in);
                    if (listing)
		      printf("(FAILED: %s - %s)\n",p,ck_errstr());
		    else if (!nolist)
		      printf("?%s - %s\n",p,ck_errstr());
                    rc = 0;
                    continue;
                }
                x = 1;
                while (x) {
                    x = fread(ibuf,1,80,in);
                    if ((y = b64tob8(ibuf,x,obuf,80)) < 0) {
                        if (listing)
                          printf("(FAILED: Decoding error)\n");
			else if (!nolist)
			  printf("?Decoding error\n");
                        rc = 0;
                        break;
                    }
                    if (swapping) {
                        if (x & 1) {
                            if (listing)
                              printf("(FAILED: Swap error)\n");
			    else if (!nolist)
			      printf("?Swap error\n");
                            rc = 0;
                            break;
                        }
                        for (i = 0; i < y; i+=2) {
                            t = obuf[i];
                            obuf[i] = obuf[i+1];
                            obuf[i+1] = t;
                        }
                    }
                    if (y > 0) {
                        if (fwrite(obuf,1,y,out) < 1) {
                            if (listing)
                              printf("(FAILED: %s - %s)\n",p,ck_errstr());
			    else if (!nolist)
			      printf("?%s - %s\n",p,ck_errstr());
                            rc = 0;
                            break;
                        }
                    }
                }

            } else
#endif /* NOSPL */

            if (swapping) {             /* Swapping bytes */

                CHAR c[3];
                c[2] = NUL;

                debug(F110,"COPY swapping",line,0);

                while (1) {
                    x = fread((char *)c,1,2,in);
                    if (x < 1) {
                        if (listing)
                          printf("(OK)\n");
                        break;
                    } else if (x == 1) {
                        c[1] = c[0];
                        c[0] = NUL;
                        printf(
                            "(WARNING: Odd byte count)");
                        if (!listing) printf("\n");
                    }
                    if (fprintf(out,"%c%c",c[1],c[0]) == EOF) {
                        if (listing)
                          printf("(FAILED: %s - %s)\n",p,ck_errstr());
			else if (!nolist)
                          printf("?%s - %s\n",p,ck_errstr());
                        rc = 0;
                        break;
                    }
                }

            } else if (appending) {     /* Appending to target file */

                char c;

                debug(F110,"COPY appending",line,0);

                while (1) {
                    x = fread(&c,1,1,in);
                    if (x < 1) {
                        if (listing)
                          printf("(OK)\n");
                        break;
                    }
                    if (fwrite(&c,1,1,out) < 1) {
                        if (listing)
                          printf("(FAILED: %s - %s)\n",p,ck_errstr());
			else if (!nolist)
                          printf("?%s - %s\n",p,ck_errstr());
                        rc = 0;
                        break;
                    }
                }
            }
            if (out) fclose(out);
            if (in) fclose(in);
        }
#ifdef VMSORUNIX
        concb((char)escape);
#endif /* VMSORUNIX */
    }
    if (rc > -1) success = rc;
    return(rc);
}
#endif /* ZCOPY */
#endif /* NOFRILLS */

#ifndef NOCSETS
#ifndef NOUNICODE
static struct keytab * xfcstab = NULL;	/* For RENAME /CONVERT: */
static char cvtbufin[CKMAXPATH+8] = { NUL, NUL };
static char cvtbufout[CKMAXPATH+8] = { NUL, NUL };
static char * pcvtbufin = NULL;
static char * pcvtbufout = NULL;

static int				/* Input function xgnbyte() */
cvtfnin() {
    CHAR c;
    c = *pcvtbufin++;
    return(c ? c : -1);
}

_PROTOTYP(int cvtfnout,(char));		/* Output function for xpnbyte() */
int
#ifdef CK_ANSIC
cvtfnout(char c)
#else
cvtfnout(c) char c;
#endif	/* CK_ANSIC */
{
    if (pcvtbufout - cvtbufout >= CKMAXPATH)
      return(-1);
    *pcvtbufout++ = c;
    *pcvtbufout = NUL;
    return(1);
}

/* Convert a string from any charset to any other charset */

char *
cvtstring(s,csin,csout) char * s; int csin, csout; {
    int c;
    extern CK_OFF_T ffc;

    ckstrncpy(cvtbufin,s,CKMAXPATH);	/* Put it in a public place */
    pcvtbufin = cvtbufin;		/* with public pointers */
    pcvtbufout = cvtbufout;
    *pcvtbufout = NUL;

    if (csin == csout)			/* If the two sets are the same */
      return((char *)cvtbufin);		/* don't bother converting */

    initxlate(csin,csout);		/* Initialize the translator */

    while ((c = xgnbyte(FC_UCS2,csin,cvtfnin)) > -1) { /* Loop thru string */
	if (xpnbyte(c,TC_UCS2,csout,cvtfnout) < 0) {
	    ffc = (CK_OFF_T)0;
	    return("");
	}
    }
    /* ffc is touched by xgnbyte() but this is not file transfer */
    /* so we have to undo it */
    ffc = (CK_OFF_T)0;
    return((char *)cvtbufout);
}
#endif /* NOUNICODE */
#endif /* NOCSETS */

#ifndef NORENAME
#ifndef NOFRILLS
#ifdef ZRENAME

/* The RENAME command - expanded and improved in 8.0.212 April 2006 */

static char * ren_sub[4] = { NULL,NULL,NULL,NULL }; /* For RENAME /REPLACE */

int ren_list = 0;		        /* Default listing action for RENAME */
int ren_coll = RENX_OVWR;		/* Default collision action */

int
shorename() {
    char * s;
    switch (ren_coll) {
      case RENX_FAIL: s = "fail"; break;
      case RENX_OVWR: s = "overwrite"; break;
      case RENX_SKIP: s = "proceed"; break;
    }
    printf(" rename collision: %s\n",s);
    printf(" rename list:      %s\n",showoff(ren_list));
    return(1);
}

int
setrename() {				/* Parse SET RENAME options */
    int x, y;
    if ((x = cmkey(renamset,nrenamset,"","", xxstring)) < 0)
      return(x);
    switch (x) {
      case REN_OVW:			/* COLLISION */
	if ((x = cmkey(r_collision,nr_collision,"","", xxstring)) < 0)
	  return(x);
	if ((y = cmcfm()) < 0)
	  return(y);
	ren_coll = x;
	break;
      case DEL_LIS:			/* LIST */
	return(seton(&ren_list));
    }
    return(success = 1);
}

/* Reverse a string - Assumes a single-byte character set */

int
gnirts(s1, s2, len) char * s1, * s2; int len; {
    int n, m = 0;
    if (!s1)				/* Null source pointer, fail */
      return(0);
    n = (int) strlen(s1);
    if (n > len-1)			/* Source longer than dest, fail */
      return(0);
    s2[n--] = NUL;			/* Deposit null byte at end of dest */
    for (; n >= 0; n--) {		/* Copy the rest backwards */
	*s2++ = s1[n];
	m++;
    }
    return(m);
}

/*
  r e n a m e o n e

  Worker function to rename one file for dorenam() (below).
    old        = name of file or directory to be renamed
    new        = new name (not required for /UPPER, /LOWER, and /REPLACE)
    replacing  = 1 if doing string replacement on the name  
    casing     = 1 if converting name to lowercase, 2 if to uppercase
    all        = if doing case conversion on all names, not just monocase ones
    converting = 1 if converting character sets
    cset1      = character set to convert from (File Character Set index)
    cset2      = character set to convert to (ditto, see ck?xla.h)
    listing    = 1 to show results of rename
    nolist     = 1 to be completely silent (don't even print error messages)
    op         = 1 means simulate, 2 means check for collision, 0 means rename
    size       = length of result buffer.
    collision  = action to take if destination file already exists:
                 0 = fail
                 1 = overwrite and succeed
                 2 = skip and succeed
  Returns:
     0: on failure to rename or when a forbidden collision would have occurred.
     1: on success (file was renamed or did not need to be renamed).
  Note:
     If this code is ever built on any platform that is not Unix, Windows,
     VMS, or OS/2, this routine might need some adjustment.
*/

/* Opcodes for op... */
#define REN_OP_SIM 1			/* Simulate */
#define REN_OP_CHK 2			/* Check for collisions */

static int
renameone(old,new,
	  replacing,casing,all,converting,cset1,cset2,
	  listing,nolist,op,size,collision)
    char * old, * new;
    int replacing,casing,all,converting,cset1,cset2,
    listing,nolist,op,size,collision;
{
    char buf[CKMAXPATH];		/* Temporary filename buffer */
    char out[CKMAXPATH];		/* Buffer for new name */
    char dir[CKMAXPATH];		/* Destination directory */
    char pat[CKMAXPATH];		/* Path segment on old filename */

    char * destdir;			/* Destination directory, if any */
    char * srcpath;			/* Source path, if any */
    int rc = 1, flag = 0, skip = 0;	/* Control */
    int honorcase = 0, replaced = 0;
    int anchor = 0;			/* 1 = beginning, 2 = end */
    int occur = 0;			/* Occurrence */
    int minus = 0;			/* Occurrence is negative */
    int allbut = 0;			/* Occurrence is "all but" */
    int arg2isfile = 0;			/* Arg2 ("new") is a filename */
    
    debug(F110,"RENAMEONE old",old,0);
    debug(F110,"RENAMEONE new",new,0);
    debug(F110,"RENAMEONE ren_sub[0]",ren_sub[0],0);
    debug(F110,"RENAMEONE ren_sub[1]",ren_sub[1],0);
    debug(F110,"RENAMEONE ren_sub[2]",ren_sub[2],0);

    if (op == REN_OP_SIM && !nolist)	/* For convenience */
      listing = 1;
#ifndef NOSPL
    honorcase = inpcas[cmdlvl];		/* Inherit SET CASE value */
#else
#ifdef UNIX
    honorcase = 1;
#else
    honorcase = 0;
#endif	/* UNIX */
#endif	/* NOSPL */

    if (!old) old = "";			/* In case of bad args */
    if (!new) new = "";
    if (!*old) return(success = 0);
    ckstrncpy(out,new,CKMAXPATH);	/* So we don't write into */
    new = out;				/* our argument... */
    size = CKMAXPATH;

    pat[0] = NUL;			/* Assume no path in source file.. */
    srcpath = pat;
    {
	int n;				/* If the old name includes a path */
	n = (int)strlen(old) - 1;	/* put it in a separate place.     */
	for (; n >= 0; n--) {		/* We are renaming the file only.  */
	    if (ISDIRSEP(old[n])) {
		ckstrncpy(pat,old,CKMAXPATH);
		pat[n+1] = NUL;
		old = old+n+1;
		break;
	    }
	}
    }
    debug(F110,"RENAMEONE old 2",old,0);
    debug(F110,"RENAMEONE pat 2",pat,0);

    dir[0] = NUL;			/* Assume no destination directory */
    destdir = dir;
    if (*new) {				/* If Arg2 given */
	if (isdir(new)) {		/* If it's a directory */
	    ckstrncpy(dir,new,CKMAXPATH); /* put it here */
	} else {			/* otherwise */
	    arg2isfile++;		/* flag that it's a filename */
	}    
    }
    if (!casing && !replacing && !converting) {
	if (!*new)
	  return(success = 0);
	if (!arg2isfile) {		/* Destination is a directory? */
	    if (!isdir(old)) {		/* and source is not? */
#ifndef VMS
		int n, x = 0;		/* Concatenate them */
		if ((n = strlen(new)) > 0) /* so we can check for */
		  if (ISDIRSEP(new[n-1]))  /* collisions. */
		    x++;
		ckmakmsg(buf,size,new,x ? "" : "/", old, "");
#else
		ckmakmsg(buf,size,new, old, NULL, NULL);
#endif	/* VMS */
		debug(F110,"RENAMEONE new new",new,0);		
		new = buf;
		size = CKMAXPATH;
	    }
	}
    } else if (*new) {			/* Directory to move file to */
	int n, x = 0;			/* after changing its name */
	if (!isdir(new))
	  return(success = 0);
#ifndef VMS
	if ((n = strlen(new)) > 0)
	  if (ISDIRSEP(new[n-1]))
	    x++;
	ckmakmsg(dir,CKMAXPATH,new,x ? "" : "/", "", "");
#else
	ckstrncpy(dir,new,CKMAXPATH);
#endif	/* VMS */
    }

#ifndef NOCSETS
#ifndef NOUNICODE
    if (converting) {
	new = cvtstring(old,cset1,cset2);
    }
#endif	/* NOUNICODE */
#endif	/* NOCSETS */

    if (replacing) {			/* Replacing strings */
	int todo = 0;
	int len0, len1, len2;
	char c, *p, *s, *bp[3];

	bp[0] = old;			/* Original name */
	bp[1] = ren_sub[0];		/* String to be replaced */
	bp[2] = ren_sub[1];		/* What to replace it with */
	if (!bp[2]) bp[2] = "";

	len0 = (int)strlen(bp[0]);	/* length of original filename */
	len1 = (int)strlen(bp[1]);	/* length of target substring */
	len2 = (int)strlen(bp[2]);	/* Length of replacement string */

	if (ren_sub[2]) {		/* Optional options */
	    p = ren_sub[2];
	    while ((c = *p++)) {
		switch (c) {
		  case '^':
		    anchor = 1; occur = 0; minus = 0; allbut = 0; break;
		  case '$':
		    anchor = 2; occur = 0; minus = 0; allbut = 0; break;
		  case 'A': honorcase = 1; minus = 0; allbut = 0; break;
		  case 'a': honorcase = 0; minus = 0; allbut = 0; break;
		  case '-': minus = 1; break;
		  case '~': allbut = 1; break;
		  default:
		    if (isdigit(c)) {
			occur = c - '0';
			if (minus) occur = 0 - occur;
			anchor = 0;
		    }
		    minus = 0;
		}
	    }
	}
	if (anchor) {			/* Anchored replacement... */
	    y = len0 - len1;
	    if (y > 0) {
		int x;
		switch (anchor) {
		  case 1:		/* Anchored at beginning */
		    if (!ckstrcmp(bp[1],bp[0],len1,honorcase)) {
			x = ckstrncpy(new,bp[2],size);
			(VOID) ckstrncpy(new+x,bp[0]+len1,size-x);
			replaced = 1;
		    }
		    break;
		  case 2:		/* Anchored at end */
		    if (!ckstrcmp(bp[1],bp[0]+y,len1,honorcase)) {
			x = ckstrncpy(new,bp[0],y+1);
			(VOID) ckstrncpy(new+y,bp[2],size-x);
			replaced = 1;
		    }
		    break;
		}
	    }
	    if (!replaced) {
		ckstrncpy(new,old,size); /* Keep old name */
		replaced = 1;
	    }
	} else {			/* Replace all occurrences */
	    int j, n = 0;		/* or a particular occurrence */
	    char c;
	    int x = 0;
	    char * s0 = NULL, * s1 = NULL, * s2 = NULL;
	    p = new;			/* Pointer to new name */

	    if (occur < 0) {		/* nth occurrence from the right */
		occur = 0 - occur;
		s0 = (char *)malloc(len0+1); /* Reverse original string */
		if (s0) {
		    (VOID) gnirts(bp[0],s0,len0+1);
		    bp[0] = s0;
		} else return(0); 
		s1 = (char *)malloc(len1+1); /* Reverse target string */
		if (s1) {
		    (VOID) gnirts(bp[1],s1,len1+1);
		    bp[1] = s1;
		} else return(0); 
		s2 = (char *)malloc(len2+1); /* Reverse replacement string */
		if (s2) {
		    (VOID) gnirts(bp[2],s2,len2+1);
		    bp[2] = s2;
		} else return(0); 
		debug(F111,"RENAMEONE s0",s0,len0);
		debug(F111,"RENAMEONE s1",s1,len1);
		debug(F111,"RENAMEONE s2",s2,len2);
	    }
	    s = bp[0];			/* Pointer to old name */
	    p = new;			/* Pointer to new name */
	    j = len0 - len1 + 1;	/* How much to scan */
	    while (j-- > 0) {		/* For each character... */
		if (!ckstrcmp(bp[1],s,len1,honorcase)) { /* Match? */
		    n++;		/* Occurrence counter */
		    todo = (occur == 0) ||
			(!allbut && n == occur) ||
			(allbut && n != occur);
		    if (!todo) {	/* Desired occurrence? */
			size -= ckstrncpy(p,bp[1],size); /* No... */
			p += len1;	/* Copy target string */
			s += len1;	/* instead of replacement string */
			continue;
		    }
		    if (len2) {		/* If replacement string not empty */
			size -= ckstrncpy(p,bp[2],size); /* Copy it */
			p += len2;
		    }
		    s += len1;		/* Advance source position */
		} else {		/* No match */
		    *p++ = *s++;	/* just copy this character */
		    size--;
		}
	    }
	    while ((*p++ = *s++));	/* Done copy the rest */
	    replaced = 1;		/* Remember we changed the name */
	    if (s0) {			/* Were we doing "all but"? */
		debug(F110,"RENAMEONE new1",new,0);
		x = (int)strlen(new);	/* Unreverse the result */
		if ((p = (char *)malloc(x+2))) {
		    (VOID) gnirts(new,p,x+2);
		    debug(F110,"RENAMEONE new2",new,0);
		    ckstrncpy(new,p,x+2);
		    free(p);
		}
		if (s0) free(s0);	/* Free the temporary strings */
		if (s1) free(s1);
		if (s2) free(s2);
		debug(F110,"RENAMEONE new3",new,0);
	    }
	}
    }
    if (casing) {			/* Changing case? */
	char c, * t;			/* See if mixed case. */
	if (!replaced)
	  ckstrncpy(new,old,size);	/* Copy old name to new name */
	t = new;
	while ((c = *t++)) {
	    if (islower(c)) flag |= 1; /* Have a lowercase letter */
	    else if (isupper(c)) flag |= 2; /* Have an uppercase letter */
	    if (flag == 3) break;	/* Have a mixed-case name */
	}
	if (all || flag < 3) {	/* Not skipping or not mixed case */
	    if (casing == 1 && flag != 1) /* Change case to lower */
	      (VOID) cklower(new);
	    else if (casing == 2 && flag != 2) /* Change case to upper */
	      (VOID) ckupper(new);
	}
    }
    debug(F110,"XXX 1 new",new,0);
    debug(F110,"XXX 1 old",old,0);
    debug(F110,"XXX 1 srcpath",srcpath,0);
    debug(F110,"XXX 1 destdir",destdir,0);

    if (*destdir && !arg2isfile) {	/* Moving without renaming */
	ckstrncat(srcpath,old,CKMAXPATH);
	old = srcpath;
	new = destdir;
    } else if (*destdir || *srcpath) {	/* Were there any pathnames? */
	char tmp[CKMAXPATH];
	ckmakmsg(tmp,CKMAXPATH,srcpath,old,NULL,NULL);
	ckstrncpy(old,tmp,CKMAXPATH);
	if (*destdir) {			/* Directory-to-move-to given? */
	    ckstrncat(destdir,new,CKMAXPATH);
	    new = destdir;
	} else if (*srcpath && !arg2isfile) { /* Or was there a source path? */
	    ckstrncat(srcpath,new,CKMAXPATH);
	    new = srcpath;
	}
    }
    debug(F110,"XXX 2",new,0);

    skip = 0;				/* Can we skip this one? */
#ifdef COMMENT
    if (casing && !replaced) {
	skip = (((all == 0) && (flag == 3)) || (flag == casing)) ? 1 : 0;
	if (!skip && destdir) skip = 0;
    }
#endif	/* COMMENT */
    if (!skip) {
	if (!ckstrcmp(old,new,-1,1))
	  skip = 1;
    }
    if (op == 0 && !skip && (collision != RENX_OVWR)) {
	if (zchki(new) > (CK_OFF_T)-1) { /* New file already exists?  */
	    switch (collision) {	/* Yes, take specified action */
	      case RENX_SKIP:		/* Skip this one and proceed  */
		skip = 2;
		break;
	      case RENX_FAIL:		/* Or fail. */
		skip = 3;
		if (!listing && !nolist)
		  printf("?File already exists: %s\n",new);
	    }
	}
    }
    debug(F110,"RENAMEONE new",new,0);
    debug(F101,"RENAMEONE flag","",flag);	
    debug(F101,"RENAMEONE skip","",skip);	
    debug(F100,"RENAMEONE ----------------","",0);

    if (skip == 3) {
	if (listing) printf("%s => %s (SKIPPED: %s already exists)\n",
			    old,new,new);
	rc = 0;
    } else if (skip) {			/* Skipping this one */
	if (listing) printf("%s => %s (%s)\n",
			    old,new,
			    (skip == 2) ? "COLLISION: SKIPPED" : "SKIPPED");
    } else {				/* Have to rename this one */
	if (op == REN_OP_CHK) {		/* Checking for collisions */
	    return((zchki(new) > (CK_OFF_T)-1) ? 0 : 1 );
	} else if (op == REN_OP_SIM) {	/* Simulating */
	    if (listing) printf("%s => %s (SIMULATED)\n",old,new);
	} else {			/* Really renaming */
	    if (listing) printf("%s => %s ",old,new);
	    if (zrename(old,new) < 0) {
		rc = 0;
		if (listing)
		  printf("(FAILED: %s)\n",ck_errstr());
		else if (!nolist)
		  printf("?%s\n",ck_errstr());
	    } else {
		if (listing) printf("(OK)\n");
	    }
	}
    }
    return(success = rc);  /* Succeeds also if nothing needed to be renamed */
}

int
dorenam() {
#ifndef NOCSETS
#ifndef NOUNICODE
    extern int nfilc;
    extern struct keytab fcstab[];
    extern struct csinfo fcsinfo[];
#endif	/* NOUNICODE */
#endif	/* NOCSETS */
    int cset1 = 0, cset2 = 0;

    int i, x, z, fn, listing = 0, havename = 0, wild = 0, rc = 1, noarg = 0;
    int nolist = 0, all = 0, casing = 0, replacing = 0, getval = 0, sim = 0;
    int converting = 0, collision = 0;

    char c;
    struct FDB sw, fi;

    collision = ren_coll;		/* Inherit SET RENAME COLLISION */
    listing = ren_list;			/* Inhereit SET RENAME LIST */

    if (ren_sub[0]) makestr(&(ren_sub[0]),NULL);
    if (ren_sub[1]) makestr(&(ren_sub[1]),NULL);
    if (ren_sub[2]) makestr(&(ren_sub[2]),NULL);
    line[0] = NUL;
    tmpbuf[0] = NUL;

    cmfdbi(&sw,                         /* 1st FDB - switches */
           _CMKEY,                      /* fcode */
           "Filename or switch",        /* hlpmsg */
           "",                          /* default */
           "",                          /* addtl string data */
           nrenamsw,			/* addtl numeric data 1: tbl size */
           4,                           /* addtl numeric data 2: 4 = cmswi */
           xxstring,                    /* Processing function */
           renamsw,                     /* Keyword table */
           &fi                          /* Pointer to next FDB */
           );

    cmfdbi(&fi,                         /* 2nd FDB - file or directory name */
           _CMIFI,                      /* fcode */
           "",                          /* hlpmsg */
           "",                          /* default */
           "",                          /* addtl string data */
           3,                           /* Flags */
           0,                           /* 0 = Parse file or directory names */
           xxstring,
           NULL,
           NULL
           );

    if (cmflgs == 1) {
	printf("?File or directory name required\n");
	return(-9);
    }
    while (!havename) {
	noarg = 0;
        x = cmfdb(&sw);                 /* Parse something */
	if (x == -3) {			/* They hit Enter prematurely */
	    printf("?Command incomplete\n");
	    return(-9);
	} else if (x < 0) {		/* Other error */
	    if (x == -1)
	      return(x);
            if (iswild(atmbuf) && nzxpand(atmbuf,nzxopts) == 0)
              printf("?No files match: %s\n",brstrip(atmbuf));
            else if (zchki(atmbuf) == -1)
	      printf("?File not found: %s\n",brstrip(atmbuf));
	    else
	      printf("?Error with switch or filename: %s\n",brstrip(atmbuf));
	    return(-9);
	}
	fn = cmresult.nresult;		/* For brevity */
        switch (cmresult.fcode) {	/* Handle each kind of field */
          case _CMKEY:			/* Keyword (switch) */
            c = cmgbrk();
            if ((getval = (c == ':' || c == '=')) && !(cmgkwflgs() & CM_ARG)) {
                printf("?This switch does not take an argument\n");
                return(-9);
            }
            if (!getval && (cmgkwflgs() & CM_ARG))
	      noarg = 1;		/* Remember arg is missing */
            switch (cmresult.nresult) {	/* Handle the switch */
              case DEL_LIS:		/* /LIST */
              case DEL_VRB:		/* /VERBOSE */
                listing = 1;
                break;
              case DEL_NOL:		/* /NOLIST */
              case DEL_QUI:		/* /QUIET */
		nolist = 1;
                listing = 0;
                break;
              case DEL_SIM:		/* /SIMULATE */
                sim = 1;
                break;
	      case REN_UPP:		/* /UPPER: */
	      case REN_LOW:		/* /LOWER */
		all = 1;
		if (!noarg) {
		    if ((x = cmkey((fn == REN_UPP) ? r_upper : r_lower,
				   2, "","all",xxstring)) < 0) {
			if (x == -3)
			  x = 1;
			else
			  return(x);
		    }
		    all = x;
		}
		/* 0 = don't convert; 1 = convert to lower; 2 = to upper */
		casing = (fn == REN_UPP) ? 2 : 1;
		converting = 0;
		break;
	      case REN_OVW:		/* /COLLISION */
		if (!noarg) {
		    if ((x = cmkey(r_collision,
				   nr_collision,"","overwrite",xxstring))<0) {
			if (x == -3)
			  x = RENX_OVWR;
			else
			  return(x);
		    }
		    collision = x;
		}
		break;
	      case REN_RPL:		/* /REPLACE: */
		if (noarg) {
		    printf("?This switch requires an argument\n");
		    return(-9);
		}
		if ((x = cmfld("String to remove, or {{String1}{String2}}",
			       "",&s,xxstring)) < 0) {
		    if (x == -3) {
			printf("?Target string required\n");
			x = -9;
		    }
		    return(x);
		}
		if (s[0]) {
		    if (s[0] == '{' && s[1] == '{') /* Get the list */
		      makelist(s,ren_sub,3);
		    else
		      makestr(&(ren_sub[0]),s);
		}
		converting = 0;
		break;

	      case REN_SPA:		/* /FIXSPACES: */
		if (!noarg)
		  if ((x = cmfld("Character or string to replace spaces with",
				 "_",&s,xxstring)) < 0)
		    if (x == -3)
		      s = "_";
		    else
		      return(x);
		makestr(&(ren_sub[0])," ");
		makestr(&(ren_sub[1]),noarg ? "_" : brstrip(s));
		makestr(&(ren_sub[3]),NULL);
		converting = 0;
		break;

#ifndef NOCSETS
#ifndef NOUNICODE
	      case REN_XLA:		/* /CONVERT:cset1:cset2 */
		if (!xfcstab) {		/* Make a copy of the file charset */
		    int i, x;		/* table with CM_ARG set for each */
		    x = (nfilc + 1) * sizeof(struct keytab);
		    xfcstab = (struct keytab *)malloc(x);
		    if (!xfcstab) {
			printf("?Memory allocation failure\n");
			return(-9);
		    }
		    for (i = 0; i < nfilc; i++) {
			xfcstab[i].kwd = fcstab[i].kwd;
			xfcstab[i].kwval = fcstab[i].kwval;
			xfcstab[i].flgs = fcstab[i].flgs | CM_ARG;
		    }
		}
		if ((x = cmswi(xfcstab,nfilc,
			       "Character-set of old name","",xxstring)) < 0) {
		    if (x == -3) {
			printf("?Pair of character-set names required\n");
			return(-9);
		    } else
		      return(x);
		}
		cset1 = x;
		c = cmgbrk();
		if (!getval) {
		    printf("?Secondcharacter-set name required\n");
		    return(-9);
		}
		if ((x = cmkey(fcstab,nfilc,
			       "Character-set of new name","",xxstring)) < 0) {
		    if (x == -3) {
			printf("?Second character-set name required\n");
			return(-9);
		    } else
		      return(x);
		}
		cset2 = x;
		if (casing)
		  casing = 0;
		if (ren_sub[0])
		  makestr(&(ren_sub[0]),NULL);
		if (ren_sub[1])
		  makestr(&(ren_sub[1]),NULL);
		if (ren_sub[2])
		  makestr(&(ren_sub[2]),NULL);
		converting = 1;
		break;
#endif	/* NOUNICODE */
#endif	/* NOCSETS */
            }
            break;
          case _CMIFI:			/* File or directory name */
            s = cmresult.sresult;
            havename = 1;
            break;
          default:
	    printf("?File or directory name required\n");
	    return(-9);
        }
    }
    if (havename) {
	ckstrncpy(line,s,LINBUFSIZ);	/* Make a safe copy of source name */
    } else {
	printf("?Internal error\n");
	return(-9);			/* Shouldn't happen */
    }
    wild = cmresult.nresult;            /* Source specification wild? */
    if (!wild)
      wild = iswild(line);

    debug(F111,"RENAME WILD",line,wild);

    p = tmpbuf;                         /* Place for new name */
    p[0] = NUL;
    replacing = ren_sub[0] ? 1 : 0;

#ifdef COMMENT
    if (!(casing || replacing || converting)) {
	if ((x = cmofi(wild ? "Target directory" : "New name",
		       "",&s,xxstring)) < 0) { /* Get new name */
	    if (x == -3) {
		printf("?%s required\n",
		       wild ? "Target directory" : "New name");
		return(-9);
	    } else return(x);
	}
	ckstrncpy(p,s,TMPBUFSIZ);	/* Make a safe copy of the new name */
    }
#else
    if ((x = cmofi(wild ? "Target directory" : "New name",
		   "",&s,xxstring)) < 0) { /* Get new name */
	if (x == -3) {
	    if (casing || replacing || converting) {
		s = "";
	    } else {
		printf("?%s required\n",
		       wild ? "Target directory" : "New name");
		return(-9);
	    }
	} else return(x);
    }
    ckstrncpy(p,s,TMPBUFSIZ);		/* Make a safe copy of the new name */
#endif	/* COMMENT */

    if ((y = cmcfm()) < 0) return(y);	/* Confirm the command */

#ifdef COMMENT
#ifndef NOUNICODE
    if (converting) {
	printf(" From: %s\n",fcsinfo[cset1].keyword);
	printf(" To:   %s\n",fcsinfo[cset2].keyword);
    }
#endif	/* NOUNICODE */
    if (casing) {
	printf("CASING: %s\n", (casing == 1) ? "LOWER" : "UPPER");
    }
    if (replacing) {
	printf("REPLACING: '%s' with '%s'\n",
	       ren_sub[0],
	       ren_sub[1] ? ren_sub[1] : "");
    }
#endif	/* COMMENT */

    s = line;
    if (!wild)				/* Just one */
      return(success =
	     renameone(s,p,
		       replacing,casing,all,converting,cset1,cset2,
		       listing,nolist,sim,TMPBUFSIZ,collision));

    if (!casing && !replacing && !converting) {	/* Multiple files */
	if (!isdir(p)) {
	    printf(			/* if target is not a directory */
"?Multiple source files not allowed if target is not a directory.\n");
	    return(-9);
	}
    }
#ifdef COMMENT
    else {                              /* Show full path of target */
        char buf[CKMAXPATH];            /* (too much) */
        if (zfnqfp(p,CKMAXPATH,buf))
          ckstrncpy(tmpbuf,buf,TMPBUFSIZ);
    }
#endif /* COMMENT */

#ifdef VMS
    conres();                           /* Let Ctrl-C work. */
#endif /* VMS */
    debug(F110,"dorename line",line,0);

#ifdef ZXREWIND
    z = zxrewind();                     /* Rewind file list */
#else
    z = nzxpand(s,0);                   /* Expand file list */
#endif /* ZXREWIND */
    debug(F111,"dorename p",p,z);

#ifdef UNIX
    if (wild && z > 1)
      sh_sort(mtchs,NULL,z,0,0,filecase); /* Alphabetize the filename list */
#endif /* UNIX */

    /* For /COLLISION:FAIL make a silent pass to see if there would be any */

    if (collision == RENX_FAIL) {
	int n = 0;
	char line[CKMAXPATH+2];
	while (z-- > 0) {
	    if (!(z == 0 && !wild))
	      znext(line);
	    if (!line[0])
	      break;
	    if (!renameone((char *)line,p,
			   replacing,casing,all,converting,cset1,cset2,
			   0,1,REN_OP_CHK,TMPBUFSIZ,RENX_FAIL))
	      n++;
	}
	if (n > 0) {
	    printf("?Failed: %d file%s would be overwritten\n",
		   n, (n != 1) ? "s" : "");
#ifdef VMS
	    concb((char)escape);
#endif /* VMS */
	    return(success = 0);
	}
	/* Get the file list back. */
#ifdef ZXREWIND
	z = zxrewind();
#else
	z = nzxpand(s,0);
#endif /* ZXREWIND */
    }
    while (z-- > 0 && rc > 0) {
	char line[CKMAXPATH+2];
        if (!(z == 0 && !wild))
          znext(line);
        if (!line[0])
          break;
	rc = renameone((char *)line,p, 
		       replacing,casing,all,converting,cset1,cset2,
		       listing,nolist,sim,TMPBUFSIZ,collision);
    }
#ifdef VMS
    concb((char)escape);
#endif /* VMS */
    return(success = rc);
}
#endif /* ZRENAME */
#endif /* NOFRILLS */
#endif /* NORENAME */

#ifndef NOSPL

/* Do the RETURN command */

int
doreturn(s) char *s; {
    int x;
    extern int tra_asg;
    char * line, * lp;

    if (cmdlvl < 1) {
        printf("\n?Can't return from level %d\n",maclvl);
        return(success = 0);
    }
    line = malloc(LINBUFSIZ);
    if (line == NULL)
      return(success = 0);
    lp = line;                          /* Expand return value now */
    x = LINBUFSIZ-1;
    if (!s) s = "";
    debug(F110,"RETURN s",s,0);
    if (zzstring(s,&lp,&x) > -1) {
        s = line;
        debug(F110,"RETURN zzstring",s,0);
    }

    /* Pop from all FOR/WHILE/SWITCH/XIFs */
    while ((maclvl > 0) &&
           (m_arg[maclvl-1][0]) &&
           (cmdstk[cmdlvl].src == CMD_MD) &&
           (!strncmp(m_arg[maclvl-1][0],"_xif",4) ||
            !strncmp(m_arg[maclvl-1][0],"_for",4) ||
            !strncmp(m_arg[maclvl-1][0],"_swi",4) ||
            !strncmp(m_arg[maclvl-1][0],"_whi",4))) {
        debug(F111,"RETURN IF/FOR/WHI/SWI pop",m_arg[maclvl-1][0],maclvl);
        dogta(XXPTA);                   /* Put args back */
        popclvl();                      /* Pop up two levels */
        popclvl();
    }
    if (tra_asg) {                      /* If tracing show return value */
        if (*s)
          printf("<<< %s: \"%s\"\n", m_arg[maclvl][0], s);
        else
          printf("<<< %s: (null)\n", m_arg[maclvl][0]);
    }
    popclvl();                          /* Pop from enclosing TAKE or macro */
    debug(F111,"RETURN tolevel",s,maclvl);
    if (!s) s = "";
    if (!*s) s = NULL;
    makestr(&(mrval[maclvl+1]),s);      /* Set the RETURN value */
    free(line);
    return(success = 1);                /* Macro succeeds if we RETURN */
}
#endif /* NOSPL */

#ifndef NOSPL
/* Do the OPEN command */

int
doopen()  {                             /* OPEN { append, read, write } */
    int x, y, z = 0; char *s;
    static struct filinfo fcb;          /* (must be static) */
    if ((x = cmkey(opntab,nopn,"mode","",xxstring)) < 0) {
        if (x == -3) {
            printf("?Mode required\n");
            return(-9);
        } else return(x);
    }
    switch (x) {
      case OPN_FI_R:                    /* Old file (READ) */
        if (chkfn(ZRFILE) > 0) {
            printf("?Read file already open\n");
            return(-2);
        }
        if ((z = cmifi("File to read","",&s,&y,xxstring)) < 0) {
            if (z == -3) {
                printf("?Input filename required\n");
                return(-9);
            } else return(z);
        }
        if (y) {                                /* No wildcards allowed */
            printf("\n?Please specify a single file\n");
            return(-2);
        }
        ckstrncpy(line,s,LINBUFSIZ);
        if ((int)strlen(line) < 1) return(-2);
        if ((z = cmnum("buffer size","4096",10,&y,xxstring)) < 0)
          return(z);
        if (y < 1) {
            printf("?Positive number required\n");
            return(-9);
        }
        if ((z = cmcfm()) < 0) return(z);
        readblock = y;
        if (readbuf)
          free((char *)readbuf);
        if (!(readbuf = (CHAR *) malloc(readblock+1))) {
            printf("?Can't allocate read buffer\n");
            return(-9);
        }
        return(success = zopeni(ZRFILE,line));

#ifndef MAC
#ifndef NOPUSH
      case OPN_PI_R:                    /* Pipe/Process (!READ) */
        if (nopush) {
            printf("?Read from pipe disabled\n");
            return(success=0);
        }
        if (chkfn(ZRFILE) > 0) {
            printf("?Read file already open\n");
            return(-2);
        }
        if ((y = cmtxt("System command to read from","",&s,xxstring)) < 0) {
            if (y == -3) {
                printf("?Command name required\n");
                return(-9);
            } else return(y);
        }
        ckstrncpy(line,brstrip(s),LINBUFSIZ);
        if (!line[0]) return(-2);
        if ((y = cmcfm()) < 0) return(y);
        if (!readbuf) {
            if (!(readbuf = (CHAR *) malloc(readblock+1))) {
                printf("?Can't allocate read buffer\n");
                return(-9);
            }
        }
        return(success = zxcmd(ZRFILE,line));

      case OPN_PI_W:                    /* Write to pipe */
        if (nopush) {
            printf("?Write to pipe disabled\n");
            return(success=0);
        }
        if (chkfn(ZWFILE) > 0) {
            printf("?Write file already open\n");
            return(-2);
        }
        if ((y = cmtxt("System command to write to","",&s,xxstring)) < 0) {
            if (y == -3) {
                printf("?Command name required\n");
                return(-9);
            } else return(y);
        }
        ckstrncpy(line,brstrip(s),LINBUFSIZ);
        if (!line[0]) return(-2);
        if ((y = cmcfm()) < 0) return(y);
        success = zxcmd(ZWFILE,line);
        if (!success && msgflg)
          printf("Can't open process for writing: %s\n",line);
        return(success);
#endif /* NOPUSH */
#endif /* MAC */

      case OPN_FI_W:                    /* New file (WRITE) */
      case OPN_FI_A:                    /* (APPEND) */
        if ((z = cmofi("Name of local file to create","",&s,xxstring)) < 0) {
            if (z == -3) {
                printf("?Filename required\n");
                return(-9);
            } else return(z);
        }
        if (z == 2) {
            printf("?Sorry, %s is a directory name\n",s);
            return(-9);
        }
        if (chkfn(ZWFILE) > 0) {
            printf("?Write/Append file already open\n");
            return(-2);
        }
        fcb.bs = fcb.cs = fcb.rl = fcb.fmt = fcb.org = fcb.cc = fcb.typ = 0;
        fcb.lblopts = 0;
        fcb.dsp = (x == OPN_FI_W) ? XYFZ_N : XYFZ_A; /* Create or Append */
        ckstrncpy(line,s,LINBUFSIZ);
        if ((int)strlen(line) < 1) return(-2);
        if ((y = cmcfm()) < 0) return(y);
        return(success = zopeno(ZWFILE,line,NULL,&fcb));

#ifndef NOLOCAL
      case OPN_SER:                     /* OPEN PORT or LINE */
      case OPN_NET: {                   /* OPEN HOST */
          extern int didsetlin, ttnproto;
          if (x == OPN_NET) {
              z = ttnproto;
              ttnproto = NP_NONE;
          }
          if ((y = setlin((x == OPN_SER) ? XYLINE : XYHOST, 1, 0)) < 0) {
              if (x == OPN_NET)
                ttnproto = z;
              success = 0;
          }
          didsetlin++;
          return(y);
      }
#endif /* NOLOCAL */

      default:
        printf("?Not implemented");
        return(-2);
    }
}
#endif /* NOSPL */

#ifndef NOXFER
/*  D O X G E T  --  GET command parser with switches  */

#ifdef CK_LABELED
int g_lf_opts = -1;
extern int lf_opts;
#endif /* CK_LABELED */

int
doxget(cx) int cx; {
    extern int                          /* External variables we need */
#ifdef RECURSIVE
      recursive,
#endif /* RECURSIVE */
      xfermode, fdispla, protocol, usepipes,
      g_binary, g_xfermode, g_displa, g_rpath, g_usepipes;
    extern char * rcv_move;             /* Directory to move new files to */
    extern char * rcv_rename;           /* What to rename new files to */
    extern char * rcvexcept[];          /* RECEIVE / GET exception list */
    int opkt  =  0;                     /* Flag for O-Packet needed */

#ifdef PIPESEND
    extern int pipesend;
    extern char * rcvfilter;
#endif /* PIPESEND */
    extern struct keytab rpathtab[];
    extern int nrpathtab;
    extern CK_OFF_T calibrate;
    int asname = 0;                     /* Flag for have as-name */
    int konly = 0;                      /* Kermit-only function */
    int c, i, n, confirmed = 0;         /* Workers */
    int getval = 0;                     /* Whether to get switch value */
    int rcvcmd = 0;                     /* Whether it is the RECEIVE command */
    int mget = 0;                       /* Whether it is the MGET command */
    struct stringint pv[SND_MAX+1];    /* Temporary array for switch values */
    struct FDB sw, fl, cm;              /* FDBs for each parse function */
    char * cmdstr = "this command";

#ifdef NEWFTP
    if (cx == XXGET || cx == XXREGET || cx == XXMGET || cx == XXRETR) {
        extern int ftpget;
        extern int ftpisopen();
        if ((ftpget == 1) || ((ftpget == 2) && ftpisopen())) {
	    int x;
	    x = doftpget(cx,0);
	    debug(F101,"doftpget return","",x);
	    if (x > -1)
	      success = x;
	    debug(F101,"doftpget success","",success);
	    return(x);
	}
    }
#endif /* NEWFTP */

    debug(F101,"xget cx","",cx);

    oopts = -1;
    omode = -1;

    for (i = 0; i <= SND_MAX; i++) {    /* Initialize switch values */
        pv[i].sval = NULL;
        pv[i].ival = -1;
        pv[i].wval = (CK_OFF_T)-1;
    }
    /* Preset switch values based on top-level command that called us */

    switch (cx) {
      case XXREC:                       /* RECEIVE */
        cmdstr = "RECEIVE";
        rcvcmd = 1; break;
      case XXGET:                       /* GET */
        cmdstr = "GET";
        konly = 1;
        break;
#ifdef CK_RESEND
      case XXREGET:                     /* REGET */
        cmdstr = "REGET";
        konly = 1;
        pv[SND_BIN].ival = 1;           /* Implies /BINARY */
        pv[SND_RES].ival = 1; break;
#endif /* CK_RESEND */
      case XXRETR:                      /* RETRIEVE */
        cmdstr = "RETRIEVE";
        konly = 1;
        pv[SND_DEL].ival = 1; break;
#ifdef PIPESEND
      case XXCREC:                      /* CRECEIVE */
        cmdstr = "CRECEIVE";
        konly = 1;
        rcvcmd = 1;
        pv[SND_CMD].ival = 1; break;
      case XXCGET:                      /* CGET */
        cmdstr = "CGET";
        konly = 1;
        pv[SND_CMD].ival = 1; break;
#endif /* PIPESEND */
#ifndef NOMGET
      case XXMGET:                      /* MGET */
        cmdstr = "MGET";
        konly = 1;
        mget = 1; break;
#endif /* NOMGET */
    }
    debug(F111,"xget rcvcmd",cmdstr,rcvcmd);
    debug(F101,"xget konly","",konly);

#ifdef CK_XYZ
    if (!rcvcmd && protocol != PROTO_K) {
        printf("?Sorry, %s works only with Kermit protocol\n",cmdstr);
        return(-9);
    }
#endif /* CK_XYZ */

    /* Set up chained parse functions... */

    cmfdbi(&sw,                         /* First FDB - command switches */
           _CMKEY,                      /* fcode */
           rcvcmd ?
           "Optional name/template to store incoming files under, or switch" :
           "Remote filename, or switch", /* hlpmsg */
           "",                          /* default */
           "",                          /* addtl string data */
           rcvcmd ? nrcvtab : ngettab,  /* addtl numeric data 1: tbl size */
           4,                           /* addtl numeric data 2: 4 = cmswi */
           xxstring,                    /* Processing function */
           rcvcmd ? rcvtab : gettab,    /* Keyword table */
           &fl                          /* Pointer to next FDB */
           );
    if (rcvcmd || mget)                 /* RECEIVE or MGET */
      cmfdbi(&fl,
           _CMTXT,                      /* fcode */
           rcvcmd ?                     /* hlpmsg */
             "Output filename or Command" : /* Output filename */
             "File(s) to GET",              /* Files we are asking for */
           "",                          /* default */
           "",                          /* addtl string data */
           0,                           /* addtl numeric data 1 */
           0,                           /* addtl numeric data 2 */
#ifdef COMMENT
#ifdef CK_XYZ
           (protocol == PROTO_X || protocol == PROTO_XC) ?
             xxstring :
             (rcvcmd ? (xx_strp)0  : xxstring)
#else
           rcvcmd ? (xx_strp)0  : xxstring /* Processing function */
#endif /* CK_XYZ */
#else  /* COMMENT */
	   xxstring			/* Always evaluate - fdc 2006/02/01 */
#endif	/* COMMENT */
             ,
           NULL,
           &cm
           );
    else
      cmfdbi(&fl,                       /* Remote filename or command */
           _CMFLD,                      /* fcode */
           "Remote filename",           /* hlpmsg */
           "",                          /* default */
           "",                          /* addtl string data */
           0,                           /* addtl numeric data 1 */
           0,                           /* addtl numeric data 2 */
           xxstring,
           NULL,
           &cm
           );
    cmfdbi(&cm,                         /* Confirmation */
           _CMCFM,                      /* fcode */
           "",                          /* hlpmsg */
           "",                          /* default */
           "",                          /* addtl string data */
           0,                           /* addtl numeric data 1 */
           0,                           /* addtl numeric data 2 */
           NULL,
           NULL,
           NULL
           );

    /* (See doxsend() for fuller commentary) */

    while (1) {                         /* Parse 0 or more switches */
        x = cmfdb(&sw);                 /* Parse something */
        debug(F101,"xget cmfdb","",x);
        if (x < 0)                      /* Error */
          goto xgetx;                   /* or reparse needed */
        if (cmresult.fcode != _CMKEY)   /* Break out if not a switch */
          break;
        c = cmgbrk();                   /* Get break character */
        if ((getval = (c == ':' || c == '=')) && !(cmgkwflgs() & CM_ARG)) {
            printf("?This switch does not take an argument\n");
            x = -9;
            goto xgetx;
        }
        if (!getval && (cmgkwflgs() & CM_ARG)) {
            printf("?This switch requires an argument\n");
            x = -9;
            goto xgetx;
        }
        n = cmresult.nresult;           /* Numeric result = switch value */
        debug(F101,"xget switch","",n);

        switch (n) {                    /* Process the switch */
#ifdef PIPESEND
          case SND_CMD:                 /* These take no args */
            if (nopush) {
                printf("?Sorry, system command access is disabled\n");
                x = -9;
                goto xgetx;
            } else if (rcvfilter) {
                printf(
"?Sorry, no GET /COMMAND when RECEIVE FILTER selected\n");
                x = -9;
                goto xgetx;
            }
            if (rcvcmd)
              sw.hlpmsg = "Command, or switch"; /* Change help message */
            /* Fall thru... */
#endif /* PIPESEND */

          case SND_REC:                 /* /RECURSIVE */
            pv[SND_PTH].ival = PATH_REL; /* Implies relative pathnames */
            pv[n].ival = 1;             /* Set the recursive flag */
            break;

          case SND_RES:                 /* /RECOVER */
            pv[SND_BIN].ival = 1;       /* Implies /BINARY */
            pv[n].ival = 1;             /* Set the resend flag */
            break;

          case SND_DEL:                 /* /DELETE */
          case SND_SHH:                 /* /QUIET */
          case SND_CAL:                 /* /CALIBRATE */
          case SND_XPA:                 /* /TRANSPARENT */
            pv[n].ival = 1;             /* Just set the appropriate flag */
            break;

          case SND_PIP:                 /* /PIPES:{ON,OFF} */
            if (!getval) {
                pv[n].ival = 1;
                break;
            }
            if ((x = cmkey(onoff,2,"","on",xxstring)) < 0)
              goto xgetx;
            if (!nopush)
              pv[n].ival = x;
            break;

          /* File transfer modes - each undoes the others */

          case SND_BIN:                 /* Binary */
          case SND_TXT:                 /* Text */
          case SND_IMG:                 /* Image */
          case SND_LBL:                 /* Labeled */
            pv[SND_BIN].ival = 0;       /* Unset all */
            pv[SND_TXT].ival = 0;
            pv[SND_IMG].ival = 0;
            pv[SND_LBL].ival = 0;
            pv[n].ival = 1;             /* Set the requested one */
            break;

          case SND_EXC:                 /* Excludes */
            if (!getval) break;
            if ((x = cmfld("Pattern","",&s,xxstring)) < 0) {
                if (x == -3) {
                    printf("?Pattern required\n");
                    x = -9;
                }
                goto xgetx;
            }
            if (pv[n].sval) free(pv[n].sval);
            y = strlen(s);
            if (y > 256) {
                printf("?Pattern too long - 256 max\n");
                x = -9;
                goto xgetx;
            }
            pv[n].sval = malloc(y+1);
            if (pv[n].sval) {
                strcpy(pv[n].sval,s);   /* safe */
                pv[n].ival = 1;
            }
            break;

#ifdef COMMENT
          /* Not implemented */
          case SND_PRI:                 /* GET to printer */
            pv[n].ival = 1;
            if (!getval) break;
            if ((x = cmfld("Print options","",&s,xxstring)) < 0)
              goto xgetx;
            pv[n].sval = malloc((int)strlen(s)+1);
            if (pv[n].sval)
              strcpy(pv[n].sval,s);     /* safe */
            break;
#endif /* COMMENT */

          case SND_MOV:                 /* MOVE after */
          case SND_REN:                 /* RENAME after */
            if (!getval) break;
            if ((x = cmfld(n == SND_MOV ?
           "device and/or directory for source file after sending" :
           "new name for source file after sending",
                           "",
                           &s,
                           n == SND_MOV ? xxstring : NULL
                           )) < 0) {
                if (x == -3) {
                    printf("%s\n", n == SND_MOV ?
                           "?Destination required" :
                           "?New name required"
                           );
                    x = -9;
                }
                goto xgetx;
            }
            if (pv[n].sval) {
                free(pv[n].sval);
                pv[n].sval = NULL;
            }
            s = brstrip(s);
            y = strlen(s);
            if (y > 0) {
                pv[n].sval = malloc(y+1);
                if (pv[n].sval) {
                    strcpy(pv[n].sval,s); /* safe */
                    pv[n].ival = 1;
                }
            }
            break;

          case SND_ASN:                 /* As-name */
            if (!getval) break;
            if (mget) {
                printf("?Sorry, as-name not allowed with MGET\n");
                x = -9;
                goto xgetx;
            }
            if (
#ifdef COMMENT
		(x = cmfld("Name to store it under","",&s,NULL))
#else
		(x = cmfld("Name to store it under","",&s,xxstring))
#endif	/* COMMENT */
		< 0)
              goto xgetx;
            s = brstrip(s);
            if ((y = strlen(s)) > 0) {
                if (pv[n].sval) free(pv[n].sval);
                pv[n].sval = malloc(y+1);
                if (pv[n].sval) {
                    strcpy(pv[n].sval,s); /* safe */
                    pv[n].ival = 1;
                }
            }
            break;

#ifdef PIPESEND
          case SND_FLT:                 /* Filter */
            debug(F101,"xget /filter getval","",getval);
            if (!getval) break;
            if ((x = cmfld("Filter program to receive through",
                           "",&s,NULL)) < 0) {
                if (x == -3)
                  s = "";
                else
                  goto xgetx;
            }
            if (*s) s = brstrip(s);
            y = strlen(s);
            for (x = 0; x < y; x++) {   /* Make sure they included "\v(...)" */
                if (s[x] != '\\') continue;
                if (s[x+1] == 'v') break;
            }
            if (x == y) {
                printf(
                "?Filter must contain a replacement variable for filename.\n"
                       );
                x = -9;
                goto xgetx;
            }
            pv[n].ival = 1;
            if (pv[n].sval) {
                free(pv[n].sval);
                pv[n].sval = NULL;
            }
            if ((y = strlen(s)) > 0) {
                if ((pv[n].sval = malloc(y+1)))
                  strcpy(pv[n].sval,s); /* safe */
            }
            break;
#endif /* PIPESEND */

          case SND_PTH:                 /* Pathnames */
            if (!getval) {
                pv[n].ival = PATH_REL;
                break;
            }
            if ((x = cmkey(rpathtab,nrpathtab,"","on",xxstring)) < 0)
              goto xgetx;
            pv[n].ival = x;             /* Ditto */
            break;

          case SND_NAM:                 /* Filenames */
            if (!getval) break;
            if ((x = cmkey(fntab,nfntab,"","converted",xxstring)) < 0)
              goto xgetx;
            pv[n].ival = x;
            break;

          case SND_PRO:                 /* Protocol to use */
            if (!getval) break;
            if ((x = cmkey(protos,nprotos,"File-transfer protocol","",
                           xxstring)) < 0) {
                if (x == -3)
                  x = 0;
                else
                  goto xgetx;
            }
            debug(F111,"xget /proto",atmbuf,x);
            pv[n].ival = x;
            if (konly && x != PROTO_K) {
                printf(
"?Sorry, this command works only with Kermit protocol\n"
                       );
                x = -9;
                goto xgetx;
            }
            break;

          default:
            printf("?Unexpected switch value - %d\n",cmresult.nresult);
            x = -9;
            goto xgetx;
        }
    }
    debug(F101,"xget cmresult fcode","",cmresult.fcode);

    cmarg = line;                       /* Initialize string pointers */
    cmarg2 = tmpbuf;
    asname = 0;
    line[0] = NUL;                      /* and buffers. */
    tmpbuf[0] = NUL;

    switch (cmresult.fcode) {           /* How did we get out of switch loop */
      case _CMFLD:                      /* (3) Remote filespec */
        ckstrncpy(line,cmresult.sresult,LINBUFSIZ);
        break;
      case _CMTXT:                      /* (4) As-name */
        if (rcvcmd) {
            ckstrncpy(tmpbuf,cmresult.sresult,TMPBUFSIZ);
            if ((int)strlen(tmpbuf) > 0)
              asname = 1;
        } else {
            ckstrncpy(line,cmresult.sresult,LINBUFSIZ);
        }
      case _CMCFM:                      /* (6) Confirmation */
        confirmed = 1;
        break;
      default:
        printf("?Unexpected function code: %d\n",cmresult.fcode);
        x = -9;
        goto xgetx;
    }
    debug(F110,"xget string",cmarg,0);
    debug(F101,"xget confirmed","",confirmed);

    cmarg = brstrip(cmarg);             /* Strip any braces */

    if (!confirmed) {                   /* CR not typed yet, get more fields */
        if (pv[SND_CMD].ival > 0) {
            debug(F100,"xget calling cmtxt","",0);
            x = cmtxt("Local command to pipe into","",&s,NULL);
            if (x < 0 && x != -3) goto xgetx;
            if (x != -3) {
                ckstrncpy(tmpbuf,s,TMPBUFSIZ);
                asname = 1;
            }
        } else if (!rcvcmd) {
#ifdef VMS
            /* cmofi() fails if you give it a directory name */
            x = cmfld("Name or directory for incoming file","",&s,NULL);
            debug(F111,"xget cmfld",s,x);
#else
            x = cmofi("Name or directory for incoming file","",&s,NULL);
            debug(F111,"xget cmofi",s,x);
#endif /* VMS */
            if (x < 0 && x != -3) goto xgetx;
            if (x != -3) {
                ckstrncpy(tmpbuf,s,TMPBUFSIZ);
                if ((x = cmcfm()) < 0) goto xgetx;
                asname = 1;
            }
        }
    }
    /* Arrive here with cmarg and cmarg2 all set */

    debug(F111,"xget asname",cmarg2,asname);
    if (!asname) {
        if (pv[SND_ASN].sval)
          ckstrncpy(tmpbuf,pv[SND_ASN].sval,TMPBUFSIZ);
        else
          tmpbuf[0] = NUL;
    }
    cmarg2 = brstrip(cmarg2);           /* Strip outer braces if any. */
    debug(F110,"xget cmarg",cmarg,0);
    debug(F110,"xget cmarg2",cmarg2,0);

    if (!*cmarg &&
        (cx == XXGET || cx == XXREGET || cx == XXCGET || cx == XXMGET)) {
        printf("?A remote file specification is required\n");
        x = -9;
        goto xgetx;
    }
#ifdef PIPESEND
    if (pv[SND_CMD].ival > 0) {         /* /COMMAND sets pipesend flag */
        x = -9;
        if (!*cmarg2) {
            printf("?Command required\n");
            goto xgetx;
        } else if (nopush) {
            printf("?Sorry, system command access is disabled\n");
            goto xgetx;
        } else if (rcvfilter) {
            printf("?Sorry, no GET /COMMAND while RECEIVE FILTER selected\n");
            goto xgetx;
        } else
          pipesend = 1;
    }
    debug(F101,"xget /COMMAND pipesend","",pipesend);
#endif /* PIPESEND */

#ifdef CK_RESEND
    if (pv[SND_RES].ival > 0) {         /* REGET or GET /RECOVER */
#ifdef RECURSIVE
        if (pv[SND_REC].ival > 0) {     /* RECURSIVE */
#ifdef COMMENT
            printf("?Unsupported option combination: /RECOVER /RECURSIVE\n");
            x = -9;
            goto xgetx;
#else
            opkt = 1;
#endif /* COMMENT */
        }
#endif /* RECURSIVE */
        if (pv[SND_DEL].ival > 0) {     /* /DELETE */
#ifdef COMMENT
            printf("?Unsupported option combination: /RECOVER /DELETE\n");
            x = -9;
            goto xgetx;
#else
            opkt = 1;
#endif /* COMMENT */
        }
    }
#endif /* CK_RESEND */

    if (pv[SND_EXC].ival > 0)           /* /EXCEPT */
      makelist(pv[SND_EXC].sval,rcvexcept,NSNDEXCEPT);

#ifdef IKS_OPTION
    if (!rcvcmd
#ifdef CK_XYZ
         && protocol == PROTO_K
#endif /* CK_XYZ */
         ) {
        if (!iks_wait(KERMIT_REQ_START,1)) {
            printf(
              "?A Kermit Server is not available to process this command\n");
            x = -9;                     /* correct the return code */
            goto xgetx;
        }
    }
#endif /* IKS_OPTION */

#ifdef CK_XYZ
    {
        int po, pg;                     /* (for clarity) */
        po = pv[SND_PRO].ival;          /* /PROTOCOL option */
        pg = protocol;                  /* Protocol global  */
        if ((rcvcmd && !*cmarg2) &&     /* If no as-name was given */
            /* and /PROTOCOL is XMODEM or global protocol is XMODEM... */
            ((po <  0 && (pg == PROTO_X || pg == PROTO_XC)) ||
             (po > -1 && (po == PROTO_X || po == PROTO_XC)))
            ) {
            printf(
"Sorry, you must specify a name when receiving a file with XMODEM protocol\n"
                   );
            x = -9;
            goto xgetx;
        }
    }
#endif /* CK_XYZ */

#ifdef RECURSIVE
    if (pv[SND_REC].ival > 0) {         /* RECURSIVE */
        recursive = 1;
        pv[SND_PTH].ival = PATH_REL;    /* Implies relative pathnames too */
    }
#endif /* RECURSIVE */

    if (pv[SND_PIP].ival > -1) {
        g_usepipes = usepipes;
        usepipes = pv[SND_PIP].ival;
    }

    /* Save global protocol parameters */

    g_proto = protocol;
#ifdef CK_LABELED
    g_lf_opts = lf_opts;                /* Save labeled transfer options */
#endif /* CK_LABELED */
    g_urpsiz = urpsiz;                  /* Receive packet length */
    g_spsizf = spsizf;                  /* Send packet length flag */
    g_spsiz = spsiz;                    /* Send packet length */
    g_spsizr = spsizr;                  /* etc etc */
    g_spmax = spmax;
    g_wslotr = wslotr;
    g_prefixing = prefixing;
    g_fncact = fncact;
    g_fncnv = fncnv;
    g_fnspath = fnspath;
    g_fnrpath = fnrpath;
    g_xfrxla = xfrxla;

    if (pv[SND_PRO].ival > -1) {        /* Change according to switch */
        protocol = pv[SND_PRO].ival;
        if (ptab[protocol].rpktlen > -1)   /* copied from initproto() */
            urpsiz = ptab[protocol].rpktlen;
        if (ptab[protocol].spktflg > -1)
            spsizf = ptab[protocol].spktflg;
        if (ptab[protocol].spktlen > -1) {
            spsiz = ptab[protocol].spktlen;
            if (spsizf)
                spsizr = spmax = spsiz;
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
    }
    debug(F101,"xget protocol","",protocol);
    debug(F111,"xget cmarg2",cmarg2,xfermode);

    g_xfermode = xfermode;
    g_binary = binary;
    if (pv[SND_BIN].ival > 0) {         /* Change according to switch */
        xfermode = XMODE_M;
        binary = XYFT_B;                /* FILE TYPE BINARY */
        omode = GMOD_BIN;               /* O-Packet mode */
        debug(F101,"doxget /BINARY xfermode","",xfermode);
    } else if (pv[SND_TXT].ival > 0) {  /* Ditto for /TEXT */
        xfermode = XMODE_M;
        binary = XYFT_T;
        omode = GMOD_TXT;
        debug(F101,"doxget /TEXT xfermode","",xfermode);
    } else if (pv[SND_IMG].ival > 0) {
        xfermode = XMODE_M;
#ifdef VMS
        binary = XYFT_I;
#else
        binary = XYFT_B;
#endif /* VMS */
        omode = GMOD_TXT;
        debug(F101,"doxget /IMAGE xfermode","",xfermode);
    }
#ifdef CK_LABELED
    else if (pv[SND_LBL].ival > 0) {
        xfermode = XMODE_M;
        binary = XYFT_L;
        omode = GMOD_LBL;
        debug(F101,"doxget /LABELED xfermode","",xfermode);
    }
#endif /* CK_LABELED */
    debug(F101,"xget binary","",binary);
    debug(F101,"xget omode","",omode);

    if (pv[SND_XPA].ival > 0)           /* /TRANSPARENT */
      xfrxla = 0;                       /* Don't translate character sets */

#ifdef PIPESEND
    if (pv[SND_FLT].ival > 0)
      makestr(&rcvfilter,pv[SND_FLT].sval);
#endif /* PIPESEND */

#ifdef CK_TMPDIR
    if (pv[SND_MOV].ival > 0) {
        int len;
        char * p = pv[SND_MOV].sval;
#ifdef CK_LOGIN
        if (isguest) {
            printf("?Sorry, /MOVE-TO not available to guests\n");
            x = -9;
            goto xgetx;
        }
#endif /* CK_LOGIN */
        len = strlen(p);
        if (!isdir(p)) {                /* Check directory */
#ifdef CK_MKDIR
            char * s = NULL;
            s = (char *)malloc(len + 4);
            if (s) {
                strcpy(s,p);            /* safe */
#ifdef datageneral
                if (s[len-1] != ':') { s[len++] = ':'; s[len] = NUL; }
#else
                if (s[len-1] != '/') { s[len++] = '/'; s[len] = NUL; }
#endif /* datageneral */
                s[len++] = 'X';
                s[len] = NUL;
                x = zmkdir(s);
                free(s);
                if (x < 0) {
                    printf("?Can't create \"%s\"\n",p);
                    x = -9;
                    goto xgetx;
                }
            }
#else
            printf("?Directory \"%s\" not found\n",p);
            x = -9;
            goto xgetx;
#endif /* CK_MKDIR */
        }
        zfnqfp(p,LINBUFSIZ,line);
        makestr(&rcv_move,line);
    }
#endif /* CK_TMPDIR */

    if (pv[SND_REN].ival > 0) {         /* /RENAME-TO:name */
        char * p = pv[SND_REN].sval;
#ifdef CK_LOGIN
        if (isguest) {
            printf("?Sorry, /RENAME-TO not available to guests\n");
            x = -9;
            goto xgetx;
        }
#endif /* CK_LOGIN */
        if (!p) p = "";
        if (!*p) {
            printf("?New name required for /RENAME\n");
            x = -9;
            goto xgetx;
        }
        p = brstrip(p);
        makestr(&rcv_rename,p);
        debug(F110,"xget rcv_rename","",0);
    }

#ifdef CALIBRATE
    if (pv[SND_CAL].ival > 0)
      calibrate = (CK_OFF_T)1;
#endif /* CALIBRATE */
    g_displa = fdispla;
    if (pv[SND_SHH].ival > 0)
      fdispla = 0;
    debug(F101,"xget display","",fdispla);

    if (pv[SND_NAM].ival > -1) {        /* /FILENAMES */
        g_fncnv = fncnv;                /* Save global value */
        fncnv = pv[SND_NAM].ival;
        debug(F101,"xsend fncnv","",fncnv);
        /* We should also handle O packet filename option here */
        /* but we don't really need to since WHATAMI already handles it */
    }
    if (pv[SND_PTH].ival > -1) {        /* PATHNAMES */
        g_rpath = fnrpath;              /* Save global values */
        fnrpath = pv[SND_PTH].ival;
        debug(F101,"xsend fnrpath","",fnrpath);
#ifndef NZLTOR
        if (fnrpath != PATH_OFF) {
            g_fncnv = fncnv;
            fncnv = XYFN_L;
            debug(F101,"xsend fncnv","",fncnv);
        }
        /* We should also handle O packet pathname option here */
        /* but we don't really need to since WHATAMI already handles it */
#endif /* NZLTOR */
    }

    /* Set protocol start state */

    if (opkt) {                         /* Extended GET Options*/
        sstate = (CHAR) 'o';
        oopts = 0;
        if (pv[SND_DEL].ival > 0) oopts |= GOPT_DEL; /* GET /DELETE */
        if (pv[SND_RES].ival > 0) oopts |= GOPT_RES; /* GET /RECOVER */
        if (pv[SND_REC].ival > 0) oopts |= GOPT_REC; /* GET /RECURSIVE */
    } else if (rcvcmd)
      sstate = (CHAR) 'v';              /* RECEIVE or CRECEIVE */
    else if (pv[SND_DEL].ival > 0)
      sstate = (CHAR) 'h';              /* GET /DELETE (= RETRIEVE) */
    else if (pv[SND_RES].ival > 0)
      sstate = (CHAR) 'j';              /* GET /RECOVER (= REGET) */
    else
      sstate = (CHAR) 'r';              /* Regular GET */
    getcmd = 1;
    debug(F000,"xget sstate","",sstate);
#ifdef MAC
    what = W_RECV;
    scrcreate();
#endif /* MAC */
    if (local) {
        if (pv[SND_SHH].ival != 0)
          displa = 1;
	if (protocol == PROTO_K)	/* fdc 20070108 */
	  ttflui();
    }
    x = 0;
#ifdef PIPESEND
    if (pipesend)
      goto xgetx;
#endif /* PIPESEND */

#ifdef CK_TMPDIR
/*
  cmarg2 is also allowed to be a device or directory name;
  even the name of a directory that doesn't exist.
*/
    y = strlen(cmarg2);
    debug(F111,"xget strlen(cmarg2)",cmarg2,y);
    if ((y > 0) &&
#ifdef OS2
        ((isalpha(cmarg2[0]) &&
         cmarg2[1] == ':' &&
         cmarg2[2] == NUL) ||
        (cmarg[y-1] == '/' || cmarg[y-1] == '\\') ||
        isdir(cmarg2))
#else
#ifdef UNIXOROSK
        (cmarg2[y-1] == '/' || isdir(cmarg2))
#else
#ifdef VMS
        (cmarg2[y-1] == ']' || cmarg2[y-1] == '>' || isdir(cmarg2))
#else
#ifdef STRATUS
        (cmarg2[y-1] == '>' || isdir(cmarg2))
#else
#ifdef datageneral
        (cmarg2[y-1] == ':' || cmarg[0] == ':' || isdir(cmarg2))
#else
        isdir(cmarg2)
#endif /* datageneral */
#endif /* STRATUS */
#endif /* VMS */
#endif /* UNIXOROSK */
#endif /* OS2 */
        ) {
        debug(F110,"doxget RECEIVE cmarg2 disk or dir",cmarg2,0);
        if (!f_tmpdir) {
            int x;
            s = zgtdir();
            if (s) {
                ckstrncpy(savdir,s,TMPDIRLEN); /* remember old disk/dir */
                f_tmpdir = 1;   /* and that we did this */
            } else {
                printf("?Can't get current directory\n");
                cmarg2 = "";
                f_tmpdir = 0;
                x = -9;
                goto xgetx;
            }
#ifdef CK_MKDIR
            x = zchki(cmarg2);          /* Does as-name exist? */
            if (x == -1) {              /* Doesn't exist */
                char * p = NULL;        /* Try to create it */
                x = strlen(cmarg2);
                if ((p = (char *)malloc(x+4))) {
                    sprintf(p,"%s%s",cmarg2,"x.x"); /* SAFE (prechecked) */
                    x = zmkdir(p);
                    free(p);
                    if (x < 0) {
                        printf("?Can't create %s\n",cmarg2);
                        x = -9;
                        goto xgetx;
                    }
                }
            }
#endif /* CK_MKDIR */
            if (!zchdir(cmarg2)) {      /* change to given disk/directory, */
                printf("?Can't access %s\n",cmarg2);
                x = -9;
                goto xgetx;
            }
            cmarg2 = "";
        }
    }
#endif /* CK_TMPDIR */

    ckstrncpy(fspec,cmarg,CKMAXPATH);   /* Note - this is a REMOTE filespec */
    debug(F111,"xget fspec",fspec,fspeclen);
    debug(F110,"xget cmarg2",cmarg2,0);

  xgetx:
    for (i = 0; i < SND_MAX; i++)
      if (pv[i].sval)
        free(pv[i].sval);
    return(x);
}
#endif /* NOXFER */

#ifndef NOSPL

/*
  D O G T A  --  Do _GETARGS or _PUTARGS Command.

  Used by XIF, FOR, WHILE, and SWITCH, each of which are implemented as
  2-level macros; the first level defines the macro, the second runs it.
  This routine hides the fact that they are macros by importing the
  macro arguments (if any) from two levels up, to make them available
  in the IF, FOR, SWITCH, and WHILE commands themselves; for example as
  loop indices, etc, and within the IF/FOR/WHILE/SWITCH body itself.
  _PUTARGS is in case we changed any of these variables or used SHIFT
  on them, so the new values won't be lost as we pop up the stack.
*/
int
dogta(cx) int cx; {
    int i, n;
    char c, *p,  mbuf[4];
    extern int topargc, cmdint;
    extern char ** topxarg;

    if ((y = cmcfm()) < 0)
      return(y);
    debug(F101,"dogta cx","",cx);
    debug(F101,"dogta maclvl","",maclvl);
    if (cx == XXGTA) {
        debug(F101,"dogta _GETARGS maclvl","",maclvl);
    } else if (cx == XXPTA) {
        debug(F101,"dogta _PUTARGS maclvl","",maclvl);
    } else {
        return(-2);
    }
    if (maclvl < 1)
      return(success = 0);

    /* Make new copies of macro arguments /%0..9 */

    mbuf[0] = '%'; mbuf[1] = '0'; mbuf[2] = NUL; /* Argument name buf */

    if (cx == XXPTA) {                  /* Go NOINT because _PUTARGS */
        if (cmdint)                     /* temporarily changes maclvl. */
          connoi();                     /* Interrupts OFF. */
    }
    for (i = 0; i < 10; i++) {          /* For all args */
        c = (char) (i + '0');           /* Make name */
        mbuf[1] = (char) c;             /* Insert digit */
        if (cx == XXGTA) {              /* Get arg from level-minus-2 */
            if (maclvl == 1) p = g_var[c]; /* If at level 1 use globals 0..9 */
            else p = m_arg[maclvl-2][i];   /* Otherwise they're on the stack */
            addmac(mbuf,p);
#ifdef COMMENT
            if (maclvl > 1)
              makestr(&(m_line[maclvl]),m_line[maclvl-2]);
#endif /* COMMENT */
        } else if (cx == XXPTA) {       /* Put args level+2 */
            maclvl -= 2;                /* This is gross, it's because we're */
            addmac(mbuf,m_arg[maclvl+2][i]); /* adding macros two levels up */
            maclvl += 2;                     /* and addmac() uses maclvl. */
            count[cmdlvl - 2]  = count[cmdlvl];
            intime[cmdlvl - 2] = intime[cmdlvl];
            inpcas[cmdlvl - 2] = inpcas[cmdlvl];
            takerr[cmdlvl - 2] = takerr[cmdlvl];
            merror[cmdlvl - 2] = merror[cmdlvl];
            xquiet[cmdlvl - 2] = xquiet[cmdlvl];
            xvarev[cmdlvl - 2] = xvarev[cmdlvl];
        } else return(success = 0);     /* Bad call to this routine */
    }
    if (cx == XXPTA) {                  /* Restore interrupts if we */
        if (cmdint)                     /* turned them off above. */
          conint(trap,stptrap);
    }
    /* Now take care of the argument vector array \&_[], \v(return), */
    /* and \v(argc) by just copying the pointers. */

    if (cx == XXGTA) {                  /* GETARGS from 2 levels up */
        if (maclvl == 1) {
            a_ptr[0] = topxarg;         /* \&_[] array */
            a_dim[0] = topargc - 1;     /* Dimension doesn't include [0] */
            m_xarg[maclvl] = topxarg;
            n_xarg[maclvl] = topargc;   /* But \v(argc) does include \%0 */
            macargc[maclvl] = topargc;
            makestr(&(mrval[maclvl+1]),mrval[0]); /* (see vnlook()) */
        } else {
            a_ptr[0] = m_xarg[maclvl-2];
            a_dim[0] = n_xarg[maclvl-2];
            m_xarg[maclvl] = m_xarg[maclvl-2];
            n_xarg[maclvl] = n_xarg[maclvl-2];
            macargc[maclvl] = n_xarg[maclvl-2];
            makestr(&(mrval[maclvl+1]),mrval[maclvl-1]); /* (see vnlook()) */

        }
    } else {                            /* PUTARGS 2 levels up */
        if (maclvl > 1) {
            a_ptr[0] = m_xarg[maclvl];
            m_xarg[maclvl-2] = m_xarg[maclvl];
            a_dim[0] = n_xarg[maclvl];
            n_xarg[maclvl-2] = n_xarg[maclvl];
            macargc[maclvl-2] = n_xarg[maclvl];
        }
    }
    return(1);                  /* Internal command - don't change success */
}
#endif /* NOSPL */

#ifndef NOSPL
/*
  Do the GOTO and [_]FORWARD commands.
  s = Label to search for, cx = function code: XXGOTO, XXFWD, or XXXFWD.
*/

int
dogoto(s, cx) char *s; int cx; {
    int i, j, x, y, z, bc;
    int empty = 0, stopflg = 0;
    char * cmd;                         /* Name of this command */
    char tmplbl[LBLSIZ+1], *lp;	        /* Current label from command stream */
    char tmp2[LBLSIZ+1];		/* SWITCH label conversion buffer */
    char tmp3[LBLSIZ+1];		/* Target label */

    stopflg = (cx == XXXFWD);           /* _FORWARD (used in SWITCH) */
    bc = 0;                             /* Brace counter */

    cmd = (cx == XXGOTO) ? "GOTO" : ((cx == XXFWD) ? "FORWARD" : "_FORWARD");
    if (!s) s = "";
    if (!*s) empty = 1;

#ifdef DEBUG
    if (deblog) {
        debug(F111,"GOTO command",cmd,cx);
        debug(F101,"GOTO cmdlvl","",cmdlvl);
        debug(F101,"GOTO maclvl","",maclvl);
        debug(F101,"GOTO tlevel","",tlevel);
        debug(F111,"GOTO target",s,empty);
    }
#endif /* DEBUG */
    debug(F110,cmd,s,0);
    x = ckstrncpy(tmp3+1,s,LBLSIZ);
    debug(F101,"GOTO target len","",x);
    debug(F101,"GOTO target at x","",s[x]);
    if (s[x]) {
	debug(F100,"GOTO target overflow","",0);
	printf("?GOTO target or SWITCH case label too long\n");
	if (stopflg) dostop();		/* If in SWITCH return to prompt */
	return(success = 0);
    }
    s = tmp3+1;
    if (*s != ':') {                    /* Make copy of label */
        tmp3[0] = ':';                  /* guaranteed to start with ":" */
        s--;
    }
    if (!stopflg && !empty) {
        if (s[1] == '.' || s[1] == SP || s[1] == NUL) {
            printf("?Bad label syntax - '%s'\n",s);
	    if (stopflg) dostop();	/* If in SWITCH return to prompt */
            return(success = 0);
        }
    }
    if (cmdlvl == 0) {
        printf("?Sorry, %s only works in a command file or macro\n",cmd);
        return(success = 0);
    }
    y = strlen(s);                      /* y = length of target label */
    debug(F111,cmd,s,y);

    while (cmdlvl > 0) {                /* As long as not at top level... */
        if (cmdstk[cmdlvl].src == CMD_MD) { /* GOTO inside macro */
            int i, m, flag;
            char *xp, *tp;

            /* GOTO: rewind the macro; FORWARD: start at current position */

            lp = (cx == XXGOTO) ? macx[maclvl] : macp[maclvl];
            m = (int)strlen(lp);
            debug(F111,"GOTO in macro",lp,m);

            flag = 1;                   /* flag for valid label position */
            for (i = 0; i < m; i++,lp++) { /* search for label in macro body */
                if (*lp == '{')         /* But only at this level */
                  bc++;                 /* Anything inside braces is off */
                else if (*lp == '}')    /* limits. */
                  bc--;
                if (stopflg && bc > 0)  /* This is good for SWITCH */
                  continue;             /* but interferes with WHILE, etc. */
                if (*lp == ',') {
                    flag = 1;
                    continue;
                }
                if (flag) {             /* If in valid label position */
                    if (*lp == SP)      /* eat leading spaces */
                      continue;
                    if (*lp != ':') {   /* Look for label introducer */
                        flag = 0;       /* this isn't it */
                        continue;       /* keep looking */
                    }
                }
                if (!flag)              /* We don't have a label */
                  continue;             /*  so keep looking... */
                xp = lp; tp = tmplbl;   /* Copy the label from the macro */
                j = 0;                  /* to make it null-terminated */
                while ((*tp = *xp)) {
                    if (j++ > LBLSIZ-1) { /* j = length of word from macro */
			printf("?GOTO target or SWITCH case label too long\n");
			if (stopflg) dostop(); /* Return to prompt */
			return(success = 0);
		    }
                    if (!*tp || *tp == ',') /* Look for end of word */
                      break;
                    else tp++, xp++;    /* Next character */
                }
                *tp = NUL;              /* In case we stopped early */
                /* Now do caseless string comparison, using longest length */
                debug(F111,"macro GOTO label",s,y);
                debug(F111,"macro target label",tmplbl,j);
                if (stopflg) {          /* Allow variables as SWITCH labels */
                    int n = LBLSIZ - 1;
                    char * p = tmp2;
                    zzstring(tmplbl,&p,&n);
		    if (n < 0) {
			printf("?GOTO target or SWITCH case label too long\n");
			if (stopflg) dostop(); /* Return to prompt */
			return(0);
		    }
                    ckstrncpy(tmplbl,tmp2,LBLSIZ);
                }
                debug(F111,"GOTO s",s,y);
                debug(F111,"GOTO tmplbl",tmplbl,j);
                debug(F111,"GOTO empty",ckitoa(stopflg),empty);

                if (empty) {		   /* Empty target */
		    z = (!strcmp(s,":") && /* String is empty */
			 /* and Label is ":" or ":*"... */
			 (!strcmp(tmplbl,":") || !strcmp(tmplbl,":*")))
			? 0 : 1;
		    debug(F111,"GOTO","A",z);
                } else if (stopflg) {
                    z = ckmatch(tmplbl,s,inpcas[cmdlvl],1) ? 0 : 1;
		    debug(F111,"GOTO","B",z);
                } else {
                    z = (stopflg && inpcas[cmdlvl]) ?
                      strcmp(s,tmplbl) :
                        ckstrcmp(s,tmplbl,(y > j) ? y : j, 0);
		    debug(F111,"GOTO","C",z);
                }
                if (!z) {
                    break;
                } else if (stopflg &&
                         !ckstrcmp(":default",tmplbl,(8 > j) ? 8 : j, 0)) {
                    debug(F100,"GOTO DEFAULT","",0);
                    break;
                } else {
                    flag = 0;
                }
            }
            debug(F111,"GOTO macro i",cmd,i);
            debug(F111,"GOTO macro m",cmd,m);
            if (i >= m) {               /* Didn't find the label */
                debug(F101,"GOTO failed cmdlvl","",cmdlvl);
#ifdef COMMENT
		/* MOVED TO AFTER POPCLVL ABOUT 20 LINES DOWN 5 AUG 2002 */
		   if (stopflg)
                  return(0);
#endif /* COMMENT */
                if ((maclvl > 0) &&
                       (m_arg[maclvl-1][0]) &&
                       (cmdstk[cmdlvl].src == CMD_MD) &&
                       (!strncmp(m_arg[maclvl-1][0],"_xif",4) ||
                        !strncmp(m_arg[maclvl-1][0],"_for",4) ||
                        !strncmp(m_arg[maclvl-1][0],"_swi",4) ||
                        !strncmp(m_arg[maclvl-1][0],"_whi",4))) {
                    dogta(XXPTA);       /* Restore args */
                    debug(F101,"GOTO in XIF/FOR/WHI/SWI popping","",cmdlvl);
                    popclvl();          /* Pop an extra level */
                }
                debug(F101,"GOTO popping","",cmdlvl);
                if (!popclvl()) {       /* pop up to next higher level */
                    printf("?Label '%s' not found\n",s); /* if none */
                    return(0);          /* Quit */
                } else if (stopflg) {	/* SWITCH no case label match */
		    return(0);		/* and no DEFAULT lable. */
		} else {
		    continue;        /* otherwise look again */
		}
            }
            debug(F110,"GOTO found macro label",tmplbl,0);
            macp[maclvl] = lp;          /* set macro buffer pointer */
            return(1);
        } else if (cmdstk[cmdlvl].src == CMD_TF) {
            x = 0;                      /* GOTO issued in take file */
            debug(F111,"GOTO in TAKE file",cmd,cx);
            if (cx == XXGOTO) {         /* If GOTO, but not FORWARD, */
                rewind(tfile[tlevel]);  /* search file from beginning */
                tfline[tlevel] = 0;
            }
            while (! feof(tfile[tlevel])) {
#ifdef COMMENT
/* This is wrong - it lets us jump to labels inside inferior blocks */
                tfline[tlevel]++;
                if (fgets(line,LINBUFSIZ,tfile[tlevel]) == NULL) /* Get line */
#else
                if (getnct(line,LINBUFSIZ,tfile[tlevel],0) < 0)
#endif /* COMMENT */
                  break;                /* If no more, done, label not found */
                lp = line;              /* Got line */
                while (*lp == SP || *lp == HT)
                  lp++;                 /* Strip leading whitespace */
                if (*lp != ':') continue; /* Check for label introducer */
                while (*(lp+1) == SP) { /* Remove space between : and name */
                    *(lp+1) = ':';
                    lp++;               /* Strip leading whitespace */
                }
                tp = lp;                /* Get end of word */
                j = 0;
                while (*tp) {           /* And null-terminate it */
                    if (*tp < 33) {
                        *tp = NUL;
                        break;
                    } else tp++, j++;
                }
                if (!ckstrcmp(lp,s,(y > j) ? y : j,0)) { /* Caseless compare */
                    x = 1;              /* Got it */
                    break;              /* done. */
                } else if (stopflg &&
                           !ckstrcmp(":default",tmplbl,(8 > j) ? 8 : j,0)) {
                    x = 1;
                    break;
                }
            }
            if (x == 0) {               /* If not found, print message */
                debug(F101,"GOTO failed at cmdlvl","",cmdlvl);
                if (stopflg)
                  return(0);
                if (!popclvl()) {       /* pop up to next higher level */
                    printf("?Label '%s' not found\n",s); /* if none */
                    return(0);          /* quit */
                } else continue;        /* otherwise look again */
            }
            return(x);                  /* Send back return code */
        }
    }
    printf("?Stack problem in GOTO %s\n",s); /* Shouldn't see this */
    return(0);
}
#endif /* NOSPL */

/* Finish parsing and do the IF, XIF, and WHILE commands */

#ifndef NOSPL

/*  C H K V A R  --  Check (if it's a) Variable  */


#ifdef OLDCHKVAR
/*
  Crude and disgusting, but needed for OS/2, DOS, and Windows, where filenames
  have backslashes in them.  How do we know if a backslash in a filename is a
  directory separator, or if it's a Kermit backslash?  This routine does a
  rough syntax check of the next few characters and if it looks like it MIGHT
  be a variable, then it tries to evaluate it, and if the result is not empty,
  we say it's a variable, although sometimes it might not be -- some cases are
  truly ambiguous.  For example there might a DOS directory called \%a, and
  we also have a variable with the same name.  This is all for the sake of not
  having to tell PC users that they have to double all backslashes in file
  and directory names.
*/
#else
/*
  Somewhat less crude & disgusting.  The previous method was nondeterministic
  and (worse) it interfered with macro argument passing.  So now we only
  check the syntax of backslash-items to see if they are variables, but we
  do NOT check their values.
*/
#endif /* OLDCHKVAR */
/*
  Call with a string pointer pointing at the backslash of the purported
  variable.  Returns 1 if it has the syntax of a variable, 0 if not.
*/
int
chkvar(s) char *s; {
    int z = 0;                          /* Return code - assume failure. */
    if (!s) s = "";                     /* Watch our for null pointers. */
    if (!*s) return(0);                 /* Empty arg so not a variable. */
    if (*s == CMDQ) {                   /* Object begins with backslash. */
        char c;
        c = s[1];                       /* Character following backslash. */
        if (c) {
            int t = 0;
            if (c == CMDQ)              /* Quoted backslash */
              return(1);
            c = (char) (islower(c) ? toupper(c) : c); /* Otherwise... */
            if (c == '%') {             /* Simple variable */
#ifdef OLDCHKVAR
                t = 1;
#else
                return(1);
#endif /* OLDCHKVAR */
            } else if (c == '&') {      /* Array */
                if (!s[2]) return(0);
                if (s[3] == '[')
                  t = ckindex("]",s,4,0,1);
#ifndef OLDCHKVAR
                return((t > 0) ? 1 : 0);
#endif /* OLDCHKVAR */
            } else if (c == '$' ||      /* Environment variable */
                       c == 'V' ||      /* Built-in variable */
                       c == 'M') {      /* Macro name */
                t = (s[2] == '(');
#ifndef OLDCHKVAR
                return((t > 0) ? 1 : 0);
#endif /* OLDCHKVAR */
            } else if (c == 'F') {      /* Function reference */
                /* Don't actually call it - it might have side effects */
                int x;
                if ((x = ckindex("(",s,3,0,1))) /* Just check syntax */
                  if ((x = ckindex(")",s,x,0,1)))
                    z = 1;
                /* Insert a better syntax check here if necessary */
            }
#ifdef OLDCHKVAR
            if (t) {
                t = 255;                /* This lets us test \v(xxx) */
                lp = line;              /* and even \f...(xxx) */
                zzstring(s,&lp,&t);     /* Evaluate it, whatever it is. */
                t = strlen(line);       /* Get its length. */
                debug(F111,"chkvar",line,t);
                z = t > 0;              /* If length > 0, it's defined */
            }
#endif /* OLDCHKVAR */
        }
    }
    return(z);
}

/*  B O O L E X P  --  Evaluate a Boolean expression  */

#define BOOLLEN 1024
static char boolval[BOOLLEN];

int
boolexp(cx) int cx; {
    int x, y, z; char *s, *p;
    int parens = 0, pcount = 0, ecount = 0;
    char *q, *bx;
    struct FDB kw, nu;
#ifdef FNFLOAT
    struct FDB fl;
    CKFLOAT f1 = 0.0, f2 = 0.0;
    int f1flag = 0, f2flag = 0;
#endif /* FNFLOAT */
#ifdef OS2
    extern int keymac;
#endif /* OS2 */

    not = 0;                            /* Flag for whether "NOT" was seen */
    z = 0;                              /* Initial IF condition */
    ifargs = 0;                         /* Count of IF condition words */
    bx = boolval;                       /* Initialize boolean value */
    *bx = NUL;

  ifagain:
    cmfdbi(&kw,                         /* First FDB - command switches */
           _CMKEY,                      /* fcode */
           "Number, numeric-valued variable, Boolean expression, or keyword",
           "",                          /* default */
           "",                          /* addtl string data */
           nif,                         /* addtl numeric data 1: tbl size */
           0,                           /* addtl numeric data 2: 4 = silent */
           xxstring,                    /* Processing function */
           iftab,                       /* Keyword table */
           &nu                          /* Pointer to next FDB */
           );
    cmfdbi(&nu,                         /* 2nd FDB - An integer */
           _CMNUM,                      /* fcode */
           "",                          /* hlpmsg */
           "",                          /* Default */
           "",                          /* addtl string data */
           0,
           0,
           xxstring,
           NULL,
#ifdef FNFLOAT
           &fl
#else
           NULL
#endif /* FNFLOAT */
           );
#ifdef FNFLOAT
    cmfdbi(&fl,                         /* A floating-point number */
           _CMFLD,                      /* fcode */
           "",                          /* hlpmsg */
           "",                          /* default */
           "",                          /* addtl string data */
           0,                           /* addtl numeric data 1 */
           0,                           /* addtl numeric data 2 */
           xxstring,
           NULL,
           NULL
           );
#endif /* FNFLOAT */
    x = cmfdb(&kw);                     /* Parse a keyword or a number */
    debug(F111,"boolval cmfdb","",x);
    if (x < 0) {
        if (x == -3)
          x = -2;
        return(x);
    }
    debug(F111,"boolval switch","",cmresult.fcode);
    switch (cmresult.fcode) {           /* What did we get? */
      case _CMFLD: {			/* A "field" */
	  int i;
	  char * s;
	  s = cmresult.sresult;
/*
  C-Kermit 9.0: This allows a macro name to serve as an
  IF condition without having to enclose it in \m(...).
*/
	  if (
#ifdef FNFLOAT	  
	      !isfloat(cmresult.sresult,0) /* Not a number */
#else
	      !chknum(cmresult.sresult) /* Not a number */
#endif	/* FNFLOAT */
	      ) {
	      i = mlook(mactab,cmresult.sresult,nmac); /* Look it up */
	      if (i > -1)		/* in the macro table */
		s = mactab[x].mval;	/* and get its value */
	      else			/* Otherwise if no such macro */
		s = "0";		/* evaluate as FALSE. */
	  }
#ifdef FNFLOAT
        if (isfloat(s,0)) {		/* A floating-point number? */
            f1 = floatval;              /* Yes, get its value */
            f1flag = 1;                 /* remember we did this */
            ifc = 9999;                 /* Set special "if-code" */
        }
#else
	if (chknum(s)) {
	    cmresult.nresult = atoi(s);
	    ifc = 9999;
	    break;
	}
#endif /* FNFLOAT */
	else
	  return(-2);
      }
      case _CMNUM:                      /* A number... */
        ifc = 9999;                     /* Set special "if-code" */
        break;
      case _CMKEY:                      /* A keyword */
        ifc = cmresult.nresult;         /* Get if-code */
        break;
      default:
        return(-2);
    }
    switch (ifc) {                      /* set z = 1 for true, 0 for false */
      case 9999:                        /* Number */
#ifdef FNFLOAT
        if (f1flag) {
            z = (f1 == 0.0) ? 0 : 1;
        } else
#endif /* FNFLOAT */
        z = (cmresult.nresult == 0) ? 0 : 1;
        break;
      case XXIFLP:                      /* Left paren */
        if (pcount == 0 && ifargs > 0)
          return(-2);
        parens = 1;
        pcount++;
        ifargs++;
        *bx++ = '(';
        goto ifagain;
      case XXIFRP:                      /* Right paren */
        if (!parens)
          return(-2);
        if (--pcount < 0)
          return(-2);
        ifargs++;
        *bx++ = ')';
        *bx = NUL;
        if (pcount == 0)
          goto ifend;
        goto ifagain;
      case XXIFAN:                      /* AND (&&) */
        ifargs++;
        if (!ecount)
          return(-2);
        *bx++ = '&';
        goto ifagain;
      case XXIFOR:                      /* OR (||) */
        ifargs++;
        if (!ecount)
          return(-2);
        *bx++ = '|';
        goto ifagain;
      case XXIFNO:                      /* IF NOT [ NOT [ NOT ... ] ] */
        if (bx > boolval) {             /* evala() doesn't like cascaded */
            if (*(bx-1) == '!') {       /* unary operators... */
                *(bx-1) = NUL;          /* So here, two wrongs make a right. */
                bx--;
            } else {
                *bx++ = '!';
            }
        } else {
            *bx++ = '!';
        }
        ifargs++;
        goto ifagain;
      case XXIFTR:                      /* IF TRUE */
        z = 1;
        debug(F101,"if true","",z);
        ifargs += 1;
        break;
      case XXIFNT:                      /* IF FALSE */
        z = 0;
        debug(F101,"if true","",z);
        ifargs += 1;
        break;
      case XXIFSU:                      /* IF SUCCESS */
        z = ( success != 0 ) ? 1 : 0;
        debug(F101,"if success","",z);
        ifargs += 1;
        break;
      case XXIFFA:                      /* IF FAILURE */
        z = ( success == 0 ) ? 1 : 0;
        debug(F101,"if failure","",z);
        ifargs += 1;
        break;

      case XXIFDE:                      /* IF DEFINED */
        if ((x = cmfld("Macro or variable name","",&s,NULL)) < 0)
          return((x == -3) ? -2 : x);

        if (*s == CMDQ) {
            char * lp;
            char line[256];
            int t, x;
            if (*(s+1) == 'f' || *(s+1) == 'F') { /* Built-in function */
                extern struct keytab fnctab[];
                extern int nfuncs;
                ckstrncpy(line,s+2,256);
                if (line[0]) {
                    lp = ckstrchr(line,'(');
                    if (lp) *lp = NUL;
                    x = lookup(fnctab,line,nfuncs,&t);
                    z = x > -1;
                }
                debug(F111,"if defined function",line,z);
            } else if (*(s+1) == 'v' || *(s+1) == 'V') { /* 8.0.200 */
                extern struct keytab vartab[];
                extern int nvars;
                z = 0;
                if (*(s+2) == '(') {
                    ckstrncpy(line,s+3,256);
                    if (line[0]) {
                        lp = ckstrchr(line,')');
                        if (lp) *lp = NUL;
                        x = lookup(vartab,line,nvars,&t);
                        z = x > -1;
			if (z) {	/* 8.0.203 */
			    int t;	/* It must have a value to succeed */
			    t = 255;	/* as in C-Kermit 6.0 and 7.0 */
			    lp = line;	/* (this was broken in 8.0.200-201) */
			    zzstring(s,&lp,&t);
			    t = strlen(line);
			    z = line[0] ? 1 : 0;
			}
                    }
                }
                debug(F111,"if defined variable",line,z);
            } else {
                z = chkvar(s);          /* Starts with backslash */
                if (z > 0) {            /* Yes... */
                    t = 255;            /* than buffer so if zzstring fails  */
                    lp = line;          /* check for that -- overflow means */
                    line[0] = NUL;      /* the quantity is defined. */
                    x = zzstring(s,&lp,&t);
                    if ((x < 0 && t != 255) || !line[0])
                      z = 0;
                    debug(F111,"if defined zzstring",line,z);
                    debug(F101,"if defined zzstring t","",t);
                }
            }
        } else {
            z = (mxlook(mactab,s,nmac) > -1); /* Look for exact match */
        }
        debug(F111,"if defined final",s,z);
        ifargs += 2;
        break;

      case XXIFDC: {                    /* IF DECLARED */
          char * lp;
          char line[32];
          int j, k, t, x;
          if ((x = cmfld("Array name","",&s,NULL)) < 0)
            return((x == -3) ? -2 : x);
          if (*s == CMDQ) {
              if (*(s+1) != '&') {
                  t = 31;
                  lp = line;
                  line[0] = NUL;
                  x = zzstring(s,&lp,&t);
                  s = line;
              }
          }
          z = 0;
          if ((x = arraybounds(s,&j,&k)) > -1) {
              if (a_ptr[x]) {
                  if (j < 1)
                    z = 1;
                  else if (j <= a_dim[x])
                    z = 1;
                  if (z == 1 && k > a_dim[x])
                    z = 0;
              }
          }
          break;
      }
      case XXIFBG:                      /* IF BACKGROUND */
      case XXIFFG:                      /* IF FOREGROUND */
        bgchk();                        /* Check background status */
        if (ifc == XXIFFG)              /* Foreground */
          z = pflag ? 1 : 0;
        else z = pflag ? 0 : 1;         /* Background */
        ifargs += 1;
        break;

      case XXIFCO:                      /* IF COUNT */
        z = ( --count[cmdlvl] > 0 );
        if (cx == XXWHI) count[cmdlvl] += 2; /* Don't ask... */
        debug(F101,"if count","",z);
        ifargs += 1;
        break;

      case XXIFEX:                      /* IF EXIST */
#ifdef CK_TMPDIR
      case XXIFDI:                      /* IF DIRECTORY */
#endif /* CK_TMPDIR */
      case XXIFAB:                      /* IF ABSOLUTE */
      case XXIFLN:			/* IF LINK */
        if ((x = cmfld(
                       ((ifc == XXIFDI) ? "Directory name" : "File"),
                       "",&s,
#ifdef OS2
                       NULL             /* This allows \'s in filenames */
#else
                       xxstring
#endif /* OS2 */
                       )) < 0) {
            if (x == -3) {
                extern int cmflgs;
                if (cmflgs == 1) {
                    printf("?File or directory name required\n");
                    return(-9);
                }
            } else return(x);
        }
        s = brstrip(s);
#ifdef UNIX
	if (ifc == XXIFLN) {
	    z = isalink(s);
	} else
#endif	/* UNIX */
        if (ifc == XXIFAB) {
            z = isabsolute(s);
        } else if (ifc == XXIFEX) {
            z = (zgetfs(s) > -1L);
            debug(F101,"if exist 1","",z);
#ifdef OS2
            if (!z) {                   /* File not found. */
                int t;                  /* Try expanding variables */
                t = LINBUFSIZ-1;        /* and looking again. */
                lp = line;
                zzstring(s,&lp,&t);
                s = line;
                z = ( zchki(s) > -1L );
                debug(F101,"if exist 2","",z);
            }
#endif /* OS2 */
#ifdef CK_TMPDIR
        } else {
#ifdef VMS
            z = (zchki(s) == -2)
#else
/* Because this doesn't catch $DISK1:[FOO]BLAH.DIR;1 */
            z = isdir(s)
#ifdef OS2
              || (isalpha(s[0]) && s[1] == ':' && s[2] == NUL)
#endif /* OS2 */
#endif /* VMS */
              ;
            debug(F101,"if directory 1","",z);

            if (!z) {			/* File not found. */
                int t;                  /* Try expanding variables */
                t = LINBUFSIZ-1;        /* and looking again. */
                lp = line;
                zzstring(s,&lp,&t);
                s = line;
                z = isdir(s)
#ifdef OS2
                  || (isalpha(s[0]) && s[1] == ':' && s[2] == NUL)
#endif /* OS2 */
                    ;
                debug(F101,"if directory 2","",z);
            }
#endif /* CK_TMPDIR */
        }
        ifargs += 2;
        break;

      case XXIFEQ:                      /* IF EQUAL (string comparison) */
      case XXIFLL:                      /* IF Lexically Less Than */
      case XXIFLG:                      /* If Lexically Greater Than */
        if ((x = cmfld("first word or variable name","",&s,xxstring)) < 0) {
            if (x == -3) {
                printf("?Text required\n");
                return(-9);
            } else return(x);
        }
        s = brstrip(s);                 /* Strip braces */
        x = (int)strlen(s);
        if (x > LINBUFSIZ-1) {
            printf("?IF: strings too long\n");
            return(-2);
        }
        lp = line;                      /* lp points to first string */
        ckstrncpy(line,s,LINBUFSIZ);
        if ((y = cmfld("second word or variable name","",&s,xxstring)) < 0) {
            if (y == -3) {
                printf("?Text required\n");
                return(-9);
            } else return(y);
        }
        s = brstrip(s);
        y = (int)strlen(s);
        if (x + y + 2 > LINBUFSIZ) {
            printf("?IF: strings too long\n");
            return(-2);
        }
        tp = lp + x + 2;                /* tp points to second string */
        strcpy(tp,s);                   /* safe (checked) */
        x = ckstrcmp(lp,tp,-1,inpcas[cmdlvl]); /* Use longest length */
        switch (ifc) {
          case XXIFEQ:                  /* IF EQUAL (string comparison) */
            z = (x == 0);
            break;
          case XXIFLL:                  /* IF Lexically Less Than */
            z = (x < 0);
            break;
          case XXIFLG:                  /* If Lexically Greater Than */
            z = (x > 0);
            break;
        }
        debug(F101,"IF EQ result","",z);
        ifargs += 3;
        break;

      case XXIFVE:                      /* IF VERSION */
      case XXIFAE:                      /* IF (arithmetically) = */
      case XXIFNQ:                      /* IF != (not arithmetically equal) */
      case XXIFLT:                      /* IF <  */
      case XXIFLE:                      /* IF <= */
      case XXIFGE:                      /* IF >= */
      case XXIFGT: {                    /* IF >  */

	  /* July 2006 - converted to use CK_OFF_T rather than int to */
          /* allow long integers on platforms that have ck_off_t > 32 bits */
	  int xx;
	  CK_OFF_T n1 = (CK_OFF_T)0, n2 = (CK_OFF_T)0;
        if (ifc == XXIFVE) {
            n1 = (CK_OFF_T) vernum;
        } else {
            x = cmfld("first number or variable name","",&s,xxstring);
            if (x == -3) {
                printf("?Quantity required\n");
                return(-9);
            }
            if (x < 0) return(x);
            debug(F101,"xxifgt cmfld","",x);
            ckstrncpy(line,s,LINBUFSIZ);
            lp = brstrip(line);
            debug(F110,"xxifgt exp1",lp,0);

/* The following bit is for compatibility with old versions of MS-DOS Kermit */

            if (!ckstrcmp(lp,"count",5,0)) {
                n1 = (CK_OFF_T)count[cmdlvl];
            } else if (!ckstrcmp(lp,"version",7,0)) {
                n1 = (CK_OFF_T) vernum;
            } else if (!ckstrcmp(lp,"argc",4,0)) {
                n1 = (CK_OFF_T) macargc[maclvl];
            } else {

/* End of compatibility bit */

#ifdef FNFLOAT
                if (isfloat(lp,0) > 1) { /* Allow floating-point comparisons */
                    f1 = floatval;
                    f1flag = 1;
                } else
#endif /* FNFLOAT */
                  if (chknum(lp)) {
                      n1 = ckatofs(lp);
                  } else {              /* Check for arithmetic expression */
                      q = evala(lp);    /* cmnum() does this but ... */
                      if (chknum(q)) {	/* we're not using cmnum(). */
			  n1 = ckatofs(q);
		      } else {
			  printf("?Value not numeric - %s", lp);
			  return(-9);
		      }
                  }
            }
        }
        y = cmfld("number or variable name","",&s,xxstring);
        if (y == -3) {
            printf("?Quantity required\n");
            return(-9);
        }
        if (y < 0) return(y);
        s = brstrip(s);
        if (!*s) return(-2);
        if (ifc == XXIFVE) {
            tp = line;
        } else {
            x = (int)strlen(lp);
            tp = line + x + 2;
        }
        ckstrncpy(tp,s,LINBUFSIZ-x-2);
        debug(F110,"xxifgt exp2",tp,0);
        if (!ckstrcmp(tp,"count",5,0)) {
            n2 = (CK_OFF_T) count[cmdlvl];
        } else if (!ckstrcmp(tp,"version",7,0)) {
            n2 = (CK_OFF_T) vernum;
        } else if (!ckstrcmp(tp,"argc",4,0)) {
            n2 = (CK_OFF_T) macargc[maclvl];
        } else {
#ifdef FNFLOAT
            if (isfloat(tp,0) > 1) {
                f2 = floatval;
                f2flag = 1;
            } else
#endif /* FNFLOAT */
            if (chknum(tp)) {
                n2 = ckatofs(tp);
            } else {
                q = evala(tp);
		if (chknum(q)) {	/* we're not using cmnum(). */
		    n2 = ckatofs(q);
		} else {
		    printf("?Value not numeric - %s", tp);
		    return(-9);
		}
            }
        }
        xx = (ifc == XXIFVE) ? XXIFGE : ifc;

#ifdef FNFLOAT
        if (f1flag && !f2flag) {
            f2 = (CKFLOAT)n2;
            f2flag = 1;
        }
        if (f2flag && !f1flag)
          f1 = (CKFLOAT)n1;
        if (f1flag)
          z = ((f1 <  f2 && xx == XXIFLT)
               || (f1 != f2 && xx == XXIFNQ)
               || (f1 <= f2 && xx == XXIFLE)
               || (f1 == f2 && xx == XXIFAE)
               || (f1 >= f2 && xx == XXIFGE)
               || (f1 >  f2 && xx == XXIFGT));
        else
#endif /* FNFLOAT */
          z = ((n1 <  n2 && xx == XXIFLT)
               || (n1 != n2 && xx == XXIFNQ)
               || (n1 <= n2 && xx == XXIFLE)
               || (n1 == n2 && xx == XXIFAE)
               || (n1 >= n2 && xx == XXIFGE)
               || (n1 >  n2 && xx == XXIFGT));
        debug(F101,"xxifge z","",z);
        if (ifc == XXIFVE)
          ifargs += 2;
        else
          ifargs += 3;
        break;
      }

      case XXIFNU:                      /* IF NUMERIC */
        x = cmfld("variable name or constant","",&s,NULL);
        if (x == -3) {
            extern int cmflgs;
            if (cmflgs == 1) {
                printf("?Quantity required\n");
                return(-9);
            }
        } else if (x < 0)
          return(x);
        x = LINBUFSIZ-1;
        lp = line;
        zzstring(s,&lp,&x);
        lp = line;
        debug(F110,"xxifnu quantity",lp,0);
        z = chknum(lp);
#ifdef COMMENT
/*
  This works, but it's not wise -- IF NUMERIC is mostly used to see if a
  string really does contain only numeric characters.  If they want to force
  evaluation, they can use \feval() on the argument string.
*/
        if (!z) {                       /* Not a number */
            x_ifnum = 1;                /* Avoid "eval" error messages */
            q = evala(lp);              /* Maybe it's an expression */
            z = chknum(q);              /* that evaluates to a number */
            x_ifnum = 0;                /* Put eval messages back to normal */
            if (z) debug(F110,"xxifnu exp",lp,0);
        }
#endif /* COMMENT */
        debug(F101,"xxifnu chknum","",z);
        ifargs += 2;
        break;

#ifdef ZFCDAT
      case XXIFNE: {                    /* IF NEWER */
        char d1[20], * d2;              /* Buffers for 2 dates */
        if ((z = cmifi("First file","",&s,&y,xxstring)) < 0)
          return(z);
        ckstrncpy(d1,zfcdat(s),20);
        if ((z = cmifi("Second file","",&s,&y,xxstring)) < 0)
          return(z);
        d2 = zfcdat(s);
        if ((int)strlen(d1) != 17 || (int)strlen(d2) != 17) {
            printf("?Failure to get file date\n");
            return(-9);
        }
        debug(F110,"xxifnewer d1",d1,0);
        debug(F110,"xxifnewer d2",d2,0);
        z = (strcmp(d1,d2) > 0) ? 1 : 0;
        debug(F101,"xxifnewer","",z);
        ifargs += 2;
        break;
      }
#endif /* ZFCDAT */

#ifdef CK_IFRO
      case XXIFRO:                      /* REMOTE-ONLY advisory */
        ifargs++;
#ifdef NOLOCAL
        z = 1;
#else
        z = remonly;
#endif /* NOLOCAL */
        break;
#endif /* CK_IFRO */

      case XXIFAL:                      /* ALARM */
        ifargs++;
        debug(F101,"IF ALARM ck_alarm","",ck_alarm);
        debug(F110,"IF ALARM alrm_date",alrm_date,0);
        debug(F110,"IF ALARM alrm_time",alrm_time,0);

        if (ck_alarm < 1L || alrm_date[0] < '0' || alrm_time[0] < '0') {
            z = 0;                      /* ALARM not SET */
            break;                      /* so IF ALARM fails */
        }
        /* Get current date and time */
        ckstrncpy(tmpbuf,ckcvtdate("",1),TMPBUFSIZ);
        s = tmpbuf;
        s[8] = NUL;
        z = (int) strncmp(tmpbuf,alrm_date,8); /* Compare dates */
        debug(F101,"IF ALARM date z","",z);
        if (z == 0) {                   /* Dates are the same */
            /* Compare times */
            z = (tod2sec(tmpbuf+9) >= atol(alrm_time)) ? 1 : -1;
            debug(F101,"IF ALARM time z","",z);
        }
        tmpbuf[0] = NUL;                /* z >= 0 if alarm is passed */
        z = ((z >= 0) ? 1 : 0);         /* z <  0 otherwise */
        debug(F101,"IF ALARM final z","",z);
        break;

      case XXIFOP:                      /* IF OPEN */
        if ((x = cmkey(iotab,niot,"file or log","",xxstring)) < 0)
          return(x);
        if (x == 9999 || x == ZSTDIO) {
            bgchk();                    /* Check background status */
            z = pflag ? 1 : 0;
        } else if (x == 8888) {
            z = local ? ttchk() > -1 : 0;
#ifdef CKLOGDIAL
        } else if (x == 7777) {
            extern int dialog;
            z = dialog ? 1 : 0;
#endif /* CKLOGDIAL */
        } else
          z = (chkfn(x) > 0) ? 1 : 0;
        ifargs += 1;
        break;

      case XXIFSD:                      /* Started-From-Dialer */
#ifdef OS2
        z = StartedFromDialer;
#else
        z = 0;
#endif /* OS2 */
        break;

      case XXIFTM:                      /* Terminal-Macro */
#ifdef OS2
        z = cmdstk[cmdlvl].ccflgs & CF_KMAC;
#else
        z = 0;
#endif /* OS2 */
        break;

      case XXIFEM:                      /* Emulation is active */
#ifdef OS2
        z = 1;
#else
        z = 0;
#endif /* OS2 */
        break;

      case XXIFIK:                      /* Running as IKSD? */
        z = inserver;
        break;

      case XXIFTA:                      /* Connection is TAPI */
        z = 0;
#ifndef NODIAL
#ifdef CK_TAPI
        if (local && !network && tttapi)
          z = 1;
#endif /* CK_TAPI */
#endif /* NODIAL */
        break;

      case XXIFMA:                      /* IF MATCH */
        x = cmfld("String or variable","",&s,xxstring);
        if (x == -3) {
            extern int cmflgs;
            if (cmflgs == 1) {
                printf("?String required\n");
                return(-9);
            }
        } else if (x < 0)
          return(x);
        ckstrncpy(line,s,LINBUFSIZ);
        s = brstrip(line);
        debug(F110,"xxifma string",line,0);
        x = cmfld("Pattern","",&p,xxstring);
        if (x == -3) {
            extern int cmflgs;
            if (cmflgs == 1) {
                printf("?Pattern required\n");
                return(-9);
            }
        } else if (x < 0)
          return(x);
        ckstrncpy(tmpbuf,p,TMPBUFSIZ);
        p = brstrip(tmpbuf);
        debug(F110,"xxifma pattern",tmpbuf,0);
        z = ckmatch(p,s,inpcas[cmdlvl],1);
        break;

      case XXIFFL: {                    /* IF FLAG */
          extern int ooflag;
          z = ooflag;
          break;
      }
      case XXIFAV: {                    /* IF AVAILABLE */
          if ((x = cmkey(availtab,availtabn,"","",xxstring)) < 0)
            return(x);
          switch (x) {
            case AV_KRB4:
              z = ck_krb4_is_installed();
              break;
            case AV_KRB5:
              z = ck_krb5_is_installed();
              break;
            case AV_SRP:
              z = ck_srp_is_installed();
              break;
            case AV_SSL:
              z = ck_ssleay_is_installed();
              break;
            case AV_NTLM:
              z = ck_ntlm_is_installed();
              break;
            case AV_CRYPTO:
              z = ck_crypt_is_installed();
              break;
            case AV_SSH:
              z = ck_ssh_is_installed();
              break;
            default:
              z = 0;
          }
          break;
      }
      case XXIFAT:                      /* IF ASKTIMEOUT */
        z = asktimedout;
        break;

      case XXIFRD:                      /* IF READABLE */
      case XXIFWR:                      /* IF WRITEABLE */
        if ((x = cmfld("File or directory name",
                       "",
                       &s,
#ifdef OS2
                       NULL             /* This allows \'s in filenames */
#else
                       xxstring
#endif /* OS2 */
                       )) < 0) {
            if (x == -3) {
                extern int cmflgs;
                if (cmflgs == 1) {
                    printf("?File or directory name required\n");
                    return(-9);
                }
            } else return(x);
        }
        s = brstrip(s);
/*
  zchk[io]() do not do what we want here for directories, so we set
  a global flag telling it to behave specially in this case.  Othewise
  we'd have to change the API and change all ck?fio.c modules accordingly.
*/
        y = 0;                          /* Try-again control */
#ifdef OS2
  ifrdagain:
#endif /* OS2 */
        if (ifc == XXIFRD) {            /* IF READABLE */
            zchkid = 1;
            z = zchki(s) > -1;
            zchkid = 0;
        } else if (ifc == XXIFWR) {     /* IF WRITEABLE */
            zchkod = 1;
            z = zchko(s) > -1;
            zchkod = 0;
        }
#ifdef OS2
        if (!z && !y) {                 /* File not found. */
            int t;                      /* Try expanding variables */
            t = LINBUFSIZ-1;            /* and looking again. */
            lp = line;
            zzstring(s,&lp,&t);
            s = line;
            z = zchko(s) > -1;
            y++;
            goto ifrdagain;
        }
#endif /* OS2 */
        ifargs += 2;
        break;
      case XXIFQU:                      /* IF QUIET */
        z = quiet ? 1 : 0;
        debug(F101,"if quiet","",z);
        ifargs += 1;
        break;

      case XXIFWI:                      /* WILD */
        if ((x = cmfld("File specification","",&s,xxstring)) < 0) return(x);
        z = iswild(s);
        break;

      case XXIFCK:                      /* C-KERMIT */
#ifdef OS2
        z = 0;
#else
        z = 1;
#endif /* OS2 */
        break;

      case XXIFK9:                      /* K-95 */
#ifdef OS2
        z = 1;
#else
        z = 0;
#endif /* OS2 */
        break;

      case XXIFGU:                      /* GUI */
#ifdef KUI
        z = 1;
#else
        z = 0;
#endif /* KUI */
        break;

      case XXIFMS:                      /* MS-KERMIT */
        z = 0;
        break;

      case XXIFLO:                      /* IF LOCAL */
        z = local ? 1 : 0;
        break;

      case XXIFCM: {                    /* IF COMMAND */
          extern struct keytab cmdtab[];
          extern int ncmd;
          if ((x = cmfld("Word","",&s,xxstring)) < 0)
            return(x);
          z = lookup(cmdtab,s,ncmd,&y);
          z = (z == -2 || z > -1) ? 1 : 0;
          break;
      }
#ifdef CKFLOAT
      case XXIFFP:                      /* IF FLOAT */
        if ((x = cmfld("Number","",&s,xxstring)) < 0)
          if (x != -3)                  /* e.g. empty variable */
            return(x);
        z = isfloat(s,0);
        break;
#endif /* CKFLOAT */

      case XXIFKB:                      /* KBHIT */
        z = conchk();
        if (z < 0) z = 0;
        if (z > 1) z = 1;
        break;

      case XXIFKG: {                    /* KERBANG */
          extern int cfilef;
#ifdef COMMENT
          z = (xcmdsrc == 0) ? 0 : (cfilef && cmdlvl == 1);
#else
          z = (xcmdsrc == 0) ? 0 : (cfilef && tlevel == 0);
#endif	/* COMMENT */
          break;
      }

      case XXIFDB: {			/* IF DEBUG - 2010/03/16 */
          extern int debmsg;
          z = debmsg;
	  break;
      }

      default:                          /* Shouldn't happen */
        return(-2);
    } /* end of switch */

    if (z)
      *bx++ = '1';
    else
      *bx++ = '0';
    *bx = NUL;
    if (bx > boolval + BOOLLEN - 2) {
        printf("?Boolean expression too long");
        return(-9);
    }
    ecount++;                           /* Expression counter */
    debug(F101,"boolexp parens","",parens);
    debug(F101,"boolexp pcount","",pcount);
    if (parens && pcount > 0)
      goto ifagain;

  ifend:                                /* No more - done */
    *bx = NUL;
    z = atoi(evalx(boolval));
    debug(F111,"boolexp boolval",boolval,z);
    return(z);
}

/*  D O I F  --  Do the IF command  */

int
doif(cx) int cx; {
    int x, y, z; char *s, *p;
    char *q;
#ifdef OS2
    extern int keymac;
#endif /* OS2 */

    debug(F101,"doif cx","",cx);

    z = boolexp(cx);                    /* Evaluate the condition(s) */
    debug(F010,"doif cmdbuf",cmdbuf,0);
    debug(F101,"doif boolexp","",z);
    if (z < 0)
      return(z);

    if (cx == XXIF) {                   /* Allow IF to have XIF semantics. */
        char * p;
        p = cmpeek();
        if (!p) p = "";
        while (*p) {
            if (*p == SP || *p == HT)
              p++;
            else
              break;
        }
        if (*p == '{')
          cx = XXIFX;
    }
    switch (cx) {                       /* Separate handling for IF and XIF */

      case XXASSER:                     /* And ASSERT */
        if ((x = cmcfm()) < 0)
          return(x);
        return(success = z);

      case XXIF:                        /* This is IF... */
        ifcmd[cmdlvl] = 1;              /* We just completed an IF command */
        debug(F101,"doif condition","",z);
        if (z) {                        /* Condition is true */
            iftest[cmdlvl] = 1;         /* Remember that IF succeeded */
            if (maclvl > -1) {          /* In macro, */
                pushcmd(NULL);          /* save rest of command. */
            } else if (tlevel > -1) {   /* In take file, */
                debug(F100, "doif: pushing command", "", 0);
                pushcmd(NULL);          /* save rest of command. */
            } else {                    /* If interactive, */
                cmini(ckxech);          /* just start a new command */
                printf("\n");           /* (like in MS-DOS Kermit) */
                if (pflag) prompt(xxstring);
            }
        } else {                        /* Condition is false */
            iftest[cmdlvl] = 0;         /* Remember command failed. */
            if ((y = cmtxt("command to be ignored","",&s,NULL)) < 0)
              return(y);                /* Gobble up rest of line */
        }
        return(0);

      case XXIFX: {                     /* This is XIF (Extended IF) */
          char *p;
          char e[5];
          int i;
          if ((y = cmtxt("Object command","",&s,NULL)) < 0)
            return(y);                  /* Get object command. */
          p = s;
          lp = line;
          debug(F110,"doif THEN part",s,-54);
          if (litcmd(&p,&lp,LINBUFSIZ - 1) < 0) { /* Quote THEN-part */
              return(-2);
          }
          debug(F111,"doif THEN part 2",line,z);

          while (*p == SP) p++;         /* Strip trailing spaces */
          ifcmd[cmdlvl] = 0;            /* Assume ELSE part in same line */
          iftest[cmdlvl] = z ? 1 : 0;
          if (*p) {                     /* At end? */
              if (!z) {                 /* No, use ELSE-part, if any */
                  for (i = 0; i < 4; i++) e[i] = *p++;
                  if (ckstrcmp(e,"else",4,0)) /* See if we have an ELSE */
                    return(-2);         /* Something else - error. */
                  debug(F010,"doif ELSE line 1",p,0);
                  while (*p == SP) p++; /* Skip spaces */
                  if (*p != '{') {      /* Brace ELSE part if necessary */
                      ckmakmsg(tmpbuf,TMPBUFSIZ,"{",p," }",NULL);
                      p = tmpbuf;
                      debug(F010,"doif ELSE line 2",p,0);
                  }
                  lp = line;            /* Write over THEN part... */
                  *lp = NUL;            /* with ELSE part. */
                  if (litcmd(&p,&lp,LINBUFSIZ - 2) < 0) {
                      return(-2);
                  }
                  while (*p == SP) p++; /* Strip trailing spaces */
                  if (*p) return(-2);   /* Should be nothing here. */
                  debug(F010,"doif ELSE line 3",line,0);
              }
          } else {                      /* At end, treat like an IF command */
              if (!z) line[0] = NUL;    /* Condition not true and no ELSE */
              ifcmd[cmdlvl] = 1;        /* Allow ELSE on next line */
              debug(F101,"IF condition","",z);
          }
          if (line[0]) {
              x = mlook(mactab,"_xif",nmac); /* Get index of "_xif" macro. */
              if (x < 0) {                      /* Not there? */
                  addmmac("_xif",xif_def);      /* Put it back. */
                  if (mlook(mactab,"_xif",nmac) < 0) { /* Look it up again. */
                      printf("?XIF macro gone!\n");
                      return(success = 0);
                  }
              }
              dodo(x,line,cmdstk[cmdlvl].ccflgs | CF_IMAC);
          }
          return(0);
      }
      case XXWHI: {                     /* WHILE Command */
          p = cmdbuf;                   /* Capture IF condition */
          ifcond[0] = NUL;              /* from command buffer */
          while (*p == SP) p++;
          while (*p != SP) p++;
          ifcp = ifcond;
          ifcp += ckstrncpy(ifcp,"{ \\flit(if ( not ",IFCONDLEN);
#ifdef COMMENT
/*
  This doesn't work because it breaks on the first left brace, which does
  not necessarily start the command list, e.g. "while equal \%a {\35}".
*/
          while (*p != '{' && *p != NUL) *ifcp++ = *p++;
          p = " ) goto _..bot) } ";
          while (*ifcp++ = *p++) ;
#else
/*
  The command parser sets cmbptr to the spot where it left off parsing in
  the command buffer.
*/
          {
              extern char * cmbptr;
              if (cmbptr) {
                  while (p < cmbptr && *p != NUL)
                    *ifcp++ = *p++;
                  p = " ) goto _..bot) } ";
                  while ((*ifcp++ = *p++)) ;
              } else {
                  printf("?Internal error parsing WHILE condition\n");
                  return(-9);
              }
          }
#endif /* COMMENT */

          debug(F110,"WHILE cmd",ifcond,0);

          if ((y = cmtxt("Object command","",&s,NULL)) < 0)
            return(y);                  /* Get object command. */
          p = s;
          lp = line;
          if (litcmd(&p,&lp,LINBUFSIZ - 2) < 0) { /* Quote object command */
              return(-2);
          }
          debug(F101,"WHILE body",line,-54);
          if (line[0]) {
              char *p;
              x = mlook(mactab,"_while",nmac); /* index of "_while" macro. */
              if (x < 0) {              /* Not there? */
                  addmmac("_while",whil_def); /* Put it back. */
                  if (mlook(mactab,"_while",nmac) < 0) { /* Look it up again */
                      printf("?WHILE macro definition gone!\n");
                      return(success = 0);
                  }
              }
              p = malloc((int)strlen(ifcond) + (int)strlen(line) + 2);
              if (p) {
                  strcpy(p,ifcond);     /* safe (prechecked) */
                  strcat(p,line);       /* safe (prechecked) */
                  debug(F010,"WHILE dodo",p,0);
                  dodo(x,p,cmdstk[cmdlvl].ccflgs | CF_IMAC);
                  free(p);
                  p = NULL;
              } else {
                  printf("?Can't allocate storage for WHILE command");
                  return(success = 0);
              }
          }
          return(0);
      }
      default:
        return(-2);
    }
}
#endif /* NOSPL */

/* Set up a TAKE command file */

int
dotake(s) char *s; {
#ifndef NOSPL
    extern int tra_cmd;
#endif /* NOSPL */
#ifndef NOLOCAL
#ifdef OS2
    extern int term_io;
    int term_io_sav = term_io;
#endif /* OS2 */
#endif /* NOLOCAL */
    int slen;

    debug(F110,"dotake",s,0);
    if (!s) s = "";
    if (!*s) return(success = 0);
    slen = strlen(s);
    debug(F101,"dotake len","",slen);

    if ((tfile[++tlevel] = fopen(s,"r")) == NULL) {
        perror(s);
        debug(F110,"dotake fail",s,0);
        tlevel--;
        return(success = 0);
    } else {
        tfline[tlevel] = 0;             /* Line counter */
#ifdef VMS
        conres();                       /* So Ctrl-C will work */
#endif /* VMS */
#ifndef NOLOCAL
#ifdef OS2
        term_io = 0;                    /* Disable Terminal Emulator I/O */
#endif /* OS2 */
#endif /* NOLOCAL */
#ifndef NOSPL
        cmdlvl++;                       /* Entering a new command level */
        debug(F111,"CMD +F",s,cmdlvl);
        debug(F101,"dotake cmdlvl","",cmdlvl);
        debug(F101,"dotake tlevel","",tlevel);
        if (cmdlvl > CMDSTKL) {
            debug(F100,"dotake stack overflow","",0);
            cmdlvl--;
            debug(F111,"CMD*-F",s,cmdlvl);
            fclose(tfile[tlevel--]);
            printf("?TAKE files and/or DO commands nested too deeply\n");
            return(success = 0);
        }
        if (tfnam[tlevel]) {            /* Copy the filename */
            free(tfnam[tlevel]);
            tfnam[tlevel] = NULL;
        }
        if ((tfnam[tlevel] = malloc(strlen(s) + 1))) {
            strcpy(tfnam[tlevel],s);    /* safe */
        } else {
            printf("?Memory allocation failure\n");
            return(success = 0);
        }
        ifcmd[cmdlvl] = 0;              /* Set variables for this cmd file */
        iftest[cmdlvl] = 0;
        count[cmdlvl]  = count[cmdlvl-1];  /* Inherit this */
        intime[cmdlvl] = intime[cmdlvl-1]; /* Inherit this */
        inpcas[cmdlvl] = inpcas[cmdlvl-1]; /* Inherit this */
        takerr[cmdlvl] = takerr[cmdlvl-1]; /* Inherit this */
        merror[cmdlvl] = merror[cmdlvl-1]; /* Inherit this */
        xquiet[cmdlvl] = quiet;
        xvarev[cmdlvl] = vareval;
        xcmdsrc = CMD_TF;
        cmdstk[cmdlvl].src = CMD_TF;    /* Say we're in a TAKE file */
        cmdstk[cmdlvl].lvl = tlevel;    /* nested at this level */
        cmdstk[cmdlvl].ccflgs = cmdstk[cmdlvl-1].ccflgs;
#else
        takerr[tlevel] = takerr[tlevel-1]; /* Inherit this */
#endif /* NOSPL */
    }
#ifndef NOSPL
    if (tra_cmd)
      printf("[%d] +F: \"%s\"\n",cmdlvl,s);
#endif /* NOSPL */
#ifndef NOLOCAL
#ifdef OS2
    term_io = term_io_sav;
#endif /* OS2 */
#endif /* NOLOCAL */
    return(1);
}
#endif /* NOICP */

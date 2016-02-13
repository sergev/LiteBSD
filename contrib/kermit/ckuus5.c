#include "ckcsym.h"

int xcmdsrc = 0;

#ifdef NOICP
int cmdsrc() { return(0); }
#endif /* NOICP */

/*  C K U U S 5 --  "User Interface" for C-Kermit, part 5  */

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

#ifdef DCMDBUF
char *line;                             /* Character buffer for anything */
char *tmpbuf;
#else
char line[LINBUFSIZ+1];
char tmpbuf[TMPBUFSIZ+1];               /* Temporary buffer */
#endif /* DCMDBUF */

#ifndef NOICP

#include "ckcnet.h"
#ifndef NOCSETS
#include "ckcxla.h"
#endif /* NOCSETS */
#ifdef MAC
#include "ckmasm.h"
#endif /* MAC */
#ifdef CK_SSL
#include "ck_ssl.h"
#endif /* CK_SSL */

extern char * ck_cryear;       /* (ckcmai.c) Latest C-Kermit copyright year */

#ifdef OS2
#include "ckoetc.h"
#ifndef NT
#define INCL_NOPM
#define INCL_VIO /* Needed for ckocon.h */
#include <os2.h>
#undef COMMENT
#else /* NT */
#include <windows.h>
#define TAPI_CURRENT_VERSION 0x00010004
#include <tapi.h>
#include <mcx.h>
#include "ckntap.h"
#define APIRET ULONG
extern int DialerHandle;
extern int StartedFromDialer;
#endif /* NT */
#include "ckocon.h"
#include "ckokey.h"
#ifdef KUI
#include "ikui.h"
#endif /* KUI */
#ifdef putchar
#undef putchar
#endif /* putchar */
#define putchar(x) conoc(x)
extern int cursor_save ;
extern bool cursorena[] ;
#endif /* OS2 */

/* 2010-03-09 SMS.  VAX C V3.1-051 needs <stat.h> for off_t. */
#ifdef VMS
#include <stat.h>
#endif /* def VMS */

/* For formatted screens, "more?" prompting, etc. */

#ifdef FT18
#define isxdigit(c) isdigit(c)
#endif /* FT18 */

#ifdef STRATUS                          /* Stratus Computer, Inc.  VOS */
#ifdef putchar
#undef putchar
#endif /* putchar */
#define putchar(x) conoc(x)
#ifdef getchar
#undef getchar
#endif /* getchar */
#define getchar(x) coninc(0)
#endif /* STRATUS */

/* External variables */

extern int carrier, cdtimo, local, quiet, backgrd, bgset, sosi, xsuspend,
  binary, escape, xargs, flow, cmdmsk, duplex, ckxech, seslog, what,
  inserver, diractive, tlevel, cwdf, nfuncs, msgflg, remappd, hints, mdmtyp,
  zincnt, cmask, rcflag, success, xitsta, pflag, tnlm, tn_nlm, xitwarn,
  debses, xaskmore, parity, saveask, wasclosed, whyclosed, cdactive,
  rcdactive, keepallchars, cmd_err;

#ifdef LOCUS
extern int locus, autolocus;
#endif /* LOCUS */

#ifndef NOMSEND
extern int addlist;
#endif /* NOMSEND */

#ifdef CK_SPEED
extern int prefixing;
#endif /* CK_SPEED */

extern int g_matchdot;

#ifdef RECURSIVE
extern int recursive;
#endif /* RECURSIVE */
extern int xfiletype;

#ifdef IKSDCONF
extern char * iksdconf;
extern int iksdcf;
#endif /* IKSDCONF */

#ifdef CK_RECALL
extern int on_recall;
#endif /* CK_RECALL */

extern int ngetpath, exitonclose;
extern char * getpath[];
extern CHAR * epktmsg;

extern char * snd_move;
extern char * snd_rename;
extern char * rcv_move;
extern char * rcv_rename;
extern char * g_snd_move;
extern char * g_snd_rename;
extern char * g_rcv_move;
extern char * g_rcv_rename;

extern char * nm[];

#ifdef CK_UTSNAME
extern char unm_mch[];
extern char unm_mod[];
extern char unm_nam[];
extern char unm_rel[];
extern char unm_ver[];
#endif /* CK_UTSNAME */

#ifndef NOPUSH
#ifndef NOFRILLS
extern char editor[];
extern char editfile[];
extern char editopts[];
#ifdef BROWSER
extern char browser[];
extern char browsopts[];
extern char browsurl[];
#endif /* BROWSER */
#endif /*  NOFRILLS */
#endif /* NOPUSH */

#ifndef NOFRILLS
#ifndef NORENAME
_PROTOTYP(VOID shorename, (void));
#endif	/* NORENAME */
#endif	/* NOFRILLS */

#ifndef NOSERVER
extern char * x_user, * x_passwd, * x_acct;
#endif /* NOSERVER */

#ifdef CKLOGDIAL
extern int dialog;
extern char diafil[];
#endif /* CKLOGDIAL */

#ifdef CKROOT
extern int ckrooterr;
#endif /* CKROOT */

#ifndef NOSPL
extern int cfilef, xxdot, vareval;
extern char cmdfil[];

struct localvar * localhead[CMDSTKL];
struct localvar * localtail = NULL;
struct localvar * localnext = NULL;

_PROTOTYP( VOID shosexp, (void) );
_PROTOTYP( static VOID shoinput, (void) );
_PROTOTYP( static char gettok,   (void) );
_PROTOTYP( static VOID factor,   (void) );
_PROTOTYP( static VOID term,     (void) );
_PROTOTYP( static VOID termp,    (void) );
_PROTOTYP( static VOID exprp,    (void) );
_PROTOTYP( static VOID expr,     (void) );
_PROTOTYP( static VOID simple,   (void) );
_PROTOTYP( static VOID simpler,  (void) );
_PROTOTYP( static VOID simplest, (void) );
_PROTOTYP( static CK_OFF_T xparse,   (void) );
#endif /* NOSPL */
#ifndef NOSHOW
_PROTOTYP( int sho_iks, (void) );
#endif /* NOSHOW */

#ifdef MAC
char * ckprompt = "Mac-Kermit>";        /* Default prompt for Macintosh */
char * ikprompt = "IKSD>";
#else  /* Not MAC */
#ifdef NOSPL
#ifdef OS2
char * ckprompt = "K-95> ";             /* Default prompt for Win32 */
char * ikprompt = "IKSD> ";
#else
char * ckprompt = "C-Kermit>";
char * ikprompt = "IKSD>";
#endif /* NT */
#else  /* NOSPL */
#ifdef OS2
/* Default prompt for OS/2 and Win32 */
#ifdef NT
char * ckprompt = "[\\freplace(\\flongpath(\\v(dir)),/,\\\\)] K-95> ";
char * ikprompt = "[\\freplace(\\flongpath(\\v(dir)),/,\\\\)] IKSD> ";
#else  /* NT */
char * ckprompt = "[\\freplace(\\v(dir),/,\\\\)] K-95> ";
char * ikprompt = "[\\freplace(\\v(dir),/,\\\\)] IKSD> ";
#endif /* NT */
#else  /* OS2 */
#ifdef VMS
char * ckprompt = "\\v(dir) C-Kermit>"; /* Default prompt VMS */
char * ikprompt = "\\v(dir) IKSD>";
#else
char * ckprompt = "(\\v(dir)) C-Kermit>"; /* Default prompt for others */
char * ikprompt = "(\\v(dir)) IKSD>";
#endif /* VMS */
#endif /* NT */
#endif /* NOSPL */
#endif /* MAC */

#ifndef CCHMAXPATH
#define CCHMAXPATH 257
#endif /* CCHMAXPATH */
char inidir[CCHMAXPATH] = { NUL, NUL }; /* Directory INI file executed from */

#ifdef TNCODE
extern int tn_b_nlm;                    /* TELNET BINARY newline mode */
#endif /* TNCODE */

#ifndef NOKVERBS
extern struct keytab kverbs[];          /* Table of \Kverbs */
extern int nkverbs;                     /* Number of \Kverbs */
#endif /* NOKVERBS */

#ifndef NOPUSH
extern int nopush;
#endif /* NOPUSH */

#ifdef CK_RECALL
extern int cm_recall;
#endif /* CK_RECALL */

extern char *ccntab[];

/* Printer stuff */

extern char *printername;
extern int printpipe;
#ifdef BPRINT
extern int printbidi, pportparity, pportflow;
extern long pportspeed;
#endif /* BPRINT */

#ifdef OS2
_PROTOTYP (int os2getcp, (void) );
_PROTOTYP (int os2getcplist, (int *, int) );
#ifdef OS2MOUSE
extern int tt_mouse;
#endif /* OS2MOUSE */
extern int tt_update, tt_updmode, updmode, tt_utf8;
#ifndef IKSDONLY
extern int tt_status[];
#endif /* IKSDONLY */
#ifdef PCFONTS
extern struct keytab term_font[];
#else
#ifdef KUI
extern struct keytab * term_font;
#endif /* KUI */
#endif /* PCFONTS */
extern int ntermfont, tt_font, tt_font_size;
extern unsigned char colornormal, colorunderline, colorstatus,
    colorhelp, colorselect, colorborder, colorgraphic, colordebug,
    colorreverse, colorcmd, coloritalic;
extern int priority;
extern struct keytab prtytab[];
extern int nprty;
char * cmdmac = NULL;
#endif /* OS2 */

#ifdef VMS
_PROTOTYP (int zkermini, (char *, int, char *) );
#endif /* VMS */

extern long vernum;
extern int inecho, insilence, inbufsize, nvars, inintr;
extern char *protv, *fnsv, *cmdv, *userv, *ckxv, *ckzv, *ckzsys, *xlav,
 *cknetv, *clcmds;
#ifdef OS2
extern char *ckyv;
#endif /* OS2 */
#ifdef CK_AUTHENTICATION
extern char * ckathv;
#endif /* CK_AUTHENTICATION */
#ifdef CK_SSL
extern char * cksslv;
#endif /* CK_SSL */
#ifdef CK_ENCRYPTION
#ifndef CRYPT_DLL
extern char * ckcrpv;
#endif /* CRYPT_DLL */
#endif /* CK_ENCRYPTION */

#ifdef SSHBUILTIN
extern char *cksshv;
#ifdef SFTP_BUILTIN
extern char *cksftpv;
#endif /* SFTP_BUILTIN */
#endif /* SSHBUILTIN */

#ifdef TNCODE
extern char *cktelv;
#endif /* TNCODE */
#ifndef NOFTP
#ifndef SYSFTP
extern char * ckftpv;
#endif /* SYSFTP */
#endif /* NOFTP */

extern int srvidl;

#ifdef OS2
extern char *ckonetv;
extern int interm;
#ifdef CK_NETBIOS
extern char *ckonbiv;
#endif /* CK_NETBIOS */
#ifdef OS2MOUSE
extern char *ckomouv;
#endif /* OS2MOUSE */
#endif /* OS2 */

#ifndef NOLOCAL
extern char *connv;
#endif /* NOLOCAL */
#ifndef NODIAL
extern char *dialv;
#endif /* NODIAL */
#ifndef NOSCRIPT
extern char *loginv;
extern int secho;
#endif /* NOSCRIPT */

#ifndef NODIAL
extern int nmdm, dirline;
extern struct keytab mdmtab[];
#endif /* NODIAL */

extern int network, nettype, ttnproto;

#ifdef OS2
#ifndef NOTERM
/* SET TERMINAL items... */
extern int tt_type, tt_arrow, tt_keypad, tt_wrap, tt_answer, tt_scrsize[];
extern int tt_bell, tt_roll[], tt_ctstmo, tt_cursor, tt_pacing, tt_type_mode;
extern char answerback[];
extern struct tt_info_rec tt_info[];    /* Indexed by terminal type */
extern int max_tt;
#endif /* NOTERM */
#endif /* OS2 */

_PROTOTYP( VOID shotrm, (void) );
_PROTOTYP( int shofea, (void) );

#ifdef OS2
extern int tt_rows[], tt_cols[];
#else /* OS2 */
extern int tt_rows, tt_cols;
#endif /* OS2 */
extern int cmd_rows, cmd_cols;

#ifdef CK_TMPDIR
extern int f_tmpdir;                    /* Directory changed temporarily */
extern char savdir[];                   /* Temporary directory */
#endif /* CK_TMPDIR */

#ifndef NOLOCAL
extern int tt_crd, tt_lfd, tt_escape;
#endif /* NOLOCAL */

#ifndef NOCSETS
extern int language, nfilc, tcsr, tcsl, tcs_transp, fcharset;
extern struct keytab fcstab[];
extern struct csinfo fcsinfo[];
#ifndef MAC
extern struct keytab ttcstab[];
#endif /* MAC */
#endif /* NOCSETS */

extern long speed;

#ifndef NOXMIT
extern int xmitf, xmitl, xmitp, xmitx, xmits, xmitw, xmitt;
extern char xmitbuf[];
#endif /* NOXMIT */

extern char **xargv, *versio, *ckxsys, *dftty, *lp;

#ifdef DCMDBUF
extern char *cmdbuf, *atmbuf;           /* Command buffers */
#ifndef NOSPL
extern char *savbuf;                    /* Command buffers */
#endif /* NOSPL */
#else
extern char cmdbuf[], atmbuf[];         /* Command buffers */
#ifndef NOSPL
extern char savbuf[];                   /* Command buffers */
#endif /* NOSPL */
#endif /* DCMDBUF */

extern char toktab[], ttname[], psave[];
extern CHAR sstate, feol;
extern int cmflgs, techo, repars, ncmd;
extern struct keytab cmdtab[];

#ifndef NOSETKEY
KEY *keymap;
#ifndef OS2
#define mapkey(x) keymap[x]
#endif /* OS2 */
MACRO *macrotab;
_PROTOTYP( VOID shostrdef, (CHAR *) );
#endif /* NOSETKEY */

extern int cmdlvl;

#ifndef NOSPL
extern struct mtab *mactab;
extern struct keytab mackey[];
extern struct keytab vartab[], fnctab[], iftab[];
extern int maclvl, nmac, mecho, fndiags, fnerror, fnsuccess, nif;
#endif /* NOSPL */

FILE *tfile[MAXTAKE];                   /* TAKE file stack */
char *tfnam[MAXTAKE];
int tfline[MAXTAKE];

int topcmd = -1;                        /* cmdtab index of current command */
int havetoken = 0;
extern int dblquo;                      /* Doublequoting enabled */

#ifdef DCMDBUF                          /* Initialization filespec */
char *kermrc = NULL;
#else
char kermrcb[KERMRCL];
char *kermrc = kermrcb;
#endif /* DCMDBUF */

int noherald = 0;
int cm_retry = 1;                       /* Command retry enabled */
xx_strp xxstring = zzstring;

#ifndef NOXFER
extern int displa, bye_active, protocol, pktlog, remfile, rempipe, unkcs,
  keep, lf_opts, fncnv, pktpaus, autodl, xfrcan, xfrchr, xfrnum, srvtim,
  srvdis, query, retrans, streamed, reliable, crunched, timeouts,
  fnrpath, autopath, rpackets, spackets, epktrcvd, srvping;

#ifdef CK_AUTODL
extern int inautodl, cmdadl;
#endif /* CK_AUTODL */

#ifndef NOSERVER
extern int en_asg, en_cwd, en_cpy, en_del, en_dir, en_fin, en_bye, en_ret,
  en_get, en_hos, en_que, en_ren, en_sen, en_set, en_spa, en_typ, en_who,
  en_mai, en_pri, en_mkd, en_rmd, en_xit, en_ena;
#endif /* NOSERVER */

extern int atcapr,
  atenci, atenco, atdati, atdato, atleni, atleno, atblki, atblko,
  attypi, attypo, atsidi, atsido, atsysi, atsyso, atdisi, atdiso;

#ifdef STRATUS
extern int atfrmi, atfrmo, atcrei, atcreo, atacti, atacto;
#endif /* STRATUS */

#ifdef CK_PERMS
extern int atlpri, atlpro, atgpri, atgpro;
#endif /* CK_PERMS */

#ifdef CK_LOGIN
extern char * anonfile;                 /* Anonymous login init file */
extern char * anonroot;                 /* Anonymous file-system root */
extern char * userfile;                 /* Forbidden user file */
extern int isguest;                     /* Flag for anonymous user */
#endif /* CK_LOGIN */
#endif /* NOXFER */

#ifdef DCMDBUF
int *xquiet = NULL;
int *xvarev = NULL;
#else
int xquiet[CMDSTKL];
int xvarev[CMDSTKL];
#endif /* DCMDBUF */

char * prstring[CMDSTKL];

#ifndef NOSPL

extern long ck_alarm;
extern char alrm_date[], alrm_time[];

/* Local declarations */

static int nulcmd = 0;                  /* Flag for next cmd to be ignored */

/* Definitions for predefined macros */

/* First, the single-line macros, installed with addmac()... */

/* IBM-LINEMODE macro */
char *m_ibm = "set parity mark, set dupl half, set handsh xon, set flow none";

/* FATAL macro */
char *m_fat = "if def \\%1 echo \\%1, if not = \\v(local) 0 hangup, stop 1";

#ifdef CK_SPEED
#ifdef IRIX65
char *m_fast = "set win 30, set rec pack 4000, set prefix cautious";
#else
#ifdef IRIX
/* Because of bug in telnet server */
char *m_fast = "set window 30, set rec pack 4000, set send pack 4000,\
 set pref cautious";
#else
#ifdef pdp11
char *m_fast = "set win 3, set rec pack 1024, set prefix cautious";
#else
#ifdef BIGBUFOK
char *m_fast = "set win 30, set rec pack 4000, set prefix cautious";
#else
char *m_fast = "set win 4, set rec pack 2200, set prefix cautious";
#endif /* BIGBUFOK */
#endif /* IRIX */
#endif /* IRIX65 */
#endif /* pdp11 */
#ifdef pdp11
char *m_cautious = "set win 2, set rec pack 512, set prefixing cautious";
#else
char *m_cautious = "set win 4, set rec pack 1000, set prefixing cautious";
#endif /* pdp11 */
char *m_robust = "set win 1, set rec pack 90, set prefixing all, \
set reliable off, set clearchannel off";
#else
#ifdef BIGBUFOK
#ifdef IRIX65
char *m_fast = "set win 30, set rec pack 4000";
#else
#ifdef IRIX
char *m_fast = "set win 30, set rec pack 4000, set send pack 4000";
#else
char *m_fast = "set win 30, set rec pack 4000";
#endif /* IRIX */
#endif /* IRIX65 */
#else /* Not BIGBUFOK */
char *m_fast = "set win 4, set rec pack 2200";
#endif /* BIGBUFOK */
char *m_cautious = "set win 4, set rec pack 1000";
char *m_robust = "set win 1, set rec pack 90, set reliable off";
#endif /* CK_SPEED */

#ifdef VMS
char *m_purge = "run purge \\%*";
#endif /* VMS */

#ifdef OS2
char *m_manual = "browse \\v(exedir)docs/manual/kermit95.htm";
#endif /* OS2 */

/* Now the multiline macros, defined with addmmac()... */

/* FOR macro for \%i-style loop variables (see dofor()...) */

char *for_def[] = { "_assign _for\\v(cmdlevel) { _getargs,",
"def \\\\\\%1 \\feval(\\%2),:_..top,if \\%5 \\\\\\%1 \\%3 goto _..bot,",
"\\%6,:_..inc,incr \\\\\\%1 \\%4,goto _..top,:_..bot,_putargs},",
"def break goto _..bot, def continue goto _..inc,",
"do _for\\v(cmdlevel) \\%1 \\%2 \\%3 \\%4 { \\%5 },_assign _for\\v(cmdlevel)",
""};

/* This is the FOR macro when the loop variable is itself a macro */

char *foz_def[] = { "_assign _for\\v(cmdlevel) { _getargs,",
"def \\%1 \\feval(\\%2),:_..top,if \\%5 \\%1 \\%3 goto _..bot,",
"\\%6,:_..inc,incr \\%1 \\%4,goto _..top,:_..bot,_putargs},",
"def break goto _..bot, def continue goto _..inc,",
"do _for\\v(cmdlevel) \\%1 \\%2 \\%3 \\%4 { \\%5 },_assign _for\\v(cmdlevel)",
""};

/* WHILE macro */
char *whil_def[] = { "_assign _whi\\v(cmdlevel) {_getargs,",
":_..inc,\\%1,\\%2,goto _..inc,:_..bot,_putargs},",
"_def break goto _..bot, _def continue goto _..inc,",
"do _whi\\v(cmdlevel),_assign _whi\\v(cmdlevel)",
""};

/* SWITCH macro */
char *sw_def[] = { "_assign _sw_\\v(cmdlevel) {_getargs,",
"_forward {\\%1},\\%2,:default,:_..bot,_putargs},_def break goto _..bot,",
"do _sw_\\v(cmdlevel),_assign _sw_\\v(cmdlevel)",
""};

/* XIF macro */
char *xif_def[] = {
"_assign _if\\v(cmdlevel) {_getargs,\\%1,_putargs},",
"do _if\\v(cmdlevel),_assign _if\\v(cmdlevel)",
""};

/*
  Variables declared here for use by other ckuus*.c modules.
  Space is allocated here to save room in ckuusr.c.
*/
#ifdef DCMDBUF
struct cmdptr *cmdstk;
int
  *ifcmd  = NULL,
  *count  = NULL,
  *iftest = NULL,
  *intime = NULL,
  *inpcas = NULL,
  *takerr = NULL,
  *merror = NULL;
#else
struct cmdptr cmdstk[CMDSTKL];
int ifcmd[CMDSTKL], count[CMDSTKL], iftest[CMDSTKL], intime[CMDSTKL],
  inpcas[CMDSTKL], takerr[CMDSTKL], merror[CMDSTKL];
#endif /* DCMDBUF */

/* Macro stack */

#ifdef COMMENT
char *topline = NULL;                   /* Program invocation arg line */
char *m_line[MACLEVEL] = { NULL, NULL }; /* Stack of macro invocation lines */
#endif /* COMMENT */

char **m_xarg[MACLEVEL];                /* Pointers to arg vector arrays */
int n_xarg[MACLEVEL];                   /* Sizes of arg vector arrays */
char *m_arg[MACLEVEL][NARGS];           /* Args of each level */
int macargc[MACLEVEL];                  /* Argc of each level */
char *macp[MACLEVEL];                   /* Current position in each macro */
char *macx[MACLEVEL];                   /* Beginning of each macro def */
char *mrval[MACLEVEL];                  /* RETURN value at each level */
int lastcmd[MACLEVEL];                  /* Last command at each level */
int topargc = 0;                        /* Argc at top level */
char **topxarg = NULL;                  /* Argv at top level */
char *toparg[MAXARGLIST+2];

/* Global Variables */

char *g_var[GVARS+1];                   /* Global \%a..z pointers */
extern char varnam[];                   /* \%x variable name buffer */

/* Arrays -- Dimension must be 'z' - ARRAYBASE + 1 */
/* Note: a_link[x] < 0 means no link; >= 0 is a link */

char **a_ptr[32];                       /* Array pointers, for arrays a-z */
int a_dim[32];                          /* Dimensions for each array */
int a_link[32];                         /* Link (index of linked-to-array) */

char **aa_ptr[CMDSTKL][32];             /* Array stack for automatic arrays */
int aa_dim[CMDSTKL][32];                /* Dimensions for each array */

/* INPUT command buffers and variables */

char * inpbuf = NULL;                   /* Buffer for INPUT and REINPUT */
extern char * inpbp;                    /* Global/static pointer to it  */
char inchar[2] = { NUL, NUL };          /* Last character that was INPUT */
int  incount = 0;                       /* INPUT character count */
extern int instatus;                    /* INPUT status */
static char * i_text[] = {              /* INPUT status text */
    "success", "timeout", "interrupted", "internal error", "i/o error"
};

char lblbuf[LBLSIZ];                    /* Buffer for labels */

#else  /* NOSPL */

int takerr[MAXTAKE];
#endif /* NOSPL */

static char *prevdir = NULL;

int pacing = 0;                         /* OUTPUT pacing */

char *tp;                               /* Temporary buffer pointer */

int timelimit = 0, asktimer = 0;        /* Timers for time-limited commands */

#ifdef CK_APC                           /* Application Program Command (APC) */
int apcactive = APC_INACTIVE;
int apcstatus = APC_OFF;                /* OFF by default everywhere */
#ifdef DCMDBUF
char *apcbuf;
#else
char apcbuf[APCBUFLEN];
#endif /* DCMDBUF */
#endif /* CK_APC */

extern char pktfil[],
#ifdef DEBUG
  debfil[],
#endif /* DEBUG */
#ifdef TLOG
  trafil[],
#endif /* TLOG */
  sesfil[];

#ifndef NOFRILLS
extern int rmailf, rprintf;             /* REMOTE MAIL & PRINT items */
extern char optbuf[];
#endif /* NOFRILLS */

extern int noinit;			/* Flat to skip init file */

#ifndef NOSPL
static struct keytab kcdtab[] = {	/* Symbolic directory names */
#ifdef NT
    { "appdata",  VN_APPDATA,   0 },
    { "common",   VN_COMMON,    0 },
    { "desktop",  VN_DESKTOP,   0 },
#endif /* NT */
    { "download", VN_DLDIR,     0 },
#ifdef OS2ORUNIX
    { "exedir",   VN_EXEDIR,    0 },
#endif /* OS2ORUNIX */
    { "home",     VN_HOME,      0 },
    { "inidir",   VN_INI,       0 },
#ifdef UNIX
    { "lockdir",  VN_LCKDIR,    0 },
#endif /* UNIX */
#ifdef NT
    { "my_documents",VN_PERSONAL,  0 },
    { "personal", VN_PERSONAL,  CM_INV },
#endif /* NT */
    { "startup",  VN_STAR,      0 },
    { "textdir",  VN_TXTDIR,    0 },
    { "tmpdir",   VN_TEMP,      0 }
};
static int nkcdtab = (sizeof(kcdtab) / sizeof(struct keytab));
#endif /* NOSPL */

#ifndef NOSPL
_PROTOTYP( VOID freelocal, (int) );
_PROTOTYP( static CK_OFF_T expon, (CK_OFF_T, CK_OFF_T) );
_PROTOTYP( static CK_OFF_T gcd, (CK_OFF_T, CK_OFF_T) );
_PROTOTYP( static CK_OFF_T fact, (CK_OFF_T) );

int                     /* Initialize macro data structures. */
macini() {              /* Allocate mactab and preset the first element. */
    int i;
    if (!(mactab = (struct mtab *) malloc(sizeof(struct mtab) * MAC_MAX)))
      return(-1);
    mactab[0].kwd = NULL;
    mactab[0].mval = NULL;
    mactab[0].flgs = 0;
    for (i = 0; i < MACLEVEL; i++)
      localhead[i] = NULL;
    return(0);
}
#endif /* NOSPL */

/*  C M D S R C  --  Returns current command source  */

/*  0 = top level, 1 = file, 2 = macro, -1 = error (shouldn't happen) */

/*
  As of 19 Aug 2000 this routine is obsolete.  The scalar global variable
  xcmdsrc can be checked instead to save the overhead of a function call.
*/
int
cmdsrc() {
#ifdef COMMENT
    return(xcmdsrc);
#else
#ifndef NOSPL
    if (cmdlvl == 0)
      return(0);
    else if (cmdstk[cmdlvl].src == CMD_MD)
      return(2);
    else if (cmdstk[cmdlvl].src == CMD_TF)
      return(1);
    else
      return(-1);
#else
    if (tlevel < 0)
      return(0);
    else
      return(1);
#endif /* NOSPL */
#endif /* COMMENT */
}

/*  C M D I N I  --  Initialize the interactive command parser  */

static int cmdinited = 0;               /* Command parser initialized */
extern int cmdint;                      /* Interrupts are allowed */
#ifdef CK_AUTODL
int cmdadl = 1;                         /* Autodownload */
#else
int cmdadl = 0;
#endif /* CK_AUTODL */

char * k_info_dir = NULL;               /* Where to find text files */
#ifdef UNIX
static char * txtdir[] = {
    "/usr/local/doc/",                  /* Linux, SunOS, ... */
    "/usr/share/lib/",                  /* HP-UX 10.xx... */
    "/usr/share/doc/",                  /* Other possibilities... */
    "/usr/local/lib/",                  /* NOTE: Each of these is tried */
    "/usr/local/share/",                /* as is, and also with a kermit */
    "/usr/local/share/doc/",            /* subdirectory. */
    "/usr/local/share/lib/",
    "/opt/kermit/",                     /* Solaris */
    "/opt/kermit/doc/",
    "/opt/",
    "/usr/doc/",
    "/doc/",
    ""
};
#endif /* UNIX */

/*
  lookup() cache to speed up script execution.

  This is a static cache.  Items are stored in decreasing frequency of
  reference based on statistics from a range of scripts.  This gives
  better performance than a dynamic cache, which would require a lot more
  code and also would require system-dependent elements including system
  calls (e.g. to get subsecond times for entry aging).
*/
#ifdef USE_LUCACHE                      /* Set in ckuusr.h */
#define LUCACHE 32                      /* Change this to reduce cache size */
int lusize = 0;
char * lucmd[LUCACHE];
int luval[LUCACHE];
int luidx[LUCACHE];
struct keytab * lutab[LUCACHE];
#endif /* USE_LUCACHE */

static VOID
luinit() {                              /* Initialize lookup() cache */
    int x, y;

#ifdef USE_LUCACHE
    x = lookup(cmdtab,"if",ncmd,&y);
    lucmd[lusize] = "if";
    luval[lusize] = x;
    luidx[lusize] = y;
    lutab[lusize] = cmdtab;
    if (++lusize > LUCACHE) return;

    x = lookup(iftab,"not",nif,&y);
    lucmd[lusize] = "not";
    luval[lusize] = x;
    luidx[lusize] = y;
    lutab[lusize] = iftab;
    if (++lusize > LUCACHE) return;

    x = lookup(vartab,"cmdlevel",nvars,&y);
    lucmd[lusize] = "cmdlevel";
    luval[lusize] = x;
    luidx[lusize] = y;
    lutab[lusize] = vartab;
    if (++lusize > LUCACHE) return;

    x = lookup(cmdtab,"goto",ncmd,&y);
    lucmd[lusize] = "goto";
    luval[lusize] = x;
    luidx[lusize] = y;
    lutab[lusize] = cmdtab;
    if (++lusize > LUCACHE) return;

    x = lookup(iftab,">",nif,&y);
    lucmd[lusize] = ">";
    luval[lusize] = x;
    luidx[lusize] = y;
    lutab[lusize] = iftab;
    if (++lusize > LUCACHE) return;

    x = lookup(cmdtab,"incr",ncmd,&y);
    lucmd[lusize] = "incr";
    luval[lusize] = x;
    luidx[lusize] = y;
    lutab[lusize] = cmdtab;
    if (++lusize > LUCACHE) return;

    x = lookup(cmdtab,"def",ncmd,&y);
    lucmd[lusize] = "def";
    luval[lusize] = x;
    luidx[lusize] = y;
    lutab[lusize] = cmdtab;
    if (++lusize > LUCACHE) return;

    x = lookup(cmdtab,"_assign",ncmd,&y);
    lucmd[lusize] = "_assign";
    luval[lusize] = x;
    luidx[lusize] = y;
    lutab[lusize] = cmdtab;
    if (++lusize > LUCACHE) return;

    x = lookup(cmdtab,"echo",ncmd,&y);
    lucmd[lusize] = "echo";
    luval[lusize] = x;
    luidx[lusize] = y;
    lutab[lusize] = cmdtab;
    if (++lusize > LUCACHE) return;

    x = lookup(fnctab,"eval",nfuncs,&y);
    lucmd[lusize] = "eval";
    luval[lusize] = x;
    luidx[lusize] = y;
    lutab[lusize] = fnctab;
    if (++lusize > LUCACHE) return;

    x = lookup(fnctab,"lit",nfuncs,&y);
    lucmd[lusize] = "lit";
    luval[lusize] = x;
    luidx[lusize] = y;
    lutab[lusize] = fnctab;
    if (++lusize > LUCACHE) return;

    x = lookup(cmdtab,"do",ncmd,&y);
    lucmd[lusize] = "do";
    luval[lusize] = x;
    luidx[lusize] = y;
    lutab[lusize] = cmdtab;
    if (++lusize > LUCACHE) return;

    x = lookup(cmdtab,"_getargs",ncmd,&y);
    lucmd[lusize] = "_getargs";
    luval[lusize] = x;
    luidx[lusize] = y;
    lutab[lusize] = cmdtab;
    if (++lusize > LUCACHE) return;

    x = lookup(iftab,"<",nif,&y);
    lucmd[lusize] = "<";
    luval[lusize] = x;
    luidx[lusize] = y;
    lutab[lusize] = iftab;
    if (++lusize > LUCACHE) return;

    x = lookup(cmdtab,"_putargs",ncmd,&y);
    lucmd[lusize] = "_putargs";
    luval[lusize] = x;
    luidx[lusize] = y;
    lutab[lusize] = cmdtab;
    if (++lusize > LUCACHE) return;

    x = lookup(cmdtab,"asg",ncmd,&y);
    lucmd[lusize] = "asg";
    luval[lusize] = x;
    luidx[lusize] = y;
    lutab[lusize] = cmdtab;
    if (++lusize > LUCACHE) return;

    x = lookup(cmdtab,"else",ncmd,&y);
    lucmd[lusize] = "else";
    luval[lusize] = x;
    luidx[lusize] = y;
    lutab[lusize] = cmdtab;
#endif /* USE_LUCACHE */
}

VOID
cmdini() {
    int i = 0, x = 0, y = 0, z = 0, skip = 0;
    char * p;
#ifdef TTSPDLIST
    long * ss = NULL;
    extern int nspd;
    extern struct keytab * spdtab;
#endif /* TTSPDLIST */

#ifndef NOSPL
/*
  On stack to allow recursion!
*/
    char vnambuf[VNAML];                /* Buffer for variable names */
#endif /* NOSPL */

    if (cmdinited)                      /* Already initialized */
      return;                           /* Don't do it again */

    for (i = 0; i < CMDSTKL; i++)       /* Prompt strings for each */
      prstring[i] = NULL;               /* command level */

#ifndef NOCSETS
    p = getenv("K_CHARSET");            /* Set default file character set */
    if (p) {                            /* from environment */
        x = lookup(fcstab,p,nfilc,&y);
        if (x > -1)
          fcharset = x;
    }
#endif /* NOCSETS */

    p = getenv("K_INFO_DIRECTORY");     /* Find Kermit info directory */
    if (p && *p && strlen(p) <= CKMAXPATH)
      makestr(&k_info_dir,p);
    if (!k_info_dir) {
        p = getenv("K_INFO_DIR");
        if (p && *p && strlen(p) <= CKMAXPATH)
          makestr(&k_info_dir,p);
    }
#ifdef UNIX
    if (k_info_dir) {                   /* Look for Kermit docs directory */
        if (zchki(k_info_dir) == -2) {
            char xbuf[CKMAXPATH+32], *s = "";
            if (ckrchar(k_info_dir) != '/')
              s = "/";
            ckmakmsg(xbuf,CKMAXPATH+32,k_info_dir,s,"ckubwr.txt",NULL);
            if (zchki(xbuf) < 0)
              makestr(&k_info_dir,NULL);
        }
    }
    if (!k_info_dir) {
        char xbuf[CKMAXPATH+32];
        int i;
        for (i = 0; *(txtdir[i]); i++) {
            ckmakmsg(xbuf,CKMAXPATH+32,txtdir[i],"ckubwr.txt",NULL,NULL);
            if (zchki(xbuf) > 0) {
                makestr(&k_info_dir,txtdir[i]);
                debug(F110,"k_info_dir 1",k_info_dir,0);
                break;
            }
            ckmakmsg(xbuf,CKMAXPATH+32,
                     txtdir[i],"kermit/","ckubwr.txt",NULL);
            if (zchki(xbuf) > 0) {
                ckmakmsg(xbuf,CKMAXPATH+32,txtdir[i],"kermit/",NULL,NULL);
                makestr(&k_info_dir,xbuf);
                debug(F110,"k_info_dir 2",k_info_dir,0);
                break;
            }
            ckmakmsg(xbuf,CKMAXPATH+32,
                     txtdir[i],"ckermit/","ckubwr.txt",NULL);
            if (zchki(xbuf) > 0) {
                ckmakmsg(xbuf,CKMAXPATH+32,txtdir[i],"ckermit/",NULL,NULL);
                makestr(&k_info_dir,xbuf);
                debug(F110,"k_info_dir 3",k_info_dir,0);
                break;
            }
        }
        if (k_info_dir) {               /* Make sure it ends with "/" */
            if (ckrchar(k_info_dir) != '/') {
                char xbuf[CKMAXPATH+32];
                ckmakmsg(xbuf,CKMAXPATH+32,k_info_dir,"/",NULL,NULL);
                makestr(&k_info_dir,xbuf);
            }
        }
    }
#else
#ifdef OS2
    {
        char xdir[CKMAXPATH+8], *s = "";
        extern char startupdir[];
        xdir[0] = NUL;
        if (ckrchar(startupdir) != '/')
          s = "/";
        if (strlen(s) + strlen(startupdir) + 5 < CKMAXPATH + 8 )
          ckmakmsg(xdir,CKMAXPATH+8,s,startupdir,"DOC/",NULL);
        makestr(&k_info_dir,xdir);
    }
#endif /* OS2 */
#endif /* UNIX */

#ifdef TTSPDLIST
    if (!spdtab && (ss = ttspdlist())) { /* Get speed list if necessary */
        int j, k, m = 0, n;             /* Create sorted keyword table */
        char buf[16];
        char * p;
        if ((spdtab =
             (struct keytab *) malloc(sizeof(struct keytab) * ss[0]))) {
            for (i = 1; i <= ss[0]; i++) { /* ss[0] = number of elements */
                if (ss[i] < 1L) break;     /* Shouldn't happen */
                buf[0] = NUL;              /* Make string */
                sprintf(buf,"%ld",ss[i]);  /* SAFE */
                if (ss[i] == 8880L)
                  ckstrncpy(buf,"75/1200",sizeof(buf));
                if (ss[i] == 134L)
                  ckstrncat(buf,".5",16);
                n = strlen(buf);
                if ((n > 0) && (p = (char *)malloc(n+1))) {
                    if (m > 0) {        /* Have at least one in list */
                        for (j = 0;     /* Find slot */
                             j < m && strcmp(buf,spdtab[j].kwd) > 0;
                             j++
                             )
                          ;
                        if (j < m) {    /* Must insert */
                            for (k = m-1; k >= j; k--) { /* Move others down */
                                spdtab[k+1].kwd = spdtab[k].kwd;
                                spdtab[k+1].flgs = spdtab[k].flgs;
                                spdtab[k+1].kwval = spdtab[k].kwval;
                            }
                        }
                    } else              /* First one */
                      j = 0;
                    ckstrncpy(p,buf,n+1); /* Add new speed */
                    spdtab[j].kwd = p;
                    spdtab[j].flgs = 0;
                    spdtab[j].kwval = (int) ss[i] / 10;
                    m++;                /* Count this one */
                }
            }
        }
        nspd = m;
    }
#endif /* TTSPDLIST */

#ifndef NOSPL
    /* Allocate INPUT command buffer */
    if (!inpbuf) {
        if (!(inpbuf = (char *) malloc(INPBUFSIZ+8)))
          fatal("cmdini: no memory for INPUT buffer");
    }
    for (x = 0; x < INPBUFSIZ; x++)     /* Initialize it */
      inpbuf[x] = NUL;
    inpbp = inpbuf;                     /* Initialize pointer */
    inbufsize = INPBUFSIZ;              /* and size. */
#endif /* NOSPL */

#ifdef DCMDBUF
    if (cmsetup() < 0) fatal("Can't allocate command buffers!");

#ifndef NOSPL
    /* Allocate command stack allowing command parser to call itself */

    if (!(cmdstk = (struct cmdptr *) malloc(sizeof(struct cmdptr)*CMDSTKL)))
      fatal("cmdini: no memory for cmdstk");
    if (!(ifcmd = (int *) malloc(sizeof(int)*CMDSTKL)))
      fatal("cmdini: no memory for ifcmd");
    if (!(count = (int *) malloc(sizeof(int)*CMDSTKL)))
      fatal("cmdini: no memory for count");
    if (!(iftest = (int *) malloc(sizeof(int)*CMDSTKL)))
      fatal("cmdini: no memory for iftest");
    if (!(intime = (int *) malloc(sizeof(int)*CMDSTKL)))
      fatal("cmdini: no memory for intime");
    if (!(inpcas = (int *) malloc(sizeof(int)*CMDSTKL)))
      fatal("cmdini: no memory for inpcas");
    if (!(takerr = (int *) malloc(sizeof(int)*CMDSTKL)))
      fatal("cmdini: no memory for takerr");
    if (!(merror = (int *) malloc(sizeof(int)*CMDSTKL)))
      fatal("cmdini: no memory for merror");
    if (!(xquiet = (int *) malloc(sizeof(int)*CMDSTKL)))
      fatal("cmdini: no memory for xquiet");
    if (!(xvarev = (int *) malloc(sizeof(int)*CMDSTKL)))
      fatal("cmdini: no memory for xvarev");
    if (!kermrc)
      if (!(kermrc = (char *) malloc(KERMRCL+1)))
        fatal("cmdini: no memory for kermrc");
#ifdef CK_APC
/* Application Program Command buffer */
    if (!(apcbuf = malloc(APCBUFLEN + 1)))
        fatal("cmdini: no memory for apcbuf");
#endif /* CK_APC */
#endif /* NOSPL */

/* line[] and tmpbuf[] are the two string buffers used by the command parser */

    if (!(line = malloc(LINBUFSIZ + 1)))
        fatal("cmdini: no memory for line");
    if (!(tmpbuf = malloc(LINBUFSIZ + 1)))
        fatal("cmdini: no memory for tmpbuf");
#endif /* DCMDBUF */

#ifndef NOSPL
#ifdef CK_MINPUT
    {                                   /* Initialize MINPUT pointers */
        int i;
        extern char *ms[];
        for (i = 0; i < MINPMAX; i++)
          ms[i] = NULL;
    }
#endif /* CK_MINPUT */

    if (macini() < 0)                   /* Allocate macro buffers */
      fatal("Can't allocate macro buffers!");

    ifcmd[0] = 0;                       /* Command-level related variables. */
    iftest[0] = 0;                      /* Initialize variables at top level */
    count[0] = 0;                       /* of stack... */
    intime[0] = 0;
    inpcas[0] = 0;
    takerr[0] = 0;
    merror[0] = 0;
    xquiet[0] = quiet;
    xvarev[0] = vareval;
#endif /* NOSPL */

#ifndef NOSPL
    cmdlvl = 0;                         /* Initialize the command stack */
    xcmdsrc = CMD_KB;
    cmdstk[cmdlvl].src = CMD_KB;        /* Source is console */
    cmdstk[cmdlvl].lvl = 0;             /* Level is 0 */
    cmdstk[cmdlvl].ccflgs = 0;          /* No flags */
#endif /* NOSPL */

    tlevel = -1;                        /* Take file level = keyboard */
    for (i = 0; i < MAXTAKE; i++)       /* Initialize command file names */
      tfnam[i] = NULL;

    cmsetp(ckprompt);                   /* Set up C-Kermit's prompt */
                                        /* Can't set IKSD prompt here since */
                                        /* we do not yet know if we are IKSD */
#ifndef NOSPL

    initmac();                          /* Initialize macro table */

/* Predefine built-in one-line macros */

    addmac("ibm-linemode",m_ibm);       /* IBM-LINEMODE */
    addmac("fatal",m_fat);              /* FATAL macro */
    y = addmac("fast",m_fast);          /* FAST macro */
    addmac("cautious",m_cautious);      /* CAUTIOUS macro */
    addmac("robust",m_robust);          /* ROBUST macro */
#ifdef OS2
    addmac("manual",m_manual);          /* MANUAL macro */
#endif /* OS2 */
#ifdef VMS
    addmac("purge",m_purge);            /* PURGE macro */
#endif /* VMS */

/*
  Predefine built-in multiline macros; these are top-level commands
  that are implemented internally as macros.  NOTE: When adding a new
  one of these, remember to update the END and RETURN commands to
  account for it, or else END and RETURN from within it won't work right.
*/
    x = addmmac("_forx",for_def);       /* FOR macro */
    if (x > -1) mactab[x].flgs = CM_INV;
    x = addmmac("_forz",foz_def);       /* Other FOR macro */
    if (x > -1) mactab[x].flgs = CM_INV;
    x = addmmac("_xif",xif_def);        /* XIF macro */
    if (x > -1) mactab[x].flgs = CM_INV;
    x = addmmac("_while",whil_def);     /* WHILE macro */
    if (x > -1) mactab[x].flgs = CM_INV;
    x = addmmac("_switx",sw_def);       /* SWITCH macro */
    if (x > -1) mactab[x].flgs = CM_INV;

/* Fill in command-line argument vector */

    sprintf(vnambuf,"\\&@[%d]",xargs);  /* SAFE */
    if (inserver) {                     /* But hidden in IKSD */
        y = -1;
        xargs = 0;
    } else
      y = arraynam(vnambuf,&x,&z);      /* goes in array \&@[] */

    tmpbuf[0] = NUL;
    if (y > -1) {
        int j = -1;
        int yy = 0;
        dclarray((char)x,z);            /* Declare the array */
#ifndef NOTAKEARGS
        /* Macro argument vector */
        sprintf(vnambuf,"\\&_[%d]",z);  /* SAFE */
        yy = arraynam(vnambuf,&x,&z);   /* goes in array \&_[] */
        if (yy > -1)                    /* Name is OK */
          dclarray((char)x,z);          /* Declare the array */
#endif /* NOTAKEARGS */
        skip = 0;
        for (i = 0; i < xargs; i++) {   /* Fill the arrays */
            sprintf(vnambuf,"\\&@[%d]",i); /* SAFE */
            addmac(vnambuf,xargv[i]);
            if (cfilef && i == 0)
              continue;
#ifdef KERBANG
            if (skip) {
                j = 0;
                skip = 0;
                continue;
            }
#endif /* KERBANG */
            if (j < 0 &&                /* Assign items after "=" or "--"*/
                (!strcmp(xargv[i],"=") || !strcmp(xargv[i],"--"))
                ) {
                j = 0;                  /* to \%1..\%9 */
#ifdef KERBANG
            } else if (j < 0 &&
                       (!strcmp(xargv[i],"+") ||
                        !strncmp(xargv[i],"+ ",2) ||
                        !strncmp(xargv[i],"+\t",2))
                        ) {
                skip = 1;
                continue;
#endif /* KERBANG */
            } else if (j > -1) {
                j++;
                if (j <= 9) {
                    vnambuf[0] = '\\';
                    vnambuf[1] = '%';
                    vnambuf[2] = (char)(j+'0');
                    vnambuf[3] = NUL;
                    addmac(vnambuf,xargv[i]);
                }
                if (yy > -1) {
                    char c, * p;
                    int flag = 0;
                    p = xargv[i];
                    makestr(&(toparg[j]),p);
                    while ((c = *p++)) { if (c == SP) { flag++; break; } }
                    if (flag)
                      ckstrncat(tmpbuf,"\"",TMPBUFSIZ);
                    ckstrncat(tmpbuf,xargv[i],TMPBUFSIZ);
                    if (flag)
                      ckstrncat(tmpbuf,"\"",TMPBUFSIZ);
                    ckstrncat(tmpbuf," ",TMPBUFSIZ);
                }
            }
        }
        if (cfilef) {
            addmac("\\%0",cmdfil);
            if (yy > -1)
              makestr(&(toparg[0]),cmdfil);
        } else {
            addmac("\\%0",xargv[0]);
            if (yy > -1)
              makestr(&(toparg[0]),xargv[0]);
        }
        if (yy > -1) {
            topargc = (j < 0) ? 1 : j + 1;
            topxarg = toparg;
#ifdef COMMENT
            /* This needs work */
            if (!cfilef)
              makestr(&topline,tmpbuf);
#endif /* COMMENT */
        } else {
            topargc = 0;
            topxarg = NULL;
        }
        a_dim[0] = topargc - 1;
        a_ptr[0] = topxarg;
	debug(F111,"a_dim[0]","A",a_dim[0]);
    }
    *vnambuf = NUL;
#endif /* NOSPL */

    luinit();                           /* Initialize lookup() cache */

/* Get our home directory now.  This needed in lots of places. */

    cmdinited = 1;
}

#ifdef NT
_PROTOTYP(char * GetAppData,(int));
#endif /* NT */

VOID
doinit() {
#ifdef CKROOT
extern int ckrooterr;
#endif /* CKROOT */
    int x = 0, ok = 0;
#ifdef OS2
    char * ptr = 0;
#endif /* OS2 */

    if (!cmdinited)
      cmdini();

#ifdef MAC
    return;                             /* Mac Kermit has no init file */

#else /* !MAC */

/* If skipping init file ('-Y' on Kermit command line), return now. */

    if (noinit) {
        kermrc[0] = '\0';
        inidir[0] = '\0';
/*
  But returning from here results in inidir[] never being set to anything.
  Instead it should be set to wherever the init file *would* have been
  executed from.  So this bit of code should be removed, and then we should
  sprinkle "if (noinit)" tests throughout the following code until we have
  set inidir[], and then return without actually taking the init file.
*/
        return;
    }

#ifdef OS2
/*
  The -y init file must be fully specified or in the current directory.
  KERMRC is looked for via INIT, DPATH and PATH in that order.  Finally, our
  own executable file path is taken and the .EXE suffix is replaced by .INI
  and this is tried as the initialization file.
*/
#ifdef CK_LOGIN
    debug(F101,"doinit inserver","",inserver);
    debug(F101,"doinit isguest","",isguest);
    debug(F110,"doinit anonfile",anonfile,0);

    if (isguest && anonfile) {
      ckstrncpy(line, anonfile, LINBUFSIZ+1);
    } else
#endif /* CK_LOGIN */
      if (rcflag) {
          ckstrncpy(line,kermrc,LINBUFSIZ+1);
#ifdef CK_LOGIN
      } else if (inserver) {
          char * appdata = NULL;
#ifdef NT
          appdata = GetAppData(1);
          if ( appdata ) {
              ckmakmsg(line,LINBUFSIZ+1,appdata,
                        "Kermit 95/k95.ini",NULL,NULL);
              if ( zchki(line) < 0 )
                  line[0] = '\0';
          }
          if (line[0] == 0) {
              appdata = GetAppData(0);
              if ( appdata ) {
                  ckmakmsg(line,LINBUFSIZ+1,appdata,
                            "Kermit 95/k95.ini",NULL,NULL);
                  if ( zchki(line) < 0 )
                      line[0] = '\0';
              }
          }
#endif /* NT */
          if (line[0] == 0) {
              appdata = zhome();
              if ( appdata ) {
                  ckmakmsg(line,LINBUFSIZ+1,appdata,
#ifdef NT
                          "k95.ini",
#else /* NT */
                          "k2.ini",
#endif /* NT */
                           NULL,NULL);
                  if ( zchki(line) < 0 )
                      line[0] = '\0';
              }
          }
          debug(F110,"doinit inserver inifile",line,0);
#endif /* CK_LOGIN */
    } else {
        char * env = 0;
#ifdef NT
        env = getenv("K95.KSC");
#else
        env = getenv("K2.KSC");
#endif /* NT */
        if (!env) {
#ifdef NT
            env = getenv("K95.INI");
#else
            env = getenv("K2.INI");
#endif /* NT */
        }
        if (!env)
          env = getenv("CKERMIT.INI");
        if (!env)
          env = getenv("CKERMIT_INI");
        line[0] = '\0';

        debug(F110,"doinit env",env,0);
        if (env)
          ckstrncpy(line,env,LINBUFSIZ+1);

#ifdef NT
        if (line[0] == 0) {
            env = GetAppData(1);
            if ( env ) {
                ckmakmsg(line,LINBUFSIZ+1,env,"Kermit 95/k95.ini",NULL,NULL);
                if ( zchki(line) < 0 )
                    line[0] = '\0';
            }
        }
        if (line[0] == 0) {
            env = GetAppData(0);
            if ( env ) {
                ckmakmsg(line,LINBUFSIZ+1,env,"Kermit 95/k95.ini",NULL,NULL);
                if ( zchki(line) < 0 )
                    line[0] = '\0';
            }
        }
#endif /* NT */

        if (line[0] == 0) {
            env = zhome();
            if ( env ) {
                ckmakmsg(line,LINBUFSIZ+1,env,
#ifdef NT
                          "k95.ini",
#else /* NT */
                          "k2.ini",
#endif /* NT */
                          NULL,NULL);
                if ( zchki(line) < 0 )
                    line[0] = '\0';
            }
        }

        if (line[0] == 0)
          _searchenv(kermrc,"INIT",line);
        if (line[0] == 0)
          _searchenv(kermrc,"DPATH",line);
        if (line[0] == 0)
          _searchenv(kermrc,"PATH",line);
        if (line[0] == 0) {
            char *pgmptr = GetLoadPath();
            if (pgmptr && strlen(pgmptr) < LINBUFSIZ-8) {
                lp = strrchr(pgmptr, '\\');
                if (lp) {
                    strncpy(line, pgmptr, lp - pgmptr);
#ifdef NT
                    strcpy(line + (lp - pgmptr), "/k95.ini");
#else /* NT */
                    strcpy(line + (lp - pgmptr), "/k2.ini");
#endif /* NT */
                } else {
                    lp = strrchr(pgmptr, '.');
                    if (lp) {
                        strncpy(line, pgmptr, lp - pgmptr);
                        strcpy(line + (lp - pgmptr), ".ini");
                    }
                }
            }
        }
    }

#ifdef CKROOT
    if (!zinroot(line)) {
        debug(F110,"doinit setroot violation",line,0);
        return;
    }
#endif /* CKROOT */

    debug(F110,"doinit fopen()",line,0);
    if ((tfile[0] = fopen(line,"r")) != NULL) {
        ok = 1;
        tlevel = 0;
        tfline[tlevel] = 0;
        if (tfnam[tlevel] = malloc(strlen(line)+1))
          strcpy(tfnam[tlevel],line);   /* safe */
#ifndef NOSPL
        cmdlvl++;
        xcmdsrc = CMD_TF;
        cmdstk[cmdlvl].src = CMD_TF;
        cmdstk[cmdlvl].lvl = tlevel;
        cmdstk[cmdlvl].ccflgs = 0;
        ifcmd[cmdlvl] = 0;
        iftest[cmdlvl] = 0;
        count[cmdlvl] =  count[cmdlvl-1]; /* Inherit from previous level */
        intime[cmdlvl] = intime[cmdlvl-1];
        inpcas[cmdlvl] = inpcas[cmdlvl-1];
        takerr[cmdlvl] = takerr[cmdlvl-1];
        merror[cmdlvl] = merror[cmdlvl-1];
        xquiet[cmdlvl] = quiet;
        xvarev[cmdlvl] = vareval;
#endif /* NOSPL */
        debug(F110,"doinit init file",line,0);
    } else {
        debug(F100,"doinit no init file","",0);
    }
    ckstrncpy(kermrc,line,KERMRCL);
    for (ptr = kermrc; *ptr; ptr++)     /* Convert backslashes to slashes */
       if (*ptr == '\\')
         *ptr = '/';
#else /* not OS2 */
    lp = line;
    lp[0] = '\0';
    debug(F101,"doinit rcflag","",rcflag);
#ifdef GEMDOS
    zkermini(line, rcflag, kermrc);
#else
#ifdef VMS
    {
	int x;
	x = zkermini(line,LINBUFSIZ,kermrc);
	debug(F111,"CUSTOM zkermini",line,x);
	if (x == 0)
	  line[0] = NUL;
    }
#else /* not VMS */
#ifdef CK_LOGIN
    debug(F101,"doinit isguest","",isguest);
    if (isguest)
      ckstrncpy(lp, anonfile ? anonfile : kermrc, LINBUFSIZ);
    else
#endif /* CK_LOGIN */
      if (rcflag) {                     /* If init file name from cmd line */
          ckstrncpy(lp,kermrc,LINBUFSIZ); /* use it, */
      } else {                          /* otherwise... */
#ifdef CK_INI_A                         /* If we've a system-wide init file */
          /* And it takes precedence over the user's... */
          ckstrncpy(lp,CK_SYSINI,KERMRCL); /* Use it */
          if (zchki(lp) < 0) {          /* (if it exists...) */
#endif /* CK_INI_A */
              char * homdir;
              char * env = 0;
              line[0] = NUL;

              /* Add support for environment variable */
              env = getenv("CKERMIT.INI");
              if (!env)
                env = getenv("CKERMIT_INI");
              if (env)
                ckstrncpy(lp,env,KERMRCL);

              if (lp[0] == 0) {
                  homdir = zhome();
                  if (homdir) {         /* Home directory for init file. */
                      ckstrncpy(lp,homdir,KERMRCL);
#ifdef STRATUS
                      ckstrncat(lp,">",KERMRCL);/* VOS dirsep */
#else
                      if (lp[0] == '/') ckstrncat(lp,"/",KERMRCL);
#endif /* STRATUS */
                  }
                  ckstrncat(lp,kermrc,KERMRCL);/* Append default file name */
              }
#ifdef CK_INI_A
          }
#endif /* CK_INI_A */
#ifdef CK_INI_B                         /* System-wide init defined? */
          /* But user's ini file takes precedence */
          if (zchki(lp) < 0)            /* If user doesn't have her own, */
            ckstrncpy(lp,CK_SYSINI,KERMRCL); /* use system-wide one. */
#endif /* CK_INI_B */
      }
#endif /* VMS */
#endif /* GEMDOS */

#ifdef AMIGA
    reqoff();                           /* Disable requestors */
#endif /* AMIGA */

#ifdef USE_CUSTOM
    /* If no init file was found, execute the customization file */
    debug(F111,"CUSTOM 1",line,rcflag);
    if ((!line[0] || zchki(line) < 0) && !rcflag) {
	int x;
#ifdef OS2
	x = ckmakestr(line,LINBUFSIZ,GetAppData(1),"/","K95CUSTOM.INI",NULL);
	debug(F111,"CUSTOM 2",line,x);
	if (zchki(line) < 0) {
	    x = ckmakestr(line,LINBUFSIZ,GetAppData(0),"/","K95USER.INI",NULL);
	    debug(F111,"CUSTOM 3",line,x);
	}
#else  /* OS2 */
	x = ckstrncpy(line,zhome(),LINBUFSIZ);
#ifndef VMS
	/* VMS zhome() returns "SYS$LOGIN:" */
	if (line[x-1] != DIRSEP) {
	    line[x++] = DIRSEP;
	    line[x] = NUL;
	}
#endif /* VMS */
	x = ckstrncat(line,MYCUSTOM,LINBUFSIZ);
	debug(F111,"CUSTOM 4",line,x);
#endif /* OS2 */
    }
    debug(F110,"CUSTOM 5",line,0);
#endif /* USE_CUSTOM */

#ifdef CKROOT
    if (!zinroot(line)) {
        debug(F110,"doinit setroot violation",line,0);
        return;
    }
#endif /* CKROOT */

    debug(F110,"doinit ini file is",line,0);
    if ((tfile[0] = fopen(line,"r")) != NULL) { /* Try to open init file. */
        ok = 1;
        tlevel = 0;
        tfline[tlevel] = 0;
        if ((tfnam[tlevel] = malloc(strlen(line)+1)))
          strcpy(tfnam[tlevel],line);   /* safe */

        ckstrncpy(kermrc,line,KERMRCL);

#ifndef NOSPL
        cmdlvl++;
        ifcmd[cmdlvl] = 0;
        iftest[cmdlvl] = 0;
        count[cmdlvl] =  count[cmdlvl-1]; /* Inherit from previous level */
        intime[cmdlvl] = intime[cmdlvl-1];
        inpcas[cmdlvl] = inpcas[cmdlvl-1];
        takerr[cmdlvl] = takerr[cmdlvl-1];
        merror[cmdlvl] = merror[cmdlvl-1];
        xquiet[cmdlvl] = quiet;
        xvarev[cmdlvl] = vareval;
        debug(F101,"doinit open ok","",cmdlvl);
        xcmdsrc = CMD_TF;
        cmdstk[cmdlvl].src = CMD_TF;
        cmdstk[cmdlvl].lvl = tlevel;
        cmdstk[cmdlvl].ccflgs = 0;
#endif /* NOSPL */
    } else if (rcflag) {
        /* Print an error message only if a specific file was asked for. */
        printf("?%s - %s\n", ck_errstr(), line);
    }

#ifdef datageneral
/* If CKERMIT.INI not found in home directory, look in searchlist */
    if (/* homdir && */ (tlevel < 0)) {
        ckstrncpy(lp,kermrc,LINBUFSIZ);
        if ((tfile[0] = fopen(line,"r")) != NULL) {
            ok = 1;
            tlevel = 0;
            tfline[tlevel] = 0;
            if (tfnam[tlevel] = malloc(strlen(line)+1))
              strcpy(tfnam[tlevel],line); /* safe */
#ifndef NOSPL
            cmdlvl++;
            xcmdsrc = CMD_TF;
            cmdstk[cmdlvl].src = CMD_TF;
            cmdstk[cmdlvl].lvl = tlevel;
            cmdstk[cmdlvl].ccflgs = 0;
            ifcmd[cmdlvl] = 0;
            iftest[cmdlvl] = 0;
            count[cmdlvl] =  count[cmdlvl-1]; /* Inherit from previous level */
            intime[cmdlvl] = intime[cmdlvl-1];
            inpcas[cmdlvl] = inpcas[cmdlvl-1];
            takerr[cmdlvl] = takerr[cmdlvl-1];
            merror[cmdlvl] = merror[cmdlvl-1];
            xquiet[cmdlvl] = quiet;
	    xvarev[cmdlvl] = vareval;
#endif /* NOSPL */
        }
    }
#endif /* datageneral */

#ifdef AMIGA                            /* Amiga... */
    reqpop();                           /* Restore requestors */
#endif /* AMIGA */
#endif /* OS2 */
#endif /* MAC */

    /* Assign value to inidir */

    if (!ok) {
        inidir[0] = NUL;
    } else {
        ckstrncpy(inidir, kermrc, CCHMAXPATH);
        x = strlen(inidir);
        if (x > 0) {
            int i;
            for (i = x - 1; i > 0; i-- ) {
                if (ISDIRSEP(inidir[i])) {
                    inidir[i+1] = NUL;
                    break;
                }
            }
        }
#ifdef NT
        GetShortPathName(inidir,inidir,CCHMAXPATH);
#endif /* NT */
    }
}

VOID
doiksdinit() {
#ifdef CK_SSL
    /* IKSD doesn't request client certs */
    ssl_verify_flag = SSL_VERIFY_NONE;
#endif /* CK_SSL */

    if (!cmdinited)
      cmdini();

#ifdef IKSDCONF
#ifdef OS2
    line[0] = '\0';
    _searchenv(iksdconf,"INIT",line);
    if (line[0] == 0)
      _searchenv(iksdconf,"DPATH",line);
    if (line[0] == 0)
      _searchenv(iksdconf,"PATH",line);
    if (line[0] == 0) {
        char *pgmptr = GetLoadPath();
        if (pgmptr && strlen(pgmptr) < LINBUFSIZ-8) {
            lp = strrchr(pgmptr, '\\');
            if (lp) {
                strncpy(line, pgmptr, lp - pgmptr);
                strcpy(line + (lp - pgmptr), "\\");
                strcpy(line + (lp - pgmptr + 1), iksdconf);
            } else {
                lp = strrchr(pgmptr, '.');
                if (lp) {
                    strncpy(line, pgmptr, lp - pgmptr);
                    strcpy(line + (lp - pgmptr), ".ksc");
                }
            }
        }
    }
    debug(F110,"doiksdinit() line",line,0);
    tfile[0] = fopen(line,"r");
#else /* OS2 */
    tfile[0] = fopen(iksdconf,"r");
#endif /* OS2 */
    if (tfile[0] != NULL) {
        tlevel = 0;
        tfline[tlevel] = 0;
#ifdef OS2
        if (tfnam[tlevel] = malloc(strlen(line)+1))
          strcpy(tfnam[tlevel],line);
#else /* OS2 */
        if ((tfnam[tlevel] = malloc(strlen(iksdconf)+1)))
          strcpy(tfnam[tlevel],iksdconf);
#endif /* OS2 */
#ifndef NOSPL
        cmdlvl++;
        xcmdsrc = CMD_TF;
        cmdstk[cmdlvl].src = CMD_TF;
        cmdstk[cmdlvl].lvl = tlevel;
        cmdstk[cmdlvl].ccflgs = 0;
        ifcmd[cmdlvl]  = 0;
        iftest[cmdlvl] = 0;
        count[cmdlvl]  = count[cmdlvl-1]; /* Inherit from previous level */
        intime[cmdlvl] = intime[cmdlvl-1];
        inpcas[cmdlvl] = inpcas[cmdlvl-1];
        takerr[cmdlvl] = takerr[cmdlvl-1];
        merror[cmdlvl] = merror[cmdlvl-1];
        xquiet[cmdlvl] = quiet;
	xvarev[cmdlvl] = vareval;
#endif /* NOSPL */
        debug(F110,"doiksdinit file ok",tfnam[tlevel],0);
    } else {
        debug(F110,"doiksdinit open failed",tfnam[tlevel],0);
    }
#endif /* IKSDCONF */
}

#ifndef NOSPL
/*
  G E T N C M

  Get next command from current macro definition.  Command is copied
  into string pointed to by argument s, max length n.   Returns:
   0 if a string was copied;
  -1 if there was no string to copy.
*/
int
getncm(s,n) char *s; int n; {
    int y = 0;				/* Character counter */
    int quote = 0;
    int kp = 0;				/* Brace up-down counter */
    int pp = 0;				/* Parenthesis up-down counter */
#ifndef NODQMACRO
    int dq = 0;				/* Doublequote counter */
#endif /* NODQMACRO */
    char *s2;                           /* Copy of destination pointer */

    s2 = s;                             /* Initialize string pointers */
    *s = NUL;                           /* and destination buffer */

    /* debug(F010,"getncm entry",macp[maclvl],0); */

    for (y = 0;                         /* Loop for n bytes max */
         macp[maclvl] && *macp[maclvl] && y < n;
         y++, s++, macp[maclvl]++) {

        *s = *macp[maclvl];             /* Get next char from macro def */

#ifndef COMMENT
/*
  This is to allow quoting of parentheses, commas, etc, in function
  arguments, but it breaks just about everything else.  DON'T REMOVE THIS
  COMMENT!  (Otherwise you'll wind up adding the same code again and breaking
  everything again.)  <-- The preceding warning should be obsolete since the
  statements below have been fixed, but in case of fire, remove the "n" from
  the <#>ifndef above.  NEW WARNING: code added 12 Apr 2002 to exempt the
  opening brace in \{nnn} from being treated as a quoted brace.
*/
        if (!quote && *s == CMDQ) {
            quote = 1;
            continue;
        }
        if (quote) {
	    int notquote = 0;
            quote = 0;
	    if (*s == '{') {		/* Check for \{nnn} (8.0.203) */
		char c, * p;
		p = macp[maclvl] + 1;
		while ((c = *p++)) {
		    if (isdigit(c))
		      continue;
		    else if (c == '}') {
			notquote++;
			break;
		    } else {
			break;
		    }
		}
	    }
	    if (notquote == 0)
	      continue;
        }
#endif /* COMMENT */

/*
  Allow braces around macro definition to prevent commas from being turned to
  end-of-lines and also treat any commas within parens as text so that
  multiple-argument functions won't cause the command to break prematurely.
  19 Oct 2001: Similar treatment was added for doublequotes, so

     define foo { echo "one, two, three" }

  would work as expected.  This doesn't seem to have broken anything but
  if something comes up later, rebuild with NODQMACRO defined.
*/
        if (*s == '{') kp++;            /* Count braces */
        if (*s == '}' && kp > 0) kp--;
        if (*s == '(') pp++;            /* Count parentheses. */
        if (*s == ')' && pp > 0) pp--;
#ifndef NODQMACRO
#ifndef COMMENT
	/* Too many false positives */
	/* No, not really -- this is indeed the best we can do */
	/* Reverted to this method Sun May 11 18:43:45 2003 */
	if (*s == '"') dq = 1 - dq;     /* Account for doublequotes */
#else  /* Fri Apr  4 13:21:29 2003 */
	/* The code below breaks the SWITCH statement */
	/* There is no way to make this work -- it would require */
	/* building in all the knowledge of command parser. */
        if (dblquo && (*s == '"')) {    /* Have doublequote */
            if (dq == 1) {		/* Close quote only if... */
                if ((*(macp[maclvl]+1) == SP) || /* followed by space or... */
		    (!*(macp[maclvl]+1)) ||      /* at end or ... */
		    /* Next char is command separator... */
		    /* Sun May 11 17:24:12 2003 */
		    (kp < 1 && pp < 1 && (*(macp[maclvl]+1) == ','))
		    )		     
                  dq = 0;		/* Close the quote */
            } else if (dq == 0) {
                /* Open quote only if at beginning or preceded by space */
                if (s > s2) {
                    if (*(s-1) == SP)
                      dq = 1;
                } else if (s == s2) {
                      dq = 1;
                }
            }
        }
#endif /* COMMENT */
#endif /* NODQMACRO */
        if (*s == ',' && pp <= 0 && kp <= 0
#ifndef NODQMACRO
            && dq == 0
#endif /* NODQMACRO */
            ) {
            macp[maclvl]++;             /* Comma not in {} or () */
            /* debug(F110,"next cmd",s,0); */
            kp = pp = 0;                /* so we have the next command */
            break;
        }
    }                                   /* Reached end. */
#ifdef COMMENT
    /* DON'T DO THIS - IT BREAKS EVERYTHING */
    *s = NUL;
#endif /* COMMENT */
    if (*s2 == NUL) {                   /* If nothing was copied, */
        /* debug(F100,"XXX getncm eom","",0); */
        popclvl();                      /* pop command level. */
        return(-1);
    } else {                            /* otherwise, tack CR onto end */
        *s++ = CR;
        *s = '\0';
        /* debug(F110,"XXX getncm OK",s,0); */
        if (mecho && pflag)             /* If MACRO ECHO ON, echo the cmd */
          printf("%s\n",s2);
    }
    return(0);
}

/*  D O M A C  --  Define and then execute a macro */

int
domac(name, def, flags) char *name, *def; int flags; {
    int x, m;
#ifndef NOLOCAL
#ifdef OS2
    extern int term_io;
    int term_io_sav = term_io;
    term_io = 0;                        /* Disable Terminal Emulator I/O */
#endif /* OS2 */
#endif /* NOLOCAL */
    m = maclvl;                         /* Current macro stack level */
    x = addmac(name, def);              /* Define a new macro */
    if (x > -1) {                       /* If successful, */
        dodo(x,NULL,flags);             /* start it (increments maclvl). */
        while (maclvl > m) {            /* Keep going till done with it, */
            debug(F101,"domac loop maclvl 1","",maclvl);
            sstate = (CHAR) parser(1);  /* parsing & executing each command, */
            debug(F101,"domac loop maclvl 2","",maclvl);
            if (sstate) proto();        /* including protocol commands. */
        }
        debug(F101,"domac loop exit maclvl","",maclvl);
    }
#ifndef NOLOCAL
#ifdef OS2
    term_io = term_io_sav;
#endif /* OS2 */
#endif /* NOLOCAL */
    return(success);
}
#endif /* NOSPL */

/*
  G E T N C T

  Get next command from TAKE (command) file.

  Call with:
   s     Pointer to buffer to read into
   n     Length of buffer
   f     File descriptor of file to read from
   flag  0 == keep line terminator on and allow continuation
         1 == discard line terminator and don't allow continuation

  Call with flag == 0 to read a command from a TAKE file;
  Call with flag != 0 to read a line from a dialing or network directory.

  In both cases, trailing comments and/or trailing whitespace is/are stripped.
  If flag == 0, continued lines are combined into one line.  A continued line
  is one that ends in hypen, or any line in a "block", which starts with "{"
  at the end of a line and ends with a matching "}" at the beginning of a
  subsequent line; blocks may be nested.

  Returns:
   0 if a string was copied,
  -1 on EOF,
  -2 on malloc failure
  -3 if line is not properly terminated
  -4 if (possibly continued) line is too long.
*/
static int lpxlen = 0;

int
getnct(s,n,f,flag) char *s; int n; FILE *f; int flag; {
    int i = 0, len = 0, buflen = 0;
    char c = NUL, cc = NUL, ccl = NUL, ccx = NUL, *s2 = NULL;
    char *lp = NULL, *lpx = NULL, *lp2 = NULL, *lp3 = NULL, *lastcomma = NULL;
    char * prev = NULL;
    int bc = 0;                         /* Block counter */

    s2 = s;                             /* Remember original pointer */
    prev = s2;				/* Here too */
    buflen = n;                         /* Remember original buffer length */

    if (n < 0)
      return(-2);

    /* Allocate a line buffer only if we don't have one that's big enough */

    debug(F111,"getnct",ckitoa(lpxlen),n);

    if (lpx && (n > lpxlen)) {          /* Have one already */
        debug(F101,"getnct new buffer","",lpxlen);
        free(lpx);                      /* But it's not big enough */
        lpx = NULL;                     /* Free current one */
        lpxlen = 0;
    }
    if (!lpx) {                         /* Get new one */
        if (!(lpx = (char *) malloc(n))) {
            debug(F101,"getnct malloc failure","",0);
            printf("?Memory allocation failure [getnct:%d]\n",n);
            return(-2);
        }
        lpxlen = n;
    }
    lp2 = lpx;
#ifdef KLUDGE
    /* NOTE: No longer used as of 14 Aug 2000 */
    lp2++;
#endif /* KLUDGE */

    while (1) {                         /* Loop to read lines from file */
        debug(F101,"getnct while (1)","",n);
        if (fgets(lp2,n,f) == NULL) {   /* Read a line into lp2 */
            debug(F110,"getnct EOF",s2,0); /* EOF */
            free(lpx);                  /* Free temporary storage */
            lpx = NULL;
            *s = NUL;                   /* Make destination be empty */
            return(-1);                 /* Return failure code */
        }

#ifndef NODIAL
        if (flag)                       /* Count this line */
          dirline++;
        else
#endif /* NODIAL */
          tfline[tlevel]++;
        len = strlen(lp2) - 1;          /* Position of line terminator */
        if (len == 0 && lp2[0] != '\n') { /* Last line in file has one char */
            lp2[++len] = '\n';          /* that is not a newline */
            lp2[len] = NUL;
        }
        debug(F010,"getnct",lp2,0);
        if (len < 0)
          len = 0;
        if (techo && pflag)             /* If TAKE ECHO ON, */
          printf("%3d. %s",             /* echo it this line. */
#ifndef NODIAL
                 flag ? dirline :
#endif /* NODIAL */
                 tfline[tlevel],
                 lp2
                 );
        lp3 = lp2;                      /* Working pointer */
        i = len;                        /* Get first nonwhitespace character */
        while (i > 0 && (*lp3 == SP || *lp3 == HT)) {
            i--;
            lp3++;
        }
        if (i == 0 && bc > 0)           /* Blank line in {...} block */
          continue;

        /* Isolate, remove, and check terminator */

        c = lp2[len];                   /* Value of line terminator */
        /* debug(F101,"getnct terminator","",c); */
        if (c < LF || c > CR) {         /* It's not a terminator */
            /* debug(F111,"getnct bad line",lp2,c); */
            if (feof(f) && len > 0 && len < n) {
                /* Kludge Alert... */
                if (!quiet)
                  printf("WARNING: Last line of %s lacks terminator\n",
                         s2 == cmdbuf ? "command file" : "directory file");
                c = lp2[++len] = '\n';  /* No big deal - supply one. */
            } else {                    /* Something's wrong, fail. */
                free(lpx);
                lpx = NULL;
                return(-3);
            }
        }
        /* Trim trailing whitespace */

        for (i = len - 1; i > -1 && lp2[i] <= SP; i--) /* Trim */
          ;
        /* debug(F101,"getnct i","",i); */
        lp2[i+1] = NUL;                 /* Terminate the string */
        /* debug(F110,"getnct lp2",lp2,0); */
        lp = lp2;                       /* Make a working pointer */

        /* Remove trailing or full-line comment */

        while ((cc = *lp)) {
            if (cc == ';' || cc == '#') { /* Comment introducer? */
                if (lp == lp2) {        /* First char on line */
                    *lp = NUL;
                    break;
                } else if (*(lp - 1) == SP || *(lp - 1) == HT) {
                    lp--;
                    *lp = NUL;  /* Or preceded by whitespace */
                    break;
                }
            }
            lp++;
        }
        if (lp > lp2)
          lp--;                         /* Back up over the NUL */

        /* Now trim any space that preceded the comment */

        while ((*lp == SP || *lp == HT) && lp >= lp2) {
            *lp = NUL;
            if (lp <= lp2)
              break;
            lp--;
        }
        /* debug(F110,"getnct comment trimmed",lp2,0); */

        len = strlen(lp2);              /* Length after trimming */

        if (n - len < 2) {              /* Check remaining space */
            debug(F111,"getnct command too long",s2,buflen);
            printf("?Line too long, maximum length: %d.\n",buflen);
            free(lpx);
            return(-4);
        }
        ccl = (len > 0) ? lp2[len-1] : 0;     /* Last character in line */
        ccx = (len > 1) ? lp2[len-2] : 0;     /* Penultimate char in line */

#ifdef COMMENT
        /* Line containing only whitespace and ,- */
        if ((len > 1) && (lp3 == lp2+len-2) && (ccl == '-') && (ccx == ','))
          continue;
#endif /* COMMENT */

#ifdef KLUDGE
/*
  If it is a command and it begins with a token (like ! or .) that is not
  followed by a space, insert a space now; otherwise cmkey() can get mighty
  confused.
*/
        if (s == s2 && !flag) {
            char *p = toktab;
            while (*p) {
                if (*p == *lp3 && *(p+1) != SP) {
                    debug(F110,"getnct token",p,0);
                    *lp3-- = SP;
                    *lp3 = *p;
                    if (lp3 < lp2) {
                        lp2--;
                        len++;
                    }
                    break;
                } else
                  p++;
            }
        }
#endif /* KLUDGE */
        lp = lp2;

        while ((*s++ = *lp++))          /* Copy result to target buffer */
          n--;                          /* accounting for length */
        s--;                            /* Back up over the NUL */

        /* Check whether this line is continued */

        if (flag)                       /* No line continuation when flag=1 */
          break;                        /* So break out of read-lines loop */

#ifdef COMMENT
        debug(F000,"getnct first char","",*lp3);
        debug(F000,"getnct last char","",ccl);
        debug(F000,"getnct next-to-last char","",ccx);
#endif /* COMMENT */

        if (bc > 0 && *lp3 == '}') {    /* First char on line is '}' */
            bc--;			/* Decrement block counter */
        }

        if (bc == 0 &&                  /* Line is continued if bc > 0 */
#ifdef COMMENT
            /* Not supported as of C-Kermit 6.0 */
            ccl != CMDQ &&              /* or line ends with CMDQ */
#endif /* COMMENT */
            ccl != '-'  &&              /* or line ends with dash */
            ccl != '{')                 /* or line ends with opening brace */
          break;                        /* None of those, we're done. */

        if (ccl == '-' || ccl == '{')   /* Continuation character */
          if (ccx == CMDQ)              /* But it's quoted */
            break;                      /* so ignore it */

        if (ccl == '{') {               /* Last char on line is '{'? */
            bc++;                       /* Count the block opener. */
        } else if (ccl == '-') {        /* Explicit continue? */
            char c, * ss;
            int state = 0, nn;
            s--;                        /* Yes, back up over terminators */
            n++;                        /* and over continuation character */
            nn = n;                     /* Save current count */
            ss = s;                     /* and pointer */
            s--;                        /* Back up over dash */
            n++;
            while (state < 2 && s >= prev) { /* Check for "{,-" */
                n++;
                c = *s--;
                if (c <= SP)
                  continue;
                if (c != ',' && c != '{')
                  break;
                switch (state) {
                  case 0:               /* Looking for comma */
                    if (c == ',')
                      state = 1;
                    break;
                  case 1:               /* Looking for left brace */
                    if (c == '{') {
                        state = 2;
                        s += 2;
                        *s = NUL;
                        bc++;
                    }
                    break;
                }
            }
            if (state != 2) { s = ss; n = nn; }
            *s = NUL;
        } else {                        /* None of those but (bc > 0) */
            lastcomma = s;
            *s++ = ',';                 /* and insert a comma */
            n--;
        }
#ifdef COMMENT
        debug(F101,"getnct bc","",bc);
        debug(F100,"getnct continued","",0);
#endif /* COMMENT */

        *s = NUL;
        prev = s;

    } /* read-lines while loop */

    if (lastcomma)
      *lastcomma = SP;
    if (!flag)                          /* Tack line terminator back on */
      *s++ = c;
    *s++ = NUL;                         /* Terminate the string */
    untab(s2);                          /* Done, convert tabs to spaces */
#ifdef DEBUG
    if (!flag) {
        debug(F010,"CMD(F)",s2,0);
    }
#endif /* DEBUG */
    free(lpx);                          /* Free temporary storage */
    return(0);                          /* Return success */
}

VOID
shostack() {                            /* Dump the command stack */
    int i;
    char *p;
#ifndef NOSPL
    for (i = cmdlvl; i > 0; i--) {
        if (cmdstk[i].src == CMD_TF) {
            p = tfnam[cmdstk[i].lvl];
            if (zfnqfp(p,TMPBUFSIZ,tmpbuf))
              p = tmpbuf;
            printf(" %2d. File  : %s (line %d)\n",
                   i,
                   p,
                   tfline[cmdstk[i].lvl]
                   );
        } else if (cmdstk[i].src == CMD_MD) {
            char * m;
            m = m_arg[cmdstk[i].lvl][0]; /* Name of this macro */
            if (i > 0) {                 /* Special handling for 2-level */
                char *s;                 /* built-in macros... */
                s = m_arg[cmdstk[i-1].lvl][0]; /* Name next level up */
                if (s && cmdstk[i-1].src == CMD_MD) {
                    if (!strcmp(s,"_forx"))
                      m = "FOR";
                    else if (!strcmp(s,"_xif"))
                      m = "XIF";
                    else if (!strcmp(s,"_while"))
                      m = "WHILE";
                    else if (!strcmp(s,"_switx"))
                      m = "SWITCH";
                }
            }
            printf(" %2d. Macro : %s\n",i,m);
        } else if (cmdstk[i].src == CMD_KB) {
            printf(" %2d. Prompt:\n",i);
        } else {
            printf(" %2d. ERROR : Command source unknown\n",i);
        }
    }
#else
    for (i = tlevel; i > -1; i--) {
        p = tfnam[i];
        if (zfnqfp(p,TMPBUFSIZ,tmpbuf))
          p = tmpbuf;
        printf(" %2d. File  : %s (line %d)\n",
               i,
               p,
               tfline[i]
               );
    }
#endif /* NOSPL */
    if (i == 0)
      printf(" %2d. Prompt: (top level)\n",0);
}

/* For command error messages - avoid dumping out the contents of some */
/* some huge FOR loop if it contains a syntax error. */

static char *
cmddisplay(s, cx) char * s; int cx; {
    static char buf[80];
    if ((int)strlen(s) > 70) {
	sprintf(buf,"%.64s...",s);	/* SAFE */
	s = buf;
    }
    return(s);
}

static VOID
cmderr() {
    if (xcmdsrc > 0) {
	switch (cmd_err) {		/* SET COMMAND ERROR-DISPLAY */
	  case 0:
	    break;
	  case 1:
	  case 2:
	    if (tlevel > -1) {
#ifndef NOSPL
		if (xcmdsrc == 2)
		  printf(
"In macro or block defined in file: %s starting about line %d\n",
			 tfnam[tlevel] ? tfnam[tlevel] : "", tfline[tlevel]
			 );
		else
#endif /* NOSPL */
		  printf("File: %s, Line: %d\n",
			 tfnam[tlevel] ? tfnam[tlevel] : "", tfline[tlevel]
			 );
	    }
#ifndef NOSPL
	    if (cmd_err == 2) {
		if (cmdstk[cmdlvl].src == CMD_MD) { /* Executing a macro? */
		    int m;
		    m = cmdstk[cmdlvl].lvl;
		    if (mlook(mactab,m_arg[m][0],nmac) >= 0)
		      printf("Macro name: %s\n", m_arg[m][0]);
		}
	    }
#endif /* NOSPL */
	    break;

	  case 3:
	    printf("Command stack:\n");
	    shostack();			    
	}
    }
}

/*  P A R S E R  --  Top-level interactive command parser.  */

/*
  Call with:
    m = 0 for normal behavior: keep parsing and executing commands
          until an action command is parsed, then return with a
          Kermit start-state as the value of this function.
    m = 1 to parse only one command, can also be used to call parser()
          recursively.
    m = 2 to read but do not execute one command.
  In all cases, parser() returns:
    0     if no Kermit protocol action required
    > 0   with a Kermit protocol start-state.
    < 0   upon error.
*/
int
parser(m) int m; {
    int tfcode, xx, yy, zz;             /* Workers */
    int is_tn = 0;
    int cdlost = 0;

#ifndef NOSPL
    int inlevel;                        /* Level we were called at */
    extern int askflag, echostars;
#endif /* NOSPL */
    char *cbp;                          /* Command buffer pointer */
#ifdef MAC
    extern char *lfiles;                /* Fake extern cast */
#endif /* MAC */
    extern int interrupted;
#ifndef NOXFER
    extern int sndcmd, getcmd, fatalio, clearrq;
#endif /* NOXFER */

#ifdef AMIGA
    reqres();                           /* Restore AmigaDOS requestors */
#endif /* AMIGA */

#ifdef OS2
    if (cursor_save > -1) {             /* Restore cursor if it was */
        cursorena[VCMD] = cursor_save;  /* turned off during file transfer */
        cursor_save = -1;
    }
#endif /* OS2 */

#ifdef IKSDB
    if (ikdbopen) slotstate(what,"COMMAND PROMPT","",""); /* IKSD database */
#endif /* IKSDB */

    is_tn = (local && network && IS_TELNET()) ||
      (!local && sstelnet);

    if (!xcmdsrc)                       /* If at top (interactive) level ... */
      concb((char)escape);              /* put console in 'cbreak' mode. */

#ifdef CK_TMPDIR
/* If we were cd'd temporarily to another device or directory ... */
    if (f_tmpdir) {
        int x;
        x = zchdir((char *) savdir);    /* ... restore previous directory */
        f_tmpdir = 0;                   /* and remember we did it. */
        debug(F111,"parser tmpdir restoring",savdir,x);
    }
#endif /* CK_TMPDIR */

#ifndef NOSPL
    inlevel = cmdlvl;           /* Current macro level */
#ifdef DEBUG
    if (deblog) {
        debug(F101,"&parser entry maclvl","",maclvl);
        debug(F101,"&parser entry inlevel","",inlevel);
        debug(F101,"&parser entry tlevel","",tlevel);
        debug(F101,"&parser entry cmdlvl","",cmdlvl);
        debug(F101,"&parser entry m","",m);
    }
#endif /* DEBUG */
#endif /* NOSPL */

#ifndef NOXFER
    ftreset();                          /* Reset global file settings */
#endif /* NOXFER */
/*
  sstate becomes nonzero when a command has been parsed that requires some
  action from the protocol module.  Any non-protocol actions, such as local
  directory listing or terminal emulation, are invoked directly from below.
*/
    sstate = 0;                         /* Start with no start state. */

#ifndef NOXFER
#ifndef NOSPL
    query = 0;                          /* QUERY not active */
#endif /* NOSPL */
#ifndef NOHINTS
    if (!success) {
        if (local && !network && carrier != CAR_OFF) {
            int x;                      /* Serial connection */
            x = ttgmdm();               /* with carrier checking */
            if (x > -1) {
                if (!(x & BM_DCD)) {
                    cdlost = 1;
                    fatalio = 1;
                }
            }
        }
    }
#ifdef DEBUG
    if (deblog) {
        debug(F101,"parser top what","",what);
        debug(F101,"parser top interrupted","",interrupted);
        debug(F101,"parser top cdlost","",cdlost);
        debug(F101,"parser top sndcmd","",sndcmd);
        debug(F101,"parser top getcmd","",getcmd);
    }
#endif /* DEBUG */
    if (cdlost && !interrupted && (sndcmd || getcmd)) {
        printf("?Connection broken (carrier signal lost)\n");
    }
    if (sndcmd && protocol == PROTO_K &&
	!success && hints && !interrupted && !fatalio && !xcmdsrc) {
        int x = 0, n = 0;
        printf("\n*************************\n");
        printf("SEND-class command failed.\n");
        printf(" Packets sent: %d\n", spackets);
        printf(" Retransmissions: %d\n",retrans);
        printf(" Timeouts: %d\n", timeouts);
        printf(" Damaged packets: %d\n", crunched);
        if (epktrcvd) {
            printf(" Transfer canceled by receiver.\n");
            printf(" Receiver's message: \"%s\"\n",(char *)epktmsg);
            n++;
        } else {
            if (epktmsg) if (*epktmsg) {
                printf(" Fatal Kermit Protocol Error: %s\n",epktmsg);
                n++;
            }
        }
        if (local && !network && carrier != CAR_OFF) {
            int xx;                     /* Serial connection */
            xx = ttgmdm();              /* with carrier checking */
            if (xx > -1) {
                if (!(xx & BM_DCD))
                  cdlost = 1;
            }
        }

#ifdef UNIX
        if (errno != 0)
#endif /* UNIX */
          {
              printf(" Most recent local OS error: \"%s\"\n",ck_errstr());
              n++;
          }
        printf(
   "\nHINTS... If the preceding error message%s not explain the failure:\n",
               (n > 1) ? "s do" : " does"
               );
#ifndef NOLOCAL
        if (local) {
            if (rpackets == 0) {
                printf(" . Did you start a Kermit receiver on the far end?\n");
            } else {
                printf(
                " . Try changing the remote Kermit's FLOW-CONTROL setting.\n");
                if (!network && mdmtyp > 0)
                  if ((3 * crunched) > spackets)
                    printf(
                " . Try placing a new call to get a cleaner connection.\n");
            }
        } else if (rpackets > 0) {
            if (flow == FLO_NONE)
             printf(" . Give me a SET FLOW XON/XOFF command and try again.\n");
            else
             printf(" . Give me a SET FLOW NONE command and try again.\n");
        }
        x++;
#endif /* NOLOCAL */

        if ((3 * timeouts) > spackets)
          printf(" . Adjust the timeout method (see HELP SET SEND).\n");
        if ((3 * retrans) > spackets)
          printf(" . Increase the retry limit (see HELP SET RETRY).\n");

#ifdef CK_SPEED
        if (prefixing != PX_ALL && rpackets > 2) {
            printf(" . Try it again with: SET PREFIXING ALL\n");
            x++;
        }
#endif /* CK_SPEED */
#ifdef STREAMING
        if (streamed) {
            printf(" . Try it again with: SET STREAMING OFF\n");
            x++;
        } else if (reliable) {
            printf(" . Try it again with: SET RELIABLE OFF\n");
            x++;
        }
#endif /* STREAMING */

#ifdef CK_SPEED
        if (clearrq > 0 && prefixing == PX_NON) {
            printf(" . Try it again with: SET CLEAR-CHANNEL OFF\n");
            x++;
        }
#endif /* CK_SPEED */
        if (!parity) {
            printf(" . Try it again with: SET PARITY SPACE\n");
            x++;
        }
        printf(" . %sive a ROBUST command and try again.\n",
               (x > 0) ? "As a last resort, g" : "G"
               );
        printf("Also:\n");
        printf(" . Be sure the source file has read permission.\n");
        printf(" . Be sure the target directory has write permission.\n");
/*
        if the file was 2G or larger make sure other Kermit supports LFs...
*/
        printf(" . Be sure the target disk has sufficient space.\n");
        printf("(Use SET HINTS OFF to suppress hints.)\n");
        printf("*************************\n\n");
    }
    debug(F101,"topcmd","",topcmd);
    if (getcmd && protocol == PROTO_K &&
	!success && hints && !interrupted && !fatalio && !xcmdsrc) {
        int x = 0;
        extern int urpsiz, wslotr;
        printf("\n*************************\n");
        printf("RECEIVE- or GET-class command failed.\n");
        printf(" Packets received: %d\n", rpackets);
        printf(" Damaged packets: %d\n", crunched);
        printf(" Timeouts: %d\n", timeouts);
        if (rpackets > 0)
          printf(" Packet length: %d\n", urpsiz);
        if (epktrcvd) {
            printf(" Transfer canceled by sender.\n");
            printf(" Sender's message: \"%s\"\n",(char *)epktmsg);
        }
#ifdef UNIX
        if (errno != 0)
#endif /* UNIX */
          printf(" Most recent local error: \"%s\"\n",ck_errstr());
        printf(
   "\nHINTS... If the preceding error message%s not explain the failure:\n",
               epktrcvd ? "s do" : " does"
               );
#ifndef NOLOCAL
        if (local) {
            if (topcmd == XXGET)
              printf(" . Did you start a Kermit SERVER on the far end?\n");
            if (rpackets == 0) {
                if (topcmd != XXGET)
                  printf(" . Did you start a Kermit SENDer on the far end?\n");
            } else {
                printf(
                " . Choose a different FLOW-CONTROL setting and try again.\n");
            }
        } else if (topcmd == XXGET)
          printf(" . Is the other Kermit in (or does it have) SERVER mode?\n");
        if (rpackets > 0 && urpsiz > 90)
          printf(" . Try smaller packets (SET RECEIVE PACKET-LENGTH).\n");
        if (rpackets > 0 && wslotr > 1 && !streamed)
          printf(" . Try a smaller window size (SET WINDOW).\n");
        if (!local && rpackets > 0) {
            if (flow == FLO_NONE)
             printf(" . Give me a SET FLOW XON/XOFF command and try again.\n");
            else
             printf(" . Give me a SET FLOW NONE command and try again.\n");
        }
        x++;
#endif /* NOLOCAL */
#ifdef STREAMING
        if (streamed) {
            printf(" . Try it again with: SET STREAMING OFF\n");
            x++;
        } else if (reliable && local) {
            printf(" . Try it again with: SET RELIABLE OFF\n");
            x++;
        } else
#endif /* STREAMING */
        if (!parity) {
            printf(" . Try it again with: SET PARITY SPACE\n");
            x++;
        }
        printf((x > 0) ?
               " . As a last resort, give a ROBUST command and try again.\n" :
               " . Give a ROBUST command and try again.\n"
               );
        printf("Also:\n");
        printf(" . Be sure the target directory has write permission.\n");
        printf(" . Be sure the target disk has sufficient space.\n");
        printf(" . Try telling the %s to SET PREFIXING ALL.\n",
               topcmd == XXGET ? "server" : "sender"
               );
        printf(" . Try giving a ROBUST command to the %s.\n",
               topcmd == XXGET ? "server" : "sender"
               );
        printf("(Use SET HINTS OFF to suppress hints.)\n");
        printf("*************************\n\n");
    }
#endif /* NOHINTS */
    getcmd = 0;
    sndcmd = 0;
    interrupted = 0;
#endif /* NOXFER */

    while (sstate == 0) {               /* Parse cmds until action requested */
        debug(F100,"parse top","",0);
	what = W_COMMAND;		/* Now we're parsing commands. */
	rcdactive = 0;			/* REMOTE CD not active */
	keepallchars = 0;		/* MINPUT not active */

#ifdef OS2
        if (apcactive == APC_INACTIVE)
          WaitCommandModeSem(-1);
#endif /* OS2 */
#ifdef IKS_OPTION
        if ((local &&
             !xcmdsrc &&
             is_tn &&
             TELOPT_ME(TELOPT_KERMIT) &&
             TELOPT_SB(TELOPT_KERMIT).kermit.me_start) ||
            (!local &&
             !cmdadl &&
             TELOPT_ME(TELOPT_KERMIT) &&
             TELOPT_SB(TELOPT_KERMIT).kermit.me_start)
            ) {
            tn_siks(KERMIT_STOP);
        }
#endif /* IKS_OPTION */

#ifndef NOXFER
        if (autopath) {
            fnrpath = PATH_AUTO;
            autopath = 0;
        }
        remfile = 0;                    /* Clear these in case REMOTE */
        remappd = 0;                    /* command was interrupted... */
        rempipe = 0;
        makestr(&snd_move,g_snd_move);  /* Restore these */
        makestr(&rcv_move,g_rcv_move);
        makestr(&snd_rename,g_snd_rename);
        makestr(&rcv_rename,g_rcv_rename);
#endif /* NOXFER */

    /* Take requested action if there was an error in the previous command */

        setint();
        debug(F101,"parser tlevel","",tlevel);
        debug(F101,"parser cmd_rows","",cmd_rows);

#ifndef NOLOCAL
        debug(F101,"parser wasclosed","",wasclosed);
        if (wasclosed) {                /* If connection was just closed */
#ifndef NOSPL
            int k;
            k = mlook(mactab,"on_close",nmac); /* Look up "on_close" */
            if (k >= 0) {               /* If found, */
                /* printf("ON_CLOSE CMD LOOP\n"); */
                dodo(k,ckitoa(whyclosed),0); /* Set it up */
            }
#endif /* NOSPL */
            whyclosed = WC_REMO;
            wasclosed = 0;
        }
#endif /* NOLOCAL */

#ifndef NOSPL
        xxdot = 0;                      /* Clear this... */

        debug(F101,"parser success","",success);
        if (success == 0) {
            if (cmdstk[cmdlvl].src == CMD_TF && takerr[cmdlvl]) {
                printf("Command file terminated by error.\n");
                popclvl();
                if (cmdlvl == 0) return(0);
            }
            if (cmdstk[cmdlvl].src == CMD_MD && merror[cmdlvl]) {
                printf("Command error: macro terminated.\n");
                popclvl();
                if (m && (cmdlvl < inlevel))
                  return((int) sstate);
            }
        }
        nulcmd = (m == 2);
        debug(F101,"parser nulcmd","",nulcmd);
#else
        if (success == 0 && tlevel > -1 && takerr[tlevel]) {
            printf("Command file terminated by error.\n");
            popclvl();
            cmini(ckxech);              /* Clear the cmd buffer. */
            if (tlevel < 0)             /* Just popped out of cmd files? */
              return(0);                /* End of init file or whatever. */
        }
#endif /* NOSPL */

#ifdef MAC
        /* Check for TAKE initiated by menu. */
        if ((tlevel == -1) && lfiles)
            startlfile();
#endif /* MAC */

        /* If in TAKE file, check for EOF */
#ifndef NOSPL
#ifdef MAC
        if
#else
        while
#endif /* MAC */
          ((cmdstk[cmdlvl].src == CMD_TF)  /* If end of take file */
               && (tlevel > -1)
               && feof(tfile[tlevel])) {
            popclvl();                  /* pop command level */
            cmini(ckxech);              /* and clear the cmd buffer. */
            if (cmdlvl == 0) {          /* Just popped out of all cmd files? */
                return(0);              /* End of init file or whatever. */
            }
        }
#ifdef MAC
        miniparser(1);
        if (sstate == 'a') {            /* if cmd-. cancel */
            debug(F100, "parser: cancel take due to sstate", "", sstate);
            sstate = '\0';
            dostop();
            return(0);                  /* End of init file or whatever. */
        }
#endif /*  MAC */

#else /* NOSPL */
        if ((tlevel > -1) && feof(tfile[tlevel])) { /* If end of take */
            popclvl();                  /* Pop up one level. */
            cmini(ckxech);              /* and clear the cmd buffer. */
            if (tlevel < 0)             /* Just popped out of cmd files? */
              return(0);                /* End of init file or whatever. */
        }
#endif /* NOSPL */


#ifndef NOSPL
        debug(F101,"parser cmdlvl","",cmdlvl);
        debug(F101,"parser cmdsrc","",cmdstk[cmdlvl].src);

        if (cmdstk[cmdlvl].src == CMD_MD) { /* Executing a macro? */
            debug(F100,"parser macro","",0);
            maclvl = cmdstk[cmdlvl].lvl; /* Get current level */
            debug(F101,"parser maclvl","",maclvl);
            cbp = cmdbuf;               /* Copy next cmd to command buffer. */
            *cbp = NUL;
            if (*savbuf) {              /* In case then-part of 'if' command */
                ckstrncpy(cbp,savbuf,CMDBL); /* was saved, restore it. */
                *savbuf = '\0';
            } else {                    /* Else get next cmd from macro def */
                if (getncm(cbp,CMDBL) < 0) {
#ifdef DEBUG
                    if (deblog) {
                        debug(F101,"parser end of macro m","",m);
                        debug(F101,"parser end of macro cmdlvl","",cmdlvl);
                        debug(F101,"parser end of macro inlevel","",inlevel);
                    }
#endif /* DEBUG */
                    if (m && (cmdlvl < inlevel))
                      return((int) sstate);
                    else /* if (!m) */ continue;
                }
            }
            debug(F010,"CMD(M)",cmdbuf,0);

        } else if (cmdstk[cmdlvl].src == CMD_TF)
#else
          if (tlevel > -1)
#endif /* NOSPL */
          {
#ifndef NOSPL
            debug(F111,"parser savbuf",savbuf,tlevel);
            if (*savbuf) {              /* In case THEN-part of IF command */
                ckstrncpy(cmdbuf,savbuf,CMDBL); /* was saved, restore it. */
                *savbuf = '\0';
            } else
#endif /* NOSPL */

              /* Get next line from TAKE file */

              if ((tfcode = getnct(cmdbuf,CMDBL,tfile[tlevel],0)) < 0) {
                  debug(F111,"parser tfcode",tfile[tlevel],tfcode);
                  if (tfcode < -1) {    /* Error */
                      printf("?Error in TAKE command file: %s\n",
                             (tfcode == -2) ? "Memory allocation failure" :
                             "Line too long or contains NUL characters"
                             );
                      dostop();
                  }
                  continue;             /* -1 means EOF */
              }

        /* If interactive, get next command from user. */

        } else {                        /* User types it in. */
            if (pflag) prompt(xxstring);
            cmini(ckxech);
        }

    /* Now we know where next command is coming from. Parse and execute it. */

        repars = 1;                     /* 1 = command needs parsing */
#ifndef NOXFER
        displa = 0;                     /* Assume no file transfer display */
#endif /* NOXFER */

        while (repars) {                /* Parse this cmd until entered. */

            debug(F101,"parser top of while loop","",0);
	    xaskmore = saveask;		/* Restore global more-prompting */
	    diractive = 0;		/* DIR command not active */
	    cdactive = 0;		/* CD command not active */
#ifndef NOSPL
	    askflag = 0;		/* ASK command not active */
	    echostars = 0;		/* Nor ASKQ */
	    debok = 1;			/* Undisable debugging */
#endif /* NOSPL */

#ifdef RECURSIVE
            /* In case of "send /recursive ./?<Ctrl-U>" etc */
            recursive = 0;              /* This is never sticky */
#endif /* RECURSIVE */
            xfiletype = -1;             /* Reset this between each command */
#ifndef NOMSEND
            addlist = 0;
#endif /* NOMSEND */
#ifdef CK_RECALL
            on_recall = 1;
#endif /* CK_RECALL */
            /* This might have been changed by a switch */
            if (g_matchdot > -1) {
                matchdot = g_matchdot;
                g_matchdot = -1;
            }
            cmres();                    /* Reset buffer pointers. */

#ifdef OS2
#ifdef COMMENT
            /* we check to see if a macro is waiting to be executed */
            /* if so, we call domac on it */
            if (cmdmac) {
                ckstrncpy(cmdbuf, cmdmac, CMDBL);
                free(cmdmac);
                cmdmac = NULL;
            }
#endif /* COMMENT */
#endif /* OS2 */
#ifndef NOXFER
            bye_active = 0;
#endif /* NOXFER */
            havetoken = 0;
            xx = cmkey2(cmdtab,ncmd,"Command","",toktab,xxstring,1);
            debug(F101,"top-level cmkey2","",xx);
            if (xx == -5) {
                yy = chktok(toktab);
                debug(F101,"top-level cmkey token","",yy);
#ifndef COMMENT
                /* Either way makes absolutely no difference */
                debug(F110,"NO UNGWORD",atmbuf,0);
                /* ungword(); */
#else
                debug(F110,"TOKEN UNGWORD",atmbuf,0);
                ungword();
#endif /* COMMENT */
                switch (yy) {
                  case '#': xx = XXCOM; break; /* Comment */
                  case ';': xx = XXCOM; break; /* Comment */
#ifndef NOSPL
                  case '.': xx = XXDEF; xxdot = 1; break; /* Assignment */
                  case ':': xx = XXLBL; break; /* GOTO label */
#endif /* NOSPL */

#ifndef NOPUSH
#ifdef CK_REDIR
                  case '<':
#endif /* CK_REDIR */
                  case '@':
                  case '!':
                    if (nopush) {
                        char *s; int x;
                        if ((x = cmtxt("Text to be ignored","",&s,NULL)) < 0)
                          return(x);
                        success = 0;
                        xx = XXCOM;
                    } else {
                        switch(yy) {
#ifdef CK_REDIR
                          case '<': xx = XXFUN; break; /* REDIRECT */
#endif /* CK_REDIR */
                          case '@':
                          case '!': xx = XXSHE; break; /* Shell escape */
                        }
                    }
                    break;
#endif /* NOPUSH */
#ifdef CK_RECALL
                  case '^': xx = XXREDO;  break;
#endif /* CK_RECALL */
#ifndef NOSPL
                  case '{': xx = XXMACRO; break;
                  case '(': xx = XXSEXP;  break;
#endif /* NOSPL */

                  default:
                    if (!quiet && !cmd_err) {
                        printf("\n?Invalid - \"%s\"\n",
			       cmddisplay((char *)cmdbuf,xx)
			       );
			cmderr();
                    }
                    xx = -2;
                }
                havetoken = 1;
                debug(F101,"HAVE TOKEN","",xx);
            }
            if (xx > -1) {
                topcmd = xx;            /* Top-level command index */
#ifndef NOSPL
                if (maclvl > -1)
                  lastcmd[maclvl] = xx;
#endif /* NOSPL */
                debug(F101,"topcmd","",topcmd);
                debug(F101,"cmflgs","",cmflgs);
            }

#ifndef NOSPL
            /* Special handling for IF..ELSE */

            debug(F101,"cmdlvl","",cmdlvl);
            debug(F101,"ifcmd[cmdlvl]","",ifcmd[cmdlvl]);

            if (ifcmd[cmdlvl])          /* Count stmts after IF */
              ifcmd[cmdlvl]++;
            if (ifcmd[cmdlvl] > 2 && xx != XXELS && xx != XXCOM)
              ifcmd[cmdlvl] = 0;

            /* Execute the command and take action based on return code. */

            if (nulcmd) {               /* Ignoring this command? */
                xx = XXCOM;             /* Make this command a comment. */
            }
            fnsuccess = 1;              /* For catching \function() errors */
#endif /* NOSPL */

            debug(F101,"calling docmd()","",xx);
            zz = docmd(xx);             /* Parse rest of command & execute. */

#ifndef NOSPL
	    {				/* For \v(lastcommand) */
		extern char * prevcmd;
		/* The exception list kind of a hack but let's try it... */
		if (ckstrcmp(cmdbuf,"_getarg",7,0) &&
		    ckstrcmp(cmdbuf,"if ",3,0) &&
		    ckstrcmp(cmdbuf,"xif ",4,0) &&
		    ckstrcmp(cmdbuf,"do _if",6,0) &&
		    ckstrcmp(cmdbuf,"_assign _if",11,0))
		  ckstrncpy(prevcmd,cmdbuf,CMDBL);
	    }
#endif	/* NOSPL */

#ifndef NOSPL
            if (fnerror && !fnsuccess)
              success = 0;
#endif /* NOSPL */
            debug(F101,"docmd returns","",zz);
            /* debug(F011,"cmdbuf",cmdbuf,30); */
            /* debug(F011,"atmbuf",atmbuf,30); */
#ifdef MAC
            if (tlevel > -1) {
                if (sstate == 'a') {    /* if cmd-. cancel */
                    debug(F110, "parser: cancel take, sstate:", "a", 0);
                    sstate = '\0';
                    dostop();
                    return(0);          /* End of init file or whatever. */
                }
            }
#endif /* MAC */
            switch (zz) {
              case -4:                  /* EOF (e.g. on redirected stdin) */
                doexit(GOOD_EXIT,xitsta); /* ...exit successfully */
              case -1:                  /* Reparse needed */
                repars = 1;             /* Just set reparse flag and... */
                continue;
#ifdef OS2
              case -7:                  /* They typed a disk letter */
                if (!zchdir((char *)cmdbuf)) {
                    perror((char *)cmdbuf);
                    success = 0;
                } else success = 1;
                repars = 0;
                continue;

#endif /* OS2 */
              case -6:                  /* Invalid command given w/no args */
              case -2: {		/* Invalid command given w/args */
		  int x = 0;
		  char * eol = "";
		  x = strlen(cmdbuf);	/* Avoid blank line */
		  if (x > 0) {
		      if (cmdbuf[x-1] != LF)
			eol = "\n";
		      printf("?Invalid: %s%s",
			     cmddisplay(cmdbuf,xx),eol
			     );
		  } else
		    printf("?Invalid\n");
	      }
	      case -9:			/* Bad, error message already done */
		success = 0;
		debug(F110,"top-level cmkey failed",cmdbuf,0);
		/* If in background w/ commands coming stdin, terminate */
		if (pflag == 0 && tlevel < 0)
		  fatal("Kermit command error in background execution");
/*
  Command retry feature, edit 190.  If we're at interactive prompting level,
  reprompt the user with as much of the command as didn't fail.
*/
#ifdef CK_RECALL
		if (cm_retry && !xcmdsrc) { /* If at top level */
		    int len;
		    char *p, *s;
		    len = strlen(cmdbuf); /* Length of command buffer */
		    p = malloc(len + 1);  /* Allocate space for copy */
		    if (p) {              /* If we got the space copy */
			strcpy(p,cmdbuf); /* the command buffer (SAFE). */
			/* Chop off final field, the one that failed. */
			s = p + len - 1;          /* Point to end */
			while (*s == SP && s > p) /* Trim blanks */
			  s--;
			while (*s != SP && s > p) /* Trim last field */
			  s--;
			if (s > p)        /* Keep the space */
			  s++;            /* after last good field */
			if (s >= p)       /* Cut off remainder */
			  *s = NUL;
			cmini(ckxech);    /* Reinitialize the parser */
			ckstrncpy(cmdbuf,p,CMDBL); /* Copy result back */
			free(p);          /* Free temporary storage */
			p = NULL;
			prompt(xxstring); /* Reprint the prompt */
			printf("%s",cmdbuf); /* Reprint partial command */
			repars = 1;          /* Force reparse */
			continue;
		    }
		} else
#endif /* CK_RECALL */
		  cmderr();

		cmini(ckxech);		/* (fall thru) */

	      case -3:			/* Empty command OK at top level */
		repars = 0;		/* Don't need to reparse. */
		continue;		/* Go back and get another command. */

	      default:			/* Command was successful. */
#ifndef NOSPL
		debug(F101,"parser preparing to continue","",maclvl);
#endif /* NOSPL */
		debug(F101,"parser success","",success);
		repars = 0;		/* Don't need to reparse. */
		continue;		/* Go back and get another command. */
	    }
        }
#ifndef NOSPL
        debug(F101,"parser breaks out of while loop","",maclvl);
        if (m && (cmdlvl < inlevel))  return((int) sstate);
#endif /* NOSPL */
    }

/* Got an action command, return start state. */

    return((int) sstate);
}

#ifndef NOSPL
/*
  OUTPUT command.
  Buffering and pacing added by L.I. Kirby, 5A(189), June 1993.
*/
#define OBSIZE 80                       /* Size of local character buffer */

static int obn;                         /* Buffer offset (high water mark) */
static char obuf[OBSIZE+1];             /* OUTPUT buffer. */
static char *obp;                       /* Pointer to output buffer. */
_PROTOTYP( static int oboc, (char) );
_PROTOTYP( static int xxout, (char *, int) );

static int
#ifdef CK_ANSIC
xxout(char *obuf, int obsize)
#else
xxout(obuf, obsize) char *obuf; int obsize;
#endif /* CK_ANSIC */
/* xxout */ {                           /* OUTPUT command's output function */
    int i, rc;

    debug(F101,"xxout obsize","",obsize);
    debug(F101,"xxout pacing","",pacing);
    debug(F111,"xxout string",obuf,strlen(obuf));

    rc = 0;                             /* Initial return code. */
    if (!obuf || (obsize <= 0))         /* Nothing to output. */
      goto xxout_x;                     /* Return successfully */

    rc = -1;                              /* Now assume failure */
    if (pacing == 0) {                    /* Is pacing enabled? */
        if ((local ?                      /* No, write entire string at once */
             ttol((CHAR *)obuf, obsize) : /* to communications device */
             conxo(obsize, obuf))         /* or to console */
            != obsize)
          goto xxout_x;
    } else {
        for (i = 0; i < obsize; i++) {  /* Write individual chars */
            if ((local ? ttoc(obuf[i]) : conoc(obuf[i])) < 0)
              goto xxout_x;
            msleep(pacing);
        }
    }
    if (duplex) {
#ifdef OS2
        if (inecho && local) {
#ifndef NOLOCAL
            for (i = 0; i < obsize; i++) { /* Write to emulator */
                scriptwrtbuf((USHORT)obuf[i]); /* which also logs session */
            }
#endif /* NOLOCAL */
            conxo(obsize,obuf);
        } else if (seslog) {            /* or log session here */
            logstr((char *) obuf, obsize);
        }
#else /* OS2 */
        if (seslog) {
            logstr((char *) obuf, obsize);
        }
        if (inecho && local) {
            conxo(obsize,obuf);
        }
#endif /* OS2 */
    }
    rc = 0;                             /* Success */
  xxout_x:
    obn = 0;                            /* Reset count */
    obp = obuf;                         /* and pointers */
    return(rc);                         /* return our return code */
}

#ifdef COMMENT
/*
  Macros for OUTPUT command execution, to make it go faster.
*/
#define obfls() ((xxout(obuf,obn)<0)?-1:0)
#define oboc(c) ((*obp++=(char)(c)),*obp=0,(((++obn)>=OBSIZE)?obfls():0))

#else /* The macros cause some compilers to generate bad code. */

static int
#ifdef CK_ANSIC
oboc(char c)
#else
oboc(c) char c;
#endif /* CK_ANSIC */
/* oboc */ {                            /* OUTPUT command's output function */

    *obp++ = c;                         /* Deposit character */
    *obp = NUL;                         /* Flush buffer if it's now full */

    return(((++obn) >= OBSIZE) ? xxout(obuf,obn) : 0);
}
#endif /* COMMENT */

/*  Routines for handling local variables -- also see popclvl().  */

VOID
freelocal(m) int m; {                   /* Free local variables */
    struct localvar * v, * tv;          /* at macro level m... */
    debug(F101,"freelocal level","",m);
    if (m < 0) return;
    v = localhead[m];                   /* List head for level m */
    while (v) {
        if (v->lv_name)                 /* Variable name */
          free(v->lv_name);
        if (v->lv_value)                /* Value */
          free(v->lv_value);
        tv = v;                         /* Save pointer to this node */
        v = v->lv_next;                 /* Get next one */
        if (tv)                         /* Free this one */
          free((char *)tv);
    }
    localhead[m] = (struct localvar *) NULL; /* Done, set list head to NULL */
}

#define MAXLOCALVAR 64

/* Return a pointer to the definition of a user-defined variable */

static char *
vardef(s,isarray,x1,x2) char * s; int * isarray, * x1, * x2; {
    char * p;
    char c;
    *isarray = 0;
    if (!s) return(NULL);
    if (!*s) return(NULL);
    p = s;
    if (*s == CMDQ) {
        p++;
        if (!*p)
          return(NULL);
        if ((c = *p) == '%') {          /* Scalar variable. */
            c = *++p;                   /* Get ID character. */
            p = "";                     /* Assume definition is empty */
            if (!c)
              return(NULL);
            if (c >= '0' && c <= '9') { /* Digit for macro arg */
                if (maclvl < 0)         /* Digit variables are global */
                  return(g_var[c]);     /* if no macro is active */
                else                    /* otherwise */
                  return(m_arg[maclvl][c - '0']); /* they're on the stack */
            } else if (isalpha(c)) {
                if (isupper(c)) c -= ('a'-'A');
                return(g_var[c]);           /* Letter for global variable */
            } else
              return(NULL);
        } else if (c == '&') {          /* Array reference. */
            int x, vbi, d;
            x = arraynam(p,&vbi,&d);    /* Get name and subscript */
            if (x > -1 || d == -17) {
                *isarray = 1;
                *x1 = vbi;
                *x2 = (d == -17) ? 0 : d;
            }
            if (x < 0)
              return(NULL);
            if (chkarray(vbi,d) >= 0) {	/* Array is declared? */
                vbi -= ARRAYBASE;       /* Convert name to index */
                if (a_dim[vbi] >= d) {  /* If subscript in range */
                    char **ap;
                    ap = a_ptr[vbi];
                    return((ap) ? ap[d] : NULL);
                }
            }
        }
        return(NULL);
    } else {
        int k;
        k = mxlook(mactab,s,nmac);
        return((k > -1) ? mactab[k].mval : NULL);
    }
}


int
addlocal(p) char * p; {
    int x, z, isarray = 0;
    char * s;
    struct localvar * v, *prev = (struct localvar *)NULL;
    extern int tra_asg; int tra_tmp;

    tra_tmp = tra_asg;

    s = vardef(p,&isarray,&x,&z);       /* Get definition of variable */
    if (isarray) {                      /* Arrays are handled specially */
        pusharray(x,z);
        return(0);
    }
    if (!s) s = "";
    if ((v = localhead[cmdlvl])) {      /* Already have some at this level? */
        while (v) {                     /* Find end of list */
            prev = v;
            v = v->lv_next;
        }
    }
    v = (struct localvar *) malloc(sizeof(struct localvar));
    if (!v) {
        printf("?Failure to allocate storage for local variables");
        return(-9);
    }
    if (!localhead[cmdlvl])             /* If first, set list head */
      localhead[cmdlvl] = v;
    else                                /* Otherwise link previous to this */
      prev->lv_next = v;
    prev = v;                           /* And make this previous */
    v->lv_next = (struct localvar *) NULL; /* No next yet */

    if (!(v->lv_name = (char *) malloc((int) strlen(p) + 1)))
      return(-1);
    strcpy(v->lv_name, p);              /* Copy name into new node (SAFE) */

    if (*s) {
        if (!(v->lv_value = (char *) malloc((int) strlen(s) + 1)))
          return(-1);
        strcpy(v->lv_value, s);         /* Copy value into new node (SAFE) */
    } else
      v->lv_value = NULL;

    tra_asg = 0;
    delmac(p,1);                        /* Delete the original macro */
    tra_asg = tra_tmp;
    return(0);
}

int
dolocal() {                             /* Do the LOCAL command */
    int i, x;
    char * s;
    char * list[MAXLOCALVAR+2];         /* Up to 64 variables per line */

    if ((x = cmtxt("Variable name(s)","",&s,NULL)) < 0)
      return(x);

    xwords(s,MAXLOCALVAR,list,0);       /* Break up line into "words" */

    /* Note: Arrays do not use the localhead list, but have their own stack */

    for (i = 1; i < MAXLOCALVAR && list[i]; i++) { /* Go through the list */
        if (addlocal(list[i]) < 0)
          goto localbad;
    }
    return(success = 1);

  localbad:
    printf("?Failure to allocate storage for local variables");
    freelocal(cmdlvl);
    return(-9);
}

/*  D O O U T P U T  --  Returns 0 on failure, 1 on success */

#ifndef NOKVERBS
#define K_BUFLEN 30
#define SEND_BUFLEN 255
#define sendbufd(x) { osendbuf[sendndx++] = x;\
 if (sendndx == SEND_BUFLEN) {dooutput(s,cx); sendndx = 0;}}
#endif /* NOKVERBS */

int outesc = 1;                         /* Process special OUTPUT escapes */

int
dooutput(s, cx) char *s; int cx; {
#ifdef SSHBUILTIN
    extern int ssh_cas;
    extern char * ssh_cmd;
#endif /* SSHBUILTIN */
    int x, xx, y, quote;                /* Workers */
    int is_tn = 0;

    is_tn = (local && network && IS_TELNET()) ||
      (!local && sstelnet);

    debug(F111,"dooutput s",s,(int)strlen(s));

    if (local) {                        /* Condition external line */
#ifdef NOLOCAL
        goto outerr;
#else
	if (ttchk() < 0) {
	    if (!network) {
		if (carrier != CAR_OFF) {
		    int x;
		    x = ttgmdm();
		    if ((x > -1) && ((x & BM_DCD) == 0)) {
			printf(
"?Carrier signal required but not present - Try SET CARRIER-WATCH OFF.\n"
                              );
			return(0);
		    }
		} else {
		    printf(
"?Problem with serial port or modem or cable - Try SHOW COMMUNICATIONS.\n"
                          );
		    return(0);
		}
	    }
	    printf("?Connection %s %s is not open or not functioning.\n",
		   network ? "to" : "on",
		   ttname
		   );
	    return(0);
	}
        if (ttvt(speed,flow) < 0) {
            printf("?OUTPUT initialization error\n");
            return(0);
        }
#endif /* NOLOCAL */
    }
#ifdef SSHBUILTIN
    if ( network && nettype == NET_SSH && ssh_cas && ssh_cmd && 
         !(strcmp(ssh_cmd,"kermit") && strcmp(ssh_cmd,"sftp"))) {
        if (!quiet)
            printf("?SSH Subsystem active: %s\n", ssh_cmd);
        return(0);
    }
#endif /* SSHBUILTIN */

    if (!cmdgquo()) {                   /* COMMAND QUOTING OFF */
        x = strlen(s);                  /* Just send the string literally */
        xx = local ? ttol((CHAR *)s,x) : conxo(x,s);
        return(success = (xx == x) ? 1 : 0);
    }
    quote = 0;                          /* Initialize backslash (\) quote */
    obn = 0;                            /* Reset count */
    obp = obuf;                         /* and pointers */

  outagain:
    while ((x = *s++)) {                /* Loop through the string */
        y = 0;                          /* Error code, 0 = no error. */
        debug(F000,"dooutput","",x);
        if (quote) {                    /* This character is quoted */
#ifndef NOKVERBS
           if (x == 'k' || x == 'K') {  /* \k or \K */
               extern struct keytab kverbs[];
               extern int nkverbs;
               extern char * keydefptr;
               extern int keymac;
               extern int keymacx;
               int x, y, brace = 0;
               int pause;
               char * p, * b;
               char kbuf[K_BUFLEN + 1]; /* Key verb name buffer */
               char osendbuf[SEND_BUFLEN +1];
               int  sendndx = 0;

               if (xxout(obuf,obn) < 0) /* Flush buffer */
                 goto outerr;
               debug(F100,"OUTPUT KVERB","",0); /* Send a KVERB */
               {                        /* Have K verb? */
                   if (!*s) {
                       break;
                   }
/*
  We assume that the verb name is {braced}, or it extends to the end of the
  string, s, or it ends with a space, control character, or backslash.
*/
                   p = kbuf;            /* Copy verb name into local buffer */
                   x = 0;
                   while ((x++ < K_BUFLEN) && (*s > SP) && (*s != CMDQ)) {
                       if (brace && *s == '}') {
                           break;
                       }
                       *p++ = *s++;
                   }
                   if (*s && !brace)    /* If we broke because of \, etc, */
                     s--;               /*  back up so we get another look. */
                   brace = 0;
                   *p = NUL;            /* Terminate. */
                   p = kbuf;            /* Point back to beginning */
                   debug(F110,"dooutput kverb",p,0);
                   y = xlookup(kverbs,p,nkverbs,&x); /* Look it up */
                   debug(F101,"dooutput lookup",0,y);
                   if (y > -1) {
                       if (sendndx) {
                           dooutput(osendbuf,cx);
                           sendndx = 0;
                       }
                       dokverb(VCMD,y);
#ifndef NOSPL
                   } else {             /* Is it a macro? */
                       y = mxlook(mactab,p,nmac);
                       if (y > -1) {
                           cmpush();
                           keymac = 1;  /* Flag for key macro active */
                           keymacx = y; /* Key macro index */
                           keydefptr = s; /* Where to resume next time */
                           debug(F111,"dooutput mxlook",keydefptr,y);
                           parser(1);
                           cmpop();
                       }
#endif /* NOSPL */
                   }
               }
               quote = 0;
               continue;
           } else
#endif /* NOKVERBS */
             if (outesc && (x == 'n' || x == 'N')) { /* \n or \N */
                 if (xxout(obuf,obn) < 0) /* Flush buffer */
                   goto outerr;
                 debug(F100,"OUTPUT NUL","",0); /* Send a NUL */
                 if (local)
                   ttoc(NUL);
                 else
                   conoc(NUL);
                 quote = 0;
                 continue;

             } else if (outesc && (x == 'b' || x == 'B')) { /* \b or \B */

                if (xxout(obuf,obn) < 0) /* Flush buffer first */
                  goto outerr;
                debug(F100,"OUTPUT BREAK","",0);
#ifndef NOLOCAL
                ttsndb();               /* Send BREAK signal */
#else
                 if (local)
                   ttoc(NUL);
                 else
                   conoc(NUL);
#endif /* NOLOCAL */
                quote = 0;              /* Turn off quote flag */
                continue;               /* and not the b or B */
#ifdef CK_LBRK
             } else if (outesc && (x == 'l' || x == 'L')) { /* \l or \L */
                 if (xxout(obuf,obn) < 0) /* Flush buffer first */
                   goto outerr;
                 debug(F100,"OUTPUT Long BREAK","",0);
#ifndef NOLOCAL
                 ttsndlb();             /* Send Long BREAK signal */
#else
                 if (local)
                   ttoc(NUL);
                 else
                   conoc(NUL);
#endif /* NOLOCAL */
                 quote = 0;             /* Turn off quote flag */
                 continue;              /* and not the l or L */
#endif /* CK_LBRK */

             } else if (x == CMDQ) {    /* Backslash itself */
                 debug(F100,"OUTPUT CMDQ","",0);
                 xx = oboc(dopar(CMDQ)); /* Output the backslash. */
                 if (xx < 0)
                   goto outerr;
                 quote = 0;
                 continue;

             } else {                   /* if \ not followed by special esc */
                /* Note: Atari ST compiler won't allow macro call in "if ()" */
                 xx = oboc(dopar(CMDQ)); /* Output the backslash. */
                 if (xx < 0)
                   goto outerr;
                 quote = 0;             /* Turn off quote flag */
             }
        } else if (x == CMDQ) {         /* This is the quote character */
            quote = 1;                  /* Go back and get next character */
            continue;                   /* which is quoted */
        }
        xx = oboc(dopar((char)x));      /* Output this character */
        debug(F111,"dooutput",obuf,obn);
        if (xx < 0)
          goto outerr;
#ifdef COMMENT
        if (seslog && duplex) {         /* Log the character if log is on */
            logchar((char)x);
        }
#endif /* COMMENT */
        if (x == '\015') {              /* String contains carriage return */
            int stuff = -1, stuff2 = -1;
            if (tnlm) {                 /* TERMINAL NEWLINE ON */
                stuff = LF;             /* Stuff LF */
            }
#ifdef TNCODE
            /* TELNET NEWLINE ON/OFF/RAW */
            if (is_tn) {
                switch (TELOPT_ME(TELOPT_BINARY) ? /* NVT or BINARY */
                        tn_b_nlm :
                        tn_nlm
                        ) {
                  case TNL_CR:
                    break;
                  case TNL_CRNUL:
                    stuff2 = stuff;
                    stuff  = NUL;
                    break;
                  case TNL_CRLF:
                    stuff2 = stuff;
                    stuff = LF;
                    break;
                }
            }
#endif /* TNCODE */
            if (stuff > -1) {           /* Stuffing another character... */
                xx = oboc(dopar((CHAR)stuff));
                if (xx < 0)
                  goto outerr;
#ifdef COMMENT
                if (seslog && duplex) { /* Log stuffed char if appropriate */
                    logchar((char)stuff);
                }
#endif /* COMMENT */
            }
            if (stuff2 > -1) {          /* Stuffing another character... */
                xx = oboc(dopar((CHAR)stuff2));
                if (xx < 0)
                  goto outerr;
#ifdef COMMENT
                if (seslog && duplex) { /* Log stuffed char if appropriate */
                    logchar((char)stuff2);
                }
#endif /* COMMENT */
            }
            if (xxout(obuf,obn) < 0)    /* Flushing is required here! */
              goto outerr;
        }
    }
    if (cx == XXLNOUT) {
        s = "\015";
        cx = 0;
        goto outagain;
    }
    if (quote == 1)                     /* String ended with backslash */
      xx = oboc(dopar(CMDQ));

    if (obn > 0)                        /* OUTPUT done */
      if (xxout(obuf,obn) < 0)          /* Flush the buffer if necessary. */
        goto outerr;
    return(1);

outerr:                                 /* OUTPUT command error handler */
    if (msgflg) printf("?OUTPUT execution error\n");
    return(0);

/* Remove "local" OUTPUT macro defininitions */

#ifdef COMMENT
/* No more macros ... */
#undef oboc
#undef obfls
#endif /* COMMENT */
}
#endif /* NOSPL */

/* Display version herald and initial prompt */

VOID
herald() {
    int x = 0, i;
    extern int srvcdmsg;
    extern char * cdmsgfile[];
    char * ssl;
    char * krb4;
    char * krb5;

#ifndef NOCMDL
    extern char * bannerfile;
    debug(F110,"herald bannerfile",bannerfile,0);
    if (bannerfile) {
        concb((char)escape);
        if (dotype(bannerfile,1,0,0,NULL,0,NULL,0,0,NULL,0) > 0) {
            debug(F111,"herald","srvcdmsg",srvcdmsg);
            if (srvcdmsg) {
                for (i = 0; i < 8; i++) {
                    debug(F111,"herald cdmsgfile[i]",cdmsgfile[i],i);
                    if (zchki(cdmsgfile[i]) > -1) {
                        printf("\n");
                        dotype(cdmsgfile[i],
                               xaskmore,0,0,NULL,0,NULL,0,0,NULL,0);
                        break;
                    }
                }
            }
            return;
        }
    }
#endif /* NOCMDL */

#ifdef COMMENT
    /* The following generates bad code in SCO compilers. */
    /* Observed in both OSR5 and Unixware 2 -- after executing this */
    /* statement when all conditions are false, x has a value of -32. */
    if (noherald || quiet || bgset > 0 || (bgset != 0 && backgrd != 0))
      x = 1;
#else
    x = 0;
    if (noherald || quiet)
      x = 1;
    else if (bgset > 0)
      x = 1;
    else if (bgset < 0 && backgrd > 0)
      x = 1;
#endif /* COMMENT */

    ssl = "";
    krb4 = "";
    krb5 = "";
#ifdef CK_AUTHENTICATION
#ifdef CK_SSL    
    ssl = "+SSL";
#endif	/* CK_SSL */
#ifdef KRB4
    krb4 = "+KRB4";
#endif	/* KRB4 */
#ifdef KRB5
    krb5 = "+KRB5";
#endif	/* KRB5 */
#endif	/* CK_AUTHENTICATION */

    if (x == 0) {
#ifdef datageneral
        printf("%s, for%s\n",versio,ckxsys);
#else
#ifdef OSK
        printf("%s, for%s\n",versio,ckxsys);
#else
#ifdef CK_64BIT
        printf("%s, for%s%s%s%s (64-bit)\n\r",versio,ckxsys,ssl,krb4,krb5);
#else
        printf("%s, for%s%s%s%s\n\r",versio,ckxsys,ssl,krb4,krb5);
#endif/* CK_64BIT */
#endif /* OSK */
#endif /* datageneral */
        printf(" Copyright (C) 1985, %s,\n", ck_cryear);
        printf("  Trustees of Columbia University in the City of New York.\n");
#ifdef OS2
       shoreg();
#endif /* OS2 */

        if (!quiet && !backgrd) {
#ifdef COMMENT
/* "Default file-transfer mode is AUTOMATIC" is useless information... */
            char * s;
            extern int xfermode;
#ifdef VMS
            s = "AUTOMATIC";
#else
            if (xfermode == XMODE_A) {
                s = "AUTOMATIC";
            } else {
                s = gfmode(binary,1);
            }
            if (!s) s = "";
#endif /* VMS */
            if (*s)
              printf("Default file-transfer mode is %s\n", s);
#endif /* COMMENT */

            debug(F111,"herald","srvcdmsg",srvcdmsg);
            if (srvcdmsg) {
                for (i = 0; i < 8; i++) {
                    debug(F111,"herald cdmsgfile[i]",cdmsgfile[i],i);
                    if (zchki(cdmsgfile[i]) > -1) {
                        printf("\n");
                        dotype(cdmsgfile[i],
                               xaskmore,0,0,NULL,0,NULL,0,0,NULL,0);
                        break;
                    }
                }
            }
            printf("Type ? or HELP for help.\n");
        }
    }
}

/*  G F M O D E  --  Get File (transfer) Mode  */

char *
gfmode(binary,upcase) int binary, upcase; {
    char * s;
    switch (binary) {
      case XYFT_T: s = upcase ? "TEXT" : "text"; break;
#ifdef VMS
      case XYFT_B: s = upcase ? "BINARY FIXED" : "binary fixed"; break;
      case XYFT_I: s = upcase ? "IMAGE" : "image"; break;
      case XYFT_L: s = upcase ? "LABELED" : "labeled"; break;
      case XYFT_U: s = upcase ? "BINARY UNDEF" : "binary undef"; break;
#else
#ifdef MAC
      case XYFT_B: s = upcase ? "BINARY" : "binary"; break;
      case XYFT_M: s = upcase ? "MACBINARY" : "macbinary"; break;
#else
      case XYFT_B: s = upcase ? "BINARY" : "binary"; break;
#ifdef CK_LABELED
      case XYFT_L: s = upcase ? "LABELED" : "labeled"; break;
#endif /* CK_LABELED */
#endif /* MAC */
#endif /* VMS */
      case XYFT_X: s = upcase ? "TENEX" : "tenex"; break;
      default: s = "";
    }
    return(s);
}

#ifndef NOSPL
static int
isaa(s) char * s; {                     /* Is associative array */
    char c;
    int x;
    if (!s) s = "";
    if (!*s) return(0);
    s++;
    while ((c = *s++)) {
        if (c == '<') {
            x = strlen(s);
            return ((*(s+x-1) == '>') ? 1 : 0);
        }
    }
    return(0);
}

/*  M L O O K  --  Lookup the macro name in the macro table  */

/*
  Call this way:  v = mlook(table,word,n);

    table - a 'struct mtab' table.
    word  - the target string to look up in the table.
    n     - the number of elements in the table.

  The keyword table must be arranged in ascending alphabetical order, and
  all letters must be lowercase.

  Returns the table index, 0 or greater, if the name was found, or:

   -3 if nothing to look up (target was null),
   -2 if ambiguous,
   -1 if not found.

  A match is successful if the target matches a keyword exactly, or if
  the target is a prefix of exactly one keyword.  It is ambiguous if the
  target matches two or more keywords from the table.
*/
int
mlook(table,cmd,n) struct mtab table[]; char *cmd; int n; {
    register int i;
    int v, w, cmdlen = 0;
    char c = 0, c1, * s;
    if (!cmd) cmd = "";

    for (s = cmd; *s; s++) cmdlen++;    /* (instead of strlen) */
    debug(F111,"MLOOK",cmd,cmdlen);

    c1 = *cmd;
    if (isupper(c1))
      c1 = tolower(c1);
    if (cmdlen < 1)
      return(-3);

/* Not null, look it up */

    if (n < 12) {                       /* Not worth it for small tables */
        i = 0;
    } else {                            /* Binary search for where to start */
        int lo = 0;
        int hi = n;
        int count = 0;
        while (lo+2 < hi && ++count < 12) {
            i = lo + ((hi - lo) / 2);
            c = *(table[i].kwd);
            if (isupper(c)) c = tolower(c);
            if (c < c1) {
                lo = i;
            } else {
                hi = i;
            }
        }
        i = (c < c1) ? lo+1 : lo;
    }
    for ( ; i < n-1; i++) {
        s = table[i].kwd;
        if (!s) s = "";
        if (!*s) continue;              /* Empty table entry */
        c = *s;
        if (isupper(c)) c = tolower(c);
        if (c1 != c) continue;          /* First char doesn't match */
        if (!ckstrcmp(s,cmd,-1,0))      /* Have exact match? */
          return(i);
        v = !ckstrcmp(s,cmd,cmdlen,0);
        w = ckstrcmp(table[i+1].kwd,cmd,cmdlen,0);
        if (v && w)                     /* Have abbreviated match? */
          return(i);
        if (v)                          /* Ambiguous? */
          return(-2);
        if (w > 0)                      /* Past our alphabetic area? */
          return(-1);
    }

/* Last (or only) element */

    if (!ckstrcmp(table[n-1].kwd,cmd,cmdlen,0))
      return(n-1);

    return(-1);
}

/* mxlook is like mlook, but an exact full-length match is required */

int
mxlook(table,cmd,n) char *cmd; struct mtab table[]; int n; {
    register int i;
    int w, cmdlen = 0, one = 0;
    register char c = 0, c1, * s;

    if (!cmd) cmd = "";                 /* Check args */

    for (s = cmd; *s; s++) cmdlen++;    /* (instead of strlen) */
    debug(F111,"MXLOOK",cmd,cmdlen);

    c1 = *cmd;                          /* First char of string to look up */
    if (isupper(c1))
      c1 = tolower(c1);
    if (!*(cmd+1))                      /* Special handling for 1-char names */
      one = 1;

    if (cmdlen < 1)                     /* Nothing to look up */
      return(-3);

    if (n < 12) {                       /* Not worth it for small tables */
        i = 0;
    } else {                            /* Binary search for where to start */
        int lo = 0;
        int hi = n;
        int count = 0;
        while (lo+2 < hi && ++count < 12) {
            i = lo + ((hi - lo) / 2);
            c = *(table[i].kwd);
            if (isupper(c)) c = tolower(c);
            if (c < c1) {
                lo = i;
            } else {
                hi = i;
            }
        }
        i = (c < c1) ? lo+1 : lo;
    }
    for ( ; i < n; i++) {               /* Look thru table */
        s = table[i].kwd;               /* This entry */
        if (!s) s = "";
        if (!*s) continue;              /* Empty table entry */
        c = *s;
        if (isupper(c)) c = tolower(c);
        if (c1 != c) continue;          /* First char doesn't match */
        if (one) {                      /* Name is one char long */
            if (!*(s+1))
              return(i);                /* So is table entry */
        }
#ifdef COMMENT
        if (((int)strlen(s) == cmdlen) &&
            (!ckstrcmp(s,cmd,cmdlen,0))) return(i);
#else
        w = ckstrcmp(s,cmd,-1,0);
        if (!w) return(i);
        if (w > 0) return(-1);
#endif /* COMMENT */
    }
    return(-1);
}

/* mxxlook is like mxlook, but case-sensitive */

int
mxxlook(table,cmd,n) char *cmd; struct mtab table[]; int n; {
    int i, cmdlen;
    if (!cmd) cmd = "";
    if (((cmdlen = strlen(cmd)) < 1) || (n < 1)) return(-3);
    /* debug(F111,"mxxlook target",cmd,n); */
    for (i = 0; i < n; i++) {
        if (((int)strlen(table[i].kwd) == cmdlen) &&
            (!strncmp(table[i].kwd,cmd,cmdlen)))
          return(i);
    }
    return(-1);
}

static int
traceval(nam, val) char * nam, * val; { /* For TRACE command */
    if (val)
      printf(">>> %s: \"%s\"\n", nam, val);
    else
      printf(">>> %s: (undef)\n", nam);
    return(0);
}

#ifdef USE_VARLEN                       /* Not used */

/*  V A R L E N  --  Get length of variable's value.

  Given a variable name, return the length of its definition or 0 if the
  variable is not defined.  If it is defined, set argument s to point to its
  definition.  Otherwise set s to NULL.
*/
int
varlen(nam,s) char *nam; char **s; {    /* Length of value of variable */
    int x, z;
    char *p = NULL, c;

    *s = NULL;
    if (!nam) return(0);                /* Watch out for null pointer */
    if (*nam == CMDQ) {
        nam++;
        if (*nam == '%') {              /* If it's a variable name */
            if (!(c = *(nam+1))) return(0); /* Get letter or digit */
            p = (char *)0;              /* Initialize value pointer */
            if (maclvl > -1 && c >= '0' && c <= '9') { /* Digit? */
                p = m_arg[maclvl][c - '0']; /* Pointer from macro-arg table */
            } else {                    /* It's a global variable */
                if (c < 33 || c > GVARS) return(0);
                p = g_var[c];           /* Get pointer from global-var table */
            }
        } else if (*nam == '&') {               /* An array reference? */
            char **q;
            if (arraynam(nam,&x,&z) < 0) /* If syntax is bad */
              return(-1);               /* return -1. */
            x -= ARRAYBASE;             /* Convert name to number. */
            if ((q = a_ptr[x]) == NULL) /* If array not declared, */
              return(0);                /* return -2. */
            if (z > a_dim[x])           /* If subscript out of range, */
              return(0);                /* return -3. */
            p = q[z];
        }
    } else {                            /* Macro */
        z = isaa(nam);
        x = z ? mxxlook(mactab,nam,nmac) : mlook(mactab,nam,nmac);
        if (x < 0)
          return(0);
        p = mactab[x].mval;
    }
    if (p)
      *s = p;
    else
      p = "";
    return((int)strlen(p));
}
#endif /* USE_VARLEN */

/*
  This routine is for the benefit of those compilers that can't handle
  long string constants or continued lines within them.  Long predefined
  macros like FOR, WHILE, and XIF have their contents broken up into
  arrays of string pointers.  This routine concatenates them back into a
  single string again, and then calls the real addmac() routine to enter
  the definition into the macro table.
*/
int
addmmac(nam,s) char *nam, *s[]; {       /* Add a multiline macro definition */
    int i, x, y; char *p;
    x = 0;                              /* Length counter */
    for (i = 0; (y = (int)strlen(s[i])) > 0; i++) { /* Add up total length */
        debug(F111,"addmmac line",s[i],y);
        x += y;
    }
    debug(F101,"addmmac lines","",i);
    debug(F101,"addmmac loop exit","",y);
    debug(F111,"addmmac length",nam,x);
    if (x < 0) return(-1);

    p = malloc(x+1);                    /* Allocate space for all of it. */
    if (!p) {
        printf("?addmmac malloc error: %s\n",nam);
        debug(F110,"addmmac malloc error",nam,0);
        return(-1);
    }
    *p = '\0';                          /* Start off with null string. */
    for (i = 0; *s[i]; i++)             /* Concatenate them all together. */
      ckstrncat(p,s[i],x+1);
    y = (int)strlen(p);                 /* Final precaution. */
    debug(F111,"addmmac constructed string",p,y);
    if (y == x) {
        y = addmac(nam,p);              /* Add result to the macro table. */
    } else {
        debug(F100,"addmmac length mismatch","",0);
        printf("\n!addmmac internal error!\n");
        y = -1;
    }
    free(p);                            /* Free the temporary copy. */
    return(y);
}

/* Here is the real addmac routine. */

/* Returns -1 on failure, macro table index >= 0 on success. */

int mtchanged = 0;

int
addmac(nam,def) char *nam, *def; {      /* Add a macro to the macro table */
    int i, x, y, z, namlen, deflen, flag = 0;
    int replacing = 0, deleting = 0;
    char * p = NULL, c, *s;
    extern int tra_asg; int tra_tmp;

    if (!nam) return(-1);
    if (!*nam) return(-1);

#ifdef IKSD
    if (inserver &&
#ifdef IKSDCONF
        iksdcf
#else /* IKSDCONF */
        1
#endif /* IKSDCONF */
        ) {
        if (!ckstrcmp("on_exit",nam,-1,0) ||
            !ckstrcmp("on_logout",nam,-1,0))
          return(-1);
    }
#endif /* IKSD */

    namlen = 0;
    p = nam;
    while (*p++) namlen++;              /* (instead of strlen) */

    tra_tmp = tra_asg;                  /* trace... */
    debug(F111,"addmac nam",nam,namlen);
    if (!def) {                         /* Watch out for null pointer */
        deflen = 0;
        debug(F111,"addmac def","(null pointer)",deflen);
    } else {
        deflen = 0;
        p = def;
        while (*p++) deflen++;          /* (instead of strlen) */
        debug(F010,"addmac def",def,0);
    }
#ifdef USE_VARLEN                       /* NOT USED */
    /* This does not boost performance much because varlen() does a lot */
    x = varlen(nam,&s);
    if (x > 0 && x >= deflen) {
        strcpy(s,def);                  /* NOT USED */
        flag = 1;
        p = s;
    }
#endif /* USE_VARLEN */

    if (*nam == CMDQ) nam++;            /* Backslash quote? */
    if (*nam == '%') {                  /* Yes, if it's a variable name, */
        if (!(c = *(nam + 1))) return(-1); /* Variable name letter or digit */
        if (!flag) {
            tra_asg = 0;
            delmac(nam,0);              /* Delete any old value. */
            tra_asg = tra_tmp;
        }
        if (deflen < 1) {               /* Null definition */
            p = NULL;                   /* Better not malloc or strcpy! */
        } else if (!flag) {             /* A substantial definition */
            p = malloc(deflen + 1);     /* Allocate space for it */
            if (!p) {
                printf("?addmac malloc error 2\n");
                return(-1);
            } else strcpy(p,def);       /* Copy def into new space (SAFE) */
        }

        /* Now p points to the definition, or is a null pointer */

        if (c >= '0' && c <= '9') {     /* \%0-9 variable */
            if (maclvl < 0) {           /* Are we calling or in a macro? */
                g_var[c] = p;           /* No, it's top level one */
                makestr(&(toparg[c - '0']),p); /* Take care \&_[] too */
            } else {                    /* Yes, it's a macro argument */
                m_arg[maclvl][c - '0'] = p; /* Assign the value */
                makestr(&(m_xarg[maclvl][c - '0']),p); /* And a copy here */
            }
        } else {                        /* It's a \%a-z variable */
            if (c < 33 || (unsigned int)c > GVARS) return(-1);
            if (isupper(c)) c = (char) tolower(c);
            g_var[c] = p;               /* Put pointer in global-var table */
        }
        if (tra_asg) traceval(nam,p);
        return(0);
    } else if (*nam == '&') {           /* An array reference? */
        char **q = NULL;
        int rc = 0;
        if ((y = arraynam(nam,&x,&z)) < 0) /* If syntax is bad */
          return(-1);                   /* return -1. */
        if (chkarray(x,z) < 0)          /* If array not declared or */
          rc = -2;                      /* subscript out of range, ret -2 */
        if (!flag) {
            tra_asg = 0;
            delmac(nam,0);              /* Delete any old value. */
            tra_asg = tra_tmp;
        }
        x -= ARRAYBASE;                 /* Convert name letter to index. */
        if (x > 'z' - ARRAYBASE + 1)
          rc = -1;
        if (rc != -1) {
            if ((q = a_ptr[x]) == NULL) /* If array not declared, */
              return(-3);               /* return -3. */
        }
        if (rc < 0)
          return(rc);
        if (!flag) {
            if (deflen > 0) {
                if ((p = malloc(deflen+1)) == NULL) { /* Allocate space */
                    printf("addmac macro error 7: %s\n",nam);
                    return(-4);         /* for new def, return -4 on fail. */
                }
                strcpy(p,def);          /* Copy def into new space (SAFE). */
            } else p = NULL;
        }
        q[z] = p;                       /* Store pointer to it. */
        if (x == 0) {                   /* Arg vector array */
            if (z >= 0 && z <= 9) {     /* Copy values to corresponding  */
                if (maclvl < 0) {       /* \%1..9 variables. */
                    makestr(&(toparg[z]),p);
                } else {
                    makestr(&(m_arg[maclvl][z]),p);
                }
            }
        }
        if (tra_asg) traceval(nam,p);
        return(0);                      /* Done. */
    }

/* Not a macro argument or a variable, so it's a macro definition */

#ifdef USE_VARLEN
    if (flag) {
        if (tra_asg) traceval(nam,p);
        return(0);
    }
#endif /* USE_VARLEN */
    x = isaa(nam) ?                     /* If it's an associative array */
      mxxlook(mactab,nam,nmac) :        /* look it up this way */
        mxlook(mactab,nam,nmac);        /* otherwise this way. */
    if (x > -1) {                       /* If found... */
        if (deflen > 0)                 /* and a new definition was given */
          replacing = 1;                /* we're replacing */
        else                            /* otherwise */
          deleting = 1;                 /* we're deleting */
    }
    if (deleting) {                     /* Deleting... */
        if (delmac(nam,0) < 0)
          return(-1);
        mtchanged++;
        if (tra_asg) traceval(nam,p);
        return(0);
    } else if (deflen < 1)              /* New macro with no definition */
      return(0);                        /* Nothing to do. */

    if (replacing) {                    /* Replacing an existing macro */
        if (mactab[x].mval) {           /* If it currently has a definition, */
            free(mactab[x].mval);       /* free it. */
            mactab[x].mval = NULL;
        }
        mtchanged++;
        y = x;                          /* Replacement index. */

    } else {                            /* Adding a new macro... */
        char c1, c2;                    /* Use fast lookup to find the */
        c1 = *nam;                      /* alphabetical slot. */
        if (isupper(c1)) c1 = (char) tolower(c1);

        if (nmac < 5) {                 /* Not worth it for small tables */
            y = 0;
        } else {                        /* First binary search to find */
            int lo = 0;                 /* where to start */
            int hi = nmac;
            int count = 0;
            char c = 0;
            while (lo+2 < hi && ++count < 12) {
                y = lo + ((hi - lo) / 2);
                c = *(mactab[y].kwd);
                if (isupper(c)) c = (char) tolower(c);
                if (c < c1) {
                    lo = y;
                } else {
                    hi = y;
                }
            }
            y = (c < c1) ? lo+1 : lo;
        }
        /* Now search linearly from starting location */
        for ( ; y < MAC_MAX && mactab[y].kwd != NULL; y++) {
            c2 = *(mactab[y].kwd);
            if (isupper(c2)) c2 = (char) tolower(c2);
            if (c1 > c2)
              continue;
            if (c1 < c2)
              break;
            if (ckstrcmp(nam,mactab[y].kwd,-1,0) <= 0)
              break;
        }
        if (y == MAC_MAX) {             /* Macro table is full. */
            debug(F101,"addmac table overflow","",y);
            printf("?Macro table overflow\n");
            return(-1);
        }
        if (mactab[y].kwd != NULL) {    /* Must insert */
            for (i = nmac; i > y; i--) { /* Move the rest down one slot */
                mactab[i].kwd = mactab[i-1].kwd;
                mactab[i].mval = mactab[i-1].mval;
                mactab[i].flgs = mactab[i-1].flgs;
            }
        }
        mtchanged++;
        p = malloc(namlen + 1);         /* Allocate space for name */
        if (!p) {
            printf("?Addmac: Out of memory - \"%s\"\n",nam);
            return(-1);
        }
        strcpy(p,nam);                  /* Copy name into new space (SAFE) */
        mactab[y].kwd = p;              /* Add pointer to table */
    }
    if (deflen > 0) {                   /* If we have a definition */
        p = malloc(deflen + 1);         /* Get space */
        if (p == NULL) {
            printf("?Space exhausted - \"%s\"\n", nam);
            if (mactab[y].kwd) {
                free(mactab[y].kwd);
                mactab[y].kwd = NULL;
            }
            return(-1);
        } else {
            strcpy(p,def);              /* Copy the definition (SAFE) */
        }
    } else {                            /* definition is empty */
        p = NULL;
    }
    mactab[y].mval = p;                 /* Macro points to definition */
    mactab[y].flgs = 0;                 /* No flags */
    if (!replacing)                     /* If new macro */
      nmac++;                           /* count it */
    if (tra_asg) traceval(nam,p);
    return(y);
}

int
xdelmac(x) int x; {                     /* Delete a macro given its index */
    int i;
    extern int tra_asg;
    if (x < 0) return(x);
    if (tra_asg)
      traceval(mactab[x].kwd,NULL);

    if (mactab[x].kwd) {                /* Free the storage for the name */
        free(mactab[x].kwd);
        mactab[x].kwd = NULL;
    }
    if (mactab[x].mval) {               /* and for the definition */
        free(mactab[x].mval);
        mactab[x].mval = NULL;
    }
    for (i = x; i < nmac; i++) {        /* Now move up the others. */
        mactab[i].kwd = mactab[i+1].kwd;
        mactab[i].mval = mactab[i+1].mval;
        mactab[i].flgs = mactab[i+1].flgs;
    }
    nmac--;                             /* One less macro */

    mactab[nmac].kwd = NULL;            /* Delete last item from table */
    mactab[nmac].mval = NULL;
    mactab[nmac].flgs = 0;
    mtchanged++;
    return(0);
}

int
delmac(nam,exact) char *nam; int exact; { /* Delete the named macro */
    int x, z;
    char *p, c;
    extern int tra_asg;

    if (!nam) return(0);                /* Watch out for null pointer */
    debug(F110,"delmac nam",nam,0);
#ifdef IKSD
    if ( inserver &&
#ifdef IKSDCONF
        iksdcf
#else /* IKSDCONF */
        1
#endif /* IKSDCONF */
        ) {
        if (!ckstrcmp("on_exit",nam,-1,0) ||
            !ckstrcmp("on_logout",nam,-1,0))
          return(-1);
    }
#endif /* IKSD */

    if (*nam == CMDQ) nam++;
    if (*nam == '%') {                  /* If it's a variable name */
        if (!(c = *(nam+1))) return(0); /* Get variable name letter or digit */
        p = (char *)0;                  /* Initialize value pointer */
        if (maclvl < 0 && c >= '0' && c <= '9') { /* Top-level digit? */
            p = toparg[c - '0'];
            if (p) if (p != g_var[c]) {
                free(p);
                toparg[c - '0'] = NULL;
            }
            p = g_var[c];
            g_var[c] = NULL;            /* Zero the table entry */
        } else if (maclvl > -1 && c >= '0' && c <= '9') { /* Digit? */
            p = m_xarg[maclvl][c - '0'];
            if (p) if (p != g_var[c]) {
                free(p);
                m_xarg[maclvl][c - '0'] = NULL;
            }
            p = m_arg[maclvl][c - '0']; /* Get pointer from macro-arg table */
            m_arg[maclvl][c - '0'] = NULL; /* Zero the table pointer */
        } else {                        /* It's a global variable */
            if (c < 33 || (unsigned int)c > GVARS) return(0);
            p = g_var[c];               /* Get pointer from global-var table */
            g_var[c] = NULL;            /* Zero the table entry */
        }
        if (p) {
            debug(F010,"delmac def",p,0);
            free(p);                    /* Free the storage */
            p = NULL;
        } else debug(F110,"delmac def","(null pointer)",0);
        if (tra_asg) traceval(nam,NULL);
        return(0);
    }
    if (*nam == '&') {                  /* An array reference? */
        char **q;
        if (arraynam(nam,&x,&z) < 0)    /* If syntax is bad */
          return(-1);                   /* return -1. */
        x -= ARRAYBASE;                 /* Convert name to number. */
        if ((q = a_ptr[x]) == NULL)     /* If array not declared, */
          return(-2);                   /* return -2. */
        if (z > a_dim[x])               /* If subscript out of range, */
          return(-3);                   /* return -3. */
        if (q[z]) {                     /* If there is an old value, */
            debug(F010,"delmac def",q[z],0);
            if (x != 0)                 /* Macro arg vector is just a copy */
              free(q[z]);               /* Others are real so free them */
            q[z] = NULL;
            if (x == 0) {               /* Arg vector array */
                if (z >= 0 && z <= 9) { /* Copy values to corresponding  */
                    if (maclvl < 0) {   /* \%1..9 variables. */
                        makestr(&(toparg[z]),NULL);
                    } else {
                        makestr(&(m_arg[maclvl][z]),NULL);
                    }
                }
            }
            if (tra_asg) traceval(nam,NULL);
        } else debug(F010,"delmac def","(null pointer)",0);
    }

   /* Not a variable or an array, so it must be a macro. */

    z = isaa(nam);
    debug(F111,"delmac isaa",nam,z);
    debug(F111,"delmac exact",nam,exact);
    x = z ? mxxlook(mactab,nam,nmac) :
      exact ? mxlook(mactab,nam,nmac) :
        mlook(mactab,nam,nmac);
    if (x < 0) {
        debug(F111,"delmac mlook",nam,x);
        return(x);
    }
    return(xdelmac(x));
}

VOID
initmac() {                             /* Init macro & variable tables */
    int i, j, x;

    nmac = 0;                           /* No macros */
    for (i = 0; i < MAC_MAX; i++) {     /* Initialize the macro table */
        mactab[i].kwd = NULL;
        mactab[i].mval = NULL;
        mactab[i].flgs = 0;
    }
    mtchanged++;
    x = (MAXARGLIST + 1) * sizeof(char **);
    for (i = 0; i < MACLEVEL; i++) {    /* Init the macro argument tables */
        m_xarg[i] = (char **) malloc(x);
        mrval[i] = NULL;                /* Macro return value */
        /* Pointer to entire argument vector, level i, for \&_[] array */
        for (j = 0; j <= MAXARGLIST; j++) { /* Macro argument list */
            if (j < 10)                 /* For the \%0..\%9 variables */
              m_arg[i][j] = NULL;       /* Pointer to arg j, level i. */
            if (m_xarg[i])              /* For \&_[] - all args. */
              m_xarg[i][j] = NULL;
        }
    }
    for (i = 0; i < GVARS; i++) {       /* And the global variables table */
        g_var[i] = NULL;
    }
    /* And the table of arrays */
    for (i = 0; i < (int) 'z' - ARRAYBASE + 1; i++) {
        a_ptr[i] = (char **) NULL;      /* Null pointer for each */
        a_dim[i] = 0;                   /* and a dimension of zero */
        a_link[i] = -1;
        for (j = 0; j < CMDSTKL; j++) {
            aa_ptr[j][i] = (char **) NULL;
            aa_dim[j][i] = 0;
        }
    }
}

int
popclvl() {                             /* Pop command level, return cmdlvl */
    extern int tra_cmd;
    struct localvar * v;
    int i, topcmd;
    debug(F101,"popclvl cmdlvl","",cmdlvl);
    if (cmdlvl > 0) {
        if ((v = localhead[cmdlvl])) {  /* Did we save any variables? */
            while (v) {                 /* Yes */
                if (v->lv_value)        /* Copy old ones back */
                  addmac(v->lv_name,v->lv_value);
                else
                  delmac(v->lv_name,1);
                v = v->lv_next;
            }
            freelocal(cmdlvl);          /* Free local storage */
        }
        /* Automatic arrays do not use the localhead list */

        for (i = 0; i < 28; i++) {      /* Free any local arrays */
            if (aa_ptr[cmdlvl][i]) {	/* Does this one exist? */
                dclarray((char)(i+ARRAYBASE),-1); /* Destroy global one */
                a_ptr[i] = aa_ptr[cmdlvl][i];
                a_dim[i] = aa_dim[cmdlvl][i];
                aa_ptr[cmdlvl][i] = (char **)NULL;
                aa_dim[cmdlvl][i] = 0;
            } else if (aa_dim[cmdlvl][i] == -23) { /* Secret code */
                dclarray((char)(i+ARRAYBASE),-1); /* (see pusharray()) */
                aa_ptr[cmdlvl][i] = (char **)NULL;
                aa_dim[cmdlvl][i] = 0;
            }

            /* Otherwise do nothing - it is a local array that was declared */
            /* at a level above this one so leave it alone. */
        }
    }
    if (cmdlvl < 1) {                   /* If we're already at top level */
        cmdlvl = 0;                     /* just make sure all the */
        tlevel = -1;                    /* stack pointers are set right */
        maclvl = -1;                    /* and return */
    } else if (cmdstk[cmdlvl].src == CMD_TF) { /* Reading from TAKE file? */
        debug(F101,"popclvl tlevel","",tlevel);
        if (tlevel > -1) {              /* Yes, */
            fclose(tfile[tlevel]);      /* close it */

            if (tra_cmd)
              printf("[%d] -F: \"%s\"\n",cmdlvl,tfnam[tlevel]);
            debug(F111,"CMD -F",tfnam[tlevel],cmdlvl);
            if (tfnam[tlevel]) {        /* free storage for name */
                free(tfnam[tlevel]);
                tfnam[tlevel] = NULL;
            }
            tlevel--;                   /* and pop take level */
            cmdlvl--;                   /* and command level */
            quiet = xquiet[cmdlvl];
            vareval = xvarev[cmdlvl];
        } else
          tlevel = -1;
    } else if (cmdstk[cmdlvl].src == CMD_MD) { /* In a macro? */
        topcmd = lastcmd[maclvl];
        debug(F101,"popclvl maclvl","",maclvl);
        if (maclvl > -1) {              /* Yes, */
#ifdef COMMENT
            int i;
            char **q;
#endif /* COMMENT */
            macp[maclvl] = "";          /* set macro pointer to null string */
            *cmdbuf = '\0';             /* clear the command buffer */

            if ((maclvl > 0) &&         /* 2 May 1999 */
                (m_arg[maclvl-1][0]) &&
                (!strncmp(m_arg[maclvl-1][0],"_xif",4) ||
                 !strncmp(m_arg[maclvl-1][0],"_for",4) ||
                 !strncmp(m_arg[maclvl-1][0],"_swi",4) ||
                 !strncmp(m_arg[maclvl-1][0],"_whi",4)) &&
                mrval[maclvl+1]) {
                makestr(&(mrval[maclvl-1]),mrval[maclvl+1]);
            }
            if (maclvl+1 < MACLEVEL) {
                if (mrval[maclvl+1]) {  /* Free any deeper return values. */
                    free(mrval[maclvl+1]);
                    mrval[maclvl+1] = NULL;
                }
            }
            if (tra_cmd)
              printf("[%d] -M: \"%s\"\n",cmdlvl,m_arg[cmdstk[cmdlvl].lvl][0]);
            debug(F111,"CMD -M",m_arg[cmdstk[cmdlvl].lvl][0],cmdlvl);

            maclvl--;                   /* Pop macro level */
            cmdlvl--;                   /* and command level */
            debug(F101,"popclvl mac new maclvl","",maclvl);
            debug(F010,"popclvl mac mrval[maclvl+1]",mrval[maclvl+2],0);

            quiet = xquiet[cmdlvl];
	    vareval = xvarev[cmdlvl];
            if (maclvl > -1) {
                a_ptr[0] = m_xarg[maclvl];
                a_dim[0] = n_xarg[maclvl] - 1;
		debug(F111,"a_dim[0]","B",a_dim[0]);
            } else {
                a_ptr[0] = topxarg;
                a_dim[0] = topargc - 1;
		debug(F111,"a_dim[0]","C",a_dim[0]);
            }
        } else {
            maclvl = -1;
        }
#ifndef NOSEXP
        debug(F101,"popclvl topcmd","",topcmd);
        if (topcmd == XXSEXP) {
            extern char * sexpval;
            makestr(&(mrval[maclvl+1]),sexpval);
        }
#endif /* NOSEXP */
    } else {
        cmdlvl--;
    }
    debug(F101,"popclvl cmdlvl","",cmdlvl);
    if (prstring[cmdlvl]) {
        cmsetp(prstring[cmdlvl]);
        makestr(&(prstring[cmdlvl]),NULL);
    }
#ifndef MAC
    if (cmdlvl < 1 || xcmdsrc == CMD_KB) { /* If at prompt */
        setint();
        concb((char)escape);            /* Go into cbreak mode */
    }
#endif /* MAC */
    xcmdsrc = cmdstk[cmdlvl].src;
    debug(F101,"popclvl xcmdsrc","",xcmdsrc);
    debug(F101,"popclvl tlevel","",tlevel);
    return(cmdlvl < 1 ? 0 : cmdlvl);    /* Return command level */
}
#else /* No script programming language */
int popclvl() {                         /* Just close current take file. */
    if (tlevel > -1) {                  /* if any... */
        if (tfnam[tlevel]) {
            free(tfnam[tlevel]);
            tfnam[tlevel] = NULL;
        }
        fclose(tfile[tlevel--]);
    }
    if (tlevel == -1) {                 /* And if back at top level */
        setint();
        concb((char)escape);            /* and go back into cbreak mode. */
    }
    xcmdsrc = tlevel > -1 ? CMD_TF : 0;
    return(tlevel + 1);
}
#endif /* NOSPL */


#ifndef NOSPL
static int
iseom(m) char * m; {                    /* Test if at end of macro def */
    if (!m)
      m = "";
    debug(F111,"iseom",m,maclvl);
    while (*m) {
        /* Anything but Space and Comma means more macro is left */
        if ((*m > SP) && (*m != ',')) {
            debug(F111,"iseom return",m,0);
            return(0);
        }
        m++;
    }
    debug(F111,"iseom return",m,1);
    return(1);                          /* Nothing left */
}
#endif /* NOSPL */

/* Pop all command levels that can be popped */

int
prepop() {
    if (cmdlvl > 0) {                   /* If command level is > 0 and... */
        while (
#ifndef NOSPL
               ((cmdstk[cmdlvl].src == CMD_TF) && /* Command source is file */
#endif /* NOSPL */
            (tlevel > -1) &&
            feof(tfile[tlevel]))        /* And at end of file... */
#ifndef NOSPL
               /* Or command source is macro... */
               || ((cmdstk[cmdlvl].src == CMD_MD) &&
                (maclvl > -1) &&
                iseom(macp[maclvl])))  /* and at end of macro, then... */
#endif /* NOSPL */
        {
              popclvl();                /* pop command level. */
        }
    }
    return(cmdlvl < 1 ? 0 : cmdlvl);    /* Return command level */
}

/* STOP - get back to C-Kermit prompt, no matter where from. */

int
dostop() {
    extern int cmddep;
    while (popclvl()) ;         /* Pop all macros & take files */
#ifndef NOSPL
    if (cmddep > -1)            /* And all recursive cmd pkg invocations */
      while (cmpop() > -1) ;
#endif /* NOSPL */
    cmini(ckxech);              /* Clear the command buffer. */
    return(0);
}

/* Close the given log */

int
doclslog(x) int x; {
    int y;
    switch (x) {
#ifdef DEBUG
      case LOGD:
        if (deblog <= 0) {
            printf("?Debugging log wasn't open\n");
            return(0);
        }
        debug(F100,"Debug Log Closed","",0L);
        *debfil = '\0';
        deblog = 0;
        return(zclose(ZDFILE));
#endif /* DEBUG */

#ifndef NOXFER
      case LOGP:
        if (pktlog <= 0) {
            printf("?Packet log wasn't open\n");
            return(0);
        }
        *pktfil = '\0';
        pktlog = 0;
        return(zclose(ZPFILE));
#endif /* NOXFER */

#ifndef NOLOCAL
      case LOGS:
        if (seslog <= 0) {
            printf("?Session log wasn't open\n");
            return(0);
        }
        *sesfil = '\0';
        setseslog(0);
        return(zclose(ZSFILE));
#endif /* NOLOCAL */

#ifdef TLOG
      case LOGT: {
#ifdef IKSD
          extern int iklogopen, xferlog;
#endif /* IKSD */
          if (tralog <= 0
#ifdef IKSD
              && !iklogopen
#endif /* IKSD */
              ) {
              if (msgflg)
                printf("?Transaction log wasn't open\n");
              return(0);
          }
#ifdef IKSD
          if (iklogopen && !inserver) {
              close(xferlog);
              iklogopen = 0;
          }
#endif /* IKSD */
          if (tralog) {
              tlog(F100,"Transaction Log Closed","",0L);
              zclose(ZTFILE);
          }
          *trafil = '\0';
          tralog = 0;
          return(1);
      }
#endif /* TLOG */

#ifdef CKLOGDIAL
      case LOGM:
        if (dialog <= 0) {
            if (msgflg) printf("?Connection log wasn't open\n");
            return(0);
        }
        *diafil = '\0';
        dialog = 0;
        return(zclose(ZDIFIL));
#endif /* CKLOGDIAL */

#ifndef NOSPL
      case LOGW:                        /* WRITE file */
      case LOGR:                        /* READ file */
        y = (x == LOGR) ? ZRFILE : ZWFILE;
        if (chkfn(y) < 1)               /* If no file to close */
          return(1);                    /* succeed silently. */
        return(zclose(y));              /* Otherwise, close the file. */
#endif /* NOSPL */

      default:
        printf("\n?Unexpected log designator - %d\n", x);
        return(0);
    }
}

static int slc = 0;                     /* Screen line count */

char *
showstring(s) char * s; {
    return(s ? s : "(null)");
}

char *
showoff(x) int x; {
    return(x ? "on" : "off");
}

char *
showooa(x) int x; {
    switch (x) {
      case SET_OFF:  return("off");
      case SET_ON:   return("on");
      case SET_AUTO: return("automatic");
      default:       return("(unknown)");
    }
}

#ifdef GEMDOS
isxdigit(c) int c; {
    return(isdigit(c) ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F'));
}
#endif /* GEMDOS */

#ifndef NOSETKEY
#ifdef OS2
static struct keytab shokeytab[] = {    /* SHOW KEY modes */
    "all",    1, 0,
    "one",    0, 0
};
static int nshokey = (sizeof(shokeytab) / sizeof(struct keytab));

#define SHKEYDEF TT_MAX+5
struct keytab shokeymtab[] = {
    "aaa",       TT_AAA,     CM_INV,    /* AnnArbor */
    "adm3a",     TT_ADM3A,   0,         /* LSI ADM-3A */
    "adm5",      TT_ADM5,    0,         /* LSI ADM-5 */
    "aixterm",   TT_AIXTERM, 0,         /* IBM AIXterm */
    "annarbor",  TT_AAA,     0,         /* AnnArbor */
    "ansi-bbs",  TT_ANSI,    0,         /* ANSI.SYS (BBS) */
    "at386",     TT_AT386,   0,         /* Unixware ANSI */
    "avatar/0+", TT_ANSI,    0,         /* AVATAR/0+ */
    "ba80",      TT_BA80,    0,         /* Nixdorf BA80 */
    "be",        TT_BEOS,    CM_INV|CM_ABR,
    "beos-ansi", TT_BEOS,    CM_INV,    /* BeOS ANSI */
    "beterm",    TT_BEOS,    0,         /* BeOS Console */
    "d200",      TT_DG200,   CM_INV|CM_ABR, /* Data General DASHER 200 */
    "d210",      TT_DG210,   CM_INV|CM_ABR, /* Data General DASHER 210 */
    "d217",      TT_DG217,   CM_INV|CM_ABR, /* Data General DASHER 217 */
    "default",   SHKEYDEF,   0,
    "dg200",     TT_DG200,   0,         /* Data General DASHER 200 */
    "dg210",     TT_DG210,   0,         /* Data General DASHER 210 */
    "dg217",     TT_DG217,   0,         /* Data General DASHER 217 */
    "emacs",     TT_KBM_EMACS,   0,     /* Emacs mode */
    "h19",       TT_H19,     CM_INV,    /* Heath-19 */
    "heath19",   TT_H19,     0,         /* Heath-19 */
    "hebrew",    TT_KBM_HEBREW, 0,      /* Hebrew mode */
    "hft",       TT_HFT,     0,         /* IBM HFT */
    "hp2621a",   TT_HP2621,  0,         /* HP 2621A */
    "hpterm",    TT_HPTERM,  0,         /* HP TERM */
    "hz1500",    TT_HZL1500, 0,         /* Hazeltine 1500 */
    "ibm3151",   TT_IBM31,   0,         /* IBM 3101-xx,3161 */
    "linux",     TT_LINUX,   0,         /* Linux */
    "qansi",     TT_QANSI,   0,         /* QNX ANSI */
    "qnx",       TT_QNX,     0,         /* QNX */
    "russian",   TT_KBM_RUSSIAN, 0,     /* Russian mode */
    "scoansi",   TT_SCOANSI, 0,         /* SCO ANSI */
    "sni-97801", TT_97801,   0,         /* Sinix 97801 */
    "sun",       TT_SUN,     0,         /* Sun Console */
#ifdef OS2PM
#ifdef COMMENT
    "tek4014", TT_TEK40, 0,
#endif /* COMMENT */
#endif /* OS2PM */
    "tty",     TT_NONE,  0,
    "tvi910+", TT_TVI910, 0,
    "tvi925",  TT_TVI925, 0,
    "tvi950",  TT_TVI950, 0,
    "vc404",   TT_VC4404, 0,
    "vc4404",  TT_VC4404, CM_INV,
    "vip7809", TT_VIP7809, 0,
    "vt100",   TT_VT100, 0,
    "vt102",   TT_VT102, 0,
    "vt220",   TT_VT220, 0,
    "vt220pc", TT_VT220PC, 0,
    "vt320",   TT_VT320, 0,
    "vt320pc", TT_VT320PC, 0,
    "vt52",    TT_VT52,  0,
    "wp",      TT_KBM_WP, 0,
    "wy160",   TT_WY160,  0,
    "wy30",    TT_WY30,  0,
    "wy370",   TT_WY370, 0,
    "wy50",    TT_WY50,  0,
    "wy60",    TT_WY60,  0,
    "wyse30",  TT_WY30,  CM_INV,
    "wyse370", TT_WY370, CM_INV,
    "wyse50",  TT_WY50,  CM_INV,
    "wyse60",  TT_WY60,  CM_INV
};
int nshokeym = (sizeof(shokeymtab) / sizeof(struct keytab));
#endif /* OS2 */

VOID
#ifdef OS2
shokeycode(c,m) int c, m;
#else
shokeycode(c) int c;
#endif
/* shokeycode */ {
    KEY ch;
    CHAR *s;
#ifdef OS2
    int i;
    con_event km;
#else /* OS2 */
    int km;
#endif /* OS2 */

#ifdef OS2
    extern int mskkeys;
    char * mstr = "";

    if (c >= KMSIZE) {
        bleep(BP_FAIL);
        return;
    }
#else /* OS2 */
    printf(" Key code \\%d => ", c);
#endif /* OS2 */

#ifndef OS2
    km = mapkey(c);

#ifndef NOKVERBS
    if (IS_KVERB(km)) {                 /* \Kverb? */
        int i, kv;
        kv = km & ~(F_KVERB);
        printf("Verb: ");
        for (i = 0; i < nkverbs; i++)
          if (kverbs[i].kwval == kv) {
              printf("\\K%s",kverbs[i].kwd);
              break;
          }
        printf("\n");
    } else
#endif /* NOKVERBS */
      if (IS_CSI(km)) {
          int xkm = km & 0xFF;
          if (xkm <= 32 || xkm >= 127)
            printf("String: \\{27}[\\{%d}\n",xkm);
          else
            printf("String: \\{27}[%c\n",xkm);
      } else if (IS_ESC(km)) {
          int xkm = km & 0xFF;
          if (xkm <= 32 || xkm >= 127)
            printf("String: \\{27}\\{%d}\n",xkm);
          else
            printf("String: \\{27}%c\n",xkm);
      } else if (macrotab[c]) {         /* See if there's a macro */
          printf("String: ");           /* If so, display its definition */
          s = macrotab[c];
          shostrdef(s);
          printf("\n");
#ifndef NOKVERBS
    } else if (km >= 0x100) {           /* This means "undefined" */
        printf("Undefined\n");
#endif /* NOKVERBS */
    } else {                            /* No macro, show single character */
        printf("Character: ");
        ch = km;
        if (ch < 32 || ch == 127
#ifdef OS2
            || ch > 255
#endif /* OS2 */
#ifndef NEXT
#ifndef AUX
#ifndef XENIX
#ifndef OS2
            || (ch > 127 && ch < 160)
#endif /* OS2 */
#endif /* XENIX */
#endif /* AUX */
#endif /* NEXT */
            )
/*
  These used to be %d, but gcc 1.93 & later complain about type mismatches.
  %u is supposed to be totally portable.
*/
          printf("\\%u",(unsigned int) ch);
        else printf("%c \\%u",(CHAR) (ch & 0xff),(unsigned int) ch);
        if (ch == (KEY) c)
          printf(" (self, no translation)\n");
        else
          printf("\n");
    }
#else /* OS2 */
    if (m < 0) {
        km = mapkey(c);
        mstr = "default";
    } else {
        km = maptermkey(c,m);
        for (i = 0; i < nshokeym; i++) {
            if (m == shokeymtab[i].kwval) {
                mstr = shokeymtab[i].kwd;
                break;
            }
        }
    }
    s = keyname(c);
    debug(F111,"shokeycode mstr",mstr,m);
    debug(F111,"shokeycode keyname",s,c);
    printf(" %sKey code \\%d %s (%s) => ",
            mskkeys ? "mskermit " : "",
            mskkeys ? cktomsk(c) : c,
            s == NULL ? "" : s, mstr);

    switch (km.type) {
#ifndef NOKVERBS
      case kverb: {
          int i, kv;
          kv = km.kverb.id & ~(F_KVERB);
          printf("Verb: ");
          for (i = 0; i < nkverbs; i++) {
              if (kverbs[i].kwval == kv) {
                  printf("\\K%s",kverbs[i].kwd);
                  break;
              }
          }
          printf("\n");
          break;
      }
#endif /* NOKVERBS */
      case csi: {
          int xkm = km.csi.key & 0xFF;
          if (xkm <= 32 || xkm >= 127)
            printf("String: \\{27}[\\{%d}\n",xkm);
          else
            printf("String: \\{27}[%c\n",xkm);
          break;
      }
      case esc: {
          int xkm = km.esc.key & 0xFF;
          if (xkm <= 32 || xkm >= 127)
            printf("String: \\{%d}\\{%d}\n",ISDG200(tt_type)?30:27,xkm);
          else
            printf("String: \\{%d}%c\n",ISDG200(tt_type)?30:27,xkm);
          break;
      }
      case macro: {
          printf("String: ");           /* Macro, display its definition */
          shostrdef(km.macro.string);
          printf("\n");
          break;
      }
      case literal: {
          printf("Literal string: ");   /* Literal, display its definition */
          shostrdef(km.literal.string);
          printf("\n");
          break;
      }
      case error: {
          if (c >= 0x100) {
              printf("Undefined\n");
          } else {
              printf("Character: ");
              ch = c;
              if (ch < 32 || ch == 127 || ch > 255
#ifndef NEXT
#ifndef AUX
#ifndef XENIX
#ifndef OS2
                   || (ch > 127 && ch < 160)
#endif /* OS2 */
#endif /* XENIX */
#endif /* AUX */
#endif /* NEXT */
                   )
/*
  These used to be %d, but gcc 1.93 & later complain about type mismatches.
  %u is supposed to be totally portable.
*/
                  printf("\\%u",(unsigned int) ch);
              else printf("%c \\%u",(CHAR) (ch & 0xff),(unsigned int) ch);
              printf(" (self, no translation)\n");
          }
          break;
      }
      case key: {
          printf("Character: ");
          ch = km.key.scancode;
          if (ch < 32 || ch == 127 || ch > 255
#ifndef NEXT
#ifndef AUX
#ifndef XENIX
#ifndef OS2
              || (ch > 127 && ch < 160)
#else
               || (ch > 127)
#endif /* OS2 */
#endif /* XENIX */
#endif /* AUX */
#endif /* NEXT */
              )
/*
  These used to be %d, but gcc 1.93 & later complain about type mismatches.
  %u is supposed to be totally portable.
*/
            printf("\\%u",(unsigned int) ch);
          else printf("%c \\%u",(CHAR) (ch & 0xff),(unsigned int) ch);
          if (ch == (KEY) c)
            printf(" (self, no translation)\n");
          else
            printf("\n");
          break;
      }
    }
#endif /* OS2 */
}
#endif /* NOSETKEY */

VOID
shostrdef(s) CHAR * s; {
    CHAR ch;
    if (!s) s = (CHAR *)"";
    while ((ch = *s++)) {
        if (ch < 32 || ch == 127 || ch == 255
/*
  Systems whose native character sets have graphic characters in C1...
*/
#ifndef NEXT                            /* NeXT */
#ifndef AUX                             /* Macintosh */
#ifndef XENIX                           /* IBM PC */
#ifdef OS2
/*
  It doesn't matter whether the local host can display 8-bit characters;
  they are not portable among character-sets and fonts.  Who knows what
  would be displayed...
*/
            || (ch > 127)
#else /* OS2 */
            || (ch > 127 && ch < 160)
#endif /* OS2 */
#endif /* XENIX */
#endif /* AUX */
#endif /* NEXT */
            )
          printf("\\{%d}",ch);          /* Display control characters */
        else putchar((char) ch);        /* in backslash notation */
    }
}

#define xxdiff(v,sys) strncmp(v,sys,strlen(sys))

#ifndef NOSHOW
VOID
shover() {
#ifdef OS2
    extern char ckxsystem[];
#endif /* OS2 */
    extern char *ck_patch, * cklibv;
    printf("\nVersions:\n %s\n",versio);
    printf(" Numeric: %ld\n",vernum);
#ifdef OS2
    printf(" Operating System: %s\n", ckxsystem);
#else /* OS2 */
    printf(" Built for: %s\n", ckxsys);
#ifdef CK_UTSNAME
    if (unm_nam[0])
      printf(" Running on: %s %s %s %s\n", unm_nam,unm_ver,unm_rel,unm_mch);
#endif /* CK_UTSNAME */
    printf(" Patches: %s\n", *ck_patch ? ck_patch : "(none)");
#endif /* OS2 */
    if (xxdiff(ckxv,ckxsys))
      printf(" %s for%s\n",ckxv,ckxsys);
    else
      printf(" %s\n",ckxv);
    if (xxdiff(ckzv,ckzsys))
      printf(" %s for%s\n",ckzv,ckzsys);
    else
      printf(" %s\n",ckzv);
    printf(" %s\n",cklibv);
    printf(" %s\n",protv);
    printf(" %s\n",fnsv);
    printf(" %s\n %s\n",cmdv,userv);
#ifndef NOCSETS
    printf(" %s\n",xlav);
#endif /* NOCSETS */
#ifndef MAC
#ifndef NOLOCAL
    printf(" %s\n",connv);
#ifdef OS2
    printf(" %s\n",ckyv);
#endif /* OS2 */
#endif /* NOLOCAL */
#endif /* MAC */
#ifndef NODIAL
    printf(" %s\n",dialv);
#endif /* NODIAL */
#ifndef NOSCRIPT
    printf(" %s\n",loginv);
#endif /* NOSCRIPT */
#ifdef NETCONN
    printf(" %s\n",cknetv);
#ifdef OS2
    printf(" %s\n",ckonetv);
#ifdef CK_NETBIOS
    printf(" %s\n",ckonbiv);
#endif /* CK_NETBIOS */
#endif /* OS2 */
#endif /* NETCONN */
#ifdef TNCODE
    printf(" %s\n",cktelv);
#endif /* TNCODE */
#ifdef SSHBUILTIN
    printf(" %s\n",cksshv);
#ifdef SFTP_BUILTIN
    printf(" %s\n",cksftpv);
#endif /* SFTP_BUILTIN */
#endif /* SSHBUILTIN */
#ifdef OS2
#ifdef OS2MOUSE
    printf(" %s\n",ckomouv);
#endif /* OS2MOUSE */
#endif /* OS2 */
#ifdef NEWFTP
    printf(" %s\n",ckftpv);
#endif /* NEWFTP */
#ifdef CK_AUTHENTICATION
    printf(" %s\n",ckathv);
#endif /* CK_AUTHENTICATION */
#ifdef CK_ENCRYPTION
#ifdef CRYPT_DLL
    printf(" %s\n",ck_crypt_dll_version());
#else /* CRYPT_DLL */
    printf(" %s\n",ckcrpv);
#endif /* CRYPT_DLL */
#endif /* CK_ENCRYPTION */
#ifdef CK_SSL
    printf(" %s\n",cksslv);
#endif /* CK_SSL */
    printf("\n");
}

#ifdef CK_LABELED
VOID
sholbl() {
#ifdef VMS
    printf("VMS Labeled File Features:\n");
    printf(" acl %s (ACL info %s)\n",
           showoff(lf_opts & LBL_ACL),
           lf_opts & LBL_ACL ? "preserved" : "discarded");
    printf(" backup-date %s (backup date/time %s)\n",
           showoff(lf_opts & LBL_BCK),
           lf_opts & LBL_BCK ? "preserved" : "discarded");
    printf(" name %s (original filename %s)\n",
           showoff(lf_opts & LBL_NAM),
           lf_opts & LBL_NAM ? "preserved" : "discarded");
    printf(" owner %s (original file owner id %s)\n",
           showoff(lf_opts & LBL_OWN),
           lf_opts & LBL_OWN ? "preserved" : "discarded");
    printf(" path %s (original file's disk:[directory] %s)\n",
           showoff(lf_opts & LBL_PTH),
           lf_opts & LBL_PTH ? "preserved" : "discarded");
#else
#ifdef OS2
    printf("OS/2 Labeled File features (attributes):\n");
    printf(" archive:   %s\n", showoff(lf_opts & LBL_ARC));
    printf(" extended:  %s\n", showoff(lf_opts & LBL_EXT));
    printf(" hidden:    %s\n", showoff(lf_opts & LBL_HID));
    printf(" read-only: %s\n", showoff(lf_opts & LBL_RO ));
    printf(" system:    %s\n", showoff(lf_opts & LBL_SYS));
#endif /* OS2 */
#endif /* VMS */
}
#endif /* CK_LABELED */

VOID
shotcs(csl,csr) int csl, csr; {         /* Show terminal character set */
#ifndef NOCSETS
#ifdef OS2
#ifndef NOTERM
    extern struct _vtG G[4], *GL, *GR;
    extern int decnrcm, sni_chcode;
    extern int tt_utf8, dec_nrc, dec_kbd, dec_lang;
    extern prncs;

    printf(" Terminal character-sets:\n");
    if (IS97801(tt_type_mode)) {
        if (cmask == 0377)
          printf("     Mode: 8-bit Mode\n");
        else
          printf("     Mode: 7-bit Mode\n");
        printf("     CH.CODE is %s\n",sni_chcode?"On":"Off");
    } else if (ISVT100(tt_type_mode)) {
        if (decnrcm)
          printf("     Mode: 7-bit National Mode\n");
        else
          printf("     Mode: 8-bit Multinational Mode\n");
    }
    if ( isunicode() )
        printf("    Local: Unicode display / %s input\n",
                csl == TX_TRANSP ? "transparent" :
                csl == TX_UNDEF ? "undefined" : txrinfo[csl]->keywd);
    else
        printf("    Local: %s\n",
                csl == TX_TRANSP ? "transparent" :
                csl == TX_UNDEF ? "undefined" : txrinfo[csl]->keywd);

    if ( tt_utf8 ) {
        printf("   Remote: UTF-8\n");
    } else {
        printf("   Remote: %sG0: %s (%s)\n",
           GL == &G[0] ? "GL->" : GR == &G[0] ? "GR->" : "    ",
           txrinfo[G[0].designation]->keywd,
           G[0].designation == TX_TRANSP ? "" :
           G[0].size == cs94 ? "94 chars" :
           G[0].size == cs96 ? "96 chars" : "multi-byte");
        printf("           %sG1: %s (%s)\n",
           GL == &G[1] ? "GL->" : GR == &G[1] ? "GR->" : "    ",
           txrinfo[G[1].designation]->keywd,
           G[1].designation == TX_TRANSP ? "" :
           G[1].size == cs94 ? "94 chars" :
           G[1].size == cs96 ? "96 chars" : "multi-byte");
        printf("           %sG2: %s (%s)\n",
           GL == &G[2] ? "GL->" : GR == &G[2] ? "GR->" : "    ",
           txrinfo[G[2].designation]->keywd,
           G[2].designation == TX_TRANSP ? "" :
           G[2].size == cs94 ? "94 chars" :
           G[2].size == cs96 ? "96 chars" : "multi-byte");
        printf("           %sG3: %s (%s)\n",
           GL == &G[3] ? "GL->" : GR == &G[3] ? "GR->" : "    ",
           txrinfo[G[3].designation]->keywd,
           G[3].designation == TX_TRANSP ? "" :
           G[3].size == cs94 ? "94 chars" :
           G[3].size == cs96 ? "96 chars" : "multi-byte");
    }
    printf("\n");
    printf(" Keyboard character-sets:\n");
    printf("   Multinational: %s\n",txrinfo[dec_kbd]->keywd);
    printf("        National: %s\n",txrinfo[dec_nrc]->keywd);
    printf("\n");
    printf(" Printer character-set: %s\n",txrinfo[prncs]->keywd);
#endif /* NOTERM */
#else /* OS2 */
#ifndef MAC
    char *s;

    debug(F101,"TERM LOCAL CSET","",csl);
    debug(F101,"TERM REMOTE CSET","",csr);
    printf(" Terminal character-set: ");
    if (tcs_transp) {                   /* No translation */
        printf("transparent\n");
    } else {                            /* Translation */
        printf("%s (remote) %s (local)\n",
               fcsinfo[csr].keyword,fcsinfo[csl].keyword);
        if (csr != csl) {
            switch(gettcs(csr,csl)) {
              case TC_USASCII:  s = "ascii";        break;
              case TC_1LATIN:   s = "latin1-iso";   break;
              case TC_2LATIN:   s = "latin2-iso";   break;
              case TC_CYRILL:   s = "cyrillic-iso"; break;
              case TC_JEUC:     s = "japanese-euc"; break;
              case TC_HEBREW:   s = "hebrew-iso";   break;
              case TC_GREEK:    s = "greek-iso";    break;
              case TC_9LATIN:   s = "latin9-iso";   break;
              default:          s = "transparent";  break;
            }
            if (strcmp(s,fcsinfo[csl].keyword) &&
                strcmp(s,fcsinfo[csr].keyword))
              printf("                         (via %s)\n",s);
        }
    }
#endif /* MAC */
#endif /* OS2 */
#endif /* NOCSETS */
}

#ifndef NOLOCAL
#ifdef OS2
extern char htab[];
VOID
shotabs() {
    int i,j,k,n;

    printf("Tab Stops:\n\n");
    for (i = 0, j = 1, k = VscrnGetWidth(VCMD); i < MAXTERMCOL; ) {
        do {
            printf("%c",htab[++i]=='T'?'T':'-');
        } while (i % k && i < MAXTERMCOL);
        printf("\n");
        for ( ; j <= i; j++) {
            switch ( j%10 ) {
	      case 1:
                printf("%c",j == 1 ? '1' : '.');
                break;
	      case 2:
	      case 3:
	      case 4:
	      case 5:
	      case 6:
	      case 7:
                printf("%c",'.');
                break;
	      case 8:
                n = (j+2)/100;
                if (n)
		  printf("%d",n);
                else 
		  printf("%c",'.');
                break;
	      case 9:
                n = (j+1)%100/10;
                if (n)
		  printf("%d",n);
                else if (j>90)
		  printf("0");
                else 
		  printf("%c",'.');
                break;
	      case 0:
                printf("0");
                break;
            }
        }
        printf("\n");
    }
#ifdef COMMENT
    for (i = 1; i <= 70; i++)
      printf("%c",htab[i]=='T'?'T':'-');
    printf("\n1.......10........20........30........40........50........60\
........70\n\n");
    for (; i <= 140; i++)
      printf("%c",htab[i]=='T'?'T':'-');
    printf("\n........80........90.......100.......110.......120.......130\
.......140\n\n");
    for (; i <= 210; i++)
      printf("%c",htab[i]=='T'?'T':'-');
    printf("\n.......150.......160.......170.......180.......190.......200\
.......210\n\n");
    for (; i <= 255; i++)
      printf("%c",htab[i]=='T'?'T':'-');
    printf("\n.......220.......230.......240.......250..255\n");
#endif

}
#endif /* OS2 */
#endif /* NOLOCAL */

#ifdef OS2MOUSE
VOID
shomou() {
    int button, event, id, i;
    char * name = "";

    printf("Mouse settings:\n");
    printf("   Button Count:   %d\n",mousebuttoncount());
    printf("   Active:         %s\n\n",showoff(tt_mouse));

    for (button = 0; button < MMBUTTONMAX; button++)
      for (event = 0; event < MMEVENTSIZE; event++)
        if (mousemap[button][event].type != error)
          switch (mousemap[button][event].type) {
            case key:
              printf("   %s = Character: %c \\%d\n",
                     mousename(button,event),
                     mousemap[button][event].key.scancode,
                     mousemap[button][event].key.scancode );
              break;
            case kverb:
              id = mousemap[button][event].kverb.id & ~(F_KVERB);
              if (id != K_IGNORE) {
                  for (i = 0; i< nkverbs; i++)
                    if (id == kverbs[i].kwval) {
                        name = kverbs[i].kwd;
                        break;
                    }
                  printf("   %s = Kverb: \\K%s\n",
                         mousename(button,event),
                         name
                         );
              }
              break;
            case macro:
              printf("   %s = Macro: ",
                     mousename(button,event) );
              shostrdef(mousemap[button][event].macro.string);
              printf("\n");
              break;
          }
}
#endif /* OS2MOUSE */

#ifndef NOLOCAL
VOID
shotrm() {
    char *s;
    extern char * getiact();
    extern int tt_print, adl_err;
#ifndef NOTRIGGER
    extern char * tt_trigger[];
#endif /* NOTRIGGER */
#ifdef CKTIDLE
    extern char * tt_idlesnd_str;
    extern int tt_idlesnd_tmo;
    extern int tt_idlelimit, tt_idleact;
#endif /* CKTIDLE */
#ifdef OS2
    extern int wy_autopage, autoscroll, sgrcolors, colorreset, user_erasemode,
      decscnm, decscnm_usr, tt_diff_upd, tt_senddata,
      wy_blockend, marginbell, marginbellcol, tt_modechg, dgunix;
    int lines = 0;
#ifdef KUI
    extern CKFLOAT tt_linespacing[];
    extern tt_cursor_blink;
#endif /* KUI */
#ifdef PCFONTS
    int i;
    char *font;

    if (IsOS2FullScreen()) {            /* Determine the font name */
        if (!os2LoadPCFonts()) {
            for (i = 0; i < ntermfont; i++) {
                if (tt_font == term_font[i].kwval) {
                    font = term_font[i].kwd;
                    break;
                }
            }
        } else {
            font = "(DLL not available)";
        }
    } else {
        font =     "(full screen only)";
    }
#endif /* PCFONTS */
#ifdef KUI
    char font[64] = "(unknown)";
    if ( ntermfont > 0 ) {
        int i;
        for (i = 0; i < ntermfont; i++) {
            if (tt_font == term_font[i].kwval) {
                ckstrncpy(font,term_font[i].kwd,59);
                ckstrncat(font," ",64);
                ckstrncat(font,ckitoa(tt_font_size/2),64);
                if ( tt_font_size % 2 )
                    ckstrncat(font,".5",64);
                break;
            }
        }
    }
#endif /* KUI */

    printf("Terminal parameters:\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
    printf(" %19s: %1d%-12s  %13s: %1d%-14s\n",
           "Bytesize: Command",
           (cmdmsk == 0377) ? 8 : 7,
           " bits","Terminal",
           (cmask == 0377) ? 8 : 7," bits");
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
    printf(" %19s: %-13s","Type",
           (tt_type >= 0 && tt_type <= max_tt) ?
           tt_info[tt_type].x_name :
           "unknown" );
    if (tt_type >= 0 && tt_type <= max_tt)
      if (strlen(tt_info[tt_type].x_id))
        printf("  %13s: <ESC>%s","ID",tt_info[tt_type].x_id);
    printf("\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
    printf(" %19s: %-13s  %13s: %-15s\n","Echo",
           duplex ? "local" : "remote","Locking-shift",showoff(sosi));
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
    printf(" %19s: %-13s  %13s: %-15s\n","Newline-mode", showoff(tnlm),
	   "Cr-display",tt_crd ? "crlf" : "normal");
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
    printf(" %19s: %-13s  %13s: %-15s\n","Cursor",
#ifdef KUI
	    (tt_cursor == 2) ? (tt_cursor_blink ? 
				 "full (*)" : "full (.)") :
	    (tt_cursor == 1) ? (tt_cursor_blink ? 
				 "half (*)" : "half (.)") :
	    (tt_cursor_blink ? "underline (*)" : "underline (.)"),
#else /* KUI */
           (tt_cursor == 2) ? "full" :
           (tt_cursor == 1) ? "half" : "underline",
#endif /* KUI */
#ifdef CK_AUTODL
           "autodownload",autodl == TAD_ON ?
           (adl_err ? "on, error stop" : "on, error continue") : 
           autodl == TAD_ASK ? 
           (adl_err ? "ask, error stop" : "ask, error continue") :
           "off"
#else /* CK_AUTODL */
           "", ""
#endif /* CK_AUTODL */
           );
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
    printf(" %19s: %-13s  %13s: %-15s\n","Arrow-keys",
           tt_arrow ? "application" : "cursor",
           "Keypad-mode", tt_keypad ? "application" : "numeric"
           );

    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }

    /* Just to make sure we are using current info */
    updanswerbk();

    /*
       This line doesn't end with '\n' because the answerback string
       is terminated with a newline
    */
    printf(" %19s: %-13s  %13s: %-15s","Answerback",
           showoff(tt_answer),"response",answerback);
    switch (tt_bell) {
      case XYB_NONE:
        s = "none";
        break;
      case XYB_VIS:
        s= "visible";
        break;
      case XYB_AUD | XYB_BEEP:
        s="beep";
        break;
      case XYB_AUD | XYB_SYS:
        s="system sounds";
        break;
      default:
        s="(unknown)";
    }
    printf(" %19s: %-13s  %13s: %-15s\n","Bell",s,
           "Wrap",showoff(tt_wrap));
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
    printf(" %19s: %-13s  %13s: %-15s\n","Autopage",showoff(wy_autopage),
           "Autoscroll",showoff(autoscroll));
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
    printf(" %19s: %-13s  %13s: %-15s\n","SGR Colors",showoff(sgrcolors),
           "ESC[0m color",colorreset?"default-color":"current-color");
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
    printf(" %19s: %-13s  %13s: %-15s\n",
            "Erase color",user_erasemode?"default-color":"current-color",
           "Screen mode",decscnm?"reverse":"normal");
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }

    printf(" %19s: %-13d  %13s: %-15d\n","Transmit-timeout",tt_ctstmo,
           "Output-pacing",tt_pacing);
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }

    printf(" %19s: %-13d  %13s: %s\n","Idle-timeout",tt_idlelimit,
           "Idle-action", getiact());

    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
    printf(" %19s: %-13s  %13s: %-15s\n","Send data",
          showoff(tt_senddata),"End of Block", wy_blockend?"crlf/etx":"us/cr");
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
#ifndef NOTRIGGER
    printf(" %19s: %-13s  %13s: %d seconds\n","Auto-exit trigger",
           tt_trigger[0],"Output pacing",tt_pacing );
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
#endif /* NOTRIGGER */
    printf(" %19s: %-13s  %13s: %-15d\n","Margin bell",
           showoff(marginbell),"at column", marginbellcol);
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
    switch (tt_modechg) {
      case TVC_DIS: s = "disabled"; break;
      case TVC_ENA: s = "enabled";  break;
      case TVC_W95: s = "win95-restricted"; break;
      default: s = "(unknown)";
    }
    printf(" %19s: %-13s  %13s: %-15s\n","DG Unix mode",
           showoff(dgunix),"Video change", s);
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }

#ifdef CK_APC
    if (apcstatus == APC_ON) s = "on";
    else if (apcstatus == APC_OFF) s = "off";
    else if (apcstatus == APC_ON|APC_UNCH) s = "unchecked";
    else if (apcstatus == APC_ON|APC_NOINP) s = "no-input";
    else if (apcstatus == APC_ON|APC_UNCH|APC_NOINP) s = "unchecked-no-input";
    printf(" %19s: %-13s  %13s: %-15s\n",
           "APC", s,
#ifdef PCFONTS
           "Font (VGA)",font
#else /* PCFONTS */
#ifdef KUI
           "Font",font
#else
           "Font","(not supported)"
#endif /* KUI */
#endif /* PCFONTS */

           );
#endif /* CK_APC */
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }

#ifdef CK_TTGWSIZ                       /* Console terminal screen size */
    if (tt_cols[VTERM] < 0 || tt_rows[VTERM] < 0)
      ttgwsiz();                        /* Try to get latest size */
#endif /* CK_TTGWSIZ */
    printf(" %19s: %-13d  %13s: %-15d\n","Height",tt_rows[VTERM],
           "Width",tt_cols[VTERM]);
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
#ifdef KUI
    printf(" %19s: %-12f  %14s: %-15d\n","Line spacing",tt_linespacing[VTERM],
           "Display Height",VscrnGetDisplayHeight(VTERM));
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
#endif /* KUI */
    printf(" %19s: %-13s  %13s: %d lines\n","Roll-mode",
          tt_roll[VTERM]?"insert":"overwrite","Scrollback", tt_scrsize[VTERM]);
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }

    if (updmode == tt_updmode)
      if (updmode == TTU_FAST)
        s = "fast (fast)";
      else
        s = "smooth (smooth)";
    else
      if (updmode == TTU_FAST)
        s = "fast (smooth)";
      else
        s = "smooth (fast)";

    printf(" %19s: %-13s  %13s: %d ms\n","Screen-update: mode",s,
           "interval",tt_update);
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
    printf(" %19s: %-13s  %13s: %-15s\n",
           "Screen-optimization",showoff(tt_diff_upd),
           "Status line",showoff(tt_status[VTERM]));
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
    printf(" %19s: %-13s  %13s: %-15s\n","Debug",
           showoff(debses),"Session log", seslog? sesfil : "(none)" );
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }

    /* Display colors (should become SHOW COLORS) */
    {
        USHORT row, col;
        char * colors[16] = {
            "black","blue","green","cyan","red","magenta","brown","lgray",
            "dgray","lblue","lgreen","lcyan","lred","lmagent","yellow","white"
        };
        printf("\n");
        if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }

        printf(" Color:");
#ifndef ONETERMUPD
        GetCurPos(&row, &col);
        WrtCharStrAtt("border",    6, row, 9, &colorborder );
        WrtCharStrAtt("debug",     5, row, 17, &colordebug );
        WrtCharStrAtt("helptext",  8, row, 25, &colorhelp );
        WrtCharStrAtt("reverse",   7, row, 34, &colorreverse );
        WrtCharStrAtt("select",    6, row, 42, &colorselect );
        WrtCharStrAtt("status",    6, row, 50, &colorstatus );
        WrtCharStrAtt("terminal",  8, row, 58, &colornormal );
        WrtCharStrAtt("underline", 9, row, 67, &colorunderline );
#endif /* ONETERMUPD */
        row = VscrnGetCurPos(VCMD)->y+1;
        VscrnWrtCharStrAtt(VCMD, "border",    6, row, 9, &colorborder );
        VscrnWrtCharStrAtt(VCMD, "debug",     5, row, 17, &colordebug );
        VscrnWrtCharStrAtt(VCMD, "helptext",  8, row, 25, &colorhelp );
        VscrnWrtCharStrAtt(VCMD, "reverse",   7, row, 34, &colorreverse );
        VscrnWrtCharStrAtt(VCMD, "select",    6, row, 42, &colorselect );
        VscrnWrtCharStrAtt(VCMD, "status",    6, row, 50, &colorstatus );
        VscrnWrtCharStrAtt(VCMD, "terminal",  8, row, 58, &colornormal );
        VscrnWrtCharStrAtt(VCMD, "underline",  9, row, 67, &colorunderline );
        printf("\n");
        if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }

        /* Foreground color names */
        printf("%6s: %-8s%-8s%-9s%-8s%-8s%-8s%-9s%-9s\n","fore",
                "",
                colors[colordebug&0x0F],
                colors[colorhelp&0x0F],
                colors[colorreverse&0x0F],
                colors[colorselect&0x0F],
                colors[colorstatus&0x0F],
                colors[colornormal&0x0F],
                colors[colorunderline&0x0F]);
        if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }

        /* Background color names */
        printf("%6s: %-8s%-8s%-9s%-8s%-8s%-8s%-9s%-9s\n","back",
                colors[colorborder],
                colors[colordebug>>4],
                colors[colorhelp>>4],
                colors[colorreverse>>4],
                colors[colorselect>>4],
                colors[colorstatus>>4],
                colors[colornormal>>4],
                colors[colorunderline>>4] );
        if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
        printf("\n");
        if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
        printf(" Color:");
#ifndef ONETERMUPD
        GetCurPos(&row, &col);
        WrtCharStrAtt("graphic",   7, row, 9, &colorgraphic );
        WrtCharStrAtt("command",   7, row, 17, &colorcmd );
        WrtCharStrAtt("italic",    6, row, 26, &coloritalic );
#endif /* ONETERMUPD */
        row = VscrnGetCurPos(VCMD)->y+1;
        VscrnWrtCharStrAtt(VCMD, "graphic",   7, row, 9,  &colorgraphic );
        VscrnWrtCharStrAtt(VCMD, "command",   7, row, 17, &colorcmd );
        VscrnWrtCharStrAtt(VCMD, "italic",    6, row, 26, &coloritalic );
        printf("\n");
        if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }

        /* Foreground color names */
        printf("%6s: %-8s%-8s%-8s\n","fore",
                colors[colorgraphic&0x0F],
                colors[colorcmd&0x0F],
                colors[coloritalic&0x0F]);
        if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }

        /* Background color names */
        printf("%6s: %-8s%-8s%-8s\n","back",
                colors[colorgraphic>>4],
                colors[colorcmd>>4],
                colors[coloritalic>>4]);
        if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
    }
    printf("\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
    {
        extern int trueblink, truedim, truereverse, trueunderline, trueitalic;
        printf(
	    " Attribute:  \
blink: %-3s  dim: %-3s  italic: %-3s  reverse: %-3s  underline: %-3s\n",
	    trueblink?"on":"off", truedim?"on":"off", trueitalic?"on":"off", 
	    truereverse?"on":"off", trueunderline?"on":"off");
        if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
    }
    {
        extern vtattrib WPattrib;
        printf(" ASCII Protected chars: %s%s%s%s%s%s%s\n",
                WPattrib.blinking?"blink ":"",
                WPattrib.italic?"italic ":"",
                WPattrib.reversed?"reverse ":"",
                WPattrib.underlined?"underline ":"",
                WPattrib.bold?"bold ":"",
                WPattrib.dim?"dim ":"",
                WPattrib.invisible?"invisible ":"");
        if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
    }

    printf("\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
    (VOID) shoesc(escape);
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
    printf(" See SHOW CHARACTER-SETS for character-set info\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }

#else /* OS2 */   /* Beginning of new non-OS2 version */

    printf("\n");
    printf("Terminal parameters:\n");
    printf(" %19s: %1d%-12s  %13s: %1d%-14s\n",
           "Bytesize: Command",
           (cmdmsk == 0377) ? 8 : 7,
           " bits","Terminal",
           (cmask == 0377) ? 8 : 7," bits");
    s = getenv("TERM");
#ifdef XPRINT
    printf(" %19s: %-13s  %13s: %-15s\n",
           "Type",
           s ? s : "(unknown)",
           "Print",
           showoff(tt_print)
           );
#else
    printf(" %19s: %-13s\n","Type", s ? s : "(unknown)");
#endif /* XPRINT */
    printf(" %19s: %-13s  %13s: %-15s\n","Echo",
           duplex ? "local" : "remote","Locking-shift",showoff(sosi));
    printf(" %19s: %-13s  %13s: %-15s\n","Newline-mode",
           showoff(tnlm),"Cr-display",tt_crd ? "crlf" : "normal");

#ifdef CK_APC
    if (apcstatus == APC_ON) s = "on";
    else if (apcstatus == APC_OFF) s = "off";
    else if (apcstatus == (APC_ON|APC_UNCH)) s = "unchecked";
    else if (apcstatus == (APC_ON|APC_NOINP)) s = "no-input";
    else if (apcstatus == (APC_ON|APC_UNCH|APC_NOINP))
      s = "unchecked-no-input";
    printf(" %19s: %-13s  %13s: %-15s\n",
           "APC", s,
#ifdef CK_AUTODL
           "Autodownload", autodl ?
           (adl_err ? "on, error stop" : "on, error continue") : "off"
#else
           "",""
#endif /* CK_AUTODL */
           );
#endif /* CK_APC */

#ifdef CK_TTGWSIZ                       /* Console terminal screen size */
    ttgwsiz();                          /* Try to get latest size */
    printf(" %19s: %-13d  %13s: %-15d\n","Height",tt_rows, "Width", tt_cols);
#endif /* CK_TTGWSIZ */

    printf(" %19s: %-13s  %13s: %-15s\n","Debug",
           showoff(debses),"Session log", seslog? sesfil : "(none)" );

#ifdef CKTIDLE
    printf(" %19s: %-13d  %13s: %s\n","Idle-timeout",tt_idlelimit,
           "Idle-action", getiact());
#endif /* CKTIDLE */

    printf(" %19s: %-13s  ","Lf-display", tt_lfd ? "crlf" : "normal");
#ifdef UNIX
#ifndef NOJC
    printf("%13s: %-15s","Suspend", showoff(xsuspend));
#endif /* NOJC */
#endif /* UNIX */
    printf("\n");

#ifndef NOTRIGGER
    printf(" %19s: %-13s\n","Trigger",
           tt_trigger[0] ? tt_trigger[0] : "(none)");
#endif /* NOTRIGGER */
    printf("\n");

    (VOID) shoesc(escape);
#ifndef NOCSETS
    shotcs(tcsl,tcsr);          /* Show terminal character sets */
#endif /* NOCSETS */

#endif /* OS2 */
}

VOID
shmdmlin() {                            /* Briefly show modem & line */
#ifndef NODIAL
#ifndef MINIDIAL
#ifdef OLDTBCODE
    extern int tbmodel;
    _PROTOTYP( char * gtbmodel, (void) );
#endif /* OLDTBCODE */
#endif /* MINIDIAL */
#endif /* NODIAL */
    if (local)
#ifdef OS2
      printf(" Port: %s, Modem type: ",ttname);
#else
      printf(" Line: %s, Modem type: ",ttname);
#endif /* OS2 */
    else
      printf(
#ifdef OS2
" Communication device not yet selected with SET PORT\n Modem type: "
#else
" Communication device not yet selected with SET LINE\n Modem type: "
#endif /* OS2 */
             );
#ifndef NODIAL
    printf("%s",gmdmtyp());
#ifndef MINIDIAL
#ifdef OLDTBCODE
    if (tbmodel) printf(" (%s)",gtbmodel()); /* Telebit model info */
#endif /* OLDTBCODE */
#endif /* MINIDIAL */
#else
    printf("(disabled)");
#endif /* NODIAL */
}

#ifdef CK_TAPI
void
shotapi(int option) {
    int rc=0,k ;
    char *s=NULL;
    LPDEVCFG        lpDevCfg = NULL;
    LPCOMMCONFIG    lpCommConfig = NULL;
    LPMODEMSETTINGS lpModemSettings = NULL;
    DCB *           lpDCB = NULL;
    extern struct keytab * tapiloctab;  /* Microsoft TAPI Locations */
    extern int ntapiloc;
    extern struct keytab * tapilinetab; /* Microsoft TAPI Line Devices */
    extern int ntapiline;
    extern int tttapi;                  /* TAPI in use */
    extern int tapipass;                /* TAPI Passthrough mode */
    extern int tapiconv;                /* TAPI Conversion mode */
    extern int tapilights;
    extern int tapipreterm;
    extern int tapipostterm;
    extern int tapimanual;
    extern int tapiinactivity;
    extern int tapibong;
    extern int tapiusecfg;
    extern char tapiloc[];
    extern int tapilocid;
    extern int TAPIAvail;

    if (!TAPIAvail) {
        printf("TAPI Support not enabled\r\n");
        return;
    }
    switch (option) {
      case 0:
        printf("TAPI Settings:\n");
        printf("  Line:                      %s\n",
               tttapi ? ttname : "(none in use)");

        cktapiBuildLocationTable(&tapiloctab, &ntapiloc);
        if (tapilocid == -1)
          tapilocid = cktapiGetCurrentLocationID();

        /* Find the current tapiloc entry */
        /* and use it as the default. */
        for (k = 0; k < ntapiloc; k++) {
            if (tapiloctab[k].kwval == tapilocid)
              break;
        }
        if (k >= 0 && k < ntapiloc)
          s = tapiloctab[k].kwd;
        else
          s = "(unknown)";
        printf("  Location:                  %s\n",s);
        printf("  Modem-dialing:             %s\n",tapipass?"off":"on");
        printf("  Phone-number-conversions:  %s\n",
                tapiconv==CK_ON?"on":tapiconv==CK_AUTO?"auto":"off");
        printf("  Modem-lights:              %s %s\n",tapilights?"on ":"off",
                tapipass?"(n/a)":"");
        printf("  Predial-terminal:          %s %s\n",tapipreterm?"on ":"off",
                tapipass?"(n/a)":"");
        printf("  Postdial-terminal:         %s %s\n",tapipostterm?"on ":"off",
                tapipass?"(n/a)":"");
        printf("  Manual-dial:               %s %s\n",tapimanual?"on ":"off",
                tapipass?"(n/a)":"");
        printf("  Inactivity-timeout:        %d seconds %s\n",tapiinactivity,
                tapipass?"(n/a)":"");
        printf("  Wait-for-bong:             %d seconds %s\n",tapibong,
                tapipass?"(n/a)":"");
        printf("  Use-windows-configuration: %s %s\n",
                tapiusecfg?"on ":"off", tapipass?"(n/a)":"");
        printf("\n");

#ifdef BETATEST
        if (tapipass) {
printf("K-95 uses the TAPI Line in an exclusive mode.  Other applications\n");
printf("may open the device but may not place calls nor answer calls.\n");
printf("Dialing is performed using the K-95 dialing procedures.  SET MODEM\n");
printf("TYPE TAPI after the SET TAPI LINE command to activate the modem\n");
printf("definition associated with the active TAPI LINE device.\n\n");

        } else {

printf("K-95 uses the TAPI Line in a cooperative mode.  Other applications\n");
printf("may open the device, place and answer calls.  Dialing is performed\n");
printf("by TAPI.  K-95 SET MODEM commands are not used.\n\n");
        }

        if (tapiconv == CK_ON ||
            tapiconv == CK_AUTO && !tapipass) {
printf(
"Phone numbers are converted from canonical to dialable form by TAPI\n");
printf("using the dialing rules specified in the TAPI Dialing Properties\n");
printf("dialog.\n\n");

        } else {

printf(
"Phone numbers are converted from canonical to dialable form by K-95\n");
printf(
"using the dialing rules specified with the SET DIAL commands.  TAPI\n");
printf(
"Dialing Properties are imported automaticly upon startup and whenever\n");
printf("the TAPI Dialing Properties are altered or when the TAPI Location\n");
printf("is changed.\n\n");
        }
#endif /* BETATEST */

        if (tapipass) {
            printf("Type SHOW MODEM to see MODEM configuration.\n");
            if (tapiconv == CK_ON)
              printf("Type SHOW DIAL to see DIAL-related items.\n");
        } else {
            if (tapiconv == CK_ON || tapiconv == CK_AUTO)
              printf("Type SHOW DIAL to see DIAL-related items.\n");
        }
        break;
      case 1:
        cktapiDisplayTapiLocationInfo();
        break;
      case 2:
        rc = cktapiGetModemSettings(&lpDevCfg,&lpModemSettings,
                                     &lpCommConfig,&lpDCB);
        if (rc) {
            cktapiDisplayModemSettings(lpDevCfg,lpModemSettings,
                                         lpCommConfig,lpDCB);
        } else {
            printf("?Unable to retrieve Modem Settings\n");
        }
        break;
      case 3: {
          HANDLE hModem = GetModemHandleFromLine((HLINE)0);
          if (hModem)
            DisplayCommProperties(hModem);
          else
            printf("?Unable to retrieve a valid Modem Handle\n");
          CloseHandle(hModem);
          break;
      }
    }
    printf("\n");
}
#endif /* CK_TAPI */
#endif /* NOLOCAL */

#ifdef PATTERNS
static VOID
shopat() {
    extern char * binpatterns[], * txtpatterns[];
    extern int patterns, filepeek;
    char **p, *s;
    int i, j, k, n, flag, width;
#ifdef CK_TTGWSIZ
    ttgwsiz();                          /* Try to get latest size */
#ifdef OS2
    width = tt_cols[VCMD];
#else /* OS2 */
    width = tt_cols;
#endif /* OS2 */
    if (width < 1)
#endif /* CK_TTGWSIZ */
      width = 80;
    printf("\n");
    printf(" Set file type:            %s\n",gfmode(binary,1));
    printf(" Set file patterns:        %s", showooa(patterns));
#ifdef CK_LABELED
    if (binary == XYFT_L)
      printf(" (but SET FILE TYPE LABELED overrides)\n");
    else
#endif /* CK_LABELED */
#ifdef VMS
    if (binary == XYFT_I)
      printf(" (but SET FILE TYPE IMAGE overrides)\n");
    else
#endif /* VMS */
    if (filepeek)
      printf(" (but SET FILE SCAN ON overrides)\n");
    else
      printf("\n");
    printf(" Maximum patterns allowed: %d\n", FTPATTERNS);
    for (k = 0; k < 2; k++) {           /* For each kind of patter */
        printf("\n");
        if (k == 0) {                   /* binary... */
            printf(" File binary-patterns: ");
            p = binpatterns;
        } else {                        /* text... */
            printf(" File text-patterns:   ");
            p = txtpatterns;
        }
        if (!p[0]) {
            printf("(none)\n");
        } else {
            printf("\n ");
            n = 2;
            for (i = 0; i < FTPATTERNS; i++) { /* For each pattern */
                if (!p[i])              /* Done */
                  break;
                s = p[i];               /* Look for embedded space */
                for (j = 0, flag = 1; *s; s++, j++) /* and also get length */
                  if (*s == SP)
                    flag = 3;
                n += j + flag;          /* Length of this line */
                if (n >= width - 1) {
                    printf("\n ");
                    n = j+2;
                }
                printf(flag == 3 ? " {%s}" : " %s", p[i]);
            }
            if (n > 2)
              printf("\n");
        }
    }
    printf("\n");
}
#endif /* PATTERNS */

#ifndef NOSPL
static VOID
shooutput() {
    printf(" Output pacing:          %d (milliseconds)\n",pacing);
    printf(" Output special-escapes: %s\n", showoff(outesc));
}

static VOID
shoinput() {
#ifdef CKFLOAT
    extern char * inpscale;
#endif	/* CKFLOAT */

#ifdef CK_AUTODL
    printf(" Input autodownload:     %s\n", showoff(inautodl));
#endif /* CK_AUTODL */
    printf(" Input cancellation:     %s\n", showoff(inintr));
    printf(" Input case:             %s\n", inpcas[cmdlvl] ?
           "observe" : "ignore");
    printf(" Input buffer-length:    %d\n", inbufsize);
    printf(" Input echo:             %s\n", showoff(inecho));
    printf(" Input silence:          %d (seconds)\n", insilence);
#ifdef OS2
    printf(" Input terminal:         %s\n", showoff(interm));
#endif /* OS2 */
    printf(" Input timeout:          %s\n", intime[cmdlvl] ?
           "quit" : "proceed");
#ifdef CKFLOAT
    printf(" Input scale-factor:     %s\n", inpscale ? inpscale : "1.0");
#endif	/* CKFLOAT */

    if (instatus < 0)
      printf(" Last INPUT:             -1 (INPUT command not yet given)\n");
    else
      printf(" Last INPUT:             %d (%s)\n", instatus,i_text[instatus]);
}
#endif /* NOSPL */

#ifndef NOSPL
int
showarray() {
#ifdef COMMENT
    char * p, * q, ** ap;
    int i;
#endif /* COMMENT */
    char *s; int x = 0, y;
    int range[2];

    if ((y = cmfld("Array name","",&s,NULL)) < 0)
      if (y != -3)
        return(y);
    ckstrncpy(line,s,LINBUFSIZ);
    s = line;
    if ((y = cmcfm()) < 0)
      return(y);
    if (*s) {
        char ** ap;
        if ((x = arraybounds(s,&(range[0]),&(range[1]))) < 0) {
            printf("?Bad array: %s\n",s);
            return(-9);
        }
        ap = a_ptr[x];
        if (!ap) {
            printf("Array not declared: %s\n", s);
            return(success = 1);
        } else {
            int i, n, max;
            max = (range[1] > 0) ?
              range[1] :
                ((range[0] > 0) ? range[0] : a_dim[x]);
            if (range[0] < 0)
              range[0] = 0;
            if (max > a_dim[x])
              max = a_dim[x];
            n = 1;
            printf("\\&%c[]: Dimension = %d",arrayitoa(x),a_dim[x]);
            if (a_link[x] > -1)
              printf(" (Link to \\&%c[])",arrayitoa(a_link[x]));
            printf("\n");
            for (i = range[0]; i <= max; i++) {
                if (ap[i]) {
                    printf("%3d. %s\n",i,ap[i]);
                    if (xaskmore) {
                        if (cmd_cols > 0) {
                            x = strlen(ap[i]) + 5;
                            y = (x % cmd_cols) ? 1 : 0;
                            n += (x / cmd_cols) + y;
                        } else {
                            n++;
                        }
                        if (n > (cmd_rows - 3)) {
                            if (!askmore())
                              break;
                            else
                              n = 0;
                        }
                    }
                }
            }
        }
        return(1);
    }

    /* All arrays - just show name and dimension */

    for (y = 0; y < (int) 'z' - ARRAYBASE + 1; y++) {
        if (a_ptr[y]) {
            if (x == 0) printf("Declared arrays:\n");
            x = 1;
            printf(" \\&%c[%d]",
                   (y == 1) ? 64 : y + ARRAYBASE, a_dim[y]);
            if (a_link[y] > -1)
              printf(" => \\&%c[]",arrayitoa(a_link[y]));
            printf("\n");
        }
        if (!x) printf(" No arrays declared\n");
    }
    return(1);
}
#endif /* NOSPL */

int
doshow(x) int x; {
    int y, z, i; long zz;
    extern int optlines;
    char *s;
#ifdef OS2
    extern int os2gks;
    extern int tt_kb_mode;
#endif /* OS2 */
    extern int srvcdmsg;
    extern char * cdmsgstr, * ckcdpath;

#ifndef NOSETKEY
    if (x == SHKEY) {                   /* SHOW KEY */
        int c;
#ifdef OS2
        if ((x = cmkey(shokeytab,nshokey,"How many keys should be shown?",
                        "one",xxstring)) < 0) return(x);
        switch (tt_kb_mode) {
          case KBM_EM:
            s = "emacs";
            break;
          case KBM_HE:
            s = "hebrew";
            break;
          case KBM_RU:
            s = "russian";
            break;
          case KBM_EN:
          default:
            s = "default";
            break;
        }
        if ((z = cmkey(shokeymtab,nshokeym,"Which definition should be shown?",
                        s,xxstring)) < 0) return(z);
        if (z == SHKEYDEF)
          z = -1;
#endif /* OS2 */
        if ((y = cmcfm()) < 0) return(y);
#ifdef IKSD
        if (inserver) {
            printf("Sorry, command disabled.\r\n");
            return(success = 0);
        }
#endif /* IKSD */

#ifdef MAC
        printf("Not implemented\n");
        return(0);
#else /* Not MAC */
#ifdef OS2
        if (x) {
            con_event evt;
            for (c = 0; c < KMSIZE; c++) {
                evt = (z < 0) ? mapkey(c) : maptermkey(c,z);
                if (evt.type != error) {
                    shokeycode(c,z);
                }
            }
        } else {
#endif /* OS2 */
            printf(" Press key: ");
#ifdef UNIX
#ifdef NOSETBUF
            fflush(stdout);
#endif /* NOSETBUF */
#endif /* UNIX */
            conbin((char)escape);       /* Put terminal in binary mode */
#ifdef OS2
            os2gks = 0;                 /* Raw scancode processing */
#endif /* OS2 */
            c = congks(0);              /* Get character or scan code */
#ifdef OS2
            os2gks = 1;                 /* Cooked scancode processing */
#endif /* OS2 */
            concb((char)escape);        /* Restore terminal to cbreak mode */
            if (c < 0) {                /* Check for error */
                printf("?Error reading key\n");
                return(0);
            }
#ifndef OS2
/*
  Do NOT mask when it can be a raw scan code, perhaps > 255
*/
            c &= cmdmsk;                /* Apply command mask */
#endif /* OS2 */
            printf("\n");
#ifdef OS2
            shokeycode(c,z);
#else /* OS2 */
            shokeycode(c);
#endif /* OS2 */
#ifdef OS2
        }
#endif /* OS2 */
        return(1);
#endif /* MAC */
    }
#ifndef NOKVERBS
    if (x == SHKVB) {                   /* SHOW KVERBS */
        if ((y = cmcfm()) < 0) return(y);
#ifdef IKSD
        if (inserver) {
            printf("Sorry, command disabled.\r\n");
            return(success = 0);
        }
#endif /* IKSD */
        printf("\nThe following %d keyboard verbs are available:\n\n",nkverbs);
        kwdhelp(kverbs,nkverbs,"","\\K","",3,0);
        printf("\n");
        return(1);
    }
#ifdef OS2
    if (x == SHUDK) {                   /* SHOW UDKs */
        extern void showudk(void);
        if ((y = cmcfm()) < 0) return(y);
#ifdef IKSD
        if (inserver) {
            printf("Sorry, command disabled.\r\n");
            return(success = 0);
        }
#endif /* IKSD */
        showudk();
        return(1);
    }
#endif /* OS2 */
#endif /* NOKVERBS */
#endif /* NOSETKEY */

#ifndef NOSPL
    if (x == SHMAC) {                   /* SHOW MACRO */
        struct FDB kw, fl, cm;
        int i, k, n = 0, left, flag, confirmed = 0;
        char * p, *q[64];
        for (i = 0; i < nmac; i++) {    /* copy the macro table */
            mackey[i].kwd = mactab[i].kwd; /* into a regular keyword table */
            mackey[i].kwval = i;        /* with value = pointer to macro tbl */
            mackey[i].flgs = mactab[i].flgs;
        }
        p = line;
        left = LINBUFSIZ;
        while (!confirmed && n < 64) {
            cmfdbi(&kw,                 /* First FDB - macro table */
                   _CMKEY,              /* fcode */
                   "Macro name",        /* hlpmsg */
                   "",                  /* default */
                   "",                  /* addtl string data */
                   nmac,                /* addtl numeric data 1: tbl size */
                   0,                   /* addtl numeric data 2: 4 = cmswi */
                   xxstring,            /* Processing function */
                   mackey,              /* Keyword table */
                   &fl                  /* Pointer to next FDB */
                   );
            cmfdbi(&fl,                 /* 2nd FDB - something not in mactab */
                   _CMFLD,              /* fcode */
                   "",                  /* hlpmsg */
                   "",                  /* default */
                   "",                  /* addtl string data */
                   0,                   /* addtl numeric data 1 */
                   0,                   /* addtl numeric data 2 */
                   xxstring,
                   NULL,
                   &cm
                   );
            cmfdbi(&cm,                 /* 3rd FDB - Confirmation */
                   _CMCFM,              /* fcode */
                   "",                  /* hlpmsg */
                   "",                  /* default */
                   "",                  /* addtl string data */
                   0,                   /* addtl numeric data 1 */
                   0,                   /* addtl numeric data 2 */
                   NULL,
                   NULL,
                   NULL
                   );
            x = cmfdb(&kw);             /* Parse something */
            if (x < 0)
              return(x);
            s = atmbuf;                 /* What the user typed */
            switch (cmresult.fcode) {
              case _CMKEY:              /* If it was a keyword */
                y = mlook(mactab,atmbuf,nmac); /* get full name */
                if (y > -1)
                  s = mactab[y].kwd;    /* (fall thru on purpose...) */
              case _CMFLD:
                k = ckstrncpy(p,s,left) + 1; /* Copy result to list */
                left -= k;
                if (left <= 0) {
                    *p = NUL;
                    break;
                }
                q[n++] = p;             /* Point to this item */
                p += k;                 /* Move buffer pointer past it */
                break;
              case _CMCFM:              /* End of command */
                confirmed++;
              default:
                break;
            }
        }
        if (n == 0) {
            printf("Macros:\n");
            slc = 1;
            for (y = 0; y < nmac; y++)
              if (shomac(mactab[y].kwd,mactab[y].mval) < 0) break;
            return(1);
        }
        slc = 0;
        for (i = 0; i < n; i++) {
            flag = 0;
            s = q[i];
            if (!s) s = "";
            if (!*s) continue;
            if (iswild(s)) {            /* Pattern match */
                for (k = 0, x = 0; x < nmac; x++) {
                    if (ckmatch(s,mactab[x].kwd,0,1)) {
                        shomac(mactab[x].kwd,mactab[x].mval);
                        k++;
                    }
                }
                if (!k)
                  x = -1;
                else
                  continue;
            } else {                    /* Exact match */
                x = mxlook(mactab,s,nmac);
                flag = 1;
            }
            if (flag && x == -1)
              x = mlook(mactab,s,nmac);
            switch (x) {
              case -3:                  /* Nothing to look up */
              case -1:                  /* Not found */
                printf("%s - (not defined)\n",s);
                break;
              case -2:                  /* Ambiguous, matches more than one */
                printf("%s - ambiguous\n",s);
                break;
              default:                  /* Matches one exactly */
                shomac(mactab[x].kwd,mactab[x].mval);
                break;
            }
        }
        return(1);
    }
#endif /* NOSPL */

/*
  Other SHOW commands only have two fields.  Get command confirmation here,
  then handle with big switch() statement.
*/
#ifndef NOSPL
    if (x != SHBUI && x != SHARR)
#endif /* NOSPL */
      if ((y = cmcfm()) < 0)
        return(y);

#ifdef COMMENT
    /* This restriction is too general. */
#ifdef IKSD
    if (inserver &&
#ifdef CK_LOGIN
        isguest
#else
        0
#endif /* CK_LOGIN */
        ) {
        printf("Sorry, command disabled.\r\n");
        return(success = 0);
    }
#endif /* IKSD */
#endif /* COMMENT */

    switch (x) {

#ifdef ANYX25
#ifndef IBMX25
      case SHPAD:
        shopad(0);
        break;
#endif /* IBMX25 */
#endif /* ANYX25 */

      case SHNET:
#ifdef NOLOCAL
        printf(" No network support in this version of C-Kermit.\n");
#else
#ifndef NETCONN
        printf(" No network support in this version of C-Kermit.\n");
#else
        shonet();
#endif /* NETCONN */
#endif /* NOLOCAL */
        break;

      case SHPAR:
        shopar();
        break;

#ifndef NOXFER
      case SHATT:
        shoatt();
        break;
#endif /* NOXFER */

#ifndef NOSPL
      case SHCOU:
        printf(" %d\n",count[cmdlvl]);
        break;
#endif /* NOSPL */

#ifndef NOSERVER
      case SHSER:                       /* Show Server */
        i = 0;
#ifndef NOFRILLS
        printf("Function:          Status:\n");
        i++;
        printf(" GET                %s\n",nm[en_get]);
        i++;
        printf(" SEND               %s\n",nm[en_sen]);
        i++;
        printf(" MAIL               %s\n",nm[inserver ? 0 : en_mai]);
        i++;
        printf(" PRINT              %s\n",nm[inserver ? 0 : en_pri]);
        i++;
#ifndef NOSPL
        printf(" REMOTE ASSIGN      %s\n",nm[en_asg]);
        i++;
#endif /* NOSPL */
        printf(" REMOTE CD/CWD      %s\n",nm[en_cwd]);
        i++;
#ifdef ZCOPY
        printf(" REMOTE COPY        %s\n",nm[en_cpy]);
        i++;
#endif /* ZCOPY */
        printf(" REMOTE DELETE      %s\n",nm[en_del]);
        printf(" REMOTE DIRECTORY   %s\n",nm[en_dir]);
        printf(" REMOTE HOST        %s\n",nm[inserver ? 0 : en_hos]);
        i += 3;
#ifndef NOSPL
        printf(" REMOTE QUERY       %s\n",nm[en_que]);
        i++;
#endif /* NOSPL */
        printf(" REMOTE MKDIR       %s\n",nm[en_mkd]);
        printf(" REMOTE RMDIR       %s\n",nm[en_rmd]);
        printf(" REMOTE RENAME      %s\n",nm[en_ren]);
        printf(" REMOTE SET         %s\n",nm[en_set]);
        printf(" REMOTE SPACE       %s\n",nm[en_spa]);
        printf(" REMOTE TYPE        %s\n",nm[en_typ]);
        printf(" REMOTE WHO         %s\n",nm[inserver ? 0 : en_who]);
        printf(" BYE                %s\n",nm[en_bye]);
        printf(" FINISH             %s\n",nm[en_fin]);
        printf(" EXIT               %s\n",nm[en_xit]);
        printf(" ENABLE             %s\n",nm[en_ena]);
        i += 11;
#endif /* NOFRILLS */
        if (i > cmd_rows - 3) { if (!askmore()) return(1); else i = 0; }
        printf("Server timeout:      %d\n",srvtim);
        if (++i > cmd_rows - 3) { if (!askmore()) return(1); else i = 0; }
        printf("Server idle-timeout: %d\n",srvidl);
        if (++i > cmd_rows - 3) { if (!askmore()) return(1); else i = 0; }
        printf("Server keepalive     %s\n", showoff(srvping));
        if (++i > cmd_rows - 3) { if (!askmore()) return(1); else i = 0; }
        printf("Server cd-message    %s\n", showoff(srvcdmsg));
        if (srvcdmsg && cdmsgstr)
          printf("Server cd-message    %s\n", cdmsgstr);
        if (++i > cmd_rows - 3) { if (!askmore()) return(1); else i = 0; }
        printf("Server display:      %s\n", showoff(srvdis));
        if (++i > cmd_rows - 3) { if (!askmore()) return(1); else i = 0; }
        printf("Server login:        ");
        if (!x_user) {
            printf("(none)\n");
        } else {
            printf("\"%s\", \"%s\", \"%s\"\n",
                   x_user,
                   x_passwd ? x_passwd : "",
                   x_acct ? x_acct : ""
                   );
        }
        if (++i > cmd_rows - 3) { if (!askmore()) return(1); else i = 0; }
        printf("Server get-path: ");
        if (ngetpath == 0) {
            printf("    (none)\n");
        } else {
            printf("\n");
            i += 3;
            for (x = 0; x < ngetpath; x++) {
                if (getpath[x]) printf(" %d. %s\n", x, getpath[x]);
                if (++i > (cmd_rows - 3)) { /* More than a screenful... */
                    if (!askmore())
                      break;
                    else
                      i = 0;
                }
            }
        }
        break;
#endif /* NOSERVER */

      case SHSTA:                       /* Status of last command */
        printf(" %s\n", success ? "SUCCESS" : "FAILURE");
        return(0);                      /* Don't change it */

      case SHSTK: {                     /* Stack for MAC debugging */
#ifdef MAC
          long sp;
          sp = -1;
          loadA0 ((char *)&sp);         /* set destination address */
          SPtoaA0();                    /* move SP to destination */
          printf("Stack at 0x%x\n", sp);
          show_queue();                 /* more debugging */
          break;
#else
          shostack();
#endif /* MAC */
          break;
      }


#ifndef NOLOCAL
#ifdef OS2
      case SHTAB:                       /* SHOW TABS */
#ifdef IKSD
        if (inserver) {
            printf("Sorry, command disabled.\r\n");
            return(success = 0);
        }
#endif /* IKSD */
        shotabs();
        break;
#endif /* OS2 */

      case SHTER:                       /* SHOW TERMINAL */
#ifdef IKSD
        if (inserver) {
            printf("Sorry, command disabled.\r\n");
            return(success = 0);
        }
#endif /* IKSD */
        shotrm();
        break;

#ifdef OS2
      case SHVSCRN:                     /* SHOW Virtual Screen - for debug */
        shovscrn();
        break;
#endif /* OS2 */
#endif /* NOLOCAL */

#ifdef OS2MOUSE
      case SHMOU:                       /* SHOW MOUSE */
#ifdef IKSD
        if (inserver) {
            printf("Sorry, command disabled.\r\n");
            return(success = 0);
        }
#endif /* IKSD */
        shomou();
        break;
#endif /* OS2MOUSE */

#ifndef NOFRILLS
      case SHVER:
        shover();
        break;
#endif /* NOFRILLS */

#ifndef NOSPL
      case SHBUI:                       /* Built-in variables */
	line[0] = NUL;
        if ((y = cmtxt("Variable name or pattern","",&s,xxstring)) < 0)
          return(y);
        ckstrncpy(line,s,LINBUFSIZ);
        /* if (line[0]) ckstrncat(line,"*",LINBUFSIZ); */

      case SHFUN:                       /* or built-in functions */
#ifdef CK_TTGWSIZ
#ifdef OS2
        if (tt_cols[VTERM] < 0 || tt_rows[VTERM] < 0)
          ttgwsiz();
#else /* OS2 */
        if (ttgwsiz() > 0) {            /* Get current screen size */
            if (tt_rows > 0 && tt_cols > 0) {
                cmd_rows = tt_rows;
                cmd_cols = tt_cols;
            }
        }
#endif /* OS2 */
#endif /* CK_TTGWSIZ */

        if (x == SHFUN) {               /* Functions */
            printf("\nThe following functions are available:\n\n");
            kwdhelp(fnctab,nfuncs,"","\\F","()",3,0);
            printf("\n");
#ifndef NOHELP
            printf(
"HELP FUNCTION <name> gives the calling conventions of the given function.\n\n"
                   );
#endif /* NOHELP */
            break;
        } else {                        /* Variables */
	    int j, flag = 0, havearg = 0;
	    struct stringarray * q = NULL;
	    char ** pp;
	    if (line[0]) {		/* Have something to search for */
		havearg = 1;		/* Maybe a list of things */
		q = cksplit(1,0,line,NULL,"_-^$*?[]{}",0,0,0);
		if (!q) break;
		pp = q->a_head;
	    }
	    i = 0;
	    for (y = 0; y < nvars; y++) {
		if ((vartab[y].flgs & CM_INV))
		  continue;
		if (havearg) {		/* If I have something to match */
		    char * s2;
		    for (flag = 0, j = 1; j <= q->a_size && !flag; j++) {
			s2 = pp[j] ? pp[j] : "";
#ifdef COMMENT
/* This is not needed because it's what the 4 arg does in ckmatch() */
			len = strlen(s2);
			if (len > 0) {
			    if (s2[len-1] != '$') {/* To allow anchors */
				ckmakmsg(line,LINBUFSIZ,pp[j],"*",NULL,NULL);
				s2 = line;
			    }
			}
#endif /* COMMENT */
			if (ckmatch(s2,vartab[y].kwd,0,4) > 0) {
			    flag = 1;	/* Matches */
			    break;
			}
		    }
		    if (!flag)		/* Doesn't match */
		      continue;
		}
		s = nvlook(vartab[y].kwd);
		printf(" \\v(%s) = ",vartab[y].kwd);
		if (vartab[y].kwval == VN_NEWL) { /* \v(newline) */
		    while (*s)		/* Show control chars symbolically */
		      printf("\\{%d}",*s++);
		    printf("\n");
		} else if (vartab[y].kwval == VN_IBUF  || /* \v(input) */
			   vartab[y].kwval == VN_QUE   || /* \v(query) */
#ifdef OS2
			   vartab[y].kwval == VN_SELCT || /* \v(select) */
#endif /* OS2 */
			   (vartab[y].kwval >= VN_M_AAA && /* modem ones */
			    vartab[y].kwval <= VN_M_ZZZ)
			   ) {
		    int r = 12;		/* This one can wrap around */
		    char buf[10];
		    while (*s) {
			if (isprint(*s)) {
			    buf[0] = *s;
			    buf[1] = NUL;
			    r++;
			} else {
			    sprintf(buf,"\\{%d}",*s); /* SAFE */
			    r += (int) strlen(buf);
			}
			if (r >= cmd_cols - 1) {
			    printf("\n");
			    r = 0;
			    i++;
			}
			printf("%s",buf);
			s++;
		    }
		    printf("\n");
		} else
		  printf("%s\n",s);
		if (++i > (cmd_rows - 3)) { /* More than a screenful... */
		    if ((y >= nvars - 1) || !askmore())
		      break;
		    else
		      i = 0;
		}
	    }
        }
        break;

      case SHVAR:                       /* Global variables */
        x = 0;                          /* Variable count */
        slc = 1;                        /* Screen line count for "more?" */
        for (y = 33; y < GVARS; y++)
          if (g_var[y]) {
              if (x++ == 0) printf("Global variables:\n");
              sprintf(line," \\%%%c",y); /* SAFE */
              if (shomac(line,g_var[y]) < 0) break;
          }
        if (!x) printf(" No variables defined\n");
        break;

      case SHARG: {                     /* Args */
          char * s1, * s2;
          if (maclvl > -1) {
              printf("Macro arguments at level %d (\\v(argc) = %d):\n",
                     maclvl,
                     macargc[maclvl]
                     );
              for (y = 0; y < macargc[maclvl]; y++) {
                  s1 = m_arg[maclvl][y];
                  if (!s1) s1 = "(NULL)";
                  s2 = m_xarg[maclvl][y];
                  if (!s2) s2 = "(NULL)";
                  if (y < 10)
                    printf(" \\%%%d = %s\n",y,s1);
                  else
                    printf(" \\&_[%d] = %s\n",y,s2);
              }
          } else {
              printf("Top-level arguments (\\v(argc) = %d):\n", topargc);
              for (y = 0; y < topargc; y++) {
                  s1 = g_var[y + '0'];
                  if (!s1) s1 = "(NULL)";
                  s2 = toparg[y];
                  if (!s2) s2 = "(NULL)";
                  if (y < 10 && g_var[y])
                    printf(" \\%%%d = %s\n",y,s1);
                  if (toparg[y])
                    printf(" \\&_[%d] = %s\n",y,s2);
              }
          }
        }
        break;

      case SHARR:                       /* Arrays */
        return(showarray());
#endif /* NOSPL */

#ifndef NOXFER
      case SHPRO:                       /* Protocol parameters */
        shoparp();
        printf("\n");
        break;
#endif /* NOXFER */

#ifndef NOLOCAL
      case SHCOM:                       /* Communication parameters */
        printf("\n");
        shoparc();
#ifdef OS2
        {
            int i;
            char *s = "(unknown)";
            for (i = 0; i < nprty; i++)
              if (prtytab[i].kwval == priority) {
                  s = prtytab[i].kwd;
                  break;
              }
            printf(" Priority: %s\n", s );
        }
#endif /* OS2 */

        printf("\n");
#ifdef NETCONN
        if (!network
#ifdef IKSD
             && !inserver
#endif /* IKSD */
             ) {
#endif /* NETCONN */
            shomdm();
            printf("\n");
#ifdef NETCONN
        }
#endif /* NETCONN */

#ifndef NODIAL
#ifdef IKSD
        if ( !inserver )
#endif /* IKSD */
        {
            printf("Type SHOW DIAL to see DIAL-related items.\n");
            printf("Type SHOW MODEM to see modem-related items.\n");
#ifdef CK_TAPI
            printf("Type SHOW TAPI to see TAPI-related items.\n");
#endif /* CK_TAPI */
            printf("\n");
        }
#endif /* NODIAL */
        break;
#endif /* NOLOCAL */

      case SHFIL:                       /* File parameters */
        shofil();
        /* printf("\n"); */             /* (out o' space) */
        break;

#ifndef NOCSETS
      case SHLNG:                       /* Languages */
        shoparl();
        break;
#endif /* NOCSETS */

#ifndef NOSPL
      case SHSCR:                       /* Scripts */
        printf(" Command quoting:        %s\n", showoff(cmdgquo()));
        printf(" Take  echo:             %s\n", showoff(techo));
        printf(" Take  error:            %s\n", showoff(takerr[cmdlvl]));
        printf(" Macro echo:             %s\n", showoff(mecho));
        printf(" Macro error:            %s\n", showoff(merror[cmdlvl]));
        printf(" Quiet:                  %s\n", showoff(quiet));
        printf(" Variable evaluation:    %s [\\%%x and \\&x[] variables]\n",
	       vareval ? "recursive" : "simple");
        printf(" Function diagnostics:   %s\n", showoff(fndiags));
        printf(" Function error:         %s\n", showoff(fnerror));
#ifdef CKLEARN
        {
            extern char * learnfile;
            extern int learning;
            if (learnfile) {
                printf(" LEARN file:             %s (%s)\n",
                       learnfile,
                       learning ? "ON" : "OFF"
                       );
            } else
              printf(" LEARN file:             (none)\n");
        }
#endif /* CKLEARN */
        shoinput();
        shooutput();
#ifndef NOSCRIPT
        printf(" Script echo:            %s\n", showoff(secho));
#endif /* NOSCRIPT */
        printf(" Command buffer length:  %d\n", CMDBL);
        printf(" Atom buffer length:     %d\n", ATMBL);
        break;
#endif /* NOSPL */

#ifndef NOXMIT
      case SHXMI:
        printf("\n");
        printf(" File type:                       %s\n",
               binary ? "binary" : "text");
#ifndef NOCSETS
        printf(" File character-set:              %s\n",
               fcsinfo[fcharset].keyword);
#ifdef OS2
        if ( isunicode() ) {
        printf(" Terminal Character (remote):     %s\n",
              tt_utf8 ? "utf-8" : tcsr == TX_TRANSP ? "transparent" :
              tcsr == TX_UNDEF ? "undefined" : txrinfo[tcsr]->keywd);
        printf(" Terminal Character (local):      %s\n",
              tcsl == TX_TRANSP ? "transparent" :
              tcsl == TX_UNDEF ? "undefined" : txrinfo[tcsl]->keywd);
        } else {
        printf(" Terminal Character (remote):     %s\n",
              tt_utf8 ? "utf-8" : tcsr == TX_TRANSP ? "transparent" :
              tcsr == TX_UNDEF ? "undefined" : txrinfo[tcsr]->keywd);
        printf(" Terminal Character (local):      %s\n",
              tcsl == TX_TRANSP ? "transparent" :
              tcsl == TX_UNDEF ? "undefined" : txrinfo[tcsl]->keywd);
        }
#else /* OS2 */
        printf(" Terminal character-set (remote): %s\n",
               fcsinfo[tcsr].keyword);
        printf(" Terminal character-set (local):  %s\n",
               fcsinfo[tcsl].keyword);
#endif /* OS2 */
#endif /* NOCSETS */
        printf(" Terminal bytesize:               %d\n",
               (cmask == 0xff) ? 8 : 7);
        printf(" Terminal echo:                   %s\n",
               duplex ? "local" : "remote");
        printf(" Transmit EOF:                    ");
        if (*xmitbuf == NUL) {
            printf("(none)\n");
        } else {
            char *p;
            p = xmitbuf;
            while (*p) {
                if (*p < SP)
                  printf("^%c",ctl(*p));
                else
                  printf("%c",*p);
                p++;
            }
            printf("\n");
        }
        if (xmitf)
          printf(" Transmit Fill:                   %d\n", xmitf);
        else
          printf(" Transmit Fill:                   (none)\n");
        printf(" Transmit Linefeed:               %s\n",showoff(xmitl));
        if (xmitp)
          printf(" Transmit Prompt:                 %d (%s)\n",
                 xmitp,
                 chartostr(xmitp)
                 );
        else
          printf(" Transmit Prompt:                 (none)\n");
        printf(" Transmit Echo:                   %s\n", showoff(xmitx));
        printf(" Transmit Locking-Shift:          %s\n", showoff(xmits));
        printf(" Transmit Pause:                  %d (millisecond%s)\n",
               xmitw,
               (xmitw == 1) ? "" : "s"
               );
        printf(" Transmit Timeout:                %d (second%s)\n",
               xmitt,
               (xmitt == 1) ? "" : "s"
               );
        printf("\n");
        break;
#endif /* NOXMIT */

#ifndef NODIAL
      case SHMOD:                       /* SHOW MODEM */
#ifdef IKSD
        if (inserver) {
            printf("Sorry, command disabled.\r\n");
            return(success = 0);
        }
#endif /* IKSD */
        shomodem();                     /* Show SET MODEM items */
        break;
#endif /* NODIAL */

#ifndef MAC
      case SHDFLT:
        printf("%s\n",zgtdir());
        break;
#endif /* MAC */

#ifndef NOLOCAL
      case SHESC:
#ifdef IKSD
        if (inserver) {
            printf("Sorry, command disabled.\r\n");
            return(success = 0);
        }
#endif /* IKSD */
        return(shoesc(escape));

#ifndef NODIAL
      case SHDIA:                       /* SHOW DIAL */
#ifdef IKSD
        if (inserver) {
            printf("Sorry, command disabled.\r\n");
            return(success = 0);
        }
#endif /* IKSD */
        shmdmlin();
        printf(", speed: ");
        if ((zz = ttgspd()) < 0) {
            printf("unknown");
        } else {
            if (zz == 8880) printf("75/1200"); else printf("%ld",zz);
        }
        if (carrier == CAR_OFF) s = "off";
        else if (carrier == CAR_ON) s = "on";
        else if (carrier == CAR_AUT) s = "auto";
        else s = "unknown";
        printf(", carrier: %s", s);
        if (carrier == CAR_ON) {
            if (cdtimo) printf(", timeout: %d sec", cdtimo);
            else printf(", timeout: none");
        }
        printf("\n");
        doshodial();
        if (local
#ifdef NETCONN
            && !network
#endif /* NETCONN */
            ) {
            printf("Type SHOW MODEM to see modem settings.\n");
#ifdef CK_TAPI
            printf("Type SHOW TAPI to see TAPI-related items\n");
#endif /* CK_TAPI */
            printf("Type SHOW COMMUNICATIONS to see modem signals.\n");
        }
        break;
#endif /* NODIAL */
#endif /* NOLOCAL */

#ifndef NOXFER
#ifdef CK_LABELED
      case SHLBL:                       /* Labeled file info */
        sholbl();
        break;
#endif /* CK_LABELED */
#endif /* NOXFER */

      case SHCSE:                       /* Character sets */
#ifdef NOCSETS
        printf(
" Character set translation is not supported in this version of C-Kermit\n");
#else
        shocharset();
#ifndef NOXFER
        printf("\n Unknown-Char-Set: %s\n",
               unkcs ? "Keep" : "Discard");
#endif /* NOXFER */
#ifdef OS2
        printf("\n");
#endif /* OS2 */
        shotcs(tcsl,tcsr);
        printf("\n");
#ifdef OS2
        /* PC Code Page information */
        {
            char cpbuf[128];
            int cplist[16], cps;
            int activecp;
            cps = os2getcplist(cplist, sizeof(cplist));

            sprintf(cpbuf,              /* SAFE */
                    "%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d",
                     cps > 1 ? cplist[1] : 0,
                     cps > 2 ? cplist[2] : 0, cps > 3 ? cplist[3] : 0,
                     cps > 4 ? cplist[4] : 0, cps > 5 ? cplist[5] : 0,
                     cps > 6 ? cplist[6] : 0, cps > 7 ? cplist[7] : 0,
                     cps > 8 ? cplist[8] : 0, cps > 9 ? cplist[9] : 0,
                     cps > 10 ? cplist[10] : 0, cps > 11 ? cplist[11] : 0,
                     cps > 12 ? cplist[12] : 0
                     );
            printf(" Code Pages:\n");
            activecp = os2getcp();
            if ( activecp ) {
              printf("     Active: %d\n",activecp);
              if (!isWin95())
                printf("  Available: %s\n",cpbuf);
            } else
              printf("     Active: n/a\n");
            printf("\n");
        }
#endif /* OS2 */
#endif /* NOCSETS */
        break;

      case SHFEA:                       /* Features */
        shofea();
        break;

#ifdef CK_SPEED
      case SHCTL:                       /* Control-Prefix table */
        shoctl();
        break;
#endif /* CK_SPEED */

      case SHEXI: {
          extern int exithangup;
          printf("\n Exit warning %s\n", xitwarn ?
                 (xitwarn == 1 ? "on" : "always") : "off");
          printf(" Exit on-disconnect: %s\n", showoff(exitonclose));
          printf(" Exit hangup: %s\n", showoff(exithangup));
          printf(" Current exit status: %d\n\n", xitsta);
          break;
      }
      case SHPRT: {
#ifdef PRINTSWI
          extern int printtimo, printertype, noprinter;
          extern char * printterm, * printsep;
          extern int prncs;
#ifdef BPRINT
          extern int printbidi;
#endif /* BPRINT */
#endif /* PRINTSWI */

#ifdef IKSD
        if (inserver &&
#ifdef CK_LOGIN
            isguest
#else /* CK_LOGIN */
            0
#endif /* CK_LOGIN */
             ) {
            printf("Sorry, command disabled.\r\n");
            return(success = 0);
        }
#endif /* IKSD */
#ifdef PRINTSWI
          if (noprinter) {
              printf("Printer: (none)\n\n");
              break;
          }
#endif /* PRINTSWI */

          printf("Printer: %s%s\n",

                 printpipe ? "| " : "",
                 printername ? printername :
#ifdef OS2
                 "PRN"
#else
                 "(default)"
#endif /* OS2 */
                 );
#ifdef PRINTSWI
#ifdef BPRINT
          if (printbidi) {
              printf(" /BIDIRECTIONAL\n");
              if (pportspeed > 0)
                printf(" /SPEED:%ld\n",pportspeed);
              printf(" /PARITY:%s\n",parnam((char)pportparity));
              printf(" /FLOW:%s\n",
                     pportflow == FLO_NONE ? "NONE" :
                     (pportflow == FLO_RTSC ? "RTS/CTS" : "XON/XOFF")
                     );
          } else
            printf(" /OUTPUT-ONLY\n");
#endif /* BPRINT */
          switch (printertype) {
            case PRT_NON: printf(" /NONE\n"); break;
            case PRT_FIL: printf(" /FILE\n"); break;
            case PRT_PIP: printf(" /PIPE\n"); break;
            case PRT_DOS: printf(" /DOS-DEVICE\n"); break;
            case PRT_WIN: printf(" /WINDOWS-QUEUE\n"); break;
          }
          printf(" /TIMEOUT:%d\n",printtimo);
          if (printterm) {
              printf(" /END-OF-JOB-STRING:");
              shostrdef(printterm);
              printf("\n");
          } else
            printf(" /END-OF-JOB-STRING:(none)\n");
          printf(" /JOB-HEADER-FILE:%s\n",printsep ? printsep : "(none)");
          printf(" /CHARACTER-SET: %s\n",txrinfo[prncs]->keywd);
#endif /* PRINTSWI */
          printf("\n");
          break;
      }

      case SHCMD: {
#ifdef DOUBLEQUOTING
          extern int dblquo;
#endif /* DOUBLEQUOTING */
#ifdef CK_AUTODL
          printf(" Command autodownload: %s\n",showoff(cmdadl));
#else
          printf(" Command autodownload: (not available)\n");
#endif /* CK_AUTODL */
          printf(" Command bytesize: %d bits\n", (cmdmsk == 0377) ? 8 : 7);
          printf(" Command error-display: %d\n", cmd_err);
#ifdef CK_RECALL
          printf(" Command recall-buffer-size: %d\n",cm_recall);
#else
          printf(" Command recall-buffer not available in this version\n");
#endif /* CK_RECALL */
#ifdef CK_RECALL
          printf(" Command retry: %s\n",showoff(cm_retry));
#else
          printf(" Command retry not available in this version\n");
#endif /* CK_RECALL */
          printf(" Command interruption: %s\n", showoff(cmdint));
          printf(" Command quoting: %s\n", showoff(cmdgquo()));
#ifdef DOUBLEQUOTING
          printf(" Command doublequoting: %s\n", showoff(dblquo));
#endif /* DOUBLEQUOTING */
          printf(" Command more-prompting: %s\n", showoff(xaskmore));
          printf(" Command height: %d\n", cmd_rows);
          printf(" Command width:  %d\n", cmd_cols);
#ifndef IKSDONLY
#ifdef OS2
          printf(" Command statusline: %s\n",showoff(tt_status[VCMD]));
#endif /* OS2 */
#endif /* IKSDONLY */
#ifdef LOCUS
          printf(" Locus:          %s",
                 autolocus ? (autolocus == 2 ? "ask" : "auto") :
		 (locus ? "local" : "remote"));
          if (autolocus)
            printf(" (%s)", locus ? "local" : "remote");
          printf("\n");
#endif /* LOCUS */
          printf(" Hints:          %s\n", showoff(hints));
          printf(" Quiet:          %s\n", showoff(quiet));
          printf(" Maximum command length: %d\n", CMDBL);
#ifndef NOSPL
          {
              char * s;
              int k;
              printf(" Maximum number of macros: %d\n", MAC_MAX);
              printf(" Macros defined: %d\n", nmac);
              printf(" Maximum macro depth: %d\n", MACLEVEL);
              printf(" Maximum TAKE depth: %d\n", MAXTAKE);
              s = "(not defined)";
              k = mlook(mactab,"on_unknown_command",nmac);
              if (k > -1) if (mactab[k].mval) s = mactab[k].mval;
              printf(" ON_UNKNOWN_COMMAND: %s\n",s);
          }
#endif /* NOSPL */
#ifdef UNIX
#ifndef NOJC
          printf(" Suspend: %s\n", showoff(xsuspend));
#endif /* NOJC */
#endif /* UNIX */
          printf(" Access to external commands and programs%s allowed\n",
#ifndef NOPUSH
                 !nopush ? "" :
#endif /* NOPUSH */
                  " not");
          break;
      }

#ifndef NOSPL
      case SHALRM:
        if (ck_alarm)
          printf("Alarm at %s %s\n",alrm_date,alrm_time);
        else
          printf("(no alarm set)\n");
        break;
#endif /* NOSPL */

#ifndef NOMSEND
      case SHSFL: {
          extern struct filelist * filehead;
          if (!filehead) {
              printf("send-list is empty\n");
          } else {
              struct filelist * flp;
              char * s;
              flp = filehead;
              while (flp) {
                  s = flp->fl_alias;
                  if (!s) s = "(none)";
                  printf("%s, mode: %s, alias: %s\n",
                         flp->fl_name,
                         gfmode(flp->fl_mode,0),
                         s
                         );
                  flp = flp->fl_next;
              }
          }
      }
      break;
#endif /* NOMSEND */

#ifdef CKXXCHAR
      case SHDBL:
        shodbl();
        break;
#endif /* CKXXCHAR */

#ifndef NOPUSH
#ifndef NOFRILLS
      case SHEDIT:
        if (!editor[0]) {
            s = getenv("EDITOR");
            if (s) ckstrncpy(editor,s,CKMAXPATH);
        }
        printf("\n editor:  %s\n", editor[0] ? editor : "(none)");
        if (editor[0]) {
            printf(" options: %s\n", editopts[0] ? editopts : "(none)");
            printf(" file:    %s\n", editfile[0] ? editfile : "(none)");
        }
        printf("\n");
        break;

#ifdef BROWSER
      case SHBROWSE:
        if (!browser[0]) {
            s = getenv("BROWSER");
            if (s) ckstrncpy(browser,s,CKMAXPATH);
        }
        printf("\n browser: %s\n", browser[0] ? browser : "(none)");
        if (browser[0]) {
            printf(" options: %s\n", browsopts[0] ? browsopts : "(none)");
            printf(" url:     %s\n", browsurl[0] ? browsurl : "(none)");
        }
        printf("\n");
        break;
#endif /* BROWSER */
#endif /*  NOFRILLS */
#endif /* NOPUSH */

#ifndef NOLOCAL
#ifdef CK_TAPI
      case SHTAPI:                      /* TAPI options */
#ifdef IKSD
        if (inserver) {
            printf("Sorry, command disabled.\r\n");
            return(success = 0);
        }
#endif /* IKSD */
        shotapi(0);
        break;
      case SHTAPI_L:                    /* TAPI Locations */
#ifdef IKSD
        if (inserver) {
            printf("Sorry, command disabled.\r\n");
            return(success = 0);
        }
#endif /* IKSD */
        shotapi(1);
        break;
      case SHTAPI_M:                    /* TAPI Modem */
#ifdef IKSD
        if (inserver) {
            printf("Sorry, command disabled.\r\n");
            return(success = 0);
        }
#endif /* IKSD */
        shotapi(2);
        break;
      case SHTAPI_C:                    /* TAPI Comm */
#ifdef IKSD
        if (inserver) {
            printf("Sorry, command disabled.\r\n");
            return(success = 0);
        }
#endif /* IKSD */
        shotapi(3);
        break;
#endif /* CK_TAPI */

      case SHTCP:                       /* SHOTCP */
        printf("\n");
        shotcp(0);
        printf("\n");
        break;

#ifdef TNCODE
      case SHTEL:                       /* TELNET */
        printf("\n");
        shotel(0);
        printf("\n");
        break;

      case SHTOPT:                      /* TELNET OPTIONS */
        printf("\n");
        shotopt(0);
        printf("\n");
        break;
#endif /* TNCODE */

#ifdef CK_TRIGGER
      case SHTRIG: {
          extern char * tt_trigger[], * triggerval;
          int i;
          if (!tt_trigger[0]) {
              printf(" Triggers: (none)\n");
          } else {
              printf(" Triggers:\n");
              for (i = 0; i < TRIGGERS; i++) {
                  if (!tt_trigger[i])
                    break;
                  printf("  \"%s\"\n",tt_trigger[i]);
              }
              printf(" Most recent trigger encountered: ");
              if (triggerval)
                printf("\"%s\"\n",triggerval);
              else
                printf("(none)\n");
          }
          break;
      }
#endif /* CK_TRIGGER */
#endif /* NOLOCAL */

#ifndef NOSPL
      case SHINP:
        shoinput();
        break;
#endif /* NOSPL */

      case SHLOG: {
#ifndef MAC
#ifdef IKSD
          if (inserver &&
#ifdef CK_LOGIN
              isguest
#else /* CK_LOGIN */
              0
#endif /* CK_LOGIN */
             ) {
            printf("Sorry, command disabled.\r\n");
            return(success = 0);
        }
#endif /* IKSD */
#ifdef DEBUG
          printf("\n Debug log:       %s", deblog ? debfil : "(none)");
	  {
	      extern int debtim;
	      if (debtim) printf(" (timestamps)");
	      printf("\n");
	  }
#endif /* DEBUG */
#ifndef NOXFER
          printf(" Packet log:      %s\n",   pktlog ? pktfil : "(none)");
#endif /* NOXFER */
#ifndef NOLOCAL
          printf(" Session log:     %s",   seslog ? sesfil : "(none)");
	  {
	      extern int sessft, slogts, slognul;
	      switch (sessft) {
		case XYFT_T: printf(" (text)"); break;
		case XYFT_B: printf(" (binary)"); break;
		case XYFT_D: printf(" (debug)"); break;
	      }
	      if (slogts) printf("(timestamped)");
	      if (slognul) printf("(null-padded)");
	      printf("\n");
	  }

#endif /* NOLOCAL */
#ifdef TLOG
          printf(" Transaction log: %s (%s)\n",
                 (tralog ? (*trafil ? trafil : "(none)") : "(none)"),
                 (tlogfmt ? ((tlogfmt == 2) ? "ftp" : "verbose") : "brief")
                 );
#endif /* TLOG */
#ifdef CKLOGDIAL
            printf(" Connection log:  %s\n", dialog ? diafil : "(none)");
#endif /* CKLOGDIAL */
          printf("\n");
#endif /* MAC */
          break;
      }

#ifndef NOSPL
      case SHOUTP:                      /* OUTPUT */
        shooutput();
        break;
#endif /* NOSPL */

#ifdef PATTERNS
      case SHOPAT:                      /* PATTERNS */
        shopat();
        break;
#endif /* PATTERNS */

#ifdef STREAMING
      case SHOSTR: {                    /* STREAMING */
          extern int streamrq, clearrq, cleared;
          extern long tfcps;
          debug(F101,"SHOW RELIABLE reliable","",reliable);
          printf("\n Reliable:     %s\n",showooa(reliable));
          printf(" Clearchannel: %s\n",showooa(clearrq));
          printf(" Streaming:    %s\n\n",showooa(streamrq));
          if ((!local && (streamrq == SET_ON)) ||
              (streamrq == SET_AUTO && reliable))
            printf(" Streaming will be done if requested.\n");
          else if ((streamrq == SET_OFF) ||
                   ((streamrq == SET_AUTO) && !reliable))
            printf(" Streaming will not be requested and will not be done.\n");
          else if ((streamrq == SET_ON) ||
                   ((streamrq == SET_AUTO) && reliable))
            printf(
" Streaming will be requested and will be done if the other Kermit agrees.\n");
          printf(" Last transfer: %sstreaming%s, %ld cps.\n",
                 streamed > 0 ? "" : "no ",
                 cleared ? ", clearchannel" : "",
                 tfcps
                 );
          printf("\n");
          break;
      }
#endif /* STREAMING */

      case SHOIKS:
        return(sho_iks());
        break;

#ifdef CK_AUTHENTICATION
      case SHOAUTH:
        return(sho_auth(0));
#endif /* CK_AUTHENTICATION */

#ifndef NOFTP
      case SHOFTP: {
#ifdef IKSD
        if (inserver) {
            printf("Sorry, command disabled.\r\n");
            return(success = 0);
        }
#endif /* IKSD */
#ifdef SYSFTP
        {
            extern char ftpapp[], ftpopts[];
            printf(" ftp-client:  %s\n", ftpapp[0] ? ftpapp : "(none)");
            if (ftpapp[0])
              printf(" ftp options: %s\n", ftpopts[0] ? ftpopts : "(none)");
        }
#else
#ifdef NEWFTP
        shoftp(0);
#else
        printf("(No FTP client included in this version of Kermit.)\n");
#endif /* NEWFTP */
#endif /* SYSFTP */
        break;
      }
#endif /* NOFTP */

#ifndef NOCMDL
      case SHXOPT: {
#ifdef IKSDB
          extern int dbenabled;
          extern char * dbfile, * dbdir;
#endif /* IKSDB */
#ifdef CKWTMP
          extern int ckxwtmp;
          extern char * wtmpfile;
#endif /* CKWTMP */
#ifdef CK_LOGIN
          extern int ckxanon, xferlog, logintimo;
          extern char * xferfile;
#ifdef UNIX
          extern int ckxpriv;
#endif /* UNIX */
#ifdef CK_PERMS
          extern int ckxperms;
#endif /* CK_PERMS */
#endif /* CK_LOGIN */
          extern char * bannerfile, * helpfile;

#ifdef IKSD
          if (inserver &&
#ifdef CK_LOGIN
              isguest
#else /* CK_LOGIN */
              0
#endif /* CK_LOGIN */
              ) {
              printf("Sorry, command disabled.\r\n");
              return(success = 0);
          }
#endif /* IKSD */
          printf("\n");
          if (!cmdint)
            printf(" --nointerrupts\n");
          printf(" --bannerfile=%s\n",bannerfile ? bannerfile : "(null)");
          printf(" --cdfile:%s\n",cdmsgstr ? cdmsgstr : "(null)");
          printf(" --cdmessage:%d\n",srvcdmsg);
          printf(" --helpfile:%d\n",helpfile);
          if (inserver) {
              printf("\n");
              break;
          }
#ifdef CKSYSLOG
#ifdef SYSLOGLEVEL
          printf(" --syslog:%d (forced)\n",ckxsyslog);
#else
          printf(" --syslog:%d\n",ckxsyslog);
#endif /* SYSLOGLEVEL */
#endif /* CKSYSLOG */
#ifdef CKWTMP
          printf(" --wtmplog:%d\n",ckxwtmp);
          printf(" --wtmpfile=%s\n",wtmpfile ? wtmpfile : "(null)");
#endif /* CKWTMP */
#ifdef IKSD
#ifdef CK_LOGIN
          printf(" --anonymous:%d\n",ckxanon);
#ifdef UNIX
          printf(" --privid:%d\n",ckxpriv);
#endif /* UNIX */
#ifdef CK_PERMS
          printf(" --permission:%04o\n",ckxperms);
#endif /* CK_PERMS */
          printf(" --initfile:%s\n",anonfile ? anonfile : "(null)");
          printf(" --userfile:%s\n",userfile ? userfile : "(null)");
          printf(" --root:%s\n",anonroot ? anonroot : "(null)");
          printf(" --xferlog=%d\n",xferlog);
          printf(" --xferfile=%s\n",xferfile ? xferfile : "(null)");
          printf(" --timeout=%d\n",logintimo);
#endif /* CK_LOGIN */
#ifdef IKSDB
          printf(" --database=%d\n",dbenabled);
          printf(" --dbfile=%s\n",dbfile ? dbfile : "(null)");
          if (dbdir)
            printf("   (db directory=[%s])\n",dbdir);
#endif /* IKSDB */
#ifdef IKSDCONF
          printf(" IKSD conf=%s\n",iksdconf);
#endif /* IKSDCONF */
#endif /* IKSD */
          printf("\n");
          break;
      }
#endif /* NOCMDL */

      case SHCD: {
	  extern char * myhome;
	  s = getenv("CDPATH");
	  if (!s) s = "(none)";
	  printf("\n current directory:  %s\n", zgtdir());
	  printf(" previous directory: %s\n", prevdir ? prevdir : "(none)");
	  printf(" cd home:            %s\n", homepath());
	  printf(" cd path:            %s\n", ckcdpath ? ckcdpath : s);
	  printf(" cd message:         %s\n", showoff(srvcdmsg & 2));
	  printf(" server cd-message:  %s\n", showoff(srvcdmsg & 1));
	  printf(" cd message file:    %s\n\n",cdmsgstr ? cdmsgstr : "(none)");
	  break;
      }
#ifndef NOCSETS
      case SHASSOC:
        (VOID) showassoc();
        break;
#endif /* NOCSETS */

#ifdef CKLOGDIAL
      case SHCONNX:
#ifdef NEWFTP
        if (ftpisconnected()) {
            extern char cxlogbuf[];
            dologshow(W_FTP | 1);
            if (cxlogbuf[0])
              dologshow(1);
        } else {
#endif /* NEWFTP */
            dologshow(1);
#ifdef NEWFTP
        }
#endif /* NEWFTP */
        break;
#endif /* CKLOGDIAL */

      case SHOPTS:
        optlines = 0;
#ifndef NOFRILLS
        (VOID) showdelopts();
#endif /* NOFRILLS */
#ifdef DOMYDIR
        (VOID) showdiropts();
#endif /* DOMYDIR */
#ifdef CKPURGE
        (VOID) showpurgopts();
#endif /* CKPURGE */
        (VOID) showtypopts();
        break;

#ifndef NOLOCAL
      case SHOFLO:
        (VOID) shoflow();
        break;
#endif /* NOLOCAL */

#ifndef NOXFER
      case SHOXFER:
        (VOID) shoxfer();
        break;
#endif /* NOXFER */

#ifdef CK_RECALL
      case SHHISTORY:
        (VOID) cmhistory();
        break;
#endif /* CK_RECALL */

#ifndef NOSEXP
#ifndef NOSPL
      case SHSEXP:
        (VOID) shosexp();
        break;
#endif /* NOSPL */
#endif /* NOSEXP */

#ifdef ANYSSH
      case SHOSSH:
        (VOID) shossh();
        break;
#endif /* ANYSSH */

#ifdef KUI
      case SHOGUI:
        (VOID) shogui();
        break;
#endif /* KUI */

#ifndef NOFRILLS
#ifndef NORENAME
      case SHOREN:
        (VOID) shorename();
        break;
#endif	/* NORENAME */
#endif	/* NOFRILLS */

      default:
        printf("\nNothing to show...\n");
        return(-2);
    }
    return(success = 1);
}

#ifndef NOXFER
int
shoatt() {
    printf("Attributes: %s\n", showoff(atcapr));
    if (!atcapr) return(0);
    printf(" Blocksize: %s\n", showoff(atblki));
    printf(" Date: %s\n", showoff(atdati));
    printf(" Disposition: %s\n", showoff(atdisi));
    printf(" Encoding (Character Set): %s\n", showoff(atenci));
    printf(" Length: %s\n", showoff(atleni));
    printf(" Type (text/binary): %s\n", showoff(attypi));
    printf(" System ID: %s\n", showoff(atsidi));
    printf(" System Info: %s\n", showoff(atsysi));
#ifdef CK_PERMS
    printf(" Permissions In:  %s\n", showoff(atlpri));
    printf(" Permissions Out: %s\n", showoff(atlpro));
#endif /* CK_PERMS */
#ifdef STRATUS
    printf(" Format: %s\n", showoff(atfrmi));
    printf(" Creator: %s\n", showoff(atcrei));
    printf(" Account: %s\n", showoff(atacti));
#endif /* STRATUS */
    return(0);
}
#endif /* NOXFER */

#ifndef NOSPL
int                                     /* SHOW MACROS */
shomac(s1, s2) char *s1, *s2; {
    int x, n, pp;
    pp = 0;                             /* Parenthesis counter */

    debug(F110,"shomac s1",s1,0);
    debug(F110,"shomac s2",s2,0);

#ifdef IKSD
    if ( inserver &&
#ifdef IKSDCONF
        iksdcf
#else /* IKSDCONF */
        1
#endif /* IKSDCONF */
        ) {
        if (!ckstrcmp("on_exit",s1,-1,0) ||
            !ckstrcmp("on_logout",s1,-1,0))
          return(0);
    }
#endif /* IKSD */

    if (!s1)
      return(0);
    else
      printf("%s = ",s1);               /* Print blank line and macro name */
    n = (int)strlen(s1) + 4;            /* Width of current line */
    if (!s2) s2 = "(not defined)";

    while ((x = *s2++)) {               /* Loop thru definition */
        if (x == '(') pp++;             /* Treat commas within parens */
        if (x == ')') pp--;             /* as ordinary text */
        if (pp < 0) pp = 0;             /* Outside parens, */
        if (x == ',' && pp == 0) {      /* comma becomes comma-dash-NL. */
            putchar(',');
            putchar('-');
            x = '\n';
        }
        if (inserver && (x == '\n'))    /* Send CR before LF */
          putchar(CR);
        putchar((CHAR)x);               /* Output the character */
        if (x == '\n') {                /* If it was a newline */
#ifdef UNIX
#ifdef NOSETBUF
            fflush(stdout);
#endif /* NOSETBUF */
#endif /* UNIX */
            putchar(' ');               /* Indent the next line 1 space */
            while(*s2 == ' ') s2++;     /* skip past leading blanks */
            n = 2;                      /* restart the character counter */
            slc++;                      /* and increment the line counter. */
        } else if (++n > (cmd_cols - 1)) { /* If line is too wide */
            putchar('-');               /* output a dash */
            if (inserver)
              putchar(CR);              /* and a carriage return */
            putchar(NL);                /* and a newline */
#ifdef UNIX
#ifdef NOSETBUF
            fflush(stdout);
#endif /* NOSETBUF */
#endif /* UNIX */
            n = 1;                      /* and restart the char counter */
            slc++;                      /* and increment the line counter */
        }
        if (n < 3 && slc > (cmd_rows - 3)) { /* If new line and screen full */
            if (!askmore()) return(-1); /* ask if they want more. */
            n = 1;                      /* They do, start a new line */
            slc = 0;                    /* and restart line counter */
        }
    }
    if (inserver)
      putchar(CR);
    putchar(NL);                        /* End of definition */
    if (++slc > (cmd_rows - 3)) {
        if (!askmore()) return(-1);
        slc = 0;
    }
    return(0);
}
#endif /* NOSPL */
#endif /* NOSHOW */

int x_ifnum = 0;                        /* Flag for IF NUMERIC active */

#ifndef NOSPL
/* Evaluate an arithmetic expression. */
/* Code adapted from ev, by Howie Kaye of Columbia U & others. */

static int xerror = 0;
int divbyzero = 0;
static char *cp;
static CK_OFF_T tokval;
static char curtok;
static CK_OFF_T expval;

#define LONGBITS (8*sizeof (CK_OFF_T))
#define NUMBER 'N'
#define N_EOT 'E'

/*
 Replacement for strchr() and index(), neither of which seem to be universal.
*/

static char *
#ifdef CK_ANSIC
windex(char * s, char c)
#else
windex(s,c) char *s, c;
#endif /* CK_ANSIC */
/* windex */ {
    while (*s != NUL && *s != c) s++;
    if (*s == c) return(s); else return(NULL);
}


/*
 g e t t o k

 Returns the next token.  If token is a NUMBER, sets tokval appropriately.
*/
static char
gettok() {
    char tbuf[80] /* ,*tp */ ;          /* Buffer to accumulate number */

    while (isspace(*cp))                /* Skip past leading spaces */
      cp++;

    debug(F110,"GETTOK",cp,0);

    switch (*cp) {
      case '$':                         /* ??? */
      case '+':                         /* Add */
      case '-':                         /* Subtract or Negate */
      case '@':                         /* Greatest Common Divisor */
      case '*':                         /* Multiply */
      case '/':                         /* Divide */
      case '%':                         /* Modulus */
      case '<':                         /* Left shift */
      case '>':                         /* Right shift */
      case '&':                         /* And */
      case '|':                         /* Or */
      case '#':                         /* Exclusive Or */
      case '~':                         /* Not */
      case '^':                         /* Exponent */
      case '!':                         /* Factorial */
      case '(':                         /* Parens for grouping */
      case ')': return(*cp++);          /* operator, just return it */
      case '\n':
      case '\0': return(N_EOT);         /* End of line, return that */
    }
#ifdef COMMENT
/* This is the original code, which allows only integer numbers. */

    if (isxdigit(*cp)) {                /* Digit, must be a number */
        int radix = 10;                 /* Default radix */
        for (tp = tbuf; isxdigit(*cp); cp++)
          *tp++ = (char) (isupper(*cp) ? tolower(*cp) : *cp);
        *tp = '\0';                     /* End number */
        switch(isupper(*cp) ? tolower(*cp) : *cp) { /* Examine break char */
          case 'h':
          case 'x': radix = 16; cp++; break; /* if radix signifier... */
          case 'o':
          case 'q': radix = 8; cp++; break;
          case 't': radix = 2; cp++; break;
        }
        for (tp = tbuf, tokval = 0; *tp != '\0'; tp++)  {
            int dig;
            dig = *tp - '0';            /* Convert number */
            if (dig > 10) dig -= 'a'-'0'-10;
            if (dig >= radix) {
                if (cmdlvl == 0 && !x_ifnum && !xerror)
                  printf("?Invalid digit '%c' in number\n",*tp);
                xerror = 1;
                return(NUMBER);
            }
            tokval = radix*tokval + dig;
        }
        return(NUMBER);
    }
    if (cmdlvl == 0 && !x_ifnum && !xerror)
      printf("Invalid character '%c' in input\n",*cp);
    xerror = 1;
    cp++;
    return(gettok());
#else
/* This code allows non-numbers to be treated as macro names */
    {
        int i, x;
        char * s, * cp1;
        cp1 = cp;
        tp = tbuf;
        for (i = 0; i < 80; i++) {
            /* Look ahead to next break character */
            /* pretty much anything that is not in the switch() above. */
            if (isalpha(*cp) || isdigit(*cp) ||
                *cp == '_' || *cp == ':' || *cp == '.' ||
                *cp == '[' || *cp == ']' ||
                *cp == '{' || *cp == '}'
                )
              tbuf[i] = *cp++;
            else
              break;
        }
        if (i >= 80) {
            printf("Too long - \"%s\"\n", cp1);
            xerror = 1;
            cp++;
            return(gettok());
        }
        if (xerror) return(NUMBER);

        tbuf[i] = NUL;
        s = tbuf;
        if (!isdigit(tbuf[0])) {
            char * s2 = NULL;
            x = mxlook(mactab,tbuf,nmac);
            debug(F111,"gettok mxlook",tbuf,x);
            if (x < 0) {
                if (cmdlvl == 0 && !x_ifnum && !xerror)
                  printf("Bad number - \"%s\"\n",tbuf);
                xerror = 1;
                cp++;
                return(gettok());
            }
            s2 = mactab[x].mval;
            if (!s2) s2 = "";
            if (*s2) s = s2;
        }
#ifdef CKFLOAT
        x = isfloat(s,0);
#else
        x = chknum(s);
#endif /* CKFLOAT */
        if (x > 0) {
            tokval = ckatofs(s);
        } else {
            if (cmdlvl == 0 && !x_ifnum && !xerror)
              printf("Bad number - \"%s\"\n",tbuf);
            xerror = 1;
            cp++;
            return(gettok());
        }
        return(NUMBER);
    }
#endif /* COMMENT */
}

static CK_OFF_T
#ifdef CK_ANSIC
expon(CK_OFF_T x, CK_OFF_T y)
#else
expon(x,y) CK_OFF_T x,y;
#endif /* CK_ANSIC */
/* expon */ {
    CK_OFF_T result = 1;
    int sign = 1;
    if (y < 0) return(0);
    if (x < 0) {
        x = -x;
        if (y & 1) sign = -1;
    }
    while (y != 0) {
        if (y & 1) result *= x;
        y >>= 1;
        if (y != 0) x *= x;
  }
  return(result * sign);
}

/*
 * factor ::= simple | simple ^ factor
 *
 */
static VOID
factor() {
    CK_OFF_T oldval;
    simple();
    if (curtok == '^') {
        oldval = expval;
        curtok = gettok();
        factor();
        expval = expon(oldval,expval);
    }
}

/*
 * termp ::= NULL | {*,/,%,&} factor termp
 *
 */
static VOID
termp() {
    while (curtok == '*' || curtok == '/' || curtok == '%' || curtok == '&') {
        CK_OFF_T oldval;
        char op;
        op = curtok;
        curtok = gettok();              /* skip past operator */
        oldval = expval;
        factor();
        switch(op) {
          case '*': expval = oldval * expval; break;
          case '/':
          case '%':
            if (expval == 0) {
                if (!x_ifnum)
                  printf("?Divide by zero\n");
                xerror = 1;
                divbyzero = 1;
                expval = -1;
            } else
              expval = (op == '/') ? (oldval / expval) : (oldval % expval);
            break;
          case '&':
            expval = oldval & expval; break;
        }
    }
}

static CK_OFF_T
#ifdef CK_ANSIC
fact(CK_OFF_T x)
#else
fact(x) CK_OFF_T x;
#endif /* CK_ANSIC */
/* fact */ {                            /* factorial */
    CK_OFF_T result = 1;
    while (x > 1)
      result *= x--;
    return(result);
}

/*
 * term ::= factor termp
 *
 */
static VOID
term() {
    factor();
    termp();
}

static CK_OFF_T
#ifdef CK_ANSIC
gcd(CK_OFF_T x, CK_OFF_T y)
#else
gcd(x,y) CK_OFF_T x,y;
#endif /* CK_ANSIC */
/* gcd */ {                             /* Greatest Common Divisor */
    int nshift = 0;
    if (x < 0) x = -x;
    if (y < 0) y = -y;                  /* validate arguments */
    if (x == 0 || y == 0) return(x + y);    /* this is bogus */

    while (!((x & 1) | (y & 1))) {      /* get rid of powers of 2 */
        nshift++;
        x >>= 1;
        y >>= 1;
    }
    while (x != 1 && y != 1 && x != 0 && y != 0) {
        while (!(x & 1)) x >>= 1;       /* eliminate unnecessary */
        while (!(y & 1)) y >>= 1;       /* powers of 2 */
        if (x < y) {                    /* force x to be larger */
            CK_OFF_T t;
            t = x;
            x = y;
            y = t;
        }
        x -= y;
    }
    if (x == 0 || y == 0) return((x + y) << nshift); /* gcd is non-zero one */
    else return((CK_OFF_T) 1 << nshift);    /* else gcd is 1 */
}

/*
 * exprp ::= NULL | {+,-,|,...} term exprp
 *
 */
static VOID
exprp() {
    while (windex("+-|<>#@",curtok) != NULL) {
        CK_OFF_T oldval;
        char op;
        op = curtok;
        curtok = gettok();              /* skip past operator */
        oldval = expval;
        term();
        switch(op) {
          case '+' : expval = oldval + expval; break;
          case '-' : expval = oldval - expval; break;
          case '|' : expval = oldval | expval; break;
          case '#' : expval = oldval ^ expval; break;
          case '@' : expval = gcd(oldval,expval); break;
          case '<' : expval = oldval << expval; break;
          case '>' : expval = oldval >> expval; break;
        }
    }
}

/*
 * expr ::= term exprp
 *
 */
static VOID
expr() {
    term();
    exprp();
}

static CK_OFF_T
xparse() {
    curtok = gettok();
    expr();
#ifdef COMMENT
    if (curtok == '$') {
        curtok = gettok();
        if (curtok != NUMBER) {
            if (cmdlvl == 0 && !x_ifnum)
              printf("?Illegal radix\n");
            xerror = 1;
            return(0);
        }
        curtok = gettok();
    }
#endif /* COMMENT */
    if (curtok != N_EOT) {
        if (cmdlvl == 0 && !x_ifnum && !xerror)
          printf("?Extra characters after expression\n");
        xerror = 1;
    }
    return(expval);
}

char *                                  /* Silent front end for evala() */
evalx(s) char *s; {
    char * p;
    int t;
    t = x_ifnum;
    x_ifnum = 1;
    p = evala(s);
    x_ifnum = t;
    return(p);
}

char *
evala(s) char *s; {
    CK_OFF_T v;				/* Numeric value */
    if (!s) return("");
    xerror = 0;                         /* Start out with no error */
    divbyzero = 0;
    cp = s;                             /* Make the argument global */
    v = xparse();                       /* Parse the string */
    return(xerror ? "" : ckfstoa(v));	/* Return empty string on error */
}

/*
 * simplest ::= NUMBER | ( expr )
 *
 */
static VOID
simplest() {
    char * p;
    p = cp;
    if (curtok == NUMBER)
      expval = tokval;
    else if (curtok == '(') {
        curtok = gettok();              /* skip over paren */
        expr();
        if (curtok != ')') {
            if (cmdlvl == 0 && !x_ifnum && !xerror)
              printf("?Missing right parenthesis\n");
            xerror = 1;
        }
        debug(F110,"GETTOK SIMPLEST ()",p,0);

    } else {
        if (cmdlvl == 0 && !x_ifnum && !xerror)
          printf("?Operator unexpected\n");
        xerror = 1;
    }
    curtok = gettok();
}

/*
 * simpler ::= simplest | simplest !
 *
 */
static VOID
simpler() {
    simplest();
    if (curtok == '!') {
        curtok = gettok();
        expval = fact(expval);
    }
}

/*
 * simple ::= {-,~,!} simpler | simpler
 *
 */

static VOID
simple() {
    if (curtok == '-' || curtok == '~' || curtok == '!' || curtok == '+') {
        int op = curtok;
        curtok = gettok();              /* skip over - sign */
        simpler();                      /* parse the factor again */
        if (op != '+')
          expval = (op == '-') ? -expval : ((op == '!') ? !expval : ~expval);
    } else simpler();
}

/*  D C L A R R A Y  --  Declare an array  */
/*
  Call with:
   char a = single character designator for the array, e.g. "a".
   int  n = size of array.  -1 means to undeclare the array.
  Returns:
   0 or greater on success, having created the requested array with
     with n+1 elements, 0..n.  If an array of the same name existed
     previously, it is destroyed.  The new array has all its elements
     initialized to NULL pointers.  When an array is successfully created,
     the return value is its index (0 = 'a', 1 = 'b', and so on.)
  -1 on failure (because 'a' out of range or malloc failure).
*/
int
#ifdef CK_ANSIC
dclarray(char a, int n)
#else
dclarray(a,n) char a; int n;
#endif /* CK_ANSIC */
/* dclarray */ {
    char c, **p; int i, n2, rc;

    if (a > 63 && a < 91) a += 32;      /* Convert letters to lowercase */
    if (a < ARRAYBASE || a > 122)       /* Verify name */
      return(-1);

    if (n < 0)                          /* Check arg */
      return(-1);
    if (n+1 < 0)                        /* MAXINT+1 wraps around */
      return(-1);

    c = a;
    a -= ARRAYBASE;                     /* Convert name to number */
    rc = a;				/* Array index will be return code */
    if ((p = a_ptr[a]) != NULL) {       /* Delete old array of same name */
        if (a_link[a] > -1) {           /* Is it a link? */
            if (n == 0) {               /* If we're just deleting it */
                a_ptr[a] = (char **) NULL; /* clear all the info. */
                a_dim[a] = 0;
                a_link[a] = -1;
                return(0);
            }                           /* Not deleting */
            a = a_link[a];              /* Switch to linked-to array */
        }
        n2 = a_dim[a];                  /* Real array */
        for (i = 0; i <= n2; i++) {     /* First delete its elements */
            if (p[i]) {
                free(p[i]);
                p[i] = NULL;
            }
        }
        free((char *)a_ptr[a]);         /* Then the element list */
        if (n == 0) {                   /* If undeclaring this array... */
            for (i = 0; i < 122 - ARRAYBASE; i++) { /* Any linked arrays? */
                if (i != a && a_link[i] == a) {     /* Find them */
                    a_ptr[i] = (char **) NULL;      /* and remove them */
                    a_dim[i] = 0;
                    a_link[i] = -1;
                }
            }
        }
        a_ptr[a] = (char **) NULL;      /* Remove pointer to element list */
        a_dim[a] = 0;                   /* Set dimension at zero. */
        a_link[a] = -1;                 /* Unset link word */
    }
    if (n < 0)				/* If only undeclaring, */
      return(0);			/*  we're done. */
    p = (char **) malloc((n+1) * sizeof(char **)); /* Allocate for new array */
    if (p == NULL) return(-1);          /* Check */
    a_ptr[a] = p;                       /* Save pointer to member list */
    a_dim[a] = n;                       /* Save dimension */
    for (i = 0; i <= n; i++)            /* Initialize members to null */
      p[i] = NULL;
    for (i = 0; i < (int) 'z' - ARRAYBASE; i++) { /* Any linked arrays? */
        if (i != a && a_link[i] == a) { /* Find and update them */
            a_ptr[i] = p;
            a_dim[i] = n;
        }
    }
    return(rc);
}

/*  X A R R A Y  -- Convert array name to array index  */

int
xarray(s) char * s; {
    char buf[8];
    int x;
    char c;

    if (!s) s = "";
    debug(F110,"xarray",s,0);
    if (!*s)
      return(-1);
    x = strlen(s);

    buf[0] = NUL;
    buf[1] = NUL;
    buf[2] = s[0];
    buf[3] = (x > 0) ? s[1] : NUL;
    buf[4] = (x > 1) ? s[2] : NUL;
    buf[5] = (x > 2) ? s[3] : NUL;
    buf[6] = NUL;
    debug(F110,"xarray buf[3]",&buf[3],0);
    s = buf+2;
    if (*s == '&') {
        buf[1] = CMDQ;
        s--;
    } else if (*s != CMDQ) {
        buf[0] = CMDQ;
        buf[1] = '&';
        s = buf;
    }
    debug(F110,"xarray s",s,0);
    c = *(s+2);
    if (isupper(c))
      c = tolower(c);
    if (c == '@')
      c = 96;
    x = (int)c - ARRAYBASE;
    if (*(s+3) == '[')
      *(s+3) = NUL;
    if (x < 0) {
	return(-1);
    }
    if (x > ('z' - ARRAYBASE)) {
	debug(F101,"xarray x out of range","",x);
	return(-1);
    }
    if (*(s+3)) {
	debug(F110,"xarray syntax",s,0);
	return(-1);
    }
    return(x);
}

/*
  boundspair() -- parses blah[n:m]

  For use with array segment specifiers and compact substring notation.
  Ignores the "blah" part, gets the values of n and m, which can be
  numbers, variables, or arithmetic expressions; anything that resolves
  to a number.

  Call with:
   s    - string to parse
   sep  - array of permissible bounds separator chars
   lo   - pointer to low-bound result (or -1)
   hi   - pointer to hi-bound result (or -1)
   zz   - pointer to separator char that was encountered (or NUL)
  Returns:
   -1 on failure
    0 on success
*/      

int
#ifdef CK_ANSIC
boundspair(char *s, char *sep, int *lo, int *hi, char *zz)
#else
boundspair(s,sep,lo,hi,zz) char *s, *sep, *zz; int *lo, *hi;
#endif /* CK_ANSIC */
{
    int i, x, y, range[2], bc = 0;
    char c = NUL, *s2 = NULL, buf[256], *p, *q, *r, *e[2], *tmp = NULL;

    debug(F110,"boundspair s",s,0);
    debug(F110,"boundspair sep",sep,0);

    *lo = -1;                           /* Default bounds */
    *hi = -1;
    *zz = 0;				/* Default bounds separator */

    range[0] = -1;                      /* It's OK -- get contents */
    range[1] = -1;                      /* of subscript brackets */
    if (!s) s = "";
    if (!*s)
      return(-1);
    makestr(&tmp,s);                    /* Make a pokeable copy */
    p = tmp;
    q = NULL;
    r = NULL;
    for (p = s; *p; p++) {		/* Get the two elements */
	if (*p == '[') {
	    bc++;			/* Bracket counter */
	    if (bc == 1 && !q) q = p+1;
	} else if (*p == ']') {
	    bc--;
	    if (bc == 0 && q) *p = NUL;
	} else if (bc == 1) {		/* If within brackers */
	    s2 = ckstrchr(sep,*p);	/* Check for separator */
	    if (s2) {
		debug(F000,"boundspair *s2","",*s2);
		if (c) {        
		    debug(F000,"boundspair","Too many separators",*s2);
		    makestr(&tmp,NULL);
		    return(-1);
		}
		c = *s2;		/* Separator character */
		*p = NUL;
		r = p+1;
	    }
	}
    }
    if (bc == 0 && !q) {
	/* This allows such constructions as "show array a" */
	debug(F110,"boundspair","no brackets",0);
	makestr(&tmp,NULL);
	return(0);
    }
    if (bc != 0 || !q) {
	debug(F110,"boundspair","unbalanced or missing brackets",0);
	makestr(&tmp,NULL);
	return(-1);
    }
    if (!q) q = "";
    if (!*q) q = "-1";
    if (!r) r = "";
    if (!*r) r = "-1";

    e[0] = q;
    e[1] = r;

    debug(F000,"boundspair c","",c);
    debug(F110,"boundspair q",q,0);
    debug(F110,"boundspair r",r,0);

    for (i = 0; i < 2 && e[i]; i++) {
	y = 255;			/* Expand variables, etc. */
	s = buf;
	zzstring(e[i],&s,&y);
	s = evalx(buf);			/* Evaluate it arithmetically */
	if (s) if (*s)
	  ckstrncpy(buf,s,256);
	if (!chknum(buf)) {		/* Did we get a number? */
	    debug(F110,"boundspair element not numeric",buf,0);
	    makestr(&tmp,NULL);		/* No, fail. */
	    return(-1);
	}
	range[i] = atoi(buf);
    }
    makestr(&tmp,NULL);                 /* Free temporary poked string */
    *lo = range[0];                     /* Return what we got */
    *hi = range[1];
    *zz = c;
    debug(F101,"boundspair lo","",*lo);
    debug(F101,"boundspair hi","",*hi);
    return(0);
}

/*  A R R A Y B O U N D S  --  Parse array segment notation \&a[n:m]  */

/*
  Call with s = array reference, plus two pointers to ints.
  Returns -1 on error, or array index, with the two ints set as follows:
   \&a[]     -1, -1
   \&a[3]     3, -1
   \&a[3:17]  3, 17
  The array need not be declared -- this routine is just for parsing.
*/
int
arraybounds(s,lo,hi) char * s; int * lo, * hi; {
    int i, x, y, range[2];
    char zz, buf[256], * p, * q;
    char * tmp = NULL;

    *lo = -1;                           /* Default bounds */
    *hi = -1;

    if (!s) s = "";                     /* Defense de null args */
    if (!*s)
      return(-1);

    x = xarray(s);                      /* Check basic structure */
    debug(F111,"arraybounds xarray",s,x);
    if (x < 0)                          /* Not OK, fail. */
      return(-1);
    y = boundspair(s,":",lo,hi,&zz);
    debug(F111,"arraybounds boundspair",s,y);
    debug(F101,"arraybounds lo","",*lo);
    debug(F101,"arraybounds hi","",*hi);
    if (y < 0)				/* Get bounds */
      return(-1);
    return(x);
}

/*  A R R A Y N A M  --  Parse an array name  */

/*
  Call with pointer to string that starts with the array reference.
  String may begin with either \& or just &.
  On success,
    Returns letter ID (always lowercase) in argument c,
      which can also be accent grave (` = 96; '@' is converted to grave);
    Dimension or subscript in argument n;
    IMPORTANT: These arguments must be provided by the caller as addresses
    of ints (not chars), for example:
      char *s; int x, y;
      arraynam(s,&x,&y);
  On failure, returns a negative number, with args n and c set to zero.
*/
int
arraynam(ss,c,n) char *ss; int *c; int *n; {
    int i, y, pp, len;
    char x;
    char *s, *p, *sx, *vnp;
    /* On stack to allow for recursive calls... */
    char vnbuf[ARRAYREFLEN+1];          /* Entire array reference */
    char ssbuf[ARRAYREFLEN+1];          /* Subscript in "plain text" */
    char sxbuf[16];                     /* Evaluated subscript */

    *c = *n = 0;                        /* Initialize return values */
    len = strlen(ss);
    for (pp = 0,i = 0; i < len; i++) {          /* Check length */
        if (ss[i] == '[') {
            pp++;
        } else if (ss[i] == ']') {
            if (--pp == 0)
              break;
        }
    }
    if (i > ARRAYREFLEN) {
        printf("?Array reference too long - %s\n",ss);
        return(-9);
    }
    ckstrncpy(vnbuf,ss,ARRAYREFLEN);
    vnp = vnbuf;
    if (vnbuf[0] == CMDQ && vnbuf[1] == '&') vnp++;
    if (*vnp != '&') {
        printf("?Not an array - %s\n",vnbuf);
        return(-9);
    }
    x = *(vnp + 1);                     /* Fold case of array name */

    /* We don't use isupper & tolower here on purpose because these */
    /* would produce undesired effects with accented letters. */
    if (x > 63 && x < 91) x  = *(vnp + 1) = (char) (x + 32);
    if ((x < ARRAYBASE) || (x > 122) || (*(vnp+2) != '[')) {
        if (msgflg) {
            printf("?Invalid format for array name - %s\n",vnbuf);
            return(-9);
        } else
          return(-2);
    }
    *c = x;                             /* Return the array name */
    s = vnp+3;                          /* Get dimension */
    p = ssbuf;
    pp = 1;                             /* Bracket counter */
    for (i = 0; i < ARRAYREFLEN && *s != NUL; i++) { /* Copy up to ] */
        if (*s == '[') pp++;
        if (*s == ']' && --pp == 0) break;
        *p++ = *s++;
    }
    if (*s != ']') {
        printf("?No closing bracket on array dimension - %s\n",vnbuf);
        return(-9);
    }
    p--;                                /* Trim whitespace from end */
    while (*p == SP || *p == HT)
      p--;
    p++;
    *p = NUL;                           /* Terminate subscript with null */
    p = ssbuf;                          /* Point to beginning of subscript */
    while (*p == SP || *p == HT)        /* Trim whitespace from beginning */
      p++;
    sx = sxbuf;                         /* Where to put expanded subscript */
    y = 16;
    {
	/* Even if VARIABLE-EVALUATION SIMPLE use RECURSIVE for subscripts */
	/* NOTE: This is vulnerable to SIGINT and whatnot... */
	int tmp = vareval;		/* Save VARIABLE-EVALUATION setting */
	vareval = 1;			/* Force it to RECURSIVE */
	zzstring(p,&sx,&y);		/* Convert variables, etc. */
	vareval = tmp;			/* Restore VARIABLE-EVALUATION */
    }
    sx = sxbuf;
    while (*sx == SP) sx++;
    /* debug(F110,"arraynam sx","",sx); */
    if (!*sx) {                         /* Empty brackets... */
        *n = -17;                       /* (Secret code :-) */
        return(-2);
    }
    p = evala(sx);                      /* Run it thru \fneval()... */
    if (p) if (*p) ckstrncpy(sxbuf,p,16); /* We know it has to be a number. */

    if (!chknum(sxbuf)) {               /* Make sure it's all digits */
        if (msgflg) {
            printf("?Array dimension or subscript missing or not numeric\n");
            return(-9);
        } else
          return(-2);
    }
    if ((y = atoi(sxbuf)) < 0) {
        if (cmflgs == 0) printf("\n");
        if (msgflg) {
            printf("?Array dimension or subscript not positive or zero\n");
            return(-9);
        } else
          return(-2);
    }
    *n = y;                             /* Return the subscript or dimension */
    return(0);
}

/* chkarray returns 0 or greater if array exists, negative otherwise */

int
chkarray(a,i) int a, i; {               /* Check if array is declared */
    int x;                              /* and if subscript is in range */
    if (a == 64) a = 96;                /* Convert atsign to grave accent */
    x = a - ARRAYBASE;                  /* Values must be in range 95-122 */
#ifdef COMMENT
    if (x == 0 && maclvl < 0)           /* Macro arg vector but no macro */
      return(0);
#endif /* COMMENT */
    if (x < 0 || x > 'z' - ARRAYBASE)   /* Not in range */
      return(-2);
    if (a_ptr[x] == NULL) return(-1);   /* Not declared */
    if (i > a_dim[x]) return(-2);       /* Declared but out of range. */
    return(a_dim[x]);                   /* All ok, return dimension */
}

#ifdef COMMENT                          /* This isn't used. */
char *
arrayval(a,i) int a, i; {               /* Return value of \&a[i] */
    int x; char **p;                    /* (possibly NULL) */
    if (a == 64) a = 96;                /* Convert atsign to grave accent */
    x = a - ARRAYBASE;                  /* Values must be in range 95-122 */
    if (x < 0 || x > 27) return(NULL);  /* Not in range */
    if ((x > 0) && (p = a_ptr[x]) == NULL) /* Array not declared */
      return(NULL);
    if (i > a_dim[x])                   /* Subscript out of range. */
      return(NULL);
    return(p[i]);                       /* All ok, return pointer to value. */
}
#endif /* COMMENT */

/*
  pusharray() is called when an array name is included in a LOCAL statement.
  It moves the pointers from the global definition to the stack, and removes
  the global definition.  Later, if the same array is declared in the local
  context, it occupies the global definition in the normal way.  But when
  popclvl() is called, it replaces the global definition with the one saved
  here.  The "secret code" is used to indicate to popclv() that it should
  remove the global array when popping through this level -- otherwise if a
  local array were declared that had no counterpart at any higher level, it
  would never be deleted.  This allows Algol-like inheritance to work both
  on the way down and on the way back up.
*/
int
pusharray(x,z) int x, z; {
    int y;
    debug(F000,"pusharray x","",x);
    debug(F101,"pusharray z","",z);
    y = chkarray(x,z);
    debug(F101,"pusharray y","",y);
    x -= ARRAYBASE;                     /* Convert name letter to index. */
    if (x < 0 || x > 27)
      return(-1);
    if (y < 0) {
        aa_ptr[cmdlvl][x] = (char **) NULL;
        aa_dim[cmdlvl][x] = -23;        /* Secret code (see popclvl()) */
    } else {
        aa_ptr[cmdlvl][x] = a_ptr[x];
        aa_dim[cmdlvl][x] = y;
    }
    a_ptr[x] = (char **) NULL;
    a_dim[x] = 0;
    return(0);
}

/*  P A R S E V A R  --  Parse a variable name or array reference.  */
/*
 Call with:
   s  = pointer to candidate variable name or array reference.
   *c = address of integer in which to return variable ID.
   *i = address of integer in which to return array subscript.
 Returns:
   -2:  syntax error in variable name or array reference.
    1:  successful parse of a simple variable, with ID in c.
    2:  successful parse of an array reference, w/ID in c and subscript in i.
*/
int
parsevar(s,c,i) char *s; int *c, *i; {
    char *p;
    int x,y,z;

    p = s;
    if (*s == CMDQ) s++;                /* Point after backslash */

    if (*s != '%' && *s != '&') {       /* Make sure it's % or & */
        printf("?Not a variable name - %s\n",p);
        return(-9);
    }
    if ((int)strlen(s) < 2) {
        printf("?Incomplete variable name - %s\n",p);
        return(-9);
    }
    if (*s == '%' && *(s+2) != '\0') {
        printf("?Only one character after '%%' in variable name, please\n");
        return(-9);
    }
    if (*s == '&' && *(s+2) != '[') {
        printf("?Array subscript expected - %s\n",p);
        return(-9);
    }
    if (*s == '%') {                    /* Simple variable. */
        y = *(s+1);                     /* Get variable ID letter/char */
        if (isupper(y)) y -= ('a'-'A'); /* Convert upper to lower case */
        *c = y;                         /* Set the return values. */
        *i = -1;                        /* No array subscript. */
        return(1);                      /* Return 1 = simple variable */
    }
    if (*s == '&') {                    /* Array reference. */
        y = arraynam(s,&x,&z);          /* Go parse it. */
        debug(F101,"parsevar arraynam","",y);
        if ((y) < 0) {
            if (y == -2)
              return(pusharray(x,z));
            if (y != -9)
              printf("?Invalid array reference - %s\n",p);
            return(-9);
        }
        if (chkarray(x,z) < 0) {        /* Check if declared, etc. */
            printf("?Array not declared or subscript out of range\n");
            return(-9);
        }
        *c = x;                         /* Return array letter */
        *i = z;                         /* and subscript. */
        return(2);
    }
    return(-2);                         /* None of the above. */
}


#define VALN 32

/* Get the numeric value of a variable */
/*
  Call with pointer to variable name, pointer to int for return value.
  Returns:
    0 on success with second arg containing the value.
   -1 on failure (bad variable syntax, variable not defined or not numeric).
*/
int
varval(s,v) char *s; CK_OFF_T *v; {
    char valbuf[VALN+1];                /* s is pointer to variable name */
    char name[256];
    char *p;
    int y;

    if (*s != CMDQ) {                   /* Handle macro names too */
        ckmakmsg(name,256,"\\m(",s,")",NULL);
        s = name;
    }
    p = valbuf;                         /* Expand variable into valbuf. */
    y = VALN;
    if (zzstring(s,&p,&y) < 0) return(-1);
    p = valbuf;                         /* Make sure value is numeric  */
    if (!*p) {                          /* Be nice -- let an undefined */
        valbuf[0] = '0';                /* variable be treated as 0.   */
        valbuf[1] = NUL;
    }
    if (chknum(p)) {                    /* Convert numeric string to int */
        *v = ckatofs(p);		/* OK */
    } else {                            /* Not OK */
        p = evala(p);                   /* Maybe it's an expression */
        if (!chknum(p))                 /* Did it evaluate? */
          return(-1);                   /* No, failure. */
        else                            /* Yes, */
          *v = ckatofs(p);		/* success */
    }
    return(0);
}

/* Increment or decrement a variable */
/* Returns -1 on failure, 0 on success */

int
incvar(s,x,z) char *s; CK_OFF_T x; int z; {  /* Increment a numeric variable */
    CK_OFF_T n;				/* s is pointer to variable name */
                                        /* x is amount to increment by */
                                        /* z != 0 means add */
                                        /* z = 0 means subtract */
    if (varval(s,&n) < 0)               /* Convert numeric string to int */
      return(-1);
    if (z)                              /* Increment it by the given amount */
      n += x;
    else                                /* or decrement as requested. */
      n -= x;
    addmac(s,ckfstoa(n));		/* Replace old variable */
    return(0);
}

/* D O D O  --  Do a macro */

/*
  Call with x = macro table index, s = pointer to arguments.
  Returns 0 on failure, 1 on success.
*/

int
dodo(x,s,flags) int x; char *s; int flags; {
    int y;
    extern int tra_asg, tra_cmd; int tra_tmp;
#ifndef NOLOCAL
#ifdef OS2
    extern int term_io;
    int term_io_sav = term_io;
#endif /* OS2 */
#endif /* NOLOCAL */

    if (x < 0)                          /* It can happen! */
      return(-1);

    tra_tmp = tra_asg;

    if (++maclvl >= MACLEVEL) {         /* Make sure we have storage */
        debug(F101,"dodo maclvl too deep","",maclvl);
        --maclvl;
        printf("Macros nested too deeply\n");
        return(0);
    }
    macp[maclvl] = mactab[x].mval;      /* Point to the macro body */
    macx[maclvl] = mactab[x].mval;      /* Remember where the beginning is */

#ifdef COMMENT
    makestr(&(m_line[maclvl]),s);       /* Entire arg string for "\%*" */
#endif /* COMMENT */

    cmdlvl++;                           /* Entering a new command level */
    if (cmdlvl >= CMDSTKL) {            /* Too many macros + TAKE files? */
        debug(F101,"dodo cmdlvl too deep","",cmdlvl);
        cmdlvl--;
        printf("?TAKE files and DO commands nested too deeply\n");
        return(0);
    }
#ifdef DEBUG
    if (deblog) {
        debug(F111,"CMD +M",mactab[x].kwd,cmdlvl);
        debug(F010,"CMD ->",s,0);
    }
#endif /* DEBUG */

#ifdef VMS
    conres();                           /* So Ctrl-C, etc, will work. */
#endif /* VMS */
#ifndef NOLOCAL
#ifdef OS2
    term_io = 0;                        /* Disable terminal emulator I/O */
#endif /* OS2 */
#endif /* NOLOCAL */
    ifcmd[cmdlvl] = 0;
    iftest[cmdlvl] = 0;
    count[cmdlvl] = count[cmdlvl-1];    /* Inherit COUNT from previous level */
    intime[cmdlvl] = intime[cmdlvl-1];  /* Inherit previous INPUT TIMEOUT */
    inpcas[cmdlvl] = inpcas[cmdlvl-1];  /*   and INPUT CASE */
    takerr[cmdlvl] = takerr[cmdlvl-1];  /*   and TAKE ERROR */
    merror[cmdlvl] = merror[cmdlvl-1];  /*   and MACRO ERROR */
    xquiet[cmdlvl] = quiet;
    xvarev[cmdlvl] = vareval;
    xcmdsrc = CMD_MD;
    cmdstk[cmdlvl].src = CMD_MD;        /* Say we're in a macro */
    cmdstk[cmdlvl].lvl = maclvl;        /* and remember the macro level */
    cmdstk[cmdlvl].ccflgs = flags & ~CF_IMAC; /* Set flags */

    /* Initialize return value except in FOR, WHILE, IF, and SWITCH macros */

    if (!(flags & CF_IMAC) && mrval[maclvl]) {
        free(mrval[maclvl]);
        mrval[maclvl] = NULL;
    }

    /* Clear old %0..%9 arguments */

    addmac("%0",mactab[x].kwd);         /* Define %0 = name of macro */
    makestr(&(m_xarg[maclvl][0]),mactab[x].kwd);
    varnam[0] = '%';
    varnam[2] = '\0';
    tra_asg = 0;
    for (y = 1; y < 10; y++) {          /* Clear args %1..%9 */
        if (m_arg[maclvl][y]) {         /* Don't call delmac() unless */
            varnam[1] = (char) (y + '0'); /* we have to... */
            delmac(varnam,0);
        }
    }
    tra_asg = tra_tmp;

/* Assign the new args one word per arg, allowing braces to group words */

    xwords(s,MAXARGLIST,NULL,0);

#ifndef NOLOCAL
#ifdef OS2
    term_io = term_io_sav;
#endif /* OS2 */
#endif /* NOLOCAL */
    if (tra_cmd)
      printf("[%d] +M: \"%s\"\n",cmdlvl,mactab[x].kwd);
    return(1);
}

/* Insert "literal" quote around each comma-separated command to prevent */
/* its premature expansion.  Only do this if object command is surrounded */
/* by braces. */

static char* flit = "\\flit(";

int
litcmd(src,dest,n) char **src, **dest; int n; {
    int bc = 0, pp = 0;
    char c, *s, *lp, *ss;

    s = *src;
    lp = *dest;

    debug(F010,"litcmd",s,0);

    while (*s == SP) s++;               /* Strip extra leading spaces */

    if (*s == '{') {                    /* Starts with brace */
        pp = 0;                         /* Paren counter */
        bc = 1;                         /* Count leading brace */
        *lp++ = *s++;                   /* Copy it */
        if (--n < 1) return(-1);        /* Check space */
        while (*s == SP) s++;           /* Strip interior leading spaces */
        ss = flit;                      /* Point to "\flit(" */
        while ((*lp++ = *ss++))         /* Copy it */
          if (--n < 1)                  /* and check space */
            return(-1);
        lp--;                           /* Back up over null */

        while (*s) {                    /* Go thru rest of text */
            c = *s;
            if (c == '{') bc++;         /* Count brackets */
            if (c == '(') pp++;         /* and parens */
            if (c == ')') {             /* Right parenthesis. */
                pp--;                   /* Count it. */
                if (pp < 0) {           /* An unbalanced right paren... */
#ifdef COMMENT
/*
  The problem here is that "\{" appears to be a quoted brace and therefore
  isn't counted; then the "}" matches an earlier opening brace, causing
  (e.g.) truncation of macros by getncm().
*/
                    if (n < 5)          /* Out of space in dest buffer? */
                      return(-1);       /* If so, give up. */
                    *lp++ = CMDQ;       /* Must be quoted to prevent */
                    *lp++ = '}';        /* premature termination of */
                    *lp++ = '4';        /* \flit(...) */
                    *lp++ = '1';
                    *lp++ = '}';
                    n -= 5;
#else
/* Here we rely on the fact the \nnn never takes more than 3 digits */
                    if (n < 4)          /* Out of space in dest buffer? */
                      return(-1);       /* If so, give up. */
                    *lp++ = CMDQ;       /* Must be quoted to prevent */
                    *lp++ = '0';        /* premature termination of */
                    *lp++ = '4';        /* \flit(...) */
                    *lp++ = '1';
                    n -= 4;
#endif /* COMMENT */
                    pp++;               /* Uncount it. */
                    s++;
                    continue;
                }
            }
            if (c == '}') {             /* Closing brace. */
                if (--bc == 0) {        /* Final one? */
                    *lp++ = ')';        /* Add closing paren for "\flit()" */
                    if (--n < 1) return(-1);
                    *lp++ = c;
                    if (--n < 1) return(-1);
                    s++;
                    break;
                }
            }
            *lp++ = c;                  /* General case */
            if (--n < 1) return(-1);
            s++;
        }
        *lp = NUL;
    } else {                            /* No brackets around, */
        while ((*lp++ = *s++))          /* just copy. */
          if (--n < 1)
            return(-1);
        lp--;
    }
    *src = s;                           /* Return updated source */
    *dest = lp;                         /* and destination pointers */
    if (bc)                             /* Fail if braces unbalanced */
      return(-1);
    else                                /* Otherwise succeed. */
      return(0);
}
#endif /* NOSPL */

/* Functions moved here from ckuusr.c to even out the module sizes... */

/*
  Breaks up string s -- IN PLACE! -- into a list of up to max words.
  Pointers to each word go into the array list[].
  max is the maximum number of words (pointers).
  If list is NULL, then they are added to the macro table.
  flag = 0 means the last field is to be one word, like all the other fields,
         so anything after it is discarded.
  flag = 1 means the last field extends to the end of the string, even if
         there are lots of words left, so the last field contains the
         remainder of the string.
*/
VOID
xwords(s,max,list,flag) char *s; int max; char *list[]; int flag; {
    char *p;
    int b, i, k, q, y, z;
#ifndef NOSPL
    int macro;
    macro = (list == NULL);
    debug(F010,"xwords",s,0);
#endif /* NOSPL */

#ifdef XWORDSDEBUG
    printf("XWORDS string=%s\n",s);
    printf("XWORDS max=%d\n",max);
#endif /* XWORDSDEBUG */
    p = s;                              /* Pointer to beginning of string */
    q = 0;                              /* Flag for doublequote removal */
    b = 0;                              /* Flag for outer brace removal */
    k = 0;                              /* Flag for in-word */
    y = 0;                              /* Brace nesting level */
    z = 0;                              /* "Word" counter, 0 thru max */

    if (list)
      for (i = 0; i <= max; i++)        /* Initialize pointers */
        list[i] = NULL;

    if (flag) max--;

    while (1) {                         /* Go thru word list */
        if (!s || (*s == '\0')) {       /* No more characters? */
            if (k != 0) {               /* Was I in a word? */
                if (z == max) break;    /* Yes, only go up to max. */
                z++;                    /* Count this word. */
#ifdef XWORDSDEBUG
                printf("1 z++ = %d\n", z);
#endif /* XWORDSDEBUG */
#ifndef NOSPL
                if (macro) {            /* Doing macro args */
                    if (z < 10) {
                        varnam[1] = (char) (z + '0'); /* Assign last arg */
                        addmac(varnam,p);
                    }
                    if (z <= max) {
#ifdef COMMENT
                        if (maclvl < 0)
                          addmac(varnam,p);
                        else
#endif /* COMMENT */
                          makestr(&(m_xarg[maclvl][z]),p);
                    }
                } else {                /* Not doing macro args */
#endif /* NOSPL */
                    list[z] = p;        /* Assign pointer. */
#ifdef XWORDSDEBUG
                    printf("[1]LIST[%d]=\"%s\"\n",z,list[z]);
#endif /* XWORDSDEBUG */
#ifndef NOSPL
                }
#endif /* NOSPL */
                break;                  /* And get out. */
            } else break;               /* Was not in a word */
        }
        if (k == 0 && (*s == SP || *s == HT)) { /* Eat leading blanks */
            s++;
            continue;
        } else if (q == 0 && *s == '{') { /* An opening brace */
            if (k == 0 && y == 0) {     /* If leading brace */
                p = s+1;                /* point past it */
                b = 1;                  /* and flag that we did this */
            }
            k = 1;                      /* Flag that we're in a word */
            y++;                        /* Count the brace. */
        } else if (q == 0 && *s == '}') { /* A closing brace. */
            y--;                        /* Count it. */
            if (y <= 0 && b != 0) {     /* If it matches the leading brace */
                char c;
                c = *(s+1);
                if (!c || c == SP || c == HT) { /* at EOL or followed by SP */
                    *s = SP;            /* change it to a space */
                    b = 0;              /* and we're not in braces any more */
                }
            }
#ifdef DOUBLEQUOTING
        /* Opening doublequote */
        } else if (k == 0 && b == 0 && *s == '"' && dblquo) {
            y++;
            p = s+1;                    /* point past it */
            q = 1;                      /* and flag that we did this */
            k = 1;                      /* Flag that we're in a word */
        /* Closing double quote */
        } else if (q > 0 && k > 0 && b == 0 && *s == '"' && dblquo) {
            char c;
            c = *(s+1);
            if (!c || c == SP || c == HT) { /* at EOL or followed by SP */
                y--;
                *s = SP;                /* change it to a space */
                q = 0;                  /* and we're not in quotes any more */
            }
#endif /* DOUBLEQUOTING */

        } else if (*s != SP && *s != HT) { /* Nonspace means we're in a word */
            if (k == 0) {               /* If we weren't in a word before, */
                p = s;                  /* Mark the beginning */
                if (flag && z == max) { /* Want last word to be remainder? */
                    z++;
#ifdef XWORDSDEBUG
                    printf("1 z++ = %d\n", z);
#endif /* XWORDSDEBUG */
                    list[z] = p;        /* Yes, point to it */
#ifdef XWORDSDEBUG
                    printf("[4]LIST[%d]=\"%s\"\n",z,list[z]);
#endif /* XWORDSDEBUG */
                    break;              /* and quit */
                }
                k = 1;                  /* Set in-word flag */
            }
        }
        /* If we're not inside a braced quantity, and we are in a word, and */
        /* we have hit whitespace, then we have a word. */
        if ((y < 1) && (k != 0) && (*s == SP || *s == HT) && !b) {
            if (!flag || z < max)       /* if we don't want to keep rest */
              *s = '\0';                /* terminate the arg with null */
            k = 0;                      /* say we're not in a word any more */
            y = 0;                      /* start braces off clean again */
            if (z == max) break;        /* Only go up to max. */
            z++;                        /* count this arg */
#ifdef XWORDSDEBUG
            printf("1 z++ = %d\n", z);
#endif /* XWORDSDEBUG */

#ifndef NOSPL
            if (macro) {
                if (z < 10) {
                    varnam[1] = (char) (z + '0'); /* compute its name */
                    addmac(varnam,p);   /* add it to the macro table */
                }
                if (z <= max) {
#ifdef COMMENT
                    if (maclvl < 0)
                      addmac(varnam,p);
                    else
#endif /* COMMENT */
                      makestr(&(m_xarg[maclvl][z]),p);
                }
            } else {
#endif /* NOSPL */
                list[z] = p;
#ifdef XWORDSDEBUG
                printf("[2]LIST[%d]=\"%s\"\n",z,list[z]);
#endif /* XWORDSDEBUG */
#ifndef NOSPL
            }
#endif /* NOSPL */
            p = s+1;
        }
        s++;                            /* Point past this character */
    }
    if ((z == 0) && (y > 1)) {          /* Extra closing brace(s) at end */
        z++;
#ifndef NOSPL
        if (macro) {
            if (z < 10) {
                varnam[1] = z + '0';    /* compute its name */
                addmac(varnam,p);       /* Add rest of line to last arg */
            }
            if (z <= max) {
#ifdef COMMENT
                if (maclvl < 0)
                  addmac(varnam,p);
                else
#endif /* COMMENT */
                  makestr(&(m_xarg[maclvl][z]),p);
            }
        } else {
#endif /* NOSPL */
            list[z] = p;
#ifdef XWORDSDEBUG
            printf("[3]LIST[%d]=\"%s\"\n",z,list[z]);
#endif /* XWORDSDEBUG */
#ifndef NOSPL
        }
#endif /* NOSPL */
    }
#ifndef NOSPL
    if (macro) {                        /* Macro */
        if (maclvl < 0) {
            a_dim[0] = z;               /* Array dimension is one less */
            topargc = z + 1;            /* than \v(argc) */
	    debug(F111,"a_dim[0]","D",a_dim[0]);
        } else {
            macargc[maclvl] = z + 1;    /* Set \v(argc) variable */
            n_xarg[maclvl] = z + 1;     /* This is the actual number */
            a_ptr[0] = m_xarg[maclvl];  /* Point \&_[] at the args */
            a_dim[0] = z;               /* And give it this dimension */
	    debug(F111,"a_dim[0]","E",a_dim[0]);
        }
    }
#endif /* NOSPL */
    return;
}

#ifndef NOSPL

/*  D O S H I F T  --  Do the SHIFT Command; shift macro args left by n */

/*  Note: at some point let's consolidate m_arg[][] and m_xarg[][]. */

int
doshift(n) int n; {                     /* n = shift count */
    int i, top, level;
    char /* *s, *m, */ buf[6];          /* Buffer to build scalar names */
    char * sx = tmpbuf;
    int nx = TMPBUFSIZ;

    debug(F101,"SHIFT count","",n);
    debug(F101,"SHIFT topargc","",topargc);

    if (n < 1)                          /* Stay in range */
      return(n == 0 ? 1 : 0);

    level = maclvl;
    top = (level < 0) ? topargc : macargc[level];

    if (n >= top)
      n = top - 1;

#ifdef DEBUG
    if (deblog) {
        debug(F101,"SHIFT count 2","",n);
        debug(F101,"SHIFT level","",level);
        if (level > -1)
          debug(F101,"SHIFT macargc[level]","",macargc[level]);
    }
#endif /* DEBUG */

    buf[0] = '\\';                      /* Initialize name template */
    buf[1] = '%';
    buf[2] = NUL;
    buf[3] = NUL;

    for (i = 1; i <= n; i++) {          /* Free shifted-over args */
        if (level < 0) {
            makestr(&(toparg[i]),NULL);
        } else {
            makestr(&(m_xarg[level][i]),NULL);
        }
        if (i < 10) {                   /* Is this necessary? */
            buf[2] = (char)(i+'0');
            delmac(buf,0);
        }
    }
    for (i = 1; i <= top-n; i++) {      /* Shift remaining args */
        if (level < 0) {
#ifdef COMMENT
            toparg[i] = toparg[i+n];    /* Full vector */
#else
            makestr(&(toparg[i]),toparg[i+n]); /* Full vector */
#endif /* COMMENT */
            if (i < 10)                 /* Scalars... */
              makestr(&(g_var[i+'0']),toparg[i+n]);
        } else {
#ifdef COMMENT
            m_xarg[level][i] = m_xarg[level][i+n];
#else
            makestr(&(m_xarg[level][i]),m_xarg[level][i+n]);
#endif /* COMMENT */
            if (i < 10) {
                buf[2] = (char)(i+'0');
                debug(F010,"SHIFT buf",buf,0);
                addmac(buf,m_xarg[level][i+n]);
            }
        }
    }
    for (i = top-n; i <= top; i++) {    /* Clear n args from the end */
        if (level < 0) {
#ifdef COMMENT
            toparg[i] = NULL;
#else
            makestr(&(toparg[i]),NULL);
#endif /* COMMENt */
            if (i < 10)
              makestr(&(g_var[i+'0']),NULL);
        } else {
#ifdef COMMENT
            m_xarg[level][i] = NULL;
#else
            makestr(&(m_xarg[level][i]),NULL);
#endif /* COMMENt */
            if (i < 10) {
                buf[2] = (char)(i+'0');
                delmac(buf,0);
            }
        }
    }
    if (level > -1) {                   /* Macro args */
        macargc[level] -= n;            /* Adjust count */
        n_xarg[maclvl] = macargc[level]; /* Here too */
        a_dim[0] = macargc[level] - 1;  /* Adjust array dimension */
	debug(F111,"a_dim[0]","F",a_dim[0]);
        zzstring("\\fjoin(&_[],{ },1)",&sx,&nx); /* Handle \%* */
#ifdef COMMENT
        makestr(&(m_line[level]),tmpbuf);
#endif /* COMMENT */
    } else {                            /* Ditto for top level */
        topargc -= n;
        a_dim[0] = topargc - 1;
	debug(F111,"a_dim[0]","G",a_dim[0]);
        zzstring("\\fjoin(&_[],{ },1)",&sx,&nx);
#ifdef COMMENT
        makestr(&topline,tmpbuf);
#endif /* COMMENT */
    }
    return(1);
}
#endif /* NOSPL */

int
docd(cx) int cx; {                      /* Do the CD command */
    int x;
    extern int server, srvcdmsg, cdactive;
    extern char * cdmsgfile[], * ckcdpath;
    char *s, *p;
#ifdef MAC
    char temp[34];
#endif /* MAC */
#ifdef IKSDCONF
extern int iksdcf;
#endif /* IKSDCONF */

#ifndef NOFRILLS
    if (cx == XXBACK) {
        if ((x = cmcfm()) < 0)
        cwdf = 1;
        if (prevdir) {
            s = zgtdir();
            if (!zchdir(prevdir)) {
                cwdf = 0;
                perror(s);
            } else {
                makestr(&prevdir,s);
            }
        }
        return(cwdf);
    }
#endif /* NOFRILLS */

    if (cx == XXCDUP) {
#ifdef VMS
        s = "[-]";
#else
#ifdef datageneral
        s = "^";
#else
        s = "..";
#endif /* datageneral */
#endif /* VMS */
        ckstrncpy(line,s,LINBUFSIZ);
        goto gocd;
    }
#ifndef NOSPL
    if (cx == XXKCD) {			/* Symbolic (Kermit) CD */
	char * p;
	int n, k;
	x = cmkey(kcdtab,nkcdtab,"Symbolic directory name","home",xxstring);
	if (x < 0)
	  return(x);
	x = lookup(kcdtab,atmbuf,nkcdtab,&k); /* Get complete keyword */
	if (x < 0) {
	    printf("?Lookup error\n");	/* shouldn't happen */
	    return(-9);
	}
        if ((x = cmcfm()) < 0)
	  return(x);
	if (k == VN_HOME) {		/* HOME: allow SET HOME to override */
	    ckstrncpy(line,homepath(),LINBUFSIZ);
	} else {			/* Other symbolic name */
	    /* Convert to variable syntax */
	    ckmakmsg(tmpbuf,TMPBUFSIZ,"\\v(",kcdtab[k].kwd,")",NULL);
	    p = line;			/* Expand the variable */
	    n = LINBUFSIZ;
	    zzstring(tmpbuf,&p,&n);
	    if (!line[0]) {		/* Fail if variable not defined */
		printf("?%s - not defined\n",tmpbuf);
		return(success = 0);
	    }
	}
	s = line;			/* All OK, go try to CD... */
	goto gocd;
    }
#endif /* NOSPL */

    cdactive = 1;
#ifdef GEMDOS
    if ((x = cmdir("Name of local directory, or carriage return",
                   homepath(),
                   &s,
                   NULL
                   )
         ) < 0 )
      return(x);
#else
#ifdef OS2
    if ((x = cmdirp("Name of PC disk and/or directory,\n\
       or press the Enter key for the default",
                    homepath(),
                    &s,
                    ckcdpath ? ckcdpath : getenv("CDPATH"),
                    xxstring
                    )
         ) < 0 )
      return(x);
#else
#ifdef MAC
    x = ckstrncpy(temp,homepath(),32);
    if (x > 0) if (temp[x-1] != ':') { temp[x] = ':'; temp[x+1] = NUL; }
    if ((x = cmtxt("Name of Macintosh volume and/or folder,\n\
 or press the Return key for the desktop on the boot disk",
                   temp,&s, xxstring)) < 0 )
      return(x);
#else
    if ((x = cmdirp("Carriage return for home directory,\n\
or name of directory on this computer",
#ifdef VMS
                    "SYS$LOGIN",        /* With no colon */
#else
                    homepath(),		/* In VMS this is "SYS$LOGIN:" */
#endif /* VMS */
                    &s,
                    ckcdpath ? ckcdpath : getenv("CDPATH"),
                    xxstring
                    )) < 0)
      return(x);
#endif /* MAC */
#endif /* OS2 */
#endif /* GEMDOS */
    ckstrncpy(line,s,LINBUFSIZ);        /* Make a safe copy */
    s = line;
#ifdef VMS
    if (ckmatch("*.DIR;1$",s,0,0))
      if (cvtdir(s,tmpbuf,TMPBUFSIZ) > 0)
        s = tmpbuf;
#endif /* VMS */
    debug(F110,"docd",s,0);
#ifndef MAC
    if ((x = cmcfm()) < 0)              /* Get confirmation */
      return(x);
#endif /* MAC */

  gocd:

#ifdef datageneral
    x = strlen(line);                   /* homdir ends in colon, */
    if (x > 1 && line[x-1] == ':')      /* and "dir" doesn't like that... */
      line[x-1] = NUL;
#endif /* datageneral */

#ifdef MAC
    cwdf = 1;
    if (!zchdir(s)) {
        cwdf = 0;
        if (*s != ':') {                /* If it failed, */
            char *p;                    /* supply leading colon */
            int len = (int)strlen(s) + 2;
            p = malloc(len);            /* and try again... */
            if (p) {
                strcpy(p,":");          /* safe */
                strcat(p,s);            /* safe */
                if (zchdir(p))
                  cwdf = 1;
                free(p);
                p = NULL;
            }
        }
    }
    if (!cwdf)
      perror(s);
#else
    p = zgtdir();
    if (!zchdir(s)) {
        cwdf = 0;
#ifdef CKROOT
        if (ckrooterr)
          printf("?Off limits: \"%s\"\n",s);
        else
#endif /* CKROOT */
          perror(s);
    } else cwdf = 1;
#endif /* MAC */

    x = 0;
    if (cwdf) {
        makestr(&prevdir,p);
        debug(F111,"docd","srvcdmsg",srvcdmsg);
        if (srvcdmsg
#ifdef IKSDCONF
            && !(inserver && !iksdcf)
#endif /* IKSDCONF */
            ) {
            int i;
            for (i = 0; i < 8; i++) {
                debug(F111,"docd cdmsgfile[i]",cdmsgfile[i],i);
                if (zchki(cdmsgfile[i]) > -1) {
                    x = 1;
                    dotype(cdmsgfile[i],xaskmore,0,0,NULL,0,NULL,0,0,NULL,0);
                    break;
                }
            }
        }
    }
/* xdocd: */
    if (!x && srvcdmsg && !server
#ifdef IKSDCONF
        && !(inserver && !iksdcf)
#endif /* IKSDCONF */
        && !quiet && !xcmdsrc)
      printf("%s\n", zgtdir());

    return(cwdf);
}

static int on_ctrlc = 0;

VOID
fixcmd() {                      /* Fix command parser after interruption */
#ifndef NOSPL
#ifndef NOONCTRLC
    if (nmac) {                         /* Any macros defined? */
        int k;                          /* Yes */
        char * s = "on_ctrlc";          /* Name of Ctrl-C handling macro */
        k = mlook(mactab,s,nmac);       /* Look it up. */
        if (k >= 0) {                   /* If found, */
            if (on_ctrlc++ == 0) {      /* if not already executing, */
                if (dodo(k,"",0) > -1)  /* set it up, */
                  parser(1);            /* execute it, */
            }
            delmac(s,1);                /* and undefine it. */
        }
    }
    on_ctrlc = 0;
#endif /* NOONCTRLC */
#endif /* NOSPL */
    dostop();                   /* Back to top level (also calls conint()). */
    bgchk();                    /* Check background status */
    if (*psave) {               /* If old prompt saved, */
        cmsetp(psave);          /* restore it. */
        *psave = NUL;
    }
    success = 0;                /* Tell parser last command failed */
}

#ifndef NOSHOW                          /* SHOW FEATURES */
/*
  Note, presently optlist[] index overflow is not checked.
  There is plenty of room (less than 360 entries for 1000 slots).
  When space starts to get tight, check for noptlist >= NOPTLIST
  every time noptlist is incremented.
*/
#define NOPTLIST 1024
static int noptlist = 0;
static char * optlist[NOPTLIST+1];
static int hpos = 0;

int
prtopt(lines,s) int * lines; char *s; { /* Print an option */
    int y, i;                           /* Does word wrap. */
    if (!s) s = "";
    i = *lines;
    if (!*s) {                          /* Empty argument */
        if (hpos > 0) {                 /* means to end this line. */
            printf("\n");               /* Not needed if already at */
            if (++i > (cmd_rows - 3)) { /* beginning of new line. */
                if (!askmore())
                  return(0);
                else
                  i = 0;
            }
        }
        printf("\n");                   /* And then make a blank line */
        if (++i > (cmd_rows - 3)) {
            if (!askmore())
              return(0);
            else
              i = 0;
        }
        hpos = 0;
        *lines = i;
        return(1);
    }
    y = (int)strlen(s) + 1;
    hpos += y;
    debug(F101,"prtopt hpos","",hpos);
    debug(F101,"prtopt cmd_cols","",cmd_cols);

    if (
#ifdef OS2
        hpos > ((cmd_cols > 40) ? (cmd_cols - 1) : 79)
#else /* OS2 */
        hpos > ((tt_cols > 40) ? (tt_cols - 1) : 79)
#endif /* OS2 */
        ) {
        printf("\n");
        if (++i > (cmd_rows - 3)) {
            if (!askmore())
              return(0);
            else
              i = 0;
        }
        printf(" %s",s);
        hpos = y;
    } else
      printf(" %s",s);
    *lines = i;
    return(1);
}

static VOID
initoptlist() {
    int i;
    if (noptlist > 0)
      return;
    for (i = 0; i < NOPTLIST; i++)
      optlist[i] = NULL;

#ifdef MAC
#ifdef MPW
    makestr(&(optlist[noptlist++]),"MPW");
#endif /* MPW */
#endif /* MAC */

#ifdef MAC
#ifdef THINK_C
    makestr(&(optlist[noptlist++]),"THINK_C");
#endif /* THINK_C */
#endif /* MAC */

#ifdef __386__
    makestr(&(optlist[noptlist++]),"__386__");
#endif /* __386__ */

/* Memory models... */

#ifdef __FLAT__
    makestr(&(optlist[noptlist++]),"__FLAT__");
#endif /* __FLAT__ */
#ifdef __SMALL__
    makestr(&(optlist[noptlist++]),"__SMALL__");
#endif /* __SMALL__ */
#ifdef __MEDIUM__
    makestr(&(optlist[noptlist++]),"__MEDIUM__");
#endif /* __MEDIUM__ */
#ifdef __COMPACT__
    makestr(&(optlist[noptlist++]),"__COMPACT__");
#endif /* __COMPACT__ */
#ifdef __LARGE__
    makestr(&(optlist[noptlist++]),"__LARGE__");
#endif /* __LARGE__ */

#ifdef DEBUG
#ifdef IFDEBUG
    makestr(&(optlist[noptlist++]),"IFDEBUG");
#else
    makestr(&(optlist[noptlist++]),"DEBUG");
#endif /* IFDEBUG */
#endif /* DEBUG */
#ifdef TLOG
    makestr(&(optlist[noptlist++]),"TLOG");
#endif /* TLOG */
#ifdef BIGBUFOK
    makestr(&(optlist[noptlist++]),"BIGBUFOK");
#endif /* BIGBUFOK */
#ifdef INPBUFSIZ
    sprintf(line,"INPBUFSIZ=%d",INPBUFSIZ); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* INPBUFSIZE */
#ifdef LINBUFSIZ
    sprintf(line,"LINBUFSIZ=%d",LINBUFSIZ); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* LINBUFSIZE */
#ifdef INBUFSIZE
    sprintf(line,"INBUFSIZE=%d",INBUFSIZE); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* INBUFSIZE */
#ifdef OBUFSIZE
    sprintf(line,"OBUFSIZE=%d",OBUFSIZE); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* OBUFSIZE */
#ifdef FD_SETSIZE
    sprintf(line,"FD_SETSIZE=%d",FD_SETSIZE); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* FD_SETSIZE */
#ifdef XFRCAN
    makestr(&(optlist[noptlist++]),"XFRCAN");
#endif /* XFRCAN */
#ifdef XPRINT
    makestr(&(optlist[noptlist++]),"XPRINT");
#endif /* XPRINT */
#ifdef PIPESEND
    makestr(&(optlist[noptlist++]),"PIPESEND");
#endif /* PIPESEND */
#ifdef CK_SPEED
    makestr(&(optlist[noptlist++]),"CK_SPEED");
#endif /* CK_SPEED */
#ifdef CK_FAST
    makestr(&(optlist[noptlist++]),"CK_FAST");
#endif /* CK_FAST */
#ifdef CK_APC
    makestr(&(optlist[noptlist++]),"CK_APC");
#endif /* CK_APC */
#ifdef CK_AUTODL
    makestr(&(optlist[noptlist++]),"CK_AUTODL");
#endif /* CK_AUTODL */
#ifdef CK_MKDIR
    makestr(&(optlist[noptlist++]),"CK_MKDIR");
#endif /* CK_MKDIR */
#ifdef NOMKDIR
    makestr(&(optlist[noptlist++]),"NOMKDIR");
#endif /* NOMKDIR */
#ifdef CK_LABELED
    makestr(&(optlist[noptlist++]),"CK_LABELED");
#endif /* CK_LABELED */
#ifdef NODIAL
    makestr(&(optlist[noptlist++]),"NODIAL");
#endif /* NODIAL */
#ifdef MINIDIAL
    makestr(&(optlist[noptlist++]),"MINIDIAL");
#endif /* MINIDIAL */
#ifdef WHATAMI
    makestr(&(optlist[noptlist++]),"WHATAMI");
#endif /* WHATAMI */
#ifdef DYNAMIC
    makestr(&(optlist[noptlist++]),"DYNAMIC");
#endif /* DYNAMIC */
#ifndef NOSPL
    sprintf(line,"CMDDEP=%d",CMDDEP);   /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* NOSPL */

#ifdef MAXPATHLEN
    sprintf(line,"MAXPATHLEN=%d",MAXPATHLEN); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* MAXPATHLEN */

#ifdef DEVNAMLEN
    sprintf(line,"DEVNAMLEN=%d",DEVNAMLEN); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* DEVNAMLEN */

#ifdef NO_PARAM_H
    makestr(&(optlist[noptlist++]),"NO_PARAM_H");
#endif /* NO_PARAM_H */

#ifdef INCL_PARAM_H
    makestr(&(optlist[noptlist++]),"INCL_PARAM_H");
#endif /* INCL_PARAM_H */

    sprintf(line,"CKMAXPATH=%d",CKMAXPATH); /* SAFE */
    makestr(&(optlist[noptlist++]),line);

    sprintf(line,"CKMAXOPEN=%d",CKMAXOPEN); /* SAFE */
    makestr(&(optlist[noptlist++]),line);

    sprintf(line,"Z_MAXCHAN=%d",Z_MAXCHAN); /* SAFE */
    makestr(&(optlist[noptlist++]),line);

#ifdef OPEN_MAX
    sprintf(line,"OPEN_MAX=%d",OPEN_MAX); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* OPEN_MAX */

#ifdef _POSIX_OPEN_MAX
    sprintf(line,"_POSIX_OPEN_MAX=%d",_POSIX_OPEN_MAX); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* _POSIX_OPEN_MAX */

#ifdef CKCHANNELIO
    {
        extern int z_maxchan;
#ifdef UNIX
        extern int ckmaxfiles;
        sprintf(line,"ckmaxfiles=%d",ckmaxfiles); /* SAFE */
        makestr(&(optlist[noptlist++]),line);
#endif /* UNIX */
        sprintf(line,"z_maxchan=%d",z_maxchan); /* SAFE */
        makestr(&(optlist[noptlist++]),line);
    }
#endif /* CKCHANNELIO */

#ifdef FOPEN_MAX
    sprintf(line,"FOPEN_MAX=%d",FOPEN_MAX); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* FOPEN_MAX */

#ifdef MAXGETPATH
    sprintf(line,"MAXGETPATH=%d",MAXGETPATH); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* MAXGETPATH */

#ifdef CMDBL
    sprintf(line,"CMDBL=%d",CMDBL);     /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* CMDBL */

#ifdef VNAML
    sprintf(line,"VNAML=%d",VNAML);     /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* VNAML */

#ifdef ARRAYREFLEN
    sprintf(line,"ARRAYREFLEN=%d",ARRAYREFLEN); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* ARRAYREFLEN */

#ifdef UIDBUFLEN
    sprintf(line,"UIDBUFLEN=%d",UIDBUFLEN); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* UIDBUFLEN */

#ifdef FORDEPTH
    sprintf(line,"FORDEPTH=%d",FORDEPTH); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* FORDEPTH */

#ifdef MAXTAKE
    sprintf(line,"MAXTAKE=%d",MAXTAKE); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* MAXTAKE */

#ifdef MACLEVEL
    sprintf(line,"MACLEVEL=%d",MACLEVEL); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* MACLEVEL */

#ifdef MAC_MAX
    sprintf(line,"MAC_MAX=%d",MAC_MAX); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* MAC_MAX */

#ifdef _LARGEFILE_SOURCE
    makestr(&(optlist[noptlist++]),"_LARGEFILE_SOURCE");
#endif	/* _LARGEFILE_SOURCE */

#ifdef _FILE_OFFSET_BITS
    sprintf(line,"_FILE_OFFSET_BITS=%d",_FILE_OFFSET_BITS); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif	/* _FILE_OFFSET_BITS */

#ifdef __USE_FILE_OFFSET64
    makestr(&(optlist[noptlist++]),"__USE_FILE_OFFSET64");
#endif	/* __USE_FILE_OFFSET64 */

#ifdef __USE_LARGEFILE64
    makestr(&(optlist[noptlist++]),"__USE_LARGEFILE64");
#endif	/* __USE_LARGEFILE64 */

#ifdef COMMENT
#ifdef CHAR_MAX
    sprintf(line,"CHAR_MAX=%llx",CHAR_MAX); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* CHAR_MAX */
#ifdef UCHAR_MAX
    sprintf(line,"UCHAR_MAX=%llx",UCHAR_MAX); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* UCHAR_MAX */
#ifdef SHRT_MAX
    sprintf(line,"SHRT_MAX=%llx",SHRT_MAX); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* SHRT_MAX */
#ifdef USHRT_MAX
    sprintf(line,"USHRT_MAX=%llx",USHRT_MAX); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* USHRT_MAX */
#ifdef INT_MAX
    sprintf(line,"INT_MAX=%llx",INT_MAX); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* INT_MAX */
#ifdef UINT_MAX
    sprintf(line,"UINT_MAX=%llx",UINT_MAX); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* UINT_MAX */
#ifdef MAX_LONG
    sprintf(line,"MAX_LONG=%llx",MAX_LONG); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* MAX_LONG */
#ifdef LONG_MAX
    sprintf(line,"LONG_MAX=%llx",LONG_MAX); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* LONG_MAX */
#ifdef ULONG_MAX
    sprintf(line,"ULONG_MAX=%llx",ULONG_MAX); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* ULONG_MAX */
#ifdef MAXINT
    sprintf(line,"MAXINT=%llx",MAXINT); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* MAXINT */
#ifdef MAXLONG
    sprintf(line,"MAXLONG=%llx",MAXLONG); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* MAXLONG */
#ifdef NT
#ifdef LLONG_MAX
    sprintf(line,"LLONG_MAX=%I64x",LLONG_MAX); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* LLONG_MAX */
#ifdef ULLONG_MAX
    sprintf(line,"ULLONG_MAX=%I64x",ULLONG_MAX); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* ULLONG_MAX */
#ifdef MAXLONGLONG
    sprintf(line,"MAXLONGLONG=%I64x",MAXLONGLONG);  /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* MAXLONGLONG */
#else
#ifdef LLONG_MAX
    sprintf(line,"LLONG_MAX=%llx",LLONG_MAX); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* LLONG_MAX */
#ifdef ULLONG_MAX
    sprintf(line,"ULLONG_MAX=%llx",ULLONG_MAX); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* ULLONG_MAX */
#ifdef MAXLONGLONG
    sprintf(line,"MAXLONGLONG=%llx",MAXLONGLONG);  /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* MAXLONGLONG */
#endif
#ifdef _INTEGRAL_MAX_BITS
    sprintf(line,"_INTEGRAL_MAX_BITS=%d",_INTEGRAL_MAX_BITS);  /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* _INTEGRAL_MAX_BITS */
#endif	/* COMMENT */

#ifdef MINPUTMAX
    sprintf(line,"MINPUTMAX=%d",MINPUTMAX); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* MINPUTMAX */

#ifdef MAXWLD
    sprintf(line,"MAXWLD=%d",MAXWLD); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#else
#ifdef OS2
    makestr(&(optlist[noptlist++]),"MAXWLD=unlimited");
#endif /* OS2 */
#endif /* MAXWLD */

#ifdef MSENDMAX
    sprintf(line,"MSENDMAX=%d",MSENDMAX); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* MSENDMAX */

#ifdef MAXDDIR
    sprintf(line,"MAXDDIR=%d",MAXDDIR); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* MAXDDIR */

#ifdef MAXDNUMS
    sprintf(line,"MAXDNUMS=%d",MAXDNUMS); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* MAXDNUMS */

#ifdef UNIX
    makestr(&(optlist[noptlist++]),"UNIX");
#endif /* UNIX */

#ifdef VMS
    makestr(&(optlist[noptlist++]),"VMS");
#ifdef __VMS_VER
    sprintf(line,"__VMS_VER=%d",__VMS_VER); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* __VMS_VER */
#ifdef VMSV70
    makestr(&(optlist[noptlist++]),"VMSV70");
#endif /* VMSV70 */
#ifdef OLD_VMS
    makestr(&(optlist[noptlist++]),"OLD_VMS");
#endif /* OLD_VMS */
#ifdef vms
    makestr(&(optlist[noptlist++]),"vms");
#endif /* vms */
#ifdef VMSV60
    makestr(&(optlist[noptlist++]),"VMSV60");
#endif /* VMSV60 */
#ifdef VMSV80
    makestr(&(optlist[noptlist++]),"VMSV80");
#endif /* VMSV80 */
#ifdef VMSSHARE
    makestr(&(optlist[noptlist++]),"VMSSHARE");
#endif /* VMSSHARE */
#ifdef NOVMSSHARE
    makestr(&(optlist[noptlist++]),"NOVMSSHARE");
#endif /* NOVMSSHARE */
#endif /* VMS */

#ifdef datageneral
    makestr(&(optlist[noptlist++]),"datageneral");
#endif /* datageneral */
#ifdef apollo
    makestr(&(optlist[noptlist++]),"apollo");
#endif /* apollo */
#ifdef aegis
    makestr(&(optlist[noptlist++]),"aegis");
#endif /* aegis */
#ifdef A986
    makestr(&(optlist[noptlist++]),"A986");
#endif /* A986 */
#ifdef AMIGA
    makestr(&(optlist[noptlist++]),"AMIGA");
#endif /* AMIGA */
#ifdef CONVEX9
    makestr(&(optlist[noptlist++]),"CONVEX9");
#endif /* CONVEX9 */
#ifdef CONVEX10
    makestr(&(optlist[noptlist++]),"CONVEX10");
#endif /* CONVEX9 */
#ifdef MAC
    makestr(&(optlist[noptlist++]),"MAC");
#endif /* MAC */
#ifdef AUX
    makestr(&(optlist[noptlist++]),"AUX");
#endif /* AUX */

#ifdef OS2
    makestr(&(optlist[noptlist++]),"OS2");
#ifdef NT
    makestr(&(optlist[noptlist++]),"NT");
#endif /* NT */
#endif /* OS2 */

#ifdef OSK
    makestr(&(optlist[noptlist++]),"OS9");
#endif /* OSK */

#ifdef MSDOS
    makestr(&(optlist[noptlist++]),"MSDOS");
#endif /* MSDOS */

#ifdef DIRENT
    makestr(&(optlist[noptlist++]),"DIRENT");
#endif /* DIRENT */

#ifdef SDIRENT
    makestr(&(optlist[noptlist++]),"SDIRENT");
#endif /* SDIRENT */

#ifdef NDIR
    makestr(&(optlist[noptlist++]),"NDIR");
#endif /* NDIR */

#ifdef XNDIR
    makestr(&(optlist[noptlist++]),"XNDIR");
#endif /* XNDIR */

#ifdef SAVEDUID
    makestr(&(optlist[noptlist++]),"SAVEDUID");
#endif /* SAVEDUID */

#ifdef RENAME
    makestr(&(optlist[noptlist++]),"RENAME");
#endif /* RENAME */

#ifdef CK_TMPDIR
    makestr(&(optlist[noptlist++]),"CK_TMPDIR");
#endif /* CK_TMPDIR */

#ifdef NOCCTRAP
    makestr(&(optlist[noptlist++]),"NOCCTRAP");
#endif /* NOCCTRAP */

#ifdef NOCOTFMC
    makestr(&(optlist[noptlist++]),"NOCOTFMC");
#endif /* NOCOTFMC */

#ifdef NOFRILLS
    makestr(&(optlist[noptlist++]),"NOFRILLS");
#endif /* NOFRILLS */

#ifdef PARSENSE
    makestr(&(optlist[noptlist++]),"PARSENSE");
#endif /* PARSENSE */

#ifdef TIMEH
    makestr(&(optlist[noptlist++]),"TIMEH");
#endif /* TIMEH */

#ifdef NOTIMEH
    makestr(&(optlist[noptlist++]),"TIMEH");
#endif /* NOTIMEH */

#ifdef SYSTIMEH
    makestr(&(optlist[noptlist++]),"SYSTIMEH");
#endif /* SYSTIMEH */

#ifdef NOSYSTIMEH
    makestr(&(optlist[noptlist++]),"SYSTIMEH");
#endif /* NOSYSTIMEH */

#ifdef SYSTIMEBH
    makestr(&(optlist[noptlist++]),"SYSTIMEBH");
#endif /* SYSTIMEBH */

#ifdef NOSYSTIMEBH
    makestr(&(optlist[noptlist++]),"SYSTIMEBH");
#endif /* NOSYSTIMEBH */

#ifdef UTIMEH
    makestr(&(optlist[noptlist++]),"UTIMEH");
#endif /* UTIMEH */

#ifdef SYSUTIMEH
    makestr(&(optlist[noptlist++]),"SYSUTIMEH");
#endif /* SYSUTIMEH */

#ifdef CK_NEED_SIG
    makestr(&(optlist[noptlist++]),"CK_NEED_SIG");
#endif /* CK_NEED_SIG */

#ifdef CK_TTYFD
    makestr(&(optlist[noptlist++]),"CK_TTYFD");
#endif /* CK_TTYFD */

#ifdef NETCONN
    makestr(&(optlist[noptlist++]),"NETCONN");
#endif /* NETCONN */

#ifdef TCPSOCKET
    makestr(&(optlist[noptlist++]),"TCPSOCKET");
#ifdef NOTCPOPTS
    makestr(&(optlist[noptlist++]),"NOTCPOPTS");
#endif /* NOTCPOPTS */
#ifdef CK_DNS_SRV
    makestr(&(optlist[noptlist++]),"CK_DNS_SRV");
#endif /* CK_DNS_SRV */
#ifdef NO_DNS_SRV
    makestr(&(optlist[noptlist++]),"NO_DNS_SRV");
#endif /* NO_DNS_SRV */
#ifdef CKGHNLHOST
    makestr(&(optlist[noptlist++]),"CKGHNLHOST");
#endif /* CKGHNLHOST */
#ifdef NOLISTEN
    makestr(&(optlist[noptlist++]),"NOLISTEN");
#endif /* NOLISTEN */
#ifdef SOL_SOCKET
    makestr(&(optlist[noptlist++]),"SOL_SOCKET");
#endif /* SOL_SOCKET */
#ifdef SO_OOBINLINE
    makestr(&(optlist[noptlist++]),"SO_OOBINLINE");
#endif /* SO_OOBINLNE */
#ifdef SO_DONTROUTE
    makestr(&(optlist[noptlist++]),"SO_DONTROUTE");
#endif /* SO_DONTROUTE */
#ifdef SO_KEEPALIVE
    makestr(&(optlist[noptlist++]),"SO_KEEPALIVE");
#endif /* SO_KEEPALIVE */
#ifdef SO_LINGER
    makestr(&(optlist[noptlist++]),"SO_LINGER");
#endif /* SO_LINGER */
#ifdef TCP_NODELAY
    makestr(&(optlist[noptlist++]),"TCP_NODELAY");
#endif /* TCP_NODELAY */
#ifdef SO_SNDBUF
    makestr(&(optlist[noptlist++]),"SO_SNDBUF");
#endif /* SO_SNDBUF */
#ifdef SO_RCVBUF
    makestr(&(optlist[noptlist++]),"SO_RCVBUF");
#endif /* SO_RCVBUF */
#ifdef h_addr
    makestr(&(optlist[noptlist++]),"h_addr");
#endif /* h_addr */
#ifdef HADDRLIST
    makestr(&(optlist[noptlist++]),"HADDRLIST");
#endif /* HADDRLIST */
#ifdef CK_SOCKS
    makestr(&(optlist[noptlist++]),"CK_SOCKS");
#ifdef CK_SOCKS5
    makestr(&(optlist[noptlist++]),"CK_SOCKS5");
#endif /* CK_SOCKS5 */
#ifdef CK_SOCKS_NS
    makestr(&(optlist[noptlist++]),"CK_SOCKS_NS");
#endif /* CK_SOCKS_NS */
#endif /* CK_SOCKS */
#ifdef RLOGCODE
    makestr(&(optlist[noptlist++]),"RLOGCODE");
#endif /* RLOGCODE */
#ifdef NETCMD
    makestr(&(optlist[noptlist++]),"NETCMD");
#endif /* NETCMD */
#ifdef NONETCMD
    makestr(&(optlist[noptlist++]),"NONETCMD");
#endif /* NONETCMD */
#ifdef NETPTY
    makestr(&(optlist[noptlist++]),"NETPTY");
#endif /* NETPTY */
#ifdef CK_ENVIRONMENT
    makestr(&(optlist[noptlist++]),"CK_ENVIRONMENT");
#endif /* CK_ENVIRONMENT */
#endif /* TCPSOCKET */
#ifdef TNCODE
    makestr(&(optlist[noptlist++]),"TNCODE");
#endif /* TNCODE */
#ifdef CK_FORWARD_X
    makestr(&(optlist[noptlist++]),"CK_FORWARD_X");
#endif /* CK_FORWARD_X */
#ifdef TN_COMPORT
    makestr(&(optlist[noptlist++]),"TN_COMPORT");
#endif /* TN_COMPORT */
#ifdef MULTINET
    makestr(&(optlist[noptlist++]),"MULTINET");
#endif /* MULTINET */
#ifdef DEC_TCPIP
    makestr(&(optlist[noptlist++]),"DEC_TCPIP");
#endif /* DEC_TCPIP */
#ifdef TCPWARE
    makestr(&(optlist[noptlist++]),"TCPWARE");
#endif /* TCPWARE */
#ifdef UCX50
    makestr(&(optlist[noptlist++]),"UCX50");
#endif /* UCX50 */
#ifdef CMU_TCPIP
    makestr(&(optlist[noptlist++]),"CMU_TCPIP");
#endif /* CMU_TCPIP */
#ifdef TTLEBUF
    makestr(&(optlist[noptlist++]),"TTLEBUF");
#endif /* TTLEBUF */
#ifdef NETLEBUF
    makestr(&(optlist[noptlist++]),"NETLEBUF");
#endif /* NETLEBUF */
#ifdef IKS_OPTION
    makestr(&(optlist[noptlist++]),"IKS_OPTION");
#endif /* IKS_OPTION */
#ifdef IKSDB
    makestr(&(optlist[noptlist++]),"IKSDB");
#endif /* IKSDB */
#ifdef IKSDCONF
    makestr(&(optlist[noptlist++]),"IKSDCONF");
#endif /* IKSDCONF */
#ifdef CK_LOGIN
    makestr(&(optlist[noptlist++]),"CK_LOGIN");
#endif /* CK_LOGIN */
#ifdef CK_PAM
    makestr(&(optlist[noptlist++]),"CK_PAM");
#endif /* CK_PAM */
#ifdef CK_SHADOW
    makestr(&(optlist[noptlist++]),"CK_SHADOW");
#endif /* CK_SHADOW */
#ifdef CONGSPD
    makestr(&(optlist[noptlist++]),"CONGSPD");
#endif /* CONGSPD */
#ifdef SUNX25
    makestr(&(optlist[noptlist++]),"SUNX25");
#endif /* SUNX25 */
#ifdef IBMX25
    makestr(&(optlist[noptlist++]),"IBMX25");
#endif /* IBMX25 */
#ifdef HPX25
    makestr(&(optlist[noptlist++]),"HPX25");
#endif /* HPX25 */
#ifdef DECNET
    makestr(&(optlist[noptlist++]),"DECNET");
#endif /* DECNET */
#ifdef SUPERLAT
    makestr(&(optlist[noptlist++]),"SUPERLAT");
#endif /* SUPERLAT */
#ifdef NPIPE
    makestr(&(optlist[noptlist++]),"NPIPE");
#endif /* NPIPE */
#ifdef CK_NETBIOS
    makestr(&(optlist[noptlist++]),"CK_NETBIOS");
#endif /* CK_NETBIOS */
#ifdef ATT7300
    makestr(&(optlist[noptlist++]),"ATT7300");
#endif /* ATT7300 */
#ifdef ATT6300
    makestr(&(optlist[noptlist++]),"ATT6300");
#endif /* ATT6300 */
#ifdef HDBUUCP
    makestr(&(optlist[noptlist++]),"HDBUUCP");
#endif /* HDBUUCP */
#ifdef USETTYLOCK
    makestr(&(optlist[noptlist++]),"USETTYLOCK");
#endif /* USETTYLOCK */
#ifdef USE_UU_LOCK
    makestr(&(optlist[noptlist++]),"USE_UU_LOCK");
#endif /* USE_UU_LOCK */
#ifdef HAVE_LOCKDEV
    makestr(&(optlist[noptlist++]),"HAVE_LOCKDEV");
#endif /* HAVE_LOCKDEV */
#ifdef HAVE_BAUDBOY
    makestr(&(optlist[noptlist++]),"HAVE_BAUDBOY");
#endif /* HAVE_BAUDBOY */
#ifdef HAVE_OPENPTY
    makestr(&(optlist[noptlist++]),"HAVE_OPENPTY");
#endif /* HAVE_OPENPTY */
#ifdef TTPTYCMD
    makestr(&(optlist[noptlist++]),"TTPTYCMD");
#endif /* TTPTYCMD */
#ifdef NOUUCP
    makestr(&(optlist[noptlist++]),"NOUUCP");
#endif /* NOUUCP */
#ifdef LONGFN
    makestr(&(optlist[noptlist++]),"LONGFN");
#endif /* LONGFN */
#ifdef RDCHK
    makestr(&(optlist[noptlist++]),"RDCHK");
#endif /* RDCHK */
#ifdef SELECT
    makestr(&(optlist[noptlist++]),"SELECT");
#endif /* SELECT */
#ifdef USLEEP
    makestr(&(optlist[noptlist++]),"USLEEP");
#endif /* USLEEP */
#ifdef NAP
    makestr(&(optlist[noptlist++]),"NAP");
#endif /* NAP */
#ifdef NAPHACK
    makestr(&(optlist[noptlist++]),"NAPHACK");
#endif /* NAPHACK */
#ifdef CK_POLL
    makestr(&(optlist[noptlist++]),"CK_POLL");
#endif /* CK_POLL */
#ifdef NOIEXTEN
    makestr(&(optlist[noptlist++]),"NOIEXTEN");
#endif /* NOIEXTEN */
#ifdef EXCELAN
    makestr(&(optlist[noptlist++]),"EXCELAN");
#endif /* EXCELAN */
#ifdef INTERLAN
    makestr(&(optlist[noptlist++]),"INTERLAN");
#endif /* INTERLAN */
#ifdef NOFILEH
    makestr(&(optlist[noptlist++]),"NOFILEH");
#endif /* NOFILEH */
#ifdef NOSYSIOCTLH
    makestr(&(optlist[noptlist++]),"NOSYSIOCTLH");
#endif /* NOSYSIOCTLH */
#ifdef DCLPOPEN
    makestr(&(optlist[noptlist++]),"DCLPOPEN");
#endif /* DCLPOPEN */
#ifdef NOSETBUF
    makestr(&(optlist[noptlist++]),"NOSETBUF");
#endif /* NOSETBUF */
#ifdef NOXFER
    makestr(&(optlist[noptlist++]),"NOXFER");
#endif /* NOXFER */
#ifdef NOCURSES
    makestr(&(optlist[noptlist++]),"NOCURSES");
#endif /* NOCURSES */
#ifdef NOSERVER
    makestr(&(optlist[noptlist++]),"NOSERVER");
#endif /* NOSERVER */
#ifdef NOPATTERNS
    makestr(&(optlist[noptlist++]),"NOPATTERNS");
#else
#ifdef PATTERNS
    makestr(&(optlist[noptlist++]),"PATTERNS");
#endif /* PATTERNS */
#endif /* NOPATTERNS */
#ifdef NOCKEXEC
    makestr(&(optlist[noptlist++]),"NOCKEXEC");
#else
#ifdef CKEXEC
    makestr(&(optlist[noptlist++]),"CKEXEC");
#endif /* CKEXEC */
#endif /* NOCKEXEC */
#ifdef NOAUTODL
    makestr(&(optlist[noptlist++]),"NOAUTODL");
#endif /* NOAUTODL */
#ifdef NOMSEND
    makestr(&(optlist[noptlist++]),"NOMSEND");
#endif /* NOMSEND */
#ifdef NOFDZERO
    makestr(&(optlist[noptlist++]),"NOFDZERO");
#endif /* NOFDZERO */
#ifdef NOPOPEN
    makestr(&(optlist[noptlist++]),"NOPOPEN");
#endif /* NOPOPEN */
#ifdef NOPARTIAL
    makestr(&(optlist[noptlist++]),"NOPARTIAL");
#endif /* NOPARTIAL */
#ifdef NOKVERBS
    makestr(&(optlist[noptlist++]),"NOKVERBS");
#endif /* NOKVERBS */
#ifdef NOSETREU
    makestr(&(optlist[noptlist++]),"NOSETREU");
#endif /* NOSETREU */
#ifdef LCKDIR
    makestr(&(optlist[noptlist++]),"LCKDIR");
#endif /* LCKDIR */
#ifdef ACUCNTRL
    makestr(&(optlist[noptlist++]),"ACUCNTRL");
#endif /* ACUCNTRL */
#ifdef BSD4
    makestr(&(optlist[noptlist++]),"BSD4");
#endif /* BSD4 */
#ifdef BSD44
    makestr(&(optlist[noptlist++]),"BSD44");
#endif /* BSD44 */
#ifdef BSD41
    makestr(&(optlist[noptlist++]),"BSD41");
#endif /* BSD41 */
#ifdef BSD43
    makestr(&(optlist[noptlist++]),"BSD43");
#endif /* BSD43 */
#ifdef BSD29
    makestr(&(optlist[noptlist++]),"BSD29");
#endif /* BSD29 */
#ifdef BSDI
    makestr(&(optlist[noptlist++]),"BSDI");
#endif /* BSDI */
#ifdef __bsdi__
    makestr(&(optlist[noptlist++]),"__bsdi__");
#endif /* __bsdi__ */
#ifdef __NetBSD__
    makestr(&(optlist[noptlist++]),"__NetBSD__");
#endif /* __NetBSD__ */
#ifdef __OpenBSD__
    makestr(&(optlist[noptlist++]),"__OpenBSD__");
#endif /* __OpenBSD__ */
#ifdef __FreeBSD__
    makestr(&(optlist[noptlist++]),"__FreeBSD__");
#endif /* __FreeBSD__ */
#ifdef FREEBSD4
    makestr(&(optlist[noptlist++]),"FREEBSD4");
#endif /* FREEBSD4 */
#ifdef FREEBSD8
    makestr(&(optlist[noptlist++]),"FREEBSD8");
#endif /* FREEBSD8 */
#ifdef FREEBSD9
    makestr(&(optlist[noptlist++]),"FREEBSD9");
#endif /* FREEBSD9 */
#ifdef __linux__
    makestr(&(optlist[noptlist++]),"__linux__");
#endif /* __linux__ */
#ifdef LINUX_HI_SPD
    makestr(&(optlist[noptlist++]),"LINUX_HI_SPD");
#endif /* LINUX_HI_SPD */
#ifdef LYNXOS
    makestr(&(optlist[noptlist++]),"LYNXOS");
#endif /* LYNXOS */
#ifdef V7
    makestr(&(optlist[noptlist++]),"V7");
#endif /* V7 */
#ifdef AIX370
    makestr(&(optlist[noptlist++]),"AIX370");
#endif /* AIX370 */
#ifdef RTAIX
    makestr(&(optlist[noptlist++]),"RTAIX");
#endif /* RTAIX */
#ifdef HPUX
    makestr(&(optlist[noptlist++]),"HPUX");
#endif /* HPUX */
#ifdef HPUX9
    makestr(&(optlist[noptlist++]),"HPUX9");
#endif /* HPUX9 */
#ifdef HPUX10
    makestr(&(optlist[noptlist++]),"HPUX10");
#endif /* HPUX10 */
#ifdef HPUX1000
    makestr(&(optlist[noptlist++]),"HPUX1000");
#endif /* HPUX1000 */
#ifdef HPUX1100
    makestr(&(optlist[noptlist++]),"HPUX1100");
#endif /* HPUX1100 */
#ifdef HPUXPRE65
    makestr(&(optlist[noptlist++]),"HPUXPRE65");
#endif /* HPUXPRE65 */
#ifdef DGUX
    makestr(&(optlist[noptlist++]),"DGUX");
#endif /* DGUX */
#ifdef DGUX430
    makestr(&(optlist[noptlist++]),"DGUX430");
#endif /* DGUX430 */
#ifdef DGUX540
    makestr(&(optlist[noptlist++]),"DGUX540");
#endif /* DGUX540 */
#ifdef DGUX543
    makestr(&(optlist[noptlist++]),"DGUX543");
#endif /* DGUX543 */
#ifdef DGUX54410
    makestr(&(optlist[noptlist++]),"DGUX54410");
#endif /* DGUX54410 */
#ifdef DGUX54411
    makestr(&(optlist[noptlist++]),"DGUX54411");
#endif /* DGUX54411 */
#ifdef sony_news
    makestr(&(optlist[noptlist++]),"sony_news");
#endif /* sony_news */
#ifdef CIE
    makestr(&(optlist[noptlist++]),"CIE");
#endif /* CIE */
#ifdef XENIX
    makestr(&(optlist[noptlist++]),"XENIX");
#endif /* XENIX */
#ifdef SCO_XENIX
    makestr(&(optlist[noptlist++]),"SCO_XENIX");
#endif /* SCO_XENIX */
#ifdef ISIII
    makestr(&(optlist[noptlist++]),"ISIII");
#endif /* ISIII */
#ifdef I386IX
    makestr(&(optlist[noptlist++]),"I386IX");
#endif /* I386IX */
#ifdef RTU
    makestr(&(optlist[noptlist++]),"RTU");
#endif /* RTU */
#ifdef PROVX1
    makestr(&(optlist[noptlist++]),"PROVX1");
#endif /* PROVX1 */
#ifdef PYRAMID
    makestr(&(optlist[noptlist++]),"PYRAMID");
#endif /* PYRAMID */
#ifdef TOWER1
    makestr(&(optlist[noptlist++]),"TOWER1");
#endif /* TOWER1 */
#ifdef UTEK
    makestr(&(optlist[noptlist++]),"UTEK");
#endif /* UTEK */
#ifdef ZILOG
    makestr(&(optlist[noptlist++]),"ZILOG");
#endif /* ZILOG */
#ifdef TRS16
    makestr(&(optlist[noptlist++]),"TRS16");
#endif /* TRS16 */
#ifdef MINIX
    makestr(&(optlist[noptlist++]),"MINIX");
#endif /* MINIX */
#ifdef MINIX2
    makestr(&(optlist[noptlist++]),"MINIX2");
#endif /* MINIX2 */
#ifdef MINIX3
    makestr(&(optlist[noptlist++]),"MINIX3");
#endif /* MINIX3 */
#ifdef MINIX315
    makestr(&(optlist[noptlist++]),"MINIX315");
#endif /* MINIX315 */
#ifdef C70
    makestr(&(optlist[noptlist++]),"C70");
#endif /* C70 */
#ifdef AIXPS2
    makestr(&(optlist[noptlist++]),"AIXPS2");
#endif /* AIXPS2 */
#ifdef AIXRS
    makestr(&(optlist[noptlist++]),"AIXRS");
#endif /* AIXRS */
#ifdef UTSV
    makestr(&(optlist[noptlist++]),"UTSV");
#endif /* UTSV */
#ifdef ATTSV
    makestr(&(optlist[noptlist++]),"ATTSV");
#endif /* ATTSV */
#ifdef SVR3
    makestr(&(optlist[noptlist++]),"SVR3");
#endif /* SVR3 */
#ifdef SVR4
    makestr(&(optlist[noptlist++]),"SVR4");
#endif /* SVR4 */
#ifdef DELL_SVR4
    makestr(&(optlist[noptlist++]),"DELL_SVR4");
#endif /* DELL_SVR4 */
#ifdef ICL_SVR4
    makestr(&(optlist[noptlist++]),"ICL_SVR4");
#endif /* ICL_SVR4 */
#ifdef OSF
    makestr(&(optlist[noptlist++]),"OSF");
#endif /* OSF */
#ifdef OSF1
    makestr(&(optlist[noptlist++]),"OSF1");
#endif /* OSF1 */
#ifdef __OSF
    makestr(&(optlist[noptlist++]),"__OSF");
#endif /* __OSF */
#ifdef __OSF__
    makestr(&(optlist[noptlist++]),"__OSF__");
#endif /* __OSF__ */
#ifdef __osf__
    makestr(&(optlist[noptlist++]),"__osf__");
#endif /* __osf__ */
#ifdef __OSF1
    makestr(&(optlist[noptlist++]),"__OSF1");
#endif /* __OSF1 */
#ifdef __OSF1__
    makestr(&(optlist[noptlist++]),"__OSF1__");
#endif /* __OSF1__ */
#ifdef PTX
    makestr(&(optlist[noptlist++]),"PTX");
#endif /* PTX */
#ifdef POSIX
    makestr(&(optlist[noptlist++]),"POSIX");
#endif /* POSIX */
#ifdef BSD44ORPOSIX
    makestr(&(optlist[noptlist++]),"BSD44ORPOSIX");
#endif /* BSD44ORPOSIX */
#ifdef SVORPOSIX
    makestr(&(optlist[noptlist++]),"SVORPOSIX");
#endif /* SVORPOSIX */
#ifdef SVR4ORPOSIX
    makestr(&(optlist[noptlist++]),"SVR4ORPOSIX");
#endif /* SVR4ORPOSIX */
#ifdef OS2ORVMS
    makestr(&(optlist[noptlist++]),"OS2ORVMS");
#endif /* OS2ORVMS */
#ifdef OS2ORUNIX
    makestr(&(optlist[noptlist++]),"OS2ORUNIX");
#endif /* OS2ORUNIX */
#ifdef VMSORUNIX
    makestr(&(optlist[noptlist++]),"VMSORUNIX");
#endif /* VMSORUNIX */
#ifdef VMS64BIT
    makestr(&(optlist[noptlist++]),"VMS64BIT");	/* VMS on Alpha or IA64 */
#endif /* VMS64BIT */
#ifdef VMSI64
    makestr(&(optlist[noptlist++]),"VMSI64"); /* VMS on IA64 */
#endif /* VMSI64 */
#ifdef _POSIX_SOURCE
    makestr(&(optlist[noptlist++]),"_POSIX_SOURCE");
#endif /* _POSIX_SOURCE */
#ifdef _XOPEN_SOURCE
    makestr(&(optlist[noptlist++]),"_XOPEN_SOURCE");
#endif
#ifdef _ALL_SOURCE
    makestr(&(optlist[noptlist++]),"_ALL_SOURCE");
#endif
#ifdef _SVID3
    makestr(&(optlist[noptlist++]),"_SVID3");
#endif /* _SVID3 */
#ifdef Plan9
    makestr(&(optlist[noptlist++]),"Plan9");
#endif /* Plan9 */
#ifdef SOLARIS
    makestr(&(optlist[noptlist++]),"SOLARIS");
#ifdef SOLARIS24
    makestr(&(optlist[noptlist++]),"SOLARIS24");
#endif /* SOLARIS24 */
#ifdef SOLARIS25
    makestr(&(optlist[noptlist++]),"SOLARIS25");
#endif /* SOLARIS25 */
#ifdef SOLARIS26
    makestr(&(optlist[noptlist++]),"SOLARIS26");
#endif /* SOLARIS26 */
#ifdef SOLARIS7
    makestr(&(optlist[noptlist++]),"SOLARIS7");
#endif /* SOLARIS7 */
#ifdef SOLARIS8
    makestr(&(optlist[noptlist++]),"SOLARIS8");
#endif /* SOLARIS8 */
#ifdef SOLARIS9
    makestr(&(optlist[noptlist++]),"SOLARIS9");
#endif /* SOLARIS9 */
#ifdef SOLARIS10
    makestr(&(optlist[noptlist++]),"SOLARIS10");
#endif /* SOLARIS10 */
#endif /* SOLARIS */

#ifdef SUNOS4
    makestr(&(optlist[noptlist++]),"SUNOS4");
#endif /* SUNOS4 */
#ifdef SUN4S5
    makestr(&(optlist[noptlist++]),"SUN4S5");
#endif /* SUN4S5 */
#ifdef IRIX
    makestr(&(optlist[noptlist++]),"IRIX");
#endif /* IRIX */
#ifdef ENCORE
    makestr(&(optlist[noptlist++]),"ENCORE");
#endif /* ENCORE */
#ifdef ultrix
    makestr(&(optlist[noptlist++]),"ultrix");
#endif
#ifdef sxaE50
    makestr(&(optlist[noptlist++]),"sxaE50");
#endif
#ifdef mips
    makestr(&(optlist[noptlist++]),"mips");
#endif
#ifdef MIPS
    makestr(&(optlist[noptlist++]),"MIPS");
#endif
#ifdef vax
    makestr(&(optlist[noptlist++]),"vax");
#endif
#ifdef VAX
    makestr(&(optlist[noptlist++]),"VAX");
#endif
#ifdef alpha
    makestr(&(optlist[noptlist++]),"alpha");
#endif
#ifdef ALPHA
    makestr(&(optlist[noptlist++]),"ALPHA");
#endif
#ifdef __ALPHA
    makestr(&(optlist[noptlist++]),"__ALPHA");
#endif
#ifdef __alpha
    makestr(&(optlist[noptlist++]),"__alpha");
#endif
#ifdef __AXP
    makestr(&(optlist[noptlist++]),"__AXP");
#endif
#ifdef AXP
    makestr(&(optlist[noptlist++]),"AXP");
#endif
#ifdef axp
    makestr(&(optlist[noptlist++]),"axp");
#endif
#ifdef __ALPHA__
    makestr(&(optlist[noptlist++]),"__ALPHA__");
#endif
#ifdef __alpha__
    makestr(&(optlist[noptlist++]),"__alpha__");
#endif
#ifdef sun
    makestr(&(optlist[noptlist++]),"sun");
#endif
#ifdef sun3
    makestr(&(optlist[noptlist++]),"sun3");
#endif
#ifdef sun386
    makestr(&(optlist[noptlist++]),"sun386");
#endif
#ifdef _SUN
    makestr(&(optlist[noptlist++]),"_SUN");
#endif
#ifdef sun4
    makestr(&(optlist[noptlist++]),"sun4");
#endif
#ifdef sparc
    makestr(&(optlist[noptlist++]),"sparc");
#endif
#ifdef _CRAY
    makestr(&(optlist[noptlist++]),"_CRAY");
#endif /* _CRAY */
#ifdef NEXT33
    makestr(&(optlist[noptlist++]),"NEXT33");
#endif
#ifdef NEXT
    makestr(&(optlist[noptlist++]),"NEXT");
#endif
#ifdef NeXT
    makestr(&(optlist[noptlist++]),"NeXT");
#endif
#ifdef MACH
    makestr(&(optlist[noptlist++]),"MACH");
#endif

#ifdef MACOSX
    makestr(&(optlist[noptlist++]),"MACOSX");
#endif
#ifdef MACOSX10
    makestr(&(optlist[noptlist++]),"MACOSX10");
#endif
#ifdef MACOSX103
    makestr(&(optlist[noptlist++]),"MACOSX10e");
#endif
#ifdef COMMENT
/* not used */
#ifdef MACOSX103
    makestr(&(optlist[noptlist++]),"MACOSX103");
#endif
#endif	/* COMMENT */

#ifdef sgi
    makestr(&(optlist[noptlist++]),"sgi");
#endif
#ifdef M_SYS5
    makestr(&(optlist[noptlist++]),"M_SYS5");
#endif
#ifdef __SYSTEM_FIVE
    makestr(&(optlist[noptlist++]),"__SYSTEM_FIVE");
#endif
#ifdef sysV
    makestr(&(optlist[noptlist++]),"sysV");
#endif
#ifdef M_XENIX                          /* SCO Xenix V and UNIX/386 */
    makestr(&(optlist[noptlist++]),"M_XENIX");
#endif
#ifdef M_UNIX                           /* SCO UNIX */
    makestr(&(optlist[noptlist++]),"M_UNIX");
#endif
#ifdef _M_UNIX                          /* SCO UNIX 3.2v4 = ODT 2.0 */
    makestr(&(optlist[noptlist++]),"_M_UNIX");
#endif
#ifdef CK_SCOV5
    makestr(&(optlist[noptlist++]),"CK_SCOV5");
#endif
#ifdef SCO_OSR504
    makestr(&(optlist[noptlist++]),"SCO_OSR504");
#endif
#ifdef M_IA64
    makestr(&(optlist[noptlist++]),"M_IA64");
#endif
#ifdef _M_IA64
    makestr(&(optlist[noptlist++]),"_M_IA64");
#endif
#ifdef ia64
    makestr(&(optlist[noptlist++]),"ia64");
#endif
#ifdef _ia64
    makestr(&(optlist[noptlist++]),"_ia64");
#endif
#ifdef _ia64_
    makestr(&(optlist[noptlist++]),"_ia64_");
#endif
#ifdef __ia64
    makestr(&(optlist[noptlist++]),"__ia64");
#endif
#ifdef M_I686
    makestr(&(optlist[noptlist++]),"M_I686");
#endif
#ifdef _M_I686
    makestr(&(optlist[noptlist++]),"_M_I686");
#endif
#ifdef i686
    makestr(&(optlist[noptlist++]),"i686");
#endif
#ifdef M_I586
    makestr(&(optlist[noptlist++]),"M_I586");
#endif
#ifdef _M_I586
    makestr(&(optlist[noptlist++]),"_M_I586");
#endif
#ifdef i586
    makestr(&(optlist[noptlist++]),"i586");
#endif
#ifdef M_I486
    makestr(&(optlist[noptlist++]),"M_I486");
#endif
#ifdef _M_I486
    makestr(&(optlist[noptlist++]),"_M_I486");
#endif
#ifdef i486
    makestr(&(optlist[noptlist++]),"i486");
#endif
#ifdef M_I386
    makestr(&(optlist[noptlist++]),"M_I386");
#endif
#ifdef _M_I386
    makestr(&(optlist[noptlist++]),"_M_I386");
#endif
#ifdef i386
    makestr(&(optlist[noptlist++]),"i386");
#endif
#ifdef __i386
    makestr(&(optlist[noptlist++]),"__i386");
#endif
#ifdef __x86
    makestr(&(optlist[noptlist++]),"__x86");
#endif
#ifdef __amd64
    makestr(&(optlist[noptlist++]),"__amd64");
#endif
#ifdef _ILP32
    makestr(&(optlist[noptlist++]),"_ILP32");
#endif
#ifdef _ILP64
    makestr(&(optlist[noptlist++]),"_ILP64");
#endif
#ifdef _LP32
    makestr(&(optlist[noptlist++]),"_LP32");
#endif
#ifdef _LP64
    makestr(&(optlist[noptlist++]),"_LP64");
#endif
#ifdef __LP32__
    makestr(&(optlist[noptlist++]),"__LP32__");
#endif
#ifdef __LP64__
    makestr(&(optlist[noptlist++]),"__LP64__");
#endif
#ifdef _XGP4_2
    makestr(&(optlist[noptlist++]),"_XGP4_2");
#endif
#ifdef __ppc__
    makestr(&(optlist[noptlist++]),"__ppc__");
#endif
#ifdef __ppc32__
    makestr(&(optlist[noptlist++]),"__ppc32__");
#endif
#ifdef __ppc64__
    makestr(&(optlist[noptlist++]),"__ppc64__");
#endif
#ifdef CK_64BIT
    makestr(&(optlist[noptlist++]),"CK_64BIT");
#endif
#ifdef i286
    makestr(&(optlist[noptlist++]),"i286");
#endif
#ifdef M_I286
    makestr(&(optlist[noptlist++]),"M_I286");
#endif
#ifdef __sparc
    makestr(&(optlist[noptlist++]),"__sparc");
#endif
#ifdef __sparcv8
    makestr(&(optlist[noptlist++]),"__sparcv8");
#endif
#ifdef __sparcv9
    makestr(&(optlist[noptlist++]),"__sparcv9");
#endif
#ifdef mc68000
    makestr(&(optlist[noptlist++]),"mc68000");
#endif
#ifdef mc68010
    makestr(&(optlist[noptlist++]),"mc68010");
#endif
#ifdef mc68020
    makestr(&(optlist[noptlist++]),"mc68020");
#endif
#ifdef mc68030
    makestr(&(optlist[noptlist++]),"mc68030");
#endif
#ifdef mc68040
    makestr(&(optlist[noptlist++]),"mc68040");
#endif
#ifdef M_68000
    makestr(&(optlist[noptlist++]),"M_68000");
#endif
#ifdef M_68010
    makestr(&(optlist[noptlist++]),"M_68010");
#endif
#ifdef M_68020
    makestr(&(optlist[noptlist++]),"M_68020");
#endif
#ifdef M_68030
    makestr(&(optlist[noptlist++]),"M_68030");
#endif
#ifdef M_68040
    makestr(&(optlist[noptlist++]),"M_68040");
#endif
#ifdef m68k
    makestr(&(optlist[noptlist++]),"m68k");
#endif
#ifdef m88k
    makestr(&(optlist[noptlist++]),"m88k");
#endif
#ifdef pdp11
    makestr(&(optlist[noptlist++]),"pdp11");
#endif
#ifdef iAPX
    makestr(&(optlist[noptlist++]),"iAPX");
#endif
#ifdef hpux
    makestr(&(optlist[noptlist++]),"hpux");
#endif
#ifdef __hpux
    makestr(&(optlist[noptlist++]),"__hpux");
#endif
#ifdef __hp9000s800
    makestr(&(optlist[noptlist++]),"__hp9000s800");
#endif
#ifdef __hp9000s700
    makestr(&(optlist[noptlist++]),"__hp9000s700");
#endif
#ifdef __hp9000s500
    makestr(&(optlist[noptlist++]),"__hp9000s500");
#endif
#ifdef __hp9000s300
    makestr(&(optlist[noptlist++]),"__hp9000s300");
#endif
#ifdef __hp9000s200
    makestr(&(optlist[noptlist++]),"__hp9000s200");
#endif
#ifdef AIX
    makestr(&(optlist[noptlist++]),"AIX");
#endif
#ifdef _AIXFS
    makestr(&(optlist[noptlist++]),"_AIXFS");
#endif
#ifdef u370
    makestr(&(optlist[noptlist++]),"u370");
#endif
#ifdef u3b
    makestr(&(optlist[noptlist++]),"u3b");
#endif
#ifdef u3b2
    makestr(&(optlist[noptlist++]),"u3b2");
#endif
#ifdef multimax
    makestr(&(optlist[noptlist++]),"multimax");
#endif
#ifdef balance
    makestr(&(optlist[noptlist++]),"balance");
#endif
#ifdef ibmrt
    makestr(&(optlist[noptlist++]),"ibmrt");
#endif
#ifdef _IBMRT
    makestr(&(optlist[noptlist++]),"_IBMRT");
#endif
#ifdef ibmrs6000
    makestr(&(optlist[noptlist++]),"ibmrs6000");
#endif
#ifdef _AIX
    makestr(&(optlist[noptlist++]),"_AIX");
#endif /* _AIX */
#ifdef _IBMR2
    makestr(&(optlist[noptlist++]),"_IBMR2");
#endif
#ifdef UNIXWARE
    makestr(&(optlist[noptlist++]),"UNIXWARE");
#endif
#ifdef QNX
    makestr(&(optlist[noptlist++]),"QNX");
#ifdef __QNX__
    makestr(&(optlist[noptlist++]),"__QNX__");
#ifdef __16BIT__
    makestr(&(optlist[noptlist++]),"__16BIT__");
#endif
#ifdef CK_QNX16
    makestr(&(optlist[noptlist++]),"CK_QNX16");
#endif
#ifdef __32BIT__
    makestr(&(optlist[noptlist++]),"__32BIT__");
#endif
#ifdef CK_QNX32
    makestr(&(optlist[noptlist++]),"CK_QNX32");
#endif
#endif /* __QNX__ */
#endif /* QNX */

#ifdef QNX6
    makestr(&(optlist[noptlist++]),"QNX6");
#endif /* QNX6 */

#ifdef NEUTRINO
    makestr(&(optlist[noptlist++]),"NEUTRINO");
#endif /* NEUTRINO */

#ifdef __STRICT_BSD__
    makestr(&(optlist[noptlist++]),"__STRICT_BSD__");
#endif
#ifdef __STRICT_ANSI__
    makestr(&(optlist[noptlist++]),"__STRICT_ANSI__");
#endif
#ifdef _ANSI_C_SOURCE
    makestr(&(optlist[noptlist++]),"_ANSI_C_SOURCE");
#endif
#ifdef __STDC__
    makestr(&(optlist[noptlist++]),"__STDC__");
#endif
#ifdef cplusplus
    makestr(&(optlist[noptlist++]),"cplusplus");
#endif
#ifdef __DECC
    makestr(&(optlist[noptlist++]),"__DECC");
#ifdef __DECC_VER
    sprintf(line,"__DECC_VER=%d",__DECC_VER); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* __DECC_VER */
#endif /* __DECC */
#ifdef __CRTL_VER
    sprintf(line,"__CRTL_VER=%d",__CRTL_VER); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* __CRTL_VER */
#ifdef __GNUC__                         /* gcc in ansi mode */
    makestr(&(optlist[noptlist++]),"__GNUC__");
#endif
#ifdef GNUC                             /* gcc in traditional mode */
    makestr(&(optlist[noptlist++]),"GNUC");
#endif
#ifdef __EGCS__                         /* egcs in ansi mode */
    makestr(&(optlist[noptlist++]),"__EGCS__");
#endif
#ifdef __egcs__                         /* egcs in ansi mode */
    makestr(&(optlist[noptlist++]),"__egcs__");
#endif
#ifdef __WATCOMC__
    makestr(&(optlist[noptlist++]),"__WATCOMC__");
#endif
#ifdef CK_ANSIC
    makestr(&(optlist[noptlist++]),"CK_ANSIC");
#endif
#ifdef CK_ANSILIBS
    makestr(&(optlist[noptlist++]),"CK_ANSILIBS");
#endif
#ifdef CKCONINTB4CB
    makestr(&(optlist[noptlist++]),"CKCONINTB4CB");
#endif /* CKCONINTB4CB */
#ifdef NOTERMCAP
    makestr(&(optlist[noptlist++]),"NOTERMCAP");
#endif /* NOTERMCAP */
#ifdef __GLIBC__
    makestr(&(optlist[noptlist++]),"__GLIBC__");
#endif
#ifdef _SC_JOB_CONTROL
    makestr(&(optlist[noptlist++]),"_SC_JOB_CONTROL");
#endif
#ifdef _POSIX_JOB_CONTROL
    makestr(&(optlist[noptlist++]),"_POSIX_JOB_CONTROL");
#endif
#ifdef SIG_I
    makestr(&(optlist[noptlist++]),"SIG_I");
#endif /* SIG_I */
#ifdef SIG_V
    makestr(&(optlist[noptlist++]),"SIG_V");
#endif /* SIG_V */
#ifdef CK_POSIX_SIG
    makestr(&(optlist[noptlist++]),"CK_POSIX_SIG");
#endif
#ifdef SVR3JC
    makestr(&(optlist[noptlist++]),"SVR3JC");
#endif
#ifdef _386BSD
    makestr(&(optlist[noptlist++]),"_386BSD");
#endif
#ifdef _BSD
    makestr(&(optlist[noptlist++]),"_BSD");
#endif
#ifdef USE_MEMCPY
    makestr(&(optlist[noptlist++]),"USE_MEMCPY");
#endif /* USE_MEMCPY */
#ifdef USE_LSTAT
    makestr(&(optlist[noptlist++]),"USE_LSTAT");
#endif /* USE_LSTAT */
#ifdef TERMIOX
    makestr(&(optlist[noptlist++]),"TERMIOX");
#endif /* TERMIOX */
#ifdef STERMIOX
    makestr(&(optlist[noptlist++]),"STERMIOX");
#endif /* STERMIOX */
#ifdef CK_CURSES
    makestr(&(optlist[noptlist++]),"CK_CURSES");
#endif /* CK_CURSES */
#ifdef CK_NEWTERM
    makestr(&(optlist[noptlist++]),"CK_NEWTERM");
#endif /* CK_NEWTERM */
#ifdef CK_WREFRESH
    makestr(&(optlist[noptlist++]),"CK_WREFRESH");
#endif /* CK_WREFRESH */
#ifdef CK_PCT_BAR
    makestr(&(optlist[noptlist++]),"CK_PCT_BAR");
#endif /* CK_PCT_BAR */
#ifdef CK_DTRCD
    makestr(&(optlist[noptlist++]),"CK_DTRCD");
#endif /* CK_DTRCD */
#ifdef CK_DTRCTS
    makestr(&(optlist[noptlist++]),"CK_DTRCTS");
#endif /* CK_DTRCTS */
#ifdef CK_RTSCTS
    makestr(&(optlist[noptlist++]),"CK_RTSCTS");
#endif /* CK_RTSCTS */
#ifdef POSIX_CRTSCTS
    makestr(&(optlist[noptlist++]),"POSIX_CRTSCTS");
#endif /* POSIX_CRTSCTS */
#ifdef FIXCRTSCTS
    makestr(&(optlist[noptlist++]),"FIXCRTSCTS");
#endif /* FIXCRTSCTS */
#ifdef HWPARITY
    makestr(&(optlist[noptlist++]),"HWPARITY");
#endif /* HWPARITY */
#ifdef CK_SYSINI
#ifdef CK_INI_A
    makestr(&(optlist[noptlist++]),"CK_INI_A");
    ckmakmsg(line,LINBUFSIZ,"CK_SYSINI=\"",CK_SYSINI,"\"",NULL);
    makestr(&(optlist[noptlist++]),line);
#else
#ifdef CK_INI_B
    makestr(&(optlist[noptlist++]),"CK_INI_B");
    ckmakmsg(line,LINBUFSIZ,"CK_SYSINI=\"",CK_SYSINI,"\"",NULL);
    makestr(&(optlist[noptlist++]),line);
#else
    makestr(&(optlist[noptlist++]),"CK_SYSINI");
#endif /* CK_INI_B */
#endif /* CK_INI_A */
#endif /* CK_DSYSINI */
#ifdef CK_DSYSINI
    makestr(&(optlist[noptlist++]),"CK_DSYSINI");
#endif /* CK_DSYSINI */
#ifdef CK_TTGWSIZ
    makestr(&(optlist[noptlist++]),"CK_TTGWSIZ");
#endif /* CK_TTGWSIZ */
#ifdef CK_NAWS
    makestr(&(optlist[noptlist++]),"CK_NAWS");
#endif /* CK_NAWS */
#ifdef MDMHUP
    makestr(&(optlist[noptlist++]),"MDMHUP");
#endif /* MDMHUP */
#ifdef HUP_CLOSE_POSIX
    makestr(&(optlist[noptlist++]),"HUP_CLOSE_POSIX");
#endif /* HUP_CLOSE_POSIX */
#ifdef NO_HUP_CLOSE_POSIX
    makestr(&(optlist[noptlist++]),"NO_HUP_CLOSE_POSIX");
#endif /* NO_HUP_CLOSE_POSIX */
#ifdef DCMDBUF
    makestr(&(optlist[noptlist++]),"DCMDBUF");
#endif /* DCMDBUF */
#ifdef CK_RECALL
    makestr(&(optlist[noptlist++]),"CK_RECALL");
#endif /* CK_RECALL */
#ifdef BROWSER
    makestr(&(optlist[noptlist++]),"BROWSER");
#endif /* BROWSER */
#ifdef CLSOPN
    makestr(&(optlist[noptlist++]),"CLSOPN");
#endif /* CLSOPN */
#ifdef STRATUS
    makestr(&(optlist[noptlist++]),"STRATUS");
#endif /* STRATUS */
#ifdef __VOS__
    makestr(&(optlist[noptlist++]),"__VOS__");
#endif /* __VOS__ */
#ifdef STRATUSX25
    makestr(&(optlist[noptlist++]),"STRATUSX25");
#endif /* STRATUSX25 */
#ifdef OS2MOUSE
    makestr(&(optlist[noptlist++]),"OS2MOUSE");
#endif /* OS2MOUSE */
#ifdef CK_REXX
    makestr(&(optlist[noptlist++]),"CK_REXX");
#endif /* CK_REXX */
#ifdef CK_TIMERS
    makestr(&(optlist[noptlist++]),"CK_TIMERS");
#endif /* CK_TIMERS */
#ifdef TTSPDLIST
    makestr(&(optlist[noptlist++]),"TTSPDLIST");
#endif /* TTSPDLIST */
#ifdef CK_PERMS
    makestr(&(optlist[noptlist++]),"CK_PERMS");
#endif /* CK_PERMS */
#ifdef CKTUNING
    makestr(&(optlist[noptlist++]),"CKTUNING");
#endif /* CKTUNING */
#ifdef NEWFTP
    makestr(&(optlist[noptlist++]),"NEWFTP");
#endif /* NEWFTP */
#ifdef SYSFTP
    makestr(&(optlist[noptlist++]),"SYSFTP");
#endif /* SYSFTP */
#ifdef NOFTP
    makestr(&(optlist[noptlist++]),"NOFTP");
#endif /* NOFTP */
#ifdef CKHTTP
    makestr(&(optlist[noptlist++]),"CKHTTP");
#endif /* CKHTTP */
#ifdef NOHTTP
    makestr(&(optlist[noptlist++]),"NOHTTP");
#endif /* NOHTTP */
#ifdef CKROOT
    makestr(&(optlist[noptlist++]),"CKROOT");
#endif /* CKROOT */
#ifdef CKREALPATH
    makestr(&(optlist[noptlist++]),"CKREALPATH");
#endif /* CKREALPATH */
#ifdef STREAMING
    makestr(&(optlist[noptlist++]),"STREAMING");
#endif /* STREAMING */
#ifdef UNPREFIXZERO
    makestr(&(optlist[noptlist++]),"UNPREFIXZERO");
#endif /* UNPREFIXZERO */
#ifdef CKREGEX
    makestr(&(optlist[noptlist++]),"CKREGEX");
#endif /* CKREGEX */
#ifdef ZXREWIND
    makestr(&(optlist[noptlist++]),"ZXREWIND");
#endif /* ZXREWIND */
#ifdef CKSYSLOG
    makestr(&(optlist[noptlist++]),"CKSYSLOG");
#endif /* CKSYSLOG */
#ifdef SYSLOGLEVEL
    sprintf(line,"SYSLOGLEVEL=%d",SYSLOGLEVEL); /* SAFE */
    makestr(&(optlist[noptlist++]),line);
#endif /* SYSLOGLEVEL */
#ifdef NOSEXP
    makestr(&(optlist[noptlist++]),"NOSEXP");
#endif /* NOSEXP */
#ifdef CKLEARN
    makestr(&(optlist[noptlist++]),"CKLEARN");
#else
#ifdef NOLOEARN
    makestr(&(optlist[noptlist++]),"NOLOEARN");
#endif /* NOLOEARN */
#endif /* CKLEARN */

#ifdef NOFLOAT
    makestr(&(optlist[noptlist++]),"NOFLOAT");
#else
#ifdef FNFLOAT
    makestr(&(optlist[noptlist++]),"FNFLOAT");
#endif /* FNFLOAT */
#ifdef CKFLOAT
#ifdef GFTIMER
    makestr(&(optlist[noptlist++]),"GFTIMER");
#endif /* GFTIMER */
#ifdef CKFLOAT_S
    ckmakmsg(line,LINBUFSIZ,"CKFLOAT=",CKFLOAT_S,NULL,NULL);
    makestr(&(optlist[noptlist++]),line);
#else
    makestr(&(optlist[noptlist++]),"CKFLOAT");
#endif /* CKFLOAT_S */
#endif /* CKFLOAT */
#endif /* NOFLOAT */

#ifdef SSH
    makestr(&(optlist[noptlist++]),"SSH");
#endif /* SSH */
#ifdef NETDLL
    makestr(&(optlist[noptlist++]),"NETDLL");
#endif /* NETDLL */
#ifdef NETFILE
    makestr(&(optlist[noptlist++]),"NETFILE");
#endif /* NETFILE */
#ifdef CK_TAPI
    makestr(&(optlist[noptlist++]),"CK_TAPI");
#endif /* CK_TAPI */
#ifdef CK_SSL
    makestr(&(optlist[noptlist++]),"CK_SSL");
#ifdef OPENSSL_VERSION_TEXT
    ckmakmsg(line,LINBUFSIZ,
	     "OPENSSL_VERSION_TEXT=","\"",OPENSSL_VERSION_TEXT,"\"");
    makestr(&(optlist[noptlist++]),line);
#endif	/* OPENSSL_VERSION_TEXT */
#endif /* CK_SSL */
    debug(F101,"initoptlist noptlist","",noptlist);
    sh_sort(optlist,NULL,noptlist,0,0,0);
}

int
shofea() {
    int i;
    int flag = 0;
    int lines = 1;
#ifdef FNFLOAT
    extern int fp_digits, fp_rounding;
#endif /* FNFLOAT */
    extern int byteorder;
    printf("%s\n",versio);
    if (inserver)
      return(1);

    debug(F101,"shofea NOPTLIST","",NOPTLIST);
    initoptlist();
    debug(F101,"shofea noptlist","",noptlist);
#ifdef OS2
#ifdef NT
#ifdef _M_ALPHA
    printf("Microsoft Windows Operating Systems for Alpha CPUs.\n");
#else /* _M_ALPHA */
#ifdef _M_PPC
    printf("Microsoft Windows Operating Systems for PowerPC CPUs.\n");
#else /* _M_PPC */
#ifdef _M_MRX000
    printf("Microsoft Windows Operating Systems for MIPS CPUs.\n");
#else /* _M_MRX000 */
#ifdef _M_IX86
    printf("Microsoft Windows Operating Systems for 32-bit Intel CPUs.\n");
#else /* _M_IX86 */
    UNKNOWN WINDOWS PLATFORM
#endif /* _M_IX86 */
#endif /* _M_MRX000 */
#endif /* _M_PPC */
#endif /* _M_ALPHA */
#else /* NT */
#ifdef M_I286
    printf("IBM OS/2 16-bit.\n");
#else
    printf("IBM OS/2 32-bit.\n");
#endif /* M_I286 */
#endif /* NT */
    lines++;
#endif /* OS2 */
    printf("\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    printf("Major optional features included:\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }

    if (sizeof(CK_OFF_T) == 8) {
	printf(" Large files and large integers (64 bits)\n");
        if (++lines > cmd_rows - 3) {
	    if (!askmore()) return(1); else lines = 0;
	}
    }
#ifdef NETCONN
    printf(" Network support (type SHOW NET for further info)\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#ifdef IKS_OPTION
    printf(" Telnet Kermit Option\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* IKS_OPTION */
#ifdef CK_AUTHENTICATION
    printf(" Telnet Authentication Option\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#ifdef CK_KERBEROS
#ifdef KRB4
#ifdef KRB5
    printf(" Kerberos(TM) IV and Kerberos V authentication\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#else /* KRB5 */
    printf(" Kerberos(TM) IV authentication\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* KRB5 */
#else /* KRB4 */
#ifdef KRB5
    printf(" Kerberos(TM) V authentication\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* KRB5 */
#endif /* KRB4 */
#endif /* CK_KERBEROS */
#ifdef CK_SRP
    printf(" SRP(TM) (Secure Remote Password) authentication\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* CK_SRP */
#ifdef CK_SSL
    printf(" Secure Sockets Layer (SSL)\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    printf(" Transport Layer Security (TLS)\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* CK_SSL */
#ifdef SSHBUILTIN
    printf(" Secure Shell (SSH) [internal]\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* SSHBUILTIN */
#ifdef SSHCMD
    printf(" Secure Shell (SSH) [external]\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* SSHCMD */
#ifdef CK_ENCRYPTION
    printf(" Telnet Encryption Option\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#ifdef CK_DES
    printf(" Telnet DES Encryption\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* CK_DES */
#ifdef CK_CAST
    printf(" Telnet CAST Encryption\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* CK_CAST */

#ifdef CK_KERBEROS
#ifdef KRB5
#ifdef ALLOW_KRB_3DES_ENCRYPT
    printf(" Kerberos 3DES/AES Telnet Encryption\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* ALLOW_KRB_3DES_ENCRYPT */
#endif /* KRB5 */
#endif /* CK_KERBEROS */

#endif /* CK_ENCRYPTION */
#endif /* CK_AUTHENTICATION */
#ifdef CK_FORWARD_X
    printf(" X Windows forwarding\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* CK_FORWARD_X */
#ifdef TN_COMPORT
    printf(" Telnet Remote Com Port Control Option\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* TN_COMPORT */
#ifdef CK_SOCKS
#ifdef CK_SOCKS5
    printf(" SOCKS 5\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#else /* CK_SOCKS5 */
    printf(" SOCKS 4\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* CK_SOCKS5 */
#endif /* CK_SOCKS */
#ifdef NEWFTP
    printf(" Built-in FTP client\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* NEWFTP */
#ifdef CKHTTP
    printf(" Built-in HTTP client\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* CKHTTP */
#endif /* NETCONN */

#ifdef CK_RTSCTS
    printf(" Hardware flow control\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* CK_RTSCTS */

#ifdef CK_XYZ
#ifdef XYZ_INTERNAL
    printf(" Built-in XYZMODEM protocols\n");
#else
    printf(" External XYZMODEM protocol support\n");
#endif /* XYZ_INTERNAL */
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* CK_XYZ */

#ifndef NOCSETS
    printf(" Latin-1 (West European) character-set translation\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#ifdef LATIN2
    printf(" Latin-2 (East European) character-set translation\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* LATIN2 */
#ifdef CYRILLIC
    printf(" Cyrillic (Russian, Ukrainian, etc) character-set translation\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* CYRILLIC */
#ifdef GREEK
    printf(" Greek character-set translation\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* GREEK */
#ifdef HEBREW
    printf(" Hebrew character-set translation\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* HEBREW */
#ifdef KANJI
    printf(" Japanese character-set translation\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* KANJI */
#ifdef UNICODE
    printf(" Unicode character-set translation\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* UNICODE */
#ifdef CKOUNI
    if (isunicode())
      printf(" Unicode support for ISO-2022 Terminal Emulation\n");
    else
      printf(" Unicode translation for Terminal Character-Sets\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* CKOUNI */
#endif /* NOCSETS */

#ifdef NETPTY
    printf(" Pseudoterminal control\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* NETPTY */

#ifdef CK_REDIR
    printf(" REDIRECT command\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* CK_REDIR */

#ifdef CK_RESEND
    printf(" RESEND command\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* CK_RESEND */

#ifndef NOXFER
#ifdef CK_CURSES
    printf(" Fullscreen file transfer display\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* CK_CURSES */
#endif /* NOXFER */

#ifdef CK_SPEED
    printf(" Control-character unprefixing\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* CK_SPEED */

#ifdef STREAMING
    printf(" Streaming\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* STREAMING */

#ifdef CK_AUTODL
    printf(" Autodownload\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* CK_AUTODL */

#ifdef OS2MOUSE
    printf(" Mouse support\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* OS2MOUSE */

#ifdef CK_REXX
    printf(" REXX script language interface\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* CK_REXX */

#ifdef IKSD
#ifdef CK_LOGIN
    printf(" Internet Kermit Service with user login support\n");
#else /* CK_LOGIN */
    printf(" Internet Kermit Service without user login support\n");
#endif /* CK_LOGIN */
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* IKSD */

    printf("\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    printf("Major optional features not included:\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }

    if (sizeof(CK_OFF_T) <= 4) {
	printf(" No large files or large integers\n");
        if (++lines > cmd_rows - 3) {
	    if (!askmore()) return(1); else lines = 0;
	}
    }

#ifdef NOXFER
    printf(" No file-transfer protocols\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#else
#ifndef CK_CURSES
#ifndef MAC
    printf(" No fullscreen file transfer display\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* MAC */
#endif /* CK_CURSES */

#ifdef NOSERVER
    printf(" No server mode\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* NOSERVER */

#ifndef CK_SPEED
    printf(" No control-character unprefixing\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* CK_SPEED */

#ifndef STREAMING
    printf(" No streaming\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* STREAMING */

#ifndef CK_AUTODL
    printf(" No autodownload\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* CK_AUTODL */

#ifndef CK_XYZ
    printf(" No built-in XYZMODEM protocols\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* CK_XYZ */

#ifdef NOFLOAT
    printf(" No floating-point arithmetic\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
    printf(" No S-Expressions (LISP interpreter)\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#else
#ifdef NOSEXP
    printf(" No S-Expressions (LISP interpreter)\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif	/* NOSEXP */
#endif	/* NOFLOAT */

#ifdef NOTLOG
    printf(" No transaction log\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* NOTLOG */
#endif /* NOXFER */

#ifdef NODEBUG
    printf(" No debugging\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* NODEBUG */

#ifdef NOHELP
    printf(" No built-in help\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* NOHELP */

#ifdef NOLOCAL
    printf(" No making connections\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#else
#ifndef NETCONN
    printf(" No network support\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#else /* NETCONN */
#ifndef IKS_OPTION
    printf(" No Telnet Kermit Option\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* IKS_OPTION */
#endif /* NETCONN */

#ifdef NOSSH
    printf(" No Secure Shell (SSH)\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* NOSSH */
#ifndef CK_AUTHENTICATION
    printf(" No Kerberos(TM) authentication\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    printf(" No SRP(TM) (Secure Remote Password) protocol\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    printf(" No Secure Sockets Layer (SSL) protocol\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    printf(" No Transport Layer Security (TLS) protocol\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    printf(" No encryption\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#else /* CK_AUTHENTICATION */
#ifndef CK_KERBEROS
    printf(" No Kerberos(TM) authentication\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#else /* CK_KERBEROS */
#ifndef KRB4
    printf(" No Kerberos(TM) IV authentication\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* KRB4 */
#ifndef KRB5
    printf(" No Kerberos(TM) V authentication\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* KRB5 */
#endif /* CK_KERBEROS */
#ifndef CK_SRP
    printf(" No SRP(TM) (Secure Remote Password) authentication\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* CK_SRP */
#ifndef CK_SSL
    printf(" No Secure Sockets Layer (SSL) protocol\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    printf(" No Transport Layer Security (TLS) protocol\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* CK_SSL */
#ifndef CK_ENCRYPTION
    printf(" No Telnet Encryption Option\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#else /* CK_ENCRYPTION */
#ifndef OS2
#ifndef CK_DES
    printf(" No Telnet DES encryption\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* CK_DES */
#ifndef CK_CAST
    printf(" No Telnet CAST encryption\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* CK_CAST */

#ifdef COMMENT
#ifdef CK_KERBEROS
#ifdef KRB5
#ifndef ALLOW_KRB_3DES_ENCRYPT
    printf(" No Kerberos 3DES/AES Telnet Encryption\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* ALLOW_KRB_3DES_ENCRYPT */
#endif /* KRB5 */
#endif /* CK_KERBEROS */
#endif /* COMMENT */

#endif /* OS2 */
#endif /* CK_ENCRYPTION */
#endif /* CK_AUTHENTICATION */
#ifndef CK_FORWARD_X
    printf(" No X Windows forwarding\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* CK_FORWARD_X */
#ifndef TN_COMPORT
    printf(" No Telnet Remote Com Port Control Option\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* TN_COMPORT */
#ifndef CK_SOCKS
    printf(" No SOCKS\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* CK_SOCKS */
#ifndef NEWFTP
    printf(" No built-in FTP client\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* NEWFTP */
#ifdef NOHTTP
    printf(" No built-in HTTP client\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* NOHTTP */

#ifdef NODIAL
    printf(" No DIAL command\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#else
#ifdef MINIDIAL
    printf(" Support for most modem types excluded\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* MINIDIAL */
#endif /* NODIAL */
#endif /* NOLOCAL */

#ifndef CK_RTSCTS
#ifndef MAC
    printf(" No hardware flow control\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* MAC */
#endif /* CK_RTSCTS */

#ifdef NOXMIT
    printf(" No TRANSMIT command\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* NOXMIT */

#ifdef NOSCRIPT
    printf(" No SCRIPT command\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* NOSCRIPT */

#ifdef NOSPL
    printf(" No script programming features\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* NOSPL */

#ifdef NOCSETS
    printf(" No character-set translation\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#else

#ifndef LATIN2
    printf(" No Latin-2 character-set translation\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* LATIN2 */

#ifdef NOGREEK
    printf(" No Greek character-set translation\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* NOGREEK */

#ifdef NOHEBREW
    printf(" No Hebrew character-set translation\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* NOHEBREW */

#ifdef NOUNICODE
    printf(" No Unicode character-set translation\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* NOUNICODE */

#ifdef NOCYRIL
    printf(" No Cyrillic character-set translation\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* NOCYRIL */

#ifndef KANJI
    printf(" No Kanji character-set translation\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* KANJI */
#endif /* NOCSETS */

#ifdef NOCMDL
    printf(" No command-line arguments\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* NOCMDL */

#ifdef NOPUSH
    printf(" No escape to system\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* NOPUSH */

#ifdef NOJC
#ifdef UNIX
    printf(" No UNIX job control\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* UNIX */
#endif /* NOJC */

#ifdef NOSETKEY
    printf(" No SET KEY command\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* NOSETKEY */

#ifndef CK_REDIR
    printf(" No REDIRECT or PIPE command\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* CK_REDIR */

#ifdef UNIX
#ifndef NETPTY
    printf(" No pseudoterminal control\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* NETPTY */
#endif /* UNIX */

#ifndef CK_RESEND
    printf(" No RESEND command\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* CK_RESEND */

#ifdef OS2
#ifdef __32BIT__
#ifndef OS2MOUSE
    printf(" No Mouse support\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* __32BIT__ */
#endif /* OS2 */
#endif /* OS2MOUSE */

#ifdef OS2
#ifndef NT
#ifndef CK_REXX
    printf(" No REXX script language interface\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* CK_REXX */
#endif /* NT */
#endif /* OS2 */

#ifndef IKSD
    printf(" No Internet Kermit Service\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    flag = 1;
#endif /* IKSD */

    if (flag == 0) {
        printf(" None\n");
        if (++lines > cmd_rows - 3)
          { if (!askmore()) return(1); else lines = 0; }
    }
    printf("\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }

#ifdef CK_UTSNAME
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    printf("Host info:\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    printf(" Machine:    %s\n",unm_mch[0] ? unm_mch : "(unknown)");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    printf(" Model:      %s\n",unm_mod[0] ? unm_mod : "(unknown)");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    printf(" OS:         %s\n",unm_nam[0] ? unm_nam : "(unknown)");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    printf(" OS Release: %s\n",unm_rel[0] ? unm_rel : "(unknown)");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    printf(" OS Version: %s\n",unm_ver[0] ? unm_ver : "(unknown)");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
    printf("\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* CK_UTSNAME */

/*
  Print compile-time (-D) options, as well as C preprocessor
  predefined symbols that might affect us...
*/
#ifdef KTARGET
    {
        char * s;                       /* Makefile target */
        s = KTARGET;
        if (!s) s = "";
        if (!*s) s = "(unknown)";
        printf("\n");
        if (++lines > cmd_rows - 3) {
            if (!askmore()) return(1); else lines = 0;
        }
        printf("Target: %s\n", s);
        if (++lines > cmd_rows - 3) {
            if (!askmore()) return(1); else lines = 0;
        }
    }
#endif /* KTARGET */

#ifdef __VERSION__
#ifdef __GNUC__
    printf("GCC version: %s\n", __VERSION__);
#else
    printf("Compiler version: %s\n", __VERSION__);
#endif /* __GNUC__ */
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }
#endif /* __VERSION__ */

#ifdef __DATE__                         /* GNU and other ANSI */
#ifdef __TIME__
    printf("Compiled %s %s, options:\n", __DATE__, __TIME__);
#else
    printf("Compiled %s, options:\n", __DATE__);
#endif /* __TIME__ */
#else /* !__DATE__ */
    printf("Compiler options:\n");
#endif /* __DATE__ */
    if (++lines > cmd_rows - 3) { if (!askmore()) return(1); else lines = 0; }

    for (i = 0; i < noptlist; i++)      /* Print sorted option list */
      if (!prtopt(&lines,optlist[i]))
        return(0);

    if (!prtopt(&lines,"")) return(0);  /* Start a new section */

/* Sizes of data types */

    ckmakmsg(line,
             LINBUFSIZ,
             "byte order: ",
             byteorder ? "little" : "big",
             " endian",
             NULL
             );
    {
/* Whether to use %d or %ld with sizeof is a portability issue, so... */
	int size = 0;

	if (!prtopt(&lines,line)) return(0);
	if (!prtopt(&lines,"")) return(0);  /* Start a new section */

	size = (int)sizeof(int);
	sprintf(line,"sizeofs: int=%d",size); /* SAFE */
	if (!prtopt(&lines,line)) return(0);

	size = (int)sizeof(long);
	sprintf(line,"long=%d",size); /* SAFE */
	if (!prtopt(&lines,line)) return(0);

#ifndef OS2
	/* Windows doesn't have off_t */
	size = (int)sizeof(off_t);
	sprintf(line,"off_t=%d",size); /* SAFE */
	if (!prtopt(&lines,line)) return(0);
#endif /* OS2 */

	size = (int)sizeof(CK_OFF_T);
	sprintf(line,"CK_OFF_T=%d",size); /* SAFE */
	if (!prtopt(&lines,line)) return(0);

#ifdef BIGBUFOK
	size = (int)sizeof(size_t);
	sprintf(line,"size_t=%d",size); /* SAFE */
	if (!prtopt(&lines,line)) return(0);
#endif /* BIGBUFOK */

	size = (int)sizeof(short);
	sprintf(line,"short=%d",size); /* SAFE */
	if (!prtopt(&lines,line)) return(0);

	size = (int)sizeof(char);
	sprintf(line,"char=%d",size); /* SAFE */
	if (!prtopt(&lines,line)) return(0);

	size = (int)sizeof(char *);
	sprintf(line,"char*=%d",size); /* SAFE */
	if (!prtopt(&lines,line)) return(0);

	size = (int)sizeof(float);
	sprintf(line,"float=%d",size); /* SAFE */
	if (!prtopt(&lines,line)) return(0);

	size = (int)sizeof(double);
	sprintf(line,"double=%d",size); /* SAFE */
	if (!prtopt(&lines,line)) return(0);
    }

#ifdef FNFLOAT
    if (!prtopt(&lines,"")) return(0);  /* Start a new section */
    if (!prtopt(&lines,"floating-point:")) return(0);
    sprintf(line,"precision=%d",fp_digits); /* SAFE */
    if (!prtopt(&lines,line)) return(0);
    sprintf(line,"rounding=%d",fp_rounding); /* SAFE */
    if (!prtopt(&lines,line)) return(0);
#endif /* FNFLOAT */

    prtopt(&lines,"");
    return(0);
}
#endif /* NOSHOW */
#endif /* NOICP */

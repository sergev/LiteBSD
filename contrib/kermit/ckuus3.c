#ifdef SSHTEST
#define SSHBUILTIN
#endif /* SSHTEST */

#include "ckcsym.h"                     /* Symbol definitions */

/*  C K U U S 3 --  "User Interface" for C-Kermit, part 3  */

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

/*  SET command (but much material has been split off into ckuus7.c). */

/*
  Kermit-specific includes.
  Definitions here supersede those from system include files.
*/
#include "ckcdeb.h"                     /* Debugging & compiler things */
#include "ckcasc.h"                     /* ASCII character symbols */
#include "ckcker.h"                     /* Kermit application definitions */
#include "ckcxla.h"                     /* Character set translation */
#include "ckcnet.h"                     /* Network symbols */

char pwbuf[PWBUFL+1]  = { NUL, NUL };
int pwflg = 0;
int pwcrypt = 0;

#ifndef NOICP

#ifdef CK_AUTHENTICATION
#include "ckuath.h"
#endif /* CK_AUTHENTICATION */
#ifdef CK_SSL
#include "ck_ssl.h"
#endif /* CK_SSL */
#include "ckuusr.h"                     /* User interface symbols */
#ifdef OS2
#include "ckcuni.h"
#ifdef SSHBUILTIN
#include "ckossh.h"
#endif /* SSHBUILTIN */
#ifdef CK_NETBIOS
#include <os2.h>
#ifdef COMMENT                          /* Would you believe */
#undef COMMENT                          /* <os2.h> defines this ? */
#endif /* COMMENT */
#include "ckonbi.h"
extern UCHAR NetBiosAdapter;
#endif /* CK_NETBIOS */
#include "ckocon.h"
#include "ckokey.h"
#ifndef NOTERM
extern unsigned char colorcmd;  /* Command-screen colors */
extern struct keytab ttyclrtab[];
extern int nclrs;
extern int tt_cols[], tt_rows[], tt_szchng[], tt_status[];
#endif /* NOTERM */
_PROTOTYP(int setprty, (void));
extern char startupdir[], exedir[];
extern int tt_modechg;
#ifdef NT
#include <windows.h>
#include <tapi.h>
#include "ckntap.h"                     /* Microsoft TAPI */
#endif /* NT */
#endif /* OS2 */

#ifndef OS2
extern char * exedir;
#endif /* OS2 */

#ifdef CK_RECALL
extern int cm_retry;
#endif /* CK_RECALL */

#ifdef NEWFTP
extern int ftpisopen();
#endif /* NEWFTP */

extern int cmdint;
extern int srvidl;

#ifdef CKFLOAT
extern CKFLOAT floatval;		/* (see isfloat()) */
#endif /* CKFLOAT */

#ifndef NOPUSH
#ifndef NOFRILLS
#ifdef VMS
char editor[CKMAXPATH + 1] = "edit";
#else
char editor[CKMAXPATH + 1] = { NUL, NUL };
#endif /* VMS */
char editopts[128] = { NUL, NUL };
char editfile[CKMAXPATH + 1] = { NUL, NUL };
#ifdef BROWSER
char browser[CKMAXPATH + 1] = { NUL, NUL };
char browsopts[128] = { NUL, NUL };
char browsurl[4096] = { NUL, NUL };
#endif /* BROWSER */
#endif /*  NOFRILLS */
#endif /* NOPUSH */

#ifndef NOFRILLS
#ifndef NORENAME
_PROTOTYP(int setrename, (void));
#endif	/* NORENAME */
#endif	/* NOFRILLS */

/* Variables */

int cmd_quoting = 1;
int cmd_err = 1;
extern int hints, xcmdsrc;

#ifdef CK_KERBEROS
char * k4pwprompt = NULL;               /* Kerberos 4 password prompt */
char * k4prprompt = NULL;               /* Kerberos 4 principal prompt */
char * k5pwprompt = NULL;               /* Kerberos 5 password prompt */
char * k5prprompt = NULL;               /* Kerberos 5 principal prompt */
#endif /* CK_KERBEROS */
#ifdef CK_SRP
char * srppwprompt = NULL;
#endif /* CK_SRP */

extern char * ckprompt, * ikprompt;     /* Default prompt */
extern xx_strp xxstring;

extern char * cdmsgfile[], * cdmsgstr;

extern int
  local, server, success, dest, sleepcan, inserver, flow, autoflow, binary,
  parity, escape, what, turn, duplex, backgrd, hwparity, stopbits, turnch,
  mdmtyp, network, quiet, nettype, carrier, debses, debtim, cdtimo, nlangs,
  bgset, pflag, msgflg, cmdmsk, xsuspend, techo, pacing, xitwarn, xitsta,
  outesc, cmd_cols, cmd_rows, ckxech, xaskmore, haveline, didsetlin, isguest,
  mdmsav, clearrq, saveask, debmsg;

extern int reliable, setreliable, matchdot, matchfifo, dir_dots;

#ifndef NOSERVER
  extern int en_pri;
#endif /* NOSERVER */

#ifdef IKSDCONF
extern int iksdcf;
#endif /* IKSDCONF */
#ifdef TCPSOCKET
  extern int tn_exit;
#endif /* TCPSOCKET */
#ifdef TNCODE
  char * tn_pr_uid = NULL;
#endif /* TNCODE */
  extern int exitonclose;

#ifndef NOKVERBS
extern int nkverbs;
extern struct keytab kverbs[];
#endif /* NOKVERBS */

extern int ttnproto;                    /* Network protocol */

extern char *ccntab[];                  /* Names of control chars */

#ifdef CK_APC
extern int apcactive, apcstatus;
#endif /* CK_APC */

#ifndef NOSCRIPT
extern int secho;                       /* Whether SCRIPT cmd should echo */
#endif /* NOSCRIPT */

#ifdef DCMDBUF
extern char *atmbuf, *atxbuf;
#else
extern char atmbuf[], atxbuf[];
#endif /* DCMDBUF */
extern int cmflgs;

extern char psave[];
extern char uidbuf[];
extern int  sl_uid_saved;
int DeleteStartupFile = 0;

extern int cmdlvl;                      /* Overall command level */

#ifndef NOSPL
_PROTOTYP( static int parsdir, (int) );
char prmbuf[PWBUFL+1] = { NUL, NUL };
int fndiags = 1;                        /* Function diagnostics on/off */
int fnerror = 1;                        /* Function error treatment */

#ifdef DCMDBUF
extern int *count, *takerr, *merror, *inpcas;
#else
extern int count[], takerr[], merror[], inpcas[];
#endif /* DCMDBUF */
extern int mecho;                       /* Macro echo */
extern long ck_alarm;
extern char alrm_date[], alrm_time[];
#else
extern int takerr[];
#endif /* NOSPL */

extern int x_ifnum;
extern int bigsbsiz, bigrbsiz;          /* Packet buffers */

extern long speed;                      /* Terminal speed */

extern char ttname[];                   /* Communication device name */
extern char myhost[] ;
extern char inidir[];                   /* Ini File directory */

#ifndef NOSETKEY
extern KEY *keymap;                     /* Character map for SET KEY (1:1)  */
extern MACRO *macrotab;                 /* Macro map for SET KEY (1:string) */
#endif /* NOSETKEY */
#ifdef OS2
int wideresult;                         /* For wide OS/2 scan codes/cmnum() */
#endif /* OS2 */

#ifndef NOLOCAL
#ifdef OS2
extern int tt_scrsize[];                /* Scrollback buffer Sizes */
#endif /* OS2 */
#endif /* NOLOCAL */

/* Printer settings */

extern char * printername;              /* NULL if printer not redirected */
extern int printpipe;
extern int noprinter;
#ifdef PRINTSWI
int printtimo = 0;
char * printterm = NULL;
char * printsep = NULL;
int printertype = 0;
#ifdef BPRINT
int printbidi = 0;                      /* SET BPRINTER (bidirectional) */
long pportspeed = 0L;                   /* Bidirection printer port speed, */
int pportparity = 0;                    /*  parity, */
int pportflow = FLO_KEEP;               /*  and flow control */
#endif /* BPRINT */
#ifdef OS2
extern int txt2ps;                      /* Text2PS conversion? */
extern int ps_width, ps_length;         /* Text2PS dimensions */
#endif /* OS2 */
#endif /* PRINTSWI */

#ifdef OS2
extern int tcp_avail;                   /* Nonzero if TCP/IP is available */
#ifdef DECNET
extern int dnet_avail;                  /* Ditto for DECnet */
#endif /* DECNET */
#ifdef SUPERLAT
extern int slat_avail;
#endif /* SUPERLAT */
#endif /* OS2 */

static struct keytab logintab[] = {
    { "password", LOGI_PSW, CM_INV },
    { "prompt",   LOGI_PRM, CM_INV },
    { "userid",   LOGI_UID, 0 }
};

#ifndef NOCSETS
/* system-independent character sets, defined in ckcxla.[ch] */
extern struct csinfo tcsinfo[];
extern struct langinfo langs[];

/* Other character-set related variables */
extern int tcharset, tslevel, language;
#endif /* NOCSETS */

/* File-transfer variable declarations */

#ifndef NOXFER
#ifdef CK_AUTODL
extern int cmdadl;
#endif /* CK_AUTODL */

#ifndef NOSERVER
extern int ngetpath;
extern char * getpath[];
#endif /* NOSERVER */

extern struct ck_p ptab[];

extern CHAR sstate;                     /* Protocol start state */
extern CHAR myctlq;                     /* Control-character prefix */
extern CHAR myrptq;                     /* Repeat-count prefix */

extern int protocol, size, spsiz, spmax, urpsiz, srvtim, srvcdmsg, slostart,
  srvdis, xfermode, ckdelay, keep, maxtry, unkcs, bctr, bctf, ebqflg, swcapr,
  wslotr, lscapr, lscapu, spsizr, rptena, rptmin, docrc, xfrcan, xfrchr,
  xfrnum, xfrbel, xfrint, srvping, g_xfermode, xfrxla;

#ifdef PIPESEND
extern int usepipes;
#endif /* PIPESEND */

#ifdef CKXXCHAR                         /* DOUBLE / IGNORE char table */
extern int dblflag, ignflag, dblchar;
extern short dblt[];
#endif /* CKXXCHAR */

#ifdef CK_SPEED
extern short ctlp[];                    /* Control-prefix table */
extern int prefixing;
static struct keytab pfxtab[] = {
    "all",         PX_ALL, 0,
    "cautious",    PX_CAU, 0,
    "minimal",     PX_WIL, 0,
    "none",        PX_NON, 0
};
#endif /* CK_SPEED */
#endif /* NOXFER */

/* Declarations from cmd package */

#ifdef DCMDBUF
extern char *cmdbuf;                    /* Command buffer */
extern char *line;
extern char *tmpbuf;
#else
extern char cmdbuf[];                   /* Command buffer */
extern char line[];                     /* Character buffer for anything */
extern char tmpbuf[];
#endif /* DCMDBUF */

/* From main ckuser module... */

extern char *tp, *lp;                   /* Temporary buffer */

extern int tlevel;                      /* Take Command file level */

#ifndef NOLOCAL
extern int sessft;                      /* Session-log file type */
extern int slogts;                      /* Session-log timestamps on/off */
extern int slognul;			/* Lines null-terminated */
#endif /* NOLOCAL */

char * tempdir = NULL;

#ifdef VMS
int vms_msgs = 1;                       /* SET MESSAGES */
extern int batch;
#endif /* VMS */

/* Keyword tables for SET commands */

#ifdef CK_SPEED
struct keytab ctltab[] = {
    "prefixed",   1, 0,                 /* Note, the values are important. */
    "unprefixed", 0, 0
};
#endif /* CK_SPEED */

static struct keytab oldnew[] = {
    "new", 0, 0,
    "old", 1, 0
};

#define MCH_FIFO 1
#define MCH_DOTF 2
struct keytab matchtab[] = {
    { "dotfile", MCH_DOTF, 0 },
    { "fifo",    MCH_FIFO, 0 }
};
int nmatchtab = (sizeof(matchtab) / sizeof(struct keytab));

#ifndef NOSPL
static struct keytab functab[] = {
    "diagnostics", FUNC_DI, 0,
    "error",       FUNC_ER, 0
};
static int nfunctab = (sizeof(functab) / sizeof(struct keytab));

struct keytab outptab[] = {             /* SET OUTPUT parameters */
    "pacing", 0, 0,                     /* only one so far... */
    "special-escapes", 1, 0
};
int noutptab = (sizeof(outptab) / sizeof(struct keytab)); /* How many */
#endif /* NOSPL */

struct keytab chktab[] = {              /* Block check types */
    "1", 1, 0,                          /* 1 =  6-bit checksum */
    "2", 2, 0,                          /* 2 = 12-bit checksum */
    "3", 3, 0,                          /* 3 = 16-bit CRC */
    "4", 4, 0,				/* Same as B */
    "5", 5, 0,				/* Same as F */
    "blank-free-2", 4, CM_INV,		/* B = 12-bit checksum, no blanks */
    "force-3", 5, CM_INV		/* F = Force CRC on ALL packets */
};
static int nchkt = (sizeof(chktab) / sizeof(struct keytab));

struct keytab rpttab[] = {              /* SET REPEAT */
    "counts",    0, 0,                  /* On or Off */
#ifdef COMMENT
    "minimum",   1, 0,                  /* Threshhold */
#endif /* COMMENT */
    "prefix",    2, 0                   /* Repeat-prefix character value */
};

#ifndef NOLOCAL
/* For SET [ MODEM ] CARRIER, and also for SET DIAL CONNECT */

struct keytab crrtab[] = {
    "automatic", CAR_AUT, 0,            /* 2 */
    "off",       CAR_OFF, 0,            /* 0 */
    "on",        CAR_ON,  0             /* 1 */
};
int ncrr = 3;
#endif /* NOLOCAL */

struct keytab ooatab[] = {              /* On/Off/Auto table */
    "automatic", SET_AUTO, 0,           /* 2 */
    "off",       SET_OFF,  0,           /* 0 */
    "on",        SET_ON,   0            /* 1 */
};

struct keytab ooetab[] = {              /* On/Off/Stderr table 2010/03/12 */
    "off",       SET_OFF, 0,		/* for SET DEBUG MESSAGES */
    "on",        SET_ON,  0,
    "s",         2,       CM_ABR|CM_INV,
    "st",        2,       CM_ABR|CM_INV,
    "std",       2,       CM_ABR|CM_INV,
    "stderr",    2,       0,
    "stdout",    SET_ON,  CM_INV
};
static int nooetab = (sizeof(ooetab) / sizeof(struct keytab));

struct keytab ooktab[] = {              /* On/Off/Ask table */
    "ask",       2,        0,           /* 2 */
    "off",       SET_OFF,  0,           /* 0 */
    "on",        SET_ON,   0            /* 1 */
};

struct keytab qvtab[] = {               /* Quiet/Verbose table */
    "quiet", 1, 0,
    "verbose", 0, 0
};
int nqvt = 2;

/* For SET DEBUG */

#define DEB_OFF  0
#define DEB_ON   1
#define DEB_SES  2
#define DEB_TIM  3
#define DEB_LEN  4
#define DEB_MSG  5

struct keytab dbgtab[] = {
    "linelength", DEB_LEN, CM_INV,
    "m",          DEB_MSG, CM_ABR|CM_INV,
    "message",    DEB_MSG, 0,
    "msg",        DEB_MSG, CM_INV,
    "off",        DEB_OFF, 0,
    "on",         DEB_ON,  0,
    "session",    DEB_SES, 0,
    "timestamps", DEB_TIM, 0
};
int ndbg = (sizeof(dbgtab) / sizeof(struct keytab));

#ifndef NOLOCAL
/* Transmission speeds */

#ifdef TTSPDLIST /* Speed table constructed at runtime . . . */

struct keytab * spdtab = NULL;
int nspd = 0;

#else
/*
  Note, the values are encoded in cps rather than bps because 19200 and higher
  are too big for some ints.  All but 75bps are multiples of ten.  Result of
  lookup in this table must be multiplied by 10 to get actual speed in bps.
  If this number is 70, it must be changed to 75.  If it is 888, this means
  75/1200 split speed.

  The values are generic, rather than specific to UNIX.  We can't use B75,
  B1200, B9600, etc, because non-UNIX versions of C-Kermit will not
  necessarily have these symbols defined.  The BPS_xxx symbols are
  Kermit-specific, and are defined in ckcdeb.h or on the CC command line.

  Like all other keytabs, this one must be in "alphabetical" order,
  rather than numeric order.
*/
struct keytab spdtab[] = {
    "0",      0,  CM_INV,
    "110",   11,  0,
#ifdef BPS_115K
 "115200",11520,  0,
#endif /* BPS_115K */
  "1200",   120,  0,
#ifdef BPS_134
  "134.5",  134,  0,
#endif /* BPS_134 */
#ifdef BPS_14K
  "14400", 1440,  0,
#endif /* BPS_14K */
#ifdef BPS_150
  "150",     15,  0,
#endif /* BPS_150 */
#ifdef BPS_1800
  "1800",     180,  0,
#endif /* BPS_150 */
#ifdef BPS_19K
  "19200", 1920,  0,
#endif /* BPS_19K */
#ifdef BPS_200
  "200",     20,  0,
#endif /* BPS_200 */
#ifdef BPS_230K
  "230400", 23040, 0,
#endif /* BPS_230K */
  "2400",   240,  0,
#ifdef BPS_28K
  "28800", 2880,  0,
#endif /* BPS_28K */
  "300",     30,  0,
#ifdef BPS_3600
  "3600",   360,  0,
#endif /* BPS_3600 */
#ifdef BPS_38K
  "38400", 3840,  0,
#endif /* BPS_38K */
#ifdef BPS_460K
  "460800", 46080,  0,                  /* Need 32 bits for this... */
#endif /* BPS_460K */
  "4800",   480,  0,
#ifdef BPS_50
  "50",       5,  0,
#endif /* BPS_50 */
#ifdef BPS_57K
  "57600", 5760,  0,
#endif /* BPS_57K */
  "600",     60,  0,
#ifdef BPS_7200
  "7200",   720,  0,
#endif /* BPS_7200 */
#ifdef BPS_75
  "75",       7,  0,
#endif /* BPS_75 */
#ifdef BPS_7512
  "75/1200",888,  0,                    /* Code "888" for split speed */
#endif /* BPS_7512 */
#ifdef BPS_76K
  "76800", 7680,  0,
#endif /* BPS_76K */
#ifdef BPS_921K
  "921600", 92160,0,                    /* Need 32 bits for this... */
#endif /* BPS_921K */
  "9600",   960,  0
};
int nspd = (sizeof(spdtab) / sizeof(struct keytab)); /* How many speeds */
#endif /* TTSPDLIST */

#ifdef TN_COMPORT
struct keytab tnspdtab[] = {            /* RFC 2217 TELNET COMPORT Option */
    "115200", 11520,  0,                /* (add any other defined speeds) */
    "1200",     120,  0,
    "14400",   1440,  0,
    "19200",   1920,  0,
    "230400", 23040,  0,
    "2400",     240,  0,
    "28800",   2880,  0,
    "300",       30,  0,
    "38400",   3840,  0,
    "460800", 46080,  0,
    "4800",     480,  0,
    "57600",   5760,  0,
    "600",       60,  0,
    "9600",     960,  0
};
int ntnspd = (sizeof(tnspdtab) / sizeof(struct keytab)); /* How many speeds */
#endif /* TN_COMPORT */
#endif /* NOLOCAL */

#ifndef NOCSETS
extern struct keytab lngtab[];          /* Languages for SET LANGUAGE */
extern int nlng;
#endif /* NOCSETS */

#ifndef NOLOCAL
/* Duplex keyword table */

struct keytab dpxtab[] = {
    "full",      0, 0,
    "half",      1, 0
};
#endif /* NOLOCAL */

/* Flow Control */

struct keytab cxtypesw[] = {
#ifdef DECNET
    "/decnet",         CXT_DECNET,  0,
#endif /* DECNET */
    "/direct-serial",  CXT_DIRECT,  0,
#ifdef DECNET
    "/lat",            CXT_LAT,     0,
#else
#ifdef SUPERLAT
    "/lat",            CXT_LAT,     0,
#endif /* SUPERLAT */
#endif /* DECNET */
    "/modem",          CXT_MODEM,   0,
#ifdef NPIPE
    "/named-pipe",     CXT_NPIPE,   0,
#endif /* NPIPE */
#ifdef NETBIOS
    "/netbios",        CXT_NETBIOS, 0,
#endif /* NETBIOS */
    "/remote",         CXT_REMOTE,  0,
#ifdef TCPSOCKET
    "/tcpip",          CXT_TCPIP,   0,
#endif /* TCPSOCKET */
#ifdef ANYX25
    "/x.25",           CXT_X25,     0,
#endif /* ANYX25 */
    "", 0, 0
};
int ncxtypesw = (sizeof(cxtypesw) / sizeof(struct keytab));

#ifdef TN_COMPORT
struct keytab tnflotab[] = {            /* SET FLOW-CONTROL keyword table */
    "dtr/cd",    FLO_DTRC, 0,           /* for RFC 2217 Telnet COMPORT */
    "dtr/cts",   FLO_DTRT, 0,
    "keep",      FLO_KEEP, 0,
    "none",      FLO_NONE, 0,
    "rts/cts",   FLO_RTSC, 0,
    "xon/xoff",  FLO_XONX, 0
};
int ntnflo = (sizeof(tnflotab) / sizeof(struct keytab));
#endif /* TN_COMPORT */

struct keytab flotab[] = {              /* SET FLOW-CONTROL keyword table */
    "automatic", FLO_AUTO, CM_INV,      /* Not needed any more */
#ifdef CK_DTRCD
    "dtr/cd",    FLO_DTRC, 0,
#endif /* CK_DTRCD */
#ifdef CK_DTRCTS
    "dtr/cts",   FLO_DTRT, 0,
#endif /* CK_DTRCTS */
    "keep",      FLO_KEEP, 0,
    "none",      FLO_NONE, 0,
#ifdef CK_RTSCTS
    "rts/cts",   FLO_RTSC, 0,
#endif /* CK_RTSCTS */
#ifndef Plan9
    "xon/xoff",  FLO_XONX, 0,
#endif /* Plan9 */
    "", 0, 0
};
int nflo = (sizeof(flotab) / sizeof(struct keytab)) - 1;

/*  Handshake characters  */

struct keytab hshtab[] = {
    "bell", 007, 0,
    "code", 998, 0,
    "cr",   015, 0,
    "esc",  033, 0,
    "lf",   012, 0,
    "none", 999, 0,                     /* (can't use negative numbers) */
    "xoff", 023, 0,
    "xon",  021, 0
};
int nhsh = (sizeof(hshtab) / sizeof(struct keytab));

#ifndef NOLOCAL
static struct keytab sfttab[] = {       /* File types for SET SESSION-LOG */
    "ascii",     XYFT_T, CM_INV,
    "binary",    XYFT_B, 0,
    "debug",     XYFT_D, 0,
    "null-padded-lines", 998, 0,
    "text",      XYFT_T, 0,
    "timestamped-text", 999, 0
};
static int nsfttab = (sizeof(sfttab) / sizeof(struct keytab));
#endif /* NOLOCAL */

#ifndef NODIAL

#ifdef NETCONN                          /* Networks directory depends */
int nnetdir = 0;                        /* on DIAL code -- fix later... */
char *netdir[MAXDDIR+2];
#endif /* NETCONN */

_PROTOTYP( static int setdial, (int) );
_PROTOTYP( static int setdcd, (void) );
_PROTOTYP( static int cklogin, (void) );

#ifndef MINIDIAL
#ifdef OLDTBCODE
extern int tbmodel;                     /* Telebit model ID */
#endif /* OLDTBCODE */
#endif /* MINIDIAL */

extern MDMINF *modemp[];                /* Pointers to modem info structs */
extern struct keytab mdmtab[];          /* Modem types (in module ckudia.c) */
extern int nmdm;                        /* Number of them */

_PROTOTYP(static int dialstr,(char **, char *));

extern int dialhng, dialtmo, dialksp, dialdpy, dialmhu, dialec, dialdc;
extern int dialrtr, dialint, dialudt, dialsrt, dialrstr, mdmwaitd;
extern int mdmspd, dialfc, dialmth, dialesc, dialfld, dialidt, dialpace;
extern int mdmspk, mdmvol, dialtest;

int dialcvt = 2;                        /* DIAL CONVERT-DIRECTORY */
int dialcnf = 0;                        /* DIAL CONFIRMATION */
int dialcon = 2;                        /* DIAL CONNECT */
int dialcq  = 0;                        /* DIAL CONNECT AUTO quiet/verbose */
extern long dialmax, dialcapas;
int usermdm = 0;
extern int ndialdir;
extern char *dialini,   *dialmstr, *dialmprmt, *dialdir[], *dialcmd,  *dialnpr,
 *dialdcon, *dialdcoff, *dialecon, *dialecoff, *dialhcmd,  *dialx3,
 *dialhwfc, *dialswfc,  *dialnofc, *dialtone,  *dialpulse, *dialname, *diallac;
extern char *diallcc,   *dialixp,  *dialixs,   *dialldp,   *diallds,  *dialtfp,
 *dialpxi,  *dialpxo,   *dialsfx,  *dialaaon,  *dialaaoff;
extern char *diallcp,   *diallcs,  *dialini2,  *dialmac;
extern char *dialspoff, *dialspon, *dialvol1,  *dialvol2,  *dialvol3;

char *dialtocc[MAXTPCC] = { NULL, NULL };
int ndialtocc = 0;
char *dialpucc[MAXTPCC] = { NULL, NULL };
int ndialpucc = 0;

char *dialtfc[MAXTOLLFREE] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};
int ntollfree = 0;

char *dialpxx[MAXPBXEXCH] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};
int ndialpxx = 0;

char *diallcac[MAXLOCALAC] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};
int nlocalac = 0;

static struct keytab drstrtab[] = {
    "international", 5, 0,
    "local",         2, 0,
    "long-distance", 4, 0,
    "none",          6, 0
};

static struct keytab dcnvtab[] = {
    "ask",  2, 0,
    "off",  0, 0,
    "on",   1, 0
};

struct keytab setmdm[] = {
    "capabilities",     XYDCAP,  0,
    "carrier-watch",    XYDMCD,  0,
    "command",          XYDSTR,  0,
    "compression",      XYDDC,   CM_INV,
    "data-compression", XYDDC,   0,
    "dial-command",     XYDDIA,  0,
    "error-correction", XYDEC,   0,
    "escape-character", XYDESC,  0,
    "flow-control",     XYDFC,   0,
    "hangup-method",    XYDMHU,  0,
#ifndef NOXFER
    "kermit-spoof",     XYDKSP,  0,
#endif /* NOXFER */
    "maximum-speed",    XYDMAX,  0,
    "name",             XYDNAM,  0,
    "speaker",          XYDSPK,  0,
    "speed-matching",   XYDSPD,  0,
    "type",             XYDTYP,  0,
    "volume",           XYDVOL,  0
};
int nsetmdm = (sizeof(setmdm) / sizeof(struct keytab));

struct keytab voltab[] = {
    "high",   3,  0,
    "low",    1,  0,
    "medium", 2,  0
};

struct keytab mdmcap[] = {
    "at-commands",      CKD_AT,  0,
    "compression",      CKD_DC,  0,
    "dc",               CKD_DC,  CM_INV,
    "ec",               CKD_EC,  CM_INV,
    "error-correction", CKD_EC,  0,
    "hardware-flow",    CKD_HW,  0,
    "hwfc",             CKD_HW,  CM_INV,
    "itu",              CKD_V25, CM_INV,
    "kermit-spoof",     CKD_KS,  0,
    "ks",               CKD_KS,  CM_INV,
    "sb",               CKD_SB,  CM_INV,
    "software-flow",    CKD_SW,  0,
    "speed-buffering",  CKD_SB,  0,
    "swfc",             CKD_SW,  CM_INV,
    "tb",               CKD_TB,  CM_INV,
    "telebit",          CKD_TB,  0,
    "v25bis-commands",  CKD_V25, 0
};
int nmdmcap = (sizeof(mdmcap) / sizeof(struct keytab));

#ifdef COMMENT                          /* SET ANSWER not implemented yet */
static struct keytab answertab[] = {
    { "caller-id",  XYA_CID,  0 };
    { "rings",      XYA_RNG,  0 };
    { "", 0, 0 }
};
static int nanswertab =  (sizeof(answertab) / sizeof(struct keytab)) - 1;
#endif /* COMMENT */

struct keytab dialtab[] = {             /* SET DIAL table */
    "area-code",        XYDLAC, 0,      /* Also still includes items     */
    "compression",      XYDDC,  CM_INV, /* that were moved to SET MODEM, */
    "confirmation",     XYDCNF, 0,      /* but they are CM_INVisible...  */
    "connect",          XYDCON, 0,
    "convert-directory",XYDCVT, 0,
    "country-code",     XYDLCC, 0,
    "dial-command",     XYDDIA, CM_INV,
    "directory",        XYDDIR, 0,
    "display",          XYDDPY, 0,
    "escape-character", XYDESC, CM_INV,
    "error-correction", XYDEC,  CM_INV,
    "flow-control",     XYDFC,  CM_INV,
    "force-long-distance", XYDFLD, 0,
    "hangup",           XYDHUP, 0,
    "ignore-dialtone",  XYDIDT, 0,
    "interval",         XYDINT, 0,
    "in",               XYDINI, CM_INV|CM_ABR,
    "init-string",      XYDINI, CM_INV,
    "intl-prefix",      XYDIXP, 0,
    "intl-suffix",      XYDIXS, 0,
#ifndef NOXFER
    "kermit-spoof",     XYDKSP, CM_INV,
#endif /* NOXFER */
    "lc-area-codes",    XYDLLAC, 0,
    "lc-prefix",        XYDLCP, 0,
    "lc-suffix",        XYDLCS, 0,
    "ld-prefix",        XYDLDP, 0,
    "ld-suffix",        XYDLDS, 0,
    "local-area-code",  XYDLAC, CM_INV,
    "local-prefix",     XYDLCP, CM_INV,
    "local-suffix",     XYDLCS, CM_INV,
    "m",                XYDMTH, CM_INV|CM_ABR,
#ifndef NOSPL
    "macro",            XYDMAC, 0,      /* 195 */
#endif /* NOSPL */
#ifdef MDMHUP
    "me",               XYDMTH, CM_INV|CM_ABR,
#endif /* MDMHUP */
    "method",           XYDMTH, 0,
    "mnp-enable",       XYDMNP, CM_INV, /* obsolete but still accepted */
#ifdef MDMHUP
    "modem-hangup",     XYDMHU, CM_INV,
#endif /* MDMHUP */
    "pacing",           XYDPAC,  0,
    "pbx-exchange",     XYDPXX,  0,
    "pbx-inside-prefix",XYDPXI,  0,
    "pbx-outside-prefix",XYDPXO, 0,
    "prefix",           XYDNPR,  0,
    "pulse-countries",  XYDPUCC, 0,
    "restrict",         XYDRSTR, 0,
    "retries",          XYDRTM,  0,
    "sort",             XYDSRT,  0,
    "speed-matching",   XYDSPD,  CM_INV,
    "string",           XYDSTR,  CM_INV,
    "suffix",           XYDSFX,  0,
    "test",             XYDTEST, 0,
    "timeout",          XYDTMO,  0,
    "tf-area-code",     XYDTFC,  CM_INV,
    "tf-prefix",        XYDTFP,  CM_INV,
    "toll-free-area-code",XYDTFC,0,
    "toll-free-prefix", XYDTFP,  0,
    "tone-countries",   XYDTOCC, 0
};
int ndial = (sizeof(dialtab) / sizeof(struct keytab));

#ifdef MDMHUP
struct keytab mdmhang[] = {
    "dtr",           0, 0,
    "modem-command", 1, 0,
    "rs232-signal",  0, 0,
    "v24-signal",    0, CM_INV
};
#endif /* MDMHUP */

static struct keytab mdmcmd[] = {
    "autoanswer",       XYDS_AN, 0,     /* autoanswer */
    "compression",      XYDS_DC, 0,     /* data compression */
    "dial-mode-prompt", XYDS_MP, 0,     /* dial mode prompt */
    "dial-mode-string", XYDS_MS, 0,     /* dial mode string */
    "error-correction", XYDS_EC, 0,     /* error correction */
    "hangup-command",   XYDS_HU, 0,     /* hangup command */
    "hardware-flow",    XYDS_HW, 0,     /* hwfc */
    "ignore-dialtone",  XYDS_ID, 0,     /* ignore dialtone */
    "init-string",      XYDS_IN, 0,     /* init string */
    "no-flow-control",  XYDS_NF, 0,     /* no flow control */
    "predial-init",     XYDS_I2, 0,     /* last-minute setup commands */
    "pulse",            XYDS_DP, 0,     /* pulse */
    "software-flow",    XYDS_SW, 0,     /* swfc */
    "speaker",          XYDS_SP, 0,     /* Speaker */
    "tone",             XYDS_DT, 0,     /* tone */
    "volume",           XYDS_VO, 0      /* Volume */
};
static int nmdmcmd = (sizeof(mdmcmd) / sizeof(struct keytab));

struct keytab dial_fc[] = {
    "auto",     FLO_AUTO, 0,
    "none",     FLO_NONE, 0,
    "rts/cts",  FLO_RTSC, 0,
    "xon/xoff", FLO_XONX, 0
};

struct keytab dial_m[] = {              /* DIAL METHOD */
    "auto",    XYDM_A, 0,
    "default", XYDM_D, 0,
    "pulse",   XYDM_P, 0,
    "tone",    XYDM_T, 0
};
int ndial_m = (sizeof(dial_m)/sizeof(struct keytab));
#endif /* NODIAL */

#ifdef CK_TAPI
struct keytab tapitab[] = {             /* Top-Level Microsoft TAPI */
    "configure-line",     XYTAPI_CFG,  0,
    "dialing-properties", XYTAPI_DIAL, 0
};
int ntapitab = (sizeof(tapitab)/sizeof(struct keytab));

struct keytab settapitab[] = {          /* SET Microsoft TAPI */
    "inactivity-timeout", XYTAPI_INA,  0,
    "line",               XYTAPI_LIN,  0,
    "location",           XYTAPI_LOC,  0,
    "manual-dialing",     XYTAPI_MAN,  0,
    "modem-dialing",      XYTAPI_PASS, 0,
    "modem-lights",       XYTAPI_LGHT, 0,
    "phone-number-conversions",   XYTAPI_CON,  0,
    "port",               XYTAPI_LIN,  CM_INV,
    "post-dial-terminal", XYTAPI_PST,  0,
    "pre-dial-terminal",  XYTAPI_PRE,  0,
    "use-windows-configuration", XYTAPI_USE, 0,
    "wait-for-credit-card-tone", XYTAPI_BNG, 0
};
int nsettapitab = (sizeof(settapitab)/sizeof(struct keytab));

struct keytab * tapiloctab = NULL;      /* Microsoft TAPI Locations */
int ntapiloc = 0;
extern struct keytab * tapilinetab;     /* Microsoft TAPI Line Devices */
extern int ntapiline;
extern int tttapi;                      /* TAPI in use */
extern int tapipass;                    /* TAPI Passthrough mode */
extern int tapiconv;                    /* TAPI Conversion mode */
extern int tapilights;
extern int tapipreterm;
extern int tapipostterm;
extern int tapimanual;
extern int tapiinactivity;
extern int tapibong;
extern int tapiusecfg;
#endif /* CK_TAPI */

#ifndef NOPUSH
extern int nopush;
extern int wildena;
#ifdef UNIX
struct keytab wildtab[] = {             /* SET WILDCARD-EXPANSION */
#ifdef UNIX
    "kermit",  WILD_KER, 0,		/* By Kermit */
#endif	/* UNIX */
    "off",     WILD_OFF, 0,		/* Disabled */
    "on",      WILD_ON,  0,		/* Enabled */
#ifdef UNIX
    "shell",   WILD_SHE, 0,		/* By Shell */
#endif	/* UNIX */
    "", 0, 0
};
int nwild = (sizeof(wildtab) / sizeof(struct keytab)) - 1;

struct keytab wdottab[] = {             /* cont'd */
    "/match-dot-files",    1, 0,
    "/no-match-dot-files", 0, 0
};
extern int wildxpand;
#endif /* UNIX */
#endif /* NOPUSH */

#ifdef NETCONN
extern struct keytab netcmd[], netkey[];
extern int nnets, nnetkey;
#ifdef TCPSOCKET
extern struct keytab tcpopt[];
extern int ntcpopt;
#endif /* TCPSOCKET */
#ifdef NPIPE
char pipename[PIPENAML+1] = { NUL, NUL };
#endif /* NPIPE */
#ifdef CK_NETBIOS
extern unsigned char NetBiosName[];
#endif /* CK_NETBIOS */
#endif /* NETCONN */

#ifdef ANYX25
struct keytab x25tab[] = {
    "call-user-data",    XYUDAT, 0,
    "closed-user-group", XYCLOS, 0,
    "reverse-charge",    XYREVC, 0
};
int nx25 = (sizeof(x25tab) / sizeof(struct keytab));

#ifndef IBMX25
struct keytab padx3tab[] = {
    "break-action",         PAD_BREAK_ACTION,           0,
    "break-character",      PAD_BREAK_CHARACTER,        0,
    "character-delete",     PAD_CHAR_DELETE_CHAR,       0,
    "cr-padding",           PAD_PADDING_AFTER_CR,       0,
    "discard-output",       PAD_SUPPRESSION_OF_DATA,    0,
    "echo",                 PAD_ECHO,                   0,
    "editing",              PAD_EDITING,                0,
    "escape",               PAD_ESCAPE,                 0,
    "forward",              PAD_DATA_FORWARD_CHAR,      0,
    "lf-padding",           PAD_PADDING_AFTER_LF,       0,
    "lf-insert",            PAD_LF_AFTER_CR,            0,
    "line-delete",          PAD_BUFFER_DELETE_CHAR,     0,
    "line-display",         PAD_BUFFER_DISPLAY_CHAR,    0,
    "line-fold",            PAD_LINE_FOLDING,           0,
    "pad-flow-control",     PAD_FLOW_CONTROL_BY_PAD,    0,
    "service-signals",      PAD_SUPPRESSION_OF_SIGNALS, 0,
    "timeout",              PAD_DATA_FORWARD_TIMEOUT,   0,
/* Speed is read-only */
    "transmission-rate",    PAD_LINE_SPEED,             0,
    "user-flow-control",    PAD_FLOW_CONTROL_BY_USER,   0
};
int npadx3 = (sizeof(padx3tab) / sizeof(struct keytab));
#endif /* IBMX25 */
#endif /* ANYX25 */

#ifdef TLOG
static struct keytab vbtab[] = {
    "brief",   0, 0,
#ifdef OS2ORUNIX
    "ftp",     2, 0,
#else
#ifdef VMS
    "ftp",     2, 0,
#endif /* def VMS */
#endif /* OS2ORUNIX */
    "verbose", 1, 0
};
int nvb = (sizeof(vbtab) / sizeof(struct keytab));
#endif /* TLOG */

#ifdef CKSYSLOG
static struct keytab syslogtab[] = {
    "all",         SYSLG_CX, 0,
    "commands",    SYSLG_CM, 0,
    "connection",  SYSLG_AC, 0,
    "debug",       SYSLG_DB, 0,
    "dial",        SYSLG_DI, 0,
    "file-access", SYSLG_FA, 0,
    "file-create", SYSLG_FC, 0,
    "login",       SYSLG_LI, 0,
    "none",        SYSLG_NO, 0,
    "protocol",    SYSLG_PR, 0
};
int nsyslog = (sizeof(syslogtab) / sizeof(struct keytab));
#endif /* CKSYSLOG */

/* Parity keyword table */

struct keytab partbl[] = {
    "even",    'e', 0,
#ifdef HWPARITY
    "hardware",'H', 0,
#endif /* HWPARITY */
    "mark",    'm', 0,
    "none",     0 , 0,
    "odd",     'o', 0,
    "space",   's', 0
};
int npar = (sizeof(partbl) / sizeof(struct keytab));

#ifdef HWPARITY
struct keytab hwpartbl[] = {
/* Add mark and space if needed and possible */
    "even",    'e', 0,
#ifdef OS2
    "mark",    'm', 0,
#endif /* OS2 */
    "odd",     'o', 0,
#ifdef OS2
    "space",   's', 0,
#endif /* OS2 */
    "", 0, 0
};
int nhwpar = (sizeof(hwpartbl) / sizeof(struct keytab)) - 1;
#endif /* HWPARITY */

/* On/Off table */

struct keytab onoff[] = {
    "off",       0, 0,
    "on",        1, 0
};

#define XYCD_M    0			/* CD MESSAGE */
#define XYCD_P    1			/* CD PATH */
#define XYCD_H    2			/* CD HOME */

struct keytab cdtab[] = {
    "home",      XYCD_H, 0,
    "message",   XYCD_M, 0,
    "path",      XYCD_P, 0
};
int ncdtab = (sizeof(cdtab) / sizeof(struct keytab));

struct keytab cdmsg[] = {
    "file",      2, 0,
    "off",       0, 0,
    "on",        1, 0
};
int ncdmsg = (sizeof(cdmsg) / sizeof(struct keytab));

static
struct keytab xittab[] = {              /* SET EXIT */
    "hangup",        3, 0,              /* ...HANGUP */
    "on-disconnect", 2, 0,              /* ...ON-DISCONNECT */
    "status",        0, 0,              /* ...STATUS */
    "warning",       1, 0               /* ...WARNING */
};
int nexit = (sizeof(xittab) / sizeof(struct keytab));

struct keytab xitwtab[] = {             /* SET EXIT WARNING */
    "always", 2, 0,                     /* even when not connected */
    "off",    0, 0,                     /* no warning     */
    "on",     1, 0                      /* when connected */
};
int nexitw = (sizeof(xitwtab) / sizeof(struct keytab));

struct keytab rltab[] = {
    "local",     1, 0,                  /* ECHO values */
    "off",       0, CM_INV,
    "on",        1, CM_INV,
    "remote",    0, 0
};
int nrlt = (sizeof(rltab) / sizeof(struct keytab));

/* Incomplete File Disposition table */

struct keytab ifdtab[] = {
    "discard", SET_OFF, 0,
    "keep",    SET_ON,  0
};

struct keytab ifdatab[] = {
    "auto",    SET_AUTO, 0,
    "discard", SET_OFF,  0,
    "keep",    SET_ON,   0
};

char * ifdnam[] = { "discard", "keep", "auto" };

/* SET TAKE parameters table */
static
struct keytab taktab[] = {
    "echo",  0, 0,
    "error", 1, 0,
    "off",   2, CM_INV,                 /* For compatibility */
    "on",    3, CM_INV                  /* with MS-DOS Kermit... */
};

#ifndef NOSPL
#ifdef COMMENT
/* not used */
static
struct keytab suftab[] = {              /* (what to do with) STARTUP-FILE */
    "delete", 1, 0,
    "keep",   0, 0
};
#endif /* COMMENT */

/* SET MACRO parameters table */
static
struct keytab smactab[] = {
    "echo",  0, 0,
    "error", 1, 0
};
#endif /* NOSPL */

#ifndef NOSCRIPT
static
struct keytab scrtab[] = {
    "echo",  0, 0
};
#endif /* NOSCRIPT */

/* SET COMMAND table */

/* SET COMMAND items... */

#define SCMD_BSZ 0	/* BYTESIZE */
#define SCMD_RCL 1	/* RECALL */
#define SCMD_RTR 2	/* RETRY */
#define SCMD_QUO 3	/* QUOTING */
#define SCMD_COL 4	/* COLOR */
#define SCMD_HIG 5	/* HEIGHT */
#define SCMD_WID 6	/* WIDTH */
#define SCMD_CUR 7	/* CURSOR-POSITION */
#define SCMD_SCR 8	/* SCROLLBACK */
#define SCMD_MOR 9	/* MORE-PROMPTING */
#define SCMD_INT 10     /* INTERRUPTION */
#define SCMD_ADL 11     /* AUTODOWNLOAD */
#define SCMD_STA 12     /* STATUSLINE */
#define SCMD_DBQ 13	/* DOUBLEQUOTING */
#define SCMD_CBR 14	/* CBREAK */
#define SCMD_BFL 15	/* BUFFER-SIZE (not used) */
#define SCMD_ERR 16	/* ERROR */
#define SCMD_VAR 17	/* VARIABLE-EVALUATION */

static struct keytab scmdtab[] = {
#ifdef CK_AUTODL
    "autodownload",       SCMD_ADL, 0,
#endif /* CK_AUTODL */
#ifdef COMMENT
/*
  To implement this requires that we change CMDBL and ATMBL
  from compile-time symbols to runtime variables.  Not a big deal,
  but not trivial either.
 */
    "buffer-size",        SCMD_BFL, 0,
#endif /* COMMENT */
    "bytesize",           SCMD_BSZ, 0,
    "cbreak",             SCMD_CBR, CM_INV,
#ifdef OS2
    "color",              SCMD_COL, 0,
    "cursor-position",    SCMD_CUR, 0,
#endif /* OS2 */
#ifdef DOUBLEQUOTING
    "doublequoting",      SCMD_DBQ, 0,
#endif /* DOUBLEQUOTING */
    "error-display",      SCMD_ERR, 0,
    "height",             SCMD_HIG, 0,
    "interruption",       SCMD_INT, 0,
    "more-prompting",     SCMD_MOR, 0,
    "quoting",            SCMD_QUO, 0,
#ifdef CK_RECALL
    "recall-buffer-size", SCMD_RCL, 0,
#endif /* CK_RECALL */
#ifdef CK_RECALL
    "retry",              SCMD_RTR, 0,
#endif /* CK_RECALL */
#ifdef OS2
#ifdef ONETERMUPD
    "scrollback",         SCMD_SCR, 0,
    "statusline",         SCMD_STA, 0,
#endif /* ONETERMUPD */
#endif /* OS2 */
    "variable-evaluation", SCMD_VAR,0,
    "width",              SCMD_WID, 0
};
static int nbytt = (sizeof(scmdtab) / sizeof(struct keytab));

#ifndef NOSERVER
/* Server parameters table */
static struct keytab srvtab[] = {
    "cd-message",   XYSERC, 0,
    "display",      XYSERD, 0,
    "get-path",     XYSERP, 0,
    "idle-timeout", XYSERI, 0,
    "keepalive",    XYSERK, 0,
    "login",        XYSERL, 0,
    "timeout",      XYSERT, 0
};
static int nsrvt = (sizeof(srvtab) / sizeof(struct keytab));
#endif /* NOSERVER */

static struct keytab sleeptab[] = {     /* SET SLEEP table */
    "cancellation",  0,   0
};

static struct keytab tstab[] = {        /* SET TRANSFER/XFER table */
    "bell",            XYX_BEL, 0,
#ifdef XFRCAN
    "cancellation",    XYX_CAN, 0,
#endif /* XFRCAN */
#ifndef NOCSETS
    "character-set",   XYX_CSE, 0,
#endif /* NOCSETS */
#ifndef NOSPL
    "crc-calculation", XYX_CRC, 0,
#endif /* NOSPL */
    "display",         XYX_DIS, 0,
    "interruption",    XYX_INT, 0,
    "locking-shift",   XYX_LSH, 0,
    "message",         XYX_MSG, 0,
    "mode",            XYX_MOD, 0,
    "msg",             XYX_MSG, CM_INV,
#ifdef PIPESEND
    "pipes",           XYX_PIP, 0,
#endif /* PIPESEND */
#ifdef CK_XYZ
    "protocol",        XYX_PRO, 0,
#endif /* CK_XYZ */
    "report",          XYX_RPT, 0,
    "slow-start",      XYX_SLO, 0,
#ifndef NOCSETS
    "translation",     XYX_XLA, 0,
#else
    "translation",     XYX_XLA, CM_INV,
#endif /* NOCSETS */
    "xlation",         XYX_XLA, CM_INV,
    "", 0, 0
};
static int nts = (sizeof(tstab) / sizeof(struct keytab)) - 1;

static struct keytab rtstab[] = {       /* REMOTE SET TRANSFER/XFER table */
#ifndef NOCSETS
    "character-set",   XYX_CSE, 0,
#endif /* NOCSETS */
    "mode",            XYX_MOD, 0
};
static int nrts = (sizeof(rtstab) / sizeof(struct keytab));

struct keytab xfrmtab[] = {             /* TRANSFER MODE table */
    "automatic", XMODE_A, 0,
    "manual",    XMODE_M, 0
};

#ifdef LOCUS
extern int locus, autolocus;

static struct keytab locustab[] = {
#ifdef KUI
    { "ask",     3, 0 },		/* Presently implemented in GUI only */
#endif /* KUI */
    { "auto",    2, 0 },
    { "local",   1, 0 },
    { "remote",  0, 0 }
};
static int nlocustab = (sizeof(locustab) / sizeof(struct keytab));

#endif /* LOCUS */

#ifndef NOCSETS
/* SET TRANSFER CHARACTER-SET table */

extern struct keytab tcstab[];
extern int ntcs;
#endif /* NOCSETS */

/* SET TRANSFER LOCKING-SHIFT table */
struct keytab lstab[] = {
    "forced", 2,   0,
    "off",    0,   0,
    "on",     1,   0
};
int nls = (sizeof(lstab) / sizeof(struct keytab));

/* SET TELNET tables */
#ifdef TNCODE
extern int tn_nlm, tn_b_nlm, tn_b_meu, tn_b_ume, tn_b_xfer, tn_sb_bug;
extern int tn_no_encrypt_xfer, tn_auth_krb5_des_bug;
extern int tn_wait_flg, tn_duplex, tn_delay_sb, tn_sfu;
extern int sl_tn_saved;
extern int tn_infinite;
extern int tn_rem_echo;
extern int tn_deb;
extern int tn_auth_how;
extern int tn_auth_enc;
#ifdef CK_FORWARD_X
extern char * tn_fwdx_xauthority;
#endif /* CK_FORWARD_X */
#ifdef CK_AUTHENTICATION
static struct keytab setauth[] = {
#ifdef CK_KERBEROS
    "k4",        AUTH_KRB4, CM_INV,
    "k5",        AUTH_KRB5, CM_INV,
    "kerberos4", AUTH_KRB4, 0,
    "kerberos5", AUTH_KRB5, 0,
    "kerberos_iv",AUTH_KRB4, CM_INV,
    "kerberos_v", AUTH_KRB5, CM_INV,
    "krb4",      AUTH_KRB4, CM_INV,
    "krb5",      AUTH_KRB5, CM_INV,
#endif /* CK_KERBEROS */
#ifdef CK_SRP
    "srp",       AUTH_SRP,  0,
#endif /* CK_SRP */
#ifdef CK_SSL
    "ssl",      AUTH_SSL,   0,
    "tls",      AUTH_TLS,   0,
#endif /* CK_SSL */
    "",         0,      0
};
static int nsetauth = sizeof(setauth)/sizeof(struct keytab) - 1;
#ifdef CK_KERBEROS
extern char * krb5_d_principal;         /* Default principal */
extern char * krb5_d_instance;
extern char * krb5_d_realm;             /* Default realm */
extern char * krb5_d_cc;                /* Default credentials cache */
extern char * krb5_d_srv;               /* Default service name */
extern int    krb5_d_lifetime;          /* Default lifetime */
extern int    krb5_d_forwardable;
extern int    krb5_d_proxiable;
extern int    krb5_d_renewable;
extern int    krb5_autoget;
extern int    krb5_autodel;
extern int    krb5_d_getk4;
extern int    krb5_checkaddrs;          /* Check TGT Addrs */
extern int    krb5_d_no_addresses;
extern char * krb5_d_addrs[];
extern char * k5_keytab;                /* Keytab file */

extern struct krb4_init_data krb4_init;
extern char * krb4_d_principal;         /* Default principal */
extern char * krb4_d_realm;             /* Default realm */
extern char * krb4_d_srv;               /* Default service name */
extern int    krb4_d_lifetime;          /* Default lifetime */
extern int    krb4_d_preauth;
extern char * krb4_d_instance;
extern int    krb4_autoget;
extern int    krb4_autodel;
extern int    krb4_checkaddrs;          /* Check TGT Addrs */
extern char * k4_keytab;                /* Keytab file */
#ifdef KRB4
extern int    k4debug;
#endif /* KRB4 */
static struct keytab krbver[] = {
    "4",                 4, 0,
    "5",                 5, 0,
    "iv",                4, CM_INV,
    "v",                 5, CM_INV
};
static int nkrbver = sizeof(krbver)/sizeof(struct keytab);

static struct keytab kdestab[] = {
    "never",            KRB_DEL_NO, 0,
    "no",               KRB_DEL_NO, CM_INV,
    "on-close",         KRB_DEL_CL, 0,
    "on-exit",          KRB_DEL_EX, 0
};
static int nkdestab = sizeof(kdestab)/sizeof(struct keytab);

static struct keytab k4tab[] = {
    "autodel",           XYKRBDEL, CM_INV,
    "autodestroy",       XYKRBDEL, 0,
    "autoget",           XYKRBGET, 0,
    "check-address",     XYKRBADR, 0,
    "debug",             XYKRBDBG, CM_INV,
    "instance",          XYKRBINS, 0,
    "keytab",            XYKRBKTB, 0,
    "lifetime",          XYKRBLIF, 0,
    "preauth",           XYKRBPRE, 0,
    "principal",         XYKRBPR,  0,
    "prompt",            XYKRBPRM, 0,
    "realm",             XYKRBRL,  0,
    "service-name",      XYKRBSRV, 0
};
static int nk4tab = sizeof(k4tab)/sizeof(struct keytab);

static struct keytab k5tab[] = {
    "addresses",         XYKRBADD, 0,
    "autodelete",        XYKRBDEL, CM_INV,
    "autodestroy",       XYKRBDEL, 0,
    "autoget",           XYKRBGET, 0,
    "cc",                XYKRBCC,  CM_INV,
    "check-address",     XYKRBADR, 0,
    "credentials-cache", XYKRBCC,  0,
    "forwardable",       XYKRBFWD, 0,
    "get-k4-tgt",        XYKRBK5K4,0,
    "instance",          XYKRBINS, 0,
    "keytab",            XYKRBKTB, 0,
    "lifetime",          XYKRBLIF, 0,
    "no-addresses",      XYKRBNAD, 0,
    "principal",         XYKRBPR,  0,
    "prompt",            XYKRBPRM, 0,
    "proxiable",         XYKRBPRX, 0,
    "realm",             XYKRBRL,  0,
    "renewable",         XYKRBRNW, 0,
    "service-name",      XYKRBSRV, 0
};
static int nk5tab = sizeof(k5tab)/sizeof(struct keytab);

#define KRB_PW_PRM 1
#define KRB_PR_PRM 2

static struct keytab krbprmtab[] = {
    "password",  KRB_PW_PRM, 0,
    "principal", KRB_PR_PRM, 0
};

#endif /* CK_KERBEROS */
#ifdef CK_SRP
static struct keytab srptab[] = {
    "prompt",            XYSRPPRM, 0
};
static int nsrptab = sizeof(srptab)/sizeof(struct keytab);
#define SRP_PW_PRM 1

static struct keytab srpprmtab[] = {
    "password",  SRP_PW_PRM, 0
};
#endif /* CK_SRP */
#ifdef CK_SSL
static struct keytab ssltab[] = {
    "certs-ok",          XYSSLCOK,  CM_INV,
    "cipher-list",       XYSSLCL,   0,
    "crl-dir",           XYSSLCRLD, 0,
    "crl-file",          XYSSLCRL,  0,
    "debug",             XYSSLDBG,  0,
    "dh-key-file",       XYSSLDKFL, CM_INV,
    "dh-param-file",     XYSSLDPFL, 0,
    "dsa-cert-chain-file", XYSSLDCCF, 0,
    "dsa-cert-file",     XYSSLDCFL, 0,
    "dsa-key-file",      XYSSLDKFL, 0,
    "dummy",             XYSSLDUM,  CM_INV,
    "only",              XYSSLON,   CM_INV,
    "random-file",       XYSSLRND,  0,
    "rsa-cert-chain-file", XYSSLRCCF, 0,
    "rsa-cert-file",     XYSSLRCFL, 0,
    "rsa-key-file",      XYSSLRKFL, 0,
    "verbose",           XYSSLVRB,  0,
    "verify",            XYSSLVRF,  0,
    "verify-dir",        XYSSLVRFD, 0,
    "verify-file",       XYSSLVRFF, 0
};
static int nssltab = sizeof(ssltab)/sizeof(struct keytab);
static struct keytab sslvertab[] = {
    "fail-if-no-peer-cert", SSL_VERIFY_PEER |
                            SSL_VERIFY_FAIL_IF_NO_PEER_CERT, 0,
    "no",               SSL_VERIFY_NONE, 0,
    "none",             SSL_VERIFY_NONE, CM_INV,
    "off",              SSL_VERIFY_NONE, CM_INV,
    "on",               SSL_VERIFY_PEER, CM_INV,
    "peer-cert",        SSL_VERIFY_PEER, 0
};
static int nsslvertab = sizeof(sslvertab)/sizeof(struct keytab);
#endif /* CK_SSL */
#endif /* CK_AUTHENTICATION */
#ifdef CK_ENCRYPTION
int cx_type = CX_AUTO;
extern int sl_cx_type;
#endif /* CK_ENCRYPTION */
extern char *tcp_address;
#ifndef NOHTTP
extern char * tcp_http_proxy;
extern char * tcp_http_proxy_user;
extern char * tcp_http_proxy_pwd;
extern char * tcp_http_proxy_agent;
#endif /* NOHTTP */
#ifdef NT
#ifdef CK_SOCKS
extern char *tcp_socks_svr;
extern char *tcp_socks_user;
#ifdef CK_SOCKS_NS
extern char *tcp_socks_ns;
#endif /* CK_SOCKS_NS */
#endif /* CK_SOCKS */
#endif /* NT */

#define UPW_USER  1
#define UPW_PASS  2
#define UPW_AGENT 3

static struct keytab userpass[] = {
    { "/agent",   UPW_AGENT, CM_ARG },
    { "/password", UPW_PASS, CM_ARG },
    { "/user",     UPW_USER, CM_ARG },
};
static int nuserpass = sizeof(userpass)/sizeof(struct keytab);

static struct keytab tnnegtab[] = {     /* TELNET NEGOTIATION table */
    "accepted",  TN_NG_AC, 0,
    "refused",   TN_NG_RF, 0,
    "req",       TN_NG_RQ, CM_INV|CM_ABR,
    "requ",      TN_NG_RQ, CM_INV|CM_ABR,
    "reque",     TN_NG_RQ, CM_INV|CM_ABR,
    "reques",    TN_NG_RQ, CM_INV|CM_ABR,
    "request",   TN_NG_RQ, CM_INV|CM_ABR,
    "requeste",  TN_NG_RQ, CM_INV|CM_ABR,
    "requested", TN_NG_RQ, 0,
    "required",  TN_NG_MU, 0
};
static int ntnnegtab = sizeof(tnnegtab)/sizeof(struct keytab);

#ifdef CK_ENCRYPTION
static struct keytab typkwd[] = {
    "/type", 0, CM_ARG
};

static struct keytab tnenctab[] = {     /* TELNET ENCRYPTION table */
    "accepted",   TN_NG_AC,    CM_INV,
    "refused",    TN_NG_RF,    CM_INV,
    "req",        TN_NG_RQ,    CM_INV|CM_ABR,
    "requ",       TN_NG_RQ,    CM_INV|CM_ABR,
    "reque",      TN_NG_RQ,    CM_INV|CM_ABR,
    "reques",     TN_NG_RQ,    CM_INV|CM_ABR,
    "request",    TN_NG_RQ,    CM_INV|CM_ABR,
    "requeste",   TN_NG_RQ,    CM_INV|CM_ABR,
    "requested",  TN_NG_RQ,    CM_INV,
    "required",   TN_NG_MU,    CM_INV,
    "start",      TN_EN_START, CM_INV,
    "stop",       TN_EN_STOP,  CM_INV,
    "type",       TN_EN_TYP,   0
};
static int ntnenc = sizeof(tnenctab)/sizeof(struct keytab) ;
#endif /* CK_ENCRYPTION */

#ifdef CK_FORWARD_X
static struct keytab tnfwdxtab[] = {    /* TELNET FORWARD-X table */
    "no-encryption",    1,  CM_INV,
    "xauthority-file",  0,  0
};
static int ntnfwdx = sizeof(tnfwdxtab)/sizeof(struct keytab) ;
#endif /* CK_FORWARD_X */

static struct keytab tnbugtab[] = {     /* TELNET BUG table */
    "auth-krb5-des",         4, 0,
    "binary-me-means-u-too", 0, 0,
    "binary-u-means-me-too", 1, 0,
    "infinite-loop-check",   2, 0,
    "sb-implies-will-do",    3, 0
};

#ifdef CK_ENVIRONMENT
static struct keytab tnenvtab[] = {     /* TELNET ENVIRONMENT table */
    "acct",     TN_ENV_ACCT,    0,
    "display",  TN_ENV_DISP,    0,
    "job",      TN_ENV_JOB,     0,
    "location", TN_ENV_LOC,     0,
    "off",      TN_ENV_OFF,     CM_INV,
    "on",       TN_ENV_ON,      CM_INV,
    "printer",  TN_ENV_PRNT,    0,
    "systemtype",TN_ENV_SYS,    0,
    "user",     TN_ENV_USR,     0,
    "uservar",  TN_ENV_UVAR,    0,
    "", 0, 0
};
static int ntnenv = sizeof(tnenvtab)/sizeof(struct keytab) - 1;
#endif /* CK_ENVIRONMENT */

#ifdef CK_AUTHENTICATION
static struct keytab tnauthtab[] = {    /* TELNET AUTHENTICATION table */
    "accepted",   TN_NG_AC,  CM_INV,
    "encrypt-flag", TN_AU_ENC, 0,
    "forwarding", TN_AU_FWD,   0,
    "how-flag",   TN_AU_HOW,   0,
    "refused",    TN_NG_RF,  CM_INV,
    "req",        TN_NG_RQ,  CM_INV|CM_ABR,
    "requ",       TN_NG_RQ,  CM_INV|CM_ABR,
    "reque",      TN_NG_RQ,  CM_INV|CM_ABR,
    "reques",     TN_NG_RQ,  CM_INV|CM_ABR,
    "request",    TN_NG_RQ,  CM_INV|CM_ABR,
    "requeste",   TN_NG_RQ,  CM_INV|CM_ABR,
    "requested",  TN_NG_RQ,  CM_INV,
    "required",   TN_NG_MU,  CM_INV,
    "type",       TN_AU_TYP, 0
};
static int ntnauth = sizeof(tnauthtab)/sizeof(struct keytab) ;

struct keytab autyptab[] = {    /* TELNET AUTHENTICATION TYPE table */
    "automatic",  AUTH_AUTO, 0,
#ifdef CK_KERBEROS
    "k4",         AUTH_KRB4, CM_INV,
    "k5",         AUTH_KRB5, CM_INV,
    "kerberos4",  AUTH_KRB4, 0,
    "kerberos5",  AUTH_KRB5, 0,
    "kerberos_iv",AUTH_KRB4, CM_INV,
    "kerberos_v", AUTH_KRB5, CM_INV,
    "krb4",       AUTH_KRB4, CM_INV,
    "krb5",       AUTH_KRB5, CM_INV,
#endif /* CK_KERBEROS */
    "none",       AUTH_NONE, 0,
#ifdef NT
    "ntlm",       AUTH_NTLM, 0,
#endif /* NT */
#ifdef CK_SRP
    "srp",        AUTH_SRP,  0,
#endif /* CK_SRP */
#ifdef CK_SSL
    "ssl",        AUTH_SSL,  0,
#endif /* CK_SSL */
    "", 0, 0
};
int nautyp = sizeof(autyptab)/sizeof(struct keytab) - 1;

struct keytab auhowtab[] = {    /* TELNET AUTHENTICATION HOW table */
    "any",     TN_AUTH_HOW_ANY,     0,
    "mutual",  TN_AUTH_HOW_MUTUAL,  0,
    "one-way", TN_AUTH_HOW_ONE_WAY, 0,
    "", 0, 0
};
int nauhow = sizeof(auhowtab)/sizeof(struct keytab) - 1;

struct keytab auenctab[] = {    /* TELNET AUTHENTICATION ENCRYPT table */
    "any",     TN_AUTH_ENC_ANY,     0,
    "none",    TN_AUTH_ENC_NONE,    0,
    "telopt",  TN_AUTH_ENC_TELOPT,  0,
#ifdef CK_SSL
    "tls",     TN_AUTH_ENC_TLS,     0,
#endif /* CK_SSL */
    "", 0, 0
};
int nauenc = sizeof(auenctab)/sizeof(struct keytab) - 1;
#endif /* CK_AUTHENTICATION */

#define TN_NL_BIN 3
#define TN_NL_NVT 4
static struct keytab tn_nlmtab[] = {    /* TELNET NEWLINE-MODE table */
    "binary-mode", TN_NL_BIN, 0,        /* Binary mode */
    "nvt",    TN_NL_NVT, 0,             /* NVT mode */
    "off",    TNL_CRNUL, CM_INV,        /* CR-NUL (TELNET spec) */
    "on",     TNL_CRLF,  CM_INV,        /* CR-LF (TELNET spec) */
    "raw",    TNL_CR,    CM_INV         /* CR only (out of spec) */
};
static int ntn_nlm = (sizeof(tn_nlmtab) / sizeof(struct keytab));

static struct keytab tnlmtab[] = {      /* TELNET NEWLINE-MODE table */
    "cr",     TNL_CR,    CM_INV,        /* CR only (out of spec) */
    "cr-lf",  TNL_CRLF,  CM_INV,        /* CR-LF (TELNET spec) */
    "cr-nul", TNL_CRNUL, CM_INV,        /* CR-NUL (TELNET spec) */
    "lf",     TNL_LF,    CM_INV,        /* LF instead of CR-LF */
    "off",    TNL_CRNUL, 0,             /* CR-NUL (TELNET spec) */
    "on",     TNL_CRLF,  0,             /* CR-LF (TELNET spec) */
    "raw",    TNL_CR,    0              /* CR only (out of spec) */
};
static int ntnlm = (sizeof(tnlmtab) / sizeof(struct keytab));

struct keytab tntab[] = {
#ifdef CK_AUTHENTICATION
    "authentication",       CK_TN_AU,  0,
#endif /* CK_AUTHENTICATION */
    "b",                    CK_TN_BM,  CM_INV|CM_ABR,
    "bi",                   CK_TN_BM,  CM_INV|CM_ABR,
    "bin",                  CK_TN_BM,  CM_INV|CM_ABR,
    "bina",                 CK_TN_BM,  CM_INV|CM_ABR,
    "binar",                CK_TN_BM,  CM_INV|CM_ABR,
    "binary",               CK_TN_BM,  CM_INV|CM_ABR,
    "binary-",              CK_TN_BM,  CM_INV|CM_ABR,
    "binary-mode",          CK_TN_BM,  CM_INV,
    "binary-transfer-mode", CK_TN_XF,  0,
    "binary-xfer-mode",     CK_TN_XF,  CM_INV,
    "bug",                  CK_TN_BUG, 0,
    "debug",                CK_TN_DB,  0,
    "delay-sb",             CK_TN_DL,  0,
    "echo",                 CK_TN_EC,  0,
#ifdef CK_ENCRYPTION
    "encryption",      CK_TN_ENC,  0,
#endif /* CK_ENCRYPTION */
#ifdef CK_ENVIRONMENT
    "environment",     CK_TN_ENV,  0,
#endif /* CK_ENVIRONMENT */
#ifdef CK_FORWARD_X
    "forward-x",       CK_TN_FX,   0,
#endif /* CK_FORWARD_X */
#ifdef IKS_OPTION
    "kermit",          CK_TN_IKS,  CM_INV,
#endif /* IKS_OPTION */
#ifdef CK_SNDLOC
    "location",        CK_TN_LOC,  0,
#endif /* CK_SNDLOC */
#ifdef CK_NAWS
    "naws",            CK_TN_NAWS, CM_INV,
#endif /* CK_NAWS */
    "newline-mode",    CK_TN_NL,   0,
    "no-encrypt-during-xfer", CK_TN_NE, CM_INV,
    "prompt-for-userid",CK_TN_PUID,0,
    "remote-echo",     CK_TN_RE,   0,
#ifdef CK_SSL
    "start-tls",       CK_TN_TLS,  CM_INV,
#endif /* CK_SSL */
#ifdef NT
    "sfu-compatibility", CK_TN_SFU, 0,
#else
    "sfu-compatibility", CK_TN_SFU, CM_INV,
#endif /* NT */
    "terminal-type",   CK_TN_TT,   0,
    "wait-for-negotiations", CK_TN_WAIT, 0,
#ifdef CK_ENVIRONMENT
    "xdisplay-location",CK_TN_XD, CM_INV,
#endif /* CK_ENVIRONMENT */
    "", 0, 0
};
int ntn = (sizeof(tntab) / sizeof(struct keytab)) - 1;

struct keytab tnopttab[] = {
#ifdef CK_AUTHENTICATION
    "authentication",  CK_TN_AU,   0,
#else
    "authentication",  CK_TN_AU,   CM_INV,
#endif /* CK_AUTHENTICATION */
    "binary-mode",     CK_TN_BM,   0,
#ifdef TN_COMPORT
    "c",               CK_TN_CPC,   CM_INV|CM_ABR,
    "co",              CK_TN_CPC,   CM_INV|CM_ABR,
    "com",             CK_TN_CPC,   CM_INV|CM_ABR,
    "com-port-control",CK_TN_CPC,   0,
    "comport-control", CK_TN_CPC,   CM_INV,
#else /* TN_COMPORT */
    "com-port-control",CK_TN_CPC,  CM_INV,
    "comport-control", CK_TN_CPC,   CM_INV,
#endif /* TN_COMPORT */
    "echo",            CK_TN_EC,   0,
#ifdef CK_ENCRYPTION
    "encryption",      CK_TN_ENC,  0,
#else
    "encryption",      CK_TN_ENC,  CM_INV,
#endif /* CK_ENCRYPTION */
#ifdef CK_FORWARD_X
    "forward-x",       CK_TN_FX,   0,
#else /* CK_FORWARD_X */
    "forward-x",       CK_TN_FX,   CM_INV,
#endif /* CK_FORWARD_X */
    "ibm-sak",         CK_TN_SAK,  CM_INV,
#ifdef IKS_OPTION
    "kermit",          CK_TN_IKS,  0,
#else
    "kermit",          CK_TN_IKS,  CM_INV,
#endif /* IKS_OPTION */
    "lflow",           CK_TN_FLW,  CM_INV,
    "logout",          CK_TN_LOG,  0,
#ifdef CK_NAWS
    "naws",            CK_TN_NAWS, 0,
#else
    "naws",            CK_TN_NAWS, CM_INV,
#endif /* CK_NAWS */
#ifdef CK_ENVIRONMENT
    "new-environment", CK_TN_ENV,  0,
#else
    "new-environment", CK_TN_ENV,  CM_INV,
#endif /* CK_ENVIRONMENT */
    "pragma-heartbeat",CK_TN_PHR,  CM_INV,
    "pragma-logon",    CK_TN_PLG,  CM_INV,
    "pragma-sspi",     CK_TN_PSP,  CM_INV,
    "sak",             CK_TN_SAK,  CM_INV,
#ifdef CK_SNDLOC
    "send-location",   CK_TN_LOC,  0,
#else
    "send-location",   CK_TN_LOC,  CM_INV,
#endif /* CK_SNDLOC */
    "sga",             CK_TN_SGA, CM_INV|CM_ABR,
#ifdef CK_SSL
    "start-tls",       CK_TN_TLS,  0,
#else
    "start-tls",       CK_TN_TLS,  CM_INV,
#endif /* CK_SSL */
    "suppress-go-aheads", CK_TN_SGA, 0,
    "terminal-type",   CK_TN_TT,   0,
    "ttype",           CK_TN_TT,   CM_INV|CM_ABR,
#ifdef CK_ENVIRONMENT
    "xdisplay-location", CK_TN_XD, 0,
#else
    "xdisplay-location", CK_TN_XD, CM_INV,
#endif /* CK_ENVIRONMENT */
    "", 0, 0
};
int ntnopt = (sizeof(tnopttab) / sizeof(struct keytab)) - 1;

struct keytab tnoptsw[] = {
    "/client",  CK_TN_CLIENT,   0,
    "/server",  CK_TN_SERVER,   0
};
int ntnoptsw = (sizeof(tnoptsw) / sizeof(struct keytab));
#endif /* TNCODE */

struct keytab ftrtab[] = {              /* Feature table */
#ifndef NOCSETS                         /* 0 = we have it, 1 = we don't */
"character-sets",       0, 0,
#else
"character-sets",       1, 0,
#endif /* NOCSETS */
#ifndef NOCYRIL
"cyrillic",             0, 0,
#else
"cyrillic",             1, 0,
#endif /* NOCYRIL */

#ifndef NOLOGDIAL
"cx-log",               0, 0,
#else
"cx-log",               1, 0,
#endif /* NOLOGDIAL */

#ifndef NODEBUG
"debug",                0, 0,
#else
"debug",                1, 0,
#endif /* NODEBUG */

#ifndef NODIAL
"dial",                 0, 0,
#else
"dial",                 1, 0,
#endif /* NODIAL */

#ifdef DYNAMIC
"dynamic-memory",       0, 0,
#else
"dynamic-memory",       1, 0,
#endif /* DYNAMIC */

#ifndef NOXFER
"file-transfer",        0, 0,
#else
"file-transfer",        1, 0,
#endif /* NOXFER */

#ifdef XXFWD
"forward",              0, 0,
#else
"forward",              1, 0,
#endif /* XXFWD */

#ifdef NEWFTP
"ftp",                  0, 0,
#else
"ftp",                  1, 0,
#endif /* NEWFTP */

#ifdef CK_CURSES
"fullscreen-display",   0, 0,
#else
"fullscreen-display",   1, 0,
#endif /* CK_CURSES */
#ifdef GREEK
"greek",                0, 0,
#else
"greek",                1, 0,
#endif /* GREEK */
#ifdef HEBREW
"hebrew",               0, 0,
#else
"hebrew",               1, 0,
#endif /* HEBREW */
#ifndef NOHELP
"help",                 0, 0,
#else
"help",                 1, 0,
#endif /* NOHELP */

#ifndef NOIKSD
"iksd",                 0, 0,
#else
"iksd",                 1, 0,
#endif /* NOIKSD */

#ifndef NOSPL
"if-command",           0, 0,
#else
"if-command",           1, 0,
#endif /* NOSPL */
#ifndef NOJC
#ifdef UNIX
"job-control",          0, 0,
#else
"job-control",          1, 0,
#endif /* UNIX */
#else
"job-control",          1, 0,
#endif /* NOJC */
#ifdef KANJI
"kanji",                0, 0,
#else
"kanji",                1, 0,
#endif /* KANJI */

#ifndef NOXFER
"kermit",               0, 0,
#else
"kermit",               1, 0,
#endif /* NOXFER */

#ifdef CK_KERBEROS
"kerberos",             0, 0,
#else
"kerberos",             1, 0,
#endif /* CK_KERBEROS */

#ifndef NOCSETS
"latin1",               0, 0,
#else
"latin1",               1, 0,
#endif /* NOCSETS */
#ifdef LATIN2
"latin2",               0, 0,
#else
"latin2",               1, 0,
#endif /* LATIN2 */

#ifdef CKLEARN
"learned-scripts",       0, 0,
#else
"learned-scripts",       1, 0,
#endif /* CKLEARN */

#ifndef NOLOCAL
"making-connections",   0, 0,
#else
"making-connections",   1, 0,
#endif /* NOLOCAL */

#ifdef NETCONN
"network",              0, 0,
#else
"network",              1, 0,
#endif /* NETCONN */

#ifdef NT
#ifdef CK_AUTHENTICATION
"ntlm",                 1, 0,
#else /* CK_AUTHENTICATION */
"ntlm",                 0, 0,
#endif /* CK_AUTHENTICATION */
#else /* NT */
"ntlm",                 0, 0,
#endif /* NT */

#ifdef PIPESEND
"pipes",                0, 0,
#else
#ifdef NETCMD
"pipes",                0, 0,
#endif /* NETCMD */
#endif /* PIPESEND */
#ifndef PIPESEND
#ifndef NETCMD
"pipes",                1, 0,
#endif /* PIPESEND */
#endif /* NETCMD */

#ifdef NETPTY
"pty",                  0, 0,
#else
"pty",                  1, 0,
#endif /* NETPTY */

#ifndef NOPUSH
"push",                 0, 0,
#else
"push",                 1, 0,
#endif /* PUSH */

#ifdef CK_REDIR
"redirect",             0, 0,
#else
"redirect",             1, 0,
#endif /* CK_REDIR */

#ifdef CK_RTSCTS
"rts/cts",              0, 0,
#else
"rts/cts",              1, 0,
#endif /* RTS/CTS */

#ifndef NOSCRIPT
"script-command",       0, 0,
#else
"script-command",       1, 0,
#endif /* NOSCRIPT */
#ifndef NOSERVER
"server-mode",          0, 0,
#else
"server-mode",          1, 0,
#endif /* NOSERVER */

#ifndef NOSEXP
"sexpression",          0, 0,
#else
"sexpression",          1, 0,
#endif /* NOSEXP */

#ifdef SFTP_BUILTIN
"sftp",                 1, 0,
#else
"sftp",                 0, 0,
#endif /* SFTP_BUILTIN */

#ifndef NOSHOW
"show-command",         0, 0,
#else
"show-command",         1, 0,
#endif /* NOSHOW */

#ifdef CK_SRP
"srp",                  0, 0,
#else
"srp",                  1, 0,
#endif /* CK_SRP */

#ifdef SSHBUILTIN
"ssh",                  0, 0,
#else /* SSHBUILTIN */
"ssh",                  1, 0,
#endif /* SSHBUILTIN */

#ifdef CK_SSL
"ssl/tls",              0, 0,
#else
"ssl/tls",              1, 0,
#endif /* CK_SSL */

#ifndef NOXMIT
"transmit",             0, 0,
#else
"transmit",             1, 0,
#endif /* NOXMIT */

#ifdef UNICODE
"unicode",              0, 0,
#else
"unicode",              1, 0,
#endif /* UNICODE */

#ifdef CK_XYZ
"xyzmodem",             0, 0,
#else
"xyzmodem",             1, 0,
#endif /* CK_XYZ */

"", 0, 0
};
int nftr = (sizeof(ftrtab) / sizeof(struct keytab)) - 1;

struct keytab desttab[] = {             /* SET DESTINATION */
#ifdef CALIBRATE
    "calibrate", DEST_N, CM_INV,
#endif /* CALIBRATE */
    "disk",    DEST_D, 0,
#ifdef CALIBRATE
    "nowhere", DEST_N, 0,
#endif /* CALIBRATE */
    "printer", DEST_P, 0,
    "screen",  DEST_S, 0
};
int ndests =  (sizeof(desttab) / sizeof(struct keytab));

#ifndef NOSPL           /* Used only with script programming items... */

#ifndef NOSERVER                        /* This is just to avoid some */
#define CK_PARSDIR                      /* "statement not reached" */
#else                                   /* complaints... */
#ifndef NODIAL
#define CK_PARSDIR
#endif /* NODIAL */
#endif /* NOSERVER */

/*
  cx == 0 means dial directory
  cx == 1 means network directory
  cx == 2 means a directory path list
*/
static int
parsdir(cx) int cx; {
    int i, x, y, dd;                    /* Workers */
    int nxdir;
    char *s;
    char ** xdir;
    char *pp[MAXGETPATH];               /* Temporary name pointers */
#ifdef ZFNQFP
    struct zfnfp * fnp;
#ifdef OS2
    char * env;
    char dirpath[4096];
#else /* OS2 */
    char dirpath[1024];                 /* For fully qualified filenames */
#endif /* OS2 */
#endif /* ZFNQFP */

    int max = 0;                        /* Maximum number of things to parse */
    char c;

#ifndef NODIAL
    if (cx == 0) {                      /* Dialing */
        nxdir = ndialdir;
        xdir = dialdir;
        max = MAXDDIR;
    } else
#ifdef NETCONN
    if (cx == 1) {                      /* Network */
        nxdir = nnetdir;
        xdir = netdir;
        max = MAXDDIR;
    } else
#endif /* NETCONN */
#endif /* NODIAL */
#ifndef NOSERVER
    if (cx == 2) {                      /* GET path */
        nxdir = ngetpath;
        xdir = getpath;
        max = MAXGETPATH;
    } else                              /* Called with invalid function code */
#endif /* NOSERVER */
      return(-2);

    for (i = 0; i < MAXGETPATH; i++)    /* Init these. */
      pp[i] = NULL;

#ifdef CK_PARSDIR
    dd = 0;                             /* Temporary name counter */
    while (1) {
        if (cx != 2) {                  /* Dialing or Network Directory */
#ifdef OS2
            int len;
            char * appdata0 = NULL, * appdata1 = NULL;
#ifdef NT
            env = getenv("K95PHONES");
            makestr(&appdata0,(char *)GetAppData(0));
            makestr(&appdata1,(char *)GetAppData(1));
#else /* NT */
            env = getenv("K2PHONES");
#endif /* NT */
            if (!env)
              env = getenv("K95PHONES");
            if (!env)
              env = "";

            dirpath[0] = '\0';
            len = strlen(env) + 2*strlen(startupdir) + 2*strlen(inidir)
                + (appdata0?2*strlen(appdata0):0) 
                + (appdata1?2*strlen(appdata1):0)
                + 2*strlen(zhome()) + 2*strlen(exedir) + 8*strlen("PHONES/")
                + 12;
            if (len < 4096)             /* SAFE */
              sprintf(dirpath,
                    "%s%s%s;%s%s;%s%s%s%s%s%s%s%s%s;%s%s;%s;%s%s",
                    /* Semicolon-separated path list */
                    env,
                    (env[0] && env[strlen(env)-1] == ';') ? "" : ";",
                    startupdir,
                    startupdir, "PHONES/",
                    appdata1 ? appdata1 : "", 
                    appdata1 ? "Kermit 95;" : "",
                    appdata1 ? appdata1 : "", 
                    appdata1 ? "Kermit 95/PHONES/;" : "",
                    appdata0 ? appdata0 : "", 
                    appdata0 ? "Kermit 95;" : "",
                    appdata0 ? appdata0 : "", 
                    appdata0 ? "Kermit 95/PHONES/;" : "",
                    inidir,
                    inidir, "PHONES/",
                    zhome(),
                    zhome(), "PHONES/",
                    exedir,
                    exedir, "PHONES/"
                    );
#ifdef NT
            makestr(&appdata0,NULL);
            makestr(&appdata1,NULL);
#endif /* NT */
#else
#ifdef UNIX
            y = 1024;
            s = dirpath;
            zzstring("\\v(home)",&s,&y);
#endif /* UNIX */
#endif /* OS2 */
            y = cmifip(
                  "Names of one or more directory files, separated by spaces",
                       "",&s,&x,0,
#ifdef OS2ORUNIX
                       dirpath,
#else
                       NULL,
#endif /* OS2ORUNIX */
                       xxstring
                       );
        } else {                        /* List of directory names */
            x = 0;
            y = cmdir("Directory name","",&s,xxstring);
        }
        if (y < 0) {
            if (y == -3) {              /* EOL or user typed <CR> */
                if ((y = cmcfm()) < 0) return(y);
                for (i = 0; i < max; i++) { /* Clear these */
                    if (i < nxdir && xdir[i]) {
                        free(xdir[i]);
                    }
                    xdir[i] = (i < dd) ? pp[i] : NULL;
                }
#ifndef NODIAL
                if (cx == 0)
                  ndialdir = dd;
#ifdef NETCONN
                if (cx == 1)
                  nnetdir = dd;
#endif /* NETCONN */
#endif /* NODIAL */
#ifndef NOSERVER
                if (cx == 2)
                  ngetpath = dd;
#endif /* NOSERVER */
                return(success = 1);

            } else {                    /* Parse error */
                for (i = 0; i < dd; i++) {  /* Free temp storage */
                    if (pp[i]) free(pp[i]); /* but don't change */
                    pp[i] = NULL;           /* anything else */
                }
                return(y);
            }
        }
        if (x) {
            printf("?Wildcards not allowed\n");
            return(-9);
        }
#ifdef CK_TMPDIR
        if (cx == 2 && !isdir(s)) {
            printf("?Not a directory - %s\n", s);
            return(-9);
        }
#endif /* CK_TMPDIR */

#ifdef ZFNQFP
        if (cx < 2) {
            if (!isabsolute(s)) {       /* If not relative get full path */
                if ((fnp = zfnqfp(s,TMPBUFSIZ - 1,tmpbuf))) {
                    if (fnp->fpath)
                      if ((int) strlen(fnp->fpath) > 0)
                        s = fnp->fpath;
                }
            }
        }
#endif /* ZFNQFP */
        c = NUL;
        x = strlen(s);
        if (x > 0)                      /* Get last char */
          c = s[x-1];
        debug(F000,"parsdir s",s,c);
        if ((pp[dd] = malloc(strlen(s)+2)) == NULL) {
            printf("?Internal error - malloc\n");
            for (i = 0; i < dd; i++) {  /* Free temp storage */
                if (pp[i]) free(pp[i]);
                pp[i] = NULL;
            }
            return(-9);
        } else {                        /* Have storage for name */
            strcpy(pp[dd],s);           /* Copy string into new storage */
            debug(F111,"parsdir pp[dd] 1",pp[dd],dd);
#ifndef NOXFER
            if (cx == 2) {              /* If we are parsing directories */
                char dirsep[2];
                extern int myindex;     /* Append directory separator if */
                extern struct sysdata sysidlist[]; /* it is missing...   */
                debug(F101,"parsdir myindex","",myindex);
                if (myindex > -1)
                  if (sysidlist[myindex].sid_unixlike)
                    if (c != sysidlist[myindex].sid_dirsep) {
                        dirsep[0] = sysidlist[myindex].sid_dirsep;
                        dirsep[1] = NUL;
                        strcat(pp[dd], (char *) dirsep); /* safe */
                    }
            }
#endif /* NOXFER */
            debug(F111,"parsdir pp[dd] 2",pp[dd],dd);
            if (++dd > max) {
                printf("?Too many directories - %d max\n", max);
                for (i = 0; i < dd; i++) {  /* Free temp storage */
                    if (pp[i]) free(pp[i]);
                    pp[i] = NULL;
                }
            }
        }
    }
#endif /* CK_PARSDIR */
}
#endif /* NOSPL */

#ifndef NOSERVER
static int
cklogin() {
    int x;
    char * s;
    char username[LOGINLEN+1];
    char password[LOGINLEN+1];
    char account[LOGINLEN+1];
    extern char * x_user, * x_passwd, * x_acct;
    extern int x_login, x_logged;

    username[0] = NUL;
    password[0] = NUL;
    account[0]  = NUL;

    x = cmfld("username", "", &s, xxstring);
    if (x != -3) {
        if (x < 0)
          return(x);
        if ((int)strlen(s) > LOGINLEN) {
            printf("\"%s\" - too long, %d max\n", s, LOGINLEN);
            return(-9);
        }
        ckstrncpy(username,s,LOGINLEN+1);
        x = cmfld("password", "", &s, xxstring);
        if (x != -3) {
            if (x < 0)
              return(x);
            if ((int)strlen(s) > LOGINLEN) {
                printf("\"%s\" - too long, %d max\n", s, LOGINLEN);
                return(-9);
            }
            ckstrncpy(password,s,LOGINLEN+1);
            x = cmfld("account", "", &s, xxstring);
            if (x != -3) {
                if (x < 0)
                  return(x);
                if ((int)strlen(s) > LOGINLEN) {
                    printf("\"%s\" - too long, %d max\n", s, LOGINLEN);
                    return(-9);
                }
                ckstrncpy(account,s,LOGINLEN+1);
                if ((x = cmcfm()) < 0)
                  return(x);
            }
        }
    }
    makestr(&x_user,username);
    makestr(&x_passwd,password);
    makestr(&x_acct,account);
    x_login = (x_user) ? 1 : 0;
    x_logged = 0;
    return(1);
}
#endif /* NOSERVER */

#ifndef NOLOCAL
static int
setdcd() {
    int x, y, z = 0;
    if ((y = cmkey(crrtab,ncrr,"","automatic",xxstring)) < 0) return(y);
    if (y == CAR_ON) {
        x = cmnum("Carrier wait timeout, seconds","0",10,&z,xxstring);
        if (x < 0) return(x);
    }
    if ((x = cmcfm()) < 0) return(x);
    carrier = ttscarr(y);
    cdtimo = z;
    return(1);
}
#endif /* NOLOCAL */

extern struct keytab yesno[];
extern int nyesno;

/* g e t y e s n o  */

static struct keytab q0yesno[] = {      /* Yes/No/Quit keyword table */
    "no",    0, 0,
    "ok",    1, 0,
    "yes",   1, 0
};
static int nq0yesno = (sizeof(q0yesno) / sizeof(struct keytab));

static struct keytab q1yesno[] = {      /* Yes/No/Quit keyword table */
    "no",    0, 0,
    "ok",    1, 0,
    "quit",  2, 0,
    "yes",   1, 0
};
static int nq1yesno = (sizeof(q1yesno) / sizeof(struct keytab));

static struct keytab q2yesno[] = {      /* Yes/No/Quit keyword table */
    "go",    3, 0,
    "no",    0, 0,
    "ok",    1, 0,
    "yes",   1, 0
};
static int nq2yesno = (sizeof(q2yesno) / sizeof(struct keytab));

static struct keytab q3yesno[] = {      /* Yes/No/Quit keyword table */
    "go",    3, 0,
    "no",    0, 0,
    "ok",    1, 0,
    "quit",  2, 0,
    "yes",   1, 0
};
static int nq3yesno = (sizeof(q3yesno) / sizeof(struct keytab));


/* Ask question, get yes/no answer */

int
getyesno(msg, flags) char * msg; int flags; {
#ifdef CK_RECALL
    extern int on_recall;               /* around Password prompting */
#endif /* CK_RECALL */
    int y, z;

#ifndef NOLOCAL
#ifdef OS2
    extern int vmode, win95_popup, startflags;
    int vmode_sav = vmode;
#endif /* OS2 */
#endif /* NOLOCAL */

#ifdef CK_APC
    if ( apcactive != APC_INACTIVE && (apcstatus & APC_NOINP) ) {
        return(success = 0);
    }
#endif /* CK_APC */

#ifndef NOLOCAL
#ifdef OS2
#ifdef COMMENT
    if (win95_popup && !(startflags & 96)
#ifdef IKSD
        && !inserver
#endif /* IKSD */
        )
      return(popup_readyesno(vmode,NULL,msg,flags));
#endif /* COMMENT */
    if (vmode == VTERM) {
        vmode = VCMD;
        VscrnIsDirty(VTERM);
        VscrnIsDirty(VCMD);
    }
#endif /* OS2 */
#endif /* NOLOCAL */
#ifdef VMS
/*
  In VMS, whenever a TAKE file or macro is active, we restore the
  original console modes so Ctrl-C/Ctrl-Y can work.  But here we
  go interactive again, so we have to temporarily put them back.
*/
    if (!xcmdsrc)
      concb((char)escape);
#endif /* VMS */

#ifdef CK_RECALL
    on_recall = 0;
#endif /* CK_RECALL */
    cmsavp(psave,PROMPTL);              /* Save old prompt */
    cmsetp(msg);                        /* Make new prompt */
    z = 0;                              /* Initialize answer to No. */
    cmini(ckxech);                      /* Initialize parser. */
    do {
        prompt(NULL);                   /* Issue prompt. */
        switch (flags) {
          case 0:  y = cmkey(q0yesno,nq0yesno,"","",NULL); break;
          case 1:  y = cmkey(q1yesno,nq1yesno,"","",NULL); break;
          case 2:  y = cmkey(q2yesno,nq2yesno,"","",NULL); break;
          default: y = cmkey(q3yesno,nq3yesno,"","",NULL);
        }
        if (y < 0) {
            if (y == -4) {              /* EOF */
                z = y;
                break;
            } else if (y == -3)         /* No answer? */
              printf(" Please respond; type '?' to see valid answers.\n");
            cmini(ckxech);
        } else {
            z = y;                      /* Save answer */
            y = cmcfm();                /* Get confirmation */
        }
    } while (y < 0);                    /* Continue till done */
    cmsetp(psave);                      /* Restore real prompt */
#ifdef VMS
    if (cmdlvl > 0)                     /* In VMS and not at top level, */
      conres();                         /*  restore console again. */
#endif /* VMS */
#ifndef NOLOCAL
#ifdef OS2
    if (vmode != vmode_sav) {
        vmode = VTERM;
        VscrnIsDirty(VCMD);
        VscrnIsDirty(VTERM);
    }
#endif /* OS2 */
#endif /* NOLOCAL */
    return(z);
}

#ifdef KUI
extern HWND hwndConsole;
_PROTOTYP(int gui_txt_dialog,(char *,char *,int,char *,int,char *,int));
_PROTOTYP(int gui_mtxt_dialog,(char *,int,struct txtbox []));
_PROTOTYP(int gui_position,(int, int));
_PROTOTYP(int gui_resize_mode,(int));
_PROTOTYP(int gui_win_run_mode,(int));
_PROTOTYP(int gui_saveas_dialog,(char *,char *, int, char *, char *, int));
extern int gui_dialog;
#endif /* KUI */

/* u q _ o k  --  User Query, get Yes/No or OK Cancel  */
/*
  Call with:  
    preface: Explanatory text to print, or NULL.
    prompt:  Prompt.
    mask:    Bitmask for legal responses: 1 = OK or Yes; 2 = No or Cancel.
    help:    Help text (array of strings or NULL) [not used by parser].
    dflt:    Default response (1 or 2) [not used by parser].
  Returns:
   -1:       Invalid argument(s).
    0:       User said No or Cancel.
    1        User said Yes or OK.    
  Notes:
    preface and prompt should not include final line terminator but may
    include embedded ones.  Help text is in case GUI dialog needs a Help
    button; final element of help-string array is "".  dflt is used by GUI
    to highlight the default response button.
*/
int
#ifdef CK_ANSIC
uq_ok(char * preface, char * prompt, int mask,char ** help, int dflt)
#else /* CK_ANSIC */
uq_ok(preface,prompt,mask,help,dflt)
    char * preface, * prompt, ** help;
    int mask, dflt;
#endif /* CK_ANSIC */
/* uq_ok */ {
    int rc, len;
    char * text=NULL;

    if (!prompt)
      return(-1);

    if ((mask & 3) == 1) {		/* OK (GUI only) */
#ifdef KUI
      if ( gui_dialog ) {
	/* This one is for popup help, alerts, etc */
        if (preface) {
            len = strlen(preface) + strlen(prompt) + 4;
            text = (char *)malloc(len);
            ckmakmsg(text,len,preface,"\n",prompt,NULL);
        }
        rc = MessageBox(hwndConsole,
                         text ? text : prompt,
                         prompt,
                         MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);
        ShowWindowAsync(hwndConsole,SW_SHOWNORMAL);
        SetForegroundWindow(hwndConsole);
        if (text)
	  free(text);
        if (!rc)
	  return(-1);
        else 
	  return(1);
      } else
#endif  /* KUI */
      {
	if (preface)			/* Just display the text, if any */
	  printf("%s\n",preface);
	if (prompt)
	  printf("%s\n",prompt);
        return(1);
      }
    } else if ((mask & 3) == 3) {	/* Yes/No or OK/Cancel */
#ifdef KUI
      if ( gui_dialog ) {
        if (preface) {
            len = strlen(preface) + strlen(prompt) + 4;
            text = (char *)malloc(len);
            ckmakmsg(text,len,preface,"\n",prompt,NULL);
        }
        rc = MessageBox(hwndConsole,
                         text ? text : prompt,
                         prompt,
                         MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL | 
                         (dflt == 2 ? MB_DEFBUTTON2 : MB_DEFBUTTON1));
        ShowWindowAsync(hwndConsole,SW_SHOWNORMAL);
        SetForegroundWindow(hwndConsole);
        if (text)
	  free(text);
        if (!rc)
	  return(-1);
        else if (rc == IDNO || rc == IDCANCEL)
	  return(0);
        else
	  return(1);
      } else
#endif  /* KUI */
      {
	if (preface)
	  printf("%s\n",preface);
	return(getyesno(prompt,0));
      }
    } else {
	printf("?Internal error: uq_ok()\n");
	return(-1);
    }
}

/* u q _ t x t  --  User Query, get single text response  */
/*
  Call with:  
    preface: Explanatory text to print, or NULL.
    prompt:  Prompt. 
    echo:    0 = don't echo; 1 = echo; 2 = echo with asterisks.
    help:    Help text (array of strings or NULL) [not used by parser].
    buf:     Pointer to result buffer.
    buflen:  Length of result buffer.
    dflt:    Default response text or NULL [not used by parser].
    timer:   Optional Timeout
  Returns:
    0:       User said No or Cancel.
    1        User said Yes or OK.    
  Notes:
    preface, prompt, and help as for uq_ok().
*/
int
#ifdef CK_ANSIC
uq_txt(char * preface, char * prompt, int echo, char ** help, char * buf, 
       int buflen, char *dflt, int timer)
#else /* CK_ANSIC */
uq_txt(preface,prompt,echo,help,buf,buflen,dflt,timer)
    char * preface, * prompt, ** help, * buf, * dflt; 
    int buflen, echo, timer;
#endif /* CK_ANSIC */
{
#ifndef NOLOCAL
#ifdef OS2
    extern int vmode;
    extern int startflags;
    extern int win95_popup;
#endif /* OS2 */
#endif /* NOLOCAL */
    int rc; 

    if (buflen < 1 || !buf)
      return(0);
#ifdef KUI
    if ( gui_dialog ) {
        rc = gui_txt_dialog(preface,prompt,echo,buf,buflen,dflt,timer);
        if ( rc > -1 )
            return(rc);
    /* Otherwise, the dialog could not be created.  Fallback to text mode */
    } 
#endif /* KUI */
#ifndef NOLOCAL
#ifdef OS2
    if (win95_popup && !(startflags & 96)
#ifdef IKSD
         && !inserver
#endif /* IKSD */
         ) {
        debok = 0;                          /* Don't log */
        if (echo == 1)
                popup_readtext(vmode,preface,prompt,buf,buflen,0);
            else
                popup_readpass(vmode,preface,prompt,buf,buflen,0);
        debok = 1;
        return(1);
    }
#endif /* OS2 */
#endif /* NOLOCAL */

    if (preface)
      printf("%s\n",preface);
    if (echo == 1)
      readtext(prompt,buf,buflen);
    else
      readpass(prompt,buf,buflen);
    return(1);				/* (no buttons in parser) */
}

/* u q _ m t x t  --  User Query, get multiple text responses */
/*
  Call with:  
    preface: Explanatory text to print, or NULL.
    help:    Help text (array of strings or NULL) [not used by parser].
    n:       Number of responses wanted.
    field:   Array of struct txtbox, one element per field, see ckuusr.h.
  Returns:
    0:       User said No or Cancel.
    1        User said Yes or OK.    
  Notes:
    preface and help as for uq_ok().
*/
int
#ifdef CK_ANSIC
uq_mtxt(char * preface,char **help, int n, struct txtbox field[])
#else /* CK_ANSIC */
uq_mtxt(preface,help,n,field)
    char * preface; char ** help; int n; struct txtbox field[]; 
#endif /* CK_ANSIC */
{
#ifndef NOLOCAL
#ifdef OS2
    extern int vmode;
    extern int startflags;
    extern int win95_popup;
#endif /* OS2 */
#endif /* NOLOCAL */
    int i, rc;

    if (n < 1 || !field)
      return(0);
#ifdef KUI
    if ( gui_dialog ) {
        rc = gui_mtxt_dialog(preface, n, field);
        if ( rc > -1 )
            return(rc);
    /* Otherwise, the dialog could not be created.  Fallback to text mode */
    }
#endif /* KUI */
#ifndef NOLOCAL
#ifdef OS2
    if (win95_popup && !(startflags & 96)
#ifdef IKSD
         && !inserver
#endif /* IKSD */
         ) {
        debok = 0;                          /* Don't log */
        for (i = 0; i < n; i++) {
            if (field[i].t_echo == 1)
                popup_readtext(vmode,preface,field[i].t_lbl,field[i].t_buf,field[i].t_len,0);
            else
                popup_readpass(vmode,preface,field[i].t_lbl,field[i].t_buf,field[i].t_len,0);
        }
        debok = 1;
        return(1);
    }
#endif /* OS2 */
#endif /* NOLOCAL */

    if (preface)
      printf("%s\n",preface);
    for (i = 0; i < n; i++) {
	if (field[i].t_echo == 1)
	  readtext(field[i].t_lbl,field[i].t_buf,field[i].t_len);
	else
	  readpass(field[i].t_lbl,field[i].t_buf,field[i].t_len);
    }
    return(1);
}

/* u q _ f i l e  --  User Query, get file or directory name  */
/*
  Call with:  
    preface: Explanatory text to print, or NULL.
    prompt:  Prompt string.
    fc:      Function code:
	       1 = input (existing) file
	       2 = existing directory 
	       3 = create new output file
	       4 = output file allowing append access
    help:    Help text (array of strings or NULL) [not used by parser].
    dflt:    Default response.
    result:  Pointer to result buffer.
    rlength: Length of result buffer.

  Returns:
   -1:       Invalid argument, result too long, or other error.
    0:       User Canceled.
    1:       OK, with file/pathname copied to result buffer.
    2:       Like 1, but for output file that is to be appended to.

  Notes:
    1. preface and prompt should not include final line terminator but may
       include embedded ones.  Help text is in case GUI dialog needs a Help
       button; final element of help-string array is "".

    2. The default might be a filename, a directory name, a relative
       pathname, or an absolute pathname.  This routine must convert it into
       into a fully qualified (absolute) pathname so the user knows exactly
       where the file is to be found or stored.  In addition, the Windows
       version of this routine must separate the directory part from the
       name part, so it can display the given directory in the file dialog,
       and put name in the filename box to be edited, replaced, or
       accepted.

    3. When called with FC 4, the Windows version should include "New" and
       "Append" buttons in the dialog. so the user can say whether the file
       should overwrite any file of the same name, or be appended to it.
*/

int
#ifdef CK_ANSIC
uq_file(char * preface, char * fprompt, int fc, char ** help,
	char * dflt, char * result, int rlength)
#else /* CK_ANSIC */
uq_file(preface,fprompt,fc,help,dflt,result,rlength)
    char * preface, * fprompt, ** help, * dflt, * result;
    int fc, rlength;
#endif /* CK_ANSIC */
/* uq_file */ {

    int rc = -1, x, y, z;
    char * s, * p, * fullpath;
    char filebuf[CKMAXPATH+1];

#ifdef CK_RECALL
    extern int on_recall;
#endif /* CK_RECALL */

#ifdef KUI
    if ( gui_dialog ) {
        rc = gui_saveas_dialog(preface,fprompt,fc,dflt,result,rlength);
        return rc;
    }
#endif /* KUI */

#ifdef CK_RECALL
    on_recall = 0;
#endif /* CK_RECALL */

    if (preface)			/* If prefatory text given... */
      printf("%s\n",preface);		/* display it. */

    cmsavp(psave,PROMPTL);              /* Save old prompt */

    /* We get the full pathname of the proposed output file just so */
    /* we can show it to the user but we don't use it ourselves. */

    p = NULL;				/* Build new prompt */
    if (!dflt) dflt = "";
    if (*dflt)				/* Have default filename */
      zfnqfp(dflt,CKMAXPATH+1,filebuf);	/* Get full path */
    else
      ckmakmsg(filebuf,CKMAXPATH+1,zgtdir(),"newfile",NULL,NULL);
    fullpath = filebuf;
    x = strlen(fullpath);

    /* If no prompt given, build one that shows the proposed full pathname. */

    if (!fprompt) fprompt = "";
    if (!*fprompt) fprompt = x ? " Filename" : " Filename: ";
    y = strlen(fprompt);
    if (x > 0) {			/* Have default pathname? */
	p = (char *)malloc(x + y + 7);	/* Get temp storage */
	if (p) {			/* Build prompt */
	    ckmakmsg(p,x+y+7,fprompt," [",fullpath,"]: ");
	    fprompt = p;
	}
    }
    cmsetp(fprompt);			/* Make new prompt */
    if (p) free(p);			/* Free temp storage */
    cmini(ckxech);                      /* Initialize parser. */
    x = -1;
    do {
        prompt(NULL);                   /* Issue prompt. */
        switch (fc) {			/* Parse depends on function code */
          case 1:			/* Input file */
	    x = cmifi("Name of existing file",dflt,&s,&y,xxstring);
	    rc = 1;
	    break;
	  case 2:			/* Directory */
	    x = cmdir("Directory name",dflt,&s,xxstring);
	    rc = 1;
	    break;
          case 3:			/* New output file */
	    /* Fall thru... */
	  case 4:			/* Output file - Append allowed */
	    x = cmofi("Output file specification",dflt,&s,xxstring);
	    rc = (fc == 4) ? 1 : 2;
	    break;
          default:			/* Bad function code */
	    goto x_uq_file;
        }
        if (x < 0) {			/* Parse error */
	    filebuf[0] = NUL;
            if (x == -4) {              /* EOF */
                break;
            } else if (x == -3)         /* No answer? */
              printf(fc == 2 ?
		     " Please enter a directory name.\n" :
		     " Please enter a filename.\n"
		     );
            cmini(ckxech);
        } else {
	    z = strlen(s);
	    if (z > rlength || ckstrncpy(filebuf,brstrip(s),CKMAXPATH+1) < z) {
		printf("?Name too long\n");
		x = -9;
	    } else
	      x = cmcfm();		/* Get confirmation */
        }
	if (fc == 1 && x > -1 && y > 0) {
	    printf("?Wildcards not allowed\n");
	    x = -9;
	}
    } while (x < 0);                    /* Continue till done */

  x_uq_file:
    if (x < 0)
      rc = -1;

    cmsetp(psave);                      /* Restore real prompt */

    if (rc > 0)
      ckstrncpy(result,filebuf,rlength);
    return(rc);
}


#ifdef CK_PERMS
#ifdef UNIX

_PROTOTYP( int zsetperm, (char *, int));

/* CHMOD command for UNIX only */

#define CHM_DIR 0
#define CHM_DOT 1
#define CHM_FIL 2
#define CHM_LIS 3
#define CHM_NOL 4
#define CHM_QUI 5
#define CHM_REC 6
#define CHM_VRB 7
#define CHM_PAG 8
#define CHM_NOP 9
#define CHM_TYP 10
#define CHM_SIM 11

static struct keytab uchmodsw[] = {
    "/directories", CHM_DIR, 0,
    "/dotfiles",    CHM_DOT, 0,
    "/files",       CHM_FIL, 0,
    "/list",        CHM_LIS, 0,
    "/nolist",      CHM_NOL, 0,
    "/nopage",      CHM_NOP, 0,
    "/page",        CHM_PAG, 0,
    "/quiet",       CHM_QUI, CM_INV,
    "/recursive",   CHM_REC, 0,
    "/simulate",    CHM_SIM, 0,
    "/type",        CHM_TYP, CM_ARG,
    "/verbose",     CHM_VRB, CM_INV,
};
static int nchmodsw = (sizeof(uchmodsw) / sizeof(struct keytab));

int
douchmod() {
    extern int recursive, nscanfile, diractive;
#ifdef CK_TTGWSIZ
    extern int tt_rows, tt_cols;
    int n = 0;
#endif /* CK_TTGWSIZ */
    int i, files = 1, t1 = 1, t2 = 0, x, y, z, verbose = 0, rc = 1, paging;
    int xmode = -1, fs = 0, getval = 0, simulate = 0, wild = 0;
    char c, * s;
    struct FDB sw, nu;

    if (xaskmore < 0) {
#ifdef CK_TTGWSIZ
        xaskmore = 1;
#else
        xaskmore = 0;
#endif /* CK_TTGWSIZ */
    }
    paging = xaskmore;

    cmfdbi(&sw,                         /* First FDB - command switches */
           _CMKEY,                      /* fcode */
           "Octal file permission code, or switch",
           "",                          /* default */
           "",                          /* addtl string data */
           nchmodsw,                    /* addtl numeric data 1: tbl size */
           4,                           /* addtl numeric data 2: 4 = cmswi */
           xxstring,                    /* Processing function */
           uchmodsw,                    /* Keyword table */
           &nu                          /* Pointer to next FDB */
           );
    cmfdbi(&nu,
           _CMNUM,                      /* Number */
           "",                          /* Help message */
           "",                          /* Default */
           "",                          /* N/A */
           8,                           /* Radix = 8 */
           0,                           /* N/A */
           xxstring,                    /* Processing function */
           NULL,                        /* N/A */
           NULL                         /* Next */
           );

    while (1) {
        if ((x = cmfdb(&sw)) < 0) {
            if (x == -3) {
                x = -9;
                printf("?Filename required\n");
            }
            return(x);
        }
        if (cmresult.fcode != _CMKEY)
          break;
        c = cmgbrk();
        getval = (c == ':' || c == '=');
        if (getval && !(cmgkwflgs() & CM_ARG)) {
            printf("?This switch does not take an argument\n");
            return(-9);
        }
        if (!getval && (cmgkwflgs() & CM_ARG)) {
            printf("?This switch requires an argument\n");
            return(-9);
        }
        switch (cmresult.nresult) {
          case CHM_DIR:
            t1 = 1;
            t2 = 1;
            break;
          case CHM_DOT:
            matchdot = 1;
            break;
          case CHM_FIL:
            t1 = 0;
            t2 = 0;
            break;
          case CHM_LIS:
          case CHM_VRB:
            verbose = 1;
            break;
          case CHM_NOL:
          case CHM_QUI:
            verbose = 0;
            break;
          case CHM_REC:
            recursive = 1;
            break;
          case CHM_PAG:
            verbose = 1;
            paging = 1;
            break;
          case CHM_NOP:
            paging = 0;
            break;
          case CHM_SIM:
            simulate = 1;
            break;
          case CHM_TYP: {
              extern struct keytab txtbin[];
              if ((x = cmkey(txtbin,3,"","",xxstring)) < 0)
                return(x);
              if (x == 2) {             /* ALL */
                  xmode = -1;
              } else {                  /* TEXT or BINARY only */
                  xmode = x;
                  fs = 1;
              }
              break;
          }
        }
    }
    z = cmresult.nresult;
    x = cmifi2("File specification","",&s,&wild,t1,NULL,xxstring,t2);
    if (x < 0) {
        if (x == -3) {
            printf("?A file specification is required\n");
            return(-9);
        } else
          return(x);
    }
    ckstrncpy(tmpbuf,s,TMPBUFSIZ);
    s = tmpbuf;
    if ((x = cmcfm()) < 0)
      return(x);
#ifdef ZXREWIND
    if (wild) files = zxrewind();
#else
    if (wild) files = nzxpand(s,0);
#endif /* ZXREWIND */

    if (paging > -1)
      xaskmore = paging;

#ifdef CK_TTGWSIZ
    if (verbose && paging) {
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
    }
#endif /* CK_TTGWSIZ */

    for (i = 0; i < files; i++) {
        if (files == 1 && wild == 0) {  /* For "chmod 777 ." */
            ckstrncpy(line,s,LINBUFSIZ);
        } else {
            x = znext(line);
            if (x < 1) {
                if (i == 0) {
                    printf("?No files match - \"%s\"\n",line);
                    return(-9);
                }
                return(1);
            }
        }
        if (fs) {
#ifdef VMSORUNIX
            /* If /TYPE:TEXT or BINARY given, skip directories and links */
            /* since they are neither text nor binary. */
            extern int zgfs_dir, zgfs_link;
            zgetfs(line);
            if (zgfs_dir || zgfs_link)
              continue;
#else
            if (zchki(line) < 0)
              continue;
#endif /* VMSORUNIX */
            /* Regular file, scan it */
            switch (scanfile(line,&y,nscanfile)) {
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
        if (simulate) {
#ifdef UNIX
            extern int zchkod;          /* Unidentified Flying */
            int xx = zchkod;            /* API Extension... */
            zchkod = 1;
#endif /* UNIX */
            if (zchko(line) < 0)
              printf("%s - Access denied\n",line);
            else
              printf("%s - OK\n",line);
#ifdef UNIX
            zchkod = xx;
#endif /* UNIX */
        } else {
            if (zsetperm(line,z) < 1) {
                if (verbose || files < 2) {
                    printf("%s: %s\n",line,ck_errstr());
                }
                rc = 0;
            } else if (verbose) {
                printf("%s  %s\n",ziperm(line),line);
            }
        }
#ifdef CK_TTGWSIZ
        if (verbose && paging) {        /* Pause at end of screen */
            if (cmd_rows > 0 && cmd_cols > 0) {
                if (++n > cmd_rows - 3) {
                    if (!askmore())
                      break;
                    else
                      n = 0;
                }
            }
        }
#endif /* CK_TTGWSIZ */

    }
    return(success = rc);
}
#endif /* UNIX */
#endif /* CK_PERMS */

#ifndef NOSPL                           /* S-Expressions */
#ifndef NOSEXP

struct keytab sexptab[] = {
    "depth-limit", 1, 0,
    "echo-result", 0, 0,
    "truncate-all-results", 2
};

static int sexpmaxdep = 1000;           /* Maximum depth */

#define xxfloat(s,x) \
((isdigit(*s)||(*s=='-')||(*s=='+')||(*s=='.')||(*s=='\040'))?isfloat(s,x):0)

#define SX_ADD  1                       /* Symbols for built-in operators */
#define SX_SUB  2
#define SX_MUL  3
#define SX_DIV  4
#define SX_POW  5
#define SX_SET  6
#define SX_MOD  7
#define SX_EVA  8
#define SX_EXP  9
#define SX_AEQ 10
#define SX_ALT 11
#define SX_AGT 12
#define SX_ALE 13
#define SX_AGE 14
#define SX_MIN 15
#define SX_MAX 16
#define SX_SQR 17
#define SX_FLR 18
#define SX_CEI 19
#define SX_TRU 20
#define SX_ABS 21
#define SX_ROU 22
#define SX_LET 23
#define SX_LGN 24
#define SX_LGX 25
#define SX_FLO 26
#define SX_IFC 27
#define SX_NOT 28
#define SX_NEQ 29
#define SX_AND 30
#define SX_LOR 31
#define SX_SIN 32
#define SX_COS 33
#define SX_TAN 34
#define SX_BWA 35
#define SX_BWO 36
#define SX_BWX 37
#define SX_BWN 38
#define SX_XOR 39
#define SX_INC 40
#define SX_DEC 41
#define SX_QUO 42
#define SX_STR 43

/* Operator flags */

#define SXF_PRE 256                     /* Predicate */
#define SXF_ONE 512                     /* Requires one arg */
#define SXF_TWO 1024                    /* Requires two args or more */
#define SXF_FLO 2048                    /* Coerce to floating-point */

/* Built-in constants */

#define SXC_NIL 1                       /* NIL */
#define SXC_PI  2                       /* PI */
#define SXC_T   3                       /* T */

/*
  This is an xlookup() table and so need not be in "alhabetical" order.
  Therefore entries are arranged to minimize search for most common
  operators.
*/
static struct keytab sexpops[] = {      /* Built-in operators */
    "setq",    SX_SET, 0,               /* Global assignment */
    "+",       SX_ADD, 0,               /* Simple arithmetic */
    "-",       SX_SUB, 0,
    "*",       SX_MUL, 0,
    "/",       SX_DIV, SXF_TWO,
    "^",       SX_POW, SXF_TWO,

    "if",      SX_IFC, SXF_TWO,         /* IF */
    "let",     SX_LET, 0,               /* Local assignment */
    "not",     SX_NOT, SXF_ONE,         /* NOT */
    "mod",     SX_MOD, SXF_TWO,         /* Modulus */

    "<",       SX_ALT, SXF_PRE,		/* Comparisons */
    ">",       SX_AGT, SXF_PRE,
    "<=",      SX_ALE, SXF_PRE,
    "=",       SX_AEQ, SXF_PRE,
    ">=",      SX_AGE, SXF_PRE,
    "!=",      SX_NEQ, SXF_PRE,

    "++",      SX_INC, SXF_ONE|SXF_TWO, /* Increment */
    "--",      SX_DEC, SXF_ONE|SXF_TWO, /* Decrement */

    "**",      SX_POW, SXF_TWO,         /* Common synonyms */
    "==",      SX_AEQ, SXF_PRE,
    "!",       SX_NOT, SXF_ONE,
    ".",       SX_EVA, 0,

    "and",     SX_AND, 0,               /* Logical operators */
    "or",      SX_LOR, 0,
    "xor",     SX_XOR, SXF_TWO,

    "max",     SX_MAX, SXF_ONE|SXF_TWO, /* Max and min */
    "min",     SX_MIN, SXF_ONE|SXF_TWO,

    "%",       SX_MOD, SXF_TWO,         /* More synonyms */
    "||",      SX_LOR, 0,
    "&&",      SX_AND, 0,

    "quote",   SX_QUO, SXF_ONE,
    "string",  SX_STR, SXF_ONE,

    "eval",    SX_EVA, 0,               /* Assorted commands */
    "abs",     SX_ABS, SXF_ONE,
    "truncate",SX_TRU, SXF_ONE|SXF_FLO,
    "round",   SX_ROU, SXF_ONE|SXF_TWO|SXF_FLO,
    "ceiling", SX_CEI, SXF_ONE|SXF_FLO,
    "floor",   SX_FLR, SXF_ONE|SXF_FLO,
    "float",   SX_FLO, SXF_ONE|SXF_FLO,

#ifdef FNFLOAT
    "sqrt",    SX_SQR, SXF_ONE|SXF_FLO, /* Floating point functions */
    "exp",     SX_EXP, SXF_ONE|SXF_FLO,
    "sin",     SX_SIN, SXF_ONE|SXF_FLO,
    "cos",     SX_COS, SXF_ONE|SXF_FLO,
    "tan",     SX_TAN, SXF_ONE|SXF_FLO,
    "log",     SX_LGN, SXF_ONE|SXF_FLO,
    "log10",   SX_LGX, SXF_ONE|SXF_FLO,
#endif /* FNFLOAT */

    "#",       SX_BWX, SXF_TWO,         /* Bitwise operators */
    "&",       SX_BWA, 0,
    "|",       SX_BWO, 0,
    "~",       SX_BWN, SXF_ONE,
    "", 0, 0                            /* (end) */
};
static int nsexpops = (sizeof(sexpops) / sizeof(struct keytab)) - 1;

static struct keytab sexpconsts[] = {   /* Built-in constants */
    "nil", SXC_NIL, 0,                  /* NIL (false) */
    "pi",  SXC_PI,  0,                  /* Pi (3.1415926...) */
    "t",   SXC_T,   0,                  /* T (true) */
    "", 0, 0
};
static int nsexpconsts = (sizeof(sexpconsts) / sizeof(struct keytab)) - 1;

int sexprc = 0;                         /* S-Expression error flag */
int sexppv = -1;                        /* Predicate value */
static int sexptrunc = 0;		/* Flag to force all results to int */

#define SXMLEN 64                       /* Macro arg list initial length */
#include <math.h>                       /* Floating-point functions */

_PROTOTYP( char * fpformat, (CKFLOAT, int, int) );
_PROTOTYP( CKFLOAT ckround, (CKFLOAT, int, char *, int) );

extern char math_pi[];                  /* Value of Pi */
extern int sexpecho;                    /* SET SEXPRESSION ECHO value */
extern char * sexpval;                  /* Last top-level S-Expression value */
extern char * lastsexp;                 /* Last S-Expression */
int sexprmax = 0;                       /* Longest result (for stats) */
int sexpdmax = 0;                       /* Max depth reached (for stats) */
int sexpdep  = 0;                       /* dosexp() recursion depth */
static int * sxrlen = NULL;             /* Result stack string sizes */
static char ** sxresult = NULL;         /* Result stack */

/*  s h o s e x p  --  Show S-Expression info  */

VOID
shosexp() {
    printf("\n");
    printf(" sexpression echo-result: %s\n",showooa(sexpecho));
    printf(" sexpression depth-limit: %d\n",sexpmaxdep);
    printf("\n");
    printf(" maximum depth reached:   %d\n",sexpdmax);
    printf(" longest result returned: %d\n",sexprmax);
    printf("\n");
    printf(" truncate all results:    %s\n",showoff(sexptrunc));
    printf("\n");
    printf(" last sexpression:        %s\n",lastsexp ? lastsexp : "(none)");
    printf(" last value:              %s\n",sexpval ? sexpval : "(none)");
    printf("\n");
}


static char *
sexpdebug(s) char * s; {
    /* For debugging -- includes recursion depth in each debug entry */
    static char buf[64];
    ckmakmsg(buf,64,"dosexp[",ckitoa(sexpdep),"] ",s);
    return((char *)buf);
}

/*  d o s e x p  --  S-Expression Reader  */

/*  Returns value as string (empty, numeric, or non-numeric) */

static char sxroundbuf[32];		/* For ROUND result */

char *
dosexp(s) char *s; {                    /* s = S-Expression */
    extern struct mtab *mactab;         /* Macro table */
    extern int maclvl, nmac;
    extern char *mrval[];
    extern int makestrlen;              /* (see makestr()) */
    struct stringarray * q = NULL;      /* cksplit() return type */
    char * p[SEXPMAX+1], ** p2;         /* List items (must be on stack) */
    char * line = NULL;                 /* For building macro argument list */
    int nosplit = 0;
    int linelen = 0;
    int linepos = 0;
    int quote = 0;                      /* LISP quote flag */
    char * s2;                          /* Workers */
    int kw, kwflags, mx = 0, x = 0;
    int not = 0, truncate = 0, builtin = 0;
    int fpflag = 0, quit = 0, macro = 0;
    CK_OFF_T result = 0, i, j, k, n = 0;
    CKFLOAT fpj, fpresult = 0.0;        /* Floating-point results */
    int pflag = 0;                      /* Have predicate */
    int presult = 0;                    /* Predicate result */
    int mustfree = 0;                   /* If we malloc'd we must free */

    sexppv = -1;                        /* Predicate value */
    s2 = "";                            /* Default return value */

    debug(F111,sexpdebug("entry 1"),s,sexprc);

    if (++sexpdep > sexpmaxdep) {       /* Keep track of depth */
        printf("?S-Expression depth limit exceeded: %d\n",sexpmaxdep);
        sexprc++;
        debug(F111,sexpdebug("max depth exceeded"),s,sexprc);
    }
    if (sexpdep > sexpdmax)             /* For stats */
      sexpdmax = sexpdep;

    if (sexprc)                         /* Error, quit all levels */
      goto xdosexp;                     /* Always goto common exit point */

    debug(F111,sexpdebug("entry 2"),s,sexprc);

    if (!s) s = "";                     /* Null or empty arg */

    while (*s == SP) s++;               /* Strip leading spaces */
    if (!*s)                            /* so empty result */
      goto xdosexp;
/*
  Allocate result stack upon first use, or after it has been resized with
  SET SEXP DEPTH-LIMIT.
*/
    if (!sxresult) {
        sxresult = (char **)malloc(sexpmaxdep * sizeof(char *));
        if (!sxresult) {
            printf("?Memory allocation failure - \"%s\"\n", s);
            sexprc++;
            goto xdosexp;
        }
        sxrlen = (int *)malloc(sexpmaxdep * sizeof(int));
        if (!sxrlen) {
            printf("?Memory allocation failure - \"%s\"\n", s);
            sexprc++;
            goto xdosexp;
        }
        for (i = 0; i < sexpmaxdep; i++) {
            sxresult[i] = NULL;         /* Result pointers */
            sxrlen[i] = 0;              /* Buffer sizes */
        }
    }
    s2 = s;                             /* s2 is the result pointer */
    k = 0;                              /* Length accumulator */
    if (s[0] == '(') {                  /* Starts with open paren? */
        while (*s2++) k++;              /* Get length */
        if (s[k-1] == ')') {            /* Strip outer parens if any */
            s[k-1] = NUL;
            s++;
            k -= 2;
            while (*s == SP) {          /* Strip leading spaces from result */
                s++;
                k--;
            }
            while (k > 0 && s[k-1] == SP) { /* And trailing spaces. */
                s[k-1] = NUL;
                k--;
            }
        }
        if (!*s) {                      /* If nothing remains */
            s2 = "";                    /* return empty result. */
            goto xdosexp;
        }
    }
    /* Break result up into "words" (an SEXP counts as a word) */

    for (i = 0; i < SEXPMAX+1; i++ ) {	/* Clear the operands */
	p[i] = NULL;
    }
    if (!*(s+1) || !*(s+2)) {           /* No need to call cksplit() */
        n = 1;                          /* if it's one or two chars. */
        p[1] = s;                       /* No need to malloc this either. */
	nosplit = 1;
        debug(F101,sexpdebug("nosplit"),"",n);
        if (s[0] == '(') {              /* () empty */
            s2 = "";
            goto xdosexp;
        }
    } else {
	nosplit = 0;
        q = cksplit(1,SEXPMAX,s,NULL,"\\%[]&$+-/=*^_@!{}/<>|.#~'`:;?",8,39,0);
        if (!q)
          goto xdosexp;
        n = q->a_size;                  /* Number of items */
        debug(F101,sexpdebug("split"),"",n);
        if (n < 0 || n > SEXPMAX) {     /* Check for too many */
            printf("?Too many operands: max = %d\n",SEXPMAX);
            sexprc++;
            goto xdosexp;
        }
        if (n == 0)                     /* None, result is NULL, done. */
          goto xdosexp;
        if (n == 1 && s[0] == '(') {    /* One but it's another SEXP */
            s2 = dosexp(s);
            goto xdosexp;
        }
        p2 = q->a_head;                 /* Point to result array. */
        for (i = 1; i <= n; i++) {      /* We must copy it because */
            p[i] = NULL;                /* recursive calls to dosexp() */
            if (p2[i])                  /* write over the same array */
              makestr(&(p[i]),p2[i]);
        }
        if (s[0] == '(') {              /* Operator is an S-Expression */
            s2 = dosexp(p[1]);          /* Replace it by its value */
            makestr(&(p[1]),s2);
        }
        mustfree++;                     /* Remember to free it */
    }
    debug(F110,sexpdebug("head"),p[1],0);

    if (n == 1 && p[1]) {
        if (*(p[1]) == '\047') {       /* Apostrophe = LISP quote character */
            s2 = p[1];
            goto xdosexp;
        }
    }
/*
  This section sidesteps xlookup() of the most common operators.
  It's not necessary but it speeds up SEXP-heavy loops by about 10%.
*/
    kwflags = 0;
    if (n > 0) {                        /* Look up the operator */
        s2 = p[1];                      /* Prelookup optimization... */
        if (!s2)
          s2 = "";
        if (!*s2)
          goto xdosexp;
        kw = 0;
        x = 0;
        if (isdigit(*s2)) {             /* Digit */
            x = -2;

        } else if (isalpha(*s2) && !*(s2+1)) { /* Single letter */
            x = -1;

        } else if (*s2 == 's' || *s2 == 'S') { /* SETQ */
            s2++;
            if (*s2 == 'e' || *s2 == 'E') {
                s2++;
                if (*s2 == 't' || *s2 == 'T') {
                    s2++;
                    if (*s2 == 'q' || *s2 == 'Q') {
                        if (!*(s2+1)) {
                            x = SX_SET;
                            kwflags = 0;
                            builtin = 1;
                        }
                    }
                }
            }
        }
        if (!x) {
            if (!*(s2+1)) {             /* Common single-character ops */
                if (*s2 == '+') {
                    x = SX_ADD;
                    kwflags = 0;
                    builtin = 1;
                } else if (*s2 == '-') {
                    x = SX_SUB;
                    kwflags = 0;
                    builtin = 1;
                } else if (*s2 == '*') {
                    x = SX_MUL;
                    kwflags = 0;
                    builtin = 1;
                } else if (*s2 == '/') {
                    x = SX_DIV;
                    kwflags = SXF_TWO;
                    builtin = 1;
                }
            }
            if (!x) {                   /* None of the above, look it up */
                x = xlookup(sexpops,p[1],nsexpops,&kw);
                if (x > 0) {
                    kwflags = sexpops[kw].flgs;
                    builtin = 1;
                }
            }
        }
    }
    /* If none of the above, check built-in constants */

    if (x == -1) {
        x = xlookup(sexpconsts,p[1],nsexpconsts,&kw);
        if (x > 0) {
            switch (x) {
              case SXC_NIL:
                s2 = "";
                goto xdosexp;
              case SXC_PI:
                s2 = math_pi;
                goto xdosexp;
              case SXC_T:
                s2 = "1";
                goto xdosexp;
            }
        }
    }
    if (n == 1) {                       /* Not an expression */
        if (builtin) {                  /* Built-in operand? */
            switch (x) {		/* Operators with default values */
              case SX_EVA:
                s2 = "";
                goto xdosexp;
              case SX_MUL:              /* (*) */
                s2 = sexpval ? sexpval : "1";
                goto xdosexp;
              case SX_AND:              /* (AND) */
              case SX_BWA:              /* Bitwise (&) */
                result++;
              case SX_LOR:              /* (OR) */
              case SX_BWO:              /* Bitwise (|) */
              case SX_ADD:              /* (+) */
              case SX_SUB:              /* (-) */
                s2 = result ? "1" : "0";
                goto xdosexp;
            }

        } else {                        /* Not a built-in operand */
            char * p1;
            p1 = p[1];
            while (*p1 == SP) p1++;
            if (!isalpha(*p1)) {
                if (xxfloat(p1,0) > 0) { /* Is it a number? */
                    s2 = p1;
                    while (*s2 == '+') s2++;
                } else if (*p1 == '(') { /* An S-Expression? */

#ifdef COMMENT
                    s2 = dosexp(s2);
#else
                    s2 = dosexp(p1);
#endif /* COMMENT */
                }
                goto xdosexp;
            } else if (x < 1) {         /* Is it a variable? */
                j = mxlook(mactab,p[1],nmac); /* Look it up */
                debug(F111,sexpdebug("n==1 mxlook"),p[1],j);
                s2 = (j > -1) ? mactab[j].mval : "";
                if (!s2) s2 = "";
                if (xxfloat(s2,0) > 0)  /* Macro value is a number */
                  goto xdosexp;
                if (j > -1) {           /* It's a macro */
                    mx = j;
                    x = j;              /* whose definition is not numeric */
                    if (*s2 == '(') {   /* Is it an S-Expression? */
                        /* We have to allocate memory on the stack */
                        /* to call ourselves recursively on it */
                        /* otherwise we'll wipe out the macro definition */
                        char * s3 = NULL;
                        /* int k = 0; */
                        s3 = s2;
                        while (*s3++) k++;
                        s3 = (char *)malloc(k + 4);
                        if (s3) {
                            strcpy(s3,s2);   /* SAFE */
                            s2 = dosexp(s3); /* Evaluate it */
                            free(s3);
                        } else {
                            printf("?Memory allocation failure - \"%s\"\n",s2);
                            sexprc++;
                        }
                        goto xdosexp;
                    }
                    if (*s2 == '\047') {
                        s2++;
#ifdef COMMENT
			/* Dumps core if petty optimization was taken */
                        makestr(&(p[1]),s2);
#else
			if (!nosplit && p[1]) free(p[1]);
			p[1] = (char *)malloc((int)strlen(s2) + 1);
#endif /* COMMENT */
                        s2 = p[1];
			if (!s2) s2 = "";
                        if (*s2 == '(') {
                            if (s2[makestrlen-1] == ')') {
                                s2[makestrlen-1] = NUL;
                                s2++;
                            }
                        }
                        debug(F110,sexpdebug("'A"),s2,0);
                        goto xdosexp;
                    }
                    macro++;            /* Not an S-Expression */
                } else {                /* Not found in macro table */
                    printf("?Not defined - \"%s\"\n", p[1]);
                    sexprc++;
                    goto xdosexp;
                }
            }
        }
    } else if (x < 1 && !macro) {       /* n > 1 and not a built-in operator */
        x = mxlook(mactab,p[1],nmac);   /* See if it's a macro */
        debug(F111,sexpdebug("n!=1 mxlook"),p[1],x);
        if (x < 0) {
            printf("?Invalid operand - \"%s\"\n",p[1]);
            sexprc++;
            goto xdosexp;
        }
        mx = x;
        macro++;
    }
    if (builtin) {                      /* Built-in operator... */
        if (kwflags) {
            int flgs;
            if ((flgs = (kwflags & (SXF_ONE|SXF_TWO)))) {
                switch (flgs) {
                  case (SXF_ONE|SXF_TWO):
                    if (n < 2) {
                        printf("?Too few operands - \"%s\"\n",s);
                        sexprc++;
                        goto xdosexp;
                    }
                    break;
                  case SXF_TWO:
                    if (n < 3) {
                        printf("?Too few operands - \"%s\"\n",s);
                        sexprc++;
                        goto xdosexp;
                    }
                    break;
                  case SXF_ONE:
		    if (n != 2) {
                        printf("?Too %s operands - \"%s\"\n",
                               (n > 2) ? "many" : "few", s);
                        sexprc++;
                        goto xdosexp;
                    }
                }
            }
            if (kwflags & SXF_PRE) {    /* Predicate? */
		if (n < 2) {
		    printf("?Too few operands - \"%s\"\n",s);
		    sexprc++;
		    goto xdosexp;
		}
                pflag = 1;
                presult = 1;
            }
            if (kwflags & SXF_FLO)      /* Operator requires floating point */
              fpflag++;                 /* Force it */

	    if (x == SX_ROU) {		/* ROUND can have 1 or 2 arguments */
		if (n < 2 || n > 3) {
		    printf("?Too %s operands - \"%s\"\n",
			   (n > 3) ? "many" : "few", s);
		    sexprc++;
		    goto xdosexp;
		}
	    }
	    if (x == SX_ROU) {
		/* But they are not "cumulative" like other SEXP args */
		/* So this case is handled specially */
		char buf1[32], buf2[32];
		float r;
		char * s0, * s1;
		char * q0, * q1;

		s0 = p[2];
		if (!s0) s0 = "";
		if (!*s0) s0 = "0";
		q0 = dosexp(s0);
		ckstrncpy(buf1,q0,32);
		q0 = buf1;

		s1 = p[3];
		if (!s1) s1 = "";
		if (!*s1) s1 = "0";
		q1 = dosexp(s1);
		if (!q1) q1 = "";
		if (!*q1) q1 = "0";
		ckstrncpy(buf2,q1,32);
		q1 = buf2;

		r = ckround(atof(q0),(int)(atof(q1)),sxroundbuf,31);
		s2 = sxroundbuf;
		sexprc = 0;
		goto xdosexp;
	    }
        }
        if (x == SX_SET || x == SX_LET || /* Assignment is special */
            x == SX_INC || x == SX_DEC) {
            int rc;
            char c, * m, * s3;
            if (n == 1) {
                s2 = "";
                goto xdosexp;
            }
            s2 = NULL;
            for (i = 1; i < n; i += 2) { /* Loop thru operand pairs */
                rc = 0;
                s3 = p[i+1];
                c = *s3;
                debug(F110,sexpdebug("target p"),s3,0);

                /* Make sure target doesn't have multiple words */
                while (*s3) { if (*s3 < '!') { rc = 1; break; }; s3++; }
                s3 = p[i+1];
                if (rc) {               /* If it does it must have been */
                    char * s4;          /* an SEXP so evaluate it */
                    s3 = dosexp(s3);
                    s4 = s3;
                    rc = 0;
                    while (*s4) { if (*s4 < '!') { rc = 1; break; }; s4++; }
                    if (rc == 0) makestr(&(p[i+1]),s3);
                }

                /* And that it's not a number, etc. */
                if (rc > 0 || isdigit(c) || c == '(') {
                    printf("?Invalid assignment - \"%s\"\n",s);
                    sexprc++;
                    goto xdosexp;
                } else if (isalpha(c)) {
                    rc = xlookup(sexpconsts,s3,nsexpconsts,NULL);
                    if (rc > 0) {
                        printf("?Assignment to constant - \"%s\"\n",s);
                        sexprc++;
                        goto xdosexp;
                    }
                }

                /* If ++ or --, get current value of variable */
                if (x == SX_INC || x == SX_DEC) {
                    int ok = 1;
                    char buf[32];
                    if (c == CMDQ) {    /* A backslash variable */
                        int n = 32;
                        char * s = buf;
                        buf[0] = NUL;
                        if (zzstring(s3,&s,&n) < 0 || !buf[0])
                          ok = 0;
                        s2 = buf;
                    } else {            /* A macro */
                        if ((k = mxlook(mactab,s3,nmac)) < 0)
                          ok = 0;
                        else
                          s2 = mactab[k].mval;
                    }
                    if (!ok) {
                        printf("?Not defined - \"%s\"\n",p[i+1]);
                        sexprc++;
                        goto xdosexp;
                    }
                    if (!s2) s2 = "";
                    k = xxfloat(s2,0);
                    if (k < 1) {
                        printf("?Not numeric - \"%s\"\n",p[i+1]);
                        sexprc++;
                        goto xdosexp;
                    }
                    while (*s2 == '+') s2++;
                    result = ckatofs(s2);
                    fpresult = floatval;
                    if (k > 1 || fpresult != result)
                      fpflag++;
                }
                if (n < i+2) {          /* Variable with no value */
                    s2 = "";
                    if (x == SX_SET || x == SX_LET) {
                        delmac(p[i+1],1); /* Delete the variable */
                        break;
                    } else {
                        s2 = "1";
                    }
                } else {                /* Variable with value */
                    k = xxfloat(p[i+2],0); /* Is it a number? */
                    if (k > 0) {
                        s2 = p[i+2];
                        while (*s2 == '+') s2++;
                    } else {
                        s2 = dosexp(p[i+2]); /* Have value, evaluate it */
                        if (sexprc) goto xdosexp;
                        if (!s2) s2 = "";
                        if (!*s2 && (x == SX_INC || x == SX_DEC))
                          continue;
                    }
                }
                if (x == SX_INC || x == SX_DEC) {
                    k = xxfloat(s2,0);
                    if (k < 1) {
                        printf("?Not numeric - \"%s\"\n",s2);
                        sexprc++;
                        goto xdosexp;
                    }
                    while (*s2 == '+') s2++;
                    j = ckatofs(s2);
                    if (k > 1) {
                        fpj = floatval;
                        fpflag++;
                    } else {
                        fpj = (CKFLOAT)j;
                    }
                    if (x == SX_INC) {
                        result += j;
                        fpresult += fpj;
                    } else if (x == SX_DEC) {
                        result -= j;
                        fpresult -= fpj;
                    }
#ifdef FNFLOAT
                    if (result != fpresult) fpflag++;
#endif	/* FNFLOAT */
                    s2 = (fpflag && !sexptrunc) ?
			fpformat(fpresult,0,0) : ckfstoa(result);
                }
                if (x == SX_LET && cmdlvl > 0) /* LET makes var local */
                  addlocal(p[i+1]);
                if ((rc = addmac(p[i+1],s2)) < 0) { /* Add the value */
                    switch (rc) {
                      case -3: m = "Array not declared"; break;
                      case -2: m = "Subscript out of range"; break;
                      case -4: m = "Out of memory"; break;
                      default: m = "Error creating variable";
                    }
                    printf("?%s - \"%s\"\n",m,s);
                    sexprc++;
                    goto xdosexp;
                }
                if (s2) result = ckatofs(s2);
            }
            goto xdosexp;
        } else if (x == SX_IFC) {               /* Conditional expression */
            int true = 0;
            if (n > 4) {
                printf("?Too many operands: IF - \"%s\"\n",s);
                sexprc++;
                goto xdosexp;
            }
            s2 = dosexp(p[2]);
            if (sexprc) goto xdosexp;
            if (s2) {
                j = ckatofs(s2);
                if (xxfloat(s2,0) == 2) {
                    fpflag++;
                    fpresult = (CKFLOAT)result;
                    fpj = floatval;
                } else {
                    fpj = atof(s2);
                }
                true = ((fpj != 0.0) ? 1 : 0);
            }
            if (!true && n < 4) {
                s2 = NULL;
            } else {
                s2 = dosexp(true ? p[3] : p[4]);
                if (sexprc) goto xdosexp;
                j = s2 ? ckatofs(s2) : 0;
                if (xxfloat(s2,0) == 2) {
                    fpflag++;
                    fpresult = (CKFLOAT)result;
                    fpj = floatval;
                } else {
                    fpj = s2 ? atof(s2) : 0.0;
                }
                fpresult = fpj;
                result = j;
            }
            goto xdosexp;
        } else if (x == SX_QUO) {
#ifndef COMMENT
            int xx;
            xx = strlen(p[2]);
            p[3] = (char *)malloc(xx+4);
            s2 = p[3];
            ckmakmsg(p[3],xx+4,"'(",p[2],")",NULL);
            n++;
#else
            s2 = p[2];
#endif /* COMMENT */
            goto xdosexp;
        } else if (x == SX_STR) {
            int xx;
            s2 = dosexp(p[2]);
            if (sexprc) goto xdosexp;
            xx = strlen(s2);
            p[3] = (char *)malloc(xx+4);
            ckmakmsg(p[3],xx+4,"'(",s2,")",NULL);
            s2 = p[3];
            n++;
            goto xdosexp;
        }
    }
    /* Arithmetic operator or macro - Loop thru operands */

    quit = 0;                           /* Short-circuit flag. */
    if (macro && n > 1) {               /* If operator is a macro */
        if (!line) {                    /* allocate local buffer for */
            line = (char *)malloc(SXMLEN); /* the evaluated argument list. */
            if (!line) {
                printf("?Memory allocation failure - \"%s\"\n",p[1]);
                sexprc++;
                goto xdosexp;
            }
            linelen = SXMLEN;
            /* debug(F101,"dosexp macro arg buffer","",linelen); */
        }
        linepos = 0;
        line[linepos] = NUL;
    }
    for (i = 1; ((i < n) && !sexprc && !quit); i++) { /* Loop thru operands */
        quote = 0;
        s2 = p[i+1];                    /* Get operand */
        if (!s2) s2 = "";

#ifdef COMMENT
        if (*s2 == '\047') {            /* Is it quoted? */
            debug(F110,sexpdebug("'B"),s2,0);
            s2++;                       /* Space past the quote */
            quote++;
            if (*s2 == '(') {           /* Quoted S-Expression? */
                char c4, * s4 = s2+1;   /* Strip outer parens */
                while ((c4 = *s4++)) {
                    if (c4 == ')' && !*s4) {
                        s2++;
                        *(s4-1) = NUL;
                        break;
                    }
                }
            }
            debug(F110,sexpdebug("'C"),s2,0);

        } else {                        /* Not quoted */
            s2 = dosexp(p[i+1]);        /* evaluate it */
            if (sexprc) goto xdosexp;
            if (!s2) s2 = "";
            if (!macro && x == SX_EVA)
              continue;
        }
#else
        if (*s2 != '\047') {            /* Is it quoted? */
            s2 = dosexp(p[i+1]);        /* No, evaluate it */
            if (sexprc) goto xdosexp;
            if (!s2) s2 = "";
            if (!macro && x == SX_EVA)
              continue;
        }
        if (*s2 == '\047') {            /* Is result quoted? */
            debug(F110,sexpdebug("'B"),s2,0);
            s2++;                       /* Space past the quote */
            quote++;
            if (*s2 == '(') {           /* Quoted S-Expression? */
                char c4, * s4 = s2+1;   /* Strip outer parens */
                while ((c4 = *s4++)) {
                    if (c4 == ')' && !*s4) {
                        s2++;
                        *(s4-1) = NUL;
                        break;
                    }
                }
            }
            debug(F110,sexpdebug("'C"),s2,0);
        }
#endif /* COMMENT */
        if (macro) {
            debug(F111,sexpdebug("macro arg"),s2,i);
            if (!*s2) quote++;
            if (!quote) {
                register char c4, * s4 = s2;
                while ((c4 = *s4++)) if (c4 == SP) { quote++; break; }
            }
            if (quote) line[linepos++] = '{';
            while ((line[linepos++] = *s2++)) {
                if (linepos > linelen - 3) {
                    char * tmp = NULL;
                    line[linepos] = NUL;
                    linelen += SXMLEN;
                    tmp = (char *) malloc(linelen);
                    if (!tmp) {
                        printf("?Memory re-allocation failure - \"%s...\"\n",
                               line);
                        sexprc++;
                        goto xdosexp;
                    }
                    strcpy(tmp,line);
                    free(line);
                    line = tmp;
                }
            }
            linepos--;                  /* Back up over NUL */
            if (quote)
              line[linepos++] = '}';    /* End quote group */
            line[linepos++] = SP;       /* add a space */
            line[linepos] = NUL;        /* and a NUL */
            continue;
        }
        if (!quote) {                   /* Built-in operator... */
            s2 = dosexp(s2);
            if (sexprc) goto xdosexp;
            if (!s2) s2 = "";
        }
        if (x == SX_EVA)
          continue;

        if (!*s2) {
            /* An empty value is not a legal number */
            /* but it is a legal truth value */
            if (x != SX_AND && x != SX_LOR && x != SX_NOT) {
                printf("?Not Numeric - \"%s\"\n",p[i+1]);
                sexprc++;
                goto xdosexp;
            }
            j = 0;
            fpj = 0.0;
        } else {
            j = ckatofs(s2);
            /* Switch to floating-point upon encountering any f.p. arg */
            /* OR... if integer is too big */
            if (!fpflag) if (xxfloat(s2,0) == 2)
              fpflag++;
            fpj = atof(s2);
        }
        if (i == 1) {                   /* Initial result is first operand */
            result = (n == 2 && x == SX_SUB) ? 0-j : j;
            fpresult = (n == 2 && x == SX_SUB) ? -fpj : fpj;
	    if ((x == SX_AND && result == 0) ||	/* Short circuit */
		(x == SX_LOR && result != 0))
	      quit++;
            if (!(kwflags & SXF_ONE))	/* Command w/single arg */
              continue;
        }
        if (x == SX_MOD || x == SX_DIV) {
            if (!result)
              fpflag++;
            if (!fpj) {
                printf("?Divide by zero - \"%s\"\n",cmdbuf);
                sexprc++;
                goto xdosexp;
            }
        }
        switch (x) {			/* Accumulate result */

          case SX_EVA:                  /* EVAL */
            result = j;
            fpresult = fpj;
            break;

          case SX_ADD:                  /* + */
            result += j;
            fpresult += fpj;
#ifdef FNFLOAT
            if (result != fpresult)
              fpflag++;
#endif	/* FNFLOAT */
            break;

          case SX_SUB:                  /* - */
            result -= j;
            fpresult -= fpj;
#ifdef FNFLOAT
            if (result != fpresult)
              fpflag++;
#endif	/* FNFLOAT */
            break;

          case SX_MUL:                  /* * */
            result *= j;
            fpresult *= fpj;
#ifdef FNFLOAT
            if (result != fpresult)
              fpflag++;
#endif	/* FNFLOAT */
            break;

          case SX_AND:                  /* AND */
            result = result && j;
            if (!result) quit++;
            fpresult = fpresult && fpj;
            break;

          case SX_LOR:                  /* OR */
            result = result || j;
            if (!result) quit++;
            fpresult = fpresult || fpj;
            break;

          case SX_MOD:                  /* Modulus */
            result = result % j;
#ifdef FNFLOAT
            fpresult = (CKFLOAT)fmod(fpresult,fpj);
            if (result != fpresult)
              fpflag++;
#else
            fpresult = result;
#endif /* FNFLOAT */
            break;

          case SX_DIV:                  /* / */
	    if (j) {
		result /= j;
		fpresult /= fpj;
#ifdef FNFLOAT
		if (result != fpresult)
		  fpflag++;
#endif	/* FNFLOAT */
	    } else {
		fpresult /= fpj;
		result = fpj;
#ifdef FNFLOAT
		  fpflag++;
#endif	/* FNFLOAT */
	    }
            break;

          case SX_AEQ:                  /* Test for equality */
            if (fpflag) {
                if (fpresult != fpj)
                  presult = 0;
            } else {
                if (result != j)
                  presult = 0;
            }
            break;

          case SX_NEQ:                  /* Test for ineqality */
            if (fpflag) {
                if (fpresult == fpj)
                  presult = 0;
            } else {
                if (result == j)
                  presult = 0;
            }
            break;

          case SX_ALE:                  /* Arithmetic less-equal */
            if (fpflag) {
                if (fpj < fpresult)
                  presult = 0;
                fpresult = fpj;
            } else {
                if (j < result)
                  presult = 0;
                result = j;
            }
            break;

          case SX_ALT:                  /* Arithmetic less-than */
            if (fpflag) {
                if (fpj <= fpresult)
                  presult = 0;
                fpresult = fpj;
            } else {
                if (j <= result)
                  presult = 0;
                result = j;
            }
            break;

          case SX_AGT:                  /* Arithmetic greater-than */
            if (fpflag) {
                if (fpj >= fpresult)
                  presult = 0;
                fpresult = fpj;
            } else {
                if (j >= result)
                  presult = 0;
                result = j;
            }
            break;

          case SX_AGE:                  /* Arithmetic greater-equal */
            if (fpflag) {
                if (fpj > fpresult)
                  presult = 0;
                fpresult = fpj;
            } else {
                if (j > result)
                  presult = 0;
                result = j;
            }
            break;

          case SX_POW:                  /* Raise to power */
#ifdef FNFLOAT
            {
                double dummy;
                if (!fpj) {
                    fpresult = 1.0;
                } else if ((!fpresult && fpj <= 0.0)) {
                    printf("?Divide by zero - \"%s\"\n",cmdbuf);
                    sexprc++;
                    goto xdosexp;
                } else if (fpresult < 0.0 && modf(fpj,&dummy)) {
                    printf("?Domain error - \"%s\"\n",cmdbuf);
                    sexprc++;
                    goto xdosexp;
                } else {
                    fpresult = (CKFLOAT)pow(fpresult,fpj);
                }
            }
#endif /* FNFLOAT */
            if (j == 0) {
                result = 1;
            } else {
                CK_OFF_T z, sign = 0;
                if (j < 0) {
                    if (result == 0) {
                        printf("?Divide by zero - \"%s\"\n",cmdbuf);
                        sexprc++;
                        goto xdosexp;
                    }
                    j = 0 - j;
                    sign++;
                }
                z = result;
                while (--j > 0)
                  result *= z;
                if (sign)
                  result = 1 / result;
            }
#ifdef FNFLOAT
            if (result != fpresult)
              fpflag++;
#endif	/* FNFLOAT */
            break;

#ifdef FNFLOAT
          case SX_EXP:                  /* e to the given power */
            fpresult = (CKFLOAT) exp(fpj);
            break;

          case SX_LGN:                  /* Natural log */
          case SX_LGX:                  /* Log base 10 */
          case SX_SQR:                  /* Square root */
            if (fpj < 0.0) {
                printf("?Argument out of range - \"%s\"\n",cmdbuf);
                sexprc++;
                goto xdosexp;
            }
            if (x == SX_SQR)
              fpresult = (CKFLOAT) sqrt(fpj);
            else if (x == SX_LGN)
              fpresult = (CKFLOAT) log(fpj);
            else
              fpresult = (CKFLOAT) log10(fpj);
            break;

          case SX_SIN:                  /* sine */
            fpresult = (CKFLOAT) sin(fpj);
            break;

          case SX_COS:                  /* cosine */
            fpresult = (CKFLOAT) cos(fpj);
            break;

          case SX_TAN:                  /* tangent */
            fpresult = (CKFLOAT) tan(fpj);
            break;
#endif /* FNFLOAT */

          case SX_CEI:                  /* Ceiling */
            if (j != fpj)
              if (fpj > 0.0)
                fpj += 1.0;
            fpresult = fpj;
            fpflag = 1;
            truncate = 1;
            break;

          case SX_FLR:                  /* Floor */
            if (j != fpj)
              if (fpj < 0.0)
                fpj -= 1.0;
            fpresult = fpj;
            fpflag = 1;
            truncate = 1;
            break;

          case SX_TRU:                  /* Truncate */
            fpresult = fpj;
            fpflag = 1;
            truncate = 1;
            break;

          case SX_ABS:                  /* Absolute value */
            result = (j < 0) ? 0 - j : j;
#ifdef FNFLOAT
            fpresult = (fpj < 0.0) ? 0.0 - fpj : fpj;
            if (result != fpresult)
              fpflag++;
#endif	/* FNFLOAT */
            break;

          case SX_MAX:                  /* Max */
            if (j != fpj)
              fpflag++;
            if (fpflag) {
                if (fpj > fpresult)
                  fpresult = fpj;
            } else
              if (j > result)
                result = j;
            break;

          case SX_MIN:                  /* Min */
            if (j != fpj)
              fpflag++;
            if (fpflag) {
                if (fpj < fpresult)
                  fpresult = fpj;
            } else
              if (j < result)
                result = j;
            break;

          case SX_FLO:                  /* Float */
            fpflag++;
            fpresult = result;
            fpj = j;
            break;

          case SX_NOT:                  /* NOT (reverse truth value) */
            fpflag = 0;
            not++;
            break;

          case SX_BWA:                  /* Bitwise AND */
            fpflag = 0;
            result &= j;
            break;

          case SX_BWO:                  /* Bitwise OR */
            fpflag = 0;
            result |= j;
            break;

          case SX_BWX:                  /* Bitwise XOR */
          case SX_XOR:                  /* Logical XOR */
            if (n > 3) {
                printf("?Too many operands - \"%s\"\n",s);
                sexprc++;
                goto xdosexp;
            }
            fpflag = 0;
            if (x == SX_BWX) {
                result ^= j;
            } else {
                result = (result && !j) || (!result && j);
                if (result) result = 1;
            }
            break;

          case SX_BWN:                  /* Bitwise Not */
            fpflag = 0;
            result = ~result;
            break;

          default:
            printf("BAD OP [%s]\n",p[1]);
            sexprc++;
        }
    }
    if (!pflag)                         /* Not a predicate */
      sexppv = -1;                      /* So unset this */

  /* domacro: */

    if (macro) {                        /* User-defined macro */
        extern int fsexpflag;           /* (see fneval():ckuus4.c) */
        int lookagain = 0;              /* Maybe the macro table changed */
        if (mactab[mx].kwd) {           /* Check and see */
            if (ckstrcmp(mactab[mx].kwd,p[1],-1,0))
              lookagain++;
        } else
          lookagain++;
        if (lookagain) {                /* The table changed */
            mx = mxlook(mactab,p[1],nmac); /* Get the macro's new index */
            debug(F111,sexpdebug("macro moved"),p[1],mx);
            if (mx < 0) {                  /* Yikes! */
                printf("?Macro disappeared! - \"%s\"\n",p[1]);
                sexprc++;
                goto xdosexp;
            }
        }
        debug(F111,sexpdebug("macro mx"),mactab[mx].kwd,mx);
        if (fsexpflag) {                /* If embedded in a function call */
            if (cmpush() > -1) {        /* get a new copy of the parsing */
                extern int ifc;         /* environment, */
                int k, ifcsav = ifc;    /* save the IF state */
                dodo(mx,line,0);        /* Set up the macro */
                k = parser(1);          /* Call the parser to execute it */
                cmpop();                /* Pop back to previous level */
                ifc = ifcsav;           /* restore IF state */
                if (k == 0)             /* If no error */
                  s2 = mrval[maclvl+1]; /* get return value, if any */
                if (!s2) s2 = "";
                debug(F110,sexpdebug("macro return"),s2,0);
            } else {
                printf("?Resources exhausted - \"%s\"\n",s);
                sexprc++;
            }
        } else {                        /* Not embedded in a function call */
            dodo(mx,line,0);            /* As above but without cmpush/pop() */
            k = parser(1);
            if (k == 0)
              s2 = mrval[maclvl+1];
            if (!s2) s2 = "";
        }
    } else if (pflag) {                 /* Predicate */
        if (not) presult = presult ? 0 : 1;
        sexppv = presult;               /* So set predicate value (0 or 1) */
        s2 = presult ? "1" : "0";
    } else if (fpflag && !sexptrunc) {	/* Result is floating-point */
        if (not) fpresult = fpresult ? 0.0 : 1.0;
        s2 = fpformat(fpresult,0,0);
    } else if (x != SX_EVA) {
        if (not) result = result ? 0 : 1;
        s2 = ckfstoa(result);
    }

/* Common exit point.  Always come here to exit. */

  xdosexp:

    if (!s2) s2 = "";
    if (!sexprc && s2) {                /* Have a result */
        char * sx;
        char * q2 = s2; int xx = 0;
        if (*s2) {
            while (*q2++) xx++;         /* Get length */
            if (xx > sexprmax)          /* (stats) */
              sexprmax = xx;
        } else
          xx = 0;
        if (xx > sxrlen[sexpdep] || !sxresult[sexpdep]) {
            int k;
            k = xx + xx / 4;
            if (k < 32) k = 32;
            if (sxresult[sexpdep])
              free(sxresult[sexpdep]);
            if ((sxresult[sexpdep] = (char *)malloc(k))) {
                sxrlen[sexpdep] = k;
            } else {
                printf("?Memory allocation failure - \"%s\"\n",s2);
                sexprc++;
            }
        }
        sx = sxresult[sexpdep];         /* Point to result buffer */
        while ((*sx++ = *s2++)) ;       /* copy result. */
        if (fpflag && truncate) {       /* Floating point + truncate */
            sx = sxresult[sexpdep];     /* at decimal point */
            for (i = xx - 1; i >= 0; i--) {
                if (sx[i] == '.') {
                    sx[i] = NUL;
                    if (i == 0) {       /* If nothing left */
                        sx[0] = '0';    /* put a zero. */
                        sx[1] = NUL;
                    }
                }
            }
        }
    }
    if (line)                           /* If macro arg buffer allocated */
      free(line);                       /* free it. */
    if (mustfree) {                     /* And free local copy of split list */
        for (i = 1; i <= n; i++) {
            if (p[i]) free(p[i]);
        }
    }
    debug(F111,sexpdebug("exit"),sxresult[sexpdep],sexprc);
    return(sxresult[sexpdep--]);
}
#endif /* NOSEXP */
#endif /* NOSPL */

int                                     /* CHECK command */
dochk() {
    int x, y;
    if ((y = cmkey(ftrtab,nftr,"","",xxstring)) < 0)
      return(y);
    ckstrncpy(line,atmbuf,LINBUFSIZ);
    if ((y = cmcfm()) < 0)
      return(y);
#ifndef NOPUSH
    if (!ckstrcmp(line,"push",(int)strlen(line),0)) {
        if (msgflg)                     /* If at top level... */
          printf(" push%s available\n", nopush ? " not" : "");
        else if (nopush && !backgrd)
          printf(" CHECK: push not available\n");
        return(success = 1 - nopush);
    }
#endif /* NOPUSH */
#ifdef PIPESEND
    if (!ckstrcmp(line,"pipes",(int)strlen(line),0)) {
        if (msgflg)                     /* If at top level... */
          printf(" pipes%s available\n",
                 (nopush || protocol != PROTO_K) ? " not" : "");
        else if ((nopush || protocol != PROTO_K) && !backgrd)
          printf(" CHECK: pipes not available\n");
        return(success = 1 - nopush);
    }
#endif /* PIPESEND */
    y = lookup(ftrtab,line,nftr,&x);    /* Look it up */
    debug(F111,"dochk",ftrtab[x].kwd,y);
    if (msgflg)                         /* If at top level... */
      printf(" %s%s available\n", ftrtab[x].kwd, y ? " not" : "");
    else if (y && !backgrd)
      printf(" CHECK: %s not available\n", ftrtab[x].kwd);
    return(success = 1 - y);
}

#ifndef NOLOCAL
#ifdef CKLOGDIAL

/* Connection log and elapsed-time reporting */

extern char cxlogbuf[];                 /* Log record buffer */
extern char diafil[];                   /* Log file name */
extern int dialog, cx_active;           /* Flags */
static long cx_prev = 0L;               /* Elapsed time of previous session */

#endif /* CKLOGDIAL */
#endif /* NOLOCAL */

VOID
dologend() {                            /* Write record to connection log */
#ifdef LOCUS
    extern int locus, autolocus;
#endif /* LOCUS */

#ifndef NOLOCAL
#ifdef CKLOGDIAL
    long d1, d2, t1, t2;
    char buf[32], * p;
#endif /* CKLOGDIAL */
#endif /* NOLOCAL */

#ifdef LOCUS
    if (autolocus) {
        int x = locus;
#ifdef NEWFTP
	debug(F101,"dologend ftpisconnected","",ftpisconnected());
        setlocus(ftpisconnected() ? 0 : 1, 1);
#else
        setlocus(1,1);
#endif /* NEWFTP */
    }
#endif /* LOCUS */

#ifndef NOLOCAL
#ifdef CKLOGDIAL
    debug(F101,"dologend dialog","",dialog);
    debug(F101,"dologend cxlogbuf[0]","",cxlogbuf[0]);
#ifdef CKSYSLOG
    debug(F101,"dologend ckxlogging","",ckxlogging);
#endif /* CKSYSLOG */

    if (!cx_active || !cxlogbuf[0])     /* No active record */
      return;

    cx_active = 0;                      /* Record is not active */
    debug(F111,"dologend cxlogbuf 1",cxlogbuf,cx_active);

    d1 = mjd((char *)cxlogbuf);         /* Get start date of this session */
    ckstrncpy(buf,ckdate(),31);         /* Get current date */
    d2 = mjd(buf);                      /* Convert them to mjds */
    p = cxlogbuf;                       /* Get start time */
    p[11] = NUL;
    p[14] = NUL;                        /* Convert to seconds */
    t1 = atol(p+9) * 3600L + atol(p+12) * 60L + atol(p+15);
    p[11] = ':';
    p[14] = ':';
    p = buf;                            /* Get end time */
    p[11] = NUL;
    p[14] = NUL;
    t2 = atol(p+9) * 3600L + atol(p+12) * 60L + atol(p+15);
    t2 = ((d2 - d1) * 86400L) + (t2 - t1); /* Compute elapsed time */
    debug(F101,"dologend t2","",t2);
    if (t2 > -1L) {
        cx_prev = t2;
        p = hhmmss(t2);
        debug(F110,"dologend hhmmss",p,0);
        ckstrncat(cxlogbuf,"E=",CXLOGBUFL); /* Append to log record */
        ckstrncat(cxlogbuf,p,CXLOGBUFL);
        debug(F110,"dologend cxlogbuf 2",cxlogbuf,0);
    } else
      cx_prev = 0L;
    debug(F101,"dologend cx_prev","",cx_prev);
    if (dialog) {                       /* If logging */
        int x;
        x = diaopn(diafil,1,1);         /* Open log in append mode */
        debug(F101,"dologend diaopn","",x);
        x = zsoutl(ZDIFIL,cxlogbuf);    /* Write the record */
        debug(F101,"dologend zsoutl","",x);
        x = zclose(ZDIFIL);             /* Close the log */
        debug(F101,"dologend zclose","",x);
    }
#ifdef CKSYSLOG
    debug(F101,"dologend ckxlogging","",ckxlogging);
    if (ckxlogging) {
        int x;
        x = ckindex("T=DIAL",cxlogbuf,0,0,1);
        debug(F111,"dologend ckxsyslog",cxlogbuf,ckxsyslog);
        debug(F111,"dologend ckindex","T=DIAL",x);
        if (x > 0) {
            if (ckxsyslog >= SYSLG_DI) {
                debug(F110,"dologend syslog",cxlogbuf+18,0);
                cksyslog(SYSLG_DI,1,"CONNECTION",(char *)(cxlogbuf+18),"");
            } else if (ckxsyslog >= SYSLG_AC) {
                debug(F110,"dologend syslog",cxlogbuf+18,0);
                cksyslog(SYSLG_AC,1,"CONNECTION",(char *)(cxlogbuf+18),"");
            }
        }
    }
#endif /* CKSYSLOG */
#endif /* CKLOGDIAL */
#endif /* NOLOCAL */
}

#ifndef NOLOCAL
#ifdef CKLOGDIAL

/*  D O L O G S H O W  --  Show session/connection info  */

/* Call with fc == 1 to show, fc == 0 to only calculate. */
/* Returns session elapsed time in seconds. */
/* If no session active, returns elapsed time of previous session, if any, */
/* otherwise 0 */

long
dologshow(fc) int fc; {                 /* SHOW (current) CONNECTION */
    long d1, d2, t1, t2 = 0, prev;
    char c, buf1[32], buf2[32], * info[32], * p, * s;
    char * xlogbuf, xbuf[CXLOGBUFL+1];
    int i, x = 0, z, ftp = 0, active = 0;

#ifdef NEWFTP
    extern char ftplogbuf[];
    extern long ftplogprev;
    extern int ftplogactive;
    if (fc & W_FTP) {
        fc &= 63;
        ftp = 1;
        xlogbuf = ftplogbuf;
        prev = ftplogprev;
        active = ftplogactive;
    } else {
#endif /* NEWFTP */
        ftp = 0;
        xlogbuf = cxlogbuf;
        prev = cx_prev;
        active = cx_active;
#ifdef NEWFTP
    }
#endif /* NEWFTP */

    debug(F101,"dologshow local","",local);
    debug(F101,"dologshow ftp","",ftp);
    debug(F111,"dologshow active",xlogbuf,active);

    if (!xlogbuf[0]) {
        if (fc) {
            if (didsetlin || ftp)
              printf(" %s: No record.\n", ftp ? "FTP" : "Kermit");
            else
              printf(" %s: No connection.\n", ftp ? "FTP" : "Kermit");
        }
        return(prev);
    }

#ifdef NEWFTP
    if (ftp) {
        z = ftpisconnected() ? 1 : -1;
    } else {
#endif /* NEWFTP */
        if (local) {                    /* See if we have an open connection */
            z = ttchk();
            debug(F101,"dologshow ttchk","",z);
            z = (z > -1) ? 1 : -2;
        } else {
            z = active ? 1 : -2;
        }
#ifdef NEWFTP
    }
#endif /* NEWFTP */
    if (z < 0L) {
        if (!fc)
          return(prev);
        else
          t2 = prev;
    }
    /* Note: NOT ckstrncpy! */
    strncpy(buf1,xlogbuf,17);           /* Copy of just the timestamp */
    buf1[17] = NUL;                     /* Terminate it */
    ckstrncpy(xbuf,xlogbuf+18,CXLOGBUFL); /* Copy that can be poked */
    debug(F111,"dologshow prev",xbuf,prev);

    xwords(xbuf,31,info,1);             /* Break up into fields */
    d1 = mjd(buf1);                     /* Convert start time to MJD */
    ckstrncpy(buf2,ckdate(),31);        /* Current date */
    d2 = mjd(buf2);                     /* Convert to MJD */
    p = buf1;                           /* Point to start time */
    p[11] = NUL;
    p[14] = NUL;                        /* Convert to seconds */
    t1 = atol(p+9) * 3600L + atol(p+12) * 60L + atol(p+15);
    p[11] = ':';
    p[14] = ':';
    p = buf2;                           /* Ditto for current time */
    p[11] = NUL;
    p[14] = NUL;
    if (z > -1L) {
        t2 = atol(p+9) * 3600L + atol(p+12) * 60L + atol(p+15);
        t2 = ((d2 - d1) * 86400L) + (t2 - t1); /* Elapsed time so far */
    }
    if (fc) {
        p = NULL;
        if (t2 > -1L)                   /* Convert seconds to hh:mm:ss */
          p = hhmmss(t2);
        if (z > -1)
          s = "Active";
        else if (z == -2)
          s = "Closed";
        else
          s = "Unknown";
        printf("\n");                   /* Show results */
        printf(" Status:       %s\n",s);
        printf(" Opened:       %s\n",buf1);
        printf(" User:         %s\n",info[1] ? info[1] : "");
        printf(" PID:          %s\n",info[2] ? info[2] : "");
        for (i = 3; info[i]; i++) {
            c = info[i][0];
            s = (info[i]) ? info[i]+2 : "";
            switch (c) {
              case 'T': printf(" Type:         %s\n", s); break;
              case 'N': printf(" To:           %s\n", s); break;
              case 'P': printf(" Port:         %s\n", s); break;
              case 'H': printf(" From:         %s\n", s); break;
              case 'D': printf(" Device:       %s\n", s); break;
              case 'O': printf(" Origin:       %s\n", s); break;
              case 'E': break;
              default:  printf(" %s\n",info[i] ? info[i] : "");
            }
        }
        if (z < 0L)
          printf(" Elapsed time: %s\n", hhmmss(t2));
        else
          printf(" Elapsed time: %s\n", p ? p : "(unknown)");
        x = 0;
#ifdef NETCONN
#ifdef SSHBUILTIN
        if ( IS_SSH() ) x++;
#endif /* SSHBUILTIN */
#ifdef CK_ENCRYPTION
        if (ck_tn_encrypting() && ck_tn_decrypting()) x++;
#endif /* CK_ENCRYPTION */
#ifdef CK_SSL
        if (tls_active_flag || ssl_active_flag) x++;
#endif /* CK_SSL */
#ifdef RLOGCODE
#ifdef CK_KERBEROS
#ifdef CK_ENCRYPTION
        if (ttnproto == NP_EK4LOGIN || ttnproto == NP_EK5LOGIN) x++;
#endif /* CK_ENCRYPTION */
#endif /* CK_KERBEROS */
#endif /* RLOGCODE */
#endif /* NETCONN */
        if (z > 0)
          printf(" Encrypted:    %s\n", x ? "Yes" : "No");
        printf(" Log:          %s\n", dialog ? diafil : "(none)");
        printf("\n");
    }
    return(t2 > -1L ? t2 : 0L);
}

VOID
dologline() {
    char * p;
    int n, m = 0;

    dologend();                         /* Previous session not closed out? */
    cx_active = 1;                      /* Record is active */
    cx_prev = 0L;
    p = ckdate();                       /* Get timestamp */
    n = ckstrncpy(cxlogbuf,p,CXLOGBUFL-1); /* Start record off with it */
    if (!uidbuf[0]) {
        debug(F100,"dologline uidbuf empty","",0);
#ifdef UNIX                             /* Who has whoami()... */
        ckstrncpy(uidbuf,(char *)whoami(),UIDBUFLEN);
#else
#ifdef STRATUS
        ckstrncpy(uidbuf,(char *)whoami(),UIDBUFLEN);
#else
        ckstrncpy(uidbuf,"UNKNOWN",UIDBUFLEN);
#endif /* STRATUS */
#endif /* UNIX */
    }
    m = strlen(uidbuf) + strlen(myhost) + strlen(ttname) + 32;
    if (n+m < CXLOGBUFL-1) {            /* Add serial device info */
        p = cxlogbuf+n;
        sprintf(p," %s %s T=SERIAL H=%s D=%s ", /* SAFE */
                uidbuf,
                ckgetpid(),
                myhost,
                ttname
                );
    } else
      ckstrncpy(cxlogbuf,"LOGLINE BUFFER OVERFLOW",CXLOGBUFL);
    debug(F110,"dologline",cxlogbuf,0);
}

#ifdef NETCONN
VOID
dolognet() {
    char * p, * s = "NET", * uu = uidbuf;
    char * port = "";
    int n, m, tcp = 0;
    char * h = NULL;

    dologend();                         /* Previous session not closed out? */
    cx_prev = 0L;
    cx_active = 1;                      /* Record is active */
    p = ckdate();
    n = ckstrncpy(cxlogbuf,p,CXLOGBUFL);
#ifdef TCPSOCKET
    if (nettype == NET_TCPB || nettype == NET_TCPA) {
	tcp++;
	s = "TCP";
    } else if (nettype == NET_SSH) {
	s = "SSH";
	tcp++;
    }
#endif /* TCPSOCKET */
#ifdef ANYX25
    if (nettype == NET_SX25 || nettype == NET_VX25 || nettype == NET_IX25)
      s = "X25";
#endif /* ANYX25 */
#ifdef DECNET
    if (nettype == NET_DEC)
      s = "DECNET";
#endif /* DECNET */
#ifdef SUPERLAT
    if (nettype == NET_SLAT)
      s = "SUPERLAT";
#endif /* SUPERLAT */
#ifdef CK_NETBIOS
    if (nettype == NET_BIOS)
      s = "NETBIOS";
#endif /* CK_NETBIOS */

    if (!uu[0]) {
        debug(F100,"dolognet uidbuf empty","",0);
#ifdef OS2ORUNIX                        /* Who has whoami()... */
        uu = (char *)whoami();
#else
#ifdef STRATUS
        uu = (char *)whoami();
#else
        uu = "UNKNOWN";
#endif /* STRATUS */
#endif /* UNIX */
    }
#ifdef TCPSOCKET
    if (tcp) {
	int k;
	makestr(&h,myhost);
	if ((k = ckindex(":",h,0,0,0)) > 0) {
	    h[k-1] = NUL;
	    port = &h[k];
	} else {
	    int svcnum = gettcpport();
	    if (svcnum > 0)
	      port = ckitoa(svcnum);
	    else
	      port = "unk";
	}
    }
#endif	/* TCPSOCKET */
    m = strlen(uu) + strlen(myhost) + strlen(ttname) + strlen(s) + 32;
    if (n+m < CXLOGBUFL-1) {            /* SAFE */
        p = cxlogbuf+n;
        sprintf(p," %s %s T=%s N=%s H=%s P=%s ",
                uu,
                ckgetpid(),
                s,
                ttname,
                myhost,
		port
                );
    } else
      ckstrncpy(cxlogbuf,"LOGNET BUFFER OVERFLOW",CXLOGBUFL);
    debug(F110,"dolognet cxlogbuf",cxlogbuf,0);
    if (h) makestr(&h,NULL);
}
#endif /* NETCONN */
#endif /* CKLOGDIAL */

#ifndef NODIAL
/*
  Parse a DIAL-related string, stripping enclosing braces, if any.
*/
static int
dialstr(p,msg) char **p; char *msg; {
    int x;
    char *s;

    if ((x = cmtxt(msg, "", &s, xxstring)) < 0)
      return(x);
    s = brstrip(s);                     /* Strip braces around. */
    debug(F110,"dialstr",s,0);
    makestr(p,*s?s:NULL);
    return(success = 1);
}

VOID
initmdm(x) int x; {
    MDMINF * p;
    int m;

    mdmtyp = x;                         /* Set global modem type */
    debug(F101,"initmdm mdmtyp","",mdmtyp);
    debug(F101,"initmdm usermdm","",usermdm);
    if (x < 1) return;

    m = usermdm ? usermdm : mdmtyp;

    p = modemp[m];                      /* Point to modem info struct, and */
    /* debug(F101,"initmdm p","",p); */
    if (p) {
        dialec = p->capas & CKD_EC;     /* set DIAL ERROR-CORRECTION, */
        dialdc = p->capas & CKD_DC;     /* DIAL DATA-COMPRESSION, and */
        mdmspd = p->capas & CKD_SB ? 0 : 1; /* DIAL SPEED-MATCHING from it. */
        dialfc = FLO_AUTO;                  /* Modem's local flow control.. */
        dialmax   = p->max_speed;
        dialcapas = p->capas;
        dialesc   = p->esc_char;
    } else if (mdmtyp > 0) {
        printf("WARNING: modem info for \"%s\" not filled in yet\n",
               gmdmtyp()
               );
    }

/* Reset or set the SET DIAL STRING items ... */

#ifdef DEBUG
    if (deblog) {
        debug(F110,"initmdm dialini",dialini,0);
        debug(F110,"initmdm dialmstr ",dialmstr,0);
        debug(F110,"initmdm dialmprmt",dialmprmt,0);
        debug(F110,"initmdm dialcmd",dialcmd,0);
        debug(F110,"initmdm dialdcon",dialdcon,0);
        debug(F110,"initmdm dialdcoff",dialdcoff,0);
        debug(F110,"initmdm dialecon",dialecon,0);
        debug(F110,"initmdm dialecoff",dialecoff,0);
        debug(F110,"initmdm dialhcmd",dialhcmd,0);
        debug(F110,"initmdm dialhwfc",dialhwfc,0);
        debug(F110,"initmdm dialswfc",dialswfc,0);
        debug(F110,"initmdm dialnofc",dialnofc,0);
        debug(F110,"initmdm dialtone",dialtone,0);
        debug(F110,"initmdm dialpulse",dialpulse,0);
        debug(F110,"initmdm dialname",dialname,0);
        debug(F110,"initmdm dialaaon",dialaaon,0);
        debug(F110,"initmdm dialaaoff",dialaaoff,0);
        debug(F110,"initmdm dialx3",dialx3,0);
        debug(F110,"initmdm dialspon",dialspon,0);
        debug(F110,"initmdm dialspoff",dialspoff,0);
        debug(F110,"initmdm dialvol1",dialvol1,0);
        debug(F110,"initmdm dialvol2",dialvol2,0);
        debug(F110,"initmdm dialvol3",dialvol3,0);
        debug(F110,"initmdm dialini2",dialini2,0);
    }
#endif /* DEBUG */

    if (usermdm && p) { /* USER-DEFINED: copy info from specified template */

        makestr(&dialini  ,p->wake_str);
        makestr(&dialmstr ,p->dmode_str);
        makestr(&dialmprmt,p->dmode_prompt);
        makestr(&dialcmd  ,p->dial_str);
        makestr(&dialdcon ,p->dc_on_str);
        makestr(&dialdcoff,p->dc_off_str);
        makestr(&dialecon ,p->ec_on_str);
        makestr(&dialecoff,p->ec_off_str);
        makestr(&dialhcmd ,p->hup_str);
        makestr(&dialhwfc ,p->hwfc_str);
        makestr(&dialswfc ,p->swfc_str);
        makestr(&dialnofc ,p->nofc_str);
        makestr(&dialtone ,p->tone);
        makestr(&dialpulse,p->pulse);
        makestr(&dialname ,"This space available (use SET MODEM NAME)");
        makestr(&dialaaon ,p->aa_on_str);
        makestr(&dialaaoff,p->aa_off_str);
        makestr(&dialx3   ,p->ignoredt);
        makestr(&dialspon ,p->sp_on_str);
        makestr(&dialspoff,p->sp_off_str);
        makestr(&dialvol1 ,p->vol1_str);
        makestr(&dialvol2 ,p->vol2_str);
        makestr(&dialvol3 ,p->vol3_str);
        makestr(&dialini2 ,p->ini2);

    } else {                    /* Not user-defined, so wipe out overrides */

        if (dialini)   makestr(&dialini,NULL);   /* Init-string */
        if (dialmstr)  makestr(&dialmstr,NULL);  /* Dial-mode-str */
        if (dialmprmt) makestr(&dialmprmt,NULL); /* Dial-mode-pro */
        if (dialcmd)   makestr(&dialcmd,NULL);   /* Dial-command  */
        if (dialdcon)  makestr(&dialdcon,NULL);  /* DC ON command */
        if (dialdcoff) makestr(&dialdcoff,NULL); /* DC OFF command */
        if (dialecon)  makestr(&dialecon,NULL);  /* EC ON command */
        if (dialecoff) makestr(&dialecoff,NULL); /* EC OFF command */
        if (dialhcmd)  makestr(&dialhcmd,NULL);  /* Hangup command */
        if (dialhwfc)  makestr(&dialhwfc,NULL);  /* Flow control... */
        if (dialswfc)  makestr(&dialswfc,NULL);  /*  */
        if (dialnofc)  makestr(&dialnofc,NULL);  /*  */
        if (dialtone)  makestr(&dialtone,NULL);  /* Dialing method */
        if (dialpulse) makestr(&dialpulse,NULL); /*  */
        if (dialname)  makestr(&dialname,NULL);  /* Modem name */
        if (dialaaon)  makestr(&dialaaon,NULL);  /* Autoanswer On */
        if (dialaaoff) makestr(&dialaaoff,NULL); /* Autoanswer Off */
        if (dialx3)    makestr(&dialx3,NULL);    /* Ignore dialtone */
        if (dialspon)  makestr(&dialspon,NULL);  /* Speaker On */
        if (dialspoff) makestr(&dialspoff,NULL); /* Speaker Off */
        if (dialvol1)  makestr(&dialvol1,NULL);  /* Low volume */
        if (dialvol2)  makestr(&dialvol2,NULL);  /* Medium volume */
        if (dialvol3)  makestr(&dialvol3,NULL);  /* High volume */
        if (dialini2)  makestr(&dialini2,NULL);  /* Init string 2 */
    }
    if (autoflow)                       /* Maybe change flow control */
      setflow();

#ifndef MINIDIAL
#ifdef OLDTBCODE
    tbmodel = 0;           /* If it's a Telebit, we don't know the model yet */
#endif /* OLDTBCODE */
#endif /* MINIDIAL */
}

#ifdef COMMENT
/* Not implemented yet */
int
setanswer() {
    int x, y;
    extern int ans_cid, ans_ring;
    if ((x = cmkey(answertab,nanswertab,"","",xxstring)) < 0)
      return(x);
    switch (x) {
      case XYA_CID:
        return(seton(&ans_cid));
      case XYA_RNG:
        y = cmnum("How many rings before answering","1",10,&x,xxstring);
        y = setnum(&ans_rings,x,y,254);
        return(y);
    }
}
#endif /* COMMENT */

int
setmodem() {                            /* SET MODEM */

    int x, y, z;
    long zz;
    struct FDB k1, k2;
    extern int mdmset;

    cmfdbi(&k1,_CMKEY,
           "Modem parameter","","",nsetmdm, 0, xxstring, setmdm, &k2);
    cmfdbi(&k2,_CMKEY,"","","",nmdm,0,xxstring,mdmtab,NULL);
    x = cmfdb(&k1);
    if (x < 0) {                        /* Error */
        if (x == -2 || x == -9)
          printf("?No keywords match: \"%s\"\n",atmbuf);
        return(x);
    }
    y = cmresult.nresult;               /* Keyword value */
    if (cmresult.fdbaddr == &k2) {      /* Modem-type keyword table */
        if ((x = cmcfm()) < 0)
          return(x);
        usermdm = 0;
        initmdm(cmresult.nresult);      /* Set the modem type. */
        return(success = 1);            /* Done */
    }
    switch (cmresult.nresult) {         /* SET MODEM keyword table. */
#ifdef MDMHUP
      case XYDMHU:                      /* DIAL MODEM-HANGUP */
        if ((y = cmkey(mdmhang,4,"how to hang up modem",
                       "modem-command", xxstring)) < 0)
          return(y);
        if ((x = cmcfm()) < 0)
          return(x);
        dialmhu = y;
#ifdef COMMENT
/* Nope, I fixed it (2001 11 08) */
#ifdef CK_SCOV5
        if (dialmhu == 0 && !quiet) {
            printf(
"\n WARNING: RS-232 signal sampling and manipulation do not work\n"
                    );
            printf(
" in the standard SCO OSR5 serial i/o drivers.  SET MODEM HANGUP-METHOD\n"
                   );
            printf(
" MODEM-COMMAND is recommended for OSR5.\n\n"
                    );
        }
#endif /* CK_SCOV5 */
#endif /* COMMENT */
        return(success = 1);
#endif /* MDMHUP */

      case XYDCAP:
        zz = 0L;
        y = 0;
        while (y != -3) {
            if ((y = cmkey(mdmcap,nmdmcap,
                           "capability of modem", "", xxstring)) < 0) {
                if (y == -3)
                  break;
                else
                  return(y);
            }
            zz |= y;
        }
        if ((x = cmcfm()) < 0)
          return(x);
        dialcapas = zz;
        debug(F101,"setmodem autoflow","",autoflow);
        debug(F101,"setmodem flow 1","",flow);
        if (autoflow)                   /* Maybe change flow control */
          setflow();
        debug(F101,"setmodem flow 2","",flow);
        mdmspd = zz & CKD_SB ? 0 : 1;   /* Set MODEM SPEED-MATCHING from it. */
        return(success = 1);

      case XYDMAX:
#ifdef TN_COMPORT
        if (network && istncomport())
          x = cmkey(tnspdtab,ntnspd,line,"",xxstring);
        else
#endif /* TN_COMPORT */
          x = cmkey(spdtab,nspd,line,"",xxstring);
        if (x < 0) {
            if (x == -3) printf("?value required\n");
            return(x);
        }
        if ((y = cmcfm()) < 0) return(y);
        dialmax = (long) x * 10L;
        if (dialmax == 70) dialmax = 75;
        return(success = 1);

      case XYDSTR:                      /* These moved from SET DIAL */
      case XYDDC:
      case XYDEC:
      case XYDESC:
      case XYDFC:
      case XYDKSP:
      case XYDSPD:
      case XYDDIA:
        return(setdial(x));

      case XYDTYP:
        if ((y = cmkey(mdmtab,nmdm,"modem type","none", xxstring)) < 0)
          return(y);
        if (y == dialudt) {             /* User-defined modem type */
            if ((x = cmkey(mdmtab,nmdm,"based on existing modem type",
                           "unknown", xxstring)) < 0)
              return(x);
        }
        if ((z = cmcfm()) < 0)
          return(z);
        usermdm = 0;
        usermdm = (y == dialudt) ? x : 0;
        initmdm(y);
        mdmset = (mdmtyp > 0);
        return(success = 1);

      case XYDNAM:
        return(dialstr(&dialname,"Descriptive name for modem"));

      case XYDMCD:                      /* SET MODEM CARRIER-WATCH */
        return(setdcd());

      case XYDSPK:                      /* SET MODEM SPEAKER */
        return(seton(&mdmspk));

      case XYDVOL:                      /* SET MODEM VOLUME */
        if ((x = cmkey(voltab,3,"","medium",xxstring)) < 0)
          return(x);
        if ((y = cmcfm()) < 0)
          return(y);
        mdmvol = x;
        return(success = 1);

      default:
        printf("Unexpected SET MODEM parameter\n");
        return(-9);
    }
}

static int                              /* Set DIAL command options */
setdial(y) int y; {
    int x = 0, z = 0;
    char *s = NULL;

    if (y < 0)
      if ((y = cmkey(dialtab,ndial,"","",xxstring)) < 0)
        return(y);
    switch (y) {
      case XYDHUP:                      /* DIAL HANGUP */
        return(seton(&dialhng));
      case XYDINI:                      /* DIAL INIT-STRING */
        return(dialstr(&dialini,"Modem initialization string"));
      case XYDNPR:                      /* DIAL PREFIX */
        return(dialstr(&dialnpr,"Telephone number prefix"));
      case XYDDIA:                      /* DIAL DIAL-COMMAND */
        x = cmtxt("Dialing command for modem,\n\
 include \"%s\" to stand for phone number,\n\
 for example, \"set dial dial-command ATDT%s\\13\"",
                  "",
                  &s,
                  xxstring);
        if (x < 0 && x != -3)           /* Handle parse errors */
          return(x);
        s = brstrip(s);                 /* Strip braces or quotes */
        y = x = strlen(s);              /* Get length of text */
        if (y > 0) {                    /* If there is any text (left), */
            for (x = 0; x < y; x++) {   /* make sure they included "%s" */
                if (s[x] != '%') continue;
                if (s[x+1] == 's') break;
            }
            if (x == y) {
                printf(
"?Dial-command must contain \"%cs\" for phone number.\n",'%');
                return(-9);
            }
        }
        if (dialcmd) {                  /* Free any previous string. */
            free(dialcmd);
            dialcmd = (char *) 0;
        }
        if (y > 0) {
            dialcmd = malloc(y + 1);    /* Allocate space for it */
            if (dialcmd)
              strcpy(dialcmd,s);        /* and make a safe copy. */
        }
        return(success = 1);
#ifndef NOXFER
      case XYDKSP:                      /* DIAL KERMIT-SPOOF */
        return(seton(&dialksp));
#endif /* NOXFER */
      case XYDTMO:                      /* DIAL TIMEOUT */
        y = cmnum("Seconds to wait for call completion","0",10,&x,xxstring);
        if (y < 0) return(y);
        y = cmnum("Kermit/modem timeout differential","10",10,&z,xxstring);
        if (y < 0) return(y);
        if ((y = cmcfm()) < 0)
          return(y);
        dialtmo = x;
        mdmwaitd = z;
      case XYDESC:                      /* DIAL ESCAPE-CHARACTER */
        y = cmnum("ASCII value of character to escape back to modem",
                  "43",10,&x,xxstring);
        y = setnum(&dialesc,x,y,128);
        if (y > -1 && dialesc < 0)      /* No escape character */
          dialmhu = 0;                  /* So no hangup by modem command */
        return(y);
      case XYDDPY:                      /* DIAL DISPLAY */
        return(seton(&dialdpy));
      case XYDSPD:                      /* DIAL SPEED-MATCHING */
                                        /* used to be speed-changing */
        if ((y = seton(&mdmspd)) < 0) return(y);
#ifdef COMMENT
        mdmspd = 1 - mdmspd;            /* so here we reverse the meaning */
#endif /* COMMENT */
        return(success = 1);
      case XYDMNP:                      /* DIAL MNP-ENABLE */
      case XYDEC:                       /* DIAL ERROR-CORRECTION */
        x = seton(&dialec);
        if (x > 0)
          if (!dialec) dialdc = 0;      /* OFF also turns off compression */
        return(x);

      case XYDDC:                       /* DIAL COMPRESSION */
        x = seton(&dialdc);
        if (x > 0)
          if (dialdc) dialec = 1;       /* ON also turns on error correction */
        return(x);

#ifdef MDMHUP
      case XYDMHU:                      /* DIAL MODEM-HANGUP */
        return(seton(&dialmhu));
#endif /* MDMHUP */

#ifndef NOSPL
      case XYDDIR:                      /* DIAL DIRECTORY (zero or more) */
        return(parsdir(0));             /* 0 means DIAL */
#endif /* NOSPL */

      case XYDSTR:                      /* DIAL STRING */
        if ((y = cmkey(mdmcmd,nmdmcmd,"","",xxstring)) < 0) return(y);
        switch (y) {
          case XYDS_AN:                 /* Autoanswer ON/OFF */
          case XYDS_DC:                 /* Data compression ON/OFF */
          case XYDS_EC:                 /* Error correction ON/OFF */
            if ((x = cmkey(onoff,2,"","on",xxstring)) < 0)
              return(x);
            sprintf(tmpbuf,"Modem's command to %sable %s", /* SAFE */
                    x ? "en" : "dis",
                    (y == XYDS_DC) ? "compression" :
                    ((y == XYDS_EC) ? "error-correction" :
                    "autoanswer")
                    );
            if (x) {
                if (y == XYDS_DC)
                  return(dialstr(&dialdcon,tmpbuf));
                else if (y == XYDS_EC)
                  return(dialstr(&dialecon,tmpbuf));
                else
                  return(dialstr(&dialaaon,tmpbuf));
            } else {
                if (y == XYDS_DC)
                  return(dialstr(&dialdcoff,tmpbuf));
                else if (y == XYDS_EC)
                  return(dialstr(&dialecoff,tmpbuf));
                else
                  return(dialstr(&dialaaoff,tmpbuf));
            }
          case XYDS_HU:                 /*    hangup command */
            return(dialstr(&dialhcmd,"Modem's hangup command"));
          case XYDS_HW:                 /*    hwfc */
            return(dialstr(&dialhwfc,
                           "Modem's command to enable hardware flow control"));
          case XYDS_IN:                 /*    init */
            return(dialstr(&dialini,"Modem's initialization string"));
          case XYDS_NF:                 /*    no flow control */
            return(dialstr(&dialnofc,
                           "Modem's command to disable local flow control"));
          case XYDS_PX:                 /*    prefix */
            return(dialstr(&dialnpr,"Telephone number prefix for dialing"));
          case XYDS_SW:                 /*    swfc */
            return(dialstr(&dialswfc,
                   "Modem's command to enable local software flow control"));
          case XYDS_DT:                 /*    tone dialing */
            return(dialstr(&dialtone,
                   "Command to configure modem for tone dialing"));
          case XYDS_DP:                 /*    pulse dialing */
            return(dialstr(&dialpulse,
                           "Command to configure modem for pulse dialing"));
          case XYDS_MS:                 /*    dial mode string */
            return(dialstr(&dialmstr,
                         "Command to enter dial mode"));
          case XYDS_MP:                 /*    dial mode prompt */
            return(dialstr(&dialmprmt,
                           "Modem response upon entering dial mode"));
          case XYDS_SP:                 /* SPEAKER OFF */
            if ((x = cmkey(onoff,2,"","on",xxstring)) < 0) return(x);
            if (x)
              return(dialstr(&dialspon,"Command to turn modem speaker on"));
            else
              return(dialstr(&dialspoff,"Command to turn modem speaker off"));

          case XYDS_VO:                 /* VOLUME LOW */
            if ((x = cmkey(voltab,3,"","medium",xxstring)) < 0) return(x);
            switch (x) {
              case 0:
              case 1:
                return(dialstr(&dialvol1,
                               "Command for low modem speaker volume"));
              case 2:
                return(dialstr(&dialvol2,
                           "Command for medium modem speaker volume"));

              case 3:
                return(dialstr(&dialvol3,
                               "Command for high modem speaker volume"));
              default:
                return(-2);
            }

          case XYDS_ID:                 /* IGNORE-DIALTONE */
            return(dialstr(&dialx3,
                           "Command to tell modem to ignore dialtone"));

          case XYDS_I2:                 /* PREDIAL-INIT */
            return(dialstr(&dialini2,
                           "Command to send to modem just prior to dialing"));

          default:
            printf("?Unexpected SET DIAL STRING parameter\n");
        }

      case XYDFC:                       /* DIAL FLOW-CONTROL */
        if ((y = cmkey(dial_fc,4,"","auto",xxstring)) < 0) return(y);
        if ((x = cmcfm()) < 0) return(x);
        dialfc = y;
        return(success = 1);

      case XYDMTH: {                    /* DIAL METHOD */
        extern int dialmauto;
        if ((y = cmkey(dial_m,ndial_m,"","default",xxstring)) < 0)
          return(y);
        if ((x = cmcfm()) < 0)
          return(x);
        if (y == XYDM_A) {              /* AUTO */
            dialmauto = 1;              /* local country code, if known. */
            dialmth = XYDM_D;
        } else {
          dialmauto = 0;                /* use the method specified */
          dialmth = y;
        }
        return(success = 1);
      }
      case XYDRTM:
        y = cmnum("Number of times to try dialing a number",
                  "1",10,&x,xxstring);
	z = setnum(&dialrtr,x,y,-1);
	if (z > -1 && dialrtr < 0) {
	    printf("?Sorry, negative dial retries not valid: %d\n",dialrtr);
	    return(-9);
	}
        return(z);

      case XYDINT:
        y = cmnum("Seconds to wait between redial attempts",
                  "30",10,&x,xxstring);
        z = setnum(&dialint,x,y,-1);
	if (z > -1 && dialint < 0) {
	    printf("?Sorry, negative dial interval not valid: %d\n",dialint);
	    return(-9);
	}
        return(z);

      case XYDLAC:                      /* DIAL AREA-CODE */
        if ((x = dialstr(&diallac,"Area code you are calling from")) < 0)
          return(x);
        if (diallac) {
            if (!rdigits(diallac)) {
                printf("?Sorry, area code must be numeric\n");
                if (*diallac == '(')
                  printf("(please omit the parentheses)\n");
                if (*diallac == '/')
                  printf("(no slashes, please)\n");
                if (diallac) free(diallac);
                diallac = NULL;
                return(-9);
            }
        }
        return(x);

      case XYDCNF:                      /* CONFIRMATION */
        return(success = seton(&dialcnf));

      case XYDCVT:                      /* CONVERT-DIRECTORY */
        if ((y = cmkey(dcnvtab,3,"","ask",xxstring)) < 0)
          return(y);
        if ((x = cmcfm()) < 0)
          return(x);
        dialcvt = y;
        return(success = 1);

      case XYDLCC:                      /* DIAL COUNTRY-CODE */
        x = dialstr(&diallcc,"Country code you are calling from");
        if (x < 1) return(x);
        if (diallcc) {
            if (!rdigits(diallcc)) {
                printf("?Sorry, country code must be numeric\n");
                if (*diallcc == '+')
                  printf("(please omit the plus sign)\n");
                if (diallcc) free(diallcc);
                diallcc = NULL;
                return(-9);
            }
            if (!strcmp(diallcc,"1")) { /* Set defaults for USA and Canada */
                if (!dialldp)           /* Long-distance prefix */
                  makestr(&dialldp,"1");
                if (!dialixp)           /* International dialing prefix */
                  makestr(&dialixp,"011");
                if (ntollfree == 0) {   /* Toll-free area codes */
                    if ((dialtfc[0] = malloc(4))) {
                        strcpy(dialtfc[0],"800"); /* 1970-something */
                        ntollfree++;
                        if ((dialtfc[1] = malloc(4))) {
                            strcpy(dialtfc[1],"888"); /* 1996 */
                            ntollfree++;
                            if ((dialtfc[2] = malloc(4))) {
                                strcpy(dialtfc[2],"877"); /* 5 April 1998 */
                                ntollfree++;
                                if ((dialtfc[3] = malloc(4))) {
                                    strcpy(dialtfc[3],"866"); /* 2000? */
                                    ntollfree++;
                                }
                            }
                        }
                    }
                }
                if (!dialtfp)           /* Toll-free dialing prefix */
                  makestr(&dialtfp,"1");
#ifdef COMMENT
/* The time for this is past */
            } else if (!strcmp(diallcc,"358") &&
                       ((int) strcmp(zzndate(),"19961011") > 0)
                       ) {              /* Finland */
                if (!dialldp)           /* Long-distance prefix */
                  makestr(&dialldp,"9");
                if (!dialixp)           /* International dialing prefix */
                  makestr(&dialixp,"990");
#endif /* COMMENT */
            } else {                    /* Everywhere else ... */
                if (!dialldp) {
                    if ((dialldp = malloc(4)))
                      strcpy(dialldp,"0");
                }
                if (!dialixp) {
                    if ((dialixp = malloc(4)))
                      strcpy(dialixp,"00");
                }
            }
            if (!strcmp(diallcc,"33"))  /* France */
              dialfld = 1;              /* Long-distance dialing is forced */
        }
        return(success = 1);

      case XYDIXP:                      /* DIAL INTL-PREFIX */
        return(dialstr(&dialixp,"International dialing prefix"));

      case XYDIXS:                      /* DIAL INTL-SUFFIX */
        return(dialstr(&dialixs,"International dialing suffix"));

      case XYDLDP:                      /* DIAL LD-PREFIX */
        return(dialstr(&dialldp,"Long-distance dialing prefix"));

      case XYDLDS:                      /* DIAL LD-SUFFIX */
        return(dialstr(&diallds,"Long-distance dialing suffix"));

      case XYDLCP:                      /* DIAL LC-PREFIX */
        return(dialstr(&diallcp,"Local dialing prefix"));

      case XYDLCS:                      /* DIAL LC-SUFFIX */
        return(dialstr(&diallcs,"Local dialing suffix"));

#ifdef COMMENT
      case XYDPXX:                      /* DIAL PBX-EXCHANGE */
        return(dialstr(&dialpxx,"Exchange of PBX you are calling from"));
#endif /* COMMENT */

      case XYDPXI: {                    /* DIAL PBX-INTERNAL-PREFIX */
#ifdef COMMENT
          return(dialstr(&dialpxi,
                       "Internal-call prefix of PBX you are calling from"));
#else
          int x;
          if ((x = cmtxt("Internal-call prefix of PBX you are calling from",
                         "",&s,NULL)) < 0) /* Don't evaluate */
            return(x);
#ifndef NOSPL
          if (*s) {
              char c, * p = tmpbuf;
              if (*s == '\\') {
                  c = *(s+1);
                  if (isupper(c)) c = tolower(c);
                  if (c != 'f' &&
                      ckstrcmp(s,"\\v(d$px)",8,0) &&
                      ckstrcmp(s,"\\v(d$pxx)",9,0) &&
                      ckstrcmp(s,"\\v(d$p)",7,0)) {
                      x = TMPBUFSIZ;
                      zzstring(s,&p,&x);
                      s = tmpbuf;
                  }
              }
          }
#endif /* NOSPL */
          makestr(&dialpxi,s);
          return(1);
      }
#endif /* COMMENT */

      case XYDPXO:                      /* DIAL PBX-OUTSIDE-PREFIX */
        return(dialstr(&dialpxo,
                       "Outside-line prefix of PBX you are calling from"));

      case XYDSFX:                      /* DIAL INTL-SUFFIX */
        return(dialstr(&dialsfx," Telephone number suffix for dialing"));

      case XYDSRT:                      /* DIAL SORT */
        return(success = seton(&dialsrt));

      case XYDPXX:                      /* DIAL PBX-EXCHANGE */
      case XYDTFC: {                    /* DIAL TOLL-FREE-AREA-CODE  */
          int n, i;                     /* (zero or more of them...) */
          char * p[MAXTOLLFREE];        /* Temporary pointers */
          char * m;
          for (n = 0; n < MAXTOLLFREE; n++) {
              if (n == 0) {
                  m = (y == XYDTFC) ?
                  "Toll-free area code(s) in the country you are calling from"
                    : "Exchange(s) of PBX you are calling from";
              } else {
                  m = (y == XYDTFC) ?
                    "Another toll-free area code"
                      : "Another PBX exchange";
              }
              if ((x = cmfld(m,"",&s,xxstring)) < 0)
                break;
              if (s) {
                  int k;
                  k = (int) strlen(s);
                  if (k > 0) {
                      if ((p[n] = malloc(k + 1)))
                        strcpy(p[n], s); /* safe */
                  } else break;
              } else break;
          }
          if (x == -3) {                /* Command was successful */
              int m;
              m = (y == XYDTFC) ? ntollfree : ndialpxx;
              if ((x = cmcfm()) < 0)
                return(x);
              x = 1;
              for (i = 0; i < m; i++) { /* Remove old list, if any */
                  if  (y == XYDTFC)
                    makestr(&(dialtfc[i]),NULL);
                  else
                    makestr(&(dialpxx[i]),NULL);
              }
              if  (y == XYDTFC)
                ntollfree = n;          /* New count */
              else
                ndialpxx = n;
              for (i = 0; i < n; i++) { /* New list */
                  if  (y == XYDTFC)
                    makestr(&(dialtfc[i]),p[i]);
                  else
                    makestr(&(dialpxx[i]),p[i]);
              }
              x = 1;
          }
          for (i = 0; i < n; i++)
            if (p[i]) free(p[i]);
          return(x);
      }

      case XYDTFP:                      /* TOLL-FREE-PREFIX */
        return(dialstr(&dialtfp,
                       " Long-distance prefix for toll-free dialing"));

      case XYDCON:                      /* CONNECT */
        z = -1;
        if ((y = cmkey(crrtab,ncrr,"","auto",xxstring)) < 0) return(y);
        if (y != CAR_OFF)               /* AUTO or ON? */
          if ((z = cmkey(qvtab,nqvt,"","verbose",xxstring)) < 0) return(z);
        if ((x = cmcfm()) < 0) return(x);
        if (z > -1)
          dialcq = z;
        dialcon = y;
        return(success = 1);

      case XYDRSTR:                     /* RESTRICT */
        if ((y = cmkey(drstrtab,4,"","none",xxstring)) < 0) return(y);
        if ((x = cmcfm()) < 0) return(x);
        dialrstr = y;
        return(success = 1);

      case XYDLLAC: {                   /* Local area-code list  */
          int n, i;                     /* (zero or more of them...) */
          char * p[MAXLOCALAC]; /* Temporary pointers */
          for (n = 0; n < MAXLOCALAC; n++) {
              if ((x = cmfld(
                    "Area code to which calls from your area are local",
                       "",&s,xxstring)) < 0)
                break;
              if (s) {
                  int k;
                  k = (int) strlen(s);
                  if (k > 0) {
                      if ((p[n] = malloc(k + 1)))
                        strcpy(p[n], s); /* safe */
                  } else break;
              } else break;
          }
          if (x == -3) {                /* Command was successful */
              if ((x = cmcfm()) < 0)
                return(x);
              for (i = 0; i < nlocalac; i++) /* Remove old list, if any */
                if (diallcac[i]) {
                    free(diallcac[i]);
                    diallcac[i] = NULL;
                }
              nlocalac = n;             /* New count */
              for (i = 0; i < nlocalac; i++) /* New list */
                diallcac[i] = p[i];
              return(success = 1);
          } else {                      /* Parse error, undo everything */
              for (i = 0; i < n; i++)
                if (p[i]) free(p[i]);
              return(x);
          }
      }

      case XYDFLD:
        return(success = seton(&dialfld));

      case XYDIDT:                      /* DIAL IGNORE-DIALTONE */
        return(seton(&dialidt));

      case XYDPAC:
        y = cmnum(
              "Milliseconds to pause between each character sent to dialer",
                  "",10,&x,xxstring);
        return(setnum(&dialpace,x,y,9999));

#ifndef NOSPL
      case XYDMAC:
        if ((x = cmfld("Name of macro to execute just prior to dialing",
                       "",&s,xxstring)) < 0) {
            if (x == -3)
              s = NULL;
            else
              return(x);
        }
        if (s) {
            if (!*s) {
                s = NULL;
            } else {
                ckstrncpy(line,s,LINBUFSIZ);
                s = line;
            }
        }
        if ((x = cmcfm()) < 0)
          return(x);
        makestr(&dialmac,s);
        return(success = 1);
#endif /* NOSPL */

      case XYDPUCC:                     /* Pulse country codes */
      case XYDTOCC: {                   /* Tone country codes */
          int n, i;
          char * p[MAXTPCC];
          char * m;
          for (n = 0; n < MAXTPCC; n++) {
              if (n == 0) {
                  m = (y == XYDPUCC) ?
                  "Country code where Pulse dialing is required"
                    : "Country code where Tone dialing is available";
              } else
                m = "Another country code";
              if ((x = cmfld(m,"",&s,xxstring)) < 0)
                break;
              if (s) {
                  int k;
                  k = (int) strlen(s);
                  if (k > 0) {
                      if ((p[n] = malloc(k + 1)))
                        strcpy(p[n], s); /* safe */
                  } else break;
              } else break;
          }
          if (x == -3) {                /* Command was successful */
              int m;
              m = (y == XYDPUCC) ? ndialpucc : ndialtocc;
              if ((x = cmcfm()) < 0)
                return(x);
              x = 1;
              for (i = 0; i < m; i++) { /* Remove old list, if any */
                  if (y == XYDPUCC)
                    makestr(&(dialpucc[i]),NULL);
                  else
                    makestr(&(dialtocc[i]),NULL);
              }
              if (y == XYDPUCC) {
                  ndialpucc = n;                /* New count */
              } else {
                  ndialtocc = n;
              }
              for (i = 0; i < n; i++) { /* New list */
                  if (y == XYDPUCC) {
                      makestr(&(dialpucc[i]),p[i]);
                  } else {
                      makestr(&(dialtocc[i]),p[i]);
                  }
              }
              x = 1;
          }
          for (i = 0; i < n; i++)
            if (p[i]) free(p[i]);
          return(x);
      }
      case XYDTEST:
        return(seton(&dialtest));

      default:
        printf("?Unexpected SET DIAL parameter\n");
        return(-9);
    }
}

#ifndef NOSHOW
int                                     /* SHOW MODEM */
shomodem() {
    MDMINF * p;
    int x, n, mdm;
    char c;
    long zz;

#ifdef IKSD
    if (inserver) {
        printf("Sorry, command disabled\r\n");
        return(success = 0);
    }
#endif /* IKSD */

    shmdmlin();
    printf("\n");

    mdm = (mdmtyp > 0) ? mdmtyp : mdmsav;
    p = (mdm > 0) ? modemp[mdm] : NULL;

    if (p) {
        printf(" %s\n\n", dialname ? dialname : p->name);

        printf(" Modem capabilities:    ");
        zz = dialcapas ? dialcapas : p->capas;
        if (!zz) {
            printf(" (none)");
        } else {
            if (zz & CKD_AT) printf(" AT");
            if (zz & CKD_V25) printf(" ITU");
            if (zz & CKD_SB) printf(" SB");
            if (zz & CKD_EC) printf(" EC");
            if (zz & CKD_DC) printf(" DC");
            if (zz & CKD_HW) printf(" HWFC");
            if (zz & CKD_SW) printf(" SWFC");
            if (zz & CKD_KS) printf(" KS");
            if (zz & CKD_TB) printf(" TB");
        }
        printf("\n Modem carrier-watch:    ");
        if (carrier == CAR_OFF) printf("off\n");
        else if (carrier == CAR_ON) printf("on\n");
        else if (carrier == CAR_AUT) printf("auto\n");
        else printf("unknown\n");

        printf(" Modem maximum-speed:    ");
        zz = (dialmax > 0L) ? dialmax : p->max_speed;
        if (zz > 0)
          printf("%ld bps\n", zz);
        else
          printf("(unknown)\n");
        printf(" Modem error-correction: %s\n", dialec ? "on" : "off");
        printf(" Modem compression:      %s\n", dialdc ? "on" : "off");
        printf(" Modem speed-matching:   %s",   mdmspd ? "on" : "off");
        printf(" (interface speed %s)\n", mdmspd ? "changes" : "is locked");
        printf(" Modem flow-control:     ");
        if (dialfc == FLO_NONE) printf("none\n");
        else if (dialfc == FLO_XONX) printf("xon/xoff\n");
        else if (dialfc == FLO_RTSC) printf("rts/cts\n");
        else if (dialfc == FLO_AUTO) printf("auto\n");
        printf(" Modem hangup-method:    %s\n",
               dialmhu ?
               "modem-command" :
               "rs232-signal"
               );
        printf(" Modem speaker:          %s\n", showoff(mdmspk));
        printf(" Modem volume:           %s\n",
               (mdmvol == 2) ? "medium" : ((mdmvol <= 1) ? "low" : "high"));
        printf(" Modem kermit-spoof:     %s\n", dialksp ? "on" : "off");
        c = (char) (x = (dialesc ? dialesc : p->esc_char));
        printf(" Modem escape-character: %d", x);
        if (isprint(c))
          printf(" (= \"%c\")",c);
        printf(
"\n\nMODEM COMMANDs (* = set automatically by SET MODEM TYPE):\n\n");
        debug(F110,"show dialini",dialini,0);
        printf(" %c Init-string:          ", dialini ? ' ' : '*' );
        shods(dialini ? dialini : p->wake_str);
        printf(" %c Dial-mode-string:     ", dialmstr ? ' ' : '*' );
        shods(dialmstr ? dialmstr : p->dmode_str);
        n = local ? 19 : 20;
        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
        printf(" %c Dial-mode-prompt:     ", dialmprmt ? ' ' : '*' );
        shods(dialmprmt ? dialmprmt : p->dmode_prompt);
        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
        printf(" %c Dial-command:         ", dialcmd ? ' ' : '*' );
        shods(dialcmd ? dialcmd : p->dial_str);
        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
        printf(" %c Compression on:       ", dialdcon ? ' ' : '*' );
        if (!dialdcon)
          debug(F110,"dialdcon","(null)",0);
        else
          debug(F110,"dialdcon",dialdcon,0);
        shods(dialdcon ? dialdcon : p->dc_on_str);
        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
        printf(" %c Compression off:      ", dialdcoff ? ' ' : '*' );
        shods(dialdcoff ? dialdcoff : p->dc_off_str);
        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
        printf(" %c Error-correction on:  ", dialecon ? ' ' : '*' );
        shods(dialecon ? dialecon : p->ec_on_str);
        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
        printf(" %c Error-correction off: ", dialecoff ? ' ' : '*' );
        shods(dialecoff ? dialecoff : p->ec_off_str);
        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
        printf(" %c Autoanswer on:        ", dialaaon ? ' ' : '*' );
        shods(dialaaon ? dialaaon : p->aa_on_str);
        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
        printf(" %c Autoanswer off:       ", dialaaoff ? ' ' : '*' );
        shods(dialaaoff ? dialaaoff : p->aa_off_str);
        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;

        printf(" %c Speaker on:           ", dialspon ? ' ' : '*' );
        shods(dialspon ? dialspon : p->sp_on_str);
        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
        printf(" %c Speaker off:          ", dialspoff ? ' ' : '*' );
        shods(dialspoff ? dialspoff : p->sp_off_str);
        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
        printf(" %c Volume low:           ", dialvol1 ? ' ' : '*' );
        shods(dialvol1 ? dialvol1 : p->vol1_str);
        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
        printf(" %c Volume medium:        ", dialvol2 ? ' ' : '*' );
        shods(dialvol2 ? dialvol2 : p->vol2_str);
        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
        printf(" %c Volume high:          ", dialvol3 ? ' ' : '*' );
        shods(dialvol3 ? dialvol3 : p->vol3_str);
        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;

        printf(" %c Hangup-command:       ", dialhcmd ? ' ' : '*' );
        shods(dialhcmd ? dialhcmd : p->hup_str);
        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
        printf(" %c Hardware-flow:        ", dialhwfc ? ' ' : '*' );
        shods(dialhwfc ? dialhwfc : p->hwfc_str);
        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
        printf(" %c Software-flow:        ", dialswfc ? ' ' : '*' );
        shods(dialswfc ? dialswfc : p->swfc_str);
        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
        printf(" %c No-flow-control:      ", dialnofc ? ' ' : '*' );
        shods(dialnofc ? dialnofc : p->nofc_str);
        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
        printf(" %c Pulse:                ", dialpulse ? ' ' : '*');
        shods(dialpulse ? dialpulse : p->pulse);
        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
        printf(" %c Tone:                 ", dialtone ? ' ' : '*');
        shods(dialtone ? dialtone : p->tone);

        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
        printf(" %c Ignore-dialtone:      ", dialx3 ? ' ' : '*');
        shods(dialx3 ? dialx3 : p->ignoredt);

        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
        printf(" %c Predial-init:         ", dialini2 ? ' ' : '*');
        shods(dialini2 ? dialini2 : p->ini2);

        if (++n > cmd_rows - 4) if (!askmore()) return(0); else n = 0;
        printf("\n For more info: SHOW DIAL and SHOW COMMUNICATIONS\n");

    } else if (mdm > 0) {
        printf("Modem info for \"%s\" not filled in yet\n", gmdmtyp());
    } else printf(
" No modem selected, so DIAL and most SET MODEM commands have no effect.\n\
 Use SET MODEM TYPE to select a modem.\n");
    return(success = 1);
}
#endif /* NOSHOW */
#endif /* NODIAL */

#ifdef CK_TAPI
int                                             /* TAPI action commands */
dotapi() {
    int x,y;
    char *s;

    if (!TAPIAvail) {
        printf("\nTAPI is unavailable on this system.\n");
        return(-9);
    }
    if ((y = cmkey(tapitab,ntapitab,"MS TAPI command","",xxstring)) < 0)
      return(y);
    switch (y) {
      case XYTAPI_CFG: {                        /* TAPI CONFIGURE-LINE */
          extern struct keytab * tapilinetab;
          extern struct keytab * _tapilinetab;
          extern int ntapiline;
          extern int LineDeviceId;
          int lineID=LineDeviceId;
          if (TAPIAvail)
            cktapiBuildLineTable(&tapilinetab, &_tapilinetab, &ntapiline);
          if (tapilinetab && _tapilinetab && ntapiline > 0) {
              int i=0, j = 9999, k = -1;

              if ( LineDeviceId == -1 ) {
                  /* Find out what the lowest numbered TAPI device is */
                  /* and use it as the default.                       */
                  for (i = 0; i < ntapiline; i++ ) {
                      if (tapilinetab[i].kwval < j) {
                          k = i;
                      }
                  }
              } else {
                  /* Find the LineDeviceId in the table and use that entry */
                  for (i = 0; i < ntapiline; i++ ) {
                      if (tapilinetab[i].kwval == LineDeviceId) {
                          k = i;
                          break;
                      }
                  }
              }
              if (k >= 0)
                s = _tapilinetab[k].kwd;
              else
                s = "";

              if ((y = cmkey(_tapilinetab,ntapiline,
                              "TAPI device name",s,xxstring)) < 0)
                return(y);
              lineID = y;
          }
          if ((x = cmcfm()) < 0) return(x);
#ifdef IKSD
          if (inserver) {
              printf("Sorry, command disabled\r\n");
              return(success = 0);
          }
#endif /* ISKD */
          cktapiConfigureLine(lineID);
          break;
      }
      case XYTAPI_DIAL:                 /* TAPI DIALING-PROPERTIES */
        if ((x = cmcfm()) < 0)
          return(x);
#ifdef IKSD
        if (inserver) {
            printf("Sorry, command disabled\r\n");
            return(success = 0);
        }
#endif /* ISKD */
        cktapiDialingProp();
        break;
    }
    return(success = 1);
}

static int                              /* SET TAPI command options */
settapi() {
    int x, y;
    char *s;

    if (!TAPIAvail) {
        printf("\nTAPI is unavailable on this system.\n");
        return(-9);
    }
    if ((y = cmkey(settapitab,nsettapitab,"MS TAPI option","",xxstring)) < 0)
      return(y);
    switch (y) {
      case XYTAPI_USE:
        return (success = seton(&tapiusecfg));
      case XYTAPI_LGHT:
        return (success = seton(&tapilights));
      case XYTAPI_PRE:
        return (success = seton(&tapipreterm));
      case XYTAPI_PST:
        return (success = seton(&tapipostterm));
      case XYTAPI_INA:
        y = cmnum("seconds of inactivity before auto-disconnect",
                  "0",10,&x,xxstring);
        return(setnum(&tapiinactivity,x,y,65535));
      case XYTAPI_BNG:
        y = cmnum("seconds to wait for credit card tone",
                  "8",10,&x,xxstring);
        return(setnum(&tapibong,x,y,90));
      case XYTAPI_MAN:
        return (success = seton(&tapimanual));
      case XYTAPI_CON:                  /* TAPI CONVERSIONS */
        return (success = setonaut(&tapiconv));
      case XYTAPI_LIN:                  /* TAPI LINE */
        x = setlin(XYTAPI_LIN,1,0);
        if (x > -1) didsetlin++;
        return(x);
      case XYTAPI_PASS: {               /* TAPI PASSTHROUGH */
#ifdef NODIAL
          printf("\n?Modem-dialing not supported\n");
          return(-9);
#else /* NODIAL */
          /* Passthrough became Modem-dialing which is an antonym */
          success = seton(&tapipass);
          tapipass = !tapipass;
          return (success);
#endif /* NODIAL */        
      }
      case XYTAPI_LOC: {                /* TAPI LOCATION */
          extern char tapiloc[];
          extern int tapilocid;
          int i = 0, j = 9999, k = -1;

          cktapiBuildLocationTable(&tapiloctab, &ntapiloc);
          if (!tapiloctab || !ntapiloc) {
              printf("\nNo TAPI Locations are configured for this system\n");
              return(-9);
          }
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
            s = "";

          if ((y = cmkey(tapiloctab,ntapiloc, "TAPI location",s,xxstring)) < 0)
            return(y);

          if ((x = cmcfm()) < 0)
            return(x);
#ifdef IKSD
          if (inserver) {
              printf("Sorry, command disabled\r\n");
              return(success = 0);
          }
#endif /* IKSD */
          cktapiFetchLocationInfoByID( y );
#ifndef NODIAL
          CopyTapiLocationInfoToKermitDialCmd();
#endif /* NODIAL */
        }
        break;
    }
    return(success=1);
}
#endif /* CK_TAPI */
#endif /* NOLOCAL */

#ifndef NOSPL
/* Method for evaluating \%x and \&x[] variables */

static struct keytab varevaltab[] = {
    { "recursive", 1, 0 },
    { "simple",    0, 0 }
};
static int nvarevaltab = (sizeof(varevaltab) / sizeof(struct keytab));

int
setvareval() {
    int x = 0, y = 0;
    extern int vareval;
#ifdef DCMDBUF
    extern int * xvarev;
#else
    extern int xvarev[];
#endif /* DCMDBUF */

    if ((x = cmkey(varevaltab,
		   nvarevaltab, 
		   "Method for evaluating \\%x and \\&x[] variables",
		   "",
		   xxstring)) < 0)
      return(x);
    if ((y = cmcfm()) < 0)
      return(y);
    xvarev[cmdlvl] = x;
    vareval = x;
    return(success = 1);
}

#ifdef CK_ANSIC                         /* SET ALARM */
int
setalarm(long xx)
#else
int
setalarm(xx) long xx;
#endif /* CK_ANSIC */
/* setalarm */ {
#ifdef COMMENT
    int yyyy, mm, dd, x;
    char *s;
    long zz;
    char buf[6];
#endif /* COMMENT */
    long sec, jd;
    char xbuf[20], * p;

    debug(F101,"setalarm xx","",xx);
    ck_alarm = 0L;                      /* 0 = no alarm (in case of error) */
    if (xx < 0L) {
        printf("%ld - illegal value, must be 0 or positive\n", xx);
        return(-9);
    }
    if (xx == 0L) {                     /* CLEAR ALARM */
        alrm_date[0] = NUL;
        alrm_time[0] = NUL;
        return(1);
    }
#ifdef COMMENT
    x = 8;                              /* Get current date */
    s = alrm_date;
    if (zzstring("\\v(ndate)",&s,&x) < 0) {
        printf("Internal date error, sorry.\n");
        alrm_date[0] = SP;
        return(-9);
    }
    x = 5;                              /* Get current time */
    s = alrm_time;
    if (zzstring("\\v(ntime)",&s,&x) < 0) {
        printf("Internal time error, sorry.\n");
        alrm_time[0] = SP;
        return(-9);
    }
    sprintf(buf,"%05ld",atol(alrm_time)); /* SAFE (20) */
    ckstrncpy(alrm_time,buf,8);
    debug(F110,"SET ALARM date (1)",alrm_date,0);
    debug(F110,"SET ALARM time (1)",alrm_time,0);

    if ((zz = atol(alrm_time) + xx) < 0L) {
        printf("Internal time conversion error, sorry.\n");
        return(-9);
    }
    if (zz >= 86400L) {                 /* Alarm crosses midnight */
        char d[10];                     /* Local date buffer */
        int lastday;                    /* Last day of this month */

        ckstrncpy(d,alrm_date,8);       /* We'll have to change the date */

        x = (zz / 86400L);              /* How many days after today */

        dd = atoi((char *)(d+6));       /* Parse yyyymmdd */
        d[6] = NUL;                     /* into yyyy, mm, dd ... */
        mm = atoi((char *)(d+4));
        d[4] = NUL;
        yyyy = atoi((char *)d);

        /* How many days in this month */

        lastday = mdays[mm];
        if (mm == 2 && yyyy % 4 == 0)   /* Works thru 2099 AD... */
          lastday++;

        if (dd + x > lastday) {         /* Dumb loop */
            int y;

            x -= (mdays[mm] - dd);      /* Deduct rest of this month's days */

            /* There's a more elegant way to do this... */

            while (1) {
                mm++;                   /* Next month */
                if (mm > 12) {          /* Wrap around */
                    mm = 1;             /* Jan, next year */
                    yyyy++;
                }
                y = mdays[mm];          /* Days in new month */
                if (mm == 2 && yyyy % 4 == 0) /* Feb in leap year */
                  y++;                  /* Works until 2100 AD */
                if (x - y < 1)
                  break;
                x -= y;
            }
            dd = x;                     /* Day of alarm month */
        } else dd += x;

        sprintf(alrm_date,"%04d%02d%02d",yyyy,mm,dd); /* SAFE (24) */
        zz = zz % 86400L;
    }
    sprintf(alrm_time,"%ld",zz);        /* SAFE (24) */
    debug(F110,"SET ALARM date (2)",alrm_date,0);
    debug(F110,"SET ALARM time (2)",alrm_time,0);
    ck_alarm = xx;
#else
    /* Jul 1998 */
    ckstrncpy(xbuf,ckcvtdate("",1),20); /* Get current date and time */
    p = xbuf;
    ckstrncpy(alrm_date,xbuf,10);
    alrm_date[8] = NUL;
    sec = atol(p+9) * 3600L + atol(p+12) * 60L + atol(p+15);
    debug(F110,"SET ALARM date (1)",alrm_date,0);
    debug(F101,"SET ALARM time (1)","",sec);
    if ((sec += xx) < 0L) {
        printf("Internal time conversion error, sorry.\n");
        return(-9);
    }
    if (sec >= 86400L) {                /* Alarm crosses midnight */
        long days;
        days = sec / 86400L;
        jd = mjd(p) + days;             /* Get modified Julian date */
        ckstrncpy(alrm_date,mjd2date(jd),10);
        sec %= 86400L;
    }
    sprintf(alrm_time,"%05ld",sec);     /* SAFE (24) */
    debug(F110,"SET ALARM date (2)",alrm_date,0);
    debug(F110,"SET ALARM time (2)",alrm_time,0);
    ck_alarm = 1;                       /* Alarm is set */

#endif /* COMMENT */
    return(success = 1);
}
#endif /* NOSPL */

#ifndef NOSETKEY
int
dosetkey() {                            /* SET KEY */
    int x, y;
    int flag = 0;
    int kc;                             /* Key code */
    char *s;                            /* Key binding */
#ifndef NOKVERBS
    char *p;                            /* Worker */
#endif /* NOKVERBS */
#ifdef OS2
    extern int os2gks;
    extern int mskkeys;
    extern int initvik;
#endif /* OS2 */

    x_ifnum = 1;
    y = cmnum("numeric key code, or the word CLEAR,","",10,&kc,xxstring);
    x_ifnum = 0;
    if (y < 0) {
        debug(F111,"SET KEY",atmbuf,y);
        if (y == -2) {                  /* Not a valid number */
            if ((y = strlen(atmbuf)) < 0) /* Check for SET KEY CLEAR */
              return(-2);
            if (ckstrcmp(atmbuf,"clear",y,0))
              return(-2);
            if ((x = cmcfm()) < 0)
              return(x);
            for (y = 0; y < KMSIZE; y++) {
                keymap[y] = (KEY) y;
                macrotab[y] = NULL;
            }
#ifdef OS2
            keymapinit();               /* Special OS/2 initializations */
            initvik = 1;                /* Update the VIK table */
#endif /* OS2 */
            return(1);
        } else if (y == -3) {           /* SET KEY <Return> */
            printf(" Press key to be defined: "); /* Prompt for a keystroke */
#ifdef UNIX
#ifdef NOSETBUF
            fflush(stdout);
#endif /* NOSETBUF */
#endif /* UNIX */
            conbin((char)escape);       /* Put terminal in binary mode */
#ifdef OS2
            os2gks = 0;                 /* Turn off Kverb preprocessing */
#endif /* OS2 */
            kc = congks(0);             /* Get character or scan code */
#ifdef OS2
            os2gks = 1;                 /* Turn on Kverb preprocessing */
#endif /* OS2 */
            concb((char)escape);        /* Restore terminal to cbreak mode */
            if (kc < 0) {               /* Check for error */
                printf("?Error reading key\n");
                return(0);
            }
#ifdef OS2
            shokeycode(kc,-1);          /* Show current definition */
#else
            shokeycode(kc);             /* Show current definition */
#endif /* OS2 */
            flag = 1;                   /* Remember it's a multiline command */
        } else                          /* Error */
          return(y);
    }

    /* Normal SET KEY <scancode> <value> command... */

#ifdef OS2
    if (mskkeys)
      kc = msktock(kc);
#endif /* OS2 */

    if (kc < 0 || kc >= KMSIZE) {
        printf("?key code must be between 0 and %d\n", KMSIZE - 1);
        return(-9);
    }
    if (kc == escape) {
        printf("Sorry, %d is the CONNECT-mode escape character\n",kc);
        return(-9);
    }
#ifdef OS2
    wideresult = -1;
#endif /* OS2 */
    if (flag) {
        cmsavp(psave,PROMPTL);
        cmsetp(" Enter new definition: ");
        cmini(ckxech);
        cmflgs = 0;
        prompt(NULL);
    }
  def_again:
    if (flag)
      cmres();
    if ((y = cmtxt("key definition,\n\
or Ctrl-C to cancel this command,\n\
or Enter to restore default definition",
                   "",&s,NULL)) < 0) {
        if (flag)                       /* Handle parse errors */
          goto def_again;
        else
          return(y);
    }
    s = brstrip(s);
#ifndef NOKVERBS
    p = s;                              /* Save this place */
#endif /* NOKVERBS */
/*
  If the definition included any \Kverbs, quote the backslash so the \Kverb
  will still be in the definition when the key is pressed.  We don't do this
  in zzstring(), because \Kverbs are valid only in this context and nowhere
  else.

  We use this code active for all versions that support SET KEY, even if they
  don't support \Kverbs, because otherwise \K would behave differently for
  different versions.
*/
    for (x = 0, y = 0; s[x]; x++, y++) { /* Convert \K to \\K */
        if ((x > 0) &&
            (s[x] == 'K' || s[x] == 'k')
            ) {                         /* Have K */

            if ((x == 1 && s[x-1] == CMDQ) ||
                (x > 1 && s[x-1] == CMDQ && s[x-2] != CMDQ)) {
                line[y++] = CMDQ;       /* Make it \\K */
            }
            if (x > 1 && s[x-1] == '{' && s[x-2] == CMDQ) {
                line[y-1] = CMDQ;       /* Have \{K */
                line[y++] = '{';        /* Make it \\{K */
            }
        }
        line[y] = s[x];
    }
    line[y++] = NUL;                    /* Terminate */
    s = line + y + 1;                   /* Point to after it */
    x = LINBUFSIZ - (int) strlen(line) - 1; /* Calculate remaining space */
    if ((x < (LINBUFSIZ / 2)) ||
        (zzstring(line, &s, &x) < 0)) { /* Expand variables, etc. */
        printf("?Key definition too long\n");
        if (flag) cmsetp(psave);
        return(-9);
    }
    s = line + y + 1;                   /* Point to result. */

#ifndef NOKVERBS
/*
  Special case: see if the definition starts with a \Kverb.
  If it does, point to it with p, otherwise set p to NULL.
*/
    p = s;
    if (*p++ == CMDQ) {
        if (*p == '{') p++;
        p = (*p == 'k' || *p == 'K') ? p + 1 : NULL;
    }
#endif /* NOKVERBS */

    if (macrotab[kc]) {                 /* Possibly free old macro from key. */
        free((char *)macrotab[kc]);
        macrotab[kc] = NULL;
    }
    switch (strlen(s)) {                /* Action depends on length */
      case 0:                           /* Reset to default binding */
        keymap[kc] = (KEY) kc;
        break;
      case 1:                           /* Single character */
        keymap[kc] = (CHAR) *s;
        break;
      default:                          /* Character string */
#ifndef NOKVERBS
        if (p) {
            y = xlookup(kverbs,p,nkverbs,&x); /* Look it up */
            debug(F101,"set key kverb lookup",0,y); /* exact match required */
            if (y > -1) {
                keymap[kc] = F_KVERB | y;
                break;
            }
        }
#endif /* NOKVERBS */
        keymap[kc] = (KEY) kc;
        macrotab[kc] = (MACRO) malloc(strlen(s)+1);
        if (macrotab[kc])
          strcpy((char *) macrotab[kc], s); /* safe */
        break;
    }
    if (flag) cmsetp(psave);
#ifdef OS2
    initvik = 1;                        /* Update VIK table */
#endif /* OS2 */
    return(1);
}
#endif /* NOSETKEY */

#ifdef STOPBITS
struct keytab stoptbl[] = {
    { "1", 1, 0 },
    { "2", 2, 0 }
};
#endif /* STOPBITS */

static struct keytab sertbl[] = {
    { "7E1", 0, 0 },
    { "7E2", 1, 0 },
    { "7M1", 2, 0 },
    { "7M2", 3, 0 },
    { "7O1", 4, 0 },
    { "7O2", 5, 0 },
    { "7S1", 6, 0 },
    { "7S2", 7, 0 },
#ifdef HWPARITY
    { "8E1", 9, 0 },
    { "8E2", 10, 0 },
#endif /* HWPARITY */
    { "8N1", 8, 0 },
#ifdef HWPARITY
    { "8N2", 11, 0 },
    { "8O1", 12, 0 },
    { "8O2", 13, 0 },
#endif /* HWPARITY */
    { "", 0, 0 }
};
static int nsertbl = (sizeof(sertbl) / sizeof(struct keytab)) - 1;

static char * sernam[] = {              /* Keep this in sync with sertbl[] */
  "7E1", "7E2", "7M1", "7M2", "7O1", "7O2", "7S1", "7S2",
  "8N1", "8E1", "8E2", "8N2", "8O1", "8O2"
};

static struct keytab optstab[] = {      /* SET OPTIONS table */
#ifndef NOFRILLS
    { "delete",    XXDEL,   0},            /* DELETE */
#endif /* NOFRILLS */
    { "directory", XXDIR,   0},         /* DIRECTORY */
#ifdef CKPURGE
    { "purge",     XXPURGE, 0},         /* PURGE */
#endif /* CKPURGE */
    { "type",      XXTYP,   0},         /* TYPE */
    { "", 0, 0}
};
static int noptstab =  (sizeof(optstab) / sizeof(struct keytab)) - 1;

#ifndef NOXFER
/*
  PROTOCOL SELECTION.  Kermit is always available.  If CK_XYZ is defined at
  compile time, then the others become selections also.  In OS/2 and
  Windows, they are integrated and the various SET commands (e.g. "set file
  type") affect them as they would Kermit.  In other OS's (UNIX, VMS, etc),
  they are external protocols which are run via Kermit's REDIRECT mechanism.
  All we do is collect and verify the filenames and pass them along to the
  external protocol.
*/
struct keytab protos[] = {
#ifdef CK_XYZ
    "g",          PROTO_G,  CM_INV,
#endif /* CK_XYZ */
    "kermit",     PROTO_K,  0,
#ifdef CK_XYZ
    "other",      PROTO_O,  0,
    "x",          PROTO_X,  CM_INV|CM_ABR,
    "xmodem",     PROTO_X,  0,
    "xmodem-crc", PROTO_XC, 0,
    "y",          PROTO_Y,  CM_INV|CM_ABR,
    "ymodem",     PROTO_Y,  0,
    "ymodem-g",   PROTO_G,  0,
    "zmodem",     PROTO_Z,  0
#endif /* CK_XYZ */
};
int nprotos =  (sizeof(protos) / sizeof(struct keytab));

#ifndef XYZ_INTERNAL
#ifndef NOPUSH
#define EXP_HANDLER 1
#define EXP_STDERR  2
#define EXP_TIMO    3

static struct keytab extprotab[] = {
    { "handler",          EXP_HANDLER, 0 },
    { "redirect-stderr",  EXP_STDERR, 0 },
    { "timeout",          EXP_TIMO, 0 }
};
static int nxtprotab =  (sizeof(extprotab) / sizeof(struct keytab));
#endif	/* NOPUSH */
#endif	/* XYZ_INTERNAL */

#define XPCMDLEN 71

_PROTOTYP(static int protofield, (char *, char *, char *));
_PROTOTYP(static int setproto, (void));

static int
protofield(current, help, px) char * current, * help, * px; {

    char *s, tmpbuf[XPCMDLEN+1];
    int x;

    if (current)                        /* Put braces around default */
      ckmakmsg(tmpbuf,TMPBUFSIZ,"{",current,"}",NULL);
    else
      tmpbuf[0] = NUL;

    if ((x = cmfld(help, (char *)tmpbuf, &s, xxstring)) < 0)
      return(x);
    if ((int)strlen(s) > XPCMDLEN) {
        printf("?Sorry - maximum length is %d\n", XPCMDLEN);
        return(-9);
    } else if (*s) {
        strcpy(px,s);                   /* safe */
    } else {
        px = NULL;
    }
    return(x);
}

static int
setproto() {                            /* Select a file transfer protocol */
    /* char * s = NULL; */
    int x = 0, y;
    char s1[XPCMDLEN+1], s2[XPCMDLEN+1], s3[XPCMDLEN+1];
    char s4[XPCMDLEN+1], s5[XPCMDLEN+1], s6[XPCMDLEN+1], s7[XPCMDLEN+1];
    char * p1 = s1, * p2 = s2, *p3 = s3;
    char * p4 = s4, * p5 = s5, *p6 = s6, *p7 = s7;

#ifdef XYZ_INTERNAL
    extern int p_avail;
#else
#ifndef CK_REDIR
    x = 1;
#endif /* CK_REDIR */
#endif /* XYZ_INTERNAL */
    s1[0] = NUL;
    s2[0] = NUL;
    s3[0] = NUL;
    s4[0] = NUL;
    s5[0] = NUL;
    s6[0] = NUL;

    if ((y = cmkey(protos,nprotos,"","kermit",xxstring)) < 0)
      return(y);

    if (x && y != PROTO_K) {
        printf(
           "?Sorry, REDIRECT capability required for external protocols.\n");
        return(-9);
    }
    if ((x = protofield(ptab[y].h_b_init,
     "Optional command to send to host prior to uploading in binary mode",
               p1)) < 0) {
        if (x == -3) {
            protocol = y;               /* Set protocol but don't change */
            return(1);                  /* anything else */
        } else
          return(x);
    }
    if ((x = protofield(ptab[y].h_t_init,
     "Optional command to send to host prior to uploading in text mode",
               p2)) < 0) {
        if (x == -3)
          goto protoexit;
        else
          return(x);
    }

    if (y == PROTO_K) {
        if ((x = protofield(ptab[y].h_x_init,
                    "Optional command to send to host to start Kermit server",
                            p3)) < 0) {
            if (x == -3)
              goto protoexit;
            else
              return(x);
        }
    }


#ifndef XYZ_INTERNAL                    /* If XYZMODEM are external... */

    if (y != PROTO_K) {
        if ((x = protofield(ptab[y].p_b_scmd,
                 "External command to SEND in BINARY mode with this protocol",
                            p4)) < 0) {
            if (x == -3)
              goto protoexit;
            else
              return(x);
        }
        if ((x = protofield(ptab[y].p_t_scmd,
                 "External command to SEND in TEXT mode with this protocol",
                            p5)) < 0) {
            if (x == -3)
              goto protoexit;
            else
              return(x);
        }
        if ((x = protofield(ptab[y].p_b_rcmd,
               "External command to RECEIVE in BINARY mode with this protocol",
                            p6)) < 0) {
            if (x == -3)
              goto protoexit;
            else
              return(x);
        }
        if ((x = protofield(ptab[y].p_t_rcmd,
                 "External command to RECEIVE in TEXT mode with this protocol",
                            p7)) < 0) {
            if (x == -3)
              goto protoexit;
            else
              return(x);
        }
    }
#endif /* XYZ_INTERNAL */

    if ((x = cmcfm()) < 0)              /* Confirm the command */
      return(x);

protoexit:                              /* Common exit from this routine */

#ifdef XYZ_INTERNAL
    if (!p_avail) {
        bleep(BP_WARN);
        printf("\n?X,Y, and Zmodem are unavailable\n");
        return(success = 0);
    }
#endif /* XYZ_INTERNAL */

    p1 = brstrip(p1);
    p2 = brstrip(p2);
    p3 = brstrip(p3);
    p4 = brstrip(p4);
    p5 = brstrip(p5);
    p6 = brstrip(p6);
    p7 = brstrip(p7);
    initproto(y,p1,p2,p3,p4,p5,p6,p7);
    return(success = 1);
}

#ifndef NOPUSH
#ifndef XYZ_INTERNAL

#define DEF_EXP_TIMO 12	 /* Default timeout for external protocol (seconds) */

int exp_handler = 0;			/* These are exported */
int exp_timo = DEF_EXP_TIMO;
int exp_stderr = SET_AUTO;

VOID
shoextern() {				/* Invoked by SHOW PROTOCOL */
    printf("\n External-protocol handler:         %s\n",
	   exp_handler ? (exp_handler == 1 ? "pty" : "system") : "automatic");
#ifdef COMMENT
    printf(" External-protocol redirect-stderr: %s\n", showooa(exp_stderr));
#endif	/* COMMENT */
    printf(" External-protocol timeout:         %d (sec)\n", exp_timo);
}

static struct keytab setexternhandler[] = {
    { "automatic", 0, 0 },
    { "pty",       1, 0 },
    { "system",    2, 0 }
};

int
setextern() {				/* SET EXTERNAL-PROTOCOL */
    int x, y;
    if ((x = cmkey(extprotab,nxtprotab,"","",xxstring)) < 0)
      return(x);
    switch (x) {
      case EXP_HANDLER:
	if ((x = cmkey(setexternhandler,3,"","automatic",xxstring)) < 0)
	  return(x);
	if ((y = cmcfm()) < 0)
	  return(y);
	exp_handler = x;
	break;
	
#ifdef COMMENT
      case EXP_STDERR:
	if ((x = cmkey(ooatab,3,"","automatic",xxstring)) < 0)
	  return(x);
	if ((y = cmcfm()) < 0)
	  return(y);
	exp_stderr = x;
	break;
#endif	/* COMMENT */

      case EXP_TIMO:
	y = cmnum("Inactivity timeout, seconds,",ckitoa(DEF_EXP_TIMO),
		  10,&x,xxstring);
        return(setnum(&exp_timo,x,y,-1));
    }
    return(success = 1);
}
#endif	/* XYZ_INTERNAL */
#endif	/* NOPUSH */

int
setdest() {
    int x, y;
    if ((y = cmkey(desttab,ndests,"","disk",xxstring)) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    dest = y;
    return(1);
}
#endif /* NOXFER */

#ifdef DECNET
struct keytab dnettab[] = {
#ifndef OS2ONLY
    "cterm", NP_CTERM, 0,
#endif /* OS2ONLY */
    "lat",   NP_LAT,   0
};
int ndnet =  (sizeof(dnettab) / sizeof(struct keytab));
#endif /* DECNET */

/*  S E T P R I N T E R  --  SET PRINTER command  */

#ifdef PRINTSWI
static struct keytab prntab[] = {       /* SET PRINTER switches */
    "/bidirectional",    PRN_BID, 0,
#ifdef OS2
    "/character-set",    PRN_CS,  CM_ARG,
#endif /* OS2 */
    "/command",          PRN_PIP, CM_ARG,
    "/dos-device",       PRN_DOS, CM_ARG,
    "/end-of-job-string",PRN_TRM, CM_ARG,
    "/file",             PRN_FIL, CM_ARG,
#ifdef BPRINT
    "/flow-control",     PRN_FLO, CM_ARG,
#endif /* BPRINT */
    "/job-header-file",  PRN_SEP, CM_ARG,
#ifdef OS2
    "/length",           PRN_LEN, CM_ARG,
#endif /* OS2 */
    "/none",             PRN_NON, 0,
#ifdef OS2
    "/nopostscript",     PRN_RAW, 0,
    "/nops",             PRN_RAW, CM_INV,
#endif /* OS2 */
    "/output-only",      PRN_OUT, 0,
#ifdef BPRINT
    "/parity",           PRN_PAR, CM_ARG,
#endif /* BPRINT */
    "/pipe",             PRN_PIP, CM_ARG|CM_INV,
#ifdef OS2
    "/postscript",       PRN_PS,  0,
    "/ps",               PRN_PS,  CM_INV,
#endif /* OS2 */
    "/separator",        PRN_SEP, CM_ARG|CM_INV,
#ifdef BPRINT
    "/speed",            PRN_SPD, CM_ARG,
#endif /* BPRINT */
    "/timeout",          PRN_TMO, CM_ARG,
    "/terminator",       PRN_TRM, CM_ARG|CM_INV,
#ifdef OS2
#ifdef NT
    "/w",                PRN_WIN, CM_ARG|CM_ABR|CM_INV,
    "/wi",               PRN_WIN, CM_ARG|CM_ABR|CM_INV,
#endif /* NT */
    "/width",            PRN_WID, CM_ARG,
#endif /* OS2 */
#ifdef NT
    "/windows-queue",    PRN_WIN, CM_ARG,
#endif /* NT */
    "",                 0,      0
};
int nprnswi =  (sizeof(prntab) / sizeof(struct keytab)) - 1;
#endif /* PRINTSWI */

static int
setprinter(xx) int xx; {
    int x, y;
    char * s;
    char * defname = NULL;
#ifdef OS2
    extern int prncs;
#endif /* OS2 */

#ifdef BPRINT
    char portbuf[64];
    long portspeed = 0L;
    int portparity = 0;
    int portflow = 0;
#endif /* BPRINT */

#ifdef PRINTSWI
    int c, i, n, wild, confirmed = 0;   /* Workers */
    int getval = 0;                     /* Whether to get switch value */
    struct stringint pv[PRN_MAX+1];    /* Temporary array for switch values */
    struct FDB sw, of, cm;              /* FDBs for each parse function */
    int haveque = 0;
    int typeset = 0;
#endif /* PRINTSWI */

#ifdef NT
    struct keytab * printtab = NULL, * _printtab = NULL;
    int nprint = 0, printdef=0;
#endif /* NT */

#ifdef OS2
    defname = "PRN";                    /* default */
#else
#ifdef VMS
    defname = "LPT:";
#else
#ifdef UNIX
    defname = "|lpr";
#endif /* UNIX */
#endif /* VMS */
#endif /* OS2 */

#ifdef PRINTSWI
#ifdef NT
    haveque = Win32EnumPrt(&printtab,&_printtab,&nprint,&printdef);
    haveque = haveque && nprint;
#endif /* NT */

    for (i = 0; i <= PRN_MAX; i++) {    /* Initialize switch values */
        pv[i].sval = NULL;              /* to null pointers */
        pv[i].ival = -1;                /* and -1 int values */
        pv[i].wval = (CK_OFF_T)-1;	/* and -1 wide values */
    }
    if (xx == XYBDCP) {                 /* SET BPRINTER == /BIDIRECTIONAL */
        pv[PRN_BID].ival = 1;
        pv[PRN_OUT].ival = 0;
    }

    /* Initialize defaults based upon current printer settings */
    if (printername) {
        defname = printername;
        switch (printertype) {
          case PRT_WIN: pv[PRN_WIN].ival = 1; break;
          case PRT_DOS: pv[PRN_DOS].ival = 1; break;
          case PRT_PIP: pv[PRN_PIP].ival = 1; break;
          case PRT_FIL: pv[PRN_FIL].ival = 1; break;
          case PRT_NON: pv[PRN_NON].ival = 1; break;
        }
    }
#ifdef BPRINT
    /* only set the BIDI flag if we are bidi */
    if (printbidi)
        pv[PRN_BID].ival = 1;

    /* serial port parameters may be set for non-bidi devices */
    pv[PRN_SPD].ival = pportspeed / 10L;
    pv[PRN_PAR].ival = pportparity;
    pv[PRN_FLO].ival = pportflow;
#endif /* BPRINT */
    if (printtimo)
        pv[PRN_TMO].ival = printtimo;
    if (printterm) {
        pv[PRN_TRM].ival = 1;
        makestr(&pv[PRN_TRM].sval,printterm);
    }
    if (printsep) {
        pv[PRN_SEP].ival = 1;
        makestr(&pv[PRN_SEP].sval,printsep);
    }
    if (txt2ps) {
        pv[PRN_PS].ival = 1;
        pv[PRN_WID].ival = ps_width;
        pv[PRN_LEN].ival = ps_length;
    } else {
        pv[PRN_RAW].ival = 1;
    }

    /* Set up chained parse functions... */

    cmfdbi(&sw,                         /* First FDB - command switches */
           _CMKEY,                      /* fcode */
           "Switch",                    /* hlpmsg */
           "",                          /* default */
           "",                          /* addtl string data */
           nprnswi,                     /* addtl numeric data 1: tbl size */
           4,                           /* addtl numeric data 2: 4 = cmswi */
           xxstring,                    /* Processing function */
           prntab,                      /* Keyword table */
           &cm                          /* Pointer to next FDB */
           );
    cmfdbi(&cm,                         /* Second fdb for confirmation */
           _CMCFM,                      /* fcode */
           "",                          /* hlpmsg */
           "",                          /* default */
           "",                          /* addtl string data */
           0,                           /* addtl numeric data 1 */
           0,                           /* addtl numeric data 2 */
           NULL,
           NULL,
           &of
           );
    cmfdbi(&of,                         /* Third FDB for printer name */
           _CMOFI,                      /* fcode */
           "Printer or file name",      /* hlpmsg */
           defname,                     /* default */
           "",                          /* addtl string data */
           0,                           /* addtl numeric data 1: tbl size */
           0,                           /* addtl numeric data 2: 4 = cmswi */
           xxstring,                    /* Processing function */
           NULL,                        /* Nothing */
           NULL
           );

    while (1) {                         /* Parse 0 or more switches */
        x = cmfdb(&sw);                 /* Parse switch or other thing */
        debug(F101,"setprinter cmfdb","",x);
        if (x < 0)                      /* Error */
          goto xsetprn;                 /* or reparse needed */
        if (cmresult.fcode != _CMKEY)   /* Break out if not a switch */
          break;
        if (cmresult.fdbaddr != &sw)    /* Advanced usage :-) */
          break;
        c = cmgbrk();                   /* Get break character */
        getval = (c == ':' || c == '='); /* to see how they ended the switch */
        n = cmresult.nresult;           /* Numeric result = switch value */
        debug(F101,"setprinter switch","",n);

        switch (n) {                    /* Process the switch */
          case PRN_PS:                  /* Text to Postscript */
            pv[PRN_PS].ival = 1;
            pv[PRN_BID].ival = 0;
            pv[PRN_OUT].ival = 1;
            pv[PRN_RAW].ival = 0;
            break;

          case PRN_RAW:                 /* Non-Postscript */
            pv[PRN_PS].ival = 0;
            pv[PRN_RAW].ival = 1;
            break;

          case PRN_BID:                 /* Bidirectional */
            pv[PRN_BID].ival = 1;
            pv[PRN_OUT].ival = 0;
            pv[PRN_PS].ival = 0;
            pv[PRN_RAW].ival = 1;
            break;

          case PRN_OUT:                 /* Output-only */
            pv[PRN_OUT].ival = 1;
            pv[PRN_BID].ival = 0;
            pv[PRN_PS].ival = 0;
            pv[PRN_RAW].ival = 1;
            break;

          case PRN_NON:                 /* NONE */
            typeset++;
            pv[n].ival = 1;
            pv[PRN_SPD].ival = 0;
            pv[PRN_PAR].ival = 0;
            pv[PRN_FLO].ival = FLO_KEEP;
            break;

#ifdef UNIX
          case PRN_WIN:
#endif /* UNIX */
          case PRN_DOS:                 /* DOS printer name */
          case PRN_FIL:                 /* Or filename */
          case PRN_PIP:
            typeset++;
            if (pv[n].sval) free(pv[n].sval);
            pv[n].sval = NULL;
            pv[PRN_NON].ival = 0;       /* Zero any previous selections */
            pv[PRN_WIN].ival = 0;
            pv[PRN_DOS].ival = 0;
            pv[PRN_FIL].ival = 0;
            pv[PRN_PIP].ival = 0;
            pv[n].ival = 1;             /* Flag this one */
            if (!getval) break;         /* No value wanted */

            if (n == PRN_FIL) {         /* File, check accessibility */
                int wild = 0;
                if ((x = cmiofi("Filename","kermit.prn",&s,&wild,xxstring))< 0)
                  if (x == -9) {
                      if (zchko(s) < 0) {
                          printf("Can't create \"%s\"\n",s);
                          return(x);
                      }
                  } else goto xsetprn;
                if (iswild(s)) {
                    printf("?A single file please\n");
                    return(-9);
                }
                pv[PRN_SPD].ival = 0;
                pv[PRN_PAR].ival = 0;
                pv[PRN_FLO].ival = FLO_KEEP;
            } else if ((x = cmfld(n == PRN_DOS ? /* Value wanted - parse it */
                           "DOS printer device name" : /* Help message */
                           (n == PRN_PIP ?
                           "Program name" :
                           "Filename"),
                           n == PRN_DOS ?
                           "PRN" :      /* Default */
                           "",
                           &s,
                           xxstring
                           )) < 0)
              goto xsetprn;
            s = brstrip(s);             /* Strip enclosing braces */
            while (*s == SP)            /* Strip leading blanks */
              s++;
            if (n == PRN_PIP) {         /* If /PIPE: */
                if (*s == '|') {        /* strip any extraneous pipe sign */
                    s++;
                    while (*s == SP)
                      s++;
                }
                pv[PRN_SPD].ival = 0;
                pv[PRN_PAR].ival = 0;
                pv[PRN_FLO].ival = FLO_KEEP;
            }
            if ((y = strlen(s)) > 0)    /* Anything left? */
              if (pv[n].sval = (char *) malloc(y+1)) /* Yes, keep it */
                strcpy(pv[n].sval,s);   /* safe */
            break;
#ifdef NT
          case PRN_WIN:                 /* Windows queue name */
            typeset++;
            if (pv[n].sval) free(pv[n].sval);
            pv[n].sval = NULL;
            pv[PRN_NON].ival = 0;
            pv[PRN_DOS].ival = 0;
            pv[PRN_FIL].ival = 0;
            pv[n].ival = 1;
            pv[PRN_SPD].ival = 0;
            pv[PRN_PAR].ival = 0;
            pv[PRN_FLO].ival = FLO_KEEP;

            if (!getval || !haveque)
              break;
            if ((x = cmkey(_printtab,nprint,"Print queue name",
                           _printtab[printdef].kwd,xxstring)) < 0) {
                if (x != -2)
                  goto xsetprn;

                if (pv[PRN_WIN].sval) free(pv[PRN_WIN].sval);
                s = atmbuf;
                if ((y = strlen(s)) > 0)
                  if (pv[n].sval = (char *)malloc(y+1))
                    strcpy(pv[n].sval,s); /* safe */
            } else {
                if (pv[PRN_WIN].sval) free(pv[PRN_WIN].sval);
                for (i = 0; i < nprint; i++) {
                    if (x == printtab[i].kwval) {
                        s = printtab[i].kwd;
                        break;
                    }
                }
                if ((y = strlen(s)) > 0)
                  if (pv[n].sval = (char *)malloc(y+1))
                    strcpy(pv[n].sval,s); /* safe */
            }
            break;
#endif /* NT */

          case PRN_SEP:                 /* /JOB-HEADER (separator) */
            if (pv[n].sval) free(pv[n].sval);
            pv[n].sval = NULL;
            pv[n].ival = 1;
            if (!getval) break;
            if ((x = cmifi("Filename","",&s,&y,xxstring)) < 0)
              goto xsetprn;
            if (y) {
                printf("?Wildcards not allowed\n");
                x = -9;
                goto xsetprn;
            }
            if ((y = strlen(s)) > 0)
              if (pv[n].sval = (char *) malloc(y+1))
                strcpy(pv[n].sval,s);   /* safe */
            break;

          case PRN_TMO:                 /* /TIMEOUT:number */
            pv[n].ival = 0;
            if (!getval) break;
            if ((x = cmnum("Seconds","0",10,&y,xxstring)) < 0)
              goto xsetprn;
            if (y > 999) {
                printf("?Sorry - 999 is the maximum\n");
                x = -9;
                goto xsetprn;
            } else
              pv[n].ival = y;
            break;

          case PRN_TRM:                 /* /END-OF-JOB:string */
            if (pv[n].sval) free(pv[n].sval);
            pv[n].sval = NULL;
            pv[n].ival = 1;
            if (!getval) break;
            if ((x = cmfld("String (enclose in braces if it contains spaces)",
                           "",&s,xxstring)) < 0)
              goto xsetprn;
            s = brstrip(s);
            if ((y = strlen(s)) > 0)
              if (pv[n].sval = (char *) malloc(y+1))
                strcpy(pv[n].sval,s);   /* safe */
            break;

#ifdef BPRINT
          case PRN_FLO:
            if (!getval) break;
            if ((x = cmkey(flotab,nflo,
                              "Serial printer-port flow control",
                              "rts/cts",xxstring)) < 0)
              goto xsetprn;
            pv[n].ival = x;
            break;

#ifndef NOLOCAL
          case PRN_SPD:
            if (!getval) break;

            /* TN_COMPORT here too? */

            if ((x = cmkey(spdtab,      /* Speed (no default) */
                           nspd,
                           "Serial printer-port interface speed",
                           "9600",
                           xxstring)
                 ) < 0)
              goto xsetprn;
            pv[n].ival = x;
            break;
#endif /* NOLOCAL */

          case PRN_PAR:
            pv[n].ival = 0;
            if (!getval) break;
            if ((x = cmkey(partbl,npar,"Serial printer-port parity",
                           "none",xxstring)) < 0)
              goto xsetprn;
            pv[n].ival = x;
            break;
#endif /* BPRINT */

#ifdef OS2
          case PRN_LEN:
            if (!getval) break;
            if ((x = cmnum("PS page length", "66",10,&y,xxstring)) < 0)
              goto xsetprn;
            pv[n].ival = y;
            break;

          case PRN_WID:
            if (!getval) break;
            if ((x = cmnum("PS page width", "80",10,&y,xxstring)) < 0)
              goto xsetprn;
            pv[n].ival = y;
            break;

          case PRN_CS:
              pv[n].ival = 0;
              if (!getval) break;
              if ((y = cmkey(
#ifdef CKOUNI
                       txrtab,ntxrtab,
#else /* CKOUNI */
                       ttcstab,ntermc,
#endif /* CKOUNI */
                       "auto-print/printscreen character-set",
                       "cp437",xxstring)) < 0)
                  goto xsetprn;
              pv[n].ival = y;
              break;
#endif /* OS2 */

          default:
            printf("?Unexpected switch value - %d\n",cmresult.nresult);
            x = -9;
            goto xsetprn;
        }
    }
    line[0] = NUL;                      /* Initialize printer name value */
    switch (cmresult.fcode) {           /* How did we get here? */
      case _CMOFI:                      /* They typed a filename */
        ckstrncpy(line,cmresult.sresult,LINBUFSIZ); /* Name */
        wild = cmresult.nresult;        /* Wild flag */
        if (!typeset) {                 /* A printer name without a type */
            pv[PRN_NON].ival = 0;       /* is supposed to be treated as  */
            pv[PRN_WIN].ival = 0;       /* a DOS or Pipe printer.  We    */
            pv[PRN_FIL].ival = 0;       /* clear all the flags and let   */
            pv[PRN_PIP].ival = 0;       /* the code below dope out the   */
            pv[PRN_DOS].ival = 0;       /* type.                         */
        }
#ifdef NT
        else if (pv[PRN_WIN].ival && lookup(_printtab,line,nprint,&y)) {
            /* invalid Window Queue name */
            printf("?invalid Windows Printer Queue name: \"%s\"\r\n",line);
            x = -9;
            goto xsetprn;
        }
#endif /* NT */
        if ((x = cmcfm()) < 0)          /* Confirm the command */
          goto xsetprn;
        break;
      case _CMCFM:                      /* They entered the command */
        if (pv[PRN_DOS].ival > 0)
          ckstrncpy(line,pv[PRN_DOS].sval ? pv[PRN_DOS].sval : "",LINBUFSIZ);
        else if (pv[PRN_WIN].ival > 0)
          ckstrncpy(line,pv[PRN_WIN].sval ? pv[PRN_WIN].sval : "",LINBUFSIZ);
        else if (pv[PRN_FIL].ival > 0)
          ckstrncpy(line,pv[PRN_FIL].sval ? pv[PRN_FIL].sval : "",LINBUFSIZ);
        else if (pv[PRN_PIP].ival > 0)
          ckstrncpy(line,pv[PRN_PIP].sval ? pv[PRN_PIP].sval : "",LINBUFSIZ);
        break;
      default:                          /* By mistake */
        printf("?Unexpected function code: %d\n",cmresult.fcode);
        x = -9;
        goto xsetprn;
    }

#else  /* No PRINTSWI */

    if ((x = cmofi("Printer or file name",defname,&s,xxstring)) < 0)
      return(x);
    if (x > 1) {
        printf("?Directory names not allowed\n");
        return(-9);
    }
    while (*s == SP || *s == HT) s++;   /* Trim leading whitespace */
    ckstrncpy(line,s,LINBUFSIZ);        /* Make a temporary safe copy */
    if ((x = cmcfm()) < 0) return(x);   /* Confirm the command */
#endif /* PRINTSWI */

#ifdef IKSD
    if (inserver && (isguest
#ifndef NOSERVER
                     || !ENABLED(en_pri)
#endif /* NOSERVER */
                     )) {
        printf("Sorry, printing disabled\r\n");
        return(success = 0);
    }
#endif /* ISKD */

#ifdef PRINTSWI
#ifdef BPRINT
    if (printbidi) {                    /* If bidi printing active */
#ifndef UNIX
        bprtstop();                     /* Stop it before proceeding */
#endif /* UNIX */
        printbidi = 0;
    }
    if (pv[PRN_SPD].ival > 0) {
        portspeed = (long) pv[PRN_SPD].ival * 10L;
        if (portspeed == 70L) portspeed = 75L;
    }
    if (pv[PRN_PAR].ival > 0)
        portparity = pv[PRN_PAR].ival;
    if (pv[PRN_FLO].ival > 0)
        portflow = pv[PRN_FLO].ival;
#endif /* BPRINT */
#endif /* PRINTSWI */

    s = line;                           /* Printer name, if given */

#ifdef OS2ORUNIX
#ifdef PRINTSWI
    if (pv[PRN_PIP].ival > 0) {         /* /PIPE was given? */
        printpipe = 1;
        noprinter = 0;
        if (*s ==  '|') {               /* It might still have a pipe sign */
            s++;                        /* if name give later */
            while (*s == SP)            /* so remove it and spaces */
              s++;
        }
    } else
#endif /* PRINTSWI */
      if (*s == '|') {                  /* Or pipe implied by name? */
          s++;                          /* Point past pipe sign */
          while (*s == SP)              /* Gobble whitespace */
            s++;
          if (*s) {
              printpipe = 1;
              noprinter = 0;
          }
      } else {
          printpipe = 0;
      }

#ifdef PRINTSWI
#ifdef BPRINT
    if (printpipe && pv[PRN_BID].ival > 0) {
        printf("?Sorry, pipes not allowed for bidirectional printer\n");
        return(-9);
    }
#endif /* BPRINT */
#endif /* PRINTSWI */
#endif /* OS2ORUNIX */

#ifdef OS2
    if ( pv[PRN_CS].ival > 0 ) 
        prncs = pv[PRN_CS].ival;

    if ( pv[PRN_PS].ival > 0 ) {
        txt2ps = 1;
        ps_width = pv[PRN_WID].ival <= 0 ? 80 : pv[PRN_WID].ival;
        ps_length = pv[PRN_LEN].ival <= 0 ? 66 : pv[PRN_LEN].ival;
    }
#endif /* OS2 */

    y = strlen(s);                      /* Length of name of new print file */
    if (y > 0
#ifdef OS2
        && ((y != 3) || (ckstrcmp(s,"PRN",3,0) != 0))
#endif /* OS2 */
        ) {
        if (printername) {              /* Had a print file before? */
            free(printername);          /* Remove its name */
            printername = NULL;
        }
        printername = (char *) malloc(y + 1); /* Allocate space for it */
        if (!printername) {
            printf("?Memory allocation failure\n");
            return(-9);
        }
        strcpy(printername,s);          /* (safe) Copy new name to new space */
        debug(F110,"printername",printername,0);
    }

#ifdef PRINTSWI
    /* Set printer type from switches that were given explicitly */

    if (pv[PRN_NON].ival > 0) {         /* No printer */
        printertype = PRT_NON;
        noprinter = 1;
        printpipe = 0;
    } else if (pv[PRN_FIL].ival > 0) {  /* File */
        printertype = PRT_FIL;
        noprinter = 0;
        printpipe = 0;
    } else if (pv[PRN_PIP].ival > 0) {  /* Pipe */
        printertype = PRT_PIP;
        noprinter = 0;
        printpipe = 1;
    } else if (pv[PRN_WIN].ival > 0) {  /* Windows print queue */
        printertype = PRT_WIN;
        noprinter = 0;
        printpipe = 0;
    } else if (pv[PRN_DOS].ival > 0) {  /* DOS device */
        printertype = PRT_DOS;
        noprinter = 0;
        printpipe = 0;
    } else if (line[0]) {               /* Name given without switches */
        noprinter = 0;
        printertype = printpipe ? PRT_PIP : PRT_DOS;
#ifdef NT
        /* was the command SET PRINTER windows-queue ? */
        y = lookup(_printtab,line,nprint,&x);
        if (y >= 0) {
            printertype = PRT_WIN;
            if (pv[PRN_WIN].sval) free(pv[PRN_WIN].sval);
            if (printername) {          /* Had a print file before? */
                free(printername);      /* Remove its name */
                printername = NULL;
            }
            pv[PRN_WIN].sval = NULL;
            pv[PRN_WIN].ival = 1;
            s = printtab[x].kwd;        /* Get full new name */
            if ((y = strlen(s)) > 0) {
                makestr(&pv[PRN_WIN].sval,s);
                makestr(&printername,s);
                if (!printername) {
                    printf("?Memory allocation failure\n");
                    return(-9);
                }
                debug(F110,"printername",printername,0);
            }
        } else if ( y == -2 ) {
            /* Ambiguous Print Queue Name */
            printf("?Ambiguous printer name provided.\n");
            return(-9);
        }
#endif /* NT */
    }

#ifdef BPRINT
    /* Port parameters may be set for non-bidi mode */

    pportspeed = portspeed;             /* Set parameters */
    pportparity = portparity;
    pportflow = portflow;

    if (pv[PRN_BID].ival > 0) {         /* Bidirectional */
#ifdef UNIX
        printbidi = 1;                  /* (just to test parsing...) */
#else
        printbidi = bprtstart();        /* Start bidirectional printer */
#endif /* UNIX */
        return(success = printbidi);
    } else
      printbidi = 0;                    /* Not BPRINTER, unset flag */
#endif /* BPRINT */

    if (pv[PRN_TMO].ival > -1) {        /* Take care of timeout */
        printtimo = pv[PRN_TMO].ival;
    }
    if (pv[PRN_TRM].ival > 0) {         /* Termination string */
        if (printterm) {
            free(printterm);
            printterm = NULL;
        }
        if (pv[PRN_TRM].sval)
          makestr(&printterm,pv[PRN_TRM].sval);
    }
    if (pv[PRN_SEP].ival > 0) {         /* and separator file */
        if (printsep) {
            free(printsep);
            printsep = NULL;
        }
        if (pv[PRN_SEP].sval)
          makestr(&printsep,pv[PRN_SEP].sval);
    }
#endif /* PRINTSWI */

#ifdef UNIXOROSK
    if (!printpipe
#ifdef PRINTSWI
        && !noprinter
#endif /* PRINTSWI */
        ) {                             /* File - check access */
        if (zchko(s) < 0) {
            printf("?Access denied - %s\n",s);
            x = -9;
            goto xsetprn;
        }
    }
#endif /* UNIXOROSK */

    x = 1;                              /* Return code */

  xsetprn:                              /* Common exit */
#ifdef PRINTSWI
    for (i = 0; i <= PRN_MAX; i++) {    /* Free malloc'd memory */
        if (pv[i].sval)
          free(pv[i].sval);
    }
#endif /* PRINTSWI */
    success = (x > 0) ? 1 : 0;
    return(x);
}

#ifdef ANYSSH
/* The SET SSH command */

#define SSH_CMD  1                      /* SET SSH COMMAND */

#ifdef SSHBUILTIN                       /* Built-in SET SSH options */
#define SSH_ADD  2                      /* Add */
#define SSH_AFW  3                      /* Agent-forwarding */
#define SSH_CHI  4                      /* Check Host IP */
#define SSH_XFW  5                      /* X11-forwarding */
#define SSH_DYF  6                      /* Dynamic forwarding */
#define SSH_GWP  7                      /* Gatewa portgs */
#define SSH_GSS  8                      /* GSSAPI */
#define SSH_KBD  9                      /* KBD Interactive Devices */
#define SSH_K4  10                      /* Kerberos 4 */
#define SSH_K5  11                      /* Kerberos 5 */
#define SSH_SHK 12                      /* Strict Host Key Check */
#define SSH_V1  13                      /* SSH V1 */
#define SSH_V2  14                      /* SSH V2 */
#define SSH_PRP 15                      /* Privd port */
#define SSH_CMP 16                      /* Compression */
#define SSH_XAL 17                      /* X Auth Location */
#define SSH_SHH 18                      /* Quiet */
#define SSH_VER 19                      /* Version */
#define SSH_VRB 20                      /* Verbosity level */
#define SSH_IDF 21                      /* Identity File */
#define SSH_CFG 22                      /* Use OpenSSH Config */
#define SSH_HBT 23                      /* Heartbeat Interval */
#endif /* SSHBUILTIN */

static struct keytab sshtab[] = {       /* SET SSH command table */
#ifdef SSHBUILTIN
    { "agent-forwarding",        SSH_AFW,  0 },
    { "check-host-ip",           SSH_CHI,  0 },
    { "compression",             SSH_CMP,  0 },
    { "dynamic-forwarding",      SSH_DYF,  0 },
    { "gateway-ports",           SSH_GWP,  0 },
    { "gssapi",                  SSH_GSS,  0 },
    { "heartbeat-interval",      SSH_HBT,  0 },
    { "identity-file",           SSH_IDF,  0 },
#ifdef COMMENT
    { "kbd-interactive-devices", SSH_KBD,  0 },
#endif /* COMMENT */
    { "k4",                      SSH_K4, CM_INV },
    { "k5",                      SSH_K5, CM_INV },
    { "kerberos4",               SSH_K4,   0 },
    { "kerberos5",               SSH_K5,   0 },
    { "krb4",                    SSH_K4, CM_INV },
    { "krb5",                    SSH_K5, CM_INV },
    { "privileged-port",         SSH_PRP,  0 },
    { "quiet",                   SSH_SHH,  0 },
    { "strict-host-key-check",   SSH_SHK,  0 },
    { "use-openssh-config",      SSH_CFG,  0 },
    { "v1",                      SSH_V1,   0 },
    { "v2",                      SSH_V2,   0 },
    { "verbose",                 SSH_VRB,  0 },
    { "version",                 SSH_VER,  0 },
    { "x11-forwarding",          SSH_XFW,  0 },
    { "xauth-location",          SSH_XAL,  0 },
#else
#ifdef SSHCMD
    { "command",                 SSH_CMD,  0 },
#endif /* SSHCMD */
#endif /* SSHBUILTIN */
    { "", 0, 0 }
};
static int nsshtab = (sizeof(sshtab) / sizeof(struct keytab)) - 1;

#ifdef SSHBUILTIN
static struct keytab sshver[] = {       /* SET SSH VERSION command table */
    { "1",          1,  0 },
    { "2",          2,  0 },
    { "automatic",  0,  0 }
};

#define SSHA_CRS   1
#define SSHA_DSA   2
#define SSHA_GSS   3
#define SSHA_HOS   4
#define SSHA_KBD   5
#define SSHA_K4    6
#define SSHA_K5    7
#define SSHA_PSW   8
#define SSHA_PK    9
#define SSHA_SKE  10
#define SSHA_TIS  11
#define SSHA_EXT  12
#define SSHA_SRP  13

static struct keytab ssh2aut[] = {      /* SET SSH V2 AUTH command table */
    { "external-keyx",      SSHA_EXT, 0 },
    { "gssapi",             SSHA_GSS, 0 },
    { "hostbased",          SSHA_HOS, 0 },
    { "keyboard-interactive",  SSHA_KBD, 0 },
    { "password",           SSHA_PSW, 0 },
    { "publickey",          SSHA_PK,  0 },
    { "srp-gex-sha1",       SSHA_SRP, 0 },
    { "", 0, 0 }
};
static int nssh2aut = (sizeof(ssh2aut) / sizeof(struct keytab)) - 1;

#define SSHF_LCL   1
#define SSHF_RMT   2

static struct keytab addfwd[] = {       /* SET SSH ADD command table */
    { "local-port-forward",  SSHF_LCL, 0 },
    { "remote-port-forward", SSHF_RMT, 0 },
    { "", 0, 0 }
};
static int naddfwd = (sizeof(addfwd) / sizeof(struct keytab)) - 1;

#define SSH1_CIF   1
#define SSH1_GNH   2
#define SSH1_UNH   3
#define SSH1_K54   4

#define SSH2_CIF   1
#define SSH2_GNH   2
#define SSH2_UNH   3
#define SSH2_ARK   4
#define SSH2_HKA   5
#define SSH2_MAC   6
#define SSH2_AUT   7

static struct keytab sshv1tab[] = {     /* SET SSH V1 command table */
    { "cipher",                  SSH1_CIF, 0 },
    { "global-known-hosts-file", SSH1_GNH, 0 },
    { "k5-reuse-k4-messages",    SSH1_K54, CM_INV },
    { "user-known-hosts-file",   SSH1_UNH, 0 },
    { "", 0, 0 }
};
static int nsshv1tab = (sizeof(sshv1tab) / sizeof(struct keytab)) - 1;

static struct keytab sshv2tab[] = {     /* SET SSH V2 command table */
    { "authentication",          SSH2_AUT, 0 },
    { "auto-rekey",              SSH2_ARK, 0 },
    { "ciphers",                 SSH2_CIF, 0 },
    { "global-known-hosts-file", SSH2_GNH, 0 },
    { "hostkey-algorithms",      SSH2_HKA, 0 },
    { "macs",                    SSH2_MAC, 0 },
    { "user-known-hosts-file",   SSH2_UNH, 0 },
    { "", 0, 0 }
};
static int nsshv2tab = (sizeof(sshv2tab) / sizeof(struct keytab)) - 1;

#define SSHC_3DES 1                     /* 3DES */
#define SSHC_3CBC 2                     /* 3DES-CBC */
#define SSHC_A128 3                     /* AES128-CBC */
#define SSHC_A192 4                     /* AES192-CBC */
#define SSHC_A256 5                     /* AES256-CBC */
#define SSHC_ARC4 6                     /* ARCFOUR */
#define SSHC_FISH 7                     /* BLOWFISH */
#define SSHC_BCBC 9                     /* BLOWFISH-CBC */
#define SSHC_C128 8                     /* CAST128-CBC */
#define SSHC_1DES 10                    /* DES */

static struct keytab ssh1ciphers[] = {
    { "3des",         SSHC_3DES, 0 },
    { "blowfish",     SSHC_FISH, 0 },
    { "des",          SSHC_1DES, 0 },
    { "", 0, 0 }
};
static int nssh1ciphers = (sizeof(ssh1ciphers) / sizeof(struct keytab)) - 1;

static struct keytab ssh2ciphers[] = {  /* SET SSH V2 CIPHERS command table */
    { "3des-cbc",        SSHC_3DES, 0 },
    { "aes128-cbc",      SSHC_A128, 0 },
    { "aes192-cbc",      SSHC_A192, 0 },
    { "aes256-cbc",      SSHC_A256, 0 },
    { "arcfour",         SSHC_ARC4, 0 },
    { "blowfish-cbc",    SSHC_FISH, 0 },
    { "cast128-cbc",     SSHC_C128, 0 },
    { "rijndael128-cbc", SSHC_A128, 0 },
    { "rijndael192-cbc", SSHC_A192, 0 },
    { "rijndael256-cbc", SSHC_A256, 0 },
    { "", 0, 0 }
};
static int nssh2ciphers = (sizeof(ssh2ciphers) / sizeof(struct keytab)) - 1;

#define SSHM_SHA        1               /* HMAC-SHA1 */
#define SSHM_SHA_96     2               /* HMAC-SHA1-96 */
#define SSHM_MD5        3               /* HMAC-MD5 */
#define SSHM_MD5_96     4               /* HMAC-MD5-96 */
#define SSHM_RIPE       5               /* HMAC-RIPEMD160 */

static struct keytab ssh2macs[] = {     /* SET SSH V2 MACS command table */
    { "hmac-md5",       SSHM_MD5,    0 },
    { "hmac-md5-96",    SSHM_MD5_96, 0 },
    { "hmac-ripemd160", SSHM_RIPE,   0 },
    { "hmac-sha1",      SSHM_SHA,    0 },
    { "hmac-sha1-96",   SSHM_SHA_96, 0 },
    { "", 0, 0 }
};
static int nssh2macs = (sizeof(ssh2macs) / sizeof(struct keytab)) - 1;

static struct keytab tgtpass[] = {
    { "tgt-passing", 1, 0, },
    { "", 0, 0 }
};
static int ntgtpass = (sizeof(tgtpass) / sizeof(struct keytab)) - 1;

static struct keytab gssapitab[] = {
    { "delegate-credentials", 1, 0, },
    { "key-exchange",         2, CM_INV, },
    { "", 0, 0 }
};
static int ngssapitab = (sizeof(gssapitab) / sizeof(struct keytab)) - 1;

#define HKA_RSA 1
#define HKA_DSS 2

static struct keytab hkatab[] = {
    { "ssh-dss", HKA_DSS, 0, },
    { "ssh-rsa", HKA_RSA, 0, },
    { "", 0, 0 }
};
static int nhkatab = (sizeof(hkatab) / sizeof(struct keytab)) - 1;

int                                     /* SET SSH variables */
  ssh_afw = 0,                          /* agent forwarding */
  ssh_xfw = 0,                          /* x11 forwarding   */
  ssh_prp = SET_OFF,                    /* privileged ports */
  ssh_cmp = 1,                          /* compression */
  ssh_shh = 0,                          /* quiet       */
  ssh_ver = 0,                          /* protocol version (auto,1,2) */
  ssh_vrb = 2,                          /* Report errors */
  ssh_chkip = 0,                        /* SSH Check Host IP flag */
  ssh_gwp = 0,                          /* gateway ports */
  ssh_dyf = 0,                          /* dynamic forwarding */
  ssh_gsd = 0,                          /* gssapi delegate credentials */
  ssh_k4tgt = 0,                        /* k4 tgt passing */
  ssh_k5tgt = 0,                        /* k5 tgt passing */
  ssh_shk = 2,                          /* Strict host key (no, yes, ask) */
  ssh2_ark = 1,                         /* Auto re-key */
  ssh_cas = 0,                          /* command as subsys */
  ssh_cfg = 0,                          /* use OpenSSH config? */
  ssh_gkx = 1,                          /* gssapi key exchange */
  ssh_k5_is_k4 = 1,                     /* some SSH v1 use same codes */
  ssh_hbt = 0,                          /* heartbeat (seconds) */
  ssh_dummy = 0;                        /* bottom of list */

char                                    /* The following are to be malloc'd */
  * ssh1_cif = NULL,                    /* v1 cipher */
  * ssh2_cif = NULL,                    /* v2 cipher list */
  * ssh2_mac = NULL,                    /* v2 mac list */
  * ssh2_auth = NULL,                   /* v2 authentication list */
  * ssh_hst = NULL,                     /* hostname */
  * ssh_prt = NULL,                     /* port/service */
  * ssh_cmd = NULL,                     /* command to execute */
  * ssh_xal = NULL,                     /* xauth-location */
  * ssh1_gnh = NULL,                    /* v1 global known hosts file */
  * ssh1_unh = NULL,                    /* v1 user known hosts file */
  * ssh2_gnh = NULL,                    /* v2 global known hosts file */
  * ssh2_unh = NULL,                    /* v2 user known hosts file */
  * ssh2_hka = NULL,                    /* Host Key Algorithms */
  * xxx_dummy = NULL;

char * ssh_idf[32] = {                  /* Identity file list */
  NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
  NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
  NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
  NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL
};
char * ssh_tmp[32] = {                  /* Temp identity file list */
  NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
  NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
  NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
  NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL
};
int ssh_idf_n = 0;

extern int    ssh_pf_lcl_n,
              ssh_pf_rmt_n;
extern struct ssh_pf ssh_pf_lcl[32];    /* Port forwarding structs */
extern struct ssh_pf ssh_pf_rmt[32];    /* (declared in ckuusr.c) */
#endif /* SSHBUILTIN */

#ifdef SFTP_BUILTIN
static struct keytab sftptab[] = {
    { "end-of-line",            XY_SFTP_EOL, 0, },
    { "remote-character-set",   XY_SFTP_RCS, 0, },
    { "", 0, 0 }
};
static int nsftptab = (sizeof(sftptab) / sizeof(struct keytab)) - 1;
#endif /* SFTP_BUILTIN */

VOID
shossh() {
#ifdef SSHBUILTIN
    int i, n = 0;                       /* ADD askmore()! */

    printf("\nSSH is built in:\n\n");

    printf(" ssh host:                        %s\n",showstring(ssh_hst));
    printf(" ssh port:                        %s\n",showstring(ssh_prt));
    printf(" ssh command:                     %s\n",showstring(ssh_cmd));
    printf(" ssh agent-forwarding:            %s\n",showoff(ssh_afw));
    printf(" ssh check-host-ip:               %s\n",showoff(ssh_chkip));
    printf(" ssh compression:                 %s\n",showoff(ssh_cmp));
    printf(" ssh dynamic-forwarding:          %s\n",showoff(ssh_dyf));
    if (ssh_pf_lcl[0].p1 && ssh_pf_lcl[0].host && ssh_pf_lcl[0].p2) {
      printf(" ssh forward-local-port:          %d %s %d\n",
             ssh_pf_lcl[0].p1, ssh_pf_lcl[0].host, ssh_pf_lcl[0].p2);
      for ( n=1;n<ssh_pf_lcl_n;n++ )
        printf("                       :          %d %s %d\n",
               ssh_pf_lcl[n].p1, ssh_pf_lcl[n].host, ssh_pf_lcl[n].p2);
    } else
      printf(" ssh forward-local-port:         (none)\n");
    if (ssh_pf_rmt[0].p1 && ssh_pf_rmt[0].host && ssh_pf_rmt[0].p2) {
      printf(" ssh forward-remote-port:         %d %s %d\n",
             ssh_pf_rmt[0].p1, ssh_pf_rmt[0].host, ssh_pf_rmt[0].p2);
      for ( n=1;n<ssh_pf_rmt_n;n++ )
        printf("                        :         %d %s %d\n",
               ssh_pf_rmt[n].p1, ssh_pf_rmt[n].host, ssh_pf_rmt[n].p2);
    } else
      printf(" ssh forward-remote-port:        (none)\n");
    printf(" ssh gateway-ports:               %s\n",showoff(ssh_gwp));
    printf(" ssh gssapi delegate-credentials: %s\n",showoff(ssh_gsd));
    printf(" ssh gssapi key-exchange        : %s\n",showoff(ssh_gkx));
    printf(" ssh identity-file:               %d\n",ssh_idf_n);
    for (i = 0; i < ssh_idf_n; i++)
      printf("  %2d. %s\n",i+1,showstring(ssh_idf[i]));
    printf(" ssh heartbeat interval:          %d\n", ssh_hbt);
    printf(" ssh k4 tgt-passing:              %s\n",showoff(ssh_k4tgt));
    printf(" ssh k5 tgt-passing:              %s\n",showoff(ssh_k5tgt));

    printf(" ssh privileged-port:             %s\n",showooa(ssh_prp));
    printf(" ssh quiet:                       %s\n",showoff(ssh_shh));
    printf(" ssh strict-host-key-check:       %d\n",ssh_shk);
    printf(" ssh use-openssh-config:          %s\n",showoff(ssh_cfg));
    printf(" ssh verbose:                     %d\n",ssh_vrb);
    printf(" ssh version:                     %s\n",
           ssh_ver ? ckitoa(ssh_ver) : "automatic"
           );
    printf(" ssh x11-forwarding:              %s\n",showooa(ssh_xfw));
    printf(" ssh xauth-location:              %s\n",showstring(ssh_xal));
    printf("\n");
    printf(" ssh v1 cipher:                   %s\n",showstring(ssh1_cif));
    printf(" ssh v1 global-known-hosts-file:  %s\n",showstring(ssh1_gnh));
    printf(" ssh v1 user-known-hosts-file:    %s\n",showstring(ssh1_unh));
    printf("\n");
    printf(" ssh v2 authentication:           %s\n",showstring(ssh2_auth));
    printf(" ssh v2 auto-rekey:               %s\n",showoff(ssh2_ark));
    printf(" ssh v2 ciphers:                  %s\n",showstring(ssh2_cif));
    printf(" ssh v2 command-as-subsystem:     %s\n",showoff(ssh_cas));
    printf(" ssh v2 global-known-hosts-file:  %s\n",showstring(ssh2_gnh));
    printf(" ssh v2 hostkey-algorithms:       %s\n",showstring(ssh2_hka));
    printf(" ssh v2 mac:                      %s\n",showstring(ssh2_mac));
    printf(" ssh v2 user-known-hosts-file:    %s\n",showstring(ssh2_unh));
#else
#ifdef SSHCMD
    extern char * sshcmd, * defsshcmd;
    char * s;
    s = sshcmd ? sshcmd : defsshcmd;
    printf("\n SSH is external.\n\n");
    printf(" ssh command: %s\n",showstring(s));
#endif /* SSHCMD */
#endif /* SSHBUILTIN */
    printf("\n");
}

static int
dosetssh() {
#ifdef SSHCMD
    extern char * sshcmd;
#endif /* SSHCMD */
#ifdef SSHBUILTIN
#ifndef SSHTEST
    extern int sl_ssh_xfw_saved, sl_ssh_ver_saved;
#endif /* SSHTEST */
#endif /* SSHBUILTIN */
    int cx, x, y, z;
    char * s;

    if ((cx = cmkey(sshtab,nsshtab,"","command", xxstring)) < 0)
      return(cx);
    switch (cx) {
#ifdef SSHCMD
      case SSH_CMD:                     /* Command */
        if ((x = cmtxt("Command to start ssh","ssh -e none",
                       &s,xxstring)) < 0)
          return(x);
        makestr(&sshcmd,s);
        return(success = 1);
#endif /* SSHCMD */

#ifdef SSHBUILTIN
      case SSH_AFW:                     /* Agent-forwarding */
        return(success = seton(&ssh_afw));

      case SSH_CHI:                     /* Check Host IP */
        return(success = seton(&ssh_chkip));
        break;

      case SSH_CMP:                     /* Compression */
        return(success = seton(&ssh_cmp));

      case SSH_DYF:                     /* Dynamic Forwarding */
        return(success = seton(&ssh_dyf));

      case SSH_GWP:                     /* Gateway ports */
        return(success = seton(&ssh_gwp));

      case SSH_GSS:                     /* GSSAPI */
        if ((y = cmkey(gssapitab,ngssapitab,"","", xxstring)) < 0)
          return(y);
        switch (y) {
          case 1:                       /* Delegate credentials */
            return(success = seton(&ssh_gsd));
          case 2:                       /* key-exchange */
            return(success = seton(&ssh_gkx));
        }
        if ((x = cmcfm()) < 0)
          return(x);
        return(success = 0);

#ifdef COMMENT
      case SSH_KBD:                     /* Kbd Interactive Devices */
        if ((x = cmcfm()) < 0)
          return(x);
        /* TO BE FILLED IN */
        return(-2);
#endif /* COMMENT */

      case SSH_K4:                      /* Kerberos IV */
      case SSH_K5:                      /* Kerberos V */
        if ((y = cmkey(tgtpass,1,"","tgt-passing", xxstring)) < 0)
          return(y);
        switch (y) {
          case 1:
            return(success = (cx == SSH_K4) ?
                   seton(&ssh_k4tgt) : seton(&ssh_k5tgt));
        }
        if ((x = cmcfm()) < 0)
          return(x);
        return(success = 0);

      case SSH_PRP:                     /* Privd port */
        return(success = seton(&ssh_prp));

      case SSH_SHH:                     /* Quiet */
        return(success = seton(&ssh_shh));

      case SSH_SHK:                     /* Strict Host Key Check */
        if ((y = cmkey(ooktab,3,"","", xxstring)) < 0)
          return(y);
        if ((x = cmcfm()) < 0)
          return(x);
        ssh_shk = y;
        return(success = 1);

      case SSH_HBT:
	x = cmnum("Heartbeat interval, seconds","0",10,&z,xxstring);
	if (x < 0) return(x);
	if ((x = cmcfm()) < 0) return(x);
	ssh_hbt = z;
	return(success = 1);

      case SSH_V1:                      /* SSH V1 */
        if ((y = cmkey(sshv1tab,nsshv1tab,"","", xxstring)) < 0)
          return(y);
        switch (y) {
          case SSH1_K54:
            return(success = seton(&ssh_k5_is_k4));
          case SSH1_CIF:                /* Not a list */
            if ((y = cmkey(ssh1ciphers,nssh1ciphers,"","", xxstring)) < 0)
              if (y != -3)
                return(y);
            if ((x = cmcfm()) < 0) return(x);
            if (y == -3) {
                makestr(&ssh1_cif,NULL);
            } else {
                for (x = 0; x < nssh1ciphers; x++)
                  if (ssh1ciphers[x].kwval == y)
                    break;
                makestr(&ssh1_cif,ssh1ciphers[x].kwd);
            }
            return(1);
          case SSH1_GNH:
          case SSH1_UNH:
            if ((x = cmifi("Filename","",&s,&z,xxstring)) < 0) {
                if (x != -3)
                  return(x);
            } else {
                ckstrncpy(line,s,LINBUFSIZ);
                if (zfnqfp(line,TMPBUFSIZ,tmpbuf))
                  ckstrncpy(line,tmpbuf,LINBUFSIZ);
            }
            s = (x == -3) ? NULL : line;
            if ((x = cmcfm()) < 0)
              return(x);
            switch (y) {
              case SSH1_GNH: makestr(&ssh1_gnh,s); break;
              case SSH1_UNH: makestr(&ssh1_unh,s); break;
            }
            return(1);
        }

      case SSH_V2:                      /* SSH V2 */
        if ((y = cmkey(sshv2tab,nsshv2tab,"","", xxstring)) < 0)
          return(y);
        switch (y) {
          case SSH2_ARK:
            return(success = seton(&ssh2_ark));
        case SSH2_AUT: {                        /* Authentication */
#define TMPCNT 12
            int i, j, tmp[TMPCNT];
            for (i = 0; i < TMPCNT; i++)
              tmp[i] = 0;
            for (i = 0; i < TMPCNT; i++) {
                if ((y = cmkey(ssh2aut,nssh2aut,
                               "Authentication method","",xxstring)) < 0) {
                    if (y == -3)
                      break;
                    return(y);
                }
                for (j = 0; j < i; j++) {
                    if (tmp[j] == y) {
                        printf("\r\n?Choice has already been used.\r\n");
                        return(-9);
                    }
                }
                tmp[i] = y;
            }
            if ((z = cmcfm()) < 0)
              return(z);

            if (ssh2_auth) {
                free(ssh2_auth);
                ssh2_auth = NULL;
            }
            if (i > 0) {
                int len = 0;
                for (j = 0; j < i; j++) {
                    for (x = 0; x < nssh2aut; x++)
                      if (ssh2aut[x].kwval == tmp[j] && !ssh2aut[x].flgs)
                        break;
                    len += strlen(ssh2aut[x].kwd) + 1;
                }
                ssh2_auth = malloc(len);
                ssh2_auth[0] = '\0';
                for (j = 0; j < i; j++) {
                    for (x = 0; x < nssh2aut; x++)
                      if (ssh2aut[x].kwval == tmp[j] && !ssh2aut[x].flgs)
                        break;
                    ckstrncat(ssh2_auth,ssh2aut[x].kwd,len);
                    if (j < i - 1)
                      ckstrncat(ssh2_auth,",",len);
                }
            }
            return(success = 1);
#undef TMPCNT
          }
        case SSH2_CIF: {
#define TMPCNT 12
            int i, j, tmp[TMPCNT];
            for (i = 0; i < TMPCNT; i++)
              tmp[i] = 0;

            for (i = 0; i < TMPCNT; i++) {
                if ((y = cmkey(ssh2ciphers,nssh2ciphers,
                               "","", xxstring)) < 0) {
                    if (y == -3)
                      break;
                    return(y);
                }
                for (j = 0; j < i; j++) {
                    if (tmp[j] == y) {
                        printf("\r\n?Choice has already been used.\r\n");
                        return(-9);
                    }
                }
                tmp[i] = y;
            }
            if ((z = cmcfm()) < 0)
              return(z);

            if (ssh2_cif) {
                free(ssh2_cif);
                ssh2_cif = NULL;
            }
            if (i > 0) {
                int len = 0;
                for (j=0; j < i; j++) {
                    for (x = 0; x < nssh2ciphers; x++)
                      if (ssh2ciphers[x].kwval == tmp[j] &&
                          !ssh2ciphers[x].flgs)
                        break;
                    len += strlen(ssh2ciphers[x].kwd) + 1;
                }
                ssh2_cif = malloc(len);
                ssh2_cif[0] = '\0';
                for (j = 0; j < i; j++) {
                  for (x = 0; x < nssh2ciphers; x++)
                    if (ssh2ciphers[x].kwval == tmp[j] && !ssh2ciphers[x].flgs)
                      break;
                    ckstrncat(ssh2_cif,ssh2ciphers[x].kwd,len);
                    if (j < i - 1)
                      ckstrncat(ssh2_cif,",",len);
                }
            }
            return(success = 1);
#undef TMPCNT
        }
        case SSH2_MAC: {
#define TMPCNT 12
            int i, j, tmp[TMPCNT];
            for (i = 0; i < TMPCNT; i++)
              tmp[i] = 0;

            for (i = 0; i < TMPCNT; i++) {
                if ((y = cmkey(ssh2macs,nssh2macs,"","", xxstring)) < 0) {
                    if (y == -3)
                      break;
                    return(y);
                }
                for (j = 0; j < i; j++) {
                    if (tmp[j] == y) {
                        printf("\r\n?Choice has already been used.\r\n");
                        return(-9);
                    }
                }
                tmp[i] = y;
            }
            if ((z = cmcfm()) < 0)
                return(z);

            if (ssh2_mac) {
                free(ssh2_mac);
                ssh2_mac = NULL;
            }
            if (i > 0) {
                int len = 0;
                for (j = 0; j < i; j++) {
                    for (x = 0; x < nssh2macs; x++)
                      if (ssh2macs[x].kwval == tmp[j] && !ssh2macs[x].flgs)
                        break;
                    len += strlen(ssh2macs[x].kwd) + 1;
                }
                ssh2_mac = malloc(len);
                ssh2_mac[0] = '\0';
                for (j=0; j < i; j++) {
                    for (x = 0; x < nssh2macs; x++)
                      if (ssh2macs[x].kwval == tmp[j] && !ssh2macs[x].flgs)
                        break;
                    ckstrncat(ssh2_mac,ssh2macs[x].kwd,len);
                    if (j < i - 1)
                      ckstrncat(ssh2_mac,",",len);
                }
            }
            return(success = 1);
#undef TMPCNT
          }
          case SSH2_HKA: {
#define TMPCNT 12
            int i, j, tmp[TMPCNT];
            for (i = 0; i < TMPCNT; i++)
              tmp[i] = 0;

            for (i = 0; i < TMPCNT; i++) {
                if ((y = cmkey(hkatab,nhkatab,
                               "","", xxstring)) < 0) {
                    if (y == -3)
                      break;
                    return(y);
                }
                for (j = 0; j < i; j++) {
                    if (tmp[j] == y) {
                        printf("\r\n?Choice has already been used.\r\n");
                        return(-9);
                    }
                }
                tmp[i] = y;
            }
            if ((z = cmcfm()) < 0)
              return(z);

            if (ssh2_hka) {
                free(ssh2_hka);
                ssh2_hka = NULL;
            }
            if (i > 0) {
                int len = 0;
                for (j=0; j < i; j++) {
                    for (x = 0; x < nhkatab; x++)
                      if (hkatab[x].kwval == tmp[j] &&
                          !hkatab[x].flgs)
                        break;
                    len += strlen(hkatab[x].kwd) + 1;
                }
                ssh2_hka = malloc(len);
                ssh2_hka[0] = '\0';
                for (j = 0; j < i; j++) {
                  for (x = 0; x < nhkatab; x++)
                    if (hkatab[x].kwval == tmp[j] && !hkatab[x].flgs)
                      break;
                    ckstrncat(ssh2_hka,hkatab[x].kwd,len);
                    if (j < i - 1)
                      ckstrncat(ssh2_hka,",",len);
                }
            }
            return(success = 1);
#undef TMPCNT
          }
          case SSH2_GNH:
          case SSH2_UNH:
            if ((x = cmifi("Filename","",&s,&z,xxstring)) < 0) {
                if (x != -3)
                  return(x);
            } else {
                ckstrncpy(line,s,LINBUFSIZ);
                if (zfnqfp(line,TMPBUFSIZ,tmpbuf))
                  ckstrncpy(line,tmpbuf,LINBUFSIZ);
            }
            s = (x == -3) ? NULL : line;
            if ((x = cmcfm()) < 0)
              return(x);
            switch (y) {
              case SSH2_GNH: makestr(&ssh2_gnh,s); break;
              case SSH2_UNH: makestr(&ssh2_unh,s); break;
              default: return(success = 0);
            }
            return(success = 1);
        }

      case SSH_VRB:                     /* Verbosity level */
        y = cmnum("SSH verbosity level, 0-7","2",10,&x,xxstring);
        return(setnum(&ssh_vrb,x,y,7));

      case SSH_VER:                     /* Version */
        if ((y = cmkey(sshver,3,"","auto", xxstring)) < 0)
          return(y);
        if ((x = cmcfm()) < 0)
          return(x);
        ssh_ver = y;                    /* 0 == AUTO */
#ifndef SSHTEST
        sl_ssh_ver_saved = 0;
#endif /* SSHTEST */
        return(success = 1);

      case SSH_IDF: {                   /* Identity file */
        int i, n;
        for (i = 0; i < 32; i++) {
            if ((x = cmifi("Filename","",&s,&y,xxstring)) < 0) {
                if (x == -3)
                  break;
                return(x);
            }
            if (!zfnqfp(s,LINBUFSIZ,line))
              ckstrncpy(line,s,LINBUFSIZ);
            makestr(&ssh_tmp[i],line);
        }
        n = i;
        if ((x = cmcfm()) < 0) {
            for (i = 0; i < n; i++)
              makestr(&(ssh_tmp[i]),NULL);
            return(x);
        }
        for (i = 0; i < 32; i++) {
            makestr(&(ssh_idf[i]),NULL);
            if (i < n) {
                ssh_idf[i] = ssh_tmp[i];
                ssh_tmp[i] = NULL;
            } else {
                makestr(&(ssh_tmp[i]),NULL);
            }
        }
        ssh_idf_n = n;
        return(success = 1);
      }
      case SSH_XFW:                     /* X11-forwarding */
        success = seton(&ssh_xfw);
#ifndef SSHTEST
        if (success)
          sl_ssh_xfw_saved = 0;
#endif /* SSHTEST */
        return(success);

      case SSH_XAL:                     /* SSH Xauth Location */
        if ((x = cmifi("Path to executable", "",&s,&y,xxstring)) < 0) {
            if (x != -3)
              return(x);
        } else {
            ckstrncpy(line,s,LINBUFSIZ);
            if (zfnqfp(line,TMPBUFSIZ,tmpbuf))
              ckstrncpy(line,tmpbuf,LINBUFSIZ);
        }
        s = (x == -3) ? NULL : line;
        if ((x = cmcfm()) < 0) return(x);
        makestr(&ssh_xal,s);
        return(success = 1);

      case SSH_CFG:                     /* Use OpenSSH Config */
        return(success = seton(&ssh_cfg));
#endif /* SSHBUILTIN */

      default:
        return(-2);
    }
}
#endif /* ANYSSH */

#ifdef SFTP_BUILTIN
static int
dosetsftp() {
    int cx, x, y, z;
    char * s;

    if ((cx = cmkey(sftptab,nsftptab,"","", xxstring)) < 0)
      return(cx);
    switch (cx) {
    case XY_SFTP_EOL:
    case XY_SFTP_RCS:
    default:
        return(-2);
    }
}
#endif /* SFTP_BUILTIN */

#ifdef KUI
#include "ikui.h"
extern ULONG RGBTable[16];

#define GUI_RGB  1
#define GUI_WIN  2
#define GUI_FON  3
#define GUI_DIA  4
#define GUI_TLB  5
#define GUI_MNB  6
#define GUI_CLS  7

#define GUIW_POS 1
#define GUIW_RES 2
#define GUIW_RUN 3
#define GUIWR_NON 0
#define GUIWR_FON 1
#define GUIWR_DIM 2
#define GUIWN_RES 1
#define GUIWN_MIN 2
#define GUIWN_MAX 3

static struct keytab guitab[] = {
    { "close",       GUI_CLS,  0 },
    { "dialogs",     GUI_DIA,  0 },
    { "font",        GUI_FON,  0 },
    { "menubar",     GUI_MNB,  0 },
    { "rgbcolor",    GUI_RGB,  0 },
    { "toolbar",     GUI_TLB,  0 },
    { "window",      GUI_WIN,  0 },
    { "", 0, 0}
};
static int nguitab = (sizeof(guitab) / sizeof(struct keytab));

static struct keytab guiwtab[] = {
    { "position",    GUIW_POS, 0 },
    { "resize-mode", GUIW_RES, 0 },
    { "run-mode",    GUIW_RUN, 0 },
    { "", 0, 0}
};
static int nguiwtab = (sizeof(guiwtab) / sizeof(struct keytab));

static struct keytab guiwrtab[] = {
    { "change-dimensions",  GUIWR_DIM, 0 },
    { "none",               GUIWR_NON, 0 },
    { "scale-font",         GUIWR_FON, 0 },
    { "", 0, 0}
};
static int nguiwrtab = (sizeof(guiwrtab) / sizeof(struct keytab));

static struct keytab guiwntab[] = {
    { "maximize",  GUIWN_MAX, 0 },
    { "minimize",  GUIWN_MIN, 0 },
    { "restore",   GUIWN_RES, 0 },
    { "", 0, 0}
};
static int nguiwntab = (sizeof(guiwntab) / sizeof(struct keytab));

static struct keytab rgbtab[] = {
    { "black",         0, 0 },
    { "blue",          1, 0 },
    { "brown",         6, 0 },
    { "cyan",          3, 0 },
    { "darkgray",      8, 0 },
    { "dgray",         8, CM_INV },
    { "green",         2, 0 },
    { "lblue",         9, CM_INV },
    { "lcyan",        11, CM_INV },
    { "lgreen",       10, CM_INV },
    { "lgray",         7, CM_INV },
    { "lightblue",     9, 0 },
    { "lightcyan",    11, 0 },
    { "lightgreen",   10, 0 },
    { "lightgray",     7, 0 },
    { "lightmagenta", 13, 0 },
    { "lightred",     12, 0 },
    { "lmagenta",     13, CM_INV },
    { "lred",         12, CM_INV },
    { "magenta",       5, 0 },
    { "red",           4, 0 },
    { "white",        15, 0 },
    { "yellow",       14, 0 },

};
int nrgb = (sizeof(rgbtab) / sizeof(struct keytab));

VOID
shogui() {
    extern gui_dialog;
    extern HWND getHwndKUI();
    unsigned char cmdsav = colorcmd;
    int i, red, green, blue, lines=0;
    char * s;


    printf("GUI paramters:\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
    printf("  Dialogs:     %s\n",showoff(gui_dialog));
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
    printf("  Position:    %d,%d\n",get_gui_window_pos_x(),
            get_gui_window_pos_y());
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
    printf("  Resolution:  %d x %d\n",GetSystemMetrics(SM_CXSCREEN),
            GetSystemMetrics(SM_CYSCREEN));
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
    printf("  Run-mode:    %s\n",IsIconic(getHwndKUI()) ? "minimized" :
            IsZoomed(getHwndKUI()) ? "maximized" : "restored");
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
    switch ( get_gui_resize_mode() ) {
      case GUIWR_NON:
        s = "none";
        break;
      case GUIWR_FON:
        s = "scales font";
        break;
      case GUIWR_DIM:
        s= "changes dimensions";
        break;
    }
    printf("  Resize-mode: %s\n",s);
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
    printf("\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }

    printf("RGB Color Table:\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
    printf("  Color              Red Green Blue\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
    printf("  ------------------------------------------\n");
    if (++lines > cmd_rows - 3) { if (!askmore()) return; else lines = 0; }
    for (i = 0; i < nrgb; i++) {
        if (!rgbtab[i].flgs) {
            blue = (RGBTable[rgbtab[i].kwval] & 0x00FF0000)>>16;
            green = (RGBTable[rgbtab[i].kwval] & 0x0000FF00)>>8;
            red = (RGBTable[rgbtab[i].kwval] & 0x000000FF);
            printf("  %-18s %3d  %3d  %3d  ",rgbtab[i].kwd,red,green,blue);
            colorcmd = rgbtab[i].kwval << 4;
            printf("********");
            colorcmd = cmdsav;
            printf("\n");
            if (++lines > cmd_rows - 3) {
		if (!askmore())
		  return;
		else
		  lines = 0;
	    }
        }
    }
    printf("\n");
}

int
setrgb() {
    int cx, red = 0, blue = 0, green = 0, z, x;

    if ((cx = cmkey(rgbtab,nrgb,"","",xxstring)) < 0)
      return(cx);
    if ((z = cmnum("Red value, 0-255","",10,&red,xxstring)) < 0)
      return(z);
    if ((z = cmnum("Green value, 0-255","",10,&green,xxstring)) < 0)
      return(z);
    if ((z = cmnum("Blue value, 0-255","",10,&blue,xxstring)) < 0)
      return(z);
    if ((x = cmcfm()) < 0) return(x);
    if (cx > 15 || red > 255 || blue > 255 || green > 255)
      return(-2);
    RGBTable[cx] = (unsigned)(((unsigned)blue << 16) |
        (unsigned)((unsigned)green << 8) |
        (unsigned)red);
    return(success = 1);
}

/*
  Set GUI window position: XY coordinates of upper left corner,
  expressed as pixel numbers in the current screen resolution.
  (0,0) means put ourselves in the upper left corner.
  Can we check for out of bounds?
*/

int
setguiwin() {
    int cx, x, y, z;
    if ((cx = cmkey(guiwtab,nguiwtab,"","",xxstring)) < 0)
      return(cx);
    switch (cx) {
      case GUIW_POS:
        if ((z = cmnum("X coordinate (pixel number)","",10,&x,xxstring)) < 0)
          return(z);
        if ((z = cmnum("Y coordinate (pixel number)","",10,&y,xxstring)) < 0)
          return(z);
        if ((z = cmcfm()) < 0)
          return(z);
        if (x < 0 || y < 0) {
            printf("?Coordinates must be 0 or greater\n");
            return(-9);
        }
        gui_position(x,y);
        return(success = 1);
      case GUIW_RES:
        if ((x = cmkey(guiwrtab,nguiwrtab,"","",xxstring)) < 0)
          return(x);
        if ((z = cmcfm()) < 0)
          return(z);
        gui_resize_mode(x);
        return(success = 1);
      case GUIW_RUN:
	if ((x = cmkey(guiwntab,nguiwntab,"","",xxstring)) < 0)
	  return(x);
	if ((z = cmcfm()) < 0)
	  return(z);
	gui_win_run_mode(x);
	return(success = 1);
      default:
        return(-2);
    }
}

int
setguifont() {				/* Assumes that CKFLOAT is defined! */

    extern struct keytab * term_font;
    extern struct keytab * _term_font;
    extern int tt_font, tt_font_size, ntermfont;
    int x, y, z;
    char *s;

    if (ntermfont == 0)
      BuildFontTable(&term_font, &_term_font, &ntermfont);
    if (!(term_font && _term_font && ntermfont > 0)) {
        printf("?Internal error: Failure to enumerate fonts\n");
        return(-9);
    }
    if ((x = cmkey(_term_font,ntermfont,"","",xxstring)) < 0)
      return(x);
    if ((z = cmfld("Height of font in points","12",&s,xxstring)) < 0)
      return(z);
    if (isfloat(s,0) < 1) {		/* (sets floatval) */
	printf("?Integer or floating-point number required\n");
	return(-9);
    }
    if (floatval < 0.5) {
	printf("?Positive number required\n");
	return(-9);
    }
    if ((z = cmcfm()) < 0)
      return(z);
    tt_font = x;			/* Font index */
    tt_font_size = (int)(floatval * 2);	/* Font size in half points */
    KuiSetProperty(KUI_TERM_FONT, (long)tt_font, (long)tt_font_size);
    return(success = 1);
}

VOID
setguidialog(x) int x;
{
    extern int gui_dialog;
    gui_dialog = x;
    KuiSetProperty(KUI_GUI_DIALOGS, (long)x, 0L);
}

VOID
setguimenubar(x) int x;
{
    KuiSetProperty(KUI_GUI_MENUBAR, (long)x, 0L);
}

VOID
setguitoolbar(x) int x;
{
    KuiSetProperty(KUI_GUI_TOOLBAR, (long)x, 0L);
}

VOID
setguiclose(x) int x;
{
    KuiSetProperty(KUI_GUI_CLOSE, (long)x, 0L);
}

int
setgui() {
    int cx, x, rc;
    if ((cx = cmkey(guitab,nguitab,"","",xxstring)) < 0)
      return(cx);
    switch (cx) {
      case GUI_DIA:
        rc = seton(&x);
        if (rc >= 0)
          setguidialog(x);
        return(rc);
      case GUI_FON:
        return(setguifont());
      case GUI_RGB:
        return(setrgb());
      case GUI_WIN:
        return(setguiwin());
      case GUI_TLB:
        rc = seton(&x);
        if (rc >= 0)
          setguitoolbar(x);
        return(rc);
      case GUI_MNB:
        rc = seton(&x);
        if (rc >= 0)
          setguimenubar(x);
        return(rc);
      case GUI_CLS:
        rc = seton(&x);
        if (rc >= 0)
          setguiclose(x);
        return(rc);
      default:
        return(-2);
    }
}
#endif /* KUI */

VOID
setexitwarn(x) int x; 
{
    xitwarn = x;
#ifdef KUI
    KuiSetProperty(KUI_EXIT_WARNING, (long)x, 0L);
#endif /* KUI */
}

#ifndef NOLOCAL
VOID
setdebses(x) int x; {
#ifdef OS2
    if ((debses != 0) && (x == 0))	/* It was on and we turned it off? */
      os2debugoff();			/* Fix OS/2 coloration */
#endif /* OS2 */
    debses = x;
#ifdef KUI
    KuiSetProperty(KUI_TERM_DEBUG,x,0);
#endif /* KUI */
}
#endif /* NOLOCAL */

/*  D O P R M  --  Set a parameter.  */
/*
 Returns:
  -2: illegal input
  -1: reparse needed
   0: success
*/
int
doprm(xx,rmsflg) int xx, rmsflg; {
    int i = 0, x = 0, y = 0, z = 0;
    long zz = 0L;
    char *s = NULL, *p = NULL;
#ifdef OS2
    char portbuf[64];
    long portspeed = 0L;
    int portparity = 0;
    int portflow = 0;
#endif /* OS2 */

#ifndef NOSETKEY
#ifdef OS2
    if (xx == XYMSK)
      return(setmsk());
#endif /* OS2 */
#endif /* NOSETKEY */

    if (xx == XYFLAG) {                 /* SET FLAG */
        extern int ooflag;
        return(success = seton(&ooflag));
    }
    if (xx == XYPRTR                    /* SET PRINTER (or BPRINTER) */
#ifdef BPRINT
        || xx == XYBDCP
#endif /* BPRINT */
        )
      return(setprinter(xx));

    switch (xx) {

#ifdef ANYX25                           /* SET X25 ... */
case XYX25:
        return(setx25());

#ifndef IBMX25
case XYPAD:                             /* SET PAD ... */
        return(setpadp());
#endif /* IBMX25 */
#endif /* ANYX25 */

#ifndef NOXFER
      case XYEOL:       /* These have all been moved to set send/receive... */
      case XYLEN:       /* Let the user know what to do. */
      case XYMARK:
      case XYNPAD:
      case XYPADC:
      case XYTIMO:
        printf("...Use SET SEND or SET RECEIVE instead.\n");
        printf("Type HELP SET SEND or HELP SET RECEIVE for more info.\n");
        return(success = 0);

      case XYATTR:                      /* File Attribute packets */
        return(setat(rmsflg));

      case XYIFD:                       /* Incomplete file disposition */
        if ((y = cmkey(ifdatab,3,"","auto",xxstring)) < 0) return(y);
        if ((x = cmcfm()) < 0) return(x);
        if (rmsflg) {
            sstate = setgen('S',
                            "310",
                            y == 0 ? "0" : (y == 1 ? "1" : "2"),
                            ""
                            );
            return((int) sstate);
        } else {
            keep = y;
            return(success = 1);
        }
#endif /* NOXFER */

      case XYMATCH:			/* [ REMOTE ] SET MATCH...  */
#ifndef NOXFER
	if ((z = cmkey(matchtab,nmatchtab,"","",xxstring)) < 0)
	  return(z);
	if (rmsflg) {
            if ((y = cmkey(onoff,2,"","on",xxstring)) < 0) return(y);
	    if ((x = cmcfm()) < 0) return(x);
	    switch (z) {
	      case MCH_DOTF:
		return(sstate = setgen('S',"330", y == 0 ? "0" : "1", ""));
	      case MCH_FIFO:
		return(sstate = setgen('S',"331", y == 0 ? "0" : "1", ""));
	      default:
		return(-2);
	      }
	  }
#endif /* NOXFER */
	  switch (z) {
	    case MCH_FIFO:
	      return(success = seton(&matchfifo));
	    case MCH_DOTF:
	      x = seton(&matchdot); 
	      if (x < 0) return(x);
	      dir_dots = -1;
	      return(success = x);
	    default:
	      return(-2);
	  }

#ifndef NOSPL
      case XYINPU:                      /* SET INPUT */
        return(setinp());
#endif /* NOSPL */

#ifdef NETCONN
      case XYNET: {                     /* SET NETWORK */

          struct FDB k1, k2;

          cmfdbi(&k1,_CMKEY,"","","",nnetkey, 0, xxstring, netkey, &k2);
          cmfdbi(&k2,_CMKEY,"","","",nnets,   0, xxstring, netcmd, NULL);

#ifdef OS2     /* Hide network-type keywords for networks not installed */
          for (z = 0; z < nnets; z++) {
              if (netcmd[z].kwval == NET_TCPB && tcp_avail == 0)
                netcmd[z].flgs =  CM_INV;
#ifdef SSHBUILTIN
              if (netcmd[z].kwval == NET_SSH &&
                   !ck_ssleay_is_installed())
                netcmd[z].flgs =  CM_INV;
#endif /* SSHBUILTIN */
#ifdef DECNET
              else if (netcmd[z].kwval == NET_DEC  && dnet_avail == 0)
                netcmd[z].flgs =  CM_INV;
#endif /* DECNET */
#ifdef CK_NETBIOS
              else if (netcmd[z].kwval == NET_BIOS && netbiosAvail == 0)
                netcmd[z].flgs =  CM_INV;
#endif /* CK_NETBIOS */
#ifdef SUPERLAT
              else if (netcmd[z].kwval == NET_SLAT  && slat_avail == 0)
                netcmd[z].flgs =  CM_INV;
#endif /* SUPERLAT */
          }
          if (tcp_avail)                /* Default network type */
            ckstrncpy(tmpbuf,"tcp/ip",TMPBUFSIZ);
#ifdef SSHBUILTIN
          else if ( ck_ssleay_is_installed() )
            ckstrncpy(tmpbuf,"ssh",TMPBUFSIZ);
#endif /* SSHBUILTIN */
#ifdef DECNET
          else if (dnet_avail)
            ckstrncpy(tmpbuf,"decnet",TMPBUFSIZ);
#endif /* DECNET */
#ifdef SUPERLAT
          else if (slat_avail)
            ckstrncpy(tmpbuf,"superlat",TMPBUFSIZ);
#endif /* SUPERLAT */
#ifdef CK_NETBIOS
          else if (netbiosAvail)
            ckstrncpy(tmpbuf,"netbios",TMPBUFSIZ);
#endif /* CK_NETBIOS */
          else ckstrncpy(tmpbuf,"named-pipe",TMPBUFSIZ);
#else  /* OS2 */
#ifdef TCPSOCKET
          ckstrncpy(tmpbuf,"tcp/ip",TMPBUFSIZ);
#else
#ifdef ANYX25
          ckstrncpy(tmpbuf,"x.25",TMPBUFSIZ);
#else
          ckstrncpy(tmpbuf,"",TMPBUFSIZ);
#endif /* ANYX25 */
#endif /* TCPSOCKET */
#endif /* OS2 */

          x = cmfdb(&k1);
          if (x < 0) {                  /* Error */
              if (x == -2 || x == -9)
                printf("?No keywords match: \"%s\"\n",atmbuf);
              return(x);
          }
          z = cmresult.nresult;         /* Keyword value */
          if (cmresult.fdbaddr == &k1) { /* Which table? */
#ifndef NOSPL
#ifndef NODIAL
              if (z == XYNET_D)
                return(parsdir(1));
#endif /* NODIAL */
#endif /* NOSPL */
              if ((z = cmkey(netcmd,nnets,"",tmpbuf,xxstring)) < 0)
                return(z);
          }

#ifdef NETCMD
          if (z == NET_CMD && nopush) {
              printf("\n?Sorry, access to external commands is disabled\n");
              return(-9);
          }
#endif /* NETCMD */

#ifndef NOPUSH
#ifdef NETPTY
          if (z == NET_PTY && nopush) {
              printf("\n?Sorry, access to external commands is disabled\n");
              return(-9);
          }
#endif /* NETPTY */
#endif /* NOPUSH */

#ifdef OS2
          if (z == NET_TCPB && tcp_avail == 0) {
              printf(
"\n?Sorry, either TCP/IP is not available on this system or\n\
necessary DLLs did not load.  Use SHOW NETWORK to check network status.\n");
              return(-9);
#ifdef SSHBUILTIN
          } else if (z == NET_SSH && !ck_ssleay_is_installed()) {
            printf("\n?Sorry, SSH is not available on this system.\n") ;
            return(-9);
#endif /* SSHBUILTIN */
#ifdef CK_NETBIOS
          } else if (z == NET_BIOS && netbiosAvail == 0) {
              printf("\n?Sorry, NETBIOS is not available on this system.\n") ;
              return(-9);
#endif /* CK_NETBIOS */
#ifdef DECNET
          } else if (z == NET_DEC && dnet_avail == 0) {
              printf("\n?Sorry, DECnet is not available on this system.\n") ;
              return(-9);
#endif /* DECNET */
#ifdef SUPERLAT
          } else if (z == NET_SLAT && slat_avail == 0) {
              printf("\n?Sorry, SuperLAT is not available on this system.\n") ;
              return(-9);
#endif /* SUPERLAT */
          }
#endif /* OS2 */

#ifdef NPIPEORBIOS
          if (z == NET_PIPE ||          /* Named pipe -- also get pipename */
              z == NET_BIOS) {          /* NETBIOS -- also get local name */
              char *defnam;
#ifdef CK_NETBIOS
              char tmpnbnam[NETBIOS_NAME_LEN+1];
#endif /* CK_NETBIOS */
              /* Construct default name  */
              if (z == NET_PIPE) {      /* Named pipe */
                  defnam = "kermit";    /* Default name is always "kermit" */
              } 
#ifdef CK_NETBIOS
	      else {			/* NetBIOS */
                  if (NetBiosName[0] != SP) { /* If there is already a name, */
                      char *p = NULL;
                      int n;            /* use it as the default. */
                      ckstrncpy(tmpnbnam,NetBiosName,NETBIOS_NAME_LEN+1);
                      /* convert trailing spaces to NULs */
                      p = &tmpnbnam[NETBIOS_NAME_LEN-1];
                      while (*p == SP) {
                          *p = NUL;
                          p--;
                      }
                      defnam = tmpnbnam;
                  } else if (*myhost)   /* Otherwise use this PC's host name */
                    defnam = (char *) myhost;
                  else                  /* Otherwise use "kermit" */
                    defnam = "kermit";
              }
#endif /* CK_NETBIOS */
              if ((y = cmtxt((z == NET_PIPE) ? "name of named-pipe" :
                             "local NETBIOS name",
                             defnam, &s, xxstring)) < 0)
                return(y);
#ifdef NPIPE
              pipename[0] = NUL;
#endif /* NPIPE */
              if ((y = (int) strlen(s)) < 1) {
                  printf("?You must also specify a %s name\n",
                         (z == NET_PIPE) ? "pipe" : "local NETBIOS" );
                  return(-9);
              }
#ifdef CK_NETBIOS
              if (z == NET_BIOS) {
                  if (!netbiosAvail) {
                      printf("?NETBIOS is not available on this system.\n") ;
                      return(-9);
                  }
                  if (y - NETBIOS_NAME_LEN > 0) {
                      printf("?NETBIOS name too long, %ld maximum\n",
                             NETBIOS_NAME_LEN);
                      return(-9);
                  } else if ( !strcmp(s,tmpnbnam) ) {
                      nettype = z;      /* Returning to old connection... */
                      return(success = 1); /* Done */
                  } else if (strcmp("                ",NetBiosName)) {
                      printf("?NETBIOS name already assigned to \"%s\"\n",
                             NetBiosName);
                      return(-9);
                  } else {
                      NCB ncb;
                      APIRET rc;
                      ckstrncpy(NetBiosName,s,16);
                      for (x = y; x < NETBIOS_NAME_LEN; x++)
                        NetBiosName[x] = SP;
                      NetBiosName[NETBIOS_NAME_LEN] = NUL;
                      printf("Checking \"%s\" as a unique NetBIOS name...\n",
                             NetBiosName);
                      rc = NCBAddName( NetbeuiAPI,
                                      &ncb, NetBiosAdapter, NetBiosName );
                      if (rc) {
                          printf(
                "?Sorry, \"%s\" is already in use by another NetBIOS node.\n",
                                 NetBiosName);
                          for (x = 0; x < NETBIOS_NAME_LEN; x++)
                            NetBiosName[x] = SP;
                          return(-9);
                      }
                  }
              }
#endif /* CK_NETBIOS */
#ifdef NPIPE
              if (z == NET_PIPE)
                ckstrncpy(pipename,s,PIPENAML);
#endif /* NPIPE */
          } else
#endif /* NPIPEORBIOS */
#ifdef DECNET
            if (z == NET_DEC) {
                /* Determine if we are using LAT or CTERM */
                if ((y = cmkey(dnettab,
                               ndnet,"DECNET protocol","lat",xxstring)) < 0)
                  return(y);
                if ((x = cmcfm()) < 0) return(x);
                ttnproto = y;
            } else
#endif /* DECNET */
#ifdef NETDLL
              if (z == NET_DLL) {
                  /* Find out which DLL they are using */
                  char dllname[256]="";
                  char * p=NULL;
                  if ((x = cmifi("Dynamic load library",
                                 "",&p,&y,xxstring)) < 0) {
                      if (x == -3) {
                          printf("?Name of DLL required\n");
                          return(-9);
                      }
                      return(x);
                  }
                  ckstrncpy(dllname,p,256);
                  if ((x = cmcfm()) < 0) return(x);

                  if (netdll_load(dllname) < 0) /* Try to load the dll */
                    return(success = 0);
                  else {
                      nettype = z;
                      return(success = 1);
                  }
              } else
#endif /* NETDLL */
                if ((x = cmcfm()) < 0) return(x);
          nettype = z;
          if (
#ifdef DECNET
              (nettype != NET_DEC)  &&
#endif /* DECNET */
#ifdef NPIPE
              (nettype != NET_PIPE) &&
#endif /* NPIPE */
#ifdef CK_NETBIOS
              (nettype != NET_BIOS) &&
#endif /* CK_NETBIOS */
#ifdef NETFILE
              (nettype != NET_FILE) &&
#endif /* NETFILE */
#ifdef NETCMD
              (nettype != NET_CMD) &&
#endif /* NETCMD */
#ifdef NETPTY
              (nettype != NET_PTY) &&
#endif /* NETPTY */
#ifdef NETDLL
              (nettype != NET_DLL) &&
#endif /* NETDLL */
#ifdef SUPERLAT
              (nettype != NET_SLAT) &&
#endif /* SUPERLAT */
              (nettype != NET_SX25) &&
              (nettype != NET_VX25) &&
#ifdef IBMX25
              (nettype != NET_IX25) &&
#endif /* IBMX25 */
#ifdef SSHBUILTIN
              (nettype != NET_SSH) &&
#endif /* SSHBUILTIN */
              (nettype != NET_TCPB)) {
              printf("?Network type not supported\n");
              return(success = 0);
          } else {
              return(success = 1);
          }
      }

#ifndef NOTCPOPTS
#ifdef TCPSOCKET
      case XYTCP: {
        extern int ttyfd;

        if ((z = cmkey(tcpopt,ntcpopt,"TCP option","nodelay",xxstring)) < 0)
          return(z);

        switch (z) {
#ifndef NOHTTP
          case XYTCP_HTTP_PROXY: {
	      struct FDB sw, tx;
	      int n, x;
	      char ubuf[LOGINLEN+1], pbuf[LOGINLEN+1], abuf[256];
	      ubuf[0] = pbuf[0] = abuf[0] = 0;

	      cmfdbi(&sw,		/* First FDB - switches */
		     _CMKEY,		/* fcode */
		     "HTTP proxy server host[:port] or switch",
		     "",		/* default */
		     "",		/* addtl string data */
		     nuserpass,		/* addtl numeric data 1: tbl size */
		     4,			/* addtl numeric data 2: 4 = cmswi */
		     xxstring,		/* Processing function */
		     userpass,		/* Keyword table */
		     &tx		/* Pointer to next FDB */
		     );
	      cmfdbi(&tx,
		     _CMTXT,		/* fcode */
		     "HTTP proxy server host[:port]",
		     "",		/* default */
		     "",		/* addtl string data */
		     0,			/* addtl numeric data 1 */
		     0,			/* addtl numeric data 2 */
		     xxstring,
		     NULL,
		     NULL
		     );
	      while (1) {
		  if ((x = cmfdb(&sw)) < 0) {
		      if (x == -3) {
			  x = -9;
			  printf("?Hostname required\n");
		      }
		      return(x);
		  }
		  if (cmresult.fcode != _CMKEY)
		    break;
		  n = cmresult.nresult;
		  switch (n) {
		    case UPW_USER:
		    case UPW_PASS:
		    case UPW_AGENT:
		      if ((x = cmfld((n == UPW_USER) ?
				     "Username" :
				     ((n == UPW_PASS) ? "Password" : "Agent"),
				     "", &s, xxstring)) < 0) {
			  if (x != -3)
			    return(x);
		      }
		      ckstrncpy((n == UPW_USER) ? ubuf :
                        ((n == UPW_PASS) ? pbuf : abuf), s, 
                        (n == UPW_AGENT) ? 256 : (LOGINLEN+1));
		  }
	      }
	      if (cmresult.fcode != _CMTXT)
		return(-2);
	      s = cmresult.sresult;
	      if (s) if (!*s) s = NULL;

#ifdef IKSDCONF
	      if (iksdcf)
		return(success = 0);
#endif /* IKSDCONF */
	      makestr(&tcp_http_proxy_user,ubuf[0]?ubuf:NULL);
	      makestr(&tcp_http_proxy_pwd,pbuf[0]?pbuf:NULL);
	      makestr(&tcp_http_proxy_agent,abuf[0]?abuf:NULL);
	      makestr(&tcp_http_proxy,s);
	      memset(pbuf,0,sizeof(pbuf));
	      return(success = 1);
	  }
#endif /* NOHTTP */
/*
  It would have been easy to combine XYTCP_SOCKS_SVR with the previous
  one except for the #ifdefs...
*/
#ifdef NT
#ifdef CK_SOCKS
          case XYTCP_SOCKS_SVR: {
	      char ubuf[LOGINLEN+1], pbuf[LOGINLEN+1];
	      char * p = getenv("SOCKS_SERVER");
	      struct FDB sw, tx;
	      int n, x;

	      if (!p) p = "";

	      cmfdbi(&sw,		/* First FDB - switches */
		     _CMKEY,		/* fcode */
		     "SOCKS server host[:port] or switch",
		     "",		/* default */
		     "",		/* addtl string data */
		     nuserpass,		/* addtl numeric data 1: tbl size */
		     4,			/* addtl numeric data 2: 4 = cmswi */
		     xxstring,		/* Processing function */
		     userpass,		/* Keyword table */
		     &tx		/* Pointer to next FDB */
		     );
	      cmfdbi(&tx,
		     _CMTXT,		/* fcode */
		     "SOCKS server host[:port]",
		     p,			/* default */
		     "",		/* addtl string data */
		     0,			/* addtl numeric data 1 */
		     0,			/* addtl numeric data 2 */
		     xxstring,
		     NULL,
		     NULL
		     );
	      while (1) {
		  if ((x = cmfdb(&sw)) < 0) {
		      if (x == -3) {
			  x = -9;
			  printf("?Hostname required\n");
		      }
		      return(x);
		  }
		  if (cmresult.fcode != _CMKEY)
		    break;
		  n = cmresult.nresult;
		  switch (n) {
		    case UPW_USER:
		    case UPW_PASS:
		      if ((x = cmfld((n == UPW_USER) ? "Username" : "Password",
				     "", &s, xxstring)) < 0) {
			  if (x != -3)
			    return(x);
		      }
		      ckstrncpy((n == UPW_USER) ? ubuf : pbuf, s, LOGINLEN+1);
		  }
	      }
	      if (cmresult.fcode != _CMTXT)
		return(-2);
	      s = cmresult.sresult;
	      if (s) if (!*s) s = NULL;

#ifdef IKSDCONF
	      if (iksdcf)
		return(success = 0);
#endif /* IKSDCONF */
	      makestr(&tcp_socks_user,ubuf);
              memset(pbuf,0,sizeof(pbuf));
	      makestr(&tcp_socks_svr,s);
	      return(success = 1);
	  }

#ifdef CK_SOCKS_NS
          case XYTCP_SOCKS_NS: {
            char * p = getenv("SOCKS_NS");
            if (!p) p = "";
            if ((y = cmtxt("hostname or IP of SOCKS Name Server",p,
                            &s,xxstring)) < 0)
                return(y);
#ifdef IKSDCONF
              if (iksdcf) return(success = 0);
#endif /* IKSDCONF */
              if (tcp_socks_ns) {
                  free(tcp_socks_ns);   /* Free any previous storage */
                  tcp_socks_ns = NULL;
              }
              if (s == NULL || *s == NUL) { /* If none given */
                  tcp_socks_ns = NULL;  /* remove the override string */
                  return(success = 1);
              } else if ((tcp_socks_ns = malloc(strlen(s)+1))) {
                  strcpy(tcp_socks_ns,s);
                  return(success = 1);
              } else
                return(success = 0);
          }
#endif /* CK_SOCKS_NS */
#endif /* CK_SOCKS */
#endif /* NT */
          case XYTCP_ADDRESS:
            if ((y = cmtxt("preferred IP Address for TCP connections","",
                           &s,xxstring)) < 0)
              return(y);
#ifdef IKSDCONF
            if (iksdcf) return(success = 0);
#endif /* IKSDCONF */
            if (tcp_address) {
                free(tcp_address);      /* Free any previous storage */
                tcp_address = NULL;
            }
            if (s == NULL || *s == NUL) { /* If none given */
                tcp_address = NULL;     /* remove the override string */
                return(success = 1);
            } else if ((tcp_address = malloc(strlen(s)+1))) {
                strcpy(tcp_address,s);
                return(success = 1);
            } else
              return(success = 0);
#ifdef SO_KEEPALIVE
          case XYTCP_KEEPALIVE:
            if ((z = cmkey(onoff,2,"","on",xxstring)) < 0) return(z);
            if ((y = cmcfm()) < 0) return(y);
#ifdef IKSDCONF
            if (iksdcf) return(success = 0);
#endif /* IKSDCONF */
#ifdef SSHBUILTIN
            if (network && nettype == NET_SSH && ssh_sock != -1)
              success = keepalive(ssh_sock,z);
            else
#endif /* SSHBUILTIN */
	      success = keepalive(ttyfd,z);
            return(success);
#endif /* SO_KEEPALIVE */
#ifdef SO_DONTROUTE
          case XYTCP_DONTROUTE:
            if ((z = cmkey(onoff,2,"","off",xxstring)) < 0) return(z);
            if ((y = cmcfm()) < 0) return(y);
#ifdef IKSDCONF
            if (iksdcf) return(success = 0);
#endif /* IKSDCONF */
#ifdef SSHBUILTIN
            if (network && nettype == NET_SSH && ssh_sock != -1)
              success = dontroute(ssh_sock,z);
            else
#endif /* SSHBUILTIN */
	      success = dontroute(ttyfd,z);
            return(success);
#endif /* SO_DONTROUTE */
#ifdef TCP_NODELAY
          case XYTCP_NODELAY:
            if ((z = cmkey(onoff,2,"","off",xxstring)) < 0) return(z);
            if ((y = cmcfm()) < 0) return(y);
#ifdef IKSDCONF
            if (iksdcf) return(success = 0);
#endif /* IKSDCONF */
#ifdef SSHBUILTIN
            if (network && nettype == NET_SSH && ssh_sock != -1)
              success = no_delay(ssh_sock,z);
            else
#endif /* SSHBUILTIN */
	      success = no_delay(ttyfd,z);
            return(success);
          case XYTCP_NAGLE:             /* The inverse of NODELAY */
            if ((z = cmkey(onoff,2,"","on",xxstring)) < 0) return(z);
            if ((y = cmcfm()) < 0) return(y);
#ifdef IKSDCONF
            if (iksdcf) return(success = 0);
#endif /* IKSDCONF */
#ifdef SSHBUILTIN
            if (network && nettype == NET_SSH && ssh_sock != -1)
              success = no_delay(ssh_sock,z);
            else
#endif /* SSHBUILTIN */
	      success = no_delay(ttyfd,!z);
            return(success);
#endif /* TCP_NODELAY */
#ifdef SO_LINGER
          case XYTCP_LINGER:
            if ((z = cmkey(onoff,2,"","on",xxstring)) < 0)
              return(z);
            if (z) {                    /* if on, we need a timeout value */
                if ((x = cmnum("Linger timeout in 10th of a millisecond",
                               "0",10,&y,xxstring)) < 0)
                  return(x);
            } else
              y = 0;
            if ((x = cmcfm()) < 0)
              return(x);
#ifdef IKSDCONF
            if (iksdcf) return(success = 0);
#endif /* IKSDCONF */
#ifdef SSHBUILTIN
            if (network && nettype == NET_SSH && ssh_sock != -1)
              success = ck_linger(ssh_sock,z,y);
            else
#endif /* SSHBUILTIN */
	      success = ck_linger(ttyfd,z,y);
            return(success);
#endif /* SO_LINGER */
#ifdef SO_SNDBUF
          case XYTCP_SENDBUF:
            x = cmnum("Send buffer size, bytes","8192",10,&z,xxstring);
            if (x < 0) return(x);
            if ((x = cmcfm()) < 0) return(x);
#ifdef IKSDCONF
            if (iksdcf) return(success = 0);
#endif /* IKSDCONF */
#ifdef SSHBUILTIN
            if (network && nettype == NET_SSH && ssh_sock != -1)
              success = sendbuf(ssh_sock,z);
            else
#endif /* SSHBUILTIN */
	      success = sendbuf(ttyfd,z);
            return(success);
#endif /* SO_SNDBUF */
#ifdef SO_RCVBUF
          case XYTCP_RECVBUF:
            x = cmnum("Receive buffer size, bytes","8192",10,&z,xxstring);
            if (x < 0) return(x);
            if ((x = cmcfm()) < 0) return(x);
#ifdef IKSDCONF
            if (iksdcf) return(success = 0);
#endif /* IKSDCONF */

/* Note: The following is not 16-bit safe */

#ifndef QNX16
            if (x > 52248) {
                printf("?Warning: receive buffers larger than 52248 bytes\n");
                printf(" may not be understood by all hosts.  Performance\n");
                printf(" may suffer.\n");
                return(-9);
            }
#endif /* QNX16 */
#ifdef SSHBUILTIN
            if (network && nettype == NET_SSH && ssh_sock != -1)
              success = recvbuf(ssh_sock,z);
            else
#endif /* SSHBUILTIN */
	      success = recvbuf(ttyfd,z);
            return(success);
#endif /* SO_RCVBUF */

#ifdef VMS
#ifdef DEC_TCPIP
          case XYTCP_UCX: {             /* UCX 2.0 port swabbing bug */
              extern int ucx_port_bug;
              return(success = seton(&ucx_port_bug));
          }
#endif /* DEC_TCPIP */
#endif /* VMS */

          case XYTCP_RDNS: {
              extern int tcp_rdns;
              return(success = setonaut(&tcp_rdns));
          }

#ifdef CK_DNS_SRV
          case XYTCP_DNS_SRV: {
              extern int tcp_dns_srv;
              return(success = seton(&tcp_dns_srv));
          }
#endif /* CK_DNS_SRV */

          default:
            return(0);
        }
      }
#endif /* TCPSOCKET */
#endif /* NOTCPOPTS */
#endif /* NETCONN */
    }

    switch (xx) {

#ifndef NOLOCAL
#ifdef NETCONN
      case XYHOST: {                    /* SET HOST */
          z = ttnproto;                 /* Save protocol in case of failure */
#ifdef DECNET
          if (nettype != NET_DEC)
#endif /* DECNET */
            ttnproto = NP_NONE;
          if ((y = setlin(XYHOST,1,0)) <= 0) { /* Sets success to 1 */
              debug(F101,"SET HOST fail mdmtyp","",mdmtyp);
              ttnproto = z;             /* Failed, restore protocol */
              success = 0;
          }
          didsetlin++;
          debug(F101,"SET HOST OK mdmtyp","",mdmtyp);
          debug(F101,"SET HOST reliable","",reliable);
          return(y);
      }
#endif /* NETCONN */

      case XYLINE:                      /* SET LINE (= SET PORT) */
        debug(F101,"setlin flow 1","",flow);
        x = setlin(xx,1,0);
        if (x > -1) didsetlin++;
        debug(F101,"SET LINE setlin","",x);
        debug(F101,"SET LINE flow","",flow);
        debug(F101,"SET LINE local","",local);
        debug(F101,"SET LINE reliable","",reliable);
        return(x);
#endif /* NOLOCAL */

#ifndef NOSETKEY
      case XYKEY:                       /* SET KEY */
        return(dosetkey());
#endif /* NOSETKEY */

#ifndef NOCSETS
      case XYLANG:                      /* Language */
        if ((y = cmkey(lngtab,nlng,"","none",xxstring)) < 0)
          return(y);
        if ((x = cmcfm()) < 0) return(x);

        /* Look up language and get associated character sets */
        for (i = 0; (i < nlangs) && (langs[i].id != y); i++) ;
        if (i >= nlangs) {
            printf("?internal error, sorry\n");
            return(success = 0);
        } /*  */
        language = i;                   /* All good, set the language, */
        return(success = 1);
#endif /* NOCSETS */

#ifndef MAC
      case XYBACK:                      /* BACKGROUND */
        if ((z = cmkey(onoff,2,"","",xxstring)) < 0) return(z);
        if ((y = cmcfm()) < 0) return(y);
#ifdef COMMENT
        bgset = z;                      /* 0 = off (foreground) */
#ifdef VMS                              /* 1 = on (background) */
        if (batch && bgset == 0)        /* To enable echoing of commands */
          ckxech = 1;                   /* in VMS batch logs */
#endif /* VMS */
#else  /* COMMENT */
        if (z) {                        /* 1 = Background */
            bgset = 1;
            backgrd = 1;
#ifdef VMS
            batch = 1;
#endif /* VMS */
        } else {                        /* 0 = Foreground */
            bgset = 0;
            backgrd = 0;
#ifdef VMS
            batch = 0;
#endif /* VMS */
        }
#endif /* COMMENT */
        success = 1;
        bgchk();
        return(success);
#endif /* MAC */

      case XYQUIE: {                    /* QUIET */
#ifdef DCMDBUF
          extern int * xquiet;
#else
          extern int xquiet[];
#endif /* DCMDBUF */
          x = seton(&quiet);
          if (x < 0) return(x);
          xquiet[cmdlvl] = quiet;
          return(success = x);
      }

#ifndef NOXFER
      case XYBUF: {                     /* BUFFERS */
#ifdef DYNAMIC
          int sb, rb;
          if ((y = cmnum("Send buffer size","",10,&sb,xxstring)) < 0) {
              if (y == -3) printf("?Buffer size required\n");
              return(y);
          }
          if (sb < 0) {
              if (*atmbuf == '-')
                printf("?Negative numbers can't be used here\n");
              else printf("?Integer overflow, use a smaller number please\n");
              return(-9);
          } else if (sb < 80) {
              printf("?Too small\n");
              return(-9);
          }
          if ((y=cmnum("Receive buffer size",ckitoa(sb),10,&rb,xxstring)) < 0)
            return(y);
          if (rb < 0) {
              if (*atmbuf == '-')
                printf("?Negative numbers can't be used here\n");
              else printf("?Integer overflow, use a smaller number please\n");
              return(-9);
          } else if (rb < 80) {
              printf("?Too small\n");
              return(-9);
          }
          if ((y = cmcfm()) < 0) return(y);
          if ((y = inibufs(sb,rb)) < 0) return(y);
          y = adjpkl(urpsiz,wslotr,bigrbsiz); /* Maybe adjust packet sizes */
          if (y != urpsiz) urpsiz = y;
          y = adjpkl(spsiz,wslotr,bigsbsiz);
          if (y != spsiz) spsiz = spmax = spsizr = y;
          return(success = 1);
#else
          printf("?Sorry, not available\n");
          return(success = 0);
#endif /* DYNAMIC */
      }

      case XYCHKT:                      /* BLOCK-CHECK */
        if ((x = cmkey(chktab,nchkt,"","3",xxstring)) < 0) return(x);
        if ((y = cmcfm()) < 0) return(y);
	if (x == 5) {
	    bctf = 1;
#ifdef COMMENT
	    printf("?5 - Not implemented yet\n");
	    return(success = 0);
#endif	/* COMMENT */
	}
        bctr = x;                       /* Set local too even if REMOTE SET */

        if (rmsflg) {
            if (x == 4) {
                tmpbuf[0] = 'B';
                tmpbuf[1] = '\0';
            } else
              ckstrncpy(tmpbuf,ckitoa(x),TMPBUFSIZ);
            sstate = setgen('S', "400", tmpbuf, "");
            return((int) sstate);
        } else {
            return(success = 1);
        }
#endif /* NOXFER */

#ifndef NOLOCAL
#ifndef MAC                             /* The Mac has no RS-232 */
case XYCARR:                            /* CARRIER-WATCH */
        return(setdcd());
#endif /* MAC */
#endif /* NOLOCAL */
    }

#ifdef TNCODE
    switch (xx) {                       /* Avoid long switch statements... */
      case XYTELOP: {
          int c, n;                     /* Workers */
          int getval = 0;               /* Whether to get switch value */
          int tnserver = 0;             /* Client by default */
          int opt = -1;                 /* Telnet Option */
          struct FDB sw, op;            /* FDBs for each parse function */
#ifdef CK_AUTHENTICATION
          extern int sl_topt_a_s_saved;
          extern int sl_topt_a_c_saved;
          extern int sl_topt_e_s_saved;
          extern int sl_topt_e_c_saved;
#endif /* CK_AUTHENTICATION */
#ifdef IKSD
          if (inserver)                 /* Server by default when IKSD */
            tnserver = 1;
#endif /* IKSD */

          /* Set up chained parse functions... */

          cmfdbi(&op,                   /* First fdb - telopts*/
                 _CMKEY,                /* fcode */
                 "/client, /server or", /* hlpmsg */
                 "",                    /* default */
                 "",                    /* addtl string data */
                 ntnopt,                /* addtl numeric data 1 */
                 0,                     /* addtl numeric data 2 */
                 xxstring,
                 tnopttab,
                 &sw
                 );
          cmfdbi(&sw,                   /* Second FDB - command switches */
                 _CMKEY,                /* fcode */
                 "",                    /* hlpmsg */
                 "",                    /* default */
                 "",                    /* addtl string data */
                 ntnoptsw,              /* addtl numeric data 1: tbl size */
                 4,                     /* addtl numeric data 2: 4 = cmswi */
                 xxstring,              /* Processing function */
                 tnoptsw,               /* Keyword table */
                 NULL                   /* Pointer to next FDB */
                 );

          while (opt < 0) {             /* Parse 0 or more switches */
              x = cmfdb(&op);           /* Parse switch or other thing */
              debug(F101,"XYTELOP cmfdb","",x);
              if (x < 0)                /* Error */
                return(x);              /* or reparse needed */
              if (cmresult.fcode != _CMKEY) /* Break out if not a switch */
                break;
              c = cmgbrk();             /* Get break character */
              getval = (c == ':' || c == '='); /* see how switch ended */
              if (getval && !(cmresult.kflags & CM_ARG)) {
                  printf("?This switch does not take arguments\n");
                  return(-9);
              }
              z = cmresult.nresult;     /* Numeric result = switch value */
              debug(F101,"XYTELOP switch","",z);

              switch (z) {              /* Process the switch */
                case CK_TN_CLIENT:
                  tnserver = 0;
                  break;
                case CK_TN_SERVER:
                  tnserver = 1;
                    break;
                case CK_TN_EC:
                  opt = TELOPT_ECHO;
                  break;
                case CK_TN_TT:
                  opt = TELOPT_TTYPE;
                  break;
                case CK_TN_BM:
                  opt = TELOPT_BINARY;
                  break;
                case CK_TN_ENV:
                  opt = TELOPT_NEWENVIRON;
                  break;
                case CK_TN_LOC:
                  opt = TELOPT_SNDLOC;
                  break;
                case CK_TN_AU:
                  opt = TELOPT_AUTHENTICATION;
                  break;
                case CK_TN_FX:
                  opt = TELOPT_FORWARD_X;
                  break;
                case CK_TN_ENC:
                  opt = TELOPT_ENCRYPTION;
                  break;
                case CK_TN_IKS:
                  opt = TELOPT_KERMIT;
                  break;
                case CK_TN_TLS:
                  opt = TELOPT_START_TLS;
                  break;
                case CK_TN_XD:
                  opt = TELOPT_XDISPLOC;
                  break;
                case CK_TN_NAWS:
                  opt = TELOPT_NAWS;
                  break;
                case CK_TN_SGA:
                  opt = TELOPT_SGA;
                  break;
                case CK_TN_PHR:
                  opt = TELOPT_PRAGMA_HEARTBEAT;
                  break;
                case CK_TN_PSP:
                  opt = TELOPT_SSPI_LOGON;
                  break;
                case CK_TN_PLG:
                  opt = TELOPT_PRAGMA_LOGON;
                  break;
                case CK_TN_SAK:
                  opt = TELOPT_IBM_SAK;
                  break;
                case CK_TN_CPC:
                  opt = TELOPT_COMPORT;
                  break;
                case CK_TN_LOG:
                  opt = TELOPT_LOGOUT;
                  break;
                case CK_TN_FLW:
                  opt = TELOPT_LFLOW;
                  break;
                default:
                  printf("?Unexpected value - %d\n",z);
                  return(-9);
              }
#ifdef COMMENT
              if (cmresult.fdbaddr == &op)
                break;
#endif /* COMMENT */
          }
          switch (opt) {
            case TELOPT_ECHO:           /* Options only the Server WILL */
            case TELOPT_FORWARD_X:
            case TELOPT_SEND_URL:
            case TELOPT_IBM_SAK:
            case TELOPT_LOGOUT:
              if ((x = cmkey(tnnegtab,
                             ntnnegtab,
                             "desired server state",
   TELOPT_MODE(tnserver?TELOPT_DEF_S_ME_MODE(opt):TELOPT_DEF_C_U_MODE(opt)),
                             xxstring)
                   ) < 0)
                return(x);
              if ((z = cmcfm()) < 0)
                  return(z);
              if (tnserver) {
                  TELOPT_DEF_S_ME_MODE(opt) = x;
                  TELOPT_ME_MODE(opt) = x;
              } else {
                  TELOPT_DEF_C_U_MODE(opt) = x;
                  TELOPT_U_MODE(opt) = x;
              }
              break;

            case TELOPT_TTYPE:          /* Options only the Client WILL */
            case TELOPT_NEWENVIRON:
            case TELOPT_SNDLOC:
            case TELOPT_AUTHENTICATION:
            case TELOPT_START_TLS:
            case TELOPT_XDISPLOC:
            case TELOPT_NAWS:
            case TELOPT_LFLOW:
            case TELOPT_COMPORT:
              if ((x = cmkey(tnnegtab,
                             ntnnegtab,
                             "desired client state",
    TELOPT_MODE(!tnserver?TELOPT_DEF_S_U_MODE(opt):TELOPT_DEF_C_ME_MODE(opt)),
                             xxstring)
                   ) < 0)
                return(x);
              if ((z = cmcfm()) < 0)
                return(z);
              if (tnserver) {
                  TELOPT_DEF_S_U_MODE(opt) = x;
                  TELOPT_U_MODE(opt) = x;
#ifdef CK_AUTHENTICATION
                  if (opt == TELOPT_AUTHENTICATION)
                    sl_topt_a_s_saved = 0;
#endif /* CK_AUTHENTICATION */
              } else {
                  TELOPT_DEF_C_ME_MODE(opt) = x;
                  TELOPT_ME_MODE(opt) = x;
#ifdef CK_AUTHENTICATION
                  if (opt == TELOPT_AUTHENTICATION)
                    sl_topt_a_c_saved = 0;
#endif /* CK_AUTHENTICATION */
              }
              break;

            default:
              if ((x = cmkey(tnnegtab,
                             ntnnegtab,
                             tnserver ?
                             "desired server state" :
                             "desired client state",
    TELOPT_MODE(tnserver?TELOPT_DEF_S_ME_MODE(opt):TELOPT_DEF_C_ME_MODE(opt)),
                             xxstring
                             )
                   ) < 0)
                return(x);
              if ((y = cmkey(tnnegtab,
                             ntnnegtab,
                             !tnserver ? "desired server state" :
                             "desired client state",
    TELOPT_MODE(!tnserver?TELOPT_DEF_S_U_MODE(opt):TELOPT_DEF_C_U_MODE(opt)),
                             xxstring
                             )
                   ) < 0)
                return(y);
              if ((z = cmcfm()) < 0)
                return(z);
              if (tnserver) {
                  TELOPT_DEF_S_ME_MODE(opt) = x;
                  TELOPT_ME_MODE(opt) = x;
                  TELOPT_DEF_S_U_MODE(opt) = y;
                  TELOPT_U_MODE(opt) = y;
#ifdef CK_ENCRYPTION
                  if (opt == TELOPT_ENCRYPTION)
                    sl_topt_e_s_saved = 0;
#endif /* CK_ENCRYPTION */
              } else {
                  TELOPT_DEF_C_ME_MODE(opt) = x;
                  TELOPT_ME_MODE(opt) = x;
                  TELOPT_DEF_C_U_MODE(opt) = y;
                  TELOPT_U_MODE(opt) = y;
#ifdef CK_ENCRYPTION
                  if (opt == TELOPT_ENCRYPTION)
                    sl_topt_e_c_saved = 0;
#endif /* CK_ENCRYPTION */
              }
          }
          return(success = 1);
      }

      case XYTEL:                       /* TELNET */
        if ((z = cmkey(tntab,ntn,"parameter for TELNET negotiations", "",
                       xxstring)) < 0)
          return(z);
        switch (z) {
          case CK_TN_EC:                /* ECHO */
            if ((x = cmkey(rltab,nrlt,
                           "initial TELNET echoing state",
                           "local",xxstring)) < 0)
              return(x);
            if ((y = cmcfm()) < 0) return(y);
            tn_duplex = x;
            return(success = 1);

          case CK_TN_RE:                /* REMOTE-ECHO */
            return(success = seton(&tn_rem_echo));

          case CK_TN_DB:                /* DEBUG */
            return(success = seton(&tn_deb));

          case CK_TN_TT:                /* TERMINAL TYPE */
            if ((y = cmtxt("terminal type for TELNET connections","",
                           &s,xxstring)) < 0)
              return(y);
            if (tn_term) {
                free(tn_term);          /* Free any previous storage */
                tn_term = NULL;
            }
            if (s == NULL || *s == NUL) { /* If none given */
                tn_term = NULL;         /* remove the override string */
                return(success = 1);
            } else if ((tn_term = malloc(strlen(s)+1))) {
                strcpy(tn_term,s);
                return(success = 1);
            } else return(success = 0);

#ifdef CK_FORWARD_X
          case CK_TN_FX:                /* FORWARD-X */
            if ((x=cmkey(tnfwdxtab,ntnfwdx,"","xauthority-file",xxstring)) < 0)
              return(x);
            switch (x) {
              case 0: {                 /* Xauthority-File */
                  x = cmifi("Full path of .Xauthority file","",&s,&y,xxstring);
                  if (x < 0 && x != -3)
                    return(x);
                  makestr(&tn_fwdx_xauthority,s);
                  return(success = 1);
              }
              case 1: {                 /* No-Encryption */
                  extern int fwdx_no_encrypt;
                  return(success = seton(&fwdx_no_encrypt));
              }
            }
            return(success = 0);
#endif /* CK_FORWARD_X */

          case CK_TN_NL:                /* TELNET NEWLINE-MODE */
            if ((x = cmkey(tn_nlmtab,ntn_nlm,"","nvt",xxstring)) < 0)
              return(x);
            if (x == TN_NL_BIN) {
              if ((x = cmkey(tnlmtab,ntnlm,"","raw",xxstring)) < 0)
                return(x);
              if ((y = cmcfm()) < 0)
                return(y);
              tn_b_nlm = x;
              return(success = 1);
          } else if (x == TN_NL_NVT) {
              if ((x = cmkey(tnlmtab,ntnlm,"","on",xxstring)) < 0)
                return(x);
              if ((y = cmcfm()) < 0)
                return(y);
              tn_nlm = x;
              return(success = 1);
          } else {
              if ((y = cmcfm()) < 0)
                return(y);
              tn_nlm = x;
              return(success = 1);
          }

        case CK_TN_XF:                  /* BINARY-TRANSFER-MODE */
            if ((z = cmkey(onoff,2,"","on",xxstring)) < 0) return(z);
            if ((y = cmcfm()) < 0) return(y);
            tn_b_xfer = z;
            return(success = 1);

        case CK_TN_NE:                  /* NO-ENCRYPT-DURING-XFER */
            if ((z = cmkey(onoff,2,"","on",xxstring)) < 0) return(z);
            if ((y = cmcfm()) < 0) return(y);
#ifdef CK_APC
            /* Don't let this be set remotely */
            if (apcactive == APC_LOCAL ||
                (apcactive == APC_REMOTE && !(apcstatus & APC_UNCH)))
              return(success = 0);
#endif /* CK_APC */
            tn_no_encrypt_xfer = z;
            return(success = 1);

          case CK_TN_BM:                /* BINARY-MODE */
            if ((x = cmkey(tnnegtab,ntnnegtab,"","refused",xxstring)) < 0)
              return(x);
            if ((y = cmcfm()) < 0)
              return(y);
            TELOPT_DEF_S_ME_MODE(TELOPT_BINARY) = x;
            TELOPT_DEF_S_U_MODE(TELOPT_BINARY) = x;
            TELOPT_DEF_C_ME_MODE(TELOPT_BINARY) = x;
            TELOPT_DEF_C_U_MODE(TELOPT_BINARY) = x;
            return(success = 1);

#ifdef IKS_OPTION
          case CK_TN_IKS:               /* KERMIT */
            if ((x = cmkey(tnnegtab,ntnnegtab,"DO","accept",xxstring)) < 0)
              return(x);
            if ((y = cmkey(tnnegtab,ntnnegtab,"WILL","accept",xxstring)) < 0)
              return(y);
            if ((z = cmcfm()) < 0)
              return(z);
            TELOPT_DEF_S_ME_MODE(TELOPT_KERMIT) = y;
            TELOPT_DEF_S_U_MODE(TELOPT_KERMIT) = x;
            TELOPT_DEF_C_ME_MODE(TELOPT_KERMIT) = y;
            TELOPT_DEF_C_U_MODE(TELOPT_KERMIT) = x;
            return(success = 1);
#endif /* IKS_OPTION */

#ifdef CK_SSL
          case CK_TN_TLS:               /* START_TLS */
            if ((x = cmkey(tnnegtab,ntnnegtab,"me","accept",xxstring)) < 0)
              return(x);
            if ((y = cmkey(tnnegtab,ntnnegtab,"u","accept",xxstring)) < 0)
              return(y);
            if ((z = cmcfm()) < 0)
              return(z);
            TELOPT_DEF_S_ME_MODE(TELOPT_START_TLS) = x;
            TELOPT_DEF_S_U_MODE(TELOPT_START_TLS) = y;
            TELOPT_DEF_C_ME_MODE(TELOPT_START_TLS) = x;
            TELOPT_DEF_C_U_MODE(TELOPT_START_TLS) = y;
            return(success = 1);
#endif /* CK_SSL */

#ifdef CK_NAWS
          case CK_TN_NAWS:              /* NAWS */
            if ((x = cmkey(tnnegtab,ntnnegtab,"me","accept",xxstring)) < 0)
              return(x);
            if ((y = cmkey(tnnegtab,ntnnegtab,"u","accept",xxstring)) < 0)
              return(y);
            if ((z = cmcfm()) < 0)
              return(z);
            TELOPT_DEF_S_ME_MODE(TELOPT_NAWS) = x;
            TELOPT_DEF_S_U_MODE(TELOPT_NAWS) = y;
            TELOPT_DEF_C_ME_MODE(TELOPT_NAWS) = x;
            TELOPT_DEF_C_U_MODE(TELOPT_NAWS) = y;
            return(success = 1);
#endif /* CK_NAWS */

#ifdef CK_AUTHENTICATION
          case CK_TN_AU:                /* AUTHENTICATION */
            if ((x = cmkey(tnauthtab,ntnauth,"","",xxstring)) < 0)
              return(x);
            if (x == TN_AU_FWD) {
                extern int forward_flag;
                return(success = seton(&forward_flag));
            } else if (x == TN_AU_TYP) {
                extern int auth_type_user[];
                extern int sl_auth_type_user[];
                extern int sl_auth_saved;
                int i, j, atypes[AUTHTYPLSTSZ];

                for (i = 0; i < AUTHTYPLSTSZ; i++) {
                    if ((y = cmkey(autyptab,nautyp,"",
                                   i == 0 ? "automatic" : "" ,
                                   xxstring)) < 0) {
                        if (y == -3)
                          break;
                        return(y);
                    }
                    if (i > 0 && (y == AUTHTYPE_AUTO || y == AUTHTYPE_NULL)) {
                        printf(
                        "\r\n?Choice may only be used in first position.\r\n");
                        return(-9);
                    }
                    for (j = 0; j < i; j++) {
                        if (atypes[j] == y) {
                            printf("\r\n?Choice has already been used.\r\n");
                            return(-9);
                        }
                    }
                    atypes[i] = y;
                    if (y == AUTHTYPE_NULL || y == AUTHTYPE_AUTO) {
                        i++;
                        break;
                    }
                }
                if (i < AUTHTYPLSTSZ)
                  atypes[i] = AUTHTYPE_NULL;
                if ((z = cmcfm()) < 0)
                  return(z);
                sl_auth_saved = 0;
                for (i = 0; i < AUTHTYPLSTSZ; i++) {
                    auth_type_user[i] = atypes[i];
                    sl_auth_type_user[i] = 0;
                }
            } else if (x == TN_AU_HOW) {
                if ((y = cmkey(auhowtab,nauhow,"","any",xxstring)) < 0)
                  return(y);
                if ((z = cmcfm()) < 0)
                  return(z);
                tn_auth_how = y;
            } else if (x == TN_AU_ENC) {
                if ((y = cmkey(auenctab,nauenc,"","encrypt",xxstring)) < 0)
                  return(y);
                if ((z = cmcfm()) < 0)
                  return(z);
                tn_auth_enc = y;
            } else {
                if ((y = cmcfm()) < 0)
                  return(y);
                TELOPT_DEF_C_ME_MODE(TELOPT_AUTHENTICATION) = x;
                TELOPT_DEF_S_U_MODE(TELOPT_AUTHENTICATION) = x;
            }
            return(success = 1);
#endif /* CK_AUTHENTICATION */

#ifdef CK_ENCRYPTION
          case CK_TN_ENC: {             /* ENCRYPTION */
              int c, tmp = -1;
              int getval = 0;
              static struct keytab * tnetbl = NULL;
              static int ntnetbl = 0;

              if ((y = cmkey(tnenctab,ntnenc,"","",xxstring)) < 0)
                return(y);
              switch (y) {
                case TN_EN_TYP:
                  x = ck_get_crypt_table(&tnetbl,&ntnetbl);
                  debug(F101,"ck_get_crypt_table x","",x);
                  debug(F101,"ck_get_crypt_table n","",ntnetbl);
                  if (x < 1 || !tnetbl || ntnetbl < 1) /* Didn't get it */
                    x = 0;
                  if (!x) {
                      printf("?Oops, types not loaded\n");
                      return(-9);
                  }
                  if ((x = cmkey(tnetbl,ntnetbl,"type of encryption",
                                 "automatic",xxstring)) < 0)
                    return(x);
                  if ((z = cmcfm()) < 0)
                    return(z);
                  cx_type = x;
                  sl_cx_type = 0;
                  break;
                case TN_EN_START:
                  if ((z = cmcfm()) < 0)
                    return(z);
#ifdef CK_APC
                  /* Don't let this be set remotely */
                  if (apcactive == APC_LOCAL ||
                      apcactive == APC_REMOTE && !(apcstatus & APC_UNCH))
                    return(success = 0);
#endif /* CK_APC */
                  ck_tn_enc_start();
                  break;
                case TN_EN_STOP:
                  if ((z = cmcfm()) < 0)
                    return(z);
#ifdef CK_APC
                  /* Don't let this be set remotely */
                  if (apcactive == APC_LOCAL ||
                      apcactive == APC_REMOTE && !(apcstatus & APC_UNCH))
                    return(success = 0);
#endif /* CK_APC */
                  ck_tn_enc_stop();
                  break;
                default:
                  if ((z = cmcfm()) < 0)
                    return(z);
                  TELOPT_DEF_C_ME_MODE(TELOPT_ENCRYPTION) = y;
                  TELOPT_DEF_C_U_MODE(TELOPT_ENCRYPTION) = y;
                  TELOPT_DEF_S_ME_MODE(TELOPT_ENCRYPTION) = y;
                  TELOPT_DEF_S_U_MODE(TELOPT_ENCRYPTION) = y;
              }
              return(success = 1);
          }
#endif /* CK_ENCRYPTION */

          case CK_TN_BUG:               /* BUG */
            if ((x = cmkey(tnbugtab,4,"",
                           "binary-me-means-u-too",xxstring)) < 0)
              return(x);
            if ((z = cmkey(onoff,2,"","off",xxstring)) < 0) return(z);
            if ((y = cmcfm()) < 0) return(y);
            switch (x) {
              case 0:
                tn_b_meu = z;
                break;
              case 1:
                tn_b_ume = z;
                break;
              case 2:
                tn_infinite = z;
                break;
              case 3:
                tn_sb_bug = z;
                break;
              case 4:
                tn_auth_krb5_des_bug = z;
                break;
            }
            return(success = 1);

#ifdef CK_ENVIRONMENT
          case CK_TN_XD:                /* XDISPLOC */
            if ((x = cmkey(tnnegtab,ntnnegtab,"me","accept",xxstring)) < 0)
              return(x);
            if ((y = cmkey(tnnegtab,ntnnegtab,"u","accept",xxstring)) < 0)
              return(y);
            if ((z = cmcfm()) < 0)
              return(z);
            TELOPT_DEF_S_ME_MODE(TELOPT_XDISPLOC) = x;
            TELOPT_DEF_S_U_MODE(TELOPT_XDISPLOC) = y;
            TELOPT_DEF_C_ME_MODE(TELOPT_XDISPLOC) = x;
            TELOPT_DEF_C_U_MODE(TELOPT_XDISPLOC) = y;
            return(success = 1);

          case CK_TN_ENV: {
              char * msg = "value of telnet environment variable";
              extern int tn_env_flg;
              extern char tn_env_acct[], tn_env_disp[], tn_env_job[],
              tn_env_prnt[], tn_env_sys[];
              extern char * tn_loc;
              if ((x = cmkey(tnenvtab,ntnenv,"","",xxstring)) < 0)
                return(x);
              if (x == TN_ENV_UVAR) {   /* User variables */
                  char * uvar=NULL;
                  char * uval=NULL;
                  char * env;
                  extern char * tn_env_uservar[8][2];

                  /* Get the user variable name */
                  if ((x = cmfld("Name of Environment Variable","",&s,
                                 xxstring)) < 0)
                    return(x);
                  makestr(&uvar,s);

                  env = getenv(uvar);
                  if (!env) env = "";

                  if ((x = cmtxt("Value of Environment Variable",env,
                                 &s,xxstring)) < 0)
                    return(x);
                  if (*s)
                    makestr(&uval,s);

                  /* Now that we have the variable and perhaps a value */
                  /* there are three possibilities: (1) new variable   */
                  /* and associated value; (2) variable already exists */
                  /* but we have a new value; (3) variable already     */
                  /* exists but no new value therefore the user wants  */
                  /* to clear variable.                                */

                  /* Try to find an existing variable */
                  for (x = 0; x < 8; x++) {
                      if (!ckstrcmp(tn_env_uservar[x][0],uvar,-1,0)) {
                          if (uval) {
                              free(tn_env_uservar[x][1]);
                              tn_env_uservar[x][1] = uval;
                              free(uvar);
                              return(success = 1);
                          } else {
                              free(tn_env_uservar[x][0]);
                              tn_env_uservar[x][0] = NULL;
                              free(tn_env_uservar[x][1]);
                              tn_env_uservar[x][1] = NULL;
                              free(uvar);
                              return(success = 1);
                          }
                      }
                  }

                  /* Couldn't find one; look for empty location to insert */
                  for (x = 0; x < 8; x++) {
                      if (!tn_env_uservar[x][0]) {
                          tn_env_uservar[x][0] = uvar;
                          tn_env_uservar[x][1] = uval;
                          return(success = 1);
                      }
                  }
                  printf("?Sorry, no space for variable.\n");
                  return(success = 0);
              }
              if (x == TN_ENV_OFF || x == TN_ENV_ON) {
                  if ((y = cmcfm()) < 0) return(y);
#ifdef IKSD
                  if (inserver) {
                      printf("?Sorry, command disabled.\r\n");
                      return(success = 0);
                  }
#endif /* IKSD */
                  tn_env_flg = x == TN_ENV_OFF ? 0 : 1;
                  return(success = 1);
              }

              /* Not ON/OFF - Get the value */
              z = cmdgquo();
              cmdsquo(0);
              if ((y = cmtxt(msg, "", &s, xxstring)) < 0) {
                  cmdsquo(z);
                  return(y);
              }
              cmdsquo(z);
#ifdef IKSD
              if (inserver)
                return(success = 0);
#endif /* IKSD */
              if ((int)strlen(s) > 63) {
                  printf("Sorry, too long\n");
                  return(-9);
              }
              switch (x) {
                case TN_ENV_USR:
                  ckstrncpy(uidbuf,s,UIDBUFLEN);
                  sl_uid_saved = 0;
                  break;
                case TN_ENV_ACCT:
                  ckstrncpy(tn_env_acct,s,64);
                  break;
                case TN_ENV_DISP:
                  ckstrncpy(tn_env_disp,s,64);
                  break;
                case TN_ENV_JOB:
                  ckstrncpy(tn_env_job,s,64);
                  break;
                case TN_ENV_PRNT:
                  ckstrncpy(tn_env_prnt,s,64);
                  break;
                case TN_ENV_SYS:
                  ckstrncpy(tn_env_sys,s,64);
                  break;
                case TN_ENV_LOC:
                  if (!*s) s = NULL;
                  makestr(&tn_loc,s);
                  break;
                case TN_ENV_UVAR:
                  printf("\n?Not yet implemented\n");
                  break;
              }
              return(success = 1);
          }
#endif /* CK_ENVIRONMENT */

#ifdef CK_SNDLOC
          case CK_TN_LOC: {             /* LOCATION */
              extern char * tn_loc;
              if ((y = cmtxt("Location string","",&s,xxstring)) < 0)
                return(y);
              if (!*s) s = NULL;
              makestr(&tn_loc,s);
              return(success = 1);
          }
#endif /* CK_SNDLOC */
          case CK_TN_SFU:               /* Microsoft SFU compatibility */
            if ((z = cmkey(onoff,2,"","on",xxstring)) < 0) return(z);
            if ((y = cmcfm()) < 0) return(y);
            tn_sfu = z;
            return(success = 1);
            break;

          case CK_TN_WAIT:              /* WAIT-FOR-NEGOTIATIONS */
            if ((z = cmkey(onoff,2,"","on",xxstring)) < 0) return(z);
            if ((y = cmcfm()) < 0) return(y);
#ifdef IKSD
            if (inserver &&
#ifdef IKSDCONF
                iksdcf
#else
                1
#endif /* IKSDCONF */
                ) {
                printf("?Sorry, command disabled.\r\n");
                return(success = 0);
            }
#endif /* IKSD */
            tn_wait_flg = z;
            sl_tn_saved = 0;
            return(success = 1);

          case CK_TN_DL:                /* DELAY SUBNEGOTIATIONS */
            if ((z = cmkey(onoff,2,"","on",xxstring)) < 0) return(z);
            if ((y = cmcfm()) < 0) return(y);
#ifdef IKSD
            if (inserver &&
#ifdef IKSDCONF
                iksdcf
#else
                1
#endif /* IKSDCONF */
                ) {
                printf("?Sorry, command disabled.\r\n");
                return(success = 0);
            }
#endif /* IKSD */
            tn_delay_sb = z;
            return(success = 1);

          case CK_TN_PUID: {            /* PROMPT-FOR-USERID */
              int i,len;
              if ((y = cmtxt("Prompt string","",&s,xxstring)) < 0)
                return(y);
	      s = brstrip(s);
              /* we must check to make sure there are no % fields */
              len = strlen(s);
              for (i = 0; i < len; i++) {
                  if (s[i] == '%') {
                      if (s[i+1] != '%') {
                          printf("%% fields are not used in this command.\n");
                          return(-9);
                      }
                      i++;
                  }
              }
              makestr(&tn_pr_uid,s);
              return(success = 1);
          }
          default:
            return(-2);
        }
    }
#endif /* TNCODE */

    switch (xx) {
#ifndef NOSPL
      case XYCOUN:                      /* SET COUNT */
        x = cmnum("Positive number","0",10,&z,xxstring);
        if (x < 0) return(x);
        if ((x = cmcfm()) < 0) return(x);
        if (z < 0) {
            printf("?A positive number, please\n");
            return(0);
        }
        debug(F101,"XYCOUN: z","",z);
        return(success = setnum(&count[cmdlvl],z,0,10000));
#endif /* NOSPL */

#ifndef NOSPL
      case XYCASE:
        return(success = seton(&inpcas[cmdlvl]));
#endif /* NOSPL */

      case XYCMD:                       /* COMMAND ... */
        if ((y = cmkey(scmdtab,nbytt,"","",xxstring)) < 0)
          return(y);
        switch (y) {
          case SCMD_CBR:
            if ((y = cmcfm()) < 0)
              return(y);
            concb((char)escape);
            return(success = 1);

          case SCMD_BSZ:
            if ((y = cmnum("bytesize for command characters, 7 or 8","7",10,&x,
                           xxstring)) < 0)
              return(y);
            if (x != 7 && x != 8) {
                printf("\n?The choices are 7 and 8\n");
                return(success = 0);
            }
            if ((y = cmcfm()) < 0) return(y);
            if (x == 7) cmdmsk = 0177;
            else if (x == 8) cmdmsk = 0377;
            return(success = 1);
#ifdef CK_RECALL
          case SCMD_RCL:
            if ((y = cmnum("maximum number of commands in recall buffer","10",
                           10,&x,xxstring)) < 0)
              return(y);
            if ((y = cmcfm()) < 0) return(y);
            return(success = cmrini(x));
#endif /* CK_RECALL */
#ifdef CK_RECALL
          case SCMD_RTR:
            return(success = seton(&cm_retry));
#endif /* CK_RECALL */
          case SCMD_MOR:                /* More-prompting */
            success = seton(&xaskmore);
            if (success)
              saveask = xaskmore;
            return(success);
          case SCMD_QUO:
            if ((x = seton(&y)) < 0) return(x);
            cmdsquo(y);                 /* Do it the right way */
            cmd_quoting = y;            /* Also keep a global copy */
            /* Set string-processing function */
#ifdef datageneral
            xxstring = y ? zzstring : (xx_strp) NULL;
#else
#ifdef CK_ANSIC
            xxstring = y ? zzstring : (xx_strp) NULL;
#else
            xxstring = y ? zzstring : (xx_strp) NULL;
#endif /* CK_ANSIC */
#endif /* datageneral */
            return(success = 1);

#ifdef OS2
#ifndef NOLOCAL
          case SCMD_COL: {              /* Command-screen colors */
              int fg, bg;
              fg = cmkey(ttyclrtab, nclrs,
                         "foreground color and then background color",
                         "white",
                         xxstring);
              if (fg < 0)
                return(fg);
              if ((bg = cmkey(ttyclrtab,nclrs,
                              "background color","black",xxstring)) < 0)
                return(bg);
              if ((y = cmcfm()) < 0)
                return(y);
              colorcmd = fg | bg << 4;
              return(success = 1);
          }
          case SCMD_SCR:                /* Command Scrollback size */
            if ((y = cmnum("COMMAND scrollback buffer size, lines","512",10,&x,
                           xxstring)) < 0)
              return(y);
            /* The max number of lines is the RAM  */
            /* we can actually dedicate to a       */
            /* scrollback buffer given the maximum */
            /* process memory space of 512MB       */
            if (x < 256 || x > 2000000L) {
                printf("\n?The size must be between 256 and 2,000,000.\n");
                return(success = 0);
            }
            if ((y = cmcfm()) < 0) return(y);
            tt_scrsize[VCMD] = x;
            VscrnInit( VCMD );
            return(success = 1);

          case SCMD_WID: {
              if ((y = cmnum("Number of columns in display window",
                         "80",10,&x,xxstring)) < 0)
                return(y);
              if ((y = cmcfm()) < 0) return(y);

              os2_setcmdwidth(x);
              return(success = 1);
          }
          case SCMD_HIG:
            if ((y = cmnum("Number of rows in display window",
                           "24",10,&x,xxstring)) < 0)
              return(y);
            if ((y = cmcfm()) < 0) return(y);
            os2_setcmdheight(x);
            return(success = 1);

          case SCMD_STA: {
              extern int marginbot;
              if ((y = cmkey(onoff,2,"","on",xxstring)) < 0) return(y);
              if ((x = cmcfm()) < 0) return(x);
              if (y != tt_status[VCMD]) {
                  /* Might need to fixup the margins */
                  tt_status[VCMD] = y;
                  if (y) {
                      tt_szchng[VCMD] = 2;
                      tt_rows[VCMD]--;
                      cmd_rows--;
                      VscrnInit(VCMD);  /* Height set here */
                      printf("\n");
                  } else {
                      tt_szchng[VCMD] = 1;
                      tt_rows[VCMD]++;
                      cmd_rows++;
                      VscrnInit(VCMD);  /* Height set here */
                  }
              }
              return(success = 1);
          }

          case SCMD_CUR: {
              int row, col;
              position * ppos;

              ppos = VscrnGetCurPos(VCMD);
#ifdef NT
#define itoa _itoa
#endif /* NT */
              itoa(ppos->y+1, tmpbuf, 10);
              if ((y = cmnum("row (1-based)",tmpbuf,10,&row,xxstring)) < 0)
                return(y);

              itoa(ppos->x+1, tmpbuf, 10);
              if ((y = cmnum("col (1-based)",tmpbuf,10,&col,xxstring)) < 0)
                return(y);
              if ((x = cmcfm()) < 0) return(x);

              lgotoxy( VCMD, col, row ) ;
              VscrnIsDirty( VCMD );
              return(success=1);
          }
#endif /* NOLOCAL */
#else
          case SCMD_WID:
            y = cmnum("Command screen width, characters","80",10,&x,xxstring);
            return(setnum(&cmd_cols,x,y,1024));

          case SCMD_HIG:
            y = cmnum("Command screen height, rows","24",10,&x,xxstring);
            return(setnum(&cmd_rows,x,y,1024));
#endif /* OS2 */

          case SCMD_INT:
            return(seton(&cmdint));

#ifdef CK_AUTODL
          case SCMD_ADL:
            return(seton(&cmdadl));
#endif /* CK_AUTODL */

#ifdef DOUBLEQUOTING
          case SCMD_DBQ: {
              extern int dblquo;
              return(seton(&dblquo));
          }
#endif /* DOUBLEQUOTING */

	  case SCMD_ERR:
            y = cmnum("Error message verbosity level, 0-3","1",10,&x,xxstring);
            return(setnum(&cmd_err,x,y,3));

	  case SCMD_VAR:
	    return(setvareval());

          default:
            return(-2);
        }
    }

    switch (xx) {

      case XYDFLT:                      /* SET DEFAULT = CD */
        return(success = docd(XXCWD));

case XYDEBU:                            /* SET DEBUG { on, off, session } */
        if ((y = cmkey(dbgtab,ndbg,"","",xxstring)) < 0)
          return(y);
        if (y == DEB_TIM)
#ifdef COMMENT
          return(seton(&debtim) < 0 ? x : (success = 1));
#else
          /* why this change? */
          return(success = seton(&debtim));
#endif /* COMMENT */

#ifdef IKSD
        if (inserver && isguest) {
            printf("?Sorry, command disabled.\r\n");
            return(success = 0);
    }
#endif /* IKSD */

        switch (y) {
          case DEB_LEN:
            y = cmnum("Max length for debug log strings","",10,&x,xxstring);
            if ((z = setnum(&debxlen,x,y,-1)) < 0)
              return(z);
            if ((x = cmcfm()) < 0)
              return(x);
            return(success = 1);

          case DEB_OFF:
            if ((x = cmcfm()) < 0)
              return(x);
#ifndef NOLOCAL
            setdebses(0);
#endif /* NOLOCAL */
#ifdef DEBUG
            if (deblog) doclslog(LOGD);
#endif /* DEBUG */
            return(success = 1);

          case DEB_ON:
            if ((x = cmcfm()) < 0)
              return(x);
#ifdef DEBUG
            deblog = debopn("debug.log", 0);
            return(success = deblog ? 1 : 0);
#else
            printf("?Sorry, debug log feature not enabled\n");
            return(success = 0);
#endif /* DEBUG */
          case DEB_SES:
            if ((x = cmcfm()) < 0)
              return(x);
#ifndef NOLOCAL
            setdebses(1);
#endif /* NOLOCAL */
            return(success = 1);

	  case DEB_MSG:			/* Debug messages 2010/03/12 */
	    if ((y = cmkey(ooetab,nooetab,"","on",xxstring)) < 0) return(y);
	    if ((x = cmcfm()) < 0) return(x);
	    debmsg = y;
	    return(1);
        }
        break;

#ifndef NOXFER
      case XYDELA:                      /* SET DELAY */
        y = cmnum("Number of seconds before starting to send",
                  "5",10,&x,xxstring);
        if (x < 0) x = 0;
        return(success = setnum(&ckdelay,x,y,999));
#endif /* NOXFER */

      default:
        break;
    }

    switch (xx) {
#ifdef CK_TAPI
      case XYTAPI:
        return(settapi());
#endif /* CK_TAPI */
#ifndef NODIAL
      case XYDIAL:                      /* SET MODEM or SET DIAL */
        return(setdial(-1));
      case XYMODM:
        return(setmodem());
#ifdef COMMENT
      /* not implemented yet */
      case XYANSWER:                    /* SET ANSWER */
        return(setanswer());
#endif /* COMMENT */
#endif /* NODIAL */

#ifndef NOLOCAL
      case XYDUPL:                      /* SET DUPLEX */
        if ((y = cmkey(dpxtab,2,"","full",xxstring)) < 0) return(y);
        if ((x = cmcfm()) < 0) return(x);
        duplex = y;
        return(success = 1);

      case XYLCLE:                      /* LOCAL-ECHO (= DUPLEX) */
        return(success = seton(&duplex));

      case XYESC:                       /* SET ESCAPE */
        return(success = setcc(ckitoa(DFESC),&escape));
#endif /* NOLOCAL */

      case XYEXIT:                      /* SET EXIT */
        if ((z = cmkey(xittab,nexit,"","",xxstring)) < 0)
          return(z);
        switch (z) {
          case 0:                       /* STATUS */
            y = cmnum("EXIT status code","",10,&x,xxstring);
            return(success = setnum(&xitsta,x,y,-1));
          case 1:                       /* WARNING */
            if ((z = cmkey(xitwtab,nexitw,"","",xxstring)) < 0)
              return(z);
            if ((y = cmcfm()) < 0) return(y);
            setexitwarn(z);
            return(success = 1);
          case 2:
            success = seton(&exitonclose);
#ifdef TCPSOCKET
            if (success) tn_exit = exitonclose;
#endif /* TCPSOCKET */
            return(success);
          case 3: {
              extern int exithangup;
              return((success = seton(&exithangup)));
          }
          default:
            return(-2);
        } /* End of SET EXIT switch() */
      default:
        break;
    }

    switch (xx) {

      case XYFILE:                      /* SET FILE */
        return(setfil(rmsflg));

      case XYFLOW: {                    /* FLOW-CONTROL */
          extern int cxflow[];
          struct FDB k1, k2;
          int tncomport = 0;
          char * m;

#ifdef TN_COMPORT
          if (network && istncomport())
            tncomport = 1;
#endif /* TN_COMPORT */

          if (tncomport) {
              m = "Flow control type, one of the following:\n\
   dtr/cd    dtr/cts   keep    none    rts/cts   xon/xoff\n\
 or connection type";
          } else {
          /* All this is because chained FDB's don't give chained help yet */
              m =
#ifdef Plan9
#ifdef CK_RTSCTS
           "Flow control type, one of the following:\n\
   keep   none    rts/cts\n\
 or connection type",
#else
           "Flow control type, one of the following:\n\
   keep   none\n\
 or connection type";
#endif /* CK_RTSCTS */
#else
#ifdef CK_RTSCTS
#ifdef CK_DTRCD
#ifdef CK_DTRCTS
           "Flow control type, one of the following:\n\
   dtr/cd    dtr/cts   keep    none    rts/cts   xon/xoff\n\
 or connection type";
#else /* CK_DTRCTS */
           "Flow control type, one of the following:\n\
   dtr/cd    keep    none    rts/cts   xon/xoff\n\
            or connection type";
#endif /* CK_DTRCTS */
#else /* CK_DTRCD */
#ifdef CK_DTRCTS
           "Flow control type, one of the following:\n\
   dtr/cts   keep   none    rts/cts   xon/xoff\n\
 or connection type";
#else /* CK_DTRCTS */
           "Flow control type, one of the following:\n\
   keep   none    rts/cts   xon/xoff\n\
 or connection type";
#endif /* CK_DTRCTS */
#endif /* CK_DTRCD */
#else
           "Flow control type, one of the following:\n\
   keep   none    xon/xoff\n\
 or connection type";
#endif /* CK_RTSCTS */
#endif /* Plan9 */
          }
          cmfdbi(&k1,_CMKEY,m,"","",ncxtypesw, 4, xxstring, cxtypesw, &k2);
          cmfdbi(&k2,
                 _CMKEY,
                 "",
                 "",
                 "",
#ifdef TN_COMPORT
                 (tncomport ? ntnflo : nflo),
#else
                 nflo,
#endif /* TN_COMPORT */
                 0,
                 xxstring,
#ifdef TN_COMPORT
                 (tncomport ? tnflotab : flotab),
#else
                 flotab,
#endif /* TN_COMPORT */
                 NULL
                 );
          x = cmfdb(&k1);
          if (x < 0) {                  /* Error */
              if (x == -2 || x == -9)
                printf("?No keywords or switches match: \"%s\"\n",atmbuf);
              return(x);
          }
          z = cmresult.nresult;         /* Keyword value */
          if (cmresult.fdbaddr == &k2) { /* Flow-control type keyword table */
              if ((x = cmcfm()) < 0)    /* Set it immediately */
                return(x);
              flow = z;
              debug(F101,"set flow","",flow);
#ifdef CK_SPEED
              if (flow == FLO_XONX)     /* Xon/Xoff forces prefixing */
                ctlp[XON] = ctlp[XOFF] = ctlp[XON+128] = ctlp[XOFF+128] = 1;
#endif /* CK_SPEED */
              autoflow = (flow == FLO_AUTO);
              return(success = 1);      /* Done */
          }
          debug(F101,"set flow /blah 1","",z); /* SET FLOW /for-what */
          if ((y = cmkey(flotab,nflo,"Flow control type","none",xxstring)) < 0)
            return(y);
          if ((x = cmcfm()) < 0)
            return(x);
          debug(F101,"set flow /blah 2","",y);
          if (y == FLO_AUTO) {
              printf(
  "?Sorry, \"automatic\" can not be assigned to a connection type.\n");
              return(-9);
          } else if (z >= 0 && z <= CXT_MAX)
            cxflow[z] = y;
          debug(F101,"set flow","",flow);
          debug(F101,"set flow autoflow","",autoflow);
          return(success = 1);
      }

      case XYHAND:                      /* HANDSHAKE */
        if ((y = cmkey(hshtab,nhsh,"","none",xxstring)) < 0) return(y);
        if (y == 998) {
            if ((x = cmnum("ASCII value","",10,&y,xxstring)) < 0)
              return(x);
            if ((y < 1) || ((y > 31) && (y != 127))) {
                printf("?Character must be in ASCII control range\n");
                return(-9);
            }
        }
        if ((x = cmcfm()) < 0) return(x);
        turn = (y > 0127) ? 0 : 1;
        turnch = y;
        return(success = 1);

#ifndef NOSPL
      case XYMACR:                      /* SET MACRO */
        if ((y = cmkey(smactab,2,"","",xxstring)) < 0) return(y);
        switch (y) {
          case 0: return(success = seton(&mecho));
          case 1: return(success = seton(&merror[cmdlvl]));
          default: return(-2);
        }
#endif /* NOSPL */

      case XYMSGS:
#ifdef VMS
        if ((z = cmkey(onoff,2,"","",xxstring)) < 0) return(z);
        if ((y = cmcfm()) < 0) return(y);
        vms_msgs = z;
        printf("Sorry, SET MESSAGES not implemented yet\n");
        return(success = 0);
#endif /* VMS */
      default:
        break;
    }

    switch (xx) {

      case XYPARI:                      /* PARITY */
        if ((y = cmkey(partbl,npar,"","none",xxstring)) < 0)
          return(y);

        /* If parity not none, then we also want 8th-bit prefixing */

#ifdef HWPARITY
        if (y == 'H') {                 /* Hardware */
            if ((x = cmkey(hwpartbl,nhwpar,"","even",xxstring)) < 0)
              return(x);
        }
#endif /* HWPARITY */

        if ((z = cmcfm()) < 0)
          return(z);

#ifdef HWPARITY
        if (y == 'H') {                 /* 8 data bits plus hardware parity */
            parity = 0;
#ifndef NOXFER
            ebqflg = 0;
#endif /* NOXFER */
            hwparity = x;
        } else {                        /* 7 data bits + software parity */
            hwparity = 0;
#endif /* HWPARITY */
            parity = y;
#ifndef NOXFER
            ebqflg = (parity) ? 1 : 0;
#endif /* NOXFER */
#ifdef HWPARITY
        }
#endif /* HWPARITY */

#ifdef TN_COMPORT
        if (network && istncomport())
          tnsettings(parity, 0);
#endif /* TN_COMPORT */

        return(success = 1);

#ifndef NOFRILLS
      case XYPROM:                      /* SET PROMPT */
/*
  Note: xxstring not invoked here.  Instead, it is invoked every time the
  prompt is issued.  This allows the prompt string to contain variables
  that can change, like \v(dir), \v(time), etc.
*/
        ckmakmsg(line,                  /* Default might have trailing space */
                 LINBUFSIZ,
                 "{",
                 inserver ? ikprompt : ckprompt,
                 "}",
                 NULL
                 );
        if ((x = cmtxt("Program's command prompt",line,&s,NULL)) < 0)
          return(x);
        s = brstrip(s);                 /* Remove enclosing braces, if any */
        cmsetp(s);                      /* Set the prompt */
        return(success = 1);
#endif /* NOFRILLS */

#ifndef NOXFER
      case XYRETR:                      /* RETRY: per-packet retry limit */
        y = cmnum("Maximum retries per packet","10",10,&x,xxstring);
        if (x < 0) x = 0;
        if ((x = setnum(&maxtry,x,y,999)) < 0) return(x);
#ifdef COMMENT
        if (maxtry <= wslotr) {
            printf("?Retry limit must be greater than window size\n");
            return(success = 0);
        }
#endif /* COMMENT */
        if (rmsflg) {
            sstate = setgen('S', "403", ckitoa(maxtry), "");
            return((int) sstate);
        } else return(success = x);
#endif /* NOXFER */

#ifndef NOSERVER
      case XYSERV:                      /* SET SERVER items */
        if ((y = cmkey(srvtab,nsrvt,"","",xxstring)) < 0) return(y);
        switch (y) {
          case XYSERI:
            if ((y = cmnum("Number of seconds, or 0 for no idle timeout",
                           "0",10,&x,xxstring)) < 0)
              return(y);
            if (x < 0)
              x = 0;
            if ((y = cmcfm()) < 0)
              return(y);
#ifndef OS2
            srvtim = 0;
#endif /* OS2 */
            srvidl = x;
            return(success = 1);
          case XYSERT:
            if ((y = cmnum("Interval for server NAKs, 0 = none",
                           ckitoa(DSRVTIM),
                           10,&x, xxstring)) < 0)
              return(y);
            if (x < 0) {
                printf(
                   "\n?Specify a positive number, or 0 for no server NAKs\n");
                return(0);
            }
            if ((y = cmcfm()) < 0) return(y);
            if (rmsflg) {
                sstate = setgen('S', "404", ckitoa(x), "");
                return((int) sstate);
            } else {
#ifndef OS2
                srvidl = 0;
#endif /* OS2 */
                srvtim = x;             /* Set the server timeout variable */
                return(success = 1);
            }
          case XYSERD:                  /* SERVER DISPLAY */
            return(success = seton(&srvdis)); /* ON or OFF... */

#ifndef NOSPL
          case XYSERP:                  /* SERVER GET-PATH */
            return(parsdir(2));
#endif /* NOSPL */

          case XYSERL:                  /* SERVER LOGIN */
            return(cklogin());

          case XYSERC:                  /* SERVER CD-MESSAGE */
            x = rmsflg ?
              cmkey(onoff,2,"","",xxstring) :
                cmkey(cdmsg,3,"","",xxstring);
            if (x < 0)
              return(x);
            if (x == 2) {               /* CD-MESSAGE FILE */
                if ((x = cmtxt("Name of file","",&s,NULL)) < 0)
                  return(x);
                if (!*s) {
                    s = NULL;
                    srvcdmsg = 0;
                }
                makestr(&cdmsgstr,s);
                makelist(cdmsgstr,cdmsgfile,8);
                return(success = 1);
            }
            if ((y = cmcfm()) < 0)      /* CD-MESSAGE ON/OFF */
              return(y);
            if (rmsflg) {
                sstate = setgen('S', "420", x ? "1" : "0", "");
                return((int) sstate);
            } else {
                if (x > 0)
                  srvcdmsg |= 1;
                else
                  srvcdmsg &= 2;
                return(success = 1);
            }
          case XYSERK:                  /* SERVER KEEPALIVE */
            return(success = seton(&srvping)); /* ON or OFF... */

          default:
            return(-2);
        }
#endif /* NOSERVER */
    }

    switch (xx) {
#ifdef UNIX
#ifndef NOJC
      case XYSUSP:                      /* SET SUSPEND */
        seton(&xsuspend);		/* on or off... */
        return(success = 1);
#endif /* NOJC */
#endif /* UNIX */

      case XYTAKE:                      /* SET TAKE */
        if ((y = cmkey(taktab,4,"","",xxstring)) < 0) return(y);
        switch (y) {
          case 0: return(success = seton(&techo));
#ifndef NOSPL
          case 1: return(success = seton(&takerr[cmdlvl]));
#else
          case 1: return(success = seton(&takerr[tlevel]));
#endif /* NOSPL */
          case 2: techo = 0; return(success = 1); /* For compatibility with */
          case 3: techo = 1; return(success = 1); /* MS-DOS Kermit */
          default: return(-2);
        }

#ifndef NOSCRIPT
      case XYSCRI:                      /* SET SCRIPT */
        if ((y = cmkey(scrtab,1,"","echo",xxstring)) < 0) return(y);
        switch (y) {
          case 0: return(success = seton(&secho));
          default: return(-2);
        }
#endif /* NOSCRIPT */

      default:
        break;
    }

#ifndef NOLOCAL
    switch (xx) {
      case XYTERM:                      /* SET TERMINAL */
        x = settrm();
        success = (x > 0) ? 1 : 0;
        return(x);

#ifdef NT
      case XYWIN95:                     /* SET WIN95 workarounds */
        x = setwin95();
        success = (x > 0 ? 1 : 0);
        return(x);
#endif /* NT */

#ifdef OS2
      case XYDLR:                       /* SET DIALER workarounds */
        x = setdialer();
        success = (x > 0 ? 1 : 0);
        return(x);

      case XYTITLE:                     /* SET TITLE of window */
        x = settitle();
        success = (x > 0 ? 1 : 0);
        return(x);
#endif /* OS2 */

#ifdef OS2MOUSE
      case XYMOUSE:                     /* SET MOUSE */
        return(success = setmou());
#endif /* OS2MOUSE */

      case XYBELL:                      /* SET BELL */
        return(success = setbell());

#ifdef OS2
      case XYPRTY:
        return(success = setprty() );
#endif /* OS2 */

      default:
        break;
    }
#endif /* NOLOCAL */

    switch (xx) {

/* SET SEND/RECEIVE protocol parameters. */

#ifndef NOXFER
      case XYRECV:
      case XYSEND:
        return(setsr(xx,rmsflg));
#endif /* NOXFER */

#ifndef NOLOCAL
      case XYSESS:                      /* SESSION-LOG */
        if ((x = cmkey(sfttab,nsfttab,"type of file",
#ifdef OS2
                       "binary",
#else /* OS2 */
                       "text",
#endif /* OS2 */
                       xxstring
                       )
             ) < 0)
          return(x);
        if ((y = cmcfm()) < 0)
          return(y);
        if (x == 999) {                 /* TIMESTAMPED-TEXT */
            sessft = XYFT_T;            /* Implies text */
            slogts = 1;                 /* and timestamps */
	} else if (x == 998) {		/* NULL-PADDED-LINES */
            slognul = 1;		/* adds NUL after ^J */
        } else {                        /* A regular type */
            sessft = x;                 /* The type */
            slogts = 0;                 /* No timestampes */
        }
        return(success = 1);

      case XYSPEE:                      /* SET SPEED */
        lp = line;
        if (local && !network) {
          ckmakmsg(lp,
                   LINBUFSIZ,
                   "Transmission rate for ",
                   ttname,
                   " (bits per second)",
                   NULL
                   );
        } else {
          ckstrncpy(lp,
                    "Serial-port speed (bits per second)",
                    LINBUFSIZ
                    );
        }
        zz = -1L;

#ifdef TN_COMPORT
        if (network && istncomport())
          x = cmkey(tnspdtab,ntnspd,line,"",xxstring);
        else
#endif /* TN_COMPORT */
          x = cmkey(spdtab,nspd,line,"",xxstring);
        if (x < 0) {
            if (x == -3) printf("?value required\n");
#ifdef USETCSETSPEED
            /* In this case, any number can be tried */
            /* There's a parse error message but the request still goes thru */
            if (rdigits(atmbuf))
              zz = atol(atmbuf);
            else
#endif /* USETCSETSPEED */
              return(x);
        }
        if ((y = cmcfm()) < 0) return(y);
#ifdef IKSD
        if (inserver) {
            printf("?Sorry, command disabled.\r\n");
            return(success = 0);
        }
#endif /* IKSD */
        if (!local) {
            printf("?SET SPEED has no effect without prior SET LINE\n");
            return(success = 0);
        } else if (network
#ifdef TN_COMPORT
                   && !istncomport()
#endif /* TN_COMPORT */
                   ) {
            printf("\n?Speed cannot be set for network connections\n");
            return(success = 0);
        }

/*
  Note: This way of handling speeds is not 16-bit safe for speeds greater
  than 230400.  The argument to ttsspd() should have been a long.
*/
#ifdef USETCSETSPEED
        if (zz > -1L)
          x = zz / 10L;
#endif /* USETCSETSPEED */
        zz = (long) x * 10L;
        if (zz == 130L) zz = 134L;
        if (zz == 70L) zz = 75L;        /* (see spdtab[] definition) */
        if (ttsspd(x) < 0)  {           /* Call ttsspd with cps, not bps! */
            printf("?Unsupported line speed - %ld\n",zz);
            return(success = 0);
        } else {
#ifdef CK_TAPI
            if (!tttapi || tapipass)
              speed = ttgspd();         /* Read it back */
            else
              speed = zz;
#else /* CK_TAPI */
            speed = ttgspd();           /* Read it back */
#endif /* CK_TAPI */
            if (speed != zz)  {         /* Call ttsspd with cps, not bps! */
                printf("?SET SPEED fails, speed is %ld\n",speed);
                return(success = 0);
            }
            if (pflag && !xcmdsrc) {
                if (speed == 8880)
                  printf("%s, 75/1200 bps\n",ttname);
                else if (speed == 134)
                  printf("%s, 134.5 bps\n",ttname);
                else
                  printf("%s, %ld bps\n",ttname,speed);
            }
            return(success = 1);
        }
#endif /* NOLOCAL */

#ifndef NOXFER
      case XYXFER:                      /* SET TRANSFER */
        if ((y = cmkey(rmsflg ? rtstab : tstab, /* (or REMOTE SET TRANSFER) */
                       rmsflg ? nrts : nts,
                       "","character-set",xxstring)) < 0) return(y);
        switch (y) {
#ifdef XFRCAN
          case XYX_CAN:                 /* CANCELLATION */
            if ((z = cmkey(onoff,2,"","",xxstring)) < 0) return(z);
            if (z == 0) {               /* OFF */
                if ((y = cmcfm()) < 0) return(y);
                xfrcan = 0;
            } else {
                if ((y = cmnum("ASCII code for cancellation character",
                               "3",10,&x,
                               xxstring)) < 0)
                  return(y);
                if (x > 31 && x != 127) {
                    printf("Cancel character must be 0-31 or 127\n");
                    return(-9);
                }
                if ((y = cmnum("How many required to cause cancellation",
                               "2",10,&z, xxstring)) < 0)
                  return(y);
                if (z < 2) {
                    printf("Number must be 2 or greater\n");
                    return(-9);
                }
                if ((y = cmcfm()) < 0) return(y);
                xfrcan = 1;             /* CANCELLATION ON */
                xfrchr = x;             /* Using this character */
                xfrnum = z;             /* Needing this many of them */
            }
            return(success = 1);
#endif /* XFRCAN */

#ifndef NOCSETS
          case XYX_CSE:                 /* CHARACTER-SET */
            if ((y = cmkey(tcstab,ntcs,"","transparent",xxstring)) < 0)
              return(y);
            if ((x = cmcfm()) < 0) return(x);
            if (rmsflg) {
                sstate = setgen('S', "405", tcsinfo[y].designator, "");
                return((int) sstate);
            } else {
                extern int s_cset, fcharset, axcset[], tcs_save;
                tslevel = (y == TC_TRANSP) ? 0 : 1; /* transfer syntax level */
		xfrxla = tslevel;
                tcharset = y;           /* transfer character set */
                /* SEND CHARACTER-SET AUTO */
                if (tslevel > 0 && s_cset == XMODE_A)
                  if (y > -1 && y <= MAXTCSETS)
                    if (axcset[y] > -1 && axcset[y] > MAXFCSETS)
                      fcharset = axcset[y]; /* Auto-pick file charset */
                setxlatype(tcharset,fcharset); /* Translation type */
		tcs_save = -1;
                return(success = 1);
            }
#endif /* NOCSETS */

          case XYX_LSH:                 /* LOCKING-SHIFT */
            if ((y = cmkey(lstab,nls,"","on",xxstring)) < 0)
              return(y);
            if ((x = cmcfm()) < 0) return(x);
            lscapr = (y == 1) ? 1 : 0;  /* ON: requested = 1 */
            lscapu = (y == 2) ? 2 : 0;  /* FORCED:  used = 1 */
            return(success = 1);

/* #ifdef CK_XYZ */
          case XYX_PRO:                 /* Protocol */
#ifndef OS2
            if (inserver) {
                printf("?Sorry, only Kermit protocol is available\n");
                return(-9);
            }
#endif /* OS2 */
            return(setproto());
/* #endif */ /* CK_XYZ */

          case XYX_MOD:                 /* Mode */
            if ((y = cmkey(xfrmtab,2,"","automatic",xxstring)) < 0)
              return(y);
            if ((x = cmcfm()) < 0) return(x);
            if (rmsflg) {
                sstate = setgen('S', "410", y == XMODE_A ? "0" : "1", "");
                return((int)sstate);
            }
            g_xfermode = y;
            xfermode = y;
#ifdef NEWFTP
	    if (ftpisopen()) {		/* If an FTP connection is open */
		extern int ftp_xfermode; /* change its transfer mode too */
		ftp_xfermode = xfermode;
	    }	      
#endif	/* NEWFTP */
            return(success = 1);

#ifndef NOLOCAL
          case XYX_DIS:                 /* Display */
            return(doxdis(1));		/* 1 == Kermit */
#endif /* NOLOCAL */

          case XYX_SLO:                 /* Slow-start */
            return(seton(&slostart));

#ifndef NOSPL
          case XYX_CRC:                 /* CRC */
            return(seton(&docrc));
#endif /* NOSPL */

          case XYX_BEL:                 /* Bell */
            return(seton(&xfrbel));

#ifdef PIPESEND
          case XYX_PIP:                 /* Pipes */
#ifndef NOPUSH
            if (nopush) {
#endif /* NOPUSH */
                printf("Sorry, access to pipes is disabled\n");
                return(-9);
#ifndef NOPUSH
            } else
#endif /* NOPUSH */
              return(seton(&usepipes));
#endif /* PIPESEND */

          case XYX_INT:                 /* Interruption */
            return(seton(&xfrint));

          case XYX_XLA:
            return(seton(&xfrxla));     /* Translation */

          case XYX_MSG: {
              extern char * xfrmsg;
              if ((x = cmtxt("Prompt string","",&s,xxstring)) < 0)
                return(x);
              if (!*s) s = NULL;
              makestr(&xfrmsg,s);
              return(success = 1);

	  }
	  case XYX_RPT: {
	      extern int whereflg;
	      return(seton(&whereflg));
          }
          default:
            return(-2);
        }
#endif /* NOXFER */
    }

    switch (xx) {

#ifndef NOXMIT
      case XYXMIT:                      /* SET TRANSMIT */
        return(setxmit());
#endif /* NOXMIT */

#ifndef NOXFER
#ifndef NOCSETS
      case XYUNCS:                      /* UNKNOWN-CHARACTER-SET */
        if ((y = cmkey(ifdtab,2,"","discard",xxstring)) < 0) return(y);
        if ((x = cmcfm()) < 0) return(x);
        unkcs = y;
        return(success = 1);
#endif /* NOCSETS */
#endif /* NOXFER */

#ifndef NOPUSH
#ifdef UNIX
      case XYWILD:                      /* WILDCARD-EXPANSION */
        if ((y = cmkey(wildtab,nwild,
                       "Wildcard expansion option","on",xxstring)) < 0)
          return(y);
        if ((z = cmkey(wdottab,
                       2,
                       "whether to match filenames that start with \".\"",
                       "/no-match-dot-files",
                       xxstring)
             ) < 0)
          return(z);
        if ((x = cmcfm()) < 0) return(x);
        if (nopush) {
            if (y > 0) {
                printf("Shell expansion is disabled\n");
                return(success = 0);
            }
        }
	switch (y) {
	  case WILD_ON:
	    wildena = 1; 
	    break;
	  case WILD_OFF:
	    wildena = 0; 
	    break;
	  case WILD_KER:
	    wildxpand = 0; 		/* These are the previous */
	    break;			/* hardwired values */
	  case WILD_SHE:
	    wildxpand = 1; 
	    break;
	}
        matchdot = z;
        return(success = 1);
#endif /* UNIX */
#endif /* NOPUSH */

#ifndef NOXFER
      case XYWIND:                      /* WINDOW-SLOTS */
        if (protocol == PROTO_K) {
            y = cmnum("Window size for Kermit protocol, 1 to 32",
                      "1", 10, &x, xxstring);
            y = setnum(&z,x,y,MAXWS);   /* == 32 */
        }
#ifdef CK_XYZ
        else if (protocol == PROTO_Z) {
            y = cmnum("Window size for ZMODEM protocol, 0 to 65535",
                      "0", 10, &x, xxstring);
            y = setnum(&z,x,y,65535);
        }
#endif /* CK_XYZ */
        else {
            y = cmnum("Window size for current protocol",
                      "", 10, &x, xxstring);
            y = setnum(&z,x,y,65472);   /* Doesn't matter - we won't use it */
        }
        if (y < 0) return(y);
        if (protocol == PROTO_K) {
            if (z < 1)
              z = 1;
        }
#ifdef CK_XYZ
        else if (protocol == PROTO_Z) {
            /* Zmodem windowing is closer to Kermit packet length */
            /* than Kermit window size.  If Window size is zero   */
            /* an end of frame and CRC is sent only at the end of */
            /* the file (default).  Otherwise, an End of Frame    */
            /* and CRC are sent after Window Size number of bytes */
            if (z < 0)                  /* Disable windowing  */
              z = 0;
        } else {
            printf("?SET WINDOW does not apply to %s protocol\n",
                   ptab[protocol].p_name
                   );
        }
#endif /* CK_XYZ */

#ifdef COMMENT
        /* This is taken care of automatically now in protocol negotiation */
        if (maxtry < z) {
            printf("?Window slots must be less than retry limit\n");
            return(success = 0);
        }
#endif /* COMMENT */
        if (protocol == PROTO_K && rmsflg) { /* Set remote window size */
            wslotr = z;                 /* Set local window size too */
            ptab[protocol].winsize = wslotr;
            sstate = setgen('S', "406", ckitoa(z), "");
            return((int) sstate);
        }
        wslotr = z;                     /* Set requested window size here */
        ptab[protocol].winsize = wslotr; /* and in protocol-specific table */
        if (protocol == PROTO_K) {      /* And for Kermit only... */
            swcapr = (wslotr > 1) ? 1 : 0; /* set window bit in capas word */
            if (wslotr > 1) {           /* Window size > 1? */
                /* Maybe adjust packet size */
                y = adjpkl(urpsiz,wslotr,bigrbsiz);
                if (y != urpsiz) {      /* Did it change? */
                    urpsiz = y;
                    if (msgflg)
                      printf(
" Adjusting receive packet-length to %d for %d window slots\n",
                             urpsiz,
                             wslotr
                             );
                }
            }
        }
        return(success = 1);
#endif /* NOXFER */
    }

    switch (xx) {

#ifndef NOSPL
      case XYOUTP:                      /* OUTPUT command parameters */
        if ((y = cmkey(outptab,noutptab,"OUTPUT command parameter","pacing",
                       xxstring)) < 0)
          return(y);
        switch(y) {                     /* Which parameter */
          case OUT_PAC:                 /* PACING */
            y = cmnum("Milliseconds to pause between each OUTPUT character",
                      "100", 10,&x,xxstring);
            y = setnum(&z,x,y,16383);   /* Verify and get confirmation */
            if (y < 0) return(y);
            if (z < 0) z = 0;           /* (save some space) */
            pacing = z;
            return(success = 1);
          case OUT_ESC:                 /* Special-escapes */
            return(seton(&outesc));
          default:                      /* (shouldn't happen) */
            return(-2);
        }
#endif /* NOSPL */

#ifdef CK_SPEED
      case XYQCTL: {
          short *p;
          int zz;
          if ((z = cmkey(ctltab,2, "control-character prefixing option",""
                         ,xxstring)) < 0)
            return(z);
          /* Make space for a temporary copy of the prefixing table */

          p = (short *)malloc(256 * sizeof(short));
          if (!p) {
              printf("?Internal error - malloc failure\n");
              return(-9);
          }
          for (i = 0; i < 256; i++) p[i] = ctlp[i]; /* Copy current table */

          switch (z) {
            case 0:                     /* UNPREFIXED control character */
            case 1:                     /* PREFIXED control character */
              while (1) {               /* Collect a list of numbers */
#ifndef NOSPL
                  x_ifnum = 1;          /* Turn off complaints from eval() */
#endif /* NOSPL */
                  if ((x = cmnum((z == 0) ?
"\n Numeric ASCII value of control character that needs NO prefix,\n\
 or the word \"all\", or carriage return to complete the list" :
"\n Numeric ASCII value of control character that MUST BE prefixed,\n\
 or the word \"all\", or carriage return to complete the list",
                                 "",10,&y,xxstring
                                 )) < 0) {
#ifndef NOSPL
                      x_ifnum = 0;
#endif /* NOSPL */
                      if (x == -3) {
                          if ((x = cmcfm()) < 0) return(x);
                          break;
                      }
                      if (x == -2) {
                          if (p) { free((char *)p); p = NULL; }
                          debug(F110,"SET CONTROL atmbuf",atmbuf,0);
                          if (!ckstrcmp(atmbuf,"all",3,0) || /* "ALL" */
                              !ckstrcmp(atmbuf,"al",2,0) ||
                              !ckstrcmp(atmbuf,"a",1,0)) {
                              if ((x = cmcfm()) < 0) /* Get confirmation */
                                return(x);
			      prefixing = z ? PX_ALL : PX_NON;
			      setprefix(prefixing);
			      return(success = 1);
                          } else {	/* Not number, not ALL */
                              printf(
                                 "?Please specify a number or the word ALL\n");
                              return(-9);
                          }
                      } else {
                          if (p) free((char *)p);
                          return(x);
                      }
                  }
#ifndef NOSPL
                  x_ifnum = 0;
#endif /* NOSPL */
#ifdef UNPREFIXZERO
                  zz = 0;
#else
#ifndef OS2
                  zz = 1 - z;
#else
                  zz = 0;               /* Allow 0 (but only for Zmodem) */
#endif /* OS2 */
#endif /* UNPREFIXZERO */

            /* printf("x = %d, y = %d, z = %d, zz = %d\n", x,y,z,zz); */

                  if ((y >  31 && y < 127) || /* A specific numeric value */
                      (y > 159 && y < 255) || /* Check that it is a valid */
                      (y < zz) ||       /* control code. */
                      (y > 255)) {
                      printf("?Values allowed are: %d-31, 127-159, 255\n",zz);
                      if (p) free((char *)p);
                      return(-9);
                  }
                  x = y & 127;          /* Get 7-bit value */
                  if (z == 0) {         /* If they are saying it is safe... */
                      /* If flow control is Xon/Xoff */
                      if (((flow == FLO_XONX) &&
                           /* XON & XOFF chars not safe. */
                           (x == XON || x == XOFF))
                          ) {
                          if (msgflg)
                            printf(
                              "Sorry, not while Xon/Xoff is in effect.\n");
                          if (p) free((char *)p);
                          return(-9);
                      }
#ifdef TNCODE
                      else if (network && IS_TELNET()
                               && (y == CR ||
                                   (unsigned) y == (unsigned) 255)) {
                          if (msgflg)
                            printf("Sorry, not on a TELNET connection.\n");
                          if (p) free((char *)p);
                          return(-9);
                      }
#endif /* TNCODE */
                  }
                  p[y] = (char) z;      /* All OK, set flag */
              } /* End of while loop */
/*
  Get here only if they have made no mistakes.  Copy temporary table back to
  permanent one, then free temporary table and return successfully.
*/
              for (i = 0; i < 256; i++) ctlp[i] = p[i];
              if (p) free((char *)p);
              if (z > 0) clearrq = 0;   /* 199 (see SET PREFIXING) */
              return(success = 1);
            default:
              return(-2);
          }
      }
#endif /* CK_SPEED */
    }

    switch (xx) {

#ifndef NOXFER
      case XYREPT:
        if ((y = cmkey(rpttab,2,
                       "repeat-count compression parameter","",xxstring)) < 0)
          return(y);
        switch(y) {
          case 0:
            return(success = seton(&rptena)); /* REPEAT COUNTS = ON, OFF */
          case 1:                       /* REPEAT MININUM number */
            printf("(not implemented yet, nothing happens)\n");
            return(-9);
          case 2:                       /* REPEAT PREFIX char */
            if ((x = cmnum("ASCII value","",10,&z,xxstring)) < 0)
              return(x);
            if ((x = cmcfm()) < 0) return(x);
            if ((z > 32 && z < 63) || (z > 95 && z < 127)) {
                if (y == 1) rptmin = (CHAR) z; else myrptq = (CHAR) z;
                return(success = 1);
            } else {
                printf("?Illegal value for prefix character\n");
                return(-9);
            }
        }
#endif /* NOXFER */

#ifndef NOSPL
      case XYALRM: {
#ifndef COMMENT
          int yy;
          long zz;
          zz = -1L;
          yy = x_ifnum;
          x_ifnum = 1;                  /* Turn off internal complaints */
          y = cmnum("Seconds from now, or time of day as hh:mm:ss",
                    "0" ,10, &x, xxstring);
          x_ifnum = yy;
          if (y < 0) {
              if (y == -2) {            /* Invalid number or expression */
                  zz = tod2sec(atmbuf); /* Convert to secs since midnight */
                  if (zz < 0L) {
                      printf("?Number, expression, or time of day required\n");
                      return(-9);
                  } else {
                      char now[32];     /* Current time */
                      char *p;
                      long tnow;
                      p = now;
                      ztime(&p);
                      tnow = atol(p+11) * 3600L +
                        atol(p+14) * 60L + atol(p+17);
                      if (zz < tnow)    /* User's time before now */
                        zz += 86400L;   /* So make it tomorrow */
                      zz -= tnow;       /* Seconds from now. */
                  }
              } else
                return(y);
          }
          if (x < 0) {
              printf("?Alarm time is in the past.\n");
              return(-9);
          }
          if ((y = cmcfm()) < 0) return(y);
          if (zz > -1L) {               /* Time of day given? */
              x = zz;
              if (zz != (long) x) {
                  printf(
"Sorry, arithmetic overflow - hh:mm:ss not usable on this platform.\n"
                         );
                  return(-9);
              }
          }
          return(setalarm((long)x));
      }
#else
/*
  This is to allow long values where int and long are not the same, e.g.
  on 16-bit systems.  But something is wrong with it.
*/
        if ((y = cmtxt("seconds from now", "0", &s, xxstring)) < 0)
          return(y);
        if (rdigits(s)) {
            return(setalarm(atol(s)));
        } else {
            printf("%s - not a number\n",s);
            return(-9);
        }
#endif /* COMMENT */
#endif /* NOSPL */

#ifndef NOXFER
      case XYPROTO:
        return(setproto());
#endif /* NOXFER */

/*
  C-Kermit unprefixes control characters automatically on network connections
  if CLEAR-CHANNEL is ON, which it is by default.  But not all network
  connections are transparent to all control characters.  For example, the
  DEC-20, even when you TELNET to it, is sensitive to Ctrl-O and Ctrl-T.
  If you tell C-Kermit to SET CONTROL PREFIX 15 and/or 20, it doesn't help
  because CLEAR-CHANNEL is still in effect.  If the user goes to the trouble
  to set up some prefixing, then Kermit should do what the user said.  In
  C-Kermit 7.1 Alpha.03 we change the code to set clearrq to 0 if the user
  gives a SET PREFIXING or SET CONTROL PREFIX command.
*/

#ifdef CK_SPEED
      case XYPREFIX: {
#ifdef COMMENT
          extern int clearrq;
#endif /* COMMENT */
          if ((z = cmkey(pfxtab, 4, "control-character prefixing option",
                         "", xxstring)) < 0)
            return(z);
          if ((x = cmcfm()) < 0) return(x);
          clearrq = 0;                  /* 199 */
          setprefix(z);
#ifdef COMMENT
          if (hints && (z == PX_ALL || z == PX_CAU) && clearrq) {
        printf("Hint: Use SET CLEAR-CHANNEL OFF to disable negotiation of\n");
        printf("      SET PREFIXING NONE during file transfers on reliable\n");
        printf("      connections.\n");
          }
#endif /* COMMENT */
          return(success = 1);
      }
#endif /* CK_SPEED */

#ifndef NOSPL
      case XYLOGIN:
        if ((z = cmkey(logintab, 3, "value for login script","userid",
                       xxstring)) < 0)
          return(z);
        x = cmdgquo();
        if (z == LOGI_PSW)
          cmdsquo(0);
        if ((y = cmtxt("text","", &s, NULL)) < 0) {
            cmdsquo(x);
            return(y);
        }
        cmdsquo(x);
#ifdef IKSD
        if (inserver)
          return(success = 0);
#endif /* IKSD */
        s = brstrip(s);
        if ((int)strlen(s) > 63) {
            printf("Sorry, too long\n");
            return(-9);
        }
        switch(z) {
          case LOGI_UID:
            ckstrncpy(uidbuf,s,UIDBUFLEN);
            sl_uid_saved = 0;
            break;
          case LOGI_PSW:
            ckstrncpy(pwbuf,s,PWBUFL);
            if (pwbuf[0]) {
                pwflg = 1;
#ifdef OS2
                pwcrypt = 1;
#else /* OS2 */
                pwcrypt = 0;
#endif /* OS2 */
            }
            break;
          case LOGI_PRM:
            ckstrncpy(prmbuf,s,PWBUFL);
        }
        return(success = 1);
#endif /* NOSPL */
    }

    switch (xx) {

      case XYSTARTUP:
        if ((y = cmkey(ifdtab,2,"","discard",xxstring)) < 0) return(y);
        if ((x = cmcfm()) < 0) return(x);
        DeleteStartupFile = (y != 0) ? 0 : 1;
        return(success = 1);

      case XYTMPDIR:
        x = cmdir("Name of temporary directory","",&s,xxstring);
        if (x == -3)
          s = "";
        else if (x < 0)
          return(x);
        if ((x = cmcfm()) < 0) return(x);
        makestr(&tempdir,s);
        return(tempdir ? 1 : 0);

#ifndef NOXFER
      case XYDEST:                      /* DESTINATION */
        return(setdest());
#endif /* NOXFER */

#ifndef NOPUSH
#ifndef NOFRILLS

/* Editor, Browser, and FTP Client */

      case XYEDIT:                      /* EDITOR */
#ifdef IKSD
        if (inserver) {
            printf("?Sorry, command disabled.\r\n");
            return(success = 0);
        }
#endif /* IKSD */
#ifdef CK_APC
        /* Don't let this be set remotely */
        if (apcactive == APC_LOCAL ||
            (apcactive == APC_REMOTE && !(apcstatus & APC_UNCH)))
          return(success = 0);
#endif /* CK_APC */

#ifdef OS2ORUNIX
        {
            char *p = getenv("PATH");
            char *e;
            e = editor[0] ? (char *) editor : getenv("EDITOR");
            if (!e) e = "";
            if (p)
              x = cmifip("Name of preferred editor",e,&s,&y,0,p,xxstring);
            else
              x = cmifi("Full path of preferred editor",e,&s,&y,xxstring);
            if (x < 0 && x != -3)
              return(x);
        }
#else
#ifdef VMS
        if ((y = cmtxt("DCL command for editing", "edit", &s, NULL)) < 0) {
            if (x != -3)
              return(x);
        }
#else
        if ((x = cmifi("Full path of preferred editor","",&s,&y,xxstring))<0) {
            if (x != -3)
              return(x);
        }
#endif /* VMS */
#endif /* OS2ORUNIX */
#ifdef VMS
        ckstrncpy(editor,s,CKMAXPATH);
        editopts[0] = NUL;
#else
        if (y != 0) {
            printf("?A single file please\n");
            return(-2);
        }
        ckstrncpy(line,s,LINBUFSIZ);
        if ((x = cmtxt("editor command-line options","",&s,NULL)) < 0)
          return(x);
        ckstrncpy(tmpbuf,s,TMPBUFSIZ);
        if ((z = cmcfm()) < 0) return(z);
        if (line[0]) {
            zfnqfp(line,CKMAXPATH,editor);
            ckstrncpy(editopts,tmpbuf,128);
        } else {
            editor[0] = NUL;
            editopts[0] = NUL;
        }
#endif /* VMS */
        return(success = 1);

#ifndef NOFTP
#ifndef SYSFTP
#ifdef TCPSOCKET
      case XYFTPX:
        return(dosetftp());             /* SET FTP */
#endif /* TCPSOCKET */
#endif /* SYSFTP */
#endif /* NOFTP */

#ifdef BROWSER
#ifndef NOFTP
#ifdef SYSFTP
      case XYFTP:                       /* SET FTP-CLIENT */
#endif /* SYSFTP */
#endif /* NOFTP */
      case XYBROWSE:                    /* SET BROWSER */
        {
            char *p = getenv("PATH");
            char *app = (char *) browser, *opts = (char *) browsopts;
#ifndef NOFTP
#ifdef SYSFTP
            extern char ftpapp[], ftpopts[];
            if (xx == XYFTP) {
                app = (char *)ftpapp;
                opts = (char *)ftpopts;
            }
#endif /* SYSFTP */
#endif /* NOFTP */
#ifdef IKSD
            if (inserver) {
                printf("?Sorry, command disabled.\r\n");
                return(success = 0);
            }
#endif /* IKSD */
#ifdef CK_APC
            /* Don't let this be set remotely */
            if (apcactive == APC_LOCAL ||
                (apcactive == APC_REMOTE && !(apcstatus & APC_UNCH)))
              return(success = 0);
#endif /* CK_APC */
#ifdef OS2ORUNIX
            if (p)
              x = cmifip(xx == XYBROWSE ?
                         "Name of preferred browser" :
                         "Name of preferred ftp client",
#ifdef OS2
                         xx == XYFTP ? "ftp.exe" : ""
#else
                         xx == XYFTP ? "ftp" : ""
#endif /* OS2 */
                         ,&s,&y,0,p,xxstring
                         );
            else
              x = cmifi(xx == XYBROWSE ?
                        "Full path of preferred browser" :
                        "Full path of preferred ftp client",
                        "",&s,&y,xxstring
                        );
            if (x < 0 && x != -3)
              return(x);
#else
#ifdef VMS
            if ((x = cmtxt("DCL command to start your preferred Web browser",
                           "", &s, NULL)) < 0) {
                if (x != -3)
                  return(x);
            }
#else
            if ((x = cmifi("Full path of preferred browser","",&s,&y,xxstring)
                 ) < 0) {
                if (x != -3)
                  return(x);
            }
#endif /* VMS */
#endif /* OS2ORUNIX */
#ifdef VMS
            ckstrncpy(app,s,CKMAXPATH);
            *opts = NUL;
#else
            if (y != 0) {
                printf("?A single file please\n");
                return(-2);
            }
            ckstrncpy(line,s,LINBUFSIZ);
            if ((x = cmtxt(xx == XYBROWSE ?
                           "browser command-line options" :
                           "ftp client command-line options",
                           "",&s,NULL)
                 ) < 0)
              return(x);
            ckstrncpy(tmpbuf,s,TMPBUFSIZ);
            if ((z = cmcfm()) < 0) return(z);
            if (line[0]) {
                zfnqfp(line,CKMAXPATH,app);
                ckstrncpy(opts, tmpbuf, 128);
            } else {
                *app = NUL;
                *opts = NUL;
            }
#endif /* VMS */
            return(success = 1);
        }
#endif /* BROWSER */
#endif /* NOFRILLS */
#endif /* NOPUSH */

#ifdef CK_CTRLZ
      case XYEOF: {                     /* SET EOF */
          extern int eofmethod; extern struct keytab eoftab[];
          if ((x = cmkey(eoftab,3,"end-of-file detection method","",
                         xxstring)) < 0)
            return(x);
          if ((y = cmcfm()) < 0)
            return(y);
          eofmethod = x;
          return(success = 1);
      }
#endif /* CK_CTRLZ */

#ifdef SESLIMIT
      case XYLIMIT: {  /* Session-Limit (length of session in seconds) */
          extern int seslimit;
#ifdef OS2
          extern int downloaded;
#endif /* OS2 */
          y = cmnum("Maximum length of session, seconds","0",10,&x,xxstring);
          if (inserver &&
#ifdef IKSDCONF
              iksdcf
#else
              1
#endif /* IKSDCONF */
#ifdef OS2
               || downloaded
#endif /* OS2 */
              ) {
              if ((z = cmcfm()) < 0)
                return(z);
              printf("?Sorry, command disabled.\r\n");
              return(success = 0);
          }
          return(setnum(&seslimit,x,y,86400));
      }
#endif /* SESLIMIT */

      case XYRELY: {                    /* SET RELIABLE */
          if ((x = cmkey(ooatab,3,"","automatic",xxstring)) < 0)
            return(x);
          if ((y = cmcfm()) < 0) return(y);
          reliable = x;
          setreliable = (x != SET_AUTO);
          debug(F101,"SET RELIABLE reliable","",reliable);
          return(success = 1);
      }

#ifdef STREAMING
      case XYSTREAM: {                  /* SET STREAMING */
          extern int streamrq;
          if ((x = cmkey(ooatab,3,"","automatic",xxstring)) < 0)
            return(x);
          if ((y = cmcfm()) < 0) return(y);
          streamrq = x;
          return(success = 1);
      }
#endif /* STREAMING */

#ifdef CKSYSLOG
      case XYSYSL: {
          if ((x = cmkey(syslogtab,nsyslog,"","",xxstring)) < 0)
            return(x);
          if ((y = cmcfm()) < 0) return(y);
#ifdef IKSD
          if (inserver &&
#ifdef IKSDCONF
              iksdcf
#else
              1
#endif /* IKSDCONF */
              ) {
              printf("?Sorry, command disabled.\n");
              return(success = 0);
          }
#endif /* IKSD */
#ifdef CK_APC
          /* Don't let this be set remotely */
          if (apcactive == APC_LOCAL ||
              (apcactive == APC_REMOTE && !(apcstatus & APC_UNCH)))
            return(success = 0);
#endif /* CK_APC */
          ckxsyslog = x;
          return(success = 1);
      }
#endif /* CKSYSLOG */

#ifdef TLOG
      case XYTLOG: {                    /* SET TRANSACTION-LOG */
          extern int tlogsep;
          if ((x = cmkey(vbtab,nvb,"","verbose",xxstring)) < 0)
            return(x);
          if (x == 0) {
              if ((y = cmtxt("field separator",",",&s,NULL)) < 0) return(y);
              s = brstrip(s);
              if (*s) {
                  if (s[1]) {
                      printf("?A single character, please.\n");
                      return(-9);
                  } else if ((*s >= '0' && *s <= '9') ||
                             (*s >= 'A' && *s <= 'Z') ||
                             (*s >= 'a' && *s <= 'z')) {
                      printf("?A non-alphanumeric character, please.\n");
                      return(-9);
                  } else
                    tlogsep = *s;
              }
          } else {
              if ((y = cmcfm()) < 0) return(y);
          }
#ifdef IKSD
          if (inserver && isguest) {
              printf("?Sorry, command disabled.\n");
              return(success = 0);
          }
#endif /* IKSD */
#ifdef CK_APC
          /* Don't let this be set remotely */
          if (apcactive == APC_LOCAL ||
              (apcactive == APC_REMOTE && !(apcstatus & APC_UNCH)))
            return(success = 0);
#endif /* CK_APC */
          tlogfmt = x;
          return(success = 1);
      }
#endif /* TLOG */

      case XYCLEAR: {                   /* SET CLEARCHANNEL */
          if ((x = cmkey(ooatab,3,"","automatic",xxstring)) < 0)
            return(x);
          if ((y = cmcfm()) < 0) return(y);
          clearrq = x;
          return(success = 1);
      }

#ifdef CK_AUTHENTICATION
      case XYAUTH: {                    /* SET AUTHENTICATION */
#ifdef CK_KERBEROS
          int kv = 0;
          extern struct krb_op_data krb_op;
#endif /* CK_KERBEROS */
          char * p = NULL;
          if ((x =
               cmkey(setauth,nsetauth,"authentication type","",xxstring)) < 0)
            return(x);
          switch (x) {
#ifdef CK_KERBEROS
            case AUTH_KRB4: kv = 4; break; /* Don't assume values are same */
            case AUTH_KRB5: kv = 5; break;
#endif /* CK_KERBEROS */
#ifdef CK_SRP
            case AUTH_SRP: break;
#endif /* CK_SRP */
#ifdef CK_SSL
            case AUTH_SSL:
            case AUTH_TLS:
              break;
#endif /* CK_SSL */
            default:
              printf("?Authorization type not supported yet - \"%s\"\n",
                     atmbuf);
              return(-9);
          }
#ifdef IKSD
          if (inserver &&
#ifdef IKSDCONF
              iksdcf
#else
              1
#endif /* IKSDCONF */
              ) {
              if ((y = cmcfm()) < 0) return(y);
              printf("?Sorry, command disabled.\n");
              return(success = 0);
          }
#endif /* IKSD */
#ifdef CK_APC
          /* Don't let this be set remotely */
          if (apcactive == APC_LOCAL ||
              apcactive == APC_REMOTE && !(apcstatus & APC_UNCH)) {
              if ((y = cmcfm()) < 0) return(y);
              return(success = 0);
          }
#endif /* CK_APC */

          switch(x) {
#ifdef CK_KERBEROS
            case AUTH_KRB4:
            case AUTH_KRB5: {
                if ((x = cmkey(kv == 4 ? k4tab : k5tab,
                               kv == 4 ? nk4tab : nk5tab,
                               "Kerberos parameter","",xxstring)) < 0) {
                    return(x);
                }
                s = "";
                switch (x) {
#ifdef KRB4
                  case XYKRBDBG:
                    if (kv == 4) {
                        if ((y = seton(&k4debug)) < 0)
                          return(y);
#ifdef NT
                        ck_krb4_debug(k4debug);
#endif /* NT */
                    } else {
                        return(-9);
                    }
                    break;
#endif /* KRB4 */
                  case XYKRBLIF:
                    if ((y = cmnum("TGT lifetime","600",10,&z,xxstring)) < 0)
                      return(y);
                    break;
                  case XYKRBPRE:
                    if (kv == 4) {
                        if ((y = seton(&krb4_d_preauth)) < 0)
                          return(y);
                    } else {
                        return(-9);
                    }
                    break;
                  case XYKRBINS:
                    if ((y = cmtxt("Instance name","",&s,xxstring)) < 0)
                      return(y);
                    break;
                  case XYKRBFWD:
                    if (kv == 5) {
                        if ((y = seton(&krb5_d_forwardable)) < 0)
                          return(y);
                    } else {
                        return(-9);
                    }
                    break;
                  case XYKRBPRX:
                    if (kv == 5) {
                        if ((y = seton(&krb5_d_proxiable)) < 0)
                          return(y);
                    } else {
                        return(-9);
                    }
                    break;
                  case XYKRBRNW:
                    if ((y = cmnum("TGT renewable lifetime",
                                   "0",10,&z,xxstring)) < 0)
                      return(y);
                    break;
                  case XYKRBADR:
                    if (kv == 5) {
                        if ((y = seton(&krb5_checkaddrs)) < 0)
                          return(y);
                    } else {
                        if ((y = seton(&krb4_checkaddrs)) < 0)
                          return(y);
                    }
                    break;
                  case XYKRBNAD:
                    if (kv == 5) {
                        if ((y = seton(&krb5_d_no_addresses)) < 0)
                          return(y);
                    }
                    break;
                  case XYKRBADD:
                    if (kv == 5) {
                        char * tmpaddrs[KRB5_NUM_OF_ADDRS];
                        for (i = 0; i < KRB5_NUM_OF_ADDRS; i++)
                          tmpaddrs[i] = NULL;

                        if ((y =
                             cmfld("List of IP addresses","",&s,xxstring)) < 0)
                          return(y);
                        makelist(s,tmpaddrs,KRB5_NUM_OF_ADDRS);
                        if ((y = cmcfm()) < 0) {
                            for (i = 0; i < KRB5_NUM_OF_ADDRS; i++) {
                                if (tmpaddrs[i] != NULL)
                                  free(tmpaddrs[i]);
                            }
                            return(y);
                        }
                        for (i = 0;
                             i < KRB5_NUM_OF_ADDRS && tmpaddrs[i];
                             i++) {
                            if (inet_addr(tmpaddrs[i]) == 0xffffffff) {
                                printf("invalid ip address: %s\n",
                                       tmpaddrs[i]);
                                for (i = 0; i < KRB5_NUM_OF_ADDRS; i++) {
                                    if (tmpaddrs[i] != NULL)
                                      free(tmpaddrs[i]);
                                }
                                return(-9);
                            }
                        }
                        for (i = 0;
                             i < KRB5_NUM_OF_ADDRS && krb5_d_addrs[i];
                             i++) {
                            if (krb5_d_addrs[i])
                              free(krb5_d_addrs[i]);
                            krb5_d_addrs[i] = NULL;
                        }
                        for (i = 0;
                             i < KRB5_NUM_OF_ADDRS && tmpaddrs[i];
                             i++) {
                            krb5_d_addrs[i] = tmpaddrs[i];
                            tmpaddrs[i] = NULL;
                        }
                        krb5_d_addrs[i] = NULL;
                        return(success = 1);
                    }
                    break;

                  case XYKRBGET:
                    if (kv == 5) {
                        if ((y = seton(&krb5_autoget)) < 0)
                          return(y);
                    } else {
                        if ((y = seton(&krb4_autoget)) < 0)
                          return(y);
                    }
                    break;
                  case XYKRBDEL:
                    if ((z = cmkey(kdestab,nkdestab,
                                   "Auto Destroy Tickets",
                                   "never",xxstring)) < 0)
                      return(z);
                    break;
                  case XYKRBPR:
                    if ((y = cmtxt("User ID",uidbuf,&s,xxstring)) < 0)
                      return(y);
                    break;
                  case XYKRBRL:
                    if ((y = cmtxt("Name of realm","",&s,xxstring)) < 0)
                      return(y);
                    break;
                  case XYKRBKTB:
                    y = cmifi("Filename","",&s,&z,xxstring);
                    if (y != -3) {
                       if (y < 0)
                         return(y);
                       if (z) {
                         printf("?Wildcards not allowed\n");
                         return(-9);
                       }
                    }
                    break;
                  case XYKRBCC:
                    if ((y = cmofi("Filename","",&s,xxstring)) < 0)
                      return(y);
                    break;
                  case XYKRBSRV:
                    if ((y = cmtxt("Name of service to use in ticket",
                                   (kv == 4 ? "rcmd" : "host"),
                                   &s,
                                   xxstring
                                   )) < 0)
                      return(y);
                    break;
                  case XYKRBK5K4:
                    if (kv == 5) {
                        if ((y = seton(&krb5_d_getk4)) < 0)
                          return(y);
                    } else {
                        return(-9);
                    }
                    break;
                  case XYKRBPRM:        /* Prompt */
                    if ((z = cmkey(krbprmtab,2,"","",xxstring)) < 0)
                      return(z);
                    if ((y = cmtxt((z == KRB_PW_PRM) ?
  "Text of prompt;\nmay contain \"%s\" to be replaced by principal name" :
  "Text of prompt",
                                   "",
                                   &s,
                                   xxstring
                                   )
                         ) < 0)
                      return(y);
                    break;
                }
                ckstrncpy(line,s,LINBUFSIZ);
                s = line;
                if ((y = cmcfm()) < 0)
                  return(y);
#ifdef IKSD
                if (inserver &&
#ifdef IKSDCONF
                    iksdcf
#else /* IKSDCONF */
                    1
#endif /* IKSDCONF */
                    )
                  return(success = 0);
#endif /* IKSD */

                switch (x) {            /* Copy value to right place */
                  case XYKRBLIF:        /* Lifetime */
                    if (kv == 4)
                      krb4_d_lifetime = z;
                    else
                      krb5_d_lifetime = z;
                    break;
                  case XYKRBRNW:
                    if (kv == 5)
                      krb5_d_renewable = z;
                    break;
                  case XYKRBPR:         /* Principal */
                    s = brstrip(s);	/* Strip braces around. */
                    if (kv == 4)
                      makestr(&krb4_d_principal,s);
                    else
                      makestr(&krb5_d_principal,s);
                    break;
                  case XYKRBINS:        /* Instance */
                    if (kv == 4)
                      makestr(&krb4_d_instance,s);
                    else
                      makestr(&krb5_d_instance,s);
                    break;
                  case XYKRBRL:         /* Realm */
                    if (kv == 4)
                      makestr(&krb4_d_realm,s);
                    else
                      makestr(&krb5_d_realm,s);
                    break;
                  case XYKRBKTB:        /* Key Table */
                    if (kv == 4)
                      makestr(&k4_keytab,s);
                    else
                      makestr(&k5_keytab,s);
                    break;
                  case XYKRBCC:         /* Credentials cache */
                    makestr(&krb5_d_cc,s);
                    break;
                  case XYKRBSRV:        /* Service Name */
                    if (kv == 4)
                      makestr(&krb4_d_srv,s);
                    else
                      makestr(&krb5_d_srv,s);
                    break;
                  case XYKRBDEL:
                    if (kv == 5)
                      krb5_autodel = z;
                    else
                      krb4_autodel = z;
                    break;
                  case XYKRBPRM:        /* Prompt */
		    s = brstrip(s);
                    switch (z) {
                      case KRB_PW_PRM: { /* Password */
                          /* Check that there are no more than */
                          /* two % fields and % must followed by 's'. */
                          int i,n,len;
                          len = strlen(s);
                          for (i = 0, n = 0; i < len; i++) {
                              if (s[i] == '%') {
                                  if (s[i+1] != '%') {
                                      if (s[i+1] != 's') {
                                          printf(
                                           "Only %%s fields are permitted.\n"
                                                 );
                                          return(-9);
                                      }
                                      if (++n > 2) {
                                          printf(
                                      "Only two %%s fields are permitted.\n");
                                          return(-9);
                                      }
                                  }
                                  i++;
                              }
                          }
                          if (kv == 5)
                            makestr(&k5pwprompt,s);
                          else
                            makestr(&k4pwprompt,s);
                          break;
                      }
                      case KRB_PR_PRM: { /* Principal */
                          /* Check to make sure there are no % fields */
                          int i,len;
                          len = strlen(s);
                          for (i = 0; i < len; i++) {
                              if (s[i] == '%') {
                                  if (s[i+1] != '%') {
                                      printf(
                                  "%% fields are not used in this command.\n");
                                      return(-9);
                                  }
                                  i++;
                              }
                          }
                          if (kv == 5)
                            makestr(&k5prprompt,s);
                          else
                            makestr(&k4prprompt,s);
                          break;
                      }
                    }
                }
                break;
            }
#endif /* CK_KERBEROS */
#ifdef CK_SRP
            case AUTH_SRP: {
                if ((x = cmkey(srptab, nsrptab,
                               "SRP parameter","",xxstring)) < 0) {
                    return(x);
                }
                s = "";
                switch (x) {
                  case XYSRPPRM:        /* Prompt */
                    if ((z = cmkey(srpprmtab,1,"","",xxstring)) < 0)
                      return(z);
                    if ((y = cmtxt(
  "Text of prompt;\nmay contain one \"%s\" to be replaced by the username",
                                   "",
                                   &s,
                                   xxstring
                                   )
                         ) < 0)
                      return(y);
                    break;
                }
                ckstrncpy(line,s,LINBUFSIZ);
                s = line;
                if ((y = cmcfm()) < 0)
                  return(y);
                switch (x) {            /* Copy value to right place */
                  case XYSRPPRM:        /* Prompt */
		    s = brstrip(s);
                    switch (z) {
                      case SRP_PW_PRM: { /* Password */
                          /* Check %s fields */
                          int i,n,len;
                          len = strlen(s);
                          for (i = 0, n = 0; i < len; i++) {
                              if (s[i] == '%') {
                                  if (s[i+1] != '%') {
                                      if (s[i+1] != 's') {
                                          printf(
                                          "Only %%s fields are permitted.\n");
                                          return(-9);
                                      }
                                      if (++n > 1) {
                                          printf(
                                       "Only one %%s field is permitted.\n");
                                          return(-9);
                                      }
                                  }
                                  i++;
                              }
                          }
                          makestr(&srppwprompt,s);
                          break;
                      }
                    }
                }
                break;
            }
#endif /* CK_SRP */
#ifdef CK_SSL
            case AUTH_SSL:
            case AUTH_TLS: {
                if ((z = cmkey(ssltab, nssltab,
                           (x == AUTH_SSL ? "SSL parameter" : "TLS parameter"),
                           "",xxstring)) < 0)
                  return(z);
                s = "";
                switch (z) {
                  case XYSSLRCFL:       /* SSL/TLS RSA Certs file */
                  case XYSSLRCCF:       /* SSL/TLS RSA Certs Chain file */
                  case XYSSLRKFL:       /* SSL/TLS RSA Key File */
                  case XYSSLDCFL:       /* SSL/TLS DSA Certs file */
                  case XYSSLDCCF:       /* SSL/TLS DSA Certs Chain file */
                  case XYSSLDKFL:       /* SSL/TLS DH Key File */
                  case XYSSLDPFL:       /* SSL/TLS DH Param File */
                  case XYSSLCRL:        /* SSL/TLS CRL File */
                  case XYSSLVRFF:       /* SSL/TLS Verify File */
                  case XYSSLRND:        /* SSL/TLS Random File */
                    y = cmifi("Filename","",&s,&x,xxstring);
                    if (y != -3) {
                        if (y < 0)
                          return(y);
                        if (x) {
                            printf("?Wildcards not allowed\n");
                            return(-9);
                        }
                    }
                    ckstrncpy(line,s,LINBUFSIZ);
                    s = line;
                    s = brstrip(s);
                    if ((y = cmcfm()) < 0)
                      return(y);
                    switch (z) {
                      case XYSSLRCFL:   /* SSL/TLS RSA Certs file */
                        if (!s[0] && ssl_rsa_cert_file) {
                            free(ssl_rsa_cert_file);
                            ssl_rsa_cert_file = NULL;
                        } else if (s[0]) {
                            makestr(&ssl_rsa_cert_file,s);
                            if (!ssl_rsa_key_file)
                              makestr(&ssl_rsa_key_file,s);
                        }
                        break;
                      case XYSSLRCCF:   /* SSL/TLS RSA Certs Chain file */
                          if (!s[0] && ssl_rsa_cert_chain_file) {
                              free(ssl_rsa_cert_chain_file);
                              ssl_rsa_cert_chain_file = NULL;
                          } else if (s[0]) {
                              makestr(&ssl_rsa_cert_chain_file,s);
                          }
                          break;
                      case XYSSLRKFL:   /* SSL/TLS RSA Key File */
                        if (!s[0] && ssl_rsa_key_file) {
                            free(ssl_rsa_key_file);
                            ssl_rsa_key_file = NULL;
                        } else if (s[0]) {
                            makestr(&ssl_rsa_key_file,s);
                        }
                        break;
                      case XYSSLDCFL:   /* SSL/TLS DSA Certs file */
                        if (!s[0] && ssl_dsa_cert_file) {
                            free(ssl_dsa_cert_file);
                            ssl_dsa_cert_file = NULL;
                        } else if (s[0]) {
                            makestr(&ssl_dsa_cert_file,s);
                            if (!ssl_dh_key_file)
                              makestr(&ssl_dh_key_file,s);
                        }
                        break;
                      case XYSSLDCCF:   /* SSL/TLS DSA Certs Chain file */
                          if (!s[0] && ssl_dsa_cert_chain_file) {
                              free(ssl_dsa_cert_chain_file);
                              ssl_dsa_cert_chain_file = NULL;
                          } else if (s[0]) {
                              makestr(&ssl_dsa_cert_chain_file,s);
                          }
                          break;
                      case XYSSLDKFL:   /* SSL/TLS DH Key File */
                        if (!s[0] && ssl_dh_key_file) {
                            free(ssl_dh_key_file);
                            ssl_dh_key_file = NULL;
                        } else if (s[0]) {
                            makestr(&ssl_dh_key_file,s);
                        }
                        break;
                      case XYSSLDPFL:   /* SSL/TLS DH Param File */
                        if (!s[0] && ssl_dh_param_file) {
                            free(ssl_dh_param_file);
                            ssl_dh_param_file = NULL;
                        } else if (s[0]) {
                            makestr(&ssl_dh_param_file,s);
                        }
                        break;
                      case XYSSLCRL:    /* SSL/TLS CRL File */
                        if (!s[0] && ssl_crl_file) {
                            free(ssl_crl_file);
                            ssl_crl_file = NULL;
                        } else if (s[0]) {
                            makestr(&ssl_crl_file,s);
                        }
                        break;
                      case XYSSLVRFF:   /* SSL/TLS Verify File */
                        if (!s[0] && ssl_verify_file) {
                            free(ssl_verify_file);
                            ssl_verify_file = NULL;
                        } else if (s[0]) {
                            makestr(&ssl_verify_file,s);
                        }
                        break;
                      case XYSSLRND:    /* SSL/TLS Random File */
                        if (!s[0] && ssl_rnd_file) {
                            free(ssl_rnd_file);
                            ssl_rnd_file = NULL;
                        } else if (s[0]) {
                            makestr(&ssl_rnd_file,s);
                        }
                        break;
                    }
                    break;

                  case XYSSLCRLD:
                  case XYSSLVRFD: {
                    char * d = NULL;
                    if (z == XYSSLVRFD)
                      d= getenv("SSL_CERT_DIR");
                    if (d == NULL)
                        d = "";
                    if ((y = cmdir("Directory",d,&s,xxstring)) < 0)
		      if (y != -3)
			return(y);
                    ckstrncpy(line,s,LINBUFSIZ);
                    s = line;
                    s = brstrip(s);
                    if ((y = cmcfm()) < 0)
                      return(y);
                    switch(z) {
                      case XYSSLCRLD:
                        if (!s[0] && ssl_crl_dir) {
                            free(ssl_crl_dir);
                            ssl_crl_dir = NULL;
                        } else if (s[0]) {
                            makestr(&ssl_crl_dir,s);
                        }
                        break;
                      case XYSSLVRFD:
                        if (!s[0] && ssl_verify_dir) {
                            free(ssl_verify_dir);
                            ssl_verify_dir = NULL;
                        } else if (s[0]) {
                            makestr(&ssl_verify_dir,s);
                        }
                        break;
                    }
                    break;
                  }
                  case XYSSLCOK:        /* SSL/TLS Certs-Ok flag */
                    if ((y = seton(&ssl_certsok_flag)) < 0)
                      return(y);
                    break;
                  case XYSSLDBG:                /* SSL/TLS Debug flag */
                    if ((y = seton(&ssl_debug_flag)) < 0)
                      return(y);
                    break;
                  case XYSSLON:         /* SSL/TLS Only flag */
                    switch (x) {
                      case AUTH_SSL:
                        if ((y = seton(&ssl_only_flag)) < 0)
                          return(y);
                        break;
                      case AUTH_TLS:
                        if ((y = seton(&tls_only_flag)) < 0)
                          return(y);
                        break;
                    }
                    break;
                  case XYSSLVRB:        /* SSL/TLS Verbose flag */
                    if ((y = seton(&ssl_verbose_flag)) < 0)
                      return(y);
                    break;
                  case XYSSLVRF:        /* SSL/TLS Verify flag */
                    if ((x = cmkey(sslvertab, nsslvertab,
                                   "SSL/TLS verify mode",
                                   "peer-cert",xxstring)) < 0)
                      return(x);
                    if ((y = cmcfm()) < 0)
                      return(y);
                    ssl_verify_flag = x;
                    break;
                  case XYSSLDUM:
                    if ((y = seton(&ssl_dummy_flag)) < 0)
                      return(y);
                    break;
                  case XYSSLCL: {               /* SSL/TLS Cipher List */
#ifdef COMMENT
                      /* This code is used to generate a colon delimited */
                      /* list of the ciphers currently in use to be used */
                      /* as the default for cmtxt().  However, a better  */
                      /* default is simply the magic keyword "ALL".      */
                      CHAR def[1024] = "";
                      if (ssl_con != NULL) {
                          CHAR * p = NULL, *q = def;
                          int i, len;

                          for (i = 0; ; i++) {
                              p = (CHAR *) SSL_get_cipher_list(ssl_con,i);
                              if (p == NULL)
                                break;
                              len = strlen(p);
                              if (q+len+1 >= def+1024)
                                break;
                              if (i != 0)
                                *q++ = ':';
                              strcpy(q,p);
                              q += len;
                          }
                      }
#endif /* COMMENT */
                      char * p = getenv("SSL_CIPHER");
                      if (!p)
                        p = "ALL";
                      if ((y = cmtxt(
                    "Colon-delimited list of ciphers or ALL (case-sensitive)",
                                     p,
                                     &s,
                                     xxstring
                                     )
                           ) < 0)
                        return(y);
                      makestr(&ssl_cipher_list,s);
                      if (ssl_con == NULL) {
                          SSL_library_init();
                          ssl_ctx = (SSL_CTX *)
                            SSL_CTX_new((SSL_METHOD *)TLSv1_method());
                          if (ssl_ctx != NULL)
                            ssl_con= (SSL *) SSL_new(ssl_ctx);
                      }
                      if (ssl_con) {
                          SSL_set_cipher_list(ssl_con,ssl_cipher_list);
                      }
                      break;
                  }
                }
                break;
            }
#endif /* CK_SSL */
            default:
              break;
          }
          return(success = 1);
      }
#endif /* CK_AUTHENTICATION */

#ifndef NOSPL
      case XYFUNC:
        if ((x = cmkey(functab,nfunctab,"","diagnostics",xxstring)) < 0)
          return(x);
        switch (x) {
          case FUNC_DI: return(seton(&fndiags));
          case FUNC_ER: return(seton(&fnerror));
          default:      return(-2);
        }
#endif /* NOSPL */

      case XYSLEEP:                     /* SET SLEEP / PAUSE */
        if ((x = cmkey(sleeptab,1,"","cancellation",xxstring)) < 0)
          return(x);
        return(seton(&sleepcan));

      case XYCD:                        /* SET CD */
        if ((x = cmkey(cdtab,ncdtab,"","",xxstring)) < 0)
          return(x);
        switch (x) {
	  case XYCD_H: {		/* SET CD HOME */
	      extern char * myhome;
	      if ((y = cmdir("Directory name",zhome(),&s,xxstring)) < 0)
		return(y);
	      makestr(&myhome,s);
	      return(success = 1);
	  }
          case XYCD_M:                  /* SET CD MESSAGE */
            if ((x = cmkey(cdmsg,ncdmsg,"","",xxstring)) < 0)
              return(x);
            if (x == 2) {               /* CD MESSAGE FILE */
                if ((x = cmtxt("Name of file","",&s,NULL)) < 0)
                  return(x);
                if (!*s) {
                    s = NULL;
#ifndef NOXFER
                    srvcdmsg = 0;
#endif /* NOXFER */
                }
                makestr(&cdmsgstr,s);
                makelist(cdmsgstr,cdmsgfile,8);
                return(success = 1);
            }

            if ((y = cmcfm()) < 0) return(y); /* CD-MESSAGE ON/OFF */
#ifndef NOXFER
            if (x > 0)
              srvcdmsg |= 2;
            else
              srvcdmsg &= 1;
#endif /* NOXFER */
            return(success = 1);

          case XYCD_P: {                /* SET CD PATH */
              extern char * ckcdpath;
              if ((x = cmtxt("CD PATH string","",&s,xxstring)) < 0)
                return(x);
              makestr(&ckcdpath,s);
              return(success = 1);
          }
        }

#ifndef NOLOCAL
#ifdef STOPBITS
      case XYSTOP:                      /* STOP-BITS */
        if ((x = cmkey(stoptbl,2,"Stop bits for serial device","",
                       xxstring)) < 0)
          return(x);
        if ((y = cmcfm()) < 0)
          return(y);
        if (x > 0 && x < 3) {
            stopbits = x;
#ifdef TN_COMPORT
            if (network && istncomport()) {
                tnsettings(-1, x);
                return(success = 1);
            }
#endif /* TN_COMPORT */
#ifdef HWPARITY
            return(success = 1);
#else /* HWPARITY */
            return(-2);
#endif /* HWPARITY */
        } else
          return(-2);
#endif /* STOPBITS */

      case XYDISC: {
          extern int clsondisc;
          return(seton(&clsondisc));
      }

      case XYSERIAL: {
          /* char c; */
          extern int cmask;
          if ((x = cmkey(sertbl,nsertbl,
                         "Serial device character size, parity, and stop bits",
                         "8N1", xxstring)) < 0)
            return(x);
          ckstrncpy(line,atmbuf,LINBUFSIZ); /* Associated keyword string */
          s = line;
          if ((y = cmcfm()) < 0)
            return(y);
          ckstrncpy(line,sernam[x],LINBUFSIZ);
          s = line;
          if (s[0] != '8' && s[0] != '7') /* Char size */
            return(-2);
          else
            z = s[0] - '0';
          if (isupper(s[1]))            /* Parity */
            s[1] = tolower(s[1]);
          if (s[2] != '1' && s[2] != '2') /* Stop bits */
            return(-2);
          else
            stopbits = s[2] - '0';
          if (z == 8) {                 /* 8 bits + parity (or not) */
              parity = 0;               /* Set parity */
              hwparity = (s[1] == 'n') ? 0 : s[1];
              setcmask(8);              /* Also set TERM BYTESIZE to 8 */
          } else {                      /* 7 bits plus parity */
              parity = (s[1] == 'n') ? 0 : s[1];
              hwparity = 0;
              setcmask(7);              /* Also set TERM BYTESIZE to 7 */
          }
#ifdef TN_COMPORT
          if (network && !istncomport())
            tnsettings(parity, stopbits);
#endif /* TN_COMPORT */

          return(success = 1);          /* from SET SERIAL */
      }

      case XYOPTS: {                    /* SET OPTIONS */
          extern int setdiropts();
          extern int settypopts();
#ifdef CKPURGE
          extern int setpurgopts();
#endif /* CKPURGE */
          if ((x = cmkey(optstab,noptstab,"for command","", xxstring)) < 0)
            return(x);
          switch (x) {
#ifndef NOFRILLS
            case XXDEL:
              return(setdelopts());
#endif /* NOFRILLS */
            case XXDIR:
              return(setdiropts());
            case XXTYP:
              return(settypopts());
#ifdef CKPURGE
            case XXPURGE:
              return(setpurgopts());
#endif /* CKPURGE */
            default:
              return(-2);
          }
      }
#endif /* NOLOCAL */
#ifndef NOXFER
      case XYQ8FLG: {
          extern int q8flag;
          return(seton(&q8flag));
      }
      case XYTIMER: {
          extern int asktimer;
          y = cmnum("Time limit for ASK command, seconds","0",10,&x,xxstring);
#ifdef QNX16
          return(setnum(&asktimer,x,y,32767));
#else
          return(setnum(&asktimer,x,y,86400));
#endif /* QNX16 */
      }
      case XYFACKB: {
          extern int fackbug;
          return(seton(&fackbug));
      }
#endif /* NOXFER */

      case XYHINTS:
        return(seton(&hints));

#ifndef NOSPL
      case XYEVAL: {
          extern int oldeval;
          if ((x = cmkey(oldnew,2,"","", xxstring)) < 0)
            return(x);
          if ((y = cmcfm()) < 0)
            return(y);
          oldeval = x;
          return(success = 1);
      }
#endif /* NOSPL */

#ifndef NOXFER
      case XYFACKP: {
          extern int fackpath;
          return(seton(&fackpath));
      }
#endif /* NOXFER */

      case XYQNXPL: {
          extern int qnxportlock;
          return(seton(&qnxportlock));
      }

#ifndef NOCMDL
#ifdef IKSD
      case XYIKS: {
          int setiks();
          return(setiks());
      }
#endif /* IKSD */
#endif /* NOCMDL */

#ifdef CKROOT
      case XYROOT:
        return(dochroot());
#endif /* CKROOT */

#ifndef NOSPL
#ifndef NOSEXP
      case XYSEXP: {
          if ((x = cmkey(sexptab,3,"","", xxstring)) < 0)
            return(x);
          switch (x) {
            case 0:
              if ((x = cmkey(ooatab,3,"","automatic", xxstring)) < 0)
                return(x);
              if ((y = cmcfm()) < 0)
                return(y);
              sexpecho = x;
              break;
            case 1: {
                int i, xx;
                xx = sexpmaxdep;
                if ((y = cmnum("Maximum recursion depth",
                               "1000",10,&x,xxstring)) < 0)
                  return(y);
                z = setnum(&sexpmaxdep,x,y,-1);
                if (z < 0)
                  return(z);
                if (sxresult) {         /* Free old stack if allocated */
                    for (i = 0; i < xx; i++)
                      if (sxresult[i]) free(sxresult[i]);
                    free((char *)sxresult);
                    if (sxrlen) free((char *)sxrlen);
                    sxresult = NULL;
                    sxrlen = NULL;
                }
                break;
            }
	    case 2:
	      return(seton(&sexptrunc));
          }
          return(success = 1);
      }
#endif /* NOSEXPL */
#endif /* NOSPL */

#ifdef NEWFTP
      case XYGPR: {
          extern struct keytab gprtab[];
          extern int ftpget;
          if ((x = cmkey(gprtab,3,"","kermit", xxstring)) < 0)
            return(x);
          if ((y = cmcfm()) < 0)
            return(y);
          ftpget = x;
          return(success = 1);
      }
#endif /* NEWFTP */

#ifdef ANYSSH
      case XYSSH:
        return(dosetssh());
#endif /* ANYSHH */

#ifdef SFTP_BUILTIN
      case XYSFTP:
        return(dosetsftp());
#endif /* SFTP_BUILTIN */

#ifdef LOCUS
      case XYLOCUS:
          if ((x = cmkey(locustab,nlocustab,"",
#ifdef KUI
			 "ask"
#else
			 "auto"
#endif /* KUI */
			 ,xxstring)) < 0)
            return(x);
          if ((y = cmcfm()) < 0)
            return(y);
          if (x == 2 || x == 3) {	/* AUTO or ASK */
              setautolocus(x - 1);	/* Two forms of automatic locusing */
	      /* setlocus(1,0); */      /* we're not changing the locus here */
          } else {			/* LOCAL or REMOTE */
              setautolocus(0);		/* No automatic Locus changing */
              setlocus(x,0);		/* Set Locus as requested */
          }
          return(success = 1);
#endif /* LOCUS */

#ifdef KUI
      case XYGUI:
        return(setgui());
#endif /* KUI */

#ifndef NOFRILLS
#ifndef NORENAME
      case XY_REN:			/* SET RENAME */
	return(setrename());
#endif	/* NORENAME */
#endif	/* NOFRILLS */

#ifndef NOPUSH
#ifdef CK_REDIR
#ifndef NOXFER
      case XYEXTRN:			/* SET EXTERNAL-PROTOCOL */
	return(setextern());
#endif	/* NOXFER */
#endif	/* CK_REDIR */
#endif	/* NOPUSH */

#ifndef NOSPL
      case XYVAREV:			/* SET VARIABLE-EVALUATION */
	return(setvareval());
#endif	/* NOSPL */

      default:
         if ((x = cmcfm()) < 0) return(x);
         printf("Not implemented - %s\n",cmdbuf);
         return(success = 0);
    }
}

/*
  H U P O K  --  Is Hangup OK?

  Issues a warning and gets OK from user depending on whether a connection
  seems to be open and what the SET EXIT WARNING setting is.  Returns:
    0 if not OK to hang up or exit (i.e. user said No);
    nonzero if OK.
  Argument x is used to differentiate the EXIT command from SET LINE / HOST.
*/
int
hupok(x) int x; {                       /* Returns 1 if OK, 0 if not OK */
    int y, z = 1;
    extern int exithangup;
#ifdef VMS
    extern int batch;

    if (batch)                          /* No warnings in batch */
      return(1);
#else
#ifdef UNIX
    if (backgrd)                        /* No warnings in background */
      return(1);
#endif /* UNIX */
#endif /* VMS */

#ifndef K95G
    debug(F101,"hupok local","",local);

    if (!local)                         /* No warnings in remote mode */
      return(1);
#endif /* K95G */

    if (x == 0 && exithangup == 0)      /* EXIT and EXIT HANGUP is OFF */
      return(1);

    debug(F101,"hupok x","",x);
    debug(F101,"hupok xitwarn","",xitwarn);
    debug(F101,"hupok network","",network);
    debug(F101,"hupok haveline","",haveline);

    if ((local && xitwarn) ||           /* Is a connection open? */
        (!x && xitwarn == 2)) {         /* Or Always give warning on EXIT */
        int needwarn = 0;
        char warning[256];

        if (network) {
            if (ttchk() >= 0)
              needwarn = 1;
            /* A connection seems to be open but it can't possibly be */
            if (!haveline)
              needwarn = 0;
            if (needwarn) {
                if (strcmp(ttname,"*"))
                    ckmakmsg(warning,256,
                              " A network connection to ",ttname,
                              " might still be active.\n",NULL);
                else
                  ckstrncpy(warning,
                   " An incoming network connection might still be active.\n",
                             256);
            }
        } else {                        /* Serial connection */
            if (carrier == CAR_OFF)     /* SET CARRIER OFF */
              needwarn = 0;             /* so we don't care about carrier. */
            else if ((y = ttgmdm()) >= 0) /* else, get modem signals */
              needwarn = (y & BM_DCD);  /* Check for carrier */
            else                        /* If we can't get modem signals... */
              needwarn = (ttchk() >= 0);
            /* A connection seems to be open but it can't possibly be */
            if (!haveline || !exithangup)
              needwarn = 0;
            if (needwarn)
                ckmakmsg(warning,256,
                     " A serial connection might still be active on ",
                     ttname,".\n",NULL);
        }

/* If a warning was issued, get user's permission to EXIT. */

        if (needwarn || (!x && xitwarn == 2
#ifndef K95G
			&& local
#endif /* K95G */
			 )) {
            if ( !needwarn )
                ckstrncpy(warning, "No active connections", 256);

#ifdef COMMENT
	    printf("%s",warning);
            z = getyesno(x ? "OK to close? " : "OK to exit? ",0);
            debug(F101,"hupok getyesno","",z);
            if (z < -3) z = 0;
#else
	    z = uq_ok(warning,
		      x ? "OK to close? " : "OK to exit? ",
		      3,
		      NULL,
		      0
		      );
            debug(F101,"hupok uq_ok","",z);
	    if (z < 0) z = 0;
#endif /* COMMENT */
        }
    }
    return(z);
}

#ifndef NOSHOW
VOID
shoctl() {                              /* SHOW CONTROL-PREFIXING */
#ifdef CK_SPEED
    int i;
#ifdef OS2
    int zero;
#endif /* OS2 */
    printf(
"\ncontrol quote = %d, applied to (0 = unprefixed, 1 = prefixed):\n\n",
           myctlq);
#ifdef OS2
#ifndef UNPREFIXZERO
    zero = ctlp[0];
    if (protocol == PROTO_K)            /* Zero can't be unprefixed */
      ctlp[0] = 1;                      /* for Kermit */
#endif /* UNPREFIXZERO */
#endif /* OS2 */
    for (i = 0; i < 16; i++) {
        printf("  %3d: %d   %3d: %d ",i,ctlp[i], i+16, ctlp[i+16]);
        if (i == 15)
          printf("  127: %d",ctlp[127]);
        else
          printf("        ");
        printf("  %3d: %d   %3d: %d ",i+128,ctlp[i+128], i+144, ctlp[i+144]);
        if (i == 15)  printf("  255: %d",ctlp[255]);
        printf("\n");
    }
    printf("\n");
#ifndef UNPREFIXZERO
#ifdef OS2
    ctlp[0] = zero;
#endif /* OS2 */
#endif /* UNPREFIXZERO */

#endif /* CK_SPEED */
}

#ifndef NOXFER
VOID
shodbl() {                              /* SHOW DOUBLE/IGNORE */
#ifdef CKXXCHAR
    int i, n = 0;
    printf("\nSET SEND DOUBLE characters:\n");
    for (i = 0; i < 255; i++) {
        if (dblt[i] & 2) {
            n++;
            printf(" %d", i);
        }
    }
    if (n == 0)
      printf(" (none)");
    n = 0;
    printf("\nSET RECEIVE IGNORE characters:\n");
    for (i = 0; i < 255; i++) {
        if (dblt[i] & 1) {
            n++;
            printf(" %d", i);
        }
    }
    if (n == 0)
      printf(" (none)");
    printf("\n\n");
#endif /* CKXXCHAR */
}
#endif /* NOXFER */
#endif /* NOSHOW */

#ifndef NOPUSH
#ifdef CK_REXX
/*
  Rexx command.  Note, this is not OS/2-specific, because Rexx also runs
  on other systems where C-Kermit also runs, like the Amiga.
*/
#define REXBUFL 100                     /* Change this if neccessary */
char rexxbuf[REXBUFL] = { '\0' };       /* Rexx's return value (string) */

int
dorexx() {
    int x, y;
    char *rexxcmd;

        if ((x = cmtxt("Rexx command","",&rexxcmd,xxstring)) < 0)
          return(x);

#ifdef IKSD
    if (inserver) {
        printf("?Sorry, command disabled.\r\n");
        return(success = 0);
    }
#endif /* IKSD */
#ifdef CK_APC
    /* Don't let this be set remotely */
    if (apcactive == APC_LOCAL ||
        apcactive == APC_REMOTE && !(apcstatus & APC_UNCH))
      return(success = 0);
#endif /* CK_APC */

        ckstrncpy(line,rexxcmd,LINBUFSIZ);
        rexxcmd = line;
#ifdef OS2
        return(os2rexx(rexxcmd,rexxbuf,REXBUFL));
#else /* !OS2 */
        printf("Sorry, nothing happens.\n");
        return(success = 0);
#endif /* OS2 */
}
#endif /* CK_REXX */
#endif /* NOPUSH */
#else  /* NOICP */
VOID
dologend() {
    /* Dummy write record to connection log */
}
#endif /* NOICP */

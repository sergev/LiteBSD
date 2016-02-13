#include "ckcsym.h"

/*  C K U U S X --  "User Interface" common functions. */

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

/*
  This module contains user interface functions needed by both the interactive
  user interface and the command-line-only user interface, as well as the
  screen-control routines (curses and equivalent).
*/

/* Includes */

#include "ckcdeb.h"
#include "ckcasc.h"
#include "ckcker.h"
#include "ckuusr.h"
#include "ckcxla.h"

#ifndef NOHTERMCAP
#ifdef NOTERMCAP
#define NOHTERMCAP
#else
#ifndef BSD44
#define NOHTERMCAP
#else
#ifdef __bsdi__
#define NOHTERMCAP
#else
#ifdef OPENBSD
#define NOHTERMCAP
#else
#ifdef MACOSX
#define NOHTERMCAP
#endif /* MACOSX */
#endif /* OPENBSD */
#endif /* __bsdi__ */
#endif /* BSD44 */
#endif /* NOTERMCAP */
#endif /* NOHTERMCAP */

#ifndef NOTERMCAP
#ifdef BSD44
#ifndef NOHTERMCAP
#include <termcap.h>
#endif /* NOHTERMCAP */
#endif /* BSD44 */
#else  /* !BSD44 */
#ifdef linux
#include <term.h>
#endif /* linux */
#endif /* NOTERMCAP */

#ifdef OS2
#include <string.h>
_PROTOTYP(char * os2_gethostname, (void));
#define getpid _getpid
#endif /* OS2 */
#ifdef BSD44
#include <errno.h>
#endif /* BSD44 */

extern xx_strp xxstring;

#ifdef OS2
#include "ckcnet.h"
#else /* OS2 */
_PROTOTYP( char * ckgetpeer, (VOID));
_PROTOTYP(int getlocalipaddr, (void));
_PROTOTYP(int istncomport, (void));
#ifndef NOCKGETFQHOST
_PROTOTYP( char * ckgetfqhostname,(char *));
#endif	/* NOCKGETFQHOST */
#ifndef NETCONN
/*
  We should just pull in ckcnet.h here, but it causes a conflict with curses.h.
*/
#ifdef TCPSOCKET
#define NETCONN
#else
#ifdef SUNX25
#define NETCONN
#else
#ifdef STRATUSX25
#define NETCONN
#else
#ifdef IBMX25
#define NETCONN
#else
#ifdef HPX25
#define NETCONN
#else
#ifdef DECNET
#define NETCONN
#else
#ifdef NPIPE
#define NETCONN
#else
#ifdef CK_NETBIOS
#define NETCONN
#ifdef SUPERLAT
#define NETCONN
#else
#endif /* SUPERLAT */
#endif /* TCPSOCKET */
#endif /* SUNX25 */
#endif /* STRATUSX25 */
#endif /* IBMX25 */
#endif /* HPX25 */
#endif /* DECNET */
#endif /* NPIPE */
#endif /* CK_NETBIOS */
#endif /* NETCONN */
#endif /* OS2 */

#ifndef TCPSOCKET
#ifdef MULTINET
#define TCPSOCKET
#endif /* MULTINET */
#ifdef DEC_TCPIP
#define TCPSOCKET
#endif /* DEC_TCPIP */
#ifdef WINTCP
#define TCPSOCKET
#endif /* WINTCP */
#ifdef TCPWARE
#define TCPSOCKET
#endif /* TCPWARE */
#endif /* TCPSOCKET */

#ifdef OS2
#ifdef NT
#include <windows.h>
#include <tapi.h>
#include "ckntap.h"
#else /* NT */
#define INCL_VIO
#include <os2.h>
#endif /* NT */
#ifdef COMMENT                          /* Would you believe */
#undef COMMENT                          /* <os2.h> defines this ? */
#endif /* COMMENT */
#ifdef CK_NETBIOS
#include "ckonbi.h"
#endif /* CK_NETBIOS */

#include "ckocon.h"
extern ascreen commandscreen;
#ifdef KUI
#include "ikui.h"
#endif /* KUI */
#endif /* OS2 */

#ifdef NT
#include "cknwin.h"
#endif /* NT */
#ifdef OS2
#include "ckowin.h"
#include "ckosyn.h"
#endif /* OS2 */

#ifdef CK_TAPI
extern int tttapi;
extern int tapipass;
#endif /* CK_TAPI */

#ifdef CK_KERBEROS
#include "ckuath.h"
#endif /* CK_KERBEROS */

#ifndef WINTCP
#include <signal.h>
#endif /* WINTCP */

#ifdef VMS
#include <descrip.h>
#include <ssdef.h>
#include <stsdef.h>
#ifndef OLD_VMS
#include <lib$routines.h>  /* Not for VAX C 2.3 */
#else
#include <libdef.h>
#endif /* OLD_VMS */
#ifdef WINTCP
#include <signal.h>
#endif /* WINTCP */
#endif /* VMS */

#ifdef DCLFDOPEN
/* fdopen() needs declaring because it's not declared in <stdio.h> */
_PROTOTYP( FILE * fdopen, (int, char *) );
#endif /* DCLFDOPEN */

#ifdef DCLPOPEN
/* popen() needs declaring because it's not declared in <stdio.h> */
_PROTOTYP( FILE * popen, (char *, char *) );
#endif /* DCLPOPEN */

int tt_crd = 0;                         /* Carriage return display */
int tt_lfd = 0;                         /* Linefeed display */
int interrupted = 0;                    /* Interrupted from keyboard flag */
int fxd_inited = 0;			/* Fullscreen stuff initialized */

#ifdef DEBUG
char debfil[CKMAXPATH+1];               /* Debugging log file name */
#endif /* DEBUG */

#ifdef TLOG
char trafil[CKMAXPATH+1];               /* Transaction log file name */
#endif /* TLOG */

char sesfil[CKMAXPATH+1];               /* Session log file name */

#ifdef CKLOGDIAL
char diafil[CKMAXPATH+1];               /* Connection log file name */
char cxlogbuf[CXLOGBUFL+1];             /* Connection log record buffer */
int cx_active = 0;                      /* Connection is active */
extern int dialog;
#endif /* CKLOGDIAL */

#ifdef DYNAMIC
static char *cmdstr = NULL;             /* Place to build generic command */
#else
#ifdef pdp11
static char cmdstr[256];
#else
static char cmdstr[4096];
#endif /* pdp11 */
#endif /* DYNAMIC */

#ifndef NOMSEND
char fspec[CMDBL+4];                    /* Filename string for \v(filespec) */
int fspeclen = CMDBL;
#else
char fspec[CKMAXPATH+4];
int fspeclen = CKMAXPATH;
#endif /* NOMSEND */

char * rfspec = NULL;			/* Received filespec: local */
char * prfspec = NULL;			/* Preliminary rfspec */
char * sfspec = NULL;			/* Sent filespec: local */
char * psfspec = NULL;			/* Preliminary sfspec */
char * srfspec = NULL;			/* Received filespec: remote */
char * psrfspec = NULL;			/* Preliminary srfspec */
char * rrfspec = NULL;			/* Sent filespec: remote */
char * prrfspec = NULL;			/* Preliminary rrfspec */

int success = 1,                        /* Command success/failure flag */
    cmdlvl = 0,                         /* Command level */
    action = 0,				/* Action selected on command line */
    slogts = 0,				/* Session-log timestamps on/off */
    slognul = 0,			/* Session-log null-terminated lines */
#ifdef UNIX
    sessft = XYFT_T,                    /* Session log file type */
#else
    sessft = XYFT_B,			/* (text for UNIX binary for others) */
#endif /* UNIX */
    pflag = 1,                          /* Print prompt */
    msgflg = 1;                         /* Print informational messages */

extern int xaskmore, saveask;		/* More-prompting */

#ifdef CK_APC
extern int apcactive;
#endif /* CK_APC */
/* External variables */

extern int local, quiet, binary, network, what, parity, xitsta, escape,
  tlevel, bgset, backgrd, xsuspend, cmdint, nettype, seslog, dfloc;

extern int cmd_rows, cmd_cols, xcmdsrc;

extern char cmdfil[];

#ifdef VMS
extern int batch;
#endif /* VMS */

#ifdef datageneral                      /* 2/12/92 ENH */
#include <sysid.h>
extern int con_reads_mt, conint_ch, conint_avl;
#endif /* datageneral */

extern long speed;

extern char ttname[], *dftty, *cmarg, **cmlist, *versio, myhost[];

#ifndef NOCSETS
extern int fcharset, tcharset, xfrxla;
extern struct csinfo fcsinfo[], tcsinfo[];
#endif /* NOCSETS */

#ifdef OS2
extern unsigned char colorcmd;
#endif /* OS2 */

#ifdef NOXFER

int fdispla = XYFD_N;

#else  /* NOXFER is not defined */

#ifdef OS2                              /* File transfer display type */
int fdispla = XYFD_C;                   /* Curses (fullscreen) if we have it */
#else
#ifdef CK_CURSES
int fdispla = XYFD_C;
#else
int fdispla = XYFD_S;                   /* Otherwise CRT */
#endif /* CK_CURSES */
#endif /* OS2 */

extern struct ck_p ptab[];
extern int protocol, xfrbel, xfrint;

#ifdef STREAMING
extern int streaming, streamok;
#endif /* STREAMING */

/* Used internally */

#ifdef KUI
_PROTOTYP( VOID screeng, (int, char, long, char *) );
#endif	/* KUI */
_PROTOTYP( VOID screenc, (int, char, CK_OFF_T, char *) );


#ifdef CK_CURSES
#ifndef DYNAMIC
static char xtrmbuf[TRMBUFL];           /* tgetent() buffer */
char * trmbuf = xtrmbuf;
#else
char * trmbuf = NULL;
#endif /* DYNAMIC */
_PROTOTYP( static VOID dpyinit, (void) );
_PROTOTYP( static long shocps, (int, CK_OFF_T, CK_OFF_T) );
_PROTOTYP( static CK_OFF_T shoetl, (CK_OFF_T, long, CK_OFF_T, CK_OFF_T) );
#endif /* CK_CURSES */

static int ft_win = 0;  /* Fullscreen file transfer display window is active */

/* Variables declared here */

static char * skreason[] = {
    "",					/* 0 */
    "Remote file not older",		/* SKP_DAT */
    "Identical modification times",	/* SKP_EQU */
    "Type",				/* SKP_TYP */
    "Size",				/* SKP_SIZ */
    "Name collision",			/* SKP_NAM */
    "Exception List",			/* SKP_EXL */
    "Dot file",				/* SKP_DOT */
    "Backup file",			/* SKP_BKU */
    "Recovery not needed",		/* SKP_RES */
    "Access denied",			/* SKP_ACC */
    "Not a regular file",		/* SKP_NRF */
    "Simulated",			/* SKP_SIM */
    "Simulated - Remote file older",	/* SKP_XUP */
    "Simulated - No remote file",	/* SKP_XNX */
};
static int nskreason = (sizeof(skreason) / sizeof(char *));

char *
gskreason(n) int n; {
    return((n > 0 && n < nskreason) ? skreason[n] : "");
}

char pktfil[CKMAXPATH+1];               /* Packet log file name */

#ifndef NOMSEND                         /* Multiple SEND */
char *msfiles[MSENDMAX];
#endif /* NOMSEND */

#ifdef CK_TIMERS
extern long rttdelay;
extern int  rttflg;
#endif /* CK_TIMERS */
extern int rcvtimo;

#ifdef CK_RESEND
extern int sendmode;
extern CK_OFF_T sendstart, rs_len;
#endif /* CK_RESEND */

#ifdef CK_PCT_BAR                       /* File transfer thermometer */
int thermometer = 1;                    /* ON by default */
#endif /* CK_PCT_BAR */

#ifdef GFTIMER
CKFLOAT gtv = -1.0, oldgtv = -1.0;
#else
#ifndef OS2
static
#endif /* OS2 */
  long gtv = -1L, oldgtv = -1L;
#endif /* GFTIMER */

extern int server, bctu, rptflg, ebqflg, spsiz, urpsiz, wmax, czseen, cxseen,
  winlo, displa, timint, npad, ebq, bctr, rptq, atcapu, lpcapu,
  swcapu, wslotn, wslotr, rtimo, mypadn, sq, capas, rpsiz, tsecs,
  pktlog, lscapu, dest, srvdis, wslots, spackets, spktl, rpktl,
  retrans, wcur, numerrs, fsecs, whatru, crunched, timeouts,
  rpackets, fncnv, bye_active, discard, inserver, diractive, cdactive;

extern long filcnt, filrej, rptn, filcps, tfcps, cps, peakcps;
extern CK_OFF_T ffc, tfc, fsize; 

long oldcps = 0L;

extern CHAR *rdatap, padch, seol, ctlq, mypadc, eol, *epktmsg;
extern char *xfrmsg;

#ifdef IKSDB
FILE * dbfp = NULL;                     /* File pointer to database file */

int dbenabled = 1;                      /* Flag for database is enabled */
extern int ikdbopen;                    /* Flag for database is open */

unsigned long mydbseek = 0L;            /* Seek pointer to my record */
int mydbslot = 0;                       /* My slot number */
unsigned long myflags = 0L;             /* My flags */
unsigned long myatype = 0L;             /* My authorization type */
unsigned long myamode = 0L;             /* My authorization mode */
unsigned long mystate = 0L;             /* My state (SEND, RECEIVE, etc) */
unsigned long mypid = 0L;               /* My PID */
unsigned long myip = 0L;                /* My IP address */
unsigned long peerip = 0L;              /* My peer's IP address */

unsigned long dbip = 0L;                /* IP address in db record */
unsigned long dbpid = 0L;               /* PID in db record */
unsigned long dbflags = 0L;             /* Flags field in db record */
unsigned long dblastused = 0L;          /* Last in-use record in db */
char dbrec[DB_RECL];                    /* Database record buffer */

char * dbdir   = NULL;                  /* Database directory */
char * dbfile  = NULL;                  /* Database file full pathname */
char myhexip[33] = { NUL, NUL };        /* My IP address in hex */
char peerhexip[33] = { NUL, NUL };      /* Client's IP address in hex */
#endif /* IKSDB */

#ifdef GFTIMER
extern CKFLOAT fpfsecs, fptsecs, fpxfsecs;
#else
extern long xfsecs;
#endif /* GFTIMER */
#endif /* NOXFER */

#ifdef TCPSOCKET
#ifdef NEWFTP
extern char * ftp_host, ftp_srvtyp[];
extern int ftp_csx, ftp_csl, ftp_deb;
#endif /* NEWFTP */
extern char myipaddr[];
#endif /* TCPSOCKET */

#ifndef NOICP
#ifndef NOSPL
    extern struct mtab *mactab;         /* For ON_EXIT macro. */
    extern int nmac;
#endif /* NOSPL */
#ifdef DCMDBUF
extern char *cmdbuf;                    /* Command buffer */
#else
extern char cmdbuf[];                   /* Command buffer */
#endif /* DCMDBUF */
extern int cmd_quoting;
#endif /* NOICP */

#ifndef NOCCTRAP
#ifdef NT
#include <setjmpex.h>
#else /* NT */
#include <setjmp.h>
#endif /* NT */
#include "ckcsig.h"
extern ckjmpbuf cmjbuf;
#endif /* NOCCTRAP */

extern int xfiletype, nscanfile;

int
shoesc(escape) int escape; {
    extern char * ccntab[];		/* C0 control character name table */
    extern int tt_escape;
    if ((escape > 0 && escape < 32) || (escape == 127)) {
	printf(" Escape character: Ctrl-%c (ASCII %d, %s): %s\r\n",
	       ctl(escape),
	       escape,
	       (escape == 127 ? "DEL" : ccntab[escape]),
	       tt_escape ? "enabled" : "disabled"
	       );
    } else {
	printf(" Escape character: Code %d",escape);
	if (escape > 160 && escape < 256)
	  printf(" (%c)",escape);
	printf(": %s\r\n", tt_escape ? "enabled" : "disabled");	
    }
    return(0);
}

#ifndef NOXFER
/*  P R E S E T  --  Reset global protocol variables  */

extern int recursive;

#ifdef PATTERNS
int patterns = SET_AUTO;                /* Whether to use filename patterns */
extern int g_patterns;			/* For saving and restoring */
#else
int patterns = SET_OFF;
#endif /* PATTERNS */

#ifndef NOICP
#ifdef CK_LABELED
extern int g_lf_opts, lf_opts;
#endif /* CK_LABELED */
extern int g_matchdot, g_usepipes, usepipes;
extern int g_binary, g_proto, g_displa, g_spath, g_rpath, g_fncnv;
extern int g_recursive;
extern int g_xfermode, xfermode;
extern int g_urpsiz, g_spsizf, g_spsiz;
extern int g_spsizr, g_spmax, g_wslotr, g_prefixing, g_fncact;
extern int g_fnspath, g_fnrpath, g_skipbup;
extern int nolinks;
#ifdef CKSYMLINK
extern int zgfs_link;
#endif /* CKSYMLINK */
#ifndef NOSPL
extern int g_pflg, pwflg, g_pcpt, pwcrypt;
extern char * g_pswd, pwbuf[];
#endif /* NOSPL */
#endif /* NOICP */

extern int spsizf, spsizr, spmax, prefixing, fncact, fnspath, fnrpath;
extern int moving;                      /* SEND criteria */
extern char sndafter[], sndbefore[], *sndexcept[], *rcvexcept[];
extern CK_OFF_T sndlarger, sndsmaller, calibrate;
extern int rmailf, rprintf, skipbup;
extern char optbuf[];

#ifdef PIPESEND
extern char * g_sfilter, * g_rfilter;
extern char * sndfilter, * rcvfilter;
#endif /* PIPESEND */
extern char ** sndarray;

VOID
ftreset() {
#ifndef NOICP
    int i;
    extern char * filefile;
    extern int reliable, xreliable, c_save, ss_save, slostart, urclear;
    extern int oopts, omode, oname, opath, kactive, autopath;
    extern char * snd_move;             /* Directory to move sent files to */
    extern char * snd_rename;           /* What to rename sent files to */
    extern char * rcv_move;
    extern char * rcv_rename;
    extern char * g_snd_move;
    extern char * g_snd_rename;
    extern char * g_rcv_move;
    extern char * g_rcv_rename;

#ifdef CK_TMPDIR
    extern int f_tmpdir;
    extern char savdir[];
#endif /* CK_TMPDIR */

#ifdef CK_SPEED
#ifdef COMMENT
    extern int f_ctlp;
    extern short s_ctlp[], ctlp[];
#endif /* COMMENT */
#endif /* CK_SPEED */

#ifndef NOCSETS
    extern int fcs_save, tcs_save;
    extern int g_xfrxla, xfrxla;
#endif /* NOCSETS */

/* Restore / reset per-command file-transfer switches */

    makestr(&snd_move,g_snd_move);
    makestr(&rcv_move,g_rcv_move);
    makestr(&snd_rename,g_snd_rename);
    makestr(&rcv_rename,g_rcv_rename);

    kactive = 0;                        /* Kermit protocol no longer active */
    oopts = -1;                         /* O-Packet Options */
    omode = -1;                         /* O-Packet Transfer Mode */
    oname = -1;                         /* O-Packet Filename Options */
    opath = -1;                         /* O-Packet Pathname Options */

#ifdef CK_RESEND
    rs_len = 0L;			/* REGET position */
#endif /* CK_RESEND */

#ifdef COMMENT
#ifdef CK_SPEED
    if (f_ctlp) {
        for (i = 0; i < 256; i++)
          ctlp[i] = s_ctlp[i];
        f_ctlp = 0;
    }
#endif /* CK_SPEED */
#endif /* COMMENT */

#ifdef CK_TMPDIR
    if (f_tmpdir) {			/* If we changed to download dir */
	zchdir((char *) savdir);	/* Go back where we came from */
	f_tmpdir = 0;
    }
#endif /* CK_TMPDIR */

    calibrate = 0L;                     /* Calibration run */
    if (xreliable > -1) {
	reliable = xreliable;
	debug(F101,"ftreset reliable","",reliable);
    }
    urclear = 0;

    if (autopath) {                     /* SET RECEIVE PATHNAMES AUTO */
        fnrpath = PATH_AUTO;
        autopath = 0;
    }
    if (filefile) {                     /* File list */
        zclose(ZMFILE);
        makestr(&filefile,NULL);
    }
    if (c_save > -1) {                  /* Block Check Type */
        bctr = c_save;
        c_save = -1;
    }
    if (ss_save > -1) {                 /* Slow Start */
        slostart = ss_save;
        ss_save = -1;
    }
#ifdef CK_LABELED
    if (g_lf_opts > -1) {
        lf_opts = g_lf_opts;            /* Restore labeled transfer options */
        g_lf_opts = -1;
    }
#endif /* CK_LABELED */

#ifndef NOCSETS
    if (tcs_save > -1) {                /* Character sets */
        tcharset = tcs_save;
        tcs_save = -1;
    }
    if (fcs_save > -1) {
        fcharset = fcs_save;
        fcs_save = -1;
    }
    if (g_xfrxla > -1) {
	xfrxla = g_xfrxla;
	g_xfrxla = -1;
    }
    setxlatype(tcharset,fcharset);      /* Translation type */
#endif /* NOCSETS */

#ifdef NETCONN
#ifndef NOSPL
    if (g_pswd) {
        ckstrncpy(pwbuf,g_pswd,PWBUFL);
        makestr(&g_pswd,NULL);
    }
    if (g_pflg > -1) {
        pwflg = g_pflg;
        g_pflg = -1;
    }
    if (g_pcpt > -1) {
        pwcrypt = g_pcpt;
        g_pcpt = -1;
    }
#endif /* NOSPL */
#endif /* NETCONN */

    if (g_binary > -1) {                /* File type */
        binary = g_binary;
        g_binary = -1;
    }
    if (g_xfermode > -1) {              /* Transfer mode */
        xfermode = g_xfermode;
        g_xfermode = -1;
    }
#ifdef PATTERNS
    if (g_patterns > -1) {              /* Filename patterns */
        patterns = g_patterns;
        g_patterns = -1;
    }
#endif /* PATTERNS */

    if (g_usepipes > -1) {
        usepipes = g_usepipes;
        g_usepipes = -1;
    }
    if (g_matchdot > -1) {
        matchdot = g_matchdot;
        g_matchdot = -1;
    }
    if (g_proto > -1) {                 /* Protocol */
        protocol = g_proto;
        g_proto = -1;
    }
    if (g_urpsiz > -1) {
        urpsiz = g_urpsiz;
        debug(F101,"ftreset restoring urpsiz","",urpsiz);
        g_urpsiz = -1;
    }
    if (g_spsizf > -1) {
        spsizf = g_spsizf;
        debug(F101,"ftreset restoring spsizf","",spsizf);
        g_spsizf = -1;
    }
    if (g_spsiz > -1) {
        spsiz = g_spsiz;
        debug(F101,"ftreset restoring spsiz","",spsiz);
        g_spsiz = -1;
    }
    if (g_spsizr > -1) {
        spsizr = g_spsizr;
        debug(F101,"ftreset restoring spsizr","",spsizr);
        g_spsizr = -1;
    }
    if (g_spmax > -1) {
        spmax = g_spmax;
        g_spmax = -1;
    }
    if (g_wslotr > -1) {
        wslotr = g_wslotr;
        g_wslotr = -1;
    }
    if (g_prefixing > -1) {
        prefixing = g_prefixing;
        g_prefixing = -1;
    }
    if (g_fncact > -1) {
        fncact = g_fncact;
        g_fncact = -1;
    }
    if (g_fncnv > -1) {
        fncnv = g_fncnv;
        g_fncnv = -1;
    }
    if (g_fnspath > -1) {
        fnspath = g_fnspath;
        g_fnspath = -1;
    }
    if (g_fnrpath > -1) {
        fnrpath = g_fnrpath;
        g_fnrpath = -1;
    }
    if (g_skipbup > -1) {
        skipbup = g_skipbup;
        g_skipbup = -1;
    }
    nolinks = 2;			/* /FOLLOWLINKS is never global */
    recursive = 0;                      /* /RECURSIVE can never be global */
    xfiletype = -1;

    if (g_displa > -1) {                /* File transfer display */
        fdispla = g_displa;
        g_displa = -1;
    }
    if (g_spath > -1) {                 /* Send pathnames */
        fnspath = g_spath;
        g_spath = -1;
    }
    if (g_rpath > -1) {                 /* Receive pathnames */
        fnrpath = g_rpath;
        g_rpath = -1;
    }
    if (g_fncnv > -1) {                 /* Filename conversion */
        fncnv = g_fncnv;
        g_fncnv = -1;
    }
#ifdef PIPESEND
    makestr(&sndfilter,g_sfilter);      /* Send filter */
    makestr(&rcvfilter,g_rfilter);      /* Receive filter */
#endif /* PIPESEND */

#ifndef NOFRILLS
    rmailf = rprintf = 0;               /* MAIL and PRINT modifiers for SEND */
    optbuf[0] = NUL;                    /* MAIL and PRINT options */
#endif /* NOFRILLS */

    moving = 0;                         /* Reset delete-after-send indicator */
    sndafter[0]  = NUL;                 /* Reset SEND selection switches */
    sndbefore[0] = NUL;

    for (i = 0; i < NSNDEXCEPT; i++) {
        if (sndexcept[i])
          free(sndexcept[i]);
        sndexcept[i] = NULL;
        if (rcvexcept[i])
          free(rcvexcept[i]);
        rcvexcept[i] = NULL;
    }
    sndlarger =  (CK_OFF_T)-1;
    sndsmaller = (CK_OFF_T)-1;
    debug(F101,"present sndsmaller","",sndsmaller);
#ifdef GFTIMER
    gtv = -1.0;
    oldgtv = -1.0;
#else
    gtv = -1L;
    oldgtv = -1L;
#endif /* GFTIMER */
#endif /* NOICP */
}
#endif /* NOXFER */

char *
ttgtpn() {				/* Get typical port name */
/*
  Ideally this routine would be implemented in each of the cku?io.* modules,
  but that requires changing the API definition.
*/
    return(
#ifdef OS2
#ifdef OS2ONLY
"COM1"
#else  /* OS2ONLY */
"TAPI [ name ] or COM1"
#endif /* OS2ONLY */
#else  /* OS2 */
#ifdef VMS
"TXA0:, TTA0:, or LTA0:"
#else  /* VMS */
#ifdef SOLARIS
"/dev/cua/a"
#else  /* SOLARIS */
#ifdef HPUX10
"/dev/cua0p0"
#else  /* HPUX10 */
#ifdef HPUX
"/dev/cua00"
#else  /* HPUX */
#ifdef __FreeBSD__
"/dev/cuaa0"
#else  /* __FreeBSD__ */
#ifdef __linux__
"/dev/ttyS0"
#else  /* __linux__ */
#ifdef BSD44
"/dev/tty00"
#else  /* BSD44 */
#ifdef OSK
"/t1"
#else  /* OSK */
#ifdef QNX
"/dev/ser1"
#else  /* QNX */
#ifdef QNX6
"/dev/ser1"
#else  /* QNX6 */
#ifdef UNIXWARE
"/dev/term/00 or /dev/tty00"
#else  /* UNIXWARE */
#ifdef CK_SCOV5
"/dev/tty1A"
#else  /* CK_SCOV5 */
#ifdef CK_SCO32V4
"/dev/tty1A"
#else  /* CK_SCO32V4 */
#ifdef M_XENIX
"/dev/tty1A"
#else  /* M_XENIX */
#ifdef AIXRS
"/dev/tty0"
#else  /* AIXRS */
#ifdef DGUX
"/dev/tty00"
#else  /* DGUX */
#ifdef datageneral
"@con1"
#else  /* datageneral */
#ifdef IRIX
"/dev/ttym0"
#else  /* IRIX */
#ifdef SUNOS4
"/dev/ttyh0"
#else  /* SUNOS4 */
#ifdef SV68R3V6
"/dev/scc0"
#else  /* SV68R3V6 */
#ifdef MOTSV88R4
"/dev/contty00"
#else  /* MOTSV88R4 */
#ifdef NEXT
"/dev/cufa"
#else
#ifdef OSF
"/dev/ttyd1"
#else
#ifdef SINIX
"/dev/ttyc1"
#else
#ifdef UNIX
"/dev/cua, /dev/acu, /dev/tty0, etc"
#else  /* UNIX */
"(sorry no example available)"
#endif /* UNIX */
#endif /* SINIX */
#endif /* OSF */
#endif /* NEXT */
#endif /* MOTSV88R4 */
#endif /* SV68R3V6 */
#endif /* SUNOS4 */
#endif /* IRIX */
#endif /* datageneral */
#endif /* DGUX */
#endif /* AIX */
#endif /* M_XENIX */
#endif /* CK_SCO32V4 */
#endif /* CK_SCOV5 */
#endif /* UNIXWARE */
#endif /* QNX6 */
#endif /* QNX */
#endif /* OSK */
#endif /* BSD44 */
#endif /* __linux__ */
#endif /* __FreeBSD__ */
#endif /* HPUX */
#endif /* HPUX10 */
#endif /* SOLARIS */
#endif /* VMS */
#endif /* OS2 */
	   );
}

/*  C K _ E R R S T R  --  Return message from most recent system error */

#ifdef CKROOT
extern int ckrooterr;
#endif /* CKROOT */

char *
ck_errstr() {
#ifdef USE_STRERROR
#ifndef CK_ANSILIBS
    /* Should have been declared in <string.h> */
_PROTOTYP( char * strerror, (int) );
#endif /* CK_ANSILIBS */
#ifdef CKROOT
    if (ckrooterr)
      return("Off limits");
#endif /* CKROOT */
    return(strerror(errno));
#else  /* !USE_STRERROR */
#ifdef VMS
    extern char * ckvmserrstr(unsigned long);
#ifdef CKROOT
    if (ckrooterr)
      return("Off limits");
#endif /* CKROOT */
    return(ckvmserrstr(0L));
#else  /* !VMS */
#ifdef BSD44
#ifdef __386BSD__
#ifndef NDSYSERRLIST
    extern int sys_nerr;
    extern char *sys_errlist[];
#endif /* NDSYSERRLIST */
#else  /* !__386BSD__ */
#ifndef __bsdi__
#ifndef NDSYSERRLIST
    extern int sys_nerr;
    extern const char *const sys_errlist[];
#endif /* NDSYSERRLIST */
#endif /* __bsdi__ */
#endif /* __386BSD__ */
#ifdef CKROOT
    if (ckrooterr)
      return("Off limits");
    else
#endif /* CKROOT */
    if (errno >= sys_nerr)
      return("Error number out of range");
    else
      return((char *) sys_errlist[errno]);
#else /* !BSD44 */
#ifdef ATTSV
#ifndef NDSYSERRLIST
    extern int sys_nerr;
    extern char *sys_errlist[];
#endif /* NDSYSERRLIST */
#ifdef CKROOT
    if (ckrooterr)
      return("Off limits");
    else
#endif /* CKROOT */
    if (errno >= sys_nerr)
      return("Error number out of range");
    else
      return((char *) sys_errlist[errno]);
#else /* !ATTSV */
#ifdef BSD4
#ifndef NDSYSERRLIST
    extern int sys_nerr;
    extern char *sys_errlist[];
#endif /* NDSYSERRLIST */
#ifdef CKROOT
    if (ckrooterr)
      return("Off limits");
    else
#endif /* CKROOT */
    if (errno >= sys_nerr)
      return("Error number out of range");
    else
      return((char *) sys_errlist[errno]);
#else
#ifdef OS2
#ifndef NDSYSERRLIST
    extern char *sys_errlist[];
#endif /* NDSYSERRLIST */
#ifdef NT
    extern int_sys_nerr;
#endif /* NT */
    char *e;
#ifdef CKROOT
    if (ckrooterr)
      return("Off limits");
#endif /* CKROOT */
    e = (errno > -1
#ifdef NT
         && errno <= _sys_nerr
#endif /* NT */
         ) ?
#ifdef NT
         (char *) sys_errlist[errno]
#else /* NT */
         /* I don't know how to get a CLIB error string in OS/2 */
         strerror(errno)
#endif /* NT */
             : "";
    return(e ? e : "");
#else /* OS2 */
    return("");
#endif /* OS2 */
#endif /* BSD4 */
#endif /* ATTSV */
#endif /* BSD44 */
#endif /* VMS */
#endif /* USE_STRERROR */
}

#ifdef PATTERNS
/*
  Filename pattern recognition lists for automatic text/binary switching.
  These are somewhat passe after the addition of scanfile()  (7.0).
  But with the addition of FTP [M]GET, they're back in style (8.0).

  Although, with FTP the lists need to be used in the reverse.  With
  Kermit the list is used to imply the types of the local system.  Whereas
  with FTP, the list must be used to imply the type of the remote system.
  Therefore, all platforms must now support all of the lists.
*/
char *txtpatterns[FTPATTERNS+1] = { NULL, NULL };
char *binpatterns[FTPATTERNS+1] = { NULL, NULL };
/*
  Default pattern lists for each platform...

  NOTE: In most cases we leave ".hlp", ".ini", and ".scr" alone; although they
  are traditionally text types, they are binary in Windows.  So they are
  handled by the prevailing SET FILE TYPE, rather than automatically.
  Similarly for ".dat", ".inf", and so on.  Also ".ps" since PostScript files
  are not always text.  ".log" is omitted since logs can be text or binary,
  except in VMS they are usually text, etc etc.

  Later (Sep 2003): Add PostScript to binary patterns.
*/
static char *txtp[SYS_MAX][FTPATTERNS] = {
    /* UNKNOWN */ {
	NULL, NULL
    },
    {					/* UNIX */
    "*.txt","*.c","*.h","*.r","*.w","*.cpp","*.cc","*.ksc","*.bwr","*.upd",
    "*.html","*.htm","*.mss","*.tex","*.nr","[Mm]akefile", "*.hex", "*.hqx",
    "*.for","*.f77","*.f","*.F","*.s","*.pas","*.java","*.el","*.lisp","*.sh",
    "*.m4","*.perl","*.pl","*.pod","*.pm","*.awk","*.sno","*.spt","*.sed",
    "*.ksc","*.TXT", "*read.me", "*READ.ME", ".*", "*/.*", "*.mem","*.mac",
    NULL
    },
    {					/* WIN32 */
    "*.txt","*.ksc","*.htm","*.html","*.bat","*.cmd","*.jav","*.asm", "*.hex",
    "*.hqx", "*.c", "*.h", "*.cpp", "*.hpp", "*.cxx", "*.cxx", "*.w",
    "*.java", "*.bwr", "*.upd", "*.mak", "read.me", "*.map", "makefile",
    "*.mem","*.mac","*.cc","*.pl","*.pod","*.pm","*.m4",NULL
    },
    {					/* VMS */
    "*.com","*.txt","*.c",  "*.for","*.pas","*.rno","*.rnh","*.mar","*.bli",
    "*.hlp","*.mss","*.doc","*.bwr","*.cld","*.hex","*.bas","*.ini","*.log",
    "*.mms","*.opt","*.ksc","*.perl","*.pl","*.pod","*.pm","*.sno","*.spt",
    "*.mem",NULL
    },
    {					/* OS2 */
    "*.txt","*.ksc","*.htm","*.html","*.bat","*.cmd","*.jav","*.asm", "*.hex",
    "*.hqx", "*.c", "*.h", "*.cpp", "*.hpp", "*.cxx", "*.cxx", "*.w",
    "*.java", "*.bwr", "*.upd", "*.mak", "read.me", "*.map", "makefile",
    NULL
    },
    {					/* DOS */
    "*.txt","*.ksc","*.htm","*.bat","*.cmd","*.jav","*.asm", "*.hex",
    "*.hqx", "*.c", "*.h", "*.cpp", "*.hpp", "*.cxx", "*.cxx", "*.w",
    "*.bwr", "*.upd", "*.mak", "read.me", "*.map", "makefile", NULL
    },
    {					/* TOPS-10 */
    "*.cmd","*.hlp","*.doc","*.ini","*.txt","*.mac","*.for","*.sai","*.bli",
    "*.pas","*.sno","*.spt","*.pcl","*.mss","*.rno","*.b36","*.tex","*.pub",
    "*.req","*.r36","*.mem","*.bwr","*.ccl","*.ctl","*.rnh","*.ksc",NULL
    },
    {					/* TOPS-20 */
    "*.cmd","*.hlp","*.doc","*.ini","*.txt","*.mac","*.for","*.sai","*.bli",
    "*.pas","*.sno","*.spt","*.pcl","*.mss","*.rno","*.b36","*.tex","*.pub",
    "*.req","*.r36","*.mem","*.bwr","*.ccl","*.ctl","*.rnh","*.ksc",NULL
    },
    {					/* STRATUS VOS */
    "*.txt","*.ksc","*.htm","*.html","*.bat", "*.cmd","*.jav","*.asm","*.hex",
    "*.hqx","*.c",  "*.h",  "*.w",   "*.java","*.bwr","*.upd","*.ttp","*.cm",
    "*.pl1","*.emacs", "read.me", "*.pl", "makefile", NULL
    },
    {					/* DG AOS/VS */
    "*.txt", "*.c", "*.h", "*.w", "*.er", "*.bwr", "*.upd", "read.me",
    "*.cli", "*.ksc", NULL
    },
    {					/* OSK */
    "*.c","*.cpp","*.h","*.a","*akefile", /* program sources */
    "*.for","*.f77","*.f","*.F","*.s","*.pas","*.java","*.el","*.lisp",
    "*.sh","*.perl","*.awk","*.sno","*.spt","*.sed",
    "*.txt","*.w",			/* general text */
    "*.ksc","*.bwr","*.upd",
    "*.html","*.htm","*.mss","*.tex","*.nr","*.hex", "*.hqx",
    "*.TXT", "*read.me", "*READ.ME", ".*", "*/.*",
    NULL
    }
};

/* Note: .DOC added to (some) binary patterns June 1998... Microsoft wins. */

static char *binp[SYS_MAX][FTPATTERNS] = {
    {					/* UNKNOWN */
    NULL, NULL
    },
    {					/* UNIX */
    "*.gz","*.Z","*.tgz","*.gif", "*.tar","*.zip","*.o","*.so","*.a","*.out",
    "*.exe", "*.jpg", "*.jpeg", "*.tif","*.tiff", "*.pdf", "*.so.*", "*.class",
    "*.rpm", "*.bmp", "*.bz2", "*.BMP", "*.dll", "*.doc", "*.vxd", "*.dcx",
    "*.xl*", "*.lzh", "*.lhz", "*.au", "*.voc", "*.mpg", "*.mpeg","[wk]ermit",
    "*.ps", NULL
    },
    {					/* WIN32 */
    "*.exe", "*.zip", "*.obj", "*.com", "*.gif", "*.jpg", "*.wav", "*.ram",
    "*.class","*.cla","*.dll", "*.drv", "*.ocx", "*.vbx", "*.lib", "*.ico",
    "*.bmp", "*.tif", "*.tar", "*.gz",  "*.tgz", "*.xl*", "*.doc", "*.vxd",
    "*.pdf", "*.lzh", "*.vxd", "*.snd", "*.au", "* .voc", "*.mpg", "*.mpeg",
    "*.ps", NULL
    },
    {					/* VMS */
    "*.exe","*.obj","*.bak","*.bin","*.adf","*.stb","*.mai","*.sys","*.dmp",
    "*.ps", "*.dat","*.par", NULL
    },
    {					/* OS2 */
    "*.exe", "*.zip", "*.obj", "*.com", "*.gif", "*.jpg", "*.wav", "*.ram",
    "*.class", "*.cla", "*.dll", "*.drv", "*.ocx", "*.vbx", "*.lib", "*.ico",
    "*.bmp", "*.tif", "*.tar", "*.gz", "*.tgz", "*.xl*", "*.doc", "*.vxd",
    "*.pdf", "*.ps", "*.lzh", NULL
    },
    {					/* DOS */
    "*.exe", "*.zip", "*.obj", "*.com", "*.gif", "*.jpg", "*.wav", "*.ram",
    "*.cla", "*.dll", "*.drv", "*.ocx", "*.vbx", "*.lib", "*.ico",
    "*.bmp", "*.tif", "*.tar", "*.gz", "*.tgz", "*.xl*", "*.doc", "*.vxd",
    "*.pdf", "*.ps", "*.lzh", NULL
    },
    {					/* TOPS10 */
    "*.exe","*.sav","*.bin","*.rim","*.rel","*.unv","*.lib","*.tap","*.dvi",
    "*.ps", NULL
    },
    {					/* TOPS20 */
    "*.exe","*.sav","*.bin","*.rim","*.rel","*.unv","*.lib","*.tap","*.dvi",
    "*.ps", NULL
    },
    {					/* STRATUS VOS */
    "*.exe", "*.zip", "*.obj", "*.com", "*.gif", "*.jpg", "*.wav", "*.ram",
    "*.class", "*.cla", "*.dll", "*.drv", "*.ocx", "*.vbx", "*.lib", "*.ico",
    "*.bmp", "*.tif", "*.tar", "*.gz", "*.tgz", "*.xl*", "*.doc", "*.vxd",
    "*.pdf", "*.ps", "*.lzh", "*.pm", NULL
    },
    {					/* DG */
    "*.ob", "*.pr", "*.dmp", "*.ps", NULL
    },
    { /* OSK */
    "*.gz","*.Z","*.z","*.tgz","*.lhz","*.tar",	/* archivers */
    "*.zip","*.ar","*.zoo","*.rpm","*.lzh",
    /* object files, libraries, executables */
    "*.r","*.l","*.exe", "*.dll", "*.so.*", "*.class",
    /* images */
    "*.gif", "*.jpg", "*.jpeg", "*.tif","*.tiff", "*.pdf", "*.ps",
    "*.bmp", "*.bz2", "*.BMP","*.pcx",
    NULL
    }
};

/*
  Set up default pattern lists so they can be freed and re-malloc'd.
  Each pattern list must terminated by a null element.
*/
VOID
initpat() {
    int i;
    for (i = 0; i < FTPATTERNS; i++) {
        txtpatterns[i] = NULL;
        binpatterns[i] = NULL;
    }
    for (i = 0; i < FTPATTERNS; i++) {
#ifdef UNIX
        makestr(&(txtpatterns[i]),txtp[SYS_UNIX][i]);
#else /* UNIX */
#ifdef OS2
#ifdef NT
        makestr(&(txtpatterns[i]),txtp[SYS_WIN32][i]);
#else /* NT */
        makestr(&(txtpatterns[i]),txtp[SYS_OS2][i]);
#endif /* NT */
#else /* OS2 */
#ifdef VMS
        makestr(&(txtpatterns[i]),txtp[SYS_VMS][i]);
#else /* VMS */
#ifdef STRATUS
        makestr(&(txtpatterns[i]),txtp[SYS_VOS][i]);
#else /* STRATUS */
#ifdef datageneral
        makestr(&(txtpatterns[i]),txtp[SYS_DG][i]);
#else /* datageneral */
#ifdef OSK
        makestr(&(txtpatterns[i]),txtp[SYS_OSK][i]);
#else /* OSK */
        makestr(&(txtpatterns[i]),txtp[SYS_UNK][i]);
#endif /* OSK */
#endif /* datageneral */
#endif /* STRATUS */
#endif /* VMS */
#endif /* OS2 */
#endif /* UNIX */
        if (!txtp[i])
          break;
    }
    for (i = 0; i < FTPATTERNS; i++) {
#ifdef UNIX
        makestr(&(binpatterns[i]),binp[SYS_UNIX][i]);
#else /* UNIX */
#ifdef OS2
#ifdef NT
        makestr(&(binpatterns[i]),binp[SYS_WIN32][i]);
#else /* NT */
        makestr(&(binpatterns[i]),binp[SYS_OS2][i]);
#endif /* NT */
#else /* OS2 */
#ifdef VMS
        makestr(&(binpatterns[i]),binp[SYS_VMS][i]);
#else /* VMS */
#ifdef STRATUS
        makestr(&(binpatterns[i]),binp[SYS_VOS][i]);
#else /* STRATUS */
#ifdef datageneral
        makestr(&(binpatterns[i]),binp[SYS_DG][i]);
#else /* datageneral */
#ifdef OSK
        makestr(&(binpatterns[i]),binp[SYS_OSK][i]);
#else /* OSK */
        makestr(&(binpatterns[i]),binp[SYS_UNK][i]);
#endif /* OSK */
#endif /* datageneral */
#endif /* STRATUS */
#endif /* VMS */
#endif /* OS2 */
#endif /* UNIX */
        if (!binp[i])
          break;
    }
}

/*
  m a t c h n a m e  --  Compare filename with text & binary name patterns.

  Returns:
    0 if name matches a text pattern but not a binary pattern.
    1 if name matches a binary pattern but not a text pattern.
   -1 if name matches no patterns.
   -2 if name matches a binary pattern and a text pattern.
*/
int
matchname(filename, local, os) char * filename; int local; int os; {
    int rc = -1;			/* Return code */
    char * name, * p;
#ifdef OS2ORUNIX
    char tmpbuf[CKMAXPATH+1];
#endif /* OS2ORUNIX */

    name = filename ? filename : "";	/* Copy of original arg */
    if (patterns && *name) {		/* If PATTERNS ON... */
	int i;

#ifdef OS2ORUNIX
	if (ckmatch("*.~[1-9]*~",name,1,1)) { /* Name has backup suffix? */
	    int k;
	    k = ckstrncpy(tmpbuf,name,CKMAXPATH+1); /* Yes, copy and strip */
	    for (i = k - 3; i > 4; i--) {
		if (tmpbuf[i] == '~' && tmpbuf[i-1] == '.') {
		    tmpbuf[i-1] = NUL;
		    break;
		}
	    }
	    name = tmpbuf;		/* And point to stripped copy */
	}
#endif /* OS2ORUNIX */
	zstrip(name,&p);		/* Strip pathname too */
	name = p;

        if (local) {
            if (txtpatterns[0]) {	/* Search text patterns */
                for (i = 0; i < FTPATTERNS && txtpatterns[i]; i++) {
                    if (ckmatch(txtpatterns[i],name,filecase,1)) {
                        rc = 0;
                        break;
                    }
                }
            }
            if (binpatterns[0]) {	/* And search binary patterns */
                for (i = 0; i < FTPATTERNS && binpatterns[i]; i++) {
                    if (ckmatch(binpatterns[i],name,filecase,1)) {
                        rc = (rc > -1) ? -2 : 1;
                        break;
                    }
                }
            }
	} else {
            if (os >= 0 && os < SYS_MAX) {
		if (txtp[os][0]) {
		    for (i = 0; i < FTPATTERNS && txtp[os][i]; i++) {
			if (ckmatch(txtp[os][i],name,filecase,1)) {
			    rc = 0;
			    break;
			}
		    }
		}
		if (binp[os][0]) {
		    for (i = 0; i < FTPATTERNS && binp[os][i]; i++) {
			if (ckmatch(binp[os][i],name,filecase,1)) {
			    rc = (rc > -1) ? -2 : 1;
			    break;
			}
		    }
		}
	    }
        }
    }
    debug(F111,"matchname",name,rc);
    return(rc);
}
#endif /* PATTERNS */

#ifdef UNICODE
#ifndef NOEVENMAX
#define EVENMAX
#endif /* NOEVENMAX */
#endif /* UNICODE */

/*  S C A N F I L E  --  Analyze a file's contents  */

/*
  Call with:
    name:    Pointer to name of existing file.
    flag:    Pointer to int in which to return additional numeric data.

  Returns:
    -1 on failure (to open file or to read from it).
    Integer, 0..4, on success indicating file type:
     0 = 7-bit text (flag = -1)
     1 = UTF-8 text (flag = -1)
     2 = UCS-2 text (flag =  0: big-endian; flag = 1: little-endian)
     3 = 8-bit text (flag =  0: no C1 bytes; flag = 1: includes C1 bytes)
     4 = Text       (type unknown)
     5 = binary     (flag = -1)

  If UNICODE is defined:

   1. If file begins with a valid BOM, it is believed.  Otherwise we
      read the first 4K of the file (since it might be email with verbose
      headers) and analyze it:

   2. If file contains only valid UTF-8 sequences, we call it UTF-8;
      otherwise:

   3. If the file contains lots of alternate 0 bytes, we call it UCS-2, and
      set the polarity according to whether the preponderance of them are in
      even or odd positions; otherwise:

   4. If EVENMAX is defined and the file contains lots of alternate bytes that
      are identical, even if they aren't zero, and the number of such bytes
      is at least four times the length of the maximum run of alternating
      identical bytes of the opposite polarity, we call it UCS-2; otherwise:

   5. If the file contained no bytes with their 8th bits on and no controls
      other than CR, LF, HT, and FF, we call it ASCII; otherwise:

   6. If it contains C0 control characters other than CR, LF, HT, and FF, we
      call it binary; otherwise:

   7. We call it 8-bit text, character set unknown (could be Latin-1 or
      anything else).

   Note that malformed UTF-8 is not diagnosed as UTF-8.

   If UNICODE is not defined:

   1. If the file contains C0 control characters other than CR, LF, HT, and
      FF, we call it binary; otherwise:

   2. If the file contains any 8-bit bytes, we call it 8-bit text; otherwise:

   3. We call it 7-bit text.

   In the non-Unicode case, UCS-2 is diagnosed as binary, but UTF-8 as
   8-bit text.

   There is no significant speed difference between the Unicode and
   non-Unicode cases.
*/
int
scanfile(name,flag,nscanfile) char * name; int * flag, nscanfile; {
    FILE * fp;				/* File pointer */
    unsigned char buf[SCANFILEBUF];	/* File data buffer for analysis */
    int x, val = -1, count = 0;		/* Workers */
    int rc = -1;			/* Return code */
    int pv = -1;			/* Pattern-match value */
    int eof = 0;			/* Flag for file EOF encountered */
    int bytes = 0;			/* Total byte count */
#ifdef UNICODE
    unsigned int c0, c1;		/* First 2 file bytes (for BOM) */
#endif /* UNICODE */
    extern int pipesend, filepeek;

    register int i;			/* Loop control */
    int readsize = 0;			/* How much to read */
    int eightbit = 0;			/* Number of bytes with 8th bit on */
    int c0controls = 0;			/* C0 non-text control-char counter */
    int c0noniso = 0;			/* C0 non-ISO control-char counter */
    int c1controls = 0;			/* C1 control-character counter */
    unsigned int c;			/* Current character */
    int runmax = 0;			/* Longest run of 0 bytes */
    int runzero = 0;			/* Run of 0 bytes */
    int pctzero = 0;			/* Percentage of 0 bytes */
    int txtcz = 0;
#ifdef CK_CTRLZ
    extern int eofmethod;
#endif /* CK_CTRLZ */

#ifdef UNICODE
    int notutf8 = 0;			/* Nonzero if definitely not UTF-8 */
    int utf8state = 0;			/* UTF-8 recognizer state */
    int oddzero = 0;			/* Number of 0 bytes in odd postions */
    int evenzero = 0;			/* and in even positions */
    int lfnul = 0;			/* Number of <LF><NUL> sequences */
    int crlf = 0;			/* Number of <CRLF> sequences */
#else
    int notutf8 = 1;
#endif /* UNICODE */

#ifdef COMMENT
#ifdef EVENMAX
    int oddrun = 0, oddmax = 0, oddbyte = 0, oddmaxbyte = 0;
    int evenrun = 0, evenmax = 0, evenbyte = 0, evenmaxbyte = 0;
#endif /* EVENMAX */
#endif /* COMMENT */

#ifndef NOXFER
    if (pipesend || calibrate || sndarray) /* Only for real files */
      return(-1);
#endif /* NOXFER */
    debug(F111,"scanfile",name,nscanfile);
#ifdef PATTERNS
    if (!filepeek) {
	pv = matchname(name,1,-1);
	if (pv < 0)
	  rc = -1;
	else
	  rc = (pv == 1) ? FT_BIN : FT_TEXT;
	debug(F111,"scanfile !filepeek result",name,rc);
	return(rc);
    }
#endif /* PATTERNS */

#ifdef VMS
/* We don't scan in VMS where text files have various record formats in  */
/* which record headers contain seemingly non-text bytes.  So the best   */
/* we can do in VMS is tell whether the file is text or binary, period.  */
    {
	int b, x;
	b = binary;			/* Save current binary setting */
	if (zopeni(ZIFILE,name) > 0) {	/* In VMS this sets binary */
	    x = binary;			/* Get result */
	    zclose(ZIFILE);		/* Close the file */
	    binary = b;			/* Restore previous binary setting */
	    rc = x ? FT_BIN : FT_TEXT;
	    val = 0;
	    goto xscanfile;
	}
    }
#endif /* VMS */

    eof = 0;				/* End-of-file reached indicator */
#ifdef OS2
    fp = fopen(name, "rb");		/* Open the file in binary mode */
#else
    fp = fopen(name, "r");
#endif /* OS2 */

    if (!fp)				/* Failed? */
      return(-1);

    while (1) {				/* One or more gulps from file */
	if (eof) {			/* EOF from last time? */
	    debug(F111,"scanfile at EOF",name,bytes);
	    if (runzero > runmax)
	      runmax = runzero;
	    break;
	}
	if (nscanfile < 0) {		/* Reading whole file */
	    readsize = SCANFILEBUF;
	} else {			/* Reading first nscanfilee bytes */
	    readsize = nscanfile - bytes;
	    if (readsize < 1)
	      break;
	    if (readsize > SCANFILEBUF)
	      readsize = SCANFILEBUF;
	}
	debug(F101,"scanfile readsize","",readsize);
	count = fread(buf,1,readsize,fp); /* Read a buffer */
	if (count == EOF || count == 0) {
	    debug(F111,"scanfile EOF",name,count);
	    break;
	}
	debug(F111,"scanfile buffer ok",name,count);

	if (bytes == 0 && count > 8) {
	    /* PDF files can look like text in the beginning. */
	    if (!ckstrcmp((char *)buf,"%PDF-1.",7,1)) {
		if (isdigit(buf[7])) {
		    if (buf[8] == '\015' ||
			count > 9 && buf[8] == SP && buf[9] == '\015') {
#ifdef DEBUG
			buf[8] = NUL;
			debug(F110,"scanfile PDF",buf,0);
#endif /* DEBUG */
			binary = 1;	/* But they are binary. */
			break;
		    }
		}
	    } else if (!ckstrcmp((char *)buf,"%!PS-Ado",8,1)) {
		/* Ditto for PostScript */
#ifdef DEBUG
		int i;
		for (i = 8; i < count; i++) {
		    if (buf[i] < '!') {
			buf[i] = NUL;
			break;
		    }
		}
		debug(F110,"scanfile PostScript",buf,0);
#endif /* DEBUG */
		binary = 1;
		break;
#ifndef NOPCLSCAN
	    } else if (!ckstrcmp((char *)buf,") HP-PCL",8,1)) {
		/* HP PCL printer language */
#ifdef DEBUG
		int i;
		for (i = 8; i < count; i++) {
		    if (buf[i] < '!') {
			buf[i] = NUL;
			break;
		    }
		}
		debug(F110,"scanfile PCL",buf,0);
#endif /* DEBUG */
		binary = 1;
		break;
	    } 
#endif /* NOPCLSCAN */
#ifndef NOPJLSCAN
	      else if (buf[0] == '\033' && (buf[1] == 'E' || buf[1] == '%')) {
		/* Ditto for PJL Job printer header */
#ifdef DEBUG
		int i;
		for (i = 2; i < count; i++) {
		    if (buf[i] < '!') {
			buf[i] = NUL;
			break;
		    }
		}
		debug(F110,"scanfile PJL Job printer header",buf,0);
#endif /* DEBUG */
		binary = 1;
		break;
#endif /* NOPJLSCAN */
	    }
	}

#ifdef UNICODE
	if (bytes == 0 && count > 1) {
	    int incl_cnt = 0;

	    /* First look for BOM */

	    c0 = (unsigned)((unsigned)buf[0]&0xFF); /* First file byte */
	    c1 = (unsigned)((unsigned)buf[1]&0xFF); /* Second byte */

	    if (c0 == 0xFE && c1 == 0xFF) {	/* UCS-2 BE */
		rc = FT_UCS2;
		val = 0;
		debug(F111,"scanfile UCS2 BOM BE",ckitoa(val),rc);
		incl_cnt++;
	    } else if (c0 == 0xFF && c1 == 0xFE) { /* UCS-2 LE */
		rc = FT_UCS2;
		val = 1;
		debug(F111,"scanfile UCS2 BOM LE",ckitoa(val),rc);
		incl_cnt++;
	    } else if (count > 2) if (c0 == 0xEF && c1 == 0xBB &&
		       (unsigned)((unsigned)buf[2]&0xFF) == 0xBF) {
		rc = FT_UTF8;
		debug(F111,"scanfile UTF8 BOM",ckitoa(val),rc);
		incl_cnt++;
	    }
	    if (incl_cnt) {		/* Have BOM */
		bytes += count;
		goto xscanfile;
	    }
	}
#endif /* UNICODE */

	bytes += count;			/* Count bytes read */
	eof = feof(fp);			/* Flag for at EOF  */

	for (i = 0; i < count; i++) {	/* For each byte... */
	    c = (unsigned)buf[i];	/* For ease of reference */
	    if (!c) {			/* Zero byte? */
#ifdef EVENMAX
		if (i&1)		/* In odd position */
		  oddzero++;
		else
		  evenzero++;		/* In even position */
#endif /* EVENMAX */
		runzero++;
	    } else {			/* Not a zero byte */
		if (runzero > runmax)
		  runmax = runzero;
		if (runmax > 2)		/* That's all we need to be certain */
		  break;		/* it's a binary file. */
		runzero = 0;
	    }

#ifdef COMMENT
#ifdef EVENMAX

/* This is to catch UCS-2 with a non-ASCII, non-Latin-1 repertoire  */

	    if (i > 1) {	      /* Look for runs of alternating chars */
		if (i&1) {
		    if (c == buf[i-2]) { /* In odd positions */
			oddrun++;
			oddbyte = c;
		    } else {
			oddmax = oddrun;
			oddmaxbyte = oddbyte;
		    }
		} else {		/* and even positions */
		    if (c == buf[i-2]) {
			evenrun++;
			evenbyte = c;
		    } else {
			evenmax = evenrun;
			evenmaxbyte = evenbyte;
		    }
		}
	    }
#endif /* EVENMAX */
#endif /* COMMENT */

	    if ((c & 0x80) == 0) {	/* We have a 7-bit byte */
#ifdef UNICODE
		if (i > 0 && c == 10) { /* Linefeed */
		    if (buf[i-1] == 0) lfnul++; /* Preceded by NUL */
		    else if (buf[i-1] == 13) crlf++; /* or by CR... */
		}
#endif /* UNICODE */
		if (c < ' ') {		/* Check for CO controls */
		    if (c != LF && c != CR && c != HT && c != FF) {
			c0controls++;
			if (c != ESC && c != SO && c != SI)
			  c0noniso++;
		    }
		    if ((c == '\032')	/* Ctrl-Z */
#ifdef COMMENT
			&& eof && (i >= count - 2)
#endif /* COMMENT */
			) {
			c0controls--;
			c0noniso--;
#ifdef CK_CTRLZ
			if (eofmethod == XYEOF_Z && txtcz == 0) {
			    if (c0controls == 0) /* All text prior to Ctrl-Z */
			      txtcz = 1;
			}
#endif /* CK_CTRLZ */
		    }
		}
#ifdef UNICODE
		if (!notutf8 && utf8state) { /* In UTF-8 sequence? */
		    utf8state = 0;
		    debug(F000,"scanfile","7-bit byte in UTF8 sequence",c);
		    notutf8++;		/* Then it's not UTF-8 */
		    continue;
		}
#endif /* UNICODE */
	    } else {			/* We have an 8-bit byte */
		eightbit++;		/* Count it */
		if (c >= 0x80 && c < 0xA0) /* Check for C1 controls */
		  c1controls++;
#ifdef UNICODE
		if (!notutf8) {		/* If it might still be UTF8... */
		    switch (utf8state) { /* Enter the UTF-8 state machine */
		      case 0:		 /* First byte... */
			if ((c & 0xE0) == 0xC0) { /* Tells number of */
			    utf8state = 1;        /* subsequent bytes */
			} else if ((c & 0xF0) == 0xE0) {
			    utf8state = 2;
			} else if ((c & 0xF8) == 0xF0) {
			    utf8state = 3;
			} else {
			    notutf8++;
			}
			break;
		      case 1:		/* Subsequent byte */
		      case 2:
		      case 3:
			if ((c & 0xC0) != 0x80) { /* Must start with 10 */
			    debug(F000,"scanfile",
				  "bad byte in UTF8 sequence",c);
			    notutf8++;
			    break;
			}
			utf8state--;	/* Good, one less in this sequence */
			break;
		      default:		/* Shouldn't happen */
			debug(F111,"scanfile","bad UTF8 state",utf8state);
			notutf8++;
		    }
		}
#endif /* UNICODE */
	    }
	}
    }
    fclose(fp);				/* Close the file */
    debug(F101,"scanfile bytes","",bytes);

    if (bytes == 0)			/* If nothing was read */
      return(-1);			/* we're done. */

#ifdef EVENMAX
    /* In case we had a run that never broke... */
#ifdef COMMENT
    if (oddmax == 0) {
	oddmax = oddrun;
	oddmaxbyte = oddbyte;
    }
    if (evenmax == 0) {
	evenmax = evenrun;
	evenmaxbyte = evenbyte;
    }
#endif /* COMMENT */
    if (runmax == 0) {
	runmax = runzero;
    }
#endif /* EVENMAX */

#ifdef UNICODE
    if (bytes > 100)			/* Bytes is not 0 */
      pctzero = (evenzero + oddzero) / (bytes / 100);
    else
      pctzero = ((evenzero + oddzero) * 100) / bytes;
#endif /* UNICODE */

#ifdef DEBUG
    if (deblog) {			/* If debugging, dump statistics */
	debug(F101,"scanfile c0controls ","",c0controls);
	debug(F101,"scanfile c0noniso   ","",c0noniso);
	debug(F101,"scanfile c1controls ","",c1controls);
	debug(F101,"scanfile eightbit   ","",eightbit);
#ifdef UNICODE
	debug(F101,"scanfile crlf       ","",crlf);
	debug(F101,"scanfile lfnul      ","",lfnul);
	debug(F101,"scanfile notutf8    ","",notutf8);
	debug(F101,"scanfile evenzero   ","",evenzero);
	debug(F101,"scanfile oddzero    ","",oddzero);
	debug(F101,"scanfile even/odd   ","",(evenzero / (oddzero + 1)));
	debug(F101,"scanfile odd/even   ","",(oddzero / (evenzero + 1)));
	debug(F101,"scanfile pctzero    ","",pctzero);
#endif /* UNICODE */
#ifdef COMMENT
#ifdef EVENMAX
	debug(F101,"scanfile oddmax     ","",oddmax);
	debug(F101,"scanfile oddmaxbyte ","",oddmaxbyte);
	debug(F101,"scanfile evenmax    ","",evenmax);
	debug(F101,"scanfile evenmaxbyte","",evenmaxbyte);
#endif /* EVENMAX */
#endif /* COMMENT */
	debug(F101,"scanfile runmax     ","",runmax);
    }
#endif /* DEBUG */

#ifdef UNICODE
    x = eightbit ? bytes / 20 : bytes / 4; /* For UCS-2... */

    if (runmax > 2) {			/* File has run of more than 2 NULs */
	debug(F100,"scanfile BIN runmax","",0);
	rc = FT_BIN;			/* so it can't be any kind of text. */
	goto xscanfile;

    } else if (rc == FT_UCS2 || (rc == FT_UTF8 && runmax == 0)) {
	goto xscanfile;			/* File starts with a BOM */

    } else if (eightbit > 0 && !notutf8) { /* File has 8-bit data */
	if (runmax > 0) {		   /* and runs of NULs */
	    debug(F100,"scanfile BIN (nnUTF8) runmax","",0);
	    rc = FT_BIN;		   /* UTF-8 doesn't have NULs */
	} else {			   /* No NULs */
	    debug(F100,"scanfile UTF8 (nnUTF8 + runmax == 0)","",0);
	    rc = FT_UTF8;		   /* and not not UTF-8, so is UTF-8 */
	}
	goto xscanfile;
    }
/*
  For UCS-2 detection, see if the text contains lines delimited by
  ASCII controls and containing spaces, ASCII digits, or other ASCII
  characters, thus forcing the presence of a certain percentage of zero bytes.
  For this purpose require 20% zero bytes, with at least six times as many
  in even (odd) positions as in odd (even) positions.
*/
    if ((evenzero >= x && oddzero == 0) ||
	((((evenzero / (oddzero + 1)) > 6) && (pctzero > 20)) &&
	(crlf == 0) &&
	(lfnul > 1))
	) {
	    debug(F100,"scanfile UCS2 noBOM BE (even/oddzero)","",0);
	rc = FT_UCS2;
	val = 0;
    } else if ((evenzero == 0 && oddzero >= x) ||
	       ((((oddzero / (evenzero + 1)) > 6) && (pctzero > 20)) &&
	       (crlf == 0) &&
	       (lfnul > 1))
	       ) {
	debug(F100,"scanfile UCS2 noBOM LE (even/oddzero)","",0);
	rc = FT_UCS2;
	val = 1;

#ifdef COMMENT
#ifdef EVENMAX
/*
  If the tests above fail, we still might have UCS-2 if there are significant
  runs of identical bytes in alternating positions, but only if it also has
  unusual C0 controls (otherwise we'd pick up hex files here).  NOTE: We
  don't actually do this -- EVENMAX is not defined (see comments above at
  first occurrence of EVENMAX).
*/
    } else if (c0noniso && evenmax > bytes / 4) {
	debug(F100,"scanfile UCS2 BE (evenmax)","",0);
	rc = FT_UCS2;
	val = 0;
    } else if (c0noniso && oddmax > bytes / 4) {
	debug(F100,"scanfile UCS2 LE (evenmax)","",0);
	rc = FT_UCS2;
	val = 1;
#endif /* EVENMAX */
#endif /* COMMENT */

    }
/*
  It seems to be UCS-2 but let's be more certain since there is no BOM...
  If the number of 7- and 8-bit characters is approximately equal, it might
  be a compressed file.  In this case we decide based on the name.
*/
    if (rc == FT_UCS2) {
	if (eightbit > 0) {
	    int j, k;
	    j = (c1controls * 100) / (c0controls + 1);
	    debug(F101,"scanfile c1/c0      ","",j);
	    k = (bytes * 100) / eightbit;
	    debug(F101,"scanfile pct 8bit   ","",k);
	    if (k > 40 && k < 60 && j > 60) {
		if (ckmatch("{*.Z,*.gz,*.zip,*.ZIP}",name,1,1)) {
		    debug(F110,"scanfile 8-bit BIN compressed",name,0);
		    rc = FT_BIN;
		    goto xscanfile;
		}
	    }
	}
	/* Small file - not enough evidence unless ... */

	if (bytes < 100) {
	    if (oddzero != 0 && evenzero != 0) {
		debug(F100,"scanfile small UCS2 doubtful","",0);
		rc = FT_BIN;
		goto xscanfile;
	    } else if (oddzero == 0 && evenzero == 0) {
		rc = eightbit ? FT_8BIT : FT_7BIT;
	    }
	}
	goto xscanfile;			/* Seems to be UCS-2 */
    }

/* If none of the above, it's probably not Unicode.  */

    if (!eightbit) {			/* It's 7-bit */
	if (c0controls) {		/* This would be strange */
	    if ((c0noniso > 0) && (txtcz == 0)) {
		debug(F100,"scanfile 7-bit BIN (c0coniso)","",0);
		rc = FT_BIN;
	    } else {
		debug(F100,"scanfile 7-bit ISO2022 TEXT (no c0noniso)","",0);
		rc = FT_7BIT;
	    }
	} else {			/* 7-bit text */
	    debug(F100,"scanfile 7-bit TEXT (no c0controls)","",0);
	    rc = FT_7BIT;
	}
    } else if (!c0noniso || txtcz) {	/* 8-bit text */
	debug(F100,"scanfile 8-bit TEXT (no c0noniso)","",0);
	rc = FT_8BIT;
	val = c1controls ? 1 : 0;
    } else {				/* 8-bit binary */
	debug(F100,"scanfile 8-bit BIN (c0noniso)","",0);
	rc = FT_BIN;
    }

#else  /* !UNICODE */

    if (c0noniso) {
	debug(F100,"scanfile 8-bit BIN (c0noniso)","",0);
	rc = FT_BIN;
    } else if (eightbit) {
	debug(F100,"scanfile 8-bit TEXT (no c0noniso)","",0);
	rc = FT_8BIT;
	val = c1controls ? 1 : 0;
    } else {
	debug(F100,"scanfile 7-bit TEXT (no c0noniso)","",0);
	rc = FT_7BIT;
    }

#endif /* UNICODE */

  xscanfile:
    if (flag) *flag = val;
    debug(F101,"scanfile result     ","",rc);
    return(rc);
}

/*
  scanstring - like scan file but for a string.
  This is just a quick butchery of scanfile without thinking too much.
*/
int
scanstring(s) char * s; {
    int x, val = -1, count = 0;		/* Workers */
    int rc = -1;			/* Return code */
    int pv = -1;			/* Pattern-match value */
    int bytes = 0;			/* Total byte count */
#ifdef UNICODE
    unsigned int c0, c1;		/* First 2 file bytes (for BOM) */
#endif /* UNICODE */
    extern int pipesend, filepeek;

    register int i;			/* Loop control */
    int readsize = 0;			/* How much to read */
    int eightbit = 0;			/* Number of bytes with 8th bit on */
    int c0controls = 0;			/* C0 non-text control-char counter */
    int c0noniso = 0;			/* C0 non-ISO control-char counter */
    int c1controls = 0;			/* C1 control-character counter */
    unsigned int c;			/* Current character */
    int runmax = 0;			/* Longest run of 0 bytes */
    int runzero = 0;			/* Run of 0 bytes */
    int pctzero = 0;			/* Percentage of 0 bytes */
    int txtcz = 0;

#ifdef UNICODE
    int notutf8 = 0;			/* Nonzero if definitely not UTF-8 */
    int utf8state = 0;			/* UTF-8 recognizer state */
    int oddzero = 0;			/* Number of 0 bytes in odd postions */
    int evenzero = 0;			/* and in even positions */
    int lfnul = 0;			/* Number of <LF><NUL> sequences */
    int crlf = 0;			/* Number of <CRLF> sequences */
#else
    int notutf8 = 1;
#endif /* UNICODE */

    char * buf = s;
    if (!s) s = "";
    count = strlen(s);

#ifdef UNICODE
    if (bytes == 0 && count > 1) {
	int incl_cnt = 0;

	/* First look for BOM */

	c0 = (unsigned)((unsigned)buf[0]&0xFF); /* First file byte */
	c1 = (unsigned)((unsigned)buf[1]&0xFF); /* Second byte */

	if (c0 == 0xFE && c1 == 0xFF) {	/* UCS-2 BE */
	    rc = FT_UCS2;
	    val = 0;
	    debug(F111,"scanstring UCS2 BOM BE",ckitoa(val),rc);
	    incl_cnt++;
	} else if (c0 == 0xFF && c1 == 0xFE) { /* UCS-2 LE */
	    rc = FT_UCS2;
	    val = 1;
	    debug(F111,"scanstring UCS2 BOM LE",ckitoa(val),rc);
	    incl_cnt++;
	} else if (count > 2) if (c0 == 0xEF && c1 == 0xBB &&
		   (unsigned)((unsigned)buf[2]&0xFF) == 0xBF) {
	    rc = FT_UTF8;
	    debug(F111,"scanstring UTF8 BOM",ckitoa(val),rc);
	    incl_cnt++;
	}
	if (incl_cnt) {		/* Have BOM */
	    bytes += count;
	    goto xscanstring;
	}
    }
#endif /* UNICODE */

    bytes += count;			/* Count bytes read */

    for (i = 0; i < count; i++) {	/* For each byte... */
	c = (unsigned)buf[i];	/* For ease of reference */
	if (!c) {			/* Zero byte? */
	    goto xscanstring;	/* Null terminated string */
	}
	if ((c & 0x80) == 0) {	/* We have a 7-bit byte */
#ifdef UNICODE
	    if (i > 0 && c == 10) { /* Linefeed */
		if (buf[i-1] == 0) lfnul++; /* Preceded by NUL */
		else if (buf[i-1] == 13) crlf++; /* or by CR... */
	    }
#endif /* UNICODE */
	    if (c < ' ') {		/* Check for CO controls */
		if (c != LF && c != CR && c != HT && c != FF) {
		    c0controls++;
		    if (c != ESC && c != SO && c != SI)
		      c0noniso++;
		}
		if ((c == '\032')	/* Ctrl-Z */
		    ) {
		    c0controls--;
		    c0noniso--;
		}
	    }
#ifdef UNICODE
	    if (!notutf8 && utf8state) { /* In UTF-8 sequence? */
		utf8state = 0;
		debug(F000,"scanstring","7-bit byte in UTF8 sequence",c);
		notutf8++;		/* Then it's not UTF-8 */
		continue;
	    }
#endif /* UNICODE */
	} else {			/* We have an 8-bit byte */
	    eightbit++;		/* Count it */
	    if (c >= 0x80 && c < 0xA0) /* Check for C1 controls */
	      c1controls++;
#ifdef UNICODE
	    if (!notutf8) {		/* If it might still be UTF8... */
		switch (utf8state) { /* Enter the UTF-8 state machine */
		  case 0:		 /* First byte... */
		    if ((c & 0xE0) == 0xC0) { /* Tells number of */
			utf8state = 1;        /* subsequent bytes */
		    } else if ((c & 0xF0) == 0xE0) {
			utf8state = 2;
		    } else if ((c & 0xF8) == 0xF0) {
			utf8state = 3;
		    } else {
			notutf8++;
		    }
		    break;
		  case 1:		/* Subsequent byte */
		  case 2:
		  case 3:
		    if ((c & 0xC0) != 0x80) { /* Must start with 10 */
			debug(F000,"scanstring",
			      "bad byte in UTF8 sequence",c);
			notutf8++;
			break;
		    }
		    utf8state--;	/* Good, one less in this sequence */
		    break;
		  default:		/* Shouldn't happen */
		    debug(F111,"scanstring","bad UTF8 state",utf8state);
		    notutf8++;
		}
	    }
#endif /* UNICODE */
	}
    }
    if (bytes == 0)			/* If nothing was read */
      return(-1);			/* we're done. */

#ifdef UNICODE
    if (bytes > 100)			/* Bytes is not 0 */
      pctzero = (evenzero + oddzero) / (bytes / 100);
    else
      pctzero = ((evenzero + oddzero) * 100) / bytes;
#endif /* UNICODE */

#ifdef UNICODE
    x = eightbit ? bytes / 20 : bytes / 4; /* For UCS-2... */

    if (runmax > 2) {			/* File has run of more than 2 NULs */
	debug(F100,"scanstring BIN runmax","",0);
	rc = FT_BIN;			/* so it can't be any kind of text. */
	goto xscanstring;

    } else if (rc == FT_UCS2 || (rc == FT_UTF8 && runmax == 0)) {
	goto xscanstring;			/* File starts with a BOM */

    } else if (eightbit > 0 && !notutf8) { /* File has 8-bit data */
	if (runmax > 0) {		   /* and runs of NULs */
	    debug(F100,"scanstring BIN (nnUTF8) runmax","",0);
	    rc = FT_BIN;		   /* UTF-8 doesn't have NULs */
	} else {			   /* No NULs */
	    debug(F100,"scanstring UTF8 (nnUTF8 + runmax == 0)","",0);
	    rc = FT_UTF8;		   /* and not not UTF-8, so is UTF-8 */
	}
	goto xscanstring;
    }
/*
  It seems to be UCS-2 but let's be more certain since there is no BOM...
  If the number of 7- and 8-bit characters is approximately equal, it might
  be a compressed file.  In this case we decide based on the name.
*/
    if (rc == FT_UCS2) {
	if (bytes < 100) {
	    if (oddzero != 0 && evenzero != 0) {
		debug(F100,"scanstring small UCS2 doubtful","",0);
		rc = FT_BIN;
		goto xscanstring;
	    } else if (oddzero == 0 && evenzero == 0) {
		rc = eightbit ? FT_8BIT : FT_7BIT;
	    }
	}
	goto xscanstring;			/* Seems to be UCS-2 */
    }

/* If none of the above, it's probably not Unicode.  */

    if (!eightbit) {			/* It's 7-bit */
	if (c0controls) {		/* This would be strange */
	    if ((c0noniso > 0) && (txtcz == 0)) {
		debug(F100,"scanstring 7-bit BIN (c0coniso)","",0);
		rc = FT_BIN;
	    } else {
		debug(F100,"scanstring 7-bit ISO2022 TEXT (no c0noniso)","",0);
		rc = FT_7BIT;
	    }
	} else {			/* 7-bit text */
	    debug(F100,"scanstring 7-bit TEXT (no c0controls)","",0);
	    rc = FT_7BIT;
	}
    } else if (!c0noniso || txtcz) {	/* 8-bit text */
	debug(F100,"scanstring 8-bit TEXT (no c0noniso)","",0);
	rc = FT_8BIT;
	val = c1controls ? 1 : 0;
    } else {				/* 8-bit binary */
	debug(F100,"scanstring 8-bit BIN (c0noniso)","",0);
	rc = FT_BIN;
    }

#else  /* !UNICODE */

    if (c0noniso) {
	debug(F100,"scanstring 8-bit BIN (c0noniso)","",0);
	rc = FT_BIN;
    } else if (eightbit) {
	debug(F100,"scanstring 8-bit TEXT (no c0noniso)","",0);
	rc = FT_8BIT;
	val = c1controls ? 1 : 0;
    } else {
	debug(F100,"scanstring 7-bit TEXT (no c0noniso)","",0);
	rc = FT_7BIT;
    }

#endif /* UNICODE */

  xscanstring:
    debug(F101,"scanstring result     ","",rc);
    return(rc);
}



/*  F I L E S E L E C T  --  Select this file for sending  */

int
#ifdef CK_ANSIC
fileselect(
    char *f, char *sa, char *sb, char *sna, char *snb,
    CK_OFF_T minsiz, CK_OFF_T maxsiz,
    int nbu, int nxlist,
    char ** xlist
)
#else
fileselect(f,sa,sb,sna,snb,minsiz,maxsiz,nbu,nxlist,xlist)
 char *f,*sa,*sb,*sna,*snb; CK_OFF_T minsiz,maxsiz;
 int nbu,nxlist; char ** xlist;
#endif /* CK_ANSIC */
/* fileselect */ {
    char *fdate;
    int n;
    CK_OFF_T z;

    debug(F111,"fileselect minsiz",ckfstoa(minsiz),minsiz);
    debug(F111,"fileselect maxsiz",ckfstoa(maxsiz),maxsiz);
    debug(F111,"fileselect (CK_OFF_T)-1",ckfstoa((CK_OFF_T)-1),(CK_OFF_T)-1);

    if (!sa) sa = "";
    if (!sb) sb = "";
    if (!sna) sna = "";
    if (!snb) snb = "";

#ifdef CKSYMLINK
#ifndef NOICP
#ifndef NOXFER
    if (nolinks) {
	CK_OFF_T zz;
	zz = zgetfs(f);
	debug(F111,"fileselect NOLINKS zgetfs",f,zz);
	if (zz < (CK_OFF_T)0)
	  return(0);
	debug(F111,"fileselect NOLINKS zgfs_link",f,zgfs_link);
	if (zgfs_link)
	  return(0);
    }
#endif /* NOXFER */
#endif /* NOICP */
#endif /* CKSYMLINK */

    debug(F110,"fileselect",f,0);
    if (*sa || *sb || *sna || *snb) {
	fdate = zfcdat(f);		/* Date/time of this file */
	if (!fdate) fdate = "";
	n = strlen(fdate);
	debug(F111,"fileselect fdate",fdate,n);
	if (n != 17)			/* Failed to get it */
	  return(1);
	/* /AFTER: */
	if (sa[0] && (strcmp(fdate,(char *)sa) <= 0)) {
	    debug(F110,"fileselect sa",sa,0);
	    /* tlog(F110,"Skipping (too old)",f,0); */
	    return(0);
	}
	/* /BEFORE: */
	if (sb[0] && (strcmp(fdate,(char *)sb) >= 0)) {
	    debug(F110,"fileselect sb",sb,0);
	    /* tlog(F110,"Skipping (too new)",f,0); */
	    return(0);
	}
	/* /NOT-AFTER: */
	if (sna[0] && (strcmp(fdate,(char *)sna) > 0)) {
	    debug(F110,"fileselect sna",sna,0);
	    /* tlog(F110,"Skipping (too new)",f,0); */
	    return(0);
	}
	/* /NOT-BEFORE: */
	if (snb[0] && (strcmp(fdate,(char *)snb) < 0)) {
	    debug(F110,"fileselect snb",snb,0);
	    /* tlog(F110,"Skipping (too old)",f,0); */
	    return(0);
	}
    }
    /* Smaller or larger */
    if (minsiz > (CK_OFF_T)-1 || maxsiz > (CK_OFF_T)-1) {
	z = zchki(f);			/* Get size */
	debug(F101,"fileselect filesize","",z);
	if (z < (CK_OFF_T)0)
	  return(1);
	if ((minsiz > (CK_OFF_T)-1) && (z >= minsiz)) {
	    debug(F111,"fileselect minsiz skipping",f,minsiz);
	    /* tlog(F111,"Skipping (too big)",f,z); */
	    return(0);
	}
	if ((maxsiz > (CK_OFF_T)-1) && (z <= maxsiz)) {
	    debug(F111,"fileselect maxsiz skipping",f,maxsiz);
	    /* tlog(F110,"Skipping (too small)",f,0); */
	    return(0);
	}
    }
    if (nbu) {				/* Skipping backup files? */
	if (ckmatch(
#ifdef CKREGEX
		    "*.~[0-9]*~"	/* Not perfect but close enough. */
#else
		    "*.~*~"		/* Less close. */
#endif /* CKREGEX */
		    ,f,filecase,1)) {
	    debug(F110,"fileselect skipping backup",f,0);
	    return(0);
	}
    }
    for (n = 0; xlist && n < nxlist; n++) {
	if (!xlist[n]) {
	    debug(F101,"fileselect xlist empty",0,n);
	    break;
	}
	if (ckmatch(xlist[n],f,filecase,1)) {
	    debug(F111,"fileselect xlist",xlist[n],n);
	    debug(F110,"fileselect skipping",f,0);
	    return(0);
	}
    }
    if (xfiletype > -1) {
	n = scanfile(f,NULL,nscanfile);
	if (n < 0) {
	    n = binary ? 1 : 0;
	} else {
	    n = (n == FT_BIN) ? 1 : 0;
	}
	if (n != xfiletype)
	  return(0);
    }
    debug(F110,"fileselect selecting",f,0);
    return(1);
}


#ifdef TCPSOCKET
#ifdef NT
extern int WSASafeToCancel;
#endif /* NT */
#endif /* TCPSOCKET */

VOID
setflow() {
    extern int flow, autoflow, mdmtyp, cxtype, cxflow[];
#ifndef NODIAL
    extern int dialcapas, dialfc;
    extern MDMINF * modemp[];
    MDMINF * p = NULL;
    long bits = 0;
#endif /* NODIAL */

    debug(F101,"setflow autoflow","",autoflow);

/* #ifdef COMMENT */
/* WHY WAS THIS COMMENTED OUT? */
    if (!autoflow)                      /* Only if FLOW is AUTO */
      return;
/* #endif */ /* COMMENT */

    debug(F101,"setflow local","",local);
    debug(F101,"setflow network","",network);
    debug(F101,"setflow cxtype","",cxtype);

#ifdef TN_COMPORT
    if (network && istncomport()) {
	flow = cxflow[CXT_MODEM];
        debug(F101,"setflow TN_COMPORT flow","",flow);
        return;
    }
#endif /* TN_COMPORT */

    if (network || !local || cxtype == CXT_DIRECT) {
        flow = cxflow[cxtype];          /* Set appropriate flow control */
        debug(F101,"setflow flow","",flow);
        return;
    }
    if (cxtype != CXT_MODEM)            /* Connection type should be modem */
      return;

#ifndef NODIAL
    bits = dialcapas;                   /* Capability bits */
    if (!bits) {                        /* No bits? */
        p = modemp[mdmtyp];             /* Look in modem info structure */
        if (p)
          bits = p->capas;
    }
    if (dialfc == FLO_AUTO) {           /* If DIAL flow is AUTO */
#ifdef CK_RTSCTS                        /* If we can do RTS/CTS flow control */
        if (bits & CKD_HW)              /* and modem can do it too */
          flow = FLO_RTSC;              /* then switch to RTS/CTS */
        else                            /* otherwise */
          flow = FLO_XONX;              /* use Xon/Xoff. */
#else
#ifndef NEXT
#ifndef IRIX
        flow = FLO_XONX;                /* Use Xon/Xoff. */
#endif /* IRIX */
#endif /* NEXT */
#endif /* CK_RTSCTS */
    }
#endif /* NODIAL */
    debug(F101,"setflow modem flow","",flow);
    return;
}

#ifndef NOLOCAL
#ifdef CK_TRIGGER

/*  A U T O E X I T C H K  --  Check for CONNECT-mode trigger string  */
/*
  Returns -1 if trigger not found, or else the trigger index, 0 or greater.
  (Replace with fancier and more efficient matcher later...)
  NOTE: to prevent unnecessary function call overhead, call this way:

    x = tt_trigger[0] ? autoexitchk(c) : -1;

*/
int
#ifdef CK_ANSIC
autoexitchk(CHAR c)
#else
autoexitchk(c) CHAR c;
#endif /* CK_ANSIC */
/* autoexitchk */ {
    extern CHAR * tt_trmatch[];
    extern char * tt_trigger[];
    int i;
    for (i = 0; i < TRIGGERS; i++) {
        if (!tt_trigger[i]) {           /* No more triggers in list */
            break;
        } else if (*tt_trigger[i]) {
            if (!tt_trmatch[i])         /* Just starting? */
              tt_trmatch[i] = (CHAR *)tt_trigger[i]; /* Set match pointer */
            if (c == *tt_trmatch[i]) {  /* Compare this character */
                tt_trmatch[i]++;        /* It matches */
                if (!*tt_trmatch[i]) {  /* End of match string? */
                    tt_trmatch[i] = (CHAR *) tt_trigger[i]; /* Yes, rewind, */
                    debug(F101,"autoexitchk",tt_trigger[i],i); /* log, */
                    return(i);          /* and return success */
                }
            } else                      /* No match */
              tt_trmatch[i] = (CHAR *) tt_trigger[i]; /* Rewind match string */
        } /* and go on the next match string */
    }
    return(-1);                         /* No match found */
}
#endif /* CK_TRIGGER */

#ifndef NOSHOW
/*  S H O M D M  --  Show modem signals  */

VOID
shomdm() {
/*
  Note use of "\r\n" to make sure this report prints right, even when
  called during CONNECT mode.
*/
    int y;
    y = ttgmdm();
    switch (y) {
      case -3: printf(
                 "Modem signals unavailable in this version of Kermit\r\n");
               break;
      case -2: printf("No modem control for this device\r\n"); break;
      case -1: printf("Modem signals unavailable\r\n"); break;
      default:
#ifndef MAC
        printf(
          " Carrier Detect      (CD):  %s\r\n",(y & BM_DCD) ? "On": "Off");
        printf(
          " Dataset Ready       (DSR): %s\r\n",(y & BM_DSR) ? "On": "Off");
#endif /* MAC */
        printf(
          " Clear To Send       (CTS): %s\r\n",(y & BM_CTS) ? "On": "Off");
#ifndef STRATUS
#ifndef MAC
        printf(
          " Ring Indicator      (RI):  %s\r\n",(y & BM_RNG) ? "On": "Off");
#endif /* MAC */
        printf(
          " Data Terminal Ready (DTR): %s\r\n",
#ifdef NT
          "(unknown)"
#else /* NT */
          (y & BM_DTR) ? "On": "Off"
#endif /* NT */
          );
#ifndef MAC
        printf(
          " Request To Send     (RTS): %s\r\n",
#ifdef NT
          "(unknown)"
#else /* NT */
          (y & BM_RTS) ? "On": "Off"
#endif /* NT */
          );
#endif /* MAC */
#endif /* STRATUS */
    }
#ifdef BETADEBUG
#ifdef CK_TAPI
    if (tttapi && !tapipass) {
        LPDEVCFG        lpDevCfg = NULL;
        LPCOMMCONFIG    lpCommConfig = NULL;
        LPMODEMSETTINGS lpModemSettings = NULL;
        DCB *           lpDCB = NULL;

        if (cktapiGetModemSettings(&lpDevCfg,&lpModemSettings,
                                    &lpCommConfig,&lpDCB)) {
            printf("\n");
            cktapiDisplayModemSettings(lpDevCfg,lpModemSettings,
                                       lpCommConfig,lpDCB);
        }
    }
#endif /* CK_TAPI */
#endif /* BETADEBUG */
}
#endif /* NOSHOW */
#endif /* NOLOCAL */

#ifndef NOXFER
/*  S D E B U  -- Record spar results in debugging log  */

VOID
sdebu(len) int len; {
    debug(F111,"spar: data",(char *) rdatap,len);
    debug(F101," spsiz ","", spsiz);
    debug(F101," timint","",timint);
    debug(F101," npad  ","",  npad);
    debug(F101," padch ","", padch);
    debug(F101," seol  ","",  seol);
    debug(F101," ctlq  ","",  ctlq);
    debug(F101," ebq   ","",   ebq);
    debug(F101," ebqflg","",ebqflg);
    debug(F101," bctr  ","",  bctr);
    debug(F101," rptq  ","",  rptq);
    debug(F101," rptflg","",rptflg);
    debug(F101," lscapu","",lscapu);
    debug(F101," atcapu","",atcapu);
    debug(F101," lpcapu","",lpcapu);
    debug(F101," swcapu","",swcapu);
    debug(F101," wslotn","", wslotn);
    debug(F101," whatru","", whatru);
}
/*  R D E B U -- Debugging display of rpar() values  */

VOID
rdebu(d,len) CHAR *d; int len; {
    debug(F111,"rpar: data",d,len);
    debug(F101," rpsiz ","", xunchar(d[0]));
    debug(F101," rtimo ","", rtimo);
    debug(F101," mypadn","",mypadn);
    debug(F101," mypadc","",mypadc);
    debug(F101," eol   ","",   eol);
    debug(F101," ctlq  ","",  ctlq);
    debug(F101," sq    ","",    sq);
    debug(F101," ebq   ","",   ebq);
    debug(F101," ebqflg","",ebqflg);
    debug(F101," bctr  ","",  bctr);
    debug(F101," rptq  ","",  d[8]);
    debug(F101," rptflg","",rptflg);
    debug(F101," capas ","", capas);
    debug(F101," bits  ","",d[capas]);
    debug(F101," lscapu","",lscapu);
    debug(F101," atcapu","",atcapu);
    debug(F101," lpcapu","",lpcapu);
    debug(F101," swcapu","",swcapu);
    debug(F101," wslotr","", wslotr);
    debug(F101," rpsiz(extended)","",rpsiz);
}

#ifdef COMMENT
/*  C H K E R R  --  Decide whether to exit upon a protocol error  */

VOID
chkerr() {
    if (backgrd && !server) fatal("Protocol error");
}
#endif /* COMMENT */
#endif /* NOXFER */

/*  F A T A L  --  Fatal error message */

VOID
fatal(msg) char *msg; {
    extern int initflg;
    static int initing = 0;
    if (!msg) msg = "";
    debug(F111,"fatal",msg,initflg);

    if (!initflg) {			/* If called from prescan */
	if (initing)			/* or called from sysinit() */
          exit(253);
	initing = 1;
	sysinit();
    }

    debug(F111,"fatal",msg,xitsta);
    tlog(F110,"Fatal:",msg,0L);
#ifdef VMS
    if (strncmp(msg,"%CKERMIT",8))
      conol("%CKERMIT-E-FATAL, ");
    conoll(msg);
#else /* !VMS */
    conoll(msg);
#endif /* VMS */
#ifdef OS2
#ifndef NOXFER
    if (xfrbel) {
        bleep(BP_FAIL);
        sleep(1);
        bleep(BP_FAIL);
    }
#endif /* NOXFER */

#endif /* OS2 */
    doexit(BAD_EXIT,xitsta | 1);        /* Exit indicating failure */
}

#ifndef NOXFER
/*  B L D L E N  --  Make length-encoded copy of string  */

char *
bldlen(str,dest) char *str, *dest; {
    int len;
    len = (int)strlen(str);
    if (len > 94)
      *dest = SP;
    else
      *dest = (char) tochar(len);
    strcpy(dest+1,str);			/* Checked below in setgen() */
    return(dest+len+1);
}


/*  S E T G E N  --  Construct a generic command  */
/*
  Call with Generic command character followed by three string arguments.
  Trailing strings are allowed to be empty ("").  Each string except the last
  non-empty string must be less than 95 characters long.  The final nonempty
  string is allowed to be longer.
*/
CHAR
#ifdef CK_ANSIC
setgen(char type, char * arg1, char * arg2, char * arg3)
#else
setgen(type,arg1,arg2,arg3) char type, *arg1, *arg2, *arg3;
#endif /* CK_ANSIC */
/* setgen */ {
    char *upstr, *cp;
#ifdef DYNAMIC
    if (!cmdstr)
      if (!(cmdstr = malloc(MAXSP + 1)))
        fatal("setgen: can't allocate memory");
#endif /* DYNAMIC */

    cp = cmdstr;
    *cp++ = type;
    *cp = NUL;
    if (!arg1) arg1 = "";
    if (!arg2) arg2 = "";
    if (!arg3) arg3 = "";
    if (((int)strlen(arg1)+(int)strlen(arg2)+(int)strlen(arg3)+4) < MAXSP) {
	if (*arg1 != NUL) {
	    upstr = bldlen(arg1,cp);
	    if (*arg2 != NUL) {
		upstr = bldlen(arg2,upstr);
		if (*arg3 != NUL) bldlen(arg3,upstr);
	    }
	}
	cmarg = cmdstr;
	debug(F110,"setgen",cmarg,0);
	return('g');
    }
    return('E');
}
#endif /* NOXFER */

#ifndef NOMSEND
static char *mgbufp = NULL;

/*  F N P A R S E  --  */

/*
  Argument is a character string containing one or more filespecs.
  This function breaks the string apart into an array of pointers, one
  to each filespec, and returns the number of filespecs.  Used by server
  when it receives a GET command to allow it to process multiple file
  specifications in one transaction.  Sets cmlist to point to a list of
  file pointers, exactly as if they were command line arguments.

  This version of fnparse treats spaces as filename separators.  If your
  operating system allows spaces in filenames, you'll need a different
  separator.

  This version of fnparse mallocs a string buffer to contain the names.  It
  cannot assume that the string that is pointed to by the argument is safe.
*/
int
fnparse(string) char *string; {
    char *p, *s, *q;
    int r = 0, x;                       /* Return code */
#ifdef RECURSIVE
    debug(F111,"fnparse",string,recursive);
#endif /* RECURSIVE */

    if (mgbufp) free(mgbufp);           /* Free this from last time. */
    mgbufp = malloc((int)strlen(string)+2);
    if (!mgbufp) {
        debug(F100,"fnparse malloc error","",0);
        return(0);
    }
#ifndef NOICP
#ifndef NOSPL
    ckstrncpy(fspec,string,fspeclen);   /* Make copy for \v(filespec) */
#endif /* NOSPL */
#endif /* NOICP */
    s = string;                         /* Input string */
    p = q = mgbufp;                     /* Point to the copy */
    r = 0;                              /* Initialize our return code */
    while (*s == SP || *s == HT)        /* Skip leading spaces and tabs */
      s++;
    for (x = strlen(s);                 /* Strip trailing spaces */
         (x > 1) && (s[x-1] == SP || s[x-1] == HT);
         x--)
      s[x-1] = NUL;
    while (1) {                         /* Loop through rest of string */
        if (*s == CMDQ) {               /* Backslash (quote character)? */
            if ((x = xxesc(&s)) > -1) { /* Go interpret it. */
                *q++ = (char) x;        /* Numeric backslash code, ok */
            } else {                    /* Just let it quote next char */
                s++;                    /* get past the backslash */
                *q++ = *s++;            /* deposit next char */
            }
            continue;
        } else if (*s == SP || *s == NUL) { /* Unquoted space or NUL? */
            *q++ = NUL;                 /* End of output filename. */
            msfiles[r] = p;             /* Add this filename to the list */
            debug(F111,"fnparse",msfiles[r],r);
            r++;                        /* Count it */
            if (*s == NUL) break;       /* End of string? */
            while (*s == SP) s++;       /* Skip repeated spaces */
            p = q;                      /* Start of next name */
            continue;
        } else *q++ = *s;               /* Otherwise copy the character */
        s++;                            /* Next input character */
    }
    debug(F101,"fnparse r","",r);
    msfiles[r] = "";                    /* Put empty string at end of list */
    cmlist = msfiles;
    return(r);
}
#endif /* NOMSEND */

char *                                  /* dbchr() for DEBUG SESSION */
dbchr(c) int c; {
    static char s[8];
    char *cp = s;

    c &= 0xff;
    if (c & 0x80) {                     /* 8th bit on */
        *cp++ = '~';
        c &= 0x7f;
    }
    if (c < SP) {                       /* Control character */
        *cp++ = '^';
        *cp++ = (char) ctl(c);
    } else if (c == DEL) {
        *cp++ = '^';
        *cp++ = '?';
    } else {                            /* Printing character */
        *cp++ = (char) c;
    }
    *cp = '\0';                         /* Terminate string */
    cp = s;                             /* Return pointer to it */
    return(cp);
}

/*  C K H O S T  --  Get name of local host (where C-Kermit is running)  */

/*
  Call with pointer to buffer to put hostname in, and length of buffer.
  Copies hostname into buffer on success, puts null string in buffer on
  failure.
*/
#ifdef BSD44
#define BSD4
#undef ATTSV
#endif /* BSD44 */

#ifdef SVORPOSIX
#ifndef BSD44
#ifndef apollo
#include <sys/utsname.h>
#endif /* apollo */
#endif /* BSD44 */
#else
#ifdef BELLV10
#include <utsname.h>
#endif /* BELLV10 */
#endif /* SVORPOSIX*/

#ifdef CKSYSLOG
extern char uidbuf[], * clienthost;
#endif /* CKSYSLOG */

VOID
ckhost(vvbuf,vvlen) char * vvbuf; int vvlen; {

#ifndef NOPUSH
    extern int nopush;
#ifndef NOSERVER
    extern int en_hos;
#endif /* NOSERVER */
#endif /* NOPUSH */

#ifdef pdp11
    *vvbuf = NUL;
#else  /* Everything else - rest of this routine */

    char *g;
    int havefull = 0;
#ifdef VMS
    int x;
#endif /* VMS */

#ifdef SVORPOSIX
#ifndef BSD44
#ifndef _386BSD
#ifndef APOLLOSR10
    struct utsname hname;
#endif /* APOLLOSR10 */
#endif /* _386BSD */
#endif /* BSD44 */
#endif /* SVORPOSIX */
#ifdef datageneral
    int ac0 = (char *) vvbuf, ac1 = -1, ac2 = 0;
#endif /* datageneral */

#ifndef NOPUSH
    if (getenv("CK_NOPUSH")) {          /* No shell access allowed */
        nopush = 1;                     /* on this host... */
#ifndef NOSERVER
        en_hos = 0;
#endif /* NOSERVER */
    }
#endif /* NOPUSH */

    *vvbuf = NUL;                       /* How let's get our host name ... */

#ifndef BELLV10                         /* Does not have gethostname() */
#ifndef OXOS
#ifdef SVORPOSIX
#ifdef APOLLOSR10
    ckstrncpy(vvbuf,"Apollo",vvlen);
#else
#ifdef BSD44
    if (gethostname(vvbuf,vvlen) < 0)
      *vvbuf = NUL;
#else
#ifdef _386BSD
    if (gethostname(vvbuf,vvlen) < 0) *vvbuf = NUL;
#else
#ifdef QNX
#ifdef TCPSOCKET
    if (gethostname(vvbuf,vvlen) < 0) *vvbuf = NUL;
#else
    if (uname(&hname) > -1) ckstrncpy(vvbuf,hname.nodename,vvlen);
#endif /* TCPSOCKET */
#else /* SVORPOSIX but not _386BSD or BSD44 */
#ifdef __ia64__
    if (uname(&hname) > -1) ckstrncpy(vvbuf,hname.nodename,vvlen);
#else
    if (uname(&hname) > -1) {
	char * p;
	p = hname.nodename;
#ifdef TCPSOCKET
#ifndef NOCKGETFQHOST
	if (!ckstrchr(p,'.'))
	  p = (char *)ckgetfqhostname(p);
#endif /* NOCKGETFQHOST */
#endif /* TCPSOCKET */
	if (!p) p = "";
	if (!*p) p = "(unknown)";
	ckstrncpy(vvbuf,p,vvlen);
    }
#endif /* __ia64__ */
#endif /* QNX */
#endif /* _386BSD */
#endif /* BSD44 */
#endif /* APOLLOSR10 */
#else /* !SVORPOSIX */
#ifdef BSD4
    if (gethostname(vvbuf,vvlen) < 0) *vvbuf = NUL;
#else /* !BSD4 */
#ifdef VMS
    g = getenv("SYS$NODE");
    if (g) ckstrncpy(vvbuf,g,vvlen);
    x = (int)strlen(vvbuf);
    if (x > 1 && vvbuf[x-1] == ':' && vvbuf[x-2] == ':') vvbuf[x-2] = NUL;
#else
#ifdef datageneral
    if (sys($HNAME,&ac0,&ac1,&ac2) == 0) /* successful */
        vvlen = ac2 + 1;                /* enh - have to add one */
#else
#ifdef OS2                              /* OS/2 */
    g = os2_gethostname();
    if (g) ckstrncpy(vvbuf,g,vvlen);
#else /* OS2 */
#ifdef OSK
#ifdef TCPSOCKET
        if (gethostname(vvbuf, vvlen) < 0) *vvbuf = NUL;
#endif /* TCPSOCKET */
#endif /* OSK */
#endif /* OS2 */
#endif /* datageneral */
#endif /* VMS */
#endif /* BSD4 */
#endif /* SVORPOSIX */
#else /* OXOS */
    /* If TCP/IP is not installed, gethostname() fails, use uname() */
    if (gethostname(vvbuf,vvlen) < 0) {
        if (uname(&hname) > -1)
            ckstrncpy(vvbuf,hname.nodename,vvlen);
        else
            *vvbuf = NUL;
    }
#endif /* OXOS */
#endif /* BELLV10 */
    if (*vvbuf == NUL) {                /* If it's still empty */
        g = getenv("HOST");             /* try this */
        if (g) ckstrncpy(vvbuf,g,vvlen);
    }
    vvbuf[vvlen-1] = NUL;               /* Make sure result is terminated. */
#endif /* pdp11 */
}
#ifdef BSD44
#undef BSD4
#define ATTSV
#endif /* BSD44 */

/*
  A S K M O R E  --  Poor person's "more".
  Returns 0 if no more, 1 if more wanted.
*/
int
askmore() {
    char c;
    int rv, cx;
#ifdef IKSD
    extern int timelimit;
#endif /* IKSD */
#ifdef IKSDCONF
    extern int iksdcf;
#endif /* IKSDCONF */
#ifdef CK_APC
    extern int apcstatus, apcactive;
#endif /* CK_APC */

#ifdef NOICP
    return(1);
#else
    if (!xaskmore)
      return(1);
#ifdef IKSDCONF
    if (inserver && !iksdcf)
      return(1);
#endif /* IKSDCONF */
#ifdef CK_APC
    if (apcactive == APC_LOCAL ||
        (apcactive == APC_REMOTE && (apcstatus & APC_NOINP)))
        return(1);
#endif /* CK_APC */
#ifdef VMS
    if (batch)
      return(1);
#else
#ifdef UNIX
    if (backgrd)
      return(1);
#endif /* UNIX */
#endif /* VMS */

#ifndef VMS
    concb((char)escape);                /* Force CBREAK mode. */
#endif /* VMS */

    rv = -1;
    while (rv < 0) {
#ifndef OS2
        printf("more? ");
#ifdef UNIX
#ifdef NOSETBUF
        fflush(stdout);
#endif /* NOSETBUF */
#endif /* UNIX */
#else
        printf("more? ");
        fflush(stdout);
#endif /* OS2 */

#ifdef IKSD
        if (inserver) {
            cx = cmdgetc(timelimit);
            if (cx < -1 && timelimit) {
                printf("\n?IKS idle timeout - Goodbye.\n");
                doexit(GOOD_EXIT,0);
            } else if (cx == -1) {	/* Connection lost */
                doexit(BAD_EXIT,0);
            }
            c = (char) cx;
        } else {
#endif /* IKSD */
#ifdef VMS
	    conbin((char)escape);	/* Protect against Ctrl-Z */
	    cx = coninc(0);
	    concb((char)escape);
#else
	    cx = cmdgetc(0);
#endif /* VMS */
	    debug(F101,"askmore cmdgetc","",cx);
	    if (cx == EOF) {
		debug(F100,"askmore EOF","",0);
#ifdef VMS
		c = '\032';
#else
		c = 'n';
#endif /* VMS */
	    } else {
		c = (char)cx;
	    }
	    debug(F101,"askmore c","",c);

#ifdef IKSD
	}
#endif /* IKSD */
        switch (c) {
          /* Yes */
	  case 'p': case 'P': case 'g': case 'G': /* Proceed or Go */
	    xaskmore = 0;
	    /* fall thru on purpose */

          case SP: case 'y': case 'Y': case 012:  case 015:
#ifdef OSK
            write(1, "\015      \015", sizeof "\015      \015" - 1);
#else
            printf("\015      \015");
#endif /* OSK */
            rv = 1;
            break;
          /* No */
          case 'n': case 'N': case 'q': case 'Q':
#ifdef OSK
            printf("\n");
#else
            printf("\015\012");
#endif /* OSK */
            rv = 0;
            break;
	  case '\003':
	  case '\004':
	  case '\032':
#ifdef OSK
	    printf("^%c...\n", (c + 0100));
#else
	    printf("^%c...\015\012", (c + 0100));
#endif /* OSK */
	    rv = 0;
	    break;
          /* Invalid answer */
          default:
            debug(F111,"askmore","invalid answer",c);
            printf("Y or space-bar for yes, N for no, G to show the rest\n");
            continue;
        }
#ifdef OS2
        printf("\r                                                   \r");
        fflush(stdout);
#endif /* OS2 */
    }
    return(rv);
#endif /* NOICP */
}

/*  T R A P  --  Terminal interrupt handler */

SIGTYP
#ifdef CK_ANSIC
trap(int sig)
#else
trap(sig) int sig;
#endif /* CK_ANSIC */
/* trap */ {
    extern int b_save, f_save;
#ifndef NOICP
    extern int timelimit;
#endif /* NOICP */
#ifdef OS2
    extern unsigned long startflags;
#ifndef NOSETKEY
    extern int os2gks;
#endif /* NOSETKEY */
    int i;
#endif /* OS2 */
#ifndef NOSPL
    extern int i_active, instatus;
#endif /* NOSPL */
#ifdef VMS
    int i; FILE *f;
#endif /* VMS */
    extern int zchkod, zchkid;
#ifndef NOSPL
    extern int unkmacro;
#endif /* NOSPL */

    debok = 1;
#ifdef NTSIG
    connoi();
#endif /* NTSIG */
#ifdef __EMX__
    signal(SIGINT, SIG_ACK);
#endif
#ifdef GEMDOS
/* GEM is not reentrant, no i/o from interrupt level */
    cklongjmp(cmjbuf,1);                /* Jump back to parser now! */
#endif /* GEMDOS */

#ifdef DEBUG
    if (deblog) {
	debug(F100,"*********************","",0);
	if (sig == SIGINT)
	  debug(F101,"trap caught SIGINT","",sig);
	else 
	  debug(F101,"trap caught signal","",sig);
	debug(F100,"*********************","",0);
    }
#endif /* DEBUG */

#ifdef OS2
    if ( sig == SIGBREAK && (startflags & 128) ) {
        debug(F101,"trap ignoring SIGBREAK","",sig);
        return;
    }
#endif /* OS2 */

#ifndef NOICP
    timelimit = 0;                      /* In case timed ASK interrupted */
#ifndef NOSPL
    unkmacro = 0;			/* Or ON_UNKNOWN_MACRO interrupted.. */
#endif /* NOSPL */
#endif /* NOICP */
    zchkod = 0;                         /* Or file expansion interrupted... */
    zchkid = 0;
    interrupted = 1;

    if (what & W_CONNECT) {		/* Are we in CONNECT mode? */
/*
  The HP workstation Reset key sends some kind of ueber-SIGINT that can not
  be SIG_IGNored, so we wind up here somehow (even though this is *not* the
  current SIGINT handler).  Just return.
*/
        debug(F101,"trap: SIGINT caught during CONNECT","",sig);
        SIGRETURN;
    }
#ifndef NOSPL
    if (i_active) {                     /* INPUT command was active? */
        i_active = 0;                   /* Not any more... */
        instatus = INP_UI;              /* INPUT status = User Interrupted */
    }
#endif /* NOSPL */

#ifndef NOXFER
    ftreset();                          /* Restore global protocol settings */
    binary = b_save;                    /* Then restore these */
    fncnv  = f_save;
    bye_active = 0;
    diractive = 0;
    cdactive = 0;
#endif /* NOXFER */
    zclose(ZIFILE);                     /* If we were transferring a file, */
    zclose(ZOFILE);                     /* close it. */
#ifndef NOICP
    cmdsquo(cmd_quoting);               /* If command quoting was turned off */
#ifdef CKLEARN
    {
	extern FILE * learnfp;
	extern int learning;
	if (learnfp) {
	    fclose(learnfp);
	    learnfp = NULL;
	    learning = 0;
	}
    }
#endif /* CKLEARN */
#endif /* NOICP */
#ifdef CK_APC
    delmac("_apc_commands",1);
    apcactive = APC_INACTIVE;
#endif /* CK_APC */

#ifdef VMS
/*
  Fix terminal.
*/
    if (ft_win) {                       /* If curses window open */
        debug(F100,"^C trap() curses","",0);
        xxscreen(SCR_CW,0,0L,"");       /* Close it */
        conres();                       /* Restore terminal */
        i = printf("^C...");            /* Echo ^C to standard output */
    } else {
        conres();
        i = printf("^C...\n");          /* Echo ^C to standard output */
    }
    if (i < 1 && ferror(stdout)) {      /* If there was an error */
        debug(F100,"^C trap() error","",0);
        fclose(stdout);                 /* close standard output */
        f = fopen(dftty, "w");          /* open the controlling terminal */
        if (f) stdout = f;              /* and make it standard output */
        printf("^C...\n");              /* and echo the ^C again. */
    }
#else                                   /* Not VMS */
#ifdef STRATUS
    conres();                           /* Set console back to normal mode */
#endif /* STRATUS */
#ifndef NOXFER
    if (ft_win) {                       /* If curses window open, */
        debug(F100,"^C trap() curses","",0);
        xxscreen(SCR_CW,0,0L,"");	/* close it. */
        printf("^C...");                /* Echo ^C to standard output */
    } else {
#endif /* NOXFER */
        printf("^C...\n");
#ifndef NOXFER
    }
#endif /* NOXFER */
#endif /* VMS */
#ifdef datageneral
    connoi_mt();                        /* Kill asynch task that listens to */
    ttimoff();
    conres();                           /* the keyboard */
#endif /* datageneral */

#ifndef NOCCTRAP
/*  This is stupid -- every version should have ttimoff()...  */
#ifdef UNIX
    ttimoff();                          /* Turn off any timer interrupts */
#else
#ifdef OSK
    ttimoff();                          /* Turn off any timer interrupts */
#else
#ifdef STRATUS
    ttimoff();                          /* Turn off any timer interrupts */
#else
#ifdef OS2
#ifndef NOSETKEY
    os2gks = 1;                         /* Turn back on keycode mapping  */
#endif /* NOSETKEY */
#ifndef NOLOCAL
    for (i = 0; i < VNUM; i++)
      VscrnResetPopup(i);
#endif /* NOLOCAL */
#ifdef TCPSOCKET
#ifdef NT
    /* WSAIsBlocking() returns FALSE in Win95 during a blocking accept call */
    if ( WSASafeToCancel /* && WSAIsBlocking() */ ) {
        WSACancelBlockingCall();
    }
#endif /* NT */
#endif /* TCPSOCKET */
#ifdef CK_NETBIOS
    NCBCancelOutstanding();
#endif /* CK_NETBIOS */
    ttimoff();                          /* Turn off any timer interrupts */
#else
#ifdef VMS
    ttimoff();                          /* Turn off any timer interrupts */
#endif /* VMS */
#endif /* OS2 */
#endif /* STRATUS */
#endif /* OSK */
#endif /* UNIX */

#ifdef NETPTY
    /* Clean up Ctrl-C out of REDIRECT or external protocol */
    {
	extern PID_T pty_fork_pid;
	extern int pty_master_fd, pty_slave_fd;
	int x;

	signal(SIGCHLD,SIG_IGN);	/* We don't want this any more */

	debug(F101,"trap pty_master_fd","",pty_master_fd);
	if (pty_master_fd > 2) {
	    x = close(pty_master_fd);
	    debug(F101,"trap pty_master_fd close","",x);
	}
	pty_master_fd = -1;
	debug(F101,"trap pty_slave_fd","",pty_slave_fd);
	if (pty_slave_fd > 2) {
	    x = close(pty_slave_fd);
	    debug(F101,"trap pty_slave_fd close","",x);
	}
	pty_slave_fd = -1;
	debug(F101,"trap pty_fork_pid","",pty_fork_pid);
	if (pty_fork_pid > 0) {
	    x = kill(pty_fork_pid,0);	/* See if the fork is really there */
	    debug(F111,"trap pty_fork_pid kill 0 errno",ckitoa(x),errno);
	    if (x == 0) {		/* Seems to be active */
		x = kill(pty_fork_pid,SIGHUP); /* Ask it to clean up & exit */
		debug(F101,"trap pty_fork_pid kill SIGHUP","",x);
		msleep(100);
		errno = 0;
		x = kill(pty_fork_pid,0); /* Is it still there? */
		if (x == 0
#ifdef ESRCH
		    /* This module is not always exposed to <errno.h> */
		    || errno != ESRCH
#endif	/* ESRCH */
		    ) {
		    x = kill(pty_fork_pid,SIGKILL);
		    debug(F101,"trap pty_fork_pid kill SIGKILL","",x);
		}
	    }
	    pty_fork_pid = -1;
	}
    }
#endif	/* NETPTY */

#ifdef OSK
    sigmask(-1);
/*
  We are in an intercept routine but do not perform a F$RTE (done implicitly
  by rts).  We have to decrement the sigmask as F$RTE does.  Warning: longjump
  only restores the cpu registers, NOT the fpu registers.  So don't use fpu at
  all or at least don't use common fpu (double or float) register variables.
*/
#endif /* OSK */

#ifdef NTSIG
    PostCtrlCSem();
#else /* NTSIG */
    debug(F100,"trap about to longjmp","",0);
#ifdef NT
    cklongjmp(ckjaddr(cmjbuf),1);
#else /* NT */
    cklongjmp(cmjbuf,1);
#endif /* NT */
#endif /* NTSIG */
#else /* NOCCTRAP */
/* No Ctrl-C trap, just exit. */
#ifdef CK_CURSES                        /* Curses support? */
    xxscreen(SCR_CW,0,0L,"");           /* Close curses window */
#endif /* CK_CURSES */
    doexit(BAD_EXIT,what);              /* Exit poorly */
#endif /* NOCCTRAP */
    SIGRETURN;
}


/*  C K _ T I M E  -- Returns pointer to current time. */

char *
ck_time() {
    static char tbuf[10];
    char *p;
    int x;

    ztime(&p);                          /* "Thu Feb  8 12:00:00 1990" */
    if (!p)                             /* like asctime()! */
      return("");
    if (*p) {
        for (x = 11; x < 19; x++)       /* copy hh:mm:ss */
          tbuf[x - 11] = p[x];          /* to tbuf */
        tbuf[8] = NUL;                  /* terminate */
    }
    return(tbuf);                       /* and return it */
}

/*  C C _ C L E A N  --  Cleanup after terminal interrupt handler */

#ifdef GEMDOS
int
cc_clean() {
    zclose(ZIFILE);                     /* If we were transferring a file, */
    zclose(ZOFILE);                     /* close it. */
    printf("^C...\n");                  /* Not VMS, no problem... */
}
#endif /* GEMDOS */


/*  S T P T R A P -- Handle SIGTSTP (suspend) signals */

SIGTYP
#ifdef CK_ANSIC
stptrap(int sig)
#else
stptrap(sig) int sig;
#endif /* CK_ANSIC */
/* stptrap */ {

#ifndef NOJC
    int x; extern int cmflgs;
    debug(F101,"stptrap() caught signal","",sig);
    if (!xsuspend) {
        printf("\r\nsuspend disabled\r\n");
#ifndef NOICP
        if (what & W_COMMAND) {		/* If we were parsing commands */
            prompt(xxstring);           /* reissue the prompt and partial */
            if (!cmflgs)                /* command (if any) */
              printf("%s",cmdbuf);
        }
#endif /* NOICP */
    } else {
        conres();                       /* Reset the console */
#ifndef OS2
        /* Flush pending output first, in case we are continued */
        /* in the background, which could make us block */
        fflush(stdout);

        x = psuspend(xsuspend);		/* Try to suspend. */
        if (x < 0)
#endif /* OS2 */
          printf("Job control not supported\r\n");
        conint(trap,stptrap);           /* Rearm the trap. */
        debug(F100,"stptrap back from suspend","",0);
        switch (what) {
          case W_CONNECT:               /* If suspended during CONNECT? */
            conbin((char)escape);       /* put console back in binary mode */
            debug(F100,"stptrap W_CONNECT","",0);
            break;
#ifndef NOICP
          case W_COMMAND:               /* Suspended in command mode */
            debug(F101,"stptrap W_COMMAND pflag","",pflag);
            concb((char)escape);        /* Put back CBREAK tty mode */
            if (pflag) {                /* If command parsing was */
                prompt(xxstring);       /* reissue the prompt and partial */
                if (!cmflgs)            /* command (if any) */
                  printf("%s",cmdbuf);
            }
            break;
#endif /* NOICP */
          default:                      /* All other cases... */
            debug(F100,"stptrap default","",0);
            concb((char)escape);        /* Put it back in CBREAK mode */
            break;
        }
    }
#endif /* NOJC */
    SIGRETURN;
}

#ifdef TLOG
#define TBUFL 300

/*  T L O G  --  Log a record in the transaction file  */
/*
 Call with a format and 3 arguments: two strings and a number:
   f     - Format, a bit string in range 0-7, bit x is on, arg #x is printed.
   s1,s2 - String arguments 0 and 1.
   n     - Long, argument 2.
*/
VOID
#ifdef CK_ANSIC
dotlog(int f, char *s1, char *s2, CK_OFF_T n)
#else
dotlog(f,s1,s2,n) int f; CK_OFF_T n; char *s1, *s2;
#endif /* CK_ANSIC */
/* dotlog */ {
    static char s[TBUFL];
    extern int tlogfmt;
    char *sp = s; int x;
    if (!s1) s1 = "";
    if (!s2) s2 = "";

    if (!tralog) return;                /* If no transaction log, don't */
    if (tlogfmt != 1) return;
    switch (f) {
      case F000:                        /* 0 (special) "s1 n s2"  */
        if ((int)strlen(s1) + (int)strlen(s2) + 15 > TBUFL)
          sprintf(sp,"?T-Log string too long");
        else
	  sprintf(sp,"%s %s %s",s1,ckfstoa(n),s2);
        if (zsoutl(ZTFILE,s) < 0) tralog = 0;
        break;
      case F001:                        /* 1, " n" */
        sprintf(sp," %s",ckfstoa(n));
        if (zsoutl(ZTFILE,s) < 0) tralog = 0;
        break;
      case F010:                        /* 2, "[s2]" */
        x = (int)strlen(s2);
        if (s2[x] == '\n') s2[x] = '\0';
        if (x + 6 > TBUFL)
          sprintf(sp,"?String too long");
        else sprintf(sp,"[%s]",s2);
        if (zsoutl(ZTFILE,"") < 0) tralog = 0;
        break;
      case F011:                        /* 3, "[s2] n" */
        x = (int)strlen(s2);
        if (s2[x] == '\n') s2[x] = '\0';
        if (x + 6 > TBUFL)
          sprintf(sp,"?String too long");
        else sprintf(sp,"[%s] %s",s2,ckfstoa(n));
        if (zsoutl(ZTFILE,s) < 0) tralog = 0;
        break;
      case F100:                        /* 4, "s1" */
        if (zsoutl(ZTFILE,s1) < 0) tralog = 0;
        break;
      case F101:                        /* 5, "s1: n" */
        if ((int)strlen(s1) + 15 > TBUFL)
          sprintf(sp,"?String too long");
        else sprintf(sp,"%s: %s",s1,ckfstoa(n));
        if (zsoutl(ZTFILE,s) < 0) tralog = 0;
        break;
      case F110:                        /* 6, "s1 s2" */
        x = (int)strlen(s2);
        if (s2[x] == '\n') s2[x] = '\0';
        if ((int)strlen(s1) + x + 4 > TBUFL)
          sprintf(sp,"?String too long");
        else
	  sprintf(sp,"%s%s%s",s1,((*s2 == ':') ? "" : " "),s2);
        if (zsoutl(ZTFILE,s) < 0) tralog = 0;
        break;
      case F111:                        /* 7, "s1 s2: n" */
        x = (int)strlen(s2);
        if (s2[x] == '\n') s2[x] = '\0';
        if ((int)strlen(s1) + x + 15 > TBUFL)
          sprintf(sp,"?String too long");
        else
	  sprintf(sp,"%s%s%s: %s",s1,((*s2 == ':') ? "" : " "),s2,ckfstoa(n));
        if (zsoutl(ZTFILE,s) < 0) tralog = 0;
        break;
      default:
        sprintf(sp,"?Invalid format for tlog() - %d",f);
        if (zsoutl(ZTFILE,s) < 0) tralog = 0;
    }
}

/*
  D O X L O G

  This is the transaction-log writer for BRIEF format.
  The idea is produce one record (line) per file.  Each record
  has the following delimited fields:
    Date (yyyymmdd)
    Time (hh:mm:ss)
    Action: SEND or RECV
    File name
    File size
    Transfer mode (text, binary, image, labeled, etc).
    Status: OK or FAILED
    Free-form comments in doublequotes
  The default separator is comma.
  If a field contains the separator, it is enclosed in doublequotes.
*/
VOID
#ifdef CK_ANSIC
doxlog(int x, char * fn, CK_OFF_T fs, int fm, int status, char * msg)
#else
doxlog(x, fn, fs, fm, status, msg)
    int x; char * fn; CK_OFF_T fs; int fm; int status; char * msg;
#endif /* CK_ANSIC */
/* doxlog */ {
    extern int tlogsep;
    char sep[2];
    char buf[CKMAXPATH+256], * bufp;
    char tmpbuf[32];
    char * s, * p;
    int len, left, ftp = 0, k;

    if (!tralog) return;                /* If no transaction log, don't */

    if (!fn) fn = "";                   /* Protect against null pointers */
    if (!msg) msg = "";
    if (x & W_FTP)
      ftp++;

    sep[0] = (char) tlogsep;
    sep[1] = NUL;
    if (!sep[0]) sep[0] = ',';

    bufp = buf;
    left = sizeof(buf);
    debug(F101,"XXX doxlog left 1","",left);

    p = zzndate();                      /* Date */
    ckmakmsg(buf, left, p ? p : "00000000", sep, NULL, NULL);
    bufp += 9;
    left -= 9;
    debug(F111,"XXX doxlog left 2",buf,left);

    ztime(&p);
    ckstrncpy(bufp,p+11,left);
    bufp += 8;
    left -= 8;
    debug(F111,"XXX doxlog left 3",buf,left);

    if (ftp) {
	if (!(x & (W_SEND|W_RECV)))
	  return;
	s =  (x & W_SEND) ? "PUT" : "GET";
	k = 3;
    } else {
	s =  (x & W_SEND) ? "SEND" : "RECV";
	k = 4;
    }
    ckmakmsg(bufp,left,sep,s,sep,NULL);
    bufp += k + 2;
    left -= (k + 2);
    debug(F111,"XXX doxlog left 4",buf,left);

    s = "";
    if (ckstrchr(fn,sep[0]))		/* Filename */
      s = "\"";
    ckmakmsg(bufp,left,s,fn,s,sep);
    sprintf(tmpbuf,"%s",ckfstoa(fs));	/* Size */
    ckstrncat(buf,tmpbuf,CKMAXPATH);
    ckstrncat(buf,sep,CKMAXPATH);
    debug(F110,"doxlog 4",buf,0);

#ifdef NOICP
    /* Transfer mode */
    ckstrncpy(tmpbuf, (binary ? "binary" : "text"), TMPBUFSIZ);
#else
    ckstrncpy(tmpbuf,gfmode(fm,0),TMPBUFSIZ);
#endif /* NOICP */
    if (ckstrchr(tmpbuf,sep[0])) {      /* Might contain spaces */
        ckstrncat(buf,"\"",CKMAXPATH);
        ckstrncat(buf,tmpbuf,CKMAXPATH);
        ckstrncat(buf,"\"",CKMAXPATH);
    } else
      ckstrncat(buf,tmpbuf,CKMAXPATH);
    ckstrncat(buf,sep,CKMAXPATH);
    debug(F110,"doxlog 5",buf,0);

    ckstrncat(buf, status ? "FAILED" : "OK",CKMAXPATH);
    len = strlen(buf);
    left = CKMAXPATH+256 - len;
    if (left < 2) fatal("doxlog buffer overlow");

    debug(F111,"XXX doxlog left 5",buf,left);

    debug(F110,"doxlog buf 1", buf, len);
    s = buf + len;
    if (status == 0 && left > 32) {
        long cps = 0L;
#ifdef GFTIMER
	debug(F101,"DOXLOG fpxfsecs","",(long)(fpxfsecs * 1000));
        if (fpxfsecs) cps = (long)((CKFLOAT) fs / fpxfsecs);
        sprintf(s,"%s\"%0.3fsec %ldcps\"",sep,fpxfsecs,cps);
#else
        if (xfsecs) cps = fs / xfsecs;
        sprintf(s,"%s\"%ldsec %ldcps\"",sep,xfsecs,cps);
#endif /* GFTIMER */
    } else if ((int)strlen(msg) + 4 < left) {
        sprintf(s,"%s\"%s\"",sep,msg);
    }
    debug(F111,"XXX doxlog left 5",buf,left);

    debug(F110,"doxlog 5",buf,0);
    x = zsoutl(ZTFILE,buf);
    debug(F101,"doxlog zsoutl","",x);
    if (x < 0) tralog = 0;
}
#endif /* TLOG */

#ifndef MAC
/*
  The rest of this file is for all implementations but the Macintosh.
*/

#ifdef CK_CURSES
static int repaint = 0;                 /* Transfer display needs repainting */
#endif /* CK_CURSES */

#ifndef NOXFER
/*  C H K I N T  --  Check for console interrupts  */

/*
  Used during file transfer in local mode only:
  . If user has not touched the keyboard, returns 0 with no side effects.
  . If user typed S or A (etc, see below) prints status message and returns 0.
  . If user typed X or F (etc, see below) returns 0 with cxseen set to 1.
  . If user typed Z or B (etc, see below) returns 0 with czseen set to 1.
  . If user typed E or C (etc, see below) returns -1.
*/
int
chkint() {
    int ch, cn, ofd; long zz;
    if (!xfrint)
      return(0);
    if ((!local) || (quiet)) return(0); /* Only do this if local & not quiet */
#ifdef datageneral
    if (con_reads_mt)                   /* if conint_mt task is active */
      if (conint_avl) {                 /* and there's an interrupt pending */
          cn = 1;                       /* process it */
          ch = conint_ch;
          conint_avl = 0;               /* turn off flag so conint_mt can */
      } else                            /* proceed */
        return(0);
    else                                /* if conint_mt not active */
      if ((ch = coninc(2)) < 0)         /* try to get char manually */
        return(0);                      /* I/O error, or no data */
      else                              /* if successful, set cn so we */
        cn = 1;                         /* know we got one */
    debug(F101,"chkint got keyboard character",ch,cn);
#else /* !datageneral */
#ifdef NTSIG
    {
        extern int TlsIndex;
        struct _threadinfo * threadinfo;
        threadinfo = (struct _threadinfo *) TlsGetValue(TlsIndex);
        if (threadinfo) {
            if (!WaitSem(threadinfo->DieSem,0))
              return -1;                /* Cancel Immediately */
        }
    }
#endif /* NTSIG */
    cn = conchk();                      /* Any input waiting? */
    debug(F101,"conchk","",cn);
    if (cn < 1) return(0);
    ch = coninc(5) ;
    debug(F101,"coninc","",ch);
    if (ch < 0) return(0);
#endif /* datageneral */

    ch &= 0177;
    switch (ch) {
      case 'A': case 'a': case 0001:    /* Status report */
      case 'S': case 's':
        if (fdispla != XYFD_R && fdispla != XYFD_S && fdispla != XYFD_N)
          return(0);                    /* Only for serial, simple or none */
        ofd = fdispla;                  /* [MF] Save file display type */
        if (fdispla == XYFD_N)
          fdispla = XYFD_R;             /* [MF] Pretend serial if no display */
        xxscreen(SCR_TN,0,0l,"Status report:");
        xxscreen(SCR_TN,0,0l," file type: ");
        if (binary) {
            switch(binary) {
              case XYFT_L: xxscreen(SCR_TZ,0,0l,"labeled"); break;
              case XYFT_I: xxscreen(SCR_TZ,0,0l,"image"); break;
              case XYFT_U: xxscreen(SCR_TZ,0,0l,"binary undefined"); break;
              default:
              case XYFT_B: xxscreen(SCR_TZ,0,0l,"binary"); break;
            }
        } else {
#ifdef NOCSETS
            xxscreen(SCR_TZ,0,0l,"text");
#else
            xxscreen(SCR_TU,0,0l,"text, ");
            if (tcharset == TC_TRANSP || xfrxla == 0) {
                xxscreen(SCR_TZ,0,0l,"transparent");
            } else {
                if (what & W_SEND) {
                    xxscreen(SCR_TZ,0,0l,tcsinfo[tcharset].keyword);
                    xxscreen(SCR_TU,0,0l," => ");
                    xxscreen(SCR_TZ,0,0l,fcsinfo[fcharset].keyword);
                } else {
                    xxscreen(SCR_TZ,0,0l,fcsinfo[fcharset].keyword);
                    xxscreen(SCR_TU,0,0l," => ");
                    xxscreen(SCR_TZ,0,0l,tcsinfo[tcharset].keyword);
                }
            }
#endif /* NOCSETS */
        }
        xxscreen(SCR_QE,0,filcnt," file number");
        if (fsize) xxscreen(SCR_QE,0,fsize," size");
        xxscreen(SCR_QE,0,ffc," characters so far");
        if (fsize > 0L) {
#ifdef CK_RESEND
            zz = what & W_SEND ? sendstart : what & W_RECV ? rs_len : 0;
            zz = ( (ffc + zz) * 100L ) / fsize;
#else
            zz = ( ffc * 100L ) / fsize;
#endif /* CK_RESEND */
            xxscreen(SCR_QE,0,zz,      " percent done");
        }
        if (bctu == 4) {                /* Block check */
            xxscreen(SCR_TU,0,0L," block check: ");
            xxscreen(SCR_TZ,0,0L,"blank-free-2");
        } else xxscreen(SCR_QE,0,(long)bctu,  " block check");
        xxscreen(SCR_QE,0,(long)rptflg," compression");
        xxscreen(SCR_QE,0,(long)ebqflg," 8th-bit prefixing");
        xxscreen(SCR_QE,0,(long)lscapu," locking shifts");
        if (!network)
          xxscreen(SCR_QE,0, speed, " speed");
        if (what & W_SEND)

          xxscreen(SCR_QE,0,spsiz, " packet length");
        else if (what & W_RECV || what & W_REMO)
          xxscreen(SCR_QE,0,urpsiz," packet length");
        xxscreen(SCR_QE,0,wslots,  " window slots");
        fdispla = ofd; /* [MF] Restore file display type */
        return(0);

      case 'B': case 'b': case 0002:    /* Cancel batch */
      case 'Z': case 'z': case 0032:
        czseen = 1;
        interrupted = 1;
        xxscreen(SCR_ST,ST_MSG,0l,
                 (((what & W_RECV) && (wslots > 1)) ?
                  "Canceling batch, wait... " :
                  "Canceling batch... ")
                 );
        return(0);

      case 'F': case 'f': case 0006:    /* Cancel file */
      case 'X': case 'x': case 0030:
        cxseen = 1;
        interrupted = 1;
        xxscreen(SCR_ST,ST_MSG,0l,
                 (((what & W_RECV) && (wslots > 1)) ?
                  "Canceling file, wait... " :
                  "Canceling file... ")
                 );
        return(0);

      case 'R': case 'r': case 0022:    /* Resend packet */
      case 0015: case 0012:
#ifdef STREAMING
        if (streaming)
          return(0);
#endif /* STREAMING */
        xxscreen(SCR_ST,ST_MSG,0l,"Resending packet... ");
        numerrs++;
        resend(winlo);
        return(0);

#ifdef datageneral
      case '\03':                       /* We're not trapping ^C's with */
        trap(0);                        /* signals, so we check here    */
#endif /* datageneral */

      case 'C': case 'c':               /* Ctrl-C */
#ifndef datageneral
      case '\03':
#endif /* datageneral */

      case 'E': case 'e':               /* Send error packet */
      case 0005:
        interrupted = 1;
        return(-1);

#ifdef CK_CURSES
      case 0014:                        /* Ctrl-L to refresh screen */
      case 'L': case 'l':               /* Also accept L (upper, lower) */
      case 0027:                        /* Ctrl-W synonym for VMS & Ingres */
        repaint = 1;
        return(0);
#endif /* CK_CURSES */

      case 'T':
      case 't':				/* Turn on debug-log timestamps */
#ifdef DEBUG
	{
	    extern int debtim;
	    if (ch == 'T') {
		debtim = 1;
		xxscreen(SCR_ST,ST_MSG,0l,
			 "Debug timestamps On... ");
	    } else {
		debtim = 1;
		xxscreen(SCR_ST,ST_MSG,0l,
			 "Debug timestamps Off... ");
	    }
	}
#endif /* DEBUG */
	return(0);

      case 'D':
#ifdef DEBUG
	if (!deblog) {
	    debopn("debug.log",0);
	    if (deblog) {
		xxscreen(SCR_ST,ST_MSG,0l,"debug.log open... ");
	    } else {
		xxscreen(SCR_ST,ST_MSG,0l,"debug.log open FAILED... ");
	    }
	} else {
	    xxscreen(SCR_ST,ST_MSG,0l,"Debug log On... ");
	}
	if (deblog)
	  debok = 1;
#endif /* DEBUG */
	return(0);

      case 'd':				/* Turn off debugging */
#ifdef DEBUG
	if (deblog)
	  xxscreen(SCR_ST,ST_MSG,0l,"Debug log Off... ");
	debok = 0;
#endif /* DEBUG */
	return(0);

      default:                          /* Anything else, print message */
        intmsg(1L);
        return(0);
    }
}

/*  I N T M S G  --  Issue message about terminal interrupts  */

VOID
#ifdef CK_ANSIC
intmsg(long n)
#else
intmsg(n) long n;
#endif /* CK_ANSIC */
/* intmsg */ {
#ifdef CK_NEED_SIG
    char buf[80];
#endif /* CK_NEED_SIG */

    if (!displa || quiet)               /* Not if we're being quiet */
      return;
    if (server && (!srvdis || n > -1L)) /* Special for server */
      return;
#ifdef CK_NEED_SIG
    buf[0] = NUL;                       /* Keep compilers happy */
#endif /* CK_NEED_SIG */
#ifndef OXOS
#ifdef SVORPOSIX
    conchk();                           /* Clear out pending escape-signals */
#endif /* SVORPOSIX */
#endif /* ! OXOS */
#ifdef VMS
    conres();                           /* So Ctrl-C will work */
#endif /* VMS */
    if ((!server && n == 1L) || (server && n < 0L)) {

#ifdef CK_NEED_SIG
        if (xfrint) {
	    ckmakmsg(buf,
		     80,
		     "Type escape character (",
		     dbchr(escape),
		     ") followed by:",
		     NULL
		     );
            xxscreen(SCR_TN,0,0l,buf);
        }
#endif /* CK_NEED_SIG */

        if (xfrint) {
            if (protocol == PROTO_K) {
 xxscreen(SCR_TN,0,0l,"X to cancel file,  CR to resend current packet");
 xxscreen(SCR_TN,0,0l,"Z to cancel group, A for status report");
 xxscreen(SCR_TN,0,0l,"E to send Error packet, Ctrl-C to quit immediately: ");
            } else {
                xxscreen(SCR_TN,0,0l,"Ctrl-C to cancel file transfer: ");
            }
        } else {
            xxscreen(SCR_TN,0,0l,"Transfer interruption disabled. ");
        }
    }
    else xxscreen(SCR_TU,0,0l," ");
}

#ifndef NODISPLAY
static int newdpy = 0;                  /* New display flag */
static char fbuf[80];                   /* Filename buffer */
static char abuf[80];                   /* As-name buffer */
static char a2buf[80];                  /* Second As-name buffer */
static CK_OFF_T oldffc = 0L;
static CK_OFF_T dots = 0L;
static int hpos = 0;

static VOID                             /* Initialize Serial or CRT display */
dpyinit() {
    int m = 0, n = 0;
    char * s = "";

    newdpy = 0;                         /*  Don't do this again */
    oldffc = (CK_OFF_T)0;		/*  Reset this */
    dots = (CK_OFF_T)0;			/*  and this.. */
    oldcps = cps = 0L;

    conoll("");				/* New line */
    if (what & W_SEND) s = "Sending: ";	/* Action */
    else if (what & W_RECV) s = "Receiving: ";
    n = (int)strlen(s) + (int)strlen(fbuf);
    conol(fbuf);
    m = (int)strlen(abuf) + 4;
    if (n + m > cmd_cols) {
        conoll("");
        n = 0;
    } else
      n += m;
    if (*abuf) {
        conol(" => ");
        conol(abuf);
    }
    m = (int)strlen(a2buf) + 4;
    if (n + m > cmd_cols) {
        conoll("");
        n = 0;
    } else
      n += m;
    if (*a2buf) {
        conol(" => ");
        conol(a2buf);
    }
    *fbuf = NUL; *abuf = NUL; *a2buf = NUL;
    conoll("");
    if (fsize > (CK_OFF_T)-1) {		/* Size */
        sprintf(fbuf,"Size: %s, Type: ",ckfstoa(fsize)); /* SAFE (80) */
        conol(fbuf); *fbuf = NUL;
    } else conol("Size: unknown, Type: ");
    if (binary) {			/* Type */
        switch(binary) {
              case XYFT_L: conol("labeled"); break;
              case XYFT_I: conol("image"); break;
              case XYFT_U: conol("binary undefined"); break;
              default:
              case XYFT_B: conol("binary"); break;
        }
    } else {
#ifdef NOCSETS
        conol("text");
#else
        conol("text, ");
        if (tcharset == TC_TRANSP || xfrxla == 0) {
            conol("transparent");
        } else {
            if (what & W_SEND) {
                conol(fcsinfo[fcharset].keyword);
                conol(" => ");
                conol(tcsinfo[tcharset].keyword);
            } else {
                conol(tcsinfo[tcharset].keyword);
                conol(" => ");
                conol(fcsinfo[fcharset].keyword);
            }
        }
#endif /* NOCSETS */
    }
#ifdef STREAMING
    if (streaming)
      conol(", STREAMING");
#endif /* STREAMING */
    conoll("");

    if (fdispla == XYFD_S) {            /* CRT field headings */
/*
  Define CK_CPS to show current transfer rate.
  Leave it undefined to show estimated time remaining.
  Estimated-time-remaining code from Andy Fyfe, not tested on
  pathological cases.
*/
#define CK_CPS

#ifdef CK_CPS
        conoll("    File   Percent       Packet");
        conoll("    Bytes  Done     CPS  Length");
#else
        conoll("    File   Percent  Secs Packet");
        conoll("    Bytes  Done     Left Length");
#endif /* CK_CPS */
        newdpy = 0;
    }
    hpos = 0;
}

/*
  showpkt(c)
  c = completion code: 0 means transfer in progress, nonzero means it's done.
  Show the file transfer progress counter and perhaps verbose packet type.
*/
VOID
#ifdef CK_ANSIC
showpkt(char c)
#else
showpkt(c) char c;
#endif /* CK_ANSIC */
/* showpkt */ {

#ifndef GFTIMER
    long et;                            /* Elapsed time, entire batch  */
#endif /* GFTIMER */
    CK_OFF_T howfar;			/* How far into file */
    long pd;                            /* Percent done, this file     */
    long tp;                            /* Transfer rate, entire batch */
    long ps;                            /* Packet size, current packet */
    CK_OFF_T mytfc;			/* Local copy of byte counter  */

#ifdef GFTIMER
    CKFLOAT tnow;
#endif /* GFTIMER */

    if (newdpy)                         /* Put up filenames, etc, */
      dpyinit();                        /* if they're not there already. */

    howfar = ffc;                       /* How far */
/*
  Calculate CPS rate even if not displaying on screen for use in file
  transfer statistics.
*/
#ifdef GFTIMER
    tnow = gftimer();                   /* Time since we started */
    ps = (what & W_RECV) ? rpktl : spktl; /* Packet size */
#ifdef CK_RESEND
    if (what & W_SEND)			/* In case we didn't start at */
      howfar += sendstart;              /*  the beginning... */
    else if (what & W_RECV)
      howfar += rs_len;
#endif /* CK_RESEND */
    pd = -1;                            /* Percent done. */
    if (c == NUL) {                     /* Still going, figure % done */
        if (!fsize) return;		/* Empty file, don't bother */
        pd = (fsize > 99) ? (howfar / (fsize / (CK_OFF_T)100)) : 0;
        if (pd > 100) pd = 100;         /* Expansion */
    }
    if (c != NUL)
      if (!cxseen && !discard && !czseen)
        pd = 100;                       /* File complete, so 100%. */

    mytfc = (pd < 100) ? tfc + ffc : tfc;    /* CPS */
    tp = (long)((tnow > 0.0) ? (CKFLOAT) mytfc / tnow : 0);
    if (c && (tp == 0))
      tp = ffc;

    cps = tp;                           /* Set global variable */
    if (cps > peakcps &&                /* Peak transfer rate */
         ((what & W_SEND && spackets > wslots + 4) ||
	  (!(what & W_SEND) && spackets > 10))) {
        peakcps = cps;
    }

#else  /* Not GFTIMER */

    et = gtimer();                      /* Elapsed time  */
    ps = (what & W_RECV) ? rpktl : spktl; /* Packet length */
#ifdef CK_RESEND
    if (what & W_SEND)			/* And if we didn't start at */
      howfar += sendstart;              /*  the beginning... */
    else if (what & W_RECV)
      howfar += rs_len;
#endif /* CK_RESEND */
    pd = -1;                            /* Percent done. */
    if (c == NUL) {                     /* Still going, figure % done */
        if (fsize == 0L) return;        /* Empty file, don't bother */
        pd = (fsize > 99) ? (howfar / (fsize / (CK_OFF_T)100)) : 0;
        if (pd > 100) pd = 100;         /* Expansion */
    }
    if (c != NUL)
      if (!cxseen && !discard && !czseen)
        pd = 100;                       /* File complete, so 100%. */


#ifndef CK_CPS
/*
  fsecs = time (from gtimer) that this file started (set in sfile()).
  Rate so far is ffc / (et - fsecs),  estimated time for remaining bytes
  is (fsize - ffc) / (ffc / (et - fsecs)).
*/
    tp = (howfar > 0) ? (fsize - howfar) * (et - fsecs) / howfar : 0;
#endif /* CK_CPS */

#ifdef CK_CPS
    mytfc = (pd < 100) ? tfc + ffc : tfc;
    tp = (et > 0) ? mytfc / et : 0;	/* Transfer rate */
    if (c && (tp == 0))			/* Watch out for subsecond times */
        tp = ffc;

    cps = tp;				/* Set global variable */
    if (cps > peakcps &&                /* Peak transfer rate */
         ((what & W_SEND && spackets > wslots + 4) ||
	  (!(what & W_SEND) && spackets > 10))) {
        peakcps = cps;
    }
#endif /* CK_CPS */

#endif /* GFTIMER */

    if (fdispla == XYFD_S) {            /* CRT display */
        char buffer[128];
	/* These sprintfs should be safe until we have 32-digit numbers */

        if (pd > -1L)
          sprintf(buffer, "%c%9s%5ld%%%8ld%8ld ", CR,ckfstoa(howfar),pd,tp,ps);
        else
          sprintf(buffer, "%c%9s      %8ld%8ld ", CR,ckfstoa(howfar),tp,ps);
        conol(buffer);
        hpos = 31;
    } else if (fdispla == XYFD_R) {     /* SERIAL */
        long i, k;
        if (howfar - oldffc < 1024)     /* Update display every 1K */
          return;
        oldffc = howfar;                /* Time for new display */
        k = (howfar / 1024L) - dots;    /* How many K so far */
        for (i = 0L; i < k; i++) {
            if (hpos++ > (cmd_cols - 3)) { /* Time to wrap? */
                conoll("");
                hpos = 0;
            }
            conoc('.');                 /* Print a dot for this K */
            dots++;                     /* Count it */
        }
    }
}


/*  C K S C R E E N  --  Screen display function  */

/*
  ckscreen(f,c,n,s)
    f - argument descriptor
    c - a character or small integer
    n - a long integer
    s - a string.

  and global fdispla = SET FILE DISPLAY value:

    XYFD_N = NONE
    XYFD_R = SERIAL:     Dots, etc, works on any terminal, even hardcopy.
    XYFD_S = CRT:        Works on any CRT, writes over current line.
    XYFD_C = FULLSCREEN: Requires terminal-dependent screen control.
    XYFD_B = BRIEF:      Like SERIAL but only filename & completion status.
    XYFD_G = GUI;        Windows GUI, same behavior as FULLSCREEN
*/
VOID
#ifdef CK_ANSIC
ckscreen(int f, char c,CK_OFF_T n,char *s)
#else
ckscreen(f,c,n,s) int f; char c; CK_OFF_T n; char *s;
#endif /* CK_ANSIC */
/* screen */ {
    char buf[80];
    int len;                            /* Length of string */
#ifdef UNIX
#ifndef NOJC
    int obg;
_PROTOTYP( VOID conbgt, (int) );
#endif /* NOJC */
#endif /* UNIX */
    int ftp = 0;

    ftp = (what & W_FTP) ? 1 : 0;	/* FTP or Kermit? */

    if (!local && !ftp)			/* In remote mode - don't do this */
      return;

    if (!s) s = "";

    if (!fxd_inited)                    /* Initialize if necessary */
      fxdinit(fdispla);

#ifdef UNIX
#ifndef NOJC
    obg = backgrd;                      /* Previous background status */
    conbgt(1);                          /* See if running in background */
    if (!backgrd && obg) {              /* Just came into foreground? */
        concb((char)escape);            /* Put console back in CBREAK mode */
        setint();                       /* Restore interrupts */
    }
#endif /* NOJC */
#endif /* UNIX */

    if ((f != SCR_WM) && (f != SCR_EM)) /* Always update warnings & errors */
      if (!displa ||
          (backgrd && bgset) ||
          fdispla == XYFD_N ||
          (server && !srvdis)
          )
        return;

#ifdef VMS
    if (f == SCR_FN)                    /* VMS - shorten the name */
      s = zrelname(s,zgtdir());
#endif /* VMS */

    if (dest == DEST_S)                 /* SET DESTINATION SCREEN */
      return;                           /*  would interfere... */

#ifdef KUI
    if (fdispla == XYFD_G) {            /* If gui display selected */
        screeng(f,c,n,s);               /* call the gui version */
        return;
    }
#endif /* KUI */
#ifdef CK_CURSES
    if (fdispla == XYFD_C) {            /* If fullscreen display selected */
        screenc(f,c,n,s);               /* call the fullscreen version */
        return;
    }
#endif /* CK_CURSES */

    len = (int)strlen(s);               /* Length of string */

    switch (f) {                        /* Handle our function code */
      case SCR_FN:                      /* Filename */
        if (fdispla == XYFD_B) {
#ifdef NEWFTP
	    if (ftp)
	      printf(" %s %s", what & W_SEND ? "PUT" : "GET", s);
	    else
#endif /* NEWFTP */
	      printf(" %s %s", what & W_SEND ? "SEND" : "RECV", s);
#ifdef UNIX
            fflush(stdout);
#endif /* UNIX */
            return;
        }
#ifdef MAC
        conoll(""); conol(s); conoc(SP); hpos = len + 1;
#else
        ckstrncpy(fbuf,s,80);
        abuf[0] = a2buf[0] = NUL;
        newdpy = 1;                     /* New file so refresh display */
#endif /* MAC */
        return;

      case SCR_AN:                      /* As-name */
        if (fdispla == XYFD_B) {
#ifdef COMMENT
            printf("(as %s) ",s);
#endif /* COMMENT */
            return;
        }
#ifdef MAC
        if (hpos + len > 75) { conoll(""); hpos = 0; }
        conol("=> "); conol(s);
        if ((hpos += (len + 3)) > 78) { conoll(""); hpos = 0; }
#else
        if (abuf[0]) {
            ckstrncpy(a2buf,s,80);
        } else {
            ckstrncpy(abuf,s,80);
        }
#endif /* MAC */
        return;

      case SCR_FS:                      /* File-size */
        if (fdispla == XYFD_B) {
            printf(" (%s) (%s byte%s)",
#ifdef NOICP
                   (binary ? "binary" : "text")
#else
                   gfmode(binary,0)
#endif /* NOICP */
                   , ckfstoa(n), n == 1 ? "" : "s");
#ifdef UNIX
            fflush(stdout);
#endif /* UNIX */
            return;
        }
#ifdef MAC
        sprintf(buf,", Size: %s",ckfstoa(n));  conoll(buf);  hpos = 0;
#endif /* MAC */
        return;

      case SCR_XD:                      /* X-packet data */
        if (fdispla == XYFD_B)
          return;
#ifdef MAC
        conoll(""); conoll(s); hpos = 0;
#else
        ckstrncpy(fbuf,s,80);
        abuf[0] = a2buf[0] = NUL;
#endif /* MAC */
        return;

      case SCR_ST:                      /* File status */
        switch (c) {
          case ST_OK:                   /* Transferred OK */
            showpkt('Z');               /* Update numbers one last time */
            if (fdispla == XYFD_B) {
#ifdef GFTIMER
		if (fpxfsecs)
		  printf(": OK (%0.3f sec, %ld cps)",fpxfsecs,
			 (long)((CKFLOAT)ffc / fpxfsecs));
#else
		if (xfsecs)
		  printf(": OK (%d sec, %ld cps)",xfsecs,ffc/xfsecs);
#endif /* GFTIMER */
		printf("\n");
                return;
            }
            if ((hpos += 5) > 78) conoll(""); /* Wrap screen line. */
            conoll(" [OK]"); hpos = 0;  /* Print OK message. */
            if (fdispla == XYFD_S) {    /* We didn't show Z packet when */
                conoc('Z');             /* it came, so show it now. */
                hpos = 1;
            }
            return;

          case ST_DISC:                 /*  Discarded */
            if (fdispla == XYFD_B) {
                printf(": DISCARDED\n");
                return;
            }
            if ((hpos += 12) > 78) conoll("");
            conoll(" [discarded]"); hpos = 0;
            return;

          case ST_INT:                  /*  Interrupted */
            if (fdispla == XYFD_B) {
                printf(": INTERRUPTED\n");
                return;
            }
            if ((hpos += 14) > 78) conoll("");
            conoll(" [interrupted]"); hpos = 0;
            return;

	  case ST_SIM:
            if (fdispla == XYFD_B) {
		if (n == SKP_XNX)
		  printf(": WOULD BE TRANSFERRED (New file)\n");
		else if (n == SKP_XUP)
		  printf(": WOULD BE TRANSFERRED (Remote file older)\n");
		else if (n == SKP_SIM)
		  printf(": WOULD BE TRANSFERRED\n");
		else if (n > 0 && n < nskreason)
		  printf(": SKIPPED (%s)\n",skreason[n]);
		else
		  printf(": SKIPPED\n");
                return;
            } else if (fdispla == XYFD_S) {
                if (fdispla == XYFD_S && fbuf[0]) { /* CRT display */
                    conoll("");         /* New line */
                    if (what & W_SEND) conol("Would Send: "); /* Action */
                    else if (what & W_RECV) conol("Would Receive: ");
                    conol(fbuf);
                    if (*abuf) conol(" => "); conol(abuf); /* Names */
                    if (*a2buf) conol(" => "); conol(a2buf); /* Names */
                    *fbuf = NUL; *abuf = NUL; *a2buf = NUL;
                }
                conoll(" [simulated]");
                return;
            }
            if ((hpos += 10) > 78) conoll("");
            conol(" [simulated]"); hpos = 0;
            return;

          case ST_SKIP:                 /*  Skipped */
            if (fdispla == XYFD_B) {
		if (n == SKP_XNX)
		  printf(": WOULD BE TRANSFERRED (New file)\n");
		else if (n == SKP_XUP)
		  printf(": WOULD BE TRANSFERRED (Remote file older)\n");
		else if (n == SKP_SIM)
		  printf(": WOULD BE TRANSFERRED\n");
		else if (n > 0 && n < nskreason)
		  printf(": SKIPPED (%s)\n",skreason[n]);
		else
		  printf(": SKIPPED\n");
                return;
            } else if (fdispla == XYFD_S) {
                if (fdispla == XYFD_S && fbuf[0]) { /* CRT display */
                    conoll("");         /* New line */
                    if (what & W_SEND) conol("Sending: "); /* Action */
                    else if (what & W_RECV) conol("Receiving: ");
                    conol(fbuf);
                    if (*abuf) conol(" => "); conol(abuf); /* Names */
                    if (*a2buf) conol(" => "); conol(a2buf); /* Names */
                    *fbuf = NUL; *abuf = NUL; *a2buf = NUL;
                }
                conoll(" [skipped]");
                return;
            }
            if ((hpos += 10) > 78) conoll("");
	    conol(" "); conol(fbuf);
            conoll(" [skipped]"); hpos = 0;
            return;

          case ST_ERR:                  /* Error */
            if (fdispla == XYFD_B) {
                printf(": ERROR: %s\n",s);
                return;
            }
            conoll("");
            conol("Error: "); conoll(s); hpos = 0;
            return;

          case ST_MSG:                  /* Message */
#ifdef NEWFTP
            if (fdispla == XYFD_B) {
                if (ftp && ftp_deb)
		  printf(": MESSAGE: %s\n",s);
                return;
            }
#endif /* NEWFTP */
            conoll("");
            conol("Message: ");
            conoll(s);
            hpos = 0;
            return;

          case ST_REFU:                 /* Refused */
            if (fdispla == XYFD_B) {
                printf(": REFUSED\n");
                return;
            } else if (fdispla == XYFD_S) {
                if (fdispla == XYFD_S && fbuf[0]) { /* CRT display */
                    conoll("");         /* New line */
                    if (what & W_SEND) conol("Sending: "); /* Action */
                    else if (what & W_RECV) conol("Receiving: ");
                    conol(fbuf);
                    if (*abuf) conol(" => "); conol(abuf);      /* Names */
                    if (*a2buf) conol(" => "); conol(a2buf);    /* Names */
                    *fbuf = NUL; *abuf = NUL; *a2buf = NUL;
                    conoll("");
                }
                conol("Refused: "); conoll(s);
                return;
            }
            conoll("");
            conol("Refused: "); conoll(s); hpos = 0;
            return;

          case ST_INC:                  /* Incomplete */
            if (fdispla == XYFD_B) {
                printf(": INCOMPLETE\n");
                return;
            }
            if ((hpos += 12) > 78) conoll("");
            conoll(" [incomplete]"); hpos = 0;
            return;

          default:
            conoll("*** screen() called with bad status ***");
            hpos = 0;
            return;
        }

#ifdef MAC
      case SCR_PN:                      /* Packet number */
        if (fdispla == XYFD_B) {
            return;
        }
	ckmakmsg(buf,80,s,": ",ckltoa(n),NULL);
        conol(buf); hpos += (int)strlen(buf); return;
#endif /* MAC */

      case SCR_PT:                      /* Packet type or pseudotype */
        if (fdispla == XYFD_B)
          return;
        if (c == 'Y') return;           /* Don't bother with ACKs */
        if (c == 'D') {                 /* In data transfer phase, */
            showpkt(NUL);               /* show progress. */
            return;
        }
#ifndef AMIGA
        if (hpos++ > 77) {              /* If near right margin, */
            conoll("");                 /* Start new line */
            hpos = 0;                   /* and reset counter. */
        }
#endif /* AMIGA */
        if (c == 'Z' && fdispla == XYFD_S)
          return;
        else
          conoc(c);                     /* Display the packet type. */
#ifdef AMIGA
        if (c == 'G') conoll("");       /* New line after G packets */
#endif /* AMIGA */
        return;

      case SCR_TC:                      /* Transaction complete */
        if (xfrbel) bleep(BP_NOTE);
        if (fdispla == XYFD_B) {        /* Brief display... */
            if (filcnt > 1) {
                long fx;
                fx = filcnt - filrej;
                printf(" SUMMARY: %ld file%s", fx, ((fx == 1) ? "" : "s"));
                printf(", %s byte%s", ckfstoa(tfc), ((tfc == 1) ? "" : "s"));
#ifdef GFTIMER
                printf(", %0.3f sec, %ld cps", fptsecs, tfcps);
#else
                printf(", %ld sec, %ld cps", tsecs, tfcps);
#endif /* GFTIMER */
                printf(".\n");
            }
        } else {
            conoll("");
        }
#ifdef UNIX
        fflush(stdout);
#endif /* UNIX */
        return;

      case SCR_EM:                      /* Error message */
        if (fdispla == XYFD_B) {
            printf(" ERROR: %s\n",s);
            return;
        }
        conoll(""); conoc('?'); conoll(s); hpos = 0; return;

      case SCR_WM:                      /* Warning message */
        if (fdispla == XYFD_B) {
            printf(" WARNING: %s\n",s);
            return;
        }
        conoll(""); conoll(s); hpos = 0; return;

      case SCR_MS:                      /* Message from other Kermit */
        if (fdispla == XYFD_B) {
            printf(" MESSAGE: %s\n",s);
            return;
        }
        conoll(""); conoll(s); hpos = 0; return;

      case SCR_TU:                      /* Undelimited text */
        if (fdispla == XYFD_B)
          return;
        if ((hpos += len) > 77) { conoll(""); hpos = len; }
        conol(s); return;

      case SCR_TN:                      /* Text delimited at beginning */
        if (fdispla == XYFD_B)
          return;
        conoll(""); conol(s); hpos = len; return;

      case SCR_TZ:                      /* Text delimited at end */
        if (fdispla == XYFD_B)
          return;
        if ((hpos += len) > 77) { conoll(""); hpos = len; }
        conoll(s); return;

      case SCR_QE:                      /* Quantity equals */
        if (fdispla == XYFD_B)
          return;
	ckmakmsg(buf,80,s,": ",ckltoa(n),NULL);
        conoll(buf); hpos = 0; return;

      case SCR_CW:                      /* Close fullscreen window */
        return;                         /* No window to close */

      case SCR_CD:
        return;

      default:
        conoll("*** screen() called with bad object ***");
        hpos = 0;
        return;
    }
}
#endif /* NODISPLAY */

/*  E R M S G  --  Nonfatal error message  */

/* Should be used only for printing the message text from an Error packet. */

VOID
ermsg(msg) char *msg; {                 /* Print error message */
    debug(F110,"ermsg",msg,0);
    if (local)
      xxscreen(SCR_EM,0,0L,msg);
    tlog(F110,"Protocol Error:",msg,0L);
}
#endif /* NOXFER */

VOID
setseslog(x) int x; {
    seslog = x;
#ifdef KUI
    KuiSetProperty(KUI_TERM_CAPTURE,x,0);
#endif /* KUI */
}

VOID
doclean(fc) int fc; {                   /* General cleanup */
#ifdef OS2ORUNIX
    extern int ttyfd;
#endif /* OS2ORUNIX */
    extern int  keep;
    extern int exithangup;
#ifndef NOXFER
    extern char filnam[];
#endif /* NOXFER */
#ifndef NOICP
    int x;

    if (fc > 0)
      dostop();                 /* Stop all command files and end macros */
#endif /* NOICP */

#ifndef NOXFER
    if (pktlog) {
        *pktfil = '\0';
        pktlog = 0;
        zclose(ZPFILE);
    }
#endif /* NOXFER */
    if (seslog) {
        *sesfil = '\0';
        setseslog(0);
        zclose(ZSFILE);
    }
#ifdef TLOG
    if (tralog) {
        tlog(F100,"Transaction Log Closed","",0L);
        *trafil = '\0';
        tralog = 0;
        zclose(ZTFILE);
    }
#endif /* TLOG */

    debug(F100,"doclean calling dologend","",0);
    dologend();                         /* End current log record if any */
#ifdef COMMENT
    if (dialog) {                       /* If connection log open */
	dialog = 0;
        *diafil = '\0';                 /* close it. */
        zclose(ZDIFIL);
    }
#endif /* COMMENT */

#ifndef NOICP
#ifndef NOSPL
    zclose(ZRFILE);                     /* READ and WRITE files, if any. */
    zclose(ZWFILE);
#ifndef NOXFER
    zclose(ZIFILE);                     /* And other files too */
    x = chkfn(ZOFILE);			/* Download in progress? */
    debug(F111,"doclean chkfn ZOFILE",filnam,x);
    debug(F111,"doclean keep","",keep);
    zclose(ZOFILE);			/* Close output file */
    if (x > 0 && !keep) {		/* If it was being downloaded */
	if (filnam[0])
	  x = zdelet(filnam);		/* Delete if INCOMPLETE = DISCARD */
	debug(F111,"doclean download filename",filnam,x);
    }
#endif /* NOXFER */
    zclose(ZSYSFN);
    zclose(ZMFILE);

    if (fc < 1) {                       /* RESETing, not EXITing */
#ifdef DEBUG
        if (deblog) {                   /* Close the debug log. */
            *debfil = '\0';
            deblog = 0;
            zclose(ZDFILE);
        }
#endif /* DEBUG */
        return;
    }
#endif /* NOSPL */
#endif /* NOICP */

#ifndef NOLOCAL
    debug(F101,"doclean exithangup","",exithangup);
    if (local && exithangup) {		/* Close communication connection */
        extern int haslock;
	int x;
	
	x = ttchk();
	debug(F101,"doclean ttchk()","",x);
#ifdef OS2ORUNIX
	debug(F101,"doclean ttyfd","",ttyfd);
#endif /* OS2ORUNIX */
        if (x >= 0
#ifdef OS2
            || ttyfd != -1
#else
#ifdef UNIX
            || haslock                  /* Make sure we get lockfile! */
            || (!network && ttyfd > -1)
#endif /* UNIX */
#endif /* OS2 */
            ) {
            extern int wasclosed, whyclosed;
	    debug(F100,"doclean hanging up and closing","",0);
            if (msgflg) {
#ifdef UNIX
                fflush(stdout);
#endif /* UNIX */
                printf("Closing %s...",ttname);
            }
#ifndef NODIAL
            mdmhup();                   /* Hangup the modem??? */
#endif /* NODIAL */
            ttclos(0);                  /* Close external line, if any */
            if (msgflg) {
                printf("OK\n");
#ifdef UNIX
                fflush(stdout);
#endif /* UNIX */
            }
            if (wasclosed) {
                whyclosed = WC_CLOS;
#ifndef NOSPL
                if (nmac) {             /* Any macros defined? */
                    int k;              /* Yes */
                    k = mlook(mactab,"on_close",nmac);  /* Look this up */
                    if (k >= 0) {                       /* If found, */
                        wasclosed = 0;
                        /* printf("ON_CLOSE DOCLEAN\n"); */
                        *(mactab[k].kwd) = NUL;         /* See comment below */
                        if (dodo(k,ckitoa(whyclosed),0) > -1) /* set it up, */
                          parser(1);                    /* and execute it */
                    }
                }
#endif /* NOSPL */
                wasclosed = 0;
            }
        }
        ckstrncpy(ttname,dftty,TTNAMLEN); /* Restore default tty */
        local = dfloc;                  /* And default remote/local status */
    }
#ifdef DEBUG
    else if (local) debug(F100,"doclean hangup/close skipped","",0);
#endif /* DEBUG */
#endif /* NOLOCAL */

#ifdef NEWFTP
    ftpbye();				/* If FTP connection open, close it */
#endif /* NEWFTP */

#ifdef IKSD
    if (inserver)
      ttclos(0);			/* If IKSD, close socket */
#endif /* IKSD */

#ifndef NOSPL
/*
  If a macro named "on_exit" is defined, execute it.  Also remove it from the
  macro table, in case its definition includes an EXIT or QUIT command, which
  would cause much recursion and would prevent the program from ever actually
  EXITing.
*/
    if (nmac) {                         /* Any macros defined? */
        int k;                          /* Yes */
        char * cmd = "on_exit";         /* MSVC 2.x compiler error */
        k = mlook(mactab,cmd,nmac);     /* Look up "on_exit" */
        if (k >= 0) {                   /* If found, */
#ifdef COMMENT
	    /* This makes a mess if ON_EXIT itself executes macros */
            *(mactab[k].kwd) = NUL;     /* poke its name from the table, */
#else
	    /* Replace the keyword with something that doesn't wreck the */
	    /* order of the keyword table */
	    ckstrncpy(mactab[k].kwd,"on_exxx",8);
#endif /* COMMENT */
            if (dodo(k,"",0) > -1)      /* set it up, */
              parser(1);                /* and execute it */
        }
    }
#endif /* NOSPL */
/*
  Put console terminal back to normal.  This is done here because the
  ON_EXIT macro calls the parser, which meddles with console terminal modes.
*/
    conres();                           /* Restore console terminal. */

#ifdef COMMENT
/* Should be no need for this, and maybe it's screwing things up? */
    connoi();                           /* Turn off console interrupt traps */
#endif /* COMMENT */

    /* Delete the Startup File if we are supposed to. */
#ifndef NOICP
    {
        extern int DeleteStartupFile;
        debug(F111,"doclean DeleteStartupFile",cmdfil,DeleteStartupFile);
        if (DeleteStartupFile) {
            int rc = zdelet(cmdfil);
            debug(F111,"doclean zdelet",cmdfil,rc);
        }
    }
#endif /* NOICP */
    syscleanup();                       /* System-dependent cleanup, last */
}

/*  D O E X I T  --  Exit from the program.  */

/*
  First arg is general, system-independent symbol: GOOD_EXIT or BAD_EXIT.
  If second arg is -1, take 1st arg literally.
  If second arg is not -1, work it into the exit code.
*/
VOID
doexit(exitstat,code) int exitstat, code; {
    extern int x_logged, quitting;
#ifdef OS2
    extern int display_demo;
    extern int SysInited;
#endif /* OS2 */
#ifdef CK_KERBEROS
#ifdef KRB4
    extern int krb4_autodel;
#endif /* KRB4 */
#ifdef KRB5
    extern int krb5_autodel;
#endif /* KRB5 */
#endif /* CK_KERBEROS */

#ifdef VMS
    char envstr[64];
    static $DESCRIPTOR(symnam,"CKERMIT_STATUS");
    static struct dsc$descriptor_s symval;
#endif /* VMS */
    int i;

#ifdef DEBUG
#ifdef USE_LUCACHE
    extern long lucalls, luhits, xxhits, luloop;
    extern int lusize;
#endif /* USE_LUCACHE */
#ifndef NOSPL
    extern int cmdstats[];
#endif /* NOSPL */

    quitting++;

#ifdef OS2
    if ( !SysInited ) {
        static int initing = 0;
        if ( initing )
	  exit(253);
        initing = 1;
        sysinit();
    }
#endif /* OS2 */

    if (deblog) {
#ifdef USE_LUCACHE
	debug(F101,"lookup cache size","",lusize);
	debug(F101,"lookup calls ....","",lucalls);
	debug(F101,"lookup cache hits","",luhits);
	debug(F101,"lookup start hits","",xxhits);
	debug(F101,"lookup loop iterations","",luloop);
#endif /* USE_LUCACHE */
#ifndef NOSPL
	for (i = 0; i < 256; i++) {
	    if (cmdstats[i])
	      debug(F111,"CMSTATS",ckitoa(i),cmdstats[i]);
	}
#endif /* NOSPL */
	debug(F101,"doexit exitstat","",exitstat);
	debug(F101,"doexit code","",code);
	debug(F101,"doexit xitsta","",xitsta);
    }
#endif /* DEBUG */

#ifdef CK_KERBEROS
    /* If we are automatically destroying Kerberos credentials on Exit */
    /* do it now. */
#ifdef KRB4
    if (krb4_autodel == KRB_DEL_EX) {
        extern struct krb_op_data krb_op;
        krb_op.version = 4;
        krb_op.cache = NULL;
        ck_krb4_destroy(&krb_op);
    }
#endif /* KRB4 */
#ifdef KRB5
    if (krb5_autodel == KRB_DEL_EX) {
        extern struct krb_op_data krb_op;
        extern char * krb5_d_cc;
        krb_op.version = 5;
        krb_op.cache = krb5_d_cc;
        ck_krb5_destroy(&krb_op);
    }
#endif /* KRB5 */
#endif /* CK_KERBEROS */

#ifndef NOLOCAL
#ifdef OS2
    if (SysInited)
    {
#ifdef DCMDBUF
        extern struct cmdptr *cmdstk;
#else
        extern struct cmdptr cmdstk[];
#endif /* DCMDBUF */
        extern int tt_status[];
        extern int vmode;

        /* If there is a demo screen to be displayed, display it */
        if (display_demo) {
            demoscrn(VCMD);
            display_demo = 0;
        }
#ifndef KUI
        /* This is going to be hideous.  If we have a status line */
        /* in the command window turn it off before we exit.      */

        if ( tt_status[VCMD] && vmode == VCMD ) {
            domac("_clear_statusline","set command statusline off",
                   cmdstk[cmdlvl].ccflgs);
            delmac("_clear_statusline",1);
            RequestScreenMutex(-1);
            VscrnIsDirty(vmode);
            ReleaseScreenMutex();
            while ( IsVscrnDirty(vmode) )
                msleep(200);
            RequestScreenMutex(-1);
            ReleaseScreenMutex();
        }
#endif /* KUI */
        DialerSend(OPT_KERMIT_EXIT,exitstat);
#ifndef KUI
        debug(F100,"doexit about to msleep","",0);

        if ( isWin95() )
            msleep(250);
#endif /* KUI */
    }
#endif /* OS2 */
#endif /* NOLOCAL */

#ifdef IKSD
#ifdef CK_LOGIN
    if (inserver && x_logged) {
#ifndef NOSPL
/*
  If a macro named "on_logout" is defined, execute it.  Also remove it from the
  macro table, in case its definition includes an EXIT or QUIT command, which
  would cause much recursion and would prevent the program from ever actually
  EXITing.
*/
	if (nmac) {			/* Any macros defined? */
	    int k;			/* Yes */
	    char * cmd = "on_logout";	/* MSVC 2.x compiler error */
	    k = mlook(mactab,cmd,nmac);	/* Look up "on_logout" */
	    if (k >= 0) {		/* If found, */
		*(mactab[k].kwd) = NUL;	/* poke its name from the table, */
		if (dodo(k,"",0) > -1)	/* set it up, */
		  parser(1);		/* and execute it */
	    }
	}
#endif /* NOSPL */
	zvlogout();
    }
#endif /* CK_LOGIN */
#endif /* IKSD */

    debug(F100,"doexit about to doclean","",0);
    doclean(1);                         /* Clean up most things */

#ifdef VMS
    if (code == -1)
      code = 0;                         /* Since we set two different items */
    sprintf(envstr,"%d", exitstat | code); /* SAFE */
    symval.dsc$w_length = (int)strlen(envstr);
    symval.dsc$a_pointer = envstr;
    symval.dsc$b_class = DSC$K_CLASS_S;
    symval.dsc$b_dtype = DSC$K_DTYPE_T;
    i = 2;                              /* Store in global table */
#ifdef COMMENT                          /* Martin Zinser */
    LIB$SET_SYMBOL(&symnam, &symval, &i);
#else
    lib$set_symbol(&symnam, &symval, &i);
#endif /* COMMENT */
    if (exitstat == BAD_EXIT)
      exitstat = SS$_ABORT | STS$M_INHIB_MSG;
    if (exitstat == GOOD_EXIT)
      exitstat = SS$_NORMAL | STS$M_INHIB_MSG;
#else /* Not VMS */
    if (code != -1)                     /* Take 1st arg literally */
      exitstat |= code;
#endif /* VMS */

#ifdef IKSD
#ifdef IKSDB
    debug(F101,"doexit ikdbopen","",ikdbopen);
    if (ikdbopen && dbfp) {             /* If IKSD database open */
        int x;
        x = freeslot(mydbslot);         /* Free our slot... */
        debug(F101,"doexit freeslot","",x);
        fclose(dbfp);                   /* and close it. */
    }
#endif /* IKSDB */
#endif /* IKSD */

/* We have put this off till the very last moment... */

#ifdef DEBUG
    if (deblog) {                       /* Close the debug log. */
        debug(F101,"C-Kermit EXIT status","",exitstat);
        *debfil = '\0';
        deblog = 0;
        zclose(ZDFILE);
    }
#endif /* DEBUG */

#ifdef OS2
    _exit(exitstat);            /* Exit from C-Kermit (no matter what) */
#else /* OS2 */
    exit(exitstat);                     /* Exit from C-Kermit */
#endif /* OS2 */
}

VOID
bgchk() {                               /* Check background status */
    if (bgset < 0) {                    /* They didn't type SET BACKGROUND */
#ifdef VMS                              /* Set prompt flag based on */
        pflag = !batch;                 /* what we detected at startup. */
#else
        pflag = !backgrd;
#endif /* VMS */
    } else {                            /* Otherwise SET BACKGROUND value */
        pflag = (bgset == 0 ? 1 : 0);
    }

#ifndef NOICP
    /* Message flag on only if at top level, pflag is on, and QUIET is OFF */
    if (!xcmdsrc)
      msgflg = (pflag == 0) ? 0 : !quiet;
    else msgflg = 0;
#else
    msgflg = 0;
#endif /* NOICP */
}

/* Set console interrupts */

VOID
setint() {                              /* According to SET COMMAND INTERRUP */
    int x = 0;
    if (cmdint)  x |= 1;
    if (xsuspend) x |= 2;
    debug(F101,"setint","",x);

    switch (x) {                        /* Set the desired combination */
      case 0: connoi(); break;          /* No interrupts */
      case 1: conint(trap,SIG_IGN); break;
      case 2: conint(SIG_IGN,stptrap); break;
      case 3: conint(trap,stptrap); break;
    }
    bgchk();                            /* Check background status */
}

#ifdef DEBUG
/*  D E B U G  --  Enter a record in the debugging log  */

/*
 Call with a format, two strings, and a number:
   f  - Format, a bit string in range 0-7.
        If bit x is on, then argument number x is printed.
   s1 - String, argument number 1.  If selected, printed as is.
   s2 - String, argument number 2.  If selected, printed in brackets.
   n  - Long int, argument 3.  If selected, printed preceded by equals sign.

   f=0 is special: print s1,s2, and interpret n as a char.

   f=F011 (3) is also special; in this case s2 is interpeted as a counted
   string that might contain NULs.  n is the length.  If n is negative, this
   means the string has been truncated and ".." should be printed after the
   first n bytes.  NUL and LF bytes are printed as "<NUL>" and "<LF>".

   Globals:
     deblog: nonzero if debug log open.
     debok:  nonzero if ok to write entries.
*/
/*
  WARNING: Don't change DEBUFL without changing sprintf() formats below,
  accordingly.
*/
#define DBUFL 4000
/*
  WARNING: This routine is not thread-safe, especially when Kermit is
  executing on multiple CPUs -- as different threads write to the same
  static buffer, the debug statements are all interleaved.  To be fixed
  later...
*/
static char *dbptr = (char *)0;

int
#ifdef CK_ANSIC
dodebug(int f, char *s1, char *s2, CK_OFF_T n)
#else
dodebug(f,s1,s2,n) int f; char *s1, *s2; CK_OFF_T n;
#endif /* CK_ANSIC */
/* dodebug */ {
    char *sp;
    int len1, len2;
    extern int debtim;
#ifdef OS2
    extern int SysInited;
#endif /* OS2 */

    if (!deblog || !debok)
      return(0);

#ifdef COMMENT
    /* expensive... */
    if (!chkfn(ZDFILE))			/* Debug log not open, don't. */
      return(0);
#endif /* COMMENT */
    if (!dbptr) {                       /* Allocate memory buffer */
        dbptr = malloc(DBUFL+4);        /* This only happens once */
        if (!dbptr) {
            zclose(ZDFILE);
            return(0);
        }
    }
/*
  This prevents infinite recursion in case we accidentally put a debug()
  call in this routine, or call another routine that contains debug() calls.
  From this point on, all returns from this return must be via goto xdebug,
  which sets deblog back to 1.
*/
#ifdef OS2
    if (SysInited) {
	if (RequestDebugMutex(30000))
	    goto xdebug;
    }
#else /* OS2 */
    deblog = 0;                         /* Prevent infinite recursion */
#endif /* OS2 */

    if (debtim) {                       /* Timestamp */
        char *tb, tsbuf[48];
        ztime(&tb);
        ckstrncpy(tsbuf,tb,32);
        if (ztmsec > -1L) {
	    sprintf(tsbuf+19,".%03ld ",ztmsec); /* SAFE */
	} else {
	    tsbuf[19] = ':';
	    tsbuf[20] = SP;
	    tsbuf[21] = NUL;
	}
        zsout(ZDFILE,tsbuf+11);
    }
    if (!s1) s1="(NULL)";
    if (!s2) s2="(NULL)";

    len1 = strlen(s1);
    len2 = strlen(s2);

#ifdef COMMENT
/*
  This should work, but it doesn't.
  So instead we'll cope with overflow via sprintf formats.
  N.B.: UNFORTUNATELY, this means we have to put constants in the
  sprintf formats.
*/
    if (f != F011 && (!f || (f & 6))) { /* String argument(s) included? */
        x = (int) strlen(s1) + (int) strlen(s2) + 18;
        if (x > dbufl) {                /* Longer than buffer? */
            if (dbptr)                  /* Yes, free previous buffer */
              free(dbptr);
            dbptr = (char *) malloc(x + 2); /* Allocate a new one */
            if (!dbptr) {
                zsoutl(ZDFILE,"DEBUG: Memory allocation failure");
                deblog = 0;
                zclose(ZDFILE);
                goto xdebug;
            } else {
                dbufl = x;
                sprintf(dbptr,"DEBUG: Buffer expanded to %d\n", x + 18);
                zsoutl(ZDFILE,dbptr);
            }
        }
    }
#endif /* COMMENT */

#ifdef COMMENT
/* The aforementioned sprintf() formats were like this: */
        if (n > 31 && n < 127)
          sprintf(sp,"%.100s%.2000s:%c\n",s1,s2,(CHAR) n);
        else if (n < 32 || n == 127)
          sprintf(sp,"%.100s%.2000s:^%c\n",s1,s2,(CHAR) ((n+64) & 0x7F));
        else if (n > 127 && n < 160)
          sprintf(sp,"%.100s%.2000s:~^%c\n",s1,s2,(CHAR)((n-64) & 0x7F));
        else if (n > 159 && n < 256)
          sprintf(sp,"%.100s%.2000s:~%c\n",s1,s2,(CHAR) (n & 0x7F));
        else sprintf(sp,"%.100s%.2000s:%ld\n",s1,s2,n);
/*
  But, naturally, it turns out these are not portable either, so now
  we do the stupidest possible thing.
*/
#endif /* COMMENT */

#ifdef BIGBUFOK
/* Need to accept longer strings when debugging authenticated connections */
    if (f == F010) {
        if (len2 + 2 >= DBUFL) s2 = "(string too long)";
    } else if (f != F011 && f != F100) {
        if (len1 > 100) s1 = "(string too long)";
        if (len2 + 101 >= DBUFL) s2 = "(string too long)";
    }
#else
    if (f != F011) {
        if (len1 > 100) s1 = "(string too long)";
        if (len2 + 101 >= DBUFL) s2 = "(string too long)";
    }
#endif /* BIGBUFOK */

    sp = dbptr;

    switch (f) {                /* Write log record according to format. */
      case F000:                /* 0 = print both strings, and n as a char. */
        if (len2 > 0) {
            if ((n > 31 && n < 127) || (n > 159 && n < 256))
              sprintf(sp,"%s[%s]=%c\n",s1,s2,(CHAR) n);
            else if (n < 32 || n == 127)
              sprintf(sp,"%s[%s]=^%c\n",s1,s2,(CHAR) ((n+64) & 0x7F));
            else if (n > 127 && n < 160)
              sprintf(sp,"%s[%s]=~^%c\n",s1,s2,(CHAR)((n-64) & 0x7F));
            else sprintf(sp,"%s[%s]=0x%lX\n",s1,s2,(long)n);
        } else {
            if ((n > 31 && n < 127) || (n > 159 && n < 256))
              sprintf(sp,"%s=%c\n",s1,(CHAR) n);
            else if (n < 32 || n == 127)
              sprintf(sp,"%s=^%c\n",s1,(CHAR) ((n+64) & 0x7F));
            else if (n > 127 && n < 160)
              sprintf(sp,"%s=~^%c\n",s1,(CHAR)((n-64) & 0x7F));
            else sprintf(sp,"%s=0x%lX\n",s1,(long)n);
        }
        if (zsout(ZDFILE,dbptr) < 0) {
            deblog = 0;
            zclose(ZDFILE);
        }
#ifdef CKSYSLOG
        if (ckxsyslog >= SYSLG_DB && ckxlogging) {
            cksyslog(SYSLG_DB,1,"debug",dbptr,NULL);
        }
#endif /* CKSYSLOG */
        break;

      case F001:                        /* 1, "=n" */
#ifdef COMMENT
        /* This was never used */
        sprintf(sp,"=%s\n",ckfstoa(n));
#else
        /* Like F111, but shows number n in hex */
	ckmakxmsg(sp,DBUFL,
		  s1,
		  (*s1 ? ":" : ""),
		  s2,
		  (*s2 ? ":" : ""),
		  ckltox(n),
		  "\n",
		  NULL,NULL,NULL,NULL,NULL,NULL
		  );
#endif /* COMMENT */
        if (zsout(ZDFILE,dbptr) < 0) {
            deblog = 0;
            zclose(ZDFILE);
        }
#ifdef CKSYSLOG
        if (ckxsyslog >= SYSLG_DB && ckxlogging) {
            cksyslog(SYSLG_DB,1,"debug",dbptr,NULL);
        }
#endif /* CKSYSLOG */
        break;

/*
  This one was never used so (October 2000) we now use it like F011,
  except in this case we treat s2 as NUL terminated.
*/
      case F010:
	n = -debxlen;
/*
  This one treats n as the length of the string s2, which may contain NULs.
  It's good for logging NUL-bearing data in the debug log.
*/
      case F011: {
	  int i, j, contd = 0;
	  char * p = s2, *pbuf = NULL;	/* p = source pointer */
	  int m;			/* pbuf = destination pointer */

	  if (f == F011) {
	      if (n < 0) {		/* n = size of source */
		  n = 0 - n;		/* Negative means to add "..." */
		  contd = 1;
	      }
	  } else {
	      int x, flag = 0;
	      x = strlen(s2);
	      if (n < 0) {
		  flag = 1;
		  n = 0 - n;
	      }
	      if (x < n)
		n = x;
	  }
	  if (n == 0)			/* 0 means do nothing */
	    goto xdebug;
	  m = DBUFL - 8;		/* Get size for interpreted part */
	  if (n > m)			/* Ensure requested size not too big */
	    n = m;
	  pbuf = dbptr;			/* Construction pointer */
	  i = 0;
	  pbuf[i++] = '[';		/* Interpret the string into it */
	  for (j = 0; j < n && i < m-4; p++,j++) { /* char by char... */
	      if (*p == LF) {
		  if (i >= m-4)
		    break;
		  pbuf[i++] = '<';
		  pbuf[i++] = 'L';
		  pbuf[i++] = 'F';
		  pbuf[i++] = '>';
		  continue;
	      } else if (*p == CR) {
		  if (i >= m-4)
		    break;
		  pbuf[i++] = '<';
		  pbuf[i++] = 'C';
		  pbuf[i++] = 'R';
		  pbuf[i++] = '>';
		  continue;
	      } else if (*p == HT) {
		  if (i >= m-5)
		    break;
		  pbuf[i++] = '<';
		  pbuf[i++] = 'T';
		  pbuf[i++] = 'A';
		  pbuf[i++] = 'B';
		  pbuf[i++] = '>';
		  continue;
	      } else if (*p) {
		  pbuf[i++] = *p;
		  continue;
	      } else {
		  if (i >= m-5)
		    break;
		  pbuf[i++] = '<';
		  pbuf[i++] = 'N';
		  pbuf[i++] = 'U';
		  pbuf[i++] = 'L';
		  pbuf[i++] = '>';
		  continue;
	      }
	  }
	  if (i < m-2 && (*p || contd)) {
	      pbuf[i++] = '.';
	      pbuf[i++] = '.';
	  }
	  pbuf[i++] = ']';
	  pbuf[i] = NUL;
	  if (zsout(ZDFILE,s1) < 0) {
	      deblog = 0;
	      zclose(ZDFILE);
	  }
	  if (zsoutl(ZDFILE,pbuf) < 0) {
	      deblog = 0;
	      zclose(ZDFILE);
	  }
#ifdef CKSYSLOG
	  if (ckxsyslog >= SYSLG_DB && ckxlogging) {
	      cksyslog(SYSLG_DB,1,"debug",s1,pbuf);
	  }
#endif /* CKSYSLOG */
        }
        break;

      case F100:                        /* 4, "s1" */
        if (zsoutl(ZDFILE,s1) < 0) {
            deblog = 0;
            zclose(ZDFILE);
        }
#ifdef CKSYSLOG
        if (ckxsyslog >= SYSLG_DB && ckxlogging) {
            cksyslog(SYSLG_DB,1,"debug",s1,NULL);
        }
#endif /* CKSYSLOG */
        break;
      case F101:                        /* 5, "s1=n" */
        sprintf(sp,"%s=%s\n",s1,ckfstoa(n));
        if (zsout(ZDFILE,dbptr) < 0) {
            deblog = 0;
            zclose(ZDFILE);
        }
#ifdef CKSYSLOG
        if (ckxsyslog >= SYSLG_DB && ckxlogging) {
            cksyslog(SYSLG_DB,1,"debug",dbptr,NULL);
        }
#endif /* CKSYSLOG */
        break;
      case F110:                        /* 6, "s1[s2]" */
        sprintf(sp,"%s[%s]\n",s1,s2);
        if (zsout(ZDFILE,dbptr) < 0) {
            deblog = 0;
            zclose(ZDFILE);
        }
#ifdef CKSYSLOG
        if (ckxsyslog >= SYSLG_DB && ckxlogging) {
            cksyslog(SYSLG_DB,1,"debug",dbptr,NULL);
        }
#endif /* CKSYSLOG */
        break;
      case F111:                        /* 7, "s1[s2]=n" */
        sprintf(sp,"%s[%s]=%s\n",s1,s2,ckfstoa(n));
        if (zsout(ZDFILE,dbptr) < 0) {
            deblog = 0;
            zclose(ZDFILE);
        }
#ifdef CKSYSLOG
        if (ckxsyslog >= SYSLG_DB && ckxlogging) {
            cksyslog(SYSLG_DB,1,"debug",dbptr,NULL);
        }
#endif /* CKSYSLOG */
        break;
      default:
        sprintf(sp,"\n?Invalid format for debug() - %d\n",f);
        if (zsout(ZDFILE,dbptr) < 0) {
            deblog = 0;
            zclose(ZDFILE);
        }
#ifdef CKSYSLOG
        if (ckxsyslog >= SYSLG_DB && ckxlogging) {
            cksyslog(SYSLG_DB,1,"debug",dbptr,NULL);
        }
#endif /* CKSYSLOG */
        break;
    }
  xdebug:                               /* Common exit point */
#ifdef OS2
    if (SysInited)
	ReleaseDebugMutex();
#else /* OS2 */
    deblog = 1;                         /* Restore this */
#endif /* OS2 */
    return(0);
}

int
#ifdef CK_ANSIC
dohexdump(CHAR *msg, CHAR *st, int cnt)
#else
dohexdump(msg,st,cnt) CHAR *msg; CHAR *st; int cnt;
#endif /* CK_ANSIC */
/* dohexdump */ {
    int i = 0, j = 0, k = 0;
    char tmp[8];
#ifdef OS2
    extern int SysInited;
#endif /* OS2 */

    if (!deblog) return(0);		/* If no debug log, don't. */
    if (!dbptr) {                       /* Allocate memory buffer */
        dbptr = malloc(DBUFL+1);        /* This only happens once */
        if (!dbptr) {
            deblog = 0;
            zclose(ZDFILE);
            return(0);
        }
    }

#ifdef OS2
    if (SysInited) {
	if (RequestDebugMutex(30000))
	    goto xdebug;
    }
#else /* OS2 */
    deblog = 0;                         /* Prevent infinite recursion */
#endif /* OS2 */

    if (msg != NULL) {
	ckmakxmsg(dbptr,
		  DBUFL,
		  "HEXDUMP: ",
		  (char *)msg,
		  " (",
		  ckitoa(cnt),
		  " bytes)\n",
		  NULL,NULL,NULL,NULL,NULL,NULL,NULL
		 );
        if (zsout(ZDFILE,dbptr) < 0) {
            deblog = 0;
            zclose(ZDFILE);
	    goto xdebug;
        }
    } else {
	ckmakmsg(dbptr,
		 DBUFL,
		 "HEXDUMP: (",
		 ckitoa(cnt),
		 " bytes)\n",
		 NULL
		 );
        zsout(ZDFILE,dbptr);
        if (zsout(ZDFILE,dbptr) < 0) {
            deblog = 0;
            zclose(ZDFILE);
	    goto xdebug;
        }
    }
    for (i = 0; i < cnt; i++) {
        dbptr[0] = '\0';
        for (j = 0 ; (j < 16); j++) {
            if ((i + j) < cnt)
	      sprintf(tmp,
		      "%s%02x ",
		      (j == 8 ? "| " : ""),
		      (CHAR) st[i + j]
		      );
            else
	      sprintf(tmp,
		      "%s   ",
		      (j == 8 ? "| " : "")
		      );
            ckstrncat(dbptr,tmp,DBUFL+1);
        }
        ckstrncat(dbptr," ",DBUFL+1);
        for (k = 0; (k < 16) && ((i + k) < cnt); k++) {
            sprintf(tmp,
                    "%s%c",
                    (k == 8 ? " " : ""),
                    isprint(st[i + k]) ? st[i + k] : '.'
                    );
            ckstrncat(dbptr,tmp,DBUFL+1);
        }
        ckstrncat(dbptr,"\n",DBUFL+1);
        i += j - 1;
        if (zsout(ZDFILE,dbptr) < 0) {
            deblog = 0;
            zclose(ZDFILE);
	    goto xdebug;
        }
    } /* end for */


  xdebug:
#ifdef OS2
    if (SysInited)
      ReleaseDebugMutex();
#else /* OS2 */
    deblog = 1;
#endif /* OS2 */
    return(0);
}
#endif /* DEBUG */

/*  Session Log... */

int tsstate = 0;

VOID
#ifdef OS2
logchar(unsigned short c)
#else /* OS2 */
#ifdef CK_ANSIC
logchar(char c)
#else
logchar(c) char c;
#endif /* CK_ANSIC */
#endif /* OS2 */
/* logchar */ {                         /* Log character c to session log */
    extern int slognul;
    int oktolog = 0;
#ifndef NOLOCAL
    if (!seslog)
      return;

    if ((sessft != XYFT_T) || (
#ifdef UNIX
	 c != '\r' &&
#else
#ifdef datageneral
	 c != '\r' &&
#else
#ifdef STRATUS
	 c != '\r' &&
#else
#ifdef AMIGA
	 c != '\r' &&
#else
#ifdef GEMDOS
	 c != '\r' &&
#endif /* GEMDOS */
#endif /* AMIGA */
#endif /* STRATUS */
#endif /* datageneral */
#endif /* UNIX */
#ifdef OSK
	 c != '\n' &&
#else
#ifdef MAC
	 c != '\n' &&
#endif /* MAC */
#endif /* OSK */
	 c != XON &&
	 c != XOFF))
      oktolog = 1;
    if (c == '\0' && !sessft)		/* NUL in text mode */
      if (slognul) oktolog = 1;		/* only if padding (2009/10/22) */
    if (!oktolog)
      return;
    if (slogts) {			/* Log is timestamped */
	if (tsstate == 0) {		/* State = between-lines */
	    char * p;			/* zstime() pointer */
	    char ts[48];		/* timestamp buffer */
	    ztime(&p);			/* Get asctime() string */
	    ckstrncpy(ts,p,32);		/* Make safe copy */
	    if (ztmsec > -1L) {		/* Add msecs if we have them */
		sprintf(&ts[19],".%03ld: ",ztmsec); /* SAFE */
	    } else {
		ts[19] = ':';
		ts[20] = SP;
		ts[21] = NUL;
	    }
	    if (zsout(ZSFILE,&ts[11]) < 0)
	      goto xlogchar;
	}
    }
    if (c == '\n')			/* At end of line? */
      tsstate = 0;			/* yes */
    else
      tsstate = 1;			/* no */
    if (zchout(ZSFILE,(CHAR)(c & 0xFF)) < 0) /* Log the character */
      goto xlogchar;
    if (tsstate == 0 && slognul != 0) {	/* Null-terminating lines? */
	if (zchout(ZSFILE,(CHAR)0) < 0)	/* Add a NUL */
	  goto xlogchar;
    }
    return;

  xlogchar:
    conoll("");
    conoll("ERROR WRITING SESSION LOG, LOG CLOSED!");
    setseslog(0);
    zclose(ZSFILE);
#endif /* NOLOCAL */
}

VOID
logstr(s, len) char * s; int len; {     /* Log string to session log */
#ifndef NOLOCAL
    int n = 0;
    if (!s)
      return;
    while (seslog && (n < len))
      logchar(s[n++]);
#endif /* NOLOCAL */
}

#ifdef CK_CURSES
int
ck_repaint() {
    repaint = 1;
    return(0);
}

#ifdef STRATUS
/* VOS has curses but no tgetent() */
int
tgetent(s1, s2) char * s1, * s2; {
    return(1);
}
#endif /* STRATUS */

#ifdef VMS
#ifdef __DECC
_PROTOTYP(int tgetent,(char *, char *));
#endif /* __DECC */
#endif /* VMS */

/*
  There are three different ways to do fullscreen on VMS.
  1. Use the real curses library, VAXCCURSE.
  2. Use do-it-yourself code.
  3. Use the Screen Manager, SMG$.

  Method 1 doesn't work quite right; you can't call endwin(), so once you've
  started curses mode, you can never leave.

  Method 2 doesn't optimize the screen, and so much more time is spent in
  screen writes.  This actually causes file transfers to fail because the
  tty device input buffer can be overrun while the screen is being updated,
  especially on a slow MicroVAX that has small typeahead buffers.

  In the following #ifdef block, #define one of them and #undef the other 2.

  So now let's try method 3...
*/
#ifdef VMS
#define CK_SMG                          /* Screen Manager */
#undef MYCURSES                         /* Do-it-yourself */
#undef VMSCURSE                         /* VAXCCURSE library */
#endif /* VMS */
/*
  But just before New Years, 2000, the SMG library seemed to break on
  both VMS systems we have here (an Alpha with VMS 7.1 and a VAX with 5.5).
  So back to MYCURSES, which works fine.
*/
#ifdef VMS
#undef CK_SMG
#define MYCURSES
#endif /* VMS */

#ifdef MYCURSES
#define stdscr 0
#ifdef CK_WREFRESH
#undef CK_WREFRESH
#endif /* CK_WREFRESH */
#endif /* MYCURSES */

/*  S C R E E N C  --  Screen display function, uses curses  */

/* Idea for curses display contributed by Chris Pratt of APV Baker, UK */

/* Avoid conficts with curses.h */

#ifdef QNX
/* Same as ckcasc.h, but in a different radix... */
#ifdef ESC
#undef ESC
#endif /* ESC */
#endif /* QNX */

#ifndef MYCURSES
#undef VOID                             /* This was defined in ckcdeb.h */
#endif /* MYCURSES */

#undef BS                               /* These were defined in ckcasc.h */
#undef CR
#undef NL
#undef SO
#ifdef US
#undef US
#endif	/* US */
#undef SP                               /* Used in ncurses */
#define CHR_SP 32                       /* Use this instead */

#ifdef VMS                              /* VMS fullscreen display */
#ifdef MYCURSES                         /* Do-it-yourself method */
extern int isvt52;                      /* From CKVTIO.C */
#define printw printf
#else
#ifdef VMSCURSE                         /* VMS curses library VAXCCURSE */
#include <curses.h>
/* Note: Screen manager doesn't need a header file */
#endif /* VMSCURSE */
#endif /* MYCURSES */
#else                                   /* Not VMS */
#ifdef MYCURSES                         /* Do-it-yourself method */
#define isvt52 0                        /* Used by OS/2, VT-100/ANSI always */
#ifdef CKXPRINTF
#define printw ckxprintf
#else /* CKXPRINTF */
#ifdef KUI
#define printw Vscrnprintw
#else /* KUI */
#define printw printf
#endif /* KUI */
#endif /* CKXPRINTF */
#else                                   /* Use real curses */
#ifdef CK_NCURSES                       /* or ncurses... */
#ifdef CKXPRINTF                        /* Our printf macro conflicts with */
#undef printf                           /* use of "printf" in ncurses.h */
#endif /* CKXPRINTF */
#include <ncurses.h>
#ifdef CKXPRINTF
#define printf ckxprintf
#endif /* CKXPRINTF */
#else  /* Not ncurses */
#ifdef CKXPRINTF                        /* Our printf macro conflicts with */
#undef printf                           /* use of "printf" in curses.h */
#endif /* CKXPRINTF */
#ifdef M_XENIX				/* SCO XENIX... */
#ifdef M_TERMCAP
#undef M_TERMCAP
#endif /* M_TERMCAP */
#ifndef M_TERMINFO
#define M_TERMINFO
#endif /* M_TERMINFO */
#endif /* M_XENIX */
#ifdef RTAIX
#undef NLS				/* Avoid 'redeclaration of free'. */
#endif /* RTAIX */
#include <curses.h>
#ifdef CKXPRINTF
#define printf ckxprintf
#endif /* CKXPRINTF */
#endif /* CK_NCURSES */
#endif /* MYCURSES */
#endif /* VMS */

#ifdef BUG999
_PROTOTYP(int tgetent,(char *, char *));
_PROTOTYP(char *tgetstr,(char *, char **));
_PROTOTYP(int tputs,(char *, int, int (*)()));
_PROTOTYP(char *tgoto,(const char *, int, int));
#endif	/* BUG999 */

#endif /* CK_CURSES */

/*  F X D I N I T  --  File Xfer Display Initialization  */

#ifdef CK_CURSES
#ifndef MYCURSES
#ifndef CK_SMG
static
#ifdef CK_ANSIC
/* Can't use VOID because of curses.h */
void
ck_termset(int);
#else
ck_termset();
#endif /* CK_ANSIC */
#endif /* CK_SMG */
#endif /* MYCURSES */
#endif /* CK_CURSES */

#ifdef NOTERMCAP
static int notermcap = 1;
#else
static int notermcap = 0;
#endif /* NOTERMCAP */

#ifndef NODISPLAY
CKVOID
fxdinit(xdispla) int xdispla; {
#ifndef COHERENT
#ifndef OS2
#ifndef STRATUS
    char *s;
    int x, dummy;

    debug(F101,"fxdinit xdispla","",xdispla);
    debug(F101,"fxdinit fxd_inited","",fxd_inited);

#ifdef IKSD
#ifndef NOXFER
    /* No curses for IKSD */
    if (inserver) {
        fdispla = XYFD_N;
        return;
    }
    if (fxd_inited)                     /* Only do this once */
      return;
#endif /* NOXFER */
#endif /* IKSD */

    if (xdispla == XYFD_R || xdispla == XYFD_S || xdispla == XYFD_B) {
	if (xfrmsg) {
	    printf("%s\n",xfrmsg);
	    makestr(&xfrmsg,NULL);
	}
    }

#ifdef CK_CURSES
#ifdef VMS
    /* Force BRIEF in Batch logs */
    if (batch && (xdispla == XYFD_C || xdispla == XYFD_S))
      xdispla = XYFD_B;
#else
    if (xdispla == XYFD_C || xdispla == 9999) {

#ifdef DYNAMIC
        if (!trmbuf) {
/*
  Allocate tgetent() buffer.  Make it big -- some termcaps can be huge;
  tgetent() merrily writes past the end of the buffer, causing core dumps
  or worse.
*/
            trmbuf = (char *)malloc(TRMBUFL);
            if (!trmbuf) {
                notermcap = 1;
                debug(F101,"fxdinit malloc trmbuf","FAILED",TRMBUFL);
                fdispla = XYFD_S;
                return;
            }
#ifdef COMMENT
            debug(F111,"fxdinit malloc trmbuf","OK",TRMBUFL);
            debug(F001,"fxdinit trmbuf","",trmbuf);
            memset(trmbuf,'\0',(size_t)TRMBUFL);
            debug(F100,"fxdinit memset OK","",0);
#endif /* COMMENT */
        }
#endif /* DYNAMIC */

        debug(F100,"fxdinit before getenv(TERM)","",0);
        s = getenv("TERM");
        debug(F110,"fxdinit after getenv(TERM)",s,0);
        if (!s) s = "";
        if (*s) {
            debug(F110,"fxdinit before tgetent()",s,0);
            x = tgetent(trmbuf,s);
            debug(F111,"fxdinit tgetent",s,x);
        } else {
            x = 0;
            notermcap = 1;
            debug(F110,"fxdinit TERM null - no tgetent",s,0);
        }
        if (x < 1 && !quiet && !backgrd
#ifdef VMS
            && !batch
#endif /* VMS */
            ) {
            printf("Warning: terminal type unknown: \"%s\"\n",s);
#ifdef COMMENT
	    /* Confusing - nobody knows what this means */
            printf("SCREEN command will use ANSI sequences.\n");
#endif /* COMMENT */
            if (local)
              printf("Fullscreen file transfer display disabled.\n");
            fdispla = XYFD_S;
        }
#ifndef MYCURSES
#ifndef CK_SMG
        ck_termset(x);
#endif /* CK_SMG */
#endif /* MYCURSES */
        fxd_inited = 1;
    }
#endif /* CK_CURSES */
#endif /* VMS */
#endif /* STRATUS */
#endif /* OS2 */
#endif /* COHERENT */
}
#endif /* NODISPLAY */

#ifdef CK_CURSES
#ifdef CK_SMG
/*
  Long section for Screen Manager starts here...
  By William Bader.
*/
#include "ckvvms.h"
#ifdef OLD_VMS
#include <smgdef.h>                     /* use this on VAX C 2.4 */
/* #include <smgmsg.h> */
#else
#include <smg$routines.h>               /* Martin Zinser */
#endif /* OLD_VMS */

extern unsigned int vms_status;     /* Used for system service return status */

static long smg_pasteboard_id = -1;     /* pasteboard identifier */
static long smg_display_id = -1;        /* display identifier */
static int smg_open = 0;                /* flag if smg current open */
static int smg_inited = 0;              /* flag if smg initialized */

#ifdef COMMENT
#define clrtoeol()      SMG$ERASE_LINE(&smg_display_id, 0, 0)

#define clear()         SMG$ERASE_DISPLAY(&smg_display_id, 0, 0, 0, 0)

#define touchwin(scr)   SMG$REPAINT_SCREEN(&smg_pasteboard_id)

#else  /* Not COMMENT */

#define clrtoeol()      smg$erase_line(&smg_display_id, 0, 0)

#define clear()         smg$erase_display(&smg_display_id, 0, 0, 0, 0)

#define touchwin(scr)   smg$repaint_screen(&smg_pasteboard_id)
#endif /* COMMENT */

#define clearok(curscr,ok)              /* Let wrefresh() do the work */

#define wrefresh(cursrc) touchwin(scr)

static void
move(row, col) int row, col; {
    /* Change from 0-based for curses to 1-based for SMG */
    if (!smg_open)
      return;
    ++row; ++col;
    debug(F111,"VMS smg move",ckitoa(row),col);
#ifdef COMMENT                          /* Martin Zinser */
    CHECK_ERR("move: smg$set_cursor_abs",
              SMG$SET_CURSOR_ABS(&smg_display_id, &row, &col));
#else
    CHECK_ERR("move: smg$set_cursor_abs",
              smg$set_cursor_abs(&smg_display_id, &row, &col));
#endif /* COMMENT */
    debug(F101,"VMS smg move vms_status","",vms_status);
}

#ifdef VMS_V40
#define OLD_VMS
#endif /* VMS_V40 */
#ifdef VMS_V42
#define OLD_VMS
#endif /* VMS_V42 */
#ifdef VMS_V44
#define OLD_VMS
#endif /* VMS_V44 */

static int
initscr() {
    int rows = 24, cols = 80;
    int row = 1, col = 1;

    debug(F101,"VMS initscr smg_pasteboard_id A","",smg_pasteboard_id);

    if (smg_pasteboard_id == -1) { /* Open the screen */
#ifdef OLD_VMS                     /* Note: Routine calls lowercased 9/96 */
        CHECK_ERR("initscr: smg$create_pasteboard",
                  smg$create_pasteboard(&smg_pasteboard_id, 0, 0, 0, 0));
#else
        /* For VMS V5, not tested */
        CHECK_ERR("initscr: smg$create_pasteboard",
                  smg$create_pasteboard(&smg_pasteboard_id, 0, 0, 0, 0, 0));
#endif /* OLD_VMS */
    }
    debug(F101,"VMS initscr smg_pasteboard_id B","",smg_pasteboard_id);
    if (smg_pasteboard_id == -1) {
	printf("?Error initializing fullscreen display\n");
	fdispla = XYFD_S;
	dpyinit();
	return(0);
    }
    debug(F101,"VMS initscr smg_display_id","",smg_display_id);
    if (smg_display_id == -1) {         /* Create a display window */

#ifdef COMMENT                          /* Martin Zinser */
        CHECK_ERR("initscr: smg$create_virtual_display",
                  SMG$CREATE_VIRTUAL_DISPLAY(&rows, &cols, &smg_display_id,
                                             0, 0, 0));

        /* Connect the display window to the screen */
        CHECK_ERR("initscr: smg$paste_virtual_display",
                  SMG$PASTE_VIRTUAL_DISPLAY(&smg_display_id,&smg_pasteboard_id,
                                            &row,&col));
#else
        CHECK_ERR("initscr: smg$create_virtual_display",
                  smg$create_virtual_display(&rows, &cols, &smg_display_id,
                                             0, 0, 0));

        /* Connect the display window to the screen */
        CHECK_ERR("initscr: smg$paste_virtual_display",
                  smg$paste_virtual_display(&smg_display_id,&smg_pasteboard_id,
                                            &row,&col));
#endif /* COMMENT */
    }
    debug(F101,"VMS initscr smg_open A","",smg_open);
    if (!smg_open) {                    /* Start a batch update */
        smg_open = 1;
#ifdef COMMENT
        CHECK_ERR("initscr: smg$begin_pasteboard_update",
                  SMG$BEGIN_PASTEBOARD_UPDATE(&smg_pasteboard_id));
#else
        CHECK_ERR("initscr: smg$begin_pasteboard_update",
                  smg$begin_pasteboard_update(&smg_pasteboard_id));
#endif /* COMMENT */
	debug(F101,"VMS initscr smg$begin_pasteboard_update","",vms_status);
    }
    debug(F101,"VMS initscr smg_open B","",smg_open);
    smg_inited = 1;
    return(1);
}

static void
refresh() {
    debug(F101,"refresh smg_pasteboard_id","",smg_pasteboard_id);

    if (smg_open == 0 || smg_pasteboard_id == -1)
      return;

#ifdef COMMENT                          /* Martin Zinser */
    CHECK_ERR("refresh: smg$end_pasteboard_update",
              SMG$END_PASTEBOARD_UPDATE(&smg_pasteboard_id));
    CHECK_ERR("refresh: smg$begin_pasteboard_update",
              SMG$BEGIN_PASTEBOARD_UPDATE(&smg_pasteboard_id));
#else
    CHECK_ERR("refresh: smg$end_pasteboard_update",
              smg$end_pasteboard_update(&smg_pasteboard_id));
    CHECK_ERR("refresh: smg$begin_pasteboard_update",
              smg$begin_pasteboard_update(&smg_pasteboard_id));
#endif /* COMMENT */
}

static void
endwin() {
    if (!smg_open)
      return;

    smg_open = 0;

#ifdef COMMENT
    CHECK_ERR("endwin: smg$end_pasteboard_update",
              SMG$END_PASTEBOARD_UPDATE(&smg_pasteboard_id));
#else
    CHECK_ERR("endwin: smg$end_pasteboard_update",
              smg$end_pasteboard_update(&smg_pasteboard_id));
#endif /* COMMENT */

    move(22, 0);

#ifdef COMMENT
/*
  These calls clear the screen.
  (convert routine calls to lowercase - Martin Zinser)
*/
    CHECK_ERR("endwin: smg$delete_virtual_display",
              SMG$DELETE_VIRTUAL_DISPLAY(&smg_display_id));
    smg_display_id = -1;

    CHECK_ERR("endwin: smg$delete_pasteboard",
              SMG$DELETE_PASTEBOARD(&smg_pasteboard_id, 0));
    smg_pasteboard_id = -1;
#endif /* COMMENT */
}

#ifdef COMMENT
/* DECC 6.2 screams bloody murder about printw ("not enough args") */
/* but adding the following prototype only makes it holler louder. */
#ifdef __DECC
/* "varargs" prototype for printw */
_PROTOTYP(static int printw,(char *, ...));
#endif /* __DECC */
#endif /* COMMENT */

#ifdef __DECC
#include <stdarg.h>
_PROTOTYP(static void printw,(char *, ...));
static void
printw(char *str,...) {
    char buf[255];
    va_list ap;
    $DESCRIPTOR(text_dsc, 0);
    text_dsc.dsc$a_pointer=buf;
    if (!smg_open)
      return;
    va_start(ap,str);
    text_dsc.dsc$w_length = vsprintf(buf, str, ap);
    va_end(ap);
    CHECK_ERR("printw: smg$put_chars",
              smg$put_chars(&smg_display_id, &text_dsc, 0, 0, 0, 0, 0));
}
#else
static void
printw(str, a1, a2, a3, a4, a5, a6, a7, a8)
    char *str;
    long a1, a2, a3, a4, a5, a6, a7, a8;
/* printw */ {
    char buf[255];
    $DESCRIPTOR(text_dsc, 0);
    if (!smg_open)
      return;
    text_dsc.dsc$a_pointer=buf;
    text_dsc.dsc$w_length = sprintf(buf, str, a1, a2, a3, a4, a5, a6, a7, a8);
    CHECK_ERR("printw: smg$put_chars",
              smg$put_chars(&smg_display_id, &text_dsc, 0, 0, 0, 0, 0));
}
#endif /* __DECC */

#define CK_CURPOS
int
ck_curpos(row, col) {
    debug(F111,"VMS smg ck_curpos",ckitoa(row),col);
    if (!smg_inited || !smg_open) {
        initscr();
    }
    debug(F101,"VMS smg curpos smg_open","",smg_open);
    if (!smg_open)
      return(0);
    debug(F111,"VMS smg ck_curpos",ckitoa(row-1),col-1);
    move(row - 1, col - 1);             /* SMG is 0-based */
    refresh();
    /* endwin(); */
    return(0);
}

int
ck_cls() {
    debug(F101,"VMS smg ck_cls smg_inited","",smg_inited);
    if (!smg_inited || !smg_open) {
        initscr();
    }
    debug(F101,"VMS smg ck_cls smg_open","",smg_open);
    if (!smg_open)
      return(0);
    clear();
    refresh();
    /* endwin(); */
    return(0);
}

int
ck_cleol() {
    debug(F101,"VMS smg ck_cleol smg_inited","",smg_inited);
    if (!smg_inited || !smg_open) {
        initscr();
    }
    debug(F101,"VMS smg ck_cleol smg_open","",smg_open);
    if (!smg_open)
      return(0);
    clrtoeol();
    refresh();
    /* endwin(); */
    return(0);
}
#endif /* CK_SMG */

#ifdef MYCURSES
/*
  Do-it-yourself curses implementation for VMS, OS/2 and other ANSI/VT-100's.
  Supports only the VT52 and VT1xx (and later VT2xx/3xx/4xx) terminals.
  By Terry Kennedy, St Peters College.

  First, some stuff we can just ignore:
*/

static int
touchwin(x) int x; {
    return(0);
}
static int
initscr() {
    return(0);
}
static int
refresh() {
    return(0);
}
static int
endwin() {
    return(0);
}

/*
 * Now, some stuff we need to do:
 */

_PROTOTYP( int move, (int, int) );
#ifndef OS2
int
move(row, col) int row, col; {
    if (isvt52)
      printf("\033Y%c%c", row + 037, col + 037);
    else
      printf("\033[%d;%dH", row + 1, col + 1);
    return(0);
}

int
clear() {
    move(0,0);
    if (isvt52)
      printf("\033J");
    else
      printf("\033[J");
    return(0);
}

int
clrtoeol() {
    if (isvt52)
      printf("\033K");
    else
      printf("\033[K");
    return(0);
}

#define CK_CURPOS
int
ck_cls() {
    return(clear());
}

int
ck_cleol() {
    return(clrtoeol());
}

int
ck_curpos(row, col) int row, col; {
    move(row, col);
    return(0);
}

#else /* OS2 */
/* Windows NT and Windows 95 do not provide ANSI emulation */
/* Therefore we might as well not use it for OS/2 either   */

int
move(row, col) int row, col; {
#ifndef ONETERMUPD
    SetCurPos(row, col);
#endif /* ONETERMUPD */
    lgotoxy( VCMD, col+1, row+1);
    VscrnIsDirty(VCMD);
    return(0);
}

int
clear() {
    viocell cell;
    move(0,0);
#ifdef ONETERMUPD
    if (VscrnGetBufferSize(VCMD) > 0) {
        VscrnScroll(VCMD, UPWARD, 0,
                    VscrnGetHeight(VCMD)-(1),
                    VscrnGetHeight(VCMD)-(0), TRUE, CHR_SP);
        cleartermscreen(VCMD);
    }
#else
    cell.c = ' ';
    cell.a = colorcmd;
    WrtNCell(cell, cmd_rows * cmd_cols, 0, 0);
#endif /* ONETERMUPD */
    return(0);
}

int
clrtoeol() {
    USHORT row, col;
    viocell cell;

    cell.c = ' ';
    cell.a = colorcmd;
#ifndef ONETERMUPD
    GetCurPos(&row, &col );
    WrtNCell(cell, cmd_cols - col -1, row, col);
#endif /* ONETERMUPD */
    clrtoeoln(VCMD,CHR_SP);
    return(0);
}

#define CK_CURPOS
int
ck_curpos(row, col) int row, col; {
    move(row, col);
    return(0);
}

int
ck_cls() {
    return(clear());
}

int
ck_cleol() {
    return(clrtoeol());
}

#endif /* OS2 */
#endif /* MYCURSES */

#ifndef NOTERMCAP
#ifndef CK_CURPOS
#define CK_CURPOS

/* Termcap/Terminfo section */

static char cur_cls[32] = { NUL, NUL };
static char cur_cleol[32] = { NUL, NUL };
static char cur_cm[64] = { NUL, NUL };
static char tgsbuf[128] = { NUL, NUL };

static
#ifdef CK_ANSIC
void
#endif /* CK_ANSIC */
ck_termset(x) int x; {
    cur_cls[0] = NUL;
    cur_cleol[0] = NUL;
    cur_cm[0] = NUL;
#ifdef tgetent
    debug(F100,"tgetent is a macro","",0);
#endif /* tgetent */
#ifdef tgetstr
    debug(F100,"tgetstr is a macro","",0);
#endif /* tgetstr */
#ifdef tputs
    debug(F100,"tputs is a macro","",0);
#endif /* tputs */
#ifdef tgoto
    debug(F100,"tgoto is a macro","",0);
#endif /* tgoto */
#ifdef NOTERMCAP
    /* tgetstr() gets a segmentation fault on OSF/1 */
    debug(F100,"ck_termset NOTERMCAP","",0);
#else
    if (notermcap) {
        debug(F100,"ck_termset notermcap","",0);
        return;
    }
    debug(F101,"ck_termset x","",x);
    if (x > 0) {
        char * bp;
        bp = tgsbuf;
        *bp = NUL;
        debug(F110,"ck_termset calling tgetstr","cl",0);
        if (tgetstr("cl", &bp)) {       /* Get clear-screen code */
            debug(F110,"ck_termset tgetstr cl",tgsbuf,"");
            if ((int)strlen(tgsbuf) < 32)
              ckstrncpy(cur_cls,tgsbuf,32);
        } else
          return;
        bp = tgsbuf;
        if (tgetstr("ce", &bp)) {       /* Get clear-to-end-of-line code */
            debug(F110,"ck_termset tgetstr ce",tgsbuf,"");
            if ((int)strlen(tgsbuf) < 32)
              ckstrncpy(cur_cleol,tgsbuf,32);
        } else
          return;
        bp = tgsbuf;
        if (tgetstr("cm", &bp)) {       /* Get cursor-movement code */
            debug(F110,"ck_termset tgetstr cm",tgsbuf,"");
            if ((int)strlen(tgsbuf) < 64)
              ckstrncpy(cur_cm,tgsbuf,64);
        } else
          return;
    }
#endif /* NOTERMCAP */
}

#ifndef TPUTSFNTYPE
#ifdef TPUTSISVOID
#define TPUTSFNTYPE void
#else
#define TPUTSFNTYPE int
#endif /* TPUTSISVOID */
#endif /* TPUTSFNTYPE */

#ifndef TPUTSARGTYPE
#ifdef HPUX9
#define TPUTSARGTYPE char
#else
#ifdef HPUX10
#define TPUTSARGTYPE char
#else
#define TPUTSARGTYPE int
#endif /* HPUX10 */
#endif /* HPUX9 */
#endif /* TPUTSARGTYPE */

static TPUTSFNTYPE
#ifdef CK_ANSIC
ck_outc(TPUTSARGTYPE x)
#else
ck_outc(x) TPUTSARGTYPE x;
#endif /* CK_ANSIC */
{                                       /* To satisfy tputs() arg3 prototype */
    int rc;
    char c;
    c = (char) x;
    rc = (inserver) ? ttoc(c) : conoc(c);
#ifndef TPUTSISVOID
    return(rc);
#endif /* TPUTSISVOID */
}

int
ck_curpos(row, col) int row, col; {
#ifdef CK_ANSIC
    TPUTSFNTYPE (*fn)(TPUTSARGTYPE);
#else
    TPUTSFNTYPE (*fn)();
#endif /* CK_ANSIC */
    if (!fxd_inited)
      fxdinit(9999);
    if (!cur_cm[0]) {                   /* We don't have escape sequences */
#ifdef COMMENT
        return(-1);                     /* Do nothing */
#else
        /* Both C-Kermit's SCREEN command and ANSI/VT100 are 1-based */
        printf("\033[%d;%dH", row, col); /* Or default to ANSI */
#endif /* COMMENT */
    } else {
        fn = ck_outc;
        /* termcap/terminfo is 0-based */
        tputs(
#ifdef TPUTSARG1CONST
              (const char *)
#endif /* TPUTSARG1CONST */
              tgoto(cur_cm,col-1,row-1),1,fn);
    }
    return(0);
}

int
ck_cls() {
#ifdef CK_ANSIC
    TPUTSFNTYPE (*fn)(TPUTSARGTYPE);
#else
    TPUTSFNTYPE (*fn)();
#endif /* CK_ANSIC */
    if (!fxd_inited)
      fxdinit(9999);
    if (!cur_cls[0]) {                  /* If we don't have escape sequences */
#ifdef COMMENT
        return(-1);                     /* Do nothing */
#else
        printf("\033[;H\033[2J");       /* Or default to ANSI */
#endif /* COMMENT */
    } else {
        fn = ck_outc;
        debug(F111,"ck_cls 2",cur_cls,fxd_inited);
        tputs(cur_cls,cmd_rows,fn);
    }
    return(0);
}

int
ck_cleol() {
#ifdef CK_ANSIC
    TPUTSFNTYPE (*fn)(TPUTSARGTYPE);
#else
    TPUTSFNTYPE (*fn)();
#endif /* CK_ANSIC */
    if (!fxd_inited)
      fxdinit(9999);
    if (!cur_cleol[0]) {                /* If we don't have escape sequences */
#ifdef COMMENT
        return(-1);                     /* Do nothing */
#else
        printf("\033[K");               /* Or use ANSI */
#endif /* COMMENT */
    } else {
        fn = ck_outc;
        tputs(cur_cleol,1,fn);
    }
    return(0);
}
#endif /* CK_CURPOS */
#else
static void
ck_termset(x) int x; {
    if (x) return;
}
#endif /* NOTERMCAP */

#ifndef CK_CURPOS
#define CK_CURPOS
int
ck_cls() {
    printf("\033[;H\033[2J");
    return(0);
}

int
ck_cleol() {
    printf("\033[K");
    return(0);
}

int
ck_curpos(row, col) int row, col; {
    printf("\033[%d;%dH", row, col);
    return(0);
}
#endif /* CK_CURPOS */


#ifndef NOXFER
static int cinit = 0;                   /* Flag for curses init'd */
static int cendw = 0;                   /* endwin() was called */

static
#ifdef CK_ANSIC                         /* Because VOID used by curses.h */
void
#else
#ifdef MYCURSES
VOID
#else
int
#endif /* MYCURSES */
#endif /* CK_ANSIC */
#ifdef CK_ANSIC                         /* Update % transfered and % bar */
updpct(long old, long new)
#else /* CK_ANSIC */
updpct(old, new) long old, new;
#endif /* CK_ANSIC */
/* updpct */ {
#ifdef COMMENT
    int m, n;
    move(CW_PCD,22);
    printw("%ld", new);
#ifdef KUI
#ifndef K95G
    KuiSetProperty(KUI_FILE_TRANSFER, (long) CW_PCD, (long) new);
#endif /* K95G */
#endif /* KUI */
#ifdef CK_PCT_BAR
    if (thermometer) {
        if (old > new) {
            old = 0;
            move(CW_PCD, 26);
            clrtoeol();
        }
        m = old/2;
        move(CW_PCD, 26 + m);
        n = new / 2 - m;
#ifndef OS2
        while (n > 0) {
            if ((m + 1) % 5 == 0)
              printw("*");
            else
              printw("=");
            m++;
            n--;
        }
        if (new % 2 != 0) printw("-");
        /* move(CW_PCD, 22+53); */
#else /* OS2 */
        while (n > 0) {
            printw("%c", '\333');
            m++; n--;
        }
        if (new % 2 != 0)
          printw("%c", '\261');
#endif /* OS2 */
    }
#endif /* CK_PCT_BAR */
    /* clrtoeol(); */
#else  /* !COMMENT */
#ifdef OS2
#define CHAR1   '\333'          /* OS2 - CP437 */
#define CHAR2   '\261'
#else
#define CHAR1   '/'             /* Default */
#define CHAR2   '-'
#endif /* OS2 */
    debug(F101,"updpct old","",old);
    debug(F101,"updpct new","",new);
    move(CW_PCD,22);
    printw("%-3ld", new); /*  (was)   printw("%ld", new);  */
#ifdef KUI
#ifndef K95G
    KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_PCD, (long) new );
#endif /* K95G */
#endif /* KUI */
#ifdef CK_PCT_BAR
    if (thermometer) {
        int m, n;

        if (old > new) {
            old = 0 ;
            move(CW_PCD, 26);
            clrtoeol();
        }
        if (new <= 100L) {
            m = old / 2;
            n = new / 2 - m;
            move(CW_PCD, 26+m);
            while (n-- > 0)
              printw("%c", CHAR1);
            if (new % 2 != 0)
              printw("%c", CHAR2);
        }
    }
#endif /* CK_PCT_BAR */
#endif /* COMMENT */
}

static CK_OFF_T old_tr = (CK_OFF_T)-1;	/* Time remaining previously */

static CK_OFF_T
#ifdef CK_ANSIC
shoetl(CK_OFF_T old_tr, long cps, CK_OFF_T fsiz, CK_OFF_T howfar)
#else
    shoetl(old_tr, cps, fsiz, howfar) long cps; CK_OFF_T old_tr, fsiz, howfar;
#endif /* CK_ANSIC */
/* shoetl */ {                          /* Estimated time left in transfer */
    CK_OFF_T tr;			/* Time remaining, seconds */

#ifdef GFTIMER
    if (fsiz > 0L && cps > 0L)
      tr = (CK_OFF_T)((CKFLOAT)(fsiz - howfar) / (CKFLOAT)cps);
    else
      tr = (CK_OFF_T)-1;
#else
    tr = (fsiz > 0L && cps > 0L) ?
      ((fsiz - howfar) / cps) :
        (CK_OFF_T)-1;
#endif /* GFTIMER */
    move(CW_TR,22);
    if (tr > (CK_OFF_T)-1) {
        if (tr != old_tr) {
            printw("%s",hhmmss(tr));
#ifdef KUI
#ifndef K95G
            KuiSetProperty(KUI_FILE_TRANSFER, (long)CW_TR, (long)hhmmss(tr));
#endif /* K95G */
#endif /* KUI */
            clrtoeol();
        }
    } else {
        printw("(unknown)");
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_TR, (long) "(unknown)" );
#endif /* K95G */
#endif /* KUI */
        clrtoeol();
    }
    return(tr);
}

static long
#ifdef CK_ANSIC
shocps(int pct, CK_OFF_T fsiz, CK_OFF_T howfar)
#else
shocps(pct, fsiz, howfar) int pct; CK_OFF_T fsiz, howfar;
#endif /* CK_ANSIC */
/* shocps */ {
#ifdef CPS_WEIGHTED
    static CK_OFF_T oldffc = 0L;
#endif /* CPS_WEIGHTED */
#ifdef GFTIMER
    CKFLOAT secs, xx;
#else
    CK_OFF_T secs, xx;
#endif /* GFTIMER */

#ifdef GFTIMER
    xx = (gtv >= 0.0) ? gtv : 0.0;      /* Floating-point version */
    gtv = gftimer();
    if ((gtv - oldgtv) < (CKFLOAT) 1.0) /* Only do this once per second */
      return(oldcps);
    oldgtv = xx;
#else
    xx = (gtv >= 0) ? gtv : 0;          /* Whole-number version */
    gtv = gtimer();
    if ((gtv - oldgtv) < 1)
      return(oldcps);
    oldgtv = xx;
#endif /* GFTIMER */

#ifdef CPS_WEIGHTED
    /* debug(F100,"SHOCPS: WEIGHTED","",0); */
    if (gtv != oldgtv) {                /* The first packet is ignored */
        if (ffc < oldffc)
          oldffc = ffc;
        oldcps = cps;
        if (oldcps && oldgtv >
#ifdef GFTIMER
            1.0
#else
            1
#endif /* GFTIMER */
            ) {                         /* The first second is ignored */
/*
  This version of shocps() produces a weighted average that some
  people like, but most people find it disconcerting and bombard us
  with questions and complaints about why the CPS figure fluctuates so
  wildly.  So now you only get the weighted average if you build the
  program yourself with CPS_WEIGHTED defined.
*/
#ifndef CPS_VINCE
#ifdef GFTIMER
            cps = (long)((((CKFLOAT)oldcps * 3.0) +
                   (CKFLOAT)(ffc - oldffc) / (gtv-oldgtv) ) / 4.0);
#else
            cps = ( (oldcps * 3) + (ffc - oldffc) / (gtv-oldgtv) ) / 4;
#endif /* GFTIMER */
#else
/* And an alternate weighting scheme from Vincent Fatica... */
            cps = (3 *
             ((1+pct/300)*oldffc/oldgtv+(1-pct/100)*(ffc-oldffc)/(gtv-oldgtv)))
              / 4;
#endif /* CPS_VINCE */
        } else {
            /* No weighted average since there is nothing to weigh */
#ifdef GFTIMER
            cps = (long)(gtv != 0.0 ?
              (CKFLOAT)(ffc - oldffc) / (gtv - oldgtv) :
                (ffc - oldffc)) ;
#else
            cps = gtv ? (ffc - oldffc) / (gtv - oldgtv) : (ffc - oldffc) ;
#endif /* GFTIMER */
        }
#ifdef COMMENT
#ifdef DEBUG
        if (deblog) {
            debug(F101,"SHOCPS: pct   ","",pct);
            debug(F101,"SHOCPS: gtv   ","",gtv);
            debug(F101,"SHOCPS: oldgtv","",oldgtv);
            debug(F101,"SHOCPS: dgtv  ","",(long)(gtv-oldgtv));
            debug(F101,"SHOCPS: ffc   ","",ffc);
            debug(F101,"SHOCPS: oldffc","",oldffc);
            debug(F101,"SHOCPS: dffc  ","",ffc-oldffc);
            debug(F101,"SHOCPS: cps   ","",cps);
        }
#endif /* DEBUG */
#endif /* COMMENT */
        move(CW_CP,22);
        printw("%ld", cps);
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_CP, (long) cps );
#endif /* K95G */
#endif /* KUI */
        clrtoeol();
        oldffc = ffc;
    }
#else /* !CPS_WEIGHTED */
#ifdef COMMENT
#ifdef DEBUG
    if (deblog) {
	debug(F100,"SHOCPS: NOT WEIGHTED","",0);
        debug(F101,"SHOCPS: pct    ","",pct);
        debug(F101,"SHOCPS: gtv    ","",gtv);
        debug(F101,"SHOCPS: oldgtv ","",oldgtv);
        debug(F101,"SHOCPS: dgtv   ","",(long)gtv - (long)oldgtv);
        debug(F101,"SHOCPS: ffc    ","",ffc);
        debug(F101,"SHOCPS: oldffc ","",oldffc);
        debug(F101,"SHOCPS: dffc   ","",ffc-oldffc);
        debug(F101,"SHOCPS: cps    ","",cps);
        debug(F101,"SHOCPS: filcnt ","",filcnt);
#ifdef GFTIMER
        debug(F101,"SHOCPS: fpfsecs","",fpfsecs);
#endif /* GFTIMER */
    }
    debug(F101,"shocps gtv","",gtv);
#endif /* DEBUG */
#ifdef GFTIMER
#endif /* COMMENT */
    /* debug(F101,"shocps fpfsecs","",fpfsecs); */
    secs = gtv - fpfsecs;
    /* debug(F101,"shocps secs","",(long)secs); */
    if (secs > 0.0) {
        cps = (long)((CKFLOAT) ffc / secs);
        /* debug(F101,"shocps cps","",cps); */
        move(CW_CP,22);
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_CP, (long) cps );
#endif /* K95G */
#endif /* KUI */
        printw("%ld", cps);
        clrtoeol();
    }
#else  /* Not GFTIMER */
    if ((secs = gtv - fsecs) > 0) {
        cps = (secs < 1L) ? ffc : ffc / secs;
        move(CW_CP,22);
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_CP, (long) cps );
#endif /* K95G */
#endif /* KUI */
        printw("%ld", cps);
        clrtoeol();
    }
#endif /* GFTIMER */
#endif /* CPS_WEIGHTED */

    if (cps > peakcps &&                /* Peak transfer rate */
        ((what & W_SEND && spackets > wslots + 4) ||
	 (!(what & W_SEND) && spackets > 10))) {
        peakcps = cps;
    }
    old_tr = shoetl(old_tr, cps, fsiz, howfar);
    return(cps);
}

static
#ifdef CK_ANSIC                         /* Because VOID used by curses.h */
void
#else
#ifdef MYCURSES
VOID
#else
int
#endif /* MYCURSES */
#endif /* CK_ANSIC */
scrft() {                               /* Display file type */
    char xferstr[256];
    xferstr[0] = NUL;
    debug(F101,"scrft binary","",binary);
    if (binary) {
        switch(binary) {
          case XYFT_L:
            ckstrncpy(xferstr,"LABELED",256);
            break;
          case XYFT_I:
            ckstrncpy(xferstr,"IMAGE",256);
            break;
          case XYFT_U:
            ckstrncpy(xferstr,"BINARY UNDEFINED",256);
            break;
	  case XYFT_M:
            ckstrncpy(xferstr,"MACBINARY",256);
            break;
	  case XYFT_X:
            ckstrncpy(xferstr,"TENEX",256);
            break;
          default:
          case XYFT_B:
            ckstrncpy(xferstr,"BINARY",256);
            break;
        }
#ifdef CK_RESEND
        if (what & W_SEND && sendstart > 0L) {
            if (sendmode == SM_PSEND) {
                ckstrncat(xferstr, " / partial", 256);
            } else if (sendmode == SM_RESEND) {
                ckstrncat(xferstr, " / resend", 256);
            }
        } else if (what & W_RECV && rs_len > 0L) {
            ckstrncat(xferstr, " / resend", 256);
        }
#endif /* CK_RESEND */
    } else {

#ifndef NOCSETS
        ckstrncpy(xferstr,"TEXT",256);
#ifdef NEWFTP
#ifndef NOUNICODE
	if (what & W_FTP) {
	    if (ftp_csx < 0)
	      ckstrncat(xferstr," (no translation)", 256);
	    else
	      ckmakxmsg(&xferstr[4],252,
		       " (",
		       fcsinfo[(what & W_SEND) ? ftp_csl : ftp_csx].keyword,
		       " => ",
		       fcsinfo[(what & W_SEND) ? ftp_csx : ftp_csl].keyword,
		       ")",
		       NULL,NULL,NULL,NULL,NULL,NULL,NULL
		       );
	} else
#endif /* NOUNICODE */
#endif /* NEWFTP */
	  if (tcharset == TC_TRANSP) {
            ckstrncat(xferstr, " (no translation)", 256);
        } else {
            if (what & W_SEND) {
                sprintf( &xferstr[strlen(xferstr)], /* safe */
                        " (%s => %s)",
                        fcsinfo[fcharset].keyword, /* built-in keywords */
                        tcsinfo[tcharset].keyword  /* lengths are controlled */
			);
            } else {
                sprintf( &xferstr[strlen(xferstr)], /* safe */
                        " (%s => %s)",
                        tcsinfo[tcharset].keyword, /* built-in keywords */
                        fcsinfo[fcharset].keyword); /* lengths controlled */
            }
        }
#endif /* NOCSETS */
    }
    move(CW_TYP,22);
    printw("%s", xferstr);
    clrtoeol();
#ifdef KUI
#ifndef K95G
    KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_TYP, (long) xferstr );
#endif /* K95G */
#endif /* KUI */
    return;
}

#ifdef CK_NEWTERM
static FILE *ck_stdout = NULL;
static int ck_fd = -1;
#endif /* CK_NEWTERM */

static long pct = 0L, oldpct = 0L, oldrtt = -1L;
static int oldtyp = 0, oldwin = -1, oldtry = -1, oldlen = -1, oldtim = -1;

#ifdef NETCONN
static char *netname[] = {
    "none",				/* 00 */
    "TCP/IP",				/* 01 TCP (Sockets) */
    "TCP/IP",				/* 02 TCP (Streams) */
    "X.25",				/* 03 SunLink X.24  */
    "DECnet",				/* 04 DECnet  */
    "VAX PSI",				/* 05 VAX PSI */
    "Named Pipes",			/* 06 LAN Manager Named Pipe */
    "X.25",				/* 07 Stratus VOS X.25 */
    "NetBIOS",				/* 08 IBM NETBIOS */
    "SuperLAT",				/* 07 Meridian SuperLAT */
    "File",				/* 10 File */
    "Command",				/* 11 Subprocess (pipe) */
    "DLL",				/* 12 DLL does i/o */
    "X.25",				/* 13 IBM AIXLink X.25 */
    "X.25",				/* 14 HP-UX X.25 */
    "PTY",				/* 15 Pseudoterminal */
    "SSH",				/* 16 SSH */
    "<ERROR>",				/* 17 In case new types are added */
    "<ERROR>",				/* 18 but nobody remembers to update */
    "<ERROR>",				/* 19 this table ... */
    NULL				/* 20 */
};
static int nnetname = (sizeof(netname) / sizeof(char *));

#endif /* NETCONN */

#ifdef CK_ANSIC
void
screenc(int f, char c,CK_OFF_T n,char *s)
#else
CKVOID
screenc(f,c,n,s)
int f;          /* argument descriptor */
char c;         /* a character or small integer */
CK_OFF_T n;     /* a long integer */
char *s;        /* a string */
#endif /* CK_ANSIC */
/* screenc() */ {
#ifdef CK_SSL
    extern int tls_active_flag, ssl_active_flag;
#endif /* CK_SSL */
#ifdef RLOGCODE
    extern int ttnproto;
#endif /* RLOGCODE */
    static int q = 0;
    static long fcnt = 0L;		/* Number of files transferred */
    static CK_OFF_T fsiz = (CK_OFF_T)-1; /* Copy of file size */
    static CK_OFF_T fbyt = 0L; /* Total file bytes of all files transferred */
    static CK_OFF_T howfar = 0L; /* How much of current file has been xfer'd */
    static int  pctlbl = 0L;  /* Percent done vs Bytes so far */
    long cps = 0L;

    int net = 0;
    int xnet = 0;
    int ftp = 0;
    int len;                            /* Length of string */
    int errors = 0;                     /* Error counter */
    int x;                              /* Worker */

    debug(F101,"screenc cinit","",cinit);
    debug(F101,"screenc cendw","",cendw);

    if (!s) s = "";                     /* Always do this. */

    ftp = (what & W_FTP) ? 1 : 0;	/* FTP or Kermit */
    net = network || ftp;
    xnet = ftp ? 1 : nettype;		/* NET_TCPB == 1 */

    if (cinit == 0 || cendw > 0) {      /* Handle borderline cases... */
        if (f == SCR_CW) {              /* Close window, but it's not open */
            ft_win = 0;
            return;
        }
        debug(F111,"screenc A",s,f);
        if (f == SCR_EM ||
           (f == SCR_PT && c == 'E')) { /* Fatal error before window open */
            conoll(""); conoc('?'); conoll(s); return; /* Regular display */
        }
    }
    if (cinit == 0) {                   /* Only call initscr() once */
	char * s;
	/* Check these now -- if they are defined but not numeric */
	/* they can crash curses */
	s = getenv("LINES");
	if (s) if (!rdigits(s)) {
	    printf("?LINES variable not numeric: \"%s\".\n",s);
	    printf("(Fullscreen display disabled)\n");
	    fdispla = XYFD_S;
	    return;
	}
	s = getenv("COLUMNS");
	if (s) if (!rdigits(s)) {
	    printf("?COLUMNS variable not numeric: \"%s\".\n",s);
	    printf("(Fullscreen display disabled)\n");
	    fdispla = XYFD_S;
	    return;
	}
        cendw = 1;                      /* New window needs repainting */
#ifdef COMMENT
        if (!initscr()) {               /* Oops, can't initialize window? */
/*
  In fact, this doesn't happen.  "man curses" says initscr() halts the
  entire program if it fails, which is true on the systems where I've
  tested it.  It will fail if your terminal type is not known to it.
  That's why SET FILE DISPLAY FULLSCREEN calls tgetent() to make sure the
  terminal type is known before allowing a curses display.
*/
            fprintf(stderr,"CURSES INITSCR ERROR\r\n");
            fdispla = XYFD_S;           /* Fall back to CRT display */
            return;
        } else {
            cinit++;                    /* Window initialized ok */
            debug(F100,"CURSES INITSCR OK","",0);
        }
#else                                   /* Save some memory. */
#ifdef CK_NEWTERM
        /* (From Andy Fyfe <andy@vlsi.cs.caltech.edu>)
           System V curses seems to reserve the right to alter the buffering
           on the output FILE* without restoring it.  Fortunately System V
           curses provides newterm(), an alternative to initscr(), that
           allows us to specify explicitly the terminal type and input and
           output FILE pointers.  Thus we duplicate stdout, and let curses
           have the copy.  The original remains unaltered.  Unfortunately,
           newterm() seems to be particular to System V.
        */
        s = getenv("TERM");
        if (ck_fd < 0) {
            ck_fd = dup(fileno(stdout));
            ck_stdout = (ck_fd >= 0) ? (FILE *)fdopen(ck_fd, "w") : NULL;
        }
        debug(F100,"screenc newterm...","",0);

/* NOTE: It might be necessary to do this with stdin too! */
/* This would have been the case in FreeBSD 4.1 but they fixed the */
/* problem by restoring the buffering of stdin before the final release. */
/* (But T.E. Dickey says stdin is not buffered?) */

        if (ck_stdout == NULL || newterm(s, ck_stdout, stdin) == 0) {
            fprintf(stderr,
              "Fullscreen display not supported for terminal type: %s\r\n",s);
            fdispla = XYFD_S;           /* Use CRT instead */
            return;
        }
        debug(F100,"screenc newterm ok","",0);
#else
        debug(F100,"screen calling initscr","",0);
        initscr();                      /* Initialize curses. */
        debug(F100,"screen initscr ok","",0);
#endif /* CK_NEWTERM */
        cinit++;                        /* Remember curses was initialized. */
#endif /* COMMENT */
    }
    ft_win = 1;                         /* Window is open */
    if (repaint) {
#ifdef CK_WREFRESH
/*
  This totally repaints the screen, just what we want, but we can only
  do this with real curses, and then only if clearok() and wrefresh() are
  provided in the curses library.
*/
#ifdef OS2
        RestoreCmdMode();
#else
#ifdef QNX
#ifndef QNX16
        clearok(stdscr, 1);             /* QNX doesn't have curscr */
#endif /* QNX16 */
        wrefresh(stdscr);
#else
        wrefresh(curscr);
#endif /* QNX */
#endif /* OS2 */
#else  /* No CK_WREFRESH */
/*
  Kermit's do-it-yourself method, works with all types of fullscreen
  support, but does not repaint all the fields.  For example, the filename
  is lost, because it arrives at a certain time and never comes again, and
  Kermit presently does not save it anywhere.  Making this method work for
  all fields would be a rather major recoding task, duplicating what curses
  already does, and would add a lot of complexity and storage space.
*/
        cendw = 1;
#endif /* CK_WREFRESH */
        repaint = 0;
    }
    if (cendw) {                        /* endwin() was called previously */
#ifdef VMS
        initscr();                      /* (or should have been!) */
        clear();
        touchwin(stdscr);
        refresh();
#else
#ifdef QNX
/*
  In QNX, if we don't call initscr() here we core dump.
  I don't have any QNX curses documentation, but other curses manuals
  say that initscr() should be called only once per application, and
  experience shows that on other systems, calling initscr() here generally
  results in a core dump.
*/
        debug(F100,"screenc re-calling initscr QNX","",0);
        initscr();
        clear();
        refresh();
#ifdef COMMENT
/*
  But even so, second and subsequent curses displays are messed up.
  Calling touchwin, refresh, etc, doesn't make any difference.
*/
        debug(F100,"screenc calling touchwin QNX","",0);
        touchwin(stdscr);
        debug(F100,"screenc calling refresh QNX","",0);
        refresh();
#endif /* COMMENT */

#else /* All others... */
        debug(F100,"screenc calling clear","",0);
        clear();
        debug(F100,"screenc clear ok","",0);
#endif /* QNX */
#endif /* VMS */
        debug(F100,"screenc setup ok","",0);
        debug(F100,"screenc doing first move","",0);
        move(CW_BAN,0);                 /* Display the banner */
        debug(F110,"screenc myhost",myhost,0);
#ifdef TCPSOCKET
        debug(F110,"screenc myipaddr",myipaddr,0);
#endif /* TCPSOCKET */
#ifdef HPUX1010
        debug(F100,"screenc calling first printw...","",0);
/* Right here is where HP-UX 10.10 libxcurse.1 Rev 76.20 hangs... */
#endif /* HPUX1010 */
        if (myhost[0]) {
#ifdef TCPSOCKET
            if (!myipaddr[0]
#ifdef OS2
                 /* We need to perform this test because on non-TCP/IP */
                 /* systems the call to getlocalipaddr() results in a  */
                 /* DNS Lookup which takes several minutes to time out */
                 && net &&
                 (xnet == NET_TCPA || xnet == NET_TCPB
#ifdef SSHBUILTIN
                  || xnet == NET_SSH
#endif /* SSHBUILTIN */
                  )
#endif /* OS2 */
                 )
              getlocalipaddr();
            if (myipaddr[0] && strcmp((char *)myhost,(char *)myipaddr))
              printw("%s, %s [%s]",versio,(char *)myhost,(char *)myipaddr);
            else
#endif /* TCPSOCKET */
              printw("%s, %s",versio,(char *)myhost);
        } else {
            printw("%s",versio);
        }
#ifdef HPUX1010
        debug(F100,"screenc first printw returns","",0);
#endif /* HPUX1010 */
        move(CW_DIR,3);
        printw("Current Directory: %s",zgtdir());
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_DIR, (long) zgtdir() );
#endif /* K95G */
#endif /* KUI */
        if (net) {
            move(CW_LIN,8);
            printw("Network Host: %s",
#ifdef NEWFTP
		   ftp ? (ftp_host ? ftp_host : "(unknown)") :
#endif /* NEWFTP */
		   ttname
		   );
        } else {
            move(CW_LIN,0);
            printw("Communication Device: %s",ttname);
        }
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_LIN, (long) ttname );
#endif /* K95G */
#endif /* KUI */

        if (net) {
            move(CW_SPD,8);
            printw("Network Type: ");
        } else {
            move(CW_SPD,1);
            printw("Communication Speed: ");
        }
        move(CW_SPD,22);                /* Serial speed or network type */
        if (net) {
#ifdef NETCONN
	    int secure = 0;
	    char * xname;
	    if (xnet > nnetname)
	      xname = "[ERROR]";
	    else
	      xname = netname[xnet];
#ifdef NEWFTP
            if (ftp) {
		if (ftpissecure())
		  secure = 1;
	    } else
#endif /* NEWFTP */
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
                 ) {
		secure = 1;
	    }
	    if (secure) {
#ifdef KUI
#ifndef K95G
                char buf[30];
                sprintf(buf,"%s (SECURE)",xname);
                KuiSetProperty(KUI_FILE_TRANSFER,
                               (long) CW_SPD,
                               (long) buf
                               );
#endif /* K95G */
#endif /* KUI */
                printw("%s (SECURE)",xname);
            } else {
                printw("%s",xname);
#ifdef KUI
#ifndef K95G
                KuiSetProperty(KUI_FILE_TRANSFER,
                               (long) CW_SPD,
                               (long) xname
                               );
#endif /* K95G */
#endif /* KUI */
            }
#else
            printw("(network)");
#ifdef KUI
#ifndef K95G
            KuiSetProperty(KUI_FILE_TRANSFER,
                           (long) CW_SPD,
                           (long) "(network)"
                           );
#endif /* K95G */
#endif /* KUI */
#endif /* NETCONN */
        } else {
            if (speed < 0L)
              speed = ttgspd();
            if (speed > 0L) {
                if (speed == 8880) {
                    printw("75/1200");
#ifdef KUI
#ifndef K95G
                    KuiSetProperty(KUI_FILE_TRANSFER,
                                   (long) CW_SPD,
                                   (long) "75/1200"
                                   );
#endif /* K95G */
#endif /* KUI */
                } else {
                    char speedbuf[64] ;
                    sprintf(speedbuf, "%ld", speed);
                    printw("%s",speedbuf);
#ifdef KUI
#ifndef K95G
                    KuiSetProperty(KUI_FILE_TRANSFER,
                                   (long) CW_SPD,
                                   (long) speedbuf
                                   );
#endif /* K95G */
#endif /* KUI */
                }
            } else {
                printw("unknown");
#ifdef KUI
#ifndef K95G
                KuiSetProperty(KUI_FILE_TRANSFER,
                               (long) CW_SPD,
                               (long) "(unknown)"
                               );
#endif /* K95G */
#endif /* KUI */
            }
        }
        move(CW_PAR,14);
        printw("Parity: %s",ftp ? "none" : parnam((char)parity));
#ifdef KUI
#ifndef K95G
        KuiSetProperty(KUI_FILE_TRANSFER,
                       (long) CW_PAR,
                       (long) parnam((char)parity)
                       );
#endif /* K95G */
#endif /* KUI */
#ifdef CK_TIMERS
        if (/* rttflg && */ protocol == PROTO_K) {
            move(CW_TMO, 9); printw("RTT/Timeout:"); }
#endif /* CK_TIMERS */
        move(CW_TYP,11); printw("File Type:");
        move(CW_SIZ,11); printw("File Size:");
        move(CW_PCD, 8);
        clrtoeol();
        pctlbl = (what & W_SEND);
        printw("%s:", pctlbl ? "Percent Done" : "Bytes So Far");

#ifdef XYZ_INTERNAL
        move(CW_BAR, 1);
        printw("%10s Protocol:", ftp ? "FTP" : ptab[protocol].p_name);
#endif /* XYZ_INTERNAL */
#ifdef CK_PCT_BAR
        if (thermometer) {
            oldpct = pct = 0;
            move(CW_BAR,22);
            printw("    ...10...20...30...40...50...60...70...80...90..100");
            move(CW_BAR,22+56);
        }
#endif /* CK_PCT_BAR */
        move(CW_TR,  1); printw("Estimated Time Left:");
        move(CW_CP,  2); printw("Transfer Rate, CPS:");
        move(CW_WS,  8); printw("Window Slots:%s",
                                ((protocol == PROTO_K) && !ftp) ?
                                "" : " N/A"
                                );
        move(CW_PT,  9); printw("Packet Type:");
        if (ftp || protocol != PROTO_K) {
	    move(CW_PT,22);
            printw("%s", "N/A");
            move(CW_PC,  11); printw("I/O Count:");
            move(CW_PL,  10); printw("I/O Length:");
        } else {
            move(CW_PC,  8); printw("Packet Count:");
            move(CW_PL,  7); printw("Packet Length:");
        }
#ifndef COMMENT
        move(CW_PR,  9); printw("Error Count:");
#else
        move(CW_PR,  2); printw("Packet Retry Count:");
#endif
#ifdef COMMENT
        move(CW_PB,  2); printw("Packet Block Check:");
#endif /* COMMENT */
        move(CW_ERR,10); printw("Last Error:");
        move(CW_MSG, 8); printw("Last Message:");
	if (xfrmsg) {
	    move(CW_MSG, 22); printw("%s",xfrmsg);
	    makestr(&xfrmsg,NULL);
	}
        move(CW_INT, 0);
        if (!xfrint) {
            printw("(Transfer interruption is disabled)");
        } else {
#ifdef CK_NEED_SIG
            printw(
"<%s>X to cancel file, <%s>Z to cancel group, <%s><CR> to resend last packet",
                   dbchr(escape), dbchr(escape), dbchr(escape)
                   );
            move(CW_INT + 1, 0);
            printw(
"<%s>E to send Error packet, ^C to quit immediately, <%s>L to refresh screen.",
                   dbchr(escape), dbchr(escape)
                   );
#else /* !CK_NEED_SIG */
            move(CW_INT, 0);
#ifdef OS2
            if (protocol == PROTO_K) {
                printw(
"X to cancel file, Z to cancel group, <Enter> to resend last packet,"
                       );
            }
#else /* !OS2 */
#ifdef VMS                              /* In VMS avoid bottom line */
            printw(
"X: Cancel this file; E: Cancel transfer; ^C: Quit now; ^W: Refresh screen."
                   );
#else
            printw(
"X to cancel file, Z to cancel group, <CR> to resend last packet,"
                   );
#endif /* VMS */
#endif /* OS2 */

#ifndef VMS
            move(CW_INT + 1, 0);
            if (protocol == PROTO_K) {
                printw(
"E to send Error packet, ^C to quit immediately, ^L to refresh screen."
                       );
            } else {
                printw("^C to cancel file transfer.");
            }
#endif /* VMS */
#endif /* CK_NEED_SIG */
        }
        refresh();
        cendw = 0;
    }
    debug(F101,"SCREENC switch","",f);
    debug(F000,"SCREENC c","",c);
    debug(F101,"SCREENC n","",n);

    len = strlen(s);                    /* Length of argument string */
    switch (f) {                        /* Handle our function code */
      case SCR_FN:                      /* Filename */
        oldpct = pct = 0L;              /* Reset percents */
#ifdef GFTIMER
        gtv = (CKFLOAT) -1.0;
        /* oldgtv = (CKFLOAT) -1.0; */
#else
        gtv = -1L;
        /* oldgtv = -1L; */
#endif /* GFTIMER */
        oldwin = -1;
        fsiz = (CK_OFF_T)-1;		/* Invalidate previous file size */
        move(CW_PCD,22);                /* Erase percent done from last time */
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_PCD, (long) 0 );
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_FFC, (long) 0 );
#endif /* K95G */
#endif /* KUI */
        clrtoeol();
        move(CW_SIZ,22);                /* Erase file size from last time */
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_SIZ, (long) 0 );
#endif /* K95G */
#endif /* KUI */
        clrtoeol();
        move(CW_ERR,22);                /* And last error message */
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_ERR, (long) "" );
#endif /* K95G */
#endif /* KUI */
        clrtoeol();
#ifdef COMMENT
#ifdef STREAMING
        if (protocol == PROTO_K && streamok) {
            move(CW_BAR, 1);
#ifdef XYZ_INTERNAL
            printw("   Kermit STREAMING:");
#else
            printw("          STREAMING:");
#endif /* XYZ_INTERNAL */
        }
#endif /* STREAMING */
#endif /* COMMENT */

        if (what & W_SEND) {		/* If we're sending... */
#ifdef NEWFTP
	    if (what & W_FTP) {		/* FTP */
                move(CW_NAM,10);
                printw("   FTP PUT:");
	    } else
#endif /* NEWFTP */
#ifdef CK_RESEND
            switch (sendmode) {		/* Kermit */
              case SM_RESEND:
                move(CW_NAM,10);
                printw(" RESENDING:");
                break;
              default:
                move(CW_NAM,10);
                printw("   SENDING:");
                break;
            }
#else
            move(CW_NAM,10);
            printw("   SENDING:");
#endif /* CK_RESEND */

        } else if (what & W_RECV) {	/* If we're receiving... */
#ifdef NEWFTP
	    if (what & W_FTP) {		/* FTP */
                move(CW_NAM,10);
                printw("   FTP GET:");
	    } else {
#endif /* NEWFTP */
		move(CW_NAM,10);
		printw(" RECEIVING:");
#ifdef NEWFTP
	    }
        } else if (what == (W_FTP|W_FT_DELE)) {
		move(CW_NAM,10);
		printw("FTP DELETE:");
#endif /* NEWFTP */
        } else {                        /* If we don't know... */
            move(CW_NAM,10);            /* (should never see this) */
            printw(" File Name:");
        }
        move(CW_NAM,22);                /* Display the filename */
        if (len > 57) {
            printw("%.55s..",s);
            len = 57;
        } else printw("%s",s);
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_NAM, (long) s );
#endif /* K95G */
#endif /* KUI */
        q = len;                        /* Remember name length for later */
        clrtoeol();
        scrft();                        /* Display file type (can change) */
        refresh();
#ifdef OS2
        SaveCmdMode(0, 0);
#endif /* OS2 */
        return;

      case SCR_AN:                      /* File as-name */
        if (q + len + 4 < 58) {         /* Will fit */
            move(CW_NAM, 22 + q);
            printw(" => %s",s);
#ifdef KUI
#ifndef K95G
            KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_NAM, (long) s );
#endif /* K95G */
#endif /* KUI */
        } else {                        /* Too long */
            move(CW_NAM, 22);           /* Overwrite previous name */
            q = 0;
            if (len + 4 > 57) {                                 /* wg15 */
                printw(" => %.51s..",s);                        /* wg15 */
                len = 53;                                       /* wg15 */
            } else printw(" => %s",s);                          /* wg15 */
#ifdef KUI
#ifndef K95G
            KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_NAM, (long) s  );
#endif /* K95G */
#endif /* KUI */
        }
        q += len + 4;                   /* Remember horizontal position */
        clrtoeol();
        refresh();
#ifdef OS2
        SaveCmdMode(0, 0);
#endif /* OS2 */
        return;

      case SCR_FS:                      /* File size */
        fsiz = n;
        move(CW_SIZ,22);
        if (fsiz > (CK_OFF_T)-1) {
#ifdef KUI
#ifndef K95G
            KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_SIZ, (long) n );
#endif /* K95G */
#endif /* KUI */
            printw("%s",ckfstoa(n));
        }
	if (fsiz == -2L) {
	    printw("POSSIBLY EXCEEDS LOCAL FILE SIZE LIMIT");
	}
        clrtoeol();
#ifdef COMMENT
        move(CW_PCD, 8);
        if (fsiz > (CK_OFF_T)-1) {	/* Put up percent label */
            pctlbl = 1;
	    clrtoeol();
            printw("Percent Done:");
        }
#else
	move(CW_PCD, 8);
	clrtoeol();
        if (fsiz > (CK_OFF_T)-1) {	/* Put up percent label */
            pctlbl = 1;
            printw("Percent Done:");
        } else {
            pctlbl = 0;
            printw("Bytes So Far:");
	}
#endif /* COMMENT */
        clrtoeol();
        scrft();                        /* File type */
        refresh();
#ifdef OS2
        SaveCmdMode(0, 0);
#endif /* OS2 */
        return;

      case SCR_PT:                      /* Packet type or pseudotype */
        if (spackets < 5) {
            extern int sysindex;
            extern struct sysdata sysidlist[];
            /* Things that won't change after the 4th packet */
            move(CW_PAR,22);
            printw("%s",parnam((char)parity));
#ifdef KUI
#ifndef K95G
            KuiSetProperty( KUI_FILE_TRANSFER,
                           (long) CW_PAR,
                           (long) parnam((char)parity)
                           );
#endif /* K95G */
#endif /* KUI */
            clrtoeol();
#ifdef COMMENT
            move(CW_PB, 22);            /* Block check on this packet */
            if (bctu == 4)
              printw("B");
            else
              printw("%d",bctu);
            clrtoeol();
#endif /* COMMENT */
            if (
#ifdef NEWFTP
		(ftp && (spackets == 1 || rpackets == 1)) ||
#endif /* NEWFTP */
		spackets == 4
		) {
                move(CW_LIN,8);
                if (
#ifdef NEWFTP
		    ftp ||
#endif /* NEWFTP */
		    ((protocol == PROTO_K) && (sysindex > -1))
		    ) {
                    if (net) {
                        move(CW_LIN,8);
                        printw("Network Host: %s (%s)",
#ifdef NEWFTP
			       ftp ? (ftp_host ? ftp_host : "") :
#endif /* NEWFTP */
			       ttname,
#ifdef NEWFTP
			       ftp ? ftp_srvtyp :
#endif /* NEWFTP */
			       sysidlist[sysindex].sid_name
			       );
                    } else {
                        move(CW_LIN,0);
                        printw("Communication Device: %s (remote host is %s)",
                             ttname,
                             sysidlist[sysindex].sid_name
                             );
                    }
                    clrtoeol();
                }
            }
        }
#ifdef CK_TIMERS
        if (/* rttflg && */ protocol == PROTO_K) {
            long xx;
            if (
#ifdef STREAMING
                streaming && oldwin != -2
#else
                0
#endif /* STREAMING */
                ) {
                move(CW_TMO, 22);
                printw("00 / 00");
                clrtoeol();
            } else {
                xx = (rttdelay + 500) / 1000;
                if (xx != oldrtt || rcvtimo != oldtim) {
                    move(CW_TMO, 22);
                    printw("%02ld / %02d", xx, rcvtimo);
                    oldrtt = xx;
                    oldtim = rcvtimo;
                    clrtoeol();
                }
            }
        }
#endif /* CK_TIMERS */

        x = (what & W_RECV) ?          /* Packet length */
          rpktl+(protocol==PROTO_K?1:0) :
            spktl;
        if (x != oldlen) {              /* But only if it changed. */
            move(CW_PL, 22);
            printw("%d",x);
#ifdef KUI
#ifndef K95G
            KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_PL, (long) x );
#endif /* K95G */
#endif /* KUI */
            clrtoeol();
            oldlen = x;
        }
        move(CW_PC, 22);                /* Packet count (always). */

        printw("%d", (what & W_RECV) ? rpackets : spackets);
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_PC, (long) spackets );
#endif /* K95G */
#endif /* KUI */
        clrtoeol();

        if (protocol == PROTO_K && !ftp) { /* Window slots */
            char ws[16];
            int flag;
            flag = 0;
#ifdef STREAMING
            if (streaming) {
                if (oldwin != -2) {
                    sprintf(ws,"STREAMING");
                    flag = 1;
                    oldwin = -2;
                }
            } else
#endif /* STREAMING */
              if (wcur != oldwin) {
                  sprintf(ws, "%d of %d", wcur < 1 ? 1 : wcur, wslotn);
                  flag = 1;
                  oldwin = wcur;
              }
            if (flag) {
                move(CW_WS, 22);
                printw("%s", ws);
                clrtoeol();
#ifdef KUI
#ifndef K95G
                KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_WS, (long) ws );
#endif /* K95G */
#endif /* KUI */
            }
        }
        errors = retrans + crunched + timeouts;
        if (errors != oldtry) {         /* Retry count, if changed */
            move(CW_PR, 22);
            printw("%d",errors);
#ifdef KUI
#ifndef K95G
            KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_PR, (long) errors );
#endif /* K95G */
#endif /* KUI */
            clrtoeol();
            oldtry = errors;
        }
	/* Sender's packet type */
        if (!ftp && (c != oldtyp && c != 'Y' && c != 'N')) {
            char type[2];
            sprintf(type, "%c",c);
            move(CW_PT,22);
            printw("%s", type);
#ifdef KUI
#ifndef K95G
            KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_PT, (long) type );
#endif /* K95G */
#endif /* KUI */
            clrtoeol();
            oldtyp = c;
        }
        switch (c) {                    /* Now handle specific packet types */
          case 'S':                     /* Beginning of transfer */
            fcnt = fbyt = 0L;           /* Clear counters */
#ifdef GFTIMER
            gtv = -1.0;
#else /* GFTIMER */
            gtv = -1L;                  /* And old/new things... */
#endif /* GFTIMER */
            oldpct = pct = 0L;
            break;

          case 'Z':                     /* or EOF */
            debug(F101,"screenc SCR_PT Z pktnum","",n);
            debug(F101,"screenc SCR_PT Z oldpct","",oldpct);
            debug(F101,"screenc SCR_PT Z pct","",pct);
          case 'D':                     /* Data packet */
            if (fsiz > 0L) {            /* Show percent done if known */
                oldpct = pct;           /* Remember previous percent */
                howfar = ffc;
#ifdef CK_RESEND
                if (what & W_SEND)	/* Account for PSEND or RESEND */
                  howfar += sendstart;
                else if (what & W_RECV)
                  howfar += rs_len;
#endif /* CK_RESEND */
                /* Percent done, to be displayed... */
                if (c == 'Z') {
                    if (!discard && !cxseen && !czseen) pct = 100L;
                } else
                  pct = (fsiz > 99L) ? (howfar / (fsiz / 100L)) : 0L;
                if (pct > 100L ||       /* Allow for expansion and */
                   (oldpct == 99L && pct < 0L)) /* other boundary conditions */
                  pct = 100L;
                if (pct != oldpct)      /* Only do this 100 times per file */
                  updpct(oldpct, pct);
            } else {
                move(CW_PCD,22);
                printw("%s", ckfstoa(ffc));
            }
#ifdef KUI
#ifndef K95G
            KuiSetProperty(KUI_FILE_TRANSFER, (long) CW_FFC, (long) howfar);
#endif /* K95G */
#endif /* KUI */
            cps = shocps((int) pct, fsiz, howfar);
            /* old_tr = shoetl(old_tr, cps, fsiz, howfar); */
            break;

          case '%':                     /* Timeouts, retransmissions */
            cps = shocps((int) pct, fsiz, howfar);
            /* old_tr = shoetl(old_tr, cps, fsiz, howfar); */

            errors = retrans + crunched + timeouts;
            if (errors != oldtry) {     /* Error count, if changed */
                move(CW_PR, 22);
                printw("%d",errors);
                clrtoeol();
#ifdef KUI
#ifndef K95G
                KuiSetProperty(KUI_FILE_TRANSFER,
                               (long) CW_PR, (long) errors
                               );
#endif /* K95G */
#endif /* KUI */
                }
                oldtry = errors;
                if (s) if (*s) {
                    move(CW_ERR,22);
                    printw("%s",s);
                    clrtoeol();
#ifdef KUI
#ifndef K95G
                    KuiSetProperty(KUI_FILE_TRANSFER, (long) CW_ERR, (long) s);
#endif /* K95G */
#endif /* KUI */
            }
            break;

          case 'E':                     /* Error packet */
#ifdef COMMENT
            move(CW_ERR,22);            /* Print its data field */
            if (*s) {
                printw("%s",s);
#ifdef KUI
#ifndef K95G
                KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_ERR, (long) s );
#endif /* K95G */
#endif /* KUI */
            }
            clrtoeol();
#endif /* COMMENT */
            fcnt = fbyt = 0L;           /* So no bytes for this file */
            break;
          case 'Q':                     /* Crunched packet */
            cps = shocps((int) pct, fsiz, howfar);
            /* old_tr = shoetl(old_tr, cps, fsiz, howfar); */
            move(CW_ERR,22);
            printw("Damaged Packet");
#ifdef KUI
#ifndef K95G
            KuiSetProperty(KUI_FILE_TRANSFER,
                           (long) CW_ERR,
                           (long) "Damaged Packet"
                           );
#endif /* K95G */
#endif /* KUI */
            clrtoeol();
            break;
          case 'q':                     /* Ctrl-C or connection lost */
            move(CW_MSG,22);
	    clrtoeol();
            if (!s) s = "";
            printw(*s ? s : "User interruption or connection lost");
#ifdef KUI
#ifndef K95G
            KuiSetProperty(KUI_FILE_TRANSFER,
                           (long) CW_MSG,
                           (long) s
                           );
#endif /* K95G */
#endif /* KUI */
            break;
          case 'T':                     /* Timeout */
            cps = shocps((int) pct, fsiz, howfar);
            /* old_tr = shoetl(old_tr, cps, fsiz, howfar); */
            move(CW_ERR,22);
            printw("Timeout %d sec",rcvtimo);
#ifdef KUI
#ifndef K95G
            KuiSetProperty(KUI_FILE_TRANSFER,
                           (long) CW_ERR,
                           (long) "Timeout"
                           );
#endif /* K95G */
#endif /* KUI */
            clrtoeol();
            errors = retrans + crunched + timeouts;
            if (errors != oldtry) {     /* Error count, if changed */
                move(CW_PR, 22);
                printw("%d",errors);
#ifdef KUI
#ifndef K95G
                KuiSetProperty(KUI_FILE_TRANSFER,
                               (long) CW_PR, (long) errors
                               );
#endif /* K95G */
#endif /* KUI */
                clrtoeol();
                oldtry = errors;
            }
            break;
          default:                      /* Others, do nothing */
            break;
        }
        refresh();
#ifdef OS2
        SaveCmdMode(0, 0);
#endif /* OS2 */
        return;

      case SCR_ST:                      /* File transfer status */
        debug(F101,"screenc SCR_ST c","",c);
        debug(F101,"screenc SCR_ST success","",success);
        debug(F101,"screenc SCR_ST cxseen","",cxseen);
#ifdef COMMENT
        move(CW_PCD,22);                /* Update percent done */
        if (c == ST_OK) {               /* OK, print 100 % */
            if (pctlbl)
              updpct(oldpct,100);
            else
              printw("%s", ckfstoa(ffc));
#ifdef KUI
#ifndef K95G
            KuiSetProperty(KUI_FILE_TRANSFER, (long) CW_FFC, (long) ffc);
#endif /* K95G */
#endif /* KUI */
            pct = 100;
            oldpct = 0;
        } else if (fsiz > 0L)           /* Not OK, update final percent */
/*
  The else part writes all over the screen -- howfar and/or fsiz have
  been reset as a consequence of the not-OKness of the transfer.
*/
          if (pctlbl)
            updpct(oldpct, (howfar * 100L) / fsiz);
        clrtoeol();
#else
        if (c == ST_OK) {               /* OK, print 100 % */
            move(CW_PCD,22);            /* Update percent done */
            if (pctlbl) {
		if (oldpct == 0)	/* Switching from "bytes so far" */
		  clrtoeol();		/* to "percent done"... */
		updpct(oldpct,100);
	    } else
              printw("%s", ckfstoa(ffc));
#ifdef KUI
#ifndef K95G
            KuiSetProperty(KUI_FILE_TRANSFER, (long) CW_FFC, (long) ffc);
#endif /* K95G */
#endif /* KUI */
#ifdef COMMENT
            pct = 100;
            oldpct = 0;
#endif /* COMMENT */
            clrtoeol();
        }
#endif /* COMMENT */

#ifdef COMMENT
/* No, leave it there so they can read it */
        move(CW_MSG,22);                /* Remove any previous message */
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_MSG, (long) "" );
#endif /* K95G */
#endif /* KUI */
        clrtoeol(); refresh();
#endif /* COMMENT */

        move(CW_TR, 22);
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_TR, (long) "" );
#endif /* K95G */
#endif /* KUI */
        clrtoeol(); refresh();

        switch (c) {                    /* Print new status message */
          case ST_OK:                   /* Transfer OK */
            fcnt++;                     /* Count this file */
	    if (what == (W_FTP|W_FT_DELE)) {
		move(CW_MSG,22);
		clrtoeol();
		printw("Delete OK");
	    } else {
		fbyt += ffc;
		move(CW_MSG,22);
		clrtoeol();
		printw("Transfer OK");
	    }
#ifdef KUI
#ifndef K95G
            KuiSetProperty(KUI_FILE_TRANSFER,
                           (long) CW_MSG,
                           (long) "Transfer OK"
                           );
#endif /* K95G */
#endif /* KUI */
            clrtoeol(); refresh();
            return;

          case ST_DISC:                 /* Discarded */
            move(CW_ERR,22);
            printw("File discarded");
#ifdef KUI
#ifndef K95G
            KuiSetProperty(KUI_FILE_TRANSFER,
                           (long) CW_ERR,
                           (long) "File discarded"
                           );
#endif /* K95G */
#endif /* KUI */
#ifdef COMMENT
            pct = oldpct = 0;
#endif /* COMMENT */
            clrtoeol(); refresh();
            return;

          case ST_INT:                  /* Interrupted */
            move(CW_ERR,22);
            printw("Transfer interrupted");
#ifdef KUI
#ifndef K95G
            KuiSetProperty(KUI_FILE_TRANSFER,
                           (long) CW_ERR,
                           (long) "Transfer interrupted"
                           );
#endif /* K95G */
#endif /* KUI */
#ifdef COMMENT
            pct = oldpct = 0;
#endif /* COMMENT */
            clrtoeol(); refresh();
            return;

          case ST_SKIP:                 /* Skipped */
            move(CW_ERR,22);
	    if (n > 0 && n < nskreason)
	      printw("File skipped (%s)",skreason[n]);
	    else
	      printw("File skipped");
#ifdef KUI
#ifndef K95G
            KuiSetProperty(KUI_FILE_TRANSFER,
                           (long) CW_ERR,
                           (long) "File skipped"
                           );
#endif /* K95G */
#endif /* KUI */
#ifdef COMMENT
            pct = oldpct = 0;
#endif /* COMMENT */
            clrtoeol(); refresh();
            return;

          case ST_ERR:                  /* Error message */
            move(CW_ERR,22);
            if (!s) s = (char *)epktmsg;
            printw("%s",s);
#ifdef KUI
#ifndef K95G
            KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_ERR, (long) s );
#endif /* K95G */
#endif /* KUI */
#ifdef COMMENT
            pct = oldpct = 0;
#endif /* COMMENT */
            clrtoeol(); refresh();
            return;

          case ST_REFU:                 /* Refused */
            move(CW_ERR,22);
            if (*s) {
                char errbuf[64] ;
                sprintf( errbuf, "Refused, %s", s ) ;
                printw("%s", errbuf);
#ifdef KUI
#ifndef K95G
                KuiSetProperty(KUI_FILE_TRANSFER,(long) CW_ERR,(long) errbuf);
#endif /* K95G */
#endif /* KUI */
            } else {
                printw("Refused");
#ifdef KUI
#ifndef K95G
                KuiSetProperty(KUI_FILE_TRANSFER,(long)CW_ERR,(long)"Refused");
#endif /* K95G */
#endif /* KUI */
            }
#ifdef COMMENT
            pct = oldpct = 0;
#endif /* COMMENT */
            clrtoeol(); refresh();
            return;

          case ST_INC:
            move(CW_ERR,22);
            printw("Incomplete");
#ifdef KUI
#ifndef K95G
            KuiSetProperty(KUI_FILE_TRANSFER,(long)CW_ERR,(long)"Incomplete");
#endif /* K95G */
#endif /* KUI */
#ifdef COMMENT
            pct = oldpct = 0;
#endif /* COMMENT */
            clrtoeol(); refresh();
            return;

          case ST_MSG:
            move(CW_MSG,22);
            printw("%s",s);
#ifdef KUI
#ifndef K95G
            KuiSetProperty(KUI_FILE_TRANSFER,(long)CW_MSG,(long)s);
#endif /* K95G */
#endif /* KUI */
            clrtoeol(); refresh();
            return;

          default:                      /* Bad call */
            move(CW_ERR,22);
            printw("*** screen() called with bad status ***");
#ifdef KUI
#ifndef K95G
            KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_ERR,
                       (long) "*** screen() called with bad status ***" );
#endif /* K95G */
#endif /* KUI */
            clrtoeol(); refresh(); return;
        }

      case SCR_TC: {                    /* Transaction complete */
          char msgbuf[128];
          move(CW_CP,22);               /* Overall transfer rate */
#ifdef KUI
#ifndef K95G
          KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_CP, tfcps);
#endif /* K95G */
#endif /* KUI */
          printw("%s", ckfstoa(tfcps));
          clrtoeol();
          if (success) {
              move(CW_MSG,22);          /* Print statistics in message line */
              clrtoeol();
          }
          if (success) {
              sprintf(msgbuf,
                      "SUCCESS.  Files: %ld, Bytes: %s, %ld CPS",
                      filcnt - filrej,
                      ckfstoa(fbyt),
                      tfcps
                      );
              printw("%s", msgbuf);
#ifdef KUI
#ifndef K95G
              KuiSetProperty(KUI_FILE_TRANSFER,
                             (long) CW_MSG,
                             (long) msgbuf
                             );
#endif /* K95G */
#endif /* KUI */
              clrtoeol();

          }
          move(CW_TR, 1);
          printw("       Elapsed Time: %s",hhmmss((long)
#ifdef GFTIMER
                                                  (fptsecs + 0.5)
#else
                                                  tsecs
#endif /* GFTIMER */
                                                   ));
#ifdef KUI
#ifndef K95G
          KuiSetProperty(KUI_FILE_TRANSFER,
                         (long) CW_TR,
                         (long) hhmmss((long)
#ifdef GFTIMER
                                       (fptsecs + 0.5)
#else
                                       tsecs
#endif /* GFTIMER */
                                       ));
#endif /* K95G */
#endif /* KUI */
          clrtoeol();
          move(23,0); clrtoeol();       /* Clear instructions lines */
          move(22,0); clrtoeol();       /* to make room for prompt. */
          refresh();

#ifdef GFTIMER
          oldgtv = (CKFLOAT) -1.0;
#else
          oldgtv = -1L;
#endif /* GFTIMER */

#ifndef VMSCURSE
	  debug(F100,"screenc endwin A","",0);
          endwin();
#ifdef COMMENT
/*
  Why and when was this call to conres() added?  It makes no sense,
  and it breaks echoing on Solaris 8.
*/
#ifdef SOLARIS
          conres();
#endif /* SOLARIS */
#endif /* COMMENT */
#endif /* VMSCURSE */

#ifdef COMMENT
          pct = 100; oldpct = 0;        /* Reset these for next time. */
#endif /* COMMENT */
          oldtyp = 0; oldrtt = -1L; oldtry = -1; oldlen = -1;
          oldtim = -1;
          cendw = 1;
          if (xfrbel) bleep(BP_NOTE);   /* Close window, then beep. */
#ifdef UNIX
          fflush(stdout);
#endif /* UNIX */
          ft_win = 0;                   /* Window closed. */
          return;
      }
      case SCR_EM:                      /* Error packet (fatal) */
        move (CW_ERR,22);
        printw("FAILURE: %s",s);
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_ERR, (long) s );
#endif /* K95G */
#endif /* KUI */
        if (xfrbel) bleep(BP_FAIL);
#ifdef COMMENT
        pct = oldpct = 0;
#endif /* COMMENT */
        clrtoeol(); refresh(); return;

      case SCR_QE:                      /* Quantity equals */
      case SCR_TU:                      /* Undelimited text */
      case SCR_TN:                      /* Text delimited at start */
      case SCR_TZ:                      /* Text delimited at end */
        return;                         /* (ignored in fullscreen display) */

      case SCR_MS:			/* Message from Kermit partner */
        move(CW_MSG,22);
	printw("%s",s);
        clrtoeol(); refresh(); return;

      case SCR_XD:                      /* X-packet data */
        pct = oldpct = 0;
        move(CW_NAM,22);
        printw("%s",s);
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_NAM, (long) s );
#endif /* K95G */
#endif /* KUI */
        clrtoeol(); refresh(); return;

      case SCR_CW:                      /* Close Window */
        clrtoeol(); move(23,0); clrtoeol(); move(22,0); clrtoeol();
        refresh();
#ifdef COMMENT
        pct = 100; oldpct = 0;          /* Reset these for next time. */
#endif /* COMMENT */
        oldtyp = 0; oldrtt = -1L; oldtry = -1; oldlen = -1;
        oldtim = -1;

#ifndef VMSCURSE
	debug(F100,"screenc endwin B","",0);
        endwin();
#endif /* VMSCURSE */
        ft_win = 0;                     /* Flag that window is closed. */
        cendw = 1; return;

      case SCR_CD:                      /* Display current directory */
        move(CW_DIR,22);
         printw("%s", s);
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_DIR, (long) s );
#endif /* K95G */
#endif /* KUI */
        clrtoeol();
        refresh();
#ifdef OS2
        SaveCmdMode(0, 0);
#endif /* OS2 */
        return;

      default:                          /* Bad call */
        move (CW_ERR,22);
#ifdef KUI
#ifndef K95G
        KuiSetProperty(KUI_FILE_TRANSFER,
                       (long) CW_ERR,
                       (long) "*** screen() called with bad function code ***"
                       );
#endif /* K95G */
#endif /* KUI */
        printw("*** screen() called with bad function code ***");
        clrtoeol(); refresh(); return;
    }
}
#endif /* CK_CURSES */

#ifdef KUI
#ifdef CK_ANSIC
void
screeng(int f, char c,long n,char *s)
#else
CKVOID
screeng(f,c,n,s)
int f;          /* argument descriptor */
char c;         /* a character or small integer */
long n;         /* a long integer */
char *s;        /* a string */
#endif /* CK_ANSIC */
/* screeng() */ {
#ifdef CK_SSL
    extern int tls_active_flag, ssl_active_flag;
#endif /* CK_SSL */
#ifdef RLOGCODE
    extern int ttnproto;
#endif /* RLOGCODE */
    static int q = 0;
    static CK_OFF_T fsiz = (CK_OFF_T)-1; /* Copy of file size */
    static long fcnt = 0L;    /* Number of files transferred */
    static long fbyt = 0L;    /* Total file bytes of all files transferred */
    static CK_OFF_T howfar = (CK_OFF_T)0; /* How much of file xfer'd so far */
    static int  pctlbl = 0L;  /* Percent done vs Bytes so far */
    long cps = 0L;

    int net = 0;
    int xnet = 0;
    int ftp = 0;
    int len;                            /* Length of string */
    int errors = 0;                     /* Error counter */
    int x;                              /* Worker */

    debug(F101,"screeng cinit","",cinit);
    debug(F101,"screeng cendw","",cendw);

    if (!s) s = "";                     /* Always do this. */

    ftp = (what & W_FTP) ? 1 : 0;	/* FTP or Kermit */
    net = network || ftp;
    xnet = ftp ? 1 : nettype;		/* NET_TCPB == 1 */

    if (cinit == 0 || cendw > 0) {      /* Handle borderline cases... */
        if (f == SCR_CW) {              /* Close window, but it's not open */
            ft_win = 0;
            return;
        }
        debug(F111,"screeng A",s,f);
        if (f == SCR_EM ||
           (f == SCR_PT && c == 'E')) { /* Fatal error before window open */
            conoll(""); conoc('?'); conoll(s); return; /* Regular display */
        }
    }
    if (cinit == 0) {                   /* Only call initscr() once */
	/* Check these now -- if they are defined but not numeric */
	/* they can crash curses */
        cendw = 1;                      /* New window needs repainting */
        debug(F100,"screeng calling initscr","",0);
        initscr();                      /* Initialize curses. */
        debug(F100,"screeng initscr ok","",0);
        cinit++;                        /* Remember curses was initialized. */
    }
    ft_win = 1;                         /* Window is open */
    if (repaint) {
#ifdef CK_WREFRESH
/*
  This totally repaints the screen, just what we want, but we can only
  do this with real curses, and then only if clearok() and wrefresh() are
  provided in the curses library.
*/
        RestoreCmdMode();
#else  /* No CK_WREFRESH */
/*
  Kermit's do-it-yourself method, works with all types of fullscreen
  support, but does not repaint all the fields.  For example, the filename
  is lost, because it arrives at a certain time and never comes again, and
  Kermit presently does not save it anywhere.  Making this method work for
  all fields would be a rather major recoding task, duplicating what curses
  already does, and would add a lot of complexity and storage space.
*/
        cendw = 1;
#endif /* CK_WREFRESH */
        repaint = 0;
    }
    if (cendw) {                        /* endwin() was called previously */
        debug(F100,"screeng setup ok","",0);
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_DIR, (long) zgtdir() );
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_LIN,
                        (long) (
#ifdef NEWFTP
                                ftp ? (ftp_host ? ftp_host : "(unknown)") :
#endif /* NEWFTP */
                                ttname) );

        if (net) {
#ifdef NETCONN
	    int secure = 0;
	    char * xname;
	    if (xnet > nnetname)
	      xname = "[ERROR]";
	    else
	      xname = netname[xnet];
#ifdef NEWFTP
            if (ftp) {
		if (ftpissecure())
		  secure = 1;
	    } else
#endif /* NEWFTP */
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
                 ) {
		secure = 1;
	    }
	    if (secure) {
                char buf[30];
                sprintf(buf,"%s (SECURE)",xname);
                KuiSetProperty(KUI_FILE_TRANSFER,
                               (long) CW_SPD,
                               (long) buf
                               );
            } else {
                KuiSetProperty(KUI_FILE_TRANSFER,
                               (long) CW_SPD,
                               (long) xname
                               );
            }
#else
            KuiSetProperty(KUI_FILE_TRANSFER,
                           (long) CW_SPD,
                           (long) "(network)"
                           );
#endif /* NETCONN */
        } else {
            if (speed < 0L)
              speed = ttgspd();
            if (speed > 0L) {
                if (speed == 8880) {
                    KuiSetProperty(KUI_FILE_TRANSFER,
                                   (long) CW_SPD,
                                   (long) "75/1200"
                                   );
                } else {
                    char speedbuf[64] ;
                    sprintf(speedbuf, "%ld", speed);
                    KuiSetProperty(KUI_FILE_TRANSFER,
                                   (long) CW_SPD,
                                   (long) speedbuf
                                   );
                }
            } else {
                KuiSetProperty(KUI_FILE_TRANSFER,
                               (long) CW_SPD,
                               (long) "(unknown)"
                               );
            }
        }
        KuiSetProperty(KUI_FILE_TRANSFER,
                       (long) CW_PAR,
                       (long) parnam((char)parity)
                       );
        pctlbl = (what & W_SEND);
        cendw = 0;
    }
    debug(F101,"SCREENC switch","",f);
    debug(F000,"SCREENC c","",c);
    debug(F101,"SCREENC n","",n);

    len = strlen(s);                    /* Length of argument string */
    switch (f) {                        /* Handle our function code */
      case SCR_FN:                      /* Filename */
        oldpct = pct = 0L;              /* Reset percents */
#ifdef GFTIMER
        gtv = (CKFLOAT) -1.0;
        /* oldgtv = (CKFLOAT) -1.0; */
#else
        gtv = -1L;
        /* oldgtv = -1L; */
#endif /* GFTIMER */
        oldwin = -1;
        fsiz = (CK_OFF_T)-1L;		/* Invalidate previous file size */
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_PCD, (long) 0 );
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_FFC, (long) 0 );
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_SIZ, (long) 0 );
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_ERR, (long) "" );

        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_NAM, (long) s );
        q = len;                        /* Remember name length for later */
        scrft();                        /* Display file type (can change) */
#ifdef OS2
        SaveCmdMode(0, 0);
#endif /* OS2 */
        return;

      case SCR_AN:                      /* File as-name */
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_NAM, (long) s );
#ifdef OS2
        SaveCmdMode(0, 0);
#endif /* OS2 */
        return;

      case SCR_FS:                      /* File size */
        fsiz = n;
        if (fsiz > (CK_OFF_T)-1) {
            KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_SIZ, (long) n );
        }
        if (fsiz > (CK_OFF_T)-1) {	/* Put up percent label */
            pctlbl = 1;
        } else {
            pctlbl = 0;
	}
        scrft();                        /* File type */
#ifdef OS2
        SaveCmdMode(0, 0);
#endif /* OS2 */
        return;

      case SCR_PT:                      /* Packet type or pseudotype */
        if (spackets < 5) {
            extern int sysindex;
            extern struct sysdata sysidlist[];
            /* Things that won't change after the 4th packet */
            KuiSetProperty( KUI_FILE_TRANSFER,
                           (long) CW_PAR,
                           (long) parnam((char)parity)
                           );
            if (
#ifdef NEWFTP
		(ftp && (spackets == 1 || rpackets == 1)) ||
#endif /* NEWFTP */
		spackets == 4
		) {
                if (
#ifdef NEWFTP
		    ftp ||
#endif /* NEWFTP */
		    ((protocol == PROTO_K) && (sysindex > -1))
		    ) {
                    char msgbuf[128];
                    if (net) {
                        sprintf(msgbuf,"Network Host: %s (%s)",
#ifdef NEWFTP
			       ftp ? (ftp_host ? ftp_host : "") :
#endif /* NEWFTP */
			       ttname,
#ifdef NEWFTP
			       ftp ? ftp_srvtyp :
#endif /* NEWFTP */
			       sysidlist[sysindex].sid_name
			       );
                    } else {
                        sprintf(msgbuf,
				"Communication Device: %s (remote host is %s)",
				ttname,
				sysidlist[sysindex].sid_name
				);
                    }
                    KuiSetProperty( KUI_FILE_TRANSFER,
				    (long) CW_LIN,
				    (long) msgbuf
				    );
                }
            }
        }
#ifdef CK_TIMERS
        if (/* rttflg && */ protocol == PROTO_K) {
            long xx;
            if (
#ifdef STREAMING
                streaming && oldwin != -2
#else
                0
#endif /* STREAMING */
                ) {
                char msgbuf[64];
                sprintf(msgbuf,"00 / 00");
                KuiSetProperty( KUI_FILE_TRANSFER,
				(long) CW_TMO,
				(long) msgbuf
				);
            } else {
                xx = (rttdelay + 500) / 1000;
                if (xx != oldrtt || rcvtimo != oldtim) {
                    char msgbuf[64];
                    sprintf(msgbuf,"%02ld / %02d", xx, rcvtimo);
                    KuiSetProperty( KUI_FILE_TRANSFER,
				    (long) CW_TMO,
				    (long) msgbuf
				    );
                    oldrtt = xx;
                    oldtim = rcvtimo;
                    clrtoeol();
                }
            }
        }
#endif /* CK_TIMERS */

        x = (what & W_RECV) ?          /* Packet length */
          rpktl+(protocol==PROTO_K?1:0) :
            spktl;
        if (x != oldlen) {              /* But only if it changed. */
            KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_PL, (long) x );
            oldlen = x;
        }
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_PC, (long) spackets );

        if (protocol == PROTO_K && !ftp) { /* Window slots */
            char ws[16];
            int flag;
            flag = 0;
#ifdef STREAMING
            if (streaming) {
                if (oldwin != -2) {
                    sprintf(ws,"STREAMING");
                    flag = 1;
                    oldwin = -2;
                }
            } else
#endif /* STREAMING */
              if (wcur != oldwin) {
                  sprintf(ws, "%d of %d", wcur < 1 ? 1 : wcur, wslotn);
                  flag = 1;
                  oldwin = wcur;
              }
            if (flag) {
                KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_WS, (long) ws );
            }
        }
        errors = retrans + crunched + timeouts;
        if (errors != oldtry) {         /* Retry count, if changed */
            KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_PR, (long) errors );
            oldtry = errors;
        }
	/* Sender's packet type */
        if (!ftp && (c != oldtyp && c != 'Y' && c != 'N')) {
            char type[2];
            sprintf(type, "%c",c);
            KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_PT, (long) type );
            oldtyp = c;
        }
        switch (c) {                    /* Now handle specific packet types */
          case 'S':                     /* Beginning of transfer */
            fcnt = fbyt = 0L;           /* Clear counters */
#ifdef GFTIMER
            gtv = -1.0;
#else /* GFTIMER */
            gtv = -1L;                  /* And old/new things... */
#endif /* GFTIMER */
            oldpct = pct = 0L;
            break;

          case 'Z':                     /* or EOF */
            debug(F101,"screeng SCR_PT Z pktnum","",n);
            debug(F101,"screeng SCR_PT Z oldpct","",oldpct);
            debug(F101,"screeng SCR_PT Z pct","",pct);
          case 'D':                     /* Data packet */
            if (fsiz > 0L) {            /* Show percent done if known */
                oldpct = pct;           /* Remember previous percent */
                howfar = ffc;
#ifdef CK_RESEND
                if (what & W_SEND)	/* Account for PSEND or RESEND */
                  howfar += sendstart;
                else if (what & W_RECV)
                  howfar += rs_len;
#endif /* CK_RESEND */
                /* Percent done, to be displayed... */
                if (c == 'Z') {
                    if (!discard && !cxseen && !czseen) pct = 100L;
                } else
                  pct = (fsiz > 99L) ? (howfar / (fsiz / 100L)) : 0L;
                if (pct > 100L ||       /* Allow for expansion and */
                   (oldpct == 99L && pct < 0L)) /* other boundary conditions */
                  pct = 100L;
                if (pct != oldpct)      /* Only do this 100 times per file */
                  updpct(oldpct, pct);
            } else {
                KuiSetProperty(KUI_FILE_TRANSFER, (long) CW_FFC, (long) ffc);
            }
            KuiSetProperty(KUI_FILE_TRANSFER, (long) CW_FFC, (long) howfar);
            cps = shocps((int) pct, fsiz, howfar);
            /* old_tr = shoetl(old_tr, cps, fsiz, howfar); */
            break;

          case '%':                     /* Timeouts, retransmissions */
            cps = shocps((int) pct, fsiz, howfar);
            /* old_tr = shoetl(old_tr, cps, fsiz, howfar); */

            errors = retrans + crunched + timeouts;
            if (errors != oldtry) {     /* Error count, if changed */
                KuiSetProperty(KUI_FILE_TRANSFER,
                               (long) CW_PR,
			       (long) errors
                               );
                }
                oldtry = errors;
                if (s) if (*s) {
                    KuiSetProperty(KUI_FILE_TRANSFER, (long) CW_ERR, (long) s);
            }
            break;

          case 'E':                     /* Error packet */
            if (*s) {
                KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_ERR, (long) s );
            }
            fcnt = fbyt = 0L;           /* So no bytes for this file */
            break;
          case 'Q':                     /* Crunched packet */
            cps = shocps((int) pct, fsiz, howfar);
            /* old_tr = shoetl(old_tr, cps, fsiz, howfar); */
            KuiSetProperty(KUI_FILE_TRANSFER,
                           (long) CW_ERR,
                           (long) "Damaged Packet"
                           );
            break;
          case 'q':                     /* Ctrl-C or connection lost */
            if (!s) s = "";
            if (!*s) s = "User interruption or connection lost";
            KuiSetProperty(KUI_FILE_TRANSFER,
                           (long) CW_MSG,
                           (long) s
                           );
            break;
          case 'T':                     /* Timeout */
            cps = shocps((int) pct, fsiz, howfar);
            /* old_tr = shoetl(old_tr, cps, fsiz, howfar); */
            KuiSetProperty(KUI_FILE_TRANSFER,
                           (long) CW_ERR,
                           (long) "Timeout"
                           );
            errors = retrans + crunched + timeouts;
            if (errors != oldtry) {     /* Error count, if changed */
                KuiSetProperty(KUI_FILE_TRANSFER,
                               (long) CW_PR, (long) errors
                               );
                oldtry = errors;
            }
            break;
          default:                      /* Others, do nothing */
            break;
        }
#ifdef OS2
        SaveCmdMode(0, 0);
#endif /* OS2 */
        return;

      case SCR_ST:                      /* File transfer status */
        debug(F101,"screeng SCR_ST c","",c);
        debug(F101,"screeng SCR_ST success","",success);
        debug(F101,"screeng SCR_ST cxseen","",cxseen);
#ifdef COMMENT
        if (c == ST_OK) {               /* OK, print 100 % */
            if (pctlbl)
              updpct(oldpct,100);
            else
                KuiSetProperty(KUI_FILE_TRANSFER, (long) CW_FFC, (long) ffc);
            pct = 100;
            oldpct = 0;
        } else if (fsiz > 0L)           /* Not OK, update final percent */
/*
  The else part writes all over the screen -- howfar and/or fsiz have
  been reset as a consequence of the not-OKness of the transfer.
*/
          if (pctlbl)
            updpct(oldpct, (howfar * 100L) / fsiz);
#else
        if (c == ST_OK) {               /* OK, print 100 % */
            if (pctlbl) {
		updpct(oldpct,100);
	    } else
                KuiSetProperty(KUI_FILE_TRANSFER, (long) CW_FFC, (long) ffc);
#ifdef COMMENT
            pct = 100;
            oldpct = 0;
#endif /* COMMENT */
        }
#endif /* COMMENT */

        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_TR, (long) "" );

        switch (c) {                    /* Print new status message */
          case ST_OK:                   /* Transfer OK */
            fcnt++;                     /* Count this file */
	    if (what == (W_FTP|W_FT_DELE)) {
                KuiSetProperty(KUI_FILE_TRANSFER,
                                (long) CW_MSG,
                                (long) "Delete OK"
                                );
	    } else {
		fbyt += ffc;
                KuiSetProperty(KUI_FILE_TRANSFER,
                                (long) CW_MSG,
                                (long) "Transfer OK"
                                );
	    }
            return;

          case ST_DISC:                 /* Discarded */
            KuiSetProperty(KUI_FILE_TRANSFER,
                           (long) CW_ERR,
                           (long) "File discarded"
                           );
#ifdef COMMENT
            pct = oldpct = 0;
#endif /* COMMENT */
            return;

          case ST_INT:                  /* Interrupted */
            KuiSetProperty(KUI_FILE_TRANSFER,
                           (long) CW_ERR,
                           (long) "Transfer interrupted"
                           );
#ifdef COMMENT
            pct = oldpct = 0;
#endif /* COMMENT */
            return;

        case ST_SKIP: {                /* Skipped */
            char errbuf[64] ;
	    if (n > 0 && n < nskreason)
                sprintf( errbuf, "File skipped, (%s)", skreason[n] ) ;
	    else
                sprintf( errbuf, "File skipped" ) ;
            KuiSetProperty(KUI_FILE_TRANSFER,
                           (long) CW_ERR,
                           (long) errbuf
                           );
#ifdef COMMENT
            pct = oldpct = 0;
#endif /* COMMENT */
            return;
        }
          case ST_ERR:                  /* Error message */
            if (!s) s = (char *)epktmsg;
            KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_ERR, (long) s );
#ifdef COMMENT
            pct = oldpct = 0;
#endif /* COMMENT */
            return;

          case ST_REFU:                 /* Refused */
            if (*s) {
                char errbuf[64] ;
                sprintf( errbuf, "Refused, %s", s ) ;
                KuiSetProperty(KUI_FILE_TRANSFER,(long) CW_ERR,(long) errbuf);
            } else {
                KuiSetProperty(KUI_FILE_TRANSFER,(long)CW_ERR,(long)"Refused");
            }
#ifdef COMMENT
            pct = oldpct = 0;
#endif /* COMMENT */
            return;

          case ST_INC:
            KuiSetProperty(KUI_FILE_TRANSFER,(long)CW_ERR,(long)"Incomplete");
#ifdef COMMENT
            pct = oldpct = 0;
#endif /* COMMENT */
            return;

          case ST_MSG:
            KuiSetProperty(KUI_FILE_TRANSFER,(long)CW_MSG,(long)s);
            return;

          default:                      /* Bad call */
            KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_ERR,
                       (long) "*** screen() called with bad status ***" );
            return;
        }

      case SCR_TC: {                    /* Transaction complete */
          char msgbuf[128];
          KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_CP, tfcps);
          if (success) {
              sprintf(msgbuf,
                      "SUCCESS.  Files: %s, Bytes: %ld, %ld CPS",
                      filcnt - filrej,
                      ckfstoa(fbyt),
                      tfcps
                      );
              KuiSetProperty(KUI_FILE_TRANSFER,
                             (long) CW_MSG,
                             (long) msgbuf
                             );
          }
          KuiSetProperty(KUI_FILE_TRANSFER,
                         (long) CW_TR,
                         (long) hhmmss((long)
#ifdef GFTIMER
                                       (fptsecs + 0.5)
#else
                                       tsecs
#endif /* GFTIMER */
                                       ));

#ifdef GFTIMER
          oldgtv = (CKFLOAT) -1.0;
#else
          oldgtv = -1L;
#endif /* GFTIMER */

#ifdef COMMENT
          pct = 100; oldpct = 0;        /* Reset these for next time. */
#endif /* COMMENT */
          oldtyp = 0; oldrtt = -1L; oldtry = -1; oldlen = -1;
          oldtim = -1;
          cendw = 1;
          if (xfrbel) bleep(BP_NOTE);   /* Close window, then beep. */
          ft_win = 0;                   /* Window closed. */
          return;
      }
      case SCR_EM:                      /* Error packet (fatal) */
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_ERR, (long) s );
        if (xfrbel) bleep(BP_FAIL);
#ifdef COMMENT
        pct = oldpct = 0;
#endif /* COMMENT */
        return;

      case SCR_QE:                      /* Quantity equals */
      case SCR_TU:                      /* Undelimited text */
      case SCR_TN:                      /* Text delimited at start */
      case SCR_TZ:                      /* Text delimited at end */
        return;                         /* (ignored in fullscreen display) */

      case SCR_XD:                      /* X-packet data */
        pct = oldpct = 0;
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_NAM, (long) s );
        return;

      case SCR_CW:                      /* Close Window */
#ifdef COMMENT
        pct = 100; oldpct = 0;          /* Reset these for next time. */
#endif /* COMMENT */
        oldtyp = 0; oldrtt = -1L; oldtry = -1; oldlen = -1;
        oldtim = -1;

        ft_win = 0;                     /* Flag that window is closed. */
        cendw = 1; return;

      case SCR_CD:                      /* Display current directory */
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_DIR, (long) s );
#ifdef OS2
        SaveCmdMode(0, 0);
#endif /* OS2 */
        return;

      default:                          /* Bad call */
        KuiSetProperty(KUI_FILE_TRANSFER,
                       (long) CW_ERR,
                       (long) "*** screen() called with bad function code ***"
                       );
        return;
    }
}
#endif /* KUI */
#endif /* MAC */

#endif /* NOXFER */

#ifndef CK_CURPOS
/* Dummies for when cursor control is not supported */
int
ck_curpos(row, col) {
    return(-1);
}

int
ck_cls() {
    return(-1);
}

int
ck_cleol() {
    return(-1);
}
#endif /* CK_CURPOS */

#ifndef NOIKSD
#ifdef IKSDB

struct iksdbfld dbfld[] = {
   /* Offset    Length    Type   */
    { DB_FLAGS, dB_FLAGS, DBT_HEX },    /*  0 db_FLAGS Flags */
    { DB_ATYPE, dB_ATYPE, DBT_HEX },    /*  1 db_ATYPE Auth type */
    { DB_AMODE, dB_AMODE, DBT_HEX },    /*  3 db_AMODE Auth mode */
    { DB_STATE, dB_STATE, DBT_HEX },    /*  2 db_STATE State */
    { DB_MYPID, dB_MYPID, DBT_HEX },    /*  5 db_MYPID PID */
    { DB_SADDR, dB_SADDR, DBT_HEX },    /*  4 db_SADDR Server address */
    { DB_CADDR, dB_CADDR, DBT_HEX },    /*  6 db_CADDR Client address */
    { DB_START, dB_START, DBT_DAT },    /*  7 db_START Session start */
    { DB_LASTU, dB_LASTU, DBT_DAT },    /*  8 db_LASTU Last update */
    { DB_ULEN,  dB_ULEN,  DBT_HEX },    /*  9 db_ULEN  Username length */
    { DB_DLEN,  dB_DLEN,  DBT_HEX },    /* 10 db_DLEN  Directory name length */
    { DB_ILEN,  dB_ILEN,  DBT_HEX },    /* 11 db_ILEN  Info length */
    { DB_PAD1,  dB_PAD1,  DBT_UND },    /* 12 db_PAD1  (Reserved) */
    { DB_USER,  dB_USER,  DBT_STR },    /* 13 db_USER  Username */
    { DB_DIR,   dB_DIR,   DBT_STR },    /* 14 db_DIR   Current Directory */
    { DB_INFO,  dB_INFO,  DBT_STR }     /* 15 db_INFO  State-specific info */
};

static char lcknam[CKMAXPATH+1];        /* Lockfile pathname */
static char tmplck[CKMAXPATH+1];        /* Temporary lockfile name */

static char * updmode =                 /* Update mode for fopen() */
#ifdef OS2
  "r+b"
#else
#ifdef VMS
  "r+b"
#else
  "r+"
#endif /* VMS */
#endif /* OS2 */
  ;

/*  D B I N I T  --  Initialize the IKSD database...  */

int
dbinit() {
    extern int dbinited;
    int x = 0;
    debug(F110,"dbinit dbdir 1",dbdir,0);
    debug(F110,"dbinit dbfile 1",dbfile,0);
    if (dbinited)
      return(0);
#ifdef OS2
    if (!dbdir) {
#ifdef NT
        char * p = NULL;
        if (!isWin95()) {
            p = getenv("SystemRoot");
        } else {
            p = getenv("winbootdir");
            if (!p)  p = getenv("windir");
        }
        if (!p) p = "C:/";
        dbdir = malloc(strlen(p)+2);
        strcpy(dbdir,p);		/* safe */
        p = dbdir;
        while (*p) {
            if (*p == '\\')
              *p = '/';
            p++;
        }
        if (*(p-1) != '/' ) {
            *p++ = '/';
            *p = '\0';
        }
#else /* NT */
        makestr(&dbdir,"C:/");
#endif /* NT */
    }
#else /* OS2 */
    if (!dbdir)
      makestr(&dbdir,IK_DBASEDIR);
#endif /* OS2 */

    if (!dbfile) {
        char * s = "";
        x = strlen(dbdir);
        if (dbdir[x-1] != '/') {
            s = "/";
            x++;
        }
        x += (int)strlen(IK_DBASEFIL);
        dbfile = (char *)malloc(x+1);
        sprintf(dbfile,"%s%s%s",dbdir,s,IK_DBASEFIL);
    }
    debug(F110,"dbinit dbdir 2",dbdir,0);
    debug(F110,"dbinit dbfile 2",dbfile,0);
    mypid = getpid();                   /* Get my pid */
    debug(F101,"dbinit mypid","",mypid);

    if (!myhexip[0]) {                  /* Set my hex IP address */
#ifdef TCPSOCKET
        extern unsigned long myxipaddr;
        if (getlocalipaddr() > -1) {
            myip = myxipaddr;
            sprintf(myhexip,"%08lx",myip); /* (Needs fixing for IPv6) */
        } else
#endif /* TCPSOCKET */
          ckstrncpy(myhexip,"00000000",9);
    }
    debug(F111,"dbinit myip",myhexip,myip);
    if (!peerhexip[0]) {                /* Get peer's  hex IP address */
#ifdef TCPSOCKET
        extern unsigned long peerxipaddr;
        if (ckgetpeer()) {
            peerip = peerxipaddr;
            sprintf(peerhexip,"%08lx",peerip); /* (Needs fixing for IPv6) */
            debug(F111,"dbinit peerip",peerhexip,peerip);
        } else {
            debug(F101,"dbinit ckgetpeer failure","",errno);
            ckstrncpy(peerhexip,"00000000",9);
        }
#else
        ckstrncpy(peerhexip,"00000000",9);
#endif /* TCPSOCKET */
    }
    debug(F111,"dbinit peerip",peerhexip,peerip);
    debug(F101,"dbinit dbenabled","",dbenabled);
    if (dbenabled && inserver) {
        mydbslot = getslot();
        debug(F111,"dbinit getslot",ckitoa(ikdbopen),x);
        if (ikdbopen) dbinited = 1;
    }
    return(0);
}

/*  U P D S L O T  --  Update slot n  */

/*
  Opens the database if necessary, seeks to slot n, writes current record
  and adds current time to last-update field.  n is the record sequence number
  (0, 1, 2, ...), not the seek pointer.   Returns -1 on failure, 0 on success.
*/
int
updslot(n) int n; {                     /* Update our slot */
    int rc = 0;
    CK_OFF_T position;

    debug(F111,"updslot","ikdbopen",ikdbopen);
    if (!ikdbopen)                      /* Not if not ok */
      return(0);
    if (!dbfp) {                        /* Open database if not open */
        dbfp = fopen(dbfile,updmode);   /* In update no-truncate mode */
        if (!dbfp) {
            debug(F110,"updslot fopen failed",dbfile,0);
            ikdbopen = 0;
            return(-1);
        }
    }
    /* debug(F111,"updslot dbfile",dbfile,dbfp); */
    position = n * DB_RECL;
    if (CKFSEEK(dbfp,position,0) < 0) {	/* Seek to desired slot */
        debug(F111,"updslot fseek failed",dbfile,mydbseek);
        ikdbopen = 0;
        rc = -1;
    } else {
        /* Update the update time */
        strncpy(&dbrec[dbfld[db_LASTU].off],
                ckdate(),
                dbfld[db_LASTU].len
                );
        if (fwrite(dbrec,1,DB_RECL,dbfp) < DB_RECL) { /* Write the record */
            debug(F110,"updslot fwrite failed",dbfile,0);
            ikdbopen = 0;
            rc = -1;
        } else {                        /* Flush the write */
            fflush(dbfp);
        }
    }
    return(rc);
}

/*  I N I T S L O T --  Initialize slot n with my info  */

int
initslot(n) int n; {                    /* Initialize slot */
    int k;
#ifdef TCPSOCKET
    extern unsigned long peerxipaddr;
#endif /* TCPSOCKET */

    debug(F101,"initslot","",n);

#ifdef USE_MEMCPY
    memset(dbrec,32,DB_RECL);
#else
    for (k = 0; k < DB_RECL; k++)
      dbrec[k] = '\040';
#endif /* USE_MEMCPY */

    myflags = DBF_INUSE;                /* Set in-use flag */
    mystate = W_NOTHING;
    myatype = 0L;
    myamode = 0L;

    k = dbfld[db_FLAGS].len;            /* Length of flags field */
    strncpy(&dbrec[dbfld[db_FLAGS].off],ulongtohex(myflags,k),k);

    k = dbfld[db_ATYPE].len;
    strncpy(&dbrec[dbfld[db_ATYPE].off],ulongtohex(myatype,k),k);

    k = dbfld[db_AMODE].len;
    strncpy(&dbrec[dbfld[db_AMODE].off],ulongtohex(myamode,k),k);

    k = dbfld[db_STATE].len;
    strncpy(&dbrec[dbfld[db_STATE].off],ulongtohex(mystate,k),k);

    k = dbfld[db_SADDR].len;
    strncpy(&dbrec[dbfld[db_SADDR].off],ulongtohex(myip,k),k);

#ifdef TCPSOCKET
    ckgetpeer();
    k = dbfld[db_CADDR].len;
    strncpy(&dbrec[dbfld[db_CADDR].off],ulongtohex(peerxipaddr,k),k);
#else
    k = dbfld[db_CADDR].len;
    strncpy(&dbrec[dbfld[db_CADDR].off],ulongtohex(0L,k),k);
#endif /* TCPSOCKET */

    k = dbfld[db_MYPID].len;
    strncpy(&dbrec[dbfld[db_MYPID].off],ulongtohex(mypid,k),k);

    k = dbfld[db_START].len;
    strncpy(&dbrec[dbfld[db_START].off],ckdate(),k);

    k = dbfld[db_ULEN].len;
    strncpy(&dbrec[dbfld[db_ULEN].off],"0000",4);

    k = dbfld[db_DLEN].len;
    strncpy(&dbrec[dbfld[db_DLEN].off],"0000",4);

    k = dbfld[db_ILEN].len;
    strncpy(&dbrec[dbfld[db_ILEN].off],"0000",4);

    strncpy(&dbrec[dbfld[db_INFO].off],"INIT",4);
    return(updslot(n));
}

int
slotstate(x,s1,s2,s3) int x; char *s1, *s2, *s3; {
    int k, l1, l2, l3, z;
    mystate = x;
    debug(F101,"slotstate ikdbopen","",ikdbopen);
    if (!ikdbopen)
      return(-1);
    if (!s1) s1 = "";
    l1 = strlen(s1);
    if (!s2) s2 = "";
    l2 = strlen(s2);
    if (!s3) s3 = "";
    l3 = strlen(s3);
    strncpy(&dbrec[DB_STATE],ulongtohex(mystate,4),4);
    k = dbfld[db_ILEN].len;
    z = l1 + l2 + l3 + 2;
    if (z > dB_INFO)
      z = dB_INFO;
    strncpy(&dbrec[DB_ILEN],ulongtohex((unsigned long)z,k),k);
    k = dbfld[db_INFO].len;
    z = dbfld[db_INFO].off;
    if (l1 <= k) {
        lset(&dbrec[z],s1,l1+1,32);
        z += l1+1;
        k -= l1+1;
        if (l2 <= k) {
            lset(&dbrec[z],s2,l2+1,32);
            z += l2+1;
            k -= l2+1;
            if (l3 <= k)
              lset(&dbrec[z],s3,k,32);
        }
    }
#ifdef DEBUG
    if (deblog) {
        char buf[128];
        int i;
        strncpy(buf,&dbrec[DB_INFO],127);
        buf[127] = NUL;
        for (i = 126; i > 0 && buf[i] == 32; i--) buf[i] = 0;
        debug(F111,"slotstate",buf,mystate);
    }
#endif /* DEBUG */
    z = updslot(mydbslot);
    debug(F101,"slotstate updslot","",z);
    return(z);
}

int
slotdir(s1,s2) char * s1, * s2; {       /* Update current directory */
    int k, len1, len2;
    if (!ikdbopen)
      return(-1);
    if (!s1) s1 = "";
    if (!s2) s2 = "";
    len1 = strlen(s1);
    len2 = strlen(s2);
    k = dbfld[db_DLEN].len;
    strncpy(&dbrec[DB_DLEN],ulongtohex((unsigned long)(len1+len2),k),k);
    k = dbfld[db_DIR].len;
    if (len1 > 0) {
        lset(&dbrec[dbfld[db_DIR].off],s1,len1,32);
        lset(&dbrec[dbfld[db_DIR].off+len1],s2,k-len1,32);
    } else {
        lset(&dbrec[dbfld[db_DIR].off],s2,k,32);
    }
    return(updslot(mydbslot));
}

/*  F R E E S L O T  --  Free slot n  */

int
freeslot(n) int n; {
    int k;
    if (!ikdbopen)
      return(0);
    dbflags = 0L;
    if (n == mydbslot) {
        dbflags = myflags & ~DBF_INUSE;
        dbflags &= ~DBF_LOGGED;
    }
    k = dbfld[db_FLAGS].len;
    strncpy(&dbrec[dbfld[db_FLAGS].off],ulongtohex(dbflags,k),k);
    return(updslot(n));
}

/*  G E T S L O T  --  Find a free database slot; returns slot number  */

#ifdef UNIX
#include <fcntl.h>			/* For creat() */
#endif	/* UNIX */

int
getslot() {                             /* Find a free slot for us */
    FILE * rfp = NULL;                  /* Returns slot number (0, 1, ...) */
    char idstring[64];                  /* PID string buffer (decimal) */
    char pidbuf[64], * s;
    int j, k, n, x, rc = -1;
    int lockfd, tries, haveslot = 0;
    long lockpid;
    CK_OFF_T i;
    /* char ipbuf[17]; */

    if (!myhexip[0])                    /* Set my hex IP address if not set */
      ckstrncpy((char *)myhexip,"7F000001",33);
    sprintf(idstring,"%08lx:%010ld\n",myip,mypid);
    debug(F110,"getslot idstring", idstring, 0);

    /* Make temporary lockfile name IP.PID (hex.hex) */
    /* This should fit in 14 chars -- huge PIDs are usually not possible */
    /* on 14-char filename systems. */

    sprintf(tmplck,"%s%08lx.%lx",dbdir,myip,mypid);
    debug(F110,"getslot tempfile",tmplck,0);

    /* Make a temporary file */

    lockfd = creat(tmplck, 0600);	/* BUT THIS ISN'T PORTABLE */
    if (lockfd < 0) {
        debug(F111,"getslock temp lockfile create failure", tmplck, errno);
        return(-1);
    }
    /* Write my (decimal) PID into the temp file */

    write(lockfd,idstring,(int)strlen(idstring));
    if (close(lockfd) < 0) {            /* Close lockfile */
        debug(F101,"getslot error closing temp lockfile", "", errno);
        return(-1);
    }
    sprintf(lcknam,"%s%s",dbdir,IK_LOCKFILE); /* Build lockfile name */
    debug(F110,"getslot lockfile",lcknam,0);

    rfp = fopen(lcknam,"r");            /* See if lockfile exists */
    if (rfp) {                          /* If so... */
        rset(pidbuf,"",64,0);
        x = fread(pidbuf,1,63,rfp);     /* Read ID string from it */
        fclose(rfp);                    /* and close it quickly */
        debug(F110,"getslot lock exists",pidbuf,0);
        if (x > 0) {                    /* If we have a PID, check it */
            char * s = pidbuf;
            while (*s) {
                if (islower(*s)) *s = toupper(*s);
                if (*s == ':') {
                    *s = NUL;
                    debug(F110,"getslot lock IP",pidbuf,0);
                    debug(F110,"gteslot my   IP",myhexip,0);
                    if (!strcmp(pidbuf,myhexip)) { /* Same IP address? */
                        lockpid = atol(s+1); /* Yes, now get PID */
                        debug(F101,"getslot lockpid","",lockpid);

                        /* Check if PID lockpid on this computer is alive */
                        x = zchkpid(lockpid);
                        if (!x) {
                            debug(F100,"getslot PID stale,removing lock","",0);
                            unlink(lcknam);
                        }
                        break;
                    }
                }
                s++;
            }
        } else {
            debug(F111,"getslot lockfile open failure",lcknam,errno);
        }
    }
    /* Try IK_LCKTRIES (16) times to rename temp file to lockfile */

    for (tries = IK_LCKTRIES; tries > 0; tries--) {
        if (zrename(tmplck,lcknam) == 0)
          break;
        debug(F101,"getslot database locked by pid", "", dbpid);
        sleep(IK_LCKSLEEP);
    }
    if (tries < 1) {                    /* Couldn't */
        debug(F110,"getslot create lock failure",lcknam,0);
        return(-1);
    }
    /* Have lock, open database */

    debug(F110,"getslot has lock",lcknam,0); /* Have lock */

    if (!dbfile)
      return(-1);

    /* If database doesn't exist, create it. */

    debug(F110,"getslot dbfile",dbfile,0);
    if (zchki(dbfile) < 0) {
        debug(F110,"getslot creating new database",dbfile,0);
        x = creat(dbfile,0660);
        if (x < 0) {
            debug(F111,"getslot creat() failed", dbfile, errno);
            goto xslot;
        }
        close(x);
    }
    dbfp = fopen(dbfile,updmode);       /* Open it in update mode */
    if (!dbfp) {
        debug(F111,"getslot fopen failed",dbfile,errno);
        goto xslot;
    }
    /* Now find a free (or new) slot... */

    dblastused = 0L;                    /* Seek pointer to last record inuse */
    mydbseek = 0L;                      /* Seek pointer for my record */

    /* Quickly read the whole database; n = record counter, i = seek pointer */

    for (n = 0, i = 0; !feof(dbfp); i += DB_RECL, n++) {
        x = fread(dbrec,1,DB_RECL,dbfp); /* Read a record */
        if (x < 1)                      /* EOF not caught by feof() */
          break;
#ifndef NOFTRUNCATE
        if (x != DB_RECL) {             /* Watch out for trailing junk */
            debug(F101,"getslot bad size","",x);  /* (Shouldn't happen...) */
#ifdef COHERENT
            chsize(fileno(dbfp),i);
#else
            ftruncate(fileno(dbfp),(CK_OFF_T)i);
#endif /* COHERENT */
            x = 0;
            CKFSEEK(dbfp,i,0);
            break;
        }
#endif /* NOFTRUNCATE */
        debug(F101,"getslot record","",n);
        k = dbfld[db_FLAGS].off;
        j = dbfld[db_FLAGS].len;
        dbflags = hextoulong(&dbrec[k],j);
        debug(F001,"getslot dbflags","",dbflags);
        k = dbfld[db_MYPID].off;
        j = dbfld[db_MYPID].len;
        dbpid  = hextoulong(&dbrec[k],j);
        debug(F001,"getslot dbpid","",dbpid);
        k = dbfld[db_SADDR].off;
        j = dbfld[db_SADDR].len;
        dbip = hextoulong(&dbrec[k],j);
        debug(F001,"getslot dbip","",dbip);

        if (dbflags & DBF_INUSE) {      /* Remember last slot in use */
            x = 0;                      /* Make sure it's REALLY in use */
            if (dbpid == mypid && dbip == myip) { /* Check for PID == my PID */
                x = 1;
                debug(F101,"getslot record pid","",dbpid);
            } else {                    /* Or for stale PID */
                x = zchkpid(dbpid);
                debug(F101,"getslot zchkpid()","",x);
            }
            if (!x) {                   /* Bogus record */
                x = freeslot(n);
                debug(F101,"getslot stale record pid: freeslot()","",x);
                if (x > -1 && !haveslot)
                  dbflags = 0;
            } else {                    /* It's really in use */
                dblastused = i;
            }
        }
        if (!haveslot) {                /* If I don't have a slot yet */
            if (!(dbflags & DBF_INUSE)) {       /* Claim this one */
                debug(F101,"getslot free slot", "", n);
                haveslot = 1;
                mydbseek = i;
                mydbslot = n;           /* But keep going... */
            }
        }
    }
    /* Come here with i == seek pointer to first record after eof */

    if (!haveslot) {                    /* Found no free slot so add to end */
        debug(F101,"getslot new slot","",n);
        haveslot = 1;
        mydbseek = i;
        mydbslot = n;
    }
    ikdbopen = 1;                       /* OK to make database entries */
    debug(F101,"getslot records","",n);
    debug(F101,"getslot dblastused","",dblastused);
    debug(F101,"getslot i","",i);

    /* Trim stale records from end */

#ifndef NOFTRUNCATE
    if (i > dblastused+DB_RECL) {
        debug(F101,"getslot truncating at","",dblastused+DB_RECL);
#ifdef COHERENT
        x = chsize(fileno(dbfp),dblastused+DB_RECL);
#else
        x = ftruncate(fileno(dbfp),(CK_OFF_T)(dblastused+DB_RECL));
#endif /* COHERENT */
        if (x < 0)                      /* (Not fatal) */
          debug(F101,"getslot ftruncate failed", "", errno);
    }
#endif /* NOFTRUNCATE */

    /* Initialize my record */

    if (initslot(mydbslot) < 0) {
        debug(F101,"getslot initslot() error","",n);
        ikdbopen = 0;
        goto xslot;
    }
    debug(F101,"getslot OK","",mydbslot);
    rc = mydbslot;                      /* OK return code */

  xslot:                                /* Unlock the database and return */
    if (unlink(lcknam) < 0) {
        debug(F111,"getslot lockfile removal failed",lcknam,errno);
        rc = -1;
    }
    return(rc);
}
#endif /* IKSDB */
#endif /* NOIKSD */

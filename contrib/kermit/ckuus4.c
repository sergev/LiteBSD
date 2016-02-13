#include "ckcsym.h"

/*  C K U U S 4 --  "User Interface" for C-Kermit, part 4  */

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
  File ckuus4.c -- Functions moved from other ckuus*.c modules to even
  out their sizes.
*/
#include "ckcdeb.h"
#include "ckcasc.h"
#include "ckcker.h"
#include "ckcnet.h"                     /* Network symbols */
#include "ckuusr.h"
#include "ckuver.h"
#include "ckcxla.h"                     /* Character sets */

#ifdef CK_AUTHENTICATION
#include "ckuath.h"
#endif /* CK_AUTHENTICATION */
#ifdef CK_SSL
#include "ck_ssl.h"
#endif /* CK_SSL */

#ifdef VMS
#include <errno.h>                      /* For \v(errno) */
extern char * ckvmserrstr(unsigned long);
#ifndef OLD_VMS
#include <lib$routines.h>               /* Not for VAX C 2.4 */
#else
#include <libdef.h>
#endif /* OLD_VMS */
_PROTOTYP(int vmsttyfd, (void) );
#endif /* VMS */

#ifdef OS2
#ifndef NT
#define INCL_NOPM
#define INCL_VIO                        /* Needed for ckocon.h */
#include <os2.h>
#undef COMMENT
#else
#include <windows.h>
#include <tapi.h>
#include "ckntap.h"
#define APIRET ULONG
#endif /* NT */
#include "ckocon.h"
#include "ckoetc.h"
int StartedFromDialer = 0;
HWND hwndDialer = 0;
LONG KermitDialerID = 0;
#ifdef putchar
#undef putchar
#endif /* putchar */
#define putchar(x) conoc(x)
#ifdef CK_PID
#include <process.h>
#endif /* CK_PID */
#endif /* OS2 */

#ifdef KUI
extern struct keytab * term_font;
extern int ntermfont, tt_font, tt_font_size;
#endif /* KUI */

extern xx_strp xxstring;

#ifdef DEC_TCPIP
#include <descrip>
#include <dvidef>
#include <dcdef>
#endif /* DEC_TCPIP */

#ifdef FNFLOAT
#include <math.h>                       /* Floating-point functions */
#endif /* FNFLOAT */

extern int quiet, network, xitsta, escape, nopush, xferstat,
  exitonclose, tn_exit, ttnproto, autodl, flow, byteorder, what, lastxfer;

extern int filepeek, nscanfile, makestrlen;
extern char * k_info_dir;

#ifndef MAC
#ifndef AMIGA
extern int ttyfd;
#endif /* MAC */
#endif /* AMIGA */

#ifdef TNCODE
extern int tn_nlm, tn_b_nlm, tn_b_xfer, tn_sb_bug;
extern int tn_rem_echo;
extern int tn_b_meu, tn_b_ume, tn_auth_krb5_des_bug;
#endif /* TNCODE */

static char * lastkwval = NULL;

char * xferfile = NULL;
int xferlog = 0;

extern int local, xargc, stayflg, rcflag, bgset, backgrd, cfilef,
  inserver, srvcdmsg, success;

#ifdef VMS
extern int batch;
#endif /* VMS */

extern char cmdfil[], *versio, *ckxsys, **xargv;
#ifdef DEBUG
extern char debfil[];                   /* Debug log file name */
extern int debtim;
#endif /* DEBUG */

extern int noinit;

static char ndatbuf[10];

char *months[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

char *
zzndate() {                             /* Returns today's date as yyyymmdd */
    char * p = NULL;
    int x;

/* WARNING - This will fail if asctime() returns non-English month names */

    ztime(&p);                          /* Get "asctime" string */
    if (p == NULL || *p == NUL) return("");
    for (x = 20; x < 24; x++)           /* yyyy */
      ndatbuf[x - 20] = p[x];
    ndatbuf[6] = (char) ((p[8] == ' ') ? '0' : p[8]);
    ndatbuf[7] = p[9];                  /* dd */
    for (x = 0; x < 12; x++)            /* mm */
      if (!strncmp(p+4,months[x],3)) break;
    if (x == 12) {
        ndatbuf[4] = ndatbuf[5] = '?';
    } else {
        x++;
        ndatbuf[4] = (char) ((x < 10) ? '0' : '1');
        ndatbuf[5] = (char) ((x % 10) + 48);
    }
    ndatbuf[8] = NUL;
    debug(F110,"zzndate return",ndatbuf,0);
    return((char *)ndatbuf);
}

#ifdef DCMDBUF
extern struct cmdptr *cmdstk;
extern char *line, *tmpbuf;
#else
extern struct cmdptr cmdstk[];
extern char line[], tmpbuf[];
#endif /* DCMDBUF */

#ifdef OS2
extern char exedir[];
#else
extern char * exedir;
#endif /* OS2 */

extern int nettype;

#ifndef NOICP                           /* Most of this file... */
#ifdef CKLOGDIAL
extern char diafil[];
#endif /* CKLOGDIAL */

#ifndef AMIGA
#ifndef MAC
#include <signal.h>
#endif /* MAC */
#endif /* AMIGA */

#ifdef SV68				/* July 2006 believe it or not */
#ifndef SEEK_CUR
#include <unistd.h>
#endif	/* SEEK_CUR */
#endif	/* SV68 */

#ifdef SCO32				/* June 2011 believe it or not... */
#ifdef XENIX
#ifndef SEEK_CUR
#include <unistd.h>
#endif	/* SEEK_CUR */
#endif	/* XENIX */
#endif	/* SCO32 */

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


#ifdef ANYX25
extern int revcall, closgr, cudata;
int x25ver;
extern char udata[];
#ifndef IBMX25
extern int npadx3;
extern CHAR padparms[];
extern struct keytab padx3tab[];
#endif /* !IBMX25 */
#ifdef IBMX25
/* global variables only available for IBM X.25 - possibly interesting for
 * other implementations
 */
extern x25addr_t local_nua;
extern x25addr_t remote_nua;
#endif /* IBMX25 */
#endif /* ANYX25 */

#ifdef NETCONN
#ifndef NODIAL
extern int nnetdir;
extern char *netdir[];
#endif /* NODIAL */
extern char ipaddr[];

#ifdef CK_NETBIOS
extern unsigned short netbiosAvail;
extern unsigned long NetbeuiAPI;
extern unsigned char NetBiosName[];
extern unsigned char NetBiosAdapter;
extern unsigned char NetBiosLSN;
#endif /* CK_NETBIOS */

#ifdef TCPSOCKET
extern char myipaddr[];
extern int tcp_rdns;
#ifdef CK_DNS_SRV
extern int tcp_dns_srv;
#endif /* CK_DNS_SRV */
extern char * tcp_address;
#ifndef NOHTTP
extern char * tcp_http_proxy;
#endif /* NOHTTP */
#ifdef NT
#ifdef CK_SOCKS
extern char * tcp_socks_svr;
#ifdef CK_SOCKS_NS
extern char * tcp_socks_ns;
#endif /* CK_SOCKS_NS */
#endif /* CK_SOCKS */
#endif /* NT */

#ifndef NOTCPOPTS
#ifdef SOL_SOCKET
#ifdef SO_LINGER
extern int tcp_linger;
extern int tcp_linger_tmo;
#endif /* SO_LINGER */
#ifdef SO_DONTROUTE
extern int tcp_dontroute;
#endif /* SO_DONTROUTE */
#ifdef TCP_NODELAY
extern int tcp_nodelay;
#endif /* TCP_NODELAY */
#ifdef SO_SNDBUF
extern int tcp_sendbuf;
#endif /* SO_SNDBUF */
#ifdef SO_RCVBUF
extern int tcp_recvbuf;
#endif /* SO_RCVBUF */
#ifdef SO_KEEPALIVE
extern int tcp_keepalive;
#endif /* SO_KEEPALIVE */
#endif /* SOL_SOCKET */
#endif /* NOTCPOPTS */
#endif /* TCPSOCKET */
#endif /* NETCONN */

extern char * floname[];

#ifndef NOSPL
extern int vareval;			/* Variable evaluation method */
extern int fndiags;                     /* Function diagnostics on/off */
extern int divbyzero;
int itsapattern = 0;
int isinbuflen = 0;
int isjoin = 0;
#ifdef CK_APC
extern int apcactive;                   /* Nonzero = APC command was rec'd */
extern int apcstatus;                   /* Are APC commands being processed? */
#ifdef DCMDBUF
extern char *apcbuf;                    /* APC command buffer */
#else
extern char apcbuf[];
#endif /* DCMDBUF */
#endif /* CK_APC */

extern char evalbuf[];                  /* EVALUATE result */
extern char uidbuf[], pwbuf[], prmbuf[];
_PROTOTYP( static char * fneval, (char *, char * [], int, char * ) );
_PROTOTYP( static VOID myflsh, (void) );
_PROTOTYP( static char * getip, (char *) );
_PROTOTYP( int delta2sec, (char *, long *) );

#ifdef NEWFTP
_PROTOTYP( char * ftp_cpl_mode, (void) );
_PROTOTYP( char * ftp_dpl_mode, (void) );
_PROTOTYP( char * ftp_authtype, (void) );
#endif /* NEWFTP */

#ifndef NOHTTP
_PROTOTYP( char * http_host, (void) );
_PROTOTYP( int http_isconnected, (void) );
_PROTOTYP( char * http_security, (void) );
#endif /* NOHTTP */

#ifndef NOSEXP
_PROTOTYP( char * dosexp, (char *) );
int fsexpflag = 0;
#endif /* NOSEXP */

static char hexdigits[16] = {
    '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
};
extern char * tempdir;

#ifdef CK_REXX
extern char rexxbuf[];
#endif /* CK_REXX */

extern int tfline[];

char *wkdays[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};
#endif /* NOSPL */

#ifdef OS2
extern char startupdir[], inidir[];
#else
#ifdef VMSORUNIX
extern char startupdir[];
#endif /* VMSORUNIX */
#endif /* OS2 */

#ifdef OS2
_PROTOTYP (int os2getcp, (void) );
#ifdef TCPSOCKET
extern char tcpname[];
#endif /* TCPSOCKET */
extern int tcp_avail;
#ifdef DECNET
extern int dnet_avail;
#endif /* DECNET */
#ifdef SUPERLAT
extern int slat_avail;
#endif /* SUPERLAT */

#ifndef NOTERM
extern int tt_type, max_tt;
extern struct tt_info_rec tt_info[];
#endif /* NOTERM */
extern int tt_rows[], tt_cols[];
#else /* OS2 */
extern int tt_rows, tt_cols;
#endif /* OS2 */

#ifdef CK_TAPI
extern int tttapi;
extern int tapipass;
extern struct keytab * tapilinetab;
extern struct keytab * _tapilinetab;
extern int ntapiline;
#endif /* CK_TAPI */

extern struct keytab colxtab[];
extern int ncolx;

extern char ttname[], *zinptr, *kermrc;
extern char inidir[];

extern int activecmd, remonly, cmd_rows, cmd_cols, parity, seslog,
  sessft, sosi, hwparity, tsecs, xargs, zincnt, tlevel, insilence, cmdmsk,
  timint, timef, inbufsize, dialog, binary, carrier, cdtimo, cmask, duplex,
  fmask, inecho, nmac, turnch, turn, kbchar;

#ifndef NOXFER
extern CHAR eol,  mypadc, mystch, padch, seol, stchr, * epktmsg, feol;
extern char *cksysid;
extern struct ck_p ptab[];
extern int
  protocol, prefixing, xfrbel, xfrcan, xfrint, xfrchr, xfrnum, pktpaus,
  lscapr, lscapu, xfermode, dest, slostart, maxrps, maxsps, maxtry, mypadn,
  npad, pkttim, bigrbsiz, bigsbsiz, keep, atcapr, autopar, bctr, bctu,
  crunched, ckdelay, ebq, ebqflg, pktlog, retrans, rpackets, rptflg, rptq,
  rtimo, spackets, spsiz, spsizf, spsizr, timeouts, fncact, fncnv, urpsiz,
  wmax, wslotn, wslotr, fdispla, spmax, fnrpath, fnspath, crc16;
#endif /* NOXFER */

#ifdef OS2
extern int zxpn;
extern int viewonly;
#endif /* OS2 */

#ifndef NOXFER
#ifdef GFTIMER
extern CKFLOAT fptsecs, fpxfsecs;
#endif /* GFTIMER */
extern long xfsecs, tfcps;

#ifdef CK_TMPDIR
extern char *dldir;
#endif /* CK_TMPDIR */
#endif /* NOXFER */

#ifdef RECURSIVE
extern int recursive;
#endif /* RECURSIVE */

#ifdef VMS
extern int frecl;
#endif /* VMS */

extern CK_OFF_T ffc, tfc, tlci, tlco;
extern long filcnt, rptn, speed,  ccu, ccp, vernum, xvernum;

#ifndef NOSPL
extern char fspec[], myhost[];
#endif /* NOSPL */

extern char *tfnam[];                   /* Command file names */

extern char pktfil[],                   /* Packet log file name */
#ifdef TLOG
  trafil[],                             /* Transaction log file name */
#endif /* TLOG */
  sesfil[];                             /* Session log file name */

#ifndef NOXMIT                          /* TRANSMIT command variables */
extern char xmitbuf[];
extern int xmitf, xmitl, xmitx, xmits, xmitw, xmitt;
#endif /* NOXMIT */

extern int cmdlvl;

#ifndef NOSPL
/* Script programming language items */
extern char **a_ptr[];                  /* Arrays */
extern int a_dim[];
static char * inpmatch = NULL;
#ifdef CKFLOAT
char * inpscale = NULL;
#endif	/* CKFLOAT */
extern char * inpbuf, inchar[];         /* Buffers for INPUT and REINPUT */
extern char *inpbp;                     /* And pointer to same */
static char *r3 = (char *)0;
extern int incount;                     /* INPUT character count */
extern int m_found;                     /* MINPUT result */
extern int maclvl;                      /* Macro invocation level */
extern struct mtab *mactab;             /* Macro table */
extern char *mrval[];
extern int macargc[], topargc;

#ifdef COMMENT
extern char *m_line[];
extern char *topline;
#endif /* COMMENT */

extern char *m_arg[MACLEVEL][10]; /* You have to put in the dimensions */
extern char *g_var[GVARS];        /* for external 2-dimensional arrays. */
#ifdef DCMDBUF
extern int *count, *inpcas;
#else
extern int count[], inpcas[];
#endif /* DCMDBUF */
#endif /* NOSPL */

#ifdef UNIX
extern int haslock;                     /* For UUCP locks */
extern char flfnam[];
#ifndef USETTYLOCK
extern char lock2[];
#endif /* USETTYLOCK */
#endif /* UNIX */

#ifdef OS2ORUNIX
extern int maxnam, maxpath;             /* Longest name, path length */
#endif /* OS2ORUNIX */

extern int mdmtyp, mdmsav;

#ifndef NODIAL
/* DIAL-related variables */
extern char modemmsg[];
extern MDMINF *modemp[];                /* Pointers to modem info structs */
extern int nmdm, dialhng, dialtmo, dialksp, dialdpy, dialsrt, dialsta;
extern int dialrtr, dialint, dialrstr, dialcon, dialcq, dialfld;
extern int mdmspd, dialec, dialdc, dialmth, dialmauto, dialesc;
extern char *dialnum,   *dialini,  *dialdir[], *dialcmd,  *dialnpr,
 *dialdcon, *dialdcoff, *dialecon, *dialecoff, *dialhcmd, *diallac,
 *dialhwfc, *dialswfc,  *dialnofc, *dialpulse, *dialtone, *dialname,
 *dialaaon, *dialaaoff, *dialmac;
extern char *diallcc,   *dialixp,  *dialixs,   *dialldp,  *diallds,
 *dialpxi,  *dialpxo,   *dialsfx,  *dialtfp;
extern char *diallcp,   *diallcs;
extern int ntollfree, ndialpxx, nlocalac;
extern char *dialtfc[], *diallcac[], *dialpxx[], *matchpxx;
extern int ndialpucc, ndialtocc;
extern char *dialtocc[], *dialpucc[];
extern int ndialdir, dialcnf, dialcvt, dialidt, dialpace;
extern long dialmax, dialcapas;

extern struct keytab mdmtab[];

#ifdef BIGBUFOK
#define ARGBUFSIZ 8191
#else
#define ARGBUFSIZ 1023
#endif /* BIGBUFOK */

#ifdef BIGBUFOK
extern char * dialmsg[];
#endif /* BIGBUFOK */

#endif /* NODIAL */

#ifndef NOCSETS
/* Translation stuff */
extern int nfilc;
extern struct keytab fcstab[];
extern int fcharset, tcharset, tslevel, language, nlng, tcsr, tcsl;
extern int dcset7, dcset8;
extern struct keytab lngtab[];
extern struct csinfo fcsinfo[], tcsinfo[];
extern struct langinfo langs[];
#ifdef CK_ANSIC
extern CHAR (*xls[MAXTCSETS+1][MAXFCSETS+1])(CHAR); /* Character set */
extern CHAR (*xlr[MAXTCSETS+1][MAXFCSETS+1])(CHAR); /* translation functions */
#else
extern CHAR (*xls[MAXTCSETS+1][MAXFCSETS+1])(); /* Character set */
extern CHAR (*xlr[MAXTCSETS+1][MAXFCSETS+1])(); /* translation functions. */
#endif /* CK_ANSIC */
#ifdef UNICODE
    extern int ucsbom, ucsorder;
#endif /* UNICODE */
#endif /* NOCSETS */

#ifndef NOSPL
/* Built-in variable names, maximum length VNAML (20 characters) */

struct keytab vartab[] = {
    { "_line",     VN_TFLN,  CM_INV},   /* 192 */
#ifdef OS2
    { "_regname",  VN_REGN,  CM_INV},   /* 1.1.12 */
    { "_regorg",   VN_REGO,  CM_INV},   /* 1.1.12 */
    { "_regnum",   VN_REGS,  CM_INV},   /* 1.1.12 */
#endif /* OS2 */
    { "apcactive", VN_APC,   CM_INV},   /* 192 */
#ifdef NT
    { "appdata",   VN_APPDATA, 0},      /* 201 */
#endif /* NT */
    { "argc",      VN_ARGC,  0},
    { "args",      VN_ARGS,  0},
    { "authname",  VN_AUTHN, 0},        /* 196 */
    { "authstate", VN_AUTHS, 0},        /* 195 */
    { "authtype",  VN_AUTHT, 0},        /* 195 */
    { "bits",      VN_BITS,  0},        /* 212 */
    { "blockcheck",VN_BLK,   0},        /* 195 */
#ifdef BROWSER
    { "browser",   VN_BROWSR,0},        /* 193 */
    { "browsopts", VN_BROPT, 0},        /* 193 */
    { "browsurl",  VN_URL,   0},        /* 193 */
    { "buildid",   VN_BUILD, 0},        /* 199 */
#endif /* BROWSER */
    { "byteorder", VN_BYTE,  0},        /* 195 */
#ifndef NOCSETS
    { "charset",   VN_CSET,  0},        /* 192 */
#endif /* NOCSETS */
    { "cmdbufsize",VN_CMDBL, 0},        /* 195 */
    { "cmdfile",   VN_CMDF,  0},
    { "cmdlevel",  VN_CMDL,  0},
    { "cmdsource", VN_CMDS,  0},
    { "cols",      VN_COLS,  0},        /* 190 */
#ifdef NT
    { "common",    VN_COMMON, 0},       /* 201 */
#endif /* NT */
    { "connection",VN_CONN,  0},        /* 190 */
    { "count",     VN_COUN,  0},
#ifndef NOXFER
    { "cps",       VN_CPS,   0},        /* 190 */
#endif /* NOXFER */
    { "cpu",       VN_CPU,   0},
#ifndef NOXFER
    { "crc16",     VN_CRC16, 0},        /* 192 */
    { "ctty",      VN_TTYNAM,0},        /* 196 */
#endif /* NOXFER */
#ifndef NOLOGDIAL
#ifndef NOLOCAL
    { "cx_time",   VN_CXTIME,0},        /* 195 */
    { "cx_status", VN_CX_STA,0},        /* 199 */
#endif /* NOLOCAL */
#endif /* NOLOGDIAL */
#ifndef NODIAL
    { "d$ac",      VN_D_AC,  0},        /* 192 */
    { "d$cc",      VN_D_CC,  0},        /* 192 */
    { "d$ip",      VN_D_IP,  0},        /* 192 */
    { "d$lc",      VN_D_LCP, 0},        /* 193 */
    { "d$lcp",     VN_D_LCP, CM_INV},   /* 193 */
    { "d$lp",      VN_D_LP,  0},        /* 192 */
    { "d$px",      VN_D_PXX, 0},        /* 195 */
    { "d$pxx",     VN_D_PXX, CM_INV},   /* 195 */
#endif /* NODIAL */
    { "date",      VN_DATE,  0},
    { "day",       VN_DAY,   0},
#ifdef NT
    { "desktop",   VN_DESKTOP, 0},     /* 201 */
#endif /* NT */
#ifndef NODIAL
    { "dialcount", VN_DRTR,  0},        /* 195 */
    { "dialmessage",VN_DMSG, 0},	/* 212 */
    { "dialnumber",VN_DNUM,  0},        /* 192 */
    { "dialresult",VN_MDMSG, 0},        /* 192 */
    { "dialstatus",VN_DIAL,  0},        /* 190 */
    { "dialsuffix",VN_PDSFX, 0},        /* 193 */
    { "dialtype",  VN_DTYPE, 0},        /* 193 */
#endif /* NODIAL */
    { "directory", VN_DIRE,  0},
#ifndef NODIAL
    { "dm_hf",     VN_DM_HF, 0},        /* 199 */
    { "dm_lp",     VN_DM_LP, 0},        /* 195 */
    { "dm_sp",     VN_DM_SP, 0},        /* 195 */
    { "dm_pd",     VN_DM_PD, 0},        /* 195 */
    { "dm_td",     VN_DM_TD, 0},        /* 195 */
    { "dm_wa",     VN_DM_WA, 0},        /* 195 */
    { "dm_wb",     VN_DM_WB, 0},        /* 199 */
    { "dm_wd",     VN_DM_WD, 0},        /* 195 */
    { "dm_rc",     VN_DM_RC, 0},        /* 195 */
#endif /* NODIAL */
#ifndef NOXFER
    { "download",  VN_DLDIR, 0},        /* 192 */
#endif /* NOXFER */
    { "editor",    VN_EDITOR,0},
    { "editfile",  VN_EDFILE,0},
    { "editopts",  VN_EDOPT, 0},
    { "errno",     VN_ERRNO, 0},        /* 192 */
    { "errstring", VN_ERSTR, 0},        /* 192 */
    { "escape",    VN_ESC,   0},        /* 193 */
    { "evaluate",  VN_EVAL,  0},        /* 190 */
#ifdef OS2ORUNIX
    { "exedir",    VN_EXEDIR,0},        /* 192 */
#endif /* OS2ORUNIX */
    { "exitstatus",VN_EXIT,  0},
#ifdef CKCHANNELIO
    { "f_count",   VN_FCOU,  0},        /* 195 */
    { "f_error",   VN_FERR,  0},        /* 195 */
    { "f_max",     VN_FMAX,  0},        /* 195 */
    { "fileerror", VN_FERR,  CM_INV},   /* 195 */
    { "filemax",   VN_FERR,  CM_INV},   /* 195 */
#endif /* CKCHANNELIO */
    { "filename",  VN_FNAM,  0},        /* 193 */
    { "filenumber",VN_FNUM,  0},        /* 193 */
    { "filespec",  VN_FILE,  0},
    { "fsize",     VN_FFC,   0},        /* 190 */
#ifdef GFTIMER
    { "ftime",     VN_FTIME, 0},        /* 199 */
#else
    { "ftime",     VN_NTIM,  CM_INV},
#endif /* GFTIMER */
#ifndef NOFTP
#ifndef SYSFTP
    { "ftp_code",         VN_FTP_C, 0}, /* 199 */
    { "ftp_cpl",          VN_FTP_B, 0}, /* 199 */
    { "ftp_connected",    VN_FTP_X, 0}, /* 199 */
    { "ftp_dpl",          VN_FTP_D, 0}, /* 199 */
    { "ftp_getputremote", VN_FTP_G, 0}, /* 199 */
    { "ftp_host",         VN_FTP_H, 0}, /* 199 */
    { "ftp_loggedin",     VN_FTP_L, 0}, /* 199 */
    { "ftp_message",      VN_FTP_M, 0}, /* 199 */
    { "ftp_msg",          VN_FTP_M, CM_INV}, /* 199 */
    { "ftp_security",     VN_FTP_Z, 0}, /* 199 */
    { "ftp_server",       VN_FTP_S, 0}, /* 199 */
#endif /* SYSFTP */
#endif /* NOFTP */
    { "ftype",     VN_MODE,  0},        /* 190 */
#ifdef KUI
    { "gui_fontname", VN_GUI_FNM, 0},	/* 205 */
    { "gui_fontsize", VN_GUI_FSZ, 0},	/* 205 */
    { "gui_runmode", VN_GUI_RUN, 0},    /* 205 */
    { "gui_xpos",    VN_GUI_XP,  0},    /* 205 */
    { "gui_xres",    VN_GUI_XR,  0},    /* 205 */
    { "gui_ypos",    VN_GUI_YP,  0},    /* 205 */
    { "gui_yres",    VN_GUI_YR,  0},    /* 205 */
#endif /* KUI */
    { "herald",    VN_HERALD, 0},
    { "home",      VN_HOME,   0},
    { "host",      VN_HOST,   0},
    { "hour",      VN_HOUR,   0},       /* 200 */
#ifndef NOHTTP
    { "http_code",      VN_HTTP_C, 0},  /* 199 */
    { "http_connected", VN_HTTP_N, 0},  /* 199 */
    { "http_host",      VN_HTTP_H, 0},  /* 199 */
    { "http_message",   VN_HTTP_M, 0},  /* 199 */
    { "http_security",  VN_HTTP_S, 0},  /* 199 */
#endif /* NOHTTP */
    { "hwparity",  VN_HWPAR, 0},        /* 195 */
    { "input",     VN_IBUF,  0},
    { "inchar",    VN_ICHR,  0},
    { "incount",   VN_ICNT,  0},
    { "inidir",    VN_INI,   0},        /* 192 */
    { "inmatch",   VN_MATCH, 0},        /* 196 */
    { "inmessage", VN_INPMSG,0},        /* 212 */
    { "inscale",   VN_ISCALE,0},        /* 210 */
    { "instatus",  VN_ISTAT, 0},        /* 192 */
    { "intime",    VN_INTIME,0},        /* 193 */
    { "inwait",    VN_INTMO, 0},        /* 195 */
    { "ip",        VN_IPADDR, CM_ABR|CM_INV},
    { "ipaddress", VN_IPADDR,0},        /* 192 */
    { "iprompt",   VN_PROMPT,0},        /* 199 */
    { "kbchar",    VN_KBCHAR,0},        /* 196 */
#ifndef NOLOCAL
#ifdef OS2
    { "keyboard",  VN_KEYB,  0},
#endif /* OS2 */
#endif /* NOLOCAL */
#ifdef CK_KERBEROS
    { "krb4errmsg",    VN_K4EMSG,0},
    { "krb4errno",     VN_K4ENO, 0},
    { "krb4principal", VN_K4PRN, 0},
    { "krb4realm",     VN_K4RLM, 0},
    { "krb4service",   VN_K4SRV, 0},
    { "krb5cc",        VN_K5CC,  0},
    { "krb5errmsg",    VN_K5EMSG,0},
    { "krb5errno",     VN_K5ENO, 0},
    { "krb5principal", VN_K5PRN, 0},
    { "krb5realm",     VN_K5RLM, 0},
    { "krb5service",   VN_K5SRV, 0},
#endif /* CK_KERBEROS */
    { "lastcommand",   VN_PREVCMD, 0},	/* 299 */
#ifndef NOLASTFILE
    { "lastfilespec",  VN_LASTFIL, 0},	/* 212 */
#endif	/* NOLASTFILE */
    { "lastkeywordvalue",  VN_LASTKWV, 0}, /* 212 */
    { "lastkwvalue",   VN_LASTKWV, CM_ABR|CM_INV}, /* 212 */
    { "line",          VN_LINE,  0},
    { "local",         VN_LCL,   0},
#ifdef UNIX
    { "lockdir",       VN_LCKDIR,0},	/* 195 */
    { "lockpid",       VN_LCKPID,0},	/* 195 */
#endif /* UNIX */
    { "log_connection", VN_LOG_CON, 0}, /* 206 */
    { "log_debug", VN_LOG_DEB, 0},      /* 206 */
    { "log_packet", VN_LOG_PKT, 0},     /* 206 */
    { "log_session", VN_LOG_SES, 0},    /* 206 */
    { "log_transaction", VN_LOG_TRA, 0},/* 206 */
    { "maclevel",  VN_MACLVL,0},        /* 195 */
    { "macro",     VN_MAC,   0},
#ifdef FNFLOAT
    { "math_e",    VN_MA_E,  0},        /* 195 */
    { "math_pi",   VN_MA_PI, 0},        /* 195 */
    { "math_precision", VN_MA_PR, 0},   /* 195 */
#endif /* FNFLOAT */
    { "minput",    VN_MINP,  0},        /* 192 */
    { "model",     VN_MODL,  0},        /* 193 */
    { "modem",     VN_MDM,   0},
#ifndef NOLOCAL
#ifdef OS2
    { "mousecurx", VN_MOU_X, 0},        /* K95 1.1.14 */
    { "mousecury", VN_MOU_Y, 0},        /* K95 1.1.14 */
#endif /* OS2 */
#endif /* NOLOCAL */
#ifndef NODIAL
    { "m_aa_off",  VN_M_AAX, 0},        /* all 192... */
    { "m_aa_on",   VN_M_AAO, 0},
    { "m_dc_off",  VN_M_DCX, 0},
    { "m_dc_on",   VN_M_DCO, 0},
    { "m_dial",    VN_M_DCM, 0},
    { "m_ec_off",  VN_M_ECX, 0},
    { "m_ec_on",   VN_M_ECO, 0},
    { "m_fc_hw",   VN_M_HWF, 0},
    { "m_fc_no",   VN_M_NFC, 0},
    { "m_fc_sw",   VN_M_SWF, 0},
    { "m_hup",     VN_M_HUP, 0},
    { "m_init",    VN_M_INI, 0},
    { "m_name",    VN_M_NAM, 0},        /* 195 */
    { "m_pulse",   VN_M_PDM, 0},
    { "m_sig_cd",  VN_MS_CD, 0},        /* 195 */
    { "m_sig_cts", VN_MS_CTS,0},        /* 195 */
    { "m_sig_dsr", VN_MS_DSR,0},        /* 195 */
    { "m_sig_dtr", VN_MS_DTR,0},        /* 195 */
    { "m_sig_ri",  VN_MS_RI, 0},        /* 195 */
    { "m_sig_rts", VN_MS_RTS,0},        /* 195 */
    { "m_tone",    VN_M_TDM, 0},
#endif /* NODIAL */
    { "name",      VN_NAME,  0},
    { "ndate",     VN_NDAT,  0},
    { "nday",      VN_NDAY,  0},
    { "newline",   VN_NEWL,  0},
    { "ntime",     VN_NTIM,  0},
    { "osname",    VN_OSNAM, 0},        /* 193 */
    { "osrelease", VN_OSREL, 0},        /* 193 */
    { "osversion", VN_OSVER, 0},        /* 193 */
#ifndef NOXFER
    { "packetlen", VN_RPSIZ, 0},        /* 192 */
#endif /* NOXFER */
    { "parity",    VN_PRTY,  0},        /* 190 */
    { "password",  VN_PWD,   CM_INV},   /* 192 */
#ifdef NT
    { "personal",  VN_PERSONAL, 0},     /* 201 */
#endif /* NT */
#ifdef PEXITSTAT
    { "pexitstat", VN_PEXIT, 0},        /* 193 */
#endif /* PEXITSTAT */
#ifdef CK_PID
    { "pid",       VN_PID,   0},        /* 193 */
#endif /* CK_PID */
    { "platform",  VN_SYSV,  0},
    { "printer",   VN_PRINT, 0},        /* 193 */
    { "program",   VN_PROG,  0},
    { "prompt",    VN_PRM,   CM_INV},   /* 192 */
#ifndef NOXFER
    { "protocol",  VN_PROTO, 0},        /* 192 */
    { "p_8bit",    VN_P_8BIT,0},        /* 193 */
    { "p_ctl",     VN_P_CTL, 0},        /* 193 */
    { "p_rpt",     VN_P_RPT, 0},        /* 193 */
    { "query",     VN_QUE,   0},        /* 190 */
#endif /* NOXFER */
    { "remoteip",  VN_HOSTIP,0},	/* 212 */
    { "return",    VN_RET,   0},
#ifdef CK_REXX
    { "rexx",      VN_REXX,  0},        /* 190 */
#endif /* CK_REXX */
#ifdef TN_COMPORT
    { "rfc2217_signature", VN_TNC_SIG, 0}, /* 201 */
    { "rfc2717_signature", VN_TNC_SIG, CM_INV}, /* 202 */
#endif /* TN_COMPORT */
    { "rows",      VN_ROWS,  0},        /* 190 */
#ifndef NOSEXP
    { "sdepth",    VN_LSEXP,0},         /* 199 */
#endif /* NOSEXP */
    { "secure",    VN_SECURE, 0},       /* 199 */
#ifndef NOLOCAL
#ifdef OS2
    { "select",    VN_SELCT, 0},        /* 192 */
#endif /* OS2 */
#endif /* NOLOCAL */
    { "sendlist",  VN_SNDL,  0},
    { "serial",    VN_SERIAL,0},        /* 195 */
    { "setlinemsg",VN_SLMSG, 0},        /* 195 */
#ifndef NOSEXP
    { "sexpression",VN_SEXP, 0},        /* 199 */
#endif /* NOSEXP */
    { "speed",     VN_SPEE,  0},
#ifdef OS2
    { "space",     VN_SPA,   0},
    { "startup",   VN_STAR,  0},        /* 190 */
#else
#ifdef UNIX
    { "startup",   VN_STAR,  0},        /* 193 */
#else
#ifdef VMS
    { "startup",   VN_STAR,  0},        /* 193 */
#endif /* VMS */
#endif /* UNIX */
#endif /* OS2 */
    { "status",    VN_SUCC,  0},
#ifndef NOSEXP
    { "svalue",    VN_VSEXP, 0},        /* 199 */
#endif /* NOSEXP */
#ifndef NOXFER
    { "sysid",     VN_SYSI,  0},
#endif /* NOXFER */
    { "system",    VN_SYST,  0},
    { "terminal",  VN_TTYP,  0},
#ifdef OS2
#ifndef NOKVERBS
    { "termkey",   VN_TRMK,  CM_INV},   /* 192 */
#endif /* NOKVERBS */
#endif /* OS2 */
    { "test",      VN_TEST,  0},        /* 193 */
    { "textdir",   VN_TXTDIR,0},        /* 195 */
#ifndef NOXFER
    { "tfsize",    VN_TFC,   0},
    { "tftime",    VN_TFTIM, 0},        /* 195 */
#endif /* NOXFER */
    { "time",      VN_TIME,  0},
    { "timestamp", VN_NOW,   0},        /* 200 */
    { "tmpdir",    VN_TEMP,  0},        /* 192 */
#ifdef CK_TRIGGER
    { "trigger",   VN_TRIG,  0},        /* 193 */
#endif /* CK_TRIGGER */
#ifdef CK_TTYFD
    { "ttyfd",     VN_TTYF,  0},
#endif /* CK_TTYFD */
    { "ty_ln",     VN_TY_LN, 0},        /* 195 */
    { "ty_lc",     VN_TY_LC, 0},        /* 195 */
    { "ty_lm",     VN_TY_LM, 0},        /* 195 */
#ifdef BROWSER
    { "url",       VN_URL,   CM_INV},   /* 193 */
#endif /* BROWSER */
    { "userid",    VN_UID,   0},        /* 192 */
    { "vareval",   VN_VAREVAL, 0},	/* 212 */
    { "version",   VN_VERS,  0},
#ifndef NOXFER
    { "window",    VN_WINDO, 0},        /* 192 */
#endif /* NOXFER */
#ifdef IBMX25
    { "x25local_nua", VN_X25LA, 0},     /* 193 */
    { "x25remote_nua", VN_X25RA, 0},    /* 193 */
#endif /* IBMX25 */
#ifdef CK_SSL
    { "x509_issuer",  VN_X509_I, 0},
    { "x509_subject", VN_X509_S, 0},
#endif /* CK_SSL */
#ifndef NOXFER
    { "xferstatus",VN_XFSTAT,0},        /* 193 */
    { "xfermsg",   VN_XFMSG, 0},        /* 193 */
    { "xfer_badpackets", VN_XF_BC, 0},  /* 195 */
    { "xfer_timeouts",   VN_XF_TM, 0},  /* 195 */
    { "xfer_retransmits",VN_XF_RX, 0},  /* 195 */
#endif /* NOXFER */
    { "xprogram",  VN_XPROG, 0},        /* 193 */
    { "xversion",  VN_XVNUM, 0}         /* 192 */
};
int nvars = (sizeof(vartab) / sizeof(struct keytab));
#endif /* NOSPL */

#ifndef NOSPL
struct keytab fnctab[] = {              /* Function names */
#ifdef OS2
    { ".oox",       FN_OOX, CM_INV},    /* ... */
#endif /* OS2 */

#ifdef CKCHANNELIO
    { "_eof",       FN_FEOF,   0},
    { "_errmsg",    FN_FERMSG, 0},
    { "_getblock",  FN_FGBLK,  0},
    { "_getchar",   FN_FGCHAR, 0},
    { "_getline",   FN_FGLINE, 0},
    { "_handle",    FN_FILNO,  0},
    { "_line",      FN_NLINE,  0},
    { "_pos",       FN_FPOS,   0},
    { "_putblock",  FN_FPBLK,  0},
    { "_putchar",   FN_FPCHAR, 0},
    { "_putline",   FN_FPLINE, 0},
    { "_status",    FN_FSTAT,  0},
#endif /* CKCHANNELIO */

    { "aaconvert",  FN_AADUMP, 0},      /* Associative Array conversion */
    { "absolute",   FN_ABS,  0},        /* Absolute value */
#ifdef TCPSOCKET
    { "addr2name",  FN_HSTADD,CM_INV},  /* IP Address to Hostname */
    { "addrtoname", FN_HSTADD,CM_INV},  /* IP Address to Hostname */
#endif /* TCPSOCKET */
    { "arraylook",  FN_ALOOK,0},        /* Array lookup */
    { "b64decode",  FN_FMB64,0},        /* Base-64 conversion */
    { "b64encode",  FN_TOB64,0},        /* ... */
    { "basename",   FN_BSN,  0},        /* Basename */
    { "break",      FN_BRK,  0},        /* Break (as in Snobol) */
    { "ca",         FN_CAP,  CM_INV|CM_ABR}, /* Abbreviation for capitablize */
    { "cap",        FN_CAP,  CM_INV|CM_ABR}, /* Abbreviation for capitablize */
    { "capitalize", FN_CAP,  0},        /* First Letter -> uppercase */
    { "caps",       FN_CAP,  CM_INV},   /* ditto */
    { "character",  FN_CHR,  0},        /* Character from code */
    { "checksum",   FN_CHK,  0},        /* Checksum */
    { "cmdstack",   FN_CMDSTK,0},       /* Command stack */
    { "cmpdates",   FN_CMPDATE,0},      /* Compare dates */
    { "code",       FN_COD,  0},        /* Code from character */
#ifndef NOPUSH
    { "command",    FN_CMD,  0},        /* Output from a command */
#endif /* NOPUSH */
    { "contents",   FN_CON,  0},        /* Definition (contents) of variable */
    { "count",      FN_COUNT, 0},       /* Occurrences of string in string */
    { "crc16",      FN_CRC,  0},        /* CRC-16 */
#ifdef OS2
    { "crypt",      FN_CRY, CM_INV},
#endif /* OS2 */
    { "cvtcset",    FN_XLATE, 0},	/* Convert character set */
    { "cvtdate",    FN_DTIM, 0},        /* Convert free date/time to std */
#ifdef ZFCDAT
    { "date",       FN_FD,   0},        /* File modification/creation date */
#endif /* ZFCDAT */
    { "day",        FN_DAY,  0},        /* Day of week */
    { "dayofyear",  FN_JDATE,0},        /* Date to Day of Year */
    { "decodehex",  FN_UNPCT, 0},	/* Decode string with hex escapes */
    { "definition", FN_DEF,  0},        /* Return definition of given macro */
    { "delta2secs", FN_DELSEC, 0},      /* Delta time to seconds */
    { "deltatosecs", FN_DELSEC, CM_INV}, /* Delta time to seconds */
#ifndef NODIAL
    { "dialconvert",FN_PNCVT,0},        /* Convert portable phone number */
#endif /* NODIAL */
    { "diffdates",  FN_DIFDATE,0},      /* Difference of two date-times */
    { "dimension",  FN_DIM,  0},        /* Dimension of array */
    { "dir",        FN_DIR,  CM_INV|CM_ABR}, /* Abbreviation for direct.. */
    { "dire",       FN_DIR,  CM_INV|CM_ABR}, /* Abbreviation for direct.. */
    { "direc",      FN_DIR,  CM_INV|CM_ABR}, /* Abbreviation for direct.. */
    { "direct",     FN_DIR,  CM_INV|CM_ABR}, /* Abbreviation for direct.. */
    { "directo",    FN_DIR,  CM_INV|CM_ABR}, /* Abbreviation for direct.. */
    { "director",   FN_DIR,  CM_INV|CM_ABR}, /* Abbreviation for direct.. */
    { "directories",FN_DIR,  0},        /* List of directories */
    { "directory",  FN_DIR,  CM_INV},	/* List of directories */
    { "dirname",    FN_DNAM, 0},        /* Directory part of filename */
    { "dos2unixpath",FN_PC_DU, },       /* DOS to UNIX path */
    { "dostounixpath",FN_PC_DU, CM_INV}, /* DOS to UNIX path */
    { "doy",        FN_JDATE,CM_INV},   /* Date to Day of Year */
    { "doy2date",   FN_DATEJ,0},        /* Day of Year to date */
    { "doytodate",  FN_DATEJ,CM_INV},   /* Day of Year to date */
    { "emailaddress",FN_EMAIL, 0},	/* Email address */
#ifdef FN_ERRMSG
    { "errstring",  FN_ERRMSG,0},       /* Error code to message */
#endif /* FN_ERRMSG */
    { "evaluate",   FN_EVA,  0},        /* Evaluate given arith expression */
    { "execute",    FN_EXE,  0},        /* Execute given macro */
    { "files",      FN_FC,   0},        /* File count */
#ifdef FNFLOAT
    { "fpabsolute", FN_FPABS, 0},       /* Floating-point absolute value */
    { "fpadd",      FN_FPADD, 0},       /* FP add */
    { "fpcosine",   FN_FPCOS, 0},       /* FP cosine */
    { "fpdivide",   FN_FPDIV, 0},       /* FP divide */
    { "fpexp",      FN_FPEXP, 0},       /* FP e to the x */
    { "fpint",      FN_FPINT, 0},       /* FP to integer */
    { "fplog10",    FN_FPLOG, 0},       /* FP base-10 logarithm */
    { "fplogn",     FN_FPLN,  0},       /* FP natural logarithm */
    { "fpmaximum",  FN_FPMAX, 0},       /* FP maxinum */
    { "fpminimum",  FN_FPMIN, 0},       /* FP mininum */
    { "fpmodulus",  FN_FPMOD, 0},       /* FP modulus */
    { "fpmultiply", FN_FPMUL, 0},       /* FP multiply */
    { "fpraise",    FN_FPPOW, 0},       /* FP raise to a power */
    { "fpround",    FN_FPROU, 0},       /* FP round */
    { "fpsine",     FN_FPSIN, 0},       /* FP sine */
    { "fpsqrt",     FN_FPSQR, 0},       /* FP square root */
    { "fpsubtract", FN_FPSUB, 0},       /* FP subtract */
    { "fptangent",  FN_FPTAN, 0},       /* FP tangent */
#endif /* FNFLOAT */
    { "function",   FN_FUNC, 0 },       /* Test for existence of a function */
    { "getpidinfo", FN_PID, 0  },       /* Get PID info */
    { "hex2ip",     FN_HEX2IP,0},       /* Hex to IP address */
    { "hextoip",    FN_HEX2IP,CM_INV},  /* Hex to IP address */
    { "hex2n",      FN_HEX2N, CM_INV},  /* Hex to decimal number */
    { "hexify",     FN_HEX,   0},       /* Hexify (string) */
    { "index",      FN_IND,   0},       /* Index (string search) */
    { "ip2hex",     FN_IP2HEX,0},       /* IP address to hex */
    { "iptohex",    FN_IP2HEX,CM_INV},  /* IP address to hex */
    { "ipaddress",  FN_IPA,   0},       /* Find and return IP address */
    { "jdate",      FN_JDATE, CM_INV},  /* Date to Day of Year */
    { "join",       FN_JOIN,  0},       /* Join array elements */
    { "keywordvalue",  FN_KWVAL, 0},    /* Keyword=Value */
#ifdef CK_KERBEROS
    { "krbflags",      FN_KRB_FG, 0},   /* Kerberos functions */
    { "krbisvalid",    FN_KRB_IV, 0},
    { "krbnextticket", FN_KRB_NX, 0},
    { "krbtickets",    FN_KRB_TK, 0},
    { "krbtimeleft",   FN_KRB_TT, 0},
#endif /* CK_KERBEROS */
    { "kwvalue",    FN_KWVAL, CM_INV},	/* Keyword=Value */
    { "left",       FN_LEF,  0},        /* Leftmost n characters of string */
    { "length",     FN_LEN,  0},        /* Return length of argument */
    { "literal",    FN_LIT,  0},        /* Return argument literally */
#ifdef NT
    { "longpathname",FN_LNAME,0},	/* GetLongPathName() */
#else
    { "longpathname",FN_FFN,CM_INV},
#endif /* NT */
    { "lop",        FN_STL,  0},        /* Lop */
    { "lopx",       FN_LOPX, 0},        /* Lopx */
    { "lower",      FN_LOW,  0},        /* Return lowercased argument */
    { "lpad",       FN_LPA,  0},        /* Return left-padded argument */
    { "ltrim",      FN_LTR,  0},        /* Left-Trim */
    { "maximum",    FN_MAX,  0},        /* Return maximum of two arguments */
    { "minimum",    FN_MIN,  0},        /* Return minimum of two arguments */
    { "mjd",        FN_MJD,  0},        /* Date to Modified Julian Date */
    { "mjd2date",   FN_MJD2, 0},        /* MJD to Date */
    { "mjdtodate",  FN_MJD2, CM_INV},   /* MJD to Date */
    { "modulus",    FN_MOD,  0},        /* Return modulus of two arguments */
#ifdef COMMENT
    { "msleep",     FN_MSLEEP,0},       /* Sleep for n milliseconds */
#endif /* COMMENT */
    { "n2hex",      FN_2HEX, CM_INV},   /* Number to hex */
    { "n2octal",    FN_2OCT, CM_INV},   /* Number to octal */
    { "n2time",     FN_N2TIM,0},        /* Number to hh:mm:ss */
#ifdef TCPSOCKET
    { "name2addr",  FN_HSTNAM,CM_INV},  /* Hostname to IP Address */
#endif /* TCPSOCKET */
    { "nday",       FN_NDAY, 0},        /* Numeric day of week */
    { "nextfile",   FN_FIL,  0},        /* Next file in list */
    { "ntime",      FN_NTIM, 0},        /* Time to seconds since midnight */
    { "ntohex",     FN_2HEX, CM_INV},   /* Number to hex */
    { "ntooctal",   FN_2OCT, CM_INV},   /* Number to octal */
    { "ntotime",    FN_N2TIM,CM_INV},   /* Number to hh:mm:ss */
    { "oct2n",      FN_OCT2N,CM_INV},   /* Octal to decimal number */
    { "octton",     FN_OCT2N,CM_INV},   /* Octal to decimal number */
    { "pathname",   FN_FFN,  0},        /* Full file name */
    { "pattern",    FN_PATTERN, 0},     /* Pattern (for INPUT) */
#ifdef CK_PERMS
    { "permissions",FN_PERM, 0},        /* Permissions of file */
#else
    { "permissions",FN_PERM, CM_INV},   /* Permissions of file */
#endif /* CK_PERMS */
#ifdef SEEK_CUR
    { "pictureinfo",FN_PICTURE, 0 },	/* Picture orientation/dimensions */
#endif	/* SEEK_CUR */
    { "radix",      FN_RADIX, 0 },	/* Radix conversion */
#ifndef NORANDOM
    { "random",     FN_RAND, 0},        /* Random number */
#endif /* NORANDOM */
#ifndef NOPUSH
    { "rawcommand", FN_RAW,  0},        /* Output from a command (raw) */
#endif /* NOPUSH */
#ifdef RECURSIVE
    { "rdirectories", FN_RDIR, 0},      /* Recursive directory list */
#endif /* RECURSIVE */
    { "recurse",    FN_RECURSE, 0},	/* Recursive variable evaluation */
#ifdef RECURSIVE
    { "rfiles",       FN_RFIL, 0},      /* Recursive file list */
#endif /* RECURSIVE */
    { "rep",        FN_REP, CM_INV|CM_ABR},
    { "repeat",     FN_REP,  0},        /* Repeat argument given # of times */
    { "replace",    FN_RPL,  0},        /* Replace characters in string */
    { "reverse",    FN_REV,  0},        /* Reverse the argument string */
    { "right",      FN_RIG,  0},        /* Rightmost n characters of string */
    { "rindex",     FN_RIX,  0},        /* Right index */
    { "rpad",       FN_RPA,  0},        /* Right-pad the argument */
    { "rsearch",    FN_RSEARCH, 0},     /* R-L Search for pattern in string */
#ifdef OS2
    { "scrncurx",   FN_SCRN_CX,  0},    /* Screen Cursor X Pos */
    { "scrncury",   FN_SCRN_CY,  0},    /* Screen Cursor Y Pos */
    { "scrnstr",    FN_SCRN_STR, 0},    /* Screen String */
#endif /* OS2 */
    { "search",     FN_SEARCH, 0},      /* L-R Search for pattern in string */
#ifndef NOSEXP
    { "sexpression",FN_SEXP, 0},        /* S-Expression */
#endif /* NOSEXP */
#ifdef NT
    { "shortpathname",FN_SNAME,0},	/* GetShortPathName() */
#else
    { "shortpathname",FN_FFN,CM_INV},
#endif /* NT */
    { "size",       FN_FS,   0},        /* File size */
#ifdef COMMENT
    { "sleep",      FN_SLEEP,0},        /* Sleep for n seconds */
#endif /* COMMENT */
    { "span",       FN_SPN,  0},        /* Span - like Snobol */
    { "split",      FN_SPLIT,0},        /* Split string into words */
    { "squeeze",    FN_SQUEEZE,0},	/* Squeeze whitespace in string */
    { "strcmp",     FN_STRCMP,0},	/* String comparison */
    { "stringtype", FN_STRINGT,0},	/* String type (7-bit, 8-bit, UTF-8) */
    { "stripb",     FN_STB,  0},        /* Strip enclosing braces/brackets */
    { "stripn",     FN_STN,  0},        /* Strip n chars */
    { "stripx",     FN_STX,  0},        /* Strip suffix */
    { "su",         FN_SUB,  CM_INV|CM_ABR},
    { "sub",        FN_SUB,  CM_INV|CM_ABR},
    { "subs",       FN_SUB,  CM_INV|CM_ABR},
    { "subst",      FN_SUB,  CM_INV|CM_ABR},
    { "substitute", FN_SUBST,0},        /* Substitute chars */
    { "substring",  FN_SUB,  0},        /* Extract substring from argument */
    { "tablelook",  FN_TLOOK,0},        /* Table lookup */
    { "time",       FN_TIME, 0},        /* Free-format time to hh:mm:ss */
    { "tod2secs",   FN_NTIM, CM_INV},   /* Time-of-day-to-secs-since-midnite */
    { "todtosecs",  FN_NTIM, CM_INV},   /* Time-of-day-to-secs-since-midnite */
    { "trim",       FN_TRM,  0},        /* Trim */
    { "unhexify",   FN_UNH,  0},        /* Unhexify */
    { "unix2dospath",FN_PC_UD, 0},      /* UNIX to DOS path */
    { "unixtodospath",FN_PC_UD, CM_INV}, /* UNIX to DOS path */
    { "untabify",   FN_UNTAB,0},        /* Untabify */
    { "upper",      FN_UPP,  0},        /* Return uppercased argument */
    { "utcdate",    FN_TOGMT,0},        /* Date-time to UTC (GMT) */
    { "verify",     FN_VER,  0},        /* Verify */
    { "word",       FN_WORD, 0},        /* Extract a word */
    { "", 0, 0}
};
int nfuncs = (sizeof(fnctab) / sizeof(struct keytab)) - 1;
#endif /* NOSPL */

#ifndef NOSPL                           /* Buffer for expansion of */
#ifdef BIGBUFOK                         /* built-in variables. */
#define VVBUFL 1024
#else
#define VVBUFL 256
#endif /* BIGBUFOK */
char vvbuf[VVBUFL+1];
#endif /* NOSPL */

struct keytab disptb[] = {              /* Log file disposition */
    { "append",    1,  0},
    { "new",       0,  0}
};

#ifdef CKFLOAT

/* I N I T F L O A T  --  Deduce floating-point precision by inspection */

int fp_rounding = 0;                /* Nonzero if printf("%f") rounds */
int fp_digits = 0;                  /* Digits of floating point precision */

#ifdef COMMENT
/* For looking at internal floating-point representations */
static char fp_xbuf[128];
static char *
tohex(s, n) CHAR * s; int n; {
    int x;
    char * p = fp_xbuf;
    while (n-- > 0) {
        x = (*s >> 4) & 0x0f;
        *p++ = hexdigits[x];
        x = *s++ & 0x0f;
        *p++ = hexdigits[x];
    }
    *p = NUL;
    return((char *)fp_xbuf);
}
#endif /* COMMENT */

char math_pi[] = "3.1415926535897932384626433832795";
char math_e[] =  "2.7182818284590452353602874713527";

VOID
initfloat() {
    char * buf = NULL;
    int i, x, y;
/*
  We malloc a big temporary buffer for sprintf() to minimize likelihood of
  (and damage from) sprintf buffer overflows.  In any case, the only way this
  could happen would be if sprintf() itself had bugs, since the format
  descriptor says to cut it off at 250 decimal places.
*/
    if ((buf = (char *)malloc(4096))) {
        sprintf(buf,"%0.250f",(10.0 / 3.0));
        for (i = 2; i < 250 && buf[i] == '3'; i++) ;
        x = i - 1;
        debug(F111,"initfloat 10.0/3.0",buf,x);
        sprintf(buf,"%0.250f",(4.0 / 9.0));
        for (i = 2; i < 250 && buf[i] == '4'; i++) ;
        y = i - 1;
        debug(F111,"initfloat 4.0/9.0",buf,y);
        fp_digits = (x < y) ? x : y;
        if (fp_digits < sizeof(math_pi) - 1) {
            math_pi[fp_digits+1] = NUL;
            math_e[fp_digits+1] = NUL;
        }
        sprintf(buf,"%0.6f",(7.0 / 9.0));
        if (buf[7] == '8') fp_rounding = 1;
        debug(F111,"initfloat 7.0/9.0",buf,fp_rounding);
        debug(F101,"initfloat precision","",fp_digits);
        free(buf);
    }
}
#endif /* CKFLOAT */

/*
  P R E S C A N -- A quick look through the command-line options for
  items that must be handled before the initialization file is executed.
*/
#ifdef NT
extern int StartedFromDialer;
#endif /* NT */
#ifdef OS2
extern int k95stdio;
unsigned long startflags = 0L;
#endif /* OS2 */

static char *
findinpath(arg) char * arg; {
#ifdef OS2
    char * scriptenv, * keymapenv;
    int len;
#endif /* OS2 */
#ifdef DCMDBUF
    extern char * cmdbuf;
#else
    extern char cmdbuf[];
#endif /* DCMDBUF */
    char takepath[4096];
    char * s;
    int x, z;

    /* Set up search path... */
#ifdef OS2
    char * appdata0 = NULL, *appdata1 = NULL;
#ifdef NT
    scriptenv = getenv("K95SCRIPTS");
    keymapenv = getenv("K95KEYMAPS");
    makestr(&appdata0,(char *)GetAppData(0));
    makestr(&appdata1,(char *)GetAppData(1));
#else /* NT */
    scriptenv = getenv("K2SCRIPTS");
    keymapenv = getenv("K2KEYMAPS");
#endif /* NT */
    if (!scriptenv)
      scriptenv = getenv("CK_SCRIPTS");
    if (!scriptenv)
      scriptenv = "";
    if (!keymapenv)
      keymapenv = getenv("CK_KEYMAPS");
    if (!keymapenv)
      keymapenv = "";

    debug(F110,"startupdir",startupdir,0);
    debug(F110,"common appdata directory",appdata1,0);
    debug(F110,"appdata directory",appdata0,0);
    debug(F110,"inidir",inidir,0);
    debug(F110,"home",zhome(),0);
    debug(F110,"exedir",exedir,0);

    len = strlen(scriptenv) + strlen(keymapenv) + 3*strlen(startupdir)
        + 3*strlen(inidir) + 3*strlen(zhome()) + 3*strlen(exedir)
        + (appdata0 ? 3*strlen(appdata0) : 0) 
        + (appdata1 ? 3*strlen(appdata1) : 0)
        + 6*strlen("SCRIPTS/") + 6*strlen("KEYMAPS/") + 16;

    if (len >= 4096) {                  /* SAFE (length is checked) */
        takepath[0] = '\0';
        debug(F111,"findinpath error - path length too long","len",len);
    } else
      sprintf(takepath,
              /* semicolon-separated path list */
    "%s%s%s%s%s;%s%s;%s%s;%s%s%s%s%s%s%s%s%s%s%s%s%s;%s%s;%s%s;%s;%s%s;%s%s",
              scriptenv,
              (scriptenv[0] && scriptenv[strlen(scriptenv)-1]==';')?"":";",
              keymapenv,
              (keymapenv[0] && keymapenv[strlen(keymapenv)-1]==';')?"":";",
              startupdir,
              startupdir, "SCRIPTS/",
              startupdir, "KEYMAPS/",
              appdata1 ? appdata1 : "", 
              appdata1 ? "Kermit 95;" : "",
              appdata1 ? appdata1 : "",
              appdata1 ? "Kermit 95/SCRIPTS/;" : "",
              appdata1 ? appdata1 : "",
              appdata1 ? "Kermit 95/KEYMAPS/;" : "",
              appdata0 ? appdata0 : "",
              appdata0 ? "Kermit 95;" : "",
              appdata0 ? appdata0 : "",
              appdata0 ? "Kermit 95/SCRIPTS/;" : "",
              appdata0 ? appdata0 : "",
              appdata0 ? "Kermit 95/KEYMAPS/;" : "",
              inidir,
              inidir, "SCRIPTS/",
              inidir, "KEYMAPS/",
              zhome(),
              zhome(), "SCRIPTS/",
              zhome(), "KEYMAPS/",
              exedir,
              exedir, "SCRIPTS/",
              exedir, "KEYMAPS/"
              );
    debug(F110,"findinpath takepath",takepath,0);
#ifdef NT
    makestr(&appdata0,NULL);
    makestr(&appdata1,NULL);
#endif /* NT */
#else /* not OS2 */
#ifndef NOSPL
    z = 1024;                           /* Look in home directory */
    s = takepath;
    zzstring("\\v(home)",&s,&z);
#else
    takepath[0] = '\0';
#endif /* NOSPL */
#endif /* OS2 */
/*
  All the logic for searching the take path is in the command parser.
  So even though we aren't parsing commands, we initialize and call the
  parser from here, with the purported filename stuffed into the command
  buffer, followed by some carriage returns to make the parser return.
  If the file is not found, or otherwise not accessible, the parser prints
  an appropriate message, and then we just exit.
*/
    cmdini();                           /* Allocate command buffers etc */
    cmini(0);                           /* Initialize them */
    /* Stuff filename into command buf with braces in case of spaces */
    ckmakmsg(cmdbuf,CMDBL,"{",arg,"}",NULL);
    debug(F110,"findinpath cmdbuf",cmdbuf,0);
    ckstrncat(cmdbuf,"\r\r",CMDBL);     /* And some carriage returns */
    if (cmifip("","",&s,&x,0,takepath,xxstring) < 0)
      return(NULL);
    cmres();
    return(s);
}

static int tr_int;                      /* Flag if TRANSMIT interrupted */

#ifndef MAC
SIGTYP
#ifdef CK_ANSIC
trtrap(int foo)                         /* TRANSMIT interrupt trap */
#else
trtrap(foo) int foo;                    /* TRANSMIT interrupt trap */
#endif /* CK_ANSIC */
/* trtrap */ {
#ifdef __EMX__
    signal(SIGINT, SIG_ACK);
#endif
    tr_int = 1;                         /* (Need arg for ANSI C) */
    SIGRETURN;
}
#endif /* MAC */
#endif /* NOICP */

#ifdef UNIX
VOID
getexedir() {
    extern char * xarg0;
    long xx;
  /*
    Unix provides no standard service for this.  We look in argv[0], and if
    we're lucky there's a full pathname.  If not we do a PATH search.
  */
    if (ckstrchr(xarg0,'/')) {          /* Global copy of argv[0] */
        int i, k;
        char * p = NULL;
        if ((k = ckstrncpy(tmpbuf,xarg0,TMPBUFSIZ-2)) > 0) {
            p = tmpbuf;
            /* Convert to fully qualified pathname */
            if (tmpbuf[0]) if (tmpbuf[0] != '/') {
                line[0] = NUL;
                zfnqfp(tmpbuf,LINBUFSIZ-2,(char *)line);
                if (line[0])
                  p = line;
            }
            xx = zchki(p);
            if (xx > -1) {		/* Is the result an existing file? */
                k = strlen(p);
                for (i = k-1; i > 0; i--) { /* Yes, strip name part */
                    if (p[i] == '/') {
                        if (i < k-1)
                          p[i+1] = NUL;
                        break;
                    }
                }
            }
	    makestr(&exedir,p);		/* Save the result */
        }
    }
    if (!exedir && xarg0) {             /* Not found? */
        char * p;
        p = getenv("PATH");             /* Search the PATH */
        if (p) {                        /* If there is one... */
            char * q, * PATH = NULL;
            int k;
            makestr(&PATH,p);           /* Pokeable copy of PATH string */
            if (PATH) {                 /* If malloc succeeded... */
                p = PATH;
                while (p && *p) {        /* Loop through segments */
                    q = ckstrchr(p,':'); /* End of this segment */
                    if (q == p) {       /* Null PATH segment */
                        p++;            /* Skip over colon */
                        continue;
                    }
                    if (q)              /* If not at end of PATH string */
                      *q++ = NUL;       /* zero out the colon */
                    if ((k = ckstrncpy(tmpbuf,p,TMPBUFSIZ)) > 0) {
                        if (tmpbuf[k-1] != '/') { /* Copy this PATH segment */
                            tmpbuf[k++] = '/';    /* Append '/' if needed */
                            tmpbuf[k] = NUL;
                        }
                        /* Append the argv[0] value */
                        if (ckstrncpy(&tmpbuf[k],xarg0,TMPBUFSIZ) > 0) {
                            if (zchki(tmpbuf) > -1) { /* File exists? */
                                tmpbuf[k] = NUL;      /* Yes, we're done */
                                zfnqfp(tmpbuf,LINBUFSIZ,(char *)line);
                                makestr(&exedir,line);
                                break;
                            }
                        } else break;
                    } else break;
                    p = q;              /* Not found, go to next segment  */
                } /* while */
                free(PATH);             /* Free PATH copy */
            }
        }
        if (!exedir) {                  /* Still nothing? */
            if (zchki(xarg0) > -1) {    /* Maybe it's in the current dir */
                zfnqfp(zgtdir(),LINBUFSIZ,(char *)line);
                makestr(&exedir,line);
            }
        }
    }
    if (!exedir) {                      /* Still nothing? */
        makestr(&exedir,"/");           /* Fake it with with root. */
    }
}
#endif /* UNIX */

int arg_x = 0;
static int x_prescan = 0;

/*
  The argument y once meant something but I can't imagine what so now
  it's ignored.  (Prior to 22 Aug 98, prescan() was called twice by main(),
  and the arg differentiated the two calls.  But this caused all sorts of
  problems & confusion, so I commented out the second call.  This issue might
  need to be revisited.)
*/
VOID
prescan(dummy) int dummy; {             /* Arg is ignored. */
    extern int howcalled;
    int yargc; char **yargv;
    char x;
    char *yp, *yy;
#ifdef DEBUG
    int debcount = 0;
#endif /* DEBUG */
    int z;

    if (x_prescan)                      /* Only run once */
      return;
    x_prescan = 1;

    yargc = xargc;                      /* Make copy of arg vector */
    yargv = xargv;

#ifndef NOICP
#ifdef DCMDBUF
    if (!kermrc)
      if (!(kermrc = (char *) malloc(KERMRCL+1)))
        fatal("prescan: no memory for kermrc");
#endif /* DCMDBUF */
    ckstrncpy(kermrc,KERMRC,KERMRCL);   /* Default init file name */
#endif /* NOICP */

#ifdef OS2
    yp = getenv("K95STARTFLAGS");
    if (yp) {
        startflags = atoi(yp);
    }
#endif /* OS2 */

#ifdef IKSD
    if (howcalled == I_AM_IKSD)         /* Internet Kermit Service daemon */
      inserver = 1;                     /* (See inserver section of ckcmai) */
#endif /* IKSD */

/* Command line options for Kermit */

#ifndef NOCMDL
    if (yargc > 1
        && *yargv[1] != '-'
        && (yargv[1][0] != '=')
#ifdef KERBANG
        && (yargv[1][0] != '+')
#endif /* KERBANG */
#ifdef IKSD
        && (howcalled != I_AM_IKSD)
#endif /* IKSD */
        ) {                             /* Filename as 1st argument */
#ifndef NOICP
        char *s;
#endif /* NOICP */
#ifndef NOURL
        extern int haveurl;
        extern struct urldata g_url;
        if (urlparse(yargv[1],&g_url)) {
            if (!ckstrcmp(g_url.svc,"ftp",-1,0) ||
                !ckstrcmp(g_url.svc,"ftps",-1,0)) {
                haveurl = 1;
                howcalled = I_AM_FTP;
            } else if (!ckstrcmp(g_url.svc,"telnet",-1,0) ||
                       !ckstrcmp(g_url.svc,"telnets",-1,0)) {
                haveurl = 1;
                howcalled = I_AM_TELNET;
            } else if (!ckstrcmp(g_url.svc,"ssh",-1,0)) {
                haveurl = 1;
                howcalled = I_AM_SSH;
            } else if (!ckstrcmp(g_url.svc,"iksd",-1,0) ||
                       !ckstrcmp(g_url.svc,"kermit",-1,0)) {
                haveurl = 1;
                howcalled = I_AM_KERMIT;
            } else if (!ckstrcmp(g_url.svc,"http",-1,0) ||
                       !ckstrcmp(g_url.svc,"https",-1,0)) {
                haveurl = 1;
                howcalled = I_AM_HTTP;
            }
            if (haveurl) {
                while (--yargc > 0) {   /* Go through command-line args */
                    yargv++;            /* looking for -Y and -d */
                    yp = *yargv+1;
                    if (**yargv == '-') {
                        x = *(*yargv+1);
                        while (x) {
                            switch (x) {
#ifndef NOICP
			      case '+':
			      case '-':
                                if (doxarg(yargv,1) < 0) {
                                    fatal("Extended argument error");
                                }
                                yp = "";
                                break;
#endif /* NOICP */
                              case 'Y':
                                noinit++;
                                break;

                              case 'q':
                                quiet = 1;
                                break;

                              case 'h':
                                  noinit = 1;
#ifdef OS2
                                  startflags |= 2;    /* No network DLLs */
                                  startflags |= 4;    /* No TAPI DLLs */
                                  startflags |= 8;    /* No Security DLLs */
                                  startflags |= 16;   /* No Zmodem DLLs */
                                  startflags |= 32;   /* Stdin */
                                  startflags |= 64;   /* Stdout */
#endif /* OS2 */
                                  break;
                              case 'd': /* = SET DEBUG ON */
#ifdef DEBUG
                                if (debcount++ > 0)
                                  debtim = 1;
                                if (!deblog)
                                  deblog = debopn("debug.log",0);
#endif /* DEBUG */
                                break;
#ifdef OS2
                              case 'W':
                                if (*(yp+1))
                                  fatal("invalid argument bundling after -W");
                                yargv++, yargc--;
                                if (yargc < 1)
                                  fatal("Window handle missing");
                                hwndDialer = (HWND) atol(*yargv);
                                StartedFromDialer = 1;
                                yargv++, yargc--;
                                KermitDialerID = atol(*yargv) ;
                                break;
                              case '#': /* K95 initialization options */
                                if (*(yp+1)) {
                                    fatal("invalid argument bundling");
                                }
                                yargv++, yargc--;
                                if (yargc < 1)
                                  fatal("-# argument missing");
                                startflags |= atol(*yargv);
                                break;
#endif /* OS2 */
                            }
                            if (!yp)
                              break;
                            x = *++yp;
                        }
                    }
                }
                return;
            }
        }
        /* after this point non-Kermit personalities must return */
        switch (howcalled) {
	  case I_AM_KERMIT:
	  case I_AM_IKSD:
	  case I_AM_SSHSUB:
            break;
	  default:
            return;
        }
#endif /* NOURL */

#ifndef NOICP
        /* If it is not a URL that we recognize, try to treat it as a file */

        if (!isabsolute(yargv[1]))      /* If not absolute */
          s = findinpath(yargv[1]);     /* Look in PATH */
        else
          s = yargv[1];
        if (!s)
          doexit(BAD_EXIT,xitsta);
        zfnqfp(s,CKMAXPATH,cmdfil);     /* In case of CD in file */
        yargc -= 1;                     /* Skip past the filename */
        yargv += 1;                     /* Otherwise we'll get an error */
#endif /* NOICP */
    }

#ifndef NOCMDL
#ifdef NEWFTP
    if (howcalled == I_AM_FTP) {        /* Kermit's FTP client personality */
        while (--yargc > 0) {           /* Go through command-line args */
            yargv++;                    /* looking for -Y and -d */
            yp = *yargv+1;
            if (**yargv == '-') {
                x = *(*yargv+1);
                while (x) {
                    switch (x) {
#ifndef NOICP
		      case '+':
		      case '-':
                        if (doxarg(yargv,1) < 0) {
                            fatal("Extended argument error");
                        }
                        yp = "";
                        break;
#endif /* NOICP */
                      case 'Y':
                        noinit++;
                        break;

                      case 'q':
                          quiet = 1;
                          break;

                      case 'h':
                        noinit = 1;
#ifdef OS2
                        startflags |= 2;    /* No network DLLs */
                        startflags |= 4;    /* No TAPI DLLs */
                        startflags |= 8;    /* No Security DLLs */
                        startflags |= 16;   /* No Zmodem DLLs */
                        startflags |= 32;   /* Stdin */
                        startflags |= 64;   /* Stdout */
#endif /* OS2 */
                        break;
                      case 'd':             /* = SET DEBUG ON */
#ifdef DEBUG
                        if (debcount++ > 0)
                          debtim = 1;
                        if (!deblog)
                          deblog = debopn("debug.log",0);
#endif /* DEBUG */
                        break;
#ifdef OS2
                      case 'W':
                        if (*(yp+1))
                          fatal("invalid argument bundling after -W");
                        yargv++, yargc--;
                        if (yargc < 1)
                          fatal("Window handle missing");
                        hwndDialer = (HWND) atol(*yargv);
                        StartedFromDialer = 1;
                        yargv++, yargc--;
                        KermitDialerID = atol(*yargv) ;
                        break;
                      case '#':         /* K95 initialization options */
                        if (*(yp+1)) {
                            fatal("invalid argument bundling");
                        }
                        yargv++, yargc--;
                        if (yargc < 1)
                          fatal("-# argument missing");
                        startflags |= atol(*yargv);
                        break;
#endif /* OS2 */
                    }
                    if (!yp)
                      break;
                    x = *++yp;
                }
            }
        }
        return;
    }
#endif /* NEWFTP */
#endif /* NOCMDL */

    while (--yargc > 0) {               /* Go through command-line args */
        yargv++;
        yp = *yargv+1;                  /* Pointer for bundled args */
        if (**yargv == '=')             /* Same rules as cmdlin()... */
          return;
        debug(F110,"prescan *yargv",*yargv,0);

#ifndef NOICP
#ifdef KERBANG
        yy = *yargv;
        if (!strcmp(yy,"+") || (*yy == '+' && *(yy+1) < (char)33)) {
            char * s;
            yargv++;
            noinit = 1;
            if (!*yargv)
              return;
            cfilef = 1;
            s = findinpath(*yargv);
            if (s) {
                zfnqfp(s,CKMAXPATH,cmdfil);
                return;
            } else
              doexit(BAD_EXIT,xitsta);
        }
#endif /* KERBANG */
#endif /* NOICP */
        if (!strcmp(*yargv,"--"))       /* getopt() conformance */
          return;
#ifdef VMS
        else if (**yargv == '/')
          continue;
#endif /* VMS */
        else if (**yargv == '-') {      /* Got an option (begins with dash) */
            x = *(*yargv+1);            /* Get option letter */
            while (x) {                 /* Allow for bundled options */
                debug(F000,"prescan arg","",x);
                switch (x) {
#ifndef NOICP
                  case '+':
                  case '-':
                    if (doxarg(yargv,1) < 0) {
                        fatal("Extended argument error");
                    }
#ifndef COMMENT				/* Jeff 28 Apr 2003 */
                    yp = NULL;		/* (not "") */
#else
                    yargv++, yargc--;
                    yp = *yargv;
#endif /* COMMENT */
                    break;
#endif /* NOICP */

                  case '7':             /* Undocumented... */
                    sstelnet = 1;       /* (because it doesn't work) */
                    break;
#ifdef IKSD
                  case 'A': {
                      char * p;
                      inserver = 1;     /* Flag that we are doing this */
                      srvcdmsg = 2;     /* Preset this */
                      /* See inserver section of ckcmai.c for more settings */
#ifdef OS2
                      if (*(yp+1)) {
                          fatal("invalid argument bundling after -A");
                      }
#ifdef NT
                      /* Support for Pragma Systems Telnet/Terminal Servers */
                      p = getenv("PRAGMASYS_INETD_SOCK");
                      if (p && atoi(p) != 0) {
                          ttname[0] = '$';
                          ckstrncpy(&ttname[1],p,TTNAMLEN-1);
                          break;
                      }
#endif /* NT */
                      yargv++, yargc--;
                      if (yargc < 1 || **yargv == '-') {
                          fatal("-A argument missing");
                      } else {
                          ttname[0] = '$';
                          ckstrncpy(&ttname[1],*yargv,TTNAMLEN-1);
                      }
#endif /* OS2 */
                      break;
                  }
#endif /* IKSD */

#ifdef OS2
                  case 'W':
                    if (*(yp+1))
                      fatal("invalid argument bundling after -W");
                    yargv++, yargc--;
                    if (yargc < 1)
                      fatal("Window handle missing");
#ifdef COMMENT
                    if (dummy) {
                        yargv++, yargc--;
                        break;
                    } else {
#endif /* COMMENT */
                        hwndDialer = (HWND) atol(*yargv);
                        StartedFromDialer = 1;
                        yargv++, yargc--;
                        KermitDialerID = atol(*yargv) ;
#ifdef COMMENT
                    }
#endif /* COMMENT */
                    break;

                  case '#':             /* K95 initialization options */
                    if (*(yp+1)) {
                        fatal("invalid argument bundling");
                    }
                    yargv++, yargc--;
                    if (yargc < 1)
                      fatal("-# argument missing");
                    startflags |= atol(*yargv);
                    break;
#endif /* OS2 */

#ifndef NOSPL
                  case 'M':                             /* My User Name */
                    if (*(yp+1)) {
                        fatal("invalid argument bundling");
                    }
                    yargv++, yargc--;
                    if ((yargc < 1) || (**yargv == '-')) {
                        fatal("missing username");
                    }
                    if ((int)strlen(*yargv) > UIDBUFLEN) {
                        fatal("username too long");
                    }
#ifdef COMMENT
/*
  This can't work.  uidbuf is overwritten in sysinit() which has yet to be
  called.  This cannot be set in prescan().
*/
#ifdef IKSD
                    if (!inserver)
#endif /* IKSD */
                      ckstrncpy(uidbuf,*yargv,UIDBUFLEN);
#endif /* COMMENT */
                    break;
#endif /* NOSPL */
                  case 'R':             /* Remote-only advisory */
#ifdef CK_IFRO
                    remonly = 1;
#endif /* CK_IFRO */
                    break;
                  case 'S':             /* STAY */
                    stayflg = 1;
                    break;
                  case 'h':
                    noinit = 1;
#ifdef OS2
                    startflags |= 2;    /* No network DLLs */
                    startflags |= 4;    /* No TAPI DLLs */
                    startflags |= 8;    /* No Security DLLs */
                    startflags |= 16;   /* No Zmodem DLLs */
                    startflags |= 32;   /* Stdin */
                    startflags |= 64;   /* Stdout */
#endif /* OS2 */
                    break;
#ifndef NOICP
                  case 'Y':             /* No init file */
                    noinit = 1;
                    break;
#endif /* NOICP */

                  case 'q':
                      quiet = 1;
                      break;

                  case 'd':             /* = SET DEBUG ON */
#ifdef DEBUG
                    if (debcount++ > 0)
                      debtim = 1;
                    if (!deblog)
                      deblog = debopn("debug.log",0);
#endif /* DEBUG */
                    break;

                  case 'x':             /* Server */
                    arg_x = 1;          /* Note in advance */
                    break;
#ifndef NOICP
                  case 'y':             /* Alternative init file */
                    noinit = 0;
                    yargv++, yargc--;
                    if (yargc < 1) fatal("missing name in -y");
                    /* Replace init file name */
                    ckstrncpy(kermrc,*yargv,KERMRCL);
                    rcflag = 1;         /* Flag that this has been done */
                    debug(F111,"prescan kermrc",kermrc,rcflag);
                    break;
#endif /* NOICP */
                  case 'z':             /* = SET BACKGROUND OFF */
                    bgset = 0;
                    backgrd = 0;
#ifdef VMS
                    batch = 0;
#endif /* VMS */
                    break;

                  case 'B':             /* Force background (batch) */
                    bgset = 1;
                    backgrd = 1;
#ifdef VMS
                    batch = 1;
#endif /* VMS */
                    break;

#ifdef CK_NETBIOS
                  case 'N':
                    {
                        int n ;
                        yargv++, yargc--;
#ifdef COMMENT
                        if (y)
                          break;
#endif /* COMMENT */
                        if (strlen(*yargv) != 1 || (*yargv)[0] == 'X') {
                            NetBiosAdapter = -1;
                        } else {
                            n = atoi(*yargv);
                            if (n >= 0 && n <= 9)
                              NetBiosAdapter = n;
                            else
                              NetBiosAdapter = -1;
                        }
                    }
                    break;
#endif /* CK_NETBIOS */
                  default:
                    break;
                }
                if (!yp)
                  break;
                x = *++yp;              /* See if options are bundled */
            }
        }
    }
#endif /* NOCMDL */
}

/*  G E T T C S  --  Get Transfer (Intermediate) Character Set  */

/*
  Given two file character sets, this routine picks out the appropriate
  "transfer" character set to use for translating between them.
  The transfer character set number is returned.

  Translation between two file character sets is done, for example,
  by the CONNECT, TRANSMIT, and TRANSLATE commands.

  Translation between Kanji character sets is not yet supported.
*/
int
gettcs(cs1,cs2) int cs1, cs2; {
#ifdef NOCSETS                          /* No character-set support */
    return(0);                          /* so no translation */
#else
    int tcs = TC_TRANSP;
#ifdef KANJI
/* Kanji not supported yet */
    if (fcsinfo[cs1].alphabet == AL_JAPAN ||
        fcsinfo[cs2].alphabet == AL_JAPAN )
      tcs = TC_TRANSP;
    else
#endif /* KANJI */
#ifdef CYRILLIC
/*
  I can't remember why we don't test both sets here, but I think there
  must have been a reason...
*/
      if (fcsinfo[cs2].alphabet == AL_CYRIL)
        tcs = TC_CYRILL;
      else
#endif /* CYRILLIC */
#ifdef HEBREW
          if (fcsinfo[cs1].alphabet == AL_HEBREW ||
              fcsinfo[cs2].alphabet == AL_HEBREW )
            tcs = TC_HEBREW;
          else
#endif /* HEBREW */
#ifdef GREEK
          if (fcsinfo[cs1].alphabet == AL_GREEK ||
              fcsinfo[cs2].alphabet == AL_GREEK )
            tcs = TC_GREEK;
          else
#endif /* GREEK */

            /* Roman sets ... */

#ifdef LATIN2                           /* East European */
        if (cs1 == FC_2LATIN  || cs2 == FC_2LATIN || /* Latin-2 */
            cs1 == FC_CP852   || cs2 == FC_CP852  || /* CP852 */
            cs1 == FC_CP1250  || cs2 == FC_CP1250 || /* Windows Latin-2 */
            cs1 == FC_MAZOVIA || cs2 == FC_MAZOVIA)  /* Polish Mazovia */
          tcs = TC_2LATIN;
        else
#endif /* LATIN2 */
                                        /* West European Euro-aware */
          if (cs1 == FC_CP858 || cs1 == FC_9LATIN ||
              cs2 == FC_CP858 || cs2 == FC_9LATIN)
            tcs = TC_9LATIN;
          else                          /* Traditional West European */
            tcs = TC_1LATIN;
    return(tcs);
#endif /* NOCSETS */
}

#ifndef NOLOCAL
/*  D O C O N E C T  --  Do the connect command  */
/*
  q = 0 means issue normal informational message about how to get back, etc.
  q != 0 means to skip the message.
*/

int
doconect(q,async) int q, async; {
    int x;                              /* Return code */
#ifdef CK_AUTODL
    extern CHAR ksbuf[];
#endif /* CK_AUTODL */
#ifndef NOKVERBS                        /* Keyboard macro material */
    extern int keymac, keymacx;
#endif /* NOKVERBS */
    extern int justone, adl_err;
    int qsave;                          /* For remembering "quiet" value */
#ifdef OS2
    extern int term_io;
    extern int display_demo;
    int term_io_save;
#ifdef KUI
    extern int kui_async;
#endif /* KUI */
#endif /* OS2 */
    int is_tn = 0;

#ifdef IKSD
    if (inserver) {
        if (!quiet)
          printf("?Sorry, IKSD cannot CONNECT.\r\n");
        return(success = 0);
    }
#endif /* IKSD */

    is_tn =
#ifdef TNCODE
      (local && network && IS_TELNET()) || (!local && sstelnet)
#else
        0
#endif /* TNCODE */
          ;
/*
  Saving, changing, and restoring the global "quiet" variable around calls
  to conect() to control whether the verbose CONNECT message is printed is
  obviously less elegant than passing a parameter to conect(), but we do it
  this way to avoid the need to change all of the ck?con.c modules.  NOTE:
  it is important to restore the value immediately upon return in case there
  is an autodownload or APC.
*/
    qsave = quiet;                      /* Save it */
    if (!quiet && q > -1)
      quiet = q;                        /* Use argument temporarily */
    conres();                           /* Put console back to normal */
    debug(F101,"doconect justone 1","",justone);
#ifdef CK_AUTODL
    ksbuf[0] = NUL;                     /* Autodownload packet buffer */
#endif /* CK_AUTODL */
#ifdef OS2
    display_demo = 1;                   /* Remember to display demo */
#endif /* OS2 */

#ifdef IKS_OPTION
    if (is_tn && TELOPT_U(TELOPT_KERMIT) && ttchk() >= 0
#ifdef OS2
       && !viewonly
#endif /* OS2 */
        ) {
        /* If the remote side is in a state of IKS START-SERVER    */
        /* we request that the state be changed.  We will detect   */
        /* a failure to adhere to the request when we call ttinc() */
        if (!iks_wait(KERMIT_REQ_STOP,0) && !tcp_incoming) {
            if (!quiet) {
                printf("\r\nEnter Client/Server Mode...  Use:\r\n\r\n");
                printf(
" REMOTE LOGIN <user> <password> to log in to the server if necessary.\r\n");
                printf(" SEND and GET for file transfer.\r\n");
                printf(" REMOTE commands for file management.\r\n");
                printf(" FINISH to terminate Client/Server mode.\r\n");
                printf(" BYE to terminate and close connection.\r\n");
                printf(" REMOTE HELP for additional information.\r\n\r\n");
            }
            quiet = qsave;
            return(0);      /* Failure */
        }
    }

    /* Let our peer know our state. */
#ifdef CK_AUTODL
    if (is_tn && TELOPT_ME(TELOPT_KERMIT)
#ifdef OS2
        && !viewonly
#endif /* OS2 */
         ) {
        if (autodl && !TELOPT_SB(TELOPT_KERMIT).kermit.me_start) {
            tn_siks(KERMIT_START);      /* Send Kermit-Server Start */
        } else if (!autodl && TELOPT_SB(TELOPT_KERMIT).kermit.me_start) {
            tn_siks(KERMIT_STOP);
        }
    }
#else /* CK_AUTODL */
    if (is_tn && TELOPT_ME(TELOPT_KERMIT) &&
	TELOPT_SB(TELOPT_KERMIT).kermit.me_start)
        tn_siks(KERMIT_STOP);
#endif /* CK_AUTODL */
#endif /* IKS_OPTION */

    debug(F101,"doconect flow","",flow);
#ifdef OS2
    debug(F101,"doconect async","",async);
#ifdef KUI
    if (kui_async)
      async = 1;;
#endif /* KUI */
    x = conect(async);                  /* Connect the first time */
#else /* OS2 */
    x = conect();
#endif /* OS2 */
    debok = 1;

#ifdef IKS_OPTION
    if (TELOPT_U(TELOPT_KERMIT) &&
        TELOPT_SB(TELOPT_KERMIT).kermit.u_start &&
        !tcp_incoming && !quiet && ttchk() >= 0
        ) {
        printf("\r\nEnter Client/Server Mode...  Use:\r\n\r\n");
        printf(
" REMOTE LOGIN <user> <password> to log in to the server if necessary.\r\n");
        printf(" SEND and GET for file transfer.\r\n");
        printf(" REMOTE commands for file management.\r\n");
        printf(" FINISH to terminate Client/Server mode.\r\n");
        printf(" BYE to terminate and close connection.\r\n");
        printf(" REMOTE HELP for additional information.\r\n\r\n");
    }
#endif /* IKS_OPTION */

    quiet = qsave;                      /* Restore "quiet" value */
    debug(F101,"doconect justone 2","",justone);

#ifdef NETCONN
    if (network && tn_exit && ttchk() < 0)
      doexit(GOOD_EXIT,xitsta);         /* Exit with good status */
#endif /* NETCONN */

#ifdef OS2ORUNIX
    /* Exit on disconnect if the port is not open or carrier detect */
    if (exitonclose && (ttchk() < 0))
      doexit(GOOD_EXIT,xitsta);
#endif /* OS2ORUNIX */

#ifdef CKCONINTB4CB
    /* The order makes a difference in HP-UX 8.00. */
    /* The other order makes it think it's in the background when it */
    /* returns from CONNECT (Apr 1999). */
    setint();
    concb((char)escape);                /* Restore console for commands */
#else
    /* This is how it has always been so better leave it */
    /* this way for all non-HP-UX-8.00 builds. */
    concb((char)escape);                /* Restore console for commands */
    setint();
#endif /* CKCONINTB4CB */

#ifdef OS2
    if (!async) {
        term_io_save = term_io;         /* Disable I/O by emulator */
        term_io = 0;
#endif /* OS2 */

#ifdef CK_APC
/*
  If an APC command was received during CONNECT mode, we define it now
  as a macro, execute the macro, and then return to CONNECT mode.
  We do this in a WHILE loop in case additional APCs come during subsequent
  CONNECT sessions.
*/
        debug(F101,"doconect apcactive","",apcactive);
        debug(F101,"doconect success","",success);

        while (x > 0 && (apcactive == APC_LOCAL ||
                         (apcactive == APC_REMOTE && apcstatus != APC_OFF))) {
            debug(F101,"doconect justone 3","",justone);
            if (mlook(mactab,"_apc_commands",nmac) == -1) {
                debug(F110,"doconect about to execute APC",apcbuf,0);
                domac("_apc_commands",apcbuf,cmdstk[cmdlvl].ccflgs|CF_APC);
                delmac("_apc_commands",1);
#ifdef DEBUG
            } else {
                debug(F100,"doconect APC in progress","",0);
#endif /* DEBUG */
            }
            debug(F101,"doconect apcactive after domac","",apcactive);
            if (!apcactive) {               /* In case CLEAR APC was in APC */
                debug(F101,"doconect quit APC loop: apcactive","",apcactive);
                break;
            }
            /* Also don't reconnect if autodownload failed - very confusing! */
            /* Let them view the local screen to see what happened. - fdc */

            /* This should be conditional.  If someone is relying on the */
            /* connect mode autodownload for the kermit server to use with */
            /* a remotely executed script we should be able to return to */
            /* connect mode on the failure.  What we really need to do is */
            /* report the status of the transfer and then return to CONNECT. */
            /* In unix this would simply be a printf(), but in K95 it could */
            /* use a popup dialog to report the status. - Jeff */

#ifndef NOXFER
            debug(F101,"doconect xferstat","",xferstat);
            if (apcactive == APC_LOCAL && !xferstat && adl_err != 0) {
                debug(F101,"doconect quit APC loop: xferstat","",xferstat);
                apcactive = APC_INACTIVE;
                break;
            }
#endif /* NOXFER */
#ifdef OS2
            msleep(250);
#endif /* OS2 */
            debug(F101,"doconect justone 4","",justone);
            qsave = quiet;              /* Do this again... */
            if (!quiet && q > -1)
              quiet = q;
#ifdef CK_AUTODL
            ksbuf[0] = NUL;
#endif /* CK_AUTODL */
#ifdef IKS_OPTION
#ifdef CK_AUTODL
            if (is_tn &&
                TELOPT_ME(TELOPT_KERMIT) &&
                !TELOPT_SB(TELOPT_KERMIT).kermit.me_start &&
                autodl
#ifdef CK_APC
                && !apcactive
#endif /* CK_APC */
#ifdef OS2
                && !viewonly
#endif /* OS2 */
                ) {
                tn_siks(KERMIT_START);  /* Send Kermit-Server Start */
            }
#endif /* CK_AUTODL */
#endif /* IKS_OPTION */
#ifndef OS2
            x = conect();               /* Re-CONNECT. */
#else /* OS2 */
            x = conect(0);
            term_io = term_io_save;
#endif /* OS2 */
            debok = 1;
            quiet = qsave;
            debug(F101,"doconect justone 5","",justone);
#ifdef NETCONN
            if (network && ttchk() < 0) {
                if (tn_exit || exitonclose)
                  doexit(GOOD_EXIT,xitsta);
                else
                  break;
            }
#endif /* NETCONN */

#ifdef OS2ORUNIX
            /* If connection dropped */
            if (ttchk() < 0) {
                concb((char)escape);    /* Restore console. */
                if (exitonclose)
                  doexit(GOOD_EXIT,xitsta);
                else
                  break;
            }
#endif /* OS2ORUNIX */
        } /* Loop back for more. */
#endif /* CK_APC */

#ifndef NOKVERBS
        if ((keymac > 0) && (keymacx > -1)) { /* Executing a keyboard macro? */
            /* Set up the macro and return */
            /* Do not clear the keymac flag */
#ifdef OS2
	    term_io = term_io_save;
#endif /* OS2 */
            return(dodo(keymacx,NULL,CF_KMAC|cmdstk[cmdlvl].ccflgs));
        }
#endif /* NOKVERBS */
#ifdef OS2
        term_io = term_io_save;
    } /* if (!async) */
#endif /* OS2 */

#ifdef CKCONINTB4CB
    /* The order makes a difference in HP-UX 8.00. */
    /* The other order makes it think it's in the background when it */
    /* returns from CONNECT (Apr 1999). */
    setint();
    concb((char)escape);                /* Restore console for commands */
#else
    /* This is how it has always been so better leave it */
    /* this way for all non-HP-UX-8.00 builds. */
    concb((char)escape);                /* Restore console for commands */
    setint();
#endif /* CKCONINTB4CB */
#ifdef OS2
    if (!async)
#endif /* OS2 */
      what = W_COMMAND;                 /* Back in command mode. */
    return(x);                          /* Done. */
}
#endif /* NOLOCAL */

#ifndef NOICP
#ifdef COMMENT
/*
  It seemed that this was needed for OS/2, in which \v(cmdfile) and other
  file-oriented variables or functions can return filenames containing
  backslashes, which are subsequently interpreted as quotes rather than
  directory separators (e.g. see commented section for VN_CMDF below).
  But the problem can't be cured at this level.  Example:

    type \v(cmdfile)

  Without doubling, the filename is parsed correctly, but then when passed
  to UNIX 'cat' through the shell, the backslash is removed, and then cat
  can't open the file.  With doubling, the filename is not parsed correctly
  and the TYPE command fails immediately with a "file not found" error.
*/
/*
  Utility routine to double all backslashes in a string.
  s1 is pointer to source string, s2 is pointer to destination string,
  n is length of destination string, both NUL-terminated.
  Returns 0 if OK, -1 if not OK (destination string too short).
*/
int
dblbs(s1,s2,n) char *s1, *s2; int n; {
    int i = 0;
    while (*s1) {
        if (*s1 == '\\') {
            if (++i > n) return(-1);
            *s2++ = '\\';
        }
        if (++i > n) return(-1);
        *s2++ = *s1++;
    }
    *s2 = NUL;
    return(0);
}
#endif /* COMMENT */

char *
gmdmtyp() {                             /* Get modem type */
#ifndef NODIAL
    int i, x;

    debug(F111,"gmdmtyp","mdmtyp",mdmtyp);
    debug(F111,"gmdmtyp","mdmsav",mdmsav);

    x = mdmtyp;
    if (x < 0)                          /* In case of network dialing */
      x = mdmsav;
    if (x < 1)
      return("none");
    else
      for (i = 0; i < nmdm; i++)
        if ((mdmtab[i].kwval == x) && (mdmtab[i].flgs == 0))
          return(mdmtab[i].kwd);
#endif /* NODIAL */
    return("none");
}

#ifndef NOXMIT
#ifndef NOLOCAL
/*  T R A N S M I T  --  Raw upload  */

/*  Obey current line, duplex, parity, flow, text/binary settings. */
/*  Returns 0 upon apparent success, 1 on obvious failure.  */

/***
 Things to add:
 . Make both text and binary mode obey set file bytesize.
 . Maybe allow user to specify terminators other than CR?
 . Maybe allow user to specify prompts other than single characters?
 . Make STATISTICS also work for TRANSMIT.
 . If TRANSMIT is done without echo, make some kind of (optional) display.
 . Make the same optimization for binary-mode transmit that was done for
   text-mode (in the no-echo / no-prompt / no-pause case).
***/

/*  T R A N S M I T  --  Raw upload  */

/*  s is the filename, t is the turnaround (prompt) character  */

/*
  Maximum number of characters to buffer.
  Must be less than LINBUFSIZ
*/
#ifdef OS2
#define XMBUFS 4096                     /* For compatibility with XYZmodem */
#else /* OS2 */
#define XMBUFS 1024
#endif /* OS2 */

#ifdef TNCODE
#ifndef IAC
#define IAC 255
#endif /* IAC */
#endif /* TNCODE */

#define OUTXBUFSIZ 15
static CHAR inxbuf[OUTXBUFSIZ+1];       /* Host-to-screen expansion buffer */
static int inxcount = 0;                /* and count */
static CHAR outxbuf[OUTXBUFSIZ+1];      /* Keyboard-to-host expansion buf */
static int outxcount = 0;               /* and count */

/*  T R A N S M I T  --  Unguarded non-protocol file transmission  */
/*
  Call with:
    char * s:   Name of file to transmit.
    char t:     Turnaround char for text-mode transmission (normally LF).
    int xlate:  nonzero = charset translation for text-mode xfer, 0 = skip.
    int binary: nonzero = transmit in binary mode, 0 = in text mode.
*/
#define XBBUFSIZ 508			/* For binary blasting */
static CHAR xbbuf[XBBUFSIZ+4];

int
#ifdef CK_ANSIC
transmit(char * s, char t, int xlate, int binary, int xxecho)
#else
transmit(s,t,xlate,binary,xxecho) char *s; char t; int xlate, binary, xxecho;
#endif /* CK_ANSIC */
/* transmit */ {
#ifdef MAC
    extern char sstate;
    int count = 100;
#else
    int count = 0;
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
    int eof = 0;                        /* End of File flag */
    int eol = 0;                        /* End of Line flag */
    int rc = 1;                         /* Return code. 0=fail, 1=succeed. */
    int myflow;                         /* Local copy of global flow... */
    int is_tn = 0;                      /* Do Telnet negotiations */
    int xbufsiz = XMBUFS;               /* Size of TRANSMIT buffer */
    int x, y, c, i;                     /* Int workers... */
    int control = 0;                    /* Echo loop control */
    long nbytes = 0;                    /* File byte count */
    long zz;                            /* Long worker */
    char *p;                            /* Char * worker */

#ifdef PIPESEND
    extern int pipesend;
#endif /* PIPESEND */

#ifndef NOCSETS
    int tcs = TC_TRANSP;                /* Intermediate (xfer) char set */
    int langsv = L_USASCII;             /* Save current language */
    int unicode = 0;
    int tcssize = 0;

#ifdef CK_ANSIC /* ANSI C prototypes... */
    CHAR (*sxo)(CHAR);
    CHAR (*rxo)(CHAR);
    CHAR (*sxi)(CHAR);
    CHAR (*rxi)(CHAR);
#else /* Not ANSI C... */
    CHAR (*sxo)();
    CHAR (*rxo)();
    CHAR (*sxi)();
    CHAR (*rxi)();
#endif /* CK_ANSIC */
#ifdef UNICODE
    union ck_short uc;
    int bomorder = 0;
#ifdef CK_ANSIC
    extern int (*xl_ufc[MAXFCSETS+1])(USHORT);  /* Unicode to FCS */
    extern USHORT (*xl_fcu[MAXFCSETS+1])(CHAR); /* FCS to Unicode */
    extern int (*xuf)(USHORT);
    extern USHORT (*xfu)(CHAR);
#else
    extern int (*xl_ufc[MAXFCSETS+1])();
    extern USHORT (*xl_fcu[MAXFCSETS+1])();
    extern int (*xuf)();
    extern USHORT (*xfu)();
#endif /* CK_ANSIC */
#endif /* UNICODE */
#endif /* NOCSETS */

    debug(F101,"xmit t","",t);
    debug(F101,"xmit xlate","",xlate);
    debug(F101,"xmit binary","",binary);

#ifdef PIPESEND
    if (pipesend) {
        if (nopush) return(-2);
        if (zxcmd(ZIFILE,s) < 1) {
            printf("?Can't start command: %s\n",s);
            return(0);
        }
    } else
#endif /* PIPESEND */
    if (zopeni(ZIFILE,s) == 0) {        /* Open the file to be transmitted */
        printf("?Can't open file %s\n",s);
        return(0);
    }
    x = -1;                             /* Open the communication channel */
    if (ttopen(ttname,&x,mdmtyp,cdtimo) < 0) {  /* (no harm if already open) */
        printf("Can't open device %s\n",ttname);
        return(0);
    }
    zz = x ? speed : -1L;
    if (binary) {                       /* Binary file transmission */
        myflow = (flow == FLO_XONX) ? FLO_NONE : flow;

        if (ttvt(zz,myflow) < 0) {      /* So no Xon/Xoff! */
            printf("Can't condition line\n");
            return(0);
        }
    } else {
        if (ttpkt(zz,flow,parity) < 0) { /* Put the line in "packet mode" */
            printf("Can't condition line\n"); /* so Xon/Xoff will work, etc. */
            return(0);
        }
    }
    is_tn =
#ifdef TNCODE
      (local && network && IS_TELNET()) || (!local && sstelnet)
#else
        0
#endif /* TNCODE */
          ;

#ifndef NOCSETS
/* Set up character set translations */

    tcs = 0;                            /* "Transfer" or "Other" charset */
    sxo = rxo = NULL;                   /* Initialize byte-to-byte functions */
    sxi = rxi = NULL;
    unicode = 0;                        /* Assume Unicode won't be involved */
    if (!binary && xlate) {             /* Set up charset translations */
/*
  In the SENDING direction, we are converting from the local file's
  character-set (fcharset) to the remote terminal charset (tcsr).  In the
  RECEIVING direction (echoing) we are converting from the remote end of the
  terminal charset (tcsr) to its local end (tcsl), which is not necessarily
  the same as the file character-set.  Especially when the file character
  set is UCS-2, which is not a valid terminal character set.  The various
  combinations are represented in this table:

  FCS = File Character Set
  RCS = Remote Terminal Character Set
  CCS = Console (Local Terminal) Character Set

   8   4   2   1
  FCS FCS RCS CCS
  UCS UTF UTF UTF
   0   0   0   0   =   0   =   No translation
   0   0   0   1   =   1   =   FCS -> RCS, Echo RCS -> UTF
   0   0   1   0   =   2   =   FCS -> UTF, Echo UTF -> CCS
   0   0   1   1   =   3   =   FCS -> UTF, Echo no translation

   0   1   0   0   =   4   =   UTF -> RCS, Echo RCS -> CCS
   0   1   0   1   =   5   =   UTF -> RCS, Echo RCS -> UTF
   0   1   1   0   =   6   =   UTF -> UTF, Echo UTF -> CCS
   0   1   1   1   =   7   =   No translation

   1   0   0   0   =   8   =   UCS -> RCS, Echo RCS -> CCS
   1   0   0   1   =   9   =   UCS -> RCS, Echo RCS -> UTF
   1   0   1   0   =  10   =   UCS -> UTF, Echo UTF -> CCS
   1   0   1   1   =  11   =   UCS -> UTF, Echo no translation
*/
#ifdef UNICODE
        xfu = NULL;                     /* Unicode translation functions */
        xuf = NULL;
        bomorder = ucsorder;            /* UCS-2 byte order */

        if (fcharset == FC_UCS2)        /* File charset is UCS-2 */
          unicode |= 8;
        else if (fcharset == FC_UTF8)   /* File charset is UTF-8 */
          unicode |= 4;
        if (tcsr == FC_UTF8)            /* Remote term charset is UTF-8 */
          unicode |= 2;
        if (tcsl == FC_UTF8)            /* Local term charset is UTF-8 */
          unicode |= 1;
#endif /* UNICODE */
/*
  When Unicode not involved -- TCS is the intermediate (xfer) set, and:
  sxo = File-to-Intermediate charset function
  rxo = Intermediate-to-Remote-Terminal charset function
  sxi = Remote-Terminal-to-Intermediate
  rxi = Intermediate-to-Local-Terminal
*/
        tcs = gettcs(tcsr,fcharset);    /* Get intermediate set. */
        sxo = xls[tcs][fcharset];       /* translation function */
        rxo = xlr[tcs][tcsr];           /* pointers for output functions */
        sxi = xls[tcs][tcsr];           /* and for input functions. */
        rxi = xlr[tcs][tcsl];
/*
  At this point we have unicode nonzero if Unicode is involved in the
  conversion, and to 0 if it is not.
  The following is to prevent use of zmstuff() and zdstuff() by translation
  functions (stuffing works with file i/o, not with communication i/o).
*/
        langsv = language;              /* Save current SET LANGUAGE */
        language = L_USASCII;           /* No language-specific translations */
    }
#endif /* NOCSETS */

    i = 0;                              /* Beginning of buffer. */
#ifndef MAC
#ifndef AMIGA
    oldsig = signal(SIGINT, trtrap);    /* Save current interrupt trap. */
#endif /* AMIGA */
#endif /* MAC */
    tr_int = 0;                         /* Have not been interrupted (yet). */
    rc = 1;                             /* Return code presumed good. */
#ifdef VMS
    conres();
#endif /* VMS */

#ifndef NOCSETS
    debug(F101,"XMIT unicode","",unicode);
#ifdef UNICODE
    debug(F101,"XMIT bomorder","",bomorder);
#endif /* UNICODE */
#endif /* NOCSETS */

    c = 0;                              /* Initial condition */
    while (c > -1 && !eof) {            /* Loop for all characters in file */
        eol = 0;
#ifdef MAC
        /*
         * It is expensive to run the miniparser so don't do it for
         * every character.
         */
        if (--count < 0) {
            count = 100;
            miniparser(1);
            if (sstate == 'a') {
                sstate = '\0';
                goto xmitfail;
            }
        }
#else /* Not MAC */
        if (tr_int) {                   /* Interrupted? */
            printf("^C...\n");          /* Print message */
            goto xmitfail;
        }
#endif /* MAC */
        c = zminchar();                 /* Get a file character */
#ifdef COMMENT
/* too much */
#ifdef DEBUG
        if (deblog) {
            if (c < 0)
              debug(F101,"XMIT zminchar","",c);
            else
              debug(F000,"XMIT zminchar","",c);
        }
#endif /* DEBUG */
#endif /* COMMENT */
        if (c < -1) {                   /* Other error */
            printf("?TRANSMIT file read error: %s\n",ck_errstr());
            goto xmitfail;
        } else if (c > -1) {
            nbytes++;
            c &= fmask;                 /* Apply SET FILE BYTESIZE mask */
        } else if (c == -1) {
            eof = 1;
            debug(F101,"XMIT eof","",eof);
        }
        if (binary) {                   /* Binary... */
            if (c == -1) {              /* If EOF */
                rc = 1;                 /* Success */
                eof = 1;
                goto xmitexit;          /* Done */
            }
            if (!xmitw && !xxecho) {    /* Special "blast" mode */
                if (count == XBBUFSIZ) { /* File input buffer full... */
                    while (count > 0) {
                        errno = 0;
                        y = ttol(xbbuf,count);
                        if (y < 0) {    /* try to send it. */
                            printf("?TRANSMIT output error: %s\n",
                                   ck_errstr());
                            debug(F111,"XMIT binary ttol error",
                                  ck_errstr(),errno);
                            rc = 0;
                            break;
                        }
                        if (y < 0) break;
                        count -= y;
                    }
                    count = 0;
                }
                xbbuf[count++] = c;
#ifdef TNCODE
                if (c == IAC && is_tn)  /* Telnet IAC */
                  xbbuf[count++] = IAC; /* must be doubled */
#endif /* TNCODE */
                continue;
            }
            if (ttoc(dopar((char) c)) < 0) { /* else just send the char */
                printf("?Can't transmit character\n");
                goto xmitfail;
            }
#ifdef TNCODE
            if (c == IAC && is_tn)      /* Quote Telnet IAC */
              ttoc((char)IAC);
#endif /* TNCODE */

            if (xmitw)                  /* Pause if requested */
              msleep(xmitw);

            if (xxecho) {               /* SET TRANSMIT ECHO ON? */
                if (duplex) {           /* Yes, for half duplex */
#ifndef NOLOCAL
#ifdef OS2
                    /* Echo to emulator */
                    scriptwrtbuf((USHORT)(c & cmdmsk));
#endif /* OS2 */
#endif /* NOLOCAL */
                    if (conoc((char)(c & cmdmsk)) < 0) /* echo locally. */
                      goto xmitfail;
                } else {                /* For full duplex, */
                    int i, n;           /* display whatever is there. */
                    n = ttchk();        /* See how many chars are waiting */
                    if (n < 0) {        /* Connection dropped? */
                        printf("?Connection lost\n");
                        goto xmitfail;
                    }
                    for (i = 0; i < n; i++) { /* Read and echo that many. */
                        x = ttinc(xmitt); /* Timed read just in case. */
                        if (x > -1) {   /* If no timeout */
                            if (parity) x &= 0x7f; /* display the char, */
#ifndef NOLOCAL
#ifdef OS2
                            /* Echo to emulator */
                            scriptwrtbuf((USHORT)x);
#endif /* OS2 */
#endif /* NOLOCAL */
                            if (conoc((char)(x & cmdmsk)) < 0) {
                                printf("?Output error\n");
                                goto xmitfail;
                            }
                        } else if (x == -2) {
                            printf("Connection closed.\n");
                            ttclos(1);
                            goto xmitfail;
                        } else if (x == -3) {
                            printf(
                            "Session Limit exceeded - closing connection.\n"
                                   );
                            ttclos(1);
                            goto xmitfail;
                        } else {
                            printf("?Communications error\n");
                            goto xmitfail;
                        }
                    }
                }
            } else ttflui();            /* Not echoing, just flush input. */

        } else {                        /* Text mode, line at a time. */
#ifdef UNICODE
            if (fcharset == FC_UCS2 && xlate) { /* Special for UCS-2 */
                char xbuf[8];
                x = 1 - (nbytes & 1);   /* Odd or even byte */
                if (x == 0)             /* Note: 1 = the 1st, 0 = 2nd, etc */
                  uc.x_short = 0;
                if (bomorder)           /* Little Endian */
                  x = 1 - x;            /* Save byte in appropriate half */
                debug(F101,"XMIT UCS2 x","",x);
                uc.x_char[x] = (CHAR) (c & 0xff);
                if (nbytes & 1)         /* First byte, go back for next */
                  continue;
                if (nbytes == 2) {      /* UCS-2 Byte Order Mark */
                    if (uc.x_short == (USHORT) 0xfeff) {
                        debug(F100,"XMIT UCS2 BOM FEFF","",bomorder);
                        continue;
                    } else if (uc.x_short == (USHORT) 0xfffe) {
                        bomorder = 1 - bomorder;
                        debug(F100,"XMIT UCS2 BOM FFFE (swap)","",bomorder);
                        continue;
                    }
                }
                sprintf(xbuf,"%04X",uc.x_short); /* SAFE */
                debug(F111,"XMIT UCS2",xbuf,uc.x_short);
                if (nbytes & 1)         /* Special eol test for UCS-2 */
                  if (uc.x_short == '\n')
                    eol = 1;
#ifdef COMMENT
                if (uc.x_short == 0x2028 || uc.x_short == 0x2029)
                    eol = 1;
#endif /* COMMENT */
            } else
#endif /* UNICODE */
              if (c == '\n') {          /* Normal eol test otherwise */
                  eol = 1;
            }
            if (eol) {                  /* End of line? */
                int stuff = -1;
                debug(F101,"XMIT eol length","",i);
                if (i == 0) {           /* Blank line? */
                    if (xmitf)          /* Yes, insert fill if asked. */
                      line[i++] = dopar((char) xmitf);
                }
                if (i == 0 || ((char) line[i-1]) != ((char) dopar(CR)))
                  line[i++] = dopar(CR); /* Terminate it with CR */
                if (xmitl) {
                    stuff = LF;
#ifdef TNCODE
                } else if (is_tn && (tn_nlm != TNL_CR)) {
                    /* TELNET NEWLINE ON/OFF/RAW */
                    stuff = (tn_nlm == TNL_CRLF) ? LF : NUL;
#endif /* TNCODE */
                }
                if (stuff > -1)
                  line[i++] = dopar((char)stuff);
                line[i] = NUL;
                debug(F111,"XMIT eol line",line,i);

            } else if (c != -1) {       /* Not a newline, regular character */
                int k, x;
                outxbuf[0] = c;         /* In case of no translation */
                outxcount = 1;          /* Assume result is one byte */
#ifndef NOCSETS
                switch (unicode) {
                  case 0:               /* No Unicode involved */
                  case 1:
                    if (xlate) {        /* If not /TRANSPARENT */
                        /* Local-to-intermediate */
                        if (sxo) c = (*sxo)((char)c);
                        /* Intermediate-to-remote */
                        if (rxo) c = (*rxo)((char)c);
                        outxbuf[0] = c;
                    }
                    break;
#ifdef UNICODE
                  case 2:               /* Local byte to UTF-8 */
                  case 3:
                    xfu = xl_fcu[fcharset];
                    tcssize = fcsinfo[fcharset].size;
                    outxcount =
                      b_to_u((CHAR)c,outxbuf,OUTXBUFSIZ,tcssize);
                    break;
                  case 4:               /* Local UTF-8 to remote byte */
                  case 5:
                    xuf = xl_ufc[tcsr];
                    x = u_to_b((CHAR)c); /* Convert to byte */
                    if (x == -1) {      /* If more input bytes needed */
                        continue;       /* go back and get them */
                    } else if (x == -2) { /* LS or PS (shouldn't happen) */
                        outxbuf[0] = CR;
                    } else if (x == -9) { /* UTF-8 error */
                        outxbuf[0] = '?'; /* Insert error char */
                        outxbuf[1] = u_to_b2(); /* Insert next char */
                        outxcount = 2;
                    } else {
                        outxbuf[0] =    /* Otherwise store result */
                          (unsigned)(x & 0xff);
                    }
                    break;
                  case 6:               /* UTF-8 to UTF-8 */
                  case 7:
                    break;
                  case 8:               /* UCS-2 to byte */
                  case 9:
                    xuf = xl_ufc[tcsr];
                    outxbuf[0] = (*xuf)(uc.x_short);
                    break;
                  case 10:
                  case 11: {            /* UCS-2 to UTF-8 */
                      int j;
                      CHAR * buf = NULL;
                      x = ucs2_to_utf8(uc.x_short,&buf);
                      if (x < 0) {
                          outxbuf[0] = 0xff; /* (= U+FFFD) */
                          outxbuf[1] = 0xbd;
                          x = 2;
                      }
                      for (j = 0; j < x; j++)
                        outxbuf[j] = buf[j];
                      outxcount = x;
                      break;
                  }
#endif /* UNICODE */
                }
#endif /* NOCSETS */
                outxbuf[outxcount] = NUL;
                debug(F111,"XMIT outxbuf",outxbuf,outxcount);
/*
  Now the input character (1 or more bytes) is translated into the output
  expansion buffer (1 or more bytes); outxcount = number of bytes to add to
  the TRANSMIT line buffer, which we do here, taking care of parity, SI/SO
  processing, and quoting Telnet IACs.
*/
                for (k = 0; k < outxcount; k++) {
                    c = outxbuf[k];
                    if (xmits && parity && (c & 0200)) { /* If shifting */
                        line[i++] = dopar(SO); /* needs to be done, */
                        line[i++] = dopar((char)c); /* do it here, */
                        line[i++] = dopar(SI); /* crudely. */
                    } else {
                        line[i++] = dopar((char)c);
#ifdef TNCODE
                        if (c == (CHAR)IAC && is_tn)
                          line[i++] = (CHAR)IAC;
#endif /* TNCODE */
                    }
                }
            }
/*
  Send characters if buffer full, or at end of line, or at end of file.
  (End of line only if echoing, waiting for a prompt, or pausing.)
*/
            debug(F000,"XMIT c",ckitoa(i),c);
            if (i >= xbufsiz || eof || (eol && (xxecho || xmitw || t))) {
                p = line;
                line[i] = '\0';
                debug(F111,"transmit buf",p,i);
                if (ttol((CHAR *)p,i) < 0) { /* try to send it. */
                    printf("?TRANSMIT output error: %s\n",ck_errstr());
                    rc = 0;
                    break;
                }
                i = 0;                  /* Reset buffer pointer. */
/*
  Now we handle the echo.  If the user wants to see it, or if we have to
  wait for the turnaround character, t.  If the echo is being displayed,
  and terminal character-set translation is required, we do it here.
*/
                if (duplex && xxecho) {  /* If local echo, echo it */
                    if (parity || cmdmsk == 0x7f) { /* Strip hi bits */
                        char *ss = line;             /* if necessary */
                        while (*ss) {
                            *ss &= 0x7f;
                            ss++;
                        }
                    }
#ifndef NOLOCAL
#ifdef OS2
                    {                   /* Echo to emulator */
                        char *ss = p;
                        while (*ss) {
                            scriptwrtbuf((USHORT)*ss);
                            ss++;
                        }
                    }
#endif /* OS2 */
#endif /* NOLOCAL */
                    if (conoll(p) < 0)
                      goto xmitfail;
                }
                if (xmitw)              /* Sleep TRANSMIT PAUSE interval */
                  msleep(xmitw);

                control = 0;            /* Readback loop control */
                if (t != 0 && eol)      /* TRANSMIT PROMPT given and at EOL */
                  control |= 1;
                if (xxecho && !duplex)   /* Echo desired and is remote */
                  control |= 2;

                if (control) {          /* Do this if reading back the echo */
                    int n;
                    x = 0;
                    while (1) {
                        if (control & 1) { /* Termination criterion */
                            if (x == t)    /* for turnaround */
                              break;
                        } else if (control & 2) { /* And for echoing */
                            if ((n = ttchk()) < 1)
                              break;
                        }
                        if ((x = ttinc(xmitt)) < 0) { /* Read with timeout */
                            switch (x) {
                              case -2:
                                printf("Connection closed.\n");
                                ttclos(1);
                                goto xmitfail;
                              case -3:
                                printf(
                              "Session Limit exceeded - closing connection.\n"
                                       );
                                ttclos(1); /* full thru... */
                                goto xmitfail;
                              default:
                                printf("?Timeout\n");
                                goto xmitfail;
                            }
                        }
                        if (x > -1 && (control & 2)) { /* Echo any echoes */
                            if (parity)
                              x &= 0x7f;
                            c = x;
#ifndef NOLOCAL
#ifdef OS2
                            scriptwrtbuf((USHORT)x);
#endif /* OS2 */
#endif /* NOLOCAL */
                            inxbuf[0] = c;
                            inxcount = 1;
#ifndef NOCSETS
                            switch (unicode & 3) { /* Remote bits */
                              case 0:
                                if (xlate) {
                                    if (sxi) c = (*sxi)((CHAR)c);
                                    if (rxi) c = (*rxi)((CHAR)c);
                                    inxbuf[0] = c;
                                }
                                break;
#ifdef UNICODE
                              case 1:   /* Remote Byte to local UTF-8 */
                                xfu = xl_fcu[tcsr];
                                tcssize = fcsinfo[tcsr].size;
                                inxcount =
                                  b_to_u((CHAR)c,
                                         inxbuf,
                                         OUTXBUFSIZ,
                                         tcssize
                                         );
                                break;
                              case 2:   /* Remote UTF-8 to local Byte */
                                xuf = xl_ufc[tcsl];
                                x = u_to_b((CHAR)c);
                                if (x < 0)
                                  continue;
                                inxbuf[0] = (unsigned)(x & 0xff);
                                break;
                              case 3:   /* UTF-8 to UTF-8 */
                                break;
#endif /* UNICODE */
                            }
#endif /* NOCSETS */
                            inxbuf[inxcount] = NUL;
                            if (conxo(inxcount,(char *)inxbuf) < 0)
                              goto xmitfail;
                        }
                    }
                } else                  /* Not echoing */
                  ttflui();             /* Just flush input buffer */
            } /* End of buffer-dumping block */
        } /* End of text mode */
        if (eof) {
            rc = 1;
            goto xmitexit;
        }
    } /* End of character-reading loop */

  xmitfail:                             /* Failure exit point */
    rc = 0;

  xmitexit:                             /* General exit point */
    if (rc > 0) {
        if (binary && !xmitw && !xxecho) { /* "blasting"? */
            while (count > 0) {            /* Partial buffer still to go? */
                errno = 0;
                y = ttol(xbbuf,count);
                if (y < 0) {
                    printf("?TRANSMIT output error: %s\n",
                           ck_errstr());
                    debug(F111,"XMIT binary eof ttol error",
                          ck_errstr(),errno);
                    rc = 0;
                    break;
                }
                count -= y;
            }
        } else if (!binary && *xmitbuf) { /* Anything to send at EOF? */
            p = xmitbuf;                /* Yes, point to string. */
            while (*p)                  /* Send it. */
              ttoc(dopar(*p++));        /* Don't worry about echo here. */
        }
    }

#ifndef AMIGA
#ifndef MAC
    signal(SIGINT,oldsig);              /* Put old signal action back. */
#endif /* MAC */
#endif /* AMIGA */
#ifdef VMS
    concb(escape);                      /* Put terminal back, */
#endif /* VMS */
    zclose(ZIFILE);                     /* Close file, */
#ifndef NOCSETS
    language = langsv;                  /* restore language, */
#endif /* NOCSETS */
    ttres();                            /* and terminal modes, */
    return(rc);                         /* and return successfully. */
}
#endif /* NOLOCAL */
#endif /* NOXMIT */

#ifndef NOCSETS

_PROTOTYP( CHAR (*sxx), (CHAR) );       /* Local translation function */
_PROTOTYP( CHAR (*rxx), (CHAR) );       /* Local translation function */
_PROTOTYP( CHAR zl1as, (CHAR) );        /* Latin-1 to ascii */
_PROTOTYP( CHAR xl1as, (CHAR) );        /* ditto */

/*  X L A T E  --  Translate a local file from one character set to another */

/*
  Translates input file (fin) from character set csin to character set csout
  and puts the result in the output file (fout).  The two character sets are
  file character sets from fcstab.
*/

int
xlate(fin, fout, csin, csout) char *fin, *fout; int csin, csout; {

#ifndef MAC
#ifdef OS2
    extern int k95stdout;
    extern int wherex[], wherey[];
    extern unsigned char colorcmd;
#ifdef NT
    SIGTYP (* oldsig)(int);             /* For saving old interrupt trap. */
#else /* NT */
    SIGTYP (* volatile oldsig)(int);    /* For saving old interrupt trap. */
#endif /* NT */
#else /* OS2 */
    SIGTYP (* oldsig)();
#endif /* OS2 */
#endif /* MAC */
#ifdef CK_ANSIC
    int (*fn)(char);                    /* Output function pointer */
#else
    int (*fn)();
#endif /* CK_ANSIC */
    extern int xlatype;
    int filecode;                       /* Code for output file */
    int scrnflg = 0;

    int z = 1;                          /* Return code. */
    int x, c, c2;                       /* Workers */
#ifndef UNICODE
    int tcs;
#endif /* UNICODE */

    ffc = 0L;

    if (zopeni(ZIFILE,fin) == 0) {      /* Open the file to be translated */
#ifdef COMMENT
        /* An error message was already printed by zopeni() */
        printf("?Can't open input file %s\n",fin);
#endif /* COMMENT */
        return(0);
    }
#ifdef MAC
/*
  If user specified no output file, it goes to the screen.  For the Mac,
  this must be done a special way (result goes to a new window); the Mac
  doesn't have a "controlling terminal" device name.
*/
    filecode = !strcmp(fout,CTTNAM) ? ZCTERM : ZOFILE;
#else
#ifdef VMS
    filecode = !strcmp(fout,CTTNAM) ? ZCTERM : ZMFILE;
#else
#ifdef OS2
    filecode = (!stricmp(fout,"con") || !stricmp(fout,"con:")) ?
        ZCTERM : ZMFILE;
    if ((filecode == ZCTERM) && !k95stdout && !inserver)
        csout = FC_UCS2;
#else /* OS2 */
    filecode = ZOFILE;
#endif /* OS2 */
#endif /* VMS */
#endif /* MAC */
    if (zopeno(filecode,fout,NULL,NULL) == 0) { /* And the output file */
        printf("?Can't open output file %s\n",fout);
        return(0);
    }
#ifndef AMIGA
#ifndef MAC
    oldsig = signal(SIGINT, trtrap);    /* Save current interrupt trap. */
#endif /* MAC */
#endif /* AMIGA */

    scrnflg = (filecode == ZCTERM);     /* Set output function */
    if (scrnflg)
      fn = NULL;
    else if (filecode == ZMFILE)
      fn = putmfil;
    else
      fn = putfil;

    tr_int = 0;                         /* Have not been interrupted (yet). */
    z = 1;                              /* Return code presumed good. */

    if (!scrnflg && !quiet)
      printf(" %s (%s) => %s (%s)\n",   /* Say what we're doing. */
             fin, fcsinfo[csin].keyword,
             fout,fcsinfo[csout].keyword
             );

#ifndef UNICODE
/*
  Non-Unicode picks the "most appropriate" transfer character set as the
  intermediate set, which results in loss of any characters that the source
  and target sets have in common, but are lacking from the intermediate set.
*/
#ifdef KANJI
    /* Special handling for Japanese... */

    if (fcsinfo[csin].alphabet == AL_JAPAN ||
         fcsinfo[csout].alphabet == AL_JAPAN) {
        USHORT eu;
        int c, x, y;

        xpnbyte(-1,0,0,NULL);           /* Reset output machine */
        xlatype = XLA_JAPAN;

        while ((c = xgnbyte(FC_JEUC,csin,NULL)) > -1) { /* Get an EUC byte */
            if (tr_int) {               /* Interrupted? */
                printf("^C...\n");      /* Print message */
                z = 0;
                break;
            }
            /* Send EUC byte to output machine */
            if ((x = xpnbyte(c,TC_JEUC,csout,fn)) < 0) {
                z = -1;
                break;
            }
        }
        goto xxlate;
    }
#endif /* KANJI */

    /* Regular bytewise conversion... */

    tcs = gettcs(csin,csout);           /* Get intermediate set. */
    if (csin == csout) {                /* Input and output sets the same? */
        sxx = rxx = NULL;               /* If so, no translation. */
    } else {                            /* Otherwise, set up */
        if (tcs < 0 || tcs > MAXTCSETS ||
            csin < 0 || csin > MAXFCSETS ||
            csout < 0 || csout > MAXFCSETS) {
            debug(F100,"XLATE csets out of range","",0);
            sxx = rxx = NULL;
        } else {
            sxx = xls[tcs][csin];       /* translation function */
            rxx = xlr[tcs][csout];      /* pointers. */
            if (rxx == zl1as) rxx = xl1as;
        }
    }
    while ((c = zminchar()) != -1) { /* Loop for all characters in file */
        if (tr_int) {                   /* Interrupted? */
            printf("^C...\n");          /* Print message */
            z = 0;
            break;
        }
        if (sxx) c = (*sxx)((CHAR)c);   /* From fcs1 to tcs */
        if (rxx) c = (*rxx)((CHAR)c);   /* from tcs to fcs2 */
        if (zchout(filecode,(char)c) < 0) { /* Output xlated character */
            z = -1;
            break;
        }
    }
    goto xxlate;                        /* Done. */

#else  /* UNICODE */
/*
   Use Unicode as the intermediate character set.  It's simple and gives
   little or no loss, but the overhead is a bit higher.
*/
    initxlate(csin,csout);              /* Set up translation functions */

    if (xlatype == XLA_NONE) {
        while ((c = zminchar()) != -1) { /* Loop for all characters in file */
            if (tr_int) {               /* Interrupted? */
                printf("^C...\n");      /* Print message */
                z = 0;
                break;
            }
            if (zchout(filecode,(char)c) < 0) { /* Output xlated character */
                z = -1;
                break;
            }
        }
        goto xxlate;                    /* Done. */
    }


#ifndef NOLOCAL
#ifdef OS2
    if (csout == FC_UCS2 &&             /* we're translating to UCS-2 */
        filecode == ZCTERM &&           /* for the real screen... */
        !k95stdout && !inserver
        ) {
        union {
            USHORT ucs2;
            UCHAR  bytes[2];
        } output;

        while (1) {                     /* In this case we go two-by-two. */
            if ((c = xgnbyte(FC_UCS2,csin,NULL)) < 0)
              break;
            output.bytes[0] = c;
            if ((c = xgnbyte(FC_UCS2,csin,NULL)) < 0)
              break;
            output.bytes[1] = c;

            if (tr_int) {               /* Interrupted? */
                printf("^C...\n");      /* Print message */
                z = 0;
                break;
            }

            VscrnWrtUCS2StrAtt(VCMD,
                               &output.ucs2,
                               1,
                               wherey[VCMD],
                               wherex[VCMD],
                               &colorcmd
                               );
        }
    } else
#endif /* OS2 */
#endif /* NOLOCAL */

      /* General case: Get next byte translated from fcs to UCS-2 */

#ifdef COMMENT
      while ((c = xgnbyte(FC_UCS2,csin,NULL)) > -1 &&
              (c2 = xgnbyte(FC_UCS2,csin,NULL)) > -1) {
          extern int fileorder;

          if (tr_int) {                 /* Interrupted? */
              printf("^C...\n");        /* Print message */
              z = 0;
              break;
          }
          debug(F001,"XLATE c","",c);
          debug(F001,"XLATE c2","",c2);

          /* And then send UCS-2 byte to translate-and-output machine */

          if ((x = xpnbyte(fileorder?c2:c,TC_UCS2,csout,fn)) < 0) {
              z = -1;
              break;
          }
          if ((x = xpnbyte(fileorder?c:c2,TC_UCS2,csout,fn)) < 0) {
              z = -1;
              break;
          }
      }
#else
    while ((c = xgnbyte(FC_UCS2,csin,NULL)) > -1) {
          if (tr_int) {                 /* Interrupted? */
              printf("^C...\n");        /* Print message */
              z = 0;
              break;
          }
          if ((x = xpnbyte(c,TC_UCS2,csout,fn)) < 0) {
              z = -1;
              break;
          }
      }
#endif /* COMMENT */

#endif /* UNICODE */

  xxlate:                               /* Common exit point */

#ifndef AMIGA
#ifndef MAC
    signal(SIGINT,oldsig);              /* Put old signal action back. */
#endif /* MAC */
#endif /* AMIGA */
    tr_int = 0;
    if (z < 0) {
        if (z == -1)
          printf("?File output error: %s\n",ck_errstr());
        z = 0;
    }
    zclose(ZIFILE);                     /* Close files */
    zclose(filecode);                   /* ... */
    return(success = z);                /* and return status. */
}

int
doxlate() {
#ifdef OS2ONLY
    extern int tt_font;
#endif /* OS2ONLY */
#ifdef UNIX
    extern char ** mtchs;               /* zxpand() file list */
#endif /* UNIX */
    int x, y, incs, outcs, multiple = 0, wild = 0, fc = 0, len = 0;
    int ofisdir = 0;
    char * s, * tocs = "";

    if ((x = cmifi("File(s) to translate","",&s,&wild,xxstring)) < 0) {
        if (x == -3) {
            printf("?Name of an existing file\n");
            return(-9);
        } else
          return(x);
    }
    ckstrncpy(line,s,LINBUFSIZ);        /* Save copy of string just parsed. */

    if ((incs = cmkey(fcstab,nfilc,"from character-set","",xxstring)) < 0)
      return(incs);

#ifdef OS2
    if (isunicode())
      tocs = "ucs2";
    else
#endif /* OS2 */
      tocs = getdcset();

    if ((outcs = cmkey(fcstab,nfilc,"to character-set",tocs,xxstring)) < 0)
      return(outcs);
    if ((x = cmofi("output file",CTTNAM,&s,xxstring)) < 0) return(x);
    if (x > 1)
      ofisdir = 1;

    len = ckstrncpy(tmpbuf,s,TMPBUFSIZ);
    if ((y = cmcfm()) < 0) return(y);   /* Confirm the command */

    if (len < 1)
      return(-2);

    if (ofisdir)
      multiple = 2;
    else if (wild) {
        if (isdir(tmpbuf))
          multiple = 2;
        else if (!strcmp(tmpbuf,CTTNAM))
          multiple = 1;
#ifdef OS2
        else if (!stricmp(tmpbuf,"con") || !stricmp(tmpbuf,"con:"))
          multiple = 1;
#else
#ifdef UNIXOROSK
        else if (!strncmp(tmpbuf,"/dev/",4))
          multiple = 1;
#endif /* UNIXOROSK */
#endif /* OS2 */
        if (!multiple) {
            printf("?A single file please\n");
            return(-9);
        }
    }
    if (!multiple) {                    /* Just one file */
        return(success = xlate(line,tmpbuf,incs,outcs));
    } else {                            /* Translate multiple files */
        char dirbuf[CKMAXPATH+4];
        int k;
#ifndef ZXREWIND
        int flags = ZX_FILONLY;
#endif /* ZXREWIND */

        if (multiple == 2) {            /* Target is a directory */
            k = ckstrncpy(dirbuf,tmpbuf,CKMAXPATH+1) - 1;
            if (k < 0)
              return(-2);
#ifdef OS2ORUNIX
            if (dirbuf[k] != '/') {
                dirbuf[k+1] = '/';
                dirbuf[k+2] = NUL;
            }
#else
#ifdef OSK
            if (dirbuf[k] != '/') {
                dirbuf[k+1] = '/';
                dirbuf[k+2] = NUL;
            }
#else
#ifdef VMS
            if (ckmatch("*.DIR;1",s,0,0))
              k = cvtdir(tmpbuf,dirbuf,TMPBUFSIZ);
            if (dirbuf[k] != ']' &&
                dirbuf[k] != '>' &&
                dirbuf[k] != ':')
              return(-2);
#else
#ifdef datageneral
            if (dirbuf[k] != ':') {
                dirbuf[k+1] = ':';
                dirbuf[k+2] = NUL;
            }
#else
#ifdef STRATUS
            if (dirbuf[k] != '>') {
                dirbuf[k+1] = '>';
                dirbuf[k+2] = NUL;
            }
#endif /* STRATUS */
#endif /* datageneral */
#endif /* VMS */
#endif /* OSK */
#endif /* OS2ORUNIX */
        }

#ifdef ZXREWIND
        fc = zxrewind();                /* Rewind the file list */
#else
        if (matchdot)  flags |= ZX_MATCHDOT;
        fc = nzxpand(line,flags);
#endif /* ZXREWIND */

        if (fc < 1) {
            printf("?Wildcard expansion error\n");
            return(-9);
        }
#ifdef UNIX
        sh_sort(mtchs,NULL,fc,0,0,filecase); /* Sort the file list */
#endif /* UNIX */

        while (1) {                     /* Loop through the files */
            znext(line);
            if (!line[0])
              break;
            if (multiple == 2)
              ckmakmsg(tmpbuf,TMPBUFSIZ,dirbuf,line,NULL,NULL);
            if (xlate(line,tmpbuf,incs,outcs) < 1)
              return(success = 0);
        }
    }
    return(success = 1);
}
#endif /* NOCSETS */

static char hompthbuf[CKMAXPATH+1];

char *
homepath() {
    int x;
    extern char * myhome;
    char * h;

    h = myhome ? myhome : zhome();
    hompthbuf[0] = NUL;
#ifdef UNIXOROSK
    x = ckstrncpy(hompthbuf,h,CKMAXPATH+1);
    if (x <= 0) {
        hompthbuf[0] = '/';
        hompthbuf[1] = NUL;
    } else if (x < CKMAXPATH - 2 && hompthbuf[x-1] != '/') {
        hompthbuf[x] = '/';
        hompthbuf[x+1] = NUL;
    }
    return(hompthbuf);
#else
#ifdef STRATUS
    if (strlen(h) < CKMAXPATH)
      sprintf(hompthbuf,"%s>",h);	/* SAFE */
    return(hompthbuf);
#else
    return(h);
#endif /* STRATUS */
#endif /* UNIXOROSK */
}

/*  D O L O G  --  Do the log command  */

int
dolog(x) int x; {
    int y, disp; char *s = NULL, * p = NULL, * q = NULL;
    extern int isguest;
#ifdef ZFNQFP
    struct zfnfp * fnp;
#endif /* ZFNQFP */

    if (isguest) {
        printf("?Anonymous log creation not allowed\n");
        return(-9);
    }
    switch (x) {                        /* Which log... */

#ifdef DEBUG
      case LOGD:
        q = "debug.log";
        y = cmofi("Name of debugging log file",q,&s,xxstring);
        break;
#endif /* DEBUG */

      case LOGP:
        q = "packet.log";
        y = cmofi("Name of packet log file",q,&s,xxstring);
        break;

#ifndef NOLOCAL
      case LOGS:
        q = "session.log";
        y = cmofi("Name of session log file",q,&s,xxstring);
        break;
#endif /* NOLOCAL */

#ifdef TLOG
      case LOGT:
        q = "transact.log";
        y = cmofi("Name of transaction log file",q,&s,xxstring);
        break;
#endif /* TLOG */

#ifdef CKLOGDIAL
      case LOGM: {
          int m, n;
          char mypath[CKMAXPATH+1];
          q = CXLOGFILE;
          m = ckstrncpy(mypath,homepath(),CKMAXPATH);
          n = strlen(CXLOGFILE);
          if (m + n < CKMAXPATH)
            ckstrncat(mypath,CXLOGFILE,CKMAXPATH);
          else
            ckstrncpy(mypath,CXLOGFILE,CKMAXPATH);
          y = cmofi("Name of connection log file",mypath,&s,xxstring);
          break;
      }
#endif /* CKLOGDIAL */

      default:
        printf("\n?Unknown log designator - %d\n",x);
        return(-2);
    }
    if (y < 0) return(y);
    if (y == 2) {                       /* If they gave a directory name */
        int k;
        char * ds = "/";
        k = strlen(s);
        if (k > 0 && s[k-1] == '/') ds = "";
        ckmakmsg(tmpbuf,TMPBUFSIZ,s,ds,q,NULL);
        s = tmpbuf;
    }
#ifdef ZFNQFP
#ifdef OS2ORUNIX
    if (*s != '|')                      /* Allow for pipes */
#else
#ifdef OSK
    if (*s != '|')
#endif /* OSK */
#endif /* OS2ORUNIX */
      if ((fnp = zfnqfp(s,TMPBUFSIZ - 1,tmpbuf))) {
          if (fnp->fpath)
            if ((int) strlen(fnp->fpath) > 0)
              s = fnp->fpath;
      } /* else if error keep original string */
#endif /* ZFNQFP */

    ckstrncpy(line,s,LINBUFSIZ);
    s = line;
#ifdef MAC
    y = 0;
#else

    p = "new";
#ifdef TLOG
    if ((x == LOGT && tlogfmt == 2) || x == LOGM)
      p = "append";
#endif /* TLOG */

    if ((y = cmkey(disptb,2,"Disposition",p,xxstring)) < 0)
      return(y);
#endif /* MAC */
    disp = y;
    if ((y = cmcfm()) < 0) return(y);

    switch (x) {

#ifdef DEBUG
      case LOGD:
        return(deblog = debopn(s,disp));
#endif /* DEBUG */

#ifndef NOXFER
      case LOGP:
        return(pktlog = pktopn(s,disp));
#endif /* NOXFER */

#ifndef NOLOCAL
      case LOGS:
        setseslog(sesopn(s,disp));
        return(seslog);
#endif /* NOLOCAL */

#ifdef TLOG
      case LOGT:
        return(tralog = traopn(s,disp));
#endif /* TLOG */

#ifdef CKLOGDIAL
      case LOGM:
        return(dialog = diaopn(s,disp,0));
#endif /* CKLOGDIAL */

      default:
        return(-2);
    }
}

#ifndef NOXFER
int
pktopn(s,disp) char *s; int disp; {
    static struct filinfo xx;

    if (!s)
      s = "";
    if (!*s)
      return(0);

    debug(F111,"pktopn",s,disp);

    zclose(ZPFILE);

#ifdef OS2ORUNIX
    if (s[0] == '|') {                  /* Pipe */
        char * p = s + 1;
        debug(F110,"pktopn p",p,0);
        while (*p) {
            if (*p != ' ')
              break;
            else
              p++;
        }
        debug(F110,"pktopn pipe",p,0);
        pktlog = zxcmd(ZPFILE,p);
        debug(F101,"pktopn seslog","",seslog);
    } else {                            /* File */
#endif /* OS2ORUNIX */
        if (disp) {
            xx.bs = 0; xx.cs = 0; xx.rl = 0; xx.org = 0; xx.cc = 0;
            xx.typ = 0; xx.dsp = XYFZ_A; xx.os_specific = "";
            xx.lblopts = 0;
            pktlog = zopeno(ZPFILE,s,NULL,&xx);
        } else pktlog = zopeno(ZPFILE,s,NULL,NULL);
        if (!pktlog && !quiet)
          printf("?%s - %s\n",s,ck_errstr());
#ifdef OS2ORUNIX
    }
#endif /* OS2ORUNIX */
    if (pktlog > 0)
      ckstrncpy(pktfil,s,CKMAXPATH+1);
    else
      *pktfil = '\0';
    return(pktlog);
}
#endif /* NOXFER */

int
traopn(s,disp) char *s; int disp; {
#ifdef TLOG
    static struct filinfo xx;

    if (!s)
      s = "";
    if (!*s)
      return(0);

    debug(F111,"traopn",s,disp);
    debug(F101,"traopn tlogfmt","",tlogfmt);

    zclose(ZTFILE);

#ifdef OS2ORUNIX
    if (tlogfmt == 2) {                 /* FTP format is special... */
        VOID doiklog();
        if (!disp)                      /* Append? */
          if (zchki(s) > -1)            /* No - does file exist? */
            (VOID) zdelet(s);           /* Yes - delete it. */
        xferlog = 1;
        ckstrncpy(trafil,s,CKMAXPATH);
        makestr(&xferfile,s);
        doiklog();
        return(1);
    }
    if (s[0] == '|') {                  /* Pipe */
        char * p = s + 1;
        debug(F110,"traopn p",p,0);
        while (*p) {
            if (*p != ' ')
              break;
            else
              p++;
        }
        debug(F110,"traopn pipe",p,0);
        tralog = zxcmd(ZTFILE,p);
        debug(F101,"traopn tralog","",tralog);
    }
#endif /* OS2ORUNIX */

    if (s[0] != '|') {                  /* File */
        if (disp) {
            xx.bs = 0; xx.cs = 0; xx.rl = 0; xx.org = 0; xx.cc = 0;
            xx.typ = 0; xx.dsp = XYFZ_A; xx.os_specific = "";
            xx.lblopts = 0;
            tralog = zopeno(ZTFILE,s,NULL,&xx);
        } else tralog = zopeno(ZTFILE,s,NULL,NULL);
    }
    if (!tralog && !quiet)
      printf("?%s - %s\n",s,ck_errstr());
    if (tralog > 0 && tlogfmt > 0) {
        ckstrncpy(trafil,s,CKMAXPATH);
        tlog(F110,"Transaction Log:",versio,0L);
#ifndef MAC
        tlog(F100,ckxsys,"",0L);
#endif /* MAC */
        ztime(&s);
        tlog(F100,s,"",0L);
    } else
      *trafil = '\0';
    return(tralog);
#else
    return(0);
#endif /* TLOG */
}

#ifndef NOLOCAL
int
sesopn(s,disp) char * s; int disp; {
    static struct filinfo xx;
    extern int tsstate;

    tsstate = 0;                        /* Session log timestamp state */

    if (!s)
      s = "";
    if (!*s)
      return(0);

    debug(F111,"sesopn",s,disp);

    zclose(ZSFILE);

#ifdef OS2ORUNIX
    if (s[0] == '|') {                  /* Pipe */
        char * p = s + 1;
        debug(F110,"sesopn p",p,0);
        while (*p) {
            if (*p != ' ')
              break;
            else
              p++;
        }
        debug(F110,"sesopn pipe",p,0);
        setseslog(zxcmd(ZSFILE,p));
        debug(F101,"sesopn seslog","",seslog);
    } else {                            /* File */
#endif /* OS2ORUNIX */
        if (disp) {
            xx.bs = 0; xx.cs = 0; xx.rl = 0; xx.org = 0; xx.cc = 0;
            xx.typ = 0; xx.dsp = XYFZ_A; xx.os_specific = "";
            xx.lblopts = 0;
            setseslog(zopeno(ZSFILE,s,NULL,&xx));
        } else
          setseslog(zopeno(ZSFILE,s,NULL,NULL));
        if (!seslog && !quiet)
          printf("?%s - %s\n",s,ck_errstr());
#ifdef OS2ORUNIX
    }
#endif /* OS2ORUNIX */
    if (seslog > 0)
      ckstrncpy(sesfil,s,CKMAXPATH+1);
    else
      *sesfil = '\0';
    return(seslog);
}
#endif /* NOLOCAL */
#endif /* NOICP */

int
debopn(s,disp) char *s; int disp; {
#ifdef DEBUG
#ifdef CK_UTSNAME
    extern char unm_mch[], unm_nam[], unm_rel[], unm_ver[], unm_mod[];
#endif /* CK_UTSNAME */
#ifdef OS2
    extern char ckxsystem[];
#endif /* OS2 */
    char *tp;
    static struct filinfo xx;

    if (!s)
      s = "";
    if (!*s)
      return(0);

    zclose(ZDFILE);

#ifdef OS2ORUNIX
    if (s[0] == '|') {                  /* Pipe */
        char * p = s + 1;
        debug(F110,"debopn p",p,0);
        while (*p) {
            if (*p != ' ')
              break;
            else
              p++;
        }
        debug(F110,"debopn pipe",p,0);
        deblog = zxcmd(ZDFILE,p);
        debug(F101,"debopn deblog","",deblog);
    } else {                            /* File */
#endif /* OS2ORUNIX */
        if (disp) {
            xx.bs = 0; xx.cs = 0; xx.rl = 0; xx.org = 0; xx.cc = 0;
            xx.typ = 0; xx.dsp = XYFZ_A; xx.os_specific = "";
            xx.lblopts = 0;
            deblog = zopeno(ZDFILE,s,NULL,&xx);
        } else
          deblog = zopeno(ZDFILE,s,NULL,NULL);
        if (!deblog && !quiet)
          printf("?%s - %s\n",s,ck_errstr());
#ifdef OS2ORUNIX
    }
#endif /* OS2ORUNIX */
    if (deblog > 0) {
        ckstrncpy(debfil,s,CKMAXPATH+1);
        debug(F110,"Debug Log ",versio,0);
#ifndef MAC
#ifdef OS2
        debug(F110,ckxsys,ckxsystem,0);
#else /* OS2 */
        debug(F100,ckxsys,"",0);
#endif /* OS2 */
#endif /* MAC */
#ifdef CK_UTSNAME
        if (unm_mch[0]) {
            debug(F110,"uname machine",unm_mch,0);
            if (unm_mod[0])
              debug(F110,"uname model  ",unm_mod,0);
            debug(F110,"uname sysname",unm_nam,0);
            debug(F110,"uname release",unm_rel,0);
            debug(F110,"uname version",unm_ver,0);
        }
#ifdef KTARGET
        {
            char * s;                   /* Makefile target */
            s = KTARGET;
            if (!s) s = "";
            if (!*s) s = "(unknown)";
            debug(F110,"build target",s,0);
        }
#endif /* KTARGET */
        deblog = 0;
        ztime(&tp);
        deblog = 1;
        debug(F100,tp,"",0);
#endif /* UTSNAME */
        debug(F101,"byteorder","",byteorder);
#ifndef NOICP
#ifndef NOLOCAL
        if (local) {
            debug(F110,"Active connection: ",ttname,0);
            if (!network) {
                debug(F101,"Speed","",speed);
                if (hwparity)
                  debug(F110,"Parity[hardware]",parnam((char)hwparity),0);
                else
                  debug(F110,"Parity",parnam((char)parity),0);
                deblog = 0;
                debug(F110,"Modem",gmdmtyp(),0);
                deblog = 1;
            }
        } else {
            debug(F110,"Active connection: ","none",0);
        }
#endif /* NOLOCAL */
#endif /* NOICP */
    } else *debfil = '\0';
    return(deblog);
#else
    return(0);
#endif /* MAC */
}


/*  C K D A T E  --  Returns current date/time in standard format  */

static char nowbuf[18];

char *
ckdate() {
    extern struct keytab cmonths[];
    int x;
    char * t;                   /* Substitute today's date */
    char dbuf[32];
    ztime(&t);

/*  012345678901234567890123 */
/*  Sat Jul  4 12:16:43 1998 */

    ckstrncpy(dbuf,t,32);
    t = dbuf;
    debug(F110,"ckdate dbuf",dbuf,0);
    nowbuf[0] = t[20];
    nowbuf[1] = t[21];
    nowbuf[2] = t[22];
    nowbuf[3] = t[23];

    nowbuf[4] = NUL;
    debug(F110,"ckdate nowbuf",nowbuf,0);

    t[7] = NUL;
    if ((x = lookup(cmonths,t+4,12,NULL)) < 0) {
        debug(F110,"ckdate bad month",t,0);
        return("<BAD_MONTH>");
    }
    sprintf(nowbuf+4,"%02d",x);         /* SAFE */
    nowbuf[6] = (t[8] == SP) ? '0' : t[8];
    nowbuf[7] = t[9];
    nowbuf[8] = ' ';

    nowbuf[9] = NUL;
    debug(F110,"ckdate nowbuf",nowbuf,0);

    for (x = 11; x < 19; x++) nowbuf[x-2] = t[x];
    nowbuf[17] = NUL;
    debug(F110,"ckdate nowbuf",nowbuf,0);

    return((char *)nowbuf);
}

#ifndef NOICP
#ifdef CKLOGDIAL

/*
  fc = 0 for initial open, meaning open, then close immediately.
  fc > 0 for subsequent opens, meaning open for use, leave open.
*/
int
diaopn(s,disp,fc) char *s; int disp, fc; {
    static struct filinfo xx;

    if (!s)
      s = "";
    if (!*s)
      return(0);

    debug(F110,"diaopn log",s,0);
    debug(F101,"diaopn fc",s,fc);
    debug(F101,"diaopn disp 1",s,disp);
    if (fc) disp = 1;                   /* Force append if open for use */
    debug(F101,"diaopn disp 2",s,disp);

    zclose(ZDIFIL);                     /* In case a log was already open */

#ifdef OS2ORUNIX
    if (s[0] == '|') {                  /* Pipe */
        char * p = s + 1;
        debug(F110,"diaopn p",p,0);
        while (*p) {
            if (*p != ' ')
              break;
            else
              p++;
        }
        debug(F110,"diaopn pipe",p,0);
        dialog = zxcmd(ZDIFIL,p);
        debug(F101,"diaopn dialog","",dialog);
    } else {                            /* File */
#endif /* OS2ORUNIX */
        if (disp) {
            xx.bs = 0; xx.cs = 0; xx.rl = 0; xx.org = 0; xx.cc = 0;
            xx.typ = 0; xx.dsp = XYFZ_A; xx.os_specific = "";
            xx.lblopts = 0;
            dialog = zopeno(ZDIFIL,s,NULL,&xx);
        } else dialog = zopeno(ZDIFIL,s,NULL,NULL);
        if (!dialog)
          printf("?%s - %s\n",s,ck_errstr());
#ifdef OS2ORUNIX
    }
#endif /* OS2ORUNIX */
    if (dialog > 0)
      ckstrncpy(diafil,s,CKMAXPATH+1);
    else
      *diafil = '\0';
    if (fc == 0)                        /* Initial open */
      zclose(ZDIFIL);                   /* close it */
    return(dialog);
}
#endif /* CKLOGDIAL */

#ifndef NOSHOW

/*  SHOW command routines */

char *
shoxm() {
    char * s;
    switch (binary) {
      case XYFT_T: s = "text";         break;
#ifdef VMS
      case XYFT_B: s = "binary fixed"; break;
      case XYFT_I: s = "image";        break;
      case XYFT_L: s = "labeled";      break;
      case XYFT_U: s = "binary undef"; break;
#else
#ifdef MAC
      case XYFT_B: s = "binary";       break;
      case XYFT_M: s = "macbinary";    break;
#else
      case XYFT_B: s = "binary";       break;
#ifdef CK_LABELED
      case XYFT_L: s = "labeled";      break;
#endif /* CK_LABELED */
#endif /* MAC */
#endif /* VMS */
      default: s = "unknown"; break;
    }
    return(s);
}

#ifndef NOXFER
VOID                                    /* SHOW TRANSFER */
shoxfer() {
    extern int docrc, usepipes, xfrxla, whereflg;
    extern char * xfrmsg;
    printf("\n");
    printf(" Transfer Bell: %s\n",showoff(xfrbel));
    printf(" Transfer Interruption: %s\n",showoff(xfrint));
    printf(" Transfer Cancellation: %s\n",showoff(xfrcan));
#ifndef NOCSETS
    printf(" Transfer Translation:  %s\n",showoff(xfrxla));
    printf(" Transfer Character-set: ");
    if (tcharset == TC_TRANSP)
      printf("Transparent\n");
    else
      printf("%s\n",tcsinfo[tcharset].keyword);
#endif /* NOCSETS */
    printf(" Transfer CRC-calculation: %s\n",showoff(docrc));
    printf(" Transfer Display: ");
    switch (fdispla) {
      case XYFD_N: printf("%s\n","none"); break;
      case XYFD_R: printf("%s\n","serial"); break;
      case XYFD_C: printf("%s\n","fullscreen"); break;
      case XYFD_S: printf("%s\n","crt"); break;
      case XYFD_B: printf("%s\n","brief"); break;
      case XYFD_G: printf("%s\n","gui"); break;
    }
    printf(" Transfer Message: %s\n", xfrmsg ? xfrmsg : "(none)");
    printf(" Transfer Locking-shift: ");
    if (lscapu == 2) {
        printf("forced");
    } else {
        printf("%s", (lscapr ? "enabled" : "disabled"));
        if (lscapr) printf(",%s%s", (lscapu ? " " : " not "), "used");
    }
    printf("\n Transfer Mode: %s\n",
           xfermode == XMODE_A ?
           "automatic" :
           "manual"
           );
    printf(" Transfer Pipes: %s\n", showoff(usepipes));
    printf(" Transfer Protocol: %s\n",ptab[protocol].p_name);
    printf(" Transfer Report: %s\n",showoff(whereflg));
    printf(" Transfer Slow-start: %s\n",showoff(slostart));
    printf("\n");
}
#endif /* NOXFER */

VOID
shoflow() {
    int i, x;
    extern int cxflow[], cxtype, ncxname, nfloname, autoflow;
    extern char * cxname[];
    printf("\nConnection type:        %s\n",cxname[cxtype]);
    if (autoflow) {
        printf("Current flow-control:   %s\n", floname[cxflow[cxtype]]);
        printf("Switches automatically: yes\n");
    } else {
        printf("Current flow-control:   %s\n", floname[flow]);
        printf("Switches automatically: no\n");
    }
    printf("\nDefaults by connection type:\n");
    debug(F111,"shoflow cxtype",cxname[cxtype],cxtype);
    debug(F101,"shoflow flow","",flow);
    for (i = 0; i < ncxname; i++) {
#ifdef NOLOCAL
        if (i > 0) break;
#endif /* NOLOCAL */
#ifndef NETCONN
        if (i > 2) break;
#endif /* NETCONN */
#ifndef DECNET
        if (i == CXT_DECNET) continue;
#endif /* DECNET */
#ifndef DECNET
#ifndef SUPERLAT
        if (i == CXT_LAT) continue;
#endif /* SUPERLAT */
#endif /* DECNET */
#ifndef CK_NETBIOS
        if (i == CXT_NETBIOS) continue;
#endif /* CK_NETBIOS */
#ifndef NPIPE
        if (i == CXT_NPIPE) continue;
#endif /* NPIPE */
#ifndef NETCMD
        if (i == CXT_PIPE) continue;
#endif /* NETCMD */
#ifndef ANYX25
        if (i == CXT_X25) continue;
#endif /* ANYX25 */
        x = cxflow[i];
        debug(F101,"shoflow x","",x);
        if (x < nfloname)
          printf("  %-14s: %s\n",cxname[i],floname[x]);
        else
          printf("  %-14s: (%d)\n",cxname[i],x);
    }
    printf("\n");
}

#ifndef NOLOCAL
#ifdef ANYX25
int
shox25(n) int n; {
    if (nettype == NET_SX25) {
        printf("SunLink X.25 V%d.%d",x25ver / 10,x25ver % 10);
        if (ttnproto == NP_X3) printf(", PAD X.3, X.28, X.29 protocol,");
        printf("\n");
        if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
        printf(" Reverse charge call %s",
               revcall ? "selected" : "not selected");
        printf (", Closed user group ");
        if (closgr > -1)
          printf ("%d",closgr);
        else
          printf ("not selected");
        printf("\n");
        if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
        printf(" Call user data %s.\n", cudata ? udata : "not selected");
        if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
    } else if (nettype == NET_VX25) {
        if (ttnproto == NP_X3) printf(", PAD X.3, X.28, X.29 protocol,");
        printf("\n");
        if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
        printf(" Reverse charge call %s",
               revcall ? "selected" : "not selected");
        printf (", Closed user group [unsupported]");
        if (closgr > -1)
          printf ("%d",closgr);
        else
          printf ("not selected");
        printf (",");
        printf("\n");
        if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
        printf(" Call user data %s.\n", cudata ? udata : "not selected");
        if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
    } else if (nettype == NET_IX25) {
        printf("AIX NPI X.25\n");
        if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
        printf("\n Reverse charge call %s",
               revcall ? "selected" : "not selected");
        printf (", Closed user group [unsupported]");
        if (closgr > -1)
          printf ("%d",closgr);
        else
          printf ("not selected");
        printf (",");
        printf("\n Call user data %s.\n", cudata ? udata : "not selected");
    }
    return(n);
}

#ifndef IBMX25
int
shopad(n) int n; {
    int i;
    printf("\nX.3 PAD Parameters:\n");
    if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
    for (i = 0; i < npadx3; i++) {
        printf(" [%d] %s %d\n",padx3tab[i].kwval,padx3tab[i].kwd,
               padparms[padx3tab[i].kwval]);
        if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
    }
    return(n);
}
#endif /* IBMX25 */
#endif /* ANYX25 */

VOID
shoparc() {
    extern int reliable, stopbits, clsondisc;
    int i; char *s;
    long zz;

#ifdef NEWFTP
    if (ftpisconnected()) {
        shoftp(1);
        printf("\n");
    }
#endif /* NEWFTP */

    printf("Communications Parameters:\n");

    if (network
#ifdef IKSD
         || inserver
#endif /* IKSD */
         ) {
        printf(" Network Host: %s%s",ttname,
               (reliable == SET_ON || (reliable == SET_AUTO && !local)
#ifdef TN_COMPORT
               && !istncomport()
#endif /* TN_COMPORT */
#ifdef IKSD
               || inserver
#endif /* IKSD */
               ) ? " (reliable)" : "");
#ifdef TN_COMPORT
        if (istncomport()) {
            int modemstate;
            char * oflow, * iflow = "", * parity, * stopsize, * signature;
            int baud = tnc_get_baud();

            switch (tnc_get_oflow()) {
              case TNC_CTL_OFLOW_NONE:
                oflow = "none";
                break;
              case TNC_CTL_OFLOW_XON_XOFF:
                oflow = "xon/xoff";
                break;
              case TNC_CTL_OFLOW_RTS_CTS:
                oflow = "rts/cts";
                break;
              case TNC_CTL_OFLOW_DCD:
                oflow = "dcd";
                break;
              case TNC_CTL_OFLOW_DSR:
                oflow = "dsr";
                break;
              default:
                oflow = "(unknown)";
            }
            switch (tnc_get_iflow()) {
              case TNC_CTL_IFLOW_NONE:
                iflow = "none";
                break;
              case TNC_CTL_IFLOW_XON_XOFF:
                iflow = "xon/xoff";
                break;
              case TNC_CTL_IFLOW_RTS_CTS:
                iflow = "rts/cts";
                break;
              case TNC_CTL_IFLOW_DTR:
                break;
              default:
                iflow = oflow;
            }
            switch (tnc_get_parity()) {
              case TNC_PAR_NONE:
                parity = "none";
                break;
              case TNC_PAR_ODD:
                parity = "odd";
                break;
              case TNC_PAR_EVEN:
                parity = "even";
                break;
              case TNC_PAR_MARK:
                parity = "mark";
                break;
              case TNC_PAR_SPACE:
                parity = "space";
                break;
              default:
                parity = "(unknown)";
            }
            switch (tnc_get_stopsize()) {
              case TNC_SB_1:
                stopsize = "1";
                break;
              case TNC_SB_1_5:
                stopsize = "1.5";
                break;
              case TNC_SB_2:
                stopsize = "2";
                break;
              default:
                stopsize = "(unknown)";
            }
	    signature = (char *)tnc_get_signature();
            printf("\n  Signature            : %s\n",signature?signature:"");
            if (baud <= 0)
              printf("  Speed                : (unknown)\n");
            else
              printf("  Speed                : %d\n", baud);
            printf("  Outbound Flow Control: %s\n", oflow);
            printf("  Inbound Flow Control : %s\n", iflow);
            printf("  Parity               : %s\n", parity);
            printf("  Data Size            : %d\n", tnc_get_datasize());
            printf("  Stop Bits            : %s\n", stopsize);
            printf("  DTR Signal           : %d\n", tnc_get_dtr_state());
            printf("  RTS Signal           : %d\n", tnc_get_rts_state());
            printf("  Modem State:\n");
            modemstate = tnc_get_ms();
            if (modemstate & TNC_MS_EDGE_RING)
              printf("    Trailing Edge Ring Detector On\n");
            else
              printf("    Trailing Edge Ring Detector Off\n");
            if (modemstate & TNC_MS_CTS_SIG)
              printf("    CTS Signal On\n");
            else
              printf("    CTS Signal Off\n");
            if (modemstate & TNC_MS_DSR_SIG)
              printf("    DSR Signal On\n");
            else
              printf("    DSR Signal Off\n");
            if (modemstate & TNC_MS_RI_SIG)
              printf("    Ring Indicator On\n");
            else
              printf("    Ring Indicator Off\n");
            if (modemstate & TNC_MS_RLSD_SIG)
              printf("    RLSD (CD) Signal On\n");
            else
              printf("    RLSD (CD) Signal Off\n");
            printf("\n");
        }
#endif /* TN_COMPORT */
    } else {

        printf(" %s: %s%s, speed: ",
#ifdef OS2
               "Port",
#else
               "Line",
#endif /* OS2 */
               ttname,
#ifdef CK_TTYFD
               (local &&
#ifdef VMS
                vmsttyfd() < 0
#else
                ttyfd == -1
#endif /* VMS */
                ) ?
                 " (closed)" :
                   (reliable == SET_ON ? " (reliable)" : "")
#else
               ""
#endif /* CK_TTYFD */
               );
        if (
#ifdef CK_TTYFD
#ifdef VMS
            vmsttyfd() < 0
#else
            ttyfd == -1
#endif /* VMS */
            ||
#endif /* CK_TTYFD */
            (zz = ttgspd()) < 0) {
            printf("unknown");
        } else {
            if (speed == 8880) printf("75/1200");
            else if (speed == 134) printf("134.5");
            else printf("%ld",zz);
        }
    }
    if (network
#ifdef IKSD
         || inserver
#endif /* IKSD */
         )
      printf("\n Mode: ");
    else
      printf(", mode: ");
    if (local) printf("local"); else printf("remote");
    if (network == 0
#ifdef IKSD
         && !inserver
#endif/* IKSD */
         ) {
#ifdef CK_TAPI
        if (tttapi && !tapipass )
          printf(", modem: %s","TAPI");
        else
#endif /* CK_TAPI */
        printf(", modem: %s",gmdmtyp());
    } else {
#ifdef NETCONN
       if (nettype == NET_TCPA) printf(", TCP/IP");
       if (nettype == NET_TCPB) printf(", TCP/IP");
       if (nettype == NET_DEC) {
           if (ttnproto == NP_LAT) printf(", DECnet LAT");
           else if ( ttnproto == NP_CTERM ) printf(", DECnet CTERM");
           else printf(", DECnet");
       }
       if (nettype == NET_SLAT) printf(", Meridian Technologies' SuperLAT");
#ifdef NETFILE
       if (nettype == NET_FILE) printf(", local file");
#endif /* NETFILE */
#ifdef NETCMD
       if (nettype == NET_CMD) printf(", pipe");
#endif /* NETCMD */
#ifdef NETPTY
       if (nettype == NET_PTY) printf(", pseudoterminal");
#endif /* NETPTY */
#ifdef NETDLL
       if (nettype == NET_DLL) printf(", dynamic load library");
#endif /* NETDLL */
       if (nettype == NET_PIPE) printf(", Named Pipes");
#ifdef SSHBUILTIN
       if (nettype == NET_SSH)
         printf(", Secure Shell protocol (SECURE)");
#endif /* SSHBUILTIN */
#ifdef ANYX25
       if (shox25(0) < 0) return;
#endif /* ANYX25 */
       if (IS_TELNET()) {
           printf(", telnet protocol");
           if (0
#ifdef CK_ENCRYPTION
               || ck_tn_encrypting() && ck_tn_decrypting()
#endif /* CK_ENCRYPTION */
#ifdef CK_SSL
               || tls_active_flag || ssl_active_flag
#endif /* CK_SSL */
               )
             printf(" (SECURE)");
       }
#ifdef RLOGCODE
       else if (ttnproto == NP_RLOGIN || ttnproto == NP_K4LOGIN ||
                ttnproto == NP_K5LOGIN)
         printf(", rlogin protocol");
       else if (ttnproto == NP_EK4LOGIN || ttnproto == NP_EK5LOGIN)
         printf(", rlogin protocol (SECURE)");
#endif /* RLOGCODE */
#ifdef CK_KERBEROS
#ifdef KRB5
       else if (ttnproto == NP_K5U2U)
         printf(", Kerberos 5 User to User protocol (SECURE)");
#endif /* KRB5 */
#endif /* CK_KERBEROS */
#endif /* NETCONN */
    }
    printf("\n");
    if (hwparity && local && !network)
      s = parnam((char)hwparity);
    else
      s = parnam((char)parity);
    printf(" Parity: %s%s",hwparity ? "hardware " : "", s);
#ifndef NOLOCAL
    if (local && !network) {
        int sb;
        char c;
        c = s[0];
        if (islower(c)) c = toupper(c);
        sb = stopbits;
        if (sb < 1) {
            sb = (speed > 0 && speed <= 110L) ? 2 : 1;
            printf(", stop-bits: (default)");
        } else {
            printf(", stop-bits: %d",sb);
        }
        if (hwparity)
          printf(" (8%c%d)",c,sb);
        else if (parity)
          printf(" (7%c%d)",c,sb);
        else
          printf(" (8N%d)",sb);
        printf("\n D");
    } else
      printf(", d");
#endif /* NOLOCAL */

    printf("uplex: %s, ", duplex ? "half" : "full");
    debug(F101,"shoparp flow","",flow);
    printf("flow: %s", floname[flow]);
    printf(", handshake: ");
    if (turn) printf("%d\n",turnch); else printf("none\n");
#ifdef COMMENT
    if (local && !network) {            /* SET CARRIER-WATCH */
#endif /* COMMENT */
        if (carrier == CAR_OFF) s = "off";
        else if (carrier == CAR_ON) s = "on";
        else if (carrier == CAR_AUT) s = "auto";
        else s = "unknown";
        printf(" Carrier-watch: %s", s);
        if (carrier == CAR_ON) {
            if (cdtimo) printf(", timeout: %d sec", cdtimo);
            else printf(", timeout: none");
        }
#ifdef COMMENT
    }
#endif /* COMMENT */
    printf(", close-on-disconnect: %s\n",showoff(clsondisc));

#ifdef UNIX                             /* UUCP lockfile, UNIX only */
    if (local) {
#ifndef NOUUCP
        if (!network && haslock && *flfnam)
          printf(" Lockfile: %s",flfnam);
#ifndef USETTYLOCK
        if (!network && haslock && lock2[0])
          printf("\n Secondary lockfile: %s",lock2);
#endif /* USETTYLOCK */
#else
#ifdef QNX
        {
            extern int qnxportlock, qnxopencount();
            if (local)
              printf(" Qnx-port-lock: %s, Open count: %d",
                     showoff(qnxportlock),
                     qnxopencount()
                     );
            else
              printf(" Qnx-port-lock: %s", showoff(qnxportlock));
        }
#endif /* QNX */
#endif /* NOUUCP */
        printf("\n");
    } else {
        char * s;
        s = ttglckdir();
        if (!s) s = "";
        printf(" Lockfile directory: %s\n", *s ? s : "(none)");
    }
#endif /* UNIX */
#ifndef MACOSX
    if (!local) {
        printf(" Typical port device name: %s\n",ttgtpn());
    }
#endif	/* MACOSX */
    if (local) {
        int i;
        i = parity ? 7 : 8;
        if (i == 8) i = (cmask == 0177) ? 7 : 8;
        printf(" Terminal bytesize: %d,",i);
        printf(" escape character: %d (^%c)\n",escape,ctl(escape));
    }
}

int
shotcp(n) int n; {
#ifdef TCPSOCKET
    if (nettype == NET_TCPA || nettype == NET_TCPB) {
        printf("SET TCP parameters:\n");
        if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
        printf(" Reverse DNS lookup: %s\n", showooa(tcp_rdns));
        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;

#ifdef CK_DNS_SRV
        printf(" DNS Service Records lookup: %s\n", showooa(tcp_dns_srv));
        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
#endif /* CK_DNS_SRV */

#ifndef NOTCPOPTS
#ifdef SOL_SOCKET
#ifdef SO_KEEPALIVE
        printf(" Keepalive: %s\n", showoff(tcp_keepalive));
        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
#endif /* SO_KEEPALIVE */

#ifdef SO_LINGER
        printf(" Linger: %s", tcp_linger ? "on, " : "off\n" );
        if (tcp_linger) {
            if (tcp_linger_tmo)
              printf("%d x 10 milliseconds\n",tcp_linger_tmo);
            else
              printf("no timeout\n");
        }
        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
#endif /* SO_LINGER */

#ifdef SO_DONTROUTE
        printf(" DontRoute: %s\n", tcp_dontroute ? "on" : "off" );
        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
#endif /* SO_DONTROUTE */

#ifdef TCP_NODELAY
        printf(" Nodelay: %s\n", showoff(tcp_nodelay));
        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
#endif /* TCP_NODELAY */

#ifdef SO_SNDBUF
        if (tcp_sendbuf <= 0)
          printf(" Send buffer: (default size)\n");
        else
          printf(" Send buffer: %d bytes\n", tcp_sendbuf);
        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
#endif /* SO_SNDBUF */
#ifdef SO_RCVBUF
        if (tcp_recvbuf <= 0)
          printf(" Receive buffer: (default size)\n");
        else
          printf(" Receive buffer: %d bytes\n", tcp_recvbuf);
        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
#endif /* SO_RCVBUF */
#endif /* SOL_SOCKET */
#endif /* NOTCPOPTS */
        printf(" address: %s\n",tcp_address ? tcp_address : "(none)");
        if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
#ifndef NOHTTP
        printf(" http-proxy: %s\n",tcp_http_proxy ? tcp_http_proxy : "(none)");
        if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
#endif /* NOHTTP */
#ifdef NT
#ifdef CK_SOCKS
        printf(" socks-server: %s\n",tcp_socks_svr ? tcp_socks_svr : "(none)");
        if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
#ifdef CK_SOCKS_NS
        printf(" socks-name-server: %s\n",
               tcp_socks_ns ? tcp_socks_ns : "(none)");
        if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
#endif /* CK_SOCKS_NS */
#endif /* CK_SOCKS */
#endif /* NT */
    }
#endif /* TCPSOCKET */
    return(n);
}

#ifdef TNCODE
int
shotopt(n) int n; {
    int opt;

    printf("%-21s %12s %12s %12s %12s\n\n",
           "Telnet Option","Me (client)","U (client)",
           "Me (server)","U (server)");
    n += 2;
    if (n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;

    for ( opt = TELOPT_FIRST; opt <= TELOPT_LAST; opt++) {
        switch (opt) {
          case TELOPT_AUTHENTICATION:
          case TELOPT_ENCRYPTION:
          case TELOPT_TTYPE:
          case TELOPT_NAWS:
          case TELOPT_BINARY:
          case TELOPT_NEWENVIRON:
          case TELOPT_SNDLOC:
          case TELOPT_XDISPLOC:
          case TELOPT_SGA:
          case TELOPT_ECHO:
          case TELOPT_KERMIT:
          case TELOPT_START_TLS:
          case TELOPT_FORWARD_X:
          case TELOPT_COMPORT:
            break;
          default:
            continue;
        }
        printf("%03d %-17s ",
               opt, TELOPT(opt)
               );
        printf("%12s %12s ",
               TELOPT_MODE(TELOPT_DEF_C_ME_MODE(opt)),
               TELOPT_MODE(TELOPT_DEF_C_U_MODE(opt))
               );
        printf("%12s %12s\n",
               TELOPT_MODE(TELOPT_DEF_S_ME_MODE(opt)),
               TELOPT_MODE(TELOPT_DEF_S_U_MODE(opt))
               );

        if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
        if (sstelnet)
          printf("%21s %12s %12s %12s %12s\n",
                 "",
                 "",
                 "",
                 (TELOPT_ME(opt)?"WILL":"WONT"),
                 (TELOPT_U(opt)?"DO":"DONT")
                 );
        else
          printf("%21s %12s %12s %12s %12s\n",
                 "",
                 (TELOPT_ME(opt)?"WILL":"WONT"),
                 (TELOPT_U(opt)?"DO":"DONT"),
                 "",
                 ""
                 );
        if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
    }
    return(n);
}

int
shotel(n) int n; {
    extern int tn_duplex;
#ifdef CK_ENVIRONMENT
    extern int tn_env_flg;
    extern char tn_env_acct[];
    extern char tn_env_job[];
    extern char tn_env_prnt[];
    extern char tn_env_sys[];
    extern char * tn_env_uservar[8][2];
    int x;
#endif /* CK_ENVIRONMENT */
#ifdef CK_SNDLOC
    extern char * tn_loc;
#endif /* CK_SNDLOC */
    printf("SET TELNET parameters:\n echo: %s\n NVT newline-mode: ",
           tn_duplex ? "local" : "remote");
    switch (tn_nlm) {
      case TNL_CRNUL: printf("%s\n","off (cr-nul)"); break;
      case TNL_CRLF:  printf("%s\n","on (cr-lf)"); break;
      case TNL_CR:    printf("%s\n","raw (cr)"); break;
      case TNL_LF:    printf("%s\n","(lf)"); break;
    }
    if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
#ifdef CK_AUTHENTICATION
    {
        int type = ck_tn_authenticated();
        printf(" authentication: ");
        switch (sstelnet ?
                TELOPT_U_MODE(TELOPT_AUTHENTICATION) :
                 TELOPT_ME_MODE(TELOPT_AUTHENTICATION)
                ) {
          case TN_NG_AC: printf( "accepted " ); break;
          case TN_NG_RF: printf( "refused  " ); break;
          case TN_NG_RQ: printf( "requested"); break;
          case TN_NG_MU: printf( "required "); break;
        }

#ifdef CK_SSL
        if ((ssl_active_flag || tls_active_flag) &&
             ck_tn_auth_valid() == AUTH_VALID &&
             (!TELOPT_U(TELOPT_AUTHENTICATION) ||
               type == AUTHTYPE_NULL ||
               type == AUTHTYPE_AUTO))
            printf("   in use: X.509 certificate\n");
        else
#endif /* CK_SSL */
          printf("   in use: %s\n",AUTHTYPE_NAME(type));
        if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
        if (forward_flag)
          printf("  credentials forwarding requested %s\n",
                 forwarded_tickets ? "and completed" :
                 "but not completed");
        else
          printf("  credentials forwarding disabled\n");
        if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
    }
#endif /* CK_AUTHENTICATION */
#ifdef CK_ENCRYPTION
    {
        int i,x;
        int e_type = ck_tn_encrypting();
        int d_type = ck_tn_decrypting();
        char * e_str = NULL, * d_str = NULL;
        static struct keytab * tnetbl = NULL;
        static int ntnetbl = 0;

        x = ck_get_crypt_table(&tnetbl,&ntnetbl);

        for (i = 0; i < ntnetbl; i++) {
            if (e_type == tnetbl[i].kwval)
              e_str = tnetbl[i].kwd;
            if (d_type == tnetbl[i].kwval)
              d_str = tnetbl[i].kwd;
        }
        printf(" encryption: ");
        switch (TELOPT_ME_MODE(TELOPT_ENCRYPTION)) {
          /* This should be changed to report both ME and U modes */
          case TN_NG_AC: printf( "accepted " ); break;
          case TN_NG_RF: printf( "refused  " ); break;
          case TN_NG_RQ: printf( "requested"); break;
          case TN_NG_MU: printf( "required "); break;
        }
        printf("       in use: ");
        switch ((e_type ? 1 : 0) | (d_type ? 2 : 0)) {
          case 0:
            printf("plain text in both directions");
            break;
          case 1:
            printf("%s output, plain text input",e_str);
            break;
          case 2:
            printf("plain text output, %s input",d_str);
            break;
          case 3:
            printf("%s output, %s input",e_str,d_str);
            break;
        }
        printf("\n");
        if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
    }
#endif /* CK_ENCRYPTION */
#ifdef IKS_OPTION
    printf(" kermit: ");
    switch (TELOPT_U_MODE(TELOPT_KERMIT)) {
      case TN_NG_AC: printf( "u, accepted;  " ); break;
      case TN_NG_RF: printf( "u, refused;   " ); break;
      case TN_NG_RQ: printf( "u, requested; "); break;
      case TN_NG_MU: printf( "u, required;  "); break;
    }
    switch (TELOPT_ME_MODE(TELOPT_KERMIT)) {
      case TN_NG_AC: printf( "me, accepted;  " ); break;
      case TN_NG_RF: printf( "me, refused;   " ); break;
      case TN_NG_RQ: printf( "me, requested; "); break;
      case TN_NG_MU: printf( "me, required;  "); break;
    }
    if (TELOPT_U(TELOPT_KERMIT))
      printf(" u, %s",
             TELOPT_SB(TELOPT_KERMIT).kermit.u_start ?
             "started" :
             "stopped"
             );
    else
      printf(" u, n/a");
    if (TELOPT_ME(TELOPT_KERMIT))
      printf(" me, %s;",
             TELOPT_SB(TELOPT_KERMIT).kermit.me_start ?
             "started" :
             "stopped"
             );
    else
      printf(" me, n/a;");
    printf("\n");
    if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
#endif /* IKS_OPTION */
    printf(" BINARY newline-mode: ");
    switch (tn_b_nlm) {
      case TNL_CRNUL: printf("%s\n","off (cr-nul)"); break;
      case TNL_CRLF:  printf("%s\n","on (cr-lf)"); break;
      case TNL_CR:    printf("%s\n","raw (cr)"); break;
      case TNL_LF:    printf("%s\n","(lf)"); break;
    }
    if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
    printf(" binary-mode: ");
    switch (TELOPT_U_MODE(TELOPT_BINARY)) {
      case TN_NG_AC: printf( "u, accepted;  " ); break;
      case TN_NG_RF: printf( "u, refused;   " ); break;
      case TN_NG_RQ: printf( "u, requested; "); break;
      case TN_NG_MU: printf( "u, required;  "); break;
    }
    switch (TELOPT_ME_MODE(TELOPT_BINARY)) {
      case TN_NG_AC: printf( "me, accepted; " ); break ;
      case TN_NG_RF: printf( "me, refused; " ); break;
      case TN_NG_RQ: printf( "me, requested; "); break;
      case TN_NG_MU: printf( "me, required;  "); break;
    }
    printf("u, %s; me, %s\n",
           TELOPT_U(TELOPT_BINARY) ? "BINARY" : "NVT",
           TELOPT_ME(TELOPT_BINARY) ? "BINARY" : "NVT"
           );
    if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
    printf(" binary-transfer-mode: %s\n",showoff(tn_b_xfer));
    if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
    printf(" bug binary-me-means-u-too: %s\n",showoff(tn_b_meu));
    if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
    printf(" bug binary-u-means-me-too: %s\n",showoff(tn_b_ume));
    if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
    printf(" bug sb-implies-will-do: %s\n",showoff(tn_sb_bug));
    if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
    printf(" bug auth-krb5-des: %s\n",showoff(tn_auth_krb5_des_bug));
    if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
    printf(" terminal-type: ");
    if (tn_term) {
        printf("%s\n",tn_term);
    } else {
        char *p;
#ifdef OS2
        p = (tt_type >= 0 && tt_type <= max_tt) ?
          tt_info[tt_type].x_name :
            "UNKNOWN";
#else
        p = getenv("TERM");
#endif /* OS2 */
        if (p)
          printf("none (%s will be used)\n",p);
        else printf("none\n");
    }
    if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
#ifdef CK_ENVIRONMENT
    printf(" environment: %s\n", showoff(tn_env_flg));
    if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
    printf("   ACCOUNT: %s\n",tn_env_acct);
    if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
    printf("   DISPLAY: %s\n",(char *)tn_get_display() ?
            (char *)tn_get_display() : "");
    if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
    printf("   JOB    : %s\n",tn_env_job);
    if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
    printf("   PRINTER: %s\n",tn_env_prnt);
    if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
#ifndef NOSPL
    printf("   USER   : %s\n",uidbuf);
    if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
#endif /* NOSPL */
    printf("   SYSTEM : %s\n",tn_env_sys);
    if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
    for (x = 0; x < 8; x++) {
        if (tn_env_uservar[x][0] && tn_env_uservar[x][1]) {
            printf("   %-7s: %s\n",tn_env_uservar[x][0],
                   tn_env_uservar[x][1]);
            if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
        }
    }
#endif /* CK_ENVIRONMENT */
#ifdef CK_SNDLOC
    printf("  LOCATION: %s\n", tn_loc ? tn_loc : "");
    if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
#endif /* CK_SNDLOC */
#ifdef CK_FORWARD_X
    printf(" .Xauthority-file: %s\n", (char *)XauFileName() ?
            (char *)XauFileName() : "(none)");
    if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
#endif /* CK_FORWARD_X */
    return(n);
}
#endif /* TNCODE */

#ifdef CK_NETBIOS
static int
shonb(n) int n; {
    printf("NETBIOS parameters:\n");
    if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
    printf(" API       : %s\n",
           NetbeuiAPI ?
           "NETAPI.DLL - IBM Extended Services or Novell Netware Requester"
           : "ACSNETB.DLL - IBM Network Transport Services/2" ) ;
    if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
    printf(" Local Name: [%s]\n", NetBiosName);
    if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
    printf(" Adapter   : %d\n", NetBiosAdapter);
    if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
    if (NetBiosLSN > 0xFF) {
        printf(" Session   : %d\n", NetBiosLSN);
    } else {
        printf(" Session   : none active\n");
    }
    if (++n > cmd_rows - 3) if (!askmore()) return(-1); else n = 0;
    return(n);
}
#endif /* CK_NETBIOS */

#ifndef NONET
int
shonet() {

#ifndef NETCONN
    printf("\nNo networks are supported in this version of C-Kermit\n");

#else
#ifdef NOLOCAL
    printf("\nNo networks are supported in this version of C-Kermit\n");

#else /* rest of this routine */

    int i, n = 4;

#ifndef NODIAL
    if (nnetdir <= 1) {
        printf("\nNetwork directory: %s\n",netdir[0] ? netdir[0] : "(none)");
        n++;
    } else {
        int i;
        printf("\nNetwork directories:\n");
        for (i = 0; i < nnetdir; i++) {
            printf("%2d. %s\n",i,netdir[i]);
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
        }
    }
#endif /* NODIAL */

#ifdef SSHCMD
    {
        extern char * sshcmd;
        printf("SSH COMMAND: %s\n",sshcmd ? sshcmd : "ssh -e none");
        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    }
#endif /* SSHCMD */

#ifdef OS2
    printf("\nNetwork availability:\n");
#else
    printf("\nSupported networks:\n");
#endif /* OS2 */
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;

#ifdef VMS

#ifdef TCPWARE
    printf(" Process Software Corporation TCPware for OpenVMS");
#else
#ifdef MULTINET
    printf(" TGV MultiNet TCP/IP");
#else
#ifdef WINTCP
    printf(" WOLLONGONG WIN/TCP");
#else
#ifdef DEC_TCPIP
    {
        static $DESCRIPTOR(tcp_desc,"_TCP0:");
        int status;
        long devclass;
        static int itmcod = DVI$_DEVCLASS;

#ifdef COMMENT
        status = LIB$GETDVI(&itmcod, 0, &tcp_desc, &devclass);
#else
        /* Martin Zinser 9/96 */
        status = lib$getdvi(&itmcod, 0, &tcp_desc, &devclass);
#endif /* COMMENT */
        if ((status & 1) && (devclass == DC$_SCOM))
          printf(" Process Software Corporation TCPware for OpenVMS");
        else
#ifdef UCX50
          printf(" DEC TCP/IP Services for (Open)VMS 5.0");
#else
          printf(" DEC TCP/IP Services for (Open)VMS");
#endif /* UCX50 */
    }
#else
#ifdef CMU_TCPIP
    printf(" CMU-OpenVMS/IP");
#else
    printf(" None");
#endif /* CMU_TCPIP */
#endif /* DEC_TCPIP */
#endif /* WINTCP */
#endif /* MULTINET */
#endif /* TCPWARE */
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
#ifdef TNCODE
    printf(", TELNET protocol\n\n");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    n = shotel(n);
    if (n < 0) return(0);
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
#endif /* TNCODE */
    printf("\n");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    printf("\n");
    n = shotcp(++n);
    if (n < 0) return(0);
#else /* Not VMS */

#ifdef SUNX25
    printf(" SunLink X.25\n");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
#endif /* SUNX25 */

#ifdef STRATUSX25
    printf(" Stratus VOS X.25\n");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
#endif /* STRATUSX25 */

#ifdef IBMX25
    printf(" IBM AIX X.25\n");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
#endif /* IBMX25 */

#ifdef HPX25
    printf(" HP-UX X.25\n");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
#endif /* HPX25 */

#ifdef SSHBUILTIN
    if (ck_ssleay_is_installed())
        printf(" SSH V1 and V2 protocols\n");
    else
        printf(" SSH V1 and V2 protocols - not available\n");
#endif /* SSHBUILTIN */

#ifdef DECNET
#ifdef OS2
#ifdef NT
    if (dnet_avail)
      printf(" DECnet, LAT and CTERM protocols\n");
    else
      printf(" DECnet, LAT and CTERM protocols - not available\n");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
#else /* NT */
    if (dnet_avail)
      printf(" DECnet, LAT protocol\n");
    else
      printf(" DECnet, LAT protocol - not available\n");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
#endif /* NT */
#else
    printf(" DECnet\n");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
#endif /* OS2 */
#endif /* DECNET */

#ifdef NPIPE
    printf(" Named Pipes\n");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
#endif /* NPIPE */

#ifdef CK_NETBIOS
    if (netbiosAvail)
      printf(" NETBIOS\n");
    else
      printf(" NETBIOS - not available\n");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
#endif /* CK_NETBIOS */

#ifdef SUPERLAT
    if (slat_avail)
      printf(" SuperLAT\n");
    else
      printf(" SuperLAT - not available\n") ;
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
#endif /* SUPERLAT */

#ifdef TCPSOCKET
    if (
#ifdef OS2
        tcp_avail
#else
        1
#endif /* OS2 */
        ) {
        char ipaddr[16];

        if (getlocalipaddrs(ipaddr,16,0) < 0) {
#ifdef OS2ONLY
            printf(" TCP/IP via %s\n", tcpname);
#else
            printf(" TCP/IP\n");
#endif /* OS2ONLY */
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
        } else {
            int i = 1;
#ifdef OS2ONLY
          printf(" TCP/IP [%16s] via %s\n", ipaddr, tcpname);
#else
          printf(" TCP/IP [%16s]\n",ipaddr);
#endif /* OS2ONLY */
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;

            while (getlocalipaddrs(ipaddr,16,i++) >= 0) {
                printf("        [%16s]\n",ipaddr);
                if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            }
        }
        if (nettype == NET_TCPB) {
            printf("\n");
            n = shotcp(++n);
            if (n < 0) return(0);
#ifdef TNCODE
            printf("\n");
            n = shotel(++n);
            if (n < 0) return(0);
#endif /* TNCODE */
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
        }
#ifdef OS2
    } else {
        printf(" TCP/IP - not available%s\n",tcpname[0] ? tcpname : "" );
        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
#endif /* OS2 */
    }
#endif /* TCPSOCKET */

#ifdef CK_NETBIOS
    if (netbiosAvail && nettype == NET_BIOS) {
       printf("\n") ;
       if ((n = shonb(++n)) < 0) return(0);
       if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    }
#endif /* CK_NETBIOS */

#endif /* VMS */

    printf("\nActive network connection:\n");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;

    if (network) {
        printf(" Host: %s",ttname);
        if ((nettype == NET_TCPA || nettype == NET_TCPB) && *ipaddr)
          printf(" [%s]",ipaddr);
    } else
      printf(" Host: none");
    printf(", via: ");
    if (nettype == NET_TCPA || nettype == NET_TCPB)
      printf("tcp/ip\n");
    else if (nettype == NET_SX25)
      printf("SunLink X.25\n");
    else if (nettype == NET_VX25)
      printf("Stratus VOS X.25\n");
    else if (nettype == NET_IX25)
      printf("IBM AIX X.25\n");
    else if (nettype == NET_HX25)
      printf("HP-UX X.25\n");
    else if (nettype == NET_DEC) {
        if ( ttnproto == NP_LAT )
          printf("DECnet LAT\n");
        else if ( ttnproto == NP_CTERM )
          printf("DECnet CTERM\n");
        else
          printf("DECnet\n");
    } else if (nettype == NET_PIPE)
      printf("Named Pipes\n");
    else if (nettype == NET_BIOS)
      printf("NetBIOS\n");
    else if (nettype == NET_SLAT)
      printf("SuperLAT\n");

#ifdef NETFILE
    else if ( nettype == NET_FILE )
      printf("local file\n");
#endif /* NETFILE */
#ifdef NETCMD
    else if ( nettype == NET_CMD )
      printf("pipe\n");
#endif /* NETCMD */
#ifdef NETPTY
    else if ( nettype == NET_PTY )
        printf("pseudoterminal\n");
#endif /* NETPTY */
#ifdef NETDLL
    else if ( nettype == NET_DLL )
      printf("dynamic link library\n");
#endif /* NETDLL */
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;

#ifdef ANYX25
    if ((nettype == NET_SX25) ||
        (nettype == NET_VX25) ||
        (nettype == NET_IX25))
      if ((n = shox25(n)) < 0) return(0);
#endif /* ANYX25 */

#ifdef SSHBUILTIN
    if (nettype == NET_SSH) {
        printf("Secure Shell protocol\n");
        if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    }
#endif /* SSHBUILTIN */

    if (nettype == NET_TCPA || nettype == NET_TCPB) {
#ifdef RLOGCODE
        if (ttnproto == NP_RLOGIN) {
            printf(" LOGIN (rlogin) protocol\n");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
        }
#ifdef CK_KERBEROS
        else if (ttnproto == NP_K4LOGIN) {
            printf(" Kerberos 4 LOGIN (klogin) protocol\n");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
        }
        else if (ttnproto == NP_EK4LOGIN) {
            printf(" Encrypted Kerberos 4 LOGIN (eklogin) protocol\n");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
        }
        else if (ttnproto == NP_K5LOGIN) {
            printf(" Kerberos 5 LOGIN (klogin) protocol\n");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
        }
        else if (ttnproto == NP_EK5LOGIN) {
            printf(" Encrypted Kerberos 5 LOGIN (eklogin) protocol\n");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
        }
#endif /* CK_KERBEROS */
#endif /* RLOGCODE */
#ifdef CK_KERBEROS
        if (ttnproto == NP_K5U2U) {
            printf(" Kerberos 5 User to User protocol\n");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
        }
#endif /* CK_KERBEROS */

#ifdef TNCODE
        if (IS_TELNET()) {
            printf(" TELNET protocol\n");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Echoing is currently %s\n",duplex ? "local" : "remote");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
        }
#endif /* TNCODE */
        if (ttnproto == NP_TCPRAW) {
            printf(" Raw TCP socket\n");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
        }
    }
    printf("\n");
#endif /* NOLOCAL */
#endif /* NETCONN */
    return(0);
}
#endif /* NONET */

#ifndef NODIAL
VOID
shodial() {
    if (mdmtyp >= 0 || local != 0) doshodial();
}

VOID
shods(s) char *s; {                     /* Show a dial-related string */
    char c;
    if (s == NULL || !(*s)) {           /* Empty? */
        printf("(none)\n");
    } else {                            /* Not empty. */
        while ((c = *s++))              /* Can contain controls */
          if (c == '\\')                /* a backslash */
            printf("\\\\");
          else if (c > 31 && c < 127) {
              putchar(c);
          } else
            printf("\\{%d}",c);
        printf("\n");
    }
}

int
doshodial() {

    int i, n = 2;

    printf(" Dial status:  %d", dialsta);

#ifdef BIGBUFOK
    if (dialsta > 90)
      printf(" = Unknown error");
    else if (dialsta < 0)
      printf(" = (none)");
    else if (dialsta < 35 && dialmsg[dialsta])
      printf(" = %s", dialmsg[dialsta]);
#endif /* BIGBUFOK */
    n++;
    if (ndialdir <= 1) {
        printf("\n Dial directory: %s\n",dialdir[0] ? dialdir[0] : "(none)");
    } else {
        int i;
        printf("\n Dial directories:\n");
        for (i = 0; i < ndialdir; i++)
          printf("%2d. %s\n",i+1,dialdir[i]);
        n += ndialdir;
    }
    printf(" Dial method:  ");
    if      (dialmauto)         printf("auto   ");
    else if (dialmth == XYDM_D) printf("default");
    else if (dialmth == XYDM_P) printf("pulse  ");
    else if (dialmth == XYDM_T) printf("tone   ");
    printf("         Dial sort: %s\n",dialsrt ? "on" : "off");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    printf(" Dial hangup:  %s             Dial display: %s\n",
           dialhng ? "on " : "off", dialdpy ? "on" : "off");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    if (dialrtr > 0) {
        printf(" Dial retries: %-12d    Dial interval: %d\n",
               dialrtr, dialint);
    } else {
        printf(" Dial retries: (auto)          Dial interval: %d\n", dialint);
    }
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    printf(" Dial timeout: ");
#ifdef CK_TAPI
    if (tttapi && !tapipass)
        printf("(tapi)");
    else
#endif /* CK_TAPI */
    if (dialtmo > 0)
      printf("%4d sec", dialtmo);
    else
      printf("0 (auto)");
    printf("        Redial number: %s\n",dialnum ? dialnum : "(none)");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    printf(" Dial confirmation: %s        Dial convert-directory: %s\n",
           dialcnf ? "on " : "off",
           dialcvt ? ((dialcvt == 1) ? "on" : "ask") : "off");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    printf(" Dial ignore-dialtone: %s", dialidt ? "on " : "off");
    printf("     Dial pacing: %d\n",dialpace);
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    printf(
" Dial prefix:                  %s\n", dialnpr ? dialnpr : "(none)");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    printf(
" Dial suffix:                  %s\n", dialsfx ? dialsfx : "(none)");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    printf(
" Dial country-code:            %-12s", diallcc ? diallcc : "(none)");
    printf("Dial connect:  %s", dialcon ? ((dialcon == 1) ? "on" : "auto")
           : "off");
    if (dialcon != CAR_OFF)
      printf(" %s", dialcq ? "quiet" : "verbose");
    printf(
"\n Dial area-code:               %-12s", diallac ? diallac : "(none)");
    n++;
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    printf("Dial restrict: ");
    if (dialrstr == 5) printf("international\n");
    else if (dialrstr == 4) printf("long-distance\n");
    else if (dialrstr == 2) printf("local\n");
    else if (dialrstr == 6) printf("none\n");
    else printf("?\n");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    printf(" Dial lc-area-codes:           ");
    if (nlocalac == 0)
      printf("(none)");
    else
      for (i = 0; i < nlocalac; i++)
        printf("%s ", diallcac[i]);
    printf(
"\n Dial lc-prefix:               %s\n", diallcp ? diallcp : "(none)");
    n++;
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    printf(
" Dial lc-suffix:               %s\n", diallcs ? diallcs : "(none)");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    printf(
" Dial ld-prefix:               %s\n", dialldp ? dialldp : "(none)");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    printf(
" Dial ld-suffix:               %s\n", diallds ? diallds : "(none)");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    printf(
" Dial force-long-distance      %s\n", showoff(dialfld));
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    printf(
" Dial intl-prefix:             %s\n", dialixp ? dialixp : "(none)");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    printf(
" Dial intl-suffix:             %s\n", dialixs ? dialixs : "(none)");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    printf(
" Dial toll-free-area-code:     ");
    if (ntollfree == 0)
      printf("(none)");
    else
      for (i = 0; i < ntollfree; i++)
        printf("%s ", dialtfc[i]);
    printf("\n");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;

    printf(
" Dial pulse-countries:         ");
    if (ndialpucc == 0)
      printf("(none)");
    else
      for (i = 0; i < ndialpucc; i++)
        printf("%s ", dialpucc[i]);
    printf("\n");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;

    printf(
" Dial tone-countries:          ");
    if (ndialtocc == 0)
      printf("(none)");
    else
      for (i = 0; i < ndialtocc; i++)
        printf("%s ", dialtocc[i]);
    printf("\n");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;

    printf(
	" Dial toll-free-prefix:        %s\n",
	dialtfp ? dialtfp :
	(dialldp ? dialldp : "(none)")
	);
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    printf(" Dial pbx-exchange:            ");
    if (ndialpxx == 0)
      printf("(none)");
    else
      for (i = 0; i < ndialpxx; i++)
        printf("%s ", dialpxx[i]);
    printf("\n");

    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    printf(
" Dial pbx-inside-prefix:       %s\n", dialpxi ? dialpxi : "(none)");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    printf(
" Dial pbx-outside-prefix:      %s\n", dialpxo ? dialpxo : "(none)");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    printf(
" Dial macro:                   %s\n", dialmac ? dialmac : "(none)");
    return(0);
}
#endif /* NODIAL */
#endif /* NOLOCAL */

/*  Show File Parameters */

static char *
pathval(x) int x; {
    switch (x) {
      case PATH_OFF:  return("off");
      case PATH_ABS:  return("absolute");
      case PATH_REL:  return("relative");
      case PATH_AUTO: return("auto");
      default: return("unknown");
    }
}

VOID
shofil() {
    char *s; int i = 0, n = 1;
    extern char * ifdnam[];
    extern int wildena;
#ifdef UNIX
    extern int wildxpand;
#endif /* UNIX */
    extern char * snd_move, * snd_rename, * rcv_move, * rcv_rename;
#ifdef PATTERNS
    extern int patterns;
#endif /* PATTERNS */
    extern char * rfspec, * sfspec;
#ifdef UNIX
    extern int zobufsize, zofbuffer, zofblock;
#endif /* UNIX */
#ifdef CK_CTRLZ
    extern int eofmethod;
#endif /* CK_CTRLZ */

    printf("\n");

#ifdef VMS
    printf(" File record-Length:      %5d\n",frecl);
    n++;
#endif /* VMS */

#ifndef NOXFER
    printf(" Transfer mode:           %s\n",
           xfermode == XMODE_A ?
           "automatic" :
           "manual"
           );
    n++;
#ifdef PATTERNS
    printf(" File patterns:           %s", showooa(patterns));
    if (xfermode == XMODE_M && patterns)
      printf(" (but disabled by TRANSFER-MODE MANUAL)");
    else if (patterns)
      printf(" (SHOW PATTERNS for list)");
    printf("\n");
    n++;
#endif /* PATTERNS */
    if (filepeek)
      printf(" File scan:               on %d\n", nscanfile);
    else
      printf(" File scan:               off\n");
    if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
    if (xfermode == XMODE_A)
      printf(" Default file type:       %s\n",shoxm());
    else
      printf(" File type:               %s\n",shoxm());
    n++;
    if (fncnv == XYFN_L)
      s = "literal";
    else if (fncnv == XYFN_C)
      s = "converted";
    else
      s = "(unknown)";
    printf(" File names:              %s\n",s);
    n++;
    printf(" Send pathnames:          %s\n", pathval(fnspath));
    n++;
    printf(" Receive pathnames:       %s\n", pathval(fnrpath));
    n++;
#ifdef UNIXOROSK
    printf(" Match dot files:         %s\n", matchdot ? "yes" : "no");
    n++;
#ifdef UNIX
    printf(" Wildcard-expansion:      %s (%s)\n", showoff(wildena),
	   wildxpand ? "shell" : "kermit");
    n++;
#endif /* UNIX */
#endif /* UNIXOROSK */
    printf(" File collision:          ");
    for (i = 0; i < ncolx; i++)
      if (colxtab[i].kwval == fncact) break;
    printf("%s\n", (i == ncolx) ? "unknown" : colxtab[i].kwd);
    if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
    printf(" File destination:        %s\n",
           (dest == DEST_D) ? "disk" :
           ((dest == DEST_S) ? "screen" :
            ((dest == DEST_N) ? "nowhere" :
            "printer"))
           );
    if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
    s = (keep >= 0 && keep <= 2) ? ifdnam[keep] : "keep";
    printf(" File incomplete:         %s\n",s);
    if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
    printf(" File bytesize:           %d\n",(fmask == 0177) ? 7 : 8);
    if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
#ifndef NOCSETS
    printf(" File character-set:      %s\n",fcsinfo[fcharset].keyword);
    if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
    printf(" File default 7-bit:      %s\n",fcsinfo[dcset7].keyword);
    if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
    printf(" File default 8-bit:      %s\n",fcsinfo[dcset8].keyword);
    if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
#ifdef UNICODE
    printf(" File UCS bom:            %s\n",showoff(ucsbom));
    if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
    printf(" File UCS byte-order:     %s-endian\n",
           ucsorder ? "little" : "big");
    if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
    printf(" Computer byteorder:      %s-endian\n",
           byteorder ? "little" : "big");
    if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
#endif /* UNICODE */
#endif /* NOCSETS */

    printf(" File end-of-line:        ");
    i = feol;
    switch (feol) {
      case XYFA_C: printf("%s\n","cr"); break;
      case XYFA_L: printf("%s\n","lf"); break;
      case XYFA_2: printf("%s\n","crlf"); break;
      default: printf("%d\n",i);
    }
    if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
#endif /* NOXFER */

#ifdef CK_CTRLZ
    printf(" File eof:                %s\n", eofmethod ? "ctrl-z" : "length");
    if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
#endif /* CK_CTRLZ */
#ifndef NOXFER
#ifdef CK_TMPDIR
    printf(" File download-directory: %s\n", dldir ? dldir : "(none)");
    if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
#ifdef COMMENT
    i = 256;
    s = line;
    zzstring("\\v(tmpdir)",&s,&i);
    printf(" Temporary directory:     %s\n", line);
    if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
#endif /* COMMENT */
#endif /* CK_TMPDIR */
#ifdef VMS
    {
        extern int vmssversions, vmsrversions;
        printf(" Send version-numbers:    %s\n",showoff(vmssversions));
        if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
        printf(" Receive version-numbers: %s\n",showoff(vmsrversions));
        if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
    }
#endif /* VMS */
    printf(" Send move-to:            %s\n",
           snd_move ? snd_move : "(none)");
    if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
    printf(" Send rename-to:          %s\n",
           snd_rename ? snd_rename : "(none)");
    if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
    printf(" Receive move-to:         %s\n",
           rcv_move ? rcv_move : "(none)");
    if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
    printf(" Receive rename-to:       %s\n",
           rcv_rename ? rcv_rename : "(none)");
    if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
#endif /* NOXFER */
#ifdef KERMRC
    printf(" Initialization file:     %s\n", noinit ? "(none)" :
#ifdef CK_SYSINI
           CK_SYSINI
#else
           kermrc
#endif /* CK_SYSINI */
           );
#endif /* KERMRC */
    if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;

    if (k_info_dir) {
        printf(" Kermit doc files:        %s\n", k_info_dir);
        if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
    }

#ifdef CKROOT
    s = zgetroot();
    printf(" Root set:                %s\n", s ? s : "(none)");
#endif /* CKROOT */

#ifdef UNIX
    printf(" Disk output buffer:      %d (writes are %s, %s)\n",
           zobufsize,
           zofbuffer ? "buffered" : "unbuffered",
           zofblock ? "blocking" : "nonblocking"
           );
    if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
#ifdef DYNAMIC
    printf(" Stringspace:             %d\n", zsetfil(0,2));
    if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
    printf(" Listsize:                %d\n", zsetfil(0,4));
    if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
#endif /* DYNAMIC */
#endif /* UNIX */
#ifdef OS2ORUNIX
    printf(" Longest filename:        %d\n", maxnam);
    if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
    printf(" Longest pathname:        %d\n", maxpath);
    if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
#endif /* OS2ORUNIX */

    printf(" Last file sent:          %s\n", sfspec ? sfspec : "(none)");
    if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
    printf(" Last file received:      %s\n", rfspec ? rfspec : "(none)");
    if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
    printf("\n Also see:\n");
    n++;
    if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
    printf(" SHOW PROTOCOL, SHOW XFER");
#ifdef CK_LABELED
    printf(", SHOW LABELED");
#endif /* CK_LABELED */
#ifdef PATTERNS
    printf(", SHOW PATTERNS");
#endif /* PATTERNS */
#ifdef STREAMING
    printf(", SHOW STREAMING");
#endif /* STREAMING */
#ifndef NOCSETS
    printf(", SHOW CHARACTER-SETS");
#endif /* NOCSETS */
    printf("\n\n");
}

#ifndef NOXFER
VOID
shoparp() {                             /* Protocol */
    extern int docrc, skipbup;
    char *s;

#ifdef CK_TIMERS
    extern int rttflg;
#endif /* CK_TIMERS */

    printf("Protocol: %s\n",ptab[protocol].p_name);

    if (protocol == PROTO_K) {
        printf("\nProtocol Parameters:   Send    Receive");
        if (timef)
          printf("\n Timeout (used=%2d):%7d*%8d ", timint, rtimo, pkttim);
        else
          printf("\n Timeout (used=%2d):%7d%9d ",  timint, rtimo, pkttim);
#ifdef XFRCAN
        printf("       Cancellation:    %s",showoff(xfrcan));
        if (xfrcan)
          printf(" %d %d", xfrchr, xfrnum);
#endif /* XFRCAN */
        printf("\n Padding:      %11d%9d", npad,   mypadn);
        if (bctr == 4)
          printf("        Block Check: blank-free-2\n");
        else
          printf("        Block Check: %6d\n",bctr);
        printf(  " Pad Character:%11d%9d", padch,  mypadc);
        printf("        Delay:       %6d\n",ckdelay);
        printf(  " Pause:        %11d%9d", pktpaus, pktpaus);
        printf("        Attributes:      %s\n",showoff(atcapr));
        printf(  " Packet Start: %11d%9d", mystch, stchr);
        printf("        Max Retries: %6d%s\n",
               maxtry,
               (maxtry == 0) ? " (unlimited)" : ""
               );
        printf(  " Packet End:   %11d%9d", seol,   eol);
        if (ebqflg)
          printf("        8th-Bit Prefix: '%c'",ebq);
        else
          printf("        8th-Bit Prefix: ('%c' but not used)",ebq);
        printf(  "\n Packet Length:%11d ", spmax);
        printf("%8d     ",  urpsiz);
        if (rptflg)
          printf("   Repeat Prefix:  '%c'",rptq);
        else
          printf("   Repeat Prefix:  ('%c' but not used)",rptq);
        printf(  "\n Maximum Length: %9d%9d", maxsps, maxrps);
        printf("        Window Size:%7d set, %d used\n",wslotr,wmax);
        printf(    " Buffer Size:  %11d%9d", bigsbsiz, bigrbsiz);
        printf("        Locking-Shift:    ");
        if (lscapu == 2) {
            printf("forced");
        } else {
            printf("%s", (lscapr ? "enabled" : "disabled"));
            if (lscapr) printf(",%s%s", (lscapu ? " " : " not "), "used");
        }
        printf("\n\n");

        if (!(s = ptab[protocol].h_b_init)) s = "";
        printf(" Auto-upload command (binary): ");
        if (*s) {
            shostrdef((CHAR *)s);
            printf("\n");
        } else {
            printf("(none)\n");
        }
        if (!(s = ptab[protocol].h_t_init)) s = "";
        printf(" Auto-upload command (text):   ");
        if (*s) {
            shostrdef((CHAR *)s);
            printf("\n");
        } else {
            printf("(none)\n");
        }
        if (!(s = ptab[protocol].h_x_init)) s = "";
        printf(" Auto-server command:          ");
        if (*s) {
            shostrdef((CHAR *)s);
            printf("\n");
        } else {
            printf("(none)\n");
        }
        tmpbuf[0] = NUL;
#ifdef CK_TIMERS
        if (rttflg) {
            extern int mintime, maxtime;
            sprintf(tmpbuf," Packet timeouts: dynamic %d:%d", /* SAFE */
                    mintime,
                    maxtime);
        } else {
            sprintf(tmpbuf," Packet timeouts: fixed"); /* SAFE */
        }
#endif /* CK_TIMERS */
        if (tmpbuf[0])
          printf("%-31s",tmpbuf);
        printf("Send backup: %s\n",showoff(!skipbup));

        printf(" Transfer mode:   %s", xfermode == XMODE_A ?
               "automatic   " :
               "manual      "
               );
        printf(" Transfer slow-start: %s, crc: %s\n",
               showoff(slostart),
               showoff(docrc)
               );
#ifdef PIPESEND
        {
            extern int usepipes;
            printf(" Transfer pipes:  %s         ",usepipes ? "on " : "off");
        }
#endif /* PIPESEND */
#ifndef NOCSETS
        printf(" Transfer character-set: ");
        if (tcharset == TC_TRANSP)
          printf("transparent\n");
        else
          printf("%s\n", tcsinfo[tcharset].keyword );
#endif /* NOCSETS */
#ifdef PIPESEND
        {
            extern char * sndfilter, * rcvfilter;
            printf(" Send filter:     %s\n", sndfilter ? sndfilter : "(none)");
            printf(" Receive filter:  %s\n", rcvfilter ? rcvfilter : "(none)");
        }
#endif /* PIPESEND */
        printf("\nAlso see:\n");
        printf(" SHOW FILE, SHOW XFER");

#ifdef CK_LABELED
        printf(", SHOW LABELED");
#endif /* CK_LABELED */
#ifdef PATTERNS
        printf(", SHOW PATTERNS");
#endif /* PATTERNS */
#ifdef STREAMING
        printf(", SHOW STREAMING");
#endif /* STREAMING */
#ifndef NOCSETS
        printf(", SHOW CHARACTER-SETS");
#endif /* NOCSETS */
    }

#ifdef CK_XYZ
#ifdef XYZ_INTERNAL
    if (protocol != PROTO_K) {
        int i;
        int x;
        printf(" File type: %s\n", binary ? "binary" : "text");
        if (protocol == PROTO_Z) {              /* Zmodem */
            printf(" Window size:   ");
            if (ptab[protocol].winsize < 1)
              printf("none\n");
            else
              printf("%d\n",wslotr);
#ifdef COMMENT
            printf(" Packet (frame) length: ");
            if (ptab[protocol].spktlen < 0)
              printf("none\n");
            else
              printf("%d\n",spmax);
#endif /* COMMENT */
        } else {
            if (ptab[protocol].spktlen >= 1000)
              printf(" 1K packets\n");
            else
              printf(" 128-byte packets\n");
        }
        printf(" Pathname stripping when sending:   %s\n",
               showoff(ptab[protocol].fnsp)
               );
        printf(" Pathname stripping when receiving: %s\n",
               showoff(ptab[protocol].fnrp)
               );
        printf(" Filename collision action:         ");
        for (i = 0; i < ncolx; i++)
          if (colxtab[i].kwval == fncact) break;
        printf("%-12s", (i == ncolx) ? "unknown" : colxtab[i].kwd);

        printf("\n Escape control characters:          ");
        x = ptab[protocol].prefix;
        if (x == PX_ALL)
          printf("all\n");
        else if (x == PX_CAU || x==PX_WIL)
          printf("minimal\n");
        else
          printf("none\n");
        if (!(s = ptab[protocol].h_b_init))
          s = "";
        printf(" Autoreceive command (binary): %s\n", *s ? s : "(none)");
        if (!(s = ptab[protocol].h_t_init))
          s = "";
        printf(" Autoreceive command (text):   %s\n", *s ? s : "(none)");
    }
#else
#ifndef NOPUSH
    if (protocol != PROTO_K) {
	_PROTOTYP( VOID shoextern, (void) );
        printf("\nExecuted by external commands:\n\n");
        s = ptab[protocol].p_b_scmd;
        if (!s) s = "";
        printf(" SEND command (binary):        %s\n", *s ? s : "(none)");
        s = ptab[protocol].p_t_scmd;
        if (!s) s = "";
        printf(" SEND command (text):          %s\n", *s ? s : "(none)");
        s = ptab[protocol].p_b_rcmd;
        if (!s) s = "";
        printf(" RECEIVE command (binary):     %s\n", *s ? s : "(none)");
        s = ptab[protocol].p_t_rcmd;
        if (!s) s = "";
        printf(" RECEIVE command (text):       %s\n", *s ? s : "(none)");
        s = ptab[protocol].h_b_init;
        if (!s) s = "";
        printf(" Autoreceive command (binary): %s\n", *s ? s : "(none)");
        s = ptab[protocol].h_t_init;
        if (!s) s = "";
        printf(" Autoreceive command (text):   %s\n", *s ? s : "(none)");
	(VOID) shoextern();
    }
#endif /* NOPUSH */
#endif /* XYZ_INTERNAL */
#endif /* CK_XYZ */
}
#endif /* NOXFER */

#ifndef NOCSETS
/* Character-set items */

extern int s_cset, r_cset, axcset[], afcset[];
extern struct keytab xfrmtab[];

VOID
shoparl() {
#ifdef COMMENT
    int i;
/* Misleading... */
    printf("\nAvailable Languages:\n");
    for (i = 0; i < MAXLANG; i++) {
        printf(" %s\n",langs[i].description);
    }
#else
    printf("\nLanguage-specific translation rules: %s\n",
           language == L_USASCII ? "none" : langs[language].description);
    shocharset();
    printf("\n\n");
#endif /* COMMENT */
}

VOID
shocharset() {
    int x;
#ifdef COMMENT
    char * s = "Unknown";
    extern int xlatype;
#endif /* COMMENT */

#ifndef NOXFER
    extern int xfrxla;
#endif /* NOXFER */

    debug(F101,"SHOW FILE CHAR","",fcharset);
    printf("\n");
#ifndef NOXFER
    printf(" Transfer Translation: %s\n", showoff(xfrxla));
    if (!xfrxla) {
        printf(
      " Because transfer translation is off, the following are ignored:\n\n");
    }
#endif /* NOXFER */
    printf(" File Character-Set: %s (%s), ",
           fcsinfo[fcharset].keyword,
           fcsinfo[fcharset].name
           );
    if ((x = fcsinfo[fcharset].size) == 128)
      printf("7-bit");
    else if (x == 256)
      printf("8-bit");
    else
      printf("multibyte");
    printf("\n");
    printf(" File Scan: %s\n",showoff(filepeek));
    printf("   Default 7bit-Character-Set: %s\n",fcsinfo[dcset7].keyword);
    printf("   Default 8bit-Character-Set: %s\n",fcsinfo[dcset8].keyword);
    printf(" Transfer Character-Set");
#ifdef COMMENT
    if (tslevel == TS_L2)
      printf(": (international)");
    else
#endif /* COMMENT */
    if (tcharset == TC_TRANSP)
      printf(": Transparent");
    else
      printf(": %s (%s)",tcsinfo[tcharset].keyword, tcsinfo[tcharset].name);
    printf("\n");
#ifdef COMMENT
    switch (xlatype) {
      case XLA_NONE: s = "None"; break;
      case XLA_BYTE: s = "Byte"; break;
      case XLA_JAPAN: s = "Japanese"; break;
      case XLA_UNICODE: s = "Unicode"; break;
    }
    printf("\n Translation type: %s\n",s);
#endif /* COMMENT */
    printf(" SEND character-set-selection: %s\n",xfrmtab[s_cset].kwd);
    printf(" RECEIVE character-set-selection: %s\n",xfrmtab[r_cset].kwd);
    if (s_cset == XMODE_A || r_cset == XMODE_A)
      printf(
      " (Use SHOW ASSOCIATIONS to list automatic character-set selections.)\n"
             );
}

VOID
showassoc() {
    int i, k, n = 4;
    char * s;
    printf("\nFor incoming files:\n\n");
    printf("Transfer Character-Set   File Character-Set\n");
    for (i = 1; i <= MAXTCSETS; i++) {
        k = axcset[i];
        if (k < 0 || k > MAXFCSETS)
          s = "(none)";
        else
          s = fcsinfo[k].keyword;
        if (!s) s = "";
        if (!*s) s = "(none)";
        printf(" %-25s%s\n",tcsinfo[i].keyword,s);
        if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
    }
    printf("\nFor outbound files:\n\n");
    n += 2;
    if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
    printf("File Character-Set       Transfer Character-Set\n");
    if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
    for (i = 0; i <= MAXFCSETS; i++) {
        k = afcset[i];
        if (k < 0 || k > MAXTCSETS)
          s = "(none)";
        else
          s = tcsinfo[k].keyword;
        if (!s) s = "";
        if (!*s) s = "(none)";
        printf(" %-25s%s\n",fcsinfo[i].keyword,s);
        if (++n > cmd_rows - 3) if (!askmore()) return; else n = 0;
    }
}
#endif /* NOCSETS */

VOID
shopar() {
    printf("Show what?  (Type \"show ?\" for a list of possibilities.)\n");
}
#endif /* NOSHOW */

#ifndef NOXFER
/*  D O S T A T  --  Display file transfer statistics.  */

int
dostat(brief) int brief; {
    extern long filrej, peakcps;
    extern int lastspmax, streamed, cleared, streamok;
    extern char whoareu[];
    int n = 0, ftp = 0;
    extern int docrc, interrupted, fatalio;

    ftp = lastxfer & W_FTP;

#ifdef CK_TTGWSIZ
#ifdef OS2
    if (tt_cols[VTERM] < 0 || tt_rows[VTERM] < 0)
      ttgwsiz();
#else /* OS2 */
    if (ttgwsiz() > 0) {
        if (tt_rows > 0 && tt_cols > 0) {
            cmd_rows = tt_rows;
            cmd_cols = tt_cols;
        }
    }
#endif /* OS2 */
#endif /* CK_TTGWSIZ */

    debug(F101,"dostat xferstat","",xferstat);
    if (xferstat < 0) {
        printf(" No file transfers yet.\n");
        return(1);
    }
    n = 0;
    if (brief) { printf("\n"); n++; };
    printf(" protocol               : %s\n",
           ftp ? "ftp" : ptab[protocol].p_name);
    n++;
    printf(" status                 : ");
    if (xferstat) printf("SUCCESS\n");
    else if (interrupted) printf("FAILURE (interrupted)\n");
    else if (fatalio) printf("FAILURE (i/o error)\n");
    else printf("FAILURE\n");
#ifndef XYZ_INTERNAL
    if (!ftp && protocol != PROTO_K) {
        printf("\n external protocol statistics not available\n");
        return(1);
    }
#endif /* XYZ_INTERNAL */
    n++;
    if (!ftp) {
        if (!xferstat > 0) {
            if (docrc)
              printf(" crc-16 of file(s)      : %ld\n", crc16);
            else
              printf(" crc-16 of file(s)      : (disabled)\n");
            n++;
        }
        if (!xferstat && *epktmsg) {
            printf(" reason                 : %s\n", epktmsg);
            n++;
        }
    }
    if (!brief) {
#ifdef NEWFTP
        if (ftp) {
            extern char ftp_srvtyp[];
            printf(" remote system type     : %s\n",ftp_srvtyp);
        } else
#endif /* NEWFTP */
          if (whoareu[0]) {
            printf(" remote system type     : %s\n",
                   getsysid((char *)whoareu));
            n++;
        }
        printf(" files transferred      : %ld\n",filcnt - filrej);
        if (!ftp)
          printf(" files not transferred  : %ld\n",filrej);
        printf(" characters last file   : %s\n",ckfstoa(ffc));
        printf(" total file characters  : %s\n",ckfstoa(tfc));
        n += ftp ? 3 : 4;
        if (!ftp) {
            printf(" communication line in  : %s\n",ckfstoa(tlci));
            printf(" communication line out : %s\n",ckfstoa(tlco));
            printf(" packets sent           : %d\n", spackets);
            printf(" packets received       : %d\n", rpackets);
            n += 4;
        }
    }
    if (ftp) goto dotimes;

    printf(" damaged packets rec'd  : %d\n", crunched);
    printf(" timeouts               : %d\n", timeouts);
    printf(" retransmissions        : %d\n", retrans);
    n += 3;

    if (!brief) {
        if (filcnt > 0) {
            printf(" parity                 : %s",parnam((char)parity));
            n++;
            if (autopar) { printf(" (detected automatically)"); n++; }
            printf(
                 "\n control characters     : %ld prefixed, %ld unprefixed\n",
                   ccp, ccu);
            n++;
            printf(" 8th bit prefixing      : ");
            n++;
            if (ebqflg) printf("yes [%c]\n",ebq); else printf("no\n");
            n++;
            printf(" locking shifts         : %s\n", lscapu ? "yes" : "no");
            n++;
        }
    }
    if (++n > cmd_rows - 3) { if (!askmore()) return(1); else n = 0; }
    if (streamed > 0)
      printf(" window slots used      : (streaming)\n");
    else
      printf(" window slots used      : %d of %d\n", wmax, wslotr);
    if (++n > cmd_rows - 3) { if (!askmore()) return(1); else n = 0; }
    printf(" reliable:              : %s%s\n",
           streamok ? "" : "not ", "negotiated");
    if (++n > cmd_rows - 3) { if (!askmore()) return(1); else n = 0; }
    printf(" clearchannel:          : %s%s\n",
           cleared  ? "" : "not ", "negotiated");
    if (++n > cmd_rows - 3) { if (!askmore()) return(1); else n = 0; }

    if (!brief) {
        printf(" packet length          : %d (send), %d (receive)\n",
               lastspmax, urpsiz);
        if (++n > cmd_rows - 3) { if (!askmore()) return(1); else n = 0; }
        printf(" compression            : ");
        if (rptflg)
          printf("yes [%c] (%ld)\n",(char) rptq,rptn);
        else
          printf("no\n");
        if (++n > cmd_rows - 3) { if (!askmore()) return(1); else n = 0; }
        if (bctu == 4)
          printf(" block check type used  : blank-free-2\n");
        else
          printf(" block check type used  : %d\n",bctu);
        if (++n > cmd_rows - 3) { if (!askmore()) return(1); else n = 0; }
    }

  dotimes:

#ifdef GFTIMER
#ifdef COMMENT
    printf(" elapsed time           : %0.3f sec, %s\n", fptsecs,hhmmss(tsecs));
#endif /* COMMENT */
    printf(" elapsed time           : %s (%0.3f sec)\n",
           hhmmss((long)(fptsecs + 0.5)),fptsecs);
#else
#ifdef COMMENT
    printf(" elapsed time           : %s (%d sec)\n",hhmmss(tsecs),tsecs);
#endif /* COMMENT */
    printf(" elapsed time           : %d sec, %s\n",tsecs,hhmmss(tsecs));
#endif /* GFTIMER */
    if (++n > cmd_rows - 3) { if (!askmore()) return(1); else n = 0; }
    if (!ftp && local && !network && !brief) {
        if (speed <= 0L) speed = ttgspd();
        if (speed > 0L) {
            if (speed == 8880)
              printf(" transmission rate      : 75/1200 bps\n");
            else
              printf(" transmission rate      : %ld bps\n",speed);
            if (++n > cmd_rows - 3) { if (!askmore()) return(1); else n = 0; }
        }
    }
    if (!ftp && local && !network &&    /* Only makes sense for */
        mdmtyp == 0 &&                  /* direct serial connections */
        speed > 99L &&                  /* when we really know the speed */
        speed != 8880L
        ) {
        int eff;
        eff = (((tfcps * 100L) / (speed / 100L)) + 5L) / 10L;
        printf(" effective data rate    : %ld cps (%d%%)\n",tfcps,eff);
    } else
      printf(" effective data rate    : %ld cps\n", tfcps);
    if (!ftp && peakcps > 0L && peakcps > tfcps)
      printf(" peak data rate         : %ld cps\n", peakcps);
    if (brief)
      printf("\nUse STATISTICS /VERBOSE for greater detail.\n\n");
    return(1);
}
#endif /* NOXFER */

#ifndef NOSPL

/* The INPUT command */

/*
  NOTE: An INPUT timeout of 0 means to perform a nonblocking read of the
  material that has already arrived and is waiting to be read, and perform
  matches against it, without doing any further reads.  It should succeed
  or fail instantaneously.
*/

/* Output buffering for "doinput" */

#ifdef pdp11
#define MAXBURST 16             /* Maximum size of input burst */
#else
#define MAXBURST 1024
#endif /* pdp11 */
#ifdef OSK
static CHAR *conbuf;            /* Buffer to hold output for console */
#else
static CHAR conbuf[MAXBURST];   /* Buffer to hold output for console */
#endif /* OSK */
static int concnt = 0;          /* Number of characters buffered */
#ifdef OSK
static CHAR *sesbuf;            /* Buffer to hold output for session log */
#else
static CHAR sesbuf[MAXBURST];   /* Buffer to hold output for session log */
#endif /* OSK */
static int sescnt = 0;          /* Number of characters buffered */

extern int debses;                      /* TERMINAL DEBUG ON/OFF */

static VOID                             /* Flush INPUT echoing */
myflsh() {                              /* and session log output. */
    if (concnt > 0) {
        if (debses) {                   /* Terminal debugging? */
            int i;
            for (i = 0; i < concnt; i++)
              conol(dbchr(conbuf[i]));
        } else
          conxo(concnt, (char *) conbuf);
        concnt = 0;
    }
    if (sescnt > 0) {
        logstr((char *) sesbuf, sescnt);
        sescnt = 0;
    }
}

/* Execute the INPUT and MINPUT commands */

int instatus = -1;
long inetime = -1L;
int inwait = 0;
int nowrap = 0;

/* For returning the input sequence that matched */

#ifdef BIGBUFOK
#define MATCHBUFSIZ 8191
#else
#define MATCHBUFSIZ 1023
#endif /* BIGBUFOK */
static char * matchbuf = NULL;
static int matchindex = 0;
static int burst = 0;                      /* Chars remaining in input burst */
/*
  timo = How long to wait:
         < 0 = Wait forever
           0 = Don't wait at all - material must already have arrived
         > 0 = Wait this many seconds
  ms   = Array of strings to wait for.
  mp   = Array of flags.
         If mp[i] == 0, ms[i] is literal, else it's a pattern.
  flags = bit mask
    INPSW_NOM = /NOMATCH = 1
    INPSW_CLR = /CLEAR   = 2
    INPSW_NOW = /NOWRAP  = 4
    INPSW_COU = /COUNT   = 8
  count = /COUNT: value if a /COUNT switch was given.

 Returns:
    0 on failure, 1 on success.
*/
#ifndef ES_NORMAL
#define ES_NORMAL 0
#endif	/* ES_NORMAL */
extern int inesc[], oldesc[];

int
doinput(timo,ms,mp,flags,count)
    int timo; char *ms[]; int mp[]; int flags; int count; {
    extern int inintr;
#ifdef CK_AUTODL
    extern int inautodl;
#endif /* CK_AUTODL */
    int x, y, i, t, rt, icn, anychar = 0, mi[MINPMAX];
#ifdef GFTIMER
    CKFLOAT fpt = 0.0;
#endif /* GFTIMER */
    int savecount = 0;
    int nomatch = 0;
    int clearfirst = 0;
    int lastchar = 0;
    int waiting = 0;
    int imask = 0;
    char ch, *xp, *s;
    CHAR c;
#ifndef NOLOCAL
#ifdef OS2
    extern int term_io;
    int term_io_save;
#endif /* OS2 */
#endif /* NOLOCAL */
#ifdef TNCODE
    static int cr = 0;
#endif /* TNCODE */
    int is_tn = 0;
#ifdef SSHBUILTIN
    extern int ssh_cas;
    extern char * ssh_cmd;
#endif /* SSHBUILTIN */
    int noescseq = 0;			/* Filter escape sequences */

    debug(F101,"input count","",count);
    debug(F101,"input flags","",flags);

/*
  CK_BURST enables the INPUT speedup code, which depends on ttchk() returning
  accurate information.  If INPUT fails with this code enabled, change the
  above "#define" to "#undef".
*/
#define CK_BURST

/***** CHANGE THIS TO A SET INPUT PARAMETER *****/

    noescseq = (sessft == XYFT_T);	/* Filter escape sequences */

    imask = cmask;
    if (parity) imask = 0x7f;
    inwait = timo;                      /* For \v(inwait) */

    /* Options from command switches */

    nowrap = flags & INPSW_NOW;		/* 4 = /NOWRAP */
    nomatch = flags & INPSW_NOM;	/* 1 = /NOMATCH */
    clearfirst = flags & INPSW_CLR;	/* 2 = /CLEAR */
    savecount = count;

    makestr(&inpmatch,NULL);
    if (!matchbuf) {
        matchbuf = malloc(MATCHBUFSIZ+1);
        matchbuf[0] = NUL;
    }
    matchindex = 0;

    /* If last time through we returned because of /NOWRAP and buffer full */
    /* now we have to clear the buffer to make room for another load. */

    if (nowrap && instatus == INP_BF)
      clearfirst = 1;

    if (clearfirst) {			/* INPUT /CLEAR */
	int i;
	myflsh();			/* Flush screen and log buffers */
	for (i = 0; i < inbufsize; i++)
	  inpbuf[i] = NUL;
	inpbp = inpbuf;
    }
    is_tn =
#ifdef TNCODE
        (local && network && IS_TELNET()) || (!local && sstelnet)
#else
         0
#endif /* TNCODE */
          ;

#ifdef CK_SSL
    if (is_tn) if (ssl_raw_flag || tls_raw_flag) is_tn = 0;
#endif	/* CK_SSL */

    instatus = INP_IE;                  /* 3 = internal error */
    kbchar = 0;

#ifdef OSK
    if (conbuf == NULL) {
        if ((conbuf = (CHAR *)malloc(MAXBURST*2)) == NULL) {
            return(0);
        }
        sesbuf = conbuf + MAXBURST;
    }
#endif /* OSK */

#ifndef NOLOCAL
    if (local) {                        /* In local mode... */
        if ((waiting = ttchk()) < 0) {  /* check that connection is open */
	    if (!quiet) {
		if ((!network 
#ifdef TN_COMPORT
		      || istncomport()
#endif /* TN_COMPORT */
		      ) && carrier != CAR_OFF)
		    printf("?Carrier detect failure on %s.\n", ttname);
		else
		    printf("?Connection %s %s is not open.\n",
		       network ? "to" : "on",
		       ttname
		       );
	    }
            instatus = INP_IO;
            return(0);
        }
        debug(F101,"doinput waiting","",waiting);
        y = ttvt(speed,flow);           /* Put line in "ttvt" mode */
        if (y < 0) {
            printf("?INPUT initialization error\n");
            instatus = INP_IO;
            return(0);                  /* Watch out for failure. */
        }
    }
#endif /* NOLOCAL */

#ifdef SSHBUILTIN
    if ( network && nettype == NET_SSH && ssh_cas && ssh_cmd && 
         !(strcmp(ssh_cmd,"kermit") && strcmp(ssh_cmd,"sftp"))) {
        if (!quiet)
	  printf("?SSH Subsystem active: %s\n", ssh_cmd);
        instatus = INP_IKS;
        return(0);
    }
#endif /* SSHBUILTIN */

    debug(F111,"doinput ms[0]",ms[0],waiting);

    if (!ms[0] || isemptystring(ms[0])) { /* No search string was given nor */
	if (count < 2)			  /* a /COUNT: switch so we just */
	  anychar = 1;			  /* wait for the first character */
    }
    if (nomatch) anychar = 0;		/* Don't match anything */

    if (!anychar && waiting == 0 && timo == 0)
      return(0);

#ifndef NODEBUG
    if (deblog) {
        char xbuf[24];
        debug(F101,"doinput anychar","",anychar);
        debug(F101,"doinput timo","",timo);
        debug(F101,"doinput echo","",inecho);
#ifdef CK_BURST
        debug(F101,"doinput burst","",burst);
#endif	/* CK_BURST */
        y = -1;
        while (ms[++y]) {
            sprintf(xbuf,"doinput string %2d",y); /* SAFE (24) */
            debug(F111,xbuf,ms[y],mp[y]);
        }
    }
#endif /* NODEBUG */

#ifdef IKS_OPTION
    if (is_tn) {
        /* If the remote side is in a state of IKS START-SERVER    */
        /* we request that the state be changed.  We will detect   */
        /* a failure to adhere to the request when we call ttinc() */
        if (TELOPT_U(TELOPT_KERMIT) &&
            TELOPT_SB(TELOPT_KERMIT).kermit.u_start)
          iks_wait(KERMIT_REQ_STOP,0);  /* Send Request-Stop */
#ifdef CK_AUTODL
        /* If we are processing packets during INPUT and we have not */
        /* sent a START message, do so now.                          */
        if (inautodl && TELOPT_ME(TELOPT_KERMIT) &&
			!TELOPT_SB(TELOPT_KERMIT).kermit.me_start) {
            tn_siks(KERMIT_START);      /* Send Kermit-Server Start */
        }
#endif /* CK_AUTODL */
    }
#endif /* IKS_OPTION */
    x = 0;                              /* Return code, assume failure */
    instatus = INP_TO;                  /* Status, assume timeout */

    for (y = 0; y < MINPMAX; y++)	/* Initialize... */
      mi[y] = 0;                        /*  ..string pattern match position */

    if (!inpcas[cmdlvl]) {              /* INPUT CASE = IGNORE?  */
        y = -1;
        while ((xp = ms[++y])) {	/* Convert each target to lowercase */
            while (*xp) {
                if (isupper(*xp)) *xp = (char) tolower(*xp);
                xp++;
            }
        }
    }
    rtimer();                           /* Reset timer. */
#ifdef GFTIMER
    rftimer();                          /* Floating-point timer too. */
#endif /* GFTIMER */
    inetime = -1L;                      /* Initialize elapsed time. */
    t = 0;                              /* Time now is 0. */
    m_found = 0;                        /* Default to timed-out */
    incount = 0;                        /* Character counter */
    rt = (timo == 0) ? 0 : 1;           /* Character-read timeout interval */

#ifndef NOLOCAL
#ifdef OS2
    term_io_save = term_io;             /* Disable I/O by emulator */
    term_io = 0;
#endif /* OS2 */
#endif /* NOLOCAL */

    while (1) {                         /* Character-getting loop */
#ifdef CK_APC
        /* Check to see if there is an Autodown or other APC command */
        if (apcactive == APC_LOCAL ||
            (apcactive == APC_REMOTE && apcstatus != APC_OFF)) {
            if (mlook(mactab,"_apc_commands",nmac) == -1) {
                debug(F110,"doinput about to execute APC",apcbuf,0);
                domac("_apc_commands",apcbuf,cmdstk[cmdlvl].ccflgs|CF_APC);
                delmac("_apc_commands",1);
                apcactive = APC_INACTIVE;
#ifdef DEBUG
            } else {
                debug(F100,"doinput APC in progress","",0);
#endif /* DEBUG */
            }
        }
#endif /* CK_APC */

        if (timo == 0 && waiting < 1) { /* Special exit criterion */
            instatus = INP_TO;          /* for timeout == 0 */
            break;
        }
        if (local) {                    /* One case for local */
            y = ttinc(rt);              /* Get character from comm device */
            debug(F101,"doinput ttinc(rt) returns","",y);
            if (y < -1) {               /* Connection failed. */
                instatus = INP_IO;      /* Status = i/o error */
#ifndef NOLOCAL
#ifdef OS2
                term_io = term_io_save;
#endif /* OS2 */
#endif /* NOLOCAL */
                switch (y) {
                  case -2:              /* Connection lost */
                    if (local && !network && carrier != CAR_OFF) {
                        dologend();
                        printf("Connection closed.\n");
                        ttclos(1);
                    }
                    break;
                  case -3:
                    dologend();
                    printf("Session Limit exceeded - closing connection.\n");
                    ttclos(1);
                  default:
                    break;
                }
                debug(F111,"doinput Connection failed","returning 0",y);
                return(0);
            }
            if (inintr) {
                debug(F111,"doinput","inintr",inintr);
                if ((icn = conchk()) > 0) { /* Interrupted from keyboard? */
                    debug(F101,"input interrupted from keyboard","",icn);
                    kbchar = coninc(0);
                    if (kbchar >= 0) {
                        while (--icn > 0) {
                            debug(F110,"doinput","absorbing",0);
                            coninc(0);      /* Yes, absorb what was typed. */
                        }
                        instatus = INP_UI;  /* Fail and remember why. */
                        break;
                    }
                }
            }
        } else {                        /* Another for remote */
            y = coninc(rt);
            debug(F101,"doinput coninc(rt) returns","",y);
        }
        if (y > -1) {                   /* A character arrived */
            debug(F111,"doinput","a character arrived",y);
            if (timo == 0)
              waiting--;
#ifndef OS2
#define TN_NOLO
#endif /* OS2 */
#ifdef NOLOCAL
#define TN_NOLO
#endif /* NOLOCAL */

#ifdef TN_NOLO
            debug(F100,"doinput TN_NOLO","",0);
#ifdef TNCODE
            /* Check for telnet protocol negotiation */
            if (is_tn) {
                switch (y & 0xff) {
                  case IAC:
                    cr = 0;
                    myflsh();   /* Break from input burst for tn_doop() */
#ifdef CK_BURST
                    burst = 0;
#endif /* CK_BURST */
                    waiting -= 2;       /* (not necessarily...) */
                    switch (tn_doop((CHAR)(y & 0xff),duplex,ttinc)) {
                      case 2: duplex = 0; continue;
                      case 1: duplex = 1; continue;
#ifdef IKS_OPTION
                      case 4:
                        if (TELOPT_SB(TELOPT_KERMIT).kermit.u_start &&
                             !tcp_incoming) {
                            instatus = INP_IKS;
                            printf(
 " Internet Kermit Service in SERVER mode.\n Please use REMOTE commands.\n"
                                   );
                            break;
                        }
                        continue;
#endif /* IKS_OPTION */
                      case 6:           /* TELNET DO LOGOUT received */
			continue;
		      case 7:
		      case 3:		/* A quoted IAC */
			break;
                      default:
			continue;
                    }
                  case CR:
                    cr = 1;
                    break;
                  case NUL:
                    if (!TELOPT_U(TELOPT_BINARY) && cr) {
                        cr = 0;
                        continue;
                    }
                    cr = 0;
                    break;
                  default:
                    cr = 0;
                }
                /* I'm echoing remote chars */
                if (TELOPT_ME(TELOPT_ECHO) && tn_rem_echo)
                  ttoc((char)y);
            }
#endif /* TNCODE */
#ifdef CK_AUTODL
            /* Check for file transfer packets */
            if (inautodl) autodown(y);
#endif /* CK_AUTODL */
#else  /* TN_NOLO */
            debug(F100,"doinput !TN_NOLO","",0);
#ifdef TNCODE
            /* Check for telnet protocol negotiation */
            if (is_tn) {
                int tx;
                switch (y & 0xff) {
                  case IAC:
                    myflsh();   /* Break from input burst for tn_doop() */
#ifdef CK_BURST
                    burst = 0;
#endif /* CK_BURST */
#ifdef IKS_OPTION
                    tx = scriptwrtbuf((USHORT)y);
                    if (tx == 4) {
                        if (TELOPT_U(TELOPT_KERMIT) && 
			    TELOPT_SB(TELOPT_KERMIT).kermit.u_start &&
                            !tcp_incoming
                            ) {
                            instatus = INP_IKS;
                            printf(
  " Internet Kermit Service in SERVER mode.\n Please use REMOTE commands.\n"
                                   );
                            break;
                        }
                    } else if (tx == 6) {
                        /* TELNET DO LOGOUT received */

                    }
#else /* IKS_OPTION */
                    /* Handles Telnet negotiations */
                    tx = scriptwrtbuf((USHORT)y);
                    if (tx == 6) {
                        /* TELNET DO LOGOUT received */
                    }
#endif /* IKS_OPTION */
                    waiting -= 2;       /* (not necessarily...) */
                    cr = 0;
                    continue;           /* and autodownload check */
                  case CR:
                    cr = 1;
                    tx = scriptwrtbuf((USHORT)y);
                    if (tx == 6) {
                        /* TELNET DO LOGOUT received */
                    }
                    break;
                  case NUL:
                    cr = 0;
                    if (!TELOPT_U(TELOPT_BINARY) && cr)
                      continue;
                    tx = scriptwrtbuf((USHORT)y);
                    if (tx == 6) {
                        /* TELNET DO LOGOUT received */
                    }
                    break;
                  default:
                    cr = 0;
                    tx = scriptwrtbuf((USHORT)y);
                    if (tx == 6) {
                        /* TELNET DO LOGOUT received */
                    }
                }
                /* I'm echoing remote chars */
                if (TELOPT_ME(TELOPT_ECHO) && tn_rem_echo)
                  ttoc((CHAR)y);
            } else
#endif /* TNCODE */
              /* Handles terminal emulation responses */
              scriptwrtbuf((USHORT)y);
#endif /* TN_NOLO */

            /* Real input character to be checked */

#ifdef CK_BURST
            burst--;                    /* One less character waiting */
            debug(F101,"doinput burst","",burst);
#endif /* CK_BURST */
            c = (CHAR) (imask & (CHAR) y); /* Mask off any parity */
            inchar[0] = c;              /* Remember character for \v(inchar) */
#ifdef COMMENT
#ifdef CK_BURST
            /* Update "lastchar" time only once during input burst */
            if (burst <= 0)
#endif /* CK_BURST */
#endif /* COMMENT */
              lastchar = gtimer();      /* Remember when it came */

            if (c == '\0') {            /* NUL, we can't use it */
                if (anychar) {          /* Except if any character will do? */
                    x = 1;              /* Yes, done. */
		    instatus = INP_OK;
                    incount = 1;        /* This must be the first and only. */
                    break;
                } else goto refill;	/* Otherwise continue INPUTting */
            }
            *inpbp++ = c;               /* Store char in circular buffer */
            incount++;                  /* Count it for \v(incount) */

	    if (flags & INPSW_COU) {	/* INPUT /COUNT */
		if (--count < 1) {
		    x = 1;
		    instatus = INP_OK;
                    incount = savecount;
                    break;
		}
	    }
            if (matchbuf) {
                if (matchindex < MATCHBUFSIZ) {
                    matchbuf[matchindex++] = c;
                    matchbuf[matchindex] = NUL;
                }
            }
#ifdef MAC
            {
                extern char *ttermw;    /* fake pointer cast */
                if (inecho) {
                    outchar(ttermw, c); /* echo to terminal window */
                    /* this might be too much overhead to do here ? */
                    updatecommand(ttermw);
                }
            }
#else /* Not MAC */
            if (inecho) {               /* Buffer console output */
                conbuf[concnt++] = c;
            }
#endif /* MAC */
#ifndef OS2
            if (seslog) {
		int dummy = 0, skip = 0;
#ifndef NOLOCAL
		if (noescseq) {
		    dummy = chkaes(c,0);
		    if (inesc[0] != ES_NORMAL || oldesc[0] != ES_NORMAL)
		      skip = 1;
		}
#endif	/* NOLOCAL */
#ifdef UNIXOROSK
		if (sessft == XYFT_T) {
#ifdef UNIX
		    if (c == '\r')
#else
#ifdef OSK
		    if (c == '\012')
#endif /* OSK */
#endif /* UNIX */
		      skip = 1;
		}
#endif	/* UNIXOROSK */
		if (!skip)
                  sesbuf[sescnt++] = c; /* Buffer session log output */
            }
#endif /* OS2 */
            if (anychar) {              /* Any character will do? */
                x = 1;
		instatus = INP_OK;
                break;
            }
            if (!inpcas[cmdlvl]) {      /* Ignore alphabetic case? */
                if (isupper(c))         /* Yes, convert input char to lower */
                  c = (CHAR) tolower(c);
            }
            debug(F000,"doinput char","",c);

            /* Here is the matching section */

            y = -1;                     /* Loop thru search strings */
            while (!nomatch && (s = ms[++y])) {	/* ...as many as we have. */
                if (mp[y]) {            /* Pattern match? */
#ifdef COMMENT
                    int j;
                    /* This is gross but it works... */
                    /* We could just as easily have prepended '*' to the  */
                    /* pattern and skipped the loop, except then we would */
                    /* not have any way to identify the matching string.  */
                    for (j = 0; j < matchindex; j++) {
                        if (ckmatch(s,&matchbuf[j],1,1)) {
                            matchindex = j;
			    instatus = INP_OK;
                            x = 1;
                            break;
                        }
                    }
                    if (x > 0)
                      break;
#else
                    /* July 2001 - ckmatch() returns match position. */
                    /* It works and it's not gross. */
		    /* (4 = floating pattern) */
                    x = ckmatch(s,matchbuf,inpcas[cmdlvl],1+4);
                    if (x > 0) {
                        matchindex = x - 1;
			instatus = INP_OK;
                        x = 1;
                        break;
                    }
#endif /* COMMENT */
                    continue;
                }                       /* Literal match. */
                i = mi[y];              /* Match-position in search string. */
                debug(F000,"compare char","",(CHAR)s[i]);
                if (c == (CHAR) s[i]) { /* Check for match */
                    i++;                /* Got one, go to next character */
                } else {                /* Don't have a match */
                    int j;
                    for (j = i; i > 0; ) { /* Back up in search string */
                        i--; /* (Do this here to prevent compiler foulup) */
                        /* j is the length of the substring that matched */
                        if (c == (CHAR) s[i]) {
                            if (!strncmp(s,&s[j-i],i)) {
                                i++;          /* c actually matches -- cfk */
                                break;
                            }
                        }
                    }
                }
                if ((CHAR) s[i] == (CHAR) '\0') { /* Matched to end? */
                    ckstrncpy(matchbuf,ms[y],MATCHBUFSIZ);
                    matchindex = 0;
		    instatus = INP_OK;  /* Yes, */
                    x = 1;            
                    break;              /* done. */
                }
                mi[y] = i;              /* No, remember match-position */
            }
            if (x == 1) {               /* Set \v(minput) result */
		instatus = INP_OK;
                m_found = y + 1;
                break;
            }
            if (inpbp >= inpbuf + inbufsize) { /* Reached end of buffer? */
		if (nowrap) {		/* If /NOWRAP...*/
		    instatus = INP_BF;	/* ...return indicating buffer full. */
		    *inpbp = NUL;
		    goto xinput;
		}
                *inpbp = NUL;           /* Make it null-terminated */
                inpbp = inpbuf;         /* Yes. */
            }
        }
#ifdef CK_BURST
        else if (y <= -1 && burst > 0) {
            debug(F111,"doinput (y<=-1&&burst>0)","burst",burst);
                                        /* A timeout occurred so there can't */
            burst = 0;                  /* be data waiting; must check timo */
        }
      refill:
        if (burst <= 0) {               /* No buffered chars remaining... */
            myflsh();                   /* Flush buffered output */
            if (local) {                /* Get size of next input burst */
                burst = ttchk();
                if (burst < 0) {        /* ttchk() says connection is closed */
                    instatus = INP_IO;  /* Status = i/o error */
#ifndef NOLOCAL
#ifdef OS2
                    term_io = term_io_save;
#endif /* OS2 */
#endif /* NOLOCAL */

		    if ((!network 
#ifdef TN_COMPORT
		         || istncomport()
#endif /* TN_COMPORT */
			 ) && carrier != CAR_OFF) {
	/* The test is written this way because the Microsoft compiler
	 * is producing bad code if written:
	 *
	 *  if (network && (!istncomport() || carrier == CAR_OFF) )
	 */
			break;
                     } else {
			 printf("Fatal error - disconnected.\n");
			 ttclos(1);
			 break;
		     }
                }
                if (inintr) {
                    if ((icn = conchk()) > 0) { /* Interrupt from keyboard? */
                        kbchar = coninc(0);
                        debug(F101,"input interrupted from keyboard","",icn);
                        while (--icn > 0) coninc(0); /* Yes, absorb chars. */
                        break;          /* And fail. */
                    }
                }
            } else {
                burst = conchk();
            }
            debug(F101,"doinput burst","",burst);
            /* Prevent overflow of "conbuf" and "sesbuf" */
            if (burst > MAXBURST)
              burst = MAXBURST;

            /* Did not match, timer exceeded? */
            t = gtimer();
            debug(F111,"doinput gtimer","burst",t);
            debug(F101,"doinput timo","",timo);
            if ((t >= timo) && (timo > 0))
              break;
            else if (insilence > 0 && (t - lastchar) > insilence)
              break;
        } else {
            debug(F111,"doinput (burst > 0)","burst",burst);
        }
#else  /* CK_BURST */
      refill:
        myflsh();                       /* Flush buffered output */
        /* Did not match, timer exceeded? */
        t = gtimer();
        debug(F111,"doinput gtimer","no burst",t);
        debug(F101,"doinput timo","",timo);
        if ((t >= timo) && (timo > -1))
          break;
        else if (insilence > 0 && (t - lastchar) > insilence)
          break;
#endif /* CK_BURST */
    }                                   /* Still have time left, continue. */
  xinput:
    myflsh();                           /* Flush buffered output */
    if (instatus == INP_BF) {		/* Buffer full and /NOWAIT */
	x = 0;				/* Must not succeed */
    } else {				/* Buffer full and /NOWAIT */
	if (nomatch) x = 1;		/* Succeed if nomatch and timed out */
	if (x > 0 && !nomatch)
	  instatus = 0;
    }
#ifndef NOLOCAL
#ifdef OS2
    term_io = term_io_save;
#endif /* OS2 */
#endif /* NOLOCAL */
#ifdef COMMENT
#ifdef IKS_OPTION
#ifdef CK_AUTODL
    if (is_tn && TELOPT_ME(TELOPT_KERMIT) && inautodl) {
        tn_siks(KERMIT_STOP);           /* Send Kermit-Server Stop */
    }
#endif /* CK_AUTODL */
#endif /* IKS_OPTION */
#endif /* COMMENT */

#ifdef GFTIMER
    fpt = gftimer();                    /* Get elapsed time */

/* If a long is 32 bits, it would take about 50 days for this to overflow. */

    inetime = (int)(fpt * (CKFLOAT)1000.0);
#else
    inetime = (int)(gtimer() * 1000);
#endif /* GFTIMER */

    if (x > 0)
      makestr(&inpmatch,&matchbuf[matchindex]); /* \v(inmatch) */
    return(x);                          /* Return the return code. */
}
#endif /* NOSPL */

#ifndef NOSPL
/* REINPUT Command */

/*
  Note, the timeout parameter is required, but ignored.  Syntax is compatible
  with MS-DOS Kermit except timeout can't be omitted.  This function only
  looks at the characters already received and does not read any new
  characters from the connection.
*/
int
doreinp(timo,s,pat) int timo; char *s; int pat; {
    int x, y, i;
    char *xx, *xp, *xq = (char *)0;
    CHAR c;

    if (!s) s = "";
    debug(F101,"doreinput pat","",pat);

    y = (int)strlen(s);
    debug(F111,"doreinput search",s,y);

    if (y > inbufsize) {                /* If search string longer than */
        debug(F101,"doreinput inbufsize","",inbufsize);
        return(0);                      /* input buffer, fail. */
    }
    makestr(&inpmatch,NULL);
    if (!matchbuf)
      matchbuf = malloc(MATCHBUFSIZ+1);
    matchindex = 0;

    x = 0;                              /* Return code, assume failure */
    i = 0;                              /* String pattern match position */

    if (!inpcas[cmdlvl]) {              /* INPUT CASE = IGNORE?  */
        xp = malloc(y+2);               /* Make a separate copy of the */
        if (!xp) {                      /* search string. */
            printf("?malloc error 6\n");
            return(x);
        } else xq = xp;                 /* Keep pointer to beginning. */
        while (*s) {                    /* Yes, convert to lowercase */
            *xp = *s;
            if (isupper(*xp)) *xp = (char) tolower(*xp);
            xp++; s++;
        }
        *xp = NUL;                      /* Terminate it! */
        s = xq;                         /* Move search pointer to it. */
    }
    xx = *inpbp ? inpbp : inpbuf;       /* Current INPUT buffer pointer */
    do {
        c = *xx++;                      /* Get next character */
        if (!c) break;
        if (xx >= inpbuf + inbufsize)   /* Wrap around if necessary */
          xx = inpbuf;
        if (!inpcas[cmdlvl]) {          /* Ignore alphabetic case? */
            if (isupper(c)) c = (CHAR) tolower(c); /* Yes */
        }
        if (pat) {
            int j;
            if (matchbuf) {
                if (matchindex < MATCHBUFSIZ) {
                    matchbuf[matchindex++] = c;
                    matchbuf[matchindex] = NUL;
                }
                for (j = 0; j < matchindex; j++) { /* Gross but effective */
                    if (ckmatch(s,&matchbuf[j],1,1)) {
                        debug(F101,"GOT IT","",j);
                        matchindex = j;
                        x = 1;
                        break;
                    }
                }
            }
            if (x > 0)
              break;
            continue;
        }
        debug(F000,"doreinp char","",c);
        debug(F000,"compare char","",(CHAR) s[i]);
        if (((char) c) == ((char) s[i])) { /* Check for match */
            i++;                        /* Got one, go to next character */
        } else {                        /* Don't have a match */
            int j;
            for (j = i; i > 0; ) {      /* [jrs] search backwards for it  */
                i--;
                if (((char) c) == ((char) s[i])) {
                    if (!strncmp(s,&s[j-i],i)) {
                        i++;
                        break;
                    }
                }
            }
        }                               /* [jrs] or return to zero from -1 */
        if (s[i] == '\0') {             /* Matched all the way to end? */
            ckstrncpy(matchbuf,s,MATCHBUFSIZ);
            matchindex = 0;
            x = 1;                      /* Yes, */
            break;                      /* done. */
        }
    } while (xx != inpbp && x < 1);     /* Until back where we started. */

    if (!inpcas[cmdlvl]) if (xq) free(xq); /* Free this if it was malloc'd. */
    makestr(&inpmatch,&matchbuf[matchindex]); /* \v(inmatch) */
    return(x);                          /* Return search result. */
}

/*  X X S T R I N G  --  Interpret strings containing backslash escapes  */
/*  Z Z S T R I N G  --  (new name...)  */
/*
 Copies result to new string.
  strips enclosing braces or doublequotes.
  interprets backslash escapes.
  returns 0 on success, nonzero on failure.
  tries to be compatible with MS-DOS Kermit.

 Syntax of input string:
  string = chars | "chars" | {chars}
  chars = (c*e*)*
  where c = any printable character, ascii 32-126
  and e = a backslash escape
  and * means 0 or more repetitions of preceding quantity
  backslash escape = \operand
  operand = {number} | number | fname(operand) | v(name) | $(name) | m(name)
  number = [r]n[n[n]]], i.e. an optional radix code followed by 1-3 digits
  radix code is oO (octal), xX (hex), dD or none (decimal) (see xxesc()).
*/

#ifndef NOFRILLS
int
yystring(s,s2) char *s; char **s2; {    /* Reverse a string */
    int x;
    static char *new;
    new = *s2;
    if (!s || !new) return(-1);         /* Watch out for null pointers. */
    if ((x = (int)strlen(s)) == 0) {    /* Recursion done. */
        *new = '\0';
        return(0);
    }
    x--;                                /* Otherwise, call self */
    *new++ = s[x];                      /* to reverse rest of string. */
    s[x] = 0;
    return(yystring(s,&new));
}
#endif /* NOFRILLS */

static char ipabuf[16] = { NUL };       /* IP address buffer */

static char *
getip(s) char *s; {
    char c=NUL;                         /* Workers... */
    int i=0, p=0, d=0;
    int state = 0;                      /* State of 2-state FSA */

    while ((c = *s++)) {
        switch(state) {
          case 0:                       /* Find first digit */
            i = 0;                      /* Output buffer index */
            ipabuf[i] = NUL;            /* Initialize output buffer */
            p = 0;                      /* Period counter */
            d = 0;                      /* Digit counter */
            if (isdigit(c)) {           /* Have first digit */
                d = 1;                  /* Count it */
                ipabuf[i++] = c;        /* Copy it */
                state = 1;              /* Change state */
            }
            break;

          case 1:                       /* In numeric field */
            if (isdigit(c)) {           /* Have digit */
                if (++d > 3)            /* Too many */
                  state = 0;            /* Start over */
                else                    /* Not too many */
                  ipabuf[i++] = c;      /* Keep it */
            } else if (c == '.' && p < 3) { /* Have a period */
                p++;                    /* Count it */
                if (d == 0)             /* Not preceded by a digit */
                  state = 0;            /* Start over */
                else                    /* OK */
                  ipabuf[i++] = c;      /* Keep it */
                d = 0;                  /* Reset digit counter */
            } else if (p == 3 && d > 0) { /* Not part of address */
                ipabuf[i] = NUL;        /* If we have full IP address */
                return((char *)ipabuf); /* Return it */
            } else {                    /* Otherwise */
                state = 0;              /* Start over */
                ipabuf[0] = NUL;        /* (in case no more chars left) */
            }
        }
    }                                   /* Fall thru at end of string */
    ipabuf[i] = NUL;                    /* Maybe we have one */
    return((p == 3 && d > 0) ? (char *)ipabuf : "");
}
#endif /* NOSPL */

/* Date Routines */

/* Z J D A T E  --  Convert yyyymmdd date to Day of Year */

static int jdays[12] = {  0,31,59,90,120,151,181,212,243,273,304,334 };
static int ldays[12] = {  0,31,60,91,121,152,182,213,244,274,305,335 };
static char zjdbuf[12] = { NUL, NUL };
/*
  Deinde, ne in posterum a XII kalendas aprilis aequinoctium recedat,
  statuimus bissextum quarto quoque anno (uti mos est) continuari debere,
  praeterquam in centesimis annis; qui, quamvis bissextiles antea semper
  fuerint, qualem etiam esse volumus annum MDC, post eum tamen qui deinceps
  consequentur centesimi non omnes bissextiles sint, sed in quadringentis
  quibusque annis primi quique tres centesimi sine bissexto transigantur,
  quartus vero quisque centesimus bissextilis sit, ita ut annus MDCC, MDCCC,
  MDCCCC bissextiles non sint. Anno vero MM, more consueto dies bissextus
  intercaletur, februario dies XXIX continente, idemque ordo intermittendi
  intercalandique bissextum diem in quadringentis quibusque annis perpetuo
  conservetur.  - Gregorius XIII, Anno Domini MDLXXXII.
*/
char *
zjdate(date) char * date; {             /* date = yyyymmdd */
    char year[5];
    char month[3];
    char day[3];
    int d, m, x, y;
    int leapday, j;
    char * time = NULL;

    if (!date) date = "";               /* Validate arg */
    x = strlen(date);
    if (x < 1) return("0");
    if (x < 8) return("-1");
    for (x = 0; x < 8; x++)
      if (!isdigit(date[x]))
        return("-1");

    if (date[8]) if (date[9])
      time = date + 9;

    year[0] = date[0];                  /* Isolate year */
    year[1] = date[1];
    year[2] = date[2];
    year[3] = date[3];
    year[4] = '\0';

    month[0] = date[4];                 /* Month */
    month[1] = date[5];
    month[2] = '\0';;

    day[0] = date[6];                   /* And day */
    day[1] = date[7];
    day[2] = '\0';

    leapday = 0;                        /* Assume no leap day */
    y = atoi(year);
    m = atoi(month);
    d = atoi(day);
    if (m > 2) {                        /* No Leap day before March */
        if (y % 4 == 0) {               /* If year is divisible by 4 */
            leapday = 1;                /* It's a Leap year */
            if (y % 100 == 0) {         /* Except if divisible by 100 */
                if (y % 400 != 0)       /* but not by 400 */
                  leapday = 0;
            }
        }
    }
    j = jdays[m - 1] + d + leapday;     /* Day of year */
    if (time)
      sprintf(zjdbuf,"%04d%03d %s",y,j,time); /* SAFE */
    else
      sprintf(zjdbuf,"%04d%03d",y,j);   /* SAFE */
    return((char *)zjdbuf);
}

static char jzdbuf[32];

/* J Z D A T E  --  Convert Day of Year to yyyyddmm date */

char *
jzdate(date) char * date; {             /* date = yyyyddd */
    char year[5];                       /* with optional time */
    char day[4];
    char * time = NULL, * p;
    int d, m, x, y;
    int leapday, j;
    int * zz;

    if (!date) date = "";               /* Validate arg */
    x = strlen(date);

    debug(F111,"jzdate len",date,x);

    if (x < 1) return("0");
    if (x < 7) return("-1");
    if (x > 8) time = date + 8;

    for (x = 0; x < 7; x++)
      if (!isdigit(date[x]))
        return("-1");

    year[0] = date[0];                  /* Isolate year */
    year[1] = date[1];
    year[2] = date[2];
    year[3] = date[3];
    year[4] = '\0';

    debug(F110,"jzdate year",year,0);

    day[0] = date[4];                   /* And day */
    day[1] = date[5];
    day[2] = date[6];
    day[3] = '\0';

    debug(F110,"jzdate day",day,0);

    j = atoi(day);
    if (j > 366)
      return("-1");

    leapday = 0;                        /* Assume no leap day */
    y = atoi(year);
    if (y % 4 == 0) {                   /* If year is divisible by 4 */
        leapday = 1;                    /* It's a Leap year */
        if (y % 100 == 0) {             /* Except if divisible by 100 */
            if (y % 400 != 0)           /* but not by 400 */
              leapday = 0;
        }
    }
    debug(F101,"jzdate leapday","",leapday);
    zz = leapday ? ldays : jdays;

    for (x = 0; x < 11; x++)
      if (j > zz[x] && j <= zz[x+1])
        break;
    m = x + 1;

    debug(F101,"jzdate m","",m);

    d = j - zz[x];

    debug(F101,"jzdate d","",d);

    if (time)
      sprintf(jzdbuf,"%04d%02d%02d %s",y,m,d,time); /* SAFE */
    else
      sprintf(jzdbuf,"%04d%02d%02d",y,m,d); /* SAFE */

    debug(F101,"jzdate jzdbuf",jzdbuf,0);

    p = ckcvtdate((char *)jzdbuf, 0);   /* Convert to standard form */
    ckstrncpy(jzdbuf,p,32);
    if (!time) jzdbuf[8] = NUL;         /* Remove time if not wanted */
    return((char *)jzdbuf);
}

/* M J D  --  Modified Julian Date */
/*
  Call with:
    Standard-format date-time string: yyyymmdd[ hh:mm:ss].
    The time, if any, is ignored.

  Returns:
    -1L on error, otherwise:
    The number of days since 17 Nov 1858 as a whole number:
    16 Nov 1858 = -1, 17 Nov 1858 = 0, 18 Nov 1858 = 1, 19 Nov 1858 = 2, ...

  The Modified Julian Date is defined by the International Astronomical
  Union as the true Julian date minus 2400000.5 days.  The true Julian
  date is the number days since since noon of 1 January 4713 BCE of the
  Julian proleptic calendar.  Conversions between calendar dates and
  Julian dates, however, assume Gregorian dating.
*/
long
mjd(date) char * date; {
    char year[5];
    char month[3];
    char day[3];
    int x, a, d, m, y;
    long z;

    if (!date) date = "";               /* Validate arg */
    x = strlen(date);
    if (x < 1) return(0L);
    if (x < 8) return(-1L);
    for (x = 0; x < 8; x++)
      if (!isdigit(date[x]))
        return(-1L);

    year[0] = date[0];                  /* Isolate year */
    year[1] = date[1];
    year[2] = date[2];
    year[3] = date[3];
    year[4] = '\0';

    month[0] = date[4];                 /* Month */
    month[1] = date[5];
    month[2] = '\0';;
    m = atoi(month);

    day[0] = date[6];                   /* And day */
    day[1] = date[7];
    day[2] = '\0';
    d = atoi(day);

    a = (14-m)/12;                      /* Calculate true Julian date */
    y = atoi(year) + 4800 - a;
    m = m + 12 * a - 3;
    z = d + (long)(306*m+5)/10 + (long)(y*365) + y/4 - y/100 + y/400 - 32045L;

    z -= 2400001L;                      /* Convert JD to MJD */

    return(z);
}

static char mjd2dbuf[32];

/*  M J D 2 D A T E  --  Converts MJD to yyyymmdd  */

char *
#ifdef CK_ANSIC
mjd2date(long mjd)
#else
mjd2date(mjd) long mjd;
#endif /* CK_ANSIC */
/* mjd2date */ {
    long jd, l, n;
    int d, m, y;
    jd = (long)(mjd + 2400001L);
    l = jd + 68569;
    n = 4 * l / 146097L;
    l = l - (146097 * n + 3) / 4;
    y = 4000 * (l + 1) / 1461001L;
    l = l - 1461 * y / 4 + 31;
    m = 80 * l / 2447;
    d = l - 2447 * m / 80;
    l = m / 11;
    m = m + 2 - 12 * l;
    y = 100 * (n - 49) + y + l;
    sprintf(mjd2dbuf,"%04d%02d%02d",y,m,d); /* SAFE */
    return((char *)mjd2dbuf);
}

#ifndef NOSPL
static char ** flist = (char **) NULL;  /* File list for \fnextfile() */
static int flistn = 0;                  /* Number of items in file list */

/*
  The function return-value buffer must be global, since fneval() returns a
  pointer to it.  fneval() is called only by zzstring(), which always copies
  the result out of this buffer to somewhere else, so it's OK to have only
  one buffer for this in most cases.  However, since function calls can be
  nested -- e.g. functions whose arguments are functions, or recursive
  functions, at some point we should convert this to an array of buffers,
  indexed by function depth (which might or might not be the same as the
  "depth" variable).  Also, since function results are potentially quite big,
  we'd need to allocate and deallocate dynamically as we descend and ascend
  function depth.  Left for a future release...
*/
char fnval[FNVALL+2];                   /* Function return value  */
static int fndepth = 0;                 /* (we don't actually use this yet) */
int fnsuccess = 1;
extern int fnerror;

/* f p f o r m a t  --  Floating-point number nicely formatted.  */
/*
   Returns results from a circular 1K buffer.
   Don't count on too many results remaining available at once; it could
   be anywhere from 5 to maybe 100, depending on the sizes of the results.
*/
#ifdef CKFLOAT
#define FPFMTSIZ 1024
static char fpfmtbuf[FPFMTSIZ] = { NUL, NUL };
static int fpfbufpos = 0;               /* (why was this char before?) */

char *
fpformat(fpresult,places,round) CKFLOAT fpresult; int places, round; {
    char fbuf[16];                      /* For creating printf format */
    int nines = 0, sign = 0, x, y, i, j, size = 0;
    char * buf;
    CKFLOAT ftmp;

    x = places ? places : (fp_digits ? fp_digits : 6);

    debug(F101,"fpformat fpresult","",fpresult);
    debug(F101,"fpformat places","",places);
    debug(F101,"fpformat fpfbufpos 1","",fpfbufpos);

    ftmp = fpresult;
    if (ftmp < 0.0) ftmp = 0.0 - fpresult;

#ifdef FNFLOAT
    if (!fp_rounding &&                 /* If printf doesn't round, */
        (places > 0 ||                  /* round result to decimal places. */
         (places == 0 && round)))
      fpresult += (0.5 / pow(10.0,(CKFLOAT)places));
    y = (ftmp == 0.0) ? 1 : (int)log10(ftmp);
    size = y + x + 3;                   /* Estimated length of result */
    if (fpresult < 0.0) size++;
#else
    size = 200;                         /* No way to estimate, be generous */
#endif /* FNFLOAT */

    debug(F101,"fpformat size","",size);

    if (fpfbufpos > (FPFMTSIZ - size))  /* Wrap around if necessary */
      fpfbufpos = 0;
    debug(F101,"fpformat fpfbufpos 1","",fpfbufpos);

    buf = &fpfmtbuf[fpfbufpos];

    if (places > 0) {                   /* If places specified */
        /* use specified places to write given number of digits */
        sprintf(fbuf,"%%0.%df",places); /* SAFE */
        sprintf(buf,fbuf,fpresult);     /* SAFE */
    } else {                            /* Otherwise... */
        /* Go for max precision */
        sprintf(fbuf,"%%0.%df",fp_digits); /* SAFE */
        sprintf(buf,fbuf,fpresult);     /* SAFE */
    }
    if (buf[0] == '-') sign = 1;
    debug(F111,"fpresult 1 errno",buf,errno); /* Check for over/underflow */
    debug(F111,"fpresult 1 fpfbufpos",buf,fpfbufpos);
    /* Give requested decimal places */
    for (i = sign; i < FPFMTSIZ && buf[i]; i++) {
        if (buf[i] == '.')              /* First find the decimal point */
          break;
        else if (i > fp_digits + sign - 1) /* replacing garbage */
          buf[i] = '0';                 /* digits with 0... */
    }
    if (buf[i] == '.') {                /* Have decimal point */
        int gotend = 0;
        /* places < 0 so truncate fraction */
        if (places < 0 || (places == 0 && round)) {
            buf[i] = NUL;
        } else if (places > 0) {        /* d > 0 so this many decimal places */
            i++;                           /* First digit after decimal */
            for (j = 0; j < places; j++) { /* Truncate after d decimal */
                if (!buf[j+i])        /* places or extend to d  */
                  gotend = 1;              /* decimal places. */
                if (gotend || j+i+sign > fp_digits)
                  buf[j+i] = '0';
            }
            buf[j+i] = NUL;
        } else {                        /* places == 0 so Do The Right Thing */
            for (j = (int)strlen(buf) - 1; j > i+1; j--) {
                if ((j - sign) > fp_digits)
                  buf[j] = '0';
                if (buf[j] == '0')
                  buf[j] = NUL; /* Strip useless trailing 0's. */
                else
                  break;
            }
        }
    }
    fpfmtbuf[FPFMTSIZ-1] = NUL;
    j = strlen(buf);
    sign = 0;
    for (i = j-1; i >= 0; i--) {
        if (buf[i] == '9')
          nines++;
        else
          break;
    }
    /* Do something about xx.xx99999999... */
    if (nines > 5) {
        if (isdigit(buf[i]) && i < FPFMTSIZ - 2) {
            buf[i] = buf[i] + 1;
            buf[i+1] = '0';
            buf[i+2] = '\0';
        }
    }
    if (!strncmp(buf,"-0.0",FPFMTSIZ))
      ckstrncpy(buf,"0.0",FPFMTSIZ);
    fpfbufpos += (int)strlen(buf) + 1;
    return((char *)buf);
}
#endif /* CKFLOAT */

static VOID
evalerr(fn) char * fn; {
    if (fndiags) {
        if (divbyzero)
          ckmakmsg(fnval,FNVALL,"<ERROR:DIVIDE_BY_ZERO:\\f",fn,"()>",NULL);
        else
          ckmakmsg(fnval,FNVALL,"<ERROR:ARG_NOT_NUMERIC:\\f",fn,"()>",NULL);
    }
}


static int
ckcindex(c,s) char c, *s; {
    int rc;
    if (!c || !s) return(0);
    for (rc = 0; s[rc]; rc++) {
	if (c == s[rc]) return(rc+1);
    }
    return(0);
}

static char *
dokwval(s,sep) char * s, * sep; {
    char c = '\0', * p, * kw = NULL, * vp = NULL;
    char * rc = "0";			/* Return code */
    int x = 0;
    if (!s) return(rc);
    if (!*s) return(rc);
    debug(F110,"kwval arg",s,0);
    debug(F110,"kwval sep",sep,0);
    p = (char *)malloc((int)strlen(s)+1);
    if (!p) goto xdokwval;
    strcpy(p,s);                        /* SAFE */
    s = p;
    while (*s < '!' && *s > '\0')       /* Get first nonblank */
      s++;
    if (!*s) goto xdokwval;
    if (ckcindex(*s,sep))		/* Separator but no keyword */
      goto xdokwval;
    kw = s;                             /* Keyword */
    while (*s > ' ') {
	if (ckcindex(*s,sep)) {		/* keyword=... */
            c = *s;
            break;
        }
        s++;
    }
    if (*kw) rc = "1";			/* Have keyword, promote return code */
    *s++ = NUL;                         /* Terminate keyword */
    while (*s < '!' && *s > '\0')       /* Skip blanks */
      s++;
    if (!c && ckcindex(*s,sep)) {
        c = *s++;                       /* Have separator */
        while (*s < '!' && *s > '\0')   /* Skip blanks */
          s++;
    }
    if (c) {
        vp = s;
	if (*vp) rc = "2";		/* Have value, another promotion */
#ifdef COMMENT
        while (*s > ' ')                /* Skip to end */
          s++;
        *s = NUL;                       /* Terminate value */
#endif	/* COMMENT */
    }
    debug(F110,"kwval c",ckctoa(c),0);
    debug(F110,"kwval keyword",kw,0);
    debug(F110,"kwval value",vp,0);
    makestr(&lastkwval,kw);
    vp = brstrip(vp);
    debug(F110,"kwval value",vp,0);
    x = addmac(kw,vp);
    debug(F111,"kwval addmac",kw,x);
  xdokwval: 
    if (p) free(p);
    return((x < 0) ? "-1" : rc);
}

static int
isaarray(s) char * s; {			/* Is s an associative array element */
    int state = 0;
    CHAR c;
    if (!s) return(0);
    while ((c = *s++)) {
	if (!isprint(c)) {
	    return(0);
	} else if (c == '<') {
	    if (state != 0)
	      return(0);
	    state = 1;
	} else if (c == '>') {
	    return((state != 1 || *s) ? 0 : 1);
	}
    }
    return(0);
}

static char *                           /* Evaluate builtin functions */
fneval(fn,argp,argn,xp) char *fn, *argp[]; int argn; char * xp; {
    int i=0, j=0, k=0, len1=0, len2=0, len3=0, n=0, t=0, x=0, y=0;
    int cx, failed = 0;                 /* Return code, 0 = ok */
    long z = 0L;
    char *bp[FNARGS + 1];               /* Pointers to malloc'd strings */
    char c = NUL;
    char *p = NULL, *s = NULL;
    char *val1 = NULL, *val2 = NULL;    /* Pointers to numeric string values */

#ifdef RECURSIVE
    int rsave = recursive;
#endif /* RECURSIVE */
#ifdef OS2
    int zsave = zxpn;
#endif /* OS2 */

    if (!fn) fn = "";                   /* Protect against null pointers */
    if (!*fn) return("");

    for (i = 0; i < FNARGS; i++)        /* Initialize argument pointers */
      bp[i] = NULL;
/*
  IMPORTANT: Note that argn is not an accurate count of the number of
  arguments.  We can't really tell if an argument is null until after we
  execute the code below.  So argn is really the maximum number of arguments
  we might have.  Argn should always be at least 1, even if the function is
  called with empty parentheses (but don't count on it).
*/
    debug(F111,"fneval",fn,argn);
    debug(F110,"fneval",argp[0],0);
    if (argn > FNARGS)                  /* Discard excess arguments */
      argn = FNARGS;

    fndepth++;
    debug(F101,"fneval fndepth","",fndepth);
    p = fnval;
    fnval[0] = NUL;
    y = lookup(fnctab,fn,nfuncs,&x);    /* Look up the function name */
    cx = y;                             /* Because y is too generic... */
    if (cx < 0) {                        /* Not found */
        failed = 1;
        if (fndiags) {                  /* FUNCTION DIAGNOSTIC ON */
            int x;
            x = strlen(fn);
            /* The following sprintf's are safe */
            switch (cx) {
              case -1:
                if (x + 32 < FNVALL)
                  sprintf(fnval,"<ERROR:NO_SUCH_FUNCTION:\\f%s()>",fn);
                else
                  sprintf(fnval,"<ERROR:NO_SUCH_FUNCTION>");
                break;
              case -2:
                if (x + 26 < FNVALL)
                  sprintf(fnval,"<ERROR:NAME_AMBIGUOUS:\\f%s()>",fn);
                else
                  sprintf(fnval,"<ERROR:NAME_AMBIGUOUS>");
                break;
              case -3:
                sprintf(fnval,"<ERROR:FUNCTION_NAME_MISSING:\\f()>");
                break;
              default:
                if (x + 26 < FNVALL)
                  sprintf(fnval,"<ERROR:LOOKUP_FAILURE:\\f%s()>",fn);
                else
                  sprintf(fnval,"<ERROR:LOOKUP_FAILURE>");
                break;
            }
        }
        goto fnend;                     /* Always leave via common exit */
    }
    fn = fnctab[x].kwd;                 /* Full name of function */

    if (argn < 0) {
        failed = 1;
        p = fnval;
        if (fndiags)
          sprintf(fnval,"<ERROR:MISSING_ARG:\\f%s()>",fn);
        goto fnend;
    }
    if (cx == FN_LIT) {                 /* literal(arg1) */
        debug(F010,"flit",xp,0);
        p = xp ? xp : "";               /* Return a pointer to arg itself */
        goto fnend;
    }

#ifdef DEBUG
    if (deblog) {
        int j;
        for (j = 0; j < argn; j++)
	  debug(F111,"fneval arg",argp[j],j);
    }
#endif /* DEBUG */
    for (j = argn-1; j >= 0; j--) {     /* Uncount empty trailing args */
        if (!argp[j])
          argn--;
        else if (!*(argp[j]))
          argn--;
        else break;
    }
    debug(F111,"fneval argn",fn,argn);
/*
  \fliteral() and \fcontents() are special functions that do not evaluate
  their arguments, and are treated specially here.  After these come the
  functions whose arguments are evaluated in the normal way.
*/
#ifdef COMMENT
    /* (moved up) */
    if (cx == FN_LIT) {                 /* literal(arg1) */
        debug(F110,"flit",xp,0);
        p = xp ? xp : "";               /* Return a pointer to arg itself */
        goto fnend;
    }
#endif /* COMMENT */
    if (cx == FN_CON) {                 /* Contents of variable, unexpanded. */
        char c;
        int subscript = 0;        
        if (!(p = argp[0]) || !*p) {
            failed = 1;
            p = fnval;
            if (fndiags)
              sprintf(fnval,"<ERROR:MISSING_ARG:\\fcontents()>");
            goto fnend;
        }
        p = brstrip(p);
        if (*p == CMDQ) p++;
        if ((c = *p) == '%') {          /* Scalar variable. */
            c = *++p;                   /* Get ID character. */
            p = "";                     /* Assume definition is empty */
            if (!c) {                   /* Double paranoia */
                failed = 1;
                p = fnval;
                if (fndiags)
                  sprintf(fnval,"<ERROR:ARG_BAD_VARIABLE:\\fcontents()>");
                goto fnend;
            }
            if (c >= '0' && c <= '9') { /* Digit for macro arg */
                if (maclvl < 0)         /* Digit variables are global */
                  p = g_var[c];         /* if no macro is active */
                else                    /* otherwise */
                  p = m_arg[maclvl][c - '0']; /* they're on the stack */
            } else if (c == '*') {
#ifdef COMMENT
                p = (maclvl > -1) ? m_line[maclvl] : topline;
                if (!p) p = "";
#else
                int nx = FNVALL;
                char * sx = fnval;
                p = fnval;
#ifdef COMMENT
                if (cmdsrc() == 0 && topline)
                  p = topline;
                else
#endif /* COMMENT */
                  if (zzstring("\\fjoin(&_[],{ },1)",&sx,&nx) < 0) {
                    failed = 1;
                    p = fnval;
                    if (fndiags)
                      sprintf(fnval,"<ERROR:OVERFLOW:\\fcontents()>");
		    debug(F110,"zzstring fcontents(\\%*)",p,0);
                }
#endif /* COMMENT */
            } else {
                if (isupper(c)) c -= ('a'-'A');
                p = g_var[c];           /* Letter for global variable */
            }
            if (!p) p = "";
            goto fnend;
        } else if (c == '&') {          /* Array reference. */
            int vbi, d;
            if (arraynam(p,&vbi,&d) < 0) { /* Get name and subscript */
                failed = 1;
                p = fnval;
                if (fndiags)
                  sprintf(fnval,"<ERROR:ARG_BAD_ARRAY:\\fcontents()>");
                goto fnend;
            }
	    subscript = chkarray(vbi,d); /* Check the array */
            if (subscript >= 0) {	/* Array is declared? */
                vbi -= ARRAYBASE;       /* Convert name to index */
                if (a_dim[vbi] >= d) {  /* If subscript in range */
                    char **ap;
                    ap = a_ptr[vbi];    /* get data pointer */
                    if (ap) {           /* and if there is one */
                        p = ap[d];	/* return it */
                        goto fnend;
                    }
                }
            } else {			/* Array not declared or element */
                fnval[0] = NUL;		/* out of range - return null string */
                p = fnval;		/* fdc 2010-12-30 */
                goto fnend;	
            }
        } else {
            failed = 1;
            p = fnval;
            if (fndiags)
              sprintf(fnval,"<ERROR:ARG_NOT_VARIABLE:\\fcontents()>");
            goto fnend;
        }
    }
    p = fnval;                          /* Default result pointer */
    fnval[0] = NUL;                     /* Default result = empty string */

    for (i = 0; i < argn; i++) {        /* Loop to expand each argument */
        n = MAXARGLEN;                  /* Allow plenty of space */
        bp[i] = s = malloc(n+1);        /* Allocate space for this argument */
        if (bp[i] == NULL) {            /* Handle failure to get space */
            failed = 1;
            if (fndiags)
              ckmakmsg(fnval,FNVALL,"<ERROR:MALLOC_FAILURE:\\f",fn,"()>",NULL);
            goto fnend;
        }
        p = argp[i] ? argp[i] : "";     /* Point to this argument */

/*
  Trim leading and trailing spaces from the original argument, before
  evaluation.  This code new to edit 184.  Fixed in edit 199 to trim
  blanks BEFORE stripping braces.

*/
        {
            int x, j;
            x = strlen(p);
            j = x - 1;                  /* Trim trailing whitespace */
            while (j > 0 && (*(p + j) == SP || *(p + j) == HT))
              *(p + j--) = NUL;
            while (*p == SP || *p == HT) /* Strip leading whitespace */
              p++;
            x = strlen(p);
            if (*p == '{' && *(p+x-1) == '}') { /* NOW strip braces */
                p[x-1] = NUL;
                p++;
                x -= 2;
            }
        }

/* Now evaluate the argument */

        debug(F111,"fneval calling zzstring",p,n);
        t = zzstring(p,&s,&n);          /* Expand arg into new space */
        debug(F101,"fneval zzstring","",t);
        debug(F101,"fneval zzstring","",n);
        if (t < 0) {
            debug(F101,"fneval zzstring fails, arg","",i);
            failed = 1;
            if (fndiags) {
                if (n == 0)
                  ckmakmsg(fnval,FNVALL,
                           "<ERROR:ARG_TOO_LONG:\\f",fn,"()>",NULL);
                else
                  ckmakmsg(fnval,FNVALL,
                           "<ERROR:ARG_EVAL_FAILURE:\\f",fn,"()>",NULL);
            }
            goto fnend;
        }
        debug(F111,"fneval arg",bp[i],i);
    }

#ifdef DEBUG
    if (deblog) {
        int j;
        for (j = 0; j < argn; j++) {
            debug(F111,"fneval arg post eval",argp[j],j);
            debug(F111,"fneval evaluated arg",bp[j],j);
        }
    }
#endif /* DEBUG */
    {
	/* Adjust argn for empty trailing arguments. */
	/* For example when an arg is a variable name but the */
	/* variable has no value.   July 2006. */
	int j, old; char *p;
	old = argn;
	for (j = argn - 1; j >= 0; j--) {
	    p = bp[j];
	    if (!p)
	      argn--;
	    else if (!*p)
	      argn--;
	    else
	      break;
	}
#ifdef DEBUG
	if (argn != old)
	  debug(F101,"fneval adjusted argn","",argn);
#endif	/* DEBUG */
    }	

/*
  From this point on, bp[0..argn-1] are not NULL and all must be freed
  before returning.
*/
    if (argn < 1) {                     /* Catch required args missing */
        switch (cx) {
          case FN_DEF:
          case FN_EVA:
          case FN_EXE:
          case FN_CHR:
          case FN_COD:
          case FN_MAX:
          case FN_MIN:
          case FN_MOD:
          case FN_FD:
          case FN_FS:
          case FN_TOD:
          case FN_FFN:
          case FN_BSN:
          case FN_RAW:
          case FN_CMD:
          case FN_2HEX:
          case FN_2OCT:
          case FN_DNAM:
#ifdef FN_ERRMSG
          case FN_ERRMSG:
#endif /* FN_ERRMSG */
#ifdef CK_KERBEROS
          case FN_KRB_TK:
          case FN_KRB_NX:
          case FN_KRB_IV:
          case FN_KRB_TT:
          case FN_KRB_FG:
#endif /* CK_KERBEROS */
          case FN_MJD2:
          case FN_N2TIM:
          case FN_DIM:
          case FN_DATEJ:
          case FN_PNCVT:
          case FN_PERM:
          case FN_ALOOK:
          case FN_TLOOK:
          case FN_ABS:
          case FN_AADUMP:
          case FN_JOIN:
#ifdef CKFLOAT
          case FN_FPABS:
          case FN_FPEXP:
          case FN_FPLOG:
          case FN_FPLN:
          case FN_FPMOD:
          case FN_FPSQR:
          case FN_FPADD:
          case FN_FPDIV:
          case FN_FPMUL:
          case FN_FPPOW:
          case FN_FPSUB:
          case FN_FPINT:
          case FN_FPROU:
          case FN_FPSIN:
          case FN_FPCOS:
          case FN_FPTAN:
#endif /* CKFLOAT */
#ifdef TCPSOCKET
          case FN_HSTADD:
          case FN_HSTNAM:
#endif /* TCPSOCKET */
          case FN_DELSEC:
#ifdef COMMENT
          case FN_KWVAL:
          case FN_SLEEP:
          case FN_MSLEEP:
#endif /* COMMENT */
#ifdef NT
          case FN_SNAME:
          case FN_LNAME:
#endif /* NT */
            failed = 1;
            p = fnval;
            if (fndiags)
              ckmakmsg(fnval,FNVALL,"<ERROR:MISSING_ARG:\\f",fn,"()>",NULL);
            goto fnend;
        }
    }
    p = fnval;                          /* Reset these again. */
    fnval[0] = NUL;

    switch (cx) {                       /* Do function on expanded args. */
#ifdef TCPSOCKET
      case FN_HSTADD:
        p = ckaddr2name(bp[0]);
        goto fnend;
      case FN_HSTNAM:
        p = ckname2addr(bp[0]);
        goto fnend;
#endif /* TCPSOCKET */

      case FN_DEF:                      /* \fdefinition(arg1) */
        k = isaarray(bp[0]) ?
	    mxxlook(mactab,bp[0],nmac) :
		mxlook(mactab,bp[0],nmac);
        p = (k > -1) ? mactab[k].mval : "";
        goto fnend;

      case FN_EVA:                      /* \fevaluate(arg1) */
        p = *(bp[0]) ? evalx(bp[0]) : "";
        if (!*p && fndiags) {
            failed = 1;
            p = fnval;
            evalerr(fn);
        }
        goto fnend;

      case FN_EXE:                      /* \fexecute(arg1) */
        j = (int)strlen(s = bp[0]);     /* Length of macro invocation */
        p = "";                         /* Initialize return value to null */
        if (j) {                        /* If there is a macro to execute */
            while (*s == SP) s++,j--;   /* strip leading spaces */
            p = s;                      /* remember beginning of macro name */
            for (i = 0; i < j; i++) {   /* find end of macro name */
                if (*s == SP)
                  break;
                s++;
            }
            if (*s == SP)       {       /* if there was a space after */
                *s++ = NUL;             /* terminate the macro name */
                while (*s == SP) s++;   /* skip past any extra spaces */
            } else
              s = "";                   /* maybe there are no arguments */
            if (p && *p) {
                k = mlook(mactab,p,nmac); /* Look up the macro name */
                debug(F111,"fexec mlook",p,k);
            } else
              k = -1;
            if (k < 0) {
                char * p2 = p;
                failed = 1;
                p = fnval;
                if (fndiags)
                  ckmakxmsg(fnval,FNVALL,
                            "<ERROR:NO_SUCH_MACRO:\\f",fn,"(",p2,")>",
                            NULL,NULL,NULL,NULL,NULL,NULL,NULL);
                goto fnend;
            }
/*
  This is just a WEE bit dangerous because we are copying up to 9 arguments
  into the space reserved for one.  It won't overrun the buffer, but if there
  are lots of long arguments we might lose some.  The other problem is that if
  the macro has more than 3 arguments, the 4th through last are all
  concatenated onto the third.  (The workaround is to use spaces rather than
  commas to separate them.)  Leaving it like this to avoid having to allocate
  tons more buffers.
*/
            if (argn > 1) {             /* Commas used instead of spaces */
                int i;
                char *p = bp[0];        /* Reuse this space */
                *p = NUL;               /* Make into dodo() arg list */
                for (i = 1; i < argn; i++) {
                    ckstrncat(p,bp[i],MAXARGLEN);
                    ckstrncat(p," ",MAXARGLEN);
                }
                s = bp[0];              /* Point to new list */
            }
            p = "";                     /* Initialize return value */
            if (k >= 0) {               /* If macro found in table */
                /* Go set it up (like DO cmd) */
                if ((j = dodo(k,s,cmdstk[cmdlvl].ccflgs)) > 0) {
                    if (cmpush() > -1) { /* Push command parser state */
                        extern int ifc;
                        int ifcsav = ifc; /* Push IF condition on stack */
                        k = parser(1);  /* Call parser to execute the macro */
                        cmpop();        /* Pop command parser */
                        ifc = ifcsav;   /* Restore IF condition */
                        if (k == 0) {   /* No errors, ignore action cmds. */
                            p = mrval[maclvl+1]; /* If OK, set return value. */
                            if (p == NULL) p = "";
                        }
                    } else {            /* Can't push any more */
                        debug(F100,"zzstring fneval fexec failure","",0);
                        printf("\n?\\fexec() too deeply nested\n");
                        while (cmpop() > -1) ;
                        p = "";
                    }
                }
            }
        }
        debug(F110,"zzstring fneval fexecute final p",p,0);
        goto fnend;

#ifdef RECURSIVE
      case FN_RDIR:                     /* \frdir..() - Recursive dir count */
      case FN_RFIL:                     /* \frfiles() - Recursive file count */
        /* recursive = 2; */            /* fall thru... */
#endif /* RECURSIVE */
      case FN_FC:                       /* \ffiles() - File count. */
      case FN_DIR: {                    /* \ffdir.() - Directory count. */
          char abuf[16], *s;
          char ** ap = NULL;
          int x, xflags = 0;
          if (matchdot)
            xflags |= ZX_MATCHDOT;
          if (cx == FN_RDIR || cx == FN_RFIL) {
              xflags |= ZX_RECURSE;
#ifdef CKSYMLINK
              /* Recursive - don't follow symlinks */
              xflags |= ZX_NOLINKS;
#endif /* CKSYMLINK */
          }
          failed = 0;
          if (argn < 1) {
              p = "0";
              goto fnend;
          }
          if (cx == FN_DIR || cx == FN_RDIR) { /* Only list directories */
	      debug(F100,"FN_DIR or FN_RDIR","",0);
              xflags |= ZX_DIRONLY;
#ifdef OS2
              zxpn = 1;                 /* Use the alternate list */
#endif /* OS2 */
          } else {                      /* List only files */
	      debug(F100,"Not FN_DIR or FN_RDIR","",0);
              xflags |= ZX_FILONLY;
#ifdef OS2
              zxpn = 1;                 /* Use the alternate list */
#endif /* OS2 */
          }
          if (*(bp[0])) {
              k = nzxpand(bp[0],xflags);
              if (k < 0) k = 0;
              sprintf(fnval,"%d",k);    /* SAFE */
              p = fnval;
          } else
            p = "0";

          if (argn > 1) {               /* Assign list to array */
              fnval[0] = NUL;           /* Initial return value */
              ckstrncpy(abuf,bp[1],16); /* Get array reference */
              s = abuf;
              if (*s == CMDQ) s++;
              failed = 1;               /* Assume it's bad */
              p = fnval;                /* Point to result */
              if (fndiags)              /* Default is this error message */
                ckmakmsg(fnval,FNVALL,
                         "<ERROR:ARG_BAD_ARRAY:\\f",fn,"()>",NULL);
              if (s[0] != '&')          /* "Address" of array */
                goto fnend;
              if (s[2])
                if (s[2] != '[' || s[3] != ']')
                  goto fnend;
              if (s[1] >= 64 && s[1] < 91) /* Convert upper to lower */
                s[1] += 32;
              if ((x = dclarray(s[1],k)) < 0) /* File list plus count */
                goto fnend;
              failed = 0;               /* Unset failure flag */
              ap = a_ptr[x];            /* Point to array we just declared */
              sprintf(fnval,"%d",k);    /* SAFE */
          }
#ifdef OS2
          if (ap) {                     /* We are making an array */
              int i;
              char tmp[16];
              ap[0] = NULL;             /* Containing number of files    */
              makestr(&(ap[0]),ckitoa(k));

              ckstrncpy(tmp,fnval,16);  /* Save return value */

              for (i = 1; i <= k; i++) { /* Fill it */
                  ap[i] = NULL;
                  znext(fnval);         /* Next filename */
                  if (!*fnval)          /* No more, done */
                    break;              /* In case a premature end */
                  makestr(&(ap[i]),fnval);
              }
#ifdef ZXREWIND
              k = zxrewind();           /* Reset the file expansion */
#else
              k = nzxpand(bp[0],xflags);
#endif /* ZXREWIND */
              ckstrncpy(fnval,tmp,FNVALL); /* Restore return value */
          }
#else /* OS2 */
          {                             /* Make copies of the list */
              int i; char tmp[16];
              if (flist) {              /* Free old file list, if any */
                  for (i = 0; flist[i]; i++) { /* and each string */
                      free(flist[i]);
                      flist[i] = NULL;
                  }
                  free((char *)flist);
              }
              ckstrncpy(tmp,fnval,16);  /* Save our return value */
              flist = (char **) malloc((k+1) * sizeof(char *)); /* New array */
              if (flist) {
                  for (i = 0; i <= k; i++) { /* Fill it */
                      flist[i] = NULL;
                      znext(fnval);     /* Next filename */
                      if (!*fnval)      /* No more, done */
                        break;
                      makestr(&(flist[i]),fnval);
                  }
                  if (ap) {             /* If array pointer given */
                      ap[0] = NULL;
                      makestr(&(ap[0]),ckitoa(k));
                      for (i = 0; i < k; i++) { /* Copy file list to array */
                          ap[i+1] = NULL;
                          makestr(&(ap[i+1]),flist[i]);
                      }
                  }
              }
              ckstrncpy(fnval,tmp,FNVALL); /* Restore return value */
              flistn = 0;               /* Reset global list pointer */
          }
#endif /* OS2 */
#ifdef RECURSIVE
          recursive = rsave;
#endif /* RECURSIVE */
#ifdef OS2
          zxpn = zsave;
#endif /* OS2 */
      }
      goto fnend;

      case FN_FIL:                      /* \fnextfile() - Next file in list. */
        p = fnval;                      /* (no args) */
        *p = NUL;
#ifdef OS2
        zxpn = 1;                       /* OS/2 - use the alternate list */
        znext(p);                       /* Call system-dependent function */
        zxpn = zsave;                   /* Restore original list */
#else
        if (flist)                      /* Others, use our own list. */
          if (flist[flistn])
	    p = flist[flistn++];
#endif /* OS2 */
        goto fnend;

    } /* Break up big switch... */

    switch (cx) {
      case FN_IND:                      /* \findex(s1,s2,start,occurrence) */
      case FN_RIX:                      /* \frindex(s1,s2,start,occurrence) */
      case FN_SEARCH:                   /* \fsearch(pat,string,start,occ) */
      case FN_RSEARCH:			/* \frsearch(pat,string,start,occ) */
      case FN_COUNT: {			/* \fcount(s1,s2,start) */
	int i = 0, right = 0, search = 0, count = 0;
	int desired = 1;
        right = (cx == FN_RIX || cx == FN_RSEARCH);
        search = (cx == FN_SEARCH || cx == FN_RSEARCH);
	count = (cx == FN_COUNT);
        p = "0";
        if (argn > 1) {                 /* Only works if we have 2 or 3 args */
            int start = 0;
            char * pat = NULL;
            len1 = (int)strlen(pat = bp[0]); /* length of string to look for */
            len2 = (int)strlen(s = bp[1]); /* length of string to look in */
            if (len1 < 1 || len2 < 1)   /* Watch out for empty strings */
              goto fnend;
            start = right ? -1 : 0;     /* Default starting position */
            if (argn > 2) {
                val1 = *(bp[2]) ? evalx(bp[2]) : "1";
		if (argn > 3) {
		    val2 = *(bp[3]) ? evalx(bp[3]) : "1";
		    if (chknum(val2)) desired = atoi(val2);
		    if (desired * len1 > len2) goto fnend;
		}
                if (chknum(val1)) {
                    int t;
                    t = atoi(val1);
                    if (!search) {      /* Index or Rindex */
                        j = len2 - len1; /* Length difference */
                        t--;             /* Convert position to 0-based */
                        if (t < 0) t = 0;
                        start = t;
                        if (!right && start < 0) start = 0;
                    } else {            /* Search or Rsearch */
                        int x;
                        if (t < 0) t = 0;
                        if (right) {    /* Right to left */
                            if (t > len2) t = len2;
                            start = len2 - t - 1;
                            if (start < 0)
                              goto fnend;
                            x = len2 - t;
                            s[x] = NUL;
                        } else {        /* Left to right */
                            start = t - 1;
                            if (start < 0) start = 0;
                            if (start >= len2)
                              goto fnend;
                        }
                    }
                } else {
                    failed = 1;
                    evalerr(fn);
                    p = fnval;
                    goto fnend;
                }
            }
	    if (count) {		/* \fcount() */
		int j;
		for (i = 0; start < len2; i++) {
		    j = ckindex(pat,bp[1],start,0,inpcas[cmdlvl]);
		    if (j == 0) break;
		    start = j;
		}

	    } else if (search) {	/* \fsearch() or \frsearch() */

                if (right && pat[0] == '^') {
                    right = 0;
                    start = 0;
                }
                if (right) {		/* From right */
		    int k, j = 1;
                    if (start < 0)
		      start = len2 - 1;
		    i = 0;
		    while (start >= 0 && j <= desired) {
			for (i = start;
			     (i >= 0) && 
				 !(k = ckmatch(pat,s+i,inpcas[cmdlvl],1+4));
			     i--) ;
			if (k < 1) {	/* No match */
			    i = 0;
			    break;
			}
			if (j == desired) { /* The match we want? */
			    i += k;	/* Yes, return string index */
			    break;
			}
			j++;		/* No, count this match */
			s[i] = NUL;	/* null it out */
			start = i-1;	/* move left and look again */
		    }

                } else {		/* From left */
		    int j;
		    i = 0;
		    for (j = 1; j <= desired && start < len2; j++) {
			i = ckmatch(pat,&s[start],inpcas[cmdlvl],1+4);
			if (i == 0 || j == desired) break;
			start += i + 1;
		    }			
		    if (j == desired && i != 0)
		      i += start;
		    else
		      i = 0;
                }
            } else {			/* index or rindex */
		int j = 0;
		i = 0;
		for (j = 1; j <= desired && start < len2; j++) {
		    i = ckindex(pat,bp[1],start,right,inpcas[cmdlvl]);
		    if (i == 0 || j == desired) break;
		    start = (right) ? len2 - i + 1 : i;
		}
		if (j != desired)
		  i = 0;
            }
            sprintf(fnval,"%d",i);      /* SAFE */
            p = fnval;
        }
        goto fnend;
      }

      case FN_RPL:                      /* \freplace(s1,s2,s3) */
      /*
        s = bp[0] = source string
            bp[1] = match string
            bp[2] = replacement string
            bp[3] = which occurrence (default = all);
        p = fnval = destination (result) string
      */
        if (argn < 1)                   /* Nothing */
          goto fnend;
        if (argn < 2) {                 /* Only works if we have 2 or 3 args */
            ckstrncpy(p,bp[0],FNVALL);
        } else {
            int occur = 0, xx = 0, j2;
            len1 = (int)strlen(bp[0]);  /* length of string to look in */
            len2 = (int)strlen(bp[1]);  /* length of string to look for */
            len3 = (argn < 3) ? 0 : (int)strlen(bp[2]); /* Len of replacemnt */
            j = len1 - len2 + 1;
            j2 = j;
            if (argn > 3) {
                if (chknum(bp[3])) {
                    occur = atoi(bp[3]);
                } else {
                    failed = 1;
                    if (fndiags)
                      ckmakmsg(fnval,FNVALL,
                               "<ERROR:ARG_NOT_NUMERIC:\\f",fn,"()>",NULL);
                    goto fnend;
                }
            }
            /* If args out of whack... */
            if (j < 1 || len1 == 0 || len2 == 0) {
                ckstrncpy(p,bp[0],FNVALL); /* just return original string */
                p[FNVALL] = NUL;
            } else {
              ragain:
                s = bp[0];              /* Point to beginning of string */
                while (j-- > 0) {       /* For each character */
                    if (!ckstrcmp(bp[1],s,len2,inpcas[cmdlvl]) &&
                        (occur == 0 || occur == ++xx)) {
                        if (len3) {
                            ckstrncpy(p,bp[2],FNVALL);
                            p += len3;
                        }
                        s += len2;      /* and skip past it. */
                    } else {            /* No, */
                        *p++ = *s++;    /* just copy this character */
                    }
                }
                *p = NUL;
                while ((*p++ = *s++));
                if (occur < 0) {        /* cheap... */
                    occur = xx + occur + 1;
                    xx = 0;
                    p = fnval;
                    j = j2;
                    if (occur > 0)
                      goto ragain;
                }
            }
        }
        p = fnval;
        goto fnend;

      case FN_CHR:                      /* \fcharacter(arg1) */
        val1 = *(bp[0]) ? evalx(bp[0]) : "";
        if (chknum(val1)) {             /* Must be numeric */
            i = atoi(val1);
            if (i >= 0 && i < 256) {    /* Must be an 8-bit value */
                p = fnval;
                *p++ = (char) i;
                *p = NUL;
                p = fnval;
            } else {
                failed = 1;
                if (fndiags)
                  ckmakmsg(fnval,FNVALL,
                           "<ERROR:ARG_OUT_OF_RANGE:\\f",fn,"()>",NULL);
            }
        } else {
            failed = 1;
            evalerr(fn);
        }
        goto fnend;

      case FN_COD:                      /* \fcode(char) */
        if ((int)strlen(bp[0]) > 0) {
            p = fnval;
            i = *bp[0];
            sprintf(p,"%d",(i & 0xff)); /* SAFE */
        } else p = "0";			/* Can't happen */
        goto fnend;

      case FN_LEN:                      /* \flength(arg1) */
        if (argn > 0) {
            p = fnval;
            sprintf(p,"%d",(int)strlen(bp[0])); /* SAFE */
        } else p = "0";
        goto fnend;

      case FN_LOW:                      /* \flower(arg1) */
        s = bp[0] ? bp[0] : "";
        p = fnval;
        while (*s) {
            if (isupper(*s))
              *p = (char) tolower(*s);
            else
              *p = *s;
            p++; s++;
        }
        *p = NUL;
        p = fnval;
        goto fnend;

      case FN_MAX:                      /* \fmax(arg1,arg2) */
      case FN_MIN:                      /* \fmin(arg1,arg2) */
      case FN_MOD:                      /* \fmod(arg1,arg2) */
        val1 = *(bp[0]) ? evalx(bp[0]) : "";
#ifdef COMMENT
        /* No longer necessary because evalx() no longer overwrites its */
        /* result every time it's called (2000/09/23). */
        free(bp[0]);
        bp[0] = NULL;
#endif /* COMMENT */
        if (argn > 1) {
#ifdef COMMENT
            /* Ditto... */
            bp[0] = malloc((int)strlen(val1)+1);
            if (bp[0])
              strcpy(bp[0],val1);       /* safe */
            val1 = bp[0];
#endif /* COMMENT */
            val2 = *(bp[1]) ? evalx(bp[1]) : "";
            if (chknum(val1) && chknum(val2)) {
                i = atoi(val1);
                j = atoi(val2);
                switch (y) {
                  case FN_MAX:
                    if (j < i) j = i;
                    break;
                  case FN_MIN:
                    if (j > i) j = i;
                    break;
                  case FN_MOD:
                    if (j == 0) {
                        failed = 1;
                        if (fndiags)
                          ckmakmsg(fnval,FNVALL,
                                   "<ERROR:DIVIDE_BY_ZERO:\\f",fn,"()>",NULL);
                        else
                          fnval[0] = NUL;
                        goto fnend;
                    } else
                      j = i % j;
                }
                p = fnval;
                sprintf(p,"%d",j);      /* SAFE */
            } else {
                failed = 1;
                evalerr(fn);
            }
        } else p = val1;
        goto fnend;
    } /* Break up big switch... */

    switch (y) {
      case FN_SUB:                      /* \fsubstr(arg1,arg2,arg3) */
      case FN_RIG:                      /* \fright(arg1,arg2) */
      case FN_LEF:                      /* \fleft(arg1,arg2) */
        if (argn < 1)
          goto fnend;
        val1 = "";
        if (argn > 1)
          if (*(bp[1]))
            val1 =  evalx(bp[1]);
#ifdef COMMENT
        if (bp[1]) free(bp[1]);         /* Have to copy this */
        bp[1] = malloc((int)strlen(val1)+1);
        if (!bp[1]) {
            failed = 1;
            if (fndiags) {
                p = fnval;
                ckmakmsg(fnval,FNVALL,
                         "<ERROR:MALLOC_FAILURE:\\f",fn,"()>",NULL);
            }
            goto fnend;
        }
        strcpy(bp[1],val1);             /* safe */
        val1 = bp[1];
#endif /* COMMENT */
        val2 = "";
        if (argn > 2)
          if (*(bp[2]))
            val2 = evalx(bp[2]);
        if (
            ((argn > 1) && (int)strlen(val1) && !rdigits(val1)) ||
            ((cx == FN_SUB) &&
              ((argn > 2) && (int)strlen(val2) && !rdigits(val2)))
            ) {
            failed = 1;
            evalerr(fn);
        } else {
            int lx;
            p = fnval;                  /* pointer to result */
            lx = strlen(bp[0]);         /* length of arg1 */
            if (cx == FN_SUB) {         /* substring */
                k = (argn > 2) ? atoi(val2) : MAXARGLEN; /* length */
                j = (argn > 1) ? atoi(val1) : 1; /* start pos for substr */
            } else if (cx == FN_LEF) {  /* left */
                k = (argn > 1) ? atoi(val1) : lx;
                j = 1;
            } else {                             /* right */
                k = (argn > 1) ? atoi(val1) : lx; /* length */
                j = lx - k + 1;                  /* start pos for right */
                if (j < 1) j = 1;
            }
            if (k > 0 && j <= lx) {              /* if start pos in range */
                s = bp[0]+j-1;                   /* point to source string */
                for (i = 0; (i < k) && (*p++ = *s++); i++) ;  /* copy */
            }
            *p = NUL;                   /* terminate the result */
            p = fnval;                  /* and point to it. */
        }
        goto fnend;

      case FN_UPP:                      /* \fupper(arg1) */
        s = bp[0] ? bp[0] : "";
        p = fnval;
        while (*s) {
            if (islower(*s))
              *p = (char) toupper(*s);
            else
              *p = *s;
            p++; s++;
        }
        *p = NUL;
        p = fnval;
        goto fnend;

      case FN_REP:                      /* \frepeat(text,number) */
        if (argn < 1)
          goto fnend;
        val1 = "1";
        if (argn > 1)
          if (*(bp[1]))
            val1 = evalx(bp[1]);
        if (chknum(val1)) {             /* Repeat count */
            n = atoi(val1);
            debug(F111,"SUNDAY frepeat n",val1,n);
            if (n > 0) {                /* Make n copies */
                p = fnval;
                *p = '\0';
                k = (int)strlen(bp[0]); /* Make sure string has some length */
                debug(F111,"SUNDAY frepeat k","",k);
                debug(F111,"SUNDAY frepeat FNVALL","",FNVALL);
                if (k * n >= FNVALL) {  /* But not too much... */
                    failed = 1;
                    if (fndiags)
                      ckmakmsg(fnval,FNVALL,
                               "<ERROR:RESULT_TOO_LONG:\\f",fn,"()>",NULL);
                    else
                      fnval[0] = NUL;
                    p = fnval;
                    goto fnend;
                }
                if (k > 0) {            /* If there is something to copy */
                    for (i = 0; i < n; i++) { /* Copy loop */
                        s = bp[0];
                        for (j = 0; j < k; j++) {
                            if ((p - fnval) >= FNVALL)
                              break;    /* shouldn't happen... */
                            else
                              *p++ = *s++;
                        }
                    }
                    *p = NUL;
                }
            }
        } else {
            failed = 1;
            evalerr(fn);
        }
        p = fnval;
        goto fnend;

#ifndef NOFRILLS
      case FN_REV:                      /* \freverse() */
        if (argn < 1)
          goto fnend;
        yystring(bp[0],&p);
        goto fnend;
#endif /* NOFRILLS */

      case FN_RPA:                      /* \frpad() and \flpad() */
      case FN_LPA:
        if (argn < 1)
          goto fnend;
        val1 = "";
        if (argn > 1)
          if (*(bp[1]))
            val1 = evalx(bp[1]);
        if (argn == 1) {                /* If a number wasn't given */
            p = fnval;                  /* just return the original string */
            ckstrncpy(p,bp[0],FNVALL);
        } else if (argn > 1 &&  !*val1) {
            failed = 1;
            evalerr(fn);
        } else /* if (chknum(val1)) */ { /* Repeat count */
            char pc;
            n = atoi(val1);
            if (n >= 0) {
                p = fnval;
                k = (int)strlen(bp[0]); /* Length of string to be padded */
                if (k >= n) {           /* It's already long enough */
                    ckstrncpy(p,bp[0],FNVALL);
                } else {
                    if (n + k <= FNVALL) {
                        pc = (char) ((argn < 3) ? SP : *bp[2]);
                        if (!pc) pc = SP;
                        if (cx == FN_RPA) { /* RPAD */
                            strncpy(p,bp[0],k); /* (leave it like this) */
                            p[k] = NUL;
                            p += k;
                            for (i = k; i < n; i++)
                              *p++ = pc;
                        } else {        /* LPAD */
                            n -= k;
                            for (i = 0; i < n; i++)
                              *p++ = pc;
                            strncpy(p,bp[0],k); /* (leave it like this) */
                            p[k] = NUL;
                            p += k;
                        }
                    }
                    *p = NUL;
                }
            }
        }
        p = fnval;
        goto fnend;

#ifdef ZFCDAT
      case FN_FD:                       /* \fdate(filename) */
        p = fnval;
        s = zfcdat(bp[0]);
        if (!s) s = "";
        if (!*s) {
            failed = 1;
            if (fndiags)
              ckmakmsg(fnval,FNVALL,"<ERROR:FILE_NOT_FOUND:\\f",fn,"()>",NULL);
            goto fnend;
        }
        ckstrncpy(fnval,s,FNVALL);
#endif /* ZFCDAT */
        goto fnend;

    } /* Break up big switch... */

    switch (y) {
      case FN_FS: {			/* \fsize(filename) */
	  CK_OFF_T z;
	  p = fnval;
	  z = zchki(bp[0]);
	  if (z < (CK_OFF_T)0) {
	      failed = 1;
	      if (fndiags) {
		  if (z == (CK_OFF_T)-1)
		    ckmakmsg(fnval,FNVALL,
			     "<ERROR:FILE_NOT_FOUND:\\f",fn,"()>",NULL);
		  else if (z == (CK_OFF_T)-2)
		    ckmakmsg(fnval,FNVALL,
			     "<ERROR:FILE_NOT_READABLE:\\f",fn,"()>",NULL);
		  else if (z == (CK_OFF_T)-3)
		    ckmakmsg(fnval,FNVALL,
			     "<ERROR:FILE_NOT_ACCESSIBLE:\\f",fn,"()>",NULL);
		  else
		    ckmakmsg(fnval,FNVALL,
			     "<ERROR:FILE_ERROR:\\f",fn,"()>",NULL);
	      }
	      goto fnend;
	  }
	  ckstrncpy(fnval,ckfstoa(z),FNVALL);
	  goto fnend;
      }
      case FN_VER:                      /* \fverify() */
	p = "-1";
	if (argn == 1)			/* No second arg */
	  goto fnend;
	else if (!bp[1])		/* Or second arg null */
	  goto fnend;
	else if (!*(bp[1]))		/* or empty. */
	  goto fnend;
        p = "0";
        if (argn > 1) {                 /* Only works if we have 2 or 3 args */
            int start;
            char *s2, ch1, ch2;
            start = 0;
            if (argn > 2) {             /* Starting position specified */
                val1 = *(bp[2]) ? evalx(bp[2]) : "0";
                if (chknum(val1)) {
                    start = atoi(val1) /* - 1 */;
                    if (start < 0) start = 0;
                    if (start > (int)strlen(bp[1]))
                      goto verfin;
                } else {
                    failed = 1;
                    evalerr(fn);
                    goto fnend;
                }
            }
            i = start;
            p = "0";
            for (s = bp[1] + start; *s; s++,i++) {
                ch1 = *s;
                if (!inpcas[cmdlvl]) if (islower(ch1)) ch1 = toupper(ch1);
                j = 0;
                for (s2 = bp[0]; *s2; s2++) {
                    ch2 = *s2;
                    if (!inpcas[cmdlvl]) if (islower(ch2)) ch2 = toupper(ch2);
                    if (ch1 == ch2) {
                        j = 1;
                        break;
                    }
                }
                if (j == 0) {
                    sprintf(fnval,"%d",i+1); /* SAFE */
                    p = fnval;
                    break;
                }
            }
        }
      verfin:
        goto fnend;

      case FN_IPA:                      /* Find and return IP address */
        if (argn > 0) {                 /* in argument string. */
            int start = 0;
            if (argn > 1) {             /* Starting position specified */
                if (chknum(bp[1])) {
                    start = atoi(bp[1]) - 1;
                    if (start < 0) start = 0;
                } else {
                    failed = 1;
                    if (fndiags)
                      ckmakmsg(fnval,FNVALL,
                               "<ERROR:ARG_NOT_NUMERIC:\\f",fn,"()>",NULL);
                    goto fnend;
                }
            }
            p = getip(bp[0]+start);
        } else p = "";
        goto fnend;

#ifdef OS2
      case FN_CRY:
        p = "";
        if (argn > 0) {
            p = fnval;
            ckstrncpy(p,bp[0],FNVALL);
            ck_encrypt(p);
        }
        goto fnend;

      case FN_OOX:
        p = "";
        if (argn > 0)
          p = (char *) ck_oox(bp[0], (argn > 1) ? bp[1] : "");
        goto fnend;
#endif /* OS2 */

      case FN_HEX:                      /* \fhexify(arg1) */
        if (argn < 1)
          goto fnend;
        if ((int)strlen(bp[0]) < (FNVALL / 2)) {
            s = bp[0];
            p = fnval;
            while (*s) {
                x = (*s >> 4) & 0x0f;
                *p++ = hexdigits[x];
                x = *s++ & 0x0f;
                *p++ = hexdigits[x];
            }
            *p = NUL;
            p = fnval;
        }
        goto fnend;

      case FN_UNTAB:			/* \funtab(arg1) */
	if (argn < 1)
	  goto fnend;
	if ((int)strlen(bp[0]) < (FNVALL * 2)) {
	    s = bp[0];
	    p = fnval;
	    if (untabify(bp[0],p,FNVALL) < 0) {
		failed = 1;
		if (fndiags)
		  ckmakmsg(fnval,FNVALL,
			   "<ERROR:OVERFLOW:\\f",fn,"()>",NULL);
	    }
	    goto fnend;
	}

      case FN_UNH: {                    /* \funhex(arg1) */
          int c[2], i;
          if (argn < 1)
            goto fnend;
          if ((int)strlen(bp[0]) < (FNVALL * 2)) {
              s = bp[0];
              p = fnval;
              while (*s) {
                  for (i = 0; i < 2; i++) {
                      c[i] = *s++;
                      if (!c[i]) { p = ""; goto unhexfin; }
                      if (islower(c[i])) c[i] = toupper(c[i]);
                      if (c[i] >= '0' && c[i] <= '9') {
                          c[i] -= 0x30;
                      } else if (c[i] >= 'A' && c[i] <= 'F') {
                          c[i] -= 0x37;
                      } else {
                          failed = 1;
                          if (fndiags)
                            ckmakmsg(fnval,
                                     FNVALL,
                                     "<ERROR:ARG_OUT_OF_RANGE:\\f",
                                     fn,
                                     "()>",
                                     NULL
                                     );
                          goto fnend;
                      }
                  }
                  *p++ = ((c[0] << 4) & 0xf0) | (c[1] & 0x0f);
              }
              *p = NUL;
              p = fnval;
          }
        unhexfin:
          goto fnend;
      }

      case FN_BRK: {                    /* \fbreak() */
          char * c;                     /* Characters to break on */
          char c2, s2;
          int start = 0;
          int done = 0;
          if (argn < 1)
            goto fnend;
          if (argn > 2) {
              s = bp[2] ? bp[2] : "0";
              if (chknum(s)) {
                  start = atoi(s);
                  if (start < 0) start = 0;
                  if (start > (int)strlen(bp[0]))
                    goto brkfin;
              } else {
                  failed = 1;
                  if (fndiags)
                    ckmakmsg(fnval,FNVALL,
                             "<ERROR:ARG_NOT_NUMERIC:\\f",fn,"()>",NULL);
                  goto fnend;
              }
          }
          s = bp[0] + start;            /* Source pointer */

          while (*s && !done) {
              s2 = *s;
              if (!inpcas[cmdlvl] && islower(s2)) s2 = toupper(s2);
              c = bp[1] ? bp[1] : "";   /* Character to break on */
              while (*c) {
                  c2 = *c;
                  if (!inpcas[cmdlvl] && islower(c2)) c2 = toupper(c2);
                  if (c2 == s2) {
                      done = 1;
                      break;
                  }
                  c++;
              }
              if (done) break;
              *p++ = *s++;
          }
          *p = NUL;                     /* terminate the result */
          p = fnval;                    /* and point to it. */
        brkfin:
          goto fnend;
      }

      case FN_SPN: {                    /* \fspan() */
          char *q;
          char c1, c2;
          int start = 0;
          if (argn < 1)
            goto fnend;
          if (argn > 2) {               /* Starting position */
              s = bp[2] ? bp[2] : "0";
              if (chknum(s)) {
                  start = atoi(s);
                  if (start < 0) start = 0;
              } else {
                  failed = 1;
                  if (fndiags)
                    ckmakmsg(fnval,FNVALL,
                             "<ERROR:ARG_NOT_NUMERIC:\\f",fn,"()>",NULL);
                  goto fnend;
              }
          }
          s = bp[0] + start;            /* Source pointer */
          if (argn > 1 &&
              (int)strlen(bp[1]) > 0 &&
              start <= (int)strlen(bp[0])) {
              while (*s) {              /* Loop thru source string */
                  q = bp[1];            /* Span string */
                  c1 = *s;
                  if (!inpcas[cmdlvl])
                    if (islower(c1)) c1 = toupper(c1);
                  x = 0;
                  while ((c2 = *q++)) {
                      if (!inpcas[cmdlvl])
                        if (islower(c2)) c2 = toupper(c2);
                      if (c1 == c2) { x = 1; break; }
                  }
                  if (!x) break;
                  *p++ = *s++;
              }
              *p = NUL;                 /* Terminate and return the result */
              p = fnval;
          }
          goto fnend;
      }
    } /* Break up big switch... */

    switch (y) {
      case FN_TRM:                      /* \ftrim(s1[,s2]) */
      case FN_LTR:                      /* \fltrim(s1[,s2]) */
        if (argn < 1)
          goto fnend;
        if ((len1 = (int)strlen(bp[0])) > 0) {
            if (len1 > FNVALL)
              len1 = FNVALL;
            s = " \t\r\n";
            if (argn > 1)               /* Trim list given */
              s = bp[1];
            len2 = (int)strlen(s);
            if (len2 < 1) {             /* or not... */
                s = " \t\r\n";          /* Default is to trim whitespace */
                len2 = 2;
            }
            if (cx == FN_TRM) {         /* Trim from right */
                char * q, p2, q2;
                ckstrncpy(fnval,bp[0],FNVALL); /* Copy string to output */
                p = fnval + len1 - 1;   /* Point to last character */

                while (p >= (char *)fnval) { /* Go backwards */
                    q = s;              /* Point to trim list */
                    p2 = *p;
                    if (!inpcas[cmdlvl])
                      if (islower(p2)) p2 = toupper(p2);
                    while (*q) {        /* Is this char in trim list? */
                        q2 = *q;
                        if (!inpcas[cmdlvl])
                          if (islower(q2)) q2 = toupper(q2);
                        if (p2 == q2) { /* Yes, null it out */
                            *p = NUL;
                            break;
                        }
                        q++;
                    }
                    if (!*q)            /* Trim list exhausted */
                      break;            /* So we're done. */
                    p--;                /* Else keep trimming */
                }
            } else {                    /* Trim from left */
                char * q, p2, q2;
                p = bp[0];              /* Source */
                while (*p) {
                    p2 = *p;
                    if (!inpcas[cmdlvl])
                      if (islower(p2)) p2 = toupper(p2);
                    q = s;
                    while (*q) {        /* Is this char in trim list? */
                        q2 = *q;
                        if (!inpcas[cmdlvl])
                          if (islower(q2)) q2 = toupper(q2);
                        if (p2 == q2) { /* Yes, point past it */
                            p++;        /* and try next source character */
                            break;
                        }
                        q++;            /* No, try next trim character */
                    }
                    if (!*q)            /* Trim list exhausted */
                      break;            /* So we're done. */
                }
                ckstrncpy(fnval,p,FNVALL);
            }
            p = fnval;
        } else p = "";
        goto fnend;

      case FN_CAP:                      /* \fcapitalize(arg1) */
        if (argn < 1)
          goto fnend;
        s = bp[0];
        p = fnval;
        x = 0;
        while ((c = *s++)) {
            if (isalpha(c)) {
                if (x == 0) {
                    x = 1;
                    if (islower(c))
                      c = toupper(c);
                } else if (isupper(c))
                  c = tolower(c);
            }
            *p++ = c;
        }
        *p = NUL;
        p = fnval;
        goto fnend;

#ifdef COMMENT
      case FN_TOD:                      /* Time of day to secs since midnite */
        sprintf(fnval,"%ld",tod2sec(bp[0])); /* SAFE */
        goto fnend;
#endif /* COMMENT */

      case FN_FFN:                      /* Full pathname of file */
        zfnqfp(bp[0],FNVALL,p);
        if (!p) p = "";
        goto fnend;

      case FN_CHK: {                    /* \fchecksum() */
          long chk = 0;
          p = (argn > 0) ? bp[0] : "";
          while (*p) chk += *p++;
          sprintf(fnval,"%lu",chk);     /* SAFE */
          p = fnval;
          goto fnend;
      }

#ifndef NOXFER
      case FN_CRC:                      /* \fcrc16() */
        if (argn > 0)
          sprintf(fnval,"%u",           /* SAFE */
                  chk3((CHAR *)bp[0],(int)strlen(bp[0])));
        else
          p = "0";
        goto fnend;
#endif /* NOXFER */

      case FN_BSN:                      /* \fbasename() */
        zstrip(bp[0],&p);
        goto fnend;

#ifndef NOLOCAL
#ifdef OS2
      case FN_SCRN_CX:                  /* \fscrncurx() */
        p = fnval;
        sprintf(p,"%d",(int)VscrnGetCurPos(VTERM)->x); /* SAFE */
        goto fnend;

      case FN_SCRN_CY:                  /* \fscrncury() */
        p = fnval;
        sprintf(p,"%d",(int)VscrnGetCurPos(VTERM)->y); /* SAFE */
        goto fnend;

      case FN_SCRN_STR: {               /* \fscrnstr() */
          videoline * line = NULL;
          viocell * cells = NULL;
          int row = 0, col = 0, len = 0;
          /* NOTE: On Unicode systems, the screen contents are stored in */
          /* in Unicode.  Therefore, we should really be performing a    */
          /* conversion to the local character set.                      */

          /* 6/18/2000 - added the translation to lcs */

          if (bp[0] == NULL || bp[0][0] == '\0') {
              row = 0;
          } else {
              if (chknum(bp[0])) {
                  row = atoi(bp[0]);
                  if (row < 0)
                    row = 0;
              } else {
                  failed = 1;
                  if (fndiags)
                    ckmakmsg(fnval,FNVALL,
                             "<ERROR:ARG_NOT_NUMERIC:\\f",fn,"()>",NULL);
                  goto fnend;
              }
          }
          line = VscrnGetLineFromTop( VTERM, (USHORT) row );
          if (line != NULL) {
              if (bp[1] == NULL || bp[1][0] == '\0')
                col = 0;
              else {
                  if (chknum(bp[0])) {
                      col = atoi(bp[1]);
                      if (col < 0 || col >= line->width)
                        col = 0;
                  } else {
                      failed = 1;
                      if (fndiags)
                        ckmakmsg(fnval,FNVALL,
                                 "<ERROR:ARG_NOT_NUMERIC:\\f",fn,"()>",NULL);
                      goto fnend;
                  }
              }
              if (bp[2] == NULL || bp[2][0] == '\0') {
                  len = line->width - (col+1);
              } else {
                  if (!chknum(bp[2])) {
                      failed = 1;
                      if (fndiags)
                        ckmakmsg(fnval,FNVALL,
                                 "<ERROR:ARG_NOT_NUMERIC:\\f",fn,"()>",NULL);
                      goto fnend;
                  }
                  len = atoi(bp[2]);
                  if (len < 0 || len > line->width)
                    len = line->width;
              }
              cells = line->cells;
              for (i = 0; i < len; i++) {
                  int pos = i + col;
                  if (pos < line->width) {
                      if (isunicode())
                        fnval[i] = (CHAR) utolxlat(cells[pos].c);
                      else
                        fnval[i] = (CHAR) (cells[pos].c & 0xFF);
                      if (fnval[i] == 0)
                        fnval[i] = SP;
                  } else
                    fnval[i] = SP;
              }
              fnval[i] = '\0';
          } else {
              fnval[0] = '\0';
          }
          p = fnval;
          goto fnend;
      }
#endif /* OS2 */
#endif /* NOLOCAL */

#ifndef NOPUSH
      case FN_RAW:                      /* \frawcommand() */
      case FN_CMD: {                    /* \fcommand() */
          int x, c, n = FNVALL;
          x = 0;                        /* Completion flag */
/*
  ZIFILE can be safely used because we can't possibly be transferring a file
  while executing this function.
*/
          if (!nopush && zxcmd(ZIFILE,bp[0]) > 0) { /* Open the command */
              while (n-- > -1) {        /* Read from it */
                  if ((c = zminchar()) < 0) {
                      x = 1;             /* EOF - set completion flag */
                      if (cx == FN_CMD) { /* If not "rawcommand" */
                          p--;           /* remove trailing newlines */
                          while (*p == CR || *p == LF)
                            p--;
                          p++;
                      }
                      *p = NUL;         /* Terminate the string */
                      break;
                  } else                /* Command still running */
                    *p++ = c;           /* Copy the bytes */
              }
              zclose(ZIFILE);           /* Close the command */
          }
          /* Return null string if command's output was too long. */
          p = fnval;
          if (!x) {
              failed = 1;
              if (fndiags)
                ckmakmsg(fnval,FNVALL,
                         "<ERROR:RESULT_TOO_LONG:\\f",fn,"()>",NULL);
          }
          goto fnend;
      }
#endif /* NOPUSH */
    } /* Break up big switch... */

    switch (y) {
      case FN_STX:                      /* \fstripx(string,c) */
        if (!(s = bp[0]))               /* Make sure there is a string */
          goto fnend;
        c = '.';                        /* Character to strip from */
        if (argn > 1) if (*bp[1]) c = *bp[1];
        n = ckstrncpy(fnval,bp[0],FNVALL);
        while (--n >= 0) {
            if (fnval[n] == c) {
                fnval[n] = NUL;
                break;
            }
        }
        p = fnval;
        goto fnend;

      case FN_STL:                      /* \flop(string,c) */
      case FN_LOPX: {			/* \flopx(string,c) */
	  int n = 1;
	  if (!(s = bp[0]))		/* Make sure there is a string */
	    goto fnend;
	  c = '.';			/* Character to strip to */
	  if (argn > 1) if (*bp[1]) c = *bp[1];
	  if (argn > 2) if (*bp[2]) {
#ifndef NOFLOAT
	      n = 0;
	      if (isfloat(bp[2],0)) {
		  n = (int)floatval;
		  if (n < 0) n = 0;
	      } else
#endif	/* NOFLOAT */
		n = atoi(bp[2]);
	  }
	  x = 0;
	  if (cx == FN_LOPX) {		/* Lopx (from right) */
	      if (n == 0)
		goto fnend;
	      s += strlen(s) - 1;	/* We already know it's > 0 */
	      while (s-- >= bp[0]) {
		  if (*s == c) {
		      n--;
		      if (n == 0) {
			  s++;
			  x = 1;
			  break;
		      }
		  }
	      }
	      if (!x) s = "";
	  } else {			/* Lop (from left) */
	      if (n == 0) {
		  p = bp[0];
		  goto fnend;
	      }
	      while (*s++) {
		  if (*(s-1) == c) {
		      if (--n == 0) {
			  x = 1;
			  break;
		      }
		  }
	      }
	      if (!x) s = bp[0];
	  }
	  ckstrncpy(fnval,s,FNVALL);
	  p = fnval;
	  goto fnend;
      }
      case FN_STN:                      /* \fstripn(string,n) */
        if (argn < 1)                   /* Remove n chars from right */
          goto fnend;
        val1 = "0";
        if (argn > 1)
          if (*(bp[1]))
            val1 = evalx(bp[1]);
        if (!chknum(val1)) {
            failed = 1;
            evalerr(fn);
            goto fnend;
        }
        n = atoi(val1);
        if (n < 0) n = 0;
        k = (int)strlen(s = bp[0]) - n;
        if (k < 0) k = 0;
        p = fnval;
        while (k-- > 0)
          *p++ = *s++;
        *p = NUL;
        p = fnval;
        goto fnend;

      case FN_STB: {                    /* \fstripb(string,c) */
          char c2 = NUL;
          int i, k = 0;
          char * gr_opn = "\"{'([<";    /* Group open brackets */
          char * gr_cls = "\"}')]>";    /* Group close brackets */

          p = fnval;
          *p = NUL;
          if (!(s = bp[0]))             /* Make sure there is a string */
            goto fnend;
          if ((x = strlen(s)) < 1)
            goto fnend;
          c = NUL;                      /* Brace/bracket kind */
          if (argn > 1) {
              if (*bp[1]) {
                  if (chknum(bp[1])) {
                      k = atoi(bp[1]);
                      if (k < 0) k = 63;
                      for (i = 0; i < 6; i++) {
                          if (k & (1<<i)) {
                              if (s[0] == gr_opn[i] && s[x-1] == gr_cls[i]) {
                                  ckstrncpy(fnval,s+1,FNVALL);
                                  fnval[x-2] = NUL;
                                  goto fnend;
                              }
                          }
                      }
                      ckstrncpy(fnval,s,FNVALL); /* No match */
                      goto fnend;
                  }
              }
          }
	  c = !bp[1] ? 0 : *bp[1];
          if (!c) c = s[0];
          if (argn > 2) if (*bp[2]) c2 = *bp[2];
          if (*s == c) {
              if (!c2) {
                  switch (c) {
                    case '(': c2 = ')'; break;
                    case '[': c2 = ']'; break;
                    case '{': c2 = '}'; break;
                    case '<': c2 = '>'; break;
                    case '"': c2 = '"'; break;
                    case 39:  c2 = 39;  break;
                    case 96:  c2 = 39;  break;
                    default:
                      if (argn == 2) {
                          c2 = c;
                      } else {
                          strncpy(fnval,s,x); /* Leave it like this */
                          fnval[x] = NUL;
                          goto fnend;
                      }
                  }
              }
              if (s[x-1] == c2) {
                  strncpy(fnval,s+1,x-2); /* Leave it like this */
                  fnval[x-2] = NUL;
                  goto fnend;
              }
          }
          strncpy(fnval,s,x);
          fnval[x] = NUL;
          goto fnend;
      }

      case FN_2HEX:                     /* Number to hex */
      case FN_2OCT:                     /* Number to octal */
        val1 = evalx(bp[0]);
        if (!*val1) {
            failed = 1;
            evalerr(fn);
            goto fnend;
        }
        sprintf(fnval, cx == FN_2HEX ? "%lx" : "%lo", atol(val1)); /* SAFE */
        if (cx == FN_2HEX && (int)(strlen(fnval)&1))
          sprintf(fnval,"0%lx",atol(val1)); /* SAFE */
        p = fnval;
        goto fnend;

      case FN_DNAM: {                   /* Directory part of file name */
          char *s;
          zfnqfp(bp[0],FNVALL,p);       /* Get full name */
          if (!isdir(p)) {              /* Is it already a directory? */
              zstrip(p,&s);             /* No get basename */
              if (*s) {
                  x = ckindex(s,p,0,0,0); /* Pos of latter in former */
                  if (x > 0) p[x-1] = NUL;
              }
          }
          if (!p) p = "";
          goto fnend;
      }

#ifndef NORANDOM
      case FN_RAND:                     /* Random number */
#ifdef CK_SSL
        if (RAND_bytes((unsigned char *)&k,sizeof(k)) < 0)
#endif /* CK_SSL */
          k = rand();
        x = 0;
        if (argn > 0) {
            if (!chknum(bp[0])) {
                failed = 1;
                if (fndiags)
                  ckmakmsg(fnval,FNVALL,
                           "<ERROR:ARG_NOT_NUMERIC:\\f",fn,"()>",NULL);
                goto fnend;
            }
            x = atoi(bp[0]);
        }
#ifdef COMMENT
        sprintf(fnval,"%d", (x > 0 && k > 0) || (x < 0 && k < 0) ? k % x : 
                (x == 0 ? 0 : (0 - (k % (-x)))));
#else
        debug(F111,"rand",ckitoa(x),k);
#ifdef SUNOS4
/* This is really strange but on SunOS, if we are requesting random numbers */
/* between 0 and 4 or less, they always come out in sequence: 0 1 2 3 0 1 2 */
/* Shifting the result of rand() in this case gives a more random result.   */
        if (x < 5)
          k = k >> 5;
#endif /* SUNOS4 */
        if ((x > 0 && k > 0) || (x < 0 && k < 0))
          x = k % x;
        else if (x == 0)
          x = 0;
        else
          x = 0 - (k % (-x));
        debug(F101,"rand x","",x);
        sprintf(fnval,"%d", x);         /* SAFE */
#endif /* COMMENT */
        p = fnval;
        goto fnend;
#endif /* NORANDOM */
    } /* Break up big switch... */

    switch (y) {
      case FN_SPLIT:                    /* \fsplit(s1,a,s2,s3,mask) */
      case FN_WORD: {                   /* \fword(s1,n,s2,s3,mask) */
          int wordnum = 0;
          int splitting = 0;
          int x;
          int array = 0;
          int grouping = 0;
          int nocollapse = 0;
          char * sep = "";
          char * notsep = "";
          char * bp0 = NULL;
          char * bp1 = NULL;
          char   abuf[16];
          struct stringarray * q = NULL;

          splitting = (cx == FN_SPLIT); /* Our job */
	  debug(F101,"FN_SPLIT splitting","",splitting);

          fnval[0] = splitting ? '0' : NUL; /* Initial return value */
          fnval[1] = NUL;
          p = fnval;
          bp0 = bp[0];                  /* Source string */
          if (!bp0) bp0 = "";
          debug(F111,"fsplit bp[0]",bp0,argn);
          if (argn < 1 || !*bp0)        /* If none, return default value */
            goto fnend;

          bp1 = bp[1];                  /* Function-dependent arg */
          if (!bp1) bp1 = "";           /* (array or number) */
          debug(F110,"fsplit bp[1]",bp1,0);
          if (bp[5]) {
              if (!chknum(bp[5])) {
                  failed = 1;
                  ckmakmsg(fnval,FNVALL,
                           "<ERROR:ARG_NOT_NUMERIC:\\f",fn,"()>",NULL);
                  goto fnend;
              }
              x = atoi(bp[5]);
              nocollapse = x;
          }
          if (!splitting) {             /* \fword(): n = desired word number */
              val1 = "1";               /* Default is first word */
              if (argn > 1)             /* Word number supplied */
                if (*bp1)
                  val1 = evalx(bp1);
              if (!chknum(val1)) {
                  failed = 1;
                  ckmakmsg(fnval,FNVALL,
                           "<ERROR:ARG_NOT_NUMERIC:\\f",fn,"()>",NULL);
                  goto fnend;
              }
              n = atoi(val1);
          } else if (argn > 1 && *bp1) { /* \fsplit(): n = word count */
              ckstrncpy(abuf,bp1,16);   /* Get array reference */
              debug(F110,"fsplit abuf 1",abuf,0);
              failed = 1;               /* Assume it's bad */
              if (fndiags)              /* Default is this error message */
                ckmakmsg(fnval,FNVALL,
                         "<ERROR:ARG_BAD_ARRAY:\\f",fn,"()>",NULL);
              if (abuf[0] != '&')       /* "Address" of array */
                goto fnend;             /* It's bad */
              if (abuf[2]) {            /* Check for brackets */
                  if (abuf[2] != '[' || abuf[3] != ']') {
                      goto fnend;       /* Bad */
                  }
              }
              debug(F110,"fsplit abuf 2",abuf,0);
              if (abuf[1] > 64 && abuf[1] < 91) /* Convert upper to lower */
                abuf[1] += 32;
              if (abuf[1] < 97 || abuf[1] > 122) { /* Check for a-z */
                  goto fnend;
              }
              debug(F110,"fsplit abuf 3",abuf,0);
              array = 1;
              fnval[0] = NUL;           /* No error, erase message */
              failed = 0;               /* Unset failure flag */
              n = 0;                    /* Initialize word counter */
          }
          if (argn > 2)                 /* Have break set? */
            sep = bp[2];
          debug(F111,"fsplit sep",sep,argn);
          if (argn > 3)                 /* Have include set? */
            notsep = bp[3];
          debug(F111,"fsplit notsep",notsep,argn);
          if (argn > 4) {               /* Have grouping set? */
              char * bp4 = bp[4];
              debug(F111,"fsplit bp4",bp4,argn);
              if (!bp4) bp4 = "0";
              if (!*bp4) bp4 = "0";
              if (chknum(bp4)) {
                  grouping = atoi(bp4);
                  if (grouping == -1)
                    grouping = 127;
              } else {
                  failed = 1;
                  if (fndiags)
                    ckmakmsg(fnval,FNVALL,
                             "<ERROR:ARG_NOT_NUMERIC:\\f",fn,"()>",NULL);
                  goto fnend;
              }
          }
          /* Args parsed, now do the work */

          debug(F111,"fsplit bp0",bp0,n);
          q = cksplit(splitting,n,bp0,sep,notsep,grouping,0,nocollapse);

          wordnum = q ? q->a_size : -1; /* Check result */
          if (wordnum < 0) {
              failed = 1;               /* Failure */
              if (fndiags)
                ckmakmsg(fnval,FNVALL,
                         (wordnum == -1) ?
                         "<ERROR:MALLOC_FAILURE:\\f" :
                         "<ERROR:TOO_MANY_WORDS:\\f",
                         fn,
                         "()>",
                         NULL
                         );
              goto fnend;
          }
          if (splitting) {              /* \fsplit() result */
              ckstrncpy(fnval,ckitoa(wordnum),FNVALL);
              if (array) {              /* Array was not declared. */
                  int i;
                  if ((x = dclarray(abuf[1],wordnum)) < 0) { /* Declare it. */
                      failed = 1;
                      if (fndiags)
                        ckmakmsg(fnval,FNVALL,
                                 "<ERROR:MALLOC_FAILURE:\\f",fn,"()>",NULL);
                      goto fnend;
                  }
                  for (i = 1; i <= wordnum; i++) { /* Copy results */
                      makestr(&(a_ptr[x][i]),q->a_head[i]);
                  }
                  a_ptr[x][0] = NULL;   /* Array is 1-based */
                  makestr(&(a_ptr[x][0]),fnval); /* Element = size */
              }
          } else {                      /* \fword() result */
              char * s;
              s = q->a_head[1];
              if (!s) s = "";
              ckstrncpy(fnval,s,FNVALL);
          }
          goto fnend;                   /* Done */
      }

    } /* Break up big switch... */

    switch (y) {

#ifdef CK_KERBEROS
      case FN_KRB_TK:                   /* Kerberos tickets */
      case FN_KRB_NX:                   /* Kerberos next ticket */
      case FN_KRB_IV:                   /* Kerberos ticket is valid */
      case FN_KRB_FG:                   /* Kerberos Ticket flags */
      case FN_KRB_TT: {                 /* Kerberos ticket time */
          int kv = 0;                   /* Kerberos version */
          int n = 0;
          char * s = NULL;
          if (rdigits(bp[0])) {
              kv = atoi(bp[0]);
          } else {
              failed = 1;
              if (fndiags)
                ckmakmsg(fnval,FNVALL,
                         "<ERROR:ARG_NOT_NUMERIC:\\f",fn,"()>",NULL);
              goto fnend;
          }
          if (kv != 4 && kv != 5) {
              failed = 1;
              if (fndiags)
                ckmakmsg(fnval,FNVALL,
                         "<ERROR:ARG_OUT_OF_RANGE:\\f",fn,"()>",NULL);
              goto fnend;
          }
          if ((cx == FN_KRB_IV || cx == FN_KRB_TT || cx == FN_KRB_FG) &&
               argn < 2) {
              failed = 1;
              if (fndiags)
                ckmakmsg(fnval,FNVALL,"<ERROR:MISSING_ARG:\\f",fn,"()>",NULL);
              goto fnend;
          }
          switch (y) {
            case FN_KRB_TK:             /* Number of Kerberos tickets */
#ifdef CK_AUTHENTICATION
              switch (kv) {
                case 4:
                  n = ck_krb4_get_tkts();
                  sprintf(fnval, "%d", (n >= 0) ? n : 0); /* SAFE */
                  goto fnend;
                case 5: {
                    extern char * krb5_d_cc;
                    n = ck_krb5_get_tkts(krb5_d_cc);
                    sprintf(fnval, "%d", (n >= 0) ? n : 0); /* SAFE */
                    goto fnend;
                }
              }
#else
              sprintf(fnval,"%d",0);    /* SAFE */
#endif /* CK_AUTHENTICATION */
              goto fnend;

            case FN_KRB_NX:             /* Kerberos next ticket */
#ifdef CK_AUTHENTICATION
              switch (kv) {
                case 4:
                  s = ck_krb4_get_next_tkt();
                  ckstrncpy(fnval, s ? s : "",FNVALL);
                  goto fnend;
                case 5:
                  s = ck_krb5_get_next_tkt();
                  ckstrncpy(fnval, s ? s : "",FNVALL);
                  goto fnend;
              }
#else
              sprintf(fnval,"k%d next-ticket-string",kv); /* SAFE */
#endif /* CK_AUTHENTICATION */
              goto fnend;

            case FN_KRB_IV:             /* Kerberos ticket is valid */
#ifdef CK_AUTHENTICATION
              /* Return 1 if valid, 0 if not */
              switch (kv) {
                case 4:
                  n = ck_krb4_tkt_isvalid(bp[1]);
                  sprintf(fnval, "%d", n > 0 ? 1 : 0); /* SAVE */
                  goto fnend;
                case 5: {
                    extern char * krb5_d_cc;
                    n = ck_krb5_tkt_isvalid(krb5_d_cc,bp[1]);
                    sprintf(fnval,"%d", n > 0 ? 1 : 0); /* SAFE */
                    goto fnend;
                }
              }
#else
              sprintf(fnval,"%d",0);    /* SAFE */
#endif /* CK_AUTHENTICATION */
              goto fnend;

            case FN_KRB_TT:             /* Kerberos ticket time */
#ifdef CK_AUTHENTICATION
              switch (kv) {
                case 4:
                  n = ck_krb4_tkt_time(bp[1]);
                  sprintf(fnval,"%d", n >= 0 ? n : 0); /* SAFE */
                  goto fnend;
                case 5: {
                    extern char * krb5_d_cc;
                    n = ck_krb5_tkt_time(krb5_d_cc,bp[1]);
                    sprintf(fnval,"%d", n >= 0 ? n : 0); /* SAFE */
                    goto fnend;
                }
              }
#else
              ckstrncpy(fnval,"600",FNVALL); /* Some time */
#endif /* CK_AUTHENTICATION */
              goto fnend;

            case FN_KRB_FG:             /* Kerberos ticket flags */
#ifdef CK_AUTHENTICATION
              switch (kv) {
                case 4:
                  fnval[0] = '\0';
                  goto fnend;
                case 5: {
                    extern char * krb5_d_cc;
                    ckstrncpy(fnval,ck_krb5_tkt_flags(krb5_d_cc,bp[1]),FNVALL);
                    goto fnend;
                }
              }
#else
              fnval[0] = '\0';
#endif /* CK_AUTHENTICATION */
              goto fnend;
          }
          p = fnval;
          goto fnend;
      }
#endif /* CK_KERBEROS */

#ifdef FN_ERRMSG
      case FN_ERRMSG:
        if (rdigits(bp[0])) {
            k = atoi(bp[0]);
        } else {
            failed = 1;
            if (fndiags)
             ckmakmsg(fnval,FNVALL,"<ERROR:ARG_NOT_NUMERIC:\\f",fn,"()>",NULL);
            goto fnend;
        }
#ifdef VMS
        ckstrncpy(fnval,ckvmserrstr(k),FNVALL);
#else
        x = errno;
        errno = k;
        ckstrncpy(fnval,ck_errstr(),FNVALL);
        errno = x;
#endif /* VMS */
        p = fnval;
        goto fnend;
#endif /* FN_ERRMSG */

      case FN_DIM: {
          int max;
          char abuf[16], *s;
          fnval[0] = NUL;               /* Initial return value */
          ckstrncpy(abuf,bp[0],16);     /* Get array reference */
          s = abuf;
          if (*s == CMDQ) s++;
          failed = 1;                   /* Assume it's bad */
          p = fnval;                    /* Point to result */
          if (fndiags)                  /* Default is this error message */
            ckmakmsg(fnval,FNVALL,"<ERROR:ARG_BAD_ARRAY:\\f",fn,"()>",NULL);
          if (s[0] != '&') {            /* "Address" of array */
              goto fnend;
          }
          if (s[2]) {
              if (s[2] != '[' || s[3] != ']') {
                  goto fnend;
              }
          }
          if (s[1] >= 64 && s[1] < 91)  /* Convert upper to lower */
            s[1] += 32;
          if (s[1] < 95 || s[1] > 122) { /* Check for a-z */
              goto fnend;	 	 /* Bad */
          }
          if ((max = chkarray(s[1],1)) < 1) /* (second arg was 1) */
            max = 0;
          failed = 0;                   /* Unset failure flag */
          sprintf(fnval,"%d",max);      /* SAFE */
          goto fnend;
      }

    } /* Break up big switch... */

    switch (y) {
      case FN_JDATE:
        if (argn < 1)                   /* Check number of args */
          p = ckdate();                 /* None, get today's date-time */
        else                            /* Some */
          p = bp[0];                    /* Use first */
        p = ckcvtdate(p,0);             /* Convert to standard form */
        ckstrncpy(fnval,zjdate(p),FNVALL); /* Convert to Julian */
        p = fnval;                      /* Point to result */
        failed = 0;
        if (*p == '-') {
            failed = 1;
            if (fndiags)                /* Default is this error message */
              ckmakmsg(fnval,FNVALL,"<ERROR:ARG_BAD_DATE:\\f",fn,"()>",NULL);
        }
        goto fnend;

      case FN_DATEJ:
        ckstrncpy(fnval,jzdate(bp[0]),FNVALL); /* Convert to yyyy<dayofyear> */
        p = fnval;                      /* Point to result */
        failed = 0;
        if (*p == '-') {
            failed = 1;
            if (fndiags)                /* Default is this error message */
              ckmakmsg(fnval,FNVALL,"<ERROR:ARG_BAD_DATE:\\f",fn,"()>",NULL);
        }
        goto fnend;

      case FN_DTIM:                     /* \fcvtdate() */
      case FN_TIME:                     /* Free-format time to hh:mm:ss */
      case FN_NTIM:                     /* Time to sec since midnight */
        s = (argn > 0) ? bp[0] : "";
        if (!s) s = "";
        if (!*s)
          p = ckdate();                 /* None, get today's date */
        else                            /* Some */
          p = bp[0];                    /* Use first */
	{
	    char * s;
	    s = p;
	    while (*s) {
		if (*s < 32) {
		    *s = NUL;
		    break;
		}
		s++;
	    }
	    /* do { if (*s < '!') *s = NUL; break; } while (*s++); */
	}
        p = ckcvtdate(p,2);             /* Convert to standard form */
        if (*p == '<') {
            failed = 1;
            if (fndiags)                /* Default is this error message */
              ckmakmsg(fnval,FNVALL,
                       "<ERROR:ARG_BAD_DATE_OR_TIME:\\f",fn,"()>",NULL);
            p = fnval;
            goto fnend;
        }
        if (argn > 1) {			/* Format code */
            s = bp[1];
            if (!s) s = "";
            if (!*s) s = "0";
            if (!chknum(s)) {
                failed = 1;
                if (fndiags)
                  ckmakmsg(fnval,FNVALL,
                           "<ERROR:ARG_NOT_NUMERIC:\\f",fn,"()>",NULL);
                p = fnval;
                goto fnend;
            }
            x = atoi(s);
            /* if (x) */ p = shuffledate(p,x);
        }
        if (cx == FN_TIME) {
            p += 9;
        } else if (cx == FN_NTIM) {
            long sec = 0L;
            p[11] = NUL;
            p[14] = NUL;
            sec = atol(p+9) * 3600L + atol(p+12) * 60L + atol(p+15);
            sprintf(fnval,"%ld",sec);   /* SAFE */
            p = fnval;
        }
        goto fnend;

      case FN_MJD:                      /* Modified Julian Date */
        if (argn < 1)                   /* Check number of args */
          p = zzndate();                /* None, get today's date-time */
        else                            /* Some */
          p = bp[0];                    /* Use first */
        p = ckcvtdate(p,0);             /* Convert to standard form */
        if (*p == '-') {
            failed = 1;
            if (fndiags)                /* Default is this error message */
              ckmakmsg(fnval,FNVALL,"<ERROR:ARG_BAD_DATE:\\f",fn,"()>",NULL);
            goto fnend;
        }
        /* Convert to modified Julian date */
        sprintf(fnval,"%ld",mjd(p));    /* SAFE */
        p = fnval;                      /* Point to result */
        goto fnend;

      case FN_MJD2: {
          long k = 0L;
          int n = 0;
          p = evalx(bp[0]);
          if (*p == '-') {
              p++;
              n = 1;
          }
          if (!rdigits(p)) {
              failed = 1;
              evalerr(fn);
              p = fnval;
              goto fnend;
          } else {
              k = atol(p);
              if (n) k = -k;
          }
          ckstrncpy(fnval,mjd2date(k),FNVALL); /* Convert to Date */
          p = fnval;                    /* Point to result */
          failed = 0;
          goto fnend;
      }

#ifndef NODIAL
      case FN_PNCVT: {                  /* Convert phone number */
          extern char * pncvt();
          failed = 0;
          p = pncvt(bp[0]);
          if (!p) p = "";
          if (!*p) {
            failed = 1;
            if (fndiags)                /* Default is this error message */
              ckmakmsg(fnval,FNVALL,
                       "<ERROR:ARG_BAD_PHONENUM:\\f",fn,"()>",NULL);
        }
        goto fnend;
      }
#endif /* NODIAL */

      case FN_DAY:
      case FN_NDAY:
        if (argn < 1)                   /* Check number of args */
          p = zzndate();                /* None, get today's date-time */
        else                            /* Some */
          p = bp[0];                    /* Use first */
        p = ckcvtdate(p,0);             /* Convert to standard form */
        if (*p == '-') {
            failed = 1;
            if (fndiags)                /* Default is this error message */
              ckmakmsg(fnval,FNVALL,"<ERROR:ARG_BAD_DATE:\\f",fn,"()>",NULL);
            goto fnend;
        }
        failed = 0;
        z = mjd(p);                     /* Convert to modified Julian date */
        z = z % 7L;
        if (z < 0) {
            z = 0 - z;
            k = 6 - ((int)z + 3) % 7;
        } else {
            k = ((int)z + 3) % 7;	/* Day of week */
        }
        p = fnval;                      /* Point to result */
        if (cx == FN_NDAY)
          sprintf(fnval,"%d",k);        /* SAFE */
        else
          ckstrncpy(fnval,wkdays[k],FNVALL);
        goto fnend;

      case FN_N2TIM: {                  /* Sec since midnight to hh:mm:ss */
          long k = 0L;
          int n = 0, hh, mm, ss;
          char * s = bp[0];
          if (argn < 1)                 /* If no arg substitute 0 */
            s = "0";
          p = evalx(s);                 /* Evaluate expression silently */
          if (*p == '-') {              /* Check result for minus sign */
              p++;
              n = 1;
          }
          if (!rdigits(p)) { /* Check for numeric */
              failed = 1;
              ckmakmsg(fnval,FNVALL,
                       "<ERROR:ARG_NOT_NUMERIC:\\f",fn,"()>",NULL);
              p = fnval;
              goto fnend;
          } else {
              k = atol(p);
              if (n) k = -k;
          }
          if (k < 0) {                  /* Check for negative */
              failed = 1;
              if (fndiags)
                ckmakmsg(fnval,FNVALL,
                         "<ERROR:ARG_OUT_OF_RANGE:\\f",fn,"()>",NULL);
              p = fnval;
              goto fnend;
          }
          hh = k / 3600L;               /* Have positive number */
          mm = (k % 3600L) / 60L;       /* break it down... */
          ss = ((k % 3600L) % 60L);

          sprintf(fnval,"%02d:%02d:%02d",hh,mm,ss); /* SAFE */
          p = fnval;
          failed = 0;
          goto fnend;
      }

      case FN_PERM: {                   /* File permissions */
          p = fnval;
          z = zchki(bp[0]);
          if (z < 0) {
              failed = 1;
              if (fndiags) {
                  if (z == -1)
                    ckmakmsg(fnval,FNVALL,
                             "<ERROR:FILE_NOT_FOUND:\\f",fn,"()>",NULL);
                  else if (z == -2)
                    ckmakmsg(fnval,FNVALL,
                             "<ERROR:FILE_NOT_READABLE:\\f",fn,"()>",NULL);
                  else if (z == -3)
                    ckmakmsg(fnval,FNVALL,
                             "<ERROR:FILE_NOT_ACCESSIBLE:\\f",fn,"()>",NULL);
                  else
                    ckmakmsg(fnval,FNVALL,
                             "<ERROR:FILE_ERROR:\\f",fn,"()>",NULL);
              }
              goto fnend;
          }
#ifdef CK_PERMS
          ckstrncpy(fnval,ziperm(bp[0]),FNVALL);
#else
          ckstrncpy(fnval,"(unknown)",FNVALL);
#endif /* CK_PERMS */
          goto fnend;
      }
      case FN_TLOOK:                    /* tablelook() */
      case FN_ALOOK: {                  /* arraylook() */
          int i, x, hi, lo, max, cmdlen;
          char abuf[16], *s, *pat;
          char kwbuf[256];
          char delim = ':';
          failed = 1;                   /* Assume failure */
          ckstrncpy(fnval,"-1",FNVALL);
          pat = bp[0];                  /* Point to search pattern */
          if (!pat) pat = "";           /* Watch out for NULL pointer */
          cmdlen = strlen(pat);         /* Get pattern length */
          if (argn < 2 /* || cmdlen < 1 */ ) { /* Need two args */
              if (fndiags)
                ckmakmsg(fnval,FNVALL,"<ERROR:MISSING_ARG:\\f",fn,"()>",NULL);
              goto fnend;
          }
          ckstrncpy(abuf,bp[1],16);     /* Get array reference */
          if (argn > 2)
            delim = *(bp[2]);
          s = abuf;
          if ((x = arraybounds(s,&lo,&hi)) < 0) { /* Get index and bounds */
              if (fndiags)
               ckmakmsg(fnval,FNVALL,"<ERROR:ARG_BAD_ARRAY:\\f",fn,"()>",NULL);
              goto fnend;
          }
          p = fnval;                    /* Point to result */
          max = a_dim[x];               /* Size of array */
          if (lo < 0) lo = 0;           /* Use given range if any */
          if (lo > max) lo = max;
          if (hi < 0) hi = max;
          if (hi > max) hi = max;
          failed = 0;                   /* Unset failure flag */
          if (max < 1)
            goto fnend;
          kwbuf[255] = NUL;
          for (i = lo; i <= hi; i++) {
              if (!a_ptr[x][i])
                continue;
              if (cx == FN_ALOOK) {
                  if (ckmatch(pat,a_ptr[x][i],inpcas[cmdlvl],1+4)) {
                      sprintf(fnval,"%d",i); /* SAFE */
                      goto fnend;
                  }
              } else if (cx == FN_TLOOK) {
                  char * aa;
                  int j = 0, v = 0, len;
                  if (i == hi)
                    break;
                  aa = a_ptr[x][i];     /* Point to this array element */
                  if (!aa) aa = "";
                  while (j < 254 && *aa) { /* Isolate keyword */
                      if (*aa == delim)
                        break;
                      kwbuf[j++] = *aa++;
                  }
                  kwbuf[j] = NUL;
                  len = j;
                  v = 0;
                  if ((len == cmdlen && !ckstrcmp(kwbuf,pat,len,0)) ||
                      ((v = !ckstrcmp(kwbuf,pat,cmdlen,0)) &&
                       ckstrcmp(a_ptr[x][i+1],pat,cmdlen,0))) {
                      sprintf(fnval,"%d",i); /* SAFE */
                      goto fnend;
                  }
                  if (v) {              /* Ambiguous */
                      ckstrncpy(fnval,"-2",FNVALL);
                      goto fnend;
                  }
              }
          }
          if (cx == FN_TLOOK) {         /* tablelook() last element */
              ckstrncpy(fnval,"-1",FNVALL);
              if (!ckstrcmp(a_ptr[x][hi],pat,cmdlen,0))
                sprintf(fnval,"%d",hi); /* SAFE */
          }
          goto fnend;
      }
      case FN_TOB64:                    /* Base-64 conversion */
      case FN_FMB64:
        p = fnval;
        *p = NUL;
        if (argn < 1)
          goto fnend;
        if (cx == FN_TOB64) {
            x = b8tob64(bp[0],-1,fnval,FNVALL);
        } else {
            x = strlen(bp[0]);
            if (x % 4) {                /* length must be multiple of 4 */
                failed = 1;
                ckmakmsg(fnval,FNVALL,
                         "<ERROR:ARG_INCOMPLETE:\\f",fn,"()>",NULL);
                goto fnend;
            }
            b64tob8(NULL,0,NULL,0);     /* Reset */
            x = b64tob8(bp[0],-1,fnval,FNVALL);
            b64tob8(NULL,0,NULL,0);     /* Reset again */
        }
        if (x < 0) {
            failed = 1;
            if (fndiags) {
                char * m = "INTERNAL_ERROR";
                switch (x) {
                  case -1: m = "ARG_TOO_LONG"; break;
                  case -2: m = "ARG_OUT_OF_RANGE"; break;
                }
                if (ckmakmsg(fnval,FNVALL,"<ERROR:",m,"\\f",fn) > 0)
                  ckstrncat(fnval,"()>",FNVALL);
            }
        }
        goto fnend;

      case FN_ABS: {
          char * s;
          s = bp[0];
          if (*s == '-' || *s == '+')
            s++;
          if (!rdigits(s)) {
              if (fndiags)
                ckmakmsg(fnval,FNVALL,
                         "<ERROR:ARG_NOT_NUMERIC:\\f",fn,"()>",NULL);
              goto fnend;
          }
          ckstrncpy(fnval,s,FNVALL);
          goto fnend;
      }

      case FN_AADUMP: {
          char abuf[16], *s = NULL, **ap = NULL, **vp = NULL;
          char pattern[VNAML];
          int slen, i, j, k, first = -1;
          extern int xdelmac();
          p = fnval;
          if (argn < 2) {
              if (fndiags)
                ckmakmsg(fnval,FNVALL,"<ERROR:MISSING_ARG2:\\f",fn,"()>",NULL);
              goto fnend;
          }
          debug(F101,"aaconvert argn","",argn);
          s = bp[0];
          slen = strlen(s);

          /* Count elements so we can create the array */

          ckmakmsg(pattern,VNAML,s,"<*>",NULL,NULL);
          for (k = 0, i = 0; i < nmac; i++) {
              if (ckmatch(pattern,mactab[i].kwd,0,1)) {
                  if (first < 0)        /* Remember location of first match */
                    first = i;
                  k++;
              }
          }
          debug(F101,"aaconvert matches","",k);
          debug(F101,"aaconvert first","",first);
          fnval[0] = NUL;               /* Initial return value */
          ckstrncpy(abuf,bp[1],16);     /* Get array reference */
          s = abuf;
          if (*s == CMDQ) s++;
          p = fnval;                    /* Point to result */
          if (fndiags)                  /* Default is this error message */
            ckmakmsg(fnval,FNVALL,"<ERROR:ARG_BAD_ARRAY:\\f",fn,"()>",NULL);
          if (s[0] != '&')              /* Address of array */
            goto fnend;
          if (s[2])
            if (s[2] != '[' || s[3] != ']')
              goto fnend;
          if (s[1] >= 64 && s[1] < 91)  /* Convert upper to lower */
            s[1] += 32;
          if ((x = dclarray(s[1],k)) < 0) /* Declare array to size */
            goto fnend;
          ap = a_ptr[x];                /* Point to array we just declared */
          /* debug(F111,"aaconvert array 1",abuf,ap); */
          abuf[0] = NUL;
          if (argn > 2) {
              ckstrncpy(abuf,bp[2],16); /* Get value array reference */
              s = abuf;
              if (*s == CMDQ) s++;
              if (s[0] != '&')          /* Address of array */
                goto fnend;
              if (s[2])
                if (s[2] != '[' || s[3] != ']')
                  goto fnend;
              if (s[1] >= 64 && s[1] < 91) /* Convert upper to lower */
                s[1] += 32;
              if ((x = dclarray(s[1],k)) < 0)
                goto fnend;
              vp = a_ptr[x];            /* Point to array we just declared */
          }
          /* debug(F111,"aaconvert array 2",abuf,vp); */
          makestr(&ap[0],ckitoa(k));
          if (vp) makestr(&vp[0],ckitoa(k));
          if (fndiags)
           ckmakmsg(fnval,FNVALL,"<ERROR:ASSOCIATIVE_ARRAY:\\f",fn,"()>",NULL);

          /* Copy macro index & value to the arrays and then remove the */
          /* macro, so the 'first' pointer keeps indicating the next one. */
          /* We could combine the initial counting loop with this one but */
          /* then it would be harder to create the array and anyway this */
          /* function is plenty fast as it is. */

          for (i = 1; i <= k; ) {
              if (!ckmatch(pattern,mactab[first].kwd,0,1)) {
                  debug(F111,"aaconvert oddball",mactab[first].kwd,first);
                  first++;
                  continue;
              }
              ckstrncpy(tmpbuf,mactab[first].kwd,TMPBUFSIZ); /* Macro name */
              s = tmpbuf;                       /* Make writeable copy */
              s += slen;                        /* Isolate "index" */
              j = strlen(s) - 1;
              if (*s != '<' || *(s+j) != '>') { /* Check syntax */
                  /* This shouldn't happen */
                  debug(F111,"aaconvert ERROR",mactab[first].kwd,first);
                  goto fnend;
              }
              *(s+j) = NUL;             /* Remove final '>' */
              debug(F111,"aaconvert",s+1,i);
              makestr(&(ap[i]),s+1);    /* Set first array to index */
              if (vp)
                makestr(&(vp[i]),mactab[first].mval); /* 2nd to value */
              if (xdelmac(first) < 0)
                goto fnend;
              i++;
          }
          sprintf(fnval,"%d",k);        /* SAFE */
          p = fnval;                    /* Return size of array */
          debug(F110,"aaconvert return",p,0);
          failed = 0;                   /* Unset failure flag */
          goto fnend;
      }

    } /* End of switch() */

#ifdef FNFLOAT
/*
  Floating-point functions.  To be included only if FNFLOAT is defined, which
  should happen only if CKFLOAT is also defined, and if the math library is
  linked in.  Even then, we might have float-vs-double confusion as well as
  confusion about what the final "%f" format effector is supposed to reference
  (32 bits, 64 bits, etc).  Expect trouble if CKFLOAT does not match the data
  type of math library functions or args.
*/
    if (cx == FN_FPABS ||               /* Floating-point functions */
        cx == FN_FPADD ||
        cx == FN_FPDIV ||
        cx == FN_FPEXP ||
        cx == FN_FPLOG ||
        cx == FN_FPLN  ||
        cx == FN_FPMOD ||
        cx == FN_FPMAX ||
        cx == FN_FPMIN ||
        cx == FN_FPMUL ||
        cx == FN_FPPOW ||
        cx == FN_FPSQR ||
        cx == FN_FPINT ||
        cx == FN_FPSUB ||
        cx == FN_FPROU ||
        cx == FN_FPSIN ||
        cx == FN_FPCOS ||
        cx == FN_FPTAN) {
        CKFLOAT farg[2], fpresult = 0.0;
        char fpbuf[64], * bp0;
        double dummy;
        /* int sign = 0; */
        int i, j, places = 0;
        int argcount = 1;

        failed = 1;
        p = fnval;
        bp0 = bp[0];
        if (!bp0)
          bp0 = "0";
        else if (!*bp0)
          bp0 = "0";
        if (!isfloat(bp0,0)) {
            k = mxlook(mactab,bp0,nmac);
            bp0 = (k > -1) ? mactab[k].mval : NULL;
            if (bp0) {
                if (!isfloat(bp0,0)) {
                    if (fndiags)
                      ckmakmsg(fnval,FNVALL,
                               "<ERROR:ARG_NOT_FLOAT:\\f",fn,"()>",NULL);
                    goto fnend;
                }
            }
        }
        if (cx == FN_FPINT) {           /* Float to int */
            failed = 0;
            ckstrncpy(fnval,bp0,FNVALL);
            for (i = 0; fnval[i]; i++) {
                if (fnval[i] == '.') {
                    fnval[i] = NUL;
                    break;
                }
            }
            goto fnend;
        }
        switch (y) {                    /* These need 2 args */
          case FN_FPADD:
          case FN_FPDIV:
          case FN_FPMOD:
          case FN_FPMAX:
          case FN_FPMIN:
          case FN_FPMUL:
          case FN_FPPOW:
          case FN_FPSUB:
            argcount = 2;
        }
        /* Missing arguments are supplied as 0.0 */

        debug(F111,fn,"argcount",argcount);
        for (i = 0; i < argcount; i++) { /* Get floating-point args */
#ifdef DEBUG
            if (deblog) {
                ckmakmsg(fpbuf,
                         64,
                         "bp[",
                         ckitoa(i),
                         bp[i] ? bp[i] : "(null)",
                         "]"
                         );
                debug(F100,fpbuf,"",0);
            }
#endif /* DEBUG */
            if (!bp[i]) {
                farg[i] = 0.0;
            } else if (!*(bp[i])) {
                farg[i] = 0.0;
            } else if (!isfloat(bp[i],0)) {
                char * tmp;
                k = mxlook(mactab,bp[i],nmac);
                tmp = (k > -1) ? mactab[k].mval : NULL;
                if (tmp) {
                    if (!isfloat(tmp,0)) {
                        if (fndiags)
                          ckmakmsg(fnval,FNVALL,
                                   "<ERROR:ARG_NOT_FLOAT:\\f",fn,"()>",NULL);
                        goto fnend;
                    }
                }
            }
            farg[i] = floatval;

#ifdef DEBUG
            if (deblog) {
                sprintf(fpbuf,"farg[%d]=%f",i,farg[i]); /* SAFE */
                debug(F100,fpbuf,"",0);
            }
#endif /* DEBUG */
        }
        if (bp[argcount]) {             /* Get decimal places */
            char * s;
            s = bp[argcount];
            if (!s) s = "";
            if (!*s) s = "0";
            s = evalx(s);
            if (!s) s = "";
            if (!*s) {
                evalerr(fn);
                goto fnend;
            }
            places = atoi(s);
        }
        errno = 0;
        failed = 0;
        switch (y) {                    /* Now do the requested function */
          case FN_FPABS:                /* Floating-point absolute value */
#ifndef COMMENT
            fpresult = fabs(farg[0]);
#else
            if (farg[0] < 0.0)
              fpresult = 0.0 - farg[0];
#endif /* COMMENT */
            break;
          case FN_FPADD:                /* FP add */
            fpresult = farg[0] + farg[1];
            break;
          case FN_FPDIV:                /* FP divide */
          case FN_FPMOD:                /* FP modulus */
            if (!farg[1]) {
                failed = 1;
                if (fndiags)
                  ckmakmsg(fnval,FNVALL,
                           "<ERROR:DIVIDE_BY_ZERO:\\f",fn,"()>",NULL);
            } else
              fpresult = (cx == FN_FPDIV) ?
                (farg[0] / farg[1]) :
                  fmod(farg[0],farg[1]);
            break;
          case FN_FPEXP:                /* FP e to the x */
            fpresult = (CKFLOAT) exp(farg[0]);
            break;
          case FN_FPLOG:                /* FP base-10 logarithm */
          case FN_FPLN:                 /* FP natural logarithm */
            if (farg[0] < 0.0) {
                failed = 1;
                if (fndiags)
                  ckmakmsg(fnval,FNVALL,
                           "<ERROR:ARG_OUT_OF_RANGE:\\f",fn,"()>",NULL);
            } else
              fpresult = (cx == FN_FPLOG) ? log10(farg[0]) : log(farg[0]);
            break;
          case FN_FPMUL:                /* FP multiply */
            fpresult = farg[0] * farg[1];
            break;
          case FN_FPPOW:                /* FP raise to a power */
            fpresult = modf(farg[1],&dummy);
            if ((!farg[0] && farg[1] <= 0.0) ||
                (farg[0] < 0.0 && fpresult)) {
                failed = 1;
                if (fndiags)
                  ckmakmsg(fnval,FNVALL,
                           "<ERROR:ARG_OUT_OF_RANGE:\\f",fn,"()>",NULL);
            } else
              fpresult = pow(farg[0],farg[1]);
            break;
          case FN_FPSQR:                /* FP square root */
            if (farg[0] < 0.0) {
                failed = 1;
                if (fndiags)
                  ckmakmsg(fnval,FNVALL,
                           "<ERROR:ARG_OUT_OF_RANGE:\\f",fn,"()>",NULL);
            } else
              fpresult = sqrt(farg[0]);
            break;
          case FN_FPSUB:                /* FP subtract */
            fpresult = farg[0] - farg[1];
            break;
          case FN_FPROU:                /* FP round */
            fpresult = farg[0];
            break;
          case FN_FPSIN:                /* FP sine */
            fpresult = (CKFLOAT) sin(farg[0]);
            break;
          case FN_FPCOS:                /* FP cosine */
            fpresult = (CKFLOAT) cos(farg[0]);
            break;
          case FN_FPTAN:                /* FP tangent */
            fpresult = (CKFLOAT) tan(farg[0]);
            break;
          case FN_FPMAX:
            fpresult = (farg[0] > farg[1]) ? farg[0] : farg[1];
            break;
          case FN_FPMIN:
            fpresult = (farg[0] < farg[1]) ? farg[0] : farg[1];
            break;
        }

        /* Get here with fpresult = function result */

        if (errno) {                    /* If range or domain error */
            failed = 1;
            if (fndiags)
              ckmakmsg(fnval,FNVALL,
                       "<ERROR:FLOATING-POINT-OP:\\f",fn,"()>",NULL);
        }
        if (failed)                     /* and/or any other kind of error, */
          goto fnend;                   /* fail. */
#ifndef COMMENT
        /* Call routine containing code that was formerly inline */
        ckstrncpy(fnval,fpformat(fpresult,places,cx == FN_FPROU),FNVALL);
#else
        {
            char fbuf[16];              /* For creating printf format */
            if (!fp_rounding &&         /* If printf doesn't round, */
                (places > 0 ||          /* round result to decimal places. */
                 (places == 0 && cx == FN_FPROU)))
              fpresult += (0.5 / pow(10.0,(CKFLOAT)places));
            if (places > 0) {                   /* If places specified */
                /* use specified places to write given number of digits */
                sprintf(fbuf,"%%0.%df",places); /* SAFE */
                sprintf(fnval,fbuf,fpresult);   /* SAFE */
            } else {                            /* Otherwise... */
#ifdef COMMENT
/*
  Here we want to print exactly fp_digits significant digits, no matter which
  side of the decimal point they are on.  That is, we want want the default
  format to show the maximum number of non-garbage digits, AND we want the last
  such digit to be rounded.  Of course there is no way to do that, since the
  digit after the last non-garbage digit is, well, garbage.  So the following
  clever ruse does no good.
*/
                int sign = 0, m = 0;
                sprintf(fnval,"%f",fpresult);
                if (fnval[0] == '-') sign = 1;
                for (i = sign; i < FNVALL; i++) {
                    if (isdigit(fnval[i]))
                      m++;
                    else
                      break;
                }
                if (m > 1) {
                    int d = fp_digits - m;
                    if (d < 1) d = 1;
                    sprintf(fbuf,"%%%d.%df",fp_digits+sign+1,d);
                } else {
                    sprintf(fbuf,"%%0.%df",fp_digits);
                }
                sprintf(fnval,fbuf,fpresult);
#else
                /* Go for max precision */
                sprintf(fbuf,"%%0.%df",fp_digits); /* SAFE */
                sprintf(fnval,fbuf,fpresult); /* SAFE */

#endif /* COMMENT */
            }
            if (fnval[0] == '-') sign = 1;
        }
        debug(F111,"fpresult 1",fnval,errno); /* Check for over/underflow */
        for (i = sign; fnval[i]; i++) { /* Give requested decimal places */
            if (fnval[i] == '.')        /* First find the decimal point */
              break;
            else if (i > fp_digits + sign - 1) /* replacing garbage */
              fnval[i] = '0';           /* digits with 0... */
        }
        if (fnval[i] == '.') {          /* Have decimal point */
            int gotend = 0;
            /* d < 0 so truncate fraction */
            if (places < 0 || (places == 0 && cx == FN_FPROU)) {
                fnval[i] = NUL;
            } else if (places > 0) {    /* d > 0 so this many decimal places */
                i++;                           /* First digit after decimal */
                for (j = 0; j < places; j++) { /* Truncate after d decimal */
                    if (!fnval[j+i])           /* places or extend to d  */
                      gotend = 1;              /* decimal places. */
                    if (gotend || j+i+sign > fp_digits)
                      fnval[j+i] = '0';
                }
                fnval[j+i] = NUL;
            } else {                    /* d == 0 so Do The Right Thing */
                for (j = (int)strlen(fnval) - 1; j > i+1; j--) {
                    if ((j - sign) > fp_digits)
                      fnval[j] = '0';
                    if (fnval[j] == '0')
                      fnval[j] = NUL;   /* Strip useless trailing 0's. */
                    else
                      break;
                }
            }
        }
#endif /* COMMENT */
        debug(F111,"fpresult 2",fnval,errno);
        goto fnend;

    }
#endif /* FNFLOAT */

#ifdef CKCHANNELIO
    if (cx == FN_FSTAT  ||              /* File functions */
        cx == FN_FPOS   ||
        cx == FN_FEOF   ||
        cx == FN_FGCHAR ||
        cx == FN_FGLINE ||
        cx == FN_FGBLK  ||
        cx == FN_FPCHAR ||
        cx == FN_FPLINE ||
        cx == FN_FPBLK  ||
        cx == FN_NLINE  ||
        cx == FN_FERMSG ||
        cx == FN_FILNO) {
        int x = 0, t = 0, channel;
        long z;
        extern int z_maxchan;

        failed = 1;                     /* Assume failure */
        p = fnval;                      /* until we validate args */
        if (cx == FN_FERMSG) {
            extern int z_error;
            if (argn < 1) {
                x = z_error;
            } else if (chknum(bp[0])) {
                x = atoi(bp[0]);
            } else if (fndiags)
              ckmakmsg(fnval,FNVALL,
                       "<ERROR:ARG_NOT_NUMERIC:\\f",fn,"()>",NULL);
            failed = 0;
            ckstrncpy(fnval,ckferror(x),FNVALL);
            goto fnend;
        }
        if (argn < 1) {                 /* All file functions need channel */
	    if (cx == FN_FSTAT) {	/* Except f_status(), e.g. when */
		fnval[0] = '0';		/* called with a variable that */
		fnval[1] = NUL;		/* hasn't been defined yet. */
		failed = 0;
	    } else {
		if (fndiags)
		 ckmakmsg(fnval,FNVALL,"<ERROR:MISSING_ARG:\\f",fn,"()>",NULL);
	    }
            goto fnend;
        }
        if (rdigits(bp[0])) {           /* Channel must be numeric */
            channel = atoi(bp[0]);
        } else {                        /* Fail if it isn't */
            if (fndiags)
              ckmakmsg(fnval,FNVALL,
                       "<ERROR:ARG_NOT_NUMERIC:\\f",fn,"()>",NULL);
            goto fnend;
        }
        if (channel < 0 || channel > z_maxchan) { /* Check channel range */
            if (fndiags)
              ckmakmsg(fnval,FNVALL,
                       "<ERROR:ARG_OUT_OF_RANGE:\\f",fn,"()>",NULL);
            goto fnend;
        }
        x = z_getmode(channel);         /* Find out about the channel */

        failed = 0;                     /* Assume success from here down */
        if (cx == FN_FSTAT) {           /* Status / modes of channel */
            if (x > -1)
              x &= FM_RWB;              /* Mask out irrelevant bits */
            else                        /* In this case not open is OK */
              x = 0;                    /* 0 if not open, 1-7 if open */
            sprintf(fnval,"%d",x);      /* SAFE */
            goto fnend;
        } else if (x < 1) {             /* Not \f_status() so must be open */
            failed = 1;
            if (fndiags)
              ckmakmsg(fnval,FNVALL,"<ERROR:FILE_NOT_OPEN:\\f",fn,"()>",NULL);
            goto fnend;
        }
        switch (y) {                    /* Do the requested function */
          case FN_FPOS:                 /* Get position */
            z = z_getpos(channel);	/* FIX THIS */
            sprintf(fnval,"%ld",z);     /* SAFE */
            goto fnend;

          case FN_NLINE:                /* Get line number */
            z = z_getline(channel);	/* FIX THIS */
            sprintf(fnval,"%ld",z);     /* SAFE */
            goto fnend;

          case FN_FEOF:                 /* Check EOF */
            t = 0;
            if (x & FM_EOF) t = 1;
            sprintf(fnval,"%d",t);      /* SAFE */
            goto fnend;

          case FN_FILNO:                /* Get file handle */
            x = z_getfnum(channel);
            sprintf(fnval,"%d",x);      /* SAFE */
            goto fnend;

          case FN_FPBLK:                /* Read or write block */
          case FN_FGBLK:
            if (argn < 2) {
                if (fndiags)
                  ckmakmsg(fnval,FNVALL,
                           "<ERROR:MISSING_ARG:\\f",fn,"()>",NULL);
                goto fnend;
            }
            if (rdigits(bp[1])) {
                t = atoi(bp[1]);
            } else {
                if (fndiags)
                  ckmakmsg(fnval,FNVALL,
                           "<ERROR:ARG_NOT_NUMERIC:\\f",fn,"()>",NULL);
                goto fnend;
            }
          case FN_FGCHAR:               /* Read or write character or line */
          case FN_FPCHAR:
          case FN_FGLINE:
          case FN_FPLINE:
            fnval[0] = NUL;
            switch (y) {
              case FN_FGCHAR: t = z_in(channel,fnval,FNVALL,1,1); break;
              case FN_FGLINE: t = z_in(channel,fnval,FNVALL,FNVALL-1,0); break;
              case FN_FGBLK:
                if (t >= FNVALL) t = FNVALL - 1;
                t = z_in(channel,fnval,FNVALL,t,1);
                break;
              case FN_FPCHAR: t = z_out(channel,bp[1],1,1);  break;
              case FN_FPLINE: t = z_out(channel,bp[1],-1,0); break;
              case FN_FPBLK:  t = z_out(channel,bp[1],-1,1); break;
            }
            if (t < 0) {                /* Handle read/write error */
                failed = 1;
                if (fndiags && t != FX_EOF)
                  ckmakmsg(fnval,FNVALL,
                           "<ERROR:FILE_ERROR_%d:\\f",fn,"()>",NULL);
                goto fnend;
            }
            if (cx == FN_FGCHAR)        /* Null terminate char */
              fnval[1] = NUL;
            /* Write (put) functions return numeric status code */
            if (cx == FN_FPCHAR || cx == FN_FPLINE || cx == FN_FPBLK)
              sprintf(fnval,"%d",t);    /* SAFE */
            goto fnend;
        }
    }
#endif /* CKCHANNELIO */

    if (cx == FN_SQUEEZE) {		/* String function \fsqueeze() */
	/* Squeeze out whitespace */
	/* Add options later for whether to trim leading and trailing blanks */
        /* and what to do about control characters, 8-bit whitespace, etc */
	int started = 0;		/* Flag for first non-whitespace */
	int n = 0;			/* Blank/Tab counter */
        s = bp[0] ? bp[0] : "";
        p = fnval;			/* Result buffer */
	while (*s) {			/* While there is input */
	    if (!started && (*s == ' ' || *s == '\011')) {
		s++;			/* Skip past leading whitespace */
		continue;
	    }
	    started++;			/* Leading whitespace was skipped */
	    if (*s != ' ' && *s != '\011') { /* Have a nonspace char */
		n = 0;			/* reset space counter */
		*p++ = *s++;		/* copy char to destination */
		continue;
	    }		    
	    if (n++ > 0) {		/* Have blank or tab */
		s++;			/* don't copy more than one */
		continue;
	    }
	    *p++ = ' ';			/* Deposit one space */
	    s++;			/* and go to next source char */
	}
	if (*(p-1) == ' ') p--;		/* Remove trailing space */
        *p = NUL;			/* Terminate string */
        p = fnval;			/* point to beginning */
        goto fnend;			/* Done. */
    }
    if (cx == FN_PATTERN) {             /* \fpattern() for INPUT */
        itsapattern = 1;
        if (argn > 0) {
            p = fnval;
            ckstrncpy(fnval,bp[0],FNVALL);
        } else p = "";
        goto fnend;
    }
    if (cx == FN_HEX2N || cx == FN_OCT2N) { /* \fhex2n(), \foct2n() */
        p = "0";
        if (argn < 1)
          goto fnend;
        p = ckradix(bp[0], ((cx == FN_HEX2N) ? 16 : 8), 10);
        if (!p) {
            if (fndiags)
              ckmakmsg(fnval,FNVALL,
                       "<ERROR:ARG_OUT_OF_RANGE:\\f",fn,"()>",NULL);
            goto fnend;
        }
        failed = 0;
        ckstrncpy(fnval,p,FNVALL);
        p = fnval;
        goto fnend;
    }

    if (cx == FN_HEX2IP) {
        int c[2], ip[4], i, k;
        p = "0";
        if (argn < 1)
          goto fnend;
        s = bp[0];
        if ((int)strlen(s) != 8) {
            failed = 1;
            if (fndiags)
              ckmakmsg(fnval,FNVALL,
                       "<ERROR:ARG_OUT_OF_RANGE:\\f",fn,"()>",NULL);
            goto fnend;
        }
        p = fnval;
        for (k = 0; k < 8; k += 2) {
            for (i = 0; i < 2; i++) {
                c[i] = *s++;
                if (islower(c[i])) c[i] = toupper(c[i]);
                if (c[i] >= '0' && c[i] <= '9') {
                    c[i] -= 0x30;
                } else if (c[i] >= 'A' && c[i] <= 'F') {
                    c[i] -= 0x37;
                } else {
                    failed = 1;
                    if (fndiags)
                      ckmakmsg(fnval,FNVALL,
                               "<ERROR:ARG_OUT_OF_RANGE:\\f",fn,"()>",NULL);
                    goto fnend;
                }
                ip[k/2] = c[0] << 4 | c[1];
            }
            sprintf(p,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]); /* SAFE */
        }
        goto fnend;
    }
    if (cx == FN_IP2HEX) {
        int ip[4], i;
        char * q;
        p = "00000000";
        if (argn < 1)
          goto fnend;
        s = bp[0];
        p = fnval;
        for (i = 0; i < 3; i++) {
            q = ckstrchr(s,'.');
            if (q) {
                *q++ = NUL;
                ip[i] = atoi(s);
                s = q;
            } else {
                failed = 1;
                if (fndiags)
                  ckmakmsg(fnval,FNVALL,
                           "<ERROR:ARG_OUT_OF_RANGE:\\f",fn,"()>",NULL);
                goto fnend;
            }
        }
        ip[3] = atoi(s);
        sprintf(p,"%02x%02x%02x%02x",ip[0],ip[1],ip[2],ip[3]); /* SAFE */
        goto fnend;
    }
    if (cx == FN_RADIX) {
        failed = 1;
        p = fnval;
        if (argn < 3) {
            if (fndiags)
              ckmakmsg(fnval,FNVALL,"<ERROR:MISSING_ARG:\\f",fn,"()>",NULL);
            goto fnend;
        }
        if (!rdigits(bp[1]) || !rdigits(bp[2])) {
            if (fndiags)
              ckmakmsg(fnval,FNVALL,
                       "<ERROR:ARG_NOT_NUMERIC:\\f",fn,"()>",NULL);
            goto fnend;
        }
        p = ckradix(bp[0],atoi(bp[1]),atoi(bp[2]));
        if (!p) {
            if (fndiags)
              ckmakmsg(fnval,FNVALL,
                       "<ERROR:ARG_OUT_OF_RANGE:\\f",fn,"()>",NULL);
            goto fnend;
        }
        failed = 0;
        ckstrncpy(fnval,p,FNVALL);
        p = fnval;
        goto fnend;
    }
    if (cx == FN_JOIN) {
        int i, x, y, z, flag, flag2, hi, lo, max, seplen, grouping = 0;
        char abuf[16], c, *s, *q, *sep = NULL;
        char * gr_opn = "\"{'([<";      /* Group open brackets */
        char * gr_cls = "\"}')]>";      /* Group close brackets */
        char lb[2], rb[2];              /* Selected left and right brackets */
	int csv = 0, tsv = 0;		/* Function flags */
	char specialchar = 0;		/* Field char that triggers grouping */
	char *s2 = NULL;		/* Address of malloc'd storage */

        failed = 1;                     /* Assume failure */
        fnval[0] = NUL;
        debug(F101,"FNJOIN ARGN","",argn);

        ckstrncpy(abuf,bp[0],16);       /* Get array reference */
        s = abuf;
        if ((x = arraybounds(s,&lo,&hi)) < 0) {  /* Get index and bounds */
            if (fndiags)
              ckmakmsg(fnval,FNVALL,"<ERROR:ARG_BAD_ARRAY:\\f",fn,"()>",NULL);
            goto fnend;
        }
        p = fnval;                      /* Point to result */
        max = a_dim[x];                 /* Size of array */
        if (lo < 0) lo = 1;             /* Use given range if any */
        if (lo > max) lo = max;
#ifdef COMMENT
	hi = max;
#else
/*
  This is a workaround for the problem in which the dimension of the \&_[]
  array (but not its contents) grows upon entry to a SWITCH block.  But this
  code prevents the dimension from growing.  Go figure.
*/
        if (hi < 0) {			/* Bounds not given */
            if (x)			/* Regular array */
	      hi = max;
	    else			/* Argument vector array */
	      for (hi = max; hi >= lo; hi--) { /* ignore any trailing */
		  if (!a_ptr[x][hi]) continue; /* empty elements */
		  if (!*(a_ptr[x][hi])) continue;
		  break;
	      }
	}
#endif /* COMMENT */
        if (hi > max) hi = max;
        failed = 0;                     /* Unset failure flag */
        if (max < 1)
          goto fnend;
        sep = " ";                      /* Separator */
        lb[0] = NUL;			/* Group start char (as string) */
        rb[0] = NUL;
        lb[1] = NUL;			/* Group end char as string */
        rb[1] = NUL;

        if (argn > 1) {
	    if (bp[1]) if (*bp[1]) {	/* If arg1 given and not empty */
		if (!strcmp(bp[1],"CSV")) { /* Special "CSV" symbolic arg */
		    csv++;		/* Make a comma separated list */
		    sep = ",";		/* Comma */
		    specialchar = *sep;	/* Separator is special character */
		    grouping = 1;	/* Group with doublequotes */
		    lb[0] = '"';	/* and here */
		    rb[0] = '"';	/* they are */
		} else if (!strcmp(bp[1],"TSV")) { /* "TSV" symbolic arg */
		    tsv++;		/* Make a Tab separated list */
		    sep = "\011";	/* Tab */
		    specialchar = *sep;
		    grouping = 0;	/* No grouping */
		} else			/* Normal case */
		  sep = bp[1];		/* use the separator char specified */
	    }
	}
        if (argn > 2 && !csv && !tsv) {	/* Grouping? */
            char * bp2 = bp[2];
            if (!bp2) bp2 = "0";
            if (!*bp2) bp2 = "0";
            if (chknum(bp2)) {
                grouping = atoi(bp2);
                if (grouping < 0 || grouping > 63)
                  grouping = 1;
            } else {
                failed = 1;
                if (fndiags)
                  ckmakmsg(fnval,FNVALL,
                           "<ERROR:ARG_NOT_NUMERIC:\\f",fn,"()>",NULL);
                goto fnend;
            }
            if (grouping) {             /* Take lowest-order one */
                int j, k;               /* and set the others to 0 */
                for (k = 0; k < 6; k++) {
                    j = 1 << k;
                    if (grouping & j) {
                        lb[0] = gr_opn[k];
                        rb[0] = gr_cls[k];
                        break;
                    }
                }
            }
        }
	if (!csv && !tsv) {		/* Normal case, not CSV or TSV */
	    specialchar = SP;		/* Special character is space */
	    if (argn > 3)		/* Nonzero 4th arg for no separator */
	      if (chknum(bp[3]))
		if (atoi(bp[3]) > 0)
		  sep = NULL;
	    if (!sep) {
		sep = "";
		seplen = 0;
	    } else
	      seplen = strlen(sep);
	}
        for (i = lo; i <= hi; i++) {    /* Loop thru selected array elements */
            s = a_ptr[x][i];            /* Get next element */
            if (!s)
              s = "";
            flag = 0;                   /* Flag to indicate grouping needed */
	    flag2 = 0;			/* Flag for internal doublequotes */
            if (grouping) {             /* Does this element need quoting? */
                q = s;                  /* Look for special character */
                while ((c = *q++)) {	/* If found */
		    if (c == specialchar) /* grouping is required */
		      flag++;
		    if (csv && (c == '"')) /* Character that needs doubling */
		      flag2++;		   /* in comma-separated list */
		    if (flag && !csv)	/* Exit early if no more to do */
		      break;
		}
            }
            y = strlen(s);              /* Get length of this element */
	    if ((y > 0) && csv && !flag) { /* CSV item needs grouping */
		if (s[0] == SP || s[y-1] == SP || /* if it has leading */
		    s[0] == HT || s[y-1] == HT)	/* or trailing whitespace */
		  flag++;		/* then it needs grouping */
	    }
	    if (flag || flag2) {	/* String needs grouping or quoting */
		char *ss = s;
                q = (char *)malloc(y + flag2 + 3); /* Make new buffer */
		if (q) {
		    s2 = q;		/* and this is what to free */
		    if (flag)		/* If grouping */
		      *q++ = lb[0];	/* put opening group quote */
		    while (*ss) {	/* Loop through string */
			if (flag2 && (*ss == '"')) /* If CSV and this a '"' */
			  *q++ = *ss;	           /* double it. */
			*q++ = *ss++;	/* Copy the character */
		    }
		    if (flag)		/* If grouping */
		      *q++ = rb[0];	/* add closing group quote */
		    *q = NUL;		/* terminate the result. */
		    s = s2;
		    y = strlen(s);
		}
	    }
            z = 0;                      /* Number of chars copied */
            flag = 0;                   /* flag is now buffer-overrun flag */
            if (y > 0)                  /* If this string is not empty */
              z = ckstrncat(fnval,s,FNVALL); /* copy it. */
	    if (s2) free(s2);		/* Free temp storage */
            if (z < y)                  /* Now check for buffer overrun. */
              flag++;
            if (!flag && *sep && i < hi) { /* If buffer still has room */
                z = ckstrncat(fnval,sep,FNVALL); /* copy delimiter */
                if (z < seplen)
                  flag++;
            }
            if (flag) {
                failed = 1;
                if (fndiags)
                  ckmakmsg(fnval,FNVALL,
                           "<ERROR:RESULT_TOO_LONG:\\f",fn,"()>",NULL);
                goto fnend;
            }
        }
	isjoin = 1;
        goto fnend;
    }
    if (cx == FN_SUBST) {               /* \fsubstitute() */
        CHAR c, * s, * r, * tp[2], buf1[256], buf2[256], buf3[256];
        int len, i, j, state = 0, lo = 0, hi = 0;

        failed = 0;
        p = fnval;                      /* Result pointer */
        *p = NUL;
        if (!bp[0])                     /* No target, no result*/
          goto fnend;

        len = strlen(bp[0]);            /* Length of source */
        if (len == 0)
          goto fnend;
        if (len > FNVALL) {
            failed = 1;
            if (fndiags)
              ckmakmsg(fnval,FNVALL,
                       "<ERROR:RESULT_TOO_LONG:\\f",fn,"()>",NULL);
            goto fnend;
        }
        if (!bp[1]) {
            ckstrncpy(bp[0],fnval,FNVALL);
            goto fnend;
        }
        tp[0] = buf1;                   /* For s2-s3 interpretation loop */
        tp[1] = buf2;

        for (i = 0; i < 256; i++) {     /* Initialize working buffers */
            buf1[i] = 0;                /* s2 expansion buffer */
            buf2[i] = 0;                /* s3 expansion buffer */
            buf3[i] = i;                /* Translation table */
        }
        for (i = 0; i < 2; i++) {       /* Interpret s2 and s3 */
            s = (CHAR *)bp[i+1];        /* Arg pointer */
            if (!s) s = (CHAR *)"";
            r = tp[i];                  /* To construct interpreted arg */
            j = 0;                      /* Output buf pointer */
            state = 0;                  /* Initial state */
            while (c = *s++) {          /* Loop thru arg chars */
                if (j > 255)            /* Output buf full */
                  break;
                switch (state) {
                  case 0:               /* Normal state */
                    switch (c) {
                      case '\\':        /* Have quote */
                        state = 1;
                        break;
                      case '[':         /* Have range starter */
                        state = 2;
                        break;
                      default:          /* Anything else */
                        r[j++] = c;
                        break;
                    }
                    continue;
                  case 1:               /* Quoted char */
                    r[j++] = c;
                    state = 0;
                    continue;
                  case 2:               /* Range bottom */
                    lo = c;
                    state++;
                    continue;
                  case 3:               /* Range separater */
                    if (c != '-') {
                        failed = 1;
                        if (fndiags)
                          ckmakmsg(fnval,FNVALL,
                                   "<ERROR:BAD_RANGE:\\f",fn,"()>",NULL);
                        goto fnend;
                    }
                    state++;
                    continue;
                  case 4:               /* Range top */
                    hi = c;
                    state++;
                    continue;
                  case 5:               /* Range end */
                    if (c != ']') {
                        failed = 1;
                        if (fndiags)
                          ckmakmsg(fnval,FNVALL,
                                   "<ERROR:BAD_RANGE:\\f",fn,"()>",NULL);
                        goto fnend;
                    }
                    for (k = lo; k <= hi && j < 255; k++) /* Fill in */
                      r[j++] = k;
                    lo = 0; hi = 0;     /* Reset */
                    state = 0;
                    continue;
                }
            }
        }
        for (i = 0; i < 256 && buf1[i]; i++) {  /* Create translation table */
            k = (unsigned)buf1[i];
            buf3[k] = buf2[i];
        }
        s = (CHAR *)bp[0];              /* Point to source string */
        for (i = 0; i < len; i++) {     /* Translation loop */
            k = (unsigned)s[i];         /* Get next char */
            if (!buf3[k])               /* Remove this char */
              continue;
            *p++ = buf3[k];             /* Substitute this char */
        }
        *p = NUL;
        p = fnval;
        goto fnend;
    }

#ifndef NOSEXP
    if (cx == FN_SEXP) {                /* \fsexpression(arg1) */
        char * p2;
        fsexpflag++;
        p = (argn > 0) ? dosexp(bp[0]) : "";
        fsexpflag--;
        p2 = fnval;
        while ((*p2++ = *p++)) ;
        p = fnval;
        goto fnend;
    }
#endif /* NOSEXP */

    if (cx == FN_CMDSTK) {              /* \fcmdstack(n1,n2) */
        int i, j, k;
        char * s;

        if (bp[0])
          val1 = *(bp[0]) ? evalx(bp[0]) : ckitoa(cmdlvl);
        else
          val1 = ckitoa(cmdlvl);
#ifdef COMMENT
        free(bp[0]);                    /* (evalx() always uses same buffer) */
        bp[0] = NULL;                   /* (not any more!) */
#endif /* COMMENT */
        failed = 1;
        if (argn > 1) {
#ifdef COMMENT
            makestr(&(bp[0]),val1);
            val1 = bp[0];
#endif /* COMMENT */
            val2 = *(bp[1]) ? evalx(bp[1]) : "0";
            if (!(chknum(val1) && chknum(val2))) {
                if (fndiags)
                  ckmakmsg(fnval,FNVALL,
                           "<ERROR:ARG_NOT_NUMERIC:\\f",fn,"()>",NULL);
                goto fnend;
            }
        } else {
            val1 = ckitoa(cmdlvl);
            val2 = "0";
        }
        i = atoi(val1);                 /* Level */
        j = atoi(val2);                 /* Flags */
        if (i < 0 || i > cmdlvl) {
            if (fndiags)
              ckmakmsg(fnval,FNVALL,
                       "<ERROR:ARG_OUT_OF_RANGE:\\f",fn,"()>",NULL);
            goto fnend;
        }
        failed = 0;
        p = fnval;
        k = cmdstk[i].src;              /* What (prompt, file, macro) */
        if (j) {
            ckstrncpy(fnval,ckitoa(k),FNVALL);
            goto fnend;
        }
        switch (k) {
          case CMD_KB:
            ckstrncpy(fnval,"(prompt)",FNVALL);
            break;
          case CMD_TF:
            s = tfnam[cmdstk[i].lvl];
            if (!zfnqfp(s,FNVALL,fnval))
              ckstrncpy(fnval,s,FNVALL);
            break;
          case CMD_MD:
            ckstrncpy(fnval,m_arg[cmdstk[i].lvl][0],FNVALL);
            break;
        }
        goto fnend;
    }
#ifdef CKFLOAT
    if (cx == FN_DIFDATE) {             /* \fdiffdates(d1,d2) */
        char * d1, * d2;
        d1 = bp[0] ? bp[0] : ckdate();
        d2 = bp[1] ? bp[1] : ckdate();
        p = (char *)cmdiffdate(d1,d2);
        if (!p) {
            failed = 1;
            if (fndiags) {
                ckmakmsg(fnval,FNVALL,"<ERROR:BAD_DATE:\\f",fn,"()>",NULL);
                p = fnval;
            }
        }
        goto fnend;
    }
#endif /* CKFLOAT */
    if (cx == FN_CMPDATE) {             /* \fcmddates(d1,d2) */
        int x = 0;
        char d1[18], d2[18], * dp;
        failed = 0;
        d1[0] = NUL;
        d2[0] = NUL;
        p = fnval;
        dp = cmcvtdate(bp[0],1);
        if (dp) {
            ckstrncpy(d1,dp,18);
            if ((dp = cmcvtdate(bp[1],1))) {
                ckstrncpy(d2,dp,18);
                x = 1;
            }
        }
        if (x == 0) {
            failed = 1;
            if (fndiags)
              ckmakmsg(fnval,FNVALL,"<ERROR:BAD_DATE:\\f",fn,"()>",NULL);
        } else {
            x = strcmp(d1,d2);
            if (x > 0)
              x = 1;
            else if (x < 0)
              x = -1;
            sprintf(fnval,"%d",x);
        }
        goto fnend;
    }
    if (cx == FN_TOGMT) {               /* \futcdate(d1) */
        char * d1, * dp;
        char datebuf[32];
        char d2[32];
        p = fnval;
        failed = 1;
        if ((dp = cmcvtdate(bp[0],1))) { /* The given date */
            ckstrncpy(datebuf,dp,18);
            ckstrncpy(d2,dp,18);        /* local time */
            ckstrncat(datebuf,"Z",19);  /* Same time GMT */
            if ((dp = cmcvtdate(datebuf,1))) /* converted to local time */
              ckstrncpy(datebuf,dp,18);
            if ((p = (char *)cmdiffdate(d2,datebuf))) { /* Get offset */
                ckstrncat(d2,p,32);     /* Append offset to local time */
                if ((dp = cmcvtdate(d2,1))) {
                    failed = 0;
                    ckstrncpy(fnval,dp,FNVALL);
                    p = fnval;
                }
            }
        }
        if (failed && fndiags)
          ckmakmsg(fnval,FNVALL,"<ERROR:BAD_DATE:\\f",fn,"()>",NULL);
        goto fnend;
    }
    if (cx == FN_DELSEC) {              /* \fdelta2secs(delta-time) */
        long secs;
        p = fnval;
        if ((x = delta2sec(bp[0],&secs)) < 0) {
            failed = 1;
            if (fndiags)
              ckmakmsg(fnval,FNVALL,
                       (x == -1) ?
                         "<ERROR:BAD_DELTA_TIME:\\f" :
                         "<ERROR:OVERFLOW:\\f",
                       fn,
                       "()>",
                       NULL
                       );
            goto fnend;
        }
        sprintf(p,"%ld",secs);
        goto fnend;
    }
    if (cx == FN_PC_DU) {
        char c, * s = bp[0];
        if (!s) s = "";
        p = fnval;
        while ((c = *s++)) {
            if (c == ':') {
                if (*s != '\\')
                  *p++ = '/';
            } else if (c == '\\') {
                *p++ = '/';
            } else {
                *p++ = c;
            }
        }
        *p = NUL;
        p = fnval;
        goto fnend;
    }
    if (cx == FN_PC_UD) {               /* Unix to DOS path */
        char c, * s = bp[0];
        if (!s) s = "";
        if (*s == '~') {                /* Skip leading tilde */
            s++;
            if (*s == '/')
              s++;
        }
        p = fnval;
        while ((c = *s++))
          *p ++ = (c == '/') ? '\\' : c;
        *p = NUL;
        p = fnval;
        goto fnend;
    }
    if (cx == FN_KWVAL) {               /* Keyword=Value */
        p = dokwval(bp[0],bp[1]?bp[1]:"=");
        goto fnend;
    }
#ifdef COMMENT
/* Cute idea but doesn't work */
    if (cx == FN_SLEEP || cx == FN_MSLEEP) {
        p = "";
        if (chknum(bp[0])) {
            x = atoi(bp[0]);
        } else {
            failed = 1;
            if (fndiags) {
                ckmakmsg(fnval,FNVALL,
                         "<ERROR:ARG_NOT_NUMERIC:\\f",fn,"()>",NULL);
                p = fnval;
            }
            goto fnend;
        }
        if (cx == FN_SLEEP)
          x *= 1000;
        msleep(x);
        goto fnend;
    }
#endif /* COMMENT */

#ifdef NT
    if (cx == FN_SNAME) {
        GetShortPathName(bp[0],fnval,FNVALL);
        goto fnend;
    }
    if (cx == FN_LNAME) {
        ckGetLongPathName(bp[0],fnval,FNVALL);
        goto fnend;
    }
#endif /* NT */

/*
  \femailaddress():
  Picks the email address out of an RFC 2822 From: or Sender: header.
  Added 26 Nov 2005.  Handles all common, and some uncommon, cases but
  doesn't totally bother about nested comments.  Needed this for fetching
  email from a POP server and then constructing the BSD "From " line.
  Works with or without the "From: " or "Sender: " tag.
*/
    if (cx == FN_EMAIL) {
        char c, * s = bp[0], * s2, * s3, * ap = "";
	int k, state = 0, quote = 0, infield = 0;
	int pc = 0;			/* For nested comments */
        if (!s) s = "";
	if (!*s) goto xemail;

	if (ckindex("From: ",s,0,0,0) == 1) s += 5;
	if (ckindex("Sender: ",s,0,0,0) == 1) s += 7;

	k = strlen(s);			/* Strip junk from end */
	if (k < 1) goto xemail;
	k--;
	while (k >= 0 && s[k] == CR || s[k] == LF)
	  s[k--] = NUL;
	while (k >= 0 && s[k] == SP || s[k] == HT)
	  s[k--] = NUL;
	if (k == 0)
	  goto xemail;

#ifndef COMMENT			     /* Simple method if not 100% foolproof */
	k = 0;
	for (s2 = s; *s2; s2++) {	/* Find at-sign */
	    if (*s2 == '@') {
		k++;			/* If more than one use rightmost */
		s3 = s2;
	    }
	}
	if (k < 1)			/* No at-sign */
	  goto xemail;

	for (ap = s3-1; ap >= s; ap--) { /* Back up to beginning of address */
	    if (isspace(*ap) || *ap == '<') {
		ap++;
 		break;
	    }
	    if (ap == s)
	      break;
	}
	for (s2 = s3+1; *s2; s2++) {	/* Find end of address */
	    if (isspace(*s2) || *s2 == '>')
	      break;
	}
	*s2-- = NUL;
	if (*ap == '[' && *s2 == ']') {	/* Handle [blah@blah.blah] */
	    ap++;
	    *s2 = NUL;
	}
	if (!ckstrcmp(ap,"mailto:",7,0)) /* Handle mailto: URLs */
	  ap += 7;

#else  /* Too complicated and error-prone */

	k = 0;
	for (s2 = s; *s2; s2++) {	/* Strip leading whitespace */
	    if (*s2 == SP || *s2 == HT) {
		k = 1;
		break;
	    }
	}
	if (!k) {			/* Simple address */
	    ap = s;
	    goto xemail;
	}
	do {				/* Not simple, have to extract it */
	    if (quote) {
		quote = 0;
		continue;
	    } else if (*s == '\\') {
		quote = 1;
		continue;
	    }
	    switch (state) {
	      case 0:
		if (!infield && *s == '"') { /* Quoted string */
		    infield = 1;
		    c = '"';
		    state = 1;
		} else if (!infield && *s == '(') { /* Comment in parens */
		    pc++;
		    infield = 1;
		    c = ')';
		    if (*ap) *s = NUL;
		    state = 1;
		} else if (!infield && *s == '<') { /* Address */
		    infield = 1;
		    c = '>';
		    ap = s+1;
		    state = 2;
		} else if (infield && (*s == SP || *s == HT)) {
		    infield = 0;
		} else {		/* One or more bare words */
		    infield = 1;	/* Could be an address */
		    if (!*ap) ap = s;	/* Could be comments */
		}
		continue;
	      case 1:			/* In Quoted string or Comment */
		if (infield && *s == c) { /* Look for end */
		    infield = 0;
		    *s++ = NUL;
		    while (*s == SP || *s == HT) s++;
		    if (!*ap)
		      ap = s;
		    state = 0;
		}
		continue;
	      case 2:			/* In address */
		if (infield && *s == c) { /* Looking for end */
		    infield = 0;
		    *s = NUL;
		    break;
		}
	    }
	} while (*s++);

      xemail:
	if (*ap) {
	    while (*ap == SP || *ap == HT) ap++;
	}
	k = strlen(ap) - 1;
	while (k >= 0 && (ap[k] == SP || ap[k] == HT))
	  ap[k--] = NUL;
	if (*ap) {
	    failed = 0;
	    if (*ap == '<') {
		k = strlen(ap);
		if (*(ap+k-1) == '>') {
		    ap[k-1] = NUL;
		    ap++;
		}
	    }
	} else
	  failed = 1;
	/* Here we might also want check against "*@*.*" */
#endif	/* COMMENt */
      xemail:
	ckstrncpy(fnval,ap,FNVALL);
	goto fnend;
    }

#ifdef SEEK_CUR
/*
   \fpicture():   Get dimensions of GIF or JPG image.
   fdc June 2006
*/
    if (cx == FN_PICTURE) {
	FILE *fp = NULL;
	int c, x, w = 0, h = 0, eof = 0;
	unsigned int i, j, k;
	unsigned char buf[1024];
	char abuf[16], * p, * s;
	char ** ap = NULL;

	p = fnval;			/* Point to result */
	failed = 1;			/* Assume failure */
	if (argn > 1) {
	    int xi;
	    ckstrncpy(abuf,bp[1],16);	/* Get array reference */
	    s = abuf;
	    if (*s == CMDQ) s++;
	    if (fndiags)		/* Default is this error message */
	      ckmakmsg(fnval,FNVALL,
		       "<ERROR:ARG_BAD_ARRAY:\\f",fn,"()>",NULL);
	    if (s[0] != '&')		/* "Address" of array */
	      goto fnend;
	    if (s[2])
	      if (s[2] != '[' || s[3] != ']')
		goto fnend;
	    if (s[1] >= 64 && s[1] < 91) /* Convert upper to lower */
	      s[1] += 32;
	    if ((xi = dclarray(s[1],2)) < 0) /* Two elements */
	      goto fnend;
	    ap = a_ptr[xi];		/* Point to array we just declared */
	}
	s = bp[0];			/* Filename */
	failed = 0;			/* From here on we don't fail */
	p[0] = '0';			/* Default return value */
	p[1] = NUL;
	if (!ckmatch("*.{jpg,jpeg,gif}$",s,0,1+4)) /* Appropriate name? */
	  goto fnend;			/* No, fail */
	fp = fopen(s, "r");		/* Open it */
	if (fp == NULL) {		/* Can't, fail */
	    p[0] = '-';
	    p[1] = '1';
	    p[2] = NUL;			/* Return -1 */
	    goto fnend;
	}
	k = strlen(s);
	if (!ckstrcmp(&s[k-4],".gif",4,0)) { /* GIF file */
	    if (fread(buf,1,10,fp) != 10) {
		fclose(fp);
		goto fnend;
	    }
	    /* Check signature */
	    if (ckstrcmp((char *)buf,"GIF87a",6,0) &&
		ckstrcmp((char *)buf,"GIF89a",6,0)) {
		fclose(fp);
		goto fnend;
	    }
	    w = buf[6] + 256 * buf[7];
	    h = buf[8] + 256 * buf[9];
	    goto picend;
	} else if (!ckstrcmp(&s[k-4],".jpg",4,0) || /* JPEG file */
		   !ckstrcmp(&s[k-5],".jpeg",5,0)) {
	    if (fread(buf,1,2,fp) != 2) {
		fclose(fp);
		goto fnend;
	    }
	    if (buf[0] != 0xff || buf[1] != 0xd8) { /* Check signature */
		fclose(fp);
		goto fnend;
	    }
	    eof = 0;
	    while (!eof) {		/* Loop for each marker */
		while (!eof) {		/* Find next marker */
		    c = getc(fp);
		    if (c == (unsigned int)EOF) {
			eof++;
			break;
		    }
		    if (c == 0xff) {
			buf[0] = c;
			c = getc(fp);
			if (c == (unsigned int)EOF) {
			    eof++;
			    break;
			}
			buf[1] = c;
			if (c == 0xd9)
			  eof++;
			if (c >= 0xc0 && c <= 0xfe)
			  break;
		    }
		}
		if (eof) break;
		x = buf[1];
		if (x == 0xc0 || x == 0xc1 || x == 0xc2 || x == 0xc3 ||
		    x == 0xc9 || x == 0xca || x == 0xcb) {
		    if (fread(buf,1,7,fp) != 7) {
			fclose(fp);
			goto fnend;
		    }
		    h = buf[3] * 256 + buf[4];
		    w = buf[5] * 256 + buf[6];
		    goto picend;
		} else {		/* Not a desired field */
		    if (feof(fp)) {
			eof++;
			break;
		    }
		    if (fread(buf,1,2,fp) != 2) { /* Length of this field */
			fclose(fp);
			goto fnend;
		    }
		    j = 256 * buf[0] + buf[1] - 2; /* Skip next field */
		    if (CKFSEEK(fp,(CK_OFF_T)j,SEEK_CUR) != 0) {
			fclose(fp);
			goto fnend;
		    }
		}
	    } 
	}
      picend:
	fclose(fp);
	if (ap) {
	    makestr(&(ap[0]),"2");
	    makestr(&(ap[1]),ckitoa(w));
	    makestr(&(ap[2]),ckitoa(h));
	}
	if (w > 0 && h > 0) {
	    if (w > h) p[0] = '1';
	    if (h >= w) p[0] = '2';
	}
	goto fnend;
    }
#endif	/* SEEK_CUR */

    if (cx == FN_PID) {
	int x = -1;
	if (chknum(bp[0])) {		/* Need numeric argument */
	    int pid;
	    pid = atoi(bp[0]);		/* Convert to int */
#ifdef UNIX
	    if (kill(pid,0) < 0) {	/* Test it */
		if (errno ==
#ifdef ESRCH
		    ESRCH		/* No such process */
#else
		    3
#endif	/* ESRCH */
		    )
		  x = 0;
	    } else			/* Process exists */
	      x = 1;
#endif	/* UNIX */
	}
	sprintf(fnval,"%d",x);		/* SAFE */
	goto fnend;
    }

    if (cx == FN_FUNC) {
	char * s = bp[0];
	p = "0";
        debug(F111,"ffunc",s,argn);
	if (argn > 0) {
	    int x, y;
	    for (p = s; *p; p++) {	/* Chop off trailing parens if any */
		if (*p == '(') {
		    *p = NUL;
		    break;
		}
	    }
	    /* Chop off leading "\\f" or "\f" or "f" */
	    p = s;
	    if (*p == CMDQ)		/* Allow for \\f... */
	      p++;
	    if (*p == CMDQ && (*(p+1) == 'f' || *(p+1) == 'F')) { /* or \f */
		p += 2;
	    } else if (*p == 'f' || *p == 'F') { /* or just f */
		p++;
	    }
	    y = lookup(fnctab,p,nfuncs,&x); /* Look up the result */
	    debug(F111,"ffunc",p,y);
	    p = (y > -1) ? "1" : "0";
	}
	goto fnend;
    }
    if (cx == FN_RECURSE) {
	int t, n;
	char * s;
	fnval[0] = NUL;			/* Default result is empty string */
	s = bp[0];			/* Check for null argument */
	if (!s) s = "";			/* or empty argument */
	if (!*s) goto fnend;		/* in which case return empty string */
        n = FNVALL;			/* Not empty, max size for result */
        s = fnval;			/* Location of result */
	{
	    /* Force VARIABLE-EVALUATION SIMPLE RECURSIVE */
	    /* NOTE: This is vulnerable to SIGINT and whatnot... */
	    int tmp = vareval;		/* Save VARIABLE-EVALUATION setting */
	    vareval = 1;		/* Force it to RECURSIVE */
	    zzstring(bp[0],&s,&n);	/* Expand arg into result space */ 
	    vareval = tmp;		/* Restore VARIABLE-EVALUATION */
	}
	goto fnend;
    }

    if (cx == FN_XLATE) {		/* f_cvtcset() */
#ifdef NOFRILLS
	ckstrncpy(fnval,bp[0],FNVALL);	
#else
#ifndef NOUNICODE
	_PROTOTYP( char * cvtstring, (char *, int, int) );
        char * string, * cset1, * cset2;
	int id1, id2;
#endif	/* NOUNICODE */
        fnval[0] = NUL;	
#ifdef NOUNICODE
	ckstrncpy(fnval,bp[0],FNVALL);	
#else
        string = bp[0] ? bp[0] : "";	/* String to convert */
	if (!*string) goto fnend;	/* It's empty */

        cset1 = bp[1] ? bp[1] : "ascii"; /* Current charset of string */
        cset2 = bp[2] ? bp[2] : "ascii"; /* Charset to convert to */

	id1 = lookup(fcstab,cset1,nfilc,NULL); /* Lookup 'from' set */
	if (id1 < 0) {
            failed = 1;
	    ckmakmsg(fnval,FNVALL,"<ERROR:UNKNOWN_CHARSET:\\f",fn,"()>",NULL);
	    goto fnend;
        }
	id2 = lookup(fcstab,cset2,nfilc,NULL); /* Lookup 'to' set */
	if (id2 < 0) {
            failed = 1;
	    ckmakmsg(fnval,FNVALL,"<ERROR:UNKNOWN_CHARSET:\\f",fn,"()>",NULL);
	    goto fnend;
        }
	string = cvtstring(string,id1,id2);
	ckstrncpy(fnval,string,FNVALL);
#endif	/* NOUNICODE */
#endif	/* NOFRILLS */
	goto fnend;
    }

/* Decode strings containing hex escapes */

    if (cx == FN_UNPCT) {		/* \fdecodehex() */
        char *s1, *s2;
	char *prefix;			/* Can be 1 or 2 chars */
	char buf[3];
	int n = 0, k;

	p = fnval;
	*p = NUL;
        if (argn < 1) goto fnend;	/* Empty string */

	s1 = bp[0] ? bp[0] : "";	/* Original string */
	prefix = bp[1] ? bp[1] : "%%";	/* Hex byte prefix */
	n = (int)strlen(prefix);	/* Length of prefix */
	if (n < 1 || n > 2) {		/* must be 1 or 2 */
	    ckmakmsg(fnval,FNVALL,
		       "<ERROR:INVALID_HEX_PREFIX:\\f",fn,"()>",NULL);
	    goto xunpct;
	}
        while (*s1) {
	    if (!ckstrcmp(s1,prefix,n,0)) { /* Case-independent */
		if (!*(s1+n)) {
		    ckmakmsg(fnval,FNVALL,
			     "<ERROR:INCOMPLETE_SEQUENCE:\\f",fn,"()>",NULL);
		    goto xunpct;
		}
		buf[0] = *(s1+n);	/* First hex character */
		buf[1] = *(s1+n+1);	/* Second hex character */
		buf[2] = NUL;
		if ((k = ckhexbytetoint((char *)buf)) > -1) {
		    *p++ = (char) k;	/* Deposit decoded result */
		    s1 += 2+n;		/* and advance the source pointer */
		} else {		/* Fail on conversion error */
		    ckmakmsg(fnval,FNVALL,
			     "<ERROR:NON_HEX_CHARS:\\f",fn,"()>",NULL);
		    goto xunpct;
		}
	    } else {			/* Not a hex escape sequence */
		*p++ = *s1++;		/* Just copy the character */
	    }
        }
	*p = NUL;			/* Terminate the result string */
        failed = 0;			/* Say we didn't fail */
        p = fnval;			/* Set up result pointer */
        goto fnend;			/* and finish */

      xunpct:				/* Error exit */
	p = fnval;
	failed = 1;
	goto fnend;
    }

/* Check a string for encoding family */

    if (cx == FN_STRINGT) {		/* \fstringtype() */
	p = "UNK";
	switch (scanstring(bp[0])) {
	  case FT_7BIT: p = "7BIT"; break;
	  case FT_8BIT: p = "8BIT"; break;
	  case FT_UTF8: p = "UTF8"; break;
	  case FT_UCS2: p = "UCS2"; break;
	  case FT_TEXT: p = "TEXT"; break;
	  case FT_BIN:  p = "BINARY"; break;
	}
	ckstrncpy(fnval,p,FNVALL);
	p = fnval;
	goto fnend;
    }

/* String compare s1, s2, [ case ], [ start ] , [ len ] */

    if (cx == FN_STRCMP) {
        int docase = 0;			/* Case matters or not */
        int start = 0;			/* Start of substring */
	int len = -1;			/* Length of substring to compare */
	int x; char * s1, * s2;		/* workers */

        p = "0";			/* Return value */
        if (argn == 0) {		/* Two null strings are equal */
	    ckstrncpy(fnval,p,FNVALL);
	    p = fnval;
	    goto fnend;
	}
        if (argn == 1) {		/* Non-null string > null string */
	    p = "1";
	    ckstrncpy(fnval,p,FNVALL);
	    p = fnval;
	    goto fnend;
	}
	if (argn > 2) {
	    s = *(bp[2]) ? evalx(bp[2]) : "0"; /* 0 = caseless */
	    if (chknum(s)) docase = atoi(s);
	    if (argn > 3) {
		s = *(bp[3]) ? evalx(bp[3]) : "1"; /* start is 1-based */
		if (chknum(s)) start = atoi(s);
		if (argn > 4) {
		    s = *(bp[4]) ? evalx(bp[4]) : "-1";	/* -1 = whole thing */
		    if (chknum(s)) len = atoi(s);
		}		
	    }        
	}
	if (start > 0) start--; 	/* start is 0-based internally */
	s1 = bp[0];			/* Get length of first arg */
	x = (int)strlen(s1);
	if (x > start)			/* Point to start position of s1 */
	  s1 += start;
	else
	  s1 = "";
	s2 = bp[1];			/* Get length of second arg */
	x = (int)strlen(s2);
	if (x > start)			/* Point to start position of s2 */
	  s2 += start;
	else
	  s2 = "";
	x = ckstrcmp(s,s2,len,docase);
	p = ckitoa(x);
	ckstrncpy(fnval,p,FNVALL);
	p = fnval;
	goto fnend;
    }

/* Note: when adding new functions remember to update dohfunc in ckuus2.c. */

    failed = 1;
    if (fndiags)
      ckmakmsg(fnval,FNVALL,"<ERROR:UNKNOWN_FUNCTION:\\f",fn,"()>",NULL);

  fnend:
    /* Free temporary storage for aguments */
    for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
    fndepth--;
    if (failed) {                       /* Handle failure */
        debug(F111,"fnend",fnval,errno);
        if (!p) p = "";
        if (p[0]) {
            /* In case this wasn't caught above... */
            k = strlen(p);
            if (p[0] != '<' && p[k-1] != '>') {
                ckmakmsg(fnval,FNVALL,"<ERROR:BAD_ARG:\\f",fn,"()>",NULL);
                p = fnval;
            }
        } else {
            ckmakmsg(fnval,FNVALL,"<ERROR:UNKNOWN:\\f",fn,"()>",NULL);
            p = fnval;
        }
        if (fnerror)                    /* SET FUNCTION ERROR ON */
          fnsuccess = 0;                /* Make command fail (see ckuus5.c) */
        debug(F111,"fneval failed",p,fnsuccess);
        if (fndiags)                    /* SET FUNCTION DIAGNOSTICS ON */
          printf("?%s\n",p);            /* Print error message now. */
        else
          return("");                   /* Return nothing. */
    }
    return(p);
}
#endif /* NOSPL */

static char ckpidbuf[32] = "????";

#ifdef VMS
_PROTOTYP(long zgpid,(void));
#endif /* VMS */

char *
ckgetpid() {                            /* Return pid as string */
#ifdef CK_PID
#ifdef OS2
#define getpid _getpid
    unsigned long zz;
#else
    long zz;
#endif /* OS2 */
#ifdef VMS
    zz = zgpid();
#else
    zz = getpid();
#endif /* VMS */
    sprintf(ckpidbuf,"%ld",zz);         /* SAFE */
#endif /* CK_PID */
    return((char *)ckpidbuf);
}

#ifndef NOSPL
#define EMBUFLEN 128                    /* Error message buffer length */

static char embuf[EMBUFLEN+1];

char *                                  /* Evaluate builtin variable */
nvlook(s) char *s; {
    int x, y, cx;
    long z;
    char *p;
#ifndef NODIAL
    MDMINF * m;
#endif /* NODIAL */
#ifndef NOKVERBS                        /* Keyboard macro material */
    extern int keymac, keymacx;
#endif /* NOKVERBS */
#ifdef CK_LOGIN
    extern int isguest;
#endif /* CK_LOGIN */
    if (!s) s = "";
    x = strlen(s);
    if (fndiags) {                      /* FUNCTION DIAGNOSTIC ON */
        if (x + 32 < EMBUFLEN)
          sprintf(embuf,"<ERROR:NO_SUCH_VARIABLE:\\v(%s)>",s); /* SAFE */
        else
          sprintf(embuf,"<ERROR:NO_SUCH_VARIABLE>"); /* SAFE */
    } else                              /* FUNCTION DIAGNOSTIC OFF */
      embuf[0] = NUL;
    x = VVBUFL;
    p = vvbuf;
    if (zzstring(s,&p,&x) < 0) {        /* e.g. for \v(\%a) */
        y = -1;
    } else {
        s = vvbuf;
        y = lookup(vartab,s,nvars,&x);
    }
    cx = y;                             /* y is too generic */
#ifndef NODIAL
    m = (mdmtyp > 0) ? modemp[mdmtyp] : NULL; /* For \v(m_xxx) variables */
#endif /* NODIAL */

    debug(F101,"nvlook y","",y);

    switch (y) {
      case VN_ARGC:                     /* ARGC */
        sprintf(vvbuf,"%d",maclvl < 0 ? topargc : macargc[maclvl]); /* SAFE */
        return(vvbuf);

      case VN_ARGS:                     /* ARGS */
        sprintf(vvbuf,"%d",xargs);      /* SAFE */
        return(vvbuf);

      case VN_COUN:                     /* COUNT */
        sprintf(vvbuf,"%d",count[cmdlvl]); /* SAFE */
        return(vvbuf);

      case VN_DATE:                     /* DATE */
        ztime(&p);                      /* Get "asctime" string */
        if (p == NULL || *p == NUL) return(NULL);
        vvbuf[0] = p[8];                /* dd */
        vvbuf[1] = p[9];
        vvbuf[2] = SP;
        vvbuf[3] = p[4];                /* mmm */
        vvbuf[4] = p[5];
        vvbuf[5] = p[6];
        vvbuf[6] = SP;
        for (x = 20; x < 24; x++)       /* yyyy */
          vvbuf[x - 13] = p[x];
        vvbuf[11] = NUL;
        return(vvbuf);

      case VN_NDAT:                     /* Numeric date */
        ckstrncpy(vvbuf,zzndate(),VVBUFL);
        return(vvbuf);

      case VN_DIRE:                     /* DIRECTORY */
        s = zgtdir();                   /* Get current directory */
        if (!s)
#ifdef UNIXOROSK
          s = "./";
#else
#ifdef VMS
          s = "[]";
#else
          s = "";
#endif /* VMS */
#endif /* UNIXOROSK */
        ckstrncpy(vvbuf,s,VVBUFL);
        s = vvbuf;
#ifdef UNIXOROSK
        x = strlen(s);
        if (x < VVBUFL - 1) {
            if (s[x-1] != '/') {
                s[x] = '/';
                s[x+1] = NUL;
            }
        }
#endif /* UNIXOROSK */
        return(s);

      case VN_FILE:                     /* filespec */
        return(fspec);

      case VN_HOST:                     /* host name */
        if (*myhost) {                  /* If known */
            return(myhost);             /* return it. */
        } else {                        /* Otherwise */
            ckstrncpy(vvbuf,"unknown",VVBUFL); /* just say "unknown" */
            return(vvbuf);
        }

      case VN_SYST:                     /* System type */
#ifdef UNIX
        ckstrncpy(vvbuf,"UNIX",VVBUFL);
#else
#ifdef VMS
        ckstrncpy(vvbuf,"VMS",VVBUFL);
#else
#ifdef OSK
        ckstrncpy(vvbuf,"OS9/68K",VVBUFL);
#else
#ifdef AMIGA
        ckstrncpy(vvbuf,"Amiga",VVBUFL);
#else
#ifdef MAC
        ckstrncpy(vvbuf,"Macintosh",VVBUFL);
#else
#ifdef OS2
#ifdef NT
        ckstrncpy(vvbuf,"WIN32",VVBUFL) ;
#else /* NT */
        ckstrncpy(vvbuf,"OS/2",VVBUFL);
#endif /* NT */
#else
#ifdef datageneral
        ckstrncpy(vvbuf,"AOS/VS",VVBUFL);
#else
#ifdef GEMDOS
        ckstrncpy(vvbuf,"Atari_ST",VVBUFL);
#else
#ifdef STRATUS
        ckstrncpy(vvbuf,"Stratus_VOS",VVBUFL);
#else
        ckstrncpy(vvbuf,"unknown",VVBUFL);
#endif /* STRATUS */
#endif /* GEMDOS */
#endif /* datageneral */
#endif /* OS2 */
#endif /* MAC */
#endif /* AMIGA */
#endif /* OSK */
#endif /* VMS */
#endif /* UNIX */
        return(vvbuf);

      case VN_SYSV:                     /* System herald */
#ifdef IKSD
#ifdef CK_LOGIN
        if (inserver && isguest)
          return("");
#endif /* CK_LOGIN */
#endif /* IKSD */
        for (x = y = 0; x < VVBUFL; x++) {
            if (ckxsys[x] == SP && y == 0) continue;
            vvbuf[y++] = (char) ((ckxsys[x] == SP) ? '_' : ckxsys[x]);
        }
        vvbuf[y] = NUL;
        return(vvbuf);
    } /* Break up long switch statements... */

    switch(y) {
      case VN_TIME:                     /* TIME. Assumes that ztime returns */
        ztime(&p);                      /* "Thu Feb  8 12:00:00 1990" */
        if (p == NULL || *p == NUL)     /* like asctime()! */
          return("");
        for (x = 11; x < 19; x++)       /* copy hh:mm:ss */
          vvbuf[x - 11] = p[x];         /* to vvbuf */
        vvbuf[8] = NUL;                 /* terminate */
        return(vvbuf);                  /* and return it */

      case VN_NTIM:                     /* Numeric time */
        ztime(&p);                      /* "Thu Feb  8 12:00:00 1990" */
        if (p == NULL || *p == NUL)     /* like asctime()! */
          return(NULL);
        z = atol(p+11) * 3600L + atol(p+14) * 60L + atol(p+17);
        sprintf(vvbuf,"%ld",z);         /* SAFE */
        return(vvbuf);

#ifdef CK_TTYFD
      case VN_TTYF:                     /* TTY file descriptor */
        sprintf(vvbuf,"%d",             /* SAFE */
#ifdef VMS
                vmsttyfd()
#else
                ttyfd
#endif /* VMS */
                );
        return(vvbuf);
#endif /* CK_TTYFD */

      case VN_VERS:                     /* Numeric Kermit version number */
        sprintf(vvbuf,"%ld",vernum);    /* SAFE */
        return(vvbuf);

      case VN_XVNUM:                    /* Product-specific version number */
        sprintf(vvbuf,"%ld",xvernum);   /* SAFE */
        return(vvbuf);

      case VN_HOME:                     /* Home directory */
        return(homepath());

      case VN_IBUF:                     /* INPUT buffer */
        return((char *)inpbuf);

      case VN_ICHR:                     /* INPUT character */
        inchar[1] = NUL;
        return((char *)inchar);

      case VN_ICNT:                     /* INPUT character count */
        sprintf(vvbuf,"%d",incount);    /* SAFE */
        return(vvbuf);

      case VN_SPEE: {                   /* Transmission SPEED */
          long t;
          t = ttgspd();
          if (t < 0L)
            sprintf(vvbuf,"unknown");   /* SAFE */
          else
            sprintf(vvbuf,"%ld",t);     /* SAFE */
          return(vvbuf);
      }

      case VN_SUCC:                     /* SUCCESS flag */
        /* Note inverted sense */
        sprintf(vvbuf,"%d",(success == 0) ? 1 : 0); /* SAFE */
        return(vvbuf);

      case VN_LINE: {                   /* LINE */
#ifdef DEBUG
          if (deblog) {
              debug(F111,"\\v(line) local",ttname,local);
              debug(F111,"\\v(line) inserver","",inserver);
#ifdef TNCODE
              debug(F111,"\\v(line) tcp_incoming","",tcp_incoming);
#endif /* TNCODE */
#ifdef CK_TAPI
              debug(F111,"\\v(line) tttapi","",tttapi);
#endif /* CK_TAPI */
          }
#endif /* DEBUG */

#ifdef CK_TAPI
          if (tttapi) {                 /* If I have made a TAPI connection */
              int i;                    /* return the TAPI device name */
              for (i = 0; i < ntapiline; i++) {
                  if (!strcmp(ttname,tapilinetab[i].kwd)) {
                      p = _tapilinetab[i].kwd;
                      return(p);
                  }
              }
          }
#endif /* CK_TAPI */
#ifndef NOXFER
          if (inserver                  /* If I am a TCP server */
#ifdef TNCODE
              || tcp_incoming
#endif /* TNCODE */
              )
#ifdef TCPSOCKET
            p = ckgetpeer();            /* return peer name */
          else
#endif /* TCPSOCKET */
#endif /* NOXFER */
          if (local)                    /* Otherwise if in local mode */
            p = (char *) ttname;        /* return SET LINE / SET HOST name */
          else                          /* Otherwise */
            p = "";                     /* return empty string */
          if (!p)                       /* In case ckgetpeer() returns */
            p = "";                     /* null pointer... */
          debug(F110,"\\v(line) p",p,0);
          if (!*p)
            p = (char *) ttname;
          return(p);
      }
      case VN_PROG:                     /* Program name */
        return("C-Kermit");

    } /* Break up long switch statements... */

    switch(y) {
      case VN_RET:                      /* Value of most recent RETURN */
        debug(F111,"\\v(return)",mrval[maclvl+1],maclvl+1);
        p = mrval[maclvl+1];
        if (p == NULL) p = "";
        return(p);

      case VN_FFC:                      /* Size of most recent file */
        sprintf(vvbuf, "%s", ckfstoa(ffc)); /* SAFE */
        return(vvbuf);

      case VN_TFC:                      /* Size of most recent file group */
        sprintf(vvbuf, "%s", ckfstoa(tfc)); /* SAFE */
        return(vvbuf);

      case VN_CPU:                      /* CPU type */
#ifdef IKSD
#ifdef CK_LOGIN
        if (inserver && isguest)
          return("");
#endif /* CK_LOGIN */
#endif /* IKSD */
#ifdef OS2
         {
            char * getcpu(void) ;
            return getcpu();
         }
#else /* OS2 */
#ifdef CKCPU
        return(CKCPU);                  /* Traditionally, compile-time value */
#else
#ifdef CK_UTSNAME
        {                               /* But if none, try runtime value */
            extern char unm_mch[];
            return((char *)unm_mch);
        }
#else
        p = getenv("HOSTTYPE");		/* 20091116 */
	if (p) if (*p) return(p);
        return("unknown");
#endif /* CK_UTSNAME */
#endif /* CKCPU */
#endif /* OS2 */

      case VN_CMDL:                     /* Command level */
        sprintf(vvbuf, "%d", cmdlvl);   /* SAFE */
        return(vvbuf);

      case VN_DAY:                      /* Day of week */
        ztime(&p);
        if (p != NULL && *p != NUL)     /* ztime() succeeded. */
          ckstrncpy(vvbuf,p,4);
        else
          vvbuf[0] = NUL;               /* ztime() failed. */
        return(vvbuf);                  /* Return what we got. */

      case VN_NDAY: {                   /* Numeric day of week */
          long k;
          z = mjd(zzndate());           /* Get modified Julian date */
          k = (((int)(z % 7L)) + 3) % 7; /* Get day number */
          sprintf(vvbuf,"%ld",k);       /* SAFE */
          return(vvbuf);
      }

      case VN_LCL:                      /* Local (vs remote) mode */
        ckstrncpy(vvbuf, local ? "1" : "0",VVBUFL);
        return(vvbuf);

      case VN_CMDS:                     /* Command source */
        if (cmdstk[cmdlvl].src == CMD_KB)
          ckstrncpy(vvbuf,"prompt",VVBUFL);
        else if (cmdstk[cmdlvl].src == CMD_MD)
          ckstrncpy(vvbuf,"macro",VVBUFL);
        else if (cmdstk[cmdlvl].src == CMD_TF)
          ckstrncpy(vvbuf,"file",VVBUFL);
        else
          ckstrncpy(vvbuf,"unknown",VVBUFL);
        return(vvbuf);

      case VN_CMDF:                     /* Current command file name */
#ifdef COMMENT                          /* (see comments above) */
        if (tfnam[tlevel]) {            /* (near dblbs declaration) */
            dblbs(tfnam[tlevel],vvbuf,VVBUFL);
            return(vvbuf);
        } else return("");
#else
        if (tlevel < 0)
          return("");
        else
          return(tfnam[tlevel] ? tfnam[tlevel] : "");
#endif /* COMMENT */

      case VN_MAC:                      /* Current macro name */
        return((maclvl > -1) ? m_arg[maclvl][0] : "");

      case VN_EXIT:
        sprintf(vvbuf,"%d",xitsta);     /* SAFE */
        return(vvbuf);

    } /* Break up long switch statements... */

    switch(y) {
      case VN_PRTY: {                   /* Parity */
          char *ss;
          switch (parity) {
            case 0:   ss = "none";  break;
            case 'e': ss = "even";  break;
            case 'm': ss = "mark";  break;
            case 'o': ss = "odd";   break;
            case 's': ss = "space"; break;
            default:  ss = "unknown"; break;
          }
          ckstrncpy(vvbuf,ss,VVBUFL);
          return(vvbuf);
      }

      case VN_DIAL:
        sprintf(vvbuf,"%d",             /* SAFE */
#ifndef NODIAL
                dialsta
#else
                -1
#endif /* NODIAL */
                );
        return(vvbuf);

#ifndef NODIAL
      case VN_DMSG:
#ifdef BIGBUFOK
	ckstrncpy(vvbuf,dialmsg[dialsta],VVBUFL); /* Safe if src == NULL */
#endif	/* BIGBUFOK */
	return((char *)vvbuf);
#endif	/* NODIAL */

#ifdef OS2
      case VN_KEYB:
        ckstrncpy(vvbuf,conkbg(),VVBUFL);
        return(vvbuf);
      case VN_SELCT: {
#ifndef NOLOCAL
          const char * selection = GetSelection();
          return( (char *) (selection ? selection : "" )) ;
#else
          return("");
#endif /* NOLOCAL */
      }
#endif /* OS2 */

#ifndef NOXFER
      case VN_CPS:
        sprintf(vvbuf,"%ld",tfcps);     /* SAFE */
        return(vvbuf);
#endif /* NOXFER */

      case VN_MODE:                     /* File transfer mode */
        switch (binary) {
          case XYFT_T: ckstrncpy(vvbuf,"text",VVBUFL); break;
          case XYFT_B:
          case XYFT_U: ckstrncpy(vvbuf,"binary",VVBUFL); break;
          case XYFT_I: ckstrncpy(vvbuf,"image",VVBUFL); break;
          case XYFT_L: ckstrncpy(vvbuf,"labeled",VVBUFL); break;
          case XYFT_M: ckstrncpy(vvbuf,"macbinary",VVBUFL); break;
          default:     ckstrncpy(vvbuf,"unknown",VVBUFL);
        }
        return(vvbuf);

#ifdef CK_REXX
      case VN_REXX:
        return(rexxbuf);
#endif /* CK_REXX */

      case VN_NEWL:                     /* System newline char or sequence */
#ifdef UNIX
        ckstrncpy(vvbuf,"\n",VVBUFL);
#else
#ifdef datageneral
        ckstrncpy(vvbuf,"\n",VVBUFL);
#else
#ifdef OSK
        ckstrncpy(vvbuf,"\15",VVBUFL);  /* Remember, these are octal... */
#else
#ifdef MAC
        ckstrncpy(vvbuf,"\15",VVBUFL);
#else
#ifdef OS2
        ckstrncpy(vvbuf,"\15\12",VVBUFL);
#else
#ifdef STRATUS
        ckstrncpy(vvbuf,"\n",VVBUFL);
#else
#ifdef VMS
        ckstrncpy(vvbuf,"\15\12",VVBUFL);
#else
#ifdef AMIGA
        ckstrncpy(vvbuf,"\n",VVBUFL);
#else
#ifdef GEMDOS
        ckstrncpy(vvbuf,"\n",VVBUFL);
#else
        ckstrncpy(vvbuf,"\n",VVBUFL);
#endif /* GEMDOS */
#endif /* AMIGA */
#endif /* VMS */
#endif /* STRATUS */
#endif /* OS2 */
#endif /* MAC */
#endif /* OSK */
#endif /* datageneral */
#endif /* UNIX */
        return(vvbuf);

      case VN_ROWS:                     /* ROWS */
      case VN_COLS:                     /* COLS */
        ckstrncpy(vvbuf,(cx == VN_ROWS) ? "24" : "80",VVBUFL); /* Default */
#ifdef CK_TTGWSIZ
#ifdef OS2
        if (tt_cols[VTERM] < 0 || tt_rows[VTERM] < 0)
          ttgwsiz();
        sprintf(vvbuf,"%d",             /* SAFE */
                (cx == VN_ROWS) ? tt_rows[VTERM] : tt_cols[VTERM]);
#else /* OS2 */
        if (ttgwsiz() > 0)              /* Get window size */
          if (tt_cols > 0 && tt_rows > 0) /* sets tt_rows, tt_cols */
            sprintf(vvbuf,"%d",         /* SAFE */
                    (cx == VN_ROWS) ? tt_rows : tt_cols);
#endif /* OS2 */
#endif /* CK_TTGWSIZ */
        return(vvbuf);

      case VN_TTYP:
#ifdef NOTERM
        ckstrncpy(vvbuf,"unknown",VVBUFL);
#else
#ifdef OS2
        sprintf(vvbuf, "%s",            /* SAFE */
                (tt_type >= 0 && tt_type <= max_tt) ?
                tt_info[tt_type].x_name :
                "unknown"
                );
#else
#ifdef MAC
        ckstrncpy(vvbuf,"vt320",VVBUFL);
#else
        p = getenv("TERM");
        ckstrncpy(vvbuf,p ? p : "unknown",VVBUFL+1);
#endif /* MAC */
#endif /* OS2 */
#endif /* NOTERM */
        return(vvbuf);

      case VN_MINP:                     /* MINPUT */
        sprintf(vvbuf, "%d", m_found);  /* SAFE */
        return(vvbuf);
    } /* Break up long switch statements... */

    switch(y) {
      case VN_CONN:                     /* CONNECTION */
        if (!local) {
          ckstrncpy(vvbuf,"remote",VVBUFL);
        } else {
            if (!network)
              ckstrncpy(vvbuf,"serial",VVBUFL);
#ifdef TCPSOCKET
            else if (nettype == NET_TCPB || nettype == NET_TCPA) {
                if (ttnproto == NP_TELNET)
                  ckstrncpy(vvbuf,"tcp/ip_telnet",VVBUFL);
#ifdef CK_SSL
                else if (ttnproto == NP_SSL || ttnproto == NP_SSL_RAW)
                  ckstrncpy(vvbuf,"tcp/ip_ssl",VVBUFL);
                else if (ttnproto == NP_TLS || ttnproto == NP_SSL_RAW)
                  ckstrncpy(vvbuf,"tcp/ip_tls",VVBUFL);
#endif /* CK_SSL */
                else
                  ckstrncpy(vvbuf,"tcp/ip",VVBUFL);
            }
#endif /* TCPSOCKET */
#ifdef SSHBUILTIN
            else if (nettype == NET_SSH)
                  ckstrncpy(vvbuf,"tcp/ip_ssh",VVBUFL);
#endif /* SSHBUILTIN */
#ifdef ANYX25
            else if (nettype == NET_SX25 ||
                     nettype == NET_VX25 ||
                     nettype == NET_IX25
                     )
              ckstrncpy(vvbuf,"x.25",VVBUFL);
#endif /* ANYX25 */
#ifdef DECNET
            else if (nettype == NET_DEC) {
                if (ttnproto == NP_LAT)
                  ckstrncpy(vvbuf,"decnet_lat",VVBUFL);
                else if ( ttnproto == NP_CTERM )
                  ckstrncpy(vvbuf,"decnet_cterm",VVBUFL);
                else
                  ckstrncpy(vvbuf,"decnet",VVBUFL);
            }
#endif /* DECNET */
#ifdef SUPERLAT
            else if (nettype == NET_SLAT)
              ckstrncpy(vvbuf,"superlat",VVBUFL);
#endif /* SUPERLAT */
#ifdef NETFILE
            else if (nettype == NET_FILE)
              ckstrncpy(vvbuf,"local_file",VVBUFL);
#endif /* NETFILE */
#ifdef NETCMD
            else if (nettype == NET_CMD)
              ckstrncpy(vvbuf,"pipe",VVBUFL);
#endif /* NETCMD */
#ifdef NETPTY
            else if (nettype == NET_PTY)
              ckstrncpy(vvbuf,"pseudoterminal",VVBUFL);
#endif /* NETPTY */
#ifdef NETDLL
            else if (nettype == NET_DLL)
              ckstrncpy(vvbuf,"dynamic_link_library",VVBUFL);
#endif /* NETDLL */

#ifdef NPIPE
            else if (nettype == NET_PIPE)
              ckstrncpy(vvbuf,"named_pipe",VVBUFL);
#endif /* NPIPE */
#ifdef CK_NETBIOS
            else if (nettype == NET_BIOS)
              ckstrncpy(vvbuf,"netbios",VVBUFL);
#endif /* CK_NETBIOS */
            else
              ckstrncpy(vvbuf,"unknown",VVBUFL);
        }
        return(vvbuf);

#ifndef NOXFER
      case VN_SYSI:                     /* System ID, Kermit code */
        return((char *)cksysid);
#endif /* NOXFER */

#ifdef OS2
      case VN_SPA: {
          unsigned long space = zdskspace(0);
          if (space > 0 && space < 1024)
            sprintf(vvbuf,"-1");
          else
            sprintf(vvbuf,"%lu",space); /* SAFE */
          return(vvbuf);
      }
#endif /* OS2 */

#ifndef NOXFER
      case VN_QUE: {
          extern char querybuf[];
          return(querybuf);
      }
#endif /* NOXFER */

#ifndef NOCSETS
      case VN_CSET:
#ifdef OS2
        sprintf(vvbuf,"cp%d",os2getcp()); /* SAFE */
#else
        ckstrncpy(vvbuf,fcsinfo[fcharset].keyword,VVBUFL+1);
#endif /* OS2 */
        return(vvbuf);
#endif /* NOCSETS */

#ifdef OS2
      case VN_EXEDIR:
        return(exedir);
      case VN_STAR:
        return(startupdir);
#else
      case VN_EXEDIR:
        return(exedir ? exedir : "");
#ifdef VMSORUNIX
      case VN_STAR:
        return(startupdir);
#endif /* VMSORUNIX */
#endif /* OS2 */

      case VN_INI:
        return(inidir);

      case VN_MDM:
        return(gmdmtyp());

      case VN_EVAL:
        return(evalbuf);

#ifndef NODIAL
      case VN_D_CC:                     /* DIAL COUNTRY-CODE */
        return(diallcc ? diallcc : "");

      case VN_D_AC:                     /* DIAL AREA-CODE */
        return(diallac ? diallac : "");

      case VN_D_IP:                     /* DIAL INTERNATIONAL-PREFIX */
        return(dialixp ? dialixp : "");

      case VN_D_LP:                     /* DIAL LD-PREFIX */
        return(dialldp ? dialldp : "");

      case VN_D_LCP:                    /* DIAL LOCAL-PREFIX */
        return(diallcp ? diallcp : "");

      case VN_D_PXX:                    /* DIAL PBX-EXCHANGE that matched */
        return(matchpxx ? matchpxx : "");
#else
      case VN_D_CC:                     /* DIAL COUNTRY-CODE */
      case VN_D_AC:                     /* DIAL AREA-CODE */
      case VN_D_IP:                     /* DIAL INTERNATIONAL-PREFIX */
      case VN_D_LP:                     /* DIAL LD-PREFIX */
      case VN_D_LCP:                    /* DIAL LOCAL-PREFIX */
      case VN_D_PXX:                    /* DIAL PBX-EXCHANGE */
        return("");
#endif /* NODIAL */
      case VN_UID:
#ifdef UNIX
        {
#ifdef IKSD
            if (inserver)
              return((char *)uidbuf);
            else
#endif /* IKSD */
              if (uidbuf[0])
                return((char *)uidbuf);
              else
                return(whoami());
        }
#else
        return((char *)uidbuf);
#endif /* UNIX */
    } /* Break up long switch statements... */

    switch(y) {
      case VN_PWD:
#ifdef OS2
        if (activecmd == XXOUT || activecmd == XXLNOUT) {
            ckstrncpy(vvbuf,pwbuf,VVBUFL);
            ck_encrypt((char *)vvbuf);
            return((char *)vvbuf);
        } else
#endif /* OS2 */
          return((char *)pwbuf);

      case VN_PRM:
        return((char *)prmbuf);

      case VN_PROTO:
#ifdef NOXFER
        return("none");
#else
#ifdef CK_XYZ
        return(ptab[protocol].p_name);
#else
        return("kermit");
#endif /* CK_XYZ */
#endif /* NOXFER */

#ifndef NOXFER
#ifdef CK_TMPDIR
      case VN_DLDIR:
        return(dldir ? dldir : "");
#endif /* CK_TMPDIR */
#endif /* NOXFER */

#ifndef NODIAL
      case VN_M_INI:                    /* Modem init string */
        return(dialini ? dialini : (m ? m->wake_str : ""));

      case VN_M_DCM:                    /* Modem dial command */
        return(dialcmd ? dialcmd : (m ? m->dial_str : ""));

      case VN_M_DCO:                    /* Modem data compression on */
        return(dialdcon ? dialdcon : (m ? m->dc_on_str : ""));

      case VN_M_DCX:                    /* Modem data compression off */
        return(dialdcoff ? dialdcoff : (m ? m->dc_off_str : ""));

      case VN_M_ECO:                    /* Modem error correction on */
        return(dialecon ? dialecon : (m ? m->ec_on_str : ""));

      case VN_M_ECX:                    /* Modem error correction off */
        return(dialecoff ? dialecoff : (m ? m->ec_off_str : ""));

      case VN_M_AAO:                    /* Modem autoanswer on */
        return(dialaaon ? dialaaon : (m ? m->aa_on_str : ""));

      case VN_M_AAX:                    /* Modem autoanswer off */
        return(dialaaoff ? dialaaoff : (m ? m->aa_off_str : ""));

      case VN_M_HUP:                    /* Modem hangup command */
        return(dialhcmd ? dialhcmd : (m ? m->hup_str : ""));

      case VN_M_HWF:                    /* Modem hardware flow command */
        return(dialhwfc ? dialhwfc : (m ? m->hwfc_str : ""));

      case VN_M_SWF:                    /* Modem software flow command */
        return(dialswfc ? dialswfc : (m ? m->swfc_str : ""));

      case VN_M_NFC:                    /* Modem no flow-control command */
        return(dialnofc ? dialnofc : (m ? m->nofc_str : ""));

      case VN_M_PDM:                    /* Modem pulse dialing mode */
        return(dialpulse ? dialpulse : (m ? m->pulse : ""));

      case VN_M_TDM:                    /* Modem tone dialing mode */
        return(dialtone ? dialtone : (m ? m->tone : ""));

      case VN_M_NAM:                    /* Modem full name */
        return(dialname ? dialname : (m ? m->name : ""));
#else
      case VN_M_INI:                    /* Modem init string */
      case VN_M_DCM:                    /* Modem dial command */
      case VN_M_DCO:                    /* Modem data compression on */
      case VN_M_DCX:                    /* Modem data compression off */
      case VN_M_ECO:                    /* Modem error correction on */
      case VN_M_ECX:                    /* Modem error correction off */
      case VN_M_AAO:                    /* Modem autoanswer on */
      case VN_M_AAX:                    /* Modem autoanswer off */
      case VN_M_HUP:                    /* Modem hangup command */
      case VN_M_HWF:                    /* Modem hardware flow command */
      case VN_M_SWF:                    /* Modem software flow command */
      case VN_M_NFC:                    /* Modem no flow-control command */
      case VN_M_PDM:                    /* Modem pulse dialing mode */
      case VN_M_TDM:                    /* Modem tone dialing mode */
      case VN_M_NAM:
        return("");
#endif /* NODIAL */

      case VN_ISTAT:                    /* INPUT status */
        sprintf(vvbuf, "%d", instatus); /* SAFE */
        return(vvbuf);

      case VN_TEMP:                     /* Temporary directory */
        if (tempdir) {
            p = tempdir;
        } else {
#ifdef OS2
#ifdef NT
            p = getenv("K95TMP");
#else
            p = getenv("K2TMP");
#endif /* NT */
            if ( !p )
#endif /* OS2 */
              p = getenv("CK_TMP");
            if (!p) p = getenv("TMPDIR");
            if (!p) p = getenv("TEMP");
            if (!p) p = getenv("TMP");

#ifdef OS2ORUNIX
            if (p) {
                int len = strlen(p);
                if (p[len-1] != '/'
#ifdef OS2
                    && p[len-1] != '\\'
#endif /* OS2 */
                     ) {
                    static char foo[CKMAXPATH];
                    ckstrncpy(foo,p,CKMAXPATH);
                    ckstrncat(foo,"/",CKMAXPATH);
                    p = foo;
                }
            } else
#else /* OS2ORUNIX */
            if (!p)
#endif /* OS2ORUNIX */
#ifdef UNIX                             /* Systems that have a standard */
              p = "/tmp/";              /* temporary directory... */
#else
#ifdef datageneral
              p = ":TMP:";
#else
              p = "";
#endif /* datageneral */
#endif /* UNIX */
        }
        ckstrncpy(vvbuf,p,VVBUFL);
        p = vvbuf;

/* This needs generalizing for VOS, AOS/VS, etc... */

        while (*p) {
#ifdef OS2
            if (*p == '\\') *p = '/';
#endif /* OS2 */
            p++;
        }
#ifndef VMS
        if (p > vvbuf) {
            char c =                    /* Directory termination character */
#ifdef MAC
              ':'
#else
#ifdef datageneral
              ':'
#else
#ifdef STRATUS
              '>'
#else
              '/'
#endif /* STRATUS */
#endif /* datageneral */
#endif /* MAC */
                ;

            if (*(p-1) != c) {
                *p++ = c;
                *p = NUL;
            }
        }
#endif /* VMS */
        return(vvbuf);
    } /* Break up long switch statements... */

    switch(y) {
      case VN_ERRNO:                    /* Error number */
#ifdef VMS
        {
            extern int vms_lasterr;
            sprintf(vvbuf, "%d", vms_lasterr); /* SAFE */
        }
#else
        sprintf(vvbuf, "%d", errno);    /* SAFE */
#endif /* VMS */
        return(vvbuf);

      case VN_ERSTR:                    /* Error string */
        ckstrncpy(vvbuf,ck_errstr(),VVBUFL);
        return(vvbuf);

#ifndef NOXFER
      case VN_RPSIZ:                    /* RECEIVE packet-length */
        sprintf(vvbuf,"%d",urpsiz);     /* SAFE */
        return(vvbuf);

      case VN_WINDO:                    /* WINDOW slots */
        sprintf(vvbuf,"%d",wslotr);     /* SAFE */
        return(vvbuf);
#endif /* NOXFER */

      case VN_TFLN:                     /* TAKE-file line number */
        if (tlevel > -1) {
            sprintf(vvbuf, "%d", tfline[tlevel]); /* SAFE */
            return(vvbuf);
        } else
          return("0");

      case VN_MDMSG:                    /* DIALRESULT */
#ifndef NODIAL
        return((char *)modemmsg);
#else
        return("");
#endif /* NODIAL */

      case VN_DNUM:                     /* DIALNUMBER */
#ifndef NODIAL
        return(dialnum ? (char *) dialnum : "");
#else
        return("");
#endif /* NODIAL */

      case VN_APC:
        sprintf(vvbuf, "%d",            /* SAFE */
#ifdef CK_APC
                apcactive
#else
                0
#endif /* CK_APC */
                );
        return((char *)vvbuf);

#ifdef OS2
#ifndef NOKVERBS
      case VN_TRMK:
          sprintf(vvbuf, "%d", keymac); /* SAFE */
        return((char *)vvbuf);
#endif /* NOKVERBS */
#endif /* OS2 */

      case VN_IPADDR:
#ifdef TCPSOCKET
#ifndef OSK
      /* This dumps core on OS-9 for some reason, but only if executed */
      /* before we have made a TCP connection.  This is obviously not */
      /* the ideal fix. */
        if (!myipaddr[0])
          getlocalipaddr();
#endif /* OSK */
#endif /* TCPSOCKET */
        ckstrncpy(vvbuf,
#ifdef TCPSOCKET
                (char *)myipaddr,
#else
                "",
#endif /* TCPSOCKET */
                VVBUFL);
        return((char *)vvbuf);

#ifndef NOXFER
      case VN_CRC16:                    /* CRC-16 of most recent transfer */
        sprintf(vvbuf,"%d",crc16);      /* SAFE */
        return(vvbuf);
#endif /* NOXFER */

#ifdef CK_PID
      case VN_PID:
#ifdef IKSD
#ifdef CK_LOGIN
        if (inserver && isguest)
          return("");
#endif /* CK_LOGIN */
#endif /* IKSD */
        return(ckgetpid());
#endif /* CK_PID */

#ifndef NOXFER
      case VN_FNAM: {                   /* \v(filename) */
          extern char filnam[], ofn1[], *sfspec, *rrfspec;
          char * tmp;
          switch (what) {               /* File transfer is in progress */
#ifdef NEWFTP
            case (W_FTP|W_RECV):
            case (W_FTP|W_SEND):
              return((char *)filnam);
#endif /* NEWFTP */
            case W_RECV:
            case W_REMO:
              return((char *)ofn1);
            default:                    /* Most recent file transferred */
              if (filnam[0]) {          /* (if any) */
                  return((char *)filnam);
              } else if (lastxfer & W_SEND && sfspec) {
                  if (fnspath == PATH_OFF)
                    zstrip(sfspec,&tmp);
                  else
                    tmp = sfspec;
                  return(tmp);
              } else if (lastxfer & W_RECV && rrfspec) {
                  if (fnrpath == PATH_OFF)
                    zstrip(rrfspec,&tmp);
                  else
                    tmp = rrfspec;
                  return(tmp);
              } else
                return("");
          }
      }
      case VN_FNUM:                     /* \v(filenum) */
        sprintf(vvbuf,"%ld",filcnt);    /* SAFE */
        return((char *)vvbuf);
#endif /* NOXFER */

#ifdef PEXITSTAT
      case VN_PEXIT: {
          extern int pexitstat;
          sprintf(vvbuf,"%d",pexitstat); /* SAFE */
          return((char *)vvbuf);
      }
#endif /* PEXITSTAT */

#ifndef NOXFER
      case VN_P_8BIT:
        vvbuf[0] = parity ? ebq : NUL;
        vvbuf[1] = NUL;
        return((char *)vvbuf);

      case VN_P_CTL: {
          extern CHAR myctlq;
          vvbuf[0] = myctlq;
          vvbuf[1] = NUL;
          return((char *)vvbuf);
      }
      case VN_P_RPT: {
          extern int rptena;
          vvbuf[0] = rptena ? rptq : NUL;
          vvbuf[1] = NUL;
          return((char *)vvbuf);
      }
#endif /* NOXFER */

#ifdef OS2
      case VN_REGN:
        return(get_reg_name());
      case VN_REGO:
        return(get_reg_corp());
      case VN_REGS:
        return(get_reg_sn());
#endif /* OS2 */
    } /* Break up long switch statements... */

    switch(y) {
      case VN_XPROG:
#ifdef OS2
#ifdef NT
#ifdef KUI
        return("K-95G");
#else
        return("K-95");
#endif /* KUI */
#else
        return("K/2");
#endif /* NT */
#else
        return("C-Kermit");
#endif /* OS2 */

      case VN_EDITOR:
#ifdef NOFRILLS
        return("");
#else
#ifdef NOPUSH
        return("");
#else
        {
            extern char editor[];
            char *ss;
            if (!editor[0]) {
                ss = getenv("EDITOR");
                if (ss) {
                    ckstrncpy(editor,ss,CKMAXPATH);
                }
            }
            debug(F110,"\\v(editor)",editor,0);
            return(editor[0] ? (char *)editor : "");
        }
#endif /* NOPUSH */
#endif /* NOFRILLS */

      case VN_EDOPT:
#ifdef NOFRILLS
        return("");
#else
#ifdef NOPUSH
        return("");
#else
        {
            extern char editopts[];
            return(editopts[0] ? (char *)editopts : "");
        }
#endif /* NOPUSH */
#endif /* NOFRILLS */

      case VN_EDFILE:
#ifdef NOFRILLS
        return("");
#else
#ifdef NOPUSH
        return("");
#else
        {
            extern char editfile[];
            return(editfile[0] ? (char *)editfile : "");
        }
#endif /* NOPUSH */
#endif /* NOFRILLS */

#ifdef BROWSER
      case VN_BROWSR: {
          extern char browser[];
          if (!browser[0]) {
              s = getenv("BROWSER");
              if (s) ckstrncpy(browser,s,CKMAXPATH);
          }
          return(browser[0] ? (char *)browser : "");
      }
      case VN_BROPT: {
          extern char browsopts[];
          return(browsopts[0] ? (char *)browsopts : "");
      }
      case VN_URL: {
          extern char browsurl[];
          return(browsurl[0] ? (char *)browsurl : "");
      }
#endif /* BROWSER */
      case VN_HERALD:
        return((char *)versio);

      case VN_TEST: {                   /* test */
          extern char * ck_s_test, * ck_s_tver;
          if (!ck_s_test) ck_s_test = "";
          if (!ck_s_tver) ck_s_tver = "";
          if (*ck_s_test) {
              ckstrncpy(vvbuf,ck_s_test,VVBUFL);
              if (*ck_s_tver) {
                  ckstrncat(vvbuf,".",VVBUFL);
                  ckstrncat(vvbuf,ck_s_tver,VVBUFL);
              }
          } else
            ckstrncpy(vvbuf,"0",VVBUFL);
          return((char *)vvbuf);
      }

#ifndef NOXFER
      case VN_XFSTAT:                   /* xferstatus */
        x = xferstat;                   /* Like success */
        if (x > -1) x = (x == 0) ? 1 : 0; /* External value is reversed */
        sprintf(vvbuf,"%d",x);          /* SAFE */
        return((char *)vvbuf);

      case VN_XFMSG:                    /* xfermsg */
        return((char *)epktmsg);

#ifndef NOMSEND
      case VN_SNDL: {                   /* sendlist */
          extern int filesinlist;
          sprintf(vvbuf,"%d",filesinlist); /* SAFE */
          return((char *)vvbuf);
      }
#endif /* NOMSEND */
#endif /* NOXFER */

#ifdef CK_TRIGGER
      case VN_TRIG: {
          extern char * triggerval;
          return(triggerval ? triggerval : "");
      }
#endif /* CK_TRIGGER */
#ifdef OS2MOUSE
#ifdef OS2
      case VN_MOU_X: {
          extern int MouseCurX;
          sprintf(vvbuf,"%d",MouseCurX); /* SAFE */
          return((char *)vvbuf);
      }
      case VN_MOU_Y: {
          extern int MouseCurY;
          sprintf(vvbuf,"%d",MouseCurY); /* SAFE */
          return((char *)vvbuf);
      }
#endif /* OS2 */
#endif /* OS2MOUSE */
      case VN_PRINT: {
          extern int printpipe;
          extern char * printername;
#ifdef PRINTSWI
          extern int noprinter;
          if (noprinter) return("");
#endif /* PRINTSWI */
          ckmakmsg(vvbuf,VVBUFL,
                   printpipe ? "|" : "",
                   printername ? printername :
#ifdef OS2
                   "PRN",
#else
                   "(default)",
#endif /* OS2 */
                   NULL,
                   NULL
                   );
          return((char *)vvbuf);
      }
    } /* Break up long switch statements... */

    switch(y) {
      case VN_ESC:                      /* Escape character */
        sprintf(vvbuf,"%d",escape);     /* SAFE */
        return((char *)vvbuf);

      case VN_INTIME:
        sprintf(vvbuf,"%ld",inetime);   /* SAFE */
        return((char *)vvbuf);

      case VN_INTMO:
        sprintf(vvbuf,"%d",inwait);     /* SAFE */
        return((char *)vvbuf);

      case VN_SECURE:
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
            )
          return("1");
        else
          return("0");

      case VN_AUTHN:
#ifdef CK_AUTHENTICATION
        {
            extern char szUserNameAuthenticated[];
            return((char *)szUserNameAuthenticated);
        }
#else /* CK_AUTHENTICATION */
        return((char *)"");
#endif /* CK_AUTHENTICATION */

      case VN_AUTHS:
#ifdef CK_AUTHENTICATION
        switch (ck_tn_auth_valid()) {
          case AUTH_UNKNOWN:
            return((char *)"unknown");
          case AUTH_OTHER:
            return((char *)"other");
          case AUTH_USER:
            return((char *)"user");
          case AUTH_VALID:
            return((char *)"valid");
          case AUTH_REJECT:
          default:
            return((char *)"rejected");
        }
#else /* CK_AUTHENTICATION */
        return((char *)"rejected");
#endif /* CK_AUTHENTICATION */

      case VN_AUTHT:
#ifdef CK_AUTHENTICATION
#ifdef CK_SSL
        if ((ssl_active_flag || tls_active_flag) &&
            ck_tn_auth_valid() == AUTH_VALID &&
            (sstelnet ? (!TELOPT_U(TELOPT_AUTHENTICATION)) :
                        (!TELOPT_ME(TELOPT_AUTHENTICATION))) ||
             ck_tn_authenticated() == AUTHTYPE_NULL ||
             ck_tn_authenticated() == AUTHTYPE_AUTO)
          return("X_509_CERTIFICATE");
        else
#endif /* CK_SSL */
          return(AUTHTYPE_NAME(ck_tn_authenticated()));
#else /* CK_AUTHENTICATION */
        return((char *)"NULL");
#endif /* CK_AUTHENTICATION */

#ifdef CK_KERBEROS
      case VN_K4PRN: {
          extern char * krb4_d_principal;
          if (krb4_d_principal)
            ckstrncpy(vvbuf,krb4_d_principal,VVBUFL+1);
          else
            *vvbuf = NUL;
          return((char *)vvbuf);
      }
      case VN_K5PRN: {
          extern char * krb5_d_principal;
          if (krb5_d_principal)
            ckstrncpy(vvbuf,krb5_d_principal,VVBUFL+1);
          else
            *vvbuf = NUL;
          return((char *)vvbuf);
      }
      case VN_K4RLM: {
          extern char * krb4_d_realm;
          if (krb4_d_realm) {
              ckstrncpy(vvbuf,krb4_d_realm,VVBUFL+1);
          } else {
              char * s = ck_krb4_getrealm();
              ckstrncpy(vvbuf,s ? s : "",VVBUFL+1);
          }
          return((char *)vvbuf);
      }
      case VN_K4SRV: {
          extern char * krb4_d_srv;
          if (krb4_d_srv)
            ckstrncpy(vvbuf,krb4_d_srv,VVBUFL+1);
          else
            ckstrncpy(vvbuf,"rcmd",VVBUFL);
          return((char *)vvbuf);
      }
      case VN_K5RLM: {
          extern char * krb5_d_realm;
          extern char * krb5_d_cc;
          if (krb5_d_realm) {
              ckstrncpy(vvbuf,krb5_d_realm,VVBUFL+1);
          } else {
              char * s = ck_krb5_getrealm(krb5_d_cc);
              ckstrncpy(vvbuf,s,VVBUFL+1);
          }
          return((char *)vvbuf);
      }
      case VN_K5CC: {
          extern char * krb5_d_cc;
          if (krb5_d_cc)
            ckstrncpy(vvbuf,krb5_d_cc,VVBUFL+1);
          else
            ckstrncpy(vvbuf,ck_krb5_get_cc_name(),VVBUFL+1);
          return((char *)vvbuf);
      }
      case VN_K5SRV: {
          extern char * krb5_d_srv;
          if (krb5_d_srv)
            ckstrncpy(vvbuf,krb5_d_srv,VVBUFL+1);
          else
            ckstrncpy(vvbuf,"host",VVBUFL);
          return((char *)vvbuf);
      }
      case VN_K4ENO: {
        extern int krb4_errno;
        sprintf(vvbuf,"%d",krb4_errno); /* SAFE */
        return((char *)vvbuf);
      }
      case VN_K5ENO: {
        extern int krb5_errno;
        sprintf(vvbuf,"%d",krb5_errno); /* SAFE */
        return((char *)vvbuf);
      }
      case VN_K4EMSG: {
        extern char * krb4_errmsg;
        ckstrncpy(vvbuf,krb4_errmsg?krb4_errmsg:"",VVBUFL+1);
        return((char *)vvbuf);
      }
      case VN_K5EMSG: {
        extern char * krb5_errmsg;
        ckstrncpy(vvbuf,krb5_errmsg,VVBUFL+1);
        return((char *)vvbuf);
      }
#endif /* CK_KERBEROS */
#ifdef CK_SSL
      case VN_X509_S:
        if (ssl_active_flag)
          ckstrncpy(vvbuf,ssl_get_subject_name(ssl_con),VVBUFL+1);
        else if (tls_active_flag)
          ckstrncpy(vvbuf,ssl_get_subject_name(tls_con),VVBUFL+1);
        else
          ckstrncpy(vvbuf,"",VVBUFL+1);
        return((char *)vvbuf);
      case VN_X509_I:
        if (ssl_active_flag)
          ckstrncpy(vvbuf,ssl_get_issuer_name(ssl_con),VVBUFL+1);
        else if (tls_active_flag)
          ckstrncpy(vvbuf,ssl_get_issuer_name(tls_con),VVBUFL+1);
        else
          ckstrncpy(vvbuf,"",VVBUFL+1);
        return((char *)vvbuf);
#endif /* CK_SSL */

      case VN_OSNAM:
#ifdef IKSD
#ifdef CK_LOGIN
        if (inserver && isguest)
          return("");
#endif /* CK_LOGIN */
#endif /* IKSD */
#ifdef CK_UTSNAME
        {
            extern char unm_nam[];
            return((char *)unm_nam);
        }
#else
        for (x = y = 0; x < VVBUFL; x++) {
            if (ckxsys[x] == SP && cx == 0) continue;
            vvbuf[y++] = (char) ((ckxsys[x] == SP) ? '_' : ckxsys[x]);
        }
        vvbuf[y] = NUL;
        return(vvbuf);
#endif /* CK_UTSNAME */

      case VN_OSVER: {
#ifdef CK_UTSNAME
          extern char unm_ver[];
#ifdef IKSD
#ifdef CK_LOGIN
          if (inserver && isguest)
            return("");
#endif /* CK_LOGIN */
#endif /* IKSD */
          return((char *)unm_ver);
#else
          return("");
#endif /* CK_UTSNAME */
      }

      case VN_OSREL: {
#ifdef CK_UTSNAME
          extern char unm_rel[];
#ifdef IKSD
#ifdef CK_LOGIN
          if (inserver && isguest)
            return("");
#endif /* CK_LOGIN */
#endif /* IKSD */
          return((char *)unm_rel);
#else
          return("");
#endif /* CK_UTSNAME */
      }
    } /* Break up long switch statements... */

    switch(y) {
      case VN_NAME: {
          extern char * myname;
          return(myname);
      }

      case VN_MODL: {
#ifdef CK_UTSNAME
          extern char unm_mod[], unm_mch[];
          int y = VVBUFL - 1;
          char * s = unm_mod;
#endif /* CK_UTSNAME */
#ifdef IKSD
#ifdef CK_LOGIN
          if (inserver && isguest)
            return("");
#endif /* CK_LOGIN */
#endif /* IKSD */

#ifdef COMMENT                          /* was HPUX */
          if (!unm_mod[0] && !nopush)
            zzstring("\\fcommand(model)",&s,&y);
/*
   Another possibility would be:
     "\\fcommand(ksh -c 'whence model 1>&- && model || uname -m')"
   But that would depend on having ksh.
*/
#else
#ifdef OSF32                            /* Digital UNIX 3.2 and higher... */
/* Note: Ultrix has /etc/sizer, but it is not publicly executable. */
/* sizer -c outputs 'cpu:<tab><tab>"DECxxxx"' */
          if (!unm_mod[0]) {
              char * p;
              int flag = 0;
              zzstring("\\fcommand(/usr/sbin/sizer -c)",&s,&y);
              debug(F110,"DU model",unm_mod,0);
              s = unm_mod;
              p = unm_mod;
              while (*p) {              /* Extract the part in quotes */
                  if (*p == '"') {
                      if (flag)
                        break;
                      flag = 1;
                      p++;
                      continue;
                  }
                  if (flag)
                    *s++ = *p;
                  p++;
              }
              *s = NUL;
          }
#endif /* OSF32 */
#endif /* COMMENT */

#ifdef CK_UTSNAME
          if (unm_mod[0])
            return((char *)unm_mod);
          else
            return((char *)unm_mch);
#else
          return("");
#endif /* CK_UTSNAME */
      }

#ifdef IBMX25
      /* X.25 variables (local and remote address) */
      case VN_X25LA:
        if (!local_nua[0] && !x25local_nua(local_nua))
          *vvbuf = NULL;
        else
          ckstrncpy(vvbuf,local_nua,VVBUFL+1);
        return((char *)vvbuf);

      case VN_X25RA:
        if (!remote_nua[0])
          *vvbuf = NULL;
        else
          ckstrncpy(vvbuf,remote_nua,VVBUFL+1);
        return((char *)vvbuf);
#endif /* IBMX25 */

#ifndef NODIAL
      case VN_PDSFX: {
          extern char pdsfx[];
          return((char *)pdsfx);
      }
      case VN_DTYPE: {
          extern int dialtype;
          sprintf(vvbuf,"%d",dialtype); /* SAFE */
          return((char *)vvbuf);
      }
#endif /* NODIAL */

#ifdef UNIX
      case VN_LCKPID: {
          extern char lockpid[];
          return((char *)lockpid);
      }
#endif /* UNIX */

#ifndef NOXFER
      case VN_BLK:
        sprintf(vvbuf,"%d",bctr);       /* SAFE */
        return((char *)vvbuf);

      case VN_TFTIM:
        sprintf(vvbuf,                  /* SAFE */
#ifdef GFTIMER
                "%ld", (long)(fptsecs + 0.5)
#else
                "%d", tsecs
#endif /* GFTIMER */
                );
        return((char *)vvbuf);
#endif /* NOXFER */

      case VN_HWPAR:
      case VN_SERIAL: {
          int sb;
          char c, * ss;
          extern int stopbits;
          vvbuf[0] = NUL;
          if (hwparity && local && !network)
            ss = parnam((char)hwparity);
          else
            ss = parnam((char)parity);
          if (cx == VN_HWPAR) {
              ckstrncpy(vvbuf,ss,VVBUFL);
              return((char *)vvbuf);
          }
          c = ss[0];
          if (islower(c)) c = toupper(c);
          sb = stopbits;
          if (sb < 1)
            sb = (speed > 0 && speed <= 110L) ? 2 : 1;
          if (hwparity)
            sprintf(vvbuf," 8%c%d",c,sb); /* SAFE */
          else if (parity)
            sprintf(vvbuf," 7%c%d",c,sb); /* SAFE */
          else
            sprintf(vvbuf," 8N%d",sb);  /* SAFE */
          return((char *)vvbuf);
      }

#ifdef UNIX
      case VN_LCKDIR: {
#ifndef NOUUCP
          extern char * uucplockdir;
          ckstrncpy(vvbuf,uucplockdir,VVBUFL);
          x = strlen(vvbuf);
          if (x > 0) {
              if (vvbuf[x-1] != '/') {
                  vvbuf[x] = '/';
                  vvbuf[x+1] = NUL;
              }
          }
#else
          vvbuf[0] = NUL;
#endif /* NOUUCP */
          return((char *)vvbuf);
      }
#endif /* UNIX */
    } /* Break up long switch statements... */

    switch(y) {
#ifndef NODIAL
      case VN_DM_LP:
      case VN_DM_SP:
      case VN_DM_PD:
      case VN_DM_TD:
      case VN_DM_WA:
      case VN_DM_WD:
      case VN_DM_HF:
      case VN_DM_WB:
      case VN_DM_RC: {
          extern char * getdm();
          ckstrncpy(vvbuf,getdm(y),VVBUFL);
          return((char *)vvbuf);
      }
#endif /* NODIAL */

      case VN_TY_LN:
      case VN_TY_LC: {
          extern int typ_lines;
          sprintf(vvbuf,"%d",typ_lines); /* SAFE */
          return((char *)vvbuf);
      }
      case VN_TY_LM: {
          extern int typ_mtchs;
          sprintf(vvbuf,"%d",typ_mtchs); /* SAFE */
          return((char *)vvbuf);
      }
      case VN_MACLVL:
        sprintf(vvbuf,"%d",maclvl);     /* SAFE */
        return((char *)vvbuf);
    } /* Break up long switch statements... */

    switch(y) {
#ifndef NOLASTFILE
      case VN_LASTFIL: {
	  extern char * lastfile;
	  return(lastfile ? lastfile : "");
      }
#endif	/* NOLASTFILE */
#ifndef NOXFER
      case VN_XF_BC:
        sprintf(vvbuf,"%d",crunched);   /* SAFE */
        return((char *)vvbuf);

      case VN_XF_TM:
        sprintf(vvbuf,"%d",timeouts);   /* SAFE */
        return((char *)vvbuf);

      case VN_XF_RX:
        sprintf(vvbuf,"%d",retrans);    /* SAFE */
        return((char *)vvbuf);
#endif /* NOXFER */

      case VN_MS_CD:                    /* Modem signals */
      case VN_MS_CTS:
      case VN_MS_DSR:
      case VN_MS_DTR:
      case VN_MS_RI:
      case VN_MS_RTS: {
          int x, z = -1;
          x = ttgmdm();                 /* Try to get them */
          if (x > -1) {
              switch (y) {
                case VN_MS_CD:  z = (x & BM_DCD) ? 1 : 0; break;
                case VN_MS_DSR: z = (x & BM_DSR) ? 1 : 0; break;
                case VN_MS_CTS: z = (x & BM_CTS) ? 1 : 0; break;
#ifdef MAC
                case VN_MS_DTR: z = (x & BM_DTR) ? 1 : 0; break;
#else
#ifndef STRATUS
                case VN_MS_RI:  z = (x & BM_RNG) ? 1 : 0; break;
#ifndef NT
                case VN_MS_DTR: z = (x & BM_DTR) ? 1 : 0; break;
                case VN_MS_RTS: z = (x & BM_RTS) ? 1 : 0; break;
#endif /* NT */
#endif /* STRATUS */
#endif /* MAC */
              }
          }
          sprintf(vvbuf,"%d",z);        /* SAFE */
          return((char *)vvbuf);
      }
      case VN_MATCH:                    /* INPUT MATCH */
        return(inpmatch ? inpmatch : "");

#ifdef CKFLOAT
      case VN_ISCALE:			/* INPUT SCALE-FACTOR */
        return(inpscale ? inpscale : "1.0");
#endif	/* CKFLOAT */

      case VN_SLMSG: {                  /* SET LINE / HOST message */
          extern char * slmsg;
          vvbuf[0] = NUL;
          if (slmsg)
            ckstrncpy(vvbuf,slmsg,VVBUFL);
         return(vvbuf);
      }

      case VN_TXTDIR:                   /* TEXTDIR */
        return(k_info_dir ? k_info_dir : "");

#ifdef FNFLOAT
      case VN_MA_PI:
        return(math_pi);

      case VN_MA_E:
        return(math_e);

      case VN_MA_PR:
        sprintf(vvbuf,"%d",fp_digits);  /* SAFE */
        return(vvbuf);
#endif /* FNFLOAT */

      case VN_CMDBL:
        sprintf(vvbuf,"%d",CMDBL);      /* SAFE */
        return(vvbuf);

#ifdef CKCHANNELIO
      case VN_FERR: {
          extern int z_error;
          sprintf(vvbuf,"%d",z_error);  /* SAFE */
          return(vvbuf);
      }
      case VN_FMAX: {
          extern int z_maxchan;
          sprintf(vvbuf,"%d",z_maxchan); /* SAFE */
          return(vvbuf);
      }
      case VN_FCOU: {
          extern int z_filcount;
          sprintf(vvbuf,"%d",z_filcount); /* SAFE */
          return(vvbuf);
      }
#endif /* CKCHANNELIO */

#ifndef NODIAL
      case VN_DRTR: {
          extern int dialcount;
          sprintf(vvbuf,"%d",dialcount); /* SAFE */
          return(vvbuf);
      }
#endif /* NODIAL */

#ifndef NOLOGDIAL
#ifndef NOLOCAL
      case VN_CXTIME:
        sprintf(vvbuf,"%ld",dologshow(0)); /* SAFE */
        return(vvbuf);
#endif /* NOLOCAL */
#endif /* NOLOGDIAL */

      case VN_BYTE:
        sprintf(vvbuf,"%d",byteorder);  /* SAFE */
        return(vvbuf);

      case VN_KBCHAR:
        vvbuf[0] = NUL;
        vvbuf[1] = NUL;
        if (kbchar > 0)
          vvbuf[0] = (kbchar & 0xff);
        return(vvbuf);

      case VN_TTYNAM: {
#ifdef HAVECTTNAM
          extern char cttnam[];
          return((char *)cttnam);
#else
          return(CTTNAM);
#endif /* HAVECTTNAM */
      }

      case VN_PROMPT:
        return(cmgetp());

      case VN_BUILD: {
          extern char * buildid;
          return(buildid);
      }

#ifndef NOSEXP
      case VN_SEXP: {
          extern char * lastsexp;
          return(lastsexp ? lastsexp : "");
      }
      case VN_VSEXP: {
          extern char * sexpval;
          return(sexpval ? sexpval : "");
      }
      case VN_LSEXP: {
          extern int sexpdep;
          ckstrncpy(vvbuf,ckitoa(sexpdep),VVBUFL);
          return(vvbuf);
      }
#endif /* NOSEXP */

#ifdef GFTIMER
      case VN_FTIME: {
          CKFLOAT f;
          ztime(&p);
          if (p == NULL || *p == NUL)
            return(NULL);
          z = atol(p+11) * 3600L + atol(p+14) * 60L + atol(p+17);
          f = (CKFLOAT)z + ((CKFLOAT)ztusec) / 1000000.0;
          sprintf(vvbuf,"%f",f);        /* SAFE */
          return(vvbuf);
      }
#endif /* GFTIMER */

#ifndef NOHTTP
      case VN_HTTP_C: {                 /* HTTP Code */
          extern int http_code;
          return(ckitoa(http_code));
      }
      case VN_HTTP_N:                   /* HTTP Connected */
        return( http_isconnected() ? "1" : "0");
      case VN_HTTP_H:                   /* HTTP Host */
        return( (char *)http_host() );
      case VN_HTTP_M: {                 /* HTTP Message */
          extern char http_reply_str[];
          return((char *)http_reply_str);
      }
      case VN_HTTP_S:                   /* HTTP Security */
        return((char *)http_security());
#endif /* NOHTTP */

#ifdef NEWFTP
      case VN_FTP_B:
        return((char *)ftp_cpl_mode());
      case VN_FTP_D:
        return((char *)ftp_dpl_mode());
      case VN_FTP_Z:
        return((char *)ftp_authtype());
      case VN_FTP_C: {
          extern int ftpcode;
          return(ckitoa(ftpcode));
      }
      case VN_FTP_M: {
          extern char ftp_reply_str[];
          if (isdigit(ftp_reply_str[0]) &&
              isdigit(ftp_reply_str[1]) &&
              isdigit(ftp_reply_str[2]) &&
              ftp_reply_str[3] == ' ')
            return(&ftp_reply_str[4]);
          else
            return(ftp_reply_str);
      }
      case VN_FTP_S: {
          extern char ftp_srvtyp[];
          return((char *)ftp_srvtyp);
      }
      case VN_FTP_H: {
          extern char * ftp_host;
          return(ftp_host ? ftp_host : "");
      }
      case VN_FTP_X: {                  /* FTP Connected */
          return(ftpisconnected() ? "1" : "0");
      }
      case VN_FTP_L: {                  /* FTP Logged in */
          return(ftpisloggedin() ? "1" : "0");
      }
      case VN_FTP_G: {                  /* FTP GET-PUT-REMOTE */
          extern int ftpget;
          char * s = "";
          switch (ftpget) {
            case 0: s = "kermit"; break;
            case 1: s = "ftp"; break;
            case 2: s = "auto"; break;
          }
          return(s);
      }
#endif /* NEWFTP */

#ifndef NOLOCAL
      case VN_CX_STA: {                 /* CONNECT status */
          extern int cx_status;
          return(ckitoa(cx_status));
      }
#endif /* NOLOCAL */
      case VN_NOW:                      /* Timestamp */
        return(ckcvtdate(p,0));

      case VN_HOUR:                     /* Hour of the day */
        ztime(&p);                      /* "Thu Feb  8 12:00:00 1990" */
        if (!p) p = "";
        if (!*p) return(p);
        vvbuf[0] = p[11];
        vvbuf[1] = p[12];
        vvbuf[2] = NUL;
        return(vvbuf);                  /* and return it */

      case VN_BITS:			/* Bits (16, 32, 64) */
	if (sizeof(long) > 4)
	  return(ckitoa(8*sizeof(long)));
	else
	  return(ckitoa(8*sizeof(int)));

      case VN_LASTKWV:			/* 212 */
	return(lastkwval ? lastkwval : "");

      case VN_HOSTIP: {			/* 212 */
#ifdef TCPSOCKET
	  extern char hostipaddr[];
	  return((char *)hostipaddr);
#else
	  return("");
#endif	/* TCPSOCKET */
      }
      case VN_INPMSG:
	switch (instatus) {
	  case INP_OK:  return("SUCCESS");
	  case INP_TO:  return("Timed out");
	  case INP_UI:  return("Keyboard interrupt");
	  case INP_IE:  return("Internal error");
	  case INP_IO:  return("I/O error or connection lost");
	  case INP_IKS: return("INPUT disabled");
	  case INP_BF:  return("Buffer filled and /NOWRAP set");
	  default:      return("Unknown");
	}

      case VN_VAREVAL:			/* 212 */
	return(vareval ? "recursive" : "simple");

      case VN_LOG_CON:			/* \v(...) for log files */
#ifdef CKLOGDIAL
        return(diafil);
#else 
        return("");
#endif
      case VN_LOG_PKT:
#ifndef NOXFER
        return(pktfil);
#else
        return("");
#endif
      case VN_LOG_SES:
#ifndef NOLOCAL
        return(sesfil);
#else
        return("");
#endif
      case VN_LOG_TRA:
#ifdef TLOG
        return(trafil);
#else
        return("");
#endif
      case VN_LOG_DEB:
#ifdef DEBUG
        return(debfil);
#else
        return("");
#endif
      case VN_PREVCMD: {
	  extern char * prevcmd;
	  return(prevcmd ?  prevcmd : "");
      }
    }

#ifndef NODIAL
    switch (y) {                        /* Caller ID values */
      extern char
        * callid_date, * callid_time, * callid_name,
        * callid_nmbr, * callid_mesg;

      case VN_CI_DA:
        return(callid_date ? callid_date : "");

      case VN_CI_TI:
        return(callid_time ? callid_time : "");

      case VN_CI_NA:
        return(callid_name ? callid_name : "");

      case VN_CI_NU:
        return(callid_nmbr ? callid_nmbr : "");

      case VN_CI_ME:
        return(callid_mesg ? callid_mesg : "");

    } /* End of variable-name switches */
#endif /* NODIAL */

#ifdef NT
    switch (y) {
      case VN_PERSONAL:
        p = (char *)GetPersonal();
        if (p) {
            GetShortPathName(p,vvbuf,VVBUFL);
            return(vvbuf);
        }
        return("");
      case VN_DESKTOP:
          p = (char *)GetDesktop();
          if (p) {
              GetShortPathName(p,vvbuf,VVBUFL);
              return(vvbuf);
          }
          return("");
      case VN_COMMON:
        p = (char *)GetAppData(1);
        if (p) {
            ckmakmsg(vvbuf,VVBUFL,p,"Kermit 95/",NULL,NULL);
            GetShortPathName(vvbuf,vvbuf,VVBUFL);
            return(vvbuf);
        }
        return("");
      case VN_APPDATA:
        p = (char *)GetAppData(0);
        if (p) {
            ckmakmsg(vvbuf,VVBUFL,p,"Kermit 95/",NULL,NULL);
            GetShortPathName(vvbuf,vvbuf,VVBUFL);
            return(vvbuf);
        }
        return("");
    }
#endif /* NT */

#ifdef TN_COMPORT
    switch (y) {
      case VN_TNC_SIG: {
        p = (char *) tnc_get_signature();
        ckstrncpy(vvbuf,p ? p : "",VVBUFL);
        return(vvbuf);
      }
    }
#endif /* TN_COMPORT */

#ifdef KUI
    switch (y) {
      case VN_GUI_RUN: {
	  extern HWND getHwndKUI();
	  if ( IsIconic(getHwndKUI()) )
            return("minimized");
	  if ( IsZoomed(getHwndKUI()) )
            return("maximized");
	  return("restored");
      }
      case VN_GUI_XP:
        sprintf(vvbuf,"%d",get_gui_window_pos_x());  /* SAFE */
        return(vvbuf);
      case VN_GUI_YP:
        sprintf(vvbuf,"%d",get_gui_window_pos_y());  /* SAFE */
        return(vvbuf);
      case VN_GUI_XR:
        sprintf(vvbuf,"%d",GetSystemMetrics(SM_CXSCREEN));  /* SAFE */
        return(vvbuf);
      case VN_GUI_YR:
        sprintf(vvbuf,"%d",GetSystemMetrics(SM_CYSCREEN));  /* SAFE */
        return(vvbuf);
      case VN_GUI_FNM:
          if ( ntermfont > 0 ) {
              int i;
              for (i = 0; i < ntermfont; i++) {
                  if (tt_font == term_font[i].kwval) {
                      ckstrncpy(vvbuf,term_font[i].kwd,VVBUFL);
                      return(vvbuf);
                  }
              }
          }
          return("(unknown)");
      case VN_GUI_FSZ:
          ckstrncpy(vvbuf,ckitoa(tt_font_size/2),VVBUFL);
          if ( tt_font_size % 2 )
              ckstrncat(vvbuf,".5",VVBUFL);
          return(vvbuf);
    }
#endif /* KUI */

    fnsuccess = 0;
    if (fnerror) {
        fnsuccess = 0;
    }
    if (fndiags) {
        if (!embuf[0])
          ckstrncpy(embuf,"<ERROR:NO_SUCH_VARIABLE>",EMBUFLEN);
        printf("?%s\n",embuf);
        return((char *)embuf);
    } else
      return("");
}
#endif /* NOSPL */


/*
  X X S T R I N G  --  Expand variables and backslash codes.

    int xxtstring(s,&s2,&n);

  Expands \ escapes via recursive descent.
  Argument s is a pointer to string to expand (source).
  Argument s2 is the address of where to put result (destination).
  Argument n is the length of the destination string (to prevent overruns).
  Returns -1 on failure, 0 on success,
    with destination string null-terminated and s2 pointing to the
    terminating null, so that subsequent characters can be added.
    Failure reasons include destination buffer is filled up.
*/

#define XXDEPLIM 100                    /* Recursion depth limit */
/*
  In Windows the stack is limited to 256K so big character arrays like
  vnambuf can't be on the stack in recursive functions like zzstring().
  But that's no reason use malloc() in Unix or VMS, which don't have
  this kind of restriction.
*/
#ifdef DVNAMBUF				/* Dynamic vnambuf[] */
#undef DVNAMBUF				/* Clean slate */
#endif /* DVNAMBUF */

#ifndef NOSPL				/* Only if SPL included */
#ifdef OS2				/* Only for K95 */
#define DVNAMBUF
#endif /* OS2 */
#endif /* NOSPL */

int
zzstring(s,s2,n) char *s; char **s2; int *n; {
    int x,                              /* Current character */
        y,                              /* Worker */
        pp,                             /* Paren level */
        kp,                             /* Brace level */
        argn,                           /* Function argument counter */
        n2,                             /* Local copy of n */
        d,                              /* Array dimension */
        vbi,                            /* Variable id (integer form) */
        argl,                           /* String argument length */
        nx,                             /* Save original length */
	quoting = 0;			/* 299 */

    char vb,                            /* Variable id (char form) */
        *vp,                            /* Pointer to variable definition */
        *new,                           /* Local pointer to target string */
#ifdef COMMENT
        *old,                           /* Save original target pointer */
#endif /* COMMENT */
        *p,                             /* Worker */
        *q,                             /* Worker */
        *s3;                            /* Worker */
    int  x3;                            /* Worker */
    char *r  = (char *)0;               /* For holding function args */
    char *r2 = (char *)0;
    char *r3p;

#ifndef NOSPL
#ifdef DVNAMBUF
    char * vnambuf = NULL;              /* Buffer for variable/function name */
#else /* DVNAMBUF */
    char vnambuf[VNAML];                /* Buffer for variable/function name */
#endif /* DVNAMBUF */
    char *argp[FNARGS];                 /* Pointers to function args */
#endif /* NOSPL */

    static int depth = 0;               /* Call depth, avoid overflow */

    n2 = *n;                            /* Make local copies of args */
    nx = n2;

#ifdef COMMENT
    /* This is always 32K in BIGBUFOK builds */
    if (depth == 0)
      debug(F101,"zzstring top-level n","",n2);
#endif	/* COMMENT */

    new = *s2;                          /* for one less level of indirection */
#ifdef COMMENT
    old = new;
#endif /* COMMENT */

#ifndef NOSPL
    itsapattern = 0;			/* For \fpattern() */
    isjoin = 0;				/* For \fjoin() */
#endif /* NOSPL */
    depth++;                            /* Sink to a new depth */
    if (depth > XXDEPLIM) {             /* Too deep? */
        printf("?definition is circular or too deep\n");
        debug(F101,"zzstring fail","",depth);
        depth = 0;
        *new = NUL;
        return(-1);
    }
    if (!s || !new) {                   /* Watch out for null pointers */
        debug(F101,"zzstring fail 2","",depth);
        if (new)
          *new = NUL;
        depth = 0;
        return(-1);
    }
    s3 = s;
    argl = 0;
    while (*s3++) argl++;              /* Get length of source string */
    debug(F010,"zzstring entry",s,0);
    if (argl == 0) {                    /* Empty string */
        debug(F111,"zzstring empty arg",s,argl);
        depth = 0;
        *new = NUL;
        return(0);
    }
    if (argl < 0) {                     /* Watch out for garbage */
        debug(F101,"zzstring fail 3","",depth);
        *new = NUL;
        depth = 0;
        return(-1);
    }
#ifdef DVNAMBUF
    debug(F100,"vnambuf malloc...","",0);
    vnambuf = malloc(VNAML);
    if (vnambuf == NULL) {
        printf("?Out of memory");
        return(-1);
    }
    debug(F100,"vnambuf malloc ok","",0);
#endif /* DVNAMBUF */

    while ((x = *s)) {                  /* Loop for all characters */
        if (x != CMDQ) {                /* Is it the command-quote char? */
            *new++ = *s++;              /* No, normal char, just copy */
            if (--n2 < 0) {             /* and count it, careful of overflow */
                debug(F101,"zzstring overflow 1","",depth);
                depth = 0;
#ifdef DVNAMBUF
                if (vnambuf) free(vnambuf);
#endif /* DVNAMBUF */
                return(-1);
            }
            continue;
        }

/* We have the command-quote character. */

        x = *(s+1);                     /* Get the following character. */
        if (isupper(x)) x = tolower(x);
        switch (x) {                    /* Act according to variable type */
#ifndef NOSPL
          case 0:                       /* It's a lone backslash */
            *new++ = *s++;
            if (--n2 < 0) {
                debug(F101,"zzstring overflow 2","",0);
#ifdef DVNAMBUF
                if (vnambuf) free(vnambuf);
#endif /* DVNAMBUF */
                return(-1);
            }
            break;
          case '%':                     /* Variable */
            s += 2;                     /* Get the letter or digit */
            vb = *s++;                  /* and move source pointer past it */
            vp = NULL;                  /* Assume definition is empty */
            if (vb >= '0' && vb <= '9') { /* Digit for macro arg */
                if (maclvl < 0)         /* Digit variables are global */
                  vp = g_var[vb];       /* if no macro is active */
                else                    /* otherwise */
                  vp = m_arg[maclvl][vb - '0']; /* they're on the stack */
            } else if (vb == '*') {     /* Macro args string */
#ifdef COMMENT
                /* This doesn't take changes into account */
                vp = (maclvl >= 0) ? m_line[maclvl] : topline;
                if (!vp) vp = "";
#else
		char * ss = new;
                if (zzstring("\\fjoin(&_[],,1)",&new,&n2) < 0) {
#ifdef DVNAMBUF
		    if (vnambuf) free(vnambuf);
#endif /* DVNAMBUF */
		    return(-1);
		}
		debug(F110,"zzstring \\%*",ss,0);
                break;
#endif /* COMMENT */
            } else {
                if (isupper(vb)) vb += ('a'-'A');
                vp = g_var[vb];         /* Letter for global variable */
            }
            if (!vp) vp = "";
#ifdef COMMENT
            if (vp) {                   /* If definition not empty */
#endif /* COMMENT */
		if (vareval) {
		    debug(F010,"zzstring %n vp",vp,0);
		    /* call self to evaluate it */
		    if (zzstring(vp,&new,&n2) < 0) {
			debug(F101,"zzstring fail 6","",depth);
#ifdef DVNAMBUF
			if (vnambuf) free(vnambuf);
#endif /* DVNAMBUF */
			return(-1);	/* Pass along failure */
		    }
		} else {
		    while ((*new++ = *vp++)) /* copy it to output string. */
		      if (--n2 < 0) {
			  if (q) free(q);
			  debug(F101,"zzstring overflow 4.5","",depth);
#ifdef DVNAMBUF
			  if (vnambuf) free(vnambuf);
#endif /* DVNAMBUF */
			  return(-1);
		      }
		    new--;		/* Back up over terminating null */
		    n2++;		/* to allow for further deposits. */
		}
#ifdef COMMENT
            } else {
                debug(F110,"zzstring %n vp","(NULL)",0);
                n2 = nx;
                new = old;
                *new = NUL;
            }
#endif /* COMMENT */
            break;
          case '&':                     /* An array reference */
            x = arraynam(s,&vbi,&d);    /* Get name and subscript */
            debug(F111,"zzstring arraynam",s,x);
            if (x < 0) {
                debug(F101,"zzstring fail 7","",depth);
#ifdef DVNAMBUF
                if (vnambuf) free(vnambuf);
#endif /* DVNAMBUF */
                return(-1);
            }
            pp = 0;                     /* Bracket counter */
            while (*s) {                /* Advance source pointer... */
                if (*s == '[') pp++;
                if (*s == ']' && --pp == 0) break;
                s++;
            }
            if (*s == ']') s++;         /* ...past the closing bracket. */

            x = chkarray(vbi,d);        /* Array is declared? */
            debug(F101,"zzstring chkarray","",x);
            if (x > -1) {
#ifdef COMMENT
                char * s1 = NULL;
#endif /* COMMENT */
                vbi -= ARRAYBASE;       /* Convert name to index */

                  if (a_dim[vbi] >= d) { /* If subscript in range */
                    char **ap;
                    ap = a_ptr[vbi];    /* get data pointer */
                    if (ap) {           /* and if there is one */
                        if (ap[d]) {    /* If definition not empty */
                            debug(F111,"zzstring ap[d]",ap[d],d);
			    if (vareval) {
				if (zzstring(ap[d],&new,&n2) < 0) {
				    debug(F101,"zzstring fail 8","",depth);
#ifdef DVNAMBUF
				    if (vnambuf) free(vnambuf);
#endif /* DVNAMBUF */
				    return(-1); /* Pass along failure */
				}
			    } else {
				vp = ap[d];
				while ((*new++ = *vp++)) /* copy to result */
				  if (--n2 < 0) {
				      if (q) free(q);
				      debug(F101,
					    "zzstring overflow 8.5","",depth);
#ifdef DVNAMBUF
				      if (vnambuf) free(vnambuf);
#endif /* DVNAMBUF */
				      return(-1);
				  }
				new--;	/* Back up over terminating null */
				n2++;	/* to allow for further deposits. */
			    }
			}

                    } else {
                        /* old = new; */
                        n2 = nx;
                    }
                }
	    }
            break;

          case 'f':                     /* A builtin function */
            q = vnambuf;                /* Copy the name */
            y = 0;                      /* into a separate buffer */
            s += 2;                     /* point past 'F' */
            while (y++ < VNAML) {
                if (*s == '(') { s++; break; } /* Look for open paren */
                if ((*q = *s) == NUL) break;   /* or end of string */
                s++; q++;
            }
            *q = NUL;                   /* Terminate function name */
            if (y >= VNAML) {           /* Handle pathological case */
                while (*s && (*s != '(')) /* of very long string entered */
                  s++;                    /* as function name. */
                if (*s == ')') s++;       /* Skip past it. */
            }
            r = r2 = malloc(argl+2);    /* And make a place to copy args */
            /* debug(F101,"zzstring r2","",r2); */
            if (!r2) {                  /* Watch out for malloc failure */
                debug(F101,"zzstring fail 9","",depth);
                *new = NUL;
                depth = 0;
#ifdef DVNAMBUF
                if (vnambuf) free(vnambuf);
#endif /* DVNAMBUF */
                return(-1);
            }
            if (r3) free(r3); /* And another to copy literal arg string */
            r3 = malloc(argl+2);
            /* debug(F101,"zzstring r3","",r3); */
            if (!r3) {
                debug(F101,"zzstring fail 10","",depth);
                depth = 0;
                *new = NUL;
                if (r2) free(r2);
#ifdef DVNAMBUF
                if (vnambuf) free(vnambuf);
#endif /* DVNAMBUF */
                return(-1);
            } else
              r3p = r3;
            argn = 0;                   /* Argument counter */
            argp[argn++] = r;           /* Point to first argument */
            y = 0;                      /* Completion flag */
            pp = 1;                     /* Paren level (already have one). */
            kp = 0;
            while (1) {                 /* Copy each argument, char by char. */
                *r3p++ = *s;            /* This is a literal copy for \flit */
                if (!*s) break;

                if (*s == '{') {        /* Left brace */
                    kp++;
                }
                if (*s == '}') {        /* Right brace */
                    kp--;
                }
                if (*s == '(' && kp <= 0) { /* Open paren not in brace */
                    pp++;               /* Count it */
                }
                *r = *s;                /* Now copy resulting byte */
                if (!*r)                /* If NUL, done. */
                  break;
                if (*r == ')' && kp <= 0) { /* Closing paren, count it. */
                    if (--pp == 0) {    /* Final one? */
                        *r = NUL;       /* Make it a terminating null */
                        *(r3p - 1) = NUL;
                        s++;            /* Point past it in source string */
                        y = 1;          /* Flag we've got all the args */
                        break;          /* Done with while loop */
                    }
                }
                if (*r == ',' && kp <= 0) { /* Comma */
                    if (pp == 1) {          /* If not within ()'s, */
                        if (argn >= FNARGS) { /* Too many args */
                            s++; r++;   /* Keep collecting flit() string */
                            continue;
                        }
                        *r = NUL;           /* New arg, skip past comma */
                        argp[argn++] = r+1; /* In range, point to new arg */
                    }                   /* Otherwise just skip past  */
                }
                s++; r++;               /* Advance pointers */
            }
            if (!y)                     /* If we didn't find closing paren */
              argn = -1;
#ifdef DEBUG
            if (deblog) {
                char buf[24];
                debug(F111,"zzstring function name",vnambuf,y);
                debug(F010,"zzstring function r3",r3,0);
                for (y = 0; y < argn; y++) {
                    sprintf(buf,"arg %2d ",y);
                    debug(F010,buf,argp[y],0);
                }
            }
#endif /* DEBUG */
	    {
	     /* In case the function name itself is constructed */
		char buf[64]; char * p = buf; int n = 64; 
		if (zzstring(vnambuf,&p,&n) > -1)
		  ckstrncpy(vnambuf,buf,64);
	    }
            vp = fneval(vnambuf,argp,argn,r3); /* Evaluate the function. */
            if (vp) {                      /* If definition not empty */
                while ((*new++ = *vp++)) { /* copy it to output string */
                    if (--n2 < 0) {        /* watch out for overflow */
                        debug(F101,"zzstring fail 12","",depth);
                        if (r2) { free(r2); r2 = NULL; }
                        if (r3) { free(r3); r3 = NULL; }
#ifdef DVNAMBUF
                        if (vnambuf) free(vnambuf);
#endif /* DVNAMBUF */
                        return(-1);
                    }
                }
                new--;                  /* Back up over terminating null */
                n2++;                   /* to allow for further deposits. */
            }
            if (r2) { free(r2); r2 = NULL; }
            if (r3) { free(r3); r3 = NULL; }
            break;
	  case 'q':			/* 299 String to be take literally */
	    quoting = 1;		/* 299 */
          case '$':                     /* An environment variable */
          case 'v':                     /* Or a named builtin variable. */
          case 'm':                     /* Or a macro /long variable */
          case 's':                     /* 196 Macro substring */
          case ':':                     /* 196 \-variable substring */
            pp = 0;
            p = s+2;                    /* $/V/M must be followed by (name) */
            if (*p != '(') {            /* as in \$(HOME) or \V(count) */
                *new++ = *s++;          /* If not, just copy it */
                if (--n2 < 0) {
                    debug(F101,"zzstring overflow 3","",depth);
#ifdef DVNAMBUF
                    if (vnambuf) free(vnambuf);
#endif /* DVNAMBUF */
                    return(-1);
                }
                break;
            }
            pp++;
            p++;                        /* Point to 1st char of name */
            q = vnambuf;                /* Copy the name */
            y = 0;                      /* into a separate buffer */
	    debug(F110,">>>> \\q(ARG)",p,0);
            while (y++ < VNAML) {       /* Watch out for name too long */
                if (*p == '(') {        /* Parens can be nested... */
		    if (*(p-1) != CMDQ)	/* 299 */
		      pp++;
                } else if (*p == ')') { /* Name properly terminated with ')' */
		    if (*(p-1) != CMDQ)	/* 299 */
		      pp--;
                    if (pp == 0) {
                        p++;            /* Move source pointer past ')' */
                        break;
                    }
                }
                if ((*q = *p) == NUL)   /* String ends before ')' */
                  break;
                p++; q++;               /* Advance pointers */
            }
            *q = NUL;                   /* Terminate the variable name */
            if (y >= VNAML) {           /* Handle pathological case */
                while (*p && (*p != ')')) /* of very long string entered */
                  p++;                    /* as variable name. */
                if (*p == ')') p++;       /* Skip ahead to the end of it. */
            }
            s = p;                      /* Adjust global source pointer */
            s3 = vnambuf;
            x3 = 0;
            while (*s3++) x3++;
            p = malloc(x3 + 1);         /* Make temporary space */
            if (p && !quoting) {	/* If we got the space */
                vp = vnambuf;           /* Point to original */
                strcpy(p,vp);           /* (safe) Make a copy of it */
                y = VNAML;              /* Length of name buffer */
                zzstring(p,&vp,&y);     /* Evaluate the copy */
                free(p);                /* Free the temporary space */
                p = NULL;
            }
            debug(F110,"zzstring vname",vnambuf,0);
            q = NULL;
	    if (x == 'q') {		/* 299 Quoting this string */
		vp = vnambuf;		/* 299 */
		debug(F110,">>> VP",vp,0);
	    } else if (x == '$') {	/* Look up its value */
                vp = getenv(vnambuf);   /* This way for environment variable */
            } else if (x == 'm' || x == 's' || x == ':') { /* Macro / substr */
                int k, x1 = -1, x2 = -1;
		char c = NUL; 
                k = strlen(vnambuf);
                /* \s(name[n:m]) -- Compact substring notation */
                if ((x == 's' || x == ':') && (k > 1)) { /* Substring wanted */
		    int bprc;
                    if (vnambuf[k-1] == ']') {
                        int i;
                        for (i = 0; i < k-1; i++) {
                            if (vnambuf[i] == '[') {
				bprc = boundspair(vnambuf,":_.",&x1,&x2,&c);
				debug(F111,"zzstring boundspair",vnambuf,bprc);
				debug(F000,"zzstring boundspair c","",c);
				if (bprc > -1) {
				    vnambuf[i] = NUL;
				    if (x1 < 1)
				      x1 = 1;
				    x1--;	/* Adjust to 0-base */
				}
				break;
                            }
                        }
		    }
                }
                if (x == ':') {		/* Variable type (s or :) */
                    vp = vnambuf;
                } else {
		    y = isaarray(vnambuf) ?
			mxxlook(mactab,vnambuf,nmac) :
			mxlook(mactab,vnambuf,nmac);
                    if (y > -1) {	/* Got definition */
                        vp = mactab[y].mval;
                    } else {
                        vp = NULL;
                    }
                }
		debug(F111,"zzstring vp",vp,(vp==NULL)?0:strlen(vp));

                if (vp) {
                    if ((x == 's' || x == ':') && (k > 1)) {
                        /* Compact substring notation */
                        if (x2 == 0) {  /* Length */
                            vp = NULL;
                        } else if (x1 > -1) { /* Start */
                            k = strlen(vp);
			    debug(F101,">>> k","",k);
			    /* If it's off the end, result is empty */
                            if (x1 > k) {
                                vp = NULL;
                            } else if (k > 0) {
				/* Stay in bounds */
				if (c == '_' && x2 > k)	/* startpos_endpos */
				  x2 = k;
				if (c == ':' && x1 + x2 > k) /* start:length */
				  x2 = -1;
				debug(F101,">>> x2","",x2);
				debug(F000,">>> c","",c);
                                if ((q = malloc(k+1))) {
                                    strcpy(q,vp); /* safe */
				    if (c == '.') {
					q[x1+1] = NUL;
					debug(F000,"XXX. q",q,c);
				    }
				    if (c == ':') { /* start:length */
					if ((x2 > -1) && ((x1 + x2) <= k)) {
					    q[x1+x2] = NUL;
					}
					debug(F000,"XXX: q",q,c);
				    } else if (c == '_') { /* start_endpos */
					if (x1 >= x2) {
					    q[x1 = 0] = NUL;
					} else if (x2 < k && x2 > -1) {
					    q[x2] = NUL;
					}
					debug(F000,"XXX_ q",q,c);
				    }
				    vp = q+x1;
                                } else vp = NULL;
                            } else vp = NULL;
                        }

			debug(F110,"XXX vnambuf",vnambuf,0);
			debug(F000,"XXX c","",c);
			debug(F101,"XXX x1","",x1);
			debug(F101,"XXX x2","",x2);
			debug(F110,"XXX result",vp,0);
#ifdef DEBUG
                        if (deblog) {
                            if (!vp) {
                            } else {
                                k = strlen(vp);
                            }
                        }
#endif /* DEBUG */
                    }
                }
            } else {                    /* or */
                vp = nvlook(vnambuf);   /* this way for builtin variable */
            }
            if (vp) {                   /* If definition not empty */
                while ((*new++ = *vp++)) /* copy it to output string. */
                  if (--n2 < 0) {
                      if (q) free(q);
                      debug(F101,"zzstring overflow 4","",depth);
#ifdef DVNAMBUF
                      if (vnambuf) free(vnambuf);
#endif /* DVNAMBUF */
                      return(-1);
                  }
                new--;                  /* Back up over terminating null */
                n2++;                   /* to allow for further deposits. */
            }
            if (q) {
                free(q);
                q = NULL;
            }
            break;
#endif /* NOSPL */                      /* Handle \nnn even if NOSPL. */

#ifndef NOKVERBS
        case 'K':
        case 'k': {
            extern struct keytab kverbs[];
            extern int nkverbs;
#define K_BUFLEN 30
            char kbuf[K_BUFLEN + 1];    /* Key verb name buffer */
            int x, y, z, brace = 0;
            s += 2;
/*
  We assume that the verb name is {braced}, or it extends to the end of the
  string, s, or it ends with a space, control character, or backslash.
*/
            p = kbuf;                   /* Copy verb name into local buffer */
            x = 0;
            if (*s == '{')  {
                s++;
                brace++;
            }
            while ((x++ < K_BUFLEN) && (*s > SP) && (*s != CMDQ)) {
                if (brace && *s == '}') {
                    s++;
                    break;
                }
                *p++ = *s++;
            }
            brace = 0;
            *p = NUL;                   /* Terminate. */
            p = kbuf;                   /* Point back to beginning */
            debug(F110,"zzstring kverb",p,0);
            y = xlookup(kverbs,p,nkverbs,&x); /* Look it up */
            debug(F101,"zzstring lookup",0,y);
            if (y > -1) {
                dokverb(VCMD,y);
#ifndef NOSPL
            } else {                    /* Is it a macro? */
                y = mxlook(mactab,p,nmac);
                if (y > -1) {
                    debug(F111,"zzstring mxlook",p,y);
                    if ((z = dodo(y,NULL,cmdstk[cmdlvl].ccflgs)) > 0) {
                        if (cmpush() > -1) {  /* Push command parser state */
                            extern int ifc;
                            int ifcsav = ifc; /* Push IF condition on stack */
                            y = parser(1);    /* New parser to execute macro */
                            cmpop();          /* Pop command parser */
                            ifc = ifcsav;     /* Restore IF condition */
                            if (y == 0) {     /* No errors, ignore actions */
                                p = mrval[maclvl+1]; /* If OK set return val */
                                if (p == NULL) p = "";
                            }
                        } else {                /* Can't push any more */
                            debug(F101,"zzstring pushed too deep","",depth);
                            printf(
                               "\n?Internal error: zzstring stack overflow\n"
                                   );
                            while (cmpop() > -1);
                            p = "";
                        }
                    }
                }
#endif /* NOSPL */
            }
            break;
        }
#endif /* NOKVERBS */

        default:                        /* Maybe it's a backslash code */
          y = xxesc(&s);                /* Go interpret it */
          if (y < 0) {                  /* Upon failure */
              *new++ = (char) x;        /* Just quote the next character */
              s += 2;                   /* Move past the pair */
              n2 -= 2;
              if (n2 < 0) {
                  debug(F101,"zzstring overflow 5","",depth);
#ifdef DVNAMBUF
                  if (vnambuf) free(vnambuf);
#endif /* DVNAMBUF */
                  return(-1);
              }
              continue;                 /* and go back for more */
          } else {
              *new++ = (char) y;        /* else deposit interpreted value */
              if (--n2 < 0) {
                  debug(F101,"zzstring overflow 6","",depth);
#ifdef DVNAMBUF
                  if (vnambuf) free(vnambuf);
#endif /* DVNAMBUF */
                  return(-1);
              }
          }
        }
    }
    *new = NUL;                         /* Terminate the new string */
    debug(F010,"zzstring while exit",*s2,0);

    depth--;                            /* Adjust stack depth gauge */
    *s2 = new;                          /* Copy results back into */
    *n = n2;                            /* the argument addresses */
    debug(F101,"zzstring ok","",depth);
#ifdef DVNAMBUF
    if (vnambuf) free(vnambuf);
#endif /* DVNAMBUF */
    return(0);                          /* and return. */
}
#endif /* NOICP */

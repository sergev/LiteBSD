#include "ckcsym.h"

/*  C K U U S 7 --  "User Interface" for C-Kermit, part 7  */

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
  This file created from parts of ckuus3.c, which became too big for
  Mark Williams Coherent compiler to handle.
*/

/*
  Definitions here supersede those from system include files.
*/
#include "ckcdeb.h"                     /* Debugging & compiler things */
#include "ckcasc.h"                     /* ASCII character symbols */
#include "ckcker.h"                     /* Kermit application definitions */
#include "ckcxla.h"                     /* Character set translation */
#include "ckcnet.h"                     /* Network symbols */
#include "ckuusr.h"                     /* User interface symbols */
#include "ckucmd.h"
#include "ckclib.h"

#ifdef VMS
#ifndef TCPSOCKET
#include <errno.h>
#endif /* TCPSOCKET */
#endif /* VMS */

#ifdef OS2
#ifndef NT
#define INCL_NOPM
#define INCL_VIO                        /* Needed for ckocon.h */
#define INCL_DOSMODULEMGR
#include <os2.h>
#undef COMMENT
#else /* NT */
#define APIRET ULONG
#include <windows.h>
#include <tapi.h>
#include "cknwin.h"
#include "ckntap.h"
#endif /* NT */
#include "ckowin.h"
#include "ckocon.h"
#include "ckodir.h"
#ifdef OS2MOUSE
#include "ckokey.h"
#endif /* OS2MOUSE */
#ifdef KUI
#include "ikui.h"
#endif /* KUI */
#ifdef putchar
#undef putchar
#endif /* putchar */
#define putchar(x) conoc(x)
extern int mskkeys;
extern int mskrename;
#endif /* OS2 */

#ifdef CK_AUTHENTICATION
#include "ckuath.h"
#endif /* CK_AUTHENTICATION */
#ifdef CK_SSL
#include "ck_ssl.h"
#endif /* CK_SSL */
#ifdef SSHBUILTIN
#include "ckossh.h"
#endif /* SSHBUILTIN */
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

char * slmsg = NULL;

static int x, y = 0, z;
static char *s;

extern CHAR feol;
extern int g_matchdot, hints, xcmdsrc, rcdactive;

extern char * k_info_dir;

#ifdef CK_LOGIN
#ifdef CK_PAM
int gotemptypasswd = 0;   /* distinguish empty passwd from none given */
#endif /* CK_PAM */
#endif /* CK_LOGIN */

#ifndef NOSPL
extern int nmac;
extern struct mtab *mactab;
#endif /* NOSPL */

#ifndef NOXFER
#ifdef CK_SPEED
extern short ctlp[];                    /* Control-char prefixing table */
#endif /* CK_SPEED */

#ifdef PIPESEND
extern char * sndfilter, * g_sfilter;
extern char * rcvfilter, * g_rfilter;
#endif /* PIPESEND */

extern char * snd_move;
extern char * snd_rename;
extern char * g_snd_move;
extern char * g_snd_rename;
extern char * rcv_move;
extern char * rcv_rename;
extern char * g_rcv_move;
extern char * g_rcv_rename;

#ifdef PATTERNS
extern char *binpatterns[], *txtpatterns[];
extern int patterns;
#endif /* PATTERNS */

extern char * remdest;
#ifdef CK_TMPDIR
char * dldir = NULL;
#endif /* CK_TMPDIR */

extern struct ck_p ptab[];

extern int protocol, remfile, rempipe, remappd, reliable, xreliable, fmask,
  fncnv, frecl, maxrps, wslotr, bigsbsiz, bigrbsiz, urpsiz, rpsiz, spsiz,
  bctr, npad, timef, timint, spsizr, spsizf, maxsps, spmax, nfils, displa,
  atcapr, pkttim, rtimo, fncact, mypadn, fdispla, f_save, pktpaus, setreliable,
  fnrpath, fnspath, atenci, atenco, atdati, atdato, atleni, atleno, atblki,
  atblko, attypi, attypo, atsidi, atsido, atsysi, atsyso, atdisi, atdiso;

extern int stathack;

extern int atfrmi, atfrmo;
#ifdef STRATUS
extern int atcrei, atcreo, atacti, atacto;
#endif /* STRATUS */
#ifdef CK_PERMS
extern int atlpri, atlpro, atgpri, atgpro;
#endif /* CK_PERMS */

extern CHAR
  sstate, eol, seol, stchr, mystch, mypadc, padch, ctlq, myctlq;

#ifdef IKSD
extern int inserver;
#ifdef IKSDCONF
extern int iksdcf;
#endif /* IKSDCONF */
#endif /* IKSD */

extern char *cmarg, *cmarg2;

#ifndef NOFRILLS
extern char optbuf[];                   /* Buffer for MAIL or PRINT options */
extern int rprintf;                     /* REMOTE PRINT flag */
#endif /* NOFRILLS */
#endif /* NOXFER */

#ifdef CK_TRIGGER
extern char * tt_trigger[];
#endif /* CK_TRIGGER */

extern int tcs_transp;
#ifdef PCTERM
extern int tt_pcterm;
#endif /* PCTERM */
#ifdef NT
extern int tt_vtnt;
#endif /* NT */

#ifdef SSHBUILTIN
int sl_ssh_xfw = 0;
int sl_ssh_xfw_saved = 0;
int sl_ssh_ver = 0;
int sl_ssh_ver_saved = 0;
#endif /* SSHBUILTIN */

#ifdef CK_AUTHENTICATION
extern int auth_type_user[];
int sl_auth_type_user[AUTHTYPLSTSZ] = {AUTHTYPE_NULL, AUTHTYPE_NULL};
int sl_auth_saved = 0;
int sl_topt_a_su = 0;
int sl_topt_a_s_saved = 0;
int sl_topt_a_cm = 0;
int sl_topt_a_c_saved = 0;
#endif /* CK_AUTHENTICATION */
#ifdef CK_ENCRYPTION
extern int cx_type;
int sl_cx_type = 0;
int sl_cx_saved = 0;
int sl_topt_e_su = 0;
int sl_topt_e_sm = 0;
int sl_topt_e_s_saved = 0;
int sl_topt_e_cu = 0;
int sl_topt_e_cm = 0;
int sl_topt_e_c_saved = 0;
#endif /* CK_ENCRYPTION */
extern char uidbuf[];
static int uidflag = 0;
char sl_uidbuf[UIDBUFLEN] = { NUL, NUL };
int  sl_uid_saved = 0;
#ifdef TNCODE
int  sl_tn_wait = 0;
int  sl_tn_saved = 0;
#endif /* TNCODE */

#ifdef TNCODE
extern int tn_wait_flg;
#endif /* TNCODE */

VOID
slrestor() {
#ifdef CK_AUTHENTICATION
    int x;
    if (sl_auth_saved) {
        for (x = 0; x < AUTHTYPLSTSZ; x++)
          auth_type_user[x] = sl_auth_type_user[x];
        sl_auth_saved = 0;
    }
    if (sl_topt_a_s_saved) {
        TELOPT_DEF_S_U_MODE(TELOPT_AUTHENTICATION) = sl_topt_a_su;
        sl_topt_a_s_saved = 0;
    }
    if (sl_topt_a_c_saved) {
        TELOPT_DEF_C_ME_MODE(TELOPT_AUTHENTICATION) = sl_topt_a_cm;
        sl_topt_a_c_saved = 0;
    }
#endif /* CK_AUTHENTICATION */
#ifdef CK_ENCRYPTION
    if (sl_cx_saved) {
        cx_type = sl_cx_type;
        sl_cx_saved = 0;
    }
    if (sl_topt_e_s_saved) {
        TELOPT_DEF_S_U_MODE(TELOPT_ENCRYPTION)  = sl_topt_e_su;
        TELOPT_DEF_S_ME_MODE(TELOPT_ENCRYPTION) = sl_topt_e_sm;
        sl_topt_e_s_saved = 0;
    }
    if (sl_topt_e_c_saved) {
        TELOPT_DEF_C_U_MODE(TELOPT_ENCRYPTION)  = sl_topt_e_cu;
        TELOPT_DEF_C_ME_MODE(TELOPT_ENCRYPTION) = sl_topt_e_cm;
        sl_topt_e_c_saved = 0;
    }
#endif /* CK_ENCRYPTION */
    if (sl_uid_saved) {
        ckstrncpy(uidbuf,sl_uidbuf,UIDBUFLEN);
        sl_uid_saved = 0;
    }
#ifdef TNCODE
    if (sl_tn_saved) {
        tn_wait_flg = sl_tn_wait;
        sl_tn_saved = 0;
    }
#endif /* TNCODE */
#ifdef SSHBUILTIN
    if (sl_ssh_xfw_saved) {
        ssh_xfw = sl_ssh_xfw;
        sl_ssh_xfw_saved = 0;
    }
    if (sl_ssh_ver_saved) {
        ssh_ver = sl_ssh_ver;
        sl_ssh_ver_saved = 0;
    }
#endif /* SSHBUILTIN */
}

int oldplex = -1;                       /* Duplex holder around network */

#ifndef NOICP
#ifdef LOCUS
extern int locus, autolocus;
#endif /* LOCUS */
#ifndef NODIAL
extern int dialsta;
#endif /* NODIAL */

/* Note: gcc -Wall wants braces around each keyword table entry. */

static struct keytab psltab[] = {       /* SET LINE/PORT command options */
    { "/connect", SL_CNX, 0 },
#ifdef OS2ORVMS
    { "/noshare", SL_NSH, 0 },
#endif /* OS2ORVMS */
    { "/server",  SL_SRV, 0 },
#ifdef OS2ORVMS
    { "/share",   SL_SHR, 0 },
#endif /* OS2ORVMS */
    { "", 0, 0 }
};
static int npsltab = sizeof(psltab)/sizeof(struct keytab) - 1;

#ifdef NETCONN
static struct keytab shtab[] = {        /* SET HOST command options */
#ifdef NETCMD
    /* (COMMAND is also a network type) */
    { "/command",      SL_CMD,    CM_INV },
#endif /* NETCMD */
    { "/connect",      SL_CNX,    0 },
    { "/network-type", SL_NET,    CM_ARG },
    { "/nowait",       SL_NOWAIT, 0 },
#ifndef NOSPL
#ifdef CK_AUTHENTICATION
    { "/password",     SL_PSW,    CM_ARG },
#endif /* CK_AUTHENTICATION */
#endif /* NOSPL */
#ifdef NETCMD
    { "/pipe",         SL_CMD,    0 },
#endif /* NETCMD */
#ifdef NETPTY
    { "/pty",          SL_PTY,    0 },
#endif /* NETPTY */
    { "/server",       SL_SRV,    0 },
    { "/timeout",      SL_TMO,    CM_ARG },
    { "/userid",       SL_UID,    CM_ARG },
    { "/wait",         SL_WAIT,   0 },
    { "", 0, 0 }
};
static int nshtab = sizeof(shtab)/sizeof(struct keytab) - 1;

static struct keytab shteltab[] = {     /* TELNET command options */
#ifdef CK_AUTHENTICATION
    { "/auth",         SL_AUTH,   CM_ARG },
#endif /* CK_AUTHENTICATION */
#ifdef CK_ENCRYPTION
    { "/encrypt",      SL_ENC,    CM_ARG },
#endif /* CK_ENCRYPTION */
    { "/nowait",       SL_NOWAIT, 0 },
#ifndef NOSPL
#ifdef CK_AUTHENTICATION
    { "/password",     SL_PSW,    CM_ARG },
#endif /* CK_AUTHENTICATION */
#endif /* NOSPL */
    { "/timeout",      SL_TMO,    CM_ARG },
    { "/userid",       SL_UID,    CM_ARG },
    { "/wait",         SL_WAIT,   0 },
    { "", 0 ,0 }
};
static int nshteltab = sizeof(shteltab)/sizeof(struct keytab) - 1;

#ifdef RLOGCODE
static struct keytab shrlgtab[] = {     /* SET HOST RLOGIN command options */
#ifdef CK_KERBEROS
#ifdef CK_ENCRYPTION
    { "/encrypt",      SL_ENC, 0 },
#endif /* CK_ENCRYPTION */
    { "/k4",           SL_KRB4, CM_INV },
    { "/k5",           SL_KRB5, CM_INV },
    { "/kerberos4",    SL_KRB4, 0 },
    { "/kerberos5",    SL_KRB5, 0 },
    { "/kerberos_iv",  SL_KRB4, CM_INV },
    { "/kerberos_v",   SL_KRB5, CM_INV },
    { "/krb4",         SL_KRB4, CM_INV },
    { "/krb5",         SL_KRB5, CM_INV },
#endif /* CK_KERBEROS */
    { "", 0 ,0 }
};
static int nshrlgtab = sizeof(shrlgtab)/sizeof(struct keytab)-1;
#endif /* RLOGCODE */

extern struct keytab netcmd[];
extern int nnets;
#ifndef NODIAL
extern int dirline;
extern int nnetdir;                     /* Network services directory */
extern char *netdir[];
_PROTOTYP( VOID ndreset, (void) );
char *nh_p[MAXDNUMS + 1];               /* Network directory entry pointers */
char *nh_p2[MAXDNUMS + 1];              /* Network directory entry nettype */
char *nh_px[4][MAXDNUMS + 1];           /* Network-specific stuff... */
#endif /* NODIAL */
int nhcount = 0;
int ndinited = 0;
char * n_name = NULL;                   /* Network name pointer */
#endif /* NETCONN */

_PROTOTYP(int remtxt, (char **) );
_PROTOTYP(VOID rmsg, (void) );
_PROTOTYP(static int remcfm, (void) );

extern int nopush;

int mdmsav = -1;                        /* Save modem type around network */
extern int isguest;                     /* Global flag for anonymous login */

extern xx_strp xxstring;

extern int success, binary, b_save, ckwarn, msgflg, quiet, cmask, pflag, local,
  nettype, escape, mdmtyp, duplex, dfloc, network, cdtimo, autoflow, tnlm,
  sosi, tlevel, lf_opts, backgrd, flow, debses, parity, ttnproto, ckxech,
  x_ifnum, cmflgs, haveline, cxtype, cxflow[], maclvl;

#ifdef DCMDBUF
extern struct cmdptr *cmdstk;           /* The command stack itself */
#else
extern struct cmdptr cmdstk[];          /* The command stack itself */
#endif /* DCMDBUF */
extern FILE * tfile[];
extern char * macp[];

extern char psave[];                    /* For saving & restoring prompt */
extern int sprmlen, rprmlen;

#ifdef OS2
static struct keytab strmkeytab[] = {
    { "clear",   0, 0 },
    { "default", 1, 0 }
};
static int nstrmkeytab = sizeof(strmkeytab)/sizeof(struct keytab);

static struct keytab strmswitab[] = {
    { "/literal", 0, 0 }
};
static int nstrmswitab = sizeof(strmswitab)/sizeof(struct keytab);

static struct keytab normrev[] = {
    { "dark-display", 0, 0 },
    { "light-display", 1, 0 },
    { "normal",   0, 0 },
    { "reverse",  1, 0 }
};

static struct keytab prnmtab[] = {
    { "auto", 1, 0 },
    { "copy", 2, 0 },
    { "off",  0, 0 },
    { "on",   1, CM_INV },              /* Compatibility with XPRINT version */
    { "user", 3, 0 },
    { "transparent", 3, CM_INV }        /* not really transparent */
};
static int nprnmtab = sizeof(prnmtab)/sizeof(struct keytab);

extern int tt_diff_upd;

#ifdef NT
#define stricmp _stricmp
extern int tt_attr_bug;
#endif /* NT */
extern int tt_rows[], tt_cols[];
extern int tt_cols_usr;
extern int tt_szchng[VNUM];
int tt_modechg = TVC_ENA;
extern int tt_url_hilite, tt_url_hilite_attr;
extern struct _vtG G[4];
extern int priority;
extern bool send_c1;
int send_c1_usr = FALSE;
extern int sgrcolors;
extern int marginbell, marginbellcol;
extern int autoscroll, wy_autopage;
extern int tt_sac;
extern int dec_nrc, dec_lang, dec_kbd;
#else /* OS2 */
extern int tt_rows, tt_cols;
#endif /*  OS2 */

extern int tt_escape;
extern long speed;

extern char *dftty;

extern char *tp, *lp;                   /* Temporary buffer & pointers */
extern char ttname[];

#ifdef CK_TAPI
int tttapi = 0;                         /* is Line TAPI? */
struct keytab * tapilinetab = NULL;
struct keytab * _tapilinetab = NULL;
int ntapiline = 0;
#endif /* CK_TAPI */

#ifdef NETCONN                          /* Network items */

#ifdef ANYX25
extern int revcall, closgr, cudata, nx25;
extern char udata[];
extern struct keytab x25tab[];
#ifndef IBMX25
extern int npadx3;
extern CHAR padparms[];
extern struct keytab padx3tab[];
#endif /* IBMX25 */
#endif /* ANYX25 */

#ifdef OS2
extern bool ttshare;
#ifndef NT
extern bool ttslip,ttppp;
#endif /* NT */
#endif /* OS2 */
#ifdef NPIPE
extern char pipename[];
#endif /* NPIPE */

#ifdef TCPSOCKET
static struct keytab tcprawtab[] = {    /* SET HOST options */
    { "/default",    NP_DEFAULT,    CM_INV },
#ifdef CK_AUTHENTICATION
#ifdef CK_KERBEROS
#ifdef RLOGCODE
    { "/ek4login",    NP_EK4LOGIN,    0 },
    { "/ek5login",    NP_EK5LOGIN,    0 },
    { "/k4login",     NP_K4LOGIN,     0 },
    { "/k5login",     NP_K5LOGIN,     0 },
#endif /* RLOGCODE */
#ifdef KRB5_U2U
    { "/k5user2user", NP_K5U2U,       0 },
#endif /* KRB5_U2U */
#endif /* CK_KERBEROS */
#endif /* CK_AUTHENTICATION */
    { "/no-telnet-init", NP_NONE,   0 },
    { "/none",       NP_NONE,   CM_INV },
    { "/raw-socket", NP_TCPRAW, 0 },
#ifdef RLOGCODE
    { "/rlogin",     NP_RLOGIN, 0 },
#endif /* RLOGCODE */
#ifdef CK_SSL
    { "/ssl",        NP_SSL,    0 },
    { "/ssl-raw",    NP_SSL_RAW, 0 },
    { "/ssl-telnet", NP_SSL_TELNET, 0 },
#endif /* CK_SSL */
    { "/telnet",     NP_TELNET, 0 },
#ifdef CK_SSL
    { "/tls",        NP_TLS,    0 },
    { "/tls-raw",    NP_TLS_RAW, 0 },
    { "/tls-telnet", NP_TLS_TELNET, 0 },
#endif /* CK_SSL */
    { "", 0, 0 }
};
static int ntcpraw = (sizeof(tcprawtab) / sizeof(struct keytab)) - 1;

#ifdef RLOGCODE
_PROTOTYP( int rlog_naws, (void) );
#endif /* RLOGCODE */
#endif /* TCPSOCKET */

#ifdef SUPERLAT
extern char slat_pwd[18];
#endif /* SUPERLAT */
#endif /* NETCONN */

#ifdef COMMENT
#ifndef NOSETKEY
extern KEY *keymap;
#ifndef OS2
#define mapkey(x) keymap[x]
#endif /* OS2 */
extern MACRO *macrotab;
#ifndef NOKVERBS
extern struct keytab kverbs[];
extern int nkverbs;
#endif /* NOKVERBS */
#endif /* NOSETKEY */
#else
#ifndef NOSETKEY
extern KEY *keymap;
extern MACRO *macrotab;
#ifndef NOKVERBS
extern struct keytab kverbs[];
extern int nkverbs;
#endif /* NOKVERBS */
#endif /* NOSETKEY */
#endif /* COMMENT */

#ifdef OS2                              /* AUTODOWNLOAD parameters */
extern int    adl_kmode, adl_zmode;     /* Match Packet to signal download */
extern char * adl_kstr;                 /* KERMIT Download String */
extern char * adl_zstr;                 /* ZMODEM Download String */
extern int adl_kc0, adl_zc0;            /* Process ADL C0s in emulation */
#endif /* OS2 */

/* Keyword tables ... */

extern struct keytab onoff[], rltab[];
extern int nrlt;

#ifndef NOCSETS
static struct keytab fdfltab[] = {
    { "7bit-character-set", 7, 0 },
    { "8bit-character-set", 8, 0 }
};
static int nfdflt = (sizeof(fdfltab) / sizeof(struct keytab));
#endif /* NOCSETS */

/* SET FILE parameters */

static struct keytab filtab[] = {
#ifndef NOXFER
#ifdef PATTERNS
    { "binary-patterns",   XYFIBP,  0 },
#endif /* PATTERNS */
    { "bytesize",         XYFILS,   0 },
#ifndef NOCSETS
    { "character-set",    XYFILC,   0 },
#endif /* NOCSETS */
    { "collision",        XYFILX,   0 },
    { "default",          XYF_DFLT, 0 },
    { "destination",      XYFILY,   0 },
    { "display",          XYFILD,   CM_INV },
#ifdef CK_TMPDIR
    { "download-directory", XYFILG, 0 },
#endif /* CK_TMPDIR */
#endif /* NOXFER */
    { "end-of-line",      XYFILA,   0 },
    { "eol",              XYFILA,   CM_INV },
#ifdef CK_CTRLZ
    { "eof",              XYFILV,   0 },
#endif /* CK_CTRLZ */
#ifndef NOXFER
    { "fastlookups",      9997,     CM_INV },
    { "incomplete",       XYFILI,   0 },
#ifndef datageneral
    { "inspection",       XYF_INSP, CM_INV },
#endif /* datageneral */
#ifdef CK_LABELED
    { "label",            XYFILL, 0 },
#endif /* CK_LABELED */

#ifdef UNIX
#ifdef DYNAMIC
    { "listsize",         XYF_LSIZ, 0 },
#endif /* DYNAMIC */
#endif /* UNIX */

    { "names",            XYFILN, 0 },
#ifdef UNIX
    { "output",           XYFILH, 0 },
#endif /* UNIX */
#ifdef PATTERNS
    { "patterns",         XYFIPA, 0 },
#endif /* PATTERNS */
#ifdef COMMENT /* Not implemented (but see CHMOD) */
    { "permissions",      XYF_PRM, CM_INV },
    { "protection",       XYF_PRM, 0 },
#endif /* COMMENt */
#ifdef VMS
    { "record-length",    XYFILR, 0 },
#endif /* VMS */
#ifndef datageneral
    { "scan",             XYF_INSP, 0 },
#endif /* datageneral */

#ifdef UNIX
#ifdef DYNAMIC
    { "stringspace",      XYF_SSPA, 0 },
#endif /* DYNAMIC */
#endif /* UNIX */

#ifdef PATTERNS
    { "t",                XYFILT, CM_INV|CM_ABR },
    { "text-patterns",    XYFITP, 0 },
#endif /* PATTERNS */
#endif /* NOXFER */
    { "type",             XYFILT, 0 },
#ifdef UNICODE
    { "ucs",              XYFILU, 0 },
#endif /* UNICODE */
#ifndef NOXFER
    { "warning",          XYFILW, CM_INV }
#endif /* NOXFER */
};
static int nfilp = (sizeof(filtab) / sizeof(struct keytab));

struct keytab pathtab[] = {
    { "absolute",  PATH_ABS,  0      },
    { "none",      PATH_OFF,  CM_INV },
    { "off",       PATH_OFF,  0      },
    { "on",        PATH_ABS,  CM_INV },
    { "relative",  PATH_REL,  0      }
};
int npathtab = (sizeof(pathtab) / sizeof(struct keytab));

struct keytab rpathtab[] = {
    { "absolute",  PATH_ABS,  0      },
    { "auto",      PATH_AUTO, 0      },
    { "none",      PATH_OFF,  CM_INV },
    { "off",       PATH_OFF,  0      },
    { "on",        PATH_ABS,  CM_INV },
    { "relative",  PATH_REL,  0      }
};
int nrpathtab = (sizeof(rpathtab) / sizeof(struct keytab));

#ifdef CK_CTRLZ
struct keytab eoftab[] = {              /* EOF detection method */
    { "ctrl-z",          1, 0      },
    { "length",          0, 0      },
    { "noctrl-z",        0, CM_INV }
};
#endif /* CK_CTRLZ */

struct keytab fttab[] = {               /* File types for SET FILE TYPE */
    { "ascii",     XYFT_T, CM_INV },
#ifdef VMS
    { "b",         XYFT_B, CM_INV|CM_ABR },
#endif /* VMS */
    { "binary",    XYFT_B, 0 },
#ifdef VMS
    { "block",     XYFT_I, CM_INV },
    { "image",     XYFT_I, 0 },
#endif /* VMS */
#ifdef CK_LABELED
    { "labeled",   XYFT_L, 0 },
#endif /* CK_LABELED */
#ifdef MAC
    { "macbinary", XYFT_M, 0 },
#endif /* MAC */
    { "text",      XYFT_T, 0 }
};
int nfttyp = (sizeof(fttab) / sizeof(struct keytab));

static struct keytab rfttab[] = {       /* File types for REMOTE SET FILE */
    { "ascii",     XYFT_T, CM_INV },
    { "binary",    XYFT_B, 0 },
#ifdef VMS
    { "labeled",   XYFT_L, 0 },
#else
#ifdef OS2
    { "labeled",   XYFT_L, 0 },
#endif /* OS2 */
#endif /* VMS */
    { "text",      XYFT_T, 0 }
};
static int nrfttyp = (sizeof(rfttab) / sizeof(struct keytab));

#ifdef OS2ORUNIX
#define ZOF_BLK  0
#define ZOF_NBLK 1
#define ZOF_BUF  2
#define ZOF_NBUF 3
static struct keytab zoftab[] = {
    { "blocking",    ZOF_BLK,  0 },
    { "buffered",    ZOF_BUF,  0 },
    { "nonblocking", ZOF_NBLK, 0 },
    { "unbuffered",  ZOF_NBUF, 0 }
};
static int nzoftab = (sizeof(zoftab) / sizeof(struct keytab));
#endif /* OS2ORUNIX */

extern int query;                       /* Global flag for QUERY active */

#ifndef NOSPL
#ifndef NOXFER
static struct keytab vartyp[] = {       /* Variable types for REMOTE QUERY */
    { "global",   (int) 'G', CM_INV },
    { "kermit",   (int) 'K', 0 },
    { "system",   (int) 'S', 0 },
    { "user",     (int) 'G', 0 }
};
static int nvartyp = (sizeof(vartyp) / sizeof(struct keytab));
#endif /* NOXFER */
#endif /* NOSPL */

#ifdef CK_TIMERS
static struct keytab timotab[] = {      /* Timer types */
    { "dynamic", 1, 0 },
    { "fixed",   0, 0 }
};
#endif /* CK_TIMERS */

#ifdef DCMDBUF
extern char *atxbuf, *atmbuf;           /* Atom buffer */
extern char *cmdbuf;                    /* Command buffer */
extern char *line, *tmpbuf;             /* Character buffers for anything */
extern int *intime;                     /* INPUT TIMEOUT */

#else  /* Not DCMDBUF ... */

extern char atxbuf[], atmbuf[];         /* Atom buffer */
extern char cmdbuf[];                   /* Command buffer */
extern char line[], tmpbuf[];           /* Character buffer for anything */
extern int intime[];

#endif /* DCMDBUF */

#ifndef NOCSETS
extern struct keytab fcstab[];          /* For SET FILE CHARACTER-SET */
extern struct csinfo fcsinfo[];         /* File character set info. */
extern struct keytab ttcstab[];
extern int nfilc, fcharset, tcharset, ntermc, tcsr, tcsl, dcset7, dcset8;
#ifdef CKOUNI
extern int tt_utf8;
#endif /* CKOUNI */
#ifdef OS2
_PROTOTYP( int os2setcp, (int) );
_PROTOTYP( int os2getcp, (void) );
_PROTOTYP( void os2debugoff, (void) );
#endif /* OS2 */
#endif /* NOCSETS */

extern int cmdlvl;                      /* Overall command level */

#ifndef NOSPL
#ifdef DCMDBUF
extern int *inpcas;                     /* INPUT CASE setting on cmd stack */
#else
extern int inpcas[];
#endif /* DCMDBUF */
#endif /* NOSPL */

#ifdef CK_CURSES
#ifndef VMS
_PROTOTYP(int tgetent,(char *, char *));
#else
#ifdef __DECC
_PROTOTYP(int tgetent,(char *, char *));
#endif /* __DECC */
#endif /* VMS */
#endif /* CK_CURSES */

#ifndef NOXMIT
#define XMITF 0                         /* SET TRANSMIT values */
#define XMITL 1                         /* (Local to this module) */
#define XMITP 2
#define XMITE 3
#define XMITX 4
#define XMITS 5
#define XMITW 6
#define XMITT 7

#define XMBUFL 50
extern int xmitf, xmitl, xmitp, xmitx, xmits, xmitw, xmitt;
char xmitbuf[XMBUFL+1] = { NUL };       /* TRANSMIT eof string */

struct keytab xmitab[] = {              /* SET TRANSMIT */
    { "echo",          XMITX, 0 },
    { "eof",           XMITE, 0 },
    { "fill",          XMITF, 0 },
    { "linefeed",      XMITL, 0 },
    { "locking-shift", XMITS, 0 },
    { "pause",         XMITW, 0 },
    { "prompt",        XMITP, 0 },
    { "timeout",       XMITT, 0 }
};
int nxmit = (sizeof(xmitab) / sizeof(struct keytab));
#endif /* NOXMIT */

/* For SET FILE COLLISION */
/* Some of the following may be possible for some C-Kermit implementations */
/* but not others.  Those that are not possible for your implementation */
/* should be ifdef'd out. */

struct keytab colxtab[] = { /* SET FILE COLLISION options */
#ifndef MAC
    { "append",    XYFX_A, 0 },         /* append to old file */
#endif /* MAC */
#ifdef COMMENT
    { "ask",       XYFX_Q, 0 },         /* ask what to do (not implemented) */
#endif
    { "backup",    XYFX_B, 0 },         /* rename old file */
#ifndef MAC
    /* This crashes Mac Kermit. */
    { "discard",   XYFX_D, CM_INV },	/* don't accept new file */
    { "no-supersede", XYFX_D, CM_INV }, /* ditto (MSK compatibility) */
#endif /* MAC */
    { "overwrite", XYFX_X, 0 },         /* overwrite the old file */
    { "reject",    XYFX_D, 0 },		/* (better word than discard) */
    { "rename",    XYFX_R, 0 },         /* rename the incoming file */
#ifndef MAC                             /* This crashes Mac Kermit. */
    { "update",    XYFX_U, 0 },         /* replace if newer */
#endif /* MAC */
    { "", 0, 0 }
};
int ncolx = (sizeof(colxtab) / sizeof(struct keytab)) - 1;

static struct keytab rfiltab[] = {      /* for REMOTE SET FILE */
#ifndef NOCSETS
    { "character-set", XYFILC, 0 },
#endif /* NOCSETS */
    { "collision",     XYFILX, 0 },
    { "incomplete",    XYFILI, 0 },
    { "names",         XYFILN, 0 },
    { "record-length", XYFILR, 0 },
    { "type",          XYFILT, 0 }
};
int nrfilp = (sizeof(rfiltab) / sizeof(struct keytab));

struct keytab eoltab[] = {              /* File eof delimiters */
    { "cr",        XYFA_C, 0 },
    { "crlf",      XYFA_2, 0 },
    { "lf",        XYFA_L, 0 }
};
static int neoltab = (sizeof(eoltab) / sizeof(struct keytab));

struct keytab fntab[] = {               /* File naming */
    { "converted", XYFN_C, 0      },
    { "literal",   XYFN_L, 0      },
    { "standard",  XYFN_C, CM_INV }
};
int nfntab = (sizeof(fntab) / sizeof(struct keytab));

#ifndef NOLOCAL
/* Terminal parameters table */
static struct keytab trmtab[] = {
#ifdef OS2
    { "answerback",    XYTANS,    0 },
#endif /* OS2 */
#ifdef CK_APC
    { "apc",           XYTAPC,    0 },
#endif /* CK_APC */
#ifdef OS2
    { "arrow-keys",    XYTARR,    0 },
#endif /* OS2 */
#ifdef NT
    { "at",            XYTATTR,   CM_INV|CM_ABR },
    { "att",           XYTATTR,   CM_INV|CM_ABR },
    { "attr",          XYTATTR,   CM_INV|CM_ABR },
    { "attr-bug",      XYTATTBUG, CM_INV },
#endif /* NT */
#ifdef OS2
    { "attribute",     XYTATTR,   0 },
#endif /* OS2 */
#ifdef CK_APC
#ifdef CK_AUTODL
   { "autodownload",   XYTAUTODL, 0, },
#endif /* CK_AUTODL */
#endif /* CK_APC */
#ifdef OS2
    { "autopage",      XYTAPAGE,  0 },
    { "autoscroll",    XYTASCRL,  0 },
    { "bell",          XYTBEL,    CM_INV },
#endif /* OS2 */
    { "bytesize",      XYTBYT,    0 },
#ifndef NOCSETS
    { "character-set", XYTCS,     0 },
#endif /* NOCSETS */
#ifdef OS2
    { "code-page",     XYTCPG,    0 },
    { "color",         XYTCOL,    0 },
    { "controls",      XYTCTRL,   0 },
#endif /* OS2 */
    { "cr-display",    XYTCRD,    0 },
#ifdef OS2
    { "cursor",        XYTCUR,    0 },
#endif /* OS2 */
    { "debug",         XYTDEB,    0 },
#ifdef OS2
    { "dg-unix-mode",  XYTUNX,    0 },
#endif /* OS2 */
    { "echo",          XYTEC,     0 },
    { "escape-character", XYTESC, 0 },
#ifdef OS2
#ifdef PCFONTS
    { "font",          XYTFON,    0 },
#else
#ifdef KUI
    { "font",          XYTFON,    0 },
#endif /* KUI */
#endif /* PCFONTS */
#endif /* OS2 */
    { "height",        XYTHIG,    0 },
#ifdef CKTIDLE
    { "idle-action",   XYTIACT,   0 },
    { "idle-limit",    XYTITMO,   CM_INV },
    { "idle-send",     XYTIDLE,   CM_INV },
    { "idle-timeout",  XYTITMO,   0 },
#endif /* CKTIDLE */
#ifdef OS2
#ifndef NOCSETS
    { "kbd-follows-gl/gr", XYTKBDGL, 0 },
#endif /* NOCSETS */
    { "key",           XYTKEY,    0 },
    { "keyboard-mode", XYTKBMOD,  0 },
    { "keypad-mode",   XYTKPD,    0 },
#endif /* OS2 */
    { "lf-display",    XYTLFD,    0 },
#ifndef NOCSETS
#ifdef OS2
#ifndef KUI
    { "line-spacing",  XYTLSP,    CM_INV },
    { "local-character-set", XYTLCS,  0 },
#else
    { "line-spacing",  XYTLSP,    0 },
    { "local-character-set", XYTLCS,  CM_INV },
#endif /* KUI */
#else
    { "local-character-set", XYTLCS,  CM_INV },
#endif /* OS2 */
#endif /* NOCSETS */
    { "locking-shift", XYTSO,     0 },
#ifdef OS2
    { "margin-bell",   XYTMBEL,   0 },
#endif /* OS2 */
#ifdef OS2MOUSE
    { "mouse",         XYTMOU,    CM_INV },
#endif /* OS2MOUSE */
    { "newline-mode",  XYTNL,     0 },
#ifdef OS2
    { "output-pacing", XYTPAC,    0 },
#ifdef PCTERM
    { "pcterm",        XYTPCTERM, 0 },
#endif /* PCTERM */
#endif /* OS2 */
#ifdef OS2ORUNIX
    { "print",         XYTPRN,    0 },
#endif /* OS2ORUNIX */
#ifndef NOCSETS
#ifdef OS2
    { "remote-character-set", XYTRCS,  0 },
#else
    { "remote-character-set", XYTRCS,  CM_INV },
#endif /* OS2 */
#endif /* NOCSETS */
#ifdef OS2
    { "roll-mode",       XYTROL, 0 },
    { "s",               XYTUPD, CM_ABR|CM_INV },
    { "sc",              XYTUPD, CM_ABR|CM_INV },
    { "scr",             XYTUPD, CM_ABR|CM_INV },
    { "scree",           XYTUPD, CM_ABR|CM_INV },
    { "screen",          XYTUPD, CM_ABR|CM_INV },
    { "screen-",         XYTUPD, CM_ABR|CM_INV },
    { "screen-mode",     XYTSCNM,   0 },
    { "screen-optimize", XYTOPTI,   0 },
    { "screen-update",   XYTUPD,    0 },
    { "scrollback",      XYSCRS,    0 },
    { "send-data",         XYTSEND, 0 },
    { "send-end-of-block", XYTSEOB, 0 },
    { "sgr-colors",            XYTSGRC,  0 },
    { "sni-ch.code",           XYTSNICC, 0 },
    { "sni-firmware-versions", XYTSNIFV, 0 },
    { "sni-language",          XYTVTLNG, 0 },
    { "sni-pagemode",          XYTSNIPM, CM_INV },
    { "sni-scrollmode",              XYTSNISM, CM_INV },
    { "spacing-attribute-character", XYTSAC,   CM_INV },
    { "statusline",                  XYTSTAT,  0 },
    { "tra",                         XYTCTS,   CM_INV|CM_ABR },
    { "transmit-timeout",            XYTCTS,   0 },
#endif /* OS2 */

#ifdef OS2ORUNIX
    { "transparent-print", XYTPRN,   CM_INV },
#endif /* OS2ORUNIX */

#ifdef CK_TRIGGER
    { "trigger",           XYTRIGGER,0 },
#endif /* CK_TRIGGER */
#ifdef OS2
    { "type",              XYTTYP,   0 },
#else
    { "type",              XYTTYP,   CM_INV },
#endif /* OS2 */

#ifndef NOCSETS
#ifdef UNICODE
#ifdef CKOUNI
    { "unicode",           XYTUNI,   CM_INV },
#endif /* CKOUNI */
#endif /* UNICODE */
#endif /* NOCSETS */
#ifdef OS2
    { "unix-mode",         XYTUNX,   CM_INV },
    { "url-highlight",     XYTURLHI, 0 },
#ifdef NT
    { "video-change",      XYTVCH,   0 },
#endif /* NT */
    { "vt-language",       XYTVTLNG, 0 },
    { "vt-nrc-mode",       XYTVTNRC, 0 },
#endif /* OS2 */
    { "width",             XYTWID,   0 },
#ifdef OS2
    { "wrap",              XYTWRP,   0 },
#endif /* OS2 */
    { "", 0, 0 }
};
int ntrm = (sizeof(trmtab) / sizeof(struct keytab)) - 1;

#ifdef OS2
struct keytab termctrl[] = {    /* SET TERM CONTROLS */
    { "7",      7, 0 },
    { "8",      8, 0 }
};
int ntermctrl = (sizeof(termctrl) / sizeof(struct keytab));

struct keytab curontab[] = {    /* SET TERM CURSOR */
#ifdef KUI
    { "noblink", 2, 0 },
#else
    { "noblink", 2, CM_INV },
#endif /* KUI */
    { "off",     0, 0 },
    { "on",      1, 0 }
};
int ncuron = (sizeof(curontab) / sizeof(struct keytab));

struct keytab rolltab[] = {   /* Set TERM Roll Options */
    { "insert",    TTR_INSERT, 0      },
    { "keystrokes",TTR_KEYS,   0      },
    { "off",       TTR_OVER,   CM_INV },
    { "on",        TTR_INSERT, CM_INV },
    { "overwrite", TTR_OVER,   0      }
};
int nroll = (sizeof(rolltab) / sizeof(struct keytab));

struct keytab rollkeytab[] = {		/* Set TERM ROLL KEYSTROKES */
    { "ignore",            TTRK_IGN, 0 },
    { "restore-and-send",  TTRK_RST, 0 },
    { "send",              TTRK_SND, 0 }
};
int nrollkey = (sizeof(rollkeytab) / sizeof(struct keytab));

#define TT_GR_ALL 4
#define TT_GR_G0  0
#define TT_GR_G1  1
#define TT_GR_G2  2
#define TT_GR_G3  3
#define TT_GR_KBD 4
struct keytab graphsettab[] = {  /* DEC VT Graphic Sets */
    { "all",      TT_GR_ALL, 0 },
    { "g0",       TT_GR_G0,  0 },
    { "g1",       TT_GR_G1,  0 },
    { "g2",       TT_GR_G2,  0 },
    { "g3",       TT_GR_G3,  0 },
    { "keyboard", TT_GR_KBD, 0 }
};
int ngraphset = (sizeof(graphsettab) / sizeof(struct keytab));
#endif /* OS2 */

struct keytab adltab[] = {              /* Autodownload Options */
    { "ask",     TAD_ASK, 0 },
    { "error",   TAD_ERR, 0 },
#ifdef OS2
    { "kermit",  TAD_K,   0 },
#endif /* OS2 */
    { "off",     TAD_OFF, 0 },
    { "on",      TAD_ON,  0 },
#ifdef OS2
    { "zmodem",  TAD_Z,   0 },
#endif /* OS2 */
    { "", 0, 0 }
};
int nadltab = (sizeof(adltab) / sizeof(struct keytab)) - 1;

struct keytab adlerrtab[] = {           /* Autodownload Error Options */
    { "continue", 0, 0 },
    { "go",       0, CM_INV },
    { "stop",     1, 0 }
};
int nadlerrtab = (sizeof(adlerrtab) / sizeof(struct keytab));

#ifdef OS2
struct keytab adlxtab[] = {             /* Autodownload Options */
    { "c0-conflicts",     TAD_X_C0,     0 },
    { "detection-method", TAD_X_DETECT, 0 },
    { "string",           TAD_X_STR,    0 }
};
int nadlxtab = (sizeof(adlxtab) / sizeof(struct keytab));

struct keytab adldtab[] = {             /* Auto-dl Detection Methods */
    { "packet",           ADL_PACK,     0 },
    { "string",           ADL_STR,      0 }
};
int nadldtab = (sizeof(adldtab) / sizeof(struct keytab));

struct keytab adlc0tab[] = {            /* Auto-dl Detection Methods */
    { "ignored-by-emulator",    0,      0 },
    { "processed-by-emulator",  1,      0 }
};
int nadlc0tab = (sizeof(adlc0tab) / sizeof(struct keytab));

#ifndef NOCSETS
struct keytab vtlangtab[] = {
    { "belgian",        VTL_BELGIAN , 0 },
    { "british",        VTL_BRITISH , 0 },
    { "canadian",       VTL_CANADIAN, 0 },
    { "czech",          VTL_CZECH   , 0 },
    { "danish",         VTL_DANISH  , 0 },
    { "dutch",          VTL_DUTCH   , 0 },
    { "finnish",        VTL_FINNISH , 0 },
    { "french",         VTL_FRENCH  , 0 },
    { "french-canadian",VTL_FR_CAN  , 0 },
    { "german",         VTL_GERMAN  , 0 },
    { "greek",          VTL_GREEK   , 0 },
    { "hebrew",         VTL_HEBREW  , 0 },
    { "hungarian",      VTL_HUNGARIA, 0 },
    { "italian",        VTL_ITALIAN , 0 },
    { "latin-american", VTL_LATIN_AM, 0 },
    { "north-american", VTL_NORTH_AM, 0 },
    { "norwegian",      VTL_NORWEGIA, 0 },
    { "polish",         VTL_POLISH  , 0 },
    { "portugese",      VTL_PORTUGES, 0 },
    { "romanian",       VTL_ROMANIAN, 0 },
    { "russian",        VTL_RUSSIAN , 0 },
    { "scs",            VTL_SCS     , CM_INV },
    { "slovak",         VTL_SLOVAK  , 0 },
    { "spanish",        VTL_SPANISH , 0 },
    { "swedish",        VTL_SWEDISH , 0 },
    { "swiss-french",   VTL_SW_FR   , 0 },
    { "swiss-german",   VTL_SW_GR   , 0 },
    { "turkish-f",      VTL_TURK_F  , CM_INV },
    { "turkish-q",      VTL_TURK_Q  , CM_INV }
};
int nvtlangtab = (sizeof(vtlangtab) / sizeof(struct keytab));
#endif /* NOCSETS */
#endif /* OS2 */

struct keytab crdtab[] = {              /* Carriage-return display */
    { "crlf",        1, 0 },
    { "normal",      0, 0 }
};
extern int tt_crd;                      /* Carriage-return display variable */
extern int tt_lfd;			/* Linefeed display variable */

#ifdef CK_APC
extern int apcstatus, apcactive;
static struct keytab apctab[] = {       /* Terminal APC parameters */
    {  "no-input", APC_ON|APC_NOINP,0 },
    { "off",       APC_OFF,  0 },
    { "on",        APC_ON,   0 },
    { "unchecked", APC_ON|APC_UNCH, 0 },
    { "unchecked-no-input", APC_ON|APC_NOINP|APC_UNCH, 0 }
};
int napctab = (sizeof(apctab) / sizeof(struct keytab));
#endif /* CK_APC */
#endif /* NOLOCAL */

extern int autodl, adl_err, adl_ask;

struct keytab beltab[] = {              /* Terminal bell mode */
#ifdef OS2
    { "audible", XYB_AUD,  0 },
    { "none",    XYB_NONE, 0 },
#else
    { "audible", XYB_AUD,  CM_INV },
    { "none",    XYB_NONE, CM_INV },
#endif /* OS2 */
#ifdef OS2
    { "off",     XYB_NONE, CM_INV },
    { "on",      XYB_AUD,  CM_INV },
#else
    { "off",     XYB_NONE, 0 },
    { "on",      XYB_AUD,  0 },
#endif /* OS2 */
#ifdef OS2
    { "visible", XYB_VIS,  0 },
#endif /* OS2 */
    { "", 0, 0 }
};
int nbeltab = sizeof(beltab)/sizeof(struct keytab) - 1;

int tt_unicode = 1;                     /* Use Unicode if possible */
#ifdef CKTIDLE
int tt_idlesnd_tmo = 0;                 /* Idle Send Timeout, disabled */
char * tt_idlesnd_str = NULL;           /* Idle Send String, none */
char * tt_idlestr = NULL;
extern int tt_idleact, tt_idlelimit;
#endif /* CKTIDLE */

#ifdef OS2
#ifndef NOLOCAL
/*
  OS/2 serial communication devices.
*/
struct keytab os2devtab[] = {
    { "1",    1, CM_INV },                      /* Invisible synonyms, like */
    { "2",    2, CM_INV },                      /* "set port 1" */
    { "3",    3, CM_INV },
    { "4",    4, CM_INV },
    { "5",    5, CM_INV },
    { "6",    6, CM_INV },
    { "7",    7, CM_INV },
    { "8",    8, CM_INV },
    { "com1", 1, 0 },                   /* Real device names */
    { "com2", 2, 0 },
    { "com3", 3, 0 },
    { "com4", 4, 0 },
    { "com5", 5, 0 },
    { "com6", 6, 0 },
    { "com7", 7, 0 },
    { "com8", 8, 0 },
#ifdef OS2ONLY
    { "slipcom1", 1, 0 },                       /* For use with SLIP driver */
    { "slipcom2", 2, 0 },                       /* shared access */
    { "slipcom3", 3, 0 },
    { "slipcom4", 4, 0 },
    { "slipcom5", 5, 0 },
    { "slipcom6", 6, 0 },
    { "slipcom7", 7, 0 },
    { "slipcom8", 8, 0 },
    { "pppcom1", 1, 0 },                        /* For use with PPP driver */
    { "pppcom2", 2, 0 },                        /* shared access */
    { "pppcom3", 3, 0 },
    { "pppcom4", 4, 0 },
    { "pppcom5", 5, 0 },
    { "pppcom6", 6, 0 },
    { "pppcom7", 7, 0 },
    { "pppcom8", 8, 0 }
#endif /* OS2ONLY */
};
int nos2dev = (sizeof(os2devtab) / sizeof(struct keytab)) - 1;

#ifdef OS2ONLY
struct keytab os2ppptab[] = {
    { "0",    0, CM_INV },
    { "1",    1, CM_INV },                      /* Invisible synonyms, like */
    { "2",    2, CM_INV },                      /* "set port 1" */
    { "3",    3, CM_INV },
    { "4",    4, CM_INV },
    { "5",    5, CM_INV },
    { "6",    6, CM_INV },
    { "7",    7, CM_INV },
    { "8",    8, CM_INV },
    { "9",    9, CM_INV },
    { "ppp0", 0, 0 },
    { "ppp1", 1, 0 },                   /* For use with PPP driver */
    { "ppp2", 2, 0 },                   /* shared access */
    { "ppp3", 3, 0 },
    { "ppp4", 4, 0 },
    { "ppp5", 5, 0 },
    { "ppp6", 6, 0 },
    { "ppp7", 7, 0 },
    { "ppp8", 8, 0 },
    { "ppp9", 9, 0 }
};
int nos2ppp = (sizeof(os2ppptab) / sizeof(struct keytab));
#endif /* OS2ONLY */

/*
  Terminal parameters that can be set by SET commands.
  Used by the ck?con.c terminal emulator code.
  For now, only used for #ifdef OS2.  Should add these for Macintosh.
*/
int tt_arrow = TTK_NORM;                /* Arrow key mode: normal (cursor) */
int tt_keypad = TTK_NORM;               /* Keypad mode: normal (numeric) */
int tt_shift_keypad = 0;                /* Keypad Shift mode: Off */
int tt_wrap = 1;                        /* Terminal wrap, 1 = On */
int tt_type = TT_VT220;                 /* Terminal type, initially VT220 */
int tt_type_mode = TT_VT220;            /* Terminal type set by host command */
int tt_cursor = 0;                      /* Terminal cursor, 0 = Underline */
int tt_cursor_usr = 0;                  /* Users Terminal cursor type */
int tt_cursorena_usr = 1;               /* Users Terminal cursor enabled */
int tt_cursor_blink = 1;                /* Terminal Cursor Blink */
int tt_answer = 0;                      /* Terminal answerback (disabled) */
int tt_scrsize[VNUM] = {512,512,512,1}; /* Terminal scrollback buffer size */
int tt_roll[VNUM] = {1,1,1,1};          /* Terminal roll (on) */
int tt_rkeys[VNUM] = {1,1,1,1};		/* Terminal roll keys (send) */
int tt_pacing = 0;                      /* Terminal output-pacing (none) */
int tt_ctstmo = 15;                     /* Terminal transmit-timeout */
int tt_codepage = -1;                   /* Terminal code-page */
int tt_update = 100;                    /* Terminal screen-update interval */
int tt_updmode = TTU_FAST;              /* Terminal screen-update mode FAST */
extern int updmode;
#ifndef KUI
int tt_status[VNUM] = {1,1,0,0};        /* Terminal status line displayed */
int tt_status_usr[VNUM] = {1,1,0,0};
#else  /* KUI */
extern CKFLOAT floatval;
CKFLOAT tt_linespacing[VNUM] = {1.0,1.0,1.0,1.0};
#ifdef K95G
int tt_status[VNUM] = {1,1,0,0};        /* Terminal status line displayed */
int tt_status_usr[VNUM] = {1,1,0,0};
#else /* K95G */
int tt_status[VNUM] = {0,0,0,0};        /* Terminal status line displayed */
int tt_status_usr[VNUM] = {0,0,0,0};
#endif /* K95G */
#endif /* KUI */
int tt_senddata = 0;                    /* Let host read terminal data */
extern int wy_blockend;                 /* Terminal Send Data EOB type */
int tt_hidattr = 1;                     /* Attributes are hidden */

extern unsigned char colornormal, colorselect,
colorunderline, colorstatus, colorhelp, colorborder,
colorgraphic, colordebug, colorreverse, coloritalic;

extern int trueblink, trueunderline, truereverse, trueitalic, truedim;

extern int bgi, fgi;
extern int scrninitialized[];

struct keytab audibletab[] = {          /* Terminal Bell Audible mode */
    { "beep",          XYB_BEEP, 0 },   /* Values ORd with bell mode */
    { "system-sounds", XYB_SYS,  0 }
};
int naudibletab = sizeof(audibletab)/sizeof(struct keytab);

struct keytab akmtab[] = {              /* Arrow key mode */
    { "application", TTK_APPL, 0 },
    { "cursor",      TTK_NORM, 0 }
};
struct keytab kpmtab[] = {              /* Keypad mode */
    { "application", TTK_APPL, 0 },
    { "numeric",     TTK_NORM, 0 }
};

struct keytab ttcolmodetab[] = {
    { "current-color", 0, 0 },
    { "default-color", 1, 0 }
};
int ncolmode = sizeof(ttcolmodetab)/sizeof(struct keytab);

#define TTCOLNOR  0
#define TTCOLREV  1
#define TTCOLUND  2
#define TTCOLSTA  3
#define TTCOLHLP  4
#define TTCOLBOR  5
#define TTCOLSEL  6
#define TTCOLDEB  7
#define TTCOLGRP  8
#define TTCOLITA  9
#define TTCOLRES  10
#define TTCOLERA  11

struct keytab ttycoltab[] = {                   /* Terminal Screen coloring */
    { "border",             TTCOLBOR, 0 },      /* Screen border color */
    { "debug-terminal",     TTCOLDEB, 0 },      /* Debug color */
    { "erase",              TTCOLERA, 0 },      /* Erase mode */
    { "graphic",            TTCOLGRP, 0 },      /* Graphic Color */
    { "help-text",          TTCOLHLP, 0 },      /* Help screens */
    { "italic",             TTCOLITA, 0 },      /* Italic Color */
    { "normal",             TTCOLNOR, CM_INV }, /* Normal screen text */
    { "reset-on-esc[0m",    TTCOLRES, 0 },      /* Reset on ESC [ 0 m */
    { "reverse-video",      TTCOLREV, 0 },      /* Reverse video */
    { "status-line",        TTCOLSTA, 0 },      /* Status line */
    { "selection",          TTCOLSEL, 0 },      /* Selection color */
    { "terminal-screen",    TTCOLNOR, 0 },      /* Better name than "normal" */
    { "underlined-text",    TTCOLUND, 0 }       /* Underlined text */
};
int ncolors = (sizeof(ttycoltab) / sizeof(struct keytab));

#define TTATTNOR  0
#define TTATTBLI  1
#define TTATTREV  2
#define TTATTUND  3
#define TTATTPRO  4
#define TTATTBLD  5
#define TTATTDIM  6
#define TTATTINV  7
#define TTATTITA  8
#define TTATTDONE 9

struct keytab ttyattrtab[] = {
    { "blink",     TTATTBLI, 0 },
    { "dim",       TTATTDIM, 0 },
    { "italic",    TTATTITA, 0 },
    { "protected", TTATTPRO, 0 },
    { "reverse",   TTATTREV, 0 },
    { "underline", TTATTUND, 0 }
};
int nattrib = (sizeof(ttyattrtab) / sizeof(struct keytab));

struct keytab ttyprotab[] = {
    { "blink",       TTATTBLI,  0 },
    { "bold",        TTATTBLD,  0 },
    { "dim",         TTATTDIM,  0 },
    { "done",        TTATTDONE, CM_INV },
    { "invisible",   TTATTINV,  0 },
    { "italic",      TTATTITA,  0 },
    { "normal",      TTATTNOR,  0 },
    { "reverse",     TTATTREV,  0 },
    { "underlined",  TTATTUND,  0 }

};
int nprotect = (sizeof(ttyprotab) / sizeof(struct keytab));

struct keytab ttyseobtab[] = {
    { "crlf_etx",  1, 0 },
    { "us_cr",     0, 0 }
};

struct keytab ttyclrtab[] = {           /* Colors */
    { "black",         0, 0      },
    { "blue",          1, 0      },
    { "brown",         6, 0      },
    { "cyan",          3, 0      },
    { "darkgray",      8, CM_INV },
    { "dgray",         8, 0      },
    { "green",         2, 0      },
    { "lblue",         9, CM_INV },
    { "lcyan",        11, CM_INV },
    { "lgray",         7, CM_INV },
    { "lgreen",       10, CM_INV },
    { "lightblue",     9, 0      },
    { "lightcyan",    11, 0      },
    { "lightgray",     7, 0      },
    { "lightgreen",   10, 0      },
    { "lightmagenta", 13, 0      },
    { "lightred",     12, 0      },
    { "lmagenta",     13, CM_INV },
    { "lred",         12, CM_INV },
    { "magenta",       5, 0      },
    { "red",           4, 0      },
    { "white",        15, 0      },
    { "yellow",       14, 0      }
};
int nclrs = (sizeof (ttyclrtab) / sizeof (struct keytab));

struct keytab ttycurtab[] = {
    { "full",        TTC_BLOCK, 0 },
    { "half",        TTC_HALF,  0 },
    { "underline",   TTC_ULINE, 0 }
};
int ncursors = 3;

struct keytab ttyptab[] = {
    { "aaa",      TT_AAA,     CM_INV },     /* AnnArbor */
    { "adm3a",    TT_ADM3A,   0 },          /* LSI ADM-3A */
    { "adm5",     TT_ADM5,    0 },          /* LSI ADM-5 */
    { "aixterm",  TT_AIXTERM, 0 },          /* IBM AIXterm */
    { "annarbor", TT_AAA,     0 },          /* AnnArbor */
    { "ansi-bbs", TT_ANSI,    0 },          /* ANSI.SYS (BBS) */
    { "at386",    TT_AT386,   0 },          /* Unixware ANSI */
    { "avatar/0+",TT_ANSI,    0 },          /* AVATAR/0+ */
    { "ba80",     TT_BA80,    0 },          /* Nixdorf BA80 */
    { "be",       TT_BEOS,    CM_INV|CM_ABR },
    { "beos-ansi",TT_BEOS,    CM_INV },     /* BeOS ANSI */
    { "beterm",   TT_BEOS,    0 },          /* BeOS Terminal (as of PR2 ) */
    { "d200",     TT_DG200,   CM_INV|CM_ABR }, /* Data General DASHER 200 */
    { "d210",     TT_DG210,   CM_INV|CM_ABR }, /* Data General DASHER 210 */
    { "d217",     TT_DG217,   CM_INV|CM_ABR }, /* Data General DASHER 217 */
    { "dg200",    TT_DG200,   0 },          /* Data General DASHER 200 */
    { "dg210",    TT_DG210,   0 },          /* Data General DASHER 210 */
    { "dg217",    TT_DG217,   0 },          /* Data General DASHER 217 */
    { "h1500",    TT_HZL1500, CM_INV },     /* Hazeltine 1500 */
    { "h19",      TT_H19,     CM_INV },     /* Heath-19 */
    { "heath19",  TT_H19,     0 },          /* Heath-19 */
    { "hft",      TT_HFT,     0 },          /* IBM High Function Terminal */
    { "hp2621a",  TT_HP2621,  0 },          /* HP 2621A */
    { "hpterm",   TT_HPTERM,  0 },          /* HP TERM */
    { "hz1500",   TT_HZL1500, 0 },          /* Hazeltine 1500 */
    { "ibm3151",  TT_IBM31,   0 },          /* IBM 3101-xx,3161 */
    { "linux",    TT_LINUX,   0 },          /* Linux */
    { "qansi",    TT_QANSI,   0 },          /* QNX ANSI */
    { "qnx",      TT_QNX,     0 },          /* QNX Console */
    { "scoansi",  TT_SCOANSI, 0 },          /* SCO ANSI */
    { "sni-97801",TT_97801,   0 },          /* SNI 97801 */
    { "sun",      TT_SUN,     0 },          /* SUN Console */
/*
  The idea of NONE is to let the console driver handle the escape sequences,
  which, in theory at least, would give not only ANSI emulation, but also any
  other kind of emulation that might be provided by alternative console
  drivers, if any existed.

  For this to work, ckocon.c would need to be modified to make higher-level
  calls, like VioWrtTTY(), DosWrite(), or (simply) write(), rather than
  VioWrt*Cell() and similar, and it would also have to give up its rollback
  feature, and its status line and help screens would also have to be
  forgotten or else done in an ANSI way.

  As matters stand, we already have perfectly good ANSI emulation built in,
  and there are no alternative console drivers available, so there is no point
  in having a terminal type of NONE, so it is commented out.  However, should
  you uncomment it, it will work like a "glass tty" -- no escape sequence
  interpretation at all; somewhat similar to debug mode, except without the
  debugging (no highlighting of control chars or escape sequences); help
  screens, status line, and rollback will still work.
*/
#ifdef OS2PM
#ifdef COMMENT
    { "tek4014", TT_TEK40,  0 },
#endif /* COMMENT */
#endif /* OS2PM */
    { "tty",     TT_NONE,   0 },
    { "tvi910+", TT_TVI910, 0 },
    { "tvi925",  TT_TVI925, 0 },
    { "tvi950",  TT_TVI950, 0 },
    { "vc404",   TT_VC4404, 0 },
    { "vc4404",  TT_VC4404, CM_INV },
    { "vip7809", TT_VIP7809,0 },
    { "vt100",   TT_VT100,  0 },
    { "vt102",   TT_VT102,  0 },
    { "vt220",   TT_VT220,  0 },
    { "vt220pc", TT_VT220PC,0 },
    { "vt320",   TT_VT320,  0 },
    { "vt320pc", TT_VT320PC,0 },
    { "vt52",    TT_VT52,   0 },
#ifdef NT
    { "vtnt",    TT_VTNT,   0 },
#else /* NT */
    { "vtnt",    TT_VTNT,  CM_INV },
#endif /* NT */
    { "wy160",   TT_WY160,  0 },
    { "wy30",    TT_WY30,   0 },
    { "wy370",   TT_WY370,  0 },
    { "wy50",    TT_WY50,   0 },
    { "wy60",    TT_WY60,   0 },
    { "wyse30",  TT_WY30,   CM_INV },
    { "wyse370", TT_WY370,  CM_INV },
    { "wyse50",  TT_WY50,   CM_INV },
    { "wyse60",  TT_WY60,   CM_INV }
};
int nttyp = (sizeof(ttyptab) / sizeof(struct keytab));

struct keytab ttkeytab[] = {
    { "aaa",       TT_AAA,        CM_INV },        /* AnnArbor */
    { "adm3a",     TT_ADM3A,      0 },             /* LSI ADM-3A */
    { "adm5",      TT_ADM5,       0 },             /* LSI ADM-5 */
    { "aixterm",   TT_AIXTERM,    0 },             /* IBM AIXterm */
    { "annarbor",  TT_AAA,        0 },             /* AnnArbor */
    { "ansi-bbs",  TT_ANSI,       0 },             /* ANSI.SYS (BBS) */
    { "at386",     TT_AT386,      0 },             /* Unixware ANSI */
    { "avatar/0+", TT_ANSI,       0 },             /* AVATAR/0+ */
    { "ba80",      TT_BA80,       0 },             /* Nixdorf BA80 */
    { "be",        TT_BEOS,       CM_INV|CM_ABR },
    { "beos-ansi", TT_BEOS,       CM_INV },        /* BeOS ANSI */
    { "beterm",    TT_BEOS,       0 },             /* BeOS Terminal (DR2) */
    { "d200",      TT_DG200,      CM_INV|CM_ABR }, /* DG DASHER 200 */
    { "d210",      TT_DG210,      CM_INV|CM_ABR }, /* DG DASHER 210 */
    { "d217",      TT_DG217,      CM_INV|CM_ABR }, /* DG DASHER 217 */
    { "dg200",     TT_DG200,      0 },             /* DG DASHER 200 */
    { "dg210",     TT_DG210,      0 },             /* DG DASHER 210 */
    { "dg217",     TT_DG217,      0 },             /* DG DASHER 217 */
    { "emacs",     TT_KBM_EMACS,  0 },             /* Emacs mode */
    { "h19",       TT_H19,        CM_INV },        /* Heath-19 */
    { "heath19",   TT_H19,        0 },             /* Heath-19 */
    { "hebrew",    TT_KBM_HEBREW, 0 },             /* Hebrew mode */
    { "hft",       TT_HFT,        0 },             /* IBM High Function Term */
    { "hp2621a",   TT_HP2621,     0 },             /* HP 2621A */
    { "hpterm",    TT_HPTERM,     0 },             /* HP TERM */
    { "hz1500",    TT_HZL1500,    0 },             /* Hazeltine 1500 */
    { "ibm3151",   TT_IBM31,      0 },             /* IBM 3101-xx,3161 */
    { "linux",     TT_LINUX,      0 },             /* Linux */
    { "qansi",     TT_QANSI,      0 },             /* QNX ANSI */
    { "qnx",       TT_QNX,        0 },             /* QNX */
    { "russian",   TT_KBM_RUSSIAN,0 },             /* Russian mode */
    { "scoansi",   TT_SCOANSI,    0 },             /* SCO ANSI */
    { "sni-97801", TT_97801,      0 },             /* SNI 97801 */
    { "sun",       TT_SUN,        0 },             /* SUN Console */
#ifdef OS2PM
#ifdef COMMENT
    { "tek4014",   TT_TEK40,      0 },
#endif /* COMMENT */
#endif /* OS2PM */
    { "tty",       TT_NONE,       0 },
    { "tvi910+",   TT_TVI910,     0 },
    { "tvi925",    TT_TVI925,     0 },
    { "tvi950",    TT_TVI950,     0 },
    { "vc404",     TT_VC4404,     0 },
    { "vc4404",    TT_VC4404,     CM_INV },
    { "vip7809",   TT_VIP7809,    0 },
    { "vt100",     TT_VT100,      0 },
    { "vt102",     TT_VT102,      0 },
    { "vt220",     TT_VT220,      0 },
    { "vt220pc",   TT_VT220PC,    0 },
    { "vt320",     TT_VT320,      0 },
    { "vt320pc",   TT_VT320PC,    0 },
    { "vt52",      TT_VT52,       0 },
    { "vtnt",      TT_VTNT,       CM_INV },
    { "wp",        TT_KBM_WP,     0 },             /* Word Perfect mode */
    { "wy160",     TT_WY160,      0 },
    { "wy30",      TT_WY30,       0 },
    { "wy370",     TT_WY370,      0 },
    { "wy50",      TT_WY50,       0 },
    { "wy60",      TT_WY60,       0 },
    { "wyse30",    TT_WY30,       CM_INV },
    { "wyse370",   TT_WY370,      CM_INV },
    { "wyse50",    TT_WY50,       CM_INV },
    { "wyse60",    TT_WY60,       CM_INV }
};
int nttkey = (sizeof(ttkeytab) / sizeof(struct keytab));

#ifndef NOSETKEY
struct keytab kbmodtab[] = {
    { "emacs",   KBM_EM, 0      },
    { "english", KBM_EN, CM_INV },
    { "hebrew",  KBM_HE, 0      },
    { "normal",  KBM_EN, 0      },
    { "none",    KBM_EN, CM_INV },
    { "russian", KBM_RU, 0      },
    { "wp",      KBM_WP, 0      }
};
int nkbmodtab = (sizeof(kbmodtab) / sizeof(struct keytab));
#endif /* NOSETKEY */
#endif /* NOLOCAL */

int tt_inpacing = 0;                    /* input-pacing (none) */

struct keytab prtytab[] = { /* OS/2 Priority Levels */
    { "foreground-server", XYP_SRV, 0       },
    { "idle",              XYP_IDLE, CM_INV },
    { "regular",           XYP_REG, 0       },
    { "time-critical",     XYP_RTP, 0       }
};
int nprty = (sizeof(prtytab) / sizeof(struct keytab));
#endif /* OS2 */

#ifdef NT
struct keytab win95tab[] = { /* Win95 work-arounds */
    { "8.3-filenames",         XYW8_3,    0 },
    { "alt-gr",                XYWAGR,    0 },
    { "horizontal-scan-line-substitutions", XYWHSL, 0 },
    { "keyboard-translation",  XYWKEY,    0 },
    { "lucida-substitutions",  XYWLUC,    0 },
    { "overlapped-io",         XYWOIO,    0 },
    { "popups",                XYWPOPUP,  0 },
    { "select-bug",            XYWSELECT, 0 }
};
int nwin95 = (sizeof(win95tab) / sizeof(struct keytab));
#endif /* NT */

#ifdef OS2MOUSE
extern int wideresult;
int tt_mouse = 1;                       /* Terminal mouse on/off */

struct keytab mousetab[] = {            /* Mouse items */
    { "activate", XYM_ON,     0 },
    { "button",   XYM_BUTTON, 0 },
    { "clear",    XYM_CLEAR,  0 },
    { "debug",    XYM_DEBUG,  0 }
};
int nmtab = (sizeof(mousetab)/sizeof(struct keytab));

struct keytab mousebuttontab[] = {      /* event button */
    { "1",             XYM_B1, 0 },
    { "2",             XYM_B2, 0 },
    { "3",             XYM_B3, 0 },
    { "one",           XYM_B1, CM_INV },
    { "three",         XYM_B3, CM_INV },
    { "two",           XYM_B2, CM_INV }
};
int nmbtab = (sizeof(mousebuttontab) / sizeof(struct keytab));

struct keytab mousemodtab[] = {         /* event button key modifier */
    { "alt",              XYM_ALT,   0 },
    { "alt-shift",        XYM_SHIFT|XYM_ALT, 0 },
    { "ctrl",             XYM_CTRL,  0 },
    { "ctrl-alt",         XYM_CTRL|XYM_ALT, 0 },
    { "ctrl-alt-shift",   XYM_CTRL|XYM_SHIFT|XYM_ALT, 0 },
    { "ctrl-shift",       XYM_CTRL|XYM_SHIFT, 0 },
    { "none",             0, 0 },
    { "shift",            XYM_SHIFT, 0 }
};
int nmmtab = (sizeof(mousemodtab) / sizeof(struct keytab));

struct keytab mclicktab[] = {           /* event button click modifier */
    { "click",        XYM_C1,   0 },
    { "drag",         XYM_DRAG, 0 },
    { "double-click", XYM_C2,   0 }
};
int nmctab = (sizeof(mclicktab) / sizeof(struct keytab));

#ifndef NOKVERBS
extern int nkverbs;
extern struct keytab kverbs[];
#endif /* NOKVERBS */
#endif /* OS2MOUSE */

/* #ifdef VMS */
struct keytab fbtab[] = {               /* Binary record types for VMS */
    { "fixed",     XYFT_B, 0 },         /* Fixed is normal for binary */
    { "undefined", XYFT_U, 0 }          /* Undefined if they ask for it */
};
int nfbtyp = (sizeof(fbtab) / sizeof(struct keytab));
/* #endif */

#ifdef VMS
struct keytab lbltab[] = {              /* Labeled File info */
    { "acl",         LBL_ACL, 0 },
    { "backup-date", LBL_BCK, 0 },
    { "name",        LBL_NAM, 0 },
    { "owner",       LBL_OWN, 0 },
    { "path",        LBL_PTH, 0 }
};
int nlblp = (sizeof(lbltab) / sizeof(struct keytab));
#else
#ifdef OS2
struct keytab lbltab[] = {              /* Labeled File info */
    { "archive",   LBL_ARC, 0 },
    { "extended",  LBL_EXT, 0 },
    { "hidden",    LBL_HID, 0 },
    { "read-only", LBL_RO,  0 },
    { "system",    LBL_SYS, 0 }
};
int nlblp = (sizeof(lbltab) / sizeof(struct keytab));
#endif /* OS2 */
#endif /* VMS */

#ifdef CK_CURSES
#ifdef CK_PCT_BAR
static struct keytab fdftab[] = {       /* SET FILE DISPLAY FULL options */
    { "thermometer",    1, 0, },
    { "no-thermometer", 0, 0  }
};
extern int thermometer;
#endif /* CK_PCT_BAR */
#endif /* CK_CURSES */

static struct keytab fdtab[] = {        /* SET FILE DISPLAY options */
#ifdef MAC                              /* Macintosh */
    { "fullscreen", XYFD_R,      0 },   /* Full-screen but not curses */
    { "none",       XYFD_N,      0 },
    { "off",        XYFD_N, CM_INV },
    { "on",         XYFD_R, CM_INV },
    { "quiet",      XYFD_N, CM_INV },
#else                                   /* Not Mac */
    { "brief", XYFD_B, 0 },             /* Brief */
    { "crt", XYFD_S, 0 },               /* CRT display */
#ifdef CK_CURSES
#ifdef COMMENT
    { "curses",     XYFD_C,  CM_INV },  /* Full-screen, curses */
#endif /* COMMENT */
    { "fullscreen", XYFD_C,  0 },       /* Full-screen, whatever the method */
#endif /* CK_CURSES */
#ifdef KUI
    { "gui",    XYFD_G, 0 },            /* GUI */
#endif /* KUI */        
    { "none",   XYFD_N, 0      },       /* No display */
    { "off",    XYFD_N, CM_INV },       /* Ditto */
    { "on",     XYFD_R, CM_INV },       /* On = Serial */
    { "quiet",  XYFD_N, CM_INV },       /* No display */
    { "serial", XYFD_R, 0      },       /* Serial */
#endif /* MAC */
    { "", 0, 0 }
};
int nfdtab = (sizeof(fdtab) / sizeof(struct keytab)) - 1;

struct keytab rsrtab[] = {              /* For REMOTE SET RECEIVE */
    { "packet-length", XYLEN,  0 },
    { "timeout",       XYTIMO, 0 }
};
int nrsrtab = (sizeof(rsrtab) / sizeof(struct keytab));

/* Send/Receive Parameters */

struct keytab srtab[] = {
    { "backup", XYBUP, 0 },
#ifndef NOCSETS
    { "character-set-selection", XYCSET, 0 },
#endif /* NOCSETS */
    { "control-prefix", XYQCTL, 0 },
#ifdef CKXXCHAR
    { "double-character", XYDBL, 0 },
#endif /* CKXXCHAR */
    { "end-of-packet", XYEOL, 0 },
#ifdef PIPESEND
    { "filter", XYFLTR, 0 },
#endif /* PIPESEND */
#ifdef CKXXCHAR
    { "ignore-character", XYIGN, 0 },
#endif /* CKXXCHAR */
    { "i-packets", 993, 0 },
    { "move-to", XYMOVE, 0 },
    { "negotiation-string-max-length", XYINIL, CM_INV },
    { "packet-length", XYLEN, 0 },
    { "pad-character", XYPADC, 0 },
    { "padding", XYNPAD, 0 },
    { "pathnames", XYFPATH, 0 },
    { "pause", XYPAUS, 0 },
#ifdef CK_PERMS
    { "permissions", 994, 0},		/* 206 */
#endif /* CK_PERMS */
    { "quote", XYQCTL, CM_INV },        /* = CONTROL-PREFIX */
    { "rename-to", XYRENAME, 0 },
    { "start-of-packet", XYMARK, 0 },
    { "timeout", XYTIMO, 0 },
#ifdef VMS
    { "version-numbers", 887, 0 },      /* VMS version numbers */
#endif /* VMS */
    { "", 0, 0 }
};
int nsrtab = (sizeof(srtab) / sizeof(struct keytab)) - 1;

#ifdef UNICODE
#define UCS_BOM 1
#define UCS_BYT 2
static struct keytab ucstab[] = {
    { "bom",        UCS_BOM, 0 },
    { "byte-order", UCS_BYT, 0 },
    { "", 0, 0 }
};
int nucstab = (sizeof(ucstab) / sizeof(struct keytab)) - 1;

static struct keytab botab[] = {
    { "big-endian",    0, 0 },
    { "little-endian", 1, 0 }
};
static int nbotab = 2;
#endif /* UNICODE */

/* REMOTE SET */

struct keytab rmstab[] = {
    { "attributes",  XYATTR, 0      },
    { "block-check", XYCHKT, 0      },
    { "file",        XYFILE, 0      },
    { "incomplete",  XYIFD,  CM_INV },  /* = REMOTE SET FILE INCOMPLETE */
    { "match",       XYMATCH,0      },
    { "receive",     XYRECV, 0      },
    { "retry",       XYRETR, 0      },
    { "server",      XYSERV, 0      },
    { "transfer",    XYXFER, 0      },
    { "window",      XYWIND, 0      },
    { "xfer",        XYXFER, CM_INV }
};
int nrms = (sizeof(rmstab) / sizeof(struct keytab));

struct keytab attrtab[] = {
#ifdef STRATUS
    { "account",       AT_ACCT, 0 },
#endif /* STRATUS */
    { "all",           AT_XALL, 0 },
#ifdef COMMENT
    { "blocksize",     AT_BLKS, 0 },    /* (not used) */
#endif /* COMMENT */
#ifndef NOCSETS
    { "character-set", AT_ENCO, 0 },
#endif /* NOCSETS */
#ifdef STRATUS
    { "creator",       AT_CREA, 0 },
#endif /* STRATUS */
    { "date",          AT_DATE, 0 },
    { "disposition",   AT_DISP, 0 },
    { "encoding",      AT_ENCO, CM_INV },
    { "format",        AT_RECF, CM_INV },
    { "length",        AT_LENK, 0 },
    { "off",           AT_ALLN, 0 },
    { "on",            AT_ALLY, 0 },
#ifdef COMMENT
    { "os-specific",   AT_SYSP, 0 },    /* (not used by UNIX or VMS) */
#endif /* COMMENT */
#ifdef CK_PERMS
    { "protection",    AT_LPRO, 0 },
    { "permissions",   AT_LPRO, CM_INV },
#endif /* CK_PERMS */
    { "record-format", AT_RECF, 0 },
    { "system-id",     AT_SYSI, 0 },
    { "type",          AT_FTYP, 0 }
};
int natr = (sizeof(attrtab) / sizeof(struct keytab)); /* how many attributes */

#ifdef CKTIDLE
struct keytab idlacts[] = {
    { "exit",       IDLE_EXIT, 0 },
    { "hangup",     IDLE_HANG, 0 },
    { "output",     IDLE_OUT,  0 },
    { "return",     IDLE_RET,  0 },
#ifdef TNCODE
    { "telnet-nop", IDLE_TNOP, 0 },
    { "telnet-ayt", IDLE_TAYT, 0 },
#endif /* TNCODE */
    { "", 0, 0 }
};
int nidlacts = (sizeof(idlacts) / sizeof(struct keytab)) - 1;
#endif /* CKTIDLE */

#ifndef NOSPL
extern int indef, inecho, insilence, inbufsize, inautodl, inintr;
#ifdef CKFLOAT
extern CKFLOAT inscale;
#endif	/* CKFLOAT */
extern char * inpbuf, * inpbp;
#ifdef OS2
extern int interm;
#endif /* OS2 */
struct keytab inptab[] = {              /* SET INPUT parameters */
#ifdef CK_AUTODL
    { "autodownload",    IN_ADL, 0 },
#endif /* CK_AUTODL */
    { "buffer-length",   IN_BUF, 0 },
    { "cancellation",    IN_CAN, 0 },
    { "case",            IN_CAS, 0 },
    { "default-timeout", IN_DEF, CM_INV }, /* There is no default timeout */
    { "echo",            IN_ECH, 0 },
#ifdef OS2
    { "pacing",          IN_PAC, CM_INV },
#endif /* OS2 */
    { "scale-factor",    IN_SCA, 0 },
    { "silence",         IN_SIL, 0 },
#ifdef OS2
    { "terminal",        IN_TRM, 0 },
#endif /* OS2 */
    { "timeout-action",  IN_TIM, 0 }
};
int ninp = (sizeof(inptab) / sizeof(struct keytab));

struct keytab intimt[] = {              /* SET INPUT TIMEOUT parameters */
    { "proceed", 0, 0 },                /* 0 = proceed */
    { "quit",    1, 0 }                 /* 1 = quit */
};

struct keytab incast[] = {              /* SET INPUT CASE parameters */
    { "ignore",  0, 0 },                /* 0 = ignore */
    { "observe", 1, 0 }                 /* 1 = observe */
};
#endif /* NOSPL */

struct keytab nabltab[] = {             /* For any command that needs */
    { "disabled", 0, 0 },
    { "enabled",  1, 0 },
    { "off",      0, CM_INV },          /* these keywords... */
    { "on",       1, CM_INV }
};
int nnabltab = sizeof(nabltab) / sizeof(struct keytab);

#ifdef OS2
struct keytab tvctab[] = {              /* SET TERM VIDEO-CHANGE */
    { "disabled",     TVC_DIS, 0 },
    { "enabled",      TVC_ENA, 0 },
#ifdef NT
    { "win95-safe",   TVC_W95, 0 },
#endif /* NT */
    { "", 0, 0 }
};
int ntvctab = (sizeof(tvctab) / sizeof(struct keytab)) - 1;

struct keytab msktab[] = { /* SET MS-DOS KERMIT compatibilities */
#ifdef COMMENT
    { "color",    MSK_COLOR,  0 },
#endif /* COMMENT */
    { "file-renaming", MSK_REN, 0 },
    { "keycodes", MSK_KEYS,   0 }
};
int nmsk = (sizeof(msktab) / sizeof(struct keytab));

struct keytab scrnupd[] = {             /* SET TERMINAL SCREEN-UPDATE */
    { "fast",   TTU_FAST,   0 },
    { "smooth", TTU_SMOOTH, 0 }
};
int nscrnupd = (sizeof(scrnupd) / sizeof(struct keytab));

#ifdef PCFONTS
/* This definition of the term_font[] table is only for     */
/* the OS/2 Full Screen Session and is not used on Windows */
struct keytab term_font[] = {           /* SET TERMINAL FONT */
#ifdef COMMENT
    { "cp111", TTF_111, 0 },
    { "cp112", TTF_112, 0 },
    { "cp113", TTF_113, 0 },
#endif /* COMMENT */
    { "cp437", TTF_437, 0 },
    { "cp850", TTF_850, 0 },
#ifdef COMMENT
    { "cp851", TTF_851, 0 },
#endif /* COMMENT */
    { "cp852", TTF_852, 0 },
#ifdef COMMENT
    { "cp853", TTF_853, 0 },
    { "cp860", TTF_860, 0 },
    { "cp861", TTF_861, 0 },
#endif /* COMMENT */
    { "cp862", TTF_862, 0 },
#ifdef COMMENT
    { "cp863", TTF_863, 0 },
    { "cp864", TTF_864, 0 },
    { "cp865", TTF_865, 0 },
#endif /* COMMENT */
    { "cp866", TTF_866, 0 },
#ifdef COMMENT
    { "cp880", TTF_880, 0 },
    { "cp881", TTF_881, 0 },
    { "cp882", TTF_882, 0 },
    { "cp883", TTF_883, 0 },
    { "cp884", TTF_884, 0 },
    { "cp885", TTF_885, 0 },
#endif /* COMMENT */
    { "default",TTF_ROM,0 }
};
int ntermfont = (sizeof(term_font) / sizeof(struct keytab));
int tt_font = TTF_ROM;                  /* Terminal screen font */
#else /* PCFONTS */
#ifdef NT
#ifdef KUI
struct keytab * term_font = NULL;
struct keytab * _term_font = NULL;
char * tt_facename = NULL;
int ntermfont = 0;
int tt_font = 0;
int tt_font_size = 0;
#endif /* KUI */
#endif /* NT */
#endif /* PCFONTS */

struct keytab anbktab[] = {             /* For any command that needs */
    { "message", 2, 0 },                /* these keywords... */
    { "off",     0, 0 },
    { "on",      1, 0 },
    { "unsafe-messag0", 99, CM_INV },
    { "unsafe-message", 3,  CM_INV }
};
int nansbk = (sizeof(anbktab) / sizeof(struct keytab));

int win95_popup = 1;
#ifdef NT
#ifdef KUI
int win95lucida = 0;
int win95hsl = 1;
#else /* KUI */
int win95lucida = 1;
int win95hsl = 1;
#endif /* KUI */
#else /* NT */
int win95lucida = 0;
int win95hsl = 1;
#endif /* NT */
#ifdef NT
int win95altgr  = 0;
extern int win95selectbug;
extern int win95_8_3;

#ifdef COMMENT
extern CHAR (*xlr[MAXTCSETS+1][MAXFCSETS+1])(CHAR);
extern struct keytab tcstab[];
extern int ntcs;
#endif /* COMMENT */
extern int maxow, maxow_usr; owwait;    /* Overlapped I/O variables */
#endif /* NT */
#endif /* OS2 */


/* The following routines broken out of doprm() to give compilers a break. */

/*  S E T O N  --  Parse on/off (default on), set parameter to result  */

int
seton(prm) int *prm; {
    int x, y;
    if ((y = cmkey(onoff,2,"","on",xxstring)) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    *prm = y;
    return(1);
}

/*  S E T O N A U T O --  Parse on/off/auto (default auto) & set result */

struct keytab onoffaut[] = {
    { "auto", SET_AUTO, 0 },            /* 2 */
    { "off",  SET_OFF,  0 },            /* 0 */
    { "on",   SET_ON,   0 }             /* 1 */
};

int
setonaut(prm) int *prm; {
    int x, y;
    if ((y = cmkey(onoffaut,3,"","auto",xxstring)) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    *prm = y;
    return(1);
}

/*  S E T N U M  --  Set parameter to result of cmnum() parse.  */
/*
 Call with pointer to integer variable to be set,
   x = number from cnum parse, y = return code from cmnum,
   max = maximum value to accept, -1 if no maximum.
 Returns -9 on failure, after printing a message, or 1 on success.
*/
int
setnum(prm,x,y,max) int x, y, *prm, max; {
    debug(F101,"setnum","",y);
    if (y == -3) {
        printf("\n?Value required\n");
        return(-9);
    }
    if (y == -2) {
        printf("%s?Not a number: %s\n",cmflgs == 1 ? "" : "\n", atxbuf);
        return(-9);
    }
    if (y < 0) return(y);
    if (max > -1 && x > max) {
        printf("?Sorry, %d is the maximum\n",max);
        return(-9);
    }
    if ((y = cmcfm()) < 0) return(y);
    *prm = x;
    return(1);
}

/*  S E T C C  --  Set parameter var to an ASCII control character value.  */
/*
  Parses a number, or a literal control character, or a caret (^) followed
  by an ASCII character whose value is 63-95 or 97-122, then gets confirmation,
  then sets the parameter to the code value of the character given.  If there
  are any parse errors, they are returned, otherwise on success 1 is returned.
*/
int
setcc(dflt,var) char *dflt; int *var; {
    int x, y;
    unsigned int c;
    char *hlpmsg = "Control character,\n\
 numeric ASCII value,\n\
 or in ^X notation,\n\
 or preceded by a backslash and entered literally";

    /* This is a hack to turn off complaints from expression evaluator. */
    x_ifnum = 1;
    y = cmnum(hlpmsg, dflt, 10, &x, xxstring); /* Parse a number */
    x_ifnum = 0;                               /* Allow complaints again */
    if (y < 0) {                        /* Parse failed */
        if (y != -2)                    /* Reparse needed or somesuch */
          return(y);                    /* Pass failure back up the chain */
    }
    /* Real control character or literal 8-bit character... */

    for (c = strlen(atmbuf) - 1; c > 0; c--) /* Trim */
      if (atmbuf[c] == SP) atmbuf[c] = NUL;

    if (y < 0) {                        /* It was not a number */
        if (((c = atmbuf[0])) && !atmbuf[1]) { /* Literal character? */
            c &= 0xff;
            if (((c > 31) && (c < 127)) || (c > 255)) {
                printf("\n?%d: Out of range - must be 0-31 or 127-255\n",c);
                return(-9);
            } else {
                if ((y = cmcfm()) < 0)  /* Confirm */
                  return(y);
                *var = c;               /* Set the variable */
                return(1);
            }
        } else if (atmbuf[0] == '^' && !atmbuf[2]) { /* Or ^X notation? */
            c = atmbuf[1];
            if (islower((char) c))      /* Uppercase lowercase letters */
              c = toupper(c);
            if (c > 62 && c < 96) {     /* Check range */
                if ((y = cmcfm()) < 0)
                  return(y);
                *var = ctl(c);          /* OK */
                return(1);
            } else {
                printf("?Not a control character - %s\n", atmbuf);
                return(-9);
            }
        } else {                        /* Something illegal was typed */
            printf("?Invalid - %s\n", atmbuf);
            return(-9);
        }
    }
    if (((x > 31) && (x < 127)) || (x > 255)) { /* They typed a number */
        printf("\n?%d: Out of range - must be 0-31 or 127-255\n",x);
        return(-9);
    }
    if ((y = cmcfm()) < 0)              /* In range, confirm */
      return(y);
    *var = x;                           /* Set variable */
    return(1);
}

#ifndef NOSPL                           /* The SORT command... */

static struct keytab srtswtab[] = {     /* SORT command switches */
    { "/case",    SRT_CAS, CM_ARG },
    { "/key",     SRT_KEY, CM_ARG },
    { "/numeric", SRT_NUM, 0 },
    { "/range",   SRT_RNG, CM_ARG },
    { "/reverse", SRT_REV, 0 }
};
static int nsrtswtab = sizeof(srtswtab)/sizeof(struct keytab);

extern char **a_ptr[];                  /* Array pointers */
extern int a_dim[];                     /* Array dimensions */

int
dosort() {                              /* Do the SORT command */
    char c, *p = NULL, ** ap, ** xp = NULL;
    struct FDB sw, fl, cm;
    int hi, lo;
    int xn = 0, xr = -1, xk = -1, xc = -1, xs = 0;
    int getval = 0, range[2], confirmed = 0;

    cmfdbi(&sw,                         /* First FDB - command switches */
           _CMKEY,                      /* fcode */
           "Array name or switch",
           "",                          /* default */
           "",                          /* addtl string data */
           nsrtswtab,                   /* addtl numeric data 1: tbl size */
           4,                           /* addtl numeric data 2: 4 = cmswi */
           NULL,                        /* Processing function */
           srtswtab,                    /* Keyword table */
           &fl                          /* Pointer to next FDB */
           );
    cmfdbi(&fl,                         /* Anything that doesn't match */
           _CMFLD,                      /* fcode */
           "Array name",                /* hlpmsg */
           "",                          /* default */
           "",                          /* addtl string data */
           0,                           /* addtl numeric data 1 */
           0,                           /* addtl numeric data 2 */
           NULL,
           NULL,
           &cm
           );
    cmfdbi(&cm,                         /* Or premature confirmation */
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

    range[0] = -1;
    range[1] = -1;

    while (1) {                         /* Parse 0 or more switches */
        x = cmfdb(&sw);
        if (x < 0)
          return(x);
        if (cmresult.fcode != _CMKEY)   /* Break out if not a switch */
          break;
        c = cmgbrk();
        getval = (c == ':' || c == '=');
        if (getval && !(cmresult.kflags & CM_ARG)) {
            printf("?This switch does not take arguments\n");
            return(-9);
        }
        switch (cmresult.nresult) {
          case SRT_REV:
            xr = 1;
            break;
          case SRT_KEY:
            if (getval) {
                if ((y = cmnum("Column for comparison (1-based)",
                               "1",10,&x,xxstring)) < 0)
                  return(y);
                xk = x - 1;
            } else
              xk = 0;
            break;
          case SRT_CAS:
            if (getval) {
                if ((y = cmkey(onoff,2,"","on",xxstring)) < 0)
                  return(y);
                xc = y;
            } else
              xc = 1;
            break;
          case SRT_RNG:                 /* /RANGE */
            if (getval) {
                char buf[32];
                char buf2[16];
                int i;
                char * p, * q;
                if ((y = cmfld("low:high element","1",&s,NULL)) < 0)
                  return(y);
                s = brstrip(s);
                ckstrncpy(buf,s,32);
                p = buf;
                for (i = 0; *p && i < 2; i++) { /* Get low and high */
                    q = p;              /* Start of this piece */
                    while (*p) {        /* Find end of this piece */
                        if (*p == ':') {
                            *p = NUL;
                            p++;
                            break;
                        }
                        p++;
                    }
                    y = 15;             /* Evaluate this piece */
                    s = buf2;
                    zzstring(q,&s,&y);
                    s = evalx(buf2);
                    if (s) if (*s) ckstrncpy(buf2,s,16);
                    if (!rdigits(buf2)) {
                        printf("?Not numeric: %s\n",buf2);
                        return(-9);
                    }
                    range[i] = atoi(buf2);
                }
            }
            break;
          case SRT_NUM:                 /* /NUMERIC */
            xn = 1;
            break;
          default:
            return(-2);
        }
    }
    switch (cmresult.fcode) {
      case _CMCFM:
        confirmed = 1;
        break;
      case _CMFLD:
        ckstrncpy(line,cmresult.sresult,LINBUFSIZ); /* Safe copy of name */
        s = line;
        break;
      default:
        printf("?Unexpected function code: %d\n",cmresult.fcode);
        return(-9);
    }
    if (confirmed) {
        printf("?Array name required\n");
        return(-9);
    }
    ckmakmsg(tmpbuf,TMPBUFSIZ,
             "Second array to sort according to ",s,NULL,NULL);
    if ((x = cmfld(tmpbuf,"",&p,NULL)) < 0)
      if (x != -3)
        return(x);
    tmpbuf[0] = NUL;
    ckstrncpy(tmpbuf,p,TMPBUFSIZ);
    p = tmpbuf;
    if ((x = cmcfm()) < 0)              /* Get confirmation */
      return(x);

    x = arraybounds(s,&lo,&hi);         /* Get array index & bounds */
    if (x < 0) {                        /* Check */
        printf("?Bad array name: %s\n",s);
        return(-9);
    }
    if (lo > -1) range[0] = lo;         /* Set range */
    if (hi > -1) range[1] = hi;
    ap = a_ptr[x];                      /* Get pointer to array element list */
    if (!ap) {                          /* Check */
        printf("?Array not declared: %s\n", s);
        return(-9);
    }
    if (range[0] < 0)                   /* Starting element */
      range[0] = 1;
    if (range[1] < 0)                   /* Final element */
      range[1] = a_dim[x];
    if (range[1] > a_dim[x]) {
        printf("?range %d:%d exceeds array dimension %d\n",
               range[0],range[1],a_dim[x]
               );
        return(-9);
    }
    ap += range[0];
    xs = range[1] - range[0] + 1;       /* Number of elements to sort */
    if (xs < 1) {                       /* Check */
        printf("?Bad range: %d:%d\n",range[0],range[1]);
        return(-9);
    }
    if (xk < 0) xk = 0;                 /* Key position */
    if (xr < 0) xr = 0;                 /* Reverse flag */
    if (xn)                             /* Numeric flag */
      xc = 2;
    else if (xc < 0)                    /* Not numeric */
      xc = inpcas[cmdlvl];              /* so alpha case option */

    if (*p) {                           /* Parallel array given? */
        y = xarray(p);                  /* Yes, get its index. */
        if (y < 0) {
            printf("?Bad array name: %s\n", p);
            return(-9);
        }
        if (y != x) {                   /* If the 2 arrays are different  */
            xp = a_ptr[y];              /* Pointer to 2nd array element list */
            if (!xp) {
                printf("?Array not declared: %s\n", p);
                return(-9);
            }
            if (a_dim[y] < range[1]) {
                printf("?Array %s smaller than %s\n", p, s);
                return(-9);
            }
            xp += range[0];             /* Set base to same as 1st array */
        }
    }
    sh_sort(ap,xp,xs,xk,xr,xc);         /* Sort the array(s) */
    return(success = 1);                /* Always succeeds */
}
#endif /* NOSPL */

static struct keytab purgtab[] = {      /* PURGE command switches */
    { "/after",        PU_AFT,  CM_ARG },
    { "/ask",          PU_ASK,  0 },
    { "/before",       PU_BEF,  CM_ARG },
    { "/delete",       PU_DELE, CM_INV },
#ifdef UNIXOROSK
    { "/dotfiles",     PU_DOT,  0 },
#endif /* UNIXOROSK */
    { "/except",       PU_EXC,  CM_ARG },
    { "/heading",      PU_HDG,  0 },
    { "/keep",         PU_KEEP, CM_ARG },
    { "/larger-than",  PU_LAR,  CM_ARG },
    { "/list",         PU_LIST, 0 },
    { "/log",          PU_LIST, CM_INV },
    { "/noask",        PU_NASK, 0 },
    { "/nodelete",     PU_NODE, CM_INV },
#ifdef UNIXOROSK
    { "/nodotfiles",   PU_NODOT,0 },
#endif /* UNIXOROSK */
    { "/noheading",    PU_NOH,  0 },
    { "/nol",          PU_NOLI, CM_INV|CM_ABR },
    { "/nolist",       PU_NOLI, 0 },
    { "/nolog",        PU_NOLI, CM_INV },
#ifdef CK_TTGWSIZ
    { "/nopage",       PU_NOPA, 0 },
#endif /* CK_TTGWSIZ */
    { "/not-after",    PU_NAF,  CM_ARG },
    { "/not-before",   PU_NBF,  CM_ARG },
    { "/not-since",    PU_NAF,  CM_INV|CM_ARG },
#ifdef CK_TTGWSIZ
    { "/page",         PU_PAGE, 0 },
#endif /* CK_TTGWSIZ */
    { "/quiet",        PU_QUIE, CM_INV },
#ifdef RECURSIVE
    { "/recursive",    PU_RECU, 0 },
#endif /* RECURSIVE */
    { "/since",        PU_AFT,  CM_ARG|CM_INV },
    { "/simulate",     PU_NODE, 0 },
    { "/smaller-than", PU_SMA,  CM_ARG },
    { "/verbose",      PU_VERB, CM_INV }
};
static int npurgtab = sizeof(purgtab)/sizeof(struct keytab);





int
bkupnum(s,i) char * s; int *i; {
    int k = 0, pos = 0;
    char * p = NULL, *q;
    *i = pos;
    if (!s) s = "";
    if (!*s)
      return(-1);
    if ((k = strlen(s)) < 5)
      return(-1);

    if (s[k-1] != '~')
      return(-1);
    pos = k - 2;
    q = s + pos;
    while (q >= s && isdigit(*q)) {
        p = q--;
        pos--;
    }
    if (!p)
      return(-1);
    if (q < s+2)
      return(-1);
    if (*q != '~' || *(q-1) != '.')
      return(-1);
    pos--;
    *i = pos;
    debug(F111,"bkupnum",s+pos,pos);
    return(atoi(p));
}

#ifdef CKPURGE
/* Presently only for UNIX because we need direct access to the file array. */
/* Not needed for VMS anyway, because we don't make backup files there. */

#define MAXKEEP 32                      /* Biggest /KEEP: value */

static int
  pu_keep = 0, pu_list = 0, pu_dot = 0, pu_ask = 0, pu_hdg = 0;

#ifdef CK_TTGWSIZ
static int pu_page = -1;
#else
static int pu_page = 0;
#endif /* CK_TTGWSIZ */

#ifndef NOSHOW
VOID
showpurgopts() {                        /* SHOW PURGE command options */
    int x = 0;
    extern int optlines;
    prtopt(&optlines,"PURGE");
    if (pu_ask > -1) {
        x++;
        prtopt(&optlines, pu_ask ? "/ASK" : "/NOASK");
    }
#ifdef UNIXOROSK
    if (pu_dot > -1) {
        x++;
        prtopt(&optlines, pu_dot ? "/DOTFILES" : "/NODOTFILES");
    }
#endif /* UNIXOROSK */
    if (pu_keep > -1) {
        x++;
        ckmakmsg(tmpbuf,TMPBUFSIZ,"/KEEP:",ckitoa(pu_keep),NULL,NULL);
        prtopt(&optlines,tmpbuf);
    }
    if (pu_list > -1) {
        x++;
        prtopt(&optlines, pu_list ? "/LIST" : "/NOLIST");
    }
    if (pu_hdg > -1) {
        x++;
        prtopt(&optlines, pu_hdg ? "/HEADING" : "/NOHEADING");
    }
#ifdef CK_TTGWSIZ
    if (pu_page > -1) {
        x++;
        prtopt(&optlines, pu_page ? "/PAGE" : "/NOPAGE");
    }
#endif /* CK_TTGWSIZ */
    if (!x) prtopt(&optlines,"(no options set)");
    prtopt(&optlines,"");
}
#endif /* NOSHOW */

int
setpurgopts() {                         /* Set PURGE command options */
    int c, z, getval = 0;
    int
      x_keep  = -1, x_list = -1, x_page = -1,
      x_hdg   = -1, x_ask  = -1, x_dot  = -1;

    while (1) {
        if ((y = cmswi(purgtab,npurgtab,"Switch","",xxstring)) < 0) {
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
          case PU_KEEP:
            z = 1;
            if (c == ':' || c == '=')
              if ((y = cmnum("How many backup files to keep",
                             "1",10,&z,xxstring)) < 0)
                return(y);
            if (z < 0 || z > MAXKEEP) {
                printf("?Please specify a number between 0 and %d\n",
                       MAXKEEP
                       );
                return(-9);
            }
            x_keep = z;
            break;
          case PU_LIST:
          case PU_VERB:
            x_list = 1;
            break;
          case PU_QUIE:
          case PU_NOLI:
            x_list = 0;
            break;
#ifdef CK_TTGWSIZ
          case PU_PAGE:
            x_page = 1;
            break;
          case PU_NOPA:
            x_page = 0;
            break;
#endif /* CK_TTGWSIZ */
          case PU_HDG:
            x_hdg = 1;
            break;
          case PU_NOH:
            x_hdg = 0;
            break;
          case PU_ASK:
            x_ask = 1;
            break;
          case PU_NASK:
            x_ask = 0;
            break;
#ifdef UNIXOROSK
          case PU_DOT:
            x_dot = 1;
            break;
          case PU_NODOT:
            x_dot = 0;
            break;
#endif /* UNIXOROSK */
          default:
            printf("?This option can not be set\n");
            return(-9);
        }
    }
    if ((x = cmcfm()) < 0)              /* Get confirmation */
      return(x);
    if (x_keep > -1)                    /* Set PURGE defaults. */
      pu_keep = x_keep;
    if (x_list > -1)
      pu_list = x_list;
#ifdef CK_TTGWSIZ
    if (x_page > -1)
      pu_page = x_page;
#endif /* CK_TTGWSIZ */
    if (x_hdg > -1)
      pu_hdg = x_hdg;
    if (x_ask > -1)
      pu_ask = x_ask;
    if (x_dot > -1)
      pu_dot = x_dot;
    return(success = 1);
}

int
dopurge() {                             /* Do the PURGE command */
    extern char ** mtchs;
    extern int xaskmore, cmd_rows, recursive;
    int simulate = 0, asking = 0;
    int listing = 0, paging = -1, lines = 0, deleting = 1, errors = 0;
    struct FDB sw, sf, cm;
    int g, i, j, k, m = 0, n, x, y, z, done = 0, count = 0, flags = 0;
    int tokeep = 0, getval = 0, havename = 0, confirmed = 0;
    int xx[MAXKEEP+1];                  /* Array of numbers to keep */
    int min = -1;
    int x_hdg = 0, fs = 0, rc = 0;
    CK_OFF_T minsize = -1L, maxsize = -1L;
    char namebuf[CKMAXPATH+4];
    char basebuf[CKMAXPATH+4];
    char
      * pu_aft = NULL,
      * pu_bef = NULL,
      * pu_naf = NULL,
      * pu_nbf = NULL,
      * pu_exc = NULL;
    char * pxlist[8];                   /* Exception list */

    if (pu_keep > -1)                   /* Set PURGE defaults. */
      tokeep = pu_keep;
    if (pu_list > -1)
      listing = pu_list;
#ifdef CK_TTGWSIZ
    if (pu_page > -1)
      paging = pu_page;
#endif /* CK_TTGWSIZ */

    for (i = 0; i <= MAXKEEP; i++)      /* Clear this number buffer */
      xx[i] = 0;
    for (i = 0; i < 8; i++)             /* Initialize these... */
      pxlist[i] = NULL;

    g_matchdot = matchdot;              /* Save these... */

    cmfdbi(&sw,                         /* 1st FDB - PURGE switches */
           _CMKEY,                      /* fcode */
           "Filename or switch",        /* hlpmsg */
           "",                          /* default */
           "",                          /* addtl string data */
           npurgtab,                    /* addtl numeric data 1: tbl size */
           4,                           /* addtl numeric data 2: 4 = cmswi */
           xxstring,                    /* Processing function */
           purgtab,                     /* Keyword table */
           &sf                          /* Pointer to next FDB */
           );
    cmfdbi(&sf,                         /* 2nd FDB - filespec to purge */
           _CMIFI,                      /* fcode */
           "",
           "",                          /* default */
           "",                          /* addtl string data */
           0,                           /* addtl numeric data 1 */
           0,                           /* addtl numeric data 2 */
           xxstring,
           NULL,
           &cm
           );
    cmfdbi(&cm,                         /* Or premature confirmation */
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

    while (!havename && !confirmed) {
        x = cmfdb(&sw);                 /* Parse something */
        if (x < 0) {                    /* Error */
            rc = x;
            goto xpurge;
        } else if (cmresult.fcode == _CMKEY) {
            char c;
            c = cmgbrk();
            if ((getval = (c == ':' || c == '=')) && !(cmgkwflgs() & CM_ARG)) {
                printf("?This switch does not take an argument\n");
                rc = -9;
                goto xpurge;
            }
            if (!getval && (cmgkwflgs() & CM_ARG)) {
                printf("?This switch requires an argument\n");
                rc = -9;
                goto xpurge;
            }
            switch (k = cmresult.nresult) {
              case PU_KEEP:
                z = 1;
                if (c == ':' || c == '=') {
                    if ((y = cmnum("How many backup files to keep",
                                   "1",10,&z,xxstring)) < 0) {
                        rc = y;
                        goto xpurge;
                    }
                }
                if (z < 0 || z > MAXKEEP) {
                    printf("?Please specify a number between 0 and %d\n",
                           MAXKEEP
                           );
                    rc = -9;
                    goto xpurge;
                }
                tokeep = z;
                break;
              case PU_LIST:
                listing = 1;
                break;
              case PU_NOLI:
                listing = 0;
                break;
#ifdef CK_TTGWSIZ
              case PU_PAGE:
                paging = 1;
                break;
              case PU_NOPA:
                paging = 0;
                break;
#endif /* CK_TTGWSIZ */
              case PU_DELE:
                deleting = 1;
                break;
              case PU_NODE:
                deleting = 0;
                simulate = 1;
                listing = 1;
                break;
              case PU_ASK:
                asking = 1;
                break;
              case PU_NASK:
                asking = 0;
                break;
              case PU_AFT:
              case PU_BEF:
              case PU_NAF:
              case PU_NBF:
                if ((x = cmdate("File-time","",&s,0,xxstring)) < 0) {
                    if (x == -3) {
                        printf("?Date-time required\n");
                        rc = -9;
                    } else
                      rc = x;
                    goto xpurge;
                }
                fs++;
                switch (k) {
                  case PU_AFT: makestr(&pu_aft,s); break;
                  case PU_BEF: makestr(&pu_bef,s); break;
                  case PU_NAF: makestr(&pu_naf,s); break;
                  case PU_NBF: makestr(&pu_nbf,s); break;
                }
                break;
              case PU_SMA:
              case PU_LAR:
                if ((x = cmnum("File size in bytes","0",10,&y,xxstring)) < 0) {
                    rc = x;
                    goto xpurge;
                }
                fs++;
                switch (cmresult.nresult) {
                  case PU_SMA: minsize = y; break;
                  case PU_LAR: maxsize = y; break;
                }
                break;
              case PU_DOT:
                matchdot = 1;
                break;
              case PU_NODOT:
                matchdot = 0;
                break;
              case PU_EXC:
                if ((x = cmfld("Pattern","",&s,xxstring)) < 0) {
                    if (x == -3) {
                        printf("?Pattern required\n");
                        rc = -9;
                    } else
                      rc = x;
                    goto xpurge;
                }
                fs++;
                makestr(&pu_exc,s);
                break;
              case PU_HDG:
                x_hdg = 1;
                break;
#ifdef RECURSIVE
              case PU_RECU:             /* /RECURSIVE */
                recursive = 2;
                break;
#endif /* RECURSIVE */
              default:
                printf("?Not implemented yet - \"%s\"\n",atmbuf);
                rc = -9;
                goto xpurge;
            }
        } else if (cmresult.fcode == _CMIFI) {
            havename = 1;
        } else if (cmresult.fcode == _CMCFM) {
            confirmed = 1;
        } else {
            rc = -2;
            goto xpurge;
        }
    }
    if (havename) {
#ifdef CKREGEX
        ckmakmsg(line,LINBUFSIZ,cmresult.sresult,".~[1-9]*~",NULL,NULL);
#else
        ckmakmsg(line,LINBUFSIZ,cmresult.sresult,".~*~",NULL,NULL);
#endif /* CKREGEX */
    } else {
#ifdef CKREGEX
        ckstrncpy(line,"*.~[1-9]*~",LINBUFSIZ);
#else
        ckstrncpy(line,"*.~*~",LINBUFSIZ);
#endif /* CKREGEX */
    }
    if (!confirmed) {
        if ((x = cmcfm()) < 0) {
            rc = x;
            goto xpurge;
        }
    }
    /* Parse finished - now action */

#ifdef CK_LOGIN
    if (isguest) {
        printf("?File deletion by guests not permitted.\n");
        rc = -9;
        goto xpurge;
    }
#endif /* CK_LOGIN */

#ifdef CK_TTGWSIZ
    if (paging < 0)                     /* /[NO]PAGE not given */
      paging = xaskmore;                /* so use prevailing */
#endif /* CK_TTGWSIZ */

    lines = 0;
    if (x_hdg > 0) {
        printf("Purging %s, keeping %d...%s\n",
               s,
               tokeep,
               simulate ? " (SIMULATION)" : "");
        lines += 2;
    }
    flags = ZX_FILONLY;
    if (recursive) flags |= ZX_RECURSE;
    n = nzxpand(line,flags);            /* Get list of backup files */
    if (tokeep < 1) {                   /* Deleting all of them... */
        for (i = 0; i < n; i++) {
            if (fs) if (fileselect(mtchs[i],
                                   pu_aft,pu_bef,pu_naf,pu_nbf,
                                   minsize,maxsize,0,8,pxlist) < 1) {
                if (listing > 0) {
                    printf(" %s (SKIPPED)\n",mtchs[i]);
#ifdef CK_TTGWSIZ
                    if (paging)
                      if (++lines > cmd_rows - 3) {
                          if (!askmore()) goto xpurge; else lines = 0;
                      }
#endif /* CK_TTGWSIZ */
                }
                continue;
            }
            if (asking) {
                int x;
                ckmakmsg(tmpbuf,TMPBUFSIZ," Delete ",mtchs[i],"?",NULL);
                x = getyesno(tmpbuf,1);
                switch (x) {
                  case 0: continue;
                  case 1: break;
                  case 2: goto xpurge;
                }
            }
            x = deleting ? zdelet(mtchs[i]) : 0;
            if (x > -1) {
                if (listing)
                  printf(" %s (%s)\n", mtchs[i],deleting ? "OK" : "SELECTED");
                count++;
            } else {
                errors++;
                if (listing)
                  printf(" %s (FAILED)\n", mtchs[i]);
            }
#ifdef CK_TTGWSIZ
            if (listing && paging)
              if (++lines > cmd_rows - 3) {
                  if (!askmore()) goto xpurge; else lines = 0;
              }
#endif /* CK_TTGWSIZ */
        }
        goto xpurge;
    }
    if (n < tokeep) {                   /* Not deleting any */
        count = 0;
        if (listing)
          printf(" Matches = %d: Not enough to purge.\n",n);
        goto xpurge;
    }

    /* General case - delete some but not others */

    sh_sort(mtchs,NULL,n,0,0,filecase); /* Alphabetize the list (ESSENTIAL) */

    g = 0;                              /* Start of current group */
    for (i = 0; i < n; i++) {           /* Go thru sorted file list */
        x = znext(namebuf);             /* Get next file */
        if (x < 1 || !namebuf[0] || i == n - 1) /* No more? */
          done = 1;                     /* NOTE: 'done' must be 0 or 1 only */
        if (fs) if (fileselect(namebuf,
                               pu_aft,pu_bef,pu_naf,pu_nbf,
                               minsize,maxsize,0,8,pxlist) < 1) {
            if (listing > 0) {
                printf(" %s (SKIPPED)\n",namebuf);
                if (++lines > cmd_rows - 3)
                  if (!askmore()) goto xpurge; else lines = 0;
            }
            continue;
        }
        if (x > 0)
          if ((m = bkupnum(namebuf,&z)) < 0) /* This file's backup number. */
            continue;
        for (j = 0; j < tokeep; j++) {  /* Insert in list. */
            if (m > xx[j]) {
                for (k = tokeep - 1; k > j; k--)
                  xx[k] = xx[k-1];
                xx[j] = m;
                break;
            }
        }
        /* New group? */
        if (done || (i > 0 && ckstrcmp(namebuf,basebuf,z,1))) {
            if (i + done - g > tokeep) { /* Do we have enough to purge? */
                min = xx[tokeep-1];     /* Yes, lowest backup number to keep */
                debug(F111,"dopurge group",basebuf,min);
                for (j = g; j < i + done; j++) { /* Go through this group */
                    x = bkupnum(mtchs[j],&z);    /* Get file backup number */
                    if (x > 0 && x < min) {      /* Below minimum? */
                        x = deleting ? zdelet(mtchs[j]) : 0;
                        if (x < 0) errors++;
                        if (listing)
                          printf(" %s (%s)\n",
                                 mtchs[j],
                                 ((x < 0) ? "ERROR" :
                                  (deleting ? "DELETED" : "SELECTED"))
                                 );
                        count++;
                    } else if (listing) /* Not below minimum - keep this one */
                      printf(" %s (KEPT)\n",mtchs[j]);
#ifdef CK_TTGWSIZ
                    if (listing && paging)
                      if (++lines > cmd_rows - 3) {
                          if (!askmore()) goto xpurge; else lines = 0;
                      }
#endif /* CK_TTGWSIZ */
                }
            } else if (listing && paging) { /* Not enough to purge */
                printf(" %s.~*~ (KEPT)\n",basebuf);
#ifdef CK_TTGWSIZ
                if (++lines > cmd_rows - 3) {
                    if (!askmore()) goto xpurge; else lines = 0;
                }
#endif /* CK_TTGWSIZ */
            }
            for (j = 0; j < tokeep; j++) /* Clear the backup number list */
              xx[j] = 0;
            g = i;                      /* Reset the group pointer */
        }
        if (done)                       /* No more files, done. */
          break;
        strncpy(basebuf,namebuf,z);     /* Set basename of this file */
        basebuf[z] = NUL;
    }
  xpurge:                               /* Common exit point */
    if (g_matchdot > -1) {
        matchdot = g_matchdot;          /* Restore these... */
        g_matchdot = -1;
    }
    if (rc < 0) return(rc);             /* Parse error */
    if (x_hdg)
      printf("Files purged: %d%s\n",
             count,
             deleting ? "" : " (not really)"
             );
    return(success = count > 0 ? 1 : (errors > 0) ? 0 : 1);
}
#endif /* CKPURGE */

#ifndef NOXFER
#ifndef NOLOCAL
int
doxdis(which) int which; {		/* 1 = Kermit, 2 = FTP */
    extern int nolocal;
    int x, y = 0, z;
#ifdef NEWFTP
    extern int ftp_dis;
#endif /* NEWFTP */

#ifdef COMMENT
    char *s;
#endif /* COMMENT */

    if ((x = cmkey(fdtab,nfdtab,"file transfer display style","",
                   xxstring)) < 0)
      return(x);
#ifdef CK_PCT_BAR
    if ((y = cmkey(fdftab,2,"","thermometer",xxstring)) < 0)
      return(y);
#endif /* CK_PCT_BAR */
    if ((z = cmcfm()) < 0) return(z);
#ifdef CK_CURSES
    if (x == XYFD_C) {                  /* FULLSCREEN */
#ifdef COMMENT
#ifndef MYCURSES
        extern char * trmbuf;           /* Real curses */
        int z;
#endif /* MYCURSES */
#endif /* COMMENT */

        if (nolocal)                    /* Nothing to do in this case */
	  return(success = 1);

#ifdef COMMENT
#ifndef MYCURSES
#ifndef VMS
        s = getenv("TERM");
        debug(F110,"doxdis TERM",s,0);
        if (!s) s = "";
        fxdinit(x);
        if (*s && trmbuf) {             /* Don't call tgetent */
            z = tgetent(trmbuf,s);      /* if trmbuf not allocated */
            debug(F111,"doxdis tgetent",s,z);
        } else {
            z = 0;
            debug(F110,"doxdis tgetent skipped",s,0);
        }
        if (z < 1) {
            printf("Sorry, terminal type unknown: \"%s\"\n",s);
            return(success = 0);
        }
#endif /* VMS */
#endif /* MYCURSES */
#else
        fxdinit(x);
#endif /* COMMENT */

#ifdef CK_PCT_BAR
        thermometer = y;
#endif /* CK_PCT_BAR */

        line[0] = '\0';                 /* (What's this for?) */
    }
#endif /* CK_CURSES */
    if (which == 1)			/* It's OK. */
      fdispla = x;
#ifdef NEWFTP
    else if (which == 2)
      ftp_dis = x;
#endif /* NEWFTP */
    return(success = 1);
}
#endif /* NOLOCAL */
#endif /* NOXFER */

int
setfil(rmsflg) int rmsflg; {
#ifdef COMMENT
    extern int en_del;
#endif /* COMMENT */
#ifndef NOXFER
    if (rmsflg) {
        if ((y = cmkey(rfiltab,nrfilp,"Remote file parameter","",
                       xxstring)) < 0) {
            if (y == -3) {
                printf("?Remote file parameter required\n");
                return(-9);
            } else return(y);
        }
    } else {
#endif /* NOXFER */
        if ((y = cmkey(filtab,nfilp,"File parameter","",xxstring)) < 0)
          return(y);
#ifndef NOXFER
    }
#endif /* NOXFER */
    switch (y) {
#ifdef COMMENT                          /* Not needed */
      case XYFILB:                      /* Blocksize */
        if ((y = cmnum("file block size",ckitoa(DBLKSIZ),10,&z,xxstring)) < 0)
          return(y);
        if ((x = cmcfm()) < 0) return(x);
        if (rmsflg) {
            sstate = setgen('S', "311", ckitoa(z), "");
            return((int) sstate);
        } else {
            fblksiz = z;
            return(success = 1);
        }
#endif /* COMMENT */

#ifndef NOXFER
      case XYFILS:                      /* Byte size */
        if ((y = cmnum("file byte size (7 or 8)","8",10,&z,xxstring)) < 0)
          return(y);
        if (z != 7 && z != 8) {
            printf("\n?The choices are 7 and 8\n");
            return(0);
        }
        if ((y = cmcfm()) < 0) return(y);
        if (z == 7) fmask = 0177;
        else if (z == 8) fmask = 0377;
        return(success = 1);

#ifndef NOCSETS
      case XYFILC: {                    /* Character set */
          char * csetname = NULL;
          extern int
            r_cset, s_cset, afcset[];   /* SEND CHARACTER-SET AUTO or MANUAL */

          struct FDB kw, fl;
          cmfdbi(&kw,                   /* First FDB - command switches */
                 _CMKEY,                /* fcode */
                 rmsflg ? "server character-set name" : "",  /* help */
                 "",                    /* default */
                 "",                    /* addtl string data */
                 nfilc,                 /* addtl numeric data 1: tbl size */
                 0,                     /* addtl numeric data 2: 0 = keyword */
                 xxstring,              /* Processing function */
                 fcstab,                /* Keyword table */
                 rmsflg ? &fl : NULL    /* Pointer to next FDB */
           );
          cmfdbi(&fl,                   /* Anything that doesn't match */
                 _CMFLD,                /* fcode */
                 "",                    /* hlpmsg */
                 "",                    /* default */
                 "",                    /* addtl string data */
                 0,                     /* addtl numeric data 1 */
                 0,                     /* addtl numeric data 2 */
                 xxstring,
                 NULL,
                 NULL
                 );
          if ((x = cmfdb(&kw)) < 0)
            return(x);
          if (cmresult.fcode == _CMKEY) {
              x = cmresult.nresult;
              csetname = fcsinfo[x].keyword;
          } else {
              ckstrncpy(line,cmresult.sresult,LINBUFSIZ);
              csetname = line;
          }
          if ((z = cmcfm()) < 0) return(z);
          if (rmsflg) {
              sstate = setgen('S', "320", csetname, "");
              return((int) sstate);
          }
          fcharset = x;
          if (s_cset == XMODE_A)        /* If SEND CHARACTER-SET is AUTO */
            if (x > -1 && x <= MAXFCSETS)
              if (afcset[x] > -1 && afcset[x] <= MAXTCSETS)
                tcharset = afcset[x]; /* Pick corresponding xfer charset */
          setxlatype(tcharset,fcharset); /* Translation type */
          /* If I say SET FILE CHARACTER-SET blah, I want to be blah! */
          r_cset = XMODE_M;             /* Don't switch incoming set! */
          x = fcsinfo[fcharset].size;   /* Also set default x-bit charset */
          if (x == 128)                 /* 7-bit... */
            dcset7 = fcharset;
          else if (x == 256)            /* 8-bit... */
            dcset8 = fcharset;
          return(success = 1);
      }
#endif /* NOCSETS */

#ifndef NOLOCAL
      case XYFILD:                      /* Display */
        return(doxdis(1));		/* 1 == kermit */
#endif /* NOLOCAL */
#endif /* NOXFER */

      case XYFILA:                      /* End-of-line */
#ifdef NLCHAR
        s = "";
        if (NLCHAR == 015)
          s = "cr";
        else if (NLCHAR == 012)
          s = "lf";
        if ((x = cmkey(eoltab, neoltab,
                       "local text-file line terminator",s,xxstring)) < 0)
          return(x);
#else
        if ((x = cmkey(eoltab, neoltab,
                       "local text-file line terminator","crlf",xxstring)) < 0)
          return(x);
#endif /* NLCHAR */
        if ((z = cmcfm()) < 0) return(z);
        feol = (CHAR) x;
        return(success = 1);

#ifndef NOXFER
      case XYFILN:                      /* Names */
        if ((x = cmkey(fntab,nfntab,"how to handle filenames","converted",
                       xxstring)) < 0)
          return(x);
        if ((z = cmcfm()) < 0) return(z);
        if (rmsflg) {
            sstate = setgen('S', "301", ckitoa(1 - x), "");
            return((int) sstate);
        } else {
            ptab[protocol].fncn = x;    /* Set structure */
            fncnv = x;                  /* Set variable */
            f_save = x;                 /* And set "permanent" variable */
            return(success = 1);
        }

      case XYFILR:                      /* Record length */
        if ((y = cmnum("file record length",
                       ckitoa(DLRECL),10,&z,xxstring)) < 0)
          return(y);
        if ((x = cmcfm()) < 0) return(x);
        if (rmsflg) {
            sstate = setgen('S', "312", ckitoa(z), "");
            return((int) sstate);
        } else {
            frecl = z;
            return(success = 1);
        }

#ifdef COMMENT
      case XYFILO:                      /* Organization */
        if ((x = cmkey(forgtab,nforg,"file organization","sequential",
                       xxstring)) < 0)
          return(x);
        if ((y = cmcfm()) < 0) return(y);
        if (rmsflg) {
            sstate = setgen('S', "314", ckitoa(x), "");
            return((int) sstate);
        } else {
            forg = x;
            return(success = 1);
        }
#endif /* COMMENT */

#ifdef COMMENT                          /* Not needed */
      case XYFILF:                      /* Format */
        if ((x = cmkey(frectab,nfrec,"file record format","stream",
                       xxstring)) < 0)
          return(x);
        if ((y = cmcfm()) < 0) return(y);
        if (rmsflg) {
            sstate = setgen('S', "313", ckitoa(x), "");
            return((int) sstate);
        } else {
            frecfm = x;
            return(success = 1);
        }
#endif /* COMMENT */

#ifdef COMMENT
      case XYFILP:                      /* Printer carriage control */
        if ((x = cmkey(fcctab,nfcc,"file carriage control","newline",
                       xxstring)) < 0)
          return(x);
        if ((y = cmcfm()) < 0) return(y);
        if (rmsflg) {
            sstate = setgen('S', "315", ckitoa(x), "");
            return((int) sstate);
        } else {
            fcctrl = x;
            return(success = 1);
        }
#endif /* COMMENT */
#endif /* NOXFER */

      case XYFILT:                      /* Type */
        if ((x = cmkey(rmsflg ? rfttab  : fttab,
                       rmsflg ? nrfttyp : nfttyp,
                       "type of file transfer","text",xxstring)) < 0)
          return(x);

#ifdef VMS
        /* Allow VMS users to choose record format for binary files */
        if ((x == XYFT_B) && (rmsflg == 0)) {
            if ((x = cmkey(fbtab,nfbtyp,"VMS record format","fixed",
                           xxstring)) < 0)
              return(x);
        }
#endif /* VMS */
        if ((y = cmcfm()) < 0) return(y);
        binary = x;
        b_save = x;
#ifdef MAC
        (void) mac_setfildflg(binary);
#endif /* MAC */
#ifndef NOXFER
        if (rmsflg) {
            /* Allow for LABELED in VMS & OS/2 */
            sstate = setgen('S', "300", ckitoa(x), "");
            return((int) sstate);
        } else {
#endif /* NOXFER */
            return(success = 1);
#ifndef NOXFER
        }
#endif /* NOXFER */

#ifndef NOXFER
      case XYFILX:                      /* Collision Action */
        if ((x = cmkey(colxtab,ncolx,"Filename collision action","backup",
                       xxstring)) < 0)
          return(x);
        if ((y = cmcfm()) < 0) return(y);
#ifdef CK_LOGIN
        if (isguest) {
            /* Don't let guests change existing files */
            printf("?This command not valid for guests\n");
            return(-9);
        }
#endif /* CK_LOGIN */
#ifdef COMMENT
        /* Not appropriate - DISABLE DELETE only refers to server */
        if ((x == XYFX_X || x == XYFX_B || x == XYFX_U || x == XYFX_A) &&
            (!ENABLED(en_del))) {
            printf("?Sorry, file deletion is disabled.\n");
            return(-9);
        }
#endif /* COMMENT */
        fncact = x;
        ptab[protocol].fnca = x;
        if (rmsflg) {
            sstate = setgen('S', "302", ckitoa(fncact), "");
            return((int) sstate);
        } else {
            if (fncact == XYFX_R) ckwarn = 1; /* FILE WARNING implications */
            if (fncact == XYFX_X) ckwarn = 0; /* ... */
            return(success = 1);
        }

      case XYFILW:                      /* Warning/Write-Protect */
        if ((x = seton(&ckwarn)) < 0) return(x);
        if (ckwarn)
          fncact = XYFX_R;
        else
          fncact = XYFX_X;
        return(success = 1);

#ifdef CK_LABELED
      case XYFILL:                      /* LABELED FILE parameters */
        if ((x = cmkey(lbltab,nlblp,"Labeled file feature","",
                       xxstring)) < 0)
          return(x);
        if ((success = seton(&y)) < 0)
          return(success);
        if (y)                          /* Set or reset the selected bit */
          lf_opts |= x;                 /* in the options bitmask. */
        else
          lf_opts &= ~x;
        return(success);
#endif /* CK_LABELED */

      case XYFILI: {                    /* INCOMPLETE */
          extern struct keytab ifdatab[];
          extern int keep;
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
      }

#ifdef CK_TMPDIR
      case XYFILG: {                    /* Download directory */
          int x;
          char *s;
#ifdef ZFNQFP
          struct zfnfp * fnp;
#endif /* ZFNQFP */
#ifdef MAC
          char temp[34];
#endif /* MAC */

#ifdef GEMDOS
          if ((x = cmdir("Name of local directory, or carriage return",
                         "",&s,
                         NULL)) < 0 ) {
              if (x != -3)
                return(x);
          }
#else
#ifdef OS2
          if ((x = cmdir("Name of PC disk and/or directory,\n\
       or press the Enter key to use current directory",
                         "",&s,xxstring)) < 0 ) {
              if (x != -3)
                return(x);
          }
#else
#ifdef MAC
          x = ckstrncpy(temp,zhome(),32);
          if (x > 0) if (temp[x-1] != ':') { temp[x] = ':'; temp[x+1] = NUL; }
          if ((x = cmtxt("Name of Macintosh volume and/or folder,\n\
 or press the Return key for the desktop on the boot disk",
                         temp,&s, xxstring)) < 0 )
            return(x);
#else
          if ((x = cmdir("Name of local directory, or carriage return",
                         "", &s, xxstring)) < 0 ) {
              if (x != -3)
                return(x);
          }
#endif /* MAC */
#endif /* OS2 */
#endif /* GEMDOS */
          debug(F110,"download dir",s,0);

#ifndef MAC
          if (x == 2) {
              printf("?Wildcards not allowed in directory name\n");
              return(-9);
          }
#endif /* MAC */

#ifdef ZFNQFP
          if ((fnp = zfnqfp(s,TMPBUFSIZ - 1,tmpbuf))) {
              if (fnp->fpath)
                if ((int) strlen(fnp->fpath) > 0)
                  s = fnp->fpath;
          }
          debug(F110,"download zfnqfp",s,0);
#endif /* ZFNQFP */

          ckstrncpy(line,s,LINBUFSIZ);  /* Make a safe copy */
#ifndef MAC
          if ((x = cmcfm()) < 0)        /* Get confirmation */
            return(x);
#endif /* MAC */

#ifdef CK_LOGIN
        if (isguest) {
            /* Don't let guests change existing files */
            printf("?This command not valid for guests\n");
            return(-9);
        }
#endif /* CK_LOGIN */
          x = strlen(s);

          if (x) {
#ifdef datageneral			/* AOS/VS */
              if (s[x-1] == ':')        /* homdir ends in colon, */
                s[x-1] = NUL;           /* and "dir" doesn't like that... */
#else
#ifdef OS2ORUNIX			/* Unix or K-95... */
	      if ((x < (LINBUFSIZ - 2)) && /* Add trailing dirsep */
		  (s[x-1] != '/')) {	/* if none present.  */
		  s[x] = '/';		/* Note that Windows path has */
		  s[x+1] = NUL;		/* been canonicalized to forward */
	      }                		/* slashes at this point. */
#endif /* OS2ORUNIX */
#endif /* datageneral */
              makestr(&dldir,s);
          } else
            makestr(&dldir,NULL);       /* dldir is NULL when not assigned */

          return(success = 1);
      }
#endif /* CK_TMPDIR */
      case XYFILY:
        return(setdest());
#endif /* NOXFER */

#ifdef CK_CTRLZ
      case XYFILV: {                    /* EOF */
          extern int eofmethod;
          if ((x = cmkey(eoftab,3,"end-of-file detection method","",
                         xxstring)) < 0)
            return(x);
          if ((y = cmcfm()) < 0)
            return(y);
          eofmethod = x;
          return(success = 1);
      }
#endif /* CK_CTRLZ */

#ifndef NOXFER
#ifdef UNIX
      case XYFILH: {                    /* OUTPUT */
          extern int zofbuffer, zobufsize, zofblock;
#ifdef DYNAMIC
          extern char * zoutbuffer;
#endif /* DYNAMIC */

          if ((x = cmkey(zoftab,nzoftab,"output file writing method","",
                         xxstring)) < 0)
            return(x);
          if (x == ZOF_BUF || x == ZOF_NBUF) {
              if ((y = cmnum("output buffer size","32768",10,&z,xxstring)) < 0)
                return(y);
              if (z < 1) {
                  printf("?Bad size - %d\n", z);
                  return(-9);
              }
          }
          if ((y = cmcfm()) < 0) return(y);
          switch (x) {
            case ZOF_BUF:
            case ZOF_NBUF:
              zofbuffer = (x == ZOF_BUF);
              zobufsize = z;
              break;
            case ZOF_BLK:
            case ZOF_NBLK:
              zofblock = (x == ZOF_BLK);
              break;
          }
#ifdef DYNAMIC
          if (zoutbuffer) free(zoutbuffer);
          if (!(zoutbuffer = (char *)malloc(z))) {
              printf("MEMORY ALLOCATION ERROR - FATAL\n");
              doexit(BAD_EXIT,-1);
          } else
            zobufsize = z;
#else
          if (z <= OBUFSIZE) {
              zobufsize = z;
          } else {
              printf("?Sorry, %d is too big - %d is the maximum\n",z,OBUFSIZE);
              return(-9);
          }
#endif /* DYNAMIC */
          return(success = 1);
      }
#endif /* UNIX */

#ifdef PATTERNS
      case XYFIBP:                      /* BINARY-PATTERN */
      case XYFITP: {                    /* TEXT-PATTERN */
          char * tmp[FTPATTERNS];
          int i, n = 0;
          while (n < FTPATTERNS) {
              tmp[n] = NULL;
              if ((x = cmfld("Pattern","",&s,xxstring)) < 0)
                break;
              ckstrncpy(line,s,LINBUFSIZ);
              s = brstrip(line);
              makestr(&(tmp[n++]),s);
          }
          if (x == -3) x = cmcfm();
          for (i = 0; i <= n; i++) {
              if (x > -1) {
                  if (y == XYFIBP)
                    makestr(&(binpatterns[i]),tmp[i]);
                  else
                    makestr(&(txtpatterns[i]),tmp[i]);
              }
              free(tmp[i]);
          }
          if (y == XYFIBP)              /* Null-terminate the list */
            makestr(&(binpatterns[i]),NULL);
          else
            makestr(&(txtpatterns[i]),NULL);
          return(x);
      }

      case XYFIPA:                      /* PATTERNS */
        if ((x = setonaut(&patterns)) < 0)
          return(x);
        return(success = 1);
#endif /* PATTERNS */
#endif /* NOXFER */

#ifdef UNICODE
      case XYFILU: {                    /* UCS */
          extern int ucsorder, ucsbom, byteorder;
          if ((x = cmkey(ucstab,nucstab,"","",xxstring)) < 0)
            return(x);
          switch (x) {
            case UCS_BYT:
              if ((y = cmkey(botab,nbotab,
                             "Byte order",
                             byteorder ? "little-endian" : "big-endian",
                             xxstring
                             )
                   ) < 0)
                return(y);
              if ((x = cmcfm()) < 0)
                return(x);
              ucsorder = y;
              return(success = 1);
            case UCS_BOM:
              if ((y = cmkey(onoff,2,"","on",xxstring)) < 0)
                return(y);
              if ((x = cmcfm()) < 0)
                return(x);
              ucsbom = y;
              return(success = 1);
            default:
              return(-2);
          }
      }
#endif /* UNICODE */

#ifndef datageneral
      case XYF_INSP: {                  /* SCAN (INSPECTION) */
          extern int filepeek, nscanfile;
          if ((x = cmkey(onoff,2,"","on",xxstring)) < 0)
            return(x);
          if (y) {
              if ((y = cmnum("How much to scan",ckitoa(SCANFILEBUF),
                             10,&z,xxstring)) < 0)
                return(y);
          }
          if ((y = cmcfm()) < 0)
            return(y);
#ifdef VMS
          filepeek = 0;
          nscanfile = 0;
          return(success = 0);
#else
          filepeek = x;
          nscanfile = z;
          return(success = 1);
#endif /* VMS */
      }
#endif /* datageneral */

      case XYF_DFLT:
        y = 0;
#ifndef NOCSETS
        if ((y = cmkey(fdfltab,nfdflt,"","",xxstring)) < 0)
          return(y);
        if (y == 7 || y == 8) {
            if (y == 7)
              s = fcsinfo[dcset7].keyword;
            else
              s = fcsinfo[dcset8].keyword;
            if ((x = cmkey(fcstab,nfilc,"character-set",s,xxstring)) < 0)
              return(x);
        }
        ckstrncpy(line,fcsinfo[x].keyword,LINBUFSIZ);
        s = line;
#endif /* NOCSETS */
        if ((z = cmcfm()) < 0)
          return(z);
        switch (y) {
#ifndef NOCSETS
          case 7:
            if (fcsinfo[x].size != 128) {
                printf("%s - Not a 7-bit set\n",s);
                return(-9);
            }
            dcset7 = x;
            break;
          case 8:
            if (fcsinfo[x].size != 256) {
                printf("%s - Not an 8-bit set\n",s);
                return(-9);
            }
            dcset8 = x;
            break;
#endif /* NOCSETS */
          default:
            return(-2);
        }
        return(success = 1);

#ifndef NOXFER
      case 9997:                        /* FASTLOOKUPS */
        return(success = seton(&stathack));
#endif /* NOXFER */

#ifdef UNIX
#ifdef DYNAMIC
      case XYF_LSIZ: {                  /* LISTSIZE */
          int zz;
          y = cmnum("Maximum number of filenames","",10,&x,xxstring);
          if ((x = setnum(&zz,x,y,-1)) < 0)
            return(x);
          if (zsetfil(zz,3) < 0) {
              printf("?Memory allocation failure\n");
              return(-9);
          }
          return(success = 1);
      }
      case XYF_SSPA: {                  /* STRINGSPACE */
          int zz;
          y = cmnum("Number of characters for filename list",
                    "",10,&x,xxstring);
          if ((x = setnum(&zz,x,y,-1)) < 0)
            return(x);
          if (zsetfil(zz,1) < 0) {
              printf("?Memory allocation failure\n");
              return(-9);
          }
          return(success = 1);
      }

#endif /* DYNAMIC */
#endif /* UNIX */

      default:
        printf("?unexpected file parameter\n");
        return(-2);
    }
}

#ifdef UNIX
#ifndef NOPUTENV
#ifdef BIGBUFOK
#define NPUTENVS 4096
#else
#define NPUTENVS 128
#endif	/* BIGBUFOK */
/* environment variables must be static, not automatic */

static char * putenvs[NPUTENVS];	/* Array of environment var strings */
static int nputenvs = -1;		/* Pointer into array */
/*
  If anyone ever notices the limitation on the number of PUTENVs, the list
  can be made dynamic, we can recycle entries with the same name, etc.
*/
int
doputenv(s1, s2) char * s1; char * s2; {
    char * s, * t = tmpbuf;		/* Create or alter environment var */

    if (nputenvs == -1) {		/* Table not used yet */
	int i;				/* Initialize the pointers */
	for (i = 0; i < NPUTENVS; i++)
	  putenvs[i] = NULL;
	nputenvs = 0;
    }
    if (!s1) return(1);			/* Nothing to do */
    if (!*s1) return(1);		/* ditto */

    if (ckindex("=",s1,0,0,0)) {	/* Does the name contain an '='? */
	printf(				/* putenv() does not allow this. */
	 /* This also catches the 'putenv name=value' case */
         "?PUTENV - Equal sign in variable name - 'help putenv' for info.\n");
        return(-9);
    }
    nputenvs++;				/* Point to next free string */

    debug(F111,"doputenv s1",s1,nputenvs);
    debug(F111,"doputenv s2",s2,nputenvs);

    if (nputenvs > NPUTENVS - 1) {	/* Notice the end */
	printf("?PUTENV - static buffer space exhausted\n");
	return(-9);
    }
    /* Quotes are not needed but we allow them for familiarity */
    /* but then we strip them, so syntax is same as for Unix shell */

    if (s2) {
	s2 = brstrip(s2);
    } else {
	s2 = (char *)"";
    }
    ckmakmsg(t,TMPBUFSIZ,s1,"=",s2,NULL);
    debug(F111,"doputenv",t,nputenvs);
    (VOID) makestr(&(putenvs[nputenvs]),t); /* Make a safe permananent copy */
    if (!putenvs[nputenvs]) {
	printf("?PUTENV - memory allocation failure\n");
	return(-9);
    }
    if (putenv(putenvs[nputenvs])) {
	printf("?PUTENV - %s\n",ck_errstr());
	return(-9);
    } else return(success = 1);
}
#endif	/* NOPUTENV */
#endif	/* UNIX */

int
settrmtyp() {
#ifdef OS2
#ifdef TNCODE
    extern int ttnum;                    /* Last Telnet Terminal Type sent */
    extern int ttnumend;                 /* Has end of list been found */
#endif /* TNCODE */
    if ((x = cmkey(ttyptab,nttyp,"","vt220",xxstring)) < 0)
      return(x);
    if ((y = cmcfm()) < 0)
      return(y);
    settermtype(x,1);
#ifdef TNCODE
    /* So we send the correct terminal name to the host if it asks for it */
    ttnum = -1;                         /* Last Telnet Terminal Type sent */
    ttnumend = 0;                       /* end of list not found */
#endif /* TNCODE */
    return(success = 1);
#else  /* Not OS2 */
#ifdef UNIX
    extern int fxd_inited;
    x = cmtxt("Terminal type name, case sensitive","",&s,NULL);
#ifdef NOPUTENV
    success = 1;
#else
    success = doputenv("TERM",s);	/* Set the TERM variable */
#ifdef CK_CURSES
    fxd_inited = 0;	       /* Force reinitialization of curses database */
    (void)doxdis(0);		     /* Re-initialize file transfer display */
    concb((char)escape);		/* Fix command terminal */
#endif	/* CK_CURSES */
#endif	/* NOPUTENV */
    return(success);
#else
    printf(
"\n Sorry, this version of C-Kermit does not support the SET TERMINAL TYPE\n");
    printf(
" command.  Type \"help set terminal\" for further information.\n");
    return(success = 0);
#endif	/* UNIX */
#endif /* OS2 */
}

#ifndef NOLOCAL
#ifdef OS2
/* MS-DOS KERMIT compatibility modes */
int
setmsk() {
    if ((y = cmkey(msktab,nmsk,"MS-DOS Kermit compatibility mode",
                    "keycodes",xxstring)) < 0) return(y);

    switch ( y ) {
#ifdef COMMENT
      case MSK_COLOR:
        return(seton(&mskcolors));
#endif /* COMMENT */
      case MSK_KEYS:
        return(seton(&mskkeys));
      case MSK_REN:
        return(seton(&mskrename));
      default:                          /* Shouldn't get here. */
        return(-2);
    }
}
#endif /* OS2 */

#ifdef CKTIDLE
static char iactbuf[132];

char *
getiact() {
    switch (tt_idleact) {
      case IDLE_RET:  return("return");
      case IDLE_EXIT: return("exit");
      case IDLE_HANG: return("hangup");
#ifdef TNCODE
      case IDLE_TNOP: return("Telnet NOP");
      case IDLE_TAYT: return("Telnet AYT");
#endif /* TNCODE */

      case IDLE_OUT: {
          int c, k, n;
          char * p, * q, * t;
          k = ckstrncpy(iactbuf,"output ",132);
          n = k;
          q = &iactbuf[k];
          p = tt_idlestr;
          if (!p) p = "";
          if (!*p) return("output NUL");
          while ((c = *p++) && n < 131) {
              c &= 0xff;
              if (c == '\\') {
                  if (n > 130) break;
                  *q++ = '\\';
                  *q++ = '\\';
                  *q = NUL;
                  n += 2;
              } else if ((c > 32 && c < 127) || c > 159) {
                  *q++ = c;
                  *q = NUL;
                  n++;
              } else {
                  if (n > (131 - 6))
                    break;
                  sprintf(q,"\\{%d}",c);
                  k = strlen(q);
                  q += k;
                  n += k;
                  *q = NUL;
              }
          }
          *q = NUL;
#ifdef OS2
          k = tt_cols[VTERM];
#else
          k = tt_cols;
#endif /* OS2 */
          if (n > k - 52) {
              n = k - 52;
              iactbuf[n-2] = '.';
              iactbuf[n-1] = '.';
              iactbuf[n] = NUL;
          }
          return(iactbuf);
      }
      default: return("unknown");
    }
}
#endif /* CKTIDLE */

#ifndef NOCSETS
VOID
setlclcharset(x) int x; {
    int i;
    tcsl = y;                   /* Local character set */
#ifdef OS2
    for (i = 0; i < 4; i++) {
        G[i].init = TRUE;
        x = G[i].designation;
        G[i].c1 = (x != tcsl) && cs_is_std(x);
        x = G[i].def_designation;
        G[i].def_c1 = (x != tcsl) && cs_is_std(x);
    }
#endif /* OS2 */
}

VOID
setremcharset(x, z) int x, z; {
    int i;

#ifdef KUI
    KuiSetProperty( KUI_TERM_REMCHARSET, (long) x, (long) z ) ;
#endif /* KUI */
#ifdef UNICODE
    if (x == TX_TRANSP)
#else /* UNICODE */
    if (x == FC_TRANSP)
#endif /* UNICODE */
    {                           /* TRANSPARENT? */
#ifndef OS2
        tcsr = tcsl;            /* Make both sets the same */
#else /* OS2 */
#ifdef CKOUNI
        tt_utf8 = 0;            /* Turn off UTF8 flag */
        tcsr = tcsl = dec_kbd = TX_TRANSP; /* No translation */
        tcs_transp = 1;

        if (!cs_is_nrc(tcsl)) {
            G[0].def_designation = G[0].designation = TX_ASCII;
            G[0].init = TRUE;
            G[0].def_c1 = G[0].c1 = FALSE;
            G[0].size = cs94;
            G[0].national = FALSE;
        }
        for (i = cs_is_nrc(tcsl) ? 0 : 1; i < 4; i++) {
            G[i].def_designation = G[i].designation = tcsl;
            G[i].init = TRUE;
            G[i].def_c1 = G[i].c1 = FALSE;
            switch (cs_size(G[i].designation)) { /* 94, 96, or 128 */
            case 128:
            case 96:
                G[i].size = G[i].def_size = cs96;
                break;
            case 94:
                G[i].size = G[i].def_size = cs94;
                break;
            default:
                G[i].size = G[i].def_size = csmb;
                break;
            }
            G[i].national = cs_is_nrc(x);
        }
#else /* CKOUNI */
        tcsr = tcsl;            /* Make both sets the same */
        for (i = 0; i < 4; i++) {
            G[i].def_designation = G[i].designation = FC_TRANSP;
            G[i].init = FALSE;
            G[i].size = G[i].def_size = cs96;
            G[i].c1 = G[i].def_c1 = FALSE;
            G[i].rtoi = NULL;
            G[i].itol = NULL;
            G[i].ltoi = NULL;
            G[i].itor = NULL;
            G[i].national = FALSE;
        }
#endif /* CKOUNI */
#endif /* OS2 */
        return;
    }
#ifdef OS2
#ifdef CKOUNI
    else if (x == TX_UTF8) {
        tcs_transp = 0;
        tt_utf8 = 1;            /* Turn it on if we are UTF8 */
        return;
    }
#endif /* CKOUNI */
    else {
        tcs_transp = 0;
        tcsr = x;                       /* Remote character set */
#ifdef CKOUNI
        tt_utf8 = 0;                    /* Turn off UTF8 flag */
#endif /* CKOUNI */

        if (z == TT_GR_ALL) {
            int i;
#ifdef UNICODE
            dec_kbd = x;
#endif /* UNICODE */
            for (i = 0; i < 4; i++) {
                G[i].init = TRUE;
                if ( i == 0 && !cs_is_nrc(x) ) {
                    G[0].designation = G[0].def_designation = FC_USASCII;
                    G[0].size = G[0].def_size = cs94;
                    G[0].national = 1;
                } else {
                    G[i].def_designation = G[i].designation = x;
                    switch (cs_size(x)) {       /* 94, 96, or 128 */
                    case 128:
                    case 96:
                        G[i].size = G[i].def_size = cs96;
                        break;
                    case 94:
                        G[i].size = G[i].def_size = cs94;
                        break;
                    default:
                        G[i].size = G[i].def_size = csmb;
                        break;
                    }
                    G[i].national = cs_is_nrc(x);
                }
                G[i].c1 = G[i].def_c1 = x != tcsl && cs_is_std(x);
            }
#ifdef UNICODE
        } else if (z == TT_GR_KBD) {    /* Keyboard only */
            dec_kbd = x;
#endif /* UNICODE */
        } else {                        /* Specific Gn */
            G[z].def_designation = G[z].designation = x;
            G[z].init = TRUE;
            switch (cs_size(x)) {       /* 94, 96, or 128 */
            case 128:
            case 96:
                G[z].size = G[z].def_size = cs96;
                break;
            case 94:
                G[z].size = G[z].def_size = cs94;
                break;
            default:
                G[z].size = G[z].def_size = csmb;
                break;
            }
            G[z].c1 = G[z].def_c1 = x != tcsl && cs_is_std(x);
            G[z].national = cs_is_nrc(x);
        }
    }
#else /* not OS2 */
    tcsr = x;                   /* Remote character set */
#endif /* OS2 */
}
#endif /* NOCSETS */

VOID
setcmask(x) int x; {
    if (x == 7) {
        cmask = 0177;
    } else if (x == 8) {
        cmask = 0377;
        parity = 0;
    }
#ifdef KUI      
    KuiSetProperty(KUI_TERM_CMASK,x,0);
#endif /* KUI */
}

#ifdef CK_AUTODL
VOID
setautodl(x,y) int x,y; {
    autodl = x;
    adl_ask = y;
#ifdef KUI      
    KuiSetProperty(KUI_TERM_AUTODOWNLOAD,x?(y?2:1):0,0);
#endif /* KUI */
}
#endif /* CK_AUTODL */

#ifdef OS2
VOID
seturlhl(int x) {
    tt_url_hilite = x;
#ifdef KUI      
    KuiSetProperty(KUI_TERM_URL_HIGHLIGHT,x,0);
#endif /* KUI */
}

VOID
setaprint(int x) {
    extern int aprint;
    aprint = x;
#ifdef KUI
    KuiSetProperty(KUI_TERM_PRINTERCOPY,x,0);
#endif /* KUI */
}
#endif /* OS2 */

int
settrm() {
    int i = 0;
#ifdef OS2
    extern int colorreset, user_erasemode;
#endif /* OS2 */
    if ((y = cmkey(trmtab,ntrm,"", "",xxstring)) < 0) return(y);
#ifdef MAC
    printf("\n?Sorry, not implemented yet.  Please use the Settings menu.\n");
    return(-9);
#else
#ifdef IKSD
    if (inserver) {
        if ((y = cmcfm()) < 0) return(y);
        printf("?Sorry, command disabled.\r\n");
        return(success = 0);
    }
#endif /* IKSD */

    switch (y) {
      case XYTBYT:                      /* SET TERMINAL BYTESIZE */
        if ((y = cmnum("bytesize for terminal connection","8",10,&x,
                       xxstring)) < 0)
          return(y);
        if (x != 7 && x != 8) {
            printf("\n?The choices are 7 and 8\n");
            return(success = 0);
        }
        if ((y = cmcfm()) < 0) return(y);
        setcmask(x);
#ifdef OS2
        if (IS97801(tt_type_mode))
          SNI_bitmode(x);
#endif /* OS2 */
        return(success = 1);

      case XYTSO:                       /* SET TERMINAL LOCKING-SHIFT */
        return(seton(&sosi));

      case XYTNL:                       /* SET TERMINAL NEWLINE-MODE */
        return(seton(&tnlm));

#ifdef OS2
      case XYTCOL:
        if ((x = cmkey(ttycoltab,ncolors,"","terminal",xxstring)) < 0)
          return(x);
        else if (x == TTCOLRES) {
            if ((y = cmkey(ttcolmodetab,ncolmode,
                           "","default-color",xxstring)) < 0)
              return(y);
            if ((z = cmcfm()) < 0)
              return(z);
            colorreset = y;
            return(success = 1);
        } else if (x == TTCOLERA) {
            if ((y = cmkey(ttcolmodetab,ncolmode,"",
                           "current-color",xxstring)) < 0)
              return(y);
            if ((z = cmcfm()) < 0)
              return(z);
            user_erasemode = y;
            return(success=1);
        } else {                        /* No parse error */
            int fg = 0, bg = 0;
            fg = cmkey(ttyclrtab, nclrs,
                       (x == TTCOLBOR ?
                        "color for screen border" :
                        "foreground color and then background color"),
                       "lgray", xxstring);
            if (fg < 0)
              return(fg);
            if (x != TTCOLBOR) {
                if ((bg = cmkey(ttyclrtab,nclrs,
                                "background color","blue",xxstring)) < 0)
                  return(bg);
            }
            if ((y = cmcfm()) < 0)
              return(y);
            switch (x) {
              case TTCOLNOR:
                colornormal = fg | bg << 4;
                fgi = fg & 0x08;
                bgi = bg & 0x08;
                break;
              case TTCOLREV:
                colorreverse = fg | bg << 4;
                break;
              case TTCOLITA:
                coloritalic = fg | bg << 4;
                break;
              case TTCOLUND:
                colorunderline = fg | bg << 4;
                break;
              case TTCOLGRP:
                colorgraphic = fg | bg << 4;
                break;
              case TTCOLDEB:
                colordebug = fg | bg << 4;
                break;
              case TTCOLSTA:
                colorstatus = fg | bg << 4;
                break;
              case TTCOLHLP:
                colorhelp = fg | bg << 4;
                break;
              case TTCOLBOR:
                colorborder = fg;
                break;
              case TTCOLSEL:
                colorselect = fg | bg << 4;
                break;
              default:
                printf("%s - invalid\n",cmdbuf);
                return(-9);
                break;
            }
            scrninitialized[VTERM] = 0;
            VscrnInit(VTERM);
        }
        return(success = 1);

      case XYTCUR: {                    /* SET TERMINAL CURSOR */
          extern int cursorena[];
          extern int cursoron[] ;       /* Cursor state on/off       */
          if ((x = cmkey(ttycurtab,ncursors,"","underline",xxstring)) < 0)
            return(x);
          if ((z = cmkey(curontab,ncuron,"","on",xxstring)) < 0)
            return(z);
          if ((y = cmcfm()) < 0) return(y);
          tt_cursor = tt_cursor_usr = x;
          if ( z == 2 ) {
              cursorena[VTERM] = tt_cursorena_usr = 1;
              tt_cursor_blink = 0;
          } else {
              cursorena[VTERM] = tt_cursorena_usr = z;/* turn cursor on/off */
              tt_cursor_blink = 1;
          }
          cursoron[VTERM] = FALSE; /* Force newcursor to restore the cursor */
          return(success = 1);
      }
#endif /* OS2 */

      case XYTTYP:                      /* SET TERMINAL TYPE */
        return(settrmtyp());

#ifdef OS2
      case XYTARR:                      /* SET TERMINAL ARROW-KEYS */
        if ((x = cmkey(akmtab,2,"","",xxstring)) < 0) return(x);
        if ((y = cmcfm()) < 0) return(y);
        tt_arrow = x;                   /* TTK_NORM / TTK_APPL; see ckuusr.h */
        return(success = 1);

      case XYTKPD:                      /* SET TERMINAL KEYPAD-MODE */
        if ((x = cmkey(kpmtab,2,"","",xxstring)) < 0) return(x);
        if ((y = cmcfm()) < 0) return(y);
        tt_keypad = x;                  /* TTK_NORM / TTK_APPL; see ckuusr.h */
        return(success = 1);

      case XYTUNX: {                    /* SET TERM UNIX-MODE (DG) */
        extern int dgunix,dgunix_usr;
        x = seton(&dgunix);
        dgunix_usr = dgunix;
        return(x);
      }
      case XYTKBMOD: {                  /* SET TERM KEYBOARD MODE */
          extern int tt_kb_mode;
          if ((x = cmkey(kbmodtab,
                         nkbmodtab,
                         "normal",
                         "special keyboard mode for terminal emulation",
                         xxstring)
               ) < 0)
            return(x);
          if ((y = cmcfm()) < 0) return(y);
          tt_kb_mode = x;
          return(success = 1);
      }

      case XYTWRP:                      /* SET TERMINAL WRAP */
        return(seton(&tt_wrap));

      case XYSCRS:
        if ((y = cmnum("CONNECT scrollback buffer size, lines","2000",10,&x,
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
        tt_scrsize[VTERM] = x;
        VscrnInit(VTERM);
        return(success = 1);
#endif /* OS2 */

#ifndef NOCSETS
      case XYTCS: {                     /* SET TERMINAL CHARACTER-SET */
        int eol;
          /* set terminal character-set <remote> <local> */
        if ((x = cmkey(
#ifdef CKOUNI
                       txrtab,ntxrtab,
#else  /* CKOUNI */
                       ttcstab,ntermc,
#endif /* CKOUNI */
                       "remote terminal character-set","",xxstring)) < 0)
          return(x);

#ifdef UNICODE
        if (x == TX_TRANSP
#ifdef CKOUNI
            || x == TX_UTF8
#endif /* CKOUNI */
           ) {
              if ((y = cmcfm()) < 0)    /* Confirm the command */
                  return(y);
#ifdef OS2
            if ( isunicode() && x == TX_TRANSP ) {
                /* If we are in unicode display mode then transparent
                 * only affects the output direction.  We need to know
                 * the actual remote character set in order to perform
                 * the tcsr -> ucs2 translation for display.
                 */
                x = y = tcsl;
            } else
#endif /* OS2 */
                y = x;
        }
#else /* UNICODE */
        if (x == FC_TRANSP) {
            if ((y = cmcfm()) < 0)      /* Confirm the command */
                return(y);
            y = x;
        }
#endif /* UNICODE */

        /* Not transparent or UTF8, so get local set to translate it into */
        s = "";
#ifdef OS2
        y = os2getcp();                 /* Default is current code page */
        switch (y) {
          case 437: s = "cp437"; break;
          case 850: s = "cp850"; break;
          case 852: s = "cp852"; break;
          case 857: s = "cp857"; break;
          case 858: s = "cp858"; break;
          case 862: s = "cp862"; break;
          case 866: s = "cp866"; break;
          case 869: s = "cp869"; break;
          case 1250: s = "cp1250"; break;
          case 1251: s = "cp1251"; break;
          case 1252: s = "cp1252"; break;
          case 1253: s = "cp1253"; break;
          case 1254: s = "cp1254"; break;
          case 1255: s = "cp1255"; break;
          case 1256: s = "cp1256"; break;
          case 1257: s = "cp1257"; break;
          case 1258: s = "cp1258"; break;
        }
#ifdef PCFONTS
/*
   If the user has loaded a font with SET TERMINAL FONT then we want
   to change the default code page to the font that was loaded.
*/
        if (tt_font != TTF_ROM) {
            for (y = 0; y < ntermfont; y++ ) {
                if (term_font[y].kwval == tt_font) {
                    s = term_font[y].kwd;
                    break;
                }
            }
        }
#endif /* PCFONTS */
#else  /* Not K95... */
        s = fcsinfo[fcharset].keyword;
#endif /* OS2 */

        if ((y = cmkey(
#ifdef CKOUNI
                       txrtab,ntxrtab,
#else /* CKOUNI */
                       ttcstab,ntermc,
#endif /* CKOUNI */
                       "local character-set",s,xxstring)) < 0)
          return(y);

#ifdef UNICODE
        if (y == TX_UTF8) {
            printf("?UTF8 may not be used as a local character set.\r\n");
            return(-9);
        }
#endif /* UNICODE */
#ifdef OS2
        if ((z = cmkey(graphsettab,ngraphset,
                       "DEC VT intermediate graphic set","all",xxstring)) < 0)
            return(z);
#endif /* OS2 */
        if ((eol = cmcfm()) < 0)
            return(eol); /* Confirm the command */

        /* End of command parsing - actions begin */
        setlclcharset(y);
        setremcharset(x,z);
        return(success = 1);
      }
#endif /* NOCSETS */

#ifndef NOCSETS
      case XYTLCS:                      /* SET TERMINAL LOCAL-CHARACTER-SET */
        /* set terminal character-set <local> */
        s = getdcset();                 /* Get display character-set name */
        if ((y = cmkey(
#ifdef CKOUNI
                       txrtab,ntxrtab,
#else /* CKOUNI */
                       fcstab,nfilc,
#endif /* CKOUNI */
                       "local character-set",s,xxstring)) < 0)
          return(y);

#ifdef UNICODE
          if (y == TX_UTF8) {
              printf("?UTF8 may not be used as a local character set.\r\n");
              return(-9);
          }
#endif /* UNICODE */
          if ((z = cmcfm()) < 0) return(z); /* Confirm the command */

          /* End of command parsing - action begins */

        setlclcharset(y);
        return(success = 1);
#endif /* NOCSETS */

#ifndef NOCSETS
#ifdef UNICODE
      case XYTUNI:                      /* SET TERMINAL UNICODE */
        return(seton(&tt_unicode));
#endif /* UNICODE */

      case XYTRCS:                      /* SET TERMINAL REMOTE-CHARACTER-SET */
        /* set terminal character-set <remote> <Graphic-set> */
        if ((x = cmkey(
#ifdef CKOUNI
                txrtab, ntxrtab,
#else /* CKOUNI */
                ttcstab,ntermc,
#endif /* CKOUNI */
                       "remote terminal character-set","",xxstring)) < 0)
          return(x);

#ifdef UNICODE
        if (x == TX_TRANSP
#ifdef CKOUNI
            || x == TX_UTF8
#endif /* CKOUNI */
           ) {
              if ((y = cmcfm()) < 0)    /* Confirm the command */
                  return(y);
#ifdef OS2
            if ( isunicode() && x == TX_TRANSP ) {
                /* If we are in unicode display mode then transparent
                 * only affects the output direction.  We need to know
                 * the actual remote character set in order to perform
                 * the tcsr -> ucs2 translation for display.
                 */
                x = tcsl;
            }
#endif /* OS2 */
        }
#else /* UNICODE */
        if (x == FC_TRANSP) {
          if ((y = cmcfm()) < 0)        /* Confirm the command */
            return(y);
        }
#endif /* UNICODE */
        else {
#ifdef OS2
          if ((z = cmkey(graphsettab,ngraphset,
                      "DEC VT intermediate graphic set","all",xxstring)) < 0)
            return(z);
#endif /* OS2 */
          if ((y = cmcfm()) < 0)        /* Confirm the command */
            return(y);
        }
        /* Command parsing ends here */

        setremcharset(x,z);
        return(success = 1);
#endif /* NOCSETS */

      case XYTEC:                       /* SET TERMINAL ECHO */
        if ((x = cmkey(rltab,nrlt,"which side echos during CONNECT",
                       "remote", xxstring)) < 0) return(x);
        if ((y = cmcfm()) < 0) return(y);
#ifdef NETCONN
        oldplex = x;
#endif /* NETCONN */
        duplex = x;
        return(success = 1);

      case XYTESC:                      /* SET TERM ESC */
        if ((x = cmkey(nabltab,nnabltab,"","enabled",xxstring)) < 0)
          return(x);
        if ((y = cmcfm()) < 0) return(y);
        tt_escape = x;
        return(1);

      case XYTCRD:                      /* SET TERMINAL CR-DISPLAY */
        if ((x = cmkey(crdtab,2,"", "normal", xxstring)) < 0) return(x);
        if ((y = cmcfm()) < 0) return(y);
        tt_crd = x;
        return(success = 1);

      case XYTLFD:                      /* SET TERMINAL LF-DISPLAY */
        if ((x = cmkey(crdtab,2,"", "normal", xxstring)) < 0) return(x);
        if ((y = cmcfm()) < 0) return(y);
        tt_lfd = x;
        return(success = 1);

#ifdef OS2
      case XYTANS: {                    /* SET TERMINAL ANSWERBACK */
/*
  NOTE: We let them enable and disable the answerback sequence, but we
  do NOT let them change it, and we definitely do not let the host set it.
  This is a security feature.

  As of 1.1.8 we allow the SET TERM ANSWERBACK MESSAGE <string> to be
  used just as MS-DOS Kermit does.  C0 and C1 controls as well as DEL
  are not allowed to be used as characters.  They are translated to
  underscore.  This may not be set by APC.
*/
          if ((x = cmkey(anbktab,nansbk,"", "off", xxstring)) < 0)
            return(x);
          if (x < 2) {
              if ((y = cmcfm()) < 0)
                return(y);
              tt_answer = x;
              return(success = 1);
          } else if ( x == 2 || x == 3) {
              int len = 0;
              extern int safeanswerbk;
              extern char useranswerbk[];
              if ((y = cmtxt("Answerback extension","",&s,xxstring)) < 0)
                return(y);
              if (apcactive == APC_LOCAL ||
                  (apcactive == APC_REMOTE && !(apcstatus & APC_UNCH)))
                return(success = 0);
              len = strlen(s);
              if (x == 2) {
                  /* Safe Answerback's don't have C0/C1 chars */
                  for (z = 0; z < len; z++) {
                      if ((s[z] & 0x7F) <= SP || (s[z] & 0x7F) == DEL)
                        useranswerbk[z] = '_';
                      else
                        useranswerbk[z] = s[z];
                  }
                  useranswerbk[z] = '\0';
                  safeanswerbk = 1 ;    /* TRUE */
              } else {
                  ckstrncpy(useranswerbk,s,60); /* (see ckocon.c) */
                  safeanswerbk = 0;     /* FALSE */
              }
              updanswerbk();
              return(success = 1);
          } else
            return(success = 0);
      }
#endif /* OS2 */

#ifdef CK_APC
      case XYTAPC:
        if ((y = cmkey(apctab,napctab,
                       "application program command execution","",
                       xxstring)) < 0)
          return(y);
        if ((x = cmcfm()) < 0)
          return(x);
        if (apcactive == APC_LOCAL ||
            (apcactive == APC_REMOTE && !(apcstatus & APC_UNCH)))
          return(success = 0);
        apcstatus = y;
        return(success = 1);

#ifdef CK_AUTODL
      case XYTAUTODL:                   /* AUTODOWNLOAD */
        if ((y = cmkey(adltab,nadltab,"Auto-download options","",
                       xxstring)) < 0)
          return(y);
        switch (y) {
          case TAD_ON:
          case TAD_OFF:
            if ((x = cmcfm()) < 0)
              return(x);
            setautodl(y,0);
            break;
          case TAD_ASK:
            if ((x = cmcfm()) < 0)
              return(x);
            setautodl(TAD_ON,1);
            break;
          case TAD_ERR:
            if ((y = cmkey(adlerrtab,nadlerrtab,"","", xxstring)) < 0)
              return(y);
            if ((x = cmcfm()) < 0)
              return(x);
            adl_err = y;
            break;
#ifdef OS2
          case TAD_K:
            if ((y = cmkey(adlxtab,nadlxtab,"","", xxstring)) < 0)
              return(y);
            switch (y) {
              case TAD_X_C0:
                if ((y = cmkey(adlc0tab,nadlc0tab,"",
                               "processed-by-emulator",xxstring)) < 0)
                  return(y);
                if ((x = cmcfm()) < 0)
                  return(x);
                adl_kc0 = y;
                break;
              case TAD_X_DETECT:
                if ((y = cmkey(adldtab,nadldtab,"","packet",xxstring)) < 0)
                  return(y);
                if ((x = cmcfm()) < 0)
                  return(x);
                adl_kmode = y;
                break;
              case TAD_X_STR:
                if ((y = cmtxt("Kermit start string","KERMIT READY TO SEND...",
                               &s,xxstring)) < 0)
                  return(y);
                free(adl_kstr);
                adl_kstr = strdup(s);
                break;
            }
            break;

          case TAD_Z:
            if ((y = cmkey(adlxtab,nadlxtab,"","",xxstring)) < 0)
              return(y);
            switch (y) {
              case TAD_X_C0:
                if ((y = cmkey(adlc0tab,nadlc0tab,"",
                               "processed-by-emulator",xxstring)) < 0)
                  return(y);
                if ((x = cmcfm()) < 0)
                  return(x);
                adl_zc0 = y;
                break;
              case TAD_X_DETECT:
                if ((y = cmkey(adldtab,nadldtab,"","packet",xxstring)) < 0)
                  return(y);
                if ((x = cmcfm()) < 0)
                  return(x);
                adl_zmode = y;
                break;
              case TAD_X_STR:
                if ((y = cmtxt("","rz\\{13}",&s,xxstring)) < 0)
                  return(y);
                free(adl_zstr);
                adl_zstr = strdup(s);
                break;
            }
            break;
#endif /* OS2 */
        }
        return(success = 1);

#endif /* CK_AUTODL */
#endif /* CK_APC */

#ifdef OS2
      case XYTBEL:
        return(success = setbell());

      case XYTMBEL:                     /* MARGIN-BELL */
        if ((y = cmkey(onoff,2,"","on",xxstring)) < 0) return(y);
        if (y) {                        /* ON */
            if ((z = cmnum("Column at which to set margin bell",
                           "72",10,&x,xxstring)) < 0)
              return(z);
        }
        if ((z = cmcfm()) < 0) return(z);
        marginbell = y;
        marginbellcol = x;
        return(success = 1);
#endif /* OS2 */

#ifdef CKTIDLE
      case XYTIDLE:                     /* IDLE-SEND */
      case XYTITMO:                     /* IDLE-TIMEOUT */
        if ((z = cmnum("seconds of idle time to wait, or 0 to disable",
                       "0",10,&x,xxstring)) < 0)
          return(z);
        if (y == XYTIDLE) {
            if ((y = cmtxt("string to send, may contain kverbs and variables",
                           "\\v(newline)",&s,xxstring)) < 0)
              return(y);
            tt_idlesnd_tmo = x;         /* (old) */
            tt_idlelimit = x;           /* (new) */
            makestr(&tt_idlestr,brstrip(s)); /* (new) */
            tt_idlesnd_str = tt_idlestr; /* (old) */
            tt_idleact = IDLE_OUT;      /* (new) */
        } else {
            if ((y = cmcfm()) < 0)
              return(y);
            tt_idlelimit = x;
        }
#ifdef OS2
        puterror(VTERM);
#endif /* OS2 */
        return(success = 1);

      case XYTIACT: {                   /* SET TERM IDLE-ACTION */
          if ((y = cmkey(idlacts,nidlacts,"","",xxstring)) < 0)
            return(y);
          if (y == IDLE_OUT) {
              if ((x = cmtxt("string to send, may contain kverbs and variables"
                             , "",&s,xxstring)) < 0)
                return(x);
              makestr(&tt_idlestr,brstrip(s)); /* (new) */
              tt_idlesnd_str = tt_idlestr; /* (old) */
          } else {
              if ((x = cmcfm()) < 0)
                return(x);
          }
          tt_idleact = y;
          return(success = 1);
      }
#endif /* CKTIDLE */

      case XYTDEB:                      /* TERMINAL DEBUG */
        y = seton(&x);                  /* Go parse ON or OFF */
        if (y > 0)                      /* Command succeeded? */
          setdebses(x);
        return(y);

#ifdef OS2
      case XYTASCRL:                    /* SET TERMINAL AUTOSCROLL */
          y = seton(&autoscroll);
          return(y);

      case XYTAPAGE:                    /* SET TERMINAL AUTOPAGE */
          y = seton(&wy_autopage);
          return(y);

      case XYTROL:                      /* SET TERMINAL ROLL */
        if ((y = cmkey(rolltab,nroll,"scrollback mode","insert",xxstring))<0)
	  return(y);
	if (y == TTR_KEYS) {
	    if ((x = cmkey(rollkeytab,nrollkey,"","send",xxstring))<0)
	      return(x);
	    if ((z = cmcfm()) < 0) return(z);
	    tt_rkeys[VTERM] = x;
	} else {
	    if ((x = cmcfm()) < 0) return(x);
	    tt_roll[VTERM] = y;
	}
        return(success = 1);

      case XYTCTS:                      /* SET TERMINAL TRANSMIT-TIMEOUT */
        y = cmnum("Maximum seconds to allow CTS off during CONNECT",
                  "5",10,&x,xxstring);
        return(setnum(&tt_ctstmo,x,y,10000));

      case XYTCPG: {                    /* SET TERMINAL CODE-PAGE */
        int i;
        int cp = -1;
        y = cmnum("PC code page to use during terminal emulation",
                  ckitoa(os2getcp()),10,&x,xxstring);
        if ((x = setnum(&cp,x,y,11000)) < 0) return(x);
        if (os2setcp(cp) != 1) {
#ifdef NT
            if (isWin95())
              printf(
                 "Sorry, Windows 95 does not support code page switching\n");
            else
#endif /* NT */
              printf(
                 "Sorry, %d is not a valid code page for this system.\n",cp);
            return(-9);
        }
    /* Force the terminal character-sets conversions to be updated */
        for ( i = 0; i < 4; i++ )
          G[i].init = TRUE;
        return(1);
    }

      case XYTPAC:                      /* SET TERMINAL OUTPUT-PACING */
        y = cmnum(
           "Pause between sending each character during CONNECT, milliseconds",
                  "-1",10,&x,xxstring);
        return(setnum(&tt_pacing,x,y,10000));

#ifdef OS2MOUSE
      case XYTMOU: {                    /* SET TERMINAL MOUSE */
          int old_mou = tt_mouse;
          if ((x = seton(&tt_mouse)) < 0)
            return(x);
          if (tt_mouse != old_mou)
            if (tt_mouse)
              os2_mouseon();
            else
              os2_mouseoff();
          return(1);
      }
#endif /* OS2MOUSE */
#endif /* OS2 */

      case XYTWID: {
          if ((y = cmnum(
#ifdef OS2
                         "number of columns in display window during CONNECT",
#else
                         "number of columns on your screen",
#endif /* OS2 */
                         "80",10,&x,xxstring)) < 0)
            return(y);
          if ((y = cmcfm()) < 0) return(y);
#ifdef OS2
          return(success = os2_settermwidth(x));
#else  /* Not OS/2 */
          tt_cols = x;
          return(success = 1);
#endif /* OS2 */
      }

      case XYTHIG:
        if ((y = cmnum(
#ifdef OS2
 "number of rows in display window during CONNECT, not including status line",
 tt_status[VTERM]?"24":"25",
#else
 "24","number of rows on your screen",
#endif /* OS2 */
                       10,&x,xxstring)) < 0)
          return(y);
        if ((y = cmcfm()) < 0) return(y);

#ifdef OS2
        return (success = os2_settermheight(x));
#else  /* Not OS/2 */
        tt_rows = x;
        return(success = 1);
#endif /* OS2 */

#ifdef OS2
      case XYTPRN: {                    /* Print Mode */
          extern bool xprint, aprint, cprint, uprint;
          if ((y = cmkey(prnmtab,nprnmtab,"","off", xxstring)) < 0) return(y);
          if ((x = cmcfm()) < 0) return(x);
          switch (y) {
            case 0:
              if (cprint || uprint || aprint || xprint)
                printeroff();
              cprint = xprint = uprint = 0;
              setaprint(0);
              break;
            case 1:
              if (!(cprint || uprint || aprint || xprint))
                printeron();
              setaprint(1);
              cprint = xprint = uprint = 0;
              break;
            case 2:
              if (!(cprint || uprint || aprint || xprint))
                printeron();
              cprint = 1;
              setaprint(0);
	      xprint = uprint = 0;
              break;
            case 3:
              if (!(cprint || uprint || aprint || xprint))
                printeron();
              uprint = 1;
              setaprint(0);
              xprint = cprint = 0;
              break;
          }
          return(1);
      }
#else
#ifdef XPRINT
      case XYTPRN: {
          extern int tt_print;
          if ((x = seton(&tt_print)) < 0)
            return(x);
          return(success = 1);
      }
#endif /* XPRINT */
#endif /* OS2 */

#ifdef OS2
      case XYTSCNM: {
          extern int decscnm, decscnm_usr;
          if ((y = cmkey(normrev,4,"",
                         decscnm_usr?"reverse":"normal",
                         xxstring)
               ) < 0)
            return(y);
          if ((x = cmcfm()) < 0) return(x);
          decscnm_usr = y;
          if (decscnm != decscnm_usr)
            flipscreen(VTERM);
          return(1);
    }
    case XYTOPTI:
        if ((y = cmkey(onoff,2,"",tt_diff_upd?"on":"off",
                        xxstring)) < 0) return(y);
        if ((x = cmcfm()) < 0) return(x);
        tt_diff_upd = y;
        return(1);
    case XYTUPD: {
        int mode, delay;
        if ((mode = cmkey(scrnupd,nscrnupd,"","fast",xxstring)) < 0) {
            return(mode);
        } else {
            y = cmnum(
            "Pause between FAST screen updates in CONNECT mode, milliseconds",
                      "100",10,&x,xxstring
                      );
            if (x < 0 || x > 1000 ) {
                printf(
            "\n?The update rate must be between 0 and 1000 milliseconds.\n"
                       );
                return(success = 0);
            }
            if ((y = cmcfm()) < 0) return(y);

            updmode = tt_updmode = mode;
            return(setnum(&tt_update,x,y,10000));
        }
    }
    case XYTCTRL:
          if ((x = cmkey(termctrl,ntermctrl,"","7",xxstring)) < 0) {
              return(x);
          } else {
              if ((y = cmcfm()) < 0)
                  return(y);
              switch ( x ) {
              case 8:
                  send_c1 = send_c1_usr = TRUE;
                  break;
              case 7:
              default:
                  send_c1 = send_c1_usr = FALSE;
                  break;
              }
          }
          return(success = TRUE);
          break;

#ifdef PCFONTS
      case XYTFON:
        if ( !IsOS2FullScreen() ) {
            printf(
        "\n?SET TERMINAL FONT is only supported in Full Screen sessions.\n");
            return(success = FALSE);
        }

        if ((x = cmkey(term_font,ntermfont,"","default",xxstring)) < 0) {
            return(x);
        } else {
            if ((y = cmcfm()) < 0) return(y);
            if ( !os2LoadPCFonts() ) {
                tt_font = x;
                return(success = TRUE);
            } else {
                printf(
      "\n?PCFONTS.DLL is not available in CKERMIT executable directory.\n");
                return(success = FALSE);
            }
        }
        break;
#else /* PCFONTS */
#ifdef NT
#ifdef KUI
    case XYTFON:
        return(setguifont());           /* ckuus3.c */
#endif /* KUI */
#endif /* NT */
#endif /* PCFONTS */

      case XYTVCH: {
          extern int pheight, marginbot, cmd_rows, cmd_cols;
          if ((x = cmkey(tvctab,ntvctab,"",isWin95()?"win95-safe":"enabled",
                         xxstring)) < 0)
            return(x);
          if ((y = cmcfm()) < 0) return(y);
#ifndef KUI
          if (x != tt_modechg) {
              switch (x) {
                case TVC_DIS:
                  /* When disabled the heights of all of the virtual screens */
                  /* must be equal to the physical height of the console     */
                  /* window and may not be changed.                          */
                  /* The width of the window may not be altered.             */
                  tt_modechg = TVC_ENA;                 /* Temporary */
                  if (marginbot > pheight-(tt_status[VTERM]?1:0))
                    marginbot = pheight-(tt_status[VTERM]?1:0);
                  tt_szchng[VCMD] = 1 ;
                  tt_rows[VCMD] = pheight;
                  VscrnInit(VCMD);
                  SetCols(VCMD);
                  cmd_rows = y;

                  tt_szchng[VTERM] = 2 ;
                  tt_rows[VTERM] = pheight - (tt_status[VTERM]?1:0);
                  VscrnInit(VTERM);

                  break;

                case TVC_ENA:
                  /* When enabled the physical height of the console windows */
                  /* should be adjusted to the height of the virtual screen  */
                  /* The width may be set to anything.                       */
                  /* nothing to do                                           */
                  break;

              case TVC_W95:
                  /* Win95-safe mode allows the physical height to change    */
                  /* but restricts it to a width of 80 and a height equal to */
                  /* 25, 43, or 50.  Must be adjusted now.                   */
                  /* The virtual heights must be equal to the above.         */
                  if (pheight != 25 && pheight != 43 && pheight != 50) {
                      if (pheight < 25)
                        y = 25;
                      else if (pheight < 43)
                        y = 43;
                      else
                        y = 50;
                  } else
                    y = pheight;

                  tt_modechg = TVC_ENA; /* Temporary */

                  tt_szchng[VCMD] = 1;
                  tt_rows[VCMD] = y;
                  tt_cols[VCMD] = 80;
                  VscrnInit(VCMD);
                  SetCols(VCMD);
                  cmd_rows = y;
                  cmd_cols = 80;

                  marginbot = y-(tt_status[VTERM]?1:0);
                  tt_szchng[VTERM] = 2;
                  tt_rows[VTERM] = y - (tt_status[VTERM]?1:0);
                  tt_cols[VTERM] = 80;
                  VscrnInit(VTERM);
                  break;
              }
              tt_modechg = x;
          }
          return(success = 1);
#else
          return(success = 0);
#endif /* KUI */
      }
      case XYTSTAT: {
          extern int marginbot;
          if ((y = cmkey(onoff,2,"","on",xxstring)) < 0) return(y);
          if ((x = cmcfm()) < 0) return(x);
          if (y != tt_status[VTERM] || y != tt_status_usr[VTERM]) {
              /* Might need to fixup the margins */
              if ( marginbot == VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0) )
                if (y) {
                    marginbot--;
                } else {
                    marginbot++;
                }
              tt_status_usr[VTERM] = tt_status[VTERM] = y;
              if (y) {
                    tt_szchng[VTERM] = 2;
                    tt_rows[VTERM]--;
                    VscrnInit(VTERM);  /* Height set here */
#ifdef TNCODE
                    if (TELOPT_ME(TELOPT_NAWS))
                      tn_snaws();
#endif /* TNCODE */
#ifdef RLOGCODE
                    if (TELOPT_ME(TELOPT_NAWS))
                      rlog_naws();
#endif /* RLOGCODE */
#ifdef SSHBUILTIN
                    if (TELOPT_ME(TELOPT_NAWS))
                      ssh_snaws();
#endif /* SSHBUILTIN */
              } else {
                  tt_szchng[VTERM] = 1;
                  tt_rows[VTERM]++;
                  VscrnInit(VTERM);     /* Height set here */
#ifdef TNCODE
                  if (TELOPT_ME(TELOPT_NAWS))
                    tn_snaws();
#endif /* TNCODE */
#ifdef RLOGCODE
                  if (TELOPT_ME(TELOPT_NAWS))
                    rlog_naws();
#endif /* RLOGCODE */
#ifdef SSHBUILTIN
                  if (TELOPT_ME(TELOPT_NAWS))
                    ssh_snaws();
#endif /* SSHBUILTIN */
              }
          }
          return(1);
      }
#endif /* OS2 */

#ifdef NT
      case XYTATTBUG:
        if ((y = cmkey(onoff,2,"","on",xxstring)) < 0) return(y);
        if ((x = cmcfm()) < 0) return(x);
        tt_attr_bug = y;
        return(1);
#endif /* NT */

#ifdef OS2
      case XYTSGRC:
        if ((y = cmkey(onoff,2,"","on",xxstring)) < 0) return(y);
        if ((x = cmcfm()) < 0) return(x);
        sgrcolors = y;
        return(1);

      case XYTSEND:
          if ((y = cmkey(onoff,2,"","on",xxstring)) < 0) return(y);
          if ((x = cmcfm()) < 0) return(x);
          tt_senddata = y;
          return(1);

      case XYTSEOB:
          if ((y = cmkey(ttyseobtab,2,"","us_cr",xxstring)) < 0) return(y);
          if ((x = cmcfm()) < 0) return(x);
          wy_blockend = y;
          return(1);

      case XYTURLHI: {
          int done = 0, attr = VT_CHAR_ATTR_NORMAL;

          if ((x = cmkey(onoff,2,"","on",xxstring)) < 0)
            return(x);
          if (x) {
              z = 0;
              while (!done) {
                  if ((y = cmkey(ttyprotab,nprotect,"",
                                 z?"done":"reverse",xxstring)) < 0)
                    return(y);
                  switch (y) {
                    case TTATTDONE:
                      done = TRUE;
                      break;
                    case TTATTBLI:
                      attr |= VT_CHAR_ATTR_BLINK;
                      break;
                    case TTATTREV:
                      attr |= VT_CHAR_ATTR_REVERSE;
                      break;
                    case TTATTITA:
                      attr |= VT_CHAR_ATTR_ITALIC;
                      break;
                    case TTATTUND:
                      attr |= VT_CHAR_ATTR_UNDERLINE;
                      break;
                    case TTATTBLD:
                      attr |= VT_CHAR_ATTR_BOLD;
                      break;
                    case TTATTDIM:
                      attr |= VT_CHAR_ATTR_DIM;
                      break;
                    case TTATTINV:
                      attr |= VT_CHAR_ATTR_INVISIBLE;
                      break;
                    case TTATTNOR:
                      break;
                  }
                  z = 1;                /* One attribute has been chosen */
              }
          }
          if ((z = cmcfm()) < 0) return(z);
          seturlhl(x);
          if (x)
            tt_url_hilite_attr = attr;
          return(1);
      }
      case XYTATTR:
        if ((x = cmkey(ttyattrtab,nattrib,"","underline",xxstring)) < 0)
          return(x);
        switch (x) {
          case TTATTBLI:
            if ((y = cmkey(onoff,2,"","on",xxstring)) < 0) return(y);
            if ((x = cmcfm()) < 0) return(x);
            trueblink = y;
#ifndef KUI
            if ( !trueblink && trueunderline ) {
                trueunderline = 0;
                printf("Warning: Underline being simulated by color.\n");
            }

#endif /* KUI */
            break;

          case TTATTDIM:
            if ((y = cmkey(onoff,2,"","on",xxstring)) < 0) return(y);
            if ((x = cmcfm()) < 0) return(x);
            truedim = y;
            break;

          case TTATTREV:
            if ((y = cmkey(onoff,2,"","on",xxstring)) < 0) return(y);
            if ((x = cmcfm()) < 0) return(x);
            truereverse = y;
            break;

          case TTATTUND:
            if ((y = cmkey(onoff,2,"","on",xxstring)) < 0) return(y);
            if ((x = cmcfm()) < 0) return(x);
            trueunderline = y;
#ifndef KUI
            if (!trueblink && trueunderline) {
                trueblink = 1;
                printf("Warning: True blink mode is active.\n");
            }
#endif /* KUI */
            break;

          case TTATTITA:
              if ((y = cmkey(onoff,2,"","on",xxstring)) < 0) return(y);
              if ((x = cmcfm()) < 0) return(x);
              trueitalic = y;
            break;

          case TTATTPRO: {      /* Set default Protected Character attribute */
              extern vtattrib WPattrib;    /* current WP Mode Attrib */
              extern vtattrib defWPattrib; /* default WP Mode Attrib */
              vtattrib wpa = {0,0,0,0,0,1,0,0,0,0,0};   /* Protected */
              int done = 0;

              x = 0;
              while (!done) {
                  if ((y = cmkey(ttyprotab,nprotect,"",
                                 x?"done":"dim",xxstring)) < 0)
                    return(y);
                  switch (y) {
                    case TTATTNOR:
                      break;
                    case TTATTBLI:      /* Blinking doesn't work */
                      wpa.blinking = TRUE;
                      break;
                    case TTATTREV:
                      wpa.reversed = TRUE;
                      break;
                    case TTATTITA:
                      wpa.italic = TRUE;
                      break;
                    case TTATTUND:
                      wpa.underlined = TRUE;
                      break;
                    case TTATTBLD:
                      wpa.bold = TRUE;
                      break;
                    case TTATTDIM:
                      wpa.dim = TRUE;
                      break;
                    case TTATTINV:
                      wpa.invisible = TRUE ;
                      break;
                    case TTATTDONE:
                      done = TRUE;
                      break;
                  }
                  x = 1;                /* One attribute has been chosen */
              }
              if ((x = cmcfm()) < 0) return(x);
              WPattrib = defWPattrib = wpa;
              break;
          }
        }
        return(1);

      case XYTKEY: {                    /* SET TERMINAL KEY */
          int t, x, y;
          int clear = 0, deflt = 0;
          int confirmed = 0;
          int flag = 0;
          int kc = -1;                  /* Key code */
          int litstr = 0;               /* Literal String? */
          char *s = NULL;               /* Key binding */
#ifndef NOKVERBS
          char *p = NULL;               /* Worker */
#endif /* NOKVERBS */
          con_event defevt;
          extern int os2gks;
          extern int mskkeys;
          extern int initvik;
          struct FDB kw,sw,nu,cm;

          defevt.type = error;

          if ((t = cmkey(ttkeytab,nttkey,"","",xxstring)) < 0)
            return(t);
          cmfdbi(&nu,                   /* First FDB - command switches */
                 _CMNUM,                /* fcode */
                 "/literal, keycode, or action",
                 "",                    /* default */
                 "",                    /* addtl string data */
                 10,                    /* addtl numeric data 1: radix */
                 0,                     /* addtl numeric data 2: 0 */
                 xxstring,              /* Processing function */
                 NULL,                  /* Keyword table */
                 &sw                    /* Pointer to next FDB */
                 );                     /*  */
          cmfdbi(&sw,                   /* Second FDB - switches */
                 _CMKEY,                /* fcode */
                 "",
                 "",                    /* default */
                 "",                    /* addtl string data */
                 nstrmswitab,           /* addtl numeric data 1: tbl size */
                 4,                     /* addtl numeric data 2: 4 = cmswi */
                 xxstring,              /* Processing function */
                 strmswitab,            /* Keyword table */
                 &kw                    /* Pointer to next FDB */
                 );
          cmfdbi(&kw,                   /* Third FDB - command switches */
                 _CMKEY,                /* fcode */
                 "/literal, keycode, or action",
                 "",                    /* default */
                 "",                    /* addtl string data */
                 nstrmkeytab,           /* addtl numeric data 1: tbl size */
                 0,                     /* addtl numeric data 2 */
                 xxstring,              /* Processing function */
                 strmkeytab,            /* Keyword table */
                 &cm                    /* Pointer to next FDB */
                 );
          cmfdbi(&cm,                   /* Final FDB - Confirmation */
                 _CMCFM,                /* fcode */
                 "",
                 "",                    /* default */
                 "",                    /* addtl string data */
                 0,                     /* addtl numeric data 1: tbl size */
                 0,                     /* addtl numeric data 2: 4 = cmswi */
                 xxstring,              /* Processing function */
                 NULL,                  /* Keyword table */
                 NULL                   /* Pointer to next FDB */
                 );
          while (kc < 0) {
              x = cmfdb(&nu);           /* Parse something */
              if (x < 0)
                return(x);

              switch (cmresult.fcode) {
                case _CMCFM:
                  printf(" Press key to be defined: ");
                  conbin((char)escape); /* Put terminal in binary mode */
                  os2gks = 0;           /* Turn off Kverb preprocessing */
                  kc = congks(0);       /* Get character or scan code */
                  os2gks = 1;           /* Turn on Kverb preprocessing */
                  concb((char)escape);  /* Restore terminal to cbreak mode */
                  if (kc < 0) {         /* Check for error */
                      printf("?Error reading key\n");
                      return(0);
                  }
                  shokeycode(kc,t);     /* Show current definition */
                  flag = 1;             /* Remember it's a multiline command */
                  break;
                case _CMNUM:
                  kc = cmresult.nresult;
                  break;
                case _CMKEY:
                  if (cmresult.fdbaddr == &sw) { /* Switch */
                      if (cmresult.nresult == 0)
                        litstr = 1;
                  } else if (cmresult.fdbaddr == &kw) { /* Keyword */
                      if (cmresult.nresult == 0)
                        clear = 1;
                      else
                        deflt = 1;
                      if ((x = cmcfm()) < 0)
                        return(x);
                      if (clear)
                        clearkeymap(t);
                      else if (deflt)
                        defaultkeymap(t);
                      initvik = 1;
                      return(1);
                  }
              }
          }

    /* Normal SET TERMINAL KEY <terminal> <scancode> <value> command... */

          if (mskkeys)
            kc = msktock(kc);

          if (kc < 0 || kc >= KMSIZE) {
              printf("?key code must be between 0 and %d\n", KMSIZE - 1);
              return(-9);
          }
          if (kc == escape) {
              printf("Sorry, %d is the CONNECT-mode escape character\n",kc);
              return(-9);
          }
          wideresult = -1;
          if (flag) {
              cmsavp(psave,PROMPTL);
              cmsetp(" Enter new definition: ");
              cmini(ckxech);
          }
        def_again:
          if (flag) prompt(NULL);
          if ((y = cmtxt("key definition,\n\
 or Ctrl-C to cancel this command,\n\
 or Enter to restore default definition",
                         "",&s,NULL)) < 0) {
              if (flag)                 /* Handle parse errors */
                goto def_again;
              else
                return(y);
          }
          s = brstrip(s);
#ifndef NOKVERBS
          p = s;                        /* Save this place */
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
                  ) {                   /* Have K */

                  if ((x == 1 && s[x-1] == CMDQ) ||
                      (x > 1 && s[x-1] == CMDQ && s[x-2] != CMDQ)) {
                      line[y++] = CMDQ; /* Make it \\K */
                  }
                  if (x > 1 && s[x-1] == '{' && s[x-2] == CMDQ) {
                      line[y-1] = CMDQ; /* Have \{K */
                      line[y++] = '{';  /* Make it \\{K */
                  }
              }
              line[y] = s[x];
          }
          line[y++] = NUL;              /* Terminate */
          s = line + y + 1;             /* Point to after it */
          x = LINBUFSIZ - (int) strlen(line) - 1; /* Get remaining space */
          if ((x < (LINBUFSIZ / 2)) ||
              (zzstring(line, &s, &x) < 0)) { /* Expand variables, etc. */
              printf("?Key definition too long\n");
              if (flag) cmsetp(psave);
              return(-9);
          }
          s = line + y + 1;             /* Point to result. */

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

          switch (strlen(s)) {          /* Action depends on length */
            case 0:                     /* Clear individual key def */
              deletekeymap(t,kc);
              break;
            case 1:
              if (!litstr) {
                  defevt.type = key;    /* Single character */
                  defevt.key.scancode = *s;
                  break;
              }
            default:                    /* Character string */
#ifndef NOKVERBS
              if (p) {
                  y = xlookup(kverbs,p,nkverbs,&x); /* Look it up */
                  /* Need exact match */
                  debug(F101,"set key kverb lookup",0,y);
                  if (y > -1) {
                      defevt.type = kverb;
                      defevt.kverb.id = y;
                      break;
                  }
              }
#endif /* NOKVERBS */
              if (litstr) {
                  defevt.type = literal;
                  defevt.literal.string = (char *) malloc(strlen(s)+1);
                  if (defevt.literal.string)
                    strcpy(defevt.literal.string, s); /* safe */
              } else {
                  defevt.type = macro;
                  defevt.macro.string = (char *) malloc(strlen(s)+1);
                  if (defevt.macro.string)
                    strcpy(defevt.macro.string, s); /* safe */
              }
              break;
          }
          insertkeymap(t, kc, defevt);
          if (flag)
            cmsetp(psave);
          initvik = 1;                  /* Update VIK table */
          return(1);
      }

#ifdef PCTERM
      case XYTPCTERM:                   /* PCTERM Keyboard Mode */
        if ((x = seton(&tt_pcterm)) < 0) return(x);
        return(success = 1);
#endif /* PCTERM */
#endif /* OS2 */

#ifdef CK_TRIGGER
      case XYTRIGGER:
        if ((y = cmtxt("String to trigger automatic return to command mode",
                       "",&s,xxstring)) < 0)
          return(y);
        makelist(s,tt_trigger,TRIGGERS);
        return(1);
#endif /* CK_TRIGGER */

#ifdef OS2
      case XYTSAC:
        if ((y = cmnum("ASCII value to use for spacing attributes",
                       "32",10,&x,xxstring)) < 0)
          return(y);
        if ((y = cmcfm()) < 0) return(y);
        tt_sac = x;
        return(success = 1);

      case XYTKBDGL: {      /* SET TERM KBD-FOLLOWS-GL/GR */
          extern int tt_kb_glgr;        /* from ckoco3.c */
          if ((x = seton(&tt_kb_glgr)) < 0)
              return(x);
          return(success = 1);
      }
#ifndef NOCSETS
      case XYTVTLNG:        /* SET TERM DEC-LANGUAGE */
        if ((y = cmkey(vtlangtab,nvtlangtab,"VT language",
                       IS97801(tt_type_mode)?"german":"north-american",
                       xxstring)) < 0)
          return(y);
        if ((x = cmcfm()) < 0) return(x);

        /* A real VT terminal would use the language to set the   */
        /* default keyboard language for both 8-bit multinational */
        /* and 7-bit national modes.  For 8-bit mode it would     */
        /* set the terminal character-set to the ISO set if it    */
        /* is not already set.                                    */
        /* Latin-1 can be replaced by DEC Multinational           */
        switch (y) {
          case VTL_NORTH_AM:  /* North American */
            /* Multinational: Latin-1   */
            /* National:      US_ASCII  */
            dec_lang = y;
            dec_nrc = TX_ASCII;
            dec_kbd = TX_8859_1;
            break;
          case VTL_BRITISH :
            /* Multinational: Latin-1   */
            /* National:      UK_ASCII  */
            dec_lang = y;
            dec_nrc = TX_BRITISH;
            dec_kbd = TX_8859_1;
            break;
          case VTL_FRENCH  :
          case VTL_BELGIAN :
          case VTL_CANADIAN:
            /* Multinational: Latin-1   */
            /* National:      FR_ASCII  */
            dec_lang = y;
            dec_nrc = TX_FRENCH;
            dec_kbd = TX_8859_1;
            break;
          case VTL_FR_CAN  :
            /* Multinational: Latin-1   */
            /* National:      FC_ASCII  */
            dec_lang = y;
            dec_nrc = TX_CN_FRENCH;
            dec_kbd = TX_8859_1;
            break;
          case VTL_DANISH  :
          case VTL_NORWEGIA:
            /* Multinational: Latin-1   */
            /* National:      NO_ASCII  */
            dec_lang = y;
            dec_nrc = TX_NORWEGIAN;
            dec_kbd = TX_8859_1;
            break;
          case VTL_FINNISH :
            /* Multinational: Latin-1   */
            /* National:      FI_ASCII  */
            dec_lang = y;
            dec_nrc = TX_FINNISH;
            dec_kbd = TX_8859_1;
            break;
          case VTL_GERMAN  :
            /* Multinational: Latin-1   */
            /* National:      GR_ASCII  */
            dec_lang = y;
            dec_nrc = TX_GERMAN;
            dec_kbd = TX_8859_1;
            break;
          case VTL_DUTCH   :
            /* Multinational: Latin-1   */
            /* National:      DU_ASCII  */
            dec_lang = y;
            dec_nrc = TX_DUTCH;
            dec_kbd = TX_8859_1;
            break;
          case VTL_ITALIAN :
            /* Multinational: Latin-1   */
            /* National:      IT_ASCII  */
            dec_lang = y;
            dec_nrc = TX_ITALIAN;
            dec_kbd = TX_8859_1;
            break;
          case VTL_SW_FR   :
          case VTL_SW_GR   :
            /* Multinational: Latin-1   */
            /* National:      CH_ASCII  */
            dec_lang = y;
            dec_nrc = TX_SWISS;
            dec_kbd = TX_8859_1;
            break;
          case VTL_SWEDISH :
            /* Multinational: Latin-1   */
            /* National:      SW_ASCII  */
            dec_lang = y;
            dec_nrc = TX_SWEDISH;
            dec_kbd = TX_8859_1;
            break;
          case VTL_SPANISH :
            /* Multinational: Latin-1   */
            /* National:      SP_ASCII  */
            dec_lang = y;
            dec_nrc = TX_SPANISH;
            dec_kbd = TX_8859_1;
            break;
          case VTL_PORTUGES:
            /* Multinational: Latin-1   */
            /* National:      Portugese ASCII  */
            dec_lang = y;
            dec_nrc = TX_PORTUGUESE;
            dec_kbd = TX_8859_1;
            break;
          case VTL_HEBREW  :
            /* Multinational: Latin-Hebrew / DEC-Hebrew  */
            /* National:      DEC 7-bit Hebrew  */
            dec_lang = y;
            dec_nrc = TX_HE7;
            dec_kbd = TX_8859_8;
            break;
          case VTL_GREEK   :
            /* Multinational: Latin-Greek / DEC-Greek   */
            /* National:      DEC Greek NRC             */
            /* is ELOT927 equivalent to DEC Greek????   */
            dec_lang = y;
            dec_nrc = TX_ELOT927;
            dec_kbd = TX_8859_7;
            break;
#ifdef COMMENT
          case VTL_TURK_Q  :
          case VTL_TURK_F  :
            /* Multinational: Latin-Turkish / DEC-Turkish   */
            /* National:      DEC 7-bit Turkish             */
            break;
#endif /* COMMENT */
          case VTL_HUNGARIA:
            /* Multinational: Latin-2   */
            /* National:      no national mode  */
            dec_lang = y;
            dec_nrc = TX_HUNGARIAN;
            dec_kbd = TX_8859_2;
            break;
          case VTL_SLOVAK  :
          case VTL_CZECH   :
          case VTL_POLISH  :
          case VTL_ROMANIAN:
            /* Multinational: Latin-2   */
            /* National:      no national mode  */
            dec_lang = y;
            dec_nrc = TX_ASCII;
            dec_kbd = TX_8859_2;
            break;
          case VTL_RUSSIAN :
            /* Multinational: Latin-Cyrillic / KOI-8   */
            /* National:      DEC Russian NRC  */
            dec_lang = y;
            dec_nrc = TX_KOI7;
            dec_kbd = TX_8859_5;
            break;
          case VTL_LATIN_AM:
            /* Multinational: not listed in table   */
            /* National:      not listed in table  */
            dec_lang = y;
            dec_nrc = TX_ASCII;
            dec_kbd = TX_8859_1;
            break;
#ifdef COMMENT
          case VTL_SCS     :
            /* Multinational: Latin-2   */
            /* National:      SCS NRC   */
            break;
#endif /* COMMENT */
          default:
            return(success = 0);
        }
        if (IS97801(tt_type_mode)) {
            SNI_bitmode(cmask == 0377 ? 8 : 7);
        }
        return(success = 1);
#endif /* NOCSETS */

      case XYTVTNRC: {                  /* SET TERM DEC-NRC-MODE */
          extern int decnrcm_usr, decnrcm;        /* from ckoco3.c */
          if ((x = seton(&decnrcm_usr)) < 0)
            return(x);
          decnrcm = decnrcm_usr;
          return(success = 1);
      }
      case XYTSNIPM: {                  /* SET TERM SNI-PAGEMODE */
          extern int sni_pagemode, sni_pagemode_usr;
          if ((y = cmkey(onoff,2,"","on",xxstring)) < 0) return(y);
          if ((x = cmcfm()) < 0) return(x);
          sni_pagemode_usr = sni_pagemode = y;
          return(success = 1);
      }
      case XYTSNISM: {                  /* SET TERM SNI-SCROLLMODE */
          extern int sni_scroll_mode, sni_scroll_mode_usr;
          if ((y = cmkey(onoff,2,"","on",xxstring)) < 0) return(y);
          if ((x = cmcfm()) < 0) return(x);
          sni_scroll_mode_usr = sni_scroll_mode = y;
          return(success = 1);
      }
      case XYTSNICC: {  /* SET TERM SNI-CH.CODE */
          extern int sni_chcode_usr;
          if ((y = cmkey(onoff,2,"","on",xxstring)) < 0) return(y);
          if ((x = cmcfm()) < 0) return(x);
          sni_chcode_usr = y;
          SNI_chcode(y);
          return(success = 1);
      }
      case XYTSNIFV: {  /* SET TERM SNI-FIRMWARE-VERSIONS */
          extern CHAR sni_kbd_firmware[], sni_term_firmware[];
          CHAR kbd[7],term[7];

          if ((x = cmfld("Keyboard Firmware Version",sni_kbd_firmware,
                         &s, xxstring)) < 0)
            return(x);
          if ((int)strlen(s) != 6) {
              printf("?Sorry - the firmware version must be 6 digits long\n");
              return(-9);
          }
          for (i = 0; i < 6; i++) {
              if (!isdigit(s[i])) {
   printf("?Sorry - the firmware version can only contain digits [0-9]\n");
                  return(-9);
              }
          }
          ckstrncpy(kbd,s,7);

          if ((x = cmfld("Terminal Firmware Version",sni_term_firmware,
                         &s, xxstring)) < 0)
            return(x);
          if ((int)strlen(s) != 6) {
              printf("?Sorry - the firmware version must be 6 digits long\n");
              return(-9);
          }
          for (i = 0; i < 6; i++) {
              if (!isdigit(s[i])) {
   printf("?Sorry - the firmware version can only contain digits [0-9]\n");
                   return(-9);
              }
          }
          ckstrncpy(term,s,7);
          if ((x = cmcfm()) < 0) return(x);

          ckstrncpy(sni_kbd_firmware,kbd,7);
          ckstrncpy(sni_term_firmware,term,7);
          return(success = 1);
    }

    case XYTLSP: {              /* SET TERM LINE-SPACING */
        if ((x = cmfld("Line Spacing","1",&s, xxstring)) < 0)
          return(x);
        if (isfloat(s,0) < 1) {		/* (sets floatval) */
            printf("?Integer or floating-point number required\n");
            return(-9);
        }
        if (floatval < 1.0 || floatval > 3.0) {
            printf("?Value must within the range 1.0 and 3.0 (inclusive)\n");
            return(-9);
        }
        if ((x = cmcfm()) < 0) return(x);
#ifdef KUI
        tt_linespacing[VCMD] = tt_linespacing[VTERM] = floatval;
        return(success = 1);
#else /* KUI */
        printf("?Sorry, Line-spacing is only supported in K95G.EXE.\n");
        return(success = 0);
#endif /* KUI */
    }
#endif /* OS2 */

      default:                          /* Shouldn't get here. */
        return(-2);
    }
#endif /* MAC */
#ifdef COMMENT
    /*
      This was supposed to shut up picky compilers but instead it makes
      most compilers complain about "statement not reached".
    */
    return(-2);
#endif /* COMMENT */
#ifdef OS2
    return(-2);
#endif /* OS2 */
}

#ifdef OS2
int
settitle(void) {
    extern char usertitle[];
    if ((y = cmtxt("title text","",&s,xxstring)) < 0)
      return(y);
#ifdef IKSD
    if (inserver) {
        printf("?Sorry, command disabled.\r\n");
        return(success = 0);
    }
#endif /* IKSD */
    s = brstrip(s);
    ckstrncpy(usertitle,s,64);
    os2settitle("",1);
    return(1);
}

static struct keytab dialertab[] = {    /* K95 Dialer types */
    "backspace",        0, 0,
    "enter",            1, 0
};
static int ndialer = 2;

int
setdialer(void) {
    int t, x, y;
    int clear = 0, deflt = 0;
    int kc;                             /* Key code */
    char *s = NULL;                     /* Key binding */
#ifndef NOKVERBS
    char *p = NULL;                     /* Worker */
#endif /* NOKVERBS */
    con_event defevt;
    extern int os2gks;
    extern int mskkeys;
    extern int initvik;

    defevt.type = error;

    if (( x = cmkey(dialertab, ndialer,
                    "Kermit-95 dialer work-arounds",
                    "", xxstring)) < 0 )
      return(x);
    switch (x) {
      case 0:                           /* Backspace */
        kc = 264;
        break;
      case 1:                           /* Enter */
        kc = 269;
        break;
      default:
        printf("Illegal value in setdialer()\n");
        return(-9);
    }
    if ((y = cmtxt("Key definition","",&s,xxstring)) < 0)
      return(y);

#ifdef IKSD
    if (inserver) {
        printf("?Sorry, command disabled.\r\n");
        return(success = 0);
    }
#endif /* IKSD */
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

    /* Clear the definition for SET KEY */
    if (macrotab[kc]) {                 /* Possibly free old macro from key. */
        free((char *)macrotab[kc]);
        macrotab[kc] = NULL;
    }
    keymap[kc] = (KEY) kc;

    /* Now reprogram the default value for all terminal types */
    /* remember to treat Wyse and Televideo terminals special */
    /* because of their use of Kverbs for Backspace and Enter */
    for (t = 0; t <= TT_MAX; t++) {
        if ( ISDG200(t) && kc == 264) {
            extern char * udkfkeys[] ;
            if (kc == 264) {            /* \Kdgbs */
                if (udkfkeys[83])
                  free(udkfkeys[83]);
                udkfkeys[83] = strdup(s);
            }
        } else if (ISWYSE(t) || ISTVI(t)) {
            extern char * udkfkeys[] ;
            if (kc == 264) {            /* \Kwybs or \Ktvibs */
                if (udkfkeys[32])
                  free(udkfkeys[32]);
                udkfkeys[32] = strdup(s);
            }
            if (kc == 269) {            /* \Kwyenter and \Kwyreturn */
                if (udkfkeys[39])       /* \Ktvienter and \Ktvireturn */
                  free(udkfkeys[39]);
                udkfkeys[39] = strdup(s);
                if (udkfkeys[49])
                  free(udkfkeys[49]);
                udkfkeys[49] = strdup(s);
            }
        } else {
            switch (strlen(s)) {        /* Action depends on length */
              case 0:                   /* Clear individual key def */
                deletekeymap(t,kc);
                break;
              case 1:
                defevt.type = key;      /* Single character */
                defevt.key.scancode = *s;
                break;
              default:                  /* Character string */
#ifndef NOKVERBS
                if (p) {
                    y = xlookup(kverbs,p,nkverbs,&x); /* Look it up */
                    /* Exact match req'd */
                    debug(F101,"set key kverb lookup",0,y);
                    if (y > -1) {
                        defevt.type = kverb;
                        defevt.kverb.id = y;
                        break;
                    }
                }
#endif /* NOKVERBS */
                defevt.type = macro;
                defevt.macro.string = (char *) malloc(strlen(s)+1);
                if (defevt.macro.string)
                  strcpy(defevt.macro.string, s); /* safe */
                break;
            }
            insertkeymap( t, kc, defevt ) ;
            initvik = 1;                /* Update VIK table */
        }
    }
    return(1);
}
#endif /* OS2 */

#ifdef NT
int
setwin95( void ) {
    int x, y, z;

    if (( y = cmkey(win95tab, nwin95,
                    "Windows 95 specific work-arounds",
                    "keyboard-translation",
                    xxstring)) < 0 )
        return (y);
    switch (y) {
      case XYWPOPUP:
        if ((y = cmkey(onoff,2,"popups are used to prompt the user for data",
                       "on",xxstring)) < 0)
          return(y);
        if ((x = cmcfm()) < 0) return(x);
        win95_popup = y;
        return(1);

      case XYW8_3:
        if ((y = cmkey(onoff,2,"8.3 FAT file names","off",xxstring)) < 0)
          return(y);
        if ((x = cmcfm()) < 0) return(x);
        win95_8_3 = y;
        return(1);

      case XYWSELECT:
        if ((y = cmkey(onoff,2,"\"select()\" fails on write","off",
             xxstring)) < 0)
          return(y);
        if ((x = cmcfm()) < 0) return(x);
        win95selectbug = y;
        return(1);

      case XYWAGR:
        if ((y = cmkey(onoff,2,"Right-Alt is Alt-Gr","off",xxstring)) < 0)
          return(y);
        if ((x = cmcfm()) < 0) return(x);
        win95altgr = y;
        return(1);

      case XYWOIO:
        if ((y = cmkey(onoff,2,"Use Overlapped I/O","on",xxstring)) < 0)
          return(y);
        if (y) {
            if ((x = cmnum("Maximum number of outstanding I/O requests",
                           "10",10,&z,xxstring)) < 0)
              return(x);
            if (z < 1 || z > 7) {
                printf(
"?Maximum outstanding I/O requests must be between 1 and 7.\n");
                return(-9);
            }
        } else
          z = 1;
        if ((x = cmcfm()) < 0) return(x);
        owwait = !y;
        maxow = maxow_usr = z;
        return(1);

      case XYWKEY:
#ifndef COMMENT
        printf("\n?\"Keyboard-Translation\" is no longer required.\n");
        return(-9);
#else /* COMMENT */
        if (( z = cmkey(tcstab, ntcs,
                        "Keyboard Character Set",
                        "latin1-iso",
                        xxstring)) < 0)
          return (z);
        if ((x = cmcfm()) < 0)
          return(x);

        win95kcsi = z;
        win95kl2 = (win95kcsi == TC_2LATIN);

        if (win95kcsi == TC_TRANSP) {
            win95kcs = NULL;
        } else {
#ifdef UNICODE
            win95kcs = xlr[win95kcsi][tx2fc(tcsl)];
#else /* UNICODE */
            win95kcs = xlr[win95kcsi][tcsl];
#endif /* UNICODE */
        }
        return(1);
#endif /* COMMENT */

      case XYWLUC:
        if ((y = cmkey(onoff,2,"Unicode-to-Lucida-Console substitutions",
                       "on",xxstring)) < 0)
          return(y);
        if ((x = cmcfm()) < 0) return(x);
        win95lucida = y;
        return(1);

      case XYWHSL:
	if ((y = cmkey(onoff,2,"Horizontal Scan Line substitutions",
		       "on",xxstring)) < 0)
	  return(y);
	if ((x = cmcfm()) < 0) return(x);
	win95hsl = y;
	return(1);

      default:
        printf("Illegal value in setwin95()\n");
        return(-9);
    }
}
#endif /* NT */

#ifdef OS2
int
setprty (
#ifdef CK_ANSIC
    void
#endif /* CK_ANSIC */
/* setprty */ ) {
    int x, y, z;

    if (( y = cmkey(prtytab, nprty,
                    "priority level of terminal and communication threads",
                    "foreground-server",
                    xxstring)) < 0 )
      return (y);

    if ((x = cmcfm()) < 0)
      return (x);
#ifdef IKSD
    if (inserver &&
#ifdef IKSDCONF
         iksdcf
#else
         1
#endif /* IKSDCONF */
    ) {
        if ((y = cmcfm()) < 0) return(y);
        printf("?Sorry, command disabled.\r\n");
        return(success = 0);
    }
#endif /* IKSD */
    priority = y;
    return(TRUE);
}
#endif /* OS2 */

int
setbell() {
    int y, x;
#ifdef OS2
    int z;
#endif /* OS2 */

    if ((y = cmkey(beltab,nbeltab,
#ifdef OS2
        "how console and terminal bells should\nbe generated", "audible",
#else
        "Whether Kermit should ring the terminal bell (beep)", "on",
#endif /* OS2 */
                   xxstring)) < 0)
          return(y);

#ifdef IKSD
    if (inserver) {
        if ((y = cmcfm()) < 0) return(y);
        printf("?Sorry, command disabled.\r\n");
        return(success = 0);
    }
#endif /* IKSD */

    switch (y) {                        /* SET BELL */
      case XYB_NONE:
#ifdef OS2
      case XYB_VIS:
#endif /* OS2 */
        if ((x = cmcfm()) < 0)
          return(x);
#ifdef OS2
        tt_bell = y;
#else
        tt_bell = 0;
#endif /* OS2 */
        break;

      case XYB_AUD:
#ifdef OS2
        if ((x = cmkey(audibletab, naudibletab,
               "how audible console and terminal\nbells should be generated",
                       "beep",xxstring))<0)
          return(x);
        if ((z = cmcfm()) < 0)
          return(z);
        tt_bell = y | x;
#else
        /* This lets C-Kermit accept but ignore trailing K95 keywords */
        if ((x = cmtxt("Confirm with carriage return","",&s,xxstring)) < 0)
          return(x);
        tt_bell = 1;
#endif /* OS2 */
        break;
    }
    return(1);
}

#ifdef OS2MOUSE
int
setmou(
#ifdef CK_ANSIC
       void
#endif /* CK_ANSIC */
 /* setmou */ ) {
    extern int initvik;
    int button = 0, event = 0;
    char * p;

    if ((y = cmkey(mousetab,nmtab,"","",xxstring)) < 0)
      return(y);

#ifdef IKSD
    if (inserver) {
        if ((y = cmcfm()) < 0) return(y);
        printf("?Sorry, command disabled.\r\n");
        return(success = 0);
    }
#endif /* IKSD */

    if (y == XYM_ON) {                  /* MOUSE ACTIVATION */
        int old_mou = tt_mouse;
        if ((x = seton(&tt_mouse)) < 0)
            return(x);
        if (tt_mouse != old_mou)
          if (tt_mouse)
            os2_mouseon();
          else
            os2_mouseoff();
        return(1);
    }

    if (y == XYM_DEBUG) {               /* MOUSE DEBUG */
        extern int MouseDebug;
        if ((x = seton(&MouseDebug)) < 0)
            return(x);
        return(1);
    }

    if (y == XYM_CLEAR) {               /* Reset Mouse Defaults */
        if ((x = cmcfm()) < 0) return(x);
        mousemapinit(-1,-1);
        initvik = 1;                    /* Update VIK Table */
        return 1;
    }
    if (y != XYM_BUTTON) {              /* Shouldn't happen. */
        printf("Internal parsing error\n");
        return(-9);
    }

    /* MOUSE EVENT ... */

    if ((button = cmkey(mousebuttontab,nmbtab,
                        "Button number","1",
                        xxstring)) < 0)
      return(button);

    if ((y =  cmkey(mousemodtab,nmmtab,
                    "Keyboard modifier","none",
                    xxstring)) < 0)
      return(y);

    event |= y;                         /* OR in the bits */

    if ((y =  cmkey(mclicktab,nmctab,"","click",xxstring)) < 0)
      return(y);

    /* Two bits are assigned, if neither are set then it is button one */

    event |= y;                 /* OR in the bit */

    wideresult = -1;

    if ((y = cmtxt("definition,\n\
or Ctrl-C to cancel this command,\n\
or Enter to restore default definition",
                   "",&s,NULL)) < 0) {
        return(y);
    }
    s = brstrip(s);
    p = s;                              /* Save this place */
/*
  If the definition included any \Kverbs, quote the backslash so the \Kverb
  will still be in the definition when the key is pressed.  We don't do this
  in zzstring(), because \Kverbs are valid only in this context and nowhere
  else.  This code copied from SET KEY, q.v. for addt'l commentary.
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
#else
    p = NULL;
#endif /* NOKVERBS */

    /* free the old definition if necessary */
    if (mousemap[button][event].type == macro) {
        free( mousemap[button][event].macro.string);
        mousemap[button][event].macro.string = NULL;
    }
    switch (strlen(s)) {                /* Action depends on length */
      case 0:                           /* Reset to default binding */
        mousemapinit( button, event );
        break;
      case 1:                           /* Single character */
            mousemap[button][event].type = key;
        mousemap[button][event].key.scancode = *s;
        break;
      default:                          /* Character string */
#ifndef NOKVERBS
        if (p) {
            y = xlookup(kverbs,p,nkverbs,&x); /* Look it up */
            debug(F101,"set mouse kverb lookup",0,y); /* need exact match */
            if (y > -1) {
            /* Assign the kverb to the event */
            mousemap[button][event].type = kverb;
            mousemap[button][event].kverb.id = F_KVERB | y;
            break;
            }
        }
#endif /* NOKVERBS */

       /* Otherwise, it's a macro, so assign the macro to the event */
       mousemap[button][event].type = macro;
       mousemap[button][event].macro.string = (MACRO) malloc(strlen(s)+1);
       if (mousemap[button][event].macro.string)
         strcpy((char *) mousemap[button][event].macro.string, s); /* safe */
        break;
    }
    initvik = 1;                        /* Update VIK Table */
    if ( (button == XYM_B3) && (mousebuttoncount() < 3) && !quiet )
    {
        printf("?Warning: this machine does not have a three button mouse.\n");
        return(0);
    }
    return(1);
}
#endif /* OS2MOUSE */
#endif /* NOLOCAL */

#ifndef NOXFER
int                                     /* SET SEND/RECEIVE */
setsr(xx, rmsflg) int xx; int rmsflg; {
    if (xx == XYRECV)
      ckstrncpy(line,"Parameter for inbound packets",LINBUFSIZ);
    else
      ckstrncpy(line,"Parameter for outbound packets",LINBUFSIZ);

    if (rmsflg) {
        if ((y = cmkey(rsrtab,nrsrtab,line,"",xxstring)) < 0) {
            if (y == -3) {
                printf("?Remote receive parameter required\n");
                return(-9);
            } else return(y);
        }
    } else {
        if ((y = cmkey(srtab,nsrtab,line,"",xxstring)) < 0) return(y);
    }
    switch (y) {
      case XYQCTL:                      /* CONTROL-PREFIX */
        if ((x = cmnum("ASCII value of control prefix","",10,&y,xxstring)) < 0)
          return(x);
        if ((x = cmcfm()) < 0) return(x);
        if ((y > 32 && y < 63) || (y > 95 && y < 127)) {
            if (xx == XYRECV)
              ctlq = (CHAR) y;          /* RECEIVE prefix, use with caution! */
            else
              myctlq = (CHAR) y;        /* SEND prefix, OK to change */
            return(success = 1);
        } else {
            printf("?Illegal value for prefix character\n");
            return(-9);
        }

      case XYEOL:
        if ((y = setcc("13",&z)) < 0)
            return(y);
        if (z > 31) {
            printf("Sorry, the legal values are 0-31\n");
            return(-9);
        }
        if (xx == XYRECV)
          eol = (CHAR) z;
        else
          seol = (CHAR) z;
        return(success = y);

      case XYLEN:
        y = cmnum("Maximum number of characters in a packet","90",10,&x,
                  xxstring);
        if (xx == XYRECV) {             /* Receive... */
            if ((y = setnum(&z,x,y,maxrps)) < 0)
              return(y);
            if (protocol != PROTO_K) {
                printf("?Sorry, this command does not apply to %s protocol.\n",
                       ptab[protocol].p_name
                       );
                printf("Use SET SEND PACKET-LENGTH for XYZMODEM\n");
                return(-9);
            }
            if (z < 10) {
                printf("Sorry, 10 is the minimum\n");
                return(-9);
            }
            if (rmsflg) {
                sstate = setgen('S', "401", ckitoa(z), "");
                return((int) sstate);
            } else {
                if (protocol == PROTO_K) {
                    if (z > MAXRP) z = MAXRP;
                    y = adjpkl(z,wslotr,bigrbsiz);
                    if (y != z) {
                        urpsiz = y;
                        if (!xcmdsrc)
                          if (msgflg) printf(
" Adjusting receive packet-length to %d for %d window slots\n",
                                             y, wslotr);
                    }
                    urpsiz = y;
                    ptab[protocol].rpktlen = urpsiz;
                    rpsiz =  (y > 94) ? 94 : y;
                } else {
#ifdef CK_XYZ
                    if ((protocol == PROTO_X || protocol == PROTO_XC) &&
                         z != 128 && z != 1024) {
                        printf("Sorry, bad packet length for XMODEM.\n");
                        printf("Please use 128 or 1024.\n");
                        return(-9);
                    }
#endif /* CK_XYZ */
                    urpsiz = rpsiz = z;
                }
            }
        } else {                        /* Send... */
            if ((y = setnum(&z,x,y,maxsps)) < 0)
              return(y);
            if (z < 10) {
                printf("Sorry, 10 is the minimum\n");
                return(-9);
            }
            if (protocol == PROTO_K) {
                if (z > MAXSP) z = MAXSP;
                spsiz = z;              /* Set it */
                y = adjpkl(spsiz,wslotr,bigsbsiz);
                if (y != spsiz && !xcmdsrc)
                  if (msgflg)
                    printf("Adjusting packet size to %d for %d window slots\n",
                           y,wslotr);
            } else
              y = z;
#ifdef CK_XYZ
            if ((protocol == PROTO_X || protocol == PROTO_XC) &&
                 z != 128 && z != 1024) {
                printf("Sorry, bad packet length for XMODEM.\n");
                printf("Please use 128 or 1024.\n");
                return(-9);
            }
#endif /* CK_XYZ */
            spsiz = spmax = spsizr = y; /* Set it and flag that it was set */
            spsizf = 1;                 /* to allow overriding Send-Init. */
            ptab[protocol].spktflg = spsizf;
            ptab[protocol].spktlen = spsiz;
        }
        if (pflag && protocol == PROTO_K && !xcmdsrc) {
            if (z > 94 && !reliable && msgflg) {
                /* printf("Extended-length packets requested.\n"); */
                if (bctr < 2 && z > 200) printf("\
Remember to SET BLOCK 2 or 3 for long packets.\n");
            }
            if (speed <= 0L) speed = ttgspd();
#ifdef COMMENT
/*
  Kermit does this now itself.
*/
            if (speed <= 0L && z > 200 && msgflg) {
                printf("\
Make sure your timeout interval is long enough for %d-byte packets.\n",z);
            }
#endif /* COMMENT */
        }
        return(success = y);

      case XYMARK:
#ifdef DOOMSDAY
/*
  Printable start-of-packet works for UNIX and VMS only!
*/
        x_ifnum = 1;
        y = cmnum("Code for packet-start character","1",10,&x,xxstring);
        x_ifnum = 0;
        if ((y = setnum(&z,x,y,126)) < 0) return(y);
#else
        if ((y = setcc("1",&z)) < 0)
            return(y);
#endif /* DOOMSDAY */
        if (xx == XYRECV)
          stchr = (CHAR) z;
        else {
            mystch = (CHAR) z;
#ifdef IKS_OPTION
            /* If IKS negotiation in use   */
            if (TELOPT_U(TELOPT_KERMIT) || TELOPT_ME(TELOPT_KERMIT))
              tn_siks(KERMIT_SOP);      /* Report change to other side */
#endif /* IKS_OPTION */
        }
        return(success = y);

      case XYNPAD:                      /* PADDING */
        y = cmnum("How many padding characters for inbound packets","0",10,&x,
                  xxstring);
        if ((y = setnum(&z,x,y,94)) < 0) return(y);
        if (xx == XYRECV)
          mypadn = (CHAR) z;
        else
          npad = (CHAR) z;
        return(success = y);

      case XYPADC:                      /* PAD-CHARACTER */
        if ((y = setcc("0",&z)) < 0) return(y);
        if (xx == XYRECV) mypadc = z; else padch = z;
        return(success = y);

      case XYTIMO:                      /* TIMEOUT */
        if (xx == XYRECV) {
            y = cmnum("Packet timeout interval",ckitoa(URTIME),10,&x,xxstring);
            if ((y = setnum(&z,x,y,94)) < 0) return(y);

            if (rmsflg) {               /* REMOTE SET RECEIVE TIMEOUT */
                sstate = setgen('S', "402", ckitoa(z), "");
                return((int) sstate);
            } else {                    /* SET RECEIVE TIMEOUT */
                pkttim = z;             /*   Value to put in my negotiation */
            }                           /*   packet for other Kermit to use */

        } else {                        /* SET SEND TIMEOUT */
#ifdef CK_TIMERS
            extern int rttflg, mintime, maxtime;
            int tmin = 0, tmax = 0;
#endif /* CK_TIMERS */
            y = cmnum("Packet timeout interval",ckitoa(DMYTIM),10,&x,xxstring);
            if (y == -3) {              /* They cancelled a previous */
                x = DMYTIM;             /* SET SEND command, so restore */
                timef = 0;              /* and turn off the override flag */
                y = cmcfm();
            }
#ifdef CK_TIMERS
            if (y < 0) return(y);
            if (x < 0) {
                printf("?Out of range - %d\n",x);
                return(-9);
            }
            if ((z = cmkey(timotab,2,"","dynamic",xxstring)) < 0) return(z);
            if (z) {
                if ((y = cmnum("Minimum timeout to allow",
                               "1",10,&tmin,xxstring)) < 0)
                  return(y);
                if (tmin < 1) {
                    printf("?Out of range - %d\n",tmin);
                    return(-9);
                }
                if ((y = cmnum("Maximum timeout to allow",
                               "0",10,&tmax,xxstring)) < 0)
                  return(y);
                /* 0 means let Kermit choose, < 0 means no maximum */
            }
            if ((y = cmcfm()) < 0)
              return(y);
            rttflg = z;                 /* Round-trip timer flag */
            z = x;
#else
            if ((y = setnum(&z,x,y,94)) < 0)
              return(y);
#endif /* CK_TIMERS */
            timef = 1;                  /* Turn on the override flag */
            timint = rtimo = z;         /* Override value for me to use */
#ifdef CK_TIMERS
            if (rttflg) {               /* Lower and upper bounds */
                mintime = tmin;
                maxtime = tmax;
            }
#endif /* CK_TIMERS */
        }
        return(success = 1);

      case XYFPATH:                     /* PATHNAMES */
        if (xx == XYRECV) {
            y = cmkey(rpathtab,nrpathtab,"","auto",xxstring);
        } else {
            y = cmkey(pathtab,npathtab,"","off",xxstring);
        }
        if (y < 0) return(y);

        if ((x = cmcfm()) < 0) return(x);
        if (xx == XYRECV) {             /* SET RECEIVE PATHNAMES */
            fnrpath = y;
            ptab[protocol].fnrp = fnrpath;
        } else {                        /* SET SEND PATHNAMES */
            fnspath = y;
            ptab[protocol].fnsp = fnspath;
        }
        return(success = 1);            /* Note: 0 = ON, 1 = OFF */
        /* In other words, ON = leave pathnames ON, OFF = take them off. */

      case XYPAUS:                      /* SET SEND/RECEIVE PAUSE */
        y = cmnum("Milliseconds to pause between packets","0",10,&x,xxstring);
        if ((y = setnum(&z,x,y,15000)) < 0)
          return(y);
        pktpaus = z;
        return(success = 1);

#ifdef CKXXCHAR                         /* SET SEND/RECEIVE IGNORE/DOUBLE */
      case XYIGN:
      case XYDBL: {
          int i, zz;
          short *p;
          extern short dblt[];
          extern int dblflag, ignflag;

          /* Make space for a temporary copy of the ignore/double table */

          zz = y;
#ifdef COMMENT
          if (zz == XYIGN && xx == XYSEND) {
              blah blah who cares
          }
          if (zz == XYDBL && xx == XYRECV) {
              blah blah
          }
#endif /* COMMENT */
          p = (short *)malloc(256 * sizeof(short));
          if (!p) {
              printf("?Internal error - malloc failure\n");
              return(-9);
          }
          for (i = 0; i < 256; i++) p[i] = dblt[i]; /* Copy current table */

          while (1) {                   /* Collect a list of numbers */
#ifndef NOSPL
              x_ifnum = 1;              /* Turn off complaints from eval() */
#endif /* NOSPL */
              if ((x = cmnum(zz == XYDBL ?
                             "Character to double" :
                             "Character to ignore",
                             "",10,&y,xxstring
                             )) < 0) {
#ifndef NOSPL
                  x_ifnum = 0;
#endif /* NOSPL */
                  if (x == -3)          /* Done */
                    break;
                  if (x == -2) {
                      if (p) { free(p); p = NULL; }
                      debug(F110,"SET S/R DOUBLE/IGNORE atmbuf",atmbuf,0);
                      if (!ckstrcmp(atmbuf,"none",4,0) ||
                          !ckstrcmp(atmbuf,"non",3,0) ||
                          !ckstrcmp(atmbuf,"no",2,0) ||
                          !ckstrcmp(atmbuf,"n",1,0)) {
                          if ((x = cmcfm()) < 0) /* Get confirmation */
                            return(x);
                          for (y = 0; y < 256; y++)
                            dblt[y] &= (zz == XYDBL) ? 1 : 2;
                          if (zz == XYDBL) dblflag = 0;
                          if (zz == XYIGN) ignflag = 0;
                          return(success = 1);
                      } else {
                          printf(
                            "?Please specify a number or the word NONE\n");
                          return(-9);
                      }
                  } else {
                      free(p);
                      p = NULL;
                      return(x);
                  }
              }
#ifndef NOSPL
              x_ifnum = 0;
#endif /* NOSPL */
              if (y < 0 || y > 255) {
                  printf("?Please enter a character code in range 0-255\n");
                  free(p);
                  p = NULL;
                  return(-9);
              }
              p[y] |= (zz == XYDBL) ? 2 : 1;
              if (zz == XYDBL) dblflag = 1;
              if (zz == XYIGN) ignflag = 1;
          } /* End of while loop */

          if ((x = cmcfm()) < 0) return(x);
/*
  Get here only if they have made no mistakes.  Copy temporary table back to
  permanent one, then free temporary table and return successfully.
*/
          if (p) {
              for (i = 0; i < 256; i++) dblt[i] = p[i];
              free(p);
              p = NULL;
          }
          return(success = 1);
      }
#endif /* CKXXCHAR */

#ifdef PIPESEND
      case XYFLTR: {                    /* SET { SEND, RECEIVE } FILTER */
          if ((y = cmtxt((xx == XYSEND) ?
                "Filter program for sending files -\n\
 use \\v(filename) to substitute filename" :
                "Filter program for receiving files -\n\
 use \\v(filename) to substitute filename",
                         "",&s,NULL)) < 0)
            return(y);
          if (!*s) {                    /* Removing a filter... */
              if (xx == XYSEND && sndfilter) {
                  makestr(&g_sfilter,NULL);
                  makestr(&sndfilter,NULL);
              } else if (rcvfilter) {
                  makestr(&g_rfilter,NULL);
                  makestr(&rcvfilter,NULL);
              }
              return(success = 1);
          }                             /* Adding a filter... */
          s = brstrip(s);               /* Strip any braces */
          y = strlen(s);
          if (xx == XYSEND) {           /* For SEND filter... */
              for (x = 0; x < y; x++) { /* make sure they included "\v(...)" */
                  if (s[x] != '\\') continue;
                  if (s[x+1] == 'v') break;
              }
              if (x == y) {
                  printf(
              "?Filter must contain a replacement variable for filename.\n"
                         );
                  return(-9);
              }
          }
          if (xx == XYSEND) {
              makestr(&sndfilter,s);
              makestr(&g_sfilter,s);
          } else {
              makestr(&rcvfilter,s);
              makestr(&g_rfilter,s);
          }
          return(success = 1);
      }
#endif /* PIPESEND */

      case XYINIL:
        y = cmnum("Max length for protocol init string","-1",10,&x,xxstring);
        if ((y = setnum(&z,x,y,-1)) < 0)
          return(y);
        if (xx == XYSEND)
          sprmlen = z;
        else
          rprmlen = z;
        return(success = 1);

      case 993: {
          extern int sendipkts;
          if (xx == XYSEND) {
              if ((x = seton(&sendipkts)) < 0)
                return(x);
          }
          return(1);
      }
#ifdef CK_PERMS
      case 994:
	switch(xx) {
	  case XYSEND:
	    if ((x = seton(&atlpro)) < 0) return(x);
	    atgpro = atlpro;
	    return(1);
	  case XYRECV:
	    if ((x = seton(&atlpri)) < 0) return(x);
	    atgpri = atlpri;
	    return(1);
	  default:
	    return(-2);
	}
#endif /* CK_PERMS */

#ifndef NOCSETS
      case XYCSET: {                    /* CHARACTER-SET-SELECTION */
          extern struct keytab xfrmtab[];
          extern int r_cset, s_cset;
          if ((y = cmkey(xfrmtab,2,"","automatic",xxstring)) < 0)
            return(y);
          if ((x = cmcfm()) < 0)
            return(x);
          if (xx == XYSEND)
            s_cset = y;
          else
            r_cset = y;
          return(success = 1);
      }
#endif /* NOCSETS */

      case XYBUP:
        if ((y = cmkey(onoff,2,"","on",xxstring)) < 0)
          return(y);
        if ((x = cmcfm()) < 0) return(x);
        if (xx == XYSEND) {
            extern int skipbup;
            skipbup = (y == 0) ? 1 : 0;
            return(success = 1);
        } else {
            printf(
"?Please use SET FILE COLLISION to choose the desired action\n");
            return(-9);
        }

      case XYMOVE:
#ifdef COMMENT
        y = cmdir("Directory to move file(s) to after successful transfer",
                  "",&s,xxstring);
#else
        y = cmtxt("Directory to move file(s) to after successful transfer",
		  "",&s,xxstring);
#endif /* COMMENT */

        if (y < 0 && y != -3)
          return(y);
        ckstrncpy(line,s,LINBUFSIZ);
        s = brstrip(line);

#ifdef COMMENT
	/* Only needed for cmdir() */
        if ((x = cmcfm()) < 0)
          return(x);
#endif /* COMMENT */
	
	/* Check directory existence if absolute */
	/* THIS MEANS IT CAN'T INCLUDE ANY DEFERRED VARIABLES! */
	if (s) if (*s) {
	    if (isabsolute(s) && !isdir(s)) {
		printf("?Directory does not exist - %s\n",s);
		return(-9);
	    }
	}
        if (xx == XYSEND) {
            if (*s) {
#ifdef COMMENT
		/* Allow it to be relative */
                zfnqfp(s,LINBUFSIZ,line);
#endif /* COMMENT */
                makestr(&snd_move,line);
                makestr(&g_snd_move,line);
            } else {
                makestr(&snd_move,NULL);
                makestr(&g_snd_move,NULL);
            }
        } else {
            if (*s) {
#ifdef COMMENT
		/* Allow it to be relative */
                zfnqfp(s,LINBUFSIZ,line);
#endif /* COMMENT */
                makestr(&rcv_move,line);
                makestr(&g_rcv_move,line);
            } else {
                makestr(&rcv_move,NULL);
                makestr(&g_rcv_move,NULL);
            }
        }
        return(success = 1);

      case XYRENAME:
        y = cmtxt("Template to rename file(s) to after successful transfer",
                  "",&s,NULL);		/* NOTE: no xxstring */
        if (y < 0 && y != -3)		/* Evaluation is deferred */
          return(y);
        ckstrncpy(line,s,LINBUFSIZ);
        s = brstrip(line);
        if ((x = cmcfm()) < 0)
          return(x);
        if (xx == XYSEND) {
            if (*s) {
                makestr(&snd_rename,s);
                makestr(&g_snd_rename,s);
            } else {
                makestr(&snd_rename,NULL);
                makestr(&g_snd_rename,NULL);
            }
        } else {
            if (*s) {
                makestr(&rcv_rename,s);
                makestr(&g_rcv_rename,s);
            } else {
                makestr(&rcv_rename,NULL);
                makestr(&g_rcv_rename,NULL);
            }
        }
        return(success = 1);

#ifdef VMS
      case 887:				/* VERSION-NUMBERS */
        if (xx == XYSEND) {
            extern int vmssversions;
            return(seton(&vmssversions));
        } else {
            extern int vmsrversions;
            return(seton(&vmsrversions));
        }
#endif /* VMS */

      default:
        return(-2);
    }                                   /* End of SET SEND/RECEIVE... */
}
#endif /* NOXFER */

#ifndef NOXMIT
int
setxmit() {
    if ((y = cmkey(xmitab,nxmit,"","",xxstring)) < 0) return(y);
    switch (y) {
      case XMITE:                       /* EOF */
        y = cmtxt("Characters to send at end of file,\n\
 Use backslash codes for control characters","",&s,xxstring);
        if (y < 0) return(y);
        if ((int)strlen(s) > XMBUFL) {
            printf("?Too many characters, %d maximum\n",XMBUFL);
            return(-2);
        }
        ckstrncpy(xmitbuf,s,XMBUFL);
        return(success = 1);

      case XMITF:                       /* Fill */
        y = cmnum("Numeric code for blank-line fill character","0",10,&x,
                  xxstring);
        if ((y = setnum(&z,x,y,127)) < 0) return(y);
        xmitf = z;
        return(success = 1);
      case XMITL:                       /* Linefeed */
        return(seton(&xmitl));
      case XMITS:                       /* Locking-Shift */
        return(seton(&xmits));
      case XMITP:                       /* Prompt */
        y = cmnum("Numeric code for host's prompt character, 0 for none",
                  "10",10,&x,xxstring);
        if ((y = setnum(&z,x,y,127)) < 0) return(y);
        xmitp = z;
        return(success = 1);
      case XMITX:                       /* Echo */
        return(seton(&xmitx));
      case XMITW:                       /* Pause */
        y = cmnum("Number of milliseconds to pause between binary characters\n\
or text lines during transmission","0",10,&x,xxstring);
        if ((y = setnum(&z,x,y,1000)) < 0) return(y);
        xmitw = z;
        return(success = 1);
      case XMITT:                       /* Timeout */
        y = cmnum("Seconds to wait for each character to echo",
                  "1",10,&x,xxstring);
        if ((y = setnum(&z,x,y,1000)) < 0) return(y);
        xmitt = z;
        return(success = 1);
      default:
        return(-2);
    }
}
#endif /* NOXMIT */

#ifndef NOXFER
/*  D O R M T  --  Do a remote command  */

VOID
rmsg() {
    if (pflag && !quiet && fdispla != XYFD_N)
      printf(
#ifdef CK_NEED_SIG
       " Type your escape character, %s, followed by X or E to cancel.\n",
       dbchr(escape)
#else
       " Press the X or E key to cancel.\n"
#endif /* CK_NEED_SIG */
      );
}

static int xzcmd = 0;                   /* Global copy of REMOTE cmd index */

/*  R E M C F M  --  Confirm a REMOTE command  */
/*
  Like cmcfm(), but allows for a redirection indicator on the end,
  like "> filename" or "| command".  Returns what cmcfm() would have
  returned: -1 if reparse needed, etc etc blah blah.  On success,
  returns 1 with:

    char * remdest containing the name of the file or command.
    int remfile set to 1 if there is to be any redirection.
    int remappd set to 1 if output file is to be appended to.
    int rempipe set to 1 if remdest is a command, 0 if it is a file.
*/
static int
remcfm() {
    int x;
    char *s;
    char c;

    remfile = 0;
    rempipe = 0;
    remappd = 0;

    if ((x = cmtxt(
             "> filename, | command,\n\
or type carriage return to confirm the command",
                   "",&s,xxstring)) < 0)
      return(x);
    if (remdest) {
        free(remdest);
        remdest = NULL;
    }
    debug(F101,"remcfm local","",local);
    debug(F110,"remcfm s",s,0);
    debug(F101,"remcfm cmd","",xzcmd);
/* 
  This check was added in C-Kermit 6.0 or 7.0 but it turns out to be
  unhelpful in the situation where the remote is running a script that sends
  REMOTE commands to the local workstation.  What happens is, the local
  server executes the command and sends the result back as screen text, which
  is indicated by using an X packet instead of an F packet as the file
  header.  There are two parts to this: executing the command under control
  of the remote Kermit, which is desirable (and in fact some big applications
  depend on it, and therefore never installed any new C-Kermit versions after
  5A), and displaying the result.  Commenting out the check allows the
  command to be executed, but the result is still sent back to the remote in
  a file transfer, where it vanishes into the ether.  Actually it's on the
  communication connection, mixed in with the packets.  Pretty amazing that
  the file transfer still works, right?
*/
#ifdef COMMENT
    if (!*s) {                          /* No redirection indicator */
        if (!local &&
            (xzcmd == XZDIR || xzcmd == XZTYP ||
             xzcmd == XZXIT || xzcmd == XZSPA ||
             xzcmd == XZHLP || xzcmd == XZPWD ||
             xzcmd == XZLGI || xzcmd == XZLGO ||
             xzcmd == XZWHO || xzcmd == XZHOS)) {
            printf("?\"%s\" has no effect in remote mode\n",cmdbuf);
            return(-9);
        } else
          return(1);
    }
#endif	/* COMMENT */
    c = *s;                             /* We have something */
    if (c != '>' && c != '|') {         /* Is it > or | ? */
        printf("?Not confirmed\n");     /* No */
        return(-9);
    }
    s++;                                /* See what follows */
    if (c == '>' && *s == '>') {        /* Allow for ">>" too */
        s++;
        remappd = 1;                    /* Append to output file */
    }
    while (*s == SP || *s == HT) s++;   /* Strip intervening whitespace */
    if (!*s) {
        printf("?%s missing\n", c == '>' ? "Filename" : "Command");
        return(-9);
    }
    if (c == '>' && zchko(s) < 0) {     /* Check accessibility */
        printf("?Access denied - %s\n", s);
        return(-9);
    }
    remfile = 1;                        /* Set global results */
    rempipe = (c == '|');
    if (rempipe
#ifndef NOPUSH
        && nopush
#endif /* NOPUSH */
        ) {
        printf("?Sorry, access to external commands is disabled.\n");
        return(-9);
    }
    makestr(&remdest,s);
#ifndef NODEBUG
    if (deblog) {
        debug(F101,"remcfm remfile","",remfile);
        debug(F101,"remcfm remappd","",remappd);
        debug(F101,"remcfm rempipe","",rempipe);
        debug(F110,"remcfm remdest",remdest, 0);
    }
#endif /* NODEBUG */
    return(1);
}

/*  R E M T X T  --  Like remcfm()...  */
/*
   ... but for REMOTE commands that end with cmtxt().
   Here we must decipher braces to discover whether the trailing
   redirection indicator is intended for local use, or to be sent out
   to the server, as in:

     remote host blah blah > file                 This end
     remote host { blah blah } > file             This end
     remote host { blah blah > file }             That end
     remote host { blah blah > file } > file      Both ends

   Pipes too:

     remote host blah blah | cmd                  This end
     remote host { blah blah } | cmd              This end
     remote host { blah blah | cmd }              That end
     remote host { blah blah | cmd } | cmd        Both ends

   Or both:

     remote host blah blah | cmd > file           This end, etc etc...

   Note: this really only makes sense for REMOTE HOST, but why be picky?
   Call after calling cmtxt(), with pointer to string that cmtxt() parsed,
   as in "remtxt(&s);".

   Returns:
    1 on success with braces & redirection things removed & pointer updated,
   -9 on failure (bad indirection), after printing error message.
*/
int
remtxt(p) char ** p; {
    int i, x, bpos, ppos;
    char c, *s, *q;

    remfile = 0;                        /* Initialize global results */
    rempipe = 0;
    remappd = 0;
    if (remdest) {
        free(remdest);
        remdest = NULL;
    }
    s = *p;
    if (!s)                             /* No redirection indicator */
      s = "";
#ifdef COMMENT
    if (!*s) {                          /* Ditto */
        if (!local &&
            (xzcmd == XZDIR || xzcmd == XZTYP ||
             xzcmd == XZXIT || xzcmd == XZSPA ||
             xzcmd == XZHLP || xzcmd == XZPWD ||
             xzcmd == XZLGI || xzcmd == XZLGO ||
             xzcmd == XZWHO || xzcmd == XZHOS)) {
            printf("?\"%s\" has no effect in remote mode\n",cmdbuf);
            if (hints) {
                printf("Hint: Try again with an output redirector.\n");
            }
            return(-9);
        } else
          return(1);
    }
#endif	/* COMMENT */
    bpos = -1;                          /* Position of > (bracket) */
    ppos = -1;                          /* Position of | (pipe) */
    x = strlen(s);                      /* Length of cmtxt() string */

    for (i = x-1; i >= 0; i--) {        /* Search right to left. */
        c = s[i];
        if (c == '}')                   /* Break on first right brace */
          break;                        /* Don't look at contents of braces */
        else if (c == '>')              /* Record position of > */
          bpos = i;
        else if (c == '|')              /* and of | */
          ppos = i;
    }
    if (bpos < 0 && ppos < 0) {         /* No redirectors. */
#ifdef COMMENT
        if (!local &&
            (xzcmd == XZDIR || xzcmd == XZTYP ||
             xzcmd == XZXIT || xzcmd == XZSPA ||
             xzcmd == XZHLP || xzcmd == XZPWD ||
             xzcmd == XZLGI || xzcmd == XZLGO ||
             xzcmd == XZWHO || xzcmd == XZHOS)) {
            printf("?\"%s\" has no effect in remote mode\n",cmdbuf);
            if (hints) {
                printf("Hint: Try again with an output redirector.\n");
            }
            return(-9);
        }
#endif	/* COMMENT */
        s = brstrip(s);                 /* Remove outer braces if any. */
        *p = s;                         /* Point to result */
        return(1);                      /* and return. */
    }
    remfile = 1;                        /* It's | or > */
    i = -1;                             /* Get leftmost symbol */
    if (bpos > -1)                      /* Bracket */
      i = bpos;
    if (ppos > -1 && (ppos < bpos || bpos < 0)) { /* or pipe */
        i = ppos;
        rempipe = 1;
    }
    if (rempipe
#ifndef NOPUSH
        && nopush
#endif /* NOPUSH */
        ) {
        printf("?Sorry, access to external commands is disabled.\n");
        return(-9);
    }
    c = s[i];                           /* Copy of symbol */

    if (c == '>' && s[i+1] == '>')      /* ">>" for append? */
      remappd = 1;                     /* It's not just a flag it's a number */

    q = s + i + 1 + remappd;            /* Point past symbol in string */
    while (*q == SP || *q == HT) q++;   /* and any intervening whitespace */
    if (!*q) {
        printf("?%s missing\n", c == '>' ? "Filename" : "Command");
        return(-9);
    }
    if (c == '>' && zchko(q) < 0) {     /* (Doesn't work for | cmd > file) */
        printf("?Access denied - %s\n", q);
        return(-9);
    }
    makestr(&remdest,q);                /* Create the destination string */
    q = s + i - 1;                      /* Point before symbol */
    while (q > s && (*q == SP || *q == HT)) /* Strip trailing whitespace */
      q--;
    *(q+1) = NUL;                       /* Terminate the string. */
    s = brstrip(s);                     /* Remove any braces */
    *p = s;                             /* Set return value */

#ifndef NODEBUG
    if (deblog) {
        debug(F101,"remtxt remfile","",remfile);
        debug(F101,"remtxt remappd","",remappd);
        debug(F101,"remtxt rempipe","",rempipe);
        debug(F110,"remtxt remdest",remdest, 0);
        debug(F110,"remtxt command",s,0);
    }
#endif /* NODEBUG */

    return(1);
}

int
plogin(xx) int xx; {
    char *p1 = NULL, *p2 = NULL, *p3 = NULL;
    int psaved = 0, rc = 0;
#ifdef CK_RECALL
    extern int on_recall;               /* around Password prompting */
#endif /* CK_RECALL */
    debug(F101,"plogin local","",local);

    if (!local || (network && ttchk() < 0)) {
        printf("?No connection\n");
        return(-9);
    }
    if ((x = cmfld("User ID","",&s,xxstring)) < 0) { /* Get User ID */
        if (x != -3) return(x);
    }
    y = strlen(s);
    if (y > 0) {
        if ((p1 = malloc(y + 1)) == NULL) {
            printf("?Internal error: malloc\n");
            rc = -9;
            goto XZXLGI;
        } else
          strcpy(p1,s);                 /* safe */
        if ((rc = cmfld("Password","",&s,xxstring)) < 0)
          if (rc != -3) goto XZXLGI;
        y = strlen(s);
        if (y > 0) {
            if ((p2 = malloc(y + 1)) == NULL) {
                printf("?Internal error: malloc\n");
                rc = -9;
                goto XZXLGI;
            } else
              strcpy(p2,s);             /* safe */
            if ((rc = cmfld("Account","",&s,xxstring)) < 0)
              if (rc != -3) goto XZXLGI;
            y = strlen(s);
            if (y > 0) {
                if ((p3 = malloc(y + 1)) == NULL) {
                    printf("?Internal error: malloc\n");
                    rc = -9;
                    goto XZXLGI;
                } else
                  strcpy(p3,s);         /* safe */
            }
        }
    }
    if ((rc = remtxt(&s)) < 0)          /* Confirm & handle redirectors */
      goto XZXLGI;

    if (!p1) {                          /* No Userid specified... */
        debok = 0;                      /* Don't log this */
        /* Prompt for username, password, and account */
#ifdef CK_RECALL
        on_recall = 0;
#endif /* CK_RECALL */
        cmsavp(psave,PROMPTL);          /* Save old prompt */
        psaved = 1;
        debug(F110,"REMOTE LOGIN saved",psave,0);

        cmsetp("Username: ");           /* Make new prompt */
        concb((char)escape);            /* Put console in cbreak mode */
        cmini(1);
        prompt(xxstring);
        rc = -9;
        for (x = -1; x < 0; ) {         /* Prompt till they answer */
            cmres();                    /* Reset the parser */
            x = cmtxt("","",&s,NULL);   /* Get a literal line of text */
        }
        y = strlen(s);
        if (y < 1) {
            printf("?Canceled\n");
            goto XZXLGI;
        }
        if ((p1 = malloc(y + 1)) == NULL) {
            printf("?Internal error: malloc\n");
            goto XZXLGI;
        } else
          strcpy(p1,s);                 /* safe */

        cmsetp("Password: ");           /* Make new prompt */
        concb((char)escape);            /* Put console in cbreak mode */
        cmini(0);                       /* No echo */
        prompt(xxstring);
        debok = 0;
        for (x = -1; x < 0 && x != -3; ) { /* Get answer */
            cmres();                    /* Reset the parser */
            x = cmtxt("","",&s,NULL);   /* Get literal line of text */
        }
        if ((p2 = malloc((int)strlen(s) + 1)) == NULL) {
            printf("?Internal error: malloc\n");
            goto XZXLGI;
        } else
          strcpy(p2,s);                 /* safe */
        printf("\r\n");
        if ((rc = cmcfm()) < 0)
          goto XZXLGI;
    }
    sstate = setgen('I',p1,p2,p3);      /* Get here with at least user ID */
    rc = 0;

  XZXLGI:                               /* Common exit point */
    if (psaved)
      cmsetp(psave);                    /* Restore original prompt */
    if (p3) { free(p3); p3 = NULL; }    /* Free malloc'd storage */
    if (p2) { free(p2); p2 = NULL; }
    if (p1) { free(p1); p1 = NULL; }
    if (rc > -1) {
        if (local && rc > -1)           /* If local, flush tty input buffer */
          ttflui();
    }
    return(rc);
}

#ifdef OS2
#ifndef NOLOCAL
int
dormt(xx) int xx; {
    int rc = 0;
    extern int term_io;
    int term_io_sav = term_io;
#ifdef NEWFTP
    extern int ftpget, ftpisopen();
    if ((ftpget == 1) || ((ftpget == 2) && ftpisopen()))
      return(doftprmt(xx,0));
#endif /* NEWFTP */
    term_io = 0;
    rc = xxdormt(xx);
    term_io = term_io_sav;
    return rc;
}


int
xxdormt(xx) int xx;
#else /* NOLOCAL */
int
dormt(xx) int xx;
#endif /* NOLOCAL */
#else /* OS2 */
int
dormt(xx) int xx;
#endif /* OS2 */
{                                       /* REMOTE commands */
    int x, y, retcode;
    char *s, sbuf[50], *s2;

#ifdef NEWFTP
    extern int ftpget, ftpisopen();
    if ((ftpget == 1) || ((ftpget == 2) && ftpisopen()))
      return(doftprmt(xx,0));
#endif /* NEWFTP */

    remfile = 0;                        /* Clear these */
    rempipe = 0;
    remappd = 0;

    if (xx < 0) return(xx);             /* REMOTE what? */

    xzcmd = xx;                         /* Make global copy of arg */

    if (xx == XZSET) {                  /* REMOTE SET */
        if ((y = cmkey(rmstab,nrms,"","",xxstring)) < 0) {
            if (y == -3) {
                printf("?Parameter name required\n");
                return(-9);
            } else return(y);
        }
        return(doprm(y,1));
    }

    switch (xx) {                       /* Others... */

      case XZCDU:
        if ((x = cmcfm()) < 0) return(x);
        printf("?Sorry, REMOTE CDUP not supported yet\n");
        return(-9);

      case XZCWD:                       /* CWD (CD) */
        if ((x = cmtxt("Remote directory name","",&s,xxstring)) < 0)
          return(x);
        if ((x = remtxt(&s)) < 0)
          return(x);
        debug(F111,"XZCWD: ",s,x);
        *sbuf = NUL;
        s2 = sbuf;
/*
  The following is commented out because since the disappearance of the
  DECSYSTEM-20 from the planet, no known computer requires a password for
  changing directory.
*/
#ifdef DIRPWDPR
        if (*s != NUL) {                /* If directory name given, */
                                        /* get password on separate line. */
            if (tlevel > -1) {          /* From take file... */

                if (fgets(sbuf,50,tfile[tlevel]) == NULL)
                  fatal("take file ends prematurely in 'remote cwd'");
                debug(F110," pswd from take file",s2,0);
                for (x = (int)strlen(sbuf);
                     x > 0 && (sbuf[x-1] == NL || sbuf[x-1] == CR);
                     x--)
                  sbuf[x-1] = '\0';

            } else {                    /* From terminal... */

                printf(" Password: ");  /* get a password */
#ifdef IKSD
                if (!local && inserver) {
                    x = coninc(0);
                } else
#endif /* IKSD */
#ifdef OS2
                  x = is_a_tty(0) ? coninc(0) : /* with no echo ... */
                    getchar();
#else /* OS2 */
                x = getchar();
#endif /* OS2 */
                while ((x != NL) && (x != CR)) {
                    if ((x &= 0177) == '?') {
                        printf("? Password of remote directory\n Password: ");
                        s2 = sbuf;
                        *sbuf = NUL;
                    } else if (x == ESC) /* Mini command line editor... */
                      bleep(BP_WARN);
                    else if (x == BS || x == 0177)
                      s2--;
                    else if (x == 025) {        /* Ctrl-U */
                        s2 = sbuf;
                        *sbuf = NUL;
                    } else
                      *s2++ = x;

                    /* Get the next character */
#ifdef IKSD
                    if (!local && inserver) {
                        x = coninc(0);
                    } else
#endif /* IKSD */
#ifdef OS2
                    x = is_a_tty(0) ? coninc(0) : /* with no echo ... */
                      getchar();
#else /* OS2 */
                    x = getchar();
#endif /* OS2 */
                }
                *s2 = NUL;
                putchar('\n');
            }
            s2 = sbuf;
        } else s2 = "";
#endif /* DIRPWDPR */

        debug(F110," password",s2,0);
	rcdactive = 1;
        sstate = setgen('C',s,s2,"");
        retcode = 0;
        break;

      case XZDEL:                               /* Delete */
        if ((x = cmtxt("Name of remote file(s) to delete",
                       "",&s,xxstring)) < 0) {
            if (x == -3) {
                printf("?Name of remote file(s) required\n");
                return(-9);
            } else return(x);
        }
        if ((x = remtxt(&s)) < 0)
          return(x);
        if (local) ttflui();            /* If local, flush tty input buffer */
        retcode = sstate = rfilop(s,'E');
        break;

      case XZDIR:                       /* Directory */
        if ((x = cmtxt("Remote directory or file specification","",&s,
                       xxstring)) < 0)
          return(x);
        if ((x = remtxt(&s)) < 0)
          return(x);
        if (local) ttflui();            /* If local, flush tty input buffer */
        rmsg();
        retcode = sstate = setgen('D',s,"","");
        break;

      case XZHLP:                       /* Help */
        if ((x = remcfm()) < 0) return(x);
        sstate = setgen('H',"","","");
        retcode = 0;
        break;

      case XZHOS:                       /* Host */
        if ((x = cmtxt("Command for remote system","",&s,xxstring)) < 0)
          return(x);
        if ((x = remtxt(&s)) < 0)
          return(x);
        if ((y = (int)strlen(s)) < 1)
          return(x);
        ckstrncpy(line,s,LINBUFSIZ);
        cmarg = line;
        rmsg();
        retcode = sstate = 'c';
        break;

#ifndef NOFRILLS
      case XZKER:
        if ((x = cmtxt("Command for remote Kermit","",&s,xxstring)) < 0)
          return(x);
        if ((x = remtxt(&s)) < 0)
          return(x);
        if ((int)strlen(s) < 1)  {
            if (x == -3) {
                printf("?Remote Kermit command required\n");
                return(-9);
            } else return(x);
        }
        ckstrncpy(line,s,LINBUFSIZ);
        cmarg = line;
        retcode = sstate = 'k';
        rmsg();
        break;

      case XZLGI:                       /* Login */
	rcdactive = 1;			/* Suppress "Logged in" msg if quiet */
        return(plogin(XXREM));

      case XZLGO: {                     /* Logout */
          extern int bye_active;
          if ((x = remcfm()) < 0) return(x);
          sstate = setgen('I',"","","");
          retcode = 0;
          bye_active = 1;               /* Close connection when done */
          break;
      }

      case XZPRI:                       /* Print */
        if (!atdiso || !atcapr) {       /* Disposition attribute off? */
            printf("?Disposition Attribute is Off\n");
            return(-2);
        }
        cmarg = "";
        cmarg2 = "";
        if ((x = cmifi("Local file(s) to print on remote printer","",&s,&y,
                       xxstring)) < 0) {
            if (x == -3) {
                printf("?Name of local file(s) required\n");
                return(-9);
            }
            return(x);
        }
        ckstrncpy(line,s,LINBUFSIZ);    /* Make a safe copy of filename */
        *optbuf = NUL;                  /* Wipe out any old options */
        if ((x = cmtxt("Options for remote print command","",&s,xxstring)) < 0)
          return(x);
        if ((x = remtxt(&s)) < 0)
          return(x);
        if ((int)strlen(optbuf) > 94) { /* Make sure this is legal */
            printf("?Option string too long\n");
            return(-9);
        }
        ckstrncpy(optbuf,s,OPTBUFLEN);  /* Make a safe copy of options */
        nfils = -1;                     /* Expand file list internally */
        cmarg = line;                   /* Point to file list. */
        rprintf = 1;                    /* REMOTE PRINT modifier for SEND */
        sstate = 's';                   /* Set start state to SEND */
        if (local) displa = 1;
        retcode = 0;
        break;
#endif /* NOFRILLS */

      case XZSPA:                       /* Space */
        if ((x = cmtxt("Confirm, or remote directory name",
                       "",&s,xxstring)) < 0)
          return(x);
        if ((x = remtxt(&s)) < 0)
          return(x);
        retcode = sstate = setgen('U',s,"","");
        break;

      case XZMSG:                       /* Message */
        if ((x = cmtxt("Short text message for server","",&s,xxstring)) < 0)
          return(x);
        if ((x = remtxt(&s)) < 0)
          return(x);
        retcode = sstate = setgen('M',s,"","");
        break;

#ifndef NOFRILLS
      case XZTYP:                       /* Type */
        if ((x = cmtxt("Remote file specification","",&s,xxstring)) < 0)
          return(x);
        if ((int)strlen(s) < 1) {
            printf("?Remote filename required\n");
            return(-9);
        }
        if ((x = remtxt(&s)) < 0)
          return(x);
        rmsg();
        retcode = sstate = rfilop(s,'T');
        break;
#endif /* NOFRILLS */

#ifndef NOFRILLS
      case XZWHO:
        if ((x = cmtxt("Remote user name, or carriage return",
                       "",&s,xxstring)) < 0)
          return(x);
        if ((x = remtxt(&s)) < 0)
          return(x);
        retcode = sstate = setgen('W',s,"","");
        break;
#endif /* NOFRILLS */

      case XZPWD:                       /* PWD */
        if ((x = remcfm()) < 0) return(x);
        sstate = setgen('A',"","","");
        retcode = 0;
        break;

#ifndef NOSPL
      case XZQUE: {                     /* Query */
          char buf[2];
          extern char querybuf[], * qbufp;
          extern int qbufn;
          if ((y = cmkey(vartyp,nvartyp,"","",xxstring)) < 0)
            return(y);
          if ((x = cmtxt(y == 'F' ? "Remote function invocation" :
                         ('K' ? "Remote variable name or function":
                         "Remote variable name"),
                         "",
                         &s,
                         (y == 'K') ? xxstring : NULL
                         )) < 0)        /* Don't evaluate */
            return(x);
          if ((x = remtxt(&s)) < 0)
            return(x);
          query = 1;                    /* QUERY is active */
          qbufp = querybuf;             /* Initialize query response buffer */
          qbufn = 0;
          querybuf[0] = NUL;
          buf[0] = (char) (y & 127);
          buf[1] = NUL;
          retcode = sstate = setgen('V',"Q",(char *)buf,s);
          break;
      }

      case XZASG: {                     /* Assign */
          char buf[VNAML];
          if ((y = cmfld("Remote variable name","",&s,NULL)) < 0) /* No eval */
            return(y);
          if ((int)strlen(s) >= VNAML) {
              printf("?Too long\n");
              return(-9);
          }
          ckstrncpy(buf,s,VNAML);
          if ((x = cmtxt("Assignment for remote variable",
                   "",&s,xxstring)) < 0) /* Evaluate this one */
            return(x);
          if ((x = remtxt(&s)) < 0)
            return(x);
#ifdef COMMENT
/*
  Server commands can't be long packets.  In principle there's no reason
  why they shouldn't be, except that we don't know at this point if the
  server is capable of accepting long packets because we haven't started
  the protocol yet.  In practice, allowing a long packet here breaks a lot
  of assumptions, causes buffer overruns and crashes, etc.  To be fixed
  later.  (But since this is commented out, evidently I fixed it later...)
*/
          if ((int)strlen(s) > 85) {    /* Allow for encoding expansion */
              printf("?Sorry, value is too long - 85 characters max\n");
              return(-9);
          }
#endif /* COMMENT */
          retcode = sstate = setgen('V',"S",(char *)buf,s);
          break;
      }
#endif /* NOSPL */

      case XZCPY: {                     /* COPY */
          char buf[TMPBUFSIZ];
          buf[TMPBUFSIZ-1] = '\0';
          if ((x = cmfld("Name of remote file to copy","",&s,xxstring)) < 0) {
              if (x == -3) {
                  printf("?Name of remote file required\n");
                  return(-9);
              }
              else
                return(x);
          }
          ckstrncpy(buf,s,TMPBUFSIZ);
          if ((x = cmfld("Name of remote destination file or directory",
                         "",&s, xxstring)) < 0) {
              if (x == -3) {
                  printf("?Name of remote file or directory required\n");
                  return(-9);
              } else return(x);
          }
          ckstrncpy(tmpbuf,s,TMPBUFSIZ);
          if ((x = remcfm()) < 0)
            return(x);
          if (local) ttflui();          /* If local, flush tty input buffer */
          retcode = sstate = setgen('K',buf,tmpbuf,"");
          break;
      }
      case XZREN: {                     /* Rename */
          char buf[TMPBUFSIZ];
          buf[TMPBUFSIZ-1] = '\0';
          if ((x = cmfld("Name of remote file to rename",
                         "",&s,xxstring)) < 0) {
              if (x == -3) {
                  printf("?Name of remote file required\n");
                  return(-9);
              } else return(x);
          }
          ckstrncpy(buf,s,TMPBUFSIZ);
          if ((x = cmfld("New name of remote file","",&s, xxstring)) < 0) {
              if (x == -3) {
                  printf("?Name of remote file required\n");
                  return(-9);
              } else return(x);
          }
          ckstrncpy(tmpbuf,s,TMPBUFSIZ);
          if ((x = remcfm()) < 0)
            return(x);
          if (local) ttflui();          /* If local, flush device buffer */
          retcode = sstate = setgen('R',buf,tmpbuf,"");
          break;
      }
      case XZMKD:                       /* mkdir */
      case XZRMD:                       /* rmdir */
        if ((x = cmtxt((xx == XZMKD) ?
                       "Name of remote directory to create" :
                       "Name of remote directory to delete",
                       "",
                       &s,
                       xxstring
                       )) < 0) {
            if (x == -3) {
                printf("?Name required\n");
                return(-9);
            } else return(x);
        }
        if ((x = remtxt(&s)) < 0)
          return(x);
        if (local) ttflui();            /* If local, flush tty input buffer */
        retcode = sstate = rfilop(s, (char)(xx == XZMKD ? 'm' : 'd'));
        break;

      case XZXIT:                       /* Exit */
        if ((x = remcfm()) < 0) return(x);
        sstate = setgen('X',"","","");
        retcode = 0;
        break;

      default:
        if ((x = remcfm()) < 0) return(x);
        printf("?Not implemented - %s\n",cmdbuf);
        return(-2);
    }
    if (local && retcode > -1)          /* If local, flush tty input buffer */
      ttflui();
    return(retcode);
}


/*  R F I L O P  --  Remote File Operation  */

CHAR
#ifdef CK_ANSIC
rfilop(char * s, char t)
#else
rfilop(s,t) char *s, t;
#endif /* CK_ANSIC */
/* rfilop */ {
    if (*s == NUL) {
        printf("?File specification required\n");
        return((CHAR) 0);
    }
    debug(F111,"rfilop",s,t);
    return(setgen(t,s,"",""));
}
#endif /* NOXFER */

#ifdef ANYX25
int
setx25() {
    if ((y = cmkey(x25tab,nx25,"X.25 call options","",xxstring)) < 0)
      return(y);
    switch (y) {
      case XYUDAT:
        if ((z = cmkey(onoff,2,"X.25 call user data","",xxstring))
            < 0) return(z);
        if (z == 0) {
            if ((z = cmcfm()) < 0) return(z);
            cudata = 0;             /* disable call user data */
            return (success = 1);
        }
        if ((x = cmtxt("X.25 call user data string","",&s,xxstring)) < 0)
          return(x);
        if ((int)strlen(s) == 0) {
            return (-3);
        } else if ((int)strlen(s) > MAXCUDATA) {
            printf("?The length must be > 0 and <= %d\n",MAXCUDATA);
            return(-2);
        }
        if ((y = cmcfm()) < 0) return(y);
        ckstrncpy(udata,s,MAXCUDATA);
        cudata = 1;                     /* X.25 call user data specified */
        return (success = 1);
      case XYCLOS:
        if ((z = cmkey(onoff,2,"X.25 closed user group call","",xxstring))
            < 0) return(z);
        if (z == 0) {
            if ((z = cmcfm()) < 0) return(z);
            closgr = -1;                /* disable closed user group */
            return (success = 1);
        }
        if ((y = cmnum("0 <= cug index >= 99","",10,&x,xxstring)) < 0)
          return(y);
        if (x < 0 || x > 99) {
            printf("?The choices are 0 <= cug index >= 99\n");
            return(-2);
        }
        if ((y = cmcfm()) < 0) return(y);
        closgr = x;                     /* closed user group selected */
        return (success = 1);

      case XYREVC:
        if((z = cmkey(onoff,2,"X.25 reverse charge call","",xxstring)) < 0)
          return(z);
        if ((x = cmcfm()) < 0) return(x);
        revcall = z;
        return (success = 1);
    }
}

#ifndef IBMX25
int
setpadp() {
    if ((y = cmkey(padx3tab,npadx3,"PAD X.3 parameter name","",xxstring)) < 0)
      return(y);
    x = y;
    switch (x) {
      case PAD_BREAK_CHARACTER:
        if ((y = cmnum("PAD break character value","",10,&z,xxstring)) < 0)
          return(y);
        if ((y = cmcfm()) < 0) return(y);
        break;
      case PAD_ESCAPE:
        if ((y = cmnum("PAD escape","",10,&z,xxstring)) < 0) return(y);
        if (z != 0 && z != 1) {
            printf("?The choices are 0 or 1\n");
            return(-2);
        }
        if ((y = cmcfm()) < 0) return(y);
        break;
      case PAD_ECHO:
        if ((y = cmnum("PAD echo","",10,&z,xxstring)) < 0) return(y);
        if (z != 0 && z != 1) {
            printf("?The choices are 0 or 1\n");
            return(-2);
        }
        if ((y = cmcfm()) < 0) return(y);
        break;
      case PAD_DATA_FORWARD_CHAR:
        if ((y = cmnum("PAD data forward char","",10,&z,xxstring)) < 0)
          return(y);
        if (z != 0 && z != 2) {
            printf("?The choices are 0 or 2\n");
            return(-2);
        }
        if ((y = cmcfm()) < 0) return(y);
        break;
      case PAD_DATA_FORWARD_TIMEOUT:
        if ((y = cmnum("PAD data forward timeout","",10,&z,xxstring)) < 0)
            return(y);
        if (z < 0 || z > 255) {
            printf("?The choices are 0 or 1 <= timeout <= 255\n");
            return(-2);
        }
        if ((y = cmcfm()) < 0) return(y);
        break;
      case PAD_FLOW_CONTROL_BY_PAD:
        if ((y = cmnum("PAD pad flow control","",10,&z,xxstring)) < 0)
          return(y);
        if (z != 0 && z != 1) {
            printf("?The choices are 0 or 1\n");
            return(-2);
        }
        if ((y = cmcfm()) < 0) return(y);
        break;
      case PAD_SUPPRESSION_OF_SIGNALS:
        if ((y = cmnum("PAD service","",10,&z,xxstring)) < 0) return(y);
        if (z != 0 && z != 1) {
            printf("?The choices are 0 or 1\n");
            return(-2);
        }
        if ((y = cmcfm()) < 0) return(y);
        break;

      case PAD_BREAK_ACTION:
        if ((y = cmnum("PAD break action","",10,&z,xxstring)) < 0) return(y);
        if (z != 0 && z != 1 && z != 2 && z != 5 && z != 8 && z != 21) {
            printf("?The choices are 0, 1, 2, 5, 8 or 21\n");
            return(-2);
        }
        if ((y = cmcfm()) < 0) return(y);
        break;

      case PAD_SUPPRESSION_OF_DATA:
        if ((y = cmnum("PAD data delivery","",10,&z,xxstring)) < 0) return(y);
        if (z != 0 && z != 1) {
            printf("?The choices are 0 or 1\n");
            return(-2);
        }
        if ((y = cmcfm()) < 0) return(y);
        break;

      case PAD_PADDING_AFTER_CR:
        if ((y = cmnum("PAD crpad","",10,&z,xxstring)) < 0) return(y);
        if (z < 0 || z > 7) {
            printf("?The choices are 0 or 1 <= crpad <= 7\n");
            return(-2);
        }
        if ((y = cmcfm()) < 0) return(y);
        break;

      case PAD_LINE_FOLDING:
        if ((y = cmnum("PAD linefold","",10,&z,xxstring)) < 0) return(y);
        if (z < 0 || z > 255) {
            printf("?The choices are 0 or 1 <= linefold <= 255\n");
            return(-2);
        }
        if ((y = cmcfm()) < 0) return(y);
        break;

      case PAD_LINE_SPEED:
        if ((y = cmnum("PAD baudrate","",10,&z,xxstring)) < 0) return(y);
        if (z < 0 || z > 18) {
            printf("?The choices are 0 <= baudrate <= 18\n");
            return(-2);
        }
        if ((y = cmcfm()) < 0) return(y);
        break;

      case PAD_FLOW_CONTROL_BY_USER:
        if ((y = cmnum("PAD terminal flow control","",10,&z,xxstring)) < 0)
            return(y);
        if (z != 0 && z != 1) {
            printf("?The choices are 0 or 1\n");
            return(-2);
        }
        if ((y = cmcfm()) < 0) return(y);
        break;

      case PAD_LF_AFTER_CR:
        if ((y = cmnum("PAD crpad","",10,&z,xxstring)) < 0) return(y);
        if (z < 0 || z == 3 || z > 7) {
            printf("?The choices are 0, 1, 2, 4, 5, 6 or 7\n");
            return(-2);
        }
        if ((y = cmcfm()) < 0) return(y);
        break;

      case PAD_PADDING_AFTER_LF:
        if ((y = cmnum("PAD lfpad","",10,&z,xxstring)) < 0) return(y);
        if (z < 0 || z > 7) {
            printf("?The choices are 0 or 1 <= lfpad <= 7\n");
            return(-2);
        }
        if ((y = cmcfm()) < 0) return(y);
        break;

      case PAD_EDITING:
        if ((y = cmnum("PAD edit control","",10,&z,xxstring)) < 0) return(y);
        if (z != 0 && z != 1) {
            printf("?The choices are 0 or 1\n");
            return(-2);
        }
        if ((y = cmcfm()) < 0) return(y);
        break;

      case PAD_CHAR_DELETE_CHAR:
        if ((y = cmnum("PAD char delete char","",10,&z,xxstring)) < 0)
            return(y);
        if (z < 0 || z > 127) {
            printf("?The choices are 0 or 1 <= chardelete <= 127\n");
            return(-2);
        }
        if ((y = cmcfm()) < 0) return(y);
        break;

      case PAD_BUFFER_DELETE_CHAR:
        if ((y = cmnum("PAD buffer delete char","",10,&z,xxstring)) < 0)
            return(y);
        if (z < 0 || z > 127) {
            printf("?The choices are 0 or 1 <= bufferdelete <= 127\n");
            return(-2);
        }
        if ((y = cmcfm()) < 0) return(y);
        break;

      case PAD_BUFFER_DISPLAY_CHAR:
        if ((y = cmnum("PAD display line char","",10,&z,xxstring)) < 0)
            return(y);
        if (z < 0 || z > 127) {
            printf("?The choices are 0 or 1 <= displayline <= 127\n");
            return(-2);
        }
        if ((y = cmcfm()) < 0) return(y);
        break;
    }
    padparms[x] = z;
    return(success = 1);
}
#endif /* IBMX25 */
#endif /* ANYX25 */

#ifndef NOXFER
int
setat(rmsflg) int rmsflg; {
    int xx;
    if ((y = cmkey(attrtab,natr,"File Attribute packets","",xxstring)) < 0)
      return(y);
    if (y == AT_XALL) {                 /* ATTRIBUTES ALL ON or ALL OFF */
        if ((z = seton(&xx)) < 0) return(z);
        if (rmsflg) {
            printf("Sorry, command not available\n");
            return(-9);
        } else {
            atenci = xx;                /* Encoding in */
            atenco = xx;                /* Encoding out */
            atdati = xx;                /* Date in */
            atdato = xx;                /* Date out */
            atdisi = xx;                /* Disposition in/out */
            atdiso = xx;
            atleni = xx;                /* Length in/out (both kinds) */
            atleno = xx;
            atblki = xx;                /* Blocksize in/out */
            atblko = xx;
            attypi = xx;                /* File type in/out */
            attypo = xx;
            atsidi = xx;                /* System ID in/out */
            atsido = xx;
            atsysi = xx;                /* System-dependent params in/out */
            atsyso = xx;
#ifdef CK_PERMS                         /* Protection */
            atlpri = xx;                /* Local in */
            atlpro = xx;                /* Local out */
            atgpri = xx;                /* Generic in */
            atgpro = xx;                /* Generic out */
#endif /* CK_PERMS */
#ifdef STRATUS
            atfrmi = xx;                /* Format in/out */
            atfrmo = xx;
            atcrei = xx;                /* Creator id in/out */
            atcreo = xx;
            atacti = xx;                /* Account in/out */
            atacto = xx;
#endif /* STRATUS */
        }
        return(z);
    } else if (y == AT_ALLY || y == AT_ALLN) { /* ATTRIBUTES ON or OFF */
        if ((x = cmcfm()) < 0) return(x);
        atcapr = (y == AT_ALLY) ? 1 : 0;
        if (rmsflg) {
            sstate = setgen('S', "132", atcapr ? "1" : "0", "");
            return((int) sstate);
        } else return(success = 1);
    }
    /* Otherwise, it's an individual attribute that wants turning off/on */

    if ((z = cmkey(onoff,2,"","",xxstring)) < 0) return(z);
    if ((x = cmcfm()) < 0) return(x);

/* There are better ways to do this... */
/* The real problem is that we're not separating the in and out cases */
/* and so we have to arbitrarily pick the "in" case, i.e tell the remote */
/* server to ignore incoming attributes of the specified type, rather */
/* than telling it not to send them.  The protocol does not (yet) define */
/* codes for "in-and-out-at-the-same-time". */

    switch (y) {
#ifdef CK_PERMS
/* We're lumping local and generic protection together for now... */
      case AT_LPRO:
      case AT_GPRO:
        if (rmsflg) {
            sstate = setgen('S', "143", z ? "1" : "0", "");
            return((int) sstate);
        }
        atlpri = atlpro = atgpri = atgpro = z; break;
#endif /* CK_PERMS */
      case AT_DISP:
        if (rmsflg) {
            sstate = setgen('S', "142", z ? "1" : "0", "");
            return((int) sstate);
        }
        atdisi = atdiso = z; break;
      case AT_ENCO:
        if (rmsflg) {
            sstate = setgen('S', "141", z ? "1" : "0", "");
            return((int) sstate);
        }
        atenci = atenco = z; break;
      case AT_DATE:
        if (rmsflg) {
            sstate = setgen('S', "135", z ? "1" : "0", "");
            return((int) sstate);
        }
        atdati = atdato = z; break;
      case AT_LENB:
      case AT_LENK:
        if (rmsflg) {
            sstate = setgen('S', "133", z ? "1" : "0", "");
            return((int) sstate);
        }
        atleni = atleno = z; break;
      case AT_BLKS:
        if (rmsflg) {
            sstate = setgen('S', "139", z ? "1" : "0", "");
            return((int) sstate);
        }
        atblki = atblko = z; break;
      case AT_FTYP:
        if (rmsflg) {
            sstate = setgen('S', "134", z ? "1" : "0", "");
            return((int) sstate);
        }
        attypi = attypo = z; break;
#ifdef STRATUS
      case AT_CREA:
        if (rmsflg) {
            sstate = setgen('S', "136", z ? "1" : "0", "");
            return((int) sstate);
        }
        atcrei = atcreo = z; break;
      case AT_ACCT:
        if (rmsflg) {
            sstate = setgen('S', "137", z ? "1" : "0", "");
            return((int) sstate);
        }
        atacti = atacto = z; break;
#endif /* STRATUS */
      case AT_SYSI:
        if (rmsflg) {
            sstate = setgen('S', "145", z ? "1" : "0", "");
            return((int) sstate);
        }
        atsidi = atsido = z; break;
      case AT_RECF:
        if (rmsflg) {
            sstate = setgen('S', "146", z ? "1" : "0", "");
            return((int) sstate);
        }
        atfrmi = atfrmo = z; break;
      case AT_SYSP:
        if (rmsflg) {
            sstate = setgen('S', "147", z ? "1" : "0", "");
            return((int) sstate);
        }
        atsysi = atsyso = z; break;
      default:
        printf("?Not available\n");
        return(-2);
    }
    return(1);
}
#endif /* NOXFER */

#ifndef NOSPL
int
setinp() {
    if ((y = cmkey(inptab,ninp,"","",xxstring)) < 0) return(y);
    switch (y) {
#ifdef OS2
      case IN_PAC:                      /* SET INPUT PACING */
        z = cmnum("milliseconds","0",10,&x,xxstring);
        return(setnum(&tt_inpacing,x,z,1000));
      case IN_TRM:                      /* SET INPUT TERMINAL */
        return(seton(&interm));
#endif /* OS2 */
      case IN_DEF:                      /* SET INPUT DEFAULT-TIMEOUT */
        z = cmnum("Positive number","",10,&x,xxstring);
        return(setnum(&indef,x,z,94));
#ifdef CKFLOAT
      case IN_SCA:                      /* SET INPUT SCALE-FACTOR */
	if ((x = cmfld("Number such as 2 or 0.5","1.0",&s, xxstring)) < 0)
	  return(x);
        if (isfloat(s,0)) {		/* A floating-point number? */
            extern char * inpscale;
	    inscale = floatval;		/* Yes, get its value */
	    makestr(&inpscale,s);	/* Save it as \v(inscale) */
	    return(success = 1);
	} else {
	    return(-2);
	}
#endif	/* CKFLOAT */
      case IN_TIM:                      /* SET INPUT TIMEOUT-ACTION */
        if ((z = cmkey(intimt,2,"","",xxstring)) < 0) return(z);
        if ((x = cmcfm()) < 0) return(x);
        intime[cmdlvl] = z;
        return(success = 1);
      case IN_CAS:                      /* SET INPUT CASE */
        if ((z = cmkey(incast,2,"","",xxstring)) < 0) return(z);
        if ((x = cmcfm()) < 0) return(x);
        inpcas[cmdlvl] = z;
        return(success = 1);
      case IN_ECH:                      /* SET INPUT ECHO */
        return(seton(&inecho));
      case IN_SIL:                      /* SET INPUT SILENCE */
        z = cmnum("Seconds of inactivity before INPUT fails","",10,&x,
                  xxstring);
        return(setnum(&insilence,x,z,-1));

      case IN_BUF:                      /* SET INPUT BUFFER-SIZE */
        if ((z = cmnum("Number of bytes in INPUT buffer",
                       ckitoa(INPBUFSIZ),10,&x, xxstring)) < 0)
          return(z);
        if ((y = cmcfm()) < 0) return(y);
        inbufsize = 0;
        if (inpbuf) {
            free(inpbuf);
            inpbuf = NULL;
            inpbp = NULL;
        }
        if (!(s = (char *)malloc(x + 1)))
          return(0);
        inpbuf = s;
        inpbp = s;
        inbufsize = x;
        for (x = 0; x <= inbufsize; x++)
          inpbuf[x] = NUL;
        return(success = 1);

#ifdef CK_AUTODL
      case IN_ADL:                      /* AUTODOWNLOAD */
        return(seton(&inautodl));
#endif /* CK_AUTODL */

      case IN_CAN:                      /* SET INPUT INTERRUPTS */
        return(seton(&inintr));
    }
    return(0);
}
#endif /* NOSPL */

#ifdef NETCONN
VOID
ndreset() {
#ifndef NODIAL                          /* This depends on DIAL... */
    int i=0, j=0;
    if (!ndinited)                      /* Don't free garbage... */
      return;
    for (i = 0; i < nhcount; i++) {     /* Clean out previous list */
        if (nh_p[i])
          free(nh_p[i]);
        nh_p[i] = NULL;
        if (nh_p2[i])
          free(nh_p2[i]);
        nh_p2[i] = NULL;
        for (j = 0; j < 4; j++) {
            if (nh_px[j][i])
              free(nh_px[j][i]);
            nh_px[j][i] = NULL;
        }
    }
#endif /* NODIAL */
}

VOID
ndinit() {                              /* Net directory pointers */
#ifndef NODIAL                          /* This depends on DIAL... */
    int i, j;
    if (ndinited++)                     /* Don't do this more than once. */
      return;
    for (i = 0; i < MAXDDIR; i++) {     /* Init all pointers to NULL */
        netdir[i] = NULL;
    }
    for (i = 0; i < MAXDNUMS; i++) {
        nh_p[i] = NULL;
        nh_p2[i] = NULL;
        for (j = 0; j < 4; j++)
          nh_px[j][i] = NULL;
    }
#endif /* NODIAL */
}

#ifndef NODIAL
#ifdef NETCONN
VOID                                    /* Get net defaults from environment */
getnetenv() {
    char *p = NULL;

    makestr(&p,getenv("K_NET_DIRECTORY")); /* Dialing directories */
    if (p) {
        int i;
        xwords(p,MAXDDIR,netdir,0);
        for (i = 0; i < MAXDDIR; i++) { /* Fill in any gaps... */
            if (!netdir[i+1])
              break;
            else
              netdir[i] = netdir[i+1];
            debug(F111,"netdir[i]",netdir[i],i);
        }
        nnetdir = i;
    }
}
#endif /* NETCONN */
#endif /* NODIAL */

int
#ifdef CK_ANSIC
lunet(char *s)                          /* s = name to look up   */
#else
lunet(s) char *s;
#endif /* CK_ANSIC */
/* lunet */ {
#ifndef NODIAL                          /* This depends on DIAL... */
    int n, n1, t, dd = 0;
    int ambiguous = 0;
    FILE * f;
    char *line = NULL;
    extern int dialdpy;
    int netdpy = dialdpy;
    char *info[8];

    nhcount = 0;                        /* Set this before returning */

    if (!s || nnetdir < 1)              /* Validate arguments */
      return(-1);

    if (isdigit(*s) || *s == '*' || *s == '.')
      return(0);

    if ((n1 = (int) strlen(s)) < 1)     /* Length of string to look up */
      return(-1);

    if (!(line = malloc(1024)))         /* Allocate input buffer */
      return(-1);

  lu_again:
    f = NULL;                           /* Network directory file descriptor */
    t = nhcount = 0;                    /* Match count */
    dd = 0;                             /* Directory counter */

    dirline = 0;
    while (1) {                         /* We make one pass */
        if (!f) {                       /* Directory not open */
            if (dd >= nnetdir)          /* No directories left? */
              break;                    /* Done. */
            if ((f = fopen(netdir[dd],"r")) == NULL) { /* Open it */
                perror(netdir[dd]);     /* Can't, print message saying why */
                dd++;
                continue;               /* But go on to next one. */
            }
            if (netdpy)
              printf("Opening %s...\n",netdir[dd]);
            dd++;
        }
        line[0] = NUL;
        if (getnct(line,1023,f,1) < 0) { /* Read a line */
            if (f) {                    /* f can be clobbered! */
                fclose(f);              /* Close the file */
                f = NULL;               /* Indicate next one needs opening */
            }
            continue;
        }
        if (!line[0])                   /* Empty line */
          continue;

        xwords(line,7,info,0);          /* Parse it */

        if (!info[1] || !info[2] || !info[3]) /* Required fields */
          continue;
        if (*info[1] == ';')            /* Full-line comment */
          continue;
        if ((n = (int) strlen(info[1])) < 1) /* Length of name-tag */
          continue;
        if (n < n1)                     /* Search name is longer */
          continue;                     /* Can't possibly match */
        if (ambiguous && n != n1)
          continue;
        if (ckstrcmp(s,info[1],n1,0))   /* Compare using length of */
          continue;                     /* search string s. */

        /* Have a match */

        makestr(&(nh_p[nhcount]), info[3]);    /* address */
        makestr(&(nh_p2[nhcount]),info[2]);    /* net type */
        makestr(&(nh_px[0][nhcount]),info[4]); /* net-specific stuff... */
        makestr(&(nh_px[1][nhcount]),info[5]);
        makestr(&(nh_px[2][nhcount]),info[6]);
        makestr(&(nh_px[3][nhcount]),info[7]);

        nhcount++;                      /* Count this match */
        if (nhcount > MAXDNUMS) {       /* Watch out for too many */
            printf("Warning: %d matches found, %d max\n",
                   nhcount,
                   MAXDNUMS
                   );
            nhcount = MAXDNUMS;
            break;
        }
        if (nhcount == 1) {             /* First one - save entry name */
            if (n_name) {               /* Free the one from before if any */
                free(n_name);
                n_name = NULL;
            }
            if (!(n_name = (char *)malloc(n + 1))) { /* Allocate new storage */
                printf("?memory allocation error - lunet:3\n");
                if (line) {
                    free(line);
                    line = NULL;
                }
                nhcount = 0;
                return(-1);
            }
            t = n;                      /* Remember its length */
            strcpy(n_name,info[1]);     /* safe */
        } else {                        /* Second or subsequent one */
            if ((int) strlen(info[1]) == t) /* Lengths compare */
              if (!ckstrcmp(n_name,info[1],t,0)) /* Caseless compare OK */
                continue;

            /* Name given by user matches entries with different names */

            if (ambiguous)              /* Been here before */
              break;

            ambiguous = 1;              /* Now an exact match is required */
            ndreset();                  /* Clear out previous list */
            goto lu_again;              /* Do it all over again. */
        }
    }
    if (line) {
        free(line);
        line = NULL;
    }
    if (nhcount == 0 && ambiguous)
      printf("?\"%s\" - ambiguous in network directory\n",s);
#else
    nhcount = 0;
#endif /* NODIAL */
    return(nhcount);
}
#endif /* NETCONN */

#ifndef NOLOCAL
/*  C L S C O N N X  --  Close connection  */

int
clsconnx(ask) int ask; {
    int x, rc = 0;
#ifdef NEWFTP
    extern int ftpget, ftpisopen();
    if ((ftpget == 1) || ((ftpget == 2) && !local && ftpisopen()))
      return(success = ftpbye());
#endif /* NEWFTP */
    debug(F101,"clsconnx local","",local);
    if (local) {
        x = ask ? hupok(1) : 1;         /* Make sure it's OK to close */
        if (!x) {
            rc = -1;
            debug(F101,"clsconnx hupok says no","",rc);
            return(rc);
        }
        ttflui();                       /* Clear away buffered up junk */
#ifndef NODIAL
#ifdef OS2ONLY
/* Don't hangup a line that is shared with the SLIP or PPP driver */
        if (!ttslip && !ttppp)
#endif /* OS2ONLY */
          mdmhup();
#endif /* NODIAL */
        if (network && msgflg)
          printf(" Closing connection\n");
        ttclos(0);                      /* Close old connection, if any */
        rc = 1;
        {
            extern int wasclosed, whyclosed;
            if (wasclosed) {
                whyclosed = WC_CLOS;
#ifndef NOSPL
                if (nmac) {             /* Any macros defined? */
                    int k;              /* Yes */
                    /* printf("ON_CLOSE CLSCONNX\n"); */
                    wasclosed = 0;
                    k = mlook(mactab,"on_close",nmac);  /* Look this up */
                    if (k >= 0) {                       /* If found, */
                        if (dodo(k,ckitoa(whyclosed),0) > -1) /* set it up, */
                          parser(1);                    /* and execute it */
                    }
                }
#endif /* NOSPL */
                whyclosed = WC_REMO;
                wasclosed = 0;
            }
        }
    }
#ifdef VMS                              /* Or maybe #ifndef UNIX? */
    else {                              /* Need to do this in VMS to */
        ttclos(0);                      /* free the tty channel number */
        rc = 1;                         /* obtained in ttopen() or else */
    }                                   /* subsequent ttopen's won't work */
#endif /* VMS */
    dologend();
    haveline = 0;
    if (mdmtyp < 0) {                   /* Switching from net to async? */
        if (mdmsav > -1)                /* Restore modem type from last */
          mdmtyp = mdmsav;              /* SET MODEM command, if any. */
        else
          mdmtyp = 0;
        mdmsav = -1;
    }
    if (network)
      network = 0;
#ifdef NETCONN
    if (oldplex > -1) {                 /* Restore previous duplex setting. */
        duplex = oldplex;
        oldplex = -1;
    }
#endif /* NETCONN */
#ifndef MAC
    ckstrncpy(ttname,dftty,TTNAMLEN);   /* Restore default communication */
#endif /* MAC */
    local = dfloc;                      /* device and local/remote status */
    if (local) {
        cxtype = CXT_DIRECT;            /* Something reasonable */
        speed = ttgspd();               /* Get the current speed */
    } else {
        cxtype = CXT_REMOTE;
        speed = -1L;
    }
#ifndef NOXFER
    if (xreliable > -1 && !setreliable) {
        reliable = xreliable;
        debug(F101,"clsconnx reliable A","",reliable);
    } else if (!setreliable) {
        reliable = SET_AUTO;
        debug(F101,"clsconnx reliable B","",reliable);
    }
#endif /* NOXFER */
    setflow();                          /* Revert flow control */
    return(rc);
}

int
clskconnx(x) int x; {                   /* Close Kermit connection only */
    int t, rc;                          /* (not FTP) */
#ifdef NEWFTP
    extern int ftpget;
    t = ftpget;
    ftpget = 0;
#endif /* NEWFTP */
    rc = clsconnx(x);
#ifdef NEWFTP
    ftpget = t;
#endif /* NEWFTP */
    return(rc);
}

/* May 2002: setlin() decomposition starts here ... */

#ifdef OS2
#define SRVBUFSIZ PIPENAML
#else /* OS2 */
#define SRVBUFSIZ 63
#endif /* OS2 */
#define HOSTNAMLEN 15*65

int netsave = -1;
static char * tmpstring = NULL;
static char * tmpusrid = NULL;

#ifdef SSHCMD
char * sshcmd = NULL;
char * defsshcmd = "ssh -e none";
#else
#ifdef SSHBUILTIN
char * sshrcmd = NULL;
char * sshtmpcmd = NULL;
#endif /* SSHBUILTIN */
#endif /* SSHCMD */

/* c x _ f a i l  --  Common error exit routine for cx_net, cx_line */

int
cx_fail(msg, text) int msg; char * text; {
    makestr(&slmsg,text);		/* For the record (or GUI) */
    if (msg)				/* Not GUI, not quiet, etc */
      printf("?%s\n",text);		/* Print error message */
    slrestor();				/* Restore LINE/HOST to known state */
    return(msg ? -9 : (success = 0));	/* Return appropriate code */
}

#ifdef NETCONN
/* c x _ n e t  --  Make a network connection */

/*
  Call with:
    net      = network type
    protocol = protocol type
    host     = string pointer to host name.
    svc      = string pointer to service or port on host.
    username = username for connection
    password = password for connection
    command  = command to execute
    param1   = Telnet: Authentication type
               SSH:    Version
    param2   = Telnet: Encryption type
               SSH:    Command as Subsystem
    param3   = Telnet: 1 to wait for negotiations, 0 otherwise
               SSH:    X11 Forwarding
    cx       = 1 to automatically enter Connect mode, 0 otherwise.
    sx       = 1 to automatically enter Server mode, 0 otherwise.
    flag     = if no host name given, 1 = close current connection, 0 = resume
    gui      = 1 if called from GUI dialog, 0 otherwise.
  Returns:
    1 on success
    0 on failure and no message printed, slmsg set to failure message.
   -9 on failure and message printed, ditto.
*/
int
#ifdef CK_ANSIC
cx_net( int net, int protocol, char * xhost, char * svc, 
        char * username, char * password, char * command,
        int param1, int param2, int param3, int cx, int sx, int flag, int gui)
#else /* CK_ANSIC */
cx_net(net, protocol, xhost, svc,
       username, password, command,
       param1, param2, param3, cx, sx, flag, gui)
    char * xhost, * svc, * username, *password, *command; 
    int net, protocol, cx, sx, flag, param1, param2, param3, gui; 
#endif /* CK_ANSIC */
/* cx_net */ {

    int i, n, x, msg;
    int _local = -1;

    extern char pwbuf[], * g_pswd;
    extern int pwflg, pwcrypt, g_pflg, g_pcpt, nolocal;

    char srvbuf[SRVBUFSIZ+1];		/* Service */
    char hostbuf[HOSTNAMLEN];		/* Host buffer to manipulate */
    char hostname[HOSTNAMLEN];		/* Copy of host parameter */
    char * host = hostbuf;		/* Pointer to copy of host param */

    if (!xhost) xhost = "";		/* Watch out for null pointers */
    if (!svc) svc = "";
    ckstrncpy(host,xhost,HOSTNAMLEN);	/* Avoid buffer confusion */

    debug(F110,"cx_net host",host,0);
    debug(F111,"cx_net service",svc,SRVBUFSIZ);
    debug(F101,"cx_net network type","",net);

    msg = (gui == 0) && msgflg;		/* Whether to print messages */

#ifndef NODIAL
    debug(F101,"cx_net nnetdir","",nnetdir);
    x = 0;				/* Look up in network directory */
    if (*host == '=') {			/* If number starts with = sign */
	host++;				/* strip it */
	while (*host == SP) host++;	/* and any leading spaces */
	debug(F110,"cx_net host 2",host,0);
	nhcount = 0;
    } else if (*host) {			/* We want to look it up. */
	if (nnetdir > 0)		/* If there is a directory... */
	  x = lunet(host);		/* (sets nhcount) */
	else				/* otherwise */
	  nhcount = 0;			/* we didn't find any there */
	if (x < 0)			/* Internal error? */
	  return(cx_fail(msg,"Network directory lookup error"));
	debug(F111,"cx_net lunet nhcount",host,nhcount);
    }
#endif /* NODIAL */

    /* New connection wanted.  Make a copy of the host name/address... */

    if (clskconnx(1) < 0)		/* Close current Kermit connection */
      return(cx_fail(msg,"Error closing previous connection"));

    if (*host) {			/* They gave a hostname */
	_local = 1;			/* Network connection always local */
	if (mdmsav < 0)
	  mdmsav = mdmtyp;		/* Remember old modem type */
	mdmtyp = -net;			/* Special code for network */
    } else {				/* They just said "set host" */
	host = dftty;			/* So go back to normal */
	_local = dfloc;			/* default tty, location, */
	if (flag) {			/* Close current connection */
	    setflow();			/* Maybe change flow control */
	    haveline = 1;		/* (* is this right? *) */
	    dologend();
#ifndef NODIAL
	    dialsta = DIA_UNK;
#endif /* NODIAL */
#ifdef LOCUS
	    if (autolocus) {
		setlocus(1,1);
	    }
#endif /* LOCUS */
            /* XXX - Is this right? */
	    /* Should we be returning without doing anything ? */
	    /* Yes it's right -- we closed the old connection just above. */
	    return(success = 1);        
	}
    }
    success = 0;
    if (host != line)                   /* line[] is a global */
      ckstrncpy(line,host,LINBUFSIZ);
    ckstrncpy(hostname,host,HOSTNAMLEN);
    ckstrncpy(srvbuf,svc,SRVBUFSIZ+1);

#ifndef NODIAL
    if ((nhcount > 1) && msg) {
	int k;
	printf("%d entr%s found for \"%s\"%s\n",
	       nhcount,
	       (nhcount == 1) ? "y" : "ies",
	       s,
	       (nhcount > 0) ? ":" : "."
	       );
	for (i = 0; i < nhcount; i++) {
		printf("%3d. %-12s => %-9s %s",
		       i+1,n_name,nh_p2[i],nh_p[i]);
	    for (k = 0; k < 4; k++) { /* Also list net-specific items */
		if (nh_px[k][i])      /* free format... */
		  printf(" %s",nh_px[k][i]);
		else
		  break;
	    }
	    printf("\n");
	}
    }
    if (nhcount == 0)
      n = 1;
    else
      n = nhcount;
#else
    n = 1;
    nhcount = 0;
#endif /* NODIAL */

    for (i = 0; i < n; i++) {		/* Loop for each entry found */
	debug(F101,"cx_net loop i","",i);
#ifndef NODIAL
	if (nhcount > 0) {		/* If we found at least one entry... */
	    ckstrncpy(line,nh_p[i],LINBUFSIZ); /* Copy current entry */
	    if (lookup(netcmd,nh_p2[i],nnets,&x) > -1) { /* Net type */
		int xx;
		xx = netcmd[x].kwval;
		/* User specified SSH so don't let net directory override */
		if (net != NET_SSH || xx != NET_TCPB) {
		    net = xx;
		    mdmtyp  = 0 - net;
		}
	    } else {
		makestr(&slmsg,"Network type not supported");
		if (msg)
		  printf("Error - network type \"%s\" not supported\n",
			 nh_p2[i]
		         );
		continue;
	    }
	    switch (net) {		/* Net-specific directory things */
#ifdef SSHBUILTIN
	      case NET_SSH:		/* SSH */
                /* Any SSH specific network directory stuff? */
                break;                  /* NET_SSH */
#endif /* SSHBUILTIN */

	      case NET_TCPB: {		/* TCP/IP TELNET,RLOGIN,... */
#ifdef TCPSOCKET
		  char *q;
		  int flag = 0;

		  /* Extract ":service", if any, from host string */
		  debug(F110,"cx_net service 1",line,0);
		  for (q = line; (*q != '\0') && (*q != ':'); q++)
		    ;
		  if (*q == ':') { *q++ = NUL; flag = 1; }
		  debug(F111,"cx_net service 2",line,flag);

		  /* Get service, if any, from directory entry */

		  if (!*srvbuf) {
		      if (nh_px[0][i]) {
			  ckstrncpy(srvbuf,nh_px[0][i],SRVBUFSIZ);
			  debug(F110,"cx_net service 3",srvbuf,0);
		      }
		      if (flag) {
			  ckstrncpy(srvbuf,q,SRVBUFSIZ);
			  debug(F110,"cx_net service 4",srvbuf,0);
		      }
		  }
		  ckstrncpy(hostname,line,HOSTNAMLEN);

		  /* If we have a service, append to host name/address */
		  if (*srvbuf) {
		      ckstrncat(line, ":", LINBUFSIZ);
		      ckstrncat(line, srvbuf, LINBUFSIZ);
		      debug(F110,"cx_net service 5",line,0);
		  }
#ifdef RLOGCODE
		  /* If no service given but command was RLOGIN */
		  else if (ttnproto == NP_RLOGIN) { /* add this... */
		      ckstrncat(line, ":login",LINBUFSIZ);
		      debug(F110,"cx_net service 6",line,0);
		  }
#ifdef CK_AUTHENTICATION
#ifdef CK_KERBEROS
		  else if (ttnproto == NP_K4LOGIN ||
			   ttnproto == NP_K5LOGIN) { /* add this... */
		      ckstrncat(line, ":klogin",LINBUFSIZ);
		      debug(F110,"cx_net service 7",line,0);
		  }
		  else if (ttnproto == NP_EK4LOGIN ||
			   ttnproto == NP_EK5LOGIN) { /* add this... */
		      ckstrncat(line, ":eklogin",LINBUFSIZ);
		      debug(F110,"cx_net service 8",line,0);
		  }
#endif /* CK_KERBEROS */
#endif /* CK_AUTHENTICATION */
#endif /* RLOGCODE */
		  else {		/* Otherwise, add ":telnet". */
		      ckstrncat(line, ":telnet", LINBUFSIZ);
		      debug(F110,"cx_net service 9",line,0);
		  }
		  if (username) {	/* This is a parameter... */
		      ckstrncpy(uidbuf,username,UIDBUFLEN);
		      uidflag = 1;
		  }
		  /* Fifth field, if any, is user ID (for rlogin) */

		  if (nh_px[1][i] && !uidflag)
		    ckstrncpy(uidbuf,username,UIDBUFLEN);
#ifdef RLOGCODE
		  if (IS_RLOGIN() && !uidbuf[0])
		    return(cx_fail(msg,"Username required"));
#endif /* RLOGCODE */
#endif /* TCPSOCKET */
		  break;
	      }
	      case NET_PIPE:		/* Pipe */
#ifdef NPIPE
		if (!pipename[0]) { /* User didn't give a pipename */
		    if (nh_px[0][i]) { /* But directory entry has one */
			if (strcmp(pipename,"\\pipe\\")) {
			    ckstrncpy(pipename,"\\pipe\\",LINBUFSIZ);
			    ckstrncat(srvbuf,nh_px[0][i],PIPENAML-6);
			} else {
			    ckstrncpy(pipename,nh_px[0][i],PIPENAML);
			}
			debug(F110,"cx_net pipeneme",pipename,0);
		    }
		}
#endif /* NPIPE */
		break;

	      case NET_SLAT:            /* LAT / CTERM */
#ifdef SUPERLAT
		if (!slat_pwd[0]) { /* User didn't give a password */
		    if (nh_px[0][i]) { /* But directory entry has one */
			ckstrncpy(slat_pwd,nh_px[0][i],18);
			debug(F110,"cx_net SuperLAT password",slat_pwd,0);
		    }
		}
#endif /* SUPERLAT */
		break;

	      case NET_SX25:        /* X.25 keyword parameters */
	      case NET_IX25:
	      case NET_VX25: {
#ifdef ANYX25
		  int k;            /* Cycle through the four fields */
		  for (k = 0; k < 4; k++) {
		      if (!nh_px[k][i]) /* Bail out if none left */
			break;
		      if (!ckstrcmp(nh_px[k][i],"cug=",4,0)) {
			  closgr = atoi(nh_px[k][i]+4);
			  debug(F101,"X25 CUG","",closgr);
		      } else if (!ckstrcmp(nh_px[k][i],"cud=",4,0)) {
			  cudata = 1;
			  ckstrncpy(udata,nh_px[k][i]+4,MAXCUDATA);
			  debug(F110,"X25 CUD",cudata,0);
		      } else if (!ckstrcmp(nh_px[k][i],"rev=",4,0)) {
			  revcall = !ckstrcmp(nh_px[k][i]+4,"=on",3,0);
			  debug(F101,"X25 REV","",revcall);
#ifndef IBMX25
		      } else if (!ckstrcmp(nh_px[k][i],"pad=",4,0)) {
			  int x3par, x3val;
			  char *s1, *s2;
			  s1 = s2 = nh_px[k][i]+4; /* PAD parameters */
			  while (*s2) {            /* Pick them apart */
			      if (*s2 == ':') {
				  *s2 = NUL;
				  x3par = atoi(s1);
				  s1 = ++s2;
				  continue;
			      } else if (*s2 == ',') {
				  *s2 = NUL;
				  x3val = atoi(s1);
				  s1 = ++s2;
				  debug(F111,"X25 PAD",x3par,x3val);
				  if (x3par > -1 &&
				      x3par <= MAXPADPARMS)
				    padparms[x3par] = x3val;
				  continue;
			      } else
				s2++;
			  }
#endif /* IBMX25 */
		      }
		  }
#endif /* ANYX25 */
		  break;
	      }
	      default:			/* Nothing special for other nets */
		break;
	    }
	} else
#endif /* NODIAL */
	{				/* No directory entries found. */
	    ckstrncpy(line,hostname,LINBUFSIZ); /* Put this back... */
	    /* If the user gave a TCP service */
	    if (net == NET_TCPB || net == NET_SSH)
	      if (*srvbuf) {		/* Append it to host name/address */
		  ckstrncat(line, ":", LINBUFSIZ);
		  ckstrncat(line, srvbuf,LINBUFSIZ);
	      }
	}
	/*
	   Get here with host name/address and all net-specific
	   parameters set, ready to open the connection.
	*/
	mdmtyp = -net;			/* This should have been done */
					/* already but just in case ... */

	debug(F110,"cx_net net line[] before ttopen",line,0);
	debug(F101,"cx_net net mdmtyp before ttopen","",mdmtyp);
	debug(F101,"cx_net net ttnproto","",ttnproto);

#ifdef SSHBUILTIN
        if (net == NET_SSH) {
            makestr(&ssh_hst,hostname);        /* Stash everything */
            if (username) {
                if (!sl_uid_saved) {
                    ckstrncpy(sl_uidbuf,uidbuf,UIDBUFLEN);
                    sl_uid_saved = 1;
                }
                ckstrncpy(uidbuf,username,UIDBUFLEN);
            }
            if (srvbuf[0]) {
                makestr(&ssh_prt,srvbuf);
            } else
                makestr(&ssh_prt,NULL);

            if (command) {
                makestr(&ssh_cmd,brstrip(command));
                ssh_cas = param2;
            } else
                makestr(&ssh_cmd,NULL);

            if (param1 > -1) {
#ifndef SSHTEST
                if (!sl_ssh_ver_saved) {
                    sl_ssh_ver = ssh_ver;
                    sl_ssh_ver_saved = 1;
                }
#endif /* SSHTEST */
                ssh_ver = param1;
            }
            if (param3 > -1) {
#ifndef SSHTEST
                if (!sl_ssh_xfw_saved) {
                    sl_ssh_xfw = ssh_xfw;
                    sl_ssh_xfw_saved = 1;
                }
#endif /* SSHTEST */
                ssh_xfw = param3;
            }
        } else                          /* NET_SSH */
#endif /* SSHBUILTIN */
#ifdef TCPSOCKET
	  if (net == NET_TCPB) {
            switch (protocol) {
#ifdef CK_SSL
#ifdef COMMENT
/*
  Jeff's version from 30 Dec 2006 - doesn't work - SSL/TLS_RAW still
  start Telnet negotions if a 0xff byte comes in.
*/
	      case NP_SSL_RAW:
                ttnproto = NP_SSL_RAW;
		debug(F101,"NP_SSL_RAW ttnproto","",ttnproto);
                ssl_only_flag = 1;
                tls_only_flag = 0;
                break;

	      case NP_TLS_RAW:
		  ttnproto = NP_TLS_RAW;
		  debug(F101,"NP_TLS_RAW ttnproto","",ttnproto);
		  ssl_only_flag = 0;
		  tls_only_flag = 1;
		  break;

	      case NP_SSL:
                ttnproto = NP_SSL;
		debug(F101,"NP_SSL ttnproto","",ttnproto);
                ssl_only_flag = 1;
                tls_only_flag = 0;
                break;

	      case NP_TLS:
                ttnproto = NP_TLS;
		debug(F101,"NP_TLS ttnproto","",ttnproto);
                ssl_only_flag = 0;
                tls_only_flag = 1;
                break;

	      case NP_SSL_TELNET:
                ttnproto = NP_TELNET;
		debug(F101,"NP_SSL_TELNET ttnproto","",ttnproto);
                ssl_only_flag = 1;
                tls_only_flag = 0;
                break;

	      case NP_TLS_TELNET:
                ttnproto = NP_TELNET;
		debug(F101,"NP_TLS_TELNET ttnproto","",ttnproto);
                ssl_only_flag = 0;
                tls_only_flag = 1;
                break;
#else
/* fdc version of 4 Dec 2006 works OK */
	      case NP_SSL_RAW:
	      case NP_SSL:
		ssl_raw_flag = (protocol == NP_SSL_RAW) ? 1 : 0;
                ttnproto = protocol;
		debug(F101,protocol==NP_SSL ?
		      "NP_SSL ttnproto" :
		      "NP_SSL_RAW ttnproto",
		      "",ttnproto);
                ssl_only_flag = 1;
                tls_only_flag = 0;
                break;

	      case NP_TLS:
	      case NP_TLS_RAW:
		tls_raw_flag = (protocol == NP_SSL_RAW) ? 1 : 0;
                ttnproto = protocol;
		debug(F101,protocol==NP_TLS ?
		      "NP_TLS ttnproto" :
		      "NP_TLS_RAW ttnproto",
		      "",ttnproto);
                ssl_only_flag = 0;
                tls_only_flag = 1;
                break;

	      case NP_SSL_TELNET:
		ssl_raw_flag = 0;
                ttnproto = NP_TELNET;
		debug(F101,"NP_SSL_TELNET ttnproto","",ttnproto);
                ssl_only_flag = 1;
                tls_only_flag = 0;
                break;

	      case NP_TLS_TELNET:
		tls_raw_flag = 0;
                ttnproto = NP_TELNET;
		debug(F101,"NP_TLS_TELNET ttnproto","",ttnproto);
                ssl_only_flag = 0;
                tls_only_flag = 1;
                break;
#endif	/* COMMENT */
#endif /* CK_SSL */

	      case NP_NONE:
	      case NP_TCPRAW:
	      case NP_RLOGIN:
	      case NP_K4LOGIN:
	      case NP_K5LOGIN:
	      case NP_EK4LOGIN:
	      case NP_EK5LOGIN:
	      case NP_TELNET:
	      case NP_KERMIT:
	      default:
                ttnproto = protocol;
#ifdef CK_SSL
#ifdef COMMENT
		/* Jeff version from 30 Dec 2006 */
                ssl_only_flag = 0;
                tls_only_flag = 0;
#else
		/* fdc version from 4 Dec 2006 */
		ssl_raw_flag = 0;
		tls_raw_flag = 0;
                ssl_only_flag = 0;
                tls_only_flag = 0;
#endif	/* COMMENT */
#endif /* CK_SSL */
                break;
            }
#ifdef CK_AUTHENTICATION
            if ((ttnproto == NP_TELNET || ttnproto == NP_KERMIT) &&
		param1 > -1) {
	    if (!sl_auth_saved) {
		int x;
		for (x = 0; x < AUTHTYPLSTSZ; x++)
		  sl_auth_type_user[x] = auth_type_user[x];
		sl_auth_saved = 1;
	    }
	    if (!sl_topt_a_s_saved) {
		sl_topt_a_su = TELOPT_DEF_S_U_MODE(TELOPT_AUTHENTICATION);
		sl_topt_a_s_saved = 1;
	    }
	    if (!sl_topt_a_c_saved) {
		sl_topt_a_cm = TELOPT_DEF_C_ME_MODE(TELOPT_AUTHENTICATION);
		sl_topt_a_c_saved = 1;
	    }
	    switch (param1) {
	      case AUTHTYPE_AUTO:
		auth_type_user[0] = AUTHTYPE_AUTO;
		TELOPT_DEF_S_U_MODE(TELOPT_AUTHENTICATION) = TN_NG_RQ;
		TELOPT_DEF_C_ME_MODE(TELOPT_AUTHENTICATION) = TN_NG_RQ;
		break;
	      case AUTHTYPE_NULL:
		auth_type_user[0] = AUTHTYPE_NULL;
		TELOPT_DEF_S_U_MODE(TELOPT_AUTHENTICATION) = TN_NG_RF;
		TELOPT_DEF_C_ME_MODE(TELOPT_AUTHENTICATION) = TN_NG_RF;
		break;
#ifdef CK_SRP
	      case AUTHTYPE_SRP:
		auth_type_user[0] = AUTHTYPE_SRP;
		auth_type_user[1] = AUTHTYPE_NULL;
		TELOPT_DEF_S_U_MODE(TELOPT_AUTHENTICATION) = TN_NG_MU;
		TELOPT_DEF_C_ME_MODE(TELOPT_AUTHENTICATION) = TN_NG_MU;
		break;
#endif /* CK_SRP */
#ifdef CK_SSL
	      case AUTHTYPE_SSL:
		auth_type_user[0] = AUTHTYPE_SSL;
		auth_type_user[1] = AUTHTYPE_NULL;
		TELOPT_DEF_S_U_MODE(TELOPT_AUTHENTICATION) = TN_NG_MU;
		TELOPT_DEF_C_ME_MODE(TELOPT_AUTHENTICATION) = TN_NG_MU;
		break;
#endif /* CK_SSL */
#ifdef NT
	      case AUTHTYPE_NTLM:
		auth_type_user[0] = AUTHTYPE_NTLM;
		auth_type_user[1] = AUTHTYPE_NULL;
		TELOPT_DEF_S_U_MODE(TELOPT_AUTHENTICATION) = TN_NG_MU;
		TELOPT_DEF_C_ME_MODE(TELOPT_AUTHENTICATION) = TN_NG_MU;
		break;
#endif /* NT */
#ifdef CK_KERBEROS
	      case AUTHTYPE_KERBEROS_V4:
		auth_type_user[0] = AUTHTYPE_KERBEROS_V4;
		auth_type_user[1] = AUTHTYPE_NULL;
		TELOPT_DEF_S_U_MODE(TELOPT_AUTHENTICATION) = TN_NG_MU;
		TELOPT_DEF_C_ME_MODE(TELOPT_AUTHENTICATION) = TN_NG_MU;
		break;

	      case AUTHTYPE_KERBEROS_V5:
		auth_type_user[0] = AUTHTYPE_KERBEROS_V5;
		auth_type_user[1] = AUTHTYPE_NULL;
		TELOPT_DEF_S_U_MODE(TELOPT_AUTHENTICATION) = TN_NG_MU;
		TELOPT_DEF_C_ME_MODE(TELOPT_AUTHENTICATION) = TN_NG_MU;
		break;
#endif /* CK_KERBEROS */
	    }
	}
	/*
	   If the user requires a particular type of Kerberos connection,
	   make sure we have a valid TGT.
	*/
	makestr(&slmsg,"Authentication failure");
	if ((ttnproto == NP_TELNET || ttnproto == NP_KERMIT) &&
	    (line[0] == '*' &&
	     TELOPT_DEF_S_U_MODE(TELOPT_AUTHENTICATION) == TN_NG_MU ||
	     line[0] != '*' &&
	     TELOPT_DEF_C_ME_MODE(TELOPT_AUTHENTICATION) == TN_NG_MU)
	    ) {
#ifdef CK_KERBEROS
	    if ( auth_type_user[0] == AUTHTYPE_KERBEROS_V4 ) {
		extern int krb4_autoget;
		if (!ck_krb4_is_installed())
		  return(cx_fail(msg,
	      "Required authentication method (Kerberos 4) is not installed"));
#ifdef COMMENT
		/* This code results in false failures when using */
		/* kerberos to machines in realms other than the  */
		/* default since we don't know the realm of the   */
		/* other machine until perform the reverse DNS    */
		/* lookup.                                        */
		else if (line[0] != '*' && !ck_krb4_is_tgt_valid() &&
			   (!krb4_autoget ||
			    krb4_autoget && !ck_krb4_autoget_TGT(NULL))) {
		    return(cx_fail(msg,
			   "Kerberos 4: Ticket Getting Ticket not valid"));
		}
#endif /* COMMENT */
	    } else if (auth_type_user[0] == AUTHTYPE_KERBEROS_V5) {
		extern int krb5_autoget;
		if (!ck_krb5_is_installed()) {
		    return(cx_fail(msg,
	   "Required authentication method (Kerberos 5) is not installed"));
		}
#ifdef COMMENT
		/* This code results in false failures when using */
		/* kerberos to machines in realms other than the  */
		/* default since we don't know the realm of the   */
		/* other machine until perform the reverse DNS    */
		/* lookup.                                        */
		else if (line[0] != '*' && !ck_krb5_is_tgt_valid() &&
			   (!krb5_autoget ||
			    krb5_autoget && !ck_krb5_autoget_TGT(NULL))) {
		    return(cx_fail(msg,
			 "Kerberos 5: Ticket Getting Ticket not valid."));
		}
#endif /* COMMENT */
	    }
#endif /* CK_KERBEROS */
#ifdef NT
	    if (auth_type_user[0] == AUTHTYPE_NTLM) {
		if (!ck_ntlm_is_installed()) {
		    return(cx_fail(msg,
		   "Required authentication method (NTLM) is not installed"));
		} else if (line[0] != '*' && !ck_ntlm_is_valid(0)) {
		    return(cx_fail(msg,"NTLM: Credentials are unavailable."));
		}
	    }
#endif /* NT */
#ifdef CK_SSL
	    if (auth_type_user[0] == AUTHTYPE_SSL) {
		if (!ck_ssleay_is_installed()) {
		    return(cx_fail(msg,
		     "Required authentication method (SSL) is not installed"));
		}
	    }
#endif /* CK_SSL */
#ifdef CK_SRP
	    if (auth_type_user[0] == AUTHTYPE_SRP) {
		if (!ck_srp_is_installed()) {
		    return(cx_fail(msg,
		     "Required authentication method (SRP) is not installed"));
		}
	    }
#endif /* CK_SRP */
	}
#endif /* CK_AUTHENTICATION */
#ifdef CK_ENCRYPTION
	if ((ttnproto == NP_TELNET || ttnproto == NP_KERMIT) &&
	     param2 > -1) {
	    if (!sl_cx_saved) {
		sl_cx_type = cx_type;
		sl_cx_saved = 1;
	    }
	    if (!sl_topt_e_s_saved) {
		sl_topt_e_su = TELOPT_DEF_S_U_MODE(TELOPT_ENCRYPTION);
		sl_topt_e_sm = TELOPT_DEF_S_ME_MODE(TELOPT_ENCRYPTION);
		sl_topt_e_s_saved = 1;
	    }
	    if (!sl_topt_e_c_saved) {
		sl_topt_e_cu = TELOPT_DEF_C_U_MODE(TELOPT_ENCRYPTION);
		sl_topt_e_cm = TELOPT_DEF_C_ME_MODE(TELOPT_ENCRYPTION);
		sl_topt_e_c_saved = 1;
	    }
	    cx_type = param2;
	    if (cx_type == CX_AUTO) {
		TELOPT_DEF_S_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RQ;
		TELOPT_DEF_S_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RQ;
		TELOPT_DEF_C_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RQ;
		TELOPT_DEF_C_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RQ;
	    } else if (cx_type == CX_NONE) {
		TELOPT_DEF_S_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
		TELOPT_DEF_S_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
		TELOPT_DEF_C_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
		TELOPT_DEF_C_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
	    } else {
		TELOPT_DEF_S_U_MODE(TELOPT_ENCRYPTION) = TN_NG_MU;
		TELOPT_DEF_S_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_MU;
		TELOPT_DEF_C_U_MODE(TELOPT_ENCRYPTION) = TN_NG_MU;
		TELOPT_DEF_C_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_MU;
	    }
	}
	if (ttnproto == NP_EK4LOGIN || ttnproto == NP_EK5LOGIN ||
	    (ttnproto == NP_TELNET || ttnproto == NP_KERMIT) &&
	    ((line[0] == '*' &&
	      TELOPT_DEF_S_U_MODE(TELOPT_ENCRYPTION) == TN_NG_MU &&
	      TELOPT_DEF_S_ME_MODE(TELOPT_ENCRYPTION) == TN_NG_MU) ||
	     (line[0] != '*' &&
	      TELOPT_DEF_C_U_MODE(TELOPT_ENCRYPTION) == TN_NG_MU &&
	      TELOPT_DEF_C_ME_MODE(TELOPT_ENCRYPTION) == TN_NG_MU))
	    ) {
	    if (!ck_crypt_is_installed()) {
		return(cx_fail(msg,
		  "Required Encryption methods are not installed"));
	    }
	}
#endif /* CK_ENCRYPTION */
#ifdef RLOGCODE
#ifdef CK_KERBEROS
#ifdef KRB4
	if (ttnproto == NP_K4LOGIN || ttnproto == NP_EK4LOGIN) {
	    extern int krb4_autoget;
	    char tgt[256];
	    char * realm;

	    /* We don't have the full hostname at yet so  */
	    /* we do a DNS lookup before calling ttopen() */ 

	    realm = ck_krb4_realmofhost(ckgetfqhostname(hostname));
	    ckmakmsg(tgt,256,"krbtgt.",realm,"@",realm);
	    if (!ck_krb4_is_installed()) {
		return(cx_fail(msg,
		 "Required authentication method (Kerberos 4) is not installed"
			       ));
	    } else {
		if ((ck_krb4_tkt_isvalid(tgt) <= 0) &&
		    (!krb4_autoget ||
		     krb4_autoget && !ck_krb4_autoget_TGT(realm))) {
		    return(cx_fail(msg,
			   "Kerberos 4: Ticket Getting Ticket not valid"));
		}
	    }
	}
#endif /* KRB4 */
#ifdef KRB5
	if (ttnproto == NP_K5LOGIN || ttnproto == NP_EK5LOGIN ||
	    ttnproto == NP_K5U2U)
	{
	    extern int krb5_autoget;
	    char tgt[256];
	    char * realm;

	    /* Must get full hostname before calling ttopen() */

	    realm = ck_krb5_realmofhost(ckgetfqhostname(hostname));
	    ckmakmsg(tgt,256,"krbtgt/",realm,"@",realm);

	    if (!ck_krb5_is_installed()) {
		return(cx_fail(msg,
                 "Required authentication method (Kerberos 5) not installed"));
	    } else if (!((ck_krb5_tkt_isvalid(NULL,tgt) > 0) ||
			  ck_krb5_is_tgt_valid()) &&
		       (!krb5_autoget ||
			krb5_autoget && !ck_krb5_autoget_TGT(realm))) {
		return(cx_fail(msg,
		       "Kerberos 5: Ticket Getting Ticket not valid."));
	    }
	}
#endif /* KRB5 */
#endif /* CK_KERBEROS */
#endif /* RLOGCODE */

#ifndef NOSPL
#ifdef RLOGCODE
	if (username) {
	    if (!sl_uid_saved) {
		ckstrncpy(sl_uidbuf,uidbuf,UIDBUFLEN);
		sl_uid_saved = 1;
	    }
	    ckstrncpy(uidbuf,username,UIDBUFLEN);
	    uidflag = 1;
	}
#endif /* RLOGCODE */
#ifdef TNCODE
	if (!sl_tn_saved) {
	    sl_tn_wait = tn_wait_flg;
	    sl_tn_saved = 1;
	}
	tn_wait_flg = param3;
#endif /* TNCODE */
#endif /* NOSPL */
	} /* if (net == NET_TCPB) */
#endif /* TCPSOCKET */

#ifndef NOSPL
#ifdef CK_SECURITY
	if (password) {
	    if (password[0]) {
		ckstrncpy(pwbuf,password,PWBUFL+1);
		pwflg = 1;
		pwcrypt = 0;
	    } else
		pwflg = 0;
	}
#endif /* CK_SECURITY */
#endif /* NOSPL */

	/* Try to open - network */
	ckstrncpy(ttname,line,TTNAMLEN);
	y = ttopen(line, &_local, mdmtyp, 0 );

#ifndef NOHTTP
	/*  If the connection failed and we are using an HTTP Proxy
	 *  and the reason for the failure was an authentication
	 *  error, then we need to give the user to ability to
	 *  enter a username and password, just like a browser.
	 *
	 *  I tried to do all of this within the netopen() call
	 *  but it is much too much work.
	 */
	while (y < 0 && tcp_http_proxy != NULL ) {

	    if (tcp_http_proxy_errno == 401 ||
		tcp_http_proxy_errno == 407 ) {
		char uid[UIDBUFLEN];
		char pwd[256];
                struct txtbox tb[2];
                int ok;

                tb[0].t_buf = uid;
                tb[0].t_len = UIDBUFLEN;
                tb[0].t_lbl = "Proxy Userid: ";
                tb[0].t_dflt = NULL;
                tb[0].t_echo = 1;
                tb[1].t_buf = pwd;
                tb[1].t_len = 256;
                tb[1].t_lbl = "Proxy Passphrase: ";
                tb[1].t_dflt = NULL;
                tb[1].t_echo = 2;

                ok = uq_mtxt("Proxy Server Authentication Required\n",
                              NULL, 2, tb);

		if (ok && uid[0]) {
		    char * proxy_user, * proxy_pwd;

		    proxy_user = tcp_http_proxy_user;
		    proxy_pwd  = tcp_http_proxy_pwd;

		    tcp_http_proxy_user = uid;
		    tcp_http_proxy_pwd = pwd;

		    ckstrncpy(ttname,line,TTNAMLEN);
		    y = ttopen(line, &_local, mdmtyp, 0);
		    memset(pwd,0,sizeof(pwd));
		    tcp_http_proxy_user = proxy_user;
		    tcp_http_proxy_pwd = proxy_pwd;
		} else
		  break;
	    } else
	      break;
	}
#endif /* NOHTTP */
	if (y < 0) {
	    slrestor();
	    makestr(&slmsg,"Network connection failure");
#ifdef VMS
	    if (msg && hints && !xcmdsrc && IS_RLOGIN()) {
		makestr(&slmsg,"RLOGIN failure");
		if  (socket_errno == EACCES) {
		    printf("*************************\n");
		    printf(
	   "Hint: RLOGIN requires privileges to open an outbound port.\n");
		    printf(
		    "(Use SET HINTS OFF to suppress future hints.)\n");
		    printf("*************************\n");
		}
	    }
#else  /* Not VMS... */
	    if (errno) {
		int x;
		debug(F111,"set host line, errno","",errno);
		makestr(&slmsg,ck_errstr());
		if (msg) {
#ifdef OS2
		    printf("Can't connect to %s\n",line);
#else /* OS2 */
#ifdef UNIX
		    if (hints && !xcmdsrc && IS_RLOGIN()) {
			makestr(&slmsg,"RLOGIN failure");
			printf("*************************\n");
			printf(
	 "Hint: RLOGIN requires privileges to open an outbound port.\n");
			printf(
	 "(Use SET HINTS OFF to suppress future hints.)\n");
			printf("*************************\n");
		    }
#endif /* UNIX */
#endif /* OS2 */
		} else printf("Can't connect to %s\n",line);
	    } else
#endif /* VMS */
	      if (msg) printf("Can't open connection to %s\n",line);
	    continue;
	} else {
	    success = 1;
#ifndef NODIAL
	    dialsta = DIA_UNK;
#endif /* NODIAL */
	    switch (net) {
	      case NET_TCPA:
	      case NET_TCPB:
		cxtype = CXT_TCPIP;
#ifdef COMMENT
/* This works but it messes up interactive anonymous login */
#ifndef NOXFER
#ifdef IKS_OPTION
		/* If we have connected to an Internet Kermit service */
		/* and a /USER: switch was given, then log in. */

		if (TELOPT_U(TELOPT_KERMIT) || TELOPT_ME(TELOPT_KERMIT)) {
		    debug(F111,"cx_net IKSD /USER:",uidbuf,haveuser);
		    if (haveuser /* && cx == 0 */ ) { /* /USER: given */
			char * psw = pwbuf; /* Do we have a password? */
			if (!*psw) {        /* No... */
			    if (!strcmp(uidbuf,"anonymous") ||
				!strcmp(uidbuf,"ftp")) {
				extern char myhost[];
				char * u = (char *)sl_uidbuf;
				char * h = (char *)myhost;
				if (!*u) u = "nobody";
				if (!*h) h = "nowhere";
				ckmakmsg(tmpbuf,TMPBUFSIZ,u,"@",h,NULL);
				psw = tmpbuf;
				debug(F110,"cx_net IKSD anon",psw,0);
			    } else {
				readpass(" Password: ",pwbuf,PWBUFL);
			    }
			}
			sstate = setgen('I',uidbuf,psw,"");
		    }
		}
#endif /* IKS_OPTION */
#endif /* NOXFER */
#endif /* COMMENT */
		break;
	      case NET_SSH:
		cxtype = CXT_SSH;
		duplex = 0;         /* Remote echo */
		break;
	      case NET_SLAT:
		cxtype = CXT_LAT;
		break;
	      case NET_SX25:
	      case NET_IX25:
	      case NET_HX25:
	      case NET_VX25:
		cxtype = CXT_X25;
		break;
	      case NET_BIOS:
		cxtype = CXT_NETBIOS;
		break;
	      case NET_FILE:
	      case NET_PIPE:
	      case NET_CMD:
	      case NET_DLL:
	      case NET_PTY:
		cxtype = CXT_PIPE;
		break;
	      default:
		cxtype = CXT_PIPE;
		break;
	    }
	    break;
	}
    } /* for-loop */
    s = line;

    debug(F101,"cx_net post ttopen success","",success);
    if (!success) {
        local = dfloc;                  /* Go back to normal */
#ifndef MAC
        ckstrncpy(ttname,dftty,TTNAMLEN); /* Restore default tty name */
#endif /* MAC */
        speed = ttgspd();
        network = 0;                    /* No network connection active */
        haveline = 0;
        if (mdmtyp < 0) {               /* Switching from net to async? */
            if (mdmsav > -1)            /* Restore modem type from last */
              mdmtyp = mdmsav;          /* SET MODEM command, if any. */
            else
              mdmtyp = 0;
            mdmsav = -1;
        }
        return(0);                      /* Return failure */
    }
    if (_local > -1) local = _local;    /* Opened ok, set local/remote. */
    makestr(&slmsg,NULL);
    network = (mdmtyp < 0);             /* Remember connection type. */
    ckstrncpy(ttname,s,TTNAMLEN);       /* Copy name into real place. */
    debug(F110,"cx_net ok",ttname,0);
    debug(F101,"cx_net network","",network);
#ifndef NOXFER
    if ((reliable != SET_OFF || !setreliable)) /* Assume not reliable. */
      reliable = SET_OFF;
#endif /* NOXFER */
    if (!network || istncomport())	
      speed = ttgspd();                 /* Get the current speed. */
    debug(F101,"cx_net local","",local);
    if (network) {
        debug(F101,"cx_net net","",net);
#ifndef NOXFER
        /* Force prefixing of 255 on TCP/IP connections... */
        if (net == NET_TCPB
#ifdef SSHBUILTIN
             || net == NET_SSH
#endif /* SSHBUILTIN */
             ) {
            debug(F101,"cx_net reliable A","",reliable);
#ifdef CK_SPEED
            ctlp[(unsigned)255] = 1;
#endif /* CK_SPEED */
            if ((reliable != SET_OFF || !setreliable)) {
#ifdef TN_COMPORT
                if (istncomport()) {    /* Telnet communication port */
                    reliable = SET_OFF; /* Transport is not reliable */
                    debug(F101,"cx_net reliable istncomport()","",1);
                } else {
                    reliable = SET_ON;  /* Transport is reliable end to end */
                    debug(F101,"cx_net reliable istncomport()","",0);
                }
#else
                reliable = SET_ON;      /* Transport is reliable end to end */
#endif /* ifdef TN_COMPORT */
            }
            debug(F101,"cx_net reliable B","",reliable);
        } else if (net == NET_SX25 ||
                   net == NET_VX25 ||
                   net == NET_IX25 ||
                   net == NET_HX25) {
            duplex = 1;                 /* Local echo for X.25 */
            if (reliable != SET_OFF || !setreliable)
              reliable = SET_ON;        /* Transport is reliable end to end */
        }
#endif /* NOXFER */
    }
#ifndef NOXFER
    debug(F101,"cx_net reliable","",reliable);
#endif /* NOXFER */
#ifdef OS2
    if (mdmtyp <= 0)                    /* Network or Direct Connection */
      DialerSend(OPT_KERMIT_CONNECT, 0);
#endif /* OS2 */

  xcx_net:

    setflow();                          /* Set appropriate flow control */

    haveline = 1;
#ifdef CKLOGDIAL
    dolognet();
#endif /* CKLOGDIAL */

#ifndef NOSPL
    if (local) {
        if (nmac) {                     /* Any macros defined? */
            int k;                      /* Yes */
            k = mlook(mactab,"on_open",nmac);   /* Look this up */
            if (k >= 0) {                       /* If found, */
                if (dodo(k,ttname,0) > -1)      /* set it up, */
                  parser(1);                    /* and execute it */
            }
        }
    }
#endif /* NOSPL */

    if (local && (cx || sx)) {          /* /CONNECT or /SERVER switch given */
        if (cx) {                       /* /CONNECT */
	    if (!gui) {
		/* Command was confirmed so we can pre-pop command level.  */
		/* This is so CONNECT module won't think we're executing a */
		/* script if CONNECT was the final command in the script.  */
		if (cmdlvl > 0)
		  prepop();
	    }
#ifndef NODIAL
            dialsta = DIA_UNK;
#endif /* NODIAL */
#ifdef LOCUS
            if (autolocus) {
		setlocus(1,1);
            }
#endif /* LOCUS */
            success = doconect(0, cmdlvl == 0 ? 1 : 0);
            if (ttchk() < 0)
              dologend();
	    debug(F101,"cx_net post doconect success","",success);
            return(success);
#ifndef NOXFER
        } else if (sx) {                /* /SERVER */
            sstate = 'x';
#ifdef MAC
            what = W_RECV;
            scrcreate();
#endif /* MAC */
            if (local) displa = 1;
#ifdef AMIGA
            reqoff();                   /* No DOS requestors while server */
#endif /* AMIGA */
#endif /* NOXFER */
        }
    }
#ifndef NODIAL
    dialsta = DIA_UNK;
#endif /* NODIAL */
#ifdef LOCUS
    if (autolocus) {
        setlocus(1,1);
    }
#endif /* LOCUS */
    return(success = 1);
}
#endif /* NETCONN */

/* c x _ s e r i a l  --  Make a serial connection */

/*
  Call with:
    device  = string pointer to device name.
    cx      = 1 to automatically enter Connect mode, 0 otherwise.
    sx      = 1 to automatically enter Server mode, 0 otherwise.
    shr     = 1 if device should be opened in shareable mode, 0 otherwise.
    flag    = if no dev name given: 1 = close current connection, 0 = resume.
    gui     = 1 if called from GUI dialog, 0 otherwise.
  Returns:
    1 on success
    0 on failure and no message printed, slmsg set to failure message.
   -9 on failure and message printed, ditto.
*/

/* these are bit flags */
#define CX_TAPI 1
#define CX_PPP  2
#define CX_SLIP 4

int
#ifdef CK_ANSIC
cx_serial(char *device, 
          int cx, int sx, int shr, int flag, int gui, int special)
#else /* CK_ANSIC */
cx_serial(device, cx, sx, shr, flag, gui, special)
    char * device; int cx, sx, shr, flag, gui, special; 
#endif /* CK_ANSIC */
/* cx_serial */ {
    int i, n, x, y, msg;
    int _local = -1;
    char *s;

    debug(F110,"cx_serial device",device,0);
    s = device;
    msg = (gui == 0) && msgflg;		/* Whether to print messages */
    success = 0;

#ifndef NODIAL
    dialsta = DIA_UNK;
#endif /* NODIAL */
    debug(F101,"cx_serial mdmtyp","",mdmtyp);
    if (clskconnx(1) < 0)		/* Close the Kermit connection */
      return(success = 0);
    if (*s) {				/* They gave a device name */
	_local = -1;			/* Let ttopen decide about it */
    } else {				/* They just said "set line" */
	s = dftty;			/* so go back to normal tty */
	_local = dfloc;			/* and mode. */
    }
#ifdef VMS
    {
	extern int ok_to_share;
	ok_to_share = shr;
    }
#endif /* VMS */

#ifdef OS2                              /* Must wait until after ttclos() */
#ifdef NT                               /* to change these settings       */
#ifdef CK_TAPI
    tttapi = special & CX_TAPI;
#endif /* CK_TAPI */
#else
    ttslip = special & CX_SLIP;
    ttppp  = special & CX_PPP;
#endif /* NT */
    ttshare = shr;			/* Shareable device ? */
    debug(F110,"OS2 SET PORT final s",s,"");
#endif /* OS2 */

    /* Open the new line */        

    ckstrncpy(ttname,s,TTNAMLEN);
    if ((y = ttopen(s,&_local,mdmtyp,cdtimo)) > -1) {
	cxtype = (mdmtyp > 0) ? CXT_MODEM : CXT_DIRECT;
#ifndef NODIAL
	dialsta = DIA_UNK;
#ifdef CK_TAPI
	/* if the line is a tapi device, then we need to auto-execute */
	/* SET MODEM TYPE TAPI - which we do the equivalent of here.  */
	if (tttapi) {
	    extern int usermdm;
	    usermdm = 0;
	    initmdm(38);		/* From ckudia.c n_TAPI == 38 */
	}
#endif /* CK_TAPI */
#endif /* NODIAL */
	success = 1;
    } else {				/* Failed */
#ifdef OS2ONLY
	if (!strcmp(s,dftty))   /* Do not generate an error with dftty */
	  ;
	else if (y == -6 && ttslip) {
	    makestr(&slmsg,"Can't access SLIP driver");
	    if (msg) printf("?%s\n",slmsg);
	} else if (y == -6 && ttppp) {
	    makestr(&slmsg,"Can't access PPP driver");
	    if (msg) printf("?%s\n",slmsg);
	} else
#endif /* OS2ONLY */
	  if (y == -2) {
	      makestr(&slmsg,"Timed out - no carrier");
	      if (msg) {
		  printf("?%s\n",slmsg);
		  if (hints) {
		      printf("\n*************************\n");
		      printf(
		       "HINT (Use SET HINTS OFF to suppress future hints):\n");
		      printf(
			  "Try SET CARRIER OFF and SET LINE again, or else\n");
		      printf("SET MODEM, SET LINE, and then DIAL.\n");
		      printf("*************************\n\n");
		  }
	      }
	  } else if (y == -3) {
	      makestr(&slmsg,"Access to lock denied");
	      if (msg) {
#ifdef UNIX
		  printf(
		   "Sorry, write access to UUCP lockfile directory denied.\n");
#ifndef NOHINTS
		  if (hints) {
		      printf("\n*************************\n");
		      printf(
		       "HINT (Use SET HINTS OFF to suppress future hints):\n");
		      printf(
	  "Please read the installation instructions file, %sckuins.txt,\n",
	                     k_info_dir ? k_info_dir : ""
                             );
		      printf(
	  "or the UNIX appendix of the manual, \"Using C-Kermit\"\n"
                             );
		      printf(
          "or visit http://www.columbia.edu/kermit/ckuins.html \n"
                             );
		      printf("*************************\n\n");
		  }
#endif /* NOHINTS */
#else
		  printf("Sorry, access to lock denied: %s\n",s);
#endif /* UNIX */
	      }
	  } else if (y == -4) {
	      makestr(&slmsg,"Access to device denied");
	      if (msg) {
		  printf("Sorry, access to device denied: %s\n",s);
#ifdef UNIX
#ifndef NOHINTS
		  if (hints) {
		      printf("\n*************************\n");
		      printf(
		      "HINT (Use SET HINTS OFF to suppress future hints):\n");
		      printf(
	    "Please read the installation instructions file, %sckuins.txt,\n",
                             k_info_dir ? k_info_dir : ""
                             );
		      printf(
	    "or the UNIX appendix of the manual, \"Using C-Kermit\".\n"
                             );
		      printf("*************************\n\n");
		  }
#endif /* NOHINTS */
#endif /* UNIX */
	      }
	  } else if (y == -5) {
	      makestr(&slmsg,"Device is in use or unavailable");
	      if (msg)
#ifdef VMS
		printf(
		  "Sorry, device is in use or otherwise unavailable: %s\n",s);
#else
	      printf("Sorry, device is in use: %s\n",s);
#endif /* VMS */
	  } else {			/* Other error. */
	      makestr(&slmsg,"Device open failed");
	      if (
#ifdef VMS
		  1
#else
		  errno
#endif /* VMS */
		  ) {
		  int x;		/* Find a safe, long buffer */
		  makestr(&slmsg,ck_errstr());
#ifndef VMS
		  debug(F111,"cx_serial serial errno",slmsg,errno);
#endif /* VMS */
		  if (msg)
		    printf("Connection to %s failed: %s\n",s,slmsg);
	      } else if (msg)
		printf("Sorry, can't open connection: %s\n",s);
	  }
    }
    network = 0;			/* No network connection active */
    speed = ttgspd();
    if (!success) {
        local = dfloc;                  /* Go back to normal */
#ifndef MAC
        ckstrncpy(ttname,dftty,TTNAMLEN); /* Restore default tty name */
#endif /* MAC */
        haveline = 0;
        if (mdmtyp < 0) {               /* Switching from net to async? */
            if (mdmsav > -1)            /* Restore modem type from last */
              mdmtyp = mdmsav;          /* SET MODEM command, if any. */
            else
              mdmtyp = 0;
            mdmsav = -1;
        }
        return(msg ? -9 : 0);		/* Return failure */
    }
    if (_local > -1)
      local = _local;			/* Opened ok, set local/remote. */
    makestr(&slmsg,NULL);		/* Erase SET LINE message */
    ckstrncpy(ttname,s,TTNAMLEN);       /* Copy name into real place. */
    debug(F110,"cx_serial ok",ttname,0);
#ifndef NOXFER
    if ((reliable != SET_OFF || !setreliable)) /* Assume not reliable. */
      reliable = SET_OFF;
#endif /* NOXFER */

  xcx_serial:
    setflow();                          /* Set appropriate flow control */
    haveline = 1;
#ifdef CKLOGDIAL
      dologline();
#endif /* CKLOGDIAL */

#ifndef NOSPL
    if (local) {
        if (nmac) {                     /* Any macros defined? */
            int k;                      /* Yes */
            k = mlook(mactab,"on_open",nmac);   /* Look this up */
            if (k >= 0) {                       /* If found, */
                if (dodo(k,ttname,0) > -1)      /* set it up, */
                  parser(1);                    /* and execute it */
            }
        }
    }
#endif /* NOSPL */

    if (local && (cx || sx)) {          /* /CONNECT or /SERVER switch given */
        extern int carrier;
        if (carrier != CAR_OFF) {	/* Looking for carrier? */
            /* Open() turns on DTR -- wait up to a second for CD to come up */
            int i, x;
            for (i = 0; i < 10; i++) {  /* WAIT 1 CD... */
                x = ttgmdm();
                if (x < 0 || x & BM_DCD)
                  break;
                msleep(100);
            }
        }
        if (cx) {                       /* /CONNECT */
            /* Command was confirmed so we can pre-pop command level. */
            /* This is so CONNECT module won't think we're executing a */
            /* script if CONNECT was the final command in the script. */

            if (cmdlvl > 0)
              prepop();
#ifndef NODIAL
            dialsta = DIA_UNK;
#endif /* NODIAL */
#ifdef LOCUS
            if (autolocus) {
                setlocus(1,1);
            }
#endif /* LOCUS */
            success = doconect(0, cmdlvl == 0 ? 1 : 0);
            if (ttchk() < 0)
              dologend();
            return(success);
#ifndef NOXFER
        } else if (sx) {                /* /SERVER */
            sstate = 'x';
#ifdef MAC
            what = W_RECV;
            scrcreate();
#endif /* MAC */
            if (local) displa = 1;
#ifdef AMIGA
            reqoff();                   /* No DOS requestors while server */
#endif /* AMIGA */
#endif /* NOXFER */
        }
    }
#ifndef NODIAL
    dialsta = DIA_UNK;
#endif /* NODIAL */
#ifdef LOCUS
    if (autolocus) {
        setlocus(1,1);
    }
#endif /* LOCUS */
    return(success = 1);
}


/* S E T L I N -- parse name of and then open communication device. */
/*
  Call with:
    xx == XYLINE for a serial (tty) line, XYHOST for a network host,
    zz == 0 means if user doesn't give a device name, continue current
            active connection (if any);
    zz != 0 means if user doesn't give a device name, then close the
            current connection and restore the default communication device.
    fc == 0 to just make the connection, 1 to also CONNECT (e.g. "telnet").
*/
int
setlin(xx, zz, fc) 
    int xx, zz, fc; 
{
    extern char pwbuf[], * g_pswd;
    extern int pwflg, pwcrypt, g_pflg, g_pcpt, nolocal;
    int wait;
    /* int tn_wait_sv; */
    int mynet;
    int _local = -1;
    int c, i, haveswitch = 0;
    int haveuser = 0;
    int getval = 0;
    int wild = 0;                       /* Filespec has wildcards */
    int cx = 0;                         /* Connect after */
    int sx = 0;                         /* Become server after */
    int a_type = -1;                    /* Authentication type */
    int e_type = -1;                    /* Telnet /ENCRYPT type */
#ifdef CK_ENCRYPTION
    int encrypt = 0;                    /* Encrypted? */
#endif /* CK_ENCRYPTION */
    int shr = 0;                        /* Share serial device */
    int confirmed = 0;                  /* Command has been entered */
    struct FDB sw, tx, nx;
#ifdef OS2
    struct FDB fl;
#endif /* OS2 */

    char * ss;
#ifdef TCPSOCKET
    int rawflg = 0;
#endif /* TCPSOCKET */

    char srvbuf[SRVBUFSIZ+1];

#ifdef OS2
#ifdef NT
    int xxtapi = 0;
#else
    int xxslip = 0, xxppp = 0;
#endif /* NT */
#endif /* OS2 */

    int dossh = 0;

    debug(F101,"setlin fc","",fc);
    debug(F101,"setlin zz","",zz);
    debug(F101,"setlin xx","",xx);

#ifdef SSHCMD
    if (xx == XXSSH) {                  /* SSH becomes PTY SSH ... */
        dossh = 1;
        xx = XYHOST;
    } else if (!ckstrcmp("ssh ",line,4,0)) { /* 2010/03/01 */
        dossh = 1;
        xx = XYHOST;
    }
    debug(F101,"setlin dossh","",dossh);
#endif /* SSHCMD */

#ifdef TNCODE
    /* tn_wait_sv = tn_wait_flg; */
    wait = tn_wait_flg;
#else
    /* tn_wait_sv = 0; */
    wait = 0;
#endif /* TNCODE */

    mynet = nettype;

    if (nolocal) {
        makestr(&slmsg,"Making connections is disabled");
        printf("?Sorry, making connections is disabled\n");
        return(-9);
    }
    if (netsave > -1)
      nettype = netsave;

    if (fc != 0 || zz == 0)             /* Preset /CONNECT switch */
      cx = 1;

    debug(F101,"setlin cx","",cx);

    *srvbuf = NUL;

    line[0] = NUL;
    s = line;

#ifdef NETCONN
#ifdef CK_SECURITY
    if (tmpstring)
        makestr(&tmpstring,NULL);
#endif /* CK_SECURITY */
    if (tmpusrid)
        makestr(&tmpusrid,NULL);
#endif /* NETCONN */

    autoflow = 1;                       /* Enable automatic flow setting */

    if (xx == XYHOST) {                 /* SET HOST <hostname> */
#ifndef NETCONN
        makestr(&slmsg,"Network connections not supported");
        printf("?%s\n",slmsg);
        return(-9);
#else /* NETCONN */
#ifndef NOPUSH
        if ((mynet == NET_CMD || mynet == NET_PTY || dossh) && nopush) {
            makestr(&slmsg,"Access to external commands is disabled");
            printf("?Sorry, access to external commands is disabled\n");
            return(-9);
        }
#endif /* NOPUSH */

#ifdef SSHCMD
        if (dossh) {                    /* SSH connection via pty */
            int k;
	    extern int ttyfd;		/* 2010/03/01 */
            k = ckstrncpy(line, sshcmd ? sshcmd : defsshcmd, LINBUFSIZ);
            debug(F111,"setlin sshcmd 1",line,k);
            if ((x = cmtxt("Optional switches and hostname","",&s,xxstring))<0)
              return(x);

            /* 2010-03-30 */
	    if (!*s && ttyfd < 0 && !ckstrcmp("ssh ",ttname,4,0)) { 
		x = ckstrncpy(line,ttname,LINBUFSIZ);
	    } else {
		if (!*s) {
		    printf("?SSH to where?\n");
		    return(-9);
		}
		if (k < LINBUFSIZ) {
		    line[k++] = SP;
		    line[k] = NUL;
		    debug(F111,"setlin sshcmd 2",line,k);
		} if (k < LINBUFSIZ) {
		    ckstrncpy(&line[k],s,LINBUFSIZ-k);
		    debug(F111,"setlin sshcmd 3",line,k);
		} else {
		    printf("?Too long\n");
		    return(-9);
		}
	    }
	    x = cx_net( NET_PTY,                /* network type */
                        0,                      /* protocol (not used) */
                        line,                   /* host */
                        NULL,                   /* service (not used) */
                        NULL,                   /* username (not used) */
                        NULL,                   /* password (not used) */
                        NULL,                   /* command (not used) */
                        -1,-1,-1,               /* params 1-3 (not used) */
                        1,                      /* connect immediately */
                        sx,                     /* server? */
                        zz,                     /* close current? */
                        0);                     /* not gui */
	    debug(F111,"setlin cx_net",line,x);
	    return(x);
        }
#endif /* SSHCMD */

/*
  Here we parse optional switches and then the hostname or whatever,
  which depends on the network type.  The tricky part is, the network type
  can be set by a switch.
*/
#ifndef NOSPL
        makestr(&g_pswd,pwbuf);         /* Save global pwbuf */
        g_pflg = pwflg;                 /* and flag */
        g_pcpt = pwcrypt;
#endif /* NOSPL */

        confirmed = 0;
        haveswitch = 0;
#ifdef NETFILE
        if (mynet != NET_FILE) {
#endif /* NETFILE */
            ss = (mynet == NET_CMD || mynet == NET_PTY) ?
              "Command, or switch" :
                (mynet == NET_TCPA || mynet == NET_TCPB
                  || mynet == NET_SSH) ?
                  "Hostname, ip-address, or switch" :
                    "Host or switch";
            if (fc) {
                if (mynet == NET_TCPB &&
                    (ttnproto == NP_TELNET || ttnproto == NP_KERMIT)) {
                    if (nshteltab) {
                        haveswitch++;
                        cmfdbi(&sw,_CMKEY,ss,"","",nshteltab,4,xxstring,
                             shteltab,&nx);
                    }
                }
#ifdef RLOGCODE
                else if (mynet == NET_TCPB && ttnproto == NP_RLOGIN) {
                    if (nshrlgtab) {
                        haveswitch++;
                        cmfdbi(&sw,_CMKEY,ss,"","",nshrlgtab,4,xxstring,
                               shrlgtab,&nx);
                    }
                }
#endif /* RLOGCODE */
            } else {
                haveswitch++;
                cmfdbi(&sw,_CMKEY,ss,"","",nshtab,4,xxstring,shtab,&nx);
            }
#ifdef NETFILE
        }
#endif /* NETFILE */
        if (mynet == NET_TCPB || mynet == NET_SLAT ||
	    mynet == NET_SSH  || mynet == NET_DEC) {
            cmfdbi(&nx,_CMFLD,"Host","","",0,0,xxstring,NULL,NULL);
#ifdef NETFILE
        } else if (mynet == NET_FILE) {
            cmfdbi(&nx,_CMIFI,"Filename","","",0,0,xxstring,NULL,NULL);
#endif /* NETFILE */
#ifdef PTYORPIPE
        } else if (mynet == NET_CMD || mynet == NET_PTY) {
            cmfdbi(&nx,_CMTXT,"Command","","",0,0,xxstring,NULL,NULL);
#endif /* PTYORPIPE */
        } else {
            cmfdbi(&nx,_CMTXT,"Host","","",0,0,xxstring,NULL,NULL);
        }
        while (1) {
            x = cmfdb(haveswitch ? &sw : &nx);
            debug(F101,"setlin cmfdb","",x);
            if (x < 0)
              if (x != -3)
                return(x);
            if (x == -3) {
                if ((x = cmcfm()) < 0) {
                    return(x);
                } else {
                    confirmed = 1;
                    break;
                }
            }
            if (cmresult.fcode != _CMKEY) {    /* Not a switch */
                ckstrncpy(line,cmresult.sresult,LINBUFSIZ); /* Save the data */
                s = line;                      /* that was parsed... */
                if (cmresult.fcode == _CMIFI) {
                    wild = cmresult.nresult;
                } else if (cmresult.fcode == _CMTXT) {
                    confirmed = 1;
                }
                break;                  /* and break out of this loop */
            }
            c = cmgbrk();               /* Have switch - get break character */
            getval = (c == ':' || c == '='); /* Must parse an agument? */
            if (getval && !(cmresult.kflags & CM_ARG)) {
                printf("?This switch does not take arguments\n");
                return(-9);
            }
            if (!getval && (cmgkwflgs() & CM_ARG)) {
                printf("?This switch requires an argument\n");
                return(-9);
            }
            switch (cmresult.nresult) { /* It's a switch.. */
              case SL_CNX:              /* /CONNECT */
                cx = 1;
                sx = 0;
                break;
              case SL_SRV:              /* /SERVER */
                cx = 0;
                sx = 1;
                break;
#ifdef NETCMD
              case SL_CMD:              /* /COMMAND */
                netsave = mynet;
                mynet = NET_CMD;
                break;
#endif /* NETCMD */
#ifdef NETPTY
              case SL_PTY:              /* /PTY */
                netsave = mynet;
                mynet = NET_PTY;
                break;
#endif /* NETPTY */
              case SL_NET:              /* /NETWORK-TYPE */
                if ((x = cmkey(netcmd,nnets,"","",xxstring)) < 0)
                  return(x);
                mynet = x;
                break;

#ifdef CK_SECURITY
              case SL_PSW:              /* /PASSWORD: */
                if (!getval)
                  break;
                debok = 0;
                if ((x = cmfld("Password","",&s,xxstring)) < 0) {
                    if (x == -3) {
                        makestr(&tmpstring,"");
                    } else {
                        return(x);
                    }
                } else {
                    s = brstrip(s);
                    if ((x = (int)strlen(s)) > PWBUFL) {
                        makestr(&slmsg,"Internal error");
                        printf("?Sorry, too long - max = %d\n",PWBUFL);
                        return(-9);
                    }
                    makestr(&tmpstring,s);
                }
                break;
#endif /* CK_SECURITY */

              case SL_UID:              /* /USERID: */
                if (!getval)
                  break;
                if ((x = cmfld("Userid","",&s,xxstring)) < 0) {
                    if (x == -3) {
                        makestr(&tmpusrid,"");
                    } else {
                        return(x);
                    }
                } else {
                    s = brstrip(s);
                    if ((x = (int)strlen(s)) > 63) {
                        makestr(&slmsg,"Internal error");
                        printf("?Sorry, too long - max = %d\n",63);
                        return(-9);
                    }
                    makestr(&tmpusrid,s);
                    haveuser = 1;
                }
                break;

#ifdef CK_AUTHENTICATION
#ifdef CK_SRP
              case SL_SRP:
                a_type = AUTHTYPE_SRP;
                break;
#endif /* CK_SRP */
#ifdef CK_SSL
              case SL_SSL:
                a_type = AUTHTYPE_SSL;
                break;
#endif /* CK_SSL */
#ifdef NT
              case SL_NTLM:
                a_type = AUTHTYPE_NTLM;
                break;
#endif /* NT */
#ifdef CK_KERBEROS
              case SL_KRB4:
                a_type = AUTHTYPE_KERBEROS_V4;
                if (ttnproto == NP_RLOGIN)
                  ttnproto =
#ifdef CK_ENCRYPTION
                    encrypt ? NP_EK4LOGIN :
#endif /* CK_ENCRYPTION */
                      NP_K4LOGIN;
                else if (ttnproto == NP_K5LOGIN)
                  ttnproto = NP_K4LOGIN;
#ifdef CK_ENCRYPTION
                else if (ttnproto == NP_EK5LOGIN)
                  ttnproto = NP_EK4LOGIN;
#endif /* CK_ENCRYPTION */
                break;
              case SL_KRB5:
                a_type = AUTHTYPE_KERBEROS_V5;
                if (ttnproto == NP_RLOGIN)
                  ttnproto =
#ifdef CK_ENCRYPTION
                    encrypt ? NP_EK5LOGIN :
#endif /* CK_ENCRYPTION */
                      NP_K5LOGIN;
                else if (ttnproto == NP_K4LOGIN)
                  ttnproto = NP_K5LOGIN;
#ifdef CK_ENCRYPTION
                else if (ttnproto == NP_EK4LOGIN)
                  ttnproto = NP_EK5LOGIN;
#endif /* CK_ENCRYPTION */
                break;
#endif /* CK_KERBEROS */
              case SL_AUTH: {
                  extern struct keytab autyptab[];
                  extern int nautyp;
                  if ((x = cmkey(autyptab,nautyp,"type of authentication",
                                 "automatic",xxstring)) < 0)
                    return(x);
                  a_type = x;
                  break;
              }
#endif /* CK_AUTHENTICATION */
#ifdef CK_ENCRYPTION
              case SL_ENC:
                switch (ttnproto) {
                  case NP_K4LOGIN:
                    ttnproto = NP_EK4LOGIN;
                    encrypt = 1;
                    break;
                  case NP_K5LOGIN:
                    ttnproto = NP_EK5LOGIN;
                    encrypt = 1;
                    break;
                  case NP_KERMIT:
                  case NP_TELNET: {
                      static struct keytab * tnetbl = NULL;
                      static int ntnetbl = 0;
                      x = ck_get_crypt_table(&tnetbl,&ntnetbl);
                      debug(F101,"ck_get_crypt_table x","",x);
                      debug(F101,"ck_get_crypt_table n","",ntnetbl);
                      if (x < 1 || !tnetbl || ntnetbl < 1) /* Didn't get it */
                        x = 0;
                      if (!x) {
                          makestr(&slmsg,"Internal error");
                          printf("?Oops, types not loaded\n");
                          return(-9);
                      }
                      if ((x = cmkey(tnetbl,ntnetbl,"type of encryption",
                                     "automatic",xxstring)) < 0)
                        return(x);
                      e_type = x;
                      break;
                  }
                }
                break;
#endif /* CK_ENCRYPTION */
              case SL_WAIT:
                wait = 1;
                break;
              case SL_NOWAIT:
                wait = 0;
                break;
            }
        }

#ifdef NETFILE
        if (mynet == NET_FILE) {        /* Parsed by cmifi() */
	    if ((x = cmcfm()) < 0)	/* Needs confirmation */
	      return(x);
	    x = cx_net(mynet,		/* nettype */
		       0,		/* protocol (not used) */
		       line,		/* host */
		       "",		/* port */
		       NULL,		/* alternate username */
		       NULL,		/* password */
		       NULL,		/* command to execute */
		       0,		/* param1 */
		       0,		/* param2 */
		       0,		/* param3 */
		       cx,		/* enter CONNECT mode */
		       sx,		/* enter SERVER mode */
		       zz,		/* close connection if open */
		       0		/* gui */
		       );
        }
#endif /* NETFILE */

#ifdef NETCMD
        if (mynet == NET_CMD || mynet == NET_PTY) {
            char *p = NULL;
            if (!confirmed) {
                if ((x = cmtxt("Rest of command","",&s,xxstring)) < 0)
                  return(x);
                if (*s) {
                    ckstrncat(line," ",LINBUFSIZ);
                    ckstrncat(line,s,LINBUFSIZ);
                }
                s = line;
            }
            /* s == line - so we must protect the line buffer */
            s = brstrip(s);
            makestr(&p,s);
            ckstrncpy(line,p,LINBUFSIZ);
            makestr(&p,NULL);

            x = cx_net( mynet,                  /* nettype */
                        0,                      /* protocol (not used) */
                        line,                   /* host */
                        "",                     /* port */
                        NULL,                   /* alternate username */
                        NULL,                   /* password */
                        NULL,                   /* command to execute */
                        0,                      /* param1 */
                        0,                      /* param2 */
                        0,                      /* param3 */
                        cx,                     /* enter CONNECT mode */
                        sx,                     /* enter SERVER mode */
                        zz,                     /* close connection if open */
                        0                       /* gui */
                        );
        }
#endif /* NETCMD */

#ifdef NPIPE                            /* Named pipe */
        if (mynet == NET_PIPE) {        /* Needs backslash twiddling */
            if (line[0]) {
                if (strcmp(line,"*")) {    /* If remote, begin with */
                    char * p = NULL;
                    makestr(&p,line);      
                    ckstrncpy(line,"\\\\",LINBUFSIZ); /* server name */
                    ckstrncat(line,p,LINBUFSIZ);
                    makestr(&p,NULL);      
                } else {
                    line[0]='\0';
                }
                ckstrncat(line,"\\pipe\\", LINBUFSIZ); /* Make pipe name */
                ckstrncat(line,pipename, LINBUFSIZ); /* Add name of pipe */

                x = cx_net(mynet,	/* nettype */
			   0,		/* protocol (not used) */
			   line,	/* host */
			   "",		/* port */
			   NULL,	/* alternate username */
			   NULL,	/* password */
			   NULL,	/* command to execute */
			   0,		/* param1 */
			   0,		/* param2 */
			   0,		/* param3 */
			   cx,		/* enter CONNECT mode */
			   sx,		/* enter SERVER mode */
			   zz,		/* close connection if open */
			   0		/* gui */
			   );
            }
        }
#endif /* NPIPE */

#ifdef SUPERLAT
        if (mynet == NET_SLAT) {        /* Needs password, etc. */
            slat_pwd[0] = NUL;          /* Erase any previous password */
            debok = 0;
            if (*line) {		/* If they gave a host name... */
                if ((x = cmfld(
                     "password,\n or carriage return if no password required",
                               "",
                               &s,
                               xxstring
                               )) < 0 && x != -3)
                  return(x);
                ckstrncpy(slat_pwd,s,18); /* Set the password, if any */
            }
            if ((x = cmcfm()) < 0) return(x); /* Confirm the command */

            x = cx_net(mynet,		/* nettype */
                       0,		/* protocol (not used) */
                       line,		/* host */
                       "",		/* port */
                       NULL,		/* alternate username */
                       NULL,		/* password */
                       NULL,		/* command to execute */
                       0,		/* param1 */
                       0,		/* param2 */
                       0,		/* param3 */
                       cx,		/* enter CONNECT mode */
                       sx,		/* enter SERVER mode */
                       zz,		/* close connection if open */
                       0		/* gui */
                       );
        }
#endif /* SUPERLAT */

#ifdef DECNET
        if (mynet == NET_DEC) {  
            if (!line[0]) {                   /* If they gave a host name... */
                printf("?hostname required\n");
                return(-3);
            }
            if ((x = cmcfm()) < 0) return(x); /* Confirm the command */

            x = cx_net(mynet,		/* nettype */
                       0,		/* protocol (not used) */
                       line,		/* host */
                       "",		/* port */
                       NULL,		/* alternate username */
                       NULL,		/* password */
                       NULL,		/* command to execute */
                       0,		/* param1 */
                       0,		/* param2 */
                       0,		/* param3 */
                       cx,		/* enter CONNECT mode */
                       sx,		/* enter SERVER mode */
                       zz,		/* close connection if open */
                       0		/* gui */
                       );
        }
#endif /* DECNET */

#ifdef SSHBUILTIN
        if (mynet == NET_SSH) {		/* SSH connection */
            int k, havehost = 0, trips = 0;
            int    tmpver = -1, tmpxfw = -1, tmpssh_cas;
#ifndef SSHTEST
            extern int sl_ssh_xfw, sl_ssh_xfw_saved;
            extern int sl_ssh_ver, sl_ssh_ver_saved;
#endif /* SSHTEST */
            extern struct keytab sshopnsw[];
            extern int nsshopnsw;
            extern char *ssh_tmpcmd, *ssh_tmpport;
            struct FDB sw, kw, fl;

            debug(F110,"setlin SSH service 0",srvbuf,0);
            debug(F110,"setlin SSH host s 2",s,0);
            if (*s) {           /* If they gave a host name... */
                debug(F110,"setlin SSH host s 1",s,0);
                if (*s == '*') {
                    makestr(&slmsg,"Incoming connections not supported");
                    printf(
     "?Sorry, incoming connections not supported for SSH.\n"
                           );
                    return(-9);
                }
                ckstrncpy(line,s,LINBUFSIZ);
            } else {
                printf("?hostname required\n");
                return(-3);
            }

            /* Parse [ port ] [ switches ] */
            cmfdbi(&kw,                 /* Switches */
                    _CMKEY,
                    "Port number or service name,\nor switch",
                    "",
                    "",
                    nsshopnsw,
                    4,
                    xxstring,
                    sshopnsw,
                    &fl
                    );
            cmfdbi(&fl,                 /* Port number or service name */
                    _CMFLD,
                    "",
                    "",
                    "",
                    0,
                    0,
                    xxstring,
                    NULL,
                    NULL
                    );
            trips = 0;                  /* Explained below */
            while (1) {                 /* Parse port and switches */
                y = cmfdb(&kw);         /* Get a field */
                if (y == -3)            /* User typed CR so quit from loop */
                    break;
                if (y < 0)              /* Other parse error, pass it back */
                    return(y);
                switch (cmresult.fcode) { /* Field or Keyword? */
                case _CMFLD:              /* Field */
                    ckstrncpy(srvbuf,cmresult.sresult,SRVBUFSIZ);
                    break;
                case _CMKEY:            /* Keyword */
                    switch (cmresult.nresult) { /* Which one? */
                    case SSHSW_PWD:
                        if (!cmgbrk()) {
                            printf("?This switch requires an argument\n");
                            return(-9);
                        }
                        debok = 0;
                        if ((y = cmfld("Password","",&s,xxstring)) < 0) {
                            if (y == -3) {
                                makestr(&tmpstring,"");
                            } else {
                                return(y);
                            }
                        } else {
                            s = brstrip(s);
                            if ((y = (int)strlen(s)) > PWBUFL) {
                                makestr(&slmsg,"Internal error");
                                printf("?Sorry, too long - max = %d\n",PWBUFL);
                                return(-9);
                            }
                            makestr(&tmpstring,s);
                        }
                        break;
                    case SSHSW_USR:             /* /USER: */
                        if (!cmgbrk()) {
                            printf("?This switch requires an argument\n");
                            return(-9);
                        }
                        if ((y = cmfld("Username","",&s,xxstring)) < 0)
                            return(y);
                        s = brstrip(s);
                        makestr(&tmpusrid,s);
                        break;
                    case SSHSW_VER:
                        if ((y = cmnum("Number","",10,&z,xxstring)) < 0)
                            return(y);
                        if (z < 1 || z > 2) {
                            printf("?Out of range: %d\n",z);
                            return(-9);
                        }
                        tmpver = z;
                        break;
                    case SSHSW_CMD:
                    case SSHSW_SUB:
                        if ((y = cmfld("Text","",&s,xxstring)) < 0)
                          return(y);
                        makestr(&ssh_tmpcmd,s);
                        tmpssh_cas = (cmresult.nresult == SSHSW_SUB);
                        break;
                    case SSHSW_X11:
                        if ((y = cmkey(onoff,2,"","on",xxstring)) < 0)
                            return(y);
                        tmpxfw = y;
                        break;
                    default:
                        return(-2);
                    }
                }
                if (trips++ == 0) {     /* After first time through */
                    cmfdbi(&kw,         /* only parse switches, not port. */
                            _CMKEY,
                            "Switch",
                            "",
                            "",
                            nsshopnsw,
                            4,
                            xxstring,
                            sshopnsw,
                            NULL
                            );
                }
            }
            if ((y = cmcfm()) < 0)      /* Get confirmation */
                return(y);

            debug(F110,"setlin pre-cx_net line",line,0);
            debug(F110,"setlin pre-cx_net srvbuf",srvbuf,0);
            x = cx_net( mynet,                  /* nettype */
                        0,                      /* protocol (not used) */
                        line,                   /* host */
                        srvbuf,                 /* port */
                        tmpusrid,               /* alternate username */
                        tmpstring,              /* password */
                        ssh_tmpcmd,             /* command to execute */
                        tmpver,                 /* param1 - ssh version */
                        tmpssh_cas,             /* param2 - ssh cas  */
                        tmpxfw,                 /* param3 - ssh x11fwd */
                        cx,                     /* enter CONNECT mode */
                        sx,                     /* enter SERVER mode */
                        zz,                     /* close connection if open */
                        0                       /* gui */
                        );
            if (tmpusrid)
                makestr(&tmpusrid,NULL);
            if (ssh_tmpcmd)
                makestr(&ssh_tmpcmd,NULL);
        }
#endif /* SSHBUILTIN */

#ifdef TCPSOCKET
        if (mynet == NET_TCPB) {        /* TCP/IP connection */
            debug(F110,"setlin service 0",srvbuf,0);
            debug(F110,"setlin host s 2",s,0);
            if (*s) {			/* If they gave a host name... */
                debug(F110,"setlin host s 1",s,0);
#ifdef NOLISTEN
                if (*s == '*') {
                    makestr(&slmsg,"Incoming connections not supported");
                    printf(
     "?Sorry, incoming connections not supported in this version of Kermit.\n"
                           );
                    return(-9);
                }
#endif /* NOLISTEN */
#ifdef RLOGCODE
                /* Allow a username if rlogin is requested */
                if (mynet == NET_TCPB &&
                    (ttnproto == NP_RLOGIN || ttnproto == NP_K5LOGIN ||
                     ttnproto == NP_EK5LOGIN || ttnproto == NP_K4LOGIN ||
                     ttnproto == NP_EK4LOGIN
                    )) {
                    int y;
                    uidflag = 0;
                    /* Check for "host:service" */
                    for ( ; (*s != '\0') && (*s != ':'); s++) ;
                    if (*s) {   /* Service, save it */
                        *s = NUL;
                        ckstrncpy(srvbuf,++s,SRVBUFSIZ);
                    } else {            /* No :service, then use default. */
#ifdef VMS
                        switch (ttnproto) {
                          case NP_RLOGIN:
                            ckstrncpy(srvbuf,"513",SRVBUFSIZ); /* "login" */
                            break;
                          case NP_K4LOGIN:
                          case NP_K5LOGIN:
                            ckstrncpy(srvbuf,"543",SRVBUFSIZ); /* "klogin" */
                            break;
                          case NP_EK4LOGIN:
                          case NP_EK5LOGIN:
                            ckstrncpy(srvbuf,"2105",SRVBUFSIZ); /* "eklogin" */
                            break;
                        }
#else /* VMS */
                        switch (ttnproto) {
                          case NP_RLOGIN:
                            ckstrncpy(srvbuf,"login",SRVBUFSIZ);
                            break;
                          case NP_K4LOGIN:
                          case NP_K5LOGIN:
                            ckstrncpy(srvbuf,"klogin",SRVBUFSIZ);
                            break;
                          case NP_EK4LOGIN:
                          case NP_EK5LOGIN:
                            ckstrncpy(srvbuf,"eklogin",SRVBUFSIZ);
                            break;
                        }
#endif /* VMS */
                    }
                    if (!confirmed) {
                        y = cmfld("Userid on remote system",
                                  uidbuf,&s,xxstring);
                        if (y < 0 && y != -3)
                          return(y);
                        if ((int)strlen(s) > 63) {
                            makestr(&slmsg,"Internal error");
                            printf("Sorry, too long\n");
                            return(-9);
                        }
                        makestr(&tmpusrid,s);
                    }
                } else {        /* TELNET or SET HOST */
#endif /* RLOGCODE */
                    /* Check for "host:service" */
                    for ( ; (*s != '\0') && (*s != ':'); s++) ;
                    if (*s) {   /* Service, save it */
                        *s = NUL;
                        ckstrncpy(srvbuf,++s,SRVBUFSIZ);
                    } else if (!confirmed) {
                        /* No :service, let them type one. */
                        if (*line != '*') { /* Not incoming */
                            if (mynet == NET_TCPB && ttnproto == NP_KERMIT) {
                                if ((x = cmfld(
                                               "TCP service name or number",
                                               "kermit",&s,xxstring)
                                     ) < 0 && x != -3)
                                  return(x);
#ifdef RLOGCODE
                            } else if (mynet == NET_TCPB &&
                                       ttnproto == NP_RLOGIN) {
                                if ((x = cmfld(
  "TCP service name or number,\n or carriage return for rlogin (513)",
                                               "login",&s,xxstring)
                                     ) < 0 && x != -3)
                                  return(x);
#ifdef CK_AUTHENTICATION
#ifdef CK_KERBEROS
                            } else if (mynet == NET_TCPB &&
                                       (ttnproto == NP_K4LOGIN ||
                                       ttnproto == NP_K5LOGIN)) {
                                if ((x = cmfld(
  "TCP service name or number,\n or carriage return for klogin (543)",
                                               "klogin",&s,xxstring)
                                     ) < 0 && x != -3)
                                  return(x);
                            } else if (mynet == NET_TCPB &&
                                       (ttnproto == NP_EK4LOGIN ||
                                        ttnproto == NP_EK5LOGIN)) {
                                if ((x = cmfld(
  "TCP service name or number,\n or carriage return for eklogin (2105)",
                                               "eklogin",&s,xxstring)
                                     ) < 0 && x != -3)
                                  return(x);
#endif /* CK_KERBEROS */
#endif /* CK_AUTHENTICATION */
#endif /* RLOGCODE */
                            } else {
                                /* Do not set a default value in this call */
                                /* If you do then it will prevent entries  */
                                /* in the network directory from accessing */
                                /* alternate ports.                        */

                                if ((x = cmfld(
                                               "TCP service name or number",
                                               "",&s,xxstring)
                                     ) < 0 && x != -3)
                                  return(x);
                            }
                        } else { /* Incoming connection */
                            if ((x = cmfld("TCP service name or number",
                                           "",&s,xxstring)
                                 ) < 0 && x != -3)
                              return(x);
                        }
                        if (*s)         /* If they gave a service, */
                          ckstrncpy(srvbuf,s,SRVBUFSIZ); /* copy it */
                        debug(F110,"setlin service 0.5",srvbuf,0);
                    }
#ifdef RLOGCODE
                }
#endif /* RLOGCODE */
                if (!confirmed) {
                    char * defproto;
                    switch (ttnproto) {
                      case NP_RLOGIN:
                        defproto = "/rlogin";
                        break;
                      case NP_K4LOGIN:
                        defproto = "/k4login";
                        break;
                      case NP_K5LOGIN:
                        defproto = "/k5login";
                        break;
                      case NP_EK4LOGIN:
                        defproto = "/ek4login";
                        break;
                      case NP_EK5LOGIN:
                        defproto = "/ek5login";
                        break;
                      case NP_KERMIT:
                      case NP_TELNET:
                        defproto = "/telnet";
                        break;
                      default:
                        defproto = "/default";
                    }
                    if ((x = cmkey(tcprawtab,ntcpraw,"Switch",defproto,
                                   xxstring)) < 0) {
                        if (x != -3)
                          return(x);
                        else if ((x = cmcfm()) < 0)
                          return(x);
                    } else {
                        rawflg = x;
                        if ((x = cmcfm()) < 0)
                          return(x);
                    }
                }
            }
            debug(F110,"setlin pre-cx_net line",line,0);
            debug(F110,"setlin pre-cx_net srvbuf",srvbuf,0);
            x = cx_net( mynet,                  /* nettype */
                        rawflg                  /* protocol */,
                        line,                   /* host */
                        srvbuf,                 /* port */
                        tmpusrid,               /* alternate username */
                        tmpstring,              /* password */
                        NULL,                   /* command to execute */
                        a_type,                 /* param1 - telnet authtype */
                        e_type,                 /* param2 - telnet enctype  */
                        wait,                   /* param3 - telnet wait */
                        cx,                     /* enter CONNECT mode */
                        sx,                     /* enter SERVER mode */
                        zz,                     /* close connection if open */
                        0                       /* gui */
                        );
        }
#endif /* TCPSOCKET */

#ifdef CK_SECURITY
        if (tmpstring)
            makestr(&tmpstring,NULL);
#endif /* CK_SECURITY */
        if (tmpusrid)
            makestr(&tmpusrid,NULL);
	debug(F111,"setlin cx_net",line,x);
	return(x);
#endif /* NETCONN */
    }

/* Serial tty device, possibly modem, connection... */

#ifdef OS2
/*
  User can type:
    COM1..COM8 = Regular COM port
    1..8       = Synonym for COM1..COM8, is translated to COM1..COM8
    _n         = (n is a number) = open file handle
    string     = any text string = name of some other kind of device,
                 taken literally, as given.
*/
    s = "Communication device name";

#ifdef CK_TAPI
    if (TAPIAvail)
      cktapiBuildLineTable(&tapilinetab, &_tapilinetab, &ntapiline);
    if (!(tapilinetab && _tapilinetab && ntapiline > 0) &&
	xx == XYTAPI_LIN ) {
	makestr(&slmsg,"TAPI device not configured");
	printf("\nNo TAPI Line Devices are configured for this system\n");
	return(-9);
    }
    if (xx == XYTAPI_LIN) {		/* Default (first) TAPI line */
	s = "tapi";			/* (whatever it is) */
    } else {				/* Query the user */
#endif /* CK_TAPI */

/* Now parse optional switches and then device name */

	confirmed = 0;
	cmfdbi(&sw,_CMKEY,"Device name, or switch",
	       "","",npsltab,4,xxstring,psltab,&fl);
	cmfdbi(&fl,_CMFLD,"",dftty,"",0,0,xxstring,NULL,NULL);
	while (1) {
	    x = cmfdb(&sw);
	    debug(F101,"setlin cmfdb","",x);
	    if (x < 0)
	      if (x != -3)
		return(x);
	    if (x == -3) {
		if ((x = cmcfm()) < 0) {
		    return(x);
		} else {
		    confirmed = 1;
		    break;
		}
	    }
	    if (cmresult.fcode == _CMFLD) {
		s = cmresult.sresult;
		break;
	    } else if (cmresult.fcode == _CMKEY) {
		switch (cmresult.nresult) {
		  case SL_CNX:		/* /CONNECT */
		    cx = 1;
		    sx = 0;
		    break;
		  case SL_SRV:		/* /SERVER */
		    cx = 0;
		    sx = 1;
		    break;
		  case SL_SHR:		/* /SHARE */
		    shr = 1;
		    break;
		  case SL_NSH:		/* /NOSHARE */
		    shr = 0;
		    break;
		}
	    }
	}
#ifdef CK_TAPI
    }
#endif /* CK_TAPI */

    debug(F110,"OS2 SET PORT s",s,0);
    y = lookup(os2devtab,s,nos2dev,&x); /* Look up in keyword table */
    debug(F101,"OS2 SET PORT x","",x);
    debug(F101,"OS2 SET PORT y","",y);
    if ((y > -1) && (x >= 0 && x < 8)) { /* User typed a digit 1..8 */
	s = os2devtab[x+8].kwd;		/* Substitite its real name */
#ifdef NT
	xxtapi = 0;
#else /* NT */
	xxslip = xxppp = 0;
#endif /* NT */
	debug(F110,"OS2 SET PORT subst s",s,"");
#ifndef NT
    } else if ((y >-1) && (x >= 16 && x < 24)) { /* SLIP access */
	s = os2devtab[x-8].kwd;		/* Substitite its real name */
	debug(F110,"OS2 SET PORT SLIP subst s",s,"");
	xxslip = 1;
	xxppp  = 0;
    } else if ((y >-1) && (x >= 24 && x < 32)) { /* PPP access */
	s = os2devtab[x-16].kwd;	/* Substitite its real name */
	debug(F110,"OS2 SET PORT PPP subst s",s,"");
	xxppp = 1;
	xxslip = 0;
	if ((y = cmkey(os2ppptab,
		       nos2ppp,
		       "PPP driver interface",
		       "ppp0",
		       xxstring)
	     ) < 0)
	  return(y);
	debug(F101,"OS2 SET PORT PPP INTERFACE y","",y);
	xxppp = (y % 10) + 1;
#endif /* NT */
    } else if (*s == '_') {		/* User used "_" prefix */
	s++;				/* Remove it */
	/* Rest must be numeric */
	debug(F110,"OS2 SET PORT HANDLE _subst s",s,0);
	if (!rdigits(s)) {
	    makestr(&slmsg,"Invalid file handle");
	    printf("?Invalid format for file handle\n");
	    return(-9);
	}
#ifdef NT
	xxtapi = 0;
#else /* NT */
	xxslip = xxppp = 0;
#endif /* NT */
    } else {				/* A normal COMx port or a string */
	s = brstrip(s);			/* Strip braces if any */
#ifdef NT
#ifdef CK_TAPI
	/* Windows TAPI support - Look up in keyword table */
	if (tapilinetab && _tapilinetab && ntapiline > 0) {
	    if (!ckstrcmp(s,"tapi",4,0)) {

		/* Find out what the lowest numbered TAPI device is */
		/* and use it as the default.                       */
		int j = 9999, k = -1;
		for (i = 0; i < ntapiline; i++) {
		    if (tapilinetab[i].kwval < j) {
			j = tapilinetab[i].kwval;
			k = i;
		    }
		}
		if (k >= 0)
		  s = _tapilinetab[k].kwd;
		else
		  s = "";

		if ((y = cmkey(_tapilinetab,ntapiline,
			       "TAPI device name",s,xxstring)) < 0)
		  return(y);

		xxtapi = 1;

		/* Get the non Underscored string */
		for (i = 0; i < ntapiline; i++ ) {
		    if (tapilinetab[i].kwval == y) {
			s = tapilinetab[i].kwd;
			break;
		    }
		}
	    } else
	      xxtapi = 0;
	}
#endif /* CK_TAPI */
#else /* NT */
	/* not OS/2 SLIP or PPP */
	xxslip = xxppp = 0;
#endif /* NT */
    }
    ckstrncpy(tmpbuf,s,TMPBUFSIZ);	/* Copy to a safe place */
    s = tmpbuf;
    if ((x = cmcfm()) < 0)
      return(x);

#else /* !OS2 */

    cmfdbi(&sw,_CMKEY,"Device name, or switch",
	   "","",npsltab,4,xxstring,psltab,&tx);
    cmfdbi(&tx,_CMTXT,"",dftty,"",0,0,xxstring,NULL,NULL);
    while (!confirmed) {
	x = cmfdb(&sw);
	debug(F101,"setlin cmfdb","",x);
	if (x < 0)
	  if (x != -3)
	    return(x);
	if (x == -3) {
	    if ((x = cmcfm()) < 0) {
		return(x);
	    } else {
		confirmed = 1;
		break;
	    }
	}
	switch (cmresult.fcode) {
	  case _CMTXT:
	    ckstrncpy(tmpbuf,cmresult.sresult,TMPBUFSIZ);
	    s = tmpbuf;
	    debug(F110,"setlin CMTXT",tmpbuf,0);
	    confirmed = 1;
	    break;
	  case _CMKEY:			/* Switch */
	    debug(F101,"setlin CMKEY",tmpbuf,cmresult.nresult);
	    switch (cmresult.nresult) {
	      case SL_CNX:		/* /CONNECT */
		cx = 1;
		sx = 0;
		break;
	      case SL_SRV:		/* /SERVER */
		cx = 0;
		sx = 1;
		break;
#ifdef VMS
	      case SL_SHR:		/* /SHARE */
		shr = 1;
		break;
	      case SL_NSH:		/* /NOSHARE */
		shr = 0;
		break;
#endif /* VMS */
	    }
	    continue;
	  default:
	    debug(F101,"setlin bad cmfdb result","",cmresult.fcode);
	    makestr(&slmsg,"Internal error");
	    printf("?Internal parsing error\n");
	    return(-9);
	}
    }
#endif /* OS2 */
    if (!confirmed)
      if ((x = cmcfm()) < 0)
	return(x);

    debug(F110,"setlin pre-cx_serial s",s,0);
    debug(F110,"setlin pre-cx_serial line",line,0);
    x = cx_serial(s,cx,sx,shr,zz,0,
#ifdef OS2
#ifdef NT
                   (xxtapi ? CX_TAPI : 0)
#else
                   (xxslip ? CX_SLIP : 0) | (xxppp ? CX_PPP : 0)
#endif /* NT */
#else /* OS2 */
                   0
#endif /* OS2 */
                   );
    debug(F111,"setlin cx_serial",line,x);
    return(x);
}
#endif /* NOLOCAL */

#ifdef CKCHANNELIO
/*
  C-Library based file-i/o package for scripts.  This should be portable to
  all C-Kermit versions since it uses the same APIs we have always used for
  processing command files.  The entire channel i/o package is contained
  herein, apart from some keyword table entries in the main keyword table
  and the help text in the HELP command module.

  On platforms like VMS and VOS, this package handles only UNIX-style
  stream files.  If desired, it can be replaced for those platforms by
  <#>ifdef'ing out this code and adding the equivalent replacement routines
  to the ck?fio.c module, e.g. for RMS-based file i/o in ckvfio.c.
*/
#ifndef NOSTAT
#ifdef VMS
/* 2010-03-09 SMS.  VAX C needs help to find "sys".  It's easier not to try. */
#include <stat.h>
#else /* def VMS */
#include <sys/stat.h>
#endif /* def VMS [else] */
#endif /* NOSTAT */

#ifdef NLCHAR
static int z_lt = 1;                    /* Length of line terminator */
#else
static int z_lt = 2;
#endif /* NLCHAR */

struct ckz_file {                       /* C-Kermit file struct */
    FILE * z_fp;                        /* Includes the C-Lib file struct */
    unsigned int z_flags;               /* Plus C-Kermit mode flags, */
    CK_OFF_T z_nline;			/* current line number if known, */
    char z_name[CKMAXPATH+2];           /* and the file's name. */
};
static struct ckz_file ** z_file = NULL; /* Array of C-Kermit file structs */
static int z_inited = 0;                /* Flag for array initialized */
int z_maxchan = Z_MAXCHAN;              /* Max number of C-Kermit channels */
int z_openmax = CKMAXOPEN;              /* Max number of open files overall */
int z_nopen = 0;                        /* How many channels presently open */
int z_error = 0;                        /* Most recent error */
int z_filcount = -1;                    /* Most recent FILE COUNT result */

#define RD_LINE 0                       /* FILE READ options */
#define RD_CHAR 1
#define RD_SIZE 2
#define RD_TRIM 8			/* Like Snobol &TRIM = 1 */
#define RD_UNTA 9			/* Untabify */

#define WR_LINE RD_LINE                 /* FILE WRITE options */
#define WR_CHAR RD_CHAR
#define WR_SIZE RD_SIZE
#define WR_STRI 3
#define WR_LPAD 4
#define WR_RPAD 5

#ifdef UNIX
extern int ckmaxfiles;                  /* Filled in by sysinit(). */
#endif /* UNIX */

/* See ckcker.h for error numbers */
/* See ckcdeb.h for Z_MAXCHAN and CKMAXOPEN definitions */
/* NOTE: For VMS we might be able to fill in ckmaxfiles */
/* from FILLM and CHANNELCNT -- find out about these... */

static char * fopnargs[] = {            /* Mode combinations for fopen() */
#ifdef COMMENT
    /* All combinations of rwa */
    "",  "r",  "w",  "rw",  "a",  "ra",  "wa",  "rwa", /* Text mode */
    "b", "rb", "wb", "rwb", "ab", "rab", "wab", "rwab" /* Binary mode */
#else
    /* Combinations and syntax permitted by C libraries... */
    "",  "r",  "w",  "r+",  "a",  "",   "a",  "", /* Text mode */
#ifdef OS2
    "",  "rb", "wb", "r+b", "ab", "",   "ab", "" /* Binary modes for K95 */
#else
#ifdef VMS
    "",  "rb", "wb", "r+b", "ab", "",   "ab", "" /* Binary modes for VMS */
#else
    "",  "r",   "w", "r+",  "a",  "",   "a",  "" /* Binary modes for UNIX */
#endif /* VMS */
#endif /* OS2 */
#endif /* COMMENT */
};
static int nfopnargs = sizeof(fopnargs) / sizeof(char *);

char *                                  /* Error messages */
ckferror(n) int n; {
    switch (n) {
      case FX_NER: return("No error");
      case FX_SYS: return(ck_errstr());
      case FX_EOF: return("End of file");
      case FX_NOP: return("File not open");
      case FX_CHN: return("Channel out of range");
      case FX_RNG: return("Parameter out of range");
      case FX_NMF: return("Too many files open");
      case FX_FOP: return("Operation conflicts with OPEN mode");
      case FX_NYI: return("OPEN mode not supported");
      case FX_BOM: return("Illegal combination of OPEN modes");
      case FX_ACC: return("Access denied");
      case FX_FNF: return("File not found");
      case FX_OFL: return("Buffer overflow");
      case FX_LNU: return("Current line number unknown");
      case FX_ROO: return("Off limits");
      case FX_UNK: return("Operation fails - reason unknown");
      default: return("Error number out of range");
    }
}

/*
  Z _ O P E N --  Open a file for the requested type of access.

  Call with:
    name:  Name of file to be opened.
    flags: Any combination of FM_xxx values except FM_EOF (ckcker.h).
  Returns:
    >= 0 on success: The assigned channel number
    <  0 on failure: A negative FX_xxx error code (ckcker.h).
*/
int
z_open(name, flags) char * name; int flags; {
    int i, n;
    FILE * t;
    char * mode;
    debug(F111,"z_open",name,flags);
    if (!name) name = "";               /* Check name argument */
    if (!name[0])
      return(z_error = FX_BFN);
    if (flags & FM_CMD)                 /* Opening pipes not implemented yet */
      return(z_error = FX_NYI);         /* (and not portable either) */
    debug(F101,"z_open nfopnargs","",nfopnargs);
    if (flags < 0 || flags >= nfopnargs) /* Range check flags */
      return(z_error = FX_RNG);
    mode = fopnargs[flags];             /* Get fopen() arg */
    debug(F111,"z_open fopen args",mode,flags);
    if (!mode[0])                       /* Check for illegal combinations */
      return(z_error = FX_BOM);
    if (!z_inited) {                    /* If file structs not inited */
        debug(F101,"z_open z_maxchan 1","",z_maxchan);
#ifdef UNIX
        debug(F101,"z_open ckmaxfiles","",ckmaxfiles);
        if (ckmaxfiles > 0) {           /* Set in ck?tio.c: sysinit() */
            int x;
            x = ckmaxfiles - ZNFILS - 5;
            if (x > z_maxchan)          /* sysconf() value greater than */
              z_maxchan = x;            /* value from header files. */
            debug(F101,"z_open z_maxchan 2","",z_maxchan);
        }
#endif /* UNIX */
        if (z_maxchan < Z_MINCHAN)      /* Allocate at least this many. */
          z_maxchan = Z_MINCHAN;
        debug(F101,"z_open z_maxchan 3","",z_maxchan);
        /* Note: This could be a pretty big chunk of memory */
        /* if z_maxchan is a big number.  If this becomes a problem */
        /* we'll need to malloc and free each element at open/close time */
#ifdef COMMENT
	/* May 2006 - it's time - in current Linux this about 3MB */
        if (!(z_file = (struct ckz_file *)
              malloc(sizeof(struct ckz_file) * (z_maxchan + 1))))
          return(z_error = FX_NMF);
        for (i = 0; i < z_maxchan; i++) {
            z_file[i].z_fp = NULL;
            z_file[i].z_flags = 0;
            z_file[i].z_nline = 0;
            *(z_file[i].z_name) = '\0';
        }
#else
	/* New economical way, allocate storage for each channel as needed */
	if (!z_file) {
	    z_file = (struct ckz_file **)malloc((z_maxchan + 1) *
						sizeof(struct ckz_file *));
	    if (!z_file)
	      return(z_error = FX_NMF);
	    for (i = 0; i < z_maxchan; i++)
	      z_file[i] = NULL;
	}
#endif	/* COMMENT */
        z_inited = 1;                   /* Remember we initialized */
    }
    for (n = -1, i = 0; i < z_maxchan; i++) { /* Find a free channel */
#ifdef COMMENT
        if (!z_file[i].z_fp) {
            n = i;
            break;
        }
#else
        if (!z_file[i]) {
	    z_file[i] = (struct ckz_file *) malloc(sizeof(struct ckz_file));
	    if (!z_file[i])
	      return(z_error = FX_NMF);
            n = i;
            break;
        }
#endif	/* COMMENT */

    }
    if (n < 0 || n >= z_maxchan)        /* Any free channels? */
      return(z_error = FX_NMF);         /* No, fail. */
    errno = 0;

    z_file[n]->z_flags = 0;		/* In case of failure... */
    z_file[n]->z_fp = NULL;		/* Set file pointer to NULL */

    t = fopen(name, mode);              /* Try to open the file. */
    if (!t) {                           /* Failed... */
        debug(F111,"z_open error",name,errno);
#ifdef EMFILE
        if (errno == EMFILE)
          return(z_error = FX_NMF);
#endif /* EMFILE */
	free(z_file[n]);
	z_file[n] = NULL;
        return(z_error = (errno ?  FX_SYS : FX_UNK)); /* Return error code */
    }
#ifdef NT
#ifdef O_SEQUENTIAL
    if (t)                              /* Caching hint for NT */
      _setmode(_fileno(t),O_SEQUENTIAL);
#endif /* O_SEQUENTIAL */
#endif /* NT */
    z_nopen++;                          /* Open, count it. */
    z_file[n]->z_fp = t;		/* Stash the file pointer */
    z_file[n]->z_flags = flags;		/* and the flags */
    z_file[n]->z_nline = 0;		/* Current line number is 0 */
    z_error = 0;
    zfnqfp(name,CKMAXPATH,z_file[n]->z_name); /* and the file's full name */
    return(n);                          /* Return the channel number */
}

int
z_close(channel) int channel; {         /* Close file on given channel */
    int x;
    FILE * t;
    if (!z_inited)                      /* Called before any files are open? */
      return(z_error = FX_NOP);
    if (channel >= z_maxchan)           /* Channel out of range? */
      return(z_error = FX_CHN);
    if (!z_file[channel])
      return(z_error = FX_NOP);
    if (!(t = z_file[channel]->z_fp))    /* Channel wasn't open? */
      return(z_error = FX_NOP);
    errno = 0;                          /* Set errno 0 to get a good reading */
    x = fclose(t);                      /* Try to close */
    if (x == EOF)                       /* On failure */
      return(z_error = FX_SYS);         /* indicate system error. */
    z_nopen--;                          /* Closed OK, decrement open count */
    z_file[channel]->z_fp = NULL;	/* Set file pointer to NULL */
    z_file[channel]->z_nline = 0;	/* Current line number is 0 */
    z_file[channel]->z_flags = 0;	/* Set flags to 0 */
    *(z_file[channel]->z_name) = '\0';	/* Clear name */
    free(z_file[channel]);
    z_file[channel] = NULL;
    return(z_error = 0);
}

/*
  Z _ O U T  --  Output string to channel.

  Call with:
    channel:     Channel number to write to.
    s:           String to write.
    length > -1: How many characters of s to write.
    length < 0:  Write entire NUL-terminated string.
    flags == 0:  Supply line termination.
    flags >  0:  Don't supply line termination.
    flags <  0:  Write 'length' NUL characters.
  Special case:
    If flags > -1 and s is empty or NULL and length == 1, write 1 NUL.
  Returns:
    Number of characters written to channel on success, or
    negative FX_xxx error code on failure.
*/
int
z_out(channel,s,length,flags) int channel, flags, length; char * s; {
    FILE * t;
    int x, n;
    char c = '\0';

    if (!s) s = "";                     /* Guard against null pointer */
#ifdef DEBUG
    if (deblog) {
        debug(F111,"z_out",s,channel);
        debug(F101,"z_out length","",length);
        debug(F101,"z_out flags","",flags);
    }
#endif /* DEBUG */
    if (!z_inited)                      /* File i/o inited? */
      return(z_error = FX_NOP);
    if (channel >= z_maxchan)           /* Channel in range? */
      return(z_error = FX_CHN);
    if (!z_file[channel])
      return(z_error = FX_NOP);
    if (!(t = z_file[channel]->z_fp))    /* File open? */
      return(z_error = FX_NOP);
    if (!((z_file[channel]->z_flags) & (FM_WRI|FM_APP))) /* In write mode? */
      return(z_error = FX_FOP);
    n = length;                         /* Length of string to write */
    if (n < 0) {                        /* Negative means get it ourselves */
        if (flags < 0)                  /* Except when told to write NULs in */
          return(z_error = FX_RNG);     /* which case args are inconsistent */
        n = strlen(s);                  /* Get length of string arg */
    }
    errno = 0;                          /* Reset errno */
    debug(F101,"z_out n","",n);
    if (flags < 0) {                    /* Writing NULs... */
        int i;
        for (i = 0; i < n; i++) {
            x = fwrite(&c,1,1,t);
            if (x < 1)
              return(z_error = (errno ? FX_SYS : FX_UNK));
        }
        z_file[channel]->z_nline = -1;   /* Current line no longer known */
        z_error = 0;
        return(i);
    } else {                            /* Writing string arg */
        if (n == 1 && !s[0])            /* Writing one char but it's NUL */
          x = fwrite(&c,1,1,t);
        else                            /* Writing non-NUL char or string */
          x = fwrite(s,1,n,t);
        debug(F101,"z_out fwrite",ckitoa(x),errno);
        if (x < n)                      /* Failure to write requested amount */
          return(z_error = (errno ? FX_SYS : FX_UNK)); /* Return error */
        if (flags == 0) {               /* If supplying line termination */
            if (fwrite("\n",1,1,t))     /* do that  */
              x += z_lt;                /* count the terminator */
            if (z_file[channel]->z_nline > -1) /* count this line */
              z_file[channel]->z_nline++;
        } else {
            z_file[channel]->z_nline = -1; /* Current line no longer known */
        }
    }
    z_error = 0;
    return(x);
}

#define Z_INBUFLEN 64

/*
  Z _ I N  --  Multichannel i/o file input function.

  Call with:
    channel number to read from.
    s = address of destination buffer.
    buflen = destination buffer length.
    length = Number of bytes to read, must be < buflen.
    flags: 0 = read a line; nonzero = read the given number of bytes.
  Returns:
    Number of bytes read into buffer or a negative error code.
    A terminating NUL is deposited after the last byte that was read.
*/
int
z_in(channel,s,buflen,length,flags)
 int channel, buflen, length, flags; char * s;
/* z_in */ {
    int i, j, x;
    FILE * t;
    char * p;

    if (!z_inited)                      /* Check everything... */
      return(z_error = FX_NOP);
    if (channel >= z_maxchan)
      return(z_error = FX_CHN);
    if (!z_file[channel])
      return(z_error = FX_NOP);
    if (!(t = z_file[channel]->z_fp))
      return(z_error = FX_NOP);
    if (!((z_file[channel]->z_flags) & FM_REA))
      return(z_error = FX_FOP);
    if (!s)                             /* Check destination */
     return(z_error = FX_RNG);
    s[0] = NUL;
    if (length == 0)                    /* Read 0 bytes - easy. */
      return(z_error = 0);
    debug(F101,"z_in channel","",channel);
    debug(F101,"z_in buflen","",buflen);
    debug(F101,"z_in length","",length);
    debug(F101,"z_in flags","",flags);
    if (length < 0 || buflen < 0)       /* Check length args */
      return(z_error = FX_RNG);
    if (buflen <= length)
      return(z_error = FX_RNG);
    errno = 0;                          /* Reset errno */
    if (flags) {                        /* Read block or byte */
	int n;				/* 20050912 */
	n = length;			/* 20050912 */
	i = 0;				/* 20050912 */
	while (n > 0) {			/* 20050912 */
	    i = fread(s,1,n,t);		/* 20050912 */
#ifdef DEBUG
	    if (deblog) {
		debug(F111,"z_in block",s,i);
		debug(F101,"z_in block errno","",errno);
		debug(F101,"z_in block ferror","",ferror(t));
		debug(F101,"z_in block feof","",feof(t));
	    }
#endif /* DEBUG */
	    if (i == 0) break;		/* 20050912 */
	    s += i;			/* 20050912 */
	    n -= i;			/* 20050912 */
	}
	/* Current line no longer known */
        z_file[channel]->z_nline = (CK_OFF_T)-1;
    } else {                            /* Read line */
#ifndef COMMENT
        /* This method is used because it's simpler than the others */
        /* and also marginally faster. */
        debug(F101,"z_in getc loop","",CKFTELL(t));
        for (i = 0; i < length; i++) {
            if ((x = getc(t)) == EOF) {
                debug(F101,"z_in getc error","",CKFTELL(t));
                s[i] = '\0';
                break;
            }
            s[i] = x;
            if (s[i] == '\n') {
                s[i] = '\0';
                break;
            }
        }
        debug(F111,"z_in line byte loop",ckitoa(errno),i);
        debug(F111,"z_in line got",s,z_file[channel]->z_nline);
        if (z_file[channel]->z_nline > -1)
          z_file[channel]->z_nline++;
#else
#ifdef COMMENT2
        /* Straightforward but strlen() slows it down. */
        s[0] = '\0';
        i = 0;
        if (fgets(s,length,t)) {
            i = strlen(s);
            if (i > 0 && s[i-1] == '\n') i--;
        }
        debug(F111,"z_in line fgets",ckitoa(errno),i);
        if (z_file[channel]->z_nline > -1)
          z_file[channel]->z_nline++;
#else
        /* This is a do-it-yourself fgets() with its own readahead and */
        /* putback.  It's a bit faster than real fgets() but not enough */
        /* to justify the added complexity or the risk of the ftell() and */
        /* fseek() calls failing. */
        int k, flag = 0;
        CK_OFF_T pos;
        for (i = 0; !flag && i <= (length - Z_INBUFLEN); i += Z_INBUFLEN) {
            k = ((length - i) < Z_INBUFLEN) ? length - i : Z_INBUFLEN;
            if ((x = fread(s+i,1,k,t)) < 1)
              break;
            s[i+x] = '\0';
            for (j = 0; j < x; j++) {
                if (s[i+j] == '\n') {
                    s[i+j] = '\0';
                    flag ++;
                    pos = CKFTELL(t);
                    if (pos > -1) {
                        pos -= (x - j - 1);
                        x = CKFSEEK(t, pos, 0);
                        i += j;
                        break;
                    } else
                      return(z_error = FX_SYS);
                }
            }
        }
        if (z_file[channel]->z_nline > -1)
          z_file[channel]->z_nline++;
        debug(F111,"z_in line chunk loop",ckitoa(errno),i);
#endif /* COMMENT2 */
#endif /* COMMENT */
    }
    debug(F111,"z_in i",ckitoa(errno),i);
    if (i < 0) i = 0;                   /* NUL-terminate result */
    s[i] = '\0';
    if (i > 0) {
        z_error = 0;
        return(i);
    }
    if (i == 0 && feof(t))              /* EOF on reading? */
      return(z_error = FX_EOF);         /* Return EOF code */
    return(errno ? (z_error = -1) : i); /* Return length or system error */
}

int
z_flush(channel) int channel; {         /* Flush output channel */
    FILE * t;
    int x;
    if (!z_inited)                      /* Regular checks */
      return(z_error = FX_NOP);
    if (channel >= z_maxchan)
      return(z_error = FX_CHN);
    if (!z_file[channel])
      return(z_error = FX_NOP);
    if (!(t = z_file[channel]->z_fp))
      return(z_error = FX_NOP);
    if (!((z_file[channel]->z_flags) & (FM_WRI|FM_APP))) /* Write access? */
      return(z_error = FX_FOP);
    errno = 0;                          /* Reset errno */
    x = fflush(t);                      /* Try to flush */
    return(x ? (z_error = FX_SYS) : 0); /* Return system error or 0 if OK */
}

int
#ifdef CK_ANSIC
z_seek(int channel, CK_OFF_T pos)	/* Move file pointer to byte */
#else
z_seek(channel,pos) int channel; CK_OFF_T pos; /* (seek to given position) */
#endif /* CK_ANSIC */
{
    int i, x = 0, rc;
    FILE * t;
    if (!z_inited)                      /* Check... */
      return(z_error = FX_NOP);
    if (channel >= z_maxchan)
      return(z_error = FX_CHN);
    if (!z_file[channel])
      return(z_error = FX_NOP);
    if (!(t = z_file[channel]->z_fp))
      return(z_error = FX_NOP);
    if (pos < 0L) {
        x = 2;
        pos = (pos == -2) ? -1L : 0L;
    }
    errno = 0;
    rc = CKFSEEK(t,pos,x);		/* Try to seek */
    debug(F111,"z_seek",ckitoa(errno),rc);
    if (rc < 0)                         /* OK? */
      return(z_error = FX_SYS); /* No. */
    z_file[channel]->z_nline = ((pos || x) ? -1 : 0);
    return(z_error = 0);
}

int
#ifdef CK_ANSIC
z_line(int channel, CK_OFF_T pos)           /* Move file pointer to line */
#else
z_line(channel,pos) int channel; CK_OFF_T pos; /* (seek to given position) */
#endif /* CK_ANSIC */
{
    int i, len, x = 0;
    CK_OFF_T current = (CK_OFF_T)0, prev = (CK_OFF_T)-1, old = (CK_OFF_T)-1;
    FILE * t;
    char tmpbuf[256];
    if (!z_inited)                      /* Check... */
      return(z_error = FX_NOP);
    if (channel >= z_maxchan)
      return(z_error = FX_CHN);
    if (!z_file[channel])
      return(z_error = FX_NOP);
    if (!(t = z_file[channel]->z_fp))
      return(z_error = FX_NOP);
    debug(F101,"z_line pos","",pos);
    if (pos < 0L) {                     /* EOF wanted */
        CK_OFF_T n;
        n = z_file[channel]->z_nline;
        debug(F101,"z_line n","",n);
        if (n < 0 || pos < 0) {
            rewind(t);
            n = 0;
        }
        while (1) {                     /* This could take a while... */
            if ((x = getc(t)) == EOF)
              break;
            if (x == '\n') {
                n++;
                if (pos == -2) {
                    old = prev;
                    prev = CKFTELL(t);
                }
            }
        }
        debug(F101,"z_line old","",old);
        debug(F101,"z_line prev","",prev);
        if (pos == -2) {
            if ((x = z_seek(channel,old)) < 0)
              return(z_error = x);
            else
              n--;
        }
        z_file[channel]->z_nline = n;
        return(z_error = 0);
    }
    if (pos == 0L) {                    /* Rewind wanted */
        z_file[channel]->z_nline = 0L;
        rewind(t);
        debug(F100,"z_line rewind","",0);
        return(0L);
    }
    tmpbuf[255] = NUL;                  /* Make sure buf is NUL terminated */
    current = z_file[channel]->z_nline;  /* Current line */
    /*
      If necessary the following could be optimized, e.g. for positioning
      to a previous line in a large file without starting over.
    */
    if (current < 0 || pos < current) { /* Not known or behind us... */
        debug(F101,"z_line rewinding","",pos);
        if ((x = z_seek(channel, 0L)) < 0) /* Rewind */
          return(z_error = x);
        if (pos == 0)                   /* If 0th line wanted we're done */
          return(z_error = 0);
        current = 0;
    }
    while (current < pos) {             /* Search for specified line */
        if (fgets(tmpbuf,255,t)) {
            len = strlen(tmpbuf);
            if (len > 0 && tmpbuf[len-1] == '\n') {
                current++;
                debug(F111,"z_line read",ckitoa(len),current);
            } else if (len == 0) {
                return(z_error = FX_UNK);
            }
        } else {
            z_file[channel]->z_nline = -1L;
            debug(F101,"z_line premature EOF","",current);
            return(z_error = FX_EOF);
        }
    }
    z_file[channel]->z_nline = current;
    debug(F101,"z_line result","",current);
    z_error = 0;
    return(current);
}

char *
z_getname(channel) int channel; {       /* Return name of file on channel */
    FILE * t;
    if (!z_inited) {
        z_error = FX_NOP;
        return(NULL);
    }
    if (channel >= z_maxchan) {
        z_error = FX_CHN;
        return(NULL);
    }
    if (!z_file[channel]) {
        z_error = FX_NOP;
        return(NULL);
    }
    if (!(t = z_file[channel]->z_fp)) {
        z_error = FX_NOP;
        return(NULL);
    }
    return((char *)(z_file[channel]->z_name));
}

int
z_getmode(channel) int channel; {       /* Return OPEN modes of channel */
    FILE * t;                           /* 0 if file not open */
#ifndef NOSTAT
#ifdef NT
    struct _stat statbuf;
#else /* NT */
    struct stat statbuf;
#endif /* NT */
#endif /* NOSTAT */
    int x;
    if (!z_inited)
      return(0);
    if (channel >= z_maxchan)
      return(z_error = FX_CHN);
    if (!z_file[channel])
      return(0);
    if (!(t = z_file[channel]->z_fp))
      return(0);
    x = z_file[channel]->z_flags;
    if (feof(t)) {                      /* This might not work for */
        x |= FM_EOF;                    /* output files */
#ifndef NOSTAT
    /* But this does if we can use it. */
    } else if (stat(z_file[channel]->z_name,&statbuf) > -1) {
        if (CKFTELL(t) == statbuf.st_size)
          x |= FM_EOF;
#endif /* NOSTAT */
    }
    return(x);
}

CK_OFF_T
z_getpos(channel) int channel; {        /* Get file pointer position */
    FILE * t;                           /* on this channel */
    CK_OFF_T x;
    if (!z_inited)
      return(z_error = FX_NOP);
    if (channel >= z_maxchan)
      return(z_error = FX_CHN);
    if (!z_file[channel])
      return(z_error = FX_NOP);
    if (!(t = z_file[channel]->z_fp))
      return(z_error = FX_NOP);
    x = CKFTELL(t);
    return((x < 0L) ? (z_error = FX_SYS) : x);
}

CK_OFF_T
z_getline(channel) int channel; {       /* Get current line number */
    FILE * t;                           /* in file on this channel */
    CK_OFF_T rc;
    if (!z_inited)
      return(z_error = FX_NOP);
    if (channel >= z_maxchan)
      return(z_error = FX_CHN);
    if (!z_file[channel])
      return(z_error = FX_NOP);
    if (!(t = z_file[channel]->z_fp))
      return(z_error = FX_NOP);
    debug(F101,"z_getline","",z_file[channel]->z_nline);
    rc = z_file[channel]->z_nline;
    return((rc < 0) ? (z_error = FX_LNU) : rc);
}

int
z_getfnum(channel) int channel; {       /* Get file number / handle */
    FILE * t;                           /* for file on this channel */
    if (!z_inited)
      return(z_error = FX_NOP);
    if (channel >= z_maxchan)
      return(z_error = FX_CHN);
    if (!z_file[channel])
      return(z_error = FX_NOP);
    if (!(t = z_file[channel]->z_fp))
      return(z_error = FX_NOP);
    z_error = 0;
    return(fileno(t));
}

/*
  Line-oriented counts and seeks are as dumb as they can be at the moment.
  Later we can speed them up by building little indexes.
*/
CK_OFF_T
z_count(channel, what) int channel, what; { /* Count bytes or lines in file */
    FILE * t;
    int i, x;
    CK_OFF_T pos, count = (CK_OFF_T)0;
    if (!z_inited)                      /* Check stuff... */
      return(z_error = FX_NOP);
    if (channel >= z_maxchan)
      return(z_error = FX_CHN);
    if (!z_file[channel])
      return(z_error = FX_NOP);
    if (!(t = z_file[channel]->z_fp))
      return(z_error = FX_NOP);
    pos = CKFTELL(t);			/* Save current file pointer */
    errno = 0;
    z_error = 0;
    if (what == RD_CHAR) {              /* Size in bytes requested */
#ifdef COMMENT
        if (!CKFSEEK(t,0L,2)) {		/* Seek to end */
            count = CKFTELL(t);		/* Get file pointer */
            CKFSEEK(t,pos,0);		/* Restore file file pointer */
            return(count);
        } else                          /* Fallback in case seek fails */
#endif	/* COMMENT */
          return(zgetfs(z_file[channel]->z_name));
    }
    rewind(t);                          /* Line count requested - rewind. */
    while (1) {                         /* Count lines. */
        if ((x = getc(t)) == EOF)       /* Stupid byte loop */
          break;                        /* but it works as well as anything */
        if (x == '\n')                  /* else... */
          count++;
    }
    x = CKFSEEK(t,pos,0);		/* Restore file pointer */
    return(count);
}

/* User interface for generalized channel-oriented file i/o */

struct keytab fctab[] = {               /* FILE subcommands */
    { "close",      FIL_CLS, 0 },
    { "count",      FIL_COU, 0 },
    { "flush",      FIL_FLU, 0 },
    { "list",       FIL_LIS, 0 },
    { "open",       FIL_OPN, 0 },
    { "read",       FIL_REA, 0 },
    { "rewind",     FIL_REW, 0 },
    { "seek",       FIL_SEE, 0 },
    { "status",     FIL_STA, 0 },
    { "write",      FIL_WRI, 0 }
};
int nfctab = (sizeof (fctab) / sizeof (struct keytab));

static struct keytab fcswtab[] = {      /* OPEN modes */
    { "/append",    FM_APP,  0 },
    { "/binary",    FM_BIN,  0 },
#ifdef COMMENT
    { "/command",   FM_CMD,  0 },       /* Not implemented */
#endif /* COMMENT */
    { "/read",      FM_REA,  0 },
    { "/write",     FM_WRI,  0 }
};
static int nfcswtab = (sizeof (fcswtab) / sizeof (struct keytab));

static struct keytab fclkwtab[] = {     /* CLOSE options */
    { "all",        1,       0 }
};

static struct keytab fsekwtab[] = {     /* SEEK symbols */
    { "eof",        1,       0 },
    { "last",       2,       0 }
};
static int nfsekwtab = (sizeof (fsekwtab) / sizeof (struct keytab));

#define SEE_LINE  RD_LINE               /* SEEK options */
#define SEE_CHAR  RD_CHAR
#define SEE_REL   3
#define SEE_ABS   4
#define SEE_FIND  5

static struct keytab fskswtab[] = {
    { "/absolute",  SEE_ABS,  0 },
    { "/byte",      SEE_CHAR, 0 },
    { "/character", SEE_CHAR, CM_INV },
    { "/find",      SEE_FIND, CM_ARG },
    { "/line",      SEE_LINE, 0 },
    { "/relative",  SEE_REL,  0 }
};
static int nfskswtab = (sizeof (fskswtab) / sizeof (struct keytab));

#define COU_LINE  RD_LINE               /* COUNT options */
#define COU_CHAR  RD_CHAR
#define COU_LIS   3
#define COU_NOL   4

static struct keytab fcoswtab[] = {
    { "/bytes",     COU_CHAR, 0      },
    { "/characters",COU_CHAR, CM_INV },
    { "/lines",     COU_LINE, 0      },
    { "/list",      COU_LIS,  0      },
    { "/nolist",    COU_NOL,  0      },
    { "/quiet",     COU_NOL,  CM_INV }
};
static int nfcoswtab = (sizeof (fcoswtab) / sizeof (struct keytab));

static struct keytab frdtab[] = {       /* READ types */
    { "/block",     RD_SIZE, CM_INV|CM_ARG },
    { "/byte",      RD_CHAR, CM_INV },
    { "/character", RD_CHAR, 0      },
    { "/line",      RD_LINE, 0      },
    { "/size",      RD_SIZE, CM_ARG },
    { "/trim",      RD_TRIM, 0      },
    { "/untabify",  RD_UNTA, 0      }
};
static int nfrdtab = (sizeof (frdtab) / sizeof (struct keytab));

static struct keytab fwrtab[] = {       /* WRITE types */
    { "/block",     WR_SIZE, CM_INV|CM_ARG },
    { "/byte",      WR_CHAR, CM_INV },
    { "/character", WR_CHAR, 0      },
    { "/line",      WR_LINE, 0      },
    { "/lpad",      WR_LPAD, CM_ARG },
    { "/rpad",      WR_RPAD, CM_ARG },
    { "/size",      WR_SIZE, CM_ARG },
    { "/string",    WR_STRI, 0      }
};
static int nfwrtab = (sizeof (fwrtab) / sizeof (struct keytab));

static char blanks[] = "\040\040\040\040"; /* Some blanks for formatting */
static char * seek_target = NULL;

int
dofile(op) int op; {                    /* Do the FILE command */
    char vnambuf[VNAML];                /* Buffer for variable names */
    char *vnp = NULL;                   /* Pointer to same */
    char zfilnam[CKMAXPATH+2];
    char * p, * m;
    struct FDB fl, sw, nu;
    CK_OFF_T z;
    int rsize, filmode = 0, relative = -1, eofflg = 0;
    int rc, x, y, cx, n, getval, dummy, confirmed, listing = -1;
    int charflag = 0, sizeflag = 0;
    int pad = 32, wr_lpad = 0, wr_rpad = 0, rd_trim = 0, rd_untab = 0;

    makestr(&seek_target,NULL);

    if (op == XXFILE) {                 /* FILE command was given */
        /* Get subcommand */
        if ((cx = cmkey(fctab,nfctab,"Operation","",xxstring)) < 0) {
            if (cx == -3) {
                printf("?File operation required\n");
                x = -9;
            }
            return(cx);
        }
    } else {                            /* Shorthand command was given */
        switch (op) {
          case XXF_CL: cx = FIL_CLS; break; /* FCLOSE */
          case XXF_FL: cx = FIL_FLU; break; /* FFLUSH */
          case XXF_LI: cx = FIL_LIS; break; /* FLIST */
          case XXF_OP: cx = FIL_OPN; break; /* etc... */
          case XXF_RE: cx = FIL_REA; break;
          case XXF_RW: cx = FIL_REW; break;
          case XXF_SE: cx = FIL_SEE; break;
          case XXF_ST: cx = FIL_STA; break;
          case XXF_WR: cx = FIL_WRI; break;
          case XXF_CO: cx = FIL_COU; break;
          default: return(-2);
        }
    }
    switch (cx) {                       /* Do requested subcommand */
      case FIL_OPN:                     /* OPEN */
        cmfdbi(&sw,                     /* Switches */
               _CMKEY,                  /* fcode */
               "Variable or switch",    /* hlpmsg */
               "",                      /* default */
               "",                      /* addtl string data */
               nfcswtab,                /* addtl numeric data 1: tbl size */
               4,                       /* addtl numeric data 2: 4 = cmswi */
               xxstring,                /* Processing function */
               fcswtab,                 /* Keyword table */
               &fl                      /* Pointer to next FDB */
               );
        cmfdbi(&fl,                     /* Anything that doesn't match */
               _CMFLD,                  /* fcode */
               "Variable",              /* hlpmsg */
               "",
               "",
               0,
               0,
               NULL,
               NULL,
               NULL
               );
        while (1) {
            x = cmfdb(&sw);             /* Parse something */
            if (x < 0) {
                if (x == -3) {
                    printf("?Variable name and file name required\n");
                    x = -9;
                }
                return(x);
            }
            if (cmresult.fcode == _CMFLD)
              break;
            else if (cmresult.fcode == _CMKEY) {
                char c;
                c = cmgbrk();
                if ((getval =
                     (c == ':' || c == '=')) && !(cmgkwflgs() & CM_ARG)) {
                    printf("?This switch does not take an argument\n");
                    return(-9);
                }
#ifdef COMMENT
                /* Uncomment if we add any switches here that take args */
                if (!getval && (cmgkwflgs() & CM_ARG)) {
                    printf("?This switch requires an argument\n");
                    return(-9);         /* (none do...) */
                }
#endif /* COMMENT */
                filmode |= cmresult.nresult; /* OR in the file mode */
            } else
              return(-2);
        }
        /* Not a switch - get the string */
        ckstrncpy(vnambuf,cmresult.sresult,VNAML);
        if (!vnambuf[0] || chknum(vnambuf)) { /* (if there is one...) */
            printf("?Variable name required\n");
            return(-9);
        }
        vnp = vnambuf;                  /* Check variable-name syntax */
        if (vnambuf[0] == CMDQ &&
            (vnambuf[1] == '%' || vnambuf[1] == '&'))
          vnp++;
        y = 0;
        if (*vnp == '%' || *vnp == '&') {
            if ((y = parsevar(vnp,&x,&dummy)) < 0) {
                printf("?Syntax error in variable name\n");
                return(-9);
            }
        }
	/* Assign a negative channel number in case we fail */
	addmac(vnambuf,"-1");

        if (!(filmode & FM_RWA))        /* If no access mode specified */
          filmode |= FM_REA;            /* default to /READ. */

        y = 0;                          /* Now parse the filename */
        if ((filmode & FM_RWA) == FM_WRI) {
	    x = cmofi("Name of new file","",&s,xxstring);
	} else if ((filmode & FM_RWA) == FM_REA) {
	    x = cmifi("Name of existing file","",&s,&y,xxstring);
	} else {
            x = cmiofi("Filename","",&s,&y,xxstring);
            debug(F111,"fopen /append x",s,x);
	}
        if (x < 0) {
            if (x == -3) {
                printf("?Filename required\n");
                x = -9;
            }
            return(x);
        }
        if (y) {                        /* No wildcards */
            printf("?Wildcards not allowed here\n");
            return(-9);
        }
        if (filmode & (FM_APP|FM_WRI)) { /* Check output access */
#ifndef VMS
            if (zchko(s) < 0) {          /* and set error code if denied */
                z_error = FX_ACC;
                printf("?Write access denied - \"%s\"\n",s);
                return(-9);
            }
#endif /* VMS */
        }
        ckstrncpy(zfilnam,s,CKMAXPATH); /* Is OK - make safe copy */
        if ((x = cmcfm()) < 0)          /* Get confirmation of command */
          return(x);
        if ((n = z_open(zfilnam,filmode)) < 0) {
            printf("?OPEN failed - %s: %s\n",zfilnam,ckferror(n));
            return(-9);
        }
        addmac(vnambuf,ckitoa(n));      /* Assign channel number to variable */
        return(success = 1);

      case FIL_REW:                     /* REWIND */
        if ((x = cmnum("Channel number","",10,&n, xxstring)) < 0) {
            if (x == -3) {
                printf("?Channel number required\n");
                x = -9;
            }
            return(x);
        }
        if ((x = cmcfm()) < 0)
          return(x);
	if (n == -9) return(success = 0);
	if (n == -8) return(success = 1);

        if ((rc = z_seek(n,0L)) < 0) {
            printf("?REWIND failed - Channel %d: %s\n",n,ckferror(rc));
            return(-9);
        }
        return(success = 1);

      case FIL_CLS:                     /* CLOSE */
#ifdef COMMENT				/* fdc 20100804 - bad idea */
         {
	    int i, j, k;		/* Supply default if only one open */
	    s = "";
	    for (k = 0, j = 0, i = 0; i < z_maxchan; i++) {
		if (z_file)
		  if (z_file[i])
		    if (z_file[i]->z_fp) { k++; j = i; }
	    }
	    if (k == 1) s = ckitoa(j);
	 }
#endif	/* COMMENT */
          cmfdbi(&nu,                   /* Second FDB - channel number */
                 _CMNUM,                /* fcode */
                 "Channel number or ALL", /* Help message */
                 s,			/* default */
                 "",                    /* addtl string data */
                 10,                    /* addtl numeric data 1: radix */
                 0,                     /* addtl numeric data 2: 0 */
                 xxstring,              /* Processing function */
                 NULL,                  /* Keyword table */
                 &sw			/* Pointer to next FDB */
                 );                     /* Pointer to next FDB */
	 cmfdbi(&sw,			/* First FDB - command switches */
                 _CMKEY,                /* fcode */
                 "",			/* help message */
		 "",			/* Default */
		 "",			/* No addtl string data */
                 1,                     /* addtl numeric data 1: tbl size */
                 0,                     /* addtl numeric data 2: 4 = cmswi */
                 xxstring,              /* Processing function */
                 fclkwtab,              /* Keyword table */
		 NULL			/* Last in chain */
                 );
        x = cmfdb(&nu);                 /* Parse something */
        if (x < 0) {
            if (x == -3) {
                printf("?Channel number or ALL required\n");
                x = -9;
            }
            return(x);
        }
        if (cmresult.fcode == _CMNUM)
          n = cmresult.nresult;
        else if (cmresult.fcode == _CMKEY)
          n = -1;
        if ((x = cmcfm()) < 0)
          return(x);
	if (n == -9) return(success = 0);
	if (n == -8) return(success = 1);

        rc = 1;
        if (n < 0) {
            int count = 0;
            int i;
            for (i = 0; i < z_maxchan; i++) {
                x = z_close(i);
                if (x == FX_SYS) {
                    printf("?CLOSE failed - Channel %d: %s\n",n,ckferror(x));
                    rc = 0;
                } else if (x > -1)
                  count++;
            }
            debug(F101,"FILE CLOSE ALL","",count);
        } else if ((x = z_close(n)) < 0) {
            printf("?CLOSE failed - Channel %d: %s\n",n,ckferror(x));
            return(-9);
        }
        return(success = rc);

      case FIL_REA:                     /* READ */
      case FIL_WRI:                     /* WRITE */
        rsize = 0;
        cmfdbi(&sw,                     /* Switches */
               _CMKEY,                  /* fcode */
               "Channel or switch",     /* hlpmsg */
               "",                      /* default */
               "",                      /* addtl string data */
               (cx == FIL_REA) ? nfrdtab : nfwrtab,
               4,                       /* addtl numeric data 2: 4 = cmswi */
               xxstring,                /* Processing function */
               (cx == FIL_REA) ? frdtab : fwrtab, /* Keyword table */
               &nu                      /* Pointer to next FDB */
               );
        cmfdbi(&nu,                     /* Channel number */
               _CMNUM,                  /* fcode */
               "Channel",
               "",                      /* default */
               "",                      /* addtl string data */
               10,                      /* addtl numeric data 1: radix */
               0,                       /* addtl numeric data 2: 0 */
               xxstring,                /* Processing function */
               NULL,                    /* Keyword table */
               NULL                     /* Pointer to next FDB */
               );
        do {
            x = cmfdb(&sw);             /* Parse something */
            if (x < 0) {
                if (x == -3) {
                    printf("?Channel number required\n");
                    x = -9;
                }
                return(x);
            }
            if (cmresult.fcode == _CMNUM) /* Channel number */
              break;
            else if (cmresult.fcode == _CMKEY) { /* Switch */
                char c;
                c = cmgbrk();
                if ((getval =
                     (c == ':' || c == '=')) && !(cmgkwflgs() & CM_ARG)) {
                    printf("?This switch does not take an argument\n");
                    return(-9);
                }
                if (!getval && (cmgkwflgs() & CM_ARG)) {
                    printf("?This switch requires an argument\n");
                    return(-9);
                }
                switch (cmresult.nresult) {
                  case WR_LINE:
                    charflag = 0;
                    sizeflag = 0;
                    rsize = 0;
                    break;
                  case WR_CHAR:
                    rsize = 1;
                    charflag = 1;
                    sizeflag = 1;
                    break;
                  case WR_SIZE:
                    if ((x = cmnum("Bytes","",10,&rsize, xxstring)) < 0) {
                        if (x == -3) {
                            printf("?Number required\n");
                            x = -9;
                        }
                        return(x);
                    }
		    if (rsize > LINBUFSIZ) {
			printf("?Maximum FREAD/FWRITE size is %d\n",LINBUFSIZ);
			rsize = 0;
			return(-9);
		    }
                    charflag = 0;
                    sizeflag = 1;
                    break;
                  case WR_STRI:
                    rsize = 1;
                    charflag = 0;
                    sizeflag = 0;
                    break;
                  case WR_LPAD:
                  case WR_RPAD:
                    if ((x = cmnum("Numeric ASCII character value",
                                   "32",10,&pad, xxstring)) < 0)
                      return(x);
                    if (cmresult.nresult == WR_LPAD)
                      wr_lpad = 1;
                    else
                      wr_rpad = 1;
                    break;
		  case RD_TRIM:
		    rd_trim = 1;
		    break;
		  case RD_UNTA:
		    rd_untab = 1;
		    break;
                }
                debug(F101,"FILE READ rsize 2","",rsize);
            } else
              return(-2);
        } while
          (cmresult.fcode == _CMKEY);

        n = cmresult.nresult;           /* Channel */
        debug(F101,"FILE READ/WRITE channel","",n);

        if (cx == FIL_WRI) {            /* WRITE */
            int len = 0;
            if ((x = cmtxt("Text","",&s,xxstring)) < 0)
              return(x);
	    if (n == -9) return(success = 0);
	    if (n == -8) return(success = 1);

            ckstrncpy(line,s,LINBUFSIZ); /* Make a safe copy */
            s = line;
            s = brstrip(s);             /* Strip braces */
            if (charflag) {             /* Write one char */
                len = 1;                /* So length = 1 */
                rsize = 1;              /* Don't supply terminator */
            } else if (!sizeflag) {     /* Write a string */
                len = -1;               /* So length is unspecified */
            } else {                    /* Write a block of given size */
                int i, k, xx;
                if (rsize > TMPBUFSIZ) {
                    z_error = FX_OFL;
                    printf("?Buffer overflow\n");
                    return(-9);
                }
                len = rsize;            /* rsize is really length */
                rsize = 1;              /* Don't supply a terminator */
                xx = strlen(s);         /* Size of given string */
                if (xx >= len) {        /* Bigger or equal */
                    s[len] = NUL;
                } else if (wr_lpad) {   /* Smaller, left-padding requested */
                    for (i = 0; i < len - xx; i++) /* Must make a copy */
                      tmpbuf[i] = pad;
                    ckstrncpy(tmpbuf+i,s,TMPBUFSIZ-i);
                    tmpbuf[len] = NUL;
                    s = tmpbuf;         /* Redirect write source */
                } else if (wr_rpad) {   /* Smaller with right-padding */
                    for (i = xx; i < len; i++)
                      s[i] = pad;
                    s[len] = NUL;
                }
            }
            if ((rc = z_out(n,s,len,rsize)) < 0) { /* Try to write */
                printf("?Channel %d WRITE error: %s\n",n,ckferror(rc));
                return(-9);
            }
        } else {                        /* FIL_REA READ */
            confirmed = 0;
            vnambuf[0] = NUL;
            x = cmfld("Variable name","",&s,NULL);
            debug(F111,"FILE READ cmfld",s,x);
            if (x < 0) {
                if (x == -3 || !*s) {
                    if ((x = cmcfm()) < 0)
                      return(x);
                    else
                      confirmed++;
                } else
                  return(x);
            }
            ckstrncpy(vnambuf,s,VNAML);
            debug(F111,"FILE READ vnambuf",vnambuf,confirmed);
            if (vnambuf[0]) {           /* Variable name given, check it */
                if (!confirmed) {
                    x = cmcfm();
                    if (x < 0)
                      return(x);
                    else
                      confirmed++;
                }
                vnp = vnambuf;
                if (vnambuf[0] == CMDQ &&
                    (vnambuf[1] == '%' || vnambuf[1] == '&'))
                  vnp++;
                y = 0;
                if (*vnp == '%' || *vnp == '&') {
                    if ((y = parsevar(vnp,&x,&dummy)) < 0) {
                        printf("?Syntax error in variable name\n");
                        return(-9);
                    }
                }
            }
            debug(F111,"FILE READ variable",vnambuf,confirmed);

            if (!confirmed)
              if ((x = cmcfm()) < 0)
                return(x);

	    if (n == -9) return(success = 0);
	    if (n == -8) return(success = 1);

            line[0] = NUL;              /* Clear destination buffer */
#ifdef COMMENT
            if (rsize >= LINBUFSIZ)     /* Don't overrun it */
              rsize = LINBUFSIZ - 1;
#endif	/* COMMENT */

            if (rsize == 0) {		/* Read a line */
		rc = z_in(n,line,LINBUFSIZ,LINBUFSIZ-1,0);
            } else {
		rc = z_in(n,line,LINBUFSIZ,rsize,1); /* Read a block */
	    }
            if (rc < 0) {               /* Error... */
                debug(F101,"FILE READ error","",rc);
                debug(F101,"FILE READ errno","",errno);
                if (rc == FX_EOF) {     /* EOF - fail but no error message */
                    return(success = 0);
                } else {                /* Other error - fail and print msg */
                    printf("?READ error: %s\n",ckferror(rc));
                    return(-9);
                }
            }
	    if (rsize == 0) {		/* FREAD /LINE postprocessing */
		if (rd_trim) {		/* Trim */
		    int i, k;
		    k = strlen(line);
		    if (k > 0) {
			for (i = k-1; i > 0; i--) {
			    if (line[i] == SP || line[i] == '\t')
			      line[i] = NUL;
			    else
			      break;
			}
		    }
		}
		if (rd_untab) {		/* Untabify */
		    if (untabify(line,tmpbuf,TMPBUFSIZ) > -1)
		      ckstrncpy(line,tmpbuf,LINBUFSIZ);
		}
	    }
            debug(F110,"FILE READ data",line,0);
            if (vnambuf[0])             /* Read OK - If variable name given */
              addmac(vnambuf,line);     /* Assign result to variable */
            else                        /* otherwise */
              printf("%s\n",line);      /* just print it */
        }
        return(success = 1);

      case FIL_SEE:                     /* SEEK */
      case FIL_COU:                     /* COUNT */
        rsize = RD_CHAR;                /* Defaults to /BYTE */
        cmfdbi(&sw,                     /* Switches */
               _CMKEY,                  /* fcode */
               "Channel or switch",     /* hlpmsg */
               "",                      /* default */
               "",                      /* addtl string data */
               ((cx == FIL_SEE) ? nfskswtab : nfcoswtab),
               4,                       /* addtl numeric data 2: 4 = cmswi */
               xxstring,                /* Processing function */
               ((cx == FIL_SEE) ? fskswtab : fcoswtab),
               &nu                      /* Pointer to next FDB */
               );
        cmfdbi(&nu,                     /* Channel number */
               _CMNUM,                  /* fcode */
               "Channel",
               "",                      /* default */
               "",                      /* addtl string data */
               10,                      /* addtl numeric data 1: radix */
               0,                       /* addtl numeric data 2: 0 */
               xxstring,                /* Processing function */
               NULL,                    /* Keyword table */
               NULL                     /* Pointer to next FDB */
               );
        do {
            x = cmfdb(&sw);             /* Parse something */
            if (x < 0) {
                if (x == -3) {
                    printf("?Channel number required\n");
                    x = -9;
                }
                return(x);
            }
            if (cmresult.fcode == _CMNUM) /* Channel number */
              break;
            else if (cmresult.fcode == _CMKEY) { /* Switch */
                char c;
                c = cmgbrk();
                if ((getval =
                     (c == ':' || c == '=')) && !(cmgkwflgs() & CM_ARG)) {
                    printf("?This switch does not take an argument\n");
                    return(-9);
                }
                if (cx == FIL_SEE) {
                    switch (cmresult.nresult) {
                      case SEE_REL: relative = 1; break;
                      case SEE_ABS: relative = 0; break;
		      case SEE_FIND: {
			  if (getval) {
			      y = cmfld("string or pattern","",&s,xxstring);
			      if (y < 0)
				return(y);
			      makestr(&seek_target,brstrip(s));
			      break;
			  }
		      }
                      default: rsize = cmresult.nresult;
                    }
                } else if (cx == FIL_COU) {
                    switch (cmresult.nresult) {
                      case COU_LIS: listing = 1; break;
                      case COU_NOL: listing = 0; break;
                      default: rsize = cmresult.nresult;
                    }
                }
            }
        } while
          (cmresult.fcode == _CMKEY);

        n = cmresult.nresult;           /* Channel */
        debug(F101,"FILE SEEK/COUNT channel","",n);
        if (cx == FIL_COU) {
            if ((x = cmcfm()) < 0)
              return(x);
	    if (n == -9) return(success = 0);
	    if (n == -8) return(success = 1);

            z_filcount = z_count(n,rsize);
            if (z_filcount < 0) {
                rc = z_filcount;
                printf("?COUNT error: %s\n",ckferror(rc));
                return(-9);
            }
            if (listing < 0)
              listing = !xcmdsrc;
            if (listing)
              printf(" %ld %s%s\n",
                     z_filcount,
                     ((rsize == RD_CHAR) ? "byte" : "line"),
                     ((z_filcount == 1L) ? "" : "s")
                     );
            return(success = (z_filcount > -1) ? 1 : 0);
        }
	m = (rsize == RD_CHAR) ?
	    "Number of bytes;\n or keyword" :
	    "Number of lines;\n or keyword";
        cmfdbi(&sw,                     /* SEEK symbolic targets (EOF) */
               _CMKEY,                  /* fcode */
               m,
               "",
               "",                      /* addtl string data */
               nfsekwtab,               /* addtl numeric data 1: table size */
               0,                       /* addtl numeric data 2: 4 = cmswi */
               xxstring,                /* Processing function */
               fsekwtab,                /* Keyword table */
               &nu                      /* Pointer to next FDB */
               );
        cmfdbi(&nu,                     /* Byte or line number */
               _CMNUW,                  /* fcode */
               "",
               "",                      /* default */
               "",                      /* addtl string data */
               10,                      /* addtl numeric data 1: radix */
               0,                       /* addtl numeric data 2: 0 */
               xxstring,                /* Processing function */
               NULL,                    /* Keyword table */
               NULL                     /* Pointer to next FDB */
               );
        x = cmfdb(&sw);                 /* Parse something */
        if (x < 0) {
            if (x == -3) {
                printf("?Channel number or EOF required\n");
                x = -9;
            }
            return(x);
        }
        if (cmresult.fcode == _CMNUW) {
            z = cmresult.wresult;
            debug(F110,"FILE SEEK atmbuf",atmbuf,0);
            if (relative < 0) {
                if (cx == FIL_SEE && (atmbuf[0] == '+' || atmbuf[0] == '-'))
                  relative = 1;
                else
                  relative = 0;
            }
        } else if (cmresult.fcode == _CMKEY) {
            eofflg = cmresult.nresult;
            relative = 0;
            y = 0 - eofflg;
        }
        if ((x = cmcfm()) < 0)
          return(x);
	if (n == -9) return(success = 0);
	if (n == -8) return(success = 1);
        y = 1;                          /* Recycle this */
        z_flush(n);
        debug(F101,"FILE SEEK relative","",relative);
        debug(F101,"FILE SEEK rsize","",rsize);

        if (rsize == RD_CHAR) {         /* Seek to byte position */
            if (relative > 0) {
                CK_OFF_T pos;
                pos = z_getpos(n);
                if (pos < (CK_OFF_T)0) {
                    rc = pos;
                    printf("?Relative SEEK failed: %s\n",ckferror(rc));
                    return(-9);
                }
                z += pos;
            } else {
                if (z < 0 && !eofflg) { /* Negative arg but not relative */
                    y = 0;              /* Remember this was bad */
                    z = 0;              /* but substitute 0 */
                }
            }
            debug(F101,"FILE SEEK /CHAR z","",z);
            if (z < 0 && !eofflg) {
                z_error = FX_RNG;
                return(success = 0);
            }
            if ((rc = z_seek(n,z)) < 0) {
                if (rc == FX_EOF) return(success = 0);
                printf("?SEEK /BYTE failed - Channel %d: %s\n",n,ckferror(rc));
                return(-9);
            }
        } else {                        /* Seek to line */
            if (relative > 0) {
                CK_OFF_T pos;
                pos = z_getline(n);
                debug(F101,"FILE SEEK /LINE pos","",pos);
                if (pos < 0) {
                    rc = pos;
                    printf("?Relative SEEK failed: %s\n",ckferror(rc));
                    return(-9);
                }
                z += pos;
            }
            debug(F101,"FILE SEEK /LINE z","",z);
            debug(F101,"FILE SEEK /LINE eofflg","",eofflg);
            if (z < 0 && !eofflg) {
                z_error = FX_RNG;
                return(success = 0);
            }
            if ((rc = z_line(n,z)) < 0) {
                if (rc == FX_EOF) return(success = 0);
                printf("?SEEK /LINE failed - Channel %d: %s\n",n,ckferror(rc));
                return(-9);
            }
        }
	/*
	  Now, having sought to the desired starting spot, if a /FIND:
	  target was specified, look for it now.
	*/
	if (seek_target) {
	    int flag = 0, ispat = 0, matchresult = 0;
	    while (!flag) {
		y = z_in(n,line,LINBUFSIZ,LINBUFSIZ-1,0);
		if (y < 0) {
		    y = 0;
		    break;
		}
		if (ispattern(seek_target)) {
		    matchresult = ckmatch(seek_target,line,inpcas[cmdlvl],1+4);
		} else {
		    /* This is faster */
		    matchresult = ckindex(seek_target,line,0,0,inpcas[cmdlvl]);
		}
		if (matchresult) {
		    flag = 1;
		    break;
		}
	    }
	    if (flag) {
		debug(F111,"FSEEK HAVE MATCH",seek_target,z_getline(n));
		/* Back up to beginning of line where target found */
		if ((y = z_line(n,z_getline(n)-1)) < 0) {
		    if (rc == FX_EOF) return(success = 0);
		    printf("?SEEK /LINE failed - Channel %d: %s\n",
			   n,ckferror(rc));
		    return(-9);
		}
		debug(F101,"FSEEK LINE","",y);
	    }
	}
        return(success = (y < 0) ? 0 : 1);

      case FIL_LIS: {                   /* LIST open files */
#ifdef CK_TTGWSIZ
          extern int cmd_rows, cmd_cols;
#endif /* CK_TTGWSIZ */
          extern int xaskmore;
          int i, x, n = 0, paging = 0;
          char * s;

          if ((x = cmcfm()) < 0)
            return(x);

#ifdef CK_TTGWSIZ
          if (cmd_rows > 0 && cmd_cols > 0)
#endif /* CK_TTGWSIZ */
            paging = xaskmore;

          printf("System open file limit:%5d\n", z_openmax);
          printf("Maximum for FILE OPEN: %5d\n", z_maxchan);
          printf("Files currently open:  %5d\n\n", z_nopen);
          n = 4;
          for (i = 0; i < z_maxchan; i++) {
              s = z_getname(i);         /* Got one? */
              if (s) {                  /* Yes */
                  char m[8];
                  m[0] = NUL;
                  printf("%2d. %s",i,s); /* Print name */
                  n++;                   /* Count it */
                  x = z_getmode(i);      /* Get modes & print them */
                  if (x > 0) {
                      if (x & FM_REA) ckstrncat(m,"R",8);
                      if (x & FM_WRI) ckstrncat(m,"W",8);
                      if (x & FM_APP) ckstrncat(m,"A",8);
                      if (x & FM_BIN) ckstrncat(m,"B",8);
                      if (m[0])
                        printf(" (%s)",m);
                      if (x & FM_EOF)
                        printf(" [EOF]");
                      else		/* And file position too */
                        printf(" %s",ckfstoa(z_getpos(i)));
                  }
                  printf("\n");
#ifdef CK_TTGWSIZ
                  if (paging > 0) {     /* Pause at end of screen */
                      if (n > cmd_rows - 3) {
                          if (!askmore())
                            break;
                          else
                            n = 0;
                      }
                  }
#endif /* CK_TTGWSIZ */
              }
          }
          return(success = 1);
      }

      case FIL_FLU:                     /* FLUSH */
        if ((x = cmnum("Channel number","",10,&n, xxstring)) < 0) {
            if (x == -3) {
                printf("?Channel number required\n");
                x = -9;
            }
            return(x);
        }
        if ((x = cmcfm()) < 0)
          return(x);
	if (n == -9) return(success = 0);
	if (n == -8) return(success = 1);
        if ((rc = z_flush(n)) < 0) {
            printf("?FLUSH failed - Channel %d: %s\n",n,ckferror(rc));
            return(-9);
        }
        return(success = 1);

      case FIL_STA:                     /* STATUS */
	{
	    int i, j, k;		/* Supply default if only one open */
	    s = "";
	    for (k = 0, j = 0, i = 0; i < z_maxchan; i++) {
		if (z_file)
		  if (z_file[i])
		    if (z_file[i]->z_fp) { k++; j = i; }
	    }
	    if (k == 1) s = ckitoa(j);
	}
        if ((x = cmnum("Channel number",s,10,&n, xxstring)) < 0) {
            if (x == -3) {
		if (z_nopen > 1) {
		    printf("?%d files open - please supply channel number\n",
			   z_nopen);
		    return(-9);
		}
            } else
	      return(x);
        }
        if ((y = cmcfm()) < 0)
          return(y);
	if ((!z_file || z_nopen == 0) && x == -3) {
	    printf("No files open\n");
	    return(success = 1);
	}
        p = blanks + 3;                 /* Tricky formatting... */
        if (n < 1000) p--;
        if (n < 100) p--;
        if (n < 10) p--;
        if ((rc = z_getmode(n)) < 0) {
            printf("Channel %d:%s%s\n",n,p,ckferror(rc));
            return(success = 0);
        } else if (!rc) {
            printf("Channel %d:%sNot open\n",n,p);
            return(success = 0);
        } else {
            CK_OFF_T xx;
            s = z_getname(n);
            if (!s) s = "(name unknown)";
            printf("Channel %d:%sOpen\n",n,p);
            printf(" File:        %s\n Modes:      ",s);
            if (rc & FM_REA) printf(" /READ");
            if (rc & FM_WRI) printf(" /WRITE");
            if (rc & FM_APP) printf(" /APPEND");
            if (rc & FM_BIN) printf(" /BINARY");
            if (rc & FM_CMD) printf(" /COMMAND");
            if (rc & FM_EOF) printf(" [EOF]");
            printf("\n Size:        %s\n",ckfstoa(z_count(n,RD_CHAR)));
            printf(" At byte:     %s\n",ckfstoa(z_getpos(n)));
            xx = z_getline(n);
            if (xx > (CK_OFF_T)-1)
              printf(" At line:     %s\n",ckfstoa(xx));
            return(success = 1);
        }
      default:
        return(-2);
    }
}
#endif /* CKCHANNELIO */

#ifndef NOSETKEY
/* Save Key maps and in OS/2 Mouse maps */
int
savkeys(name,disp) char * name; int disp; {
    char *tp;
    static struct filinfo xx;
    int savfil, i, j, k;
    char buf[1024];

    zclose(ZMFILE);

    if (disp) {
        xx.bs = 0; xx.cs = 0; xx.rl = 0; xx.org = 0; xx.cc = 0;
        xx.typ = 0; xx.dsp = XYFZ_A; xx.os_specific = "";
        xx.lblopts = 0;
        savfil = zopeno(ZMFILE,name,NULL,&xx);
    } else savfil = zopeno(ZMFILE,name,NULL,NULL);

    if (savfil) {
#ifdef OS2
        ztime(&tp);
        zsout(ZMFILE, "; Kermit 95 SAVE KEYMAP file: ");
        zsoutl(ZMFILE,tp);
        if (mskkeys) {
            zsoutl(ZMFILE,
         "if eq \"\\v(program)\" \"C-Kermit\" set mskermit keycodes on");
        } else {
            zsoutl(ZMFILE,
         "if NOT eq \"\\v(program)\" \"C-Kermit\" stop 1 C-Kermit required.");
            zsoutl(ZMFILE,"set mskermit keycodes off");
        }
        zsoutl(ZMFILE,"");
#else /* OS2 */
        ztime(&tp);
        zsout(ZMFILE, "; C-Kermit SAVE KEYMAP file: ");
        zsoutl(ZMFILE,tp);
#endif /* OS2 */

        zsoutl(ZMFILE,"; Clear previous keyboard mappings ");
        zsoutl(ZMFILE,"set key clear");
#ifdef OS2
        for (k = 0; k < nttkey; k++) {
            if (!ttkeytab[k].flgs) {
                ckmakmsg(buf,1024,
                         "set terminal key ",
                         ttkeytab[k].kwd,
                         " clear",
                         NULL
                         );
                zsoutl(ZMFILE,buf);
            }
        }
#endif /* OS2 */
        zsoutl(ZMFILE,"");

        for (i = 0; i < KMSIZE; i++) {
            if (macrotab[i]) {
                int len = strlen((char *)macrotab[i]);
#ifdef OS2
                ckmakmsg(buf,
                         1024,
                         "set key \\",
                         ckitoa(mskkeys ? cktomsk(i) : i),
                         " ",
                         NULL
                         );
#else /* OS2 */
                ckmakmsg(buf,
                         1024,
                         "set key \\",
                         ckitoa(i),
                         NULL,NULL
                         );
#endif /* OS2 */
                zsout(ZMFILE,buf);

                for (j = 0; j < len; j++) {
                    char ch = macrotab[i][j];
                    if (ch <= SP || ch >= DEL ||
                         ch == '-' || ch == ',' ||
                         ch == '{' || ch == '}' ||
                         ch == ';' || ch == '?' ||
                         ch == '.' || ch == '\'' ||
                         ch == '\\' || ch == '/' ||
                         ch == '#') {
                        ckmakmsg(buf,1024,"\\{",ckitoa((int)ch),"}",NULL);
                        zsout(ZMFILE,buf);
                    } else {
                        ckmakmsg(buf,1024,ckctoa((char)ch),NULL,NULL,NULL);
                        zsout(ZMFILE,buf);
                    }
                }
#ifdef OS2
                ckmakmsg(buf,1024,"\t; ",keyname(i),NULL,NULL);
                zsoutl(ZMFILE,buf);
#else
                zsoutl(ZMFILE,"");
#endif /* OS2 */
            } else if ( keymap[i] != i ) {
#ifndef NOKVERBS
                if (IS_KVERB(keymap[i])) {
                    for (j = 0; j < nkverbs; j++)
                      if (kverbs[j].kwval == (keymap[i] & ~F_KVERB))
                        break;
                    if (j != nkverbs) {
#ifdef OS2
#ifdef COMMENT
                        sprintf(buf, "set key \\%d \\K%s\t; %s",
                                mskkeys ? cktomsk(i) : i,
                                kverbs[j].kwd, keyname(i)
                                );
#else
                        ckmakxmsg(buf,  /* 12 string args */
                                  1024,
                                  "set key \\",
                                  ckitoa(mskkeys ? cktomsk(i) : i),
                                  " \\K",
                                  kverbs[j].kwd,
                                  "\t; ",
                                  keyname(i),
                                  NULL, NULL, NULL, NULL, NULL, NULL);
#endif /* COMMENT */
                        zsoutl(ZMFILE,buf);
#else
#ifdef COMMENT
                        sprintf(buf, "set key \\%d \\K%s", i, kverbs[j].kwd);
#else
                        ckmakmsg(buf,1024,
                                 "set key \\",
                                 ckitoa(i),
                                 " \\K",
                                 kverbs[j].kwd
                                 );
#endif /* COMMENT */
                        zsoutl(ZMFILE,buf);
#endif
                    }
                } else
#endif /* NOKVERBS */
                  {
#ifdef OS2
#ifdef COMMENT
                      sprintf(buf, "set key \\%d \\{%d}\t; %s",
                              mskkeys ? cktomsk(i) : i,
                              keymap[i],
                              keyname(i)
                              );
#else
                      ckmakxmsg(buf,    /* 8 string args */
                                1024,
                                "set key \\",
                                ckitoa(mskkeys ? cktomsk(i) : i),
                                " \\{",
                                ckitoa(keymap[i]),
                                "}\t; ",
                                keyname(i),
                                NULL,NULL,NULL,NULL,NULL,NULL);
#endif /* COMMENT */
                      zsoutl(ZMFILE,buf);
#else
#ifdef COMMENT
                      sprintf(buf, "set key \\%d \\{%d}", i, keymap[i]);
#else
                      ckmakxmsg(buf,1024,
                               "set key \\",
                               ckitoa(i),
                               " \\{",
                               ckitoa(keymap[i]),
                               "}",
                               NULL,NULL,NULL,NULL,NULL,NULL,NULL);
#endif /* COMMENT */
                      zsoutl(ZMFILE,buf);
#endif /* OS2 */
                  }
            }
        }
#ifdef OS2
        /* OS/2 also has the SET TERMINAL KEY <termtype> defines */
        for (k = 0; k < nttkey; k++) {
            extern struct keynode * ttkeymap[];
            struct keynode * pnode = NULL;

            if (ttkeytab[k].flgs)       /* Don't process CM_INV or CM_ABR */
              continue;

            zsoutl(ZMFILE,"");
            ckmakmsg(buf,1024,"; SET TERMINAL KEY ",ttkeytab[k].kwd,NULL,NULL);
            zsoutl(ZMFILE,buf);

            for (pnode = ttkeymap[ttkeytab[k].kwval];
                 pnode;
                 pnode = pnode->next
                 ) {
                switch (pnode->def.type) {
                  case key:
#ifdef COMMENT
                    sprintf(buf, "set terminal key %s \\%d \\{%d}\t; %s",
                            ttkeytab[k].kwd,
                            mskkeys ? cktomsk(pnode->key) : pnode->key,
                            pnode->def.key.scancode,
                            keyname(pnode->key)
                            );
#else
                    ckmakxmsg(buf,
                              1024,
                              "set terminal key ",
                              ttkeytab[k].kwd,
                              " \\",
                              ckitoa(mskkeys ?
                                     cktomsk(pnode->key) :
                                     pnode->key),
                              " \\{",
                              ckitoa(pnode->def.key.scancode),
                              "}\t; ",
                              keyname(pnode->key),
                              NULL,NULL,NULL,NULL
                              );
#endif /* COMMENT */
                    zsoutl(ZMFILE,buf);
                    break;
                  case kverb:
                    for (j = 0; j < nkverbs; j++)
                      if (kverbs[j].kwval == (pnode->def.kverb.id & ~F_KVERB))
                        break;
                    if (j != nkverbs) {
#ifdef COMMENT
                        sprintf(buf, "set terminal key %s \\%d \\K%s\t; %s",
                                ttkeytab[k].kwd,
                                mskkeys ? cktomsk(pnode->key) : pnode->key,
                                kverbs[j].kwd, keyname(pnode->key)
                                );
#else
                        ckmakxmsg(buf,
                                  1024,
                                  "set terminal key ",
                                  ttkeytab[k].kwd,
                                  " \\",
                                  ckitoa(mskkeys ?
                                         cktomsk(pnode->key) :
                                         pnode->key),
                                  " \\K",
                                  kverbs[j].kwd,
                                  "\t; ",
                                  keyname(pnode->key),
                                  NULL,NULL,NULL,NULL
                                  );
#endif /* COMMENT */
                        zsoutl(ZMFILE,buf);
                    }
                    break;
                  case macro: {
                      int len = strlen((char *)pnode->def.macro.string);
#ifdef COMMENT
                      sprintf(buf,"set terminal key %s \\%d ",
                              ttkeytab[k].kwd,
                              mskkeys ? cktomsk(pnode->key) : pnode->key);
#else
                      ckmakxmsg(buf,
                               1024,
                               "set terminal key ",
                               ttkeytab[k].kwd,
                               " \\",
                               ckitoa(mskkeys ?
                                      cktomsk(pnode->key) :
                                      pnode->key),
                               " ",
                               NULL,NULL,NULL,NULL,NULL,NULL,NULL
                              );
#endif /* COMMENT */
                      zsout(ZMFILE,buf);

                      for (j = 0; j < len; j++) {
                          char ch = pnode->def.macro.string[j];
                          if (ch <= SP || ch >= DEL ||
                               ch == '-' || ch == ',' ||
                               ch == '{' || ch == '}' ||
                               ch == ';' || ch == '?' ||
                               ch == '.' || ch == '\'' ||
                               ch == '\\' || ch == '/' ||
                               ch == '#') {
                              ckmakmsg(buf,1024,
                                       "\\{",ckitoa((int)ch),"}",NULL);
                              zsout(ZMFILE,buf);
                          } else {
                              ckmakmsg(buf,1024,
                                       ckctoa((char)ch),NULL,NULL,NULL);
                              zsout(ZMFILE,buf);
                          }
                      }
                      ckmakmsg(buf,1024,"\t; ",keyname(pnode->key),NULL,NULL);
                      zsoutl(ZMFILE,buf);
                      break;
                  }
                  case literal: {
                      int len = strlen((char *)pnode->def.literal.string);
#ifdef COMMENT
                      sprintf(buf,"set terminal key %s /literal \\%d ",
                              ttkeytab[k].kwd,
                              mskkeys ? cktomsk(pnode->key) : pnode->key);
#else
                      ckmakxmsg(buf,
                               1024,
                               "set terminal key ",
                               ttkeytab[k].kwd,
                               " /literal \\",
                               ckitoa(mskkeys ?
                                      cktomsk(pnode->key) :
                                      pnode->key),
                               " ",
                               NULL,NULL,NULL,NULL,NULL,NULL,NULL);
#endif /* COMMENT */
                      zsout(ZMFILE,buf);

                      for (j = 0; j < len; j++) {
                          char ch = pnode->def.literal.string[j];
                          if (ch <= SP || ch >= DEL ||
                               ch == '-' || ch == ',' ||
                               ch == '{' || ch == '}' ||
                               ch == ';' || ch == '?' ||
                               ch == '.' || ch == '\'' ||
                               ch == '\\' || ch == '/' ||
                               ch == '#') {
                              ckmakmsg(buf,1024,
                                       "\\{",ckitoa((int)ch),"}",NULL);
                              zsout(ZMFILE,buf);
                          } else {
                              ckmakmsg(buf,1024,
                                       ckctoa((char)ch),NULL,NULL,NULL);
                              zsout(ZMFILE,buf);
                          }
                      }
                      ckmakmsg(buf,1024,"\t; ",keyname(pnode->key),NULL,NULL);
                      zsoutl(ZMFILE,buf);
                      break;
                  }
                  case esc:
#ifdef COMMENT
                    sprintf(buf,
                       "set terminal key %s /literal \\%d \\{%d}\\{%d}\t; %s",
                            ttkeytab[k].kwd,
                            mskkeys ? cktomsk(pnode->key) : pnode->key,
                            ISDG200(ttkeytab[k].kwval) ? 30 : 27,
                            pnode->def.esc.key & ~F_ESC,
                            keyname(pnode->key)
                            );
#else
                    ckmakxmsg(buf,
                              1024,
                              "set terminal key ",
                              ttkeytab[k].kwd,
                              " /literal \\",
                              ckitoa(mskkeys ?
                                     cktomsk(pnode->key) :
                                     pnode->key),
                              " \\{",
                              ckitoa(ISDG200(ttkeytab[k].kwval) ? 30 : 27),
                              "}\\{",
                              ckitoa(pnode->def.esc.key & ~F_ESC),
                              "}\t; ",
                              keyname(pnode->key),
                              NULL,NULL
                              );
#endif /* COMMENT */
                    zsoutl(ZMFILE,buf);
                    break;
                  case csi:
#ifdef COMMENT
                    sprintf(buf,
                       "set terminal key %s /literal \\%d \\{27}[\\{%d}\t; %s",
                            ttkeytab[k].kwd,
                            mskkeys ? cktomsk(pnode->key) : pnode->key,
                            pnode->def.csi.key & ~F_CSI,
                            keyname(pnode->key)
                            );
#else
                    ckmakxmsg(buf,
                              1024,
                              "set terminal key ",
                              ttkeytab[k].kwd,
                              " /literal \\",
                              ckitoa(mskkeys ?
                                     cktomsk(pnode->key) :
                                     pnode->key),
                              " \\{27}[\\{",
                              ckitoa(pnode->def.csi.key & ~F_CSI),
                              "}\t; ",
                              keyname(pnode->key),
                              NULL,NULL,NULL,NULL
                              );
#endif /* COMMENT */
                    zsoutl(ZMFILE,buf);
                    break;
                  default:
                    continue;
                }
            }
        }
#endif /* OS2 */

        zsoutl(ZMFILE,"");
        zsoutl(ZMFILE,"; End");
        zclose(ZMFILE);
        return(success = 1);
    } else {
        return(success = 0);
    }
}
#endif /* NOSETKEY */

#define SV_SCRL 0
#define SV_HIST 1

#ifdef OS2
#ifndef NOLOCAL
static struct keytab trmtrmopt[] = {
    { "scrollback", SV_SCRL, 0 }
};
#endif /* NOLOCAL */
#endif /* OS2 */

static struct keytab cmdtrmopt[] = {
#ifdef CK_RECALL
    { "history",    SV_HIST, 0 },
#endif /* CK_RECALL */
#ifdef OS2
#ifndef NOLOCAL
    { "scrollback", SV_SCRL, 0 },
#endif /* NOLOCAL */
#endif /* OS2 */
    { "", 0, 0 }
};
static int ncmdtrmopt = (sizeof (cmdtrmopt) / sizeof (struct keytab)) - 1;

#ifdef OS2
#ifndef NOLOCAL
_PROTOTYP(int savscrbk, (int, char *, int));
#endif /* NOLOCAL */
#endif /* OS2 */

#ifdef CK_RECALL
_PROTOTYP(int savhistory, (char *, int));
#endif /* CK_RECALL */

int
dosave(xx) int xx; {
    int x, y = 0, disp;
    char * s = NULL;
    extern struct keytab disptb[];
#ifdef ZFNQFP
    struct zfnfp * fnp;
#endif /* ZFNQFP */

#ifndef NOSETKEY
    if (xx == XSKEY) {                  /* SAVE KEYMAP.. */
        z = cmofi("Name of Kermit command file","keymap.ksc",&s,xxstring);
    } else {
#endif /* NOSETKEY */
        switch (xx) {
          case XSCMD:                   /* SAVE COMMAND.. */
            if ((y = cmkey(cmdtrmopt, ncmdtrmopt, "What to save",
#ifdef OS2
                           "scrollback",
#else
                           "history",
#endif /* OS2 */
                           xxstring)) < 0)
              return(y);
            break;
#ifdef OS2
#ifndef NOLOCAL
          case XSTERM:                  /* SAVE TERMINAL.. */
            if ((y = cmkey(trmtrmopt,1,
                           "What to save","scrollback",xxstring)) < 0)
              return(y);
            break;
#endif /* NOLOCAL */
#endif /* OS2 */
        }
        z = cmofi("Filename",
                  ((y == SV_SCRL) ? "scrollbk.txt" : "history.txt"),
                  &s,
                  xxstring
                  );
#ifndef NOSETKEY
    }
#endif /* NOSETKEY */
    if (z < 0)                          /* Check output-file parse results */
      return(z);
    if (z == 2) {
        printf("?Sorry, %s is a directory name\n",s);
        return(-9);
    }
#ifdef ZFNQFP
    if ((fnp = zfnqfp(s,TMPBUFSIZ - 1,tmpbuf))) {/* Convert to full pathname */
        if (fnp->fpath)
          if ((int) strlen(fnp->fpath) > 0)
            s = fnp->fpath;
    }
#endif /* ZFNQFP */

    ckstrncpy(line,s,LINBUFSIZ);        /* Make safe copy of pathname */
    s = line;
#ifdef MAC
    z = 0;
#else
    /* Get NEW/APPEND disposition */
    if ((z = cmkey(disptb,2,"Disposition","new",xxstring)) < 0)
      return(z);
#endif /* MAC */
    disp = z;
    if ((x = cmcfm()) < 0)              /* Get confirmation */
      return(x);

    switch (xx) {                       /* Do action.. */
#ifndef NOSETKEY
      case XSKEY:                       /* SAVE KEYMAP */
        return (savkeys(s,disp));
#endif /* NOSETKEY */

      case XSCMD:                       /* SAVE COMMAND.. */
#ifdef OS2
#ifndef NOLOCAL
        if (y == SV_SCRL)               /* .. SCROLLBACK */
          return(success = savscrbk(VCMD,s,disp));
#endif /* NOLOCAL */
#endif /* OS2 */
#ifndef NORECALL
        if (y == SV_HIST)               /* .. HISTORY */
          return(success = savhistory(s,disp));
#endif /* NORECALL */
        break;

#ifdef OS2
#ifndef NOLOCAL
      case XSTERM:                      /* SAVE TERMINAL SCROLLBACK */
        return(success = savscrbk(VTERM,s,disp));
#endif /* NOLOCAL */
#endif /* OS2 */
    }
    success = 0;
    return(-2);
}

/*
  R E A D T E X T

  Read text with a custom prompt into given buffer using command parser but
  with no echoing or entry into recall buffer.
*/
int
readtext(prmpt, buffer, bufsiz) char * prmpt; char * buffer; int bufsiz; {
#ifdef CK_RECALL
    extern int on_recall;               /* Around Password prompting */
#endif /* CK_RECALL */
    int rc;
#ifndef NOLOCAL
#ifdef OS2
    extern int vmode;
    extern int startflags;
    int vmode_sav = vmode;

    if (!prmpt) prmpt = "";

    if (win95_popup && !(startflags & 96)
#ifdef IKSD
         && !inserver
#endif /* IKSD */
         )
      return(popup_readtext(vmode,NULL,prmpt,buffer,bufsiz,0));

    if (vmode == VTERM) {
        vmode = VCMD;
        VscrnIsDirty(VTERM);
        VscrnIsDirty(VCMD);
    }
#endif /* OS2 */
#endif /* NOLOCAL */

#ifdef CK_RECALL
    on_recall = 0;
#endif /* CK_RECALL */
    cmsavp(psave,PROMPTL);              /* Save old prompt */
    cmsetp(prmpt);                      /* Make new prompt */
    concb((char)escape);                /* Put console in cbreak mode */
    cmini(1);                           /* and echo mode */
    if (pflag) prompt(xxstring);        /* Issue prompt if at top level */
    cmres();                            /* Reset the parser */
    for (rc = -1; rc < 0; ) {           /* Prompt till they answer */
        rc = cmtxt("","",&s,NULL);      /* Get a literal line of text */
        cmres();                        /* Reset the parser again */
    }
    ckstrncpy(buffer,s,bufsiz);
    cmsetp(psave);                      /* Restore original prompt */

#ifndef NOLOCAL
#ifdef OS2
    if (vmode != vmode_sav) {
        vmode = VTERM;
        VscrnIsDirty(VCMD);
        VscrnIsDirty(VTERM);
    }
#endif /* OS2 */
#endif /* NOLOCAL */
    return(0);
}
#endif /* NOICP */

/* A general function to allow a Password or other information  */
/* to be read from the command prompt without it going into     */
/* the recall buffer or being echo'd.                           */

int
readpass(prmpt, buffer, bufsiz) char * prmpt; char * buffer; int bufsiz; {
    int x;
#ifdef NOICP
    if (!prmpt) prmpt = "";
    printf("%s", prmpt);
#ifdef COMMENT
    /* Some linkers won't allow this because it's unsafe */
    gets(buffer);
#else  /* COMMENT */
    {
        int c, i; char * p;
        p = buffer;
        for (i = 0; i < bufsiz-1; i++) {
            if ((c = getchar()) == EOF)
              break;
            if (c < SP)
              break;
            buffer[i] = c;
        }
        buffer[i] = NUL;
    }
#endif /* COMMENT */
    return(1);
#else  /* NOICP */
#ifdef CK_RECALL
    extern int on_recall;               /* around Password prompting */
#endif /* CK_RECALL */
    int rc;
#ifndef NOLOCAL
#ifdef OS2
    extern int vmode;
    extern int startflags;
    int vmode_sav = vmode;
#endif /* OS2 */
#endif /* NOLOCAL */
#ifdef CKSYSLOG
    int savlog;
#endif /* CKSYSLOG */
    if (!prmpt) prmpt = "";
#ifndef NOLOCAL
    debok = 0;                          /* Don't log */
#ifdef OS2
    if (win95_popup && !(startflags & 96)
#ifdef IKSD
         && !inserver
#endif /* IKSD */
         ) {
        x = popup_readpass(vmode,NULL,prmpt,buffer,bufsiz,0);
        debok = 1;
        return(x);
    }
#endif /* OS2 */
#endif /* NOLOCAL */

#ifdef CKSYSLOG
    savlog = ckxsyslog;                 /* Save and turn off syslogging */
    ckxsyslog = 0;
#endif /* CKSYSLOG */
#ifndef NOLOCAL
#ifdef OS2
    if (vmode == VTERM) {
        vmode = VCMD;
        VscrnIsDirty(VTERM);
        VscrnIsDirty(VCMD);
    }
#endif /* OS2 */
#endif /* NOLOCAL */
#ifdef CK_RECALL
    on_recall = 0;
#endif /* CK_RECALL */
    cmsavp(psave,PROMPTL);              /* Save old prompt */
    cmsetp(prmpt);                      /* Make new prompt */
    concb((char)escape);                /* Put console in cbreak mode */
    cmini(0);                           /* and no-echo mode */
    if (pflag) prompt(xxstring);        /* Issue prompt if at top level */
    cmres();                            /* Reset the parser */
    for (rc = -1; rc < 0; ) {           /* Prompt till they answer */
        rc = cmtxt("","",&s,NULL);      /* Get a literal line of text */
        cmres();                        /* Reset the parser again */
    }
    ckstrncpy(buffer,s,bufsiz);
    printf("\r\n");                     /* Echo a CRLF */
    cmsetp(psave);                      /* Restore original prompt */
    cmini(1);                           /* Restore echo mode */
#ifndef NOLOCAL
#ifdef OS2
    if (vmode != vmode_sav) {
        vmode = VTERM;
        VscrnIsDirty(VCMD);
        VscrnIsDirty(VTERM);
    }
#endif /* OS2 */
#endif /* NOLOCAL */
#ifdef CKSYSLOG
    ckxsyslog = savlog;                 /* Restore syslogging */
#endif /* CKSYSLOG */
    debok = 1;
    return(0);
#endif /* NOICP */
}

#ifndef NOICP
struct keytab authtab[] = {             /* Available authentication types */
#ifdef CK_KERBEROS
    { "k4",        AUTH_KRB4, CM_INV },
    { "k5",        AUTH_KRB5, CM_INV },
    { "kerberos4", AUTH_KRB4, 0      },
    { "kerberos5", AUTH_KRB5, 0      },
    { "krb4",      AUTH_KRB4, CM_INV },
    { "krb5",      AUTH_KRB5, CM_INV },
#endif /* CK_KERBEROS */
#ifdef NT
    { "ntlm",      AUTH_NTLM, 0 },
#endif /* NT */
#ifdef CK_SRP
    { "srp",       AUTH_SRP,  0 },
#endif /* CK_SRP */
#ifdef CK_SSL
    { "ssl",       AUTH_SSL,  0 },
#endif /* CK_SSL */
    { "",          0,         0 }
};
int authtabn = sizeof(authtab)/sizeof(struct keytab)-1;

#ifdef CK_KERBEROS
struct keytab kerbtab[] = {             /* Kerberos authentication types */
    { "k4",        AUTH_KRB4, CM_INV },
    { "k5",        AUTH_KRB5, CM_INV },
    { "kerberos4", AUTH_KRB4, 0      },
    { "kerberos5", AUTH_KRB5, 0      },
    { "krb4",      AUTH_KRB4, CM_INV },
    { "krb5",      AUTH_KRB5, CM_INV }
};
int kerbtabn = sizeof(kerbtab)/sizeof(struct keytab);

static struct keytab krb_s_tbl[] = {    /* AUTHENTICATE command switches: */
    { "/cache",   KRB_S_CA, CM_ARG }
};
static int krb_s_n = sizeof(krb_s_tbl)/sizeof(struct keytab);

static struct keytab krb_v_tbl[] = {    /* KERBEROS version values: */
    { "4",    4, 0 },
    { "5",    5, 0 },                   /* (add others as needed...) */
    { "auto", 0, 0 }                    /* Note: 0 = auto */
};
static int krb_v_n = sizeof(krb_v_tbl)/sizeof(struct keytab);

static struct keytab krb_a_tbl[] = {    /* KERBEROS actions: */
    { "destroy",           KRB_A_DE, 0 },
    { "initialize",        KRB_A_IN, 0 },
    { "list-credentials",  KRB_A_LC, 0 }
};
static int krb_a_n = sizeof(krb_a_tbl)/sizeof(struct keytab);

static struct keytab krb4_i_tbl[] = {   /* KERBEROS 4 INITIALIZE switches: */
    { "/brief",            KRB_I_BR, 0 },      /* /BRIEF       */
    { "/instance",         KRB_I_IN, CM_ARG }, /* /INSTANCE:   */
    { "/lifetime",         KRB_I_LF, CM_ARG }, /* /LIFETIME:   */
    { "/not-preauth",      KRB_I_NPA, 0 },     /* /NOT-PREAUTH */
    { "/password",         KRB_I_PW, CM_ARG }, /* /PASSWORD:   */
#ifdef OS2
    { "/popup",            KRB_I_POP, 0 },     /* /POPUP       */
#endif /* OS2 */
    { "/preauth",          KRB_I_PA, 0 },      /* /PREAUTH     */
    { "/realm",            KRB_I_RL, CM_ARG }, /* /REALM:      */
    { "/verbose",          KRB_I_VB, 0 },      /* /VERBOSE     */
    { "", 0, 0 }
};
static int krb4_i_n = sizeof(krb4_i_tbl)/sizeof(struct keytab) - 1;

static struct keytab krb5_i_tbl[] = {   /* KERBEROS 5 INITIALIZE switches: */
    { "/addresses",        KRB_I_ADR, CM_ARG },
    { "/forwardable",      KRB_I_FW,  0 },         /* /FORWARDABLE */
    { "/instance",         KRB_I_IN, CM_ARG }, /* /INSTANCE:   */
    { "/k4",               KRB_I_K4,  CM_INV }, /* /KERBEROS4   */
    { "/kerberos4",        KRB_I_K4,  0 },      /* /KERBEROS4   */
    { "/krb4",             KRB_I_K4,  CM_INV }, /* /KERBEROS4   */
    { "/lifetime",         KRB_I_LF,  CM_ARG }, /* /LIFETIME:   */
    { "/no-addresses",     KRB_I_NAD, 0 },      /* /NO-ADDRESSES */
    { "/no-k4",            KRB_I_NK4, CM_INV },/* /NO-KERBEROS4 */
    { "/no-kerberos4",     KRB_I_NK4, 0 },     /* /NO-KERBEROS4 */
    { "/no-krb4",          KRB_I_NK4, CM_INV },/* /NO-KERBEROS4 */
    { "/not-forwardable",  KRB_I_NFW, 0 },         /* /NOT-FORWARDABLE */
    { "/not-proxiable",    KRB_I_NPR, 0 },     /* /NOT-PROXIABLE   */
    { "/password",         KRB_I_PW,  CM_ARG }, /* /PASSWORD:   */
#ifdef OS2
    { "/popup",            KRB_I_POP, 0 },     /* /POPUP       */
#endif /* OS2 */
    { "/postdate",         KRB_I_PD, CM_ARG }, /* /POSTDATE:   */
    { "/pr",               KRB_I_PR, CM_INV|CM_ABR }, /* to allow for */
    { "/pro",              KRB_I_PR, CM_INV|CM_ABR }, /* different spellings */
    { "/prox",             KRB_I_PR, CM_INV|CM_ABR },
    { "/proxiable",        KRB_I_PR, 0 },      /* /PROXIABLE   */
    { "/proxyable",        KRB_I_PR, CM_INV }, /* /PROXYABLE   */
    { "/realm",            KRB_I_RL, CM_ARG }, /* /REALM:      */
    { "/renew",            KRB_I_RN, 0 },          /* /RENEW       */
    { "/renewable",        KRB_I_RB, CM_ARG }, /* /RENEWABLE:  */
    { "/service",          KRB_I_SR, CM_ARG }, /* /SERVICE:    */
    { "/validate",         KRB_I_VA, 0 },          /* /VALIDATE    */
    { "", 0, 0 }
};
static int krb5_i_n = sizeof(krb5_i_tbl)/sizeof(struct keytab) - 1;

static struct keytab klctab[] = {       /* List Credentials switches*/
    { "/addresses",  XYKLCAD, 0 },
    { "/encryption", XYKLCEN, 0 },
    { "/flags",      XYKLCFL, 0 }
};
static int nklctab = sizeof(klctab)/sizeof(struct keytab);

extern int krb_action;
extern struct krb_op_data krb_op;

extern struct krb5_list_cred_data krb5_lc;
extern struct krb5_init_data krb5_init;
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
extern int    krb5_d_no_addresses;
extern int    krb5_checkaddrs;
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
extern int    krb4_checkaddrs;
extern char * k4_keytab;                /* Keytab file */
#endif /* CK_KERBEROS */

#ifndef NOSHOW
int
sho_iks() {
#ifdef IKSDCONF
#ifdef CK_LOGIN
    extern int ckxsyslog, ckxwtmp, ckxanon;
#ifdef UNIX
    extern int ckxpriv;
#endif /* UNIX */
#ifdef CK_PERMS
    extern int ckxperms;
#endif /* CK_PERMS */
    extern char * anonfile, * userfile, * anonroot;
#ifdef OS2
    extern char * anonacct;
#endif /* OS2 */
#ifdef NT
    extern char * iks_domain;
#endif /* NT */
#endif /* CK_LOGIN */
#ifdef CKWTMP
    extern char * wtmpfile;
#endif /* CKWTMP */
#ifdef IKSDB
    extern char * dbfile;
    extern int dbenabled;
#endif /* IKSDB */
#ifdef CK_LOGIN
    extern int logintimo;
#endif /* CK_LOGIN */
    extern int srvcdmsg, success, iksdcf, noinit, arg_x;
    extern char * cdmsgfile[], * cdmsgstr, *kermrc;
    char * bannerfile = NULL;
    char * helpfile = NULL;
    extern int xferlog;
    extern char * xferfile;
    int i;

    if (isguest) {
        printf("?Command disabled\r\n");
        return(success = 0);
    }

    printf("IKS Settings\r\n");
#ifdef CK_LOGIN
#ifdef OS2
    printf("  Anonymous Account:   %s\r\n",anonacct?anonacct:"<none>");
#endif /* OS2 */
    printf("  Anonymous Initfile:  %s\r\n",anonfile?anonfile:"<none>");
    printf("  Anonymous Login:     %d\r\n",ckxanon);
    printf("  Anonymous Root:      %s\r\n",anonroot?anonroot:"<none>");
#endif /* CK_LOGIN */
    printf("  Bannerfile:          %s\r\n",bannerfile?bannerfile:"<none>");
    printf("  CDfile:              %s\r\n",cdmsgfile[0]?cdmsgfile[0]:"<none>");
    for ( i=1;i<16 && cdmsgfile[i];i++ )
        printf("  CDfile:              %s\r\n",cdmsgfile[i]);
    printf("  CDMessage:           %d\r\n",srvcdmsg);
#ifdef IKSDB
    printf("  DBfile:              %s\r\n",dbfile?dbfile:"<none>");
    printf("  DBenabled:           %d\r\n",dbenabled);
#endif /* IKSDB */
#ifdef CK_LOGIN
#ifdef NT
    printf("  Default-domain:      %s\r\n",iks_domain?iks_domain:".");
#endif /* NT */
#endif /* CK_LOGIN */
    printf("  Helpfile:            %s\r\n",helpfile?helpfile:"<none>");
    printf("  Initfile:            %s\r\n",kermrc?kermrc:"<none>");
    printf("  No-Initfile:         %d\r\n",noinit);
#ifdef CK_LOGIN
#ifdef CK_PERM
    printf("  Permission code:     %0d\r\n",ckxperms);
#endif /* CK_PERM */
#ifdef UNIX
    printf("  Privileged Login:    %d\r\n",ckxpriv);
#endif /* UNIX */
#endif /* CK_LOGIN */
    printf("  Server-only:         %d\r\n",arg_x);
    printf("  Syslog:              %d\r\n",ckxsyslog);
    printf("  Timeout (seconds):   %d\r\n",logintimo);
    printf("  Userfile:            %s\r\n",userfile?userfile:"<none>");
#ifdef CK_LOGIN
#ifdef CKWTMP
    printf("  Wtmplog:             %d\r\n",ckxwtmp);
    printf("  Wtmpfile:            %s\r\n",wtmpfile?wtmpfile:"<none>");
#endif /* CKWTMP */
#endif /* CK_LOGIN */
    printf("  Xferfile:            %s\r\n",xferfile?xferfile:"<none>");
    printf("  Xferlog:             %d\r\n",xferlog);
#else /* IKSDCONF */
    printf("?Nothing to show.\r\n");
#endif /* IKSDCONF */
    return(success = 1);
}

#ifdef CK_AUTHENTICATION
int
sho_auth(cx) int cx; {
    extern int auth_type_user[], cmd_rows;
    int i;
    char * p;
    int kv = 0, all = 0, n = 0;

#ifdef IKSD
    if (inserver && isguest) {
        printf("?Sorry, command disabled.\r\n");
        return(success = 0);
    }
#endif /* IKSD */
    if (cx) {
        kv = cx;
    } else if (auth_type_user[0] != AUTHTYPE_AUTO) {
        kv = auth_type_user[0];
    } else {
        all = 1;
        kv = AUTHTYPE_KERBEROS_V4;
    }
    while (kv) {
        switch (kv) {
          case AUTHTYPE_KERBEROS_V4:
            kv = all ? AUTHTYPE_KERBEROS_V5 : 0;
            if (ck_krb4_is_installed()) {
                printf(" Authentication:      Kerberos 4\n");
                if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            } else {
                printf(" Authentication:      Kerberos 4 (not installed)\n");
                if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
                continue;
            }
#ifdef CK_KERBEROS
            printf(" Keytab file:         %s\n",
                      k4_keytab ? k4_keytab : "(none)");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            if (krb_action < 0) {
                p = "(none)";
            } else {
                for (p = "", i = 0; i < krb_a_n; i++) {
                    if (krb_action == krb_a_tbl[i].kwval) {
                        p = krb_a_tbl[i].kwd;
                        break;
                    }
                }
            }
            printf(" Action:              %s\n", p);
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Default lifetime     %d\n",krb4_d_lifetime);
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Lifetime:            %d (minutes)\n",krb4_init.lifetime);
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Default preauth:     %d\n",krb4_d_preauth);
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Preauth:             %d\n",krb4_init.preauth);
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Default principal:   \"%s\"\n",
                    krb4_d_principal ? krb4_d_principal : "");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Principal:           \"%s\"\n",
                    krb4_init.principal ? krb4_init.principal : "");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Default realm:       \"%s\"\n",
                    krb4_d_realm ? krb4_d_realm : "");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Realm:               \"%s\"\n",
                    krb4_init.realm ? krb4_init.realm : "");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Default instance:    \"%s\"\n",
                    krb4_d_instance ? krb4_d_instance : "");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Instance:            \"%s\"\n",
                    krb4_init.instance ? krb4_init.instance : "");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Auto-Get TGTs:       %d\n",krb4_autoget);
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Auto-Destroy TGTs:   %s\n",
                    krb4_autodel==KRB_DEL_NO?"never":
                    krb4_autodel==KRB_DEL_CL?"on-close":"on-exit");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Check IP Addresses:  %d\n",krb4_checkaddrs);
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
#ifdef COMMENT
            printf(" Password:    \"%s\"\n",
                    krb4_init.password  ? krb4_init.password  : "");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
#endif /* COMMENT */
#endif /* CK_KERBEROS */
            printf("\n");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            break;
        case AUTHTYPE_KERBEROS_V5:
            kv = all ? AUTHTYPE_SSL : 0;
            if (ck_krb5_is_installed()) {
                if (ck_gssapi_is_installed())
                    printf(" Authentication:      Kerberos 5 plus GSSAPI\n");
                else
                    printf(" Authentication:      Kerberos 5\n");
                if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            } else {
                printf(" Authentication:      Kerberos 5 (not installed)\n");
                if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
                continue;
            }

#ifdef CK_KERBEROS
            printf(" Cache file:          %s\n",
                    krb_op.cache ? krb_op.cache : "(none)");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Default cache:       %s\n",
                    krb5_d_cc ? krb5_d_cc : "(none)");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Keytab file:         %s\n",
                      k5_keytab ? k5_keytab : "(none)");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            if (krb_action < 0) {
                p = "(none)";
            } else  {
                for (p = "", i = 0; i < krb_a_n; i++) {
                    if (krb_action == krb_a_tbl[i].kwval) {
                        p = krb_a_tbl[i].kwd;
                        break;
                    }
                }
            }
            printf(" Action:              %s\n", p);
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;

            printf(" Default forwardable  %d\n",krb5_d_forwardable);
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Forwardable:         %d\n",krb5_init.forwardable);
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Default lifetime     %d\n",krb5_d_lifetime);
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Lifetime:            %d (minutes)\n",krb5_init.lifetime);
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Postdate:            \"%s\"\n",
                    krb5_init.postdate ? krb5_init.postdate: "");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Default proxiable:   %d\n",krb5_d_proxiable);
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Proxiable:           %d\n",krb5_init.proxiable);
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Renew:               %d\n",krb5_init.renew);
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Default renewable:   %d (minutes)\n",krb5_d_renewable);
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Renewable:           %d (minutes)\n",krb5_init.renewable);
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Service:             \"%s\"\n",
                    krb5_init.service ? krb5_init.service : "");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Validate:            %d\n",krb5_init.validate);
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Default principal:   \"%s\"\n",
                    krb5_d_principal ? krb5_d_principal : "");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Principal:           \"%s\"\n",
                    krb5_init.principal ? krb5_init.principal : "");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Default instance:    \"%s\"\n",
                    krb5_d_instance ? krb5_d_instance : "");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Default realm:       \"%s\"\n",
                    krb5_d_realm ? krb5_d_realm : "");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Realm:               \"%s\"\n",
                    krb5_init.realm ? krb5_init.realm : "");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Auto-Get TGTs:       %d\n",krb5_autoget);
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Auto-Destroy TGTs:   %s\n",
                    krb5_autodel==KRB_DEL_NO?"never":
                    krb5_autodel==KRB_DEL_CL?"on-close":"on-exit");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Default get K4 TGTs: %d\n",krb5_d_getk4);
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Get K4 TGTs: %d\n",krb5_init.getk4);
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Check IP Addresses:  %d\n",krb5_checkaddrs);
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" No IP Addresses:  %d\n",krb5_d_no_addresses);
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" IP-Addresses:        ");
            if (krb5_init.addrs && krb5_init.addrs[0]) {
                for (i = 0; krb5_init.addrs[i]; i++) {
                    if (i)
                      printf(",");
                    printf("%s",krb5_init.addrs[i]);
                }
            } else if (krb5_d_addrs[0]) {
                for (i = 0;i < KRB5_NUM_OF_ADDRS && krb5_d_addrs[i];i++) {
                    if (i)
                      printf(",");
                    printf("%s",krb5_d_addrs[i]);
                }
            } else {
                printf("(use default)");
            }
            printf("\n");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
#ifdef COMMENT
            printf(" Password:            \"%s\"\n",
                    krb5_init.password  ? krb5_init.password  : "");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
#endif /* COMMENT */
#endif /* CK_KERBEROS */
            printf("\n");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            break;
#ifdef CK_SSL
          case AUTHTYPE_SSL:
            kv = all ? AUTHTYPE_SRP : 0;
            if (ck_ssleay_is_installed()) {
                printf(" Authentication:      SSL/TLS (%s)\n",
                        SSLeay_version(SSLEAY_VERSION));
                if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            } else {
                printf(" Authentication:      SSL/TLS (not installed)\n");
                if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
                continue;
            }
            printf(" RSA Certs file: %s\n",ssl_rsa_cert_file?
                  ssl_rsa_cert_file:"(none)");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" RSA Certs Chain file: %s\n",ssl_rsa_cert_chain_file?
                  ssl_rsa_cert_chain_file:"(none)");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" RSA Key file: %s\n",ssl_rsa_key_file?
                  ssl_rsa_key_file:"(none)");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" DSA Certs file: %s\n",ssl_dsa_cert_file?
                  ssl_dsa_cert_file:"(none)");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" DSA Certs Chain file: %s\n",ssl_dsa_cert_chain_file?
                  ssl_dsa_cert_chain_file:"(none)");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" DH Key file: %s\n",ssl_dh_key_file?
                  ssl_dh_key_file:"(none)");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" DH Param file: %s\n",ssl_dh_param_file?
                  ssl_dh_param_file:"(none)");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" CRL file: %s\n",ssl_crl_file?
                  ssl_crl_file:"(none)");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" CRL dir: %s\n",ssl_crl_dir?
                    ssl_crl_dir:"(none)");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Random file: %s\n",ssl_rnd_file?
                  ssl_rnd_file:"(none)");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Verify file: %s\n",ssl_verify_file?
                  ssl_verify_file:"(none)");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Verify dir: %s\n",ssl_verify_dir?
                  ssl_verify_dir:"(none)");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Cipher list: %s\n",ssl_cipher_list ? ssl_cipher_list : 
		    DEFAULT_CIPHER_LIST);
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            if (ssl_con == NULL) {
                SSL_library_init();
                ssl_ctx = (SSL_CTX *)
                  SSL_CTX_new((SSL_METHOD *)TLSv1_method());
                if (ssl_ctx != NULL)
                  ssl_con= (SSL *) SSL_new(ssl_ctx);
            }
            if (ssl_con != NULL) {
                CHAR * p = NULL;
                int i;

                for (i = 0; ; i++) {
                    p = (CHAR *) SSL_get_cipher_list(ssl_con,i);
                    if (p == NULL)
                      break;
                    printf("    %s\n",p);
                    if (++n > cmd_rows - 3)
                        if (!askmore()) return(0); else n = 0;
                }
            }
            printf(" Certs OK? %s\n",ssl_certsok_flag? "yes" : "no");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Debug mode: %s\n", ssl_debug_flag ? "on" : "off");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Verbose mode: %s\n", ssl_verbose_flag ? "on" : "off");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" Verify mode: %s\n",
                    ssl_verify_flag == SSL_VERIFY_NONE ? "none" :
                    ssl_verify_flag == SSL_VERIFY_PEER ? "peer-cert" :
                    "fail-if-no-peer-cert");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" SSL only? %s\n", ssl_only_flag ? "yes" : "no");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            printf(" TLS only? %s\n", tls_only_flag ? "yes" : "no");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
#endif /* CK_SSL */
            break;
          case AUTHTYPE_NTLM:
            kv = 0;
            if (ck_ntlm_is_installed()) {
                printf(" Authentication:      NTLM\n");
                if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
                printf(" No options\n");
                if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            } else {
                printf(" Authentication:      NTLM (not installed)\n");
                if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
                continue;
            }
            printf("\n");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            break;
          case AUTHTYPE_SRP:
            kv = all ? AUTHTYPE_NTLM : 0;
            if (ck_srp_is_installed()) {
                if (ck_krypto_is_installed())
                    printf(" Authentication:      SRP plus Krypto API\n");
                else
                    printf(" Authentication:      SRP\n");
                if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
                printf(" No options\n");
                if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            } else {
                printf(" Authentication:      SRP (not installed)\n");
                if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
                continue;
            }
            printf("\n");
            if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
            break;
        }
    }
    return(success = 1);
}
#endif /* CK_AUTHENTICATION */
#endif /* NOSHOW */

#ifdef CK_KERBEROS

/*  C P _ A U T H  --  AUTHENTICATE command parsing  */

int
cp_auth() {                             /* Command_Parse AUTHENTICATE */
    int c, i, n;                        /* Workers */
    int rc = 0;                         /* Return code */
    int getval;                         /* Parsing helpers */
    int tmpauth = 0;                    /* Temporary authentication type */
    int kv = 0;                         /* Temporary Kerberos version */
    int tmp_action = -1;                /* Temporary Kerberos action */
    int tmp_klc = 0;                    /* Temporary list-credentials */
    char tmphlp[256];                   /* For building help message */
    char * p;
    char * tmppswd  = NULL;             /* Password */
    char * tmpprinz = NULL;             /* Principal */
    char * tmprealm = NULL;             /* Realm */
    char * tmpcache = NULL;             /* Cache file */
    char * tmpinst  = NULL;             /* K4 Instance */
    char * tmpaddrs[KRB5_NUM_OF_ADDRS];
#ifdef CK_RECALL
    extern int on_recall;               /* around Password prompting */
#endif /* CK_RECALL */
    struct stringint pv[KRB_I_MAX+1];   /* Temporary array for switch values */
    struct FDB kw, sw, fl;              /* FDBs for each parse function */

    krb_action = -1;                    /* Initialize Kerberos action. */
    tmp_action = -1;                    /* And our local copy. */
    for (i = 0; i < KRB5_NUM_OF_ADDRS; i++)
      tmpaddrs[i] = NULL;

    if ((y = cmkey(kerbtab,kerbtabn,"authentication type","",xxstring)) < 0)
      {
          if (y == -3)
            printf("?Authentication type not specified - nothing happens\n");
          return(y);
      }
    tmpauth = y;
    debug(F101,"kerberos authentication","",tmpauth);
    switch (tmpauth) {
      case AUTH_KRB4: kv = 4; break;    /* Don't assume values are the same */
      case AUTH_KRB5: kv = 5; break;
      default:
        printf("?Authentication type not supported: \"%s\"\n",atmbuf);
        return(-9);
    }

    /* From here down is Kerberos */
    ini_kerb();                         /* Reset Init data to defaults */

    if (kv == 4) {                      /* Set K4 defaults */
        if (krb4_d_realm)
          makestr(&tmprealm,krb4_d_realm);
        if (krb4_d_principal)
          makestr(&tmpprinz,krb4_d_principal);
        if (krb4_d_instance)
          makestr(&tmpinst,krb4_d_instance);
    } else if (kv == 5) {               /* Set K5 defaults */
        if (krb5_d_cc)
          makestr(&tmpcache,krb5_d_cc);
        if (krb5_d_realm)
          makestr(&tmprealm,krb5_d_realm);
        if (krb5_d_principal)
          makestr(&tmpprinz,krb5_d_principal);
        if (krb5_d_instance)
          makestr(&tmpinst,krb5_d_instance);
    }
    for (i = 0; i <= KRB_I_MAX; i++) {  /* Initialize switch values */
        pv[i].sval = NULL;              /* to null pointers */
        pv[i].ival = 0;                 /* and 0 int values */
        pv[i].wval = (CK_OFF_T)-1;	/* and -1 wide values */
    }
    if (kv == 4) {                      /* Kerberos 4 */
        pv[KRB_I_LF].ival = krb4_d_lifetime;
        pv[KRB_I_PA].ival = krb4_d_preauth;

        if ((n = cmkey(krb_a_tbl,krb_a_n,"Kerberos 4 action","",xxstring)) < 0)
          {
              if (n == -3)
                printf("?Action not specified - nothing happens.\n");
              return(n);
          }
    } else if (kv == 5) {               /* Kerberos 5 */
        pv[KRB_I_FW].ival = krb5_d_forwardable;
        pv[KRB_I_PR].ival = krb5_d_proxiable;
        pv[KRB_I_LF].ival = krb5_d_lifetime;
        pv[KRB_I_RB].ival = krb5_d_renewable;
        pv[KRB_I_K4].ival = krb5_d_getk4;
        pv[KRB_I_NAD].ival = krb5_d_no_addresses;

        /* Make help message that shows switches and action keywords */
        ckstrncpy(tmphlp,"Kerberos 5 action, one of the following:\n ",256);
        for (i = 0; i < krb_a_n; i++) {
            ckstrncat(tmphlp,krb_a_tbl[i].kwd,sizeof(tmphlp));
            if (i == krb_a_n - 1)
              ckstrncat(tmphlp,"\nor switch",sizeof(tmphlp));
            else
              ckstrncat(tmphlp,"   ",sizeof(tmphlp));
        }
        /* Set up first set of chained FDB's */

        cmfdbi(&sw,                     /* First FDB - command switches */
               _CMKEY,                  /* fcode */
               tmphlp,                  /* hlpmsg */
               "",                      /* default (none) */
               "",                      /* addtl string data */
               krb_s_n,                 /* Switch table size */
               4,                       /* addtl numeric data 2: 4 = cmswi */
               xxstring,                /* Processing function */
               krb_s_tbl,               /* Switch table */
               &kw                      /* Pointer to next FDB */
               );
        cmfdbi(&kw,                     /* Second FDB - action keywords */
               _CMKEY,                  /* fcode */
               "Kerberos action",       /* hlpmsg */
               "",                      /* default (none) */
               "",                      /* addtl string data */
               krb_a_n,                 /* Switch table size */
               0,                       /* addtl num data (0 = NOT switch) */
               xxstring,                /* Processing function */
               krb_a_tbl,               /* Keyword table */
               NULL                     /* Pointer to next FDB (none) */
               );

        /* Parse */

        while (1) {                     /* Parse 0 or more switches */
            rc = cmfdb(&sw);            /* Parse something */
            debug(F101,"kerberos cmfdb 1 rc","",rc);
            if (rc < 0) {                       /* Error */
                if (rc == -3)
                  printf("?Action not specified - nothing happens.\n");
                return(rc);             /* or reparse needed */
            }
            if (cmresult.fdbaddr != &sw) /* Break out if not a switch */
              break;
            c = cmgbrk();               /* Have switch - get break character */
            getval = (c == ':' || c == '='); /* Must parse an agument? */
            if (getval && !(cmresult.kflags & CM_ARG)) {
                printf("?This switch does not take arguments\n");
                return(-9);             /* OK because nothing malloc'd yet */
            }
            if (!getval && (cmgkwflgs() & CM_ARG)) {
                printf("?This switch requires an argument\n");
                return(-9);
            }
            n = cmresult.nresult;       /* Numeric result = switch value */
            debug(F101,"kerberos command switch","",n);

            switch (n) {                /* Handle the switch */
              case KRB_S_CA:            /* /CACHE:<filename> */
                p = krb5_d_cc ? krb5_d_cc : "";
                if ((y = cmofi("Name of cache file",p,&s,xxstring)) < 0) {
                    if (y == -3)
                      s = NULL;
                    else
                      return(y);
                }
                makestr(&tmpcache,s);
                break;
              default:
                printf("?Unexpected switch value - internal error\n");
                return(-9);             /* (if) nothing malloc'd yet. */
            }
        }
        if (cmresult.fdbaddr != &kw) {  /* Checking... */
            printf("?Unexpected result - internal error\n");
            return(-9);                 /* Nothing malloc'd yet. */
        }
        n = cmresult.nresult;           /* Get keyword value */
    } else {
        printf("?Unexpected Kerberos version - Internal error\n");
        return(-9);
    }
    debug(F101,"kerberos action","",n);
    switch (n) {
      case KRB_A_IN:                    /* INITIALIZE */
      case KRB_A_DE:                    /* DESTROY */
      case KRB_A_LC:                    /* LIST-CREDENTIALS */
        tmp_action = n;                 /* OK, set */
        break;
      default:                          /* Not OK, punt. */
        printf("?Unexpected action - internal error\n");
        return(-9);
    }
    if (tmp_action == KRB_A_IN) {       /* Action is INITIALIZE */
        int x;
        cmfdbi(&sw,                     /* INITIALIZE switches */
               _CMKEY,                  /* fcode */
               "Principal,\n or optional INITIALIZE switch(es)", /* hlpmsg */
               "",                      /* default (none) */
               "",                      /* addtl string data */
               kv == 4 ?  krb4_i_n : krb5_i_n, /* Switch table size */
               4,                       /* addtl numeric data 2: 4 = cmswi */
               xxstring,                /* Processing function */
               kv == 4 ?  krb4_i_tbl : krb5_i_tbl,      /* Switch table */
               &fl                      /* Pointer to next FDB */
               );
        cmfdbi(&fl,                     /* 3rd FDB - command to send from */
               _CMFLD,                  /* fcode */
               "Principal",             /* hlpmsg */
               kv == 4 ? krb4_d_principal : krb5_d_principal, /* principal */
               "",                      /* addtl string data */
               0,                       /* addtl numeric data 1 */
               0,                       /* addtl numeric data 2 */
               xxstring,
               NULL,
               NULL
               );
        while (1) {                     /* Parse INIT switches or principal */
            rc = cmfdb(&sw);
            debug(F101,"kerberos cmfdb 2 rc","",rc);
            if (rc < 0) {
                if (rc == -3)
                  printf("?Principal name required\n");
                goto kerbx;
            }
            debug(F101,"kerberos cmfdb 2 fcode","",cmresult.fcode);
            if (cmresult.fcode != _CMKEY) /* Not a switch, quit switch loop */
              break;
            c = cmgbrk();               /* Switch - get break character */
            debug(F101,"kerberos cmfdb 2 cmgbrk","",c);
            getval = (c == ':' || c == '=');
            if (getval && !(cmresult.kflags & CM_ARG)) {
                printf("?This switch does not take arguments\n");
                return(-9);             /* OK because nothing malloc'd yet */
            }
            if (!getval && (cmgkwflgs() & CM_ARG)) {
                printf("?This switch requires an argument\n");
                return(-9);
            }
            n = cmresult.nresult;       /* Numeric result = switch value */
            switch (n) {
              /* These don't take args... */
              case KRB_I_PA:            /* /PREAUTH */
              case KRB_I_FW:            /* /FORWARDABLE */
              case KRB_I_PR:            /* /PROXIABLE */
              case KRB_I_RN:            /* /RENEW */
              case KRB_I_VA:            /* /VALIDATE */
              case KRB_I_NPA:           /* /NOT-PREAUTH */
              case KRB_I_NFW:           /* /NOT-FORWARDABLE */
              case KRB_I_NPR:           /* /NOT-PROXIABLE */
              case KRB_I_VB:            /* /VERBOSE */
              case KRB_I_BR:            /* /BRIEF */
              case KRB_I_K4:            /* /KERBEROS4 */
              case KRB_I_NK4:           /* /NO-KERBEROS4 */
              case KRB_I_POP:           /* /POPUP */
              case KRB_I_NAD:           /* /NO-ADDRESSES */
                if (getval) {
                    printf("?This switch does not take a value\n");
                    rc = -9;
                    goto kerbx;
                }
                switch (n) {
                  case KRB_I_NPA:
                    pv[KRB_I_PA].ival = 0;
                    break;
                  case KRB_I_NFW:
                    pv[KRB_I_FW].ival = 0;
                    break;
                  case KRB_I_NPR:
                    pv[KRB_I_PR].ival = 0;
                    break;
                  case KRB_I_VB:
                    pv[KRB_I_BR].ival = 0;
                    break;
                  case KRB_I_NK4:
                    pv[KRB_I_K4].ival = 0;
                    break;
                  default:
                    pv[n].ival = 1;
                }
                break;

                /* These do take arguments */

              case KRB_I_RB:            /* /RENEWABLE:<minutes> */
                pv[n].ival = 0;
                if (!getval) break;
                if ((rc = cmnum("Minutes",ckitoa(krb5_init.renewable),
                                10,&y, xxstring)) < 0)
                  goto kerbx;
                pv[n].ival = y;
                break;

              case KRB_I_LF:            /* /LIFETIME:<minutes> */
                pv[n].ival = 0;
                /* Default is previous value */
                sprintf(tmpbuf,"%d",    /* SAFE */
                        kv == 4 ?
                        krb4_init.lifetime :
                        krb5_init.lifetime
                        );
                if (!getval) break;
                if ((rc = cmnum("Minutes",tmpbuf,10,&y, xxstring)) < 0)
                  goto kerbx;
                pv[n].ival = y;
                break;

              case KRB_I_PD:            /* /POSTDATE:<timestamp> */
                if (pv[n].sval) {
                    free(pv[n].sval);
                    pv[n].sval = NULL;
                }
                if (!getval) break;
                if ((rc = cmdate("date-time","",&s,0,xxstring)) < 0)
                  goto kerbx;
                makestr(&(pv[n].sval),s);
                break;

              case KRB_I_SR:            /* /SERVICE:<name> */
                if (pv[n].sval) {
                    free(pv[n].sval);
                    pv[n].sval = NULL;
                }
                if (!getval) break;
                if ((rc = cmfld("Service-name","",&s,xxstring)) < 0)
                  goto kerbx;
                makestr(&(pv[n].sval),s);
                break;

              case KRB_I_RL:            /* /REALM:<name> */
                if (pv[n].sval) {
                    free(pv[n].sval);
                    pv[n].sval = NULL;
                }
                if (!getval) break;
                if (kv == 4)
                  p = krb4_d_realm ? krb4_d_realm : "";
                else
                  p = krb5_d_realm ? krb5_d_realm : "";
                if ((rc = cmfld("Realm",p,&s,xxstring)) < 0)
                  goto kerbx;
                makestr(&(pv[n].sval),s);
                break;

              case KRB_I_IN:            /* /INSTANCE:<name> */
                if (pv[n].sval) {
                    free(pv[n].sval);
                    pv[n].sval = NULL;
                }
                if (!getval) break;
                if (kv == 4)
                    p = krb4_d_instance ? krb4_d_instance : "";
                else
                    p = krb5_d_instance ? krb5_d_instance : "";
                if ((rc = cmfld("Instance",p,&s,xxstring)) < 0)
                  goto kerbx;
                makestr(&(pv[n].sval),s);
                break;

              case KRB_I_PW:            /* /PASSWORD:<password> */
                debok = 0;
                if (pv[n].sval) {
                    free(pv[n].sval);
                    pv[n].sval = NULL;
                }
                if (!getval) break;
                if ((rc = cmfld("Password","",&s,xxstring)) < 0)
                  if (rc != -3)
                    goto kerbx;
                makestr(&(pv[n].sval),s);
                break;

              case KRB_I_ADR:           /* /ADDRESSES:{<address-list>} */
                if (pv[n].sval) {
                    free(pv[n].sval);
                    pv[n].sval = NULL;
                }
                if (!getval) break;
                if ((rc = cmfld("List of IP addresses","",&s,xxstring)) < 0)
                  goto kerbx;
                makelist(s,tmpaddrs,KRB5_NUM_OF_ADDRS);
                for (i = 0; i < KRB5_NUM_OF_ADDRS && tmpaddrs[i]; i++) {
                    if (inet_addr(tmpaddrs[i]) == 0xffffffff) {
                        printf("invalid ip address: %s\n",tmpaddrs[i]);
                        rc = -9;
                        goto kerbx;
                    }
                }
                pv[KRB_I_NAD].ival = 0;
                break;

              default:
                printf("?Unexpected switch value - internal error\n");
                rc = -9;
                goto kerbx;
            }
        }
        if (cmresult.fcode != _CMFLD) {
            printf("?Unexected result - internal error\n");
            rc = -9;
            goto kerbx;
        }
        /* cmresult.sresult may be of the form PRINCIPAL@REALM */
        i = ckindex("@",cmresult.sresult,0,0,0);
        if (i != 0) {
            makestr(&tmprealm,&cmresult.sresult[i]);
            cmresult.sresult[i-1] = '\0';
        }
        makestr(&tmpprinz,cmresult.sresult); /* Principal (user) */

        if ((rc = cmcfm()) < 0) {       /* Now get confirmation */
            if (rc == -3) {
                printf("?Principal name required\n");
            }
            goto kerbx;
        }
        if (!tmpprinz || !tmpprinz[0]) {
            printf("?Principal name required\n");
            goto kerbx;
        }
        if (!pv[KRB_I_RN].ival && !pv[KRB_I_VA].ival) {
            /* Don't use a password if Validating or Renewing */
            if (pv[KRB_I_PW].sval) {    /* If they gave a /PASSWORD switch */
                makestr(&tmppswd,pv[KRB_I_PW].sval); /* use this value */
            }
#ifdef COMMENT
            /* Password prompting has been moved to ck_krb[45]_initTGT() */
            else {                      /* Otherwise must prompt for it */
                char prmpt[80];
                if (pv[KRB_I_RL].sval)
                  sprintf(prmpt,"%s@%s's Password: ",
                          tmpprinz,pv[KRB_I_RL].sval);
                else if (tmprealm)
                  sprintf(prmpt,"%s@%s's Password: ",
                          tmpprinz,tmprealm);
                else
                  sprintf(prmpt,"%s's Password: ",tmpprinz);
#ifdef OS2
                if (pv[KRB_I_POP].ival) {
                    char passwd[80]="";
                    readpass(prmpt,passwd,80);
                    makestr(&tmppswd,passwd);
                    memset(passwd,0,80);
                } else
#endif /* OS2 */
                {
#ifdef CK_RECALL
                    on_recall = 0;
#endif /* CK_RECALL */
                    cmsavp(psave,PROMPTL); /* Save old prompt */
                    cmsetp(prmpt);      /* Make new prompt */
                    concb((char)escape); /* Put console in cbreak mode */
                    cmini(0);           /* and no-echo mode */
                    /* Issue prompt if at top level */
                    if (pflag) prompt(xxstring);
                    cmres();            /* Reset the parser */
                    for (rc = -1; rc < 0; ) { /* Prompt till they answer */
                        /* Get a literal line of text */
                        rc = cmtxt("","",&s,NULL);
                        cmres();        /* Reset the parser again */
                    }
                    makestr(&tmppswd,s);
                    printf("\n");       /* Echo a CRLF */
                    cmsetp(psave);      /* Restore original prompt */
                }
            }
            x = 0;                      /* Check for password */
            if (tmppswd)
              if (*tmppswd)
                x = 1;
            if (!x) {
                printf("?Password required\n");
                goto kerbx;
            }
#endif /* COMMENT */
        }
    } else if (kv == 5 && tmp_action == KRB_A_LC) { /* LIST-CREDENTIALS */
        tmp_klc = 0;
        while (1) {
            if ((x = cmkey(klctab,nklctab,"Switch","",xxstring)) < 0) {
                if (x == -3) {
                    if ((rc = cmcfm()) < 0)
                      goto kerbx;
                    else
                      break;
                } else {
                    rc = x;
                    goto kerbx;
                }
            }
            tmp_klc |= x;
        }
    } else if ((rc = cmcfm()) < 0)      /* DESTROY, just confirm */
        goto kerbx;

/* Done - Move confirmed data to final locations */

    krb_action = tmp_action;            /* Action requested */
    krb_op.version = kv;                /* Kerberos version */
    krb_op.cache = tmpcache;            /* Cache file */
    tmpcache = NULL;                    /* So we don't free it */

    switch (krb_action) {
      case KRB_A_IN:                    /* INITIALIZE */
        if (kv == 5) {
            krb5_init.forwardable = pv[KRB_I_FW].ival;
            krb5_init.proxiable   = pv[KRB_I_PR].ival;
            krb5_init.lifetime    = pv[KRB_I_LF].ival;
            krb5_init.renew       = pv[KRB_I_RN].ival;
            krb5_init.renewable   = pv[KRB_I_RB].ival;
            krb5_init.validate    = pv[KRB_I_VA].ival;

            /* Here we just reassign the pointers and then set them to NULL */
            /* so they won't be freed below. */

            krb5_init.postdate = pv[KRB_I_PD].sval; pv[KRB_I_PD].sval = NULL;
            krb5_init.service  = pv[KRB_I_SR].sval; pv[KRB_I_SR].sval = NULL;
            if (pv[KRB_I_RL].sval) {
                krb5_init.realm  = pv[KRB_I_RL].sval; pv[KRB_I_RL].sval = NULL;
            } else if (tmprealm) {
                krb5_init.realm = tmprealm; tmprealm = NULL;
            }
            if (pv[KRB_I_IN].sval) {
                krb5_init.instance = pv[KRB_I_IN].sval;
                pv[KRB_I_IN].sval = NULL;
            } else if ( tmpinst ) {
                krb5_init.instance = tmpinst;
                tmpinst = NULL;
            }
            if (tmpprinz) {
                krb5_init.principal = tmpprinz;
                tmpprinz = NULL;
            }
            krb5_init.password = tmppswd;
            tmppswd = NULL;

            krb5_init.getk4 = pv[KRB_I_K4].ival;
            if (krb5_init.getk4) {
                krb4_init.lifetime = pv[KRB_I_LF].ival;
                if (krb5_init.realm)
                    makestr(&krb4_init.realm,krb5_init.realm);
                krb4_init.preauth  = krb4_d_preauth;
                krb4_init.verbose  = pv[KRB_I_BR].ival ? 0 : 1;
                if (krb5_init.principal)
                    makestr(&krb4_init.principal,krb5_init.principal);
                if (krb5_init.principal)
                    makestr(&krb4_init.password,krb5_init.password);
            }
            krb5_init.no_addresses = pv[KRB_I_NAD].ival;
            if (tmpaddrs[0]) {
                for (i = 0; i < KRB5_NUM_OF_ADDRS; i++) {
                    if (krb5_init.addrs[i]) {
                        free(krb5_init.addrs[i]);
                        krb5_init.addrs[i] = NULL;
                    }
                    krb5_init.addrs[i] = tmpaddrs[i];
                    tmpaddrs[i] = NULL;
                }
            }
        } else if (kv == 4) {           /* Same deal for Kerberos 4 */
            krb4_init.lifetime = pv[KRB_I_LF].ival;
            if (pv[KRB_I_RL].sval) {
                krb4_init.realm  = pv[KRB_I_RL].sval;
                pv[KRB_I_RL].sval = NULL;
            } else if ( tmprealm ) {
                krb4_init.realm  = tmprealm;
                tmprealm = NULL;
            }
            if (pv[KRB_I_IN].sval) {
                krb4_init.instance = pv[KRB_I_IN].sval;
                pv[KRB_I_IN].sval = NULL;
            } else if ( tmpinst ) {
                krb4_init.instance = tmpinst;
                tmpinst = NULL;
            }
            krb4_init.preauth  = pv[KRB_I_PA].ival;
            krb4_init.verbose  = pv[KRB_I_BR].ival ? 0 : 1;

            if (tmpprinz) {
                krb4_init.principal = tmpprinz;
                tmpprinz = NULL;
            }
            krb4_init.password = tmppswd;
            tmppswd = NULL;
        }
        break;
      case KRB_A_LC:                    /* List Credentials */
        krb5_lc.encryption = tmp_klc & XYKLCEN;
        krb5_lc.flags = tmp_klc & XYKLCFL;
        krb5_lc.addr  = tmp_klc & XYKLCAD;
        break;
    }

/* Common exit - Free temporary storage */

  kerbx:
    for (i = 0; i <= KRB_I_MAX; i++) {  /* Free malloc'd switch data */
        if (pv[i].sval)
          free(pv[i].sval);
    }
    for (i = 0; i < KRB5_NUM_OF_ADDRS; i++) {
        if (tmpaddrs[i])
          free(tmpaddrs[i]);
    }
    if (tmpprinz) free(tmpprinz);       /* And these too. */
    if (tmppswd)  free(tmppswd);
    if (tmpcache) free(tmpcache);
    if (tmprealm) free(tmprealm);
    if (tmpinst)  free(tmpinst);

    return(rc);                         /* Return the return code */
}
#endif /* CK_KERBEROS */

#ifdef CK_LOGIN
int
#ifdef CK_ANSIC
ckxlogin(CHAR * userid, CHAR * passwd, CHAR * acct, int promptok)
#else /* CK_ANSIC */
ckxlogin(userid, passwd, acct, promptok)
  CHAR * userid; CHAR * passwd; CHAR * acct; int promptok;
#endif /* CK_ANSIC */
/* ckxlogin */ {
#ifdef CK_RECALL
    extern int on_recall;               /* around Password prompting */
#endif /* CK_RECALL */
#ifdef COMMENT
    extern int guest;
#endif /* COMMENT */
    int rprompt = 0;                    /* Restore prompt */
#ifdef CKSYSLOG
    int savlog;
#endif /* CKSYSLOG */

    extern int what, srvcdmsg;

    int x = 0, ok = 0, rc = 0;
    CHAR * _u = NULL, * _p = NULL, * _a = NULL;

    debug(F111,"ckxlogin userid",userid,promptok);
    debug(F110,"ckxlogin passwd",passwd,0);

    isguest = 0;                        /* Global "anonymous" flag */

    if (!userid) userid = (CHAR *)"";
    if (!passwd) passwd = (CHAR *)"";

    debug(F111,"ckxlogin userid",userid,what);

#ifdef CK_RECALL
    on_recall = 0;
#endif /* CK_RECALL */

#ifdef CKSYSLOG
    savlog = ckxsyslog;                 /* Save and turn off syslogging */
#endif /* CKSYSLOG */

    if ((!*userid || !*passwd) &&       /* Need to prompt for missing info */
        promptok) {
        cmsavp(psave,PROMPTL);          /* Save old prompt */
        debug(F110,"ckxlogin saved",psave,0);
        rprompt = 1;
    }
    if (!*userid) {
        if (!promptok)
          return(0);
        cmsetp("Username: ");           /* Make new prompt */
        concb((char)escape);            /* Put console in cbreak mode */
        cmini(1);

/* Flush typeahead */

#ifdef IKS_OPTION
        debug(F101,
              "ckxlogin TELOPT_SB(TELOPT_KERMIT).kermit.me_start",
              "",
              TELOPT_SB(TELOPT_KERMIT).kermit.me_start
              );
#endif /* IKS_OPTION */

        while (ttchk() > 0) {
            x = ttinc(0);
            debug(F101,"ckxlogin flush user x","",x);
            if (x < 0)
              doexit(GOOD_EXIT,0);      /* Connection lost */
#ifdef TNCODE
            if (sstelnet) {
                if (x == IAC) {
                    x = tn_doop((CHAR)(x & 0xff),ckxech,ttinc);
                    debug(F101,"ckxlogin user tn_doop","",x);
#ifdef IKS_OPTION
                    debug(F101,
                      "ckxlogin user TELOPT_SB(TELOPT_KERMIT).kermit.me_start",
                      "",
                      TELOPT_SB(TELOPT_KERMIT).kermit.me_start
                      );
#endif /* IKS_OPTION */

                    if (x < 0)
                      goto XCKXLOG;
                    switch (x) {
                      case 1: ckxech = 1; break; /* Turn on echoing */
                      case 2: ckxech = 0; break; /* Turn off echoing */
#ifdef IKS_OPTION
                      case 4:                    /* IKS event */
                        if (!TELOPT_SB(TELOPT_KERMIT).kermit.me_start)
                          break;                 /* else fall thru... */
#endif /* IKS_OPTION */
                      case 6:                    /* Logout */
                        goto XCKXLOG;
                    }
                }
            }
#endif /* TNCODE */
        }
        if (pflag) prompt(xxstring);    /* Issue prompt if at top level */
        cmres();                        /* Reset the parser */
        for (x = -1; x < 0;) {          /* Prompt till they answer */
            /* Get a literal line of text */
            x=cmtxt("Your username, or \"ftp\", or \"anonymous\"","",&s,NULL);
            if (x == -4 || x == -10) {
                printf("\r\n%sLogin cancelled\n",
		       x == -10 ? "Timed out: " : "");
#ifdef CKSYSLOG
                ckxsyslog = savlog;
#endif /* CKSYSLOG */
                doexit(GOOD_EXIT,0);
            }
            if (sstate)                 /* Did a packet come instead? */
              goto XCKXLOG;
            cmres();                    /* Reset the parser again */
        }
        if ((_u = (CHAR *)malloc((int)strlen(s) + 1)) == NULL) {
            printf("?Internal error: malloc\n");
            goto XCKXLOG;
        } else {
            strcpy((char *)_u,s);       /* safe */
            userid = _u;
        }
    }
    ok = zvuser((char *)userid);        /* Verify username */
    debug(F111,"ckxlogin zvuser",userid,ok);

    if (!*passwd && promptok
#ifdef COMMENT
        && guest
#endif /* COMMENT */
        ) {
        char prmpt[80];

#ifdef CKSYSLOG
        savlog = ckxsyslog;             /* Save and turn off syslogging */
        ckxsyslog = 0;
#endif /* CKSYSLOG */

/* Flush typeahead again */

        while (ttchk() > 0) {
            x = ttinc(0);
            debug(F101,"ckxlogin flush user x","",x);
#ifdef TNCODE
            if (sstelnet) {
                if (x == IAC) {
                    x = tn_doop((CHAR)(x & 0xff),ckxech,ttinc);
                    debug(F101,"ckxlogin pass tn_doop","",x);
#ifdef IKS_OPTION
                    debug(F101,
                      "ckxlogin pass TELOPT_SB(TELOPT_KERMIT).kermit.me_start",
                      "",
                      TELOPT_SB(TELOPT_KERMIT).kermit.me_start
                      );
#endif /* IKS_OPTION */
                    if (x < 0)
                      goto XCKXLOG;
                    switch (x) {
                      case 1: ckxech = 1; break; /* Turn on echoing */
                      case 2: ckxech = 0; break; /* Turn off echoing */
                      case 4:                    /* IKS event */
                        if (!TELOPT_SB(TELOPT_KERMIT).kermit.me_start)
                          break;                 /* else fall thru... */
                      case 6:                    /* Logout */
                        goto XCKXLOG;
                    }
                }
            }
#endif /* TNCODE */
        }
        if (!strcmp((char *)userid,"anonymous") ||
            !strcmp((char *)userid,"ftp")) {
            if (!ok)
              goto XCKXLOG;
            ckstrncpy(prmpt,"Enter e-mail address as Password: ",80);
        } else if (*userid && strlen((char *)userid) < 60) {
#ifdef NT
            extern CHAR * pReferenceDomainName;
            if (pReferenceDomainName)
              ckmakxmsg(prmpt,
                       80,
                       "Enter ",
                       pReferenceDomainName,
                       "\\\\",
                       userid,
                       "'s Password: ",
                       NULL,NULL,NULL,NULL,NULL,NULL,NULL
                       );
            else
#endif /* NT */
              ckmakmsg(prmpt,80,"Enter ",(char *)userid,"'s Password: ",NULL);
        } else
          ckstrncpy(prmpt,"Enter Password: ",80);
        cmsetp(prmpt);                  /* Make new prompt */
        concb((char)escape);            /* Put console in cbreak mode */
        if (strcmp((char *)userid,"anonymous") &&
            strcmp((char *)userid,"ftp")) { /* and if not anonymous */
            debok = 0;
            cmini(0);                   /* and no-echo mode */
        } else {
            cmini(1);
        }
        if (pflag) prompt(xxstring);    /* Issue prompt if at top level */
        cmres();                        /* Reset the parser */
        for (x = -1; x < 0;) {          /* Prompt till they answer */
#ifdef CK_PAM
	    gotemptypasswd=0;
#endif /* CK_PAM */
            x = cmtxt("","",&s,NULL);   /* Get a literal line of text */
            if (x == -4 || x == -10) {
                printf("\r\n%sLogin cancelled\n",
		       x == -10 ? "Timed out: " : "");
#ifdef CKSYSLOG
                ckxsyslog = savlog;
#endif /* CKSYSLOG */
                doexit(GOOD_EXIT,0);
            }
#ifdef CK_PAM
	    if (!*s)
	      gotemptypasswd = 1;
#endif /* CK_PAM */
            if (sstate)                 /* In case of a Kermit packet */
              goto XCKXLOG;
            cmres();                    /* Reset the parser again */
        }
        printf("\r\n");                 /* Echo a CRLF */
        if ((_p = (CHAR *)malloc((int)strlen(s) + 1)) == NULL) {
            printf("?Internal error: malloc\n");
            goto XCKXLOG;
        } else {
            strcpy((char *)_p,s);       /* safe */
            passwd = _p;
        }
    }
#ifdef CK_PAM
    else {
        cmres();                        /* Reset the parser */

        /* We restore the prompt now because the PAM engine will call  */
        /* readpass() which will overwrite psave. */
        if (rprompt) {
            cmsetp(psave);              /* Restore original prompt */
            debug(F110,"ckxlogin restored",psave,0);
            rprompt = 0;
        }
    }
#endif /* CK_PAM */

#ifdef CKSYSLOG
    ckxsyslog = savlog;
#endif /* CKSYSLOG */

    if (ok) {
        ok = zvpass((char *)passwd);    /* Check password */
        debug(F101,"ckxlogin zvpass","",ok);
#ifdef CK_PAM
    } else {
	/* Fake pam password failure for nonexistent users */
	sleep(1);
	printf("Authentication failure\n");
#endif	/* CK_PAM */
    }

    if (ok > 0 && isguest) {
#ifndef NOPUSH
        nopush = 1;
#endif /* NOPUSH */
        srvcdmsg = 1;
    }
    rc = ok;                            /* Set the return code */
    if ((char *)uidbuf != (char *)userid)
      ckstrncpy(uidbuf,(char *)userid,UIDBUFLEN); /* Remember username */

  XCKXLOG:                              /* Common exit */
#ifdef CKSYSLOG
    ckxsyslog = savlog;                 /* In case of GOTO above */
#endif /* CKSYSLOG */
    if (rprompt) {
        cmsetp(psave);                  /* Restore original prompt */
        debug(F110,"ckxlogin restored",psave,0);
    }
    if (_u || _p || _a) {
        if (_u) free(_u);
        if (_p) free(_p);
        if (_a) free(_a);
    }
    return(rc);
}

int
ckxlogout() {
    doexit(GOOD_EXIT,0);                /* doexit calls zvlogout */
    return(0);                          /* not reached */
}
#endif /* CK_LOGIN */

#endif /* NOICP */

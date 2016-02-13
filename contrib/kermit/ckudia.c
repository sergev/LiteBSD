#include "ckcsym.h"
char *dialv = "Dial Command, 9.0.160, 16 Oct 2009";

/*  C K U D I A	 --  Module for automatic modem dialing. */

/*
  Copyright (C) 1985, 2009,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/

/*
  Authors:

  Original (version 1, 1985) author: Herm Fischer, Encino, CA.
  Contributed to Columbia University in 1985 for inclusion in C-Kermit 4.0.
  Author and maintainer since 1985: Frank da Cruz, Columbia University,
  fdc@columbia.edu.

  Contributions by many others throughout the years, including: Jeffrey
  Altman, Mark Berryman, Fernando Cabral, John Chmielewski, Joe Doupnik,
  Richard Hill, Larry Jacobs, Eric Jones, Tom Kloos, Bob Larson, Peter Mauzey,
  Joe Orost, Kevin O'Gorman, Kai Uwe Rommel, Dan Schullman, Warren Tucker, and
  many others.
*/

/*
  Entry points:
    ckdial(char * number)   Dial a number or answer a call
    dialhup()               Hang up a dialed connection
    mdmhup()                Use modem commands to hang up

  All other routines are static.
  Don't call dialhup() or mdmhup() without first calling ckdial().
*/

/*
  This module calls externally defined system-dependent functions for
  communications i/o, as described in CKCPLM.DOC, the C-Kermit Program Logic
  Manual, and thus should be portable to all systems that implement those
  functions, and where alarm() and signal() work.

  HOW TO ADD SUPPORT FOR ANOTHER MODEM:

  1. In ckuusr.h, define a modem-type number symbol (n_XXX) for the new modem,
     the next highest one.

  2. In ckuusr.h, adjust MAX_MDM to the new number of modem types.

The remaining steps are in this module:

  3. Create a MDMINF structure for it.  NOTE: The wake_str should include
     all invariant setup info, e.g. enable result codes, BREAK transparency,
     modulation negotiation, etc.  See ckcker.h for MDMINF struct definition.

  4. Add the address of the MDMINF structure to the modemp[] array,
     according to the numerical value of the modem-type number.

  5. Add the user-visible (SET MODEM) name and corresponding modem number
     to the mdmtab[] array, in alphabetical order by modem-name string.

  6. If this falls into a class like is_rockwell, is_supra, etc, add the new
     one to the definition of the class.

  7. Adjust the gethrn() routine to account for any special numeric result
     codes (if it's a Hayes compatible modem).

  8. Read through the code and add any modem-specific sections as necessary.
     For most modern Hayes-compatible modems, no specific code will be
     needed.

  NOTE: The MINIDIAL symbol is used to build this module to include support
  for only a minimum number of standard and/or generally useful modem types,
  namely Hayes 1200 and 2400, ITU-T (CCITT) V.25bis and V.25ter (V.250),
  Generic-High-Speed, "Unknown", and None.  When adding support for a new
  modem type, keep it outside of the MINIDIAL sections unless it deserves to
  be in it.
*/

#include "ckcdeb.h"
#ifndef NOLOCAL
#ifndef NODIAL
#ifndef NOICP

#ifndef CK_ATDT
#define CK_ATDT
#endif /* CK_ATDT */

#ifndef NOOLDMODEMS        /* Unless instructed otherwise, */
#define OLDMODEMS          /* keep support for old modems. */
#endif /* NOOLDMODEMS */

#ifndef M_OLD		   /* Hide old modem keywords in SET MODEM table. */
#define M_OLD 0		   /* Define as CM_INV to make them invisible. */
#endif /* M_OLD */

#ifndef M_ALIAS
#define M_ALIAS 64
#endif /* M_ALIAS */

#ifndef MAC
#include <signal.h>
#endif /* MAC */
#include "ckcasc.h"
#include "ckcker.h"
#include "ckucmd.h"
#include "ckcnet.h"
#include "ckuusr.h"

#ifdef OS2ONLY
#define INCL_VIO			/* Needed for ckocon.h */
#include <os2.h>
#undef COMMENT
#include "ckocon.h"
#endif /* OS2ONLY */

#ifdef NT
#include <windows.h>
#include <tapi.h>
#include "cknwin.h"
#include "ckntap.h"
#endif /* NT */
#ifdef OS2
#include "ckowin.h"
#endif /* OS2 */

#ifndef ZILOG
#ifdef NT
#include <setjmpex.h>
#else /* NT */
#include <setjmp.h>
#endif /* NT */
#else
#include <setret.h>
#endif /* ZILOG */

#include "ckcsig.h"        /* C-Kermit signal processing */

#ifdef MAC
#define signal msignal
#define SIGTYP long
#define alarm malarm
#define SIG_IGN 0
#define SIGALRM 1
#define SIGINT  2
SIGTYP (*msignal(int type, SIGTYP (*func)(int)))(int);
#endif /* MAC */

#ifdef AMIGA
#define signal asignal
#define alarm aalarm
#define SIGALRM (_NUMSIG+1)
#define SIGTYP void
SIGTYP (*asignal(int type, SIGTYP (*func)(int)))(int);
unsigned aalarm(unsigned);
#endif /* AMIGA */

#ifdef STRATUS
/*
  VOS doesn't have alarm(), but it does have some things we can work with.
  However, we have to catch all the signals in one place to do this, so
  we intercept the signal() routine and call it from our own replacement.
*/
#define signal vsignal
#define alarm valarm
SIGTYP (*vsignal(int type, SIGTYP (*func)(int)))(int);
int valarm(int interval);
#ifdef putchar
#undef putchar
#endif /* putchar */
#define putchar(x) conoc(x)
#ifdef getchar
#undef getchar
#endif /* getchar */
#define getchar(x) coninc(0)
#endif /* STRATUS */

#ifdef OS2
#ifdef putchar
#undef putchar
#endif /* putchar */
#define putchar(x) conoc(x)
#endif /* OS2 */

#ifndef NOHINTS
extern int hints;
#endif /* NOHINTS */

#ifdef CK_TAPI
extern int tttapi;
extern int tapipass;
#endif /* CK_TAPI */

#ifdef CKLOGDIAL
extern int dialog;
#endif /* CKLOGDIAL */

char * dialmsg[] = {			/* DIAL status strings */

    /* Keyed to numbers defined in ckcker.h -- keep in sync! */

    "DIAL succeeded",			    /*  0 DIA_OK */
    "Modem type not specified",		    /*  1 DIA_NOMO */
    "Communication device not specified",   /*  2 DIA_NOLI */
    "Communication device can't be opened", /*  3 DIA_OPEN */
    "Speed not specified",		    /*  4 DIA_NOSP */
    "Pre-DIAL hangup failed",		    /*  5 DIA_HANG */
    "Internal error",			    /*  6 DIA_IE   */
    "Device input/output error",	    /*  7 DIA_IO   */
    "DIAL TIMEOUT expired",		    /*  8 DIA_TIMO */
    "Interrupted by user",		    /*  9 DIA_INTR */
    "Modem not ready",			    /* 10 DIA_NRDY */
    "Partial dial OK",			    /* 11 DIA_PART */
    "Dial directory lookup error",	    /* 12 DIA_DIR  */
    "Hangup OK",			    /* 13 DIA_HUP  */
    NULL,				    /* 14 (undef)  */
    NULL,				    /* 15 (undef)  */
    NULL,				    /* 16 (undef)  */
    NULL,				    /* 17 (undef)  */
    NULL,				    /* 18 (undef)  */
    "No response from modem",		    /* 19 DIA_NRSP */
    "Modem command error",		    /* 20 DIA_ERR  */
    "Failure to initialize modem",	    /* 21 DIA_NOIN */
    "Phone busy",			    /* 22 DIA_BUSY */
    "No carrier",			    /* 23 DIA_NOCA */
    "No dialtone",			    /* 24 DIA_NODT */
    "Incoming call",			    /* 25 DIA_RING */
    "No answer",			    /* 26 DIA_NOAN */
    "Disconnected",			    /* 27 DIA_DISC */
    "Answered by voice",		    /* 28 DIA_VOIC */
    "Access denied / forbidden call",	    /* 29 DIA_NOAC */
    "Blacklisted",			    /* 30 DIA_BLCK */
    "Delayed",    			    /* 31 DIA_DELA */
    "Fax connection",			    /* 32 DIA_FAX  */
    "Digital line",			    /* 33 DIA_DIGI */
    "TAPI dialing failure",	            /* 34 DIA_TAPI */
    NULL				    /* 34 */
};

#ifdef COMMENT
#ifdef NOSPL
static
#endif /* NOSPL */
char modemmsg[128] = { NUL, NUL };	/* DIAL response from modem */
#endif /* COMMENT */

#ifdef NTSIG
extern int TlsIndex;
#endif /* NTSIG */

int mdmtyp = n_GENERIC;			/* Default modem type */
int mdmset = 0;				/* User explicitly set a modem type */

int					/* SET DIAL parameters */
  dialhng = 1,				/* DIAL HANGUP, default is ON */
  dialdpy = 0,				/* DIAL DISPLAY, default is OFF */
  mdmspd  = 0,				/* DIAL SPEED-MATCHING (0 = OFF) */
  mdmspk  = 1,				/* MODEM SPEAKER */
  mdmvol  = 2,				/* MODEM VOLUME */
  dialtmo = 0,				/* DIAL TIMEOUT */
  dialatmo = -1,			/* ANSWER TIMEOUT */
  dialksp = 0,				/* DIAL KERMIT-SPOOF, 0 = OFF */
  dialidt = 0,				/* DIAL IGNORE-DIALTONE */
#ifndef CK_RTSCTS
  /* If we can't do RTS/CTS then there's no flow control at first.  */
  /* So we might easily lose the echo to the init string and the OK */
  /* and then give "No response from modem" errors. */
  dialpace = 150,			/* DIAL PACING */
#else
  dialpace = -1,
#endif /* CK_RTSCTS */

  /* 0 = RS232 (drop DTR); 1 = MODEM-COMMAND (e.g. <sec>+++<sec>ATH0) */
  dialmhu = DEFMDMHUP;			/* MODEM HANGUP-METHOD */

int
  dialec = 1,				/* DIAL ERROR-CORRECTION */
  dialdc = 1,				/* DIAL COMPRESSION  */
#ifdef VMS
  /* VMS can only use Xon/Xoff */
  dialfc = FLO_XONX,			/* DIAL FLOW-CONTROL */
#else
  dialfc = FLO_AUTO,
#endif /* VMS */
  dialmth = XYDM_D,			/* DIAL METHOD (Tone, Pulse, Defalt) */
  dialmauto = 1,			/* DIAL METHOD is AUTO */
  dialesc = 0;				/* DIAL ESCAPE */

int telephony = 0;			/* Command-line '-T' option */

long dialmax = 0L,			/* Modem's max interface speed */
  dialcapas  = 0L;			/* Modem's capabilities */

int dialsta = DIA_UNK;			/* Detailed return code (ckuusr.h) */

#ifdef COMMENT
int ans_cid = 0;			/* SET ANSWER parameters */
int ans_rings = 0;			/* (not used yet...) */
#endif /* COMMENT */

int is_rockwell = 0;
int is_motorola = 0;
int is_supra = 0;
int is_hayeshispd = 0;

/* Dialing directory list */

char *dialdir[MAXDDIR];			/* DIAL DIRECTORY filename array */
int   ndialdir = 0;			/* How many dial directories */

/* User overrides for built-in modem commands */

char *dialini = NULL;			/* MODEM INIT-STRING none */
char *dialmstr = NULL;			/* MODEM DIALMODE-STRING */
char *dialmprmt = NULL;			/* MODEM DIALMODE-PROMPT */
char *dialcmd = NULL;			/* MODEM DIAL-COMMAND, default none */
char *dialname  = NULL;			/* Descriptive name for modem */
char *dialdcon  = NULL;			/* DC ON command */
char *dialdcoff = NULL;			/* DC OFF command */
char *dialecon  = NULL;			/* EC ON command */
char *dialecoff = NULL;			/* EC OFF command */
char *dialaaon  = NULL;			/* Autoanswer ON command */
char *dialaaoff = NULL;			/* Autoanswer OFF command */
char *dialhcmd  = NULL;			/* Hangup command */
char *dialhwfc  = NULL;			/* Hardware flow control command */
char *dialswfc  = NULL;			/* (Local) software f.c. command */
char *dialnofc  = NULL;			/* No (Local) flow control command */
char *dialtone  = NULL;			/* Command to force tone dialing */
char *dialpulse = NULL;			/*  ..to force pulse dialing */
char *dialx3    = NULL;			/* Ignore dialtone */
char *mdmname   = NULL;
char *dialspon  = NULL;			/* Speaker On command */
char *dialspoff = NULL;			/* Speaker Off command */
char *dialvol1  = NULL;			/* Volume Low command */
char *dialvol2  = NULL;			/* Volume Medium command */
char *dialvol3  = NULL;			/* Volume High command */
char *dialini2  = NULL;			/* Second init string */

/* Phone number options */

char *dialnpr = NULL;			/* DIAL PREFIX, ditto */
char *diallac = NULL;			/* DIAL LOCAL-AREA-CODE, ditto */
char *diallcc = NULL;			/* DIAL LOCAL-COUNTRY-CODE, ditto */
char *dialixp = NULL;			/* DIAL INTL-PREFIX */
char *dialixs = NULL;			/* DIAL INTL-SUFFIX */
char *dialldp = NULL;			/* DIAL LD-PREFIX */
char *diallds = NULL;			/* DIAL LD-SUFFIX */
char *diallcp = NULL;			/* DIAL LOCAL-PREFIX */
char *diallcs = NULL;			/* DIAL LOCAL-SUFFIX */
char *dialpxi = NULL;			/* DIAL PBX-INTERNAL-PREFIX */
char *dialpxo = NULL;			/* DIAL PBX-OUTSIDE-PREFIX */
char *dialsfx = NULL;			/* DIAL SUFFIX */
char *dialtfp = NULL;			/* DIAL TOLL-FREE-PREFIX */

char *callid_date = NULL;		/* Caller ID strings */
char *callid_time = NULL;
char *callid_name = NULL;
char *callid_nmbr = NULL;
char *callid_mesg = NULL;

extern char * d_name;
extern char * dialtfc[];		/* DIAL TOLL-FREE-AREA-CODE */
extern char * dialpxx[];		/* DIAL PBX-EXCHANGE */
extern int ntollfree;
extern int ndialpxx;

extern char * dialpucc[];		/* DIAL Pulse countries */
extern int ndialpucc;
extern char * dialtocc[];		/* DIAL Tone countries */
extern int ndialtocc;

char *dialmac   = NULL;			/* DIAL macro */

/* Countries where pulse dialing must be used (tone is not available) */
static char * pulsecc[] = { NULL };	/* (Unknown at present) */

/* Countries where tone dialing may safely be the default. */
/* "+" marks countries where pulse is also allowed. */
/* Both Pulse and Tone are allowed in Austria & Switzerland but it is not */
/* yet known if Tone is universally in those countries. */
static char * tonecc[] = {
    "1",				/* + North American Numbering Plan */
    "31",				/*   Netherlands */
    "32",				/*   Belgium */
    "33",				/*   France */
    "352",				/*   Luxembourg */
    "353",				/*   Ireland */
    "354",				/*   Iceland */
    "358",				/*   Finland */
    "39",				/*   Italy */
    "44",				/* + UK */
    "45",				/*   Denmark */
    "46",				/*   Sweden */
    "47",				/*   Norway */
    "49",				/* + Germany */
    NULL
};

#ifndef MINIDIAL
/*
  Telebit model codes:

  ATI  Model Numbers           Examples
  ---  -------------           --------
  123                          Telebit in "total Hayes-1200" emulation mode
  960                          Telebit in Conventional Command (Hayes) mode
  961  RA12C                   IBM PC internal original Trailblazer
  962  RA12E                   External original Trailblazer
  963  RM12C                   Rackmount original Trailblazer
  964  T18PC                   IBM PC internal Trailblazer-Plus (TB+)
  965  T18SA, T2SAA, T2SAS     External TB+, T1600, T2000, T3000, WB, and later
  966  T18RMM                  Rackmount TB+
  967  T2MC                    IBM PS/2 internal TB+
  968  T1000                   External T1000
  969  ?                       Qblazer
  970                          Qblazer Plus
  971  T2500                   External T2500
  972  T2500                   Rackmount T2500
*/

/* Telebit model codes */

#define TB_UNK  0			/* Unknown Telebit model */
#define TB_BLAZ 1			/* Original TrailBlazer */
#define TB_PLUS	2			/* TrailBlazer Plus */
#define TB_1000 3			/* T1000 */
#define TB_1500 4			/* T1500 */
#define TB_1600 5			/* T1600 */
#define TB_2000 6			/* T2000 */
#define TB_2500 7			/* T2500 */
#define TB_3000 8			/* T3000 */
#define TB_QBLA 9			/* Qblazer */
#define TB_WBLA 10			/* WorldBlazer */
#define TB__MAX 10			/* Highest number */

char *tb_name[] = {			/* Array of model names */
    "Unknown",				/* TB_UNK  */
    "TrailBlazer",			/* TB_BLAZ */
    "TrailBlazer-Plus",			/* TB_PLUS */
    "T1000",				/* TB_1000 */
    "T1500",				/* TB_1500 */
    "T1600",				/* TB_1600 */
    "T2000",				/* TB_2000 */
    "T2500",				/* TB_2500 */
    "T3000",				/* TB_3000 */
    "Qblazer",				/* TB_QBLA */
    "WorldBlazer",			/* TB_WBLA */
    ""
};
#endif /* MINIDIAL */

extern int flow, local, mdmtyp, quiet, backgrd, parity, seslog, network;
extern int carrier, duplex, mdmsav, reliable, setreliable;
extern int ttnproto, nettype;
extern long speed;
extern char ttname[], sesfil[];
#ifndef NOXFER
extern CHAR stchr;
extern int interrupted;
#endif /* NOXFER */

/*  Failure codes  */

#define F_TIME		1		/* timeout */
#define F_INT		2		/* interrupt */
#define F_MODEM		3		/* modem-detected failure */
#define F_MINIT		4		/* cannot initialize modem */

#ifndef CK_TAPI
static
#endif /* CK_TAPI */
#ifdef OS2
 volatile
#endif /* OS2 */
 int fail_code =  0;			/* Default failure reason. */

static int xredial = 0;
static int func_code;			/* 0 = dialing, nonzero = answering */
static int partial;
static int mymdmtyp = 0;

#define DW_NOTHING      0		/* What we are doing */
#define DW_INIT         1
#define DW_DIAL         2

static int dial_what = DW_NOTHING;	/* Nothing at first. */
static int nonverbal = 0;		/* Hayes in numeric response mode */
static MDMINF * mp;
static CHAR escbuf[6];
static long mdmcapas;

_PROTOTYP (static VOID dreset, (void) );
_PROTOTYP (static int (*xx_ok), (int,int) );
_PROTOTYP (static int ddinc, (int) );
_PROTOTYP (int dialhup, (void) );
_PROTOTYP (int getok, (int,int) );
_PROTOTYP (char * ck_time, (void) );
_PROTOTYP (static VOID ttslow, (char *, int) );
#ifdef COMMENT
_PROTOTYP (static VOID xcpy, (char *, char *, unsigned int) );
#endif /* COMMENT */
_PROTOTYP (static VOID waitfor, (char *) );
_PROTOTYP (static VOID dialoc, (char) );
_PROTOTYP (static int didweget, (char *, char *) );
_PROTOTYP (static VOID spdchg, (long) );
_PROTOTYP (static int dialfail, (int) );
_PROTOTYP (static VOID gethrw, (void) );
_PROTOTYP (static VOID gethrn, (void) );

int dialudt = n_UDEF;			/* Number of user-defined type */

/* BEGIN MDMINF STRUCT DEFINITIONS */

/*
  Declare structures containing modem-specific information.
  REMEMBER that only the first SEVEN characters of these names are
  guaranteed to be unique.

  First declare the three types that are allowed for MINIDIAL versions.
*/
static
MDMINF CCITT =				/* CCITT / ITU-T V.25bis autodialer */
/*
  According to V.25bis:
  . Even parity is required for giving commands to the modem.
  . Commands might or might not echo.
  . Responses ("Indications") from the modem are terminated by CR and LF.
  . Call setup is accomplished by:
    - DTE raises DTR (V.24 circuit 108)              [ttopen() does this]
    - Modem raises CTS (V.24 circuit 106)            [C-Kermit ignores this]
    - DTE issues a call request command ("CRN")
    - Modem responds with "VAL" ("command accepted")
    - If the call is completed:
        modem responds with "CNX" ("call connected");
        modem turns CTS (106) OFF;
        modem turns DSR (107) ON;
      else:
        modem responds with "CFI <parameter>" ("call failure indication").
  . To clear a call, the DTE turns DTR (108) OFF.
  . There is no mention of the Carrier Detect circuit (109) in the standard.
  . There is no provision for "escaping back" to the modem's command mode.

  It is not known whether there exists in real life a pure V.25bis modem.
  If there is, this code has never been tested on it.  See the Digitel entry.
*/
    {
    "Any CCITT / ITU-T V.25bis conformant modem",
    "",			/* pulse command */
    "",			/* tone command */
    40,			/* dial_time -- programmable -- */
    ",:",		/* pause_chars -- "," waits for programmable time */
                        /* ":" waits for dial tone */
    10,			/* pause_time (seconds, just a guess) */
    "",			/* wake_str (none) */
    200,		/* wake_rate (msec) */
    "VAL",		/* wake_prompt */
    "",			/* dmode_str (none) */
    "",			/* dmode_prompt (none) */
    "CRN%s\015",        /* dial_str */
    200,		/* dial_rate (msec) */
    0,			/* No esc_time */
    0,			/* No esc_char  */
    "",			/* No hup_str  */
    "",			/* hwfc_str */
    "",			/* swfc_str */
    "",			/* nofc_str */
    "",			/* ec_on_str */
    "",			/* ec_off_str */
    "",			/* dc_on_str */
    "",			/* dc_off_str */
    "CIC\015",		/* aa_on_str */
    "DIC\015",		/* aa_off_str */
    "",			/* sb_on_str */
    "",			/* sb_off_str */
    "",			/* sp_off_str */
    "",			/* sp_on_str */
    "",			/* vol1_str */
    "",			/* vol2_str */
    "",			/* vol3_str */
    "",			/* ignoredt */
    "",			/* ini2 */
    0L,			/* max_speed */
    CKD_V25,		/* capas */
    NULL		/* No ok_fn    */
};

static
MDMINF HAYES =				/* Hayes 2400 and compatible modems */
    {
    "Hayes Smartmodem 2400 and compatibles",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATE1Q0V1&S0&C1&D2\015",		/* wake_str */
#else
#ifdef VMS
    "ATQ0&S1\015",			/* wake_str */
#else
    "ATQ0\015",				/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str, user supplies D or T */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    2400L,				/* max_speed */
    CKD_AT,				/* capas */
    getok				/* ok_fn */
};

/*
  The intent of the "unknown" modem is to allow KERMIT to support
  unknown modems by having the user type the entire autodial sequence
  (possibly including control characters, etc.) as the "phone number".
  The protocol and other characteristics of this modem are unknown, with
  some "reasonable" values being chosen for some of them.  The only way to
  detect if a connection is made is to look for carrier.
*/
static
MDMINF UNKNOWN =			/* Information for "Unknown" modem */
    {
    "Unknown",				/* name */
    "",					/* pulse command */
    "",					/* tone command */
    30,					/* dial_time */
    "",					/* pause_chars */
    0,					/* pause_time */
    "",					/* wake_str */
    0,					/* wake_rate */
    "",					/* wake_prompt */
    "",					/* dmode_str */
    NULL,				/* dmode_prompt */
    "%s\015",				/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
    "",					/* ignoredt */
    "",					/* ini2 */
    0L,					/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

#ifndef MINIDIAL
static
MDMINF ATTISN =				/* AT&T ISN Network */
    {
    "",					/* pulse command */
    "",					/* tone command */
    "AT&T ISN Network",
    30,					/* Dial time */
    "",					/* Pause characters */
    0,					/* Pause time */
    "\015\015\015\015",			/* Wake string */
    900,				/* Wake rate */
    "DIAL",				/* Wake prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "%s\015",				/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
    "",					/* ignoredt */
    "",					/* ini2 */
    0L,					/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF ATTMODEM =	/* information for AT&T switched-network modems */
			/* "Number" following "dial" can include: p's and
			 * t's to indicate pulse or tone (default) dialing,
			 * + for wait for dial tone, , for pause, r for
			 * last number dialed, and, except for 2224B, some
			 * comma-delimited options like o12=y, before number.

 * "Important" options for the modems:
 *
 *	All:		Except for 2224B, enable option 12 for "transparent
 *			data," o12=y.  If a computer port used for both
 *			incoming and outgoing calls is connected to the
 *			modem, disable "enter interactive mode on carriage
 *			return," EICR.  The Kermit "dial" command can
 *			function with EIA leads standard, EIAS.
 *
 *	2212C:		Internal hardware switches at their default
 *			positions (four rockers down away from numbers)
 *			unless EICR is not wanted (rocker down at the 4).
 *			For EIAS, rocker down at the 1.
 *
 *	2224B:		Front-panel switch position 1 must be up (at the 1,
 *			closed).  Disable EICR with position 2 down.
 *			For EIAS, position 4 down.
 *			All switches on the back panel down.
 *
 *	2224CEO:	All front-panel switches down except either 5 or 6.
 *			Enable interactive flow control with o16=y.
 *			Select normal asynchronous mode with o34=0 (zero).
 *			Disable EICR with position 3 up.  For EIAS, 1 up.
 *			Reset the modem after changing switches.
 *
 *	2296A:		If option 00 (zeros) is present, use o00=0.
 *			Enable interactive flow control with o16=y.
 *			Select normal asynchronous mode with o34=0 (zero).
 *                      (available in Microcom Networking version, but
 *                      not necessarily other models of the 2296A).
 *			Enable modem-port flow control (if available) with
 * 			o42=y.  Enable asynchronous operation with o50=y.
 * 			Disable EICR with o69=n.  For EIAS, o66=n, using
 * 			front panel.
 */
    {
   "AT&T switched-network modems",
    "",					/* pulse command */
    "",					/* tone command */
    20,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
    "+",				/* wake_str */
    0,					/* wake_rate */
    "",					/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "at%s\015",				/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
    "",					/* ignoredt */
    "",					/* ini2 */
    0L,					/* max_speed */
    CKD_AT,				/* capas */
    NULL				/* ok_fn */
};

static
MDMINF ATTDTDM = /* AT&T Digital Terminal Data Module  */
		 /* For dialing: KYBD switch down, others usually up. */
    {
    "AT&T Digital Terminal Data Module",
    "",					/* pulse command */
    "",					/* tone command */
    20,					/* dial_time */
    "",					/* pause_chars */
    0,					/* pause_time */
    "",					/* wake_str */
    0,					/* wake_rate */
    "",					/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "%s\015",				/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
    "",					/* ignoredt */
    "",					/* ini2 */
    0L,					/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF DIGITEL =        /* Digitel DT-22 CCITT variant used in Brazil */
/*
  Attempts to adhere strictly to the V.25bis specification do not produce good
  results in real life.  The modem for which this code was developed: (a)
  ignores parity; (b) sometimes terminates responses with LF CR instead of CR
  LF; (c) has a Hayes-like escape sequence; (d) supports a hangup ("HUP")
  command.  Information from Fernando Cabral in Brasilia.
*/
    {
    "Digitel DT-22 CCITT dialer",
    "",				/* pulse command */
    "",				/* tone command */
    40,				/* dial_time -- programmable -- */
    ",:",		/* pause_chars -- "," waits for programmable time */
                        /* ":" waits for dial tone */
    10,			/* pause_time (seconds, just a guess) */
    "HUP\015",          /* wake_str (Not Standard CCITT) */
    200,		/* wake_rate (msec) */
    "VAL",		/* wake_prompt */
    "",			/* dmode_str (none) */
    "",			/* dmode_prompt (none) */
    "CRN%s\015",        /* dial_str */
    200,		/* dial_rate (msec) */
    1100,		/* esc_time (Not Standard CCITT) */
    43,			/* esc_char  (Not Standard CCITT) */
    "HUP\015",		/* hup_str  (Not Standard CCITT) */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "CIC\015",				/* aa_on_str */
    "DIC\015",				/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
    "",					/* ignoredt */
    "",					/* ini2 */
    0L,					/* max_speed */
    CKD_V25,				/* capas */
    getok				/* ok_fn */
};

static
MDMINF H_1200 =		/* Hayes 1200 and compatible modems */
    {
    "Hayes Smartmodem 1200 and compatibles",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATE1Q0V1\015",			/* wake_str */
#else
    "ATQ0\015",				/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "",					/* ignoredt */
    "",					/* ini2 */
    1200L,				/* max_speed */
    CKD_AT,				/* capas */
    getok				/* ok_fn */
};

static
MDMINF H_ULTRA =			/* Hayes high-speed */
    {
    "Hayes Ultra/Optima/Accura 96/144/288", /* U,O,A */
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATE1Q0V1X4N1Y0&S0&C1&D2S37=0S82=128\015", /* wake_str */
#else
#ifdef VMS
    "ATQ0X4N1Y0&S1S37=0S82=128\015",	/* wake_str */
#else
    "ATQ0X4N1Y0S37=0S82=128\015",	/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */   /* OK for U,O */
    "AT&K4\015",			/* swfc_str */   /* OK for U,O */
    "AT&K0\015",			/* nofc_str */   /* OK for U,O */
    "AT&Q5S36=7S48=7\015",		/* ec_on_str */  /* OK for U,O */
    "AT&Q0\015",			/* ec_off_str */ /* OK for U,O */
    "ATS46=2\015",			/* dc_on_str */
    "ATS46=0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    115200L,				/* max_speed */  /* (varies) */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF H_ACCURA =			/* Hayes Accura */
    {					/* GUESSING IT'S LIKE ULTRA & OPTIMA */
    "Hayes Accura",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATE1Q0V1X4N1Y0&S0&C1&D2S37=0\015",	/* wake_str */
#else
#ifdef VMS
    "ATQ0X4N1Y0&S1S37=0\015",		/* wake_str */
#else
    "ATQ0X4N1Y0S37=0\015",		/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4\015",			/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT&Q5S36=7S48=7\015",		/* ec_on_str */
    "AT&Q0\015",			/* ec_off_str */
    "ATS46=2\015",			/* dc_on_str */
    "ATS46=0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    115200L,				/* max_speed */  /* (varies) */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF PPI =				/* Practical Peripherals  */
    {
    "Practical Peripherals V.22bis or higher with V.42 and V.42bis",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef COMMENT
/* In newer models S82 (BREAK handling) was eliminated, causing an error. */
#ifdef OS2
    "ATQ0X4N1&S0&C1&D2S37=0S82=128\015", /* wake_str */
#else
    "ATQ0X4N1S37=0S82=128\015",		/* wake_str */
#endif /* OS2 */
#else /* So now we use Y0 instead */
#ifdef OS2
    "ATE1Q0V1X4N1&S0&C1&D2Y0S37=0\015",	/* wake_str */
#else
#ifdef VMS
    "ATQ0X4N1Y0&S1S37=0\015",		/* wake_str */
#else
    "ATQ0X4N1Y0S37=0\015",		/* wake_str */
#endif /* VMS */
#endif /* OS2 */
#endif /* COMMENT */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4\015",			/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT&Q5S36=7S48=7\015",		/* ec_on_str */
    "AT&Q0S36=0S48=128\015",		/* ec_off_str */
    "ATS46=2\015",			/* dc_on_str */
    "ATS46=0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str  */
    "",					/* sb_off_str  */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF DATAPORT =			/* AT&T Dataport  */
    {
    "AT&T / Paradyne DataPort V.32 or higher",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
    /*
       Note: S41=0 (use highest modulation) omitted, since it is not
       supported on the V.32 and lower models.  So let's not touch it.
    */
#ifdef OS2
    "ATQ0E1V1X6&S0&C1&D2&Q0Y0\\K5S78=0\015", /* wake_str */
#else
#ifdef VMS
    "ATQ0E1X6&S1&Q0Y0\\K5S78=0\015",	/* wake_str */
#else
    "ATQ0E1X6&Q0Y0\\K5S78=0\015",		/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT\\Q3\015",			/* hwfc_str */
    "AT\\Q1\\X0\015",			/* swfc_str */
    "AT\\Q0\015",			/* nofc_str */
    "AT\\N7\015",			/* ec_on_str */
    "AT\\N0\015",			/* ec_off_str */
    "AT%C1\015",			/* dc_on_str */
    "AT%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF UCOM_AT =			/* Microcom DeskPorte FAST ES 28.8 */
    {
    "Microcom DeskPorte FAST 28.8",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATE1Q0V1X4\\N0F0&S0&C1&D2\\K5\015", /* wake_str */
#else
#ifdef VMS
    "ATQ0X4F0&S1\\K5\015",		/* wake_str */
#else
    "ATQ0X4F0\\K5\015",			/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT\\Q3\015",			/* hwfc_str */
    "AT\\Q1\015",			/* swfc_str */
    "AT\\H0\\Q0\015",			/* nofc_str */
    "AT\\N3\015",			/* ec_on_str */
    "AT\\N0\015",			/* ec_off_str */
    "AT%C3\015",			/* dc_on_str */
    "AT%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "AT-J0\015",			/* sb_on_str */
    "AT-J1\015",			/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF ZOOM =				/* Zoom Telephonics V.32bis  */
    {
    "Zoom Telephonics V.32bis",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATE1Q0V1N1W1X4&S0&C1&D2S82=128S95=47\015", /* wake_str */
#else
#ifdef VMS
    "ATQ0E1N1W1X4&S1S82=128S95=47\015",	/* wake_str */
#else
    "ATQ0E1N1W1X4S82=128S95=47\015",	/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4\015",			/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT&Q5S36=7S48=7\015",		/* ec_on_str */
    "AT&Q0\015",			/* ec_off_str */
    "ATS46=138\015",			/* dc_on_str */
    "ATS46=136\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF ZYXEL =				/* ZyXEL U-Series */
    {
    "ZyXEL U-Series V.32bis or higher",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATE1Q0V1&S0&C1&D2&N0X5&Y1\015",	/* wake_str */
#else
#ifdef VMS
    "ATQ0E1&S1&N0X5&Y1\015",		/* wake_str */
#else
    "ATQ0E1&N0X5&Y1\015",		/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&H3\015",			/* hwfc_str */
    "AT&H4\015",			/* swfc_str */
    "AT&H0\015",			/* nofc_str */
    "AT&K3\015",			/* ec_on_str */
    "AT&K0\015",			/* ec_off_str */
    "AT&K4\015",			/* dc_on_str */
    "AT&K3\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF ZOLTRIX =			/* Zoltrix */
    {
    "Zoltrix V.32bis and V.34 modems with Rockwell ACI chipset",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
   "ATE1Q0V1F0W1X4Y0&S0&C1&D2\\K5S82=128S95=41\015", /* wake_str */
#else
#ifdef VMS
   "ATQ0E1F0W1X4Y0&S1\\K5S82=128S95=41\015", /* wake_str */
#else
   "ATQ0E1F0W1X4Y0\\K5S82=128S95=41\015", /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4S32=17S33=19\015",		/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT\\N3\015",			/* ec_on_str */
    "AT\\N1\015",			/* ec_off_str */
    "ATS46=138%C3\015",			/* dc_on_str */
    "ATS46=136%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "AT\\N0\015",			/* sb_on_str */
    "AT&Q0\015",			/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF MOTOROLA = {			/* Motorola FasTalk II or Lifestyle */
/*
  "\E" and "\X" commands removed - Motorola Lifestyle doesn't have them.
     \E0 = Don't echo while online
     \X0 = Process Xon/Xoff but don't pass through
*/
    "Motorola FasTalk II or Lifestyle",	/* Name */
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATE1Q0V1X4&S0&C1&D2\\K5\\V1\015",	/* wake_str */
#else
#ifdef VMS
    "ATQ0E1X4&S1\\K5\\V1\015",		/* wake_str */
#else
    "ATQ0E1X4\\K5\\V1\015",		/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT\\Q3\015",			/* hwfc_str */
    "AT\\Q1\015",			/* swfc_str */
    "AT\\Q0\015",			/* nofc_str */
    "AT\\N6\015",			/* ec_on_str */
    "AT\\N1\015",			/* ec_off_str */
    "AT%C1\015",			/* dc_on_str */
    "AT%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "AT\\J0\015",			/* sb_on_str */
    "AT\\J1\015",			/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF BOCA =				/* Boca */
    {
    "BOCA 14.4 Faxmodem",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATE1Q0V1F1N1W1&S0&C1&D2\\K5S37=11S82=128S95=47X4\015", /* wake_str */
#else
#ifdef VMS
    "ATQ0E1F1N1W1&S1\\K5S37=11S82=128S95=47X4\015", /* wake_str */
#else
    "ATQ0E1F1N1W1\\K5S37=11S82=128S95=47X4\015", /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4\015",			/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT\\N3S36=7S48=7\015",		/* ec_on_str */
    "AT\\N1\015",			/* ec_off_str */
    "ATS46=138\015",			/* dc_on_str */
    "ATS46=136\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF INTEL =				/* Intel */
    {
    "Intel High-Speed Faxmodem",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATE1Q0V1Y0X4&S0&C1&D2\\K1\\V2S25=50\015", /* wake_str */
#else
#ifdef VMS
    "ATQ0E1Y0X4&S1\\K1\\V2S25=50\015",	/* wake_str */
#else
    "ATQ0E1Y0X4\\K1\\V2S25=50\015",	/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "ATB1+FCLASS=0\015",		/* dmode_str */
    "OK\015",				/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT\\G1\\Q3\015",			/* hwfc_str */
    "AT\\G1\\Q1\\X0\015",		/* swfc_str */
    "AT\\G0\015",			/* nofc_str */
    "AT\\J0\\N3\"H3\015",		/* ec_on_str */
    "AT\\N1\015",			/* ec_off_str */
    "AT%C1\015",			/* dc_on_str */
    "AT%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF MULTITECH =			/* Multitech */
    {
    "Multitech MT1432 or MT2834 Series",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
/* #P0 (= no parity) is not listed in the manual for newer models */
/* so it has been removed from all three copies of the Multitech wake_str */
#ifdef OS2
    "ATE1Q0V1X4&S0&C1&D2&E8&Q0\015",	/* wake_str */
#else
#ifdef VMS
    "ATQ0E1X4&S1&E8&Q0\015",		/* wake_str */
#else
    "ATQ0E1X4&E8&Q0\015",		/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&E4&E7&E8&E11&E13\015",		/* hwfc_str */
    "AT&E5&E6&E8&E11&E13\015",		/* swfc_str */
    "AT&E3&E7&E8&E10&E12\015",		/* nofc_str */
    "AT&E1\015",			/* ec_on_str */
    "AT&E0\015",			/* ec_off_str */
    "AT&E15\015",			/* dc_on_str */
    "AT&E14\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "AT$BA0\015",			/* sb_on_str (= "baud adjust off") */
    "AT$BA1\015",			/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF SUPRA =				/* Supra */
    {
    "SupraFAXModem 144 or 288",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1N1W0X4Y0&S0&C1&D2\\K5S82=128\015", /* wake_str */
#else
#ifdef VMS
    "ATQ0E1N1W0X4Y0&S1\\K5S82=128\015",	/* wake_str */
#else
    "ATQ0E1N1W0X4Y0\\K5S82=128\015",	/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4\015",			/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT&Q5\\N3S48=7\015",		/* ec_on_str */
    "AT&Q0\\N1\015",			/* ec_off_str */
    "AT%C1S46=138\015",			/* dc_on_str */
    "AT%C0S46=136\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM\015",				/* sp_off_str */
    "ATL\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF SUPRAX =				/* Supra Express */
    {
    "Diamond Supra Express V.90",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1W0X4&C1&D2&S0\\K5\015",	/* wake_str */
#else
#ifdef VMS
    "ATQ0E1W0X4&S1\\K5\015",		/* wake_str */
#else
    "ATQ0E1W0X4\\K5\015",		/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4\015",			/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT\\N3\015",			/* ec_on_str */
    "AT\\N1\015",			/* ec_off_str */
    "AT%C2\015",			/* dc_on_str */
    "AT%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM\015",				/* sp_off_str */
    "ATL\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    230400L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF MAXTECH =			/* MaxTech */
    {
    "MaxTech XM288EA or GVC FAXModem",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4Y0&S0&C1&D2&L0&M0\\K5\015", /* wake_str */
#else
#ifdef VMS
    "ATQ0E1X4Y0&L0&M0&S1\\K5\015",	/* wake_str */
#else
    "ATQ0E1X4Y0&L0&M0\\K5\015",		/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT\\Q3\015",			/* hwfc_str */
    "AT\\Q1\\X0\015",			/* swfc_str */
    "AT\\Q0\015",			/* nofc_str */
    "AT\\N6\015",			/* ec_on_str */
    "AT\\N0\015",			/* ec_off_str */
    "AT\\N6%C1\015",			/* dc_on_str */
    "AT\\N6%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF ROLM =		/* IBM / Siemens / Rolm 8000, 9000, 9751 CBX DCM */
    {
    "IBM/Siemens/Rolm CBX Data Communications Module",
    "",					/* pulse command */
    "",					/* tone command */
    60,					/* dial_time */
    "",					/* pause_chars */
    0,					/* pause_time */
    "\015\015",				/* wake_str */
    50,					/* wake_rate */
    "MODIFY?",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "CALL %s\015",			/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
    "",					/* ignoredt */
    "",					/* ini2 */
    19200L,				/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF USR =				/* USR Courier and Sportster modems */
    {
    "US Robotics Courier, Sportster, or compatible",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4&A3&S0&C1&D2&N0&Y3S14=0\015", /* wake_str */
#else
#ifdef SUNOS4
    "ATQ0X4&A3&S0&N0&Y3S14=0\015",	/* wake_str -- needs &S0 in SunOS */
#else
#ifdef VMS
    "ATQ0X4&A3&S1&N0&Y3S14=0\015",	/* wake_str -- needs &S1 in VMS */
#else
    "ATQ0X4&A3&N0&Y3S14=0\015",		/* wake_str */
#endif /* VMS */
#endif /* SUNOS4 */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&H1&R2&I0\015",			/* hwfc_str */
    "AT&H2&R1&I2\015",			/* swfc_str */
    "AT&H0&R1&I0\015",			/* nofc_str */
    "AT&M4&B1\015",			/* ec_on_str */
    "AT&M0\015",			/* ec_off_str */
    "AT&K1\015",			/* dc_on_str */
    "AT&K0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};


static
MDMINF USRX2 =				/* USR XJ-CC1560 X2 56K */
    {
    "US Robotics / Megahertz CC/XJ-CC1560 X2",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4&A3&S0&B2&C1&D2&N0\015",	/* wake_str */
#else
#ifdef VMS
    "ATQ0X4&A3&B2&N0&S1\015",		/* wake_str */
#else
    "ATQ0X4&A3&B2&N0\015",		/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&H1&I0\015",			/* hwfc_str */
    "AT&H2&I2\015",			/* swfc_str */
    "AT&H0&I0\015",			/* nofc_str */
    "AT&M4\015",			/* ec_on_str */
    "AT&M0\015",			/* ec_off_str */
    "AT&K1\015",			/* dc_on_str */
    "AT&K0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "AT&B1\015",			/* sb_on_str */
    "AT&B0\015",			/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF OLDTB =				/* Old Telebits */
    {
    "Telebit TrailBlazer, T1000, T1500, T2000, T2500",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    60,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "\021AAAAATQ0E1V1X1&S0&C1&D2S12=50S50=0S54=3\015", /* wake_str. */
#else
#ifdef VMS
    "\021AAAAATQ0X1S12=50S50=0S54=3\015", /* wake_str. */
#else
    "\021AAAAATQ0X1&S1S12=50S50=0S54=3\015", /* wake_str. */
#endif /* VMS */
#endif /* OS2 */
    100,				/* wake_rate = 100 msec */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str, Note: no T or P */
    80,					/* dial_rate */
    1100,				/* esc_time (guard time) */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "ATS58=2S68=2\015",			/* hwfc_str */
    "ATS58=3S68=3S69=0\015",		/* swfc_str */
    "ATS58=0S68=0\015",			/* nofc_str */
    "ATS66=1S95=2\015",			/* ec_on_str */
    "ATS95=0\015",			/* ec_off_str */
    "ATS110=1S96=1\015",		/* dc_on_str */
    "ATS110=0S96=0\015",		/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    19200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW|CKD_TB|CKD_KS, /* capas */
    getok				/* ok_fn */
};

static
MDMINF NEWTB =				/* New Telebits */
    {
    "Telebit T1600, T3000, QBlazer, WorldBlazer, etc.",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    60,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "\021AAAAATQ0E1V1X2&S0&C1&D2S12=50S50=0S61=0S63=0\015", /* wake_str. */
#else
#ifdef VMS
    "\021AAAAATQ0X2&S1S12=50S50=0S61=0S63=0\015", /* wake_str. */
#else
    "\021AAAAATQ0X2S12=50S50=0S61=0S63=0\015", /* wake_str. */
#endif /* VMS */
#endif /* OS2 */
    100,				/* wake_rate = 100 msec */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str, Note: no T or P */
    80,					/* dial_rate */
    1100,				/* esc_time (guard time) */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "ATS58=2S68=2\015",			/* hwfc_str */
    "ATS58=3S68=3\015",			/* swfc_str */
    "ATS58=0S68=0\015",			/* nofc_str */
    "ATS180=3\015",			/* ec_on_str */
    "ATS180=0\015",			/* ec_off_str */
    "ATS190=1\015",			/* dc_on_str */
    "ATS190=0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    38400L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW|CKD_TB|CKD_KS, /* capas */
    getok				/* ok_fn */
};
#endif /* MINIDIAL */

static
MDMINF DUMMY = /* dummy information for modems that are handled elsewhere */
    {
    "(dummy)",
    "",					/* pulse command */
    "",					/* tone command */
    30,					/* dial_time */
    "",					/* pause_chars */
    0,					/* pause_time */
    "",					/* wake_str */
    0,					/* wake_rate */
    "",					/* wake_prompt */
    "",					/* dmode_str */
    NULL,				/* dmode_prompt */
    "%s\015",				/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
    "",					/* ignoredt */
    "",					/* ini2 */
    0L,					/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

#ifndef MINIDIAL
static
MDMINF RWV32 =				/* Generic Rockwell V.32 */
    {
    "Generic Rockwell V.32 modem",	/* ATI3, ATI4, and ATI6 for details */
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4Y0&S0&C1&D2%E2\\K5+FCLASS=0N1S37=0\015", /* wake_str */
#else
#ifdef VMS
    "ATQ0X4Y0&S1%E2\\K5+FCLASS=0N1S37=0\015", /* wake_str */
#else
    "ATQ0X4Y0%E2\\K5+FCLASS=0N1S37=0\015", /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4S32=17S33=19\015",		/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT&Q6\015",			/* ec_on_str */
    "AT&Q0\015",			/* ec_off_str */
    "AT%C1\015",			/* dc_on_str */
    "AT%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF RWV32B =				/* Generic Rockwell V.32bis */
    {
    "Generic Rockwell V.32bis modem",	/* ATI3, ATI4, and ATI6 for details */
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4Y0&S0&C1&D2%E2\\K5+FCLASS=0N1S37=0\015", /* wake_str */
#else
#ifdef VMS
    "ATQ0X4Y0&S1%E2\\K5+FCLASS=0N1S37=0\015", /* wake_str */
#else
    "ATQ0X4Y0%E2\\K5+FCLASS=0N1S37=0\015", /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4S32=17S33=19\015",		/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT&Q5\015",			/* ec_on_str */
    "AT&Q0\015",			/* ec_off_str */
    "ATS%C1\015",			/* dc_on_str */
    "ATS%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF RWV34 =				/* Generic Rockwell V.34 Data/Fax */
    {
    "Generic Rockwell V.34 modem",	/* ATI3, ATI4, and ATI6 for details */
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0V1X4Y0&C1&D2&S0%E2\\K5+FCLASS=0\015", /* wake_str */
#else
#ifdef VMS
    "ATQ0V1X4Y0&C1&D2&S1%E2\\K5+FCLASS=0\015", /* wake_str */
#else
    "ATQ0V1X4Y0&C1&D2%E2\\K5+FCLASS=0\015", /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4S32=17S33=19\015",		/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT&Q5\015",			/* ec_on_str */
    "AT&Q0\015",			/* ec_off_str */
    "ATS%C3\015",			/* dc_on_str */
    "ATS%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF RWV90 =				/* Generic Rockwell V.90 Data/Fax */
    {
    "Generic Rockwell V.90 56K modem",	/* ATI3, ATI4, and ATI6 for details */
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0V1N1X4Y0&C1&D2&S0%E2\\K5+FCLASS=0S37=0\015", /* K95 */
#else
#ifdef VMS
    "ATQ0V1N1X4Y0&C1&D2&S1%E2\\K5+FCLASS=0S37=0\015", /* wake_str */
#else
    "ATQ0V1N1X4Y0&C1&D2%E2\\K5+FCLASS=0S37=0\015", /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4S32=17S33=19\015",		/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT&Q5\015",			/* ec_on_str */
    "AT&Q0\015",			/* ec_off_str */
    "AT%C3\015",			/* dc_on_str */
    "AT%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF MWAVE =				/* IBM Mwave */
    {
    "IBM Mwave Adapter",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4Y0&S0&C1&D2&M0&Q0&N1\\K3\\T0%E2S28=0\015", /* wake_str */
#else
#ifdef VMS
    "ATQ0X4Y0&M0&S1&Q0&N1&S0\\K3\\T0%E2S28=0\015", /* wake_str */
#else
    "ATQ0X4Y0&M0&Q0&N1&S0\\K3\\T0%E2S28=0\015", /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT\\Q3\015",			/* hwfc_str */
    "",					/* swfc_str (it doesn't!) */
    "AT\\Q0\015",			/* nofc_str */
    "AT\\N7\015",			/* ec_on_str */
    "AT\\N0\015",			/* ec_off_str */
    "AT%C1\"H3\015",			/* dc_on_str */
    "AT%C0\"H0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF TELEPATH =			/* Gateway 2000 Telepath */
    {
    "Gateway 2000 Telepath II 28.8",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4&S0&C1&D2&N0&Y2#CLS=0S13=0S15=0S19=0\015", /* wake_str */
#else
#ifdef VMS
    "ATQ0X4&N0&S1&Y1#CLS=0S13=0S15=0S19=0\015", /* wake_str */
#else
    "ATQ0X4&N0&Y1#CLS=0S13=0S15=0S19=0\015", /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&H1&R2\015",			/* hwfc_str */
    "AT&H2&I2S22=17S23=19\015",		/* swfc_str */
    "AT&H0&I0&R1\015",			/* nofc_str */
    "AT&M4&B1\015",			/* ec_on_str -- also fixes speed */
    "AT&M0\015",			/* ec_off_str */
    "AT&K1\015",			/* dc_on_str */
    "AT&K0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF CARDINAL =			/* Cardinal - based on Rockwell V.34 */
    {
    "Cardinal MVP288X Series",		/* ATI3, ATI4, and ATI6 for details */
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4W1Y0%E2&S0&C1&D2\\K5+FCLASS=0+MS=11,1\015", /* wake_str */
#else
#ifdef VMS
    "ATQ0X4W1Y0&S1%E2\\K5+FCLASS=0+MS=11,1\015", /* wake_str */
#else
    "ATQ0X4W1Y0%E2\\K5+FCLASS=0+MS=11,1\015", /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4S32=17S33=19\015",		/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT&Q5S36=7S48=7\\N3\015",		/* ec_on_str */
    "AT&Q0S48=128\\N1\015",		/* ec_off_str */
    "ATS46=138%C1\015",			/* dc_on_str */
    "ATS46=136%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF LUCENT =				/* Lucent Venus or Data/Fax modem */
    {
    "Lucent Venus chipset",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0V1N1X4Y0&C1&D2&S0%E2\\K5+FCLASS=0S37=0\015", /* K95 */
#else
#ifdef VMS
    "ATQ0V1N1X4Y0&C1&D2&S1%E2\\K5+FCLASS=0S37=0\015", /* VMS */
#else
    "ATQ0V1N1X4Y0&C1&D2%E2\\K5+FCLASS=0S37=0\015", /* All others */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4S32=17S33=19\015",		/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT&Q5\015",			/* ec_on_str */
    "AT&Q0\015",			/* ec_off_str */
    "AT%C1\015",			/* dc_on_str */
    "AT%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF CONEXANT =			/* Conexant family */
    {
    "Conexant family of modems",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0V1X4&C1&D2&S0%E1+FCLASS=0\015", /* K95 */
#else
#ifdef VMS
    "ATQ0V1X4&C1&D2&S1%E1+FCLASS=0\015", /* VMS */
#else
    "ATQ0V1X4&C1&D2%E1+FCLASS=0\015", /* UNIX etc */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4S32=17S33=19\015",		/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT&Q5\015",			/* ec_on_str */
    "AT&Q0\015",			/* ec_off_str */
    "AT%C3\015",			/* dc_on_str */
    "AT%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF PCTEL =				/* PCTel chipset */
    {
    "PCTel chipset",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0V1N1X4Y0&C1&D2&S0%E2\\K5S37=0\015", /* K95 */
#else
#ifdef VMS
    "ATQ0V1N1X4Y0&C1&D2&S1%E2\\K5S37=0\015", /* VMS */
#else
    "ATQ0V1N1X4Y0&C1&D2%E2\\K5S37=0\015", /* UNIX etc */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4S32=17S33=19\015",		/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT\\N3\015",			/* ec_on_str */
    "AT\\N0\015",			/* ec_off_str */
    "AT%C1\015",			/* dc_on_str */
    "AT%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF ZOOMV34 =			/* Zoom Telephonics V.34  */
    {
    "Zoom Telephonics V.34",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0V1N1W1X4&S0&C1&D2S82=128\015", /* wake_str */
#else
#ifdef VMS
    "ATQ0V1N1W1X4&S1S82=128\015",	/* wake_str */
#else
    "ATQ0V1N1W1X4S82=128S015",		/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4\015S32=17S33=19",		/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT&Q5\015",			/* ec_on_str */
    "AT&Q0\015",			/* ec_off_str */
    "ATS%C3\015",			/* dc_on_str */
    "ATS%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF ZOOMV90 =			/* ZOOM V.90 */
    {
    "Zoom V.90 56K",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0V1N1X4Y0&C1&D2&S0%E2\\K5+FCLASS=0S37=0\015", /* K95 */
#else
#ifdef VMS
    "ATQ0V1N1X4Y0&C1&D2&S1%E2\\K5+FCLASS=0S37=0\015", /* VMS */
#else
    "ATQ0V1N1X4Y0&C1&D2%E2\\K5+FCLASS=0S37=0\015", /* All others */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4S32=17S33=19\015",		/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT&Q5\015",			/* ec_on_str */
    "AT&Q0\015",			/* ec_off_str */
    "AT%C1\015",			/* dc_on_str */
    "AT%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF ZOOMV92 =			/* ZOOM V.92 */
    {
    "Zoom V.92 with V.44 compression",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0V1N1X4Y0&C1&D2&S0%E2\\K5+FCLASS=0S37=0+MS=V92\015", /* K95 */
#else
#ifdef VMS
    "ATQ0V1N1X4Y0&C1&D2&S1%E2\\K5+FCLASS=0S37=0+MS=V92\015", /* VMS */
#else
    "ATQ0V1N1X4Y0&C1&D2%E2\\K5+FCLASS=0S37=0+MS=V92\015", /* All others */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4S32=17S33=19\015",		/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT&Q5\015",			/* ec_on_str */
    "AT&Q0\015",			/* ec_off_str */
    "AT%C1+DCS=1,1\015",		/* dc_on_str */
    "AT%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};


/*
  Now the "old" modems, all grouped together, and also within
  "if not defined MINIDIAL"...
*/
#ifdef OLDMODEMS

static
MDMINF CERMETEK =	/* Information for "Cermetek Info-Mate 212 A" modem */
    {
    "Cermetek Info-Mate 212 A",
    "",					/* pulse command */
    "",					/* tone command */
    20,					/* dial_time */
    "BbPpTt",				/* pause_chars */
    0,					/* pause_time */
    "  XY\016R\015",			/* wake_str */
    200,				/* wake_rate */
    "",					/* wake_prompt */
    "",					/* dmode_str */
    NULL,				/* dmode_prompt */
    "\016D '%s'\015",			/* dial_str */
    200,				/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
    "",					/* ignoredt */
    "",					/* ini2 */
    1200L,				/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF DF03 =		/* information for "DEC DF03-AC" modem */
    {
    "Digital DF03-AC",
    "",					/* pulse command */
    "",					/* tone command */
    27,					/* dial_time */
    "=",				/* pause_chars */
    15,					/* pause_time */
    "\001\002",				/* wake_str */
    0,					/* wake_rate */
    "",					/* wake_prompt */
    "",					/* dmode_str */
    NULL,				/* dmode_prompt */
    "%s",				/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
    "",					/* ignoredt */
    "",					/* ini2 */
    0L,					/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF DF100 =		/* information for "DEC DF100-series" modem */
			/*
			 * The telephone "number" can include "P"s and/or "T"s
			 * within it to indicate that subsequent digits are
			 * to be dialed using pulse or tone dialing.  The
			 * modem defaults to pulse dialing.  You may modify
			 * the dial string below to explicitly default all
			 * dialing to pulse or tone, but doing so prevents
			 * the use of phone numbers that you may have stored
			 * in the modem's memory.
			 */
    {
    "Digital DF-100",
    "",					/* pulse command */
    "",					/* tone command */
    30,					/* dial_time */
    "=",				/* pause_chars */
    15,					/* pause_time */
    "\001",				/* wake_str */
    0,					/* wake_rate */
    "",					/* wake_prompt */
    "",					/* dmode_str */
    NULL,				/* dmode_prompt */
    "%s#",				/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
    "",					/* ignoredt */
    "",					/* ini2 */
    0L,					/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF DF200 =		/* information for "DEC DF200-series" modem */
			/*
			 * The telephone "number" can include "P"s and/or "T"s
			 * within it to indicate that subsequent digits are
			 * to be dialed using pulse or tone dialing.  The
			 * modem defaults to pulse dialing.  You may modify
			 * the dial string below to explicitly default all
			 * dialing to pulse or tone, but doing so prevents
			 * the use of phone numbers that you may have stored
			 * in the modem's memory.
			 */
    {
    "Digital DF-200",
    "",			/* pulse command */
    "",			/* tone command */
    30,			/* dial_time */
    "=W",		/* pause_chars */	/* =: second tone; W: 5 secs */
    15,			/* pause_time */	/* worst case */
    "\002",		/* wake_str */		/* allow stored number usage */
    0,			/* wake_rate */
    "",			/* wake_prompt */
    "",			/* dmode_str */
    NULL,		/* dmode_prompt */
#ifdef COMMENT
    "%s!",		/* dial_str */
#else
    "   d %s\015",
#endif /* COMMENT */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
    "",					/* ignoredt */
    "",					/* ini2 */
    0L,					/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF GDC =		/* information for "GeneralDataComm 212A/ED" modem */
    {
    "GeneralDataComm 212A/ED",
    "",					/* pulse command */
    "",					/* tone command */
    32,					/* dial_time */
    "%",				/* pause_chars */
    3,					/* pause_time */
    "\015\015",				/* wake_str */
    500,				/* wake_rate */
    "$",				/* wake_prompt */
    "D\015",				/* dmode_str */
    ":",				/* dmode_prompt */
    "T%s\015",				/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
    "",					/* ignoredt */
    "",					/* ini2 */
    1200L,				/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF PENRIL =		/* information for "Penril" modem */
    {
    "Penril modem",
    "",					/* pulse command */
    "",					/* tone command */
    50,					/* dial_time */
    "",					/* pause_chars */
    0,					/* pause_time */
    "\015\015",				/* wake_str */
    300,				/* wake_rate */
    ">",				/* wake_prompt */
    "k\015",				/* dmode_str */
    ":",				/* dmode_prompt */
    "%s\015",				/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
    "",					/* ignoredt */
    "",					/* ini2 */
    0L,					/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF RACAL =				/* Racal Vadic VA4492E */
    {
    "Racal Vadic VA4492E",
    "",					/* pulse command */
    "",					/* tone command */
    35,			/* dial_time (manual says modem is hardwired to 60) */
    "Kk",				/* pause_chars */
    5,					/* pause_time */
    "\005\015",				/* wake_str, ^E^M */
    50,					/* wake_rate */
    "*",				/* wake_prompt */
    "D\015",				/* dmode_str */
    "?",				/* dmode_prompt */
    "%s\015",				/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    5,					/* esc_char, ^E */
    "\003\004",				/* hup_str, ^C^D */
    0,					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
    "",					/* ignoredt */
    "",					/* ini2 */
    0L,					/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF VENTEL =				/* Information for Ven-Tel modem */
    {
    "Ven-Tel",
    "",					/* pulse command */
    "",					/* tone command */
    20,					/* dial_time */
    "%",				/* pause_chars */
    5,					/* pause_time */
    "\015\015\015",			/* wake_str */
    300,				/* wake_rate */
    "$",				/* wake_prompt */
    "K\015",				/* dmode_str (was "") */
    "Number to call: ",			/* dmode_prompt (was NULL) */
    "%s\015",				/* dial_str (was "<K%s\r>") */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
    "",					/* ignoredt */
    "",					/* ini2 */
    0L,					/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF CONCORD =	/* Info for Condor CDS 220 2400b modem */
    {
    "Concord Condor CDS 220 2400b",
    "",					/* pulse command */
    "",					/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
    "\015\015",				/* wake_str */
    20,					/* wake_rate */
    "CDS >",				/* wake_prompt */
    "",					/* dmode_str */
    NULL,				/* dmode_prompt */
    "<D M%s\015>",			/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
    "",					/* ignoredt */
    "",					/* ini2 */
    2400L,				/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};
#endif /* OLDMODEMS */

static
MDMINF MICROCOM =	/* Microcom modems in native SX mode */
			/* (long answer only) */
{
    "Microcom MNP modems in SX command mode",
    "DP\015",				/* pulse command */
    "DT\015",				/* tone command */
    35,					/* dial_time */
    ",!@",		/* pause_chars (! and @ aren't pure pauses) */
    3,					/* pause_time */
/*
  The following sets 8 bits, no parity, BREAK passthru, and SE0 disables the
  escape character, which is a single character with no guard time, totally
  unsafe, so we have no choice but to disable it.  Especially since, by
  default, it is Ctrl-A, which is Kermit's packet-start character.  We would
  change it to something else, which would enable "mdmhup()", but the user
  wouldn't know about it.  Very bad.  Note: SE1 sets it to Ctrl-A, SE2
  sets it to Ctrl-B, etc (1..31 allowed).  Also SE/Q sets it to "Q".
*/
    "SE0;S1P4;SBRK5\015",		/* wake_str */
    100,				/* wake_rate */
    "!",				/* wake_prompt */
    "",					/* dmode_str */
    NULL,				/* dmode_prompt */
    "D%s\015",				/* dial_str - number up to 39 chars */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char - we can't use this */
    "",					/* hup_str - it's "H" but can't use */
    "SF13\015",				/* hwfc_str */
    "SF11\015",				/* swfc_str */
    "SF10\015",				/* nofc_str */
    "BAOFF;SMAUT\015",			/* ec_on_str */
    "BAON;SMDIR\015",			/* ec_off_str */
    "COMP1\015",			/* dc_on_str */
    "COMP0\015",			/* dc_off_str */
    "AA",				/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "SA2",				/* sp_off_str */
    "SA0",				/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
    "",					/* ignoredt */
    "",					/* ini2 */
    0L,					/* max_speed */
    CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW|CKD_KS, /* capas */
    getok				/* ok_fn */
};

static
MDMINF MICROLINK =			/* MicroLink ... */
    {					/* 14.4TQ,TL,PC;28.8TQ,TQV;2440T/TR */
    "ELSA MicroLink 14.4, 28.8, 33.6 or 56K", /* ELSA GmbH, Aachen */
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4&S0\\D0&C1&D2\\K5\015",	/* wake_str */
#else
#ifdef VMS
    "ATQ0X4&S1\\K5\015",		/* wake_str */
#else
    "ATQ0X4\\K5\015",			/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H\015",			/* hup_str */
    "AT\\Q3\015",			/* hwfc_str */
    "AT\\Q1\\X0\015",			/* swfc_str */
    "AT\\Q0\015",			/* nofc_str */
    "AT\\N3\015",			/* ec_on_str */
    "AT\\N0\015",			/* ec_off_str */
    "AT%C3\015",			/* dc_on_str */
    "AT%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "\\J0",				/* sb_on_str (?) */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF ULINKV250 =			/* MicroLink V.250 */
    {					/* 56Kflex, V.90; V.250 command set */
    "ELSA MicroLink 56K V.250",		/* ELSA GmbH, Aachen */
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    /* \D0 = DSR & CTS always on but hwfc overrides on CTS. */
    "ATQ0E1V1X4&S0\\D0&C1&D2\015",	/* wake_str */
#else
#ifdef VMS
    "ATQ0X4&S1\015",			/* wake_str */
#else
    "ATQ0X4\015",			/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT+IFC=2,2\015",			/* hwfc_str */
    "AT+IFC=1,1\015",			/* swfc_str */
    "AT+IFC=0,0\015",			/* nofc_str */
    "AT+ES=3,0\015",			/* ec_on_str */
    "AT+ES=1,0\015",			/* ec_off_str */
    "AT+DS=3,0,2048,32\015",		/* dc_on_str */
    "AT+DS=0,0\015",			/* dc_off_str */

    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str (?) */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};
#endif /* MINIDIAL */

static
MDMINF ITUTV250 =			/* ITU-T V.250 conforming modem */
{
    "Any ITU-T V.25ter/V.250 conformant modem",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
    "ATQ0E1V1X4&C1&D2\015",		/* wake_str (no &Sn in V.25) */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT+IFC=2,2\015",			/* hwfc_str */
    "AT+IFC=1,1\015",			/* swfc_str */
    "AT+IFC=0,0\015",			/* nofc_str */
    "AT+ES=3,0,2;+EB=1,0,30\015",	/* ec_on_str */
    "AT+ES=0\015",			/* ec_off_str */
    "AT+DS=3,0\015",			/* dc_on_str */
    "AT+DS=0,0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

#ifndef CK_TAPI
static
#endif /* CK_TAPI */
MDMINF GENERIC =			/* Generic high speed ... */
    {
    "Generic high-speed AT command set",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
    "",					/* wake_str */
    0,					/* wake_rate */
    "",					/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_on_str */
    "",					/* sp_off_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    115200,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW, /* capas */
    getok				/* ok_fn */
};

#ifndef MINIDIAL
static
MDMINF XJACK =				/* Megahertz X-Jack */
    {
    "Megahertz X-Jack XJ3144 / CC6144",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4N1&C1&D2\\K5\015",	/* wake_str */
#else
    "ATQ0X4N1\\K5\015",			/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4\015",			/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT\\N3&Q5\015",			/* ec_on_str */
    "AT\\N1&Q0\015",			/* ec_off_str */
    "AT%C3\015",			/* dc_on_str */
    "AT%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF SPIRITII =			/* QuickComm Spirit II */
    {
    "QuickComm Spirit II",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
    "AT&F\015",				/* wake_str */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H\015",			/* hup_str */
    "AT*F3\015",			/* hwfc_str */
    "AT*F2\015",			/* swfc_str */
    "AT*F0\015",			/* nofc_str */
    "AT*E6\015",			/* ec_on_str */
    "AT*E0\015",			/* ec_off_str */
    "AT*E9\015",			/* dc_on_str */
    "AT*E0\015",			/* dc_off_str */
    "ATS0=2\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF MONTANA = {			/* Motorola Montana */
    "Motorola Montana",			/* Name */
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4&S0&C1&D2\\K5\\V1\015",	/* wake_str */
#else
#ifdef VMS
    "ATQ0E1X4&S1\\K5\\V1\015",		/* wake_str */
#else
    "ATQ0E1X4\\K5\\V1\015",		/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT\\Q3\015",			/* hwfc_str */
    "AT\\Q1\015",			/* swfc_str */
    "AT\\Q0\015",			/* nofc_str */
    "AT\\N4\015",			/* ec_on_str */
    "AT\\N1\015",			/* ec_off_str */
    "AT%C1\015",			/* dc_on_str */
    "AT%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "AT\\J0\015",			/* sb_on_str */
    "AT\\J1\015",			/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF COMPAQ = {			/* Compaq Data+Fax Modem */
    "Compaq Data+Fax Modem",		/* Name */
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4&S0&C1&D2\015",		/* wake_str */
#else
#ifdef VMS
    "ATQ0E1X4&S1\015",			/* wake_str */
#else
    "ATQ0E1X4\015",			/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT\\Q3\015",			/* hwfc_str (same as &K3) */
    "AT\\Q1\015",			/* swfc_str (same as &K4) */
    "AT\\Q0\015",			/* nofc_str (same as &K0) */
    "AT\\N3\015",			/* ec_on_str */
    "AT\\N0\015",			/* ec_off_str */
    "AT%C1\015",			/* dc_on_str */
    "AT%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "AT\\N3\015",			/* sb_on_str */
    "AT\\N1\015",			/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL0\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};


static
MDMINF FUJITSU = {			/* Fujitsu */
    "Fujitsu Fax/Modem Adapter",	/* Name */
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4&S0&C1&D2\\K5\\N3\015",	/* wake_str */
#else
#ifdef VMS
    "ATQ0E1X4&S1\\K5\\N3\015",		/* wake_str */
#else
    "ATQ0E1X4\\K5\\N3\015",		/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\\Q3\015",			/* hwfc_str */
    "AT&K4\\Q1\015",			/* swfc_str */
    "AT&K0\\Q0\015",			/* nofc_str */
    "AT\\N3\015",			/* ec_on_str */
    "AT\\N0\015",			/* ec_off_str */
    "AT%C1",				/* dc_on_str */
    "AT%C0",				/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "AT\\J0\015",			/* sb_on_str */
    "AT\\J1\015",			/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF MHZATT =				/* Megahertz AT&T V.34 */
    {
    "Megahertz AT&T V.34",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4N1&C1&D2\\K5\015",	/* wake_str */
#else
    "ATQ0X4N1\\K5\015",			/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4\015",			/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT\\N3\015",			/* ec_on_str */
    "AT\\N0\015",			/* ec_off_str */
    "AT%C1\"H3\015",			/* dc_on_str */
    "AT%C0\"H0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "AT\\J0\015",			/* sb_on_str */
    "AT\\J1\015",			/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF SUPRASON =			/* SupraSonic */
    {
    "Diamond SupraSonic 288V+",		/* Diamond Multimedia Systems Inc */
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1N1W0X4Y0&S0&C1&D2\015",	/* wake_str */
#else
#ifdef VMS
    "ATQ0E1N1W0X4Y0&S1\015",		/* wake_str */
#else
    "ATQ0E1N1W0X4Y0\015",		/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4\015",			/* swfc_str */
    "AT&K\015",				/* nofc_str */
    "AT&Q5\\N3S48=7\015",		/* ec_on_str */
    "AT&Q0\\N1\015",			/* ec_off_str */
    "AT%C3S46=138\015",			/* dc_on_str */
    "AT%C0S46=136\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM\015",				/* sp_off_str */
    "ATL\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF BESTDATA =			/* Best Data */
    {
    "Best Data Fax Modem",		/* Best Data Fax Modem */
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1N1W0X4Y0&S0&C1&D2\015",	/* wake_str */
#else
#ifdef VMS
    "ATQ0E1N1W0X4Y0&S1\015",		/* wake_str */
#else
    "ATQ0E1N1W0X4Y0\015",		/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4\015",			/* swfc_str */
    "AT&K\015",				/* nofc_str */
    "AT&Q6\\N3\015",			/* ec_on_str */
    "AT&Q0\\N1\015",			/* ec_off_str */
    "AT%C3\015",			/* dc_on_str */
    "AT%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "AT\\N3\015",			/* sb_on_str */
    "AT\\N0\015",			/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF ATT1900 =			/* AT&T Secure Data STU III 1900 */
    {
    "AT&T Secure Data STU III Model 1900", /* name */
    "",					/* pulse command */
    "",					/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4\015",			/* wake_str */
#else
    "ATQ0E1X4\015",			/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_on_str */
    "",					/* sp_off_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
    "",					/* ignoredt */
    "",					/* ini2 */
    9600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_HW,		/* capas */
    getok				/* ok_fn */
};

/*
  Experimentation showed that hardly any of the documented commands did
  anything other that print ERROR.  At first there was no communication at
  all at 9600 bps -- turns out the interface speed was stuck at 2400.
  ATS28=130 (given at 2400 bps) allowed it to work at 9600.
*/
static
MDMINF ATT1910 =			/* AT&T Secure Data STU III 1910 */
    {					/* Adds V.32bis, V.42, V.42bis */
    "AT&T Secure Data STU III Model 1910", /* name */

/* Believe it or not, "ATT" and "ATP" result in ERROR */

    "",					/* pulse command */
    "",					/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4\015",			/* wake_str */
#else
    "ATQ0E1X4\015",			/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
#ifdef COMMENT
/* These are evidently read-only registers */
    "ATS46=138S47=0\015",		/* ec_on_str */
    "ATS46=138S47=128\015",		/* ec_off_str */
    "ATS46=138S47=0\015",		/* dc_on_str */
    "ATS46=138S47=128\015",		/* dc_off_str */
#else
    "",
    "",
    "",
    "",
#endif /* COMMENT */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_on_str */
    "",					/* sp_off_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
    "",					/* ignoredt */
    "",					/* ini2 */
    9600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW,	/* capas */
    getok				/* ok_fn */
};

static
MDMINF KEEPINTOUCH =			/* AT&T KeepinTouch Card Modem */
    {
    "AT&T KeepinTouch V.32bis Card Modem", /* Name */
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
/* This used to include &C1&S0&D2+Q0 but that gives ERROR */
    "ATQ0E1V1X4&S0&C1&D2\\K5\015",	/* wake_str */
#else
#ifdef VMS
    "ATQ0E1X4&S1\\K5\015",		/* wake_str */
#else
    "ATQ0E1X4\\K5\015",			/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT\\Q3\015",			/* hwfc_str */
    "AT\\Q1\\X0\015",			/* swfc_str */
    "AT\\Q0\015",			/* nofc_str */
    "AT\\N3-J1\015",			/* ec_on_str */
    "AT\\N1\015",			/* ec_off_str */
    "AT%C3\"H3\015",			/* dc_on_str */
    "AT%C0\"H0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "ATN0\\J0\015",			/* sb_on_str */
    "ATN1\\J1\015",			/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF ROLM_AT =		/* Rolm data phone with AT command set */
    {
    "Rolm 244PC or 600 Series with AT Command Set",
    "",					/* pulse command */
    "",					/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATE1Q0V1\015",			/* wake_str */
#else
    "ATQ0\015",				/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATDT%s\015",			/* dial_str -- always Tone */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_on_str */
    "",					/* sp_off_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
    "",					/* ignoredt */
    "",					/* ini2 */
    19200L,				/* max_speed */
    CKD_AT,				/* capas */
    getok				/* ok_fn */
};

static
MDMINF ATLAS =				/* Atlas / Newcom ixfC 33.6 */
    {
    "Atlas / Newcom 33600ixfC Data/Fax Modem", /* Name */
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATZ0&FQ0V1&C1&D2\015",		/* wake_str */
#else
    "ATZ0&FQ0V1\015",			/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4\015",			/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT\"H3\015",			/* ec_on_str */
    "AT\"H0\015",			/* ec_off_str */
    "AT%C1\015",			/* dc_on_str */
    "AT%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "ATN0\\J0\015",			/* sb_on_str */
    "ATN1\\J1\015",			/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF CODEX = {			/* Motorola Codex */
    "Motorola Codex 326X Series",	/* Name - AT&V to see settings */
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    /* &M0=Async (not sync) */
    /* *MM0=Automatic modulation negotiation */
    /* *DE22=Automatic data rate */
    "ATZQ0E1V1X4Y0*DE22*MM0&C1&M0&S0&D2\015", /* wake_str */
#else
#ifdef VMS
    "ATZQ0E1V1X4Y0*DE22*MM0&C1&M0&S1\015", /* wake_str */
#else
    "ATZQ0E1V1X4Y0*DE22*MM0&C1&M0\015",	/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT*MF1*FL3\015",			/* hwfc_str */
    "AT*MF1*FL1\015",			/* swfc_str */
    "AT*MF0*FL0\015",			/* nofc_str */
    "AT*EC0*SM3*SC0\015",		/* ec_on_str */
    "AT*SM0\015",			/* ec_off_str */
    "AT*DC1\015",			/* dc_on_str */
    "AT*DC0\015",			/* dc_off_str */
    "AT*AA5S0=1\015",			/* aa_on_str */
    "AT*AA5S0=0\015",			/* aa_off_str */
    "AT*SC1\015",			/* sb_on_str */
    "AT*SC0\015",			/* sb_off_str */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3*BD2\015",			/* ignoredt */
    "",					/* ini2 */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF MT5634ZPX =			/* Multitech */
    {
    "Multitech MT5634ZPX",		/* name */
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATE1Q0V1X4&S0&C1&D2&Q0\015",	/* wake_str */
#else
#ifdef VMS
    "ATQ0E1X4&S1&Q0\015",		/* wake_str */
#else
    "ATQ0E1X4&Q0\015",			/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4\015",			/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT\\N3\015",			/* ec_on_str */
    "AT\\N1\015",			/* ec_off_str */
    "AT%C1\015",			/* dc_on_str */
    "AT%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "AT\\J0\015",			/* sb_on_str */
    "AT\\J1\015",			/* sb_off_str (NOT SUPPORTED) */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF MOTSM56 =			/* Motorola SM56 Chipset */
    {
    "Motorola SM56 V.90 chipset",	/* name */
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0V1X4&S0&C1&D2*MM16\015",	/* wake_str */
#else
#ifdef VMS
    "ATQ0V1X4&S1&C1&D2*MM16\015",	/* wake_str */
#else
    "ATQ0V1X4&C1&D2*MM16\015",		/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT\\Q3\015",			/* hwfc_str */
    "AT\\Q1\015",			/* swfc_str */
    "AT\\Q0\015",			/* nofc_str */
    "AT\\N7\015",			/* ec_on_str */
    "AT\\N1\015",			/* ec_off_str */
    "AT%C1\015",			/* dc_on_str */
    "AT%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "AT\\J0\015",			/* sb_on_str */
    "AT\\J1\015",			/* sb_off_str (NOT SUPPORTED) */
    "ATM1\015",				/* sp_on_str */
    "ATM0\015",				/* sp_off_str */
    "ATL1\015",				/* vol1_str */
    "ATL2\015",				/* vol2_str */
    "ATL3\015",				/* vol3_str */
    "ATX3\015",				/* ignoredt */
    "",					/* ini2 */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};
#endif /* MINIDIAL */

/* END MDMINF STRUCT DEFINITIONS */

/*
  Table to convert modem numbers to MDMINF struct pointers.
  The entries MUST be in ascending order by modem number, without any
  "gaps" in the numbers, and starting from one (1).
*/

MDMINF *modemp[] = {
#ifdef MINIDIAL
    NULL,				/*  0 */
    &CCITT,				/*  1 */
    &HAYES,				/*  2 */
    &UNKNOWN,				/*  3 */
    &DUMMY,				/*  4 */
    &GENERIC,				/*  5 */
    &ITUTV250				/*  6 */
#else  /* Not MINIDIAL */
    NULL,				/*  0 */
    &ATTDTDM,				/*  1 */
    &ATTISN,				/*  2 */
    &ATTMODEM,				/*  3 */
    &CCITT,				/*  4 */
#ifdef OLDMODEMS
    &CERMETEK,				/*  5 */
    &DF03,				/*  6 */
    &DF100,				/*  7 */
    &DF200,				/*  8 */
    &GDC,				/*  9 */
#else
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
#endif /* OLDMODEMS */
    &HAYES,				/* 10 */
#ifdef OLDMODEMS
    &PENRIL,				/* 11 */
    &RACAL,				/* 12 */
#else
    NULL,
    NULL,
#endif /* OLDMODEMS */
    &UNKNOWN,				/* 13 */
#ifdef OLDMODEMS
    &VENTEL,				/* 14 */
    &CONCORD,				/* 15 */
#else
    NULL,
    NULL,
#endif /* OLDMODEMS */
    &DUMMY,				/* 16 */
    &ROLM,				/* 17 */
#ifdef OLDMODEMS
    &MICROCOM,				/* 18 */
#else
    NULL,
#endif /* OLDMODEMS */
    &USR,				/* 19 USR Courier and Sportster */
    &OLDTB,				/* 20 Old Telebits */
    &DIGITEL,				/* 21 Digitel CCITT */
    &H_1200,				/* 22 Hayes 1200 */
    &H_ULTRA,				/* 23 Hayes Ultra */
    &H_ACCURA,				/* 24 Hayes Optima */
    &PPI,				/* 25 PPI */
    &DATAPORT,				/* 26 Dataport */
    &BOCA,				/* 27 Boca */
    &MOTOROLA,				/* 28 Motorola UDS MOTOROLA */
    NULL,				/* 29 Digicomm */
    NULL,				/* 30 Dynalink */
    &INTEL,				/* 31 Intel */
    &UCOM_AT,				/* 32 Microcom in AT mode */
    &MULTITECH,				/* 33 Multitech */
    &SUPRA,				/* 34 Supra */
    &ZOLTRIX,				/* 35 Zoltrix */
    &ZOOM,				/* 36 Zoom */
    &ZYXEL,				/* 37 ZyXEL */
    &DUMMY,				/* 38 TAPI */
    &NEWTB,				/* 39 New-Telebit */
    &MAXTECH,				/* 40 MaxTech */
    &DUMMY,				/* 41 User-defined */
    &RWV32,				/* 42 Rockwell V.32 */
    &RWV32B,				/* 43 Rockwell V.32bis */
    &RWV34,				/* 44 Rockwell V.34 */
    &MWAVE,				/* 45 IBM Mwave */
    &TELEPATH,				/* 46 Gateway 2000 Telepath II 28.8 */
    &MICROLINK,				/* 47 MicroLink modems */
    &CARDINAL,				/* 48 Cardinal */
    &GENERIC,				/* 49 Generic high-speed */
    &XJACK,				/* 50 Megahertz-Xjack */
    &SPIRITII,				/* 51 QuickComm Spirit II */
    &MONTANA,				/* 52 Motorola Montana */
    &COMPAQ,				/* 53 Compaq Data+Fax */
    &FUJITSU,				/* 54 Fujitsu */
    &MHZATT,				/* 55 Megahertz AT&T V.34 */
    &SUPRASON,				/* 56 Suprasonic */
    &BESTDATA,				/* 57 Best Data */
    &ATT1900,				/* 58 AT&T Secure Data STU III 1900 */
    &ATT1910,				/* 59 AT&T Secure Data STU III 1910 */
    &KEEPINTOUCH,			/* 60 AT&T KeepinTouch */
    &USRX2,				/* 61 USR XJ-1560 X2 */
    &ROLM_AT,				/* 62 Rolm with AT command set */
    &ATLAS,				/* 63 Atlas / Newcom */
    &CODEX,				/* 64 Motorola Codex */
    &MT5634ZPX,				/* 65 Multitech MT5634ZPX */
    &ULINKV250,				/* 66 Microlink V.250 56K */
    &ITUTV250,				/* 67 Generic ITU-T V.250 */
    &RWV90,				/* 68 Rockwell V.90 56K */
    &SUPRAX,				/* 69 Diamond Supra Express V.90 */
    &LUCENT,				/* 70 Lucent Venus chipset */
    &PCTEL,				/* 71 PCTel */
    &CONEXANT,				/* 72 Conexant */
    &ZOOMV34,				/* 73 Zoom V.34 */
    &ZOOMV90,				/* 74 Zoom V.90 */
    &ZOOMV92,				/* 75 Zoom V.92 */
    &MOTSM56				/* 76 Motorola SM56 chipset */
#endif /* MINIDIAL */
};
/*
 * Declare modem names and associated numbers for command parsing,
 * and also for doing number-to-name translation.
 *
 * The entries must be in alphabetical order by modem name.
 */
struct keytab mdmtab[] = {
#ifndef MINIDIAL
    "3com-usr-megahertz-56k", n_USRX2,  CM_INV,
    "acer-v90",         n_RWV90,        M_ALIAS,
    "atlas-newcom-33600ifxC", n_ATLAS,  0,
    "att-1900-stu-iii", n_ATT1900,      0,
    "att-1910-stu-iii", n_ATT1910,      0,
    "att-7300",		n_ATTUPC,	0,
    "att-dataport",	n_DATAPORT,	0,
    "att-dtdm",		n_ATTDTDM,	0,
    "att-isn",          n_ATTISN,       0,
    "att-keepintouch",  n_KEEPINTOUCH,  0,
    "att-switched-net", n_ATTMODEM,	0,

    "att7300",		n_ATTUPC,	CM_INV,	/* old name */
    "attdtdm",		n_ATTDTDM,	CM_INV,	/* old name */
    "attisn",           n_ATTISN,       CM_INV,	/* old name */
    "attmodem",		n_ATTMODEM,	CM_INV,	/* old name */

    "bestdata",         n_BESTDATA,     0,
    "boca",		n_BOCA,		0,
    "cardinal",         n_CARDINAL,     0,
#endif /* MINIDIAL */
    "ccitt-v25bis",	n_CCITT,	CM_INV, /* Name changed to ITU-T */
#ifndef MINIDIAL
#ifdef OLDMODEMS
    "cermetek",		n_CERMETEK,	M_OLD,
#endif /* OLDMODEMS */
    "compaq",           n_COMPAQ,       0,
#ifdef OLDMODEMS
    "concord",		n_CONCORD,	M_OLD,
#endif /* OLDMODEMS */
    "conexant",         n_CONEXANT,     0,
    "courier",          n_USR,          CM_INV,
    "dataport",		n_DATAPORT,	CM_INV,	/* == att-dataport */
#ifdef OLDMODEMS
    "df03-ac",		n_DF03,		M_OLD,
    "df100-series",	n_DF100,	M_OLD,
    "df200-series",	n_DF200,	M_OLD,
#endif /* OLDMODEMS */
    "digitel-dt22",	n_DIGITEL,	0,
#endif /* MINIDIAL */
    "direct",		0,		CM_INV,	/* Synonym for NONE */
#ifndef MINIDIAL
    "fujitsu",          n_FUJITSU,      0,
    "gateway-telepath", n_TELEPATH,     0,
#ifdef OLDMODEMS
    "gdc-212a/ed",	n_GDC,		M_OLD,
    "ge",               n_GENERIC,	CM_INV|CM_ABR,
    "gen",              n_GENERIC,	CM_INV|CM_ABR,
    "gendatacomm",	n_GDC,		CM_INV,	/* Synonym for GDC */
#endif /* OLDMODEMS */
#endif /* MINIDIAL */
    "gene",             n_GENERIC,      CM_INV|CM_ABR,
    "generic",          n_GENERIC,      0,
    "generic-high-speed",n_GENERIC,     CM_INV,
    "h", 	   	n_HAYES,	CM_INV|CM_ABR,
    "ha", 	   	n_HAYES,	CM_INV|CM_ABR,
    "hay",    		n_HAYES,	CM_INV|CM_ABR,
    "haye", 	   	n_HAYES,	CM_INV|CM_ABR,
    "hayes",    	n_HAYES,	CM_INV|CM_ABR, /* Hayes 2400 */
#ifndef MINIDIAL
    "hayes-1200",	n_H_1200,	0,
#endif /* MINIDIAL */
    "hayes-2400",	n_HAYES,	0,
#ifndef MINIDIAL
    "hayes-high-speed", n_H_ACCURA,     0,
    "hayes-accura",     n_H_ACCURA,     CM_INV,
    "hayes-optima",     n_H_ACCURA,     CM_INV,
    "hayes-ultra",	n_H_ULTRA,	CM_INV,
    "hst-courier",      n_USR,          CM_INV,	/* Synonym for COURIER */
    "intel",		n_INTEL,        0,
#endif /* MINIDIAL */

    "itu-t-v250",       n_ITUTV250,     0,
    "itu-t-v25bis",	n_CCITT,	0,	/* New name for CCITT */
    "itu-t-v25ter/v250",n_ITUTV250,     CM_INV,

#ifndef MINIDIAL
    "lucent",           n_LUCENT,      0,
    "maxtech",		n_MAXTECH,     0,

    "megahertz-att-v34",    n_MHZATT,  0, /* Megahertzes */
    "megahertz-xjack",      n_XJACK,   CM_INV|CM_ABR,
    "megahertz-xjack-33.6", n_XJACK,   0,
    "megahertz-xjack-56k",  n_USRX2,   0, /* 3COM/USR/Megahertz 33.6 PC Card */

    "mi",		n_MICROCOM,	CM_INV|CM_ABR,
    "mic",		n_MICROCOM,	CM_INV|CM_ABR,
    "micr",		n_MICROCOM,	CM_INV|CM_ABR,
    "micro",		n_MICROCOM,	CM_INV|CM_ABR,
    "microc",		n_MICROCOM,	CM_INV|CM_ABR,
    "microco",		n_MICROCOM,	CM_INV|CM_ABR,
    "microcom",		n_MICROCOM,	CM_INV|CM_ABR,
    "microcom-at-mode",	n_UCOM_AT,	0, /* Microcom DeskPorte, etc */
    "microcom-sx-mode",	n_MICROCOM,	0, /* Microcom AX,QX,SX, native mode */
    "microlink",        n_MICROLINK,    0,
    "microlink-v250",   n_ULINKV250,    0,
    "motorola-codex",   n_CODEX,        0,
    "motorola-fastalk", n_MOTOROLA,	0,
    "motorola-lifestyle",n_MOTOROLA,	0,
    "motorola-montana", n_MONTANA,	0,
    "motorola-sm56-v90",n_MOTSM56,	0,
    "mt5634zpx",        n_MT5634ZPX,    0,
    "multitech",	n_MULTI,	0,
    "mwave",		n_MWAVE,	0,
#endif /* MINIDIAL */
    "none",             0,              0,
#ifndef MINIDIAL
#ifndef OLDTBCODE
    "old-telebit",      n_TELEBIT,      0,
#endif /* OLDTBCODE */
    "pctel",            n_PCTEL,        0,
#ifdef OLDMODEMS
    "penril",		n_PENRIL,	M_OLD,
#endif /* OLDMODEMS */
    "ppi",              n_PPI,		0,
#ifdef OLDMODEMS
    "racalvadic",	n_RACAL,	M_OLD,
#endif /* OLDMODEMS */
    "rockwell-v32",	n_RWV32,	0,
    "rockwell-v32bis",	n_RWV32B,	0,
    "rockwell-v34",	n_RWV34,	0,
    "rockwell-v90",	n_RWV90,	0,
    "rolm",             n_ROLM,		CM_INV|CM_ABR,
    "rolm-244pc",       n_ROLMAT,       0,
    "rolm-600-series",  n_ROLMAT,       0,
    "rolm-dcm",		n_ROLM,		0,
    "smartlink-v90",    n_USR,          M_ALIAS,
    "spirit-ii",        n_SPIRITII,     0,
    "sportster",        n_USR,          M_ALIAS,
    "sup",	        n_SUPRA,	CM_INV|CM_ABR,
    "supr",	        n_SUPRA,	CM_INV|CM_ABR,
    "supra",	        n_SUPRA,	CM_INV|CM_ABR,
    "supra-express-v90",n_SUPRAX,       0,
    "suprafaxmodem",	n_SUPRA,	0,
    "suprasonic",	n_SUPRASON,	0,
#ifdef CK_TAPI
    "tapi",		n_TAPI,		0,
#endif /* CK_TAPI */
    "te",               n_TBNEW,        CM_INV|CM_ABR,
    "tel",              n_TBNEW,        CM_INV|CM_ABR,
    "telebit",          n_TBNEW,        0,
    "telepath",         n_TELEPATH,     CM_INV,
#endif /* MINIDIAL */
    "unknown",		n_UNKNOWN,	0,
    "user-defined",     n_UDEF,		0,
#ifndef MINIDIAL

    "usr",               n_USR,         CM_INV|CM_ABR,
    "usr-212a",		 n_HAYES,	CM_INV|M_ALIAS,
    "usr-courier",       n_USR,         CM_INV,
    "usr-megahertz-56k", n_USRX2,       0,
    "usr-sportster",     n_USR,         CM_INV,
    "usr-xj1560-x2",     n_USRX2,       CM_INV,
    "usrobotics",        n_USR,         0,

    "v25bis",		n_CCITT,	CM_INV, /* Name changed to ITU-T */
#ifdef OLDMODEMS
    "ventel",		n_VENTEL,	M_OLD,
#endif /* OLDMODEMS */
    "zoltrix-v34",	n_ZOLTRIX,	0,
    "zoltrix-hsp-v90",  n_PCTEL,        M_ALIAS,
    "zoltrix-hcf-v90",  n_ITUTV250,     0,
    "zoo",		n_ZOOM,		CM_INV|CM_ABR,
    "zoom",		n_ZOOM,		CM_INV|CM_ABR,
    "zoom-v32bis",	n_ZOOM,		0,
    "zoom-v34",		n_ZOOMV34,	0,
    "zoom-v90",		n_ZOOMV90,	0,
    "zoom-v92",		n_ZOOMV92,	0,
    "zyxel",		n_ZYXEL,	0,
#endif /* MINIDIAL */
    "",                 0,              0
};
int nmdm = (sizeof(mdmtab) / sizeof(struct keytab)) - 1; /* Number of modems */

#define CONNECTED 1			/* For completion status */
#define D_FAILED  2
#define D_PARTIAL 3

static int tries = 0;
static int mdmecho = 0;			/* Assume modem does not echo */

static char *p;				/* For command strings & messages */

#define LBUFL 200
static char lbuf[LBUFL+4];
char modemmsg[LBUFL+4] = { NUL, NUL };	/* DIAL response from modem */

#ifdef DYNAMIC
#define RBUFL 256
static char *rbuf = NULL;
#else
#define RBUFL 63
static char rbuf[RBUFL+1];
#endif /* DYNAMIC */

#ifdef DYNAMIC
#define FULLNUML 256
char *fbuf = NULL;			/* For full (prefixed) phone number */
#else
#define FULLNUML 100
char fbuf[FULLNUML];
#endif /* DYNAMIC */

static ckjmpbuf sjbuf;

#ifdef CK_ANSIC
static SIGTYP (*savalrm)(int);	/* For saving alarm handler */
static SIGTYP (*savint)(int);	/* For saving interrupt handler */
#else
static SIGTYP (*savalrm)();	/* For saving alarm handler */
static SIGTYP (*savint)();	/* For saving interrupt handler */
#endif /* CK_ANSIC */

#ifdef CKLOGDIAL
static VOID
dologdial(s) char *s; {
    char buf2[16];
    char * r = NULL;
    int x, m, n;
    extern char cxlogbuf[], uidbuf[], myhost[];

    if (!s) s = "";
    if ((x = strlen(s)) > 0) {		/* Replace spaces by underscores */
	r = (char *)malloc(x+1);
	if (r) {
	    int i;
	    for (i = 0; i <= x; i++) {
		if (s[i] != 0 && s[i] <= SP)
		  r[i] = '_';
		else
		  r[i] = s[i];
	    }
	    s = r;
	}
    }
    p = ckdate();
    n = ckstrncpy(cxlogbuf,p,CXLOGBUFL);
    if (!uidbuf[0]) {
	debug(F100,"dologdial uidbuf empty","",0);
	ckstrncpy(uidbuf,(char *)whoami(),UIDBUFLEN);
    }
    m = strlen(uidbuf)+strlen(myhost)+strlen(ttname)+strlen(s)+strlen(buf2)+32;
    if (n+m < CXLOGBUFL-1) {
	p = cxlogbuf+n;
	if (diallcc && diallac) {
	    buf2[0] = '+';
	    ckmakmsg(&buf2[1],15,diallcc,"(",diallac,")");
	} else {
	    ckstrncpy(buf2,"Unknown",16);
	}
	sprintf(p," %s %s T=DIAL H=%s D=%s N=%s O=%s ",	/* safe (prechecked) */
		uidbuf,
		ckgetpid(),
		myhost,
		ttname,
		s,
		buf2
		);
	debug(F110,"dologdial cxlogbuf",cxlogbuf,0);
    } else
      sprintf(p,"LOGDIAL BUFFER OVERFLOW");
    if (r) free(r);
}
#endif /* CKLOGDIAL */

#ifndef MINIDIAL

#ifdef COMMENT
static VOID
xcpy(to,from,len)		/* Copy the given number of bytes */
    register char *to, *from;
    register unsigned int len; {
	while (len--) *to++ = *from++;
}
#endif /* COMMENT */
#endif /* MINIDIAL */

static SIGTYP
#ifdef CK_ANSIC
dialtime(int foo)			/* Timer interrupt handler */
#else
dialtime(foo) int foo;			/* Timer interrupt handler */
#endif /* CK_ANSIC */
/* dialtime */ {

    fail_code = F_TIME;			/* Failure reason = timeout */
    debug(F100,"dialtime caught SIGALRM","",0);
#ifdef BEBOX
#ifdef BE_DR_7
    alarm_expired();
#endif /* BE_DR_7 */
#endif /* BEBOX */
#ifdef OS2
    signal(SIGALRM, dialtime);
#endif /* OS2 */
#ifdef __EMX__
    signal(SIGALRM, SIG_ACK);		/* Needed for OS/2 */
#endif /* __EMX__ */

#ifdef OSK				/* OS-9 */
/*
  We are in an intercept routine but do not perform a F$RTE (done implicitly
  by RTS), so we have to decrement the sigmask as F$RTE does.  Warning:
  longjump only restores the CPU registers, NOT the FPU registers.  So, don't
  use FPU at all or at least don't use common FPU (double or float) register
  variables.
*/
    sigmask(-1);
#endif /* OSK */

#ifdef NTSIG
    if (foo == SIGALRM)
      PostAlarmSigSem();
    else
      PostCtrlCSem();
#else /* NTSIG */
#ifdef NT
    cklongjmp(ckjaddr(sjbuf),1);
#else /* NT */
    cklongjmp(sjbuf,1);
#endif /* NT */
#endif /* NTSIG */
    /* NOTREACHED */
    SIGRETURN;
}

static SIGTYP
#ifdef CK_ANSIC
dialint(int foo)			/* Keyboard interrupt handler */
#else
dialint(foo) int foo;
#endif /* CK_ANSIC */
/* dialint */ {
    fail_code = F_INT;
    debug(F100,"dialint caught SIGINT","",0);
#ifdef OS2
    signal(SIGINT, dialint);
    debug(F100,"dialint() SIGINT caught -- dialint restored","",0) ;
#endif /* OS2 */
#ifdef __EMX__
    signal(SIGINT, SIG_ACK);		/* Needed for OS/2 */
#endif /* __EMX__ */
#ifdef OSK				/* OS-9, see comment in dialtime() */
    sigmask(-1);
#endif /* OSK */
#ifdef NTSIG
    PostCtrlCSem() ;
#ifdef CK_TAPI
    PostTAPIConnectSem();
    PostTAPIAnswerSem();
#endif /* CK_TAPI */
#else /* NTSIG */
#ifdef NT
    cklongjmp(ckjaddr(sjbuf),1);
#else /* NT */
    cklongjmp(sjbuf,1);
#endif /* NT */
#endif /* NT */
    SIGRETURN;
}

/*
  Routine to read a character from communication device, handling TELNET
  protocol negotiations in case we're connected to the modem through a
  TCP/IP TELNET modem server.
*/
static int
ddinc(n) int n; {
#ifdef TNCODE
    int c = 0;
    int done = 0;
    debug(F101,"ddinc entry n","",n);
    while (!done) {
	c = ttinc(n);
	/* debug(F000,"ddinc","",c); */
	if (c < 0) return(c);
#ifndef OS2
	if ((c == IAC) && network && IS_TELNET()) {
	    switch (tn_doop((CHAR)(c & 0xff),duplex,ttinc)) {
	      case 2: duplex = 0; continue;
	      case 1: duplex = 1;
	      default: continue;
	    }
	} else done = 1;
#else /* OS2 */
	done = !(c == IAC && network && IS_TELNET());
	scriptwrtbuf(c);	/* TELNET negotiations handled by emulator */
#endif /* OS2 */
    }
    return(c & 0xff);
#else  /* TNCODE */
    return(ttinc(n));
#endif /* TNCODE */
}

static VOID
ttslow(s,millisec) char *s; int millisec; { /* Output s-l-o-w-l-y */
#ifdef TCPSOCKET
    extern int tn_nlm, tn_b_nlm;
#endif /* TCPSOCKET */
    debug(F111,"ttslow",s,millisec);
    if (dialdpy && (duplex || !mdmecho)) { /* Echo the command in case modem */
	printf("%s\n",s);		/* isn't echoing commands. */
#ifdef OS2
	{
	    char *s2 = s;		/* Echo to emulator */
	    while (*s2) {
		scriptwrtbuf((USHORT)*s2++);
	    }
	    scriptwrtbuf((USHORT)CR);
	    scriptwrtbuf((USHORT)LF);
	}
#endif /* OS2 */
    }
    for (; *s; s++) {
	ttoc(*s);
#ifdef TCPSOCKET
	if (*s == CR && network && IS_TELNET()) {
	    if (!TELOPT_ME(TELOPT_BINARY) && tn_nlm != TNL_CR)
	      ttoc((char)((tn_nlm == TNL_CRLF) ? LF : NUL));
	    else if (TELOPT_ME(TELOPT_BINARY) &&
		     (tn_b_nlm == TNL_CRLF || tn_b_nlm == TNL_CRNUL))
	      ttoc((char)((tn_b_nlm == TNL_CRLF) ? LF : NUL));
        }
#endif /* TCPSOCKET */
	if (millisec > 0)
	  msleep(millisec);
    }
}

/*
 * Wait for a string of characters.
 *
 * The characters are waited for individually, and other characters may
 * be received "in between".  This merely guarantees that the characters
 * ARE received, and in the order specified.
 */
static VOID
waitfor(s) char *s; {
    CHAR c, x;
    while ((c = *s++)) {		/* while more characters remain... */
	do {				/* wait for the character */
	    x = (CHAR) (ddinc(0) & 0177);
	    debug(F000,"dial waitfor got","",x);
	    if (dialdpy) {
		if (x != LF) conoc(x);
		if (x == CR) conoc(LF);
	    }
	} while (x != c);
    }
}

static int
didweget(s,r) char *s, *r; {	/* Looks in string s for response r */
    int lr = (int)strlen(r);	/*  0 means not found, 1 means found it */
    int i;
    debug(F110,"didweget",r,0);
    debug(F110," in",s,0);
    for (i = (int)strlen(s)-lr; i >= 0; i--)
	if ( s[i] == r[0] ) if ( !strncmp(s+i,r,lr) ) return( 1 );
    return( 0 );
}


/* R E S E T -- Reset alarms, etc. on exit. */

static VOID
dreset() {
    debug(F100,"dreset resetting alarm and signal handlers","",0);
    alarm(0);
    signal(SIGALRM,savalrm);		/* restore alarm handler */
    signal(SIGINT,savint);		/* restore interrupt handler */
    debug(F100,"dreset alarm and signal handlers reset","",0);
}

/*
  Call this routine when the modem reports that it has connected at a certain
  speed, giving that speed as the argument.  If the connection speed is not
  the same as Kermit's current communication speed, AND the modem interface
  speed is not locked (i.e. DIAL SPEED-MATCHING is not ON), then change the
  device speed to the one given.
*/
static VOID
#ifdef CK_ANSIC
spdchg(long s)
#else
spdchg(s) long s;
#endif /* CK_ANSIC */
/* spdchg */ {
    int s2;
    if (!mdmspd)			/* If modem interface speed locked, */
      return;				/*  don't do this. */
    if (speed != s) {			/* Speeds differ? */
	s2 = s / 10L;			/* Convert to cps expressed as int */
	if (ttsspd(s2) < 0) {		/* Change speed. */
	    printf(" WARNING - speed change to %ld failed.\r\n",s);
	} else {
	    printf(" Speed changed to %ld.\r\n",s);
	    speed = s;			/* Update global speed variable */
	}
    }
}

/*
  Display all characters received from modem dialer through this routine,
  for consistent handling of carriage returns and linefeeds.
*/
static VOID
#ifdef CK_ANSIC
dialoc(char c)
#else
dialoc(c) char c;
#endif /* CK_ANSIC */
{ /* dialoc */				/* Dial Output Character */
    if (dialdpy) {
	if (c != LF) conoc(c);		/* Don't echo LF */
	if (c == CR) conoc(LF);		/* Echo CR as CRLF */
    }
}

#ifndef NOSPL
char *
getdm(x) int x; {			/* Return dial modifier */
    MDMINF * mp;
    int m;
    int ishayes = 0;
    m = mdmtyp;
    if (m < 1)
      if (mdmsav > -1)
	m = mdmsav;
    if (m < 1)
      return("");
#ifndef MINIDIAL
    if (m == n_TAPI)
      m = n_HAYES;
#endif /* MINIDIAL */
    mp = modemp[m];
    ishayes = (dialcapas ? dialcapas : mp->capas) & CKD_AT;
    switch (x) {
      case VN_DM_LP:
	return(ishayes ? "," : "");
      case VN_DM_SP:
#ifdef MINIDIAL
	return("");
#else
	return(m == n_USR ? "/" : "");
#endif /* MINIDIAL */
      case VN_DM_PD:
	return(ishayes ? "P" : "");
      case VN_DM_TD:
	return(ishayes ? "T" : "");
      case VN_DM_WA:
	return(ishayes ? "@" : "");
      case VN_DM_WD:
	return(ishayes ? "W" : "");
      case VN_DM_RC:
	return(ishayes ? ";" : "");
      case VN_DM_HF:
	return(ishayes ? "!" : "");
      case VN_DM_WB:
	return(ishayes ? "$" : "");
    }
    return("");
}
#endif /* NOSPL */

static VOID
getdialmth() {
    if (dialmauto && diallcc) {		/* If DIAL METHOD AUTO... */
	int i;				/* and we know our area code... */
	for (i = 0; i < ndialtocc; i++) { /* First check Tone countries list */
	    if (!strcmp(dialtocc[i],diallcc)) {
		dialmth = XYDM_T;
		break;
	    }
	}
	for (i = 0; i < ndialpucc; i++) { /* Then Pulse countries list */
	    if (!strcmp(dialpucc[i],diallcc)) {
		dialmth = XYDM_P;
		break;
	    }
	}
    }
}

VOID				/* Get dialing defaults from environment */
getdialenv() {
    char *p = NULL;
    int i, x;

    makestr(&p,getenv("K_DIAL_DIRECTORY"));
    if (p) {
	int i;
	xwords(p,(MAXDDIR - 2),dialdir,0);
	for (i = 0; i < (MAXDDIR - 1); i++) {
	    if (!dialdir[i+1])
	      break;
	    else
	      dialdir[i] = dialdir[i+1];
	}
	ndialdir = i;
    }
    xmakestr(&diallcc,getenv("K_COUNTRYCODE")); /* My country code */
    xmakestr(&dialixp,getenv("K_LD_PREFIX"));   /* My long-distance prefix */
    xmakestr(&dialldp,getenv("K_INTL_PREFIX")); /* My international prefix */
    xmakestr(&dialldp,getenv("K_TF_PREFIX"));   /* Ny Toll-free prefix */

#ifndef NOICP
    p = getenv("K_DIAL_METHOD");	/* Local dial method */
    if (p) if (*p) {
	extern struct keytab dial_m[];
	extern int ndial_m;
	i = lookup(dial_m,p,ndial_m,&x);
	if (i > -1) {
	    if (i == XYDM_A) {
		dialmauto = 1;
		dialmth = XYDM_D;
	    } else {
		dialmauto = 0;
		dialmth = i;
	    }
	}
    }
#endif /* NOICP */

    p = NULL;
    xmakestr(&p,getenv("K_TF_AREACODE")); /* Toll-free areacodes */
    if (p) {
	int i;
	xwords(p,7,dialtfc,0);
	for (i = 0; i < 8; i++) {
	    if (!dialtfc[i+1])
	      break;
	    else
	      dialtfc[i] = dialtfc[i+1];
	}
	ntollfree = i;
	free(p);
    }
    for (i = 0; i < MAXTPCC; i++) {	/* Clear Tone/Pulse country lists */
	dialtocc[i] = NULL;
	dialpucc[i] = NULL;
    }
    for (i = 0; i < MAXTPCC; i++) {	/* Init Tone country list */
	if (tonecc[i])
	  makestr(&(dialtocc[i]),tonecc[i]);
	else
	  break;
    }
    ndialtocc = i;
    for (i = 0; i < MAXTPCC; i++) {	/* Init Pulse country list */
	if (pulsecc[i])
	  makestr(&(dialpucc[i]),pulsecc[i]);
	else
	  break;
    }
    ndialpucc = i;

    if (diallcc) {			/* Have country code */
	if (!strcmp(diallcc,"1")) {	/* If it's 1 */
	    if (!dialldp)		/* Set these prefixes... */
	      makestr(&dialldp,"1");
	    if (!dialtfp)
	      makestr(&dialtfp,"1");
	    if (!dialixp)
	      makestr(&dialixp,"011");
	    if (ntollfree == 0) {	/* Toll-free area codes */
		if ((dialtfc[0] = malloc(4))) {
		    ckstrncpy(dialtfc[0],"800",4); /* 1970-something */
		    ntollfree++;
		    if ((dialtfc[1] = malloc(4))) {
			ckstrncpy(dialtfc[1],"888",4); /* 1996 */
			ntollfree++;
			if ((dialtfc[2] = malloc(4))) {
			    ckstrncpy(dialtfc[2],"877",4); /* 5 April 1998 */
			    ntollfree++;
			    if ((dialtfc[3] = malloc(4))) {
				ckstrncpy(dialtfc[3],"866",4); /* 2000? */
				ntollfree++;
			    }
			}
		    }
		}
	    }
	} else if (!strcmp(diallcc,"358") &&
		   ((int) strcmp(zzndate(),"19961011") > 0)
		   ) {			/* Finland */
	    if (!dialldp)		/* Long-distance prefix */
	      makestr(&dialldp,"9");
	    if (!dialixp) 		/* International dialing prefix */
	      makestr(&dialixp,"990");
	} else {			/* Not NANP or Finland */
	    if (!dialldp)
	      makestr(&dialldp,"0");
	    if (!dialixp)
	      makestr(&dialixp,"00");
	}
    }
    xmakestr(&diallac,getenv("K_AREACODE"));
    xmakestr(&dialpxo,getenv("K_PBX_OCP"));
    xmakestr(&dialpxi,getenv("K_PBX_ICP"));
    p = getenv("K_PBX_XCH");
#ifdef COMMENT
    xmakestr(&dialpxx,p);
#else
    if (p) if (*p) {
	char * s = NULL;
	char * pp[MAXPBXEXCH+2];
	makestr(&s,p);			/* Make a copy for poking */
	if (s) {
	    xwords(s,MAXPBXEXCH+1,pp,0); /* Note: pp[] is 1-based. */
	    for (i = 0; i <= MAXPBXEXCH; i++) {
                if (!pp[i+1]) break;
		makestr(&(dialpxx[i]),pp[i+1]);
		ndialpxx++;
	    }
	    makestr(&s,NULL);		/* Free poked copy */
	}
    }
#endif /* COMMENT */
}

static int
dialfail(x) int x; {
    char * s;

    fail_code = x;
    debug(F101,"ckudial dialfail","",x);
    dreset();				/* Reset alarm and signal handlers */

    printf("%s Failure: ", func_code == 0 ? "DIAL" : "ANSWER");
    if (dialdpy) {			/* If showing progress */
       debug(F100,"dial display is on","",0);
	p = ck_time();			/* get current time; */
	if (*p) printf("%s: ",p);
    }
    switch (fail_code) {		/* Type of failure */
      case F_TIME: 			/* Timeout */
	if (dial_what == DW_INIT)
	  printf ("Timed out while trying to initialize modem.\n");
	else if (dial_what == DW_DIAL)
	  printf ("%s interval expired.\n",
		  func_code == 0 ? "DIAL TIMEOUT" : "ANSWER timeout");
	else
	  printf("Timeout.\n");
	fflush(stdout);
	if (mdmcapas & CKD_AT)
	  ttoc('\015');			/* Send CR to interrupt dialing */
	/* Some Hayes modems don't fail with BUSY on busy lines */
	dialsta = DIA_TIMO;
	debug(F110,"dial","timeout",0);
	break;

      case F_INT:			/* Dialing interrupted */
	printf ("Interrupted.\n");
	fflush(stdout);
#ifndef NOXFER
	interrupted = 1;
#endif /* NOXFER */
	debug(F111,"dial","interrupted",mdmcapas & CKD_AT);
	if (mdmcapas & CKD_AT)
	  ttoc('\015');			/* Send CR to interrupt dialing */
	dialsta = DIA_INTR;
	break;

    case F_MODEM:			/* Modem detected a failure */
         debug(F111,"dialfail()","lbuf",lbuf);
         if (lbuf && *lbuf) {
            printf(" \"");
            for (s = lbuf; *s; s++)
               if (isprint(*s))
                  putchar(*s);		/* Display printable reason */
            printf ("\"");
         } else printf(func_code == 0 ?
                        " Call not completed." :
                        " Call did not come in."
                        );
	printf("\n");
	debug(F110,"dial",lbuf,0);
	if (dialsta < 0) dialsta = DIA_UNSP;
	break;

      case F_MINIT:			/* Failure to initialize modem */
	printf ("Error initializing modem.\n");
	debug(F110,"dial","modem init",0);
	dialsta = DIA_NOIN;
	break;

    default:
	printf("unknown\n");
	debug(F110,"dial","unknown",0);
	fflush(stdout);
	if (mdmcapas & CKD_AT)
	  ttoc('\015');			/* Send CR to interrupt dialing */
	dialsta = DIA_INTR;
    }

#ifdef DYNAMIC
    if (rbuf) free(rbuf); rbuf = NULL;
    if (fbuf) free(fbuf); fbuf = NULL;
#endif /* DYNAMIC */

    if (dialsta < 0) dialsta = DIA_UERR; /* Set failure code */
    return(0);				/* Return zero (important) */
}

/*  C K D I A L	 --  Dial up the remote system */

/* Returns 1 if call completed, 0 otherwise */

static int mdmwait, mdmstat = 0;
#ifndef CK_TAPI
static
#endif /* CK_TAPI */
int waitct;
int mdmwaitd = 10 ;			/* dialtmo / mdmwait difference */
static char c;
static char *telnbr;

static int wr = 0;			/* wr = wake rate */
static char * ws;			/* ws = wake string */
static char * xnum = NULL;
static int inited = 0;

static SIGTYP
#ifdef CK_ANSIC
_dodial(void * threadinfo)
#else /* CK_ANSIC */
_dodial(threadinfo) VOID * threadinfo;
#endif /* CK_ANSIC */
/* _dodial */ {
    char c2;
    char *dcmd, *s, *flocmd = NULL;
    int x = 0, n = F_TIME;

#ifdef NTSIG
    signal( SIGINT, dialint );
    if (threadinfo) {			/* Thread local storage... */
	TlsSetValue(TlsIndex,threadinfo);
    }
#endif /* NTSIG */

    dcmd = dialcmd ? dialcmd : mp->dial_str;
    if ((int)strlen(dcmd) + (int)strlen(telnbr) > (LBUFL - 2)) {
	printf("DIAL command + phone number too long!\n");
	dreset();
#ifdef DYNAMIC
	if (rbuf) free(rbuf); rbuf = NULL;
	if (fbuf) free(fbuf); fbuf = NULL;
#endif /* DYNAMIC */
#ifdef NTSIG
	ckThreadEnd(threadinfo);
#endif /* NTSIG */
	SIGRETURN;	 /* No conversation with modem to complete dialing */
    }
    makestr(&xnum,telnbr);

    getdialmth();			/* Get dial method */

#ifdef CK_ATDT
    /* Combine the SET DIAL METHOD command with the DIAL command string */
    if (!dialcmd &&			/* Using default DIAL command */
	(mdmcapas & CKD_AT) &&		/* AT command set only */
	((dialmth == XYDM_T && !dialtone) || /* and using default */
	 (dialmth == XYDM_P && !dialpulse))) { /* modem commands... */
	char c;
	debug(F110,"dial atdt xnum 1",xnum,0);
	s = dcmd;
	debug(F110,"dial atdt s",s,0);
	if (*telnbr != 'T' &&
	    *telnbr != 'P' &&
	    *telnbr != 't' &&
	    *telnbr != 'p' &&
	    !ckstrcmp(s,"atd",3,0) &&
	    s[3] != 'T' &&
	    s[3] != 'P' &&
	    s[3] != 't' &&
	    s[3] != 'p') {
	    char xbuf[200];
	    c = (dialmth == XYDM_T) ? 'T' : 'P';
	    if (islower(s[0]))
	      c = tolower(c);
	    if ((int)strlen(telnbr) < 199) {
		sprintf(xbuf,"%c%s",c,telnbr);
		makestr(&xnum,xbuf);
	    }
	}
    }
#endif /* CK_ATDT */
    debug(F111,"_dodial",xnum,xredial);

    /* Hang up the modem (in case it wasn't "on hook") */
    /* But only if SET DIAL HANGUP ON... */

    if (!xredial) {			/* Modem not initalized yet. */
	inited = 0;
    }
    if (!xredial || !inited) {
	if (dialhup() < 0) {		/* Hangup first */
	    debug(F100,"_dodial dialhup failed","",0);
#ifndef MINIDIAL
	    if (mdmcapas & CKD_TB)	/* Telebits might need a BREAK */
	      ttsndb();			/*  first. */
#endif /* MINIDIAL */
	    if (dialhng && dialsta != DIA_PART) { /* If hangup failed, */
		ttclos(0);		/* close and reopen the device. */
		if (ttopen(ttname,&local,mymdmtyp,0) < 0) {
		    printf("Sorry, Can't hang up communication device.\n");
		    printf("Try 'set line %s' again.\n",ttname);
		    dialsta = DIA_HANG;
#ifdef DYNAMIC
		    if (rbuf) free(rbuf); rbuf = NULL;
		    if (fbuf) free(fbuf); fbuf = NULL;
#endif /* DYNAMIC */
		    dreset();
#ifdef NTSIG
		    ckThreadEnd(threadinfo);
#endif /* NTSIG */
		    SIGRETURN;
		}
	    }
	}
	inited = 0;			/* We hung up so must reinit */
    }
#ifndef MINIDIAL
    /* Don't start talking to Rolm too soon */
    if (mymdmtyp == n_ROLM && dialsta != DIA_PART)
      msleep(500);
#endif /* MINIDIAL */

    if (dialsta != DIA_PART		/* Some initial setups. */
#ifndef MINIDIAL
	&& mymdmtyp != n_ATTUPC
#endif /* MINIDIAL */
	) {
	fail_code = F_MINIT;		/* Default failure code */
	dial_what = DW_INIT;		/* What I'm Doing Now   */
	if (dialdpy) {			/* If showing progress, */
	    p = ck_time();		/* get timestamp.   */
	    if (!inited)
	      if (*p)
		printf(" Initializing: %s...\n",p);
	}
    }
#ifndef MINIDIAL
#ifdef ATT7300
    if (mymdmtyp == n_ATTUPC) {
/*
  For ATT7300/Unix PC's with their special internal modem.  Whole dialing
  process is handled right here, an exception to the normal structure.
  Timeout and user interrupts are enabled during dialing.  attdial() is in
  file ckutio.c.  - jrd
*/
        _PROTOTYP( int attdial, (char *, long, char *) );
	fail_code = F_MODEM;		/* Default failure code */
	dial_what = DW_DIAL;
	if (dialdpy) {			/* If showing progress */
	    p = ck_time();		/* get current time; */
	    if (*p)
	      printf(" Dialing: %s...\n",p);
	}
	alarm(waitct);			/* Set alarm */
	if (attdial(ttname,speed,telnbr)) { /* dial internal modem */
	    dreset();			/* reset alarms, etc. */
	    printf(" Call failed.\r\n");
	    dialhup();	        	/* Hangup the call */
#ifdef DYNAMIC
	    if (rbuf) free(rbuf); rbuf = NULL;
	    if (fbuf) free(fbuf); fbuf = NULL;
#endif /* DYNAMIC */
	    dialsta = DIA_UERR;
#ifdef NTSIG
	    ckThreadEnd(threadinfo);
#endif /* NTSIG */
	    SIGRETURN;			/* return failure */
	}
	dreset();			/* reset alarms, etc. */
	ttpkt(speed,FLO_DIAX,parity);	/* cancel dialing ioctl */
	if (!quiet && !backgrd) {
	    if (dialdpy) {
		printf("\n");
		printf(" Call complete.\r\n");
	    } else if (modemmsg[0])
		printf(" Call complete: \"%s\".\r\n",(char *)modemmsg);
	    else
	      printf(" Call complete.\r\n");
	}
#ifdef CKLOGDIAL
	dologdial(telnbr);
#endif /* CKLOGDIAL */

	dialsta = DIA_OK;
#ifdef DYNAMIC
	if (rbuf) free(rbuf); rbuf = NULL;
	if (fbuf) free(fbuf); fbuf = NULL;
#endif /* DYNAMIC */
#ifdef NTSIG
	ckThreadEnd(threadinfo);
#endif /* NTSIG */
	SIGRETURN;	/* No conversation with modem to complete dialing */
    } else
#endif /* ATT7300 */
#ifdef CK_TAPI
      if (tttapi && !tapipass) {	/* TAPI Dialing */
	  switch (func_code) {
	    case 0:			/* Dial */
	      if (cktapidial(telnbr)) {
		  fail_code = 0;
		  if (partial) {
		      dialsta = DIA_PART;
		  } else {
		      dialsta = DIA_OK;
		      speed = ttgspd();
		  }
	      } else {
		  if (dialsta == DIA_PART)
		    cktapihangup();
		  if (!fail_code)
		    fail_code = F_MODEM;
		  dialsta = DIA_TAPI;
	      }
	      break;
	    case 1: {			/* Answer */
		long strttime = time((long *)NULL);
		long diff = 0;
		do {
		    if (dialatmo > 0) {
			strttime += diff;
			waitct   -= diff;
		    }
		    fail_code = 0;
		    if (cktapianswer()) { /* SUCCESS */
			dialsta = DIA_OK;
			speed = ttgspd();
			break;
		    } else {		/* FAILURE */
			if (fail_code) {
			    dialsta = DIA_TAPI;
			    break;
			} else {
			    fail_code = F_MODEM;
			    dialsta = DIA_TAPI;
			}
		    }
		    if (dialatmo > 0) {
			diff = time((long *)NULL) - strttime;
		    }
		} while ((dialatmo > 0) ? (diff < waitct) : 1);
		break;
	    }
	  }
#ifdef NTSIG
	  ckThreadEnd(threadinfo);
#endif /* NTSIG */
	  SIGRETURN;
      } else
#endif /* CK_TAPI */
#endif /* MINIDIAL */

/* Modems with AT command set... */

      if ((mdmcapas & CKD_AT) && dialsta != DIA_PART) {

	  if (dialpace > -1)		/* Set intercharacter pacing */
	    wr = dialpace;
	  else
	    wr = mp->wake_rate;

	  if (dialini)			/* Get wakeup/init string */
	    ws = dialini;
	  else
	    ws = mp->wake_str;
#ifdef COMMENT
	  if (!ws) ws = "\015";		/* If none, use CR */
#endif /* COMMENT */

	  /* First get the modem's attention and enable result codes */

	  for (tries = 0; tries < 5; tries++) { /* Send short command */
	      if (tries > 0) {
		  ttoc('\015');		/* AT must go first for speed */
		  msleep(wr);		/* detection. */
	      }
	      if (mymdmtyp == n_GENERIC) /* Force word result codes */
		ttslow("ATQ0V1\015",wr); /* for generic modem type */
	      else
		ttslow("ATQ0\015",wr);
	      mdmstat = getok(tries < 2 ? 2 : tries, 1); /* Get response */
	      if (mdmstat > 0) break;	/* OK - done */
	      if (dialdpy && tries > 0) {
		  printf("\r\n No response from modem");
		  if (tries == 4) {
		      printf(".\r\n");
		      dialsta = DIA_NRSP;
#ifdef DYNAMIC
		      if (rbuf) free(rbuf); rbuf = NULL;
		      if (fbuf) free(fbuf); fbuf = NULL;
#endif /* DYNAMIC */
#ifdef NTSIG
		      ckThreadEnd(threadinfo);
#endif /* NTSIG */
		      SIGRETURN;	/* return failure */
		  }
		  printf(", retrying%s...\r\n",
			 (tries > 1) ? " again" : "");
		  fflush(stdout);
	      }
	      ttflui();
	      switch (tries) {
		case 0: msleep(100); break;
		case 1: ttsndb(); break;
		default:
		  if (network) {
		      ttsndb();
		  } else {
		      if (tries == 2) {
			  tthang();
			  ttflui();
		      } else {
			  mdmhup();
		      }
		      inited = 0;
		  }
	      }
	      fflush(stdout);
	  }
	  debug(F101,"_dodial ATQ0 mdmstat","",mdmstat);

	  if (xredial && inited) {	/* Redialing... */
	      ttoc('\015');		/* Cancel previous */
	      msleep(250);		/* Wait a bit */
#ifdef COMMENT
/* This wasn't the problem... */
	      ttflui();			/* Clear out stuff from modem setup */
	      ttslow("ATS7=60\015",wr);	/* Redo carrier wait */
	      getok(4,1);		/* Get response */
#endif /* COMMENT */
	      alarm(0);			/* Just in case... */
	      ttflui();			/* Clear out stuff from modem setup */
	      goto REDIAL;		/* Skip setup - we already did it */
	  }
/*
  Do flow control next because a long init string echoing back could
  cause data overruns, causing us to miss the OK, or (worse) to get out
  of sync entirely.
*/
	  x = 0;			/* User said SET DIAL FLOW RTS/CTS */
	  if (dialfc == FLO_RTSC ||	/* Even if Kermit's FLOW isn't...  */
	      (dialfc == FLO_AUTO && flow == FLO_RTSC)) {
	      if (dialhwfc) {		/* User-defined HWFC string */
		  if (*dialhwfc) {
		      x = 1;
		      flocmd = dialhwfc;
		  }
	      } else if ((mdmcapas & CKD_HW) && *(mp->hwfc_str)) {
		  x = 1;
		  flocmd = mp->hwfc_str;
	      }
	  } else if (dialfc == FLO_XONX || /* User said SET DIAL FLOW SOFT */
		     (dialfc == FLO_AUTO && flow == FLO_XONX)) {
	      if (dialswfc) {
		  if (*dialswfc) {
		      x = 1;
		      flocmd = dialswfc;
		  }
	      } else if ((mdmcapas & CKD_SW) && *(mp->swfc_str)) {
		  x = 1;
		  flocmd = mp->swfc_str;
	      }
	  } else if (dialfc == FLO_NONE) { /* User said SET DIAL FLOW NONE */
	      if (dialnofc) {
		  if (*dialnofc) {
		      x = 1;
		      flocmd = dialnofc;
		  }
	      } else if (mp->nofc_str && *(mp->nofc_str)) {
		  x = 1;
		  flocmd = mp->nofc_str;
	      }
	  }
	  if (x) {			/* Send the flow control command */
	      debug(F110,"_dodial flocmd",flocmd,0);
	      for (tries = 4; tries > 0; tries--) { /* Send the command */
		  ttslow(flocmd,wr);
		  mdmstat = getok(5,1);
		  if (mdmstat > 0) break;
		  if (dialdpy && tries > 1)
		    printf(" No response from modem, retrying%s...\n",
			   (tries < 4) ? " again" : "");
	      }

#ifdef CK_TTSETFLOW
#ifdef CK_RTSCTS
/*
  So far only ckutio.c has ttsetflow().
  We have just told the modem to turn on RTS/CTS flow control and the modem
  has said OK.  But we ourselves have not turned it on yet because of the
  disgusting ttpkt(...FLO_DIAL...) hack.  So now, if the computer does not
  happen to be asserting RTS, the modem will no longer send characters to it.
  So at EXACTLY THIS POINT, we must enable RTS/CTS in the device driver.
*/
	      if (dialfc == FLO_RTSC ||
		  (dialfc == FLO_AUTO && flow == FLO_RTSC)) {
		  ttsetflow(FLO_RTSC);
	      }
#endif /* CK_RTSCTS */
#endif /* CK_TTSETFLOW */
	  }
	  ttflui();			/* Clear out stuff from modem setup */
	  msleep(250);

	  if (!ws) goto xdialec;	/* No init string */
	  if (!*ws) goto xdialec;

	  for (tries = 4; tries > 0; tries--) { /* Send init string */
	      ttslow(ws,wr);
	      mdmstat = getok(4,1);	/* Get response */
	      if (mdmstat > 0) break;
	      if (dialdpy && tries > 1)
		printf(" No response from modem, retrying%s...\n",
		       (tries < 4) ? " again" : "");
	  }
	  debug(F101,"_dodial wake_str mdmstat","",mdmstat);

	  if (mdmstat < 1) {		/* Initialized OK? */
	      dialfail(F_MINIT);	/* No, fail. */
#ifdef NTSIG
	      ckThreadEnd(threadinfo);
#endif /* NTSIG */
	      SIGRETURN;
	  }

#ifndef MINIDIAL
    } else if (mymdmtyp == n_ATTDTDM && dialsta != DIA_PART) { /* AT&T ... */
	ttsndb();			/* Send BREAK */
#endif /* MINIDIAL */

    } else if (dialsta != DIA_PART) { /* All others */

	/* Place modem into command mode */

	ws = dialini ? dialini : mp->wake_str;
	if (ws && (int)strlen(ws) > 0) {
	    debug(F111,"_dodial default, wake string", ws, wr);
	    ttslow(ws, wr);
	} else debug(F100,"_dodial no wake_str","",0);
	if (mp->wake_prompt && (int)strlen(mp->wake_prompt) > 0) {
	    debug(F110,"_dodial default, waiting for wake_prompt",
		  mp->wake_prompt,0);
	    alarm(10);
	    waitfor(mp->wake_prompt);
	    alarm(0);
	} else debug(F100,"_dodial no wake_prompt","",0);
    }

/* Handle error correction, data compression, and flow control... */

  xdialec:

    if (dialsta != DIA_PART) {
	alarm(0);			/* Turn off alarm */
	debug(F100,"_dodial got wake prompt","",0);
	msleep(500);			/* Allow settling time */

	/* Enable/disable error-correction */

	x = 0;
	if (dialec) {			/* DIAL ERROR-CORRECTION is ON */
	    if (dialecon) {		/* SET DIAL STRING ERROR-CORRECTION */
		if (*dialecon) {
		    x = 1;
		    ttslow(dialecon, wr);
		}
	    } else if ((mdmcapas & CKD_EC) && *(mp->ec_on_str)) {
		x = 1;
		ttslow(mp->ec_on_str, wr);
	    }
#ifdef COMMENT
	    else printf(
		  "WARNING - I don't know how to turn on EC for this modem\n"
		     );
#endif /* COMMENT */
	} else {
	    if (dialecoff) {		/* DIAL ERROR-CORRECTION OFF */
		if (*dialecoff) {
		    x = 1;
		    ttslow(dialecoff, wr);
		}
	    } else if ((mdmcapas & CKD_EC) && *(mp->ec_off_str)) {
		x = 1;
		ttslow(mp->ec_off_str, wr);
	    }
#ifdef COMMENT
	    else printf(
		  "WARNING - I don't know how to turn off EC for this modem\n"
		     );
#endif /* COMMENT */
	}
	/* debug(F101,"ckudia xx_ok","",xx_ok); */
	if (x && xx_ok) {			/* Look for OK response */
	    debug(F100,"ckudia calling xx_ok for EC","",0);
	    x = (*xx_ok)(5,1);
	    debug(F101,"ckudia xx_ok","",x);
	    if (x < 0) {
		printf("WARNING - Trouble enabling error-correction.\n");
		printf(
" Likely cause: Your modem is an RPI model, which does not have built-in\n");
		printf(" error correction and data compression.");
	    }
	}

	/* Enable/disable data compression */

	if (x > 0) x = 0;
	if (dialdc) {
	    if (x < 0 || !dialec) {
		printf(
"WARNING - You can't have compression without error correction.\n");
	    } else if (dialdcon) {	/* SET DIAL STRING ... */
		if (*dialdcon) {
		    x = 1;
		    ttslow(dialdcon, wr);
		}
	    } else if ((mdmcapas & CKD_DC) && *(mp->dc_on_str)) {
		x = 1;
		ttslow(mp->dc_on_str, wr);
	    }
#ifdef COMMENT
	    else printf(
		  "WARNING - I don't know how to turn on DC for this modem\n"
			  );
#endif /* COMMENT */
	} else {
	    if (dialdcoff) {
		if (*dialdcoff) {
		    x = 1;
		    ttslow(dialdcoff, wr);
		}
	    } else if ((mdmcapas & CKD_DC) && *(mp->dc_off_str)) {
		x = 1;
		ttslow(mp->dc_off_str, wr);
	    }
#ifdef COMMENT
	    else printf(
"WARNING - I don't know how to turn off compression for this modem\n"
			  );
#endif /* COMMENT */
	}
	if (x && xx_ok) {			/* Look for OK response */
	    x = (*xx_ok)(5,1);
	    if (x < 0) printf("WARNING - Trouble enabling compression\n");
	}
    }

#ifndef NOXFER
#ifndef MINIDIAL
    if (mdmcapas & CKD_KS && dialsta != DIA_PART) { /* Kermit spoof */
	int r;				/* Register */
	char tbcmdbuf[64];		/* Command buffer */
	switch (mymdmtyp) {

	  case n_MICROCOM:		/* Microcoms in SX mode */
  	    if (dialksp)
	      sprintf(tbcmdbuf,"APM1;KMC%d\015",stchr);	/* safe */
	    else
	      sprintf(tbcmdbuf,"APM0\015"); /* safe */
  	    ttslow(tbcmdbuf, MICROCOM.wake_rate);
  	    alarm(3);
	    waitfor(mp->wake_prompt);
	    alarm(0);
	    break;

	  case n_TELEBIT:		/* Old and new Telebits */
	  case n_TBNEW:
	    if (!dialksp) {
		sprintf(tbcmdbuf,"ATS111=0\015"); /* safe */
	    } else {
		switch (parity) {	/* S111 value depends on parity */
		  case 'e': r = 12; break;
		  case 'm': r = 13; break;
		  case 'o': r = 11; break;
		  case 's': r = 14; break;
		  case 0:
		  default:  r = 10; break;
		}
		sprintf(tbcmdbuf,"ATS111=%d S112=%d\015",r,stchr); /* safe */
	    }
	    ttslow(tbcmdbuf, wr);

/* Not all Telebit models have the Kermit spoof, so ignore response. */

	    if (xx_ok) {		/* Get modem's response */
		x = (*xx_ok)(5,1);
	    }
	}
    }
#endif /* MINIDIAL */
#endif /* NOXFER */

    /* Speaker */

    if (mymdmtyp != n_GENERIC &&
	(mdmcapas & CKD_AT) && (dialsta != DIA_PART) &&
	!dialspon && !dialspoff &&
	!dialvol1 && !dialvol2 &&!dialvol3) {
	/* AT command set and commands have not been customized */
	/* so combine speaker and volume commands. */
	if (mdmspk)
	  sprintf(lbuf,"ATM1L%d%c",mdmvol,13); /* safe */
	else
	  sprintf(lbuf,"ATM0%c",13);	/* safe */
	ttslow(lbuf,wr);		/* Send command */
	getok(5,1);			/* Get but ignore response */
    } else if (dialsta != DIA_PART) {	/* Customized or not AT commands */
	x = 0;				/* Do it the hard way */
	if (mdmspk) {
	    if (dialspon) {
		if (*dialspon) {
		    x = 1;
		    ttslow(dialspon,wr);
		}
	    } else {
                if (mp->sp_on_str[0]) {
		    x = 1;
		    ttslow(mp->sp_on_str,wr);
		}
	    }
	} else {
	    /* s = dialspoff ? dialspoff : mp->sp_off_str; */
	    if (dialspoff) {
		if (*dialspoff) {
		    x = 1;
		    ttslow(dialspoff,wr);
		}
	    } else {
                if (mp->sp_off_str[0]) {
		    x = 1;
		    ttslow(mp->sp_off_str,wr);
		}
	    }
	}
	if (x) {
	    if (xx_ok)			/* Get response */
	      x = (*xx_ok)(5,1);
	    if (x && mdmspk) {		/* Good response and speaker on? */
		switch (mdmvol) {	/* Yes, send volume command. */
		  case 0:
		  case 1:
		    s = dialvol1 ? dialvol1 : mp->vol1_str; break;
		  case 2:
		    s = dialvol2 ? dialvol2 : mp->vol2_str; break;
		  case 3:
		    s = dialvol3 ? dialvol3 : mp->vol3_str; break;
		  default:
		    s = NULL;
		}
		if (s) if (*s) {	/* Send volume command. */
		    ttslow(s, wr);
		    if (xx_ok)		/* Get response but ignore it */
		      (*xx_ok)(5,1);
		}
	    }
	}
    }

#ifndef CK_ATDT
    /* Dialing Method */

    if (dialmth && dialsta != DIA_PART) { /* If dialing method specified... */
	char *s = "";			/* Do it here... */

	if (dialmth == XYDM_T && dialtone) /* Tone */
	  s = dialtone;
	else if (dialmth == XYDM_P && dialpulse) /* Pulse */
	  s = dialpulse;
	if (s) if (*s) {
	    ttslow(s, wr);
	    if (xx_ok)			/* Get modem's response */
	      (*xx_ok)(5,1);		/* (but ignore it...) */
	}
    }
#endif /* CK_ATDT */

    if (dialidt) {			/* Ignore dialtone? */
	char *s = "";
	s = dialx3 ? dialx3 : mp->ignoredt;
	if (s) if (*s) {
	    ttslow(s, wr);
	    if (xx_ok)			/* Get modem's response */
	      (*xx_ok)(5,1);		/* (but ignore it...) */
	}
    }
    {
	char *s = "";			/* Last-minute init string? */
	s = dialini2 ? dialini2 : mp->ini2;
	if (s) if (*s) {
	    ttslow(s, wr);
	    if (xx_ok)			/* Get modem's response */
	      (*xx_ok)(5,1);		/* (but ignore it...) */
	}
    }
    if (func_code == 1) {		/* ANSWER (not DIAL) */
	char *s;
	s = dialaaon ? dialaaon : mp->aa_on_str;
	if (!s) s = "";
	if (*s) {
	    /* Here we would handle caller ID */
	    ttslow(s, (dialpace > -1) ? wr : mp->dial_rate);
	    if (xx_ok)			/* Get modem's response */
	      (*xx_ok)(5,1);		/* (but ignore it...) */
	} else {
	    printf(
"WARNING - I don't know how to enable autoanswer for this modem.\n"
		   );
	} /* And skip all the phone-number & dialing stuff... */
	alarm(waitct);			/* This much time allowed. */
	debug(F101,"_dodial ANSWER waitct","",waitct);

    } else {				/* DIAL (not ANSWER) */

	if (dialsta != DIA_PART) {	/* Last dial was not partial */

	    char *s = "";
#ifdef COMMENT
	    s = dialaaoff ? dialaaoff : mp->aa_off_str;
#endif /* COMMENT */
	    if (s) if (*s) {
		ttslow(s, (dialpace > -1) ? wr : mp->dial_rate);
		if (xx_ok)		/* Get modem's response */
		  (*xx_ok)(5,1);	/* (but ignore it...) */
	    }

	    /* Put modem into dialing mode, if the modem requires it. */

	    if (mp->dmode_str && *(mp->dmode_str)) {
		ttslow(mp->dmode_str, (dialpace > -1) ? wr : mp->dial_rate);
		savalrm = signal(SIGALRM,dialtime);
		alarm(10);
		/* Wait for prompt, if any expected */
		if (mp->dmode_prompt && *(mp->dmode_prompt)) {
		    waitfor(mp->dmode_prompt);
		    msleep(300);
		}
		alarm(0);		/* Turn off alarm on dialing prompts */
		signal(SIGALRM,savalrm); /* Restore alarm */
	    }
	}
	/* AT-Command-Set non-Generic modem */
	if (mdmcapas & CKD_AT && mymdmtyp != n_GENERIC &&
	    dialsta != DIA_PART) {
	    if (mdmwait > 255)		/* If larger than maximum, */
	      mdmwait = 255;		/* make it maximum. */
	    if (dialesc > 0 &&		/* Modem escape character is set */
		dialmhu > 0) {		/* Hangup method is modem command */
		int x = dialesc;
		if (dialesc < 0 || dialesc > 127)
		  x = 128;
		sprintf(lbuf,
			"ATS2=%dS7=%d\015",
			dialesc ? x : mp->esc_char, mdmwait); /* safe */
	    } else
	      sprintf(lbuf,"ATS7=%d%c",mdmwait,13); /* safe */
	    ttslow(lbuf,wr);		/* Set it. */
	    mdmstat = getok(5,1);	/* Get response from modem */
	    /* If it gets an error, go ahead anyway */
	    debug(F101,"_dodial S7 mdmstat","",mdmstat);
	}
	ttflui();			/* Clear out stuff from modem setup */
	inited = 1;			/* Remember modem is initialized */

      REDIAL:
	if ((int)strlen(dcmd) + (int)strlen(xnum) > LBUFL)
	  ckstrncpy(lbuf,"NUMBER TOO LONG!",LBUFL);
	else
	  sprintf(lbuf, dcmd, xnum);	/* safe (prechecked) */
	debug(F110,"dialing",lbuf,0);
	/* Send the dialing string */
	ttslow(lbuf,dialpace > -1 ? wr : mp->dial_rate);

	fail_code = F_MODEM;		/* New default failure code changes */
	dial_what = DW_DIAL;		/* and our state, too. */
	if (dialdpy) {			/* If showing progress */
	    p = ck_time();		/* get current time; */
	    if (*p) printf(" Dialing: %s...\n",p);
#ifdef VMS
	    printf(" \n");
	    fflush(stdout);
#endif /* VMS */
	}
	alarm(waitct);			/* This much time allowed. */
	debug(F101,"_dodial waitct","",waitct);

#ifndef MINIDIAL
#ifdef OLDMODEMS
	switch (mymdmtyp) {
	  case n_RACAL:			/* Acknowledge dialing string */
	    sleep(3);
	    ttflui();
	    ttoc('\015');
	    break;
	  case n_VENTEL:
	    waitfor("\012\012");	/* Ignore the first two strings */
	    break;
	  default:
	    break;
	}
#endif /* OLDMODEMS */
#endif /* MINIDIAL */
    }

/* Check for connection */

    mdmstat = 0;			/* No status yet */
    lbuf[0] = NUL;			/* Default reason for failure */
    debug(F101,"dial awaiting response, mymdmtyp","",mymdmtyp);

#ifndef NOSPL
    modemmsg[0] = NUL;
#endif /* NOSPL */
    while (mdmstat == 0) {		/* Till we get a result or time out */

	if ((mdmcapas & CKD_AT) && nonverbal) { /* AT command set */
	    gethrn();			/* In digit result mode */
	    if (partial && dialsta == DIA_ERR) {
		/*
		   If we get an error here, the phone is still
		   off hook so we have to hang it up.
		*/
		dialhup();
		dialsta = DIA_ERR;	/* (because dialhup() changes it) */
	    }
	    continue;

	} else if (mymdmtyp == n_UNKNOWN) { /* Unknown modem type */
	    int x, y = waitct;
	    mdmstat = D_FAILED;		/* Assume failure. */
	    while (y-- > -1) {
		x = ttchk();
		if (x > 0) {
		    if (x > LBUFL) x = LBUFL;
		    x = ttxin(x,(CHAR *)lbuf);
		    if ((x > 0) && dialdpy) conol(lbuf);
		} else if (network 
#ifdef TN_COMPORT
                           && !istncomport()
#endif /* TN_COMPORT */
                           && x < 0) { /* Connection dropped */
		    inited = 0;
#ifdef NTSIG
		    ckThreadEnd(threadinfo);
#endif /* NTSIG */
		    dialsta = DIA_IO;	/* Call it an I/O error */
#ifdef DYNAMIC
		    if (rbuf) free(rbuf); rbuf = NULL;
		    if (fbuf) free(fbuf); fbuf = NULL;
#endif /* DYNAMIC */
		    SIGRETURN;
		}
		x = ttgmdm();		/* Try to read modem signals */
		if (x < 0) break;	/* Can't, fail. */
		if (x & BM_DCD) {	/* Got signals OK.  Carrier present? */
		    mdmstat = CONNECTED; /* Yes, done. */
		    break;
		}			/* No, keep waiting. */
		sleep(1);
	    }
	    continue;
	}

	for (n = -1; n < LBUFL-1; ) {	/* Accumulate modem response */
	    int xx;
	    c2 = (char) (xx = ddinc(0)); /* Read a character, blocking */
	    if (xx < 1)			/* Ignore NULs and errors */
	      continue;			/* (Timeout will handle errors) */
	    else			/* Real character, keep it */
	      lbuf[++n] = (char) (c2 & 0177);
	    dialoc(lbuf[n]);		/* Maybe echo it  */
	    if (mdmcapas & CKD_V25) {	/* V.25bis dialing... */
/*
  This assumes that V.25bis indications are all at least 3 characters long
  and are terminated by either CRLF or LFCR.
*/
		if (mymdmtyp == n_CCITT) {
		    if (n < 3) continue;
		    if ((lbuf[n] == CR) && (lbuf[n-1] == LF)) break;
		    if ((lbuf[n] == LF) && (lbuf[n-1] == CR)) break;
		}
#ifndef MINIDIAL
		else if (mymdmtyp == n_DIGITEL) {
		    if (((lbuf[n] == CR) && (lbuf[n-1] == LF)) ||
			((lbuf[n] == LF) && (lbuf[n-1] == CR)))
		      break;
		    else
		      continue;
		}
#endif /* MINIDIAL */
	    } else {			/* All others, break on CR or LF */
		if ( lbuf[n] == CR || lbuf[n] == LF ) break;
	    }
	}
	lbuf[++n] = '\0';		/* Terminate response from modem */
	debug(F111,"_dodial modem response",lbuf,n);
#ifndef NOSPL
	ckstrncpy(modemmsg,lbuf,LBUFL);	/* Call result message */
	lbuf[79] = NUL;
	{
	    int x;			/* Strip junk from end */
	    x = (int)strlen(modemmsg) - 1;
	    while (x > -1) {
		if (modemmsg[x] < (char) 33)
		  modemmsg[x] = NUL;
		else
		  break;
		x--;
	    }
	}
#endif /* NOSPL */
	if (mdmcapas & CKD_AT) {	/* Hayes AT command set */
	    gethrw();			/* in word result mode */
	    if (partial && dialsta == DIA_ERR) {
		dialhup();
		dialsta = DIA_ERR;	/* (because dialhup() changes it) */
	    }
	    continue;
	} else if (mdmcapas & CKD_V25) { /* CCITT command set */
	    if (didweget(lbuf,"VAL")) { /* Dial command confirmation */
#ifndef MINIDIAL
		if (mymdmtyp == n_CCITT)
#endif /* MINIDIAL */
		  continue;		/* Go back and read more */
#ifndef MINIDIAL
/* Digitel doesn't give an explicit connect confirmation message */
		else {
		    int n;
		    for (n = -1; n < LBUFL-1; ) {
			lbuf[++n] = c2 = (char) (ddinc(0) & 0177);
			dialoc(lbuf[n]);
			if (((lbuf[n] == CR) && (lbuf[n-1] == LF)) ||
			    ((lbuf[n] == LF) && (lbuf[n-1] == CR)))
			  break;
		    }
		    mdmstat = CONNECTED; /* Assume we're connected */
		    if (dialdpy && carrier != CAR_OFF) {
#ifdef TN_COMPORT
                        if (istncomport()) {
                            int i;
                            for (i = 0; i < 5; i++) {
                                debug(F100,"TN Com Port DCD wait...","",0);
                                if ((n = ttgmdm()) >= 0) {
                                    if ((n & BM_DCD))
                                        break;
                                    msleep(500);
                                    tnc_wait(
					(CHAR *)"_dodial waiting for DCD",1);
                                }
                            }
                        } else
#endif /* TN_COMPORT */
			  sleep(1); 	/* Wait a second */
			n = ttgmdm();	/* Try to read modem signals */
			if ((n > -1) && ((n & BM_DCD) == 0))
			  printf("WARNING - no carrier\n");
		    }
		}
#endif /* MINIDIAL */

		/* Standard V.25bis stuff */

	    } else if (didweget(lbuf,"CNX")) { /* Connected */
		mdmstat = CONNECTED;
	    } else if (didweget(lbuf, "INV")) {
		mdmstat = D_FAILED;	/* Command error */
		dialsta = DIA_ERR;
		ckstrncpy(lbuf,"INV",LBUFL);

	    } else if (didweget(lbuf,"CFI")) { /* Call Failure */

		if (didweget(lbuf,"AB")) { /* Interpret reason code */
		    ckstrncpy(lbuf,"AB: Timed out",LBUFL);
		    dialsta = DIA_TIMO;
		} else if (didweget(lbuf,"CB")) {
		    ckstrncpy(lbuf,"CB: Local DCE Busy",LBUFL);
		    dialsta = DIA_NRDY;
		} else if (didweget(lbuf,"ET")) {
		    ckstrncpy(lbuf,"ET: Busy",LBUFL);
		    dialsta = DIA_BUSY;
		} else if (didweget(lbuf, "NS")) {
		    ckstrncpy(lbuf,"NS: Number not stored",LBUFL);
		    dialsta = DIA_ERR;
		} else if (didweget(lbuf,"NT")) {
		    ckstrncpy(lbuf,"NT: No answer",LBUFL);
		    dialsta = DIA_NOAN;
		} else if (didweget(lbuf,"RT")) {
		    ckstrncpy(lbuf,"RT: Ring tone",LBUFL);
		    dialsta = DIA_RING;
		} else if (didweget(lbuf,"PV")) {
		    ckstrncpy(lbuf,"PV: Parameter value error",LBUFL);
		    dialsta = DIA_ERR;
		} else if (didweget(lbuf,"PS")) {
		    ckstrncpy(lbuf,"PS: Parameter syntax error",LBUFL);
		    dialsta = DIA_ERR;
		} else if (didweget(lbuf,"MS")) {
		    ckstrncpy(lbuf,"MS: Message syntax error",LBUFL);
		    dialsta = DIA_ERR;
		} else if (didweget(lbuf,"CU")) {
		    ckstrncpy(lbuf,"CU: Command unknown",LBUFL);
		    dialsta = DIA_ERR;
		} else if (didweget(lbuf,"FC")) {
		    ckstrncpy(lbuf,"FC: Forbidden call",LBUFL);
		    dialsta = DIA_NOAC;
		}
		mdmstat = D_FAILED;
	    } else if (didweget(lbuf,"INC")) { /* Incoming Call */
		ckstrncpy(lbuf,"INC: Incoming call",LBUFL);
		dialsta = DIA_RING;
		mdmstat = D_FAILED;
	    } else if (didweget(lbuf,"DLC")) { /* Delayed Call */
		ckstrncpy(lbuf,"DLC: Delayed call",LBUFL);
		dialsta = DIA_NOAN;
		mdmstat = D_FAILED;
	    } else			/* Response was probably an echo. */
#ifndef MINIDIAL
	      if (mymdmtyp == n_CCITT)
#endif /* MINIDIAL */
		continue;
#ifndef MINIDIAL
	      else			/* Digitel: If no error, connect. */
		mdmstat = CONNECTED;
#endif /* MINIDIAL */
	    break;

	} else if (n) {			/* Non-Hayes-compatibles... */
	    switch (mymdmtyp) {
#ifndef MINIDIAL
	      case n_ATTMODEM:
		/* Careful - "Connected" / "Not Connected" */
		if (didweget(lbuf,"Busy")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_BUSY;
		} else if (didweget(lbuf,"Not connected") ||
			   didweget(lbuf,"Not Connected")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_NOCA;
		} else if (didweget(lbuf,"No dial tone") ||
			   didweget(lbuf,"No Dial Tone")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_NODT;
		} else if (didweget(lbuf,"No answer") ||
			   didweget(lbuf,"No Answer")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_NOAN;
		} else if (didweget(lbuf,"Answered") ||
			   didweget(lbuf,"Connected")) {
		    mdmstat = CONNECTED;
		    dialsta = DIA_OK;
		}
		break;

	      case n_ATTISN:
		if (didweget(lbuf,"ANSWERED")) {
		    mdmstat = CONNECTED;
		    dialsta = DIA_OK;
		} else if (didweget(lbuf,"BUSY")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_BUSY;
		} else if (didweget(lbuf,"DISCONNECT")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_DISC;
		} else if (didweget(lbuf,"NO ANSWER")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_NOAN;
		} else if (didweget(lbuf,"WRONG ADDRESS")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_NOAC;
		}
		break;

	      case n_ATTDTDM:
		if (didweget(lbuf,"ANSWERED")) {
		    mdmstat = CONNECTED;
		} else if (didweget(lbuf,"BUSY")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_BUSY;
		} else if (didweget(lbuf,"CHECK OPTIONS")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_ERR;
		} else if (didweget(lbuf,"DISCONNECTED")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_DISC;
		} else if (didweget(lbuf,"DENIED")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_NOAC;
		}
#ifdef DEBUG
#ifdef ATT6300
		/* Horrible hack lost in history. */
		else if (deblog && didweget(lbuf,"~~"))
		  mdmstat = CONNECTED;
#endif /* ATT6300 */
#endif /* DEBUG */
		break;

#ifdef OLDMODEMS
	      case n_CERMETEK:
		if (didweget(lbuf,"\016A")) {
		    mdmstat = CONNECTED;
		    ttslow("\016U 1\015",200); /* Make transparent*/
		}
		break;

	      case n_DF03:
		/* Because response lacks CR or NL . . . */
		c = (char) (ddinc(0) & 0177);
		dialoc(c);
		debug(F000,"dial df03 got","",c);
		if ( c == 'A' ) mdmstat = CONNECTED;
		if ( c == 'B' ) mdmstat = D_FAILED;
		break;

	      case n_DF100:	     /* DF100 has short response codes */
		if (strcmp(lbuf,"A") == 0) {
		    mdmstat = CONNECTED; /* Attached */
		    dialsta = DIA_OK;
		} else if (strcmp(lbuf,"N") == 0) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_NOAN; /* No answer or no dialtone */
		} else if (strcmp(lbuf,"E") == 0 || /* Error */
			   strcmp(lbuf,"R") == 0) { /* "Ready" (?) */
		    mdmstat = D_FAILED;
		    dialsta = DIA_ERR;	/* Command error */
		}
		/* otherwise fall thru... */

	      case n_DF200:
		if (didweget(lbuf,"Attached")) {
		    mdmstat = CONNECTED;
		    dialsta = DIA_OK;
		    /*
		     * The DF100 will respond with "Attached" even if DTR
		     * and/or carrier are not present.	Another reason to
		     * (also) wait for carrier?
		     */
		} else if (didweget(lbuf,"Busy")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_BUSY;
		} else if (didweget(lbuf,"Disconnected")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_DISC;
		} else if (didweget(lbuf,"Error")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_ERR;
		} else if (didweget(lbuf,"No answer")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_NOAN;
		} else if (didweget(lbuf,"No dial tone")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_NODT;
		} else if (didweget(lbuf,"Speed:)")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_ERR;
		}
		/*
		 * It appears that the "Speed:..." response comes after an
		 * "Attached" response, so this is never seen.  HOWEVER,
		 * it would be very handy to detect this and temporarily
		 * reset the speed, since it's a nuisance otherwise.
		 * If we wait for some more input from the modem, how do
		 * we know if it's from the remote host or the modem?
		 * Carrier reportedly doesn't get set until after the
		 * "Speed:..." response (if any) is sent.  Another reason
		 * to (also) wait for carrier.
		 */
		break;

	      case n_GDC:
		if (didweget(lbuf,"ON LINE"))
		  mdmstat = CONNECTED;
		else if (didweget(lbuf,"NO CONNECT"))
		  mdmstat = D_FAILED;
		break;

	      case n_PENRIL:
		if (didweget(lbuf,"OK")) {
		    mdmstat = CONNECTED;
		} else if (didweget(lbuf,"BUSY")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_BUSY;
		    } else if (didweget(lbuf,"NO RING")) {
			mdmstat = D_FAILED;
			dialsta = DIA_NOCA;
		    }
		break;

	      case n_RACAL:
		if (didweget(lbuf,"ON LINE"))
		  mdmstat = CONNECTED;
		else if (didweget(lbuf,"FAILED CALL"))
		  mdmstat = D_FAILED;
		break;
#endif /* OLDMODEMS */

	      case n_ROLM:
		if (didweget(lbuf,"CALLING"))
		  mdmstat = 0;
		else if (didweget(lbuf,"COMPLETE"))
		  mdmstat = CONNECTED;
		else if (didweget(lbuf,"FAILED") ||
			 didweget(lbuf,"ABANDONDED")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_NOCA;
		} else if (didweget(lbuf,"NOT AVAILABLE") ||
			   didweget(lbuf,"LACKS PERMISSION") ||
			   didweget(lbuf,"NOT A DATALINE") ||
			   didweget(lbuf,"INVALID DATA LINE NUMBER") ||
			   didweget(lbuf,"INVALID GROUP NAME")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_NOAC;
		} else if (didweget(lbuf,"BUSY")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_BUSY;
		} else if (didweget(lbuf,"DOES NOT ANSWER")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_NOAN;
		}
		break;

#ifdef OLDMODEMS
	      case n_VENTEL:
		if (didweget(lbuf,"ONLINE!") ||
		    didweget(lbuf,"Online!")) {
		    mdmstat = CONNECTED;
		} else if (didweget(lbuf,"BUSY") ||
			   didweget(lbuf,"Busy")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_BUSY;
		} else if (didweget(lbuf,"DEAD PHONE")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_DISC;
		}
		break;

	      case n_CONCORD:
		if (didweget(lbuf,"INITIATING"))
		  mdmstat = CONNECTED;
		else if (didweget(lbuf,"BUSY")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_BUSY;
		} else if (didweget(lbuf,"CALL FAILED")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_NOCA;
		}
		break;
#endif /* OLDMODEMS */

	      case n_MICROCOM:
		/* "RINGBACK" means phone line ringing, continue */
		if (didweget(lbuf,"NO CONNECT")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_NOCA;
		} else if (didweget(lbuf,"BUSY")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_BUSY;
		} else if (didweget(lbuf,"NO DIALTONE")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_NODT;
		} else if (didweget(lbuf,"COMMAND ERROR")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_ERR;
		} else if (didweget(lbuf,"IN USE")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_NOAC;
		} else if (didweget(lbuf,"CONNECT")) {
		    mdmstat = CONNECTED;
		    /* trailing speed ignored */
		}
		break;

#endif /* MINIDIAL */
	      default:
		printf(
		    "PROGRAM ERROR - No response handler for modem type %d\n",
		       mymdmtyp);
		mdmstat = D_FAILED;
		dialsta = DIA_ERR;
	    }
	}
    } /* while (mdmstat == 0) */

    debug(F101,"_dodial alarm off","",x);
    alarm(0);
    if (mdmstat == D_FAILED) {		/* Failure detected by modem  */
        dialfail(F_MODEM);
#ifdef NTSIG
	ckThreadEnd(threadinfo);
#endif /* NTSIG */
        SIGRETURN;
    } else if (mdmstat == D_PARTIAL )	{ /* Partial dial command OK */
	msleep(500);
	debug(F100,"dial partial","",0);
    } else {				/* Call was completed */
	int x;
	msleep(700);			/* In case modem signals blink  */
	debug(F100,"dial succeeded","",0);
	if (
#ifndef MINIDIAL
	    mymdmtyp != n_ROLM		/* Rolm has weird modem signaling */
#else
	    1
#endif /* MINIDIAL */
	    ) {
	    alarm(3);			/* In case ttpkt() gets stuck... */
	    ttpkt(speed,FLO_DIAX,parity); /* Cancel dialing state ioctl */
            alarm(0);
	}
/*
  In case CD went off in the interval between call completion and return
  from ttpkt()...
*/
	if (carrier != CAR_OFF) {
            if ((x = ttgmdm()) >= 0) {
#ifdef TN_COMPORT
                if (istncomport() && !(x & BM_DCD)) {
                    int i;
                    for (i = 0; i < 5; i++) {
                        msleep(500);
                        tnc_wait((CHAR *)"_dodial waiting for DCD",1);
                        if ((x = ttgmdm()) >= 0) {
                            if ((x & BM_DCD))
                                break;
                        }
                    }
                }
#endif /* TN_COMPORT */
                if (!(x & BM_DCD))
		  printf("WARNING: Carrier seems to have dropped...\n");
            }
        }
    }
    dreset();				/* Reset alarms and signals. */
    if (!quiet && !backgrd) {
	if (dialdpy && (p = ck_time())) { /* If DIAL DISPLAY ON, */
	    printf(" %sall complete: %s.\n", /* include timestamp.  */
		   (mdmstat == D_PARTIAL) ?
		   "Partial c" :
		   "C",
		   p );
	} else if (modemmsg[0]) {
	    printf (" %sall complete: \"%s\".\n",
		    (mdmstat == D_PARTIAL) ? "Partial c" : "C",
		    (char *)modemmsg
		    );
	} else {
	    printf (" %sall complete.\n",
		    (mdmstat == D_PARTIAL) ?
		    "Partial c" :
		    "C"
		    );
	}
    }
#ifdef CKLOGDIAL
    dologdial(telnbr);
#endif /* CKLOGDIAL */

#ifdef DYNAMIC
    if (rbuf) free(rbuf); rbuf = NULL;
    if (fbuf) free(fbuf); fbuf = NULL;
#endif /* DYNAMIC */
    dialsta = (mdmstat == D_PARTIAL) ? DIA_PART : DIA_OK;
#ifdef NTSIG
    ckThreadEnd(threadinfo);
#endif /* NTSIG */
    SIGRETURN;
}

static SIGTYP
#ifdef CK_ANSIC
faildial(void * threadinfo)
#else /* Not CK_ANSIC */
faildial(threadinfo) VOID * threadinfo;
#endif /* CK_ANSIC */
/* faildial */ {
    debug(F100,"longjmp returns to dial routine","",0);
    dialfail(fail_code);
    SIGRETURN;
}

/*
  nbr = number to dial (string)
  x1  = Retry counter
  x2  = Number counter
  fc  = Function code:
        0 == DIAL
        1 == ANSWER
        2 == INIT/CONFIG
        3 == PARTIAL DIAL
*/

int
#ifdef OLD_DIAL
ckdial(nbr) char *nbr;
#else
ckdial(nbr, x1, x2, fc, redial) char *nbr; int x1, x2, fc, redial;
#endif /* OLD_DIAL */
/* ckdial */ {
#define ERMSGL 50
    char errmsg[ERMSGL], *erp;		/* For error messages */
    int n = F_TIME;
    char *s;
    long spdmax;
#ifdef OS2
    extern int term_io;
    int term_io_sav = term_io;
#endif /* OS2 */

    char *mmsg = "Sorry, DIAL memory buffer can't be allocated\n";
    /*
      A DIAL command implies a SET MODEM TYPE command and therefore enables
      hanging up by modem commands rather than dropping DTR.
    */
    mdmset = 1;				/* See mdmhup() */

    partial = 0;
    if (fc == 3) {			/* Partial dial requested */
	partial = 1;			/* Set flag */
	fc = 0;				/* Treat like regular dialing */
    }
    func_code = fc;			/* Make global to this module */
    telnbr = nbr;
    xredial = redial;
    debug(F111,"ckdial entry partial",ckitoa(fc),partial);
    debug(F111,"ckdial entry number",nbr,redial);

    if (fc == 1) {			/* ANSWER command? */
	/* Reset caller ID strings */
	if (callid_date) makestr(&callid_date,NULL);
	if (callid_time) makestr(&callid_time,NULL);
	if (callid_name) makestr(&callid_name,NULL);
	if (callid_nmbr) makestr(&callid_nmbr,NULL);
	if (callid_mesg) makestr(&callid_mesg,NULL);
    }

#ifdef CK_TAPI_X
    if (tttapi && tapipass) {
	if (modemp[n_TAPI] = cktapiGetModemInf()) {
	    mymdmtyp = n_TAPI;
	} else {
	    mymdmtyp = mdmtyp;
	    modemp[n_TAPI] = &GENERIC;
	}
    } else
#endif /* CK_TAPI */
    mymdmtyp = mdmtyp;
    if (mymdmtyp < 0) {			/* Whoa, network dialing... */
	if (mdmsav > -1)
	  mymdmtyp = mdmsav;
    }
    if (mymdmtyp < 0) {
	printf("Invalid modem type %d - internal error.\n",mymdmtyp);
	dialsta = DIA_NOMO;
	return(0);
    }
    dial_what = DW_NOTHING;		/* Doing nothing at first. */
    nonverbal = 0;

/* These are ONLY for the purpose of interpreting numeric result codes. */

    is_motorola =
#ifdef MINIDIAL
      0
#else
      mymdmtyp == n_SUPRA || mymdmtyp == n_SUPRASON;
#endif /* MINIDIAL */
	;

    is_motorola =
#ifdef MINIDIAL
      0
#else
      mymdmtyp == n_MOTOROLA || mymdmtyp == n_MONTANA;
#endif /* MINIDIAL */
	;

    is_rockwell =
#ifdef MINIDIAL
      0
#else
      mymdmtyp == n_RWV32 || mymdmtyp == n_RWV32B ||
	mymdmtyp == n_RWV34 || mymdmtyp == n_RWV90 ||
	  mymdmtyp == n_BOCA || mymdmtyp == n_TELEPATH ||
	    mymdmtyp == n_CARDINAL || mymdmtyp == n_BESTDATA ||
	      mymdmtyp == n_CONEXANT || mymdmtyp == n_PCTEL
#endif /* MINIDIAL */
	;

    is_hayeshispd =
#ifdef MINIDIAL
      0
#else
      mymdmtyp == n_H_ULTRA || mymdmtyp == n_H_ACCURA || n_PPI
#endif /* MINIDIAL */
	;

    is_supra =
#ifdef MINIDIAL
      0
#else
      mymdmtyp == n_SUPRA || mymdmtyp == n_SUPRAX || n_SUPRASON
#endif /* MINIDIAL */
	;

    mp = modemp[mymdmtyp];		/* Set pointer to modem info */
    if (!mp) {
	printf("Sorry, handler for this modem type not yet filled in.\n");
	dialsta = DIA_NOMO;
	return 0;
    }
    debug(F110,"dial number",telnbr,0);
#ifdef COMMENT
    debug(F110,"dial prefix",(dialnpr ? dialnpr : ""), 0);
#endif /* COMMENT */

#ifdef DYNAMIC
    *lbuf = NUL;
    debug(F101,"DIAL lbuf malloc ok","",LBUFL+1);

    if (!rbuf) {    /* This one might already have been allocated by getok() */
	if (!(rbuf = malloc(RBUFL+1))) {    /* Allocate input line buffer */
	    printf("%s", mmsg);
	    dialsta = DIA_IE;
	    return 0;
	} else
	  debug(F101,"DIAL rbuf malloc ok","",RBUFL+1);
    }
    if (!(fbuf = malloc(FULLNUML+1))) {    /* Allocate input line buffer */
	printf("%s", mmsg);
	dialsta = DIA_IE;
	if (rbuf) free(rbuf); rbuf = NULL;
	return 0;
    }
    debug(F101,"DIAL fbuf malloc ok","",FULLNUML+1);
#endif /* DYNAMIC */

    /* NOTE: mdmtyp, not mymdmtyp */

    if (ttopen(ttname,&local,mdmtyp,0) < 0) { /* Open, no carrier wait */
	erp = errmsg;
	if ((int)strlen(ttname) < (ERMSGL - 18)) /* safe, checked */
	  sprintf(erp,"Sorry, can't open %s",ttname);
	else
	  sprintf(erp,"Sorry, can't open device");
	perror(errmsg);
	dialsta = DIA_OPEN;
#ifdef DYNAMIC
	if (rbuf) free(rbuf); rbuf = NULL;
	if (fbuf) free(fbuf); fbuf = NULL;
#endif /* DYNAMIC */
	return 0;
    }

#ifdef CK_TAPI
    if (!tttapi) {
#endif /* CK_TAPI */

/* Condition console terminal and communication line */

    /* Place line into "clocal" dialing state, */
    /* important mainly for System V UNIX.     */

    if (ttpkt(speed,FLO_DIAL,parity) < 0) {
	ttclos(0);			/* If ttpkt fails do all this... */
	if (ttopen(ttname,&local,mymdmtyp,0) < 0) {
	    erp = errmsg;
	    if ((int)strlen(ttname) < (ERMSGL - 18)) /* safe, checked */
	      sprintf(erp,"Sorry, can't reopen %s",ttname);
	    else
	      sprintf(erp,"Sorry, can't reopen device");
	    perror(errmsg);
	    dialsta = DIA_OPEN;
#ifdef DYNAMIC
	    if (rbuf) free(rbuf); rbuf = NULL;
	    if (fbuf) free(fbuf); fbuf = NULL;
#endif /* DYNAMIC */
	    return 0;
	}				/* And try again. */
	if ((ttpkt(speed,FLO_DIAL,parity) < 0)
#ifdef UNIX
	&& (strcmp(ttname,"/dev/null"))
#else
#ifdef OSK
	&& (strcmp(ttname,"/nil"))
#endif /* OSK */
#endif /* UNIX */
#ifdef CK_TAPI
	     && !tttapi
#endif /* CK_TAPI */
	    ) {
	    printf("Sorry, Can't condition communication line\n");
	    printf("Try 'set line %s' again\n",ttname);
	    dialsta = DIA_OPEN;
#ifdef DYNAMIC
	    if (rbuf) free(rbuf); rbuf = NULL;
	    if (fbuf) free(fbuf); fbuf = NULL;
#endif /* DYNAMIC */
	    return 0;
	}
    }
#ifdef CK_TAPI
    }
#endif /* CK_TAPI */

    /* Modem's escape sequence... */

    if (dialesc < 0 || dialesc > 127)
      c = NUL;
    else
      c = (char) (dialesc ? dialesc : mp->esc_char);
    mdmcapas = dialcapas ? dialcapas : mp->capas;

    xx_ok = mp->ok_fn;			/* Pointer to response reader */

    if (mdmcapas & CKD_AT) {		/* Hayes compatible */
	escbuf[0] = c;
	escbuf[1] = c;
	escbuf[2] = c;
	escbuf[3] = NUL;
	/* In case this modem type is user-defined */
	if (!xx_ok) xx_ok = getok;
    } else {				/* Other */
	escbuf[0] = c;
	escbuf[1] = NUL;
	/* In case user-defined */
	if (mdmcapas & CKD_V25) if (!xx_ok) xx_ok = getok;
    }

    /* Partial dialing */

    if (mdmcapas & CKD_AT
#ifndef MINIDIAL
	|| mymdmtyp == n_MICROCOM
#endif /* MINIDIAL */
	) {
	int x;
	x = (int) strlen(telnbr);
	if (x > 0) {
	    if (telnbr[x-1] == ';') {
		partial = 1;
		debug(F110,"ckdial sets partial=1:",telnbr,0);
	    } else if (partial) {
		ckmakmsg(fbuf,FULLNUML,telnbr,";",NULL,NULL); /* add one */
		telnbr = fbuf;
	    }
	}
    }
    msleep(500);

    debug(F101,"ckdial dialtmo","",dialtmo); /* Timeout */

    if (fc == 1) {			/* ANSWER */
	waitct = (dialatmo > -1) ? dialatmo : 0;
    } else {				/* DIAL */
	if (dialtmo < 1) {		/* Automatic computation. */
#ifdef CK_TAPI
	    if (tttapi && !tapipass) {
		waitct = 1 * (int)strlen(telnbr) ; /* Worst case dial time */
		waitct += 60;		/* dialtone + completion wait times */
		for (s = telnbr; *s; s++) { /* add in pause characters time */
		    if (*s == ',') {
			waitct += 2; /* unless it was changed in the modem */
		    } else if (*s == 'W' ||
			       *s == 'w' ||
			       *s == '$' ||
			       *s == '@'
			       ) {
			waitct += 8;
		    }
		}
	    } else {
#endif /* CK_TAPI */
		waitct = 1 * (int)strlen(telnbr) ;
		/* dialtone + completion wait times */
		waitct += mp->dial_time;
		for (s = telnbr; *s; s++) {
		    for (p = mp->pause_chars; *p; p++)
		      if (*s == *p) {
			  waitct += mp->pause_time;
			  break;
		      }
		}
#ifdef CK_TAPI
	    }
#endif /* CK_TAPI */
	} else {
	    waitct = dialtmo;		/* User-specified timeout */
	}
	debug(F101,"ckdial waitct A","",waitct);
    }

/*
  waitct is our alarm() timer.
  mdmwait is how long we tell the modem to wait for carrier.
  We set mdmwait to be 5 seconds less than waitct, to increase the
  chance that we get a response from the modem before timing out.
*/
    if (waitct <= 0) {			/* 0 or negative means wait forever  */
#ifdef COMMENT
	waitct = 254;			/* These were backwards in 7.0.196 */
	mdmwait = 0;
#else
	waitct = 0;			/* Fixed in 7.0.198. */
	mdmwait = 254;
#endif /* COMMENT */
    } else {
	if (dialtmo < 1) {		/* Automatic computation. */
#ifdef XWAITCT
	    /* Addtl wait slop can be defined at compile time */
	    waitct += XWAITCT;
#endif /* XWAITCT */
	    if (waitct < 60 + mdmwaitd)
	      waitct = 60 + mdmwaitd;
	}
	if (mdmcapas & CKD_AT) {	/* AT command-set modems */
	    mdmwait = waitct;		/* S7 timeout = what user asked for */
	    waitct += mdmwaitd;		/* Kermit timeout a bit later */
	} else {			/* Non-AT */
	    mdmwait = waitct;		/* no difference */
	}
    }
    debug(F101,"ckdial waitct B","",waitct);
    if (fc == 1) {			/* ANSWER */
#ifdef COMMENT
/*
  This is wrong.  mdmwait is the value given to S7 in Hayeslike modems.
  When in autoanswer mode, this is the amount of time the modem waits for
  carrier once ringing starts.  Whereas waitct is the timeout given to the
  ANSWER command, which is an entirely different thing.  Since the default
  ANSWER timeout is 0 (meaning "wait forever"), the following statement sets
  S7 to 0, which, on some modems (like the USR Sportster) makes it hang up
  and report NO CARRIER the instant the phone rings.
*/
	mdmwait = waitct;
#else
	if (mdmwait <= 0)
	  mdmwait = 60;			/* Always wait 60 seconds. */
#endif /* COMMENT */

    }
    if (!quiet && !backgrd) {		/* Print information messages. */
#ifdef VMS
	printf(" \n");
	fflush(stdout);
#endif /* VMS */
	if (fc == 1)
	  printf(" Waiting for phone call...\n");
	else
	  printf(" %srying: %s...\n", x1 > 0 ? "Ret" : "T", telnbr);
	if (x1 == 0 && x2 == 0 && dialsta != DIA_PART) {
	    if (network) {
		printf(" Via modem server: %s, modem: %s\n",
		       ttname, gmdmtyp() );
	    } else {
#ifdef CK_TAPI
		if (tttapi && !tapipass)
		  printf(" Device: %s, modem: %s", ttname, "TAPI" );
		else
#endif /* CK_TAPI */
		printf(" Device: %s, modem: %s",
		       ttname, gmdmtyp() );
		if (speed > -1L)
		  printf(", speed: %ld\n", speed);
		else
		  printf(", speed: (unknown)\n");
	    }
	    spdmax = dialmax > 0L ? dialmax : mp->max_speed;

#ifndef NOHINTS
	    if (hints && !quiet &&
		!network && spdmax > 0L && speed > spdmax
#ifdef CK_TAPI
		&& (!tttapi || tapipass)
#endif /* CK_TAPI */
		) {
		printf("\n*************************\n");
		printf(
    "Interface speed %ld might be too high for this modem type.\n",
		       speed
		       );
		printf(
    "If dialing fails, SET SPEED to %ld or less and try again.\n",
		       spdmax
		       );
		printf("(Use SET HINTS OFF to suppress future hints.)\n");
		printf("*************************\n");
		printf("\n");
	    }
#endif /* NOHINTS */
	    printf(" %s timeout: ", fc == 0 ? "Dial" : "Answer");
	    if (waitct > 0)
	      printf("%d seconds\n",mdmwait);
	    else
	      printf(" (none)\n");
	    printf(
#ifdef MAC
	       " Type Command-. to cancel.\n"
#else
#ifdef UNIX
	       " To cancel: type your interrupt character (normally Ctrl-C).\n"
#else
	       " To cancel: type Ctrl-C (hold down Ctrl, press C).\n"
#endif /* UNIX */
#endif /* MAC */
	       );
        }
    }
    debug(F111,"ckdial",ttname,(int) (speed / 10L));
    debug(F101,"ckdial timeout","",waitct);
#ifdef OS2
    term_io = 0;
#endif /* OS2 */

/* Set timer and interrupt handlers. */
    savint = signal( SIGINT, dialint ) ; /* And terminal interrupt handler. */
    cc_alrm_execute(ckjaddr(sjbuf), 0, dialtime, _dodial, faildial);

    signal(SIGINT, savint);
#ifdef OS2
    if (dialsta == DIA_OK)		/* Dialing is completed */
      DialerSend(OPT_KERMIT_CONNECT, 0);
    term_io = term_io_sav;
#endif /* OS2 */
    if (dialsta == DIA_PART || dialsta == DIA_OK) {
	/* This is needed, e.g., for Telnet modem servers */
	if (reliable != SET_OFF || !setreliable) {
	    reliable = SET_OFF;		/* Transport is not reliable */
	    debug(F101,"ckdial reliable","",reliable);
	}
	return(1);			/* Dial attempt succeeded */
    } else {
	return(0);			/* Dial attempt failed */
    }
} /* ckdial */

/*
  getok() - wait up to n seconds for OK (0) or ERROR (4) response from modem.
  Use with Hayeslike or CCITT modems for reading the reply to a nondialing
  command.

  Second argument says whether to be strict about numeric result codes, i.e.
  to require they be preceded by CR or else be the first character in the
  response, e.g. to prevent the ATH0<CR> echo from looking like a valid
  response.  Strict == 0 is needed for ATI on Telebit, which can return the
  model number concatenated with the numeric response code, e.g. "9620"
  ("962" is the model number, "0" is the response code).  getok() Returns:

   0 if it timed out,
   1 if it succeeded,
  -1 on modem command, i/o, or other error.
*/
static ckjmpbuf okbuf;

static SIGTYP
#ifdef CK_ANSIC
oktimo(int foo)				/* Alarm handler for getok(). */
#else
oktimo(foo) int foo;			/* Alarm handler for getok(). */
#endif /* CK_ANSIC */
/* oktimo */ {

#ifdef OS2
    alarm(0);
    /* signal(SIGALRM,SIG_IGN); */
    debug(F100,"oktimo() SIGALRM caught -- SIG_IGN set","",0) ;
#endif /* OS2 */

#ifdef OSK				/* OS-9, see comment in dialtime(). */
    sigmask(-1);
#endif /* OSK */
#ifdef NTSIG
    if ( foo == SIGALRM )
      PostAlarmSigSem();
    else
      PostCtrlCSem();
#else /* NTSIG */
#ifdef NT
    cklongjmp(ckjaddr(okbuf),1);
#else /* NT */
    cklongjmp(okbuf,1);
#endif /* NTSIG */
#endif /* NT */
    /* NOTREACHED */
    SIGRETURN;
}

static int okstatus, okn, okstrict;

static SIGTYP
#ifdef CK_ANSIC
dook(void * threadinfo)
#else /* CK_ANSIC */
dook(threadinfo) VOID * threadinfo ;
#endif /* CK_ANSIC */
/* dook */ {
    CHAR c;
#ifdef DEBUG
    char * mdmmsg = "";
#endif /* DEBUG */

    int i, x;
#ifdef IKSD
    extern int inserver;
#endif /* IKSD */
#ifdef NTSIG
    signal(SIGINT,oktimo);
    if (threadinfo) {			/* Thread local storage... */
	TlsSetValue(TlsIndex,threadinfo);
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

    if (mdmcapas & CKD_V25) {		/* CCITT, easy... */
        waitfor("VAL");
        okstatus = 1;
	debug(F111,"Modem_Response(V25)","VAL",okstatus);
#ifdef NTSIG
	ckThreadEnd(threadinfo);
#endif /* NTSIG */
	SIGRETURN;
#ifndef MINIDIAL
    } else if (mymdmtyp == n_MICROCOM) { /* Microcom in SX mode, also easy */
        waitfor(MICROCOM.wake_prompt);	/* (I think...) */
	debug(F111,"Modem_Response(Microcom)",MICROCOM.wake_prompt,okstatus);
        okstatus = 1;
#ifdef NTSIG
	ckThreadEnd(threadinfo);
#endif /* NTSIG */
	SIGRETURN;
#endif /* MINIDIAL */
    } else {				/* Hayes & friends, start here... */
	okstatus = 0;			/* No status yet. */
	for (x = 0; x < RBUFL; x++)	/* Initialize response buffer */
	  rbuf[x] = SP;			/*  to all spaces */
	rbuf[RBUFL] = NUL;		/* and terminate with NUL. */
	while (okstatus == 0) {		/* While no status... */
	    x = ddinc(okn);		/* Read a character */
	    if (x < 0) {		/* I/O error */
		okstatus = -1;
#ifdef NTSIG
		ckThreadEnd(threadinfo);
#endif /* NTSIG */
		SIGRETURN;
	    }
#ifdef COMMENT
	    /* too much */
	    debug(F101,"getok ddinc","",x); /* Got a character. */
#endif /* COMMENT */
	    c = (char) (x & 0x7f);	/* Get low order 7 bits */
	    if (!c)			/* Don't deposit NULs */
	      continue;			/* or else didweget() won't work */
	    if (dialdpy) conoc((char)c); /* Echo it if requested */
	    for (i = 0; i < RBUFL-1; i++) /* Rotate buffer */
	      rbuf[i] = rbuf[i+1];
	    rbuf[RBUFL-1] = c;		/* Deposit character at end */
#ifdef COMMENT
	    /* too much */
	    debug(F000,"getok:",rbuf,(int) c); /* Log it */
#endif /* COMMENT */
	    switch (c) {		/* Interpret it. */
	      case CR:			/* Got a carriage return. */
		switch(rbuf[RBUFL-2]) {	/* Look at character before it. */
		  case '0':		/* 0 = OK numeric response */
		    if (!okstrict ||
			rbuf[RBUFL-3] == CR || rbuf[RBUFL-3] == SP) {
			nonverbal = 1;
			okstatus = 1;	/* Good response */
		    }
		    debug(F111,"Modem_Response(Hayes)","0",okstatus);
		    break;
		  case '4':		/* 4 = ERROR numeric response */
#ifndef MINIDIAL
		    /* Or Telebit model number 964! */
		    if (mymdmtyp == n_TELEBIT &&
			isdigit(rbuf[RBUFL-3]) &&
			isdigit(rbuf[RBUFL-4]))
		      break;
		    else
#endif /* MINIDIAL */
		      if (!okstrict ||
			rbuf[RBUFL-3] == CR || rbuf[RBUFL-3] == SP) {
			nonverbal = 1;
			okstatus = -1;	/* Bad command */
		    }
		    debug(F111,"Modem_Response(Hayes)","4",okstatus);
		    break;
		}
		if (dialdpy && nonverbal) /* If numeric results, */
		  conoc(LF);		  /* echo a linefeed too. */
		break;
	      case LF:			/* Got a linefeed. */
		/*
		  Note use of explicit octal codes in the string for
		  CR and LF.  We want real CR and LF here, not whatever
		  the compiler happens to replace \r and \n with...
		*/
		if (!strcmp(rbuf+RBUFL-4,"OK\015\012")) { /* Good response */
		    okstatus = 1;
		    debug(F111,"Modem_Response(Hayes)","OK",okstatus);
		}
		if (!strcmp(rbuf+RBUFL-3,"OK\012")) { /* Good response */
		    okstatus = 1;
		    debug(F111,"Modem_Response(Hayes)","OK",okstatus);
		} else if (!strcmp(rbuf+RBUFL-7,"ERROR\015\012")) { /* Error */
		    okstatus = -1;
		    debug(F111,"Modem_Response(Hayes)","ERROR",okstatus);
		} else if (!strcmp(rbuf+RBUFL-6,"ERROR\012")) { /* Error */
		    okstatus = -1;
		    debug(F111,"Modem_Response(Hayes)","ERROR",okstatus);
		}
		break;
	      /* Check whether modem echoes its commands... */
	      case 't':			/* Got little t */
		if (!strcmp(rbuf+RBUFL-3,"\015at") || /* See if it's "at" */
		    !strcmp(rbuf+RBUFL-3," at"))
		    mdmecho = 1;
		/* debug(F111,"MDMECHO-t",rbuf+RBUFL-2,mdmecho); */
		break;
	      case 'T':			/* Got Big T */
		if (!strcmp(rbuf+RBUFL-3,"\015AT") ||	/* See if it's "AT" */
		    !strcmp(rbuf+RBUFL-3," AT"))
		    mdmecho = 1;
		/* debug(F111,"MDMECHO-T",rbuf+RBUFL-3,mdmecho); */
		break;
	      default:			/* Other characters, accumulate. */
		okstatus = 0;
		break;
	    }
	}
    }
    debug(F101,"getok","",okstatus);	/* <-- It's a lie (why?) */
#ifdef NTSIG
    ckThreadEnd(threadinfo);
#endif /* NTSIG */
    SIGRETURN;
}

static SIGTYP
#ifdef CK_ANSIC
failok(void * threadinfo)
#else /* CK_ANSIC */
failok(threadinfo) VOID * threadinfo;
#endif /* CK_ANSIC */
/* failok */ {
    debug(F100,"longjmp returned to getok()","",0);
    debug(F100,"getok timeout","",0);
    SIGRETURN;
}

int
getok(n, strict) int n, strict; {
    debug(F101,"getok entry n","",n);
    okstatus = 0;
    okn = n;
    okstrict = strict;

#ifdef DYNAMIC
    if (!rbuf) {
	if (!(rbuf = malloc(RBUFL+1))) { /* Allocate input line buffer */
	    dialsta = DIA_IE;
	    return(-1);
	}
	debug(F101,"GETOK rbuf malloc ok","",RBUFL+1);
    }
#endif /* DYNAMIC */

    mdmecho = 0;			/* Assume no echoing of commands */

    debug(F100,"about to alrm_execute dook()","",0);
    alrm_execute( ckjaddr(okbuf), n, oktimo, dook, failok ) ;
    debug(F100,"returning from alrm_execute dook()","",0);

    ttflui();				/* Flush input buffer */
    return(okstatus);			/* Return status */
}

/*  G E T H R N  --  Get Hayes Result Numeric  */

static VOID
gethrn() {
    char c;
    int x;
/*
  Hayes numeric result codes (Hayes 1200 and higher):
     0 = OK
     1 = CONNECT at 300 bps (or 1200 bps on Hayes 1200 with basic code set)
     2 = RING
     3 = NO CARRIER
     4 = ERROR (in command line)
     5 = CONNECT 1200 (extended code set)
  Hayes 2400 and higher:
     6 = NO DIALTONE
     7 = BUSY
     8 = NO ANSWER
     9 = (there is no 9)
    10 = CONNECT 2400
  Reportedly, the codes for Hayes V.32 modems are:
    1x = CONNECT <suffix>
    5x = CONNECT 1200 <suffix>
    9x = CONNECT 2400 <suffix>
   11x = CONNECT 4800 <suffix>
   12x = CONNECT 9600 <suffix>
  Where:
    x:   suffix:
    R  = RELIABLE
    RC = RELIABLE COMPRESSED
    L  = LAPM
    LC = LAPM COMPRESSED
  And for Telebits, all the above, except no suffix in numeric mode, plus:
    11 = CONNECT 4800
    12 = CONNECT 9600
    13 = CONNECT 14400
    14 = CONNECT 19200
    15 = CONNECT 38400
    16 = CONNECT 57600
    20 = CONNECT 300/REL  (= MNP)
    22 = CONNECT 1200/REL (= MNP)
    23 = CONNECT 2400/REL (= MNP)
    46 = CONNECT 7512  (i.e. 75/1200)
    47 = CONNECT 1275  (i.e. 1200/75)
    48 = CONNECT 7200
    49 = CONNECT 12000
    50 = CONNECT FAST (not on T1600/3000)
    52 = RRING
    53 = DIALING
    54 = NO PROMPTTONE
    61 = CONNECT FAST/KERM (Kermit spoof)
    70 = CONNECT FAST/COMP (PEP + compression)
    71 = CONNECT FAST/KERM/COMP (PEP + compression + Kermit spoof)

  And for others, lots of special cases below...
*/
#define NBUFL 8
    char nbuf[NBUFL+1];			/* Response buffer */
    int i = 0, j = 0;			/* Buffer pointers */

    debug(F101,"RESPONSE mdmecho","",mdmecho);
    if (mdmecho) {			/* Sponge up dialing string echo. */
	while (1) {
	    c = (char) (ddinc(0) & 0x7f);
	    debug(F000,"SPONGE","",c);
	    dialoc(c);
	    if (c == CR) break;
	}
    }
    while (mdmstat == 0) {		/* Read response */
	for (i = 0; i < NBUFL; i++)	/* Clear the buffer */
	  nbuf[i] = '\0';
	i = 0;				/* Reset the buffer pointer. */
	c = (char) (ddinc(0) & 0177);	/* Get first digit of response. */
					/* using an untimed, blocking read. */
	debug(F000,"RESPONSE-A","",c);
	dialoc(c);			/* Echo it if requested. */
	if (!isdigit(c))		/* If not a digit, keep looking. */
	  continue;
	nbuf[i++] = c;			/* Got first digit, save it. */
	while (c != CR && i < 8) {	/* Read chars up to CR */
	    x = ddinc(0) & 0177;	/* Get a character. */
	    c = (char) x;		/* Got it OK. */
	    debug(F000,"RESPONSE-C","",c);
	    if (c != CR)		/* If it's not a carriage return, */
	      nbuf[i++] = c;		/*  save it. */
	    dialoc(c);			/* Echo it. */
	}
	nbuf[i] = '\0';			/* Done, terminate the buffer. */
	debug(F110,"dial hayesnv lbuf",lbuf,0);
	debug(F111,"dial hayesnv got",nbuf,i);
	/*
	   Separate any non-numeric suffix from the numeric
	   result code with a null.
	*/
	for (j = i-1; (j > -1) && !isdigit(nbuf[j]); j--)
	  nbuf[j+1] = nbuf[j];
	j++;
	nbuf[j++] = '\0';
	debug(F110,"dial hayesnv numeric",nbuf,0);
	debug(F111,"dial hayesnv suffix ",nbuf+j,j);
	/* Probably phone number echoing. */
	if ((int)strlen(nbuf) > 3)
	  continue;

	/* Now read and interpret the results... */

	i = atoi(nbuf);	/* Convert to integer */
	switch (i) {
	  case 0:
	    mdmstat = D_PARTIAL;	/* OK response */
	    break;
	  case 1:			/* CONNECT */
	    mdmstat = CONNECTED;	/* Could be any speed */
	    break;
	  case 2:			/* RING */
	    if (dialdpy)
	      printf("\r\n Local phone is ringing!\r\n");
	    mdmstat = D_FAILED;
	    dialsta = DIA_RING;
	    break;
	  case 3:			/* NO CARRIER */
	    if (dialdpy) printf("\r\n No Carrier.\r\n");
	    mdmstat = D_FAILED;
	    dialsta = DIA_NOCA;
	    break;
	  case 4:			/* ERROR */
	    if (dialdpy)
	      printf("\r\n Modem Command Error.\r\n");
	    mdmstat = D_FAILED;
	    dialsta = DIA_ERR;
	    break;
	  case 5:			/* CONNECT 1200 */
	    spdchg(1200L); /* Change speed if necessary. */
	    mdmstat = CONNECTED;
	    break;
	  case 6:			/* NO DIALTONE */
#ifndef MINIDIAL
	    if (mymdmtyp == n_MICROLINK && atoi(diallcc) == 49 && dialdpy)
	      printf("\r\n Dial Locked.\r\n"); /* Germany */
	    else
#endif /* MINIDIAL */
	      if (dialdpy)
		printf("\r\n No Dialtone.\r\n");
	    mdmstat = D_FAILED;
	    dialsta = DIA_NODT;
	    break;
	  case 7:			/* BUSY */
	    if (dialdpy) printf("\r\n Busy.\r\n");
	    mdmstat = D_FAILED;
	    dialsta = DIA_BUSY;
	    break;
	  case 8:			/* NO ANSWER */
#ifndef MINIDIAL
	    if (mymdmtyp == n_MICROLINK && atoi(diallcc) == 41 && dialdpy)
	      printf("\r\n Dial Locked.\r\n"); /* Switzerland */
	    else
#endif /* MINIDIAL */
	      if (dialdpy)
		printf("\r\n No Answer.\r\n");
	    mdmstat = D_FAILED;
	    dialsta = DIA_NOAN;
	    break;

	  case 9:
#ifndef MINIDIAL
	    if (mymdmtyp == n_XJACK || mymdmtyp == n_SUPRAX) {
		spdchg(600);
		break;
	    } /* fall thru */
#endif /* MINIDIAL */
	  case 10:			/* CONNECT 2400 */
	    spdchg(2400L);		/* Change speed if necessary. */
	    mdmstat = CONNECTED;
	    break;

#ifndef MINIDIAL

/* Starting here, we get different meanings from different manufacturers */

	  case 11:
	    if (mymdmtyp == n_USR) {
		if (dialdpy) printf(" Ringing...\r\n");
	    } else {
		spdchg(4800L);		/* CONNECT 4800 */
		mdmstat = CONNECTED;
	    }
	    break;
	  case 12:
	    if (mymdmtyp == n_USR) {
		if (dialdpy)
		  printf("\r\n Answered by voice.\r\n");
		mdmstat = D_FAILED;
		dialsta = DIA_VOIC;
	    } else if (mymdmtyp == n_KEEPINTOUCH) {
		spdchg(7200L);
		mdmstat = CONNECTED;
	    } else {
		spdchg(9600L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 13:
	    if (mymdmtyp == n_ATT1900 || mymdmtyp == n_ATT1910) {
		if (dialdpy) printf(" Wait...\r\n");
		break;
	    } else if (mymdmtyp == n_USR || mymdmtyp == n_USRX2)
	      spdchg(9600L);
	    else if (is_rockwell || is_supra ||
		mymdmtyp == n_ZOLTRIX || mymdmtyp == n_XJACK)
	      spdchg(7200L);
	    else if (mymdmtyp != n_MICROLINK)
	      spdchg(14400L);
	    mdmstat = CONNECTED;
	    break;
	  case 14:
	    if (is_rockwell || is_supra || mymdmtyp == n_XJACK)
	      spdchg(12000L);
	    else if (mymdmtyp == n_DATAPORT || mymdmtyp == n_MICROLINK)
	      spdchg(14400L);
	    else if (mymdmtyp == n_KEEPINTOUCH)
	      spdchg(9600L);
	    else if (mymdmtyp != n_USR && mymdmtyp != n_ZOLTRIX)
	      spdchg(19200L);
	    mdmstat = CONNECTED;
	    break;
	  case 15:
	    if (is_rockwell || is_supra ||
		mymdmtyp == n_ZOLTRIX || mymdmtyp == n_XJACK)
	      spdchg(14400L);
	    else if (mymdmtyp == n_USR)
	      spdchg(1200L);
	    else if (mymdmtyp == n_ZYXEL || mymdmtyp == n_INTEL)
	      spdchg(7200L);
	    else if (mymdmtyp == n_DATAPORT)
	      spdchg(19200L);
	    else
	      spdchg(38400L);
	    mdmstat = CONNECTED;
	    break;
	  case 16:
	    if (is_rockwell || is_supra ||
		mymdmtyp == n_ZOLTRIX || mymdmtyp == n_XJACK)
	      spdchg(19200L);
	    else if (mymdmtyp == n_USR)
	      spdchg(2400L);
	    else if (mymdmtyp == n_DATAPORT)
	      spdchg(7200L);
	    else if (mymdmtyp != n_ZYXEL && mymdmtyp != n_INTEL) /* 12000 */
	      spdchg(57600L);
	    mdmstat = CONNECTED;
	    break;
	  case 17:
	    if (mymdmtyp != n_DATAPORT || mymdmtyp == n_XJACK)	/* 16800 */
	      spdchg(38400L);
	    else if (mymdmtyp == n_ZYXEL || mymdmtyp == n_INTEL)
	      spdchg(14400L);
	    else if (mymdmtyp == n_KEEPINTOUCH)
	      spdchg(14400L);
	    else if (mymdmtyp == n_USR)
	      spdchg(9600L);
	    mdmstat = CONNECTED;
	    break;
	  case 18:
	    if (is_rockwell || is_supra ||
		mymdmtyp == n_ZOLTRIX || mymdmtyp == n_XJACK ||
		mymdmtyp == n_MHZATT || mymdmtyp == n_LUCENT)
	      spdchg(57600L);
	    else if (mymdmtyp == n_INTEL)
	      spdchg(19200L);
	    else if (mymdmtyp == n_USR || mymdmtyp == n_USRX2)
	      spdchg(4800L);
	    mdmstat = CONNECTED;
	    break;
	  case 19:
	    if (mymdmtyp == n_DATAPORT)
	      spdchg(300L);
	    else if (mymdmtyp == n_ZYXEL || mymdmtyp == n_INTEL)
	      spdchg(38400L);
	    else
	      spdchg(115200L);
	    mdmstat = CONNECTED;
	    break;
	  case 20:
	    if (mymdmtyp == n_USR || mymdmtyp == n_USRX2)
	      spdchg(7200L);
	    else if (mymdmtyp == n_DATAPORT)
	      spdchg(2400L);
	    else if (mymdmtyp == n_ZYXEL || mymdmtyp == n_INTEL)
	      spdchg(57600L);
	    else
	      spdchg(300L);
	    mdmstat = CONNECTED;
	    break;
	  case 21:
	    if (mymdmtyp == n_DATAPORT)
	      spdchg(4800L);
	    mdmstat = CONNECTED;
	    break;
	  case 22:
	    if (is_rockwell || is_supra || mymdmtyp == n_XJACK)
	      spdchg(8880L);
	    else if (mymdmtyp == n_DATAPORT)
	      spdchg(9600L);
	    else if (mymdmtyp == n_KEEPINTOUCH)
	      spdchg(300L);
	    else if (!is_hayeshispd)
	      spdchg(1200L);
	    mdmstat = CONNECTED;
	    break;
	  case 23:
	    if (is_hayeshispd || is_supra ||
		mymdmtyp == n_MULTI || mymdmtyp == n_XJACK)
	      spdchg(8880L);
	    else if (mymdmtyp != n_DATAPORT && !is_rockwell) /* 12000 */
	      spdchg(2400L);
	    mdmstat = CONNECTED;
	    break;
	  case 24:
	    if (is_rockwell ||  is_supra || mymdmtyp == n_XJACK) {
		mdmstat = D_FAILED;
		dialsta = DIA_DELA;	/* Delayed */
		break;
	    } else if (is_hayeshispd || mymdmtyp == n_LUCENT)
	      spdchg(7200L);
	    else if (mymdmtyp == n_DATAPORT)
	      spdchg(14400L);
	    else if (mymdmtyp == n_INTEL || mymdmtyp == n_KEEPINTOUCH)
	      spdchg(1200L);
	    mdmstat = CONNECTED;
	    break;
	  case 25:
	    if (mymdmtyp == n_USR || mymdmtyp == n_USRX2)
	      spdchg(14400L);
	    else if (mymdmtyp == n_LUCENT)
	      spdchg(12000L);
	    else if (is_motorola)
	      spdchg(9600L);
	    else if (mymdmtyp == n_INTEL || mymdmtyp == n_KEEPINTOUCH)
	      spdchg(2400L);
	    mdmstat = CONNECTED;
	    break;
	  case 26:
	    if (mymdmtyp == n_DATAPORT)
	      spdchg(19200L);
	    else if (mymdmtyp == n_INTEL || mymdmtyp == n_KEEPINTOUCH)
	      spdchg(4800L);
	    mdmstat = CONNECTED;
	    break;
	  case 27:
	    if (mymdmtyp == n_DATAPORT)
	      spdchg(38400L);
	    else if (mymdmtyp == n_INTEL || mymdmtyp == n_KEEPINTOUCH)
	      spdchg(7200L);
	    else if (mymdmtyp == n_MHZATT)
	      spdchg(8880L);
	    mdmstat = CONNECTED;
	    break;
	  case 28:
	    if (mymdmtyp == n_DATAPORT)
	      spdchg(7200L);
	    else if (mymdmtyp == n_INTEL || mymdmtyp == n_KEEPINTOUCH)
	      spdchg(9600L);
	    else if (mymdmtyp == n_MHZATT || mymdmtyp == n_LUCENT)
	      spdchg(38400L);
	    mdmstat = CONNECTED;
	    break;
	  case 29:
	    if (is_motorola)
	      spdchg(4800L);
	    else if (mymdmtyp == n_DATAPORT)
	      spdchg(19200L);
	    mdmstat = CONNECTED;
	    break;
	  case 30:
	    if (mymdmtyp == n_INTEL || mymdmtyp == n_KEEPINTOUCH) {
		spdchg(14400L);
		mdmstat = CONNECTED;
	    } /* fall thru on purpose... */
	  case 31:
	    if (mymdmtyp == n_UCOM_AT || mymdmtyp == n_MICROLINK) {
		spdchg(4800L);
		mdmstat = CONNECTED;
	    } else if (is_motorola) {
		spdchg(57600L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 32:
	    if (is_rockwell || is_supra || mymdmtyp == n_XJACK) {
		mdmstat = D_FAILED;
		dialsta = DIA_BLCK;	/* Blacklisted */
	    } else if (mymdmtyp == n_UCOM_AT || mymdmtyp == n_MICROLINK) {
		spdchg(9600L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_KEEPINTOUCH) {
		spdchg(300L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_INTEL) {
		spdchg(2400L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 33:			/* FAX connection */
	    if (is_rockwell || is_supra ||
		mymdmtyp == n_ZOLTRIX || mymdmtyp == n_XJACK) {
		mdmstat = D_FAILED;
		dialsta = DIA_FAX;
	    } else if (mymdmtyp == n_UCOM_AT ||
		       is_motorola ||
		       mymdmtyp == n_MICROLINK
		       ) {
		spdchg(9600L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_MHZATT) {
		spdchg(115200L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 34:
	    if (mymdmtyp == n_INTEL || mymdmtyp == n_KEEPINTOUCH) {
		spdchg(1200L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_MICROLINK) {
		spdchg(7200L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 35:
	    if (is_rockwell) {
		spdchg(300L);
		dialsta = CONNECTED;
	    } else if (is_motorola) {
		spdchg(14400L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_INTEL || mymdmtyp == n_KEEPINTOUCH) {
		spdchg(2400L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_MICROLINK) {
		spdchg(7200L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_ZOLTRIX || mymdmtyp == n_XJACK) /* DATA */
	      mdmstat = CONNECTED;
	    break;
	  case 36:
	    if (mymdmtyp == n_UCOM_AT) {
		spdchg(19200L);
		mdmstat = CONNECTED;
	    } else if (is_motorola) {
		spdchg(1200L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_INTEL || mymdmtyp == n_KEEPINTOUCH) {
		spdchg(4800L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 37:
	    if (mymdmtyp == n_UCOM_AT) {
		spdchg(19200L);
		mdmstat = CONNECTED;
	    } else if (is_motorola) {
		spdchg(2400L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_INTEL || mymdmtyp == n_KEEPINTOUCH) {
		spdchg(7200L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 38:
	    if (is_motorola) {
		spdchg(4800L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_INTEL || mymdmtyp == n_KEEPINTOUCH) {
		spdchg(9600L);
		mdmstat = CONNECTED;
	    } /* fall thru on purpose... */
	  case 39:
	    if (mymdmtyp == n_UCOM_AT) {
		spdchg(38400L);
		mdmstat = CONNECTED;
	    } else if (is_motorola) {
		spdchg(9600L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_MICROLINK) {
		spdchg(14400L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 40:
	    if (mymdmtyp == n_UCOM_AT) {
		mdmstat = D_FAILED;
		dialsta = DIA_NOCA;
	    } else if (is_motorola || mymdmtyp == n_INTEL ||
		       mymdmtyp == n_KEEPINTOUCH) {
		spdchg(14400L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 41:
	    if (is_motorola) {
		spdchg(19200L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 42:
	    if (mymdmtyp == n_KEEPINTOUCH) {
		spdchg(300L);
		mdmstat = CONNECTED;
	    } else if (is_motorola) {
		spdchg(38400L);
		mdmstat = CONNECTED;
	    } /* fall thru on purpose... */
	  case 43:
	    if (mymdmtyp == n_UCOM_AT) {
		spdchg(57600L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_USRX2)
	      mdmstat = CONNECTED;	/* 168000 */
	    break;
	  case 44:
	    if (is_rockwell) {
		spdchg(8800L);
		dialsta = CONNECTED;
	    } else if (is_motorola) {
		spdchg(7200L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_INTEL || mymdmtyp == n_KEEPINTOUCH) {
		spdchg(1200L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 45:
	    if (is_motorola) {
		spdchg(57600L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_INTEL || mymdmtyp == n_KEEPINTOUCH) {
		spdchg(2400L);
		mdmstat = CONNECTED;
	    } else if (n_USR) {
		spdchg(14400L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 46:
	    if (is_rockwell)
	      spdchg(1200L);
	    else if (mymdmtyp == n_INTEL || mymdmtyp == n_KEEPINTOUCH)
	      spdchg(4800L);
	    else
	      spdchg(8880L);		/* 75/1200 split speed */
	    mdmstat = CONNECTED;
	    break;
	  case 47:
	    if (is_rockwell)
	      spdchg(2400L);
	    else if (mymdmtyp == n_INTEL || mymdmtyp == n_KEEPINTOUCH)
	      spdchg(7200L);
	    else
	      printf("CONNECT 1200/75 - Not supported by C-Kermit\r\n");
	    mdmstat = CONNECTED;
	    break;
	  case 48:
	    if (is_rockwell)
	      spdchg(4800L);
	    else if (mymdmtyp == n_INTEL || mymdmtyp == n_KEEPINTOUCH)
	      spdchg(9600L);
	    else
	      spdchg(7200L);
	    mdmstat = CONNECTED;
	    break;
	  case 49:
	    if (is_rockwell)
	      spdchg(7200L);
	    mdmstat = CONNECTED;
	    break;
	  case 50:			/* CONNECT FAST */
	    if (is_rockwell)
	      spdchg(9600L);
	    else if (mymdmtyp == n_INTEL || mymdmtyp == n_KEEPINTOUCH)
	      spdchg(14400L);
	    mdmstat = CONNECTED;
	    break;
	  case 51:
	    if (mymdmtyp == n_UCOM_AT) {
		mdmstat = D_FAILED;
		dialsta = DIA_NODT;
	    }
	    break;
	  case 52:			/* RRING */
	    if (mymdmtyp == n_TELEBIT)
	      if (dialdpy) printf(" Ringing...\r\n");
	    break;
	  case 53:			/* DIALING */
	    if (mymdmtyp == n_TELEBIT)
	      if (dialdpy) printf(" Dialing...\r\n");
	    break;
	  case 54:
	    if (is_rockwell) {
		spdchg(19200L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_INTEL || mymdmtyp == n_KEEPINTOUCH) {
		spdchg(1200L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_TELEBIT) {
		if (dialdpy) printf("\r\n No Prompttone.\r\n");
		mdmstat = D_FAILED;
		dialsta = DIA_NODT;
	    }
	    break;
	  case 55:
	    if (mymdmtyp == n_INTEL || mymdmtyp == n_KEEPINTOUCH) {
		spdchg(2400L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 56:
	    if (mymdmtyp == n_INTEL || mymdmtyp == n_KEEPINTOUCH) {
		spdchg(4800L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 57:
	    if (mymdmtyp == n_INTEL || mymdmtyp == n_KEEPINTOUCH) {
		spdchg(7200L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 58:
	    if (mymdmtyp == n_INTEL || mymdmtyp == n_KEEPINTOUCH) {
		spdchg(9600L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 59:
	    if (mymdmtyp == n_INTEL)	/* 12000 */
	      mdmstat = CONNECTED;
	    break;
	  case 60:
	    if (mymdmtyp == n_INTEL || mymdmtyp == n_KEEPINTOUCH) {
		spdchg(14400L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 64:
	    if (mymdmtyp == n_INTEL) {
		spdchg(1200L);
		mdmstat = CONNECTED;
	    } else if (is_supra) {
		spdchg(28800L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 65:
	    if (mymdmtyp == n_INTEL || mymdmtyp == n_KEEPINTOUCH) {
		spdchg(2400L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 66:
	    if (mymdmtyp == n_INTEL || mymdmtyp == n_KEEPINTOUCH) {
		spdchg(4800L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 67:
	    if (mymdmtyp == n_INTEL || mymdmtyp == n_KEEPINTOUCH) {
		spdchg(7200L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 68:
	    if (mymdmtyp == n_INTEL || mymdmtyp == n_KEEPINTOUCH) {
		spdchg(9600L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 69:
	    if (mymdmtyp == n_INTEL || mymdmtyp == n_KEEPINTOUCH) /* 12000 */
	      mdmstat = CONNECTED;
	    break;
	  case 70:
	    if (mymdmtyp == n_INTEL || mymdmtyp == n_KEEPINTOUCH) {
		spdchg(14400L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 73:
	    if (mymdmtyp == n_UCOM_AT) {
		spdchg(115200L);
		mdmstat = CONNECTED;
		break;
	    } /* else fall thru */
	    if (mymdmtyp == n_TELEBIT)	/* Early models only */
	      mdmstat = CONNECTED;
	    break;
	  case 85:
	    if (mymdmtyp == n_USR || mymdmtyp == n_USRX2)
	      spdchg(19200L);
	    mdmstat = CONNECTED;
	    break;
	  case 91:			/* 21600 */
	  case 99:			/* 24000 */
	  case 103:			/* 26400 */
	    if (mymdmtyp == n_USRX2)
	      mdmstat = CONNECTED;
	    break;
	  case 107:
	    if (mymdmtyp == n_USR || mymdmtyp == n_USRX2) {
		spdchg(28800L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 151:			/* 312000 */
	  case 155:			/* 336000 */
	    if (mymdmtyp == n_USRX2)
	      mdmstat = CONNECTED;
	    break;

#endif /* MINIDIAL */
	  default:
#ifndef MINIDIAL
	    if (mymdmtyp == n_USR || mymdmtyp == n_USRX2 ||
		is_hayeshispd || is_rockwell)
#endif /* MINIDIAL */
	      if (i > 12)		/* There are hundreds of them... */
		mdmstat = CONNECTED;
	    break;
	}
    }
    if (mdmstat == CONNECTED && nbuf[j] != '\0') {
	if (dialdpy) {
	    printf("\r\n");
	    if (nbuf[j] == 'R') printf(" RELIABLE");
	    if (nbuf[j] == 'L') printf(" LAPM");
	    if (nbuf[j+1] == 'C') printf(" COMPRESSED");
	    printf("\r\n");
	}
	ckstrncpy(lbuf,nbuf,LBUFL);		/* (for messages...) */
    }
}

static VOID				/* Get Hayes Result in Word mode */
gethrw() {
    char *cptr, *s;
    long conspd;

    if (mdmspd && !network) {
	s = lbuf;
	while (*s != '\0' && *s != 'C') s++;
	cptr = (*s == 'C') ? s : NULL;
	conspd = 0L;
	if ((cptr != NULL) && !strncmp(cptr,"CONNECT ",8)) {
	    if ((int)strlen(cptr) < 9)   /* Just CONNECT, */
	      conspd = 300L;		 /* use 300 bps */
	    else if (isdigit(*(cptr+8))) /* not CONNECT FAST */
	      conspd = atol(cptr + 8);   /* CONNECT nnnn */
	    if (conspd != speed) {
		if ((conspd / 10L) > 0) {
		    if (ttsspd((int) (conspd / 10L)) < 0) {
			printf(" Can't change speed to %ld\r\n",
			       conspd);
		    } else {
			speed = conspd;
			mdmstat = CONNECTED;
			if ( !quiet && !backgrd )
			  printf(" Speed changed to %ld\r\n",
				 conspd);
		    }
		}
	    } /* Expanded to handle any conceivable speed */
	}
    }
#ifndef MINIDIAL
    if (mymdmtyp == n_TELEBIT) {
	if (didweget(lbuf,"CONNECT FAST/KERM")) {
	    mdmstat = CONNECTED;
	    if (dialdpy) printf("FAST/KERM ");
	    return;
	}
    }
#endif /* MINIDIAL */
    if (didweget(lbuf,"RRING") ||
	didweget(lbuf,"RINGING") ||
	didweget(lbuf,"DIALING")) {
	mdmstat = 0;
    } else if (didweget(lbuf,"CONNECT")) {
	mdmstat = CONNECTED;
    } else if (didweget(lbuf,"OK")) {
	if (partial) {
	    mdmstat = D_PARTIAL;
	} else {
	    mdmstat = D_FAILED;
	    dialsta = DIA_ERR;
	}
    } else if (didweget(lbuf,"NO CARRIER")) {
	mdmstat = D_FAILED;
	dialsta = DIA_NOCA;
    } else if (didweget(lbuf,"NO DIALTONE")) {
	mdmstat = D_FAILED;
	dialsta = DIA_NODT;
    } else if (didweget(lbuf,"NO DIAL TONE")) {
	mdmstat = D_FAILED;
	dialsta = DIA_NODT;
    } else if (didweget(lbuf,"BUSY")) {
	mdmstat = D_FAILED;
	dialsta = DIA_BUSY;
    } else if (didweget(lbuf,"NO ANSWER")) {
	mdmstat = D_FAILED;
	dialsta = DIA_NOAN;
    } else if (didweget(lbuf,"VOICE")) {
	mdmstat = D_FAILED;
	dialsta = DIA_VOIC;
    } else if (didweget(lbuf,"VCON")) {
	mdmstat = D_FAILED;
	dialsta = DIA_VOIC;
    } else if (didweget(lbuf,"NO PROMPT TONE")) {
	mdmstat = D_FAILED;
	dialsta = DIA_NODT;
    } else if (didweget(lbuf,"REMOTE ACCESS FAILED")) {
	mdmstat = D_FAILED;
	dialsta = DIA_NOCA;
    } else if (didweget(lbuf,"FAX")) {
	mdmstat = D_FAILED;
	dialsta = DIA_FAX;
    } else if (didweget(lbuf,"WAIT - CONNECTING") ||
	       didweget(lbuf,"WAIT-CONNECTING")) { /* AT&T STU-III 19xx */
	mdmstat = 0;
    } else if (didweget(lbuf,"DELAYED")) {
	mdmstat = D_FAILED;
	dialsta = DIA_DELA;
    } else if (didweget(lbuf,"BLACKLISTED")) {
	mdmstat = D_FAILED;
	dialsta = DIA_BLCK;
    } else if (didweget(lbuf,"COMPRESSION")) {
	mdmstat = 0;
    } else if (didweget(lbuf,"PROTOCOL")) {
	mdmstat = 0;
    } else if (didweget(lbuf,"DIAL LOCKED")) { /* Germany, Austria, Schweiz */
	mdmstat = D_FAILED;
	dialsta = DIA_BLCK;
    } else if ( didweget(lbuf,"RING") ||
	        didweget(lbuf,"RING1") || /* Distinctive Ring 1 */
		didweget(lbuf,"RING2") || /* Distinctive Ring 2 */
		didweget(lbuf,"RING3") ) {
	mdmstat = (func_code == 0) ? D_FAILED : 0;
	dialsta = DIA_RING;
    } else if (didweget(lbuf,"ERROR")) {
	mdmstat = D_FAILED;
	dialsta = DIA_ERR;
    } else if (didweget(lbuf,"CARRIER")) { /* Boca / Rockwell family */
#ifdef COMMENT
	if (is_rockwell)
#endif /* COMMENT */
	  mdmstat = 0;
#ifdef COMMENT
	/* Does CARRIER ever mean the same as CONNECT? */
	else
	  mdmstat = CONNECTED;
#endif /* COMMENT */
    } else if (didweget(lbuf,"DATA")) {	/* Boca / Rockwell family */
	/* This message is sent when the modem is in FAX mode  */
	/* So setting this to CONNECTED may not be appropriate */
	/* We must send ATO\015 to the modem in response       */
	/* Then we will get a CONNECTED message                */
	mdmstat = CONNECTED;
    } else if (didweget(lbuf,"DIGITAL LINE")) {
	mdmstat = D_FAILED;
	dialsta = DIA_DIGI;
    } else if (didweget(lbuf,"DATE")) { /* Caller ID Date */
	debug(F110,"CALLID DATE",lbuf,0);
	/* Format is "DATE     =   MMDD"   */
	makestr(&callid_date,lbuf);
    } else if (didweget(lbuf,"TIME")) { /* Caller ID Time */
	/* Format is "TIME     =   HHMM"   */
	debug(F110,"CALLID TIME",lbuf,0);
	makestr(&callid_time,lbuf);
    } else if (didweget(lbuf,"NAME")) { /* Caller ID Name */
	/* Format is "NAME     =   <listing name>"   */
	debug(F110,"CALLID NAME",lbuf,0);
	makestr(&callid_name,lbuf);
    } else if (didweget(lbuf,"NMBR")) { /* Caller ID Number */
	/* Format is "NMBR     =   <number>, 'P' or 'O'"   */
	/* 	'P' means Privacy Requested 		   */
	/*      'O' means Out of Service or Not available  */
	debug(F110,"CALLID NMBR",lbuf,0);
	makestr(&callid_nmbr,lbuf);
    } else if (didweget(lbuf,"MESG")) { /* Caller ID Unrecognized Message */
	/* Format is "MESG     =   <tag><length><data><checksum>"   */
	debug(F110,"CALLID MESG",lbuf,0);
	makestr(&callid_mesg,lbuf);
    }
}

/* Maybe hang up the phone, depending on various SET DIAL parameters. */

int
dialhup() {
    int x = 0;
    if (dialhng && dialsta != DIA_PART) { /* DIAL HANGUP ON? */
	x = mdmhup();			/* Try modem-specific method first */
	debug(F101,"dialhup mdmhup","",x);
	if (x > 0) {			/* If it worked, */
	    dialsta = DIA_HUP;
	    if (dialdpy)
	      printf(" Modem hangup OK\r\n"); /* fine. */
	} else if (network		/* If we're telnetted to */
#ifdef TN_COMPORT
                   && !istncomport()    /* (without RFC 2217)    */
#endif /* TN_COMPORT */
                   ) {		
	    dialsta = DIA_HANG;
	    if (dialdpy)		/* a modem server, just print a msg */
	      printf(" WARNING - modem hangup failed\r\n"); /* don't hangup! */
	    return(0);
	} else {			/* Otherwise */
	    x = tthang();		/* Tell the OS to turn off DTR. */
	    if (x > 0) {		/* Yes, tell results from tthang() */
		dialsta = DIA_HUP;
		if (dialdpy) printf(" Hangup OK\r\n");
	    } else if (x == 0) {
		if (dialdpy) printf(" Hangup skipped\r\n");
	    } else {
		dialsta = DIA_HANG;
		if (dialdpy) perror(" Hangup error");
	    }
	    ttflui();
	}
    } else if (dialdpy) printf(" Hangup skipped\r\n"); /* DIAL HANGUP OFF */
    return(x);
}

/*
  M D M H U P  --

  Sends escape sequence to modem, then sends its hangup command.  Returns:
   0: If modem type is 0 (direct serial connection),
      or if modem type is < 0 (network connection),
      or if no action taken because DIAL MODEM-HANGUP is OFF)
        or because no hangup string for current modem type,
      or C-Kermit is in remote mode,
      or if action taken but there was no positive response from modem;
   1: Success: modem is in command state and acknowledged the hangup command;
  -1: On modem command error.
*/
int
mdmhup() {
#ifdef MDMHUP
    int m, x = 0;
    int xparity;
    int savcarr;
    extern int ttcarr;
    char *s, *p, c;
    MDMINF * mp = NULL;

    debug(F101,"mdmhup dialmhu","",dialmhu); /* MODEM-HANGUP METHOD */
    debug(F101,"mdmhup local","",local);

    if (dialmhu == 0 || local == 0)	/* If DIAL MODEM-HANGUP is OFF, */
      return(0);			/*  or not in local mode, fail. */

    debug(F101,"mdmhup dialsta","",dialsta);
    debug(F101,"mdmhup mdmset","",mdmset);

    if (dialsta != DIA_OK && !mdmset)	/* It's not a dialed connection */
      return(0);

#ifdef CK_TAPI
    if (tttapi && !tapipass)		/* Don't hangup if using TAPI */
      return(0);
#endif /* CK_TAPI */

#ifdef COMMENT
    /* No, we still need this for modems that ignore DTR */
    if (mymdmtyp == n_GENERIC && !network)
      return(0);
#endif /* COMMENT */

    debug(F101,"mdmhup dialesc","",dialesc);
    if (dialesc < 0)
      return(0);			/* No modem escape-character, fail. */

    savcarr = ttcarr;
    ttcarr = CAR_OFF;
    x = ttchk();
    ttcarr = savcarr;
    debug(F101,"mdmhup ttchk","",x);
    if (x < 0)				/* There appears to be no connection */
      return(0);
    x = 0;

#ifdef OS2
/*
  In OS/2, if CARRIER is OFF, and there is indeed no carrier signal, any
  attempt to do i/o at this point can hang the program.  This might be true
  for other operating systems too.
*/
    if (!network			/* Not a network connection */
#ifdef TN_COMPORT
        || istncomport()
#endif /* TN_COMPORT */
	) {
	m = ttgmdm();			/* Get modem signals */
	if ((m > -1) && (m & BM_DCD == 0)) /* Check for carrier */
	  return(0);			/* No carrier, skip the rest */
    }
#endif /* OS2 */

    debug(F111,"mdmhup network",ttname,network);
    debug(F101,"mdmhup mymdmtyp","",mymdmtyp);
    debug(F101,"mdmhup mdmtyp","",mdmtyp);
    /* In case of HANGUP before DIAL */
    if (network && mdmtyp < 1)		/* SET HOST but no subsequent */
      return(0);			/* SET MODEM TYPE... */
    if (mymdmtyp == 0 && mdmtyp > 0)
      mymdmtyp = mdmtyp;
    if (mymdmtyp < 1)			/* Not using a modem */
      return(0);
    if (mymdmtyp > 0)			/* An actual modem... */
      mp = modemp[mymdmtyp];
    if (!mp) {				/* Get pointer to its MDMINF struct */
	debug(F100,"mdmhup no MDMINF","",0);
	return(0);
    }
    mdmcapas = dialcapas ? dialcapas : mp->capas;
    xx_ok = mp->ok_fn;			/* Pointer to response reader */

    s = dialhcmd ? dialhcmd : mp->hup_str; /* Get hangup command */
    if (!s) s = "";
    debug(F110,"mdmhup hup_str",s,0);
    if (!*s) return(0);			/* If none, fail. */

    if (ttpkt(speed,FLO_DIAL,parity) < 0) /* Condition line for dialing */
      return(-1);

    xparity = parity;			/* Set PARITY to NONE temporarily */
    parity = 0;

    /* In case they gave a SET MODEM ESCAPE command recently... */

    if (dialesc < 0 || dialesc > 127)
      c = NUL;
    else
      c = (char) (dialesc ? dialesc : mp->esc_char);

    if (mdmcapas & CKD_AT) {		/* Hayes compatible */
	escbuf[0] = c;
	escbuf[1] = c;
	escbuf[2] = c;
	escbuf[3] = NUL;
    } else {				/* Other */
	escbuf[0] = c;
	escbuf[1] = NUL;
    }
    debug(F110,"mdmhup escbuf",escbuf,0);
    if (escbuf[0]) {			/* Have escape sequence? */
	debug(F101,"mdmhup esc_time",0,mp->esc_time);
	if (mp->esc_time)		/* If we have a guard time */
	  msleep(mp->esc_time);		/* Pause for guard time */
	debug(F100,"mdmhup pause 1 OK","",0);

#ifdef NETCONN				/* Send modem's escape sequence */
	if (network) {			/* Must catch errors here. */
	    if (ttol((CHAR *)escbuf,(int)strlen((char *)escbuf)) < 0) {
		parity = xparity;
		return(-1);
	    }
	    debug(F110,"mdmhup ttslow net ok",escbuf,0);
	} else {
#endif /* NETCONN */
	    ttslow((char *)escbuf,wr); /* Send escape sequence */
	    debug(F110,"mdmhup ttslow ok",escbuf,0);
#ifdef NETCONN
	}
#endif /* NETCONN */

	if (mp->esc_time)		/* Pause for guard time again */
	  msleep(mp->esc_time);
	else
	  msleep(500);			/* Wait half a sec for echoes. */
	debug(F100,"mdmhup pause 1 OK","",0);
#ifdef COMMENT
	ttflui();			/* Flush response or echo, if any */
	debug(F100,"mdmhup ttflui OK","",0);
#endif /* COMMENT */
    }
    ttslow(s,wr);			/* Now Send hangup string */
    debug(F110,"mdmhup ttslow ok",s,0);
/*
  This is not exactly right, but it works.
  If we are online:
    the modem says OK when it gets the escape sequence,
    and it says NO CARRIER when it gets the hangup command.
  If we are offline:
    the modem does NOT say OK (or anything else) when it gets the esc sequence,
    but it DOES say OK (and not NO CARRIER) when it gets the hangup command.
  So the following function should read the OK in both cases.
  Of course, this is somewhat Hayes-specific...
*/
    if (xx_ok) {			/* Look for OK response */
	debug(F100,"mdmhup calling response function","",0);
	x = (*xx_ok)(3,1);		/* Give it 3 seconds, be strict. */
	debug(F101,"mdmhup hangup response","",x);
	msleep(500);			/* Wait half a sec */
	ttflui();			/* Get rid of NO CARRIER, if any */
    } else {				/* No OK function, */
	x = 1;				/* so assume it worked */
	debug(F101,"mdmhup no ok_fn","",x);
    }
    parity = xparity;			/* Restore prevailing parity */
    return(x);				/* Return OK function's return code. */

#else  /* MDMHUP not defined. */

    debug(F100,"mdmhup MDMHUP not defined","",0);
    return(0);				/* Always fail. */

#endif /* MDMHUP */
}

#endif /* NOICP */
#else /* NODIAL */

int mdmtyp = 0;				/* Default modem type */

int					/* To allow NODIAL versions to */
mdmhup() {				/* call mdmhup(), so calls to  */
    return(0);				/* mdmhup() need not be within */
}					/* #ifndef NODIAL conditionals */
#endif /* NODIAL */
#else
int mdmtyp = 0;				/* Default modem type */
#endif /* NOLOCAL */

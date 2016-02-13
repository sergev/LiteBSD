char *protv =                                                     /* -*-C-*- */
"C-Kermit Protocol Module 9.0.160, 16 Oct 2009";

int kactive = 0;			/* Kermit protocol is active */

#define PKTZEROHACK

/* C K C P R O  -- C-Kermit Protocol Module, in Wart preprocessor notation. */
/*
  Author: Frank da Cruz <fdc@columbia.edu>,
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 2009,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/
#ifndef NOXFER
#include "ckcsym.h"
#include "ckcdeb.h"
#include "ckcasc.h"
#include "ckcker.h"
#ifdef OS2
#ifndef NT
#define INCL_NOPM
#define INCL_VIO			/* Needed for ckocon.h */
#include <os2.h>
#undef COMMENT
#endif /* NT */
#include "ckocon.h"
#endif /* OS2 */

/*
 Note -- This file may also be preprocessed by the UNIX Lex program, but
 you must indent the above #include statements before using Lex, and then
 restore them to the left margin in the resulting C program before compilation.
 Also, the invocation of the "wart()" function below must be replaced by an
 invocation  of the "yylex()" function.  It might also be necessary to remove
 comments in the (%)(%)...(%)(%) section.
*/

/* State definitions for Wart (or Lex) */
%states ipkt rfile rattr rdpkt ssinit ssfile ssattr ssdata sseof sseot
%states serve generic get rgen ssopkt ropkt

_PROTOTYP(static VOID xxproto,(void));
_PROTOTYP(static VOID wheremsg,(void));
_PROTOTYP(int wart,(void));
_PROTOTYP(static int sgetinit,(int,int));
_PROTOTYP(int sndspace,(int));

/* External C-Kermit variable declarations */
  extern char *versio, *srvtxt, *cmarg, *cmarg2, **cmlist, *rf_err;
  extern char * rfspec, * sfspec, * srfspec, * rrfspec;
  extern char * prfspec, * psfspec, * psrfspec, * prrfspec;
  extern char *cdmsgfile[];
  extern char * snd_move, * snd_rename, * srimsg;
  extern char filnam[], ofilnam[], fspec[], ttname[], ofn1[];
  extern CHAR sstate, *srvptr, *data;
  extern int timint, rtimo, nfils, hcflg, xflg, flow, mdmtyp, network;
  extern int oopts, omode, oname, opath, nopush, isguest, xcmdsrc, rcdactive;
  extern int rejection, moving, fncact, bye_active, urserver, fatalio;
  extern int protocol, prefixing, filcnt, carrier, fnspath, interrupted;
  extern int recursive, inserver, nzxopts, idletmo, srvidl, xfrint;
  extern struct ck_p ptab[];
  extern int remfile, rempipe, xferstat, filestatus, wearealike, fackpath;
  extern int patterns, filepeek, gnferror;
  extern char * remdest;

#ifdef PKTZEROHACK
#define PKTZEROLEN 32
static char ipktack[PKTZEROLEN];
static int ipktlen = 0;
#endif /* PKTZEROHACK */

static int s_timint = -1;		/* For saving timeout value */
static int myjob = 0;
static int havefs = 0;
#ifdef CK_LOGIN
static int logtries = 0;
#endif /* CK_LOGIN */

static int cancel = 0;
int fackbug = 0;

#ifdef STREAMING
extern int streaming, streamok;

static VOID
streamon() {
    if (streamok) {
	debug(F100,"streamon","",0);
	streaming = 1;
	timint = 0;			/* No timeouts while streaming. */
    }
}

#ifdef COMMENT				/* (not used) */
static VOID
streamoff() {
    if (streaming) {
	debug(F100,"streamoff","",0);
	streaming = 0;
	timint = s_timint;		/* Restore timeout */
    }
}
#endif /* COMMENT */
#else /* STREAMING */
#define streamon()
#define streamoff()
#endif /* STREAMING */

#ifndef NOSPL
_PROTOTYP( int addmac, (char *, char *) );
_PROTOTYP( int zzstring, (char *, char **, int *) );
#endif /* NOSPL */
#ifndef NOICP
_PROTOTYP( int cmdsrc, (void) );
#endif /* NOICP */

#ifndef NOSERVER
  extern char * x_user, * x_passwd, * x_acct;
  extern int x_login, x_logged;
#endif /* NOSERVER */

#include "ckcnet.h"

#ifdef TNCODE
  extern int ttnproto;			/* Network protocol */
#endif /* TNCODE */

#ifdef CK_SPEED
  extern short ctlp[];			/* Control-character prefix table */
#endif /* CK_SPEED */

#ifdef TNCODE
  extern int tn_b_nlm, tn_b_xfer, tn_nlm;
#ifdef CK_ENCRYPTION
  extern int tn_no_encrypt_xfer;
#endif /* CK_ENCRYPTION */
#endif /* TNCODE */

#ifdef TCPSOCKET
#ifndef NOLISTEN
  extern int tcpsrfd;
#endif /* NOLISTEN */
#endif /* TCPSOCKET */

  extern int cxseen, czseen, server, srvdis, local, displa, bctu, bctr, bctl;
  extern int bctf;
  extern int quiet, tsecs, parity, backgrd, nakstate, atcapu, wslotn, winlo;
  extern int wslots, success, xitsta, rprintf, discard, cdtimo, keep, fdispla;
  extern int timef, stdinf, rscapu, sendmode, epktflg, epktrcvd, epktsent;
  extern int binary, fncnv;
  extern long speed, ffc, crc16, calibrate, dest;
#ifdef COMMENT
  extern char *TYPCMD, *DIRCMD, *DIRCM2;
#endif /* COMMENT */
#ifndef OS2
  extern char *SPACMD, *SPACM2, *WHOCMD;
#endif /* OS2 */
  extern CHAR *rdatap;
  extern struct zattr iattr;

#ifdef VMS
  extern int batch;
#endif /* VMS */

#ifdef GFTIMER
  extern CKFLOAT fptsecs;
#endif /* GFTIMER */

  extern CHAR *srvcmd;
  extern CHAR *epktmsg;

#ifdef CK_TMPDIR
extern int f_tmpdir;			/* Directory changed temporarily */
extern char savdir[];			/* For saving current directory */
extern char * dldir;
#endif /* CK_TMPDIR */

  extern int query;			/* Query-active flag */
#ifndef NOSPL
  extern int cmdlvl;
  char querybuf[QBUFL+1] = { NUL, NUL }; /* QUERY response buffer */
  char *qbufp = querybuf;		/* Pointer to it */
  int qbufn = 0;			/* Length of data in it */
#else
  extern int tlevel;
#endif /* NOSPL */

#ifndef NOICP
  extern int escape;
#endif /* NOICP */
/*
  If the following flag is nonzero when the protocol module is entered,
  then server mode persists for exactly one transaction, rather than
  looping until BYE or FINISH is received.
*/
extern int justone;

static int r_save = -1;
static int p_save = -1;

/* Function to let remote-mode user know where their file(s) went */

int whereflg = 1;			/* Unset with SET XFER REPORT */

static VOID
wheremsg() {
    extern int quiet, filrej;
    int n;
    n = filcnt - filrej;
    debug(F101,"wheremsg n","",n);

    debug(F110,"wheremsg prfspec",prfspec,0);
    debug(F110,"wheremsg rfspec",rfspec,0);
    debug(F110,"wheremsg psfspec",psfspec,0);
    debug(F110,"wheremsg sfspec",sfspec,0);

    debug(F110,"wheremsg prrfspec",prrfspec,0);
    debug(F110,"wheremsg rrfspec",rrfspec,0);
    debug(F110,"wheremsg psrfspec",psrfspec,0);
    debug(F110,"wheremsg srfspec",srfspec,0);

    if (!quiet && !local) {
	if (n == 1) {
	    switch (myjob) {
	      case 's':
		if (sfspec) {
		    printf(" SENT: [%s]",sfspec);
		    if (srfspec)
		      printf(" To: [%s]",srfspec);
		    printf(" (%s)\r\n", success ? "OK" : "FAILED");
		}
		break;
	      case 'r':
	      case 'v':
		if (rrfspec) {
		    printf(" RCVD: [%s]",rrfspec);
		    if (rfspec)
		      printf(" To: [%s]",rfspec);
		    printf(" (%s)\r\n", success ? "OK" : "FAILED");
		}
	    }
	} else if (n > 1) {
	    switch (myjob) {
	      case 's':
		if (sfspec) {
		    printf(" SENT: (%d files)",n);
		    if (srfspec)
		      printf(" Last: [%s]",srfspec);
		    printf(" (%s)\r\n", success ? "OK" : "FAILED");
		}
		break;
	      case 'r':
	      case 'v':
		if (rrfspec) {
		    printf(" RCVD: (%d files)",n);
		    if (rfspec)
		      printf(" Last: [%s]",rfspec);
		    printf(" (%s)\r\n", success ? "OK" : "FAILED");
		}
	    }
	} else if (n == 0) {
	    if (myjob == 's')
	      printf(" SENT: (0 files)          \r\n");
	    else if (myjob == 'r' || myjob == 'v')
	      printf(" RCVD: (0 files)          \r\n");
	}
    }
}

static VOID
rdebug() {
    if (server)
      debug(F111,"RESUME","server=1",justone);
    else
      debug(F111,"RESUME","server=0",justone);
}

/* Flags for the ENABLE and DISABLE commands */
extern int
  en_cpy, en_cwd, en_del, en_dir, en_fin, en_get, en_bye, en_mai, en_pri,
  en_hos, en_ren, en_sen, en_spa, en_set, en_typ, en_who, en_ret, en_xit,
  en_mkd, en_rmd;
#ifndef NOSPL
extern int en_asg, en_que;
#endif /* NOSPL */
extern int what, lastxfer;

/* Global variables declared here */

  int whatru = 0;			/* What are you. */
  int whatru2 = 0;			/* What are you, cont'd. */

/* Local variables */

  static char vstate = 0;  		/* Saved State   */
  static char vcmd = 0;    		/* Saved Command */
  static int reget = 0;			/* Flag for executing REGET */
  static int retrieve = 0;		/* Flag for executing RETRIEVE */
  static int opkt = 0;			/* Send Extended GET packet */

  static int x;				/* General-purpose integer */
  static char *s;			/* General-purpose string pointer */

/* Macros - Note, BEGIN is predefined by Wart (and Lex) as "state = ", */
/* BEGIN is NOT a GOTO! */
#define TINIT if (tinit(1) < 0) return(-9)
#define SERVE { TINIT; resetc(); nakstate=1; what=W_NOTHING; cmarg2=""; \
sendmode=SM_SEND; havefs=0; recursive=r_save; fnspath=p_save; BEGIN serve; }
#define RESUME { rdebug(); if (!server) { wheremsg(); return(0); } else \
if (justone) { justone=0; wheremsg(); return(0); } else { SERVE; } }

#ifdef GFTIMER
#define QUIT x=quiet; quiet=1; clsif(); clsof(1); tsecs=gtimer(); \
 fptsecs=gftimer(); quiet=x; return(success)
#else
#define QUIT x=quiet; quiet=1; clsif(); clsof(1); tsecs=gtimer(); quiet=x; \
 return(success)
#endif /* GFTIMER */

/*
  By late 1999, the big switch() statement generated from the following state
  table began choking even gcc, so here we extract the code from the larger
  states into static routines to reduce the size of the cases and the
  switch() overall.  The routines follow the state table; the prototypes are
  here.  Each of these routines simply contains the text from the
  corresponding case, but with return(-1) added in appropriate places; see
  instructions after the state table switcher.
*/
static int rc;				/* Return code for these routines */
static int rcv_s_pkt();			/* Received an S packet */
static int rcv_firstdata();		/* Received first Data packet */
static int rcv_shortreply();		/* Short reply to a REMOTE command  */
static int srv_query();			/* Server answers an query */
static int srv_copy();			/* Server executes REMOTE COPY */
static int srv_rename();		/* Server executes REMOTE RENAME */
static int srv_login();			/* Server executes REMOTE LOGIN */
static int srv_timeout();		/* Server times out */

%%

/*
  Protocol entry points, one for each start state (sstate).
  The lowercase letters are internal "inputs" from the user interface.
  NOTE: The start state letters that appear on the left margin immediately
  below can NOT be used as packet types OR as G-packet subcodes.
*/

s { TINIT;				/* Send file(s) */
    if (sinit() > 0) BEGIN ssinit;
       else RESUME; }

v { TINIT; nakstate = 1; BEGIN get; }	/* Receive file(s) */

r {					/* Client sends a GET command */
    TINIT;
    vstate = get;
    reget = 0;
    retrieve = 0;
    opkt = 0;
    vcmd = 0;
#ifdef PKTZEROHACK
    ipktack[0] = NUL;
#endif /* PKTZEROHACK */
    if (sipkt('I') >= 0)
      BEGIN ipkt;
    else
      RESUME;
}

h {					/* Client sends a RETRIEVE command */
    TINIT;
    vstate = get;
    reget = 0;
    retrieve = 1;
    opkt = 0;
    vcmd = 0;
    if (sipkt('I') >= 0)
      BEGIN ipkt;
    else
      RESUME;
}
j {					/* Client sends a REGET command */
    TINIT;
    vstate = get;
    reget = 1;
    retrieve = 0;
    opkt = 0;
    vcmd = 0;
    if (sipkt('I') >= 0)
      BEGIN ipkt;
    else
      RESUME;
}
o {					/* Client sends Extended GET Packet */
    TINIT;
    vstate = get;
    reget = oopts & GOPT_RES;
    retrieve = oopts & GOPT_DEL;
    opkt = 1;
    vcmd = 0;
    if (sipkt('I') >= 0)
      BEGIN ipkt;
    else
      RESUME;
}
c {					/* Client sends a Host command */
    TINIT;
    vstate = rgen;
    vcmd = 'C';
    if (sipkt('I') >= 0)
      BEGIN ipkt;
    else
      RESUME;
}
k { TINIT;				/* Client sends a Kermit command */
    vstate = rgen;
    vcmd = 'K';
    if (sipkt('I') >= 0)
      BEGIN ipkt;
    else
      RESUME;
}
g {					/* Client sends a REMOTE command */
    TINIT;
    vstate = rgen;
    vcmd = 'G';
    if (sipkt('I') >= 0)
      BEGIN ipkt;
    else
      RESUME;
}
x {					/* Enter server mode */
    int x;
    x = justone;
    if (!ENABLED(en_del)) {		/* If DELETE is disabled */
	if (fncact == XYFX_B ||		/* undo any file collision action */
	    fncact == XYFX_U ||		/* that could result in deletion or */
	    fncact == XYFX_A ||		/* modification of existing files. */
	    fncact == XYFX_X) {
#ifndef NOICP
	    extern int g_fncact;
	    g_fncact = fncact;		/* Save current setting */
#endif /* NOICP */
	    fncact = XYFX_R;		/* Change to RENAME */
	    debug(F101,"server DELETE disabled so fncact RENAME","",fncact);
	}
    }
    SERVE;				/* tinit() clears justone... */
    justone = x;
#ifdef IKSDB
    if (ikdbopen) slotstate(what, "SERVER", "", "");
#endif /* IKSDB */
}

a {
    int b1 = 0, b2 = 0;
    if (!data) TINIT;			/* "ABEND" -- Tell other side. */

    if (!bctf) {		     /* Block check 3 forced on all packets */
#ifndef pdp11
	if (epktflg) {			/* If because of E-PACKET command */
	    b1 = bctl; b2 = bctu;	/* Save block check type */
	    bctl = bctu = 1;		/* set it to 1 */
	}
#endif /* pdp11 */
    }
    errpkt((CHAR *)"User cancelled");	/* Send the packet */
    if (!bctf) {		     /* Block check 3 forced on all packets */
#ifndef pdp11
	if (epktflg) {			/* Restore the block check */
	    epktflg = 0;
	    bctl = b1; bctu = b2;
	}
    }
#endif /* pdp11 */
    success = 0;
    return(0);				/* Return from protocol. */
}

/*
  Dynamic states: <current-states>input-character { action }
  nakstate != 0 means we're in a receiving state, in which we send ACKs & NAKs.
*/

<rgen,get,serve,ropkt>S {		/* Receive Send-Init packet. */
    rc = rcv_s_pkt();
    cancel = 0;				/* Reset cancellation counter */
    debug(F101,"rcv_s_pkt","",rc);
    if (rc > -1) return(rc);		/* (see below) */
}

/* States in which we get replies back from commands sent to a server. */
/* Complicated because direction of protocol changes, packet number    */
/* stays at zero through I-G-S sequence, and complicated even more by  */
/* sliding windows buffer allocation. */

<ipkt>Y {				/* Get ack for I-packet */
    int x = 0;
#ifdef PKTZEROHACK
    ckstrncpy(ipktack,(char *)rdatap,PKTZEROLEN); /* Save a copy of the ACK */
    ipktlen = strlen(ipktack);
#endif /* PKTZEROHACK */
    spar(rdatap);			/* Set parameters */
    cancel = 0;
    winlo = 0;				/* Set window-low back to zero */
    debug(F101,"<ipkt>Y winlo","",winlo);
    urserver = 1;			/* So I know I'm talking to a server */
    if (vcmd) {				/* If sending a generic command */
	if (tinit(0) < 0) return(-9);	/* Initialize many things */
	x = scmd(vcmd,(CHAR *)cmarg);	/* Do that */
	if (x >= 0) x = 0;		/* (because of O-Packet) */
	debug(F101,"proto G packet scmd","",x);
	vcmd = 0;			/* and then un-remember it. */
    } else if (vstate == get) {
	debug(F101,"REGET sstate","",sstate);
	x = srinit(reget, retrieve, opkt); /* GET or REGET, etc */
    }
    if (x < 0) {			/* If command was too long */
	if (!srimsg)
	  srimsg = "Error sending string";
	errpkt((CHAR *)srimsg);		/* cancel both sides. */
	success = 0;
	RESUME;
    } else if (x > 0) {			/* Need to send more O-Packets */
	BEGIN ssopkt;
    } else {
	rtimer();			/* Reset the elapsed seconds timer. */
#ifdef GFTIMER
	rftimer();
#endif /* GFTIMER */
	winlo = 0;			/* Window back to 0, again. */
	debug(F101,"<ipkt>Y vstate","",vstate);
	nakstate = 1;			/* Can send NAKs from here. */
	BEGIN vstate;			/* Switch to desired state */
    }
}

<ssopkt>Y {				/* Got ACK to O-Packet */
    debug(F100,"CPCPRO <ssopkt>Y","",0);
    x = sopkt();
    debug(F101,"CPCPRO <ssopkt>Y x","",x);
    if (x < 0) {			/* If error */
	errpkt((CHAR *)srimsg);		/* cancel both sides. */
	success = 0;
	RESUME;
    } else if (x == 0) {		/* This was the last O-Packet */
	rtimer();			/* Reset the elapsed seconds timer. */
#ifdef GFTIMER
	rftimer();
#endif /* GFTIMER */
	winlo = 0;			/* Window back to 0, again. */
	debug(F101,"<ssopkt>Y winlo","",winlo);
	nakstate = 1;			/* Can send NAKs from here. */
	BEGIN vstate;			/* Switch to desired state */
    }
    debug(F101,"CPCPRO <ssopkt>Y not changing state","",x);
}

<ipkt>E {				/* Ignore Error reply to I packet */
    int x = 0;
    winlo = 0;				/* Set window-low back to zero */
    debug(F101,"<ipkt>E winlo","",winlo);
    if (vcmd) {				/* In case other Kermit doesn't */
	if (tinit(0) < 0) return(-9);
	x = scmd(vcmd,(CHAR *)cmarg);	/* understand I-packets. */
	if (x >= 0) x = 0;		/* (because of O-Packet) */
	vcmd = 0;			/* Otherwise act as above... */
    } else if (vstate == get) x = srinit(reget, retrieve, opkt);
    if (x < 0) {			/* If command was too long */
	errpkt((CHAR *)srimsg);		/* cancel both sides. */
	success = 0;
	RESUME;
    } else if (x > 0) {			/* Need to send more O-Packets */
	BEGIN ssopkt;
    } else {
	freerpkt(winlo);		/* Discard the Error packet. */
	debug(F101,"<ipkt>E winlo","",winlo);
	winlo = 0;			/* Back to packet 0 again. */
	nakstate = 1;			/* Can send NAKs from here. */
	BEGIN vstate;
    }
}

<get>Y {		/* Resend of previous I-pkt ACK, same seq number */
    freerpkt(0);			/* Free the ACK's receive buffer */
    resend(0);				/* Send the GET packet again. */
}

/* States in which we're being a server */

<serve,get>I {				/* Get I-packet */
#ifndef NOSERVER
    spar(rdatap);			/* Set parameters from it */
    ack1(rpar());			/* Respond with our own parameters */
#ifdef COMMENT
    pktinit();				/* Reinitialize packet numbers */
#else
#ifdef COMMENT
    /* This can't be right - it undoes the stuff we just negotiated */
    x = justone;
    tinit(1);				/* Reinitialize EVERYTHING */
    justone = x;			/* But this... */
#else
    tinit(0);				/* Initialize most things */
#endif /* COMMENT */
#endif /* COMMENT */
#endif /* NOSERVER */
    cancel = 0;				/* Reset cancellation counter */
}

<serve>R {				/* GET */
#ifndef NOSERVER
    if (x_login && !x_logged) {
	errpkt((CHAR *)"Login required");
	SERVE;
    } else if (sgetinit(0,0) < 0) {
	RESUME;
    } else {
#ifdef CKSYSLOG
	if (ckxsyslog >= SYSLG_PR && ckxlogging)
	  cksyslog(SYSLG_PR, 1, "server", "GET", (char *)srvcmd);
#endif /* CKSYSLOG */
	BEGIN ssinit;
    }
#endif /* NOSERVER */
}

<serve>H {				/* GET /DELETE (RETRIEVE) */
#ifndef NOSERVER
    if (x_login && !x_logged) {
	errpkt((CHAR *)"Login required");
	RESUME;
    } else if (!ENABLED(en_del)) {
	errpkt((CHAR *)"Deleting files is disabled");
	RESUME;
    } else if (sgetinit(0,0) < 0) {
	RESUME;
    } else {
	moving = 1;
#ifdef CKSYSLOG
	if (ckxsyslog >= SYSLG_PR && ckxlogging)
	  cksyslog(SYSLG_PR, 1, "server", "GET /DELETE", (char *)srvcmd);
#endif /* CKSYSLOG */
	BEGIN ssinit;
    }
#endif /* NOSERVER */
}

<serve>V {				/* GET /RECURSIVE */
#ifndef NOSERVER
    recursive = 1;			/* Set these before sgetinit() */
    if (fnspath == PATH_OFF)
      fnspath = PATH_REL;		/* Don't worry, they will be */
    if (x_login && !x_logged) {		/* reset next time through. */
	errpkt((CHAR *)"Login required");
	RESUME;
    } else if (sgetinit(0,0) < 0) {
	RESUME;
    } else {
#ifdef CKSYSLOG
	if (ckxsyslog >= SYSLG_PR && ckxlogging)
	  cksyslog(SYSLG_PR, 1, "server", "GET /RECURSIVE", (char *)srvcmd);
#endif /* CKSYSLOG */
	BEGIN ssinit;
    }
#endif /* NOSERVER */
}

<serve>W {				/* GET /RECURSIVE /DELETE */
#ifndef NOSERVER
    recursive = 1;			/* Set these before sgetinit() */
    if (fnspath == PATH_OFF)
      fnspath = PATH_REL;		/* Don't worry, they will be */
    moving = 1;				/* reset next time through. */
    if (x_login && !x_logged) {
	errpkt((CHAR *)"Login required");
	RESUME;
    } else if (!ENABLED(en_del)) {
	errpkt((CHAR *)"Deleting files is disabled");
	RESUME;
    } else if (sgetinit(0,0) < 0) {
	RESUME;
    } else {
#ifdef CKSYSLOG
	if (ckxsyslog >= SYSLG_PR && ckxlogging)
	  cksyslog(SYSLG_PR,1,"server",
		   "GET /RECURSIVE /DELETE",(char *)srvcmd);
#endif /* CKSYSLOG */
	BEGIN ssinit;
    }
#endif /* NOSERVER */
}

<serve>J {				/* GET /RECOVER (REGET) */
#ifndef NOSERVER
    if (x_login && !x_logged) {
	errpkt((CHAR *)"Login required");
	SERVE;
    } else if (sgetinit(1,0) < 0) {
	RESUME;
    } else {
#ifdef CKSYSLOG
	if (ckxsyslog >= SYSLG_PR && ckxlogging)
	  cksyslog(SYSLG_PR, 1, "server", "GET /RECOVER", (char *)srvcmd);
#endif /* CKSYSLOG */
	BEGIN ssinit;
    }
#endif /* NOSERVER */
}

<serve>O {				/* Extended GET */
#ifndef NOSERVER
    if (x_login && !x_logged) {		/* (any combination of options) */
	errpkt((CHAR *)"Login required");
	SERVE;
    } else if ((x = sgetinit(0,1)) < 0) {
	debug(F101,"CKCPRO <serve>O sgetinit fail","",x);
	RESUME;
    } else if (x == 0) {
	debug(F101,"CKCPRO <serve>O sgetinit done","",x);
#ifdef CKSYSLOG
	if (ckxsyslog >= SYSLG_PR && ckxlogging)
	  cksyslog(SYSLG_PR, 1, "server", "EXTENDED GET", (char *)srvcmd);
#endif /* CKSYSLOG */
	BEGIN ssinit;
    } else {				/* Otherwise stay in this state */
	debug(F101,"CKCPRO <serve>O sgetinit TBC","",x);
	ack();
	BEGIN ropkt;
    }
#endif /* NOSERVER */
}

<ropkt>O {
#ifndef NOSERVER
    if (x_login && !x_logged) {		/* (any combination of options) */
	errpkt((CHAR *)"Login required");
	SERVE;
    } else if ((x = sgetinit(0,1)) < 0) {
	debug(F101,"CKCPRO <ropkt>O sgetinit fail","",x);
	RESUME;
    } else if (x == 0) {
	debug(F101,"CKCPRO <ropkt>O sgetinit done","",x);
#ifdef CKSYSLOG
	if (ckxsyslog >= SYSLG_PR && ckxlogging)
	  cksyslog(SYSLG_PR, 1, "server", "EXTENDED GET", (char *)srvcmd);
#endif /* CKSYSLOG */
	BEGIN ssinit;
    } else {				/* Otherwise stay in this state */
	debug(F101,"CKCPRO <ropkt>O sgetinit TBC","",x);
	ack();
    }
#endif /* NOSERVER */
}

<serve>G {				/* Generic server command */
#ifndef NOSERVER
    srvptr = srvcmd;			/* Point to command buffer */
    decode(rdatap,putsrv,0);		/* Decode packet data into it */
    putsrv(NUL);			/* Insert a couple nulls */
    putsrv(NUL);			/* for termination */
    if (srvcmd[0]) {
	sstate = srvcmd[0];		/* Set requested start state */
	if (x_login && !x_logged &&	/* Login required? */
	    /* Login, Logout, and Help are allowed when not logged in */
	    sstate != 'I' && sstate != 'L' && sstate != 'H') {
	    errpkt((CHAR *)"Login required");
	    SERVE;
	} else {
	    nakstate = 0;		/* Now I'm the sender. */
	    what = W_REMO;		/* Doing a REMOTE command. */
#ifdef STREAMING
	    if (!streaming)
#endif /* STREAMING */
	      if (timint < 1)
		timint = chktimo(rtimo,timef); /* Switch to per-packet timer */
	    binary = XYFT_T;		/* Switch to text mode */
	    BEGIN generic;		/* Switch to generic command state */
	}
    } else {
	errpkt((CHAR *)"Badly formed server command"); /* report error */
	RESUME;			/* & go back to server command wait */
    }
#endif /* NOSERVER */
}

<serve>C {				/* Receive Host command */
#ifndef NOSERVER
    if (x_login && !x_logged) {
	errpkt((CHAR *)"Login required");
	SERVE;
    } else if (!ENABLED(en_hos)) {
	errpkt((CHAR *)"REMOTE HOST disabled");
	RESUME;
    } else if (nopush) {
	errpkt((CHAR *)"HOST commands not available");
	RESUME;
    } else {
	srvptr = srvcmd;		/* Point to command buffer */
	decode(rdatap,putsrv,0);	/* Decode command packet into it */
	putsrv(NUL);			/* Null-terminate */
	nakstate = 0;			/* Now sending, not receiving */
	binary = XYFT_T;		/* Switch to text mode */
	if (syscmd((char *)srvcmd,"")) { /* Try to execute the command */
	    what = W_REMO;		/* Doing a REMOTE command. */
#ifdef STREAMING
	    if (!streaming)
#endif /* STREAMING */
	      if (timint < 1)
		timint = chktimo(rtimo,timef); /* Switch to per-packet timer */
#ifdef CKSYSLOG
	    if (ckxsyslog >= SYSLG_PR && ckxlogging)
	      cksyslog(SYSLG_PR, 1, "server", "REMOTE HOST", (char *)srvcmd);
#endif /* CKSYSLOG */
	    BEGIN ssinit;		/* If OK, send back its output */
	} else {			/* Otherwise */
	    errpkt((CHAR *)"Can't do system command"); /* report error */
	    RESUME;			/* & go back to server command wait */
	}
    }
#endif /* NOSERVER */
}

<serve>q {				/* Interrupted or connection lost */
    rc = srv_timeout();
    debug(F101,"srv_timeout","",rc);
    if (rc > -1) return(rc);		/* (see below) */
}

<serve>N {				/* Server got a NAK in command-wait */
#ifndef NOSERVER
    errpkt((CHAR *)"Did you say RECEIVE instead of GET?");
    RESUME;
#endif /* NOSERVER */
}

<serve>. {				/* Any other command in this state */
#ifndef NOSERVER
    if (c != ('E' - SP) && c != ('Y' - SP)) /* except E and Y packets. */
      errpkt((CHAR *)"Unimplemented server function");
    /* If we answer an E with an E, we get an infinite loop. */
    /* A Y (ACK) can show up here if we sent back a short-form reply to */
    /* a G packet and it was echoed.  ACKs can be safely ignored here. */
    RESUME;				/* Go back to server command wait. */
#endif /* NOSERVER */
}

<generic>I {				/* Login/Out */
    rc = srv_login();
    debug(F101,"<generic>I srv_login","",rc);
    if (rc > -1) return(rc);		/* (see below) */
}

<generic>C {				/* Got REMOTE CD command */
#ifndef NOSERVER
#ifdef CKSYSLOG
    if (ckxsyslog >= SYSLG_PR && ckxlogging)
      cksyslog(SYSLG_PR, 1, "server", "REMOTE CD", (char *)srvcmd);
#endif /* CKSYSLOG */
    if (!ENABLED(en_cwd)) {
	errpkt((CHAR *)"REMOTE CD disabled");
	RESUME;
    } else {
	char * p = NULL;
	x = cwd((char *)(srvcmd+1));	/* Try to change directory */
#ifdef IKSDB
	if (ikdbopen) slotstate(what,"REMOTE CD", (char *)(srvcmd+2), "");
#endif /* IKSDB */
	if (!x) {			/* Failed */
	    errpkt((CHAR *)"Can't change directory");
	    RESUME;			/* Back to server command wait */
	} else if (x == 2) {		/* User wants message */
	    if (!ENABLED(en_typ)) {	/* Messages (REMOTE TYPE) disabled? */
		errpkt((CHAR *)"REMOTE TYPE disabled");
		RESUME;
	    } else {			/* TYPE is enabled */
		int i;
		for (i = 0; i < 8; i++) {
		    if (zchki(cdmsgfile[i]) > -1) {
			break;
		    }
		}
		binary = XYFT_T;	/* Use text mode for this. */
		if (i < 8 && sndtype(cdmsgfile[i])) { /* Have readme file? */
		    BEGIN ssinit;	/* OK */
		} else {		/* not OK */
		    p = zgtdir();
		    if (!p) p = "";
		    success = (*p) ? 1 : 0;
		    ack1((CHAR *)p);	/* ACK with new directory name */
		    success = 1;
		    RESUME;		/* wait for next server command */
		}
	    }
	} else {			/* User doesn't want message */
	    p = zgtdir();
	    if (!p) p = "";
	    success = (*p) ? 1 : 0;
	    ack1((CHAR *)p);
	    success = 1;
	    RESUME;			/* Wait for next server command */
	}
    }
#endif /* NOSERVER */
}

<generic>A {				/* Got REMOTE PWD command */
#ifndef NOSERVER
#ifdef CKSYSLOG
    if (ckxsyslog >= SYSLG_PR && ckxlogging)
      cksyslog(SYSLG_PR, 1, "server", "REMOTE PWD", NULL);
#endif /* CKSYSLOG */
    if (!ENABLED(en_cwd)) {
	errpkt((CHAR *)"REMOTE CD disabled");
	RESUME;
    } else {
	if (encstr((CHAR *)zgtdir()) > -1) { /* Encode current directory */
	    ack1(data);			/* If it fits, send it back in ACK */
	    success = 1;
	} else {			/* Failed */
	    ack();			/* Send empty ACK */
	    success = 0;		/* and indicate failure locally */
	}
	RESUME;				/* Back to server command wait */
    }
#endif /* NOSERVER */
}

<generic>D {				/* REMOTE DIRECTORY command */
#ifndef NOSERVER
    char *n2;
#ifdef CKSYSLOG
    if (ckxsyslog >= SYSLG_PR && ckxlogging)
      cksyslog(SYSLG_PR, 1, "server", "REMOTE DIRECTORY", (char *)srvcmd);
#endif /* CKSYSLOG */
    if (!ENABLED(en_dir)) {		/* If DIR is disabled, */
	errpkt((CHAR *)"REMOTE DIRECTORY disabled"); /* refuse. */
	RESUME;
    } else {				/* DIR is enabled. */
#ifdef IKSDB
	if (ikdbopen) slotstate(what,"REMOTE DIR", (char *)(srvcmd+2), "");
#endif /* IKSDB */
	if (!ENABLED(en_cwd)) {		/* But CWD is disabled */
	    zstrip((char *)(srvcmd+2),&n2); /* and they included a pathname, */
	    if (strcmp((char *)(srvcmd+2),n2)) { /* so refuse. */
		errpkt((CHAR *)"Access denied");
		RESUME;			/* Remember, this is not a goto! */
	    }
	}
	if (state == generic) {			/* It's OK to go ahead. */
#ifdef COMMENT
	    n2 = (*(srvcmd+2)) ? DIRCMD : DIRCM2;
	    if (syscmd(n2,(char *)(srvcmd+2)))  /* If it can be done */
#else
	    int x;
	    if ((x = snddir((char*)(srvcmd+2))) > 0)
#endif /* COMMENT */
	    {
		BEGIN ssinit;		/* send the results back; */
	    } else {			/* otherwise */
		if (x < 0)
		  errpkt((CHAR *)"No files match");
		else
		  errpkt((CHAR *)"Can't list directory");
		RESUME;			/* return to server command wait */
	    }
	}
    }
#endif /* NOSERVER */
}

<generic>E {				/* REMOTE DELETE (Erase) */
#ifndef NOSERVER
    char *n2;
#ifdef CKSYSLOG
    if (ckxsyslog >= SYSLG_PR && ckxlogging)
      cksyslog(SYSLG_PR, 1, "server", "REMOTE DELETE", (char *)srvcmd);
#endif /* CKSYSLOG */
    if (!ENABLED(en_del)) {
	errpkt((CHAR *)"REMOTE DELETE disabled");
	RESUME;
    } else {				/* DELETE is enabled */
#ifdef IKSDB
	if (ikdbopen) slotstate(what,"REMOTE DELETE", (char *)(srvcmd+2), "");
#endif /* IKSDB */
	if (!ENABLED(en_cwd)) {		/* but CWD is disabled */
	    zstrip((char *)(srvcmd+2),&n2); /* and they included a pathname, */
	    if (strcmp((char *)(srvcmd+2),n2)) { /* so refuse. */
		errpkt((CHAR *)"Access denied");
		RESUME;			/* Remember, this is not a goto! */
	    }
	} else if (isdir((char *)(srvcmd+2))) { /* A directory name? */
	    errpkt((CHAR *)"It's a directory");
	    RESUME;
	}
	if (state == generic) {		/* It's OK to go ahead. */
	    int x;
	    if ((x = snddel((char*)(srvcmd+2))) > 0) {
		BEGIN ssinit;		/* If OK send results back */
	    } else {			/* otherwise */
		if (x < 0)
		  errpkt((CHAR *)"File not found"); /* report failure */
		else
		  errpkt((CHAR *)"DELETE failed");
		RESUME;			/* & return to server command wait */
	    }
	}
    }
#endif /* NOSERVER */
}

<generic>F {				/* FINISH */
#ifndef NOSERVER
#ifdef CKSYSLOG
    if (ckxsyslog >= SYSLG_PR && ckxlogging)
      cksyslog(SYSLG_PR, 1, "server", "FINISH", NULL);
#endif /* CKSYSLOG */
#ifdef IKSDB
    if (ikdbopen) slotstate(what,"SERVER FINISH", "", "");
#endif /* IKSDB */
    if (!ENABLED(en_fin)) {
	errpkt((CHAR *)"FINISH disabled");
	RESUME;
    } else {
	ack();				/* Acknowledge */
	xxscreen(SCR_TC,0,0L,"");	/* Display */
	success = 1;
	return(0);			/* Done */
    }
#endif /* NOSERVER */
}

<generic>X {				/* EXIT */
#ifndef NOSERVER
#ifdef CKSYSLOG
    if (ckxsyslog >= SYSLG_PR && ckxlogging)
      cksyslog(SYSLG_PR, 1, "server", "REMOTE EXIT", NULL);
#endif /* CKSYSLOG */
#ifdef IKSDB
    if (ikdbopen) slotstate(what,"REMOTE EXIT", "", "");
#endif /* IKSDB */
    if (!ENABLED(en_xit)) {
	errpkt((CHAR *)"EXIT disabled");
	RESUME;
    } else {
	ack();				/* Acknowledge */
	xxscreen(SCR_TC,0,0L,"");	/* Display */
	doexit(GOOD_EXIT,xitsta);
    }
#endif /* NOSERVER */
}

<generic>L {				/* BYE (Logout) */
#ifndef NOSERVER
#ifdef CKSYSLOG
    if (ckxsyslog >= SYSLG_PR && ckxlogging)
      cksyslog(SYSLG_PR, 1, "server", "BYE", NULL);
#endif /* CKSYSLOG */
#ifdef IKSDB
    if (ikdbopen) slotstate(what,"SERVER BYE", "", "");
#endif /* IKSDB */
    if (!ENABLED(en_bye)) {
	errpkt((CHAR *)"BYE disabled");
	RESUME;
    } else {
	ack();				/* Acknowledge */
	success = 1;
	msleep(750);			/* Give the ACK time to get out */
	if (local)
	  ttres();			/* Reset the terminal */
	xxscreen(SCR_TC,0,0L,"");	/* Display */
	doclean(1);			/* Clean up files, etc */
#ifdef DEBUG
	debug(F100,"C-Kermit BYE - Logging out...","",0);
	zclose(ZDFILE);
#endif /* DEBUG */
#ifdef IKSD
#ifdef CK_LOGIN
	if (inserver)
	  ckxlogout();
	else
#endif /* CK_LOGIN */
#endif /* IKSD */
#ifdef TCPSOCKET
#ifndef NOLISTEN
	  if (network && tcpsrfd > 0 && !inserver)
	    doexit(GOOD_EXIT,xitsta);
	else
#endif /* NOLISTEN */
#endif /* TCPSOCKET */
	  return(zkself());		/* Try to log self out */
    }
#endif /* NOSERVER */
}

<generic>H {				/* REMOTE HELP */
#ifdef CKSYSLOG
    if (ckxsyslog >= SYSLG_PR && ckxlogging)
      cksyslog(SYSLG_PR, 1, "server", "REMOTE HELP", NULL);
#endif /* CKSYSLOG */
#ifdef IKSDB
    if (ikdbopen) slotstate(what,"REMOTE HELP", "", "");
#endif /* IKSDB */
#ifndef NOSERVER
    if (sndhlp()) {
	BEGIN ssinit;			/* try to send it */
    } else {				/* If not ok, */
	errpkt((CHAR *)"Can't send help"); /* send error message instead */
	RESUME;				/* and return to server command wait */
    }
#endif /* NOSERVER */
}

<generic>R {                            /* REMOTE RENAME */
    rc = srv_rename();
    debug(F101,"srv_rename","",rc);
    if (rc > -1) return(rc);		/* (see below) */
}

<generic>K {                            /* REMOTE COPY */
    rc = srv_copy();
    debug(F101,"srv_copy","",rc);
    if (rc > -1) return(rc);		/* (see below) */
}

<generic>S {				/* REMOTE SET */
#ifdef CKSYSLOG
    if (ckxsyslog >= SYSLG_PR && ckxlogging)
      cksyslog(SYSLG_PR, 1, "server", "REMOTE SET", (char *)srvcmd);
#endif /* CKSYSLOG */
#ifndef NOSERVER
#ifdef IKSDB
    if (ikdbopen) slotstate(what,"REMOTE SET", (char *)(srvcmd+1), "");
#endif /* IKSDB */
    if (!ENABLED(en_set)) {
	errpkt((CHAR *)"REMOTE SET disabled");
	RESUME;
    } else {
	if (remset((char *)(srvcmd+1))) { /* Try to do what they ask */
	    success = 1;
	    ack();			/* If OK, then acknowledge */
	} else				/* Otherwise */
	  errpkt((CHAR *)"Unknown REMOTE SET parameter"); /* give error msg */
	RESUME;				/* Return to server command wait */
    }
#endif /* NOSERVER */
}

<generic>T {				/* REMOTE TYPE */
#ifndef NOSERVER
    char *n2;
#ifdef CKSYSLOG
    if (ckxsyslog >= SYSLG_PR && ckxlogging)
      cksyslog(SYSLG_PR, 1, "server", "REMOTE TYPE", (char *)srvcmd);
#endif /* CKSYSLOG */
    if (!ENABLED(en_typ)) {
	errpkt((CHAR *)"REMOTE TYPE disabled");
	RESUME;
    } else {
#ifdef IKSDB
	if (ikdbopen) slotstate(what,"REMOTE TYPE", (char *)(srvcmd+2), "");
#endif /* IKSDB */
	if (!ENABLED(en_cwd)) {		/* If CWD disabled */
	    zstrip((char *)(srvcmd+2),&n2); /* and they included a pathname, */
	    if (strcmp((char *)(srvcmd+2),n2)) { /* refuse. */
		errpkt((CHAR *)"Access denied");
		RESUME;			/* Remember, this is not a goto! */
	    }
	}
	if (state == generic) {		/* It's OK to go ahead. */
	    binary = XYFT_T;		/* Use text mode for this. */
	    if (			/* (RESUME didn't change state) */
#ifdef COMMENT
	      syscmd(TYPCMD,(char *)(srvcmd+2))	/* Old way */
#else
	      sndtype((char *)(srvcmd+2)) /* New way */
#endif /* COMMENT */
		)
	      BEGIN ssinit;		/* OK */
	    else {			/* not OK */
		errpkt((CHAR *)"Can't type file"); /* give error message */
		RESUME;			/* wait for next server command */
	    }
	}
    }
#endif /* NOSERVER */
}

<generic>m {				/* REMOTE MKDIR */
#ifndef NOSERVER
#ifdef CK_MKDIR
#ifdef CKSYSLOG
    if (ckxsyslog >= SYSLG_PR && ckxlogging)
      cksyslog(SYSLG_PR, 1, "server", "REMOTE MKDIR", (char *)srvcmd);
#endif /* CKSYSLOG */
#ifdef IKSDB
    if (ikdbopen) slotstate(what,"REMOTE MKDIR", (char *)(srvcmd+2), "");
#endif /* IKSDB */
    if (!ENABLED(en_mkd)) {
	errpkt((CHAR *)"REMOTE MKDIR disabled");
	RESUME;
    } else if (!ENABLED(en_cwd)) {	/* If CWD disabled */
	errpkt((CHAR *)"Directory access restricted");
	RESUME;				/* Remember, this is not a goto! */
    }
    if (state == generic) {		/* OK to go ahead. */
	char *p = NULL;
	x = ckmkdir(0,(char *)(srvcmd+2),&p,0,1); /* Make the directory */
	if (!p) p = "";
	if (x > -1) {
	    encstr((CHAR *)p);		/* OK - encode the name */
	    ack1(data);			/* Send short-form response */
	    success = 1;
	    RESUME;
	} else {			/* not OK */
	    if (!*p) p = "Directory creation failure";
	    errpkt((CHAR *)p);		/* give error message */
	    RESUME;			/* Wait for next server command */
	}
    }
#else
    errpkt((CHAR *)"REMOTE MKDIR not available");
    RESUME;
#endif /* CK_MKDIR */
#endif /* NOSERVER */
}

<generic>d {				/* REMOTE RMDIR */
#ifndef NOSERVER
#ifdef CK_MKDIR
#ifdef CKSYSLOG
    if (ckxsyslog >= SYSLG_PR && ckxlogging)
      cksyslog(SYSLG_PR, 1, "server", "REMOTE RMDIR", (char *)srvcmd);
#endif /* CKSYSLOG */
#ifdef IKSDB
    if (ikdbopen) slotstate(what,"REMOTE RMDIR", (char *)(srvcmd+2), "");
#endif /* IKSDB */
    if (!ENABLED(en_rmd)) {
	errpkt((CHAR *)"REMOTE RMDIR disabled");
	RESUME;
    } else if (!ENABLED(en_cwd)) {	/* If CWD disabled */
	errpkt((CHAR *)"Directory access restricted");
	RESUME;				/* Remember, this is not a goto! */
    }
    if (state == generic) {		/* OK to go ahead. */
	char *p = NULL;
	x = ckmkdir(1,(char *)(srvcmd+2),&p,0,1);
	if (!p) p = "";
	if (x > -1) {
	    encstr((CHAR *)p);		/* OK - encode the name */
	    ack1(data);			/* Send short-form response */
	    success = 1;
	    RESUME;
	} else {			/* not OK */
	    if (!*p) p = "Directory removal failure";
	    errpkt((CHAR *)p);		/* give error message */
	    RESUME;			/* Wait for next server command */
	}
    }
#else
    errpkt((CHAR *)"REMOTE RMDIR not available");
    RESUME;
#endif /* CK_MKDIR */
#endif /* NOSERVER */
}

<generic>U {				/* REMOTE SPACE */
#ifndef NOSERVER
#ifdef CKSYSLOG
    if (ckxsyslog >= SYSLG_PR && ckxlogging)
      cksyslog(SYSLG_PR, 1, "server", "REMOTE SPACE", (char *)srvcmd);
#endif /* CKSYSLOG */
    if (!ENABLED(en_spa)) {
	errpkt((CHAR *)"REMOTE SPACE disabled");
	RESUME;
    } else {
	x = srvcmd[1];			/* Get area to check */
	x = ((x == NUL) || (x == SP)
#ifdef OS2
	     || (x == '!') || (srvcmd[3] == ':')
#endif /* OS2 */
	     );
#ifdef IKSDB
	if (ikdbopen) slotstate(what,
			      "REMOTE SPACE",
			      (x ? "" : (char *)srvcmd),
			      ""
			      );
#endif /* IKSDB */
	if (!x && !ENABLED(en_cwd)) {	/* CWD disabled */
	    errpkt((CHAR *)"Access denied"); /* and non-default area given, */
	    RESUME;			/* refuse. */
	} else {
#ifdef OS2
_PROTOTYP(int sndspace,(int));
	    if (sndspace(x ? toupper(srvcmd[2]) : 0)) {
		BEGIN ssinit;		/* send the report. */
	    } else {			/* If not ok, */
		errpkt((CHAR *)"Can't send space"); /* send error message */
		RESUME;			/* and return to server command wait */
	    }
#else
            if (nopush)
              x = 0;
            else
              x = (x ? syscmd(SPACMD,"") : syscmd(SPACM2,(char *)(srvcmd+2)));
	    if (x) {			/* If we got the info */
		BEGIN ssinit;		/* send it */
	    } else {			/* otherwise */
		errpkt((CHAR *)"Can't check space"); /* send error message */
		RESUME;			/* and await next server command */
	    }
#endif /* OS2 */
	}
    }
#endif /* NOSERVER */
}

<generic>W {				/* REMOTE WHO */
#ifndef NOSERVER
#ifdef CKSYSLOG
    if (ckxsyslog >= SYSLG_PR && ckxlogging)
      cksyslog(SYSLG_PR, 1, "server", "REMOTE WHO", (char *)srvcmd);
#endif /* CKSYSLOG */
#ifdef IKSDB
    if (ikdbopen) slotstate(what,"REMOTE WHO", (char *)(srvcmd+2), "");
#endif /* IKSDB */
    if (!ENABLED(en_who)) {
	errpkt((CHAR *)"REMOTE WHO disabled");
	RESUME;
    } else {
#ifdef OS2
_PROTOTYP(int sndwho,(char *));
	    if (sndwho((char *)(srvcmd+2))) {
		BEGIN ssinit;		/* try to send it */
	    } else {			/* If not ok, */
		errpkt((CHAR *)"Can't do who command"); /* send error msg */
		RESUME;			/* and return to server command wait */
	    }
#else
	if (syscmd(WHOCMD,(char *)(srvcmd+2))) {
	    BEGIN ssinit;
	} else {
	    errpkt((CHAR *)"Can't do who command");
	    RESUME;
	}
#endif /* OS2 */
    }
#endif /* NOSERVER */
}

<generic>V {				/* Variable query or set */
    rc = srv_query();
    debug(F101,"srv_query","",rc);
    if (rc > -1) return(rc);
}

<generic>M {				/* REMOTE MESSAGE command */
#ifndef NOSERVER
    debug(F110,"RMSG",(char *)srvcmd+2,0);
    xxscreen(SCR_MS,0,0L,(char *)(srvcmd+2));
    ack();
    RESUME;
#endif	/* NOSERVER */
}

<generic>q {				/* Interrupted or connection lost */
#ifndef NOSERVER
    if (fatalio) {			/* Connection lost */
#ifdef CKSYSLOG
	if (ckxsyslog >= SYSLG_PR && ckxlogging)
	  cksyslog(SYSLG_PR, 1, "server", "Interrupted", NULL);
#endif /* CKSYSLOG */
	success = 0;
	xitsta |= (what & W_KERMIT);
	QUIT;
    } else if (interrupted) {
	if (!ENABLED(en_fin)) {		/* Ctrl-C typed */
	    errpkt((CHAR *)"QUIT disabled");
	    RESUME;
	} else {
#ifdef CKSYSLOG
	    if (ckxsyslog >= SYSLG_PR && ckxlogging)
	      cksyslog(SYSLG_PR, 1, "server", "Interrupted", NULL);
#endif /* CKSYSLOG */
	    success = 0;
	    xitsta |= (what & W_KERMIT);
	    QUIT;
	}
    } else {				/* Shouldn't happen */
	debug(F100,"SERVER (generic) GOT UNEXPECTED 'q'","",0);
	QUIT;
    }
#endif /* NOSERVER */
}

<generic>. {				/* Anything else in this state... */
#ifndef NOSERVER
    errpkt((CHAR *)"Unimplemented REMOTE command"); /* Complain */
    RESUME;				/* and return to server command wait */
#endif /* NOSERVER */
}

<rgen>q {				/* Sent BYE and connection broken */
    if (bye_active && ttchk() < 0) {
	msleep(500);
	bye_active = 0;
	ttclos(0);			/* Close our end of the connection */
	clsof(0);
	return(success = 1);
    } else {				/* Other generic command */
	return(success = 0);		/* or connection not broken */
    }
}

<rgen>Y {				/* Short-Form reply */
    rc = rcv_shortreply();
    debug(F101,"<rgen>Y rcv_shortreply","",rc);
    if (rc > -1) return(rc);
}

<rgen,rfile>F {				/* File header */
    /* char *n2; */
    extern int rsn;
    debug(F101,"<rfile>F winlo 1","",winlo);
    xflg = 0;				/* Not screen data */
    if (!czseen)
      cancel = 0;			/* Reset cancellation counter */
#ifdef CALIBRATE
    if (dest == DEST_N)
      calibrate = 1;
#endif /* CALIBRATE */
    if (!rcvfil(filnam)) {		/* Figure out local filename */
	errpkt((CHAR *)rf_err);		/* Trouble */
	RESUME;
    } else {				/* Real file, OK to receive */
	char * fnp;
	debug(F111,"<rfile>F winlo 2",fspec,winlo);
	if (filcnt == 1)		/* rcvfil set this to 1 for 1st file */
	  crc16 = 0L;			/* Clear file CRC */
	fnp = fspec;			/* This is the full path */
	if (server && !ENABLED(en_cwd) || /* if DISABLE CD */
	    !fackpath			  /* or F-ACK-PATH OFF */
	    ) {
	    zstrip(fspec,&fnp);		/* don't send back full path */
	}
	encstr((CHAR *)fnp);
	if (fackbug)
	  ack();
	else
	  ack1(data);			/* Send it back in ACK */
	initattr(&iattr);		/* Clear file attribute structure */
	streamon();
	if (window(wslotn) < 0) {	/* Allocate negotiated window slots */
	    errpkt((CHAR *)"Can't open window");
	    RESUME;
	}
#ifdef IKSDB
	if (ikdbopen) slotstate(what,
			      server ? "SERVER" : "",
			      "RECEIVE",
			      fspec
			      );
#endif /* IKSDB */
	BEGIN rattr;			/* Now expect Attribute packets */
    }
}

<rgen,rfile>X {				/* X-packet instead of file header */
    xflg = 1;				/* Screen data */
    if (!czseen)
      cancel = 0;			/* Reset cancellation counter */
    ack();				/* Acknowledge the X-packet */
    initattr(&iattr);			/* Initialize attribute structure */
    streamon();
    if (window(wslotn) < 0) {		/* allocate negotiated window slots */
	errpkt((CHAR *)"Can't open window");
	RESUME;
    }
#ifndef NOSPL
    if (query) {			/* If this is the response to */
	qbufp = querybuf;		/* a query that we sent, initialize */
	qbufn = 0;			/* the response buffer */
	querybuf[0] = NUL;
    }
#endif /* NOSPL */
    what = W_REMO;			/* we're doing a REMOTE command */
#ifdef IKSDB
    if (ikdbopen) slotstate(what,
			  server ? "SERVER" : "",
			  "RECEIVE",
			  fspec
			  );
#endif /* IKSDB */
    BEGIN rattr;			/* Expect Attribute packets */
}

<rattr>A {				/* Attribute packet */
    if (gattr(rdatap,&iattr) == 0) {	/* Read into attribute structure */
#ifdef CK_RESEND
	ack1((CHAR *)iattr.reply.val);	/* Reply with data */
#else
	ack();				/* If OK, acknowledge */
#endif /* CK_RESEND */
    } else {				/* Otherwise */
	extern long fsize;
	char *r;
	r = getreason(iattr.reply.val);
	ack1((CHAR *)iattr.reply.val);	/* refuse to accept the file */
	xxscreen(SCR_ST,ST_REFU,0L,r);	/* reason */
#ifdef TLOG
	if (tralog && !tlogfmt)
	  doxlog(what,filnam,fsize,binary,1,r);
#endif /* TLOG */
    }
}

<rattr>D {				/* First data packet */
    debug(F100,"<rattr> D firstdata","",0);
    rc = rcv_firstdata();
    debug(F101,"rcv_firstdata rc","",rc);
    if (rc > -1) return(rc);		/* (see below) */
}

<rfile>B {				/* EOT, no more files */
    ack();				/* Acknowledge the B packet */
    reot();				/* Do EOT things */
#ifdef CK_TMPDIR
/* If we were cd'd temporarily to another device or directory ... */
    if (f_tmpdir) {
	int x;
	x = zchdir((char *) savdir);	/* ... restore previous directory */
	f_tmpdir = 0;			/* and remember we did it. */
	debug(F111,"ckcpro.w B tmpdir restoring",savdir,x);
    }
#endif /* CK_TMPDIR */
    RESUME;				/* and quit */
}

<rdpkt>D {				/* Got Data packet */
    debug(F101,"<rdpkt>D cxseen","",cxseen);
    debug(F101,"<rdpkt>D czseen","",czseen);
    if (cxseen || czseen || discard) {	/* If file or group interruption */
	CHAR * msg;
	msg = czseen ? (CHAR *)"Z" : (CHAR *)"X";
#ifdef STREAMING
	if (streaming) {		/* Need to cancel */
	    debug(F111,"<rdpkt>D streaming cancel",msg,cancel);
	    if (cancel++ == 0) {	/* Only do this once */
		ack1(msg);		/* Put "X" or "Z" in ACK */
	    } else if (czseen) {
		errpkt((CHAR *)"User canceled");
		RESUME;
	    } else {
		fastack();
	    }
	} else
#endif /* STREAMING */
	  ack1(msg);
    } else {				/* No interruption */
	int rc, qf;
#ifndef NOSPL
	qf = query;
#else
	qf = 0;
#endif /* NOSPL */
#ifdef CKTUNING
	rc = (binary && !parity) ?
	  bdecode(rdatap,putfil):
	    decode(rdatap, qf ? puttrm : putfil, 1);
#else
	rc = decode(rdatap, qf ? puttrm : putfil, 1);
#endif /* CKTUNING */
	if (rc < 0) {
	    discard = (keep == 0 || (keep == SET_AUTO && binary != XYFT_T));
	    errpkt((CHAR *)"Error writing data"); /* If failure, */
	    RESUME;
	} else				/* Data written OK, send ACK */
#ifdef STREAMING
	  if (streaming)
	    fastack();
	else
#endif /* STREAMING */
	  ack();
    }
}

<rattr>Z {				/* EOF immediately after A-Packet. */
    rf_err = "Can't create file";
    timint = s_timint;
    if (discard) {			/* Discarding a real file... */
	x = 1;
    } else if (xflg) {			/* If screen data */
	if (remfile) {			/* redirected to file */
	    if (rempipe)		/* or pipe */
	      x = openc(ZOFILE,remdest); /* Pipe: start command */
	    else
	      x = opena(remdest,&iattr); /* File: open with attributes */
	} else {			/* otherwise */
	    x = opent(&iattr);		/* "open" the screen */
	}
#ifdef CALIBRATE
    } else if (calibrate) {		/* If calibration run */
	x = ckopenx(&iattr);		/* do this */
#endif /* CALIBRATE */
    } else {				/* otherwise */
	x = opena(filnam,&iattr);	/* open the file, with attributes */
	if (x == -17) {			/* REGET skipped because same size */
	    discard = 1;
	    rejection = 1;
	}
    }
    if (!x || reof(filnam, &iattr) < 0) { /* Close output file */
	errpkt((CHAR *) rf_err);	/* If problem, send error msg */
	RESUME;				/* and quit */
    } else {				/* otherwise */
	if (x == -17)
	  xxscreen(SCR_ST,ST_SKIP,SKP_RES,"");
	ack();				/* acknowledge the EOF packet */
	BEGIN rfile;			/* and await another file */
    }
}

<rdpkt>q {  				/* Ctrl-C or connection loss. */
    timint = s_timint;
    window(1);				/* Set window size back to 1... */
    cxseen = 1;
    x = clsof(1);			/* Close file */
    return(success = 0);		/* Failed */
}

<rdpkt>Z {				/* End Of File (EOF) Packet */
/*  wslots = 1;	*/			/* (don't set) Window size back to 1 */
#ifndef COHERENT /* Coherent compiler blows up on this switch() statement. */
    x = reof(filnam, &iattr);		/* Handle the EOF packet */
    switch (x) {			/* reof() sets the success flag */
      case -5:				/* Handle problems */
	errpkt((CHAR *)"RENAME failed"); /* Fatal */
	RESUME;
	break;
      case -4:
	errpkt((CHAR *)"MOVE failed");	/* Fatal */
	RESUME;
	break;
      case -3:				/* If problem, send error msg */
	errpkt((CHAR *)"Can't print file"); /* Fatal */
	RESUME;
	break;
      case -2:
	errpkt((CHAR *)"Can't mail file"); /* Fatal */
	RESUME;
	break;
      case 2:				/* Not fatal */
      case 3:
	xxscreen(SCR_EM,0,0L,"Receiver can't delete temp file");
	RESUME;
	break;
      default:
	if (x < 0) {			/* Fatal */
	    errpkt((CHAR *)"Can't close file");
	    RESUME;
	} else {			/* Success */
#ifndef NOSPL
	    if (query)			/* Query reponses generally */
	      conoll("");		/* don't have line terminators */
#endif /* NOSPL */
	    if (czseen) {		/* Batch canceled? */
		if (cancel++ == 0) {	/* If we haven't tried this yet */
		    ack1((CHAR *)"Z");	/* Try it once */
		} else {		/* Otherwise */
		    errpkt((CHAR *)"User canceled"); /* quite with Error */
		    RESUME;
		}
	    } else
	      ack();			/* Acknowledge the EOF packet */
	    BEGIN rfile;		/* and await another file */
	}
    }
#else
    if (reof(filnam, &iattr) < 0) {	/* Close the file */
	errpkt((CHAR *)"Error at end of file");
	RESUME;
    } else {				/* reof() sets success flag */
	ack();
	BEGIN rfile;
    }
#endif /* COHERENT */
}

<ssinit>Y {				/* ACK for Send-Init */
    spar(rdatap);			/* set parameters from it */
    cancel = 0;
    if (bctf) {
	bctu = 3;
	bctl = 3;
    } else {
	bctu = bctr;			/* switch to agreed-upon block check */
	bctl = (bctu == 4) ? 2 : bctu;	/* Set block-check length */
    }

#ifdef CK_RESEND
    if ((sendmode == SM_RESEND) && (!atcapu || !rscapu)) { /* RESEND */
	errpkt((CHAR *) "RESEND capabilities not negotiated");
	RESUME;
    } else {
#endif /* CK_RESEND */
	what = W_SEND;			/* Remember we're sending */
	lastxfer = W_SEND;
	x = sfile(xflg);		/* Send X or F header packet */
	cancel = 0;			/* Reset cancellation counter */
	if (x) {			/* If the packet was sent OK */
	    if (!xflg && filcnt == 1)	/* and it's a real file */
	      crc16 = 0L;		/* Clear the file CRC */
	    resetc();			/* reset per-transaction counters */
	    rtimer();			/* reset timers */
#ifdef GFTIMER
	    rftimer();
#endif /* GFTIMER */
	    streamon();			/* turn on streaming */
#ifdef IKSDB
	    if (ikdbopen) slotstate(what,
				  (server ? "SERVER" : ""),
				  "SEND",
				  filnam
				  );
#endif /* IKSDB */
	    BEGIN ssfile;		/* and switch to receive-file state */
	} else {			/* otherwise send error msg & quit */
	    s = xflg ? "Can't execute command" : (char *)epktmsg;
	    if (!*s) s = "Can't open file";
	    errpkt((CHAR *)s);
	    RESUME;
	}
#ifdef CK_RESEND
    }
#endif /* CK_RESEND */
}

/*
  These states are necessary to handle the case where we get a server command
  packet (R, G, or C) reply with an S packet, but the client retransmits the
  command packet.  The input() function doesn't catch this because the packet
  number is still zero.
*/
<ssinit>R {				/* R packet was retransmitted. */
    xsinit();				/* Resend packet 0 */
}

<ssinit>G {				/* Same deal if G packet comes again */
    xsinit();
}

/* should probably add cases for O, W, V, H, J, ... */

<ssinit>C {				/* Same deal if C packet comes again */
    xsinit();
}

<ssfile>Y {				/* ACK for F or X packet */
    srvptr = srvcmd;			/* Point to string buffer */
    decode(rdatap,putsrv,0);		/* Decode data field, if any */
    putsrv(NUL);			/* Terminate with null */
    ffc = 0L;				/* Reset file byte counter */
    debug(F101,"<ssfile>Y cxseen","",cxseen);
    if (*srvcmd) {			/* If remote name was recorded */
        if (sendmode != SM_RESEND) {
	    if (fdispla == XYFD_C || fdispla == XYFD_S)
	      xxscreen(SCR_AN,0,0L,(char *)srvcmd);
	    tlog(F110," remote name:",(char *) srvcmd,0L);
	    makestr(&psrfspec,(char *)srvcmd);
        }
    }
    if (cxseen||czseen) {		/* Interrupted? */
	debug(F101,"<ssfile>Y canceling","",0);
	x = clsif();			/* Close input file */
	sxeof(1);			/* Send EOF(D) */
	BEGIN sseof;			/* and switch to EOF state. */
    } else if (atcapu) {		/* If attributes are to be used */
	if (sattr(xflg | stdinf, 1) < 0) { /* send them */
	    errpkt((CHAR *)"Can't send attributes"); /* if problem, say so */
	    RESUME;			/* and quit */
	} else BEGIN ssattr;		/* if ok, switch to attribute state */
    } else {				/* Attributes not negotiated */
	if (window(wslotn) < 0) {	/* Open window */
	    errpkt((CHAR *)"Can't open window");
	    RESUME;
	} else if ((x = sdata()) == -2) { /* Send first data packet data */
	    window(1);			/* Connection lost, reset window */
	    x = clsif();		/* Close input file */
	    return(success = 0);	/* Return failure */
	} else if (x == -9) {		/* User interrupted */
	    errpkt((CHAR *)"User cancelled"); /* Send Error packet */
	    window(1);			/* Set window size back to 1... */
	    timint = s_timint;		/* Restore timeout */
	    return(success = 0);	/* Failed */
	} else if (x < 0) {		/* EOF (empty file) or interrupted */
	    window(1);			/* put window size back to 1, */
	    debug(F101,"<ssfile>Y cxseen","",cxseen);
	    x = clsif();		/* If not ok, close input file, */
	    if (x < 0)			/* treating failure as interruption */
	      cxseen = 1;		/* Send EOF packet */
	    seof(cxseen||czseen);
	    BEGIN sseof;		/* and switch to EOF state. */
	} else {			/* First data sent OK */
	    BEGIN ssdata;		/* All ok, switch to send-data state */
	}
    }
}

<ssattr>Y {				/* Got ACK to A packet */
    ffc = 0L;				/* Reset file byte counter */
    debug(F101,"<ssattr>Y cxseen","",cxseen);
    if (cxseen||czseen) {		/* Interrupted? */
	debug(F101,"<sattr>Y canceling","",0);
	x = clsif();			/* Close input file */
	sxeof(1);			/* Send EOF(D) */
	BEGIN sseof;			/* and switch to EOF state. */
    } else if (rsattr(rdatap) < 0) {	/* Was the file refused? */
	discard = 1;			/* Set the discard flag */
	clsif();			/* Close the file */
	sxeof(1);			/* send EOF with "discard" code */
	BEGIN sseof;			/* switch to send-EOF state */
    } else if ((x = sattr(xflg | stdinf, 0)) < 0) { /* Send more? */
	errpkt((CHAR *)"Can't send attributes"); /* Trouble... */
	RESUME;
    } else if (x == 0) {		/* No more to send so now the data */
	if (window(wslotn) < 0) {	/* Allocate negotiated window slots */
	    errpkt((CHAR *)"Can't open window");
	    RESUME;
	}
	if ((x = sdata()) == -2) {	/* File accepted, send first data */
	    window(1);			/* Connection broken */
	    x = clsif();		/* Close file */
	    return(success = 0);	/* Return failure */
	} else if (x == -9) {		/* User interrupted */
	    errpkt((CHAR *)"User cancelled"); /* Send Error packet */
	    window(1);			/* Set window size back to 1... */
	    timint = s_timint;		/* Restore timeout */
	    return(success = 0);	/* Failed */
	} else if (x < 0) {		/* If data was not sent */
	    window(1);			/* put window size back to 1, */
	    debug(F101,"<ssattr>Y cxseen","",cxseen);
	    if (clsif() < 0)		/* Close input file */
	      cxseen = 1;		/* Send EOF packet */
	    seof(cxseen||czseen);
	    BEGIN sseof;		/* and switch to EOF state. */
	} else {
	    BEGIN ssdata;		/* All ok, switch to send-data state */
	}
    }
}

<ssdata>q {  				/* Ctrl-C or connection loss. */
    window(1);				/* Set window size back to 1... */
    cxseen = 1;				/* To indicate interruption */
    x = clsif();			/* Close file */
    return(success = 0);		/* Failed */
}

<ssdata>Y {				/* Got ACK to Data packet */
    canned(rdatap);			/* Check if file transfer cancelled */
    debug(F111,"<ssdata>Y cxseen",rdatap,cxseen);
    debug(F111,"<ssdata>Y czseen",rdatap,czseen);
    if ((x = sdata()) == -2) {		/* Try to send next data */
	window(1);			/* Connection lost, reset window */
	x = clsif();			/* Close file */
	return(success = 0);		/* Failed */
    } else if (x == -9) {		/* User interrupted */
	errpkt((CHAR *)"User cancelled"); /* Send Error packet */
	window(1);			/* Set window size back to 1... */
	timint = s_timint;		/* Restore original timeout */
	return(success = 0);		/* Failed */
    } else if (x < 0) {			/* EOF - finished sending data */
	debug(F101,"<ssdata>Y cxseen","",cxseen);
	window(1);			/* Set window size back to 1... */
	if (clsif() < 0)		/* Close input file */
	  cxseen = 1;			/* Send EOF packet */
	debug(F101,"<ssdata>Y CALLING SEOF()","",cxseen);
	seof(cxseen||czseen);
	BEGIN sseof;			/* and enter send-eof state */
    }
    /* NOTE: If x == 0 it means we're draining: see sdata()! */
}

<sseof>Y {				/* Got ACK to EOF */
    int g, xdiscard;
    canned(rdatap);			/* Check if file transfer cancelled */
    debug(F111,"<sseof>Y cxseen",rdatap,cxseen);
    debug(F111,"<sseof>Y czseen",rdatap,czseen);
    debug(F111,"<sseof>Y discard",rdatap,discard);
    xdiscard = discard;
    discard = 0;
    success = (cxseen == 0 && czseen == 0); /* Transfer status... */
    debug(F101,"<sseof>Y success","",success);
    if (success && rejection > 0)	    /* If rejected, succeed if */
      if (rejection != '#' &&		    /* reason was date */
	  rejection != 1 && rejection != '?') /* or name; */
	success = 0;			    /* fail otherwise. */
    cxseen = 0;				/* This goes back to zero. */
    if (success) {			/* Only if transfer succeeded... */
	xxscreen(SCR_ST,ST_OK,0L,"");
	if (!xdiscard) {
	    makestr(&sfspec,psfspec);	/* Record filenames for WHERE */
	    makestr(&srfspec,psrfspec);
	}
	if (moving) {			/* If MOVE'ing */
	    x = zdelet(filnam);		/* Try to delete the source file */
#ifdef TLOG
	    if (tralog) {
		if (x > -1) {
		    tlog(F110," deleted",filnam,0);
		} else {
		    tlog(F110," delete failed:",ck_errstr(),0);
		}
	    }
#endif /* TLOG */
	} else if (snd_move) {		/* Or move it */
	    int x;
	    x = zrename(filnam,snd_move);
#ifdef TLOG
	    if (tralog) {
		if (x > -1) {
		    tlog(F110," moved to ",snd_move,0);
		} else {
		    tlog(F110," move failed:",ck_errstr(),0);
		}
	    }
#endif /* TLOG */
	} else if (snd_rename) {	/* Or rename it */
	    char *s = snd_rename;	/* Renaming string */
#ifndef NOSPL
	    int y;			/* Pass it thru the evaluator */
	    extern int cmd_quoting;	/* for \v(filename) */
	    if (cmd_quoting) {		/* But only if cmd_quoting is on */
		y = MAXRP;
		s = (char *)srvcmd;
		zzstring(snd_rename,&s,&y);
		s = (char *)srvcmd;
	    }
#endif /* NOSPL */
	    if (s) if (*s) {
		int x;
		x = zrename(filnam,s);
#ifdef TLOG
	    if (tralog) {
		if (x > -1) {
		    tlog(F110," renamed to",s,0);
		} else {
		    tlog(F110," rename failed:",ck_errstr(),0);
		}
	    }
#endif /* TLOG */
#ifdef COMMENT
		*s = NUL;
#endif /* COMMENT */
	    }
	}
    }
    if (czseen) {			/* Check group interruption flag */
	g = 0;				/* No more files if interrupted */
    } else {				/* Otherwise... */
#ifdef COMMENT
	/* This code makes any open error fatal to a file-group transfer. */
	g = gnfile();
	debug(F111,"<sseof>Y gnfile",filnam,g);
	if (g > 0) {			/* Any more files to send? */
	    if (sfile(xflg))		/* Yes, try to send next file header */
	      BEGIN ssfile;		/* if ok, enter send-file state */
	    else {			/* otherwise */
		s = xflg ? "Can't execute command" : (char *)epktmsg;
		if (!*s) s = "Can't open file";
		errpkt((CHAR *)s);	/* send error message */
		RESUME;			/* and quit */
	    }
	} else {			/* No next file */
	    tsecs = gtimer();		/* get statistics timers */
#ifdef GFTIMER
	    fptsecs = gftimer();
#endif /* GFTIMER */
	    seot();			/* send EOT packet */
	    BEGIN sseot;		/* enter send-eot state */
	}
#else  /* COMMENT */
	while (1) {			/* Keep trying... */
	    g = gnfile();		/* Get next file */
	    debug(F111,"<sseof>Y gnfile",filnam,g);
	    if (g == 0 && gnferror == 0) /* No more, stop trying */
	      break;
	    if (g > 0) {		/* Have one */
		if (sfile(xflg)) {	/* Try to open and send F packet */
		    BEGIN ssfile;	/* If OK, enter send-file state */
		    break;		/* and break out of loop. */
		}
	    } /* Otherwise keep trying to get one we can send... */
	}
    }
    if (g == 0) {
	debug(F101,"<sseof>Y no more files","",czseen);
	tsecs = gtimer();		/* Get statistics timers */
#ifdef GFTIMER
	fptsecs = gftimer();
#endif /* GFTIMER */
	seot();				/* Send EOT packet */
	BEGIN sseot;			/* Enter send-eot state */
    }
#endif /* COMMENT */
}

<sseot>Y {				/* Got ACK to EOT */
    debug(F101,"sseot justone","",justone);
    RESUME;				/* All done, just quit */
}

E {					/* Got Error packet, in any state */
    char *s = "";
    window(1);				/* Close window */
    timint = s_timint;			/* Restore original timeout */
    if (*epktmsg)			/* Message from Error packet */
      s = (char *)epktmsg;
    if (!*s) {				/* If not there then maybe here */
	s = (char *)rdatap;
	ckstrncpy((char *)epktmsg,(char *)rdatap,PKTMSGLEN);
    }
    if (!*s)				/* Hopefully we'll never see this. */
      s = "Unknown error";
    success = 0;			/* For IF SUCCESS/FAIL. */
    debug(F101,"ckcpro.w justone at E pkt","",justone);

    success = 0;			/* Transfer failed */
    xferstat = success;			/* Remember transfer status */
    if (!epktsent) {
	x = quiet; quiet = 1;		/* Close files silently, */
	epktrcvd = 1;			/* Prevent messages from clsof() */
	clsif();
	clsof(1); 			/* discarding any output file. */
	ermsg(s);			/* Issue the message (calls screen). */
	quiet = x;			/* Restore quiet state */
    }
    tstats();				/* Get stats */
/*
  If we are executing commands from a command file or macro, let the command
  file or macro decide whether to exit, based on SET { TAKE, MACRO } ERROR.
*/
    if (
#ifndef NOICP
	!xcmdsrc &&
#endif /* NOICP */
	backgrd && !server)
      fatal("Protocol error");
    xitsta |= (what & W_KERMIT);	/* Save this for doexit(). */
#ifdef CK_TMPDIR
/* If we were cd'd temporarily to another device or directory ... */
    if (f_tmpdir) {
	int x;
	x = zchdir((char *) savdir);	/* ... restore previous directory */
	f_tmpdir = 0;			/* and remember we did it. */
	debug(F111,"ckcpro.w E tmpdir restored",savdir,x);
    }
#endif /* CK_TMPDIR */
#ifdef IKSDB
    if (ikdbopen) slotstate(what,"ERROR", (char *)epktmsg, "");
#endif /* IKSDB */
    RESUME;
}

q { success = 0; QUIT; }		/* Ctrl-C or connection loss. */

. {					/* Anything not accounted for above */
    errpkt((CHAR *)"Unexpected packet type"); /* Give error message */
    window(1);
    xitsta |= (what & W_KERMIT);	/* Save this for doexit(). */
    RESUME;				/* and quit */
}

%%

/*
  From here down to proto() are routines that were moved out of the state
  table switcher because the resulting switch() had become too large.
  To move the contents of a state-table case to a routine:
    1. Add a prototype to the list above the state table switcher.
    2. Make a routine with an appropriate name, returning int.
    3. Move the code into it.
    4. Put a call to the new routine in the former spot:
         rc = name_of_routine();
         if (rc > -1) return(rc);
    5. Add "return(-1);" after every RESUME, SERVE, or BEGIN macro and
       at the end if the code is open-ended.
*/
static int
rcv_firstdata() {
    extern int dispos;
    debug(F101,"rcv_firstdata","",dispos);

    if (discard) {			/* if we're discarding the file */
	ack1((CHAR *)"X");		/* just ack the data like this. */
	cancel++;			/* and count it */
	BEGIN rdpkt;			/* and wait for more data packets. */
	return(-1);
    } else {				/* Not discarding. */
	rf_err = "Can't open file";
	if (xflg) {			/* If screen data */
	    if (remfile) {		/* redirected to file */
		if (rempipe)		/* or pipe */
		  x = openc(ZOFILE,remdest); /* Pipe: start command */
		else
		  x = opena(remdest,&iattr); /* File: open with attributes */
	    } else {			/* otherwise */
		x = opent(&iattr);	/* "open" the screen */
	    }
	} else {			/* otherwise */
#ifdef CALIBRATE
	    if (calibrate) {		/* If calibration run */
		x = ckopenx(&iattr);	/* open nothing */
#ifdef STREAMING
		if (streaming)		/* Streaming */
		  fastack();		/* ACK without ACKing. */
		else
#endif /* STREAMING */
		  ack();		/* Send real ACK */
		BEGIN rdpkt;		/* Proceed to next state */
		return(-1);
	    } else
#endif /* CALIBRATE */
#ifdef UNIX
/*
  In UNIX we can pipe the file data into the mail program, which is to be
  preferred to writing it out to a temp file and then mailing it afterwards.
  This depends rather heavily on all UNIXes having a mail command that
  accepts '-s "subject"' on the command line.  MAILCMD (e.g. mail, Mail, mailx)
  is defined in ckufio.c.
*/
	    if (dispos == 'M') {	/* Mail... */
		char *s;
		char * tmp = NULL;
		int n = 0;
		extern char *MAILCMD;
		s = iattr.disp.val + 1;
		n = (int)strlen(MAILCMD) +    /* Mail command */
		  (int)strlen(s) +	      /* address */
		  (int)strlen(ofilnam) + 32;  /* subject */
		if (tmp = (char *)malloc(n)) {
		    ckmakxmsg(tmp,n,
			      MAILCMD," -s \"",ofilnam,"\" ",s,
			      NULL,NULL,NULL,NULL,NULL,NULL,NULL);
		    debug(F111,"rcv_firsdata mail",tmp,(int)strlen(tmp));
		    x = openc(ZOFILE,(char *)tmp);
		    free(tmp);
		} else
		  x = 0;
	    } else if (dispos == 'P') { /* Ditto for print */
		char * tmp = NULL;
		int n;
		extern char *PRINTCMD;
		n = (int)strlen(PRINTCMD) + (int)strlen(iattr.disp.val+1) + 4;
		if (tmp = (char *)malloc(n)) {
		    sprintf(tmp,	/* safe (prechecked) */
			    "%s %s", PRINTCMD, iattr.disp.val + 1);
		    x = openc(ZOFILE,(char *)tmp);
		    free(tmp);
		} else
		  x = 0;
	    } else
#endif /* UNIX */
	      x = opena(filnam,&iattr);	/* open the file, with attributes */
	}
	if (x) {			/* If file was opened ok */
	    int rc, qf;
#ifndef NOSPL
	    qf = query;
#else
	    qf = 0;
#endif /* NOSPL */

#ifdef CKTUNING
	    rc = (binary && !parity) ?
	      bdecode(rdatap,putfil):
	       decode(rdatap, qf ? puttrm : putfil, 1);
#else
	    rc = decode(rdatap, qf ? puttrm : putfil, 1);
#endif /* CKTUNING */
	    if (rc < 0) {
		errpkt((CHAR *)"Error writing data");
		RESUME;
		return(-1);
	    }
#ifdef STREAMING
	    if (streaming)		/* Streaming was negotiated */
	      fastack();		/* ACK without ACKing. */
	    else
#endif /* STREAMING */
	      ack();			/* acknowledge it */
	    BEGIN rdpkt;		/* and switch to receive-data state */
	    return(-1);
	} else {			/* otherwise */
	    errpkt((CHAR *) rf_err);	/* send error packet */
    	    RESUME;			/* and quit. */
	    return(-1);
	}
    }
}

static int
rcv_shortreply() {
#ifdef PKTZEROHACK
    success = 0;
    debug(F111,"rcv_shortreply",rdatap,ipktlen);
    if (ipktack[0] && !strncmp(ipktack,(char *)rdatap,ipktlen)) {
	/* No it's the ACK to the I packet again */
	x = scmd(vcmd,(CHAR *)cmarg);	/* So send the REMOTE command again */
	/* Maybe this should be resend() */
	debug(F110,"IPKTZEROHACK",ipktack,x);
	if (x < 0) {
	    errpkt((CHAR *)srimsg);
	    RESUME;
	    return(-1);
	}
    } else {
	ipktack[0] = NUL;
#endif /* PKTZEROHACK */
	urserver = 1;
#ifndef NOSERVER
#ifndef NOSPL
	if (query) {			/* If to query, */
	    qbufp = querybuf;		/*  initialize query response buffer */
	    qbufn = 0;
	    querybuf[0] = NUL;
	}
#endif /* NOSPL */
	x = 1;
	if (remfile) {			/* Response redirected to file */
	    rf_err = "Can't open file";
	    if (rempipe)		/* or pipe */
	      x =
#ifndef NOPUSH
		zxcmd(ZOFILE,remdest)	/* Pipe: Start command */
#else
		0
#endif /* NOPUSH */
		;
	    else
	      x = opena(remdest,&iattr); /* File: Open with attributes */
	    debug(F111,"rcv_shortreply remfile",remdest,x);
	} else {
	    x = opent(&iattr);		/* "open" the screen */
	}
	if (x) {			/* If file was opened ok */
	    if (decode(rdatap,
#ifndef NOSPL
		       (query || !remfile) ? puttrm :
#else
		       !remfile ? puttrm :
#endif /* NOSPL */
		       zputfil, 1) < 0) { /* Note: zputfil, not putfil. */
		errpkt((CHAR *)"Error writing data");
		RESUME;
		return(-1);
	    } else {
		if (rdatap)		/* If we had data */
		  if (*rdatap)		/* add a line terminator */
		    if (remfile) {	/* to file */
			zsoutl(ZOFILE,"");
		    } else {		/* or to screen. */
#ifndef NOICP
			if (!query || !xcmdsrc)
#endif /* NOICP */
			  if (!(quiet && rcdactive))
			    conoll("");
		    }
		if (bye_active && network) { /* I sent BYE or REMOTE LOGOUT */
		    msleep(500);	/* command and got the ACK... */
		    bye_active = 0;
		    ttclos(0);
		}
		clsof(0);
		if (!epktsent && !epktrcvd) /* If no error packet... */
		  success = 1;		/* success. */
		RESUME;
		return(-1);
	    }
	} else {			/* File not opened OK */
	    errpkt((CHAR *) rf_err);	/* send error message */
	    RESUME;			/* and quit. */
	    return(-1);
	}
#endif /* NOSERVER */
#ifdef PKTZEROHACK
    }
#endif /* PKTZEROHACK */
    debug(F101,"rcv_shortreply fallthru","",success);
    return(-1);
}


static int
srv_query() {
#ifndef NOSERVER
#ifndef NOSPL
    char c;
#ifdef CKSYSLOG
    if (ckxsyslog >= SYSLG_PR && ckxlogging)
      cksyslog(SYSLG_PR, 1, "server", "REMOTE QUERY", (char *)srvcmd);
#endif /* CKSYSLOG */
#ifdef IKSDB
    if (ikdbopen) slotstate(what,"REMOTE QUERY", (char *)(srvcmd+2), "");
#endif /* IKSDB */
    c = *(srvcmd+2);			/* Q = Query, S = Set */
    if (c == 'Q') {			/* Query */
	if (!ENABLED(en_que)) { /* Security */
	    errpkt((CHAR *)"REMOTE QUERY disabled");
	    RESUME;
	    return(-1);
	} else {			/* Query allowed */
	    int n; char *p, *q;
	    qbufp = querybuf;		/* Wipe out old stuff */
	    qbufn = 0;
	    querybuf[0] = NUL;
	    p = (char *) srvcmd + 3;	/* Pointer for making wrapper */
	    n = strlen((char *)srvcmd);	/* Position of end */
	    c = *(srvcmd+4);		/* Which type of variable */

	    if (*(srvcmd+6) == CMDQ) {	/* Starts with command quote? */
		p = (char *) srvcmd + 6; /* Take it literally */
		if (*p == CMDQ) p++;
	    } else {			/* They played by the rules */
		if (c == 'K') {		/* Kermit variable */
		    int k;
		    k = (int) strlen(p);
		    if (k > 0 && p[k-1] == ')') {
			p = (char *)(srvcmd + 4);
			*(srvcmd+4) = CMDQ;
			*(srvcmd+5) = 'f'; /* Function, so make it \f...() */
		    } else {
			*(srvcmd+3) = CMDQ; /* Stuff wrapping into buffer */
			*(srvcmd+4) = 'v';  /* Variable, so make it \v(...) */
			*(srvcmd+5) = '(';  /* around variable name */
			*(srvcmd+n) = ')';
			*(srvcmd+n+1) = NUL;
		    }
		} else {
		    *(srvcmd+3) = CMDQ; /* Stuff wrapping into buffer */
		    *(srvcmd+4) = 'v'; /*  Variable, so make it \v(...) */
		    *(srvcmd+5) = '(';	/* around variable name */
		    *(srvcmd+n) = ')';
		    *(srvcmd+n+1) = NUL;
		    if (c == 'S') {	/* System variable */
			*(srvcmd+4) = '$'; /*  so it's \$(...) */
		    } else if (c == 'G') { /* Non-\ Global variable */
			*(srvcmd+4) = 'm'; /*  so wrap it in \m(...) */
		    }
		}
	    }				/* Now evaluate it */
	    n = QBUFL;			/* Max length */
	    q = querybuf;		/* Where to put it */
	    if (zzstring(p,&q,&n) < 0) {
		errpkt((n > 0) ? (CHAR *)"Can't get value"
		               : (CHAR *)"Value too long"
		       );
		RESUME;
		return(-1);
	    } else {
		if (encstr((CHAR *)querybuf) > -1) { /* Encode it */
		    ack1(data);		/* If it fits, send it back in ACK */
		    success = 1;
		    RESUME;
		    return(-1);
		} else if (sndstring(querybuf)) { /* Long form response */
		    BEGIN ssinit;
		    return(-1);
		} else {		/* sndhlp() fails */
		    errpkt((CHAR *)"Can't send value");
		    RESUME;
		    return(-1);
		}
	    }
	}
    } else if (c == 'S') {		/* Set (assign) */
	if (!ENABLED(en_asg)) {		/* Security */
	    errpkt((CHAR *)"REMOTE ASSIGN disabled");
	    RESUME;
	    return(-1);
	} else {			/* OK */
	    int n;
	    n = xunchar(*(srvcmd+3));	/* Length of name */
	    n = 3 + n + 1;		/* Position of length of value */
	    *(srvcmd+n) = NUL;		/* Don't need it */
	    if (addmac((char *)(srvcmd+4),(char *)(srvcmd+n+1)) < 0)
	      errpkt((CHAR *)"REMOTE ASSIGN failed");
	    else {
		ack();
		success = 1;
	    }
	    RESUME;
	    return(-1);
	}
    } else {
	errpkt((CHAR *)"Badly formed server command");
	RESUME;
	return(-1);
    }
#else
    errpkt((CHAR *)"Variable query/set not available");
    RESUME;
    return(-1);
#endif /* NOSPL */
#endif /* NOSERVER */
}

static int
srv_copy() {
#ifndef NOSERVER
#ifdef CKSYSLOG
    if (ckxsyslog >= SYSLG_PR && ckxlogging)
      cksyslog(SYSLG_PR, 1, "server", "REMOTE COPY", (char *)srvcmd);
#endif /* CKSYSLOG */
#ifdef ZCOPY
    if (!ENABLED(en_cpy)) {
	errpkt((CHAR *)"REMOTE COPY disabled");
	RESUME;
	return(-1);
    } else {
	char *str1, *str2, f1[256], f2[256];
	int  len1, len2;
        len1 = xunchar(srvcmd[1]);	/* Separate the parameters */
        len2 = xunchar(srvcmd[2+len1]);
        strncpy(f1,(char *)(srvcmd+2),len1);
        f1[len1] = NUL;
        strncpy(f2,(char *)(srvcmd+3+len1),len2);
        f2[len2] = NUL;
#ifdef IKSDB
	if (ikdbopen) slotstate(what,"REMOTE COPY", f1, f2);
#endif /* IKSDB */
	if (!ENABLED(en_cwd)) {		/* If CWD is disabled */
	    zstrip(f1,&str1);		/* and they included a pathname, */
            zstrip(f2,&str2);
	    if (strcmp(f1,str1) || strcmp(f2,str2)) { /* Refuse. */
		errpkt((CHAR *)"Access denied");
		RESUME;			/* Remember, this is not a goto! */
		return(-1);
	    }
	}
	if (state == generic) {		/* It's OK to go ahead. */
            if (zcopy(f1,f2)) {		/* Try */
		errpkt((CHAR *)"Can't copy file"); /* give error message */
	    } else {
		success = 1;
		ack();
	    }
            RESUME;			/* wait for next server command */
	    return(-1);
	}
    }
    return(-1);
#else /* no ZCOPY */
    errpkt((CHAR *)"REMOTE COPY not available"); /* give error message */
    RESUME;				/* wait for next server command */
    return(-1);
#endif /* ZCOPY */
#endif /* NOSERVER */
}

static int
srv_rename() {
#ifndef NOSERVER
#ifdef CKSYSLOG
    if (ckxsyslog >= SYSLG_PR && ckxlogging)
      cksyslog(SYSLG_PR, 1, "server", "REMOTE RENAME", (char *)srvcmd);
#endif /* CKSYSLOG */
#ifdef ZRENAME
    if (!ENABLED(en_ren)) {
	errpkt((CHAR *)"REMOTE RENAME disabled");
	RESUME;
	return(-1);
    } else {				/* RENAME is enabled */
	char *str1, *str2, f1[256], f2[256];
	int len1, len2;
	len1 = xunchar(srvcmd[1]);	/* Separate the parameters */
	len2 = xunchar(srvcmd[2+len1]);
	strncpy(f1,(char *)(srvcmd+2),len1);
	f1[len1] = NUL;
	strncpy(f2,(char *)(srvcmd+3+len1),len2);
	f2[len2] = NUL;
	len2 = xunchar(srvcmd[2+len1]);
	strncpy(f1,(char *)(srvcmd+2),len1);
	f1[len1] = NUL;
	strncpy(f2,(char *)(srvcmd+3+len1),len2);
	f2[len2] = NUL;
#ifdef IKSDB
	if (ikdbopen) slotstate(what,"REMOTE RENAME", f1, f2);
#endif /* IKSDB */
	if (!ENABLED(en_cwd)) {		/* If CWD is disabled */
	    zstrip(f1,&str1);		/* and they included a pathname, */
	    zstrip(f2,&str2);
	    if ( strcmp(f1,str1) || strcmp(f2,str2) ) { /* refuse. */
		errpkt((CHAR *)"Access denied");
		RESUME;			/* Remember, this is not a goto! */
		return(-1);
	    }
	}
	if (state == generic) {		/* It's OK to go ahead. */
	    if (zrename(f1,f2)) {	/* Try */
		errpkt((CHAR *)"Can't rename file"); /* Give error msg */
	    } else {
		success = 1;
		ack();
	    }
	    RESUME;			/* Wait for next server command */
	    return(-1);
	}
    }
    return(-1);
#else /* no ZRENAME */
    /* Give error message */
    errpkt((CHAR *)"REMOTE RENAME not available");
    RESUME;				/* Wait for next server command */
    return(-1);
#endif /* ZRENAME */
#endif /* NOSERVER */
}

static int
srv_login() {
#ifndef NOSERVER
    char f1[LOGINLEN+1], f2[LOGINLEN+1], f3[LOGINLEN+1];
    CHAR *p;
    int len, i;

    debug(F101,"REMOTE LOGIN x_login","",x_login);
    debug(F101,"REMOTE LOGIN x_logged","",x_logged);

    f1[0] = NUL; f2[0] = NUL; f3[0] = NUL;
    len = 0;
    if (srvcmd[1])			/* First length field */
      len = xunchar(srvcmd[1]);		/* Separate the parameters */

    if (x_login) {			/* Login required */
	if (x_logged) {			/* And already logged in */
	    if (len > 0) {		/* Logging in again */
		errpkt((CHAR *)"Already logged in.");
	    } else {			/* Logging out */
		debug(F101,"REMOTE LOGOUT","",x_logged);
#ifdef CKSYSLOG
		if (ckxsyslog >= SYSLG_PR && ckxlogging)
		  cksyslog(SYSLG_PR, 1, "server", "REMOTE LOGOUT", NULL);
#endif /* CKSYSLOG */
#ifdef IKSDB
		if (ikdbopen) slotstate(what,"REMOTE LOGOUT", "", "");
#endif /* IKSDB */
		tlog(F110,"Logged out",x_user,0);
		ack1((CHAR *)"Logged out");
		success = 1;
		msleep(500);
#ifdef CK_LOGIN
		x_logged = 0;
#ifdef IKSD
		if (inserver)
		  ckxlogout();
#endif /* IKSD */
#endif /* CK_LOGIN */
	    }
	} else {			/* Not logged in yet */
	    debug(F101,"REMOTE LOGIN len","",len);
	    if (len > 0) {		/* Have username */
#ifdef CKSYSLOG
		if (ckxsyslog >= SYSLG_PR && ckxlogging)
		  cksyslog(SYSLG_PR, 1, "server", "REMOTE LOGIN", NULL);
#endif /* CKSYSLOG */
		if (len > LOGINLEN) {
		    errpkt((CHAR *)"Username too long");
		}
		p = srvcmd + 2;		/* Point to it */
		for (i = 0; i < len; i++) /* Copy it */
		  f1[i] = p[i];
		f1[len] = NUL;		/* Terminate it */
		p += len;		/* Point to next length field */
		if (*p) {		/* If we have one */
		    len = xunchar(*p++); /* decode it */
		    if (len > 0 && len <= LOGINLEN) {
			for (i = 0; i < len; i++) /* Same deal for password */
			  f2[i] = p[i];
			f2[len] = NUL;
			p += len;	/* And account */
			if (*p) {
			    len = xunchar(*p++);
			    if (len > 0 && len <= LOGINLEN) {
				for (i = 0; i < len; i++)
				  f3[i] = p[i];	/* Set but never used */
				f3[len] = NUL; /* (because account not used) */
			    }
			}
		    }
		}
		debug(F101,"REMOTE LOGIN 1","",x_logged);
#ifdef IKSD
#ifdef CK_LOGIN
		if (inserver) {		/* Log in to system for real */
		    x_logged = ckxlogin((CHAR *)f1,(CHAR *)f2,NULL,0);
		    debug(F101,"REMOTE LOGIN 2","",x_logged);
		    if (x_logged) {	/* Count attempts */
			logtries = 0;
			justone = 1;
		    } else {
			logtries++;
			sleep(logtries);
		    }
		} else
#endif /* CK_LOGIN */
#endif /* IKSD */
		  if (x_user && x_passwd) { /* User and password must match */
		      if (!strcmp(x_user,f1)) /* SET SERVER LOGIN */
			if (!strcmp(x_passwd,f2))
			  x_logged = 1;
		      debug(F101,"REMOTE LOGIN 3","",x_logged);
		  } else if (x_user) {	/* Only username given, no password */
		      if (!strcmp(x_user,f1)) /* so only username must match */
			x_logged = 1;
		      debug(F101,"REMOTE LOGIN 4","",x_logged);
		  }
#ifdef CK_LOGIN 
                else {
		    x_logged = ckxlogin((CHAR *)f1,(CHAR *)f2,NULL,0);
		    debug(F101,"REMOTE LOGIN 5","",x_logged);
                }
#endif /* CK_LOGIN */
		if (x_logged) {		/* Logged in? */
		    tlog(F110,"Logged in", x_user, 0);
		    if (isguest)
		      ack1((CHAR *)"Logged in as guest - restrictions apply");
		    else
		      ack1((CHAR *)"Logged in");
		    success = 1;
		} else {
		    tlog(F110,"Login failed", f1, 0);
		    errpkt((CHAR *)"Access denied.");
#ifdef IKSD
#ifdef CK_LOGIN
		    if (inserver && logtries > 2)
		      ckxlogout();
#endif /* CK_LOGIN */
#endif /* IKSD */
		}
	    } else {			/* LOGOUT */
		errpkt((CHAR *)"Logout ignored");
	    }
	}
    } else {				/* Login not required */
	if (len > 0)
	  errpkt((CHAR *)"Login ignored.");
	else
	  errpkt((CHAR *)"Logout ignored.");
    }
#endif /* NOSERVER */
    RESUME;
    return(-1);
}

static int
srv_timeout() {
    /* K95 does this its own way */
    if (idletmo) {
#ifdef IKSD
        if (inserver) {
           printf("\r\nIKSD IDLE TIMEOUT: %d sec\r\n", srvidl);
           doexit(GOOD_EXIT,xitsta);
        }
#endif /* IKSD */
	idletmo = 0;
	printf("\r\nSERVER IDLE TIMEOUT: %d sec\r\n", srvidl);
	xitsta |= (what & W_KERMIT);
	QUIT;
    }
#ifndef NOSERVER
    else if (fatalio) {			/* Connection lost */
#ifdef CKSYSLOG
	  if (ckxsyslog >= SYSLG_PR && ckxlogging)
	    cksyslog(SYSLG_PR, 1, "server", "Connection lost", NULL);
#endif /* CKSYSLOG */
#ifdef IKSDB
	  if (ikdbopen) slotstate(what,"SERVER DISCONNECT",(char *)srvcmd, "");
#endif /* IKSDB */
	xitsta |= what;
	QUIT;
    } else if (interrupted) {		/* Interrupted by hand */
	if (!ENABLED(en_fin)) {
	    errpkt((CHAR *)"QUIT disabled");
	    RESUME;
	    return(-1);
	} else {
	    if (what == W_SEND || what == W_RECV || what == W_REMO) {
		success = 0;
#ifdef CKSYSLOG
		if (ckxsyslog >= SYSLG_PR && ckxlogging)
		  cksyslog(SYSLG_PR, 1, "server", "Interrupted", NULL);
#endif /* CKSYSLOG */
	    } else if (what == W_NOTHING && filcnt == 0) {
		success = 1;
	    } /* Otherwise leave success alone */
	    xitsta |= (what & W_KERMIT);
	    QUIT;
	}
    } else {				/* Shouldn't happen */
	debug(F100,"SERVER (top) GOT UNEXPECTED 'q'","",0);
	QUIT;
    }
#endif /* NOSERVER */
}

static int
rcv_s_pkt() {
#ifndef NOSERVER
    if (state == rgen)
      urserver = 1;
    if (/* state == serve && */ x_login && !x_logged) {
	errpkt((CHAR *)"Login required");
	SERVE;
    } else
#endif /* NOSERVER */
      if (state == serve && !ENABLED(en_sen)) { /* Not in server mode */
	errpkt((CHAR *)"SEND disabled"); /* when SEND is disabled. */
	RESUME;
	return(-1);
    } else {				/* OK to go ahead. */
#ifdef CK_TMPDIR
	if (dldir && !f_tmpdir) {	/* If they have a download directory */
	    debug(F110,"receive download dir",dldir,0);
	    if (s = zgtdir()) {		/* Get current directory */
		debug(F110,"receive current dir",s,0);
		if (zchdir(dldir)) {	/* Change to download directory */
		    debug(F100,"receive zchdir ok","",0);
		    ckstrncpy(savdir,s,TMPDIRLEN);
		    f_tmpdir = 1;	/* Remember that we did this */
		} else
		  debug(F100,"receive zchdir failed","",0);
	    }
	}
#endif /* CK_TMPDIR */
	nakstate = 1;			/* Can send NAKs from here. */
	rinit(rdatap);			/* Set parameters */
	if (bctf) {
	    bctu = 3;
	    bctl = 3;
	} else {
	    bctu = bctr;	       /* switch to agreed-upon block check */
	    bctl = (bctu == 4) ? 2 : bctu; /* Set block-check length */
	}
	what = W_RECV;			/* Remember we're receiving */
	lastxfer = W_RECV;
	resetc();			/* Reset counters */
	rtimer();			/* Reset timer */
#ifdef GFTIMER
	rftimer();
#endif /* GFTIMER */
	streamon();
	BEGIN rfile;			/* Go into receive-file state */
    }
    return(-1);
}


/* END OF ROUTINES MOVED OUT OF STATE MACHINE */


/*  P R O T O  --  Protocol entry function  */

static int is_tn = 0;			/* It's a Telnet connection */

#ifdef CK_SPEED
int f_ctlp = 0;				/* Control-character prefix table */
#ifdef COMMENT
short s_ctlp[256];
#endif /* COMMENT */
#endif /* CK_SPEED */

/*
  This is simply a wrapper for the real protocol function just below,
  that saves any items that might be changed automatically by protocol
  negotiations and then restores them upon exit from protocol mode.
*/
VOID
proto() {
    extern int b_save, f_save, c_save, ss_save, slostart, reliable, urclear;
#ifndef NOCSETS
    extern int fcharset, fcs_save, tcharset, tcs_save;
#endif /* NOCSETS */

#ifdef PIPESEND
    extern int pipesend;
#endif /* PIPESEND */
#ifndef NOLOCAL
#ifdef OS2
    extern int cursorena[], cursor_save, term_io;
    extern BYTE vmode;
    extern int display_demo;
    int term_io_save;
#endif /* OS2 */
#endif /* NOLOCAL */
#ifdef TNCODE
    int _u_bin=0, _me_bin = 0;
#ifdef IKS_OPTION
    int /* _u_start=0, */ _me_start = 0;
#endif /* IKS_OPTION */
#endif /* TNCODE */
#ifdef PATTERNS
    int pa_save;
    int i;
#endif /* PATTERNS */
    int scan_save;

#ifdef PATTERNS
    pa_save = patterns;
#endif /* PATTERNS */
    scan_save = filepeek;

    myjob = sstate;

#ifdef CK_LOGIN
    if (isguest) {			/* If user is anonymous */
	en_pri = 0;			/* disable printing */
	en_mai = 0;			/* and disable email */
	en_del = 0;			/* and file deletion */
    }
#endif /* CK_LOGIN */

#ifndef NOLOCAL
#ifdef OS2
    cursor_save = cursorena[vmode];
    cursorena[vmode] = 0;
    term_io_save = term_io;
    term_io = 0;
#endif /* OS2 */
#endif /* NOLOCAL */
    b_save = binary;			/* SET FILE TYPE */
    f_save = fncnv;			/* SET FILE NAMES */
    c_save = bctr;
    p_save = fnspath;
    r_save = recursive;
    s_timint = timint;
    ss_save = slostart;
#ifndef NOCSETS
    fcs_save = fcharset;
    tcs_save = tcharset;
#endif /* NOCSETS */

#ifdef COMMENT
/* Don't do this because then user can never find out what happened. */
#ifdef CK_SPEED
    for (i = 0; i < 256; i++)
      s_ctlp[i] = ctlp[i];
    f_ctlp = 1;
#endif /* CK_SPEED */
#endif /* COMMENT */
    if (reliable == SET_ON)
      slostart = 0;
    is_tn = (!local && sstelnet)
#ifdef TNCODE
      || (local && network && ttnproto == NP_TELNET)
#endif /* TNCODE */
	;
#ifdef TNCODE
    if (is_tn) {
        if (tn_b_xfer && !(sstelnet || inserver)) {
	    /* Save the current state of Telnet Binary */
	    _u_bin = TELOPT_U(TELOPT_BINARY);
	    _me_bin = TELOPT_ME(TELOPT_BINARY);

	    /* If either direction is not Binary attempt to negotiate it */
	    if (!_u_bin && TELOPT_U_MODE(TELOPT_BINARY) != TN_NG_RF) {
		tn_sopt(DO,TELOPT_BINARY);
		TELOPT_UNANSWERED_DO(TELOPT_BINARY) = 1;
	    }
	    if (!_me_bin && TELOPT_ME_MODE(TELOPT_BINARY) != TN_NG_RF) {
		tn_sopt(WILL,TELOPT_BINARY);
		TELOPT_UNANSWERED_WILL(TELOPT_BINARY) = 1;
	    }
	    if (!(_me_bin && _u_bin))
	      tn_wait("proto set binary mode");
        }
#ifdef IKS_OPTION
#ifdef CK_XYZ
        if (protocol != PROTO_K) {	/* Non-Kermit protocol selected */
            if (TELOPT_U(TELOPT_KERMIT) &&
                TELOPT_SB(TELOPT_KERMIT).kermit.u_start) {
                iks_wait(KERMIT_REQ_STOP,0); /* Stop the other Server */
		/* _u_start = 1; */
            }
            if (TELOPT_ME(TELOPT_KERMIT) &&
                TELOPT_SB(TELOPT_KERMIT).kermit.me_start) {
                tn_siks(KERMIT_STOP);	/* I'm not servering */
	 	TELOPT_SB(TELOPT_KERMIT).kermit.me_start = 0;
		_me_start = 1;
            }
        } else
#endif /* CK_XYZ */
        if (sstate == 'x' || sstate == 'v') { /* Responding to a request */
            if (!inserver && TELOPT_U(TELOPT_KERMIT) &&
                TELOPT_SB(TELOPT_KERMIT).kermit.u_start) {
                iks_wait(KERMIT_REQ_STOP,0); /* Stop the other Server */
		/* _u_start = 1; */
            }
            if (TELOPT_ME(TELOPT_KERMIT) &&
                !TELOPT_SB(TELOPT_KERMIT).kermit.me_start) {
                tn_siks(KERMIT_START);	/* Send Kermit-Server Start */
	 	TELOPT_SB(TELOPT_KERMIT).kermit.me_start = 1;
            }
        } else {			/* Initiating a request */
            if (TELOPT_ME(TELOPT_KERMIT) &&
                TELOPT_SB(TELOPT_KERMIT).kermit.me_start) {
                tn_siks(KERMIT_STOP);	/* I'm not servering */
	 	TELOPT_SB(TELOPT_KERMIT).kermit.me_start = 0;
		_me_start = 1;
            }
            if (TELOPT_U(TELOPT_KERMIT) &&
	        !TELOPT_SB(TELOPT_KERMIT).kermit.u_start) {
		/* Send Req-Server-Start */
                if (!iks_wait(KERMIT_REQ_START,0)) {
                    if (sstate != 's') {
			success = 0;	/* Other Kermit refused to serve */
			if (local)
			  printf("A Kermit Server is not available\r\n");
			debug(F110,"proto()",
                             "A Kermit Server is not available",0);
			tlog(F110,"IKS client/server failure",
                             "A Kermit Server is not available",0);
			goto xxprotox;
                    }
		}
            }
        }
#endif /* IKS_OPTION */
#ifdef CK_ENCRYPTION
        if (tn_no_encrypt_xfer && !(sstelnet || inserver)) {
            ck_tn_enc_stop();
        }
#endif /* CK_ENCRYPTION */
    }
#endif /* TNCODE */

    if (!xfrint) connoi();
    xxproto();				/* Call the real protocol function */

#ifdef IKS_OPTION
  xxprotox:
#endif /* IKS_OPTION */
    xferstat = success;			/* Remember transfer status */
    kactive = 0;

#ifdef TNCODE
#ifdef CK_ENCRYPTION
        if (tn_no_encrypt_xfer && !(sstelnet || inserver)) {
            ck_tn_enc_start();
        }
#endif /* CK_ENCRYPTION */
#ifdef IKS_OPTION
    if (TELOPT_ME(TELOPT_KERMIT) &&
        TELOPT_SB(TELOPT_KERMIT).kermit.me_start && !_me_start) {
        tn_siks(KERMIT_STOP);		/* Server is stopped */
 	TELOPT_SB(TELOPT_KERMIT).kermit.me_start = 0;
    }
#endif /* IKS_OPTION */
    if (is_tn && tn_b_xfer && !(sstelnet || inserver)) {
        /* if we negotiated Binary mode try to reset it */
        if (!_u_bin) {
            /* Check to see if the state changed during the transfer */
	    if (TELOPT_U(TELOPT_BINARY)) {
		tn_sopt(DONT,TELOPT_BINARY);
		TELOPT_UNANSWERED_DONT(TELOPT_BINARY) = 1;
	    } else
	      _u_bin = 1;		/* So we don't call tn_wait() */
        }
        if (!_me_bin) {
            /* Check to see if the state changed during the transfer */
	    if (TELOPT_ME(TELOPT_BINARY)) {
		tn_sopt(WONT,TELOPT_BINARY);
		TELOPT_UNANSWERED_WONT(TELOPT_BINARY) = 1;
	    } else
	      _me_bin = 1;		/* So we don't call tn_wait() */
	}
	if (!(_me_bin && _u_bin))
	  tn_wait("proto reset binary mode");
    }
#endif /* TNCODE */

#ifdef PATTERNS
    patterns = pa_save;
#endif /* PATTERNS */
    filepeek = scan_save;

#ifdef STREAMING
    streaming = 0;
    /* streamok = 0; */
#endif /* STREAMING */
#ifdef COMMENT
#ifdef CK_SPEED
    for (i = 0; i < 256; i++)
      ctlp[i] = s_ctlp[i];
    f_ctlp = 0;
#endif /* CK_SPEED */
#endif /* COMMENT */
    urclear = 0;
    if (!success) {
	xitsta |= (what & W_KERMIT);
	tlog(F110," failed:",(char *)epktmsg,0);
    }
    debug(F111,"proto xferstat",epktmsg,xferstat);
    slostart = ss_save;
    if (s_timint > -1) {		/* Because of REMOTE SET */
	timint = s_timint;
	s_timint = -1;
    }
    recursive = r_save;
    fnspath = p_save;
    if (c_save > -1) {			/* Because of REMOTE SET */
	bctr = c_save;
	c_save = -1;
    }
    fncnv   = f_save;
    binary  = b_save;
#ifdef PIPESEND
    pipesend = 0;    			/* Next time might not be pipesend */
#endif /* PIPESEND */
#ifndef NOLOCAL
#ifdef OS2
    cursorena[vmode] = cursor_save;
    term_io = term_io_save;
    display_demo = 1;
#endif /* OS2 */
#endif /* NOLOCAL */
}

static VOID
xxproto() {
    int x;
    long lx;
#ifdef CK_XYZ
#ifdef XYZ_INTERNAL
_PROTOTYP( int pxyz, (int) );
#endif /* XYZ_INTERNAL */
#endif /* CK_XYZ */

    char xss[2];			/* String representation of sstate */
    xss[0] = sstate;
    xss[1] = NUL;
    s_timint = timint;

    debug(F101,"xxproto entry justone","",justone);
    success = 0;

    retrieve = 0;			/* Reset these ... */
    reget = 0;
    opkt = 0;

    if (local && ttchk() < 0) {		/* Giving BYE or FIN */
	if (bye_active) {		/* but there is no connection */
	    ttclos(0);
	    success = 1;
	    return;
	}
	/* Ditto for any REMOTE command */
	if (sstate == 'g' && cmarg ) {
	    if (*cmarg == 'L' || *cmarg == 'F' || *cmarg == 'X')
	      success = 1;
	    else
	      printf("?No connection\r\n");
	    return;
	}
    }

/* Set up the communication line for file transfer. */
/* NOTE: All of the xxscreen() calls prior to the wart() invocation */
/* could just as easily be printf's or, for that matter, hints. */

    if (local && (speed < 0L) && (network == 0)) {
	xxscreen(SCR_EM,0,0L,"Sorry, you must 'set speed' first");
	return;
    }
    x = -1;
    if (ttopen(ttname,&x,mdmtyp,cdtimo) < 0) {
	debug(F111,"failed: proto ttopen local",ttname,local);
	xxscreen(SCR_EM,0,0L,"Can't open line");
	return;
    }
    if (x > -1) local = x;
    debug(F111,"proto ttopen local",ttname,local);

    lx = (local && !network) ? speed : -1;
#ifdef NETCONN
#ifdef CK_SPEED
    if (is_tn) {
	ctlp[(unsigned)255] = ctlp[CR] = 1;
	if (parity == 'e' || parity == 'm') ctlp[127] = 1;
	if (flow == FLO_XONX) {		/* Also watch out for Xon/Xoff */
	    ctlp[17] = ctlp[19] = 1;
	    ctlp[17+128] = ctlp[19+128] = 1;
	}
    }
#endif /* CK_SPEED */
#endif /* NETCONN */
    if (ttpkt(lx,flow,parity) < 0) {	/* Put line in packet mode, */
	xxscreen(SCR_EM,0,0L,"Can't condition line");
	return;
    }
    if (local && !network && carrier != CAR_OFF) {
	int x;				/* Serial connection */
	x = ttgmdm();			/* with carrier checking */
	if (x > -1) {
	    if (!(x & BM_DCD)) {
		debug(F101,"proto ttgmdm","",0);
		xxscreen(SCR_EM,0,0L,"Carrier required but not detected");
		return;
	    }
	}
    }
    /* Send remote side's "receive" or "server" startup string, if any */
    if (local && ckindex((char *)xss,"srgcjhk",0,0,1)) {
	char *s = NULL;
        if (
#ifdef IKS_OPTION
	    /* Don't send auto-blah string if we know other side is serving */
	    !TELOPT_U(TELOPT_KERMIT) ||
	    !TELOPT_SB(TELOPT_KERMIT).kermit.u_start
#else
	    1
#endif /* IKS_OPTION */
	    ) {
	    if (sstate == 's') {	/* Sending file(s) */
		s = binary ? ptab[protocol].h_b_init : ptab[protocol].h_t_init;
	    } else if (protocol == PROTO_K) { /* Command for server */
		s = ptab[protocol].h_x_init;
	    }
	}
#ifdef CK_SPEED
#ifndef UNPREFIXZERO
	if (protocol == PROTO_K)	/* Because of C-strings... */
	  ctlp[0] = 1;
#endif /* UNPREFIXZERO */
#endif /* CK_SPEED */
	if (s) if (*s) {		/* If we have a command to send... */
	    char tmpbuf[356];
	    int tmpbufsiz = 356;
	    int stuff = -1, stuff2 = -1, len = 0;
	    extern int tnlm;
	    if (sstate == 's') {	/* Sending file(s) */
#ifdef CK_XYZ
		if (protocol == PROTO_X) {
		    char * s2;
		    s2 = cmarg2[0] ? cmarg2 : cmarg;
		    if ((int)strlen(s) + (int)strlen(s2) + 4 < 356)
		      sprintf(tmpbuf, s, s2);
		    else
		      tmpbuf[0] = NUL;
		} else {
#endif /* CK_XYZ */
		    ckmakmsg(tmpbuf, 356, s, NULL, NULL, NULL);
#ifdef CK_XYZ
		}
#endif /* CK_XYZ */
	    } else {			/* Command for server */
		ckstrncpy(tmpbuf,s,356);
	    }
	    ckstrncat(tmpbuf, "\015",sizeof(tmpbuf));
	    if (tnlm)			/* TERMINAL NEWLINE ON */
	      stuff = LF;		/* Stuff LF */
#ifdef TNCODE
	    /* TELNET NEWLINE MODE */
	    if (is_tn) {
		switch (TELOPT_ME(TELOPT_BINARY) ? tn_b_nlm : tn_nlm) {
		  case TNL_CR:
		    break;
		  case TNL_CRNUL:
		    break;
		  case TNL_CRLF:
		    stuff2 = stuff;
		    stuff = LF;
		    break;
		}
	    }
#endif /* TNCODE */

#ifdef NETCONN
#ifdef TCPSOCKET
#ifdef RLOGCODE
	    if (network && ttnproto == NP_RLOGIN) {
		switch (tn_b_nlm) { /* Always BINARY */
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
#endif /* RLOGCODE */
#endif /* TCPSOCKET */
#endif /* NETCONN */

	    len = strlen(tmpbuf);
	    if (stuff >= 0 && len < tmpbufsiz - 1) {
		tmpbuf[len++] = stuff;
		if (stuff2 >= 0 && len < tmpbufsiz - 1)
		  tmpbuf[len++] = stuff2;
		tmpbuf[len] = NUL;
	    }
	    ttol((CHAR *)tmpbuf,len);
	    if (protocol == PROTO_K)	/* Give remote Kermit time to start */
	      msleep(400);
	}
    }

#ifdef CK_XYZ
    if (protocol != PROTO_K) {		/* Non-Kermit protocol selected */
	char tmpbuf[356];
	int tmpbufsiz = 356;
	char * s = "";

#ifdef CK_TMPDIR
	if (sstate == 'v') {		/* If receiving and... */
	    if (dldir && !f_tmpdir) {	/* if they have a download directory */
		if (s = zgtdir()) {	/* Get current directory */
		    if (zchdir(dldir)) { /* Change to download directory */
			ckstrncpy(savdir,s,TMPDIRLEN);
			f_tmpdir = 1;	/* Remember that we did this */
		    }
		}
	    }
	}
#endif /* CK_TMPDIR */

#ifdef XYZ_INTERNAL			/* Internal */
	success = !pxyz(sstate);
#else
#ifdef CK_REDIR				/* External */
	switch (sstate) {
	  case 's':			/* 'Tis better to SEND... */
	    s = binary ? ptab[protocol].p_b_scmd : ptab[protocol].p_t_scmd;
	    break;
	  case 'v':			/* ... than RECEIVE */
	    s = binary ? ptab[protocol].p_b_rcmd : ptab[protocol].p_t_rcmd;
	    break;
	}
	if (!s) s = "";
	if (*s) {
	    if (sstate == 's') {	/* Sending */
		extern int xfermode;
		int k = 0, x = 0, b = binary;
		/*
		  If just one file we can scan it to set the xfer mode.
		  Otherwise it's up to the external protocol program.
		*/
		if (patterns && xfermode == XMODE_A && !iswild(fspec)) {
		    extern int nscanfile;
		    k = scanfile(fspec,&x,nscanfile);
		    if (k > -1) {
			b = (k == FT_BIN) ? XYFT_B : XYFT_T;
			s = b ?
			    ptab[protocol].p_b_scmd :
			    ptab[protocol].p_t_scmd;
		    }
		}
		if ((int)strlen(s) + (int)strlen(fspec) < tmpbufsiz) {
		    sprintf(tmpbuf,s,fspec); /* safe (prechecked) */
		    tlog(F110,"Sending",fspec,0L);
		}
	    } else {			/* Receiving */
		if ((int)strlen(s) + (int)strlen(cmarg2) < tmpbufsiz) {
		    sprintf(tmpbuf,s,cmarg2); /* safe (prechecked) */
		    tlog(F110,"Receiving",cmarg2,0L);
		}
	    }
	    tlog(F110," via external protocol:",tmpbuf,0);
	    debug(F110,"ckcpro ttruncmd",tmpbuf,0);
	    success = ttruncmd(tmpbuf);
	    tlog(F110," status:",success ? "OK" : "FAILED", 0);
	} else {
	    printf("?Sorry, no external protocol defined for %s\r\n",
		   ptab[protocol].p_name
		   );
	}
#else
	printf(
"Sorry, only Kermit protocol is supported in this version of Kermit\n"
	       );
#endif /* CK_REDIR */
#endif /* XYZ_INTERNAL */
	return;
    }
#endif /* CK_XYZ */

#ifdef NTSIGX
    conraw();
    connoi();
#else
    if (!local)
      connoi();				/* No console interrupts if remote */
#endif /* NTSIG */

    kactive = 1;
    if (sstate == 'x') {		/* If entering server mode, */
	extern int howcalled;
	server = 1;			/* set flag, */
	debug(F101,"server backgrd","",backgrd);
	debug(F101,"server quiet","",quiet);
	debug(F100,"SHOULD NOT SEE THIS IF IN BACKGROUND!","",0);
	if (howcalled == I_AM_SSHSUB) {	/* and issue appropriate message. */
	    ttol((CHAR *)"KERMIT READY TO SERVE...\015\012",26);
	} else if (!local) {
	    if (!quiet && !backgrd
#ifdef IKS_OPTION
                && !TELOPT_ME(TELOPT_KERMIT) /* User was told by negotiation */
#endif /* IKS_OPTION */
		) {
		conoll(srvtxt);
		conoll("KERMIT READY TO SERVE...");
	    }
	} else {
	    conol("Entering server mode on ");
	    conoll(ttname);
	    conoll("Type Ctrl-C to quit.");
	    if (srvdis) intmsg(-1L);
#ifdef TCPSOCKET
#ifndef NOLISTEN
	    if (network && tcpsrfd > 0)
	      ttol((CHAR *)"KERMIT READY TO SERVE...\015\012",26);
#endif /* NOLISTEN */
#endif /* TCPSOCKET */
	}
    } else
      server = 0;
#ifdef VMS
    if (!quiet && !backgrd)    /* So message doesn't overwrite prompt */
      conoll("");
    if (local) conres();       /* So Ctrl-C will work */
#endif /* VMS */
/*
  If in remote mode, not shushed, not in background, and at top command level,
  issue a helpful message telling what to do...
*/
    if (!local && !quiet && !backgrd) {
	if (sstate == 'v') {
	    conoll("Return to your local Kermit and give a SEND command.");
	    conoll("");
	    conoll("KERMIT READY TO RECEIVE...");
	} else if (sstate == 's') {
	    conoll("Return to your local Kermit and give a RECEIVE command.");
	    conoll("");
	    conoll("KERMIT READY TO SEND...");
	} else if ( sstate == 'g' || sstate == 'r' || sstate == 'h' ||
		    sstate == 'j' || sstate == 'c' ) {
	    conoll("Return to your local Kermit and give a SERVER command.");
	    conoll("");
	    conoll((sstate == 'r' || sstate == 'j' || sstate == 'h') ?
		   "KERMIT READY TO GET..." :
		   "KERMIT READY TO SEND SERVER COMMAND...");
	}
    }
#ifdef COMMENT
    if (!local) sleep(1);
#endif /* COMMENT */
/*
  The 'wart()' function is generated by the wart program.  It gets a
  character from the input() routine and then based on that character and
  the current state, selects the appropriate action, according to the state
  table above, which is transformed by the wart program into a big case
  statement.  The function is active for one transaction.
*/
    rtimer();				/* Reset elapsed-time timer */
#ifdef GFTIMER
    rftimer();
#endif /* GFTIMER */
    resetc();				/* & other per-transaction counters. */

    debug(F101,"proto calling wart, justone","",justone);

    wart();				/* Enter the state table switcher. */
/*
  Note: the following is necessary in case we have just done a remote-mode
  file transfer, in which case the controlling terminal modes have been
  changed by ttpkt().  In particular, special characters like Ctrl-C and
  Ctrl-\ might have been turned off (see ttpkt).  So this call to ttres() is
  essential.  IMPORTANT: restore interrupt handlers first, otherwise any
  terminal interrupts that occur before this is done in the normal place
  later will cause a crash.
*/
#ifdef OS2
    ttres();				/* Reset the communication device */
#else
    if (!local) {
	setint();			/* Arm interrupt handlers FIRST */
	msleep(500);
	ttres();			/* Then restore terminal. */
    }
#endif /* OS2 */
    xxscreen(SCR_TC,0,0L,"");		/* Transaction complete */
    x = quiet;
    quiet=1;
    clsif();				/* Failsafe in case we missed */
    clsof(1);				/* a case in the state machine. */
    quiet = x;

    if (server) {			/* Back from packet protocol. */
    	if (!quiet && !backgrd
#ifdef IKSD
	    && !inserver
#endif /* IKSD */
	    ) {				/* Give appropriate message */
	    conoll("");
	    conoll("C-Kermit server done");
        }
        server = 0;			/* Not a server any more */
    }
}

/*  S G E T I N I T  --  Handle incoming GET-Class packets  */

/*
  Returns:
   -1: On error
    0: GET packet processed OK - ready to Send.
    1: Extended GET processed OK - wait for another.
*/
static int
sgetinit(reget,xget) int reget, xget; {	/* Server end of GET command */
    char * fs = NULL;			/* Pointer to filespec */
    int i, n, done = 0;
#ifdef PIPESEND
    extern int usepipes, pipesend;
#endif /* PIPESEND */
    extern int nolinks;

    if (!ENABLED(en_get)) {		/* Only if not disabled!  */
	errpkt((CHAR *)"GET disabled");
	return(-1);
    }

    /* OK to proceed */

    nolinks = recursive;
    filcnt = 0;

#ifdef WHATAMI
    /* If they are alike this was already done in whoarewe() */
    debug(F101,"sgetinit whatru","",whatru);
    if (whatru & WMI_FLAG) {		/* Did we get WHATAMI info? */
	debug(F101,"sgetinit binary (1)","",binary);
#ifdef VMS
	if (binary != XYFT_I && binary != XYFT_L)
#else
#ifdef OS2
	  if (binary != XYFT_L)
#endif /* OS2 */
#endif /* VMS */
	    binary = (whatru & WMI_FMODE) ? /* Yes, set file type */
	      XYFT_B : XYFT_T;	/* automatically */
	debug(F101,"sgetinit binary (2)","",binary);
	if (!wearealike)
	  fncnv = (whatru & WMI_FNAME) ? 1 : 0; /* And name conversion */
    }
#endif /* WHATAMI */

    fs = (char *)srvcmd;
    srvptr = srvcmd;			/* Point to server command buffer */
    decode(rdatap,putsrv,0);		/* Decode the GET command into it */
    /* Accept multiple filespecs */
    cmarg2 = "";			/* Don't use cmarg2 */
    cmarg = "";				/* Don't use cmarg */

    done = 1;				/* Only 1 packet needed... */
    if (xget) {				/* Special decoding for Extended GET */
	char L, next, c;		/* PLV items */
	int len, val;			/* More PLV items */
	char * p = (char *)srvcmd;	/* String to decode */

	done = 0;			/* Maybe more packets needed */
	fs = NULL;			/* We don't know the filespec yet */
	c = *p++;			/* Get first parameter */

	while (c) {			/* For all parameters... */
	    debug(F000,"sgetinit c","",c);
	    L = *p++;			/* Get length */
	    if (L >= SP)		/* Decode length */
	      len = xunchar(L);
	    else if (c == '@') {	/* Allow missing EOP length field */
		len = 0;
	    } else {
		len = (xunchar(*p++) * 95);
		len += xunchar(*p++);
	    }
	    debug(F101,"sgetinit len","",len);
	    next = *(p+len);		/* Get next parameter */
	    *(p+len) = NUL;		/* Zero it out to terminal value */
	    debug(F110,"sgetinit p",p,0);
	    switch (c) {		/* Do the parameter */
	      case 'O':			/* GET Options */
		val = atoi(p);		/* Convert to int */
		debug(F101,"sgetinit O val","",val);
		if (val & GOPT_DEL) moving = 1;
		if (val & GOPT_RES) reget = 1;
		if (val & GOPT_REC) {
		    recursive = 1;
		    nolinks = 2;
		    if (fnspath == PATH_OFF)
		      fnspath = PATH_REL;
		}
		break;
	      case 'M':			/* Transfer Mode */
		val = atoi(p);
		debug(F101,"sgetinit M val","",val);
		if (val < 1)
		  break;
		patterns = 0;		/* Takes precedence over patterns */
		filepeek = 0;		/* and FILE SCAN */
		if (val == GMOD_TXT) binary = XYFT_T; /* Text */
		if (val == GMOD_BIN) binary = XYFT_B; /* Binary */
		if (val == GMOD_LBL) binary = XYFT_L; /* Labeled */
		break;
	      case 'F':			/* Filename */
		fs = p;
		debug(F110,"sgetinit filename",fs,0);
		break;
	      case '@':			/* End Of Parameters */
		done = 1;
		debug(F100,"sgetinit EOP","",0);
		break;
	      default:
		errpkt((CHAR *)"Unknown GET Parameter");
		debug(F100,"sgetinit unknown parameter","",0);
		return(-1);
	    }
	    p += (len + 1);
	    c = next;
	}
    }
    if (!fs) fs = "";			/* A filename is required */
    if (*fs) {
	havefs = 1;
	n = 0;				/* Check for quoted name */
	if ((n = strlen(fs)) > 1) {
	    /* Note: this does not allow for multiple quoted names */
	    if ((fs[0] == '{' && fs[n-1] == '}') ||
		(fs[0] == '"' && fs[n-1] == '"')) {
		fs[n-1] = '\0';
		fs++;
		debug(F111,"sgetinit unquoted filename",fs,n);
	    } else
	      n = 0;			/* This means no quoting */
	}

#ifdef PIPESEND
	debug(F111,"sgetinit",fs,usepipes);
	if (usepipes && ENABLED(en_hos) && *fs == '!') {
	    cmarg = fs + 1;		/* Point past the bang */
	    *fs = NUL;
	    nfils = -1;
	    pipesend = 1;
	    debug(F111,"sgetinit pipesend",cmarg,pipesend);
	}
	if (!pipesend) {		/* If it's not a pipe */
#endif /* PIPESEND */
	    if (n == 0) {		/* If the name was not quoted */
#ifndef NOMSEND
		nfils = fnparse(fs);	/* Allow it to be a list of names */
		debug(F111,"sgetinit A",fs,nfils);
#ifdef COMMENT
/* This doesn't work if a GET-PATH is set. */
		if (nfils == 1 && !iswild(fs)) { /* Single file */
		    char * m;
		    if ((x = zchki(fs)) < 0) { /* Check if it's sendable */
			switch (x) {
			  case -1: m = "File not found"; break;
			  case -2: m = "Not a regular file"; break;
			  case -3: m = "Read access denied"; break;
			}
			errpkt((CHAR *)m);
			return(-1);
		    }
		}
#endif /* COMMENT */
	    } else {			/* If it was quoted */
#endif /* NOMSEND */
		nzxopts = 0;
#ifdef UNIXOROSK
		if (matchdot)  nzxopts |= ZX_MATCHDOT;
#endif /* UNIXOROSK */
		if (recursive) nzxopts |= ZX_RECURSE;
		/* Treat as a single filespec */
		nfils = 0 - nzxpand(fs,nzxopts);
		debug(F111,"sgetinit B",fs,nfils);
		cmarg = fs;
	    }
#ifdef PIPESEND
	}
#endif /* PIPESEND */
    }
    if (!done) {			/* Need more O packets... */
	debug(F100,"sgetinit O-Packet TBC","",0); /* To Be Continued */
	return(1);
    }
    debug(F100,"sgetinit O-Packet done - havefs","",havefs);
    if (!havefs) {			/* Done - make sure we have filename */
	errpkt((CHAR *)"GET without filename");
	return(-1);
    }
    freerpkt(winlo);
    winlo = 0;				/* Back to packet 0 again. */
    debug(F101,"sgetinit winlo","",winlo);
    nakstate = 0;			/* Now I'm the sender! */
    if (reget) sendmode = SM_RESEND;
    if (sinit() > 0) {			/* Send Send-Init */
#ifdef STREAMING
	if (!streaming)
#endif /* STREAMING */
	  timint = chktimo(rtimo,timef); /* Switch to per-packet timer */
	return(0);			/* If successful, switch state */
    } else return(-1);			/* Else back to server command wait */
}

#else  /* NOXFER */

#include "ckcdeb.h"

VOID
proto() {
    extern int success;
    success = 0;
}
#endif /* NOXFER */

#include "ckcsym.h"
char *connv = "CONNECT Command for UNIX:select(), 9.0.139, 1 Mar 2010";

/*  C K U C N S  --  Terminal connection to remote system, for UNIX  */
/*
  Author: Frank da Cruz <fdc@columbia.edu>,
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 2010,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/

/*
  This version of the UNIX CONNECT module uses select(), which is required for
  Kerberos encryption.  Thus it can be used only on UNIX systems that support
  select() on both TCP/IP and serial connections.  A separate module that uses
  a completely portable fork() structure can be used on systems where select()
  is not available or does not work as required.
*/

#include "ckcdeb.h"			/* Common things first */

#ifndef NOLOCAL

#ifdef OSF13
#ifdef CK_ANSIC
#ifdef _NO_PROTO
#undef _NO_PROTO
#endif /* _NO_PROTO */
#endif /* CK_ANSIC */
#endif /* OSF13 */

#include <errno.h>			/* Error numbers */

#ifndef NOTIMEH
#include <time.h>			/* For FD_blah */
#ifdef SYSTIMEH				/* (IRIX 5.3) */
#include <sys/time.h>
#endif /* SYSTIMEH */
#endif /* NOTIMEH */

#ifdef BSD42HACK			/* Why is this necessary? */
#ifndef DCLTIMEVAL
#define DCLTIMEVAL
#endif /* DCLTIMEVAL */
#endif /* BSD42HACK */

/* Kermit-specific includes */

#include "ckcasc.h"			/* ASCII characters */
#include "ckcker.h"			/* Kermit things */
#include "ckucmd.h"			/* For xxesc() prototype */
#include "ckcnet.h"			/* Network symbols */
#ifndef NOCSETS
#include "ckcxla.h"			/* Character set translation */
#endif /* NOCSETS */

#ifdef BEBOX
#include <kernel/OS.h>
#include <socket.h>
#include <stdio.h>
#endif /* BEBOX */

#include <signal.h>			/* Signals */

/* All the following is for select()... */

#ifdef CKTIDLE				/* Timeouts only for SET TERM IDLE */

#ifndef DCLTIMEVAL
#ifdef UNIXWARE
#ifndef UW7
#define DCLTIMEVAL
#endif /* UW7 */
#endif /* UNIXWARE */
#endif /* DCLTIMEVAL */

#ifdef DCLTIMEVAL			/* Declare timeval ourselves */
struct timeval {
    long tv_sec;
    long tv_usec;
};
#else  /* !DCLTIMEVAL */
#ifndef NOSYSTIMEBH
#ifdef SYSTIMEBH
#include <sys/timeb.h>
#endif /* SYSTIMEBH */
#endif /* NOSYSTIMEBH */
#endif /* DCLTIMEVAL */
#endif /* CKTIDLE */

#ifndef SCO_OSR504
#ifdef SELECT_H
#include <sys/select.h>
#endif /* SELECT_H */
#endif /* SCO_OSR504 */

#ifndef FD_SETSIZE
#ifdef CK_FORWARD_X
#define FD_SETSIZE 256
#else
#define FD_SETSIZE 32
#endif /* CK_FORWARD_X */
#endif /* FD_SETSIZE */

#ifdef HPUX
#ifndef HPUX10
#ifndef HPUX1100
/* The three interior args to select() are (int *) rather than (fd_set *) */
#ifndef INTSELECT
#define INTSELECT
#endif /* INTSELECT */
#endif /* HPUX1100 */
#endif /* HPUX10 */
#endif /* HPUX */

/* Internal function prototypes */

#ifdef NEWFTP
#endif /* NEWFTP */
_PROTOTYP( VOID ttflux, (void) );
_PROTOTYP( VOID doesc, (char) );
_PROTOTYP( int hconne, (void) );
#ifndef NOSHOW
_PROTOTYP( VOID shomdm, (void) );
#endif /* NOSHOW */
_PROTOTYP( static int kbget, (void) );
_PROTOTYP( static int ckcputf, (void) );

/* External variables */

extern struct ck_p ptab[];

extern int local, escape, duplex, parity, flow, seslog, sessft, debses,
 mdmtyp, ttnproto, cmask, cmdmsk, network, nettype, sosi, tnlm,
 xitsta, what, ttyfd, ttpipe, quiet, backgrd, pflag, tt_crd, tt_lfd,
 tn_nlm, ttfdflg,
 tt_escape, justone, carrier, ttpty, hwparity;

#ifndef NODIAL
extern int dialmhu, dialsta;
#endif /* NODIAL */

#ifdef CKLEARN
extern FILE * learnfp;
extern int learning;
static ULONG learnt1;
static char learnbuf[LEARNBUFSIZ] = { NUL, NUL };
static int  learnbc = 0;
static int  learnbp = 0;
static int  learnst = 0;
#endif /* CKLEARN */

extern long speed;
extern char ttname[], sesfil[], myhost[], *ccntab[];
#ifdef TNCODE
extern int tn_b_nlm, tn_rem_echo;
#endif /* TNCODE */

#ifdef CK_TRIGGER
extern char * tt_trigger[], * triggerval;
#endif /* CK_TRIGGER */

#ifdef CKTIDLE
extern int tt_idlelimit, tt_idleact;
extern char * tt_idlestr;
static int idlelimit = 0;
#endif /* CKTIDLE */
extern int cx_status;			/* CONNECT status code */

extern int nopush;

#ifdef CK_APC
extern int apcactive;			/* Application Program Command (APC) */
extern int apcstatus;			/* items ... */
static int apclength = 0;
#ifdef DCMDBUF
extern char *apcbuf;
#else
extern char apcbuf[];
#endif /* DCMDBUF */
static int apcbuflen = APCBUFLEN - 2;
extern int protocol;
#endif /* CK_APC */
#ifndef NOXFER
extern int autodl;			/* Auto download */
#endif /* NOXFER */

#ifdef CK_AUTODL
extern CHAR ksbuf[];
extern CHAR stchr;
extern int kstartactive;
#endif /* CK_AUTODL */

#ifdef CK_ENCRYPTION
extern int me_auth;
#endif /* CK_ENCRYPTION */

#ifdef CK_XYZ
#ifdef XYZ_INTERNAL
static int zmdlok = 1;			/* Zmodem autodownloads available */
#else
static int zmdlok = 0;			/* Depends on external protocol def */
#endif /* XYZ_INTERNAL */
#else
static int zmdlok = 0;			/* Not available at all */
#endif /* CK_XYZ */

#ifndef NOSETKEY			/* Keyboard mapping */
extern KEY *keymap;			/* Single-character key map */
extern MACRO *macrotab;			/* Key macro pointer table */
static MACRO kmptr = NULL;		/* Pointer to current key macro */
#endif /* NOSETKEY */

/* Global variables local to this module */

static int
  active = 0,
  quitnow = 0,				/* <esc-char>Q was typed */
  dohangup = 0,				/* <esc-char>H was typed */
  inshift = 0,				/* SO/SI shift states */
  outshift = 0;

static char ecbuf[10], *ecbp;		/* Escape char buffer & pointer */

#ifdef CK_SMALL
#define IBUFL 1536			/* Input buffer length */
#else
#define IBUFL 4096
#endif /* CK_SMALL */

static int obc = 0;			/* Output buffer count */

#ifndef OXOS
#define OBUFL 1024			/* Output buffer length */
#else
#define OBUFL IBUFL
#endif /* OXOS */

#ifdef BIGBUFOK
#define TMPLEN 4096			/* Temporary message buffer length */
#else
#define TMPLEN 200
#endif /* BIGBUFOK */

#ifdef DYNAMIC
static char *ibuf = NULL, *obuf = NULL, *temp = NULL; /* Buffers */
#else
static char ibuf[IBUFL], obuf[OBUFL], temp[TMPLEN];
#endif /* DYNAMIC */

#ifdef TNCODE
static char tnopt[4];
#endif /* TNCODE */

#ifdef DYNAMIC
static char *ibp;			/* Input buffer pointer */
#else
static char *ibp = ibuf;		/* Input buffer pointer */
#endif /*DYNAMIC */
static int ibc = 0;			/* Input buffer count */

#ifdef DYNAMIC
static char *obp;			/* Output buffer pointer */
#else
static char *obp = obuf;		/* Output buffer pointer */
#endif /* DYNAMIC */

/* Character-set items */

static int unicode = 0;

#ifndef NOCSETS
#ifdef CK_ANSIC /* ANSI C prototypes... */
extern CHAR (*xls[MAXTCSETS+1][MAXFCSETS+1])(CHAR); /* Character set */
extern CHAR (*xlr[MAXTCSETS+1][MAXFCSETS+1])(CHAR); /* translation functions */
static CHAR (*sxo)(CHAR);	/* Local translation functions */
static CHAR (*rxo)(CHAR);	/* for output (sending) terminal chars */
static CHAR (*sxi)(CHAR);	/* and for input (receiving) terminal chars. */
static CHAR (*rxi)(CHAR);
#else /* Not ANSI C... */
extern CHAR (*xls[MAXTCSETS+1][MAXFCSETS+1])();	/* Character set */
extern CHAR (*xlr[MAXTCSETS+1][MAXFCSETS+1])();	/* translation functions. */
static CHAR (*sxo)();		/* Local translation functions */
static CHAR (*rxo)();		/* for output (sending) terminal chars */
static CHAR (*sxi)();		/* and for input (receiving) terminal chars. */
static CHAR (*rxi)();
#endif /* CK_ANSIC */
extern int language;		/* Current language. */
static int langsv;		/* For remembering language setting. */
extern struct csinfo fcsinfo[]; /* File character set info. */
extern int tcsr, tcsl;		/* Terminal character sets, remote & local. */
static int tcs;			/* Intermediate ("transfer") character set. */
static int tcssize = 0;		/* Size of tcs */
#ifdef UNICODE				/* UTF-8 support */
#ifdef CK_ANSIC
extern int (*xl_ufc[MAXFCSETS+1])(USHORT);  /* Unicode to FCS */
extern USHORT (*xl_fcu[MAXFCSETS+1])(CHAR); /* FCS to Unicode */
extern int (*xuf)(USHORT);		/* Translation function UCS to FCS */
extern USHORT (*xfu)(CHAR);		/* Translation function FCS to UCS */
#else
extern int (*xl_ufc[MAXFCSETS+1])();
extern USHORT (*xl_fcu[MAXFCSETS+1])();
extern int (*xuf)();
extern USHORT (*xfu)();
#endif /* CK_ANSIC */
#endif /* UNICODE */
#endif /* NOCSETS */

static int printing = 0;

/*
  We do not need to parse and recognize escape sequences if we are being built
  without character-set support AND without APC support.
*/
#ifdef NOESCSEQ
#ifdef XPRINT
#undef XPRINT
#endif /* XPRINT */

#else  /* NOESCSEQ not defined from outside */

#ifdef NOCSETS				/* No character sets */
#ifndef CK_APC				/* No APC */
#ifndef XPRINT				/* No transparent printing */
#define NOESCSEQ			/* So no escape sequence recognizer */
#endif /* XPRINT */
#endif /* CK_APC */
#endif /* NOCSETS */
#endif /* NOESCSEQ */

/* inesc[] and oldesc[] made global 2010/03/01 for INPUT command */

static int escseq = 0;			/* 1 = Recognizer is active */
/* static */ int inesc[2] = { 0, 0 };	/* State of sequence recognizer */
/* static */ int oldesc[2] = { -1, -1 }; /* Previous state of recognizer */

#ifdef NOESCSEQ
#define ES_NORMAL 0			/* Normal, not in an escape sequence */
#define chkaes(x,y) 0
#else
/*
  As of C-Kermit 5A(178), the CONNECT command skips past ANSI escape sequences
  to avoid translating the characters within them.  This allows the CONNECT
  command to work correctly with a host that uses a 7-bit ISO 646 national
  character set, in which characters like '[' would normally be converted to
  accented letters, ruining the terminal's interpretation (and generation)
  of escape sequences.

  As of 5A(190), the CONNECT command responds to APC escape sequences
  (ESC _ text ESC \) if the user SETs TERMINAL APC ON or UNCHECKED, and the
  program was built with CK_APC defined.

  Non-ANSI/ISO-compliant escape sequences are not handled. */

/* States for the escape-sequence recognizer. */

#define ES_NORMAL 0			/* Normal, not in an escape sequence */
#define ES_GOTESC 1			/* Current character is ESC */
#define ES_ESCSEQ 2			/* Inside an escape sequence */
#define ES_GOTCSI 3			/* Inside a control sequence */
#define ES_STRING 4			/* Inside DCS,OSC,PM, or APC string */
#define ES_TERMIN 5			/* 1st char of string terminator */

/*
  ANSI escape sequence handling.  Only the 7-bit form is treated, because
  translation is not a problem in the 8-bit environment, in which all GL
  characters are ASCII and no translation takes place.  So we don't check
  for the 8-bit single-character versions of CSI, DCS, OSC, APC, or ST.
  Here is the ANSI sequence recognizer state table, followed by the code
  that implements it.

  Definitions:
    CAN = Cancel                       01/08         Ctrl-X
    SUB = Substitute                   01/10         Ctrl-Z
    DCS = Device Control Sequence      01/11 05/00   ESC P
    CSI = Control Sequence Introducer  01/11 05/11   ESC [
    ST  = String Terminator            01/11 05/12   ESC \
    OSC = Operating System Command     01/11 05/13   ESC ]
    PM  = Privacy Message              01/11 05/14   ESC ^
    APC = Application Program Command  01/11 05/15   ESC _

  ANSI escape sequence recognizer:

    State    Input  New State  ; Commentary

    NORMAL   (start)           ; Start in NORMAL state

    (any)    CAN    NORMAL     ; ^X cancels
    (any)    SUB    NORMAL     ; ^Z cancels

    NORMAL   ESC    GOTESC     ; Begin escape sequence
    NORMAL   other             ; NORMAL control or graphic character

    GOTESC   ESC               ; Start again
    GOTESC   [      GOTCSI     ; CSI
    GOTESC   P      STRING     ; DCS introducer, consume through ST
    GOTESC   ]      STRING     ; OSC introducer, consume through ST
    GOTESC   ^      STRING     ; PM  introducer, consume through ST
    GOTESC   _      STRING     ; APC introducer, consume through ST
    GOTESC   0..~   NORMAL     ; 03/00 through 17/14 = Final character
    GOTESC   other  ESCSEQ     ; Intermediate or ignored control character

    ESCSEQ   ESC    GOTESC     ; Start again
    ESCSEQ   0..~   NORMAL     ; 03/00 through 17/14 = Final character
    ESCSEQ   other             ; Intermediate or ignored control character

    GOTCSI   ESC    GOTESC     ; Start again
    GOTCSI   @..~   NORMAL     ; 04/00 through 17/14 = Final character
    GOTCSI   other             ; Intermediate char or ignored control char

    STRING   ESC    TERMIN     ; Maybe have ST
    STRING   other             ; Consume all else

    TERMIN   \      NORMAL     ; End of string
    TERMIN   other  STRING     ; Still in string
*/

#ifdef XPRINT				/* Transparent print support */
/*
  We can't just print each byte as it comes in because then the printer-off
  sequence would be sent to the printer.  Thus we have to buffer up escape
  sequences and print them only when they are complete AND we know they are
  not the printer-off sequence.  All printing is done via zsoutx(ZMFILE,s,n).
  This allows for strings that contain NULs.  Don't mix calls to zsoutx() with
  calls to zchout(), or the output will be scrambled.  Also note that when
  printing a saved-up escape sequence, we never print its final character
  because that will be printed in the mainline code, upon return from
  chkaes().  Note that the printer-on sequence is passed to the screen; this
  is unavoidable, since we don't know what it is until after we get to the
  end, and for screen display purposes we can't buffer up escape sequences
  for numerous reasons.  Therefore we also must output the printer-off
  sequence, otherwise a real terminal or emulator will be stuck in print mode.
*/
extern int tt_print;
#define ESCBUFLEN 63
static char escbuf[ESCBUFLEN+1] = { NUL, NUL };
static int escbufc = 0;
static int dontprint = 0;

VOID
printon() {				/* Turn printing on */
    int x, pp;
    char * p;
    extern int printpipe, noprinter;
    extern char * printername;

    if (noprinter) {
	debug(F110,"PRINTER ON NOPRINTER","",0);
	return;
    }
    p = printername;
    pp = printpipe;
    if (!p) p = "";
    if (!*p) {
#ifdef ANYBSD
	p = "lpr";
#else
	p = "lp";
#endif /* ANYBSD */
	pp = 1;
	debug(F110,"PRINTER DEFAULT",p,0);
    }
    debug(F111,"PRINTER ON",p,pp);
    if (pp) {				/* Printing to pipe */
	x = zxcmd(ZMFILE,p);
    } else {				/* Append to file */
	struct filinfo xx;
	xx.bs = 0; xx.cs = 0; xx.rl = 0; xx.org = 0; xx.cc = 0;
	xx.typ = 0; xx.dsp = XYFZ_A; xx.os_specific = NUL;
	xx.lblopts = 0;
	x = zopeno(ZMFILE,p,NULL,&xx);
    }
    debug(F101,"PRINTER OPEN","",x);
    printing = 1;
}

VOID
printoff() {				/* Turn printing off */
    int x;
    extern int noprinter;
    if (noprinter) {
	printing = 0;
	debug(F100,"PRINTER OFF NOPRINTER","",0);
	return;
    }
    debug(F100,"PRINTER OFF","",0);
    if (printing) {
	x = zclose(ZMFILE);
	debug(F101,"PRINTER CLOSE","",x);
	printing = 0;
    }
}
#endif /* XPRINT */

/*
  C H K A E S  --  Check ANSI Escape Sequence.

  Call with EACH character in input stream.
  src = 0 means c is incoming from remote; 1 = char from keyboard.
  Sets global inesc[src] variable according to escape sequence state.
  Returns 0 normally, 1 if an APC sequence is to be executed.
  Handles transparent printing internally.
*/
int
#ifdef CK_ANSIC
chkaes(char c, int src)
#else
chkaes(c,src) char c; int src;
#endif /* CK_ANSIC */
/* chkaes */ {

    debug(F111,"chkaes entry inesc",ckitoa(src),inesc[src]);
    debug(F101,"chkaes c","",c);

    if (src < 0 || src > 1)		/* Don't allow bad args. */
      return(0);

    oldesc[src] = inesc[src];		/* Remember previous state */

#ifdef XPRINT
    if (inesc[src] && !src) {		/* Save up escape seq for printing  */
	if (!c) return(0);		/* Ignore NULs */
	if (escbufc < ESCBUFLEN) {
	    escbuf[escbufc++] = c;
	    escbuf[escbufc] = NUL;
	    debug(F111,"ESCBUF 1",escbuf,escbufc);
	} else {			/* Buffer overrun */
	    if (printing && escbufc)	/* Print what's there so far */
	      zsoutx(ZMFILE,escbuf,escbufc);
	    escbufc = 1;		/* clear it out */
	    escbuf[0] = c;		/* and start off fresh buffer */
	    escbuf[1] = NUL;		/* with this character. */
	}
    }
#endif /* XPRINT */

    if (c == CAN || c == SUB) {		/* CAN and SUB cancel any sequence */
#ifdef XPRINT
	if (!src) {
	    if (printing && escbufc > 1)
	      zsoutx(ZMFILE,escbuf,escbufc-1);
	    escbufc = 0;		/* Clear buffer */
	    escbuf[0] = NUL;
	}
#endif /* XPRINT */
	inesc[src] = ES_NORMAL;
    } else				/* Otherwise */

      switch (inesc[src]) {		/* enter state switcher */
	case ES_NORMAL:			/* NORMAL state */
	  if (c == ESC) {		/* Got an ESC */
	      inesc[src] = ES_GOTESC;	/* Change state to GOTESC */
#ifdef XPRINT
	      if (!src) {
		  escbufc = 1;		/* Clear escape sequence buffer */
		  escbuf[0] = c;	/* and deposit the ESC */
		  escbuf[1] = NUL;
		  debug(F111,"ESCBUF 2",escbuf,escbufc);
	      }
#endif /* XPRINT */
	  }
	  break;			/* Otherwise stay in NORMAL state */

	case ES_GOTESC:			/* GOTESC state - prev char was ESC*/
	  if (c == '[') {		/* Left bracket after ESC is CSI */
	      inesc[src] = ES_GOTCSI;	/* Change to GOTCSI state */
	  } else if (c == 'P' || (c > 0134 && c < 0140)) { /* P, ], ^, or _ */
	      inesc[src] = ES_STRING;	/* Switch to STRING-absorption state */
#ifdef XPRINT
	      debug(F111,"ESCBUF STRING",escbuf,escbufc);
#endif /* XPRINT */
#ifdef CK_APC
	      /* If APC not disabled */
	      if (!src && c == '_' && (apcstatus & APC_ON)) {
		  debug(F100,"CONNECT APC begin","",0);
		  apcactive = APC_REMOTE; /* Set APC-Active flag */
		  apclength = 0;	/* and reset APC buffer pointer */
	      }
#endif /* CK_APC */
	  } else if (c > 057 && c < 0177) { /* Final character '0' thru '~' */
	      inesc[src] = ES_NORMAL;	/* Back to normal */
#ifdef XPRINT
	      if (!src) {
		  if (printing && escbufc > 1) {
		      /* Dump esc seq buf to printer */
		      zsoutx(ZMFILE,escbuf,escbufc-1);
		      debug(F111,"ESCBUF PRINT 1",escbuf,escbufc);
		  }

		  escbufc = 0;		/* Clear parameter buffer */
		  escbuf[0] = NUL;
	      }
#endif /* XPRINT */
	  } else if (c != ESC) {	/* ESC in an escape sequence... */
	      inesc[src] = ES_ESCSEQ;	/* starts a new escape sequence */
	  }
	  break;			/* Intermediate or ignored ctrl char */

	case ES_ESCSEQ:			/* ESCSEQ -- in an escape sequence */
	  if (c > 057 && c < 0177) {	/* Final character '0' thru '~' */
	      inesc[src] = ES_NORMAL;	/* Return to NORMAL state. */
#ifdef XPRINT
	      if (!src) {
		  if (printing && escbufc > 1) {
		      zsoutx(ZMFILE,escbuf,escbufc-1);
		      debug(F111,"ESCBUF PRINT 2",escbuf,escbufc);
		  }
		  escbufc = 0;		/* Clear escseq buffer */
		  escbuf[0] = NUL;
	      }
#endif /* XPRINT */
	  } else if (c == ESC) {	/* ESC ... */
	      inesc[src] = ES_GOTESC;	/* starts a new escape sequence */
	  }
	  break;			/* Intermediate or ignored ctrl char */

	case ES_GOTCSI:			/* GOTCSI -- In a control sequence */
	  if (c > 077 && c < 0177) {	/* Final character '@' thru '~' */
#ifdef XPRINT
	      if (!src && tt_print) {	/* Printer enabled? */
		  if (c == 'i') {	/* Final char is "i"? */
		      char * p = (char *) (escbuf + escbufc - 4);
		      if (!strncmp(p, "\033[5i", 4)) { /* Turn printer on */
			  printon();
		      } else if (!strncmp(p, "\033[4i", 4)) { /* Or off... */
			  int i;
			  printoff();			/* Turn off printer. */
			  dontprint = 1;
			  for (i = 0; i < escbufc; i++)	/* And output the */
			    ckcputc(escbuf[i]);         /* sequence. */
		      } else if (printing && escbufc > 1) {
			  zsoutx(ZMFILE,escbuf,escbufc-1);
			  debug(F011,"ESCBUF PRINT 3",escbuf,escbufc);
		      }
		  } else if (printing && escbufc > 1) {
		      zsoutx(ZMFILE,escbuf,escbufc-1);
		      debug(F111,"ESCBUF PRINT 4",escbuf,escbufc);
		  }
	      }
	      if (!src) {
		  escbufc = 0;		/* Clear esc sequence buffer */
		  escbuf[0] = NUL;
	      }
#endif /* XPRINT */
	      inesc[src] = ES_NORMAL;	/* Return to NORMAL. */
	  } else if (c == ESC) {	/* ESC ... */
	      inesc[src] = ES_GOTESC;	/* starts over. */
	  }
	  break;

	case ES_STRING:			/* Inside a string */
	  if (c == ESC)			/* ESC may be 1st char of terminator */
	    inesc[src] = ES_TERMIN;	/* Go see. */
#ifdef CK_APC
	  else if (apcactive) {		/* If in APC */
	      if (apclength < apcbuflen) { /* and there is room... */
		  apcbuf[apclength++] = c; /* deposit this character. */
	      } else {			/* Buffer overrun */
		  apcactive = 0;	/* Discard what we got */
		  apclength = 0;	/* and go back to normal */
		  apcbuf[0] = 0;	/* Not pretty, but what else */
		  inesc[src] = ES_NORMAL; /* can we do?  (ST might not come) */
	      }
	  }
#endif /* CK_APC */
	  break;			/* Absorb all other characters. */

	case ES_TERMIN:			/* Maybe a string terminator */
	  if (c == '\\') {		/* which must be backslash */
	      inesc[src] = ES_NORMAL;	/* If so, back to NORMAL */
#ifdef XPRINT
	      if (!src) {
		  if (printing && escbufc > 1) { /* If printing... */
		      /* Print esc seq buffer */
		      zsoutx(ZMFILE,escbuf,escbufc-1);
		      debug(F111,"ESCBUF PRINT 5",escbuf,escbufc);
		  }
		  escbufc = 0;		/* Clear escseq buffer */
		  escbuf[0] = NUL;
	      }
#endif /* XPRINT */
#ifdef CK_APC
	      if (!src && apcactive) {	/* If it was an APC string, */
		  debug(F101,"CONNECT APC terminated","",c);
		  apcbuf[apclength] = NUL; /* terminate it and then ... */
		  return(1);
	      }
#endif /* CK_APC */
	  } else {			/* It's not a backslash so... */
	      inesc[src] = ES_STRING;	/* back to string absorption. */
#ifdef CK_APC
	      if (apcactive) {		/* In APC string */
		  if (apclength+1 < apcbuflen) { /* If enough room */
		      apcbuf[apclength++] = ESC; /* deposit the Esc */
		      apcbuf[apclength++] = c;   /* and this character too. */
		  } else {		/* Buffer overrun */
		      apcactive = 0;
		      apclength = 0;
		      apcbuf[0] = 0;
		      inesc[src] = ES_NORMAL;
		  }
	      }
#endif /* CK_APC */
	  }
      }	/* switch() */
    debug(F111,"chkaes exit inesc",ckitoa(src),inesc[src]);
    return(0);
}
#endif /* NOESCSEQ */

VOID
#ifdef CK_ANSIC
LOGCHAR(char c)
#else
LOGCHAR(c) char c;
#endif /* CK_ANSIC */
/* LOGCHAR */ {                         /* Log character c to session log */
    /* but skip over escape sequences if session log is text */
    if (escseq) {
	if ((sessft == XYFT_T) && (debses == 0) &&
	    (inesc[0] != ES_NORMAL || oldesc[0] != ES_NORMAL))
	  return;
    }
    logchar(c);
}

/*  C K C P U T C  --  C-Kermit CONNECT Put Character to Screen  */
/*
  Output is buffered to avoid slow screen writes on fast connections.
*/
static int
ckcputf() {				/* Dump the console output buffer */
    int x = 0;
    if (obc > 0)			/* If we have any characters, */
      x = conxo(obc,obuf);		/* dump them, */
    obp = obuf;				/* reset the pointer */
    obc = 0;				/* and the counter. */
    return(x);				/* Return conxo's return code */
}

/*
  NOTE: This is probably the right place for character-set translation,
  rather than down below in the mainline code.  ckcputc() would act like
  xpnbyte() in ckcfns.c, and ckcgetc() would act like xgnbyte().  This
  would shield the rest of the code from all the complexities of many-to-one
  and one-to-many conversions, and would allow handling of Kanji and other
  CJK sets along with UTF-8 and the rest.
*/
int
ckcputc(c) int c; {
    int x;

    *obp++ = c & 0xff;			/* Deposit the character */
    obc++;				/* Count it */
    if (ibc == 0 ||			/* If input buffer about empty */
	obc == OBUFL) {			/* or output buffer full */
	debug(F101,"CONNECT CKCPUTC obc","",obc);
	x = conxo(obc,obuf);		/* dump the buffer, */
	obp = obuf;			/* reset the pointer */
	obc = 0;			/* and the counter. */
	return(x);			/* Return conxo's return code */
    } else return(0);
}

/*  C K C G E T C  --  C-Kermit CONNECT Get Character  */
/*
  Buffered read from communication device.
  Returns the next character, refilling the buffer if necessary.
  On error, returns ttinc's return code (see ttinc() description).
  Dummy argument for compatible calling conventions with ttinc()
  so a pointer to this function can be passed to tn_doop().
*/
int
ckcgetc(dummy) int dummy; {
    int c, n;
#ifdef CK_SSL
    extern int ssl_active_flag, tls_active_flag;
#endif /* CK_SSL */

#ifdef CK_ENCRYPTION
    /* No buffering for possibly encrypted connections */
    if (network && IS_TELNET() && TELOPT_ME(TELOPT_AUTHENTICATION))
      return(ttinc(0));
#endif /* CK_ENCRYPTION */
#ifdef CK_SSL
    if (ssl_active_flag || tls_active_flag)
        return(ttinc(0));
#endif /* CK_SSL */

    if (ibc < 1) {			/* Need to refill buffer? */
	ibc = 0;			/* Yes, reset count */
	ibp = ibuf;			/* and buffer pointer */
	c = ttinc(0);			/* Read one character, blocking */
	if (c < 0) {			/* If error, return error code */
	    return(c);
	} else {			/* Otherwise, got one character */
	    *ibp++ = c;			/* Advance buffer pointer */
	    ibc++;			/* and count. */
	}
	if ((n = ttchk()) > 0) {	/* Any more waiting? */
	    if (n > (IBUFL - ibc))	/* Get them all at once. */
	      n = IBUFL - ibc;		/* Don't overflow buffer */
	      if ((n = ttxin(n,(CHAR *)ibp)) > 0) {
		  ibc += n;		/* Advance counter */
              }
	} else if (n < 0) {		/* Error? */
	    return(n);			/* Return the error code */
	}
	ibp = ibuf;			/* Point to beginning of buffer */
    }
    c = *ibp++ & 0xff;			/* Get next character from buffer */
    ibc--;				/* Reduce buffer count */
    /* debug(F000,"CKCGETC","",c); */
    return(c);				/* Return the character */
}

/*
   Keyboard handling, buffered for speed, which is needed when C-Kermit is
   in CONNECT mode between two other computers that are transferring data.
*/
static char *kbp;			/* Keyboard input buffer pointer */
static int kbc;				/* Keyboard input buffer count */

#ifdef CK_SMALL				/* Keyboard input buffer length */
#define KBUFL 32			/* Small for PDP-11 UNIX */
#else
#define KBUFL 257			/* Regular kernel size for others */
#endif /* CK_SMALL */

#ifdef DYNAMIC
static char *kbuf = NULL;
#else
static char kbuf[KBUFL];
#endif /* DYNAMIC */

/* Macro for reading keystrokes. */

#define CONGKS() (((--kbc)>=0) ? ((int)(*kbp++) & 0377) : kbget())

/*
  Note that we call read() directly here, normally a no-no, but in this case
  we know it's UNIX and we're only doing what coninc(0) would have done,
  except we're reading a block of characters rather than just one.  There is,
  at present, no conxin() analog to ttxin() for chunk reads, and instituting
  one would only add function-call overhead as it would only be a wrapper for
  a read() call anyway.

  Another note: We stick in this read() till the user types something.
  But we know they already did, since select() said so.  Therefore something
  would need to be mighty wrong before we get stuck here.
*/
static int				/* Keyboard buffer filler */
kbget() {
#ifdef EINTR
    int tries = 10;			/* If read() is interrupted, */
    int ok = 0;
    while (tries-- > 0) {		/* try a few times... */
#endif /* EINTR */
	kbc = conchk();			/* How many chars waiting? */
	debug(F101,"kbget kbc","",kbc);
	if (kbc < 1)
	  kbc = 1;			/* If none or dunno, wait for one. */
	else if (kbc > KBUFL)		/* If too many, */
	  kbc = KBUFL;			/* only read this many. */
	if ((kbc = read(0, kbuf, kbc)) < 1) { /* Now read it/them. */
	    debug(F101,"CONNECT kbget errno","",errno);	/* Got an error. */
#ifdef EINTR
	    if (errno == EINTR)		/* Interrupted system call. */
	      continue;			/* Try again, up to limit. */
	    else			/* Something else. */
#endif /* EINTR */
	      return(-1);		/* Pass along read() error. */
	}
#ifdef EINTR
	else { ok = 1; break; }
    }
    if (!ok) return(-1);
#endif /* EINTR */
    kbp = kbuf;				/* Adjust buffer pointer, */
    kbc--;				/* count, */
    return((int)(*kbp++) & 0377);	/* and return first character. */
}

#ifdef BEBOX
/*
 * CreateSocketPair --
 *
 *	This procedure creates a connected socket pair
 *
 * Results:
 *	0 if OK, the error if not OK.
 *
 * Side effects:
 *	None
 */
int
socketpair(int *pair) {
    int servsock;
    int val;
    struct sockaddr_in serv_addr, cli_addr;
    extern char myipaddr[];

    debug(F110,"socketpair",myipaddr,0);

    if (myipaddr[0] == 0)
      getlocalipaddr();

    servsock = socket(AF_INET, SOCK_STREAM, 0);
    if (servsock == 0) {
	return h_errno;
    }
    debug(F111,"socketpair","socket",servsock);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(0);

    val = sizeof(serv_addr);
    if (bind(servsock, (struct sockaddr *) &serv_addr, val) < 0) {
	closesocket(servsock);
	return h_errno;
    }
    debug(F111,"socketpair","bind",0);

    listen(servsock, 1);
    debug(F111,"socketpair","listen",0);

    if (getsockname(servsock, (struct sockaddr *) &serv_addr, &val) < 0) {
	closesocket(servsock);
	return h_errno;
    }
    debug(F111,"socketpair","getsockname",0);

    pair[0] = socket(AF_INET, SOCK_STREAM, 0);
    if (pair[0] == 0) {
	closesocket(servsock);
	return h_errno;
    }
    debug(F111,"socketpair","socket",pair[0]);

    memset(&cli_addr, 0, sizeof(cli_addr));
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_addr.s_addr = inet_addr(myipaddr[0]?myipaddr:"127.0.0.1");
    cli_addr.sin_port = serv_addr.sin_port;

    if (connect(pair[0],(struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0) {
	closesocket(pair[0]);
	closesocket(servsock);
	return h_errno;
    }
    debug(F111,"socketpair","connect",0);

    pair[1] = accept(servsock, (struct sockaddr *) &serv_addr, &val);
    if (pair[1] == 0) {
	closesocket(pair[0]);
	closesocket(servsock);
	return h_errno;
    }
    debug(F111,"socketpair","accept",pair[1]);

    closesocket(servsock);
    debug(F111,"socketpair","closesocket",0);
    return 0;
}

long
kbdread(void * param) {
    int sock = (int) param;
    char ch;
    int rc = 0;

    debug(F111,"kbdread","sock",sock);

    while (rc >= 0) {
	rc = read(fileno(stdin), &ch, 1); /* Read a character. */
	if (rc > 0) {
	    rc = send(sock,&ch,1,0);
	    /* debug(F000,"kbdread","send()",ch); */
	    printf("\r\ngot: %c rc = %d\r\n",ch,rc);
	} else
	  msleep(100);
    }
    debug(F110,"kbdread","terminating",0);
    return(rc);
}
#endif /* BEBOX */

#ifdef CKLEARN
static VOID
learnchar(c) int c; {			/* Learned script keyboard character */
    int cc;
    char xbuf[8];

    if (!learning || !learnfp)
      return;

    switch (learnst) {			/* Learn state... */
      case 0:				/* Neutral */
      case 1:				/* Net */
	if (learnbc > 0) {		/* Have net characters? */
	    char buf[LEARNBUFSIZ];
	    int i, j, n;
	    ULONG t;

	    t = (ULONG) time(0);	/* Calculate INPUT timeout */
	    j = t - learnt1;
	    j += (j / 4) > 0 ? (j / 4) : 1; /* Add some slop */
	    if (j < 2) j = 2;		    /* 2 seconds minimum */

	    fputs("\nINPUT ",learnfp);	/* Give INPUT command for them */
	    fputs(ckitoa(j),learnfp);
	    fputs(" {",learnfp);
	    learnt1 = t;

	    n = LEARNBUFSIZ;
	    if (learnbc < LEARNBUFSIZ) {  /* Circular buffer */
		n = learnbc;		  /*  hasn't wrapped yet. */
		learnbp = 0;
	    }
	    j = 0;			/* Copy to linear buffer */
	    for (i = 0; i < n; i++) {	/* Number of chars in circular buf */

		cc = learnbuf[(learnbp + i) % LEARNBUFSIZ];

		/* Later account for prompts that end with a newline? */

		if (cc == CR && j > 0) {
		    if (buf[j-1] != LF)
		      j = 0;
		}
		buf[j++] = cc;
	    }
	    for (i = 0; i < j; i++) {	/* Now copy out the buffer */
		cc = buf[i];		/* interpreting control chars */
		if (cc == 0) {		/* We don't INPUT NULs */
		    continue;
		} else if (cc < SP ||	/* Controls need quoting */
			   (cc > 126 && cc < 160)) {
		    ckmakmsg(xbuf,8,"\\{",ckitoa((int)cc),"}",NULL);
		    fputs(xbuf,learnfp);
		} else {		/* Plain character */
		    putc(cc,learnfp);
		}
	    }
	    fputs("}\nIF FAIL STOP 1 INPUT timeout",learnfp);
	    learnbc = 0;
	}
	learnbp = 0;
	fputs("\nPAUSE 1\nOUTPUT ",learnfp); /* Emit OUTPUT and fall thru */

      case 2:				/* Already in Keyboard state */
	if (c == 0) {
	    fputs("\\N",learnfp);
	} else if (c == -7) {
	    fputs("\\B",learnfp);
	} else if (c == -8) {
	    fputs("\\L",learnfp);
	} else if (c < SP || (c > 126 && c < 160)) {
	    ckmakmsg(xbuf,8,"\\{",ckitoa((int)c),"}",NULL);
	    fputs(xbuf,learnfp);
	} else {
	    putc(c,learnfp);
	}
    }
}
#endif /* CKLEARN */

static int printbar = 0;

#define OUTXBUFSIZ 15
static CHAR inxbuf[OUTXBUFSIZ+1];	/* Host-to-screen expansion buffer */
static int inxcount = 0;		/* and count */
static CHAR outxbuf[OUTXBUFSIZ+1];	/* Keyboard-to-host expansion buf */
static int outxcount = 0;		/* and count */

int
conect() {
    int rc = 0;				/* Return code: 0 = fail, 1 = OK */
    int i, x = 0, prev = -1;		/* Reason code in cx_status */
#ifdef CKLEARN
    int crflag = 0;
#endif /* CKLEARN */
    register int c = -1, c2, csave;	/* Characters */
#ifdef TNCODE
    int tx;				/* For Telnet negotiations */
#endif /* TNCODE */
    int apcrc = 0;			/* For APC and transparent print */
    int n, kbin, scrnout;		/* select() items... */
    fd_set in, out, err;		/* File descriptor sets */
    int gotnet = 0;			/* Flag for net ready to read */
    int gotkbd = 0;			/* Flag for keyboard ready to read */
    int oldprt = 0;			/* Used with printing */
    int msgflg = 0;
    char cbuf[2];			/* Ditto */

#ifdef BEBOX
    int tid = 0;			/* Thread ID */
    int pair[2];			/* Socket Pair */
    CHAR ch;
    CHAR buf[64];
#endif /* BEBOX */

    cx_status = CSX_INTERNAL;
    debok = 1;

#ifdef BEBOX
    {
	/* Create a socket pair to be used for the keyboard input */
	if (socketpair(pair)) {
	    debug(F110,"conect","unable to create socket pair",0);
	    return(-1);
	}
	debug(F111,"connect","socket pair[0]",pair[0]);
	debug(F111,"connect","socket pair[1]",pair[1]);

	/* Assign one end of the socket to kbin */
	kbin = pair[0];
        tid = spawn_thread(kbdread,
			   "Kbd to Socket Pair",
			    B_NORMAL_PRIORITY,
			   (void *)pair[1]
			   );
        resume_thread(tid);
	debug(F110,"connect","tid",tid);
    }
#else /* BEBOX */
    kbin = fileno(stdin);		/* stdin file descriptor */
#endif /* BEBOX */

    scrnout = fileno(stdout);		/* stdout file descriptor */

#ifdef CK_TRIGGER
    makestr(&triggerval,NULL);		/* Reset trigger */
#endif /* CK_TRIGGER */

#ifdef XPRINT
    escbufc = 0;			/* Reset esc-sequence buffer */
    escbuf[0] = NUL;
#endif /* XPRINT */
    cbuf[1] = NUL;

    ttimoff();				/* Turn off any timer interrupts */
    if (!local) {			/* Be sure we're not in remote mode */
#ifdef NETCONN
#ifdef NEWFTP
	if (ftpisconnected())
	  printf("Sorry, you can't CONNECT to an FTP server\n");
	else
#endif /* NEWFTP */
	  printf("Sorry, you must SET LINE or SET HOST first\n");
#else
	printf("Sorry, you must SET LINE first\n");
#endif /* NETCONN */
	return(0);
    }
    if (speed < 0L && network == 0 && ttfdflg == 0) {
	printf("Sorry, you must SET SPEED first\n");
	return(0);
    }
#ifdef TCPSOCKET
    if (network && !ttpipe && (nettype != NET_TCPB && nettype != NET_PTY)) {
	printf("Sorry, network type not supported\n");
	return(0);
    }
#endif /* TCPSOCKET */

#ifdef DYNAMIC
    if (!ibuf) {
	if (!(ibuf = malloc(IBUFL+1))) { /* Allocate input line buffer */
	    printf("Sorry, CONNECT input buffer can't be allocated\n");
	    return(0);
	} else {
	    ibp = ibuf;
	    ibc = 0;
	}
    }
    if (!obuf) {
	if (!(obuf = malloc(OBUFL+1))) { /* Allocate output line buffer */
	    printf("Sorry, CONNECT output buffer can't be allocated\n");
	    return(0);
	} else {
	    obp = obuf;
	    obc = 0;
	}
    }
    if (!kbuf) {
	if (!(kbuf = malloc(KBUFL+1))) { /* Allocate keyboard input buffer */
	    printf("Sorry, CONNECT keyboard buffer can't be allocated\n");
	    return(0);
	}
    }
    if (!temp) {
	if (!(temp = malloc(TMPLEN+1))) { /* Allocate temporary buffer */
	    printf("Sorry, CONNECT temporary buffer can't be allocated\n");
	    return(0);
	}
    }
#else
    obp = obuf;
    obc = 0;
#endif /* DYNAMIC */

    kbp = kbuf;				/* Always clear these. */
    *kbp = NUL;				/* No need to preserve them between */
    kbc = 0;				/* CONNECT sessions. */

#ifdef DEBUG
    if (deblog) {
	debug(F101,"CONNECT conect entry ttyfd","",ttyfd);
	debug(F101,"CONNECT conect entry ibc","",ibc);
	debug(F101,"CONNECT conect entry obc","",obc);
	debug(F101,"CONNECT conect entry kbc","",kbc);
#ifdef CK_TRIGGER
	debug(F110,"CONNECT conect trigger",tt_trigger[0],0);
#endif /* CK_TRIGGER */
	if (ttyfd > -1) {
	    n = ttchk();
	    debug(F101,"CONNECT conect entry ttchk","",n);
	}
    }
#endif /* DEBUG */

    if (ttyfd < 0) {			/* If communication device not open */
#ifdef TTLEBUF
        int n = le_inbuf();
        debug(F111,"CONNECT le_inbuf()","ttyfd < 0",n);
        if (n > 0) {
            while (n--) {
                CHAR ch;
                le_getchar(&ch);
                conoc(ch);
            }
            return(0);
        }
#endif /* TTLEBUF */

	debug(F101,"CONNECT ttnproto","",ttnproto);
	debug(F111,"CONNECT opening",ttname,0); /* Open it now */
	if (ttopen(ttname,
		   &local,
		   network ? -nettype : mdmtyp,
		   0
		   ) < 0) {
	    ckmakmsg(temp,TMPLEN,"Sorry, can't open ",ttname,NULL,NULL);
	    perror(temp);
	    debug(F110,"CONNECT open failure",ttname,0);
	    return(0);
	}

#ifdef IKS_OPTION
	/* If peer is in Kermit server mode, return now. */
	if (TELOPT_SB(TELOPT_KERMIT).kermit.u_start) {
	    cx_status = CSX_IKSD;
	    return(0);
	}
#endif /* IKS_OPTION */
    }
    dohangup = 0;			/* Hangup not requested yet */

    msgflg = !quiet
#ifdef CK_APC
      && !apcactive
#endif /* CK_APC */
	;

    if (msgflg) {
#ifdef NETCONN
	if (network) {
#ifdef CK_ENCRYPTION
	    extern int me_encrypt, u_encrypt;
	    if (ck_tn_encrypting() && ck_tn_decrypting())
	      printf("SECURE connection to host %s",ttname);
	    else
#endif /* CK_ENCRYPTION */
	      if (ttpipe || ttpty)
		printf("Connecting via command \"%s\"",ttname);
	      else
		printf("Connecting to host %s",ttname);
	} else {
#endif /* NETCONN */
	    printf("Connecting to %s",ttname);
	    if (speed > -1L) printf(", speed %ld",speed);
#ifdef NETCONN
	}
#endif /* NETCONN */
	if (tt_escape) {
	    printf("\r\n");
	    shoesc(escape);
	    printf("Type the escape character followed by C to get back,\r\n");
	    printf("or followed by ? to see other options.\r\n");
	} else {
	    printf(".\r\n\nESCAPE CHARACTER IS DISABLED\r\n\n");
	}
	if (seslog) {
	    extern int slogts;
	    char * s = "";
	    switch (sessft) {
	      case XYFT_D:
		s = "debug"; break;
	      case XYFT_T:
		s = slogts ? "timestamped-text" : "text"; break;
	      default:
		s = "binary";
	    }
	    printf("Session Log: %s, %s (%d) \r\n",sesfil,s,sessft);
	}
	if (debses) printf("Debugging Display...)\r\n");
    }

/* Condition console terminal and communication line */

    if (conbin((char)escape) < 0) {
	printf("Sorry, can't condition console terminal\n");
	fflush(stdout);
	return(0);
    }
    debug(F101,"CONNECT cmask","",cmask);
    debug(F101,"CONNECT cmdmsk","",cmdmsk);
    debug(F101,"CONNECT speed before ttvt","",speed);
    if ((n = ttvt(speed,flow)) < 0) {	/* Enter "virtual terminal" mode */
	if (!network) {
	    debug(F101,"CONNECT ttvt","",n);
	    tthang();			/* Hang up and close the device. */
	    ttclos(0);
	    dologend();
	    if (ttopen(ttname,		/* Open it again... */
		       &local,
		       network ? -nettype : mdmtyp,
		       0
		       ) < 0) {
		cx_status = CSX_INTERNAL;
		ckmakmsg(temp,TMPLEN,"Sorry, can't reopen ",ttname,NULL,NULL);
		perror(temp);
		return(0);
	    }
#ifdef IKS_OPTION
	    if (TELOPT_SB(TELOPT_KERMIT).kermit.u_start) {
		cx_status = CSX_IKSD;
		return(0);
	    }
#endif /* IKS_OPTION */

	    if (ttvt(speed,flow) < 0) {	/* Try virtual terminal mode again. */
		conres();		/* Failure this time is fatal. */
		printf("Sorry, Can't condition communication line\n");
		cx_status = CSX_INTERNAL;
		return(0);
	    }
	}
    }
    debug(F101,"CONNECT ttvt ok, escape","",escape);

    /* Despite ttvt() this is still needed in HP-UX */
    /* because of the HP-9000 <RESET> key.*/

    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);

    debug(F101,"CONNECT carrier-watch","",carrier);
    if ((!network 
#ifdef TN_COMPORT
        || istncomport()
#endif /* TN_COMPORT */
	) && (carrier != CAR_OFF)) {
	int x;
	x = ttgmdm();
	debug(F100,"CONNECT ttgmdm","",x);
	if ((x > -1) && !(x & BM_DCD)) {
#ifndef NOHINTS
	    extern int hints;
#endif /* NOHINTS */
	    debug(F100,"CONNECT ttgmdm CD test fails","",x);
	    conres();
	    printf("?Carrier required but not detected.\n");
#ifndef NOHINTS
	    cx_status = CSX_CARRIER;
	    if (!hints)
	      return(0);
	    printf("***********************************\n");
	    printf(" Hint: To CONNECT to a serial device that\n");
	    printf(" is not presenting the Carrier Detect signal,\n");
	    printf(" first tell C-Kermit to:\n\n");
	    printf("   SET CARRIER-WATCH OFF\n\n");
	    printf("***********************************\n\n");
#endif /* NOHINTS */
	    return(0);
	}
	debug(F100,"CONNECT ttgmdm ok","",0);
    }

    /* Now we are connected. */

    if (msgflg || printbar)
      printf("----------------------------------------------------\r\n");
    fflush(stdout);

#ifndef NOCSETS
/* Set up character set translations */

    unicode = 0;			/* Assume Unicode won't be involved */
    tcs = 0;				/* "Transfer" or "Other" charset */
    sxo = rxo = NULL;			/* Initialize byte-to-byte functions */
    sxi = rxi = NULL;

    if (tcsr != tcsl) {			/* Remote and local sets differ... */
#ifdef UNICODE
	if (tcsr == FC_UTF8 ||		/* Remote charset is UTF-8 */
	    tcsl == FC_UTF8) {		/* or local one is. */
	    xuf = xl_ufc[tcsl];		/* Incoming Unicode to local */
	    if (xuf || tcsl == FC_UTF8) {
		tcs = (tcsr == FC_UTF8) ? tcsl : tcsr; /* The "other" set */
		xfu = xl_fcu[tcs];	/* Local byte to remote Unicode */
		if (xfu)
		  unicode = (tcsr == FC_UTF8) ? 1 : 2;
	    }
	    tcssize = fcsinfo[tcs].size; /* Size of other character set. */
	} else {
#endif /* UNICODE */
	    tcs = gettcs(tcsr,tcsl);	/* Get intermediate set. */
	    sxo = xls[tcs][tcsl];	/* translation function */
	    rxo = xlr[tcs][tcsr];	/* pointers for output functions */
	    sxi = xls[tcs][tcsr];	/* and for input functions. */
	    rxi = xlr[tcs][tcsl];
#ifdef UNICODE
	}
#endif /* UNICODE */
    }
/*
  This is to prevent use of zmstuff() and zdstuff() by translation functions.
  They only work with disk i/o, not with communication i/o.  Luckily Russian
  translation functions don't do any stuffing...
*/
    langsv = language;
#ifndef NOCYRIL
    if (language != L_RUSSIAN)
#endif /* NOCYRIL */
      language = L_USASCII;

#ifdef COMMENT
#ifdef DEBUG
    if (deblog) {
	debug(F101,"CONNECT tcs","",tcs);
	debug(F101,"CONNECT tcsl","",tcsl);
	debug(F101,"CONNECT tcsr","",tcsr);
	debug(F101,"CONNECT fcsinfo[tcsl].size","",fcsinfo[tcsl].size);
	debug(F101,"CONNECT fcsinfo[tcsr].size","",fcsinfo[tcsr].size);
	debug(F101,"CONNECT unicode","",unicode);
    }
#endif /* DEBUG */
#endif /* COMMENT */

#ifdef CK_XYZ
#ifndef XYZ_INTERNAL
    {
	extern int binary;		/* See about ZMODEM autodownloads */
	char * s;
	s = binary ? ptab[PROTO_Z].p_b_rcmd : ptab[PROTO_Z].p_t_rcmd;
	if (!s) s = "";
	zmdlok = (*s != NUL);		/* OK if we have external commands */
    }
#endif /* XYZ_INTERNAL */
#endif /* CK_XYZ */

#ifndef NOESCSEQ
/*
  We need to activate the escape-sequence recognition feature when:
   (a) translation is elected, AND
   (b) the local and/or remote set is a 7-bit set other than US ASCII;
  Or:
   SET SESSION-LOG is TEXT (so we can strip escape sequences out of the log);
  Or:
   SET TERMINAL APC is not OFF (handled in the next statement).
*/
    escseq = (tcs != TC_TRANSP) &&	/* Not transparent */
      (fcsinfo[tcsl].size == 128 || fcsinfo[tcsr].size == 128) && /* 7 bits */
	(fcsinfo[tcsl].code != FC_USASCII); /* But not ASCII */
#endif /* NOESCSEQ */
#endif /* NOCSETS */

#ifndef NOESCSEQ
    if (!escseq) {			/* 2009/10/22 */
	if (seslog && !sessft)
	  escseq = 1;
    }
#ifdef CK_APC
    escseq = escseq || (apcstatus & APC_ON);
    apcactive = 0;			/* An APC command is not active */
    apclength = 0;			/* ... */
#endif /* CK_APC */
#ifdef XPRINT
    escseq |= tt_print;
#endif /* XPRINT */
    /* Initial state of recognizer */
    inesc[0] = ES_NORMAL;		/* Remote to screen */
    inesc[1] = ES_NORMAL;		/* Keyboard to remote */
    debug(F101,"CONNECT escseq","",escseq);
#endif /* NOESCSEQ */

    if (ttyfd > -1) {			/* (just in case...) */
	what = W_CONNECT;		/* Keep track of what we're doing */
	active = 1;
    }
#ifdef CKLEARN
    if (learning) {			/* Learned script active... */
	learnbp = 0;			/* INPUT buffer pointer */
	learnbc = 0;			/* INPUT buffer count */
	learnst = 0;			/* State (0 = neutral, none) */
	learnt1 = (ULONG) time(0);
    }
#endif /* CKLEARN */

#ifdef CKTIDLE
    idlelimit = tt_idlelimit;
#endif /* CKTIDLE */

    while (active) {			/* Big loop... */
	debug(F100,"CONNECT top of loop","",0);
	FD_ZERO(&in);			/* Clear select() structs */
	FD_ZERO(&out);
	FD_ZERO(&err);
	gotkbd = 0;
	gotnet = ttpeek();		/* Something sitting in ckutio buf */
	debug(F101,"CONNECT ttpeek","",gotnet);

	if (
#ifndef NOSETKEY
	    !kmptr			/* Check for key macro active */
#else
	    1
#endif /* NOSETKEY */
	    ) {
	    if (obc) {			/* No key macro - set up for select */
		FD_SET(ttyfd, &out);	/* Have stuff to send to net */
	    } else {
		FD_SET(kbin, &in);	/* Need to read stuff from keyboard */
	    }
#ifdef BEBOX
	    if (!(ibc || gotnet > 0))
		FD_SET(ttyfd, &in);	/* Need to read stuff from net */
#else /* BEBOX */
	    if (ibc || gotnet > 0) {
		FD_SET(scrnout, &out);	/* Have stuff to put on screen */
	    } else {
		FD_SET(ttyfd, &in);	/* Need to read stuff from net */
	    }
#endif /* BEBOX */
            FD_SET(ttyfd, &err);
#ifdef CK_FORWARD_X
            fwdx_init_fd_set(&in);
#endif /* CK_FORWARD_X */

	    /* Wait till the first one of the above is ready for i/o */
	    /* or TERM IDLE-SEND is active and we time out. */

	    errno = 0;
#ifdef CKTIDLE
	    /* This really could be moved out of the loop... */
	    if (idlelimit) {		/* Idle timeout set */
		struct timeval tv;
		if (idlelimit > 0) {	/* Positive = sec */
		    tv.tv_sec = (long) idlelimit;
		    tv.tv_usec = 0L;
		} else {		/* Negative = millisec */
		    long u = (0 - idlelimit);
		    tv.tv_sec = u / 1000L;
		    tv.tv_usec = ((u % 1000L) * 1000L);
		}
#ifdef INTSELECT
		c = select(FD_SETSIZE,(int *)&in,(int *)&out,(int *)&err, &tv);
#else
		c = select(FD_SETSIZE, &in, &out, &err, &tv);
#endif /* INTSELECT */
	    } else
#endif /* CKTIDLE */
#ifdef INTSELECT
	      c = select(FD_SETSIZE, (int *)&in, (int *)&out, (int *)&err, 0);
#else
	      c = select(FD_SETSIZE, &in, &out, &err, 0);
#endif /* INTSELECT */
	    if (c < 1) {
#ifdef CKTIDLE
		if (c == 0) {		/* Timeout */
		    debug(F101,"CONNECT select() timeout","",tt_idleact);
		    switch (tt_idleact) {
		      case IDLE_HANG: {	/* Hang up */
			  int x = 0;
#ifndef NODIAL
			  if (dialmhu)
			    x = mdmhup();
			  if (x < 1)
#endif /* NODIAL */
			    tthang();	/* fall thru deliberately... */
		      }
		      case IDLE_RET:	/* Return to command mode */
			cx_status = CSX_IDLE;
			active = 0;
			continue;
		      case IDLE_OUT:	/* OUTPUT a string */
			if (tt_idlestr) {
			    int len = strlen(tt_idlestr);
			    if (len > 0)
			      ttol((CHAR *)tt_idlestr,len);
			    else
			      ttoc(NUL); /* No string, send a NUL */
			} else
			  ttoc(NUL);	/* No string, send a NUL */
			continue;
		      case IDLE_EXIT:	/* Exit from Kermit */
			doexit(GOOD_EXIT,xitsta);
#ifdef TNCODE
		      case IDLE_TAYT:	/* Send Telnet Are You There? */
			if (network && IS_TELNET()) {
			    tnopt[0] = (CHAR) IAC;
			    tnopt[1] = (CHAR) TN_AYT;
			    tnopt[2] = NUL;
			    if (ttol((CHAR *)tnopt,2) < 0)
			      active = 0;
			}
			continue;

		      case IDLE_TNOP:	/* Send Telnet NOP */
			if (network && IS_TELNET()) {
			    tnopt[0] = (CHAR) IAC;
			    tnopt[1] = (CHAR) TN_NOP;
			    tnopt[2] = NUL;
			    if (ttol((CHAR *)tnopt,2) < 0)
			      active = 0;
			}
			continue;
#endif /* TNCODE */
		    }
		}
#endif /* CKTIDLE */

		debug(F101,"CONNECT select() errno","",errno);
		/* A too-big first arg to select() gets EBADF */
#ifdef EINTR
		if (c == -1) {
		    if (errno == EINTR) {
			continue;
		    }
		}
#endif /* EINTR */
		sleep(1);
		continue;
	    }
#ifndef BEBOX
#ifdef DEBUG
	    if (FD_ISSET(scrnout, &out)) {
		debug(F100,"CONNECT SELECT scrnout","",0);
	    }
#endif /* DEBUG */
#endif /* BEBOX */

#ifdef CK_FORWARD_X
            fwdx_check_sockets(&in);
#endif /* CK_FORWARD_X */

	    if (FD_ISSET(ttyfd, &in)) {	/* Read from net? */
		debug(F110,"CONNECT SELECT ttyfd","in",0);
		FD_CLR(ttyfd, &in);
		gotnet = 1;		/* Net is ready */
	    }
	    if (FD_ISSET(kbin, &in)) {	/* Read from keyboard? */
		debug(F100,"CONNECT SELECT kbin","",0);
		FD_CLR(kbin, &in);
		gotkbd = 1;		/* Keyboard is ready */
	    }
            if (FD_ISSET(ttyfd, &err)) {
		debug(F110,"CONNECT SELECT ttyfd","err",0);
		FD_CLR(ttyfd, &err);
#ifdef NETPTY
#ifdef HAVE_PTYTRAP
		/* Special handling for HP-UX pty i/o */
                if (ttpty) {
                    if (pty_trap_handler(ttyfd) > 0) {
                        ttclos(0);
                        goto conret1;
                    }
                    continue;
                }
#endif /* HAVE_PTYTRAP */
#endif /* NETPTY */
		gotnet = 1;		/* Net is ready (don't set if pty) */
            }
	}
#ifdef DEBUG
	if (deblog) {
	    debug(F101,"CONNECT gotkbd","",gotkbd);
	    debug(F101,"CONNECT kbc","",kbc);
#ifdef COMMENT
#ifndef NOSETKEY
	    debug(F101,"CONNECT kmptr","",kmptr);
#endif /* NOSETKEY */
#endif	/* COMMENT */
	}
#endif /* DEBUG */

	while (gotkbd || kbc > 0	/* If we have keyboard chars */
#ifndef NOSETKEY
	       || kmptr
#endif /* NOSETKEY */
	       ) {
#ifndef NOSETKEY
	    if (kmptr) {		/* Have current macro? */
		debug(F100,"CONNECT kmptr non NULL","",0);
		if ((c = (CHAR) *kmptr++) == NUL) { /* Get char from it */
		    debug(F100,"CONNECT macro empty, continuing","",0);
		    kmptr = NULL;	/* If no more chars,  */
		    continue;		/* Reset pointer and continue */
		}
		debug(F000,"CONNECT char from macro","",c);
	    } else {			/* No macro... */
#endif /* NOSETKEY */
#ifdef BEBOX
		{
		    int rc = 0;
		    if ((rc = recv(kbin,buf,1,0)) > 0)
		      c = buf[0];
		    else
		      c = -1;
		    debug(F111,"recv","rc",rc);
		    printf("\r\nrecv: %c rc=%d\r\n",buf[0],rc);
		}
#else /* BEBOX */
		c = CONGKS();		/* Yes, read from keyboard */
#endif /* BEBOX */
		gotkbd = 0;		/* Turn off select() result flag */
#ifndef NOSETKEY
	    }
#endif /* NOSETKEY */
	    if (c == -1) {
#ifdef EINTR
		if (errno == EINTR)
		  continue;
#endif /* EINTR */
		cx_status = CSX_IOERROR;
		conoc(BEL);
		goto conret0;
	    }
	    c &= cmdmsk;		/* Do any requested masking */

#ifndef NOSETKEY
/*
  Note: kmptr is NULL if we got character c from the keyboard, and it is
  not NULL if it came from a macro.  In the latter case, we must avoid
  expanding it again.
*/
	    if (!kmptr && macrotab[c]) { /* Macro definition for c? */
		debug(F000,"CONNECT macro key",macrotab[c],c);
		kmptr = macrotab[c];	/* Yes, set up macro pointer */
		continue;		/* and restart the loop, */
	    } else c = keymap[c];	/* else use single-char keymap */
#endif /* NOSETKEY */
	    if (
#ifndef NOSETKEY
		!kmptr &&
#endif /* NOSETKEY */
		(tt_escape && ((c & 0xff) == escape))) { /* Escape char? */
		debug(F000,"CONNECT got escape","",c);
#ifdef BEBOX
		if (recv(kbin,buf,1,0)>=0)
		  c = buf[0];
		else
		  c = -1;
#else /* BEBOX */
		c = CONGKS() & 0x7f;	/* Read argument */
#endif /* BEBOX */
		doesc((char) c);	/* Handle it */
		continue;		/* Back to loop */
	    }
	    csave = c;			/* Save it before translation */
	    				/* for local echoing. */
#ifdef CKLEARN
	    crflag = (c == CR);		/* Remember if it was CR. */
#endif /* CKLEARN */

#ifndef NOCSETS
	    if (inesc[1] == ES_NORMAL) { /* If not inside escape seq.. */
		/* Translate character sets */
#ifdef UNICODE
		int x;
		if (unicode == 1) {	/* Remote is UTF-8 */
		    outxcount = b_to_u((CHAR)c,outxbuf,OUTXBUFSIZ,tcssize);
		    outxbuf[outxcount] = NUL;
		} else if (unicode == 2) { /* Local is UTF-8 */
		    
		    x = u_to_b((CHAR)c);
		    if (x < 0)
		      continue;
		    outxbuf[0] = (unsigned)(x & 0xff);
		    outxcount = 1;
		    outxbuf[outxcount] = NUL;
		} else {
#endif /* UNICODE */
		    if (sxo) c = (*sxo)((char)c); /* Local-intermediate */
		    if (rxo) c = (*rxo)((char)c); /* Intermediate-remote */
		    outxbuf[0] = c;
		    outxcount = 1;
		    outxbuf[outxcount] = NUL;
#ifdef UNICODE
		}
#endif /* UNICODE */
	    } else {
		outxbuf[0] = c;
		outxcount = 1;
		outxbuf[outxcount] = NUL;
	    }
	    if (escseq)
	      apcrc = chkaes((char)c,1);
#else  /* NOCSETS */
	    outxbuf[0] = c;
	    outxcount = 1;
	    outxbuf[outxcount] = NUL;
#endif /* NOCSETS */

	    debug(F111,"OUTXBUF",outxbuf,outxcount);

	    for (i = 0; i < outxcount; i++) {
		c = outxbuf[i];
/*
 If Shift-In/Shift-Out is selected and we have a 7-bit connection,
 handle shifting here.
*/
		if (sosi) {			 /* Shift-In/Out selected? */
		    if (cmask == 0177) {	 /* In 7-bit environment? */
			if (c & 0200) {		 /* 8-bit character? */
			    if (outshift == 0) { /* If not shifted, */
				ttoc(dopar(SO)); /* shift. */
				outshift = 1;
			    }
			} else {
			    if (outshift == 1) { /* 7-bit character */
				ttoc(dopar(SI)); /* If shifted, */
				outshift = 0;    /* unshift. */
			    }
			}
		    }
		    if (c == SO) outshift = 1; /* User typed SO */
		    if (c == SI) outshift = 0; /* User typed SI */
		}
		c &= cmask;		/* Apply Kermit-to-host mask now. */
		if (c == '\015') {	/* Carriage Return */
		    int stuff = -1;
		    if (tnlm) {		/* TERMINAL NEWLINE ON */
			stuff = LF; 	/* Stuff LF */
#ifdef TNCODE
		    } else if (network && /* TELNET NEWLINE ON/OFF/RAW */
			       IS_TELNET()) {
			switch (!TELOPT_ME(TELOPT_BINARY) ? tn_nlm : tn_b_nlm){
			  case TNL_CRLF:
			    stuff = LF;
			    break;
			  case TNL_CRNUL:
			    stuff = NUL;
			    break;
			}
#endif /* TNCODE */
		    }
		    if (stuff > -1) {
			ttoc(dopar('\015'));	/* Send CR */
			if (duplex) conoc('\015'); /* Maybe echo CR */
			c = stuff;	/* Char to stuff */
			csave = c;
		    }
		}
#ifdef TNCODE
/* If user types the 0xff character (TELNET IAC), it must be doubled. */
		else		/* Not CR */
		  if ((dopar((CHAR) c) == IAC) && /* IAC (0xff) */
		      network && IS_TELNET()) { /* Send one now */
		      ttoc((char)IAC); /* and the other one just below. */
		  }
#endif /* TNCODE */
		/* Send the character */

		x = ttoc((char)dopar((CHAR) c));
		if (x > -1) {
#ifdef CKLEARN
		    if (learning) {	/* Learned script active */
			if (crflag) {	/* User typed CR */
			    learnchar(CR); /* Handle CR */
			    learnst = 0;   /* Shift to Neutral */
			} else {
			    learnchar(c);  /* Not CR */
			    learnst = 2;   /* Change state to Keyboard */
			}
		    }
#endif /* CKLEARN */
		    if (duplex) {	/* If half duplex, must echo */
			if (debses)
			  conol(dbchr(csave)); /* the original char */
			else		/* not the translated one */
			  conoc((char)csave);
			if (seslog) {	/* And maybe log it too */
			    c2 = csave;
			    if (sessft == 0 && csave == '\r')
			      c2 = '\n';
			    LOGCHAR((char)c2);
			}
		    }
		} else {
		    perror("\r\nCan't send character");
		    cx_status = CSX_IOERROR;
		    active = 0;
		    break;
		}
	    }
	}
	if (FD_ISSET(ttyfd, &out)) {
	    FD_CLR(ttyfd, &out);
	}
	while (gotnet > 0 || ibc > 0) {
	    gotnet = 0;
	    prev = c;
	    c = ckcgetc(0);		/* Get next character */
	    if (c < 0) {		/* Failed... */
		ckcputf();		/* Flush CONNECT output buffer */
		if (msgflg) {
		    printf("\r\nCommunications disconnect ");
#ifdef COMMENT
		    if (c == -3
#ifdef ultrix
/* This happens on Ultrix if there's no carrier */
			&& errno != EIO
#endif /* ultrix */
#ifdef UTEK
/* This happens on UTEK if there's no carrier */
			&& errno != EWOULDBLOCK
#endif /* UTEK */
			)
		      perror("\r\nCan't read character");
#endif /* COMMENT */
		}
#ifdef NOSETBUF
		fflush(stdout);
#endif /* NOSETBUF */
		dologend();
		tthang();		/* Hang up the connection */
		debug(F111,"CONNECT i/o error 1",ck_errstr(),errno);
		cx_status = CSX_HOSTDISC;
		goto conret0;
	    }
#ifdef TNCODE
	    tx = 0;
	    if ((c == NUL) && network && IS_TELNET()) {
		if (prev == CR) {    /* Discard <NUL> of <CR><NUL> if peer */
		    if (!TELOPT_U(TELOPT_BINARY)) {  /* not in binary mode */
			debug(F111,"CONNECT NUL",ckitoa(prev),c);
			ckcputf();	/* Flush screen output buffer */
			break;
		    }
		}
	    }
	    debug(F111,"CONNECT","c",c);
	    debug(F111,"CONNECT","network",network);
	    debug(F111,"CONNECT","IS_TELNET",IS_TELNET());
	    if ((c == IAC) && network && IS_TELNET()) {
#ifdef CK_ENCRYPTION
		int x_auth = TELOPT_ME(TELOPT_AUTHENTICATION);
#else
		int x_auth = 0;
#endif /* CK_ENCRYPTION */
		int me_bin = TELOPT_ME(TELOPT_BINARY);
		int u_bin = TELOPT_U(TELOPT_BINARY);
		debug(F100,"CONNECT got IAC","",0);
		ckcputf();		/* Dump screen-output buffer */
		if ((tx = tn_doop((CHAR)(c & 0xff),duplex,ckcgetc)) == 0) {
		    if (me_bin != TELOPT_ME(TELOPT_BINARY)) {
			me_bin = TELOPT_ME(TELOPT_BINARY);
		    } else if (u_bin != TELOPT_U(TELOPT_BINARY)) {
			u_bin = TELOPT_U(TELOPT_BINARY);
#ifdef CK_ENCRYPTION
/*
  Here we have to push back any bytes we have read using block reads, so we
  can read them again using single-character reads, so they can be decrypted
  in case there was a switch to encryption in the block.  Note that we can't
  handle switches in the encryption state itself this way -- which would be
  nice, since it would eliminate the need for single-character reads.  Why?
  Because if a series of characters has already been decrypted that shouldn't
  have been, then (a) it's ruined, and (b) so is the state of the decryption
  machine.  Too bad.
*/
		    } else if (TELOPT_ME(TELOPT_AUTHENTICATION) != 0 &&
			       TELOPT_ME(TELOPT_AUTHENTICATION) != x_auth
			       ) {
			if (ttpushback((CHAR *)ibp,ibc) > -1) {
			    ibc = 0;
			    ibp = ibuf;
			}
#endif /* CK_ENCRYPTION */
		    }
		    continue;
		} else if (tx == -1) {	/* I/O error */
		    if (msgflg)
		      printf("\r\nCommunications disconnect ");
#ifdef NOSETBUF
		    fflush(stdout);
#endif /* NOSETBUF */
		    dologend();
		    debug(F111,"CONNECT i/o error 2",ck_errstr(),errno);
		    cx_status = CSX_IOERROR;
		    goto conret0;
		} else if (tx == -2) {	/* I/O error */
		    if (msgflg)
		      printf("\r\nConnection closed by peer");
#ifdef NOSETBUF
		    fflush(stdout);
#endif /* NOSETBUF */
		    dologend();
		    debug(F111,"CONNECT i/o error 3",ck_errstr(),errno);
		    cx_status = CSX_IOERROR;
		    goto conret0;
		} else if (tx == -3) {	/* I/O error */
		    if (msgflg)
		      printf("\r\nConnection closed due to telnet policy");
#ifdef NOSETBUF
		    fflush(stdout);
#endif /* NOSETBUF */
		    dologend();
		    debug(F111,"CONNECT i/o error 4",ck_errstr(),errno);
		    cx_status = CSX_IOERROR;
		    goto conret0;
		} else if ((tx == 1) && (!duplex)) { /* ECHO change */
		    duplex = 1;		/* Turn on local echo */
		    continue;
		} else if ((tx == 2) && (duplex)) { /* ECHO change */
		    duplex = 0;
		    continue;
		} else if (tx == 3) {	/* Quoted IAC */
		    c = parity ? 127 : 255;
		}
#ifdef IKS_OPTION
                else if (tx == 4) {   /* IKS State Change */
                    if (TELOPT_SB(TELOPT_KERMIT).kermit.u_start &&
			!tcp_incoming
			) {
                        /* here we need to print a msg that the other */
                        /* side is in SERVER mode and that REMOTE     */
                        /* commands should be used.  And CONNECT mode */
                        /* should be ended.                           */
			cx_status = CSX_IKSD;
			active = 0;
                    }
                }
#endif /* IKS_OPTION */
                else if (tx == 6) {
                    /* DO LOGOUT was received */
		    if (msgflg)
		      printf("\r\nRemote Logout ");
#ifdef NOSETBUF
		    fflush(stdout);
#endif /* NOSETBUF */
		    debug(F100,"CONNECT Remote Logout","",0);
		    cx_status = CSX_TRIGGER;
		    goto conret0;
                } else
		  continue;		/* Negotiation OK, get next char. */
	    } else if (parity)
	      c &= 0x7f;

	    /* I'm echoing for the remote */
            if (TELOPT_ME(TELOPT_ECHO) && tn_rem_echo)
	      ttoc((char)c);
#endif /* TNCODE */

#ifdef CKLEARN
	 /* Learned script: Record incoming chars if not in Keyboard state */

	    if (learning && learnst != 2) { /* Learned script active */
		learnbuf[learnbp++] = c;    /* Save for INPUT command */
		if (learnbp >= LEARNBUFSIZ) /* in circular buffer */
		  learnbp = 0;              /* wrapping if at end. */
		learnbc++;                  /* Count this byte. */
		learnst = 1;                /* State is Net. */
	    }
#endif /* CKLEARN */

	    if (debses) {		/* Output character to screen */
		char *s;		/* Debugging display... */
		s = dbchr(c);		/* Make char into string */
		while (*s) {		/* Output each char from string */
		    ckcputc(*s);
		    if (seslog)		/* And maybe log it. */
		      LOGCHAR((char)*s);
		    s++;
		}
	    } else {			/* Regular display ... */
		c &= cmask;		/* Apply Kermit-to-remote mask */
		if (seslog && sessft)	/* If binary session log */
		  LOGCHAR((char)c);	/* log the character now. */
#ifndef NOXFER
#ifdef CK_AUTODL
/*
  Autodownload.  Check for Kermit S packet prior to translation, since that
  can change the packet and make it unrecognizable (as when the terminal
  character set is an ISO 646 one)...  Ditto for Zmodem start packet.
*/
		if (autodl		/* Autodownload enabled? */
#ifdef IKS_OPTION
		    || TELOPT_SB(TELOPT_KERMIT).kermit.me_start
#endif /* IKS_OPTION */
		    ) {
		    int k = 0;

		    if (kstartactive || c == stchr /* Kermit S or I packet? */
#ifdef COMMENT
			|| adl_kmode == ADLSTR /* Not used in C-Kermit */
#endif /* COMMENT */
			)
		      k = kstart((CHAR)c);
#ifdef CK_XYZ
		    if (!k && zmdlok)	/* Or an "sz" start? */
		      k = zstart((CHAR)c);
#endif /* CK_XYZ */
		    if (k) {
			int ksign = 0;
			debug(F101,"CONNECT autodownload k","",k);
			if (k < 0) { /* Minus-Protocol? */
#ifdef NOSERVER
			    goto noserver; /* Need server mode for this */
#else
			    ksign = 1; /* Remember */
			    k = 0 - k; /* Convert to actual protocol */
			    justone = 1; /* Flag for protocol module */
#endif /* NOSERVER */
			} else
			  justone = 0;
			k--;		/* Adjust [kz]start's return value */
			if (k == PROTO_K
#ifdef CK_XYZ
			    || k == PROTO_Z
#endif /* CK_XYZ */
			    ) {
                            /* Damage the packet so that it doesn't trigger */
			    /* autodownload detection downstream. */
                            if (k == PROTO_K) {
                                int i, len = strlen((char *)ksbuf);
                                for (i = 0; i < len; i++)
				  ckcputc(BS);
                            }
#ifdef CK_XYZ
                            else {
                                int i;
                                for (i = 0; i < 3; i++)
				  ckcputc(CAN);
                            }
#endif /* CK_XYZ */

#ifndef NOICP
			    /* sprintf is safe here (builtin keywords) */
			    sprintf(apcbuf,
				    "set proto %s, %s, set proto %s",
				    ptab[k].p_name,
				    ksign ? "server" : "receive",
				    ptab[protocol].p_name
				    );
			    apclength = strlen(apcbuf);
			    debug(F111,"CONNECT ksbuf",ksbuf,k);
			    debug(F110,"CONNECT autodownload",apcbuf,0);
			    apcactive = APC_LOCAL;
			    ckcputf();	/* Force screen update */
			    cx_status = CSX_APC;
			    goto conret1;
#else
/*
  Here's another way that doesn't require APC, but then we'll have to change
  all the other CONNECT modules, and then the mainline code that calls them.
*/
			    {
				extern char sstate;
				sstate = ksign ? 'x' : 'v';
				proto();
			    }
#endif /* NOICP */
			}
		    }
		}
#ifdef NOSERVER
	      noserver:
#endif /* NOSERVER */

#endif /* CK_AUTODL */
#endif /* NOXFER */
		if (sosi) {		/* Handle SI/SO */
		    if (c == SO) {	/* Shift Out */
			inshift = 1;
			continue;
		    } else if (c == SI) { /* Shift In */
			inshift = 0;
			continue;
		    }
		    if (inshift) c |= 0200;
		}
		inxbuf[0] = c;		/* In case there is no translation */
		inxcount = 1;		/* ... */
#ifndef NOCSETS
		if (inesc[0] == ES_NORMAL /* If not in an escape sequence */
		    && !printing	/* and not in transparent print */
		    ) {			/* Translate character sets */
#ifdef UNICODE
		    int x;
		    if (unicode == 1) {	/* Remote is UTF-8 */
			x = u_to_b((CHAR)c);
			if (x == -1)
			  continue;
			else if (x == -2) { /* LS or PS */
			    inxbuf[0] = CR;
			    inxbuf[1] = LF;
			    inxcount = 2;
			} else if (x == -9) { /* UTF-8 error */
			    inxbuf[0] = '?';
			    inxbuf[1] = u_to_b2();
			    inxcount = 2;
			} else {
			    inxbuf[0] = (unsigned)(x & 0xff);
			}
			c = inxbuf[0];
		    } else if (unicode == 2) { /* Local is UTF-8 */
			inxcount = b_to_u((CHAR)c,inxbuf,OUTXBUFSIZ,tcssize);
			c = inxbuf[0];
		    } else {
#endif /* UNICODE */
			if (sxi) c = (*sxi)((CHAR)c);
			if (rxi) c = (*rxi)((CHAR)c);
			inxbuf[0] = c;
#ifdef UNICODE
		    }
#endif /* UNICODE */
		}
#endif /* NOCSETS */

#ifndef NOESCSEQ
		if (escseq) {		/* If handling escape sequences */
		    oldprt = printing;	     /* remember printer state */
		    apcrc = chkaes((char)c,0); /* and update escseq state. */
		    if (printing && !oldprt) /* If printer was turned on */
		      continue;		/* don't print final char of escseq */
		}
#ifdef CK_APC
/*
  If we are handling APCs, we have several possibilities at this point:
   1. Ordinary character to be written to the screen.
   2. An Esc; we can't write it because it might be the beginning of an APC.
   3. The character following an Esc, in which case we write Esc, then char,
      but only if we have not just entered an APC sequence.
*/
		if (escseq && (apcstatus & APC_ON)) {
		    if (inesc[0] == ES_GOTESC) /* Don't write ESC yet */
		      continue;
		    else if (oldesc[0] == ES_GOTESC && !apcactive) {
			ckcputc(ESC);	/* Write saved ESC */
			if (seslog && !sessft) LOGCHAR((char)ESC);
		    } else if (apcrc) {	/* We have an APC */
			debug(F111,"CONNECT APC complete",apcbuf,apclength);
			ckcputf();	/* Force screen update */
			cx_status = CSX_APC;
			goto conret1;
		    }
		}
#endif /* CK_APC */
#endif /* NOESCSEQ */

		debug(F111,"INXBUF",inxbuf,inxcount);
		for (i = 0; i < inxcount; i++) { /* Loop thru */
		    c = inxbuf[i];	/* input expansion buffer... */
		    if (
#ifdef CK_APC
			!apcactive &&	/* Don't display APC sequences */
#endif /* CK_APC */
			!printing	/* or transparent print material */

			) {
			c &= cmdmsk;	/* Apply command mask. */

			/* Handle bare carriage returns and linefeeds */

			if (c == CR && tt_crd) { /* SET TERM CR-DISPLA CRLF? */
			    ckcputc(c);	/* Yes, output CR */
			    if (seslog && !sessft) LOGCHAR((char)c);
			    c = LF;	/* and insert a linefeed */
			}
			if (c == LF && tt_lfd) { /* SET TERM CR-DISPLA CRLF? */
			    ckcputc(CR); /* Yes, output CR */
			    if (seslog && !sessft) LOGCHAR((char)CR);
			}
#ifndef NOESCSEQ
			if (dontprint)	{ /* Do transparent printing. */
			    dontprint = 0;
			    continue;
			} else
#endif	/* NOESCSEQ */
			ckcputc(c);	/* Write character to screen */
		    }
		    if (seslog && !sessft) { /* Handle session log. */
			LOGCHAR((char)c);
		    }
#ifdef XPRINT
		    if (printing && !inesc[0]) {
			/* zchout() can't be used because */
			/* it's buffered differently. */
			cbuf[0] = c;
			zsoutx(ZMFILE,(char *)cbuf,1);
		    }
#endif /* XPRINT */

#ifdef CK_TRIGGER
		    /* Check for trigger string */
		    if (tt_trigger[0]) {
			int i;
			if ((i = autoexitchk((CHAR)c)) > -1) {
			    makestr(&triggerval,tt_trigger[i]);
			    ckcputf();	/* Force screen update */
#ifdef NOSETBUF
			    fflush(stdout); /* I mean really force it */
#endif /* NOSETBUF */
			    cx_status = CSX_TRIGGER;
			    goto conret1;
			}
		    }
#endif /* CK_TRIGGER */
		}
	    }
	}
#ifndef BEBOX
	if (FD_ISSET(scrnout, &out)) {
	    FD_CLR(scrnout, &out);
	}
#endif /* BEBOX */
    } /* End of big loop */
  conret1:				/* Come here to succeed */
    rc = 1;
  conret0:				/* Common exit point */
#ifdef BEBOX
    {
	long ret_val;
	closesocket(pair[0]);
	closesocket(pair[1]);
	x = kill(tid,SIGKILLTHR);	/* Kill thread */
	wait_for_thread (tid, &ret_val);
    }
#endif /* BEBOX */

#ifdef CKLEARN
    if (learning && learnfp)
      fputs("\n",learnfp);
#endif /* CKLEARN */

    conres();
    if (dohangup > 0) {
#ifdef NETCONN
	if (network
#ifdef TNCODE
	    && !TELOPT_ME(TELOPT_COMPORT)
#endif /* TNCODE */
	    )
	  ttclos(0);
#endif /* NETCONN */

#ifndef COMMENT
/*
  This is bad because if they said SET MODEM HANGUP-METHOD MODEM-COMMAND,
  they mean it -- we shouldn't fall back on tthang() if mdmhup() fails,
  because maybe they have some special kind of connection.  On the other
  hand, making this change prevents dialing from working at all in some
  cases.  Further study needed.
*/
#ifndef NODIAL
	if (dohangup > 1)		/* User asked for it */
	  if (mdmhup() < 1)		/* Maybe hang up via modem */
#endif /* NODIAL */
	    tthang();			/* And make sure we don't hang up */
#else
	if (!network) {			/* Serial connection. */
#ifndef NODIAL
	    if (dialmhu)		/* Hang up the way they said to. */
	      mdmhup();
	    else
#endif /* NODIAL */
	      tthang();
	}
#endif /* COMMENT */
	dologend();
	dohangup = 0;			/* again unless requested again. */
    }
    if (quitnow)			/* Exit now if requested. */
      doexit(GOOD_EXIT,xitsta);
    if (msgflg
#ifdef CK_APC
	&& !apcactive
#endif /* CK_APC */
	)
      printf("(Back at %s)", *myhost ? myhost : "local UNIX system");
#ifdef CK_APC
    if (!apcactive)
#endif /* CK_APC */
      printf("\n");
    what = W_NOTHING;			/* So console modes set right. */
#ifndef NOCSETS
    language = langsv;			/* Restore language */
#endif /* NOCSETS */
#ifdef CK_APC
    debug(F101,"CONNECT exit apcactive","",apcactive);
    debug(F101,"CONNECT exit justone","",justone);
#endif /* CK_APC */
    if (msgflg) {
#ifdef CK_APC
	if (apcactive == APC_LOCAL)
	  printf("\n");
#endif /* CK_APC */
	printf("----------------------------------------------------\n");
	printbar = 1;
    } else
	printbar = 0;
    fflush(stdout);
    return(rc);
}

/*  H C O N N E  --  Give help message for connect.  */

#define CXM_SER 1			/* Serial connections only */
#define CXM_NET 2			/* Network only (but not Telnet) */
#define CXM_TEL 4			/* Telnet only */

static struct hmsgtab {
    char * hmsg;
    int hflags;
} hlpmsg[] = {
    {"  ? or H for this message",                0},
    {"  0 (zero) to send the NUL (0) character", 0},
    {"  B to send a BREAK signal (0.275sec)",  CXM_SER},
#ifdef NETCONN
    {"  B to send a network BREAK",            CXM_NET},
    {"  B to send a Telnet BREAK",             CXM_TEL},
#endif /* NETCONN */
#ifdef CK_LBRK
    {"  L to send a Long BREAK (1.5sec)",      CXM_SER},
#endif /* CK_LBRK */
#ifdef NETCONN
    {"  I to send a network interrupt packet", CXM_NET},
    {"  I to send a Telnet Interrupt request", CXM_TEL},
#ifdef TNCODE
    {"  A to send Telnet Are-You-There?",      CXM_TEL},
#endif /* TNCODE */
#endif /* NETCONN */
    {"  U to hangup and close the connection", 0},
    {"  Q to hangup and quit Kermit",          0},
    {"  S for status",                         0},
#ifdef NOPUSH
    {"  ! to push to local shell (disabled)",  0},
    {"  Z to suspend (disabled)",              0},
#else
    {"  ! to push to local shell",             0},
#ifdef NOJC
    {"  Z to suspend (disabled)",              0},
#else
    {"  Z to suspend",                         0},
#endif /* NOJC */
#endif /* NOPUSH */
    {"  \\ backslash code:",                   0},
    {"    \\nnn  decimal character code",      0},
    {"    \\Onnn octal character code",        0},
    {"    \\Xhh  hexadecimal character code;", 0},
    {"    terminate with Carriage Return.",    0},
    {"  Type the escape character again to send the escape character itself,",
       0},
    {"  or press the space-bar to resume the CONNECT session.", 0},
    {NULL, 0}
};

int
hconne() {
    int c, i, cxtype;
    if (network)
      cxtype = IS_TELNET() ? CXM_TEL : CXM_NET;
    else
      cxtype = CXM_SER;

    conol("\r\n----------------------------------------------------\r\n");
    conoll("Press:");
    conol("  C to return to ");
    conoll(*myhost ? myhost : "the C-Kermit prompt");
    for (i = 0; hlpmsg[i].hmsg; i++) {
	if (!(hlpmsg[i].hflags) || (hlpmsg[i].hflags == cxtype))
	  conoll(hlpmsg[i].hmsg);
    }
    conol("Press a key>");		/* Prompt for command. */
    c = CONGKS() & 0177;		/* Get character, strip any parity. */
    /* No key mapping or translation here */
    if (c != CMDQ)
      conoll("");
    conoll("----------------------------------------------------");
    return(c);				/* Return it. */
}


/*  D O E S C  --  Process an escape character argument  */

VOID
#ifdef CK_ANSIC
doesc(char c)
#else
doesc(c) char c;
#endif /* CK_ANSIC */
/* doesc */ {
    CHAR d;

    debug(F101,"CONNECT doesc","",c);
    while (1) {
	if (c == escape) {		/* Send escape character */
	    d = dopar((CHAR) c); ttoc((char) d); return;
    	} else				/* Or else look it up below. */
	    if (isupper(c)) c = tolower(c);

	switch(c) {

	  case 'c':			/* Escape back to prompt */
	  case '\03':
	    cx_status = CSX_ESCAPE;
#ifdef NOICP
	    conoll("");
	    conoll("");
	    conoll(
"  WARNING: This version of C-Kermit has no command processor to escape"
		   );
	    conoll(
"  back to.  To return to your local system, log out from the remote and/or"
		   );
	    conoll(
"  use the escape character followed by the letter U to close (hang Up) the"
		   );
	    conoll(
"  connection.  Resuming your session..."
		   );
	    conoll("");
	    return;
#else
	    active = 0; conol("\r\n"); return;
#endif /* NOICP */

	  case 'b':			/* Send a BREAK signal */
	  case '\02':
#ifdef CKLEARN
	    learnchar(-7);
#endif /* CKLEARN */
	    ttsndb(); return;

#ifdef NETCONN
	  case 'i':			/* Send Interrupt */
	  case '\011':
#ifdef TCPSOCKET
	    if (network && IS_TELNET()) { /* TELNET */
		temp[0] = (CHAR) IAC;	/* I Am a Command */
		temp[1] = (CHAR) TN_IP;	/* Interrupt Process */
		temp[2] = NUL;
		ttol((CHAR *)temp,2);
	    } else
#endif /* TCPSOCKET */
	      conoc(BEL);
	    return;

#ifdef TCPSOCKET
	  case 'a':			/* "Are You There?" */
	  case '\01':
	    if (network && IS_TELNET()) {
		temp[0] = (CHAR) IAC;	/* I Am a Command */
		temp[1] = (CHAR) TN_AYT; /* Are You There? */
		temp[2] = NUL;
		ttol((CHAR *)temp,2);
	    } else conoc(BEL);
	    return;
#endif /* TCPSOCKET */
#endif /* NETCONN */

#ifdef CK_LBRK
	  case 'l':			/* Send a Long BREAK signal */
#ifdef CKLEARN
	    learnchar(-8);
#endif /* CKLEARN */
	    ttsndlb(); return;
#endif /* CK_LBRK */

	  case 'u':			/* Hangup */
       /* case '\010': */		/* No, too dangerous */
	    cx_status = CSX_USERDISC;
	    dohangup = 2; active = 0; conol("\r\nHanging up "); return;

	  case 'q':			/* Quit */
	    cx_status = CSX_USERDISC;
	    dohangup = 2; quitnow = 1; active = 0; conol("\r\n"); return;

	  case 's':			/* Status */
	    conoll("");
	    conoll("----------------------------------------------------");
#ifdef PTYORPIPE
	    if (ttpipe)
	      ckmakmsg(temp,TMPLEN," Pipe: \"",ttname,"\"",NULL);
	    else if (ttpty)
	      ckmakmsg(temp,TMPLEN," Pty: \"",ttname,"\"",NULL);
	    else
#endif /* PTYORPIPE */
	      ckmakmsg(temp,
		       TMPLEN,
		       " ",
		       (network ? "Host" : "Device"),
		       ": ",
		       ttname
		       );
	    conoll(temp);

	    /* The following sprintf's are safe, temp[] size is at least 200 */

	    if (!network && speed >= 0L) {
		sprintf(temp,"Speed %ld", speed);
		conoll(temp);
	    }
	    sprintf(temp," Terminal echo: %s", duplex ? "local" : "remote");
	    conoll(temp);
	    sprintf(temp," Terminal bytesize: %d", (cmask == 0177) ? 7 : 8);
	    conoll(temp);
	    sprintf(temp," Command bytesize: %d", (cmdmsk == 0177) ? 7 : 8);
	    conoll(temp);
            if (hwparity)
              sprintf(temp," Parity[hardware]: %s",parnam(hwparity));
            else	    
              sprintf(temp," Parity: %s", parnam(parity));
	    conoll(temp);
#ifndef NOXFER
	    sprintf(temp," Autodownload: %s", autodl ? "on" : "off");
	    conoll(temp);
#endif /* NOXFER */
	    ckmakmsg(temp,		/* (would not be safe for sprintf) */
		     TMPLEN,
		     " Session log: ",
		     *sesfil ? sesfil : "(none)",
		     NULL,
		     NULL
		     );
	    conoll(temp);
#ifndef NOSHOW
	    if (!network) shomdm();
#endif /* NOSHOW */
#ifdef CKLOGDIAL
	    {
		long z;
		z = dologshow(0);
		if (z > -1L) {
		    sprintf(temp," Elapsed time: %s",hhmmss(z));
		    conoll(temp);
		}
	    }
#endif /* CKLOGDIAL */
	    conoll("----------------------------------------------------");
	    return;

	  case 'h':			/* Help */
	  case '?':			/* Help */
	    c = hconne(); continue;

	  case '0':			/* Send a null */
	    c = '\0'; d = dopar((CHAR) c); ttoc((char) d); return;

	  case 'z': case '\032':	/* Suspend */
#ifndef NOPUSH
	    if (!nopush)
	      stptrap(0);
	    else
	      conoc(BEL);
#else
	    conoc(BEL);
#endif /* NOPUSH */
	    return;

	  case '@':			/* Start inferior command processor */
	  case '!':
#ifndef NOPUSH
	    if (!nopush) {
		conres();		      /* Put console back to normal */
		zshcmd("");		      /* Fork a shell. */
		if (conbin((char)escape) < 0) {
		    cx_status = CSX_INTERNAL;
		    printf("Error resuming CONNECT session\n");
		    active = 0;
		}
	    } else conoc(BEL);
#else
	    conoc(BEL);
#endif /* NOPUSH */
	    return;

	  case SP:			/* Space, ignore */
	    return;

	  default:			/* Other */
	    if (c == CMDQ) {		/* Backslash escape */
		int x;
		ecbp = ecbuf;
		*ecbp++ = c;
		while (((c = (CONGKS() & cmdmsk)) != '\r') && (c != '\n'))
		  *ecbp++ = c;
		*ecbp = NUL; ecbp = ecbuf;
		x = xxesc(&ecbp);	/* Interpret it */
		if (x >= 0) {		/* No key mapping here */
		    c = dopar((CHAR) x);
		    ttoc((char) c);
		    return;
		} else {		/* Invalid backslash code. */
		    conoc(BEL);
		    return;
		}
	    }
	    conoc(BEL); return; 	/* Invalid esc arg, beep */
    	}
    }
}
#endif /* NOLOCAL */

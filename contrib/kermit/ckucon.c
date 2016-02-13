#include "ckcsym.h"

char *connv = "CONNECT Command for UNIX:fork(), 9.0.117, 14 Jul 2011";

/*  C K U C O N  --  Terminal connection to remote system, for UNIX  */
/*
  Author: Frank da Cruz <fdc@columbia.edu>,
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 2011,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.

  NOTE: This module has been superseded on most platforms by ckucns.c, which
  uses select() rather than fork() for multiplexing its i/o.  This module
  is still needed for platforms that do not support select(), and also for
  its X.25 support.  Although the two modules share large amounts of code,
  their structure is radically different and therefore attempts at merging
  them have so far been unsuccessful.  (November 1998.)

  Special thanks to Eduard Vopicka, Prague University of Economics,
  Czech Republic, for valuable contributions to this module in July 1994,
  and to Neal P. Murphy of the Motorola Cellular Infrastructure Group in 1996
  for rearranging the code to allow operation on the BeBox, yet still work
  in regular UNIX.
*/
#include "ckcdeb.h"			/* Common things first */

#ifndef NOLOCAL

#ifdef BEOSORBEBOX
static double time_started = 0.0;
#include <kernel/OS.h>
_PROTOTYP( static long concld, (void *) );
#else
_PROTOTYP( static VOID concld, (void) );
#endif /* BEOSORBEBOX */

#ifdef NEXT
#undef NSIG
#include <sys/wait.h>			/* For wait() */
#endif /* NEXT */

#include <signal.h>			/* Signals */

#ifndef HPUXPRE65
#include <errno.h>			/* Error number symbols */
#else
#ifndef ERRNO_INCLUDED
#include <errno.h>			/* Error number symbols */
#endif	/* ERRNO_INCLUDED */
#endif	/* HPUXPRE65 */

#ifdef ZILOG				/* Longjumps */
#include <setret.h>
#else
#include <setjmp.h>
#endif /* ZILOG */
#include "ckcsig.h"

/* Kermit-specific includes */

#include "ckcasc.h"			/* ASCII characters */
#include "ckcker.h"			/* Kermit things */
#include "ckucmd.h"			/* For xxesc() prototype */
#include "ckcnet.h"			/* Network symbols */
#ifndef NOCSETS
#include "ckcxla.h"			/* Character set translation */
#endif /* NOCSETS */

/* Internal function prototypes */

_PROTOTYP( VOID ttflux, (void) );
_PROTOTYP( VOID doesc, (char) );
_PROTOTYP( VOID logchar, (char) );
_PROTOTYP( int hconne, (void) );
#ifndef NOSHOW
_PROTOTYP( VOID shomdm, (void) );
#endif /* NOSHOW */
_PROTOTYP( static int kbget, (void) );
_PROTOTYP( static int pipemsg, (int) );
_PROTOTYP( static int ckcputf, (void) );
_PROTOTYP( static VOID ck_sndmsg, (void) );
/*
  For inter-fork signaling.  Normally we use SIGUSR1, except on SCO, where
  we use SIGUSR2 because SIGUSR1 is used by the system.  You can define
  CK_FORK_SIG to be whatever other signal you might want to use at compile
  time.  We don't use SIGUSR2 everywhere because in some systems, like
  UnixWare, the default action for SIGUSR2 is to kill the process that gets it.
*/
#ifndef CK_FORK_SIG

#ifndef SIGUSR1				/* User-defined signals */
#define SIGUSR1 30
#endif /* SIGUSR1 */

#ifndef SIGUSR2
#define SIGUSR2 31
#endif /* SIGUSR2 */

#ifdef M_UNIX
#define CK_FORK_SIG SIGUSR2		/* SCO - use SIGUSR2 */
#else
#define CK_FORK_SIG SIGUSR1		/* Others - use SIGUSR1 */
#endif /* M_UNIX */

#endif /* CK_FORK_SIG */

/* External variables */

extern struct ck_p ptab[];

extern int local, escape, duplex, parity, flow, seslog, sessft, debses,
 mdmtyp, ttnproto, cmask, cmdmsk, network, nettype, deblog, sosi, tnlm,
 xitsta, what, ttyfd, ttpipe, quiet, backgrd, pflag, tt_crd, tt_lfd,
 tn_nlm, ttfdflg,
 tt_escape, justone, carrier, hwparity;

extern long speed;
extern char ttname[], sesfil[], myhost[], *ccntab[];
#ifdef TNCODE
extern int tn_b_nlm, tn_rem_echo;
#endif /* TNCODE */

#ifdef CK_TRIGGER
extern char * tt_trigger[], * triggerval;
#endif /* CK_TRIGGER */

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
extern int protocol;			/* Auto download */
#endif /* CK_APC */

extern int autodl;
#ifdef CK_AUTODL
extern CHAR ksbuf[];
#endif /* CK_AUTODL */

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
  quitnow = 0,				/* <esc-char>Q was typed */
  jbset = 0,				/* Flag whether jmp buf is set. */
  dohangup = 0,				/* <esc-char>H was typed */
  sjval,				/* Setjump return value */
  goterr = 0,				/* Fork/pipe creation error flag */
  inshift = 0,				/* SO/SI shift states */
  outshift = 0;

int active = 0;				/* Lower fork active flag */

static PID_T parent_id = (PID_T)0;	/* Process ID of keyboard fork */

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

/* X.25 items */

#ifdef ANYX25
static char *p;				/* General purpose pointer */
char x25ibuf[MAXIX25];			/* Input buffer */
char x25obuf[MAXOX25];			/* Output buffer */
int ibufl;				/* Length of input buffer */
int obufl;				/* Length of output buffer */
unsigned char tosend = 0;
int linkid, lcn;
static int dox25clr = 0;
#ifndef IBMX25
extern CHAR padparms[];
#endif /* IBMX25 */
#endif /* ANYX25 */

static int xpipe[2] = {-1, -1};	/* Pipe descriptor for child-parent messages */
static PID_T pid = (PID_T) 0;	/* Process ID of child */

/* Character-set items */

static int unicode = 0;

static int escseq = 0;			/* 1 = Recognizer is active */
int inesc = 0;				/* State of sequence recognizer */
int oldesc = -1;			/* Previous state of recognizer */

#define OUTXBUFSIZ 15
static CHAR inxbuf[OUTXBUFSIZ+1];	/* Host-to-screen expansion buffer */
static int inxcount = 0;		/* and count */
static CHAR outxbuf[OUTXBUFSIZ+1];	/* Keyboard-to-host expansion buf */
static int outxcount = 0;		/* and count */

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

/*
  We do not need to parse and recognize escape sequences if we are being built
  without character-set support AND without APC support.
*/
#ifdef NOCSETS				/* No character sets */
#ifndef CK_APC				/* No APC */
#ifndef NOESCSEQ
#define NOESCSEQ			/* So no escape sequence recognizer */
#endif /* NOESCSEQ */
#endif /* CK_APC */
#endif /* NOCSETS */

/* Child process events and messages */

#define CEV_NO  0			/* No event */
#define CEV_HUP 1			/* Communications hangup */
#define CEV_PAD 2			/* X.25 - change PAD parameters */
#define CEV_DUP 3			/* Toggle duplex */
#define CEV_APC 4			/* Execute APC */
#ifdef TNCODE
#define CEV_MEBIN 5			/* Change of me_binary */
#define CEV_UBIN 6			/* Change of u_binary */
#endif /* TNCODE */
#define CEV_ADL 7			/* Autodownload */
#define CEV_AUL 8			/* Autoupload */
#define CEV_TRI 9			/* Trigger string */

#ifdef NOESCSEQ
#define chkaes(x) 0
#else
/*
  As of edit 178, the CONNECT command skips past ANSI escape sequences to
  avoid translating the characters within them.  This allows the CONNECT
  command to work correctly with a host that uses a 7-bit ISO 646 national
  character set, in which characters like '[' would normally be translated
  into accented characters, ruining the terminal's interpretation (and
  generation) of escape sequences.

  As of edit 190, the CONNECT command responds to APC escape sequences
  (ESC _ text ESC \) if the user SETs TERMINAL APC ON or UNCHECKED, and the
  program was built with CK_APC defined.

  Non-ANSI/ISO-compliant escape sequences are not handled.
*/

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
/*
  chkaes() -- Check ANSI Escape Sequence.
  Call with EACH character in input stream.
  Sets global inesc variable according to escape sequence state.
  Returns 0 normally, 1 if an APC sequence is to be executed.
*/
int
#ifdef CK_ANSIC
chkaes(char c)
#else
chkaes(c) char c;
#endif /* CK_ANSIC */
/* chkaes */ {

    oldesc = inesc;			/* Remember previous state */
    if (c == CAN || c == SUB)		/* CAN and SUB cancel any sequence */
      inesc = ES_NORMAL;
    else				/* Otherwise */
      switch (inesc) {			/* enter state switcher */

	case ES_NORMAL:			/* NORMAL state */
	  if (c == ESC)			/* Got an ESC */
	    inesc = ES_GOTESC;		/* Change state to GOTESC */
	  break;			/* Otherwise stay in NORMAL state */

	case ES_GOTESC:			/* GOTESC state */
	  if (c == '[')			/* Left bracket after ESC is CSI */
	    inesc = ES_GOTCSI;		/* Change to GOTCSI state */
	  else if (c == 'P' || (c > 0134 && c < 0140)) { /* P, [, ^, or _ */
	      inesc = ES_STRING;	/* Switch to STRING-absorption state */
#ifdef CK_APC
	      if (c == '_' && pid == 0 && /* APC handled in child only */
		  (apcstatus & APC_ON)) { /* and only if not disabled. */
		  debug(F100,"CONNECT APC begin","",0);
		  apcactive = APC_REMOTE; /* Set APC-Active flag */
		  apclength = 0;	/* and reset APC buffer pointer */
	      }
#endif /* CK_APC */
	  } else if (c > 057 && c < 0177) /* Final character '0' thru '~' */
	    inesc = ES_NORMAL;		/* Back to normal */
	  else if (c != ESC)		/* ESC in an escape sequence... */
	    inesc = ES_ESCSEQ;		/* starts a new escape sequence */
	  break;			/* Intermediate or ignored ctrl char */

	case ES_ESCSEQ:			/* ESCSEQ -- in an escape sequence */
	  if (c > 057 && c < 0177)	/* Final character '0' thru '~' */
	    inesc = ES_NORMAL;		/* Return to NORMAL state. */
	  else if (c == ESC)		/* ESC ... */
	    inesc = ES_GOTESC;		/* starts a new escape sequence */
	  break;			/* Intermediate or ignored ctrl char */

	case ES_GOTCSI:			/* GOTCSI -- In a control sequence */
	  if (c > 077 && c < 0177)	/* Final character '@' thru '~' */
	    inesc = ES_NORMAL;		/* Return to NORMAL. */
	  else if (c == ESC)		/* ESC ... */
	    inesc = ES_GOTESC;		/* starts over. */
	  break;			/* Intermediate or ignored ctrl char */

	case ES_STRING:			/* Inside a string */
	  if (c == ESC)			/* ESC may be 1st char of terminator */
	    inesc = ES_TERMIN;		/* Go see. */
#ifdef CK_APC
	  else if (apcactive && (apclength < apcbuflen)) /* If in APC, */
	    apcbuf[apclength++] = c;	/* deposit this character. */
	  else {			/* Buffer overrun */
	      apcactive = 0;		/* Discard what we got */
	      apclength = 0;		/* and go back to normal */
	      apcbuf[0] = 0;		/* Not pretty, but what else */
	      inesc = ES_NORMAL;	/* can we do?  (ST might not come) */
	  }
#endif /* CK_APC */
	  break;			/* Absorb all other characters. */

	case ES_TERMIN:			/* May have a string terminator */
	  if (c == '\\') {		/* which must be backslash */
	      inesc = ES_NORMAL;	/* If so, back to NORMAL */
#ifdef CK_APC
	      if (apcactive) {		/* If it was an APC string, */
		  debug(F101,"CONNECT APC terminated","",c);
		  apcbuf[apclength] = NUL; /* terminate it and then ... */
		  return(1);
	      }
#endif /* CK_APC */
	  } else {			/* Otherwise */
	      inesc = ES_STRING;	/* Back to string absorption. */
#ifdef CK_APC
	      if (apcactive && (apclength+1 < apcbuflen)) { /* In APC string */
		  apcbuf[apclength++] = ESC; /* deposit the Esc character */
		  apcbuf[apclength++] = c;   /* and this character too */
	      }
#endif /* CK_APC */
	  }
      }
    return(0);
}
#endif /* NOESCSEQ */

/* Connect state parent/child communication signal handlers */

/* Routines used by the child process */

static int
pipemsg(n) int n; {			/* Send message ID to parent */
    int code = n & 255;
    return(write(xpipe[1], &code, sizeof(code)));
}

/* Environment pointer for CK_FORK_SIG signal handling in child... */

#ifdef CK_POSIX_SIG
static sigjmp_buf sig_env;
#else
static jmp_buf sig_env;
#endif /* CK_POSIX_SIG */

static SIGTYP				/* CK_FORK_SIG handling in child ... */
forkint(foo) int foo; {
    /* It is important to disable CK_FORK_SIG before longjmp */
    signal(CK_FORK_SIG, SIG_IGN);	/* Set to ignore CK_FORK_SIG */
    debug(F100,"CONNECT forkint - CK_FORK_SIG", "", 0);
    /* Force return from ck_sndmsg() */
    cklongjmp(sig_env, 1);
    /* NOTREACHED */
}

static VOID
ck_sndmsg() {				/* Executed by child only ... */
    debug(F100,"CONNECT ck_sndmsg, active", "", active);
    if (
#ifdef CK_POSIX_SIG
	sigsetjmp(sig_env,1)
#else
	setjmp(sig_env)
#endif /* CK_POSIX_SIG */
	== 0) {
	debug(F100,"CONNECT ck_sndmsg signaling parent","",0);
        signal(CK_FORK_SIG, forkint);	/* Set up signal handler */
        kill(parent_id, CK_FORK_SIG);	/* Kick the parent */
	debug(F100,"ck_sndmsg pausing","",0);
        for (;;) pause();		/* Wait for CK_FORK_SIG or SIGKILL */
	/* NOTREACHED */
    }
    /* We come here from forkint() via [sig]cklongjmp(sig_env,1) */
    debug(F100,"CONNECT ck_sndmsg is parent - returning", "", 0);
}

/* Routines used by the parent process */

#ifdef CK_POSIX_SIG		 /* Environment pointer for CONNECT errors */
static sigjmp_buf con_env;
#else
static jmp_buf con_env;
#endif /* CK_POSIX_SIG */
/*
  pipeint() handles CK_FORK_SIG signals from the lower (port) fork.
  It reads a function code from the pipe that connects the two forks,
  then reads additional data from the pipe, then handles it.
*/
static SIGTYP
pipeint(arg) int arg; {			/* Dummy argument */
    int code, cx, x, i /* , n */ ;

#ifndef NOCCTRAP
    extern ckjmpbuf cmjbuf;
#endif /* NOCCTRAP */
    /*
      IMPORTANT: At this point, the child fork is waiting for CK_FORK_SIG
      (eventually for SIGKILL) inside of ck_sndmsg().  So we can't get any
      subsequent CK_FORK_SIG from child before we send it CK_FORK_SIG.
    */
    signal(CK_FORK_SIG, SIG_IGN);	/* Ignore CK_FORK_SIG now */
    debug(F101,"CONNECT pipeint arg","",arg);

    read(xpipe[0], &code, sizeof(code)); /* Get function code from pipe */
    debug(F101,"CONNECT pipeint code","",code);
    cx = code & 255;			/* 8-bit version of function code */

#ifndef NOCCTRAP
#ifndef NOICP
#define USECCJMPBUF
#endif /* NOICP */
#endif /* NOCCTRAP */
/*
  Read info passed back up to us by the lower fork, depending on the function
  requested.  The same number of items must be read from the pipe in the same
  order as the lower fork put them there.  Trying to read something that's not
  there makes the program hang uninterruptibly.  Pay close attention -- notice
  how we fall through some of the cases rather than break; that's deliberate.
*/
    switch (cx) {
#ifdef CK_TRIGGER
      case CEV_TRI:			/* Trigger string */
	debug(F100,"CONNECT trigger","",0);
	read(xpipe[0], (char *)&i, sizeof(i)); /* Trigger index */
	debug(F101,"CONNECT trigger index","",i);
	makestr(&triggerval,tt_trigger[i]); /* Make a copy of the trigger */
	debug(F110,"CONNECT triggerval",triggerval,0);
	read(xpipe[0], (char *)&ibc, sizeof(ibc)); /* Copy child's */
	debug(F101,"CONNECT trigger ibc (upper)","",ibc); /* input buffer. */
	if (ibc > 0) {
	    read(xpipe[0], (char *)&ibp, sizeof(ibp));
	    read(xpipe[0], ibp, ibc);
	}
	/* Fall thru... */

#endif /* CK_TRIGGER */

      case CEV_HUP:
/*
  The CEV_HUP case is executed when the other side has hung up on us.
  In some cases, this happens before we have had a chance to execute the
  setjmp(con_env,1) call, and in that case we'd better not take the longjmp!
  A good example is when you TELNET to port 13 on the local host; it prints
  its asctime() string (26 chars) and then closes the connection.
*/
#ifdef CK_TRIGGER
	if (cx == CEV_TRI)
	  sjval = CEV_TRI;		/* Set global variable. */
	else
#endif /* CK_TRIGGER */
	  sjval = CEV_HUP;
	if (jbset) {			/* jmp_buf is initialized */
	    cklongjmp(con_env,sjval);	/* so do the right thing. */
	} else {
	    int x = 0;
#ifdef USECCJMPBUF
	    /* jmp_buf not init'd yet a close approximation... */
#ifdef CK_TRIGGER
	    if (cx == CEV_HUP)
#endif /* CK_TRIGGER */
	      ttclos(0);		/* Close our end of the connection */
	    if (pid) {
		debug(F101,"CONNECT trigger killing pid","",pid);
#ifdef BEOSORBEBOX
		{
		    long ret_val;
		    x = kill(pid,SIGKILLTHR);	/* Kill lower fork */
		    wait_for_thread (pid, &ret_val);
		}
#else
#ifdef Plan9
		x = kill(pid, SIGKILL); /* (should always use this really) */
#else
		x = kill(pid,9);	/* Kill lower fork (history) */
#endif /* Plan9 */
		wait((WAIT_T *)0);	/* Wait till gone. */
		if (x < 0) {
		    printf("ERROR: Failure to kill pid %ld: %s, errno=%d\n",
			   (long) pid, ck_errstr(), errno);
		    debug(F111,"CONNECT error killing stale pid",
			  ck_errstr(),errno);
		}
		pid = (PID_T) 0;
#endif /* BEOSORBEBOX */
	    }
	    conres();			/* Reset the console. */
	    if (!quiet) {
		printf("\r\n(Back at %s)\r\n",
		       *myhost ? myhost :
#ifdef UNIX
		       "local UNIX system"
#else
		       "local system"
#endif /* UNIX */
		       );
	    }
	    what = W_NOTHING;		/* So console modes are set right. */
	    printf("\r\n");		/* prevent prompt-stomping */
	    cklongjmp(cmjbuf,0);	/* Do what the Ctrl-C handler does */
#else
	    printf("\r\nLongjump failure - fatal\r\n");
	    doexit(GOOD_EXIT,-1);	/* Better than dumping core... */
#endif /* USECCJMPBUF */
	}
#ifdef USECCJMPBUF
#undef USECCJMPBUF
#endif /* USECCJMPBUF */

      case CEV_DUP:			/* Child sends duplex change */
	read(xpipe[0], (char *)&duplex, sizeof(duplex));
	debug(F101,"CONNECT pipeint duplex","",duplex);
	break;
#ifdef TNCODE
      case CEV_MEBIN:			/* Child sends me_binary change */
	read(xpipe[0],
	     (char *)&TELOPT_ME(TELOPT_BINARY),
	     sizeof(TELOPT_ME(TELOPT_BINARY))
	     );
	debug(F101,"CONNECT pipeint me_binary","",TELOPT_ME(TELOPT_BINARY));
	break;
      case CEV_UBIN:			/* Child sends u_binary change */
	read(xpipe[0],
	     (char *)&TELOPT_U(TELOPT_BINARY),
	     sizeof(TELOPT_U(TELOPT_BINARY))
	     );
	debug(F101,"CONNECT pipeint u_binary","",TELOPT_U(TELOPT_BINARY));
	break;
#endif /* TNCODE */

#ifdef CK_APC
      case CEV_AUL:			/* Autoupload */
	justone = 1;
	debug(F100,"CONNECT autoupload at parent","",0);
#ifdef CK_AUTODL
      case CEV_ADL:			/* Autodownload */
	apcactive = APC_LOCAL;
	if (!justone) debug(F100,"CONNECT autodownload at parent","",0);
	/* Copy child's Kermit packet if any */
	read(xpipe[0], (char *)&x, sizeof(x));
	debug(F101,"CONNECT trigger ibc (upper)","",ibc);
	if (x > 0)
	  read(xpipe[0], (char *)ksbuf, x+1);
#endif /* CK_AUTODL */
      case CEV_APC:			/* Application Program Command */
	read(xpipe[0], (char *)&apclength, sizeof(apclength));
	read(xpipe[0], apcbuf, apclength+1); /* Include trailing zero byte */
	debug(F111,"CONNECT APC at parent",apcbuf,apclength);
	read(xpipe[0], (char *)&ibc, sizeof(ibc)); /* Copy child's */
	if (ibc > 0) {				   /* input buffer. */
	    read(xpipe[0], (char *)&ibp, sizeof(ibp));
	    read(xpipe[0], ibp, ibc);
	}
	obc = 0; obp = obuf; *obuf = NUL; /* Because port fork flushed */
	sjval = CEV_APC;
	cklongjmp(con_env,sjval);
	/* NOTREACHED */
#endif /* CK_APC */

#ifdef SUNX25
      case CEV_PAD:			/* X.25 PAD parameter change */
	debug(F100,"CONNECT pipeint PAD change","",0);
	read(xpipe[0],padparms,MAXPADPARMS);
	sjval = CEV_PAD;		/* Set global variable. */
#ifdef COMMENT				/* We might not need to do this... */
	cklongjmp(con_env,sjval);
	/* NOTREACHED */
#else  /* COMMENT */
	break;
#endif /* COMMENT */
#endif /* SUNX25 */
    }
    signal(CK_FORK_SIG, pipeint);	/* Set up signal handler */
    kill(pid, CK_FORK_SIG);		/* Signal the port fork ... */
}

/*  C K C P U T C  --  C-Kermit CONNECT Put Character to Screen  */
/*
  Output is buffered to avoid slow screen writes on fast connections.
  NOTE: These could (easily?) become macros ...
*/
static int
ckcputf() {				/* Dump the output buffer */
    int x = 0;
    if (obc > 0)			/* If we have any characters, */
      x = conxo(obc,obuf);		/* dump them, */
    obp = obuf;				/* reset the pointer */
    obc = 0;				/* and the counter. */
    return(x);				/* Return conxo's return code */
}

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
  Dummy argument for compatible calling conventions with ttinc().
  NOTE: We don't have a macro for this because we have to pass
  a pointer to this function as an argument to tn_doop().
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
#ifdef COMMENT
/* too much */
    debug(F101,"CONNECT CKCGETC 1 ibc","",ibc); /* Log */
#endif /* COMMENT */
    if (ibc < 1) {			/* Need to refill buffer? */
	ibc = 0;			/* Yes, reset count */
	ibp = ibuf;			/* and buffer pointer */
	/* debug(F100,"CONNECT CKCGETC 1 calling ttinc(0)","",0); */
#ifdef COMMENT
/*
  This check is not worth the overhead.  Scenario: ttchk() returns 0, so we
  fall through to the blocking ttinc().  While in ttinc(), the connection is
  lost.  But the read() that ttinc() calls does not notice, and never returns.
  This happens at least in HP-UX, and can be seen when we turn off the modem.
*/
	if (!network && (carrier != CAR_OFF))
	  if ((n = ttchk()) < 0)	/* Make sure connection is not lost */
	    return(n);
#endif /* COMMENT */
	c = ttinc(0);			/* Read one character, blocking */
	/* debug(F101,"CONNECT CKCGETC 1 ttinc(0)","",c); */
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
#ifdef CK_ENCRYPTION
		if (TELOPT_U(TELOPT_ENCRYPTION))
		  ck_tn_decrypt(ibp,n);
#endif /* CK_ENCRYPTION */
		ibc += n;			/* Advance counter */
	    }
	} else if (n < 0) {		/* Error? */
	    return(n);			/* Return the error code */
	}
	debug(F101,"CONNECT CKCGETC 2 ibc","",ibc); /* Log how many */
	ibp = ibuf;			/* Point to beginning of buffer */
    }
    c = *ibp++ & 0xff;			/* Get next character from buffer */
    ibc--;				/* Reduce buffer count */
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
*/
/*
  Another note: We stick in this read() till the user types something.
  But the other (lower) fork is running too, and on TELNET connections,
  it will signal us to indicate echo-change negotiations, and this can
  interrupt the read().  Some UNIXes automatically restart the interrupted
  system call, others return from it with errno == EINTR.
*/
static int				/* Keyboard buffer filler */
kbget() {
#ifdef EINTR
    int tries = 10;			/* If read() is interrupted, */
    int ok = 0;
    while (tries-- > 0) {		/* try a few times... */
#endif /* EINTR */
	if ((kbc = conchk()) < 1)	/* How many chars waiting? */
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

/*  C O N C L D --  Interactive terminal connection child function */

static
#ifdef BEOSORBEBOX
long
#else
VOID
#endif /* BEOSORBEBOX */
concld (
#ifdef BEOSORBEBOX
       void *bevoid
#endif /* BEOSORBEBOX */
       ) {
    int	n;			/* General purpose counter */
    int i;			/* For loops... */
    int c = -1;			/* c is a character, but must be signed
				   integer to pass thru -1, which is the
				   modem disconnection signal, and is
				   different from the character 0377 */
    int prev;
#ifdef TNCODE
    int tx;			/* tn_doop() return code */
#endif /* TNCODE */
#ifdef CK_TRIGGER
    int ix;				/* Trigger index */
#endif /* CK_TRIGGER */
#ifndef NOESCSEQ
    int apcrc;
#endif /* NOESCSEQ */

#ifdef COMMENT
    int conret = 0;			/* Return value from conect() */
    jbchksum = -1L;
#endif /* COMMENT */
    jbset = 0;				/* jmp_buf not set yet, don't use it */
    debug(F101,"CONNECT concld entry","",CK_FORK_SIG);
 	/* *** */		/* Inferior reads, prints port input */

    if (priv_can()) {			/* Cancel all privs */
	printf("?setuid error - fatal\n");
	doexit(BAD_EXIT,-1);
    }
    signal(SIGINT, SIG_IGN);		/* In case these haven't been */
    signal(SIGQUIT, SIG_IGN);		/* inherited from above... */
    signal(CK_FORK_SIG, SIG_IGN);	/* CK_FORK_SIG not expected yet */

    inshift = outshift = 0;		/* Initial SO/SI shift state. */
    {					/* Wait for parent's setup */
	int i;
	while ((i = read(xpipe[0], &c, 1)) <= 0) {
	    if (i < 0) {
		debug(F101,"CONNECT concld setup error","",i);
		debug(F111,"CONNECT concld setup error",ck_errstr(),errno);
		pipemsg(CEV_HUP);	/* Read error - hangup */
		ck_sndmsg();		/* Send and wait to be killed */
		/* NOTREACHED */
	    } /* Restart interrupted read() */
	}
    }
    close(xpipe[0]); xpipe[0] = -1;	/* Child - prevent future reads */
#ifdef DEBUG
    if (deblog) {
	debug(F100,"CONNECT starting port fork","",0);
	debug(F101,"CONNECT port fork ibc","",ibc);
	debug(F101,"CONNECT port fork obc","",obc);
    }
#endif /* DEBUG */
    what = W_CONNECT;

    while (1) {				/* Fresh read, wait for a character. */
#ifdef ANYX25
	if (network && (nettype == NET_SX25)) {
	    bzero(x25ibuf,sizeof(x25ibuf)) ;
	    if ((ibufl = ttxin(MAXIX25,(CHAR *)x25ibuf)) < 0) {
#ifndef IBMX25
		if (ibufl == -2) {  /* PAD parms changes */
		    pipemsg(CEV_PAD);
		    write(xpipe[1],padparms,MAXPADPARMS);
		    ck_sndmsg();
		} else {
#endif /* IBMX25 */
		    if (!quiet)
		      printf("\r\nCommunications disconnect ");
		    dologend();
		    pipemsg(CEV_HUP);
		    ck_sndmsg();		/* Wait to be killed */
		    /* NOTREACHED */
#ifndef IBMX25
  		}
#endif /* IBMX25 */
		/* pause(); <--- SHOULD BE OBSOLETE NOW! */
		/* BECAUSE pause() is done inside of ck_sndmsg() */
	    }
	    if (debses) {		/* Debugging output */
		p = x25ibuf ;
		while (ibufl--) { c = *p++; conol(dbchr(c)); }
	    } else {
		if (seslog && sessft)	/* Binary session log */
		  logchar((char)c);	/* Log char before translation */

		if (sosi
#ifndef NOCSETS
		    || tcsl != tcsr
#endif /* NOCSETS */
		    ) { /* Character at a time */
		    for (i = 1; i < ibufl; i++) {
			c = x25ibuf[i] & cmask;
			if (sosi) { /* Handle SI/SO */
			    if (c == SO) {
				inshift = 1;
				continue;
			    } else if (c == SI) {
				inshift = 0;
				continue;
			    }
			    if (inshift)
			      c |= 0200;
			}
#ifndef NOCSETS
			if (inesc == ES_NORMAL) {
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
				} else {
				    inxbuf[0] = (unsigned)(x & 0xff);
				}
				c = inxbuf[0];
			    } else if (unicode == 2) { /* Local is UTF-8 */
				inxcount =
				  b_to_u((CHAR)c,inxbuf,OUTXBUFSIZ,tcssize);
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
			c &= cmdmsk; /* Apply command mask. */
			conoc(c);    /* Output to screen */
			if (seslog && !sessft) /* and session log */
			  logchar(c);
		    }
		} else {		/* All at once */
		    for (i = 1; i < ibufl; i++)
		      x25ibuf[i] &= (cmask & cmdmsk);
		    conxo(ibufl,x25ibuf);
		    if (seslog) zsoutx(ZSFILE,x25ibuf,ibufl);
		}
	    }
	    continue;

	} else {			/* Not X.25... */
#endif /* ANYX25 */
/*
  Get the next communication line character from our internal buffer.
  If the buffer is empty, refill it.
*/
	    prev = c;			/* Remember previous character */
	    c = ckcgetc(0);		/* Get next character */
	    /* debug(F101,"CONNECT c","",c); */
	    if (c < 0) {		/* Failed... */
		debug(F101,"CONNECT disconnect ibc","",ibc);
		debug(F101,"CONNECT disconnect obc","",obc);
		ckcputf();		/* Flush CONNECT output buffer */
		if (!quiet) {
		    printf("\r\nCommunications disconnect ");
#ifdef COMMENT
		    if ( c == -3
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
		tthang();		/* Hang up the connection */
		debug(F111,"CONNECT concld i/o error",ck_errstr(),errno);
		pipemsg(CEV_HUP);
		ck_sndmsg();		/* Wait to be killed */
	    }
#ifdef COMMENT
/* too much... */
	    debug(F101,"CONNECT ** PORT","",c); /* Got character c OK. */
#endif /* COMMENT */
#ifdef TNCODE
	    /* Handle TELNET negotiations... */

	    if ((c == NUL) && network && IS_TELNET()) {
		if (prev == CR)		/* Discard <NUL> of <CR><NUL> */
		  if (!TELOPT_U(TELOPT_BINARY))
		    continue;
	    }
	    if ((c == IAC) && network && IS_TELNET()) {
		int me_bin = TELOPT_ME(TELOPT_BINARY);
		int u_bin = TELOPT_U(TELOPT_BINARY);
		debug(F100,"CONNECT got IAC","",0);
		ckcputf();		/* Dump screen-output buffer */
		if ((tx = tn_doop((CHAR)(c & 0xff),duplex,ckcgetc)) == 0) {
		    if (me_bin != TELOPT_ME(TELOPT_BINARY)) {
			debug(F101,
			      "CONNECT TELNET me_bin",
			      "",
			      TELOPT_ME(TELOPT_BINARY)
			      );
			pipemsg(CEV_MEBIN); /* Tell parent */
			write(xpipe[1],
			      &TELOPT_ME(TELOPT_BINARY),
			      sizeof(TELOPT_ME(TELOPT_BINARY))
			      );
			ck_sndmsg();	/* Tell the parent fork */
		    } else if (u_bin != TELOPT_U(TELOPT_BINARY)) {
			debug(F101,
			      "CONNECT TELNET u_bin",
			      "",
			      TELOPT_U(TELOPT_BINARY)
			      );
			pipemsg(CEV_UBIN); /* Tell parent */
			write(xpipe[1],
			      &TELOPT_U(TELOPT_BINARY),
			      sizeof(TELOPT_U(TELOPT_BINARY))
			      );
			ck_sndmsg();	/* Tell the parent fork */
		    }
		    continue;
		} else if (tx == -1) {	/* I/O error */
		    if (!quiet)
		      printf("\r\nCommunications disconnect ");
#ifdef NOSETBUF
		    fflush(stdout);
#endif /* NOSETBUF */
		    dologend();
		    debug(F111,"CONNECT concld i/o error 2",ck_errstr(),errno);
		    pipemsg(CEV_HUP);
		    ck_sndmsg();	/* Wait to be killed */
		    /* NOTREACHED */
		} else if (tx == -3) {	/* I/O error */
		    if (!quiet)
		      printf("\r\nConnection closed due to telnet policy ");
#ifdef NOSETBUF
		    fflush(stdout);
#endif /* NOSETBUF */
		    dologend();
		    debug(F111,"CONNECT concld i/o error 2",ck_errstr(),errno);
		    pipemsg(CEV_HUP);
		    ck_sndmsg();	/* Wait to be killed */
		    /* NOTREACHED */
		} else if (tx == -2) {	/* I/O error */
		    if (!quiet)
		      printf("\r\nConnection closed by peer ");
#ifdef NOSETBUF
		    fflush(stdout);
#endif /* NOSETBUF */
		    dologend();
		    debug(F111,"CONNECT concld i/o error 2",ck_errstr(),errno);
		    pipemsg(CEV_HUP);
		    ck_sndmsg();	/* Wait to be killed */
		    /* NOTREACHED */
		} else if ((tx == 1) && (!duplex)) { /* ECHO change */
		    duplex = 1;		/* Turn on local echo */
		    debug(F101,"CONNECT TELNET duplex change","",duplex);
		    pipemsg(CEV_DUP);	/* Tell parent */
		    write(xpipe[1], &duplex, sizeof(duplex));
		    ck_sndmsg();	/* Tell the parent fork */
		    continue;
		} else if ((tx == 2) && (duplex)) { /* ECHO change */
		    duplex = 0;
		    debug(F101,"CONNECT TELNET duplex change","",duplex);
		    pipemsg(CEV_DUP);
		    write(xpipe[1], &duplex, sizeof(duplex));
		    ck_sndmsg();
		    continue;
		} else if (tx == 3) { /* Quoted IAC */
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
			active = 0;
		    }
		}
#endif /* IKS_OPTION */
                else if (tx == 6) {
                    /* DO LOGOUT received */
		    if (!quiet)
		      printf("\r\nRemote Logout ");
#ifdef NOSETBUF
		    fflush(stdout);
#endif /* NOSETBUF */
		    debug(F100,"CONNECT Remote Logout","",0);
		    pipemsg(CEV_HUP);
		    ck_sndmsg();	/* Wait to be killed */
		    /* NOTREACHED */
                } else
		  continue;		/* Negotiation OK, get next char. */

	    } else if (parity)
	      c &= 0x7f;

            if (TELOPT_ME(TELOPT_ECHO) && tn_rem_echo)
                ttoc(c);                /* I'm echoing for the remote */
#endif /* TNCODE */

	    if (debses) {		/* Output character to screen */
		char *s;		/* Debugging display... */
		s = dbchr(c);
		while (*s)
		  ckcputc(*s++);
	    } else {			/* Regular display ... */
		c &= cmask;		/* Apply Kermit-to-remote mask */
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
		    int k;
		    k = kstart((CHAR)c); /* Kermit S or I packet? */
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

                            /* Now damage the packet so that it does not   */
                            /* trigger autodownload detection on subsquent */
                            /* links.                                      */

                            if (k == PROTO_K) {
                                int i, len = strlen((char*)ksbuf);
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
			    /* Notify parent */
			    pipemsg(justone ? CEV_AUL : CEV_ADL);
/*
  Send our memory back up to the top fork thru the pipe.
  CAREFUL -- Write this stuff in the same order it is to be read!
*/
			    /* Copy our Kermit packet to the parent fork */
			    n = (int) strlen((char *)ksbuf);
			    write(xpipe[1], (char *)&n, sizeof(n));
			    if (n > 0)
			      write(xpipe[1], (char *)ksbuf, n+1);
			    debug(F111,"CONNECT autodownload ksbuf",ksbuf,n);
			    debug(F101,"CONNECT autodownload justone","",
				  justone);
			    /* Construct the APC command */
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

			    /* Write buffer including trailing NUL byte */
			    debug(F101,"CONNECT write xpipe apclength","",
				  apclength);
			    write(xpipe[1],
				  (char *)&apclength,
				  sizeof(apclength)
				  );
			    debug(F110,"CONNECT write xpipe apcbuf",apcbuf,0);
			    write(xpipe[1], apcbuf, apclength+1);

			    /* Copy our input buffer to the parent fork */

			    debug(F101,"CONNECT autodownload complete ibc",
				  "",ibc);
			    debug(F101,"CONNECT autodownload complete obc",
				  "",obc);
			    write(xpipe[1], (char *)&ibc, sizeof(ibc));
			    if (ibc > 0) {
				write(xpipe[1], (char *)&ibp, sizeof(ibp));
				write(xpipe[1], ibp, ibc);
			    }
			    ck_sndmsg(); /* Wait to be killed */
			    /* NOTREACHED */
			}
		    }
		}
#ifdef NOSERVER
	      noserver:
#endif /* NOSERVER */
#endif /* CK_AUTODL */
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
		if (inesc == ES_NORMAL)	{ /* If not in an escape sequence */
#ifdef UNICODE
		    int x;		/* Translate character sets */
		    CHAR ch;
		    ch = c;
		    if (unicode == 1) {	/* Remote is UTF-8 */
			x = u_to_b(ch);
			if (x < 0)
			  continue;
			inxbuf[0] = (unsigned)(x & 0xff);
			c = inxbuf[0];
		    } else if (unicode == 2) { /* Local is UTF-8 */
			inxcount = b_to_u(ch,inxbuf,OUTXBUFSIZ,tcssize);
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
		if (escseq)		/* If handling escape sequences */
		  apcrc = chkaes((char)c); /* update our state */
#ifdef CK_APC
/*
  If we are handling APCs, we have several possibilities at this point:
   1. Ordinary character to be written to the screen.
   2. An Esc; we can't write it because it might be the beginning of an APC.
   3. The character following an Esc, in which case we write Esc, then char,
      but only if we have not just entered an APC sequence.
*/
		if (escseq && (apcstatus & APC_ON)) {
		    if (inesc == ES_GOTESC)	/* Don't write ESC yet */
		      continue;
		    else if (oldesc == ES_GOTESC && !apcactive) {
			ckcputc(ESC);	/* Write saved ESC */
			if (seslog && !sessft)
			  logchar((char)ESC);
		    } else if (apcrc) {	/* We have an APC */
			debug(F111,"CONNECT APC complete",apcbuf,apclength);
			ckcputf();		/* Force screen update */
			pipemsg(CEV_APC);	/* Notify parent */
			write(xpipe[1],
			      (char *)&apclength,
			      sizeof(apclength)
			      );
			/* Write buffer including trailing NUL byte */

			write(xpipe[1], apcbuf, apclength+1);

			/* Copy our input buffer to the parent fork */

			debug(F101,"CONNECT APC complete ibc","",ibc);
			debug(F101,"CONNECT APC complete obc","",obc);
			write(xpipe[1], (char *)&ibc, sizeof(ibc));
			if (ibc > 0) {
			    write(xpipe[1], (char *)&ibp, sizeof(ibp));
			    write(xpipe[1], ibp, ibc);
			}
			ck_sndmsg();	/* Wait to be killed */
			/* NOTREACHED */
		    }
		}
#endif /* CK_APC */
#endif /* NOESCSEQ */

		for (i = 0; i < inxcount; i++) { /* Loop thru */
		    c = inxbuf[i];	/* input expansion buffer... */
		    if (
#ifdef CK_APC
			!apcactive	/* Ignore APC sequences */
#else
			1
#endif /* CK_APC */
			) {
			c &= cmdmsk;	/* Apply command mask. */
			if (c == CR && tt_crd) { /* SET TERM CR-DISPLA CRLF? */
			    ckcputc(c);	/* Yes, output CR */
			    if (seslog && !sessft)
			      logchar((char)c);
			    c = LF;	/* and insert a linefeed */
			}
			if (c == LF && tt_lfd) { /* SET TERM CR-DISPLA CRLF? */
			    ckcputc(CR); /* Yes, output CR */
			    if (seslog && !sessft) logchar((char)CR);
			}
			ckcputc(c);	/* Write character to screen */
		    }
		    if (seslog && !sessft) /* Handle session log */
		      logchar((char)c);
#ifdef CK_TRIGGER
		    /* Check for trigger string */
		    if (tt_trigger[0]) if ((ix = autoexitchk((CHAR)c)) > -1) {
			ckcputf();	/* Force screen update */
#ifdef NOSETBUF
			fflush(stdout);	/* I mean really force it */
#endif /* NOSETBUF */
			pipemsg(CEV_TRI); /* Send up trigger indication */
			write(xpipe[1], (char *)&ix, sizeof(ix)); /* index */
			write(xpipe[1], (char *)&ibc, sizeof(ibc));
			if (ibc > 0) {
			    write(xpipe[1], (char *)&ibp, sizeof(ibp));
			    write(xpipe[1], ibp, ibc);
			}
			debug(F100,"CONNECT concld trigger","",0);
			ck_sndmsg();	/* Wait to be killed */
			active = 0;	/* Shouldn't be necessary... */
			break;
		    }
		    /* NOTREACHED */
#endif /* CK_TRIGGER */
		}
	    }
#ifdef ANYX25
	}
#endif /* ANYX25 */
    }
}


/*  C O N E C T  --  Interactive terminal connection  */

int
conect() {
    int	n;			/* General purpose counter */
    int i;			/* For loops... */
    int c;			/* c is a character, but must be signed
				   integer to pass thru -1, which is the
				   modem disconnection signal, and is
				   different from the character 0377 */
    int c2;			/* A copy of c */
    int csave;			/* Another copy of c */
#ifndef NOESCSEQ
    int apcrc;
#endif /* NOESCSEQ */

    int conret = 0;			/* Return value from conect() */
    int msgflg = 0;
    /* jbchksum = -1L; */
    jbset = 0;				/* jmp_buf not set yet, don't use it */
    debok = 1;

    debug(F101,"CONNECT fork signal","",CK_FORK_SIG);
    debug(F101,"CONNECT entry pid","",pid);

    msgflg = !quiet
#ifdef CK_APC
      && !apcactive
#endif /* CK_APC */
	;
/*
  The following is to handle a fork left behind if we exit CONNECT mode
  without killing it, and then return to CONNECT mode.  This happened in
  HP-UX, where the Reset key would raise SIGINT even though SIGINT was set to
  SIG_IGN.  The code below fixes the symptom; the real fix is in the main
  SIGINT handler (if SIGINT shows up during CONNECT, just return rather than
  taking the longjmp).
*/
    if (pid) {				/* This should be 0 */
	int x = 0;
	debug(F101,"CONNECT entry killing stale pid","",pid);
	printf("WARNING: Old CONNECT fork seems to be active.\n");
	printf("Attempting to remove it...");
#ifdef BEOSORBEBOX
	{
	    long ret_val;
	    x = kill(pid,SIGKILLTHR); /* Kill lower fork */
	    wait_for_thread (pid, &ret_val);
	}
#else
#ifdef Plan9
	x = kill(pid,SIGKILL);		/* Kill lower fork */
#else
	x = kill(pid,9);
#endif /* Plan9 */
#endif /* BEOSORBEBOX */
	wait((WAIT_T *)0);		/* Wait till gone. */
	if (x < 0) {
	    printf("ERROR: Failure to kill pid %d: %s, errno=%d\n",
		   (int) pid, ck_errstr(), errno);
	    debug(F111,"CONNECT error killing stale pid",ck_errstr(),pid);
	}
	pid = (PID_T) 0;
	printf("\n");
    }
    signal(CK_FORK_SIG, SIG_IGN);	/* Initial CK_FORK_SIG handling, */
/*
  The following ttimoff() call should not be necessary, but evidently there
  are cases where a timer is left active and then goes off, taking a longjmp
  to nowhere after the program's stack has changed.  In any case, this is
  safe because the CONNECT module uses no timer of any kind, and no other timer
  should be armed while Kermit is in CONNECT mode.
*/
    ttimoff();				/* Turn off any timer interrupts */

#ifdef CK_TRIGGER
    makestr(&triggerval,NULL);		/* Reset trigger */
#endif /* CK_TRIGGER */

    if (!local) {
#ifdef NETCONN
	printf("Sorry, you must SET LINE or SET HOST first\n");
#else
	printf("Sorry, you must SET LINE first\n");
#endif /* NETCONN */
	goto conret0;
    }
    if (speed < 0L && network == 0 && ttfdflg == 0) {
	printf("Sorry, you must SET SPEED first\n");
	goto conret0;
    }
#ifdef TCPSOCKET
    if (network && (nettype != NET_TCPB)
#ifdef SUNX25
        && (nettype != NET_SX25)
#endif /* SUNX25 */
#ifdef IBMX25
	&& (nettype != NET_IX25)
#endif /* IBMX25 */
#ifdef NETCMD
        && (nettype != NET_CMD)
#endif /* NETCMD */
#ifdef NETPTY
       && (nettype != NET_PTY)
#endif /* NETPTY */
    ) {
	printf("Sorry, network type not supported\n");
	goto conret0;
    }
#endif /* TCPSOCKET */

#ifdef DYNAMIC
    if (!ibuf) {
	if (!(ibuf = malloc(IBUFL+1))) { /* Allocate input line buffer */
	    printf("Sorry, CONNECT input buffer can't be allocated\n");
	    goto conret0;
	} else {
	    ibp = ibuf;
	    ibc = 0;
	}
    }
    if (!obuf) {
	if (!(obuf = malloc(OBUFL+1))) {    /* Allocate output line buffer */
	    printf("Sorry, CONNECT output buffer can't be allocated\n");
	    goto conret0;
	} else {
	    obp = obuf;
	    obc = 0;
	}
    }
    if (!kbuf) {
	if (!(kbuf = malloc(KBUFL+1))) { /* Allocate keyboard input buffer */
	    printf("Sorry, CONNECT keyboard buffer can't be allocated\n");
	    goto conret0;
	}
    }
    if (!temp) {
	if (!(temp = malloc(TMPLEN+1))) { /* Allocate temporary buffer */
	    printf("Sorry, CONNECT temporary buffer can't be allocated\n");
	    goto conret0;
	}
    }
#else
#ifdef COMMENT
    ibp = ibuf;
    ibc = 0;
#endif /* COMMENT */
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
	    goto conret0;
	}
#ifdef IKS_OPTION
	/* If peer is in Kermit server mode, return now. */
	if (TELOPT_SB(TELOPT_KERMIT).kermit.u_start)
	  return(0);
#endif /* IKS_OPTION */
    }
    dohangup = 0;			/* Hangup not requested yet */
#ifdef ANYX25
    dox25clr = 0;			/* X.25 clear not requested yet */
#endif /* ANYX25 */

    if (msgflg) {
#ifdef NETCONN
	if (network) {
	    if (ttpipe)
	      printf("Connecting via command \"%s\"",ttname);
	    else
	      printf("Connecting to host %s",ttname);
#ifdef ANYX25
	    if (nettype == NET_SX25 || nettype == NET_IX25)
	      printf(", Link ID %d, LCN %d",linkid,lcn);
#endif /* ANYX25 */
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
	    printf("Session Log: %s, %s\r\n",sesfil,s);
	}
	if (debses) printf("Debugging Display...)\r\n");
        printf("----------------------------------------------------\r\n");
	fflush(stdout);
    }

/* Condition console terminal and communication line */

    if (conbin((char)escape) < 0) {
	printf("Sorry, can't condition console terminal\n");
	goto conret0;
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
		ckmakmsg(temp,TMPLEN,"Sorry, can't reopen ",ttname,NULL,NULL);
		perror(temp);
		goto conret0;
	    }
#ifdef IKS_OPTION
	    if (TELOPT_SB(TELOPT_KERMIT).kermit.u_start)
	      return(0);
#endif /* IKS_OPTION */
	    if (ttvt(speed,flow) < 0) {	/* Try virtual terminal mode again. */
		conres();		/* Failure this time is fatal. */
		printf("Sorry, Can't condition communication line\n");
		goto conret0;
	    }
	}
    }
    debug(F101,"CONNECT ttvt ok, escape","",escape);

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
	    if (!hints)
	      return(0);
	    printf("***********************************\n");
	    printf(" Hint: To CONNECT to a serial device that\n");
	    printf(" is not presenting the Carrier Detect signal,\n");
	    printf(" first tell C-Kermit to:\n\n");
	    printf("   SET CARRIER-WATCH OFF\n\n");
	    printf("***********************************\n\n");
#endif /* NOHINTS */
	    goto conret0;
	}
	debug(F100,"CONNECT ttgmdm ok","",0);
    }
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
   (b) the local and/or remote set is a 7-bit set other than US ASCII.
  Or:
   SET TERMINAL APC is not OFF (handled in the next statement).
*/
    escseq = (tcs != TC_TRANSP) &&	/* Not transparent */
      (fcsinfo[tcsl].size == 128 || fcsinfo[tcsr].size == 128) && /* 7 bits */
	(fcsinfo[tcsl].code != FC_USASCII); /* But not ASCII */
#endif /* NOESCSEQ */
#endif /* NOCSETS */

#ifndef NOESCSEQ
#ifdef CK_APC
    escseq = escseq || (apcstatus & APC_ON);
    apcactive = 0;			/* An APC command is not active */
    apclength = 0;			/* ... */
#endif /* CK_APC */
    inesc = ES_NORMAL;			/* Initial state of recognizer */
    debug(F101,"CONNECT escseq","",escseq);
#endif /* NOESCSEQ */

    parent_id = getpid();		/* Get parent's pid for signalling */
    debug(F101,"CONNECT parent pid","",parent_id);

    if (xpipe[0] > -1)			/* If old pipe hanging around, close */
      close(xpipe[0]);
    xpipe[0] = -1;
    if (xpipe[1] > -1)
      close(xpipe[1]);
    xpipe[1] = -1;
    goterr = 0;				/* Error flag for pipe & fork */
    if (pipe(xpipe) != 0) {		/* Create new pipe to pass info */
	perror("Can't make pipe");	/* between forks. */
	debug(F101,"CONNECT pipe error","",errno);
	goterr = 1;
    } else
#ifdef BEOSORBEBOX
    {
        pid = spawn_thread(concld, "Lower Fork", B_NORMAL_PRIORITY, NULL);
        resume_thread(pid);
    }
#else
    if ((pid = fork()) == (PID_T) -1) { /* Pipe OK, make port fork. */
	perror("Can't make port fork");
	debug(F101,"CONNECT fork error","",errno);
	goterr = 1;
    }
#endif /* BEOSORBEBOX */
    debug(F101,"CONNECT created fork, pid","",pid);
    if (goterr) {			/* Failed to make pipe or fork */
	conres();			/* Reset the console. */
	if (msgflg) {
	    printf("\r\nCommunications disconnect (Back at %s)\r\n",
		   *myhost ?
		   myhost :
#ifdef UNIX
		   "local UNIX system"
#else
		   "local system"
#endif /* UNIX */
		   );
	}
	printf("\n");
	what = W_NOTHING;		/* So console modes are set right. */
#ifndef NOCSETS
	language = langsv;		/* Restore language */
#endif /* NOCSETS */
	parent_id = (PID_T) 0;		/* Clean up */
	goto conret1;
    }
    debug(F101,"CONNECT fork pid","",pid);

/* Upper fork (KEYB fork) reads keystrokes and sends them out. */

    if (pid) {				/* pid != 0, so I am the upper fork. */
/*
  Before doing anything significant, the child fork must wait for a go-ahead
  character from xpipe[0].  Before starting to wait, we have enough time to
  clear buffers and set up the signal handler.  When done with this, we will
  allow the child to continue by satisfying its pending read.

  Remember the child and parent have separate address space.  The child has
  its own copy of input buffers, so we must clear the input buffers in the
  parent.  Otherwise strange effects may occur, like chunks of characters
  repeatedly echoed on terminal screen.  The child process is designed to
  empty its input buffers by reading all available characters and either
  echoing them on the terminal screen or saving them for future use in the
  parent.  The latter case happens during APC processing - see the code around
  CEV_APC occurrences to see how the child passes its ibuf etc to parent via
  xpipe, for preservation until the next entry to this module, to ensure that
  no characters are lost between CONNECT sessions.
*/

/*
  This one needs a bit of extra explanation...  In addition to the CONNECT
  module's own buffers, which are communicated and synchronized via xpipe,
  the low-level UNIX communication routines (ttinc, ttxin, etc) are also
  buffered, statically, in the ckutio.c module.  But when the two CONNECT
  forks split off, the lower fork is updating this buffer's pointers and
  counts, but the upper fork never finds out about it and still has the old
  ones.  The following UNIX-specific call to the ckutio.c module takes care
  of this...  Without it, we get dual echoing of incoming characters.
*/
	ttflux();
/*
  At this point, perhaps you are wondering why we use forks at all.  It is
  simply because there is no other method portable among all UNIX variations.
  Not threads, not select(), ...  (Yes, select() is more common now; it might
  actually be worth writing a variation of this module that works like BSD
  Telnet, one fork, driven by select()).
*/
	ibp = ibuf;			/* Clear ibuf[]. */
	ibc = 0;			/* Child now has its own copy */
	signal(CK_FORK_SIG, pipeint);	/* Handler for messages from child. */
	write(xpipe[1], ibuf, 1);	/* Allow child to proceed */
	close(xpipe[1]); xpipe[1] = -1; /* Parent - prevent future writes */

	what = W_CONNECT;		/* Keep track of what we're doing */
	active = 1;
	debug(F101,"CONNECT keyboard fork duplex","",duplex);
/*
  Catch communication errors or mode changes in lower fork.

  Note: Some C compilers (e.g. Cray UNICOS) interpret the ANSI C standard
  about setjmp() in a way that disallows constructions like:

        if ((var = [sig]setjmp(env)) == 0) ...

  which prevents the value returned by cklongjmp() from being used at all.
  So the signal handlers set a global variable, sjval, instead.
*/
	if (
#ifdef CK_POSIX_SIG
	    sigsetjmp(con_env,1)
#else
	    setjmp(con_env)
#endif /* CK_POSIX_SIG */
	    == 0) {			/* Normal entry... */
	    jbset = 1;			/* Now we have a longjmp buffer */
	    sjval = CEV_NO;		/* Initialize setjmp return code. */

	    debug(F101,"CONNECT setjmp normal entry","",sjval);

#ifdef ANYX25
	    if (network && (nettype == NET_SX25 || nettype == NET_IX25)) {
		obufl = 0;
		bzero (x25obuf,sizeof(x25obuf));
	    }
#endif /* ANYX25 */
/*
  Here is the big loop that gets characters from the keyboard and sends them
  out the communication device.  There are two components to the communication
  path: the connection from the keyboard to C-Kermit, and from C-Kermit to
  the remote computer.  The treatment of the 8th bit of keyboard characters
  is governed by SET COMMAND BYTESIZE (cmdmsk).  The treatment of the 8th bit
  of characters sent to the remote is governed by SET TERMINAL BYTESIZE
  (cmask).   This distinction was introduced in edit 5A(164).
*/
	    while (active) {
#ifndef NOSETKEY
		if (kmptr) {		/* Have current macro? */
		    debug(F100,"CONNECT kmptr non NULL","",0);
		    if ((c = (CHAR) *kmptr++) == NUL) { /* Get char from it */
			kmptr = NULL;	/* If no more chars,  */
			debug(F100,"CONNECT macro empty, continuing","",0);
			continue;	/* reset pointer and continue */
		    }
		    debug(F000,"CONNECT char from macro","",c);
		} else			/* No macro... */
#endif /* NOSETKEY */
		  c = CONGKS();		/* Read from keyboard */

#ifdef COMMENT
/* too much... */
		debug(F101,"CONNECT ** KEYB","",c);
#endif /* COMMENT */
                if (c == -1) {		/* If read() got an error... */
		    debug(F101,"CONNECT keyboard read errno","",errno);
#ifdef COMMENT
/*
 This seems to cause problems.  If read() returns -1, the signal has already
 been delivered, and nothing will wake up the pause().
*/
		    pause();		/* Wait for transmitter to finish. */
#else
#ifdef A986
/*
  On Altos machines with Xenix 3.0, pressing DEL in connect mode brings us
  here (reason unknown).  The console line discipline at this point has
  intr = ^C.  The communications tty has intr = DEL but we get here after
  pressing DEL on the keyboard, even when the remote system has been set not
  to echo.  With A986 defined, we stay in the read loop and beep only if the
  offending character is not DEL.
*/
		    if ((c & 127) != 127) conoc(BEL);
#else
#ifdef EINTR
/*
   This can be caused by the other fork signalling this one about
   an echoing change during TELNET negotiations.
*/
		    if (errno == EINTR)
		      continue;
#endif /* EINTR */
		    conoc(BEL);		/* Otherwise, beep */
		    active = 0;		/* and terminate the read loop */
		    continue;
#endif /* A986 */
#endif /* COMMENT */
		}
		c &= cmdmsk;		/* Do any requested masking */
#ifndef NOSETKEY
/*
  Note: kmptr is NULL if we got character c from the keyboard, and it is
  not NULL if it came from a macro.  In the latter case, we must avoid
  expanding it again.
*/
		if (!kmptr && macrotab[c]) { /* Macro definition for c? */
		    kmptr = macrotab[c];     /* Yes, set up macro pointer */
		    continue;		     /* and restart the loop, */
		} else c = keymap[c];	     /* else use single-char keymap */
#endif /* NOSETKEY */
		if (
#ifndef NOSETKEY
		    !kmptr &&
#endif /* NOSETKEY */
		    (tt_escape && (c & 0xff) == escape)) { /* Escape char? */
		    debug(F000,"CONNECT got escape","",c);
		    c = CONGKS() & 0177; /* Got esc, get its arg */
		    /* No key mapping here */
		    doesc((char) c);	/* Now process it */

		} else {		/* It's not the escape character */
		    csave = c;		/* Save it before translation */
					/* for local echoing. */
#ifndef NOCSETS
		    if (inesc == ES_NORMAL) { /* If not inside escape seq.. */
			/* Translate character sets */
#ifdef UNICODE
			int x;
			CHAR ch;
			ch = c;
			if (unicode == 1) { /* Remote is UTF-8 */
			    outxcount = b_to_u(ch,outxbuf,OUTXBUFSIZ,tcssize);
			    outxbuf[outxcount] = NUL;
			} else if (unicode == 2) { /* Local is UTF-8 */
			    x = u_to_b(ch); /* So translate to remote byte */
			    if (x < 0)
			      continue;
			    outxbuf[0] = (unsigned)(x & 0xff);
			    outxcount = 1;
			    outxbuf[outxcount] = NUL;
			} else {
#endif /* UNICODE */
			    /* Local-to-intermediate */
			    if (sxo) c = (*sxo)((char)c);
			    /* Intermediate-to-remote */
			    if (rxo) c = (*rxo)((char)c);
			    outxbuf[0] = c;
			    outxcount = 1;
			    outxbuf[outxcount] = NUL;
#ifdef UNICODE
			}
#endif /* UNICODE */
		    }
		    if (escseq)
		      apcrc = chkaes((char)c);
#else
		    outxbuf[0] = c;
		    outxcount = 1;
		    outxbuf[outxcount] = NUL;
#endif /* NOCSETS */
		    for (i = 0; i < outxcount; i++) {
			c = outxbuf[i];
/*
 If Shift-In/Shift-Out is selected and we have a 7-bit connection,
 handle shifting here.
*/
			if (sosi) {	/* Shift-In/Out selected? */
			    if (cmask == 0177) { /* In 7-bit environment? */
				if (c & 0200) {	/* 8-bit character? */
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
			    if (c == SO) outshift = 1;   /* User typed SO */
			    if (c == SI) outshift = 0;   /* User typed SI */
			}
			c &= cmask;	/* Apply Kermit-to-host mask now. */
#ifdef SUNX25
			if (network && nettype == NET_SX25) {
			    if (padparms[PAD_ECHO]) {
				if (debses)
				  conol(dbchr(c)) ;
				else
				  if ((c != padparms[PAD_CHAR_DELETE_CHAR]) &&
				    (c != padparms[PAD_BUFFER_DELETE_CHAR]) &&
				    (c != padparms[PAD_BUFFER_DISPLAY_CHAR]))
				    conoc(c) ;
				if (seslog && !sessft)
				  logchar(c);
			    }
			    if (c == CR && (padparms[PAD_LF_AFTER_CR] == 4 ||
					    padparms[PAD_LF_AFTER_CR] == 5)) {
				if (debses)
				  conol(dbchr(LF)) ;
				else
				  conoc(LF) ;
				if (seslog && !sessft)
				  logchar(LF);
			    }
			    if (c == padparms[PAD_BREAK_CHARACTER]) {
				breakact();
			    } else if (padparms[PAD_DATA_FORWARD_TIMEOUT]) {
				tosend = 1;
				x25obuf [obufl++] = c;
			    } else if (((c == padparms[PAD_CHAR_DELETE_CHAR])||
				     (c == padparms[PAD_BUFFER_DELETE_CHAR]) ||
				     (c == padparms[PAD_BUFFER_DISPLAY_CHAR]))
				       && (padparms[PAD_EDITING])) {
				if (c == padparms[PAD_CHAR_DELETE_CHAR]) {
				    if (obufl > 0) {
					conol("\b \b"); obufl--;
				    } else {}
				} else if
				  (c == padparms[PAD_BUFFER_DELETE_CHAR]) {
				      conol ("\r\nPAD Buffer Deleted\r\n");
				      obufl = 0;
				} else if
				  (c==padparms[PAD_BUFFER_DISPLAY_CHAR]) {
				      conol("\r\n");
				      conol(x25obuf);
				      conol("\r\n");
				} else {}
			    } else {
				x25obuf [obufl++] = c;
				if (obufl == MAXOX25) tosend = 1;
				else if (c == CR) tosend = 1;
			    }
			    if (tosend) {
				if (ttol((CHAR *)x25obuf,obufl) < 0) {
				    perror ("\r\nCan't send characters");
				    active = 0;
				} else {
				    bzero (x25obuf,sizeof(x25obuf));
				    obufl = 0;
				    tosend = 0;
				}
			    } else {};
			} else {
#endif /* SUNX25 */
			    if (c == '\015') { /* Carriage Return */
				int stuff = -1;
				if (tnlm) { /* TERMINAL NEWLINE ON */
				    stuff = LF;	/* Stuff LF */
#ifdef TNCODE
				} else if (network && /* TELNET NEWLINE */
					   IS_TELNET()) {
				    switch (!TELOPT_ME(TELOPT_BINARY) ?
					    tn_nlm :
					    tn_b_nlm
					    ) {
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
				    ttoc(dopar('\015')); /* Send CR */
				    if (duplex) conoc('\015'); /* Echo CR */
				    c = stuff; /* Char to stuff */
				    csave = c;
				}
			    }
#ifdef TNCODE
/* If user types the 0xff character (TELNET IAC), it must be doubled. */
			    else	/* Not CR */
			      if ((dopar((CHAR) c) == IAC) && /* IAC (0xff) */
				  network && IS_TELNET()) {
				  /* Send one copy now */
				  /* and the other one just below. */
				  ttoc((char)IAC);
			      }
#endif /* TNCODE */
			    /* Send the character */

			    if (ttoc((char)dopar((CHAR) c)) > -1) {
				if (duplex) {	/* If half duplex, must echo */
				    if (debses)
				      conol(dbchr(csave)); /* original char */
				    else /* not the translated one */
				      conoc((char)csave);
				    if (seslog) { /* And maybe log it too */
					c2 = csave;
					if (sessft == 0 && csave == '\r')
					  c2 = '\n';
					logchar((char)c2);
				    }
				}
			    } else {
				perror("\r\nCan't send character");
				active = 0;
			    }
#ifdef SUNX25
			}
#endif /* SUNX25 */
		    } /* for... */
		}
	    }

	    /* now active == 0 */
            signal(CK_FORK_SIG, SIG_IGN); /* Turn off CK_FORK_SIG */
	    sjval = CEV_NO;		/* Set to hangup */
	}				/* Come here on termination of child */

/* cklongjmp() executed in pipeint() (parent only!) comes here */

/*
  Now the child fork is gone or is waiting for CK_FORK_SIG in ck_sndmsg().
  So we can't get (in the parent) any subsequent CK_FORK_SIG signals until
  we signal the child with CK_FORK_SIG.
*/
	debug(F100,"CONNECT signaling port fork","",0);
	signal(CK_FORK_SIG, SIG_IGN);	/* Turn this off */
	debug(F101,"CONNECT killing port fork","",pid);
	if (pid) {
	    int x = 0;
#ifdef BEOSORBEBOX
	    {
		long ret_val;
		x = kill(pid,SIGKILLTHR); /* Kill lower fork */
		wait_for_thread(pid, &ret_val);
	    }
#else
#ifdef Plan9
	    x = kill(pid,SIGKILL);	/* Kill lower fork */
#else
	    x = kill(pid,9);
#endif /* Plan9 */
#endif /* BEOSORBEBOX */
	    wait((WAIT_T *)0);		/* Wait till gone. */
	    if (x < 0) {
		printf("WARNING: Failure to kill fork, pid %d: %s, errno=%d\n",
		       (int) pid, ck_errstr(), errno);
		debug(F111,"CONNECT error killing pid",ck_errstr(),errno);
	    }
	    debug(F101,"CONNECT killed port fork","",pid);
	    pid = (PID_T) 0;
	}
	if (sjval == CEV_HUP) {		/* Read error on comm device */
	    dohangup = 1;		/* so we want to hang up our side */
#ifdef NETCONN
	    if (network) {		/* and/or close network connection */
		ttclos(0);
		dologend();
#ifdef SUNX25
		if (nettype == NET_SX25) /* If X.25, restore the PAD params */
		  initpad();
#endif /* SUNX25 */
	    }
#endif /* NETCONN */
	}
#ifdef CK_APC
	if (sjval == CEV_APC) {		/* Application Program Command rec'd */
	    apcactive = APC_REMOTE;	/* Flag APC as active */
	    active = 0;			/* Flag CONNECT as inactive */
	}
#endif /* CK_APC */
	conres();			/* Reset the console. */
	if (dohangup > 0) {		/* If hangup requested, do that. */
#ifndef NODIAL
	    if (dohangup > 1)		/* User asked for it */
	      if (mdmhup() < 1)		/* Maybe hang up via modem */
#endif /* NODIAL */
		tthang();		/* And make sure we don't hang up */
	    dohangup = 0;		/* again unless requested again. */
	}

#ifdef COMMENT
#ifdef NETCONN
#ifdef SIGPIPE
	if (network && sigpiph)		/* Restore previous SIGPIPE handler */
	  (VOID) signal(SIGPIPE, sigpiph);
#endif /* SIGPIPE */
#endif /* NETCONN */
#endif /* COMMENT */

#ifdef ANYX25
	if (dox25clr) {			/* If X.25 Clear requested */
	    x25clear();			/* do that. */
#ifndef IBMX25
	    initpad();
#endif /* IBMX25 */
	    dox25clr = 0;		/* But only once. */
	}
#endif /* ANYX25 */

	if (quitnow) doexit(GOOD_EXIT,xitsta); /* Exit now if requested. */
  	if (msgflg)
	  printf("(Back at %s)", *myhost ? myhost : "local UNIX system");
#ifdef CK_APC
        if (!apcactive)
#endif /* CK_APC */
	  printf("\n");
	what = W_NOTHING;		/* So console modes set right. */
#ifndef NOCSETS
	language = langsv;		/* Restore language */
#endif /* NOCSETS */
	parent_id = (PID_T) 0;
	goto conret1;

    }
#ifndef BEOSORBEBOX
    else {	/* *** */		/* Inferior reads, prints port input */
        concld(/* (void *)&pid */);
    }
#endif /* BEOSORBEBOX */

conret1:
    conret = 1;
conret0:
    signal(CK_FORK_SIG, SIG_IGN);	/* In case this wasn't done already */
    debug(F101,"CONNECT conect exit ibc","",ibc);
    debug(F101,"CONNECT conect exit obc","",obc);
    close(xpipe[0]); xpipe[0] = -1;	/* Close the pipe */
    close(xpipe[1]); xpipe[1] = -1;
    if (msgflg) {
#ifdef CK_APC
	if (apcactive == APC_LOCAL)
	  printf("\n");
#endif /* CK_APC */
	printf("----------------------------------------------------\r\n");
    }
    fflush(stdout);
    return(conret);
}


/*  H C O N N E  --  Give help message for connect.  */

int
hconne() {
    int c;
    static char *hlpmsg[] = {
"\r\n  ? for this message",
"\r\n  0 (zero) to send a null",
"\r\n  B to send a BREAK",
#ifdef CK_LBRK
"\r\n  L to send a Long BREAK",
#endif /* CK_LBRK */
#ifdef NETCONN
"\r\n  I to send a network interrupt packet",
#ifdef TCPSOCKET
"\r\n  A to send Are You There?",
#endif /* TCPSOCKET */
#ifdef ANYX25
"\r\n  R to reset X.25 virtual circuit",
#endif /* ANYX25 */
#endif /* NETCONN */
"\r\n  U to hangup and close the connection",
"\r\n  Q to hangup and quit Kermit",
"\r\n  S for status",
#ifdef NOPUSH
"\r\n  ! to push to local shell (disabled)",
"\r\n  Z to suspend (disabled)",
#else
"\r\n  ! to push to local shell",
#ifdef NOJC
"\r\n  Z to suspend (disabled)",
#else
"\r\n  Z to suspend",
#endif /* NOJC */
#endif /* NOPUSH */
"\r\n  \\ backslash code:",
"\r\n    \\nnn  decimal character code",
"\r\n    \\Onnn octal character code",
"\r\n    \\Xhh  hexadecimal character code",
"\r\n    terminate with carriage return.",
"\r\n Type the escape character again to send the escape character, or",
"\r\n press the space-bar to resume the CONNECT command.\r\n",
"" };

    conol("\r\n----------------------------------------------------");
    conol("\r\nPress C to return to ");
    conol(*myhost ? myhost : "the C-Kermit prompt");
    conol(", or:");
    conola(hlpmsg);			/* Print the help message. */
    conol("Command>");			/* Prompt for command. */
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
	    active = 0; conol("\r\n"); return;

	case 'b':			/* Send a BREAK signal */
	case '\02':
	    ttsndb(); return;

#ifdef NETCONN
	case 'i':			/* Send Interrupt */
	case '\011':
#ifdef TCPSOCKET
#ifndef IP
#define IP 244
#endif /* IP */
	    if (network && IS_TELNET()) { /* TELNET */
		temp[0] = (CHAR) IAC;	/* I Am a Command */
		temp[1] = (CHAR) IP;	/* Interrupt Process */
		temp[2] = NUL;
		ttol((CHAR *)temp,2);
	    } else
#endif /* TCPSOCKET */
#ifdef SUNX25
	    if (network && (nettype == NET_SX25)) {
		(VOID) x25intr(0);	            /* X.25 interrupt packet */
		conol("\r\n");
	    } else
#endif /* SUNX25 */
	      conoc(BEL);
	    return;

#ifdef TCPSOCKET
	case 'a':			/* "Are You There?" */
	case '\01':
#ifndef AYT
#define AYT 246
#endif /* AYT */
	    if (network && IS_TELNET()) {
		temp[0] = (CHAR) IAC;	/* I Am a Command */
		temp[1] = (CHAR) AYT;	/* Are You There? */
		temp[2] = NUL;
		ttol((CHAR *)temp,2);
	    } else conoc(BEL);
	    return;
#endif /* TCPSOCKET */
#endif /* NETCONN */

#ifdef CK_LBRK
	case 'l':			/* Send a Long BREAK signal */
	    ttsndlb(); return;
#endif /* CK_LBRK */

	case 'u':			/* Hangup */
     /*	case '\010': */			/* No, too dangerous */
#ifdef ANYX25
            if (network && (nettype == NET_SX25 || nettype == NET_IX25))
	      dox25clr = 1;
            else
#endif /* ANYX25 */
	    dohangup = 2; active = 0; conol("\r\nHanging up "); return;

#ifdef ANYX25
        case 'r':                       /* Reset the X.25 virtual circuit */
        case '\022':
            if (network && (nettype == NET_SX25 || nettype == NET_IX25))
		(VOID) x25reset(0,0);
            conol("\r\n");
	    return;
#endif /* ANYX25 */

	case 'q':			/* Quit */
	    dohangup = 2; quitnow = 1; active = 0; conol("\r\n"); return;

	case 's':			/* Status */
	    conoll("");
	    conoll("----------------------------------------------------");
#ifdef NETCMD
	    if (ttpipe)
	      ckmakmsg(temp,TMPLEN," Pipe: \"",ttname,"\"",NULL);
	    else
#endif /* NETCMD */
	      ckmakmsg(temp,
		       TMPLEN,
		       " ",
		       (network ? "Host" : "Device"),
		       ": ",
		       ttname
		       );
	    conoll(temp);
	    if (!network && speed >= 0L) {
		sprintf(temp,"Speed %ld", speed);
		conoll(temp);
	    }
	    sprintf(temp," Terminal echo: %s", duplex ? "local" : "remote");
	    conoll(temp);
	    sprintf(temp," Terminal bytesize: %d", (cmask  == 0177) ? 7 : 8);
	    conoll(temp);
	    sprintf(temp," Command bytesize: %d", (cmdmsk == 0177) ? 7 : 8 );
	    conoll(temp);
            if (hwparity)
              sprintf(temp," Parity[hardware]: %s",parnam((char)hwparity));
            else	    
  	      sprintf(temp," Parity: %s", parnam((char)parity));
	    conoll(temp);
	    sprintf(temp," Autodownload: %s", autodl ? "on" : "off");
	    conoll(temp);
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

	case 'z': case '\032':		/* Suspend */
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

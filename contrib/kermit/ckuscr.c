#include "ckcsym.h"

#ifndef NOICP
#ifndef NOSCRIPT
char *loginv = "Script Command, 9.0.032, 16 Oct 2009";

/*  C K U S C R  --  expect-send script implementation  */

/*
  Copyright (C) 1985, 2009,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.

  Original (version 1, 1985) author: Herm Fischer, Encino, CA.
  Contributed to Columbia University in 1985 for inclusion in C-Kermit 4.0.
  Maintained since 1985 by Frank da Cruz, Columbia University,
  fdc@columbia.edu.

  The module takes a UUCP-style script of the "expect send [expect send] ..."
  format.  It is intended to operate similarly to the way the common
  UUCP L.sys login entries work.  Conditional responses are supported:
  expect[-send-expect[...]], as with UUCP.  The send keyword EOT sends a
  Control-d, and the keyword BREAK sends a break.  Letters prefixed
  by '~' are '~b' backspace, '~s' space, '~n' linefeed, '~r' return, '~x' xon,
  '~t' tab, '~q' ? (not allowed on kermit command lines), '~' ~, '~'',
  '~"', '~c' don't append return, '~o[o[o]]' octal character.  As with
  some uucp systems, sent strings are followed by ~r (not ~n) unless they
  end with ~c. Null expect strings (e.g., ~0 or --) cause a short
  delay, and are useful for sending sequences requiring slight pauses.

  This module calls externally defined system-dependent functions for
  communications i/o, as defined in ckcplm.txt, the C-Kermit Program Logic
  Manual, and thus should be portable to all systems that implement those
  functions, and where alarm() and signal() work as they do in UNIX.
*/
#include "ckcdeb.h"
#include <signal.h>
#ifdef NT
#include <setjmpex.h>
#else /* NT */
#include <setjmp.h>
#endif /* NT */
#include "ckcasc.h"
#include "ckcker.h"
#include "ckuusr.h"
#include "ckcnet.h"
#include "ckcsig.h"

_PROTOTYP( VOID flushi, (void) );
_PROTOTYP( static VOID myflsh, (void) );
_PROTOTYP( static int sequenc, (void) );
_PROTOTYP( static VOID recvseq, (void) );
_PROTOTYP( static int outseq, (void) );

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
/* VOS doesn't have alarm(), but it does have some things we can work with. */
/* however, we have to catch all the signals in one place to do this, so    */
/* we intercept the signal() routine and call it from our own replacement.  */
#define signal vsignal
#define alarm valarm
SIGTYP (*vsignal(int type, SIGTYP (*func)(int)))(int);
int valarm(int interval);
#endif /* STRATUS */

extern int sessft;
extern int local, flow, seslog, mdmtyp, msgflg, duplex, backgrd, secho, quiet;
extern int network, nettype, ttnproto;
extern long speed;
extern char ttname[];

#ifdef NTSIG
extern int TlsIndex;
#endif /* NTSIG */
#ifdef IKSD
extern int inserver;
#endif /* IKSD */

static int is_tn = 0;			/* Do Telnet negotiations */

#ifndef NOSPL
#ifdef DCMDBUF
extern struct cmdptr *cmdstk;
#else
extern struct cmdptr cmdstk[];
#endif /* DCMDBUF */
extern int techo, cmdlvl;
extern int mecho;
#endif /* NOSPL */

static int scr_echo;			/* Whether to echo script commands */

static int exp_alrm = 15;		/* Time to wait for expect string */
#define SND_ALRM 15			/* Time to allow for sending string */
#define NULL_EXP 2			/* Time to pause on null expect strg*/
#define DEL_MSEC 300			/* Milliseconds to pause on ~d */

#define SBUFL 512
static char seq_buf[SBUFL+2], *s;	/* expect-send sequence buffer */
static int got_it, no_cr;

/*  Connect state parent/child communication signal handlers */

#ifdef COMMENT
#ifdef CK_POSIX_SIG
static sigjmp_buf alrmrng;
#else
static jmp_buf alrmrng;
#endif /* CK_POSIX_SIG */
#else
static ckjmpbuf alrmrng;
#endif /* COMMENT */

static SIGTYP
#ifdef CK_ANSIC
scrtime(int foo)			/* modem read failure handler, */
#else
scrtime(foo) int foo;			/* Alarm handler */
#endif /* CK_ANSIC */
/* scrtime */ {

#ifdef BEBOX
#ifdef BE_DR_7
    alarm_expired();
#endif /* BE_DR_7 */
#endif /* BEBOX */
#ifdef NTSIG
    if (foo == SIGALRM)
      PostAlarmSigSem();
    else
      PostCtrlCSem();
#else /* NTSIG */
#ifdef NT
    cklongjmp(ckjaddr(alrmrng),1);
#else /* NT */
    cklongjmp(alrmrng,1);
#endif /* NT */
#endif /* NTSIG */
    SIGRETURN;
}

/*
 Sequence interpreter -- pick up next sequence from command string,
 decode escapes and place into seq_buf.

 If string contains a ~d (delay) then sequenc() returns a 1 expecting
 to be called again after the ~d executes.
*/
static int
sequenc() {
    int i;
    char c, oct_char;

    no_cr = 0;				/* output needs cr appended */
    for (i = 0; i < SBUFL; ) {
	if (*s == '\0' || *s == '-' || isspace(*s) ) { /* done */
	    seq_buf[i] = '\0';
	    return(0) ;
	}
	if (*s == '~') {		/* escape character */
	    s++;
	    switch (c = *s) {
		case 'n':  seq_buf[i++] = LF; break;
		case 'r':  seq_buf[i++] = CR; break;
		case 't':  seq_buf[i++] = '\t'; break;
		case 'b':  seq_buf[i++] = '\b'; break;
		case 'q':  seq_buf[i++] = '?';  break;
#ifdef COMMENT
/* The default case should catch these now... */
		case '~':  seq_buf[i++] = '~';  break;
		case '-':  seq_buf[i++] = '-';  break;
#endif /* COMMENT */
		case '\'': seq_buf[i++] = '\''; break;
		case '\"': seq_buf[i++] = '\"'; break;
		case 's':  seq_buf[i++] = ' ';  break;
		case 'x':  seq_buf[i++] = '\021'; break;
		case 'c':  no_cr = 1; break;
		case 'd': {			/* send what we have & then */
		    seq_buf[i] = '\0';		/* expect to send rest after */
		    no_cr = 1;			/* sender delays a little */
		    s++;
		    return(1);
		}
		case 'w': {			/* wait count */
		    exp_alrm = 15;		/* default to 15 sec */
		    if (isdigit(*(s+1))) {
			s++;
			exp_alrm = *s & 15;
			if (isdigit(*(s+1)) ) {
			    s++;
			    exp_alrm = exp_alrm * 10 + (*s & 15);
			}
		    }
		    break;
		}
		default:
		    if ( isdigit(c) ) {	    	/* octal character */
		    	oct_char = (char) (c & 7); /* most significant digit */
			if (isdigit( *(s+1) ) ) {
			    s++;
			    oct_char = (char) ((oct_char<<3) | ( *s & 7 ));
			    if (isdigit( *(s+1) ) ) {
				s++;
			    	oct_char = (char) ((oct_char<<3) | ( *s & 7 ));
			    }
			}
			seq_buf[i++] = oct_char;
			break;
		    } else seq_buf[i++] = *s; /* Treat ~ as quote */
	      }
	} else seq_buf[i++] = *s;	/* Plain old character */
	s++;
    }
    seq_buf[i] = '\0';
    return(0);				/* end of space, return anyway */
}


/* Output buffering for "recvseq" and "flushi" */

#define	MAXBURST 256		/* maximum size of input burst */
static CHAR conbuf[MAXBURST];	/* buffer to hold output for console */
static int concnt = 0;		/* number of characters buffered */
static CHAR sesbuf[MAXBURST];	/* buffer to hold output for session log */
static int sescnt = 0;		/* number of characters buffered */

static VOID
myflsh() {
    if (concnt > 0) {
	conxo(concnt, (char *) conbuf);
	concnt = 0;
    }
    if (sescnt > 0) {
        logstr((char *) sesbuf, sescnt);
	sescnt = 0;
    }
}

/* these variables are used to pass data between the recvseq() */
/* and the dorseq().  They are necessary because in some versions */
/* dorseq() is executed in a separate thread and data cannot be */
/* passed by parameter. */

static char *rseqe, * rseqgot, * rseqtrace ;
static int rseql;

static SIGTYP
#ifdef CK_ANSIC
dorseq(void * threadinfo)
#else /* CK_ANSIC */
dorseq(threadinfo) VOID * threadinfo;
#endif /* CK_ANSIC */
/* dorseq */ {
    int i, x;
    int burst = 0;			/* chars remaining in input burst */

#ifdef NTSIG
    setint();
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

    while (!got_it) {
	for (i = 0; i < rseql-1; i++) rseqgot[i] = rseqgot[i+1];
	x = ttinc(0);			/* Read a character */
	debug(F101,"recvseq","",x);
	if (x < 0) {
#ifdef NTSIG
	    ckThreadEnd(threadinfo);
#endif /* NTSIG */
	    SIGRETURN;			/* Check for error */
	}
#ifdef NETCONN
#ifdef TNCODE
/* Check for telnet protocol negotiation */
	if (((x & 0xff) == IAC) && is_tn) { /* Telnet negotiation */
	    myflsh();
	    burst = 0;
	    switch (tn_doop((CHAR)(x & 0xff),duplex,ttinc)) {
	      case 2: duplex = 0; continue;
	      case 1: duplex = 1;
	      default: continue;
	    }
	}
#endif /* TNCODE */
#endif /* NETCONN */
	rseqgot[rseql-1] = (char) (x & 0x7f); /* Got a character */
	burst--;			/* One less waiting */
	if (scr_echo) conbuf[concnt++] = rseqgot[rseql-1]; /* Buffer it */
	if (seslog)			/* Log it in session log */
#ifdef UNIX
	  if (sessft != 0 || rseqgot[rseql-1] != '\r')
#else
#ifdef OSK
	    if (sessft != 0 || rseqgot[rseql-1] != '\012')
#endif /* OSK */
#endif /* UNIX */
	      if (rseqgot[rseql-1])	/* Filter out NULs */
		sesbuf[sescnt++] = rseqgot[rseql-1];
	if ((int)strlen(rseqtrace) < SBUFL-2 )
	  strcat(rseqtrace,dbchr(rseqgot[rseql-1]));
	got_it = (!strncmp(rseqe, rseqgot, rseql));
	if (burst <= 0) {		/* Flush buffered output */
	    myflsh();
	    if ((burst = ttchk()) < 0) { /* Get size of next input burst */
#ifdef NTSIG
		ckThreadEnd(threadinfo);
#endif /* NTSIG */
		SIGRETURN;
	    }
	    /* prevent overflow of "conbuf" and "sesbuf" */
	    if (burst > MAXBURST)
	      burst = MAXBURST;
	}
    }
#ifdef NTSIG
    ckThreadEnd(threadinfo);
#endif /* NTSIG */
    SIGRETURN;
}

static SIGTYP
#ifdef CK_ANSIC
failrseq(void * threadinfo)
#else /* CK_ANSIC */
failrseq(threadinfo) VOID * threadinfo;
#endif /* CK_ANSIC */
/* failrseq */ {
     got_it = 0;			/* Timed out here */
     SIGRETURN;
}

/*
  Receive sequence -- see if expected response comes,
  return success (or failure) in got_it.
*/
static VOID
recvseq() {
    char *e, got[7], trace[SBUFL];
    int i, l;

    sequenc();
    l = (int)strlen(e=seq_buf);		/* no more than 7 chars allowed */
    if (l > 7) {
	e += l-7;
	l = 7;
    }
    tlog(F111,"expecting sequence",e,(long) l);
    if (l == 0) {			/* null sequence, delay a little */
	sleep (NULL_EXP);
	got_it = 1;
	tlog(F100,"got it (null sequence)","",0L);
	return;
    }
    *trace = '\0';
    for (i = 0; i < 7; i++) got[i]='\0';

    rseqtrace = trace;
    rseqe = e;
    rseqgot = got;
    rseql = l;

    alrm_execute(ckjaddr(alrmrng), exp_alrm, scrtime, dorseq, failrseq);

    tlog(F110,"received sequence: ",trace,0L);
    tlog(F101,"returning with got-it code","",(long) got_it);
    myflsh();				/* Flush buffered output */
    return;
}

/*
 Output A Sequence starting at pointer s,
 return 0 if okay,
 1 if failed to read (modem hangup or whatever)
*/
static int oseqret = 0;			/* Return code for outseq */
					/* Out here to prevent clobbering */
					/* by longjmp. */

static SIGTYP
#ifdef CK_ANSIC
dooseq(void * threadinfo)
#else /* CK_ANSIC */
dooseq(threadinfo) VOID * threadinfo;
#endif /* CK_ANSIC */
{
    int l;
    char *sb;
#ifdef TCPSOCKET
    extern int tn_nlm, tn_b_nlm;
#endif /* TCPSOCKET */

#ifdef NTSIG
    setint();
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

    l = (int)strlen(seq_buf);
    tlog(F111,"sending sequence ",seq_buf,(long) l);

    if (!strcmp(seq_buf,"EOT")) {
	ttoc(dopar('\004'));
	if (scr_echo) conol("<EOT>");
	if (seslog && duplex)
            logstr("<EOT>",5);
    } else if (!strcmp(seq_buf,"BREAK") ||
	       !strcmp(seq_buf,"\\b") ||
	       !strcmp(seq_buf,"\\B")) {
	ttsndb();
	if (scr_echo) conol("<BREAK>");
	if (seslog)
	  logstr("{BREAK}",7);
    } else {
	if (l > 0) {
	    for ( sb = seq_buf; *sb; sb++)
	      *sb = dopar(*sb);	/* add parity */
	    ttol((CHAR *)seq_buf,l); /* send it */
	    if (scr_echo && duplex) {
#ifndef NOLOCAL
#ifdef OS2
		{			/* Echo to emulator */
		    char *s = seq_buf;
		    while (*s) {
			scriptwrtbuf((USHORT)*s);
		    }
		}
#endif /* OS2 */
#endif /* NOLOCAL */
		conxo(l,seq_buf);
	    }
	    if (seslog && duplex) /* log it */
	      logstr(seq_buf,strlen(seq_buf));
	}
	if (!no_cr) {
	    ttoc( dopar(CR) );
#ifdef TCPSOCKET
	    if (is_tn) {
		if (!TELOPT_ME(TELOPT_BINARY) && tn_nlm != TNL_CR)
		  ttoc((char)((tn_nlm == TNL_CRLF) ?
			      dopar(LF) : dopar(NUL)));
		else if (TELOPT_ME(TELOPT_BINARY) &&
			 (tn_b_nlm == TNL_CRLF || tn_b_nlm == TNL_CRNUL))
		  ttoc((char)((tn_b_nlm == TNL_CRLF) ?
			      dopar(LF) : dopar(NUL)));
	    }
#endif /* TCPSOCKET */
	    if (seslog && duplex)
	      logchar(dopar(CR));
	}
    }
#ifdef NTSIG
    ckThreadEnd(threadinfo);
#endif /* NTSIG */
    SIGRETURN;
}

SIGTYP
#ifdef CK_ANSIC
failoseq(void * threadinfo)
#else /* CK_ANSIC */
failoseq(threadinfo) VOID * threadinfo;
#endif /* CK_ANSIC */
/* failoseq */ {
     oseqret = -1;		/* else -- alarm rang */
     SIGRETURN;
}

static int
outseq() {
    int delay;

    oseqret = 0;			/* Initialize return code */
    while(1) {
	delay = sequenc();
	alrm_execute( ckjaddr(alrmrng), SND_ALRM, scrtime, dooseq, failoseq ) ;

	if (!delay)
	  return(oseqret);
#ifndef MAC
	msleep(DEL_MSEC);		/* delay, loop to next send */
#endif /* MAC */
    }
}


/*  L O G I N  --  (historical misnomer) Execute the SCRIPT command */

int
dologin(cmdstr) char *cmdstr; {

#ifdef OS2
#ifdef NT
    SIGTYP (* savealm)(int);		/* Save incoming alarm function */
#else /* NT */
    SIGTYP (* volatile savealm)(int);	/* Save incoming alarm function */
#endif /* NT */
#else /* OS2 */
    SIGTYP (*savealm)();		/* Save incoming alarm function */
#endif /* OS2 */
    char *e;

    s = cmdstr;				/* Make global to this module */

    tlog(F100,loginv,"",0L);

    if (speed < 0L) speed = ttgspd();
    if (ttopen(ttname,&local,mdmtyp,0) < 0) {
	ckmakmsg(seq_buf,SBUFL,"Sorry, can't open ",ttname,NULL,NULL);
	perror(seq_buf);
	return(0);
    }
    /* Whether to echo script commands ... */
    scr_echo = (!quiet && !backgrd && secho);
#ifndef NOSPL
    if (scr_echo && cmdlvl > 1) {
	if (cmdstk[cmdlvl].src == CMD_TF)
	  scr_echo = techo;
	if (cmdstk[cmdlvl].src == CMD_MD)
	  scr_echo = mecho;
    }
#endif /* NOSPL */
    if (scr_echo) {
#ifdef NETCONN
	if (network)
	  printf("Executing SCRIPT to host %s.\n",ttname);
	else
#endif /* NETCONN */
	  printf("Executing SCRIPT through %s, speed %ld.\n",ttname,speed);
    }
#ifdef TNCODE
    /* TELNET input must be scanned for IAC */
    is_tn = (local && network && IS_TELNET()) ||
	    (!local && sstelnet);
#endif /* TNCODE */

    *seq_buf = 0;
    for (e = s; *e; e++) ckstrncat(seq_buf,dbchr(*e),SBUFL);
#ifdef COMMENT
/* Skip this because it tends to contain a password... */
    if (scr_echo) printf("SCRIPT string: %s\n",seq_buf);
#endif /* COMMENT */
    tlog(F110,"SCRIPT string: ",seq_buf, 0L);

/* Condition console terminal and communication line... */

    if (ttvt(speed,flow) < 0) {
	printf("Sorry, Can't condition communication line\n");
	return(0);
    }
    /* Save initial timer interrupt value */
    savealm = signal(SIGALRM,SIG_IGN);

    flushi();				/* Flush stale input */

/* start expect - send sequence */

    while (*s) {			/* While not done with buffer */

	while (*s && isspace(*s)) s++;	/* Skip over separating whitespaces */
					/* Gather up expect sequence */
	got_it = 0;
	recvseq();

	while (!got_it) {		/* Have it yet? */
	    if (*s++ != '-')		/* No, is there a conditional send? */
	      goto failret;		/* No, return failure */
	    flushi();			/* Yes, flush out input buffer */
	    if (outseq())		/* If unable to send, */
	      goto failret;		/* return failure. */
	    if (*s++ != '-')		/* If no conditional response here, */
	      goto failret;		/* return failure. */
	    recvseq();			/* All OK, read response from host. */
	}				/* Loop back and check got_it */

	while (*s && !isspace(*s++) ) ;	/* Skip over conditionals */
	while (*s && isspace(*s)) s++;	/* Skip over separating whitespaces */
	flushi();			/* Flush */
	if (*s) if (outseq()) goto failret; /* If any */
    }
    signal(SIGALRM,savealm);
    if (scr_echo) printf("Script successful.\n");
    tlog(F100,"Script successful.","",0L);
    return(1);

failret:
    signal(SIGALRM,savealm);
    if (scr_echo) printf("Sorry, script failed\n");
    tlog(F100,"Script failed","",0L);
    return(0);
}

/*  F L U S H I  --  Flush, but log, SCRIPT input buffer  */

VOID
flushi() {
    int n, x;
    if (
	seslog				/* Logging session? */
	|| scr_echo			/* Or console echoing? */
#ifdef NETCONN
#ifdef TNCODE
	/* TELNET input must be scanned for IAC */
	|| is_tn
#endif /* TNCODE */
#endif /* NETCONN */
	) {
        if ((n = ttchk()) < 0)		/* Yes, anything in buffer? */
	  return;
	if (n > MAXBURST) n = MAXBURST;	/* Make sure not too much, */
	myflsh();			/* and that buffers are empty. */
	while (n-- > 0) {
  	    x = ttinc(0);		/* Collect a character */
#ifdef NETCONN
#ifdef TNCODE
/* Check for telnet protocol negotiation */
  	    if (is_tn && ((x & 0xff) == IAC) ) {
		myflsh();		/* Sync output */
  		switch (tn_doop((CHAR)(x & 0xff),duplex,ttinc)) {
  		  case 2: duplex = 0; break;
  		  case 1: duplex = 1;
		  default: break;
		}

		/* Recalculate flush count */
		if ((n = ttchk()) < 0)
		  return;
		if (n > MAXBURST) n = MAXBURST;
  		continue;
  	    }
#endif /* TNCODE */
#endif /* NETCONN */
	    if (scr_echo) conbuf[concnt++] = (CHAR) x; /* buffer for console */
	    if (seslog)
#ifdef UNIX
	      if (sessft != 0 || x != '\r')
#else
#ifdef OSK
	      if (sessft != 0 || x != '\012')
#endif /* OSK */
#endif /* UNIX */
		sesbuf[sescnt++] = (CHAR) x; /* buffer for session log */
  	}
	myflsh();
    } else ttflui();			/* Otherwise just flush. */
}

#else /* NOSCRIPT */
char *loginv = "Script Command Disabled";
#endif /* NOSCRIPT */
#endif /* NOICP */

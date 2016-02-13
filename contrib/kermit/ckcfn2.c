/*  C K C F N 2  --  System-independent Kermit protocol support functions... */

/*  ...Part 2 (continued from ckcfns.c)  */

/*
  Author: Frank da Cruz <fdc@columbia.edu>,
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 2011,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/
/*
 Note -- if you change this file, please amend the version number and date at
 the top of ckcfns.c accordingly.
*/

#include "ckcsym.h"			/* Compilation options */
#include "ckcdeb.h"			/* Debugging and other symbols */
#include "ckcasc.h"			/* ASCII symbols */
#include "ckcker.h"			/* Kermit symbols */
#include "ckcxla.h"			/* Translation */
#include "ckcnet.h"			/* IKS and VMS #define TCPSOCKET */
#ifdef TCPSOCKET			/* For TELNET business in spack() */
extern int tn_nlm, ttnproto, tn_b_nlm;
#endif /* TCPSOCKET */

extern int parity, network, local, interrupted, fatalio, wasclosed;

int kstartactive = 0;			/* Flag for kstart() in a packet */

static CHAR p_tbl[] = {			/* Even parity table for dopar(). */
    (CHAR) '\000',			/* ANSI C casts '\ooo' constants  */
    (CHAR) '\201',			/* to signed char, so we have to  */
    (CHAR) '\202',			/* cast back to unsigned char...  */
    (CHAR) '\003',
    (CHAR) '\204',
    (CHAR) '\005',
    (CHAR) '\006',
    (CHAR) '\207',
    (CHAR) '\210',
    (CHAR) '\011',
    (CHAR) '\012',
    (CHAR) '\213',
    (CHAR) '\014',
    (CHAR) '\215',
    (CHAR) '\216',
    (CHAR) '\017',
    (CHAR) '\220',
    (CHAR) '\021',
    (CHAR) '\022',
    (CHAR) '\223',
    (CHAR) '\024',
    (CHAR) '\225',
    (CHAR) '\226',
    (CHAR) '\027',
    (CHAR) '\030',
    (CHAR) '\231',
    (CHAR) '\232',
    (CHAR) '\033',
    (CHAR) '\234',
    (CHAR) '\035',
    (CHAR) '\036',
    (CHAR) '\237',
    (CHAR) '\240',
    (CHAR) '\041',
    (CHAR) '\042',
    (CHAR) '\243',
    (CHAR) '\044',
    (CHAR) '\245',
    (CHAR) '\246',
    (CHAR) '\047',
    (CHAR) '\050',
    (CHAR) '\251',
    (CHAR) '\252',
    (CHAR) '\053',
    (CHAR) '\254',
    (CHAR) '\055',
    (CHAR) '\056',
    (CHAR) '\257',
    (CHAR) '\060',
    (CHAR) '\261',
    (CHAR) '\262',
    (CHAR) '\063',
    (CHAR) '\264',
    (CHAR) '\065',
    (CHAR) '\066',
    (CHAR) '\267',
    (CHAR) '\270',
    (CHAR) '\071',
    (CHAR) '\072',
    (CHAR) '\273',
    (CHAR) '\074',
    (CHAR) '\275',
    (CHAR) '\276',
    (CHAR) '\077',
    (CHAR) '\300',
    (CHAR) '\101',
    (CHAR) '\102',
    (CHAR) '\303',
    (CHAR) '\104',
    (CHAR) '\305',
    (CHAR) '\306',
    (CHAR) '\107',
    (CHAR) '\110',
    (CHAR) '\311',
    (CHAR) '\312',
    (CHAR) '\113',
    (CHAR) '\314',
    (CHAR) '\115',
    (CHAR) '\116',
    (CHAR) '\317',
    (CHAR) '\120',
    (CHAR) '\321',
    (CHAR) '\322',
    (CHAR) '\123',
    (CHAR) '\324',
    (CHAR) '\125',
    (CHAR) '\126',
    (CHAR) '\327',
    (CHAR) '\330',
    (CHAR) '\131',
    (CHAR) '\132',
    (CHAR) '\333',
    (CHAR) '\134',
    (CHAR) '\335',
    (CHAR) '\336',
    (CHAR) '\137',
    (CHAR) '\140',
    (CHAR) '\341',
    (CHAR) '\342',
    (CHAR) '\143',
    (CHAR) '\344',
    (CHAR) '\145',
    (CHAR) '\146',
    (CHAR) '\347',
    (CHAR) '\350',
    (CHAR) '\151',
    (CHAR) '\152',
    (CHAR) '\353',
    (CHAR) '\154',
    (CHAR) '\355',
    (CHAR) '\356',
    (CHAR) '\157',
    (CHAR) '\360',
    (CHAR) '\161',
    (CHAR) '\162',
    (CHAR) '\363',
    (CHAR) '\164',
    (CHAR) '\365',
    (CHAR) '\366',
    (CHAR) '\167',
    (CHAR) '\170',
    (CHAR) '\371',
    (CHAR) '\372',
    (CHAR) '\173',
    (CHAR) '\374',
    (CHAR) '\175',
    (CHAR) '\176',
    (CHAR) '\377'
};

/*  D O P A R  --  Add an appropriate parity bit to a character  */

CHAR
#ifdef CK_ANSIC
dopar(register CHAR ch)
#else
dopar(ch) register CHAR ch;
#endif /* CK_ANSIC */
    {
    register unsigned int a;
    if (!parity
#ifdef TCPSOCKET
        || (network && (ttnproto == NP_TELNET) && (TELOPT_ME(TELOPT_BINARY)))
#ifndef NOXFER
	|| (!local && sstelnet)		/* TELNET BINARY MODE */
#endif /* NOXFER */
#endif /* TCPSOCKET */
        ) return((CHAR) (ch & 255)); else a = ch & 127;
    switch (parity) {
	case 'e':  return(p_tbl[a]);	             /* Even */
	case 'm':  return((CHAR) (a | 128));         /* Mark */
	case 'o':  return((CHAR) (p_tbl[a] ^ 128));  /* Odd */
	case 's':  return((CHAR) a);	             /* Space */
	default:   return((CHAR) a);                 /* Something illegal */
    }
}

#ifndef NOXFER				/* Rest of this file... */

#define NEWDPL				/* New dynamic packet length method */

#ifdef VMS
extern int batch;
#else
extern int backgrd;
#endif /* VMS */

#ifdef DYNAMIC
extern struct pktinfo *s_pkt;		/* array of pktinfo structures */
extern struct pktinfo *r_pkt;		/* array of pktinfo structures */
#else
extern struct pktinfo s_pkt[];		/* array of pktinfo structures */
extern struct pktinfo r_pkt[];		/* array of pktinfo structures */
#endif /* DYNAMIC */

extern int sseqtbl[], rseqtbl[], sbufuse[], sacktbl[], wslots, winlo, wslotn,
  sbufnum, rbufnum, pktpaus, reliable;

#ifdef STREAMING
static int dontsend = 0;
extern int streaming;
#endif /* STREAMING */

extern int ttprty;			/* from ck*tio.c */
extern int autopar;

extern int spsiz, spmax, rpsiz, timint, timef, npad, bestlen, maxsend;
extern int rpt, rptq, rptflg, capas, spsizf, en_fin, tsecs, flow;
extern int pktnum, sndtyp, rcvtyp, bctr, bctu, bctf, bctl, rsn, rln, maxtry;
extern int size, osize, maxsize, spktl, rpktl, nfils, stdouf, fsecs;
extern int turn, turnch, displa, pktlog, seslog, xflg, mypadn;
extern int hcflg, server, cxseen, czseen, discard, slostart;
extern int nakstate, quiet, success, xitsta, what, filestatus;
extern int spackets, rpackets, timeouts, retrans, crunched, urpsiz;
extern int carrier, fdispla, srvidl;

#ifdef GFTIMER
extern CKFLOAT fptsecs, fpfsecs, fpxfsecs;
#endif /* GFTIMER */

extern long filcnt, filrej, speed, filcps, tfcps;
extern CK_OFF_T ffc, flci, flco, tlci, tlco, tfc;

extern char *cmarg, filnam[];

extern CHAR padch, mypadc, eol, seol, ctlq, sstate;
extern CHAR *recpkt, *data, myinit[];
extern CHAR *srvptr, stchr, mystch, *rdatap;
extern CHAR padbuf[];
extern CHAR * epktmsg;
extern int epktrcvd, epktsent;

#ifdef OS2				/* AUTODOWNLOAD parameters */
extern int adl_kmode, adl_zmode;	/* Match Packet to signal download */
extern char * adl_kstr;			/* KERMIT Download String */
extern char * adl_zstr;			/* ZMODEM Download String */
#endif /* OS2 */

#ifdef CK_AUTODL
CHAR ksbuf[96] = { NUL, NUL };		/* Autodownload "Kermit Start" buf */
#endif /* CK_AUTODL */

int numerrs = 0;			/* Number of packet errors so far */
int rcvtimo = 0;			/* Timeout for receiving a packet */
int idletmo = 0;			/* Flag for idle timeout */

long filcps = 0L;			/* CPS most recent file transferred */
long tfcps  = 0L;			/* CPS most recent transaction */
long xfsecs = 0L;			/* Elapsed time for most recent file */
#ifdef GFTIMER
CKFLOAT fpxfsecs = 0.0;			/* Ditto, but floating point */
#endif /* GFTIMER */

#ifdef CK_TIMERS
int rrttbl[64], srttbl[64];		/* Packet timestamp tables */
extern int rttflg;
#define RTT_SCALE 1000
long
  rttsamples,				/* Round trip time samples */
  rttdelay,				/* RTT delay */
  pktintvl,				/* Interpacket arrival time */
  rttvariance,				/* RTT variance */
  rttstddev;				/* RTT standard deviation */
#endif /* CK_TIMERS */

/* CRC generation tables */

long crcta[16] = { 0L, 010201L, 020402L, 030603L, 041004L,
  051205L, 061406L, 071607L, 0102010L, 0112211L, 0122412L, 0132613L, 0143014L,
  0153215L, 0163416L, 0173617L
};

long crctb[16] = { 0L, 010611L, 021422L, 031233L, 043044L,
  053655L, 062466L, 072277L, 0106110L, 0116701L, 0127532L, 0137323L, 0145154L,
  0155745L, 0164576L, 0174367L
};

#ifdef CK_TIMERS
/*
  Round-trip timer calculations adapted from Tim Kientzle's article,
  "Improving Kermit Performance", Dr Dobb's Journal, February 1996.
*/


/*  R T T I N I T  --  Initialize timers at start of transaction  */

VOID
rttinit() {				/* Initialize round-trip timing */
    int i;

    if (timint == 0)
      return;

    rttsamples  = 0L;			/* Samples (packets) */
    rttvariance = 0L;			/* Variance in delay */
    rttdelay    = (long) timint * RTT_SCALE; /* Delay */
    pktintvl    = (long) timint * RTT_SCALE; /* Delay */
    rttstddev   = (long) timint * RTT_SCALE; /* Standard deviation of delay */

    /* Tables of timestamps indexed by packet sequence number */

    for (i = 0; i < 64; i++) {
	rrttbl[i] = -1;			/* Time each packet was received */
	srttbl[i] = -1;			/* Time each packet was sent */
    }
    rcvtimo = timint;			/* Initial timeout is what user said */
}

/*  G E T R T T  --  Get packet round trip time  */
/*
  Call with nakstate == 0 if file sender, nonzero if receiver,
  and n == packet sequence number of the packet we just received.

  Returns:
  -1 on failure with rcvtimo set to timint (what the user said), or:
   0 on success with rcvtimo set to dynamically calculated value:
     1 <= rcvtimo <= timint * 3.
*/
int
getrtt(nakstate, n) int nakstate, n; {
    extern int mintime, maxtime;
    static int prevz = 0, prevr = 0;
    int x, y, yy, z = 0, zz = 0;	/* How long did it take to get here? */

    rcvtimo = timint;			/* Default timeout is what user said */

    if (timint == 0)			/* We're not timing out. */
      return(0);

    if (!rttflg)			/* Not supposed to be doing this? */
      return(-1);			/*  So don't */

    if (!RTT_SCALE)			/* Paranoia... */
      return(-1);

    /* rtimer() (reset timer) is not called until 1st data packet */
#ifdef GFTIMER
    /* rftimer(); */
#endif /* GFTIMER */
    /* S (F [ A ] D* Z)* B */

    /* NOTE: we calculate both the round-trip time AND the packet */
    /* arrival rate.  We don't use the RTT for anything, we just display it. */
    /* Timeouts are based on the packet arrival rate. */

    if (spackets > 3) {			/* Don't start till 4th packet */
	if (nakstate) {			/* File receiver */
	    x = rrttbl[n];		     /* Time when I got packet n */
	    y = rrttbl[n > 0 ? n - 1 : 63];  /* Time when I got packet n-1 */
	    yy = srttbl[n > 0 ? n - 1 : 63]; /* Time when I sent ACK(n-1) */
	    if (x > -1 && y > -1) {	/* Be careful */
		z = x - y;		/* Packet rate */
		zz = x - yy;		/* Round trip time */
		z++;			/* So sender & receiver differ */
		debug(F101,"RTT RECV","",z);
	    } else {			/* This shouldn't happen */
		debug(F101,"RTT RECV ERROR spackets","",spackets);
		debug(F101,"RTT RECV ERROR sequence","",n);
		return(-1);
	    }
	} else {			/* File sender */
	    x = rrttbl[n];		/* Time when I got ACK(n) */
	    y = rrttbl[n > 0 ? n - 1 : 63]; /* Time when I got packet n-1 */
	    yy = srttbl[n];		/* Time when I sent n */
	    if (x > -1 && y > -1) {
		z = x - y;		/* Packet rate */
		zz = x - yy;		/* Round trip time */
		debug(F101,"RTT SEND","",z);
	    } else {
		debug(F100,"RTT SEND ERROR","",0);
		return(-1);
	    }
	}
	if (z < 1)			/* For fast connections */
	  z = RTT_SCALE / 2;		/* Convert to scale... */
	else
	  z *= RTT_SCALE;
	debug(F101,"RTT z scaled","",z);

	if (zz < 1)			/* For fast connections */
	  zz = RTT_SCALE / 2;		/* Convert to scale... */
	else
	  zz *= RTT_SCALE;

	rttdelay = zz;			/* Round trip time of this packet */
#ifdef COMMENT
/*
  This was used in C-Kermit 7.0 (and 6.0?) but not only is it overkill,
  it also can produce ridiculously long timeouts under certain conditions.
  Replaced in 8.0 by a far simpler and more aggressive strategy.
*/
	if (rttsamples++ == 0L) {	/* First sample */
	    pktintvl = z;
	} else {			/* Subsequent samples */
	    long oldavg = pktintvl;
	    long rttdiffsq;

	    if (rttsamples > 30)	/* Use real average for first 30 */
	      rttsamples = 30;		/*  then decaying average. */

	    /* Average delay, difference squared, variance, std deviation */

	    pktintvl += (z - pktintvl) / rttsamples;
	    rttdiffsq = (z - oldavg) * (z - oldavg);
	    rttvariance += (rttdiffsq - rttvariance) / rttsamples;
	    debug(F101,"RTT stddev1","",rttstddev);
	    if (rttstddev < 1L)		/* It can be zero, in which case */
	      rttstddev = RTT_SCALE / 3; /* set it to something small... */
	    rttstddev = (rttstddev + rttvariance / rttstddev) / 2;
	}
	debug(F101,"RTT stddev2","",rttstddev);
	debug(F101,"RTT delay  ","",pktintvl);
	rcvtimo = (pktintvl + (3L * rttstddev)) / RTT_SCALE + 1;
	if (rpackets < 32)		/* Allow for slow start */
	  rcvtimo += rcvtimo + 2;
	else if (rpackets < 64)
	  rcvtimo += rcvtimo / 2 + 1;
        /* On a reliable link, don't try too hard to time out. */
	/* Especially on fast local network connections. */
        if (server && what == W_NOTHING) /* Server command wait */
	  rcvtimo = rcvtimo;		/* == srvtim */
        else if (reliable == SET_ON && rcvtimo > 0) /* Reliable */
	  rcvtimo = rcvtimo +15;	/* and not server command wait */
        else                            /* Not reliable or server cmd wait */
	  rcvtimo = rcvtimo;
	if (rcvtimo < mintime)		/* Lower bound */
	  rcvtimo = mintime;
	if (maxtime > 0) {		/* User specified an upper bound */
	    if (rcvtimo > maxtime)
	      rcvtimo = maxtime;
	} else if (maxtime == 0) {	/* User didn't specify */
	    if (rcvtimo > timint * 6)
	      rcvtimo = timint * 6;
	}
#else  /* COMMENT */
#ifdef CKFLOAT
	{
	    CKFLOAT x;
	    x = (CKFLOAT)(prevz + z + z) / 3.0;
	    rcvtimo = (int)((((CKFLOAT)x * 2.66) / RTT_SCALE) + 0.5);
	    debug(F101,"RTT rcvtimo (float)","",rcvtimo);
	}
#else
	rcvtimo = (prevz + z + z) / RTT_SCALE;
	debug(F101,"RTT rcvtimo (int)","",rcvtimo);
#endif /* CKFLOAT */
#endif /* COMMENT */

	zz = (rttdelay + 500) / 1000;
	if (rcvtimo > (zz * 3))
	  rcvtimo = zz * 3;

	if (rcvtimo < 1)
	  rcvtimo = 1;
	if (mintime > 0) {
	    if (rcvtimo < mintime)	/* Lower bound */
	      rcvtimo = mintime;
	}
	if (maxtime > 0) {		/* Upper bound */
	    if (rcvtimo > maxtime)
	      rcvtimo = maxtime;
	}
	if (rcvtimo == (prevr - 1))
	  rcvtimo++;

	debug(F101,"RTT final rcvtimo","",rcvtimo);
    }
    prevz = z;
    prevr = rcvtimo;
    return(0);
}
#endif /* CK_TIMERS */

/*  I N P U T  --  Attempt to read packet number 'pktnum'.  */

/*
 This is the function that feeds input to Kermit's finite state machine,
 in the form of a character in the range 32-126, normally a packet type
 (uppercase letter) or pseudo-packet-type (lowercase letter).

 If a special start state is in effect, that state is returned as if it were
 the type of an incoming packet.
*/
int
input() {
    int type = 0, acktype;		/* Received packet type */
    int x, y, k;			/* Workers */
    int z, pi, nf;			/* Worker, packet index, NAK flag */
    int nak2ack = 0;

    debug(F000,"input sstate","",sstate);
    debug(F101,"input nakstate","",nakstate);
    debug(F000,"input sndtyp","",sndtyp);
    debug(F101,"input xitsta","",xitsta);
    debug(F101,"input what","",what);

    while (1) {				/* Big loop... */
/*
  It is ttchk()'s responsibility to tell us if the connection is broken,
  and to do so instantly and nondestructively -- no blocking, etc, that would
  slow down file transfer.
*/
	if (ttchk() < 0) {
	    debug(F100,"input CONNECTION BROKEN","",0);
	    fatalio = 1;
	    return('q');
	}
	if (sstate != 0) {		/* If a start state is in effect, */
	    type = sstate;		/* return it like a packet type, */
	    sstate = 0;			/* and then nullify it. */
	    numerrs = 0;		/* (PWP) no errors so far */
	    return(type);
	}
	if (nakstate) {			/* This section for file receiver. */
	    if (wslots > 1) {		/* If we're doing windows, */
		x = rseqtbl[winlo];	/* see if desired packet already in. */
		debug(F101,"input winlo","",winlo);
		debug(F101,"input rseqtbl[winlo]","",rseqtbl[winlo]);
		if (x > -1) {		/* Already there? */
		    if (r_pkt[x].pk_seq == winlo) { /* (double check) */
			rsn = winlo;	            /* Yes, return its info */
			debug(F101,"input return pre-stashed packet","",rsn);
			dumprbuf();
			rdatap = r_pkt[x].pk_adr;   /* like rpack would do. */
			rln = (int)strlen((char *) rdatap);
			type = r_pkt[x].pk_typ;
			break;
		    }
		}
	    }
	    type = rpack();	        /* Try to read a packet. */
	    debug(F101,"input rpack","",type);

	    while (type == 'e') {	/* Handle echoes */
		debug(F101,"input echo discarded","",type);
		type = rpack();
	    }
#ifdef DEBUG
	    if (deblog) {
		if (type == 'D')
		  debug(F011,"input type D=",(char *)rdatap,39);
		else
		  debug(F000,"input type",(char *)rdatap,type);
	    }
#endif /* DEBUG */
#ifndef OLDCHKINT
	    if (type == 'z') {
		epktrcvd = 1;
		errpkt((CHAR *)"User cancelled.");
		type = 'E';
		break;
	    }
#endif /* OLDCHKINT */
	    if (type < -1) {
		char * s;
		s = (type == -2) ?
		  "FAILED - Interrupted" :
		    "FAILED - Connection lost";

		xxscreen(SCR_PT,'q',0L,s);
		dologend();
		return('q');		/* Ctrl-C or connection lost */
	    }
	    if (type < 0) {		/* Receive window full */
		/* Another thing to do here would be to delete */
		/* the highest packet and NAK winlo.  But that */
		/* shouldn't be necessary since the other Kermit */
		/* should not have sent a packet outside the window. */
#ifdef COMMENT
                char foo[256];
                ckmakxmsg(foo,256,"Receive window full (rpack): wslots=",
                          ckitoa(wslots)," winlo=",ckitoa(winlo)," pktnum=",
                          ckitoa(pktnum), NULL,NULL,NULL,NULL,NULL,NULL);
		errpkt((CHAR *)foo);
                debug(F100,foo,"",0);
#else
		errpkt((CHAR *)"Receive window full");
		debug(F101,"rpack receive window full","",0);
                debug(F101," wslots","",wslots);
                debug(F101," winlo","",winlo);
		debug(F101," pktnum","",pktnum);
#endif
		dumprbuf();
		type = 'E';
		break;
	    }
	    dumprbuf();

#ifdef OLDCHKINT
	    if (chkint() < 0) {		/* Check for console interrupts. */
		errpkt((CHAR *)"User cancelled."); /* (old way) */
		type = 'E';
		break;
	    }
#endif /* OLDCHKINT */

#ifdef STREAMING
	    if (streaming) {		/* Streaming */
		if (type == 'Q' || type == 'T') { /* Errors are fatal. */
		    crunched++;		/* For statistics */
		    errpkt((CHAR *)"Transmission error on reliable link.");
		    type = 'E';
		}
	    }
#endif /* STREAMING */
	    if (type == 'E') {
		debug(F101,"input got E, nakstate","",nakstate);
		break;			/* Error packet */
	    }
	    if (type == 'Q') {		/* Crunched packet. */
		crunched++;
		numerrs++;
/*
  Packet arrived damaged.  It was most likely the packet we were expecting
  next, so we send a NAK for that packet.  Prior to 5A(189), we always
  NAK'd winlo here, but that was bad because if two (or more) different
  packets were damaged, we would keep NAKing the first one and never NAK the
  other ones, which could result in a lengthy series of timeouts.  Now we
  NAK the oldest as-yet-unNAK'd missing packet.
*/
#ifdef CK_TIMERS
		rcvtimo++;		/* Stretch the timeout a little */
#endif /* CK_TIMERS */
		z = (winlo + wslots) % 64;  /* Search from winlo to z */
		debug(F101,"ZZZ crunched z","",z);
		nf = 0;			    /* NAK flag not set yet */
		for (x = winlo; x != z; x = (x + 1) % 64) {
		    debug(F101,"ZZZ x","",x);
		    if (rseqtbl[x] > -1) /* Have I received packet x? */
		      continue;		 /* Yes, go on. */
		    debug(F101,"ZZZ x not recd yet","",x);
		    pi = sseqtbl[x];	 /* No, have I NAK'd it yet? */
		    if (pi < 0 || s_pkt[pi].pk_rtr == 0) {
			debug(F101,"ZZZ x not NAK'd yet","",x);
			nack(x);	 /* No, NAK it now. */
			nf = 1;		 /* Flag that I did. */
			break;
		    }
		}
		if (!nf) {		/* If we didn't NAK anything above, */
		    debug(F101,"ZZZ NAKing winlo","",winlo);
		    if (nack(winlo) < 0) { /* we have to NAK winlo (again) */
			errpkt((CHAR *)"Too many retries."); /* Too many */
			type = 'E';
			break;
		    }
		}
		continue;
	    }

	    if (type == 'T') {		/* Timeout */
#ifndef OS2
		/* K95 does this its own way */
		if (server && srvidl) {
		    idletmo = 1;
		    debug(F101,"SERVER IDLE TIMEOUT","",srvidl);
		    return('q');
		}
#endif /* OS2 */
#ifdef CK_TIMERS
		rcvtimo++;		/* Stretch the timeout a little */
#endif /* CK_TIMERS */
		timeouts++;
		debug(F101,"input receive-state timeout, winlo","",winlo);
		/* NAK only the packet at window-low */
		debug(F101,"input sending NAK for winlo","",winlo);
		x = ttchk();
		if (x > 0)		/* Don't give up if there is still */
		  continue;		/* something to read. */
		else if (x < 0) {
		    dologend();
		    fatalio = 1;
		    return('q');	/* Connection Lost */
		}
		if (nack(winlo) < 0) {
		    debug(F101,"input sent too many naks","",winlo);
		    errpkt((CHAR *)"Too many retries.");
		    type = 'E';
		    break;
		} else continue;
	    }
	    if (rsn == winlo) {		/* Got the packet we want, done. */
#ifdef CK_TIMERS
		if (rttflg && timint)	/* Dynamic round trip timers? */
		  getrtt(nakstate, rsn); /* yes, do it. */
                else                     /* JHD 20100208 */
                  rcvtimo = timint;      /* JHD 20100208 */
#endif /* CK_TIMERS */
		debug(F101,"input rsn=winlo","",rsn);
		break;
	    }

	    /* Got a packet out of order. */

	    debug(F101,"input out of sequence, rsn","",rsn);
	    k = rseqtbl[rsn];		/* Get window slot of this packet. */
	    debug(F101,"input rseqtbl[rsn]","",k);
	    if (k < 0) {
		debug(F101,"input recv can't find index for rcvd pkt","",rsn);
		/* Was "Internal error 21" */
		/* This should not happen  */
		errpkt((CHAR *)"Sliding windows protocol error.");
		type = 'E';
		break;
	    }
	    y = chkwin(rsn,winlo,wslots); /* See what window it's in. */
	    debug(F101,"input recv chkwin","",y);
	    if (y == 1) {		/* From previous window. */
#ifdef STREAMING
		if (!streaming)		/* NO RESEND IF STREAMING! */
#endif /* STREAMING */
		  resend(rsn);		/* Resend the ACK (might have data) */
		freerpkt(rsn);		/* Get rid of received packet */
		continue;		/* Back to wait for another packet */
	    } else {			/* In this window or out of range */
		if (y < 0)		/* If out of range entirely, */
		  freerpkt(rsn);	/* release its buffer */

#ifdef STREAMING
		if (streaming) {	/* Streaming (this shouldn't happen) */
		    errpkt((CHAR *)"Sequence error on reliable link.");
		    type = 'E';
		    break;
		}
#endif /* STREAMING */

/* If our receive window is full, NAK window-low */

		if (rbufnum < 1) {	/* Receive window full? */
		    if (nack(winlo) < 0) {    /* No choice, must NAK winlo. */
			errpkt((CHAR *)"Too many retries."); /* Too many */
			type = 'E';
			break;
		    } else continue;
		}
/*
  Receive window not full.  This is a packet in the current window but it is
  not the desired packet at winlo.  So therefore there are gaps before this
  packet.  So we find the "lowest" unNAK'd missing packet, if any, between
  winlo and this one, and NAK it.  If there are no as-yet-unNAK'd missing
  packets in the window, then we send nothing and go wait for another packet.
  In theory, this could result in a timeout, but in practice it is likely that
  the already-NAK'd missing packets are already on their way.  Note, we do not
  NAK ahead of ourselves, as that only creates unnecessary retransmissions.
*/
		for (x = winlo; x != rsn; x = (x + 1) % 64) {
		    if (rseqtbl[x] > -1) /* Have I received packet x? */
		      continue;		 /* Yes, check next sequence number. */
		    pi = sseqtbl[x];	 /* No, have I NAK'd it yet? */
		    if (pi < 0 || s_pkt[pi].pk_rtr == 0) {
			nack(x);	 /* No, NAK it now. */
			break;
		    }
		}
	    }
/*!!!*/
	} else {			/* Otherwise file sender... */

#ifdef STREAMING
	    if (streaming && sndtyp == 'D') {
		debug(F101,"STREAMING input streaming","",streaming);
		debug(F000,"STREAMING input sndtyp","",sndtyp);
		rsn = winlo;
		type = 'Y';		/* Pretend we got an ACK */
	    }
#endif /* STREAMING */
	    if (!nak2ack) {		/* NAK(n+1) = ACK(n) */
		if (wslots > 1) {	/* Packet at winlo already ACK'd? */
		    if (sacktbl[winlo]) { /* If so,  */
			sacktbl[winlo] = 0; /* Turn off the ACK'd flag */
			winlo = (winlo + 1) % 64; /* Rotate the window */
			type = 'Y';	/* And return ACK */
			debug(F101,
			      "input send returning pre-stashed ACK","",
			      winlo-1);
			break;
		    }
		}
#ifdef STREAMING
		if (!(streaming && sndtyp == 'D')) { /* Not streaming | data */
		    type = rpack();	/* Try to read an acknowledgement */
		} else {		/* Streaming and in Data phase */
		    type = 'Y';		/* Assume all is normal */
		    if (chkint() < 0)	/* Check for console interrupts. */
		      type = 'z';
		    else if (ttchk() > 4 + bctu) /* Check for return traffic */
		      type = rpack();
		    debug(F000,"input streaming type","",type);
		}
#endif /* STREAMING */
		debug(F111,"input send",(char *) rdatap,(int) type);
		while (type == 'e') {	/* Handle echoes */
		    debug(F000,"echo discarded","",type);
		    type = rpack();
		}
#ifndef OLDCHKINT
		if (type == 'z') {
		    epktrcvd = 1;
		    errpkt((CHAR *)"User cancelled.");
		    type = 'E';
		    break;
		}
#endif /* OLDCHKINT */
		if (type < -1) {
		    xxscreen(SCR_PT,'q',0L,
			   ((char *)((type == -2) ?
			   "Interrupted" :
			   "Connection lost"))
			   );
		    if (type != -2)
		      dologend();
		    return('q');	/* Ctrl-C or connection lost */
		}
		if (type == -1) {
#ifdef COMMENT
                    char foo[256];
                    ckmakxmsg(foo,256,
			      "Receive window full (error 18): wslots=",
                              ckitoa(wslots),
			      " winlo=",ckitoa(winlo)," pktnum=",
                              ckitoa(pktnum), NULL,NULL,NULL,NULL,NULL,NULL);
		    errpkt((CHAR *)foo);
                    debug(F100,foo,"",0);
#else
		    errpkt((CHAR *)"Receive window full"); /* was "internal */
                    debug(F101," wslots","",wslots); /* error 18" */
                    debug(F101," winlo","",winlo);
		    debug(F101," pktnum","",pktnum);
#endif /* COMMENT */
		    dumprbuf();
		    type = 'E';
		    break;
		}
		dumprbuf();		/* Debugging */

#ifdef OLDCHKINT
		if (chkint() < 0) {	/* Check for console interrupts. */
		    errpkt((CHAR *)"User cancelled.");
		    return(type = 'E');
		}
#endif /* OLDCHKINT */

		/* Got a packet */

#ifdef STREAMING
		if (streaming) {		/* Streaming */
		    if (type == 'Q' || type == 'T') { /* Errors are fatal. */
			crunched++;	/* For statistics */
			errpkt((CHAR *)"Transmission error on reliable link.");
			type = 'E';
		    }
		}
#endif /* STREAMING */
		if (type == 'E') {
		    debug(F101,"input send got E, nakstate","",nakstate);
		    break;		/* Error packet */
		}
		if (type == 'Q') {	/* Crunched packet */
		    crunched++;		/* For statistics */
		    numerrs++;		/* For packet resizing */
		    x = resend(winlo);	/* Resend window-low */
		    if (x < 0) {
			type = 'E';
			errpkt((CHAR *)"Too many retries");
			break;
		    }
		    continue;
		}
		if (type == 'T') {	/* Timeout waiting for ACKs. */
		    timeouts++;		/* Count it */
		    numerrs++;		/* Count an error too */
		    debug(F101,"input send state timeout, winlo","",winlo);

		    /* Retransmit the oldest un-ACK'd packet. */

		    debug(F101,"input send resending winlo","",winlo);
		    if (resend(winlo) < 0) { /* Check retries */
			debug(F101,"input send too many resends","",maxtry);
			errpkt((CHAR *)"Too many retries");
			return(type = 'E');
		    }
#ifdef NEWDPL
		    /* Reduce prevailing packet length */
		    x = sseqtbl[winlo];	/* Get length of packet we want ACKd */
		    if (x > -1) {	/* Only if we have a valid index */
			if (s_pkt[x].pk_typ == 'D') { /* only for D packets */
			    spsiz = (s_pkt[x].pk_len + 8) >> 1; /* halve it */
			    if (spsiz < 20) spsiz = 20; /* within reason */
			    debug(F101,"input T cut packet length","",spsiz);
			}
		    }
#endif /* NEWDPL */
		    continue;
		}
	    }
	    /* Got an actual normal packet */

	    nak2ack = 0;		/* Unset this flag. */
	    y = chkwin(rsn,winlo,wslots); /* Is it in the window? */
	    debug(F101,"input send rsn","",rsn);
	    debug(F101,"input send winlo","",winlo);
	    debug(F101,"input send chkwin","",y);

	    if (type == 'Y') {		/* Got an ACK */
		if (y == 0) {		/* In current window */
		    if (spackets < 4)	/* Error counter doesn't count */
		      numerrs = 0;	/* until data phase. */
		    sacktbl[rsn]++;	/* Mark the packet as ACK'd */
		    x = sseqtbl[rsn];	/* Get ACK'd packet's buffer index */
		    debug(F101,"bestlen ack x","",x);
#ifdef NEWDPL
		    if (x > -1) {
			acktype = s_pkt[x].pk_typ; /* Get type */
			debug(F000,"bestlen ack type","",acktype);

			if (acktype == 'D') { /* Adjust data packet length */
			    if (spsiz > bestlen) {
				bestlen = spsiz;
				debug(F101,"bestlen B","",bestlen);
			    }
#ifdef DEBUG
			    if (deblog) {
				debug(F101,"bestlen retry","",s_pkt[x].pk_rtr);
				debug(F101,"bestlen len","",s_pkt[x].pk_len);
				debug(F101,"bestlen spackets","",spackets);
			    }
#endif /* DEBUG */
			    /* Set new best length */
			    if (s_pkt[x].pk_rtr == 0 &&
				s_pkt[x].pk_len + 8 > bestlen) {
				bestlen = s_pkt[x].pk_len + 8;
				if (bestlen > spmax)
				  bestlen = spmax;
				debug(F101,"bestlen A","",bestlen);
			    }
#ifdef DEBUG
			    if (deblog) {
				debug(F101,"bestlen wslots","",wslots);
				debug(F101,"bestlen maxsend","",maxsend);
			    }
#endif /* DEBUG */
			    /* Slow start */
			    if (slostart &&
				(maxsend <= spmax) &&
				(rpackets < 11) &&
				(numerrs == 0)) {
				spsiz = spsiz << 1;
				debug(F101,"bestlen spsiz A","",spsiz);

			    /* Creep up to best length */
			    } else if ((spackets > 5) &&
				       (spsiz < bestlen - 8)) {
				spsiz += (bestlen - spsiz) / 3;
				debug(F101,"bestlen spsiz B","",spsiz);

			    /* Push the envelope */
			    } else if ((spackets % (wslots + 1) == 0) &&
				       (spackets > 6) &&
				       (bestlen < spmax - 8) &&
				       (spsiz < spmax)) {
				spsiz += (spmax - bestlen) / 3;
				debug(F101,"bestlen spsiz C","",spsiz);
			    }
			    /* But not too far */
			    if (spsiz > spmax) {
				spsiz = spmax;
				debug(F101,"bestlen spsiz D","",spsiz);
			    }
			}
		    }
#endif /* NEWDPL */

#ifdef CK_TIMERS
		    if (rttflg && timint) /* If doing dynamic timers */
		      getrtt(nakstate, rsn); /* call routine to set it. */
		    else                     /* JHD 20100208 */
		      rcvtimo = timint;	     /* JHD 20100208 */
#endif /* CK_TIMERS */
/*
  NOTE: The following statement frees the buffer of the ACK we just got.
  But the upper layers still need the data, like if it's the ACK to an I,
  S, F, D, Z, or just about any kind of packet.  So for now, freerbuf()
  deallocates the buffer, but does not erase the data or destroy the pointer
  to it.  There's no other single place where these receive buffers can be
  correctly freed (?) ...
*/
		    freerpkt(rsn);	/* Free the ACK's buffer */
		    freesbuf(rsn);	/* *** Free the sent packet's buffer */
		    if (rsn == winlo) {	/* Got the one we want */
			sacktbl[winlo] = 0;
			winlo = (winlo + 1) % 64;
			debug(F101,"input send rotated send window","",winlo);
			break;		/* Return the ACK */
		    } else {
			debug(F101,"input send mark pkt","",rsn);
			continue;	/* Otherwise go read another packet */
		    }
		} else if (y == 1 && wslots < 2) { /* (190) ACK for previous */
		    numerrs++;		/* == NAK for current, count error */
		    debug(F101,"input send ACK for previous","",rsn);
		    freerpkt(rsn);	/* Free NAK's buffer */
		    x = resend(winlo);	/* Resend current packet */
		    if (x < 0) {
			type = 'E';
			errpkt((CHAR *)"Too many retries");
			break;
		    } else continue;	/* Resend ok, go read another packet */
		} else {		/* Other cases, just ignore */
		    debug(F101,"input send ACK out of window","",rsn);
		    freerpkt(rsn);
		    continue;
		}
	    }
	    if (type == 'N') {		/* NAK */
		numerrs++;		/* Count an error */
#ifdef STREAMING
		if (streaming) {		/* Streaming */
		    errpkt((CHAR *)"NAK received on reliable link.");
		    type = 'E';
		    break;
		}
#endif /* STREAMING */

		debug(F101,"input send NAK","",rsn);
#ifdef NEWDPL
		/* Reduce prevailing packet length */
		x = sseqtbl[rsn];	/* Length of packet that was NAK'd */
		if (x > -1) {		/* If it's a Data packet we've sent */
		    if (s_pkt[x].pk_typ == 'D') {
			spsiz = (s_pkt[x].pk_len + 8) >> 1; /* Halve length */
#ifdef COMMENT
			/* This might be a good idea -- haven't tried it ... */
			if (bestlen > 0 && spsiz > bestlen)
			  spsiz = bestlen;
#endif /* COMMENT */
			if (spsiz < 20) spsiz = 20;
			debug(F101,"input N cut packet length","",spsiz);
		    }
		}
#endif /* NEWDPL */
		freerpkt(rsn);		/* Free buffer where NAK lies. */
		if (y == 0) {		/* In current window */
		    debug(F100," in window","",0);
		    k = sseqtbl[rsn];	/* Get pointer to NAK'd packet. */
		    if (k < 0 || (k > -1 && s_pkt[k].pk_typ == ' ')) {
			x = resend(winlo); /* Packet we haven't sent yet. */
		    } else {
			x = resend(rsn); /* Resend requested packet. */
		    }
		    if (x < 0) {	/* Resend error is fatal.  */
			type = 'E';
			errpkt((CHAR *)"Too many retries");
			break;
		    } else continue;	/* Resend ok, go read another packet */
		} else if ((rsn == (pktnum + 1) % 64)) { /* NAK for next pkt */
		    if (wslots > 1) {
			debug( F101,"NAK for next packet, windowing","",rsn);
			x = resend(winlo); /* Resend window-low */
			if (x < 0) {
			    type = 'E';
			    errpkt((CHAR *)"Too many retries");
			    break;
			}
			continue;	/* Go back and read another pkt */
		    }
		    debug(F101,"NAK for next packet, no windowing","",rsn);
		    x = (rsn == 0) ? 63 : rsn - 1;
		    if (x == 0 && (sndtyp == 'S' || sndtyp == 'I')) {
			resend(0);	/* ACK for S or I packet missing */
			continue;	/* so resend the S or I */
		    }
		    rsn = x;		/* Else, treat NAK(n+1) as ACK(n) */
		    nak2ack = 1;	/* Go back and process the ACK */
		    continue;
		} else if (y > 0) {	/* NAK for pkt we can't resend */
		    debug(F101," NAK out of window","",rsn); /* bad... */
		    type = 'E';
		    errpkt((CHAR *)"NAK out of window");
		    break;
		} else continue;	/* Ignore other NAKs */
	    }				/* End of file-sender NAK handler */

            if (rsn == winlo) {		/* Not ACK, NAK, timeout, etc. */
		debug(F000,"input send unexpected type","",type);
		break;
	    }
	}				/* End of file-sender section */
    }					/* End of input() while() loop */
/*
  When the window size is 1 and we have the packet we want, there can not
  possibly be anything waiting for us on the connection that is useful to us.
  However, there might be redundant copies of a packet we already got, which
  would cause needless cycles of repeated packets.  Therefore we flush the
  communications input buffer now to try to get rid of undesired and unneeded
  packets that we have not read yet.

  Actually, the first sentence above is not entirely true: there could be an
  Error packet waiting to be read.  Flushing an E packet is bad because it
  will not be resent, and we'll go into a cycle of timing out and
  retransmitting up to the retry limit.  - fdc 2007/03/02
*/
    if (wslotn == 1			/* (not wslots!) */
#ifdef STREAMING
	&& !streaming			/* But not when streaming */
#endif /* STREAMING */
	) {
	debug(F100,"input about to flush","",0);
	ttflui();		/* Got what we want, clear input buffer. */
    }
#ifndef NEWDPL
    if (!nakstate)			/* When sending */
      rcalcpsz();			/* recalculate size every packet */
#endif /* NEWDPL */
    if (type == 'E')
      xitsta |= (what ? what : 1);	/* Remember what failed. */
    debug(F101,"input winlo","",winlo);
    debug(F101,"input rsn","",rsn);
    debug(F000,"input returning type","",type);
    return(rcvtyp = type);		/* Success, return packet type. */
}

#ifdef PARSENSE
/*  P A R C H K  --  Check if Kermit packet has parity  */

/*
  Call with s = pointer to packet, start = packet start character, n = length.
  Returns 0 if packet has no parity, -1 on error, or, if packet has parity:
    'e' for even, 'o' for odd, 'm' for mark.  Space parity cannot be sensed.
  So a return value of 0 really means either space or none.
  Returns -2 if parity has already been checked during this protocol operation.
*/
int
#ifdef CK_ANSIC
parchk(CHAR *s, CHAR start, int n)
#else
parchk(s,start,n) CHAR *s, start; int n;
#endif /* CK_ANSIC */
/* parchk */ {
    CHAR s0, s1, s2, s3;

    debug(F101,"parchk n","",n);
    debug(F101,"parchk start","",start);

    s0 = s[0] & 0x7f;			/* Mark field (usually Ctrl-A) */

    if (s0 != start || n < 5) return(-1); /* Not a valid packet */

/* Look at packet control fields, which never have 8th bit set */
/* First check for no parity, most common case. */

    if (((s[0] | s[1] | s[2] | s[3]) & 0x80) == 0)
      return(0);			/* No parity or space parity */

/* Check for mark parity */

    if (((s[0] & s[1] & s[2] & s[3]) & 0x80) == 0x80)
      return('m');			/* Mark parity */

/* Packet has some kind of parity */
/* Make 7-bit copies of control fields */

    s1 = s[1] & 0x7f;			/* LEN */
    s2 = s[2] & 0x7f;			/* SEQ */
    s3 = s[3] & 0x7f;			/* TYPE */

/* Check for even parity */

    if ((s[0] == p_tbl[s0]) &&
        (s[1] == p_tbl[s1]) &&
        (s[2] == p_tbl[s2]) &&
	(s[3] == p_tbl[s3]))
      return('e');

/* Check for odd parity */

    if ((s[0] != p_tbl[s0]) &&
        (s[1] != p_tbl[s1]) &&
        (s[2] != p_tbl[s2]) &&
	(s[3] != p_tbl[s3]))
      return('o');

/* Otherwise it's probably line noise.  Let checksum calculation catch it. */

    return(-1);
}
#endif /* PARSENSE */

/*
  Check to make sure timeout intervals are long enough to allow maximum
  length packets to get through before the timer goes off.  If not, the
  timeout interval is adjusted upwards.

  This routine is called at the beginning of a transaction, before we
  know anything about the delay characteristics of the line.  It works
  only for serial communication devices; it trusts the speed reported by
  the operating system.

  Call with a timout interval.  Returns it, adjusted if necessary.
*/
int
chktimo(timo,flag) int timo, flag; {
    long cps, z; int x, y;
#ifdef STREAMING
    debug(F101,"chktimo streaming","",streaming);
    if (streaming)
      return(0);
#endif /* STREAMING */

    debug(F101,"chktimo timo","",timo); /* Timeout before adjustment */
    debug(F101,"chktimo flag","",flag);

    if (flag)				/* Don't change timeout if user */
      return(timo);			/* gave SET SEND TIMEOUT command. */
    debug(F101,"chktimo spmax","",spmax);
    debug(F101,"chktimo urpsiz","",urpsiz);

    if (!network) {			/* On serial connections... */
	speed = ttgspd();		/* Get current speed. */
	if (speed > 0L) {
	    cps = speed / 10L;		/* Convert to chars per second */
	    if (cps > 0L) {
		long plen;		/* Maximum of send and rcv pkt size */
		z = cps * (long) timo;	/* Chars per timeout interval */
		z -= z / 10L;		/* Less 10 percent */
		plen = spmax;
		if (urpsiz > spmax) plen = urpsiz;
		debug(F101,"chktimo plen","",plen);
		if (z < plen) {		/* Compare with packet size */
		    x = (int) ((long) plen / cps); /* Adjust if necessary */
		    y = x / 10;		/* Add 10 percent for safety */
		    if (y < 2) y = 2;	/* Or 2 seconds, whichever is more */
		    x += y;
		    if (x > timo)	/* If this is greater than current */
		      timo = x;		/* timeout, change the timeout */
		    debug(F101,"chktimo new timo","",timo);
		}
	    }
	}
    }
    return(timo);
}

/*  S P A C K  --  Construct and send a packet  */

/*
  spack() sends a packet of the given type, sequence number n, with len data
  characters pointed to by d, in either a regular or extended- length packet,
  depending on len.  Returns the number of bytes actually sent, or else -1
  upon failure.  Uses global npad, padch, mystch, bctu, data.  Leaves packet
  fully built and null-terminated for later retransmission by resend().
  Updates global sndpktl (send-packet length).

  NOTE: The global pointer "data" is assumed to point into the 7th position
  of a character array (presumably in packet buffer for the current packet).
  It was used by getpkt() to build the packet data field.  spack() fills in
  the header to the left of the data pointer (the data pointer is defined
  in getsbuf() in ckcfn3.c).  If the address "d" is the same as "data", then
  the packet's data field has been built "in place" and need not be copied.
*/
int
#ifdef CK_ANSIC
spack(char pkttyp, int n, int len, CHAR *d)
#else
spack(pkttyp,n,len,d) char pkttyp; int n, len; CHAR *d;
#endif /* CK_ANSIC */
/* spack */ {
    register int i;
    int ix, j, k, x, lp, longpkt, copy, loglen;

#ifdef GFTIMER
    CKFLOAT t1 = 0.0, t2 = 0.0;
#endif /* GFTIMER */

    register CHAR *cp, *mydata;
    unsigned crc;

    copy = (d != data);			/* Flag whether data must be copied  */

#ifdef DEBUG
    if (deblog) {			/* Save lots of function calls! */
	debug(F101,"spack n","",n);
#ifdef COMMENT
	if (pkttyp != 'D') {		/* Data packets would be too long */
	    debug(F111,"spack data",data,data);
	    debug(F111,"spack d",d,d);
	}
#endif	/* COMMENT */
	debug(F101,"spack len","",len);
	debug(F101,"spack copy","",copy);
    }
#endif /* DEBUG */

    longpkt = (len + bctl + 2) > 94;	/* Decide whether it's a long packet */
    mydata = data - 7 + (longpkt ? 0 : 3); /* Starting position of header */
    k = sseqtbl[n];			/* Packet structure info for pkt n */
#ifdef COMMENT
#ifdef DEBUG
    if (deblog) {			/* Save 2 more function calls... */
	debug(F101,"spack mydata","",mydata);
	debug(F101,"spack sseqtbl[n]","",k);
	if (k < 0) {
#ifdef STREAMING
	    if (!streaming)
#endif /* STREAMING */
	      debug(F101,"spack sending packet out of window","",n);
	}
    }
#endif /* DEBUG */
#endif	/* COMMENT */
    if (k > -1) {
	s_pkt[k].pk_adr = mydata;	/* Remember address of packet. */
	s_pkt[k].pk_seq = n;		/* Record sequence number */
	s_pkt[k].pk_typ = pkttyp;	/* Record packet type */
    }
    spktl = 0;				/* Initialize length of this packet */
    i = 0;				/* and position in packet. */

/* Now fill the packet */

    mydata[i++] = mystch;		/* MARK */
    lp = i++;				/* Position of LEN, fill in later */

    mydata[i++] = tochar(n);		/* SEQ field */
    mydata[i++] = pkttyp;		/* TYPE field */
    j = len + bctl;			/* Length of data + block check */
    if (longpkt) {			/* Long packet? */
	int x;				/* Yes, work around SCO Xenix/286 */
#ifdef CKTUNING
	unsigned int chk;
#endif /* CKTUNING */
	x = j / 95;			/* compiler bug... */
        mydata[lp] = tochar(0);		/* Set LEN to zero */
        mydata[i++] = tochar(x);	/* Extended length, high byte */
        mydata[i++] = tochar(j % 95);	/* Extended length, low byte */
#ifdef CKTUNING
        /* Header checksum - skip the function calls and loops */
	  chk = (unsigned) mydata[lp]   +
	        (unsigned) mydata[lp+1] +
	        (unsigned) mydata[lp+2] +
		(unsigned) mydata[lp+3] +
		(unsigned) mydata[lp+4] ;
	mydata[i++] = tochar((CHAR) ((((chk & 0300) >> 6) + chk) & 077));
#else
        mydata[i] = '\0';		/* Terminate for header checksum */
        mydata[i++] = tochar(chk1(mydata+lp,5));
#endif /* CKTUNING */
    } else mydata[lp] = tochar(j+2);	/* Normal LEN */
/*
  When sending a file, the data is already in the right place.  If it weren't,
  it might make sense to optimize this section by using memcpy or bcopy
  (neither of which are portable), but only if our packets were rather long.
  When receiving, we're only sending ACKs so it doesn't matter.  So count the
  following loop as a sleeping dog.
*/
    if (copy)				/* Data field built in place? */
      for ( ; len--; i++) mydata[i] = *d++; /* No, must copy. */
    else				/* Otherwise, */
      i += len;				/* Just skip past data field. */
    mydata[i] = '\0';			/* Null-terminate for checksum calc. */

    switch (bctu) {			/* Block check */
        case 1:				/* 1 = 6-bit chksum */
	    ix = i - lp;		/* Avoid "order of operation" error */
	    mydata[i++] = tochar(chk1(mydata+lp,ix));
	    break;
	case 2:				/* 2 = 12-bit chksum */
	    j = chk2(mydata+lp,i-lp);
	    mydata[i++] = (unsigned)tochar((j >> 6) & 077);
   	    mydata[i++] = (unsigned)tochar(j & 077);
	    break;
        case 3:				/* 3 = 16-bit CRC */
	    crc = chk3(mydata+lp,i-lp);
	    mydata[i++] = (unsigned)tochar(((crc & 0170000)) >> 12);
	    mydata[i++] = (unsigned)tochar((crc >> 6) & 077);
	    mydata[i++] = (unsigned)tochar(crc & 077);
	    break;
	case 4:				/* 2 = 12-bit chksum, blank-free */
	    j = chk2(mydata+lp,i-lp);
	    mydata[i++] =
	      (unsigned)(tochar((unsigned)(((j >> 6) & 077) + 1)));
   	    mydata[i++] = (unsigned)(tochar((unsigned)((j & 077) + 1)));
	    break;
    }
    loglen = i;
    mydata[i++] = seol;			/* End of line (packet terminator) */
#ifdef TCPSOCKET
/*
  If TELNET connection and packet terminator is carriage return,
  we must stuff either LF or NUL, according to SET TELNET NEWLINE-MODE
  (tn_nlm), to meet the TELNET NVT specification, unless user said RAW.

  If NEWLINE-MODE is set to LF instead of CR, we still send CR-NUL
  on a NVT connection and CR on a binary connection.
*/
    if (
#ifdef STREAMING
	!dontsend &&
#endif /* STREAMING */
	((network && ttnproto == NP_TELNET) || (!local && sstelnet))
	&& seol == CR) {
        switch (TELOPT_ME(TELOPT_BINARY) ? tn_b_nlm : tn_nlm) {
	  case TNL_CR:			/* NVT or BINARY */
	    break;
	  case TNL_CRNUL:
	    mydata[i++] = NUL;
	    break;
	  case TNL_CRLF:
	    mydata[i++] = LF;
	    break;
	}
    }
#endif /* TCPSOCKET */
    mydata[i] = '\0';			/* Terminate string */
    if (
#ifdef STREAMING
	!dontsend &&
#endif /* STREAMING */
	pktlog
	)				/* Save a function call! */
      logpkt('s',n,mydata,loglen);	/* Log the packet */

    /* (PWP) add the parity quickly at the end */
    if (parity) {
	switch (parity) {
	  case 'e':			/* Even */
	    for (cp = &mydata[i-1]; cp >= mydata; cp--)
	      *cp = p_tbl[*cp];
	    break;
	  case 'm':			/* Mark */
	    for (cp = &mydata[i-1]; cp >= mydata; cp--)
	      *cp |= 128;
	    break;
	  case 'o':			/* Odd */
	    for (cp = &mydata[i-1]; cp >= mydata; cp--)
	      *cp = p_tbl[*cp] ^ 128;
	    break;
	  case 's':			/* Space */
	    for (cp = &mydata[i-1]; cp >= mydata; cp--)
	      *cp &= 127;
	    break;
	}
    }
    if (pktpaus) msleep(pktpaus);	/* Pause if requested */
    x = 0;

    if (npad) {
#ifdef STREAMING
	if (dontsend)
	  x = 0;
	else
#endif /* STREAMING */
	  x = ttol(padbuf,npad);	/* Send any padding */
    }
    if (x > -1) {
#ifdef CK_TIMERS
	if (timint > 0) {
	    if (pkttyp == 'N')
	      srttbl[n > 0 ? n-1 : 63] = gtimer();
	    else
	      srttbl[n] = gtimer();
	}
#endif /* CK_TIMERS */
	spktl = i;			/* Remember packet length */
	if (k > -1)
	  s_pkt[k].pk_len = spktl;	/* also in packet info structure */

#ifdef DEBUG
#ifdef GFTIMER
/*
  This code shows (in the debug log) how long it takes write() to execute.
  Sometimes on a congested TCP connection, it can surprise you -- 90 seconds
  or more...
*/
	if (
#ifdef STREAMING
	    !dontsend &&
#endif /* STREAMING */
	    deblog
	    )
	  t1 = gftimer();
#endif /* GFTIMER */
#endif /* DEBUG */

#ifdef STREAMING
	if (dontsend) {
	    debug(F000,"STREAMING spack skipping","",pkttyp);
	    x = 0;
	} else
#endif /* STREAMING */
	x = ttol(mydata,spktl);		/* Send the packet */
    }
#ifdef STREAMING
    if (!dontsend) {
#endif /* STREAMING */
	debug(F101,"spack spktl","",spktl);
	debug(F101,"spack ttol returns","",x);
	if (x < 0) {			/* Failed. */
	    if (local && x < -1) {
		xxscreen(SCR_ST,ST_ERR,0L,"FAILED: Connection lost");
		/* We can't send an E packet because the connection is lost. */
		epktsent = 1;		/* So pretend we sent one. */
		fatalio = 1;		/* Remember we got a fatal i/o error */
		dologend();
		ckstrncpy((char *)epktmsg,"Connection lost",PKTMSGLEN);
	    }
	    return(x);
	}
	if (spktl > maxsend)		/* Keep track of longest packet sent */
	  maxsend = spktl;
#ifdef DEBUG
#ifdef GFTIMER
	if (deblog)  {			/* Log elapsed time for write() */
	    t2 = gftimer();
	    debug(F101,"spack ttol msec","",(long)((t2-t1)*1000.0));
	}
#endif /* GFTIMER */
#endif /* DEBUG */
#ifdef STREAMING
    }
#endif /* STREAMING */

    sndtyp = pkttyp;			/* Remember packet type for echos */
#ifdef STREAMING
    if (!dontsend) {			/* If really sent, */
	spackets++;			/* count it. */
	flco += spktl;			/* Count the characters */
	tlco += spktl;			/* for statistics... */
#ifdef DEBUG
	if (deblog) {			/* Save two function calls! */
	    dumpsbuf();			/* Dump send buffers to debug log */
	    debug(F111,"spack calling screen, mydata=",mydata,n);
	}
#endif /* DEBUG */
    }
#endif /* STREAMING */
    if (local) {
	int x = 0;
	if (fdispla != XYFD_N) x = 1;
	if ((fdispla == XYFD_B) && (pkttyp == 'D' || pkttyp == 'Y')) x = 0;
	if (x)
	  xxscreen(SCR_PT,pkttyp,(long)n,(char *)mydata); /* Update screen */
    }
    return(spktl);			/* Return length */
}

/*  C H K 1  --  Compute a type-1 Kermit 6-bit checksum.  */

int
chk1(pkt,len) register CHAR *pkt; register int len; {
    register unsigned int chk;
#ifdef CKTUNING
#ifdef COMMENT
    register unsigned int m;		/* Avoid function call */
    m = (parity) ? 0177 : 0377;
    for (chk = 0; len-- > 0; pkt++)
      chk += *pkt & m;
#else
    chk = 0;
    while (len-- > 0) chk += (unsigned) *pkt++;
#endif /* COMMENT */
#else
    chk = chk2(pkt,len);
#endif /* CKTUNING */
    chk = (((chk & 0300) >> 6) + chk) & 077;
    debug(F101,"chk1","",chk);
    return((int) chk);
}

/*  C H K 2  --  Compute the numeric sum of all the bytes in the packet.  */

unsigned int
chk2(pkt,len) register CHAR *pkt; register int len; {
    register long chk;
#ifdef COMMENT
    register unsigned int m;
    m = (parity) ? 0177 : 0377;
    for (chk = 0; len-- > 0; pkt++)
      chk += *pkt & m;
#else
    /* Parity has already been stripped */
    chk = 0L;
    while (len-- > 0) chk += (unsigned) *pkt++;
#endif /* COMMENT */
    debug(F101,"chk2","",(unsigned int) (chk & 07777));
    return((unsigned int) (chk & 07777));
}

/*  C H K 3  --  Compute a type-3 Kermit block check.  */
/*
 Calculate the 16-bit CRC-CCITT of a null-terminated string using a lookup
 table.  Assumes the argument string contains no embedded nulls.
*/
#ifdef COMMENT
unsigned int
chk3(pkt,parity,len) register CHAR *pkt; int parity; register int len; {
    register long c, crc;
    register unsigned int m;
    m = (parity) ? 0177 : 0377;
    for (crc = 0; len-- > 0; pkt++) {
	c = crc ^ (long)(*pkt & m);
	crc = (crc >> 8) ^ (crcta[(c & 0xF0) >> 4] ^ crctb[c & 0x0F]);
    }
    return((unsigned int) (crc & 0xFFFF));
}
#else
unsigned int
chk3(pkt,len) register CHAR *pkt; register int len; {
    register long c, crc;
    for (crc = 0; len-- > 0; pkt++) {
	c = crc ^ (long)(*pkt);
	crc = (crc >> 8) ^ (crcta[(c & 0xF0) >> 4] ^ crctb[c & 0x0F]);
    }
    debug(F101,"chk3","",(unsigned int) (crc & 0xFFFF));
    return((unsigned int) (crc & 0xFFFF));
}
#endif /* COMMENT */

/*  N X T P K T  --  Next Packet  */
/*
  Get packet number of next packet to send and allocate a buffer for it.
  Returns:
    0 on success, with global pktnum set to the packet number;
   -1 on failure to allocate buffer (fatal);
   -2 if resulting packet number is outside the current window.
*/
int
nxtpkt() {				/* Called by file sender */
    int j, n, x;

    debug(F101,"nxtpkt pktnum","",pktnum);
    debug(F101,"nxtpkt winlo ","",winlo);
    n = (pktnum + 1) % 64;		/* Increment packet number mod 64 */
    debug(F101,"nxtpkt n","",n);
#ifdef STREAMING
    if (!streaming) {
	x = chkwin(n,winlo,wslots);	/* Don't exceed window boundary */
	debug(F101,"nxtpkt chkwin","",x);
	if (x)
	  return(-2);
	j = getsbuf(n);			/* Get a buffer for packet n */
	if (j < 0) {
	    debug(F101,"nxtpkt getsbuf failure","",j);
	    return(-1);
	}
    }
#endif /* STREAMING */
    pktnum = n;
    return(0);
}

/* Functions for sending ACKs and NAKs */

/* Note, we should only ACK the packet at window-low (winlo) */
/* However, if an old packet arrives again (e.g. because the ACK we sent */
/* earlier was lost), we ACK it again. */

int
ack() {					/* Acknowledge the current packet. */
    return(ackns(winlo,(CHAR *)""));
}

#ifdef STREAMING
int
fastack() {				/* Acknowledge packet n */
    int j, k, n, x;
    n = winlo;

    k = rseqtbl[n];			/* First find received packet n. */
    debug(F101,"STREAMING fastack k","",k);
    freesbuf(n);			/* Free current send-buffer, if any */
    if ((j = getsbuf(n)) < 0) {
	/* This can happen if we have to re-ACK an old packet that has */
        /* already left the window.  It does no harm. */
	debug(F101,"STREAMING fastack can't getsbuf","",n);
    }
    dontsend = 1;
    x = spack('Y',n,0,(CHAR *)"");	/* Now send it (but not really) */
    dontsend = 0;
    if (x < 0) return(x);
    debug(F101,"STREAMING fastack x","",x);
    if (k > -1)
      freerbuf(k);			/* don't need it any more */
    if (j > -1)
      freesbuf(j);			/* and don't need to keep ACK either */
    winlo = (winlo + 1) % 64;
    return(0);
}
#endif /* STREAMING */

int
ackns(n,s) int n; CHAR *s; {		/* Acknowledge packet n */
    int j, k, x;
    debug(F111,"ackns",s,n);

    k = rseqtbl[n];			/* First find received packet n. */
    debug(F101,"ackns k","",k);
    freesbuf(n);			/* Free current send-buffer, if any */
    if ((j = getsbuf(n)) < 0) {
	/* This can happen if we have to re-ACK an old packet that has */
        /* already left the window.  It does no harm. */
	debug(F101,"ackns can't getsbuf","",n);
    }
    x = spack('Y',n,(int)strlen((char *)s),s); /* Now send it. */
    if (x < 0) return(x);
    debug(F101,"ackns winlo","",winlo);
    debug(F101,"ackns n","",n);
    if (n == winlo) {			/* If we're acking winlo */
	if (k > -1)
	  freerbuf(k);			/* don't need it any more */
	if (j > -1)
	  freesbuf(j);			/* and don't need to keep ACK either */
	winlo = (winlo + 1) % 64;
    }
    return(0);
}

int
ackn(n) int n; {			/* Send ACK for packet number n */
    return(ackns(n,(CHAR *)""));
}

int
ack1(s) CHAR *s; {			/* Send an ACK with data. */
    if (!s) s = (CHAR *)"";
    debug(F110,"ack1",(char *)s,0);
    return(ackns(winlo,s));
}

/* N A C K  --   Send a Negative ACKnowledgment. */
/*
 Call with the packet number, n, to be NAK'd.
 Returns -1 if that packet has been NAK'd too many times, otherwise 0.
 Btw, it is not right to return 0 under error conditions.  This is
 done because the -1 code is used for cancelling the file transfer.
 More work is needed here.
*/
int
nack(n) int n; {
    int i, x;

    if (n < 0 || n > 63) {
	debug(F101,"nack bad pkt num","",n);
	return(0);
    } else debug(F101,"nack","",n);
    if ((i = sseqtbl[n]) < 0) {		/* If necessary */
	if (getsbuf(n) < 0) {		/* get a buffer for this NAK */
	    debug(F101,"nack can't getsbuf","",n);
	    return(0);
	} else i = sseqtbl[n];		/* New slot number */
    }
    if (maxtry > 0 && s_pkt[i].pk_rtr++ > maxtry) /* How many? */
      return(-1);			/* Too many... */

/* Note, don't free this buffer.  Eventually an ACK will come, and that */
/* will set it free.  If not, well, it's back to ground zero anyway...  */

    x = spack('N',n,0,(CHAR *) "");	/* NAKs never have data. */
    return(x);
}

#ifndef NEWDPL				/* This routine no longer used */
/*
 * (PWP) recalculate the optimal packet length in the face of errors.
 * This is a modified version of the algorithm by John Chandler in Kermit/370,
 * see "Dynamic Packet Size Control", Kermit News, V2 #1, June 1988.
 *
 * This implementation minimizes the total overhead equation, which is
 *
 *   Total chars = file_chars + (header_len * num_packs)
 *                            + (errors * (header_len + packet_len))
 *
 * Differentiate with respect to number of chars, solve for packet_len, get:
 *
 *   packet_len = sqrt (file_chars * header_len / errors)
 */

/*
 (FDC) New super-simple algorithm.  If there was an error in the most recent
 packet exchange, cut the send-packet size in half, down to a minimum of 20.
 If there was no error, increase the size by 5/4, up to the maximum negotiated
 length.  Seems to be much more responsive than previous algorithm, which took
 forever to recover the original packet length, and it also went crazy under
 certain conditions.

 Here's another idea for packet length resizing that keeps a history of the
 last n packets.  Push a 1 into the left end of an n-bit shift register if the
 current packet is good, otherwise push a zero.  The current n-bit value, w, of
 this register is a weighted sum of the noise hits for the last n packets, with
 the most recent weighing the most.  The current packet length is some function
 of w and the negotiated packet length, like:

   (2^n - w) / (2^n) * (negotiated length)

 If the present resizing method causes problems, think about this one a little
 more.
*/
VOID
rcalcpsz() {

#ifdef COMMENT
/* Old way */
    register long x, q;
    if (numerrs == 0) return;		/* bounds check just in case */

    /* overhead on a data packet is npad+5+bctr, plus 3 if extended packet */
    /* an ACK is 5+bctr */

    /* first set x = per packet overhead */
    if (wslots > 1)			/* Sliding windows */
      x = (long) (npad+5+bctr);		/* packet only, don't count ack */
    else				/* Stop-n-wait */
      x = (long) (npad+5+3+bctr+5+bctr); /* count packet and ack. */

    /* then set x = packet length ** 2 */
    x = x * ( ffc / (CK_OFF_T) numerrs); /* careful of overflow */

    /* calculate the long integer sqrt(x) quickly */
    q = 500;
    q = (q + x/q) >> 1;
    q = (q + x/q) >> 1;
    q = (q + x/q) >> 1;
    q = (q + x/q) >> 1;		/* should converge in about 4 steps */
    if ((q > 94) && (q < 130))	/* break-even point for long packets */
      q = 94;
    if (q > spmax) q = spmax;	/* maximum bounds */
    if (q < 10) q = 10;		/* minimum bounds */
    spsiz = q;			/* set new send packet size */
    debug(F101,"rcalcpsiz","",q);
#else
/* New way */
    debug(F101,"rcalcpsiz numerrs","",numerrs);
    debug(F101,"rcalcpsiz spsiz","",spsiz);
    if (spackets < 3) {
	numerrs = 0;
	return;
    }
    if (numerrs)
      spsiz = spsiz / 2;
    else
      spsiz = (spsiz / 4) * 5;
    if (spsiz < 20) spsiz = 20;
    if (spsiz > spmax) spsiz = spmax;
    debug(F101,"rcalcpsiz new spsiz","",spsiz);
    numerrs = 0;
#endif /* COMMENT */
}
#endif /* NEWDPL */

/*  R E S E N D  --  Retransmit packet n.  */

/*
  Returns 0 or positive on success (the number of retries for packet n).
  On failure, returns a negative number, and an error message is placed
  in recpkt.
*/
int
resend(n) int n; {			/* Send packet n again. */
    int j, k, x;
#ifdef GFTIMER
    CKFLOAT t1 = 0.0, t2 = 0.0;
#endif /* GFTIMER */

    debug(F101,"resend seq","",n);

    k = chkwin(n,winlo,wslots);		/* See if packet in current window */
    j = -1;				/* Assume it's lost */
    if (k == 0) j = sseqtbl[n];		/* See if we still have a copy of it */
    if (k != 0 || j < 0) {		/* If not.... */
	if (nakstate && k == 1) {
/*
  Packet n is in the previous window and we are the file receiver.
  We already sent the ACK and deallocated its buffer so we can't just
  retransmit the ACK.  Rather than give up, we try some tricks...
*/
	    if (n == 0 && spackets < 63 && myinit[0]) { /* ACK to Send-Init */
/*
  If the packet number is 0, and we're at the beginning of a protocol
  operation (spackets < 63), then we have to resend the ACK to an I or S
  packet, complete with parameters in the data field.  So we take a chance and
  send a copy of the parameters in an ACK packet with block check type 1.
  (Or 3 if SET BLOCK 5.)
*/
		if (bctf) {		/* Force Type 3 on all packets? */
		   x = spack('Y',0,(int)strlen((char *)myinit),(CHAR *)myinit);
		    if (x < 0) return(x);
		    logpkt('#',n,(CHAR *)"<reconstructed>",0); /* Log it */
		} else {		/* Regular Kermit protocol */
		    int bctlsav;	/* Temporary storage */
		    int bctusav;
		    bctlsav = bctl;	/* Save current block check length */
		    bctusav = bctu;	/* and type */
		    bctu = bctl = 1;	/* Set block check to 1 */
		   x = spack('Y',0,(int)strlen((char *)myinit),(CHAR *)myinit);
		    if (x < 0) return(x);
		    logpkt('#',n,(CHAR *)"<reconstructed>",0); /* Log it */
		    bctu = bctusav;	/* Restore block check type */
		    bctl = bctlsav;	/* and length */
		}
	    } else {			/* Not the first packet */
/*
  It's not the first packet of the protocol operation.  It's some other packet
  that we have already ACK'd and forgotten about.  So we take a chance and
  send an empty ACK using the current block-check type.  Usually this will
  work out OK (like when acking Data packets), and no great harm will be done
  if it was some other kind of packet (F, etc).  If we are requesting an
  interruption of the file transfer, the flags are still set, so we'll catch
  up on the next packet.
*/
		x = spack('Y',n,0,(CHAR *) "");
		if (x < 0) return(x);
	    }
	    retrans++;
	    xxscreen(SCR_PT,'%',(long)pktnum,"Retransmission");
	    return(0);
	} else {
/*
  Packet number is not in current or previous window.  We seem to hit this
  code occasionally at the beginning of a transaction, for apparently no good
  reason.  Let's just log it for debugging, send nothing, and try to proceed
  with the protocol rather than killing it.
*/
	    debug(F101,"resend PKT NOT IN WINDOW","",n);
	    debug(F101,"resend k","",k);
	    return(0);
	}
    }

/* OK, it's in the window and it's not lost. */

    debug(F101,"resend pktinfo index","",k);

    if (maxtry > 0 && s_pkt[j].pk_rtr++ > maxtry) { /* Over retry limit */
	xitsta |= what;
	return(-1);
    }
    debug(F101,"resend retry","",s_pkt[j].pk_rtr); /* OK so far */
    dumpsbuf();				/* (debugging) */
    if (s_pkt[j].pk_typ == ' ') {	/* Incompletely formed packet */
	if (nakstate) {			/* (This shouldn't happen any more) */
	    nack(n);
	    retrans++;
	    xxscreen(SCR_PT,'%',(long)pktnum,"(resend)");
	    return(s_pkt[j].pk_rtr);
	} else {			/* No packet to resend! */
#ifdef COMMENT
/*
  This happened (once) while sending a file with 2 window slots and typing
  X to the sender to cancel the file.  But since we're cancelling anyway,
  there's no need to give a scary message.
*/
	    sprintf((char *)epktmsg,
		    "resend logic error: NPS, n=%d, j=%d.",n,j);
	    return(-2);
#else
/* Just ignore it. */
	    return(0);
#endif /* COMMENT */
	}
    }
#ifdef DEBUG
#ifdef GFTIMER
    if (deblog) t1 = gftimer();
#endif /* GFTIMER */
#endif /* DEBUG */

    /* Everything ok, send the packet */
#ifdef CK_TIMERS
    if (timint > 0)
      srttbl[n] = gtimer();		/* Update the timer */
#endif /* CK_TIMERS */
    x = ttol(s_pkt[j].pk_adr,s_pkt[j].pk_len);

#ifdef DEBUG
#ifdef GFTIMER
    if (deblog)  {
	t2 = gftimer();
	debug(F101,"resend ttol msec","",(long)((t2-t1)*1000.0));
    }
#endif /* GFTIMER */
#endif /* DEBUG */
    debug(F101,"resend ttol returns","",x);

    retrans++;				/* Count a retransmission */
    xxscreen(SCR_PT,'%',(long)pktnum,"(resend)"); /* Tell user about resend */
    logpkt('S',n,s_pkt[j].pk_adr, s_pkt[j].pk_len); /* Log the resent packet */
    return(s_pkt[j].pk_rtr);		/* Return the number of retries. */
}

/*  E R R P K T  --  Send an Error Packet  */

int
errpkt(reason) CHAR *reason; {		/* ...containing the reason given */
    extern int rtimo, state, justone;
    int x, y;
    czseen = 1;				/* Also cancels batch */
    state = 0;				/* Reset protocol state */
    debug(F110,"errpkt",reason,0);
    tlog(F110,"Protocol Error:",(char *)reason,0L);
    xxscreen(SCR_EM,0,0L,reason);
    encstr(reason);
    x = spack('E',pktnum,size,data);
    ckstrncpy((char *)epktmsg,(char *)reason,PKTMSGLEN);
    y = quiet; quiet = 1; epktsent = 1;	/* Close files silently. */
    clsif(); clsof(1);
    quiet = y;
/*
  I just sent an E-packet.  I'm in local mode, I was receiving a file,
  I'm not a server, and sliding windows are in use.  Therefore, there are
  likely to be a bunch of packets already "in the pipe" on their way to me
  by the time the remote sender gets the E-packet.  So the next time I
  CONNECT or try to start another protocol operation, I am likely to become
  terribly confused by torrents of incoming material.  To prevent this,
  the following code soaks up packets from the connection until there is an
  error or timeout, without wasting too much time waiting.

  Exactly the same problem occurs when I am in remote mode or if I am
  in server mode with the justone flag set.  In remote mode not only
  does the packet data potentially get echo'd back to the sender which
  is confusing to the user in CONNECT mode, but it also may result in the
  host performing bizarre actions such as suspending the process if ^Z is
  unprefixed, etc.

  Furthermore, thousands of packets bytes in the data stream prevent the
  client from being able to process Telnet Kermit Option negotiations
  properly.
*/
#ifdef STREAMING
    /* Because streaming sets the timeout to 0... */
    if (streaming) {
	timint = rcvtimo = rtimo;
	streaming = 0;
    }
#endif /* STREAMING */
    if (what & W_RECV &&
        (!server || (server && justone)) &&
        (wslots > 1
#ifdef STREAMING
	 || streaming
#endif /* STREAMING */
	 )) {
#ifdef GFTIMER
	CKFLOAT oldsec, sec = (CKFLOAT) 0.0;
#else
	int oldsec, sec = 0;
#endif /* GFTIMER */
	debug(F101,"errpkt draining","",wslots);
	xxscreen(SCR_ST,ST_MSG,0l,"Draining incoming packets, wait...");
	while (x > -1) {		/* Don't bother if no connection */
	    oldsec = sec;
#ifdef GFTIMER
	    sec = gftimer();
	    if (oldsec != (CKFLOAT) 0.0)
	      timint = rcvtimo = (int) (sec - oldsec + 0.5);
#else
	    sec = gtimer();
	    if (oldsec != 0)
	      timint = rcvtimo = sec - oldsec + 1;
#endif /* GFTIMER */
	    if (timint < 1)
	      timint = rcvtimo = 1;
	    msleep(50);			/* Allow a bit of slop */
	    x = rpack();		/* Read a packet */
	    if (x == 'T' || x == 'z')	/* Timed out means we're done */
	      break;
	    xxscreen(SCR_PT,x,rsn,"");	/* Let user know */
	}
	xxscreen(SCR_ST,ST_MSG,0l,"Drain complete.");
    }
    if ((x = (what & W_KERMIT)))
      xitsta |= x;			/* Remember what failed. */
    success = 0;
    return(y);
}

/* scmd()  --  Send a packet of the given type */

int
#ifdef CK_ANSIC
scmd(char t, CHAR *dat)
#else
scmd(t,dat) char t; CHAR *dat;
#endif /* CK_ANSIC */
/* scmd */ {
    int x;
    extern char * srimsg;
    debug(F000,"scmd",dat,t);
    if (encstr(dat) < 0) {		/* Encode the command string */
	srimsg = "String too long";
	return(-1);
    }
    x = spack(t,pktnum,size,data);
    debug(F101,"scmd spack","",x);
    return(x);
}

/* Compose and Send GET packet */

struct opktparm {			/* O-Packet item list */
    CHAR * opktitem;
    struct opktparm * opktnext;
};

struct opktparm * opkthead = NULL;	/* Linked list of O-packet fields */
int opktcnt = 0;			/* O-Packet counter */
char * srimsg = NULL;			/* GET-Packet error message */

/* S O P K T  --  Send O-Packet */
/*
  Sends one O-Packet each time called, using first-fit method of filling
  the packet from linked list of parameters pointed to by opkthead.
  To be called repeatedly until list is empty or there is an error.
  Returns:
   -1 on failure.
    0 on success and no more fields left to send.
    1 on success but with more fields left to be sent.
*/

int
sopkt() {
    int n = 0;				/* Field number in this packet */
    int rc = 0;				/* Return code */
    int len = 0;			/* Data field length */
    char c = NUL;
    struct opktparm * o = NULL;
    struct opktparm * t = NULL;
    struct opktparm * prev = NULL;
    CHAR * dsave = data;
    int x, ssave = spsiz;

    srimsg = NULL;			/* Error message */
    o = opkthead;			/* Point to head of list */
    if (!o) {				/* Oops, no list... */
	srimsg = "GET Packet Internal Error 1";
	debug(F100,"sopkt NULL list","",0);
	return(-1);
    }
    while (o) {				/* Go thru linked list... */
	c = *(o->opktitem);		/* Parameter code */
	debug(F000,"sopkt",o->opktitem,c);
	x = encstr((CHAR *)o->opktitem);
	debug(F111,"sopkt encstr",dsave,x);
	if (x < 0) {			/* Encode this item */
	    if (n == 0) {		/* Failure, first field in packet */
		debug(F100,"sopkt overflow","",0);
		spsiz = ssave;		/* Restore these */
		data = dsave;
		o = opkthead;		/* Free linked list */
		while (o) {
		    if (o->opktitem) free(o->opktitem);
		    t = o->opktnext;
		    free((char *)o);
		    o = t;
		}
		opkthead = NULL;
		srimsg = "GET Packet Too Long for Server";
		return(-1);		/* Fail */
	    } else {			/* Not first field in packet */
		debug(F110,"sopkt leftover",o->opktitem,0);
		prev = o;		/* Make this one the new previous */
		o = o->opktnext;	/* Get next */
		c = NUL;		/* So we know we're not done */
		*data = NUL;		/* Erase any partial encoding */
		continue;		/* We can try this one again later */
	    }
	}
	n++;				/* Encoding was successful */
	debug(F111,"sopkt field",data,x);
	len += x;			/* Total data field length */
	data += x;			/* Set up for next field... */
	spsiz -= x;
	free(o->opktitem);		/* Free item just encoded */
	if (o == opkthead) {		/* If head */
	    opkthead = o->opktnext;	/* Move head to next */
	    free((char *)o);		/* Free this list node */
	    o = opkthead;
	} else {			/* If not head */
	    o = o->opktnext;		/* Get next */
	    prev->opktnext = o;		/* Link previous to next */
	}
	if (c == '@')			/* Loop exit */
	  break;
	if (!o && !opkthead) {		/* Set up End Of Parameters Field */
	    o = (struct opktparm *)malloc(sizeof(struct opktparm));
	    if (o) {
		opkthead = o;
		if (!(o->opktitem = (CHAR *)malloc(3))) {
		    free((char *)o);
		    srimsg = "GET Packet Internal Error 8";
		    return(-1);
		}
		ckstrncpy((char *)(o->opktitem), "@ ", 3);
		debug(F111,"sopkt o->opktitem",o->opktitem,
		      strlen((char *)(o->opktitem)));
		o->opktnext = NULL;
	    }
	}
    }
    data = dsave;			/* Restore globals */
    spsiz = ssave;
    debug(F110,"sopkt data",data,0);
    debug(F101,"sopkt opktcnt","",opktcnt);
    if (opktcnt++ > 0) {
	if (nxtpkt() < 0) {		/* Get next packet number and buffer */
	    srimsg = "GET Packet Internal Error 9";
	    return(-1);
	}
    }
    debug(F101,"sopkt pktnum","",pktnum);
    rc = spack((char)'O',pktnum,len,data); /* Send O-Packet */
    debug(F101,"sopkt spack","",rc);
    if (rc < 0)				/* Failed */
      srimsg = "Send Packet Failure";	/* Set message */
    else				/* Succeeded */
      rc = (c == '@') ? 0 : 1;		/* 1 = come back for more, 0 = done */
    debug(F101,"sopkt rc","",rc);
    return(rc);
}

/* S R I N I T  --  Send GET packet  */
/*
  Sends the appropriate GET-Class packet.
  Returns:
  -1 on error
   0 if packet sent successfully and we can move on to the next state
   1 if an O-packet was sent OK but more O packets still need to be sent.
*/
int
srinit(reget, retrieve, opkt) int reget, retrieve, opkt; {
    int x = 0, left = 0;
    extern int oopts, omode;
    CHAR * p = NULL;
#ifdef RECURSIVE
    extern int recursive;
    debug(F101,"srinit recursive","",recursive);
#endif /* RECURSIVE */
    debug(F101,"srinit reget","",reget);
    debug(F101,"srinit retrieve","",retrieve);
    debug(F101,"srinit opkt","",opkt);
    debug(F101,"srinit oopts","",oopts);
    debug(F101,"srinit omode","",omode);
    debug(F110,"srinit cmarg",cmarg,0);
    srimsg = NULL;

    opktcnt = 0;
    if (!cmarg) cmarg = "";
    if (!*cmarg) {
	srimsg = "GET with no filename";
	debug(F100,"srinit null cmarg","",0);
	return(-1);
    }
    if (opkt) {				/* Extended GET is totally different */
	char buf[16];
	struct opktparm * o = NULL;
	struct opktparm * prev = NULL;

        buf[0] = NUL;

	/* Build O-Packet fields and send (perhaps first) O-Packet */

	if (oopts > -1) {		/* Write Option flags */
	    o = (struct opktparm *)malloc(sizeof(struct opktparm));
	    if (!o) {
		srimsg = "GET Packet Internal Error 2";
		debug(F100,"srinit malloc fail O1","",0);
		return(-1);
	    }
	    sprintf(buf,"Ox%d",oopts);	/* safe */
	    x = (int) strlen(buf+2);
	    buf[1] = tochar(x);
	    o->opktitem = (CHAR *)malloc(x + 3);
	    if (!o->opktitem) {
		srimsg = "GET Packet Internal Error 3";
		debug(F100,"srinit malloc fail O2","",0);
		return(-1);
	    }
	    ckstrncpy((char *)(o->opktitem),buf,x+3);
	    o->opktnext = NULL;
	    if (!opkthead)
	      opkthead = o;
	    prev = o;
	}
	if (omode > -1) {		/* If Xfer Mode specified, write it */
	    o = (struct opktparm *)malloc(sizeof(struct opktparm));
	    if (!o) {
		srimsg = "GET Packet Internal Error 4";
		debug(F100,"srinit malloc fail M1","",0);
		return(-1);
	    }
	    sprintf(buf,"Mx%d",omode);	/* safe */
	    x = (int) strlen(buf+2);
	    buf[1] = tochar(x);
	    o->opktitem = (CHAR *)malloc(x + 3);
	    if (!o->opktitem) {
		srimsg = "GET Packet Internal Error 5";
		debug(F100,"srinit malloc fail O2","",0);
		return(-1);
	    }
	    ckstrncpy((char *)(o->opktitem),buf,x+3);
	    o->opktnext = NULL;
	    if (!opkthead)
	      opkthead = o;
	    else
	      prev->opktnext = o;
	    prev = o;
	}

	/* Same deal for oname and opath eventually but not needed now... */

	x = strlen(cmarg);		/* Now do filename */
	if (x > spsiz - 4) {
	    srimsg = "GET Packet Too Long for Server";
	    return(-1);
	}
	o = (struct opktparm *)malloc(sizeof(struct opktparm));
	if (!o) {
	    srimsg = "GET Packet Internal Error 6";
	    debug(F100,"srinit malloc fail F1","",0);
	    return(-1);
	}
	left = x + 6;
	o->opktitem = (CHAR *)malloc(left + 1);
	if (!o->opktitem) {
	    srimsg = "GET Packet Internal Error 7";
	    debug(F100,"srinit malloc fail F2","",0);
	    return(-1);
	}
	p = o->opktitem;
	*p++ = 'F';
	left--;
	if (x > 94) {			/* Too long for normal length */
	    *p++ = SYN;			/* Escape length with Ctrl-V */
	    *p++ = tochar(x / 95);
	    *p++ = tochar(x % 95);
	    left -= 3;
	} else {			/* Normal encoding for 94 or less */
	    *p++ = tochar(x);
	    left--;
	}
	ckstrncpy((char *)p,cmarg,left); /* Copy the filename */
	o->opktnext = NULL;
	if (!opkthead)
	  opkthead = o;
	else
	  prev->opktnext = o;
	prev = o;

	/* End of Parameters */

	prev->opktnext = NULL;		/* End of list. */
	return(sopkt());
    }

    /* Not Extended GET */

    if (encstr((CHAR *)cmarg) < 0) {	/* Encode the filename. */
	srimsg = "GET Packet Too Long for Server";
	return(-1);
    }
    if (retrieve) {			/* Send the packet. */
#ifdef RECURSIVE
	if (recursive)
	  x = spack((char)'W',pktnum,size,data); /* GET /DELETE /RECURSIVE */
	else
#endif /* RECURSIVE */
	  x = spack((char)'H',pktnum,size,data); /* GET /DELETE */
    }
#ifdef RECURSIVE
    else if (recursive)
      x = spack((char)'V',pktnum,size,data); /* GET /RECURSIVE */
#endif /* RECURSIVE */
    else
      x = spack((char)(reget ? 'J' : 'R'),pktnum,size,data); /* GET */
    if (x < 0)
      srimsg = "Send Packet Failure";
    return(x < 0 ? x : 0);
}


/*  K S T A R T  --  Checks for a Kermit packet while in terminal mode.  */

/*  (or command mode...)  */

#ifdef CK_AUTODL
int
#ifdef CK_ANSIC
kstart(CHAR ch)
#else
kstart(ch) CHAR ch;
#endif /* CK_ANSIC */
/* kstart */ {
    static CHAR * p = NULL;

#ifdef OS2
    static CHAR * pk = NULL;
#endif /* OS2 */
    ch &= 0177;				/* Strip 8th bit */

    /* Because we're in cooked mode at the command prompt... */

    if (ch == LF) {
	debug(F110,"kstart","ch == LF",0);
	if ((what == W_COMMAND || what == W_INIT || what == W_NOTHING)) {
	    if (eol == CR) {
		ch = eol;
		debug(F110,"kstart","ch = CR",0);
	    }
	}
    }

#ifdef OS2
    if (adl_kmode == ADL_STR) {
	if (!ch)
	  return(0);
	if (!pk)
	  pk = adl_kstr;

	if (ch == *pk) {
	    pk++;
	    if (*pk == '\0') {
		pk = adl_kstr;
		debug(F100, "kstart Kermit Start String","",0);
		return(PROTO_K + 1);
	    }
	} else
	  pk = adl_kstr;
    }
#endif /* OS2 */

    if (ch == stchr) {			/* Start of packet */
	kstartactive = 1;
	p = ksbuf;
	*p = ch;
	debug(F101,"kstart SOP","",ch);
    } else if (ch == eol) {		/* End of packet */
	kstartactive = 0;
	if (p) {
	    debug(F101,"kstart EOL","",ch);
	    p++;
	    if (p - ksbuf < 94 ) {
		int rc = 0;
		*p++ = ch;
		*p = NUL;
		rc = chkspkt((char *)ksbuf);
		debug(F111,"kstart EOP chkspkt", ksbuf, rc);
		p = NULL;
		if (!rc) return(0);
		if (rc == 2) rc = -1;
		debug(F111,"kstart ksbuf",ksbuf,rc);
		return(rc);
	    } else {
		debug(F110,"kstart","p - ksbuf >= 94",0);
		p = NULL;
	    }
	}
    } else if (p) {
	if (ch < SP)
	  kstartactive = 0;
	p++;
	if (p - ksbuf < 94) {
	    *p = ch;
	} else {
	    p = NULL;
	    debug(F110,"kstart","p - ksbuf >= 94",0);
	}
    }
    return(0);
}

#ifdef CK_XYZ

/*  Z S T A R T  --  Checks for a ZMODEM packet while in terminal mode.  */

int
#ifdef CK_ANSIC
zstart(CHAR ch)
#else
zstart(ch) CHAR ch;
#endif /* CK_ANSIC */
/* zstart */ {
    static CHAR * matchstr = (CHAR *) "\030B00";
    /* "rz\r**\030B00000000000000\r\033J\021"; */
    static CHAR * p = NULL;
    extern int inserver;

    if (inserver)
      return(0);

    if (!ch)
      return(0);
    if (!p) {
#ifdef OS2
	p = adl_zmode == ADL_PACK ? matchstr : adl_zstr;
#else
	p = matchstr;
#endif /* OS2 */
    }
    if (ch == *p) {
	p++;
	if (*p == '\0') {
#ifdef OS2
	    if (adl_zmode == ADL_PACK) {
		p = matchstr;
		debug(F100, "zstart Zmodem SOP","",0);
	    } else {
		p = adl_zstr;
		debug(F100, "zstart Zmodem Start String","",0);
	    }
#else
	    p = matchstr;
	    debug(F100, "zstart Zmodem SOP","",0);
#endif /* OS2 */
	    return(PROTO_Z + 1);
	}
    } else {
#ifdef OS2
	p = adl_zmode == ADL_PACK ? matchstr : adl_zstr;
#else
	p = matchstr;
#endif /* OS2 */
    }
    return(0);
}
#endif /* CK_XYZ */

#ifndef NOICP
#ifdef CK_APC
/*  A U T O D O W N  */

#ifdef CK_ANSIC
VOID
autodown(int ch)
#else
VOID
autodown(ch) int ch;
#endif /* CK_ANSIC */
/* autodown */ {

/* The Kermit and Zmodem Auto-download calls go here */

    extern int justone;			/* From protocol module */
    extern int debses, protocol, apcactive, autodl, inautodl;
#ifdef DCMDBUF
    extern char *apcbuf;
#else
    extern char apcbuf[];
#endif /* DCMDBUF */
#ifdef OS2
    extern int apclength, term_io;
#endif /* OS2 */
    int k = 0;

    if ((autodl || inautodl
#ifdef IKS_OPTION
	 || TELOPT_SB(TELOPT_KERMIT).kermit.me_start
#endif /* IKS_OPTION */
	 ) && !debses) {
#ifdef CK_XYZ
#ifdef XYZ_INTERNAL
	extern int p_avail;
#else
	int p_avail = 1;
#endif /* XYZ_INTERNAL */
	if (p_avail && zstart((CHAR) ch)) {
	    debug(F100, "Zmodem download","",0);
#ifdef OS2
#ifndef NOTERM
            apc_command(APC_LOCAL,"receive /protocol:zmodem");
#endif /* NOTERM */
#else /* OS2 */
            ckstrncpy(apcbuf,"receive /protocol:zmodem",APCBUFLEN);
	    apcactive = APC_LOCAL;
#endif /* OS2 */
	    return;
	}
#endif /* CK_XYZ */

	/* First try... */
	k = kstart((CHAR) ch);
	if (
#ifdef NOSERVER
	    k > 0
#else /* NOSERVER */
	    k
#endif /* NOSERVER */
	    ) {				/* We saw a valid S or I packet */
	    if (k < 0) {		/* Stuff RECEIVE into APC buffer */
		justone = 1;
		switch (protocol) {
#ifdef CK_XYZ
		  case PROTO_G:
		    ckstrncpy(apcbuf,
			      "set proto kermit, server, set protocol g",
			      APCBUFLEN
			      );
		    break;
		  case PROTO_X:
		    ckstrncpy(apcbuf,
			      "set proto kermit,server,set proto xmodem",
			      APCBUFLEN
			      );
		    break;
                  case PROTO_XC:
		    ckstrncpy(apcbuf,
			   "set proto kermit,server,set proto xmodem-crc",
			      APCBUFLEN
			      );
                      break;
		  case PROTO_Y:
		    ckstrncpy(apcbuf,
			      "set proto kermit,server, set protocol y",
			      APCBUFLEN
			      );
		    break;
		  case PROTO_Z:
		    ckstrncpy(apcbuf,
			      "set proto kermit,server,set proto zmodem",
			      APCBUFLEN
			      );
		    break;
#endif /* CK_XYZ */
		  case PROTO_K:
		    ckstrncpy(apcbuf,"server",APCBUFLEN);
		    break;
		}
	    } else {
		justone = 0;
                ckstrncpy(apcbuf,"receive /protocol:kermit",APCBUFLEN);
	    }
#ifdef OS2
#ifndef NOTERM
            apc_command(APC_LOCAL,apcbuf);
#endif /* NOTERM */
#else /* OS2 */
            ckstrncpy(apcbuf,"receive /protocol:zmodem",APCBUFLEN);
	    apcactive = APC_LOCAL;
#endif /* OS2 */
	    return;
	}
    }
}
#endif /* CK_APC */
#endif /* NOICP */

/*  C H K S P K T  --  Check if buf contains a valid S or I packet  */

int
chkspkt(packet) char *packet; {
    int i;
    int buflen;
    int len = -1;
    CHAR chk;
    char type = 0;
    char *s = NULL;
    char *buf = NULL;
    char tmpbuf[100];			/* Longest S/I packet is about 30 */

    if (!packet) return(0);
    buflen = ckstrncpy(tmpbuf,packet,100); /* Make a pokeable copy */
    if (buflen < 5) return(0);		/* Too short */
    if (buflen > 100) return(0); 	/* Too long to be an S or I packet */
    s = buf = tmpbuf;			/* Point to beginning of copy */

    if (*s++ != stchr) return(0);	/* SOH */
    len = xunchar(*s++);		/* Length */
    if (len < 0) return(0);
    if (*s++ != SP) return(0);		/* Sequence number */
    type = *s++;			/* Type */
    if (type != 'S' && type != 'I')
      return(0);
    if (buflen < len + 2) return(0);
    s += (len - 3);			/* Position of checksum */
    chk = (CHAR) (*s);			/* Checksum */
    *s = NUL;			   /* Temporarily null-terminate data field */
    if (xunchar(chk) != chk1((CHAR *)(buf+1),buflen-2)) { /* Check it */
	/*
	  In C-Kermit 9.0 and later, an S or I packet can have a 
	  Type 3 Block check ("help set block-check" for details).
	*/
	unsigned crc;			/* Failed... Try Type 3 block check */
	*s = chk;			/* Replace last byte */
	s -= 2;				/* Back up two bytes */
	crc = (xunchar(s[0]) << 12)	/* Convert 3 bytes to numeric CRC */
	    | (xunchar(s[1]) << 6)
	    | (xunchar(s[2]));
	chk = (CHAR)(*s);		/* Copy 1st byte of 3-byte CRC */
	*s = NUL;			/* Null-terminate data field */
	if (crc != chk3((CHAR *)(buf+1),strlen(buf+1)))
	  return(0);
    }
    return(type == 'S' ? 1 : 2);
}
#endif /* CK_AUTODL */

/* R P A C K  --  Read a Packet */

/*
  rpack reads a packet and returns the packet type, or else Q if the
  packet was invalid, or T if a timeout occurred.  Upon successful return,
  sets the values of global rsn (received sequence number),  rln (received
  data length), and rdatap (pointer to null-terminated data field), and
  returns the packet type.  NOTE: This is an inner-loop function so must be
  efficient.  Protect function calls by if-tests where possible, e.g.
  "if (pktlog) logpkt(...);".
*/
int
rpack() {
    register int i, j, x, lp;		/* Local variables */
#ifdef CKTUNING
    unsigned int chk;
#endif /* CKTUNING */
    int k, type, chklen;
    unsigned crc;
    CHAR pbc[5];			/* Packet block check */
    CHAR *sohp;				/* Pointer to SOH */
    CHAR e;				/* Packet end character */

#ifdef GFTIMER
    CKFLOAT t1 = 0.0, t2 = 0.0;
#endif /* GFTIMER */

    debug(F101,"rpack pktnum","",pktnum);

#ifndef OLDCHKINT
    if (chkint() < 0)			/* Check for console interrupts. */
      return('z');
#endif /* OLDCHKINT */

    k = getrbuf();			/* Get a new packet input buffer. */
    debug(F101,"rpack getrbuf","",k);
    if (k < 0) {			/* Return like this if none free. */
	return(-1);
    }
    recpkt = r_pkt[k].bf_adr;
    *recpkt = '\0';			/* Clear receive buffer. */
    sohp = recpkt;			/* Initialize pointers to it. */
    rdatap = recpkt;
    rsn = rln = -1;			/* In case of failure. */
    e = (turn) ? turnch : eol;		/* Use any handshake char for eol */

/* Try to get a "line". */

#ifdef CK_AUTODL
    debug(F110,"rpack ksbuf",ksbuf,0);
    if (ksbuf[0]) {			/* Kermit packet already */
	int x;				/* collected for us in CONNECT mode */
	CHAR *s1 = recpkt, *s2 = ksbuf;
	j = 0;
	while (*s2) {			/* Copy and get length */
	    *s1++ = *s2++;		/* No point optimizing this since */
	    j++;			/* it's never more than ~20 chars */
	}
	*s1 = NUL;
#ifdef PARSENSE
	x = parchk(recpkt, stchr, j);	/* Check parity */
	debug(F000,"autodownload parity","",parity);
	debug(F000,"autodownload parchk","",x);
	if (x > 0 && parity != x) {
	    autopar = 1;
	    parity = x;
	}
#endif /* PARSENSE */
	ksbuf[0] = NUL;			/* Don't do this next time! */

    } else {				/* Normally go read a packet */
#endif /* CK_AUTODL */

#ifdef DEBUG
	if (deblog) {
	    debug(F101,"rpack timint","",timint);
	    debug(F101,"rpack rcvtimo","",rcvtimo);
#ifdef STREAMING
	    debug(F101,"rpack streaming","",streaming);
#endif /* STREAMING */
#ifdef GFTIMER
	    /* Measure how long it takes to read a packet */
	    t1 = gftimer();
#endif /* GFTIMER */
	}
#endif /* DEBUG */

/* JUST IN CASE (otherwise this could clobber streaming) */

	if ((timint == 0
#ifdef STREAMING
	     || streaming
#endif /* STREAMING */
	     ) && (rcvtimo != 0)) {
	    debug(F101,"rpack timint 0 || streaming but rcvtimo","",rcvtimo);
	    rcvtimo = 0;
	}

#ifdef PARSENSE
#ifdef UNIX
/*
  So far the final turn argument is only for ck[uvdl]tio.c.  Should be added
  to the others too.  (turn == handshake character.)
*/
	j = ttinl(recpkt,r_pkt[k].bf_len - 1,rcvtimo,e,stchr,turn);
#else
#ifdef VMS
	j = ttinl(recpkt,r_pkt[k].bf_len - 1,rcvtimo,e,stchr,turn);
#else
#ifdef datageneral
	j = ttinl(recpkt,r_pkt[k].bf_len - 1,rcvtimo,e,stchr,turn);
#else
#ifdef STRATUS
	j = ttinl(recpkt,r_pkt[k].bf_len - 1,rcvtimo,e,stchr,turn);
#else
#ifdef OS2
	j = ttinl(recpkt,r_pkt[k].bf_len - 1,rcvtimo,e,stchr,turn);
#else
#ifdef OSK
	j = ttinl(recpkt,r_pkt[k].bf_len - 1,rcvtimo,e,stchr,turn);
#else
	j = ttinl(recpkt,r_pkt[k].bf_len - 1,rcvtimo,e,stchr);
#endif /* OSK */
#endif /* OS2 */
#endif /* STRATUS */
#endif /* datageneral */
#endif /* VMS */
#endif /* UNIX */
	if (parity != 0 && parity != 's' && ttprty != 0) {
	    if (parity != ttprty) autopar = 1;
	    parity = ttprty;
	}
#else /* !PARSENSE */
	j = ttinl(recpkt,r_pkt[k].bf_len - 1,rcvtimo,e);
#endif /* PARSENSE */

#ifdef DEBUG
	if (deblog)  {
	    debug(F101,"rpack ttinl len","",j);
#ifdef GFTIMER
	    t2 = gftimer();
	    debug(F101,"rpack ttinl msec","",(long)((t2-t1)*1000.0));
#endif /* GFTIMER */
	}
#endif /* DEBUG */

#ifdef STREAMING
    if (streaming && sndtyp == 'D' && j == 0)
        return('Y');
#endif /* STREAMING */

	if (j < 0) {
	    /* -1 == timeout, -2 == ^C, -3 == connection lost or fatal i/o */
	    debug(F101,"rpack: ttinl fails","",j); /* Otherwise, */
	    freerbuf(k);		/* Free this buffer */
	    if (j < -1) {		/* Bail out if ^C^C typed. */
		if (j == -2) {
		    interrupted = 1;
		    debug(F101,"rpack ^C server","",server);
		    debug(F101,"rpack ^C en_fin","",en_fin);
		} else if (j == -3) {
		    fatalio = 1;
		    debug(F101,"rpack fatalio","",en_fin);
		}
		return(j);
	    }
	    if (nakstate)		/* j == -1 is a read timeout */
	      xxscreen(SCR_PT,'T',(long)winlo,"");
	    else
	      xxscreen(SCR_PT,'T',(long)pktnum,"");
	    logpkt('r',-1,(CHAR *)"<timeout>",0);
	    if (flow == 1) ttoc(XON);	/* In case of Xoff blockage. */
	    return('T');
	}
#ifdef CK_AUTODL
    }
#endif /* CK_AUTODL */

    rpktl = j;
    tlci += j;				/* All OK, Count the characters. */
    flci += j;

/* Find start of packet */

#ifndef PARSENSE
    for (i = 0; (recpkt[i] != stchr) && (i < j); i++)
      sohp++;				/* Find mark */
    if (i++ >= j) {			/* Didn't find it. */
	logpkt('r',-1,"<timeout>",0);
	freerbuf(k);
	return('T');
    }
#else
    i = 1;				/* ttinl does this for us */
#endif /* PARSENSE */

    rpackets++;				/* Count received packet. */
    lp = i;				/* Remember LEN position. */
    if ((j = xunchar(recpkt[i++])) == 0) { /* Get packet length.  */
        if ((j = lp+5) > MAXRP) {	/* Long packet */
	    return('Q');		/* Too long */
	}

#ifdef CKTUNING
	/* Save some function-call and loop overhead... */
#ifdef COMMENT
	/* ttinl() already removed parity */
	if (parity)
#endif /* COMMENT */
	  chk = (unsigned) ((unsigned) recpkt[i-1] +
			    (unsigned) recpkt[i]   +
			    (unsigned) recpkt[i+1] +
			    (unsigned) recpkt[i+2] +
			    (unsigned) recpkt[i+3]
			    );
#ifdef COMMENT
	else
	  chk = (unsigned) ((unsigned) (recpkt[i-1] & 077) +
			    (unsigned) (recpkt[i]   & 077) +
			    (unsigned) (recpkt[i+1] & 077) +
			    (unsigned) (recpkt[i+2] & 077) +
			    (unsigned) (recpkt[i+3] & 077)
			    );
#endif /* COMMENT */
	if (xunchar(recpkt[j]) != ((((chk & 0300) >> 6) + chk) & 077))
#else
	x = recpkt[j];			/* Header checksum. */
	recpkt[j] = '\0';		/* Calculate & compare. */
	if (xunchar(x) != chk1(recpkt+lp,5))
#endif /* CKTUNING */
	  {
	      freerbuf(k);
	      logpkt('r',-1,(CHAR *)"<crunched:hdr>",0);
	      xxscreen(SCR_PT,'%',(long)pktnum,"Bad packet header");
	      return('Q');
	  }
#ifndef CKTUNING
	recpkt[j] = x;			/* Checksum ok, put it back. */
#endif /* CKTUNING */
	rln = xunchar(recpkt[j-2]) * 95 + xunchar(recpkt[j-1]) - bctl;
	j = 3;				/* Data offset. */
    } else if (j < 3) {
	debug(F101,"rpack packet length less than 3","",j);
	freerbuf(k);
	logpkt('r',-1,(CHAR *)"<crunched:len>",0);
	xxscreen(SCR_PT,'%',(long)pktnum,"Bad packet length");
	return('Q');
    } else {
	rln = j - bctl - 2;		/* Regular packet */
	j = 0;				/* No extended header */
    }
    rsn = xunchar(recpkt[i++]);		/* Sequence number */
    if (pktlog)				/* Save a function call! */
      logpkt('r',rsn,sohp,rln+bctl+j+4);
    if (rsn < 0 || rsn > 63) {
	debug(F101,"rpack bad sequence number","",rsn);
	freerbuf(k);
	if (pktlog)
	  logpkt('r',rsn,(CHAR *)"<crunched:seq>",0);
	xxscreen(SCR_PT,'%',(long)pktnum,"Bad sequence number");
	return('Q');
    }
/*
  If this packet has the same type as the packet just sent, assume it is
  an echo and ignore it.  Don't even bother with the block check calculation:
  even if the packet is corrupted, we don't want to NAK an echoed packet.
  Nor must we NAK an ACK or NAK.
*/
    type = recpkt[i++];			/* Get packet's TYPE field */
    if (type == sndtyp || (nakstate && (type == 'N' /* || type == 'Y' */ ))) {
	debug(F000,"rpack echo","",type); /* If it's an echo */
	freerbuf(k);			/* Free this buffer */
	logpkt('#',rsn,(CHAR *)"<echo:ignored>",0);
	return('e');			/* Return special (lowercase) code */
    }
/*
  Separate the data from the block check, accounting for the case where
  a packet was retransmitted after the block check switched.  The "Type 3
  Forced" business is new to C-Kermit 9.0.
*/
    if (bctf) { 			/* Type 3 forced on all packets */
	bctl = chklen = 3;
    } else if ((type == 'I' || type == 'S')) { /* Otherwise... */
	if (recpkt[11] == '5') {	/* Sender is forcing Type 3 */
	    bctf = 1;			/* So we will too */
	    bctl = chklen = 3;
	    debug(F100,"RECOGNIZE BLOCK CHECK TYPE 5","",0);
	} else {			/* Normal case */
	    /* I & S packets always have type 1 */
	    chklen = 1;
	    rln = rln + bctl - 1;
	}
    } else if (type == 'N') {		/* A NAK packet never has data */
	chklen = xunchar(recpkt[lp]) - 2;
	if (chklen < 1 || chklen > 3) {	/* JHD 13 Apr 2010 */
	    debug(F101,"rpack bad nak chklen","",chklen);
	    freerbuf(k);
	    logpkt('r',-1,(CHAR *)"<crunched:chklen>",0);
	    xxscreen(SCR_PT,'%',(long)pktnum,"(bad nak)");
	    return('Q');
	}
	rln = rln + bctl - chklen;
    } else chklen = bctl;
#ifdef DEBUG
    if (deblog) {			/* Save 2 function calls */
	debug(F101,"rpack bctl","",bctl);
	debug(F101,"rpack chklen","",chklen);
    }
#endif /* DEBUG */
    i += j;				/* Buffer index of DATA field */
    rdatap = recpkt+i;			/* Pointer to DATA field */
    if ((j = rln + i) > r_pkt[k].bf_len) { /* Make sure it fits */
	debug(F101,"packet too long","",j);
	freerbuf(k);
	logpkt('r',rsn,(CHAR *)"<overflow>",0);
	return('Q');
    }
    for (x = 0; x < chklen; x++)	/* Copy the block check */
      pbc[x] = recpkt[j+x];		/* 3 bytes at most. */
    pbc[x] = '\0';			/* Null-terminate block check string */
    recpkt[j] = '\0';			/* and the packet Data field. */

    if (chklen == 2 && bctu == 4) {	/* Adjust for Blank-Free-2 */
	chklen = 4;			/* (chklen is now a misnomer...) */
	debug(F100,"rpack block check B","",0);
    }
    switch (chklen) {			/* Check the block check */
      case 1:				/* Type 1, 6-bit checksum */
	if (xunchar(*pbc) != chk1(recpkt+lp,j-lp)) {
#ifdef DEBUG
	    if (deblog) {
		debug(F110,"checked chars",recpkt+lp,0);
		debug(F101,"block check (1)","",(int) xunchar(*pbc));
		debug(F101,"should be (1)","",chk1(recpkt+lp,j-lp));
	    }
#endif /* DEBUG */
	    freerbuf(k);
	    logpkt('r',-1,(CHAR *)"<crunched:chk1>",0);
	    xxscreen(SCR_PT,'%',(long)pktnum,"Checksum error");
	    return('Q');
	}
	break;
      case 2:				/* Type 2, 12-bit checksum */
	x = xunchar(*pbc) << 6 | xunchar(pbc[1]);
	if (x != chk2(recpkt+lp,j-lp)) { /* No match */
	    if (type == 'E') {		/* Allow E packets to have type 1 */
		recpkt[j++] = pbc[0];
		recpkt[j] = '\0';
		if (xunchar(pbc[1]) == chk1(recpkt+lp,j-lp))
		  break;
		else
		  recpkt[--j] = '\0';
	    }
#ifdef DEBUG
	    if (deblog) {
		debug(F110,"checked chars",recpkt+lp,0);
		debug(F101,"block check (2)","", x);
		debug(F101,"should be (2)","", (int) chk2(recpkt+lp,j-lp));
	    }
#endif /* DEBUG */
	    freerbuf(k);
	    logpkt('r',-1,(CHAR *)"<crunched:chk2>",0);
	    xxscreen(SCR_PT,'%',(long)pktnum,"Checksum error");
	    return('Q');
	}
	break;
      case 3:				/* Type 3, 16-bit CRC */
	crc = (xunchar(pbc[0]) << 12)
	    | (xunchar(pbc[1]) << 6)
	    | (xunchar(pbc[2]));
	if (crc != chk3(recpkt+lp,j-lp)) {
	    if (type == 'E') {		/* Allow E packets to have type 1 */
		recpkt[j++] = pbc[0];
		recpkt[j++] = pbc[1];
		recpkt[j] = '\0';
		if (xunchar(pbc[2]) == chk1(recpkt+lp,j-lp))
		  break;
		else { j -=2; recpkt[j] = '\0'; }
	    }
#ifdef DEBUG
	    if (deblog) {
		debug(F110,"checked chars",recpkt+lp,0);
		debug(F101,"block check (3)","",crc);
		debug(F101,"should be (3)","",(int) chk3(recpkt+lp,j-lp));
	    }
#endif /* DEBUG */
	    freerbuf(k);
	    logpkt('r',-1,(CHAR *)"<crunched:chk3>",0);
	    xxscreen(SCR_PT,'%',(long)pktnum,"CRC error");
	    return('Q');
	}
	break;
      case 4:				/* Type 4 = Type 2, no blanks. */
	x = (unsigned)((xunchar(*pbc) - 1) << 6) |
	  (unsigned)(xunchar(pbc[1]) - 1);
	if (x != chk2(recpkt+lp,j-lp)) {
	    if (type == 'E') {	/* Allow E packets to have type 1 */
		recpkt[j++] = pbc[0];
		recpkt[j] = '\0';
		if (xunchar(pbc[1]) == chk1(recpkt+lp,j-lp))
		  break;
		else
		  recpkt[--j] = '\0';
	    }
	    debug(F101,"bad type B block check","",x);
	    freerbuf(k);
	    logpkt('r',-1,(CHAR *)"<crunched:chkb>",0);
	    xxscreen(SCR_PT,'%',(long)pktnum,"Checksum error");
	    return('Q');
	}
	break;
      default:			/* Shouldn't happen... */
	freerbuf(k);
	logpkt('r',-1,(CHAR *)"<crunched:chkx>",0);
	xxscreen(SCR_PT,'%',(long)pktnum,"(crunched)");
	return('Q');
    }
    debug(F101,"rpack block check OK","",rsn);

/* Now we can believe the sequence number, and other fields. */
/* Here we violate strict principles of layering, etc, and look at the  */
/* packet sequence number.  If there's already a packet with the same   */
/* number in the window, we remove this one so that the window will not */
/* fill up. */

    if ((x = rseqtbl[rsn]) != -1) {	/* Already a packet with this number */
	retrans++;			/* Count it for statistics */
	debug(F101,"rpack got dup","",rsn);
	logpkt('r',rsn,(CHAR *)"<duplicate>",0);
	freerbuf(x);			/* Free old buffer, keep new packet. */
	r_pkt[k].pk_rtr++;		/* Count this as a retransmission. */
    }

/* New packet, not seen before, enter it into the receive window. */

#ifdef CK_TIMERS
    if (timint > 0)
      rrttbl[rsn] = gtimer();		/* Timestamp */
#endif /* CK_TIMERS */

    rseqtbl[rsn] = k;			/* Make back pointer */
    r_pkt[k].pk_seq = rsn;		/* Record in packet info structure */
    r_pkt[k].pk_typ = type;		/* Sequence, type,... */
    r_pkt[k].pk_adr = rdatap;		/* pointer to data buffer */
    if (local) {			/* Save a function call! */
	int x = 0;
	if (fdispla != XYFD_N) x = 1;
	if (fdispla == XYFD_B && (type == 'D' || sndtyp == 'D')) x = 0;
	if (x)				/* Update screen */
	  xxscreen(SCR_PT,(char)type,(long)rsn,(char *)sohp);
    }
    return(type);			/* Return packet type */
}

/*  L O G P K T  --  Log packet number n, pointed to by s.  */

/* c = 's' (send) or 'r' (receive) */

VOID
#ifdef CK_ANSIC
logpkt(char c,int n, CHAR *s, int len)
#else
logpkt(c,n,s,len) char c; int n; CHAR *s; int len;
#endif /* CK_ANSIC */
/* logpkt */ {
    char plog[20];
    if (!s) s = (CHAR *)"";
    if (pktlog) if (chkfn(ZPFILE) > 0) {
	if (n < 0)			/* Construct entry header */
	  sprintf(plog,"%c-xx-%02d-",c,(gtimer()%60)); /* safe */
	else
	  sprintf(plog,"%c-%02d-%02d-",c,n,(gtimer()%60)); /* safe */
	if (zsoutx(ZPFILE,plog,(int)strlen(plog)) < 0) {
	    pktlog = 0;
	    return;
	} else {
	    if (len == 0)
	      len = strlen((char *)s);
	    if (len > 0) {
		char * p;		/* Make SOP printable */
		int x;			/* so we can look at logs without */
		p = dbchr(*s);		/* triggering autodownload. */
		x = strlen(dbchr(*s));
		if (*s < 32 || (*s > 127 && *s < 160)) {
		    if (zsoutx(ZPFILE,p,x) < 0) {
			pktlog = 0;
			return;
		    } else {
			len--;
			s++;
		    }
		}
	    }
	    if (zsoutx(ZPFILE,(char *)s,len) < 0) {
		pktlog = 0;
		return;
	    } else if (zsoutx(ZPFILE,
#ifdef UNIX
			      "\n", 1
#else
#ifdef datageneral
			      "\n", 1
#else
#ifdef OSK
			      "\r", 1
#else
#ifdef MAC
			      "\r", 1
#else
			      "\015\012", 2
#endif /* MAC */
#endif /* OSK */
#endif /* datageneral */
#endif /* UNIX */
			      ) < 0) {
		pktlog = 0;
	    }
	}
    }
}

/*  T S T A T S  --  Record statistics in transaction log  */

VOID
tstats() {
    char *tp = NULL;
#ifdef GFTIMER
    CKFLOAT xx;				/* Elapsed time divisor */
#endif /* GFTIMER */

    debug(F101,"tstats xfsecs","",xfsecs);
    debug(F101,"tstats filcnt","",filcnt);
    if (filcnt == 1) {			/* Get timing for statistics */
	tsecs = xfsecs;			/* Single file, we already have it */
#ifdef GFTIMER
	debug(F101,"tstats fpxfsecs","",(int)fpxfsecs);
	fptsecs = fpxfsecs;
#endif /* GFTIMER */
    } else {				/* Multiple files */
	tsecs = gtimer();		/* Get current time */
#ifdef GFTIMER
	fptsecs = gftimer();
#endif /* GFTIMER */
    }
#ifdef GFTIMER
    if (fptsecs <= GFMINTIME)		/* Calculate CPS */
      fptsecs = (CKFLOAT) GFMINTIME;
    debug(F101,"tstats fptsecs","",(int)fptsecs);
    xx = (CKFLOAT) tfc / fptsecs;
    if (sizeof(long) <= 4) {		/* doesn't account for 16-bit longs */
	if (xx  > 2147483647.0)
	  tfcps = 2147483647L;	        /* 31 bits */
	else
	  tfcps = (long) xx;
    } else
      tfcps = (long) xx;
#else
    if (tsecs < 2L)
      tsecs = 1L;
    debug(F101,"tstats tsecs","",tsecs);
    tfcps = tfc / tsecs;
#endif /* GFTIMER */

    ztime(&tp);				/* Get time stamp */
    tlog(F100,"","",0L);		/* Leave a blank line */
    tlog(F110,"Transaction complete",tp,0L);  /* Record it */

    if (filcnt < 1) return;		/* If no files, done. */

/* If multiple files, record character totals for all files */

    if (filcnt > 1) {
	tlog(F101," files transferred       ","",filcnt - filrej);
	tlog(F101," total file characters   ","",tfc);
	tlog(F101," communication line in   ","",tlci);
	tlog(F101," communication line out  ","",tlco);
    }

/* Record timing info for one or more files */

#ifdef GFTIMER
    if (filcnt - filrej == 1) {
	tlog(F101," elapsed time (seconds)  ","",(long) fpxfsecs);
	tlog(F101," effective data rate     ","",filcps);
    } else {
	tlog(F101," elapsed time (seconds)  ","",(long) fptsecs);
	tlog(F101," effective data rate     ","",(long) xx);
    }
#else
    tlog(F101," elapsed time (seconds)  ","",tsecs);
    if (tsecs > 0)
      tlog(F101," effective data rate     ","",(tfc / tsecs));
#endif /* GFTIMER */

    tlog(F100,"","",0L);		/* Leave a blank line */
}

/*  F S T A T S  --  Record file statistics in transaction log  */

VOID
fcps() {
#ifdef GFTIMER
    double xx;
    fpxfsecs = gftimer() - fpfsecs;
    if (fpxfsecs <= GFMINTIME)
      fpxfsecs = (CKFLOAT) GFMINTIME;
    xx = (CKFLOAT) ffc / fpxfsecs;
    if (sizeof(long) <= 4) {
	if (xx  > 2147483647.0)
	  tfcps = 2147483647L;		/* 31 bits */
	else
	  filcps = (long) xx;
    } else
      filcps = (long) xx;
    if (sizeof(int) >= 4)
      xfsecs = (int) fpxfsecs;
    else if (fpxfsecs < 32768.0)
      xfsecs = (int) fpxfsecs;
    else
      xfsecs = 32767;
#else /* GFTIMER */
    xfsecs = gtimer() - fsecs;
    if (xfsecs < 1L) xfsecs = 1L;
    filcps = ffc / xfsecs;
#endif /* GFTIMER */
}

VOID
fstats() {
    tfc += ffc;
#ifdef DEBUG
    if (deblog) {
	debug(F101,"fstats tfc","",tfc);
	debug(F101,"fstats what","",what);
	debug(F110,"fstats epktmsg",epktmsg,0);
    }
#endif /* DEBUG */
#ifdef TLOG
    if (!discard && !cxseen && !czseen && what != W_NOTHING && !*epktmsg)
      tlog(F101," complete, size","",ffc);
#endif /* TLOG */
}

#endif /* NOXFER */

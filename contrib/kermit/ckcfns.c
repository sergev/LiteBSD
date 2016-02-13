char *fnsv = "C-Kermit functions, 9.0.233, 3 Jun 2011";

char *nm[] =  { "Disabled", "Local only", "Remote only", "Enabled" };

/*  C K C F N S  --  System-independent Kermit protocol support functions.  */

/*  ...Part 1 (others moved to ckcfn2,3 to make this module smaller) */

/*
  Author: Frank da Cruz <fdc@columbia.edu>,
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 2011,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/
/*
 System-dependent primitives defined in:

   ck?tio.c -- terminal (communications) i/o
   cx?fio.c -- file i/o, directory structure
*/
#include "ckcsym.h"			/* Needed for Stratus VOS */
#include "ckcasc.h"			/* ASCII symbols */
#include "ckcdeb.h"			/* Debug formats, typedefs, etc. */
#include "ckcker.h"			/* Symbol definitions for Kermit */
#include "ckcxla.h"			/* Character set symbols */
#include "ckcnet.h"                     /* VMS definition of TCPSOCKET */
#ifdef OS2
#ifdef OS2ONLY
#include <os2.h>
#endif /* OS2ONLY */
#include "ckocon.h"
#endif /* OS2 */

int docrc  = 0;				/* Accumulate CRC for \v(crc16) */
long crc16 = 0L;			/* File CRC = \v(crc16) */
int gnferror = 0;			/* gnfile() failure reason */

extern CHAR feol;
extern int byteorder, xflg, what, fmask, cxseen, czseen, nscanfile, sysindex;
extern int xcmdsrc, dispos, matchfifo;
extern int inserver;

extern int nolinks;
#ifdef VMSORUNIX
extern int zgfs_dir;
#ifdef CKSYMLINK
extern int zgfs_link;
#endif /* CKSYMLINK */
#endif /* VMSORUNIX */

#ifndef NOXFER

#ifndef NOICP
#ifndef NOSPL
extern char * clcmds;
extern int haveurl;
#ifdef CK_APC
extern int apcactive, adl_ask;
#endif /* CK_APC */
#endif /* NOSPL */
#endif /* NOICP */

extern int remfile;

/* (move these prototypes to the appropriate .h files...) */

#ifdef COMMENT
/* Not used */
#ifdef VMS
_PROTOTYP( int getvnum, (char *) );
#endif /* VMS */
#endif /* COMMENT */

_PROTOTYP( static int bgetpkt, (int) );
#ifndef NOCSETS
_PROTOTYP( int lookup, (struct keytab[], char *, int, int *) );
#endif /* NOCSETS */
#ifndef NOSPL
_PROTOTYP( int zzstring, (char *, char **, int *) );
#endif /* NOSPL */

#ifdef OS2
#include <io.h>
#ifdef OS2ONLY
#include <os2.h>
#endif /* OS2ONLY */
#endif /* OS2 */

#ifdef VMS
#include <errno.h>
#endif /* VMS */

/* Externals from ckcmai.c */

extern int srvcdmsg, srvidl, idletmo;
extern char * cdmsgfile[];
extern int spsiz, spmax, rpsiz, timint, srvtim, rtimo, npad, ebq, ebqflg,
 rpt, rptq, rptflg, capas, keep, fncact, pkttim, autopar, spsizr, xitsta;
extern int pktnum, bctr, bctu, bctf, bctl, clfils, sbufnum, protocol,
 size, osize, spktl, nfils, ckwarn, timef, spsizf, sndtyp, rcvtyp, success;
extern int parity, turn, network, whatru, fsecs, justone, slostart,
 ckdelay, displa, mypadn, moving, recursive, nettype;
extern long filcnt;
extern CK_OFF_T
 tfc, fsize, sendstart, rs_len, flci, flco, tlci, tlco, calibrate;
extern long filrej, oldcps, cps, peakcps, ccu, ccp, filestatus;
extern int fblksiz, frecl, frecfm, forg, fcctrl, fdispla, skipbup;
extern int spackets, rpackets, timeouts, retrans, crunched, wmax, wcur;
extern int hcflg, binary, fncnv, b_save, f_save, server;
extern int nakstate, discard, rejection, local, xfermode, interrupted;
extern int rq, rqf, sq, wslots, wslotn, wslotr, winlo, urpsiz, rln;
extern int fnspath, fnrpath, eofmethod, diractive, whatru2, wearealike;
extern int atcapr, atcapb, atcapu;
extern int lpcapr, lpcapb, lpcapu;
extern int swcapr, swcapb, swcapu;
extern int lscapr, lscapb, lscapu;
extern int rscapr, rscapb, rscapu;
extern int rptena, rptmin;
extern int sseqtbl[];
extern int numerrs, nzxopts;
extern long rptn;
extern int maxtry;
extern int stdouf;
extern int sendmode;
extern int carrier, ttprty;
extern int g_fnrpath;
#ifdef TCPSOCKET
extern int ttnproto;
#endif /* TCPSOCKET */

#ifndef NOSPL
extern int sndxin, sndxhi, sndxlo;
#endif /* NOSPL */

extern int g_binary, g_fncnv;

#ifdef GFTIMER
extern CKFLOAT fpfsecs;
#endif /* GFTIMER */

#ifdef OS2
extern struct zattr iattr;
#endif /* OS2 */

#ifdef PIPESEND
extern int usepipes;
#endif /* PIPESEND */
extern int pipesend;

#ifdef STREAMING
extern int streamrq, streaming, streamed, streamok;
#endif /* STREAMING */
extern int reliable, clearrq, cleared, urclear;

extern int
  atenci, atenco, atdati, atdato, atleni, atleno, atblki, atblko,
  attypi, attypo, atsidi, atsido, atsysi, atsyso, atdisi, atdiso;

extern int bigsbsiz, bigrbsiz;
extern char *versio;
extern char *filefile;
extern char whoareu[], * cksysid;

#ifndef NOSERVER
extern int ngetpath;
extern char * getpath[];
extern int fromgetpath;
#endif /* NOSERVER */

#ifdef CK_LOGIN
extern int isguest;
#endif /* CK_LOGIN */

extern int srvcmdlen;
extern CHAR *srvcmd, * epktmsg;
extern CHAR padch, mypadc, eol, seol, ctlq, myctlq, sstate, myrptq;
extern CHAR *data, padbuf[], stchr, mystch;
extern CHAR *srvptr;
extern CHAR *rdatap;
extern char *cmarg, *cmarg2, **cmlist, filnam[], ofilnam[];
extern char *rfspec, *prfspec, *rrfspec, *prrfspec, *sfspec, *psfspec, *rfspec;
extern char fspec[];
extern int fspeclen;

#ifndef NOMSEND
extern struct filelist * filehead, * filenext;
extern int addlist;
#endif /* NOMSEND */

_PROTOTYP( int lslook, (unsigned int b) ); /* Locking Shift Lookahead */
_PROTOTYP( int szeof, (CHAR *s) );
_PROTOTYP( VOID fnlist, (void) );
#endif /* NOXFER */

extern CK_OFF_T ffc;

/* Character set Translation */

#ifndef NOCSETS
extern int tcharset, fcharset, dcset7, dcset8;
extern int fcs_save, tcs_save;
extern int ntcsets, xlatype, cseqtab[];
extern struct csinfo tcsinfo[], fcsinfo[];
extern int r_cset, s_cset, afcset[];
#ifdef UNICODE
extern int ucsorder, fileorder;
#endif /* UNICODE */

_PROTOTYP( CHAR ident, (CHAR) );	/* Identity translation function */

/* Arrays of and pointers to character translation functions */

#ifdef CK_ANSIC
extern CHAR (*rx)(CHAR); /* Pointer to input character translation function */
extern CHAR (*sx)(CHAR); /* Pointer to output character translation function */
extern CHAR (*xls[MAXTCSETS+1][MAXFCSETS+1])(CHAR); /* Byte-to-Byte Send */
extern CHAR (*xlr[MAXTCSETS+1][MAXFCSETS+1])(CHAR); /* Byte-to-Byte Recv */
#ifdef UNICODE
extern int (*xut)(USHORT);	/* Translation function UCS to TCS */
extern int (*xuf)(USHORT);	/* Translation function UCS to FCS */
extern USHORT (*xtu)(CHAR);	/* Translation function TCS to UCS */
extern USHORT (*xfu)(CHAR);	/* Translation function FCS to UCS */
#endif /* UNICODE */

#else /* The same declarations again for non-ANSI comilers... */

extern CHAR (*rx)();
extern CHAR (*sx)();
extern CHAR (*xls[MAXTCSETS+1][MAXFCSETS+1])();
extern CHAR (*xlr[MAXTCSETS+1][MAXFCSETS+1])();
#ifdef UNICODE
extern int (*xut)();
extern int (*xuf)();
extern USHORT (*xtu)();
extern USHORT (*xfu)();
#endif /* UNICODE */
#endif /* CK_ANSIC */
#endif /* NOCSETS */

/* (PWP) external def. of things used in buffered file input and output */

#ifdef DYNAMIC
extern char *zinbuffer, *zoutbuffer;
#else
extern char zinbuffer[], zoutbuffer[];
#endif /* DYNAMIC */
extern char *zinptr, *zoutptr;
extern int zincnt, zoutcnt, zobufsize, xfrxla;

extern long crcta[], crctb[];		/* CRC-16 generation tables */
extern int rseqtbl[];			/* Rec'd-packet sequence # table */

#ifndef NOXFER

/* Criteria used by gnfile()... */

char sndafter[19]   = { NUL, NUL };
char sndbefore[19]  = { NUL, NUL };
char sndnafter[19]  = { NUL, NUL };
char sndnbefore[19] = { NUL, NUL };
char *sndexcept[NSNDEXCEPT]  = { NULL, NULL };
char *rcvexcept[NSNDEXCEPT]  = { NULL, NULL };
CK_OFF_T sndsmaller = (CK_OFF_T)-1;
CK_OFF_T sndlarger  = (CK_OFF_T)-1;

/* Variables defined in this module but shared by other modules. */

int xfrbel = 1;
char * ofperms = "";			/* Output file permissions */
int autopath = 0;			/* SET RECEIVE PATHNAMES AUTO flag */

#ifdef CALIBRATE
#define CAL_O 3
#define CAL_M 253

int cal_j = 0;

CHAR
cal_a[] = {
 16, 45, 98,  3, 52, 41, 14,  7, 76,165,122, 11,104, 77,166, 15,
160, 93, 18, 19,112, 85, 54, 23,232,213, 90, 27, 12, 81,126, 31,
  4,205, 34, 35,144, 73,110, 39, 28,133,218, 43,156, 65,102, 47,
 84, 61, 50, 51,208,117, 86, 55,  8,245, 74, 59, 44,125,222, 63,
 80,  1,162, 67,116,105,206, 71,120,  9,250, 75, 88, 97,  6, 79,
100,221, 82, 83, 36, 89, 94, 87, 40, 21,106, 91,236,145,150, 95,
228, 33,130, 99,148,137,198,103,108,169, 42,107,184,129, 78,111,
  0, 49,114,115, 32,121,254,119,172, 57,138,123,152,177, 22,127,
240,193,  2,131,176,  5, 38,135,204,229, 10,139,200,161,174,143,
128, 17,146,147, 68,153, 30,151, 72,217,170,155, 24,209, 62,159,
 64,225,194,163,244,201, 70,167,216,197,234,171,188,109,230,175,
212,113,178,179,132,185,190,183,136,249,202,187, 92,241,118,191,
 48,237, 66,195, 96,233,142,199,248, 37, 58,203, 60, 13,134,207,
 20, 29,210,211,164,149,182,215,220, 25, 26,219,124,157,246,223,
180,141,226,227,192,101,238,231, 56, 69,154,235,252,173, 46,239,
224,253,242,243,196, 53,214,247,168,181,186,251,140,189,158,255
};
#endif /* CALIBRATE */

char * rf_err = "Error receiving file";	/* rcvfil() error message */

#ifdef CK_SPEED
short ctlp[256] = {		/* Control-Prefix table */
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* C0  */
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* G0  */
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1, /* DEL */
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* C1  */
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* G1  */
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1  /* 255 */
};
#endif /* CK_SPEED */

int sndsrc;		/* Flag for where to get names of files to send: */
					/* -1: znext() function */
					/*  0: stdin */
					/* >0: list in cmlist or other list */
					/* -9: calibrate */

int  memstr;				/* Flag for input from memory string */
int  funcstr;				/* Flag for input from function */
int  bestlen = 0;
int  maxsend = 0;

int gnf_binary = 0;			/* Prevailing xfer mode for gnfile */

#ifdef pdp11
#define MYINITLEN 32
#else
#define MYINITLEN 100
#endif /* pdp11 */
CHAR myinit[MYINITLEN];			/* Copy of my Send-Init data */

/* Variables local to this module */

#ifdef TLOG
#ifndef XYZ_INTERNAL
static
#endif /* XYZ_INTERNAL */
char *fncnam[] = {
  "rename", "replace", "backup", "append", "discard", "ask",
  "update", "dates-differ", ""
};
#endif /* TLOG */

static char *memptr;			/* Pointer for memory strings */

#ifdef VMS
extern int batch;
#else
extern int backgrd;
#endif /* VMS */

#ifdef CK_CTRLZ
static int lastchar = 0;
#endif /* CK_CTRLZ */

#ifdef CK_ANSIC
static int (*funcptr)(void);		/* Pointer for function strings */
#else
static int (*funcptr)();
#endif /* CK_ANSIC */

#ifdef pdp11
#define CMDSTRL 50
static char cmdstr[50];			/* System command string. */
#else
#ifdef BIGBUFOK
#define CMDSTRL 1024
#else
#define CMDSTRL 256
#endif /* BIGBUFOK */
static char cmdstr[CMDSTRL+1];
#endif /* pdp11 */

static int drain;			/* For draining stacked-up ACKs. */

static int first;			/* Flag for first char from input */
static CHAR t;				/* Current character */
#ifdef COMMENT
static CHAR next;			/* Next character */
#endif /* COMMENT */

static int ebqsent = 0;			/* 8th-bit prefix bid that I sent */
static int lsstate = 0;			/* Locking shift state */
static int lsquote = 0;			/* Locking shift quote */

extern int quiet;

/*  E N C S T R  --  Encode a string from memory. */

/*
  Call this instead of getpkt() if source is a string, rather than a file.
  Note: Character set translation is never done in this case.
*/

#ifdef COMMENT
#define ENCBUFL 200
#ifndef pdp11
CHAR encbuf[ENCBUFL];
#else
/* This is gross, but the pdp11 root segment is out of space */
/* Will allocate it in ckuusr.c. */
extern CHAR encbuf[];
#endif /* pdp11 */
#endif /* COMMENT */

/*
  Encode packet data from a string in memory rather than from a file.
  Returns the length of the encoded string on success, -1 if the string
  could not be completely encoded into the currently negotiated data
  field length.
*/
int
#ifdef CK_ANSIC
encstr(CHAR *s)
#else /* CK_ANSIC */
encstr(s) CHAR* s;
#endif /* CK_ANSIC */
{
/*
  Recoded 30 Jul 94 to use the regular data buffer and the negotiated
  maximum packet size.  Previously we were limited to the length of encbuf[].
  Also, to return a failure code if the entire encoded string would not fit.
  Modified 14 Jul 98 to return length of encoded string.
*/
    int m, rc, slen; char *p;
    if (!data) {			/* Watch out for null pointers. */
	debug(F100,"SERIOUS ERROR: encstr data == NULL","",0);
	return(-1);
    }
    if (!s) s = (CHAR *)"";		/* Ditto. */
    slen = strlen((char *)s);		/* Length of source string. */
    debug(F111,"encstr",s,slen);
    rc = 0;				/* Return code. */
    m = memstr; p = memptr;		/* Save these. */
    memptr = (char *)s;			/* Point to the string. */
    /* debug(F101,"encstr memptr 1","",memptr); */
    memstr = 1;				/* Flag memory string as source. */
    first = 1;				/* Initialize character lookahead. */
    *data = NUL;			/* In case s is empty */
    debug(F101,"encstr spsiz","",spsiz);
    rc = getpkt(spsiz,0);		/* Fill a packet from the string. */
    debug(F101,"encstr getpkt rc","",rc);
    if (rc > -1 && memptr < (char *)(s + slen)) { /* Means we didn't encode */
	rc = -1;			/* the whole string. */
	debug(F101,"encstr string too big","",size);
    }
    debug(F101,"encstr getpkt rc","",rc);
    memstr = m;				/* Restore memory string flag */
    memptr = p;				/* and pointer */
    first = 1;				/* Put this back as we found it. */
    return(rc);
}

/*  Output functions passed to 'decode':  */

int			       /*  Put character in server command buffer  */
#ifdef CK_ANSIC
putsrv(char c)
#else
putsrv(c) register char c;
#endif /* CK_ANSIC */
/* putsrv */ {
    *srvptr++ = c;
    *srvptr = '\0';		/* Make sure buffer is null-terminated */
    return(0);
}

int					/*  Output character to console.  */
#ifdef CK_ANSIC
puttrm(char c)
#else
puttrm(c) register char c;
#endif /* CK_ANSIC */
/* puttrm */ {
    extern int rcdactive;
#ifndef NOSPL
    extern char * qbufp;		/* If REMOTE QUERY active, */
    extern int query, qbufn;		/* also store response in */
    if (query && qbufn++ < 1024) {	/* query buffer. */
	*qbufp++ = c;
	*qbufp = NUL;
    }
    if (!query || !xcmdsrc)
#endif /* NOSPL */
/*
  This routine is used (among other things) for printing the server's answer
  to a REMOTE command.  But REMOTE CD is special because it isn't really 
  asking for an answer from the server.  Thus some people want to suppress
  the confirmation message (e.g. when the server sends back the actual path
  of the directory CD'd to), and expect to be able to do this with SET QUIET
  ON.  But they would not want SET QUIET ON to suppress the other server
  replies, which are pointless without their answers.  Thus the "rcdactive"
  flag (REMOTE CD command is active).  Thu Oct 10 16:38:21 2002
*/
      if (!(quiet && rcdactive))	/* gross, yuk */
	conoc(c);
    return(0);
}
#endif /* NOXFER */

int					/*  Output char to file. */
#ifdef CK_ANSIC
putmfil(char c)				/* Just like putfil but to ZMFILE */
#else					/* rather than ZOFILE... */
putmfil(c) register char c;
#endif /* CK_ANSIC */
/* putmfil */ {
    debug(F000,"putfil","",c);
    if (zchout(ZMFILE, (char) (c & fmask)) < 0) {
	czseen = 1;
	debug(F101,"putfil zchout write error, setting czseen","",1);
	return(-1);
    }
    return(0);
}

int					/*  Output char to nowhere. */
#ifdef CK_ANSIC
putnowhere(char c)
#else
putnowhere(c) register char c;
#endif /* CK_ANSIC */
/* putnowhere */ {
    return(0);
}


int					/*  Output char to file. */
#ifdef CK_ANSIC
putfil(char c)
#else
putfil(c) register char c;
#endif /* CK_ANSIC */
/* putfil */ {
    debug(F000,"putfil","",c);
    if (zchout(ZOFILE, (char) (c & fmask)) < 0) {
	czseen = 1;   			/* If write error... */
	debug(F101,"putfil zchout write error, setting czseen","",1);
	return(-1);
    }
    return(0);
}

/*
  The following function is a wrapper for putfil().  The only reason for its
  existence is to be passed as a function pointer to decode(), which treats
  putfil() itself specially -- bypassing it and using an internal macro
  instead to speed things up.  Use zputfil() instead of putfil() in cases where
  we do not want this to happen, e.g. when we need to send output to the file
  with a mixture of zchout() and zsout()/zsoutl() calls (as is the case with
  incoming short-form REMOTE command replies redirected to a file), which would
  otherwise result in data written to the file out of order.
*/
int
#ifdef CK_ANSIC
zputfil(char c)
#else
zputfil(c) register char c;
#endif /* CK_ANSIC */
/* zputfil */ {
    return(putfil(c));
}

#ifndef NOXFER

/* D E C O D E  --  Kermit packet decoding procedure */

/*
 Call with string to be decoded and an output function.
 Returns 0 on success, -1 on failure (e.g. disk full).

 This is the "inner loop" when receiving files, and must be coded as
 efficiently as possible.  Note some potential problems:  if a packet
 is badly formed, having a prefixed sequence ending prematurely, this
 function, as coded, could read past the end of the packet.  This has
 never happened, thus the additional (time-consuming) tests have not
 been added.
*/

static CHAR *xdbuf;	/* Global version of decode()'s buffer pointer */
                        /* for use by translation functions. */

/* Function for pushing a character onto decode()'s input stream. */

VOID
#ifdef CK_ANSIC
zdstuff(CHAR c)
#else
zdstuff(c) CHAR c;
#endif /* CK_ANSIC */
/* zdstuff */ {
    xdbuf--;				/* Back up the pointer. */
    *xdbuf = c;				/* Stuff the character. */
}

#ifdef CKTUNING
/*
  Trimmed-down packet decoder for binary-mode no-parity transfers.
  decode() is the full version.
*/
int
#ifdef CK_ANSIC
bdecode(CHAR *buf, int (*fn)(char))
#else
bdecode(buf,fn) register CHAR *buf; register int (*fn)();
#endif /* CK_ANSIC */
/* bdecode */ {
    register unsigned int a, a7;	/* Various copies of current char */
    int ccpflg;				/* For Ctrl-unprefixing stats */
    int t;				/* Int version of character */
    int len;
    long z;				/* For CRC calculation */
    CHAR c;				/* Current character */

    if (!binary || parity || fn != putfil) /* JUST IN CASE */
      return(decode(buf,fn,1));
    debug(F100,"BDECODE","",0);

    xdbuf = buf;			/* Global copy of source pointer. */

    len = rln;				/* Number of bytes in data field */
    while (len > 0) {
        a = *xdbuf++ & 0xff;		/* Get next character */
	len--;
	rpt = 0;			/* Initialize repeat count. */
	if (a == rptq && rptflg) {	/* Got a repeat prefix? */
	    rpt = xunchar(*xdbuf++ & 0xFF); /* Yes, get the repeat count, */
	    rptn += rpt;
	    a = *xdbuf++ & 0xFF;	/* and get the prefixed character. */
	    len -= 2;
	}
	ccpflg = 0;			/* Control prefix flag. */
	if (a == ctlq) {		/* If control prefix, */
	    a  = *xdbuf++ & 0xFF;	/* get its operand */
	    len--;
	    a7 = a & 0x7F;		/* and its low 7 bits. */
	    if ((a7 >= 0100 && a7 <= 0137) || a7 == '?') { /* Controllify */
		a = ctl(a);		/* if in control range. */
		a7 = a & 0x7F;
		ccpflg = 1;		/* Note that we did this */
		ccp++;			/* Count for stats */
	    }
	} else a7 = a & 0x7f;		/* Not control quote */
	if (a7 < 32 || a7 == 127)	/* A bare control character? */
	  if (!ccpflg) ccu++;		/* Count it */
	if (!rpt) rpt = 1;
	for (; rpt > 0; rpt--) {	/* Output the char RPT times */
#ifdef CALIBRATE
	    if (calibrate) {
		ffc++;
		continue;
	    }
#endif /* CALIBRATE */
#ifdef OS2
            if (xflg && !remfile) {		/* Write to virtual screen */
                char _a;
                _a = a & fmask;
                t = conoc(_a);
                if (t < 1)
                    t = -1;
            } else
#endif /* OS2 */
	      t = zmchout(a & fmask);	/* zmchout is a macro */
	    if (t < 0) {
		debug(F101,"bdecode write error - errno","",errno);
		return(-1);
	    }
	    ffc++;			/* Count the character */
	    if (docrc && !remfile) {	/* Update file CRC */
		c = a;			/* Force conversion to unsigned char */
		z = crc16 ^ (long)c;
		crc16 = (crc16 >> 8) ^
		  (crcta[(z & 0xF0) >> 4] ^ crctb[z & 0x0F]);
	    }
	}
#ifdef CK_CTRLZ
	lastchar = a;
#endif /* CK_CTRLZ */
    }
    return(0);
}
#endif /* CKTUNING */
#endif /* NOXFER */

/*  P N B Y T E  --  Output next byte to file or other destination  */

static CK_OFF_T offc = 0L;

static int
#ifdef CK_ANSIC
pnbyte(CHAR c, int (*fn)(char))
#else
pnbyte(c,fn) CHAR c; int (*fn)();
#endif /* CK_ANSIC */
/* pnbyte */ {
    int rc;
    long z;

#ifdef OS2
#ifndef NOXFER
    if (xflg && !remfile) {		/* Write to virtual screen */
	char _a;
	_a = c & fmask;
	rc = conoc(_a);
	if (rc < 1)
	  return(-1);
    } else
#endif /* NOXFER */
#endif /* OS2 */
    {
	if (fn == putfil) {		/* Execute output function */
	    rc = zmchout(c);		/* to-file macro (fast) */
	} else if (!fn) {
	    rc = putchar(c);		/* to-screen macro (fast) */
        } else {
	    rc = (*fn)(c);		/* function call (not as fast) */
	}
	if (rc < 0)
	  return(rc);
    }
/*
  Both xgnbyte() and xpnbyte() increment ffc (the file byte counter).
  During file transfer, only one of these functions is called.  However,
  the TRANSLATE command is likely to call them both.  offc, therefore,
  contains the output byte count, necessary for handling the UCS-2 BOM.
  NOTE: It might be safe to just test for W_SEND, FTP or not.
*/
    if ((what & (W_FTP|W_SEND)) != (W_FTP|W_SEND)) {
	offc++;				/* Count the byte */
	ffc++;				/* Count the byte */
    }
#ifndef NOXFER
    if (docrc && !xflg && !remfile) {	/* Update file CRC */
	z = crc16 ^ (long)c;
	crc16 = (crc16 >> 8) ^
	  (crcta[(z & 0xF0) >> 4] ^ crctb[z & 0x0F]);
    }
#endif /* NOXFER */
    return(1);
}

/*
  X P N B Y T E  --  Translate and put next byte to file.

  Only for Unicode.  Call with next untranslated byte from incoming
  byte stream, which can be in any Transfer Character Set: UCS-2, UTF-8,
  Latin-1, Latin-Hebrew, etc.  Translates to the file character set and
  writes bytes to the output file.  Call with character to translate
  as an int, plus the transfer character set (to translate from) and the
  file character set (to translate to), or -1,0,0 to reset the UCS-2 byte
  number (which should be done at the beginning of a file).  If the transfer
  (source) character-set is UCS-2, bytes MUST arrive in Big-Endian order.
  Returns:
   -1: On error
    0: Nothing to write (mid-sequence)
   >0: Number of bytes written.
*/
#ifdef KANJI
static int jstate = 0, jx = 0;		/* For outputting JIS-7 */
static char jbuf[16] = { NUL, NUL };
#endif /* KANJI */

int
#ifdef CK_ANSIC
xpnbyte(int a, int tcs, int fcs, int (*fn)(char))
#else
xpnbyte(a,tcs,fcs,fn) int a, tcs, fcs; int (*fn)();
#endif /* CK_ANSIC */
/* xpnbyte */ {
#ifdef UNICODE
    extern int ucsbom;			/* Byte order */
#endif /* UNICODE */
    /* CHAR c; */			/* Unsigned char worker */
    static union ck_short uc, eu, sj;	/* UCS-2, EUC, and Shift-JIS workers */
    USHORT ch;				/* ditto... */
    USHORT * us = NULL;			/* ditto... */
    int c7, rc, haveuc = 0;		/* Return code and UCS-2 flag */
    int utferror = 0;			/* UTF-8 error */
    static int bn = 0;			/* UCS-2 byte number */
    int swapping = 0;			/* Swapping UCS bytes to output? */
					/* swapping must be 0 or 1 */
    if (a == -1 && (tcs | fcs) == 0) {	/* Reset in case previous run */
	bn = 0;				/* left bn at 1... */
	offc = (CK_OFF_T)0;
	debug(F101,"xpnbyte RESET","",bn);
	return(0);
    }
    debug(F001,"xpnbyte a","",a);

#ifdef UNICODE

/*
  byteorder = hardware byte order of this machine.
  ucsorder  = override by SET FILE UCS BYTE-ORDER command.
  fileorder = byte order of UCS-2 input file detected from BOM.
  swapping applies only when output charset is UCS-2.
*/
    if (ucsorder != 1 && ucsorder != 0)	/* Also just in case... */
      ucsorder = byteorder;
    if ((byteorder && !ucsorder) || (!byteorder && fileorder))
      swapping = 1;			/* Swapping bytes to output */

#ifdef COMMENT
    debug(F101,"xpnbyte ucsorder","",ucsorder);
    debug(F101,"xpnbyte swapping","",swapping);
#endif /* COMMENT */

    if (tcs == TC_UTF8) {		/* 'a' is from a UTF-8 stream */
	ch = a;
	if (fcs == TC_UTF8)		/* Output is UTF-8 too */
	  return(pnbyte(ch,fn));	/* so just copy. */
	rc = utf8_to_ucs2(ch,&us);	/* Otherwise convert to UCS-2 */
	if (rc == 0) {			/* Done with this sequence */
	    uc.x_short = *us;		/* We have a Unicode */
	    haveuc = 1;
	} else if (rc < 0) {		/* Error */
	    debug(F101,"xpnbyte UTF-8 conversion error","",rc);
	    haveuc = 1;			/* Replace by U+FFFD */
	    uc.x_short = *us;
	    utferror = 1;
	} else				/* Sequence incomplete */
	  return(0);
    } else if (tcs == TC_UCS2) {	/* 'a' is UCS-2 */
	/* Here we have incoming UCS-2 in guaranteed Big Endian order */
	/* so we must exchange bytes if local machine is Little Endian. */
	switch (bn) {			/* Which byte? */
	  case 0:			/* High... */
	    uc.x_char[byteorder] = (unsigned)a & 0xff;
	    bn++;
	    return(0);			/* Wait for next */
	  case 1:			/* Low... */
	    uc.x_char[1-byteorder] = (unsigned)a & 0xff;
	    bn = 0;			/* Done with sequence */
	    haveuc = 1;			/* Have a Unicode */
	}
    } else
#endif /* UNICODE */

#ifdef KANJI				/* Whether UNICODE is defined or not */
      if (tcs == TC_JEUC) {		/* Incoming Japanese EUC */
	int bad = 0;
	static int kanji = 0;		/* Flags set in case 0 for case 1 */
	static int kana = 0;
	switch (bn) {			/* Byte number */
	  case 0:			/* Byte 0 */
	    eu.x_short = 0;
	    if ((a & 0x80) == 0) {
		sj.x_short = (unsigned)a & 0xff; /* Single byte */
		kanji = kana = 0;
	    } else {			/* Double byte */
		c7 = a & 0x7f;
		if (c7 > 0x20 && c7 < 0x7f) { /* Kanji */
		    eu.x_char[byteorder] = (CHAR) a;  /* Store first byte */
		    bn++;		      /* Set up for second byte */
		    kanji = 1;
		    kana = 0;
		    return(0);
		} else if (a == 0x8e) {	/* SS2 -- Katakana prefix */
		    eu.x_char[byteorder] = (CHAR) a; /* Save it */
		    bn++;
		    kana = 1;
		    kanji = 0;
		    return(0);
		} else {
		    bad++;
		}
	    }
	    break;
	  case 1:			/* Byte 1 */
	    bn = 0;
	    if (kanji) {
		eu.x_char[1-byteorder] = (CHAR) a;
		sj.x_short = eu_to_sj(eu.x_short);
		break;
	    } else if (kana) {
		sj.x_short = (CHAR) (a | 0x80);
		break;
	    } else {			/* (shouldn't happen) */
		bad++;
	    }
	}
	/* Come here with one Shift-JIS character */

#ifdef UNICODE
	if (bad) {
	    uc.x_short = 0xfffd;
	} else {
	    uc.x_short = sj_to_un(sj.x_short); /* Convert to Unicode */
	}
	haveuc = 1;
#endif /* UNICODE */
    } else
#endif /* KANJI */

#ifdef UNICODE
	uc.x_short = (unsigned)a & 0xff; /* Latin-1 or whatever... */

    /* Come here with uc = the character to be translated. */
    /* If (haveuc) it's UCS-2 in native order, otherwise it's a byte. */

    debug(F101,"xpnbyte haveuc","",haveuc);

    if (haveuc) {			/* If we have a Unicode... */
	debug(F001,"xpnbyte uc.x_short","[A]",uc.x_short);
	debug(F101,"xpnbyte feol","",feol);
	if (what & W_XFER) {		/* If transferring a file */
	    if (feol && uc.x_short == CR) { /* handle eol conversion. */
		return(0);
	    } else if (feol && uc.x_short == LF) {
		uc.x_short = feol;
	    }
	}
	debug(F001,"xpnbyte uc.x_short","[B]",uc.x_short);
	if (fcs == FC_UCS2) {		/* And FCS is UCS-2 */
	    /* Write out the bytes in the appropriate byte order */
	    int count = 0;
#ifndef IKSDONLY
#ifdef OS2
            extern int k95stdout,wherex[],wherey[];
			extern unsigned char colorcmd;
            union {
                USHORT ucs2;
                UCHAR  bytes[2];
            } output;
#endif /* OS2 */
#endif /* IKSDONLY */
	    if (!offc && ucsbom) {	/* Beginning of file? */

#ifndef IKSDONLY
#ifdef OS2
                if (fn == NULL && !k95stdout && !inserver) {
		    offc++;
#ifdef COMMENT
		    /* Don't print the BOM to the display */
                    output.bytes[0] = (!ucsorder ? 0xff : 0xfe);
                    output.bytes[1] = (!ucsorder ? 0xfe : 0xff);

                    VscrnWrtUCS2StrAtt(VCMD,
                                       &output.ucs2,
                                       1,
                                       wherey[VCMD],
                                       wherex[VCMD],
                                       &colorcmd
                                       );
#endif /* COMMENT */
                } else 
#endif /* OS2 */
#endif /* IKSDONLY */
                {
		    if ((rc = pnbyte((ucsorder ? 0xff : 0xfe),fn)) < 0)
		      return(rc);	/* BOM */
		    if ((rc = pnbyte((ucsorder ? 0xfe : 0xff),fn)) < 0)
		      return(rc);
		}
		count += 2;
	    }
	    if (utferror) {
#ifndef IKSDONLY
#ifdef OS2
                if (fn == NULL && !k95stdout && !inserver) {
                    offc++;
                    output.bytes[0] = (!ucsorder ? 0xfd : 0xff);
                    output.bytes[1] = (!ucsorder ? 0xff : 0xfd);

                    VscrnWrtUCS2StrAtt(VCMD,
                                       &output.ucs2,
                                       1,
                                       wherey[VCMD],
                                       wherex[VCMD],
                                       &colorcmd
                                       );
                } else 
#endif /* OS2 */
#endif /* IKSDONLY */
		{
		    if ((rc = pnbyte((ucsorder ? 0xfd : 0xff),fn)) < 0)
		      return(rc);
		    if ((rc = pnbyte((ucsorder ? 0xff : 0xfd),fn)) < 0)
		      return(rc);
		}
		count += 2;
	    }
#ifndef IKSDONLY
#ifdef OS2
            if (fn == NULL && !k95stdout && !inserver) {
                offc++;
                output.bytes[0] = uc.x_char[swapping];
                output.bytes[1] = uc.x_char[1-swapping];

                VscrnWrtUCS2StrAtt(VCMD,
                                   &output.ucs2,
                                   1,
                                   wherey[VCMD],
                                   wherex[VCMD],
                                   &colorcmd
                                   );
            } else 
#endif /* OS2 */
#endif /* IKSDONLY */
            {
		if ((rc = pnbyte(uc.x_char[swapping],fn)) < 0)
		  return(rc);
		if ((rc = pnbyte(uc.x_char[1-swapping],fn)) < 0)
		  return(rc);
	    }
	    count += 2;
	    return(count);
	} else if (fcs == FC_UTF8) {	/* Convert to UTF-8 */
	    CHAR * buf = NULL;
	    int i, count;
	    if (utferror) {
		if ((rc = pnbyte((ucsorder ? 0xbd : 0xff),fn)) < 0)
		  return(rc);
		if ((rc = pnbyte((ucsorder ? 0xff : 0xbd),fn)) < 0)
		  return(rc);
	    }
	    if ((count = ucs2_to_utf8(uc.x_short,&buf)) < 1)
	      return(-1);
	    debug(F011,"xpnbyte buf",buf,count);
	    for (i = 0; i < count; i++)
	      if ((rc = pnbyte(buf[i],fn)) < 0)
		return(rc);
	    if (utferror)
	      count += 2;
	    return(count);
	} else {			/* Translate UCS-2 to byte */
	    if (uc.x_short == 0x2028 || uc.x_short == 0x2029) {
		if (utferror)
		  pnbyte(UNK,fn);
		if (feol)
		  return(pnbyte((CHAR)feol,fn));
		if ((rc = pnbyte((CHAR)CR,fn)) < 0)
		  return(rc);
		if ((rc = pnbyte((CHAR)LF,fn)) < 0)
		  return(rc);
		else
		  return(utferror ? 3 : 2);
	    } else if (xuf) {		/* UCS-to-FCS function */
		int x = 0;
		if (utferror)
		  pnbyte(UNK,fn);
		if ((rc = (*xuf)(uc.x_short)) < 0) /* These can fail... */
		  ch = UNK;
		else
		  ch = (unsigned)((unsigned)rc & 0xffff);
		x = pnbyte(ch,fn);
		if (x < 0)
		  return(x);
		else if (utferror)
		  x++;
		return(x);
#ifdef KANJI

/*  Also see the non-Unicode Kanji section further down in this function. */

	    } else if (fcsinfo[fcs].alphabet == AL_JAPAN) {

		/* Translate UCS-2 to Japanese set */
		debug(F001,"xpnbyte uc","",uc.x_short);
		sj.x_short = un_to_sj(uc.x_short); /* First to Shift-JIS */
		debug(F001,"xpnbyte sj","",sj.x_short);

		switch (fcs) {		/* File character set */
		  case FC_SHJIS:	/* Shift-JIS -- just output it */
		    if (sj.x_char[byteorder]) /* But not high byte if zero */
		      if ((rc = pnbyte((CHAR)sj.x_char[byteorder],fn)) < 0)
			return(rc);
		    if ((rc = pnbyte((CHAR)sj.x_char[1-byteorder],fn)) < 0)
		      return(rc);
		    return(2);
		  case FC_JEUC:		/* EUC-JP */
		    eu.x_short = sj_to_eu(sj.x_short); /* Shift-JIS to EUC */
		    debug(F001,"xpnbyte eu","",eu.x_short);
		    if (eu.x_short == 0xffff) { /* Bad */
			if ((rc = pnbyte(UNK,fn)) < 0)
			  return(rc);
			return(1);
		    } else {		/* Good */
			int count = 0;	/* Write high byte if not zero */
			if (eu.x_char[byteorder]) {
			    if ((rc=pnbyte((CHAR)eu.x_char[byteorder],fn)) < 0)
			      return(rc);
			    count++;
			}
			/* Always write low byte */
			if ((rc = pnbyte((CHAR)eu.x_char[1-byteorder],fn)) < 0)
			  return(rc);
			count++;
			return(count);
		    }
		    break;

		  case FC_JIS7:		/* JIS-7 */
		  case FC_JDEC:		/* DEC Kanji */
		    eu.x_short = sj_to_eu(sj.x_short); /* Shift-JIS to EUC */
		    if (eu.x_short == 0xffff) { /* Bad */
			debug(F001,"xpnbyte bad eu","",eu.x_short);
			if ((rc = pnbyte(UNK,fn)) < 0)
			  return(rc);
			return(1);
		    } else {		/* Good */
			int i;
			/* Use another name - 'a' hides parameter */
			/* It's OK as is but causes compiler warnings */
			char a = eu.x_char[1-byteorder]; /* Low byte */
			debug(F001,"xpnbyte eu","",eu.x_short);
			if (eu.x_char[byteorder] == 0) { /* Roman */
			    switch (jstate) {
			      case 1:	/* Current state is Katakana */
				jbuf[0] = 0x0f;	/* SI */
				jbuf[1] = a;
				jx = 2;
				break;
			      case 2:	/* Current state is Kanji */
				jbuf[0] = 0x1b;	/* ESC */
				jbuf[1] = 0x28;	/* ( */
				jbuf[2] = 0x4a;	/* J */
				jbuf[3] = a;
				jx = 4;
				break;
			      default:	/* Current state is Roman */
				jbuf[0] = a;
				jx = 1;
				break;
			    }
			    jstate = 0;	/* New state is Roman */
			} else if (eu.x_char[byteorder] == 0x8e) { /* Kana */
			    jx = 0;
			    switch (jstate) {
			      case 2:		   /* from Kanji */
				jbuf[jx++] = 0x1b; /* ESC */
				jbuf[jx++] = 0x28; /* ( */
				jbuf[jx++] = 0x4a; /* J */
			      case 0:		   /* from Roman */
				jbuf[jx++] = 0x0e; /* SO */
			      default:		   /* State is already Kana*/
				jbuf[jx++] = (a & 0x7f); /* and the char */
				break;
			    }
			    jstate = 1;	/* New state is Katakana */
			} else {	/* Kanji */
			    jx = 0;
			    switch (jstate) {
			      case 1:	/* Current state is Katakana */
				jbuf[jx++] = 0x0f; /* SI  */
			      case 0:	/* Current state is Roman */
				jbuf[jx++] = 0x1b; /* ESC */
				jbuf[jx++] = 0x24; /* $   */
				jbuf[jx++] = 0x42; /* B   */
			      default:	/* Current state is already Kanji */
				jbuf[jx++] = eu.x_char[byteorder] & 0x7f;
				jbuf[jx++] = eu.x_char[1-byteorder] & 0x7f;
				break;
			    }
			    jstate = 2;	/* Set state to Kanji */
			}
			for (i = 0; i < jx; i++) /* Output the result */
			  if ((rc = pnbyte(jbuf[i],fn)) < 0)
			    return(rc);
			return(jx);	/* Return its length */
		    }
		}
#endif /* KANJI */
	    } else {			/* No translation function */
		int count = 0;
		if (utferror) {
		    if ((rc = pnbyte((ucsorder ? 0xfd : 0xff),fn)) < 0)
		      return(rc);
		    if ((rc = pnbyte((ucsorder ? 0xff : 0xfd),fn)) < 0)
		      return(rc);
		    count += 2;
		}
		if ((rc = pnbyte(uc.x_char[swapping],fn)) < 0)
		  return(rc);
		if ((rc = pnbyte(uc.x_char[1-swapping],fn)) < 0)
		  return(rc);
		count += 2;
		return(count);
	    }
	}
    } else {				/* Byte to Unicode */
	if (xtu) {			/* TCS-to-UCS function */
	    if (((tcsinfo[tcs].size > 128) && (uc.x_short & 0x80)) ||
		tcsinfo[tcs].size <= 128)
	      uc.x_short = (*xtu)(uc.x_short);
	}
	if (fcs == FC_UCS2) {		/* And FCS is UCS-2 */
	    /* Write out the bytes in the appropriate byte order */
	    if (!offc && ucsbom) {	/* Beginning of file? */
		if ((rc = pnbyte((ucsorder ? 0xff : 0xfe),fn)) < 0) /* BOM */
		  return(rc);
		if ((rc = pnbyte((ucsorder ? 0xfe : 0xff),fn)) < 0)
		  return(rc);
	    }
	    if ((rc = pnbyte(uc.x_char[swapping],fn)) < 0)
	      return(rc);
	    if ((rc = pnbyte(uc.x_char[1-swapping],fn)) < 0)
	      return(rc);
	    return(2);
	} else if (fcs == FC_UTF8) {	/* Convert to UTF-8 */
	    CHAR * buf = NULL;
	    int i, count;
	    if ((count = ucs2_to_utf8(uc.x_short,&buf)) < 1)
	      return(-1);
	    for (i = 0; i < count; i++)
	      if ((rc = pnbyte(buf[i],fn)) < 0)
		return(rc);
	    return(count);
	} else {
	    debug(F100,"xpnbyte impossible combination","",0);
	    return(-1);
	}
    }
#else
#ifdef KANJI
/*
  This almost, but not quite, duplicates the Kanji section above.
  There is no doubt a way to combine the sections more elegantly,
  but probably only at the expense of additional execution overhead.
  As matters stand, be careful to reflect any changes in this section
  to the other Kanji section above.
*/
    if (tcs == TC_JEUC) {		/* Incoming Japanese EUC */
	int count = 0;
	switch (fcs) {			/* File character set */
	  case FC_SHJIS:		/* Shift-JIS -- just output it */
	    if (sj.x_char[byteorder])	/* But not high byte if zero */
	      if ((rc = pnbyte((CHAR)sj.x_char[byteorder],fn)) < 0)
		return(rc);
	    count++;
	    if ((rc = pnbyte((CHAR)sj.x_char[1-byteorder],fn)) < 0)
	      return(rc);
	    count++;
	    return(count);
	  case FC_JEUC:			/* EUC-JP */
	    eu.x_short = sj_to_eu(sj.x_short); /* Shift-JIS to EUC */
	    debug(F001,"xpnbyte FC_JEUC eu","",eu.x_short);
	    if (eu.x_short == 0xffff) { /* Bad */
		if ((rc = pnbyte(UNK,fn)) < 0)
		  return(rc);
		return(1);
	    } else {			/* Good */
		int count = 0;		/* Write high byte if not zero */
		if (eu.x_char[byteorder]) {
		    if ((rc = pnbyte((CHAR)eu.x_char[byteorder],fn)) < 0)
		      return(rc);
		    count++;
		}
		/* Always write low byte */
		if ((rc = pnbyte((CHAR)eu.x_char[1-byteorder],fn)) < 0)
		  return(rc);
		count++;
		return(count);
	    }
	    break;

	  case FC_JIS7:			/* JIS-7 */
	  case FC_JDEC:			/* DEC Kanji */
	    eu.x_short = sj_to_eu(sj.x_short); /* Shift-JIS to EUC */
	    if (eu.x_short == 0xffff) { /* Bad */
		debug(F001,"xpnbyte FC_JIS7 bad eu","",eu.x_short);
		if ((rc = pnbyte(UNK,fn)) < 0)
		  return(rc);
		return(1);
	    } else {			/* Good */
		int i;
		char a = eu.x_char[1-byteorder]; /* Low byte */
		debug(F001,"xpnbyte FC_JIS7 eu","",eu.x_short);
		if (eu.x_char[byteorder] == 0) { /* Roman */
		    switch (jstate) {
		      case 1:		/* Current state is Katakana */
			jbuf[0] = 0x0f;	/* SI */
			jbuf[1] = a;
			jx = 2;
			break;
		      case 2:		/* Current state is Kanji */
			jbuf[0] = 0x1b;	/* ESC */
			jbuf[1] = 0x28;	/* ( */
			jbuf[2] = 0x4a;	/* J */
			jbuf[3] = a;
			jx = 4;
			break;
		      default:		/* Current state is Roman */
			jbuf[0] = a;
			jx = 1;
			break;
		    }
		    jstate = 0;		/* New state is Roman */
		} else if (eu.x_char[byteorder] == 0x8e) { /* Kana */
		    jx = 0;
		    switch (jstate) {
		      case 2:		   /* from Kanji */
			jbuf[jx++] = 0x1b; /* ESC */
			jbuf[jx++] = 0x28; /* ( */
			jbuf[jx++] = 0x4a; /* J */
		      case 0:		   /* from Roman */
			jbuf[jx++] = 0x0e; /* SO */
		      default:		   /* State is already Kana*/
			jbuf[jx++] = (a & 0x7f); /* and the char */
			break;
		    }
		    jstate = 1;		/* New state is Katakana */
		} else {		/* Kanji */
		    jx = 0;
		    switch (jstate) {
		      case 1:		/* Current state is Katakana */
			jbuf[jx++] = 0x0f; /* SI  */
		      case 0:		/* Current state is Roman */
			jbuf[jx++] = 0x1b; /* ESC */
			jbuf[jx++] = 0x24; /* $   */
			jbuf[jx++] = 0x42; /* B   */
		      default:		/* Current state is already Kanji */
			jbuf[jx++] = eu.x_char[byteorder] & 0x7f;
			jbuf[jx++] = eu.x_char[1-byteorder] & 0x7f;
			break;
		    }
		    jstate = 2;		/* Set state to Kanji */
		}
		for (i = 0; i < jx; i++) /* Output the result */
		  if ((rc = pnbyte(jbuf[i],fn)) < 0)
		    return(rc);
		return(jx);		/* Return its length */
	    }
	  default:
	    if (sj.x_short < 0x80)
	      return(sj.x_short);
	    else
	      return('?');
	}
    }
#endif /* KANJI */
#endif /* UNICODE */
    debug(F100,"xpnbyte BAD FALLTHRU","",0);
    return(-1);
}

#ifndef NOXFER

/*  D E C O D E  --  Kermit Data-packet decoder  */

int
#ifdef CK_ANSIC
decode(CHAR *buf, int (*fn)(char), int xlate)
#else
decode(buf,fn,xlate) register CHAR *buf; register int (*fn)(); int xlate;
#endif /* CK_ANSIC */
/* decode */ {
    register unsigned int a, a7, a8, b8; /* Various copies of current char */
    int t;				/* Int version of character */
    int ssflg;				/* Character was single-shifted */
    int ccpflg;				/* For Ctrl-unprefixing stats */
    int len;
    long z;
    CHAR c;
/*
  Catch the case in which we are asked to decode into a file that is not open,
  for example, if the user interrupted the transfer, but the other Kermit
  keeps sending.
*/
    if ((cxseen || czseen || discard) && (fn == putfil))
      return(0);

#ifdef COMMENT
#ifdef CKTUNING
    if (binary && !parity)
      return(bdecode(buf,fn));
#endif /* CKTUNING */
#endif /* COMMENT */
    debug(F100,"DECODE","",0);

    xdbuf = buf;			/* Make global copy of pointer. */
    rpt = 0;				/* Initialize repeat count. */

    len = rln;				/* Number of bytes in data field */
    while (len > 0) {			/* Loop for each byte */
        a = *xdbuf++ & 0xff;		/* Get next character */
	len--;
	if (a == rptq && rptflg) {	/* Got a repeat prefix? */
	    rpt = xunchar(*xdbuf++ & 0xFF); /* Yes, get the repeat count, */
	    rptn += rpt;
	    a = *xdbuf++ & 0xFF;	/* and get the prefixed character. */
	    len -= 2;
	}
	b8 = lsstate ? 0200 : 0;	/* 8th-bit value from SHIFT-STATE */
	if (ebqflg && a == ebq) {	/* Have 8th-bit prefix? */
	    b8 ^= 0200;			/* Yes, invert the 8th bit's value, */
	    ssflg = 1;			/* remember we did this, */
	    a = *xdbuf++ & 0xFF;	/* and get the prefixed character. */
	    len--;
	} else ssflg = 0;
	ccpflg = 0;
	if (a == ctlq) {		/* If control prefix, */
	    a  = *xdbuf++ & 0xFF;	/* get its operand */
	    len--;
	    a7 = a & 0x7F;		/* and its low 7 bits. */
	    if ((a7 >= 0100 && a7 <= 0137) || a7 == '?') { /* Controllify */
		a = ctl(a);		/* if in control range. */
		a7 = a & 0x7F;
		ccpflg = 1;		/* Note that we did this */
		ccp++;			/* Count for stats */
	    }
	} else a7 = a & 0x7f;		/* Not control quote */
	if (a7 < 32 || a7 == 127) {	/* Control character? */
	    if (!ccpflg) ccu++;		/* A bare one, count it */
	    if (lscapu) {		/* If doing locking shifts... */
		if (lsstate)		/* If SHIFTED */
		  a8 = (a & ~b8) & 0xFF; /* Invert meaning of 8th bit */
		else			/* otherwise */
		  a8 = a | b8;		/* OR in 8th bit */
		/* If we're not in a quoted sequence */
		if (!lsquote && (!lsstate || !ssflg)) {
		    if (a8 == DLE) {	/* Check for DLE quote */
			lsquote = 1;	/* prefixed by single shift! */
			continue;
		    } else if (a8 == SO) { /* Check for Shift-Out */
			lsstate = 1;	/* SHIFT-STATE = SHIFTED */
			continue;
		    } else if (a8 == SI) { /* or Shift-In */
			lsstate = 0;	/* SHIFT-STATE = UNSHIFTED */
			continue;
		    }
		} else lsquote = 0;
	    }
	}
	a |= b8;			/* OR in the 8th bit */
	if (rpt == 0) rpt = 1;		/* If no repeats, then one */
#ifndef NOCSETS
	if (!binary) {			/* If in text mode, */
	    if (tcharset != TC_UCS2) {
		if (feol && a == CR)	/* Convert CRLF to newline char */
		  continue;
		if (feol && a == LF)
		  a = feol;
	    }
	    if (xlatype == XLA_BYTE)	/* Byte-for-byte - do it now */
	      if (xlate && rx) a = (*rx)((CHAR) a);
    	}
#endif /* NOCSETS */
	/* (PWP) Decoding speedup via buffered output and a macro... */
	if (fn == putfil) {
	    for (; rpt > 0; rpt--) {	/* Output the char RPT times */
#ifdef CALIBRATE
		if (calibrate) {
		    ffc++;
		    continue;
		}
#endif /* CALIBRATE */

/* Note: The Unicode and Kanji sections can probably be combined now; */
/* the Unicode method (xpnbyte()) covers Kanji too. */

#ifdef UNICODE
		if (!binary && xlatype == XLA_UNICODE)
		  t = xpnbyte((unsigned)((unsigned)a & 0xff),
			      tcharset,
			      fcharset,
			      fn
			      );
		else
#endif /* UNICODE */
#ifdef KANJI
		if (!binary && tcharset == TC_JEUC &&
		    fcharset != FC_JEUC) { /* Translating from J-EUC */
		    if (!ffc) xkanjf();
		    if (xkanji(a,fn) < 0)  /* to something else? */
		      return(-1);
		    else t = 1;
		} else
#endif /* KANJI */
		{
#ifdef OS2
		      if (xflg && !remfile) { /* Write to virtual screen */
			  char _a;
			  _a = a & fmask;
			  t = conoc(_a);
			  if (t < 1)
			    t = -1;
		      } else
#endif /* OS2 */
			t = zmchout(a & fmask); /* zmchout is a macro */
		}
		if (t < 0) {
		    debug(F101,"decode write errno","",errno);
		    return(-1);
		}
#ifdef UNICODE
		if (xlatype != XLA_UNICODE || binary) {
		    ffc++;		/* Count the character */
		    if (docrc && !xflg && !remfile) { /* Update file CRC */
			c = a;		/* Force conversion to unsigned char */
			z = crc16 ^ (long)c;
			crc16 = (crc16 >> 8) ^
			  (crcta[(z & 0xF0) >> 4] ^ crctb[z & 0x0F]);
		    }
		}
#endif /* UNICODE */
	    }
	} else {			/* Output to something else. */
	    a &= fmask;			/* Apply file mask */
	    for (; rpt > 0; rpt--) {	/* Output the char RPT times */
#ifdef CALIBRATE
		if (calibrate) {
		    ffc++;
		    continue;
		}
#endif /* CALIBRATE */
		if ((*fn)((char) a) < 0) return(-1); /* Send to output func. */
	    }
	}
#ifdef CK_CTRLZ
	lastchar = a;
#endif /* CK_CTRLZ */
    }
    return(0);
}

/*  G E T P K T -- Fill a packet data field  */

/*
  Gets characters from the current source -- file or memory string.
  Encodes the data into the packet, filling the packet optimally.
  Set first = 1 when calling for the first time on a given input stream
  (string or file).

  Call with:
    bufmax -- current send-packet size
    xlate  -- flag: 0 to skip character-set translation, 1 to translate

  Uses global variables:
    t     -- current character.
    first -- flag: 1 to start up, 0 for input in progress, -1 for EOF.
    next  -- next character (not used any more).
    data  -- pointer to the packet data buffer.
    size  -- number of characters in the data buffer.
    memstr - flag that input is coming from a memory string instead of a file.
    memptr - pointer to string in memory.
    (*sx)()  character set translation function

  Returns:
    The size as value of the function, and also sets global "size",
    and fills (and null-terminates) the global data array.
    Returns:
      0 on EOF.
     -1 on fatal (internal) error.
     -3 on timeout (e.g. when reading data from a pipe).

  Rewritten by Paul W. Placeway (PWP) of Ohio State University, March 1989.
  Incorporates old getchx() and encode() inline to reduce function calls,
  uses buffered input for much-improved efficiency, and clears up some
  confusion with line termination (CRLF vs LF vs CR).

  Rewritten again by Frank da Cruz to incorporate locking shift mechanism,
  May 1991.  And again in 1998 for efficiency, etc, with a separate
  bgetpkt() split out for binary-mode no-parity transfers.
*/

/*
  Note: Separate Kanji support dates from circa 1991 and now (1999) can most
  likely be combined with the the Unicode support: the xgnbyte()/xpnbyte()
  mechanism works for both Unicode and Kanji.
*/
#ifdef KANJI
int
kgetf(
#ifdef CK_ANSIC
      VOID
#endif /* CK_ANSIC */
      ) {
    if (funcstr)
      return((*funcptr)());
    else
      return(zminchar());
}

int
kgetm(
#ifdef CK_ANSIC
      VOID
#endif /* CK_ANSIC */
      ) {
    int x;
    if ((x = *memptr++)) return(x);
    else return(-1);
}
#endif /* KANJI */

/*
  Lookahead function to decide whether locking shift is worth it.  Looks at
  the next four input characters to see if all of their 8th bits match the
  argument.  Call with 0 or 0200.  Returns 1 on match, 0 if they don't match.
  If we don't happen to have at least 4 more characters waiting in the input
  buffer, returns 1.  Note that zinptr points two characters ahead of the
  current character because of repeat-count lookahead.
*/
int
lslook(b) unsigned int b; {		/* Locking Shift Lookahead */
    int i;
    if (zincnt < 3)			/* If not enough chars in buffer, */
      return(1);			/* force shift-state switch. */
    b &= 0200;				/* Force argument to proper form. */
    for (i = -1; i < 3; i++)		/* Look at next 5 characters to */
      if (((*(zinptr+i)) & 0200) != b)	/* see if all their 8th bits match.  */
	return(0);			/* They don't. */
    return(1);				/* They do. */
}

/* Routine to compute maximum data length for packet to be filled */

int
maxdata() {				/* Get maximum data length */
    int n, len;
    debug(F101,"maxdata spsiz 1","",spsiz);
    if (spsiz < 0)			/* How could this happen? */
      spsiz = DSPSIZ;
    debug(F101,"maxdata spsiz 2","",spsiz);
    n = spsiz - 5;			/* Space for Data and Checksum */
    if (n > 92 && n < 96) n = 92;	/* "Short" Long packets don't pay */
    if (n > 92 && lpcapu == 0)		/* If long packets needed, */
      n = 92;				/* make sure they've been negotiated */
    len = n - bctl;			/* Space for data */
    if (n > 92) len -= 3;		/* Long packet needs header chksum */
    debug(F101,"maxdata len 1","",len);
    if (len < 0) len = 10;
    debug(F101,"maxdata len 2","",len);
    return(len);
}

static CHAR leftover[9] = { '\0','\0','\0','\0','\0','\0','\0','\0','\0' };
static int nleft = 0;

#ifdef CKTUNING
/*
  When CKTUNING is defined we use this special trimmed-down version of getpkt
  to speed up binary-mode no-parity transfers.  When CKTUNING is not defined,
  or for text-mode or parity transfers, we use the regular getpkt() function.
  Call just like getpkt() but test first for transfer mode and parity.  NOTE:
  This routine is only to be called when sending a real file -- not for
  filenames, server responses, etc, because it only reads from the input file.
  See getpkt() for more detailed commentary.
*/
static int
bgetpkt(bufmax) int bufmax; {
    register CHAR rt = t, rnext;
    register CHAR *dp, *odp, *p1, *p2;
    register int x = 0, a7;

    CHAR xxrc, xxcq;			/* Pieces of prefixed sequence */

    long z;				/* A long worker (for CRC) */

    if (!binary || parity || memstr)	/* JUST IN CASE caller didn't test */
      return(getpkt(bufmax,!binary));

    if (!data) {
	debug(F100,"SERIOUS ERROR: bgetpkt data == NULL","",0);
	return(-1);
    }
    dp = data;				/* Point to packet data buffer */
    size = 0;				/* And initialize its size */
    bufmax = maxdata();			/* Get maximum data length */

#ifdef DEBUG
    if (deblog)
      debug(F101,"bgetpkt bufmax","",bufmax);
#endif /* DEBUG */

    if (first == 1) {			/* If first character of this file.. */
	ffc = (CK_OFF_T)0;		/* reset file character counter */
#ifdef COMMENT
/* Moved to below */
	first = 0;			/* Next character won't be first */
#endif /* COMMENT */
	*leftover = '\0';		/* Discard any interrupted leftovers */
	nleft = 0;

	/* Get first character of file into rt, watching out for null file */

#ifdef CALIBRATE
	if (calibrate) {
#ifdef NORANDOM
	    rt = 17;
#else
	    rt = cal_a[rand() & 0xff];
#endif /* NORANDOM */
	    first = 0;
	} else
#endif /* CALIBRATE */

	if ((x = zminchar()) < 0) {	/* EOF or error */
	    if (x == -3) {		/* Timeout. */
		size = (dp - data);
		debug(F101,"bgetpkt timeout size","",size);
		return((size == 0) ? x : size);
	    }
	    first = -1;
	    size = 0;
	    if (x == -2) {		/* Error */
		debug(F100,"bgetpkt: input error","",0);
		cxseen = 1;		/* Interrupt the file transfer */
	    } else {
		debug(F100,"bgetpkt empty file","",0);
	    }
	    return(0);
	}
	first = 0;			/* Next char will not be the first */
	ffc++;				/* Count a file character */
	rt = (CHAR) x;			/* Convert int to char */
	if (docrc && (what & W_SEND)) {	/* Accumulate file crc */
	    z = crc16 ^ (long)rt;
	    crc16 = (crc16 >> 8) ^
	      (crcta[(z & 0xF0) >> 4] ^ crctb[z & 0x0F]);
	}
	rt &= fmask;			/* Apply SET FILE BYTESIZE mask */

    } else if (first == -1 && nleft == 0) { /* EOF from last time */

        return(size = 0);
    }
/*
  Here we handle characters that were encoded for the last packet but
  did not fit, and so were saved in the "leftover" array.
*/
    if (nleft) {
	for (p1 = leftover; nleft > 0; nleft--) /* Copy leftovers */
	  *dp++ = *p1++;
	*leftover = '\0';		/* Delete leftovers */
	nleft = 0;
    }
    if (first == -1)			/* Handle EOF */
      return(size = (dp - data));

/* Now fill up the rest of the packet. */

    rpt = 0;				/* Initialize character repeat count */

    while (first > -1) {		/* Until EOF... */
#ifdef CALIBRATE
	if (calibrate) {		/* We generate our own "file" */
	    if (ffc >= calibrate) {	/* EOF */
		first = -1;
		ffc--;
	    } else {			/* Generate next character */
		if (cal_j > CAL_M * ffc)
		  cal_j = cal_a[ffc & 0xff];
		x = (unsigned)cal_a[(cal_j & 0xff)];
		if (x == rt) x ^= 2;
	    }
	    ffc++;
	    cal_j += (unsigned int)(ffc + CAL_O);
	} else
#endif /* CALIBRATE */
	if ((x = zminchar()) < 0) {	/* Check for EOF */
	    if (x == -3) {		/* Timeout. */
		t = rt;
		size = (dp-data);
		debug(F101,"bgetpkt timeout size","",size);
		return((size == 0) ? x : size);
	    }
	    first = -1;			/* Flag eof for next time. */
	    if (x == -2) cxseen = 1;	/* If error, cancel this file. */
	} else {
	    ffc++;			/* Count the character */
	    if (docrc && (what & W_SEND)) { /* Accumulate file crc */
		z = crc16 ^ (long)((CHAR)x & 0xff);
		crc16 = (crc16 >> 8) ^
		  (crcta[(z & 0xF0) >> 4] ^ crctb[z & 0x0F]);
	    }
	}
	rnext = (CHAR) (x & fmask);	/* Apply file mask */
/*
  At this point, the character we just read is in rnext,
  and the character we are about to encode into the packet is in rt.
*/
	odp = dp;			/* Remember where we started. */
	xxrc = xxcq = NUL;		/* Clear these. */
/*
  Now encode the character according to the options that are in effect:
    ctlp[]: whether this control character needs prefixing.
    rptflg: repeat counts enabled.
    Other options don't apply in this routine.
*/
	if (rptflg && (rt == rnext) && (first == 0)) { /* Got a run... */
	    if (++rpt < 94) {		/* Below max, just count */
		continue;		/* go back and get another */
	    } else if (rpt == 94) {	/* Reached max, must dump */
		xxrc = (CHAR) tochar(rpt); /* Put the repeat count here */
		rptn += rpt;		/* Accumulate it for statistics */
		rpt = 0;		/* And reset it */
	    }
	} else if (rpt > 0) {		/* End of run */
	    xxrc = (CHAR)tochar(++rpt); /* The count */
	    rptn += rpt;		/* For stats */
	    rpt = 0;			/* Reset repeat count */
	}
	a7 = rt & 0177;			/* Get low 7 bits of character */
	if (
#ifdef CK_SPEED
	    ctlp[(unsigned)(rt & 0xff)]	/* Lop off any "sign" extension */
#else
	    (a7 < SP) || (a7 == DEL)
#endif /* CK_SPEED */
	    ) {				/* Do control prefixing if necessary */
	    xxcq = myctlq;		/* The prefix */
	    ccp++;			/* Count it */
	    rt = (CHAR) ctl(rt);	/* Uncontrollify the character */
	}
#ifdef CK_SPEED
	else if ((a7 < SP) || (a7 == DEL)) /* Count an unprefixed one */
	  ccu++;
#endif /* CK_SPEED */

	if (a7 == myctlq)		/* Always prefix the control prefix */
	  xxcq = myctlq;

	if ((rptflg) && (a7 == rptq))	/* If it's the repeat prefix, */
	  xxcq = myctlq;		/* prefix it if doing repeat counts */

/* Now construct the prefixed sequence */

	if (xxrc) {			/* Repeat count */
#ifdef COMMENT
	    if (xxrc == (CHAR) '"' && !xxcq) { /* 2 in a row & not prefixed */
		*dp++ = rt;		/* So just do this */
	    } else {			/* More than two or prefixed */
		*dp++ = (CHAR) rptq; *dp++ = xxrc; /* Emit repeat sequence */
	    }
#else					/* CHECK THIS */
	    if (xxrc == (CHAR) '"' && !xxcq) { /* 2 in a row & not prefixed */
                if (dp == data) {
                    *dp++ = rt;		/* So just do this */
                } else if (*(dp-1) == rt) {
                    *dp++ = (CHAR) rptq;
		    *dp++ = xxrc;	/* Emit repeat sequence */
                } else {
                    *dp++ = rt;		/* So just do this */
                }
	    } else {			/* More than two or prefixed */
		*dp++ = (CHAR) rptq;
		*dp++ = xxrc;		/* Emit repeat sequence */
	    }
#endif /* COMMENT */
	}
	if (xxcq) { *dp++ = myctlq; }	/* Control prefix */
	*dp++ = rt;			/* Finally, the character itself */
	rt = rnext;			/* Next character is now current. */

/* Done encoding the character.  Now take care of packet buffer overflow. */

	size = dp - data;		/* How many bytes we put in buffer. */
	if (size >= bufmax) {		/* If too big, save some for next. */
	    *dp = '\0';			/* Mark the end. */
	    if (size > bufmax) {	/* if packet is overfull */
		/* Copy the part that doesn't fit into the leftover buffer, */
		/* taking care not to split a prefixed sequence. */
		int i;
		nleft = dp - odp;
		p1 = leftover;
		p2 = odp;
		for (i = 0; i < nleft; i++)
		  *p1++ = *p2++;
		size = odp - data;	/* Return truncated packet. */
		*odp = '\0';		/* Mark the new end */
	    }
	    t = rt;			/* Save for next time */
	    return(size);
	}
    }					/* Otherwise, keep filling. */
    size = dp - data;			/* End of file */
    *dp = '\0';				/* Mark the end of the data. */
    return(size);		     /* Return partially filled last packet. */
}
#endif /* CKTUNING */

VOID
dofilcrc(c) int c; {			/* Accumulate file crc */
    long z;
    z = crc16 ^ (long)c;
    crc16 = (crc16 >> 8) ^
      (crcta[(z & 0xF0) >> 4] ^ crctb[z & 0x0F]);
}

/* For SENDing from an array... */

int
agnbyte() {				/* Get next byte from array */
#ifndef NOSPL
    char c;
    static int save = 0;		/* For CRLF */
    static char ** ap = NULL;		/* Array pointer */
    static char * p = NULL;		/* Character pointer */
    static int i = 0, n = 0;		/* Array index and limit */
    extern int a_dim[];			/* Array dimension */

    if (!ap) {				/* First time thru */
	ap = sndarray;			/* Set up array pointers */
	if (!ap || (i = sndxlo) > a_dim[sndxin]) {
	    sndarray = NULL;
	    ap = NULL;
	    return(-1);
	}
	p = ap[i];			/* Point to first element in range */
	n = sndxhi;			/* Index of last element in range */
	if (sndxhi > a_dim[sndxin])	/* Adjust if necessary */
	  n = a_dim[sndxin];
    }
    if (save) {				/* If anything saved */
	c = save;			/* unsave it */
	save = 0;			/* and return it */
	return(c & 0xff);
    }
    if (i > n) {			/* No more elements */
	sndarray = NULL;
	ap = NULL;
	return(-1);
    }
    if (!p)				/* Source pointer is NULL */
      c = NUL;				/* this means an empty line */
    else				/* Source pointer not NULL */
      c = *p++;				/* Next char */
    if (!c) {				/* Char is empty? */
	if (!binary) {			/* Text: end of line. */
	    if (feol) {			/* Supply NL */
		c = feol;
	    } else {			/* or CRLF */
		save = LF;
		c = CR;
	    }
	    p = ap[++i];
	    return(c & 0xff);
	}
	while (i++ < n) {		/* Binary - get next element */
	    p = ap[i];
	    if (!p)			/* Empty line? */
	      continue;			/* Ignore it and get another */
	    c = *p++;			/* Get next char */
	    if (!c)			/* Emtpy char? */
	      continue;			/* Ignore it and get another */
	    return(c & 0xff);		/* Not empty - return it */
	}
	sndarray = NULL;
	ap = NULL;
	return(-1);			/* Done */
    }
    return(c & 0xff);			/* Char is not empty */
#else
    sndarray = NULL;
    return(-1);
#endif /* NOSPL */
}
#endif /* NOXFER */

#ifndef NOCSETS
static CHAR xlabuf[32] = { 0, 0, 0, 0, 0, 0, 0, 0 };
static int xlacount = 0;
static int xlaptr = 0;
/* static USHORT lastucs2 = 0; */

/*
  X G N B Y T E --  Get next translated byte from the input file.

  Returns the next byte that is to be put into the packet, already translated.
  This isolates getpkt() from having to know anything about translation,
  single- vs multibyte character sets, one-to-many vs many-to-one, etc, but it
  has rather high overhead, so don't call it unless you know translation is
  needed to or from Unicode, Japanese, or other multibyte character set.

  Call with:
    fcs:  File character set (source, file we are reading from)
    tcs:  Target character set (use an FC_xxx code, not a TC_xxx code)
  Returns:
    >= 0: A translated byte suitable for writing.
    <  0: Fatal error (such as EOF on input source).
  As of Sat Sep  7 18:37:41 2002:  
    When the output character-set is UCS-2, bytes are ALWAYS returned in
    big-endian order (previously they could also be returned in LE order
    under certain conditions, which was just way too confusing).
*/
int
#ifdef CK_ANSIC
xgnbyte(int tcs, int fcs, int (*fn)(void))
#else /* CK_ANSIC */
xgnbyte(tcs,fcs,fn) int tcs, fcs, (*fn)();
#endif /* CK_ANSIC */
/* xgnbyte */ {
    _PROTOTYP( int (*xx), (USHORT) ) = NULL;
    int haveuc = 0;			/* Flag for have Unicode character */
#ifdef KANJI
    int havesj = 0;			/* Have Shift-JIS character */
    int haveeu = 0;			/* Have EUC-JP character */
#endif /* KANJI */
    int rc = -1, x = 0, flag = 0;
    int utferror = 0;
    int eolflag = 0;
    unsigned int xc, thischar;
    static int swapping = 0;
    CHAR rt;
    /* USHORT ch; */
#ifdef UNICODE
    union ck_short uc;
#endif /* UNICODE */
#ifdef KANJI
    union ck_short sj, eu;		/* Shift-JIS character */
#endif /* KANJI */

#ifdef KANJI
    sj.x_short = 0;
#endif /* KANJI */

#ifdef DEBUG
    if (deblog && !ffc) {
	debug(F101,"xgnbyte initial swap","",swapping);
    }
#endif /* DEBUG */

    if (xlacount-- > 0) {		/* We already have some */
	x = xlabuf[xlaptr++];
	debug(F001,"xgnbyte from buf","",x);
	return(x);
    }
    if (xlatype != XLA_NONE) {		/* Not not translating... */
	haveuc = 0;
#ifdef UNICODE
	if (fcs == FC_UCS2) {		/* UCS-2: Read two bytes */
	    if (!ffc)			/* Beginning of file? */
	      swapping = 0;		/* Reset byte-swapping flag */
	    uc.x_short = 0;
	  bomskip:
	    x = fn ? (*fn)() : zminchar(); /* Get first byte */
	    debug(F001,"zminchar swapping","",swapping);
	    debug(F001,"zminchar C0","",x);
	    flag = 1;			/* Remember we called zminchar() */
	    if (x > -1) {		/* Didn't fail */
		ffc++;			/* Count a file byte */
		uc.x_char[swapping] = x & 0xff;
#ifndef NOXFER
		if (docrc && (what & W_SEND))
		  dofilcrc(x);
#endif /* NOXFER */
		x = fn ? (*fn)() : zminchar(); /* Get second byte */
		if (x > -1) {		/* If didn't fail */
		    debug(F001,"zminchar C1","",x);
		    ffc++;		/* count another file byte */
		    uc.x_char[1-swapping] = x & 0xff;
		    haveuc = 1;		/* And remember we have Unicode */
#ifndef NOXFER
		    if (docrc && (what & W_SEND))
		      dofilcrc(x);
#endif /* NOXFER */
		    if (ffc == (CK_OFF_T)2) { /* Second char of file */
			debug(F001,"xgnbyte 1st UCS2","",uc.x_short);
			debug(F111,"xgnbyte fileorder","A",fileorder);
			if (fileorder < 0) /* Byte order of this file */
			  fileorder = ucsorder;	/* Default is ucsorder */
			if (fileorder > 1)
			  fileorder = 1;
			debug(F111,"xgnbyte fileorder","B",fileorder);
			if (uc.x_short == (USHORT)0xfeff) {
			    swapping = 0;
			    debug(F101,
				  "xgnbyte UCS2 goodbom swap","",swapping);
			    fileorder = byteorder; /* Note: NOT 0 */
			    goto bomskip;
			} else if (uc.x_short == (USHORT)0xfffe) {
			    swapping = 1;
			    debug(F101,
				  "xgnbyte UCS2 badbom swap","",swapping);
			    fileorder = (1 - byteorder); /* Note: NOT 1 */
			    goto bomskip;
			} else if ((byteorder && !fileorder) || /* No BOM */
				   (!byteorder && fileorder > 0)) {
			    /* fileorder might have been set by scanfile() */
			    CHAR c;
			    c = uc.x_char[0];
			    uc.x_char[0] = uc.x_char[1];
			    uc.x_char[1] = c;
			    swapping = 1;
			    debug(F111,"xgnbyte UCS2 noBOM swap","A",swapping);
			} else {
			    swapping = 0;
			    debug(F111,"xgnbyte UCS2 noBOM swap","B",swapping);
			}
			debug(F111,"xgnbyte fileorder","C",fileorder);
		    }
		} else
		  return(x);
	    } else
	      return(x);
	    debug(F001,"xgnbyte UCS2","",uc.x_short);

	} else if (fcs == FC_UTF8) {	/* File is UTF-8 */
	    CHAR ch = 0;		/* Data types needed for API... */
	    USHORT * us = NULL;
	    uc.x_short = 0;
	    flag = 1;			/* We (will) have called zminchar() */
	    /* Read source bytes */
	    while ((x = fn ? (*fn)() : zminchar()) > -1) {
		ffc++;			/* Got a byte - count it */
#ifndef NOXFER
		if (docrc && (what & W_SEND))
		  dofilcrc(x);
#endif /* NOXFER */
		ch = x;
		rc = utf8_to_ucs2(ch,&us); /* Convert to UCS-2 */
		if (rc == 0) {		/* Done */
		    uc.x_short = *us;
		    haveuc = 1;
		    break;
		} else if (rc < 0) {	/* Error */
		    utferror = 1;
		    debug(F101,"xgnbyte UTF-8 input error","",rc);
		    haveuc = 1;
		    uc.x_short = *us;
		    break;
		}
	    }
	    if (x < 0)
	      return(x);
	    debug(F001,"xgnbyte UTF8->UCS2","",uc.x_short);
	}
#endif /* UNICODE */

#ifdef KANJI
#ifdef UNICODE
	else
#endif /* UNICODE */
	  if (fcsinfo[fcs].alphabet == AL_JAPAN) { /* Japanese source file */
	    int c7, x, y;
	    if (fcs == FC_JIS7) {	/* If file charset is JIS-7 */
		if (!ffc)		/* If first byte of file */
		  j7init();		/* Initialize JIS-7 parser */
		x = getj7();		/* Get a JIS-7 byte */
	    } else			/* Otherwise */
	      x = fn ? (*fn)() : zminchar(); /* Just get byte */
	    if (x < 0) {		/* Propogate EOF or error */
		debug(F100,"XGNBYTE EOF","",0);
		return(x);
	    }
	    debug(F001,"XGNBYTE x","",x);
	    ffc++;			/* Count */
#ifndef NOXFER
	    if (docrc && (what & W_SEND)) dofilcrc(x); /* Do CRC */
#endif /* NOXFER */
	    switch (fcs) {		/* What next depends on charset */
	      case FC_SHJIS:		/* Shift-JIS */
		if ((x <= 0x80) ||	/* Any 7-bit char... */
		    (x >= 0xa0 && x <= 0xdf)) { /* or halfwidth Katakana */
		    sj.x_short = (USHORT) x;    /* we read one byte. */
		} else {		/* Anything else */
		    if ((y = fn ? (*fn)() : zminchar()) < 0) /* get another */
		      return(y);
#ifndef NOXFER
		    if (docrc && (what & W_SEND)) dofilcrc(y);
#endif /* NOXFER */
		    ffc++;
		    sj.x_char[byteorder] = (CHAR) x;
		    sj.x_char[1-byteorder] = (CHAR) y;
		}
		break;

	      case FC_JIS7:		/* JIS-7 */
	      case FC_JDEC:		/* DEC Kanji */
	      case FC_JEUC:		/* EUC-JP */
		if ((x & 0x80) == 0) {	/* Convert to Shift-JIS */
		    sj.x_short = (USHORT) x; /* C0 or G0: one byte */
		    eu.x_short = (USHORT) x;
		    haveeu = 1;
		} else {
		    c7 = x & 0x7f;
		    if (c7 > 0x20 && c7 < 0x7f) { /* Kanji: two bytes */
			if ((y = (fcs == FC_JEUC) ?
			     (fn ? (*fn)() : zminchar()) :
			     getj7()	/* ^^^ */
			     ) < 0)
			  return(y);
			ffc++;
#ifndef NOXFER
			if (docrc && (what & W_SEND)) dofilcrc(y);
#endif /* NOXFER */
			eu.x_char[byteorder] = (CHAR) x;
			eu.x_char[1-byteorder] = (CHAR) y;
			sj.x_short = eu_to_sj(eu.x_short);
			haveeu = 1;
		    } else if (x == 0x8e) { /* SS2 Katakana prefix: 2 bytes */
			if ((y = (fcs == FC_JIS7) ?
			     getj7() :	/* ^^^ */
			     (fn ? (*fn)() : zminchar())
			     ) < 0)
			  return(y);
			ffc++;
#ifndef NOXFER
			if (docrc && (what & W_SEND)) dofilcrc(y);
#endif /* NOXFER */
			sj.x_short = y | 0x80;
			debug(F001,"XGNBYTE KANA SJ","",sj.x_short);
		    } else {
			/* Something that translates to U+FFFD */
			sj.x_short = UNKSJIS;
		    }
		}
		break;
	    }
	    havesj = 1;			/* Have Shift-JIS */
#ifdef UNICODE
	    uc.x_short = sj_to_un(sj.x_short); /* Translate to UCS-2 */
	    haveuc = 1;			/* Have Unicode */
#endif /* UNICODE */
	    flag = 1;			/* Have a char */
	}
#endif /* KANJI */
    }
    if (!flag) {			/* If no character was read yet... */
	if ((x = (fn ? (*fn)() : zminchar())) > -1)	/* read one now */
	  ffc++;
	debug(F101,"xgnbyte zminchar 1","",x);
	if (x < 0)
	  return(x);
	haveuc = 0;
    }
#ifdef UNICODE
    if (haveuc) {
	thischar = uc.x_short;
	/* lastucs2 = uc.x_short; */
    } else
#endif /* UNICODE */
      thischar = x;
    debug(F001,"xgnbyte thischar",haveuc ? "[UNICODE]" : "[other]",thischar);

#ifdef CK_CTRLZ				/* SET EOF CTRLZ */
    if (eofmethod == XYEOF_Z && !binary && thischar == 26) {
	debug(F100,"xgnbyte EOF on Ctrl-Z 1","",0);
	return(-1);
    }
#endif /* CK_CTRLZ */

#ifdef UNICODE
    if (!haveuc)			/* If not Unicode... */
#endif /* UNICODE */
      x &= fmask;			/* Apply SET FILE BYTESIZE mask */

    switch (xlatype) {			/* Translation type... */
#ifdef UNICODE
      case XLA_UNICODE: {		/* Unicode is involved */
	  xc = 0;
/*
  Here we must choose the appropriate translation function.  If we are being
  called by getpkt() (i.e. when transferring a file), we are translating from
  Unicode to the Transfer Character Set and therefore must use the function
  pointed to by xut.  Otherwise, e.g. during TRANSLATE, CONNECT, TRANSMIT, etc,
  we are translating from Unicode to the File Character Set and so must call
  the function pointed to by xuf.  There might be a cleaner way to set this
  up but I don't think so.  For example, setxlatype() might have been called
  too soon and so might not have known whether it was a file transfer or a
  local operation.
*/
/*
  (Many years later...) In testing this code I noticed that TRANSLATE'ing
  Russian text from UTF-8 to ISO Latin/Cyrillic produced all question marks.
  Rereading the previous paragraph it seems to me we are (I am) overloading
  this function with responsibilites, satisfying the needs of file transfer
  (local file charset -> transfer charset for outbound packet) and local file
  conversion.  In the case of TRANSLATE, we call (xgnbyte(), xpnbyte()) in a
  loop, expecting the xgnbyte() will feed UCS2 to xpnbyte().  But the
  following code does what xpnbyte() is going to do, returning (in this case)
  an ISO Latin/Cyrillic byte stream, which xpnbyte() believes to be UCS2, and
  comes up with nonsense.  Not wanting to rip the whole thing apart and start
  over, I made the following change that should do no harm, upon observing
  that if the input character set is UTF-8 or UCS-2, then when we get here it
  has already been converted to UCS2, so if we are not transferring a file, we
  don't need to do anything else except put the bytes in the right place to be
  returned, which is done further along.
*/
#ifdef COMMENT
	  /* Previous code */
	  xx = (what & W_SEND) ? xut : xuf;
#else
	  /* New code 2011-06-03 */
	  if (what & W_SEND) {
	      xx = xut;
	  } else {
	      if (fcs == FC_UCS2 || fcs == FC_UTF8)
		xx = NULL;
	      else
		xx = xuf;
	  }
#endif	/* COMMENT */

	  eolflag = 0;
	  if (haveuc) {			/* File is Unicode */
	      /* See Unicode TR13, "Converting to Other Character Sets" */
	      if (uc.x_short == 0x2028 || /* Line Separator? */
		  uc.x_short == 0x2029 || /* Paragraph Separator */
		  (feol && (uc.x_short == (USHORT)feol))
		  ) {
		  debug(F001,"xgnbyte uc eol","",uc.x_short);
		  rc = 0;
		  eolflag = 1;		/* Don't translate and handle later */
	      }
	      if (xx && !eolflag) {	/* UCS-to-TCS function (UCS->byte) */
		  rc = (*xx)(uc.x_short); /* These can fail... */
		  debug(F101,"xgnbyte xx rc","",rc);
		  if (rc < 0)		/* If it can't be translated */
		    uc.x_short = UNK;	/* Put unknown-character symbol */
		  else
		    uc.x_short = (unsigned)((unsigned)rc & 0xffff);
		  debug(F101,"xgnbyte xx uc","",uc.x_short);
	      }
#ifdef KANJI
	      if (tcs == FC_JEUC) {	/* Translating to EUC-JP */
		  USHORT sj = 0;
		  union ck_short eu;
		  debug(F001,"xgnbyte UCS->EUC UCS","",uc.x_short);
		  if (!havesj)		/* If we don't already have it */
		    sj = un_to_sj(uc.x_short); /* convert to Shift-JIS */
		  eu.x_short = sj_to_eu(sj);
		  debug(F001,"xgnbyte UCS->EUC EUC","",eu.x_short);
		  xlaptr = 0;
		  xlacount = 0;
		  if (eolflag) {
		      if (what & W_SEND) {
			  xlabuf[xlacount++] = LF;
			  return(CR);
		      } else {
			  return(feol);
		      }
		  }
		  if (eu.x_char[byteorder]) {	/* Two bytes */
		      rc = eu.x_char[byteorder];
		      xlabuf[xlacount++] = eu.x_char[1-byteorder];
		      debug(F001,"xgnbyte UCS->EUC xlabuf[0]","",xlabuf[0]);
		  } else {		/* One byte */
		      rc = eu.x_char[1-byteorder];
		  }
		  debug(F101,"xgnbyte UCS->EUC xlacount","",xlacount);
		  debug(F001,"xgnbyte UCS->EUC rc","",rc);
		  return(rc);
	      } else
#endif /* KANJI */
	      if (tcs != FC_UCS2 && tcs != FC_UTF8) {
		  if (uc.x_short & 0xff00) {	/* Decoding error */
		      debug(F001,"xgnbyte decoding error","",uc.x_short);
		      return(-2);
		  } else
		    return((unsigned int)(uc.x_short & 0xff));
	      }
	      xc = uc.x_short;

	  } else {			/* File is not Unicode */
	      USHORT ch;
	      /* Translate from single FCS byte to UCS-2 */
/*
  This is a bit nonobvious...  The blah_u() (Blah-to-Unicode) routines are
  called only with pieces of character sets, in the ISO 2022 sense.  So,
  for example, if ch is a Latin-1 character, we call the translation
  routine only if it is in the right half; if it's in the left half, it
  isn't translated, and in fact will give the wrong result if sent to the
  translation function.  That's because those functions were designed for
  use with the ISO 2022 G0..G3 sets, not for file transfer.  On the other
  hand, if it's a 7-bit character set, we *do* call the translation
  function.  (To put it another way, the left half of any 8-bit character
  set is ASCII and therefore doesn't need to be translated but 7-bit sets
  such as ISO 646 German do need translation).
*/
	      ch = (unsigned)(thischar & 0xff);
	      if (((fcsinfo[fcs].size > 128) && (ch & 0x80)) ||
		  fcsinfo[fcs].size <= 128) {
		  if (xfu) {		 /* FCS-to-UCS function */
		      ch = (*xfu)(ch);
		  }
	      }
	      xc = ch;
	  }
	  /* At this point we have a UCS-2 character in native format */
	  /* (Big Endian or Little Endian) in xc, which is an unsigned int. */

	  debug(F001,"xgnbyte xc","",xc);

	  if (tcs == FC_UTF8) {		/* Now convert to UTF-8 */
	      USHORT c;			/* NOTE: this is FC_UTF8 on purpose! */
	      CHAR * buf = NULL;
	      int i, k = 0, x;

	      xlaptr = 0;
	      if (utferror) {
		  xlabuf[k++] = 0xff;
		  xlabuf[k++] = 0xbd;
	      }
	      if (eolflag) {		/* We detected EOL in source file */
		  if (what & W_SEND) {	/* Convert to CRLF */
		      xlabuf[k++] = LF;
		      xlacount = k;
		      return((unsigned int)CR);
#ifdef COMMENT
		  } else {		/* Or to local line-end */
		      xlacount = k;
		      return((unsigned int)feol);
#endif /* COMMENT */
		  }
	      }
	      c = xc;
	      if ((x = ucs2_to_utf8(c,&buf)) < 1) {
		  debug(F101,"xgnbyte ucs2_to_utf8 error","",c);
		  return(-2);
	      }
	      debug(F101,"xgnbyte UTF8 buf[0]","",buf[0]);
	      for (i = 1; i < x; i++) {
		  xlabuf[k+i-1] = buf[i];
		  debug(F111,"xgnbyte UTF8 xlabuf",ckitoa(i-1),buf[i]);
	      }
	      xlaptr = 0;
	      xlacount = x - 1;
	      debug(F101,"xgnbyte UTF8 xlacount","",xlacount);
	      return((unsigned int)buf[0]);
	  } else {			/* Or keep it as UCS-2 */
	      int k = 0;
	      CHAR c;
	      xlaptr = 0;
	      if (utferror) {
		  xlabuf[k++] = 0xff;
		  xlabuf[k++] = 0xfd;
		  debug(F101,"xgnbyte error","",k);
	      }
	      if (eolflag) {		/* We detected EOL in source file */
		  if (what & W_SEND) {	/* Convert to CRLF */
		      xlabuf[k++] = CR;
		      xlabuf[k++] = NUL;
		      xlabuf[k++] = LF;
		      xlacount = k;
		      debug(F101,"xgnbyte send CRLF","",k);
		      return(0);	/* Return NUL */
		  } else {		/* Or to local line-end */
#ifdef COMMENT
		      /* This bypasses byte swapping that we might need */
		      xlabuf[k++] = (CHAR)feol;
		      xlacount = k;
		      debug(F101,"xgnbyte send feol","",k);
		      return(0);	/* Return NUL */
#else
		      xc = (CHAR)feol;
#endif /* COMMENT */
		  }
	      }
	      /* In which order should we return the bytes? */
#ifdef COMMENT
	      if ( (what & W_SEND) || (what & W_FTP) || (fileorder == 0)) {
#endif /* COMMENT */
		  /* ALWAYS RETURN IN BIG ENDIAN ORDER... 7 Sep 2002   */
		  /* xgnbyte() is almost always used to feed xpnbyte() */
		  /* which requires bytes in BE order. In cases where  */
		  /* xgnbyte is used in isolation, the caller can swap */
		  /* bytes itself afterwards. */
		  xlabuf[k++] = (xc >> 8) & 0xff; /* Big Endian */
		  xlabuf[k++] = xc & 0xff;
		  debug(F001,"xgnbyte->UCS2BE",
			ckitox((int)xlabuf[0]),xlabuf[1]);
#ifdef COMMENT
	      } else {			/* Little Endian */
		  xlabuf[k++] = xc & 0xff;
		  xlabuf[k++] = (xc >> 8) & 0xff;
		  debug(F001,"xgnbyte->UCS2LE",
			ckitox((int)xlabuf[0]),xlabuf[1]);
	      }
#endif /* COMMENT */
	      c = xlabuf[0];
	      xlaptr = 1;
	      xlacount = k-1;
	      debug(F101,"xgnbyte c","",c);
	      debug(F101,"xgnbyte xlaptr","",xlaptr);
	      debug(F011,"xgnbyte xlabuf",xlabuf,xlacount);
	      return((unsigned int)c);
	  }
      }
#endif /* UNICODE */
      case XLA_NONE:
	return((fn ? (*fn)() : zminchar()));
      case XLA_BYTE:			/* Byte-for-Byte translation */
	rt = x;
	if (sx)
	  rt = (*sx)(rt);
#ifdef UNICODE
	if (utferror) {
	    xlaptr = 0;
	    xlacount = 1;
	    xlabuf[0] = rt;
	    return(UNK);
	} else
#endif /* UNICODE */
	  return((unsigned int)rt);

#ifdef KANJI
      case XLA_JAPAN:			/* Come here with Shift-JIS */
	if (tcs == FC_JEUC) {		/* It better be... */
	    xlaptr = 0;
	    xlacount = 0;
	    if (!havesj) {
		printf("BAD BAD\n");
		return(-2);
	    }
	    if (!haveeu)		/* We might already have EUC too */
	      eu.x_short = sj_to_eu(sj.x_short);
	    if (eu.x_char[byteorder]) {
		xlabuf[xlacount++] = eu.x_char[1-byteorder];
		return(eu.x_char[byteorder]);
	    } else {
		return(eu.x_char[1-byteorder]);
	    }
	    break;
	}
#endif /* KANJI */

      default:
	debug(F101,"xgnbyte bad xlatype","",xlatype);
	return(-2);
    }
#ifdef COMMENT
/*    
  If there is a return() statement here, some compilers complain
  about "statement not reached".  If there is no return() statement,
  other compilers complain that "Non-void function should return a value".
  There is no path through this function that falls through to here.
*/
    debug(F100,"xgnbyte switch failure","",0);
    return(-2);
#endif /* COMMENT */
}
#endif /* NOCSETS */

#ifndef NOXFER

/*  G E T P K T  --  Fill a packet data field from the indicated source.  */

/*
  Parameters:
    bufmax: Maximum length of entire packet.
    xlate:  Flag for whether to translate charsets when in text mode.
  Returns:  Number of characters written to packet data field, 0 or more,
            Or -1 on failure (internal error),
            or -3 on timeout (e.g. when reading from a pipe).

  This is the full version allowing for parity and text-mode conversions;
  i.e. it works in all cases.   Also see bgetpkt(), a special lean/mean/fast
  packet encoder that works only for binary-mode no-parity transfers.
*/
static int uflag = 0;

int
getpkt(bufmax,xlate) int bufmax, xlate; { /* Fill one packet buffer */
    register CHAR rt = t, rnext = NUL;	  /* Register shadows of the globals */
    register CHAR *dp, *odp, *odp2, *p1, *p2; /* pointers... */
    register int x;			/* Loop index. */
    register int a7;			/* Low 7 bits of character */

    CHAR xxls, xxdl, xxrc, xxss, xxcq;	/* Pieces of prefixed sequence */

    if (binary) xlate = 0;		/* We don't translate if binary */

    if (!data) {
	debug(F100,"SERIOUS ERROR: getpkt data == NULL","",0);
	return(-1);
    }
    dp = data;				/* Point to packet data buffer */
    size = 0;				/* And initialize its size */
/*
  Assume bufmax is the receiver's total receive-packet buffer length.
  Our whole packet has to fit into it, so we adjust the data field length.
  We also decide optimally whether it is better to use a short-format or
  long-format packet when we're near the borderline.
*/
    bufmax = maxdata();			/* Get maximum data length */

    if (first == 1) {			/* If first character of this file.. */
#ifdef UNICODE
	/* Special end-of-line handling for Unicode */
	if (tcharset == TC_UCS2 || tcharset == TC_UTF8)
	  uflag = 1;
#endif /* UNICODE */
	debug(F101,"getpkt first uflag","",uflag);
	debug(F101,"getpkt first rt","",rt);
	if (!memstr && !funcstr)	/* and real file... */
	  ffc = (CK_OFF_T)0;		/* reset file character counter */
#ifdef COMMENT
	/* Moved to below... */
	first = 0;			/* Next character won't be first */
#endif /* COMMENT */
	*leftover = '\0';		/* Discard any interrupted leftovers */
	nleft = 0;
#ifndef NOCSETS
	setxlatype(tcharset,fcharset);	/* Set up charset translations */
#endif /* NOCSETS */

	/* Get first character of file into rt, watching out for null file */

#ifdef CALIBRATE
	if (calibrate && !memstr) {
#ifdef NORANDOM
	    x = rt = 53;
#else
	    x = rt = cal_a[rand() & 0xff];
#endif /* NORANDOM */
	    first = 0;
	    ffc++;
	} else
#endif /* CALIBRATE */
#ifdef KANJI
	if (xlate && tcharset == TC_JEUC) { /* Kanji text */
	    x = zkanjf();
	    if ((x = zkanji(memstr ? kgetm : kgetf)) < 0) {
	        first = -1;
	        size = 0;
	        if (x == -2) {
	            debug(F100,"getpkt zkanji: input error","",0);
	            cxseen = 1;
	        } else debug(F100,"getpkt zkanji: empty string/file","",0);
	        return(0);
	    }
	    rt = x;
	    first = 0;
	    if (!memstr) {
		ffc++;
		if (docrc && (what & W_SEND)) /* Accumulate file crc */
		  dofilcrc((int)rt);
	    }
	} else {			/* Not Kanji text */
#endif /* KANJI */
	    if (memstr) {		/* Reading data from memory string */
		/* This will not be Unicode */
		if ((rt = *memptr++) == '\0') { /* end of string ==> EOF */
		    first = -1;
		    size = 0;
		    debug(F100,"getpkt: empty string","",0);
		    return(0);
		}
		first = 0;
	    } else if (funcstr) {	/* Reading data from a function */
		/* This will not be Unicode */
		if ((x = (*funcptr)()) < 0) { /* End of input  */
		    first = -1;
		    size = 0;		/* Empty */
		    return(0);
		}
		ffc++;			/* Count a file character */
		rt = (CHAR) x;		/* Convert int to char */
		first = 0;
		debug(F000,"getpkt funcstr","",rt);

	    } else {			/* Reading data from a file */
#ifndef NOCSETS
		if (xlate && !binary) {	/* Could be Unicode */
		    if (xlatype == XLA_UNICODE) {
			/* Get next translated byte */
			x = xgnbyte(cseqtab[tcharset],fcharset,NULL);
			debug(F101,"getpkt xgnbyte","",x);
		    } else {		/* Not Unicode */
			x = zminchar();	/* Get next byte, translate below */
			debug(F101,"getpkt zminchar A","",x);
		    }
		} else {		/* Just get next byte */
#endif /* NOCSETS */
		    x = zminchar();
		    debug(F101,"getpkt zminchar B","",x);
#ifndef NOCSETS
		}
#endif /* NOCSETS */
		if (x < 0) {		/* End of file or input error */
		    if (x == -3) {	/* Timeout. */
			size = (dp-data);
			debug(F101,"getpkt timeout size","",size);
			return((size == 0) ? x : size);
		    }
		    first = -1;
		    size = 0;
		    if (x == -2) {	/* Error */
			debug(F100,"getpkt: input error","",0);
			cxseen = 1;	/* Interrupt the file transfer */
		    } else {
			debug(F100,"getpkt empty file","",0);
		    }
		    return(0);
		}
		first = 0;		/* Next character won't be first */
		rt = (CHAR) x;		/* Convert int to char */
#ifndef NOCSETS
		if (xlatype != XLA_UNICODE || binary) {
		    ffc++;
		    if (sx)
		      rt = (*sx)(rt);
		    if (docrc && (what & W_SEND))
		      dofilcrc(x);
		}
#endif /* NOCSETS */
#ifdef DEBUG
		if (deblog)
		  debug(F101,"getpkt 1st char","",rt);
#endif /* DEBUG */
		if (/* !haveuc && */ docrc && (what & W_SEND)) /* File CRC */
		  dofilcrc(x);
	    }
#ifdef KANJI
	}
#endif /* KANJI */
	/* PWP: handling of feol is done later (in the while loop)... */

    } else if ((first == -1) && (nleft == 0)) { /* EOF from last time */
#ifdef DEBUG
	if (deblog) {
	    debug(F101,"getpkt eof crc16","",crc16);
	    debug(F101,"getpkt eof ffc","",ffc);
	}
#endif /* DEBUG */
        return(size = 0);
    }
/*
  Here we handle characters that were encoded for the last packet but
  did not fit, and so were saved in the "leftover" array.
*/
    debug(F101,"getpkt nleft","",nleft);
    if (nleft) {
	for (p1 = leftover; nleft > 0; nleft--) /* Copy leftovers */
	  *dp++ = *p1++;
	*leftover = '\0';			/* Delete leftovers */
	nleft = 0;
    }
    if (first == -1)			/* Handle EOF */
      return(size = (dp - data));

/* Now fill up the rest of the packet. */

    rpt = 0;				/* Initialize character repeat count */

    while (first > -1) {		/* Until EOF... */
#ifdef CALIBRATE
	if (calibrate && !memstr) {	/* We generate our own "file" */
	    if (ffc >= calibrate) {	/* EOF */
		first = -1;
		ffc--;
	    } else {			/* Generate next character */
		if (cal_j > CAL_M * ffc)
		  cal_j = cal_a[ffc & 0xff];
		x = (unsigned)cal_a[(cal_j & 0xff)];
		if (x == rt) x ^= 2;
	    }
	    cal_j += (unsigned int)(ffc + CAL_O);
	    ffc++;
	} else
#endif /* CALIBRATE */
#ifdef KANJI
	  if (xlate && tcharset == TC_JEUC) {
	      if ((x = zkanji(memstr ? kgetm : kgetf)) < 0) {
		  first = -1;
		  if (x == -2) cxseen = 1;
	      } else if (!memstr) ffc++;
	      rnext = (CHAR) (x & fmask);
	  } else {
#endif /* KANJI */
	    if (memstr) {		/* Get next char from memory string */
		if ((x = *memptr++) == '\0') /* End of string means EOF */
		  first = -1;		/* Flag EOF for next time. */
		rnext = (CHAR) (x & fmask); /* Apply file mask */
	    } else if (funcstr) {	/* Get next char from function */
		if ((x = (*funcptr)()) < 0) /* End of string means EOF */
		  first = -1;		/* Flag EOF for next time. */
		rnext = (CHAR) (x & fmask); /* Apply file mask */
	    } else {			/* From file... */
#ifndef NOCSETS
		if (xlate && !binary) {	/* Could be Unicode */
		    if (xlatype == XLA_UNICODE) {
			/* Get next translated byte */
			x = xgnbyte(cseqtab[tcharset],fcharset,NULL);
		    } else {		/* Not Unicode */
			x = zminchar(); /* Get next byte, translate below */
			/* debug(F101,"xgnbyte B zminchar","",x); */
		    }
		} else {		/* Just get next byte */
#endif /* NOCSETS */
		    x = zminchar();
		    /* debug(F101,"xgnbyte C zminchar","",x); */
#ifndef NOCSETS
		}
#endif /* NOCSETS */
		if (x < 0) {		/* Check for EOF */
		    if (x == -3) {	/* Timeout reading from pipe */
			t = rt;
			size = (dp-data);
			debug(F101,"getpkt timeout size","",size);
			return((size == 0) ? x : size);
		    }
		    first = -1;		/* Flag eof for next time. */
		    if (x == -2) cxseen = 1; /* If error, cancel this file. */
		}
		rnext = (CHAR) (x & fmask); /* Apply file mask */
#ifndef NOCSETS
		if (xlatype != XLA_UNICODE) {
#endif /* NOCSETS */
		    ffc++;
#ifndef NOCSETS
		    if (sx)
		      rt = (*sx)(rt);
#endif /* NOCSETS */
		    if (docrc && (what & W_SEND))
		      dofilcrc(x);

#ifndef NOCSETS
		}
#endif /* NOCSETS */
	    }
#ifdef KANJI
	}
#endif /* KANJI */
/*
  At this point, the character we just read is in rnext,
  and the character we are about to encode into the packet is in rt.
*/
	odp = dp;			/* Remember where we started. */
 	xxls = xxdl = xxrc = xxss = xxcq = NUL;	/* Clear these. */
/*
  Now encode the character according to the options that are in effect:
    ctlp[]: whether this control character needs prefixing.
    binary: text or binary mode.
    rptflg: repeat counts enabled.
    ebqflg: 8th-bit prefixing enabled.
    lscapu: locking shifts enabled.
*/
	if (rptflg) {			/* Repeat processing is on? */
	    if (!uflag &&
		/*
		 * If the next char is really CRLF, then we cannot
		 * be doing a repeat (unless CR,CR,LF which becomes
		 * "~ <n-1> CR CR LF", which is OK but not most efficient).
		 * I just plain don't worry about this case.  The actual
		 * conversion from NL to CRLF is done after the rptflg if...
		 */
		(!feol || binary || (feol && (rnext != feol))) &&
		(rt == rnext) && (first == 0)) { /* Got a run... */
		if (++rpt < 94) {	/* Below max, just count */
		    continue;		/* go back and get another */
		} else if (rpt == 94) {	/* Reached max, must dump */
		    xxrc = (CHAR) tochar(rpt); /* Put the repeat count here */
		    rptn += rpt;	/* Accumulate it for statistics */
		    rpt = 0;		/* And reset it */
		}
	    } else if (rpt > 1) {	/* More than two */
		xxrc = (CHAR) tochar(++rpt); /* and count. */
		rptn += rpt;
		rpt = 0;		/* Reset repeat counter. */
	    }
	    /*
	      If (rpt == 1) we must encode exactly two characters.
	      This is done later, after the first character is encoded.
	    */
	}
	/* If it's the newline character... */
	if (!uflag && !binary && feol && (rt == feol)) {
	    if (lscapu && lsstate) {	/* If SHIFT-STATE is SHIFTED */
		if (ebqflg) {		/* If single shifts enabled, */
		    *dp++ = (CHAR) ebq;	/* insert a single shift. */
		} else {		/* Otherwise must shift in. */
		    *dp++ = myctlq;	/* Insert shift-out code */
		    *dp++ = 'O';
		    lsstate = 0;	/* Change shift state */
		}
	    }
#ifdef CK_SPEED
	    if (ctlp[CR]) {
		*dp++ = myctlq;		/* Insert carriage return directly */
		*dp++ = 'M';
		ccp++;
	    } else {
		*dp++ = CR;		/* Perhaps literally */
		ccu++;
	    }
#else /* !CK_SPEED */
	    *dp++ = myctlq;		/* Insert carriage return directly */
	    *dp++ = 'M';
	    ccp++;
#endif /* CK_SPEED */
	    rt = LF;			/* Now make next char be linefeed. */
	}
/*
  Now handle the 8th bit of the file character.  If we have an 8-bit
  connection, we preserve the 8th bit.  If we have a 7-bit connection,
  we employ either single or locking shifts (if they are enabled).
*/
	a7 = rt & 0177;			/* Get low 7 bits of character */
	if (rt & 0200) {		/* 8-bit character? */
	    if (lscapu) {		/* Locking shifts enabled? */
		if (!lsstate) {		/* Not currently shifted? */
		    x = lslook(0200);	/* Look ahead */
		    if (x != 0 || ebqflg == 0) { /* Locking shift decision */
			xxls = 'N';	   /* Need locking shift-out */
			lsstate = 1;	   /* and change to shifted state */
		    } else if (ebqflg) {   /* Not worth it */
			xxss = (CHAR) ebq; /* Use single shift */
		    }
		}
		rt = (CHAR) a7;		/* Replace character by 7-bit value */
	    } else if (ebqflg) {	/* 8th bit prefixing is on? */
		xxss = (CHAR) ebq;	/* Insert single shift */
		rt = (CHAR) a7;		/* Replace character by 7-bit value */
	    }
/*
  In case we have a 7-bit connection and this is an 8-bit character, AND
  neither locking shifts nor single shifts are enabled, then the character's
  8th bit will be destroyed in transmission, and a block check error will
  occur.
*/
	} else if (lscapu) {		/* 7-bit character */

	    if (lsstate) {		/* Comes while shifted out? */
		x = lslook(0);		/* Yes, look ahead */
		if (x || ebqflg == 0) {	/* Time to shift in. */
		    xxls = 'O';		/* Set shift-in code */
		    lsstate = 0;	/* Exit shifted state */
		} else if (ebqflg) {	/* Not worth it, stay shifted out */
		    xxss = (CHAR) ebq;	/* Insert single shift */
		}
	    }
	}
	/* If data character is significant to locking shift protocol... */
	if (lscapu && (a7 == SO || a7 == SI || a7 == DLE))
	  xxdl = 'P';			/* Insert datalink escape */

	if (
#ifdef CK_SPEED
	    /*
	      Thwart YET ANOTHER unwanted, unneeded, and unloved sign
	      extension.  This one was particularly nasty because it prevented
	      255 (Telnet IAC) from being prefixed on some platforms -- e.g.
	      VMS with VAX C -- but not others, thus causing file transfers to
	      fail on Telnet connections by sending bare IACs.  Not to mention
	      the stray memory reference.  Signed chars are a BAD idea.
	    */
	    ctlp[(unsigned)(rt & 0xff)]	/* Lop off any "sign" extension */
#else
	    (a7 < SP) || (a7 == DEL)
#endif /* CK_SPEED */
	    ) {				/* Do control prefixing if necessary */
	    xxcq = myctlq;		/* The prefix */
	    ccp++;			/* Count it */
	    rt = (CHAR) ctl(rt);	/* Uncontrollify the character */
	}
#ifdef CK_SPEED
	else if ((a7 < SP) || (a7 == DEL)) /* Count an unprefixed one */
	  ccu++;
#endif /* CK_SPEED */

	if (a7 == myctlq)		/* Always prefix the control prefix */
	  xxcq = myctlq;

	if ((rptflg) && (a7 == rptq))	/* If it's the repeat prefix, */
	  xxcq = myctlq;		/* prefix it if doing repeat counts */

	if ((ebqflg) && (a7 == ebq))	/* Prefix the 8th-bit prefix */
	  xxcq = myctlq;		/* if doing 8th-bit prefixes */

/* Now construct the entire sequence */

	if (xxls) { *dp++ = myctlq; *dp++ = xxls; } /* Locking shift */
	odp2 = dp;				    /* (Save this place) */
	if (xxdl) { *dp++ = myctlq; *dp++ = xxdl; } /* Datalink escape */
	if (xxrc) { *dp++ = (CHAR) rptq; *dp++ = xxrc; } /* Repeat count */
	if (xxss) { *dp++ = (CHAR) ebq; }           /* Single shift */
	if (xxcq) { *dp++ = myctlq; }	    	    /* Control prefix */
	*dp++ = rt;			/* Finally, the character itself */

	if (rpt == 1) {			/* Exactly two copies? */
	    rpt = 0;
	    p2 = dp;			/* Save place temporarily */
	    for (p1 = odp2; p1 < p2; p1++) /* Copy the old chars over again */
	      *dp++ = *p1;
	    if ((p2-data) <= bufmax) odp = p2; /* Check packet bounds */
	    if ((p2-data) < bufmax) odp = p2; /* Check packet bounds */
	}
	rt = rnext;			/* Next character is now current. */

/* Done encoding the character.  Now take care of packet buffer overflow. */

	if ((dp-data) >= bufmax) {	/* If too big, save some for next. */

	    debug(F000,"getpkt EOP","",rt);

	    size = (dp-data);		/* Calculate the size. */
	    *dp = '\0';			/* Mark the end. */
	    if (memstr) {		/* No leftovers for memory strings */
		if (rt)			/* Char we didn't encode yet */
		  memptr--;		/* (for encstr()) */
		return(size);
	    }
	    if ((dp-data) > bufmax) {	/* if packet is overfull */
		/* copy the part that doesn't fit into the leftover buffer, */
		/* taking care not to split a prefixed sequence. */
		int i;
		nleft = dp - odp;
		for (i = 0, p1 = leftover, p2 = odp; i < nleft; i++) {
		    *p1++ = *p2++;
		    if (memstr) memptr--; /* (for encstr) */
		}
		debug(F111,"getpkt leftover",leftover,size);
		debug(F101,"getpkt osize","",(odp-data));
		size = (odp-data);	/* Return truncated packet. */
		*odp = '\0';		/* Mark the new end */
	    }
	    t = rt;			/* Save for next time */
	    return(size);
	}
    }					/* Otherwise, keep filling. */
    size = (dp-data);			/* End of file */
    *dp = '\0';				/* Mark the end of the data. */
    debug(F111,"getpkt eof/eot",data,size); /* Fell thru before packet full, */
    return(size);		     /* return partially filled last packet. */
}

/*  T I N I T  --  Initialize a transaction  */

int epktrcvd = 0, epktsent = 0;

/*
  Call with 1 to reset everything before S/I/Y negotiation, or 0 to
  reset only the things that are not set in the S/I/Y negotiation.
  Returns -1 on failure (e.g. to create packet buffers), 0 on success.
*/
int
tinit(flag) int flag; {
    int x;
#ifdef CK_TIMERS
    extern int rttflg;
#endif /* CK_TIMERS */
    extern int rcvtimo;
    extern int fatalio;

    debug(F101,"tinit flag","",flag);

    *epktmsg = NUL;
    epktrcvd = 0;
    epktsent = 0;
    ofperms = "";
    diractive = 0;			/* DIR / REMOTE DIR not active */
    interrupted = 0;			/* Not interrupted */
    fatalio = 0;			/* No fatal i/o error */
    if (server) {
	moving  = 0;
	pipesend = 0; /* This takes care of multiple GETs sent to a server. */
    }
    bestlen = 0;			/* For packet length optimization */
    maxsend = 0;			/* Biggest data field we can send */
#ifdef STREAMING
    streamok = 0;			/* Streaming negotiated */
    streaming = 0;			/* Streaming being done now */
#endif /* STREAMING */

    binary = b_save;			/* ... */
    gnf_binary = binary;		/* Per-file transfer mode */
    retrans = 0;			/* Packet retransmission count */
    sndtyp = 0;				/* No previous packet */
    xflg = 0;				/* Reset x-packet flag */
    memstr = 0;				/* Reset memory-string flag */
    memptr = NULL;			/*  and buffer pointer */
    funcstr = 0;			/* Reset "read from function" flag */
    funcptr = NULL;			/*  and function pointer */
    autopar = 0;			/* Automatic parity detection flag */

    /* This stuff is only for BEFORE S/I/Y negotiation, not after */

    if (flag) {
	if (bctf) {		      /* Force Block Check 3 on all packets */
	    bctu = bctl = 3;		/* Set block check type to 3 */
	} else {
	    bctu = bctl = 1;		/* Reset block check type to 1 */
	}
	myinit[0] = '\0';		/* Haven't sent init string yet */
	rqf = -1;			/* Reset 8th-bit-quote request flag */
	ebq = MYEBQ;			/* Reset 8th-bit quoting stuff */
	ebqflg = 0;			/* 8th bit quoting not enabled */
	ebqsent = 0;			/* No 8th-bit prefix bid sent yet */
	sq = 'Y';			/* 8th-bit prefix bid I usually send */
	spsiz = spsizr;			/* Initial send-packet size */
	debug(F101,"tinit spsiz","",spsiz);
	wslots = 1;			/* One window slot */
	wslotn = 1;			/* No window negotiated yet */
	justone = 0;			/* (should this be zero'd here?) */
	whoareu[0] = NUL;		/* Partner's system type */
	sysindex = -1;
	wearealike = 0;
	what = W_INIT;			/* Doing nothing so far... */
    }
    fncnv = f_save;			/* Back to what user last said */
    pktnum = 0;				/* Initial packet number to send */
    cxseen = czseen = discard = 0;	/* Reset interrupt flags */
    *filnam = '\0';			/* Clear file name */
    spktl = 0;				/* And its length */
    nakstate = 0;			/* Assume we're not in a NAK state */
    numerrs = 0;			/* Transmission error counter */
    idletmo = 0;			/* No idle timeout yet */
    if (server) { 			/* If acting as server, */
	if (srvidl > 0)			/* If an idle timeout is given */
	  timint = srvidl;
	else
	  timint = srvtim;		/* use server timeout interval. */
    } else {				/* Otherwise */
	timint = chktimo(rtimo,timef);	/* and use local timeout value */
    }
    debug(F101,"tinit timint","",timint);

#ifdef CK_TIMERS
    if (rttflg && timint > 0)		/* Using round-trip timers? */
      rttinit();
    else
#endif /* CK_TIMERS */
      rcvtimo = timint;

    winlo = 0;				/* Packet 0 is at window-low */
    debug(F101,"tinit winlo","",winlo);
    x = mksbuf(1);			/* Make a 1-slot send-packet buffer */
    if (x < 0) return(x);
    x = getsbuf(0);			/* Allocate first send-buffer. */
    debug(F101,"tinit getsbuf","",x);
    if (x < 0) return(x);
    dumpsbuf();
    x = mkrbuf(wslots);			/* & a 1-slot receive-packet buffer. */
    if (x < 0) return(x);
    lsstate = 0;			/* Initialize locking shift state */
    if (autopath) {			/* SET RECEIVE PATHNAMES AUTO fixup */
	fnrpath = PATH_AUTO;
	autopath = 0;
    }
    return(0);
}

VOID
pktinit() {				/* Initialize packet sequence */
    pktnum = 0;				/* number & window low. */
    winlo = 0;
    debug(F101,"pktinit winlo","",winlo);
}

/*  R I N I T  --  Respond to S or I packet  */

VOID
rinit(d) CHAR *d; {
    char *tp = NULL;
    ztime(&tp);
    tlog(F110,"Transaction begins",tp,0L); /* Make transaction log entry */
    tlog(F110,"Global file mode:", binary ? "binary" : "text", 0L);
    tlog(F110,"Collision action:", fncnam[fncact],0);
    tlog(F100,"","",0);
    debug(F101,"rinit fncact","",fncact);
    filcnt = filrej = 0;		/* Init file counters */
    spar(d);
    ack1(rpar());
#ifdef datageneral
    if ((local) && (!quiet))            /* Only do this if local & not quiet */
        consta_mt();                    /* Start the asynch read task */
#endif /* datageneral */
}


/*  R E S E T C  --  Reset per-transaction character counters */

VOID
resetc() {
    rptn = 0;				/* Repeat counts */
    fsecs = flci = flco = (CK_OFF_T)0;	/* File chars in and out */
#ifdef GFTIMER
    fpfsecs = 0.0;
#endif /* GFTIMER */
    tfc = tlci = tlco = (CK_OFF_T)0;	/* Total file, line chars in & out */
    ccu = ccp = 0L;			/* Control-char statistics */
#ifdef COMMENT
    fsize = (CK_OFF_T)-1;		/* File size */
#else
    if (!(what & W_SEND))
      fsize = (CK_OFF_T)-1;
    debug(F101,"resetc fsize","",fsize);
#endif /* COMMENT */
    timeouts = retrans = 0;		/* Timeouts, retransmissions */
    spackets = rpackets = 0;		/* Packet counts out & in */
    crunched = 0;			/* Crunched packets */
    wcur = 0;				/* Current window size */
    wmax = 0;				/* Maximum window size used */
    peakcps = 0;                        /* Peak chars per second */
}

/*  S I N I T  --  Get & verify first file name, then send Send-Init packet */
/*
 Returns:
   1 if send operation begins successfully
   0 if send operation fails
*/
#ifdef DYNAMIC
char *cmargbuf = NULL;
#else
char cmargbuf[CKMAXPATH+1];
#endif /* DYNAMIC */
char *cmargp[2];

VOID
fnlist() {
    if (!calibrate)
      sndsrc = (nfils < 0) ? -1 : nfils; /* Source for filenames */
#ifdef DYNAMIC
    if (!cmargbuf && !(cmargbuf = malloc(CKMAXPATH+1)))
      fatal("fnlist: no memory for cmargbuf");
#endif /* DYNAMIC */
    cmargbuf[0] = NUL;			/* Initialize name buffer */

    debug(F101,"fnlist nfils","",nfils);
    debug(F110,"fnlist cmarg",cmarg,0);
    debug(F110,"fnlist cmarg2",cmarg2,0);
    if (!cmarg2) cmarg2 = "";
    if (nfils == 0) {			/* Sending from stdin or memory. */
	if ((cmarg2 != NULL) && (*cmarg2)) {
	    cmarg = cmarg2;		/* If F packet, "as-name" is used */
	    cmarg2 = "";		/* if provided */
	} else
	  cmarg = "stdin";		/* otherwise just use "stdin" */
	ckstrncpy(cmargbuf,cmarg,CKMAXPATH+1);
	cmargp[0] = cmargbuf;
	cmargp[1] = "";
	cmlist = cmargp;
	nfils = 1;
    }
}

int
sinit() {
    int x;				/* Worker int */
    char *tp, *xp, *m;			/* Worker string pointers */

    filcnt = filrej = 0;		/* Initialize file counters */

    fnlist();

    xp = "";
    if (nfils < 0) {
#ifdef PIPESEND
	if (usepipes && protocol == PROTO_K && *cmarg == '!') {
	    pipesend = 1;
	    cmarg++;
	}
#endif /* PIPESEND */
	xp = cmarg;
    } else {
#ifndef NOMSEND
	if (addlist)
	  xp = filehead->fl_name;
	else
#endif /* NOMSEND */
	  if (filefile)
	    xp = filefile;
	  else if (calibrate)
	    xp = "Calibration";
	  else
	    xp = *cmlist;
    }
    debug(F110,"sinit xp",xp,0);
    x = gnfile();			/* Get first filename. */
    debug(F111,"sinit gnfile",ckitoa(gnferror),x);
    if (x == 0) x = gnferror;		/* If none, get error reason */
    m = NULL;				/* Error message pointer */
    debug(F101,"sinit gnfil","",x);
    switch (x) {
      case -6: m = "No files meet selection criteria"; break;
      case -5: m = "Too many files match wildcard"; break;
      case -4: m = "Cancelled"; break;
      case -3: m = "Read access denied"; break;
      case -2: m = "File is not readable"; break;
#ifdef COMMENT
      case -1: m = iswild(filnam) ? "No files match" : "File not found";
	break;
      case  0: m = "No filespec given!"; break;
#else
      case  0:
      case -1: m = iswild(filnam) ? "No files match" : "File not found";
	break;
#endif /* COMMENT */
      default:
	break;
    }
    debug(F101,"sinit nfils","",nfils);
    debug(F110,"sinit filnam",filnam,0);
    if (x < 1) {			/* Didn't get a file. */
	debug(F111,"sinit msg",m,x);
	if (server) {			/* Doing GET command */
	    errpkt((CHAR *)m);		/* so send Error packet. */
	} else if (!local) { 		/* Doing SEND command */
	    interrupted = 1;		/* (To suppress hint) */
	    printf("?%s\r\n",m);
	} else {
	    xxscreen(SCR_EM,0,0l,m);	/* so print message. */
	}
	tlog(F110,xp,m,0L);		/* Make transaction log entry. */
	freerbuf(rseqtbl[0]);		/* Free the buffer the GET came in. */
	return(0);			/* Return failure code */
    }
    if (!local && !server && ckdelay > 0) /* OS-9 sleep(0) == infinite */
      sleep(ckdelay);			/* Delay if requested */
#ifdef datageneral
    if ((local) && (!quiet))            /* Only do this if local & not quiet */
      consta_mt();			/* Start the async read task */
#endif /* datageneral */
    freerbuf(rseqtbl[0]);		/* Free the buffer the GET came in. */
    sipkt('S');				/* Send the Send-Init packet. */
    ztime(&tp);				/* Get current date/time */
    tlog(F110,"Transaction begins",tp,0L); /* Make transaction log entry */
    tlog(F110,"Global file mode:", binary ? "binary" : "text", 0L);
    tlog(F100,"","",0);
    debug(F111,"sinit ok",filnam,0);
    return(1);
}

int
#ifdef CK_ANSIC
sipkt(char c)				/* Send S or I packet. */
#else
sipkt(c) char c;
#endif
/* sipkt */ {
    CHAR *rp; int k, x;
    extern int sendipkts;
    debug(F101,"sipkt pktnum","",pktnum); /* (better be 0...) */
    ttflui();				/* Flush pending input. */
    /*
      If this is an I packet and SET SEND I-PACKETS is OFF, don't send it;
      set sstate to 'Y' which makes the next input() call return 'Y' as if we
      had received an ACK to the I packet we didn't send.  This is to work
      around buggy Kermit servers that can't handle I packets.
    */
    if ((sendipkts == 0) && (c == 'I')) { /* I packet but don't send I pkts? */
	sstate = 'Y';			  /* Yikes! */
	return(0);			  /* (see input()..)*/
    }
    k = sseqtbl[pktnum];		/* Find slot for this packet */
    if (k < 0) {			/* No slot? */
	k = getsbuf(winlo = pktnum);	/* Make one. */
	debug(F101,"sipkt getsbuf","",k);
    }
    rp = rpar();			/* Get protocol parameters. */
    if (!rp) rp = (CHAR *)"";
    x = spack(c,pktnum,(int)strlen((char *)rp),rp); /* Send them. */
    return(x);
}

/*  X S I N I T  --  Retransmit S-packet  */
/*
  For use in the GET-SEND sequence, when we start to send, but receive another
  copy of the GET command because the receiver didn't get our S packet.
  This retransmits the S packet and frees the receive buffer for the ACK.
  This special case is necessary because packet number zero is being re-used.
*/
VOID
xsinit() {
    int k;
    k = rseqtbl[0];
    debug(F101,"xsinit k","",k);
    if (k > -1)
    freerbuf(k);
    resend(0);
}

/*  R C V F I L -- Receive a file  */

/*
  Incoming filename is in data field of F packet.
  This function decodes it into the srvcmd buffer, substituting an
  alternate "as-name", if one was given.
  Then it does any requested transformations (like converting to
  lowercase), and finally if a file of the same name already exists,
  takes the desired collision action.
  Returns:
    1 on success.
    0 on failure.
*/
char ofn1[CKMAXPATH+4];			/* Buffer for output file name */
char * ofn2;				/* Pointer to backup file name */
int ofn1x;				/* Flag output file already exists */
CK_OFF_T ofn1len = (CK_OFF_T)0;
int opnerr;				/* Flag for open error */

int					/* Returns success ? 1 : 0 */
rcvfil(n) char *n; {
    extern int en_cwd;
    int i, skipthis;
    char * n2;
    char * dispo;
#ifdef OS2ONLY
    char *zs, *longname, *newlongname, *pn; /* OS/2 long name items */
#endif /* OS2ONLY */
#ifdef DTILDE
    char *dirp;
#endif /* DTILDE */
    int dirflg, x, y;
#ifdef PIPESEND
    extern char * rcvfilter;
#endif /* PIPESEND */
#ifdef CALIBRATE
    extern int dest;
    CK_OFF_T csave;
    csave = calibrate;			/* So we can decode filename */
    calibrate = (CK_OFF_T)0;
#endif /* CALIBRATE */

    ofperms = "";			/* Reset old-file permissions */
    opnerr = 0;				/* No open error (yet) */
    ofn2 = NULL;			/* No new name (yet) */
    lsstate = 0;			/* Cancel locking-shift state */
    srvptr = srvcmd;			/* Decode file name from packet. */

#ifdef UNICODE
    xpnbyte(-1,0,0,NULL);		/* Reset UCS-2 byte counter. */
#endif /* UNICODE */

    debug(F110,"rcvfil rdatap",rdatap,0);
    decode(rdatap,putsrv,0);		/* Don't xlate charsets. */
#ifdef CALIBRATE
    calibrate = csave;
    if (dest == DEST_N) {
	calibrate = 1;
	cmarg2 = "CALIBRATE";
    }
#endif /* CALIBRATE */
    if (*srvcmd == '\0')		/* Watch out for null F packet. */
      ckstrncpy((char *)srvcmd,"NONAME",srvcmdlen);
    makestr(&prrfspec,(char *)srvcmd);	/* New preliminary filename */
#ifdef DTILDE
    if (*srvcmd == '~') {
	dirp = tilde_expand((char *)srvcmd); /* Expand tilde, if any. */
	if (*dirp != '\0')
	  ckstrncpy((char *)srvcmd,dirp,srvcmdlen);
    }
#else
#ifdef OS2
    if (isalpha(*srvcmd) && srvcmd[1] == ':' && srvcmd[2] == '\0')
      ckstrncat((char *)srvcmd,"NONAME",srvcmdlen);
#endif /* OS2 */
#endif /* DTILDE */

#ifndef NOICP
#ifndef NOSPL
/* File dialog when downloading...  */
    if (
#ifdef CK_APC
	(apcactive == APC_LOCAL && adl_ask) || /* Autodownload with ASK */
#endif /* CK_APC */
	(clcmds && haveurl)		/* Or "kermit:" or "iksd:" URL */
	) {
	int x;
	char fnbuf[CKMAXPATH+1];	/* Result buffer */
	char * preface;

	if (clcmds && haveurl)
	  preface = "\r\nIncoming file from Kermit server...\r\n\
Please confirm output file specification or supply an alternative:";
	else
	  preface = "\r\nIncoming file from remote Kermit...\r\n\
Please confirm output file specification or supply an alternative:";

	x = uq_file(preface,		/* Preface */
		    NULL,		/* Prompt (let uq_file() built it) */
		    3,			/* Output file */
		    NULL,		/* Help text */
		    (char *)srvcmd,	/* Default */
		    fnbuf,		/* Result buffer */
		    CKMAXPATH+1		/* Size of result buffer */
		    );
	if (x < 1) {			/* Refused */
	    rf_err = "Refused by user";
	    return(0);
	}
	ckstrncpy((char *)srvcmd,fnbuf,CKMAXPATH+1);
	if (isabsolute((char *)srvcmd)) { /* User gave an absolute path */
	    g_fnrpath = fnrpath;	/* Save current RECEIVE PATHNAMES */
	    fnrpath = PATH_ABS;		/* switch to ABSOLUTE */
	}
    }
#endif /* NOSPL */
#endif /* NOICP */

    if (!ENABLED(en_cwd)) {		/* CD is disabled */
	zstrip((char *)(srvcmd+2),&n2); /* and they included a pathname, */
	if (strcmp((char *)(srvcmd+2),n2)) { /* so refuse. */
	    rf_err = "Access denied";
	    return(0);
	}
    }
#ifdef COMMENT
    /* Wrong place for this -- handle cmarg2 first -- see below...  */

    if (zchko((char *)srvcmd) < 0) {	/* Precheck for write access */
	debug(F110,"rcvfil access denied",srvcmd,0);
	rf_err = "Write access denied";
	discard = opnerr = 1;
	return(0);
    }
    xxscreen(SCR_FN,0,0l,(char *)srvcmd); /* Put it on screen if local */
    debug(F110,"rcvfil srvcmd 1",srvcmd,0);
    tlog(F110,"Receiving",(char *)srvcmd,0L); /* Transaction log entry */
#endif /* COMMENT */

    skipthis = 0;			/* This file in our exception list? */
    for (i = 0; i < NSNDEXCEPT; i++) {
	if (!rcvexcept[i]) {
	    break;
	}
	if (ckmatch(rcvexcept[i],(char *)srvcmd,filecase,1)) {
	    skipthis = 1;
	    break;
	}
    }

#ifdef DEBUG
    if (deblog && skipthis) {
	debug(F111,"rcvfil rcvexcept",rcvexcept[i],i);
	debug(F110,"rcvfil skipping",srvcmd,0);
    }
#endif /* DEBUG */

    if (skipthis) {			/* Skipping this file */
	discard = 1;
	rejection = 1;
	rf_err = "Exception list";
	debug(F101,"rcvfil discard","",discard);
	tlog(F100," refused: exception list","",0);
	return(1);
    }

    /* File is not in exception list */

    if (!cmarg2)			/* No core dumps please */
      cmarg2 = "";
    debug(F110,"rcvfil cmarg2",cmarg2,0);

    if (*cmarg2) {			/* Check for alternate name */
#ifndef NOSPL
	int y; char *s;			/* Pass it thru the evaluator */
	extern int cmd_quoting;
	if (cmd_quoting) {
	    y = MAXRP;
	    ckstrncpy(ofn1,(char *)srvcmd,CKMAXPATH+1); /* for \v(filename) */
	    s = (char *)srvcmd;
	    zzstring(cmarg2,&s,&y);
	} else
	  *srvcmd = NUL;
	if (!*srvcmd)			/* If we got something */
#endif /* NOSPL */
	  ckstrncpy((char *)srvcmd,cmarg2,srvcmdlen);
    }
    debug(F110,"rcvfil srvcmd 2",srvcmd,0);

#ifdef PIPESEND
    /* If it starts with "bang", it's a pipe, not a file. */
    if (usepipes && protocol == PROTO_K && *srvcmd == '!' && !rcvfilter) {
	CHAR *s;
	s = srvcmd+1;			/* srvcmd[] is not a pointer. */
	while (*s) {			/* So we have to slide the contents */
	    *(s-1) = *s;		/* over 1 space to the left. */
	    s++;
	}
	*(s-1) = NUL;
	pipesend = 1;
    }
#endif /* PIPESEND */

#ifdef COMMENT
/*
  This is commented out because we need to know whether the name we are
  using was specified by the local user as an override, or came from the
  incoming packet.  In the former case, we don't do stuff to it (like
  strip the pathname) that we might do to it in the latter.
*/
    cmarg2 = "";			/* Done with alternate name */
#endif /* COMMENT */

    if ((int)strlen((char *)srvcmd) > CKMAXPATH) /* Watch out for overflow */
      *(srvcmd + CKMAXPATH - 1) = NUL;

    /* At this point, srvcmd[] contains the incoming filename or as-name. */
    /* So NOW we check for write access. */

    if (zchko((char *)srvcmd) < 0) {	/* Precheck for write access */
	debug(F110,"rcvfil access denied",srvcmd,0);
	rf_err = "Write access denied";
	discard = opnerr = 1;
	return(0);
    }
    xxscreen(SCR_FN,0,0l,(char *)srvcmd); /* Put it on screen if local */
    debug(F110,"rcvfil srvcmd 1",srvcmd,0);
    tlog(F110,"Receiving",(char *)srvcmd,0L); /* Transaction log entry */

#ifdef CK_LABELED
#ifdef VMS
/*
  If we have an as-name, this overrides the internal name if we are doing
  a labeled-mode transfer.
*/
    if (*cmarg2) {
	extern int lf_opts;
	lf_opts &= ~LBL_NAM;
    }
#endif /* VMS */
#endif /* CK_LABELED */

    debug(F111,"rcvfil pipesend",srvcmd,pipesend);

#ifdef PIPESEND
    /* Skip all the filename manipulation and collision actions */
    if (pipesend) {
	dirflg = 0;
	ofn1[0] = '!';
	ckstrncpy(&ofn1[1],(char *)srvcmd,CKMAXPATH+1);
	ckstrncpy(n,ofn1,CKMAXPATH+1);
	ckstrncpy(fspec,ofn1,CKMAXPATH+1);
	makestr(&prfspec,fspec);	/* New preliminary filename */
	debug(F110,"rcvfil pipesend",ofn1,0);
	goto rcvfilx;
    }
#endif /* PIPESEND */
/*
  This is to avoid passing email subjects through nzrtol().
  We haven't yet received the A packet so we don't yet know it's e-mail,
  so in fact we go ahead and convert it anyway, but later we get the
  original back from ofilnam[].
*/  
    dispos = 0;
    ckstrncpy(ofilnam,(char *)srvcmd,CKMAXPATH+1);

#ifdef NZLTOR
    if (*cmarg2)
      ckstrncpy((char *)ofn1,(char *)srvcmd,CKMAXPATH+1);
    else
      nzrtol((char *)srvcmd,		/* Filename from packet */
	     (char *)ofn1,		/* Where to put result */
	     fncnv,			/* Filename conversion */
	     fnrpath,			/* Pathname handling */
	     CKMAXPATH			/* Size of result buffer */
	     );
#else
    debug(F101,"rcvfil fnrpath","",fnrpath); /* Handle pathnames */
    if (fnrpath == PATH_OFF && !*cmarg2) { /* RECEIVE PATHNAMES OFF? */
	char *t;			/* Yes. */
	zstrip((char *)srvcmd,&t);	/* If there is a pathname, strip it */
	debug(F110,"rcvfil PATH_OFF zstrip",t,0);
	if (!t)				/* Be sure we didn't strip too much */
	  sprintf(ofn1,"FILE%02ld",filcnt);
	else if (*t == '\0')
	  sprintf(ofn1,"FILE%02ld",filcnt);
	else
	  ckstrncpy(ofn1,t,CKMAXPATH+1);
	ckstrncpy((char *)srvcmd,ofn1,srvcmdlen); /* Now copy it back. */
    }
/*
  SET RECEIVE PATHNAMES RELATIVE...
  The following doesn't belong here but doing it right would require
  defining and implementing a new file routine for all ck?fio.c modules.
  So for now...
*/
#ifdef UNIXOROSK
    else if (fnrpath == PATH_REL && !*cmarg2) {
	if (isabsolute((char *)srvcmd)) {
	    ofn1[0] = '.';
	    ckstrncpy(&of1n[1],(char *)srvcmd,CKMAXPATH+1);
	    ckstrncpy((char *)srvcmd,ofn1,srvcmdlen);
	    debug(F110,"rcvfil PATH_REL",ofn1,0);
	}
    }
#else
#ifdef OS2
    else if (fnrpath == PATH_REL && !*cmarg2) {
	if (isabsolute((char *)srvcmd)) {
	    char *p = (char *)srvcmd;
	    if (isalpha(*p) && *(p+1) == ':')
	      p += 2;
	    if (*p == '\\' || *p == '/')
	      p++;
	    ckstrncpy(ofn1,p,CKMAXPATH+1);
	    ckstrncpy((char *)srvcmd,ofn1,srvcmdlen);
	    debug(F110,"rcvfil OS2 PATH_REL",ofn1,0);
	}
    }
#endif /* OS2 */
#endif /* UNIXOROSK */

    /* Now srvcmd contains incoming filename with path possibly stripped */

    if (fncnv)				/* FILE NAMES CONVERTED? */
      zrtol((char *)srvcmd,(char *)ofn1); /* Yes, convert to local form */
    else
      ckstrncpy(ofn1,(char *)srvcmd,CKMAXPATH+1); /* No, copy literally. */
#endif /* NZLTOR */

#ifdef PIPESEND
    if (rcvfilter) {
	char * p = NULL, * q;
	int nn = MAXRP;
	pipesend = 1;
	debug(F110,"rcvfil rcvfilter ",rcvfilter,0);
#ifndef NOSPL
	if ((p = (char *) malloc(nn + 1))) {
	    q = p;
#ifdef COMMENT
            /* We have already processed srvcmd and placed it into ofn1 */
            ckstrncpy(ofn1,(char *)srvcmd,CKMAXPATH+1); /* For \v(filename) */
#endif /* COMMENT */
	    debug(F110,"rcvfile pipesend filter",rcvfilter,0);
	    zzstring(rcvfilter,&p,&nn);
	    debug(F111,"rcvfil pipename",q,nn);
	    if (nn <= 0) {
		printf(
		       "?Sorry, receive filter + filename too long, %d max.\n",
		       CKMAXPATH
		       );
		rf_err = "Name too long";
		free(q);
		return(0);
	    }
	    ckstrncpy((char *)srvcmd,q,MAXRP);
	    free(q);
	}
#endif /* NOSPL */
    }
#endif /* PIPESEND */

    /* Now the incoming filename, possibly converted, is in ofn1[]. */

#ifdef OS2
    /* Don't refuse the file just because the name is illegal. */
    if (!IsFileNameValid(ofn1)) {	/* Name is OK for OS/2? */
#ifdef OS2ONLY
	char *zs = NULL;
	zstrip(ofn1, &zs);		/* Not valid, strip unconditionally */
	if (zs) {
	    if (iattr.longname.len &&	/* Free previous longname, if any */
		iattr.longname.val)
	      free(iattr.longname.val);
	    iattr.longname.len = strlen(zs); /* Store in attribute structure */
	    iattr.longname.val = (char *) malloc(iattr.longname.len + 1);
	    if (iattr.longname.val)	/* Remember this (illegal) name */
	      strcpy(iattr.longname.val, zs); /* safe */
	}
#endif /* OS2ONLY */
	debug(F110,"rcvfil: invalid file name",ofn1,0);
	ChangeNameForFAT(ofn1);	/* Change to an acceptable name */
	debug(F110,"rcvfil: FAT file name",ofn1,0);

    } else {				/* Name is OK. */

	debug(F110,"rcvfil: valid file name",ofn1,0);
#ifdef OS2ONLY
	if (iattr.longname.len &&
	     iattr.longname.val)	/* Free previous longname, if any */
	  free(iattr.longname.val);
	iattr.longname.len = 0;
	iattr.longname.val = NULL;	/* This file doesn't need a longname */
#endif /* OS2ONLY */
    }
#endif /* OS2 */
    debug(F110,"rcvfil as",ofn1,0);

/* Filename collision action section. */

    dirflg =				/* Is it a directory name? */
#ifdef CK_TMPDIR
        isdir(ofn1)
#else
	0
#endif /* CK_TMPDIR */
	  ;
    debug(F101,"rcvfil dirflg","",dirflg);
    ofn1len = zchki(ofn1);		/* File already exists? */
    debug(F111,"rcvfil ofn1len",ofn1,ofn1len);
    ofn1x = (ofn1len != (CK_OFF_T)-1);

    if ( (
#ifdef UNIX
	strcmp(ofn1,"/dev/null") &&	/* It's not the null device? */
#else
#ifdef OSK
	strcmp(ofn1,"/nil") &&
#endif /* OSK */
#endif /* UNIX */
	!stdouf ) &&			/* Not copying to standard output? */
	ofn1x ||			/* File of same name exists? */
	dirflg ) {			/* Or file is a directory? */
        debug(F111,"rcvfil exists",ofn1,fncact);
#ifdef CK_PERMS
	ofperms = zgperm((char *)ofn1);	/* Get old file's permissions */
	debug(F110,"rcvfil perms",ofperms,0);
#endif /* CK_PERMS */

	debug(F101,"rcvfil fncact","",fncact);
	switch (fncact) {		/* Yes, do what user said. */
	  case XYFX_A:			/* Append */
	    ofperms = "";
	    debug(F100,"rcvfil append","",0);
	    if (dirflg) {
		rf_err = "Can't append to a directory";
		tlog(F100," error - can't append to directory","",0);
		discard = opnerr = 1;
		return(0);
	    }
	    tlog(F110," appending to",ofn1,0);
	    break;
#ifdef COMMENT
	  case XYFX_Q:			/* Query (Ask) */
	    break;			/* not implemented */
#endif /* COMMENT */
	  case XYFX_B:			/* Backup (rename old file) */
	    if (dirflg) {
		rf_err = "Can't rename existing directory";
		tlog(F100," error - can't rename directory","",0);
		discard = opnerr = 1;
		return(0);
	    }
	    znewn(ofn1,&ofn2);		/* Get new unique name */
	    tlog(F110," backup:",ofn2,0);
	    debug(F110,"rcvfil backup ofn1",ofn1,0);
	    debug(F110,"rcvfil backup ofn2",ofn2,0);
#ifdef CK_LABELED
#ifdef OS2ONLY
/*
  In case this is a FAT file system, we can't change only the FAT name, we
  also have to change the longname from the extended attributes block.
  Otherwise, we'll have many files with the same longname and if we copy them
  to an HPFS volume, only one will survive.
*/
	    if (os2getlongname(ofn1, &longname) > -1) {
		if (strlen(longname)) {
		    char tmp[10];
		    extern int ck_znewn;
		    sprintf(tmp,".~%d~",ck_znewn);
		    newlongname =
		      (char *) malloc(strlen(longname) + strlen(tmp) + 1);
		    if (newlongname) {
			strcpy(newlongname, longname); /* safe (prechecked) */
			strcat(newlongname, tmp); /* safe (prechecked) */
			os2setlongname(ofn1, newlongname);
			free(newlongname);
			newlongname = NULL;
		    }
		}
	    } else debug(F100,"rcvfil os2getlongname failed","",0);
#endif /* OS2ONLY */
#endif /* CK_LABELED */

#ifdef COMMENT
	    /* Do this later, in opena()... */
	    if (zrename(ofn1,ofn2) < 0) {
		rf_err = "Can't transform filename";
		debug(F110,"rcvfil rename fails",ofn1,0);
		discard = opnerr = 1;
		return(0);
	    }
#endif /* COMMENT */
	    break;

	  case XYFX_D:			/* Discard (refuse new file) */
	    ofperms = "";
	    discard = 1;
	    rejection = 1;		/* Horrible hack: reason = name */
	    debug(F101,"rcvfil discard","",discard);
	    tlog(F100," refused: name","",0);
	    break;

	  case XYFX_R:			/* Rename incoming file */
	    znewn(ofn1,&ofn2);		/* Make new name for it */
#ifdef OS2ONLY
	    if (iattr.longname.len) {
		char tmp[10];
		extern int ck_znewn;
		sprintf(tmp,".~%d~",ck_znewn);
		newlongname =
		  (char *) malloc(iattr.longname.len + strlen(tmp) + 1);
		if (newlongname) {
		    strcpy(newlongname, iattr.longname.val); /* safe */
		    strcat(newlongname, tmp); /* safe */
		    debug(F110,
			  "Rename Incoming: newlongname",newlongname,0);
		    if (iattr.longname.len &&
			iattr.longname.val)
		      free(iattr.longname.val);
		    iattr.longname.len = strlen(newlongname);
		    iattr.longname.val = newlongname;
		    /* free(newlongname) here ? */
		}
	    }
#endif /* OS2ONLY */
	    break;
	  case XYFX_X:			/* Replace old file */
	    debug(F100,"rcvfil overwrite","",0);
	    if (dirflg) {
		rf_err = "Can't overwrite existing directory";
		tlog(F100," error - can't overwrite directory","",0);
		discard = opnerr = 1;
#ifdef COMMENT
		return(0);
#else
		break;
#endif /* COMMENT */
	    }
	    tlog(F110,"overwriting",ofn1,0);
	    break;
	  case XYFX_U:			/* Refuse if older */
	    debug(F110,"rcvfil update",ofn1,0);
	    if (dirflg) {
		rf_err = "File has same name as existing directory";
		tlog(F110," error - directory exists:",ofn1,0);
		discard = opnerr = 1;
#ifdef COMMENT
		/* Don't send an error packet, just refuse the file */
		return(0);
#endif /* COMMENT */
	    }
	    break;			/* Not here, we don't have */
					/* the attribute packet yet. */
	  default:
	    ofperms = "";
	    debug(F101,"rcvfil bad collision action","",fncact);
	    break;
	}
    }
    debug(F110,"rcvfil ofn1",ofn1,0);
    debug(F110,"rcvfil ofn2",ofn2,0);
    debug(F110,"rcvfil ofperms",ofperms,0);
    if (fncact == XYFX_R && ofn1x && ofn2) { /* Renaming incoming file? */
	xxscreen(SCR_AN,0,0l,ofn2);	/* Display renamed name */
	ckstrncpy(n, ofn2, CKMAXPATH+1); /* Return it */
    } else {				/* No */
	xxscreen(SCR_AN,0,0l,ofn1);	/* Display regular name */
	ckstrncpy(n, ofn1, CKMAXPATH+1); /* and return it. */
    }

#ifdef CK_MKDIR
/*  Create directory(s) if necessary.  */
    if (!discard && fnrpath != PATH_OFF) { /* RECEIVE PATHAMES ON? */
	int x;
	debug(F110,"rcvfil calling zmkdir",ofn1,0); /* Yes */
	if ((x = zmkdir(ofn1)) < 0) {
	    debug(F100,"zmkdir fails","",0);
	    tlog(F110," error - directory creation failure:",ofn1,0);
	    rf_err = "Directory creation failure.";
	    discard = 1;
	    return(0);
	}
#ifdef TLOG
	else if (x > 0)
	  tlog(F110," path created:",ofn1,0);
#endif /* TLOG */
    }
#else
    debug(F110,"rcvfil CK_MKDIR not defined",ofn1,0);
#endif /* CK_MKDIR */

    if (calibrate)
      ckstrncpy(fspec,ofn1,CKMAXPATH+1);
    else
      zfnqfp(ofn1,fspeclen,fspec);
    debug(F110,"rcvfil fspec",fspec,0);

#ifdef COMMENT
    /* See comments with VMS zfnqfp()... */
#ifdef VMS
    /* zfnqfp() does not return the version number */
    if (!calibrate) {
	int x, v;
	x = strlen(ofn1);
	if (x > 0) {
	    if (ofn1[x-1] == ';') {
		v = getvnum(ofn1);
		ckstrncpy(&ofn1[x],ckitoa(v),CKMAXPATH-x);
	    }
	}
    }
#endif /* VMS */
#endif /* COMMENT */
    fspec[fspeclen] = NUL;
    makestr(&prfspec,fspec);		/* New preliminary filename */

#ifdef PIPESEND
  rcvfilx:
#endif /* PIPESEND */

    debug(F110,"rcvfilx: n",n,0);
    debug(F110,"rcvfilx: ofn1",ofn1,0);
    ffc = (CK_OFF_T)0;			/* Init per-file counters */
    cps = oldcps = 0L;
    rs_len = (CK_OFF_T)0;
    rejection = -1;
    fsecs = gtimer();			/* Time this file started */
#ifdef GFTIMER
    fpfsecs = gftimer();
    debug(F101,"rcvfil fpfsecs","",fpfsecs);
#endif /* GFTIMER */
    filcnt++;
    intmsg(filcnt);
    return(1);				/* Successful return */
}


/*  R E O F  --  Receive End Of File packet for incoming file */

/*
  Closes the received file.
  Returns:
    0 on success.
   -1 if file could not be closed.
    2 if disposition was mail, mail was sent, but temp file not deleted.
    3 if disposition was print, file was printed, but not deleted.
   -2 if disposition was mail and mail could not be sent
   -3 if disposition was print and file could not be printed
   -4 if MOVE-TO: failed
   -5 if RENAME-TO: failed
*/
int
reof(f,yy) char *f; struct zattr *yy; {
    extern char * rcv_move, * rcv_rename;
    extern int o_isopen;
    int rc = 0;				/* Return code */
    char *p;
    char c;

    debug(F111,"reof fncact",f,fncact);
    debug(F101,"reof discard","",discard);
    success = 1;			/* Assume status is OK */
    lsstate = 0;			/* Cancel locking-shift state */
    if (discard) {			/* Handle attribute refusals, etc. */
	debug(F101,"reof discarding","",0);
	success = 0;			/* Status = failed. */
	if (rejection == '#' ||		/* Unless rejection reason is */
	    rejection ==  1  ||		/* date or name (SET FILE COLLISION */
	    rejection == '?')		/* UPDATE or DISCARD) */
	  success = 1;
	debug(F101,"reof success","",success);
	filrej++;			/* Count this rejection. */
	discard = 0;			/* We never opened the file, */
	return(0);			/* so we don't close it. */
    }
#ifdef DEBUG
    if (deblog) {
	debug(F101,"reof cxseen","",cxseen);
	debug(F101,"reof czseen","",czseen);
	debug(F110,"reof rdatap",rdatap,0);
    }
#endif /* DEBUG */

    if (cxseen == 0)			/* Got cancel directive? */
      cxseen = (*rdatap == 'D');
    if (cxseen || czseen)		/* (for hints) */
      interrupted = 1;
    success = (cxseen || czseen) ? 0 : 1; /* Set SUCCESS flag appropriately */
    if (!success)			  /* "Uncount" this file */
      filrej++;
    debug(F101,"reof o_isopen","",o_isopen);

    if (o_isopen) {			/* If an output file was open... */

#ifdef CK_CTRLZ
	if (success) {
	    debug(F101,"reof lastchar","",lastchar);
	    if (!binary && eofmethod == XYEOF_Z && lastchar != 26 &&
		(!xflg || (xflg && remfile)))
	      pnbyte((char)26,putfil);
	}
#endif /* CK_CTRLZ */

	rc = clsof(cxseen || czseen);	/* Close the file (resets cxseen) */
	debug(F101,"reof closf","",rc);
	if (rc < 0) {			/* If failure to close, FAIL */
	    if (success) filrej++;
	    success = 0;
	}
	if (!calibrate) {
	    /* Set file modification date and/or permissions */
	    if (success)
	      zstime(f,yy,0);
#ifdef OS2ONLY
#ifdef CK_LABELED
	    if (success && yy->longname.len)
	      os2setlongname(f, yy->longname.val);
#endif /* CK_LABELED */
#endif /* OS2ONLY */
	}
	if (success == 0) {		/* And program return code */
	    xitsta |= W_RECV;
	} else if (success == 1) {	/* File rec'd successfully */
	    makestr(&rrfspec,prrfspec);	/* Record it for wheremsg */
	    makestr(&rfspec,prfspec);
	}

/* Handle dispositions from attribute packet... */

	c = NUL;
#ifndef NOFRILLS
	if (!calibrate && yy->disp.len != 0) {
	    p = yy->disp.val;
	    c = *p++;
#ifndef UNIX
/*
  See ckcpro.w.  In UNIX we don't use temp files any more -- we pipe the
  stuff right into mail or lpr.
*/
	    if (c == 'M') {		/* Mail to user. */
		rc = zmail(p,filnam);	/* Do the system's mail command */
		if (rc < 0) success = 0;	/* Remember status */
		tlog(F110,"mailed",filnam,0L);
		tlog(F110," to",p,0L);
		zdelet(filnam);		/* Delete the file */
	    } else if (c == 'P') {	/* Print the file. */
		rc = zprint(p,filnam);	/* Do the system's print command */
		if (rc < 0) success = 0; /* Remember status */
		tlog(F110,"printed",filnam,0L);
		tlog(F110," with options",p,0L);
#ifndef VMS
#ifndef STRATUS
		/* spooler deletes file after print complete in VOS & VMS */
		if (zdelet(filnam) && rc == 0) rc = 3; /* Delete the file */
#endif /* STRATUS */
#endif /* VMS */
	    }
#endif /* UNIX */
	}
#endif /* NOFRILLS */

	if (success &&
	    !pipesend &&
	    !calibrate && c != 'M' && c != 'P') {
	    if (rcv_move) {		/* If /MOVE-TO was given... */
		char * p = rcv_move;
#ifdef COMMENT
/* No need for this - it's a directory name */
		char tmpbuf[CKMAXPATH+1];
		extern int cmd_quoting;	/* for \v(filename) */
		if (cmd_quoting) {	/* But only if cmd_quoting is on */
		    int n;
		    n = CKMAXPATH;
		    p = tmpbuf;
		    debug(F111,"reof /move-to",rcv_move,0);
		    zzstring(rcv_move,&p,&n);
		    p = tmpbuf;
		}
#endif /* COMMENT */
/*
  Here we could create the directory if it didn't exist (and it was relative)
  but there would have to be a user-settable option to say whether to do this.
*/
		rc = zrename(filnam,p);
		debug(F111,"reof MOVE zrename",rcv_move,rc);
		if (rc > -1) {
		    tlog(F110," moving received file to",rcv_move,0);
		} else {
		    rc = -4;
		    tlog(F110," FAILED to move received file to",rcv_move,0);
		}
	    } else if (rcv_rename) {	/* Or /RENAME-TO: */
		char *s = rcv_rename;	/* This is the renaming string */
#ifndef NOSPL
		char tmpnam[CKMAXPATH+16];
		extern int cmd_quoting;	/* for \v(filename) */
		if (cmd_quoting) {	/* But only if cmd_quoting is on */
		    int n;		/* Pass it thru the evaluator */
		    n = CKMAXPATH;
		    s = (char *)tmpnam;
		    zzstring(rcv_rename,&s,&n);
		    s = (char *)tmpnam;
		}
#endif /* NOSPL */
		if (s) if (*s) {
		    rc = zrename(filnam,s);
		    debug(F111,"reof RENAME zrename",s,rc);
		    if (rc > -1) {
			tlog(F110," renaming received file to",s,0);
		    } else {
			rc = -5;
			tlog(F110," FAILED to rename received file to",s,0);
		    }
		}
	    }
	}
    }
    debug(F101,"reof success","",success);
    debug(F101,"reof returns","",rc);

    filnam[0] = NUL;			/* Erase the filename */
    return(rc);
}

/*  R E O T  --  Receive End Of Transaction  */

VOID
reot() {
    cxseen = czseen = discard = 0;	/* Reset interruption flags */
    tstats();				/* Finalize transfer statistics */
}

/*  S F I L E -- Send File header or teXt header packet  */

/*
  Call with x nonzero for X packet, zero for F packet.
  If X == 0, filename to send is in filnam[], and if cmarg2 is not null
  or empty, the file should be sent under this name rather than filnam[].
  If sndfilter not NULL, it is the name of a send filter.
  Returns 1 on success, 0 on failure.
*/
int
sfile(x) int x; {
#ifdef pdp11
#define PKTNL 64
#else
#define PKTNL 256
#endif /* pdp11 */
    char pktnam[PKTNL+1];		/* Local copy of name */
    char *s;
    int rc;
    int notafile = 0;
    extern int filepeek;
#ifdef PIPESEND
    extern char * sndfilter;

    if (sndfilter) {
	pipesend = 1;
	debug(F110,"sfile send filter ",sndfilter,0);
    }
#endif /* PIPESEND */

    notafile = calibrate || sndarray || pipesend || x;
    debug(F101,"sfile x","",x);
    debug(F101,"sfile notafile","",notafile);

#ifndef NOCSETS
    if (tcs_save > -1) {                /* Character sets */
        tcharset = tcs_save;
        tcs_save = -1;
	debug(F101,"sfile restored tcharset","",tcharset);
    }
    if (fcs_save > -1) {
        fcharset = fcs_save;
        fcs_save = -1;
	debug(F101,"sfile restored fcharset","",fcharset);
    }
    setxlatype(tcharset,fcharset);      /* Translation type */
#endif /* NOCSETS */

    /* cmarg2 or filnam (with that precedence) have the file's name */

    lsstate = 0;			/* Cancel locking-shift state */
#ifdef COMMENT
    /* Do this after making sure we can open the file */
    if (nxtpkt() < 0) return(0);	/* Bump packet number, get buffer */
#endif /* COMMENT */
    pktnam[0] = NUL;			/* Buffer for name we will send */
    if (x == 0) {			/* F-Packet setup */
	if (!cmarg2) cmarg2 = "";
#ifdef DEBUG
	if (deblog) {
	    /* debug(F111,"sfile cmarg2",cmarg2,cmarg2); */
	    debug(F101,"sfile binary 1","",binary);
	    debug(F101,"sfile wearealike","",wearealike);
	    debug(F101,"sfile xfermode","",xfermode);
	    debug(F101,"sfile filepeek","",filepeek);
#ifndef NOCSETS
	    debug(F101,"sfile s_cset","",s_cset);
	    debug(F101,"sfile tcharset","",tcharset);
	    debug(F101,"sfile xfrxla","",xfrxla);
#endif /* NOCSETS */
	}
#endif /* DEBUG */
	if (xfermode == XMODE_A		/* TRANSFER MODE AUTOMATIC */
#ifndef NOMSEND
	    && !addlist			/* And not working from a SEND-LIST */
#endif /* NOMSEND */
	    ) {
	    /* Other Kermit is on a like system and no charset translation */
	    if (wearealike
#ifndef NOCSETS
		&& (tcharset == TC_TRANSP || xfrxla == 0)
#endif /* NOCSETS */
		) {
#ifdef VMS
		if (binary != XYFT_I)
#endif /* VMS */
#ifdef CK_LABELED
		  if (binary != XYFT_L)
#endif /* CK_LABELED */
		    binary = XYFT_B;	/* Send all files in binary mode */
	    }

	    /* Otherwise select transfer mode based on file info */

	    else if (!notafile		/* but not if sending from pipe */
#ifdef CK_LABELED
		     && binary != XYFT_L /* and not if FILE TYPE LABELED */
#endif /* CK_LABELED */
#ifdef VMS
		     && binary != XYFT_I /* or FILE TYPE IMAGE */
#endif /* VMS */
		     ) {
#ifdef UNICODE
		fileorder = -1;		/* File byte order */
#endif /* UNICODE */
		if (filepeek && !notafile) { /* Real file, FILE SCAN is ON */
		    int k, x;
		    k = scanfile(filnam,&x,nscanfile); /* Scan the file */
		    debug(F101,"sfile scanfile","",k);
		    switch (k) {
		      case FT_TEXT:	/* Unspecified text */
			debug(F100,"sfile scanfile text","",0);
			binary = XYFT_T; /* SET FILE TYPE TEXT */
			break;
#ifndef NOCSETS
		      case FT_7BIT:		/* 7-bit text */
			binary = XYFT_T;	/* SET FILE TYPE TEXT */
			/* If SEND CHARSET-SELECTION AUTO  */
			/* and SET TRANSFER TRANSLATION is ON */
			debug(F100,"sfile scanfile text7","",0);
			if (s_cset == XMODE_A && xfrxla) {
			    if (fcsinfo[fcharset].size != 128) {
				fcs_save = fcharset; /* Current FCS not 7bit */
				fcharset = dcset7;   /* Use default 7bit set */
				debug(F101,"sfile scanfile 7 fcharset",
				      "",fcharset);
			    }
			    /* And also switch to appropriate TCS */
			    if (afcset[fcharset] > -1 &&
				afcset[fcharset] <= MAXTCSETS) {
				tcs_save = tcharset;
				tcharset = afcset[fcharset];
				debug(F101,"sfile scanfile 7 tcharset","",
				      tcharset);
			    }
			    setxlatype(tcharset,fcharset);
			}
			break;

		      case FT_8BIT:	/* 8-bit text */
			binary = XYFT_T; /* SET FILE TYPE TEXT */
			/* If SEND CHARSET-SELEC AUTO  */
			/* and SET TRANSFER TRANSLATION is ON */
			debug(F100,"sfile scanfile text8","",0);
			if (s_cset == XMODE_A && xfrxla) {
			    if (fcsinfo[fcharset].size != 256) {
				fcs_save = fcharset; /* Current FCS not 8bit */
				fcharset = dcset8; /* Use default 8bit set */
				debug(F101,"sfile scanfile 8 fcharset",
				      "",fcharset);
			    }
			    /* Switch to corresponding transfer charset */
			    if (afcset[fcharset] > -1 &&
				afcset[fcharset] <= MAXTCSETS) {
				tcs_save = tcharset;
				tcharset = afcset[fcharset];
				debug(F101,"sfile scanfile 8 tcharset","",
				      tcharset);
			    }
			    setxlatype(tcharset,fcharset);
			}
			break;
#ifdef UNICODE
		      case FT_UTF8:	/* UTF-8 text */
		      case FT_UCS2:	/* UCS-2 text */
			debug(F101,"sfile scanfile Unicode","",k);
			binary = XYFT_T;
			/* If SEND CHARSET-SELEC AUTO  */
			/* and SET TRANSFER TRANSLATION is ON */
			if (s_cset == XMODE_A && xfrxla) {
			    fcs_save = fcharset;
			    tcs_save = tcharset;
			    fcharset = (k == FT_UCS2) ? FC_UCS2 : FC_UTF8;
			    if (k == FT_UCS2 && x > -1)
			      fileorder = x;
			}
			/* Switch to associated transfer charset if any */
			if (afcset[fcharset] > -1 &&
			    afcset[fcharset] <= MAXTCSETS)
			  tcharset = afcset[fcharset];
			if (tcharset == TC_TRANSP) /* If none */
			  tcharset = TC_UTF8;      /* use UTF-8 */
			setxlatype(tcharset,fcharset);
			debug(F101,"sfile Unicode tcharset","",tcharset);
			break;
#endif /* UNICODE */
#endif /* NOCSETS */
		      case FT_BIN:
			debug(F101,"sfile scanfile binary","",k);
			binary = XYFT_B;
			break;
		    /* Default: Don't change anything */
		    }
		}
	    }
	    debug(F101,"sfile binary 2","",binary);
	    debug(F101,"sfile sendmode","",sendmode);
	}
    	if (*cmarg2) {			/* If we have a send-as name... */
	    int y; char *s;
#ifndef NOSPL				/* and a script programming language */
	    extern int cmd_quoting;
	    if (cmd_quoting) {		/* and it's not turned off */
		y = PKTNL;		/* pass as-name thru the evaluator */
		s = pktnam;
		zzstring(cmarg2,&s,&y);
#ifdef COMMENT
/* This ruins macros like BSEND */
		if (!pktnam[0])		/* and make sure result is not empty */
		  sprintf(pktnam,"FILE%02ld",filcnt);
#endif /* COMMENT */
	    } else
#endif /* NOSPL */
	      ckstrncpy(pktnam,cmarg2,PKTNL); /* copy it literally, */

	    debug(F110,"sfile pktnam",pktnam,0);
#ifdef COMMENT
/* We don't do this any more because now we have filename templates */
	    cmarg2 = "";		/* and blank it out for next time. */
#endif /* COMMENT */
    	}
	if (!*pktnam) {			/* No as-name... */
#ifdef NZLTOR
	    int xfncnv, xpath;
	    debug(F101,"sfile fnspath","",fnspath);
	    debug(F101,"sfile fncnv","",fncnv);
	    xfncnv = fncnv;
	    xpath = fnspath;
	    if (notafile) {		/* If not an actual file */
		xfncnv = 0;		/* Don't convert name */
		xpath = PATH_OFF;	/* Leave path off */
	    } else if (xfncnv &&
		       (!strcmp(whoareu,"U1") || !strcmp(whoareu,"UN"))
		       ) {
		/* Less-strict conversion if partner is UNIX or Win32 */
		xfncnv = -1;
	    }
	    debug(F101,"sfile xpath","",xpath);
	    debug(F101,"sfile xfncnv","",xfncnv);
	    nzltor(filnam,pktnam,xfncnv,xpath,PKTNL);

#else  /* Not NZLTOR */

	    debug(F101,"sfile fnspath","",fnspath);
	    if (fnspath == PATH_OFF	/* Stripping path names? */
		&& (!notafile)		/* (of actual files) */
		) {
		char *t;		/* Yes. */
		zstrip(filnam,&t);	/* Strip off the path. */
		debug(F110,"sfile zstrip",t,0);
		if (!t) t = "UNKNOWN";	/* Be cautious... */
		else if (*t == '\0')
		  t = "UNKNOWN";
		ckstrncpy(pktnam,t,PKTNL); /* Copy stripped name literally. */
	    } else if (fnspath == PATH_ABS && !notafile) {
		/* Converting to absolute form */
		zfnqfp(filnam,PKTNL,pktnam);
	    } else
		ckstrncpy(pktnam,filnam,PKTNL);

	    /* pktnam[] has the packet name, filnam[] has the original name. */
	    /* But we still need to convert pktnam if FILE NAMES CONVERTED.  */

	    debug(F101,"sfile fncnv","",fncnv);
	    if (fncnv && !notafile) {	/* If converting names of files */
		zltor(pktnam,(char *)srvcmd); /* convert it to common form, */
		ckstrncpy(pktnam,(char *)srvcmd,PKTNL);
		*srvcmd = NUL;
	    }
#endif /* NZLTOR */
    	}
	if (!*pktnam)			/* Failsafe... */
	  sprintf(pktnam,"FILE%02ld",filcnt);
	debug(F110,"sfile filnam 1",filnam,0);
	debug(F110,"sfile pktnam 1",pktnam,0);
#ifdef PIPESEND
/* If we have a send filter, substitute the current filename into it */

	if (sndfilter) {
	    char * p = NULL, * q;
	    int n = CKMAXPATH;
#ifndef NOSPL
	    if ((p = (char *) malloc(n + 1))) {
		q = p;
		debug(F110,"sfile pipesend filter",sndfilter,0);
		zzstring(sndfilter,&p,&n);
		debug(F111,"sfile pipename",q,n);
		if (n <= 0) {
		    printf(
			  "?Sorry, send filter + filename too long, %d max.\n",
			   CKMAXPATH
			   );
		    free(q);
		    return(0);
		}
		ckstrncpy(filnam,q,CKMAXPATH+1);
		free(q);
	    }
#endif /* NOSPL */
	}
#endif /* PIPESEND */

    	debug(F110,"sfile filnam 2",filnam,0); /* Log debugging info */
    	debug(F110,"sfile pktnam 2",pktnam,0);
    	if (openi(filnam) == 0) 	/* Try to open the input file */
	  return(0);

#ifdef CK_RESEND
/*
  The following check is done after openi() is called, since openi() itself
  can change the transfer mode (as in VMS).
*/
        if ((binary == XYFT_T
#ifdef VMS
	     || binary == XYFT_L
#endif /* VMS */
	     ) && sendmode == SM_RESEND) {
            /* Trying to RESEND/REGET a file first sent in TEXT mode. */
	    debug(F111,"sfile error - Recover vs Text",filnam,binary);
            /* Set appropriate error messages and make log entries here */
#ifdef VMS
	    if (binary == XYFT_L)
	      ckmakmsg((char *)epktmsg,
		       PKTMSGLEN,
		       "Recovery attempted in LABELED mode: ",
		       filnam,
		       NULL,
		       NULL
		       );
	    else
#endif /* VMS */
	      ckmakmsg((char *)epktmsg,
		       PKTMSGLEN,
		       "Recovery attempted in TEXT mode: ",
		       filnam,
		       NULL,
		       NULL
		       );
            return(0);
        }
	if (sendmode == SM_PSEND)	/* PSENDing? */
	  if (sendstart > (CK_OFF_T)0)	/* Starting position */
	    if (zfseek(sendstart) < 0)	/* seek to it... */
	      return(0);
#endif /* CK_RESEND */
    	s = pktnam;			/* Name for packet data field */
#ifdef OS2
	/* Never send a disk letter. */
	if (isalpha(*s) && (*(s+1) == ':'))
	  s += 2;
#endif /* OS2 */

    } else {				/* X-packet setup, not F-packet. */
	binary = XYFT_T;		/* Text always */
    	debug(F110,"sfile X packet",cmdstr,0); /* Log debugging info */
    	s = cmdstr;			/* Name for data field */
    }

    /* Now s points to the string that goes in the packet data field. */

    debug(F101,"sfile binary","",binary); /* Log debugging info */
    encstr((CHAR *)s);			/* Encode the name. */
					/* Send the F or X packet */
    /* If the encoded string did not fit into the packet, it was truncated. */

    if (nxtpkt() < 0) return(0);	/* Bump packet number, get buffer */

    rc = spack((char) (x ? 'X' : 'F'), pktnum, size, data);
    if (rc < 0)
      return(rc);

#ifndef NOCSETS
    setxlatype(tcharset,fcharset);	/* Set up charset translations */
#endif /* NOCSETS */

    if (x == 0) {			/* Display for F packet */
    	if (displa) {			/* Screen */
	    xxscreen(SCR_FN,'F',(long)pktnum,filnam);
	    xxscreen(SCR_AN,0,0L,pktnam);
	    xxscreen(SCR_FS,0,calibrate ? calibrate : fsize,"");
    	}
#ifdef pdp11
    	tlog(F110,"Sending",filnam,0L); /* Transaction log entry */
	makestr(&psfspec,filnam);	/* New filename */
#else
#ifndef ZFNQFP
    	tlog(F110,"Sending",filnam,0L);
	makestr(&psfspec,filnam);	/* New filename */
#else
	if (notafile) {			/* If not a file log simple name */
	    tlog(F110,"Sending", filnam, 0L);
	} else {			/* Log fully qualified filename */
#ifdef COMMENT
	    /* This section generates bad code in SCO 3.2v5.0.5's cc */
	    char *p = NULL, *q = filnam;
	    debug(F101,"sfile CKMAXPATH","",CKMAXPATH);
	    if ((p = malloc(CKMAXPATH+1))) {
		debug(F111,"sfile calling zfnqfp",filnam,strlen(filnam));
		if (zfnqfp(filnam, CKMAXPATH, p)) {
		    debug(F111,"sfile zfnqfp ok",p,strlen(p));
		    q = p;
		}
	    }
#else
	    char tmpbuf[CKMAXPATH+1];
	    char *p = tmpbuf, *q = filnam;
	    if (zfnqfp(filnam, CKMAXPATH, p))
	      q = p;
#endif /* COMMENT */
	    debug(F111,"sfile q",q,strlen(q));
	    tlog(F110,"Sending",q,0L);
	    makestr(&psfspec,q);	/* New preliminary filename */
#ifdef COMMENT
	    if (p) free(p);
#endif /* COMMENT */
	}
#endif /* ZFNQFP */
#endif /* pdp11 */
    	tlog(F110," as",pktnam,0L);
	if (binary) {			/* Log file mode in transaction log */
	    tlog(F101," mode: binary","",(long) binary);
	} else {			/* If text mode, check character set */
	    tlog(F100," mode: text","",0L);
#ifndef NOCSETS
	    if (tcharset == TC_TRANSP || xfrxla == 0) {
		tlog(F110," character set","transparent",0L);
	    } else {
		tlog(F110," xfer character set",tcsinfo[tcharset].name,0L);
		tlog(F110," file character set",fcsinfo[fcharset].name,0L);
	    }
#endif /* NOCSETS */
	}
    } else {				/* Display for X-packet */

    	xxscreen(SCR_XD,'X',(long)pktnum,cmdstr); /* Screen */
    	tlog(F110,"Sending from:",cmdstr,0L);	/* Transaction log */
    }
    intmsg(++filcnt);			/* Count file, give interrupt msg */
    first = 1;				/* Init file character lookahead. */
    ffc = (CK_OFF_T)0;			/* Init file character counter. */
    cps = oldcps = 0L;			/* Init cps statistics */
    rejection = -1;
    fsecs = gtimer();			/* Time this file started */
#ifdef GFTIMER
    fpfsecs = gftimer();
    debug(F101,"SFILE fpfsecs","",fpfsecs);
#endif /* GFTIMER */
    debug(F101,"SFILE fsecs","",fsecs);
    return(1);
}

/*  S D A T A -- Send a data packet */

/*
  Returns -1 if no data to send (end of file), -2 if connection is broken.
  If there is data, a data packet is sent, and sdata() returns 1.

  In the streaming case, the window is regarded as infinite and we keep
  sending data packets until EOF or there appears to be a Kermit packet on the
  reverse channel.  When not streaming and the window size is greater than 1,
  we keep sending data packets until window is full or characters start to
  appear from the other Kermit.

  In the windowing or streaming case, when there is no more data left to send
  (or when sending has been interrupted), sdata() does nothing and returns 0
  each time it is called until the acknowledgement sequence number catches up
  to the last data packet that was sent.
*/
int
sdata() {
    int i, x, len;
    char * s;

    debug(F101,"sdata entry, first","",first);
    debug(F101,"sdata drain","",drain);
/*
  The "drain" flag is used with window size > 1.  It means we have sent
  our last data packet.  If called and drain is not zero, then we return
  0 as if we had sent an empty data packet, until all data packets have
  been ACK'd, then then we can finally return -1 indicating EOF, so that
  the protocol can switch to seof state.  This is a kludge, but at least
  it's localized...
*/
    if (first == 1) drain = 0;		/* Start of file, init drain flag. */

    if (drain) {			/* If draining... */
	debug(F101,"sdata draining, winlo","",winlo);
	if (winlo == pktnum)		/* If all data packets are ACK'd */
	  return(-1);			/* return EOF indication */
	else				/* otherwise */
	  return(0);			/* pretend we sent a data packet. */
    }
    debug(F101,"sdata sbufnum","",sbufnum);
    for (i = sbufnum;
	 i > 0
#ifdef STREAMING
	 || streaming
#endif /* STREAMING */
	 ;
	 i--) {
	if (i < 0)
	  break;
        debug(F101,"sdata countdown","",i);
#ifdef STREAMING
	if (streaming) {
	    pktnum = (pktnum + 1) % 64;
	    winlo = pktnum;
	    debug(F101,"sdata streaming pktnum","",pktnum);
	} else {
#endif /* STREAMING */
	    x = nxtpkt();		/* Get next pkt number and buffer */
	    debug(F101,"sdata nxtpkt pktnum","",pktnum);
	    if (x < 0) return(0);
#ifdef STREAMING
	}
#endif /* STREAMING */
	debug(F101,"sdata packet","",pktnum);
	if (chkint() < 0)		/* Especially important if streaming */
	  return(-9);
	if (cxseen || czseen) {		/* If interrupted, done. */
	    if (wslots > 1) {
		drain = 1;
		debug(F100,"sdata cx/zseen windowing","",0);
		return(0);
	    } else {
		debug(F100,"sdata cx/zseen nonwindowing","",0);
		return(-1);
	    }
	}
#ifdef DEBUG
	if (deblog) {
	    debug(F101,"sdata spsiz","",spsiz);
	    debug(F101,"sdata binary","",binary);
	    debug(F101,"sdata parity","",parity);
	}
#endif /* DEBUG */
#ifdef CKTUNING
	if (binary && !parity && !memstr && !funcstr)
	  len = bgetpkt(spsiz);
	else
	  len = getpkt(spsiz,1);
#else
	len = getpkt(spsiz,1);
#endif /* CKTUNING */
	s = (char *)data;
	if (len == -3) {		/* Timed out (e.g.reading from pipe) */
	    s = "";			/* Send an empty data packet. */
	    len = 0;
	} else if (len == 0) {		/* Done if no data. */
	    if (pktnum == winlo)
	      return(-1);
	    drain = 1;			/* But can't return -1 until all */
	    debug(F101,"sdata eof, drain","",drain);
	    return(0);			/* ACKs are drained. */
	}
	debug(F101,"sdata pktnum","",pktnum);
	debug(F101,"sdata len","",len);
	debug(F011,"sdata data",data,52);

	x = spack('D',pktnum,len,(CHAR *)s); /* Send the data packet. */
	debug(F101,"sdata spack","",x);
	if (x < 0) {			/* Error */
	    ttchk();			/* See if connection is still there */
	    return(-2);
	}
#ifdef STREAMING
	if (streaming)			/* What an ACK would do. */
	  winlo = pktnum;
#endif /* STREAMING */
	x = ttchk();			/* Peek at input buffer. */
	debug(F101,"sdata ttchk","",x);	/* ACKs waiting, maybe?  */
	if (x < 0)			/* Or connection broken? */
	  return(-2);
/*
  Here we check to see if any ACKs or NAKs have arrived, in which case we
  break out of the D-packet-sending loop and return to the state switcher
  to process them.  This is what makes our windows slide instead of lurch.
*/
	if (
#ifdef GEMDOS
/*
  In the Atari ST version, ttchk() can only return 0 or 1.  But note: x will
  probably always be > 0, since the as-yet-unread packet terminator from the
  last packet is probably still in the buffer, so sliding windows will
  probably never happen when the Atari ST is the file sender.  The alternative
  is to say "if (0)", in which case the ST will always send a window full of
  packets before reading any ACKs or NAKs.
*/
	    x > 0

#else /* !GEMDOS */
/*
  In most other versions, ttchk() returns the actual count.
  It can't be a Kermit packet if it's less than five bytes long.
*/
	    x > 4 + bctu

#endif /* GEMDOS */
	    )
	  return(1);			/* Yes, stop sending data packets */
    }					/* and go try to read the ACKs. */
    return(1);
}


/*  S E O F -- Send an End-Of-File packet */

/*  Call with a string pointer to character to put in the data field, */
/*  or else a null pointer or "" for no data.  */

/*
  There are two "send-eof" functions.  seof() is used to send the normal eof
  packet at the end of a file's data (even if the file has no data), or when
  a file transfer is interrupted.  sxeof() is used to send an EOF packet that
  occurs because of attribute refusal or interruption prior to entering data
  state.  The difference is purely a matter of buffer allocation and packet
  sequence number management.  Both functions act as "front ends" to the
  common send-eof function, szeof().
*/

/* Code common to both seof() and sxeof() */

int
szeof(s) CHAR *s; {
    int x;
    lsstate = 0;			/* Cancel locking-shift state */
    if (!s) s = (CHAR *)"";
    debug(F111,"szeof",s,pktnum);
    if (*s) {
	x = spack('Z',pktnum,1,s);
	xitsta |= W_SEND;
#ifdef COMMENT
	tlog(F100," *** interrupted, sending discard request","",0L);
#endif /* COMMENT */
	filrej++;
    } else {
	x = spack('Z',pktnum,0,(CHAR *)"");
    }
    if (x < 0)
      return(x);

#ifdef COMMENT
    /* No, too soon */
    discard = 0;			/* Turn off per-file discard flag */
#endif /* COMMENT */

/* If we were sending from a pipe, we're not any more... */
    pipesend = 0;
    return(0);
}

int
seof(x) int x; {
    char * s;
/*
  ckcpro.w, before calling seof(), sets window size back to 1 and then calls
  window(), which clears out the old buffers.  This is OK because the final
  data packet for the file has been ACK'd.  However, sdata() has already
  called nxtpkt(), which set the new value of pktnum which seof() will use.
  So all we need to do here is is allocate a new send-buffer.
*/
    s = x ? "D" : "";
    debug(F111,"seof",s,pktnum);
    if (getsbuf(pktnum) < 0) {	/* Get a buffer for packet n */
	debug(F101,"seof can't get s-buffer","",pktnum);
	return(-1);
    }
    return(szeof((CHAR *)s));
}

/*
  Version of seof() to be called when sdata() has not been called before.  The
  difference is that this version calls nxtpkt() to allocate a send-buffer and
  get the next packet number.
*/
int
sxeof(x) int x; {
    char * s;
    s = x ? "D" : "";
    if (nxtpkt() < 0)			/* Get next pkt number and buffer */
      debug(F101,"sxeof nxtpkt fails","",pktnum);
    else
      debug(F101,"sxeof packet","",pktnum);
    return(szeof((CHAR *)s));
}

/*  S E O T -- Send an End-Of-Transaction packet */

int
seot() {
    int x;
    x = nxtpkt();
    debug(F101,"seot nxtpkt","",x);
    if (x < 0) return(-1);		/* Bump packet number, get buffer */
    x = spack('B',pktnum,0,(CHAR *)"");	/* Send the EOT packet */
    if (x < 0)
      return(x);
    cxseen = czseen = discard = 0;	/* Reset interruption flags */
    tstats();				/* Log timing info */
    return(0);
}


/*   R P A R -- Fill the data array with my send-init parameters  */

int q8flag = 0;

CHAR dada[32];				/* Use this instead of data[]. */
					/* To avoid some kind of wierd */
					/* addressing foulup in spack()... */
					/* (which might be fixed now...) */

CHAR *
rpar() {
    char *p;
    int i, x, max;
    extern int sprmlen;

    max = maxdata();			/* Biggest data field I can send */
    debug(F101, "rpar max 1","",max);
    debug(F101, "rpar sprmlen","",sprmlen);
    if (sprmlen > 1 && sprmlen < max)	/* User override */
      max = sprmlen;
    debug(F101, "rpar max 2","",max);

    if (rpsiz > MAXPACK)		/* Biggest normal packet I want. */
      dada[0] = (char) tochar(MAXPACK);	/* If > 94, use 94, but specify */
    else				/* extended packet length below... */
      dada[0] = (char) tochar(rpsiz);	/* else use what the user said. */
    dada[1] = (char) tochar(chktimo(pkttim,0)); /* When to time me out */
    dada[2] = (char) tochar(mypadn);	/* How much padding I need (none) */
    dada[3] = (char) ctl(mypadc);	/* Padding character I want */
    dada[4] = (char) tochar(eol);	/* End-Of-Line character I want */
    dada[5] = myctlq;			/* Control-Quote character I send */

    if (max < 6) { dada[6] = NUL; rqf = 0; ebq = sq = NUL; return(dada); }

    switch (rqf) {			/* 8th-bit prefix (single-shift) */
      case -1:				/* I'm opening the bids */
      case  1:				/* Other Kermit already bid 'Y' */
	if (parity) ebq = sq = MYEBQ;	/* So I reply with '&' if parity */
	break;				/*  otherwise with 'Y'. */
      case  2:				/* Other Kermit sent a valid prefix */
	if (q8flag)
	  sq = ebq;			/* Fall through on purpose */
      case  0:				/* Other Kermit bid nothing */
	break;				/* So I reply with 'Y'. */
    }
    debug(F000,"rpar 8bq sq","",sq);
    debug(F000,"rpar 8bq ebq","",ebq);
    if (lscapu == 2)			/* LOCKING-SHIFT FORCED */
      dada[6] = 'N';			/* requires no single-shift */
    else				/* otherwise send prefix or 'Y' */
      dada[6] = (char) sq;
    ebqsent = dada[6];			/* And remember what I really sent */

    if (max < 7) { dada[7] = NUL; bctr = 1; return(dada); }

    dada[7] = (char) (bctr == 4) ? 'B' : bctr + '0'; /* Block check type */

    if (max < 8) { dada[8] = NUL; rptflg = 0; return(dada); }

    if (rptena) {
	if (rptflg)			/* Run length encoding */
	  dada[8] = (char) rptq;	/* If receiving, agree */
	else				/* by replying with same character. */
	  dada[8] = (char) (rptq = myrptq); /* When sending use this. */
    } else dada[8] = SP;		/* Not enabled, put a space here. */

    /* CAPAS mask */

    if (max < 9) {
	dada[9] = NUL;
	atcapr = 0;
	lpcapr = 0;
	swcapr = 0;
	rscapr = 0;
	return(dada);
    }
    dada[9] = (char) tochar((lscapr ? lscapb : 0) | /* Locking shifts */
		     (atcapr ? atcapb : 0) | /* Attribute packets */
		     (lpcapr ? lpcapb : 0) | /* Long packets */
		     (swcapr ? swcapb : 0) | /* Sliding windows */
		     (rscapr ? rscapb : 0)); /* RESEND */
    if (max < 10) { wslotr = 1; return(dada); }
    dada[10] = (char) tochar(swcapr ? wslotr : 1); /* CAPAS+1 = Window size */

    if (max < 12) { rpsiz = 80; return(dada); }
    if (urpsiz > 94)
      rpsiz = urpsiz - 1;		/* Long packets ... */
    dada[11] = (char) tochar(rpsiz / 95); /* Long packet size, big part */
    dada[12] = (char) tochar(rpsiz % 95); /* Long packet size, little part */

    if (max < 16) return(dada);
    dada[13] = '0';			/* CAPAS+4 = WONT CHKPNT */
    dada[14] = '_';			/* CAPAS+5 = CHKINT (reserved) */
    dada[15] = '_';			/* CAPAS+6 = CHKINT (reserved) */
    dada[16] = '_';			/* CAPAS+7 = CHKINT (reserved) */
    if (max < 17) return(dada);
#ifndef WHATAMI
    dada[17] = ' ';
#else
    x = 0;
    if (server) x |= WMI_SERVE;		/* Whether I am a server */
    if (binary) x |= WMI_FMODE;		/* My file transfer mode is ... */
    if (fncnv)  x |= WMI_FNAME;		/* My filename conversion is ... */
#ifdef STREAMING
    if (streamrq == SET_ON)
      x |= WMI_STREAM;
    else if (streamrq == SET_AUTO && reliable == SET_ON)
      x |= WMI_STREAM;
    /*
      Always offer to stream when in remote mode and STREAMING is AUTO
      and RELIABLE is not OFF (i.e. is ON or AUTO).
    */
    else if (!local && streamrq == SET_AUTO && reliable != SET_OFF)
      x |= WMI_STREAM;
#endif /* STREAMING */
#ifdef TCPSOCKET
    if (clearrq == SET_ON)
      x |= WMI_CLEAR;
    else if (clearrq == SET_AUTO &&	/* SET CLEAR-CHANNEL AUTO */
	     ((network && nettype == NET_TCPB /* TCP/IP */
#ifdef RLOGCODE
                && ttnproto != NP_RLOGIN/* Rlogin is not clear */
                && !(ttnproto >= NP_K4LOGIN && ttnproto <= NP_EK5LOGIN)
#endif /* RLOGCODE */
	       )
#ifdef SSHBUILTIN
              || (network && nettype == NET_SSH)
#endif /* SSHBUILTIN */
#ifdef IKSD
	      || inserver		/* We are IKSD */
#endif /* IKSD */
	      ))
      x |= WMI_CLEAR;
#endif /* TCPSOCKET */
    x |= WMI_FLAG;
    dada[17] = (char) tochar(x);
#endif /* WHATAMI */
    i = 18;				/* Position of next field */
    p = cksysid;			/* WHOAMI (my system ID) */
    x = strlen(p);
    if (max - i < x + 1) return(dada);
    if (x > 0) {
	dada[i++] = (char) tochar(x);
	while (*p)
	  dada[i++] = *p++;
    }

    if (max < i+1) return(dada);
#ifndef WHATAMI				/* WHATAMI2 */
    dada[i++] = ' ';
#else
    debug(F101,"rpar xfermode","",xfermode);
    x = WMI2_FLAG;			/* Is-Valid flag */
    if (xfermode != XMODE_A)		/* If TRANSFER MODE is MANUAL */
      x |= WMI2_XMODE;			/* set the XFERMODE bit */
    if (recursive > 0)			/* If this is a recursive transfer */
      x |= WMI2_RECU;			/* set the RECURSIVE bit */
    dada[i++] = tochar(x);
    debug(F101,"rpar whatami2","",x);
#endif /* WHATAMI */

    dada[i] = '\0';			/* Terminate the init string */

#ifdef DEBUG
    if (deblog) {
	debug(F110,"rpar",dada,0);
	rdebu(dada,(int)strlen((char *)dada));
    }
#endif /* DEBUG */
    ckstrncpy((char *)myinit,(char *)dada,MYINITLEN);
    return(dada);			/* Return pointer to string. */
}

int
spar(s) CHAR *s; {			/* Set parameters */
    int x, y, lpsiz, biggest;
    extern int rprmlen, lastspmax;
    extern struct sysdata sysidlist[];

    whatru = 0;
    whoareu[0] = NUL;
#ifdef STREAMING
    streamok = 0;
    streaming = 0;
#endif /* STREAMING */
    biggest = rln;

    debug(F101, "spar biggest 1","",biggest);
    debug(F101, "spar rprmlen","",rprmlen);
    if (rprmlen > 1 && rprmlen < biggest)
      biggest = rprmlen;
    debug(F101, "rpar biggest 2","",biggest);
    debug(F110,"spar packet",s,0);

    s--;				/* Line up with field numbers. */

/* Limit on size of outbound packets */
    x = (biggest >= 1) ? xunchar(s[1]) : 80;
    lpsiz = spsizr;			/* Remember what they SET. */
    if (spsizf) {			/* SET-command override? */
	if (x < spsizr) spsiz = x;	/* Ignore LEN unless smaller */
    } else {				/* otherwise */
	spsiz = (x < 10) ? 80 : x;	/* believe them if reasonable */
    }
    spmax = spsiz;			/* Remember maximum size */

/* Timeout on inbound packets */
    if (timef) {
	timint = rtimo;			/* SET SEND TIMEOUT value overrides */
    } else {				/* Otherwise use requested value, */
	x = (biggest >= 2) ? xunchar(s[2]) : rtimo; /* if it is legal. */
	timint = (x < 0) ? rtimo : x;
    }
    timint = chktimo(timint,timef);	/* Adjust if necessary */

/* Outbound Padding */
    npad = 0; padch = '\0';
    if (biggest >= 3) {
	npad = xunchar(s[3]);
	if (biggest >= 4) padch = (CHAR) ctl(s[4]); else padch = 0;
    }
    if (npad) {
	int i;
	for (i = 0; i < npad; i++) padbuf[i] = dopar(padch);
    }

/* Outbound Packet Terminator */
    seol = (CHAR) (biggest >= 5) ? xunchar(s[5]) : CR;
    if ((seol < 1) || (seol > 31)) seol = CR;

/* Control prefix that the other Kermit is sending */
    x = (biggest >= 6) ? s[6] : '#';
    ctlq = (CHAR) (((x > 32 && x < 63) || (x > 95 && x < 127)) ? x : '#');

/* 8th-bit prefix */
/*
  NOTE: Maybe this could be simplified using rcvtyp.
  If rcvtyp == 'Y' then we're reading the ACK,
  otherwise we're reading the other Kermit's initial bid.
  But his horrendous code has been working OK for years, so...
*/
    rq = (biggest >= 7) ? s[7] : 0;
    if (rq == 'Y') rqf = 1;
      else if ((rq > 32 && rq < 63) || (rq > 95 && rq < 127)) rqf = 2;
        else rqf = 0;
    debug(F000,"spar 8bq rq","",rq);
    debug(F000,"spar 8bq sq","",sq);
    debug(F000,"spar 8bq ebq","",ebq);
    debug(F101,"spar 8bq rqf","",rqf);
    switch (rqf) {
      case 0:				/* Field is missing from packet. */
	ebqflg = 0;			/* So no 8th-bit prefixing. */
	break;
      case 1:				/* Other Kermit sent 'Y' = Will Do. */
	/*
          When I am the file receiver, ebqsent is 0 because I didn't send a
          negotiation yet.  If my parity is set to anything other than NONE,
          either because my user SET PARITY or because I detected parity bits
          on this packet, I reply with '&', otherwise 'Y'.

	  When I am the file sender, ebqsent is what I just sent in rpar(),
          which can be 'Y', 'N', or '&'.  If I sent '&', then this 'Y' means
          the other Kermit agrees to do 8th-bit prefixing.

          If I sent 'Y' or 'N', but then detected parity on the ACK packet
          that came back, then it's too late: there is no longer any way for
          me to tell the other Kermit that I want to do 8th-bit prefixing, so
          I must not do it, and in that case, if there is any 8-bit data in
          the file to be transferred, the transfer will fail because of block
          check errors.

          The following clause covers all of these situations:
	*/
	if (parity && (ebqsent == 0 || ebqsent == '&')) {
	    ebqflg = 1;
	    ebq = MYEBQ;
	}
	break;
      case 2:				/* Other Kermit sent a valid prefix */
	ebqflg = (ebq == sq || sq == 'Y');
	if (ebqflg) {
	    ebq = rq;
	    debug(F101,"spar setting parity to space","",ebq);
	    if (!parity) parity = ttprty = 's';
	}
    }
    if (lscapu == 2) {     /* But no single-shifts if LOCKING-SHIFT FORCED */
	ebqflg = 0;
	ebq = 'N';
    }

/* Block check */
    x = 1;
    if (biggest >= 8) {
	if (s[8] == 'B') x = 4;
	else x = s[8] - '0';
	if ((x < 1) || (x > 5)) x = 1;	/* "5" 20110605 */
    }
    bctr = x;

/* Repeat prefix */

    rptflg = 0;				/* Assume no repeat-counts */
    if (biggest >= 9) {			/* Is there a repeat-count field? */
	char t;				/* Yes. */
	t = s[9];			/* Get its contents. */
/*
  If I'm sending files, then I'm reading these parameters from an ACK, and so
  this character must agree with what I sent.
*/
	if (rptena) {			/* If enabled ... */
	    if ((char) rcvtyp == 'Y') {	/* Sending files, reading ACK. */
		if (t == myrptq) rptflg = 1;
	    } else {			/* I'm receiving files */
		if ((t > 32 && t < 63) || (t > 95 && t < 127)) {
		    rptflg = 1;
		    rptq = t;
		}
	    }
	} else rptflg = 0;
    }

/* Capabilities */

    atcapu = lpcapu = swcapu = rscapu = 0; /* Assume none of these. */
    if (lscapu != 2) lscapu = 0;	/* Assume no LS unless forced. */
    y = 11;				/* Position of next field, if any */
    if (biggest >= 10) {
        x = xunchar(s[10]);
	debug(F101,"spar capas","",x);
        atcapu = (x & atcapb) && atcapr; /* Attributes */
	lpcapu = (x & lpcapb) && lpcapr; /* Long packets */
	swcapu = (x & swcapb) && swcapr; /* Sliding windows */
	rscapu = (x & rscapb) && rscapr; /* RESEND */
	debug(F101,"spar lscapu","",lscapu);
	debug(F101,"spar lscapr","",lscapr);
	debug(F101,"spar ebqflg","",ebqflg);
	if (lscapu != 2) lscapu = ((x & lscapb) && lscapr && ebqflg) ? 1 : 0;
	debug(F101,"spar swcapr","",swcapr);
	debug(F101,"spar swcapu","",swcapu);
	debug(F101,"spar lscapu","",lscapu);
	for (y = 10; (xunchar(s[y]) & 1) && (biggest >= y); y++);
	debug(F101,"spar y","",y);
    }

/* Long Packets */
    debug(F101,"spar lpcapu","",lpcapu);
    if (lpcapu) {
        if (biggest > y+1) {
	    x = xunchar(s[y+2]) * 95 + xunchar(s[y+3]);
	    debug(F101,"spar lp len","",x);
	    if (spsizf) {		/* If overriding negotiations */
		spsiz = (x < lpsiz) ? x : lpsiz; /* do this, */
	    } else {			         /* otherwise */
		spsiz = (x > MAXSP) ? MAXSP : x; /* do this. */
	    }
	    if (spsiz < 10) spsiz = 80;	/* Be defensive... */
	}
    }
    /* (PWP) save current send packet size for optimal packet size calcs */
    spmax = spsiz;			/* Maximum negotiated length */
    lastspmax = spsiz;			/* For stats */
    if (slostart && spsiz > 499)	/* Slow start length */
      spsiz = 244;
    debug(F101,"spar slow-start spsiz","",spsiz);
    debug(F101,"spar lp spmax","",spmax);
    timint = chktimo(timint,timef);	/* Recalculate the packet timeout */

/* Sliding Windows... */

    if (swcapr) {			/* Only if requested... */
        if (biggest > y) {		/* See what other Kermit says */
	    x = xunchar(s[y+1]);
	    debug(F101,"spar window","",x);
	    wslotn = (x > MAXWS) ? MAXWS : x;
/*
  wslotn = negotiated size (from other Kermit's S or I packet).
  wslotr = requested window size (from this Kermit's SET WINDOW command).
*/
	    if (wslotn > wslotr)	/* Use the smaller of the two */
	      wslotn = wslotr;
	    if (wslotn < 1)		/* Watch out for bad negotiation */
	      wslotn = 1;
	    if (wslotn > 1) {
		swcapu = 1;		/* We do windows... */
		if (wslotn > maxtry)	/* Retry limit must be greater */
		  maxtry = wslotn + 1;	/* than window size. */
	    }
	    debug(F101,"spar window after adjustment","",x);
	} else {			/* No window size specified. */
	    wslotn = 1;			/* We don't do windows... */
	    debug(F101,"spar window","",x);
	    swcapu = 0;
	    debug(F101,"spar no windows","",wslotn);
	}
    }

/* Now recalculate packet length based on number of windows.   */
/* The nogotiated number of window slots will be allocated,    */
/* and the maximum packet length will be reduced if necessary, */
/* so that a windowful of packets can fit in the big buffer.   */

    if (wslotn > 1) {			/* Shrink to fit... */
	x = adjpkl(spmax,wslotn,bigsbsiz);
	if (x < spmax) {
	    spmax = x;
	    lastspmax = spsiz;
	    if (slostart && spsiz > 499) spsiz = 244; /* Slow start again */
	    debug(F101,"spar sending, redefine spmax","",spmax);
	}
    }
#ifdef WHATAMI
    debug(F101,"spar biggest","",biggest);
    if (biggest > y+7) {		/* Get WHATAMI info if any */
	whatru = xunchar(s[y+8]);
	debug(F101,"spar whatru","",whatru);
    }
    if (whatru & WMI_FLAG) {		/* Only valid if this bit is set */
#ifdef STREAMING
	if (whatru & WMI_STREAM) {
	    if (streamrq == SET_ON ||
		(streamrq == SET_AUTO &&
		 (reliable == SET_ON || (reliable == SET_AUTO && !local)
#ifdef TN_COMPORT
                  && !istncomport()
#endif /* TN_COMPORT */
#ifdef IKSD
                   || inserver
#endif /* IKSD */
                   ))) {
		streamok = 1;		/* Streaming negotiated */
		slostart = 0;		/* Undo slow-start machinations */
		spsiz = lastspmax;
	    }
	}
	streamed = streamok;
	debug(F101,"spar streamok","",streamok);
	debug(F101,"spar clearrq","",clearrq);
	if (clearrq == SET_ON ||
             (clearrq == SET_AUTO &&
               ((network && nettype == NET_TCPB
#ifdef RLOGCODE
                && ttnproto != NP_RLOGIN/* Rlogin is not clear */
                && !(ttnproto >= NP_K4LOGIN && ttnproto <= NP_EK5LOGIN)
#endif /* RLOGCODE */
#ifdef TN_COMPORT
                && !istncomport()
#endif /* TN_COMPORT */
                  )
#ifdef SSHBUILTIN
                 || (network && nettype == NET_SSH)
#endif /* SSHBUILTIN */
#ifdef IKSD
                 || inserver
#endif /* IKSD */
                 )))
          urclear = (whatru & WMI_CLEAR);
	debug(F101,"spar urclear","",urclear);
#ifdef CK_SPEED
	if (urclear)
	  setprefix(PX_NON);
#endif /* CK_SPEED */
	cleared = urclear;
#endif /* STREAMING */
    }
#endif /* WHATAMI */

    if (biggest > y+8) {		/* Get WHOAREYOU info if any */
	int x, z;
	x = xunchar(s[y+9]);		/* Length of it */
	z = y;
	y += (9 + x);
	debug(F101,"spar sysindex x","",x);
	debug(F101,"spar sysindex y","",y);
	debug(F101,"spar sysindex biggest","",biggest);

	if (x > 0 && x < 16 && biggest >= y) {
	    strncpy(whoareu,(char *)s+z+10,x); /* Other Kermit's system ID */
	    debug(F111,"spar whoareyou",whoareu,whoareu[0]);
	    if (whoareu[0]) {		/* Got one? */
		sysindex = getsysix((char *)whoareu);
		debug(F101,"spar sysindex",whoareu,sysindex);
	    }
	}
    } else
      goto xspar;

#ifdef WHATAMI
    y++;				/* Advance pointer */
    if (biggest >= y) {
	whatru2 = xunchar(s[y]);	/* Next field is WHATAMI2 */
	debug(F101,"spar whatru2","",whatru2);
	if (whatru2 & WMI2_FLAG) {	/* Valid only if this bit is set */
	    if (server) {		/* Server obeys client's xfer mode */
		xfermode = (whatru2 & WMI2_XMODE) ? XMODE_M : XMODE_A;
		debug(F101,"spar whatru2 xfermode","",xfermode);
	    }
	    if (whatru2 & WMI2_RECU) {	/* RECURSIVE transfer */
		if (fnrpath == PATH_AUTO) { /* If REC PATH AUTO */
		    fnrpath = PATH_REL;	/* Set it to RELATIVE */
		    autopath = 1;	/* and remember we did this */
		}
	    }
	}
    }
#endif /* WHATAMI */

  xspar:
    if (sysindex > -1) {
	char * p;
	p = sysidlist[sysindex].sid_name;
	tlog(F110,"Remote system type: ",p,0L);
	if (sysindex > 0) {		/* If partnet's system type known */
	    whoarewe();			/* see if we are a match. */
#ifdef CK_SPEED
/* Never unprefix XON and XOFF when sending to VMS */
	    debug(F111,"proto whoareu",whoareu,sysindex);
	    if (!strcmp((char *)whoareu,"D7")) {
		debug(F111,"proto special VMS prefixing","",0);
		ctlp[XON] = ctlp[XOFF] = 1;
		ctlp[XON+128] = ctlp[XOFF+128] = 1;
		ctlp[3] = 1;		/* Ctrl-C might be dangerous too */
		ctlp[14] = ctlp[15] = 1; /* And SO/SI */
		ctlp[24] = ctlp[25] = 1; /* And ^X/^Y */
		ctlp[141] = 1;		/* And CR+128 */
	    }
#endif /* CK_SPEED */
	}
    }

/* Record parameters in debug log */
#ifdef DEBUG
    if (deblog) sdebu(biggest);
#endif /* DEBUG */
    numerrs = 0;			/* Start counting errors here. */
    return(0);
}

/*  G N F I L E  --  Get name of next file to send  */
/*
  Expects global sndsrc to be:
   -9: if we are generating a file internally for calibration.
   -1: next filename to be obtained by calling znext().
    0: no next file name
    1: (or greater) next filename to be obtained from **cmlist,
       or if addlist != 0, from the "filehead" linked list,
       or if filefile pointer not null from that file (which is already open).
  Returns:
    1, with name of next file in filnam.
    0, no more files, with filnam set to empty string.
   -1, file not found
   -2, file is not readable (but then we just skip to the next one if any)
   -3, read access denied
   -4, cancelled
   -5, too many files match wildcard
   -6, no files selected
  NOTE:
    If gnfile() returns 0, then the global variable gnferror should be checked
    to find out the most recent gnfile() error, and use that instead of the
    return code (for reasons to hard to explain).
*/
int
gnfile() {
    int i = 0, x = 0;
    CK_OFF_T filesize = 0;
    int dodirstoo = 0;
    int retcode = 0;

    char fullname[CKMAXPATH+1];

    dodirstoo = ((what & (W_FTP|W_SEND)) == (W_FTP|W_SEND)) && recursive;

    debug(F101,"gnfile sndsrc","",sndsrc);
    debug(F101,"gnfile filcnt","",filcnt);
    debug(F101,"gnfile what","",what);
    debug(F101,"gnfile recursive","",recursive);
    debug(F101,"gnfile dodirstoo","",dodirstoo);
    gnferror = 0;
    fsize = (CK_OFF_T)-1;		/* Initialize file size */
    fullname[0] = NUL;

#ifdef VMS
    /* 
      In VMS, zopeni() sets binary 0/1 automatically from the file
      attributes.  Don't undo it here.
    */
    debug(F101,"gnfile VMS binary","",binary);
#else  /* VMS */
    if (!(what & W_REMO) && (xfermode == XMODE_A)
#ifndef NOMSEND
	&& !addlist
#endif /* NOMSEND */
	) {
	extern int stdinf;
	if (!stdinf)			/* Not if sending from stdin */
#ifdef WHATAMI
	  /* We don't do this in server mode because it undoes WHATAMI */
	  if (!server || (server && ((whatru & WMI_FLAG) == 0)))
#endif /* WHATAMI */
	    binary = gnf_binary;	/* Restore prevailing transfer mode */
	debug(F101,"gnfile binary = gnf_binary","",gnf_binary);
    }
#endif	/* VMS */

#ifdef PIPESEND
    debug(F101,"gnfile pipesend","",pipesend);
    if (pipesend) {			/* First one */
	if (filcnt == 0) {
	    ckstrncpy(filnam,cmarg,CKMAXPATH+1);
	    return(1);
	} else {			/* There's only one... */
	    *filnam = NUL;
	    pipesend = 0;
	    return(0);
	}
    }
#endif /* PIPESEND */

#ifdef CALIBRATE
    if (sndsrc == -9) {
	debug(F100,"gnfile calibrate","",0);
	ckstrncpy(filnam,"CALIBRATION",CKMAXPATH);
	nfils = 0;
	cal_j = 0;
	fsize = calibrate;
	sndsrc = 0;			/* For next time */
	nfils = 0;
	return(1);
    }
#endif /* CALIBRATE */

#ifndef NOSPL
    if (sndarray) {			/* Sending from an array */
	extern char sndxnam[];		/* Pseudo filename */
	debug(F100,"gnfile array","",0);
	nfils = 0;
	fsize = (CK_OFF_T)-1;		/* Size unknown */
	sndsrc = 0;
	ckstrncpy(filnam,sndxnam,CKMAXPATH);
	return(1);
    }
#endif /* NOSPL */

    if (sndsrc == 0) {			/* It's not really a file */
	if (nfils > 0) {		/* It's a pipe, or stdin */
	    ckstrncpy(filnam,*cmlist,CKMAXPATH+1); /* Copy its "name" */
	    nfils = 0;			/* There is no next file */
	    return(1);			/* OK this time */
	} else return(0);		/* but not next time */
    }

/* If file group interruption (C-Z) occurred, fail.  */

    if (czseen) {
	tlog(F100,"Transaction cancelled","",0L);
        debug(F100,"gnfile czseen","",0);
	return(-4);
    }

/* Loop through file list till we find a readable, sendable file */

    filesize = (CK_OFF_T)-1;		/* Loop exit (file size) variable */
    while (filesize < 0) {		/* Keep trying till we get one... */
	retcode = 0;
	if (sndsrc > 0) {		/* File list in cmlist or file */
	    if (filefile) {		/* Reading list from file... */
		if (zsinl(ZMFILE,filnam,CKMAXPATH) < 0) { /* Read a line */
		    zclose(ZMFILE);	                  /* Failed */
		    debug(F110,"gnfile filefile EOF",filefile,0);
		    makestr(&filefile,NULL);
		    return(0);
		}
		debug(F110,"gnfile filefile filnam",filnam,0);
	    }
	    debug(F101,"gnfile nfils","",nfils);
	    if (nfils-- > 0 || filefile) { /* Still some left? */
#ifndef NOMSEND
		if (addlist) {
		    if (filenext && filenext->fl_name) {
			ckstrncpy(filnam,filenext->fl_name,CKMAXPATH+1);
			cmarg2 =
			  filenext->fl_alias ?
			    filenext->fl_alias :
			      "";
			binary = filenext->fl_mode;
		    } else {
			printf("?Internal error expanding ADD list\n");
			return(-5);
		    }
		    filenext = filenext->fl_next;
		    debug(F111,"gnfile addlist filnam",filnam,nfils);
		} else if (sndsrc > 0 && !filefile) {
#endif /* NOMSEND */
		    ckstrncpy(filnam,*cmlist++,CKMAXPATH+1);
		    debug(F111,"gnfile cmlist filnam",filnam,nfils);
#ifndef NOMSEND
		}
#endif /* NOMSEND */
		i = 0;
#ifndef NOSERVER
		debug(F101,"gnfile ngetpath","",ngetpath);
#endif /* NOSERVER */
nextinpath:
#ifndef NOSERVER
		fromgetpath = 0;
		if (server && !isabsolute(filnam) && (ngetpath > i)) {
		    ckstrncpy(fullname,getpath[i],CKMAXPATH+1);
		    ckstrncat(fullname,filnam,CKMAXPATH);
		    debug(F111,"gnfile getpath",fullname,i);
		    fromgetpath = 1;
		    i++;
		} else {
		    i = ngetpath + 1;
#else
		    i = 1;		/* ? */
#endif /* NOSERVER */
		    ckstrncpy(fullname,filnam,CKMAXPATH+1);
		    debug(F110,"gnfile absolute",fullname,0);
#ifndef NOSERVER
		}
#endif /* NOSERVER */
		if (iswild(fullname)
#ifdef RECURSIVE
		    || recursive > 0 || !strcmp(fullname,".")
#endif /* RECURSIVE */
		    ) {	/* It looks wild... */
		    /* First check if a file with this name exists */
		    debug(F110,"gnfile wild",fullname,0);
		    if (zchki(fullname) >= 0) {
			/*
			   Here we have a file whose name actually
			   contains wildcard characters.
			*/
			goto gotnam;
		    }
#ifdef COMMENT
		    nzxopts = ZX_FILONLY; /* (was 0: 25 Jul 2001 fdc) */
#else
		    nzxopts = recursive ? 0 : ZX_FILONLY; /* 30 Jul 2001 */
#endif /* COMMENT */
		    if (nolinks) nzxopts |= ZX_NOLINKS;	/* (26 Jul 2001 fdc) */
#ifdef UNIXOROSK
		    if (matchdot) nzxopts |= ZX_MATCHDOT;
#endif /* UNIXOROSK */
		    if (recursive) nzxopts |= ZX_RECURSE;
		    x = nzxpand(fullname,nzxopts); /* Expand wildcards */
		    debug(F101,"gnfile nzxpand","",x);
		    if (x == 1) {
			int xx;
			xx = znext(fullname);
			debug(F111,"gnfile znext A",fullname,xx);
			goto gotnam;
		    }
		    if (x == 0) {	/* None match */
#ifndef NOSERVER
			if (server && ngetpath > i)
			  goto nextinpath;
#endif /* NOSERVER */
			retcode = -1;
			debug(F101,"gnfile gnferror A","",gnferror);
			gnferror = -1;
			continue;
		    }
		    if (x < 0) {	/* Too many to expand */
			debug(F101,"gnfile gnferror B","",gnferror);
			gnferror = -5;
			return(-5);
		    }
		    sndsrc = -1;	/* Change send-source to znext() */
		}
	    } else {			/* We're out of files. */
		debug(F111,"gnfile done",ckitoa(gnferror),nfils);
		*filnam = '\0';
		return(0);
	    }
	}

/* Otherwise, step to next element of internal wildcard expansion list. */

	if (sndsrc == -1) {
	    int xx = 0;
	    while (1) {
		debug(F111,"gnfile znext X",filnam,xx);
		xx = znext(filnam);
		debug(F111,"gnfile znext B",filnam,xx);
		if (!filnam[0])
		  break;
		if (dodirstoo) {
		    debug(F111,"gnfile FTP MPUT /RECURSIVE",filnam,xx);
		    break;
		}
		if (!isdir(filnam))
		  break;
	    }
	    debug(F111,"gnfile znext C",filnam,x);
	    if (!filnam[0]) {		/* If no more, */
		sndsrc = 1;		/* go back to previous list */
		debug(F101,"gnfile setting sndsrc back","",sndsrc);
		continue;
	    } else
	      ckstrncpy(fullname,filnam,CKMAXPATH+1);
	}

/* Get here with a filename. */

gotnam:
	debug(F110,"gnfile fullname",fullname,0);
	if (fullname[0]) {
#ifdef DTILDE
	    char * dirp = "";
	    if (fullname[0] == '~') {
		dirp = tilde_expand((char *)fullname);
		if (*dirp) ckstrncpy(fullname,dirp,CKMAXPATH+1);
	    }
#endif /* DTILDE */
	    filesize = zchki(fullname);	/* Check if file readable */
	    debug(F111,"gnfile zchki",fullname,filesize);
	    retcode = filesize;		/* Possible return code */
	    if (filesize == (CK_OFF_T)-2 && dodirstoo) {
		filesize = 0;
	    }
	    if (filesize < 0) {
		gnferror = (int)filesize;
		debug(F101,"gnfile gnferror C","",gnferror);
	    }
	    if (filesize == (CK_OFF_T)-1) { /* If not found */
		debug(F100,"gnfile -1","",0);
#ifndef NOSERVER
		if (server && ngetpath > i)
		  goto nextinpath;
#endif /* NOSERVER */
		debug(F110,"gnfile skipping:",fullname,0);
		tlog(F110,fullname,": open failure - skipped",0);
		xxscreen(SCR_FN,0,0l,fullname);
		xxscreen(SCR_ST,ST_SKIP,SKP_ACC,fullname);
#ifdef TLOG
		if (tralog && !tlogfmt)
		  doxlog(what,fullname,fsize,binary,1,"Skipped");
#endif /* TLOG */
		continue;
	    } else if (filesize < 0) {
		if (filesize == (CK_OFF_T)-3) { /* Exists but not readable */
		    debug(F100,"gnfile -3","",0);
		    filrej++;		/* Count this one as not sent */
		    tlog(F110,"Read access denied",fullname,0); /* Log this */
		    xxscreen(SCR_FN,0,0l,fullname);
		    xxscreen(SCR_ST,ST_SKIP,SKP_ACC,fullname); /* Display it */
#ifdef TLOG
		    if (tralog && !tlogfmt)
		      doxlog(what,fullname,fsize,binary,1,"Skipped");
#endif /* TLOG */
		}
		continue;
	    } else {
		int xx;
		fsize = filesize;
/* +++ */
    debug(F111,"gnfile sndsmaller",ckfstoa(sndsmaller),sndsmaller);
    debug(F111,"gnfile sndlarger",ckfstoa(sndlarger),sndlarger);
    debug(F111,"gnfile (CK_OFF_T)-1",ckfstoa((CK_OFF_T)-1),(CK_OFF_T)-1);

		xx = fileselect(fullname,
				sndafter, sndbefore,
				sndnafter,sndnbefore,
				sndsmaller,sndlarger,
				skipbup,
				NSNDEXCEPT,sndexcept);
		debug(F111,"gnfile fileselect",fullname,xx);
		if (!xx) {
		    filesize = (CK_OFF_T)-1;
		    gnferror = -6;
		    debug(F101,"gnfile gnferror D","",gnferror);
		    continue;
		}
		ckstrncpy(filnam,fullname,CKMAXPATH+1);
		return(1);
	    }
#ifdef COMMENT
	/* This can't be right! */
	} else {			/* sndsrc is 0... */
	    if (!fileselect(fullname,
			    sndafter, sndbefore,
			    sndnafter,sndnbefore,
			    sndsmaller,sndlarger,
			    skipbup,
			    NSNDEXCEPT,sndexcept)) {
		gnferror = -6;
		debug(F111,"gnfile fileselect",fullname,gnferror);
		filesize = (CK_OFF_T)-1;
		continue;
	    }
	    ckstrncpy(filnam,fullname,CKMAXPATH+1);
	    return(1);
#endif /* COMMENT */
	}
    }
    debug(F101,"gnfile result","",retcode);
    *filnam = '\0';
    return(0);
}

/*
  The following bunch of routines feed internally generated data to the server
  to send to the client in response to REMOTE commands like DIRECTORY, DELETE,
  and so on.  We have to write these lines in the format appropriate to our
  platform, so they can be converted to generic (CRLF) text format by the
  packetizer.
*/
#ifdef UNIX
char * endline = "\12";
#else
#ifdef datageneral
char * endline = "\12";
#else
#ifdef MAC
char * endline = "\15";
#else
#ifdef OSK
char * endline = "\15";
#else
char * endline = "\15\12";
#endif /* OSK */
#endif /* MAC */
#endif /* datageneral */
#endif /* UNIX */

#ifdef MAC
#define FNCBUFL 256
#else
#ifdef CKSYMLINK
#define FNCBUFL (CKMAXPATH + CKMAXPATH + 64)
#else
#define FNCBUFL (CKMAXPATH + 64)
#endif /* CKSYMLINK */
#endif /* MAC */

/* NB: The minimum FNCBUFL is 255 */

static CHAR funcbuf[FNCBUFL];
static int  funcnxt =  0;
static int  funclen =  0;
static int  nxpnd   = -1;
static long ndirs   =  0;
static long nfiles  =  0;
static CK_OFF_T nbytes  =  0;

int
sndstring(p) char * p; {
#ifndef NOSERVER
    nfils = 0;				/* No files, no lists. */
    xflg = 1;				/* Flag we must send X packet. */
    ckstrncpy(cmdstr,versio,CMDSTRL);	/* Data for X packet. */
    first = 1;				/* Init getchx lookahead */
    memstr = 1;				/* Just set the flag. */
    memptr = p;				/* And the pointer. */
    binary = XYFT_T;			/* Text mode for this. */
    return(sinit());
#else
    return(0);
#endif /* NOSERVER */
}

/*  S N D H L P  --  Routine to send builtin help  */

static int srvhlpnum = 0;

#ifdef IKSD
static char *nmx[] =  { "Disabled", "Disabled", "Enabled", "Enabled" };
#endif /* IKSD */

static char *
xnm(x) int x; {
#ifdef IKSD
    if (inserver)
      return(nmx[x]);
    else
#endif /* IKSD */
      return(nm[x]);
}

static int
nxthlp(
#ifdef CK_ANSIC
       void
#endif /* CK_ANSIC */
       ) {
    int x = 0;
    extern int
      en_cpy, en_cwd, en_del, en_dir, en_fin, en_get, en_bye, en_mai,
      en_pri, en_hos, en_ren, en_sen, en_spa, en_set, en_typ, en_who,
      /* en_ret, */ en_mkd, en_rmd, en_asg, en_que, en_xit, x_login, x_logged,
      xfinish;
    extern char * ckxsys;

    if (funcnxt < funclen)
      return (funcbuf[funcnxt++]);

    switch (srvhlpnum++) {
      case 0:
	x = ckstrncpy((char *)funcbuf,
		      "Client Command     Status        Description\n",
		      FNCBUFL
		      );
	if (x_login && !x_logged) {
	    x += ckstrncat((char *)funcbuf,
			   " REMOTE LOGIN       required\n",
			   FNCBUFL
			   );
	}
	if (FNCBUFL - x > 74)
	sprintf((char *)(funcbuf+x)," GET                %-14s%s\n",
		xnm(en_get),
		"Transfer file(s) from server to client."
		);
	break;

/* NOTE: The minimum funcbuf[] size is 255; all of the following are safe. */

      case 1:
	sprintf((char *)funcbuf," SEND               %-14s%s\n",
		xnm(en_sen),
		"Transfer file(s) from client to server."
		);
	break;

      case 2:
	sprintf((char *)funcbuf," MAIL               %-14s%s\n",
		xnm(inserver ? 0 : en_mai),
		"Send file(s) as e-mail."
		);
	break;

      case 3:
#ifndef NOSPL
	sprintf((char *)funcbuf," REMOTE ASSIGN      %-14s%s\n",
		xnm(en_asg),
		"Assign value to server variable or macro."
		);
#else
	sprintf((char *)funcbuf," REMOTE ASSIGN      not configured\n");
#endif /* NOSPL */

	break;
      case 4:
	sprintf((char *)funcbuf," REMOTE CD          %-14s%s\n",
		xnm(en_cwd),
		"Change server's directory."
		);
	break;

      case 5:
#ifdef ZCOPY
	sprintf((char *)funcbuf," REMOTE COPY        %-14s%s\n",
		xnm(en_cpy),
		"Copy a file on the server."
		);
#else
	sprintf((char *)funcbuf," REMOTE COPY        not configured\n");
#endif /* ZCOPY */

	break;
      case 6:
	sprintf((char *)funcbuf," REMOTE DELETE      %-14s%s\n",
		xnm(en_del),
		"Delete a file on the server."
		);
	break;

      case 7:
	sprintf((char *)funcbuf," REMOTE DIRECTORY   %-14s%s\n",
		xnm(en_dir),
		"List files on the server."
		);
	break;

      case 8:
	sprintf((char *)funcbuf," REMOTE EXIT        %-14s%s\n",
		xnm(en_xit),
		"Exit from Kermit server program."
		);
	break;

      case 9:
	sprintf((char *)funcbuf," REMOTE HOST        %-14s%s\n",
		xnm(inserver ? 0 : en_hos),
#ifdef datageneral
		"Execute a CLI command on the server."
#else
#ifdef VMS
		"Execute a DCL command on the server."
#else
		"Execute a shell command on the server."
#endif /* VMS */
#endif /* datageneral */
		);
	break;

      case 10:
	sprintf((char *)funcbuf," REMOTE PRINT       %-14s%s\n",
		xnm(inserver ? 0 : en_pri),
		"Send a file to the server for printing."
		);
	break;

      case 11:
#ifndef NOSPL
	sprintf((char *)funcbuf," REMOTE QUERY       %-14s%s\n",
		xnm(en_que),
		"Get value of server variable or macro."
		);

#else
	sprintf((char *)funcbuf," REMOTE QUERY       not configured\n");
#endif /* NOSPL */

	break;
      case 12:
	sprintf((char *)funcbuf," REMOTE MKDIR       %-14s%s\n",
		xnm(en_mkd),
		"Create a directory on the server."
		);
	break;

      case 13:
	sprintf((char *)funcbuf," REMOTE RMDIR       %-14s%s\n",
		xnm(en_rmd),
		"Remove a directory on the server."
		);
	break;

      case 14:
	sprintf((char *)funcbuf," REMOTE RENAME      %-14s%s\n",
		xnm(en_ren),
		"Rename a file on the server."
		);
	break;

      case 15:
	sprintf((char *)funcbuf," REMOTE SET         %-14s%s\n",
		xnm(en_set),
		"Set a parameter on the server"
		);
	break;

      case 16:
	sprintf((char *)funcbuf," REMOTE SPACE       %-14s%s\n",
		xnm(en_spa),
		"Inquire about disk space on the server."
		);
	break;

      case 17:
	sprintf((char *)funcbuf," REMOTE TYPE        %-14s%s\n",
		xnm(en_typ),
		"Display a server file on your screen."
		);
	break;

      case 18:
	sprintf((char *)funcbuf," REMOTE WHO         %-14s%s\n",
		xnm(inserver ? 0 : en_who),
		"List who is logged in to the server."
		);
	break;

      case 19:
	sprintf((char *)funcbuf," FINISH             %-14s%s\n",
		xnm(en_fin),
		xfinish ?
		"Exit from Kermit server program." :
		"Return the server to its command prompt."
		);
	break;

      case 20:
	sprintf((char *)funcbuf," BYE                %-14s%s\n\n",
		xnm(en_bye),
		"Log the server out and disconnect."
		);
	break;

      default:
	return(-1);
    }
    funcnxt = 0;
    funclen = strlen((char *)funcbuf);
    return(funcbuf[funcnxt++]);
}

int
sndhlp() {
#ifndef NOSERVER
    extern char * ckxsys;

    first = 1;				/* Init getchx lookahead */
    nfils = 0;				/* No files, no lists. */
    xflg = 1;				/* Flag we must send X packet. */
    ckstrncpy(cmdstr,"REMOTE HELP",CMDSTRL); /* Data for X packet. */
    sprintf((char *)funcbuf, "C-Kermit %s,%s\n\n", versio, ckxsys);
    funclen = strlen((char *)funcbuf);
#ifdef IKSD
    if (inserver) {
	sprintf((char *)(funcbuf+funclen),
		"Internet Kermit Service (EXPERIMENTAL)\n\n");
	funclen = strlen((char *)funcbuf);
    }
#endif /* IKSD */
    funcnxt = 0;
    funcptr = nxthlp;
    funcstr = 1;
    srvhlpnum = 0;
    binary = XYFT_T;			/* Text mode for this. */
    return(sinit());
#else
    return(0);
#endif /* NOSERVER */
}

/*
   Returns the next available character,
  -1 if no more data.
*/
static int
nxttype(
#ifdef CK_ANSIC
       void
#endif /* CK_ANSIC */
	) {
    int c;
    if (zchin(ZIFILE,&c) < 0) {
        zclose(ZIFILE);
        return(-1);
    } else {
	return((unsigned)c);
    }
}

/*  S N D T Y P -- TYPE a file to remote client */

int
sndtype(file) char * file; {
#ifndef NOSERVER
    char name[CKMAXPATH+1];

#ifdef OS2
    char * p = NULL;

    if (*file) {
        ckstrncpy(name, file, CKMAXPATH+1);
        /* change / to \. */
        p = name;
        while (*p) {			/* Change them back to \ */
            if (*p == '/') *p = '\\';
            p++;
        }
    } else
      return(0);
#else
    ckstrncpy(name, file, CKMAXPATH+1);
#endif /* OS2 */

    funcnxt = 0;
    funclen = strlen((char *)funcbuf);
    if (zchki(name) == -2) {
        /* Found a directory */
        return(0);
    }
    if (!zopeni(ZIFILE,name))
      return(0);

    nfils = 0;				/* No files, no lists. */
    xflg = 1;				/* Flag we must send X packet. */
    ckstrncpy(cmdstr,"type",CMDSTRL);	/* Data for X packet. */
    first = 1;				/* Init getchx lookahead */
    funcstr = 1;			/* Just set the flag. */
    funcptr = nxttype;			/* And the pointer. */
    binary = XYFT_T;			/* Text mode for this */
    return(sinit());
#else
    return(0);
#endif /* NOSERVER */
}

/*
   N X T D I R  --  Provide data for senddir()

   Returns the next available character or -1 if no more data.
*/
#ifndef NOICP
/* Directory listing parameters set by the user interface, if any. */
extern int dir_head, dir_dots, dir_back;
#endif /* NOICP */
static int sd_hdg, sd_bkp, sd_dot;	/* Local listing parameters */

static int
nxtdir(
#ifdef CK_ANSIC
       void
#endif /* CK_ANSIC */
       ) {
    char name[CKMAXPATH+1], dbuf[24], *p = NULL;
    char *dstr = NULL, * lnk = "";
    CHAR c, * linebuf = funcbuf;
#ifdef OSK
    /* Work around bugs in OSK compiler */
    char *dirtag = "directories";
    char *filetag = "files";
    char *bytetag = "bytes";
#endif /* OSK */
    CK_OFF_T len = 0;
    int x, itsadir = 0, gotone = 0;

#ifdef DEBUG
    if (deblog) {
	debug(F101,"nxtdir funcnxt","",funcnxt);
	debug(F101,"nxtdir funclen","",funclen);
	debug(F110,"nxtdir funcbuf",funcbuf+funcnxt,0);
    }
#endif /* DEBUG */
    if (funcnxt < funclen) {		/* Return next character from buffer */
	c = funcbuf[funcnxt++];
	debug(F000,"nxtdir return 1","",(unsigned)(c & 0xff));
	return((unsigned)(c & 0xff));
    }
    while (nxpnd > 0) {			/* Buffer needs refill */
        nxpnd--;
	znext(name);			/* Get next filename */
        if (!name[0]) {			/* None left - done */
            nxpnd = 0;
            return(nxtdir());
        }
	if (sd_bkp) {			/* Showing backup files? */
	    gotone = 1;			/* Yes, no need to check. */
	    break;
	}
	x = ckmatch(			/* No - see if this is one */
#ifdef CKREGEX
		    "*.~[0-9]*~"	/* Not perfect but close enough. */
#else
		    "*.~*~"		/* Less close. */
#endif /* CKREGEX */
		    ,name,filecase,1);
	debug(F111,"nxtdir ckmatch",name,x);
	if (x) {
	    continue;			/* It's a backup file - skip it */
	} else {
	    gotone = 1;			/* It's not, break from loop. */
	    break;
	}
    }
    if (gotone) {
        len = zgetfs(name);		/* Get file size */
	debug(F111,"nxtdir zgetfs",name,len);
#ifdef VMSORUNIX
	itsadir = zgfs_dir;		/* See if it's a directory */
#else
	itsadir = (len == -2 || isdir(name));
#endif /* VMSORUNIX */
        dstr = zfcdat(name);
	debug(F111,"nxtdir zcfdat",dstr,0);
	if (!dstr)
	  dstr = "0000-00-00 00:00:00";
	if (!*dstr) {
	  dstr = "0000-00-00 00:00:00";
	} else {
	    dbuf[0] = dstr[0];
	    dbuf[1] = dstr[1];
	    dbuf[2] = dstr[2];
	    dbuf[3] = dstr[3];
	    dbuf[4] = '-';
	    dbuf[5] = dstr[4];
	    dbuf[6] = dstr[5];
	    dbuf[7] = '-';
	    dbuf[8] = dstr[6];
	    dbuf[9] = dstr[7];
	    strcpy(dbuf+10,dstr+8);
	    dstr = dbuf;
	}
#ifdef CK_PERMS
#ifdef VMSORUNIX
	p = ziperm(name);		/* Get permissions */
#else
	p = zgperm(name);
#endif /* VMSORUNIX */
#else
	p = NULL;
#endif /* CK_PERMS */
	debug(F110,"domydir perms",p,0);

#ifdef VMS
	/* Make name relative */
	ckstrncpy(name,zrelname(name,zgtdir()),CKMAXPATH+1);
#endif /* VMS */

	if (itsadir) {
	    ndirs++;
	} else {
	    nfiles++;
	    nbytes += len;
	}
	lnk = "";
#ifdef UNIX
#ifdef CKSYMLINK
	if (zgfs_link) {
	    extern char linkname[];
	    lnk = linkname;
	}
	debug(F111,"nxtdir linkname",lnk,zgfs_link);
#endif /* CKSYMLINK */
#endif /* UNIX */

/*
  The following sprintf's are safe; linebuf is a pointer to funcbuf,
  which is 64 bytes larger than CKMAXPATH (or double CKMAXPATH when
  symlinks are possible).  64 allows for the fixed-field portions of
  the file listing line: permissions, size, and date.  CKMAXPATH allows
  for the longest possible pathname.
*/
	if (itsadir && len < 0) {	/* Directory */
#ifdef VMS
	    sprintf((char *)linebuf,
		    "%-22s%-10s  %s  %s\n",p,"<DIR>",dstr,name);
#else
	    if (p)
	      sprintf((char *)linebuf,
		      "%10s%-10s  %s  %s\n",p,"<DIR>",dstr,name);
	    else
	      sprintf((char *)linebuf,
		      "%-10s  %s  %s\n", "<DIR>", dstr, name);
#endif /* VMS */
	} else {			/* Regular file */
#ifdef VMS
	    sprintf((char *)linebuf,
		    "%-22s%10s  %s  %s\n", p, ckfstoa(len), dstr, name);
#else
	    if (p)
	      sprintf((char *)linebuf,
		      "%10s%10s  %s  %s%s%s\n",
		      p, ckfstoa(len), dstr, name,
		      *lnk ? " -> " : "",
		      lnk
		      );
	    else
	      sprintf((char *)linebuf,
		      "%10s  %s  %s%s%s\n",
		      ckfstoa(len), dstr, name,
		      *lnk ? " -> " : "",
		      lnk
		      );
#endif /* VMS */
	}
        funcnxt = 0;
        funclen = strlen((char *)funcbuf);
    } else if (sd_hdg && nxpnd == 0) {	/* Done, send summary */
	char *blankline = "";		/* At beginning of summary */
/*
  The idea is to prevent (a) unnecessary multiple blanklines, and (b)
  prompt-stomping.  Preventing (b) is practically impossible, because it
  depends on the client so for now always include that final CRLF.
*/
	if (!ndirs || !nbytes || !nfiles)
	  blankline = endline;
#ifdef OSK
/* Workaround bugs in OS-9 compiler... */
        if (ndirs == 1)
           dirtag = "directory";
        if (nfiles == 1)
           filetag = "file";
        if (nbytes == (CK_OFF_T)1)
           bytetag = "byte";
        sprintf((char *)funcbuf,
		"%sSummary: %ld %s, %ld %s, %s %s%s",
		blankline,
		ndirs,
		dirtag,
		nfiles,
		filetag,
		ckfstoa(nbytes),
		bytetag,
		endline);
#else
        sprintf((char *)funcbuf,
		"%sSummary: %ld director%s, %ld file%s, %s byte%s%s",
		blankline,
		ndirs,
		(ndirs == 1) ? "y" : "ies",
		nfiles,
		(nfiles == 1) ? "" : "s",
		ckfstoa(nbytes),
		(nbytes == (CK_OFF_T)1) ? "" : "s",
		endline
		);
#endif /* OSK */
        nxpnd--;
        funcnxt = 0;
        funclen = strlen((char *)funcbuf);
    } else {
        funcbuf[0] = '\0';
        funcnxt = 0;
        funclen = 0;
    }
    debug(F101,"nxtdir funclen","",funclen);

    if (funcnxt < funclen) {		/* If we have data to send... */
	c = funcbuf[funcnxt++];
	debug(F000,"nxtdir return 2","",(unsigned)(c & 0xff));
	return((unsigned)(c & 0xff));
    } else
      return(-1);			/* Nothing left, done. */
}

/*  S N D D I R -- send directory listing  */

int
snddir(spec) char * spec; {
#ifndef NOSERVER
    char * p = NULL, name[CKMAXPATH+1];
    int t = 0, rc = 0;
    char fnbuf[CKMAXPATH+1];

    debug(F111,"snddir matchdot",spec,matchdot);

#ifndef NOICP
    debug(F111,"snddir dir_dots",spec,dir_dots);
    sd_hdg = dir_head > 0;		/* Import listing parameters if any */
    sd_bkp = dir_back > 0;
    if (dir_dots > -1)
      sd_dot = dir_dots;
    else
      sd_dot = matchdot;
#else
    sd_hdg = 1;				/* Or use hardwired defaults */
    sd_bkp = 1;
    sd_dot = matchdot;
#endif /* NOICP */

    if (!spec) spec = "";
    debug(F111,"snddir sd_dot",spec,sd_dot);
    if (*spec) {
#ifdef COMMENT
	zfnqfp(spec,CKMAXPATH,name);
	debug(F110,"snddir zfnqfp",name,0);
#else
	ckstrncpy(name,spec,CKMAXPATH+1);
	debug(F110,"snddir name",name,0);
#endif /* COMMENT */
    } else {
#ifdef OS2
	strcpy(name, "*");
#else
#ifdef UNIXOROSK
	strcpy(name, "./*");
#else
#ifdef VMS
	strcpy(name, "*.*");
#else
#ifdef datageneral
	strcpy(name, "+");
#else
	debug(F101,"snddir quit (no filespec)","",0);
	return(0);
#endif /* datageneral */
#endif /* VMS */
#endif /* UNIX */
#endif /* OS2 */
    }
    debug(F110,"snddir name 1",name,0);
    ndirs = 0L;
    nfiles = 0L;
    nbytes = (CK_OFF_T)0;

    if (zfnqfp(name,CKMAXPATH,fnbuf))

    debug(F110,"snddir name 2",name,0);
    p = name + strlen(name);		/* Move it to end of list */

    /* sprintf safe because funcbuf size >= max path len + 64 */

    if (sd_hdg) {
	sprintf((char *)funcbuf,"Listing files: %s%s%s",fnbuf,endline,endline);
	funcnxt = 0;
	funclen = strlen((char *)funcbuf);
    }
    diractive = 1;

#ifdef OS2
    if (zchki(name) == -2) {		/* Found a directory */
        p--;
        if (*p == '\\' || *p == '/')
          ckstrncat(name, "*", CKMAXPATH);
        else if (*p == ':')
          ckstrncat(name, ".", CKMAXPATH);
        else
          ckstrncat(name, "\\*", CKMAXPATH);
	debug(F110,"snddir directory",name,0);
    }
#else
    if (!iswild(name) && isdir(name)) {
	char * s = name;
	p--;
#ifdef UNIXOROSK
	if (*p == '/')			/* So append wildcard to it */
	  ckstrncat(s, "*", CKMAXPATH);
	else
	  ckstrncat(s, "/*", CKMAXPATH);
#else
#ifdef VMS
	if (*p == ']' || *p == '>' || *p == ':')
	  ckstrncat(s, "*.*", CKMAXPATH);
#else
#ifdef datageneral
	if (*p == ':')
	  ckstrncat(s, "+", CKMAXPATH);
	else
	  ckstrncat(s, ":+", CKMAXPATH);
#else
#ifdef VOS
	if (*p == '>')
	  ckstrncat(s, "*", CKMAXPATH);
	else
	  ckstrncat(s, ">*", CKMAXPATH);
#endif /* VOS */
#endif /* datageneral */
#endif /* VMS */
#endif /* UNIXOROSK */
	debug(F110,"snddir directory",name,0);
    }
#endif /* OS2 */

    nzxopts = 0;
#ifdef UNIX
    {
	extern char ** mtchs;
	debug(F111,"snddir sd_dot",spec,sd_dot);
	if (sd_dot > 0)
	  nzxopts |= ZX_MATCHDOT;
	if (recursive)
	  nzxopts |= ZX_RECURSE;
	debug(F111,"snddir nzxopts",spec,nzxopts);
	nxpnd = nzxpand(name,nzxopts);	/* Get the array of names */
	sh_sort(mtchs,NULL,nxpnd,0,0,1); /* Sort the array */
    }
#else
    if (recursive) nzxopts |= ZX_RECURSE;
    nxpnd = nzxpand(name,nzxopts);
#endif /* UNIX */

    debug(F101,"snddir nzxpand nxpnd","",nxpnd);
    if (nxpnd < 1)
      return(-1);
    nfils = 0;				/* No files, no lists. */
    xflg = 1;				/* Flag we must send X packet. */
    if ((int)strlen(name) < CMDSTRL - 11) /* Data for X packet. */
      sprintf(cmdstr,"DIRECTORY %s",name); /* safe */
    else
      ckstrncpy(cmdstr,"DIRECTORY",CMDSTRL);
    first = 1;				/* Init getchx lookahead */
    funcstr = 1;			/* Just set the flag. */
    funcptr = nxtdir;			/* And the pointer. */
    binary = XYFT_T;			/* Text mode for this */
    rc = sinit();			/* 26 Aug 2005 */
    debug(F111,"snddir","sinit()",rc);
    return(rc);
#else
    return(0);
#endif /* NOSERVER */
}

/*  N X T D E L -- provide data for delete   */

/*  Returns the next available character or -1 if no more data  */

static int
nxtdel(
#ifdef CK_ANSIC
       void
#endif /* CK_ANSIC */
       ) {
    char name[257], *p = NULL;
    int len = 0;

    if (funcnxt < funclen)
      return ((unsigned)funcbuf[funcnxt++]);

    if (nxpnd > 0) {
        nxpnd--;
	znext(name);
        if (!name[0]) {
            nxpnd = 0;
            return(nxtdel());
        }
        len = zchki(name);

        /* Find just the name of the file */

        for (p = name + strlen(name); p != name && *p != '/' ; p--) ;
        if (*p == '/') p++;

	/* sprintf's safe because size of funcbuf >= 64 + maxpathlen */

        if (len > -1L) {
	    if (zdelet(name)) {
		sprintf((char *)funcbuf," %10s: %s%s","skipping",p,endline);
	    } else {
		nfiles++;
		nbytes += len;
		sprintf((char *)funcbuf," %10s: %s%s","deleted",p,endline);
	    }
        } else
	  sprintf((char *)funcbuf," directory: %s%s", p, endline);
        funcnxt = 0;
        funclen = strlen((char *)funcbuf);
    } else

    /* If done processing the expanded entries send a summary statement */

      if (nxpnd == 0) {
	  sprintf((char *)funcbuf,
		  "%s%ld file%s deleted, %s byte%s freed%s",
		  endline,
		  nfiles,
		  (nfiles == 1) ? "" : "s",
		  ckfstoa(nbytes),
		  (nbytes == (CK_OFF_T)1) ? "" : "s",
		  endline
		  );
	  nxpnd--;
	  funcnxt = 0;
	  funclen = strlen((char *)funcbuf);
      } else {
	  funcbuf[0] = '\0';
	  funcnxt = 0;
	  funclen = 0;
      }

    /* If we have data to send */

    if (funcnxt < funclen)
      return ((unsigned)funcbuf[funcnxt++]); /* Return a character */
    else
      return(-1);			/* No more input */
}

/*  S N D D E L  --  Send delete message  */

int
snddel(spec) char * spec; {
#ifndef NOSERVER
    char name[CKMAXPATH+1];
#ifdef OS2
    char * p = NULL;
#endif /* #ifdef OS2 */

    if (!*spec)
      return(0);

    ckstrncpy(name, spec, CKMAXPATH+1);

#ifdef OS2
    /* change / to \. */
    p = name;
    while (*p) {			/* Change them back to \ */
        if (*p == '/') *p = '\\';
        p++;
    }
#endif /* OS2 */

    nfiles = 0L;
    nbytes = (CK_OFF_T)0;
    sprintf((char *)funcbuf,"Deleting \"%s\"%s",name,endline);
    funcnxt = 0;
    funclen = strlen((char *)funcbuf);

    nzxopts = ZX_FILONLY;		/* Files only */
#ifdef UNIXOROSK
    if (matchdot) nzxopts |= ZX_MATCHDOT;
#endif /* UNIXOROSK */
#ifdef COMMENT
    /* Recursive deleting not supported yet */
    if (recursive) nzxopts |= ZX_RECURSE;
#endif /* COMMENT */
    nxpnd = nzxpand(name,nzxopts);
    if (nxpnd < 1)
      return(-1);
    nfils = 0;				/* No files, no lists. */
    xflg = 1;				/* Flag we must send X packet. */
    ckstrncpy(cmdstr,"REMOTE DELETE",CMDSTRL); /* Data for X packet. */
    first = 1;				/* Init getchx lookahead */
    funcstr = 1;			/* Just set the flag. */
    funcptr = nxtdel;			/* And the pointer. */
    binary = XYFT_T;			/* Use text mode for this, */
    return(sinit());
#else
    return(0);
#endif /* NOSERVER */
}

#ifdef OS2
/*  S N D S P A C E -- send disk space message  */
int
sndspace(drive) int drive; {
#ifndef NOSERVER
    static char spctext[64];
    unsigned long space;

    if (drive) {
	space = zdskspace(drive - 'A' + 1);
	if (space > 0 && space < 1024)
	  sprintf(spctext,
		  " Drive %c: unknown%s",
		  drive,
		  endline
		  );
	else
	  sprintf(spctext,
		  " Drive %c: %ldK free%s",
		  drive,
		  space / 1024L,
		  endline
		  );
    } else {
	space = zdskspace(0);
	if (space > 0 && space < 1024)
	  sprintf(spctext, " Free space: unknown%s", endline);
	else
	  sprintf(spctext, " Free space: %ldK%s", space / 1024L, endline);
    }
    nfils = 0;				/* No files, no lists. */
    xflg = 1;				/* Flag we must send X packet. */
    ckstrncpy(cmdstr,"free space",CMDSTRL); /* Data for X packet. */
    first = 1;				/* Init getchx lookahead */
    memstr = 1;				/* Just set the flag. */
    memptr = spctext;			/* And the pointer. */
    binary = XYFT_T;			/* Text mode for this. */
    return(sinit());
#else
    return(0);
#endif /* NOSERVER */
}

/*  S N D W H O -- send who message  */
int
sndwho(who) char * who; {
#ifndef NOSERVER
    nfils = 0;				/* No files, no lists. */
    xflg = 1;				/* Flag we must send X packet. */
    ckstrncpy(cmdstr,"who",CMDSTRL);	/* Data for X packet. */
    first = 1;				/* Init getchx lookahead */
    memstr = 1;				/* Just set the flag. */
#ifdef NT
    memptr = "\15\12K95 SERVER\15\12";	/* And the pointer. */
#else
    memptr = "\15\12K/2 SERVER\15\12";
#endif /* NT */
    binary = XYFT_T;			/* Use text mode */
    return(sinit());
#else
    return(0);
#endif /* NOSERVER */
}
#endif /* OS2 */

/*  C W D  --  Change server's working directory  */

/*
 String passed has first byte as length of directory name, rest of string
 is name.  Returns:
  0 on failure.
  1 on success after sending short-form response (ACK with name).
  2 on success if a CD Message file is to be sent.
*/
int
cwd(vdir) char *vdir; {
    char *cdd, *dirp;

    vdir[xunchar(*vdir) + 1] = '\0';	/* Terminate string with a null */
    dirp = vdir+1;
    tlog(F110,"Directory requested: ",dirp,0L);
    if (zchdir(dirp)) {			/* Try to change */
	cdd = zgtdir();			/* Get new working directory. */
	debug(F110,"cwd",cdd,0);
	if (srvcdmsg) {			/* Send orientation file? */
	    int i;
	    for (i = 0; i < 8; i++) {
		if (zchki(cdmsgfile[i]) > -1) {
		    xxscreen(SCR_CD,0,0l,cdd);
		    tlog(F110,"Changed directory to",cdd,0L);
		    return(2);
		}
	    }
	}
	encstr((CHAR *)cdd);		/* Send short-form reply */
	ack1(data);			/* containing directory name. */
	xxscreen(SCR_CD,0,0l,cdd);
	tlog(F110,"Changed directory to",cdd,0L);
	return(1);
    } else {
	debug(F110,"cwd failed",dirp,0);
	tlog(F110,"Failed to change directory to",dirp,0L);
	return(0);
    }
}


/*  S Y S C M D  --  Do a system command  */

/*  Command string is formed by concatenating the two arguments.  */

int
syscmd(prefix,suffix) char *prefix, *suffix; {
    extern int i_isopen;
#ifndef NOPUSH
    char *cp;

    i_isopen = 0;
    if (!prefix)
      return(0);
    if (!*prefix)
      return(0);
    for (cp = cmdstr; *prefix != '\0'; (*cp++ = *prefix++));
    while ((*cp++ = *suffix++))
#ifdef OS2
        /* This takes away more than we gain in convenience
        if (*(cp-1) == '/') *(cp-1) = '\\' */
#endif /* OS2 */
      ;					/* Copy suffix */

    debug(F110,"syscmd",cmdstr,0);

    if (zxcmd(ZIFILE,cmdstr) > 0) {
    	debug(F110,"syscmd zxcmd ok",cmdstr,0);
	nfils = sndsrc = 0;		/* Flag that input is from stdin */
    	xflg = hcflg = 1;		/* And special flags for pipe */
	binary = XYFT_T;		/* Go to text mode */
	i_isopen = 1;
    	return (sinit());		/* Send S packet */
    } else {
    	debug(F100,"syscmd zxcmd failed",cmdstr,0);
	i_isopen = 0;
    	return(0);
    }
#else
    debug(F100,"syscmd zxcmd NOPUSH",cmdstr,0);
    i_isopen = 0;
    return(0);
#endif /* NOPUSH */
}

/*  R E M S E T  --  Remote Set  */
/*  Called by server to set variables as commanded in REMOTE SET packets.  */
/*  Returns 1 on success, 0 on failure.  */

int
remset(s) char *s; {
    extern int c_save, en_del;
    int len, i, x, y;
    char *p;

    len = xunchar(*s++);		/* Length of first field */
    p = s + len;			/* Pointer to second length field */
    *p++ = '\0';			/* Zero out second length field */
    x = atoi(s);			/* Value of first field */
    debug(F111,"remset",s,x);
    debug(F110,"remset",p,0);
    switch (x) {			/* Do the right thing */
      case 132:				/* Attributes (all, in) */
	atcapr = atoi(p);
	return(1);
      case 133:				/* File length attributes */
      case 233:				/* IN/OUT combined */
      case 148:				/* Both kinds of lengths */
      case 248:
	atleni = atleno = atoi(p);
	return(1);
      case 134:				/* File Type (text/binary) */
      case 234:
	attypi = attypo = atoi(p);
	return(1);
      case 135:				/* File creation date */
      case 235:
	atdati = atdato = atoi(p);
	return(1);
      case 139:				/* File Blocksize */
      case 239:
	atblki = atblko = atoi(p);
	return(1);
      case 141:				/* Encoding / Character Set */
      case 241:
	atenci = atenco = atoi(p);
	return(1);
      case 142:				/* Disposition */
      case 242:
	atdisi = atdiso = atoi(p);
	return(1);
      case 145:				/* System ID */
      case 245:
	atsidi = atsido = atoi(p);
	return(1);
      case 147:				/* System-Dependent Info */
      case 247:
	atsysi = atsyso = atoi(p);
	return(1);
      case 232:				/* Attributes (all, out) */
	atcapr = atoi(p);
	return(1);
      case 300:				/* File type (text, binary) */
	binary = atoi(p);
	b_save = binary;
#ifndef NOICP
	g_binary = -1;
#endif /* NOICP */
	return(1);
      case 301:				/* File name conversion */
	fncnv = 1 - atoi(p);		/* (oops) */
	f_save = fncnv;
#ifndef NOICP
	g_fncnv = -1;
#endif /* NOICP */
	return(1);
      case 302:				/* File name collision */
#ifdef IKSD
#ifdef CK_LOGIN
	if (inserver && isguest)	/* May not be changed by guest */
	  return(0);
#endif /* CK_LOGIN */
#endif /* IKSD */
	x = atoi(p);
	if (!ENABLED(en_del) && (x == XYFX_X || x == XYFX_U))
	  return(0);
	if (x == XYFX_R) ckwarn = 1;	/* Rename */
	if (x == XYFX_X) ckwarn = 0;	/* Replace */
	fncact = x;
	return(1);
      case 310:				/* Incomplete File Disposition */
	keep = atoi(p);			/* Keep, Discard, Auto */
	return(1);
      case 311:				/* Blocksize */
	fblksiz = atoi(p);
	return(1);
      case 312:				/* Record Length */
	frecl = atoi(p);
	return(1);
      case 313:				/* Record format */
	frecfm = atoi(p);
	return(1);
      case 314:				/* File organization */
	forg = atoi(p);
	return(1);
      case 315:				/* File carriage control */
	fcctrl = atoi(p);
	return(1);
      case 330:				/* Match dotfiles */
#ifndef NOICP
	dir_dots = -1;			/* This undoes DIR /DOT option */
#endif /* NOICP */
	matchdot = atoi(p);
	return(1);
      case 331:				/* Match FIFOs */
	matchfifo = atoi(p);
	return(1);
      case 400:				/* Block check */
	y = atoi(p);
	if (y < 5 && y > 0) {
	    bctr = y;
	    c_save = -1;
	    return(1);
	} else if (*p == 'B') {
	    bctr = 4;
	    c_save = -1;
	    return(1);
	} else if (*p == '5') {
	    bctr = 3;
	    c_save = -1;
	    return(1);
	}
	return(0);
      case 401:				/* Receive packet-length */
	rpsiz = urpsiz = atoi(p);
	if (urpsiz > MAXRP) urpsiz = MAXRP; /* Max long-packet length */
	if (rpsiz > 94) rpsiz = 94;	    /* Max short-packet length */
	urpsiz = adjpkl(urpsiz,wslots,bigrbsiz);
	return(1);
      case 402:				/* Receive timeout */
	y = atoi(p);			/* Client is telling us */
	if (y > -1 && y < 999) {	/* the timeout that it wants */
	    pkttim = chktimo(y,timef);	/* us to tell it to use. */
	    return(1);
	} else return(0);
      case 403:				/* Retry limit */
	y = atoi(p);
	if (y > -1 && y < 95) {
	    maxtry = y;
	    return(1);
	} else return(0);
      case 404:				/* Server timeout */
	y = atoi(p);
	if (y < 0) return(0);
	srvtim = y;
	return(1);

#ifndef NOCSETS
      case 405: {				/* Transfer character set */
	  extern int s_cset, axcset[];
	  int i;
	  for (i = 0; i < ntcsets; i++) {
	      if (!strcmp(tcsinfo[i].designator,p)) break;
	  }
	  debug(F101,"remset tcharset lookup","",i);
	  if (i == ntcsets) return(0);
	  tcharset = tcsinfo[i].code;	/* If known, use it */
	  debug(F101,"remset tcharset","",tcharset);
	  if (s_cset == XMODE_A)
	    if (axcset[tcharset] > -1 && axcset[tcharset] > MAXFCSETS)
	      fcharset = axcset[tcharset]; /* Auto-pick file charset */
	  debug(F101,"remset tcharset fcharset","",fcharset);
	  setxlatype(tcharset,fcharset); /* Set up charset translations */
	  debug(F101,"remset xlatype","",xlatype);
	  debug(F101,"remset tcharset after setxlatype","",tcharset);
	  tcs_save = -1;
	  return(1);
      }
      case 320: {			/* File character set */
	  extern struct keytab fcstab[];
	  extern int nfilc, s_cset, r_cset;
	  x = lookup(fcstab,p,nfilc,&y);
	  debug(F111,"RSET FILE CHAR name",p,x);
	  if (x < 0)
	    return(0);
	  s_cset = XMODE_M;		/* No automatic charset switching */
	  r_cset = XMODE_M;
	  fcharset = x;			/* Set file charset */
	  setxlatype(tcharset,fcharset); /* and translation type */
	  fcs_save = -1;
	  return(1);
      }
#endif /* NOCSETS */

      case 406:				/* Window slots */
	y = atoi(p);
	if (y == 0) y = 1;
	if (y < 1 || y > MAXWS) return(0);
	wslotr = y;
	swcapr = 1;
	urpsiz = adjpkl(urpsiz,wslotr,bigrbsiz);
	return(1);

      case 410:				/* Transfer mode */
	y = atoi(p);			/* 0 = automatic, nonzero = manual */
	if (y != 0) y = 1;
	xfermode = y;
	debug(F101,"REMOTE SET xfermode","",xfermode);
	return(1);

      case 420:				/* SERVER CD-MESSAGE { ON, OFF } */
	y = atoi(p);			/* 0 = automatic, nonzero = manual */
	srvcdmsg = y;
	return(1);

      default:				/* Anything else... */
	return(0);
    }
}

/* Adjust packet length based on number of window slots and buffer size */

int
adjpkl(pktlen,slots,bufsiz) int pktlen, slots, bufsiz; {
    if (protocol != PROTO_K) return(pktlen);
    debug(F101,"adjpkl len","",pktlen);
    debug(F101,"adjpkl slots","",slots);
    debug(F101,"adjpkl bufsiz","",bufsiz);
    if (((pktlen + 6) * slots) > bufsiz)
      pktlen = (bufsiz / slots) - 6;
    debug(F101,"adjpkl new len","",pktlen);
    return(pktlen);
}

/* Set transfer mode and file naming based on comparison of system types */


VOID
whoarewe() {
#ifndef NOICP
    extern int g_xfermode;
#endif /* NOICP */

    wearealike = 0;

    debug(F101,"whoarewe xfermode","",xfermode);
#ifndef NOICP
    debug(F101,"whoarewe g_xfermode","",g_xfermode);
#endif /* NOICP */
    if (whoareu[0]) {			/* If we know partner's system type */
	char * p = (char *)whoareu;
	debug(F110,"whoarewe remote sysid",whoareu,0);
	if (!strcmp(p,cksysid))		/* Other system same as us */
	  wearealike = 1;

#ifdef UNIX
	else if (!strcmp(p,"L3"))	/* UNIX is sort of like AmigaDOS */
	  wearealike = 1;		/* (same directory separator) */
	else if (!strcmp(p,"N3"))	/* UNIX like Aegis */
	  wearealike = 1;
#else
#ifdef AMIGA
/* Like UNIX, but case distinctions are ignored and can begin with device:. */
	else if (!strcmp(p,"U1"))	/* Amiga is sort of like UNIX */
	  wearealike = 1;
	else if (!strcmp(p,"N3"))	/* Amiga is sort of like Aegis */
	  wearealike = 1;
#else
#ifdef OS2				/* (Includes Windows 95/NT) */

	/* DOS, GEMDOS, Windows 3.x, Windows 95, Windows NT */
	/* All "the same" for FAT partitions but all bets off otherwise */
	/* so this part needs some refinement ...  */

	else if (!strcmp(p,"U8"))	/* MS-DOS */
	  wearealike = 1;
	else if (!strcmp(p,"UO"))	/* OS/2 */
	  wearealike = 1;
	else if (!strcmp(p,"UN"))	/* Windows NT or 95 */
	  wearealike = 1;
	else if (!strcmp(p,"K2"))	/* GEMDOS */
	  wearealike = 1;
#else
#ifdef GEMDOS
	else if (!strcmp(p,"U8"))
	  wearealike = 1;
	else if (!strcmp(p,"UO"))
	  wearealike = 1;
	else if (!strcmp(p,"UN"))
	  wearealike = 1;
	else if (!strcmp(p,"K2"))
	  wearealike = 1;
#endif /* GEMDOS */
#endif /* OS2 */
#endif /* AMIGA */
#endif /* UNIX */

	/* Get here with wearealike == 1 if system types match */

	debug(F101,"whoarewe wearealike","",wearealike);
	if (!wearealike)		/* Not alike */
	  return;

	fncnv = XYFN_L;			/* Alike, so literal filenames */
	debug(F101,"whoarewe setting fncnv","",fncnv);

	if (xfermode == XMODE_A) {	/* Current xfer mode is auto */
#ifdef VMS
	    binary = XYFT_L;		/* For VMS-to-VMS, use labeled */
#else
#ifdef OS2
	    /* OS/2 but not Windows */
	    if (!strcmp(cksysid,"UO") && !strcmp((char *)whoareu,"UO"))
	      binary = XYFT_L;		/* For OS/2-to-OS/2, use labeled */
#else
	    binary = XYFT_B;		/* For all others use binary */
#endif /* OS2 */
#endif /* VMS */
	    gnf_binary = binary;	/* Prevailing type for gnfile() */
	    debug(F101,"whoarewe setting binary","",binary);
	}
    }
}
#endif /* NOXFER */

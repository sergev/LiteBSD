/*  C K C F N 3  --  Packet buffer management for C-Kermit  */

/* (plus assorted functions tacked on at the end) */

/*
  Author: Frank da Cruz <fdc@columbia.edu>,
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 2010,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/
/*
 Note -- if you change this file, please amend the version number and date at
 the top of ckcfns.c accordingly.
*/

#include "ckcsym.h"
#include "ckcdeb.h"
#include "ckcasc.h"
#include "ckcker.h"
#include "ckcxla.h"

/*  C K M K D I R  --  Create a directory  */
/*
  Call with:
    int fc    = 0 to create, nonzero to remove, a directory.
    char * s  = pointer to name of directory to create or remove.
    char ** r = address of pointer to return name or message.
    int m     = 1 to print error messages, 0 to be silent.
    int cvt   = 1 means convert s from standard format to local format;
                0 means use s as is.
  Returns:
    0 on success (directory was created or removed).
   -1 when attempt to create the directory failed.
   -2 on internal error (e.g. no code for creating directories).
  On success, the name is pointed to by p.
  On failure, the reason is pointed to by p.
*/
#ifdef CK_MKDIR
static char ckmkdbuf[CKMAXPATH+1];
#else
#ifdef datageneral
static char ckmkdbuf[CKMAXPATH+1];
#endif /* datageneral */
#endif /* CK_MKDIR */

#ifdef CK_MKDIR
int
ckmkdir(fc,s,r,m,cvt) int fc; char * s; char ** r; int m; int cvt; {
    int x, rc = -2;
    char tmpbuf[CKMAXPATH+1];
    char buf2[CKMAXPATH+1];
    if (!s) s = "";
    debug(F110,"ckmkdir 1 fc",s,fc);
    if (!*s) {
	ckmakmsg(ckmkdbuf,
		 CKMAXPATH+1,
		 (fc == 0) ? "mkdir" : "rmdir",
		 ": no name given",
		 NULL,
		 NULL
		 );
	*r = ckmkdbuf;
	return(-2);
    }
#ifdef datageneral
/* Come back and make this nicer later if anybody notices */
    if (fc == 0) {			/* mkdir */
	rc = createdir(s,0);
    } else {				/* rmdir */
	/* AOS/VS rmdir() is a no-op. */
	ckmakmsg(tmpbuf,CKMAXPATH+1,"delete ",s,NULL,NULL);
	debug(F110,"ckmkdir 2",tmpbuf,0);
	rc = system(tmpbuf);
    }
    *r = NULL;
#else /* not datageneral */

/* First make sure the name has an acceptable directory-name format */

#ifdef VMS
    {
	char *p = s;
	int lb = 0, rb = 0, sl = 0;
	while (*p) {
	    if      (*p == '[' || *p == '<') lb++;   /* Count brackets */
	    else if (*p == ']' || *p == '>') rb++;
	    else if (*p == '/') 	     sl++;	/* and slashes */
	    p++;
	}
	if (lb != 1 && rb != 1 && sl == 0 && p > s && *(p-1) != ':') {
	    /* Probably just a word - convert to VMS format */
	    ckmakmsg(buf2,
		     CKMAXPATH+1,
		     "[",
		     (*s == '.') ? "" : ".",
		     s,
		     "]"
		     );
	    s = buf2;
	} else if (lb == 0 && rb == 0 && sl != 0 && p > s && *(p-1) != ':') {
	    int flag = 0;
	    /* Seems to be in UNIX format */
	    x = strlen(s);
	    if (x > 0 && s[x-1] != '/')
	      flag = 1;
	    ckmakmsg(buf2,CKMAXPATH+1,s,flag ? "/" : "",NULL,NULL);
	    s = buf2;
	}
	if (s == buf2) {
	    ckstrncpy(tmpbuf,s,CKMAXPATH+1);
	    s = tmpbuf;
	}
	debug(F110,"ckmkdir 2+VMS",s,0);
    }
#else
#ifdef UNIXOROSK
#ifdef DTILDE
    s = tilde_expand(s);
#endif /* DTILDE */
    ckstrncpy(tmpbuf,s,CKMAXPATH+1);
    s = tmpbuf;
    x = strlen(s);
    if (x > 0 && s[x-1] != '/') {	/* Must end in "/" for zmkdir() */
	s[x] = '/';
	s[x+1] = NUL;
	debug(F110,"ckmkdir 2+UNIXOROSK",s,0);
    }
#else /* UNIXOROSK */
#ifdef OS2
    ckstrncpy(tmpbuf,s,CKMAXPATH+1);
    s = tmpbuf;
    x = strlen(s);
    if (fc == 0 && x > 0 && s[x-1] != '/') { /* Must end in "/" for zmkdir() */
	s[x] = '/';
	s[x+1] = NUL;
	debug(F110,"ckmkdir 2+OS2",s,0);
    }
#endif /* OS2 */
#endif /* UNIXOROSK */
#endif /* VMS */
#ifdef NZLTOR
    /* Server is calling us, so convert to local format if necessary */
    if (cvt) {
	nzrtol(s,(char *)buf2,1,PATH_ABS,CKMAXPATH);
	s = buf2;
	debug(F110,"ckmkdir 3",s,0);
    }
#endif /* NZLTOR */
    debug(F110,"ckmkdir 4",s,0);
    if (fc == 0) {			/* Making */
#ifdef CK_MKDIR
	rc = zmkdir(s);
#else
#ifdef NT
	rc = _mkdir(s);
#else
	rc = mkdir(s,0777);
#endif /* NT */
#endif /* CK_MKDIR */
    } else {				/* Removing */
#ifdef ZRMDIR
	rc = zrmdir(s);
#else
#ifdef NT
	rc = _rmdir(s);
#else
#ifdef OSK
	rc = -2;
#else
	rc = rmdir(s);
#endif /* OSK */
#endif /* NT */
#endif /* ZRMDIR */
    }
#endif /* datageneral */
    debug(F101,"ckmkdir rc","",rc);
    if (rc == -2) {
	ckmakmsg(ckmkdbuf,
		 CKMAXPATH,
		 "Directory ",
		 (fc == 0) ? "creation" : "removal",
		 "not implemented in this version of C-Kermit",
		 NULL
		 );
	*r = ckmkdbuf;
	if (m) printf("%s\n",*r);
    } else if (rc < 0) {
	if (m) perror(s);
	ckmakmsg(ckmkdbuf,CKMAXPATH,s,": ",ck_errstr(),NULL);
	*r = ckmkdbuf;
    } else if (fc == 0 && zfnqfp(s,CKMAXPATH,ckmkdbuf)) {
	*r = ckmkdbuf;
    } else if (fc != 0) {
	ckmakmsg(ckmkdbuf,CKMAXPATH,s,": removed",NULL,NULL);
	*r = ckmkdbuf;
    }
    return(rc);
}
#endif /* CK_MKDIR */

#ifndef NOXFER				/* Rest of this file... */

#ifndef NODISPO
#ifdef pdp11
#define NODISPO
#endif /* pdpd11 */
#endif /* NODISPO */

extern int pipesend;
#ifdef PIPESEND
extern char ** sndfilter;
#endif /* PIPESEND */

extern int unkcs, wmax, wcur, discard, bctu, bctl, local, fdispla, what,
    sendmode, opnerr, dest, epktrcvd, epktsent, filestatus, eofmethod, dispos,
    fncnv, fnrpath;

extern char * ofn2;
extern char * rfspec, * sfspec, * prfspec, * psfspec, * rrfspec, * prrfspec;
extern char ofn1[];
extern int ofn1x;
extern char * ofperms;

#ifdef VMS
extern int batch;
#else
extern int backgrd;
#endif /* VMS */

extern int xflg, remfile, remappd;
extern CHAR *data;
extern char filnam[];
#ifndef NOFRILLS
extern int rprintf, rmailf;		/* REMOTE MAIL, PRINT */
char optbuf[OPTBUFLEN];			/* Options for MAIL or REMOTE PRINT */
#endif /* NOFRILLS */
extern int wslots;
extern int fblksiz, frecl, forg, frecfm, fncact, fncsav, fcctrl, lf_opts;
extern CHAR * srvcmd;
extern int srvcmdlen;

extern int binary, spsiz;
extern int pktnum, cxseen, czseen, nfils, stdinf;
extern int memstr, stdouf, keep, sndsrc, hcflg;
extern int server, en_cwd, en_mai, en_pri;

/* Attributes in/out enabled flags */

extern int
  atenci, atenco, atdati, atdato, atleni, atleno, atblki, atblko,
  attypi, attypo, atsidi, atsido, atsysi, atsyso, atdisi, atdiso;

#ifdef CK_PERMS
extern int atlpri, atlpro, atgpri, atgpro;
#endif /* CK_PERMS */

#ifdef STRATUS
extern int atfrmi, atfrmo, atcrei, atcreo, atacti, atacto;
#endif /* STRATUS */

#ifdef datageneral
extern int quiet;
#endif /* datageneral */

extern long filcnt;
extern CK_OFF_T fsize, ffc, tfc, sendstart, calibrate;
CK_OFF_T rs_len;

#ifndef NOCSETS
_PROTOTYP (VOID setxlate, (void));
extern int tcharset, fcharset;
extern int ntcsets, xlatype, xfrxla;
extern struct csinfo tcsinfo[], fcsinfo[];
#endif /* NOCSETS */

/* Variables global to Kermit that are defined in this module */

#ifdef CKXXCHAR				/* DOUBLE / IGNORE char table */
int dblflag = 0;
int ignflag = 0;
short dblt[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
#endif /* CKXXCHAR */

int winlo;				/* packet number at low window edge  */

int sbufnum;				/* number of free buffers */
int dum001 = 1234;			/* protection... */
int sbufuse[MAXWS];			/* buffer in-use flag */
int dum003 = 1111;
int rbufnum;				/* number of free buffers */
int dum002 = 4321;			/* more protection */
int rbufuse[MAXWS];			/* buffer in-use flag */
int sseqtbl[64];			/* sequence # to buffer # table */
int rseqtbl[64];			/* sequence # to buffer # table */
int sacktbl[64];			/* sequence # ack table */

int o_isopen = 0, i_isopen = 0;		/* Input & output files are open */

#ifdef DYNAMIC
struct pktinfo *s_pkt = NULL;		/* array of pktinfo structures */
struct pktinfo *r_pkt = NULL;		/* array of pktinfo structures */
#else
struct pktinfo s_pkt[MAXWS];		/* array of pktinfo structures */
struct pktinfo r_pkt[MAXWS];		/* array of pktinfo structures */
#endif /* DYNAMIC */

#ifdef DEBUG
char xbuf[200];				/* For debug logging */
#endif /* DEBUG */

#ifdef DYNAMIC
CHAR *bigsbuf = NULL, *bigrbuf = NULL;
#else
char bigsbt[8];				/* Protection (shouldn't need this). */
					/* BUT DON'T REMOVE IT! */
CHAR bigsbuf[SBSIZ + 5];		/* Send-packet buffer area */
char bigrbt[8];				/* Safety padding */
CHAR bigrbuf[RBSIZ + 5];		/* Receive-packet area */
#endif
int bigsbsiz = SBSIZ;			/* Sizes of big send & rcv buffers. */
int bigrbsiz = RBSIZ;

#ifdef VMS
int zchkpath(char *s);
#endif /* VMS */

/* FUNCTIONS */

VOID
dofast() {
    long maxbufsiz = RBSIZ;		/* Configuration parameters */
    int maxpktsiz = MAXSP;
    extern int spsizf,			/* For bug in IRIX Telnet server */
      rpsiz, urpsiz, spsizr, spmax, wslotr;
    extern struct ck_p ptab[];

    if (maxpktsiz < 40)			/* Long packet length */
      maxpktsiz = 40;
    else if (maxpktsiz > 4000)
      maxpktsiz = 4000;
    wslotr = maxbufsiz / maxpktsiz;
    if (wslotr > MAXWS)			/* Window slots */
      wslotr = MAXWS;
    if (wslotr > 30)
      wslotr = 30;
    else if (wslotr < 1)
      wslotr = 1;
    urpsiz = adjpkl(maxpktsiz,wslotr,maxbufsiz);
    ptab[PROTO_K].rpktlen = urpsiz;
    rpsiz = (urpsiz > 94) ? 94 : urpsiz; /* Max non-long packet length */
    debug(F111,"dofast","uprsiz",urpsiz);
#ifdef IRIX
#ifndef IRIX65
    /* IRIX Telnet server chops off writes longer than 4K */
    spsiz = spmax = spsizr = urpsiz;
    debug(F101,"doarg Q IRIX spsiz","",spsiz);
    spsizf = 1;
#endif /* IRIX65 */
#endif /* IRIX */
#ifdef CK_SPEED
    setprefix(PX_CAU);			/* Cautious unprefixing */
#endif /* CK_SPEED */
}


/* For sanity, use "i" for buffer slots, "n" for packet numbers. */

/* I N I B U F S */

/*
  Allocates the big send and receive buffers.
  Call with size for big send buffer (s) and receive buffer (r).
  These sizes can be different.
  Attempts to allocate buffers of the requested size, but if it can't,
  it will allocate smaller ones.
  Sets global variables bigsbsiz and bigrbsiz to the actual sizes,
  and bigsbuf and bigrbuf pointing to the actual buffers.
  Designed to be called more than once.
  Returns 0 on success, -1 on failure.
*/

CHAR *bigbufp = NULL;

int
inibufs(s,r) int s, r; {
#ifdef DYNAMIC
    unsigned
      int size;
#ifdef OS2
    unsigned		/* Don't you wish everybody had unsigned long... */
#endif /* OS2 */
      long z;
    int x;

    debug(F101,"inibufs s","",s);
    debug(F101,"inibufs r","",r);

    if (s < 80 || r < 80) return(-1);	/* Validate arguments. */

    if (!s_pkt) {			/* Allocate packet info structures */
	if (!(s_pkt = (struct pktinfo *) malloc(sizeof(struct pktinfo)*MAXWS)))
	  fatal("ini_pkts: no memory for s_pkt");
    }
    for (x = 0; x < MAXWS; x++)
      s_pkt[x].pk_adr = NULL;		/* Initialize addresses */

    if (!r_pkt) {
	if (!(r_pkt = (struct pktinfo *) malloc(sizeof(struct pktinfo)*MAXWS)))
	  fatal("ini_pkts: no memory for s_pkt");
    }
    for (x = 0; x < MAXWS; x++)
      r_pkt[x].pk_adr = NULL;		/* Initialize addresses */

    if (!srvcmd) {			/* Allocate srvcmd buffer */
	srvcmd = (CHAR *) malloc(r + 100);
	if (!srvcmd) return(-1);
	srvcmdlen = r + 99;
	*srvcmd = NUL;
    }
    if (bigbufp) {			/* Free previous buffers, if any. */
	free(bigbufp);
	bigbufp = NULL;
    }
    size = s + r + 40;			/* Combined requested size + padding */
    z  = (unsigned) s + (unsigned) r + 40;
    debug(F101,"inibufs size 1","",size);
    debug(F101,"inibufs size z","",z);
    if ((long) size != z) {
	debug(F100,"inibufs overflow","",0);
	size = 65535;
    }

    /* Try to get the space.  If malloc fails, try to get a little less. */
    /* (Obviously, this algorithm can be refined.) */

    while (!(bigbufp = (CHAR *) malloc(size))) {
	debug(F101,"inibufs bigbuf malloc failed","",size);
	size = (size * 2) / 3;		/* Failed, cut size by 1/3. */
	if (size < 200)			/* Try again until too small. */
	  return(-1);
    }
    debug(F101,"inibufs size 2","",size); /* OK, we got some space. */

/*
  Now divide the allocated space between the send and receive buffers in the
  requested proportion.  The natural formula would be (s / (s + r)) * size
  (for the send buffer), but that doesn't work with integer arithmetic and we
  can't use floating point because some machines don't have it.  This can be
  rearranged as (s * size) / (s + r).  But (s * size) can be VERY large, too
  large for 32 bits.  So let's do it this way.  This arithmetic works for
  buffer sizes up to about 5,000,000.
*/
#define FACTOR 20L
    z = ( (long) s * FACTOR ) / ( (long) s + (long) r );
    x = ( z * ( (long) size / FACTOR ) );
    if (x < 0) return(-1);		/* Catch overflow */

    bigsbsiz = x - 5;			/* Size of send buffer */
    bigsbuf = bigbufp;			/* Address of send buffer */
    debug(F101,"inibufs bigsbsiz","",bigsbsiz);

    bigrbsiz = size - x - 5;		/* Size of receive buffer */
    bigrbuf = bigbufp + x;		/* Addresss of receive buffer */
    debug(F101,"inibufs bigrbsiz","",bigrbsiz);

    return(0);				/* Success */
#else					/* No dynamic allocation */
    bigsbsiz = SBSIZ;			/* Just use the symbols */
    bigrbsiz = RBSIZ;			/* ... */
    return(0);				/* Success. */
#endif /* DYNAMIC */
}


/* M A K E B U F  --  Makes and clears a new buffers.  */

/* Call with: */
/*  slots:  number of buffer slots to make, 1 to 32 */
/*  bufsiz: size of the big buffer */
/*  buf:    address of the big buffer */
/*  xx:     pointer to array of pktinfo structures for these buffers */

/* Subdivides the big buffer into "slots" buffers. */

/* Returns: */
/*  -1 if too many or too few slots requested,     */
/*  -2 if slots would be too small.      */
/*   n (positive) on success = size of one buffer. */
/*   with pktinfo structure initialized for this set of buffers. */

int
makebuf(slots,bufsiz,buf,xx)
/* makebuf */ int slots, bufsiz; CHAR buf[]; struct pktinfo *xx; {

    CHAR *a;
    int i, size;

    debug(F101,"makebuf","",slots);
    debug(F101,"makebuf bufsiz","",bufsiz);
    debug(F101,"makebuf MAXWS","",MAXWS);

    if (slots > MAXWS || slots < 1) return(-1);
    if (bufsiz < slots * 10 ) return(-2);

    size = bufsiz / slots;		/* Divide up the big buffer. */
    a = buf;				/* Address of first piece. */

    for (i = 0; i < slots; i++) {
	struct pktinfo *x = &xx[i];
	x->bf_adr = a;			/* Address of this buffer */
	x->bf_len = size;		/* Length of this buffer */
	x->pk_len = 0;			/* Length of data field */
        x->pk_typ = ' ';		/* packet type */
	x->pk_seq = -1;			/* packet sequence number */
        x->pk_rtr = 0;			/* retransmissions */
	*a = '\0';			/* Clear the buffer */
	a += size;			/* Position to next buffer slot */
    }
    return(size);
}

/*  M A K S B U F  --  Makes the send-packet buffer  */

int
mksbuf(slots) int slots; {
    int i, x;
    sbufnum = 0;
    if ((x = makebuf(slots,bigsbsiz,bigsbuf,s_pkt)) < 0) {
	debug(F101,"mksbuf makebuf return","",x);
	return(x);
    }
    debug(F101,"mksbuf makebuf return","",x);
    for (i = 0; i < 64; i++) {		/* Initialize sequence-number- */
	sseqtbl[i] = -1;		/* to-buffer-number table. */
        sacktbl[i] = 0;
    }
    for (i = 0; i < MAXWS; i++)
      sbufuse[i] = 0;			/* Mark each buffer as free */
    sbufnum = slots;
    wcur = 0;
    return(x);
}

/*  M A K R B U F  --  Makes the receive-packet buffer  */

int
mkrbuf(slots) int slots; {
    int i, x;
    rbufnum = 0;
    if ((x = makebuf(slots,bigrbsiz,bigrbuf,r_pkt)) < 0) {
	debug(F101,"mkrbuf makebuf return","",x);
	return(x);
    }
    debug(F101,"mkrbuf makebuf return","",x);
    for (i = 0; i < 64; i++) {		/* Initialize sequence-number- */
	rseqtbl[i] = -1;		/* to-buffer-number table. */
    }
    for (i = 0; i < MAXWS; i++)
      rbufuse[i] = 0;			/* Mark each buffer as free */
    rbufnum = slots;
    wcur = 0;
    return(x);
}

/*  W I N D O W  --  Resize the window to n  */

int
window(n) int n; {
    debug(F101,"window","",n);
    if (n < 1 || n > MAXWS) return(-1);
    if (mksbuf(n) < 0) return(-1);
    if (mkrbuf(n) < 0) return(-1);
    wslots = n;
#ifdef DEBUG
    if (deblog) dumpsbuf();
    if (deblog) dumprbuf();
#endif /* DEBUG */
    return(0);
}

/*  G E T S B U F  --  Allocate a send-buffer.  */

/*  Call with packet sequence number to allocate buffer for. */
/*  Returns: */
/*   -4 if argument is invalid (negative, or greater than 63) */
/*   -3 if buffers were thought to be available but really weren't (bug!) */
/*   -2 if the number of free buffers is negative (bug!) */
/*   -1 if no free buffers. */
/*   0 or positive, packet sequence number, with buffer allocated for it. */

int
getsbuf(n) int n; {			/* Allocate a send-buffer */
    int i;
    CHAR * p = NULL;
    if (n < 0 || n > 63) {
	debug(F101,"getsbuf bad arg","",n);
	return(-4);	/* Bad argument */
    }
    debug(F101,"getsbuf packet","",n);
    /* debug(F101,"getsbuf, sbufnum","",sbufnum); */
    if (sbufnum == 0) return(-1);	/* No free buffers. */
    if (sbufnum < 0) return(-2);	/* Shouldn't happen. */
    for (i = 0; i < wslots; i++)	/* Find the first one not in use. */
      if (sbufuse[i] == 0) {		/* Got one? */
	  sbufuse[i] = 1;		/* Mark it as in use. */
	  sbufnum--;			/* One less free buffer. */
	  *s_pkt[i].bf_adr = '\0';	/* Zero the buffer data field */
	  s_pkt[i].pk_seq = n;		/* Put in the sequence number */
          sseqtbl[n] = i;		/* Back pointer from sequence number */
          sacktbl[n] = 0;		/* ACK flag */
	  s_pkt[i].pk_len = 0;		/* Data field length now zero. */
	  s_pkt[i].pk_typ = ' ';	/* Blank the packet type too. */
	  s_pkt[i].pk_rtr = 0;		/* Zero the retransmission count */
	  p = s_pkt[i].bf_adr + 7;	/* Set global "data" address. */
	  debug(F101,"getsbuf p","",0);
	  data = p;
	  if (!data) {
	      debug(F100,"getsbuf data == NULL","",0);
              return(-3);
          }
	  if ((what & (W_SEND|W_REMO)) && (++wcur > wmax))
	    wmax = wcur;		/* For statistics. */
	  /* debug(F101,"getsbuf wcur","",wcur); */
	  return(n);			/* Return its index. */
      }
    sbufnum = 0;			/* Didn't find one. */
    return(-3);				/* Shouldn't happen! */
}

int
getrbuf() {				/* Allocate a receive buffer */
    int i;
#ifdef COMMENT
    /* This code is pretty stable by now... */
    /* Looks like we might need this after all */
    debug(F101,"getrbuf rbufnum","",rbufnum);
    debug(F101,"getrbuf wslots","",wslots);
    debug(F101,"getrbuf dum002","",dum002);
    debug(F101,"getrbuf dum003","",dum003);
#endif /* COMMENT */
    if (rbufnum == 0) return(-1);	/* No free buffers. */
    if (rbufnum < 0) return(-2);	/* Shouldn't happen. */
    for (i = 0; i < wslots; i++)	/* Find the first one not in use. */
      if (rbufuse[i] == 0) {		/* Got one? */
	  rbufuse[i] = 1;		/* Mark it as in use. */
	  *r_pkt[i].bf_adr = '\0';	/* Zero the buffer data field */
	  rbufnum--;			/* One less free buffer. */
	  debug(F101,"getrbuf new rbufnum","",rbufnum);
	  if ((what & W_RECV) && (++wcur > wmax))
	    wmax = wcur;		/* For statistics. */
	  /* debug(F101,"getrbuf wcur","",wcur); */
	  return(i);			/* Return its index. */
      }
    /* debug(F101,"getrbuf foulup","",i); */
    rbufnum = 0;			/* Didn't find one. */
    return(-3);				/* Shouldn't happen! */
}

/*  F R E E S B U F  --  Free send-buffer for given packet sequence number */

/*  Returns:  */
/*   1 upon success  */
/*  -1 if specified buffer does not exist */

int
freesbuf(n) int n; {			/* Release send-buffer for packet n. */
    int i;

    debug(F101,"freesbuf","",n);
    if (n < 0 || n > 63)		/* No such packet. */
      return(-1);
    i = sseqtbl[n];			/* Get the window slot number. */
    if (i > -1 && i <= wslots) {
	sseqtbl[n] = -1;		/* If valid, remove from seqtbl */
 	sbufnum++;			/* and count one more free buffer */
	sbufuse[i] = 0;			/* and mark it as free, */
	if (what & (W_SEND|W_REMO))	/* decrement active slots */
	  wcur--;			/* for statistics and display. */
    } else {
	debug(F101," sseqtbl[n]","",sseqtbl[n]);
	return(-1);
    }

/* The following is done only so dumped buffers will look right. */

    if (1) {
	*s_pkt[i].bf_adr = '\0';	/* Zero the buffer data field */
	s_pkt[i].pk_seq = -1;		/* Invalidate the sequence number */
	s_pkt[i].pk_len = 0;		/* Data field length now zero. */
	s_pkt[i].pk_typ = ' ';		/* Blank the packet type too. */
	s_pkt[i].pk_rtr = 0;		/* And the retries field. */
    }
    return(1);
}

int
freerbuf(i) int i; {			/* Release receive-buffer slot "i". */
    int n;

/* NOTE !! Currently, this function frees the indicated buffer, but */
/* does NOT erase the data.  The program counts on this.  Will find a */
/* better way later.... */

    /* debug(F101,"freerbuf, slot","",i); */
    if (i < 0 || i >= wslots) {		/* No such slot. */
	debug(F101,"freerbuf no such slot","",i);
	return(-1);
    }
    n = r_pkt[i].pk_seq;		/* Get the packet sequence number */
    debug(F101,"freerbuf packet","",n);
    if (n > -1 && n < 64)		/* If valid, remove from seqtbl */
      rseqtbl[n] = -1;
    if (rbufuse[i] != 0) {		/* If really allocated, */
	rbufuse[i] = 0;			/* mark it as free, */
	rbufnum++;			/* and count one more free buffer. */
	if (what & W_RECV)		/* Keep track of current slots */
	  wcur--;			/*  for statistics and display */
	debug(F101,"freerbuf rbufnum","",rbufnum);
    }

/* The following is done only so dumped buffers will look right. */

    if (1) {
     /* *r_pkt[i].bf_adr = '\0'; */	/* Zero the buffer data field */
	r_pkt[i].pk_seq = -1;		/* And from packet list */
	r_pkt[i].pk_len = 0;		/* Data field length now zero. */
	r_pkt[i].pk_typ = ' ';		/* Blank the packet type too. */
	r_pkt[i].pk_rtr = 0;		/* And the retries field. */
    }
    return(1);
}

/* This is like freerbuf, except it's called with a packet sequence number */
/* rather than a packet buffer index. */

VOID
freerpkt(seq) int seq; {
    int k;
    debug(F101,"freerpkt seq","",seq);
    k = rseqtbl[seq];
    /* debug(F101,"freerpkt k","",k); */
    if (k > -1) {
	k = freerbuf(k);
	/* debug(F101,"freerpkt freerbuf","",k); */
    }
}


/*  C H K W I N  --  Check if packet n is in window. */

/*  Returns: */
/*    0 if it is in the current window,  */
/*   +1 if it would have been in previous window (e.g. if ack was lost), */
/*   -1 if it is outside any window (protocol error),   */
/*   -2 if either of the argument packet numbers is out of range.  */

/* Call with packet number to check (n), lowest packet number in window */
/* (bottom), and number of slots in window (slots).  */

int
chkwin(n,bottom,slots) int n, bottom, slots; {
    int top, prev;

    debug(F101,"chkwin packet","",n);
    debug(F101,"chkwin winlo","",bottom);
    debug(F101,"chkwin slots","",slots);

/* First do the easy and common cases, where the windows are not split. */

    if (n < 0 || n > 63 || bottom < 0 || bottom > 63)
      return(-2);

    if (n == bottom) return(0);		/* In a perfect world... */

    top = bottom + slots;		/* Calculate window top. */
    if (top < 64 && n < top && n >= bottom)
      return(0);			/* In current window. */

    prev = bottom - slots;		/* Bottom of previous window. */
    if (prev > -1 && n < bottom && n > prev)
      return(1);			/* In previous. */

/* Now consider the case where the current window is split. */

    if (top > 63) {			/* Wraparound... */
	top -= 64;			/* Get modulo-64 sequence number */
	if (n < top || n >= bottom) {
	    return(0);			/* In current window. */
	} else {			/* Not in current window. */
	    if (n < bottom && n >= prev) /* Previous window can't be split. */
	      return(1);		/* In previous window. */
	    else
	      return(-1);		/* Not in previous window. */
	}
    }

/* Now the case where current window not split, but previous window is. */

    if (prev < 0) {			/* Is previous window split? */
	prev += 64;			/* Yes. */
	if (n < bottom || n >= prev)
	  return(1);			/* In previous window. */
    } else {				/* Previous window not split. */
	if (n < bottom && n >= prev)
	  return(1);			/* In previous window. */
    }

/* It's not in the current window, and not in the previous window... */

    return(-1);				/* So it's not in any window. */
}

int
dumpsbuf() {				/* Dump send-buffers */
#ifdef DEBUG
    int j, x, z;			/* to debug log. */

    if (! deblog) return(0);
    x = zsoutl(ZDFILE,"SEND BUFFERS:");
    if (x < 0) {
	deblog = 0;
	return(0);
    }
    x=zsoutl(ZDFILE,"buffer inuse address length data type seq flag retries");
    if (x < 0) {
	deblog = 0;
	return(0);
    }
    for (j = 0; j < wslots; j++) {
	if (!sbufuse[j])
	  continue;
	z = ((unsigned long)(s_pkt[j].bf_adr)) & 0xffff;

	sprintf(xbuf,			/* safe (200) */
		"%4d%6d%10d%5d%6d%4c%5d%6d\n",
		j,
		sbufuse[j],
		/* Avoid warnings when addresses are bigger than ints */
		z,
		s_pkt[j].bf_len,
		s_pkt[j].pk_len,
		s_pkt[j].pk_typ,
		s_pkt[j].pk_seq,
		s_pkt[j].pk_rtr
		);
	if (zsout(ZDFILE,xbuf) < 0)  {
	    deblog = 0;
	    return(0);
	}
	if (s_pkt[j].pk_adr) {
	    x = (int)strlen((char *) s_pkt[j].pk_adr);
	    if (x)
	      sprintf(xbuf,		/* safe (checked) */
		      "[%.72s%s]\n",s_pkt[j].pk_adr, x > 72 ? "..." : "");
	    else
	      sprintf(xbuf,"[(empty string)]\n"); /* safe (200) */
	} else {
	    sprintf(xbuf,"[(null pointer)]\n");	/* safe (200) */
	}
	if (zsout(ZDFILE,xbuf) < 0) {
	    deblog = 0;
	    return(0);
	}
    }
    sprintf(xbuf,"free: %d, winlo: %d\n", sbufnum, winlo); /* safe (200) */
    if (zsout(ZDFILE,xbuf) < 0) {
	deblog = 0;
	return(0);
    }
#endif /* DEBUG */
    return(0);
}
int
dumprbuf() {				/* Dump receive-buffers */
#ifdef DEBUG
    int j, x, z;
    if (! deblog) return(0);
    if (zsoutl(ZDFILE,"RECEIVE BUFFERS:") < 0) {
	deblog = 0;
	return(0);
    }
    x=zsoutl(ZDFILE,"buffer inuse address length data type seq flag retries");
    if (x < 0) {
	deblog = 0;
	return(0);
    }
    for ( j = 0; j < wslots; j++ ) {
	if (!rbufuse[j])
	  continue;
	z = ((unsigned long)(r_pkt[j].bf_adr)) & 0xffff;
	sprintf(xbuf,			/* 200, safe */
		"%4d%6d%10d%5d%6d%4c%5d%6d\n",
		j,
		rbufuse[j],
		/* Avoid warnings when addresses are bigger than ints */
		z,
		r_pkt[j].bf_len,
		r_pkt[j].pk_len,
		r_pkt[j].pk_typ,
		r_pkt[j].pk_seq,
		r_pkt[j].pk_rtr
		);
	if (zsout(ZDFILE,xbuf) < 0) {
	    deblog = 0;
	    return(0);
	}
	x = (int)strlen((char *)r_pkt[j].bf_adr);
	sprintf(xbuf,			/* safe (checked) */
		"[%.72s%s]\n",r_pkt[j].bf_adr, x > 72 ? "..." : "");
	if (zsout(ZDFILE,xbuf) < 0)  {
	    deblog = 0;
	    return(0);
	}
    }
    sprintf(xbuf,"free: %d, winlo: %d\n", rbufnum, winlo); /* safe (200) */
    if (zsout(ZDFILE,xbuf) < 0)  {
	deblog = 0;
	return(0);
    }
#endif /* DEBUG */
    return(0);
}

/*  S A T T R  --  Send an Attribute Packet  */

/*
  Sends attribute packet(s) for the current file.  If the info will not
  fit into one packet, it can be called repeatedly until all the fields
  that will fit are sent.

  Call with:
    xp == 0 if we're sending a real file (F packet), or:
    xp != 0 for screen data (X packet).
  And:
    flag == 1 for first A packet
    flag == 0 for subsequent A packets.
  Returns:
    1 or greater if an A packet was sent, or:
    0 if an S-packet was not sent because there was no data to send or
      there was no data left that was short enough to send, or:
   -1 on any kind of error.
*/

/* (don't) #define TSOFORMAT */
/* which was only for making C-Kermit send TSO-Kermit-like A packets */
/* to try to track down a problem somebody reported... */

int
sattr(xp, flag) int xp, flag; {		/* Send Attributes */

    static int max;			/* Maximum length for Attributes */
    static short done[95];		/* Field-complete array */
    static struct zattr x;		/* File attribute struct */
    static char xdate[24];

    extern char * cksysid;

    /* Some extra flags are used because the "done" array is sparse */

    int i, j, rc, aln, left = 0, numset = 0, xbin = 0; /* Workers */
    int notafile = 0;
    char *tp, c;

    notafile = sndarray || pipesend ||
#ifdef PIPESEND
      sndfilter ||
#endif /* PIPESEND */
	calibrate;

    debug(F101,"sattr flag","",flag);
    if (!flag)				/* No more attributes to send */
      if (done[xunchar('@')])
	return(0);

    /* Initialize Attribute mechanism */

    if (flag) {				/* First time here for this file? */
	initattr(&x);			/* Blank out all the fields. */
	for (j = 0; j < 95; j++)	/* Init array of completed fields */
	  done[j] = 0;
	max = maxdata();		/* Get maximum data field length */
	if (notafile || xp == 1) {	/* Is it not a real file? */
	    extern char * zzndate();
	    char * p;
	    int i;
#ifdef CALIBRATE
	    if (calibrate) {		/* Calibration run... */
		x.lengthk = calibrate / 1024L; /* We know the length */
		x.length = calibrate;
	    }
#endif /* CALIBRATE */
	    x.systemid.val = cksysid;	/* System ID */
	    x.systemid.len = (int)strlen(cksysid);
	    ckstrncpy(xdate,zzndate(),24);
	    xdate[8] = SP;
	    ztime(&p);
	    for (i = 11; i < 19; i++)	/* copy hh:mm:ss */
	      xdate[i - 2] = p[i];	/* to xdate */
	    xdate[17] = NUL;		/* terminate */
	    x.date.val = xdate;
	    x.date.len = 17;
	    debug(F111,"sattr notafile date",x.date.val,x.date.len);
	} else {			/* Real file */
	    rc = zsattr(&x);		/* Get attributes for this file  */
	    debug(F101,"sattr zsattr","",rc);
	    if (rc < 0)			/* Can't get 'em so don't send 'em */
	      return(0);
	    debug(F101,"sattr init max","",max);
	}
    }
    if (nxtpkt() < 0)			/* Got 'em, get next packet number */
      return(-1);			/* Bad news if we can't */

    i = 0;				/* Init data field character number */

    /* Do each attribute using first-fit method, marking as we go */
    /* This is rather long and repititious - could be done more cleverly */

    if (atsido && !done[xunchar(c = '.')]) { /* System type */
	if (max - i >= x.systemid.len + 2) { /* Enough space ? */
	    data[i++] = c;		     /* Yes, add parameter */
	    data[i++] = tochar(x.systemid.len);	 /* Add length */
	    for (j = 0; j < x.systemid.len; j++) /* Add data */
	      data[i++] = x.systemid.val[j];
	    numset++;			/* Count that we did at least one */
	    done[xunchar(c)] = 1;	/* Mark this attribute as done */
	} else				/* No */
	  left++;			/* so mark this one left to do */
    }
#ifdef STRATUS
    if (atcreo && !done[xunchar(c = '$')]) { /* Creator */
	if (max - i >= x.creator.len + 2) { /* Enough space ? */
	    data[i++] = c;
	    data[i++] = tochar(x.creator.len);
	    for (j = 0; j < x.creator.len; j++)
	      data[i++] = x.creator.val[j];
	    numset++;
	    done[xunchar(c)] = 1;
	} else
	  left++;
    }
    if (atacto && !done[xunchar(c = '%')]) { /* File account */
	if (max - i >= x.account.len + 2) {
	    data[i++] = c;
	    data[i++] = tochar(x.account.len);
	    for (j = 0; j < x.account.len; j++)
	      data[i++] = x.account.val[j];
	    numset++;
	    done[xunchar(c)] = 1;
	} else
	  left++;
    }
    if (atfrmo && !done[xunchar(c = '/')]) { /* Packet data format */
	if (max - i >= x.recfm.len + 2) {
	    data[i++] = c;
	    data[i++] = tochar(x.recfm.len); /*  Copy from attr structure */
	    for (j = 0; j < x.recfm.len; j++)
	      data[i++] = x.recfm.val[j];
	    numset++;
	    done[xunchar(c)] = 1;
	} else
	  left++;
    }
#endif /* STRATUS */

    xbin =				/* Is the transfer in binary mode? */
#ifdef VMS
      binary == XYFT_I || binary == XYFT_L || /* IMAGE or LABELED */
	!strncmp(x.recfm.val,"F",1)	/* or RECFM=Fxxxxxx */
#else
      binary				/* User said SET FILE TYPE BINARY  */
#endif /* VMS */
	;

    if (attypo && !done[xunchar(c = '"')]) { /* File type */
	if (max - i >= 5) {		/* Max length for this field */
	    data[i++] = c;
	    if (xbin) {			/* Binary */
		data[i++] = tochar(2);	/*  Two characters */
		data[i++] = 'B';	/*  B for Binary */
		data[i++] = '8';	/*  8-bit bytes (note assumption...) */
#ifdef CK_LABELED
		if (binary != XYFT_L
#ifdef VMS
		    && binary != XYFT_I
#endif /* VMS */
		    )
		  binary = XYFT_B;
#endif /* CK_LABELED */
	    } else {			/* Text */
#ifdef TSOFORMAT
		data[i++] = tochar(1);	/*  One character */
		data[i++] = 'A';	/*  A = (extended) ASCII with CRLFs */
#else
		data[i++] = tochar(3);	/*  Three characters */
		data[i++] = 'A';	/*  A = (extended) ASCII with CRLFs */
		data[i++] = 'M';	/*  M for carriage return */
		data[i++] = 'J';	/*  J for linefeed */
#endif /* TSOFORMAT */

#ifdef VMS
		binary = XYFT_T;	/* We automatically detected text */
#endif /* VMS */
	    }
	    numset++;
	    done[xunchar(c)] = 1;
	} else
	  left++;
    }

#ifdef TSOFORMAT
    if (attypo && !xbin && !done[xunchar(c = '/')]) { /* Record format */
	if (max - i >= 5) {
	    data[i++] = c;
	    data[i++] = tochar(3);	/*  Three characters */
	    data[i++] = 'A';		/*  A = variable with CRLFs */
	    data[i++] = 'M';		/*  M for carriage return */
	    data[i++] = 'J';		/*  J for linefeed */
	}
    }
#endif /* TSOFORMAT */

    if (attypo && !xbin && !done[xunchar(c = '*')]) { /* Text encoding */
#ifdef NOCSETS
	if (max - i >= 3) {
	    data[i++] = c;
	    data[i++] = tochar(1);	/* Length of value is 1 */
	    data[i++] = 'A';		/* A for ASCII */
	    numset++;
	    done[xunchar(c)] = 1;
	} else
	  left++;
#else
	if (tcharset == TC_TRANSP || !xfrxla) {	/* Transfer character set */
	    if (max - i >= 3) {
		data[i++] = c;		/* Encoding */
		data[i++] = tochar(1);	/* Length of value is 1 */
		data[i++] = 'A';	/* A for ASCII (i.e. text) */
		numset++;
		done[xunchar(c)] = 1;
	    } else
	      left++;
	} else {
	    tp = tcsinfo[tcharset].designator;
	    if (!tp) tp = "";
	    aln = strlen(tp);
	    if (aln > 0) {
		if (max - i >= aln + 2) {
		    data[i++] = c;	/* Encoding */
		    data[i++] = tochar(aln+1); /* Length of designator. */
		    data[i++] = 'C'; /* Text in specified charset. */
		    for (j = 0; j < aln; j++) /* Copy designator */
		      data[i++] = *tp++; /*  Example: *&I6/100 */
		    numset++;
		    done[xunchar(c)] = 1;
		} else
		  left++;
	    } else
	      done[xunchar(c)] = 1;
	}
#endif /* NOCSETS */
    }
    if (atdato && !done[xunchar(c = '#')] && /* Creation date, if any */
	(aln = x.date.len) > 0) {
	if (max - i >= aln + 2) {
	    data[i++] = c;
	    data[i++] = tochar(aln);
	    for (j = 0; j < aln; j++)
	      data[i++] = x.date.val[j];
	    numset++;
	    done[xunchar(c)] = 1;
	} else
	  left++;
    }
    /* File length in K */
    if (atleno && !done[xunchar(c = '!')] && x.lengthk > (CK_OFF_T)-1) {
#ifdef COMMENT
	sprintf((char *) &data[i+2],"%ld",x.lengthk); /* safe */
#else
	ckstrncpy((char *)&data[i+2],ckfstoa(x.lengthk),32);
#endif	/* COMMENT */
	aln = (int)strlen((char *)(data+i+2));
	if (max - i >= aln + 2) {
	    data[i] = c;
	    data[i+1] = tochar(aln);
	    i += aln + 2;
	    numset++;
	    done[xunchar(c)] = 1;
	} else {
	    data[i] = NUL;
	    left++;
	}
    }
    /* File length in bytes */
    if (atleno && !done[xunchar(c = '1')] && x.length > (CK_OFF_T)-1) {
#ifdef COMMENT
	sprintf((char *) &data[i+2],"%ld",x.length); /* safe */
#else
	ckstrncpy((char *)&data[i+2],ckfstoa(x.length),32);
#endif	/* COMMENT */
	aln = (int)strlen((char *)(data+i+2));
	if (max - i >= aln + 2) {
	    data[i] = c;
	    data[i+1] = tochar(aln);
	    i += aln + 2;
	    numset++;
	    done[xunchar(c)] = 1;
	} else {
	    data[i] = NUL;
	    left++;
	}
    }
#ifdef CK_PERMS
    if (atlpro && !done[xunchar(c = ',')] && /* Local protection */
	(aln = x.lprotect.len) > 0 && !notafile && xp == 0) {
	if (max - i >= aln + 2) {
	    data[i++] = c;
	    data[i++] = tochar(aln);
	    for (j = 0; j < aln; j++)
	      data[i++] = x.lprotect.val[j];
	    numset++;
	    done[xunchar(c)] = 1;
	} else
	  left++;
    }
    if (atgpro && !done[xunchar(c = '-')] && /* Generic protection */
	(aln = x.gprotect.len) > 0 && !notafile && xp == 0) {
	if (max - i >= aln + 2) {
	    data[i++] = c;
	    data[i++] = tochar(aln);
	    for (j = 0; j < aln; j++)
	      data[i++] = x.gprotect.val[j];
	    numset++;
	    done[xunchar(c)] = 1;
	} else
	  left++;
    }
#endif /* CK_PERMS */
    if (atblko && fblksiz && !done[xunchar(c = '(')] &&
	!notafile && xp == 0) {	/* Blocksize */
	sprintf((char *) &data[i+2],"%d",fblksiz); /* safe */
	aln = (int)strlen((char *)(data+i+2));
	if (max - i >= aln + 2) {
	    data[i] = c;
	    data[i+1] = tochar(aln);
	    i += aln + 2;
	    numset++;
	    done[xunchar(c)] = 1;
	} else {
	    data[i] = NUL;
	    left++;
	}
    }
#ifndef NOFRILLS
    if ((rprintf || rmailf) && atdiso && /* MAIL, or REMOTE PRINT?  */
	!done[xunchar(c = '+')]) {
	aln = (int) strlen(optbuf) + 1;	/* Options, if any */
	if (max - i >= aln + 2) {
	    data[i++] = c;		/* Disposition */
	    data[i++] = tochar(aln);	/* Options, if any */
	    if (rprintf)
	      data[i++] = 'P';		/* P for Print */
	    else
	      data[i++] = 'M';		/* M for Mail */
	    for (j = 0; optbuf[j]; j++)	/* Copy any options */
	      data[i++] = optbuf[j];
	    numset++;
	    done[xunchar(c)] = 1;
	} else {
	    data[i] = NUL;
	    left++;
	}
    }
#endif /* NOFRILLS */
#ifdef CK_RESEND
    if (sendmode == SM_RESEND && !done[xunchar(c = '+')]) {
	if (max - i >= 3) {
	    data[i++] = c;		/* Disposition */
	    data[i++] = tochar(1);
	    data[i++] = 'R';		/* is RESEND */
	    numset++;
	    done[xunchar(c)] = 1;
	} else
	  left++;
    }
#endif /* CK_RESEND */

    /* End of Attributes -- to be sent only after sending all others */

    debug(F111,"sattr","@",i);
    debug(F101,"sattr numset","",numset);
    debug(F101,"sattr left","",left);

    if ((left == 0 || numset == 0) && !done[xunchar(c = '@')]) {
	if (max - i >= 3) {
	    data[i++] = c;		/* End of Attributes */
	    data[i++] = SP;		/* Length 0 */
	    data[i] = NUL;		/* Make sure it's null-terminated */
	    numset++;
	    done[xunchar(c)] = 1;
	}
    }

    /* Finished - send the packet off if we have anything in it */

    if (numset) {
	data[i] = NUL;			/* Terminate last good field */
	debug(F111,"sattr sending",data,left);
	aln = (int)strlen((char *)data); /* Get overall length of attributes */
	return(spack('A',pktnum,aln,data)); /* Send it */
    } else
      return(0);
}

static char *refused = "";

static char *reason[] = {
    "size", "type", "date", "creator", "account", "area", "password",
    "blocksize", "access", "encoding", "disposition", "protection",
    "protection", "origin", "format",
    "sys-dependent",			/* 0 */
    "size",				/* 1 */
    "2",				/* 2 */
    "3",				/* 3 */
    "4",				/* 4 */
    "5",				/* 5 */
    "6",				/* 6 */
    "7",				/* 7 */
    "8",				/* 8 */
    "9",				/* 9 */
    ":",				/* : */
    ";",				/* ; */
    "<",				/* < */
    "=",				/* = */
    ">",				/* > */
    "name",				/* ? */
    "@"
};
static int nreason = sizeof(reason) / sizeof(char *);
int rejection = -1;

char *
getreason(s) char *s; {			/* Decode attribute refusal reason */
    char c, *p;
    if (rejection == 1)			/* Kludge for SET FIL COLL DISCARD */
      return("name");			/* when other Kermit doesn't... */
    p = s;
    if (*p++ != 'N') return("");	/* Should start with N */
    else if ((c = *p) > SP) {		/* get reason, */
	rejection = c;			/* remember it, */
	c -= '!';			/* get offset */
	p = ((unsigned int) ((CHAR) c) <= (unsigned int) nreason) ?
	  reason[c] :
	    "unknown";
    }
    return(p);
}

int
rsattr(s) CHAR *s; {			/* Read response to attribute packet */
    debug(F111,"rsattr",s,*s);
    if (*s == 'N') {			/* If it's 'N' followed by anything, */
	refused = getreason((char *)s);	/* they are refusing, get reason. */
	debug(F110,"rsattr refused",refused,0);
	tlog(F110," refused:",refused,0L);
	return(-1);
    }
#ifdef CK_RESEND
    if (sendmode == SM_RESEND && *s == '1') { /* RESEND length */
	int n; CK_OFF_T z; CHAR *p;
	p = s + 1;
	n = xunchar(*p++);
	debug(F101,"rsattr RESEND n","",n);
	z = (CK_OFF_T)0;
	while (n-- > 0)			/* We assume the format is good. */
	  z = (CK_OFF_T)10 * z + (CK_OFF_T)(*p++ - '0');
	debug(F101,"rsattr RESEND z","",z);
	if (z > (CK_OFF_T)0) sendstart = z;
	debug(F101,"rsattr RESEND sendstart","",sendstart);
	if (sendstart > (CK_OFF_T)0)
	  if (zfseek(sendstart) < 0)	/* Input file is already open. */
	    return(0);
#ifdef CK_CURSES
	if (fdispla == XYFD_C)
	  xxscreen(SCR_FS,0,fsize,"");	/* Refresh file transfer display */
#endif /* CK_CURSES */
    }
#endif /* CK_RESEND */
    refused = "";
    return(0);
}

/*
  Get attributes from incoming A packet.  Returns:
   0 on success, file is to be accepted
  -1 on failure, file is to be refused
*/
int
gattr(s, yy) CHAR *s; struct zattr *yy; { /* Read incoming attribute packet */
    char c, d;
    char *ff;
    int aln, i, overflow = 0;

#ifndef NOCSETS
    extern int r_cset, axcset[];
#endif /* NOCSETS */

#define ABUFL 40			/* Temporary buffer for conversions */
    char abuf[ABUFL+1];
#define RFBUFL 10			/* Record-format buffer */
    static char rfbuf[RFBUFL+1];
#define FTBUFL 10			/* File type buffer */
    static char ftbuf[FTBUFL+1];
#define DTBUFL 40			/* File creation date */
    static char dtbuf[DTBUFL+1];
#define TSBUFL 10			/* Transfer syntax */
    static char tsbuf[TSBUFL+1];
#define IDBUFL 10			/* System ID */
    static char idbuf[IDBUFL+1];
#ifndef DYNAMIC
#define DSBUFL 100			/* Disposition */
    static char dsbuf[DSBUFL+1];
#define SPBUFL 512			/* System-dependent parameters */
    static char spbuf[SPBUFL+1];
#else
#define DSBUFL 100			/* Disposition */
    static char *dsbuf = NULL;
#define SPBUFL 512			/* System-dependent parameters */
    static char *spbuf = NULL;
#endif /* DYNAMIC */
#define RPBUFL 20			/* Attribute reply */
    static char rpbuf[RPBUFL+1];

#ifdef CK_PERMS
    static char lprmbuf[CK_PERMLEN+1];
    static char gprmbuf[2];
#endif /* CK_PERMS */

    char *rp;				/* Pointer to reply buffer */
    int retcode;			/* Return code */

    d = SP;				/* Initialize disposition */
    ff = filnam;			/* Filename returned by rcvfil */
    if (fncact == XYFX_R && ofn1x && ofn1[0]) /* But watch out for FC=RENAME */
      ff = ofn1;			/* because we haven't renamed it yet */

/* Fill in the attributes we have received */

    rp = rpbuf;				/* Initialize reply buffer */
    *rp++ = 'N';			/* for negative reply. */
    *rp = NUL;
    retcode = 0;			/* Initialize return code. */

    if (dest == DEST_P) {		/* SET DESTINATION PRINTER */
#ifdef DYNAMIC
	if (!dsbuf)
	  if ((dsbuf = malloc(DSBUFL+1)) == NULL)
	    fatal("gtattr: no memory for dsbuf");
#endif /* DYNAMIC */
	dsbuf[0] = 'P';
	dsbuf[1] = '\0';
	yy->disp.val = dsbuf;
	yy->disp.len = 1;
    }
    while (c = *s++) {			/* Get attribute tag */
	aln = xunchar(*s++);		/* Length of attribute string */
	switch (c) {
#ifdef COMMENT				/* This case combined with '1' below */
	  case '!':			/* File length in K */
	    for (i = 0; (i < aln) && (i < ABUFL); i++) /* Copy it */
	      abuf[i] = *s++;
	    abuf[i] = '\0';		/* Terminate with null */
	    if (i < aln) s += (aln - i); /* If field was too long for buffer */
	    yy->lengthk = ckatofs(abuf); /* Convert to number */
	    break;
#endif	/* COMMENT */

	  case '/':			/* Record format */
	    rfbuf[1] = NUL;
	    rfbuf[2] = NUL;
	    for (i = 0; (i < aln) && (i < RFBUFL); i++) /* Copy it */
	      rfbuf[i] = *s++;
	    rfbuf[i] = NUL;		/* Terminate with null */
	    yy->recfm.val = rfbuf;	/* Pointer to string */
	    yy->recfm.len = i;		/* Length of string */
	    if ((rfbuf[0] != 'A') ||
		(rfbuf[1] && rfbuf[1] != 'M') ||
		(rfbuf[2] && rfbuf[2] != 'J')) {
		debug(F110,"gattr bad recfm",rfbuf,0);
		*rp++ = c;
		retcode = -1;
	    }
	    break;

	  case '"':			/* File type (text, binary, ...) */
	    for (i = 0; (i < aln) && (i < FTBUFL); i++)
	      ftbuf[i] = *s++;		/* Copy it into a static string */
	    ftbuf[i] = '\0';
	    if (i < aln) s += (aln - i);
	    /* TYPE attribute is enabled? */
	    if (attypi) {
		yy->type.val = ftbuf;	/* Pointer to string */
		yy->type.len = i;	/* Length of string */
		debug(F111,"gattr file type", ftbuf, i);
		debug(F101,"gattr binary 1","",binary);
		/* Unknown type? */
		if ((*ftbuf != 'A' && *ftbuf != 'B' && *ftbuf != 'I')
#ifdef CK_LABELED
/* ... Or our FILE TYPE is LABELED and the incoming file is text... */
		    || (binary == XYFT_L && *ftbuf == 'A' && !xflg)
#endif /* CK_LABELED */
		    ) {
		    retcode = -1;	/* Reject the file */
		    *rp++ = c;
		    if (!opnerr) tlog(F100," refused: type","",0);
		    break;
		}
/*
  The following code moved here from opena() so we set binary mode
  as soon as requested by the attribute packet.  That way when the first
  data packet comes, the mode of transfer can be displayed correctly
  before opena() is called.
*/
		if (yy->type.val[0] == 'A') { /* Check received attributes. */
#ifdef VMS
		    if (binary != XYFT_I) /* VMS IMAGE overrides this */
#endif /* VMS */
		      binary = XYFT_T;	/* Set current type to Text. */
		    debug(F101,"gattr binary 2","",binary);
		} else if (yy->type.val[0] == 'B') {
#ifdef CK_LABELED
		    if (binary != XYFT_L
#ifdef VMS
			&& binary != XYFT_U /* VMS special case */
#endif /* VMS */
			)
#endif /* CK_LABELED */
#ifdef MAC
		    if (binary != XYFT_M) /* If not MacBinary... */
#endif /* MAC */
		      binary = XYFT_B;
		    debug(F101,"gattr binary 3","",binary);
		}
	    }
	    break;

	  case '#':			/* File creation date */
	    for (i = 0; (i < aln) && (i < DTBUFL); i++)
	      dtbuf[i] = *s++;		/* Copy it into a static string */
	    if (i < aln) s += (aln - i);
	    dtbuf[i] = '\0';
	    if (atdati && !xflg) {	/* Real file and dates enabled */
		yy->date.val = dtbuf;	/* Pointer to string */
		yy->date.len = i;	/* Length of string */
		if (fncact == XYFX_U) {	/* Receiving in update mode? */
		    if (zstime(ff,yy,1) > 0) { /* Compare dates */
			*rp++ = c;	/* Discard if older, reason = date. */
			if (!opnerr) tlog(F100," refused: date","",0);
			retcode = -1;	/* Rejection notice. */
		    }
		}
	    }
	    break;

	  case '(':			/* File Block Size */
	    for (i = 0; (i < aln) && (i < ABUFL); i++) /* Copy it */
	      abuf[i] = *s++;
	    abuf[i] = '\0';		/* Terminate with null */
	    if (i < aln) s += (aln - i);
	    if (atblki)
	      yy->blksize = atol(abuf); /* Convert to number */
	    break;

	  case '*':			/* Encoding (transfer syntax) */
	    for (i = 0; (i < aln) && (i < TSBUFL); i++)
	      tsbuf[i] = *s++;		/* Copy it into a static string */
	    if (i < aln) s += (aln - i);
	    tsbuf[i] = '\0';
#ifndef NOCSETS
	    xlatype = XLA_NONE;		/* Assume no translation */
#endif /* NOCSETS */
	    if (atenci) {
		char * ss;
		yy->encoding.val = tsbuf; /* Pointer to string */
		yy->encoding.len = i;	/* Length of string */
		debug(F101,"gattr encoding",tsbuf,i);
		ss = tsbuf+1;
		switch (*tsbuf) {
#ifndef NOCSETS
		  case 'A':		  /* Normal, nothing special */
		    tcharset = TC_TRANSP; /* Transparent chars untranslated */
		    debug(F110,"gattr sets tcharset TC_TRANSP","A",0);
		    break;
		  case 'C':		  /* Specified character set */
		    if (!xfrxla) {	  /* But translation disabled */
			tcharset = TC_TRANSP;
			debug(F110,"gattr sets tcharset TC_TRANSP","C",0);
			break;
		    }
#ifdef UNICODE
		    if (!strcmp("I196",ss)) /* Treat I196 (UTF-8 no level) */
		      ss = "I190";	    /* as I190 (UTF-8 Level 1) */
#endif /* UNICODE */
		    if (!strcmp("I6/204",ss)) /* Treat "Latin-1 + Euro" */
		      ss = "I6/100";	      /* as I6/100 (regular Latin-1) */
		    for (i = 0; i < ntcsets; i++) {
			if (!strcmp(tcsinfo[i].designator,ss))
			  break;
		    }
		    debug(F101,"gattr xfer charset lookup","",i);
		    if (i == ntcsets) {	/* If unknown character set, */
			debug(F110,"gattr: xfer charset unknown",ss,0);
			if (!unkcs) {	/* and SET UNKNOWN DISCARD, */
			    retcode = -1; /* reject the file. */
			    *rp++ = c;
			    if (!opnerr)
			      tlog(F100," refused: character set","",0);
			}
		    } else {
			tcharset = tcsinfo[i].code; /* it's known, use it */
			debug(F101,"gattr switch tcharset","",tcharset);
			debug(F101,"gattr fcharset","",fcharset);
			if (r_cset == XMODE_A) { /* Automatic switching? */
			    if (tcharset > -1 && tcharset <= MAXTCSETS) {
				int x;
				x = axcset[tcharset];
				if (x > 0 && x <= MAXFCSETS) {
				    fcharset = x;
				    debug(F101,"gattr switch fcharset","",x);
				}
			    }
			}
			/* Set up translation type and function */
			setxlatype(tcharset,fcharset);
		    }
		break;
#endif /* NOCSETS */
	      default:			/* Something else. */
		debug(F110,"gattr unk encoding attribute",tsbuf,0);
		if (!unkcs) {		/* If SET UNK DISC */
		    retcode = -1;
		    *rp++ = c;
		    if (!opnerr) tlog(F100," refused: encoding","",0);
		}
		break;
		}
	    }
	    break;

	  case '+':			/* Disposition */
#ifdef DYNAMIC
	    if (!dsbuf)
	      if ((dsbuf = malloc(DSBUFL+1)) == NULL)
		fatal("gtattr: no memory for dsbuf");
#endif /* DYNAMIC */
	    for (i = 0; (i < aln) && (i < DSBUFL); i++)
	      dsbuf[i] = *s++;		/* Copy it into a separate string */
	    dsbuf[i] = '\0';
	    if (i < aln) s += (aln - i);
	    rs_len = (CK_OFF_T)0;
	    if (atdisi) {		/* We are doing this attribute */
		/* Copy it into the attribute structure */
		yy->disp.val = dsbuf;	/* Pointer to string */
		yy->disp.len = i;	/* Length of string */
		d = *dsbuf;
#ifndef NODISPO
/*
  Define NODISPO to disable receipt of mail or print files and of RESEND.
*/
		if (
#ifndef datageneral			/* MAIL supported only for */
#ifndef OS2				/* UNIX, VMS, and OS-9 */
#ifndef MAC
#ifndef GEMDOS
#ifndef AMIGA
		    d != 'M' &&		/* MAIL */
#endif /* AMIGA */
#endif /* GEMDOS */
#endif /* MAC */
#endif /* OS/2 */
#endif /* datageneral */
#ifdef CK_RESEND
		    d != 'R' &&		/* RESEND */
#endif /* CK_RESEND */
		    d != 'P') {		/* PRINT */
		    retcode = -1;	/* Unknown/unsupported disposition */
		    *rp++ = c;
		    if (!opnerr) tlog(F101," refused: bad disposition","",d);
		}
		dispos = d;
		debug(F000,"gattr dispos","",dispos);
		switch (d) {
#ifndef NOFRILLS
		  case 'M':
		    if (!en_mai) {
			retcode = -1;
			*rp++ = c;
			if (!opnerr) tlog(F100," refused: mail disabled","",0);
			dispos = 0;
		    }
		    break;
#endif /* NOFRILLS */
		  case 'P':
		    if (!en_pri) {
			retcode = -1;
			*rp++ = c;
			if (!opnerr)
			  tlog(F100," refused: print disabled","",0);
			dispos = 0;
		    }
		    break;

		  case 'R':
		    dispos = 0;
#ifdef CK_RESEND
		    rs_len = zgetfs(ff); /* Get length of file */
		    debug(F111,"gattr RESEND",ff,rs_len);
#ifdef VMS
		    rs_len &= (long) -512; /* Ensure block boundary if VMS */
		    rs_len -= 512;	  /* In case last block not complete */
		    debug(F111,"gattr rs_len",ff,rs_len);
#endif /* VMS */
#ifdef COMMENT
		    if (rs_len < 0L)	/* Local file doesn't exist */
		      rs_len = 0L;
#endif /* COMMENT */
/*
  Another possibility here (or later, really) would be to check if the two
  file lengths are the same, and if so, keep the prevailing collision action
  as is (note: rs_len == length of existing file; yy->length == fsize ==
  length of incoming file).  This could be complicated, though, since
  (a) we might not have received the length attribute yet, and in fact it
  might even be in a subsequent A-packet, yet (b) we have to accept or reject
  the Recover attribute now.  So better to leave as-is.  Anyway, it's probably
  more useful this way.
*/
		    if (rs_len > (CK_OFF_T)0) {
			fncsav = fncact; /* Save collision action */
			fncact = XYFX_A; /* Switch to APPEND */
		    }
#else
		    retcode = -1;	/* This shouldn't happen */
		    *rp++ = c;		/* 'cause it wasn't negotiated. */
		    if (!opnerr) tlog(F100," refused: resend","",0);
#endif /* CK_RESEND */
		}
#else  /* NODISPO */
		retcode = -1;
		*rp++ = c;
		if (!opnerr) tlog(F100," refused: NODISPO","",0);
#endif /* NODISPO */
	    }
	    break;

	  case '.':			/* Sender's system ID */
	    for (i = 0; (i < aln) && (i < IDBUFL); i++)
	      idbuf[i] = *s++;		/* Copy it into a static string */
	    idbuf[i] = '\0';
	    if (i < aln) s += (aln - i);
	    if (atsidi) {
		yy->systemid.val = idbuf; /* Pointer to string */
		yy->systemid.len = i;	/* Length of string */
	    }
	    break;

	  case '0':			/* System-dependent parameters */
#ifdef DYNAMIC
	    if (!spbuf && !(spbuf = malloc(SPBUFL)))
		fatal("gattr: no memory for spbuf");
#endif /* DYNAMIC */
	    for (i = 0; (i < aln) && (i < SPBUFL); i++)
	      spbuf[i] = *s++;		/* Copy it into a static string */
	    spbuf[i] = '\0';
	    if (i < aln) s += (aln - i);
	    if (atsysi) {
		yy->sysparam.val = spbuf; /* Pointer to string */
		yy->sysparam.len = i;	/* Length of string */
	    }
	    break;

	  case '!':			/* File length in K */
	  case '1': {			/* File length in bytes */
	      char * l2;
	      CK_OFF_T xlen;
	      for (i = 0; (i < aln) && (i < ABUFL); i++) /* Copy it */
		abuf[i] = *s++;
	      abuf[i] = '\0';		/* Terminate with null */
	      if (i < aln) s += (aln - i);
	      if (rdigits(abuf)) {	/* Make sure string is all digits */
		  xlen = ckatofs(abuf);	/* Convert to number */
		  l2 = ckfstoa(xlen);	/* Convert number back to string */
		  if (c == '1')
		    debug(F111,"gattr length",abuf,xlen);
		  else
		    debug(F111,"gattr lengthk",abuf,xlen);
		  if (ckstrcmp(abuf,l2,-1,1)) { /* This is how we check... */
		      xlen = (CK_OFF_T)-2; /* -2 = unk, possibly too long */
		      overflow++;
		      debug(F111,"gattr overflow",
			    (c == '1') ? "length" : "lengthk",
			    xlen);
		  }
		  if (c == '1') {
		      yy->length = xlen;
		      debug(F101,"gattr length","",xlen);
		  } else {
		      yy->lengthk = xlen;
		      debug(F101,"gattr lengthk","",xlen);
		  }
	      }
	      /* If the length field is not numeric accept the file */
	      /* anyway but with an unknown length */
	      break;
	  }

#ifdef CK_PERMS
	  case ',':			/* System-dependent protection code */
	    for (i = 0; (i < aln) && (i < CK_PERMLEN); i++)
	      lprmbuf[i] = *s++;	/* Just copy it - decode later */
	    lprmbuf[i] = '\0';		/* Terminate with null */
	    if (i < aln) s += (aln - i);
	    if (atlpri) {
		yy->lprotect.val = (char *)lprmbuf;
		yy->lprotect.len = i;
	    } else
	      lprmbuf[0] = NUL;
	    break;

	  case '-':			/* Generic "world" protection code */
	    gprmbuf[0] = NUL;		/* Just 1 byte by definition */
	    for (i = 0; i < aln; i++)	/* But allow for more... */
	      if (i == 0) gprmbuf[0] = *s++;
	    gprmbuf[1] = NUL;
	    if (atgpri) {
		yy->gprotect.val = (char *)gprmbuf;
		yy->gprotect.len = gprmbuf[0] ? 1 : 0;
	    } else
	      gprmbuf[0] = NUL;
	    break;
#endif /* CK_PERMS */

	  default:			/* Unknown attribute */
	    s += aln;			/* Just skip past it */
	    break;
	}
    }

    /* Check space now, because we also need to know the file type */
    /* in case zchkspa() differentiates text and binary (VMS version does) */

    if (atleni && !calibrate) {		/* Length attribute enabled? */
	if (yy->length > (CK_OFF_T)-1) { /* Length-in-bytes attribute rec'd? */
	    if (!zchkspa(ff,(yy->length))) { /* Check space */
		retcode = -1;		     /* Not enuf */
		*rp++ = '1';
		if (!opnerr) tlog(F100," refused: length bytes","",0);
	    }
	} else if (yy->lengthk > (CK_OFF_T)-1) { /* Length in K received? */
	    long xlen;
	    xlen = yy->lengthk * 1024;
	    if (!zchkspa(ff,xlen)) {
		retcode = -1;		/* Check space */
		*rp++ = '!';
		if (!opnerr) tlog(F100," refused: length K","",0);
	    }
	}
    }
    if (retcode > -1L) {		/* Remember the file size */
	if (yy->length > (CK_OFF_T)-1) {
	    fsize = yy->length;
	} else if (yy->lengthk > (CK_OFF_T)-1 && !overflow) {
	    fsize = yy->lengthk * 1024L;
	} else fsize = yy->length;	/* (e.g. -2L) */
    }

#ifdef DEBUG
    if (deblog) {
#ifdef COMMENT
	sprintf(abuf,"%ld",fsize);	/* safe */
#else
	ckstrncpy(abuf,ckfstoa(fsize),ABUFL);
#endif	/* COMMENT */
debug(F110,"gattr fsize",abuf,0);
    }
#endif /* DEBUG */

    if (retcode == 0) rp = rpbuf;	/* Null reply string if accepted */
    *rp = '\0';				/* End of reply string */

#ifdef CK_RESEND
    if (d == 'R') {			/* Receiving a RESEND? */
	debug(F101,"gattr RESEND","",retcode);
	/* We ignore retcodes because this overrides */
	if (binary != XYFT_B) {		/* Reject if not binary */
	    retcode = -1;		/* in case type field came */
	    ckstrncpy(rpbuf,"N+",RPBUFL); /* after the disposition field */
	    debug(F111,"gattr RESEND not binary",rpbuf,binary);
	} else {			/* Binary mode */
	    retcode = 0;		/* Accept the file */
	    discard = 0;		/* If SET FILE COLLISION DISCARD */
#ifdef COMMENT
	    sprintf(rpbuf+2,"%ld",rs_len); /* Reply with length of file */
#else
	    ckstrncpy(rpbuf+2,ckfstoa(rs_len),RPBUFL-2);
#endif	/* COMMENT */
	    rpbuf[0] = '1';		/* '1' means Length in Bytes */
	    rpbuf[1] = tochar((int)strlen(rpbuf+2)); /* Length of length */
	    debug(F111,"gattr RESEND OK",rpbuf,retcode);
	}
    }
#endif /* CK_RESEND */
    if (retcode == 0 && discard != 0) {	/* Do we still have a discard flag? */
	ckstrncpy(rpbuf,"N?",RPBUFL);	/* Yes, must be filename collision */
	retcode = -1;			/* "?" = name (reply-only code) */
    }
    yy->reply.val = rpbuf;		/* Add it to attribute structure */
    yy->reply.len = (int)strlen(rpbuf);
    if (retcode < 0) {			/* If we are rejecting */
	discard = 1;			/* remember to discard the file */
	rejection = rpbuf[1];		/* and use the first reason given. */
	if (fncsav != -1) {
	    fncact = fncsav;
	    fncsav = -1;
	}
    }
    debug(F111,"gattr return",rpbuf,retcode);
    return(retcode);
}

/*  I N I T A T T R  --  Initialize file attribute structure  */

int
initattr(yy) struct zattr *yy; {
    yy->lengthk = yy->length = (CK_OFF_T)-1;
    yy->type.val = "";
    yy->type.len = 0;
    yy->date.val = "";
    yy->date.len = 0;
    yy->encoding.val = "";
    yy->encoding.len = 0;
    yy->disp.val = "";
    yy->disp.len = 0;
    yy->systemid.val = "";
    yy->systemid.len = 0;
    yy->sysparam.val = "";
    yy->sysparam.len = 0;
    yy->creator.val = "";
    yy->creator.len = 0;
    yy->account.val = "";
    yy->account.len = 0;
    yy->area.val = "";
    yy->area.len = 0;
    yy->password.val = "";
    yy->password.len = 0;
    yy->blksize = -1L;
    yy->xaccess.val = "";
    yy->xaccess.len = 0;
#ifdef CK_PERMS
    if (!ofperms) ofperms = "";
    debug(F110,"initattr ofperms",ofperms,0);
    yy->lprotect.val = ofperms;
    yy->lprotect.len = 0 - strlen(ofperms); /* <-- NOTE! */
    /*
      A negative length indicates that we have a permissions string but it has
      been inherited from a previously existing file rather than picked up
      from an incoming A-packet.
    */
#else
    yy->lprotect.val = "";
    yy->lprotect.len = 0;
#endif /* CK_PERMS */
    yy->gprotect.val = "";
    yy->gprotect.len = 0;
    yy->recfm.val = "";
    yy->recfm.len = 0;
    yy->reply.val = "";
    yy->reply.len = 0;
#ifdef OS2
    yy->longname.len = 0 ;
    yy->longname.val = "" ;
#endif /* OS2 */
    return(0);
}

/*  A D E B U -- Write attribute packet info to debug log  */

int
adebu(f,zz) char *f; struct zattr *zz; {
#ifdef DEBUG
    if (deblog == 0) return(0);
    debug(F110,"Attributes for incoming file ",f,0);
    debug(F101," length in K","",(int) zz->lengthk);
    debug(F111," file type",zz->type.val,zz->type.len);
    debug(F111," creation date",zz->date.val,zz->date.len);
    debug(F111," creator",zz->creator.val,zz->creator.len);
    debug(F111," account",zz->account.val,zz->account.len);
    debug(F111," area",zz->area.val,zz->area.len);
    debug(F111," password",zz->password.val,zz->password.len);
    debug(F101," blksize","",(int) zz->blksize);
    debug(F111," access",zz->xaccess.val,zz->xaccess.len);
    debug(F111," encoding",zz->encoding.val,zz->encoding.len);
    debug(F111," disposition",zz->disp.val,zz->disp.len);
    debug(F111," lprotection",zz->lprotect.val,zz->lprotect.len);
    debug(F111," gprotection",zz->gprotect.val,zz->gprotect.len);
    debug(F111," systemid",zz->systemid.val,zz->systemid.len);
    debug(F111," recfm",zz->recfm.val,zz->recfm.len);
    debug(F111," sysparam",zz->sysparam.val,zz->sysparam.len);
    debug(F101," length","",(int) zz->length);
    debug(F110," reply",zz->reply.val,0);
#endif /* DEBUG */
    return(0);
}

/*  O P E N A -- Open a file, with attributes.  */
/*
  This function tries to open a new file to put the arriving data in.  The
  filename is the one in the srvcmd buffer.  File collision actions are:
  OVERWRITE (the existing file is overwritten), RENAME (the new file is
  renamed), BACKUP (the existing file is renamed), DISCARD (the new file is
  refused), UPDATE (the incoming file replaces the existing file only if the
  incoming file has a newer creation date).

  Returns 0 on failure, nonzero on success.
*/
extern char *rf_err;

int
opena(f,zz) char *f; struct zattr *zz; {
    int x, dispos = 0;
    static struct filinfo fcb;		/* Must be static! */

    debug(F110,"opena f",f,0);
    debug(F101,"opena discard","",discard);

    adebu(f,zz);			/* Write attributes to debug log */

    ffc = (CK_OFF_T)0;			/* Init file-character counter */

#ifdef PIPESEND
    if (pipesend)			/* Receiving to a pipe - easy. */
      return(openo(f,zz,&fcb));		/* Just open the pipe. */
#endif /* PIPESEND */

    /* Receiving to a file - set up file control structure */

    fcb.bs = fblksiz;			/* Blocksize */
#ifndef NOCSETS
    fcb.cs = fcharset;			/* Character set */
#else
    fcb.cs = 0;				/* Character set */
#endif /* NOCSETS */
    fcb.rl = frecl;			/* Record Length */
    fcb.fmt = frecfm;			/* Record Format */
    fcb.org = forg;			/* Organization */
    fcb.cc = fcctrl;			/* Carriage control */
    fcb.typ = binary;			/* Type */
    debug(F101,"opena xflg","",xflg);
    debug(F101,"opena remfile","",remfile);
    debug(F101,"opena remappd","",remappd);
    if (xflg && remfile && remappd)	/* REMOTE output redirected with >> */
      fcb.dsp = XYFZ_A;
    else
      fcb.dsp = (fncact == XYFX_A) ? XYFZ_A : XYFZ_N; /* Disposition */
    debug(F101,"opena disp","",fcb.dsp);
    fcb.os_specific = "";		/* OS-specific info */
#ifdef CK_LABELED
    fcb.lblopts = lf_opts;		/* Labeled file options */
#else
    fcb.lblopts = 0;
#endif /* CK_LABELED */

    if (zz->disp.len > 0) {		/* Incoming file has a disposition? */
	debug(F111,"open disposition",zz->disp.val,zz->disp.len);
	dispos = (int) (*(zz->disp.val));
    }
    if (!dispos && xflg && remfile && remappd) /* REMOTE redirect append ? */
      dispos = fcb.dsp;

    debug(F101,"opena dispos","",dispos);

    if (!dispos) {			         /* No special disposition? */
	if (fncact == XYFX_B && ofn1x && ofn2) { /* File collision = BACKUP? */
	    if (zrename(ofn1,ofn2) < 0) {        /* Rename existing file. */
		debug(F110,"opena rename fails",ofn1,0);
		rf_err = "Can't create backup file";
		return(0);
	    } else debug(F110,"opena rename ok",ofn2,0);
	}
    } else if (dispos == 'R') {		/* Receiving a RESEND */
	debug(F101,"opena remote len","",zz->length);
	debug(F101,"opena local len","",rs_len);
#ifdef COMMENT
        if (fncact == XYFX_R)		/* and file collision = RENAME */
	  if (ofn1x)
#endif /* COMMENT */
	if (ofn1[0])
	  f = ofn1;			/* use original name. */
        if (fncact == XYFX_R)		/* if file collision is RENAME */
          ckstrncpy(filnam,ofn1,CKMAXPATH+1); /* restore the real name */
        xxscreen(SCR_AN,0,0L,f);	/* update name on screen */
	if (zz->length == rs_len)	/* Local and remote lengths equal? */
	  return(-17);			/* Secret code */
    }
    debug(F111,"opena [file]=mode: ",f,fcb.dsp);
    if (x = openo(f,zz,&fcb)) {		/* Try to open the file. */
#ifdef pdp11
	tlog(F110," local name:",f,0L);	/* OK, open, record local name. */
	makestr(&prfspec,f);		/* New preliminary name */
#else
#ifndef ZFNQFP
	tlog(F110," local name:",f,0L);
	makestr(&prfspec,f);
#else
	{				/* Log full local pathname */
	    char *p = NULL, *q = f;
	    if ((p = malloc(CKMAXPATH+1)))
	      if (zfnqfp(filnam, CKMAXPATH, p))
		q = p;
	    tlog(F110," local name:",q,0L);
	    makestr(&prfspec,q);
	    if (p) free(p);
	}
#endif /* ZFNQFP */
#endif /* pdp11 */

	if (binary) {			/* Log file mode in transaction log */
	    tlog(F101," mode: binary","",(long) binary);
	} else {			/* If text mode, check character set */
	    tlog(F100," mode: text","",0L);
#ifndef NOCSETS
	    if (xfrxla) {
		if (fcharset > -1 && fcharset <= MAXFCSETS)
		  tlog(F110," file character-set:",fcsinfo[fcharset].name,0L);
		if (tcharset > -1 && tcharset <= MAXTCSETS)
		  tlog(F110," xfer character-set:",tcsinfo[tcharset].name,0L);
	    } else {
		  tlog(F110," character-set:","transparent",0L);
	    }
#endif /* NOCSETS */
	    debug(F111,"opena charset",zz->encoding.val,zz->encoding.len);
	}
	debug(F101,"opena binary","",binary);

#ifdef COMMENT
	if (fsize >= 0)
#endif /* COMMENT */
	  xxscreen(SCR_FS,0,fsize,"");

#ifdef datageneral
/*
  Need to turn on multi-tasking console interrupt task here, since multiple
  files may be received (huh?) ...
*/
        if ((local) && (!quiet))        /* Only do this if local & not quiet */
	  consta_mt();			/* Start the async read task */
#endif /* datageneral */

    } else {				/* Did not open file OK. */

	rf_err = ck_errstr();		/* Get system error message */
	if (*rf_err)
	  xxscreen(SCR_EM,0,0l,rf_err);
	else
	  xxscreen(SCR_EM,0,0l,"Can't open output file");
        tlog(F110,"Failure to open",f,0L);
        tlog(F110,"Error:",rf_err,0L);
	debug(F110,"opena error",rf_err,0);
    }
    return(x);				/* Pass on return code from openo */
}

/*  O P E N C  --  Open a command (in place of a file) for output */

int
openc(n,s) int n; char * s; {
    int x;
#ifndef NOPUSH
    x = zxcmd(n,s);
#else
    x = 0;
#endif /* NOPUSH */
    debug(F111,"openc zxcmd",s,x);
    o_isopen = (x > 0) ? 1 : 0;
    return(x);
}

/*  C A N N E D  --  Check if current file transfer cancelled */

int
canned(buf) CHAR *buf; {
    extern int interrupted;
    if (*buf == 'X') cxseen = 1;
    if (*buf == 'Z') czseen = 1;
    if (czseen || cxseen)
      interrupted = 1;
    debug(F101,"canned: cxseen","",cxseen);
    debug(F101," czseen","",czseen);
    return((czseen || cxseen) ? 1 : 0);
}


/*  O P E N I  --  Open an existing file for input  */

int
openi(name) char *name; {
#ifndef NOSERVER
    extern int fromgetpath;
#endif /* NOSERVER */
    int x, filno;
    char *name2;
    extern CHAR *epktmsg;

    epktmsg[0] = NUL;			/* Initialize error message */
    if (memstr || sndarray) {		/* Just return if "file" is memory. */
	i_isopen = 1;
	return(1);
    }
    debug(F110,"openi name",name,0);
    debug(F101,"openi sndsrc","",sndsrc);

    filno = (sndsrc == 0) ? ZSTDIO : ZIFILE;    /* ... */
    debug(F101,"openi file number","",filno);

#ifndef NOSERVER
    /* If I'm a server and CWD is disabled and name is not from GET-PATH... */

    if (server && !en_cwd && !fromgetpath) {
	zstrip(name,&name2);
	if (				/* ... check if pathname included. */
#ifdef VMS
	    zchkpath(name)
#else
	    strcmp(name,name2)
#endif /* VMS */
	    ) {
	    tlog(F110,name,"access denied",0L);
	    debug(F110,"openi CD disabled",name,0);
	    ckstrncpy((char *)epktmsg,"Access denied",PKTMSGLEN);
	    return(0);
	} else name = name2;
    }
#endif /* NOSERVER */

#ifdef PIPESEND
    debug(F101,"openi pipesend","",pipesend);
    if (pipesend) {
	int x;
#ifndef NOPUSH
	x = zxcmd(ZIFILE,name);
#else
	x = 0;
#endif /* NOPUSH */
	i_isopen = (x > 0) ? 1 : 0;
	if (!i_isopen)
	  ckstrncpy((char *)epktmsg,"Command or pipe failure",PKTMSGLEN);
	debug(F111,"openi pipesend zxcmd",name,x);
	return(i_isopen);
    }
#endif /* PIPESEND */

#ifdef CALIBRATE
    if (calibrate) {
	i_isopen = 1;
    	return(1);
    }
#endif /* CALIBRATE */

    x = zopeni(filno,name);		/* Otherwise, try to open it. */
    debug(F111,"openi zopeni 1",name,x);
    if (x) {
	i_isopen = 1;
    	return(1);
    } else {				/* If not found, */
	char xname[CKMAXPATH];		/* convert the name */
#ifdef NZLTOR
	nzrtol(name,xname,fncnv,fnrpath,CKMAXPATH);
#else
	zrtol(name,xname);		/* to local form and then */
#endif /*  NZLTOR */
	x = zopeni(filno,xname);	/* try opening it again. */
	debug(F111,"openi zopeni 2",xname,x);
	if (x) {
	    i_isopen = 1;
	    return(1);			/* It worked. */
        } else {
	    char * s;
	    s = ck_errstr();
	    if (s) if (!s) s = NULL;
	    if (!s) s = "Can't open file";
	    ckstrncpy((char *)epktmsg,s,PKTMSGLEN);
	    tlog(F110,xname,s,0L);
	    debug(F110,"openi failed",xname,0);
	    debug(F110,"openi message",s,0);
	    i_isopen = 0;
	    return(0);
        }
    }
}

/*  O P E N O  --  Open a new file for output.  */

int
openo(name,zz,fcb) char *name; struct zattr *zz; struct filinfo *fcb; {
    char *name2;
#ifdef DTILDE
    char *dirp;
#endif /* DTILDE */

    int channel, x;

    if (stdouf) {				/* Receiving to stdout? */
	x = zopeno(ZSTDIO,"",zz,NULL);
	o_isopen = (x > 0);
	debug(F101,"openo stdouf zopeno","",x);
	return(x);
    }
    debug(F110,"openo: name",name,0);

    if (cxseen || czseen || discard) {	/* If interrupted, get out before */
	debug(F100," open cancelled","",0); /* destroying existing file. */
	return(1);			/* Pretend to succeed. */
    }
    channel = ZOFILE;			/* SET DESTINATION DISK or PRINTER */

#ifdef PIPESEND
    debug(F101,"openo pipesend","",pipesend);
    if (pipesend) {
	int x;
#ifndef NOPUSH
	x = zxcmd(ZOFILE,(char *)srvcmd);
#else
	x = 0;
#endif /* NOPUSH */
	o_isopen = x > 0;
	debug(F101,"openo zxcmd","",x);
	return(x);
    }
#endif /* PIPESEND */

    if (dest == DEST_S) {		/* SET DEST SCREEN... */
	channel = ZCTERM;
	fcb = NULL;
    }
#ifdef DTILDE
    if (*name == '~') {
	dirp = tilde_expand(name);
	if (*dirp) ckstrncpy(name,dirp,CKMAXPATH+1);
    }
#endif /* DTILDE */
    if (server && !en_cwd) {		/* If running as server */
	zstrip(name,&name2);		/* and CWD is disabled, */
	if (strcmp(name,name2)) {	/* check if pathname was included. */
	    tlog(F110,name,"authorization failure",0L);
	    debug(F110,"openo CD disabled",name,0);
	    return(0);
	} else name = name2;
    }
    if (zopeno(channel,name,zz,fcb) <= 0) { /* Try to open the file */
	o_isopen = 0;
	debug(F110,"openo failed",name,0);
	/* tlog(F110,"Failure to open",name,0L); */
	return(0);
    } else {
	o_isopen = 1;
	debug(F110,"openo ok, name",name,0);
	return(1);
    }
}

/*  O P E N T  --  Open the terminal for output, in place of a file  */

int
opent(zz) struct zattr *zz; {
    int x;
    ffc = tfc = (CK_OFF_T)0;
    x = zopeno(ZCTERM,"",zz,NULL);
    debug(F101,"opent zopeno","",x);
    if (x >= 0) {
	o_isopen = 1;
	binary = XYFT_T;
    } else
      return(0);
    return(x);
}

/*  O P E N X  --  Open nothing (incoming file to be accepted but ignored)  */

int
ckopenx(zz) struct zattr *zz; {
    ffc = tfc = (CK_OFF_T)0;		/* Reset counters */
    o_isopen = 1;
    debug(F101,"ckopenx fsize","",fsize);
    xxscreen(SCR_FS,0,fsize,"");	/* Let screen display know the size */
    return(1);
}

/*  C L S I F  --  Close the current input file. */

int
clsif() {
    extern int xferstat, success;
    int x = 0;

    fcps();			/* Calculate CPS quickly */

#ifdef datageneral
    if ((local) && (!quiet))    /* Only do this if local & not quiet */
      if (nfils < 1)		/* More files to send ... leave it on! */
	connoi_mt();
#endif /* datageneral */

    debug(F101,"clsif i_isopen","",i_isopen);
    if (i_isopen) {			/* If input file is open... */
	if (memstr) {			/* If input was memory string, */
	    memstr = 0;			/* indicate no more. */
	} else {
	    x = zclose(ZIFILE);		/* else close input file. */
	}
#ifdef DEBUG
	if (deblog) {
	    debug(F101,"clsif zclose","",x);
	    debug(F101,"clsif success","",success);
	    debug(F101,"clsif xferstat","",xferstat);
	    debug(F101,"clsif fsize","",fsize);
	    debug(F101,"clsif ffc","",ffc);
	    debug(F101,"clsif cxseen","",cxseen);
	    debug(F101,"clsif czseen","",czseen);
	    debug(F101,"clsif discard","",czseen);
	}
#endif /* DEBUG */
	if ((cxseen || czseen) && !epktsent) { /* If interrupted */
	    xxscreen(SCR_ST,ST_INT,0l,""); /* say so */
#ifdef TLOG
	    if (tralog && !tlogfmt)
	      doxlog(what,psfspec,fsize,binary,1,"Interrupted");
#endif /* TLOG */
	} else if (discard && !epktsent) { /* If I'm refusing */
	    xxscreen(SCR_ST,ST_REFU,0l,refused); /* say why */
#ifdef TLOG
	    if (tralog && !tlogfmt) {
		char buf[128];
		ckmakmsg(buf,128,"Refused: ",refused,NULL,NULL);
		doxlog(what,psfspec,fsize,binary,1,buf);
	    }
#endif /* TLOG */
	} else if (!epktrcvd && !epktsent && !cxseen && !czseen) {
	    CK_OFF_T zz;
	    zz = ffc;
#ifdef CK_RESEND
	    if (sendmode == SM_RESEND || sendmode == SM_PSEND)
	      zz += sendstart;
#endif /* CK_RESEND */
	    debug(F101,"clsif fstats","",zz);
	    fstats();			/* Update statistics */
	    if (			/* Was the whole file sent? */
#ifdef VMS
		0			/* Not a reliable check in VMS */
#else
#ifdef STRATUS
		0			/* Probably not for VOS either */
#else
		zz < fsize
#ifdef CK_CTRLZ
		&& ((eofmethod != XYEOF_Z && !binary) || binary)
#endif /* CK_CTRLZ */
#endif /* STRATUS */
#endif /* VMS */
		) {
		xxscreen(SCR_ST,ST_INT,0l,"");
#ifdef TLOG
		if (tralog && !tlogfmt)
		  doxlog(what,psfspec,fsize,binary,1,"Incomplete");
#endif /* TLOG */
	    } else {
#ifdef COMMENT
		/* Not yet -- we don't have confirmation from the receiver */
		xxscreen(SCR_ST,ST_OK,0l,"");
#endif /* COMMENT */
#ifdef TLOG
		if (tralog && !tlogfmt)
		  doxlog(what,psfspec,fsize,binary,0,"");
#endif /* TLOG */
	    }
	}
    }
    i_isopen = 0;
    hcflg = 0;				/* Reset flags */
    sendstart = (CK_OFF_T)0;		/* Don't do this again! */
#ifdef COMMENT
/*
  This prevents a subsequent call to clsof() from deleting the file
  when given the discard flag.
*/
    *filnam = '\0';			/* and current file name */
#endif /* COMMENT */
    return(x);
}


/*  C L S O F  --  Close an output file.  */

/*  Call with disp != 0 if file is to be discarded.  */
/*  Returns -1 upon failure to close, 0 or greater on success. */

int
clsof(disp) int disp; {
    int x = 0;
    extern int success;

    fcps();				/* Calculate CPS quickly */

    debug(F101,"clsof disp","",disp);
    debug(F101,"clsof cxseen","",cxseen);
    debug(F101,"clsof success","",success);

    debug(F101,"clsof o_isopen","",o_isopen);
    if (fncsav != -1) {			/* Saved file collision action... */
	fncact = fncsav;		/* Restore it. */
	fncsav = -1;			/* Unsave it. */
    }
#ifdef datageneral
    if ((local) && (!quiet))		/* Only do this if local & not quiet */
      connoi_mt();
#endif /* datageneral */
    if (o_isopen && !calibrate) {
	if ((x = zclose(ZOFILE)) < 0) { /* Try to close the file */
	    tlog(F100,"Failure to close",filnam,0L);
	    xxscreen(SCR_ST,ST_ERR,0l,"Can't close file");
#ifdef TLOG
	    if (tralog && !tlogfmt)
	      doxlog(what,prfspec,fsize,binary,1,"Can't close file");
#endif /* TLOG */
	} else if (disp) {		/* Interrupted or refused */
	    if (keep == 0 ||		/* If not keeping incomplete files */
		(keep == SET_AUTO && binary == XYFT_T)
		) {
		if (*filnam && (what & W_RECV)) /* AND we're receiving */
		  zdelet(filnam);	/* ONLY THEN, delete it */
		if (what & W_KERMIT) {
		    debug(F100,"clsof incomplete discarded","",0);
		    tlog(F100," incomplete: discarded","",0L);
		    if (!epktrcvd && !epktsent) {
			xxscreen(SCR_ST,ST_DISC,0l,"");
#ifdef TLOG
			if (tralog && !tlogfmt)
			  doxlog(what,prfspec,fsize,binary,1,"Discarded");
#endif /* TLOG */
		    }
		}
	    } else {			/* Keep incomplete copy */
		debug(F100,"clsof fstats 1","",0);
		fstats();
		if (!discard) {	 /* Unless discarding for other reason... */
		    if (what & W_KERMIT) {
			debug(F100,"closf incomplete kept","",0);
			tlog(F100," incomplete: kept","",0L);
		    }
		}
		if (what & W_KERMIT) {
		    if (!epktrcvd && !epktsent) {
			xxscreen(SCR_ST,ST_INC,0l,"");
#ifdef TLOG
			if (tralog && !tlogfmt)
			  doxlog(what,prfspec,fsize,binary,1,"Incomplete");
#endif /* TLOG */
		    }
		}
	    }
	}
    }
    if (o_isopen && x > -1 && !disp) {
	debug(F110,"clsof OK",rfspec,0);
	makestr(&rfspec,prfspec);
	makestr(&rrfspec,prrfspec);
	fstats();
	if (!epktrcvd && !epktsent && !cxseen && !czseen) {
	    xxscreen(SCR_ST,ST_OK,0L,"");
#ifdef TLOG
	    if (tralog && !tlogfmt)
	      doxlog(what,rfspec,fsize,binary,0,"");
#endif /* TLOG */
	}
    }
    rs_len = (CK_OFF_T)0;
    o_isopen = 0;			/* The file is not open any more. */
    cxseen = 0;				/* Reset per-file interruption flag */
    return(x);				/* Send back zclose() return code. */
}

#ifdef SUNOS4S5
tolower(c) char c; { return((c)-'A'+'a'); }
toupper(c) char c; { return((c)-'a'+'A'); }
#endif /* SUNOS4S5 */
#endif /* NOXFER */

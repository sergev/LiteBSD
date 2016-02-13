/*  C K U C M D . H  --  Header file for Unix cmd package  */

/*
  Author: Frank da Cruz <fdc@columbia.edu>
  Columbia University Kermit Project, New York City.

  Copyright (C) 1985, 2009,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/

#ifndef CKUCMD_H
#define CKUCMD_H

/* Command recall */

#ifdef pdp11				/* Not enough room for this */
#ifndef NORECALL
#define NORECALL
#endif /* NORECALL */
#endif /* pdp11 */

#ifdef DYNAMIC				/* Dynamic command buffers */
/*
  Use malloc() to allocate the many command-related buffers in ckucmd.c.
*/
#ifndef DCMDBUF
#ifndef NORECALL
#define NORECALL
#endif /* NORECALL */
#endif /* DCMDBUF */

#ifndef NORECALL
#define CK_RECALL
#else
#ifdef CK_RECALL
#undef CK_RECALL
#endif /* CK_RECALL */
#endif /* NORECALL */
#else
#ifndef NORECALL
#define NORECALL
#endif /*  NORECALL */
#endif /* DYNAMIC */

#ifdef NORECALL
#ifdef CK_RECALL
#undef CK_RECALL
#endif /* CK_RECALL */
#endif /* NORECALL */

#ifdef NORECALL
#ifndef NOARROWKEYS
#define NOARROWKEYS
#endif /* NOARROWKEYS */
#endif /* NORECALL */

/* Special getchars */

#ifdef VMS
#ifdef getchar				/* This is for VMS GCC */
#undef getchar
#endif /* getchar */
#define getchar()   vms_getchar()
int vms_getchar(void);
#endif /* VMS */

#ifdef aegis
#undef getchar
#define getchar()   coninc(0)
#endif /* aegis */

#ifdef AMIGA
#undef getchar
#define getchar() coninc(0)
#endif /* AMIGA */

#ifdef Plan9
#undef getchar
#define getchar() coninc(0)
#undef putchar
#define putchar(c) conoc(c)
#undef printf
#define printf conprint
#endif /* Plan9 */

/* Sizes of things */

#ifndef CMDDEP
#ifdef BIGBUFOK
#define CMDDEP  64			/* Maximum command recursion depth */
#else
#define CMDDEP  20
#endif /* BIGBUFOK */
#endif /* CMDDEP */
#define HLPLW   78			/* Width of ?-help line */
#define HLPCW   19			/* Width of ?-help column */
#define HLPBL  100			/* Help string buffer length */
#ifdef BIGBUFOK
#define ATMBL 10238			/* Command atom buffer length*/
#else
#ifdef NOSPL
#define ATMBL  256
#else
#define ATMBL  1024
#endif /* NOSPL */
#endif /* BIGBUFOK */

#ifndef CMDBL
#ifdef NOSPL
/* No script programming language, save some space */
#define CMDBL 608			/* Command buffer length */
#else
#ifdef BIGBUFOK
#define CMDBL 32763
#else
#define CMDBL 4092
#endif /* OS2 */
#endif /* NOSPL */
#endif /* CMDBL */

/* Special characters */

#define RDIS 0022			/* Redisplay   (^R) */
#define LDEL 0025			/* Delete line (^U) */
#define WDEL 0027			/* Delete word (^W) */
#ifdef CK_RECALL
#define C_UP 0020			/* Go Up in recall buffer (^P) */
#define C_UP2 0002			/* Alternate Go Up (^B) for VMS */
#define C_DN 0016			/* Go Down in recall buffer (^N) */
#endif /* CK_RECALL */

/* Keyword flags (bits, powers of 2) */

#define CM_INV 1			/* Invisible keyword */
#define CM_ABR 2			/* Abbreviation for another keyword */
#define CM_HLP 4			/* Help-only keyword */
#define CM_ARG 8			/* An argument is required */
#define CM_NOR 16			/* No recall for this command */
#define CM_PRE 32			/* Long-form cmdline arg for prescan */
#define CM_PSH 64			/* Command disabled if nopush */
#define CM_LOC 128			/* Command disabled if nolocal */

/*
  A long-form command line option is a keyword using the regular struct keytab
  and lookup mechanisms.  Flags that make sense in this context are CM_ARG,
  indicating this option requires an argument (operand), and CM_PRE, which
  means this option must be processed before the initialization file.  The
  absence of CM_PRE means the option is to be processed after the
  initialization file in the normal manner.
*/

/* Token flags (numbers) */

#define CMT_COM 0			/* Comment (; or #) */
#define CMT_SHE 1			/* Shell escape (!) */
#define CMT_LBL 2			/* Label (:) */
#define CMT_FIL 3			/* Indirect filespec (@) (not used) */

/* Path separator for path searches */

#ifdef OS2
#define PATHSEP ';'
#else
#ifdef UNIX
#define PATHSEP ':'
#else
#define PATHSEP ','
#endif /* UNIX */
#endif /* OS2 */

#ifndef CK_KEYTAB
#define CK_KEYTAB

/* Keyword Table Template perhaps already defined in ckcdeb.h */

struct keytab {				/* Keyword table */
    char *kwd;				/* Pointer to keyword string */
    int kwval;				/* Associated value */
    int flgs;				/* Flags (as defined above) */
};
#endif /* CK_KEYTAB */

/* String preprocessing function */

#ifdef CK_ANSIC				/* ANSI C */
#ifdef M_SYSV				/* SCO Microsoft C wants no args */
typedef int (*xx_strp)();
#else
typedef int (*xx_strp)(char *, char **, int *);
#endif /* M_SYSV */
#else					/* Not ANSI C */
typedef int (*xx_strp)();
#endif /* CK_ANSIC */

/* FLDDB struct */

typedef struct FDB {
    int fcode;				/* Function code */
    char * hlpmsg;			/* Help message */
    char * dflt;			/* Default */
    char * sdata;			/* Additional string data */
    int ndata1;				/* Additional numeric data 1 */
    int ndata2;				/* Additional numeric data 2 */
    xx_strp spf;			/* String processing function */
    struct keytab * kwdtbl;		/* Keyword table */
    struct FDB * nxtfdb;		/* Pointer to next alternative */
} fdb;

typedef struct OFDB {
    struct FDB * fdbaddr;		/* Address of succeeding FDB struct */
    int fcode;				/* Function code */
    char * sresult;			/* String result */
    int nresult;			/* Integer result */
    int kflags;				/* Keyword flags if any */
    CK_OFF_T wresult;			/* Long integer ("wide") result */
} ofdb;

#ifndef CKUCMD_C
extern struct OFDB cmresult;
#endif /* CKUCMD_C */

/* Codes for primary parsing function  */

#define _CMNUM 0			/* Number */
#define _CMOFI 1			/* Output file */
#define _CMIFI 2			/* Input file */
#define _CMFLD 3			/* Arbitrary field */
#define _CMTXT 4			/* Text string */
#define _CMKEY 5			/* Keyword */
#define _CMCFM 6			/* Confirmation */
#define _CMDAT 7			/* Date/time */
#define _CMNUW 8			/* Wide version of cmnum */

/* Function prototypes */

_PROTOTYP( int xxesc, (char **) );
_PROTOTYP( int cmrini, (int) );
_PROTOTYP( VOID cmsetp, (char *) );
_PROTOTYP( VOID cmsavp, (char [], int) );
_PROTOTYP( char * cmgetp, (void) );
_PROTOTYP( VOID prompt, (xx_strp) );
_PROTOTYP( VOID pushcmd, (char *) );
_PROTOTYP( VOID cmres, (void) );
_PROTOTYP( VOID cmini, (int) );
_PROTOTYP( int cmgbrk, (void) );
_PROTOTYP( int cmgkwflgs, (void) );
_PROTOTYP( int cmpush, (void) );
_PROTOTYP( int cmpop, (void) );
_PROTOTYP( VOID untab, (char *) );
_PROTOTYP( int cmnum, (char *, char *, int, int *, xx_strp ) );
_PROTOTYP( int cmnumw, (char *, char *, int, CK_OFF_T *, xx_strp ) );
_PROTOTYP( int cmofi, (char *, char *, char **, xx_strp ) );
_PROTOTYP( int cmifi, (char *, char *, char **, int *, xx_strp ) );
_PROTOTYP( int cmiofi, (char *, char *, char **, int *, xx_strp ) );
_PROTOTYP( int cmifip,(char *, char *, char **, int *, int, char *, xx_strp ));
_PROTOTYP( int cmifi2,(char *,char *,char **,int *,int,char *,xx_strp,int ));
_PROTOTYP( int cmdir, (char *, char *, char **, xx_strp ) );
_PROTOTYP( int cmdirp, (char *, char *, char **, char *, xx_strp ) );
_PROTOTYP( int cmfld, (char *, char *, char **, xx_strp ) );
_PROTOTYP( int cmtxt, (char *, char *, char **, xx_strp ) );
_PROTOTYP( int cmkey,  (struct keytab [], int, char *, char *, xx_strp) );
_PROTOTYP( int cmkeyx, (struct keytab [], int, char *, char *, xx_strp) );
_PROTOTYP( int cmkey2,(struct keytab [],int,char *,char *,char *,xx_strp,int));
_PROTOTYP( int cmswi,  (struct keytab [], int, char *, char *, xx_strp) );
_PROTOTYP( int cmdate,(char *, char *, char **, int, xx_strp) );
_PROTOTYP( char * cmpeek, (void) );
_PROTOTYP( int cmfdb, (struct FDB *) );
_PROTOTYP( VOID cmfdbi, (struct FDB *,
			int, char *, char *, char *, int, int, xx_strp,
			struct keytab *, struct FDB *) );
_PROTOTYP( int chktok, (char *) );
_PROTOTYP( int cmcfm, (void) );
_PROTOTYP( int lookup, (struct keytab [], char *, int, int *) );
_PROTOTYP( VOID kwdhelp, (struct keytab[],int,char *,char *,char *,int,int) );
_PROTOTYP( int ungword, (void) );
_PROTOTYP( VOID unungw, (void) );
_PROTOTYP( int cmdsquo, (int) );
_PROTOTYP( int cmdgquo, (void) );
_PROTOTYP( char * ckcvtdate, (char *, int) );
_PROTOTYP( int cmdgetc, (int));
#ifndef NOARROWKEYS
_PROTOTYP( int cmdconchk, (void) );
#endif /* NOARROWKEYS */

#ifdef CK_RECALL
_PROTOTYP( char * cmgetcmd, (char *) );
_PROTOTYP( VOID addcmd, (char *) );
_PROTOTYP( VOID cmaddnext, (void) );
#endif /* CK_RECALL */
_PROTOTYP( char * cmcvtdate, (char *, int) );
_PROTOTYP( char * cmdiffdate, (char *, char *) );
_PROTOTYP( char * cmdelta, (int,
			    int,int,int,int,int,int,int,int,int,int,int,int ));
_PROTOTYP( char * shuffledate, (char *, int) );
_PROTOTYP( int filhelp, (int, char *, char *, int, int) );
_PROTOTYP( int xfilhelp, (int, char *, char *, int, int,
			  int,
			  char *, char *, char *, char *,
			  CK_OFF_T, CK_OFF_T,
			  int, int,
			  char **) );
_PROTOTYP( int delta2sec, (char *, long *) );

#ifdef DCMDBUF
_PROTOTYP( int cmsetup, (void) );
#endif /* DCMDBUF */

#endif /* CKUCMD_H */

/* End of ckucmd.h */

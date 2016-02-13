#include "ckcsym.h"

char *cmdv = "Command package 9.0.168, 12 March 2010";

/*  C K U C M D  --  Interactive command package for Unix  */

/* (In reality, it's for all platforms, not just Unix) */

/*
  Author: Frank da Cruz (fdc@columbia.edu),
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 2010,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/

#define TOKPRECHECK

#define DOCHKVAR

/* Command-terminal-to-C-Kermit character mask */

#ifdef OS2				/* K95 */
int cmdmsk = 255;			/* (always was 255) */
#else					/* All others... */
int cmdmsk = 255;			/* 31 Dec 2000 (was 127) */
#endif /* OS2 */

#ifdef BS_DIRSEP			/* Directory separator is backslash */
#undef BS_DIRSEP
#endif /* BS_DIRSEP */

#ifdef OS2
#define BS_DIRSEP
#endif /* BS_DIRSEP */

#define CKUCMD_C

#include "ckcdeb.h"                     /* Formats for debug(), etc. */
#include "ckcker.h"			/* Needed for BIGBUFOK definition */
#include "ckcnet.h"			/* Needed for server-side Telnet */
#include "ckucmd.h"			/* Needed for everything */
#include "ckuusr.h"                     /* Needed for prompt length */

#ifndef NOARROWKEYS
#ifndef NOESCSEQ
#ifdef VMSORUNIX
#define USE_ARROWKEYS			/* Use arrow keys for command recall */
#endif /* VMSORUNIX */
#endif /* NOESCSEQ */
#endif /* NOARROWKEYS */

#undef CKUCMD_C

_PROTOTYP( int unhex, (char) );
_PROTOTYP( static VOID cmdclrscn, (void) );

#ifdef CKLEARN
_PROTOTYP( VOID learncmd, (char *) );
#endif /* CKLEARN */

static char *moname[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

struct keytab cmonths[] = {
  { "april",     4, 0 },
  { "august",    8, 0 },
  { "december", 12, 0 },
  { "february",  2, 0 },
  { "january",   1, 0 },
  { "july",      7, 0 },
  { "june",      6, 0 },
  { "march",     3, 0 },
  { "may",       5, 0 },
  { "november", 11, 0 },
  { "october",  10, 0 },
  { "september", 9, 0 }
};

#ifndef NOICP     /* The rest only if interactive command parsing selected */

#ifndef NOSPL
_PROTOTYP( int chkvar, (char *) );
extern int askflag, echostars;
#endif /* NOSPL */

#ifdef CKROOT
extern int ckrooterr;
#endif /* CKROOT */

#ifdef IKSD
extern int inserver;
#endif /* IKSD */

int cmfldflgs = 0;			/* Flags for cmfld() */
int cmkwflgs = 0;			/* Flags from last keyword parse */
static int nomsg = 0;
static int blocklvl = 0;		/* Block nesting level */
static int linebegin = 0;		/* Flag for at start of a line */
static int quoting = 1;			/* Quoting is allowed */
static int swarg = 0;			/* Parsing a switch argument */
static int xcmfdb = 0;			/* Flag for parsing chained fdbs... */
static int chsrc = 0;			/* Source of character, 1 = tty */
static int newcmd = 0;			/* See addcmd() */

#ifdef BS_DIRSEP
static int dirnamflg = 0;
#endif /* BS_DIRSEP */

/*
Modeled after the DECSYSTEM-20 command parser (the COMND JSYS), RIP. Features:

 . parses and verifies keywords, filenames, text strings, numbers, other data
 . displays appropriate menu or help message when user types "?"
 . does keyword and filename completion when user types ESC or TAB
 . does partial keyword and filename completion
 . accepts any unique abbreviation for a keyword
 . allows keywords to have attributes, like "invisible" and "abbreviation"
 . can supply defaults for fields omitted by user
 . provides command retry and recall
 . provides character, word, and line deletion (but only from the end)
 . accepts input from keyboard, command files, macros, or redirected stdin
 . allows for full or half duplex operation, character or line input
 . allows \-escapes for special characters
 . allows specification of a user exit to expand variables, etc.
 . settable prompt, protected from deletion, dynamically re-evaluated each time
 . allows chained parse functions.

Functions:
 cmsetp - Set prompt (cmprom is prompt string)
 cmsavp - Save current prompt
 cmgetp = Get current prompt
 prompt - Issue prompt
 cmini  - Clear the command buffer (before parsing a new command)
 cmres  - Reset command buffer pointers (before reparsing)
 cmkey  - Parse a keyword or token (also cmkey2)
 cmswi  - Parse a switch
 cmnum  - Parse a number
 cmifi  - Parse an input file name
 cmofi  - Parse an output file name (also cmifip, cmifi2, ...)
 cmdir  - Parse a directory name (also cmdirp)
 cmfld  - Parse an arbitrary field
 cmtxt  - Parse a text string
 cmdate - Parse a date-time string
 cmcfm  - Parse command confirmation (end of line)
 cmfdb  - Parse any of a list of the foregoing (chained parse functions)

Return codes:
 -9: like -2 except this module already printed the error message
 -3: no input provided when required
 -2: input was invalid (e.g. not a number when a number was required)
 -1: reparse required (user deleted into a preceding field)
  0 or greater: success
See individual functions for greater detail.

Before using these routines, the caller should #include "ckucmd.h" and set the
program's prompt by calling cmsetp().  If the file parsing functions cmifi,
cmofi, or cmdir are to be used, this module must be linked with a ck?fio file
system support module for the appropriate system, e.g. ckufio for Unix.  If
the caller puts the terminal in character wakeup ("cbreak") mode with no echo,
then these functions will provide line editing -- character, word, and line
deletion, as well as keyword and filename completion upon ESC and help
strings, keyword, or file menus upon '?'.  If the caller puts the terminal
into character wakeup/noecho mode, care should be taken to restore it before
exit from or interruption of the program.  If the character wakeup mode is not
set, the system's own line editor may be used.

NOTE: Contrary to expectations, many #ifdef's have been added to this module.
Any operation requiring an #ifdef (like clear screen, get character from
keyboard, erase character from screen, etc) should eventually be turned into a
call to a function that is defined in ck?tio.c, but then all the ck?tio.c
modules would have to be changed...
*/

/* Includes */

#include "ckcker.h"
#include "ckcasc.h"			/* ASCII character symbols */
#include "ckucmd.h"                     /* Command parsing definitions */

#ifdef OSF13
#ifdef CK_ANSIC
#ifdef _NO_PROTO
#undef _NO_PROTO
#endif /* _NO_PROTO */
#endif /* CK_ANSIC */
#endif /* OSF13 */

#ifndef HPUXPRE65
#include <errno.h>			/* Error number symbols */
#else
#ifndef ERRNO_INCLUDED
#include <errno.h>			/* Error number symbols */
#endif	/* ERRNO_INCLUDED */
#endif	/* HPUXPRE65 */

#ifdef OS2
#ifndef NT
#define INCL_NOPM
#define INCL_VIO			/* Needed for ckocon.h */
#include <os2.h>
#undef COMMENT
#else
#define APIRET ULONG
#include <windows.h>
#endif /* NT */
#include "ckocon.h"
#include <io.h>
#endif /* OS2 */

#ifdef OSK
#define cc ccount			/* OS-9/68K compiler bug */
#endif /* OSK */

#ifdef GEMDOS				/* Atari ST */
#ifdef putchar
#undef putchar
#endif /* putchar */
#define putchar(x) conoc(x)
#endif /* GEMDOS */

#ifdef CK_AUTODL
extern int cmdadl, justone;
#endif /* CK_AUTODL */

extern int timelimit, nzxopts, nopush, nolocal, xcmdsrc, keepallchars;

#ifdef CKSYSLOG
#ifdef UNIX
#ifdef CKXPRINTF			/* Our printf macro conflicts with */
#undef printf				/* use of "printf" in syslog.h */
#endif /* CKXPRINTF */
#ifdef RTAIX
#include <sys/syslog.h>
#else  /* RTAIX */
#include <syslog.h>
#endif /* RTAIX */
#ifdef CKXPRINTF
#define printf ckxprintf
#endif /* CKXPRINTF */
#endif /* UNIX */
#endif /* CKSYSLOG */

/* Local variables */

static
int psetf = 0,                          /* Flag that prompt has been set */
    cc = 0,                             /* Character count */
    dpx = 0,                            /* Duplex (0 = full) */
    inword = 0;				/* In the middle of getting a word */

char *dfprom = "Command? ";             /* Default prompt */
#ifndef NOLASTFILE
char *lastfile = NULL;			/* Last filespec */
static char *tmplastfile = NULL;	/* Last filespec candidate */
#endif	/* NOLASTFILE */

int cmflgs;                             /* Command flags */
int cmfsav;				/* A saved version of them */

static char pushc = NUL;
static char brkchar = NUL;

#define CMDEFAULT 1023
static char cmdefault[CMDEFAULT+1];

#ifdef DCMDBUF
char *cmdbuf = NULL;			/* Command buffer */
char *savbuf = NULL;			/* Buffer to save copy of command */
char *atmbuf = NULL;			/* Atom buffer - for current field */
char *atxbuf = NULL;			/* For expanding the atom buffer */
char *prevcmd = NULL;
static char *atybuf = NULL;		/* For copying atom buffer */
static char *filbuf = NULL;		/* File name buffer */
static char *cmprom = NULL;		/* Program's prompt */
static char *cmprxx = NULL;		/* Program's prompt, unevaluated */

#ifdef CK_RECALL
/*
  Command recall is available only if we can make profligate use of malloc().
*/
#define R_MAX 10			/* How many commands to save */
int cm_recall = R_MAX;			/* Size of command recall buffer */
int on_recall = 1;			/* Recall feature is ON */
static int no_recall = 0;		/* Recall OFF for this cmd only */
static int force_add = 0;		/* Force cmd into recall buffer */
static int last_recall = -1;		/* Last recall-related action */
/*
  -1 = none
   0 = CR (a command was entered)
   1 = Up
   2 = Down
*/
int in_recall = 0;			/* Recall buffers are init'd */
static int
  current = -1,				/* Pointer to current command */
  rlast = -1;				/* Index of last command in buffer */
static char **recall = NULL;		/* Array of recall buffer pointers */
#endif /* CK_RECALL */
#else  /* !DCMDBUF */
char cmdbuf[CMDBL+4];                   /* Command buffer */
char savbuf[CMDBL+4];                   /* Buffer to save copy of command */
char atmbuf[ATMBL+4];                   /* Atom buffer */
char atxbuf[CMDBL+4];                   /* For expanding the atom buffer */
char prevcmd[CMDBL+4];			/* For displaying the last command */
static char atybuf[ATMBL+4];		/* For copying atom buffer */
static char filbuf[ATMBL+4];		/* File name buffer */
static char cmprom[PROMPTL+1];		/* Program's prompt */
static char cmprxx[PROMPTL+1];		/* Program's prompt, unevaluated */
#endif /* DCMDBUF */

/* Command buffer pointers */

#define PPVLEN VNAML			/* 20080305 Wolfram Sang (was 24) */
char ppvnambuf[PPVLEN+1] = { NUL, NUL };

char * cmbptr = NULL;			/* Current position (for export) */

static char *bp,                        /* Current command buffer position */
    *pp,                                /* Start of current field */
    *np;                                /* Start of next field */

static int ungw,			/* For ungetting words */
    atxn;				/* Expansion buffer (atxbuf) length */

#ifdef OS2
extern int wideresult;
#endif /* OS2 */

extern int cmd_cols, cmd_rows, local, quiet;

#ifdef TNCODE
#ifdef IAC
#undef IAC
#endif /* IAC */
#define IAC 255
#endif /* TNCODE */

_PROTOTYP( static int gtword, (int) );
_PROTOTYP( static int addbuf, (char *) );
_PROTOTYP( static int setatm, (char *, int) );
_PROTOTYP( static VOID cmdnewl, (char) );
_PROTOTYP( static VOID cmdchardel, (void) );
_PROTOTYP( static VOID cmdecho, (char, int) );
_PROTOTYP( static int test, (int, int) );
#ifdef GEMDOS
_PROTOTYP( extern char *strchr, (char *, int) );
#endif /* GEMDOS */

extern char * dftty;

/* The following are for use with chained FDB's */

static int crflag = 0;			/* Carriage return was typed */
static int qmflag = 0;			/* Question mark was typed */
static int esflag = 0;			/* Escape was typed */

/* Directory separator */

#ifdef GEMDOS
static char dirsep = '\\';
#else
#ifdef datageneral
static char dirsep = ':';
#else
#ifdef MAC
static char dirsep = ':';
#else
#ifdef VMS
static char dirsep = '.';
#else
#ifdef STRATUS
static char dirsep = '>';
#else
static char dirsep = '/';		/* UNIX, OS/2, OS-9, Amiga, etc. */
#endif /* STRATUS */
#endif /* VMS */
#endif /* MAC */
#endif /* datageneral */
#endif /* GEMDOS */

/*  H A S N O P A T H  */

/*  Returns 0 if filespec s includes any path segments; 1 if it doesn't. */

int
hasnopath(s) char * s; {
    char * p = NULL;
    if (!s) return(0);
    if (!*s) return(0);
    zstrip(s,&p);
    return(ckstrcmp(s,p,CKMAXPATH,filecase) == 0 ? 1 : 0);
}

/*  C K S P R E A D  --  Print string double-spaced  */

static char * sprptr = NULL;

static char *
ckspread(s) char * s; {
    int n = 0;
    char * p;
    n = strlen(s);
    if (sprptr)
      free(sprptr);
    sprptr = malloc(n + n + 3);
    if (sprptr) {
	p = sprptr;
	while (*s) {
	    *p++ = *s++;
	    *p++ = SP;
	}
	*p = NUL;
    }
    return(sprptr ? sprptr : "");
}

/*  T E S T  --  Bit test  */

static int
test(x,m) int x, m; { /*  Returns 1 if any bits from m are on in x, else 0  */
    return((x & m) ? 1 : 0);
}

/*  K W D H E L P  --  Given a keyword table, print keywords in columns.  */
/*
  Call with:
    s     - keyword table
    n     - number of entries
    pat   - pattern (left substring) that must match for each keyword
    pre   - prefix to add to each keyword
    post  - suffix to add to each keyword
    off   - offset on first screenful, allowing room for introductory text
    xhlp  - 1 to print any CM_INV keywords that are not also abbreviations.
            2 to print CM_INV keywords if CM_HLP also set
            4 if it's a switch table (to show ':' if CM_ARG)

  Arranges keywords in columns with width based on longest keyword.
  Does "more?" prompting at end of screen.
  Uses global cmd_rows and cmd_cols for screen size.
*/
VOID
kwdhelp(s,n,pat,pre,post,off,xhlp)
    struct keytab s[]; int n, off, xhlp; char *pat, *pre, *post;
/* kwdhelp */ {

    int width = 0;
    int cc;
    int cols, height, i, j, k, lc, n2 = 0;
    char *b = NULL, *p, *q;
    char *pa, *px;
    char **s2 = NULL;
    char *tmpbuf = NULL;

    cc = strlen(pat);

    if (!s) return;			/* Nothing to do */
    if (n < 1) return;			/* Ditto */
    if (off < 0) off = 0;		/* Offset for first page */
    if (!pre) pre = "";			/* Handle null string pointers */
    if (!post) post = "";
    lc = off;				/* Screen-line counter */

    if (xhlp & 4)			/* For switches */
      tmpbuf = (char *)malloc(TMPBUFSIZ+1);

    if ((s2 = (char **) malloc(n * sizeof(char *)))) {
	for (i = 0; i < n; i++) {	/* Find longest keyword */
	    s2[i] = NULL;
	    if (ckstrcmp(s[i].kwd,pat,cc,0))
	      continue;

	    if (s[i].flgs & CM_PSH	/* NOPUSH or nopush screening */
#ifndef NOPUSH
		&& nopush
#endif /* NOPUSH */
		)
	      continue;
	    if (s[i].flgs & CM_LOC	/* NOLOCAL or nolocal screening */
#ifndef NOLOCAL
		&& nolocal
#endif /* NOLOCAL */
		)
	      continue;

	    if (s[i].flgs & CM_INV) {
#ifdef COMMENT
/* This code does not show invisible keywords at all except for "help ?" */
/* and then only help topics (CM_HLP) in the top-level keyword list. */

		if ((xhlp & 2) == 0)
		  continue;
		else if ((s[i].flgs & CM_HLP) == 0)
		  continue;
#else
/* This code shows invisible keywords that are not also abbreviations when */
/* ? was typed AFTER the beginning of the field so the user can find out */
/* what they are and (for example) why completion doesn't work at this point */

		if (s[i].flgs & CM_ABR)
		  continue;
		else if ((xhlp & 3) == 0)
		  continue;
		else if ((xhlp & 2) && ((s[i].flgs & CM_HLP) == 0))
		  continue;
#endif /* COMMENT */
	    }
	    j = strlen(s[i].kwd);
	    if (!(xhlp & 4) || !tmpbuf) { /* Regular keyword table */
		s2[n2++] = s[i].kwd;	/* Copy pointers to visible ones */
	    } else {			/* Switches */
		ckmakmsg(tmpbuf,	/* Make a copy that shows ":" if */
			 TMPBUFSIZ,	/* the switch takes an argument. */
			 s[i].kwd,
			 (s[i].flgs & CM_ARG) ? ":" : "",
			 NULL,
			 NULL
			 );
		makestr(&(s2[n2]),tmpbuf);
		if (s[i].flgs & CM_ARG) j++;
		n2++;
	    }
	    if (j > width)
	      width = j;
	}
	/* Column width */
	n = n2;
    }
    if (s2 && (b = (char *) malloc(cmd_cols + 1))) { /* Make a line buffer   */
	char * bx;
	bx = b + cmd_cols;
	width += (int)strlen(pre) + (int)strlen(post) + 2;
	cols = cmd_cols / width;	/* How many columns? */
	if (cols < 1) cols = 1;
	height = n / cols;		/* How long is each column? */
	if (n % cols) height++;		/* Add one for remainder, if any */

	for (i = 0; i < height; i++) {	    /* Loop for each row */
	    for (j = 0; j < cmd_cols; j++)  /* First fill row with blanks */
	      b[j] = SP;
	    for (j = 0; j < cols; j++) {    /* Loop for each column in row */
		k = i + (j * height);       /* Index of next keyword */
		if (k < n) {		    /* In range? */
		    pa = pre;
		    px = post;
		    p = s2[k];		    /* Point to verb name */
		    q = b + (j * width) + 1; /* Where to copy it to */
		    while ((q < bx) && (*q++ = *pa++)) ; /* Copy prefix */
		    q--;		                 /* Back up over NUL */
		    while ((q < bx) && (*q++ = *p++)) ;	 /* Copy filename */
		    q--;		                 /* Back up over NUL */
		    while ((q < bx) && (*q++ = *px++)) ; /* Copy suffix */
		    if (j < cols - 1) {
			q--;
			*q = SP;	/* Replace the space */
		    }
		}
	    }
	    p = b + cmd_cols - 1;	/* Last char in line */
	    while (*p-- == SP) ;	/* Trim */
	    *(p+2) = NUL;
	    printf("%s\n",b);		/* Print the line */
	    if (++lc > (cmd_rows - 2)) { /* Screen full? */
		if (!askmore())		/* Do more-prompting... */
		  goto xkwdhelp;
		else
		  lc = 0;
	    }
	}
	/* printf("\n"); */		/* Blank line at end of report */
    } else {				/* Malloc failure, no columns */
	for (i = 0; i < n; i++) {
	    if (s[i].flgs & CM_INV)	/* Use original keyword table */
	      continue;			/* skipping invisible entries */
	    printf("%s%s%s\n",pre,s[i].kwd,post);
	    if (++lc > (cmd_rows - 2)) { /* Screen full? */
		if (!askmore())		/* Do more-prompting... */
		  goto xkwdhelp;
		else
		  lc = 0;
	    }
	}
    }
  xkwdhelp:
    if (xhlp & 4) {
	if (tmpbuf) free((char *)tmpbuf);
	for (i = 0; i < n; i++)
	  if (s2[i]) free(s2[i]);
    }
    if (s2) free(s2);			/* Free array copy */
    if (b) free(b);			/* Free line buffer */
    return;
}

/*  X F I L H E L P  --  Given a file list, print names in columns.  */
/*
  Call with:
    n     - number of entries
    pre   - prefix to add to each filename
    post  - suffix to add to each filename
    off   - offset on first screenful, allowing room for introductory text
    cmdirflg - 1 if only directory names should be listed, 0 to list all files
    fs    - call fileselect() to decide whether to include each file.
    The rest of the args are the same as for fileselect().

  Arranges filenames in columns with width based on longest filename.
  Does "more?" prompting at end of screen.
  Uses global cmd_rows and cmd_cols for screen size.
*/

int
#ifdef CK_ANSIC
xfilhelp(
    int n, char *pre, char *post, int off, int cmdirflag,
    int fs, char *sa, char *sb, char *sna, char *snb,
    CK_OFF_T minsiz, CK_OFF_T maxsiz,
    int nbu, int nxlist,
    char ** xlist
)
#else
xfilhelp(n,pre,post,off,cmdirflg,
	 fs,sa,sb,sna,snb,minsiz,maxsiz,nbu,nxlist,xlist)
    int n, off; char *pre, *post; int cmdirflg;
    int fs; char *sa,*sb,*sna,*snb; CK_OFF_T minsiz,maxsiz;
    int nbu,nxlist; char ** xlist;
#endif	/* CK_ANSIC */
 {
    char filbuf[CKMAXPATH + 1];		/* Temp buffer for one filename */
    int width = 0;
    int cols, height, i, j, k, lc, n2 = 0, rc = 0, itsadir = 0;
    char *b = NULL, *p, *q;
    char *pa, *px;
    char **s2 = NULL;
#ifdef VMS
    char * cdp = zgtdir();
#endif /* VMS */

    if (n < 1) return(0);
    if (off < 0) off = 0;		/* Offset for first page */
    if (!pre) pre = "";			/* Handle null string pointers */
    if (!post) post = "";

    lc = off;				/* Screen-line counter */

    if ((s2 = (char **) malloc(n * sizeof(char *)))) {
	for (i = 0; i < n; i++) {	/* Loop through filenames */
	    itsadir = 0;
	    s2[i] = NULL;		/* Initialize each pointer to NULL */
	    znext(filbuf);		/* Get next filename */
	    if (!filbuf[0])		/* Shouldn't happen */
	      break;
#ifdef COMMENT
	    itsadir = isdir(filbuf);	/* Is it a directory? */
	    if (cmdirflg && !itsadir)	/* No, listing directories only? */
	      continue;			/* So skip this one. */
#endif /* COMMENT */
	    if (fs) if (fileselect(filbuf,
			   sa,sb,sna,snb,
			   minsiz,maxsiz,nbu,nxlist,xlist) < 1) {
                    continue;
	    }
#ifdef VMS
	    ckstrncpy(filbuf,zrelname(filbuf,cdp),CKMAXPATH);
#endif /* VMS */
	    j = strlen(filbuf);
#ifndef VMS
	    if (itsadir && j < CKMAXPATH - 1 && j > 0) {
		if (filbuf[j-1] != dirsep) {
		    filbuf[j++] = dirsep;
		    filbuf[j] = NUL;
		}
	    }
#endif /* VMS */
	    if (!(s2[n2] = malloc(j+1))) {
		printf("?Memory allocation failure\n");
		rc = -9;
		goto xfilhelp;
	    }
	    if (j <= CKMAXPATH) {
		strcpy(s2[n2],filbuf);
		n2++;
	    } else {
		printf("?Name too long - %s\n", filbuf);
		rc = -9;
		goto xfilhelp;
	    }
	    if (j > width)		/* Get width of widest one */
	      width = j;
	}
	n = n2;				/* How many we actually got */
    }
    sh_sort(s2,NULL,n,0,0,filecase);	/* Alphabetize the list */

    rc = 1;
    if (s2 && (b = (char *) malloc(cmd_cols + 1))) { /* Make a line buffer */
	char * bx;
	bx = b + cmd_cols;
	width += (int)strlen(pre) + (int)strlen(post) + 2;
	cols = cmd_cols / width;	/* How many columns? */
	if (cols < 1) cols = 1;
	height = n / cols;		/* How long is each column? */
	if (n % cols) height++;		/* Add one for remainder, if any */

	for (i = 0; i < height; i++) {	    /* Loop for each row */
	    for (j = 0; j < cmd_cols; j++)  /* First fill row with blanks */
	      b[j] = SP;
	    for (j = 0; j < cols; j++) {    /* Loop for each column in row */
		k = i + (j * height);       /* Index of next filename */
		if (k < n) {		    /* In range? */
		    pa = pre;
		    px = post;
		    p = s2[k];		               /* Point to filename */
		    q = b + (j * width) + 1;             /* and destination */
		    while ((q < bx) && (*q++ = *pa++)) ; /* Copy prefix */
		    q--;		                 /* Back up over NUL */
		    while ((q < bx) && (*q++ = *p++)) ;	 /* Copy filename */
		    q--;		                 /* Back up over NUL */
		    while ((q < bx) && (*q++ = *px++)) ; /* Copy suffix */
		    if (j < cols - 1) {
			q--;
			*q = SP;	/* Replace the space */
		    }
		}
	    }
	    p = b + cmd_cols - 1;	/* Last char in line */
	    while (*p-- == SP) ;	/* Trim */
	    *(p+2) = NUL;
	    printf("%s\n",b);		/* Print the line */
	    if (++lc > (cmd_rows - 2)) { /* Screen full? */
		if (!askmore()) {	/* Do more-prompting... */
		    rc = 0;
		    goto xfilhelp;
		} else
		  lc = 0;
	    }
	}
	printf("\n");			/* Blank line at end of report */
	goto xfilhelp;
    } else {				/* Malloc failure, no columns */
	for (i = 0; i < n; i++) {
	    znext(filbuf);
	    if (!filbuf[0]) break;
	    printf("%s%s%s\n",pre,filbuf,post);
	    if (++lc > (cmd_rows - 2)) { /* Screen full? */
		if (!askmore()) {	 /* Do more-prompting... */
		    rc = 0;
		    goto xfilhelp;
		} else lc = 0;
	    }
	}
xfilhelp:
	if (b) free(b);
	for (i = 0; i < n2; i++)
	  if (s2[i]) free(s2[i]);
	if (s2) free((char *)s2);
	return(rc);
    }
}

/*
  Simpler front end for xfilhelp() with shorter arg list when no
  file selection is needed.
*/
int
filhelp(n,pre,post,off,cmdirflg) int n, off; char *pre, *post; int cmdirflg; {
    return(xfilhelp(n,pre,post,off,cmdirflg,
		    0,NULL,NULL,NULL,NULL,
		    (CK_OFF_T)0,(CK_OFF_T)0,0,0,(char **)NULL));
}

/*  C M S E T U P  --  Set up command buffers  */

#ifdef DCMDBUF
int
cmsetup() {
    if (!(cmdbuf = malloc(CMDBL + 4))) return(-1);
    if (!(savbuf = malloc(CMDBL + 4))) return(-1);
    savbuf[0] = '\0';
    if (!(prevcmd = malloc(CMDBL + 4))) return(-1);
    prevcmd[0] = '\0';
    if (!(atmbuf = malloc(ATMBL + 4))) return(-1);
    if (!(atxbuf = malloc(CMDBL + 4))) return(-1);
    if (!(atybuf = malloc(ATMBL + 4))) return(-1);
    if (!(filbuf = malloc(ATMBL + 4))) return(-1);
    if (!(cmprom = malloc(PROMPTL + 4))) return(-1);
    if (!(cmprxx = malloc(PROMPTL + 4))) return(-1);
#ifdef CK_RECALL
    cmrini(cm_recall);
#endif /* CK_RECALL */
    return(0);
}
#endif /* DCMDBUF */

/*  C M S E T P  --  Set the program prompt.  */

VOID
cmsetp(s) char *s; {
    if (!s) s = "";
    ckstrncpy(cmprxx,s,PROMPTL);
    psetf = 1;                          /* Flag that prompt has been set. */
}

/*  C M S A V P  --  Save a copy of the current prompt.  */

VOID
#ifdef CK_ANSIC
cmsavp(char s[], int n)
#else
cmsavp(s,n) char s[]; int n;
#endif /* CK_ANSIC */
/* cmsavp */ {
    if (psetf)				/* But not if no prompt is set. */
      ckstrncpy(s,cmprxx,n);
}

char *
cmgetp() {
    return(cmprxx);
}

int
cmgbrk() {
    return(brkchar);
}

int
cmgkwflgs() {
    return(cmkwflgs);
}

/*  P R O M P T  --  Issue the program prompt.  */

VOID
prompt(f) xx_strp f; {
    char *sx, *sy; int n;
#ifdef CK_SSL
    extern int ssl_active_flag, tls_active_flag;
#endif /* CK_SSL */
#ifdef OS2
    extern int display_demo;

    /* If there is a demo screen to be displayed, display it */
    if (display_demo && xcmdsrc == 0) {
        demoscrn(VCMD);
        display_demo = 0;
    }
#endif /* OS2 */

    if (psetf == 0)			/* If no prompt set, set default. */
      cmsetp(dfprom);

    sx = cmprxx;			/* Unevaluated copy */
    if (f) {				/* If conversion function given */
	sy = cmprom;			/* Evaluate it */
#ifdef COMMENT
	debug(F101,"prompt sx","",sx);
	debug(F101,"prompt sy","",sy);
#endif	/* COMMENT */
	n = PROMPTL;
	if ((*f)(sx,&sy,&n) < 0)	/* If evaluation failed */
	  sx = cmprxx;			/* revert to unevaluated copy */
	else if (!*cmprom)		/* ditto if it came up empty */
	  sx = cmprxx;
	else
	  sx = cmprom;
    } else
      ckstrncpy(cmprom,sx,PROMPTL);
    cmprom[PROMPTL-1] = NUL;
    if (!*sx)				/* Don't print if empty */
      return;

#ifdef OSK
    fputs(sx, stdout);
#else
#ifdef MAC
    printf("%s", sx);
#else
#ifdef IKSD
    if (inserver) {			/* Print the prompt. */
        ttoc(CR);			/* If TELNET Server */
        ttoc(NUL);			/* must folloW CR by NUL */
        printf("%s",sx);
    } else
#endif /* IKSD */
      printf("\r%s",sx);
#ifdef CK_SSL
    if (!(ssl_active_flag || tls_active_flag))
#endif /* CK_SSL */
      fflush(stdout);			/* Now! */
#endif /* MAC */
#endif /* OSK */
}

#ifndef NOSPL
VOID
pushcmd(s) char * s; {			/* For use with IF command. */
    if (!s) s = np;
    ckstrncpy(savbuf,s,CMDBL);		/* Save the dependent clause,  */
    cmres();				/* and clear the command buffer. */
    debug(F110, "pushcmd savbuf", savbuf, 0);
}

VOID
pushqcmd(s) char * s; {			/* For use with ELSE command. */
    char c, * p = savbuf;		/* Dest */
    if (!s) s = np;			/* Source */
    while (*s) {			/* Get first nonwhitespace char */
	if (*s != SP)
	  break;
	else
	  s++;
    }
    if (*s != '{') {			/* If it's not "{" */
	pushcmd(s);			/* do regular pushcmd */
	return;
    }
    while ((c = *s++)) {		/* Otherwise insert quotes */
	if (c == CMDQ)
	  *p++ = CMDQ;
	*p++ = c;
    }
    cmres();				/* and clear the command buffer. */
    debug(F110, "pushqcmd savbuf", savbuf, 0);
}
#endif /* NOSPL */

#ifdef COMMENT
/* no longer used... */
VOID
popcmd() {
    ckstrncpy(cmdbuf,savbuf,CMDBL);	/* Put back the saved material */
    *savbuf = '\0';			/* and clear the save buffer */
    cmres();
}
#endif /* COMMENT */

/*  C M R E S  --  Reset pointers to beginning of command buffer.  */

VOID
cmres() {
    inword = 0;				/* We're not in a word */
    cc = 0;				/* Character count is zero */

/* Initialize pointers */

    pp = cmdbuf;			/* Beginning of current field */
    bp = cmdbuf;			/* Current position within buffer */
    np = cmdbuf;			/* Where to start next field */

    cmfldflgs = 0;
    cmflgs = -5;                        /* Parse not yet started. */
    ungw = 0;				/* Don't need to unget a word. */
}

/*  C M I N I  --  Clear the command and atom buffers, reset pointers.  */

/*
The argument specifies who is to echo the user's typein --
  1 means the cmd package echoes
  0 somebody else (system, front end, terminal) echoes
*/
VOID
cmini(d) int d; {
#ifdef DCMDBUF
    if (!atmbuf)
      if (cmsetup()<0)
	fatal("fatal error: unable to allocate command buffers");
#endif /* DCMDBUF */
#ifdef USE_MEMCPY
    memset(cmdbuf,0,CMDBL);
    memset(atmbuf,0,ATMBL);
#else
    for (bp = cmdbuf; bp < cmdbuf+CMDBL; bp++) *bp = NUL;
    for (bp = atmbuf; bp < atmbuf+ATMBL; bp++) *bp = NUL;
#endif /* USE_MEMCPY */

    *atmbuf = *savbuf = *atxbuf = *atybuf = *filbuf = NUL;
    blocklvl = 0;			/* Block level is 0 */
    linebegin = 1;			/* At the beginning of a line */
    dpx = d;				/* Global copy of the echo flag */
    debug(F101,"cmini dpx","",dpx);
    crflag = 0;				/* Reset flags */
    qmflag = 0;
    esflag = 0;
#ifdef CK_RECALL
    no_recall = 0;			/* Start out with recall enabled */
#endif /* CK_RECALL */
    cmres();				/* Sets bp etc */
    newcmd = 1;				/* See addcmd() */
}

#ifndef NOSPL
/*
  The following bits are to allow the command package to call itself
  in the middle of a parse.  To do this, begin by calling cmpush, and
  end by calling cmpop.  As you can see, this is rather expensive.
*/
#ifdef DCMDBUF
struct cmp {
    int i[5];				/* stack for integers */
    char *c[3];				/* stack for pointers */
    char *b[8];				/* stack for buffer contents */
};
struct cmp *cmp = 0;
#else
int cmp_i[CMDDEP+1][5];			/* Stack for integers */
char *cmp_c[CMDDEP+1][5];		/* for misc pointers */
char *cmp_b[CMDDEP+1][7];		/* for buffer contents pointers */
#endif /* DCMDBUF */

int cmddep = -1;			/* Current stack depth */

int
cmpush() {				/* Save the command environment */
    char *cp;				/* Character pointer */

    if (cmddep >= CMDDEP)		/* Enter a new command depth */
      return(-1);
    cmddep++;
    debug(F101,"&cmpush to depth","",cmddep);

#ifdef DCMDBUF
    /* allocate memory for cmp if not already done */
    if (!cmp && !(cmp = (struct cmp *) malloc(sizeof(struct cmp)*(CMDDEP+1))))
      fatal("cmpush: no memory for cmp");
    cmp[cmddep].i[0] = cmflgs;		/* First do the global ints */
    cmp[cmddep].i[1] = cmfsav;
    cmp[cmddep].i[2] = atxn;
    cmp[cmddep].i[3] = ungw;

    cmp[cmddep].c[0] = bp;		/* Then the global pointers */
    cmp[cmddep].c[1] = pp;
    cmp[cmddep].c[2] = np;
#else
    cmp_i[cmddep][0] = cmflgs;		/* First do the global ints */
    cmp_i[cmddep][1] = cmfsav;
    cmp_i[cmddep][2] = atxn;
    cmp_i[cmddep][3] = ungw;

    cmp_c[cmddep][0] = bp;		/* Then the global pointers */
    cmp_c[cmddep][1] = pp;
    cmp_c[cmddep][2] = np;
#endif /* DCMDBUF */

    /* Now the buffers themselves.  A lot of repititious code... */

#ifdef DCMDBUF
    cp = malloc((int)strlen(cmdbuf)+1);	/* 0: Command buffer */
    if (cp) strcpy(cp,cmdbuf);
    cmp[cmddep].b[0] = cp;
    if (cp == NULL) return(-1);

    cp = malloc((int)strlen(savbuf)+1);	/* 1: Save buffer */
    if (cp) strcpy(cp,savbuf);
    cmp[cmddep].b[1] = cp;
    if (cp == NULL) return(-1);

    cmp[cmddep].b[2] = NULL;

    cp = malloc((int)strlen(atmbuf)+1);	/* 3: Atom buffer */
    if (cp) strcpy(cp,atmbuf);
    cmp[cmddep].b[3] = cp;
    if (cp == NULL) return(-1);

    cp = malloc((int)strlen(atxbuf)+1);	/* 4: Expansion buffer */
    if (cp) strcpy(cp,atxbuf);
    cmp[cmddep].b[4] = cp;
    if (cp == NULL) return(-1);

    cp = malloc((int)strlen(atybuf)+1);	/* 5: Atom buffer copy */
    if (cp) strcpy(cp,atybuf);
    cmp[cmddep].b[5] = cp;
    if (cp == NULL) return(-1);

    cp = malloc((int)strlen(filbuf)+1);	/* 6: File name buffer */
    if (cp) strcpy(cp,filbuf);
    cmp[cmddep].b[6] = cp;
    if (cp == NULL) return(-1);
#else
    cp = malloc((int)strlen(cmdbuf)+1);	/* 0: Command buffer */
    if (cp) strcpy(cp,cmdbuf);
    cmp_b[cmddep][0] = cp;
    if (cp == NULL) return(-1);

    cp = malloc((int)strlen(savbuf)+1);	/* 1: Save buffer */
    if (cp) strcpy(cp,savbuf);
    cmp_b[cmddep][1] = cp;
    if (cp == NULL) return(-1);

    cmp_b[cmddep][2] = NULL;

    cp = malloc((int)strlen(atmbuf)+1);	/* 3: Atom buffer */
    if (cp) strcpy(cp,atmbuf);
    cmp_b[cmddep][3] = cp;
    if (cp == NULL) return(-1);

    cp = malloc((int)strlen(atxbuf)+1);	/* 4: Expansion buffer */
    if (cp) strcpy(cp,atxbuf);
    cmp_b[cmddep][4] = cp;
    if (cp == NULL) return(-1);

    cp = malloc((int)strlen(atybuf)+1);	/* 5: Atom buffer copy */
    if (cp) strcpy(cp,atybuf);
    cmp_b[cmddep][5] = cp;
    if (cp == NULL) return(-1);

    cp = malloc((int)strlen(filbuf)+1);	/* 6: File name buffer */
    if (cp) strcpy(cp,filbuf);
    cmp_b[cmddep][6] = cp;
    if (cp == NULL) return(-1);
#endif /* DCMDBUF */

    cmini(dpx);				/* Initize the command parser */
    return(0);
}

int
cmpop() {				/* Restore the command environment */
    if (cmddep < 0) {
	debug(F100,"&cmpop called from top level","",0);
	return(-1);			/* Don't pop too much! */
    }
#ifdef DCMDBUF
    cmflgs = cmp[cmddep].i[0];		/* First do the global ints */
    cmfsav = cmp[cmddep].i[1];
    atxn = cmp[cmddep].i[2];
    ungw = cmp[cmddep].i[3];

    bp = cmp[cmddep].c[0];		/* Then the global pointers */
    pp = cmp[cmddep].c[1];
    np = cmp[cmddep].c[2];
#else
    cmflgs = cmp_i[cmddep][0];		/* First do the global ints */
    cmfsav = cmp_i[cmddep][1];
    atxn = cmp_i[cmddep][2];
    ungw = cmp_i[cmddep][3];

    bp = cmp_c[cmddep][0];		/* Then the global pointers */
    pp = cmp_c[cmddep][1];
    np = cmp_c[cmddep][2];
#endif /* DCMDBUF */

    /* Now the buffers themselves. */
    /* Note: strncpy(), not ckstrncpy() -- Here we WANT the NUL padding... */

#ifdef DCMDBUF
    if (cmp[cmddep].b[0]) {

	strncpy(cmdbuf,cmp[cmddep].b[0],CMDBL); /* 0: Command buffer */
	free(cmp[cmddep].b[0]);
	cmp[cmddep].b[0] = NULL;
    }
    if (cmp[cmddep].b[1]) {
	strncpy(savbuf,cmp[cmddep].b[1],CMDBL); /* 1: Save buffer */
	free(cmp[cmddep].b[1]);
	cmp[cmddep].b[1] = NULL;
    }
    if (cmp[cmddep].b[3]) {
	strncpy(atmbuf,cmp[cmddep].b[3],ATMBL); /* 3: Atomic buffer! */
	free(cmp[cmddep].b[3]);
	cmp[cmddep].b[3] = NULL;
    }
    if (cmp[cmddep].b[4]) {
	strncpy(atxbuf,cmp[cmddep].b[4],ATMBL); /* 4: eXpansion buffer */
	free(cmp[cmddep].b[4]);
	cmp[cmddep].b[4] = NULL;
    }
    if (cmp[cmddep].b[5]) {
	strncpy(atybuf,cmp[cmddep].b[5],ATMBL); /* 5: Atom buffer copY */
	free(cmp[cmddep].b[5]);
	cmp[cmddep].b[5] = NULL;
    }
    if (cmp[cmddep].b[6]) {
	strncpy(filbuf,cmp[cmddep].b[6],ATMBL); /* 6: Filename buffer */
	free(cmp[cmddep].b[6]);
	cmp[cmddep].b[6] = NULL;
    }
#else
    if (cmp_b[cmddep][0]) {
	strncpy(cmdbuf,cmp_b[cmddep][0],CMDBL); /* 0: Command buffer */
	free(cmp_b[cmddep][0]);
	cmp_b[cmddep][0] = NULL;
    }
    if (cmp_b[cmddep][1]) {
	strncpy(savbuf,cmp_b[cmddep][1],CMDBL); /* 1: Save buffer */
	free(cmp_b[cmddep][1]);
	cmp_b[cmddep][1] = NULL;
    }
    if (cmp_b[cmddep][3]) {
	strncpy(atmbuf,cmp_b[cmddep][3],ATMBL); /* 3: Atomic buffer! */
	free(cmp_b[cmddep][3]);
	cmp_b[cmddep][3] = NULL;
    }
    if (cmp_b[cmddep][4]) {
	strncpy(atxbuf,cmp_b[cmddep][4],ATMBL); /* 4: eXpansion buffer */
	free(cmp_b[cmddep][4]);
	cmp_b[cmddep][4] = NULL;
    }
    if (cmp_b[cmddep][5]) {
	strncpy(atybuf,cmp_b[cmddep][5],ATMBL); /* 5: Atom buffer copY */
	free(cmp_b[cmddep][5]);
	cmp_b[cmddep][5] = NULL;
    }
    if (cmp_b[cmddep][6]) {
	strncpy(filbuf,cmp_b[cmddep][6],ATMBL); /* 6: Filename buffer */
	free(cmp_b[cmddep][6]);
	cmp_b[cmddep][6] = NULL;
    }
#endif /* DCMDBUF */

    cmddep--;				/* Rise, rise */
    debug(F101,"&cmpop to depth","",cmddep);
    return(cmddep);
}
#endif /* NOSPL */

#ifdef COMMENT
VOID					/* Not used */
stripq(s) char *s; {                    /* Function to strip '\' quotes */
    char *t;
    while (*s) {
        if (*s == CMDQ) {
            for (t = s; *t != '\0'; t++) *t = *(t+1);
        }
        s++;
    }
}
#endif /* COMMENT */

/* Convert tabs to spaces, one for one */
VOID
untab(s) char *s; {
    while (*s) {
	if (*s == HT) *s = SP;
	s++;
    }
}

/*  C M N U M  --  Parse a number in the indicated radix  */

/*
 The radix is specified in the arg list.
 Parses unquoted numeric strings in the given radix.
 Parses backslash-quoted numbers in the radix indicated by the quote:
   \nnn = \dnnn = decimal, \onnn = octal, \xnn = Hexadecimal.
 If these fail, then if a preprocessing function is supplied, that is applied
 and then a second attempt is made to parse an unquoted decimal string.
 And if that fails, the preprocessed string is passed to an arithmetic
 expression evaluator.

 Returns:
   -3 if no input present when required,
   -2 if user typed an illegal number,
   -1 if reparse needed,
    0 otherwise, with argument n set to the number that was parsed
*/
/* This is the traditional cmnum() that gets an int */
int
cmnum(xhlp,xdef,radix,n,f) char *xhlp, *xdef; int radix, *n; xx_strp f; {
    CK_OFF_T z = (CK_OFF_T)0, check;
    int x;
    x = cmnumw(xhlp,xdef,radix,&z,f);
    *n = z;
    check = *n;
    if (check != z) {
	printf("?Magnitude of result too large for integer - %s\n",ckfstoa(z));
	return(-9);
    }
    return(x);
}

/*
  This is the new cmnum() that gets a "wide" result, whatever CK_OFF_T
  is defined to be, normally 32 or 64 bits, depending on the platform.
  fdc, 24 Dec 2005.
*/
int
cmnumw(xhlp,xdef,radix,n,f)
    char *xhlp, *xdef; int radix; CK_OFF_T *n; xx_strp f; {
    int x; char *s, *zp, *zq;
#ifdef COMMENT
    char lbrace, rbrace;
#endif /* COMMENT */

    if (!xhlp) xhlp = "";
    if (!xdef) xdef = "";

#ifdef COMMENT
    if (cmfldflgs & 1) {
	lbrace = '(';
	rbrace = ')';
    } else {
	lbrace = '{';
	rbrace = '}';
    }
#endif /* COMMENT */

    if (radix != 10 && radix != 8) {	/* Just do bases 8 and 10 */
        printf("cmnum: illegal radix - %d\n",radix);
        return(-2);
    } /* Easy to add others but there has never been a need for it. */
    x = cmfld(xhlp,xdef,&s,(xx_strp)0);
    debug(F101,"cmnum: cmfld","",x);
    if (x < 0) return(x);		/* Parse a field */
    zp = atmbuf;
/*
  Edit 192 - Allow any number field to be braced.  This lets us include
  spaces in expressions, but perhaps more important lets us have user-defined
  functions in numeric fields.
*/
    zp = brstrip(zp);			/* Strip braces */
    if (cmfldflgs & 1 && *zp == '(') {	/* Parens too.. */
	x = (int) strlen(atmbuf);
	if (x > 0) {
	    if (*(atmbuf+x-1) == ')') {
		*(atmbuf+x-1) = NUL;
		zp++;
	    }
	}
    }
    if (chknum(zp)) {			/* Check for number */
	if (radix == 8) {		/* If it's supposed to be octal */
	    zp = ckradix(zp,8,10);	/* convert to decimal */
	    if (!zp) return(-2);
	    if (!strcmp(zp,"-1")) return(-2);
	}
	errno = 0;			/* Got one, we're done. */
        *n = ckatofs(zp);
	if (errno) {
	    perror(zp);
	    return(-9);
	}
	debug(F101,"cmnum 1st chknum ok","",*n);
        return(0);
    } else if ((x = xxesc(&zp)) > -1) {	/* Check for backslash escape */

#ifndef OS2
	*n = x;
#else
	*n = wideresult;
#endif /* OS2 */

	debug(F101,"cmnum xxesc ok","",*n);
	return(*zp ? -2 : 0);
    } else if (f) {			/* If conversion function given */
	zq = atxbuf;			/* Try that */
	atxn = CMDBL;
	if ((*f)(zp,&zq,&atxn) < 0)	/* Convert */
	  return(-2);
	zp = atxbuf;
    }
    debug(F110,"cmnum zp 1",zp,0);
    if (!*zp) zp = xdef;		/* Result empty, substitute default */
    debug(F110,"cmnum zp 2",zp,0);
    if (chknum(zp)) {			/* Check again for decimal number */
	if (radix == 8) {		/* If it's supposed to be octal */
	    zp = ckradix(zp,8,10);	/* convert to decimal */
	    if (!zp) return(-2);
	    if (!strcmp(zp,"-1")) return(-2);
	}
	errno = 0;
        *n = ckatofs(zp);
	if (errno) {
	    perror(zp);
	    return(-9);
	}
	debug(F101,"cmnum 2nd chknum ok","",*n);
        return(0);
#ifndef NOSPL
    }  else if ((x = xxesc(&zp)) > -1) { /* Check for backslash escape */
#ifndef OS2
	*n = x;
#else
	*n = wideresult;
#endif /* OS2 */
	debug(F101,"cmnum xxesc 2 ok","",*n);
	return(*zp ? -2 : 0);
    } else if (f) {			/* Not numeric, maybe an expression */
	char * p;
	p = evala(zp);
	if (chknum(p)) {
	    if (radix == 8) {		/* If it's supposed to be octal */
		zp = ckradix(zp,8,10);	/* convert to decimal */
		if (!zp) return(-2);
		if (!strcmp(zp,"-1")) return(-2);
	    }
	    errno = 0;
	    *n = ckatofs(p);
	    if (errno) {
		perror(p);
		return(-9);
	    }
	    debug(F101,"cmnum exp eval ok","",*n);
	    return(0);
	} else return(-2);
#endif /* NOSPL */
    } else {				/* Not numeric */
	return(-2);
    }
}

#ifdef CKCHANNELIO
extern int z_error;
#endif /* CKCHANNELIO */

/*  C M O F I  --  Parse the name of an output file  */

/*
 Depends on the external function zchko(); if zchko() not available, use
 cmfld() to parse output file names.

 Returns:
   -9 like -2, except message already printed,
   -3 if no input present when required,
   -2 if permission would be denied to create the file,
   -1 if reparse needed,
    0 or 1 if file can be created, with xp pointing to name.
    2 if given the name of an existing directory.
*/
int
cmofi(xhlp,xdef,xp,f) char *xhlp, *xdef, **xp; xx_strp f; {
    int x; char *s, *zq;
#ifdef DOCHKVAR
    int tries;
#endif /* DOCHKVAR */
#ifdef DTILDE
    char *dirp;
#endif /* DTILDE */

    cmfldflgs = 0;

    if (!xhlp) xhlp = "";
    if (!xdef) xdef = "";

    if (*xhlp == NUL) xhlp = "Output file";
    *xp = "";

    debug(F110,"cmofi xdef",xdef,0);
    x = cmfld(xhlp,xdef,&s,(xx_strp)0);
    debug(F111,"cmofi cmfld returns",s,x);
    if (x < 0)
      return(x);

    s = brstrip(s);			/* Strip enclosing braces */
    debug(F110,"cmofi 1.5",s,0);

#ifdef DOCHKVAR
    tries = 0;
    {
	char *p = s;
    /*
      This is really ugly.  If we skip conversion the first time through,
      then variable names like \%a will be used as filenames (e.g. creating
      a file called %A in the root directory).  If we DON'T skip conversion
      the first time through, then single backslashes used as directory
      separators in filenames will be misinterpreted as variable lead-ins.
      So we prescan to see if it has any variable references.  But this
      module is not supposed to know anything about variables, functions,
      etc, so this code does not really belong here, but rather it should
      be at the same level as zzstring().
    */
/*
  Hmmm, this looks a lot like chkvar() except it that includes \nnn number
  escapes.  But why?  This makes commands like "mkdir c:\123" impossible.
  And in fact, "mkdir c:\123" creates a directory called "c:{".  What's worse,
  rmdir(), which *does* call chkvar(), won't let us remove it.  So let's at
  least try making cmofi() symmetrical with cmifi()...
*/
#ifdef COMMENT
	char * q;
	while ( (tries == 0) && (p = strchr(p,CMDQ)) ) {
	    q = *(p+1);			/* Char after backslash */
	    if (!q)			/* None, quit */
	      break;
	    if (isupper(q))		/* If letter, convert to lowercase */
	      q = tolower(q);
	    if (isdigit(q)) {		/* If it's a digit, */
		tries = 1;		/* assume it's a backslash code  */
		break;
	    }
	    switch (q) {
	      case CMDQ:		/* Double backslash */
		tries = 1;		/* so call the conversion function */
		break;
	      case '%':			/* Variable or array reference */
	      case '&':			/* must be followed by letter */
		if (isalpha(*(p+2)) || (*(p+2) >= '0' && *(p+2) <= '9'))
		  tries = 1;
		break;
	      case 'm': case 'v': case '$': /* \m(), \v(), \$() */
		if (*(p+2) == '(')
		  if (strchr(p+2,')'))
		    tries = 1;
		break;
	      case 'f':			/* \Fname() */
		if (strchr(p+2,'('))
		  if (strchr(p+2,')'))
		      tries = 1;
		break;
	      case '{':			/* \{...} */
		if (strchr(p+2,'}'))
		  tries = 1;
		break;
	      case 'd': case 'o':	/* Decimal or Octal number */
	        if (isdigit(*(p+2)))
		  tries = 1;
		break;
	      case 'x':			/* Hex number */
		if (isdigit(*(p+2)) ||
		    ((*(p+2) >= 'a' && *(p+2) <= 'f') ||
		     ((*(p+2) >= 'A' && *(p+2) <= 'F'))))
		  tries = 1;
	      default:
		break;
	    }
	    p++;
	}
#else
#ifndef NOSPL
	if (f) {			/* If a conversion function is given */
	    char *s = p;		/* See if there are any variables in */
	    while (*s) {		/* the string and if so, expand them */
		if (chkvar(s)) {
		    tries = 1;
		    break;
		}
		s++;
	    }
	}
#endif /* NOSPL */
#endif /* COMMENT */
    }
#ifdef OS2
o_again:
#endif /* OS2 */
    if (tries == 1)
#endif /* DOCHKVAR */
    if (f) {				/* If a conversion function is given */
	zq = atxbuf;			/* do the conversion. */
	atxn = CMDBL;
	if ((x = (*f)(s,&zq,&atxn)) < 0)
	  return(-2);
	s = atxbuf;
	if (!*s)			/* Result empty, substitute default */
	  s = xdef;
    }
    debug(F111,"cmofi 2",s,x);

#ifdef DTILDE
    dirp = tilde_expand(s);		/* Expand tilde, if any, */
    if (*dirp != '\0') {		/* right in the atom buffer. */
	if (setatm(dirp,1) < 0) {
	    printf("?Name too long\n");
	    return(-9);
	}
    }
    s = atmbuf;
    debug(F110,"cmofi 3",s,0);
#endif /* DTILDE */

    if (iswild(s)) {
        printf("?Wildcards not allowed - %s\n",s);
        return(-2);
    }
    debug(F110,"cmofi 4",s,0);

#ifdef CK_TMPDIR
    /* isdir() function required for this! */
    if (isdir(s)) {
	debug(F110,"cmofi 5: is directory",s,0);
        *xp = s;
	return(2);
    }
#endif /* CK_TMPDIR */

    if (strcmp(s,CTTNAM) && (zchko(s) < 0)) { /* OK to write to console */
#ifdef COMMENT
#ifdef OS2
/*
  We don't try again because we already prescanned the string to see if
  if it contained anything that could be used by zzstring().
*/
	if (tries++ < 1)
	  goto o_again;
#endif /* OS2 */
#endif /* COMMENT */
/*
  Note: there are certain circumstances where zchko() can give a false
  positive, so don't rely on it to catch every conceivable situation in
  which the given output file can't be created.  In other words, we print
  a message and fail here if we KNOW the file can't be created.  If we
  succeed but the file can't be opened, the code that tries to open the file
  has to print a message.
*/
	debug(F110,"cmofi 6: failure",s,0);
#ifdef CKROOT
	if (ckrooterr)
	  printf("?Off Limits: %s\n",s);
	else
#endif /* CKROOT */
	  printf("?Write permission denied - %s\n",s);
#ifdef CKCHANNELIO
	z_error = FX_ACC;
#endif /* CKCHANNELIO */
        return(-9);
    } else {
	debug(F110,"cmofi 7: ok",s,0);
        *xp = s;
        return(x);
    }
}

/*  C M I F I  --  Parse the name of an existing file  */

/*
 This function depends on the external functions:
   zchki()  - Check if input file exists and is readable.
   zxpand() - Expand a wild file specification into a list.
   znext()  - Return next file name from list.
 If these functions aren't available, then use cmfld() to parse filenames.
*/
/*
 Returns
   -4 EOF
   -3 if no input present when required,
   -2 if file does not exist or is not readable,
   -1 if reparse needed,
    0 or 1 otherwise, with:
        xp pointing to name,
        wild = 1 if name contains '*' or '?', 0 otherwise.
*/

#ifdef COMMENT /* This horrible hack has been replaced - see further down */
/*
   C M I O F I  --  Parse an input file OR the name of a nonexistent file.

   Use this when an existing file is wanted (so we get help, completion, etc),
   but if a file of the given name does not exist, the name of a new file is
   accepted.  For example, with the EDIT command (edit an existing file, or
   create a new file).  Returns -9 if file does not exist.  It is up to the
   caller to check creatability.
*/
static int nomsg = 0;
int
cmiofi(xhlp,xdef,xp,wild,f) char *xhlp, *xdef, **xp; int *wild; xx_strp f; {
    int msgsave, x;
    msgsave = nomsg;
    nomsg = 1;
    x = cmifi2(xhlp,xdef,xp,wild,0,NULL,f,0);
    nomsg = msgsave;
    return(x);
}
#endif	/* COMMENT */

int
cmifi(xhlp,xdef,xp,wild,f) char *xhlp, *xdef, **xp; int *wild; xx_strp f; {
    return(cmifi2(xhlp,xdef,xp,wild,0,NULL,f,0));
}
/*
  cmifip() is called when we want to supply a path or path list to search
  in case the filename that the user gives is (a) not absolute, and (b) can't
  be found as given.  The path string can be the name of a single directory,
  or a list of directories separated by the PATHSEP character, defined in
  ckucmd.h.  Look in ckuusr.c and ckuus3.c for examples of usage.
*/
int
cmifip(xhlp,xdef,xp,wild,d,path,f)
    char *xhlp,*xdef,**xp; int *wild, d; char * path; xx_strp f; {
    return(cmifi2(xhlp,xdef,xp,wild,0,path,f,0));
}

/*  C M D I R  --  Parse a directory name  */

/*
 This function depends on the external functions:
   isdir(s)  - Check if string s is the name of a directory
   zchki(s)  - Check if input file s exists and what type it is.
 If these functions aren't available, then use cmfld() to parse dir names.

 Returns
   -9 For all sorts of reasons, after printing appropriate error message.
   -4 EOF
   -3 if no input present when required,
   -2 if out of space or other internal error,
   -1 if reparse needed,
    0 or 1, with xp pointing to name, if directory specified,
*/
int
cmdir(xhlp,xdef,xp,f) char *xhlp, *xdef, **xp; xx_strp f; {
    int wild;
    return(cmifi2(xhlp,xdef,xp,&wild,0,NULL,f,1));
}

/* Like CMDIR but includes PATH search */

int
cmdirp(xhlp,xdef,xp,path,f) char *xhlp, *xdef, **xp; char * path; xx_strp f; {
    int wild;
    return(cmifi2(xhlp,xdef,xp,&wild,0,path,f,1));
}

/*
  cmifi2() is the base filename parser called by cmifi, cmifip, cmdir, etc.
  Use it directly when you also want to parse a directory or device
  name as an input file, as in the DIRECTORY command.  Call with:
    xhlp  -- help message on ?
    xdef  -- default response
    xp    -- pointer to result (in our space, must be copied from here)
    wild  -- flag set upon return to indicate if filespec was wild
    d     -- 0 to parse files, 1 to parse files or directories
             Add 2 to inhibit following of symlinks.
    path  -- search path for files
    f     -- pointer to string processing function (e.g. to evaluate variables)
    dirflg -- 1 to parse *only* directories, 0 otherwise
*/
int
cmifi2(xhlp,xdef,xp,wild,d,path,f,dirflg)
    char *xhlp,*xdef,**xp; int *wild, d; char * path; xx_strp f; int dirflg; {
    extern int recursive, diractive, cdactive, dblquo;
    int i, x, itsadir, xc, expanded = 0, nfiles = 0, children = -1;
    int qflag = 0;
    long y;
    CK_OFF_T filesize;
    char *sp = NULL, *zq, *np = NULL;
    char *sv = NULL, *p = NULL;
#ifdef DTILDE
    char *dirp;
#endif /* DTILDE */

#ifndef NOPARTIAL
#ifndef OS2
#ifdef OSK
    /* This large array is dynamic for OS-9 -- should do for others too... */
    extern char **mtchs;
#else
#ifdef UNIX
    /* OK, for UNIX too */
    extern char **mtchs;
#else
#ifdef VMS
    extern char **mtchs;
#else
    extern char *mtchs[];
#endif /* VMS */
#endif /* UNIX */
#endif /* OSK */
#endif /* OS2 */
#endif /* NOPARTIAL */

    if (!xhlp) xhlp = "";
    if (!xdef) xdef = "";

#ifndef NOLASTFILE
    makestr(&tmplastfile,NULL);
#endif	/* NOLASTFILE */
    nzxopts = 0;			/* zxpand() options */
    debug(F101,"cmifi d","",d);
    if (d & 2) {			/* d & 2 means don't follow symlinks */
	d ^= 2;
	nzxopts = ZX_NOLINKS;
    }
    debug(F101,"cmifi nzxopts","",nzxopts);
    cmfldflgs = 0;
    if (path)
      if (!*path)
	path = NULL;
    if (path) {				/* Make a copy we can poke */
	x = strlen(path);
	np = (char *) malloc(x + 1);
	if (np) {
	    strcpy(np, path);
	    path = sp = np;
	}
    }
    debug(F110,"cmifi2 path",path,0);

    ckstrncpy(cmdefault,xdef,CMDEFAULT); /* Copy default */
    xdef = cmdefault;

    inword = 0;				/* Initialize counts & pointers */
    cc = 0;
    xc = 0;
    *xp = "";				/* Pointer to result string */
    if ((x = cmflgs) != 1) {            /* Already confirmed? */
#ifdef BS_DIRSEP
	dirnamflg = 1;
        x = gtword(0);			/* No, get a word */
	dirnamflg = 0;
#else
        x = gtword(0);                  /* No, get a word */
#endif /* BS_DIRSEP */
    } else {				/* If so, use default, if any. */
        if (setatm(xdef,1) < 0) {
	    printf("?Default name too long\n");
	    if (np) free(np);
	    return(-9);
	}
    }
  i_path:
    *xp = atmbuf;                       /* Point to result. */

    while (1) {
        xc += cc;                       /* Count this character. */
        debug(F111,"cmifi gtword",atmbuf,xc);
	debug(F101,"cmifi switch x","",x);
        switch (x) {			/* x = gtword() return code */
	  case -10:
	    if (gtimer() > timelimit) {
#ifdef IKSD
                if (inserver) {
                    printf("\r\nIKSD IDLE TIMEOUT: %d sec\r\n", timelimit);
                    doexit(GOOD_EXIT,0);
                }
#endif /* IKSD */
		/* if (!quiet) printf("?Timed out\n"); */
		return(-10);
	    } else {
		x = gtword(0);
		continue;
	    }
	  case -9:
	    printf("Command or field too long\n");
	  case -4:			/* EOF */
	  case -2:			/* Out of space. */
	  case -1:			/* Reparse needed */
	    if (np) free(np);
	    return(x);
	  case 1:			/* CR */
	  case 0:			/* SP */
	    if (xc == 0)		/* If no input... */
	      *xp = xdef;		/* substitute the default */
#ifndef NOLASTFILE
	    makestr(&tmplastfile,*xp);	/* Make a copy before bstripping */
#endif	/* #ifndef NOLASTFILE */
	    *xp = brstrip(*xp);		/* Strip braces */
	    if (**xp == NUL) {		/* 12 mar 2001 */
		if (np) free(np);
		return(-3);
	    }
	    debug(F110,"cmifi brstrip",*xp,0);
#ifndef NOSPL
	    if (f) {			/* If a conversion function is given */
#ifdef DOCHKVAR
		char *s = *xp;		/* See if there are any variables in */
		int x;
		while (*s) {		/* the string and if so, expand them */
		    x = chkvar(s);
		    /* debug(F111,"cmifi chkvar",*xp,x); */
		    if (x) {
#endif /* DOCHKVAR */
			zq = atxbuf;
			atxn = CMDBL;
			if ((*f)(*xp,&zq,&atxn) < 0) {
			    if (np) free(np);
			    return(-2);
			}
			*xp = atxbuf;
			if (!atxbuf[0])
			  *xp = xdef;
#ifdef DOCHKVAR
			break;
		    }
		    s++;
		}
#endif /* DOCHKVAR */
	    }
#endif /* NOSPL */
	    if (**xp == NUL) {		/* 12 mar 2001 */
		if (np) free(np);
		return(-3);
	    }
#ifdef DTILDE
	    if (dirflg) {
		dirp = tilde_expand(*xp); /* Expand tilde, if any, */
		if (*dirp != '\0') {	/* in the atom buffer. */
		    if (setatm(dirp,1) < 0) {
			printf("Expanded name too long\n");
			if (np) free(np);
			return(-9);
		    }
		}
		*xp = atmbuf;
		debug(F110,"cmifi tilde_expand",*xp,0);
	    }
#endif /* DTILDE */
	    if (!sv) {			/* Only do this once */
		sv = malloc((int)strlen(*xp)+1); /* Make a safe copy */
		if (!sv) {
		    printf("?cmifi: malloc error\n");
		    if (np) free(np);
		    return(-9);
		}
		strcpy(sv,*xp);
		debug(F110,"cmifi sv",sv,0);
	    }

/* This is to get around "cd /" failing because "too many directories match" */

	    expanded = 0;		/* Didn't call zxpand */
#ifdef datageneral
	    debug(F110,"cmifi isdir 1",*xp,0);
	    {
		int y; char *s;
		s = *xp;
		y = strlen(s);
		if (y > 1 &&
		    (s[y-1] == ':' ||
		     s[y-1] == '^' ||
		     s[y-1] == '=')
		    )
		  s[y-1] = NUL;
	    }
	    debug(F110,"cmifi isdir 2",*xp,0);
#endif /*  datageneral */

#ifdef VMS
	    if (dirflg) {
		if (!strcmp(*xp,"..")) { /* For UNIXers... */
		    setatm("-",0);
		    *xp = atmbuf;
		} else if (!strcmp(*xp,".")) {
		    setatm("[]",0);
		    *xp = atmbuf;
		}
	    }
#endif /* VMS */
	    itsadir = isdir(*xp);	/* Is it a directory? */
	    debug(F111,"cmifi itsadir",*xp,itsadir);
#ifdef VMS
	    /* If they said "blah" where "blah.dir" is a directory... */
	    /* change it to [.blah]. */
	    if (!itsadir) {
		char tmpbuf[600];
		int flag = 0; char c, * p;
		p = *xp;
		while ((c = *p++) && !flag)
		  if (ckstrchr(".[]:*?<>",c))
		    flag = 1;
		debug(F111,"cmifi VMS dirname flag",*xp,flag);
		if (!flag) {
		    ckmakmsg(tmpbuf,TMPBUFSIZ,"[.",*xp,"]",NULL);
		    itsadir = isdir(tmpbuf);
		    if (itsadir) {
			setatm(tmpbuf,0);
			*xp = atmbuf;
		    }
		    debug(F111,"cmifi VMS dirname flag itsadir",*xp,itsadir);
		}
	    } else if (itsadir == 1 && *(xp[0]) == '.' && *(xp[1])) {
		char *p;
		if (p = malloc(cc + 4)) {
		    ckmakmsg(p,cc+4,"[",*xp,"]",NULL);
		    setatm(p,0);
		    *xp = atmbuf;
		    debug(F110,"cmdir .foo",*xp,0);
		    free(p);
		}
	    } else if (itsadir == 2 && !diractive) {
		int x;			/* [FOO]BAR.DIR instead of [FOO.BAR] */
		char *p;
		p = malloc(cc + 4);
		if (p) {
		    x = cvtdir(*xp,p,ATMBL); /* Convert to [FOO.BAR] */
		    if (x > 0) {
			setatm(p,0);
			*xp = atmbuf;
			debug(F110,"cmdir cvtdir",*xp,0);
		    }
		    free(p);
		}
	    }
#endif /* VMS */

	    debug(F101,"cmifi dirflg","",dirflg);
	    if (dirflg) {		/* Parsing a directory name? */
		/* Yes, does it contain wildcards? */
		if (iswild(*xp) ||
		    (diractive && (!strcmp(*xp,".")  || !strcmp(*xp,"..")))
		    ) {
		    nzxopts |= ZX_DIRONLY; /* Match only directory names */
		    if (matchdot)  nzxopts |= ZX_MATCHDOT;
		    if (recursive) nzxopts |= ZX_RECURSE;
		    debug(F111,"cmifi nzxopts 2",*xp,nzxopts);
		    y = nzxpand(*xp,nzxopts);
		    debug(F111,"cmifi nzxpand 2",*xp,y);
		    nfiles = y;
		    expanded = 1;
		} else {
#ifdef VMS
/*
  This is to allow (e.g.) "cd foo", where FOO.DIR;1 is in the
  current directory.
*/
		    debug(F111,"cmdir itsadir",*xp,itsadir);
		    if (!itsadir) {
			char *s;
			int n;
			s = *xp;
			n = strlen(s);
			if (n > 0 &&
#ifdef COMMENT
			    *s != '[' && s[n-1] != ']' &&
			    *s != '<' && s[n-1] != '>' &&
#else
			    ckindex("[",s,0,0,1) == 0 &&
			    ckindex("<",s,0,0,1) == 0 &&
#endif /* COMMENT */
			    s[n-1] != ':') {
			    char * dirbuf = NULL;
			    dirbuf = (char *)malloc(n+4);
			    if (dirbuf) {
				if (*s == '.')
				  ckmakmsg(dirbuf,n+4,"[",s,"]",NULL);
				else
				  ckmakmsg(dirbuf,n+4,"[.",s,"]",NULL);
				itsadir = isdir(dirbuf);
				debug(F111,"cmdir dirbuf",dirbuf,itsadir);
				if (itsadir) {
				    setatm(dirbuf,0);
				    *xp = atmbuf;
				    debug(F110,"cmdir new *xp",*xp,0);
				}
				free(dirbuf);
			    }

/* This is to allow CDPATH to work in VMS... */

			} else if (n > 0) {
			    char * p; int i, j, k, d;
			    char rb[2] = "]";
			    if (p = malloc(x + 8)) {
				ckstrncpy(p,*xp,x+8);
				i = ckindex(".",p,-1,1,1);
				d = ckindex(".dir",p,0,0,0);
				j = ckindex("]",p,-1,1,1);
				if (j == 0) {
				    j = ckindex(">",p,-1,1,1);
				    rb[0] = '>';
				}
				k = ckindex(":",p,-1,1,1);
				if (i < j || i < k) i = 0;
				if (d < j || d < k) d = 0;
				/* Change [FOO]BAR or [FOO]BAR.DIR */
				/* to [FOO.BAR] */
				if (j > 0 && j < n) {
				    p[j-1] = '.';
				    if (d > 0) p[d-1] = NUL;
				    ckstrncat(p,rb,x+8);
				    debug(F110,"cmdir xxx",p,0);
				}
				itsadir = isdir(p);
				debug(F111,"cmdir p",p,itsadir);
				if (itsadir) {
				    setatm(p,0);
				    *xp = atmbuf;
				    debug(F110,"cmdir new *xp",*xp,0);
				}
				free(p);
			    }
			}
		    }
#endif /* VMS */
		    y = (!itsadir) ? 0 : 1;
		    debug(F111,"cmifi y itsadir",*xp,y);
		}
	    } else {			/* Parsing a filename. */
		debug(F110,"cmifi *xp pre-zxpand",*xp,0);
#ifndef COMMENT
		nzxopts |= (d == 0) ? ZX_FILONLY : 0; /* So always expand. */
		if (matchdot)  nzxopts |= ZX_MATCHDOT;
		if (recursive) nzxopts |= ZX_RECURSE;
		y = nzxpand(*xp,nzxopts);
#else
/* Here we're trying to fix a problem in which a directory name is accepted */
/* as a filename, but this breaks too many other things. */
		/* nzxopts = 0; */
		if (!d) {
		    if (itsadir & !iswild(*xp)) {
			debug(F100,"cmifi dir when filonly","",0);
			printf("?Not a regular file: \"%s\"\n",*xp);
			if (sv) free(sv);
			if (np) free(np);
			return(-9);
		    } else {
			nzxopts |= ZX_FILONLY;
			if (matchdot)  nzxopts |= ZX_MATCHDOT;
			if (recursive) nzxopts |= ZX_RECURSE;
			y = nzxpand(*xp,nzxopts);
		    }
		}
#endif /* COMMENT */
		nfiles = y;
		debug(F111,"cmifi y nzxpand",*xp,y);
		debug(F111,"cmifi y atmbuf",atmbuf,itsadir);
		expanded = 1;
	    }
	    /* domydir() calls zxrewind() so we MUST call nzxpand() here */
	    if (!expanded && diractive) {
		debug(F110,"cmifi diractive catch-all zxpand",*xp,0);
		nzxopts |= (d == 0) ? ZX_FILONLY : (dirflg ? ZX_DIRONLY : 0);
		if (matchdot)  nzxopts |= ZX_MATCHDOT;
		if (recursive) nzxopts |= ZX_RECURSE;
		y = nzxpand(*xp,nzxopts);
		debug(F111,"cmifi diractive nzxpand",*xp,y);
		nfiles = y;
		expanded = 1;
	    }
	    *wild = (iswild(sv) || (y > 1)) && (itsadir == 0);

#ifdef RECURSIVE
	    if (!*wild) *wild = recursive;
#endif /* RECURSIVE */

	    debug(F111,"cmifi sv wild",sv,*wild);
	    debug(F101,"cmifi y","",y);
	    if (dirflg && *wild && cdactive) {
		if (y > 1) {
		    printf("?Wildcard matches more than one directory\n");
		    if (sv) free(sv);
		    if (np) free(np);
		    return(-9);
		} else {
		    znext(*xp);
		}
	    }
	    if (itsadir && d && !dirflg) { /* It's a directory and not wild */
		if (sv) free(sv);	/* and it's ok to parse directories */
		if (np) free(np);
#ifndef NOLASTFILE
		makestr(&lastfile,tmplastfile);
#endif	/* NOLASTFILE */
		return(x);
	    }
	    if (y == 0) {		/* File was not found */
		int dosearch = 0;
		dosearch = (path != NULL); /* A search path was given */
		if (dosearch) {
		    dosearch = hasnopath(sv); /* Filename includes no path */
		    debug(F111,"cmifip hasnopath",sv,dosearch);
		}
		if (dosearch) {		/* Search the path... */
		    char * ptr = path;
		    char c;
		    while (1) {
			c = *ptr;
			if (c == PATHSEP || c == NUL) {
			    if (!*path) {
				path = NULL;
				break;
			    }
			    *ptr = NUL;
#ifdef UNIX
/* By definition of CDPATH, an empty member denotes the current directory */
			    if (!*path)
			      ckstrncpy(atmbuf,".",ATMBL);
			    else
#endif /* UNIX */
			      ckstrncpy(atmbuf,path,ATMBL);
#ifdef VMS
			    atmbuf[ATMBL] = NUL;
/* If we have a logical name, evaluate it recursively */
			    if (*(ptr-1) == ':') { /* Logical name ends in : */
				char *p; int n;
				while (((n = strlen(atmbuf))  > 0) &&
				       atmbuf[n-1] == ':') {
				    atmbuf[n-1] = NUL;
				    for (p = atmbuf; *p; p++)
				      if (islower(*p)) *p = toupper(*p);
				    debug(F111,"cmdir CDPATH LN 1",atmbuf,n);
				    p = getenv(atmbuf);
				    debug(F110,"cmdir CDPATH LN 2",p,0);
				    if (!p)
				      break;
				    strncpy(atmbuf,p,ATMBL);
				    atmbuf[ATMBL] = NUL;
				}
			    }
#else
#ifdef OS2
			    if (*(ptr-1) != '\\' && *(ptr-1) != '/')
			      ckstrncat(atmbuf,"\\",ATMBL);
#else
#ifdef UNIX
			    if (*(ptr-1) != '/')
			      ckstrncat(atmbuf,"/",ATMBL);
#else
#ifdef datageneral
			    if (*(ptr-1) != ':')
			      ckstrncat(atmbuf,":",ATMBL);
#endif /* datageneral */
#endif /* UNIX */
#endif /* OS2 */
#endif /* VMS */
			    ckstrncat(atmbuf,sv,ATMBL);
			    debug(F110,"cmifip add path",atmbuf,0);
			    if (c == PATHSEP) ptr++;
			    path = ptr;
			    break;
			}
			ptr++;
		    }
		    x = 1;
		    inword = 0;
		    cc = 0;
		    xc = (int) strlen(atmbuf);
		    *xp = "";
		    goto i_path;
		}
		if (d) {
		    if (sv) free(sv);
		    if (np) free(np);
		    return(-2);
		} else {
		    if (!nomsg) {
#ifdef CKROOT
			if (ckrooterr)
			  printf("?Off Limits: %s\n",sv);
			else
#endif /* CKROOT */
			  if (!quiet)
			    printf("?No %s match - %s\n",
				 dirflg ? "directories" : "files", sv);
		    }
		    if (sv) free(sv);
		    if (np) free(np);
		    return(-9);
		}
	    } else if (y < 0) {
#ifdef CKROOT
		if (ckrooterr)
		  printf("?Off Limits: %s\n",sv);
		else
#endif /* CKROOT */
		  printf("?Too many %s match - %s\n",
			 dirflg ? "directories" : "files", sv);
		if (sv) free(sv);
		if (np) free(np);
		return(-9);
	    } else if (*wild || y > 1) {
		if (sv) free(sv);
		if (np) free(np);
#ifndef NOLASTFILE
		makestr(&lastfile,tmplastfile);
#endif	/* NOLASTFILE */
		return(x);
	    }

	    /* If not wild, see if it exists and is readable. */

	    debug(F111,"cmifi sv not wild",sv,*wild);
	    if (expanded)
	      znext(*xp);		/* Get first (only?) matching file */
	    if (dirflg)			/* Maybe wild and expanded */
	      itsadir = isdir(*xp);	/* so do this again. */
	    filesize = dirflg ? itsadir : zchki(*xp); /* Check accessibility */
	    if (expanded) {
#ifdef ZXREWIND
		nfiles = zxrewind();	/* Rewind so next znext() gets 1st */
#else

		nzxopts |= dirflg ? ZX_DIRONLY : 0;
		if (matchdot)  nzxopts |= ZX_MATCHDOT;
		if (recursive) nzxopts |= ZX_RECURSE;
		nfiles = nzxpand(*xp,nzxopts);
#endif /* ZXREWIND */
	    }
	    debug(F111,"cmifi nfiles",*xp,nfiles);
	    debug(F101,"cmifi filesize","",filesize);
	    free(sv);			/* done with this */
	    sv = NULL;
	    if (dirflg && !filesize) {
		printf("?Not a directory - %s\n",*xp);
#ifdef CKCHANNELIO
		z_error = FX_ACC;
#endif /* CKCHANNELIO */
		return(-9);
	    } else if (filesize == (CK_OFF_T)-3) {
		if (!xcmfdb) {
		    if (diractive)
		      /* Don't show filename if we're not allowed to see it */
		      printf("?Read permission denied\n");
		    else
		      printf("?Read permission denied - %s\n",*xp);
		}
		if (np) free(np);
#ifdef CKCHANNELIO
		z_error = FX_ACC;
#endif /* CKCHANNELIO */
		return(xcmfdb ? -6 : -9);
	    } else if (filesize == (CK_OFF_T)-2) {
		if (!recursive) {
		    if (np) free(np);
		    if (d) {
#ifndef NOLASTFILE
			makestr(&lastfile,tmplastfile);
#endif	/* NOLASTFILE */
			return(0);
		    }
		    if (!xcmfdb)
		      printf("?File not readable - %s\n",*xp);
#ifdef CKCHANNELIO
		    z_error = FX_ACC;
#endif /* CKCHANNELIO */
		    return(xcmfdb ? -6 : -9);
		}
	    } else if (filesize < (CK_OFF_T)0) {
		if (np) free(np);
		if (!nomsg && !xcmfdb)
		  printf("?File not found - %s\n",*xp);
#ifdef CKCHANNELIO
		z_error = FX_FNF;
#endif /* CKCHANNELIO */
		return(xcmfdb ? -6 : -9);
	    }
	    if (np) free(np);
#ifndef NOLASTFILE
	    makestr(&lastfile,tmplastfile);
#endif	/* NOLASTFILE */
	    return(x);

#ifndef MAC
	  case 2:			/* ESC */
	    debug(F101,"cmifi esc, xc","",xc);
	    if (xc == 0) {
		if (*xdef) {
		    printf("%s ",xdef); /* If at beginning of field */
#ifdef GEMDOS
		    fflush(stdout);
#endif /* GEMDOS */
		    inword = cmflgs = 0;
		    addbuf(xdef);	/* Supply default. */
		    if (setatm(xdef,0) < 0) {
			printf("Default name too long\n");
			if (np) free(np);
			return(-9);
		    }
		} else {		/* No default */
		    bleep(BP_WARN);
		}
		break;
	    }
	    if (**xp == '{') {		/* Did user type opening brace... */
		*xp = *xp + 1;
		xc--;
		cc--;
		qflag = '}';
	    } else if (dblquo && **xp == '"') {	/* or doublequote? */
		*xp = *xp + 1;		/* If so ignore it and space past it */
		xc--;
		cc--;
		qflag = '"';
	    }
#ifndef NOSPL
	    if (f) {			/* If a conversion function is given */
#ifdef DOCHKVAR
		char *s = *xp;		/* See if there are any variables in */
		while (*s) {		/* the string and if so, expand it.  */
		    if (chkvar(s)) {
#endif /* DOCHKVAR */
			zq = atxbuf;
			atxn = CMDBL;
			if ((x = (*f)(*xp,&zq,&atxn)) < 0) {
			    if (np) free(np);
			    return(-2);
			}
#ifdef DOCHKVAR
		    /* reduce cc by number of \\ consumed by conversion */
		    /* function (needed for OS/2, where \ is path separator) */
			cc -= (strlen(*xp) - strlen(atxbuf));
#endif /* DOCHKVAR */
			*xp = atxbuf;
			if (!atxbuf[0]) { /* Result empty, use default */
			    *xp = xdef;
			    cc = strlen(xdef);
			}
#ifdef DOCHKVAR
			break;
		    }
		    s++;
		}
#endif /* DOCHKVAR */
	    }
#endif /* NOSPL */

#ifdef DTILDE
	    if (dirflg && *(*xp) == '~') {
		debug(F111,"cmifi tilde_expand A",*xp,cc);
		dirp = tilde_expand(*xp); /* Expand tilde, if any... */
		if (!dirp) dirp = "";
		if (*dirp) {
		    int i, xx;
		    char * sp;
		    xc = cc;		/* Length of ~thing */
		    xx = setatm(dirp,0); /* Copy expansion to atom buffer */
		    debug(F111,"cmifi tilde_expand B",atmbuf,cc);
		    if (xx < 0) {
			printf("Expanded name too long\n");
			if (np) free(np);
			return(-9);
		    }
		    debug(F111,"cmifi tilde_expand xc","",xc);
		    for (i = 0; i < xc; i++) {
			cmdchardel();	/* Back up over ~thing */
			bp--;
		    }
		    xc = cc;		/* How many new ones we just got */
		    sp = atmbuf;
		    printf("%s",sp);	/* Print them */
		    while ((*bp++ = *sp++)) ;	/* Copy to command buffer */
		    bp--;	    	        /* Back up over NUL */
		}
		*xp = atmbuf;
	    }
#endif /* DTILDE */

	    sp = *xp + cc;

#ifdef UNIXOROSK
	    if (!strcmp(atmbuf,"..")) {
		printf(" ");
		ckstrncat(cmdbuf," ",CMDBL);
		cc++;
		bp++;
		*wild = 0;
		*xp = atmbuf;
		break;
	    } else if (!strcmp(atmbuf,".")) {
		bleep(BP_WARN);
		if (np) free(np);
		return(-1);
	    } else {
		/* This patches a glitch when user types "./foo<ESC>" */
		/* in which the next two chars are omitted from the */
		/* expansion.  There should be a better fix, however, */
		/* since there is no problem with "../foo<ESC>". */
		char *p = *xp;
		if (*p == '.' && *(p+1) == '/')
		  cc -= 2;
	    }
#endif /* UNIXOROSK */

#ifdef datageneral
	    *sp++ = '+';		/* Data General AOS wildcard */
#else
	    *sp++ = '*';		/* Others */
#endif /* datageneral */
	    *sp-- = '\0';
#ifdef GEMDOS
	    if (!strchr(*xp, '.'))	/* abde.e -> abcde.e* */
	      strcat(*xp, ".*");	/* abc -> abc*.* */
#endif /* GEMDOS */
	    /* Add wildcard and expand list. */
#ifdef COMMENT
	    /* This kills partial completion when ESC given in path segment */
	    nzxopts |= dirflg ? ZX_DIRONLY : (d ? 0 : ZX_FILONLY);
#else
	    /* nzxopts = 0; */
#endif /* COMMENT */
	    if (matchdot)  nzxopts |= ZX_MATCHDOT;
	    if (recursive) nzxopts |= ZX_RECURSE;
	    y = nzxpand(*xp,nzxopts);
	    nfiles = y;
	    debug(F111,"cmifi nzxpand",*xp,y);
	    if (y > 0) {
#ifdef OS2
                znext(filbuf);		/* Get first */
#ifdef ZXREWIND
		zxrewind();		/* Must "rewind" */
#else
		nzxpand(*xp,nxzopts);
#endif /* ZXREWIND */
#else  /* Not OS2 */
                ckstrncpy(filbuf,mtchs[0],CKMAXPATH);
#endif /* OS2 */
	    } else
	      *filbuf = '\0';
	    filbuf[CKMAXPATH] = NUL;
	    *sp = '\0';			/* Remove wildcard. */
	    debug(F111,"cmifi filbuf",filbuf,y);
	    debug(F111,"cmifi *xp",*xp,cc);

	    *wild = (y > 1);
	    if (y == 0) {
		if (!nomsg) {
#ifdef CKROOT
		    if (ckrooterr)
		      printf("?Off Limits: %s\n",atmbuf);
		    else
#endif /* CKROOT */
		      printf("?No %s match - %s\n",
			   dirflg ? "directories" : "files", atmbuf);
		    if (np) free(np);
		    return(-9);
		} else {
		    bleep(BP_WARN);
		    if (np) free(np);
		    return(-1);
		}
	    } else if (y < 0) {
#ifdef CKROOT
		if (ckrooterr)
		  printf("?Off Limits: %s\n",atmbuf);
		else
#endif /* CKROOT */
		  printf("?Too many %s match - %s\n",
			 dirflg ? "directories" : "files", atmbuf);
		if (np) free(np);
		return(-9);
	    } else if (y > 1		/* Not unique */
#ifndef VMS
		       || (y == 1 && isdir(filbuf)) /* Unique directory */
#endif /* VMS */
		       ) {
#ifndef NOPARTIAL
/* Partial filename completion */
		int j, k; char c;
		k = 0;
		debug(F111,"cmifi partial",filbuf,cc);
#ifdef OS2
		{
		    int cur = 0,
		    len = 0,
		    len2 = 0,
		    min = strlen(filbuf),
		    found = 0;
		    char localfn[CKMAXPATH+1];

		    len = min;
		    for (j = 1; j <= y; j++) {
			znext(localfn);
			if (dirflg && !isdir(localfn))
			  continue;
			found = 1;
			len2 = strlen(localfn);
			for (cur = cc;
			     cur < len && cur < len2 && cur <= min;
			     cur++
			     ) {
                            /* OS/2 or Windows, case doesn't matter */
			    if (tolower(filbuf[cur]) != tolower(localfn[cur]))
			      break;
			}
			if (cur < min)
			  min = cur;
		    }
		    if (!found)
		      min = cc;
		    filbuf[min] = NUL;
		    if (min > cc)
		      k++;
		}
#else /* OS2 */
		for (i = cc; (c = filbuf[i]); i++) {
		    for (j = 1; j < y; j++)
		      if (mtchs[j][i] != c) break;
		    if (j == y) k++;
		    else filbuf[i] = filbuf[i+1] = NUL;
		}
#endif /* OS2 */


#ifndef VMS
		/* isdir() function required for this! */
		if (y == 1 && isdir(filbuf)) { /* Dont we already know this? */
		    int len;
		    len = strlen(filbuf);
		    if (len > 0 && len < ATMBL - 1) {
			if (filbuf[len-1] != dirsep) {
			    filbuf[len] = dirsep;
			    filbuf[len+1] = NUL;
			}
		    }
/*
  At this point, before just doing partial completion, we should look first to
  see if the given directory does indeed have any subdirectories (dirflg) or
  files (!dirflg); if it doesn't we should do full completion.  Otherwise, the
  result looks funny to the user and "?" blows up the command for no good
  reason.
*/
		    {
			int flags = 0;
			filbuf[len+1] = '*';
			filbuf[len+2] = NUL;
			if (dirflg) flags = ZX_DIRONLY;
			children = nzxpand(filbuf,flags);
			debug(F111,"cmifi children",filbuf,children);
			filbuf[len+1] = NUL;
			nzxpand(filbuf,flags); /* Restore previous list */
			if (children == 0)
			  goto NOSUBDIRS;
		    }
		    if (len + 1 > cc)
		      k++;
		}
                /* Add doublequotes if there are spaces in the name */
		{
		    int x;
		    if (qflag) {
			x = (qflag == '}'); /* (or braces) */
		    } else {
			x = !dblquo;
		    }
		    if (filbuf[0] != '"' && filbuf[0] != '{')
		      k = dquote(filbuf,ATMBL,x);
		}
#endif /* VMS */
		debug(F111,"cmifi REPAINT filbuf",filbuf,k);
		if (k > 0) {		/* Got more characters */
		    debug(F101,"cmifi REPAINT cc","",cc);
		    debug(F101,"cmifi REPAINT xc","",xc);
		    debug(F110,"cmifi REPAINT bp-cc",bp-cc,0);
		    debug(F110,"cmifi REPAINT bp-xc",bp-xc,0);
		    sp = filbuf + cc;	/* Point to new ones */
		    if (qflag || strncmp(filbuf,bp-cc,cc)) { /* Repaint? */
			int x;
			x = cc;
			if (qflag) x++;
			for (i = 0; i < x; i++) {
			    cmdchardel(); /* Back up over old partial spec */
			    bp--;
			}
			sp = filbuf;	/* Point to new word start */
			debug(F110,"cmifi erase ok",sp,0);
		    }
		    cc = k;		/* How many new ones we just got */
		    printf("%s",sp);	/* Print them */
		    while ((*bp++ = *sp++)) ;	/* Copy to command buffer */
		    bp--;	    	        /* Back up over NUL */
		    debug(F110,"cmifi partial cmdbuf",cmdbuf,0);
		    if (setatm(filbuf,0) < 0) {
			printf("?Partial name too long\n");
			if (np) free(np);
			return(-9);
		    }
		    debug(F111,"cmifi partial atmbuf",atmbuf,cc);
		    *xp = atmbuf;
		}
#endif /* NOPARTIAL */
		bleep(BP_WARN);
	    } else {			/* Unique, complete it.  */
#ifndef VMS
#ifdef CK_TMPDIR
		/* isdir() function required for this! */
	      NOSUBDIRS:
		debug(F111,"cmifi unique",filbuf,children);
		if (isdir(filbuf) && children > 0) {
		    int len;
		    len = strlen(filbuf);
		    if (len > 0 && len < ATMBL - 1) {
			if (filbuf[len-1] != dirsep) {
			    filbuf[len] = dirsep;
			    filbuf[len+1] = NUL;
			}
		    }
		    sp = filbuf + cc;
		    bleep(BP_WARN);
		    printf("%s",sp);
		    cc++;
		    while ((*bp++ = *sp++)) ;
		    bp--;
		    if (setatm(filbuf,0) < 0) {
			printf("?Directory name too long\n");
			if (np) free(np);
			return(-9);
		    }
		    debug(F111,"cmifi directory atmbuf",atmbuf,cc);
		    *xp = atmbuf;
		} else {		/* Not a directory or dirflg */
#endif /* CK_TMPDIR */
#endif /* VMS */
#ifndef VMS				/* VMS dir names are special */
#ifndef datageneral			/* VS dirnames must not end in ":" */
		    if (dirflg) {
			int len;
			len = strlen(filbuf);
			if (len > 0 && len < ATMBL - 1) {
			    if (filbuf[len-1] != dirsep) {
				filbuf[len] = dirsep;
				filbuf[len+1] = NUL;
			    }
			}
		    }
#endif /* datageneral */
#endif /* VMS */
		    sp = filbuf + cc;	/* Point past what user typed. */
		    {
			int x;
			if (qflag) {
			    x = (qflag == '}');
			} else {
			    x = !dblquo;
			}
			if (filbuf[0] != '"' && filbuf[0] != '{')
			  dquote(filbuf,ATMBL,x);
		    }
		    if (qflag || strncmp(filbuf,bp-cc,cc)) { /* Repaint? */
			int x;
			x = cc;
			if (qflag) x++;
			for (i = 0; i < x; i++) {
			    cmdchardel(); /* Back up over old partial spec */
			    bp--;
			}
			sp = filbuf;	/* Point to new word start */
			debug(F111,"cmifi after erase sp=",sp,cc);
		    }
		    printf("%s ",sp);	/* Print the completed name. */
#ifdef GEMDOS
		    fflush(stdout);
#endif /* GEMDOS */
		    addbuf(sp);		/* Add the characters to cmdbuf. */
		    if (setatm(filbuf,0) < 0) { /* And to atmbuf. */
			printf("?Completed name too long\n");
			if (np) free(np);
			return(-9);
		    }
		    inword = cmflgs = 0;
		    *xp = brstrip(atmbuf); /* Return pointer to atmbuf. */
		    if (dirflg && !isdir(*xp)) {
			printf("?Not a directory - %s\n", filbuf);
			if (np) free(np);
			return(-9);
		    }
		    if (np) free(np);
#ifndef NOLASTFILE
		    makestr(&lastfile,tmplastfile);
#endif	/* NOLASTFILE */
		    return(0);
#ifndef VMS
#ifdef CK_TMPDIR
		}
#endif /* CK_TMPDIR */
#endif /* VMS */
	    }
	    break;

	  case 3:			/* Question mark - file menu wanted */
	    if (*xhlp == NUL)
	      printf(dirflg ? " Directory name" : " Input file specification");
	    else
	      printf(" %s",xhlp);
#ifdef GEMDOS
	    fflush(stdout);
#endif /* GEMDOS */
	    /* If user typed an opening quote or brace, just skip past it */

	    if (**xp == '"' || **xp == '{') {
		*xp = *xp + 1;
		xc--;
		cc--;
	    }
#ifndef NOSPL
	    if (f) {			/* If a conversion function is given */
#ifdef DOCHKVAR
		char *s = *xp;		/* See if there are any variables in */
		while (*s) {		/* the string and if so, expand them */
		    if (chkvar(s)) {
#endif /* DOCHKVAR */
			zq = atxbuf;
			atxn = CMDBL;
			if ((x = (*f)(*xp,&zq,&atxn)) < 0) {
			    if (np) free(np);
			    return(-2);
			}
#ifdef DOCHKVAR
		    /* reduce cc by number of \\ consumed by conversion */
		    /* function (needed for OS/2, where \ is path separator) */
			cc -= (strlen(*xp) - strlen(atxbuf));
#endif /* DOCHKVAR */
			*xp = atxbuf;
#ifdef DOCHKVAR
			break;
		    }
		    s++;
		}
#endif /* DOCHKVAR */
	    }
#endif /* NOSPL */
	    debug(F111,"cmifi ? *xp, cc",*xp,cc);
	    sp = *xp + cc;		/* Insert "*" at end */
#ifdef datageneral
	    *sp++ = '+';		/* Insert +, the DG wild card */
#else
	    *sp++ = '*';
#endif /* datageneral */
	    *sp-- = '\0';
#ifdef GEMDOS
	    if (! strchr(*xp, '.'))	/* abde.e -> abcde.e* */
	      strcat(*xp, ".*");	/* abc -> abc*.* */
#endif /* GEMDOS */
	    debug(F110,"cmifi ? wild",*xp,0);

	    nzxopts |= dirflg ? ZX_DIRONLY : (d ? 0 : ZX_FILONLY);

	    debug(F101,"cmifi matchdot","",matchdot);
	    if (matchdot)  nzxopts |= ZX_MATCHDOT;
	    if (recursive) nzxopts |= ZX_RECURSE;
	    y = nzxpand(*xp,nzxopts);
	    nfiles = y;
	    *sp = '\0';
	    if (y == 0) {
		if (nomsg) {
		    printf(": %s\n",atmbuf);
		    printf("%s%s",cmprom,cmdbuf);
		    fflush(stdout);
		    if (np) free(np);
		    return(-1);
		} else {
#ifdef CKROOT
		    if (ckrooterr)
		      printf("?Off Limits: %s\n",atmbuf);
		    else
#endif /* CKROOT */
		      printf("?No %s match - %s\n",
			     dirflg ? "directories" : "files", atmbuf);
		    if (np) free(np);
		    return(-9);
		}
	    } else if (y < 0) {
#ifdef CKROOT
		if (ckrooterr)
		  printf("?Off Limits: %s\n",atmbuf);
		else
#endif /* CKROOT */
		  printf("?Too many %s match - %s\n",
			 dirflg ? "directories" : "files", atmbuf);
		if (np) free(np);
		return(-9);
	    } else {
		printf(", one of the following:\n");
		if (filhelp((int)y,"","",1,dirflg) < 0) {
		    if (np) free(np);
		    return(-9);
		}
	    }
	    printf("%s%s",cmprom,cmdbuf);
	    fflush(stdout);
	    break;
#endif /* MAC */
	}
#ifdef BS_DIRSEP
        dirnamflg = 1;
        x = gtword(0);                  /* No, get a word */
	dirnamflg = 0;
#else
        x = gtword(0);                  /* No, get a word */
#endif /* BS_DIRSEP */
	*xp = atmbuf;
    }
}

/*  C M F L D  --  Parse an arbitrary field  */
/*
  Returns:
    -3 if no input present when required,
    -2 if field too big for buffer,
    -1 if reparse needed,
     0 otherwise, xp pointing to string result.

  NOTE: Global flag keepallchars says whether this routine should break on CR
  or LF: needed for MINPUT targets and DECLARE initializers, where we want to
  keep control characters if the user specifies them (March 2003).  It might
  have been better to change the calling sequence but that was not practical.
*/
int
cmfld(xhlp,xdef,xp,f) char *xhlp, *xdef, **xp; xx_strp f; {
    int x, xc;
    char *zq;

    inword = 0;				/* Initialize counts & pointers */
    cc = 0;
    xc = 0;
    *xp = "";

    debug(F110,"cmfld xdef 1",xdef,0);

    if (!xhlp) xhlp = "";
    if (!xdef) xdef = "";
    ckstrncpy(cmdefault,xdef,CMDEFAULT); /* Copy default */
    xdef = cmdefault;

    debug(F111,"cmfld xdef 2",xdef,cmflgs);
    debug(F111,"cmfld atmbuf 1",atmbuf,xc);

    if ((x = cmflgs) != 1) {            /* Already confirmed? */
        x = gtword(0);                  /* No, get a word */
    } else {
	if (setatm(xdef,0) < 0) {	/* If so, use default, if any. */
	    printf("?Default too long\n");
	    return(-9);
	}
    }
    *xp = atmbuf;                       /* Point to result. */
    debug(F111,"cmfld atmbuf 2",atmbuf,cmflgs);

    while (1) {
        xc += cc;                       /* Count the characters. */
        debug(F111,"cmfld gtword",atmbuf,xc);
        debug(F101,"cmfld x","",x);
        switch (x) {
	  case -9:
	    printf("Command or field too long\n");
	  case -4:			/* EOF */
	  case -3:			/* Empty. */
	  case -2:			/* Out of space. */
	  case -1:			/* Reparse needed */
	    return(x);
	  case 1:			/* CR */
	  case 0:			/* SP */
	    debug(F111,"cmfld 1",atmbuf,xc);
	    if (xc == 0) {		/* If no input, return default. */
		if (setatm(xdef,0) < 0) {
		    printf("?Default too long\n");
		    return(-9);
		}
	    }
	    *xp = atmbuf;		/* Point to what we got. */
	    debug(F111,"cmfld 2",atmbuf,((f) ? 1 : 0));
	    if (f) {			/* If a conversion function is given */
		zq = atxbuf;		/* employ it now. */
		atxn = CMDBL;
		if ((*f)(*xp,&zq,&atxn) < 0)
		  return(-2);
		debug(F111,"cmfld 3",atxbuf,xc);
		/* Replace by new value -- for MINPUT only keep all chars */
		if (setatm(atxbuf,keepallchars ? 3:1) < 0) { /* 16 Mar 2003 */
		    printf("Value too long\n");
		    return(-9);
		}
		*xp = atmbuf;
	    }
	    debug(F111,"cmfld 4",atmbuf,xc);
	    if (**xp == NUL) {		/* If variable evaluates to null */
		if (setatm(xdef,0) < 0) {
		    printf("?Default too long\n");
		    return(-9);
		}
		if (**xp == NUL) x = -3; /* If still empty, return -3. */
	    }
	    debug(F111,"cmfld returns",*xp,x);
	    return(x);
	  case 2:			/* ESC */
	    if (xc == 0 && *xdef) {
		printf("%s ",xdef); /* If at beginning of field, */
#ifdef GEMDOS
		fflush(stdout);
#endif /* GEMDOS */
		addbuf(xdef);		/* Supply default. */
		inword = cmflgs = 0;
		if (setatm(xdef,0) < 0) {
		    printf("?Default too long\n");
		    return(-9);
		} else			/* Return as if whole field */
		  return(0);		/* typed, followed by space. */
	    } else {
		bleep(BP_WARN);
	    }
	    break;
	  case 3:			/* Question mark */
	    debug(F110,"cmfld QUESTIONMARK",cmdbuf,0);
	    if (*xhlp == NUL)
	      printf(" Please complete this field");
	    else
	      printf(" %s",xhlp);
	    printf("\n%s%s",cmprom,cmdbuf);
	    fflush(stdout);
	    break;
        }
	debug(F111,"cmfld gtword A x",cmdbuf,x);
	x = gtword(0);
	debug(F111,"cmfld gtword B x",cmdbuf,x);
    }
}


/*  C M T X T  --  Get a text string, including confirmation  */

/*
  Print help message 'xhlp' if ? typed, supply default 'xdef' if null
  string typed.  Returns:

   -1 if reparse needed or buffer overflows.
    1 otherwise.

  with cmflgs set to return code, and xp pointing to result string.
*/
int
cmtxt(xhlp,xdef,xp,f) char *xhlp; char *xdef; char **xp; xx_strp f; {

    int x, i;
    char *xx, *zq;
    static int xc;

    if (!xhlp) xhlp = "";
    if (!xdef) xdef = "";

    cmfldflgs = 0;

    cmdefault[0] = NUL;
    if (*xdef)
      ckstrncpy(cmdefault,xdef,CMDEFAULT); /* Copy default */
    xdef = cmdefault;

    debug(F101,"cmtxt cmflgs","",cmflgs);
    inword = 0;				/* Start atmbuf counter off at 0 */
    cc = 0;
    if (cmflgs == -1) {                 /* If reparsing, */
	*xp = pp;
        xc = (int)strlen(*xp);		/* get back the total text length, */
	bp = *xp;			/* and back up the pointers. */
	np = *xp;
	pp = *xp;
    } else {                            /* otherwise, */
	/* debug(F100,"cmtxt: fresh start","",0); */
        *xp = "";                       /* start fresh. */
        xc = 0;
    }
    *atmbuf = NUL;                      /* And empty the atom buffer. */
    rtimer();				/* Reset timer */
    if ((x = cmflgs) != 1) {
	int done = 0;
	while (!done) {
	    x = gtword(0);		/* Get first word. */
	    *xp = pp;			/* Save pointer to it. */
	    /* debug(F111,"cmtxt:",*xp,cc); */
	    if (x == -10) {
		if (gtimer() > timelimit) {
		    /* if (!quiet) printf("?Timed out\n"); */
		    return(x);
		}
	    } else
	      done = 1;
	}
    }
    while (1) {				/* Loop for each word in text. */
        xc += cc;                       /* Char count for all words. */
        /* debug(F111,"cmtxt gtword",atmbuf,xc); */
        /* debug(F101,"cmtxt x","",x); */
        switch (x) {
	  case -10:
	    if (gtimer() > timelimit) {
#ifdef IKSD
                extern int inserver;
                if (inserver) {
                    printf("\r\nIKSD IDLE TIMEOUT: %d sec\r\n", timelimit);
                    doexit(GOOD_EXIT,0);
                }
#endif /* IKSD */
		/* if (!quiet) printf("?Timed out\n"); */
		return(-10);
	    } else {
		x = gtword(0);
		continue;
	    }
	  case -9:			/* Buffer overflow */
	    printf("Command or field too long\n");
	  case -4:			/* EOF */
#ifdef MAC
	  case -3:			/* Quit/Timeout */
#endif /* MAC */
	  case -2:			/* Overflow */
	  case -1:			/* Deletion */
	    return(x);
	  case 0:			/* Space */
	    xc++;			/* Just count it */
	    break;
	  case 1:			/* CR or LF */
	    if (xc == 0) *xp = xdef;
	    if (f) {			/* If a conversion function is given */
		char * sx = atxbuf;
		zq = atxbuf;		/* Point to the expansion buffer */
		atxn = CMDBL;		/* specify its length */
		/* debug(F111,"cmtxt calling (*f)",*xp,atxbuf); */
		if ((x = (*f)(*xp,&zq,&atxn)) < 0) return(-2);
		sx = atxbuf;
#ifndef COMMENT
		cc = 0;
		while (*sx++) cc++;	/* (faster than calling strlen) */
#else
		cc = (int)strlen(atxbuf);
#endif /* COMMENT */
		/* Should be equal to (CMDBL - atxn) but isn't always. */
		/* Why not? */
		if (cc < 1) {		/* Nothing in expansion buffer? */
		    *xp = xdef;		/* Point to default string instead. */
#ifndef COMMENT
		    sx = xdef;
		    while (*sx++) cc++;	/* (faster than calling strlen) */
#else
		    cc = strlen(xdef);
#endif /* COMMENT */
		} else {		/* Expansion function got something */
		    *xp = atxbuf;	/* return pointer to it. */
		}
		debug(F111,"cmtxt (*f)",*xp,cc);
	    } else {			/* No expansion function */
#ifndef COMMENT
		/* Avoid a strlen() call */
		xx = *xp;
		cc = 0;
		while (*xx++) cc++;
#else
		/* NO!  xc is apparently not always set appropriately */
		cc = xc;
#endif /* COMMENT */
	    }
	    xx = *xp;
#ifdef COMMENT
	    /* strlen() no longer needed */
	    for (i = (int)strlen(xx) - 1; i > 0; i--)
#else
	    for (i = cc - 1; i > 0; i--)
#endif /* COMMENT */
	      if (xx[i] != SP)		/* Trim trailing blanks */
		break;
	      else
		xx[i] = NUL;
	    return(x);
	  case 2:			/* ESC */
	    if (xc == 0) {		/* Nothing typed yet */
		if (*xdef) {		/* Have a default for this field? */
		    printf("%s ",xdef);	/* Yes, supply it */
		    inword = cmflgs = 0;
#ifdef GEMDOS
		    fflush(stdout);
#endif /* GEMDOS */
		    cc = addbuf(xdef);
		} else bleep(BP_WARN);	/* No default */
	    } else {			/* Already in field */
		int x; char *p;
		x = strlen(atmbuf);
		if (ckstrcmp(atmbuf,xdef,x,0)) {    /* Matches default? */
		    bleep(BP_WARN);	            /* No */
		} else if ((int)strlen(xdef) > x) { /* Yes */
		    p = xdef + x;
		    printf("%s ", p);
#ifdef GEMDOS
		    fflush(stdout);
#endif /* GEMDOS */
		    addbuf(p);
		    inword = cmflgs = 0;
		    debug(F110,"cmtxt: addbuf",cmdbuf,0);
		} else {
		    bleep(BP_WARN);
		}
	    }
	    break;
	  case 3:			/* Question Mark */
	    if (*xhlp == NUL)
	      printf(" Text string");
	    else
	      printf(" %s",xhlp);
	    printf("\n%s%s",cmprom,cmdbuf);
	    fflush(stdout);
	    break;
	  default:
	    printf("?Unexpected return code from gtword() - %d\n",x);
	    return(-2);
        }
        x = gtword(0);
    }
}

/*  C M K E Y  --  Parse a keyword  */

/*
 Call with:
   table    --  keyword table, in 'struct keytab' format;
   n        --  number of entries in table;
   xhlp     --  pointer to help string;
   xdef     --  pointer to default keyword;
   f        --  string preprocessing function (e.g. to evaluate variables)
   pmsg     --  0 = don't print error messages
                1 = print error messages
                2 = include CM_HLP keywords even if invisible
                3 = 1+2
                4 = parse a switch (keyword possibly ending in : or =)
                8 = don't strip comments (used, e.g., for "help #")
 Returns:
   -3       --  no input supplied and no default available
   -2       --  input doesn't uniquely match a keyword in the table
   -1       --  user deleted too much, command reparse required
    n >= 0  --  value associated with keyword
*/

/*
  Front ends for cmkey2(): 
  cmkey()  - The normal keyword parser
  cmkeyx() - Like cmkey() but suppresses error messages
  cmswi()  - Switch parser
*/
int
cmkey(table,n,xhlp,xdef,f)
/* cmkey */  struct keytab table[]; int n; char *xhlp, *xdef; xx_strp f; {
    return(cmkey2(table,n,xhlp,xdef,"",f,1));
}
int
cmkeyx(table,n,xhlp,xdef,f)
/* cmkeyx */  struct keytab table[]; int n; char *xhlp, *xdef; xx_strp f; {
    return(cmkey2(table,n,xhlp,xdef,"",f,0));
}
int
cmswi(table,n,xhlp,xdef,f)
/* cmswi */  struct keytab table[]; int n; char *xhlp, *xdef; xx_strp f; {
    return(cmkey2(table,n,xhlp,xdef,"",f,4));
}

int
cmkey2(table,n,xhlp,xdef,tok,f,pmsg)
    struct keytab table[];
    int n;
    char *xhlp, *xdef;
    char *tok;
    xx_strp f;
    int pmsg;
{ /* cmkey2 */
    extern int havetoken;
    int i, tl, y, z = 0, zz, xc, wordlen = 0, cmswitch;
    char *xp, *zq;

    if (!xhlp) xhlp = "";
    if (!xdef) xdef = "";

    cmfldflgs = 0;
    if (!table) {
	printf("?Keyword table missing\n");
	return(-9);
    }
    tl = (int)strlen(tok);

    inword = xc = cc = 0;		/* Clear character counters. */
    cmswitch = pmsg & 4;		/* Flag for parsing a switch */

    debug(F101,"cmkey: pmsg","",pmsg);
    debug(F101,"cmkey: cmflgs","",cmflgs);
    debug(F101,"cmkey: cmswitch","",cmswitch);
    /* debug(F101,"cmkey: cmdbuf","",cmdbuf);*/

    ppvnambuf[0] = NUL;

    if ((zz = cmflgs) == 1) {		/* Command already entered? */
	if (setatm(xdef,0) < 0) {	/* Yes, copy default into atom buf */
	    printf("?Default too long\n");
	    return(-9);
	}
        rtimer();			 /* Reset timer */
    } else {				 /* Otherwise get a command word */
        rtimer();			 /* Reset timer */
	if (pmsg & 8)			 /* 8 is for parsing HELP tokens */
	  zz = gtword(4);
	else
	  zz = gtword((pmsg == 4) ? 1 : 0);
    }

    debug(F101,"cmkey table length","",n);
    debug(F101,"cmkey cmflgs","",cmflgs);
    debug(F101,"cmkey cc","",cc);

    while (1) {
	xc += cc;
	debug(F111,"cmkey gtword xc",atmbuf,xc);
	debug(F101,"cmkey gtword zz","",zz);

	switch (zz) {
	  case -10:			/* Timeout */
	    if (gtimer() < timelimit) {
		if (pmsg & 8)		/* 8 is for parsing HELP tokens */
		  zz = gtword(4);
		else
		  zz = gtword((pmsg == 4) ? 1 : 0);
		continue;
	    } else {
#ifdef IKSD
                extern int inserver;
                if (inserver) {
                    printf("\r\nIKSD IDLE TIMEOUT: %d sec\r\n", timelimit);
                    doexit(GOOD_EXIT,0);
                }
#endif /* IKSD */
		return(-10);
            }
	  case -5:
	    return(cmflgs = 0);
	  case -9:
	    printf("Command or field too long\n");
	  case -4:			/* EOF */
	  case -3:			/* Null Command/Quit/Timeout */
	  case -2:			/* Buffer overflow */
	  case -1:			/* Or user did some deleting. */
	    return(cmflgs = zz);


	  case 1:			/* CR */
	  case 0:			/* User terminated word with space */
	  case 4:			/* or switch ending in : or = */
	    wordlen = cc;		/* Length if no conversion */
	    if (cc == 0) {		/* Supply default if we got nothing */
		if ((wordlen = setatm(xdef,(zz == 4) ? 2 : 0)) < 0) {
		    printf("?Default too long\n");
		    return(-9);
		}
	    }
	    if (zz == 1 && cc == 0)	/* Required field missing */
	      return(-3);

	    if (f) {			/* If a conversion function is given */
		char * p2;
		zq = atxbuf;		/* apply it */
		p2 = atxbuf;
		atxn = CMDBL;
		if ((*f)(atmbuf,&zq,&atxn) < 0) return(-2);
		debug(F110,"cmkey atxbuf after *f",atxbuf,0);
		if (!*p2)		/* Supply default if we got nothing */
		  p2 = xdef;
		ckstrncpy(ppvnambuf,atmbuf,PPVLEN);
		if ((wordlen = setatm(p2,(zz == 4) ? 2 : 0)) < 0) {
		    printf("Evaluated keyword too long\n");
		    return(-9);
		}
#ifdef M_UNGW
		/*
		  This bit lets us save more than one "word".
		  For example, "define \%x echo one two three", "\%x".
		  It works too, but it breaks labels, and therefore
		  WHILE and FOR loops, etc.
		*/
		if (p2[wordlen] >= SP) {
		    p2 += wordlen;
		    while (*p2 == SP) p2++;
		    if (*p2) {
			ungword();
			pp = p2;
		    }
		}
#endif /* M_UNGW */
	    }
#ifdef COMMENT				/* ^^^ */
	    if (cmswitch && *atmbuf != '/') {
		if (pmsg & 1) {
		    bleep(BP_FAIL);
                    printf("?Not a switch - %s\n",atmbuf);
		}
		cmflgs = -2;
		return(-6);
	    }
#endif	/* COMMENT */
	    if (cmswitch) {
		int i;
		for (i = 0; i < wordlen; i++) {
		    if (atmbuf[i] == ':' || atmbuf[i] == '=') {
			brkchar = atmbuf[i];
			atmbuf[i] = NUL;
			break;
		    }
		}
	    }

#ifdef TOKPRECHECK
/* This was an effective optimization but it breaks sometimes on labels. */
	    if (tl && !isalpha(atmbuf[0])) { /* Precheck for token */
		for (i = 0; i < tl; i++) { /* Save function call to ckstrchr */
		    if (tok[i] == atmbuf[0]) {
			debug(F000,"cmkey token:",atmbuf,*atmbuf);
			ungword();  /* Put back the following word */
			return(-5); /* Special return code for token */
		    }
		}
	    }
#endif /* TOKPRECHECK */

	    y = lookup(table,atmbuf,n,&z); /* Look up word in the table */
	    debug(F111,"cmkey lookup",atmbuf,y);
	    debug(F101,"cmkey zz","",zz);
	    debug(F101,"cmkey cmflgs","",cmflgs);
	    debug(F101,"cmkey crflag","",crflag);
	    switch (y) {
	      case -3:			/* Nothing to look up */
		break;
	      case -2:			/* Ambiguous */
		cmflgs = -2;
		if (pmsg & 1) {
		    bleep(BP_FAIL);
                    printf("?Ambiguous - %s\n",atmbuf);
		    return(-9);
		}
		return(-2);
	      case -1:			/* Not found at all */
#ifndef TOKPRECHECK
		if (tl) {
		    for (i = 0; i < tl; i++) /* Check for token */
		      if (tok[i] == *atmbuf) { /* Got one */
			  debug(F000,"cmkey token:",atmbuf,*atmbuf);
			  ungword();  /* Put back the following word */
			  return(-5); /* Special return code for token */
		      }
		}
#endif /* TOKPRECHECK */

		if (tl == 0) {		/* No tokens were included */
#ifdef OS2
		    /* In OS/2 and Windows, allow for a disk letter like DOS */
		    if (isalpha(*atmbuf) && *(atmbuf+1) == ':')
		      return(-7);
#endif /* OS2 */
		    if ((pmsg & 1) && !quiet) {
			bleep(BP_FAIL);
			printf("?No keywords match - %s\n",atmbuf); /* cmkey */
		    }
		    return(cmflgs = -9);
		} else {
		    if (cmflgs == 1 || cmswitch) /* cmkey2 or cmswi */
		      return(cmflgs = -6);
		    else
		      return(cmflgs = -2);
		    /* The -6 code is to let caller try another table */
		}
		break;
	      default:
#ifdef CK_RECALL
		if (test(table[z].flgs,CM_NOR)) no_recall = 1;
#endif /* CK_RECALL */
		if (zz == 4)
		  swarg = 1;
		cmkwflgs = table[z].flgs;
		break;
	    }
	    return(y);

	  case 2:			/* User terminated word with ESC */
	    debug(F101,"cmkey Esc cc","",cc);
            if (cc == 0) {
		if (*xdef != NUL) {     /* Nothing in atmbuf */
		    printf("%s ",xdef); /* Supply default if any */
#ifdef GEMDOS
		    fflush(stdout);
#endif /* GEMDOS */
		    addbuf(xdef);
		    if (setatm(xdef,0) < 0) {
			printf("?Default too long\n");
			return(-9);
		    }
		    inword = cmflgs = 0;
		    debug(F111,"cmkey: default",atmbuf,cc);
		} else {
		    debug(F101,"cmkey Esc pmsg","",0);
#ifdef COMMENT
/*
  Chained FDBs...  The idea is that this function might not have a default,
  but the next one might.  But if it doesn't, there is no way to come back to
  this one.  To be revisited later...
*/
		    if (xcmfdb)		/* Chained fdb -- try next one */
		      return(-3);
#endif /* COMMENT */
		    if (pmsg & (1|4)) {	/* So for now just beep */
			bleep(BP_WARN);
		    }
		    break;
		}
            }
	    if (f) {			/* If a conversion function is given */
		char * pp;
		zq = atxbuf;		/* apply it */
		pp = atxbuf;
		atxn = CMDBL;
		if ((*f)(atmbuf,&zq,&atxn) < 0)
		  return(-2);
		if (!*pp)
		  pp = xdef;
		if (setatm(pp,0) < 0) {
		    printf("Evaluated keyword too long\n");
		    return(-9);
		}
	    }
	    y = lookup(table,atmbuf,n,&z); /* Something in atmbuf */
	    debug(F111,"cmkey lookup y",atmbuf,y);
	    debug(F111,"cmkey lookup z",atmbuf,z);
	    if (y == -2 && z >= 0 && z < n) { /* Ambiguous */
#ifndef NOPARTIAL
		int j, k, len = 9999;	/* Do partial completion */
		/* Skip past any abbreviations in the table */
		for ( ; z < n; z++) {
		    if ((table[z].flgs & CM_ABR) == 0)
		      break;
		    if (!(table[z].flgs & CM_HLP) || (pmsg & 2))
		      break;
		}
		debug(F111,"cmkey partial z",atmbuf,z);
		debug(F111,"cmkey partial n",atmbuf,n);
		for (j = z+1; j < n; j++) {
		    debug(F111,"cmkey partial j",table[j].kwd,j);
		    if (ckstrcmp(atmbuf,table[j].kwd,cc,0))
		      break;
		    if (table[j].flgs & CM_ABR)
		      continue;
		    if ((table[j].flgs & CM_HLP) && !(pmsg & 2))
		      continue;
		    k = ckstrpre(table[z].kwd,table[j].kwd);
		    debug(F111,"cmkey partial k",table[z].kwd,k);
		    if (k < len)
		      len = k; /* Length of longest common prefix */
		}
		debug(F111,"cmkey partial len",table[z].kwd,len);
		if (len != 9999 && len > cc) {
		    ckstrncat(atmbuf,table[z].kwd+cc,ATMBL);
		    atmbuf[len] = NUL;
		    printf("%s",atmbuf+cc);
		    ckstrncat(cmdbuf,atmbuf+cc,CMDBL);
		    xc += (len - cc);
		    cc = len;
		}
#endif /* NOPARTIAL */
		bleep(BP_WARN);
		break;
	    } else if (y == -3) {
		bleep(BP_WARN);
		break;
	    } else if (y == -1) {	/* Not found */
		if ((pmsg & 1) && !quiet) {
		    bleep(BP_FAIL);
		    printf("?No keywords match - \"%s\"\n",atmbuf);
		}
		cmflgs = -2;
		return(-9);
	    }
/*
  If we found it, but it's a help-only keyword and the "help" bit is not
  set in pmsg, then not found.
*/
	    debug(F101,"cmkey flgs","",table[z].flgs);
	    if (test(table[z].flgs,CM_HLP) && ((pmsg & 2) == 0)) {
		if ((pmsg & 1) && !quiet) {
		    bleep(BP_FAIL);
		    printf("?No keywords match - %s\n",atmbuf);
		}
		cmflgs = -2;
		return(-9);
	    }
/*
  See if the keyword just found has the CM_ABR bit set in its flgs field, and
  if so, search forwards in the table for a keyword that has the same kwval
  but does not have CM_ABR (or CM_INV?) set, and then expand using the full
  keyword.  WARNING: This assumes that (a) keywords are in alphabetical order,
  and (b) the CM_ABR bit is set only if the the abbreviated keyword is a true
  abbreviation (left substring) of the full keyword.
*/
	    if (test(table[z].flgs,CM_ABR)) {
		int zz;
		for (zz = z+1; zz < n; zz++)
		  if ((table[zz].kwval == table[z].kwval) &&
		      (!test(table[zz].flgs,CM_ABR)) &&
		      (!test(table[zz].flgs,CM_INV))) {
		      z = zz;
		      break;
		  }
	    }
	    xp = table[z].kwd + cc;
	    if (cmswitch && test(table[z].flgs,CM_ARG)) {
#ifdef VMS
		printf("%s=",xp);
		brkchar = '=';
#else
		printf("%s:",xp);
		brkchar = ':';
#endif /* VMS */
	    } else {
		printf("%s ",xp);
		brkchar = SP;
	    }
#ifdef CK_RECALL
	    if (test(table[z].flgs,CM_NOR)) no_recall = 1;
#endif /* CK_RECALL */
	    cmkwflgs = table[z].flgs;
#ifdef GEMDOS
	    fflush(stdout);
#endif /* GEMDOS */
	    addbuf(xp);
	    if (cmswitch && test(table[z].flgs,CM_ARG)) {
		bp--;			/* Replace trailing space with : */
#ifdef VMS
		*bp++ = '=';
#else
		*bp++ = ':';
#endif /* VMS */
		*bp = NUL;
		np = bp;
		swarg = 1;
	    }
	    inword = 0;
	    cmflgs = 0;
	    debug(F110,"cmkey: addbuf",cmdbuf,0);
	    return(y);

	  case 3:			/* User typed "?" */
	    if (f) {			/* If a conversion function is given */
		char * pp;
		zq = atxbuf;		/* do the conversion now. */
		pp = atxbuf;
		atxn = CMDBL;
		if ((*f)(atmbuf,&zq,&atxn) < 0) return(-2);
		if (setatm(pp,0) < 0) {
		    printf("?Evaluated keyword too long\n");
		    return(-9);
		}
	    }
	    y = lookup(table,atmbuf,n,&z); /* Look up what we have so far. */
	    if (y == -1) {
		/*
		  Strictly speaking if the main keyword table search fails,
		  then we should look in the token table if one is given.
		  But in practice, tokens are also included in the main
		  keyword table.
		*/
		cmflgs = -2;
		if ((pmsg & 1) && !quiet) {
		    bleep(BP_FAIL);
		    printf(" No keywords match\n");
		    return(-9);
		}
		return(-2);
	    }
#ifndef COMMENT
	    /* This is to allow ?-help to work immediately after a token */
	    /* without having to type an intermediate space */
	    if (tl) {
		for (i = 0; i < tl; i++) /* Check for token */
		  if (tok[i] == *atmbuf) { /* Got one */
		      debug(F000,"cmkey token:",atmbuf,*atmbuf);
		      ungword();	/* Put back the following word */
		      cmflgs = 3;	/* Force help next time around */
		      return(-5);	/* Special return code for token */
		  }
	    }
#endif /* COMMENT */

	    if (*xhlp == NUL)
	      printf(" One of the following:\n");
	    else
	      printf(" %s, one of the following:\n",xhlp);
	    {
		int x;
		x = pmsg & (2|4);	/* See kwdhelp() comments */
		if (atmbuf[0])		/* If not at beginning of field */
		  x |= 1;		/* also show invisibles */
		kwdhelp(table,n,atmbuf,"","",1,x);
	    }
#ifndef NOSPL
	    if (!havetoken) {
		extern int topcmd;
		if (tl > 0 && topcmd != XXHLP) /* This is bad... */
		  printf("or a macro name (\"do ?\" for a list) ");
	    }
#endif /* NOSPL */
	    if (*atmbuf == NUL && !havetoken) {
		if (tl == 1)
		  printf("or the token %c\n",*tok);
		else if (tl > 1)
		  printf("or one of the tokens: %s\n",ckspread(tok));
	    }
	    printf("%s%s", cmprom, cmdbuf);
	    fflush(stdout);
	    break;

	  default:
	    printf("\n%d - Unexpected return code from gtword\n",zz);
	    return(cmflgs = -2);
	}
	zz = (pmsg & 8) ? gtword(4) : gtword((pmsg == 4) ? 1 : 0);
	debug(F111,"cmkey gtword zz",atmbuf,zz);
    }
}

int
chktok(tlist) char *tlist; {
    char *p;
    p = tlist;
    while (*p != NUL && *p != *atmbuf) p++;
    return((*p) ? (int) *p : 0);
}

/* Routines for parsing and converting dates and times */

#define isdatesep(c) (ckstrchr(" -/._",c))

#define CMDATEBUF 1024
char cmdatebuf[CMDATEBUF+4] = { NUL, NUL };
static char * cmdatebp = cmdatebuf;
char * cmdatemsg = NULL;

static struct keytab timeunits[] = {
    { "days",   TU_DAYS,   0 },
    { "months", TU_MONTHS, 0 },
    { "weeks",  TU_WEEKS,  0 },
    { "wks",    TU_WEEKS,  0 },
    { "years",  TU_YEARS,  0 },
    { "yrs",    TU_YEARS,  0 }
};
static int nunits = (sizeof(timeunits) / sizeof(struct keytab));

#define SYM_NOW  0
#define SYM_TODA 1
#define SYM_TOMO 2
#define SYM_YEST 3

static struct keytab symdaytab[] = {
    { "now",       SYM_NOW,  0 },
    { "today",     SYM_TODA, 0 },
    { "tomorrow",  SYM_TOMO, 0 },
    { "yesterday", SYM_YEST, 0 }
};
static int nsymdays = (sizeof(symdaytab) / sizeof(struct keytab));

static struct keytab daysofweek[] = {
    { "Friday",    5, 0 },
    { "Monday",    1, 0 },
    { "Saturday",  6, 0 },
    { "Sunday",    0, 0 },
    { "Thursday",  4, 0 },
    { "Tuesday",   2, 0 },
    { "Wednesday", 3, 0 }
};

static struct keytab usatz[] = {	/* RFC 822 timezones  */
    { "cdt",  5, 0 },			/* Values are GMT offsets */
    { "cst",  6, 0 },
    { "edt",  4, 0 },
    { "est",  5, 0 },
    { "gmt",  0, 0 },
    { "mdt",  6, 0 },
    { "mst",  7, 0 },
    { "pdt",  7, 0 },
    { "pst",  8, 0 },
    { "utc",  0, 0 },
    { "zulu", 0, 0 }
};
static int nusatz = (sizeof(usatz) / sizeof(struct keytab));


/*  C M C V T D A T E  --  Converts free-form date to standard form.  */

/*
   Call with
     s = pointer to free-format date, time, or date and time.
     t = 0: return time only if time was given in s.
     t = 1: always return time (00:00:00 if no time given in s).
     t = 2: allow time to be > 24:00:00.
   Returns:
     NULL on failure;
     Pointer to "yyyymmdd hh:mm:ss" (local date-time) on success.
*/

/*
  Before final release the following long lines should be wrapped.
  Until then we leave them long since wrapping them wrecks EMACS's
  C indentation.
*/

/* asctime pattern */
static char * atp1 = "[A-Z][a-z][a-z] [A-Z][a-z][a-z] [ 0-9][0-9] [0-9][0-9]:[0-9][0-9]:[0-9][0-9] [0-9][0-9][0-9][0-9]";

/* asctime pattern with timezone */
static char * atp2 = "[A-Z][a-z][a-z] [A-Z][a-z][a-z] [ 0-9][0-9] [0-9][0-9]:[0-9][0-9]:[0-9][0-9] [A-Z][A-Z][A-Z] [0-9][0-9][0-9][0-9]";

#define DATEBUFLEN 127
#define YYYYMMDD 12

#define isleap(y) (((y) % 4 == 0 && (y) % 100 != 0) || (y) % 400 == 0)
static int mdays[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

#define NEED_DAYS 1
#define NEED_HRS  2
#define NEED_MINS 3
#define NEED_SECS 4
#define NEED_FRAC 5

#define DELTABUF 256
static char deltabuf[DELTABUF];
static char * deltabp = deltabuf;

char *
cmdelta(yy, mo, dd, hh, mm, ss, sign, dyy, dmo, ddd, dhh, dmm, dss)
    int yy, mo, dd, hh, mm, ss, sign, dyy, dmo, ddd, dhh, dmm, dss;
/* cmdelta */ {
    int zyy, zmo, zdd, zhh, zmm, zss;
    long t1, t2, t3, t4;
    long d1 = 0, d2, d3;
    char datebuf[DATEBUFLEN+1];

#ifdef DEBUG
    if (deblog) {
	debug(F101,"cmdelta yy","",yy);
	debug(F101,"cmdelta mo","",mo);
	debug(F101,"cmdelta dd","",dd);
	debug(F101,"cmdelta hh","",hh);
	debug(F101,"cmdelta mm","",mm);
	debug(F101,"cmdelta ss","",ss);
	debug(F101,"cmdelta sign","",sign);
	debug(F101,"cmdelta dyy","",dyy);
	debug(F101,"cmdelta dmo","",dmo);
	debug(F101,"cmdelta ddd","",ddd);
	debug(F101,"cmdelta dhh","",dhh);
	debug(F101,"cmdelta dmm","",dmm);
	debug(F101,"cmdelta dss","",dss);
    }
#endif /* DEBLOG */

    if (yy < 0 || yy > 9999) {
	makestr(&cmdatemsg,"Base year out of range");
	debug(F111,"cmdelta",cmdatemsg,-1);
	return(NULL);
    }
    if (mo < 1 || mo > 12) {
	makestr(&cmdatemsg,"Base month out of range");
	debug(F111,"cmdelta",cmdatemsg,-1);
	return(NULL);
    }
    if (dd < 1 || dd > mdays[mo]) {
	makestr(&cmdatemsg,"Base day out of range");
	debug(F111,"cmdelta",cmdatemsg,-1);
	return(NULL);
    }
    if (hh < 0 || hh > 23) {
	makestr(&cmdatemsg,"Base hour out of range");
	debug(F111,"cmdelta",cmdatemsg,-1);
	return(NULL);
    }
    if (mm < 0 || mm > 59) {
	makestr(&cmdatemsg,"Base minute out of range");
	debug(F111,"cmdelta",cmdatemsg,-1);
	return(NULL);
    }
    if (ss < 0 || ss > 60) {
	makestr(&cmdatemsg,"Base second out of range");
	debug(F111,"cmdelta",cmdatemsg,-1);
	return(NULL);
    }
    sign = (sign < 0) ? -1 : 1;
    if (dmo != 0) {
        if (sign > 0) {
            mo += (sign * dmo);
            if (mo > 12) {
                yy += mo / 12;
                mo = mo % 12;
            }
        } else if (sign < 0) {
            while (dmo > 12) {
                yy--;
                dmo -= 12;
            }
            if (dmo < mo) {
                mo -= dmo;
            } else {
                yy--;
                mo = 12 - (dmo - mo);
            }
        }
    }
    if (dyy != 0) {
	yy += (sign * dyy);
	if (yy > 9999 || yy < 0) {
	    makestr(&cmdatemsg,"Result year out of range");
	    debug(F111,"cmdelta",cmdatemsg,-1);
	    return(NULL);
	}
    }
    sprintf(datebuf,"%04d%02d%02d %02d:%02d:%02d",yy,mo,dd,hh,mm,ss);
    d1 = mjd(datebuf);
    debug(F111,"cmdelta mjd",datebuf,d1);    

    t1 = hh * 3600 + mm * 60 + ss;	/* Base time to secs since midnight */
    t2 = dhh * 3600 + dmm * 60 + dss;	/* Delta time, ditto */
    t3 = t1 + (sign * t2);		/* Get sum (or difference) */
    
    d2 = (sign * ddd);			/* Delta days */
    d2 += t3 / 86400L;

    t4 = t3 % 86400L;			/* Fractional part of day */
    if (t4 < 0) {			/* If negative */
	d2--;				/* one less delta day */
	t4 += 86400L;			/* get positive seconds */
    }
    hh = (int) (t4 / 3600L);
    mm = (int) (t4 % 3600L) / 60;
    ss = (int) (t4 % 3600L) % 60;

    sprintf(datebuf,"%s %02d:%02d:%02d", mjd2date(d1+d2),hh,mm,ss);
    {
	int len, k, n;
	char * p;
	len = strlen(datebuf);
	k = deltabp - (char *)deltabuf;	/* Space used */
	n = DELTABUF - k - 1;		/* Space left */
	if (n < len) {			/* Not enough? */
	    deltabp = deltabuf;		/* Wrap around */
	    n = DELTABUF;
	}
	ckstrncpy(deltabp,datebuf,n);
	p = deltabp;
	deltabp += len + 1;
	return(p);
    }
}


/* Convert Delta Time to Seconds */

int
delta2sec(s,result) char * s; long * result; {
    long ddays = 0L, zz;
    int dsign = 1, dhours = 0, dmins = 0, dsecs = 0, units;
    int state = NEED_DAYS;
    char *p, *p2, *p3, c = 0;
    char buf[64];

    if (!s) s = "";
    if (!*s)
      return(-1);
    if ((int)strlen(s) > 63)
      return(-1);
    ckstrncpy(buf,s,64);
    p = buf;

    if (*p != '+' && *p != '-')
      return(-1);

    if (*p++ == '-')
      dsign = -1;
    while (*p == SP)			/* Skip intervening spaces */
      p++;

    while (state) {			/* FSA to parse delta time */
	if (state < 0 || !isdigit(*p))
	  return(-1);
	p2 = p;				/* Get next numeric field */
	while (isdigit(*p2))
	  p2++;
	c = *p2;			/* And break character */
	*p2 = NUL;			/* Terminate the number */
	switch (state) {		/* Interpret according to state */
	  case NEED_DAYS:		/* Initial */
	    if ((c == '-') ||		/* VMS format */
		((c == 'd' || c == 'D')
		 && !isalpha(*(p2+1)))) { /* Days */
		ddays = atol(p);
		if (!*(p2+1))			
		  state = 0;
		else			/* if anything is left */
		  state = NEED_HRS;	/* now we want hours. */
	    } else if (c == ':') {	/* delimiter is colon */
		dhours = atoi(p);	/* so it's hours */
		state = NEED_MINS;	/* now we want minutes */
	    } else if (!c) {		/* end of string */
		dhours = atoi(p);	/* it's still hours */
		state = 0;		/* and we're done */
	    } else if (isalpha(c) || c == SP) {
		if (c == SP) {		/* It's a keyword? */
		    p2++;		/* Skip spaces */
		    while (*p2 == SP)
		      p2++;
		} else {		/* or replace first letter */
		    *p2 = c;
		}
		p3 = p2;		/* p2 points to beginning of keyword */
		while (isalpha(*p3))	/* Find end of keyword */
		  p3++;
		c = *p3;		/* NUL it out so we can look it up */
		if (*p3)		/* p3 points to keyword terminator */
		  *p3 = NUL;
		if ((units = lookup(timeunits,p2,nunits,NULL)) < 0)
		  return(-1);
		*p2 = NUL;		/* Re-terminate the number */
		*p3 = c;
		while (*p3 == SP)	/* Point at field after units */
		  p3++;
		p2 = p3;
		switch (units) {
		  case TU_DAYS:
		    ddays = atol(p);
		    break;
		  default:
		    return(-1);
		}
		if (*p2) {
		    state = NEED_HRS;
		    p2--;
		} else
		  state = 0;
	    } else {			/* Anything else */
		state = -1;		/* is an error */
	    }
	    break;
	  case NEED_HRS:		/* Looking for hours */
	    if (c == ':') {
		dhours = atoi(p);
		state = NEED_MINS;
	    } else if (!c) {
		dhours = atoi(p);
		state = 0;
	    } else {
		state = -1;
	    }
	    break;
	  case NEED_MINS:		/* Looking for minutes */
	    if (c == ':') {
		dmins = atoi(p);
		state = NEED_SECS;
	    } else if (!c) {
		dmins = atoi(p);
		state = 0;
	    } else {
		state = -1;
	    }
	    break;
	  case NEED_SECS:		/* Looking for seconds */
	    if (c == '.') {
		dsecs = atoi(p);
		state = NEED_FRAC;
	    } else if (!c) {
		dsecs = atoi(p);
		state = 0;
	    } else {
		state = -1;
	    }
	    break;
	  case NEED_FRAC:		/* Fraction of second */
	    if (!c && rdigits(p)) {
		if (*p > '4')
		  dsecs++;
		state = 0;
	    } else {
		state = -1;
	    }
	    break;
	}
	if (c)				/* next field if any */
	  p = p2 + 1;
    }
    if (state < 0)
      return(-1);

    /* if days > 24854 and sizeof(long) == 32 we overflow */

    zz = ddays * 86400L;
    if (zz < 0L)			/* This catches it */
      return(-2);
    zz += dhours * 3600L + dmins * 60L + dsecs;
    zz *= dsign;
    *result = zz;
    return(0);
}


char *
cmcvtdate(s,t) char * s; int t; {
    int x, i, j, k, hh, mm, ss, ff, pmflag = 0, nodate = 0, len, dow;
    int units, isgmt = 0, gmtsign = 0, d = 0, state = 0, nday;
    int kn = 0, ft[8], isletter = 0, f2len = 0;

    int zhh = 0;			/* Timezone adjustments */
    int zmm = 0;
    int zdd = 0;

    int dsign = 1;			/* Delta-time adjustments */
    int ddays = 0;
    int dmonths = 0;
    int dyears = 0;
    int dhours = 0;
    int dmins = 0;
    int dsecs = 0;
    int havedelta = 0;

    char * fld[8], * p = "", * p2, * p3; /* Assorted buffers and pointers  */
    char * s2, * s3;
    char * year = NULL, * month = NULL, * day = NULL;
    char * hour = "00", * min = "00", * sec = "00";
    char datesep = 0;
    char tmpbuf[8];
    char xbuf[DATEBUFLEN+1];
    char ybuf[DATEBUFLEN+1];
    char zbuf[DATEBUFLEN+1];
    char yyyymmdd[YYYYMMDD];
    char dbuf[26];
    char daybuf[3];
    char monbuf[3];
    char yearbuf[5];
    char timbuf[16], *tb, cc;
    char * dp = NULL;			/* Result pointer */

    if (!s) s = "";
    tmpbuf[0] = NUL;

    while (*s == SP) s++;		/* Gobble any leading blanks */
    if (isalpha(*s))			/* Remember if 1st char is a letter */
      isletter = 1;

    len = strlen(s);
    debug(F110,"cmcvtdate",s,len);
    if (len == 0) {			/* No arg - return current date-time */
	dp = ckdate();
	goto xcvtdate;
    }
    if (len > DATEBUFLEN) {		/* Check length of arg */
	makestr(&cmdatemsg,"Date-time string too long");
	debug(F111,"cmcvtdate",cmdatemsg,-1);
	return(NULL);
    }
    hh = 0;				/* Init time to 00:00:00.0 */
    mm = 0;
    ss = 0;
    ff = 0;
    ztime(&p);
    if (!p)
      p  = "";
    if (*p) {				/* Init time to current time */
	x = ckstrncpy(dbuf,p,26);
	if (x > 17) {
	    hh = atoi(&dbuf[11]);
	    mm = atoi(&dbuf[14]);
	    ss = atoi(&dbuf[17]);
	}
    }
    ckstrncpy(yyyymmdd,zzndate(),YYYYMMDD); /* Init date to current date */
    ckstrncpy(yearbuf,yyyymmdd,5);
    ckstrncpy(monbuf,&yyyymmdd[4],3);
    ckstrncpy(daybuf,&yyyymmdd[6],3);
    year = yearbuf;
    month = monbuf;
    day = daybuf;
    nday = atoi(daybuf);
    ckstrncpy(xbuf,s,DATEBUFLEN);	/* Make a local copy we can poke */
    s = xbuf;				/* Point to it */
    s[len] = NUL;
    if (s[0] == ':') {
	p = s;
	goto dotime;
    }
    /* Special preset formats... */

    if (len >= 14) {			/* FTP MDTM all-numeric date */
	char c;
	c = s[14];			/* e.g. 19980615100045.014 */
	s[14] = NUL;
	x = rdigits(s);
	s[14] = c;
	if (x) {
	    ckstrncpy(yyyymmdd,s,8+1);
	    year = NULL;
	    p = &s[8];
	    goto dotime;
	}
    }
    x = 0;				/* Becomes > 0 for asctime format */
    if (isalpha(s[0])) {
	if (len == 24) {		/* Asctime format? */
	    /* Sat Jul 14 15:57:32 2001 */
	    x = ckmatch(atp1,s,0,0);
	    debug(F111,"cmcvtdate asctime",s,x);
	} else if (len == 28) {		/* Or Asctime plus timezone? */
	    /* Sat Jul 14 15:15:39 EDT 2001 */
	    x = ckmatch(atp2,s,0,0);
	    debug(F111,"cmcvtdate asctime+timezone",s,x);
	}
    }
    if (x > 0) {			/* Asctime format */
        int xx;
        strncpy(yearbuf,s + len - 4,4);
        yearbuf[4] = NUL;
        for (i = 0; i < 3; i++)
          tmpbuf[i] = s[i+4];
        tmpbuf[3] = NUL;
	if ((xx = lookup(cmonths,tmpbuf,12,NULL)) < 0) {
	    makestr(&cmdatemsg,"Invalid month");
	    debug(F111,"cmcvtdate",cmdatemsg,-1);
	    return(NULL);
	}
        debug(F101,"cmcvtdate asctime month","",xx);
        monbuf[0] = (xx / 10) + '0'; 
        monbuf[1] = (xx % 10) + '0'; 
        monbuf[2] = NUL;
        daybuf[0] = (s[8] == ' ' ? '0' : s[8]);
        daybuf[1] = s[9];
        daybuf[2] = NUL;
	xbuf[0] = SP;
        for (i = 11; i < 19; i++)
          xbuf[i-10] = s[i];
        xbuf[9] = NUL;
	ckmakmsg(zbuf,18,yearbuf,monbuf,daybuf,xbuf);
	debug(F110,"cmcvtdate asctime ok",zbuf,0);
	if (len == 24) {
	    dp = zbuf;
	    goto xcvtdate;
	} else {
	    int n;
	    n = ckmakmsg(ybuf,DATEBUFLEN-4,zbuf," ",NULL,NULL);
	    ybuf[n++] = s[20];
	    ybuf[n++] = s[21];
	    ybuf[n++] = s[22];
	    ybuf[n++] = NUL;
	    ckstrncpy(xbuf,ybuf,DATEBUFLEN);
	    s = xbuf;
	    isletter = 0;
	}
    }

/* Check for day of week */

    p = s;
    while (*p == SP) p++;
    dow = -1;
    if (*p) {
	p2 = p;
	cc = NUL;
	while (1) {
	    if (*p2 == ',' || *p2 == SP || !*p2) {
		cc = *p2;		/* Save break char */
		*p2 = NUL;		/* NUL it out */
		p3 = p2;		/* Remember this spot */
		if ((dow = lookup(daysofweek,p,7,NULL)) > -1) {
		    debug(F111,"cmcvtdate dow",p,dow);
		    s = p2;
		    if (cc == ',' || cc == SP) { /* Point to next field */
			s++;
			while (*s == SP) s++;
		    }
		    p = s;
		    debug(F111,"cmcvtdate dow new p",p,dow);
		    break;
		} else if (isalpha(*p) && cc == ',') {
		    makestr(&cmdatemsg,"Unrecognized day of week");
		    debug(F111,"cmcvtdate",cmdatemsg,-1);
		    return(NULL);
		} else {
		    *p3 = cc;
		    break;
		}
	    }
	    p2++;
	}
    }
    len = strlen(s);		/* Update length */
    debug(F111,"cmcvtdate s",s,len);

    debug(F111,"cmcvtdate dow",s,dow);
    if (dow > -1) {			/* Have a day-of-week number */
	long zz; int n, j;
	zz = mjd(zzndate());		/* Get today's MJD */
	debug(F111,"cmcvtdate zz","",zz);
	j = (((int)(zz % 7L)) + 3) % 7; /* Today's day-of-week number */
	debug(F111,"cmcvtdate j","",j);
	hh = 0;				/* Init time to midnight */
	mm = 0;
	ss = 0;
	if (j == dow) {
	    ckstrncpy(yyyymmdd,zzndate(),YYYYMMDD);
	    year = NULL;
	} else {
	    n = dow - j;		/* Days from now */
	    if (dow < j)
	      n += 7;
	    if (n < 0) n += 7;		/* Add to MJD */
	    zz += n;
	    ckstrncpy(yyyymmdd,mjd2date(zz),YYYYMMDD); /* New date */
	    year = NULL;
	}
	debug(F111,"cmcvtdate A",yyyymmdd,len);
	if (len == 0) {			/* No more fields after this */
	    ckmakmsg(zbuf,18,yyyymmdd," 00:00:00",NULL,NULL);
	    dp = zbuf;
	    goto xcvtdate;
	}
	isletter = 0;
	if (rdigits(p) && len < 8)	/* Next field is time? */
	  goto dotime;			/* If so go straight to time section */
	if (isdigit(*p)) {
	    if (*(p+1) == ':')
	      goto dotime;
	    else if (isdigit(*(p+1)) && (*(p+2) == ':'))
	      goto dotime;
	}
    }
    debug(F111,"cmcvtdate B s",s,dow);
    debug(F111,"cmcvtdate B p",p,dow);

    if (*s == '+' || *s == '-') {	/* Delta time only - skip ahead. */
	p = s;
	goto delta;
    }
#ifdef COMMENT
/*
  What is the purpose of this?  It breaks parsing of email dates like
  "Wed, 13 Feb 2002 17:43:02 -0800 (PST)".  Removing this code fixes the
  problem and Kermit still passes the 'dates' script.
  - fdc, Sat Nov 26 10:52:45 2005.
*/
    if (dow > -1) {
	/* Day of week given followed by something that is not a time */
	/* or a delta so it can't be valid */
	makestr(&cmdatemsg,"Invalid tokens after day of week");
	debug(F111,"cmcvtdate fail",cmdatemsg,-1);
	return(NULL);
    }
#endif	/* COMMENT */

    /* Handle "today", "yesterday", "tomorrow", and +/- n units */

    if (ckstrchr("TtYyNn",s[0])) {
	int i, k, n, minus = 0;
	char c;
	long jd;
	jd = mjd(ckdate());
	debug(F111,"cmcvtdate mjd",s,jd);

	/* Symbolic date: TODAY, TOMORROW, etc...? */

	s2 = s;				/* Find end of keyword */
	i = 0;
	while (isalpha(*s2)) {		/* and get its length */
	    i++;
	    s2++;
	}
	c = *s2;			/* Zap but save delimiter */
	*s2 = NUL;
	k = lookup(symdaytab,s,nsymdays,NULL); /* Look up keyword */
	*s2 = c;			/* Replace delimiter */
	if (k < 0)			/* Keyword not found */
	  goto normal;
	s3 = &s[i];
	while (*s3 == SP)		/* Skip whitespace */
	  s3++;
	if (*s3 == '_' || *s3 == ':')
	  s3++;

	switch (k) {			/* Have keyword */
	  case SYM_NOW:			/* NOW */
	    ckstrncpy(ybuf,ckdate(),DATEBUFLEN);
	    ckstrncpy(yyyymmdd,ybuf,YYYYMMDD);
	    year = NULL;
	    if (*s3) {			/* No overwriting current time. */
		ckstrncat(ybuf," ",DATEBUFLEN);
		ckstrncat(ybuf,s3,DATEBUFLEN);
	    }
	    break;
	  default:			/* Yesterday, Today, and Tomorrow */
	    if (k == SYM_TOMO) {	/* TOMORROW */
		strncpy(ybuf,mjd2date(jd+1),8);
	    } else if (k == SYM_YEST) {	/* YESTERDAY */
		strncpy(ybuf,mjd2date(jd-1),8);
	    } else {			/* TODAY */
		strncpy(ybuf,ckdate(),8);
	    }
	    strncpy(ybuf+8," 00:00:00",DATEBUFLEN-8); /* Default time is 0 */
	    ckstrncpy(yyyymmdd,ybuf,YYYYMMDD);
	    year = NULL;
	    if (*s3) {			/* If something follows keyword... */
		if (isdigit(*s3)) {	/* Time - overwrite default time */
		    strncpy(ybuf+8,s+i,DATEBUFLEN-8);
		} else {		/* Something else, keep default time */
		    ckstrncat(ybuf," ",DATEBUFLEN); /* and append */
		    ckstrncat(ybuf,s3,DATEBUFLEN); /* whatever we have */
		}
	    }
	}
	s = ybuf;			/* Point to rewritten date-time */
	len = strlen(s);		/* Update length */
	isletter = 0;			/* Cancel this */
    }

/* Regular free-format non-symbolic date */

  normal:

    debug(F111,"cmcvtdate NORMAL",s,len);
    debug(F111,"cmcvtdate dow",s,dow);
    if (yyyymmdd[0] && !year) {
	ckstrncpy(yearbuf,yyyymmdd,5);
	ckstrncpy(monbuf,&yyyymmdd[4],3);
	ckstrncpy(daybuf,&yyyymmdd[6],3);
	year = yearbuf;
	month = monbuf;
	day = daybuf;
	nday = atoi(daybuf);
    }
    if (isdigit(s[0])) {		/* Time without date? */
	p = s;
	if (s[1] == ':') {
	    debug(F111,"cmcvtdate NORMAL X1",s,len);
	    goto dotime;
	} else if (len > 1 && isdigit(s[1]) && s[2] == ':') {
	    debug(F111,"cmcvtdate NORMAL X2",s,len);
	    goto dotime;
	} else if (rdigits(s) && len < 8) {
	    debug(F111,"cmcvtdate NORMAL X3",s,len);
	    goto dotime;
	}
    }
    if (len >= 8 && isdigit(*s)) {	/* Check first for yyyymmdd* */
	debug(F111,"cmcvtdate NORMAL A",s,len);
	cc = s[8];
	s[8] = NUL;			/* Isolate first 8 characters */
	if (rdigits(s)) {
	    /* Have valid time separator? */
	    p2 = cc ? ckstrchr(" Tt_-:",cc) : NULL;
	    if (!cc || p2) {
		ckstrncpy(yyyymmdd,s,YYYYMMDD);	/* Valid separator */
		year = NULL;
		s += 8;			        /* or time not given */
		if (cc) s++;		        /* Keep date */
		p = s;			        /* and go handle time */
		goto dotime;
	    } else if (!p2) {
		if (isdigit(cc))
		  makestr(&cmdatemsg,"Numeric date too long");
		else
		  makestr(&cmdatemsg,"Invalid date-time separator");
		debug(F111,"cmcvtdate",cmdatemsg,-1);
		return(NULL);
	    }
	}
	s[8] = cc;			/* Put this back! */
    }
    debug(F111,"cmcvtdate NORMAL non-yyyymmdd",s,len);

    /* Free-format date -- figure it out */

#ifdef COMMENT
    if (*s && !isdigit(*s)) {
	makestr(&cmdatemsg,"Unrecognized word in date");
	debug(F111,"cmcvtdate",cmdatemsg,-1);
	return(NULL);
    }
#endif /* COMMENT */
    for (i = 0; i < 8; i++)		/* Field types */
      ft[i] = -1;
    fld[i = 0] = (p = s);		/* First field */
    while (*p) {			/* Get next two fields */
	if (isdatesep(*p)) {		/* Have a date separator */
	    if (i == 0) {
		datesep = *p;
	    } else if (i == 1 && *p != datesep) {
		makestr(&cmdatemsg,"Inconsistent date separators");
		debug(F111,"cmcvtdate",cmdatemsg,-1);
		return(NULL);
	    }
	    *p++ = NUL;			/* Replace by NUL */
	    if (*p) {			/* Now we're at the next field */
		while (*p == SP) p++;	/* Skip leading spaces */
		if (!*p) break;		/* Make sure we still have something */
		if (i == 2)		/* Last one? */
		  break;
		fld[++i] = p;		/* No, record pointer to this one */
	    } else {
		break;
	    }	    
	} else if ((*p == 'T' || *p == 't') && isdigit(*(p+1))) { /* Time */
	    *p++ = NUL;
	    break;
	} else if (*p == ':') {
	    if (i == 0 && p == s) {
		nodate = 1;
		break;
	    } else if (i != 0) {	/* After a date */
		if (i == 2) {		/* OK as date-time separator (VMS) */
		    *p++ = NUL;
		    break;
		}
		if (i < 2)
		  makestr(&cmdatemsg,"Too few fields in date");
		else
		  makestr(&cmdatemsg,"Misplaced time separator");
		debug(F111,"cmcvtdate",cmdatemsg,-1);
		return(NULL);
	    }
	    nodate = 1;			/* Or without a date */
	    break;
	}
	p++;
    }
    if (p > s && i == 0)		/* Make sure we have a date */
      nodate = 1;			/* No date. */

    if (nodate && dow > -1) {		/* Have implied date from DOW? */
	goto dotime;			/* Use, use that, go do time. */

    } else if (nodate) {		/* No date and no implied date */
	char *tmp = NULL;		/* Substitute today's date */
	ztime(&tmp);
	if (!tmp)
	  tmp  = "";
	if (!*tmp) {
	    makestr(&cmdatemsg,"Problem supplying current date");
	    debug(F111,"cmcvtdate",cmdatemsg,-1);
	    return(NULL);
	}
	ckstrncpy(dbuf,tmp,26);		/* Reformat */
	if (dbuf[8] == SP) dbuf[8] = '0';
	fld[0] = dbuf+8;		/* dd */
	dbuf[10] = NUL;
	fld[1] = dbuf+4;		/* mmm */
	dbuf[7] = NUL;
	fld[2] = dbuf+20;		/* yyyy */
	dbuf[24] = NUL;
	hh = atoi(&dbuf[11]);
	mm = atoi(&dbuf[14]);
	ss = atoi(&dbuf[17]);
	p = s;				/* Back up source pointer to reparse */
    } else if (i < 2) {
	makestr(&cmdatemsg,"Too few fields in date");
	debug(F111,"cmcvtdate",cmdatemsg,-1);
	return(NULL);
    }
    /* Have three date fields - see what they are */

    for (k = 0, j = 0; j < 3; j++) {	/* Get number of non-numeric fields */
	ft[j] = rdigits(fld[j]);
	debug(F111,"cmcvtdate fld",fld[j],j);
	if (ft[j] == 0)
	  k++;
    }
    kn = k;				/* How many numeric fields */
    month = NULL;			/* Strike out default values */
    year = NULL;
    day = NULL;

    if (k == 2 && ft[2] > 0) {		/* Jul 20, 2001 */
	int xx;
	xx = strlen(fld[1]);
	p3 = fld[1];
	if (xx > 0) if (p3[xx-1] == ',') {
	    p3[xx-1] = NUL;
	    if (rdigits(p3)) {
		k = 1;	
		ft[1] = 1;
	    } else p3[xx-1] = ',';
	}
    }
    if (k > 1) {			/* We can have only one non-numeric */
	if (nodate)
	  makestr(&cmdatemsg,"Unrecognized word in date"); 
	else if (!ft[2] && isdigit(*(fld[2])))
	  makestr(&cmdatemsg,"Invalid date-time separator"); 
	else
	  makestr(&cmdatemsg,"Too many non-numeric fields in date");
	debug(F111,"cmcvtdate",cmdatemsg,-1);
	return(NULL);
    }
    if (!ft[0]) {
	k = 0;
    } else if (!ft[1]) {
	k = 1;
    } else if (!ft[2]) {
	makestr(&cmdatemsg,"Non-digit in third date field");
	debug(F111,"cmcvtdate",cmdatemsg,-1);
	return(NULL);
    } else
      k = -1;

    if (k > -1) {
	if ((x = lookup(cmonths,fld[k],12,NULL)) < 0) {
	    makestr(&cmdatemsg,"Unknown month");
	    debug(F111,"cmcvtdate",cmdatemsg,-1);
	    return(NULL);
	}
	sprintf(tmpbuf,"%02d",x);
	month = tmpbuf;
    }
    f2len = strlen(fld[2]);		/* Length of 3rd field */

    if (k == 0) {			/* monthname dd, yyyy */
	day = fld[1];
	year = fld[2];
    } else if (((int)strlen(fld[0]) == 4)) { /* yyyy-xx-dd */
	year = fld[0];
	day = fld[2];
	if (!month)
	  month = fld[1];		/* yyyy-mm-dd */
    } else if (f2len == 4) {		/* xx-xx-yyyy */
	year = fld[2];
	if (month) {			/* dd-name-yyyy */
	    day = fld[0];
	} else {			/* xx-xx-yyyy */
	    int f0, f1;
	    f0 = atoi(fld[0]);
	    f1 = atoi(fld[1]);
	    if (((f0 > 12) && (f1 <= 12)) || (f1 <= 12 && f0 == f1)) {
		day = fld[0];		/* mm-dd-yyyy */
		month = fld[1];
	    } else if ((f0 <= 12) && (f1 > 12)) {
		if (!rdigits(fld[1])) {
		    makestr(&cmdatemsg,"Day not numeric");
		    debug(F111,"cmcvtdate",cmdatemsg,-1);
		    return(NULL);
		} else {
		    day = fld[1];	/* dd-mm-yyyy */
		}
		month = fld[0];
	    } else {
		if (!f0 || !f1)
		  makestr(&cmdatemsg,"Day or month out of range");
		else
		  makestr(&cmdatemsg,"Day and month are ambiguous");
		debug(F111,"cmcvtdate",cmdatemsg,-1);
		return(NULL);
	    }
	}
    } else if ((f2len < 4) &&		/* dd mmm yy (RFC822) */
	       !rdigits(fld[1]) &&	/* middle field is monthname */
	       rdigits(fld[2])) {
	int tmpyear;
	day = fld[0];
	if (!fld[2][1]) {
	    makestr(&cmdatemsg,"Too few digits in year");
	    debug(F111,"cmcvtdate",cmdatemsg,-1);
	    return(NULL);
	}
	tmpyear = atoi(fld[2]);
	if (tmpyear < 50)		/* RFC 2822 windowing */
	  tmpyear += 2000;
	else				/* This includes 3-digit years. */
	  tmpyear += 1900;
	year = ckitoa(tmpyear);

    } else if ((f2len < 4) && (k < 0) && ((int)strlen(fld[0]) < 4)) {
	makestr(&cmdatemsg,"Ambiguous numeric date");
	debug(F111,"cmcvtdate",cmdatemsg,-1);
	return(NULL);
    } else if ((f2len > 4) && ft[2]) {
	makestr(&cmdatemsg,"Too many digits in year");
	debug(F111,"cmcvtdate",cmdatemsg,-1);
	return(NULL);
    } else {
	makestr(&cmdatemsg,"Unexpected date format");
	debug(F111,"cmcvtdate",cmdatemsg,-1);
	return(NULL);
    }
    x = atoi(month);
    sprintf(tmpbuf,"%02d",x);		/* 2-digit numeric month */

/*
   state = 1 = hours
   state = 2 = minutes
   state = 3 = seconds
   state = 4 = fractions of seconds
*/

  dotime:
    if (isletter && (s == p)) {
	makestr(&cmdatemsg,"Unknown date-time word");
	debug(F111,"cmcvtdate",cmdatemsg,-1);
	return(NULL);
    }
    if (!year && yyyymmdd[0]) {
	debug(F110,"cmcvtdate dotime yyyymmdd",yyyymmdd,0);
	for (i = 0; i < 4; i++)
	  yearbuf[i] = yyyymmdd[i];
	yearbuf[4] = NUL;
	monbuf[0] = yyyymmdd[4];
	monbuf[1] = yyyymmdd[5];
	monbuf[2] = NUL;
	daybuf[0] = yyyymmdd[6];
	daybuf[1] = yyyymmdd[7];
	daybuf[2] = NUL;
	day = daybuf;
	nday = atoi(daybuf);
	month = monbuf;
	year = yearbuf;
    }
    if (!year) {
	makestr(&cmdatemsg,"Internal error - date not defaulted");
	debug(F111,"cmcvtdate",cmdatemsg,-1);
	return(NULL);
    }
    /* Get here with day, month, and year set */
    debug(F110,"cmcvtdate dotime day",day,0);
    debug(F110,"cmcvtdate dotime month",month,0);
    debug(F110,"cmcvtdate dotime year",year,0);
    debug(F110,"cmcvtdate dotime s",s,0);
    debug(F110,"cmcvtdate dotime p",p,0);
    x = atoi(month);
    if (x > 12 || x < 1) {
	makestr(&cmdatemsg,"Month out of range");
	debug(F111,"cmcvtdate",cmdatemsg,-1);
	return(NULL);
    }
    nday  = atoi(day);
    i = mdays[x];
    if (x == 2) if (isleap(atoi(year))) i++;
    if (nday > i || nday < 1) {
	makestr(&cmdatemsg,"Day out of range");
	debug(F111,"cmcvtdate",cmdatemsg,-1);
	return(NULL);
    }
    if (!*p && t == 0) {
	sprintf(zbuf,"%04d%02d%02d",atoi(year),atoi(month),nday);	
	dp = zbuf;
	goto xcvtdate;
    }
    if (*p == '+' || *p == '-') {	/* GMT offset without a time */
	hh = 0;				/* so default time to 00:00:00 */
	mm = 0;
	ss = 0;
	goto cmtimezone;		/* and go do timezone */
    }
    if (*p && !isdigit(*p) && *p != ':') {
	makestr(&cmdatemsg,"Invalid time");
	debug(F111,"cmcvtdate",cmdatemsg,-1);
	return(NULL);
    }
    sprintf(yyyymmdd,"%s%s%02d",year,month,nday); /* for tz calculations... */

    state = 1;				/* Initialize time-parsing FSA */
    hh = 0;				/* hours */
    mm = 0;				/* minutes */
    ss = 0;				/* seconds */
    ff = -1;				/* fraction */
    d = 0;				/* Digit counter */
    p2 = p;				/* Preliminary digit count... */
    while (isdigit(*p2)) {
	d++;
	p2++;
    }
    if (d > 6) {
	makestr(&cmdatemsg,"Too many time digits");
	debug(F111,"cmcvtdate",cmdatemsg,-1);
	return(NULL);
    }
    d = (d & 1 && *p2 != ':') ? 1 : 0;	/* Odd implies leading '0' */

    while (*p) {			/* Get the time, if any */
	if (isdigit(*p)) {		/* digit */
	    if (d++ > 1) {
		state++;
		d = 1;
	    }
	    switch (state) {
	      case 1:			/* Hours */
		hh = hh * 10 + (*p - '0');
		break;
	      case 2:			/* Minutes */
		mm = mm * 10 + (*p - '0');
		break;
	      case 3:			/* Seconds */
		ss = ss * 10 + (*p - '0');
		break;
	      case 4:			/* Fraction of second */
		if (ff < 0)
		  ff = (*p > '4') ? 1 : 0;
		break;
	    }
	} else if (*p == ':') {		/* Colon */
	    state++;
	    d = 0;
	    if (state > 3) {
		makestr(&cmdatemsg,"Too many time fields");
		debug(F111,"cmcvtdate",cmdatemsg,-1);
		return(NULL);
	    }
	} else if (*p == '.') {
	    if (state == 3) {
		state = 4;
		d = 0;
	    } else {
		makestr(&cmdatemsg,"Improper fraction");
		debug(F111,"cmcvtdate",cmdatemsg,-1);
		return(NULL);
	    }
	} else if (*p == SP) {		/* Space */
	    while (*p && (*p == SP))	/* position to first nonspace */
	      p++;
	    break;
	} else if (isalpha(*p)) {	/* AM/PM/Z or timezone */
	    break;
	} else if (*p == '+' || *p == '-') { /* GMT offset */
	    break;
	} else {
	    makestr(&cmdatemsg,"Invalid time characters");
	    debug(F111,"cmcvtdate",cmdatemsg,-1);
	    return(NULL);
	}
	p++;
    }
    if (!*p)				/* If nothing left */
      goto xcmdate;			/* go finish up */

    /* At this point we have HH, MM, SS, and FF */
    /* Now handle the rest: AM, PM, and/or timezone info */

    if (!ckstrcmp(p,"am",2,0)) {	/* AM/PM... */
	pmflag = 0;
	p += 2;
    } else if (!ckstrcmp(p,"a.m.",4,0)) {
	pmflag = 0;
	p += 4;
    } else if (!ckstrcmp(p,"pm",2,0)) {
	pmflag = 1;
	p += 2;
    } else if (!ckstrcmp(p,"p.m.",4,0)) {
	pmflag = 1;
	p += 4;
    }
    if (pmflag && hh < 12)		/* If PM was given */
      hh += 12;				/* add 12 to the hour */

    /* Now handle timezone */

  cmtimezone:
    debug(F110,"cmcvtdate timezone",p,0);

    zhh = 0;				/* GMT offset HH */
    zmm = 0;				/* GMT offset MM */
    gmtsign = 0;			/* Sign of GMT offset */
    isgmt = 0;				/* 1 if time is GMT */

    while (*p && *p == SP)		/* Gobble spaces */
      p++;
    if (!*p)				/* If nothing left */
      goto xcmdate;			/* we're done */

    if (isalpha(*p)) {			/* Something left */
	int zone = 0;			/* Alphabetic must be timezone */
	p2 = p;				/* Isolate timezone */
	p++;
	while (isalpha(*p))
	  p++;
	p3 = p;
	cc = *p;
	*p = NUL;
	p = p2;				/* Have timezone, look it up */
	zone = lookup(usatz,p,nusatz,NULL);
	debug(F111,"cmcvtdate timezone alpha",p,zone);

	if (zone < 0) {			/* Not found */
	    makestr(&cmdatemsg,"Unknown timezone");
	    debug(F111,"cmcvtdate",cmdatemsg,-1);
	    return(NULL);
	}
	isgmt++;			/* All dates are GMT from here down */
	if (zone != 0) {		/* But not this one so make it GMT */
	    hh += zone;			/* RFC 822 timezone: EST etc */
	    debug(F101,"cmcvtdate hh + zone","",hh);
	    if (hh > 23) {		/* Offset crosses date boundary */
		int i;
		long jd;
		jd = mjd(yyyymmdd);	/* Get MJD */
		jd += hh / 24;		/* Add new day(s) */
		hh = hh % 24;		/* and convert back to yyyymmdd */
		ckstrncpy(yyyymmdd,mjd2date(jd),YYYYMMDD);
		debug(F111,"cmcvtdate zone-adjusted date",yyyymmdd,hh);
		for (i = 0; i < 4; i++)
		  yearbuf[i] = yyyymmdd[i];
		yearbuf[4] = NUL;
		monbuf[0] = yyyymmdd[4];
		monbuf[1] = yyyymmdd[5];
		monbuf[2] = NUL;
		daybuf[0] = yyyymmdd[6];
		daybuf[1] = yyyymmdd[7];
		daybuf[2] = NUL;
		day = daybuf;
		nday = atoi(daybuf);
		month = monbuf;
		year = yearbuf;
	    }
	}
	p = p3;				/* Put back whatever we poked above */
	*p = cc;

    } else if (*p == '+' || *p == '-') { /* GMT/UTC offset */
	p3 = p;
	debug(F110,"cmcvtdate timezone GMT offset",p,0);
	gmtsign = (*p == '+') ? -1 : 1;
	isgmt++;
	p++;
	while (*p == SP) p++;
	d = 0;
	p2 = p;
	while (isdigit(*p)) {		/* Count digits */
	    d++;
	    p++;
	}
	if (d != 4) {			/* Strict RFC [2]822 */
	    isgmt = 0;			/* If not exactly 4 digits */
	    p = p3;			/* it's not a GMT offset. */
	    goto delta;			/* So treat it as a delta time. */
	}
	d = (d & 1 && *p != ':') ? 1 : 0; /* Odd implies leading '0' */
	p = p2;
	debug(F111,"cmcvtdate GMT offset sign",p,gmtsign);
	debug(F101,"cmcvtdate GMT offset d","",d);
	state = 1;
	while (*p) {
	    if (isdigit(*p)) {		/* digit */
		if (d++ > 1) {
		    state++;
		    d = 1;
		}
		switch (state) {
		  case 1:
		    zhh = zhh * 10 + (*p - '0');
		    break;
		  case 2:
		    zmm = zmm * 10 + (*p - '0');
		    break;
		  default:		/* Ignore seconds or fractions */
		    break;
		}			
	    } else if (*p == ':') {	/* Colon */
		state++;
		d = 0;
	    } else if (*p == SP || *p == '(') {
		break;
	    } else {
		p = p3;			/* Maybe it's not a GMT offset. */
		goto delta;		/* So treat it as a delta time. */
	    }
	    p++;
	}
    }
    debug(F110,"cmcvtdate source string after timezone",p,0);

    if (*p) {				/* Anything left? */
	p2 = p;
	while (*p2 == SP)		/* Skip past spaces */
	  p2++;
	if (*p2 == '(') {		/* RFC-822 comment? */
	    int pc = 1;			/* paren counter */
	    p2++;
	    while (*p2) {
		if (*p2 == ')') {
		    if (--pc == 0) {
			p2++;
			break;
		    }
		} else if (*p2 == ')') {
		    pc++;
		}
		p2++;
	    }		
	    while (*p2 == SP)		/* Skip past spaces */
	      p2++;
	    if (!*p2)			/* Anything left? */
	      *p = NUL;			/* No, erase comment */
	}
	if (!*p2)			/* Anything left? */
	  goto xcmdate;			/* No, done. */
	p = p2;

      delta:
	debug(F110,"cmcvtdate delta yyyymmdd",yyyymmdd,0);
	debug(F110,"cmcvtdate delta year",year,0);
	debug(F110,"cmcvtdate delta p",p,0);

	if (*p == '+' || *p == '-') {	/* Delta time */
	    int state = NEED_DAYS;	/* Start off looking for days */
	    char c = 0;
	    dsign = 1;			/* Get sign */
	    if (*p++ == '-')
	      dsign = -1;
	    while (*p == SP)		/* Skip intervening spaces */
	      p++;
	    while (state) {		/* FSA to parse delta time */
		if (state < 0 || !isdigit(*p)) {
		    makestr(&cmdatemsg,"Invalid delta time");
		    debug(F111,"cmcvtdate",cmdatemsg,-1);
		    return(NULL);
		}
		p2 = p;			/* Get next numeric field */
		while (isdigit(*p2))
		  p2++;
		c = *p2;		/* And break character */
		*p2 = NUL;		/* Terminate the number */

		switch (state) {	/* Interpret according to state */
		  case NEED_DAYS:	/* Initial */
		    if ((c == '-') ||	/* VMS format */
			((c == 'd' || c == 'D')
			 && !isalpha(*(p2+1)))) { /* Days */
			ddays = atoi(p);
			if (!*(p2+1))			
			  state = 0;
			else		      /* if anything is left */
			  state = NEED_HRS;   /* now we want hours. */
		    } else if ((c == 'W' || c == 'w') && !isalpha(*(p2+1))) {
			ddays = atoi(p) * 7;   /* weeks... */
			if (!*(p2+1))			
			  state = 0;
			else
			  state = NEED_HRS;
		    } else if ((c == 'M' || c == 'm') && !isalpha(*(p2+1))) {
			dmonths = atoi(p); /* months... */
			if (!*(p2+1))			
			  state = 0;
			else
			  state = NEED_HRS;
		    } else if ((c == 'Y' || c == 'y') && !isalpha(*(p2+1))) {
			dyears = atoi(p); /* years... */
			if (!*(p2+1))			
			  state = 0;
			else
			  state = NEED_HRS;
		    } else if (c == ':') { /* delimiter is colon */
			dhours = atoi(p);  /* so it's hours */
			state = NEED_MINS; /* now we want minutes */
		    } else if (!c) {       /* end of string */
			dhours = atoi(p);  /* it's still hours */
			state = 0;         /* and we're done */
		    } else if (isalpha(c) || c == SP) {
			if (c == SP) {	/* It's a keyword? */
			    p2++;	/* Skip spaces */
			    while (*p2 == SP)
			      p2++;
			} else {	/* or replace first letter */
			    *p2 = c;
			}
			p3 = p2;	/* p2 points to beginning of keyword */
			while (isalpha(*p3)) /* Find end of keyword */
			  p3++;
			c = *p3;	/* NUL it out so we can look it up */
			if (*p3)	/* p3 points to keyword terminator */
			  *p3 = NUL;
			units = lookup(timeunits,p2,nunits,NULL);
			if (units < 0) {
			    makestr(&cmdatemsg,"Invalid units in delta time");
			    debug(F111,"cmcvtdate",cmdatemsg,-1);
			    return(NULL);
			}
			*p2 = NUL;	/* Re-terminate the number */
			*p3 = c;
			while (*p3 == SP) /* Point at field after units */
			  p3++;
			p2 = p3;
			switch (units) {
			  case TU_DAYS:
			    ddays = atoi(p);
			    break;
			  case TU_WEEKS:
			    ddays = atoi(p) * 7;
			    break;
			  case TU_MONTHS:
			    dmonths = atoi(p);
			    break;
			  case TU_YEARS:
			    dyears = atoi(p);
			    break;
			}
			if (*p2) {
			    state = NEED_HRS;
			    p2--;
			} else
			  state = 0;

		    } else {		/* Anything else */
			state = -1;	/* is an error */
		    }
		    break;
		  case NEED_HRS:	/* Looking for hours */
		    debug(F000,"cmcvtdate NEED_HRS",p,c);
		    if (c == ':') {
			dhours = atoi(p);
			state = NEED_MINS;
		    } else if (!c) {
			dhours = atoi(p);
			state = 0;
		    } else {
			state = -1;
		    }
		    break;
		  case NEED_MINS:	/* Looking for minutes */
		    if (c == ':') {
			dmins = atoi(p);
			state = NEED_SECS;
		    } else if (!c) {
			dmins = atoi(p);
			state = 0;
		    } else {
			state = -1;
		    }
		    break;
		  case NEED_SECS:	/* Looking for seconds */
		    if (c == '.') {
			dsecs = atoi(p);
			state = NEED_FRAC;
		    } else if (!c) {
			dsecs = atoi(p);
			state = 0;
		    } else {
			state = -1;
		    }
		    break;
		  case NEED_FRAC:	/* Fraction of second */
		    if (!c && rdigits(p)) {
			if (*p > '4')
			  dsecs++;
			state = 0;
		    } else {
			state = -1;
		    }
		    break;
		}
		if (c)			/* next field if any */
		  p = p2 + 1;
	    }
	    havedelta = 1;

	} else {
	    makestr(&cmdatemsg,"Extraneous material at end");
	    debug(F111,"cmcvtdate",cmdatemsg,-1);
	    return(NULL);
	}
    }

 xcmdate:

    if ((t != 2 && hh > 24) || hh < 0) { /* Hour range check */
	makestr(&cmdatemsg,"Invalid hours");
	debug(F111,"cmcvtdate",cmdatemsg,-1);
	return(NULL);
    }
    if (mm > 59) {			/* Minute range check */
	makestr(&cmdatemsg,"Invalid minutes");
	debug(F111,"cmcvtdate",cmdatemsg,-1);
	return(NULL);
    }
    if (ff > 0) {			/* Fraction of second? */
	if (ss < 59) {
	    ss++;
	    ff = 0;
	} else if (mm < 59) {
	    ss = 0;
	    mm++;
	    ff = 0;
	} else if (hh < 24) {
	    ss = 0;
	    mm = 0;
	    hh++;
	    ff = 0;
	}
	/* Must add a day -- leave ff at 1... */
	/* (DO SOMETHING ABOUT THIS LATER) */
    }
    if (ss > 60) {			/* Seconds range check */
	makestr(&cmdatemsg,"Invalid seconds"); /* 60 is ok because of */
	debug(F111,"cmcvtdate",cmdatemsg,-1);  /* Leap Second. */
	return(NULL);
    }
    if ((mm < 0 || ss < 0) ||
	(t != 2 && (ss > 0 || mm > 0) && hh > 23)) {
	makestr(&cmdatemsg,"Invalid minutes or seconds");
	debug(F111,"cmcvtdate",cmdatemsg,-1);
	return(NULL);
    }
    debug(F110,"cmcvtdate year",year,0);
    debug(F110,"cmcvtdate month",month,0);
    debug(F101,"cmcvtdate nday","",nday);
    debug(F101,"cmcvtdate hh","",hh);
    debug(F101,"cmcvtdate mm","",mm);
    debug(F101,"cmcvtdate ss","",ss);
    debug(F101,"cmcvtdate gmtsign","",gmtsign);
    debug(F101,"cmcvtdate zhh","",zhh);
    debug(F101,"cmcvtdate zmm","",zmm);
    debug(F101,"cmcvtdate isgmt","",isgmt);

#ifdef ZLOCALTIME
/* Handle timezone -- first convert to GMT */

    zdd = 0;				/* Days changed */
    if (isgmt && (zmm || zhh)) {	/* If GMT offset given */
	long sec1, sec2, zz;
	sec1 = ss + 60 * mm + 3600 * hh;
	sec2 = gmtsign * (60 * zmm + 3600 * zhh);
	sec1 += sec2;
	if (sec1 < 0) {
	    sec1 = 0 - sec1;
	    zdd = 0L - (sec1 / 86400L);
	    sec1 = sec1 % 86400L;
	} else if (sec1 > 86400L) {
	    zdd = sec1 / 86400L;
	    sec1 = sec1 % 86400L;
	}
	ss = sec1 % 60;
	zz = sec1 / 60;
	mm = zz % 60;
	hh = zz / 60;
	debug(F101,"cmcvtdate NEW hh","",hh);
	debug(F101,"cmcvtdate NEW mm","",mm);
	debug(F101,"cmcvtdate NEW dd","",zdd);

/* At this point hh:mm:ss is in GMT and zdd is the calendar adjustment */

    }
#endif /* ZLOCALTIME */

    if (yyyymmdd[0] && !year) {
	ckstrncpy(yearbuf,yyyymmdd,5);
	ckstrncpy(monbuf,&yyyymmdd[4],3);
	ckstrncpy(daybuf,&yyyymmdd[6],3);
	year = yearbuf;
	month = monbuf;
	day = daybuf;
	nday = atoi(daybuf);
    }
    sprintf(zbuf,"%04d%02d%02d %02d:%02d:%02d", /* SAFE */
	    atoi(year),atoi(month),nday,hh,mm,ss
	    );
    dp = zbuf;

#ifdef ZLOCALTIME
    /* Now convert from GMT to local time */

    if (isgmt) {			/* If GMT convert to local time */
	debug(F110,"cmcvtdate GMT 1",dp,0);
	if (zdd) {			/* Apply any calendar adjustment */
	    long zz;
	    zz = mjd(dp) + zdd;
	    sprintf(zbuf,"%s %02d:%02d:%02d",mjd2date(zz),hh,mm,ss);
	}
	debug(F110,"cmcvtdate GMT 2",dp,0);
	if ((p = zlocaltime(dp))) {
	    debug(F110,"cmcvtdate asctime zlocaltime",p,0);
	    if (p) ckstrncpy(zbuf,p,18);
	}
	debug(F110,"cmcvtdate GMT 3",dp,0);
	for (i = 0; i < 4; i++)
	  yearbuf[i] = dp[i];
	yearbuf[4] = NUL;
	monbuf[0] = dp[4];
	monbuf[1] = dp[5];
	monbuf[2] = NUL;
	daybuf[0] = dp[6];
	daybuf[1] = dp[7];
	daybuf[2] = NUL;
	day = daybuf;
	nday = atoi(daybuf);
	month = monbuf;
	year = yearbuf;
	hh = atoi(&dp[9]);
	mm = atoi(&dp[12]);
	ss = atoi(&dp[15]);
    }
#endif /* ZLOCALTIME */

#ifdef DEBUG
    if (deblog) {
	debug(F101,"cmcvtdate hour","",hh);
	debug(F101,"cmcvtdate minute","",mm);
	debug(F101,"cmcvtdate second","",ss);
    }
#endif /* DEBLOG */

    makestr(&cmdatemsg,NULL);
    if (havedelta) {
#ifdef DEBUG
	if (deblog) {
	    debug(F110,"cmcvtdate base ",dp,0);
	    debug(F101,"cmcvtdate delta sign","",dsign);
	    debug(F101,"cmcvtdate delta yrs ","",dyears);
	    debug(F101,"cmcvtdate delta mos ","",dmonths);
	    debug(F101,"cmcvtdate delta days","",ddays);
	    debug(F101,"cmcvtdate delta hrs ","",dhours);
	    debug(F101,"cmcvtdate delta mins","",dmins);
	    debug(F101,"cmcvtdate delta secs","",dsecs);
	}
#endif /* DEBLOG */
	if (!(dp = cmdelta(atoi(year),
		    atoi(month),
		    nday, hh, mm, ss,
		    dsign, dyears, dmonths, ddays, dhours, dmins, dsecs))) {
	    debug(F111,"cmcvtdate",cmdatemsg,-1);
	    return(NULL);
	}
    }

  xcvtdate:				/* Exit point for success */
    {
	int len, k, n;
	char * p;
	debug(F110,"cmcvtdate xcvtdate dp",dp,0);
	if (!dp) dp = "";		/* Shouldn't happen */
	if (!*dp) return(NULL);		/* ... */
	len = strlen(dp);
	debug(F111,"cmcvtdate result",dp,len);
	k = cmdatebp - (char *)cmdatebuf; /* Space used */
	n = CMDATEBUF - k - 1;		/* Space left */
	if (n < len) {			/* Not enough? */
	    cmdatebp = cmdatebuf;	/* Wrap around */
	    n = CMDATEBUF;
	}
	ckstrncpy(cmdatebp,dp,n);
	p = cmdatebp;
	cmdatebp += len + 1;
	return(p);
    }
}

int
cmvdate(d) char * d; {			/* Verify date-time */
    int i;
    if (!d) return(0);
    if ((int)strlen(d) != 17) return(0);
    for (i = 0; i < 8; i++) { if (!isdigit(d[i])) return(0); }
    if (!isdigit(d[9])  || !isdigit(d[10]) ||
	!isdigit(d[12]) || !isdigit(d[13]) ||
	!isdigit(d[15]) || !isdigit(d[16]))
      return(0);
    if (!ckstrchr(" Tt_-:",d[8])) return(0);
    if (d[11] != ':' && d[14] != ':') return(0);
    return(1);
}

/* c m d i f f d a t e  --  Get difference between two date-times */

char *
cmdiffdate(d1,d2) char * d1, * d2; {
    char d1buf[9], d2buf[9];
    char x1buf[18], x2buf[18];
    char * p;

    int hh1 = 0, mm1 = 0, ss1 = 0;
    int hh2 = 0, mm2 = 0, ss2 = 0;
    int hh, mm, ss;
    int sign;
    long jd1, jd2, jd, f1, f2, fx;
    static char result[24], *rp;

    debug(F110,"cmdiffdate d1 A",d1,0);
    debug(F110,"cmdiffdate d2 A",d2,0);

    if (!(p = cmcvtdate(d1,1)))		/* Convert dates to standard format */
      return(NULL);
    ckstrncpy(x1buf,p,18);
    d1 = x1buf;

    if (!(p = cmcvtdate(d2,1)))
      return(NULL);
    ckstrncpy(x2buf,p,18);
    d2 = x2buf;

    debug(F110,"cmdiffdate d1 B",d1,0);
    debug(F110,"cmdiffdate d2 B",d2,0);
    if (!cmvdate(d1) || !cmvdate(d2))
      return(NULL);

    hh1 = atoi(&d1[9]);			/* Get hours, minutes, and seconds */
    mm1 = atoi(&d1[12]);		/* for first date */
    ss1 = atoi(&d1[15]);
    ckstrncpy(d1buf,d1,9);

    hh2 = atoi(&d2[9]);			/* ditto for second date */
    mm2 = atoi(&d2[12]);
    ss2 = atoi(&d2[15]);
    ckstrncpy(d2buf,d2,9);
    
    jd1 = mjd(d1buf);			/* Get the two Julian dates */
    jd2 = mjd(d2buf);
    f1 = ss1 + 60 * mm1 + 3600 * hh1;	/* Convert first time to seconds */

    f2 = ss2 + 60 * mm2 + 3600 * hh2;	/* Ditto for second time */
    debug(F101,"cmdiffdate jd1","",jd1);
    debug(F101,"cmdiffdate f1","",f1);
    debug(F101,"cmdiffdate jd2","",jd2);
    debug(F101,"cmdiffdate f2","",f2);
  
    if (jd2 > jd1 || (jd1 == jd2 && f2 > f1)) {
        sign = -1; 
        if (f1 > f2) {jd2--; f2 += 86400L;}
        jd = jd2 - jd1;
        fx = f2 - f1;
    } else {
        sign = 1;
        if (f2 > f1) {jd1--; f1 += 86400L;}
        jd = jd1 - jd2;
        fx = f1 - f2;
    }
    debug(F111,"cmdiffdate sign jd",sign<0?"-":"+",jd);
    debug(F101,"cmdiffdate fx","",fx);
  
    hh = (int) (fx / 3600L);		/* Convert seconds to hh:mm:ss */

    mm = (int) (fx % 3600L) / 60L;
    ss = (int) (fx % 3600L) % 60L;

    rp = result;			/* Format the result */
    *rp++ = (sign < 0) ? '-' : '+';
    if (jd != 0 && hh+mm+ss == 0) {
	sprintf(rp,"%ldd",jd);
    } else if (jd == 0) {
	if (ss == 0)
	  sprintf(rp,"%d:%02d",hh,mm);
	else
	  sprintf(rp,"%d:%02d:%02d",hh,mm,ss);
    } else {
	if (ss == 0)
	  sprintf(rp,"%ldd%d:%02d",jd,hh,mm);
	else
	  sprintf(rp,"%ldd%d:%02d:%02d",jd,hh,mm,ss);
    }
    debug(F110,"cmdiffdate result",result,0);
    return((char *)result);
}

#ifndef NOSPL
/* s h u f f l e d a t e  --  Rearrange date string */

/*
  Call with:
    A date string in standard format: yyyymmdd hh:mm:ss (time optional).
    Options:
      1: Reformat date to yyyy-mmm-dd (mmm = English month abbreviation).
      2: Reformat date to dd-mmm-yyyy (mmm = English month abbreviation).
      3: Reformat as numeric yyyymmddhhmmss.
      4: Reformat in asctime() format Sat Nov 26 11:10:34 2005
    Returns:
      Pointer to result if args valid, otherwise original arg pointer.
*/
char *
shuffledate(p,opt) char * p; int opt; {
    extern char * wkdays[];
    int len;
    char ibuf[32];
    static char obuf[48];
    char c;
    int yy, dd, mm;

    if (!p) p = "";
    if (!*p) p = ckdate();
    if (opt < 1 || opt > 4)
      return(p);
    len = strlen(p);
    if (len < 8 || len > 31) return(p);
    if (opt == 4) {			/* Asctime format (26 Nov 2005) */
	char c, * s;
	long z; int k;
	ckstrncpy(ibuf,p,31);
	k = len;
	while (k >= 0 && ibuf[k] == CR || ibuf[k] == LF)
	  ibuf[k--] = NUL;
	while (k >= 0 && ibuf[k] == SP || ibuf[k] == HT)
	  ibuf[k--] = NUL;
	if (k < 9) ckstrncpy(&ibuf[8]," 00:00:00",9);
	p = ibuf;
        z = mjd(p);                     /* Convert to modified Julian date */
        z = z % 7L;
        if (z < 0) {
            z = 0 - z;
            k = 6 - ((int)z + 3) % 7;
        } else {
            k = ((int)z + 3) % 7;	/* Day of week */
        }
	s = wkdays[k];
        obuf[0] = s[0];			/* Day of week */
        obuf[1] = s[1];
        obuf[2] = s[2];
        obuf[3] = SP;			/* Space */
	c = p[6];
        p[6] = NUL;
	mm = atoi(&ibuf[4]);		/* Month */
	s = moname[mm-1];		/* Name of month */
	p[6] = c;

        obuf[4] = s[0];			/* Month */
        obuf[5] = s[1];
        obuf[6] = s[2];
        obuf[7] = SP;			/* Space */
	if (p[6] == '0')		/* Date of month */
	  obuf[8] = SP;
	else
	  obuf[8] = p[6];
        obuf[9] = p[7];
	ckstrncpy(&obuf[10],&p[8],10);	/* Time */
        obuf[19] = SP;			/* Space */
	obuf[20] = p[0];		/* Year */
	obuf[21] = p[1];
	obuf[22] = p[2];
	obuf[23] = p[3];
	obuf[24] = NUL;
	return((char *)obuf);
    }
    if (opt == 3) {
	ckstrncpy(obuf,p,48);
	/* yyyymmdd hh:mm:ss */
	/* 01234567890123456 */
	/* yyyymmddhhmmss    */
	obuf[8] = obuf[9];
	obuf[9] = obuf[10];
	obuf[10] = obuf[12];
	obuf[11] = obuf[13];
	obuf[12] = obuf[15];
	obuf[13] = obuf[16];
	obuf[14] = NUL;
	return((char *)obuf);
    }
    ckstrncpy(ibuf,p,32);
    c = ibuf[4];			/* Warning: not Y10K compliant */
    ibuf[4] = NUL;
    if (!rdigits(ibuf))
      return(p);
    yy = atoi(ibuf);
    if (yy < 1 || yy > 9999)
      return(p);
    ibuf[4] = c;
    c = ibuf[6];
    ibuf[6] = NUL;
    if (!rdigits(&ibuf[4]))
      return(p);
    mm = atoi(&ibuf[4]);
    if (mm < 1 || mm > 12)
      return(p);
    ibuf[6] = c;
    c = ibuf[8];
    ibuf[8] = NUL;
    if (!rdigits(&ibuf[6]))
      return(p);
    dd = atoi(&ibuf[6]);
    ibuf[8] = c;
    if (dd < 1 || mm > 31)
      return(p);
    /* IGNORE WARNINGS ABOUT moname[] REFS OUT OF RANGE - it's prechecked. */
    switch (opt) {
      case 1:
	sprintf(obuf,"%04d-%s-%02d%s",yy,moname[mm-1],dd,&ibuf[8]);
	break;
      case 2:
	sprintf(obuf,"%02d-%s-%04d%s",dd,moname[mm-1],yy,&ibuf[8]);
    }
    return((char *)obuf);
}
#endif	/* NOSPL */

/*  C K C V T D A T E  --  Like cmcvtdate(), but returns string.  */
/*  For use by date-related functions */
/*  See calling conventions for cmcvtdate() above. */

char *
ckcvtdate(p,t) char * p; int t; {
    char * s;
    if (!(s = cmcvtdate(p,t)))
      return("<BAD_DATE_OR_TIME>");	/* \fblah() error message */
    else
      return(s);
}


/*  C M D A T E  --  Parse a date and/or time  */

/*
  Accepts date in various formats.  If the date is recognized,
  this routine returns 0 or greater with the result string pointer
  pointing to a buffer containing the date as "yyyymmdd hh:mm:ss".
*/
int
cmdate(xhlp,xdef,xp,quiet,f) char *xhlp, *xdef, **xp; int quiet; xx_strp f; {
    int x, rc;
    char *o, *s, *zq, *dp;

    cmfldflgs = 0;
    if (!xhlp) xhlp = "";
    if (!xdef) xdef = "";
    if (!*xhlp) xhlp = "Date and/or time";
    *xp = "";

    rc = cmfld(xhlp,xdef,&s,(xx_strp)0);
    debug(F101,"cmdate cmfld rc","",rc);
    if (rc < 0)
      return(rc);
    debug(F110,"cmdate 1",s,0);
    o = s;				/* Remember what they typed. */
    s = brstrip(s);
    debug(F110,"cmdate 2",s,0);

    x = 0;
    if (f) {				/* If a conversion function is given */
	char * pp;
	zq = atxbuf;			/* do the conversion. */
	pp = atxbuf;
	atxn = CMDBL;
	if ((x = (*f)(s,&zq,&atxn)) < 0) return(-2);
	if (!*pp)
	  pp = xdef;
	if (setatm(pp,0) < 0) {
	    if (!quiet) printf("?Evaluated date too long\n");
	    return(-9);
	}
	s = atxbuf;
    }
    dp = cmcvtdate(s,1);
    if (!dp) {
	if (!quiet) printf("?%s\n",cmdatemsg);
	return(-9);
    }
    *xp = dp;
    return(0);
}

#ifdef CK_RECALL			/* Command-recall functions */

/*  C M R I N I  --  Initialize or change size of command recall buffer */

int
cmrini(n) int n; {
    int i;
    if (recall && in_recall) {		/* Free old storage, if any */
	for (i = 0; i < cm_recall; i++) {
	    if (recall[i]) {
		free(recall[i]);
		recall[i] = NULL;
	    }
	}
	free(recall);
	recall = NULL;
    }
    cm_recall = n;			/* Set new size */
    rlast = current = -1;		/* Initialize pointers */
    if (n > 0) {
	recall = (char **)malloc((cm_recall + 1) * sizeof(char *));
	if (!recall)
	  return(1);
	for (i = 0; i < cm_recall; i++) {
	    recall[i] = NULL;
	}
	in_recall = 1;			/* Recall buffers init'd */
    }
    return(0);
}

/*  C M A D D N E X T  --  Force addition of next command */

VOID
cmaddnext() {
    if (on_recall && in_recall) {	/* Even if it doesn't come */
	force_add = 1;			/* from the keyboard */
	newcmd = 1;
	no_recall = 0;
    }
}

/*  C M G E T C M D  --  Find most recent matching command  */

char *
cmgetcmd(s) char * s; {
    int i;
    for (i = current; i >= 0; i--) {	/* Search backward thru history list */
	if (!recall[i]) continue;	/* This one's null, skip it */
	if (ckmatch(s,recall[i],0,1))	/* Match? */
	  return(recall[i]);		/* Yes, return pointer */
    }
    return(NULL);			/* No match, return NULL pointer */
}
#endif /* CK_RECALL */

/*  A D D C M D  --  Add a command to the recall buffer  */

VOID
addcmd(s) char * s; {
    int len = 0, nq = 0;
    char * p;
#ifdef CKLEARN
    extern int learning;
#endif /* CKLEARN */

    if (xcmdsrc)			/* Only for interactive commands */
      return;

    if (!newcmd)			/* The command has been here already */
      return;				/* so ignore it. */
    newcmd = 0;				/* It's new but do this only once. */

    if (!s) s = cmdbuf;
    if (s[0])
      len = strlen(s);

    if (len < 1)			/* Don't save empty commands */
      return;

    p = s;
    while (*p) { if (*p++ == '?') nq++; } /* Count question marks */

#ifdef CKLEARN
    if (learning)			/* If a learned script is active */
      learncmd(s);			/* record this command. */
#endif /* CKLEARN */

    debug(F010,"CMD(P)",s,0);		/* Maybe record it in the debug log */

#ifdef CKSYSLOG
    if (ckxlogging) {			/* Maybe record it in syslog */
	if (ckxsyslog >= SYSLG_CX || ckxsyslog >= SYSLG_CM)
	  cksyslog(SYSLG_CX, 1, "command", s, NULL);
    }
#endif /* CKSYSLOG */

#ifdef CK_RECALL
    last_recall = 0;

    if (on_recall &&			/* Command recall is on? */
	cm_recall > 0 &&		/* Recall buffer size is > 0? */
	!no_recall) {			/* Not not saving this command? */

	if (!force_add && rlast > -1)	/* If previous command was identical */
	  if (!strcmp(s,recall[rlast])) /* don't add another copy */
	    return;

	force_add = 0;			/* Reset now in case it was set */

        if (rlast >= cm_recall - 1) {	/* Recall buffer full? */
	    int i;
	    if (recall[0]) {		/* Discard oldest command */
		free(recall[0]);
		recall[0] = NULL;
	    }
	    for (i = 0; i < rlast; i++) {  /* The rest */
		recall[i] = recall[i+1];   /* move back */
	    }
	    rlast--;			/* Now we have one less */
	}
        rlast++;			/* Index of last command in buffer */
	current = rlast;		/* Also now the current command */
	if (current >= cm_recall) {	/* Shouldn't happen */
	    printf("?Command history error\n");	/* but if it does */
	    on_recall = 0;		        /* turn off command saving */
#ifdef COMMENT
	} else if (nq > 0) {		/* Have at least one question mark */
	    recall[current] = malloc(len+nq+1);
	    if (recall[current]) {
		p = recall[current];
		while (*s) {
		    if (*s == '?')
		      *p++ = '\\';
		    *p++ = *s++;
		}
		*p = NUL;
	    }
#endif /* COMMENT */
	} else {			/* Normal case, just copy */
	    recall[current] = malloc(len+1);
	    if (recall[current])
	      ckstrncpy(recall[current],s,len+1);
	}
    }
#endif /* CK_RECALL */
}


#ifdef CK_RECALL

/* C M H I S T O R Y */

VOID
cmhistory() {
    int i, lc = 1;
    for (i = 0; i <= current; i++) {
	printf(" %s\n", recall[i]);
	if (++lc > (cmd_rows - 2)) {	/* Screen full? */
	    if (!askmore())		/* Do more-prompting... */
	      break;
	    else
	      lc = 0;
	}
    }
}

int
savhistory(s,disp) char *s; int disp; {
    FILE * fp;
    int i;

    fp = fopen(s, disp ? "a" : "w");
    if (!fp) {
	perror(s);
	return(0);
    }
    for (i = 0; i <= current; i++)
      fprintf(fp,"%s\n", recall[i]);
    fclose(fp);
    return(1);
}
#endif /* CK_RECALL */

#ifdef COMMENT
/* apparently not used */
int
cmgetlc(s) char * s; {			/* Get leading char */
    char c;
    while ((c = *s++) <= SP) {
	if (!c)
	  break;
    }
    return(c);
}
#endif /* COMMENT */


/*  C M C F M  --  Parse command confirmation (end of line)  */

/*
 Returns
   -2: User typed anything but whitespace or newline
   -1: Reparse needed
    0: Confirmation was received
*/
int
cmcfm() {
    int x, xc;
    debug(F101,"cmcfm: cmflgs","",cmflgs);
    debug(F110,"cmcfm: atmbuf",atmbuf,0);
    inword = xc = cc = 0;

    setatm("",0);			/* (Probably unnecessary) */

    while (cmflgs != 1) {
        x = gtword(0);
        xc += cc;

        switch (x) {
	  case -9:
	    printf("Command or field too long\n");
	  case -4:			/* EOF */
	  case -2:
	  case -1:
	    return(x);
	  case 1:			/* End of line */
	    if (xc > 0) {
		if (xcmfdb) {
		    return(-6);
		} else {
		    printf("?Not confirmed - %s\n",atmbuf);
		    return(-9);
		}
	    } else
	      break;			/* Finish up below */
	  case 2:			/* ESC */
	    if (xc == 0) {
		bleep(BP_WARN);
		continue;		/* or fall thru. */
	    }
	  case 0:			/* Space */
	    if (xc == 0)		/* If no chars typed, continue, */
	      continue;			/* else fall thru. */
	    /* else fall thru... */

	  case 3:			/* Question mark */
	    if (xc > 0) {
		if (xcmfdb) {
		    return(-6);
		} else {
		    printf("?Not confirmed - %s\n",atmbuf);
		    return(-9);
		}
	    }
	    printf(
	       "\n Press the Return or Enter key to confirm the command\n");
	    printf("%s%s",cmprom,cmdbuf);
	    fflush(stdout);
	    continue;
	}
    }
    debok = 1;
    return(0);
}


/* The following material supports chained parsing functions. */
/* See ckucmd.h for FDB and OFDB definitions. */

struct OFDB cmresult = {		/* Universal cmfdb result holder */
    NULL,				/* Address of succeeding FDB struct */
    0,					/* Function code */
    NULL,				/* String result */
    0,					/* Integer result */
    (CK_OFF_T)0				/* Wide result */
};

VOID
cmfdbi(p,fc,s1,s2,s3,n1,n2,f,k,nxt)	/* Initialize an FDB */
    struct FDB * p;
    int fc;
    char * s1, * s2, * s3;
    int n1, n2;
    xx_strp f;
    struct keytab * k;
    struct FDB * nxt; {

    p->fcode = fc;
    p->hlpmsg = s1;
    p->dflt = s2;
    p->sdata = s3;
    p->ndata1 = n1;
    p->ndata2 = n2;
    p->spf = f;
    p->kwdtbl = k;
    p->nxtfdb = nxt;
}

/*  C M F D B  --  Parse a field with several possible functions  */

int
cmfdb(fdbin) struct FDB * fdbin; {
#ifndef NOSPL
    extern int x_ifnum;                 /* IF NUMERIC - disables warnings */
#endif /* NOSPL */
    struct FDB * in = fdbin;
    struct OFDB * out = &cmresult;
    int x = 0, n, r;
    CK_OFF_T w = (CK_OFF_T)0;
    char *s, *xp, *m = NULL;
    int errbits = 0;

    xp = bp;

    out->fcode = -1;			/* Initialize output struct */
    out->fdbaddr = NULL;
    out->sresult = NULL;
    out->nresult = 0;
/*
  Currently we make one trip through the FDBs.  So if the user types Esc or
  Tab at the beginning of a field, only the first FDB is examined for a
  default.  If the user types ?, help is given only for one FDB.  We should
  search through the FDBs for all matching possibilities -- and in particular
  display the pertinent context-sensitive help for each function, rather than
  the only the first one that works, and then rewind the FDB pointer so we
  are not locked out of the earlier ones.
*/
    cmfldflgs = 0;
    while (1) {				/* Loop through the chain of FDBs */
	nomsg = 1;
	xcmfdb = 1;
	s = NULL;
	n = 0;
	debug(F101,"cmfdb in->fcode","",in->fcode);
	switch (in->fcode) {		/* Current parsing function code */
	  case _CMNUM:
	    r = in->ndata1;
	    if (r != 10 && r != 8) r = 10;
#ifndef NOSPL
            x_ifnum = 1;                /* Disables warning messages */
#endif /* NOSPL */
	    x = cmnum(in->hlpmsg,in->dflt,r,&n,in->spf);
#ifndef NOSPL
            x_ifnum = 0;
#endif /* NOSPL */
	    debug(F101,"cmfdb cmnum","",x);
	    if (x < 0) errbits |= 1;
	    break;
	  case _CMNUW:			/* Wide cmnum - 24 Dec 2005 */
	    r = in->ndata1;
	    if (r != 10 && r != 8) r = 10;
#ifndef NOSPL
            x_ifnum = 1;                /* Disables warning messages */
#endif /* NOSPL */
	    x = cmnumw(in->hlpmsg,in->dflt,r,&w,in->spf);
#ifndef NOSPL
            x_ifnum = 0;
#endif /* NOSPL */
	    debug(F101,"cmfdb cmnumw","",w);
	    if (x < 0) errbits |= 1;
	    break;
	  case _CMOFI:
	    x = cmofi(in->hlpmsg,in->dflt,&s,in->spf);
	    debug(F101,"cmfdb cmofi","",x);
	    if (x < 0) errbits |= 2;
	    break;
	  case _CMIFI:
	    x = cmifi2(in->hlpmsg,
		       in->dflt,
		       &s,
		       &n,
		       in->ndata1,
		       in->sdata,
		       in->spf,
		       in->ndata2
		       );
	    debug(F101,"cmfdb cmifi2 x","",x);
	    debug(F101,"cmfdb cmifi2 n","",n);
	    if (x < 0) errbits |= 4;
	    break;
	  case _CMFLD:
	    cmfldflgs = in->ndata1;
	    x = cmfld(in->hlpmsg,in->dflt,&s,in->spf);
	    debug(F101,"cmfdb cmfld","",x);
	    if (x < 0) errbits |= 8;
	    break;
	  case _CMTXT:
	    x = cmtxt(in->hlpmsg,in->dflt,&s,in->spf);
	    debug(F101,"cmfdb cmtxt","",x);
	    if (x < 0) errbits |= 16;
	    break;
	  case _CMKEY:
	    x = cmkey2(in->kwdtbl,
		       in->ndata1,
		       in->hlpmsg,in->dflt,in->sdata,in->spf,in->ndata2);
	    debug(F101,"cmfdb cmkey","",x);
	    if (x < 0) errbits |= ((in->ndata2 & 4) ? 32 : 64);
	    break;
	  case _CMCFM:
	    x = cmcfm();
	    debug(F101,"cmfdb cmcfm","",x);
	    if (x < 0) errbits |= 128;
	    break;
	  default:
	    debug(F101,"cmfdb - unexpected function code","",in->fcode);
	    printf("?cmfdb - unexpected function code: %d\n",in->fcode);
	}
	debug(F101,"cmfdb x","",x);
	debug(F101,"cmfdb cmflgs","",cmflgs);
	debug(F101,"cmfdb crflag","",crflag);
	debug(F101,"cmfdb qmflag","",qmflag);
	debug(F101,"cmfdb esflag","",esflag);

	if (x > -1) {			/* Success */
	    out->fcode = in->fcode;	/* Fill in output struct */
	    out->fdbaddr = in;
	    out->sresult = s;
	    out->nresult = (in->fcode == _CMKEY) ? x : n;
	    out->wresult = w;
	    out->kflags = (in->fcode == _CMKEY) ? cmkwflgs : 0;
	    debug(F111,"cmfdb out->nresult",out->sresult,out->nresult);
	    debug(F111,"cmfdb out->wresult",out->sresult,out->wresult);
	    nomsg = 0;
	    xcmfdb = 0;
	    /* debug(F111,"cmfdb cmdbuf & crflag",cmdbuf,crflag); */
	    if (crflag) {
		cmflgs = 1;
	    }
	    return(x);			/* and return */
	}
	in = in->nxtfdb;		/* Failed, get next parsing function */
	nomsg = 0;
	xcmfdb = 0;
	if (!in) {			/* No more */
	    debug(F101,"cmfdb failure x","",x);
	    debug(F101,"cmfdb failure errbits","",errbits);
	    if (x == -6)
	      x = -9;
	    if (x == -9) {
#ifdef CKROOT
		if (ckrooterr)
		  m = "Off Limits";
		else
#endif /* CKROOT */
		/* Make informative messages for a few common cases */
		switch (errbits) {
		  case 4+32: m = "Does not match filename or switch"; break;
		  case 4+64: m = "Does not match filename or keyword"; break;
		  case 1+32: m = "Not a number or valid keyword"; break;
		  case 1+64: m = "Not a number or valid switch"; break;
		  default: m = "Not valid in this position";
		}
		printf("?%s: \"%s\"\n",m, atmbuf);
	    }
	    return(x);
	}
	if (x != -2 && x != -6 && x != -9 && x != -3) /* Editing or somesuch */
	  return(x);			/* Go back and reparse */
	pp = np = bp = xp;		/* Back up pointers */
	cmflgs = -1;			/* Force a reparse */

#ifndef NOSPL
	if (!askflag) {			/* If not executing ASK-class cmd... */
#endif /* NOSPL */
	    if (crflag) {		/* If CR was typed, put it back */
		pushc = LF;		/* But as a linefeed */
	    } else if (qmflag) {	/* Ditto for Question mark */
		pushc = '?';
	    } else if (esflag) {	/* and Escape or Tab */
		pushc = ESC;
	    }
#ifndef NOSPL
	}
#endif /* NOSPL */
    }
}

/*
   C M I O F I  --  Parse an input file OR the name of a nonexistent file.

   Replaces the commented-out version above.  This one actually works and
   has the expected straightforward interface.
*/
int
cmiofi(xhlp,xdef,xp,wild,f) char *xhlp, *xdef, **xp; int *wild; xx_strp f; {
    int x;
    struct FDB f1, f2;
    cmfdbi(&f1,_CMIFI,xhlp,xdef,"",0,0,f,NULL,&f2);
    cmfdbi(&f2,_CMOFI,"","","",0,0,f,NULL,NULL);
    x = cmfdb(&f1);
    if (x < 0) {
	if (x == -3) {
	    x = -9;
	    printf("?Filename required\n");
	}
    }
    *wild = cmresult.nresult;
    *xp = cmresult.sresult;
    return(x);
}

/*  G T W O R D  --  Gets a "word" from the command input stream  */

/*
Usage: retcode = gtword(brk);
  brk = 0 for normal word breaks (space, CR, Esc, ?)
  brk = 1 to add ':' and '=' (for parsing switches).  These characters
        act as break characters only if the first character of the field
        is slash ('/'), i.e. switch introducer.
  brk = 4 to not strip comments (used only for "help #" and "help ;").

Returns:
-10 Timelimit set and timed out
 -9 if input was too long
 -4 if end of file (e.g. pipe broken)
 -3 if null field
 -2 if command buffer overflows
 -1 if user did some deleting
  0 if word terminates with SP or tab
  1 if ... CR
  2 if ... ESC
  3 if ... ? (question mark)
  4 if ... : or = and called with brk != 0

With:
  pp pointing to beginning of word in buffer
  bp pointing to after current position
  atmbuf containing a copy of the word
  cc containing the number of characters in the word copied to atmbuf
*/

int
ungword() {				/* Unget a word */
    debug(F101,"ungword cmflgs","",cmflgs);
    if (ungw) return(0);
    cmfsav = cmflgs;
    ungw = 1;
    cmflgs = 0;
    return(0);
}

/* Un-un-get word.  Undo ungword() if it has been done. */

VOID
unungw() {
    debug(F010,"unungw atmbuf",atmbuf,0);
    if (ungw) {
	ungw = 0;
	cmflgs = cmfsav;
	atmbuf[0] = NUL;
    }
}

static int
gtword(brk) int brk; {
    int c;                              /* Current char */
    int quote = 0;                      /* Flag for quote character */
    int echof = 0;                      /* Flag for whether to echo */
    int comment = 0;			/* Flag for in comment */
    char *cp = NULL;			/* Comment pointer */
    int eintr = 0;			/* Flag for syscall interrupted */
    int bracelvl = 0;			/* nested brace counter [jrs] */
    int iscontd = 0;			/* Flag for continuation */
    int realtty = 0;			/* Stdin is really a tty */
    char firstnb  = NUL;
    char lastchar = NUL;
    char prevchar = NUL;
    char lbrace, rbrace;
    int dq = 0;				/* Doublequote flag */
    int dqn = 0;			/* and count */
    int isesc = 0;

#ifdef RTU
    extern int rtu_bug;
#endif /* RTU */

#ifdef IKSD
    extern int inserver;
#endif /* IKSD */
    extern int kstartactive;

#ifdef datageneral
    extern int termtype;                /* DG terminal type flag */
    extern int con_reads_mt;            /* Console read asynch is active */
    if (con_reads_mt) connoi_mt();      /* Task would interfere w/cons read */
#endif /* datageneral */

#ifdef COMMENT
#ifdef DEBUG
    if (deblog) {
	debug(F101,"gtword brk","",brk);
	debug(F101,"gtword cmfldflgs","",cmfldflgs);
	debug(F101,"gtword swarg","",swarg);
	debug(F101,"gtword dpx","",dpx);
	debug(F101,"gtword echof","",echof);
#ifndef NOSPL
	debug(F101,"gtword askflag","",askflag);
	debug(F101,"gtword timelimit","",timelimit);
#ifndef NOLOCAL
#ifndef NOXFER
#ifdef CK_AUTODL
	debug(F101,"gtword cmdadl","",cmdadl);
#endif /* CK_AUTODL */
#endif /* NOXFER */
#endif /* NOLOCAL */
#endif /* NOSPL */
    }
#endif /* DEBUG */
#endif /* COMMENT */

    realtty = is_a_tty(0);		/* Stdin is really a tty? */

    if (cmfldflgs & 1) {
	lbrace = '(';
	rbrace = ')';
    } else {
	lbrace = '{';
	rbrace = '}';
    }
    crflag = 0;
    qmflag = 0;
    esflag = 0;

    if (swarg) {			/* No leading space for switch args */
	inword = 1;
	swarg = 0;
    }
    if (ungw) {				/* Have a word saved? */
#ifdef M_UNGW
	/* Experimental code to allow ungetting multiple words. */
	/* See comments in ckmkey2() above. */
	int x;
	if (np > pp) pp = np;
	while (*pp == SP) pp++;
	if (!*pp) {
	    ungw = 0;
	    cmflgs = cmfsav;
	} else {
	    if ((x = setatm(pp,2)) < 0) {
		printf("?Saved word too long\n");
		return(-9);
	    }
	    if (pp[x] >= SP) {
		char *p2;
		p2 = pp;
		p2 += x;
		while (*p2 == SP) p2++;
		if (*p2) {
		    np = p2;
		    ungword();
		}
	    } else {
		ungw = 0;
		cmflgs = cmfsav;
		debug(F010,"gtword ungw return atmbuf",atmbuf,0);
	    }
	}
	return(cmflgs);
#else
	/*
	   You would think the following should be:
             while (*pp == SP) pp++;
           but you would be wrong -- making this change breaks GOTO.
        */
	while (*pp++ == SP) ;
	if (setatm(pp,2) < 0) {
	    printf("?Saved word too long\n");
	    return(-9);
	}
	ungw = 0;
	cmflgs = cmfsav;
	debug(F010,"gtword ungw return atmbuf",atmbuf,0);
	return(cmflgs);
#endif /* M_UNGW */
    }
    pp = np;                            /* Start of current field */

#ifdef COMMENT
#ifdef DEBUG
    if (deblog) {
	debug(F110,"gtword cmdbuf",cmdbuf,0);
	debug(F110,"gtword bp",bp,0);
	debug(F110,"gtword pp",pp,0);
    }
#endif /* DEBUG */
#endif /* COMMENT */
    {
	/* If we are reparsing we have to recount any braces or doublequotes */
	char * p = pp;
	char c;
	if (*p == '"')
	  dq++;
	while ((c = *p++))
	  if (c == lbrace)
	    bracelvl++;
	  else if (c == rbrace)
	    bracelvl--;
	  else if (dq && c == '"')
	    dqn++;
    }
    while (bp < cmdbuf+CMDBL) {         /* Big get-a-character loop */
	echof = 0;			/* Assume we don't echo because */
	chsrc = 0;			/* character came from reparse buf. */
#ifdef BS_DIRSEP
CMDIRPARSE:
#endif /* BS_DIRSEP */

	c = *bp;
        if (!c) {			/* If no char waiting in reparse buf */
	    if ((dpx
#ifndef NOSPL
		 || echostars
#endif /* NOSPL */
		 ) && (!pushc
#ifndef NOSPL
			|| askflag
#endif /* NOSPL */
			))		/* Get from tty, set echo flag */
	      echof = 1;
	    c = cmdgetc(timelimit);	/* Read a command character. */
#ifdef DEBUG
	    debug(F101,"gtword c","",c);
#endif /* DEBUG */

	    if (timelimit && c < -1) {	/* Timed out */
		return(-10);
	    }

#ifndef NOXFER
/*
  The following allows packet recognition in the command parser.
  Presently it works only for Kermit packets, and if our current protocol
  happens to be anything besides Kermit, we simply force it to Kermit.
  We don't use the APC mechanism here for mechanical reasons, and also
  because this way, it works even with minimally configured interactive
  versions.  Add Zmodem later...
*/
#ifdef CK_AUTODL
	    if ((!local && cmdadl)	/* Autodownload enabled? */
#ifdef IKS_OPTION
		|| TELOPT_SB(TELOPT_KERMIT).kermit.me_start
#endif /* IKS_OPTION */
		) {
		int k;
		k = kstart((CHAR)c);	/* Kermit S or I packet? */
		if (k) {
		    int ksign = 0;
		    if (k < 0) {	/* Minus-Protocol? */
#ifdef NOSERVER
			goto noserver;	/* Need server mode for this */
#else
			ksign = 1;	/* Remember */
			k = 0 - k;	/* Convert to actual protocol */
			justone = 1;	/* Flag for protocol module */
#endif /* NOSERVER */
		    } else
		      justone = 0;
		    k--;		/* Adjust kstart's return value */
		    if (k == PROTO_K) {
			extern int protocol, g_proto;
			extern CHAR sstate;
			g_proto = protocol;
			protocol = PROTO_K; /* Crude... */
			sstate = ksign ? 'x' : 'v';
			cmdbuf[0] = NUL;
			return(-3);
		    }
		}
	    }
#ifdef NOSERVER
	  noserver:
#endif /* NOSERVER */
#endif /* CK_AUTODL */
#endif /* NOXFER */

	    chsrc = 1;			/* Remember character source is tty. */
	    brkchar = c;

#ifdef IKSD
            if (inserver && c < 0) {    /* End of session? */
                debug(F111,"gtword c < 0","exiting",c);
                return(-4);             /* Cleanup and terminate */
            }
#endif /* IKSD */

#ifdef OS2
           if (c < 0) {			/* Error */
	       if (c == -3) {		/* Empty word? */
		   if (blocklvl > 0)	/* In a block */
		     continue;		/* so keep looking for block end */
		   else
		     return(-3);	/* Otherwise say we got nothing */
	       } else {			/* Not empty word */
		   return(-4);		/* So some kind of i/o error */
	       }
           }
#else
#ifdef MAC
	   if (c == -3)			/* Empty word... */
	     if (blocklvl > 0)
	       continue;
	     else
	       return(-3);
#endif /* MAC */
#endif /* OS2 */
	   if (c == EOF) {		/* This can happen if stdin not tty. */
#ifdef EINTR
/*
  Some operating and/or C runtime systems return EINTR for no good reason,
  when the end of the standard input "file" is encountered.  In cases like
  this, we get into an infinite loop; hence the eintr counter, which is reset
  to 0 upon each call to this routine.
*/
		debug(F101,"gtword EOF","",errno);
		if (errno == EINTR && ++eintr < 4) /* When bg'd process is */
		  continue;		/* fg'd again. */
#endif /* EINTR */
		return(-4);
	    }
	    c &= cmdmsk;		/* Strip any parity bit */
	}				/* if desired. */

/* Now we have the next character */

	isesc = (c == ESC);		/* A real ESC? */

	if (!firstnb && c > SP) {	/* First nonblank */
	    firstnb = c;
	    if (c == '"')		/* Starts with doublequote */
	      dq = 1;
	}
	if (c == '"')			/* Count doublequotes */
	  dqn++;

	if (quote && (c == CR || c == LF)) { /* Enter key following quote */
	    *bp++ = CMDQ;		/* Double it */
	    *bp = NUL;
	    quote = 0;
	}
        if (quote == 0) {		/* If this is not a quoted character */
	    switch (c) {
	      case CMDQ:		/* Got the quote character itself */
		if (!comment && quoting)
		  quote = 1;		/* Flag it if not in a comment */
		break;
	      case FF:			/* Formfeed. */
                c = NL;                 /* Replace with newline */
		cmdclrscn();		/* Clear the screen */
		break;
	      case HT:			/* Horizontal Tab */
		if (comment)		/* If in comment, */
		  c = SP;		/* substitute space */
		else			/* otherwise */
		  c = ESC;		/* substitute ESC (for completion) */
		break;
	      case ';':			/* Trailing comment */
	      case '#':
		if (! (brk & 4) ) {	/* If not keeping comments */
		    if (inword == 0 && quoting) { /* If not in a word */
			comment = 1;	/* start a comment. */
			cp = bp;	/* remember where it starts. */
		    }
		}
		break;
	    }
	    if (!kstartactive &&	/* Not in possible Kermit packet */
		!comment && c == SP) {	/* Space not in comment */
                *bp++ = (char) c;	/* deposit in buffer if not already */
		/* debug(F101,"gtword echof 2","",echof); */
#ifdef BEBOX
                if (echof) {
		    cmdecho((char) c, 0); /* Echo what was typed. */
                    fflush(stdout);
                    fflush(stderr);
                }
#else
                if (echof) {
		    cmdecho((char) c, 0); /* Echo what was typed. */
		    if (timelimit)
		      fflush(stdout);
		}
#endif /* BEBOX */
                if (inword == 0) {      /* If leading, gobble it. */
                    pp++;
                    continue;
                } else {                /* If terminating, return. */
		    if ((!dq && ((*pp != lbrace) || (bracelvl == 0))) ||
			(dq && dqn > 1 && *(bp-2) == '"')) {
			np = bp;
			cmbptr = np;
			if (setatm(pp,0) < 0) {
			    printf("?Field too long error 1\n");
			    debug(F111,"gtword too long #1",pp,strlen(pp));
			    return(-9);
			}
			brkchar = c;
			inword = cmflgs = 0;
			return(0);
		    }
                    continue;
                }
            }
            if (c == lbrace) {
		bracelvl++;
		/* debug(F101,"gtword bracelvl++","",bracelvl); */
	    }
            if (c == rbrace && bracelvl > 0) {
                bracelvl--;
		/* debug(F101,"gtword bracelvl--","",bracelvl); */
                if (linebegin)
		  blocklvl--;
            }
	    if ((c == '=' || c == ':') &&
		/* ^^^ */
		!kstartactive && !comment && brk /* && (firstnb == '/') */
		) {
                *bp++ = (char) c;	/* Switch argument separator */
		/* debug(F111,"gtword switch argsep",cmdbuf,brk); */
#ifdef BEBOX
                if (echof) {
		    cmdecho((char) c, 0); /* Echo what was typed. */
                    fflush(stdout);
                    fflush(stderr);
                }
#else
		if (echof) {
		    cmdecho((char) c, 0); /* Echo what was typed. */
		    if (timelimit)
		      fflush(stdout);
		}
#endif /* BEBOX */
		if ((*pp != lbrace) || (bracelvl == 0)) {
		    np = bp;
		    cmbptr = np;
		    if (setatm(pp,2) < 0) { /* ^^^ */
			printf("?Field too long error 1\n");
			debug(F111,"gtword too long #1",pp,strlen(pp));
			return(-9);
		    }
		    inword = cmflgs = 0;
		    brkchar = c;
		    return(4);
		}
            }
            if (c == LF || c == CR) {	/* CR or LF. */
		if (echof) {
                    cmdnewl((char)c);	/* echo it. */
#ifdef BEBOX
                    fflush(stdout);
                    fflush(stderr);
#endif /* BEBOX */
                }
		{
		    /* Trim trailing comment and whitespace */
		    char *qq;
		    if (comment) {	/* Erase comment */
			while (bp >= cp) /* Back to comment pointer */
			  *bp-- = NUL;
			bp++;
			pp = bp;	/* Adjust other pointers */
			inword = 0;	/* and flags */
			comment = 0;
			cp = NULL;
		    }
		    qq = inword ? pp : (char *)cmdbuf;
		    /* Erase trailing whitespace */
		    while (bp > qq && (*(bp-1) == SP || *(bp-1) == HT)) {
			bp--;
			/* debug(F000,"erasing","",*bp); */
			*bp = NUL;
		    }
		    lastchar = (bp > qq) ? *(bp-1) : NUL;
		    prevchar = (bp > qq+1) ? *(bp-2) : NUL;
		}
		if (linebegin && blocklvl > 0) /* Blank line in {...} block */
		  continue;

		linebegin = 1;		/* At beginning of next line */
		iscontd = prevchar != CMDQ &&
		  (lastchar == '-' || lastchar == lbrace);
		debug(F101,"gtword iscontd","",iscontd);

                if (iscontd) {		/* If line is continued... */
                    if (chsrc) {	/* If reading from tty, */
                        if (*(bp-1) == lbrace) { /* Check for "begin block" */
                            *bp++ = SP;	/* Insert a space for neatness */
                            blocklvl++;	/* Count block nesting level */
                        } else {	/* Or hyphen */
			    bp--;	/* Overwrite the hyphen */
                        }
                        *bp = NUL;	/* erase the dash, */
                        continue;	/* and go back for next char now. */
                    }
		} else if (blocklvl > 0) { /* No continuation character */
		    if (chsrc) {	/* But we're in a "block" */
			*bp++ = ',';	/* Add comma */
			*bp = NUL;
			continue;
		    }
		} else {		/* No continuation, end of command. */
		    *bp = NUL;		/* Terminate the command string. */
		    if (comment) {	/* If we're in a comment, */
			comment = 0;	/* Say we're not any more, */
			*cp = NUL;	/* cut it off. */
		    }
		    np = bp;		/* Where to start next field. */
		    cmbptr = np;
		    if (setatm(pp,0) < 0) { /* Copy field to atom buffer */
			debug(F111,"gtword too long #2",pp,strlen(pp));
			printf("?Field too long error 2\n");
			return(-9);
		    }
		    inword = 0;		/* Not in a word any more. */
		    crflag = 1;
                    /* debug(F110,"gtword","crflag is set",0); */
#ifdef CK_RECALL
		    current = rlast;
#endif /* CK_RECALL */
		    cmflgs = 1;
		    if (!xcmdsrc
#ifdef CK_RECALL
			|| force_add
#endif /* CK_RECALL */
			)
  		      addcmd(cmdbuf);
		    return(cmflgs);
		}
            }
/*
  This section handles interactive help, completion, editing, and history.
  Rearranged as a switch statement executed only if we're at top level since
  there is no need for any of this within command files and macros: Aug 2000.
  Jun 2001: Even if at top level, skip this if the character was fetched from
  the reparse or recall buffer, or if stdin is redirected.
*/
	    if ((xcmdsrc == 0		/* Only at top level */
#ifndef NOSPL
		|| askflag		/* or user is typing ASK response */
#endif /* NOSPL */
		 ) && chsrc != 0 && realtty) { /* from the real keyboard */

/* Use ANSI / VT100 up and down arrow keys for command recall.  */

		if (isesc && (
#ifdef IKSD
		    inserver
#else
		    0
#endif /* IKSD */
#ifdef USE_ARROWKEYS
                              || 1
#endif /* USE_ARROWKEYS */
                             )
                     ) {		/* A real ESC was typed */
		    int x;
		    msleep(200);	/* Wait 1/5 sec */
		    x = cmdconchk();	/* Was it followed by anything? */
		    debug(F101,"Arrowkey ESC cmdconchk","",x);

		    if (x > 1) {	/* If followed by at least 2 chars */
			int c2;
			c2 = cmdgetc(0); /* Get the first one */
			debug(F101,"Arrowkey ESC c2","",c2);

			if (c2 != '[' && c2 != 'O') { /* If not [ or O */
			    pushc = c2;	/* Push it and take the ESC solo */
			} else {
			    c2 = cmdgetc(0); /* Get the second one */
			    debug(F101,"Arrowkey ESC c3","",c2);
			    switch (c2) {
#ifndef NORECALL
			      case 'A':	/* Up */
				c = BEL;
				c = C_UP;
				break;
			      case 'B':	/* Down */
				c = BEL;
				c = C_DN;
				break;
			      case 'C':	/* Right */
			      case 'D':	/* Left */
#else
			      default:
#endif /* NORECALL */
				c = BEL; /* We don't use these yet */
				break;
			    }
			}
		    }
		}

		switch (c) {
		  case '?':		/* ?-Help */
#ifndef NOSPL
		    if (askflag)	/* No help in ASK response */
		      break;
#endif /* NOSPL */
		    if (quoting
			&& !kstartactive
			&& !comment
			) {
			cmdecho((char) c, 0);
			*bp = NUL;
			if (setatm(pp,0) < 0) {
			    debug(F111,"gtword too long ?",pp,strlen(pp));
			    printf("?Too long\n");
			    return(-9);
			}
			qmflag = 1;
			return(cmflgs = 3);
		    }

		  case ESC:		/* Esc or Tab completion */
		    if (!comment) {
			*bp = NUL;
			if (setatm(pp,0) < 0) {
			    debug(F111,"gtword too long Esc",pp,strlen(pp));
			    printf("?Too long\n");
			    return(-9);
			}
			esflag = 1;
			return(cmflgs = 2);
		    } else {
			bleep(BP_WARN);
			continue;
		    }

		  case BS:		/* Character deletion */
		  case RUB:
		    if (bp > cmdbuf) {	/* If still in buffer... */
			cmdchardel();	/* erase it. */
			bp--;		/* point behind it, */
			if (*bp == lbrace) bracelvl--; /* Adjust brace count */
			if (*bp == rbrace) bracelvl++;
			if ((*bp == SP) && /* Flag if current field gone */
			    (*pp != lbrace || bracelvl == 0))
			  inword = 0;
			*bp = NUL;	/* Erase character from buffer. */
		    } else {		/* Otherwise, */
			bleep(BP_WARN);
			cmres();	/* and start parsing a new command. */
			*bp = *atmbuf = NUL;
		    }
		    if (pp < bp)
		      continue;
		    else
		      return(cmflgs = -1);

		  case LDEL:		/* ^U, line deletion */
		    while ((bp--) > cmdbuf) {
			cmdchardel();
			*bp = NUL;
		    }
		    cmres();		/* Restart the command. */
		    *bp = *atmbuf = NUL;
		    inword = 0;
		    return(cmflgs = -1);

		  case WDEL:		/* ^W, word deletion */
		    if (bp <= cmdbuf) {	/* Beep if nothing to delete */
			bleep(BP_WARN);
			cmres();
			*bp = *atmbuf = NUL;
			return(cmflgs = -1);
		    }
		    bp--;
		    /* Back up over any trailing nonalphanums */
		    /* This is dependent on ASCII collating sequence */
		    /* but isalphanum() is not available everywhere. */
		    for ( ;
			 (bp >= cmdbuf) &&
			 ((*bp < '0') ||
			 ((*bp > '9') && (*bp < '@')) ||
			 ((*bp > 'Z') && (*bp < 'a')) ||
			 (*bp > 'z'));
			 bp--
			 ) {
			cmdchardel();
			*bp = NUL;
		    }
		    /* Now delete back to rightmost remaining nonalphanum */
		    for ( ; (bp >= cmdbuf) && (*bp) ; bp--) {
			if ((*bp < '0') ||
			    (*bp > '9' && *bp < '@') ||
			    (*bp > 'Z' && *bp < 'a') ||
			    (*bp > 'z'))
			  break;
			cmdchardel();
			*bp = NUL;
		    }
		    bp++;
		    inword = 0;
		    return(cmflgs = -1);

		  case RDIS: {		/* ^R, redisplay */
		      char *cpx; char cx;
		      *bp = NUL;
		      printf("\n%s",cmprom);
		      cpx = cmdbuf;
		      while ((cx = *cpx++)) {
			  cmdecho(cx,0);
		      }
		      fflush(stdout);
		      continue;
		  }
#ifndef NOLASTFILE
		  case VT:
		    if (lastfile) {
			printf("%s ",lastfile);
#ifdef GEMDOS
			fflush(stdout);
#endif /* GEMDOS */
			inword = cmflgs = 0;
			addbuf(lastfile);	/* Supply default. */
			if (setatm(lastfile,0) < 0) {
			    printf("Last name too long\n");
			    if (np) free(np);
			    return(-9);
			}
		    } else {		/* No default */
			bleep(BP_WARN);
		    }
		    return(0);
#endif	/* NOLASTFILE */
		}

#ifdef CK_RECALL
		if (on_recall &&	/* Reading commands from keyboard? */
		    (cm_recall > 0) &&	/* Saving commands? */
		    (c == C_UP || c == C_UP2)) { /* Go up one */
		    if (last_recall == 2 && current > 0)
		      current--;
		    if (current < 0) {	/* Nowhere to go, */
			bleep(BP_WARN);
			continue;
		    }
		    if (recall[current]) { /* We have a previous command */
			while ((bp--) > cmdbuf) { /* Erase current line */
			    cmdchardel();
			    *bp = NUL;
			}
			ckstrncpy(cmdbuf,recall[current],CMDBL);
#ifdef OSK
			fflush(stdout);
			write(fileno(stdout), "\r", 1);
			printf("%s%s",cmprom,cmdbuf);
#else
			printf("\r%s%s",cmprom,cmdbuf);
#endif /* OSK */
			current--;
		    }
		    last_recall = 1;
		    return(cmflgs = -1); /* Force a reparse */
		}
		if (on_recall &&	/* Reading commands from keyboard? */
		    (cm_recall > 0) &&	/* Saving commands? */
		    (c == C_DN)) {	/* Down one */
		    int x = 1;
		    if (last_recall == 1)
		      x++;
		    if (current + x > rlast) { /* Already at bottom, beep */
			bleep(BP_WARN);
			continue;
		    }
		    current += x;	/* OK to go down */
		    if (recall[current]) {
			while ((bp--) > cmdbuf) { /* Erase current line */
			    cmdchardel();
			    *bp = NUL;
			}
			ckstrncpy(cmdbuf,recall[current],CMDBL);
#ifdef OSK
			fflush(stdout);
			write(fileno(stdout), "\r", 1);
			printf("%s%s",cmprom,cmdbuf);
#else
			printf("\r%s%s",cmprom,cmdbuf);
#endif /* OSK */
			last_recall = 2;
			return(cmflgs = -1); /* Force reparse */
		    }
		}
#endif /* CK_RECALL */
	    }

	    if (c < SP && quote == 0) { /* Any other unquoted control char */
		if (!chsrc) {		/* If cmd file, point past it */
		    bp++;
		} else {
		    bleep(BP_WARN);
		}
		continue;		/* continue, don't put in buffer */
	    }
	    linebegin = 0;		/* Not at beginning of line */
#ifdef BEBOX
	    if (echof) {
                cmdecho((char) c, 0);	/* Echo what was typed. */
                fflush (stdout);
                fflush(stderr);
            }
#else
#ifdef NOSPL
            if (echof || chsrc)
#else
            if (echof || (echostars && chsrc))
#endif	/* NOSPL */
	      cmdecho((char) c, 0);	/* Echo what was typed. */
#endif /* BEBOX */
        } else {			/* This character was quoted. */
	    int qf = 1;
	    quote = 0;			/* Unset the quote flag. */
	    /* debug(F000,"gtword quote 0","",c); */
	    /* Quote character at this level is only for SP, ?, and controls */
            /* If anything else was quoted, leave quote in, and let */
	    /* the command-specific parsing routines handle it, e.g. \007 */
	    if (c > 32 && c != '?' && c != RUB && chsrc != 0) {
		/* debug(F000,"gtword quote 1","",c); */
		*bp++ = CMDQ;		/* Deposit \ if it came from tty */
		qf = 0;			/* and don't erase it from screen */
		linebegin = 0;		/* Not at beginning of line */
#ifdef BS_DIRSEP
/*
  This is a hack to handle "cd \" or "cd foo\" on OS/2 and similar systems.
  If we were called from cmdir() and the previous character was the quote
  character, i.e. backslash, and this character is the command terminator,
  then we stuff an extra backslash into the buffer without echoing, then
  we stuff the carriage return back in again, and go back and process it,
  this time with the quote flag off.
*/
	    } else if (dirnamflg && (c == CR || c == LF || c == SP)) {
		/* debug(F000,"gtword quote 2","",c); */
		*bp++ = CMDQ;
		linebegin = 0;		/* Not at beginning of line */
		*bp = (c == SP ? SP : CR);
		goto CMDIRPARSE;
#endif /* BS_DIRSEP */
	    }
#ifdef BEBOX
	    if (echof) {
                cmdecho((char) c, qf);	/* Echo what was typed. */
                fflush (stdout);
                fflush(stderr);
            }
#else
	    if (echof) cmdecho((char) c, qf); /* Now echo quoted character */
#endif /* BEBOX */
	    /* debug(F111,"gtword quote",cmdbuf,c); */
	}
#ifdef COMMENT
        if (echof) cmdecho((char) c,quote); /* Echo what was typed. */
#endif /* COMMENT */
        if (!comment) inword = 1;	/* Flag we're in a word. */
	if (quote) continue;		/* Don't deposit quote character. */
        if (c != NL) {			/* Deposit command character. */
	    *bp++ = (char) c;		/* and make sure there is a NUL */
#ifdef COMMENT
	    *bp = NUL;			/* after it */
#endif /* COMMENT */
	}
    }                                   /* End of big while */
    bleep(BP_WARN);
    printf("?Command too long, maximum length: %d.\n",CMDBL);
    cmflgs = -2;
    return(-9);
}

/* Utility functions */

/* A D D B U F  -- Add the string pointed to by cp to the command buffer  */

static int
addbuf(cp) char *cp; {
    int len = 0;
    while ((*cp != NUL) && (bp < cmdbuf+CMDBL)) {
        *bp++ = *cp++;                  /* Copy and */
        len++;                          /* count the characters. */
    }
    *bp++ = SP;                         /* Put a space at the end */
    *bp = NUL;                          /* Terminate with a null */
    np = bp;                            /* Update the next-field pointer */
    cmbptr = np;
    return(len);                        /* Return the length */
}

/*  S E T A T M  --  Deposit a token in the atom buffer.  */
/*
  Break on space, newline, carriage return, or NUL.
  Call with:
    cp = Pointer to string to copy to atom buffer.
    fcode = 0 means break on whitespace or EOL.
    fcode = 1 means don't break on space.
    fcode = 2 means break on space, ':', or '='.
    fcode = 3 means copy the whole string.
  Null-terminate the result.
  Return length of token, and also set global "cc" to this length.
  Return -1 if token was too long.
*/
static int
setatm(cp,fcode) char *cp; int fcode; {
    char *ap, *xp, *dqp = NULL, lbrace, rbrace;
    int bracelvl = 0, dq = 0;

    register char * s;
    register int n = 0;

    if (cmfldflgs & 1) {		/* Handle grouping */
	lbrace = '(';
	rbrace = ')';
    } else {
	lbrace = '{';
	rbrace = '}';
    }
    cc = 0;				/* Character counter */
    ap = atmbuf;			/* Address of atom buffer */

    s = cp;

    while (*s++) n++;			/* Save a call to strlen */

    if (n > ATMBL) {
	printf("?Command buffer overflow\n");
	return(-1);
    }
    /* debug(F111,"setatm",cp,n); */
    if (cp == ap) {			/* In case source is atom buffer */
	xp = atybuf;			/* make a copy */
#ifdef COMMENT
	strncpy(xp,ap,ATMBL);		/* so we can copy it back, edited. */
	cp = xp;
#else
	s = ap;
	while ((*xp++ = *s++)) ;	/* We already know it's big enough */
	cp = xp = atybuf;
#endif /* COMMENT */
    }
    *ap = NUL;				/* Zero the atom buffer */
    if (fcode == 1) {			/* Trim trailing blanks */
	while (--n >= 0 && cp[n] == SP)
	  ;
	cp[n+1] = NUL;
    }
    while (*cp == SP) {			/* Trim leading spaces */
	cp++;
	n--;
    }
    if (*cp == '"') {			/* Starts with doublequote? */
	dq = 1;
	dqp = cp;
    }
    while (*cp) {
        if (*cp == lbrace)
	  bracelvl++;
        else if (*cp == rbrace)
	  bracelvl--;
	if (bracelvl < 0)
	  bracelvl = 0;
	if (bracelvl == 0) {
	    if (dq) {
		if (*cp == SP || *cp == HT) {
		    if (cp > dqp+1) {
			if (*(cp-1) == '"' && *(cp-2) != CMDQ) {
			    break;
			}
		    }
		}
	    } else if ((*cp == SP || *cp == HT) && fcode != 1 && fcode != 3)
	      break;
	    if ((fcode == 2) && (*cp == '=' || *cp == ':')) break;
	    if ((fcode != 3) && (*cp == LF || *cp == CR)) break;
	}
        *ap++ = *cp++;
        cc++;
    }
    *ap = NUL;				/* Terminate the string. */
    /* debug(F111,"setatm result",atmbuf,cc); */
    return(cc);                         /* Return length. */
}

/*
  These functions attempt to hide system dependencies from the mainline
  code in gtword().  Dummy arg for cmdgetc() needed for compatibility with
  coninc(), ttinc(), etc, since a pointer to this routine can be passed in
  place of those to tn_doop().

  No longer static.  Used by askmore().  Fri Aug 20 15:03:34 1999.
*/
#define CMD_CONINC			/* How we get keyboard chars */

int
cmdgetc(timelimit) int timelimit; {	/* Get a character from the tty. */
    int c;
#ifdef IKSD
    extern int inserver;
#endif /* IKSD */
#ifdef CK_LOGIN
    extern int x_logged;
#endif /* CK_LOGIN */
#ifdef TNCODE
    static int got_cr = 0;
    extern int ckxech;
    int tx = 0, is_tn = 0;
#endif /* TNCODE */

    if (pushc
#ifndef NOSPL
	&& !askflag
#endif /* NOSPL */
	) {
        debug(F111,"cmdgetc()","pushc",pushc);
	c = pushc;
	pushc = NUL;
	if (xcmfdb && c == '?')		/* Don't echo ? twice if chaining. */
	  cmdchardel();
	return(c);
    }
#ifdef datageneral
    {
	char ch;
	c = dgncinb(0,&ch,1);		/* -1 is EOF, -2 TO,
                                         * -c is AOS/VS error */
	if (c == -2) {			/* timeout was enabled? */
	    resto(channel(0));		/* reset timeouts */
	    c = dgncinb(0,&ch,1);	/* retry this now! */
	}
	if (c < 0) return(-4);		/* EOF or some error */
	else c = (int) ch & 0177;	/* Get char without parity */
/*	echof = 1; */
    }
#else /* Not datageneral */
#ifndef MINIX2
    if (
#ifdef IKSD
	(!local && inserver) ||
#endif /* IKSD */
	timelimit > 0) {
#ifdef TNCODE
          GETNEXTCH:
            is_tn = !pushc && !local && sstelnet;
#endif /* TNCODE */
#ifdef COMMENT
	    c = coninc(timelimit > 0 ? 1 : 0);
#else /* COMMENT */
	    /* This is likely to break the asktimeout... */
	    c = coninc(timelimit);
#endif /* COMMENT */
	    /* debug(F101,"cmdgetc coninc","",c); */
#ifdef TNCODE
            if (c >= 0 && is_tn) {	/* Server-side Telnet */
                switch (c) {
		  case IAC:
                    /* debug(F111,"gtword IAC","c",c); */
                    got_cr = 0;
                    if ((tx = tn_doop((CHAR)(c & 0xff),ckxech,coninc)) == 0) {
                        goto GETNEXTCH;
                    } else if (tx <= -1) { /* I/O error */
                        /* If there was a fatal I/O error then ttclos()    */
                        /* has been called and the next GETNEXTCH attempt  */
                        /* will be !is_tn since ttclos() sets sstelnet = 0 */
                        doexit(BAD_EXIT,-1); /* (or return(-4)? */
                    } else if (tx == 1) { /* ECHO change */
                        ckxech = dpx = 1; /* Get next char */
                        goto GETNEXTCH;
                    } else if (tx == 2) { /* ECHO change */
                        ckxech = dpx = 0; /* Get next char */
                        goto GETNEXTCH;
                    } else if (tx == 3) { /* Quoted IAC */
                        c = 255;	/* proceeed with it. */
                    }
#ifdef IKS_OPTION
                    else if (tx == 4) {	/* IKS State Change */
                        goto GETNEXTCH;
                    }
#endif /* IKS_OPTION */
                    else if (tx == 6) {	/* Remote Logout */
			doexit(GOOD_EXIT,0);
                    } else {
			goto GETNEXTCH;	/* Unknown, get next char */
		    }
                    break;
#ifdef COMMENT
                  case CR:
                    if (!TELOPT_U(TELOPT_BINARY)) {
			if (got_cr) {
			    /* This means the sender is violating Telnet   */
			    /* protocol because we received two CRs in a   */
			    /* row without getting either LF or NUL.       */
			    /* This will not solve the problem but it      */
			    /* will at least allow two CRs to do something */
			    /* whereas before the user would have to guess */
			    /* to send LF or NUL after the CR.             */
			    debug(F100,"gtword CR telnet error","",0);
			    c = LF;
			} else {
			    debug(F100,"gtword skipping CR","",0);
			    got_cr = 1;	/* Remember a CR was received */
			    goto GETNEXTCH;
			}
                    } else {
			debug(F100,"gtword CR to LF","",0);
			c = LF;
                    }
                    break;
                  case LF:
                    if (!TELOPT_U(TELOPT_BINARY)) {
			got_cr = 0;
			debug(F100,"gtword LF","",0);
                    } else {
			if (got_cr) {
			    got_cr = 0;
			    debug(F100,"gtword skipping LF","",0);
			    goto GETNEXTCH;
			}
                    }
                    break;
                  case NUL:
                    if (!TELOPT_U(TELOPT_BINARY) && got_cr) {
			c = LF;
			debug(F100,"gtword NUL to LF","",0);
                    } else {
			debug(F100,"gtword NUL","",0);
                    }
                    got_cr = 0;
                    break;
#else /* COMMENT */
                  case CR:
                    if ( !TELOPT_U(TELOPT_BINARY) && got_cr ) {
                        /* This means the sender is violating Telnet   */
                        /* protocol because we received two CRs in a   */
                        /* row without getting either LF or NUL.       */
                        /* This will not solve the problem but it      */
                        /* will at least allow two CRs to do something */
                        /* whereas before the user would have to guess */
                        /* to send LF or NUL after the CR.             */
                        debug(F100,"gtword CR telnet error","",0);
                    } else {
                        got_cr = 1;	/* Remember a CR was received */
                    }
                    /* debug(F100,"gtword CR to LF","",0); */
                    c = LF;
		    break;
                  case LF:
                    if (got_cr) {
                        got_cr = 0;
                        /* debug(F100,"gtword skipping LF","",0); */
                        goto GETNEXTCH;
                    }
		    break;
                  case NUL:
                    if (got_cr) {
                        got_cr = 0;
                        /* debug(F100,"gtword skipping NUL","",0); */
                        goto GETNEXTCH;
#ifdef COMMENT
                    } else {
                      debug(F100,"gtword NUL","",0);
#endif /* COMMENT */
                    }
                    break;
#endif /* COMMENT */
#ifdef IKSD
		  case ETX:		/* Ctrl-C... */
                  case EOT:		/* EOT = EOF */
                      if (inserver
#ifdef CK_LOGIN
			  && !x_logged
#endif /* CK_LOGIN */
			  )
                          return(-4);
		    break;
#endif /* IKSD */
		  default:
                      got_cr = 0;
                }
            }
#endif /* TNCODE */
    } else {
#ifdef OS2
	c = coninc(0);
#else /* OS2 */
#ifdef CMD_CONINC
#undef CMD_CONINC
#endif /* CMD_CONINC */
	c = getchar();
#endif /* OS2 */
    }
#else  /* MINIX2 */
#undef getc
#ifdef CMD_CONINC
#undef CMD_CONINC
#endif /* CMD_CONINC */
    c = getc(stdin);
    /* debug(F101,"cmdgetc getc","",c); */
#endif /* MINIX2 */
#ifdef RTU
    if (rtu_bug) {
#ifdef CMD_CONINC
#undef CMD_CONINC
#endif /* CMD_CONINC */
	c = getchar();			/* RTU doesn't discard the ^Z */
	rtu_bug = 0;
    }
#endif /* RTU */
#endif /* datageneral */
    return(c);				/* Return what we got */
}

/* #ifdef USE_ARROWKEYS */

/* Mechanism to use for peeking into stdin buffer */

#ifndef USE_FILE_CNT			/* stdin->__cnt */
#ifndef USE_FILE__CNT			/* Note: two underscores */
#ifdef HPUX				/* HPUX 7-11 */
#ifndef HPUX5
#ifndef HPUX6
#define USE_FILE__CNT
#endif /* HPUX6 */
#endif /* HPUX5 */
#else
#ifdef ANYSCO				/* SCO UNIX, OSR5, Unixware, etc */
#ifndef OLD_UNIXWARE			/* But not Unixware 1.x or 2.0 */
#ifndef UNIXWARE2			/* or 2.1.0 */
#define USE_FILE__CNT
#endif /* UNIXWARE2 */
#endif /* OLD_UNIXWARE */
#endif /* ANYSCO */
#endif /* HPUX */
#endif /* USE_FILE__CNT */
#endif /* USE_FILE_CNT */

#ifndef USE_FILE_R			/* stdin->_r */
#ifndef USE_FILE_CNT
#ifndef USE_FILE__CNT
#ifdef BSD44				/* {Free,Open,Net}BSD, BSDI */
#define USE_FILE_R
#endif /* BSD44 */
#endif /* USE_FILE__CNT */
#endif /* USE_FILE_CNT */
#endif /* USE_FILE_R */

#ifndef USE_FILE_R			/* stdin->_cnt */
#ifndef USE_FILE_CNT
#ifndef USE_FILE__CNT
#define USE_FILE_CNT			/* Everybody else (but Linux) */
#endif /* USE_FILE__CNT */
#endif /* USE_FILE_CNT */
#endif /* USE_FILE_R */


/*
  c m d c o n c h k

  How many characters are waiting to be read at the console?  Normally
  conchk() would tell us, but in Unix and VMS cmdgetc() uses stdio getchar(),
  thus bypassing coninc()/conchk(), so we have to peek into the stdin buffer,
  which is totally nonportable.  Which is why this routine is, at least for
  now, used only for checking for arrow-key sequences from the keyboard after
  an ESC was read.  Wouldn't it be nice if the stdio package had a function
  that returned the number of bytes waiting to be read from its buffer?
  Returns 0 or greater always.
*/
int
cmdconchk() {
    int x = 0, y;
    y = pushc ? 1 : 0;			/* Have command character pushed? */
#ifdef OS2
    x = conchk();			/* Check device-driver buffer */
    if (x < 0) x = 0;
#else /* OS2 */
#ifdef CMD_CONINC			/* See cmdgetc() */
    x = conchk();			/* Check device-driver buffer */
    if (x < 0) x = 0;
#else  /* CMD_CONINC */

/* Here we must look inside the stdin buffer - highly platform dependent */

#ifdef _IO_file_flags			/* Linux */
    x = (int) ((stdin->_IO_read_end) - (stdin->_IO_read_ptr));
    debug(F101,"cmdconchk _IO_file_flags","",x);
#else  /* _IO_file_flags */
#ifdef USE_FILE_CNT			/* Traditional */
#ifdef VMS
    debug(F101,"cmdconchk (*stdin)->_cnt","",(*stdin)->_cnt);
    x = (*stdin)->_cnt;
#else
#ifdef NOARROWKEYS
    debug(F101,"cmdconchk NOARROWKEYS x","",0);
#else
    debug(F101,"cmdconchk stdin->_cnt","",stdin->_cnt);
    x = stdin->_cnt;
#endif /* NOARROWKEYS */
#endif /* VMS */
    if (x == 0) x = conchk();
    if (x < 0) x = 0;
#else  /* USE_FILE_CNT */
#ifdef USE_FILE__CNT			/* HP-UX */
    debug(F101,"cmdconchk stdin->__cnt","",stdin->__cnt);
    x = stdin->__cnt;
    if (x == 0) x = conchk();
    if (x < 0) x = 0;
#else  /* USE_FILE_CNT */
#ifdef USE_FILE_R			/* FreeBSD, OpenBSD, etc */
    debug(F101,"cmdconchk stdin->_r","",stdin->_r);
    x = stdin->_r;
    if (x == 0) x = conchk();
    if (x < 0) x = 0;

    /* Fill in any others here... */

#endif /* USE_FILE_R */
#endif /* USE_FILE__CNT */
#endif /* USE_FILE_CNT */
#endif /* _IO_file_flags */
#endif /* CMD_CONINC */
#endif /* OS2 */
    return(x + y);
}
/* #endif */ /* USE_ARROWKEYS */


static VOID
cmdclrscn() {				/* Clear the screen */
    ck_cls();
}

static VOID				/* What to echo at end of command */
#ifdef CK_ANSIC
cmdnewl(char c)
#else
cmdnewl(c) char c;
#endif /* CK_ANSIC */
/* cmdnewl */ {
#ifdef OS2
#ifdef IKSD
    extern int inserver;
    if (inserver && c == LF)
      putchar(CR);
#endif /* IKSD */
#endif /* OS2 */

    putchar(c);				/* c is the terminating character */

#ifdef WINTCP				/* what is this doing here? */
    if (c == CR) putchar(NL);
#endif /* WINTCP */

/*
  A.A. Chernov, who sent in changes for FreeBSD, said we also needed this
  for SVORPOSIX because "setup terminal by termios and curses does
  not convert \r to \n, so additional \n needed in newline function."  But
  it is also very likely to result in unwanted blank lines.
*/
#ifdef BSD44
    if (c == CR) putchar(NL);
#endif /* BSD44 */

#ifdef COMMENT
    /* OS2 no longer needs this as all CR are converted to NL in coninc() */
    /* This eliminates the ugly extra blank lines discussed above.        */
#ifdef OS2
    if (c == CR) putchar(NL);
#endif /* OS2 */
#endif /* COMMENT */
#ifdef aegis
    if (c == CR) putchar(NL);
#endif /* aegis */
#ifdef AMIGA
    if (c == CR) putchar(NL);
#endif /* AMIGA */
#ifdef datageneral
    if (c == CR) putchar(NL);
#endif /* datageneral */
#ifdef GEMDOS
    if (c == CR) putchar(NL);
#endif /* GEMDOS */
#ifdef STRATUS
    if (c == CR) putchar(NL);
#endif /* STRATUS */
}

static VOID
cmdchardel() {				/* Erase a character from the screen */
#ifndef NOSPL
    if (!echostars)
#endif	/* NOSPL */
      if (!dpx) return;
#ifdef datageneral
    /* DG '\b' is EM (^y or \031) */
    if (termtype == 1)
      /* Erase a character from non-DG screen, */
      dgncoub(1,"\010 \010",3);
    else
#endif /* datageneral */
      printf("\b \b");
#ifdef GEMDOS
    fflush(stdout);
#else
#ifdef BEBOX
    fflush(stdout);
#endif /* BEBOX */
#endif /* GEMDOS */
}

static VOID
#ifdef CK_ANSIC
cmdecho(char c, int quote)
#else
cmdecho(c,quote) char c; int quote;
#endif /* CK_ANSIC */
{ /* cmdecho */
#ifdef NOSPL
    if (!dpx) return;
#else
    if (!echostars) {
	if (!dpx) return;
    } else {
	c = (char)echostars;
    }
#endif	/* NOSPL */
    /* Echo tty input character c */
    if (quote) {
	putchar(BS);
	putchar(SP);
	putchar(BS);
#ifdef isprint
	putchar((CHAR) (isprint(c) ? c : '^' ));
#else
	putchar((CHAR) ((c >= SP && c < DEL) ? c : '^'));
#endif /* isprint */
    } else {
	putchar(c);
    }
#ifdef OS2
    if (quote==1 && c==CR) putchar((CHAR) NL);
#endif /* OS2 */
    if (timelimit)
      fflush(stdout);
}

/* Return pointer to current position in command buffer. */

char *
cmpeek() {
    return(np);
}
#endif /* NOICP */


#ifdef NOICP
#include "ckcdeb.h"
#include "ckucmd.h"
#include "ckcasc.h"
#endif /* NOICP */

/*  X X E S C  --  Interprets backslash codes  */
/*  Returns the int value of the backslash code if it is > -1 and < 256 */
/*  and updates the string pointer to first character after backslash code. */
/*  If the argument is invalid, leaves pointer unchanged and returns -1. */

int
xxesc(s) char **s; {			/* Expand backslash escapes */
    int x, y, brace, radix;		/* Returns the int value */
    char hd = '9';			/* Highest digit in radix */
    char *p;

    p = *s;				/* pointer to beginning */
    if (!p) return(-1);			/* watch out for null pointer */
    x = *p++;				/* character at beginning */
    if (x != CMDQ) return(-1);		/* make sure it's a backslash code */

    x = *p;				/* it is, get the next character */
    if (x == '{') {			/* bracketed quantity? */
	p++;				/* begin past bracket */
	x = *p;
	brace = 1;
    } else brace = 0;
    switch (x) {			/* Start interpreting */
      case 'd':				/* Decimal radix indicator */
      case 'D':
	p++;				/* Just point past it and fall thru */
      case '0':				/* Starts with digit */
      case '1':
      case '2':  case '3':  case '4':  case '5':
      case '6':  case '7':  case '8':  case '9':
	radix = 10;			/* Decimal */
	hd = '9';			/* highest valid digit */
	break;
      case 'o':				/* Starts with o or O */
      case 'O':
	radix = 8;			/* Octal */
	hd = '7';			/* highest valid digit */
	p++;				/* point past radix indicator */
	break;
      case 'x':				/* Starts with x or X */
      case 'X':
	radix = 16;			/* Hexadecimal */
	p++;				/* point past radix indicator */
	break;
      default:				/* All others */
#ifdef COMMENT
	*s = p+1;			/* Treat as quote of next char */
	return(*p);
#else
	return(-1);
#endif /* COMMENT */
    }
    /* For OS/2, there are "wide" characters required for the keyboard
     * binding, i.e \644 and similar codes larger than 255 (byte).
     * For this purpose, give up checking for < 256. If someone means
     * \266 should result in \26 followed by a "6" character, he should
     * always write \{26}6 anyway.  Now, return only the lower byte of
     * the result, i.e. 10, but eat up the whole \266 sequence and
     * put the wide result 266 into a global variable.  Yes, that's not
     * the most beautiful programming style but requires the least
     * amount of changes to other routines.
     */
    if (*p == '{') {			/* Sun May 11 20:00:40 2003 */
	brace = 1;			/* Allow {} after radix indicator */
	p++;
    }
    if (radix <= 10) {			/* Number in radix 8 or 10 */
	for ( x = y = 0;
 	      (*p) && (*p >= '0') && (*p <= hd)
#ifdef OS2
                   && (y < 5) && (x*radix < KMSIZE);
              /* the maximum needed value \8196 is 4 digits long */
              /* while as octal it requires \1377, i.e. 5 digits */
#else
                   && (y < 3) && (x*radix < 256);
#endif /* OS2 */
	      p++,y++) {
	    x = x * radix + (int) *p - 48;
	}
#ifdef OS2
        wideresult = x;			/* Remember wide result */
        x &= 255;
#endif /* OS2 */
	if (y == 0 || x > 255) {	/* No valid digits? */
	    *s = p;			/* point after it */
	    return(-1);			/* return failure. */
	}
    } else if (radix == 16) {		/* Special case for hex */
	if ((x = unhex(*p++)) < 0) { *s = p - 1; return(-1); }
	if ((y = unhex(*p++)) < 0) { *s = p - 2; return(-1); }
	x = ((x << 4) & 0xF0) | (y & 0x0F);
#ifdef OS2
        wideresult = x;
        if ((y = unhex(*p)) >= 0) {
           p++;
	   wideresult = ((x << 4) & 0xFF0) | (y & 0x0F);
           x = wideresult & 255;
        }
#endif /* OS2 */
    } else x = -1;
    if (brace && *p == '}' && x > -1)	/* Point past closing brace, if any */
      p++;
    *s = p;				/* Point to next char after sequence */
    return(x);				/* Return value of sequence */
}

int					/* Convert hex string to int */
#ifdef CK_ANSIC
unhex(char x)
#else
unhex(x) char x;
#endif /* CK_ANSIC */
/* unhex */ {

    if (x >= '0' && x <= '9')		/* 0-9 is offset by hex 30 */
      return(x - 0x30);
    else if (x >= 'A' && x <= 'F')	/* A-F offset by hex 37 */
      return(x - 0x37);
    else if (x >= 'a' && x <= 'f')	/* a-f offset by hex 57 */
      return(x - 0x57);			/* (obviously ASCII dependent) */
    else return(-1);
}

/*  L O O K U P  --  Lookup the string in the given array of strings  */

/*
  Call this way:  v = lookup(table,word,n,&x);

    table - a 'struct keytab' table.
    word  - the target string to look up in the table.
    n     - the number of elements in the table.
    x     - address of an integer for returning the table array index,
	    or NULL if you don't need a table index.

  The keyword table must be arranged in ascending alphabetical order;
  alphabetic case doesn't matter but letters are treated as lowercase
  for purposes of ordering; thus "^" and "_" come *before* the letters,
  not after them.

  Returns the keyword's associated value (zero or greater) if found,
  with the variable x set to the keyword-table index.  If is lookup()
  is not successful, it returns:

   -3 if nothing to look up (target was null),
   -2 if ambiguous,
   -1 if not found.

  A match is successful if the target matches a keyword exactly, or if
  the target is a prefix of exactly one keyword.  It is ambiguous if the
  target matches two or more keywords from the table.

  Lookup() is the critical routine in scripts and so is optimized with a
  simple static cache plus some other tricks.  Maybe it could be improved
  further with binary search or hash techniques but I doubt it since most
  keyword tables are fairly short.
*/

#ifdef USE_LUCACHE			/* Lookup cache */
extern int lusize;			/* (initialized in ckuus5.c) */
extern char * lucmd[];
extern int luval[];
extern int luidx[];
extern struct keytab * lutab[];
long luhits = 0L;
long lucalls = 0L;
long xxhits = 0L;
long luloop = 0L;
#endif /* USE_LUCACHE */

int
lookup(table,cmd,n,x) char *cmd; struct keytab table[]; int n, *x; {

    register int i, m;
    int v, len, cmdlen = 0;
    char c = NUL, c1, *s;

/* Get 1st char of search object, if it's null return -3. */

    if (!cmd || n < 1)			/* Defense de nullarg */
      return(-3);
    c1 = *cmd;				/* First character */
    if (!c1)				/* Make sure there is one */
      return(-3);
    if (isupper(c1))			/* If letter make it lowercase */
      c1 = tolower(c1);

#ifdef USE_LUCACHE			/* lookup() cache */
    m = lusize;
    lucalls++;				/* Count this lookup() call */
    for (i = 0; i < m; i++) {		/* Loop thru cache */
	if (*(lucmd[i]) == c1) {	/* Same as 1st char of search item? */
	    if (lutab[i] == table) {	/* Yes - same table too? */
		if (!strcmp(cmd,lucmd[i])) { /* Yes - compare */
		    if (x) *x = luidx[i];    /* Match - return index */
		    luhits++;                /* Count cache hit */
		    return(luval[i]);        /* Return associated value */
		}
	    }
	}
    }
#endif /* USE_LUCACHE */

/* Not null, not in cache, look it up */

    s = cmd;
    while (*s++) cmdlen++;		/* Length of target */
/*
  Quick binary search to find last table entry whose first character is
  lexically less than the first character of the search object.  This is
  the starting point of the next loop, which must go in sequence since it
  compares adjacent table entries.
*/
    if (n < 5) {			/* Not worth it for small tables */
	i = 0;
    } else {
	int lo = 0;
	int hi = n;
	int count = 0;
	while (lo+2 < hi && ++count < 12) {
	    i = lo + ((hi - lo) / 2);
	    c = *(table[i].kwd);
	    if (isupper(c)) c = tolower(c);
	    if (c < c1) {
		lo = i;
	    } else {
		hi = i;
	    }
	}
	i = (c < c1) ? lo+1 : lo;
#ifdef USE_LUCACHE
	if (i > 0) xxhits++;
#endif /* USE_LUCACHE */
    }
    for ( ; i < n-1; i++) {
#ifdef USE_LUCACHE
	luloop++;
#endif /* USE_LUCACHE */
	v = 0;
	c = *(table[i].kwd);
	if (c) {
	    if (isupper(c)) c = tolower(c);

	    /* The following is a big performance booster but makes it */
	    /* absolutely essential that all lookup() tables are in order. */

	    if (c > c1)			/* Leave early if past our mark */
	      return(-1);

#ifdef DEBUG
	    /* Use LOG DEBUG to check */

	    if (deblog) {
		if (ckstrcmp(table[i].kwd,table[i+1].kwd,0,0) > 0) {
		    printf("TABLE OUT OF ORDER [%s] [%s]\n",
			   table[i].kwd,table[i+1].kwd);

		}
	    }
#endif /* DEBUG */

	    if (c == c1) {
		len = 0;
		s = table[i].kwd;
		while (*s++) len++;
		if ((len == cmdlen && !ckstrcmp(table[i].kwd,cmd,len,0)) ||
		    ((v = !ckstrcmp(table[i].kwd,cmd,cmdlen,0)) &&
		     ckstrcmp(table[i+1].kwd,cmd,cmdlen,0))) {
		    if (x) *x = i;
		    return(table[i].kwval);
		}
	    } else v = 0;
	}
        if (v) {			/* Ambiguous */
	    if (x) *x = i;		/* Set index of first match */
	    return(-2);
	}
    }

/* Last (or only) element */

    if (!ckstrcmp(table[n-1].kwd,cmd,cmdlen,0)) {
        if (x) *x = n-1;
	/* debug(F111,"lookup",table[i].kwd,table); */
        return(table[n-1].kwval);
    } else return(-1);
}

/*
  x l o o k u p

  Like lookup, but requires a full (but case-independent) match
  and does NOT require the table to be in order.
*/
int
xlookup(table,cmd,n,x) struct keytab table[]; char *cmd; int n, *x; {
    register int i;
    int len, cmdlen, one = 0;
    register char c, c2, * s, * s2;

    if (!cmd) cmd = "";			/* Check args */
    if (!*cmd || n < 1) return(-3);

    c = *cmd;				/* First char of string to look up */
    if (!*(cmd+1)) {			/* Special handling for 1-char names */
	cmdlen = 1;
	if (isupper(c))
	  c = tolower(c);
	one = 1;
    } else {
	cmdlen = 0;
	s = cmd;
	while (*s++) cmdlen++;
	c = *cmd;
	if (isupper(c))
	  c = tolower(c);
    }
    if (cmdlen < 1)
      return(-3);

    for (i = 0; i < n; i++) {
	s = table[i].kwd;		/* This entry */
	if (!s) s = "";
	if (!*s) continue;		/* Empty table entry */
	c2 = *s;
	if (isupper(c2)) c2 = tolower(c2);
	if (c != c2) continue;		/* First char doesn't match */
	if (one) {			/* Name is one char long */
	    if (!*(s+1)) {
		if (x) *x = i;
                *cmd = c; 
		return(table[i].kwval);	/* So is table entry */
	    }
	} else {			/* Otherwise do string comparison */
	    s2 = s;
	    len = 0;
	    while (*s2++) len++;
	    if (len == cmdlen && !ckstrcmp(s,cmd,-1,0)) {
		if (x) *x = i;
		return(table[i].kwval);
	    }
	}
    }
    return(-1);
}

/* Reverse lookup */

char *
rlookup(table,n,x) struct keytab table[]; int n, x; {
    int i;
    for (i = 0; i < n; i++) {
        if (table[i].kwval == x)
	  return(table[i].kwd);
    }
    return(NULL);
}

#ifndef NOICP
int
cmdsquo(x) int x; {
    quoting = x;
    return(1);
}

int
cmdgquo() {
    return(quoting);
}
#endif /* NOICP */

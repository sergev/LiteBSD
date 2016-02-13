char * cklibv = "C-Kermit library, 9.0.052, 29 Jun 2011";

#define CKCLIB_C

/* C K C L I B . C  --  C-Kermit Library routines. */

/*
  Author: Frank da Cruz <fdc@columbia.edu>,
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1999, 2011,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/

/*
  General-purpose, system/platform/compiler-independent routines for use
  by all modules.  Many are replacements for commonly used C library
  functions that are not found on every platform, and/or that lack needed
  functionality (e.g. caseless string search/compare) or safety features.

    ckstrncpy()  - Similar to strncpy() but different (see comments).
    ckstrncat()  - Similar to strncat() but different (see comments).
    chartostr()  - Converts a char to a string (self or ctrl char name).
    ckstrchr()   - Portable strchr().
    ckstrpbrk()  - Portable strpbrk().
    cklower()    - Lowercase a string (in place).
    ckupper()    - Uppercase a string (in place).
    ckindex()    - Left or right index.
    ckstrstr()   - Portable strstr().
    ckitoa()     - Converts int to string.
    ckuitoa()    - Converts unsigned int to string.
    ckltoa()     - Converts long to string.
    ckultoa()    - Converts unsigned long to string.
    ckfstoa()    - Converts off_t-type integer (long or long long) to string.
    ckatofs()    - Converts a numeric string to an off_t-type integer.
    ckctoa()     - Converts char to string.
    ckmakmsg()   - Constructs a message from 4 source strings.
    ckmakxmsg()  - Constructs a message from 12 source strings.
    ckmatch()    - Pattern matching.
    ckmemcpy()   - Portable memcpy().
    ckrchar()    - Rightmost character of a string.
    ckstrcmp()   - Possibly caseless string comparison.
    ckstrpre()   - Caseless string prefix comparison.
    sh_sort()    - Sorts an array of strings, many options.
    brstrip()    - Strips enclosing braces (and doublequotes).
    makelist()   - Splits "{{item}{item}...}" into an array.
    makestr()    - Careful malloc() front end.
    xmakestr()   - ditto (see comments).
    ckradix()    - Convert number radix (2-36).
    b8tob64()    - Convert data to base 64.
    b64tob8()    - Convert base 64 to data.
    chknum()     - Checks if string is a (possibly signed) integer.
    rdigits()    - Checks if string is composed only of decimal digits.
    isfloat()    - Checks if string is a valid floating-point number.
    ckround()    - Rounds a floating-point number to desired precision.
    parnam()     - Returns parity name string.
    hhmmss()     - Converts seconds to hh:mm:ss string.
    lset()       - Write fixed-length field left-adjusted into a record.
    rset()       - Write fixed-length field right-adjusted into a record.
    ulongtohex() - Converts an unsigned long to a hex string.
    hextoulong() - Converts a hex string to an unsigned long.
    cksplit()    - Splits a string into an array of words.
    ispattern()  - Tells if argument string is a pattern.

  Prototypes are in ckclib.h.

  Note: This module should not contain any extern declarations.
*/
#include "ckcsym.h"
#include "ckcdeb.h"
#include "ckcasc.h"

/* Public variables */

int dblquo = 1; /* Nonzero if doublequotes can be used for grouping */

char *
ccntab[] = {	/* Names of ASCII (C0) control characters 0-31 */
    "NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL",
    "BS",  "HT",  "LF",  "VT",  "FF",  "CR",  "SO",  "SI",
    "DLE", "DC1", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB",
    "CAN", "EM",  "SUB", "ESC", "FS",  "GS",  "RS",  "US"
};

char *
c1tab[] = {	/* Names of ISO 6429 (C1) control characters 0-32 */
    "XXX", "XXX", "BPH", "NBH", "IND", "NEL", "SSA", "ESA",
    "HTS", "HTJ", "VTS", "PLD", "PLU", "RI",  "SS2", "SS3",
    "DCS", "PU1", "PU2", "STS", "CCH", "MW",  "SPA", "EPA",
    "SOS", "XXX", "SCI", "CSI", "ST",  "OSC", "PM",  "APC", "NBS"
};

#define RXRESULT 127
static char rxdigits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static char rxresult[RXRESULT+1];

/*  C K S T R N C P Y */

/*
  Copies a NUL-terminated string into a buffer whose total length is given,
  ensuring that the result is NUL-terminated even if it has to be truncated.

  Call with:
    dest = pointer to destination buffer
    src  = pointer to source string
    len  = length of destination buffer (the actual length, not one less).

  Returns:
    int, The number of bytes copied, 0 or more.

  NOTE: This is NOT a replacement for strncpy():
   . strncpy() does not require its source string to be NUL-terminated.
   . strncpy() does not necessarily NUL-terminate its result.
   . strncpy() right-pads dest with NULs if it is longer than src.
   . strncpy() treats the length argument as the number of bytes to copy.
   . ckstrncpy() treats the length argument as the size of the dest buffer.
   . ckstrncpy() doesn't dump core if given NULL string pointers.
   . ckstrncpy() returns a number.

  Use ckstrncpy() when you want to:
   . Copy an entire string into a buffer without overrun.
   . Get the length of the string back.

  Use strncpy() when you want to:
   . Copy a piece of a string.
*/
int
#ifdef CK_ANSIC
ckstrncpy(char * dest, const char * src, int len)
#else
ckstrncpy(dest,src,len) char * dest, * src; int len;
#endif /* CK_ANSIC */
{
    int i;
    if (len < 1 || !src || !dest) {	/* Nothing or nowhere to copy */
	if (dest) *dest = NUL;
	return(0);
    }
#ifndef NOCKSTRNCPY
    for (i = 0; src[i] && (i < len-1); i++) /* Args OK, copy */
      dest[i] = src[i];
    dest[i] = NUL;
#else
    i = strlen(src);
    if (i > len) i = len;
    strncpy(dest,src,i);
    dest[len] = NUL;
#endif /* NOCKSTRNCPY */
    return(i);
}

/*  C K S T R N C A T */

/*
  Appends a NUL-terminated string to a buffer whose total length is given,
  ensuring that the result is NUL-terminated even if it had to be truncated.

  Call with:
    dest = pointer to destination buffer containing a null-terminated string
    src  = pointer to null-terminated source string
    len  = length of destination buffer (the actual length, not one less).

  Returns:
    int, The number of bytes copied, 0 or more.
*/
int
#ifdef CK_ANSIC
ckstrncat(char * dest, const char * src, int len)
#else
ckstrncat(dest,src,len) char * dest, * src; int len;
#endif /* CK_ANSIC */
{
    register int i, j;
#ifdef NOCKSTRNCPY
    register char * s1, * s2;
#endif /* NOCKSTRNCPY */
    if (len < 1 || !src || !dest) {	/* Nothing or nowhere to copy */
	if (dest) *dest = NUL;
	return(0);
    }
#ifndef NOCKSTRNCPY
    /* Args OK, copy */
    for (i = 0, j = strlen(dest); src[i] && (i < len-j-1); i++)
      dest[i+j] = src[i];
    dest[i+j] = NUL;
#else
    j = 0;
    s1 = dest;
    while (*s1++) j++;			/* j = strlen(dest); */
    s1--;				/* (back up over NUL) */

    i = 0;
    s2 = (char *)src;
    while (*s2++) i++;			/* i = strlen(src); */

    if (i > (len-j))
      i = len - j;
    if (i <= 0)
      return(0);

#ifdef COMMENT
    strncpy(&dest[j],src,i);
#else
    j = i;				/* This should be a bit faster...    */
    s2 = (char *)src;		       /* depends on strcpy implementation; */
    while ((*s1++ = *s2++) && j--)	/* at least it shouldn't be slower.  */
      ;
    dest[len-1] = NUL;			/* In case of early exit. */
#endif /* COMMENT */

#endif /* NOCKSTRNCPY */
    return(i);
}

/*  C K M A K M S G  */

/*
   Constructs a message from up to 4 pieces with length checking.
   Result is always NUL terminated.  Call with:
     buf: Pointer to buffer for constructing message.
     len: Length of buffer.
     s1-s4: String pointers (can be NULL).
   Returns:
     0: Nothing was copied.
     n: (positive number) n bytes copied, all args copied successfully.
    -n: n bytes were copied, destination buffer not big enough for all.
   Also see:
     ckmakxmsg() -- accepts 12 string args.
     ckitoa(), ckltoa(), ckctoa(), ckitox(), etc.
     Use ckmak[x]msg() plus ck?to?() as a safe replacement for sprintf().
*/
int
#ifdef CK_ANSIC
ckmakmsg(char * buf, int len, char *s1, char *s2, char *s3, char *s4)
#else /* CK_ANSIC */
ckmakmsg(buf,len,s1,s2,s3,s4) char *buf, *s1, *s2, *s3, *s4; int len;
#endif /* CK_ANSIC */
{
    int i, n = 0, m = 0;
    char *s;
    char *p, *a[4];

    if (!buf) return(n);		/* No destination */
    if (len < 1) return(n);		/* No size */

    s = buf;				/* Point to destination */
    a[0] = s1; a[1] = s2; a[2] = s3; a[3] = s4;	/* Array of source strings */
    for (i = 0; i < 4; i++) {		/* Loop thru array */
	p = a[i];			/* Point to this element */
	if (p) {			/* If pointer not null */
	    n = ckstrncpy(s,p,len);	/* Copy safely */
	    m += n;			/* Accumulate total */
	    if (p[n])			/* Didn't get whole thing? */
	      return(-m);		/* return indicating buffer full */
	    len -= n;			/* Deduct from space left */
	    s += n;			/* Otherwise advance dest pointer */
	}
    }
    return(m);				/* Return total bytes copied */
}


/*  C K M A K X M S G  */

/*  Exactly like ckmakmsg(), but accepts 12 string arguments. */

int
#ifdef CK_ANSIC
ckmakxmsg(char * buf, int len,
	  char *s1, char *s2, char *s3, char  *s4, char  *s5, char *s6,
	  char *s7, char *s8, char *s9, char *s10, char *s11, char *s12)
#else /* CK_ANSIC */
ckmakxmsg(buf,len,s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,s12)
  char *buf, *s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12;
  int len;
#endif /* CK_ANSIC */
{
    int i, n = 0, m = 0;
    char *s;
    char *p, *a[12];


    if (!buf) return(n);		/* No destination */
    if (len < 1) return(n);		/* No size */

    s = buf;				/* Point to destination */
    a[0] = s1; a[1] =  s2; a[2]  = s3;  a[3] = s4; /* Source-string array */
    a[4] = s5; a[5] =  s6; a[6]  = s7;  a[7] = s8;
    a[8] = s9; a[9] = s10; a[10] = s11; a[11] = s12;
    for (i = 0; i < 12; i++) {		/* Loop thru array */
	p = a[i];			/* Point to this element */
	if (p) {			/* If pointer not null */
	    n = ckstrncpy(s,p,len);	/* Copy safely */
	    m += n;			/* Accumulate total */
	    if (p[n])			/* Didn't get whole thing? */
	      return(-m);		/* return indicating buffer full */
	    len -= n;			/* Deduct from space left */
	    s += n;			/* Otherwise advance dest pointer */
	}
    }
    return(m);				/* Return total bytes copied */
}

/*  C H A R T O S T R  */

/*  Converts a character to a string, interpreting controls.  */

char *
chartostr(x) int x; {			/* Call with char x */
    static char buf[2];			/* Returns string pointer. */
    if (x < 32)
      return(ccntab[x]);
    if (x == 127)
      return("DEL");
    if (x > 127 && x < 161)
      return(c1tab[x - 128]);
    if (x == 0xAD)
      return("SHY");
    buf[1] = NUL;
    buf[0] = (unsigned)(x & 0xff);
    return((char *)buf);
}

/*  C K R C H A R */

/*  Returns the rightmost character of the given null-terminated string */

int
ckrchar(s) char * s; {
    register CHAR c = '\0', *p;
    p = (CHAR *)s;
    if (!p) p = (CHAR *)"";		/* Null pointer == empty string */
    if (!*p) return(0);
    while (*p)				/* Crawl to end of string */
      c = *p++;
    return((unsigned)(c & 0xff));	/* Return final character */
}

/*  C K S T R C H R  */

/*  Replacement for strchr(), which is not universal.  */
/*  Call with:
     s = pointer to string to look in.
     c = character to look for.
    Returns:
     NULL if c not found in s or upon any kind of error, or:
     pointer to first occurrence of c in s, searching from left to right.
*/
char *
#ifdef CK_ANSIC
ckstrchr(char * s, char c)
#else
ckstrchr(s,c) char *s, c;
#endif /* CK_ANSIC */
/* ckstrchr */ {
    if (!s)
      return(NULL);
    while (*s && *s != c)
      s++;
    return((*s == c) ? s : NULL);
}

/*  C K S T R R C H R  */

/*  Replacement for strrchr(), which is not universal.  */
/*  Call with:
     s = pointer to string to look in.
     c = character to look for.
    Returns:
     NULL if c not found in s or upon any kind of error, or:
     pointer to first occurrence of c in s, searching from right to left.
*/
char *
#ifdef CK_ANSIC
ckstrrchr(char * s, char c)
#else
ckstrrchr(s,c) char *s, c;
#endif /* CK_ANSIC */
/* ckstrchr */ {
    char * s2 = NULL;
    if (!s)
      return(NULL);
    while (*s) {
	if (*s == c)
	  s2 = s;
	s++;
    }
    return(s2);
}


/* C K S T R P B R K  --  Portable replacement for strpbrk()  */

/* Returns pointer to first char in s1 that is also in s2, or NULL */

char *
ckstrpbrk(s1, s2) char * s1, * s2; {
    char c1, c2, * s3;
    if (!s1 || !s2) return(NULL);
    if (!*s1 || !*s2) return(NULL);
    while ((c1 = *s1++)) {
	s3 = s2;
	while ((c2 = *s3++)) {
	    if (c2 == c1)
	      return(s1-1);
	}
    }
    return(NULL);
}

/*  C K L O W E R  --  Lowercase a string IN PLACE */

/* Returns the length of the string */

int
cklower(s) char *s; {
    int n = 0;
    if (!s) return(0);
    while (*s) {
        if (isupper(*s)) *s = (char) tolower(*s);
        s++, n++;
    }
    return(n);
}

/*  C K U P P E R  --  Uppercase a string IN PLACE */

/* Returns the length of the string */

int
ckupper(s) char *s; {
    int n = 0;
    if (!s) return(0);
    while (*s) {
        if (islower(*s)) *s = (char) toupper(*s);
        s++, n++;
    }
    return(n);
}

/*  C K L T O A  --  Long to string  --  FOR DISCIPLINED USE ONLY  */

#define NUMBUF 1024
static char numbuf[NUMBUF+32] = { NUL, NUL };
static int numbp = 0;
/*
  ckltoa() and ckitoa() are like atol() and atoi() in the reverse direction,
  returning a pointer to the string representation of the given number without
  the caller having to worry about allocating or defining a buffer first.
  They manage their own internal buffer, so successive calls return different
  pointers.  However, to keep memory consumption from growing without bound,
  the buffer recycles itself.  So after several hundred calls (depending on
  the size of the numbers), some of the earlier pointers might well find
  themselves referencing something different.  Moral: You can't win in C.
  Therefore, these routines are intended mainly for generating numeric strings
  for short-term use, e.g. for passing numbers in string form as parameters to
  functions.  For long-term use, the result must be copied to a safe place.
*/
char *
#ifdef CK_ANSIC
ckltoa(long n)
#else
ckltoa(n) long n;
#endif /* CK_ANSIC */
/* ckltoa */ {
    char buf[32];			/* Internal working buffer */
    char * p, * s, * q;
    int k, x, len = 0, sign = 0;
    if (n < 0L) {			/* Sign */
	n = 0L - n;
	sign = 1;
    }
    buf[31] = NUL;
    for (k = 30; k > 0; k--) {		/* Convert number to string */
	x = n % 10L;
	buf[k] = x + '0';
	n = n / 10L;
	if (!n)
	  break;
    }
    if (sign) buf[--k] = '-';		/* Add sign if necessary */
    len = 31 - k;
    if (len + numbp > NUMBUF)
      numbp = 0;
    p = numbuf + numbp;
    q = p;
    s = buf + k;
    while ((*p++ = *s++)) ;		/* Copy */
    *p++ = NUL;
    numbp += len+1;
    return(q);				/* Return pointer */
}

/*  C K U L T O A  --  Unsigned long to string  */

char *
#ifdef CK_ANSIC
ckultoa(unsigned long n)
#else
ckultoa(n) unsigned long n;
#endif /* CK_ANSIC */
/* ckultoa */ {
    char buf[32];			/* Internal working buffer */
    char * p, * s, * q;
    int k, x, len = 0;
    buf[31] = NUL;
    for (k = 30; k > 0; k--) {		/* Convert number to string */
	x = n % 10L;
	buf[k] = x + '0';
	n = n / 10L;
	if (!n)
	  break;
    }
    len = 31 - k;
    if (len + numbp > NUMBUF)
      numbp = 0;
    p = numbuf + numbp;
    q = p;
    s = buf + k;
    while ((*p++ = *s++)) ;		/* Copy */
    numbp += len+1;
    return(q);				/* Return pointer */
}

char *
#ifdef CK_ANSIC
ckltox(long n)				/* Long int to "0x.." hex string */
#else
ckltox(n) long n;
#endif /* CK_ANSIC */
/* ckltox */ {
    char buf[32];			/* Internal working buffer */
    char *p, *q, *s, *bp = buf + 2;
    int k;
    buf[0] = '0';
    buf[1] = 'x';
    sprintf(bp, "%lx", n);
    k = strlen(bp);
    if (k&1) {
	sprintf(bp, "0%lx", n);
	k++;
    }
    k += 2;				/* "0x" */
    if (numbp + k >= NUMBUF)
      numbp = 0;
    p = numbuf + numbp;
    q = p;
    s = buf;
    while ((*p++ = *s++)) ;		/* Copy */
    *p++ = NUL;
    numbp += k+1;
    return(q);				/* Return pointer */
}


/*  C K F S T O A  --  File Size (or offset) to string  */

/* This is just like ckltoa() except for the data type of the argument. */
/* It's mainly for printing file sizes without having to know their data */
/* type, so we don't have to hardware "%ld" or "%lld" into printf()s. */
/* Works for 32 or 64 bits, according to CK_OFF_T definition. */

char *
#ifdef CK_ANSIC
ckfstoa(CK_OFF_T n)
#else
ckfstoa(n) CK_OFF_T n;
#endif /* CK_ANSIC */
/* ckfstoa */ {
    char buf[32];			/* Internal working buffer */
    char * p, * s, * q;
    int k, x, len = 0, sign = 0;

    if (n < (CK_OFF_T)0) {		/* Sign */
	n = (CK_OFF_T)0 - n;
	sign = 1;
    }
    buf[31] = NUL;			/* 2^63-1 is about 20 decimal digits */
    for (k = 30; k > 0; k--) {		/* Convert number to string */
	x = n % (CK_OFF_T)10;
	if (x < 0) {
	    /* x += 10; */
	    ckstrncpy(&buf[23],"OVERFLOW",32);
	    sign = 0;
	    k = 23;
	    break;
	}
	buf[k] = x + '0';
	n = n / (CK_OFF_T)10;
	if (!n)
	  break;
    }
    if (sign) buf[--k] = '-';		/* Add sign if necessary */
    len = 31 - k;
    if (len + numbp > NUMBUF)
      numbp = 0;
    p = numbuf + numbp;
    q = p;
    s = buf + k;
    while ((*p++ = *s++)) ;		/* Copy */
    *p++ = NUL;
    numbp += len+1;
    return(q);				/* Return pointer */
}

/*  C K A T O F S  --  String to File Size (or offset) */

/* This is the inverse of ckfstoa(), a replacement for atol() that works */
/* for either 32-bit or 64-bit arguments, according to CK_OFF_T definition. */
/* Like atol(), there is no error indication. */

CK_OFF_T
#ifdef CK_ANSIC
ckatofs(char * s)
#else
ckatofs(s) char * s;
#endif /* CK_ANSIC */
/* ckatofs */ {
    CK_OFF_T result = (CK_OFF_T)0;
    int minus = 0;
    while (*s && (*s == SP || *s == HT)) s++;
    if (*s == '+') s++;
    if (*s == '-') {
	minus = 1;
	s++;
    }
    while (isdigit(*s)) {
	result = (result * (CK_OFF_T)10) + (CK_OFF_T)(*s - '0');
	s++;
    }
    return(minus ? -result : result);
}

/*  C K I T O A  --  Int to string  -- FOR DISCIPLINED USE ONLY  */

char *
ckitoa(n) int n; {			/* See comments with ckltoa(). */
    long nn;
    nn = n;
    return(ckltoa(nn));
}


char *					/* Unsigned int to string */
ckuitoa(n) unsigned int n; {
    unsigned long nn;
    nn = n;
    return(ckultoa(nn));
}

char *
ckitox(n) int n; {			/* Int to hex */
    long nn;
    nn = n;
    return(ckltox(nn));
}

char *
#ifdef CK_ANSIC
ckctoa(char c)				/* Char to string */
#else
ckctoa(c) char c;
#endif
/* ckctoa */ {
    static char buf[32];
    static int current = 0;
    if (current >= 30)
      current = 0;
    buf[current++] = c;
    buf[current++] = '\0';
    return((char *)(buf + current - 2));
}

char *
#ifdef CK_ANSIC
ckctox(CHAR c, int flag)		/* Unsigned char to hex */
#else
ckctox(c, flag) CHAR c; int flag;
#endif
/* ckctox */ {
    static char buf[48];
    static int current = 0;
    int x;
    char h;
    if (current > 45)
      current = 0;
    x = (c >> 4) & 0x0f;
    h = rxdigits[x];
    if (!flag && isupper(rxdigits[x]))
      h = tolower(rxdigits[x]);
    buf[current++] = h;
    x = c & 0x0f;
    h = rxdigits[x];
    if (!flag && isupper(rxdigits[x]))
      h = tolower(rxdigits[x]);
    buf[current++] = h;
    buf[current++] = '\0';
    return((char *)(buf + current - 3));
}

/*  C K I N D E X  --  C-Kermit's index function  */
/*
  We can't depend on C libraries to have one, so here is our own.
  Call with:
    s1 - String to look for.
    s2 - String to look in.
     t - Offset from right or left of s2, 0 based; -1 for rightmost char in s2.
     r - 0 for left-to-right search, non-0 for right-to-left.
  icase  0 for case independence, non-0 if alphabetic case matters.
  Returns 0 if string not found, otherwise a 1-based result.
  Also returns 0 on any kind of error, e.g. junk parameters.
*/
int
ckindex(s1,s2,t,r,icase) char *s1, *s2; int t, r, icase; {
    int len1 = 0, len2 = 0, i, j, x, ot = t; /* ot = original t */
    char * s;

    if (!s1 || !s2) return(0);
    s = s1;
    while (*s++) len1++;		/* length of string to look for */
    s = s2;
    while (*s++) len2++;		/* length of string to look in */
    s = s2;
    if (t < 0) t = len2 - 1;

    j = len2 - len1;			/* length difference */

    if (j < 0 || (r == 0 && t > j))	/* search string is longer */
      return(0);
    if (r == 0) {			/* Index */
	s = s2 + t;			/* Point to beginning of target */
	for (i = 0; i <= (j - t); i++) { /* Now compare */
	    x = ckstrcmp(s1,s++,len1,icase);
	    if (!x)
	      return(i+1+t);
	}
    } else {				/* Reverse Index */
        i = len2 - len1;		/* Where to start looking */
        if (ot > 0)			/* Figure in offset if any */
	  i -= t;
	for (j = i; j > -1; j--) {
	    if (!ckstrcmp(s1,&s2[j],len1,icase))
	      return(j+1);
	}
    }
    return(0);
}

/*  C K S T R S T R  --  Portable replacement for strstr()  */

/*  Returns pointer to first occurrence of s1 in s2, or NULL */

char *
ckstrstr(s1, s2) char * s1, * s2; {
    int k;
    k = ckindex(s2,s1,0,0,1);
    return((k < 1) ? NULL : &s1[k-1]);
}


/*  B R S T R I P  --  Strip enclosing braces from arg string, in place. */
/*
  Call with:
    Pointer to string that can be poked.
  Returns:
    Pointer to string without enclosing braces.
    If original string was not braced, this is the arg pointer;
    otherwise it is 1 + the arg pointer, with the matching closing
    brace zero'd out.  If the string starts with a brace but does
    not end with a matching brace, the original pointer to the original
    string is returned.  If the arg pointer is NULL, a pointer to an
    empty string is returned.
*/
#ifdef COMMENT

/* This is the original version, handling only braces */

char *
brstrip(p) char *p; {
    if (!p) return("");
    if (*p == '{') {
	int x;
	x = (int)strlen(p) - 1;
	if (p[x] == '}') {
	    p[x] = NUL;
	    p++;
	}
    }
    return(p);
}

#else
/* New version handles braces and doublequotes */
/* WARNING: this function writes into its argument, it always has. */

char *
brstrip(p) char *p; {
    if (!p) return("");
    if (*p == '{' || (*p == '"' && dblquo)) {
	int x;
	x = (int)strlen(p) - 1;
	if (x > 0) {
	    if ((*p == '{' && p[x] == '}') ||
		(*p == '"' && p[x] == '"')) {
		if (x > 0 && p[x-1] != CMDQ) {
		    p[x] = NUL;
		    p++;
		}
	    }
	}
    }
    return(p);
}
#endif /* COMMENT */

#ifdef COMMENT

/* Even newer experimental version -- breaks many things */

char *
fnstrip(p) char *p; {
    int i, j, k, n, len;
    extern int cmd_quoting;		/* Bad - no externs allowed! */

    if (!p)
      return("");

    if (*p == '{') {
        len = strlen(p);
        n = 0;

        for (j = 0; j < len; j++ ) {
            if (p[j] == '{' &&
		(!cmd_quoting || j == 0 || p[j-1] != CMDQ)) {
                for (n = 1, i = j+1; i < len; i++ ) {
                    if (p[i] == '{' && (!cmd_quoting || p[i-1] != CMDQ))
		      n++;
                    else if (p[i] == '}' && (!cmd_quoting || p[i-1] != CMDQ)) {
                        if (--n == 0) {
                            for (k = j; k < i - 1; k++)
			      p[k] = p[k+1];
                            for (; i < len; i++ )
			      p[i-1] = p[i+1];
                            len -= 2;
                            j = i - 1;
                        }
                    }
                }
            }
        }
        if (n == 1) { /* Implied right brace at end of field */
            for (k = j; k < len; k++)
	      p[k] = p[k+1];
            len -= 1;
        }
    } else if (*p == '"') {
        len = strlen(p);
        n = 0;

        for (j = 0; j < len; j++) {
            if (p[j] == '"' &&
		(!cmd_quoting || j == 0 || p[j-1] != CMDQ)) {
                n++;

                for (i = j + 1; i < len; i++) {
                    if (p[i] == '"' && (!cmd_quoting || p[i-1] != CMDQ)) {
                        n--;

                        for (k = j; k < i - 1; k++)
			  p[k] = p[k+1];
                        for (; i < len; i++)
			  p[i-1] = p[i+1];
                        len -= 2;
                        j = i - 1;
                    }
                }
            }
        }
        if (n == 1) { /* Implied double quote at end of field */
            for (k = j; k < len; k++ )
	      p[k] = p[k+1];
            len -= 1;
        }
    }
    return(p);
}
#endif /* COMMENT */

#ifdef COMMENT
/*
  Not used -- Note: these not only write into their arg, but write past
  past the end.
*/
char *
brace(fn) char *fn; {
    int spaces = 0;
    char * p, ch, ch2;
    for (p = fn; *p; p++) {
	if (*p == SP) {
	    spaces = 1;
	    break;
	}
    }
    if (spaces) {
        p = fn;
        ch = *p;
        *p = '{';
        p++;

        while (*p) {
            ch2 = *p;
            *p = ch;
            ch = ch2;
            p++;
        }
        *p = ch;
        p++;
        *p = '}';
        p++;
        *p = '\0';
    }
    return(fn);
}
#endif /* COMMENT */

/* d q u o t e  --  Puts doublequotes around arg in place. */
/*
   Call with:
     Pointer to buffer and its total length and flag = 0 to use
     doublequotes, 1 to use braces.
   Returns:
     Number: length of result.
*/
int
dquote(fn, len, flag) char *fn; int len; int flag; {
    int spaces = 0, k = 0;
    char * p, ch, ch2;
    if (!fn)
      return(0);

    k = strlen(fn);
    for (p = fn; *p; p++) {
	if (*p == SP) {
            spaces = 1;
            break;
        }
    }
    if (spaces) {
	if (k + 2 >= len)
	  return(k);
        p = fn;
        ch = *p;
        *p = flag ? '{' : '"';
        p++;

        while (*p) {
            ch2 = *p;
            *p = ch;
            ch = ch2;
            p++;
        }
        *p = ch;
        p++;
        *p = flag ? '}' : '"';
        p++;
        *p = '\0';
    }
    return(k+2);
}


/*  U N T A B I F Y  ---  Untabify s1 into s2, assuming tabs every 8 space */

int
untabify(s1,s2,max) char * s1, * s2; int max; {
    int i, j, k, x, z;
    x = strlen(s1);
    for (i = 0, k = 0; k < x; k++) {
	if (s1[k] != '\t') {
	    if (i >= max-1) {
		s2[max-1] = '\0';
		return(-1);
	    }
	    s2[i++] = s1[k];
	    continue;
	}
	z = 8 - i%8;
	if (z == 0) z = 8;
	for (j = 0; j < z && i < max; j++)
	  s2[i++] = ' ';
    }
    s2[i] = '\0';
    return(0);
}


/*  M A K E L I S T  ---  Breaks {{s1}{s2}..{sn}} into an array of strings */
/*
  Call with:
    s    = pointer to string to break up.
    list = array of string pointers.
    len  = number of elements in array.
  NOTE: The array must be preinitialized to all NULL pointers.
  If any array element is not NULL, it is assumed to have been malloc'd
  and is therefore freed.  Do NOT call this function with an uninitialized
  array, or with an array that has had any static elements assigned to it.
*/
VOID
makelist(s,list,len) char * s; char *list[]; int len; {
    int i, n, q, bc = 0;
    char *p = NULL, *s2 = NULL;
    debug(F110,"makelist s",s,0);
    if (!s) {				/* Check for null or empty string */
	list[0] = NULL;
	return;
    }
    n = strlen(s);
    if (n == 0) {
	list[0] = NULL;
	return;
    }
    if ((s2 = (char *)malloc(n+1))) {	/* Safe copy for poking */
	strcpy(s2,s);			/* (no need for ckstrncpy here) */
	s = s2;
    }
    s = brstrip(s);			/* Strip braces */
    n = strlen(s);			/* Get length */
    if (*s != '{') {			/* Outer braces only */
	if ((p = (char *)malloc(n+1))) { /* So just one pattern */
	    strcpy(p,s);		/* (no need for ckstrncpy here) */
	    if (list[0])
	      free(list[0]);
	    list[0] = p;
	}
	if (s2) free(s2);
	return;
    }
    q = 0;				/* Inner ones too */
    i = 0;				/* so a list of patterns. */
    n = 0;
    while (*s && i < len) {
	if (*s == CMDQ) {		/* Quote... */
	    q = 1;
	    s++;
	    n++;
	    continue;
	}
	if (*s == '{' && !q) {		/* Opening brace */
	    if (bc++ == 0) {		/* Beginning of a group */
		p = ++s;
		n = 0;
	    } else {			/* It's a brace inside the group */
		n++;
		s++;
	    }
	    continue;
	} else if (*s == '}' && !q) {	/* Closing brace */
	    if (--bc == 0) {		/* End of a group */
		*s++ = NUL;
		debug(F111,"makelist element",p,i);
		if (list[i])
		  free(list[i]);
		if ((list[i] = (char *)malloc(n+1))) {
		    ckstrncpy(list[i],p,n+1); /* Note: n+1 */
		    i++;
		}
		while (*s == SP) s++;
		p = s;
		n = 0;
		continue;
	    } else {			/* Within a group */
		n++;
		s++;
	    }
	} else {			/* Regular character */
	    q = 0;
	    s++;
	    n++;
	}
    }
    if (*p && i < len) {		/* Last one */
	if (list[i])
	  free(list[i]);
	if ((list[i] = (char *)malloc(n+1))) {
	    ckstrncpy(list[i],p,n+1);
	    debug(F111,"makelist last element",p,i);
	}
    }
    i++;				/* Clear out the rest of the list */
    for ( ; i < len; i++) {
	if (list[i])
	  free (list[i]);
	list[i] = NULL;
    }
    if (s2) free(s2);
}

/*
   M A K E S T R  --  Creates a dynamically allocated string.

   Makes a new copy of string s and sets pointer p to its address.
   Handles degenerate cases, like when buffers overlap or are the same,
   one or both arguments are NULL, etc.

   The source string is assumed to be NUL-terminated.  Therefore it can not
   be a UCS-2 string or arbitrary binary data.

   The target pointer must be either NULL or else a pointer to a previously
   malloc'ed buffer.  If not, expect a core dump or segmentation fault.

   Note: The caller can tell whether this routine failed as follows:

     malloc(&p,q);
     if (q & !p) { makestr() failed };

   Really this routine should have returned a length, but since it doesn't
   we set the global variable makestrlen to the length of the result string.
*/
int makestrlen = 0;

VOID
#ifdef CK_ANSIC
makestr(char **p, const char *s)
#else
makestr(p,s) char **p, *s;
#endif
/* makestr */ {
    int x = 0;
    char *q = NULL;
#ifdef CK_ANSIC
    register const char * s2;
#else
    register char * s2;
#endif /* CK_ANSIC */
    register char * q2;

    if (*p == s)			/* The two pointers are the same. */
      return;				/* Don't do anything. */

    if (!s) {				/* New definition is null? */
	if (*p)				/* Free old storage. */
	  free(*p);
	*p = NULL;			/* Return null pointer. */
	makestrlen = 0;
	return;
    }
    s2 = s;				/* Maybe new string will fit */

#ifdef COMMENT
/*
  This is a fairly big win, allowing us to skip the malloc() and free if the
  destination string already exists and is not shorter than the source string.
  But it doesn't allow for possible overlap of source and destination.
*/
    if (*p) {				/* into old storage... */
	char * p2 = *p;
	char c;
	while (c = *p2) {
	    if (!(*p2++ = *s2++))
	      break;
	    x++;
	}
	makestrlen = x;
	if (c) return;
    }
#endif /* COMMENT */

/* Didn't fit */

    x = 0;
    while (*s2++) x++;			/* Get (rest of) length of s.  */

    if (x >= 0) {			/* Get length, even of empty string. */
	q = malloc(x+1);		/* Get and point to temp storage. */
	if (q) {
	    makestrlen = x;		/* Remember length for stats */
	    s2 = s;			/* Point back to beginning of source */
	    q2 = q;			/* Copy dest pointer to increment... */
	    while ((*q2++ = *s2++)) ;	/* Instead of calling strcpy(). */
/*
  Note: HP flexelint says that the above loop can result in creation (++) and
  access (*) of out-of-bounds pointers.  I really don't see it.
*/
	}
#ifdef DEBUG
	else {				/* This would be a really bad error */
	    char tmp[24];		/* So get a good record of it. */
	    if (x > 23) {
		ckstrncpy(tmp,s,20);
		strcpy(tmp+20,"...");
		tmp[23] = NUL;
	    } else {
		strcpy(tmp,s);		/* We already checked the length */
	    }
	    debug(F110,"MAKESTR MALLOC FAILURE ",tmp,0);
	}
#endif /* DEBUG */
    } else
      q = NULL;				/* Length of string is zero */

    if (*p)				/* Now free the original storage. */
      free(*p);
    *p = q;
}

/*  X M A K E S T R  --  Non-destructive makestr() if s is NULL.  */

VOID
#ifdef CK_ANSIC
xmakestr(char **p, const char *s)
#else
xmakestr(p,s) char **p, *s;
#endif
/* xmakestr */ {
    if (s) makestr(p,s);
}

#ifndef USE_MEMCPY
/* C K M E M C P Y  --  Portable (but slow) memcpy() */

/* Copies n bytes from s to p, allowing for overlap. */
/* For use when real memcpy() not available. */

VOID
ckmemcpy(p,s,n) char *p, *s; int n; {
    char * q = NULL;
    register int i;
    int x;

    if (!s || !p || n <= 0 || p == s)	/* Verify args */
      return;
    x = p - s;				/* Check for overlap */
    if (x < 0)
      x = 0 - x;
    if (x < n) {			/* They overlap */
	q = p;
	if (!(p = (char *)malloc(n)))	/* So use a temporary buffer */
	  return;
    }
    for (i = 0; i < n; i++)		/* Copy n bytes */
      p[i] = s[i];
    if (q) {				/* If we used a temporary buffer */
	for (i = 0; i < n; i++)		/* copy from it to destination */
	  q[i] = p[i];
	if (p) free(p);			/* and free the temporary buffer */
    }
}
#endif /* USE_MEMCPY */


/*  C K S T R C M P  --  String comparison with case-matters selection */
/*
  Call with pointers to the two strings, s1 and s2, a length, n,
  and c == 0 for caseless comparison, nonzero for case matters.
  Call with n == -1 to compare without a length limit.
  Compares up to n characters of the two strings and returns:
    1 if s1 > s2
    0 if s1 = s2
   -1 if s1 < s2
  Note: case handling is only as good as isupper() and tolower().
*/
int
ckstrcmp(s1,s2,n,c) char *s1, *s2; register int n, c; {
    register CHAR t1, t2;
    if (n == 0) return(0);
    if (!s1) s1 = "";			/* Watch out for null pointers. */
    if (!s2) s2 = "";
    if (!*s1) return(*s2 ? -1 : 0);
    if (!*s2) return(1);
    while (n--) {
	t1 = (CHAR) *s1++;		/* Get next character from each. */
	t2 = (CHAR) *s2++;
	if (!t1) return(t2 ? -1 : 0);
	if (!t2) return(1);
	if (!c) {			/* If case doesn't matter */
	    if (isupper(t1)) t1 = tolower(t1); /* Convert case. */
	    if (isupper(t2)) t2 = tolower(t2);
	}
	if (t1 < t2) return(-1);	/* s1 < s2 */
	if (t1 > t2) return(1);		/* s1 > s2 */
    }
    return(0);				/* They're equal */
}

/*  C K S T R P R E  --  Caseless string prefix comparison  */

/* Returns position of the first char in the 2 strings that doesn't match */

int
ckstrpre(s1,s2) char *s1, *s2; {
    CHAR t1, t2;
    int n = 0;
    if (!s1) s1 = "";
    if (!s2) s2 = "";
    while (1) {
	t1 = (CHAR) *s1++;
	t2 = (CHAR) *s2++;
	if (!t1 || !t2) return(n);
	if (isupper(t1)) t1 = tolower(t1);
	if (isupper(t2)) t2 = tolower(t2);
	if (t1 != t2)
	  return(n);
	n++;
    }
}

#define GLOBBING

/*  C K M A T C H  --  Match a string against a pattern  */
/*
  Call with:
    pattern to be matched.
    string to look for the pattern in.
    icase is 1 if case-sensitive, 0 otherwise.
    opts is a bitmask:
      Bit 0 (=1):
	1 = Match strings starting with '.'
	0 = Don't match them (used with UNIX filenames).
      Bit 1 (=2):
	1 = File globbing (dirseps are fences);
	0 = Dirseps are not fences.
      Bit 2 (=4):
	1 = Allow ^ and $ anchors at beginning and end of pattern.
	0 = Don't allow them (normal case for filename matching).
      Bit 3 (and beyond): Undefined.
  Works only with NUL-terminated strings.
  Pattern may contain any number of ? and/or *.
  If CKREGEX is defined, also [abc], [a-z], and/or {string,string,...}.
  (Note: REGEX is a misnomer, see below.)

  Returns:
    0 if string does not match pattern,
    >= 1, the 1-based position in the string where the match was found.

  To be done:
    Find a way to identify the piece of the string that matched the pattern,
    as in Snobol "LINE (PAT . RESULT)".  This is now partially done by
    setting matchpos and matchend (except matchend needs some tuning).  But
    these are useless unless a copy of the string is kept, or a copy of the
    matching part is made.  But that would be too costly in performance --
    this routine has to be fast because it's used for wildcard expansion.

  Note:
    Patterns are not the same as regular expressions, in which '*' means
    0 or more repetitions of the preceding item.  For example "a*b" as a
    pattern matches any string that starts with 'a' and ends with 'b'; as a
    regular expression it matches any string of zero or more a's followed by
    one b.  Regular expressions are especially useful in matching strings of
    (say) digits, or letters, e.g. "[0-9]*" matches any string of digits.
    So far, Kermit doesn't do this.
*/
static char * mypat = NULL;		/* For rewriting pattern */
static int matchpos = 0;
int matchend = 0;
static int matchdepth = 0;
static int stringpos = 0;
static char * ostring = NULL;

#define MATCHRETURN(x,y) { rc=y; where=x; goto xckmatch; }
static char * lastpat = NULL;

static int xxflag = 0;			/* Global bailout flag for ckmatch() */

int
ispattern(s) char * s; {
    int quote = 0, sbflag = 0, sb = 0, cbflag = 0, cb = 0;
    
    char c = 0;
    if (*s == '^') return(1);
    while ((c = *s++)) {
	if (quote) {
	    quote = 0;
	    continue;
	}
	if (c == '\\') {
	    quote = 1;
	    continue;
	}
	if (c == '*') return(1);
	if (c == '?') return(1);
	/* Unquoted brackets or braces must match */
	if (c == '[') { sbflag++; sb++; continue; }
	if (c == ']') { sb--; continue; }
	if (c == '{') { cbflag++; cb++; continue; }
	if (c == '}') { cb--; continue; }
	if (!*s && c == '$') return(1);
    }
    return(sbflag || cbflag);
}

int
ckmatch(pattern, string, icase, opts) char *pattern,*string; int icase, opts; {
    int q = 0, i = 0, k = -1, x, flag = 0;
    int rc = 0;				/* Return code */
    int havestar = 0;
    int where = -1;
    CHAR cp;				/* Current character from pattern */
    CHAR cs;				/* Current character from string */
    char * patstart;			/* Start of pattern */
    int plen, dot, globbing, xstar = 0;
    int bronly = 0;			/* Whole pattern is {a,b,c,...} */

    debug(F111,"CKMATCH ENTRY pat opt",pattern,opts);
    debug(F111,"CKMATCH ENTRY str dep",string,matchdepth);
    /* debug(F101,"CKMATCH ENTRY icase","",icase); */

    globbing = opts & 2;

    if (!string) string = "";
    if (!pattern) pattern = "";

    if (!*pattern) {			/* Empty pattern matches anything */
	matchdepth++;			/* (it wasn't incremented yet) */
	MATCHRETURN(0,1);
    } else if (!*string) {
	MATCHRETURN(0,0);
    }
    patstart = pattern;			/* Remember beginning of pattern */

    if (matchdepth == 0) {		/* Top-level call? */
	xxflag = 0;
	stringpos = 0;			/* Reset indices etc. */
	matchpos = 0;
	matchend = 0;
	ostring = string;
	lastpat = pattern;
	if (*pattern == '{')		/* Entire pattern is {a,b.c} */
	  bronly = 1;			/* Maybe */
	dot = (opts & 1) ||		/* Match leading dot (if file) */
	    ((opts & 2) == 0) ||	/* always if not file */
	    (pattern[0] == '.');	/* or if pattern starts with '.' */

	plen = strlen(pattern);		/* Length of pattern */
/* This would be used in calculating length of matching segment */
	if (plen > 0)			/* User's pattern ends with '*' */
	  if (pattern[plen - 1] == '*')
	    xstar = 1;
	if (pattern[0] == '*') {	/* User's pattern starts with '*' */
	    matchpos = 1;
	    debug(F111,"CKMATCH 1",string, matchpos);
	}
	if (opts & 4) {			/* ^..$ allowed (top level only) */
	    /* Rewrite pattern to account for ^..$ anchoring... */

	    if (mypat) free(mypat);	/* Get space for "*pattern*" */
	    mypat = (char *)malloc(plen + 4);
	    if (mypat) {		/* Got space? */
		char * s = pattern, * p = mypat; /* Set working pointers */
		if (*s == '^') {	/* First source char is ^ */
		    s++;		/* so skip past it */
		} else if (*s != '*') {	/* otherwise */
		    *p++ = '*';		/* prepend '*' to pattern */
		}
		while (*s) {		/* Copy rest of pattern */
		    if (!*(s+1)) {	/* Final pattern character? */
			if (*s != '$') { /* If it's not '$' */
			    *p++ = *s;	/* Copy it into the pattern */
			    if (*s++ != '*') /* And if it's also not '*' */
			      *p++ = '*'; /* append '*'. */
			}
			break;		/* Done */
		    } else		/* Not final character */
		      *p++ = *s++;	/* Just copy it */
		}
		*p = NUL;		/* Terminate the new pattern */
		pattern = mypat;	/* Make the switch */
	    }
	    debug(F110,"CKMATCH INIT pat",pattern,0);
	}
    }
    matchdepth++;			/* Now increment call depth */

#ifdef UNIX
    if (!dot) {				/* For UNIX file globbing */
	if (*string == '.' && *pattern != '.' && !matchdot) {
	    if (
#ifdef CKREGEX
		*pattern != '{' && *pattern != '['
#else
		1
#endif /* CKREGEX */
		) {
		debug(F110,"ckmatch skip",string,0);
		MATCHRETURN(1,0);
	    }
	}
    }
#endif /* UNIX */
    while (1) {
	k++;
	cp = *pattern;			/* Character from pattern */
	cs = *string;			/* Character from string */

#ifdef COMMENT
	debug(F000,"CKMATCH pat cp",pattern,cp);
	debug(F000,"CKMATCH str cs",string,cs);
#endif /* COMMENT */

	if (!cs) {			/* End of string - done. */
	    x = (!cp || (cp == '*' && !*(pattern+1))) ? 1 : 0;
	    if (x) {
		if (!matchpos) {
		    matchpos = stringpos;
		    debug(F111,"CKMATCH A",string, matchpos);
		}
		matchend = stringpos;
		MATCHRETURN(2,matchpos);
	    }
	    debug(F111,"CKMATCH ZERO d",string, matchpos);
	    matchpos = 0;
	    MATCHRETURN(16,matchpos);
	}
        if (!icase) {			/* If ignoring case */
	    if (isupper(cp))		/* convert both to lowercase. */
	      cp = tolower(cp);
	    if (isupper(cs))
	      cs = tolower(cs);
        }
	if (q) {			/* This character was quoted */
	    debug(F000,"CKMATCH QUOTED",pattern,cp);
	    q = 0;			/* Turn off quote flag */

	    if (cs == cp) {		/* Compare directly */
		if (!matchpos) {	/* Matches */
		    matchpos = stringpos;
		    debug(F111,"CKMATCH \\ new match",string, matchpos);
		}
		pattern++;
	    } else {			/* Doesn't match */
		pattern = lastpat;	/* Back up the pattern */
		matchpos = 0;
		debug(F111,"CKMATCH \\ no match",pattern, matchpos);
	    }
	    string++;
	    stringpos++;
	    continue;
	}
	if (cp == CMDQ && !q) {		/* Quote in pattern */
	    debug(F000,"CKMATCH QUOTE",pattern,cp);
	    q = 1;			/* Set flag */
	    pattern++;			/* Advance to next pattern character */
	    continue;			/* and continue. */
	}
	if (cs && cp == '?') {		/* '?' matches any char */
	    if (!matchpos) {
		matchpos = stringpos;
		debug(F111,"CKMATCH D",string, matchpos);
	    }
	    debug(F110,"CKMATCH ? pat",pattern,0);
	    debug(F110,"CKMATCH ? str",string,0);
	    pattern++;
	    string++;
	    stringpos++;
	    continue;
#ifdef CKREGEX
	} else if (cp == '[') {		/* Have bracket */
	    int q = 0;			/* My own private q */
	    char * psave = NULL;	/* and backup pointer */
	    CHAR clist[256];		/* Character list from brackets */
	    CHAR c, c1, c2;

	    for (i = 0; i < 256; i++)	/* memset() etc not portable */
	      clist[i] = NUL;
	    psave = ++pattern;		/* Where pattern starts */
	    debug(F111,"CKMATCH [] ",pattern-1, matchpos);
	    for (flag = 0; !flag; pattern++) { /* Loop thru pattern */
		c = (CHAR)*pattern;	/* Current char */
		debug(F000,">>> pattern char","",c);
		if (q) {		/* Quote within brackets */
		    q = 0;
		    clist[c] = 1;
		    continue;
		}
		if (!icase)		/* Case conversion */
		  if (isupper(c))
		    c = tolower(c);
		switch (c) {		/* Handle unquoted character */
		  case NUL:		/* End of string */
		    MATCHRETURN(4,0);	/* No matching ']' so fail */
		  case CMDQ:		/* Next char is quoted */
		    q = 1;		/* Set flag */
		    continue;		/* and continue. */
		  case '-':		/* A range is specified */
		    c1 = (pattern > psave) ? (CHAR)*(pattern-1) : NUL;
		    c2 = (CHAR)*(pattern+1); /* IGNORE OUT-OF-BOUNDS WARNING */
		    if (c2 == ']') c2 = NUL; /* (it can't happen) */
		    if (c1 == NUL) c1 = c2;
		    for (c = c1; c <= c2; c++) {
			clist[c] = 1;
			if (!icase) {
			    if (islower(c)) {
				clist[toupper(c)] = 1;
			    } else if (isupper(c)) {
				clist[tolower(c)] = 1;
			    }
			}
		    }
		    continue;
		  case ']':		/* End of bracketed sequence */
		    flag = 1;		/* Done with FOR loop */
		    break;		/* Compare what we have */
		  default:		/* Just a char */
		    clist[c] = 1;	/* Record it */
		    if (!icase) {
			if (islower(c)) {
			    clist[toupper(c)] = 1;
			} else if (isupper(c)) {
			    clist[tolower(c)] = 1;
			}
		    }
		    continue;
		}
	    }
	    debug(F000,">>> cs","",cs);
	    debug(F101,">>> clist[cs]","",clist[cs]);
	    debug(F000,">>> string",string,*string);

	    if (!clist[(unsigned)cs]) {	/* No match? */
		if (!*string) {		/* This clause 16 Jun 2005 */
		    MATCHRETURN(5,0);	/* Nope, done. */
		}
/*
  We need to fail here if the [clist] is not allowed to float.
  The [clist] is not allowed to float if it is not preceded
  by an asterisk, right?  30 Dec 2005.
*/
		if (!havestar) {
		    MATCHRETURN(500,0);
		}
		string++;		/* From here to end added 2005/6/15 */
		stringpos++;
		pattern = lastpat;	/* Back up pattern */
		k = ckmatch(pattern,string,icase,opts);
		if (xxflag) MATCHRETURN(0,0);
		if (!matchpos && k > 0)
		  matchpos = stringpos;
		MATCHRETURN(5, (*string) ? matchpos : 0);
	    }
	    if (!matchpos) {
		matchpos = stringpos;
		debug(F111,"CKMATCH [] match",string, matchpos);
	    }
	    string++;			/* Yes, advance string pointer */
	    stringpos++;
	    continue;			/* and go on. */
	} else if (cp == '{') {		/* Braces enclosing list of strings */
	    char * p, * s, * s2, * buf = NULL;
	    int n, bc = 0;
	    int len = 0;
	    debug(F111,"CKMATCH {} ",string, matchpos);
	    for (p = pattern++; *p; p++) {
		if (*p == '{') bc++;
		if (*p == '}') bc--;
		if (bc < 1) break;
	    }
	    if (bc != 0) {		/* Braces don't match */
		MATCHRETURN(6,0);	/* Fail */
	    } else {			/* Braces do match */
		int q = 0, done = 0;
		len = *p ? strlen(p+1) : 0; /* Length of rest of pattern */
		if (len)
		  bronly = 0;
		if (bronly && (matchdepth != 1))
		  bronly = 0;
		n = p - pattern;	    /* Size of list in braces */
		if ((buf = (char *)malloc(n+1))) { /* Copy so we can poke it */
		    char * tp = NULL;
		    int k, sofar;
		    ckstrncpy(buf,pattern,n+1);
		    sofar = string - ostring - matchpos + 1;
		    if (sofar < 0) sofar = 0;
		    debug(F111,"CKMATCH .. string",string,sofar);
		    debug(F111,"CKMATCH .. ostring",ostring,sofar);
		    n = 0;
		    for (s = s2 = buf; 1; s++) { /* Loop through segments */
			n++;
			if (q) {	/* This char is quoted */
			    q = 0;
			    if (!*s)
			      done = 1;
			    continue;
			}
			if (*s == CMDQ && !q) {	/* Quote next char */
			    q = 1;
			    continue;
			}
			if (!*s || *s == ',') {	/* End of this segment */
			    int tplen = 0;
			    if (!*s)	/* If end of buffer */
			      done = 1;	/* then end of last segment */
			    *s = NUL;	/* Overwrite comma with NUL */
			    debug(F111,"CKMATCH {} segment",s2,done);
			    tplen = n + len + sofar + 2;
			    if (!*s2) {	/* Empty segment, no advancement */
				k = 0;
			    } else if ((tp = (char *)malloc(tplen))) {
				int savpos, opts2;
				char * pp;
				pp = matchpos > 0 ?
				    &ostring[matchpos-1] :
				    ostring;
				if (bronly) {
				    if (matchpos > 0)
				      ckstrncpy(tp,pp,sofar+1);
				    else
				      ckstrncpy(tp,pp,sofar);
				} else {
				    tp[0] = '*';
				    tp[1] = NUL;
				    if (matchpos > 0)
				      ckstrncpy(&tp[1],pp,sofar+1);
				    else
				      ckstrncpy(&tp[1],pp,sofar);
				}
				ckstrncat(tp,s2,tplen); /* Current segment */
				ckstrncat(tp,p+1,tplen); /* rest of pattern */

				debug(F101,"CKMATCH {} matchpos","",matchpos);
				savpos = matchpos;
				matchpos = 0;
#ifdef DEBUG
				if (deblog) {
				    debug(F111,"CKMATCH {} tp",tp,matchpos);
				    debug(F111,"CKMATCH {} string",
					  string,matchpos);
				    debug(F111,"CKMATCH {} ostring",
					  ostring,savpos);
				}
#endif /* DEBUG */
				/* If segment starts with dot */
				/* then set matchdot option.. */
				opts2 = opts;
				if (*s2 == '.') opts2 |= 1;
				debug(F111,"CKMATCH {} recursing",s2,opts2);
				k = ckmatch(tp,
					    (string > ostring) ?
					    &ostring[savpos-1] : string,
					    icase,opts2);
#ifdef DEBUG
				if (deblog) {
				    debug(F101,"CKMATCH {} k","",k);
				    debug(F101,"CKMATCH {} savpos","",savpos);
				}
#endif /* DEBUG */
				free(tp);
				tp = NULL;
				if (xxflag) MATCHRETURN(0,0);
				if (k == 0) {
				    matchpos = savpos;
				}
				if (k > 0) { /* If it matched we're done */
				    MATCHRETURN(7,k);
				}
			    } else {	/* Malloc failure */
				MATCHRETURN(14,0);
			    }
			    if (k) {	/* Successful comparison */
				if (!matchpos) {
				    matchpos = stringpos;
				    debug(F111,"CKMATCH {} match",
					  string, matchpos);
				}
				string += n-1; /* Advance pointers */
				pattern = p+1;
				break;
			    }
			    if (done)	/* If no more segments */
			      break;	/* break out of segment loop. */
			    s2 = s+1;	/* Otherwise, on to next segment */
			    n = 0;
			}
		    }
		    free(buf);
		}
	    }
#endif /* CKREGEX */
	} else if (cp == '*') {		/* Pattern char is asterisk */
	    char * psave;
	    char * p, * s = NULL;	/* meaning match anything */
	    int k, n, q = 0;
	    havestar++;			/* The rest can float */
	    while (*pattern == '*')	/* Collapse successive asterisks */
	      pattern++;
	    psave = pattern;		/* First non-asterisk after asterisk */
	    lastpat = pattern - 1;	/* Ditto, global */
	    debug(F111,"CKMATCH * ",string,matchpos);
	    for (n = 0, p = psave; *p; p++,n++) { /* Find next meta char */
		if (!q) {
		    if (*p == '?' || *p == '*' || *p == CMDQ
#ifdef CKREGEX
			|| *p == '[' || *p == '{'
#endif /* CKREGEX */
			)
		      break;
#ifdef GLOBBING
		    if (globbing
#ifdef UNIXOROSK
			&& *p == '/'
#else
#ifdef VMS
			&& (*p == '.' || *p == ']' ||
			    *p == '<' || *p == '>' ||
			    *p == ':' || *p == ';')
#else
#ifdef datageneral
			&& *p == ':'
#else
#ifdef STRATUS
			&& *p == '>'
#endif /* STRATUS */
#endif /* datageneral */
#endif /* VMS */
#endif /* UNIXOROSK */
			)
		      break;
#endif /* GLOBBING */
		}
	    }
	    debug(F111,"CKMATCH * n string",string,n);
	    debug(F111,"CKMATCH * n pattrn",pattern,n);
	    debug(F111,"CKMATCH * n p",p,n);
	    if (n > 0) {		/* Literal string to match  */
		s = (char *)malloc(n+1);
		if (s) {
		    ckstrncpy(s,psave,n+1); /* Copy cuz no poking original */
		    if (*p) {
			k = ckindex(s,string,0,0,icase); /* 1-based index() */
			debug(F110,"CKMATCH * Index() string",string,0);
			debug(F110,"CKMATCH * Index() pattrn",s,0);
			debug(F101,"CKMATCH * Index() result","",k);
		    } else {		/* String is right-anchored */
			k = ckindex(s,string,-1,1,icase); /* rindex() */
			debug(F111,"CKMATCH * Rindex()",string,k);
			debug(F110,"CKMATCH * Rindex() pattrn",s,0);
			debug(F101,"CKMATCH * Rindex() result","",k);
		    }
		    free(s);
		    if (k < 1) {
			MATCHRETURN(8,0);
		    }
		    debug(F111,"CKMATCH * stringpos matchpos",
			  ckitoa(stringpos), matchpos);
		    if (!matchpos) {
			matchpos = string - ostring + k;
			debug(F111,"CKMATCH * new match ", string, matchpos);
		    }
		    string += k + n - 1;
		    stringpos += k + n - 1;
		    pattern += n;
		    debug(F111,"CKMATCH * new string", string, stringpos);
		    debug(F110,"CKMATCH * new pattrn", pattern, 0);
		    continue;
		}
	    } else if (!*p) {		/* Asterisk at end matches the rest */
		if (!globbing) {	/* (if not filename globbing) */
		    if (!matchpos) {
			matchpos = stringpos;
			debug(F111,"CKMATCH *$ ",string, matchpos);
		    }
		    matchend = stringpos;
		    MATCHRETURN(9,matchpos);
		}
#ifdef GLOBBING
		while (*string) {
		    if (globbing	/* Filespec so don't cross fields */
#ifdef OS2
			&& *string == '/' || *string == '\\' ||
			*string == ':'
#else
#ifdef UNIXOROSK
			&& *string == '/'
#else
#ifdef VMS
			&& (*string == '.' || *string == ']' ||
			    *string == '<' || *string == '>' ||
			    *string == ':' || *string == ';')
#else
#ifdef datageneral
			&& *string == ':'
#else
#ifdef STRATUS
			&& *string == '>'
#else
			&& *string == '/' /* (catch-all) */
#endif /* STRATUS */
#endif /* datageneral */
#endif /* VMS */
#endif /* UNIXOROSK */
#endif /* OS2 */
			) {
			matchend = stringpos;
			MATCHRETURN(10,0);
		    }
		    if (!matchpos) {
			matchpos = stringpos;
			debug(F111,"CKMATCH *$ match",string, matchpos);
		    }
		    string++;
		    stringpos++;
		}
#endif /* GLOBBING */
		if (!matchpos) {
		    matchpos = stringpos;
		    debug(F111,"CKMATCH ** match",string, matchpos);
		}
		matchend = stringpos;
		MATCHRETURN(11,matchpos);

	    } else {			/* A meta char follows asterisk */
		if (!*string)
		  MATCHRETURN(17, matchpos = 0);
#ifdef COMMENT
		/* This is more elegant but it doesn't work. */
		p--;
		string++;
		stringpos++;
		k = ckmatch(p,string,icase,opts);
#else
		while (*string && ((k = ckmatch(p,string,icase,opts)) < 1)) {
		    if (xxflag) MATCHRETURN(0,0);
		    string++;
		    stringpos++;
		}
		if (!*string && k < 1) {
/*
  Definitely no match so we set a global flag to inibit further backing up
  and retrying by previous incarnations, since they don't see that the string
  and/or pattern, which are on the stack, have been exhausted at this level.
*/
		    xxflag++;
		    debug(F111,"CKMATCH DEFINITELY NO MATCH",p,k);
		    MATCHRETURN(91,0);
		}
#endif	/* COMMENT */
		debug(F111,"CKMATCH *<meta> k",string, k);
		if (!matchpos && k > 0) {
		    matchpos = stringpos;
		    debug(F111,"CKMATCH *<meta> matchpos",string, matchpos);
		}
		MATCHRETURN(12, (*string) ? matchpos : 0);
	    }
	} else if (cs == cp) {
	    pattern++;
	    string++;
	    stringpos++;
	    if (!matchpos) {
		matchpos = stringpos;
		debug(F111,"CKMATCH cs=cp",string, matchpos);
	    }
	    continue;
	} else {
	    MATCHRETURN(13,0);
	}
    }
  xckmatch:
    {
#ifdef DEBUG
	char msgbuf[256];
#endif /* DEBUG */
	if (matchdepth > 0)
	  matchdepth--;
	matchpos = rc;
#ifdef DEBUG
	ckmakxmsg(msgbuf,256,
		  "CKMATCH RETURN[",
		  ckitoa(where),
		  "] matchpos=",
		  ckitoa(matchpos),
		  " matchdepth=",
		  ckitoa(matchdepth),
		  " pat=",pattern,
		  " string=",string,NULL,NULL
		  );
	debug(F110,msgbuf,string,0);
#endif /* DEBUG */
	return(rc);
    }
}


#ifdef CKFLOAT
/*  I S F L O A T  -- Verify that arg represents a floating-point number */

/*
  Portable replacement for atof(), strtod(), scanf(), etc.

  Call with:
    s = pointer to string
    flag == 0 means entire string must be a (floating-pointing) number.
    flag != 0 means to terminate scan on first character that is not legal.

  Returns:
    1 if result is a legal number;
    2 if result has a fractional part;
    0 if not or if string empty.

  Side effect:
    Sets global floatval to floating-point value if successful.

  Number need not contain a decimal point -- integer is subcase of float.
  Scientific notation not supported.
*/
CKFLOAT floatval = 0.0;			/* For returning value */

int
isfloat(s,flag) char *s; int flag; {
    int state = 0;
    int sign = 0;
    char c;
    CKFLOAT d = 0.0, f = 0.0;

    if (!s) return(0);
    if (!*s) return(0);

    while (isspace(*s)) s++;

    if (*s == '-') {			/* Handle optional sign */
	sign = 1;
	s++;
    } else if (*s == '+')
      s++;
    while ((c = *s++)) {		/* Handle numeric part */
	switch (state) {
	  case 0:			/* Mantissa... */
	    if (isdigit(c)) {
		f = f * 10.0 + (CKFLOAT)(c - '0');
		continue;
	    } else if (c == '.') {
		state = 1;
		d = 1.0;
		continue;
	    }
	    if (flag)			/* Not digit or period */
	      goto done;		/* break if flag != 0 */
	    return(0);			/* otherwise fail. */
	  case 1:			/* Fraction... */
	    if (isdigit(c)) {
		d *= 10.0;
		f += (CKFLOAT)(c - '0') / d;
		continue;
	    }
	  default:
	    if (flag)			/* Illegal character */
	      goto done;		/* Break */
	    return(0);			/* or fail, depending on flag */
	}
    }
  done:
    if (sign) f = 0.0 - f;		/* Apply sign to result */
    floatval = f;			/* Set result */
    return(d ? 2 : 1);			/* Succeed */
}

/*
  c k r o u n d  --  Rounds a floating point number or an integer.

  fpnum:
    Floating-point number to round.
  places:
    Positive...To how many decimal places.
    Zero.......Round to integer.
    Negative...-1 = nearest ten, -2 = nearest 100, -3 = nearest thousand, etc.
  obuf
    Output buffer for string result if desired.
  obuflen    
    Length of output buffer.
  Returns:
    Result as CKFLOAT (which is not going to be as exact as the string result)
    And the exact result in the string output buffer, if one was specified.
*/
CKFLOAT
#ifdef CK_ANSIC
ckround(CKFLOAT fpnum, int places, char *obuf, int obuflen)
#else
ckround(fpnum,places,obuf,obuflen)
 CKFLOAT fpnum; int places, obuflen; char *obuf;
#endif /* CK_ANSIC */
/* ckround  */ {
    char *s, *s2, *d;
    int i, p, len, x, n, digits;
    int carry = 0;
    int minus = 0;
    char buf[200];
    char * number;
    CKFLOAT value;
    extern int fp_digits;

    sprintf(buf,"%200.100f",fpnum);	/* Make string version to work with */
    number = (char *) buf;		/* Make pointer to it */

    p = places;				/* Precision */
    d = (char *)0;			/* Pointer to decimal or string end */

    s = number;				/* Fix number... */
    while (*s == ' ' || *s == '\011') s++; /* Strip leading whitespace */
    if (*s == '+') s++;			   /* Skip leading plus sign*/
    number = s;				   /* Start of number */
    if (*s == '-') { minus++; number++; s++; } /* Remember if negative */

    s = number;				/* Don't allow false precision */
    n = 0;
    while (*s && *s != '.') s++, n++;   /* Find decimal */

    if (p + n > fp_digits)		/* Too many digits */
      p = fp_digits - n;		/* Don't ask for bogus precision */
    if (p < 0) p = 0;			/* But don't ask for less than zero */
    if (n > fp_digits)			/* Integer part has too many digits */
      *s = 0;				/* but we can't truncate it */
    else				/* Magnitude is OK */
      number[fp_digits+1] = 0;		/* Truncate fractional part. */

    len = (int)strlen(number);		/* Length of non-bogus number */
    d = s;				/* Pointer to decimal point */
    if (p > 0) {			/* Rounding the fractional part */
	if (n + p < len) {		/* If it's not already shorter */
	    if (*s == '.') s++;		/* Skip past decimal */
	    s += p;			/* Go to desired spot */
	    if (*s > '4' && *s <= '9')	/* Check value of digit */
	      carry = 1;
	    *s = 0;			/* And end the string */ 
	    s--;			/* Point to last digit */
	}
    } else if (p == 0) {		/* Rounding to integer */
	if (*s == '.') {
	    *s = 0;			/* erase the decimal point */
	    if (*(s+1)) {		/* and there is a factional part */
		if (*(s+1) > '4' && *(s+1) <= '9') /* Check for carry */
		  carry = 1;
	    }
	    s--;			/* Point to last digit */
	}
    } else {				/* Rounding the integer part */
	if (s + p > number) {		/* as in "the nearest hundred" */
	    s += p;			/* Go left to desired digit */
	    *d = 0;			/* Discard fraction */
	    carry = 0;
	    if (*s > '4')		/* Check first digit of fraction */
	      carry = 1;		/* and set carry flag */
	    s2 = s;
	    while (s2 < d)		/* Fill in the rest with zeros */
	      *s2++ = '0';
	    s--;			/* Point to last digit */
	}
    }
    if (carry) {			/* Handle carry, if any */
        while (s >= number) {
            if (*s == '.') {		/* Skip backwards over decimal */
                s--;
                continue;
            }
            *s += 1;			/* Add 1 to current digit */
            carry = 0;
            if (*s <= '9') 		/* If result is 9 or less */
	      break;			/* we're done */
            *s = '0';			/* Otherwise put 0 */
            carry = 1;			/* carry the 1 */
            s--;			/* and back up to next digit */
	}
    }
#ifdef __alpha
    sscanf(number,"%f",&value);		/* Convert back to floating point */
#else
    sscanf(number,"%lf",&value);        /* Convert back to floating point */
#endif
    if (obuf) strncpy(obuf,number,obuflen); /* Set string result */
    return(value);			    /* Return floating-point result */
}

#endif /* CKFLOAT */

/* Sorting routines... */

/* S H _ S O R T  --  Shell sort -- sorts string array s in place. */

/*
  Highly defensive and relatively quick.
  Uses shell sort algorithm.

  Args:
   s = pointer to array of strings.
   p = pointer to a second array to sort in parallel s, or NULL for none.
   n = number of elements in s.
   k = position of key.
   r = ascending lexical order if zero, reverse lexical order if nonzero.
   c = 0 for case independence, 1 for case matters, 2 for numeric.

  If k is past the right end of a string, the string is considered empty
  for comparison purposes.

  Hint:
   To sort a piece of an array, call with s pointing to the first element
   and n the number of elements to sort.

  Return value:
   None.  Always succeeds, unless any of s[0]..s[n-1] are bad pointers,
   in which case memory violations are possible, but C offers no defense
   against this, so no way to gracefully return an error code.
*/
VOID
sh_sort(s,p,n,k,r,c) char **s, **p; int n, k, r, c; {
    int m, i, j, x;
    char *t, *t1, *t2, *u = NULL;
#ifdef CKFLOAT
    CKFLOAT f1, f2;
#else
    long n1, n2;
#endif /* CKFLOAT */

    if (!s) return;			/* Nothing to sort? */
    if (n < 2) return;			/* Not enough elements to sort? */
    if (k < 0) k = 0;			/* Key */

    m = n;				/* Initial group size is whole array */
    while (1) {
	m = m / 2;			/* Divide group size in half */
	if (m < 1)			/* Small as can be, so done */
	  break;
	for (j = 0; j < n-m; j++) {	/* Sort each group */
	    t = t2 = s[j+m];		/* Compare this one... */
	    if (!t)			/* But if it's NULL */
	      t2 = "";			/* make it the empty string */
	    if (p)			/* Handle parallel array, if any */
	      u = p[j+m];
	    if (k > 0 && *t2) {
		if ((int)strlen(t2) < k) /* If key too big */
		  t2 = "";		/* make key the empty string */
		else			/* Key is in string */
		  t2 = t + k;		/* so point to key position */
	    }
	    for (i = j; i >= 0; i -= m) { /* Loop thru comparands s[i..]*/
		t1 = s[i];
		if (!t1)		/* Same deal */
		  t1 = "";
		if (k > 0 && *t1) {
		    if ((int)strlen(t1) < k)
		      t1 = "";
		    else
		      t1 = s[i]+k;
		}
		if (c == 2) {		/* Numeric comparison */
		    x = 0;
#ifdef CKFLOAT
		    f2 = 0.0;
		    f1 = 0.0;
		    if (isfloat(t1,1)) {
			f1 = floatval;
			if (isfloat(t2,1))
			  f2 = floatval;
			else
			  f1 = 0.0;
		    }
		    if (f2 < f1)
		      x = 1;
		    else
		      x = -1;
#else
		    n2 = 0L;
		    n1 = 0L;
		    if (rdigits(t1)) {
			n1 = atol(t1);
			if (rdigits(t2))
			  n2 = atol(t2);
			else
			  n1 = 0L;
		    }
		    if (n2 < n1)
		      x = 1;
		    else
		      x = -1;
#endif /* CKFLOAT */
		} else {
		    x = ckstrcmp(t1,t2,-1,c); /* Compare */
		}
		if (r == 0 && x < 0)
		  break;
		if (r != 0 && x > 0)
		  break;
		s[i+m] = s[i];
		if (p) p[i+m] = p[i];
	    }
	    s[i+m] = t;
	    if (p) p[i+m] = u;
	}
    }
}


/* C K R A D I X  --  Radix converter */
/*
   Call with:
     s:   a number in string format.
     in:  int, specifying the radix of s, 2-36.
     out: int, specifying the radix to convert to, 2-36.
   Returns:
     NULL on error (illegal radix, illegal number, etc.).
     "-1" on overflow (number too big for unsigned long).
     Otherwise: Pointer to result.
*/
char *
ckradix(s,in,out) char * s; int in, out; {
    char c, *r = rxresult;
    int d, minus = 0;
#ifdef COMMENT
    unsigned long zz = 0L;
    long z = 0L;
#else
    /*
      To get 64 bits on 32-bit hardware we use off_t, but there
      is no unsigned version of off_t, so we lose the ability to
      detect overflow.
    */
    CK_OFF_T zz = (CK_OFF_T)0;
    CK_OFF_T z = (CK_OFF_T)0;
#endif	/* COMMENT */

    if (in < 2 || in > 36)		/* Verify legal input radix */
      return(NULL);
    if (out < 2 || out > 36)		/* and output radix. */
      return(NULL);
    if (*s == '+') {			/* Get sign if any */
	s++;
    } else if (*s == '-') {
	minus++;
	s++;
    }
    while (*s == SP || *s == '0')	/* Trim leading blanks or 0's */
      s++;
/*
  For detecting overflow, we use a signed copy of the unsigned long
  accumulator.  If it goes negative, we know we'll overflow NEXT time
  through the loop.
*/
    for (; *s;  s++) {			/* Convert from input radix to */
	c = *s;				/* unsigned long */
	if (islower(c)) c = toupper(c);
	if (c >= '0' && c <= '9')
	  d = c - '0';
	else if (c >= 'A' && c <= 'Z')
	  d = c - 'A' + 10;
	else
	  return(NULL);
	if (d >= in)			/* Check for illegal digit */
	  return(NULL);
	zz = zz * in + d;
	if (z < 0L)			/* Clever(?) overflow detector */
	  return("-1");
        z = zz;
    }
    if (!zz) return("0");
    r = &rxresult[RXRESULT];		/* Convert from unsigned long */
    *r-- = NUL;				/* to output radix. */
    while (zz > 0 && r > rxresult) {
	d = zz % (unsigned)out;
	*r-- = rxdigits[d];
	zz = zz / (unsigned)out;
    }
    if (minus) *r-- = '-';		/* Replace original sign */
    return((char *)(r+1));
}

#ifndef NOB64
/* Base-64 conversion routines */

static char b64[] = {			/* Encoding vector */
#ifdef pdp11
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/="
#else
  'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S',
  'T','U','V','W','X','Y','Z','a','b','c','d','e','f','g','h','i','j','k','l',
  'm','n','o','p','q','r','s','t','u','v','w','x','y','z','0','1','2','3','4',
  '5','6','7','8','9','+','/','=','\0'
#endif /* pdp11 */
};
static int b64tbl[] = {			/* Decoding vector */
    -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
    -2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -2, -1, -1,
    -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

/*
   B 8 T O B 6 4  --  Converts 8-bit data to Base64 encoding.

   Call with:
     s   = Pointer to 8-bit data;
     n   = Number of source bytes to encode (SEE NOTE).
           If it's a null-terminated string, you can use -1 here.
     out = Address of output buffer.
     len = Length of output buffer (should > 4/3 longer than input).

   Returns:
     >= 0 if OK, number of bytes placed in output buffer,
          with the subsequent byte set to NUL if space permits.
     -1 on error (output buffer space exhausted).

   NOTE:
     If this function is to be called repeatedly, e.g. to encode a data
     stream a chunk at a time, the source length must be a multiple of 3
     in all calls but the final one to avoid the generation of extraneous
     pad characters that would throw the decoder out of sync.  When encoding
     only a single string, this is not a consideration.  No internal state
     is kept, so there is no reset function.
*/
int
b8tob64(s,n,out,len) char * s,* out; int n, len; {
    int b3, b4, i, x = 0;
    unsigned int t;

    if (n < 0) n = strlen(s);

    for (i = 0; i < n; i += 3,x += 4) { /* Loop through source bytes */
	b3 = b4 = 0;
	t = (unsigned)((unsigned)((unsigned int)s[i] & 0xff) << 8);
	if (n - 1 > i) {		/* Do we have another after this? */
            t |= (unsigned)(s[i+1] & 0xff); /* Yes, OR it in */
            b3 = 1;			/* And remember */
        }
        t <<= 8;			/* Move over */
        if (n - 2 > i) {		/* Another one after that? */
            t |= (unsigned)(s[i+2] & 0xff); /* Yes, OR it in */
            b4 = 1;			/* and remember */
        }
	if (x + 4 > len)		/* Check output space */
	  return(-1);
	out[x+3] = b64[b4 ? (t & 0x3f) : 64]; /* 64 = code for '=' */
        t >>= 6;
        out[x+2] = b64[b3 ? (t & 0x3f) : 64];
        t >>= 6;
        out[x+1] = b64[t & 0x3f];
        t >>= 6;
        out[x]   = b64[t & 0x3f];
    }
    if (x < len) out[x] = NUL;		/* Null-terminate the string */
    return(x);
}


/*
   B 6 4 T O B 8  --  Converts Base64 string to 8-bit data.

   Call with:
     s   = pointer to Base64 string (whitespace ignored).
     n   = length of string, or -1 if null terminated, or 0 to reset.
     out = address of output buffer.
     len = length of output buffer.

   Returns:
     >= 0 if OK, number of bytes placed in output buffer,
          with the subsequent byte set to NUL if space permits.
     <  0 on error:
       -1 = output buffer too small for input.
       -2 = input contains illegal characters.
       -3 = internal coding error.

   NOTE:
     Can be called repeatedly to decode a Base64 stream, one chunk at a
     time.  However, if it is to be called for multiple streams in
     succession, its internal state must be reset at the beginning of
     the new stream.
*/
int
b64tob8(s,n,out,len) char * s,* out; int n, len; { /* Decode */
    static int bits = 0;
    static unsigned int r = 0;
    int i, k = 0, x, t;
    unsigned char c;

    if (n == 0) {			/* Reset state */
	bits = 0;
	r = 0;
	return(0);
    }
    x = (n < 0) ? strlen(s) : n;	/* Source length */

    n = ((x + 3) / 4) * 3;		/* Compute destination length */
    if (x > 0 && s[x-1] == '=') n--;	/* Account for padding */
    if (x > 1 && s[x-2] == '=') n--;
    if (n > len)			/* Destination not big enough */
      return(-1);			/* Fail */

    for (i = 0; i < x; i++) {		/* Loop thru source */
	c = (CHAR)s[i];			/* Next char */
        t = b64tbl[c];			/* Code for this char */
	if (t == -2) {			/* Whitespace or Ctrl */
	    n--;			/* Ignore */
	    continue;
	} else if (t == -1) {		/* Illegal code */
	    return(-2);			/* Fail. */
	} else if (t > 63 || t < 0)	/* Illegal value */
	  return(-3);			/* fail. */
	bits += 6;			/* Count bits */
	r <<= 6;			/* Make space */
	r |= (unsigned) t;		/* OR in new code */
	if (bits >= 8) {		/* Have a byte yet? */
	    bits -= 8;			/* Output it */
	    c = (unsigned) ((r >> bits) & 0xff);
	    out[k++] = c;
	}
    }
    if (k < len) out[k] = NUL;		/* Null-terminate in case it's */
    return(k);				/* a text string */
}
#endif /* NOB64 */

/* C H K N U M  --  See if argument string is an integer  */

/* Returns 1 if OK, zero if not OK */
/* If OK, string should be acceptable to atoi() or atol() or ckatofs() */
/* Allows leading space, sign */

int
chknum(s) char *s; {			/* Check Numeric String */
    int x = 0;				/* Flag for past leading space */
    int y = 0;				/* Flag for digit seen */
    char c;
    debug(F110,"chknum",s,0);
    if (!s) return(0);
    if (!*s) return(0);
    while ((c = *s++)) {		/* For each character in the string */
	switch (c) {
	  case SP:			/* Allow leading spaces */
	  case HT:
	    if (x == 0) continue;
	    else return(0);
	  case '+':			/* Allow leading sign */
	  case '-':
	    if (x == 0) x = 1;
	    else return(0);
	    break;
	  default:			/* After that, only decimal digits */
	    if (c >= '0' && c <= '9') {
		x = y = 1;
		continue;
	    } else return(0);
	}
    }
    return(y);
}


/*  R D I G I T S  -- Verify that all characters in arg ARE DIGITS  */

/*  Returns 1 if so, 0 if not or if string is empty */

int
rdigits(s) char *s; {
    if (!s) return(0);
    do {
        if (!isdigit(*s)) return(0);
        s++;
    } while (*s);
    return(1);
}

/*  P A R N A M  --  Return parity name */

char *
#ifdef CK_ANSIC
parnam(char c)
#else
parnam(c) char c;
#endif /* CK_ANSIC */
/* parnam */ {
    switch (c) {
	case 'e': return("even");
	case 'o': return("odd");
	case 'm': return("mark");
	case 's': return("space");
	case 0:   return("none");
	default:  return("invalid");
    }
}

char *					/* Convert seconds to hh:mm:ss */
#ifdef CK_ANSIC
hhmmss(long x)
#else
hhmmss(x) long x;
#endif /* CK_ANSIC */
/* hhmmss(x) */ {
    static char buf[10];
    long s, h, m;
    h = x / 3600L;			/* Hours */
    x = x % 3600L;
    m = x / 60L;			/* Minutes */
    s = x % 60L;			/* Seconds */
    if (x > -1L)
      sprintf(buf,"%02ld:%02ld:%02ld",h,m,s);
    else
      buf[0] = NUL;
    return((char *)buf);
}

/* L S E T  --  Set s into p, right padding to length n with char c; */
/*
   s is a NUL-terminated string.
   If length(s) > n, only n bytes are moved.
   The result is NOT NUL terminated unless c == NUL and length(s) < n.
   The intended of this routine is for filling in fixed-length record fields.
*/
VOID
lset(p,s,n,c) char *s; char *p; int n; int c; {
    int x;
#ifndef USE_MEMCPY
    int i;
#endif /* USE_MEMCPY */
    if (!s) s = "";
    x = strlen(s);
    if (x > n) x = n;
#ifdef USE_MEMCPY
    memcpy(p,s,x);
    if (n > x)
      memset(p+x,c,n-x);
#else
    for (i = 0; i < x; i++)
      *p++ = *s++;
    for (; i < n; i++)
      *p++ = c;
#endif /* USE_MEMCPY */
}

/* R S E T  --  Right-adjust s in p, left padding to length n with char c */

VOID
rset(p,s,n,c) char *s; char *p; int n; int c; {
    int x;
#ifndef USE_MEMCPY
    int i;
#endif /* USE_MEMCPY */
    if (!s) s = "";
    x = strlen(s);
    if (x > n) x = n;
#ifdef USE_MEMCPY
    memset(p,c,n-x);
    memcpy(p+n-x,s,x);
#else
    for (i = 0; i < (n - x); i++)
      *p++ = c;
    for (; i < n; i++)
      *p++ = *s++;
#endif /* USE_MEMCPY */
}

/*  U L O N G T O H E X  --  Unsigned long to hex  */

/*
  Converts unsigned long arg to hex and returns string pointer to
  rightmost n hex digits left padded with 0's.  Allows for longs
  up to 64 bits.  Returns pointer to result.
*/
char *
#ifdef CK_ANSIC
ulongtohex( unsigned long z, int n )
#else
ulongtohex(z,n) unsigned long z; int n; 
#endif	/* CK_ANSIC */
/* ulongtohex */ {
    static char hexbuf[17];
    int i = 16, x, k = 0;
    hexbuf[16] = '\0';
    if (n > 16) n = 16;
    k = 2 * (sizeof(long));
    for (i = 0; i < n; i++) {
	if (i > k || z == 0) {
	    hexbuf[15-i] = '0';
	} else {
	    x = z & 0x0f;
	    z = z >> 4;
	    hexbuf[15-i] = x + ((x < 10) ? '0' : 0x37);
	}
    }
    return((char *)(&hexbuf[16-i]));
}

/*  H E X T O U L O N G  --  Hex string to unsigned long  */

/*
  Converts n chars from s from hex to unsigned long.
  Returns:
   0L or positive, good result (0L is returned if arg is NULL or empty).
  -1L on error: non-hex arg or overflow.
*/
long
hextoulong(s,n) char *s; int n; {
    char buf[64];
    unsigned long result = 0L;
    int d, count = 0;
    int flag = 0;
    if (!s) s = "";
    if (!*s) {
	return(0L);
    }
    if (n < 1)
      return(0L);
    if (n > 63) n = 63;
    strncpy(buf,s,n);
    buf[n] = '\0';
    s = buf;
    while (*s) {
	d = *s++;
	if ((d == '0' || d == ' ')) {
	    if (!flag)
	      continue;
	} else {
	    flag = 1;
	}
	if (islower(d))
	  d = toupper(d);
	if (d >= '0' && d <= '9') {
	    d -= 0x30;
	} else if (d >= 'A' && d <= 'F') {
	    d -= 0x37;
	} else {
	    return(-1L);
	}
	if (++count > (sizeof(long) * 2))
	  return(-1L);
	result = (result << 4) | (d & 0x0f);
    }
    return(result);
}

/*
  c k s p l i t  --  Splits a string into words, or:
                     extracts a given word from a string.

  Allows for grouping.
  Operates on a copy of the string; does not alter the original string.
  All strings NUL-terminated.

  Call with:
    fc = function code:
         1 = split, 0 = word.
    n1 = desired word number if fc == 0.
    s1 = source string.
    s2 = break string (NULL to accept default = all non-alphanum).
    s3 = include string (NULL to accept default = all alphanum).
    n2 = grouping mask (OR desired ones together):
         1 = doublequotes,  2 = braces,    4 = apostrophes,
         8 = parens,       16 = brackets, 32 = angle brackets,
        -1 = 63 = all of these.
    n3 = group quote character, ASCII value, used and tested only for
         LISP quote, e.g. (a 'b c '(d e f)).
    n4 = 0 to collapse adjacent separators;
         nonzero not to collapse them.

  Returns:
    Pointer to struct stringarray, with size:
      -1 = memory allocation error.
      -2 = too many words in string.
       n = number of words (0 or more).

  With:
    wordarray = array of pointers to n words (n == 1 if fc == 0), 1-based.
    Each pointer is to a malloc'd string.  This array is recycled upon each
    call; if you need to keep the strings, make copies of them.  This routine
    must have control of allocation and deallocation.

  If a given character is included in the include list, it is not treated
  as a separator or as a grouping character.

  Groups may be nested only if they are formed with braces, parens, or
  brackets, but not with quotes or apostrophes since ASCII quotes have no
  intrinsic handedness.  Group-start and end characters are treated as
  separators even in the absence of other separators, so a string such as
  "a{b}c" results in three words, not one.

  Sample call to split a string into an array:
    struct stringarray * q;
    q = cksplit(1,0,s1,s2,s3,-1,0);
        q->a_size = size (>=0) or failure code (<0)
        q->a_head = pointer to array (elements 0 thru q->asize - 1).

  Sample call to extract word n from a string:
    struct stringarray * q;
    q = cksplit(0,n,s1,s2,s3,-1,0);
        q->a_size = size (1) or failure code (<0)
        q->a_head = pointer to array (element 1 is the desired word).
*/

/* States */

#define ST_BW  0			/* Between Words */
#define ST_IW  1			/* In Word */
#define ST_IG  2			/* Start Group */

/* Character Classes (bitmap) */

#define CL_SEP 1			/* Separator */
#define CL_OPN 2			/* Group Open */
#define CL_CLS 4			/* Group Close */
#define CL_DAT 8			/* Data */
#define CL_QUO 16			/* Group quote */

#ifdef BIGBUFOK
#ifndef MAXWORDS
#define MAXWORDS 4096			/* Max number of words */
#endif /* MAXWORDS */
#ifndef NESTMAX
#define NESTMAX 64			/* Maximum nesting level */
#endif /* NESTMAX */
#else
#ifndef MAXWORDS
#define MAXWORDS 128			/* Max number of words */
#endif /* MAXWORDS */
#ifndef NESTMAX
#define NESTMAX 16			/* Maximum nesting level */
#endif /* NESTMAX */
#endif /* BIGBUFOK */

/* static */ char ** wordarray = NULL;	/* Result array of word pointers */

static struct stringarray ck_sval =  {	/* Return value structure */
    NULL,				/* Pointer to array */
    0					/* Size */
};
static int * wordsize = NULL;

static VOID
setword(n,s,len) int n, len; char * s; {
    register char * p;
    register int i, k;

    if (!s) s = "";

    if (!wordarray) {			/* Allocate result array (only once) */
	if (!(wordarray = (char **)malloc((MAXWORDS+1) * sizeof(char *))))
	  return;
	if (!(wordsize = (int *)malloc((MAXWORDS+1) * sizeof(int))))
	  return;
	for (i = 0; i <= MAXWORDS; i++)	{ /* Initialize result array */
	    wordarray[i] = NULL;
	    wordsize[i] = 0;
	}
    }
    if (wordsize[n] < len /* || !wordarray[n] */ ) {
	k = (len < 16) ? 16 : len + (len / 4);
	wordarray[n] = (char *) malloc(k+1);
	wordsize[n] = (wordarray[n]) ? k : 0;
	if (wordarray[n]) {
	    p = wordarray[n];
	    while ((*p++ = *s++) && k-- > 0) ;
	}
    } else if (len > 0) {
	k = wordsize[n];		/* (In case len arg is a lie) */
	p = wordarray[n];
	while ((*p++ = *s++) && k-- > 0) {
#ifdef COMMENT
	    if ((*(s-1) == CMDQ) && *s != CMDQ) {
		k++;
		p--;
	    }
#endif /* COMMENT */
	}
    } else if (wordarray[n]) {
	wordarray[n][0] = NUL;
    }
}

static char * splitbuf = NULL;
static int nsplitbuf = 0;

/* n4 = 1 to NOT collapse adjacent separators */


struct stringarray *
cksplit(fc,n1,s1,s2,s3,n2,n3,n4) int fc,n1,n2,n3,n4; char *s1, *s2, *s3; {
    int splitting = 0;			/* What I was asked to do */
    int i, k, ko = 0, n, x, max = MAXWORDS; /* Workers */
    char * s = NULL, * ss, * p;		/* Workers */
    char * sep = "";			/* Default break set */
    char * notsep = "";			/* Default include set */
    int    grouping = 0;		/* Grouping option */
    char * gr_opn = "\"{'([<";		/* Group open brackets */
    char * gr_cls = "\"}')]>";		/* Group close brackets */
    int    gr_stk[NESTMAX];		/* Nesting bracket stack */
    int    gr_lvl = 0;			/* Nesting level */
    int    wordnum = 0;			/* Current word number */
    CHAR   c = 'A';			/* Current char (dummy start value) */
    int    class = 0;			/* Current character class */
    int    state = ST_BW;		/* Current FSA state */
    int    len = 0;			/* Length of current word */
    int    slen = 0;			/* Length of s1 */
    int    gquote = 0;			/* Quoted group */
    int    cquote = 0;			/* Quoted character */
    int    collapse = 1;		/* Collapse adjacent separators */
    int    all = 0;			/* s3 == ALL */
    int    csv = 0;			/* s3 == CSV */
    int    tsv = 0;			/* s3 == TSV */
    int    prevstate = -1;

    unsigned int hex80 = 128;
    unsigned int hexff = 255;
    CHAR notsepbuf[256];

    notsepbuf[0] = NUL;			/* Keep set for "ALL" */
    if (n4) collapse = 0;		/* Don't collapse */

    for (i = 0; i < NESTMAX; i++)	/* Initialize nesting stack */
      gr_stk[i] = 0;

    setword(1,NULL,0);
    ck_sval.a_head = wordarray;		/* Initialize return value struct */
    ck_sval.a_size = 0;

    if (!s1) s1 = "";			/* s1 = source string */
    if (!*s1) {				/* If none, nothing to do */
	return(&ck_sval);
    }
    splitting = fc;			/* Our job */
    if (splitting) {			/* If splitting n = word count */
	n = 0;				/* Initialize it */
    } else {				/* Otherwise */
#ifdef COMMENT
	if (n1 < 1) {			/* If 0 (or less) */
	    ck_sval.a_size = 0;		/* nothing to do. */
	    return(&ck_sval);
	}
#else
	if (n1 == 0) {			/* If 0 */
	    ck_sval.a_size = 0;		/* nothing to do. */
	    return(&ck_sval);
	}
#endif	/* COMMENT */
	n = n1;				/* n = desired word number. */
    }
    slen = 0;				/* Get length of s1 */
    debug(F111,"cksplit",s1,n);

    p = s1;

#ifdef COMMENT
    while (*p++) slen++;		/* Make pokeable copy of s1 */
    if (!splitbuf || slen > nsplitbuf) { /* Allocate buffer if needed */
	int xx;
	if (splitbuf)
	  free(splitbuf);
	xx = (slen < 255) ? 255 : xx + (xx / 4);
	debug(F101,"cksplit splitbuf","",xx);
	splitbuf = (char *)malloc(xx+1);
	if (!splitbuf) {		/* Memory allocation failure... */
	    ck_sval.a_size = -1;
	    return(&ck_sval);
	}
	nsplitbuf = xx;			/* Remember size of buffer */
    }
#else
/*
  The idea is to just copy the string into splitbuf (if it exists).  This
  gives us both the copy and the length so we only need to grovel through the
  string once in most cases.  Only when splitbuf doesn't exist or is too short
  do we re-malloc(), which should be very infrequent so who cares if we have
  to go through the string again in that case.
*/
    p = s1;
    s = splitbuf;
    if (splitbuf) {			/* Make pokeable copy of s1 */
	while ((*s++ = *p++) && (slen++ < nsplitbuf)) /* Try to copy */
	  ;
    }
    if (!splitbuf || slen >= nsplitbuf) { /* Need to do more... */
	int xx;
	if (splitbuf)			/* Free previous buf if any */
	  free(splitbuf);
	while (*p++) slen++;		/* Get (rest of) length */
	xx = (slen < 255) ? 255 : slen + (slen / 4); /* Size of new buffer */
	splitbuf = (char *)malloc(xx+1); /* Allocate it */
	if (!splitbuf) {		 /* Memory allocation failure... */
	    ck_sval.a_size = -1;
	    return(&ck_sval);
	}
	nsplitbuf = xx;			/* Remember (new) buffer size */
	s = splitbuf;
	p = s1;
	while ((*s++ = *p++)) ;
    }

#endif /* COMMENT */
    s = splitbuf;
    sep = s2;				/* s2 = break set */
    if (!sep) sep = "";
    notsep = s3;			/* s3 = include set */
    if (!notsep) {
	notsep = "";
    } else if ((all = !ckstrcmp(notsep,"ALL",3,1)) ||
	       (csv = !ckstrcmp(notsep,"CSV",3,1)) ||
	       (tsv = !ckstrcmp(notsep,"TSV",3,1))) {
	int i, flag; CHAR c;
	int n = 0;
	char * ss;
	if (!all && (csv || tsv)) {
	    all = 1;
	    collapse = 0;
	}
	if (csv || tsv) {
	    all = 1;
	    collapse = 0;
	}
	for (i = 1; i < 256; i++) {
	    flag = 0;
	    ss = sep;
	    while (c = *ss++ && !flag) {
		if (c == i) flag++;
	    }
	    if (!flag) notsepbuf[n++] = c;
	}
	notsepbuf[n] = NUL;
	notsep = (char *)notsepbuf;
	debug(F110,"CKMATCH NOTSEPBUF ALL",notsep,0);
    }
    if (*s && csv) {			/* For CSV skip leading whitespace */
        while (*s == SP || *s == HT)
	  s++;
	c = *s;
    }
    if (n2 == 0 && csv) n2 = 1;		/* CSV implies doublequote grouping */
    if (n2 < 0) n2 = 63;		/* n2 = grouping mask */
    grouping = n2;
    p = "";				/* Pointer to current word */
    while (c) {				/* Loop through string */
	c = *s;
	class = 0;
	if (!csv && !tsv) {		/* fdc 2010-12-30 */
	    /* In CSV and TSV splitting, backslash is not special */
	    if (!cquote && c == CMDQ) {	/* If CMDQ */
		cquote++;		/* next one is quoted */
		goto nextc;		/* go get it */
	    }

	}
	if (cquote && c == CMDQ) {	/* Quoted CMDQ is special */
	    if (state != ST_BW) {	/* because it can still separate */
		char * s2 = s-1;
		while (s2 > p) { *s2 = *(s2-1); s2--; }
		p++;
	    }
	    cquote = 0;
	}
	if (cquote) {			/* Other quoted character */
	    if (state != ST_BW) {	/* can't separate or group */
		char * s2 = s-1;
		while (s2 > p) { *s2 = *(s2-1); s2--; }
		p++;
	    }
	    class = CL_DAT;		/* so treat it as data */
	    cquote = 0;
	    x = 1;
	} else {			/* Character is not quoted */
	    if (!all && c < SP) {	/* Get its class */
		x = 0;			/* x == 0 means "is separator" */
	    } else if (*sep) {		/* Break set given */
		ss = sep;
		while (*ss && *ss != c) ss++; /* (instead of ckstrchr()) */
		x = (*ss != c);
	    } else {			/* Default break set is */
		x = ((c >= 'a' && c <= 'z') || /* all but alphanumerics */
		     (c >= '0' && c <= '9') ||
		     (c >= 'A' && c <= 'Z') ||
		     ((unsigned int)c >= hex80 && (unsigned int)c <= hexff)
		     );
	    }
	    if (x == 0 && *notsep && c) { /* Include set if given */
		ss = notsep;
		while (*ss && *ss != c) ss++; /* (instead of ckstrchr()) */
		x = (*ss == c);
	    }
	    if (c == n3 && grouping && state == ST_BW) { /* Group quote? */
		class = CL_QUO;
	    } else {
		class = x ? CL_DAT : CL_SEP; /* Class = data or separator */
	    }
	    if (grouping) {		/* Grouping? */
		int j;			/* Look for group start */
		for (k = 0; k < 6; k++) { /* Group-start char? */
		    j = 1 << k;		/* Get its mask bit value */
		    if (grouping & j) {
			if (c == gr_opn[k]) { /* Selected group opener? */
			    ko = k;
			    class |= CL_OPN;
			    if (c == '"' || c == '\'') { /* These can also */
				class |= CL_CLS;         /* be closers */
				break;
			    }
			} else if (c == gr_cls[k]) { /* Group closer? */
			    class |= CL_CLS;
			    break;
			}
		    }
		}
	    }
	}
	debug(F000,"cksplit char",s,c);
	debug(F101,"cksplit state","",state);
	debug(F101,"cksplit class","",class);

	switch (state) {		/* State switcher... */
	  case ST_BW:			/* BETWEENWORDS */
	    if (class & CL_OPN) {	/* Group opener */
		if (gr_lvl == 0 && !gquote) { /* If not in group */
		    p = s;		/* point to beginning of word */
		}
		gr_lvl++;		/* Push closer on nesting stack */
		if (gr_lvl >= NESTMAX)
		  goto xxsplit;
		gr_stk[gr_lvl] = gr_cls[ko];
		prevstate = state;
		state = ST_IG;		/* Switch to INGROUP state */
	    } else if (class & CL_DAT) { /* Data character */
		gr_lvl = 0;		/* Clear group nesting stack */
		gr_stk[gr_lvl] = 0;
		len = 0;
		p = s;			/* Point to beginning of word */
		if (gquote) {		/* Adjust for quote */
		    len++;
		    p--;
		    gquote = 0;
		}
		prevstate = state;
		state = ST_IW;		/* Switch to INWORD state */
	    } else if (class & CL_QUO) { /* Group quote */
		gquote = gr_lvl+1;	/* Remember quoted level */
		p = s - 1;
		len = 1;
	    }
	    if (collapse)
	      break;

	  case ST_IW:			/* INWORD (but not in a group) */
	    if (class & CL_SEP) {	/* Ends on any kind of separator */
		*s = NUL;		/* Terminate this word */
		if (csv) {		/* If comma-separated list */
		    char * s2 = s;	/* discard surrounding spaces */
		    while (s2 > splitbuf) { /* first backwards... */
			s2--;
			if (*s2 == SP || *s2 == HT)
			  *s2 = NUL;
			else
			  break;
		    }
		    s2 = s+1;		/* Then forwards... */
		    while (*s2 && (*s2 == SP || *s2 == HT))
		      *s2++;
		    s = s2-1;
		}
		if (!csv || prevstate != ST_IG) {
		    wordnum++;		      /* Count it */
		    if (splitting || n < 0) { /* Dispose of it appropriately */
			if (wordnum > max) {  /* Add to array if splitting */
			    ck_sval.a_size = -2;
			    return(&ck_sval);
			}
			/* This inelegant bit corrects an edge condition */
			if (csv && !*p && (!c || !*(s+1))) {
			    wordnum--;
			} else {
			    setword(wordnum,p,len);
			}
		    } else if (wordnum == n) { /* Searching for word n */
			setword(1,p,len);
			ck_sval.a_size = 1;
			return(&ck_sval);
		    }
		}
		prevstate = state;
		state = ST_BW;		/* Switch to BETWEENWORDS state */
		len = 0;
	    }
	    break;

	  case ST_IG:			/* INGROUP */
	    if (class & CL_CLS) {	/* Have group closer? */
		if (csv) {
		    if (*(s+1) == c) {
			char *s2 = s;
			while ((*s2 = *(s2+1))) s2++;
			s++;
			c = *s;
		    }
		}
		if (c == gr_stk[gr_lvl]) { /* Does it match current opener? */
		    gr_lvl--;		   /* Yes, pop stack */
		    if (gr_lvl < 0)	   /* Don't pop it too much */
		      gr_lvl = 0;
		    if (gr_lvl == 0) {	/* If at top of stack */
			if (gquote)
			  s++;
			c = *s;
			*s = NUL;	/* we have word. */
			wordnum++;	/* Count and dispose of it. */
			len--;
			if (splitting || n < 0) {
			    if (wordnum > max) {
				ck_sval.a_size = -2;
				return(&ck_sval);
			    }
			    setword(wordnum,p+1,len);
			} else if (wordnum == n) {
			    setword(1,p+1,len);
			    ck_sval.a_size = 1;
			    return(&ck_sval);
			}
			prevstate = state;
			state = ST_BW;	/* Switch to BETWEENWORDS state */
			len = 0;
		    }
		    if (gr_lvl < gquote)
		      gquote = 0;
		}
	    } else if (class & CL_OPN) { /* Have group opener */
		gr_lvl++;		 /* Push on nesting stack */
		if (gr_lvl >= NESTMAX) goto xxsplit;
		gr_stk[gr_lvl] = gr_cls[ko];
	    }
	} /* switch */

      nextc:
	s++;				/* Next char */
	if (state)
	  len++;
    } /* while (c) */

    if (gr_lvl > 0) {			/* In case of an unclosed group */
	if (splitting || n < 0) {	/* make it the last word. */
	    if (++wordnum > max) {
		ck_sval.a_size = -2;
		return(&ck_sval);
	    }
	    setword(wordnum,p+1,len);
	} else if (wordnum == n) {	/* Counting from left */
	    setword(1,p+1,len);
	    ck_sval.a_size = 1;
	    return(&ck_sval);
	} else 	if (n < 0 && (wordnum + n > -1)) { /* Counting from right */
	    char * s = wordarray[wordnum + n + 1];
	    if (!s) s = "";
	    setword(1,s,strlen(s));
	    ck_sval.a_size = 1;
	    return(&ck_sval);
        }
    }
    if (!splitting) {			/* Fword... */
	if (n < 0 && (wordnum + n > -1)) { /* Counting from right */
	    char * s = wordarray[wordnum + n + 1];
	    if (!s) s = "";
	    setword(1,s,strlen(s));
	    ck_sval.a_size = 1;
	    return(&ck_sval);
        }
	setword(1,NULL,0);		/* From left... */
	ck_sval.a_size = 0;		/* but there weren't n words */
	return(&ck_sval);
    } else {				/* Succeed otherwise */
	ck_sval.a_size = wordnum;
/* 
  Always put a null element at the end of the array.  It does no harm in
  the normal case, and it's required if we're making an argv[] array to 
  pass to execvp().  This element is not included in the count.
*/
	if (wordnum < MAXWORDS)
          setword(wordnum+1,NULL,0);
    }
#ifdef DEBUG
    if (deblog) {
	for (i = 1; i <= wordnum; i++)
	  debug(F111,"cksplit result",wordarray[i],i);
    }
#endif /* DEBUG */
    return(&ck_sval);

  xxsplit:				/* Error return */
    ck_sval.a_size = -2;
    return(&ck_sval);
}

/*
  ckhexbytetoint() expects a string of two hex characters,
  returns the int equivalent or -1 on error.
*/
int
#ifdef CK_ANSIC
ckhexbytetoint( char * s )
#else
ckhexbytetoint(s) char * s;
#endif	/* CK_ANSIC */
{
    int i, c[2];
    if (!s) return(-1);
    if ((int)strlen(s) != 2) return(-1);
    for (i = 0; i < 2; i++) {
	c[i] = *s++;
	if (!c[i]) return(-1);
	if (islower(c[i])) c[i] = toupper(c[i]);
	if (c[i] >= '0' && c[i] <= '9') {
	    c[i] -= 0x30;
	} else if (c[i] >= 'A' && c[i] <= 'F') {
	    c[i] -= 0x37;
	} else {
	    return(-1);
	}
    }
    return(c[0] * 16 + c[1]);
}


/* End of ckclib.c */

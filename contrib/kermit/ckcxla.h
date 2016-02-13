/*
  File CKCXLA.H

  System-independent character-set translation header file for C-Kermit.
*/

/*
  Author: Frank da Cruz <fdc@columbia.edu>,
  The Kermit Project - Columbia University, New York City.

  Copyright (C) 1985, 2009,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/
/*
  NOTE:
  ISO 204 is Latin-1 + Euro.
  ISO 205 is Latin-4 + Euro.
  ISO 206 is Latin-7 + Euro.
*/
#ifndef CKCXLA_H                        /* Guard against multiple inclusion */
#define CKCXLA_H

#ifndef KANJI                           /* Systems supporting Kanji */
#ifdef OS2
#define KANJI
#endif /* OS2 */
#endif /* KANJI */

#ifdef NOKANJI                          /* Except if NOKANJI is defined. */
#ifdef KANJI
#undef KANJI
#endif /* KANJI */
#endif /* NOKANJI */

#ifndef NOUNICODE
#ifndef UNICODE                         /* Unicode support */
#ifdef OS2ORUNIX                        /* Only for K95, UNIX, VMS,... */
#define UNICODE
#else
#ifdef VMS
#define UNICODE
#endif /* VMS */
#endif /* OS2ORUNIX */
#endif /* UNICODE */
#endif /* NOUNICODE */

#define XLA_NONE    0                   /* Translation types - none */
#define XLA_BYTE    1                   /* Byte-for-byte */
#define XLA_JAPAN   2                   /* Japanese */
#define XLA_UNICODE 3                   /* Unicode */

#ifndef UNIORKANJI                      /* Unicode OR Kanji */
#ifdef UNICODE                          /* i.e. some support for */
#define UNIORKANJI                      /* multibyte character sets */
#endif /* UNICODE */
#ifdef KANJI
#define UNIORKANJI
#endif /* KANJI */
#endif /* UNIORKANJI */
/*
   Disable all support for all classes of character sets
   if NOCSETS is defined.
*/
#ifdef NOCSETS

#ifdef CKOUNI
#undef CKOUNI
#endif /* CKOUNI */
#ifdef KANJI
#undef KANJI
#endif /* KANJI */
#ifdef CYRILLIC
#undef CYRILLIC
#endif /* CYRILLIC */
#ifdef LATIN2
#undef LATIN2
#endif /* LATIN2 */
#ifdef HEBREW
#undef HEBREW
#endif /* HEBREW */
#ifdef UNICODE
#undef UNICODE
#endif /* UNICODE */
#ifndef NOUNICODE
#define NOUNICODE
#endif	/* NOUNICODE */

#else /* Not NOCSETS - Rest of this file... */

#ifdef NOUNICODE                        /* Unicode */
#ifdef UNICODE
#undef UNICODE
#endif /* UNICODE */
#endif /* NOUNICODE */

#ifdef UNICODE
#ifdef OS2
#ifndef CKOUNI
#define CKOUNI                          /* Special Unicode features for K95 */
#endif /* CKOUNI */
#endif /* OS2 */
#endif /* UNICODE */

#ifndef OS2
#ifdef CKOUNI
#undef CKOUNI
#endif /* CKOUNI */
#endif /* OS2 */

#ifndef NOLATIN2                        /* If they didn't say "no Latin-2" */
#ifndef LATIN2                          /* Then if LATIN2 isn't already */
#define LATIN2                          /* defined, define it. */
#endif /* LATIN2 */
#endif /* NOLATIN2 */

#ifdef NOCYRILLIC                       /* (spelling variant...) */
#ifndef NOCYRIL
#define NOCYRIL
#endif /* NOCYRIL */
#endif /* NOCYRILLIC */

#ifndef NOCYRIL                         /* If they didn't say "no Cyrillic" */
#ifndef CYRILLIC                        /* Then if CYRILLIC isn't already */
#define CYRILLIC                        /* defined, define it. */
#endif /* CYRILLIC */
#endif /* NOCYRIL */

#ifndef NOHEBREW                        /* If they didn't say "no Hebrew" */
#ifndef HEBREW                          /* Then if HEBREW isn't already */
#define HEBREW                          /* defined, define it. */
#endif /* HEBREW */
#endif /* NOHEBREW */

#ifndef NOGREEK                         /* If not no Greek */
#ifndef GREEK                           /* then if GREEK isn't already */
#define GREEK                           /* defined, define it. */
#endif /* GREEK */
#endif /* NOGREEK */

#ifndef NOKANJI                         /* If not no Kanji */
#ifndef KANJI                           /* then if KANJI isn't already */
#define KANJI                           /* defined, define it. */
#endif /* KANJI */
#endif /* NOKANJI */

/* File ckcxla.h -- Character-set-related definitions, system independent */

/* Codes for Kermit Transfer Syntax Level (obsolete) */

#define TS_L0 0          /* Level 0 (Transparent) */
#define TS_L1 1          /* Level 1 (one standard character set) */
#define TS_L2 2          /* Level 2 (multiple character sets in same file) */

#define UNK 63           /* Symbol to use for unknown character (63 = ?) */

/*
  Codes for the base alphabet of a given character set.
  These are assigned in roughly ISO 8859 order.
  (Each is assumed to include ASCII/Roman.)
*/
#define AL_UNIV    0                    /* Universal (like ISO 10646) */
#define AL_ROMAN   1                    /* Roman (Latin) alphabet */
#define AL_CYRIL   2                    /* Cyrillic alphabet */
#define AL_ARABIC  3                    /* Arabic */
#define AL_GREEK   4                    /* Greek */
#define AL_HEBREW  5                    /* Hebrew */
#define AL_KANA    6                    /* Japanese Katakana */
#define AL_JAPAN   7                    /* Japanese Katakana+Kanji ideograms */
#define AL_HAN     8                    /* Chinese/Japanese/Korean ideograms */
#define AL_INDIA   9                    /* Indian scripts (ISCII) */
#define AL_VIET   10                    /* Vietnamese (VISCII) */
                                        /* Add more here... */
#define AL_UNK   999                    /* Unknown (transparent) */

/* Codes for languages */
/*
  NOTE: It would perhaps be better to use ISO 639-1988 2-letter "Codes for
  Representation of Names of Languages" here, shown in the comments below.
*/
#define L_ASCII       0  /* EN ASCII, English */
#define L_USASCII     0  /* EN ASCII, English */
#define L_DUTCH       1  /* NL Dutch */
#define L_FINNISH     2  /* FI Finnish */
#define L_FRENCH      3  /* FR French */
#define L_GERMAN      4  /* DE German */
#define L_HUNGARIAN   5  /* HU Hungarian */
#define L_ITALIAN     6  /* IT Italian */
#define L_NORWEGIAN   7  /* NO Norwegian */
#define L_PORTUGUESE  8  /* PT Portuguese */
#define L_SPANISH     9  /* ES Spanish */
#define L_SWEDISH    10  /* SV Swedish */
#define L_SWISS      11  /* RM Swiss (Rhaeto-Romance) */
#define L_DANISH     12  /* DA Danish */
#define L_ICELANDIC  13  /* IS Icelandic */
#define L_RUSSIAN    14  /* RU Russian */
#define L_JAPANESE   15  /* JA Japanese */
#define L_HEBREW     16  /* IW Hebrew */
#define L_GREEK      17  /*    Greek */

#define MAXLANG      17  /* Number of languages */

/*
  File character-sets are defined in the system-specific ck?xla.h file,
  except for the following ones, which must be available to all versions:
*/
#define FC_TRANSP  254                  /* Transparent */
#define FC_UNDEF   255                  /* Undefined   */
/*
  Designators for Kermit's transfer character sets.  These are all standard
  sets, or based on them.  Symbols must be unique in the first 8 characters,
  because some C preprocessors have this limit.
*/
/* LIST1 */
#define TC_TRANSP  0   /* Transparent, no character translation */
#define TC_USASCII 1   /* ISO 646 IRV / US 7-bit ASCII */
#define TC_1LATIN  2   /* ISO 8859-1, Latin Alphabet 1 */
#define TC_2LATIN  3   /* ISO 8859-2, Latin Alphabet 2 */
#define TC_CYRILL  4   /* ISO 8859-5, Latin/Cyrillic */
#define TC_JEUC    5   /* Japanese EUC = JIS 0201+0202+0208 */
#define TC_HEBREW  6   /* ISO 8859-8, Latin/Hebrew */
#define TC_GREEK   7   /* ISO 8859-7, Latin/Greek */
#define TC_9LATIN  8   /* ISO 8859-15 Latin Alphabet 9 (with Euro) */
#define TC_UCS2    9   /* ISO 10646 / Unicode UCS-2 */
#define TC_UTF8   10   /* ISO 10646 / Unicode UTF-8 */

#define MAXTCSETS 10   /* Highest Transfer Character Set Number */

#ifdef COMMENT
/*
  Not used and probably won't be due to ISO-10646 / Unicode.
*/
#define TC_3LATIN 11  /* ISO 8859-3, Latin-3 */
#define TC_4LATIN 12   /* ISO 8859-4, Latin-4 */
#define TC_5LATIN 13  /* ISO 8859-9, Latin-5 */
#define TC_ARABIC 14  /* ISO-8859-6, Latin/Arabic */
#define TC_JIS208 15  /* Japanese JIS X 0208 multibyte set */
#define TC_CHINES 16  /* Chinese Standard GB 2312-80 */
#define TC_KOREAN 17  /* Korean KS C 5601-1987 */
#define TC_ISCII  18  /* Indian standard code for ii... */
#define TC_VSCII  19  /* Vietnam standard code for ii... */
/* etc... */
#endif /* COMMENT */

/* Structure for character-set information */

struct csinfo {
    char *name;                         /* Descriptive name of character set */
    int size;                           /* Size (e.g. 128, 256, 16384) */
    int code;                           /* Like TC_1LATIN, etc.  */
    char *designator;                   /* Designator, like I2/100 = Latin-1 */
    int alphabet;                       /* Base alphabet */
    char *keyword;                      /* Keyword for this character-set */
};

/* Structure for language information */

struct langinfo {
    int id;                             /* Language ID code (L_whatever) */
    int fc;                             /* File character set to use */
    int tc;                             /* Transfer character set to use */
    char *description;                  /* Description of language */
};

/* Now take in the system-specific definitions */

#ifdef UNIX
#include "ckuxla.h"
#endif /* UNIX */

#ifdef OSK                              /* OS-9 */
#include "ckuxla.h"
#endif /* OS-9 */

#ifdef VMS                              /* VAX/VMS */
#include "ckuxla.h"
#endif /* VMS */

#ifdef GEMDOS                           /* Atari ST */
#include "ckuxla.h"
#endif /* GEMDOS */

#ifdef MAC                              /* Macintosh */
#include "ckmxla.h"
#endif /* MAC */

#ifdef OS2                              /* OS/2 */
#include "ckuxla.h"                     /* Uses big UNIX version */
#endif /* OS2 */

#ifdef AMIGA                            /* Commodore Amiga */
#include "ckuxla.h"
#endif /* AMIGA */

#ifdef datageneral                      /* Data General MV AOS/VS */
#include "ckuxla.h"
#endif /* datageneral */

#ifdef STRATUS                          /* Stratus Computer, Inc. VOS */
#include "ckuxla.h"
#endif /* STRATUS */

#ifdef UNICODE
#include "ckcuni.h"                     /* Unicode */
#endif /* UNICODE */

#ifdef KANJI
#define UNKSJIS 0x817f
_PROTOTYP(USHORT eu_to_sj, (USHORT) );  /* EUC-JP to Shift-JIS  */
_PROTOTYP(USHORT sj_to_eu, (USHORT) );  /* Shift-JIS to EUC-JP  */
_PROTOTYP( int xkanjf, (void) );
_PROTOTYP( int xkanji, (int, int (*)(char)) );
_PROTOTYP( int xkanjz, (int (*)(char) ) );
_PROTOTYP( int zkanjf, (void) );
_PROTOTYP( int zkanji, (int (*)(void)) ); /* Kanji function prototypes */
_PROTOTYP( int zkanjz, (void) );
_PROTOTYP(VOID j7init, ( void ) );      /* Initialize JIS-7 parser */
_PROTOTYP(int getj7, ( void ) );        /* Get next JIS-7 character */
#endif /* KANJI */

#ifndef MAC
#ifndef NOLOCAL
_PROTOTYP( int cs_size, (int) );
_PROTOTYP( int cs_is_std, (int) );
_PROTOTYP( int cs_is_nrc, (int) );
_PROTOTYP( VOID setremcharset, (int, int) );
_PROTOTYP( VOID setlclcharset, (int) );
#endif /* NOLOCAL */
#endif /* MAC */

_PROTOTYP(VOID setxlatype, (int, int));

#endif /* NOCSETS */
#endif /* CKCXLA_H */

/* End of ckcxla.h */

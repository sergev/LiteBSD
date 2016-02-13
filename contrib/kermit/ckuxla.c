#include "ckcsym.h"

#include "ckcdeb.h"			/* Includes... */
#include "ckcker.h"
#include "ckucmd.h"
#include "ckcxla.h"

#ifdef NOXFER
#define zdstuff(a)
#endif /* NOXFER */

#ifndef NOCSETS
char *xlav = "Character Set Translation 9.0.044, 2 Jun 2011";

/*  C K U X L A  */

/*  C-Kermit tables and functions supporting character set translation.  */
/*
  Author: Frank da Cruz <fdc@columbia.edu>,
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 2011,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/

/* Character set translation data and functions */

extern int zincnt;			/* File i/o macros and variables */
extern char *zinptr;
extern int zoutcnt;
extern char *zoutptr;
extern int byteorder;
extern int xfrxla;

int tslevel  = TS_L0;			/* Transfer syntax level (0,1,2) */
int tcharset = TC_TRANSP;		/* Transfer syntax character set */
int tcs_save = -1;			/* For save/restore term charset */
int tcs_transp = 0;			/* Term charset is TRANSPARENT flag */

#ifdef CKOUNI
int tcsr     = TX_8859_1;		/* Remote terminal character set */
#else /* CKOUNI */
int tcsr     = FC_USASCII;
#endif /* CKOUNI */
int language = L_USASCII;		/* Language */

#ifdef UNICODE
int ucsbom = 1;				/* Add BOM to new UCS files? */
int ucsorder = -1;			/* Default byte order for UCS files */
int fileorder = -1;			/* Byte order of current file */
					/* 0 = BE, 1 = LE */
#endif /* UNICODE */
/*
  Default local file and terminal character set.
  Normally ASCII, but for some systems we know otherwise.
*/
int fcs_save = -1;
#ifdef datageneral			/* Data General AOS/VS */
int fcharset = FC_DGMCS;		/* uses the DG International set */
int dcset8   = FC_DGMCS;
int dcset7   = FC_USASCII;
int tcsl     = FC_DGMCS;
#else
#ifdef NEXT				/* The NeXT workstation */
int fcharset = FC_NEXT;			/* uses its own 8-bit set */
int dcset8   = FC_NEXT;
int dcset7   = FC_USASCII;
int tcsl     = FC_NEXT;
#else
#ifdef MAC				/* The Macintosh */
int fcharset = FC_APPQD;		/* uses an extended version of */
int dcset8   = FC_APPQD;
int dcset7   = FC_USASCII;
int tcsl     = FC_APPQD;		/* Apple Quickdraw */
#else
#ifdef AUX
int fcharset = FC_APPQD;		/* Ditto for Apple A/UX */
int dcset8   = FC_APPQD;
int dcset7   = FC_USASCII;
int tcsl     = FC_APPQD;
#else
#ifdef AMIGA				/* The Commodore Amiga */
int fcharset = FC_1LATIN;		/* uses Latin-1 */
int dcset8   = FC_1LATIN;
int dcset7   = FC_USASCII;
int tcsl     = FC_1LATIN;
#else					/* All others */
#ifdef CKOUNI 				/* OS/2 Unicode */
int fcharset = FC_1LATIN;
int dcset8   = FC_1LATIN;
int dcset7   = FC_USASCII;
int tcsl     = TX_8859_1;
int prncs    = TX_CP437;
#else					/* All others */
int fcharset = FC_USASCII;		/* use ASCII by default */
int dcset8   = FC_1LATIN;		/* But default 8-bit set is Latin-1 */
int dcset7   = FC_USASCII;
int tcsl     = FC_USASCII;
int prncs    = FC_CP437;
#endif /* CKOUNI */
#endif /* AMIGA */
#endif /* AUX */
#endif /* MAC */
#endif /* NEXT */
#endif /* datageneral */

int s_cset = XMODE_A;			/* SEND charset selection = AUTO */
int r_cset = XMODE_A;			/* RECV charset selection = AUTO */
int afcset[MAXFCSETS+1];		/* Character-set associations */
int axcset[MAXTCSETS+1];
int xlatype = XLA_NONE;			/* Translation type */

#ifdef UNICODE
#ifdef CK_ANSIC
extern int (*xl_utc[MAXTCSETS+1])(USHORT);  /* Unicode to TCS */
extern int (*xl_ufc[MAXFCSETS+1])(USHORT);  /* Unicode to FCS */
extern USHORT (*xl_tcu[MAXTCSETS+1])(CHAR); /* TCS to Unicode */
extern USHORT (*xl_fcu[MAXFCSETS+1])(CHAR); /* FCS to Unicode */
#else
extern int (*xl_utc[MAXTCSETS+1])();
extern int (*xl_ufc[MAXFCSETS+1])();
extern USHORT (*xl_tcu[MAXTCSETS+1])();
extern USHORT (*xl_fcu[MAXFCSETS+1])();
#endif /* CK_ANSIC */
#endif /* UNICODE */

/* Bureaucracy section */

_PROTOTYP( CHAR xnel1, (CHAR c) );	/* NeXT to Latin-1 */
_PROTOTYP( CHAR xl143, (CHAR c) );	/* Latin-1 to IBM CP437 */
_PROTOTYP( CHAR xl1as, (CHAR c) );	/* Latin-1 to US ASCII */
_PROTOTYP( CHAR zl1as, (CHAR c) );	/* Latin-1 to US ASCII */

#ifdef CYRILLIC
_PROTOTYP( CHAR xassk, (CHAR c) );	/* ASCII to Short KOI */
_PROTOTYP( CHAR xskcy, (CHAR c) );	/* Short KOI to Latin/Cyrillic */
#endif /* CYRILLIC */

#ifdef LATIN2
_PROTOTYP( CHAR xnel2, (CHAR c) );	/* NeXT to Latin-2 */
_PROTOTYP( CHAR xl243, (CHAR c) );	/* Latin-2 to IBM CP437 */
_PROTOTYP( CHAR xl2as, (CHAR c) );	/* Latin-2 to US ASCII */
_PROTOTYP( CHAR zl2as, (CHAR c) );	/* Latin-2 to US ASCII */
_PROTOTYP( CHAR xl2r8, (CHAR c) );	/* Latin-2 to HP */
_PROTOTYP( CHAR xl2l9, (CHAR c) );	/* Latin-2 to Latin-9 */
_PROTOTYP( CHAR xl9l2, (CHAR c) );	/* Latin-9 to Latin-2 */
_PROTOTYP( CHAR xl2mz, (CHAR c) );	/* Latin-2 to Mazovia */
_PROTOTYP( CHAR xmzl2, (CHAR c) );	/* Mazovia to Latin-2 */
_PROTOTYP( CHAR xl1mz, (CHAR c) );	/* Latin-1 to Mazovia */
_PROTOTYP( CHAR xmzl1, (CHAR c) );	/* Mazovia to Latin-1 */
_PROTOTYP( CHAR xmzl9, (CHAR c) );	/* Latin-9 to Mazovia */
_PROTOTYP( CHAR xl9mz, (CHAR c) );	/* Mazovia to Latin-9 */
#endif /* LATIN2 */

/* Transfer character-set info */

struct csinfo tcsinfo[] = {
/*  Name              size code      designator alphabet keyword            */
  "TRANSPARENT",       256,TC_TRANSP, "",      AL_UNK,  "transparent",  /* 0 */
  "ASCII",             128,TC_USASCII,"",      AL_ROMAN,"ascii",        /* 1 */
  "ISO 8859-1 Latin-1",256,TC_1LATIN, "I6/100",AL_ROMAN,"latin1-iso",   /* 2 */
#ifdef LATIN2
  "ISO 8859-2 Latin-2",256,TC_2LATIN, "I6/101",AL_ROMAN,"latin2-iso",   /* 3 */
#endif /* LATIN2 */
#ifdef CYRILLIC
  /* 4 */
  "ISO 8859-5 Latin/Cyrillic",256,TC_CYRILL,"I6/144",AL_CYRIL,"cyrillic-iso",
#endif /* CYRILLIC */
#ifdef KANJI
  "Japanese EUC",16384,TC_JEUC,  "I14/87/13", AL_JAPAN, "euc-jp",       /* 5 */
#endif /* KANJI */
#ifdef HEBREW
  /* 6 */
  "ISO 8859-8 Latin/Hebrew",256,TC_HEBREW,"I6/138",AL_HEBREW,"hebrew-iso",
#endif /* HEBREW */
#ifdef GREEK
  "ISO 8859-7 Latin/Greek",256,TC_GREEK,"I6/126",AL_GREEK,"greek-iso", /*  7 */
#endif /* GREEK */
  "ISO 8859-15 Latin-9",256,TC_9LATIN,"I6/203",AL_ROMAN,"latin9-iso",  /*  8 */
  "ISO 10646 / Unicode UCS-2",64000,TC_UCS2,"I162",AL_UNIV,"ucs2",     /*  9 */
  "ISO 10646 / Unicode UTF-8",64000,TC_UTF8,"I190",AL_UNIV,"utf8",     /* 10 */
  "",0,0,"",0,""
};
int ntcsets = (sizeof(tcsinfo) / sizeof(struct csinfo)) - 1;

struct keytab tcstab[] = {		/* Keyword table for */
    "ascii",         TC_USASCII, 0,	/* SET TRANSFER CHARACTER-SET */
#ifdef CYRILLIC
    "cyrillic-iso",  TC_CYRILL,  0,
#endif /* CYRILLIC */
#ifdef GREEK
    "elot928-greek", TC_GREEK,   CM_INV,
#endif /* GREEK */
#ifdef KANJI
    "euc-jp",        TC_JEUC,    0,
#endif /* KANJI */
#ifdef GREEK
    "greek-iso",     TC_GREEK,   0,
#endif /* GREEK */
#ifdef HEBREW
    "hebrew-iso",    TC_HEBREW,  0,
#endif /* HEBREW */

#ifdef UNICODE
    "iso-10646-ucs-2", TC_UCS2,    CM_INV, /* ISO 10646 / Unicode UCS-2 */
#endif /* UNICODE */
    "iso-8859-1",    TC_1LATIN,  CM_INV, /* ISO Latin Alphabet 1 */
    "iso-8859-15",   TC_9LATIN,  CM_INV, /* ISO Latin Alphabet 9 (yes) */
    "iso-8859-2",    TC_2LATIN,  CM_INV, /* ISO Latin Alphabet 2 */
#ifdef CYRILLIC
    "iso-8859-5",    TC_CYRILL,  CM_INV, /* ISO Latin/Cyrillic Alphabet */
#endif /* CYRILLIC */
#ifdef GREEK
    "iso-8859-7",    TC_GREEK,   CM_INV, /* ISO 8859-7 Latin/Greek */
#endif /* GREEK */
#ifdef HEBREW
    "iso-8859-8",    TC_HEBREW,  CM_INV, /* ISO Latin/Hebrew */
#endif /* HEBREW */

#ifdef KANJI
    "japanese-euc",  TC_JEUC,    CM_INV,
#endif /* KANJI */
    "l",             TC_1LATIN,  CM_ABR|CM_INV,
    "la",            TC_1LATIN,  CM_ABR|CM_INV,
    "lat",           TC_1LATIN,  CM_ABR|CM_INV,
    "lati",          TC_1LATIN,  CM_ABR|CM_INV,
    "latin",         TC_1LATIN,  CM_ABR|CM_INV,
    "latin1-iso",    TC_1LATIN,  0,
#ifdef LATIN2
    "latin2-iso",    TC_2LATIN,  0,
#endif /* LATIN2 */
    "latin9-iso",    TC_9LATIN,  0,
    "transparent",   TC_TRANSP,  0,
#ifdef UNICODE
    "ucs2",          TC_UCS2,    0,
#endif /* UNICODE */
    "us-ascii",      TC_USASCII, CM_INV,
    "usascii",       TC_USASCII, CM_INV,
#ifdef UNICODE
    "utf-8",         TC_UTF8,    CM_INV,
    "utf8",          TC_UTF8,    0,
#endif /* UNICODE */
    "", 0, 0
};
int ntcs = (sizeof(tcstab) / sizeof(struct keytab)) - 1;

/* File character set information structure, indexed by character set code, */
/* as defined in ckuxla.h.  This table must be in order of file character */
/* set number! */

struct csinfo fcsinfo[] = { /* File character set information... */
  /* Descriptive Name              Size  Designator */
  "US ASCII",                     128, FC_USASCII, NULL, AL_ROMAN, "ascii",
  "British/UK ISO-646",           128, FC_UKASCII, NULL, AL_ROMAN, "british",
  "Dutch ISO-646",                128, FC_DUASCII, NULL, AL_ROMAN, "dutch",
  "Finnish ISO-646",              128, FC_FIASCII, NULL, AL_ROMAN, "finnish",
  "French ISO-646",               128, FC_FRASCII, NULL, AL_ROMAN, "french",
  "Canadian-French NRC", 128, FC_FCASCII, NULL, AL_ROMAN, "canadian-french",
  "German ISO-646",               128, FC_GEASCII, NULL, AL_ROMAN, "german",
  "Hungarian ISO-646",            128, FC_HUASCII, NULL, AL_ROMAN, "hungarian",
  "Italian ISO-646",              128, FC_ITASCII, NULL, AL_ROMAN, "italian",
  "Norwegian/Danish ISO-646",128,FC_NOASCII,NULL,AL_ROMAN,"norwegian/danish",
  "Portuguese ISO-646",           128, FC_POASCII, NULL, AL_ROMAN,"portuguese",
  "Spanish ISO-646",              128, FC_SPASCII, NULL, AL_ROMAN, "spanish",
  "Swedish ISO-646",              128, FC_SWASCII, NULL, AL_ROMAN, "swedish",
  "Swiss NRC",                    128, FC_CHASCII, NULL, AL_ROMAN, "swiss",
  "ISO 8859-1 Latin-1",           256, FC_1LATIN,  NULL, AL_ROMAN,"latin1-iso",
  "ISO 8859-2 Latin-2",           256, FC_2LATIN,  NULL, AL_ROMAN,"latin2-iso",
  "DEC Multinational", 256,  FC_DECMCS, NULL,AL_ROMAN,"dec-multinational",
  "NeXT Multinational", 256, FC_NEXT,   NULL,AL_ROMAN,"next-multinational",
  "PC Code Page 437",            256, FC_CP437,   NULL, AL_ROMAN,"cp437",
  "PC Code Page 850",            256, FC_CP850,   NULL, AL_ROMAN,"cp850",
  "PC Code Page 852",            256, FC_CP852,   NULL, AL_ROMAN,"cp852",
  "Apple Macintosh Latin", 256, FC_APPQD,   NULL, AL_ROMAN,"macintosh-latin",
  "Data General International",256,FC_DGMCS,NULL,AL_ROMAN,"dg-international",
  "Hewlett Packard Roman8",    256, FC_HPR8,    NULL, AL_ROMAN, "hp-roman8",
  "ISO 8859-5 Latin/Cyrillic", 256, FC_CYRILL,  NULL, AL_CYRIL,"cyrillic-iso",
  "CP866 Cyrillic",	       256, FC_CP866,   NULL, AL_CYRIL,"cp866",
  "Short KOI",                 128, FC_KOI7,    NULL, AL_CYRIL,"short-koi",
  "Old KOI-8 Cyrillic",        256, FC_KOI8,    NULL, AL_CYRIL,"koi8-cyrillic",
  "Japanese JIS7",	  16384, FC_JIS7,    NULL, AL_JAPAN, "jis7-kanji",
  "Japanese Shift JIS",	  16384, FC_SHJIS,   NULL, AL_JAPAN, "shift-jis-kanji",
  "Japanese EUC",	  16384, FC_JEUC,    NULL, AL_JAPAN, "euc-jp",
  "Japanese DEC Kanji",	  16384, FC_JDEC,    NULL, AL_JAPAN, "dec-kanji",
  "Hebrew-7 DEC",           128, FC_HE7,     NULL, AL_HEBREW, "hebrew-7",
  "ISO 8859-8 Latin/Hebrew",256, FC_HEBREW,  NULL, AL_HEBREW, "hebrew-iso",
  "CP862 Hebrew",           256, FC_CP862,   NULL, AL_HEBREW, "cp862-hebrew",
  "ELOT 927 Greek",         128, FC_ELOT,    NULL, AL_GREEK, "elot927-greek",
  "ISO 8859-7 Latin/Greek", 256, FC_GREEK,   NULL, AL_GREEK, "greek-iso",
  "CP869 Greek",            256, FC_CP869,   NULL, AL_GREEK, "cp869-greek",
  "ISO 8859-15 Latin-9",    256, FC_9LATIN,  NULL, AL_ROMAN, "latin9-iso",
  "PC Code Page 858",       256, FC_CP850,   NULL, AL_ROMAN, "cp858",
  "PC Code Page 855",       256, FC_CP855,   NULL, AL_CYRIL, "cp855-cyrillic",
  "Windows Code Page 1251", 256, FC_CP1251,  NULL, AL_CYRIL, "cp1251-cyrillic",
  "Bulgarian PC Code Page", 256, FC_BULGAR,  NULL, AL_CYRIL, "bulgaria-pc",
  "Windows Code Page 1250", 256, FC_CP1250,  NULL, AL_ROMAN, "cp1250",
  "Polish Mazovia PC Code Page", 256, FC_MAZOVIA, NULL, AL_ROMAN, "mazovia-pc",
  "ISO 10646 / Unicode UCS-2", 64000, FC_UCS2, NULL, AL_UNIV, "ucs2",
  "ISO 10646 / Unicode UTF-8", 64000, FC_UCS2, NULL, AL_UNIV, "utf8",
  "KOI8-R Russian+Boxdrawing",256,  FC_KOI8R, NULL, AL_CYRIL,"koi8r",
  "KOI8-U Ukrainian+Boxdrawing",256,FC_KOI8U, NULL, AL_CYRIL,"koi8u",
  "Windows Code Page 1252", 256, FC_CP1252,  NULL, AL_ROMAN, "cp1252",
  "",0,0,NULL,0,NULL
};

/* Local file character sets */
/* Includes 7-bit National Replacement Character Sets of ISO 646 */
/* Plus ISO Latin-1, DEC Multinational Character Set (MCS), NeXT char set, */
/* Various PC and Windows code pages, etc. */
/* As of C-Kermit 9.0 MIME names are included as invisible synomyms for */
/* those character sets that have MIME names. */

struct keytab fcstab[] = { /* Keyword table for 'set file character-set' */
/*
  IMPORTANT: This table is replicated below as ttcstab (terminal character
  set table).  The only differences are the addition of TRANSPARENT
  and the removal of the Kanji sets, which are not supported for terminal
  emulation.  If you make changes to this table, also change ttcstab.
*/

/* Keyword               Value       Flags */
    "apple-quickdraw",    FC_APPQD,   CM_INV, /* Apple Quickdraw */
    "ascii",              FC_USASCII, 0, /* ASCII */
    "british",            FC_UKASCII, 0, /* British NRC */
    "bulgaria-pc",        FC_BULGAR,  0, /* Bulgarian PC Code Page */
    "canadian-french",    FC_FCASCII, 0, /* French Canadian NRC */
#ifdef LATIN2
    "cp1250",             FC_CP1250,  0, /* Windows CP 1250 */
#endif /* LATIN2 */
#ifdef CYRILLIC
    "cp1251-cyrillic",    FC_CP1251,  0, /* Windows CP 1251 */
#endif /* CYRILLIC */
    "cp1252",             FC_CP1252,  0, /* Windows CP 1252 */
    "cp437",              FC_CP437,   0, /* PC CP437 */
    "cp850",              FC_CP850,   0, /* PC CP850 */
#ifdef LATIN2
    "cp852",              FC_CP852,   0, /* PC CP852 */
#endif /* LATIN2 */
#ifdef CYRILLIC
    "cp855-cyrillic",     FC_CP855,   0, /* PC CP855 */
#endif /* CYRILLIC */
    "cp858",              FC_CP858,   0, /* PC CP858 */
#ifdef HEBREW
    "cp862-hebrew",       FC_CP862,   0, /* PC CP862 */
#endif /* HEBREW */
#ifdef CYRILLIC
    "cp866-cyrillic",     FC_CP866,   0, /* CP866 Cyrillic */
#endif /* CYRILLIC */
#ifdef GREEK
    "cp869-greek",        FC_CP869,   0, /* CP869 Greek */
#endif /* GREEK */
#ifdef CYRILLIC
    "cyrillic-iso",       FC_CYRILL,  0, /* ISO Latin/Cyrillic Alphabet */
#endif /* CYRILLIC */
    "danish",             FC_NOASCII, 0, /* Norwegian and Danish NRC */
#ifdef KANJI
    "dec-kanji",          FC_JDEC,    0, /* Japanese DEC Kanji */
#endif /* KANJI */
    "dec-mcs",            FC_DECMCS,  CM_INV, /* DEC multinational char set */
    "dec-multinational",  FC_DECMCS,  0, /* DEC multinational character set */
    "dg-international",   FC_DGMCS,   0, /* Data General multinational */
    "dutch",              FC_DUASCII, 0, /* Dutch NRC */
#ifdef GREEK
    "elot927-greek",      FC_ELOT,    0, /* ELOT 927 Greek */
    "elot928-greek",      FC_GREEK,   0, /* Same as ISO 8859-7 Latin/Greek */
#endif /* GREEK */
#ifdef KANJI
    "euc-jp",             FC_JEUC,    0, /* Japanese EUC */
#endif /* KANJI */
    "finnish",            FC_FIASCII, 0, /* Finnish NRC */
    "french",             FC_FRASCII, 0, /* French NRC */
    "fr-canadian",        FC_FCASCII, CM_INV, /* French Canadian NRC */
    "german",             FC_GEASCII, 0, /* German NRC */
#ifdef GREEK
    "greek-iso",          FC_GREEK,   0, /* ISO 8859-7 Latin/Greek */
#endif /* GREEK */
#ifdef HEBREW
    "he",                 FC_HEBREW,  CM_ABR|CM_INV,
    "heb",                FC_HEBREW,  CM_ABR|CM_INV,
    "hebr",               FC_HEBREW,  CM_ABR|CM_INV,
    "hebre",              FC_HEBREW,  CM_ABR|CM_INV,
    "hebrew",             FC_HEBREW,  CM_ABR|CM_INV,
    "hebrew-7",           FC_HE7,     0, /* DEC 7-Bit Hebrew */
    "hebrew-iso",         FC_HEBREW,  0, /* ISO Latin/Hebrew */
#endif /* HEBREW */
    "hp-roman8",          FC_HPR8,    0, /* Hewlett Packard Roman8 */
    "hungarian",          FC_HUASCII, 0, /* Hungarian NRC */

    "ibm437",             FC_CP437,   CM_INV, /* PC CP437 */
    "ibm850",             FC_CP850,   CM_INV, /* PC CP850 (not in MIME) */
#ifdef LATIN2
    "ibm852",             FC_CP852,   CM_INV, /* PC CP852 */
#endif /* LATIN2 */
#ifdef CYRILLIC
    "ibm855",             FC_CP855,   CM_INV, /* PC CP855 */
#endif /* CYRILLIC */
    "ibm858",             FC_CP858,   CM_INV, /* PC CP858 (not in MIME) */
#ifdef HEBREW
    "ibm862",             FC_CP862,   CM_INV, /* PC CP862 (not in MIME) */
#endif /* HEBREW */
#ifdef CYRILLIC
    "ibm866",             FC_CP866,   CM_INV, /* CP866 Cyrillic */
#endif /* CYRILLIC */
#ifdef GREEK
    "ibm869",             FC_CP869,   CM_INV, /* CP869 Greek */
#endif /* GREEK */

#ifdef UNICODE
    "iso-10646-ucs-2",    FC_UCS2,    CM_INV, /* ISO 10646 / Unicode UCS-2 */
#endif /* UNICODE */
    "iso-8859-1",         FC_1LATIN,  CM_INV, /* ISO Latin Alphabet 1 */
    "iso-8859-15",        FC_9LATIN,  CM_INV, /* ISO Latin Alphabet 9 (yes) */
    "iso-8859-2",         FC_2LATIN,  CM_INV, /* ISO Latin Alphabet 2 */
#ifdef CYRILLIC
    "iso-8859-5",         FC_CYRILL,  CM_INV, /* ISO Latin/Cyrillic Alphabet */
#endif /* CYRILLIC */
#ifdef GREEK
    "iso-8859-7",         FC_GREEK,   CM_INV, /* ISO 8859-7 Latin/Greek */
#endif /* GREEK */
#ifdef HEBREW
    "iso-8859-8",         FC_HEBREW,  CM_INV, /* ISO Latin/Hebrew */
#endif /* HEBREW */
#ifdef KANJI
    "iso2022jp-kanji",    FC_JIS7,    CM_INV, /* Synonym for JIS-7 */
#endif /* KANJI */

    "iso646-gb",          FC_UKASCII, CM_INV, /* British NRC */
    "iso646-ca",          FC_FCASCII, CM_INV, /* French Canadian NRC */
    "iso646-de",          FC_GEASCII, CM_INV, /* German NRC */
    "iso646-dk",          FC_NOASCII, CM_INV, /* Norwegian and Danish NRC */
    "iso646-es",          FC_SPASCII, CM_INV, /* Spanish NRC */
    "iso646-fi",          FC_FIASCII, CM_INV, /* Finnish NRC */
    "iso646-fr",          FC_FRASCII, CM_INV, /* French NRC */
    "iso646-hu",          FC_HUASCII, CM_INV, /* Hungarian NRC */
    "iso646-it",          FC_ITASCII, CM_INV, /* Italian NRC */
    "iso646-no",          FC_NOASCII, CM_INV, /* Norwegian and Danish NRC */
    "iso646-po",          FC_POASCII, CM_INV, /* Portuguese NRC */
    "iso646-se",          FC_SWASCII, CM_INV, /* Swedish NRC */

    "italian",            FC_ITASCII, CM_INV, /* Italian NRC */
#ifdef KANJI
    "japanese-euc",       FC_JEUC,    CM_INV, /* Japanese EUC */
    "jis7-kanji",         FC_JIS7,    0, /* Japanese JIS7 7bit code */
#endif /* KANJI */
#ifdef CYRILLIC
    "k",                  FC_KOI8,    CM_ABR|CM_INV,
    "ko",                 FC_KOI8,    CM_ABR|CM_INV,
    "koi",                FC_KOI8,    CM_ABR|CM_INV,
    "koi7",               FC_KOI7,    0, /* Short KOI Cyrillic */
    "koi8",               FC_KOI8,    0, /* Old KOI-8 Cyrillic */
    "koi8-e",             FC_KOI8,    CM_INV, /* Old KOI-8 Cyrillic */
    "koi8-cyrillic",      FC_KOI8,    CM_INV,
    "koi8-r",             FC_KOI8R,   CM_INV, /* KOI8-R RFC1489 */
    "koi8-u",             FC_KOI8U,   CM_INV, /* KOI8-U RFC2319 */
    "koi8r",              FC_KOI8R,   0, /* KOI8-R RFC1489 */
    "koi8u",              FC_KOI8U,   0, /* KOI8-U RFC2319 */
#endif /* CYRILLIC */
    "l",                  FC_1LATIN,  CM_ABR|CM_INV,
    "la",                 FC_1LATIN,  CM_ABR|CM_INV,
    "lat",                FC_1LATIN,  CM_ABR|CM_INV,
    "lati",               FC_1LATIN,  CM_ABR|CM_INV,
    "latin",              FC_1LATIN,  CM_ABR|CM_INV,
    "latin1-iso",         FC_1LATIN,  0, /* ISO Latin Alphabet 1 */
#ifdef LATIN2
    "latin2-iso",         FC_2LATIN,  0, /* ISO Latin Alphabet 2 */
#endif /* LATIN2 */
    "latin9-iso",         FC_9LATIN,  0, /* ISO Latin Alphabet 9 */
    "macintosh-latin",    FC_APPQD,   0, /* "Extended Mac Latin" */
#ifdef LATIN2
    "mazovia-pc",         FC_MAZOVIA, 0, /* Polish Mazovia PC code page */
#endif /* LATIN2 */
    "next-multinational", FC_NEXT,    0, /* NeXT workstation */
    "norwegian",          FC_NOASCII, 0, /* Norwegian and Danish NRC */
    "portuguese",         FC_POASCII, 0, /* Portuguese NRC */
#ifdef KANJI
    "shift-jis-kanji",    FC_SHJIS,   0, /* Japanese Kanji Shift-JIS */
    "shift_jis",          FC_SHJIS,   CM_INV, /* Japanese Kanji Shift-JIS */
#endif /* KANJI */
#ifdef CYRILLIC
    "short-koi",          FC_KOI7,    0, /* Short KOI Cyrillic */
#endif /* CYRILLIC */
    "spanish",            FC_SPASCII, 0, /* Spanish NRC */
    "swedish",            FC_SWASCII, 0, /* Swedish NRC */
    "swiss",              FC_CHASCII, 0, /* Swiss NRC */
#ifdef UNICODE
    "ucs2",               FC_UCS2,    0, /* ISO 10646 / Unicode UCS-2 */
#endif /* UNICODE */
    "us-ascii",           FC_USASCII, CM_INV, /* MIME */
    "usascii",            FC_USASCII, CM_INV,
#ifdef UNICODE
    "utf-8",              FC_UTF8,    CM_INV, /* ISO 10646 / Unicode UTF-8 */
    "utf8",               FC_UTF8,    0, /* ISO 10646 / Unicode UTF-8 */
#endif /* UNICODE */
#ifdef LATIN2
    "windows-1250",       FC_CP1250,  CM_INV, /* Windows CP 1250 */
#endif /* LATIN2 */
#ifdef CYRILLIC
    "windows-1251",       FC_CP1251,  CM_INV, /* Windows CP 1251 */
#endif /* CYRILLIC */
    "windows-1252",       FC_CP1252,  CM_INV, /* Windows CP 1252 */
    "", 0, 0
};
int nfilc = (sizeof(fcstab) / sizeof(struct keytab)) - 1;

struct keytab ttcstab[] = { /* Keyword table for SET TERMINAL CHARACTER-SET */
/*
  IMPORTANT: This table is a replica of fcstab, immediately above, with the
  addition of TRANSPARENT and deletion of the Japanese sets.  If you make
  changes to this table, make the corresponding changes to fcstab.
*/
/* Keyword             Value       Flags */
    "apple-quickdraw",    FC_APPQD,   CM_INV, /* Apple Quickdraw */
    "ascii",              FC_USASCII, 0, /* ASCII */
    "british",            FC_UKASCII, 0, /* British NRC */
    "bulgaria-pc",        FC_BULGAR,  0, /* Bulgarian PC Code Page */
    "canadian-french",    FC_FCASCII, 0, /* French Canadian NRC */
#ifdef LATIN2
    "cp1250",             FC_CP1250,  0, /* Windows CP 1250 */
#endif /* LATIN2 */
#ifdef CYRILLIC
    "cp1251-cyrillic",    FC_CP1251,  0, /* Windows CP 1251 */
#endif /* CYRILLIC */
    "cp1252",             FC_CP1252,  0, /* Windows CP 1252 */
    "cp437",              FC_CP437,   0, /* PC CP437 */
    "cp850",              FC_CP850,   0, /* PC CP850 */
#ifdef LATIN2
    "cp852",              FC_CP852,   0, /* PC CP852 */
#endif /* LATIN2 */
#ifdef CYRILLIC
    "cp855-cyrillic",     FC_CP855,   0, /* PC CP855 */
#endif /* CYRILLIC */
    "cp858",              FC_CP858,   0, /* PC CP858 */
#ifdef HEBREW
    "cp862-hebrew",       FC_CP862,   0, /* PC CP862 */
#endif /* HEBREW */
#ifdef CYRILLIC
    "cp866-cyrillic",     FC_CP866,   0, /* CP866 Cyrillic */
#endif /* CYRILLIC */
#ifdef GREEK
    "cp869-greek",        FC_CP869,   0, /* CP869 Greek */
#endif /* GREEK */
#ifdef CYRILLIC
    "cyrillic-iso",       FC_CYRILL,  0, /* ISO Latin/Cyrillic Alphabet */
#endif /* CYRILLIC */
    "danish",             FC_NOASCII, 0, /* Norwegian and Danish NRC */
    "dec-mcs",            FC_DECMCS,  CM_INV, /* DEC multinational char set */
    "dec-multinational",  FC_DECMCS,  0, /* DEC multinational character set */
    "dg-international",   FC_DGMCS,   0, /* Data General multinational */
    "dutch",              FC_DUASCII, 0, /* Dutch NRC */
#ifdef GREEK
    "elot927-greek",      FC_ELOT,    0, /* ELOT 927 Greek */
    "elot928-greek",      FC_GREEK,   0, /* Same as ISO 8859-7 Latin/Greek */
#endif /* GREEK */
    "finnish",            FC_FIASCII, 0, /* Finnish NRC */
    "french",             FC_FRASCII, 0, /* French NRC */
    "fr-canadian",        FC_FCASCII, CM_INV, /* French Canadian NRC */
    "german",             FC_GEASCII, 0, /* German NRC */
#ifdef GREEK
    "greek-iso",          FC_GREEK,   0, /* ISO 8859-7 Latin/Greek */
#endif /* GREEK */
#ifdef HEBREW
    "he",                 FC_HEBREW,  CM_ABR|CM_INV,
    "heb",                FC_HEBREW,  CM_ABR|CM_INV,
    "hebr",               FC_HEBREW,  CM_ABR|CM_INV,
    "hebre",              FC_HEBREW,  CM_ABR|CM_INV,
    "hebrew",             FC_HEBREW,  CM_ABR|CM_INV,
    "hebrew-7",           FC_HE7,     0, /* DEC 7-Bit Hebrew */
    "hebrew-iso",         FC_HEBREW,  0, /* ISO Latin/Hebrew */
#endif /* HEBREW */
    "hp-roman8",          FC_HPR8,    0, /* Hewlett Packard Roman8 */
    "hungarian",          FC_HUASCII, 0, /* Hungarian NRC */

    "ibm437",             FC_CP437,   CM_INV, /* PC CP437 */
    "ibm850",             FC_CP850,   CM_INV, /* PC CP850 (not in MIME) */
#ifdef LATIN2
    "ibm852",             FC_CP852,   CM_INV, /* PC CP852 */
#endif /* LATIN2 */
#ifdef CYRILLIC
    "ibm855",             FC_CP855,   CM_INV, /* PC CP855 */
#endif /* CYRILLIC */
    "ibm858",             FC_CP858,   CM_INV, /* PC CP858 (not in MIME) */
#ifdef HEBREW
    "ibm862",             FC_CP862,   CM_INV, /* PC CP862 (not in MIME) */
#endif /* HEBREW */
#ifdef CYRILLIC
    "ibm866",             FC_CP866,   CM_INV, /* CP866 Cyrillic */
#endif /* CYRILLIC */
#ifdef GREEK
    "ibm869",             FC_CP869,   CM_INV, /* CP869 Greek */
#endif /* GREEK */

#ifdef UNICODE
    "iso-10646-ucs-2",    FC_UCS2,    CM_INV, /* ISO 10646 / Unicode UCS-2 */
#endif /* UNICODE */
    "iso-8859-1",         FC_1LATIN,  CM_INV, /* ISO Latin Alphabet 1 */
    "iso-8859-15",        FC_9LATIN,  CM_INV, /* ISO Latin Alphabet 9 (yes) */
    "iso-8859-2",         FC_2LATIN,  CM_INV, /* ISO Latin Alphabet 2 */
#ifdef CYRILLIC
    "iso-8859-5",         FC_CYRILL,  CM_INV, /* ISO Latin/Cyrillic Alphabet */
#endif /* CYRILLIC */
#ifdef GREEK
    "iso-8859-7",         FC_GREEK,   CM_INV, /* ISO 8859-7 Latin/Greek */
#endif /* GREEK */
#ifdef HEBREW
    "iso-8859-8",         FC_HEBREW,  CM_INV, /* ISO Latin/Hebrew */
#endif /* HEBREW */

    "iso646-gb",          FC_UKASCII, CM_INV, /* British NRC */
    "iso646-ca",          FC_FCASCII, CM_INV, /* French Canadian NRC */
    "iso646-de",          FC_GEASCII, CM_INV, /* German NRC */
    "iso646-dk",          FC_NOASCII, CM_INV, /* Norwegian and Danish NRC */
    "iso646-es",          FC_SPASCII, CM_INV, /* Spanish NRC */
    "iso646-fi",          FC_FIASCII, CM_INV, /* Finnish NRC */
    "iso646-fr",          FC_FRASCII, CM_INV, /* French NRC */
    "iso646-hu",          FC_HUASCII, CM_INV, /* Hungarian NRC */
    "iso646-it",          FC_ITASCII, CM_INV, /* Italian NRC */
    "iso646-no",          FC_NOASCII, CM_INV, /* Norwegian and Danish NRC */
    "iso646-po",          FC_POASCII, CM_INV, /* Portuguese NRC */
    "iso646-se",          FC_SWASCII, CM_INV, /* Swedish NRC */

    "italian",            FC_ITASCII, CM_INV, /* Italian NRC */

#ifdef CYRILLIC
    "k",                  FC_KOI8,    CM_ABR|CM_INV,
    "ko",                 FC_KOI8,    CM_ABR|CM_INV,
    "koi",                FC_KOI8,    CM_ABR|CM_INV,
    "koi7",               FC_KOI7,    0, /* Short KOI Cyrillic */
    "koi8",               FC_KOI8,    0, /* Old KOI-8 Cyrillic */
    "koi8-e",             FC_KOI8,    CM_INV, /* Old KOI-8 Cyrillic */
    "koi8-cyrillic",      FC_KOI8,    CM_INV,
    "koi8-r",             FC_KOI8R,   CM_INV, /* KOI8-R RFC1489 */
    "koi8-u",             FC_KOI8U,   CM_INV, /* KOI8-U RFC2319 */
    "koi8r",              FC_KOI8R,   0, /* KOI8-R RFC1489 */
    "koi8u",              FC_KOI8U,   0, /* KOI8-U RFC2319 */
#endif /* CYRILLIC */
    "l",                  FC_1LATIN,  CM_ABR|CM_INV,
    "la",                 FC_1LATIN,  CM_ABR|CM_INV,
    "lat",                FC_1LATIN,  CM_ABR|CM_INV,
    "lati",               FC_1LATIN,  CM_ABR|CM_INV,
    "latin",              FC_1LATIN,  CM_ABR|CM_INV,
    "latin1-iso",         FC_1LATIN,  0, /* ISO Latin Alphabet 1 */
#ifdef LATIN2
    "latin2-iso",         FC_2LATIN,  0, /* ISO Latin Alphabet 2 */
#endif /* LATIN2 */
    "latin9-iso",         FC_9LATIN,  0, /* ISO Latin Alphabet 9 */
    "macintosh-latin",    FC_APPQD,   0, /* "Extended Mac Latin" */
#ifdef LATIN2
    "mazovia-pc",         FC_MAZOVIA, 0, /* Polish Mazovia PC code page */
#endif /* LATIN2 */
    "next-multinational", FC_NEXT,    0, /* NeXT workstation */
    "norwegian",          FC_NOASCII, 0, /* Norwegian and Danish NRC */
    "portuguese",         FC_POASCII, 0, /* Portuguese NRC */

#ifdef CYRILLIC
    "short-koi",          FC_KOI7,    0, /* Short KOI Cyrillic */
#endif /* CYRILLIC */
    "spanish",            FC_SPASCII, 0, /* Spanish NRC */
    "swedish",            FC_SWASCII, 0, /* Swedish NRC */
    "swiss",              FC_CHASCII, 0, /* Swiss NRC */
    "transparent",        FC_TRANSP,  0, /* Transparent */
#ifdef UNICODE
    "ucs2",               FC_UCS2,    0, /* ISO 10646 / Unicode UCS-2 */
#endif /* UNICODE */
    "us-ascii",           FC_USASCII, CM_INV, /* MIME */
    "usascii",            FC_USASCII, CM_INV,
#ifdef UNICODE
    "utf-8",              FC_UTF8,    CM_INV, /* ISO 10646 / Unicode UTF-8 */
    "utf8",               FC_UTF8,    0, /* ISO 10646 / Unicode UTF-8 */
#endif /* UNICODE */
#ifdef LATIN2
    "windows-1250",       FC_CP1250,  CM_INV, /* Windows CP 1250 */
#endif /* LATIN2 */
#ifdef CYRILLIC
    "windows-1251",       FC_CP1251,  CM_INV, /* Windows CP 1251 */
#endif /* CYRILLIC */
    "windows-1252",       FC_CP1252,  CM_INV, /* Windows CP 1252 */
    "", 0, 0
};
int ntermc = (sizeof(ttcstab) / sizeof(struct keytab)) - 1;

/* This table contains the equivalent FCS number for each TCS. */
/* If the TC_xxx symbol definitions are ever changed, fix this table. */
/* Ditto if another TCS is added. */

int
cseqtab[MAXTCSETS+1] = {		/* TCS/FCS equivalency table */
    -1,					/*  0 = Transparent */
    FC_USASCII,				/*  1 = ASCII */
    FC_1LATIN,				/*  2 = Latin-1 */
    FC_2LATIN,				/*  3 = Latin-2 */
    FC_CYRILL,				/*  4 = Latin/Cyrillic */
    FC_JEUC,				/*  5 = Japanese EUC */
    FC_HEBREW,				/*  6 = Latin/Hebrew */
    FC_GREEK,				/*  7 = Latin/Greek */
    FC_9LATIN,				/*  8 = Latin-9 */
    FC_UCS2,				/*  9 = UCS-2 */
    FC_UTF8				/* 10 = UTF-8 */
};

/*
 Languages:

 This table allows C-Kermit to have a SET LANGUAGE command to apply special
 language-specific rules when translating from a character set that contains
 national characters into plain ASCII, like German umlaut-a becomes ae.

 Originally, I thought it would be a good idea to let SET LANGUAGE also select
 an appropriate FILE CHARACTER-SET and TRANSFER CHARACTER-SET automatically,
 and these are included in the langinfo structure.  Later I realized that this
 was a bad idea.  Any particular language (e.g. Dutch) can be represented by
 many different and incompatible character sets.

 (But we could use the new (1998) ASSOCIATE command for this...)
*/

struct langinfo langs[] = {
/*  Language code   File Charset Xfer Charset Name */
    L_USASCII,      FC_USASCII,  TC_USASCII,  "ASCII (American English)",
    L_DANISH,       FC_NOASCII,  TC_1LATIN,   "Danish",
    L_DUTCH,        FC_DUASCII,  TC_1LATIN,   "Dutch",
    L_FINNISH,      FC_FIASCII,  TC_1LATIN,   "Finnish",
    L_FRENCH,       FC_FRASCII,  TC_1LATIN,   "French",
    L_GERMAN,       FC_GEASCII,  TC_1LATIN,   "German",
#ifdef GREEK
    L_GREEK,        FC_GREEK,    TC_GREEK,    "Greek",
#endif /* GREEK */
#ifdef HEBREW
    L_HEBREW,       FC_HEBREW,   TC_HEBREW,   "Hebrew",
#endif /* HEBREW */
    L_HUNGARIAN,    FC_HUASCII,  TC_2LATIN,   "Hungarian",
    L_ICELANDIC,    FC_USASCII,  TC_1LATIN,   "Icelandic",
    L_ITALIAN,      FC_ITASCII,  TC_1LATIN,   "Italian",
#ifdef KANJI
    L_JAPANESE,     FC_JEUC,     TC_JEUC,     "Japanese",
#endif /* KANJI */
    L_NORWEGIAN,    FC_NOASCII,  TC_1LATIN,   "Norwegian",
    L_PORTUGUESE,   FC_POASCII,  TC_1LATIN,   "Portuguese",
#ifdef CYRILLIC
    L_RUSSIAN,      FC_CP866,    TC_CYRILL,   "Russian",
#endif /* CYRILLIC */
    L_SPANISH,      FC_SPASCII,  TC_1LATIN,   "Spanish",
    L_SWEDISH,      FC_SWASCII,  TC_1LATIN,   "Swedish",
    L_SWISS,        FC_CHASCII,  TC_1LATIN,   "Swiss"
};
int nlangs = (sizeof(langs) / sizeof(struct langinfo));

/*
  Keyword table for the SET LANGUAGE command.
  Only a few of these (German, Scandinavian, etc) actually do anything.
  The language is used to invoke special translation rules when converting
  from an 8-bit character set to ASCII; for example, German u-diaeresis
  becomes "ue", Dutch y-diaeresis becomes "ij".  Languages without associated
  rules are invisible (CM_INV).
*/
struct keytab lngtab[] = {
    "ascii",            L_USASCII,    CM_INV,
    "danish",           L_DANISH,     0,
    "dutch",            L_DUTCH,      0,
    "english",          L_USASCII,    CM_INV,
    "finnish",          L_FINNISH,    0,
    "french",           L_FRENCH,     0,
    "german",           L_GERMAN,     0,
#ifdef GREEK
    "greek",            L_GREEK,      CM_INV,
#endif /* GREEK */
#ifdef HEBREW
    "hebrew",           L_HEBREW,     CM_INV,
#endif /* HEBREW */
    "hungarian",        L_HUNGARIAN,  CM_INV,
    "icelandic",        L_ICELANDIC,  0,
    "italian",          L_ITALIAN,    CM_INV,
#ifdef KANJI
    "japanese",         L_JAPANESE,   CM_INV,
#endif /* KANJI */
    "norwegian",        L_NORWEGIAN,  0,
    "none",             L_USASCII,    0,
    "portuguese",       L_PORTUGUESE, CM_INV,
#ifdef CYRILLIC
    "russian",          L_RUSSIAN,    0,
#endif /* CYRILLIC */
    "spanish",          L_SPANISH,    CM_INV,
    "swedish",          L_SWEDISH,    0,
#ifdef CYRILLIC
    "ukrainian",        L_RUSSIAN,    0,
#endif /* CYRILLIC */
    "", 0, 0
};
int nlng = (sizeof(lngtab) / sizeof(struct keytab)) - 1; /* how many */


/* Translation tables ... */

/*
  For each pair of (transfer,file) character sets, we need two translation
  functions, one for sending, one for receiving.
*/

/*
  Here is the first table, Latin-1 to ASCII, fully annotated...
  This one is absolutely NOT invertible, since we're going from an 8-bit
  set to a 7-bit set.  Accented letters are mapped to unaccented
  equivalents, C1 control characters are all translated to "?", etc.
*/
CONST CHAR
yl1as[] = {  /* ISO 8859-1 Latin Alphabet 1 to US ASCII */
      /*  Source character    Description               => Translation */
      /*  Dec row/col Set                                           */
  0,  /*  000  00/00  C0 NUL  Ctrl-@                    =>  (self)  */
  1,  /*  001  00/01  C0 SOH  Ctrl-A                    =>  (self)  */
  2,  /*  002  00/02  C0 STX  Ctrl-B                    =>  (self)  */
  3,  /*  003  00/03  C0 ETX  Ctrl-C                    =>  (self)  */
  4,  /*  004  00/04  C0 EOT  Ctrl-D                    =>  (self)  */
  5,  /*  005  00/05  C0 ENQ  Ctrl-E                    =>  (self)  */
  6,  /*  006  00/06  C0 ACK  Ctrl-F                    =>  (self)  */
  7,  /*  007  00/07  C0 BEL  Ctrl-G                    =>  (self)  */
  8,  /*  008  00/08  C0 BS   Ctrl-H                    =>  (self)  */
  9,  /*  009  00/09  C0 HT   Ctrl-I                    =>  (self)  */
 10,  /*  010  00/10  C0 LF   Ctrl-J                    =>  (self)  */
 11,  /*  011  00/11  C0 VT   Ctrl-K                    =>  (self)  */
 12,  /*  012  00/12  C0 FF   Ctrl-L                    =>  (self)  */
 13,  /*  013  00/13  C0 CR   Ctrl-M                    =>  (self)  */
 14,  /*  014  00/14  C0 SO   Ctrl-N                    =>  (self)  */
 15,  /*  015  00/15  C0 SI   Ctrl-O                    =>  (self)  */
 16,  /*  016  01/00  C0 DLE  Ctrl-P                    =>  (self)  */
 17,  /*  017  01/01  C0 DC1  Ctrl-Q                    =>  (self)  */
 18,  /*  018  01/02  C0 DC2  Ctrl-R                    =>  (self)  */
 19,  /*  019  01/03  C0 DC3  Ctrl-S                    =>  (self)  */
 20,  /*  020  01/04  C0 DC4  Ctrl-T                    =>  (self)  */
 21,  /*  021  01/05  C0 NAK  Ctrl-U                    =>  (self)  */
 22,  /*  022  01/06  C0 SYN  Ctrl-V                    =>  (self)  */
 23,  /*  023  01/07  C0 ETB  Ctrl-W                    =>  (self)  */
 24,  /*  024  01/08  C0 CAN  Ctrl-X                    =>  (self)  */
 25,  /*  025  01/09  C0 EM   Ctrl-Y                    =>  (self)  */
 26,  /*  026  01/10  C0 SUB  Ctrl-Z                    =>  (self)  */
 27,  /*  027  01/11  C0 ESC  Ctrl-[                    =>  (self)  */
 28,  /*  028  01/12  C0 FS   Ctrl-\                    =>  (self)  */
 29,  /*  029  01/13  C0 GS   Ctrl-]                    =>  (self)  */
 30,  /*  030  01/14  C0 RS   Ctrl-^                    =>  (self)  */
 31,  /*  031  01/15  C0 US   Ctrl-_                    =>  (self)  */
 32,  /*  032  02/00     SP   Space                     =>  (self)  */
 33,  /*  033  02/01  G0 !    Exclamation mark          =>  (self)  */
 34,  /*  034  02/02  G0 "    Doublequote               =>  (self)  */
 35,  /*  035  02/03  G0 #    Number sign               =>  (self)  */
 36,  /*  036  02/04  G0 $    Dollar sign               =>  (self)  */
 37,  /*  037  02/05  G0 %    Percent sign              =>  (self)  */
 38,  /*  038  02/06  G0 &    Ampersand                 =>  (self)  */
 39,  /*  039  02/07  G0 '    Apostrophe                =>  (self)  */
 40,  /*  040  02/08  G0 (    Left parenthesis          =>  (self)  */
 41,  /*  041  02/09  G0 )    Right parenthesis         =>  (self)  */
 42,  /*  042  02/10  G0 *    Asterisk                  =>  (self)  */
 43,  /*  043  02/11  G0 +    Plus sign                 =>  (self)  */
 44,  /*  044  02/12  G0 ,    Comma                     =>  (self)  */
 45,  /*  045  02/13  G0 -    Hyphen, minus sign        =>  (self)  */
 46,  /*  046  02/14  G0 .    Period, full stop         =>  (self)  */
 47,  /*  047  02/15  G0 /    Slash, solidus            =>  (self)  */
 48,  /*  048  03/00  G0 0    Digit 0                   =>  (self)  */
 49,  /*  049  03/01  G0 1    Digit 1                   =>  (self)  */
 50,  /*  050  03/02  G0 2    Digit 2                   =>  (self)  */
 51,  /*  051  03/03  G0 3    Digit 3                   =>  (self)  */
 52,  /*  052  03/04  G0 4    Digit 4                   =>  (self)  */
 53,  /*  053  03/05  G0 5    Digit 5                   =>  (self)  */
 54,  /*  054  03/06  G0 6    Digit 6                   =>  (self)  */
 55,  /*  055  03/07  G0 7    Digit 7                   =>  (self)  */
 56,  /*  056  03/08  G0 8    Digit 8                   =>  (self)  */
 57,  /*  057  03/09  G0 9    Digit 9                   =>  (self)  */
 58,  /*  058  03/10  G0 :    Colon                     =>  (self)  */
 59,  /*  059  03/11  G0 ;    Semicolon                 =>  (self)  */
 60,  /*  060  03/12  G0 <    Less-than sign            =>  (self)  */
 61,  /*  061  03/13  G0 =    Equals sign               =>  (self)  */
 62,  /*  062  03/14  G0 >    Greater-than sign         =>  (self)  */
 63,  /*  063  03/15  G0 ?    Question mark             =>  (self)  */
 64,  /*  064  04/00  G0 @    Commercial at sign        =>  (self)  */
 65,  /*  065  04/01  G0 A    Letter A                  =>  (self)  */
 66,  /*  066  04/02  G0 B    Letter B                  =>  (self)  */
 67,  /*  067  04/03  G0 C    Letter C                  =>  (self)  */
 68,  /*  068  04/04  G0 D    Letter D                  =>  (self)  */
 69,  /*  069  04/05  G0 E    Letter E                  =>  (self)  */
 70,  /*  070  04/06  G0 F    Letter F                  =>  (self)  */
 71,  /*  071  04/07  G0 G    Letter G                  =>  (self)  */
 72,  /*  072  04/08  G0 H    Letter H                  =>  (self)  */
 73,  /*  073  04/09  G0 I    Letter I                  =>  (self)  */
 74,  /*  074  04/10  G0 J    Letter J                  =>  (self)  */
 75,  /*  075  04/11  G0 K    Letter K                  =>  (self)  */
 76,  /*  076  04/12  G0 L    Letter L                  =>  (self)  */
 77,  /*  077  04/13  G0 M    Letter M                  =>  (self)  */
 78,  /*  078  04/14  G0 N    Letter N                  =>  (self)  */
 79,  /*  079  04/15  G0 O    Letter O                  =>  (self)  */
 80,  /*  080  05/00  G0 P    Letter P                  =>  (self)  */
 81,  /*  081  05/01  G0 Q    Letter Q                  =>  (self)  */
 82,  /*  082  05/02  G0 R    Letter R                  =>  (self)  */
 83,  /*  083  05/03  G0 S    Letter S                  =>  (self)  */
 84,  /*  084  05/04  G0 T    Letter T                  =>  (self)  */
 85,  /*  085  05/05  G0 U    Letter U                  =>  (self)  */
 86,  /*  086  05/06  G0 V    Letter V                  =>  (self)  */
 87,  /*  087  05/07  G0 W    Letter W                  =>  (self)  */
 88,  /*  088  05/08  G0 X    Letter X                  =>  (self)  */
 89,  /*  089  05/09  G0 Y    Letter Y                  =>  (self)  */
 90,  /*  090  05/10  G0 Z    Letter Z                  =>  (self)  */
 91,  /*  091  05/11  G0 [    Left square bracket       =>  (self)  */
 92,  /*  092  05/12  G0 \    Reverse slash             =>  (self)  */
 93,  /*  093  05/13  G0 ]    Right square bracket      =>  (self)  */
 94,  /*  094  05/14  G0 ^    Circumflex accent         =>  (self)  */
 95,  /*  095  05/15  G0 _    Underline, low line       =>  (self)  */
 96,  /*  096  06/00  G0 `    Grave accent              =>  (self)  */
 97,  /*  097  06/01  G0 a    Letter a                  =>  (self)  */
 98,  /*  098  06/02  G0 b    Letter b                  =>  (self)  */
 99,  /*  099  06/03  G0 c    Letter c                  =>  (self)  */
100,  /*  100  06/04  G0 d    Letter d                  =>  (self)  */
101,  /*  101  06/05  G0 e    Letter e                  =>  (self)  */
102,  /*  102  06/06  G0 f    Letter f                  =>  (self)  */
103,  /*  103  06/07  G0 g    Letter g                  =>  (self)  */
104,  /*  104  06/08  G0 h    Letter h                  =>  (self)  */
105,  /*  105  06/09  G0 i    Letter i                  =>  (self)  */
106,  /*  106  06/10  G0 j    Letter j                  =>  (self)  */
107,  /*  107  06/11  G0 k    Letter k                  =>  (self)  */
108,  /*  108  06/12  G0 l    Letter l                  =>  (self)  */
109,  /*  109  06/13  G0 m    Letter m                  =>  (self)  */
110,  /*  110  06/14  G0 n    Letter n                  =>  (self)  */
111,  /*  111  06/15  G0 o    Letter o                  =>  (self)  */
112,  /*  112  07/00  G0 p    Letter p                  =>  (self)  */
113,  /*  113  07/01  G0 q    Letter q                  =>  (self)  */
114,  /*  114  07/02  G0 r    Letter r                  =>  (self)  */
115,  /*  115  07/03  G0 s    Letter s                  =>  (self)  */
116,  /*  116  07/04  G0 t    Letter t                  =>  (self)  */
117,  /*  117  07/05  G0 u    Letter u                  =>  (self)  */
118,  /*  118  07/06  G0 v    Letter v                  =>  (self)  */
119,  /*  119  07/07  G0 w    Letter w                  =>  (self)  */
120,  /*  120  07/08  G0 x    Letter x                  =>  (self)  */
121,  /*  121  07/09  G0 y    Letter y                  =>  (self)  */
122,  /*  122  07/10  G0 z    Letter z                  =>  (self)  */
123,  /*  123  07/11  G0 {    Left curly bracket        =>  (self)  */
124,  /*  124  07/12  G0 |    Vertical bar              =>  (self)  */
125,  /*  125  07/13  G0 }    Right curly bracket       =>  (self)  */
126,  /*  126  07/14  G0 ~    Tilde                     =>  (self)  */
127,  /*  127  07/15     DEL  Delete, Rubout            =>  (self)  */
UNK,  /*  128  08/00  C1                                =>  UNK     */
UNK,  /*  129  08/01  C1                                =>  UNK     */
UNK,  /*  130  08/02  C1                                =>  UNK     */
UNK,  /*  131  08/03  C1                                =>  UNK     */
UNK,  /*  132  08/04  C1 IND                            =>  UNK     */
UNK,  /*  133  08/05  C1 NEL                            =>  UNK     */
UNK,  /*  134  08/06  C1 SSA                            =>  UNK     */
UNK,  /*  135  08/07  C1 ESA                            =>  UNK     */
UNK,  /*  136  08/08  C1 HTS                            =>  UNK     */
UNK,  /*  137  08/09  C1                                =>  UNK     */
UNK,  /*  138  08/10  C1                                =>  UNK     */
UNK,  /*  139  08/11  C1                                =>  UNK     */
UNK,  /*  140  08/12  C1                                =>  UNK     */
UNK,  /*  141  08/13  C1 RI                             =>  UNK     */
UNK,  /*  142  08/14  C1 SS2                            =>  UNK     */
UNK,  /*  143  08/15  C1 SS3                            =>  UNK     */
UNK,  /*  144  09/00  C1 DCS                            =>  UNK     */
UNK,  /*  145  09/01  C1                                =>  UNK     */
UNK,  /*  146  09/02  C1                                =>  UNK     */
UNK,  /*  147  09/03  C1 STS                            =>  UNK     */
UNK,  /*  148  09/04  C1                                =>  UNK     */
UNK,  /*  149  09/05  C1                                =>  UNK     */
UNK,  /*  150  09/06  C1 SPA                            =>  UNK     */
UNK,  /*  151  09/07  C1 EPA                            =>  UNK     */
UNK,  /*  152  09/08  C1                                =>  UNK     */
UNK,  /*  153  09/09  C1                                =>  UNK     */
UNK,  /*  154  09/10  C1                                =>  UNK     */
UNK,  /*  155  09/11  C1 CSI                            =>  UNK     */
UNK,  /*  156  09/12  C1 ST                             =>  UNK     */
UNK,  /*  157  09/13  C1 OSC                            =>  UNK     */
UNK,  /*  158  09/14  C1 PM                             =>  UNK     */
UNK,  /*  159  09/15  C1 APC                            =>  UNK     */
 32,  /*  160  10/00  G1      No-break space            =>  SP      */
 33,  /*  161  10/01  G1      Inverted exclamation      =>  !       */
 99,  /*  162  10/02  G1      Cent sign                 =>  c       */
 35,  /*  163  10/03  G1      Pound sign                =>  #       */
 36,  /*  164  10/04  G1      Currency sign             =>  $       */
 89,  /*  165  10/05  G1      Yen sign                  =>  Y       */
124,  /*  166  10/06  G1      Broken bar                =>  |       */
 80,  /*  167  10/07  G1      Paragraph sign            =>  P       */
 34,  /*  168  10/08  G1      Diaeresis                 =>  "       */
 67,  /*  169  10/09  G1      Copyright sign            =>  C       */
 97,  /*  170  10/10  G1      Feminine ordinal          =>  a       */
 34,  /*  171  10/11  G1      Left angle quotation      =>  "       */
126,  /*  172  10/12  G1      Not sign                  =>  ~       */
 45,  /*  173  10/13  G1      Soft hyphen               =>  -       */
 82,  /*  174  10/14  G1      Registered trade mark     =>  R       */
 95,  /*  175  10/15  G1      Macron                    =>  _       */
111,  /*  176  11/00  G1      Degree sign, ring above   =>  o       */
UNK,  /*  177  11/01  G1      Plus-minus sign           =>  UNK     */
 50,  /*  178  11/02  G1      Superscript two           =>  2       */
 51,  /*  179  11/03  G1      Superscript three         =>  3       */
 39,  /*  180  11/04  G1      Acute accent              =>  '       */
117,  /*  181  11/05  G1      Micro sign                =>  u       */
 45,  /*  182  11/06  G1      Pilcrow sign              =>  -       */
 45,  /*  183  11/07  G1      Middle dot                =>  -       */
 44,  /*  184  11/08  G1      Cedilla                   =>  ,       */
 49,  /*  185  11/09  G1      Superscript one           =>  1       */
111,  /*  186  11/10  G1      Masculine ordinal         =>  o       */
 34,  /*  187  11/11  G1      Right angle quotation     =>  "       */
UNK,  /*  188  11/12  G1      One quarter               =>  UNK     */
UNK,  /*  189  11/13  G1      One half                  =>  UNK     */
UNK,  /*  190  11/14  G1      Three quarters            =>  UNK     */
 63,  /*  191  11/15  G1      Inverted question mark    =>  ?       */
 65,  /*  192  12/00  G1      A grave                   =>  A       */
 65,  /*  193  12/01  G1      A acute                   =>  A       */
 65,  /*  194  12/02  G1      A circumflex              =>  A       */
 65,  /*  195  12/03  G1      A tilde                   =>  A       */
 65,  /*  196  12/04  G1      A diaeresis               =>  A       */
 65,  /*  197  12/05  G1      A ring above              =>  A       */
 65,  /*  198  12/06  G1      A with E                  =>  A       */
 67,  /*  199  12/07  G1      C Cedilla                 =>  C       */
 69,  /*  200  12/08  G1      E grave                   =>  E       */
 69,  /*  201  12/09  G1      E acute                   =>  E       */
 69,  /*  202  12/10  G1      E circumflex              =>  E       */
 69,  /*  203  12/11  G1      E diaeresis               =>  E       */
 73,  /*  204  12/12  G1      I grave                   =>  I       */
 73,  /*  205  12/13  G1      I acute                   =>  I       */
 73,  /*  206  12/14  G1      I circumflex              =>  I       */
 73,  /*  207  12/15  G1      I diaeresis               =>  I       */
 68,  /*  208  13/00  G1      Icelandic Eth             =>  D       */
 78,  /*  209  13/01  G1      N tilde                   =>  N       */
 79,  /*  210  13/02  G1      O grave                   =>  O       */
 79,  /*  211  13/03  G1      O acute                   =>  O       */
 79,  /*  212  13/04  G1      O circumflex              =>  O       */
 79,  /*  213  13/05  G1      O tilde                   =>  O       */
 79,  /*  214  13/06  G1      O diaeresis               =>  O       */
120,  /*  215  13/07  G1      Multiplication sign       =>  x       */
 79,  /*  216  13/08  G1      O oblique stroke          =>  O       */
 85,  /*  217  13/09  G1      U grave                   =>  U       */
 85,  /*  218  13/10  G1      U acute                   =>  U       */
 85,  /*  219  13/11  G1      U circumflex              =>  U       */
 85,  /*  220  13/12  G1      U diaeresis               =>  U       */
 89,  /*  221  13/13  G1      Y acute                   =>  Y       */
 84,  /*  222  13/14  G1      Icelandic Thorn           =>  T       */
115,  /*  223  13/15  G1      German sharp s            =>  s       */
 97,  /*  224  14/00  G1      a grave                   =>  a       */
 97,  /*  225  14/01  G1      a acute                   =>  a       */
 97,  /*  226  14/02  G1      a circumflex              =>  a       */
 97,  /*  227  14/03  G1      a tilde                   =>  a       */
 97,  /*  228  14/04  G1      a diaeresis               =>  a       */
 97,  /*  229  14/05  G1      a ring above              =>  a       */
 97,  /*  230  14/06  G1      a with e                  =>  a       */
 99,  /*  231  14/07  G1      c cedilla                 =>  c       */
101,  /*  232  14/08  G1      e grave                   =>  e       */
101,  /*  233  14/09  G1      e acute                   =>  e       */
101,  /*  234  14/10  G1      e circumflex              =>  e       */
101,  /*  235  14/11  G1      e diaeresis               =>  e       */
105,  /*  236  14/12  G1      i grave                   =>  i       */
105,  /*  237  14/13  G1      i acute                   =>  i       */
105,  /*  238  14/14  G1      i circumflex              =>  i       */
105,  /*  239  14/15  G1      i diaeresis               =>  i       */
100,  /*  240  15/00  G1      Icelandic eth             =>  d       */
110,  /*  241  15/01  G1      n tilde                   =>  n       */
111,  /*  242  15/02  G1      o grave                   =>  o       */
111,  /*  243  15/03  G1      o acute                   =>  o       */
111,  /*  244  15/04  G1      o circumflex              =>  o       */
111,  /*  245  15/05  G1      o tilde                   =>  o       */
111,  /*  246  15/06  G1      o diaeresis               =>  o       */
 47,  /*  247  15/07  G1      Division sign             =>  /       */
111,  /*  248  15/08  G1      o oblique stroke          =>  o       */
117,  /*  249  15/09  G1      u grave                   =>  u       */
117,  /*  250  15/10  G1      u acute                   =>  u       */
117,  /*  251  15/11  G1      u circumflex              =>  u       */
117,  /*  252  15/12  G1      u diaeresis               =>  u       */
121,  /*  253  15/13  G1      y acute                   =>  y       */
116,  /*  254  15/14  G1      Icelandic thorn           =>  t       */
121   /*  255  15/15  G1      y diaeresis               =>  y       */
};


/* Translation tables for ISO Latin Alphabet 1 to local file character sets */

/*
  Most of the remaining tables are not annotated like the one above, because
  the size of the resulting source file would be ridiculous.  Each row in the
  following tables corresponds to a column of ISO 8859-1.
*/

CONST CHAR
yl185[] = {  /* ISO 8859-1 Latin Alphabet 1 (Latin-1) to IBM Code Page 850 */
/*
  This is based on IBM's official invertible translation.  Reference: IBM
  Character Data Representation Architecture (CDRA), Level 1, Registry,
  SC09-1291-00 (1990), p.152.  (Note: Latin-1 is IBM Code Page 00819.)  Note:
  IBM's bizarre rearrangement of C0 controls and DEL has been undone in this
  table.
*/
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
186, 205, 201, 187, 200, 188, 204, 185, 203, 202, 206, 223, 220, 219, 254, 242,
179, 196, 218, 191, 192, 217, 195, 180, 194, 193, 197, 176, 177, 178, 213, 159,
255, 173, 189, 156, 207, 190, 221, 245, 249, 184, 166, 174, 170, 240, 169, 238,
248, 241, 253, 252, 239, 230, 244, 250, 247, 251, 167, 175, 172, 171, 243, 168,
183, 181, 182, 199, 142, 143, 146, 128, 212, 144, 210, 211, 222, 214, 215, 216,
209, 165, 227, 224, 226, 229, 153, 158, 157, 235, 233, 234, 154, 237, 232, 225,
133, 160, 131, 198, 132, 134, 145, 135, 138, 130, 136, 137, 141, 161, 140, 139,
208, 164, 149, 162, 147, 228, 148, 246, 155, 151, 163, 150, 129, 236, 231, 152
};

CONST CHAR
y85l1[] = {  /* IBM Code Page 850 to Latin-1 */
/*
  This is from IBM CDRA page 153.  It is the inverse of yl185[].
  As of edit 183, this table is no longer pure CDRA.  The translations
  involving C0 controls and DEL have been removed.
*/
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
199, 252, 233, 226, 228, 224, 229, 231, 234, 235, 232, 239, 238, 236, 196, 197,
201, 230, 198, 244, 246, 242, 251, 249, 255, 214, 220, 248, 163, 216, 215, 159,
225, 237, 243, 250, 241, 209, 170, 186, 191, 174, 172, 189, 188, 161, 171, 187,
155, 156, 157, 144, 151, 193, 194, 192, 169, 135, 128, 131, 133, 162, 165, 147,
148, 153, 152, 150, 145, 154, 227, 195, 132, 130, 137, 136, 134, 129, 138, 164,
240, 208, 202, 203, 200, 158, 205, 206, 207, 149, 146, 141, 140, 166, 204, 139,
211, 223, 212, 210, 245, 213, 181, 254, 222, 218, 219, 217, 253, 221, 175, 180,
173, 177, 143, 190, 182, 167, 247, 184, 176, 168, 183, 185, 179, 178, 142, 160
};

#ifdef COMMENT
CONST CHAR
yl1r8[] = {  /* Latin-1 to Hewlett Packard Roman8 */
/* This is HP's official translation, straight from iconv */
/* It is NOT invertible. */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
160, 184, 191, 187, 186, 188, 124, 189, 171,  99, 249, 251, 126,  45,  82, 176,
179, 254,  50,  51, 168, 243, 244, 242,  44,  49, 250, 253, 247, 248, 245, 185,
161, 224, 162, 225, 216, 208, 211, 180, 163, 220, 164, 165, 230, 229, 166, 167,
227, 182, 232, 231, 223, 233, 218, 120, 210, 173, 237, 174, 219, 177, 240, 222,
200, 196, 192, 226, 204, 212, 215, 181, 201, 197, 193, 205, 217, 213, 209, 221,
228, 183, 202, 198, 194, 234, 206,  47, 214, 203, 199, 195, 207, 178, 241, 239
};
CONST CHAR
yr8l1[] = {  /* Hewlett Packard Roman8 to Latin-1 */
/* This is HP's official translation, straight from iconv */
/* It is NOT invertible. */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
160, 192, 194, 200, 202, 203, 206, 207, 180,  96,  94, 168, 126, 217, 219, 163,
175, 221, 253, 176, 199, 231, 209, 241, 161, 191, 164, 163, 165, 167, 102, 162,
226, 234, 244, 251, 225, 233, 243, 250, 224, 232, 242, 249, 228, 235, 246, 252,
197, 238, 216, 198, 229, 237, 248, 230, 196, 236, 214, 220, 201, 239, 223, 212,
193, 195, 227, 208, 240, 205, 204, 211, 210, 213, 245,  83, 115, 218,  89, 255,
222, 254, 183, 181, 182, 190,  45, 188, 189, 170, 186, 171,  42, 187, 177, 160
};
#else /* !COMMENT */
/* This is an invertible mapping, approved by HP in January 1994. */
CONST CHAR
yl1r8[] = {  /* ISO Latin-1 to HP Roman8, Invertible */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
160, 184, 191, 187, 186, 188, 169, 189, 171, 170, 249, 251, 172, 175, 190, 176,
179, 254, 235, 236, 168, 243, 244, 242, 238, 246, 250, 253, 247, 248, 245, 185,
161, 224, 162, 225, 216, 208, 211, 180, 163, 220, 164, 165, 230, 229, 166, 167,
227, 182, 232, 231, 223, 233, 218, 252, 210, 173, 237, 174, 219, 177, 240, 222,
200, 196, 192, 226, 204, 212, 215, 181, 201, 197, 193, 205, 217, 213, 209, 221,
228, 183, 202, 198, 194, 234, 206, 255, 214, 203, 199, 195, 207, 178, 241, 239
};

CONST CHAR
yr8l1[] = { /* HP Roman8 to ISO Latin-1, Invertible */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
160, 192, 194, 200, 202, 203, 206, 207, 180, 166, 169, 168, 172, 217, 219, 173,
175, 221, 253, 176, 199, 231, 209, 241, 161, 191, 164, 163, 165, 167, 174, 162,
226, 234, 244, 251, 225, 233, 243, 250, 224, 232, 242, 249, 228, 235, 246, 252,
197, 238, 216, 198, 229, 237, 248, 230, 196, 236, 214, 220, 201, 239, 223, 212,
193, 195, 227, 208, 240, 205, 204, 211, 210, 213, 245, 178, 179, 218, 184, 255,
222, 254, 183, 181, 182, 190, 185, 188, 189, 170, 186, 171, 215, 187, 177, 247
};
#endif /* COMMENT */

CONST CHAR
yl143[] = {  /* Latin-1 to IBM Code Page 437 */
/*
  Although the IBM CDRA does not include an official translation between CP437
  and ISO Latin Alphabet 1, it does include an official, invertible
  translation between CP437 and CP850 (page 196), and another from CP850 to
  Latin-1 (CP819) (page 153).  This translation was obtained with a two-step
  process based on those tables.
  As of edit 183, the translation is modified to leave C0 controls alone.
*/
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
186, 205, 201, 187, 200, 188, 204, 185, 203, 202, 206, 223, 220, 219, 254, 242,
179, 196, 218, 191, 192, 217, 195, 180, 194, 193, 197, 176, 177, 178, 213, 159,
255, 173, 155, 156, 207, 157, 221, 245, 249, 184, 166, 174, 170, 240, 169, 238,
248, 241, 253, 252, 239, 230, 244, 250, 247, 251, 167, 175, 172, 171, 243, 168,
183, 181, 182, 199, 142, 143, 146, 128, 212, 144, 210, 211, 222, 214, 215, 216,
209, 165, 227, 224, 226, 229, 153, 158, 190, 235, 233, 234, 154, 237, 232, 225,
133, 160, 131, 198, 132, 134, 145, 135, 138, 130, 136, 137, 141, 161, 140, 139,
208, 164, 149, 162, 147, 228, 148, 246, 189, 151, 163, 150, 129, 236, 231, 152
};

CONST CHAR
y43l1[] = {  /* IBM Code Page 437 to Latin-1 */
/*
  This table is the inverse of yl143[].
*/
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
199, 252, 233, 226, 228, 224, 229, 231, 234, 235, 232, 239, 238, 236, 196, 197,
201, 230, 198, 244, 246, 242, 251, 249, 255, 214, 220, 162, 163, 165, 215, 159,
225, 237, 243, 250, 241, 209, 170, 186, 191, 174, 172, 189, 188, 161, 171, 187,
155, 156, 157, 144, 151, 193, 194, 192, 169, 135, 128, 131, 133, 248, 216, 147,
148, 153, 152, 150, 145, 154, 227, 195, 132, 130, 137, 136, 134, 129, 138, 164,
240, 208, 202, 203, 200, 158, 205, 206, 207, 149, 146, 141, 140, 166, 204, 139,
211, 223, 212, 210, 245, 213, 181, 254, 222, 218, 219, 217, 253, 221, 175, 180,
173, 177, 143, 190, 182, 167, 247, 184, 176, 168, 183, 185, 179, 178, 142, 160
};

CONST CHAR
yl1aq[] = {  /* Latin-1 to Extended Mac Latin (based on Apple QuickDraw) */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
182, 183, 184, 185, 189, 196, 197, 198, 206, 207, 210, 211, 217, 218, 195, 212,
209, 215, 213, 226, 227, 228, 240, 245, 246, 247, 249, 250, 251, 253, 254, 255,
202, 193, 162, 163, 219, 180, 201, 164, 172, 169, 187, 199, 194, 208, 168, 248,
161, 177, 170, 173, 171, 181, 166, 225, 252, 176, 188, 200, 178, 179, 186, 192,
203, 231, 229, 204, 128, 129, 174, 130, 233, 131, 230, 232, 237, 234, 235, 236,
220, 132, 241, 238, 239, 205, 133, 165, 175, 244, 242, 243, 134, 160, 222, 167,
136, 135, 137, 139, 138, 140, 190, 141, 143, 142, 144, 145, 147, 146, 148, 149,
221, 150, 152, 151, 153, 155, 154, 214, 191, 157, 156, 158, 159, 224, 223, 216
};

CONST CHAR
yl1du[] = {  /* Latin-1 to Dutch ISO 646 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34, UNK,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
UNK,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, UNK, UNK, UNK,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, UNK, UNK, UNK,  39, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
 32,  33, UNK,  35, 124, UNK, UNK,  93, 123,  67, UNK,  34, UNK,  45,  82, UNK,
 91, UNK, UNK, UNK, 126, 117, UNK, UNK,  44, UNK, UNK,  34, 125,  92,  64,  63,
 65,  65,  65,  65,  91,  65,  65,  67,  69,  69,  69,  69,  73,  73,  73,  73,
UNK,  78,  79,  79,  79,  79,  79, 120,  79,  85,  85,  85,  85,  89, UNK, 115,
 97,  97,  97,  97,  97,  97,  97,  99, 101, 101, 101, 101, 105, 105, 105, 105,
UNK, 110, 111, 111, 111, 111, 111,  47, 111, 117, 117, 117, 117, 121, UNK,  91
};

CONST CHAR
yl1fi[] = {  /* Latin-1 to Finnish ISO NRC (*not* ISO 646) */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, UNK, UNK, UNK, UNK,  95,
UNK,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, UNK, UNK, UNK, UNK, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
 32,  33, UNK, UNK, UNK, UNK, UNK, UNK,  34,  67, UNK,  34, UNK,  45,  82, UNK,
UNK, UNK, UNK, UNK,  39, 117, UNK, UNK,  44, UNK, UNK,  34, UNK, UNK, UNK,  63,
 65,  65,  65,  65,  91,  93,  65,  67,  69,  69,  69,  69,  73,  73,  73,  73,
UNK,  78,  79,  79,  79,  79,  92, 120,  79,  85,  85,  85,  94,  89, UNK, 115,
 97,  97,  97,  97, 123, 125,  97,  99, 101,  96, 101, 101, 105, 105, 105, 105,
UNK, 110, 111, 111, 111, 111, 124,  47, 111, 117, 117, 117, 126, 121, UNK, 121
};

CONST CHAR
yl1fr[] = {  /* Latin-1 to French ISO 646 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34, UNK,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
UNK,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, UNK, UNK, UNK,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, UNK, UNK, UNK, UNK, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
 32,  33, UNK,  35, UNK, UNK, UNK,  93,  34,  67, UNK,  34, UNK,  45,  82, UNK,
 91, UNK, UNK, UNK,  39, 117, UNK, UNK,  44, UNK, UNK,  34, UNK, UNK, UNK,  63,
 65,  65,  65,  65,  65,  65,  65,  67,  69,  69,  69,  69,  73,  73,  73,  73,
UNK,  78,  79,  79,  79,  79,  79, 120,  79,  85,  85,  85,  85,  89, UNK, 115,
 64,  97,  97,  97,  97,  97,  97,  92, 125, 123, 101, 101, 105, 105, 105, 105,
UNK, 110, 111, 111, 111, 111, 111,  47, 111, 124, 117, 117, 117, 121, UNK, 121
};

CONST CHAR
yl1fc[] = {  /* Latin-1 to French-Canadian ISO 646 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
UNK,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, UNK, UNK, UNK, UNK,  95,
UNK,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, UNK, UNK, UNK, UNK, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
 32,  33, UNK, UNK, UNK, UNK, UNK, UNK,  34,  67, UNK,  34, UNK,  45,  82, UNK,
UNK, UNK, UNK, UNK,  39, 117, UNK, UNK,  44, UNK, UNK,  34, UNK, UNK, UNK,  63,
 65,  65,  65,  65,  65,  65,  65,  67,  69,  69,  69,  69,  73,  73,  73,  73,
UNK,  78,  79,  79,  79,  79,  79, 120,  79,  85,  85,  85,  85,  89, UNK, 115,
 64,  97,  91,  97,  97,  97,  97,  92, 125, 123,  93, 101, 105, 105,  94, 105,
UNK, 110, 111, 111,  96, 111, 111,  47, 111, 124, 117, 126, 117, 121, UNK, 121
};

CONST CHAR
yl1ge[] = {  /* Latin-1 to German ISO 646 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
UNK,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, UNK, UNK, UNK,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, UNK, UNK, UNK, UNK, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
 32,  33, UNK, UNK, UNK, UNK, UNK,  64,  34,  67, UNK,  34, UNK,  45,  82, UNK,
UNK, UNK, UNK, UNK,  39, 117, UNK, UNK,  44, UNK, UNK,  34, UNK, UNK, UNK,  63,
 65,  65,  65,  65,  91,  65,  65,  67,  69,  69,  69,  69,  73,  73,  73,  73,
UNK,  78,  79,  79,  79,  79,  92, 120,  79,  85,  85,  85,  93,  89, UNK, 126,
 97,  97,  97,  97, 123,  97,  97,  99, 101, 101, 101, 101, 105, 105, 105, 105,
UNK, 110, 111, 111, 111, 111, 124,  47, 111, 117, 117, 117, 125, 121, UNK, 121
};

CONST CHAR
yl1hu[] = {  /* Latin-1 to Hungarian ISO-646 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
 32,  33, UNK, UNK,  36, UNK, UNK, UNK,  34,  67, UNK,  34, UNK,  45,  82, UNK,
UNK,  64, UNK, UNK,  39, 117, UNK, UNK,  44, UNK, UNK,  34, UNK, UNK, UNK,  63,
 65,  65,  65,  65,  65,  65,  65,  67,  69,  91,  69,  69,  73,  73,  73,  73,
UNK,  78,  79,  79,  79,  79,  92, 120,  79,  85,  85,  85,  93,  89, UNK, 115,
 97,  96,  97,  97,  97,  97,  97,  99, 101, 123, 101, 101, 105, 105, 105, 105,
UNK, 110, 111, 111, 111, 111, 124,  47, 111, 117, 117, 117, 125, 121, UNK, 121
};

CONST CHAR
yl1it[] = {  /* Latin-1 to Italian ISO 646 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34, UNK,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
UNK,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, UNK, UNK, UNK,  94,  95,
UNK,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, UNK, UNK, UNK, UNK, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
 32,  33, UNK,  35, UNK, UNK, UNK,  64,  34,  67, UNK,  34, UNK,  45,  82, UNK,
 91, UNK, UNK, UNK,  39, 117, UNK, UNK,  44, UNK, UNK,  34, UNK, UNK, UNK,  63,
 65,  65,  65,  65,  65,  65,  65,  67,  69,  69,  69,  69,  73,  73,  73,  73,
UNK,  78,  79,  79,  79,  79,  79, 120,  79,  85,  85,  85,  85,  89, UNK, 115,
123,  97,  97,  97,  97,  97,  97,  92, 125,  93, 101, 101, 126, 105, 105, 105,
UNK, 110, 124, 111, 111, 111, 111,  47, 111,  96, 117, 117, 117, 121, UNK, 121
};

CONST CHAR
yl1ne[] = {  /* Latin-1 to NeXT */
/* NEED TO MAKE THIS ONE INVERTIBLE, LIKE CP850 */
/*
  Which means finding all the graphic characters in the NeXT set that have
  no equivalent in Latin-1 and assigning them to the UNK positions (mostly
  Latin-1 C1 controls).  Then make the ynel1[] table be the inverse of this
  one.  But first we should try to get an official Latin-1/NeXT translation
  table from NeXT, Inc.
*/
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
 32, 161, 162, 163, 168, 165, 181, 167, 200, 160, 227, 171, 190, UNK, 176, 197,
202, 209, 201, 204, 194, 157, 182, 183, 203, 192, 235, 187, 210, 211, 212, 191,
129, 130, 131, 132, 133, 134, 225, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 158, 233, 151, 152, 153, 154, 155, 156, 251,
213, 214, 215, 216, 217, 218, 241, 219, 220, 221, 222, 223, 224, 226, 228, 229,
230, 231, 236, 237, 238, 239, 240, 159, 249, 242, 243, 244, 246, 247, 252, 253
};

CONST CHAR
yl1no[] = {  /* Latin-1 to Norwegian/Danish ISO 646 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, UNK, UNK, UNK,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, UNK, UNK, UNK, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
 32,  33, UNK, UNK, UNK, UNK, UNK, UNK,  34,  67, UNK,  34, UNK,  45,  82, UNK,
UNK, UNK, UNK, UNK,  39, 117, UNK, UNK,  44, UNK, UNK,  34, UNK, UNK, UNK,  63,
 65,  65,  65,  65,  65,  93,  91,  67,  69,  69,  69,  69,  73,  73,  73,  73,
UNK,  78,  79,  79,  79,  79,  79, 120,  92,  85,  85,  85,  85,  89, UNK, 115,
 97,  97,  97,  97,  97, 125, 123,  99, 101, 101, 101, 101, 105, 105, 105, 105,
UNK, 110, 111, 111, 111, 111, 111,  47, 124, 117, 117, 117, 117, 121, UNK, 121
};

CONST CHAR
yl1po[] = {  /* Latin-1 to Portuguese ISO 646 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, UNK, UNK, UNK,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, UNK, UNK, UNK, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
 32,  33, UNK, UNK, UNK, UNK, UNK, UNK,  34,  67, UNK,  34, UNK,  45,  82, UNK,
UNK, UNK, UNK, UNK,  39, 117, UNK, UNK,  44, UNK, UNK,  34, UNK, UNK, UNK,  63,
 65,  65,  65,  91,  65,  65,  65,  92,  69,  69,  69,  69,  73,  73,  73,  73,
UNK,  78,  79,  79,  79,  93,  79, 120,  79,  85,  85,  85,  85,  89, UNK, 115,
 97,  97,  97, 123,  97,  97,  97, 124, 101, 101, 101, 101, 105, 105, 105, 105,
UNK, 110, 111, 111, 111, 125, 111,  47, 111, 117, 117, 117, 117, 121, UNK, 121
};

CONST CHAR
yl1sp[] = {  /* Latin-1 to Spanish ISO 646 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34, UNK,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
UNK,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, UNK, UNK, UNK,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122,  96, UNK, UNK, 126, 127,
126, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
 32,  91, UNK,  35, UNK, UNK, UNK,  64,  34,  67, UNK,  34, UNK,  45,  82, UNK,
123, UNK, UNK, UNK,  39, 117, UNK, UNK,  44, UNK, UNK,  34, UNK, UNK, UNK,  93,
 65,  65,  65,  65,  65,  65,  65,  67,  69,  69,  69,  69,  73,  73,  73,  73,
UNK,  92,  79,  79,  79,  79,  79, 120,  79,  85,  85,  85,  85,  89, UNK, 115,
124,  97,  97,  97,  97,  97,  97, 125, 101, 101, 101, 101, 105, 105, 105, 105,
UNK, 124, 111, 111, 111, 111, 111,  47, 111, 117, 117, 117, 117, 121, UNK, 121
};

CONST CHAR
yl1sw[] = {  /* Latin-1 to Swedish ISO 646 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
UNK,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, UNK, UNK, UNK, UNK,  95,
UNK,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, UNK, UNK, UNK, UNK, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
 32,  33, UNK, UNK, UNK, UNK, UNK, UNK,  34,  67, UNK,  34, UNK,  45,  82, UNK,
UNK, UNK, UNK, UNK,  39, 117, UNK, UNK,  44, UNK, UNK,  34, UNK, UNK, UNK,  63,
 65,  65,  65,  65,  91,  93,  65,  67,  69,  64,  69,  69,  73,  73,  73,  73,
UNK,  78,  79,  79,  79,  79,  92, 120,  79,  85,  85,  85,  94,  89, UNK, 115,
 97,  97,  97,  97, 123, 125,  97,  99, 101,  96, 101, 101, 105, 105, 105, 105,
UNK, 110, 111, 111, 111, 111, 124,  47, 111, 117, 117, 117, 126, 121, UNK, 121
};

CONST CHAR
yl1ch[] = {  /* Latin-1 to Swiss ISO 646 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34, UNK,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
UNK,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, UNK, UNK, UNK, UNK, UNK,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, UNK, UNK, UNK, UNK, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
 32,  33, UNK, UNK, UNK, UNK, UNK, UNK,  34,  67, UNK,  34, UNK,  45,  82, UNK,
UNK, UNK, UNK, UNK,  39, 117, UNK, UNK,  44, UNK, UNK,  34, UNK, UNK, UNK,  63,
 65,  65,  65,  65,  65,  65,  65,  67,  69,  69,  69,  69,  73,  73,  73,  73,
UNK,  78,  79,  79,  79,  79,  79, 120,  79,  85,  85,  85,  85,  89, UNK, 115,
 64,  97,  97,  97, 123,  97,  97,  92,  95,  91,  93, 101, 105, 105,  94, 105,
UNK, 110, 111, 111,  96, 111, 124,  47, 111,  35, 117, 126, 125, 121, UNK, 121
};

CONST CHAR
yl1dm[] = {  /* Latin-1 to DEC Multinational Character Set */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
 32, 161, 162, 163, 168, 165, 124, 167,  34, 169, 170, 171, 126, UNK,  82, UNK,
176, 177, 178, 179,  39, 181, 182, 183,  44, 185, 186, 187, 188, 189, UNK, 191,
192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
UNK, 209, 210, 211, 212, 213, 214, 120, 216, 217, 218, 219, 220, 221, UNK, 223,
224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
UNK, 241, 242, 243, 244, 245, 246,  47, 248, 249, 250, 251, 252, UNK, UNK, 253
};

CONST CHAR
yl1dg[] = {  /* Latin-1 to Data General International Character Set */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
160, 171, 167, 168, 166, 181, 191, 187, 189, 173, 169, 177, 161, 255, 174, 175,
188, 182, 164, 165, 190, 163, 178, 185, 186, 179, 170, 176, 223, 162, 220, 172,
193, 192, 194, 196, 195, 197, 198, 199, 201, 200, 202, 203, 205, 204, 206, 207,
184, 208, 210, 209, 211, 213, 212, 215, 214, 217, 216, 218, 219, 221, 222, 252,
225, 224, 226, 228, 227, 229, 230, 231, 233, 232, 234, 235, 237, 236, 238, 239,
183, 240, 242, 241, 243, 245, 244, 247, 246, 249, 248, 250, 251, 180, 254, 253
};


/* Local file character sets to ISO Latin Alphabet 1 */

#ifdef NOTUSED
CONST CHAR
yasl1[] = {  /* ASCII to Latin-1 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127
};
#endif /* NOTUSED */

CONST CHAR
yaql1[] = {  /* Extended Mac Latin (based on Apple Quickdraw) to Latin-1 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
196, 197, 199, 201, 209, 214, 220, 225, 224, 226, 228, 227, 229, 231, 233, 232,
234, 235, 237, 236, 238, 239, 241, 243, 242, 244, 246, 245, 250, 249, 251, 252,
221, 176, 162, 163, 167, 215, 182, 223, 174, 169, 178, 180, 168, 179, 198, 216,
185, 177, 188, 189, 165, 181, 128, 129, 130, 131, 190, 170, 186, 132, 230, 248,
191, 161, 172, 142, 133, 134, 135, 171, 187, 166, 160, 192, 195, 213, 136, 137,
173, 144, 138, 139, 143, 146, 247, 145, 255, 140, 141, 164, 208, 240, 222, 254,
253, 183, 147, 148, 149, 194, 202, 193, 203, 200, 205, 206, 207, 204, 211, 212,
150, 210, 218, 219, 217, 151, 152, 153, 175, 154, 155, 156, 184, 157, 158, 159
};

CONST CHAR
ydul1[] = {  /* Dutch ISO 646 to Latin-1 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34, 163,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
190,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, 255, 189, 124,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 168, 164, 188,  39, 127
};

CONST CHAR
yfil1[] = {  /* Finnish NRC (*not* ISO-646) to Latin-1 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, 196, 214, 197, 220,  95,
233,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 228, 246, 229, 252, 127
};

CONST CHAR
yfrl1[] = {  /* French ISO 646 to Latin-1 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34, 163,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
224,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, 176, 231, 167,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 233, 249, 232, 168, 127
};

CONST CHAR
yfcl1[] = {  /* French-Canadian ISO 646 to Latin-1 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
224,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, 226, 231, 234, 238,  95,
244,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 233, 249, 232, 251, 127
};

CONST CHAR
ygel1[] = {  /* German ISO 646 to Latin-1 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
167,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, 196, 214, 220,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 228, 246, 252, 223, 127
};

CONST CHAR
yitl1[] = {  /* Italian ISO 646 to Latin-1 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34, 163,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
167,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, 176, 231, 233,  94,  95,
249,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 224, 242, 232, 236, 127
};

CONST CHAR
ynel1[] = {  /* NeXT to Latin-1 */
/* NEED TO MAKE THIS ONE INVERTIBLE */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
160, 192, 193, 194, 195, 196, 197, 199, 200, 201, 202, 203, 204, 205, 206, 207,
208, 209, 210, 211, 212, 213, 214, 217, 218, 219, 220, 221, 222, 181, 215, 247,
169, 161, 162, 163, UNK, 165, UNK, 167, 164, UNK, UNK, 171, UNK, UNK, UNK, UNK,
174, UNK, UNK, UNK, 183, 166, 182, UNK, UNK, UNK, UNK, 187, UNK, UNK, 172, 191,
185,  96, 180,  94, 126, 175, UNK, UNK, 168, 178, 176, 184, 179, UNK, UNK, UNK,
UNK, 177, 188, 189, 190, 224, 225, 226, 227, 228, 229, 231, 232, 233, 234, 235,
236, 198, 237, 170, 238, 239, 240, 241, UNK, 216, UNK, 186, 242, 243, 244, 245,
246, 230, 249, 250, 251, UNK, 252, 253, UNK, 248, UNK, 223, 254, 255, UNK, UNK
};

CONST CHAR
ynol1[] = {  /* Norwegian/Danish ISO 646 to Latin-1 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, 198, 216, 197,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 230, 248, 229, 126, 127
};

CONST CHAR
ypol1[] = {  /* Portuguese ISO 646 to Latin-1 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, 195, 199, 213,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 227, 231, 245, 126, 127
};

CONST CHAR
yspl1[] = {  /* Spanish ISO 646 to Latin-1 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34, 163,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
167,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, 161, 209, 191,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 176, 241, 231, 126, 127
};

CONST CHAR
yswl1[] = {  /* Swedish ISO 646 to Latin-1 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
201,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, 196, 214, 197, 220,  95,
233,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 228, 246, 229, 252, 127
};

CONST CHAR
ychl1[] = {  /* Swiss ISO 646 to Latin-1 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34, 249,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
224,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, 233, 231, 234, 238, 232,
244,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 228, 246, 252, 251, 127
};

CONST CHAR
yhul1[] = {  /* Hungarian ISO 646 to Latin-1 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35, 164,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
193,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, 201, 214, 220,  94,  95,
225,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 233, 246, 252,  34, 127
};

CONST CHAR
ydml1[] = {  /* DEC Multinational Character Set to Latin-1 */
/* Note: This is a null translation */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};

CONST CHAR
ydgl1[] = {  /* Data General International to Latin-1 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
160, 172, 189, 181, 178, 179, 164, 162, 163, 170, 186, 161, 191, 169, 174, 175,
187, 171, 182, 185, 253, 165, 177, 240, 208, 183, 184, 167, 176, 168, 180, 166,
193, 192, 194, 196, 195, 197, 198, 199, 201, 200, 202, 203, 205, 204, 206, 207,
209, 211, 210, 212, 214, 213, 216, 215, 218, 217, 219, 220, 190, 221, 222, 188,
225, 224, 226, 228, 227, 229, 230, 231, 233, 232, 234, 235, 237, 236, 238, 239,
241, 243, 242, 244, 246, 245, 248, 247, 250, 249, 251, 252, 223, 255, 254, 173
};


/* Translation tables for Cyrillic character sets */

#ifdef CYRILLIC
CONST CHAR
ylcac[] = {  /* Latin/Cyrillic to CP866 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19, 208, 209,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
196, 179, 192, 217, 191, 218, 195, 193, 180, 194, 197, 176, 177, 178, 211, 216,
205, 186, 200, 188, 187, 201, 204, 202, 185, 203, 206, 223, 220, 219, 254, UNK,
255, 240, 132, 131, 242,  83,  73, 244,  74, 139, 141, 151, 138,  45, 246, 135,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
252, 241, 164, 163, 243, 115, 105, 245, 106, 171, 173, 231, 170,  21, 247, 167
};

CONST CHAR
ylc55[] = {  /* Latin/Cyrillic to CP855 (inverse of y55lc) */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
174, 175, 176, 177, 178, 179, 180, 185, 186, 187, 188, 191, 192, 193, 194, 195,
196, 197, 200, 201, 202, 203, 204, 205, 206, 207, 217, 218, 219, 220, 223, 254,
255, 133, 129, 131, 135, 137, 139, 141, 143, 145, 147, 149, 151, 240, 153, 155,
161, 163, 236, 173, 167, 169, 234, 244, 184, 190, 199, 209, 211, 213, 215, 221,
226, 228, 230, 232, 171, 182, 165, 252, 246, 250, 159, 242, 238, 248, 157, 224,
160, 162, 235, 172, 166, 168, 233, 243, 183, 189, 198, 208, 210, 212, 214, 216,
225, 227, 229, 231, 170, 181, 164, 251, 245, 249, 158, 241, 237, 247, 156, 222,
239, 132, 128, 130, 134, 136, 138, 140, 142, 144, 146, 148, 150, 253, 152, 154
};

CONST CHAR
ylc1251[] = {  /* Latin/Cyrillic to CP1251 (inverse of y1251lc) */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
130, 132, 133, 134, 135, 136, 137, 139, 145, 146, 147, 148, 149, 150, 151, 152,
153, 155, 164, 165, 166, 169, 171, 172, 174, 176, 177, 180, 181, 182, 183, 187,
160, 168, 128, 129, 170, 189, 178, 175, 163, 138, 140, 142, 141, 173, 161, 143,
192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255,
185, 184, 144, 131, 186, 190, 179, 191, 188, 154, 156, 158, 157, 167, 162, 159
};

CONST CHAR
ylcbu[] = {  /* Latin/Cyrillic to Bulgarian PC Code Page */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
255, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
213, 207, 208, 209, 210, 211, 212, 215, 216, 217, 218, 219, 220, 221, 222, 223,
224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 214, 253, 254
};

CONST CHAR
ylck8[] = {  /* Latin/Cyrillic to Old KOI-8 Cyrillic */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
UNK, 229, UNK, UNK, UNK,  83,  73,  73,  74, UNK, UNK, UNK, 235, UNK, 245, UNK,
225, 226, 247, 231, 228, 229, 246, 250, 233, 234, 235, 236, 237, 238, 239, 240,
242, 243, 244, 245, 230, 232, 227, 254, 251, 253, 255, 249, 248, 252, 224, 241,
193, 194, 215, 199, 196, 197, 214, 218, 201, 202, 203, 204, 205, 206, 207, 208,
210, 211, 212, 213, 198, 200, 195, 222, 219, 221, 223, 217, 216, 220, 192, 209,
UNK, 197, UNK, UNK, UNK, 115, 105, 105, 106, UNK, UNK, UNK, 203, UNK, 213, UNK
};

CONST CHAR
yaclc[] = {  /* CP866 to Latin/Cyrillic */
/* NEED TO MAKE THIS ONE INVERTIBLE */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
161, 241, 164, 244, 167, 247, 174, 254, UNK, UNK, UNK, UNK, 240, UNK, UNK, UNK
};

CONST CHAR
y55lc[] = {  /* CP855 to Latin/Cyrillic (inverse of ylc55) */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
242, 162, 243, 163, 241, 161, 244, 164, 245, 165, 246, 166, 247, 167, 248, 168,
249, 169, 250, 170, 251, 171, 252, 172, 254, 174, 255, 175, 238, 206, 234, 202,
208, 176, 209, 177, 230, 198, 212, 180, 213, 181, 228, 196, 211, 179, 128, 129,
130, 131, 132, 133, 134, 229, 197, 216, 184, 135, 136, 137, 138, 217, 185, 139,
140, 141, 142, 143, 144, 145, 218, 186, 146, 147, 148, 149, 150, 151, 152, 153,
219, 187, 220, 188, 221, 189, 222, 190, 223, 154, 155, 156, 157, 191, 239, 158,
207, 224, 192, 225, 193, 226, 194, 227, 195, 214, 182, 210, 178, 236, 204, 240,
173, 235, 203, 215, 183, 232, 200, 237, 205, 233, 201, 231, 199, 253, 159, 160
};

CONST CHAR
y1251lc[] = {  /* CP1251 to Latin/Cyrillic (inverse of ylc1251) */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
162, 163, 128, 243, 129, 130, 131, 132, 133, 134, 169, 135, 170, 172, 171, 175,
242, 136, 137, 138, 139, 140, 141, 142, 143, 144, 249, 145, 250, 252, 251, 255,
160, 174, 254, 168, 146, 147, 148, 253, 161, 149, 164, 150, 151, 173, 152, 167,
153, 154, 166, 246, 155, 156, 157, 158, 241, 240, 244, 159, 248, 165, 245, 247,
176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239
};

CONST CHAR
ybulc[] = {  /* Bulgarian PC Code Page to Latin/Cyrillic */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 209,
210, 211, 212, 213, 214, 208, 253, 215, 216, 217, 218, 219, 220, 221, 222, 223,
224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 254, 255, 128
};

CONST CHAR
yk8lc[] = {  /* Old KOI-8 Cyrillic to Latin/Cyrillic */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
238, 208, 209, 230, 212, 213, 228, 211, 229, 216, 217, 218, 219, 220, 221, 222,
223, 239, 224, 225, 226, 227, 214, 210, 236, 235, 215, 232, 237, 233, 231, 234,
206, 176, 177, 198, 180, 181, 196, 179, 197, 184, 185, 186, 187, 188, 189, 190,
191, 207, 192, 193, 194, 195, 182, 178, 204, 203, 183, 200, 205, 201, 199, 127
};

CONST CHAR
ylcsk[] = {  /* Latin/Cyrillic to Short KOI */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94, 127,
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32, 101, UNK, UNK, UNK,  83,  73,  73,  74, UNK, UNK, UNK, 107,  45, 117, UNK,
 97,  98, 119, 103, 100, 101, 118, 122, 105, 106, 107, 108, 109, 110, 111, 112,
114, 115, 116, 117, 102, 104,  99, 126, 123, 125,  39, 121, 120, 124,  96, 113,
 97,  98, 119, 103, 100, 101, 118, 122, 105, 106, 107, 108, 109, 110, 111, 112,
114, 115, 116, 117, 102, 104,  99, 126, 123, 125,  39, 121, 120, 124,  96, 113,
UNK, 101, UNK, UNK, UNK,  83,  73,  73,  74, UNK, UNK, UNK, 107, UNK, 117, UNK
};

CONST CHAR yskcy[] = {  /* Short KOI to Latin/Cyrillic */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
206, 176, 177, 198, 180, 181, 196, 179, 197, 184, 185, 186, 187, 188, 189, 190,
191, 207, 192, 193, 194, 195, 182, 178, 204, 203, 183, 200, 205, 201, 199, 127
};
#endif /* CYRILLIC */

#ifdef LATIN2

/* Latin-2 tables */

CONST CHAR
yl252[] = {				/* Latin-2 to Code Page 852 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
174, 175, 176, 177, 178, 179, 180, 185, 186, 187, 188, 191, 192, 193, 194, 195,
196, 197, 200, 201, 202, 203, 204, 205, 206, 217, 218, 219, 220, 223, 240, 254,
255, 164, 244, 157, 207, 149, 151, 245, 249, 230, 184, 155, 141, 170, 166, 189,
248, 165, 242, 136, 239, 150, 152, 243, 247, 231, 173, 156, 171, 241, 167, 190,
232, 181, 182, 198, 142, 145, 143, 128, 172, 144, 168, 211, 183, 214, 215, 210,
209, 227, 213, 224, 226, 138, 153, 158, 252, 222, 233, 235, 154, 237, 221, 225,
234, 160, 131, 199, 132, 146, 134, 135, 159, 130, 169, 137, 216, 161, 140, 212,
208, 228, 229, 162, 147, 139, 148, 246, 253, 133, 163, 251, 129, 236, 238, 250
};

CONST CHAR
y52l2[] = {				/* Code Page 852 to Latin-2 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
199, 252, 233, 226, 228, 249, 230, 231, 179, 235, 213, 245, 238, 172, 196, 198,
201, 197, 229, 244, 246, 165, 181, 166, 182, 214, 220, 171, 187, 163, 215, 232,
225, 237, 243, 250, 161, 177, 174, 190, 202, 234, 173, 188, 200, 186, 128, 129,
130, 131, 132, 133, 134, 193, 194, 204, 170, 135, 136, 137, 138, 175, 191, 139,
140, 141, 142, 143, 144, 145, 195, 227, 146, 147, 148, 149, 150, 151, 152, 164,
240, 208, 207, 203, 239, 210, 205, 206, 236, 153, 154, 155, 156, 222, 217, 157,
211, 223, 212, 209, 241, 242, 169, 185, 192, 218, 224, 219, 253, 221, 254, 180,
158, 189, 178, 183, 162, 167, 247, 184, 176, 168, 255, 251, 216, 248, 159, 160
};

CONST CHAR
yl21250[] = {				/* Latin-2 to Code Page 1250 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 139, 144, 145, 146, 147, 148,
149, 150, 151, 152, 153, 155, 166, 169, 171, 172, 174, 177, 181, 182, 183, 187,
160, 165, 162, 163, 164, 188, 140, 167, 168, 138, 170, 141, 143, 173, 142, 175,
176, 185, 178, 179, 180, 190, 156, 161, 184, 154, 186, 157, 159, 189, 158, 191,
192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};

CONST CHAR
y1250l2[] = {				/* Code Page 1250 to Latin-2 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 169, 138, 166, 171, 174, 172,
139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 185, 149, 182, 187, 190, 188,
160, 183, 162, 163, 164, 161, 150, 167, 168, 151, 170, 152, 153, 173, 154, 175,
176, 155, 178, 179, 180, 156, 157, 158, 184, 177, 186, 159, 165, 189, 181, 191,
192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};

CONST CHAR
yl2mz[] = {			     /* Latin-2 to Mazovia (NOT invertible) */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
255, 143, UNK, 156, 155,  76, 152,  21,  34,  83,  83,  84, 160,  45,  90, 161,
248, 134,  44, 146,  39, 108, 158, UNK,  44, 115, 115, 116, 166,  34, 122, 167,
 82,  65,  65,  65, 142,  76, 149, 128,  67,  69, 144,  69,  69,  73,  73,  68,
 68, 165,  78, 163,  79, 153, 153, 250,  82,  85,  85, 154, 154,  89,  84, 225,
114,  97, 131,  97, 132, 108, 141, 135,  99, 130, 145, 137, 101, 105, 140, 101,
100, 164, 110, 162, 147, 148, 148, 246, 114, 117, 117, 129, 129, 121, 116, 249
};

CONST CHAR
ymzl2[] = {			     /* Mazovia to Latin-2 (NOT INVERTIBLE) */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
128, 252, 233, 226, 228,  97, 177, 231, 101, 235, 101, 105, 238, 230, 196, 161,
202, 234, 179, 244, 246, 198, 117, 117, 166, 214, 220, 164, 163,  89, 182, 102,
172, 175, 243, 211, 242, 210, 188, 191,  63, UNK, UNK, UNK, UNK,  33,  34,  34,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, 247, UNK, 176, 255, 215, UNK, UNK, UNK, UNK, 160
};

CONST CHAR
yl2l1[] = {				/* Latin-2 to Latin-1 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
160, 'A', UNK, 'L', 164, 'L', 'S', 167, 168, 'S', 'S', 'T', 'Z', 173, 'Z', 'Z',
176, 'a', UNK, 'l', 180, 'l', 's', UNK, 184, 's', 's', 't', 'z', UNK, 'z', 'z',
'R', 193, 194, 'A', 196, 'L', 'C', 199, 'C', 201, 'E', 203, 'E', 205, 'I', 'D',
208, 'N', 'N', 211, 212, 'O', 214, 215, 'R', 'U', 218, 'U', 220, 221, 'T', 223,
'r', 225, 226, 'a', 228, 'l', 'c', 231, 'c', 233, 'e', 235, 'e', 237, 'i', 'd',
240, 'n', 'n', 243, 244, 'o', 246, 247, 'r', 'u', 250, 'u', 252, 253, 't', '.'
};

CONST CHAR
yl1l2[] = {				/* Latin-1 to Latin-2 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
160, 'A', UNK, 'L', 164, UNK, UNK, 167, 168, 'C', 'a', '<', '>', 173, 'R', UNK,
176, UNK, UNK, UNK, 180, UNK, UNK, UNK, 184, UNK, 'o', '>', UNK, UNK, UNK, UNK,
'A', 193, 194, 'A', 196, 'A', 'A', 199, 'E', 201, 'E', 203, 'I', 205, 'I', 'I',
208, 'N', 'O', 211, 212, 'O', 214, 215, 'O', 'U', 218, 'U', 220, 221, UNK, 223,
'a', 225, 226, 'a', 228, 'a', 'a', 231, 'e', 233, 'e', 235, 'i', 237, 'i', 'i',
240, 'n', 'o', 243, 244, 'o', 246, 247, 'o', 'u', 250, 'u', 252, 253, UNK, 'y'
};

CONST CHAR
yl2as[] = {				/* Latin-2 to ASCII */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
 32, 'A', UNK, 'L', UNK, 'L', 'S', UNK,  34, 'S', 'S', 'T', 'Z', '-', 'Z', 'Z',
UNK, 'a', UNK, 'l',  39, 'l', 's', UNK,  44, 's', 's', 't', 'z', UNK, 'z', 'z',
'R', 'A', 'A', 'A', 'A', 'L', 'C', 'C', 'C', 'E', 'E', 'E', 'E', 'I', 'I', 'D',
'D', 'N', 'N', 'O', 'O', 'O', 'O', 'x', 'R', 'U', 'U', 'U', 'U', 'Y', 'T', 's',
'r', 'a', 'a', 'a', 'a', 'l', 'c', 'c', 'c', 'e', 'e', 'e', 'e', 'i', 'i', 'd',
'd', 'n', 'n', 'o', 'o', 'o', 'o', '/', 'r', 'u', 'u', 'u', 'u', 'y', 't', '.'
};
#endif /* LATIN2 */

#ifdef HEBREW
/*
  8-bit Tables providing invertible translation between Latin/Hebrew and CP862.
*/
CONST CHAR
y62lh[] = {  /* PC Code Page 862 to ISO 8859-8 Latin/Hebrew */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 162, 163, 165, 128, 129,
130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 172, 189, 188, 140, 171, 187,
141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156,
157, 158, 159, 161, 164, 166, 167, 168, 169, 170, 173, 174, 175, 223, 179, 180,
182, 184, 185, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202,
203, 204, 205, 206, 207, 208, 181, 209, 210, 211, 212, 213, 214, 215, 216, 217,
218, 177, 219, 220, 221, 222, 186, 251, 176, 183, 252, 253, 254, 178, 255, 160
};

CONST CHAR
ylh62[] = {  /* ISO 8859-8 Latin/Hebrew to PC Code Page 862 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 173, 176, 177, 178,
179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194,
255, 195, 155, 156, 196, 157, 197, 198, 199, 200, 201, 174, 170, 202, 203, 204,
248, 241, 253, 206, 207, 230, 208, 249, 209, 210, 246, 175, 172, 171, 211, 212,
213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228,
229, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 242, 243, 244, 245, 205,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 247, 250, 251, 252, 254
};
/*
  7-bit table providing readable translation from DEC Hebrew-7 to CP862.
*/
CONST CHAR
yh762[] = {
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
UNK,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, 123, 124, 125, 126, 127
};
/*
  8-bit table providing readable translation from CP862 to Hebrew-7.
*/
CONST CHAR
y62h7[] = {  /* PC Code Page 862 to Hebrew-7 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, 123, 124, 125, 126, 127,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, UNK, UNK, UNK, UNK, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK
};
/*
  7-bit table providing readable translation from Hebrew-7 to ISO Latin/Hebrew.
*/
CONST CHAR
yh7lh[] = {
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 123, 124, 125, 126, 127
};
/*
  8-bit table providing readable translation from ISO Latin/Hebrew to Hebrew-7.
*/
CONST CHAR
ylhh7[] = {  /* Latin/Hebrew to Hebrew-7 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, 123, 124, 125, 126, 127,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, UNK, UNK, UNK, UNK, UNK
};
#endif /* HEBREW */

#ifdef GREEK
/*
  8-bit Tables providing invertible translation between Latin/Greek and CP869.
*/
CONST CHAR
ylg69[] = {  /* ISO 8859-7 Latin/Greek to PC Code Page 869 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
135, 147, 148, 176, 177, 178, 179, 180, 185, 186, 187, 188, 191, 192, 193, 194,
195, 196, 197, 200, 201, 202, 203, 204, 205, 206, 217, 218, 219, 220, 223, 254,
255, 139, 140, 156, 128, 129, 138, 245, 249, 151, 130, 174, 137, 240, 131, 142,
248, 241, 153, 154, 239, 247, 134, 136, 141, 143, 144, 175, 146, 171, 149, 152,
161, 164, 165, 166, 167, 168, 169, 170, 172, 173, 181, 182, 183, 184, 189, 190,
198, 199, 132, 207, 208, 209, 210, 211, 212, 213, 145, 150, 155, 157, 158, 159,
252, 214, 215, 216, 221, 222, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233,
234, 235, 237, 236, 238, 242, 243, 244, 246, 250, 160, 251, 162, 163, 253, 133
};

CONST CHAR
y69lg[] = {  /* PC Code Page 869 to ISO 8859-7 Latin/Greek */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
164, 165, 170, 174, 210, 255, 182, 128, 183, 172, 166, 161, 162, 184, 175, 185,
186, 218, 188, 129, 130, 190, 219, 169, 191, 178, 179, 220, 163, 221, 222, 223,
250, 192, 252, 253, 193, 194, 195, 196, 197, 198, 199, 189, 200, 201, 171, 187,
131, 132, 133, 134, 135, 202, 203, 204, 205, 136, 137, 138, 139, 206, 207, 140,
141, 142, 143, 144, 145, 146, 208, 209, 147, 148, 149, 150, 151, 152, 153, 211,
212, 213, 214, 215, 216, 217, 225, 226, 227, 154, 155, 156, 157, 228, 229, 158,
230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 243, 242, 244, 180,
173, 177, 245, 246, 247, 167, 248, 181, 176, 168, 249, 251, 224, 254, 159, 160
};
/*
  7-bit table providing readable translation from ELOT 927 to CP869.
*/
CONST CHAR
yeg69[] = {
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96, 164, 165, 166, 167, 168, 169, 170, 172, 173, 181, 182, 183, 184, 189, 190,
198, 199, 207, 208, 209, 210, 211, 212, 213,  32,  32,  23, 124, 125, 126, 127
};
/*
  8-bit table providing readable translation from CP869 to ELOT 927.
*/
CONST CHAR
y69eg[] = {  /* PC Code Page 869 to ELOT 927 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, 123, 124, 125, 126, 127,
UNK, UNK, UNK, UNK, UNK, UNK,  97, UNK,  46, UNK, 124,  39,  39, 101,  45, 103,
105, 105, 111, UNK, UNK, 116, 116, UNK, 120,  50,  51,  97, UNK, 101, 103, 105,
105, 105, 111, 116,  97,  98,  99, 100, 101, 102, 103, UNK, 104, 105,  34,  34,
UNK, UNK, UNK, UNK, UNK, 106, 107, 108, 109, UNK, UNK, UNK, UNK, 110, 111, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, 112, 113, UNK, UNK, UNK, UNK, UNK, UNK, UNK, 114,
115, 116, 117, 118, 119, 120,  97,  98,  99, UNK, UNK, UNK, UNK, 100, 101, UNK,
102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 114, 115,  39,
 45, UNK, 116, 117, 118, UNK, 119, UNK, UNK, UNK, 120, 116, 116, 120, UNK,  32

};
/*
  7-bit table providing readable translation from ELOT 927 to ISO Latin/Greek.
*/
CONST CHAR
yeglg[] = {
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
208, 209, 211, 212, 213, 214, 215, 216, 217,  32,  32, 123, 124, 125, 126, 127
};
/*
  8-bit table providing readable translation from ISO Latin/Greek to ELOT 927.
*/
CONST CHAR
ylgeg[] = {  /* Latin/Greek to ELOT 927 */
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, 123, 124, 125, 126, 127,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
 32,  39,  39, UNK, UNK, UNK, 124, UNK,  34, UNK, UNK,  34, UNK,  45, UNK,  45,
UNK, UNK,  50,  51,  39, UNK,  97,  46, 101, 103, 105,  34, 111, UNK, 116, 120,
UNK,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, UNK, 114, 115, 116, 117, 118, 119, 120, 105, 116,  97, 101, 103, 105,
116,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 114, 115, 116, 117, 118, 119, 120, 105, 116, 111, 116, 120, UNK
};
#endif /* GREEK */

/* Translation functions ... */

CHAR					/* The identity function... */
#ifdef CK_ANSIC
ident(CHAR c)				/* (no longer used) */
#else
ident(c) CHAR c;
#endif /* CK_ANSIC */
{ /* ident */
    return(c);				/* Instead, enter NULL in the  */
}					/* table of functions to avoid */
					/* needless function calls.    */

CHAR
#ifdef CK_ANSIC
xleft128(CHAR c)
#else
xleft128(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xleft128 */
    return((c < 128) ? c : '?');
}

CHAR
#ifdef CK_ANSIC
xleft160(CHAR c)
#else
xleft160(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xleft160 */
    return((c < 160) ? c : '?');
}


CHAR
#ifdef CK_ANSIC
xl1as(CHAR c)
#else
xl1as(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl1as */ 				/* Latin-1 to US ASCII... */
    switch(langs[language].id) {

      case L_DUTCH:
	if (c == 255) {			/* Dutch umlaut-y */
	    zmstuff('j');		/* becomes ij */
	    return('i');
	} else return(yl1as[c]);	/* all others by the book */

      case L_GERMAN:
	switch (c) {			/* German, special rules. */
	  case 196:			/* umlaut-A -> Ae */
	    zmstuff('e');
	    return('A');
	  case 214:			/* umlaut-O -> Oe */
	    zmstuff('e');
	    return('O');
	  case 220:			/* umlaut-U -> Ue */
	    zmstuff('e');
	    return('U');
	  case 228:			/* umlaut-a -> ae */
	    zmstuff('e');
	    return('a');
	  case 246:			/* umlaut-o -> oe */
	    zmstuff('e');
	    return('o');
	  case 252:			/* umlaut-u -> ue */
	    zmstuff('e');
	    return('u');
	  case 223:			/* ess-zet -> ss */
	    zmstuff('s');
	    return('s');
	  default: return(yl1as[c]);	/* all others by the book */
	}
      case L_DANISH:
      case L_FINNISH:
      case L_NORWEGIAN:
      case L_SWEDISH:
	switch (c) {			/* Scandanavian languages. */
	  case 196:			/* umlaut-A -> Ae */
          case 198:			/* AE ligature also -> Ae */
	    zmstuff('e');
	    return('A');
	  case 214:			/* umlaut-O -> Oe */
	  case 216:			/* O-slash -> Oe */
	    zmstuff('e');
	    return('O');
	  case 220:			/* umlaut-U -> Ue */
	  /*  return('Y'); replaced by "Ue" by popular demand. */
          /*  Y for Umlaut-U is only used in German names. */
	    zmstuff('e');
	    return('U');
	  case 228:			/* umlaut-a -> ae */
          case 230:			/* ditto for ae ligature */
	    zmstuff('e');
	    return('a');
	  case 246:			/* umlaut-o -> oe */
	  case 248:			/* o-slash -> oe */
	    zmstuff('e');
	    return('o');
	  case 252:			/* umlaut-u -> ue */
	  /*  return('y'); replaced by "ue" by popular demand. */
	    zmstuff('e');
	    return('u');
	  case 197:			/* A-ring -> Aa */
	    zmstuff('a');
	    return('A');
          case 229:			/* a-ring -> aa */
	    zmstuff('a');
	    return('a');
	  default: return(yl1as[c]);	/* All others by the book */
	}
      case L_ICELANDIC:			/* Icelandic. */
	switch (c) {
	  case 198:			/* uppercase AE -> AE */
	    zmstuff('e');
	    return('A');
	  case 208:			/* uppercase Eth -> D */
	    return('D');
	  case 214:			/* uppercase O-diaeresis -> Oe */
	    zmstuff('e');
	    return('O');
	  case 222:			/* uppercase Thorn -> Th */
	    zmstuff('h');
	    return('T');
	  case 230:			/* lowercase ae -> ae */
	    zmstuff('e');
	    return('a');
	  case 240:			/* lowercase Eth -> d */
	    return('d');
	  case 246:			/* lowercase O-diaeresis -> oe */
	    zmstuff('e');
	    return('o');
	  case 254:			/* lowercase Thorn -> th */
	    zmstuff('h');
	    return('t');
	  default: return(yl1as[c]);	/* All others by the book */
	}
      default:
	return(yl1as[c]);		/* None of the above, by the table. */
    }
}

CHAR					/* CP1252 to ASCII */
#ifdef CK_ANSIC
xw1as(CHAR c)
#else
xw1as(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xw1as */
    switch(c) {				/* Microsoft name... */
      case 0x80: return('?');		/* Euro Sign */
      case 0x82: return('\047');	/* Single Low-9 Quotation Mark */
      case 0x83: return('f');		/* Latin Small Letter f with Hook */
      case 0x84: return('"');		/* Double low-9 Quotation Mark */
      case 0x85: return('-');		/* Horizontal Ellipsis */
      case 0x86: return('+');		/* Dagger */
      case 0x87: return('+');		/* Double Dagger */
      case 0x88: return('^');		/* Modifier Letter Circumflex Accent */
      case 0x89: return('?');		/* Per Mille Sign */
      case 0x8A: return('S');		/* Latin Capital Letter S with Caron */
      case 0x8B: return('<');		/* Single Left Angle Quotation Mark */
      case 0x8C: return('O');		/* Latin Capital Ligature OE */
      case 0x8E: return('Z');		/* Latin Capital Letter Z with Caron */
      case 0x91: return('\047');	/* Left Single Quotation Mark */
      case 0x92: return('\047');	/* Right Single Quotation Mark */
      case 0x93: return('"');		/* Left Double Quotation Mark */
      case 0x94: return('"');		/* Right Double Quotation Mark */
      case 0x95: return('.');		/* Bullet */
      case 0x96: return('-');		/* En Dash */
      case 0x97: return('-');		/* Em Dash */
      case 0x98: return('~');		/* Small Tilde */
      case 0x99: return('T');		/* Trade Mark Sign */
      case 0x9A: return('s');		/* Latin Small Letter s with Caron */
      case 0x9B: return('>');		/* Single Right Angle Quotation Mark */
      case 0x9C: return('o');		/* Latin Small Ligature OE */
      case 0x9E: return('z');		/* Latin Small Letter z with Caron */
      case 0x9F: return('Y');		/* Latin Capital Letter Y Diaeresis */
      default:
	if (c > 0x80 && c < 0xa0)
	  return('?');
	else
	  return(yl1as[c]);
    }
}

CHAR					/* CP1252 to Latin-1 */
#ifdef CK_ANSIC
xw1l1(CHAR c)
#else
xw1l1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xw1l1 */
    if (c == 0x95) return(0xb7);	/* Middle dot */
    return((c < 160) ? xw1as(c) : c);
}

CHAR					/* Latin-1 to German */
#ifdef CK_ANSIC
xl1ge(CHAR c)
#else
xl1ge(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl1ge */
    return(yl1ge[c]);
}

CHAR					/* German to Latin-1 */
#ifdef CK_ANSIC
xgel1(CHAR c)
#else
xgel1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xgel1 */
    if (c & 0x80)
      return(UNK);
    return(ygel1[c]);
}

CHAR
#ifdef CK_ANSIC
xgeas(CHAR c)
#else
xgeas(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xgeas */				/* German ISO 646 to ASCII */
    if (c & 0x80)
      return(UNK);
    switch (c) {
      case 91:				/* umlaut-A -> Ae */
	zmstuff('e');
	return('A');
      case 92:				/* umlaut-O -> Oe */
	zmstuff('e');
	return('O');
      case 93:				/* umlaut-U -> Ue */
	zmstuff('e');
	return('U');
      case 123:				/* umlaut-a -> ae */
	zmstuff('e');
	return('a');
      case 124:				/* umlaut-o -> oe */
	zmstuff('e');
	return('o');
      case 125:				/* umlaut-u -> ue */
	zmstuff('e');
	return('u');
      case 126:				/* ess-zet -> ss */
	zmstuff('s');
	return('s');
      default:  return(c);		/* all others stay the same */
    }
}

CHAR
#ifdef CK_ANSIC
xl1w1(CHAR c)
#else
xl1w1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl1w1 */				/* Latin-1 to CP1252 (Windows L1) */
    if (c > 127 && c < 160)
      return(UNK);
    else
      return(c);
}

CHAR
#ifdef CK_ANSIC
xduas(CHAR c)
#else
xduas(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xduas */				/* Dutch ISO 646 to US ASCII */
    if (c & 0x80)
      return(UNK);
    switch (c) {
      case 64:  return(UNK);		/* 3/4 */
      case 91:				/* y-diaeresis */
	zmstuff('j');
	return('i');
      case 92:  return(UNK);		/* 1/2 */
      case 93:  return(124);		/* vertical bar */
      case 123: return(34);		/* diaeresis */
      case 124: return(UNK);		/* Florin */
      case 125: return(UNK);		/* 1/4 */
      case 126: return(39);		/* Apostrophe */
      default:  return(c);
    }
}

CHAR
#ifdef CK_ANSIC
xfias(CHAR c)
#else
xfias(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xfias */				/* Finnish ISO 646 to US ASCII */
    if (c & 0x80)
      return(UNK);
    switch (c) {
      case 91:				/* A-diaeresis */
	zmstuff('e');
	return('A');
      case 92:				/* O-diaeresis */
	zmstuff('e');
	return('O');
      case 93:				/* A-ring */
	zmstuff('a');
	return('A');
      case 94:				/* U-diaeresis */
	/* return('Y'); */
	zmstuff('e');
	return('U');
      case 96:				/* e-acute */
	return('e');
      case 123:				/* a-diaeresis */
	zmstuff('e');
	return('a');
      case 124:				/* o-diaeresis */
	zmstuff('e');
	return('o');
      case 125:				/* a-ring */
	zmstuff('a');
	return('a');
      case 126:				/* u-diaeresis */
	/* return('y'); */
	zmstuff('e');
	return('U');
      default:
	return(c);
    }
}

CHAR
#ifdef CK_ANSIC
xfras(CHAR c)
#else
xfras(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xfras */				/* French ISO 646 to US ASCII */
    if (c & 0x80)
      return(UNK);
    switch (c) {
      case 64:  return(97);		/* a grave */
      case 91:  return(UNK);		/* degree sign */
      case 92:  return(99);		/* c cedilla */
      case 93:  return(UNK);		/* paragraph sign */
      case 123: return(101);		/* e acute */
      case 124: return(117);		/* u grave */
      case 125: return(101);		/* e grave */
      case 126: return(34);		/* diaeresis */
      default:  return(c);
    }
}

CHAR
#ifdef CK_ANSIC
xfcas(CHAR c)
#else
xfcas(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xfcas */				/* French Canadian ISO 646 to ASCII */
    if (c & 0x80)
      return(UNK);
    switch (c) {
      case 64:  return('a');		/* a grave */
      case 91:  return('a');		/* a circumflex */
      case 92:  return('c');		/* c cedilla */
      case 93:  return('e');		/* e circumflex */
      case 94:  return('i');		/* i circumflex */
      case 96:  return('o');		/* o circumflex */
      case 123: return('e');		/* e acute */
      case 124: return('u');		/* u grave */
      case 125: return('e');		/* e grave */
      case 126: return('u');		/* u circumflex */
      default:  return(c);
    }
}

CHAR
#ifdef CK_ANSIC
xitas(CHAR c)
#else
xitas(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xitas */				/* Italian ISO 646 to ASCII */
    if (c & 0x80)
      return(UNK);
    switch (c) {
      case 91:  return(UNK);		/* degree */
      case 92:  return('c');		/* c cedilla */
      case 93:  return('e');		/* e acute */
      case 96:  return('u');		/* u grave */
      case 123: return('a');		/* a grave */
      case 124: return('o');		/* o grave */
      case 125: return('e');		/* e grave */
      case 126: return('i');		/* i grave */
      default:  return(c);
    }
}

CHAR
#ifdef CK_ANSIC
xneas(CHAR c)
#else
xneas(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xneas */				/* NeXT to ASCII */
    if (langs[language].id == L_FRENCH) { /* If SET LANGUAGE FRENCH */
	if (c == 234) {			/* handle OE digraph. */
	    zmstuff('E');
	    return('O');
	} else if (c == 250) {		/* Also lowercase oe. */
	    zmstuff('e');
	    return('o');
	}
    }
    c = xnel1(c);			/* Convert to Latin-1 */
    return(yl1as[c]);			/* Convert Latin-1 to ASCII */
}

CHAR
#ifdef CK_ANSIC
xnoas(CHAR c)
#else
xnoas(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xnoas */				/* Norge/Danish ISO 646 to ASCII */
    if (c & 0x80)
      return(UNK);
    switch (c) {
      case 91:
	zmstuff('E');			/* AE digraph */
	return('A');
      case 92: return('O');		/* O slash */
      case 93:				/* A ring */
	zmstuff('a');
	return('A');
      case 123:				/* ae digraph */
	zmstuff('e');
	return('a');
      case 124: return('o');		/* o slash */
      case 125:				/* a ring */
	zmstuff('a');
	return('a');
      default:  return(c);
    }
}

CHAR
#ifdef CK_ANSIC
xpoas(CHAR c)
#else
xpoas(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xpoas */				/* Portuguese ISO 646 to ASCII */
    if (c & 0x80)
      return(UNK);
    switch (c) {
      case 91:  return('A');		/* A tilde */
      case 92:  return('C');		/* C cedilla */
      case 93:  return('O');		/* O tilde */
      case 123: return('a');		/* a tilde */
      case 124: return('c');		/* c cedilla */
      case 125: return('o');		/* o tilde */
      default:  return(c);
    }
}

CHAR
#ifdef CK_ANSIC
xspas(CHAR c)
#else
xspas(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xspas */				/* Spanish ISO 646 to ASCII */
    if (c & 0x80)
      return(UNK);
    switch (c) {
      case 91:  return(33);		/* Inverted exclamation */
      case 92:  return('N');		/* N tilde */
      case 93:  return(63);		/* Inverted question mark */
      case 123: return(UNK);		/* degree */
      case 124: return('n');		/* n tilde */
      case 125: return('c');		/* c cedilla */
      default:  return(c);
    }
}

CHAR
#ifdef CK_ANSIC
xswas(CHAR c)
#else
xswas(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xswas */				/* Swedish ISO 646 to ASCII */
    if (c & 0x80)
      return(UNK);
    switch (c) {
      case 64:  return('E');		/* E acute */
      case 91:				/* A diaeresis */
	zmstuff('e');
	return('A');
      case 92:				/* O diaeresis */
	zmstuff('e');
	return('O');
      case 93:				/* A ring */
	zmstuff('a');
	return('A');
      case 94:				/* U diaeresis */
	/* return('Y'); */
	zmstuff('e');
	return('U');
      case 96:  return('e');		/* e acute */
      case 123:				/* a diaeresis */
	zmstuff('e');
	return('a');
      case 124:				/* o diaeresis */
	zmstuff('e');
	return('o');
      case 125:				/* a ring */
	zmstuff('a');
	return('a');
      case 126:				/* u diaeresis */
	/* return('y'); */
	zmstuff('e');
	return('u');
      default:  return(c);
    }
}

CHAR
#ifdef CK_ANSIC
xchas(CHAR c)
#else
xchas(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xchas */				/* Swiss ISO 646 to ASCII */
    if (c & 0x80)
      return(UNK);
    switch (c) {
      case 35:  return('u');		/* u grave */
      case 64:  return('a');		/* a grave */
      case 91:  return('e');		/* e acute */
      case 92:  return('c');		/* c cedilla */
      case 93:  return('e');		/* e circumflex */
      case 94:  return('i');		/* i circumflex */
      case 95:  return('e');		/* e grave */
      case 96:  return('o');		/* o circumflex */
      case 123:				/* a diaeresis */
	zmstuff('e');
	return('a');
      case 124:				/* o diaeresis */
	zmstuff('e');
	return('o');
      case 125:				/* u diaeresis */
	zmstuff('e');
	return('u');
      case 126: return('u');		/* u circumflex */
      default:  return(c);
    }
}

CHAR
#ifdef CK_ANSIC
xhuas(CHAR c)
#else
xhuas(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xhuas */				/* Hungarian ISO 646 to ASCII */
    if (c & 0x80)
      return(UNK);
    switch (c) {
      case 64:  return('A');		/* A acute */
      case 91:  return('E');		/* E acute */
      case 92:  return('O');		/* O diaeresis */
      case 93:  return('U');		/* U diaeresis */
      case 96:  return('a');		/* a acute */
      case 123: return('e');		/* e acute */
      case 124: return('o');		/* o acute */
      case 125: return('u');		/* u acute */
      case 126: return(34);		/* double acute accent */
      default:  return(c);
    }
}

CHAR
#ifdef CK_ANSIC
xdmas(CHAR c)
#else
xdmas(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xdmas */				/* DEC MCS to ASCII */
    if (langs[language].id == L_FRENCH) { /* If SET LANGUAGE FRENCH */
	if (c == 215) {			/* handle OE digraph. */
	    zmstuff('E');
	    return('O');
	} else if (c == 247) {		/* Also lowercase oe. */
	    zmstuff('e');
	    return('o');
	}
    }
    return(yl1as[c]);			/* Otherwise treat like Latin-1 */
}

CHAR
#ifdef CK_ANSIC
xdgas(CHAR c)
#else
xdgas(c) CHAR c;
#endif /* CK_ANSIC */
{ /*  xdgas */				/* Data General to ASCII */
    switch(c) {
      case 180: return('f');		/* Florin */
      case 183: return('<');		/* Less-equal */
      case 184: return('>');		/* Greater-equal */
      case 186: return(96);		/* Grave accent */
      case 191: return('^');		/* Uparrow */
      case 215:
	if (langs[language].id == L_FRENCH) { /* OE digraph */
	    zmstuff('E');
	    return('O');
	} else return('O');
      case 247:
	if (langs[language].id == L_FRENCH) { /* oe digraph */
	    zmstuff('e');
	    return('o');
	} else return('o');
      case 175: case 179: case 220: case 222:
      case 223: case 254: case 255:
	return(UNK);
      default:				/* The rest, convert to Latin-1 */
	return(yl1as[ydgl1[c]]);	/* and from there to ASCII */
    }
}

CHAR
#ifdef CK_ANSIC
xr8as(CHAR c)
#else
xr8as(c) CHAR c;
#endif /* CK_ANSIC */
{ /*  xr8as */				/* Hewlett Packard Roman8 to ASCII */
    switch(c) {
      case 175: return('L');		/* Lira */
      case 190: return('f');		/* Florin */
      case 235: return('S');		/* S caron */
      case 236: return('s');		/* s caron */
      case 246: return('-');		/* Horizontal bar */
      case 252: return('*');		/* Solid box */
      default:				/* The rest, convert to Latin-1 */
	return(yl1as[yr8l1[c]]);	/* and from there to ASCII */
    }
}

CHAR
#ifdef CK_ANSIC
xukl1(CHAR c)
#else
xukl1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xukl1 */				/* UK ASCII to Latin-1 */
    if (c & 0x80)
      return(UNK);
    if (c == 35)
      return(163);
    else return(c);
}

CHAR
#ifdef CK_ANSIC
xl1uk(CHAR c)
#else
xl1uk(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl1uk */				/* Latin-1 to UK ASCII */
    if (c == 163)
      return(35);
    else return(yl1as[c]);
}

CHAR					/* Latin-1 to French ISO 646 */
#ifdef CK_ANSIC
xl1fr(CHAR c)
#else
xl1fr(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl1fr */
    return(yl1fr[c]);
}


CHAR					/* French ISO 646 to Latin-1 */
#ifdef CK_ANSIC
xfrl1(CHAR c)
#else
xfrl1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xfrl1 */
    if (c & 0x80)
      return(UNK);
    return(yfrl1[c]);
}

CHAR					/* Latin-1 to Dutch ASCII */
#ifdef CK_ANSIC
xl1du(CHAR c)
#else
xl1du(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl1du */
    return(yl1du[c]);
}

CHAR
#ifdef CK_ANSIC
xdul1(CHAR c)
#else
xdul1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xdul1 */				/* Dutch ISO 646 to Latin-1 */
    if (c & 0x80)
      return(UNK);
    return(ydul1[c]);
}

CHAR
#ifdef CK_ANSIC
xfil1(CHAR c)
#else
xfil1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xfil1 */				/* Finnish ISO 646 to Latin-1 */
    if (c & 0x80)
      return(UNK);
    return(yfil1[c]);
}

CHAR
#ifdef CK_ANSIC
xl1fi(CHAR c)
#else
xl1fi(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl1fi */				/* Latin-1 to Finnish ISO 646 */
    return(yl1fi[c]);
}

CHAR
#ifdef CK_ANSIC
xfcl1(CHAR c)
#else
xfcl1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xfcl1 */				/* French Canadian ISO646 to Latin-1 */
    if (c & 0x80)
      return(UNK);
    return(yfcl1[c]);
}

CHAR
#ifdef CK_ANSIC
xl1fc(CHAR c)
#else
xl1fc(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl1fc */				/* Latin-1 to French Canadian ISO646 */
    return(yl1fc[c]);
}

CHAR
#ifdef CK_ANSIC
xitl1(CHAR c)
#else
xitl1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xitl1 */				/* Italian ISO 646 to Latin-1 */
    if (c & 0x80)
      return(UNK);
    return(yitl1[c]);
}

CHAR
#ifdef CK_ANSIC
xl1it(CHAR c)
#else
xl1it(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl1it */				/* Latin-1 to Italian ISO 646 */
    return(yl1it[c]);
}

CHAR
#ifdef CK_ANSIC
xnel1(CHAR c)
#else
xnel1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xnel1 */		 		/* NeXT to Latin-1 */
    if (langs[language].id == L_FRENCH) { /* If SET LANGUAGE FRENCH */
	if (c == 234) {			/* handle OE digraph. */
	    zmstuff('E');
	    return('O');
	} else if (c == 250) {		/* Also lowercase oe. */
	    zmstuff('e');
	    return('o');
	}
    }
    return(ynel1[c]);
}

CHAR
#ifdef CK_ANSIC
xnel9(CHAR c)
#else
xnel9(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xnel9 */		 		/* NeXT to Latin-9 */
    switch (c) {
      case 234: return(188);		/* OE */
      case 250: return(189);		/* oe */
      case 188: return(234);		/* keep it invertible... */
      case 189: return(250);		/* oe */
      default:
	return(ynel1[c]);
    }
}

CHAR
#ifdef CK_ANSIC
xl1ne(CHAR c)
#else
xl1ne(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl1ne */		 		/* Latin-1 to NeXT */
    return(yl1ne[c]);
}

CHAR
#ifdef CK_ANSIC
xl9ne(CHAR c)
#else
xl9ne(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl9ne */		 		/* Latin-9 to NeXT */
    switch (c) {
      case 188: return(234);		/* OE */
      case 189: return(250);		/* oe */
      case 234: return(188);		/* OE */
      case 250: return(189);		/* oe */
      default:
	return(yl1ne[c]);
    }
}

CHAR
#ifdef CK_ANSIC
xnol1(CHAR c)
#else
xnol1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xnol1 */		 		/* Norway/Denmark ISO 646 to Latin-1 */
    if (c & 0x80)
      return(UNK);
    return(ynol1[c]);
}

CHAR
#ifdef CK_ANSIC
xl1no(CHAR c)
#else
xl1no(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl1no */		 		/* Latin-1 to Norway/Denmark ISO 646 */
    return(yl1no[c]);
}

CHAR
#ifdef CK_ANSIC
xpol1(CHAR c)
#else
xpol1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xpol1 */				/* Portuguese ISO 646 to Latin-1 */
    if (c & 0x80)
      return(UNK);
    return(ypol1[c]);
}

CHAR
#ifdef CK_ANSIC
xl1po(CHAR c)
#else
xl1po(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl1po */				/* Latin-1 to Portuguese ISO 646 */
    return(yl1po[c]);
}

CHAR
#ifdef CK_ANSIC
xspl1(CHAR c)
#else
xspl1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xspl1 */				/* Spanish ISO 646 to Latin-1 */
    if (c & 0x80)
      return(UNK);
    return(yspl1[c]);
}

CHAR
#ifdef CK_ANSIC
xl1sp(CHAR c)
#else
xl1sp(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl1sp */				/* Latin-1 to Spanish ISO 646 */
    return(yl1sp[c]);
}

CHAR
#ifdef CK_ANSIC
xswl1(CHAR c)
#else
xswl1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xswl1 */				/* Swedish ISO 646 to Latin-1 */
    if (c & 0x80)
      return(UNK);
    return(yswl1[c]);
}

CHAR
#ifdef CK_ANSIC
xl1sw(CHAR c)
#else
xl1sw(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl1sw */				/* Latin-1 to Swedish ISO 646 */
    return(yl1sw[c]);
}

CHAR
#ifdef CK_ANSIC
xchl1(CHAR c)
#else
xchl1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xchl1 */				/* Swiss ISO 646 to Latin-1 */
    if (c & 0x80)
      return(UNK);
    return(ychl1[c]);
}

CHAR
#ifdef CK_ANSIC
xl1ch(CHAR c)
#else
xl1ch(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl1ch */				/* Latin-1 to Swiss ISO 646 */
    return(yl1ch[c]);
}

CHAR
#ifdef CK_ANSIC
xhul1(CHAR c)
#else
xhul1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xhul1 */				/* Hungarian ISO 646 to Latin-1 */
    if (c & 0x80)
      return(UNK);
    return(yhul1[c]);
}

CHAR
#ifdef CK_ANSIC
xl1hu(CHAR c)
#else
xl1hu(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl1hu */				/* Latin-1 to Hungarian ISO 646 */
    return(yl1hu[c]);
}

CHAR
#ifdef CK_ANSIC
xl1dm(CHAR c)
#else
xl1dm(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl1dm */				/* Latin-1 to DEC MCS */
    return(yl1dm[c]);
}

CHAR
#ifdef CK_ANSIC
xl9dm(CHAR c)
#else
xl9dm(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl9dm */				/* Latin-9 to DEC MCS */
    switch (c) {
      case 188: return(215);
      case 189: return(247);
      case 215: return(188);
      case 247: return(189);
      default:
	return(yl1dm[c]);
    }
}

CHAR
#ifdef CK_ANSIC
xl9w1(CHAR c)
#else
xl9w1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl9w1 */				/* Latin-9 to CP1252 */
    if (c < 128)
      return(c);
    else if (c < 160)
      return('?');
    switch (c) {
      case 0xa4: return(0x80);		/* Euro */
      case 0xa6: return(0x8a);		/* S-caron */
      case 0xa8: return(0x9a);		/* s-caron */
      case 0xb4: return(0x8e);		/* Z-caron */
      case 0xb8: return(0x9e);		/* z-caron */
      case 0xbc: return(0x8c);		/* OE */
      case 0xbd: return(0x9c);		/* oe */
      case 0xbe: return(0x9f);		/* Y-diaeresis */
      default:
	return(c);
    }
}

CHAR
#ifdef CK_ANSIC
xl1dg(CHAR c)
#else
xl1dg(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl1dg */				/* Latin-1 to DG ICS */
    return(yl1dg[c]);
}

CHAR
#ifdef CK_ANSIC
xdml1(CHAR c)
#else
xdml1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xdml1 */				/* DEC MCS to Latin-1 */
    if (langs[language].id == L_FRENCH) { /* If SET LANGUAGE FRENCH */
	if (c == 215) {			/* handle OE digraph. */
	    zmstuff('E');
	    return('O');
	} else if (c == 247) {		/* Also lowercase oe. */
	    zmstuff('e');
	    return('o');
	}
    }
    return(ydml1[c]);
}

CHAR
#ifdef CK_ANSIC
xdml9(CHAR c)
#else
xdml9(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xdml9 */				/* DEC MCS to Latin-9 */
    switch (c) {
      case 215: return(188);		/* OE */
      case 247: return(189);		/* oe */
      case 188: return(215);		/* and swap the other two... */
      case 189: return(247);		/* (1/4 and 1/2) */
      default:				/* to keep it invertible */
	return(ydml1[c]);
    }
}

CHAR
#ifdef CK_ANSIC
xdgl1(CHAR c)
#else
xdgl1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xdgl1 */				/* DG International CS to Latin-1 */
    if (langs[language].id == L_FRENCH) { /* If SET LANGUAGE FRENCH */
	if (c == 215) {			/* handle OE digraph. */
	    zmstuff('E');
	    return('O');
	} else if (c == 247) {		/* Also lowercase oe. */
	    zmstuff('e');
	    return('o');
	}
    }
    return(ydgl1[c]);
}

CHAR
#ifdef CK_ANSIC
xr8l1(CHAR c)
#else
xr8l1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xr8l1 */				/* Hewlett Packard Roman8 to Latin-1 */
    return(yr8l1[c]);
}

CHAR
#ifdef CK_ANSIC
xl1r8(CHAR c)
#else
xl1r8(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl1r8 */				/* Latin-1 to Hewlett Packard Roman8 */
    return(yl1r8[c]);
}

/* Translation functions for receiving files and translating them into ASCII */

CHAR
#ifdef CK_ANSIC
zl1as(CHAR c)
#else
zl1as(c) CHAR c;
#endif /* CK_ANSIC */
{ /* zl1as */
    switch(langs[language].id) {

      case L_DUTCH:
	if (c == 255) {			/* Dutch umlaut-y */
	    zdstuff('j');		/* becomes ij */
	    return('i');
	} else return(yl1as[c]);	/* all others by the book */

      case L_GERMAN:
	switch (c) {			/* German, special rules. */
	  case 196:			/* umlaut-A -> Ae */
	    zdstuff('e');
	    return('A');
	  case 214:			/* umlaut-O -> Oe */
	    zdstuff('e');
	    return('O');
	  case 220:			/* umlaut-U -> Ue */
	    zdstuff('e');
	    return('U');
	  case 228:			/* umlaut-a -> ae */
	    zdstuff('e');
	    return('a');
	  case 246:			/* umlaut-o -> oe */
	    zdstuff('e');
	    return('o');
	  case 252:			/* umlaut-u -> ue */
	    zdstuff('e');
	    return('u');
	  case 223:			/* ess-zet -> ss */
	    zdstuff('s');
	    return('s');
	  default: return(yl1as[c]);	/* all others by the book */
	}
      case L_DANISH:
      case L_FINNISH:
      case L_NORWEGIAN:
      case L_SWEDISH:
	switch (c) {			/* Scandanavian languages. */
	  case 196:			/* umlaut-A -> Ae */
	    zdstuff('e');
	    return('A');
	  case 214:			/* umlaut-O -> Oe */
	  case 216:			/* O-slash -> Oe */
	    zdstuff('e');
	    return('O');
	  case 220:			/* umlaut-U -> Y */
	    /* return('Y'); */
	    zdstuff('e');
	    return('U');
	  case 228:			/* umlaut-a -> ae */
	    zdstuff('e');
	    return('a');
	  case 246:			/* umlaut-o -> oe */
	  case 248:			/* o-slash -> oe */
	    zdstuff('e');
	    return('o');
	  case 252:			/* umlaut-u -> y */
	    /* return('y'); */
	    zdstuff('e');
	    return('u');
	  case 197:			/* A-ring -> Aa */
	    zdstuff('a');
	    return('A');
          case 229:			/* a-ring -> aa */
	    zdstuff('a');
	    return('a');
	  default: return(yl1as[c]);	/* All others by the book */
	}
      default:
	return(yl1as[c]);		/* No language, go by the table. */
    }
}

CHAR					/* IBM CP437 to Latin-1 */
#ifdef CK_ANSIC
x43l1(CHAR c)
#else
x43l1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* x43l1 */
    return(y43l1[c]);
}

CHAR					/* IBM CP850 to Latin-1 */
#ifdef CK_ANSIC
x85l1(CHAR c)
#else
x85l1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* x85l1 */
    return(y85l1[c]);
}

CHAR					/* Latin-1 to IBM CP437 */
#ifdef CK_ANSIC
xl143(CHAR c)
#else
xl143(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl143 */
    return(yl143[c]);
}

CHAR					/* Latin-1 to CP850 */
#ifdef CK_ANSIC
xl185(CHAR c)
#else
xl185(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl185 */
    return(yl185[c]);
}

CHAR
#ifdef CK_ANSIC
x43as(CHAR c)
#else
x43as(c) CHAR c;
#endif /* CK_ANSIC */
{ /* x43as */				/* CP437 to ASCII */
    c = y43l1[c];			/* Translate to Latin-1 */
    return(xl143(c));			/* and from Latin-1 to ASCII. */
}

CHAR
#ifdef CK_ANSIC
x85as(CHAR c)
#else
x85as(c) CHAR c;
#endif /* CK_ANSIC */
{ /* x85as */				/* CP850 to ASCII */
    c = y85l1[c];			/* Translate to Latin-1 */
    return(xl1as(c));			/* and from Latin-1 to ASCII. */
}

CHAR					/* Macintosh Latin to Latin-1 */
#ifdef CK_ANSIC
xaql1(CHAR c)
#else
xaql1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xaql1 */
    if (langs[language].id == L_FRENCH) { /* If SET LANGUAGE FRENCH */
	if (c == 206) {			/* handle OE digraph. */
	    zmstuff('E');
	    return('O');
	} else if (c == 207) {		/* Also lowercase oe. */
	    zmstuff('e');
	    return('o');
	}
    }
    return(yaql1[c]);
}

CHAR					/* Macintosh Latin to ASCII */
#ifdef CK_ANSIC
xaqas(CHAR c)
#else
xaqas(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xaqas */
    if (langs[language].id == L_FRENCH) { /* If SET LANGUAGE FRENCH */
	if (c == 206) {			/* handle OE digraph. */
	    zmstuff('E');
	    return('O');
	} else if (c == 207) {		/* Also lowercase oe. */
	    zmstuff('e');
	    return('o');
	}
    }
    c = yaql1[c];			/* Translate to Latin-1 */
    return(xl1as(c));			/* then to ASCII. */
}

CHAR					/* Latin-1 to Macintosh Latin */
#ifdef CK_ANSIC
xl1aq(CHAR c)
#else
xl1aq(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl1aq */
    return(yl1aq[c]);
}

#ifdef LATIN2

/* Translation functions for Latin Alphabet 2 */

CHAR					/* Latin-2 to Latin-1 */
#ifdef CK_ANSIC
xl2l1(CHAR c)
#else
xl2l1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xll2l1 */
    return(yl2l1[c]);
}

CHAR
#ifdef CK_ANSIC
xl2w1(CHAR c)
#else
xl2w1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl2w1 */				/* Latin-2 to CP1252 (Windows L1) */
    if (c > 127 && c < 160)
      return(UNK);
    else
      return(yl2l1[c]);
}

CHAR					/* Latin-1 to Latin-2 */
#ifdef CK_ANSIC
xl1l2(CHAR c)
#else
xl1l2(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xll1l2 */
    return(yl1l2[c]);
}

CHAR					/* CP1252 to Latin-1 */
#ifdef CK_ANSIC
xw1l2(CHAR c)
#else
xw1l2(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xw1l2 */
    switch (c) {
      case 0x8a: return(0xa9);		/* S caron */
      case 0x8e: return(0xae);		/* Z caron */
      case 0x9a: return(0xb9);		/* s caron */
      case 0x9e: return(0xbe);		/* z caron */
      default:
	return((c < 160) ? xw1as(c) : xl1l2(c));
    }
}


CHAR					/* Latin-2 to ASCII */
#ifdef CK_ANSIC
xl2as(CHAR c)
#else
xl2as(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xll2as */
    return(yl2as[c]);
}

CHAR					/* Latin-2 to CP852 */
#ifdef CK_ANSIC
xl252(CHAR c)
#else
xl252(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xll252 */
    return(yl252[c]);
}

CHAR					/* Latin-2 to Mazovia */
#ifdef CK_ANSIC
xl2mz(CHAR c)
#else
xl2mz(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xll2mz */
    return(yl2mz[c]);
}

CHAR					/* Latin-1 to Mazovia */
#ifdef CK_ANSIC
xl1mz(CHAR c)
#else
xl1mz(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xll1mz */
    return(yl2mz[yl1l2[c]]);
}

CHAR					/* Mazovia to Latin-1 */
#ifdef CK_ANSIC
xmzl1(CHAR c)
#else
xmzl1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xmzl1 */
    return(yl2l1[ymzl2[c]]);
}

CHAR					/* Mazovia to Latin-9 */
#ifdef CK_ANSIC
xmzl9(CHAR c)
#else
xmzl9(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xmzl9 */
    return(xl2l9(ymzl2[c]));
}

CHAR					/* CP852 to Latin-2 */
#ifdef CK_ANSIC
x52l2(CHAR c)
#else
x52l2(c) CHAR c;
#endif /* CK_ANSIC */
{ /* x52l2 */
    return(y52l2[c]);
}

CHAR					/* Mazovia to Latin-2 */
#ifdef CK_ANSIC
xmzl2(CHAR c)
#else
xmzl2(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xmzl2 */
    return(ymzl2[c]);
}

CHAR					/* Latin-2 to CP1250 */
#ifdef CK_ANSIC
xl21250(CHAR c)
#else
xl21250(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xll21250 */
    return(yl21250[c]);
}

CHAR					/* CP1250 to Latin-2 */
#ifdef CK_ANSIC
x1250l2(CHAR c)
#else
x1250l2(c) CHAR c;
#endif /* CK_ANSIC */
{ /* x1250l2 */
    return(y1250l2[c]);
}

CHAR					/* CP852 to ASCII */
#ifdef CK_ANSIC
x52as(CHAR c)
#else
x52as(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl52as */
    return(yl2as[y52l2[c]]);		/* CP852 -> Latin-2 -> ASCII */
}

CHAR					/* CP1250 to ASCII */
#ifdef CK_ANSIC
x1250as(CHAR c)
#else
x1250as(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl1250as */
    return(yl2as[y1250l2[c]]);		/* CP81250 -> Latin-2 -> ASCII */
}


CHAR					/* CP852 to Latin-1 */
#ifdef CK_ANSIC
x52l1(CHAR c)
#else
x52l1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl52l1 */
    return(yl2l1[y52l2[c]]);		/* CP852 -> Latin-2 -> Latin-1 */
}

CHAR					/* CP1250 to Latin-1 */
#ifdef CK_ANSIC
x1250l1(CHAR c)
#else
x1250l1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl1250l1 */
    return(yl2l1[y1250l2[c]]);		/* CP1250 -> Latin-2 -> Latin-1 */
}

CHAR					/* CP1250 to Latin-9 */
#ifdef CK_ANSIC
x1250l9(CHAR c)
#else
x1250l9(c) CHAR c;
#endif /* CK_ANSIC */
{ /* x1250l9 */
    if (c == (CHAR)128)			/* Euro */
      return((CHAR)164);
    else
      return(xl2l9(y1250l2[c]));	/* CP1250 -> Latin-2 -> Latin-9 */
}

CHAR					/* Latin-1 to CP852 */
#ifdef CK_ANSIC
xl152(CHAR c)
#else
xl152(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xll152 */
    return(yl252[yl1l2[c]]);		/* Latin-1 -> Latin-2 -> CP852 */
}

CHAR					/* Latin-1 to CP1250 */
#ifdef CK_ANSIC
xl11250(CHAR c)
#else
xl11250(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xll11250 */
    return(yl21250[yl1l2[c]]);		/* Latin-1 -> Latin-2 -> CP1250 */
}

CHAR					/* Latin-9 to CP1250 */
#ifdef CK_ANSIC
xl91250(CHAR c)
#else
xl91250(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xll91250 */
    if (c == (CHAR)164)			/* Euro */
      return((CHAR)128);
    else
      return(yl21250[xl9l2(c)]);	/* Latin-9 -> Latin-2 -> CP1250 */
}

CHAR					/* Latin-9 to Mazovia */
#ifdef CK_ANSIC
xl9mz(CHAR c)
#else
xl9mz(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xll9mz */
    return(yl2mz[xl9l2(c)]);		/* Latin-9 -> Latin-2 -> Mazovia */
}

CHAR					/* Latin-9 to Mazovia */
#ifdef CK_ANSIC
xmzas(CHAR c)
#else
xmzas(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xmzas */
    return(yl2as[xmzl2(c)]);		/* Mazovia -> Latin-2 -> ASCII */
}

CHAR					/* Latin-2 to NeXT */
#ifdef CK_ANSIC
xl2ne(CHAR c)
#else
xl2ne(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xll2ne */
    switch(c) {
      case 162: return(198);		/* Breve */
      case 163: return(232);		/* L with stroke */
      case 178: return(206);		/* Ogonek */
      case 179: return(248);		/* l with stroke */
      case 183: return(207);		/* Caron */
      case 189: return(205);		/* Double acute */
      case 208: return(144);		/* D stroke = Eth */
      case 240: return(230);		/* d stroke = eth */
      case 255: return(199);		/* Dot above */
      default:  return(yl1ne[yl2l1[c]]);
    }
}

CHAR					/* Latin-2 to CP437 */
#ifdef CK_ANSIC
xl243(CHAR c)
#else
xl243(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xll243 */
    return(yl1l2[y43l1[c]]);
}

CHAR					/* Latin-2 to CP850 */
#ifdef CK_ANSIC
xl285(CHAR c)
#else
xl285(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xll285 */
    return(yl1l2[y85l1[c]]);
}

CHAR					/* Latin-2 to Apple */
#ifdef CK_ANSIC
xl2aq(CHAR c)
#else
xl2aq(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl2aq */
    return(yl1aq[yl2l1[c]]);		/* Could do more... */
}

CHAR					/* Latin-2 to DGI */
#ifdef CK_ANSIC
xl2dg(CHAR c)
#else
xl2dg(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xll2dg */
    return(ydgl1[yl1l2[c]]);
}

CHAR					/* Latin-2 to Short KOI */
#ifdef CK_ANSIC
xl2sk(CHAR c)
#else
xl2sk(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xll2sk */
    return(islower(c) ? toupper(c) : c);
}

CHAR					/* NeXT to Latin-2 */
#ifdef CK_ANSIC
xnel2(CHAR c)
#else
xnel2(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xnel2 */
    switch (c) {
      case 144: return(208);		/* D stroke = Eth */
      case 198: return(162);		/* Breve */
      case 199: return(255);		/* Dot above */
      case 205: return(189);		/* Double acute */
      case 206: return(178);		/* Ogonek */
      case 207: return(183);		/* Caron */
      case 230: return(240);		/* d stroke = eth */
      case 232: return(163);		/* L with stroke */
      case 248: return(179);		/* l with stroke */
      default:  return(yl1l2[ynel1[c]]); /* Others, go thru Latin-1 */
    }
}

CHAR					/* CP437 to Latin-2 */
#ifdef CK_ANSIC
x43l2(CHAR c)
#else
x43l2(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl43l2 */
    return(yl1l2[y43l1[c]]);
}

CHAR					/* CP850 to Latin-2 */
#ifdef CK_ANSIC
x85l2(CHAR c)
#else
x85l2(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl85l2 */
    return(yl1l2[y85l1[c]]);
}

CHAR					/* Apple to Latin-2 */
#ifdef CK_ANSIC
xaql2(CHAR c)
#else
xaql2(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xlaql2 */
    switch (c) {
      case 249: return(162);		/* Breve accent */
      case 250: return(255);		/* Dot accent */
      case 253: return(189);		/* Double acute */
      default: return(yl1l2[yaql1[c]]);
    }
}

CHAR					/* DGI to Latin-2 */
#ifdef CK_ANSIC
xdgl2(CHAR c)
#else
xdgl2(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xldgl2 */
    return(yl1l2[ydgl1[c]]);		/* (for now) */
}

CHAR					/* Short KOI to Latin-2 */
#ifdef CK_ANSIC
xskl2(CHAR c)
#else
xskl2(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xlskl2 */
    return(islower(c) ? toupper(c) : c);
}

CHAR					/* Latin-2 to German */
#ifdef CK_ANSIC
xl2ge(CHAR c)
#else
xl2ge(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xll2ge */
    switch(c) {
      case 167: return(64);		/* Paragraph sign */
      case 196: return(91);		/* A-diaeresis */
      case 214: return(92);		/* O-diaeresis */
      case 220: return(93);		/* U-diaeresis */
      case 223: return(126);		/* double-s */
      case 228: return(123);		/* a-diaeresis */
      case 246: return(124);		/* o-diaeresis */
      case 252: return(125);		/* u-diaeresis */
      default:  return(yl2as[c]);	/* Others */
    }
}

CHAR					/* German to Latin-2 */
#ifdef CK_ANSIC
xgel2(CHAR c)
#else
xgel2(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xlgel2 */
    if (c & 0x80)
      return(UNK);
    switch(c) {
      case 64:  return(167);		/* Paragraph sign */
      case 91:  return(196);		/* A-diaeresis */
      case 92:  return(214);		/* O-diaeresis */
      case 93:  return(220);		/* U-diaeresis */
      case 123: return(228);		/* a-diaeresis */
      case 126: return(223);		/* double-s */
      case 124: return(246);		/* o-diaeresis */
      case 125: return(252);		/* u-diaeresis */
      default:  return(c);		/* Others */
    }
}

CHAR					/* Latin-2 to Hungarian */
#ifdef CK_ANSIC
xl2hu(CHAR c)
#else
xl2hu(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xll2hu */
    switch(c) {
      case 164: return(36);		/* Currency symbol */
      case 189: return(126);		/* Double acute accent */
      case 193: return(64);		/* A-acute */
      case 201: return(91);		/* E-acute */
      case 214: return(92);		/* O-diaeresis */
      case 220: return(93);		/* U-diaeresis */
      case 225: return(96);		/* a-acute */
      case 233: return(123);		/* e-acute */
      case 246: return(124);		/* o-diaeresis */
      case 252: return(125);		/* u-diaeresis */
      default:  return(yl2as[c]);	/* Others */
    }
}

CHAR					/* Hungarian to Latin-2 */
#ifdef CK_ANSIC
xhul2(CHAR c)
#else
xhul2(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xlhul2 */
    if (c & 0x80)
      return(UNK);
    switch(c) {
      case 36:  return(164);		/* Currency symbol */
      case 64:  return(193);		/* A-acute */
      case 91:  return(201);		/* E-acute */
      case 92:  return(214);		/* O-diaeresis */
      case 93:  return(220);		/* U-diaeresis */
      case 96:  return(225);		/* a-acute */
      case 123: return(233);		/* e-acute */
      case 124: return(246);		/* o-diaeresis */
      case 125: return(252);		/* u-diaeresis */
      case 126: return(189);		/* Double acute accent */
      default:  return(c);		/* Others */
    }
}

CHAR
#ifdef CK_ANSIC
xr8l2(CHAR c)
#else
xr8l2(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xr8l2 */ /* Hewlett Packard Roman8 to Latin-2 */
    switch (c) {
      case 235: return(169);		/* S caron */
      case 236: return(185);		/* s caron */
      default:  return(yl1l2[yr8l1[c]]);
    }
}

CHAR
#ifdef CK_ANSIC
xl2r8(CHAR c)
#else
xl2r8(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl2r8 */ /* Latin-2 to Hewlett Packard Roman8 Character Set */
    switch (c) {
      case 169: return(235);		/* S caron */
      case 185: return(236);		/* s caron */
      default:  return(yr8l1[yl1l2[c]]);
    }
}

#else /* NOLATIN2 */

#define xl1mz NULL
#define xmzl1 NULL
#define xl2mz NULL
#define xmzl2 NULL
#define xl9mz NULL
#define xmzl9 NULL
#define xmzas NULL

#define xl11250 NULL
#define xl21250 NULL
#define xl91250 NULL

#define x1250as NULL
#define x1250l1 NULL
#define x1250l2 NULL
#define x1250l9 NULL

#define xl2l1 NULL
#define xl2w1 NULL
#define xl1l2 NULL
#define xw1l2 NULL
#define xl2as NULL
#define xl252 NULL
#define x52l2 NULL
#define x52as NULL
#define x52l1 NULL
#define xl152 NULL
#define xl2ne NULL
#define xl243 NULL
#define xl285 NULL
#define xl2aq NULL
#define xl2dg NULL
#define xl2sk NULL
#define xnel2 NULL
#define x43l2 NULL
#define x85l2 NULL
#define xaql2 NULL
#define xdgl2 NULL
#define xskl2 NULL
#define xl2ge NULL
#define xgel2 NULL
#define xl2hu NULL
#define xhul2 NULL
#define xl2r8 NULL
#define xr8l2 NULL
#endif /* LATIN2 */

/* This one can also be used for ELOT 927, Hebrew 7, etc */

CHAR
#ifdef CK_ANSIC
xassk(CHAR c)
#else
xassk(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xassk */				/* ASCII to Short KOI */
    if (c & 0x80)
      return(UNK);
    return((c > 95) ? (c - 32) : c);	/* Fold columns 6-7 to 4-5 */
}

#ifdef CYRILLIC
/* Translation functions for Cyrillic character sets */

CHAR					/* Latin/Cyrillic to CP866 */
#ifdef CK_ANSIC
xlcac(CHAR c)
#else
xlcac(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xlcac */				/* PC Code Page 866 */
    return(ylcac[c]);
}

CHAR					/* Latin/Cyrillic to */
#ifdef CK_ANSIC
xlc55(CHAR c)
#else
xlc55(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xlc55 */				/* PC Code Page 855 */
    return(ylc55[c]);
}

CHAR					/* Latin/Cyrillic to */
#ifdef CK_ANSIC
xlc1251(CHAR c)
#else
xlc1251(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xlc1251 */				/* PC Code Page 1251 */
    return(ylc1251[c]);
}

CHAR					/* Latin/Cyrillic to... */
#ifdef CK_ANSIC
xlcbu(CHAR c)
#else
xlcbu(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xlcbu */				/* Bulgarian PC Code Page */
    return(ylcbu[c]);
}

CHAR					/* Latin/Cyrillic to Old KOI-8 */
#ifdef CK_ANSIC
xlck8(CHAR c)
#else
xlck8(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xlck8 */
    return(ylck8[c]);
}

CHAR					/* Latin/Cyrillic to KOI8-R */
#ifdef CK_ANSIC
xlckr(CHAR c)
#else
xlckr(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xlckr */
    switch(c) {
      case 0xa1: return(0xb3);		/* Io */
      case 0xf1: return(0xa3);		/* io */
      default:
	if (c > 0x7f && c < 0xc0)
	  return(UNK);
	return(ylck8[c]);
    }
}

CHAR					/* Latin/Cyrillic to  KOI8-U */
#ifdef CK_ANSIC
xlcku(CHAR c)
#else
xlcku(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xlcku */
    switch(c) {
      case 0xa1: return(0xb3);		/* Io */
      case 0xf1: return(0xa3);		/* io */
      case 0xf4: return(0xa4);		/* Ukrainian ie */
      case 0xf6: return(0xa6);		/* Ukrainian i */
      case 0xf7: return(0xa7);		/* Ukrainian yi */
      case 0xf3: return(0xad);		/* Ukrainian ghe with upturn */
      case 0xa4: return(0xb4);		/* Ukrainian Ie */
      case 0xa6: return(0xb6);		/* Ukrainian I */
      case 0xa7: return(0xb7);		/* Ukrainian Yi */
      case 0xa3: return(0xbd);		/* Ukrainian Ghe with upturn */
      default:
	if (c > 0x7f && c < 0xc0)
	  return(UNK);
	return(ylck8[c]);
    }
}

CHAR
#ifdef CK_ANSIC
xlcsk(CHAR c)
#else
xlcsk(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xlcsk */				/* Latin/Cyrillic to Short KOI */
    return(ylcsk[c]);
}

CHAR
#ifdef CK_ANSIC
xlcas(CHAR c)
#else
xlcas(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xlcas */				/* Latin/Cyrillic to ASCII */
    if (langs[language].id == L_RUSSIAN)
      return(ylcsk[c]);
    else
      return((c > 127) ? '?' : c);
}

CHAR					/* CP866 */
#ifdef CK_ANSIC
xaclc(CHAR c)
#else
xaclc(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xaclc */				/* to Latin/Cyrillic */
    return(yaclc[c]);
}

CHAR					/* CP855 */
#ifdef CK_ANSIC
x55lc(CHAR c)
#else
x55lc(c) CHAR c;
#endif /* CK_ANSIC */
{ /* x55lc */				/* to Latin/Cyrillic */
    return(y55lc[c]);
}

CHAR					/* Bulgarian PC Code Page ... */
#ifdef CK_ANSIC
xbulc(CHAR c)
#else
xbulc(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xbulc */				/* to Latin/Cyrillic */
    return(ybulc[c]);
}

CHAR					/* CP1251 */
#ifdef CK_ANSIC
x1251lc(CHAR c)
#else
x1251lc(c) CHAR c;
#endif /* CK_ANSIC */
{ /* x1251lc */				/* to Latin/Cyrillic */
    return(y1251lc[c]);
}

CHAR					/* Old KOI-8 to Latin/Cyrillic */
#ifdef CK_ANSIC
xk8lc(CHAR c)
#else
xk8lc(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xk8lc */
    return(yk8lc[c]);
}

CHAR					/* KOI8-R to Latin/Cyrillic */
#ifdef CK_ANSIC
xkrlc(CHAR c)
#else
xkrlc(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xkrlc */
    if (c == 0xb3) return(0xa1);
    else if (c == 0xa3) return(0xf1);
    else if (c > 0x7f && c < 0xc0)
      return(UNK);
    return(yk8lc[c]);
}

CHAR					/* KOI8-U to Latin/Cyrillic */
#ifdef CK_ANSIC
xkulc(CHAR c)
#else
xkulc(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xkulc */
    switch (c) {
      case 0xb3: return(0xa1);		/* Io */
      case 0xa3: return(0xf1);		/* io */
      case 0xa4: return(0xf4);		/* Ukrainian ie */
      case 0xa6: return(0xf6);		/* Ukrainian i */
      case 0xa7: return(0xf7);		/* Ukrainian yi */
      case 0xad: return(0xf3);		/* Ukrainian ghe with upturn */
      case 0xb4: return(0xa4);		/* Ukrainian Ie */
      case 0xb6: return(0xa6);		/* Ukrainian I */
      case 0xb7: return(0xa7);		/* Ukrainian Yi */
      case 0xbd: return(0xa3);		/* Ukrainian Ghe with upturn */
      /* Note substitution of Gje for Ghe-Upturn, which is not in 8859-5 */
      default:
	if (c > 0x7f && c < 0xc0)
	  return(UNK);
	return(yk8lc[c]);
    }
}

CHAR
#ifdef CK_ANSIC
xskcy(CHAR c)
#else
xskcy(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xskcy */			/* Short KOI to Latin/Cyrillic */
    return(yskcy[c & 0x7f]);
}

CHAR
#ifdef CK_ANSIC
xascy(CHAR c)
#else
xascy(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xascy */			/* ASCII to Latin/Cyrillic */
    if (langs[language].id == L_RUSSIAN) { /* If LANGUAGE == RUSSIAN  */
	return(yskcy[c & 0x7f]);	/* treat ASCII as Short KOI */
    } else return((c > 127) ? '?' : c);
}

CHAR
#ifdef CK_ANSIC
xacas(CHAR c)
#else
xacas(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xacas */			/* CP866 to ASCII */
    if (langs[language].id == L_RUSSIAN) {
	c = yaclc[c];			/* First to Latin/Cyrillic */
	return(ylcsk[c]);		/* Then to Short KOI */
    } else return((c > 127) ? '?' : c);
}

CHAR
#ifdef CK_ANSIC
x55as(CHAR c)
#else
x55as(c) CHAR c;
#endif /* CK_ANSIC */
{ /* x55as */			/* CP855 to ASCII */
    if (langs[language].id == L_RUSSIAN) {
	c = y55lc[c];			/* First to Latin/Cyrillic */
	return(ylcsk[c]);		/* Then to Short KOI */
    } else return((c > 127) ? '?' : c);
}

CHAR
#ifdef CK_ANSIC
x1251as(CHAR c)
#else
x1251as(c) CHAR c;
#endif /* CK_ANSIC */
{ /* x1251as */			/* CP81251 to ASCII */
    if (langs[language].id == L_RUSSIAN) {
	c = y1251lc[c];			/* First to Latin/Cyrillic */
	return(ylcsk[c]);		/* Then to Short KOI */
    } else return((c > 127) ? '?' : c);
}

CHAR
#ifdef CK_ANSIC
xskas(CHAR c)
#else
xskas(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xskas */				/* Short KOI to ASCII */
    return((c > 95) ? '?' : c);
}

CHAR
#ifdef CK_ANSIC
xk8as(CHAR c)
#else
xk8as(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xk8as */				/* Old KOI-8 Cyrillic to ASCII */
    if (langs[language].id == L_RUSSIAN) {
	c = yk8lc[c];			/* First to Latin/Cyrillic */
	return(ylcsk[c]);		/* Then to Short KOI */
    } else return((c > 127) ? '?' : c);
}

CHAR
#ifdef CK_ANSIC
xl1sk(CHAR c)
#else
xl1sk(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl1sk */				/* Latin-1 to Short KOI */
    c = zl1as(c);			/* Convert to ASCII */
    return(c = xassk(c));		/* Convert ASCII to Short KOI */
}

CHAR
#ifdef CK_ANSIC
xw1lc(CHAR c)
#else
xw1lc(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xw1lc */				/* CP1252 to Latin/Cyrillic */
    return((c < 160) ? xw1as(c) : zl1as(c));
}

CHAR
#ifdef CK_ANSIC
xaslc(CHAR c)
#else
xaslc(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xaslc */			/* ASCII to Latin/Cyrillic */
    if (langs[language].id == L_RUSSIAN)
      return(yskcy[c & 0x7f]);
    else return(c & 0x7f);
}

CHAR
#ifdef CK_ANSIC
xasac(CHAR c)
#else
xasac(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xasac */			/* ASCII to CP866 */
    if (c & 0x80)
      return(UNK);
    if (langs[language].id == L_RUSSIAN) { /* Use Short KOI */
	c = xskcy(c);			/* Translate to Latin/Cyrillic */
	return(ylcac[c]);		/* Then to CP866 */
    } else return(c & 0x7f);
}

CHAR
#ifdef CK_ANSIC
xas55(CHAR c)
#else
xas55(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xas55 */			/* ASCII to CP855 */
    if (c & 0x80)
      return(UNK);
    if (langs[language].id == L_RUSSIAN) { /* Use Short KOI */
	c = xskcy(c);			/* Translate to Latin/Cyrillic */
	return(ylc55[c]);		/* Then to CP866 */
    } else return(c & 0x7f);
}

CHAR
#ifdef CK_ANSIC
xas1251(CHAR c)
#else
xas1251(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xas1251 */			/* ASCII to CP81251 */
    if (c & 0x80)
      return(UNK);
    if (langs[language].id == L_RUSSIAN) { /* Use Short KOI */
	c = xskcy(c);			/* Translate to Latin/Cyrillic */
	return(ylc1251[c]);		/* Then to CP866 */
    } else return(c & 0x7f);
}

CHAR
#ifdef CK_ANSIC
xask8(CHAR c)
#else
xask8(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xask8 */			/* ASCII to KOI-8 */
    if (c & 0x80)
      return(UNK);
    if (langs[language].id == L_RUSSIAN) { /* Use Short KOI */
	c = xskcy(c);			/* Translate to Latin/Cyrillic */
	return(ylck8[c]);		/* Then to KOI-8 */
    } else return(c & 0x7f);
}
#else /* No Cyrillic */
#define xacas NULL
#define x55as NULL
#define x1251as NULL
#define xaclc NULL
#define x55lc NULL
#define x1251lc NULL
#define xasac NULL
#define xas55 NULL
#define xas1251 NULL
#define xascy NULL
#define xask8 NULL
#define xaslc NULL
#define xassk NULL
#define xk8as NULL
#define xk8lc NULL
#define xkrlc NULL
#define xkulc NULL
#define xl1sk NULL
#define xw1lc NULL
#define xlcac NULL
#define xlc55 NULL
#define xlc1251 NULL
#define xlcas NULL
#define xlck8 NULL
#define xlckr NULL
#define xlcku NULL
#define xlch7 NULL
#define xlcsk NULL
#define xskas NULL
#define xskcy NULL
#define xbulc NULL
#define xlcbu NULL
#endif /* CYRILLIC */

/* Translation functions for Hebrew character sets */

#ifdef HEBREW

CHAR
#ifdef CK_ANSIC
xash7(CHAR c)
#else
xash7(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xash7 */			/* ASCII to Hebrew-7 */
    if (c & 0x80)
      return(UNK);
    if (c == 96) return('?');
    if (c > 96 && c < 123) return(c - 32);
    else return(c);
}

CHAR
#ifdef CK_ANSIC
xl1h7(CHAR c)
#else
xl1h7(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl1h7 */			/* Latin-1 to Hebrew-7 */
    return(xash7(xl1as(c)));
}

CHAR
#ifdef CK_ANSIC
xl1lh(CHAR c)
#else
xl1lh(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl1lh */				/* Latin-1 to Latin/Hebrew */
    switch(c) {
      case 170: return('a');		/* Feminine ordinal */
      case 186: return('o');		/* Masculine ordinal */
      case 215: return(170);		/* Times */
      case 247: return(186);		/* Divide */
      default:  return( (c > 190) ? xl1as(c) : c );
    }
}

CHAR
#ifdef CK_ANSIC
xw1lh(CHAR c)
#else
xw1lh(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xw1lh */				/* CP1252 to Latin/Hebrew */
    switch(c) {
      case 170: return('a');		/* Feminine ordinal */
      case 186: return('o');		/* Masculine ordinal */
      case 215: return(170);		/* Times */
      case 247: return(186);		/* Divide */
      default:
	if (c < 160)
	  return(xw1as(c));
	else
	  return((c > 190) ? xl1as(c) : c);
    }
}

#ifdef LATIN2
CHAR
#ifdef CK_ANSIC
xl2h7(CHAR c)
#else
xl2h7(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl2h7 */				/* Latin-2 to Hebrew-7 */
    return(xash7(xl2as(c)));
}
#else
#define xl2h7 NULL
#endif /* LATIN2 */

#ifndef NOCYRIL
CHAR
#ifdef CK_ANSIC
xlch7(CHAR c)
#else
xlch7(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xlch7 */				/* Latin/Cyrillic to Hebrew-7 */
    return(xash7(xlcas(c)));
}
#endif /* NOCYRIL */

CHAR
#ifdef CK_ANSIC
xlhas(CHAR c)
#else
xlhas(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xlhas */			/* Latin/Hebrew to ASCII */
    return( (c > 127) ? '?' : c );
}

CHAR
#ifdef CK_ANSIC
xlhl1(CHAR c)
#else
xlhl1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xlhl1 */			/* Latin/Hebrew to Latin-1 */
    switch (c) {
      case 170: return(215);
      case 186: return(247);
      default: return( (c > 190) ? '?' : c );
    }
}

CHAR
#ifdef CK_ANSIC
xlhw1(CHAR c)
#else
xlhw1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xlhw1 */			/* Latin/Hebrew to CP1252 */
    if (c > 127 && c < 160)
      return('?');
    switch (c) {
      case 170: return(215);
      case 186: return(247);
      default: return( (c > 190) ? '?' : c );
    }
}

CHAR
#ifdef CK_ANSIC
xlh62(CHAR c)
#else
xlh62(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xlh62 */			/* Latin/Hebrew to CP862 */
    return(ylh62[c]);
}

CHAR
#ifdef CK_ANSIC
xl162(CHAR c)
#else
xl162(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl162 */			/* Latin-1 to CP862 */
    return(xlh62(xl1lh(c)));	/* Via Latin/Hebrew */
}

CHAR
#ifdef CK_ANSIC
xlhh7(CHAR c)
#else
xlhh7(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xlhh7 */			/* Latin/Hebrew to Hebrew-7 */
    return(ylhh7[c]);
}

CHAR
#ifdef CK_ANSIC
xh7as(CHAR c)
#else
xh7as(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xh7as */			/* Hebrew-7 to ASCII */
    if (c & 0x80)
      return(UNK);
    return( (c > 95 && c < 123) ? '?' : c );
}

CHAR
#ifdef CK_ANSIC
x62lh(CHAR c)
#else
x62lh(c) CHAR c;
#endif /* CK_ANSIC */
{ /* x62lh */			/* CP862 to Latin/Hebrew */
    return(y62lh[c]);
}

CHAR
#ifdef CK_ANSIC
x62as(CHAR c)
#else
x62as(c) CHAR c;
#endif /* CK_ANSIC */
{ /* x62as */			/* CP862 to ASCII */
    return( xlhas(x62lh(c)) );
}

CHAR
#ifdef CK_ANSIC
x62l1(CHAR c)
#else
x62l1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* x62l1 */			/* CP862 to Latin-1 */
    return( xlhl1(x62lh(c)) );
}

CHAR
#ifdef CK_ANSIC
xh7lh(CHAR c)
#else
xh7lh(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xh7lh */			/* Hebrew-7 to Latin/Hebrew */
    if (c & 0x80)
      return(UNK);
    return(yh7lh[c]);
}

#else /* No Hebrew */

#define xash7 NULL
#define xl1h7 NULL
#define xl2h7 NULL
#define xlch7 NULL
#define xl1lh NULL
#define xw1lh NULL
#define xlhas NULL
#define xlhl1 NULL
#define xlhw1 NULL
#define xl162 NULL
#define xlhh7 NULL
#define xlh62 NULL
#define xh7as NULL
#define x62as NULL
#define x62l1 NULL
#define xh7lh NULL
#define x62lh NULL

#endif /* HEBREW */

/* Translation functions for Greek character sets */

#ifdef GREEK

CHAR
#ifdef CK_ANSIC
xaseg(CHAR c)
#else
xaseg(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xaseg */			/* ASCII to ELOT 927 */
    if (c & 0x80)
      return(UNK);
    if (c > 96 && c < 123) return(c - 32);
    else return(c);
}

CHAR
#ifdef CK_ANSIC
xl1eg(CHAR c)
#else
xl1eg(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl1ge */			/* Latin-1 to ELOT 927 */
    return(xaseg(xl1as(c)));
}

CHAR
#ifdef CK_ANSIC
xl2lg(CHAR c)
#else
xl2lg(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl2lg */			/* Latin-1 to Latin/Greek */
    if (c < 160) return(c);
    else if (c == 160 || c == 168 || c == 173 || c == 174)
      return(c);
    else return('?');
}

CHAR
#ifdef CK_ANSIC
xl1lg(CHAR c)
#else
xl1lg(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl1lg */			/* Latin-1 to Latin/Greek */
    if (c < 160) return(c);
    switch(c) {
      case 160:				/* Themselves */
      case 164:
      case 166:
      case 167:
      case 168:
      case 169:
      case 171:
      case 172:
      case 173:
      case 176:
      case 177:
      case 178:
      case 179:
      case 180:
      case 187:
      case 189:
	return(c);
      case 181:				/* Lowercase mu */
	return(236);
      default:
	return(UNK);
    }
}

CHAR
#ifdef CK_ANSIC
xw1lg(CHAR c)
#else
xw1lg(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xw1lg */				/* CP1252 to Latin/Greek */
    return((c < 160) ? xw1as(c) : xl1lg(c));
}


#ifdef LATIN2
CHAR
#ifdef CK_ANSIC
xl2eg(CHAR c)
#else
xl2eg(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl2eg */				/* Latin-2 to ELOT 927 */
    return(xaseg(xl2as(c)));
}
#else
#define xl2eg NULL
#endif /* LATIN2 */

#ifndef NOCYRIL
CHAR
#ifdef CK_ANSIC
xlceg(CHAR c)
#else
xlceg(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xlceg */			/* Latin/Cyrillic to ELOT 927 */
    return(xaseg(xlcas(c)));
}
#endif /* NOCYRIL */

CHAR
#ifdef CK_ANSIC
xlgas(CHAR c)
#else
xlgas(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xlgas */			/* Latin/Greek to ASCII */
    return( (c > 127) ? '?' : c );
}

CHAR
#ifdef CK_ANSIC
xlgl1(CHAR c)
#else
xlgl1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xlgl1 */			/* Latin/Greek to Latin-1 */
    if (c == 236)
      return(181);
    else
      return(xl1lg(c));
}

CHAR
#ifdef CK_ANSIC
xlgw1(CHAR c)
#else
xlgw1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xlgw1 */			/* Latin/Greek to Latin-1 */
    if (c > 127 && c < 160)
      return('?');
    return(xlgl1(c));
}

CHAR
#ifdef CK_ANSIC
xlg69(CHAR c)
#else
xlg69(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xlg69 */			/* Latin/Greek to CP869 */
    return(ylg69[c]);
}

CHAR
#ifdef CK_ANSIC
xl169(CHAR c)
#else
xl169(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl169 */			/* Latin-1 to CP869 */
    return(xlg69(xl1lg(c)));	/* Via Latin/Greek */
}

CHAR
#ifdef CK_ANSIC
xlgeg(CHAR c)
#else
xlgeg(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xlgeg */			/* Latin/Greek to ELOT 927 */
    return(ylgeg[c]);
}

CHAR
#ifdef CK_ANSIC
xegas(CHAR c)
#else
xegas(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xegas */			/* ELOT 927 to ASCII */
    if (c & 0x80)
      return(UNK);
    return( (c > 96 && c < 123) ? '?' : c );
}

CHAR
#ifdef CK_ANSIC
x69lg(CHAR c)
#else
x69lg(c) CHAR c;
#endif /* CK_ANSIC */
{ /* x69lg */			/* CP869 to Latin/Greek */
    return(y69lg[c]);
}

CHAR
#ifdef CK_ANSIC
x69as(CHAR c)
#else
x69as(c) CHAR c;
#endif /* CK_ANSIC */
{ /* x69as */			/* CP869 to ASCII */
    return( xlgas(x69lg(c)) );
}

CHAR
#ifdef CK_ANSIC
x69l1(CHAR c)
#else
x69l1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* x69l1 */			/* CP869 to Latin-1 */
    return( xlgl1(x69lg(c)) );
}

CHAR
#ifdef CK_ANSIC
xeglg(CHAR c)
#else
xeglg(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xeglg */			/* ELOT 927 to Latin/Greek */
    return(yeglg[c]);
}

#else /* No Greek */

#define x69as NULL
#define x69l1 NULL
#define x69lg NULL
#define xaseg NULL
#define xegas NULL
#define xeglg NULL
#define xl169 NULL
#define xl1eg NULL
#define xl1lg NULL
#define xw1lg NULL
#define xl2ge NULL
#define xl2lg NULL
#define xlcge NULL
#define xlg69 NULL
#define xlgas NULL
#define xlgeg NULL
#define xlgge NULL
#define xlgl1 NULL
#define xlgw1 NULL

#endif /* GREEK */


/* Translation functions for Japanese Kanji character sets */

#ifdef KANJI
/*
  Translate Kanji Transfer Character Set (EUC) to local file character set,
  contributed by Dr. Hirofumi Fujii, Japan High Energy Research Laboratory
  (KEK), Tokyo, Japan.

  a is a byte to be translated, which may be a single-byte character,
  the Katakana prefix, the first byte of a two-byte Kanji character, or the
  second byte of 2-byte Kanji character.

  fn is the output function.

  Returns 0 on success, -1 on failure.
*/

_PROTOTYP(static int jpnxas, (int, int[]) );
_PROTOTYP(static int jpnxkt, (int, int[]) );
_PROTOTYP(static int jpnxkn, (int[], int[]) );

static int jpncnt;			/* Byte count for Japanese */
static int jpnlst;			/* Last status (for JIS7) */

static int
jpnxas(a, obuf) int a; int obuf[]; { /* Translate ASCII to local file code */
    int r;

    r = 0;
    if (fcharset == FC_JIS7) {
	switch (jpnlst) {
	  case 1:
	    obuf[0] = 0x0f;
	    obuf[1] = a;
	    r = 2;
	    break;
	  case 2:
	    obuf[0] = 0x1b;
	    obuf[1] = 0x28;
	    obuf[2] = 0x4a;
	    obuf[3] = a;
	    r = 4;
	    break;
	  default:
	    obuf[0] = a;
	    r = 1;
	    break;
	}
    } else {
	obuf[0] = a;
	r = 1;
    }
    return(r);
}

static int
jpnxkt(a, obuf) int a; int obuf[]; {
/* Translate JIS X 201 Katakana to local code */

    int r;

    r = 0;
    if (fcharset == FC_JIS7) {
	switch (jpnlst) {
	  case 2:				/* from Kanji */
	    obuf[r++] = 0x1b;
	    obuf[r++] = 0x28;
	    obuf[r++] = 0x4a;
	  case 0:				/* from Roman */
	    obuf[r++] = 0x0e;
	  default:
	    obuf[r++] = (a & 0x7f);
	  break;
	}
    } else {
	if (fcharset == FC_JEUC)
	  obuf[r++] = 0x8e;
	obuf[r++] = (a | 0x80);
    }
    return(r);
}

static int
jpnxkn(ibuf, obuf) int ibuf[], obuf[]; {
    /* Translate JIS X 0208 Kanji to local code */
    int c1, c2;
    int r;

    c1 = ibuf[0] & 0x7f;
    c2 = ibuf[1] & 0x7f;

    if (fcharset == FC_SHJIS) {
	if (c1 & 1)
	  c2 += 0x1f;
	else
	  c2 += 0x7d;

        if (c2 >= 0x7f) c2++;

        c1 = ((c1 - 0x21) >> 1) + 0x81;
        if (c1 > 0x9f) c1 += 0x40;

        obuf[0] = c1;
        obuf[1] = c2;
        r = 2;
    } else if (fcharset == FC_JIS7) {
        r = 0;
        switch (jpnlst) {
  	  case 1:
	    obuf[r++] = 0x0f; /* From Katakana */
  	  case 0:
	    obuf[r++] = 0x1b;
	    obuf[r++] = 0x24;
	    obuf[r++] = 0x42;
	  default:
	    obuf[r++] = c1;
	    obuf[r++] = c2;
	    break;
	}
    } else {
        obuf[0] = (c1 | 0x80);
        obuf[1] = (c2 | 0x80);
        r = 2;
    }
    return(r);
}

int
xkanjf() {
/* Initialize parameters for xkanji */
/* This function should be called when F/X-packet is received */
    jpncnt = jpnlst = 0;
    return(0);
}

int
#ifdef CK_ANSIC
xkanjz(int (*fn)(char))
#else
xkanjz(fn) int (*fn)();
#endif /* CK_ANSIC */
{ /* xkanjz */
/*
  Terminate xkanji
  This function must be called when Z-packet is received
  (before closing the file).
*/
    static int obuf[6];
    int r, i, c;

    if (fcharset == FC_JIS7) {
        c = 'A';			/* Dummy Roman character */
        r = jpnxas(c, obuf) - 1;	/* -1 removes Dummy character */
        if (r > 0) {
	    for (i = 0; i < r; i++)
	      if (((*fn)((char) obuf[i])) < 0)
		return(-1);
	}
    }
    return(0);
}

int
#ifdef CK_ANSIC
xkanji(int a, int (*fn)(char))
#else
xkanji(a, fn) int a; int (*fn)();
#endif /* CK_ANSIC */
{ /* xkanji */
    static int xbuf[2];
    static int obuf[8];

    int i, r;
    int c7;
    int state=0;

    r = 0;
    if (jpncnt == 0) {
	/* 1st byte */
	if ((a & 0x80) == 0) {
	    /* 8th bit is 0, i.e., single-byte code */
	    r = jpnxas(a, obuf);
	    state = 0;
	} else {
	    /* 8th bit is 1, check the range */
	    c7 = a & 0x7f;
	    if (((c7 > 0x20) && (c7 < 0x7f)) || (c7 == 0x0e)) {
	        /* double byte code */
	        xbuf[jpncnt++] = a;
	    } else {
	        /* single byte code */
	        r = jpnxas(a, obuf);
	        state = 0;
	    }
	}
    } else {
	/* not the 1st byte */
	xbuf[jpncnt++] = a;
	if (xbuf[0] == 0x8e) {
	    r = jpnxkt(xbuf[1], obuf);
	    state = 1;
	} else {
	    r = jpnxkn(xbuf, obuf);
	    state = 2;
	}
    }
    if (r > 0) {
        for (i = 0; i < r; i++ )
	  if (((*fn)((char) obuf[i])) < 0)
	    return(-1);
        jpnlst = state;
        jpncnt = 0;
    }
    return(0);
}

/*
  Function for translating from Japanese file character set
  to Japanese EUC transfer character set.
  Returns a pointer to a string containing 0, 1, or 2 bytes.
*/

/* zkanji */
static int jpnstz;			/* status for JIS-7 */
static int jpnpnd;			/* number of pending bytes */
static int jpnpnt;			/* pending buffer index */
static int jpnpbf[8];			/* pending buffer */

/* There is some duplication here between the old and new JIS-7 parsers */
/* to be cleaned up later... */

VOID
j7init() {				/* Initialize JIS-7 parser */
    jpnstz = 0;
    jpnpnd = 0;
    jpnpnt = 0;
}

int
getj7() {				/* Reads JIS-7 returns next EUC byte */
    int x;

    if (jpnpnd > 0) {			/* If something is pending */
	x = (unsigned) jpnpbf[jpnpnt++]; /* Get it */
	jpnpnd--;
	if (jpnpnd < 0) jpnpnd = 0;
	return((unsigned)x);
    }
    jpnpnt = 0;

    if ((x = zminchar()) < 0) return(x);
    while (jpnpnd == 0) {		/* While something is pending... */
	if ((x > 0x20) && (x < 0x7f)) {	/* 7-bit graphic character */
	    switch (jpnstz) {
	      case 1:			 /* Katakana */
#ifdef COMMENT
		/* This can't be right... */
		jpnpbf[jpnpnd++] = 0x80; /* Insert flag (NOT SS2???) */
#else
		jpnpbf[jpnpnd++] = 0x8e; /* Insert SS2 */
#endif /* COMMENT */
		jpnpbf[jpnpnd++] = (x | 0x80); /* Insert Kana + 8th bit */
		break;
	      case 2:			/* Kanji */
		jpnpbf[jpnpnd++] = (x | 0x80); /* Get another byte */
		if ((x = zminchar()) < 0) return(x);
		jpnpbf[jpnpnd++] = (x | 0x80);
		break;
	      default:			/* ASCII / JIS Roman */
		jpnpbf[jpnpnd++] = x;
		break;
	    }
	} else if (x == 0x0e) {		/* ^N = SO */
	    jpnstz = 1;			/* Katakana */
	    if ((x = zminchar()) < 0) return(x);
	} else if (x == 0x0f) {		/* ^O = SI */
	    jpnstz = 0;			/* ASCII / JIS Roman */
	    if ((x = zminchar()) < 0) return(x);
	} else if (x == 0x1b) {		/* Escape */
	    jpnpbf[jpnpnd++] = x;	/* Save in buffer */
	    if ((x = zminchar()) < 0) return(x);
	    jpnpbf[jpnpnd++] = x;	/* Save in buffer */
	    if (x == '$') {		/* <ESC>$ */
		if ((x = zminchar()) < 0) return(x);
		jpnpbf[jpnpnd++] = x;
		if ((x == '@') || (x == 'B')) {	/* Kanji */
		    jpnstz = 2;
		    jpnpnt = jpnpnd = 0;
		    if ((x = zminchar()) < 0) return(x);
		}
	    } else if (x == '(') {	/* <ESC>( == 94-byte single-byte set */
		if ((x = zminchar()) < 0) return(x);
		jpnpbf[jpnpnd++] = x;
		if ((x == 'B') || (x == 'J')) {	/* ASCII or JIS Roman */
		    jpnstz = 0;		        /* Set state */
		    jpnpnt = jpnpnd = 0;        /* Reset pointers */
		    if ((x = zminchar()) < 0) return(x);
		}
	    } else if (x == 0x1b) {	/* <ESC><ESC> */
		jpnpnt = jpnpnd = 0;	/* Reset pointers, stay in state */
		if ((x = zminchar()) < 0) return(x);
	    }
	} else {			/* Not <ESC> - just save it */
	    jpnpbf[jpnpnd++] = x;
	}
    }
    jpnpnt = 0;
    x = (unsigned)jpnpbf[jpnpnt++];
    jpnpnd--;
    return((unsigned)x);
}

USHORT
#ifdef CK_ANSIC
eu_to_sj(USHORT eu)			/* EUC-JP to Shift-JIS */
#else
eu_to_sj(eu) USHORT eu;
#endif /* CK_ANSIC */
{
    int c1, c2;
    union ck_short jcode,scode;

    jcode.x_short = eu;
    c1 = (jcode.x_char[byteorder] & 0x7f);
    c2 = (jcode.x_char[1-byteorder] & 0x7f);

    if (c1 & 1)
      c2 += 0x1f;
    else
      c2 += 0x7d;
    if (c2 >= 0x7f)
      c2++;
    c1 = ((c1 - 0x21) >> 1) + 0x81;
    if (c1 > 0x9f)
      c1 += 0x40;

    scode.x_char[byteorder] = c1;
    scode.x_char[1-byteorder] = c2;
    return(scode.x_short);
}


USHORT
#ifdef CK_ANSIC
sj_to_eu(USHORT sj)			/* Shift-JIS to EUC-JP */
#else
sj_to_eu(sj) USHORT sj;
#endif /* CK_ANSIC */
{
    union ck_short jcode, scode;
    int c0, c1;

    scode.x_short = sj;
    c0 = scode.x_char[byteorder];	/* Left (hi order) byte */
    c1 = scode.x_char[1-byteorder];	/* Right (lo order) byte */

    if (((c0 >= 0x81) && (c0 <= 0x9f)) || /* High order byte has 8th bit set */
        ((c0 >= 0xe0) && (c0 <= 0xfc))) { /* Kanji */
	if (c0 <= 0x9f)			  /* Two bytes in */
	  c0 -= 0x71;			  /* Do the shifting... */
	else
	  c0 -= 0xb1;
	c0 = c0 * 2 + 1;
	if (c1 > 0x7f) c1 -= 1;
	if (c1 >= 0x9e) {
	    c1 -= 0x7d;
	    c0 += 1;
	} else {
	    c1 -= 0x1f;
	}
	jcode.x_char[byteorder] = (c0 | 0x80); /* Two bytes out */
	jcode.x_char[1-byteorder] = (c1 | 0x80);

    } else if (c0 == 0) {		/* Single byte */
	if (c1 >= 0xa1 && c1 <= 0xdf) {	/* Katakana */
	    jcode.x_char[byteorder] = 0x8e; /* SS2 */
	    jcode.x_char[1-byteorder] = c1; /* Kana code */
	} else {			/* ASCII or C0 */
	    jcode.x_short = c1;
	}
    } else {				/* Something bad */
	debug(F001,"sj_to_eu bad sj","",sj);
	jcode.x_short = 0xffff;
    }
    return(jcode.x_short);
}

int
zkanjf() {				/* Initialize */
    jpnstz = jpnpnd = jpnpnt = 0;
    return(0);
}

int
zkanjz() {
    return(0);
}

int
#ifdef CK_ANSIC
zkanji(int (*fn)(void))
#else
zkanji(fn) int (*fn)();
#endif /* CK_ANSIC */
{ /* zkanji */
    /* Read Japanese local code and translate to Japanese EUC */
    int a;
    int sc[3];

    /* No pending characters */
    if (fcharset == FC_SHJIS) {		/* Translating from Shift-JIS */
        if (jpnpnd) {
            jpnpnd--;
            return(jpnpbf[jpnpnt++]);
        }
        a = (*fn)();
	jpnpnd = jpnpnt = 0;
	if (((a >= 0x81) && (a <= 0x9f)) ||
	    ((a >= 0xe0) && (a <= 0xfc))) { /* 2-byte Kanji code */
	    sc[0] = a;
	    if ((sc[1] = (*fn)()) < 0)	/* Get second byte */
	      return(sc[1]);
	    if (sc[0] <= 0x9f)
	      sc[0] -= 0x71;
	    else
	      sc[0] -= 0xb1;
	    sc[0] = sc[0] * 2 + 1;
	    if (sc[1] > 0x7f)
	      sc[1]--;
	    if (sc[1] >= 0x9e) {
	        sc[1] -= 0x7d;
	        sc[0]++;
	    } else {
	        sc[1] -= 0x1f;
	    }
	    a = (sc[0] | 0x80);
	    jpnpbf[0] = (sc[1] | 0x80);
	    jpnpnd = 1;
	    jpnpnt = 0;
	} else if ((a >= 0xa1) && (a <= 0xdf)) { /* Katakana */
	    jpnpbf[0] = a;
	    jpnpnd = 1;
	    jpnpnt = 0;
	    a = 0x8e;
	}
	return(a);
    } else if (fcharset == FC_JIS7 ) {	/* 7-bit JIS X 0208 */
        if (jpnpnd) {
            a = jpnpbf[jpnpnt++];
	    jpnpnd--;
            return(a);
        }
        jpnpnt = 0;
        if ((a = (*fn)()) < 0)
	  return(a);
        while (jpnpnd == 0) {
            if ((a > 0x20) && (a < 0x7f)) {
                switch (jpnstz) {
		  case 1:
		    jpnpbf[jpnpnd++] = 0x80; /* Katakana */
		    jpnpbf[jpnpnd++] = (a | 0x80);
		    break;
		  case 2:
		    jpnpbf[jpnpnd++] = (a | 0x80); /* Kanji */
		    if ((a = (*fn)()) < 0)
		      return(a);
		    jpnpbf[jpnpnd++] = (a | 0x80);
		    break;
		  default:
		    jpnpbf[jpnpnd++] = a; /* Single byte */
		    break;
                }
            } else if (a == 0x0e) {
                jpnstz = 1;
                if ((a = (*fn)()) < 0)
		  return(a);
            } else if (a == 0x0f) {
                jpnstz = 0;
                if ((a = (*fn)()) < 0)
		  return(a);
            } else if (a == 0x1b) {
                jpnpbf[jpnpnd++] = a;	/* Escape */
                if ((a = (*fn)()) < 0)
		  return(a);
                jpnpbf[jpnpnd++] = a;
                if (a == '$') {
                    if ((a = (*fn)()) < 0)
		      return(a);
                    jpnpbf[jpnpnd++] = a;
                    if ((a == '@') || (a == 'B')) {
                        jpnstz = 2;
			jpnpnt = jpnpnd = 0;
                        if ((a = (*fn)()) < 0)
			  return(a);
                    }
                } else if (a == '(') {
                    if ((a = (*fn)()) < 0)
		      return(a);
                    jpnpbf[jpnpnd++] = a;
                    if ((a == 'B') || (a == 'J')) {
                        jpnstz = 0;
			jpnpnt = jpnpnd = 0;
                        if ((a = (*fn)()) < 0)
			  return(a);
                    }
                } else if (a == 0x1b) {
                    jpnpnt = jpnpnd = 0;
                    if ((a = (*fn)()) < 0)
		      return(a);
                }
            } else {
                jpnpbf[jpnpnd++] = a;
            }
        }
        jpnpnt = 0;
        a = jpnpbf[jpnpnt++];
	jpnpnd--;
        return(a);
    } else {
        a = (*fn)();
        return(a);
    }
}
#endif /* KANJI */

/* Euro functions */

#ifdef LATIN2
CHAR
#ifdef CK_ANSIC
xl2l9(CHAR c)
#else
xl2l9(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl2l9 */ 				/* Latin-2 to Latin-9... */
    switch (c) {
      case 169: return((CHAR)166);	/* S caron */
      case 185: return((CHAR)168);	/* s caron */
      case 174: return((CHAR)180);	/* Z caron */
      case 190: return((CHAR)184);	/* z caron */
      default:
	return(yl2l1[c]);
    }

}

_PROTOTYP( CHAR xl258, ( CHAR ) );
CHAR
#ifdef CK_ANSIC
xl258(CHAR c)
#else
xl258(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl258 */ 				/* Latin-2 to CP858... */
    return(c);
}
#else
#define xl2l9 NULL
#define xl258 NULL
#endif /* LATIN2 */

CHAR
#ifdef CK_ANSIC
zl9as(CHAR c)
#else
zl9as(c) CHAR c;
#endif /* CK_ANSIC */
{ /* zl9as */ 				/* Latin-9 to US ASCII... */
    if (c < (CHAR)0x80) return(c);	/* Save a function call */
    switch (c) {
      case 0xa4: return(UNK);		/* Euro */
      case 0xa6: return('S');		/* S Caron */
      case 0xa8: return('s');		/* s Caron */
      case 0xb4: return('Z');		/* Z Caron */
      case 0xbc: return('O');		/* OE digraph */
      case 0xbd: return('o');		/* oe digraph */
      case 0xbe: return('Y');		/* Y diaeresis */
      default:   return(zl1as(c));	/* The rest is like Latin-1 */
    }
}

_PROTOTYP( CHAR xl9as, ( CHAR ) );
CHAR
#ifdef CK_ANSIC
xl9as(CHAR c)
#else
xl9as(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl9as */ 				/* Latin-9 to US ASCII... */
    if (c < (CHAR)0x80) return(c);	/* Save a function call */
    switch (c) {
      case 0xa4: return(UNK);		/* Euro */
      case 0xa6: return('S');		/* S Caron */
      case 0xa8: return('s');		/* s Caron */
      case 0xb4: return('Z');		/* Z Caron */
      case 0xb8: return('z');		/* z Caron */
      case 0xbc: return('O');		/* OE digraph */
      case 0xbd: return('o');		/* oe digraph */
      case 0xbe: return('Y');		/* Y diaeresis */
      default:   return(xl1as(c));	/* The rest is like Latin-1 */
    }
}

CHAR					/* CP1252 to Latin-9 */
#ifdef CK_ANSIC
xw1l9(CHAR c)
#else
xw1l9(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xw1l9 */
    switch (c) {
      case 0x80: return(0xa4);		/* Euro sign */
      case 0x8a: return(0xa6);		/* S caron */
      case 0x8c: return(0xbc);		/* OE */
      case 0x8e: return(0xb4);		/* Z caron */
      case 0x9a: return(0xa8);		/* s caron */
      case 0x9c: return(0xbd);		/* oe */
      case 0x9e: return(0xb8);		/* z caron */
      case 0x9f: return(0xbe);		/* Y diaeresis */
      case 0xa4:			/* Currency sign */
      case 0xa6:			/* Broken vertical bar */
      case 0xa8:			/* Diaeresis */
      case 0xb4:			/* Acute accent */
      case 0xb8:			/* Cedilla */
      case 0xbc:			/* 1/4 */
      case 0xbd:			/* 1/2 */
      case 0xbe: return('?');		/* 3/4 */
      default: return((c < 160) ? xw1as(c) : c);
    }
}

#ifdef LATIN2
CHAR
#ifdef CK_ANSIC
xl9l2(CHAR c)
#else
xl9l2(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl9l2 */ 				/* Latin-9 to Latin-2... */
    if (c < (CHAR)0x80) return(c);	/* Save a function call */
    switch (c) {
      case 0xa4: return(UNK);		/* Euro */
      case 0xa6: return((CHAR)0xa9);	/* S Caron */
      case 0xa8: return((CHAR)0xb9);	/* s Caron */
      case 0xb4: return((CHAR)0xae);	/* Z Caron */
      case 0xb8: return((CHAR)0xaf);	/* z Caron */
      case 0xbc: return('O');		/* OE digraph */
      case 0xbd: return('o');		/* oe digraph */
      case 0xbe: return('Y');		/* Y diaeresis */
      default:   return(xl1l2(c));	/* The rest is like Latin-1 */
    }
}
#else
#define xl9l2 NULL
#endif /* LATIN2 */

CHAR
#ifdef CK_ANSIC
xl958(CHAR c)
#else
xl958(c) CHAR c;
#endif /* CK_ANSIC */
{ /* xl958 */ 				/* Latin-9 to CP858... */
    if (c == 0xa4)			/* Euro Symbol */
      return((CHAR)0xd5);
    else if (c == 0x9e)			/* This was currency symbol */
      return((CHAR)0xcf);
    c = yl185[c];
    return(c);
}

CHAR
#ifdef CK_ANSIC
x58as(CHAR c)
#else
x58as(c) CHAR c;
#endif /* CK_ANSIC */
{ /* x58as */ 				/* CP858 to US ASCII... */
    if (c == 0xd5)			/* Euro rather than dotless i */
      return(UNK);
    else
      return(x85as(c));			/* The rest is like CP850 */
}

CHAR
#ifdef CK_ANSIC
x58l1(CHAR c)
#else
x58l1(c) CHAR c;
#endif /* CK_ANSIC */
{ /* x58l1 */ 				/* CP858 to Latin-1... */
    if (c == 0xd5)			/* Euro rather than dotless i */
      return((CHAR)0xa4);		/* Return currency symbol */
    else if (c == 0xcf)			/* This keeps it invertible */
      return((CHAR)0x9e);
    else
      return(x85l1(c));
}

#ifdef LATIN2
CHAR
#ifdef CK_ANSIC
x58l2(CHAR c)
#else
x58l2(c) CHAR c;
#endif /* CK_ANSIC */
{ /* x58l2 */ 				/* CP858 to Latin-2... */
    if (c == 0xd5)			/* Euro rather than dotless i */
      return((CHAR)0xa4);		/* Return currency symbol */
    else if (c == 0xcf)			/* This keeps it invertible */
      return((CHAR)0x9e);		/* (if it ever was...) */
    else				/* Otherwise like CP850 */
      return(x85l2(c));
}
#else
#define x58l2 NULL
#endif /* LATIN2 */

CHAR
#ifdef CK_ANSIC
x58l9(CHAR c)
#else
x58l9(c) CHAR c;
#endif /* CK_ANSIC */
{ /* x58l9 */ 				/* CP-858 to Latin-9... */
    if (c == 0xd5)			/* Euro rather than dotless i */
      return((CHAR)0xa4);		/* Return currency symbol */
    else if (c == 0xcf)			/* This keeps it invertible */
      return((CHAR)0x9e);		/* (if it ever was...) */
    else				/* Otherwise like CP850 */
      return(x85l1(c));			/* to Latin-1 */
}

/* End Euro functions */


/*  TABLES OF TRANSLATION FUNCTIONS */

/*
  First, the table of translation functions for RECEIVING files.  That is,
  *from* the TRANSFER character set *to* the FILE character set, an array of
  pointers to functions.  The first index is the TRANSFER CHARACTER-SET
  number, the second index is the FILE CHARACTER-SET number.

  These arrays must be fully populated, even if (as is the case with Kanji
  character sets), all the entries are NULL.  Otherwise, subscript
  calculations will be wrong and we'll use the wrong functions.
*/

/* Pointers to byte-for-byte translation functions */

_PROTOTYP( CHAR (*rx), (CHAR) );
_PROTOTYP( CHAR (*sx), (CHAR) );

#ifdef UNICODE
_PROTOTYP( int (*xut), (USHORT) );	/* Translation function UCS to TCS */
_PROTOTYP( int (*xuf), (USHORT) );	/* Translation function UCS to FCS */
_PROTOTYP( USHORT (*xtu), (CHAR) );	/* Translation function TCS to UCS */
_PROTOTYP( USHORT (*xfu), (CHAR) );	/* Translation function FCS to UCS */
#endif /* UNICODE */

#ifdef CK_ANSIC
CHAR (*xlr[MAXTCSETS+1][MAXFCSETS+1])(CHAR) =
#else
CHAR (*xlr[MAXTCSETS+1][MAXFCSETS+1])() =
#endif /* CK_ANSIC */
{
    NULL,			/* 0,0 transparent to us ascii */
    NULL,			/* 0,1 transparent to uk ascii */
    NULL,			/* 0,2 transparent to dutch nrc */
    NULL,			/* 0,3 transparent to finnish nrc */
    NULL,			/* 0,4 transparent to french nrc */
    NULL,			/* 0,5 transparent to fr-canadian nrc */
    NULL,			/* 0,6 transparent to german nrc */
    NULL,			/* 0,7 transparent to hungarian nrc */
    NULL,			/* 0,8 transparent to italian nrc */
    NULL,			/* 0,9 transparent to norge/danish nrc */
    NULL,			/* 0,10 transparent to portuguese nrc */
    NULL,			/* 0,11 transparent to spanish nrc */
    NULL,			/* 0,12 transparent to swedish nrc */
    NULL,			/* 0,13 transparent to swiss nrc */
    NULL,			/* 0,14 transparent to latin-1 */
    NULL,			/* 0,15 transparent to latin-2 */
    NULL,			/* 0,16 transparent to DEC MCS */
    NULL,			/* 0,17 transparent to NeXT */
    NULL,			/* 0,18 transparent to CP437 */
    NULL,			/* 0,19 transparent to CP850 */
    NULL,			/* 0,20 transparent to CP852 */
    NULL,			/* 0,21 transparent to Macintosh Latin */
    NULL,			/* 0,22 transparent to DGI */
    NULL,			/* 0,23 transparent to HP */
    NULL,			/* 0,24 transparent to Latin/Cyrillic */
    NULL,                       /* 0,25 transparent to CP866 */
    NULL,			/* 0,26 transparent to Short KOI-7 */
    NULL,                       /* 0,27 transparent to Old KOI-8 Cyrillic */
    NULL,			/* 0,28 transparent to JIS-7 */
    NULL,			/* 0,29 transparent to Shift-JIS */
    NULL,			/* 0,30 transparent to J-EUC */
    NULL,			/* 0,31 transparent to DEC Kanji */
    NULL,			/* 0,32 transparent to Hebrew-7 */
    NULL,			/* 0,33 transparent to Latin/Hebrew */
    NULL,			/* 0,34 transparent to CP862 Hebrew */
    NULL,			/* 0,35 transparent to ELOT 927 Greek */
    NULL,			/* 0,36 transparent to Latin/Greek */
    NULL,			/* 0,37 transparent to CP869 */
    NULL,			/* 0,38 transparent to Latin-9 */
    NULL,			/* 0,39 transparent to CP858 */
    NULL,			/* 0,40 transparent to CP855 */
    NULL,			/* 0,41 transparent to CP1251 */
    NULL,			/* 0,42 transparent to Bulgarian */
    NULL,			/* 0,43 transparent to CP1250 */
    NULL,			/* 0,44 transparent to Mazovia */
    NULL,			/* 0,45 transparent to UCS-2 */
    NULL,			/* 0,46 transparent to UTF-8 */
    NULL,			/* 0,47 transparent to KOI8R */
    NULL,			/* 0,48 transparent to KOI8U */
    NULL,			/* 0,49 transparent to CP1252 */
    NULL,			/* 1,0 ascii to us ascii */
    NULL,			/* 1,1 ascii to uk ascii */
    NULL,			/* 1,2 ascii to dutch nrc */
    NULL,			/* 1,3 ascii to finnish nrc */
    NULL,			/* 1,4 ascii to french nrc */
    NULL,			/* 1,5 ascii to fr-canadian nrc */
    NULL,			/* 1,6 ascii to german nrc */
    NULL,			/* 1,7 ascii to hungarian nrc */
    NULL,			/* 1,8 ascii to italian nrc */
    NULL,			/* 1,9 ascii to norge/danish nrc */
    NULL,			/* 1,10 ascii to portuguese nrc */
    NULL,			/* 1,11 ascii to spanish nrc */
    NULL,			/* 1,12 ascii to swedish nrc */
    NULL,			/* 1,13 ascii to swiss nrc */
    NULL,			/* 1,14 ascii to latin-1 */
    NULL,			/* 1,15 ascii to latin-2 */
    NULL,			/* 1,16 ascii to DEC MCS */
    NULL,			/* 1,17 ascii to NeXT */
    NULL,			/* 1,18 ascii to CP437 */
    NULL,			/* 1,19 ascii to CP850 */
    NULL,			/* 1,20 ascii to CP852 */
    NULL,			/* 1,21 ascii to Macintosh Latin */
    NULL,			/* 1,22 ascii to DGI */
    NULL,			/* 1,23 ascii to HP */
    xaslc,                      /* 1,24 ascii to Latin/Cyrillic */
    xasac,                      /* 1,25 ascii to CP866 */
    xassk,                      /* 1,26 ascii to Short KOI */
    xask8,                      /* 1,27 ascii to Old KOI-8 Cyrillic */
    NULL,			/* 1,28 ascii to JIS-7 */
    NULL,			/* 1,29 ascii to Shift-JIS */
    NULL,			/* 1,30 ascii to J-EUC */
    NULL,			/* 1,31 ascii to DEC Kanji */
    xash7,			/* 1,32 ascii to Hebrew-7 */
    NULL,			/* 1,33 ascii to Latin/Hebrew */
    NULL,			/* 1,34 ascii to CP862 Hebrew */
    xaseg,			/* 1,35 ascii to ELOT 927 Greek */
    NULL,			/* 1,36 ascii to Latin/Greek */
    NULL,			/* 1,37 ascii to CP869 */
    NULL,			/* 1,38 ascii to Latin-9 */
    NULL,			/* 1,39 ascii to CP858 */
    xas55,			/* 1,40 ascii to CP855 */
    xas1251,			/* 1,41 ascii to CP1251 */
    xleft128,			/* 1,42 ascii to bulgarian */
    xleft128,			/* 1,43 ascii to CP1250 */
    xleft128,			/* 1,44 ascii to Mazovia */
    NULL,			/* 1,45 ascii to UCS-2 */
    NULL,			/* 1,46 ascii to UTF-8 */
    NULL,			/* 1,47 ascii to KOI8-R */
    NULL,			/* 1,48 ascii to KOI8-U */
    NULL,			/* 1,49 ascii to CP1252 */
    zl1as,			/* 2,0 latin-1 to us ascii */
    xl1uk,			/* 2,1 latin-1 to uk ascii */
    xl1du,			/* 2,2 latin-1 to dutch nrc */
    xl1fi,			/* 2,3 latin-1 to finnish nrc */
    xl1fr,			/* 2,4 latin-1 to french nrc */
    xl1fc,			/* 2,5 latin-1 to fr-canadian nrc */
    xl1ge,			/* 2,6 latin-1 to german nrc */
    xl1it,			/* 2,7 latin-1 to italian nrc */
    xl1hu,			/* 2,8 latin-1 to hungarian nrc */
    xl1no,			/* 2,9 latin-1 to norge/danish nrc */
    xl1po,			/* 2,10 latin-1 to portuguese nrc */
    xl1sp,			/* 2,11 latin-1 to spanish nrc */
    xl1sw,			/* 2,12 latin-1 to swedish nrc */
    xl1ch,			/* 2,13 latin-1 to swiss nrc */
    NULL,			/* 2,14 latin-1 to latin-1 */
    xl1l2,			/* 2,15 latin-1 to latin-2 */
    xl1dm,			/* 2,16 latin-1 to DEC MCS */
    xl1ne,			/* 2,17 latin-1 to NeXT */
    xl143,			/* 2,18 latin-1 to CP437 */
    xl185,			/* 2,19 latin-1 to CP850 */
    xl152,			/* 2,20 latin-1 to CP852 */
    xl1aq,			/* 2,21 latin-1 to Macintosh Latin */
    xl1dg,			/* 2,22 latin-1 to DGI */
    xl1r8,			/* 2,23 latin-1 to HP Roman8 */
    zl1as,			/* 2,24 latin-1 to Latin/Cyrillic */
    zl1as,                      /* 2,25 latin-1 to CP866 */
    xl1sk,                      /* 2,26 latin-1 to Short KOI */
    zl1as,		       	/* 2,27 latin-1 to Old KOI-8 Cyrillic */
    NULL,			/* 2,28 latin-1 to JIS-7 */
    NULL,			/* 2,29 latin-1 to Shift-JIS */
    NULL,			/* 2,30 latin-1 to J-EUC */
    NULL,			/* 2,31 latin-1 to DEC Kanji */
    xl1h7,			/* 2,32 latin-1 to Hebrew-7 */
    xl1lh,			/* 2,33 latin-1 to Latin/Hebrew */
    xl162,			/* 2,34 latin-1 to CP862 Hebrew */
    xl1eg,			/* 2,35 latin-1 to ELOT 927 Greek */
    xl1lg,			/* 2,36 latin-1 to Latin/Greek */
    xl169,			/* 2,37 latin-1 to CP869 */
    NULL,			/* 2,38 latin-1 to Latin9 */
    xl185,			/* 2,39 latin-1 to CP858 */
    zl1as,			/* 2,40 latin-1 to CP855 */
    zl1as,			/* 2,41 latin-1 to CP1251 */
    zl1as,			/* 2,42 latin-1 to Bulgarian */
    xl11250,			/* 2,43 latin-1 to CP1250 */
    xl1mz,			/* 2,44 latin-1 to Mazovia */
    NULL,			/* 2,45 latin-1 to UCS-2 */
    NULL,			/* 2,46 latin-1 to UTF-8 */
    zl1as,			/* 2,47 latin-1 to KOI8R */
    zl1as,			/* 2,48 latin-1 to KOI8U */
    xl1w1,			/* 2,49 latin-1 to CP1252 */
    xl2as,			/* 3,0 latin-2 to us ascii */
    xl2as,			/* 3,1 latin-2 to uk ascii */
    xl2as,			/* 3,2 latin-2 to dutch nrc */
    xl2as,			/* 3,3 latin-2 to finnish nrc */
    xl2as,			/* 3,4 latin-2 to french nrc */
    xl2as,			/* 3,5 latin-2 to fr-canadian nrc */
    xl2as,			/* 3,6 latin-2 to german nrc */
    xl2as,			/* 3,7 latin-2 to italian nrc */
    xl2as,			/* 3,8 latin-2 to hungarian nrc */
    xl2as,			/* 3,9 latin-2 to norge/danish nrc */
    xl2as,			/* 3,10 latin-2 to portuguese nrc */
    xl2as,			/* 3,11 latin-2 to spanish nrc */
    xl2as,			/* 3,12 latin-2 to swedish nrc */
    xl2as,			/* 3,13 latin-2 to swiss nrc */
    xl2l1,			/* 3,14 latin-2 to latin-1 */
    NULL,			/* 3,15 latin-2 to latin-2 */
    xl2l1,			/* 3,16 latin-2 to DEC MCS */
    xl2ne,			/* 3,17 latin-2 to NeXT */
    xl243,			/* 3,18 latin-2 to CP437 */
    xl285,			/* 3,19 latin-2 to CP850 */
    xl252,			/* 3,20 latin-2 to CP852 */
    xl2aq,			/* 3,21 latin-2 to Macintosh Latin */
    xl2dg,			/* 3,22 latin-2 to DGI */
    xl2r8,			/* 3,23 latin-2 to HP */
    xl2as,			/* 3,24 latin-2 to Latin/Cyrillic */
    xl2as,                      /* 3,25 latin-2 to CP866 */
    xl2sk,                      /* 3,26 latin-2 to Short KOI */
    xl2as,		       	/* 3,27 latin-2 to Old KOI-8 Cyrillic */
    NULL,			/* 3,28 latin-2 to JIS-7 */
    NULL,			/* 3,29 latin-2 to Shift-JIS */
    NULL,			/* 3,30 latin-2 to J-EUC */
    NULL,			/* 3,31 latin-2 to DEC Kanji */
    xl2h7,			/* 3,32 latin-2 to Hebrew-7 */
    xl2as,			/* 3,33 latin-2 to Latin/Hebrew */
    xl2as,			/* 3,34 latin-2 to CP862 Hebrew */
    xassk,			/* 3,35 latin-2 to ELOT 927 Greek */
    xl2as,			/* 3,36 latin-2 to Latin/Greek */
    xl2as,			/* 3,37 latin-2 to CP869 */
    xl2l9,			/* 3,38 latin-2 to Latin-9 */
    xl258,			/* 3,39 latin-2 to CP858 */
    xl2as,			/* 3,40 latin-2 to CP855 */
    xl2as,			/* 3,41 latin-2 to CP1251 */
    xl2as,			/* 3,42 latin-2 to Bulgarian */
    xl21250,			/* 3,43 latin-2 to CP1250 */
    xl2mz,			/* 3,44 latin-2 to Mazovia */
    NULL,			/* 3,45 latin-2 to UCS-2 */
    NULL,			/* 3,46 latin-2 to UTF-8 */
    xl2as,			/* 3,47 latin-2 to KOI8R */
    xl2as,			/* 3,48 latin-2 to KOI8U */
    xl2w1,			/* 3,49 latin-2 to CP1252 */
    xlcas,			/* 4,0 latin/cyrillic to us ascii */
    xlcas,			/* 4,1 latin/cyrillic to uk ascii */
    xlcas, 		        /* 4,2 latin/cyrillic to dutch nrc */
    xlcas,			/* 4,3 latin/cyrillic to finnish ascii */
    xlcas,			/* 4,4 latin/cyrillic to french nrc */
    xlcas,			/* 4,5 latin/cyrillic to fr-canadian nrc */
    xlcas,			/* 4,6 latin/cyrillic to german nrc */
    xlcas,			/* 4,7 latin/cyrillic to italian nrc */
    xlcas,			/* 4,8 latin/cyrillic to hungarian nrc */
    xlcas,			/* 4,9 latin/cyrillic to norge/danish nrc */
    xlcas,			/* 4,10 latin/cyrillic to portuguese nrc */
    xlcas,			/* 4,11 latin/cyrillic to spanish nrc */
    xlcas,			/* 4,12 latin/cyrillic to swedish nrc */
    xlcas,			/* 4,13 latin/cyrillic to swiss nrc */
    xlcas,			/* 4,14 latin/cyrillic to latin-1 */
    xlcas,			/* 4,15 latin/cyrillic to latin-2 */
    xlcas,			/* 4,16 latin/cyrillic to DEC MCS */
    xlcas,			/* 4,17 latin/cyrillic to NeXT */
    xlcas,			/* 4,18 latin/cyrillic to CP437 */
    xlcas,			/* 4,19 latin/cyrillic to CP850 */
    xlcas,			/* 4,20 latin/cyrillic to CP852 */
    xlcas,			/* 4,21 latin/cyrillic to Macintosh Latin */
    xlcas,			/* 4,22 latin/cyrillic to DGI */
    xlcas,			/* 4,23 latin/cyrillic to HP */
    NULL,                       /* 4,24 latin/cyrillic to Latin/Cyrillic */
    xlcac,                      /* 4,25 latin/cyrillic to CP866 */
    xlcsk,                      /* 4,26 latin/cyrillic to Short KOI */
    xlck8,		       	/* 4,27 latin/cyrillic to Old KOI-8 Cyrillic */
    NULL,			/* 4,28 latin/cyril to JIS-7 */
    NULL,			/* 4,29 latin/cyril to Shift-JIS */
    NULL,			/* 4,30 latin/cyril to J-EUC */
    NULL,			/* 4,31 latin/cyril to DEC Kanji */
    xlch7,			/* 4,32 latin/cyril to Hebrew-7 */
    xlcas,			/* 4,33 latin/cyril to Latin/Hebrew */
    xlcas,			/* 4,34 latin/cyril to CP862 Hebrew */
    xassk,			/* 4,35 latin/cyril to ELOT 927 Greek */
    xleft160,			/* 4,36 latin/cyril to Latin/Greek */
    xleft128,			/* 4,37 latin/cyril to CP869 */
    xlcas,			/* 4,38 latin/cyril to latin-9 */
    xlcas,			/* 4,39 latin/cyril to CP858 */
    xlc55,			/* 4,40 latin/cyril to CP855 */
    xlc1251,			/* 4,41 latin/cyril to CP1251 */
    xlcbu,			/* 4,42 latin/cyril to Bulgarian */
    xlcas,			/* 4,43 latin/cyril to CP1250 */
    xlcas,			/* 4,44 latin/cyril to Mazovia */
    NULL,			/* 4,45 latin/cyril to UCS-2 */
    NULL,			/* 4,46 latin/cyril to UTF-8 */
    xlckr,			/* 4,47 latin/cyril to KOI8R */
    xlcku,			/* 4,48 latin/cyril to KOI8U */
    xlcas,			/* 4,49 latin/cyril to CP1252 */
    NULL,			/* 5,00 */
    NULL,			/* 5,01 */
    NULL,			/* 5,02 */
    NULL,			/* 5,03 */
    NULL,			/* 5,04 */
    NULL,			/* 5,05 */
    NULL,			/* 5,06 */
    NULL,			/* 5,07 */
    NULL,			/* 5,08 */
    NULL,			/* 5,09 */
    NULL,			/* 5,10 */
    NULL,			/* 5,11 */
    NULL,			/* 5,12 */
    NULL,			/* 5,13 */
    NULL,			/* 5,14 */
    NULL,			/* 5,15 */
    NULL,			/* 5,16 */
    NULL,			/* 5,17 */
    NULL,			/* 5,18 */
    NULL,			/* 5,19 */
    NULL,			/* 5,20 */
    NULL,			/* 5,21 */
    NULL,			/* 5,22 */
    NULL,			/* 5,23 */
    NULL,			/* 5,24 */
    NULL,			/* 5,25 */
    NULL,			/* 5,26 */
    NULL,			/* 5,27 */
    NULL,			/* 5,28 */
    NULL,			/* 5,29 */
    NULL,			/* 5,30 */
    NULL,			/* 5,31 */
    NULL,			/* 5,32 */
    NULL,			/* 5,33 */
    NULL,			/* 5,34 */
    NULL,			/* 5,35 */
    NULL,			/* 5,36 */
    NULL,			/* 5,37 */
    NULL,			/* 5,38 */
    NULL,			/* 5,39 */
    NULL,			/* 5,40 */
    NULL,			/* 5,41 */
    NULL,			/* 5,42 */
    NULL,			/* 5,43 */
    NULL,			/* 5,44 */
    NULL,			/* 5,45 */
    NULL,			/* 5,46 */
    NULL,			/* 5,47 */
    NULL,			/* 5,48 */
    NULL,			/* 5,49 */
    xlhas,			/* 6,0 latin/hebrew to us ascii */
    xlhas,			/* 6,1 latin/hebrew to uk ascii */
    xlhas, 		        /* 6,2 latin/hebrew to dutch nrc */
    xlhas,			/* 6,3 latin/hebrew to finnish ascii */
    xlhas,			/* 6,4 latin/hebrew to french nrc */
    xlhas,			/* 6,5 latin/hebrew to fr-canadian nrc */
    xlhas,			/* 6,6 latin/hebrew to german nrc */
    xlhas,			/* 6,7 latin/hebrew to italian nrc */
    xlhas,			/* 6,8 latin/hebrew to hungarian nrc */
    xlhas,			/* 6,9 latin/hebrew to norge/danish nrc */
    xlhas,			/* 6,10 latin/hebrew to portuguese nrc */
    xlhas,			/* 6,11 latin/hebrew to spanish nrc */
    xlhas,			/* 6,12 latin/hebrew to swedish nrc */
    xlhas,			/* 6,13 latin/hebrew to swiss nrc */
    xlhl1,			/* 6,14 latin/hebrew to latin-1 */
    xlhas,			/* 6,15 latin/hebrew to latin-2 */
    xlhl1,			/* 6,16 latin/hebrew to DEC MCS */
    xlhas,			/* 6,17 latin/hebrew to NeXT */
    xlhas,			/* 6,18 latin/hebrew to CP437 */
    xlhas,			/* 6,19 latin/hebrew to CP850 */
    xlhas,			/* 6,20 latin/hebrew to CP852 */
    xlhas,			/* 6,21 latin/hebrew to Macintosh Latin */
    xlhas,			/* 6,22 latin/hebrew to DGI */
    xlhas,                      /* 6,23 latin/hebrew to HP */
    xlhas,                      /* 6,24 latin/hebrew to Latin/Cyrillic */
    xlhas,                      /* 6,25 latin/hebrew to CP866 */
    NULL,                       /* 6,26 latin/hebrew to Short KOI */
    xlhas,		       	/* 6,27 latin/hebrew to Old KOI-8 Cyrillic */
    NULL,			/* 6,28 latin/hebrew to JIS-7 */
    NULL,			/* 6,29 latin/hebrew to Shift-JIS */
    NULL,			/* 6,30 latin/hebrew to J-EUC */
    NULL,			/* 6,31 latin/hebrew to DEC Kanji */
    xlhh7,			/* 6,32 latin/hebrew to Hebrew-7 */
    NULL,			/* 6,33 latin/hebrew to Latin/Hebrew */
    xlh62,			/* 6,34 latin/hebrew to CP862 Hebrew */
    NULL,			/* 6,35 latin/hebrew to ELOT 927 Greek */
    xlhas,			/* 6,36 latin/hebrew to Latin/Greek */
    xlhas,			/* 6,37 latin/hebrew to CP869 */
    xlhas,			/* 6,38 latin/hebrew to Latin-9 */
    xlhas,			/* 6,39 latin/hebrew to CP858 */
    xlhas,			/* 6,40 latin/hebrew to CP855 */
    xlhas,			/* 6,41 latin/hebrew to CP1251 */
    xlhas,			/* 6,42 latin/hebrew to Bulgarian */
    xlhas,			/* 6,43 latin/hebrew to CP1250 */
    xlhas,			/* 6,44 latin/hebrew to Mazovia */
    NULL,			/* 6,45 latin/hebrew to UCS-2 */
    NULL,			/* 6,46 latin/hebrew to UTF-8 */
    NULL,			/* 6,47 latin/hebrew to KOI8R */
    NULL,			/* 6,48 latin/hebrew to KOI8U */
    xlhw1,			/* 6,49 latin/hebrew to CP1252 */
    xlgas,			/* 7,0 latin/greek to us ascii */
    xlgas,			/* 7,1 latin/greek to uk ascii */
    xlgas, 		        /* 7,2 latin/greek to dutch nrc */
    xlgas,			/* 7,3 latin/greek to finnish ascii */
    xlgas,			/* 7,4 latin/greek to french nrc */
    xlgas,			/* 7,5 latin/greek to fr-canadian nrc */
    xlgas,			/* 7,6 latin/greek to german nrc */
    xlgas,			/* 7,7 latin/greek to italian nrc */
    xlgas,			/* 7,8 latin/greek to hungarian nrc */
    xlgas,			/* 7,9 latin/greek to norge/danish nrc */
    xlgas,			/* 7,10 latin/greek to portuguese nrc */
    xlgas,			/* 7,11 latin/greek to spanish nrc */
    xlgas,			/* 7,12 latin/greek to swedish nrc */
    xlgas,			/* 7,13 latin/greek to swiss nrc */
    xlgas,			/* 7,14 latin/greek to latin-1 */
    xlgas,			/* 7,15 latin/greek to latin-2 */
    xlgas,			/* 7,16 latin/greek to DEC MCS */
    xlgas,			/* 7,17 latin/greek to NeXT */
    xlgas,			/* 7,18 latin/greek to CP437 */
    xlgas,			/* 7,19 latin/greek to CP850 */
    xlgas,			/* 7,20 latin/greek to CP852 */
    xlgas,			/* 7,21 latin/greek to Macintosh Latin */
    xlgas,			/* 7,22 latin/greek to DGI */
    xlgas,			/* 7,23 latin/greek to HP */
    xleft160,                   /* 7,24 latin/greek to Latin/Cyrillic */
    xleft128,                   /* 7,25 latin/greek to CP866 */
    xassk,                      /* 7,26 latin/greek to Short KOI */
    xleft160,		       	/* 7,27 latin/greek to Old KOI-8 Greek */
    NULL,			/* 7,28 latin/greek to JIS-7 */
    NULL,			/* 7,29 latin/greek to Shift-JIS */
    NULL,			/* 7,30 latin/greek to J-EUC */
    NULL,			/* 7,31 latin/greek to DEC Kanji */
    NULL,			/* 7,32 latin/greek to Hebrew-7 */
    xlgas,			/* 7,33 latin/greek to Latin/Hebrew */
    xlgas,			/* 7,34 latin/greek to CP862 Hebrew */
    xlgeg,			/* 7,35 latin/greek to ELOT 927 Greek */
    NULL,			/* 7,36 latin/greek to Latin/Greek */
    xlg69,			/* 7,37 latin/greek to CP869 */
    xlgas,			/* 7,38 latin/greek to Latin-9 */
    xlgas,			/* 7,39 latin/greek to CP858 */
    xleft128,			/* 7,40 latin/greek to CP855 */
    xleft128,			/* 7,41 latin/greek to CP1251 */
    xleft128,			/* 7,42 latin/greek to Bulgarian */
    xleft128,			/* 7,43 latin/greek to CP1250 */
    xleft128,			/* 7,44 latin/greek to Mazovia */
    NULL,			/* 7,45 latin/greek to UCS-2 */
    NULL,			/* 7,46 latin/greek to UTF-8 */
    NULL,			/* 7,47 latin/greek to KOI8R */
    NULL,			/* 7,48 latin/greek to KOI8U */
    xlgw1,			/* 7,49 latin/greek to CP1252 */
    zl9as,			/* 8,0 latin-9 to us ascii */
    xl1uk,			/* 8,1 latin-9 to uk ascii */
    xl1du,			/* 8,2 latin-9 to dutch nrc */
    xl1fi,			/* 8,3 latin-9 to finnish nrc */
    xl1fr,			/* 8,4 latin-9 to french nrc */
    xl1fc,			/* 8,5 latin-9 to fr-canadian nrc */
    xl1ge,			/* 8,6 latin-9 to german nrc */
    xl1it,			/* 8,7 latin-9 to italian nrc */
    xl1hu,			/* 8,8 latin-9 to hungarian nrc */
    xl1no,			/* 8,9 latin-9 to norge/danish nrc */
    xl1po,			/* 8,10 latin-9 to portuguese nrc */
    xl1sp,			/* 8,11 latin-9 to spanish nrc */
    xl1sw,			/* 8,12 latin-9 to swedish nrc */
    xl1ch,			/* 8,13 latin-9 to swiss nrc */
    NULL,			/* 8,14 latin-9 to latin-1 */
    xl1l2,			/* 8,15 latin-9 to latin-2 */
    xl9dm,			/* 8,16 latin-9 to DEC MCS */
    xl9ne,			/* 8,17 latin-9 to NeXT */
    xl143,			/* 8,18 latin-9 to CP437 */
    xl185,			/* 8,19 latin-9 to CP850 */
    xl152,			/* 8,20 latin-9 to CP852 */
    xl1aq,			/* 8,21 latin-9 to Macintosh Latin */
    xl1dg,			/* 8,22 latin-9 to DGI */
    xl1r8,			/* 8,23 latin-9 to HP Roman8 */
    zl1as,			/* 8,24 latin-9 to Latin/Cyrillic */
    zl1as,                      /* 8,25 latin-9 to CP866 */
    xl1sk,                      /* 8,26 latin-9 to Short KOI */
    zl1as,		       	/* 8,27 latin-9 to Old KOI-8 Cyrillic */
    NULL,			/* 8,28 latin-9 to JIS-7 */
    NULL,			/* 8,29 latin-9 to Shift-JIS */
    NULL,			/* 8,30 latin-9 to J-EUC */
    NULL,			/* 8,31 latin-9 to DEC Kanji */
    xl1h7,			/* 8,32 latin-9 to Hebrew-7 */
    xl1lh,			/* 8,33 latin-9 to Latin/Hebrew */
    xl162,			/* 8,34 latin-9 to CP862 Hebrew */
    xl1eg,			/* 8,35 latin-9 to ELOT 927 Greek */
    xl1lg,			/* 8,36 latin-9 to Latin/Greek */
    xl169,			/* 8,37 latin-9 to CP869 */
    NULL,			/* 8,38 latin-9 to Latin9 */
    xl958,			/* 8,39 latin-9 to CP858 */
    zl1as,			/* 8,40 latin-9 to CP855 */
    zl1as,			/* 8,41 latin-9 to CP1251 */
    xl1as,			/* 8,42 latin-9 to Bulgarian */
    xl91250,			/* 8,43 latin-9 to CP1250 */
    xl9mz,			/* 8,44 latin-9 to Mazovia */
    NULL,			/* 8,45 latin-9 to UCS-2 */
    NULL,			/* 8,46 latin-9 to UTF-8 */
    zl1as,			/* 8,47 latin-9 to KOI8-R */
    zl1as,			/* 8,48 latin-9 to KOI8-U */
    xl9w1,			/* 8,49 latin-9 to CP1252 */
    NULL,			/* 9,00 Unicode... */
    NULL,			/* 9,01 */
    NULL,			/* 9,02 */
    NULL,			/* 9,03 */
    NULL,			/* 9,04 */
    NULL,			/* 9,05 */
    NULL,			/* 9,06 */
    NULL,			/* 9,07 */
    NULL,			/* 9,08 */
    NULL,			/* 9,09 */
    NULL,			/* 9,10 */
    NULL,			/* 9,11 */
    NULL,			/* 9,12 */
    NULL,			/* 9,13 */
    NULL,			/* 9,14 */
    NULL,			/* 9,15 */
    NULL,			/* 9,16 */
    NULL,			/* 9,17 */
    NULL,			/* 9,18 */
    NULL,			/* 9,19 */
    NULL,			/* 9,20 */
    NULL,			/* 9,21 */
    NULL,			/* 9,22 */
    NULL,			/* 9,23 */
    NULL,			/* 9,24 */
    NULL,			/* 9,25 */
    NULL,			/* 9,26 */
    NULL,			/* 9,27 */
    NULL,			/* 9,28 */
    NULL,			/* 9,29 */
    NULL,			/* 9,30 */
    NULL,			/* 9,31 */
    NULL,			/* 9,32 */
    NULL,			/* 9,33 */
    NULL,			/* 9,34 */
    NULL,			/* 9,35 */
    NULL,			/* 9,36 */
    NULL,			/* 9,37 */
    NULL,			/* 9,38 */
    NULL,			/* 9,39 */
    NULL,			/* 9,40 */
    NULL,			/* 9,41 */
    NULL,			/* 9,42 */
    NULL,			/* 9,43 */
    NULL,			/* 9,44 */
    NULL,			/* 9,45 */
    NULL,			/* 9,46 */
    NULL,			/* 9,47 */
    NULL,			/* 9,48 */
    NULL,			/* 9,49 */
    NULL,			/* 10,00 */
    NULL,			/* 10,01 */
    NULL,			/* 10,02 */
    NULL,			/* 10,03 */
    NULL,			/* 10,04 */
    NULL,			/* 10,05 */
    NULL,			/* 10,06 */
    NULL,			/* 10,07 */
    NULL,			/* 10,08 */
    NULL,			/* 10,09 */
    NULL,			/* 10,10 */
    NULL,			/* 10,11 */
    NULL,			/* 10,12 */
    NULL,			/* 10,13 */
    NULL,			/* 10,14 */
    NULL,			/* 10,15 */
    NULL,			/* 10,16 */
    NULL,			/* 10,17 */
    NULL,			/* 10,18 */
    NULL,			/* 10,19 */
    NULL,			/* 10,20 */
    NULL,			/* 10,21 */
    NULL,			/* 10,22 */
    NULL,			/* 10,23 */
    NULL,			/* 10,24 */
    NULL,			/* 10,25 */
    NULL,			/* 10,26 */
    NULL,			/* 10,27 */
    NULL,			/* 10,28 */
    NULL,			/* 10,29 */
    NULL,			/* 10,30 */
    NULL,			/* 10,31 */
    NULL,			/* 10,32 */
    NULL,			/* 10,33 */
    NULL,			/* 10,34 */
    NULL,			/* 10,35 */
    NULL,			/* 10,36 */
    NULL,			/* 10,37 */
    NULL,			/* 10,38 */
    NULL,			/* 10,39 */
    NULL,			/* 10,40 */
    NULL,			/* 10,41 */
    NULL,			/* 10,42 */
    NULL,			/* 10,43 */
    NULL,			/* 10,44 */
    NULL,			/* 10,45 */
    NULL,			/* 10,46 */
    NULL,			/* 10,47 */
    NULL,			/* 10,48 */
    NULL			/* 10,49 */
};
int nxlr = (sizeof(xlr) / sizeof(CHAR *));

/*
  Translation function table for sending files.
  Array of pointers to functions for translating from the local file
  character set to the transfer character set.  Indexed in the same
  way as the xlr array above, but with the indices reversed.
*/
#ifdef CK_ANSIC
CHAR (*xls[MAXTCSETS+1][MAXFCSETS+1])(CHAR) =
#else
CHAR (*xls[MAXTCSETS+1][MAXFCSETS+1])() =
#endif /* CK_ANSIC */
{
    NULL,			/* 0,0 us ascii to transparent */
    NULL,			/* 0,1 uk ascii to transparent */
    NULL,			/* 0,2 dutch nrc to transparent */
    NULL,			/* 0,3 finnish nrc to transparent */
    NULL,			/* 0,4 french nrc to transparent */
    NULL,			/* 0,5 fr-canadian nrc to transparent */
    NULL,			/* 0,6 german nrc to transparent */
    NULL,			/* 0,7 hungarian nrc to transparent */
    NULL,			/* 0,8 italian nrc to transparent */
    NULL,			/* 0,9 norge/danish nrc to transparent */
    NULL,			/* 0,10 portuguese nrc to transparent */
    NULL,			/* 0,11 spanish nrc to transparent */
    NULL,			/* 0,12 swedish nrc to transparent */
    NULL,			/* 0,13 swiss nrc to transparent */
    NULL,			/* 0,14 latin-1 to transparent */
    NULL,			/* 0,15 latin-2 to transparent */
    NULL,			/* 0,16 DEC MCS to transparent */
    NULL,			/* 0,17 NeXT to transparent */
    NULL,			/* 0,18 CP437 to transparent */
    NULL,			/* 0,19 CP850 to transparent */
    NULL,			/* 0,20 CP852 to transparent */
    NULL,			/* 0,21 Macintosh Latin to transparent */
    NULL,			/* 0,22 DGI to transparent */
    NULL,			/* 0,23 HP to transparent */
    NULL,			/* 0,24 Latin/Cyrillic to transparent */
    NULL,                       /* 0,25 CP866 to transparent */
    NULL,                       /* 0,26 Short KOI to transparent */
    NULL,                       /* 0,27 Old KOI-8 to transparent */
    NULL,			/* 0,28 JIS-7 to transparent */
    NULL,			/* 0,29 Shift JIS to transparent */
    NULL,			/* 0,30 Japanese EUC to transparent */
    NULL,			/* 0,31 DEC Kanji to transparent */
    NULL,			/* 0,32 Hebrew-7 to transparent */
    NULL,			/* 0,33 Latin/Hebrew to transparent */
    NULL,			/* 0,34 CP862 Hebrew to transparent */
    NULL,			/* 0,35 ELOT 927 Greek to transparent */
    NULL,			/* 0,36 Latin/Greek to transparent */
    NULL,			/* 0,37 CP869 to transparent */
    NULL,			/* 0,38 Latin-9 to transparent */
    NULL,			/* 0,39 CP858 to transparent */
    NULL,			/* 0,40 CP855 to transparent */
    NULL,			/* 0,41 CP1251 to transparent */
    NULL,			/* 0,42 Bulgarian to transparent */
    NULL,			/* 0,43 CP1250 to transparent */
    NULL,			/* 0,44 Mazovia to transparent */
    NULL,			/* 0,45 UCS-2 to transparent */
    NULL,			/* 0,46 UTF-8 to transparent */
    NULL,			/* 0,47 KOI8R to transparent */
    NULL,			/* 0,48 KOI8U to transparent */
    NULL,			/* 0,49 CP1252 to transparent */
    NULL,			/* 1,0 us ascii to ascii */
    NULL,			/* 1,1 uk ascii to ascii */
    xduas,			/* 1,2 dutch nrc to ascii */
    xfias,			/* 1,3 finnish nrc to ascii */
    xfras,			/* 1,4 french nrc to ascii */
    xfcas,			/* 1,5 french canadian nrc to ascii */
    xgeas,			/* 1,6 german nrc to ascii */
    xhuas,			/* 1,7 hungarian nrc to ascii */
    xitas,			/* 1,8 italian nrc to ascii */
    xnoas,			/* 1,9 norwegian/danish nrc to ascii */
    xpoas,			/* 1,10 portuguese nrc to ascii */
    xspas,			/* 1,11 spanish nrc to ascii */
    xswas,			/* 1,12 swedish nrc to ascii */
    xchas,			/* 1,13 swiss nrc to ascii */
    xl1as,			/* 1,14 latin-1 to ascii */
    xl2as,			/* 1,15 latin-2 to ascii */
    xdmas,			/* 1,16 dec mcs to ascii */
    xneas,			/* 1,17 NeXT to ascii */
    x43as,			/* 1,18 CP437 to ascii */
    x85as,			/* 1,19 CP850 to ascii */
    x52as,			/* 1,20 CP850 to ascii */
    xaqas,			/* 1,21 Macintosh Latin to ascii */
    xdgas,			/* 1,22 DGI to ascii */
    xr8as,			/* 1,23 HP to ASCII */
    xlcas,			/* 1,24 Latin/Cyrillic to ASCII */
    xacas,                      /* 1,25 CP866 to ASCII */
    xskas,			/* 1,26 Short KOI to ASCII */
    xk8as,                      /* 1,27 Old KOI-8 Cyrillic to ASCII */
    NULL,			/* 1,28 */
    NULL,			/* 1,29 */
    NULL,			/* 1,30 */
    NULL,			/* 1,31 */
    xh7as,			/* 1,32 Hebrew-7 to ASCII */
    xlhas,			/* 1,33 Latin/Hebrew to ASCII */
    x62as,			/* 1,34 CP862 Hebrew to ASCII */
    xegas,			/* 1,35 ELOT 927 Greek to ASCII */
    xlgas,			/* 1,36 Latin/Greek to ASCII */
    x69as,			/* 1,37 CP869 to ASCII */
    xl9as,			/* 1,38 Latin-9 to ASCII */
    x58as,			/* 1,39 CP858 to ASCII */
    x55as,			/* 1,40 CP855 to ASCII */
    x1251as,			/* 1,41 CP1251 to ASCII */
    xleft128,			/* 1,42 Bulgarian to ASCII */
    x1250as,			/* 1,43 CP1250 to ASCII */
    xmzas,			/* 1,44 Mazovia to ASCII */
    NULL,			/* 1,45 UCS-2 to ASCII */
    NULL,			/* 1,46 UTF-8 to ASCII */
    xk8as,			/* 1,47 KOI8R to ASCII */
    xk8as,			/* 1,48 KOI8U to ASCII */
    xw1as,			/* 1,49 CP1252 to ASCII */
    NULL,			/* 2,0 us ascii to latin-1 */
    xukl1,			/* 2,1 uk ascii to latin-1 */
    xdul1,			/* 2,2 dutch nrc to latin-1 */
    xfil1,			/* 2,3 finnish nrc to latin-1 */
    xfrl1,			/* 2,4 french nrc to latin-1 */
    xfcl1,			/* 2,5 french canadian nrc to latin-1 */
    xgel1,			/* 2,6 german nrc to latin-1 */
    xhul1,			/* 2,7 hungarian nrc to latin-1 */
    xitl1,			/* 2,8 italian nrc to latin-1 */
    xnol1,			/* 2,9 norwegian/danish nrc to latin-1 */
    xpol1,			/* 2,10 portuguese nrc to latin-1 */
    xspl1,			/* 2,11 spanish nrc to latin-1 */
    xswl1,			/* 2,12 swedish nrc to latin-1 */
    xchl1,			/* 2,13 swiss nrc to latin-1 */
    NULL,			/* 2,14 latin-1 to latin-1 */
    xl2l1,			/* 2,15 latin-2 to latin-1 */
    xdml1,			/* 2,16 dec mcs to latin-1 */
    xnel1,                      /* 2,17 NeXT to Latin-1 */
    x43l1,                      /* 2,18 CP437 to Latin-1 */
    x85l1,                      /* 2,19 CP850 to Latin-1 */
    x52l1,                      /* 2,20 CP852 to Latin-1 */
    xaql1,                      /* 2,21 Macintosh Latin to Latin-1 */
    xdgl1,                      /* 2,22 DGI to Latin-1 */
    xr8l1,                      /* 2,23 HP to Latin-1 */
    xlcas,                      /* 2,24 Latin/Cyrillic to Latin-1 */
    xacas,                      /* 2,25 CP866 to Latin-1 */
    xskas,                      /* 2,26 Short KOI to Latin-1 */
    xk8as,                      /* 2,27 Old KOI-8 Cyrillic to Latin-1 */
    NULL,			/* 2,28 Kanji ... */
    NULL,			/* 2,29 */
    NULL,			/* 2,30 */
    NULL,			/* 2,31 */
    xh7as,			/* 2,32 Hebrew-7 to Latin-1 */
    xlhl1,			/* 2,33 Latin/Hebrew to Latin-1 */
    x62l1,			/* 2,34 CP862 Hebrew to Latin-1 */
    xegas,			/* 2,35 ELOT 927 Greek to Latin-1 */
    xlgl1,			/* 2,36 Latin/Greek to Latin-1 */
    NULL,			/* 2,37 CP869 to Latin-1 */
    NULL,			/* 2,38 Latin-9 to Latin-1 */
    x58l1,			/* 2,39 CP858 to Latin-1 */
    x55as,			/* 2,40 CP855 to Latin-1 */
    x1251as,			/* 2,41 CP1251 to Latin-1 */
    xleft128,			/* 2,42 Bulgarian to Latin-1 */
    x1250l1,			/* 2,43 CP1250 to Latin-1 */
    xmzl1,			/* 2,44 Mazovia to Latin-1 */
    NULL,			/* 2,45 UCS-2 to Latin-1 */
    NULL,			/* 2,46 UTF-8 to Latin-1 */
    xk8as,			/* 2,47 KOI8R to Latin-1 */
    xk8as,			/* 2,48 KOI8U to Latin-1 */
    xw1l1,			/* 2,49 CP1252 to Latin-1 */
    NULL,			/* 3,0 us ascii to latin-2 */
    NULL,			/* 3,1 uk ascii to latin-2 */
    xduas,			/* 3,2 dutch nrc to latin-2 */
    xfias,			/* 3,3 finnish nrc to latin-2 */
    xfras,			/* 3,4 french nrc to latin-2 */
    xfcas,			/* 3,5 french canadian nrc to latin-2 */
    xgel2,			/* 3,6 german nrc to latin-2 */
    xhul2,			/* 3,7 hungarian nrc to latin-2 */
    xitas,			/* 3,8 italian nrc to latin-2 */
    xnoas,			/* 3,9 norwegian/danish nrc to latin-2 */
    xpoas,			/* 3,10 portuguese nrc to latin-2 */
    xspas,			/* 3,11 spanish nrc to latin-2 */
    xswas,			/* 3,12 swedish nrc to latin-2 */
    xchas,			/* 3,13 swiss nrc to latin-2 */
    xl1l2,			/* 3,14 latin-1 to latin-2 */
    NULL,			/* 3,15 latin-2 to latin-2 */
    xl1l2,			/* 3,16 dec mcs to latin-2 */
    xnel2,                      /* 3,17 NeXT to Latin-2 */
    x43l2,                      /* 3,18 CP437 to Latin-2 */
    x85l2,                      /* 3,19 CP850 to Latin-2 */
    x52l2,                      /* 3,20 CP852 to Latin-2 */
    xaql2,                      /* 3,21 Macintosh Latin to Latin-2 */
    xdgl2,                      /* 3,22 DGI to Latin-2 */
    xr8l2,                      /* 3,23 HP to Latin-2 */
    xlcas,                      /* 3,24 Latin/Cyrillic to Latin-2 */
    xacas,                      /* 3,25 CP866 to Latin-2 */
    xskas,                      /* 3,26 Short KOI to Latin-2 */
    xk8as,                      /* 3,27 Old KOI-8 Cyrillic to Latin-2 */
    NULL,			/* 3,28 Kanji ... */
    NULL,			/* 3,29 */
    NULL,			/* 3,30 */
    NULL,			/* 3,31 */
    xh7as,			/* 3,32 Hebrew-7 to Latin-2 */
    xlhas,			/* 3,33 Latin/Hebrew to Latin-2 */
    x62as,			/* 3,34 CP862 Hebrew to Latin-2 */
    xegas,			/* 3,35 ELOT 927 Greek to Latin-2 */
    xl2lg,			/* 3,36 Latin/Greek to Latin-2 */
    xleft128,			/* 3,37 CP869 to Latin-2 */
    xl9l2,			/* 3,38 Latin-9 to Latin-2 */
    x58l2,			/* 3,39 CP858 to Latin-2 */
    x55as,			/* 3,40 CP855 to Latin-2 */
    x1251as,			/* 3,41 CP1251 to Latin-2 */
    xleft128,			/* 3,42 Bulgarian to Latin-2 */
    x1250l2,			/* 3,43 CP1250 to Latin-2 */
    xmzl2,			/* 3,44 Mazovia to Latin-2 */
    NULL,			/* 3,45 UCS-2 to Latin-2 */
    NULL,			/* 3,46 UTF-8 to Latin-2 */
    xk8as,			/* 3,47 KOI8R to Latin-2 */
    xk8as,			/* 3,48 KOI8U to Latin-2 */
    xw1l2,			/* 3,49 CP1252 to latin-2 */
    xaslc,			/* 4,0 us ascii to latin/cyrillic */
    xaslc,			/* 4,1 uk ascii to latin/cyrillic */
    xduas,			/* 4,2 dutch nrc to latin/cyrillic */
    xfias,			/* 4,3 finnish nrc to latin/cyrillic */
    xfras,			/* 4,4 french nrc to latin/cyrillic */
    xfcas,			/* 4,5 french canadian nrc to latin/cyrillic */
    xgeas,			/* 4,6 german nrc to latin/cyrillic */
    xhuas,			/* 4,7 hungarian nrc to latin/cyrillic */
    xitas,			/* 4,8 italian nrc to latin/cyrillic */
    xnoas,			/* 4,9 norge/danish nrc to latin/cyrillic */
    xpoas,			/* 4,10 portuguese nrc to latin/cyrillic */
    xspas,			/* 4,11 spanish nrc to latin/cyrillic */
    xswas,			/* 4,12 swedish nrc to latin/cyrillic */
    xchas,			/* 4,13 swiss nrc to latin/cyrillic */
    xl1as,			/* 4,14 latin-1 to latin/cyrillic */
    xl2as,			/* 4,15 latin-2 to latin/cyrillic */
    xdmas,			/* 4,16 dec mcs to latin/cyrillic */
    xneas,			/* 4,17 NeXT to latin/cyrillic */
    x43as,			/* 4,18 CP437 to latin/cyrillic */
    x85as,			/* 4,19 CP850 to latin/cyrillic */
    x52as,			/* 4,20 CP852 to latin/cyrillic */
    xaqas,			/* 4,21 Macintosh Latin to latin/cyrillic */
    xdgas,			/* 4,22 DGI to Latin/Cyrillic */
    xr8as,			/* 4,23 HP to Latin/Cyrillic */
    NULL,                       /* 4,24 Latin/Cyrillic to Latin/Cyrillic */
    xaclc,                      /* 4,25 CP866 to Latin/Cyrillic */
    xskcy,                      /* 4,26 Short KOI to Latin/Cyrillic */
    xk8lc,                      /* 4,27 Old KOI-8 Cyrillic to Latin/Cyrillic */
    NULL,			/* 4,28 Kanji... */
    NULL,			/* 4,29 */
    NULL,			/* 4,30 */
    NULL,			/* 4,31 */
    xh7as,			/* 4,32 Hebrew-7 to Latin/Cyrillic */
    xlhas,			/* 4,33 Latin/Hebrew to Latin/Cyrillic */
    x62as,			/* 4,34 CP862 Hebrew to Latin/Cyrillic */
    xegas,			/* 4,35 ELOT 927 Greek to Latin/Cyrillic */
    xleft160,			/* 4,36 Latin/Greek to Latin/Cyrillic */
    xleft128,			/* 4,37 CP869 to Latin/Cyrillic */
    xl1as,			/* 4,38 latin-9 to latin/cyrillic */
    xleft128,			/* 4,39 CP858 to Latin/Cyrillic */
    x55lc,			/* 4,40 CP855 to Latin/Cyrillic */
    x1251lc,			/* 4,41 CP1251 to Latin/Cyrillic */
    xbulc,			/* 4,42 Bulgarian to Latin/Cyrillic */
    x1250as,			/* 4,43 CP1250 to Latin/Cyrillic */
    xmzas,			/* 4,44 Mazovia to Latin/Cyrillic */
    NULL,			/* 4,45 UCS-2 to Latin/Cyrillic */
    NULL,			/* 4,46 UTF-8 to Latin/Cyrillic */
    xkrlc,			/* 4,47 KOI8R to Latin/Cyrillic */
    xkulc,			/* 4,48 KOI8U to Latin/Cyrillic */
    xw1lc,			/* 4,49 CP1252 to Latin/Cyrillic */
    NULL,			/* 5,00 */
    NULL,			/* 5,01 */
    NULL,			/* 5,02 */
    NULL,			/* 5,03 */
    NULL,			/* 5,04 */
    NULL,			/* 5,05 */
    NULL,			/* 5,06 */
    NULL,			/* 4.07 */
    NULL,			/* 5,08 */
    NULL,			/* 5,09 */
    NULL,			/* 5,10 */
    NULL,			/* 5,11 */
    NULL,			/* 5,12 */
    NULL,			/* 5,13 */
    NULL,			/* 5,14 */
    NULL,			/* 5,15 */
    NULL,			/* 5,16 */
    NULL,			/* 5,17 */
    NULL,			/* 5,18 */
    NULL,			/* 5,19 */
    NULL,			/* 5,20 */
    NULL,			/* 5,21 */
    NULL,			/* 5,22 */
    NULL,			/* 5,23 */
    NULL,			/* 5,24 */
    NULL,			/* 5,25 */
    NULL,			/* 5,26 */
    NULL,			/* 5,27 */
    NULL,			/* 5,28 */
    NULL,			/* 5,29 */
    NULL,			/* 5,30 */
    NULL,			/* 5,31 */
    NULL,			/* 5,32 */
    NULL,			/* 5,33 */
    NULL,			/* 5,34 */
    NULL,			/* 5,35 */
    NULL,			/* 5,36 */
    NULL,			/* 5,37 */
    NULL,			/* 5,38 */
    NULL,			/* 5,39 */
    NULL,			/* 5,40 */
    NULL,			/* 5,41 */
    NULL,			/* 5,42 */
    NULL,			/* 5,43 */
    NULL,			/* 5,44 */
    NULL,			/* 5,45 */
    NULL,			/* 5,46 */
    NULL,			/* 5,47 */
    NULL,			/* 5,48 */
    NULL,			/* 5,49 */
    NULL,			/* 6,0 us ascii to Latin/Hebrew */
    NULL,			/* 6,1 uk ascii to Latin/Hebrew */
    xduas,			/* 6,2 dutch nrc to Latin/Hebrew */
    xfias,			/* 6,3 finnish nrc to Latin/Hebrew */
    xfras,			/* 6,4 french nrc to Latin/Hebrew */
    xfcas,			/* 6,5 french canadian nrc to Latin/Hebrew */
    xgeas,			/* 6,6 german nrc to Latin/Hebrew */
    xhuas,			/* 6,7 hungarian nrc to Latin/Hebrew */
    xitas,			/* 6,8 italian nrc to Latin/Hebrew */
    xnoas,			/* 6,9 norge/danish nrc to Latin/Hebrew */
    xpoas,			/* 6,10 portuguese nrc to Latin/Hebrew */
    xspas,			/* 6,11 spanish nrc to Latin/Hebrew */
    xswas,			/* 6,12 swedish nrc to Latin/Hebrew */
    xchas,			/* 6,13 swiss nrc to Latin/Hebrew */
    xl1lh,			/* 6,14 latin-1 to Latin/Hebrew */
    xl2as,			/* 6,15 latin-2 to Latin/Hebrew */
    xdmas,			/* 6,16 dec mcs to Latin/Hebrew */
    xneas,			/* 6,17 NeXT to Latin/Hebrew */
    x43as,			/* 6,18 CP437 to Latin/Hebrew */
    x85as,			/* 6,19 CP850 to Latin/Hebrew */
    x52as,			/* 6,20 CP852 to Latin/Hebrew */
    xaqas,			/* 6,21 Macintosh Latin to Latin/Hebrew */
    xdgas,			/* 6,22 DGI to Latin/Hebrew */
    xr8as,			/* 6,23 HP to Latin/Hebrew */
    xlcas,                      /* 6,24 Latin/Cyrillic to Latin/Hebrew */
    xacas,                      /* 6,25 CP866 to Latin/Hebrew */
    xskas,                      /* 6,26 Short KOI to Latin/Hebrew */
    xk8as,                      /* 6,27 Old KOI-8 Cyrillic to Latin/Hebrew */
    NULL,			/* 6,28 Kanji... */
    NULL,			/* 6,29 */
    NULL,			/* 6,30 */
    NULL,			/* 6,31 */
    xh7lh,			/* 6,32 Hebrew-7 to Latin/Hebrew */
    NULL,			/* 6,33 Latin/Hebrew to Latin/Hebrew */
    x62lh,			/* 6,34 CP862 Hebrew to Latin/Hebrew */
    xegas,			/* 6,35 ELOT 927 Greek to Latin/Hebrew */
    xlgas,			/* 6,36 Latin/Greek to Latin/Hebrew */
    x69as,			/* 6,37 CP869 to Latin/Hebrew */
    xl1as,			/* 6,38 latin-9 to Latin/Hebrew */
    x58as,			/* 6,39 CP858 to Latin/Hebrew */
    x55as,			/* 6,40 CP855 to Latin/Hebrew */
    x1251as,			/* 6,41 CP1251 to Latin/Hebrew */
    xleft128,			/* 6,42 Bulgarian to Latin/Hebrew */
    x1250as,			/* 6,43 CP1250 to Latin/Hebrew */
    xmzas,			/* 6,44 Mazovia to Latin/Hebrew */
    NULL,			/* 6,45 UCS-2 to Latin/Hebrew */
    NULL,			/* 6,46 UTF-8 to Latin/Hebrew */
    NULL,			/* 6,47 KOI8R to Latin/Hebrew */
    NULL,			/* 6,48 KOI8U to Latin/Hebrew */
    xw1lh,			/* 6,49 CP1252 to Latin/Hebrew */
    NULL,			/* 7,0 us ascii to Latin/Greek */
    NULL,			/* 7,1 uk ascii to Latin/Greek */
    xduas,			/* 7,2 dutch nrc to Latin/Greek */
    xfias,			/* 7,3 finnish nrc to Latin/Greek */
    xfras,			/* 7,4 french nrc to Latin/Greek */
    xfcas,			/* 7,5 french canadian nrc to Latin/Greek */
    xgeas,			/* 7,6 german nrc to Latin/Greek */
    xhuas,			/* 7,7 hungarian nrc to Latin/Greek */
    xitas,			/* 7,8 italian nrc to Latin/Greek */
    xnoas,			/* 7,9 norge/danish nrc to Latin/Greek */
    xpoas,			/* 7,10 portuguese nrc to Latin/Greek */
    xspas,			/* 7,11 spanish nrc to Latin/Greek */
    xswas,			/* 7,12 swedish nrc to Latin/Greek */
    xchas,			/* 7,13 swiss nrc to Latin/Greek */
    xl1lg,			/* 7,14 latin-1 to Latin/Greek */
    xl2lg,			/* 7,15 latin-2 to Latin/Greek */
    xl1lg,			/* 7,16 dec mcs to Latin/Greek */
    xneas,			/* 7,17 NeXT to Latin/Greek */
    xleft128,			/* 7,18 CP437 to Latin/Greek */
    x85as,			/* 7,19 CP850 to Latin/Greek */
    x52as,			/* 7,20 CP852 to Latin/Greek */
    xaqas,			/* 7,21 Macintosh Latin to Latin/Greek */
    xdgas,			/* 7,22 DGI to Latin/Greek */
    xr8as,			/* 7,23 HP to Latin/Greek */
    xleft160,                   /* 7,24 Latin/Cyrillic to Latin/Greek */
    xleft128,                   /* 7,25 CP866 to Latin/Greek */
    xskas,                      /* 7,26 Short KOI to Latin/Greek */
    xk8as,                      /* 7,27 Old KOI-8 Cyrillic to Latin/Greek */
    NULL,			/* 7,28 Kanji... */
    NULL,			/* 7,29 */
    NULL,			/* 7,30 */
    NULL,			/* 7,31 */
    xh7as,			/* 7,32 Hebrew-7 to Latin/Greek */
    NULL,			/* 7,33 Latin/Hebrew to Latin/Greek */
    x62as,			/* 7,34 CP862 Hebrew to Latin/Greek */
    xeglg,			/* 7,35 ELOT 927 Greek to Latin/Greek */
    NULL,			/* 7,36 Latin/Greek to Latin/Greek */
    x69lg,			/* 7,37 CP869 to Latin/Greek */
    xl1as,			/* 7,38 latin-9 to Latin/Greek */
    xl1as,			/* 7,39 latin-9 to Latin/Hebrew*/
    xleft128,			/* 7,40 CP855 to Latin/Greek */
    xleft128,			/* 7,41 CP1251 to Latin/Greek */
    xleft128,			/* 7,42 Bulgarian to Latin/Greek */
    x1250as,			/* 7,43 CP1250 to Latin/Greek */
    xmzas,			/* 7,44 Mazovia to Latin/Greek */
    NULL,			/* 7,45 UCS-2 to Latin/Greek */
    NULL,			/* 7,46 UTF-8 to Latin/Greek */
    NULL,			/* 7,47 KOI8R to Latin/Greek */
    NULL,			/* 7,48 KOI8U to Latin/Greek */
    xw1lg,			/* 7,49 CP1252 to Latin/Greek */
    NULL,			/* 8,0 us ascii to latin-9 */
    xukl1,			/* 8,1 uk ascii to latin-9 */
    xdul1,			/* 8,2 dutch nrc to latin-9 */
    xfil1,			/* 8,3 finnish nrc to latin-9 */
    xfrl1,			/* 8,4 french nrc to latin-9 */
    xfcl1,			/* 8,5 french canadian nrc to latin-9 */
    xgel1,			/* 8,6 german nrc to latin-9 */
    xhul1,			/* 8,7 hungarian nrc to latin-9 */
    xitl1,			/* 8,8 italian nrc to latin-9 */
    xnol1,			/* 8,9 norwegian/danish nrc to latin-9 */
    xpol1,			/* 8,10 portuguese nrc to latin-9 */
    xspl1,			/* 8,11 spanish nrc to latin-9 */
    xswl1,			/* 8,12 swedish nrc to latin-9 */
    xchl1,			/* 8,13 swiss nrc to latin-9 */
    NULL,			/* 8,14 latin-1 to latin-9 */
    xl2l9,			/* 8,15 latin-2 to latin-9 */
    xdml9,			/* 8,16 dec mcs to latin-9 */
    xnel9,                      /* 8,17 NeXT To Latin-9 */
    x43l1,                      /* 8,18 CP437 To Latin-9 */
    x85l1,                      /* 8,19 CP850 To Latin-9 */
    x52l1,                      /* 8,20 CP852 To Latin-9 */
    xaql1,                      /* 8,21 Macintosh Latin To Latin-9 */
    xdgl1,                      /* 8,22 DGI To Latin-9 */
    xr8l1,                      /* 8,23 HP To Latin-9 */
    xlcas,                      /* 8,24 Latin/Cyrillic To Latin-9 */
    xacas,                      /* 8,25 CP866 To Latin-9 */
    xskas,                      /* 8,26 Short KOI To Latin-9 */
    xk8as,                      /* 8,27 Old KOI-8 Cyrillic To Latin-9 */
    NULL,			/* 8,28 Kanji ... */
    NULL,			/* 8,29 */
    NULL,			/* 8,30 */
    NULL,			/* 8,31 */
    xh7as,			/* 8,32 Hebrew-7 To Latin-9 */
    xlhl1,			/* 8,33 Latin/Hebrew To Latin-9 */
    x62l1,			/* 8,34 CP862 Hebrew To Latin-9 */
    xegas,			/* 8,35 ELOT 927 Greek To Latin-9 */
    xlgl1,			/* 8,36 Latin/Greek To Latin-9 */
    xl169,			/* 8,37 CP869 To Latin-9 */
    NULL,			/* 8,38 Latin-9 To Latin-9 */
    x58l9,			/* 8,39 cp858 To Latin-9 */
    x55as,			/* 8,40 cp855 To Latin-9 */
    x55as,			/* 8,41 cp1251 To Latin-9 */
    xleft128,			/* 8,42 Bulgarian To Latin-9 */
    x1250l9,			/* 8,43 CP1250 To Latin-9 */
    xmzl9,			/* 8,44 Mazovia To Latin-9 */
    NULL,			/* 8,45 UCS-2 to Latin-9 */
    NULL,			/* 8,46 UTF-8 to Latin-9 */
    NULL,			/* 8,47 KOI8R to Latin-9 */
    NULL,			/* 8,48 KOI8U to Latin-9 */
    xw1l9,			/* 8,49 CP1252 to Latin-9 */
    NULL,			/* 9,00 Unicode... */
    NULL,			/* 9,01 */
    NULL,			/* 9,02 */
    NULL,			/* 9,03 */
    NULL,			/* 9,04 */
    NULL,			/* 9,05 */
    NULL,			/* 9,06 */
    NULL,			/* 9,07 */
    NULL,			/* 9,08 */
    NULL,			/* 9,09 */
    NULL,			/* 9,10 */
    NULL,			/* 9,11 */
    NULL,			/* 9,12 */
    NULL,			/* 9,13 */
    NULL,			/* 9,14 */
    NULL,			/* 9,15 */
    NULL,			/* 9,16 */
    NULL,			/* 9,17 */
    NULL,			/* 9,18 */
    NULL,			/* 9,19 */
    NULL,			/* 9,20 */
    NULL,			/* 9,21 */
    NULL,			/* 9,22 */
    NULL,			/* 9,23 */
    NULL,			/* 9,24 */
    NULL,			/* 9,25 */
    NULL,			/* 9,26 */
    NULL,			/* 9,27 */
    NULL,			/* 9,28 */
    NULL,			/* 9,29 */
    NULL,			/* 9,30 */
    NULL,			/* 9,31 */
    NULL,			/* 9,32 */
    NULL,			/* 9,33 */
    NULL,			/* 9,34 */
    NULL,			/* 9,35 */
    NULL,			/* 9,36 */
    NULL,			/* 9,37 */
    NULL,			/* 9,38 */
    NULL,			/* 9,39 */
    NULL,			/* 9,40 */
    NULL,			/* 9,41 */
    NULL,			/* 9,42 */
    NULL,			/* 9,43 */
    NULL,			/* 9,44 */
    NULL,			/* 9,45 */
    NULL,			/* 9,46 */
    NULL,			/* 9,47 */
    NULL,			/* 9,48 */
    NULL,			/* 9,49 */
    NULL,			/* 10,00 */
    NULL,			/* 10,01 */
    NULL,			/* 10,02 */
    NULL,			/* 10,03 */
    NULL,			/* 10,04 */
    NULL,			/* 10,05 */
    NULL,			/* 10,06 */
    NULL,			/* 10,07 */
    NULL,			/* 10,08 */
    NULL,			/* 10,09 */
    NULL,			/* 10,10 */
    NULL,			/* 10,11 */
    NULL,			/* 10,12 */
    NULL,			/* 10,13 */
    NULL,			/* 10,14 */
    NULL,			/* 10,15 */
    NULL,			/* 10,16 */
    NULL,			/* 10,17 */
    NULL,			/* 10,18 */
    NULL,			/* 10,19 */
    NULL,			/* 10,20 */
    NULL,			/* 10,21 */
    NULL,			/* 10,22 */
    NULL,			/* 10,23 */
    NULL,			/* 10,24 */
    NULL,			/* 10,25 */
    NULL,			/* 10,26 */
    NULL,			/* 10,27 */
    NULL,			/* 10,28 */
    NULL,			/* 10,29 */
    NULL,			/* 10,30 */
    NULL,			/* 10,31 */
    NULL,			/* 10,32 */
    NULL,			/* 10,33 */
    NULL,			/* 10,34 */
    NULL,			/* 10,35 */
    NULL,			/* 10,36 */
    NULL,			/* 10,37 */
    NULL,			/* 10,38 */
    NULL,			/* 10,39 */
    NULL,			/* 10,40 */
    NULL,			/* 10,41 */
    NULL,			/* 10,42 */
    NULL,			/* 10,43 */
    NULL,			/* 10,44 */
    NULL,			/* 10,45 */
    NULL,			/* 10,46 */
    NULL,			/* 10,47 */
    NULL,			/* 10,48 */
    NULL			/* 10,49 */
};
int nxls = (sizeof(xls) / sizeof(CHAR *));

#ifndef NOLOCAL
/*
  The following routines are useful only for terminal character sets, and so
  ifdef'd out for NOLOCAL compilations.
*/

/*
  C S _ I S _ N R C

  Returns nonzero if argument indicates a 7-bit national character set,
  zero otherwise.
*/
int
cs_is_nrc(x) int x; {
#ifdef UNICODE
    if (x == TX_J201R || x == TX_DECSPEC || x == TX_DECTECH
        || txrinfo[x] == NULL)
      return(0);
    else
      return(txrinfo[x]->flags & X2U_STD && txrinfo[x]->size == 94);
#else /* UNICODE */
    if ((cs_size(x) == 94))
      return(1);
    else
      return(0);
#endif /* UNICODE */
}

/*
  C S _ I S _ S T D

  Returns nonzero if argument indicates an ISO 4873-standard-format
  character set, i.e. one in which the control region is NOT used for
  graphics; zero otherwise.
*/
int
cs_is_std(x) int x; {
#ifdef UNICODE
    if (!txrinfo[x])			/* Even more safety */
      return(0);
    else if (txrinfo[x]->size == 128)	/* Just for safety */
      return(0);
    else
      return(txrinfo[x]->flags & X2U_STD); /* Only this should be needed */
#else
    switch (x) {
      case FC_CP437:			/* Code pages use C1 graphics */
      case FC_CP850:
      case FC_CP852:
      case FC_CP862:
      case FC_CP866:
      case FC_CP869:
      case FC_CP858:
      case FC_APPQD:			/* So do Apple and NeXTSTEP */
      case FC_NEXT:
	return(0);
      default:				/* Others behave */
	return(1);
    }
#endif /* CKOUINI */
}

int
cs_size(x) int x; {
#ifdef UNICODE
    if (!txrinfo[x])
      return(128);
    return(txrinfo[x]->size);
#else
    switch(x) {
      case FC_USASCII:
      case FC_UKASCII:
      case FC_DUASCII:
      case FC_FIASCII:
      case FC_FRASCII:
      case FC_FCASCII:
      case FC_GEASCII:
      case FC_HUASCII:
      case FC_ITASCII:
      case FC_NOASCII:
      case FC_POASCII:
      case FC_SPASCII:
      case FC_SWASCII:
      case FC_CHASCII:
      case FC_KOI7:
      case FC_HE7:
      case FC_ELOT:
	return(94);

      case FC_1LATIN:
      case FC_2LATIN:
      case FC_DECMCS:
      case FC_DGMCS:
      case FC_HPR8:
      case FC_CYRILL:
      case FC_KOI8:
      case FC_HEBREW:
      case FC_GREEK:
      case FC_9LATIN:
	return(96);

      case FC_NEXT:
      case FC_CP437:
      case FC_CP850:
      case FC_CP852:
      case FC_CP855:
      case FC_CP862:
      case FC_CP866:
      case FC_CP1251:
      case FC_APPQD:
	return(128);
#ifdef KANJI
      case FC_JIS7:
	return(-94);
      case FC_SHJIS:
	return(-128);
      case FC_JEUC:
      case FC_JDEC:
	return(-96);
#endif /* KANJI */
      case FC_CP858:
      default:
	return(-1);
    }
#endif /* UNICODE */
}
#endif /* NOLOCAL */

/*
  S E T X L A T Y P E  --  Set Translation Type

  Sets global xlatype to indicate which kind of translation:

    XLA_NONE      No translation
    XLA_BYTE      Byte-for-Byte translation
    XLA_JAPAN     Japanese Kanji translation
    XLA_UNICODE   Unicode translations

  And sets up the appropriate translation function pointers as follows:

  For no translation:
    All function pointers are NULL.

  For Byte-for-Byte transation:
    rx  = TCS to FCS (these functions are in this module...)
    sx  = FCS to TCS

  For Unicode translations:
    xfu = FCS to UCS (these functions are in ckcuni.c...)
    xtu = TCS to UCS
    xuf = UCS to FCS
    xut = UCS to TCS
*/
VOID
setxlatype(tcs, fcs) int tcs, fcs; {
#ifdef UNICODE
    xfu = NULL;				/* Unicode <-> TCS/FCS functions */
    xtu = NULL;
    xuf = NULL;
    xut = NULL;
#endif /* UNICODE */
    rx = sx = NULL;
    debug(F101,"setxlatype fcs","",fcs);
    debug(F101,"setxlatype tcs","",tcs);

    if (tcs < 0 || tcs > MAXTCSETS) {
	debug(F101,"setxlatype bad tcs","",tcs);
	return;
    }
    if (fcs < 0 || fcs > MAXFCSETS) {
	debug(F101,"setxlatype bad fcs","",fcs);
	return;
    }
    if (tcs == TC_TRANSP || xfrxla == 0) { /* Transfer charset TRANSPARENT */
	debug(F101,"setxlatype transparent because TCS==Transparent","",tcs);
	xlatype = XLA_NONE;		/* Translation type is None */
#ifdef UNICODE
    /* If any of our charsets is Unicode we use Unicode functions */
    /* even if TCS and FCS are the same because of BOM and byte swapping */
    } else if (tcs == TC_UCS2 || tcs == TC_UTF8 ||
	       fcs == FC_UCS2 || fcs == FC_UTF8) {
	debug(F101,"setxlatype Unicode tcs","",tcs);
	debug(F101,"setxlatype Unicode fcs","",fcs);
	/* Unicode <-> TCS/FCS functions */
	xfu = xl_fcu[fcs];		/* FCS -> UCS */
	xtu = xl_tcu[tcs];		/* TCS -> UCS */
	xuf = xl_ufc[fcs];		/* UCS -> FCS */
	xut = xl_utc[tcs];		/* UCS -> TCS */
        xlatype = XLA_UNICODE;		/* Translation type is Unicode */
#ifdef COMMENT
	/* These make trouble in 64-bit land */
	debug(F001,"setxlatype Unicode xfu","",(unsigned)xfu);
	debug(F001,"setxlatype Unicode xuf","",(unsigned)xuf);
#endif /* COMMENT */
#endif /* UNICODE */
    } else if (cseqtab[tcs] == fcs) {	/* Or if TCS == FCS */
	debug(F101,"setxlatype transparent because TCS==FCS","",tcs);
	xlatype = XLA_NONE;		/* translation type is also None */
#ifdef KANJI
    /* Otherwise if any of them is Japanese, we use Kanji functions */
    } else if (tcs == TC_JEUC || fcsinfo[fcs].alphabet == AL_JAPAN) {
	debug(F101,"setxlatype Japanese tcs","",tcs);
	debug(F101,"setxlatype Japanese fcs","",fcs);
        xlatype = XLA_JAPAN;		/* Translation type is Japanese */
#endif /* KANJI */
    /* Otherwise we use byte functions */
    } else {				/* Otherwise... */
	rx = xlr[tcs][fcs];		/* Input translation function */
	sx = xls[tcs][fcs];		/* Output translation function */
	debug(F101,"setxlatype Byte tcs","",tcs);
	debug(F101,"setxlatype Byte fcs","",fcs);
        xlatype = XLA_BYTE;		/* Translation type is Byte */
    }
    debug(F101,"setxlatype xlatype","",xlatype);
}

/* Set up translation between two file character sets with UCS intermediate */

#ifdef UNICODE
VOID
initxlate(csin, csout) int csin, csout; {
    xfu = NULL;
    xtu = NULL;
    xuf = NULL;
    xut = NULL;

    debug(F101,"initxlate csin","",csin);
    debug(F101,"initxlate csout","",csout);

    if (csin < 0 || csin > MAXFCSETS) {
	debug(F101,"initxlate bad csin","",csin);
	return;
    }
    if (csout < 0 || csout > MAXFCSETS) {
	debug(F101,"initxlate bad csout","",csout);
	return;
    }
    if (csin == csout && csin != FC_UCS2) {
	xlatype = XLA_NONE;		/* Translation type is None */
	return;
    }
    xlatype = XLA_UNICODE;		/* Translation type is Unicode */
    xfu = xl_fcu[csin];			/* FCS -> UCS */
    xuf = xl_ufc[csout];		/* UCS -> FCS */
    xpnbyte(-1,0,0,NULL);		/* Reset UCS-2 */
#ifdef COMMENT
    debug(F001,"initxlate Unicode xfu","",(unsigned)xfu);
    debug(F001,"initxlate Unicode xuf","",(unsigned)xuf);
#endif /* COMMENT */
    debug(F101,"initxlate xlatype","",xlatype);
}
#endif /* UNICODE */

int csetsinited = 0;

VOID
initcsets() {				/* Routine to reset or initialize */
    int i;				/* character-set associations. */

#ifdef UNICODE
    if (ucsorder < 0)			/* For creating UCS-2 files. */
      ucsorder = byteorder;
    if (ucsorder < 0)
      ucsorder = 0;
    if (fileorder < 0)			/* For reading UCS-2 files. */
      fileorder = ucsorder;
#endif /* UNICODE */

    debug(F101,"initcsets nxls","",nxls);
    debug(F101,"initcsets nxlr","",nxlr);

    debug(F101,"initcsets TERM LOCAL CSET","",tcsl);
    debug(F101,"initcsets TERM REMOTE CSET","",tcsr);

    for (i = 0; i <= MAXFCSETS; i++)	/* First clear them all... */
      afcset[i] = -1;
    for (i = 0; i <= MAXTCSETS; i++)
      axcset[i] = -1;

    /* Now set specific defaults for incoming files */

    xlatype = XLA_NONE;

    axcset[TC_TRANSP]  = FC_TRANSP;
    axcset[TC_USASCII] = FC_USASCII;

#ifdef OS2
    switch (fcharset) {
      case FC_CP850:
      case FC_CP858:
      case FC_CP437:
      case FC_1LATIN:
	axcset[TC_1LATIN]  = fcharset;
	break;
      default:
	axcset[TC_1LATIN]  = FC_CP850;
    }
#else
#ifdef HPUX
    axcset[TC_1LATIN]  = FC_HPR8;
#else
#ifdef VMS
    axcset[TC_1LATIN]  = FC_DECMCS;
#else
#ifdef NEXT
    axcset[TC_1LATIN]  = FC_NEXT;
#else
#ifdef datageneral
    axcset[TC_1LATIN]  = FC_DGMCS;
#else
    /* Should we use code pages on some PC based UNIXes? */
    axcset[TC_1LATIN]  = FC_1LATIN;
#endif /* datageneral */
#endif /* NEXT */
#endif /* VMS */
#endif /* HPUX */
#endif /* OS2 */

#ifdef OS2
    axcset[TC_2LATIN]  = FC_CP852;
    axcset[TC_CYRILL]  = FC_CP866;
    axcset[TC_JEUC]    = FC_SHJIS;
    axcset[TC_HEBREW]  = FC_CP862;
    axcset[TC_GREEK]   = FC_CP869;
    axcset[TC_9LATIN]  = FC_CP858;
    axcset[TC_UCS2]    = FC_UCS2;
    axcset[TC_UTF8]    = FC_UCS2;
#else
    axcset[TC_2LATIN]  = FC_2LATIN;
    axcset[TC_CYRILL]  = FC_CYRILL;
    axcset[TC_JEUC]    = FC_JEUC;
    axcset[TC_HEBREW]  = FC_HEBREW;
    axcset[TC_GREEK]   = FC_GREEK;
    axcset[TC_9LATIN]  = FC_9LATIN;
    axcset[TC_UCS2]    = FC_UTF8;
    axcset[TC_UTF8]    = FC_UTF8;
#endif /* OS2 */

    /* And for outbound files */

    afcset[FC_USASCII] = TC_USASCII;
    afcset[FC_UKASCII] = TC_1LATIN;
    afcset[FC_DUASCII] = TC_1LATIN;
    afcset[FC_FIASCII] = TC_1LATIN;
    afcset[FC_FRASCII] = TC_1LATIN;
    afcset[FC_FCASCII] = TC_1LATIN;
    afcset[FC_GEASCII] = TC_1LATIN;
    afcset[FC_HUASCII] = TC_2LATIN;
    afcset[FC_ITASCII] = TC_1LATIN;
    afcset[FC_NOASCII] = TC_1LATIN;
    afcset[FC_POASCII] = TC_1LATIN;
    afcset[FC_SPASCII] = TC_1LATIN;
    afcset[FC_SWASCII] = TC_1LATIN;
    afcset[FC_CHASCII] = TC_1LATIN;
    afcset[FC_1LATIN]  = TC_1LATIN;
    afcset[FC_2LATIN]  = TC_2LATIN;
    afcset[FC_DECMCS]  = TC_1LATIN;
    afcset[FC_NEXT]    = TC_1LATIN;
    afcset[FC_CP437]   = TC_1LATIN;
    afcset[FC_CP850]   = TC_1LATIN;
    afcset[FC_CP852]   = TC_2LATIN;
    afcset[FC_APPQD]   = TC_1LATIN;
    afcset[FC_DGMCS]   = TC_1LATIN;
    afcset[FC_HPR8]    = TC_1LATIN;
    afcset[FC_CYRILL]  = TC_CYRILL;
    afcset[FC_CP866]   = TC_CYRILL;
    afcset[FC_KOI7]    = TC_CYRILL;
    afcset[FC_KOI8]    = TC_CYRILL;
    afcset[FC_JIS7]    = TC_JEUC;
    afcset[FC_SHJIS]   = TC_JEUC;
    afcset[FC_JEUC]    = TC_JEUC;
    afcset[FC_JDEC]    = TC_JEUC;
    afcset[FC_HE7]     = TC_HEBREW;
    afcset[FC_HEBREW]  = TC_HEBREW;
    afcset[FC_CP862]   = TC_HEBREW;
    afcset[FC_ELOT]    = TC_GREEK;
    afcset[FC_GREEK]   = TC_GREEK;
    afcset[FC_CP869]   = TC_GREEK;
    afcset[FC_9LATIN]  = TC_9LATIN;
    afcset[FC_CP923]   = TC_9LATIN;
    afcset[FC_CP858]   = TC_9LATIN;
    afcset[FC_CP855]   = TC_CYRILL;
    afcset[FC_CP1251]  = TC_CYRILL;
    afcset[FC_BULGAR]  = TC_CYRILL;
    afcset[FC_CP1250]  = TC_2LATIN;
    afcset[FC_MAZOVIA] = TC_2LATIN;
    afcset[FC_UCS2]    = TC_UTF8;
    afcset[FC_UTF8]    = TC_UTF8;

    afcset[FC_KOI8R]   = TC_CYRILL;
    afcset[FC_KOI8U]   = TC_CYRILL;
    afcset[FC_CP1252]  = TC_1LATIN;

    csetsinited++;
    return;
}
#endif /* NOCSETS */

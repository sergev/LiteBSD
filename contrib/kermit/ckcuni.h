/*  C K C U N I . H  --  Unicode/Terminal character-set translations  */

/*
  Copyright (C) 1999, 2009,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.

  Authors:
    Frank da Cruz <fdc@columbia.edu>
      The Kermit Project, Columbia University, New York City.
    Jeffrey E Altman <jaltman@secure-endpoints.com>
      Secure Endpoints Inc., New York City

*/

/* Terminal character sets */

#ifndef CKOUNI_H
#define CKOUNI_H
#ifdef OS2
#ifndef CKOUNI
#define CKOUNI                          /* Use UNICODE for OS/2 functions */
#endif /* CKOUNI */
#ifdef KUI
#define X_CKOUNI_IN                     /* Use Unicode Input */
#define CKOUNI_OUT
#endif /* KUI */
#endif /* OS2 */

/* Terminal Character Sets */

#define TX_ASCII        0               /* US ASCII */
#define TX_BRITISH      1               /* British ISO 646 */
#define TX_CN_FRENCH    2               /* Canadian French NRC */
#define TX_CUBAN        3               /* Cuba */
#define TX_CZECH        4               /* Czech Republic */
#define TX_DANISH       5               /* Denmark / Norway ISO 646 */
#define TX_DUTCH        6               /* Dutch NRC */
#define TX_FINNISH      7               /* Finnish NRC */
#define TX_FRENCH       8               /* French ISO 646 */
#define TX_GERMAN       9               /* German ISO 646 */
#define TX_HE7         10               /* Hebrew 7 (DEC) */
#define TX_HUNGARIAN   11               /* Hungarian ISO 646 */
#define TX_ICELANDIC   12               /* Icelandic NRC */
#define TX_ITALIAN     13               /* Italian ISO 646 */
#define TX_J201R       14               /* JIS 0201 Japanese Roman */
#define TX_J201K       15               /* JIS 0201 Katakana */
#define TX_KOI7        16               /* Short KOI */
#define TX_NORWEGIAN   17               /* Denmark / Norway ISO 646 */
#define TX_PORTUGUESE  18               /* Portuguese ISO 646 */
#define TX_SPANISH     19               /* Spanish ISO 646 */
#define TX_SWEDISH     20               /* Swedish ISO 646 */
#define TX_SWE_2       21               /* Swedish for names ISO 646 */
#define TX_SWISS       22               /* Swiss NRC */
#define TX_8859_1      23               /* Latin-1 */
#define TX_8859_2      24               /* Latin-2 */
#define TX_8859_3      25               /* Latin-3 */
#define TX_8859_4      26               /* Latin-4 */
#define TX_8859_5      27               /* Latin/Cyrillic */
#define TX_8859_6      28               /* Latin/Arabic */
#define TX_8859_7      29               /* Latin/Greek */
#define TX_8859_8      30               /* Latin/Hebrew */
#define TX_8859_9      31               /* Latin-5 */
#define TX_8859_10     32               /* Latin-6 */

#define TX_KOI8        33               /* GOST 19768-74 KOI-8 */

#define TX_JIS7        34               /* JIS-7 */
#define TX_SHJIS       35               /* Shift JIS */
#define TX_JEUC        36               /* Japanese EUC */
#define TX_JDEC        37               /* Japanese DEC Kanji */

#define TX_DECMCS      38               /* DEC MCS */
#define TX_NEXT        39               /* NeXT */
#define TX_DGI         40               /* Data General International */
#define TX_MACL1       41               /* Macintosh Latin-1 */
#define TX_HPR8        42               /* HP Roman 8 */

/* Code pages */

#define TX_CP437       43               /* Original */
#define TX_CP850       44               /* Multinational (Western Europe) */
#define TX_CP852       45               /* Eastern Europe */
#define TX_CP857       46               /* Turkey */
#define TX_CP862       47               /* Hebrew */
#define TX_CP864       48               /* Arabic */
#define TX_CP866       49               /* Cyrillic */
#define TX_CP869       50               /* Greek */

#define TX_DECSPEC     51               /* DEC Special Graphics */
#define TX_DECTECH     52               /* DEC Technical */
#define TX_C0PICT      53               /* C0 Display Controls */
#define TX_C1PICT      54               /* C1 Display Controls */
#define TX_IBMC0GRPH   55               /* IBM C0 Graphics (smileys) */
#define TX_H19GRAPH    56               /* Heath/Zenith 19 Graphics */
#define TX_TVIGRAPH    57               /* Televideo Graphics */
#define TX_WYSE60G_N   58               /* Wyse 60 Native Mode Graphics */
#define TX_WYSE60G_1   59               /* Wyse 60 Graphics 1 */
#define TX_WYSE60G_2   60               /* Wyse 60 Graphics 2 */
#define TX_WYSE60G_3   61               /* Wyse 60 Graphics 3 */

/* New ones that came too late for the nice grouping... */

#define TX_ELOT927     62               /* Greek ELOT 927 */
#define TX_DGPCGRPH    63               /* DG PC Graphics */
#define TX_DGLDGRPH    64               /* DG Line Drawing Graphics */
#define TX_DGWPGRPH    65               /* DG Word Processing (etc) Graphics */
#define TX_HPLINE      66               /* HP Line Drawing */
#define TX_HPMATH      67               /* HP Math/Technical */
#define TX_QNXGRPH     68               /* QNX Graphics */

/* Siemens Nixdorf character sets */

#define TX_SNIBRACK    69               /* SNI 97801 Brackets */
#define TX_SNIEURO     70               /* SNI 97801 Euro  */
#define TX_SNIFACET    71               /* SNI 97801 Facet */
#define TX_SNIIBM      72               /* SNI 97801 "IBM" */
#define TX_SNIBLANK    73               /* SNI 97801 Blanks */

/* Windows Code pages */

#define TX_CP1252      74               /* Latin-1 Windows */
#define TX_CP1250      75               /* Latin-2 Windows */
#define TX_CP1251      76               /* Cyrillic Windows */
#define TX_CP1253      77               /* Greece Windows */
#define TX_CP1254      78               /* Turkey Windows */
#define TX_CP1257      79               /* Latin-4 Windows */

#define TX_CP856       80               /* Bulgaria CP856 (DATECS Ltd) */
#define TX_CP855       81
#define TX_CP819       82               /* Same as ISO 8859-1 */
#define TX_CP912       83               /* Same as ISO 8859-2 */
#define TX_CP913       84               /* Same as ISO 8859-3 */
#define TX_CP914       85               /* Same as ISO 8859-4 */
#define TX_CP915       86               /* Same as ISO 8859-5 */
#define TX_CP1089      87               /* Same as ISO 8859-6 */
#define TX_CP813       88               /* Same as ISO 8859-7 */
#define TX_CP916       89               /* Same as ISO 8859-8 */
#define TX_CP920       90               /* Same as ISO 8859-9 */
#define TX_CP1051      91               /* Same as HP Roman 8 */
#define TX_CP858       92               /* Multinational (W. Europe) w/Euro */
#define TX_8859_15     93               /* Latin-9 */
#define TX_CP923       94               /* Same as ISO 8859-15 */

#define TX_ELOT928     95               /* Same as ISO 8859-7 */
#define TX_CP10000     96               /* Same as original Apple Quickdraw */
#define TX_CP37        97               /* EBCDIC */
#define TX_CP1255      98               /* Israel Windows */
#define TX_CP1256      99               /* Arabic Windows */
#define TX_CP1258     100               /* Viet Nam Windows */
#define TX_MAZOVIA    101
#define TX_TRANSP     102               /* Transparent - no translation */
#define TX_HZ1500     103               /* Hazeltine 1500 graphics set */
#define TX_KOI8R      104               /* KOI8R - Russian */
#define TX_KOI8U      105               /* KOI8U - Ukrainian */
#define TX_APL1       106               /* APL ISO IR 68 */
#define TX_APL2       107               /* Dyadic Systems Inc APL */
#define TX_APL3       108               /* APL-Plus (APL-2000) */
#define TX_APL4       109               /* IBM APL/2 */
#define TX_APL5       110               /* APL-2741 */

#define MAXTXSETS     111               /* Number of terminal character sets */

/* The following are not implemented yet */
/* UTF-8 is supported as a special mode in Kermit 95 (see utf8 flag) */

#define TX_UTF7       128
#define TX_UTF8       129

#define TX_HEXBYTES   242               /* Hex bytes */
#define TX_DEBUG      243               /* Debugging but not hex bytes */

/* These are actually used */

#define TX_UNDEF      255               /* Unknown character-set */

/* Flag bit values */

#define X2U_STD   1                     /* Has standard ISO 4873 layout */
#define X2U_ISO   2                     /* ISO standard character set */
#define X2U_JIS   4                     /* Japan Industrial Standard */
#define X2U_CP    8                     /* PC Code Page */
#define X2U_DEC  16                     /* DEC Private character set */
#define X2U_CXG  32                     /* Control codes used for graphics */

struct x_to_unicode {
    int size;                           /* 94, 96, 128, or other */
    int offset;                         /* 0, 32, 33, 128, 160, ... */
    int flags;
    int family;                         /* Language family, writing system */
    char * keywd;                       /* Keyword name */
    char * name;                        /* Descriptive name */
    int code;                           /* ISO reg number if Standard */
                                        /* CP number if Code-page, etc. */
    char * final;                       /* Esc seq final char(s) (ISO, DEC) */
    unsigned short map[256];            /* Mapping table */
};

extern struct keytab txrtab[];
extern int ntxrtab;

#ifndef NULL
#define NULL (char *)0
#endif /* NULL */

#ifndef USHORT
#define USHORT unsigned short
#endif /* USHORT */

#ifndef ULONG
#define ULONG unsigned long
#endif /* ULONG */

#ifndef CHAR
#define CHAR unsigned char
#endif /* CHAR */

#ifdef CK_ANSIC
extern USHORT (*xl_u[MAXTXSETS+1])(CHAR); /* Blah-to-Unicode functions */
extern int (*xl_tx[MAXTXSETS+1])(USHORT); /* Unicode-to-Blah functions */
#else
extern USHORT (*xl_u[MAXTXSETS+1])();
extern int (*xl_tx[MAXTXSETS+1])();
#endif /* CK_ANSIC */
extern struct x_to_unicode * txrinfo[MAXTXSETS+1];

_PROTOTYP(int isunicode, (void));
_PROTOTYP(int utf8_to_ucs2, (CHAR, USHORT **));
_PROTOTYP(int ucs2_to_utf8, (USHORT, CHAR **));
_PROTOTYP(int tx_cpsub, (USHORT));
_PROTOTYP(int u_to_b, (CHAR) );
_PROTOTYP(int u_to_b2, (void) );
_PROTOTYP(int b_to_u, (CHAR, CHAR *, int, int) );

#ifdef KANJI
_PROTOTYP(USHORT sj_to_un, (USHORT) );  /* Shift-JIS to Unicode */
_PROTOTYP(USHORT un_to_sj, (USHORT) );  /* Unicode to Shift-JIS */
#endif /* KANJI */

#ifdef OS2
#ifdef NT
_inline
#else
_Inline
#endif /* NT */
int
isunicode(
#ifdef CK_ANSIC
          void
#endif /* CK_ANSIC */
          ) {
    extern int tt_unicode;
#ifdef NT
#ifdef KUI
    return(tt_unicode);
#else /* KUI */
    if (tt_unicode && !isWin95())
      return(1);
    else
      return(0);
#endif /* KUI */
#else /* NT */
    return(0);
#endif /* NT */
}
#endif /* OS2 */
#endif /* CKOUNI_H */

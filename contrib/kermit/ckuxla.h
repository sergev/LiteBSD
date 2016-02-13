/*
  File CKUXLA.H

  C-Kermit language and character-set support for UNIX, VMS, OS/2,
  AOS/VS, and other systems.

  This file should be used as a template for the language support files
  for other C-Kermit implementations -- Macintosh, etc.
*/
/*
  Author: Frank da Cruz <fdc@columbia.edu>,
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 2009,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.

*/

#ifndef CKUXLA_H
#define CKUXLA_H

/* Codes for file character sets */

/* ISO 646 and other ISO-646-like 7-bit sets */

#define FC_USASCII 0   /* US ASCII */
#define FC_UKASCII 1   /* United Kingdom ASCII */
#define FC_DUASCII 2   /* Dutch ISO 646 NRC */
#define FC_FIASCII 3   /* Finnish ISO 646 NRC */
#define FC_FRASCII 4   /* French ISO 646 NRC */
#define FC_FCASCII 5   /* French Canadian ISO 646 NRC */
#define FC_GEASCII 6   /* German ISO 646 NRC */
#define FC_HUASCII 7   /* Hungarian ISO 646 NRC */
#define FC_ITASCII 8   /* Italian *ISO 646 NRC */
#define FC_NOASCII 9   /* Norwegian and Danish ISO 646 NRC */
#define FC_POASCII 10  /* Portuguese ISO 646 NRC */
#define FC_SPASCII 11  /* Spanish ISO 646 NRC */
#define FC_SWASCII 12  /* Swedish ISO 646 NRC */
#define FC_CHASCII 13  /* Swiss ISO 646 NRC */

/* 8-bit Roman character sets */

#define FC_1LATIN  14  /* ISO 8859-1 Latin Alphabet 1 */
#define FC_2LATIN  15  /* ISO 8859-2 Latin Alphabet 2 */
#define FC_DECMCS  16  /* DEC Multinational Character Set */
#define FC_NEXT    17  /* NeXT workstation character set */
#define FC_CP437   18  /* IBM PC Code Page 437 */
#define FC_CP850   19  /* IBM PC Code Page 850 */
#define FC_CP852   20  /* IBM PC Code Page 852 */
#define FC_APPQD   21  /* Apple Quickdraw */
#define FC_DGMCS   22  /* Data General International Character Set */
#define FC_HPR8    23  /* HP Roman8 */

/* Cyrillic sets */

#define FC_CYRILL  24  /* ISO 8859-5 Latin/Cyrillic */
#define FC_CP866   25  /* PC Code Page 866 Cyrillic */
#define FC_KOI7    26  /* KOI-7 = Short KOI */
#define FC_KOI8    27  /* KOI-8 */

/* Japanese sets */

#define FC_JIS7    28  /* JIS-7 */
#define FC_SHJIS   29  /* Shifted JIS = CP932 */
#define FC_JEUC    30  /* Japanese EUC (JAE) */
#define FC_JDEC    31  /* Japanese DEC Kanji */

/* Hebrew sets */

#define FC_HE7     32  /* 7-Bit DEC Hebrew */
#define FC_HEBREW  33  /* 8-Bit ISO 8859-8 Latin/Hebrew */
#define FC_CP862   34  /* Hebrew PC Code Page */

/* Greek sets */

#define FC_ELOT    35  /* 7-Bit ELOT 927 Greek */
#define FC_GREEK   36  /* 8-Bit ISO 8859-7 Latin/Greek */
#define FC_CP869   37  /* Greek PC Code Page */

/* New Roman sets with Euro symbol */

#define FC_9LATIN  38  /* ISO 8859-15 Latin Alphabet 9 */
#define FC_CP923   38  /* Same as Latin-9 */
#define FC_CP858   39  /* Western Europe with Euro */

/* Other new additions */

#define FC_CP855   40  /* Cyrillic PC Code Page */
#define FC_CP1251  41  /* Cyrillic Windows */
#define FC_BULGAR  42  /* Bulgarian PC code page */
#define FC_CP1250  43  /* Latin 2 Windows (different from Latin-2)*/
#define FC_MAZOVIA 44  /* Polish Mazovia PC code page */

/* Unicode */

#define FC_UCS2    45  /* ISO-10646 / Unicode UCS-2 */
#define FC_UTF8    46  /* ISO-10646 / Unicode UTF-8 */

/* Recent additions */

#define FC_KOI8R   47  /* KOI8-R (RFC1489) - Russian + boxdrawing */
#define FC_KOI8U   48  /* KOI8-U (RFC2319) - Ukrainian + boxdrawing */
#define FC_CP1252  49  /* Latin 1 Windows */

#define MAXFCSETS  49  /* Highest file character set number */

#ifdef OS2
#define FC_DECSPEC 253 /* Not real character-sets */
#define FC_DECTECH 252
#endif /* OS2 */

#ifdef UNICODE
_PROTOTYP( VOID initxlate, (int, int) );
#endif /* UNICODE */

#endif /* CKUXLA_H */

/*
  File CKCASC.H
  Mnemonics for ASCII control characters (and Space) for use with C-Kermit.
*/
/*
  Author: Frank da Cruz (fdc@columbia.edu).
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 2009,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/
#ifndef CKCASC_H
#define CKCASC_H

#define NUL  '\0'       /* Null Ctrl-@*/
#define SOH    1        /* Start of header Ctrl-A */
#define STX    2        /* Ctrl-B */
#define ETX    3        /* Ctrl-C */
#define EOT    4        /* Ctrl-D */
#define ENQ    5        /* ENQ Ctrl-E */
#define ACK    6        /* Ctrl-F */
#define BEL    7        /* Bell (Beep) Ctrl-G */
#define BS     8        /* Backspace Ctrl-H */
#define HT     9        /* Horizontal Tab Ctrl-I */
#define LF    10        /* Linefeed Ctrl-J */
#define VT    11        /* Vertical Tab Ctrl-K */
#define NL   '\n'       /* Newline */
#define FF    12        /* Formfeed Ctrl-L */
#define CR    13        /* Carriage Return Ctrl-M */
#define SO    14        /* Shift Out Ctrl-N */
#define SI    15        /* Shift In Ctrl-O */
#define DLE   16        /* Datalink Escape Ctrl-P */
#define XON   17        /* XON Ctrl-Q */
#define DC1   17
#define DC2   18        /* Ctrl-R */
#define XOFF  19        /* XOFF Ctrl-S */
#define DC3   19
#define DC4   20        /* Ctrl-T */
#define NAK   21        /* Ctrl-U */
#define SYN   22        /* SYN, Ctrl-V */
#define ETB   23        /* Ctrl-W */
#define CAN   24        /* CAN, Ctrl-X */
#define XEM   25        /* Ctrl-Y (was EM but conflicts with OpenSSL) */
#define SUB   26        /* SUB Ctrl-Z */
#define ESC   27        /* Escape Ctrl-[ */
#define XFS   28        /* Field Separator,  Ctrl-Backslash */
#define XGS   29        /* Group Separator,  Ctrl-Rightbracket */
#define XRS   30        /* Record Separator, Ctrl-Circumflex */
#define US    31        /* Unit Separator,   Ctrl-Underscore */
#define SP    32        /* Space */
#define DEL  127        /* Delete (Rubout) */
#define RUB  127        /* Delete (Rubout) */

#ifdef OS2
/*
  These are needed in OS/2, so let's not cause any unnecessary conflicts.
*/
#define _CSI  0233      /* 8-bit Control Sequence Introducer */
#define _SS2  0216      /* 8-bit Single Shift 2 */
#define _SS3  0217      /* 8-bit Single Shift 3 */
#define _DCS  0220      /* 8-bit Device Control String Introducer */
#define _ST8  0234      /* 8-bit String Terminator */
#define _OSC  0235      /* 8-bit Operating System Command */
#define _PM8  0236      /* 8-bit Privacy Message */
#define _APC  0237      /* 8-bit Application Program Command */
#endif /* OS2 */
#endif /* CKCASC_H */

#ifndef _LIBPCC_LIMITS_H_
#define _LIBPCC_LIMITS_H_

#if defined __GNUC__
#  if !defined _GCC_LIMITS_H_
/* this is needed to make limits.h from the glibc headers happy, which uses
   #include_next <limits.h>
   when __GNUC__ is defined and  _GCC_LIMITS_H_ is not defined 
*/
#  define __GCC_LIMITS_H_
#  endif

#endif

/*
 * The following limits are sometimes considered
 * a property of the compiler. Not complete.
 */

#undef CHAR_BIT
#ifdef __pdp10__
#define CHAR_BIT 9
#else
#define CHAR_BIT 8
#endif

/*
 * Logic below assumes 2-complement.
 */
#undef CHAR_MIN
#undef CHAR_MAX
#ifdef __CHAR_UNSIGNED__
#define	CHAR_MIN 0
#define	CHAR_MAX UCHAR_MAX
#else
#define	CHAR_MIN SCHAR_MIN
#define	CHAR_MAX SCHAR_MAX
#endif

#undef SCHAR_MAX
#define	SCHAR_MAX __SCHAR_MAX__
#undef SCHAR_MIN
#define SCHAR_MIN (-SCHAR_MAX-1)
#undef UCHAR_MAX
#define UCHAR_MAX (2*SCHAR_MAX+1)

#undef SHRT_MAX
#define SHRT_MAX __SHRT_MAX__
#undef SHRT_MIN
#define SHRT_MIN (-SHRT_MAX-1)
#undef USHRT_MAX
#define USHRT_MAX (2*SHRT_MAX+1)

#undef INT_MAX
#define INT_MAX __INT_MAX__
#undef INT_MIN
#define INT_MIN (-INT_MAX-1)
#undef UINT_MAX
#define UINT_MAX (2U*INT_MAX+1U)

#undef LONG_MAX
#define LONG_MAX __LONG_MAX__
#undef LONG_MIN
#define LONG_MIN (-LONG_MAX-1L)
#undef ULONG_MAX
#define ULONG_MAX (2UL*LONG_MAX+1UL)

#undef LLONG_MAX
#define LLONG_MAX __LONG_LONG_MAX__
#undef LLONG_MIN
#define LLONG_MIN (-LLONG_MAX-1LL)
#undef ULLONG_MAX
#define ULLONG_MAX (2ULL*LLONG_MAX+1ULL)

#endif


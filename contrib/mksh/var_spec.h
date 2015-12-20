/*-
 * Copyright (c) 2009, 2011, 2012
 *	mirabilos <m@mirbsd.org>
 *
 * Provided that these terms and disclaimer and all copyright notices
 * are retained or reproduced in an accompanying document, permission
 * is granted to deal in this work without restriction, including un-
 * limited rights to use, publicly perform, distribute, sell, modify,
 * merge, give away, or sublicence.
 *
 * This work is provided "AS IS" and WITHOUT WARRANTY of any kind, to
 * the utmost extent permitted by applicable law, neither express nor
 * implied; without malicious intent or gross negligence. In no event
 * may a licensor, author or contributor be held liable for indirect,
 * direct, other damage, loss, or other issues arising in any way out
 * of dealing in the work, even if advised of the possibility of such
 * damage or existence of a defect, except proven that it results out
 * of said person's immediate fault when using the work as intended.
 */

#if defined(VARSPEC_DEFNS)
__RCSID("$MirOS: src/bin/mksh/var_spec.h,v 1.7 2015/12/12 21:08:44 tg Exp $");
#define FN(name)			/* nothing */
#elif defined(VARSPEC_ENUMS)
#define FN(name)			V_##name,
#define F0(name)			V_##name = 0,
#elif defined(VARSPEC_ITEMS)
#define F0(name)			/* nothing */
#define FN(name)			#name,
#endif

#ifndef F0
#define F0 FN
#endif

/* NOTE: F0 are skipped for the ITEMS array, only FN generate names */

/* 0 is always V_NONE */
F0(NONE)

/* 1 and up are special variables */
FN(BASHPID)
FN(COLUMNS)
FN(EPOCHREALTIME)
#if HAVE_PERSISTENT_HISTORY
FN(HISTFILE)
#endif
FN(HISTSIZE)
FN(IFS)
FN(LINENO)
FN(LINES)
FN(OPTIND)
FN(PATH)
FN(RANDOM)
FN(SECONDS)
FN(TMOUT)
FN(TMPDIR)

#undef FN
#undef F0
#undef VARSPEC_DEFNS
#undef VARSPEC_ENUMS
#undef VARSPEC_ITEMS

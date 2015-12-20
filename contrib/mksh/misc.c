/*	$OpenBSD: misc.c,v 1.41 2015/09/10 22:48:58 nicm Exp $	*/
/*	$OpenBSD: path.c,v 1.13 2015/09/05 09:47:08 jsg Exp $	*/

/*-
 * Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
 *		 2011, 2012, 2013, 2014, 2015
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

#include "sh.h"
#if !HAVE_GETRUSAGE
#include <sys/times.h>
#endif
#if HAVE_GRP_H
#include <grp.h>
#endif

__RCSID("$MirOS: src/bin/mksh/misc.c,v 1.240 2015/10/09 16:11:17 tg Exp $");

#define KSH_CHVT_FLAG
#ifdef MKSH_SMALL
#undef KSH_CHVT_FLAG
#endif
#ifdef TIOCSCTTY
#define KSH_CHVT_CODE
#define KSH_CHVT_FLAG
#endif
#ifdef MKSH_LEGACY_MODE
#undef KSH_CHVT_CODE
#undef KSH_CHVT_FLAG
#endif

/* type bits for unsigned char */
unsigned char chtypes[UCHAR_MAX + 1];

static const unsigned char *pat_scan(const unsigned char *,
    const unsigned char *, bool) MKSH_A_PURE;
static int do_gmatch(const unsigned char *, const unsigned char *,
    const unsigned char *, const unsigned char *) MKSH_A_PURE;
static const unsigned char *gmatch_cclass(const unsigned char *, unsigned char)
    MKSH_A_PURE;
#ifdef KSH_CHVT_CODE
static void chvt(const Getopt *);
#endif

/*XXX this should go away */
static int make_path(const char *, const char *, char **, XString *, int *);

#ifdef SETUID_CAN_FAIL_WITH_EAGAIN
/* we don't need to check for other codes, EPERM won't happen */
#define DO_SETUID(func, argvec) do {					\
	if ((func argvec) && errno == EAGAIN)				\
		errorf("%s failed with EAGAIN, probably due to a"	\
		    " too low process limit; aborting", #func);		\
} while (/* CONSTCOND */ 0)
#else
#define DO_SETUID(func, argvec) func argvec
#endif

/*
 * Fast character classes
 */
void
setctypes(const char *s, int t)
{
	unsigned int i;

	if (t & C_IFS) {
		for (i = 0; i < UCHAR_MAX + 1; i++)
			chtypes[i] &= ~C_IFS;
		/* include \0 in C_IFS */
		chtypes[0] |= C_IFS;
	}
	while (*s != 0)
		chtypes[(unsigned char)*s++] |= t;
}

void
initctypes(void)
{
	setctypes(letters_uc, C_ALPHA);
	setctypes(letters_lc, C_ALPHA);
	chtypes['_'] |= C_ALPHA;
	setctypes("0123456789", C_DIGIT);
	/* \0 added automatically */
	setctypes(TC_LEX1, C_LEX1);
	setctypes("*@#!$-?", C_VAR1);
	setctypes(TC_IFSWS, C_IFSWS);
	setctypes("=-+?", C_SUBOP1);
	setctypes("\t\n \"#$&'()*;<=>?[\\]`|", C_QUOTE);
}

/* called from XcheckN() to grow buffer */
char *
Xcheck_grow(XString *xsp, const char *xp, size_t more)
{
	const char *old_beg = xsp->beg;

	if (more < xsp->len)
		more = xsp->len;
	/* (xsp->len + X_EXTRA) never overflows */
	checkoktoadd(more, xsp->len + X_EXTRA);
	xsp->beg = aresize(xsp->beg, (xsp->len += more) + X_EXTRA, xsp->areap);
	xsp->end = xsp->beg + xsp->len;
	return (xsp->beg + (xp - old_beg));
}


#define SHFLAGS_DEFNS
#define FN(sname,cname,flags,ochar)		\
	static const struct {			\
		/* character flag (if any) */	\
		char c;				\
		/* OF_* */			\
		unsigned char optflags;		\
		/* long name of option */	\
		char name[sizeof(sname)];	\
	} shoptione_ ## cname = {		\
		ochar, flags, sname		\
	};
#include "sh_flags.gen"

#define OFC(i) (options[i][-2])
#define OFF(i) (((const unsigned char *)options[i])[-1])
#define OFN(i) (options[i])

const char * const options[] = {
#define SHFLAGS_ITEMS
#include "sh_flags.gen"
};

/*
 * translate -o option into F* constant (also used for test -o option)
 */
size_t
option(const char *n)
{
	size_t i = 0;

	if ((n[0] == '-' || n[0] == '+') && n[1] && !n[2])
		while (i < NELEM(options)) {
			if (OFC(i) == n[1])
				return (i);
			++i;
		}
	else
		while (i < NELEM(options)) {
			if (!strcmp(OFN(i), n))
				return (i);
			++i;
		}

	return ((size_t)-1);
}

struct options_info {
	int opt_width;
	int opts[NELEM(options)];
};

static void options_fmt_entry(char *, size_t, unsigned int, const void *);
static void printoptions(bool);

/* format a single select menu item */
static void
options_fmt_entry(char *buf, size_t buflen, unsigned int i, const void *arg)
{
	const struct options_info *oi = (const struct options_info *)arg;

	shf_snprintf(buf, buflen, "%-*s %s",
	    oi->opt_width, OFN(oi->opts[i]),
	    Flag(oi->opts[i]) ? "on" : "off");
}

static void
printoptions(bool verbose)
{
	size_t i = 0;

	if (verbose) {
		size_t n = 0, len, octs = 0;
		struct options_info oi;

		/* verbose version */
		shf_puts("Current option settings\n", shl_stdout);

		oi.opt_width = 0;
		while (i < NELEM(options)) {
			if ((len = strlen(OFN(i)))) {
				oi.opts[n++] = i;
				if (len > octs)
					octs = len;
				len = utf_mbswidth(OFN(i));
				if ((int)len > oi.opt_width)
					oi.opt_width = (int)len;
			}
			++i;
		}
		print_columns(shl_stdout, n, options_fmt_entry, &oi,
		    octs + 4, oi.opt_width + 4, true);
	} else {
		/* short version like AT&T ksh93 */
		shf_puts(Tset, shl_stdout);
		while (i < NELEM(options)) {
			if (Flag(i) && OFN(i)[0])
				shprintf(" -o %s", OFN(i));
			++i;
		}
		shf_putc('\n', shl_stdout);
	}
}

char *
getoptions(void)
{
	size_t i = 0;
	char c, m[(int)FNFLAGS + 1];
	char *cp = m;

	while (i < NELEM(options)) {
		if ((c = OFC(i)) && Flag(i))
			*cp++ = c;
		++i;
	}
	strndupx(cp, m, cp - m, ATEMP);
	return (cp);
}

/* change a Flag(*) value; takes care of special actions */
void
change_flag(enum sh_flag f, int what, bool newset)
{
	unsigned char oldval;
	unsigned char newval = (newset ? 1 : 0);

	if (f == FXTRACE) {
		change_xtrace(newval, true);
		return;
	}
	oldval = Flag(f);
	Flag(f) = newval = (newset ? 1 : 0);
#ifndef MKSH_UNEMPLOYED
	if (f == FMONITOR) {
		if (what != OF_CMDLINE && newval != oldval)
			j_change();
	} else
#endif
#ifndef MKSH_NO_CMDLINE_EDITING
	  if ((
#if !MKSH_S_NOVI
	    f == FVI ||
#endif
	    f == FEMACS || f == FGMACS) && newval) {
#if !MKSH_S_NOVI
		Flag(FVI) =
#endif
		    Flag(FEMACS) = Flag(FGMACS) = 0;
		Flag(f) = newval;
	} else
#endif
	  if (f == FPRIVILEGED && oldval && !newval) {
		/* Turning off -p? */

		/*XXX this can probably be optimised */
		kshegid = kshgid = getgid();
		ksheuid = kshuid = getuid();
#if HAVE_SETRESUGID
		DO_SETUID(setresgid, (kshegid, kshegid, kshegid));
#if HAVE_SETGROUPS
		/* setgroups doesn't EAGAIN on Linux */
		setgroups(1, &kshegid);
#endif
		DO_SETUID(setresuid, (ksheuid, ksheuid, ksheuid));
#else /* !HAVE_SETRESUGID */
		/* setgid, setegid, seteuid don't EAGAIN on Linux */
		setgid(kshegid);
#ifndef MKSH__NO_SETEUGID
		setegid(kshegid);
#endif
		DO_SETUID(setuid, (ksheuid));
#ifndef MKSH__NO_SETEUGID
		seteuid(ksheuid);
#endif
#endif /* !HAVE_SETRESUGID */
	} else if ((f == FPOSIX || f == FSH) && newval) {
		/* Turning on -o posix or -o sh? */
		Flag(FBRACEEXPAND) = 0;
	} else if (f == FTALKING) {
		/* Changing interactive flag? */
		if ((what == OF_CMDLINE || what == OF_SET) && procpid == kshpid)
			Flag(FTALKING_I) = newval;
	}
}

void
change_xtrace(unsigned char newval, bool dosnapshot)
{
	static bool in_xtrace;

	if (in_xtrace)
		return;

	if (!dosnapshot && newval == Flag(FXTRACE))
		return;

	if (Flag(FXTRACE) == 2) {
		shf_putc('\n', shl_xtrace);
		Flag(FXTRACE) = 1;
		shf_flush(shl_xtrace);
	}

	if (!dosnapshot && Flag(FXTRACE) == 1)
		switch (newval) {
		case 1:
			return;
		case 2:
			goto changed_xtrace;
		}

	shf_flush(shl_xtrace);
	if (shl_xtrace->fd != 2)
		close(shl_xtrace->fd);
	if (!newval || (shl_xtrace->fd = savefd(2)) == -1)
		shl_xtrace->fd = 2;

 changed_xtrace:
	if ((Flag(FXTRACE) = newval) == 2) {
		in_xtrace = true;
		Flag(FXTRACE) = 0;
		shf_puts(substitute(str_val(global("PS4")), 0), shl_xtrace);
		Flag(FXTRACE) = 2;
		in_xtrace = false;
	}
}

/*
 * Parse command line and set command arguments. Returns the index of
 * non-option arguments, -1 if there is an error.
 */
int
parse_args(const char **argv,
    /* OF_CMDLINE or OF_SET */
    int what,
    bool *setargsp)
{
	static const char cmd_opts[] =
#define SHFLAGS_NOT_SET
#define SHFLAGS_OPTCS
#include "sh_flags.gen"
#undef SHFLAGS_NOT_SET
	    ;
	static const char set_opts[] =
#define SHFLAGS_NOT_CMD
#define SHFLAGS_OPTCS
#include "sh_flags.gen"
#undef SHFLAGS_NOT_CMD
	    ;
	bool set;
	const char *opts;
	const char *array = NULL;
	Getopt go;
	size_t i;
	int optc, arrayset = 0;
	bool sortargs = false;
	bool fcompatseen = false;

	if (what == OF_CMDLINE) {
		const char *p = argv[0], *q;
		/*
		 * Set FLOGIN before parsing options so user can clear
		 * flag using +l.
		 */
		if (*p != '-')
			for (q = p; *q; )
				if (*q++ == '/')
					p = q;
		Flag(FLOGIN) = (*p == '-');
		opts = cmd_opts;
	} else if (what == OF_FIRSTTIME) {
		opts = cmd_opts;
	} else
		opts = set_opts;
	ksh_getopt_reset(&go, GF_ERROR|GF_PLUSOPT);
	while ((optc = ksh_getopt(argv, &go, opts)) != -1) {
		set = tobool(!(go.info & GI_PLUS));
		switch (optc) {
		case 'A':
			if (what == OF_FIRSTTIME)
				break;
			arrayset = set ? 1 : -1;
			array = go.optarg;
			break;

		case 'o':
			if (what == OF_FIRSTTIME)
				break;
			if (go.optarg == NULL) {
				/*
				 * lone -o: print options
				 *
				 * Note that on the command line, -o requires
				 * an option (ie, can't get here if what is
				 * OF_CMDLINE).
				 */
				printoptions(set);
				break;
			}
			i = option(go.optarg);
			if ((i == FPOSIX || i == FSH) && set && !fcompatseen) {
				/*
				 * If running 'set -o posix' or
				 * 'set -o sh', turn off the other;
				 * if running 'set -o posix -o sh'
				 * allow both to be set though.
				 */
				Flag(FPOSIX) = 0;
				Flag(FSH) = 0;
				fcompatseen = true;
			}
			if ((i != (size_t)-1) && (set ? 1U : 0U) == Flag(i))
				/*
				 * Don't check the context if the flag
				 * isn't changing - makes "set -o interactive"
				 * work if you're already interactive. Needed
				 * if the output of "set +o" is to be used.
				 */
				;
			else if ((i != (size_t)-1) && (OFF(i) & what))
				change_flag((enum sh_flag)i, what, set);
			else {
				bi_errorf("%s: %s", go.optarg, "bad option");
				return (-1);
			}
			break;

#ifdef KSH_CHVT_FLAG
		case 'T':
			if (what != OF_FIRSTTIME)
				break;
#ifndef KSH_CHVT_CODE
			errorf("no TIOCSCTTY ioctl");
#else
			change_flag(FTALKING, OF_CMDLINE, true);
			chvt(&go);
			break;
#endif
#endif

		case '?':
			return (-1);

		default:
			if (what == OF_FIRSTTIME)
				break;
			/* -s: sort positional params (AT&T ksh stupidity) */
			if (what == OF_SET && optc == 's') {
				sortargs = true;
				break;
			}
			for (i = 0; i < NELEM(options); i++)
				if (optc == OFC(i) &&
				    (what & OFF(i))) {
					change_flag((enum sh_flag)i, what, set);
					break;
				}
			if (i == NELEM(options))
				internal_errorf("parse_args: '%c'", optc);
		}
	}
	if (!(go.info & GI_MINUSMINUS) && argv[go.optind] &&
	    (argv[go.optind][0] == '-' || argv[go.optind][0] == '+') &&
	    argv[go.optind][1] == '\0') {
		/* lone - clears -v and -x flags */
		if (argv[go.optind][0] == '-') {
			Flag(FVERBOSE) = 0;
			change_xtrace(0, false);
		}
		/* set skips lone - or + option */
		go.optind++;
	}
	if (setargsp)
		/* -- means set $#/$* even if there are no arguments */
		*setargsp = !arrayset && ((go.info & GI_MINUSMINUS) ||
		    argv[go.optind]);

	if (arrayset) {
		const char *ccp = NULL;

		if (*array)
			ccp = skip_varname(array, false);
		if (!ccp || !(!ccp[0] || (ccp[0] == '+' && !ccp[1]))) {
			bi_errorf("%s: %s", array, "is not an identifier");
			return (-1);
		}
	}
	if (sortargs) {
		for (i = go.optind; argv[i]; i++)
			;
		qsort(&argv[go.optind], i - go.optind, sizeof(void *),
		    xstrcmp);
	}
	if (arrayset)
		go.optind += set_array(array, tobool(arrayset > 0),
		    argv + go.optind);

	return (go.optind);
}

/* parse a decimal number: returns 0 if string isn't a number, 1 otherwise */
int
getn(const char *s, int *ai)
{
	char c;
	mksh_ari_u num;
	bool neg = false;

	num.u = 0;

	do {
		c = *s++;
	} while (ksh_isspace(c));

	switch (c) {
	case '-':
		neg = true;
		/* FALLTHROUGH */
	case '+':
		c = *s++;
		break;
	}

	do {
		if (!ksh_isdigit(c))
			/* not numeric */
			return (0);
		if (num.u > 214748364U)
			/* overflow on multiplication */
			return (0);
		num.u = num.u * 10U + (unsigned int)ksh_numdig(c);
		/* now: num.u <= 2147483649U */
	} while ((c = *s++));

	if (num.u > (neg ? 2147483648U : 2147483647U))
		/* overflow for signed 32-bit int */
		return (0);

	if (neg)
		num.u = -num.u;
	*ai = num.i;
	return (1);
}

/**
 * pattern simplifications:
 * - @(x) -> x (not @(x|y) though)
 * - ** -> *
 */
static void *
simplify_gmatch_pattern(const unsigned char *sp)
{
	uint8_t c;
	unsigned char *cp, *dp;
	const unsigned char *ps, *se;

	cp = alloc(strlen((const void *)sp) + 1, ATEMP);
	goto simplify_gmatch_pat1a;

	/* foo@(b@(a)r)b@(a|a)z -> foobarb@(a|a)z */
 simplify_gmatch_pat1:
	sp = cp;
 simplify_gmatch_pat1a:
	dp = cp;
	se = sp + strlen((const void *)sp);
	while ((c = *sp++)) {
		if (!ISMAGIC(c)) {
			*dp++ = c;
			continue;
		}
		switch ((c = *sp++)) {
		case 0x80|'@':
		/* simile for @ */
		case 0x80|' ':
			/* check whether it has only one clause */
			ps = pat_scan(sp, se, true);
			if (!ps || ps[-1] != /*(*/ ')')
				/* nope */
				break;
			/* copy inner clause until matching close */
			ps -= 2;
			while ((const unsigned char *)sp < ps)
				*dp++ = *sp++;
			/* skip MAGIC and closing parenthesis */
			sp += 2;
			/* copy the rest of the pattern */
			memmove(dp, sp, strlen((const void *)sp) + 1);
			/* redo from start */
			goto simplify_gmatch_pat1;
		}
		*dp++ = MAGIC;
		*dp++ = c;
	}
	*dp = '\0';

	/* collapse adjacent asterisk wildcards */
	sp = dp = cp;
	while ((c = *sp++)) {
		if (!ISMAGIC(c)) {
			*dp++ = c;
			continue;
		}
		switch ((c = *sp++)) {
		case '*':
			while (ISMAGIC(sp[0]) && sp[1] == c)
				sp += 2;
			break;
		}
		*dp++ = MAGIC;
		*dp++ = c;
	}
	*dp = '\0';

	/* return the result, allocated from ATEMP */
	return (cp);
}

/* -------- gmatch.c -------- */

/*
 * int gmatch(string, pattern)
 * char *string, *pattern;
 *
 * Match a pattern as in sh(1).
 * pattern character are prefixed with MAGIC by expand.
 */
int
gmatchx(const char *s, const char *p, bool isfile)
{
	const char *se, *pe;
	char *pnew;
	int rv;

	if (s == NULL || p == NULL)
		return (0);

	se = s + strlen(s);
	pe = p + strlen(p);
	/*
	 * isfile is false iff no syntax check has been done on
	 * the pattern. If check fails, just to a strcmp().
	 */
	if (!isfile && !has_globbing(p, pe)) {
		size_t len = pe - p + 1;
		char tbuf[64];
		char *t = len <= sizeof(tbuf) ? tbuf : alloc(len, ATEMP);
		debunk(t, p, len);
		return (!strcmp(t, s));
	}

	/*
	 * since the do_gmatch() engine sucks so much, we must do some
	 * pattern simplifications
	 */
	pnew = simplify_gmatch_pattern((const unsigned char *)p);
	pe = pnew + strlen(pnew);

	rv = do_gmatch((const unsigned char *)s, (const unsigned char *)se,
	    (const unsigned char *)pnew, (const unsigned char *)pe);
	afree(pnew, ATEMP);
	return (rv);
}

/**
 * Returns if p is a syntacticly correct globbing pattern, false
 * if it contains no pattern characters or if there is a syntax error.
 * Syntax errors are:
 *	- [ with no closing ]
 *	- imbalanced $(...) expression
 *	- [...] and *(...) not nested (eg, [a$(b|]c), *(a[b|c]d))
 */
/*XXX
 * - if no magic,
 *	if dest given, copy to dst
 *	return ?
 * - if magic && (no globbing || syntax error)
 *	debunk to dst
 *	return ?
 * - return ?
 */
int
has_globbing(const char *xp, const char *xpe)
{
	const unsigned char *p = (const unsigned char *) xp;
	const unsigned char *pe = (const unsigned char *) xpe;
	int c;
	int nest = 0, bnest = 0;
	bool saw_glob = false;
	/* inside [...] */
	bool in_bracket = false;

	for (; p < pe; p++) {
		if (!ISMAGIC(*p))
			continue;
		if ((c = *++p) == '*' || c == '?')
			saw_glob = true;
		else if (c == '[') {
			if (!in_bracket) {
				saw_glob = true;
				in_bracket = true;
				if (ISMAGIC(p[1]) && p[2] == '!')
					p += 2;
				if (ISMAGIC(p[1]) && p[2] == ']')
					p += 2;
			}
			/*XXX Do we need to check ranges here? POSIX Q */
		} else if (c == ']') {
			if (in_bracket) {
				if (bnest)
					/* [a*(b]) */
					return (0);
				in_bracket = false;
			}
		} else if ((c & 0x80) && vstrchr("*+?@! ", c & 0x7f)) {
			saw_glob = true;
			if (in_bracket)
				bnest++;
			else
				nest++;
		} else if (c == '|') {
			if (in_bracket && !bnest)
				/* *(a[foo|bar]) */
				return (0);
		} else if (c == /*(*/ ')') {
			if (in_bracket) {
				if (!bnest--)
					/* *(a[b)c] */
					return (0);
			} else if (nest)
				nest--;
		}
		/*
		 * else must be a MAGIC-MAGIC, or MAGIC-!,
		 * MAGIC--, MAGIC-], MAGIC-{, MAGIC-, MAGIC-}
		 */
	}
	return (saw_glob && !in_bracket && !nest);
}

/* Function must return either 0 or 1 (assumed by code for 0x80|'!') */
static int
do_gmatch(const unsigned char *s, const unsigned char *se,
    const unsigned char *p, const unsigned char *pe)
{
	unsigned char sc, pc;
	const unsigned char *prest, *psub, *pnext;
	const unsigned char *srest;

	if (s == NULL || p == NULL)
		return (0);
	while (p < pe) {
		pc = *p++;
		sc = s < se ? *s : '\0';
		s++;
		if (!ISMAGIC(pc)) {
			if (sc != pc)
				return (0);
			continue;
		}
		switch (*p++) {
		case '[':
			if (sc == 0 || (p = gmatch_cclass(p, sc)) == NULL)
				return (0);
			break;

		case '?':
			if (sc == 0)
				return (0);
			if (UTFMODE) {
				--s;
				s += utf_ptradj((const void *)s);
			}
			break;

		case '*':
			if (p == pe)
				return (1);
			s--;
			do {
				if (do_gmatch(s, se, p, pe))
					return (1);
			} while (s++ < se);
			return (0);

		/**
		 * [*+?@!](pattern|pattern|..)
		 * This is also needed for ${..%..}, etc.
		 */

		/* matches one or more times */
		case 0x80|'+':
		/* matches zero or more times */
		case 0x80|'*':
			if (!(prest = pat_scan(p, pe, false)))
				return (0);
			s--;
			/* take care of zero matches */
			if (p[-1] == (0x80 | '*') &&
			    do_gmatch(s, se, prest, pe))
				return (1);
			for (psub = p; ; psub = pnext) {
				pnext = pat_scan(psub, pe, true);
				for (srest = s; srest <= se; srest++) {
					if (do_gmatch(s, srest, psub, pnext - 2) &&
					    (do_gmatch(srest, se, prest, pe) ||
					    (s != srest && do_gmatch(srest,
					    se, p - 2, pe))))
						return (1);
				}
				if (pnext == prest)
					break;
			}
			return (0);

		/* matches zero or once */
		case 0x80|'?':
		/* matches one of the patterns */
		case 0x80|'@':
		/* simile for @ */
		case 0x80|' ':
			if (!(prest = pat_scan(p, pe, false)))
				return (0);
			s--;
			/* Take care of zero matches */
			if (p[-1] == (0x80 | '?') &&
			    do_gmatch(s, se, prest, pe))
				return (1);
			for (psub = p; ; psub = pnext) {
				pnext = pat_scan(psub, pe, true);
				srest = prest == pe ? se : s;
				for (; srest <= se; srest++) {
					if (do_gmatch(s, srest, psub, pnext - 2) &&
					    do_gmatch(srest, se, prest, pe))
						return (1);
				}
				if (pnext == prest)
					break;
			}
			return (0);

		/* matches none of the patterns */
		case 0x80|'!':
			if (!(prest = pat_scan(p, pe, false)))
				return (0);
			s--;
			for (srest = s; srest <= se; srest++) {
				int matched = 0;

				for (psub = p; ; psub = pnext) {
					pnext = pat_scan(psub, pe, true);
					if (do_gmatch(s, srest, psub,
					    pnext - 2)) {
						matched = 1;
						break;
					}
					if (pnext == prest)
						break;
				}
				if (!matched &&
				    do_gmatch(srest, se, prest, pe))
					return (1);
			}
			return (0);

		default:
			if (sc != p[-1])
				return (0);
			break;
		}
	}
	return (s == se);
}

static const unsigned char *
gmatch_cclass(const unsigned char *p, unsigned char sub)
{
	unsigned char c, d;
	bool notp, found = false;
	const unsigned char *orig_p = p;

	if ((notp = tobool(ISMAGIC(*p) && *++p == '!')))
		p++;
	do {
		c = *p++;
		if (ISMAGIC(c)) {
			c = *p++;
			if ((c & 0x80) && !ISMAGIC(c)) {
				/* extended pattern matching: *+?@! */
				c &= 0x7F;
				/* XXX the ( char isn't handled as part of [] */
				if (c == ' ')
					/* simile for @: plain (..) */
					c = '(' /*)*/;
			}
		}
		if (c == '\0')
			/* No closing ] - act as if the opening [ was quoted */
			return (sub == '[' ? orig_p : NULL);
		if (ISMAGIC(p[0]) && p[1] == '-' &&
		    (!ISMAGIC(p[2]) || p[3] != ']')) {
			/* MAGIC- */
			p += 2;
			d = *p++;
			if (ISMAGIC(d)) {
				d = *p++;
				if ((d & 0x80) && !ISMAGIC(d))
					d &= 0x7f;
			}
			/* POSIX says this is an invalid expression */
			if (c > d)
				return (NULL);
		} else
			d = c;
		if (c == sub || (c <= sub && sub <= d))
			found = true;
	} while (!(ISMAGIC(p[0]) && p[1] == ']'));

	return ((found != notp) ? p+2 : NULL);
}

/* Look for next ) or | (if match_sep) in *(foo|bar) pattern */
static const unsigned char *
pat_scan(const unsigned char *p, const unsigned char *pe, bool match_sep)
{
	int nest = 0;

	for (; p < pe; p++) {
		if (!ISMAGIC(*p))
			continue;
		if ((*++p == /*(*/ ')' && nest-- == 0) ||
		    (*p == '|' && match_sep && nest == 0))
			return (p + 1);
		if ((*p & 0x80) && vstrchr("*+?@! ", *p & 0x7f))
			nest++;
	}
	return (NULL);
}

int
xstrcmp(const void *p1, const void *p2)
{
	return (strcmp(*(const char * const *)p1, *(const char * const *)p2));
}

/* Initialise a Getopt structure */
void
ksh_getopt_reset(Getopt *go, int flags)
{
	go->optind = 1;
	go->optarg = NULL;
	go->p = 0;
	go->flags = flags;
	go->info = 0;
	go->buf[1] = '\0';
}


/**
 * getopt() used for shell built-in commands, the getopts command, and
 * command line options.
 * A leading ':' in options means don't print errors, instead return '?'
 * or ':' and set go->optarg to the offending option character.
 * If GF_ERROR is set (and option doesn't start with :), errors result in
 * a call to bi_errorf().
 *
 * Non-standard features:
 *	- ';' is like ':' in options, except the argument is optional
 *	  (if it isn't present, optarg is set to 0).
 *	  Used for 'set -o'.
 *	- ',' is like ':' in options, except the argument always immediately
 *	  follows the option character (optarg is set to the null string if
 *	  the option is missing).
 *	  Used for 'read -u2', 'print -u2' and fc -40.
 *	- '#' is like ':' in options, expect that the argument is optional
 *	  and must start with a digit. If the argument doesn't start with a
 *	  digit, it is assumed to be missing and normal option processing
 *	  continues (optarg is set to 0 if the option is missing).
 *	  Used for 'typeset -LZ4'.
 *	- accepts +c as well as -c IF the GF_PLUSOPT flag is present. If an
 *	  option starting with + is accepted, the GI_PLUS flag will be set
 *	  in go->info.
 */
int
ksh_getopt(const char **argv, Getopt *go, const char *optionsp)
{
	char c;
	const char *o;

	if (go->p == 0 || (c = argv[go->optind - 1][go->p]) == '\0') {
		const char *arg = argv[go->optind], flag = arg ? *arg : '\0';

		go->p = 1;
		if (flag == '-' && ksh_isdash(arg + 1)) {
			go->optind++;
			go->p = 0;
			go->info |= GI_MINUSMINUS;
			return (-1);
		}
		if (arg == NULL ||
		    ((flag != '-' ) &&
		    /* neither a - nor a + (if + allowed) */
		    (!(go->flags & GF_PLUSOPT) || flag != '+')) ||
		    (c = arg[1]) == '\0') {
			go->p = 0;
			return (-1);
		}
		go->optind++;
		go->info &= ~(GI_MINUS|GI_PLUS);
		go->info |= flag == '-' ? GI_MINUS : GI_PLUS;
	}
	go->p++;
	if (c == '?' || c == ':' || c == ';' || c == ',' || c == '#' ||
	    !(o = cstrchr(optionsp, c))) {
		if (optionsp[0] == ':') {
			go->buf[0] = c;
			go->optarg = go->buf;
		} else {
			warningf(true, "%s%s-%c: %s",
			    (go->flags & GF_NONAME) ? "" : argv[0],
			    (go->flags & GF_NONAME) ? "" : ": ", c,
			    "unknown option");
			if (go->flags & GF_ERROR)
				bi_errorfz();
		}
		return ('?');
	}
	/**
	 * : means argument must be present, may be part of option argument
	 *   or the next argument
	 * ; same as : but argument may be missing
	 * , means argument is part of option argument, and may be null.
	 */
	if (*++o == ':' || *o == ';') {
		if (argv[go->optind - 1][go->p])
			go->optarg = argv[go->optind - 1] + go->p;
		else if (argv[go->optind])
			go->optarg = argv[go->optind++];
		else if (*o == ';')
			go->optarg = NULL;
		else {
			if (optionsp[0] == ':') {
				go->buf[0] = c;
				go->optarg = go->buf;
				return (':');
			}
			warningf(true, "%s%s-%c: %s",
			    (go->flags & GF_NONAME) ? "" : argv[0],
			    (go->flags & GF_NONAME) ? "" : ": ", c,
			    "requires an argument");
			if (go->flags & GF_ERROR)
				bi_errorfz();
			return ('?');
		}
		go->p = 0;
	} else if (*o == ',') {
		/* argument is attached to option character, even if null */
		go->optarg = argv[go->optind - 1] + go->p;
		go->p = 0;
	} else if (*o == '#') {
		/*
		 * argument is optional and may be attached or unattached
		 * but must start with a digit. optarg is set to 0 if the
		 * argument is missing.
		 */
		if (argv[go->optind - 1][go->p]) {
			if (ksh_isdigit(argv[go->optind - 1][go->p])) {
				go->optarg = argv[go->optind - 1] + go->p;
				go->p = 0;
			} else
				go->optarg = NULL;
		} else {
			if (argv[go->optind] && ksh_isdigit(argv[go->optind][0])) {
				go->optarg = argv[go->optind++];
				go->p = 0;
			} else
				go->optarg = NULL;
		}
	}
	return (c);
}

/*
 * print variable/alias value using necessary quotes
 * (POSIX says they should be suitable for re-entry...)
 * No trailing newline is printed.
 */
void
print_value_quoted(struct shf *shf, const char *s)
{
	unsigned char c;
	const unsigned char *p = (const unsigned char *)s;
	bool inquote = true;

	/* first, check whether any quotes are needed */
	while ((c = *p++) >= 32)
		if (ctype(c, C_QUOTE))
			inquote = false;

	p = (const unsigned char *)s;
	if (c == 0) {
		if (inquote) {
			/* nope, use the shortcut */
			shf_puts(s, shf);
			return;
		}

		/* otherwise, quote nicely via state machine */
		while ((c = *p++) != 0) {
			if (c == '\'') {
				/*
				 * multiple single quotes or any of them
				 * at the beginning of a string look nicer
				 * this way than when simply substituting
				 */
				if (inquote) {
					shf_putc('\'', shf);
					inquote = false;
				}
				shf_putc('\\', shf);
			} else if (!inquote) {
				shf_putc('\'', shf);
				inquote = true;
			}
			shf_putc(c, shf);
		}
	} else {
		unsigned int wc;
		size_t n;

		/* use $'...' quote format */
		shf_putc('$', shf);
		shf_putc('\'', shf);
		while ((c = *p) != 0) {
			if (c >= 0xC2) {
				n = utf_mbtowc(&wc, (const char *)p);
				if (n != (size_t)-1) {
					p += n;
					shf_fprintf(shf, "\\u%04X", wc);
					continue;
				}
			}
			++p;
			switch (c) {
			/* see unbksl() in this file for comments */
			case 7:
				c = 'a';
				if (0)
					/* FALLTHROUGH */
			case '\b':
				  c = 'b';
				if (0)
					/* FALLTHROUGH */
			case '\f':
				  c = 'f';
				if (0)
					/* FALLTHROUGH */
			case '\n':
				  c = 'n';
				if (0)
					/* FALLTHROUGH */
			case '\r':
				  c = 'r';
				if (0)
					/* FALLTHROUGH */
			case '\t':
				  c = 't';
				if (0)
					/* FALLTHROUGH */
			case 11:
				  c = 'v';
				if (0)
					/* FALLTHROUGH */
			case '\033':
				/* take E not e because \e is \ in *roff */
				  c = 'E';
				/* FALLTHROUGH */
			case '\\':
				shf_putc('\\', shf);

				if (0)
					/* FALLTHROUGH */
			default:
				  if (c < 32 || c > 0x7E) {
					/* FALLTHROUGH */
			case '\'':
					shf_fprintf(shf, "\\%03o", c);
					break;
				}

				shf_putc(c, shf);
				break;
			}
		}
		inquote = true;
	}
	if (inquote)
		shf_putc('\'', shf);
}

/*
 * Print things in columns and rows - func() is called to format
 * the i-th element
 */
void
print_columns(struct shf *shf, unsigned int n,
    void (*func)(char *, size_t, unsigned int, const void *),
    const void *arg, size_t max_oct, size_t max_colz, bool prefcol)
{
	unsigned int i, r, c, rows, cols, nspace, max_col;
	char *str;

	if (!n)
		return;

	if (max_colz > 2147483646) {
#ifndef MKSH_SMALL
		internal_warningf("print_columns called with %s=%zu >= INT_MAX",
		    "max_col", max_colz);
#endif
		return;
	}
	max_col = (unsigned int)max_colz;

	if (max_oct > 2147483646) {
#ifndef MKSH_SMALL
		internal_warningf("print_columns called with %s=%zu >= INT_MAX",
		    "max_oct", max_oct);
#endif
		return;
	}
	++max_oct;
	str = alloc(max_oct, ATEMP);

	/*
	 * We use (max_col + 2) to consider the separator space.
	 * Note that no spaces are printed after the last column
	 * to avoid problems with terminals that have auto-wrap,
	 * but we need to also take this into account in x_cols.
	 */
	cols = (x_cols + 1) / (max_col + 2);

	/* if we can only print one column anyway, skip the goo */
	if (cols < 2) {
		for (i = 0; i < n; ++i) {
			(*func)(str, max_oct, i, arg);
			shf_puts(str, shf);
			shf_putc('\n', shf);
		}
		goto out;
	}

	rows = (n + cols - 1) / cols;
	if (prefcol && cols > rows) {
		cols = rows;
		rows = (n + cols - 1) / cols;
	}

	nspace = (x_cols - max_col * cols) / cols;
	if (nspace < 2)
		nspace = 2;
	max_col = -max_col;
	for (r = 0; r < rows; r++) {
		for (c = 0; c < cols; c++) {
			if ((i = c * rows + r) >= n)
				break;
			(*func)(str, max_oct, i, arg);
			if (i + rows >= n)
				shf_puts(str, shf);
			else
				shf_fprintf(shf, "%*s%*s",
				    max_col, str, nspace, null);
		}
		shf_putchar('\n', shf);
	}
 out:
	afree(str, ATEMP);
}

/* strip all NUL bytes from buf; output is NUL-terminated if stripped */
void
strip_nuls(char *buf, size_t len)
{
	char *cp, *dp, *ep;

	if (!len || !(dp = memchr(buf, '\0', len)))
		return;

	ep = buf + len;
	cp = dp;

 cp_has_nul_byte:
	while (cp++ < ep && *cp == '\0')
		;	/* nothing */
	while (cp < ep && *cp != '\0')
		*dp++ = *cp++;
	if (cp < ep)
		goto cp_has_nul_byte;

	*dp = '\0';
}

/*
 * Like read(2), but if read fails due to non-blocking flag,
 * resets flag and restarts read.
 */
ssize_t
blocking_read(int fd, char *buf, size_t nbytes)
{
	ssize_t ret;
	bool tried_reset = false;

	while ((ret = read(fd, buf, nbytes)) < 0) {
		if (!tried_reset && errno == EAGAIN) {
			if (reset_nonblock(fd) > 0) {
				tried_reset = true;
				continue;
			}
			errno = EAGAIN;
		}
		break;
	}
	return (ret);
}

/*
 * Reset the non-blocking flag on the specified file descriptor.
 * Returns -1 if there was an error, 0 if non-blocking wasn't set,
 * 1 if it was.
 */
int
reset_nonblock(int fd)
{
	int flags;

	if ((flags = fcntl(fd, F_GETFL, 0)) < 0)
		return (-1);
	if (!(flags & O_NONBLOCK))
		return (0);
	flags &= ~O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0)
		return (-1);
	return (1);
}

/* getcwd(3) equivalent, allocates from ATEMP but doesn't resize */
char *
ksh_get_wd(void)
{
#ifdef MKSH__NO_PATH_MAX
	char *rv, *cp;

	if ((cp = get_current_dir_name())) {
		strdupx(rv, cp, ATEMP);
		free_gnu_gcdn(cp);
	} else
		rv = NULL;
#else
	char *rv;

	if (!getcwd((rv = alloc(PATH_MAX + 1, ATEMP)), PATH_MAX)) {
		afree(rv, ATEMP);
		rv = NULL;
	}
#endif

	return (rv);
}

#ifndef ELOOP
#define ELOOP		E2BIG
#endif

char *
do_realpath(const char *upath)
{
	char *xp, *ip, *tp, *ipath, *ldest = NULL;
	XString xs;
	size_t pos, len;
	int llen;
	struct stat sb;
#ifdef MKSH__NO_PATH_MAX
	size_t ldestlen = 0;
#define pathlen sb.st_size
#define pathcnd (ldestlen < (pathlen + 1))
#else
#define pathlen PATH_MAX
#define pathcnd (!ldest)
#endif
	/* max. recursion depth */
	int symlinks = 32;

	if (mksh_abspath(upath)) {
		/* upath is an absolute pathname */
		strdupx(ipath, upath, ATEMP);
	} else {
		/* upath is a relative pathname, prepend cwd */
		if ((tp = ksh_get_wd()) == NULL || !mksh_abspath(tp))
			return (NULL);
		ipath = shf_smprintf("%s%s%s", tp, "/", upath);
		afree(tp, ATEMP);
	}

	/* ipath and upath are in memory at the same time -> unchecked */
	Xinit(xs, xp, strlen(ip = ipath) + 1, ATEMP);

	/* now jump into the deep of the loop */
	goto beginning_of_a_pathname;

	while (*ip) {
		/* skip slashes in input */
		while (*ip == '/')
			++ip;
		if (!*ip)
			break;

		/* get next pathname component from input */
		tp = ip;
		while (*ip && *ip != '/')
			++ip;
		len = ip - tp;

		/* check input for "." and ".." */
		if (tp[0] == '.') {
			if (len == 1)
				/* just continue with the next one */
				continue;
			else if (len == 2 && tp[1] == '.') {
				/* strip off last pathname component */
				while (xp > Xstring(xs, xp))
					if (*--xp == '/')
						break;
				/* then continue with the next one */
				continue;
			}
		}

		/* store output position away, then append slash to output */
		pos = Xsavepos(xs, xp);
		/* 1 for the '/' and len + 1 for tp and the NUL from below */
		XcheckN(xs, xp, 1 + len + 1);
		Xput(xs, xp, '/');

		/* append next pathname component to output */
		memcpy(xp, tp, len);
		xp += len;
		*xp = '\0';

		/* lstat the current output, see if it's a symlink */
		if (mksh_lstat(Xstring(xs, xp), &sb)) {
			/* lstat failed */
			if (errno == ENOENT) {
				/* because the pathname does not exist */
				while (*ip == '/')
					/* skip any trailing slashes */
					++ip;
				/* no more components left? */
				if (!*ip)
					/* we can still return successfully */
					break;
				/* more components left? fall through */
			}
			/* not ENOENT or not at the end of ipath */
			goto notfound;
		}

		/* check if we encountered a symlink? */
		if (S_ISLNK(sb.st_mode)) {
#ifndef MKSH__NO_SYMLINK
			/* reached maximum recursion depth? */
			if (!symlinks--) {
				/* yep, prevent infinite loops */
				errno = ELOOP;
				goto notfound;
			}

			/* get symlink(7) target */
			if (pathcnd) {
#ifdef MKSH__NO_PATH_MAX
				if (notoktoadd(pathlen, 1)) {
					errno = ENAMETOOLONG;
					goto notfound;
				}
#endif
				ldest = aresize(ldest, pathlen + 1, ATEMP);
			}
			llen = readlink(Xstring(xs, xp), ldest, pathlen);
			if (llen < 0)
				/* oops... */
				goto notfound;
			ldest[llen] = '\0';

			/*
			 * restart if symlink target is an absolute path,
			 * otherwise continue with currently resolved prefix
			 */
			/* append rest of current input path to link target */
			tp = shf_smprintf("%s%s%s", ldest, *ip ? "/" : "", ip);
			afree(ipath, ATEMP);
			ip = ipath = tp;
			if (!mksh_abspath(ldest)) {
				/* symlink target is a relative path */
				xp = Xrestpos(xs, xp, pos);
			} else
#endif
			  {
				/* symlink target is an absolute path */
				xp = Xstring(xs, xp);
 beginning_of_a_pathname:
				/* assert: (ip == ipath)[0] == '/' */
				/* assert: xp == xs.beg => start of path */

				/* exactly two leading slashes? (SUSv4 3.266) */
				if (ip[1] == '/' && ip[2] != '/') {
					/* keep them, e.g. for UNC pathnames */
					Xput(xs, xp, '/');
				}
			}
		}
		/* otherwise (no symlink) merely go on */
	}

	/*
	 * either found the target and successfully resolved it,
	 * or found its parent directory and may create it
	 */
	if (Xlength(xs, xp) == 0)
		/*
		 * if the resolved pathname is "", make it "/",
		 * otherwise do not add a trailing slash
		 */
		Xput(xs, xp, '/');
	Xput(xs, xp, '\0');

	/*
	 * if source path had a trailing slash, check if target path
	 * is not a non-directory existing file
	 */
	if (ip > ipath && ip[-1] == '/') {
		if (stat(Xstring(xs, xp), &sb)) {
			if (errno != ENOENT)
				goto notfound;
		} else if (!S_ISDIR(sb.st_mode)) {
			errno = ENOTDIR;
			goto notfound;
		}
		/* target now either does not exist or is a directory */
	}

	/* return target path */
	afree(ldest, ATEMP);
	afree(ipath, ATEMP);
	return (Xclose(xs, xp));

 notfound:
	/* save; freeing memory might trash it */
	llen = errno;
	afree(ldest, ATEMP);
	afree(ipath, ATEMP);
	Xfree(xs, xp);
	errno = llen;
	return (NULL);

#undef pathlen
#undef pathcnd
}

/**
 *	Makes a filename into result using the following algorithm.
 *	- make result NULL
 *	- if file starts with '/', append file to result & set cdpathp to NULL
 *	- if file starts with ./ or ../ append cwd and file to result
 *	  and set cdpathp to NULL
 *	- if the first element of cdpathp doesnt start with a '/' xx or '.' xx
 *	  then cwd is appended to result.
 *	- the first element of cdpathp is appended to result
 *	- file is appended to result
 *	- cdpathp is set to the start of the next element in cdpathp (or NULL
 *	  if there are no more elements.
 *	The return value indicates whether a non-null element from cdpathp
 *	was appended to result.
 */
static int
make_path(const char *cwd, const char *file,
    /* pointer to colon-separated list */
    char **cdpathp,
    XString *xsp,
    int *phys_pathp)
{
	int rval = 0;
	bool use_cdpath = true;
	char *plist;
	size_t len, plen = 0;
	char *xp = Xstring(*xsp, xp);

	if (!file)
		file = null;

	if (mksh_abspath(file)) {
		*phys_pathp = 0;
		use_cdpath = false;
	} else {
		if (file[0] == '.') {
			char c = file[1];

			if (c == '.')
				c = file[2];
			if (c == '/' || c == '\0')
				use_cdpath = false;
		}

		plist = *cdpathp;
		if (!plist)
			use_cdpath = false;
		else if (use_cdpath) {
			char *pend = plist;

			while (*pend && *pend != MKSH_PATHSEPC)
				++pend;
			plen = pend - plist;
			*cdpathp = *pend ? pend + 1 : NULL;
		}

		if ((!use_cdpath || !plen || !mksh_abspath(plist)) &&
		    (cwd && *cwd)) {
			len = strlen(cwd);
			XcheckN(*xsp, xp, len);
			memcpy(xp, cwd, len);
			xp += len;
			if (cwd[len - 1] != '/')
				Xput(*xsp, xp, '/');
		}
		*phys_pathp = Xlength(*xsp, xp);
		if (use_cdpath && plen) {
			XcheckN(*xsp, xp, plen);
			memcpy(xp, plist, plen);
			xp += plen;
			if (plist[plen - 1] != '/')
				Xput(*xsp, xp, '/');
			rval = 1;
		}
	}

	len = strlen(file) + 1;
	XcheckN(*xsp, xp, len);
	memcpy(xp, file, len);

	if (!use_cdpath)
		*cdpathp = NULL;

	return (rval);
}

/*-
 * Simplify pathnames containing "." and ".." entries.
 *
 * simplify_path(this)			= that
 * /a/b/c/./../d/..			/a/b
 * //./C/foo/bar/../baz			//C/foo/baz
 * /foo/				/foo
 * /foo/../../bar			/bar
 * /foo/./blah/..			/foo
 * .					.
 * ..					..
 * ./foo				foo
 * foo/../../../bar			../../bar
 */
void
simplify_path(char *p)
{
	char *dp, *ip, *sp, *tp;
	size_t len;
	bool needslash;

	switch (*p) {
	case 0:
		return;
	case '/':
		/* exactly two leading slashes? (SUSv4 3.266) */
		if (p[1] == '/' && p[2] != '/')
			/* keep them, e.g. for UNC pathnames */
			++p;
		needslash = true;
		break;
	default:
		needslash = false;
	}
	dp = ip = sp = p;

	while (*ip) {
		/* skip slashes in input */
		while (*ip == '/')
			++ip;
		if (!*ip)
			break;

		/* get next pathname component from input */
		tp = ip;
		while (*ip && *ip != '/')
			++ip;
		len = ip - tp;

		/* check input for "." and ".." */
		if (tp[0] == '.') {
			if (len == 1)
				/* just continue with the next one */
				continue;
			else if (len == 2 && tp[1] == '.') {
				/* parent level, but how? */
				if (mksh_abspath(p))
					/* absolute path, only one way */
					goto strip_last_component;
				else if (dp > sp) {
					/* relative path, with subpaths */
					needslash = false;
 strip_last_component:
					/* strip off last pathname component */
					while (dp > sp)
						if (*--dp == '/')
							break;
				} else {
					/* relative path, at its beginning */
					if (needslash)
						/* or already dotdot-slash'd */
						*dp++ = '/';
					/* keep dotdot-slash if not absolute */
					*dp++ = '.';
					*dp++ = '.';
					needslash = true;
					sp = dp;
				}
				/* then continue with the next one */
				continue;
			}
		}

		if (needslash)
			*dp++ = '/';

		/* append next pathname component to output */
		memmove(dp, tp, len);
		dp += len;

		/* append slash if we continue */
		needslash = true;
		/* try next component */
	}
	if (dp == p)
		/* empty path -> dot */
		*dp++ = needslash ? '/' : '.';
	*dp = '\0';
}

void
set_current_wd(const char *nwd)
{
	char *allocd = NULL;

	if (nwd == NULL) {
		allocd = ksh_get_wd();
		nwd = allocd ? allocd : null;
	}

	afree(current_wd, APERM);
	strdupx(current_wd, nwd, APERM);

	afree(allocd, ATEMP);
}

int
c_cd(const char **wp)
{
	int optc, rv, phys_path;
	bool physical = tobool(Flag(FPHYSICAL));
	/* was a node from cdpath added in? */
	int cdnode;
	/* show where we went?, error for $PWD */
	bool printpath = false, eflag = false;
	struct tbl *pwd_s, *oldpwd_s;
	XString xs;
	char *dir, *allocd = NULL, *tryp, *pwd, *cdpath;

	while ((optc = ksh_getopt(wp, &builtin_opt, "eLP")) != -1)
		switch (optc) {
		case 'e':
			eflag = true;
			break;
		case 'L':
			physical = false;
			break;
		case 'P':
			physical = true;
			break;
		case '?':
			return (2);
		}
	wp += builtin_opt.optind;

	if (Flag(FRESTRICTED)) {
		bi_errorf("restricted shell - can't cd");
		return (2);
	}

	pwd_s = global("PWD");
	oldpwd_s = global("OLDPWD");

	if (!wp[0]) {
		/* No arguments - go home */
		if ((dir = str_val(global("HOME"))) == null) {
			bi_errorf("no home directory (HOME not set)");
			return (2);
		}
	} else if (!wp[1]) {
		/* One argument: - or dir */
		strdupx(allocd, wp[0], ATEMP);
		if (ksh_isdash((dir = allocd))) {
			afree(allocd, ATEMP);
			allocd = NULL;
			dir = str_val(oldpwd_s);
			if (dir == null) {
				bi_errorf("no OLDPWD");
				return (2);
			}
			printpath = true;
		}
	} else if (!wp[2]) {
		/* Two arguments - substitute arg1 in PWD for arg2 */
		size_t ilen, olen, nlen, elen;
		char *cp;

		if (!current_wd[0]) {
			bi_errorf("can't determine current directory");
			return (2);
		}
		/*
		 * substitute arg1 for arg2 in current path.
		 * if the first substitution fails because the cd fails
		 * we could try to find another substitution. For now
		 * we don't
		 */
		if ((cp = strstr(current_wd, wp[0])) == NULL) {
			bi_errorf("bad substitution");
			return (2);
		}
		/*-
		 * ilen = part of current_wd before wp[0]
		 * elen = part of current_wd after wp[0]
		 * because current_wd and wp[1] need to be in memory at the
		 * same time beforehand the addition can stay unchecked
		 */
		ilen = cp - current_wd;
		olen = strlen(wp[0]);
		nlen = strlen(wp[1]);
		elen = strlen(current_wd + ilen + olen) + 1;
		dir = allocd = alloc(ilen + nlen + elen, ATEMP);
		memcpy(dir, current_wd, ilen);
		memcpy(dir + ilen, wp[1], nlen);
		memcpy(dir + ilen + nlen, current_wd + ilen + olen, elen);
		printpath = true;
	} else {
		bi_errorf("too many arguments");
		return (2);
	}

#ifdef MKSH__NO_PATH_MAX
	/* only a first guess; make_path will enlarge xs if necessary */
	XinitN(xs, 1024, ATEMP);
#else
	XinitN(xs, PATH_MAX, ATEMP);
#endif

	cdpath = str_val(global("CDPATH"));
	do {
		cdnode = make_path(current_wd, dir, &cdpath, &xs, &phys_path);
		if (physical)
			rv = chdir(tryp = Xstring(xs, xp) + phys_path);
		else {
			simplify_path(Xstring(xs, xp));
			rv = chdir(tryp = Xstring(xs, xp));
		}
	} while (rv < 0 && cdpath != NULL);

	if (rv < 0) {
		if (cdnode)
			bi_errorf("%s: %s", dir, "bad directory");
		else
			bi_errorf("%s: %s", tryp, cstrerror(errno));
		afree(allocd, ATEMP);
		Xfree(xs, xp);
		return (2);
	}

	rv = 0;

	/* allocd (above) => dir, which is no longer used */
	afree(allocd, ATEMP);
	allocd = NULL;

	/* Clear out tracked aliases with relative paths */
	flushcom(false);

	/*
	 * Set OLDPWD (note: unsetting OLDPWD does not disable this
	 * setting in AT&T ksh)
	 */
	if (current_wd[0])
		/* Ignore failure (happens if readonly or integer) */
		setstr(oldpwd_s, current_wd, KSH_RETURN_ERROR);

	if (!mksh_abspath(Xstring(xs, xp))) {
		pwd = NULL;
	} else if (!physical) {
		goto norealpath_PWD;
	} else if ((pwd = allocd = do_realpath(Xstring(xs, xp))) == NULL) {
		if (eflag)
			rv = 1;
 norealpath_PWD:
		pwd = Xstring(xs, xp);
	}

	/* Set PWD */
	if (pwd) {
		char *ptmp = pwd;

		set_current_wd(ptmp);
		/* Ignore failure (happens if readonly or integer) */
		setstr(pwd_s, ptmp, KSH_RETURN_ERROR);
	} else {
		set_current_wd(null);
		pwd = Xstring(xs, xp);
		/* XXX unset $PWD? */
		if (eflag)
			rv = 1;
	}
	if (printpath || cdnode)
		shprintf("%s\n", pwd);

	afree(allocd, ATEMP);
	Xfree(xs, xp);
	return (rv);
}


#ifdef KSH_CHVT_CODE
extern void chvt_reinit(void);

static void
chvt(const Getopt *go)
{
	const char *dv = go->optarg;
	char *cp = NULL;
	int fd;

	switch (*dv) {
	case '-':
		dv = "/dev/null";
		break;
	case '!':
		++dv;
		/* FALLTHROUGH */
	default: {
		struct stat sb;

		if (stat(dv, &sb)) {
			cp = shf_smprintf("/dev/ttyC%s", dv);
			dv = cp;
			if (stat(dv, &sb)) {
				memmove(cp + 1, cp, /* /dev/tty */ 8);
				dv = cp + 1;
				if (stat(dv, &sb)) {
					errorf("%s: %s: %s", "chvt",
					    "can't find tty", go->optarg);
				}
			}
		}
		if (!(sb.st_mode & S_IFCHR))
			errorf("%s: %s: %s", "chvt", "not a char device", dv);
#ifndef MKSH_DISABLE_REVOKE_WARNING
#if HAVE_REVOKE
		if (revoke(dv))
#endif
			warningf(false, "%s: %s %s", "chvt",
			    "new shell is potentially insecure, can't revoke",
			    dv);
#endif
	    }
	}
	if ((fd = binopen2(dv, O_RDWR)) < 0) {
		sleep(1);
		if ((fd = binopen2(dv, O_RDWR)) < 0) {
			errorf("%s: %s %s", "chvt", "can't open", dv);
		}
	}
	if (go->optarg[0] != '!') {
		switch (fork()) {
		case -1:
			errorf("%s: %s %s", "chvt", "fork", "failed");
		case 0:
			break;
		default:
			exit(0);
		}
	}
	if (setsid() == -1)
		errorf("%s: %s %s", "chvt", "setsid", "failed");
	if (go->optarg[0] != '-') {
		if (ioctl(fd, TIOCSCTTY, NULL) == -1)
			errorf("%s: %s %s", "chvt", "TIOCSCTTY", "failed");
		if (tcflush(fd, TCIOFLUSH))
			errorf("%s: %s %s", "chvt", "TCIOFLUSH", "failed");
	}
	ksh_dup2(fd, 0, false);
	ksh_dup2(fd, 1, false);
	ksh_dup2(fd, 2, false);
	if (fd > 2)
		close(fd);
	rndset((unsigned long)chvt_rndsetup(go, sizeof(Getopt)));
	chvt_reinit();
}
#endif

#ifdef DEBUG
char *
strchr(char *p, int ch)
{
	for (;; ++p) {
		if (*p == ch)
			return (p);
		if (!*p)
			return (NULL);
	}
	/* NOTREACHED */
}

char *
strstr(char *b, const char *l)
{
	char first, c;
	size_t n;

	if ((first = *l++) == '\0')
		return (b);
	n = strlen(l);
 strstr_look:
	while ((c = *b++) != first)
		if (c == '\0')
			return (NULL);
	if (strncmp(b, l, n))
		goto strstr_look;
	return (b - 1);
}
#endif

#if defined(MKSH_SMALL) && !defined(MKSH_SMALL_BUT_FAST)
char *
strndup_i(const char *src, size_t len, Area *ap)
{
	char *dst = NULL;

	if (src != NULL) {
		dst = alloc(len + 1, ap);
		memcpy(dst, src, len);
		dst[len] = '\0';
	}
	return (dst);
}

char *
strdup_i(const char *src, Area *ap)
{
	return (src == NULL ? NULL : strndup_i(src, strlen(src), ap));
}
#endif

#if !HAVE_GETRUSAGE
#define INVTCK(r,t)	do {						\
	r.tv_usec = ((t) % (1000000 / CLK_TCK)) * (1000000 / CLK_TCK);	\
	r.tv_sec = (t) / CLK_TCK;					\
} while (/* CONSTCOND */ 0)

int
getrusage(int what, struct rusage *ru)
{
	struct tms tms;
	clock_t u, s;

	if (/* ru == NULL || */ times(&tms) == (clock_t)-1)
		return (-1);

	switch (what) {
	case RUSAGE_SELF:
		u = tms.tms_utime;
		s = tms.tms_stime;
		break;
	case RUSAGE_CHILDREN:
		u = tms.tms_cutime;
		s = tms.tms_cstime;
		break;
	default:
		errno = EINVAL;
		return (-1);
	}
	INVTCK(ru->ru_utime, u);
	INVTCK(ru->ru_stime, s);
	return (0);
}
#endif

/*
 * process the string available via fg (get a char)
 * and fp (put back a char) for backslash escapes,
 * assuming the first call to *fg gets the char di-
 * rectly after the backslash; return the character
 * (0..0xFF), Unicode (wc + 0x100), or -1 if no known
 * escape sequence was found
 */
int
unbksl(bool cstyle, int (*fg)(void), void (*fp)(int))
{
	int wc, i, c, fc;

	fc = (*fg)();
	switch (fc) {
	case 'a':
		/*
		 * according to the comments in pdksh, \007 seems
		 * to be more portable than \a (due to HP-UX cc,
		 * Ultrix cc, old pcc, etc.) so we avoid the escape
		 * sequence altogether in mksh and assume ASCII
		 */
		wc = 7;
		break;
	case 'b':
		wc = '\b';
		break;
	case 'c':
		if (!cstyle)
			goto unknown_escape;
		c = (*fg)();
		wc = CTRL(c);
		break;
	case 'E':
	case 'e':
		wc = 033;
		break;
	case 'f':
		wc = '\f';
		break;
	case 'n':
		wc = '\n';
		break;
	case 'r':
		wc = '\r';
		break;
	case 't':
		wc = '\t';
		break;
	case 'v':
		/* assume ASCII here as well */
		wc = 11;
		break;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
		if (!cstyle)
			goto unknown_escape;
		/* FALLTHROUGH */
	case '0':
		if (cstyle)
			(*fp)(fc);
		/*
		 * look for an octal number with up to three
		 * digits, not counting the leading zero;
		 * convert it to a raw octet
		 */
		wc = 0;
		i = 3;
		while (i--)
			if ((c = (*fg)()) >= ord('0') && c <= ord('7'))
				wc = (wc << 3) + ksh_numdig(c);
			else {
				(*fp)(c);
				break;
			}
		break;
	case 'U':
		i = 8;
		if (/* CONSTCOND */ 0)
			/* FALLTHROUGH */
	case 'u':
		  i = 4;
		if (/* CONSTCOND */ 0)
			/* FALLTHROUGH */
	case 'x':
		  i = cstyle ? -1 : 2;
		/**
		 * x:	look for a hexadecimal number with up to
		 *	two (C style: arbitrary) digits; convert
		 *	to raw octet (C style: Unicode if >0xFF)
		 * u/U:	look for a hexadecimal number with up to
		 *	four (U: eight) digits; convert to Unicode
		 */
		wc = 0;
		while (i--) {
			wc <<= 4;
			if ((c = (*fg)()) >= ord('0') && c <= ord('9'))
				wc += ksh_numdig(c);
			else if (c >= ord('A') && c <= ord('F'))
				wc += ksh_numuc(c) + 10;
			else if (c >= ord('a') && c <= ord('f'))
				wc += ksh_numlc(c) + 10;
			else {
				wc >>= 4;
				(*fp)(c);
				break;
			}
		}
		if ((cstyle && wc > 0xFF) || fc != 'x')
			/* Unicode marker */
			wc += 0x100;
		break;
	case '\'':
		if (!cstyle)
			goto unknown_escape;
		wc = '\'';
		break;
	case '\\':
		wc = '\\';
		break;
	default:
 unknown_escape:
		(*fp)(fc);
		return (-1);
	}

	return (wc);
}

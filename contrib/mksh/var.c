/*	$OpenBSD: var.c,v 1.44 2015/09/10 11:37:42 jca Exp $	*/

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
#include "mirhash.h"

#if defined(__OpenBSD__)
#include <sys/sysctl.h>
#endif

__RCSID("$MirOS: src/bin/mksh/var.c,v 1.195 2015/10/09 16:11:19 tg Exp $");

/*-
 * Variables
 *
 * WARNING: unreadable code, needs a rewrite
 *
 * if (flag&INTEGER), val.i contains integer value, and type contains base.
 * otherwise, (val.s + type) contains string value.
 * if (flag&EXPORT), val.s contains "name=value" for E-Z exporting.
 */

static struct table specials;
static uint32_t lcg_state = 5381, qh_state = 4711;
/* may only be set by typeset() just before call to array_index_calc() */
static enum namerefflag innermost_refflag = SRF_NOP;

static char *formatstr(struct tbl *, const char *);
static void exportprep(struct tbl *, const char *);
static int special(const char *);
static void unspecial(const char *);
static void getspec(struct tbl *);
static void setspec(struct tbl *);
static void unsetspec(struct tbl *);
static int getint(struct tbl *, mksh_ari_u *, bool);
static const char *array_index_calc(const char *, bool *, uint32_t *);

/*
 * create a new block for function calls and simple commands
 * assume caller has allocated and set up e->loc
 */
void
newblock(void)
{
	struct block *l;
	static const char *empty[] = { null };

	l = alloc(sizeof(struct block), ATEMP);
	l->flags = 0;
	/* TODO: could use e->area (l->area => l->areap) */
	ainit(&l->area);
	if (!e->loc) {
		l->argc = 0;
		l->argv = empty;
	} else {
		l->argc = e->loc->argc;
		l->argv = e->loc->argv;
	}
	l->exit = l->error = NULL;
	ktinit(&l->area, &l->vars, 0);
	ktinit(&l->area, &l->funs, 0);
	l->next = e->loc;
	e->loc = l;
}

/*
 * pop a block handling special variables
 */
void
popblock(void)
{
	ssize_t i;
	struct block *l = e->loc;
	struct tbl *vp, **vpp = l->vars.tbls, *vq;

	/* pop block */
	e->loc = l->next;

	i = 1 << (l->vars.tshift);
	while (--i >= 0)
		if ((vp = *vpp++) != NULL && (vp->flag&SPECIAL)) {
			if ((vq = global(vp->name))->flag & ISSET)
				setspec(vq);
			else
				unsetspec(vq);
		}
	if (l->flags & BF_DOGETOPTS)
		user_opt = l->getopts_state;
	afreeall(&l->area);
	afree(l, ATEMP);
}

/* called by main() to initialise variable data structures */
#define VARSPEC_DEFNS
#include "var_spec.h"

enum var_specs {
#define VARSPEC_ENUMS
#include "var_spec.h"
	V_MAX
};

/* this is biased with -1 relative to VARSPEC_ENUMS */
static const char * const initvar_names[] = {
#define VARSPEC_ITEMS
#include "var_spec.h"
};

void
initvar(void)
{
	int i = 0;
	struct tbl *tp;

	ktinit(APERM, &specials,
	    /* currently 14 specials: 75% of 32 = 2^5 */
	    5);
	while (i < V_MAX - 1) {
		tp = ktenter(&specials, initvar_names[i],
		    hash(initvar_names[i]));
		tp->flag = DEFINED|ISSET;
		tp->type = ++i;
	}
}

/* common code for several functions below and c_typeset() */
struct block *
varsearch(struct block *l, struct tbl **vpp, const char *vn, uint32_t h)
{
	register struct tbl *vp;

	if (l) {
 varsearch_loop:
		if ((vp = ktsearch(&l->vars, vn, h)) != NULL)
			goto varsearch_out;
		if (l->next != NULL) {
			l = l->next;
			goto varsearch_loop;
		}
	}
	vp = NULL;
 varsearch_out:
	*vpp = vp;
	return (l);
}

/*
 * Used to calculate an array index for global()/local(). Sets *arrayp
 * to true if this is an array, sets *valp to the array index, returns
 * the basename of the array. May only be called from global()/local()
 * and must be their first callee.
 */
static const char *
array_index_calc(const char *n, bool *arrayp, uint32_t *valp)
{
	const char *p;
	size_t len;
	char *ap = NULL;

	*arrayp = false;
 redo_from_ref:
	p = skip_varname(n, false);
	if (innermost_refflag == SRF_NOP && (p != n) && ksh_isalphx(n[0])) {
		struct tbl *vp;
		char *vn;

		strndupx(vn, n, p - n, ATEMP);
		/* check if this is a reference */
		varsearch(e->loc, &vp, vn, hash(vn));
		afree(vn, ATEMP);
		if (vp && (vp->flag & (DEFINED | ASSOC | ARRAY)) ==
		    (DEFINED | ASSOC)) {
			char *cp;

			/* gotcha! */
			cp = shf_smprintf("%s%s", str_val(vp), p);
			afree(ap, ATEMP);
			n = ap = cp;
			goto redo_from_ref;
		}
	}
	innermost_refflag = SRF_NOP;

	if (p != n && *p == '[' && (len = array_ref_len(p))) {
		char *sub, *tmp;
		mksh_ari_t rval;

		/* calculate the value of the subscript */
		*arrayp = true;
		strndupx(tmp, p + 1, len - 2, ATEMP);
		sub = substitute(tmp, 0);
		afree(tmp, ATEMP);
		strndupx(n, n, p - n, ATEMP);
		evaluate(sub, &rval, KSH_UNWIND_ERROR, true);
		*valp = (uint32_t)rval;
		afree(sub, ATEMP);
	}
	return (n);
}

/*
 * Search for variable, if not found create globally.
 */
struct tbl *
global(const char *n)
{
	struct block *l = e->loc;
	struct tbl *vp;
	int c;
	bool array;
	uint32_t h, val;

	/*
	 * check to see if this is an array;
	 * dereference namerefs; must come first
	 */
	n = array_index_calc(n, &array, &val);
	h = hash(n);
	c = (unsigned char)n[0];
	if (!ksh_isalphx(c)) {
		if (array)
			errorf("bad substitution");
		vp = &vtemp;
		vp->flag = DEFINED;
		vp->type = 0;
		vp->areap = ATEMP;
		*vp->name = c;
		if (ksh_isdigit(c)) {
			if (getn(n, &c) && (c <= l->argc))
				/* setstr can't fail here */
				setstr(vp, l->argv[c], KSH_RETURN_ERROR);
			vp->flag |= RDONLY;
			return (vp);
		}
		vp->flag |= RDONLY;
		if (n[1] != '\0')
			return (vp);
		vp->flag |= ISSET|INTEGER;
		switch (c) {
		case '$':
			vp->val.i = kshpid;
			break;
		case '!':
			/* if no job, expand to nothing */
			if ((vp->val.i = j_async()) == 0)
				vp->flag &= ~(ISSET|INTEGER);
			break;
		case '?':
			vp->val.i = exstat & 0xFF;
			break;
		case '#':
			vp->val.i = l->argc;
			break;
		case '-':
			vp->flag &= ~INTEGER;
			vp->val.s = getoptions();
			break;
		default:
			vp->flag &= ~(ISSET|INTEGER);
		}
		return (vp);
	}
	l = varsearch(e->loc, &vp, n, h);
	if (vp != NULL)
		return (array ? arraysearch(vp, val) : vp);
	vp = ktenter(&l->vars, n, h);
	if (array)
		vp = arraysearch(vp, val);
	vp->flag |= DEFINED;
	if (special(n))
		vp->flag |= SPECIAL;
	return (vp);
}

/*
 * Search for local variable, if not found create locally.
 */
struct tbl *
local(const char *n, bool copy)
{
	struct block *l = e->loc;
	struct tbl *vp;
	bool array;
	uint32_t h, val;

	/*
	 * check to see if this is an array;
	 * dereference namerefs; must come first
	 */
	n = array_index_calc(n, &array, &val);
	h = hash(n);
	if (!ksh_isalphx(*n)) {
		vp = &vtemp;
		vp->flag = DEFINED|RDONLY;
		vp->type = 0;
		vp->areap = ATEMP;
		return (vp);
	}
	vp = ktenter(&l->vars, n, h);
	if (copy && !(vp->flag & DEFINED)) {
		struct tbl *vq;

		varsearch(l->next, &vq, n, h);
		if (vq != NULL) {
			vp->flag |= vq->flag &
			    (EXPORT | INTEGER | RDONLY | LJUST | RJUST |
			    ZEROFIL | LCASEV | UCASEV_AL | INT_U | INT_L);
			if (vq->flag & INTEGER)
				vp->type = vq->type;
			vp->u2.field = vq->u2.field;
		}
	}
	if (array)
		vp = arraysearch(vp, val);
	vp->flag |= DEFINED;
	if (special(n))
		vp->flag |= SPECIAL;
	return (vp);
}

/* get variable string value */
char *
str_val(struct tbl *vp)
{
	char *s;

	if ((vp->flag&SPECIAL))
		getspec(vp);
	if (!(vp->flag&ISSET))
		/* special to dollar() */
		s = null;
	else if (!(vp->flag&INTEGER))
		/* string source */
		s = vp->val.s + vp->type;
	else {
		/* integer source */
		mksh_uari_t n;
		unsigned int base;
		/**
		 * worst case number length is when base == 2:
		 *	1 (minus) + 2 (base, up to 36) + 1 ('#') +
		 *	number of bits in the mksh_uari_t + 1 (NUL)
		 */
		char strbuf[1 + 2 + 1 + 8 * sizeof(mksh_uari_t) + 1];
		const char *digits = (vp->flag & UCASEV_AL) ?
		    digits_uc : digits_lc;

		s = strbuf + sizeof(strbuf);
		if (vp->flag & INT_U)
			n = vp->val.u;
		else
			n = (vp->val.i < 0) ? -vp->val.u : vp->val.u;
		base = (vp->type == 0) ? 10U : (unsigned int)vp->type;

		if (base == 1 && n == 0)
			base = 2;
		if (base == 1) {
			size_t sz = 1;

			*(s = strbuf) = '1';
			s[1] = '#';
			if (!UTFMODE || ((n & 0xFF80) == 0xEF80))
				/* OPTU-16 -> raw octet */
				s[2] = n & 0xFF;
			else
				sz = utf_wctomb(s + 2, n);
			s[2 + sz] = '\0';
		} else {
			*--s = '\0';
			do {
				*--s = digits[n % base];
				n /= base;
			} while (n != 0);
			if (base != 10) {
				*--s = '#';
				*--s = digits[base % 10];
				if (base >= 10)
					*--s = digits[base / 10];
			}
			if (!(vp->flag & INT_U) && vp->val.i < 0)
				*--s = '-';
		}
		if (vp->flag & (RJUST|LJUST))
			/* case already dealt with */
			s = formatstr(vp, s);
		else
			strdupx(s, s, ATEMP);
	}
	return (s);
}

/* set variable to string value */
int
setstr(struct tbl *vq, const char *s, int error_ok)
{
	char *salloc = NULL;
	bool no_ro_check = tobool(error_ok & 0x4);

	error_ok &= ~0x4;
	if ((vq->flag & RDONLY) && !no_ro_check) {
		warningf(true, "read-only: %s", vq->name);
		if (!error_ok)
			errorfxz(2);
		return (0);
	}
	if (!(vq->flag&INTEGER)) {
		/* string dest */
		if ((vq->flag&ALLOC)) {
#ifndef MKSH_SMALL
			/* debugging */
			if (s >= vq->val.s &&
			    s <= vq->val.s + strlen(vq->val.s)) {
				internal_errorf(
				    "setstr: %s=%s: assigning to self",
				    vq->name, s);
			}
#endif
			afree(vq->val.s, vq->areap);
		}
		vq->flag &= ~(ISSET|ALLOC);
		vq->type = 0;
		if (s && (vq->flag & (UCASEV_AL|LCASEV|LJUST|RJUST)))
			s = salloc = formatstr(vq, s);
		if ((vq->flag&EXPORT))
			exportprep(vq, s);
		else {
			strdupx(vq->val.s, s, vq->areap);
			vq->flag |= ALLOC;
		}
	} else {
		/* integer dest */
		if (!v_evaluate(vq, s, error_ok, true))
			return (0);
	}
	vq->flag |= ISSET;
	if ((vq->flag&SPECIAL))
		setspec(vq);
	afree(salloc, ATEMP);
	return (1);
}

/* set variable to integer */
void
setint(struct tbl *vq, mksh_ari_t n)
{
	if (!(vq->flag&INTEGER)) {
		struct tbl *vp = &vtemp;
		vp->flag = (ISSET|INTEGER);
		vp->type = 0;
		vp->areap = ATEMP;
		vp->val.i = n;
		/* setstr can't fail here */
		setstr(vq, str_val(vp), KSH_RETURN_ERROR);
	} else
		vq->val.i = n;
	vq->flag |= ISSET;
	if ((vq->flag&SPECIAL))
		setspec(vq);
}

static int
getint(struct tbl *vp, mksh_ari_u *nump, bool arith)
{
	mksh_uari_t c, num = 0, base = 10;
	const char *s;
	bool have_base = false, neg = false;

	if (vp->flag & SPECIAL)
		getspec(vp);
	/* XXX is it possible for ISSET to be set and val.s to be NULL? */
	if (!(vp->flag & ISSET) || (!(vp->flag & INTEGER) && vp->val.s == NULL))
		return (-1);
	if (vp->flag & INTEGER) {
		nump->i = vp->val.i;
		return (vp->type);
	}
	s = vp->val.s + vp->type;

	do {
		c = (unsigned char)*s++;
	} while (ksh_isspace(c));

	switch (c) {
	case '-':
		neg = true;
		/* FALLTHROUGH */
	case '+':
		c = (unsigned char)*s++;
		break;
	}

	if (c == '0' && arith) {
		if (ksh_eq(s[0], 'X', 'x')) {
			/* interpret as hexadecimal */
			base = 16;
			++s;
			goto getint_c_style_base;
		} else if (Flag(FPOSIX) && ksh_isdigit(s[0]) &&
		    !(vp->flag & ZEROFIL)) {
			/* interpret as octal (deprecated) */
			base = 8;
 getint_c_style_base:
			have_base = true;
			c = (unsigned char)*s++;
		}
	}

	do {
		if (c == '#') {
			/* ksh-style base determination */
			if (have_base || num < 1)
				return (-1);
			if ((base = num) == 1) {
				/* mksh-specific extension */
				unsigned int wc;

				if (!UTFMODE)
					wc = *(const unsigned char *)s;
				else if (utf_mbtowc(&wc, s) == (size_t)-1)
					/* OPTU-8 -> OPTU-16 */
					/*
					 * (with a twist: 1#\uEF80 converts
					 * the same as 1#\x80 does, thus is
					 * not round-tripping correctly XXX)
					 */
					wc = 0xEF00 + *(const unsigned char *)s;
				nump->u = (mksh_uari_t)wc;
				return (1);
			} else if (base > 36)
				base = 10;
			num = 0;
			have_base = true;
			continue;
		}
		if (ksh_isdigit(c))
			c = ksh_numdig(c);
		else if (ksh_isupper(c))
			c = ksh_numuc(c) + 10;
		else if (ksh_islower(c))
			c = ksh_numlc(c) + 10;
		else
			return (-1);
		if (c >= base)
			return (-1);
		/* handle overflow as truncation */
		num = num * base + c;
	} while ((c = (unsigned char)*s++));

	if (neg)
		num = -num;
	nump->u = num;
	return (base);
}

/*
 * convert variable vq to integer variable, setting its value from vp
 * (vq and vp may be the same)
 */
struct tbl *
setint_v(struct tbl *vq, struct tbl *vp, bool arith)
{
	int base;
	mksh_ari_u num;

	if ((base = getint(vp, &num, arith)) == -1)
		return (NULL);
	setint_n(vq, num.i, 0);
	if (vq->type == 0)
		/* default base */
		vq->type = base;
	return (vq);
}

/* convert variable vq to integer variable, setting its value to num */
void
setint_n(struct tbl *vq, mksh_ari_t num, int newbase)
{
	if (!(vq->flag & INTEGER) && (vq->flag & ALLOC)) {
		vq->flag &= ~ALLOC;
		vq->type = 0;
		afree(vq->val.s, vq->areap);
	}
	vq->val.i = num;
	if (newbase != 0)
		vq->type = newbase;
	vq->flag |= ISSET|INTEGER;
	if (vq->flag&SPECIAL)
		setspec(vq);
}

static char *
formatstr(struct tbl *vp, const char *s)
{
	int olen, nlen;
	char *p, *q;
	size_t psiz;

	olen = (int)utf_mbswidth(s);

	if (vp->flag & (RJUST|LJUST)) {
		if (!vp->u2.field)
			/* default field width */
			vp->u2.field = olen;
		nlen = vp->u2.field;
	} else
		nlen = olen;

	p = alloc((psiz = nlen * /* MB_LEN_MAX */ 3 + 1), ATEMP);
	if (vp->flag & (RJUST|LJUST)) {
		int slen = olen, i = 0;

		if (vp->flag & RJUST) {
			const char *qq = s;
			int n = 0;

			while (i < slen)
				i += utf_widthadj(qq, &qq);
			/* strip trailing spaces (AT&T uses qq[-1] == ' ') */
			while (qq > s && ksh_isspace(qq[-1])) {
				--qq;
				--slen;
			}
			if (vp->flag & ZEROFIL && vp->flag & INTEGER) {
				if (!s[0] || !s[1])
					goto uhm_no;
				if (s[1] == '#')
					n = 2;
				else if (s[2] == '#')
					n = 3;
 uhm_no:
				if (vp->u2.field <= n)
					n = 0;
			}
			if (n) {
				memcpy(p, s, n);
				s += n;
			}
			while (slen > vp->u2.field)
				slen -= utf_widthadj(s, &s);
			if (vp->u2.field - slen)
				memset(p + n, (vp->flag & ZEROFIL) ? '0' : ' ',
				    vp->u2.field - slen);
			slen -= n;
			shf_snprintf(p + vp->u2.field - slen,
			    psiz - (vp->u2.field - slen),
			    "%.*s", slen, s);
		} else {
			/* strip leading spaces/zeros */
			while (ksh_isspace(*s))
				s++;
			if (vp->flag & ZEROFIL)
				while (*s == '0')
					s++;
			shf_snprintf(p, nlen + 1, "%-*.*s",
				vp->u2.field, vp->u2.field, s);
		}
	} else
		memcpy(p, s, strlen(s) + 1);

	if (vp->flag & UCASEV_AL) {
		for (q = p; *q; q++)
			*q = ksh_toupper(*q);
	} else if (vp->flag & LCASEV) {
		for (q = p; *q; q++)
			*q = ksh_tolower(*q);
	}

	return (p);
}

/*
 * make vp->val.s be "name=value" for quick exporting.
 */
static void
exportprep(struct tbl *vp, const char *val)
{
	char *xp;
	char *op = (vp->flag&ALLOC) ? vp->val.s : NULL;
	size_t namelen, vallen;

	namelen = strlen(vp->name);
	vallen = strlen(val) + 1;

	vp->flag |= ALLOC;
	/* since name+val are both in memory this can go unchecked */
	xp = alloc(namelen + 1 + vallen, vp->areap);
	memcpy(vp->val.s = xp, vp->name, namelen);
	xp += namelen;
	*xp++ = '=';
	/* offset to value */
	vp->type = xp - vp->val.s;
	memcpy(xp, val, vallen);
	afree(op, vp->areap);
}

/*
 * lookup variable (according to (set&LOCAL)), set its attributes
 * (INTEGER, RDONLY, EXPORT, TRACE, LJUST, RJUST, ZEROFIL, LCASEV,
 * UCASEV_AL), and optionally set its value if an assignment.
 */
struct tbl *
typeset(const char *var, uint32_t set, uint32_t clr, int field, int base)
{
	struct tbl *vp;
	struct tbl *vpbase, *t;
	char *tvar;
	const char *val;
	size_t len;
	bool vappend = false;
	enum namerefflag new_refflag = SRF_NOP;

	if ((set & (ARRAY | ASSOC)) == ASSOC) {
		new_refflag = SRF_ENABLE;
		set &= ~(ARRAY | ASSOC);
	}
	if ((clr & (ARRAY | ASSOC)) == ASSOC) {
		new_refflag = SRF_DISABLE;
		clr &= ~(ARRAY | ASSOC);
	}

	/* check for valid variable name, search for value */
	val = skip_varname(var, false);
	if (val == var) {
		/* no variable name given */
		return (NULL);
	}
	if (*val == '[') {
		if (new_refflag != SRF_NOP)
			errorf("%s: %s", var,
			    "reference variable can't be an array");
		len = array_ref_len(val);
		if (len == 0)
			return (NULL);
		/*
		 * IMPORT is only used when the shell starts up and is
		 * setting up its environment. Allow only simple array
		 * references at this time since parameter/command
		 * substitution is performed on the [expression] which
		 * would be a major security hole.
		 */
		if (set & IMPORT) {
			size_t i;

			for (i = 1; i < len - 1; i++)
				if (!ksh_isdigit(val[i]))
					return (NULL);
		}
		val += len;
	}
	if (val[0] == '=') {
		strndupx(tvar, var, val - var, ATEMP);
		++val;
	} else if (set & IMPORT) {
		/* environment invalid variable name or no assignment */
		return (NULL);
	} else if (val[0] == '+' && val[1] == '=') {
		strndupx(tvar, var, val - var, ATEMP);
		val += 2;
		vappend = true;
	} else if (val[0] != '\0') {
		/* other invalid variable names (not from environment) */
		return (NULL);
	} else {
		/* just varname with no value part nor equals sign */
		strdupx(tvar, var, ATEMP);
		val = NULL;
		/* handle foo[*] => foo (whole array) mapping for R39b */
		len = strlen(tvar);
		if (len > 3 && tvar[len - 3] == '[' && tvar[len - 2] == '*' &&
		    tvar[len - 1] == ']')
			tvar[len - 3] = '\0';
	}

	if (new_refflag == SRF_ENABLE) {
		const char *qval, *ccp;

		/* bail out on 'nameref foo+=bar' */
		if (vappend)
			errorf("appending not allowed for nameref");
		/* find value if variable already exists */
		if ((qval = val) == NULL) {
			varsearch(e->loc, &vp, tvar, hash(tvar));
			if (vp == NULL)
				goto nameref_empty;
			qval = str_val(vp);
		}
		/* check target value for being a valid variable name */
		ccp = skip_varname(qval, false);
		if (ccp == qval) {
			if (ksh_isdigit(qval[0])) {
				int c;

				if (getn(qval, &c))
					goto nameref_rhs_checked;
			} else if (qval[1] == '\0') switch (qval[0]) {
			case '$':
			case '!':
			case '?':
			case '#':
			case '-':
				goto nameref_rhs_checked;
			}
 nameref_empty:
			errorf("%s: %s", var, "empty nameref target");
		}
		len = (*ccp == '[') ? array_ref_len(ccp) : 0;
		if (ccp[len]) {
			/*
			 * works for cases "no array", "valid array with
			 * junk after it" and "invalid array"; in the
			 * latter case, len is also 0 and points to '['
			 */
			errorf("%s: %s", qval,
			    "nameref target not a valid parameter name");
		}
 nameref_rhs_checked:
		/* prevent nameref loops */
		while (qval) {
			if (!strcmp(qval, tvar))
				errorf("%s: %s", qval,
				    "expression recurses on parameter");
			varsearch(e->loc, &vp, qval, hash(qval));
			qval = NULL;
			if (vp && ((vp->flag & (ARRAY | ASSOC)) == ASSOC))
				qval = str_val(vp);
		}
	}

	/* prevent typeset from creating a local PATH/ENV/SHELL */
	if (Flag(FRESTRICTED) && (strcmp(tvar, "PATH") == 0 ||
	    strcmp(tvar, "ENV") == 0 || strcmp(tvar, "SHELL") == 0))
		errorf("%s: %s", tvar, "restricted");

	innermost_refflag = new_refflag;
	vp = (set & LOCAL) ? local(tvar, tobool(set & LOCAL_COPY)) :
	    global(tvar);
	if (new_refflag == SRF_DISABLE && (vp->flag & (ARRAY|ASSOC)) == ASSOC)
		vp->flag &= ~ASSOC;
	else if (new_refflag == SRF_ENABLE) {
		if (vp->flag & ARRAY) {
			struct tbl *a, *tmp;

			/* free up entire array */
			for (a = vp->u.array; a; ) {
				tmp = a;
				a = a->u.array;
				if (tmp->flag & ALLOC)
					afree(tmp->val.s, tmp->areap);
				afree(tmp, tmp->areap);
			}
			vp->u.array = NULL;
			vp->flag &= ~ARRAY;
		}
		vp->flag |= ASSOC;
	}

	set &= ~(LOCAL|LOCAL_COPY);

	vpbase = (vp->flag & ARRAY) ? global(arrayname(tvar)) : vp;

	/*
	 * only allow export flag to be set; AT&T ksh allows any
	 * attribute to be changed which means it can be truncated or
	 * modified (-L/-R/-Z/-i)
	 */
	if ((vpbase->flag & RDONLY) &&
	    (val || clr || (set & ~EXPORT)))
		/* XXX check calls - is error here ok by POSIX? */
		errorfx(2, "read-only: %s", tvar);
	afree(tvar, ATEMP);

	/* most calls are with set/clr == 0 */
	if (set | clr) {
		bool ok = true;

		/*
		 * XXX if x[0] isn't set, there will be problems: need
		 * to have one copy of attributes for arrays...
		 */
		for (t = vpbase; t; t = t->u.array) {
			bool fake_assign;
			char *s = NULL;
			char *free_me = NULL;

			fake_assign = (t->flag & ISSET) && (!val || t != vp) &&
			    ((set & (UCASEV_AL|LCASEV|LJUST|RJUST|ZEROFIL)) ||
			    ((t->flag & INTEGER) && (clr & INTEGER)) ||
			    (!(t->flag & INTEGER) && (set & INTEGER)));
			if (fake_assign) {
				if (t->flag & INTEGER) {
					s = str_val(t);
					free_me = NULL;
				} else {
					s = t->val.s + t->type;
					free_me = (t->flag & ALLOC) ? t->val.s :
					    NULL;
				}
				t->flag &= ~ALLOC;
			}
			if (!(t->flag & INTEGER) && (set & INTEGER)) {
				t->type = 0;
				t->flag &= ~ALLOC;
			}
			t->flag = (t->flag | set) & ~clr;
			/*
			 * Don't change base if assignment is to be
			 * done, in case assignment fails.
			 */
			if ((set & INTEGER) && base > 0 && (!val || t != vp))
				t->type = base;
			if (set & (LJUST|RJUST|ZEROFIL))
				t->u2.field = field;
			if (fake_assign) {
				if (!setstr(t, s, KSH_RETURN_ERROR)) {
					/*
					 * Somewhat arbitrary action
					 * here: zap contents of
					 * variable, but keep the flag
					 * settings.
					 */
					ok = false;
					if (t->flag & INTEGER)
						t->flag &= ~ISSET;
					else {
						if (t->flag & ALLOC)
							afree(t->val.s, t->areap);
						t->flag &= ~(ISSET|ALLOC);
						t->type = 0;
					}
				}
				afree(free_me, t->areap);
			}
		}
		if (!ok)
			errorfz();
	}

	if (val != NULL) {
		char *tval;

		if (vappend) {
			tval = shf_smprintf("%s%s", str_val(vp), val);
			val = tval;
		} else
			tval = NULL;

		if (vp->flag&INTEGER) {
			/* do not zero base before assignment */
			setstr(vp, val, KSH_UNWIND_ERROR | 0x4);
			/* done after assignment to override default */
			if (base > 0)
				vp->type = base;
		} else
			/* setstr can't fail (readonly check already done) */
			setstr(vp, val, KSH_RETURN_ERROR | 0x4);

		afree(tval, ATEMP);
	}

	/* only x[0] is ever exported, so use vpbase */
	if ((vpbase->flag&EXPORT) && !(vpbase->flag&INTEGER) &&
	    vpbase->type == 0)
		exportprep(vpbase, (vpbase->flag&ISSET) ? vpbase->val.s : null);

	return (vp);
}

/**
 * Unset a variable. The flags can be:
 * |1	= tear down entire array
 * |2	= keep attributes, only unset content
 */
void
unset(struct tbl *vp, int flags)
{
	if (vp->flag & ALLOC)
		afree(vp->val.s, vp->areap);
	if ((vp->flag & ARRAY) && (flags & 1)) {
		struct tbl *a, *tmp;

		/* free up entire array */
		for (a = vp->u.array; a; ) {
			tmp = a;
			a = a->u.array;
			if (tmp->flag & ALLOC)
				afree(tmp->val.s, tmp->areap);
			afree(tmp, tmp->areap);
		}
		vp->u.array = NULL;
	}
	if (flags & 2) {
		vp->flag &= ~(ALLOC|ISSET);
		return;
	}
	/* if foo[0] is being unset, the remainder of the array is kept... */
	vp->flag &= SPECIAL | ((flags & 1) ? 0 : ARRAY|DEFINED);
	if (vp->flag & SPECIAL)
		/* responsible for 'unspecial'ing var */
		unsetspec(vp);
}

/*
 * Return a pointer to the first char past a legal variable name
 * (returns the argument if there is no legal name, returns a pointer to
 * the terminating NUL if whole string is legal).
 */
const char *
skip_varname(const char *s, bool aok)
{
	size_t alen;

	if (s && ksh_isalphx(*s)) {
		while (*++s && ksh_isalnux(*s))
			;
		if (aok && *s == '[' && (alen = array_ref_len(s)))
			s += alen;
	}
	return (s);
}

/* Return a pointer to the first character past any legal variable name */
const char *
skip_wdvarname(const char *s,
    /* skip array de-reference? */
    bool aok)
{
	if (s[0] == CHAR && ksh_isalphx(s[1])) {
		do {
			s += 2;
		} while (s[0] == CHAR && ksh_isalnux(s[1]));
		if (aok && s[0] == CHAR && s[1] == '[') {
			/* skip possible array de-reference */
			const char *p = s;
			char c;
			int depth = 0;

			while (/* CONSTCOND */ 1) {
				if (p[0] != CHAR)
					break;
				c = p[1];
				p += 2;
				if (c == '[')
					depth++;
				else if (c == ']' && --depth == 0) {
					s = p;
					break;
				}
			}
		}
	}
	return (s);
}

/* Check if coded string s is a variable name */
int
is_wdvarname(const char *s, bool aok)
{
	const char *p = skip_wdvarname(s, aok);

	return (p != s && p[0] == EOS);
}

/* Check if coded string s is a variable assignment */
int
is_wdvarassign(const char *s)
{
	const char *p = skip_wdvarname(s, true);

	return (p != s && p[0] == CHAR &&
	    (p[1] == '=' || (p[1] == '+' && p[2] == CHAR && p[3] == '=')));
}

/*
 * Make the exported environment from the exported names in the dictionary.
 */
char **
makenv(void)
{
	ssize_t i;
	struct block *l;
	XPtrV denv;
	struct tbl *vp, **vpp;

	XPinit(denv, 64);
	for (l = e->loc; l != NULL; l = l->next) {
		vpp = l->vars.tbls;
		i = 1 << (l->vars.tshift);
		while (--i >= 0)
			if ((vp = *vpp++) != NULL &&
			    (vp->flag&(ISSET|EXPORT)) == (ISSET|EXPORT)) {
				struct block *l2;
				struct tbl *vp2;
				uint32_t h = hash(vp->name);

				/* unexport any redefined instances */
				for (l2 = l->next; l2 != NULL; l2 = l2->next) {
					vp2 = ktsearch(&l2->vars, vp->name, h);
					if (vp2 != NULL)
						vp2->flag &= ~EXPORT;
				}
				if ((vp->flag&INTEGER)) {
					/* integer to string */
					char *val;
					val = str_val(vp);
					vp->flag &= ~(INTEGER|RDONLY|SPECIAL);
					/* setstr can't fail here */
					setstr(vp, val, KSH_RETURN_ERROR);
				}
				XPput(denv, vp->val.s);
			}
		if (l->flags & BF_STOPENV)
			break;
	}
	XPput(denv, NULL);
	return ((char **)XPclose(denv));
}

/*
 * handle special variables with side effects - PATH, SECONDS.
 */

/* Test if name is a special parameter */
static int
special(const char *name)
{
	struct tbl *tp;

	tp = ktsearch(&specials, name, hash(name));
	return (tp && (tp->flag & ISSET) ? tp->type : V_NONE);
}

/* Make a variable non-special */
static void
unspecial(const char *name)
{
	struct tbl *tp;

	tp = ktsearch(&specials, name, hash(name));
	if (tp)
		ktdelete(tp);
}

static time_t seconds;		/* time SECONDS last set */
static mksh_uari_t user_lineno;	/* what user set $LINENO to */

static void
getspec(struct tbl *vp)
{
	mksh_ari_u num;
	int st;
	struct timeval tv;

	switch ((st = special(vp->name))) {
	case V_COLUMNS:
	case V_LINES:
		/*
		 * Do NOT export COLUMNS/LINES. Many applications
		 * check COLUMNS/LINES before checking ws.ws_col/row,
		 * so if the app is started with C/L in the environ
		 * and the window is then resized, the app won't
		 * see the change cause the environ doesn't change.
		 */
		if (got_winch)
			change_winsz();
		break;
	}
	switch (st) {
	case V_BASHPID:
		num.u = (mksh_uari_t)procpid;
		break;
	case V_COLUMNS:
		num.i = x_cols;
		break;
	case V_HISTSIZE:
		num.i = histsize;
		break;
	case V_LINENO:
		num.u = (mksh_uari_t)current_lineno + user_lineno;
		break;
	case V_LINES:
		num.i = x_lins;
		break;
	case V_EPOCHREALTIME: {
		/* 10(%u) + 1(.) + 6 + NUL */
		char buf[18];

		vp->flag &= ~SPECIAL;
		mksh_TIME(tv);
		shf_snprintf(buf, sizeof(buf), "%u.%06u",
		    (unsigned)tv.tv_sec, (unsigned)tv.tv_usec);
		setstr(vp, buf, KSH_RETURN_ERROR | 0x4);
		vp->flag |= SPECIAL;
		return;
	}
	case V_OPTIND:
		num.i = user_opt.uoptind;
		break;
	case V_RANDOM:
		num.i = rndget();
		break;
	case V_SECONDS:
		/*
		 * On start up the value of SECONDS is used before
		 * it has been set - don't do anything in this case
		 * (see initcoms[] in main.c).
		 */
		if (vp->flag & ISSET) {
			mksh_TIME(tv);
			num.i = tv.tv_sec - seconds;
		} else
			return;
		break;
	default:
		/* do nothing, do not touch vp at all */
		return;
	}
	vp->flag &= ~SPECIAL;
	setint_n(vp, num.i, 0);
	vp->flag |= SPECIAL;
}

static void
setspec(struct tbl *vp)
{
	mksh_ari_u num;
	char *s;
	int st;

	switch ((st = special(vp->name))) {
#if HAVE_PERSISTENT_HISTORY
	case V_HISTFILE:
		sethistfile(str_val(vp));
		return;
#endif
	case V_IFS:
		setctypes(s = str_val(vp), C_IFS);
		ifs0 = *s;
		return;
	case V_PATH:
		afree(path, APERM);
		s = str_val(vp);
		strdupx(path, s, APERM);
		/* clear tracked aliases */
		flushcom(true);
		return;
	case V_TMPDIR:
		afree(tmpdir, APERM);
		tmpdir = NULL;
		/*
		 * Use tmpdir iff it is an absolute path, is writable
		 * and searchable and is a directory...
		 */
		{
			struct stat statb;

			s = str_val(vp);
			/* LINTED use of access */
			if (mksh_abspath(s) && access(s, W_OK|X_OK) == 0 &&
			    stat(s, &statb) == 0 && S_ISDIR(statb.st_mode))
				strdupx(tmpdir, s, APERM);
		}
		return;
	/* common sub-cases */
	case V_COLUMNS:
	case V_LINES:
		if (vp->flag & IMPORT) {
			/* do not touch */
			unspecial(vp->name);
			vp->flag &= ~SPECIAL;
			return;
		}
		/* FALLTHROUGH */
	case V_HISTSIZE:
	case V_LINENO:
	case V_OPTIND:
	case V_RANDOM:
	case V_SECONDS:
	case V_TMOUT:
		vp->flag &= ~SPECIAL;
		if (getint(vp, &num, false) == -1) {
			s = str_val(vp);
			if (st != V_RANDOM)
				errorf("%s: %s: %s", vp->name, "bad number", s);
			num.u = hash(s);
		}
		vp->flag |= SPECIAL;
		break;
	default:
		/* do nothing, do not touch vp at all */
		return;
	}

	/* process the singular parts of the common cases */

	switch (st) {
	case V_COLUMNS:
		if (num.i >= MIN_COLS)
			x_cols = num.i;
		break;
	case V_HISTSIZE:
		sethistsize(num.i);
		break;
	case V_LINENO:
		/* The -1 is because line numbering starts at 1. */
		user_lineno = num.u - (mksh_uari_t)current_lineno - 1;
		break;
	case V_LINES:
		if (num.i >= MIN_LINS)
			x_lins = num.i;
		break;
	case V_OPTIND:
		getopts_reset((int)num.i);
		break;
	case V_RANDOM:
		/*
		 * mksh R39d+ no longer has the traditional repeatability
		 * of $RANDOM sequences, but always retains state
		 */
		rndset((unsigned long)num.u);
		break;
	case V_SECONDS:
		{
			struct timeval tv;

			mksh_TIME(tv);
			seconds = tv.tv_sec - num.i;
		}
		break;
	case V_TMOUT:
		ksh_tmout = num.i >= 0 ? num.i : 0;
		break;
	}
}

static void
unsetspec(struct tbl *vp)
{
	/*
	 * AT&T ksh man page says OPTIND, OPTARG and _ lose special
	 * meaning, but OPTARG does not (still set by getopts) and _ is
	 * also still set in various places. Don't know what AT&T does
	 * for HISTSIZE, HISTFILE. Unsetting these in AT&T ksh does not
	 * loose the 'specialness': IFS, COLUMNS, PATH, TMPDIR
	 */

	switch (special(vp->name)) {
#if HAVE_PERSISTENT_HISTORY
	case V_HISTFILE:
		sethistfile(NULL);
		return;
#endif
	case V_IFS:
		setctypes(TC_IFSWS, C_IFS);
		ifs0 = ' ';
		break;
	case V_PATH:
		afree(path, APERM);
		strdupx(path, def_path, APERM);
		/* clear tracked aliases */
		flushcom(true);
		break;
	case V_TMPDIR:
		/* should not become unspecial */
		if (tmpdir) {
			afree(tmpdir, APERM);
			tmpdir = NULL;
		}
		break;
	case V_LINENO:
	case V_RANDOM:
	case V_SECONDS:
	case V_TMOUT:
		/* AT&T ksh leaves previous value in place */
		unspecial(vp->name);
		break;
	}
}

/*
 * Search for (and possibly create) a table entry starting with
 * vp, indexed by val.
 */
struct tbl *
arraysearch(struct tbl *vp, uint32_t val)
{
	struct tbl *prev, *curr, *news;
	size_t len;

	vp->flag = (vp->flag | (ARRAY | DEFINED)) & ~ASSOC;
	/* the table entry is always [0] */
	if (val == 0)
		return (vp);
	prev = vp;
	curr = vp->u.array;
	while (curr && curr->ua.index < val) {
		prev = curr;
		curr = curr->u.array;
	}
	if (curr && curr->ua.index == val) {
		if (curr->flag&ISSET)
			return (curr);
		news = curr;
	} else
		news = NULL;
	if (!news) {
		len = strlen(vp->name);
		checkoktoadd(len, 1 + offsetof(struct tbl, name[0]));
		news = alloc(offsetof(struct tbl, name[0]) + ++len, vp->areap);
		memcpy(news->name, vp->name, len);
	}
	news->flag = (vp->flag & ~(ALLOC|DEFINED|ISSET|SPECIAL)) | AINDEX;
	news->type = vp->type;
	news->areap = vp->areap;
	news->u2.field = vp->u2.field;
	news->ua.index = val;

	if (curr != news) {
		/* not reusing old array entry */
		prev->u.array = news;
		news->u.array = curr;
	}
	return (news);
}

/*
 * Return the length of an array reference (eg, [1+2]) - cp is assumed
 * to point to the open bracket. Returns 0 if there is no matching
 * closing bracket.
 *
 * XXX this should parse the actual arithmetic syntax
 */
size_t
array_ref_len(const char *cp)
{
	const char *s = cp;
	char c;
	int depth = 0;

	while ((c = *s++) && (c != ']' || --depth))
		if (c == '[')
			depth++;
	if (!c)
		return (0);
	return (s - cp);
}

/*
 * Make a copy of the base of an array name
 */
char *
arrayname(const char *str)
{
	const char *p;
	char *rv;

	if ((p = cstrchr(str, '[')) == 0)
		/* Shouldn't happen, but why worry? */
		strdupx(rv, str, ATEMP);
	else
		strndupx(rv, str, p - str, ATEMP);

	return (rv);
}

/* set (or overwrite, if reset) the array variable var to the values in vals */
mksh_uari_t
set_array(const char *var, bool reset, const char **vals)
{
	struct tbl *vp, *vq;
	mksh_uari_t i = 0, j = 0;
	const char *ccp = var;
	char *cp = NULL;
	size_t n;

	/* to get local array, use "local foo; set -A foo" */
	n = strlen(var);
	if (n > 0 && var[n - 1] == '+') {
		/* append mode */
		reset = false;
		strndupx(cp, var, n - 1, ATEMP);
		ccp = cp;
	}
	vp = global(ccp);

	/* Note: AT&T ksh allows set -A but not set +A of a read-only var */
	if ((vp->flag&RDONLY))
		errorfx(2, "read-only: %s", ccp);
	/* This code is quite non-optimal */
	if (reset) {
		/* trash existing values and attributes */
		unset(vp, 1);
		/* allocate-by-access the [0] element to keep in scope */
		arraysearch(vp, 0);
	}
	/*
	 * TODO: would be nice for assignment to completely succeed or
	 * completely fail. Only really effects integer arrays:
	 * evaluation of some of vals[] may fail...
	 */
	if (cp != NULL) {
		/* find out where to set when appending */
		for (vq = vp; vq; vq = vq->u.array) {
			if (!(vq->flag & ISSET))
				continue;
			if (arrayindex(vq) >= j)
				j = arrayindex(vq) + 1;
		}
		afree(cp, ATEMP);
	}
	while ((ccp = vals[i])) {
#if 0 /* temporarily taken out due to regression */
		if (*ccp == '[') {
			int level = 0;

			while (*ccp) {
				if (*ccp == ']' && --level == 0)
					break;
				if (*ccp == '[')
					++level;
				++ccp;
			}
			if (*ccp == ']' && level == 0 && ccp[1] == '=') {
				strndupx(cp, vals[i] + 1, ccp - (vals[i] + 1),
				    ATEMP);
				evaluate(substitute(cp, 0), (mksh_ari_t *)&j,
				    KSH_UNWIND_ERROR, true);
				afree(cp, ATEMP);
				ccp += 2;
			} else
				ccp = vals[i];
		}
#endif

		vq = arraysearch(vp, j);
		/* would be nice to deal with errors here... (see above) */
		setstr(vq, ccp, KSH_RETURN_ERROR);
		i++;
		j++;
	}

	return (i);
}

void
change_winsz(void)
{
	struct timeval tv;

	mksh_TIME(tv);
	BAFHUpdateMem_mem(qh_state, &tv, sizeof(tv));

#ifdef TIOCGWINSZ
	/* check if window size has changed */
	if (tty_init_fd() < 2) {
		struct winsize ws;

		if (ioctl(tty_fd, TIOCGWINSZ, &ws) >= 0) {
			if (ws.ws_col)
				x_cols = ws.ws_col;
			if (ws.ws_row)
				x_lins = ws.ws_row;
		}
	}
#endif

	/* bounds check for sane values, use defaults otherwise */
	if (x_cols < MIN_COLS)
		x_cols = 80;
	if (x_lins < MIN_LINS)
		x_lins = 24;

#ifdef SIGWINCH
	got_winch = 0;
#endif
}

uint32_t
hash(const void *s)
{
	register uint32_t h;

	BAFHInit(h);
	BAFHUpdateStr_reg(h, s);
	BAFHFinish_reg(h);
	return (h);
}

uint32_t
chvt_rndsetup(const void *bp, size_t sz)
{
	register uint32_t h;

	/* use LCG as seed but try to get them to deviate immediately */
	h = lcg_state;
	(void)rndget();
	BAFHFinish_reg(h);
	/* variation through pid, ppid, and the works */
	BAFHUpdateMem_reg(h, &rndsetupstate, sizeof(rndsetupstate));
	/* some variation, some possibly entropy, depending on OE */
	BAFHUpdateMem_reg(h, bp, sz);
	/* mix them all up */
	BAFHFinish_reg(h);

	return (h);
}

mksh_ari_t
rndget(void)
{
	/*
	 * this is the same Linear Congruential PRNG as Borland
	 * C/C++ allegedly uses in its built-in rand() function
	 */
	return (((lcg_state = 22695477 * lcg_state + 1) >> 16) & 0x7FFF);
}

void
rndset(unsigned long v)
{
	register uint32_t h;
#if defined(arc4random_pushb_fast) || defined(MKSH_A4PB)
	register uint32_t t;
#endif
	struct {
		struct timeval tv;
		void *sp;
		uint32_t qh;
		pid_t pp;
		short r;
	} z;

#ifdef DEBUG
	/* clear the allocated space, for valgrind */
	memset(&z, 0, sizeof(z));
#endif

	h = lcg_state;
	BAFHFinish_reg(h);
	BAFHUpdateMem_reg(h, &v, sizeof(v));

	mksh_TIME(z.tv);
	z.sp = &lcg_state;
	z.pp = procpid;
	z.r = (short)rndget();

#if defined(arc4random_pushb_fast) || defined(MKSH_A4PB)
	t = qh_state;
	BAFHFinish_reg(t);
	z.qh = (t & 0xFFFF8000) | rndget();
	lcg_state = (t << 15) | rndget();
	/*
	 * either we have very chap entropy get and push available,
	 * with malloc() pulling in this code already anyway, or the
	 * user requested us to use the old functions
	 */
	t = h;
	BAFHUpdateMem_reg(t, &lcg_state, sizeof(lcg_state));
	BAFHFinish_reg(t);
	lcg_state = t;
#if defined(arc4random_pushb_fast)
	arc4random_pushb_fast(&lcg_state, sizeof(lcg_state));
	lcg_state = arc4random();
#else
	lcg_state = arc4random_pushb(&lcg_state, sizeof(lcg_state));
#endif
	BAFHUpdateMem_reg(h, &lcg_state, sizeof(lcg_state));
#else
	z.qh = qh_state;
#endif

	BAFHUpdateMem_reg(h, &z, sizeof(z));
	BAFHFinish_reg(h);
	lcg_state = h;
}

void
rndpush(const void *s)
{
	register uint32_t h = qh_state;

	BAFHUpdateStr_reg(h, s);
	BAFHUpdateOctet_reg(h, 0);
	qh_state = h;
}

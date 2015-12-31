/*	$Id: cxmuldiv.c,v 1.3 2015/12/29 10:19:52 ragge Exp $	*/
/*
 * Copyright (c) 2014 Anders Magnusson (ragge@ludd.luth.se).
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Complex div and mul, close to the C99 example but without
 * math routines. May need pcc to compile correctly.
 * Uses the same calling conventions and naming as the gcc counterparts.
 *
 * Could probably use the same function for all three sizes of floats.
 * Not heavily tested.
 */

#if __FLOAT_WORD_ORDER__ != __ORDER_PDP_ENDIAN__ /* not supported yet... */

/*
 * Float (single precision).
 */
union uf {
	float f;
	unsigned int i;
};

#define	FLOAT_SIGN	0x80000000U
#define	FLOAT_EXP	0x7f800000U
#define	FLOAT_MANT	0x007fffffU
#define	FLOAT_MANTEXP	(FLOAT_EXP|FLOAT_MANT)
#define	FLOAT_INF	0x7f800000U

#define	FLOAT_ISFIN(x)	((x.i & FLOAT_MANTEXP) != FLOAT_INF)
#define	FLOAT_ISINF(x)	((x.i & FLOAT_MANTEXP) == FLOAT_INF)
#define	FLOAT_ISNAN(x)	(((x.i & FLOAT_EXP) == FLOAT_INF) && \
			    (x.i & FLOAT_MANT) != 0)
static float
pcc_copysignf(float x, float y)
{
	union uf ux, uy;

	ux.f = x;
	uy.f = y;

	ux.i = (ux.i & FLOAT_MANTEXP) | (uy.i & FLOAT_SIGN);
	return ux.f;
}

static float
pcc_scalbnf(float x, int exp)
{
	union uf x1, z1, z2;

	x1.f = x;
	if (FLOAT_ISINF(x1) || FLOAT_ISNAN(x1))
		return x;
	exp += 127;
	if (exp <= 0) {
		z2.i = (127 - 50) << 23;
		x *= z2.f;
		exp += 50;
		if (exp < 0) exp = 0;
	} else if (exp > 254) {
		z2.i = (128 + 25) << 23;
		x *= z2.f;
		exp -= 25;
		if (exp > 254) exp = 254;
	}
	z1.i = exp << 23;
	x *= z1.f;
	return x;
}

float _Complex
__divsc3(float ax, float bx, float cx, float dx)
{
	float denom;
	union uf ua, ub, uc, ud, ur, ulogbw, ux, uy;
	int uci, udi, res, ilogbw = 0;

	ua.f = ax;
	ub.f = bx;
	uc.f = cx;
	ud.f = dx;

	/* fabsf; clear sign */
	uci = uc.i & FLOAT_MANTEXP;
	udi = ud.i & FLOAT_MANTEXP;

	/* fmaxf; ordinary integer compare */
	ur.i = (uci > udi ? uci : udi);

	/* logbf */
	res = ur.i >> 23;
	if (ur.i == 0)
		ulogbw.f = (float)-1.0/ur.f;
	else if (res == 255)
		ulogbw.f = ur.f * ur.f;
	else if (res == 0)
		ulogbw.f = -126.0;
	else
		ulogbw.f = (res-127);

	/* isfinite */
	if (FLOAT_ISFIN(ulogbw)) {
		ilogbw = (int)ulogbw.f;
		uc.f = pcc_scalbnf(uc.f, -ilogbw);
		ud.f = pcc_scalbnf(ud.f, -ilogbw);
	}
	denom = uc.f * uc.f + ud.f * ud.f;
	ux.f = pcc_scalbnf((ua.f * uc.f + ub.f * ud.f) / denom, -ilogbw);
	uy.f = pcc_scalbnf((ub.f * uc.f - ua.f * ud.f) / denom, -ilogbw);

	if (FLOAT_ISNAN(ux) && FLOAT_ISNAN(uy)) {
		if ((denom == 0.0) &&
		    (!FLOAT_ISNAN(ua) || !FLOAT_ISNAN(ub))) {
			ux.f = pcc_copysignf(__builtin_inff(), uc.f) * ua.f;
			uy.f = pcc_copysignf(__builtin_inff(), uc.f) * ub.f;
		} else if ((FLOAT_ISINF(ua) || FLOAT_ISINF(ub)) &&
		    FLOAT_ISFIN(uc) && FLOAT_ISFIN(ud)) {
			ua.f = pcc_copysignf(FLOAT_ISINF(ua) ? 1.0 : 0.0, ua.f);
			ub.f = pcc_copysignf(FLOAT_ISINF(ub) ? 1.0 : 0.0, ub.f);
			ux.f = __builtin_inff() * ( ua.f * uc.f + ub.f * ud.f );
			uy.f = __builtin_inff() * ( ub.f * uc.f - ua.f * ud.f );
		} else if (FLOAT_ISINF(ulogbw) &&
		    FLOAT_ISFIN(ua) && FLOAT_ISFIN(ub)) {
			uc.f = pcc_copysignf(FLOAT_ISINF(uc) ? 1.0 : 0.0, uc.f);
			ud.f = pcc_copysignf(FLOAT_ISINF(ud) ? 1.0 : 0.0, ud.f);
			ux.f = 0.0 * ( ua.f * uc.f + ub.f * ud.f );
			uy.f = 0.0 * ( ub.f * uc.f - ua.f * ud.f );
		}
	}
	return ux.f + 1.0iF * uy.f;
}

float _Complex
__mulsc3(float fa, float fb, float fc, float fd)
{
	union uf ua, ub, uc, ud, ux, uy, uac, ubd, uad, ubc;

	ua.f = fa;
	ub.f = fb;
	uc.f = fc;
	ud.f = fd;

	uac.f = ua.f * uc.f;
	ubd.f = ub.f * ud.f;
	uad.f = ua.f * ud.f;
	ubc.f = ub.f * uc.f;

	ux.f=uac.f-ubd.f;
	uy.f=uad.f+ubc.f;


	if (FLOAT_ISNAN(ux) && FLOAT_ISNAN(uy)) {
		/* Recover infinities that computed as NaN+iNaN ... */
		int recalc = 0;
		if (FLOAT_ISINF(ua) || FLOAT_ISINF(ub) ) { // z is infinite
			/* "Box" the infinity and change NaNs
			   in the other factor to 0 */
			ua.f = pcc_copysignf(FLOAT_ISINF(ua) ? 1.0 : 0.0, ua.f);
			ub.f = pcc_copysignf(FLOAT_ISINF(ub) ? 1.0 : 0.0, ub.f);
			if (FLOAT_ISNAN(uc)) uc.f = pcc_copysignf(0.0, uc.f);
			if (FLOAT_ISNAN(ud)) ud.f = pcc_copysignf(0.0, ud.f);
			recalc = 1;
		}
		if (FLOAT_ISINF(uc) || FLOAT_ISINF(ud) ) { // w is infinite
			/* "Box" the infinity and change NaNs
			    in the other factor to 0 */
			uc.f = pcc_copysignf(FLOAT_ISINF(uc) ? 1.0 : 0.0, uc.f);
			ud.f = pcc_copysignf(FLOAT_ISINF(ud) ? 1.0 : 0.0, ud.f);
			if (FLOAT_ISNAN(ua)) ua.f = pcc_copysignf(0.0, ua.f);
			if (FLOAT_ISNAN(ub)) ub.f = pcc_copysignf(0.0, ub.f);
			recalc = 1;
		}
		if (!recalc && (FLOAT_ISINF(uac) || FLOAT_ISINF(ubd) ||
		    FLOAT_ISINF(uad) || FLOAT_ISINF(ubc))) {
			/* Recover infinities from overflow by
			   changing NaNs to 0 ... */
			if (FLOAT_ISNAN(ua)) ua.f = pcc_copysignf(0.0, ua.f);
			if (FLOAT_ISNAN(ub)) ub.f = pcc_copysignf(0.0, ub.f);
			if (FLOAT_ISNAN(uc)) uc.f = pcc_copysignf(0.0, uc.f);
			if (FLOAT_ISNAN(ud)) ud.f = pcc_copysignf(0.0, ud.f);
			recalc = 1;
		}
		if (recalc) {
			ux.f = __builtin_inff() * ( ua.f * uc.f - ub.f * ud.f );
			uy.f = __builtin_inff() * ( ua.f * ud.f + ub.f * uc.f );
		}
	}
	return ux.f + 1.0iF * uy.f;
}

/*
 * double precision functions.
 */
union ud {
	double f;
	unsigned int i[2];
};

#if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
#define	dih	i[0]
#define	dil	i[1]
#else
#define	dil	i[0]
#define	dih	i[1]
#endif


#define	DOUBLE_SIGN	0x80000000U
#define	DOUBLE_EXP	0x7ff00000U
#define	DOUBLE_MANT	0x000fffffU
#define	DOUBLE_MANTEXP	(DOUBLE_EXP|DOUBLE_MANT)
#define	DOUBLE_INF	0x7ff00000U

#define	DOUBLE_ISFIN(x)	((x.dih & DOUBLE_MANTEXP) != DOUBLE_INF)
#define	DOUBLE_ISINF(x)	((x.dih & DOUBLE_MANTEXP) == DOUBLE_INF)
#define	DOUBLE_ISNAN(x)	(((x.dih & DOUBLE_EXP) == DOUBLE_INF) && \
		    (((x.dih & DOUBLE_MANT) != 0) || ((x.dil) != 0)))

static double
pcc_copysign(double x, double y)
{
	union ud ux, uy;

	ux.f = x;
	uy.f = y;

	ux.dih = (ux.dih & DOUBLE_MANTEXP) | (uy.dih & DOUBLE_SIGN);
	return ux.f;
}

static double
pcc_scalbn(double x, int exp)
{
	union ud x1, z1, z2;

	x1.f = x;
	if (DOUBLE_ISINF(x1) || DOUBLE_ISNAN(x1))
		return x;
	exp += 1023;
	z1.dil = z2.dil = 0;
	if (exp <= 0) {
		z2.dih = (1023 - 110) << 20;
		x *= z2.f;
		exp += 110;
		if (exp < 0) exp = 0;
	} else if (exp > 2046) {
		z2.dih = (1024 + 55) << 20;
		x *= z2.f;
		exp -= 55;
		if (exp > 2046) exp = 2046;
	}
	z1.dih = exp << 20;
	x *= z1.f;
	return x;
}

double _Complex
__divdc3(double ax, double bx, double cx, double dx)
{
	double denom;
	union ud ua, ub, uc, ud, ur, ulogbw, ux, uy;
	int uci, udi, res, ilogbw = 0;

	ua.f = ax;
	ub.f = bx;
	uc.f = cx;
	ud.f = dx;

	/* fabsf; clear sign */
	uci = uc.dih & DOUBLE_MANTEXP;
	udi = ud.dih & DOUBLE_MANTEXP;

	/* fmaxf; ordinary integer compare */
	if ((uci > udi) || (uci == udi && uc.dil > ud.dil)) {
		ur.dih = uci;
		ur.dil = uc.dil;
	} else {
		ur.dih = udi;
		ur.dil = ud.dil;
	}

	/* logbf */
	res = ur.dih >> 20;
	if (ur.dih == 0 && ur.dil == 0)
		ulogbw.f = (double)-1.0/ur.f;
	else if (res == 2047)
		ulogbw.f = ur.f * ur.f;
	else if (res == 0)
		ulogbw.f = -1022.0;
	else
		ulogbw.f = (res-1023);

	/* isfinite */
	if (DOUBLE_ISFIN(ulogbw)) {
		ilogbw = (int)ulogbw.f;
		uc.f = pcc_scalbn(uc.f, -ilogbw);
		ud.f = pcc_scalbn(ud.f, -ilogbw);
	}
	denom = uc.f * uc.f + ud.f * ud.f;
	ux.f = pcc_scalbn((ua.f * uc.f + ub.f * ud.f) / denom, -ilogbw);
	uy.f = pcc_scalbn((ub.f * uc.f - ua.f * ud.f) / denom, -ilogbw);

	if (DOUBLE_ISNAN(ux) && DOUBLE_ISNAN(uy)) {
		if ((denom == 0.0) &&
		    (!DOUBLE_ISNAN(ua) || !DOUBLE_ISNAN(ub))) {
			ux.f = pcc_copysign(__builtin_inff(), uc.f) * ua.f;
			uy.f = pcc_copysign(__builtin_inff(), uc.f) * ub.f;
		} else if ((DOUBLE_ISINF(ua) || DOUBLE_ISINF(ub)) &&
		    DOUBLE_ISFIN(uc) && DOUBLE_ISFIN(ud)) {
			ua.f = pcc_copysign(DOUBLE_ISINF(ua) ? 1.0 : 0.0, ua.f);
			ub.f = pcc_copysign(DOUBLE_ISINF(ub) ? 1.0 : 0.0, ub.f);
			ux.f = __builtin_inf() * ( ua.f * uc.f + ub.f * ud.f );
			uy.f = __builtin_inf() * ( ub.f * uc.f - ua.f * ud.f );
		} else if (DOUBLE_ISINF(ulogbw) &&
		    DOUBLE_ISFIN(ua) && DOUBLE_ISFIN(ub)) {
			uc.f = pcc_copysign(DOUBLE_ISINF(uc) ? 1.0 : 0.0, uc.f);
			ud.f = pcc_copysign(DOUBLE_ISINF(ud) ? 1.0 : 0.0, ud.f);
			ux.f = 0.0 * ( ua.f * uc.f + ub.f * ud.f );
			uy.f = 0.0 * ( ub.f * uc.f - ua.f * ud.f );
		}
	}
	return ux.f + 1.0iF * uy.f;
}

double _Complex
__muldc3(double fa, double fb, double fc, double fd)
{
	union ud ua, ub, uc, ud, ux, uy, uac, ubd, uad, ubc;

	ua.f = fa;
	ub.f = fb;
	uc.f = fc;
	ud.f = fd;

	uac.f = ua.f * uc.f;
	ubd.f = ub.f * ud.f;
	uad.f = ua.f * ud.f;
	ubc.f = ub.f * uc.f;

	ux.f=uac.f-ubd.f;
	uy.f=uad.f+ubc.f;


	if (DOUBLE_ISNAN(ux) && DOUBLE_ISNAN(uy)) {
		/* Recover infinities that computed as NaN+iNaN ... */
		int recalc = 0;
		if (DOUBLE_ISINF(ua) || DOUBLE_ISINF(ub) ) { // z is infinite
			/* "Box" the infinity and change NaNs
			   in the other factor to 0 */
			ua.f = pcc_copysign(DOUBLE_ISINF(ua) ? 1.0 : 0.0, ua.f);
			ub.f = pcc_copysign(DOUBLE_ISINF(ub) ? 1.0 : 0.0, ub.f);
			if (DOUBLE_ISNAN(uc)) uc.f = pcc_copysign(0.0, uc.f);
			if (DOUBLE_ISNAN(ud)) ud.f = pcc_copysign(0.0, ud.f);
			recalc = 1;
		}
		if (DOUBLE_ISINF(uc) || DOUBLE_ISINF(ud) ) { // w is infinite
			/* "Box" the infinity and change NaNs
			    in the other factor to 0 */
			uc.f = pcc_copysign(DOUBLE_ISINF(uc) ? 1.0 : 0.0, uc.f);
			ud.f = pcc_copysign(DOUBLE_ISINF(ud) ? 1.0 : 0.0, ud.f);
			if (DOUBLE_ISNAN(ua)) ua.f = pcc_copysign(0.0, ua.f);
			if (DOUBLE_ISNAN(ub)) ub.f = pcc_copysign(0.0, ub.f);
			recalc = 1;
		}
		if (!recalc && (DOUBLE_ISINF(uac) || DOUBLE_ISINF(ubd) ||
		    DOUBLE_ISINF(uad) || DOUBLE_ISINF(ubc))) {
			/* Recover infinities from overflow by
			   changing NaNs to 0 ... */
			if (DOUBLE_ISNAN(ua)) ua.f = pcc_copysign(0.0, ua.f);
			if (DOUBLE_ISNAN(ub)) ub.f = pcc_copysign(0.0, ub.f);
			if (DOUBLE_ISNAN(uc)) uc.f = pcc_copysign(0.0, uc.f);
			if (DOUBLE_ISNAN(ud)) ud.f = pcc_copysign(0.0, ud.f);
			recalc = 1;
		}
		if (recalc) {
			ux.f = __builtin_inf() * ( ua.f * uc.f - ub.f * ud.f );
			uy.f = __builtin_inf() * ( ua.f * ud.f + ub.f * uc.f );
		}
	}
	return ux.f + 1.0iF * uy.f;
}

/*
 * Long double functions.
 */
union ul {
	long double f;
	unsigned int i[4];
};

#if defined(mach_amd64) || defined(mach_i386)
/* x87 */
#define	dil3	i[0]
#define	dil2	i[0]
#define	dil1	i[1]
#define	dlh	i[2]
#elif __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
#define	dlh	i[0]
#define	dil1	i[1]
#define	dil2	i[2]
#define	dil3	i[3]
#else
#define	dil3	i[0]
#define	dil2	i[1]
#define	dil1	i[2]
#define	dlh	i[3]
#endif

#if defined(mach_amd64) || defined(mach_i386) || defined(mach_mips)
#define	LDBL_SIGN	0x8000U
#define	LDBL_EXP	0x7fffU
#define	LDBL_MANT	0x0U
#define	LDBL_MANTEXP	(LDBL_EXP|LDBL_MANT)
#define	LDBL_INF	LDBL_EXP
#define	LDBL_HIDDEN	0x80000000U
#else
#error FIXME long double
#endif

#define	LDBL_ISFIN(x)	((x.dlh & LDBL_MANTEXP) != LDBL_INF)
#define	LDBL_ISINF(x)	(((x.dlh & LDBL_MANTEXP) == LDBL_INF) && \
		(x.dil1 == 0 && x.dil2 == 0 && x.dil3 == 0))
#define	LDBL_ISNAN(x)	(((x.dlh & LDBL_MANTEXP) == LDBL_INF) && \
		(x.dil1 != 0 || x.dil2 != 0 || x.dil3 != 0))

static long double
pcc_copysignl(long double x, long double y)
{
	union ul ux, uy;

	ux.f = x;
	uy.f = y;

	ux.dlh = (ux.dlh & LDBL_MANTEXP) | (uy.dlh & LDBL_SIGN);
	return ux.f;
}

static long double
pcc_scalbnl(long double x, int exp)
{
	union ul x1, z1, z2;

	x1.f = x;
	if (LDBL_ISINF(x1) || LDBL_ISNAN(x1))
		return x;
	exp += 16383;
	z1.dil1 = z1.dil2 = z2.dil1 = z2.dil2 = 0;
	z1.dil3 = z2.dil3 = 0;
	if (exp <= 0) {
		z2.dlh = (16383 - 130);
		z2.dil1 = LDBL_HIDDEN;
		x *= z2.f;
		exp += 130;
		if (exp < 0) exp = 0;
	} else if (exp > 32766) {
		z2.dlh = (16384 + 65);
		z2.dil1 = LDBL_HIDDEN;
		x *= z2.f;
		exp -= 65;
		if (exp > 32766) exp = 32766;
	}
	z1.dlh = exp;
	z1.dil1 = LDBL_HIDDEN;
	x *= z1.f;
	return x;
}

long double _Complex
__divxc3(long double ax, long double bx, long double cx, long double dx)
{
	long double denom;
	union ul ua, ub, uc, ud, ur, ulogbw, ux, uy, *urp;
	int uci, udi, res, ilogbw = 0;

	ua.f = ax;
	ub.f = bx;
	uc.f = cx;
	ud.f = dx;

	/* fabsf; clear sign */
	uci = uc.dlh & LDBL_MANTEXP;
	udi = ud.dlh & LDBL_MANTEXP;

	/* fmaxl; integer compare */
	if ((uci > udi) ||
	    ((uci == udi) && (uc.dil1 > ud.dil1)) ||
	    ((uci == udi) && (uc.dil1 == ud.dil1) && (uc.dil2 > ud.dil2)) ||
	    ((uci == udi) && (uc.dil1 == ud.dil1) && (uc.dil2 == ud.dil2) &&
	    (uc.dil3 > ud.dil3)))
		urp = &uc;
	else
		urp = &ud;

	ur.dlh = urp->dlh;
	ur.dil1 = urp->dil1;
	ur.dil2 = urp->dil2;
	ur.dil3 = urp->dil3;

	/* logbf */
	res = ur.dlh;
	if (res == 0 && ur.dil1 == 0 && ur.dil2 == 0 && ur.dil3 == 0)
		ulogbw.f = (long double)-1.0/ur.f;
	else if (res == 32767)
		ulogbw.f = ur.f * ur.f;
	else if (res == 0)
		ulogbw.f = -16382.0;
	else
		ulogbw.f = (res-16383);

	/* isfinite */
	if (LDBL_ISFIN(ulogbw)) {
		ilogbw = (int)ulogbw.f;
		uc.f = pcc_scalbnl(uc.f, -ilogbw);
		ud.f = pcc_scalbnl(ud.f, -ilogbw);
	}
	denom = uc.f * uc.f + ud.f * ud.f;
	ux.f = pcc_scalbnl((ua.f * uc.f + ub.f * ud.f) / denom, -ilogbw);
	uy.f = pcc_scalbnl((ub.f * uc.f - ua.f * ud.f) / denom, -ilogbw);

	if (LDBL_ISNAN(ux) && LDBL_ISNAN(uy)) {
		if ((denom == 0.0) &&
		    (!LDBL_ISNAN(ua) || !LDBL_ISNAN(ub))) {
			ux.f = pcc_copysignl(__builtin_infl(), uc.f) * ua.f;
			uy.f = pcc_copysignl(__builtin_infl(), uc.f) * ub.f;
		} else if ((LDBL_ISINF(ua) || LDBL_ISINF(ub)) &&
		    LDBL_ISFIN(uc) && LDBL_ISFIN(ud)) {
			ua.f = pcc_copysignl(LDBL_ISINF(ua) ? 1.0 : 0.0, ua.f);
			ub.f = pcc_copysignl(LDBL_ISINF(ub) ? 1.0 : 0.0, ub.f);
			ux.f = __builtin_infl() * ( ua.f * uc.f + ub.f * ud.f );
			uy.f = __builtin_infl() * ( ub.f * uc.f - ua.f * ud.f );
		} else if (LDBL_ISINF(ulogbw) &&
		    LDBL_ISFIN(ua) && LDBL_ISFIN(ub)) {
			uc.f = pcc_copysignl(LDBL_ISINF(uc) ? 1.0 : 0.0, uc.f);
			ud.f = pcc_copysignl(LDBL_ISINF(ud) ? 1.0 : 0.0, ud.f);
			ux.f = 0.0 * ( ua.f * uc.f + ub.f * ud.f );
			uy.f = 0.0 * ( ub.f * uc.f - ua.f * ud.f );
		}
	}
	return ux.f + 1.0iF * uy.f;
}

long double _Complex
__mulxc3(long double fa, long double fb, long double fc, long double fd)
{
	union ul ua, ub, uc, ud, ux, uy, uac, ubd, uad, ubc;

	ua.f = fa;
	ub.f = fb;
	uc.f = fc;
	ud.f = fd;

	uac.f = ua.f * uc.f;
	ubd.f = ub.f * ud.f;
	uad.f = ua.f * ud.f;
	ubc.f = ub.f * uc.f;

	ux.f=uac.f-ubd.f;
	uy.f=uad.f+ubc.f;


	if (LDBL_ISNAN(ux) && LDBL_ISNAN(uy)) {
		/* Recover infinities that computed as NaN+iNaN ... */
		int recalc = 0;
		if (LDBL_ISINF(ua) || LDBL_ISINF(ub) ) { // z is infinite
			/* "Box" the infinity and change NaNs
			   in the other factor to 0 */
			ua.f = pcc_copysignl(LDBL_ISINF(ua) ? 1.0 : 0.0, ua.f);
			ub.f = pcc_copysignl(LDBL_ISINF(ub) ? 1.0 : 0.0, ub.f);
			if (LDBL_ISNAN(uc)) uc.f = pcc_copysignl(0.0, uc.f);
			if (LDBL_ISNAN(ud)) ud.f = pcc_copysignl(0.0, ud.f);
			recalc = 1;
		}
		if (LDBL_ISINF(uc) || LDBL_ISINF(ud) ) { // w is infinite
			/* "Box" the infinity and change NaNs
			    in the other factor to 0 */
			uc.f = pcc_copysignl(LDBL_ISINF(uc) ? 1.0 : 0.0, uc.f);
			ud.f = pcc_copysignl(LDBL_ISINF(ud) ? 1.0 : 0.0, ud.f);
			if (LDBL_ISNAN(ua)) ua.f = pcc_copysignl(0.0, ua.f);
			if (LDBL_ISNAN(ub)) ub.f = pcc_copysignl(0.0, ub.f);
			recalc = 1;
		}
		if (!recalc && (LDBL_ISINF(uac) || LDBL_ISINF(ubd) ||
		    LDBL_ISINF(uad) || LDBL_ISINF(ubc))) {
			/* Recover infinities from overflow by
			   changing NaNs to 0 ... */
			if (LDBL_ISNAN(ua)) ua.f = pcc_copysignl(0.0, ua.f);
			if (LDBL_ISNAN(ub)) ub.f = pcc_copysignl(0.0, ub.f);
			if (LDBL_ISNAN(uc)) uc.f = pcc_copysignl(0.0, uc.f);
			if (LDBL_ISNAN(ud)) ud.f = pcc_copysignl(0.0, ud.f);
			recalc = 1;
		}
		if (recalc) {
			ux.f = __builtin_infl() * ( ua.f * uc.f - ub.f * ud.f );
			uy.f = __builtin_infl() * ( ua.f * ud.f + ub.f * uc.f );
		}
	}
	return ux.f + 1.0iF * uy.f;
}

#endif

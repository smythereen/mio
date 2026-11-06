/* SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later */
/* Copyright (c) 2026 John R. Smythe Jr. */
/*******************************************************************************
 * 
 *	Shared Memory Number Conversions: every common.h bit-size type
 *
 *		Integers  u8..u128, s8..s128 (also u_t, s_t): own 128 bit
 *			  decimal/hex conversion on two u64 words, so it
 *			  needs no printf or compiler support for 128 bits.
 *		Floats	  f32, f64: libc %g / strtof / strtod (exact in glibc).
 *			  f80, f128: own exact decimal <-> binary
 *				    conversion (bignum, round half even,
 *				    subnormals, inf, nan, -0): no strtold /
 *				    %Lg (exact only where the compiler's long
 *				    double is, not c-saw) and no libquadmath.
 *			  Output is the SHORTEST decimal (at most 9, 17,
 *			  21, 36 digits) that reads back to the same bits,
 *			  so every value round-trips.
 ******************************************************************************/
#include	<mem_io.h>
#include	<inttypes.h>

/*
 *	128 Bit Integers As Two u64 Words (lo, hi)
 */

typedef	struct	{
	u64	lo;
	u64	hi;
}	w128_t;

static	w128_t	w_load( void * data, int size, int sgn )
{
	w128_t	w;
	u8 *	p = (u8 *)data;
	int	n;

	w.lo = w.hi = 0;
	for( n = size - 1; n >= 0; n-- ) {	/* little endian */
		if( n >= 8 ) w.hi = ( w.hi << 8 ) | p[ n ];
		else w.lo = ( w.lo << 8 ) | p[ n ];
	}
	if( sgn && size < 16 && ( p[ size - 1 ] & 0x80 ) ) {	/* sign extend */
		if( size < 8 ) w.lo |= ~(u64)0 << ( size * 8 );
		w.hi = ~(u64)0;
	}
	return( w );
}

static	void	w_store( void * data, int size, w128_t w )
{
	u8 *	p = (u8 *)data;
	int	n;

	for( n = 0; n < size; n++ ) {
		p[ n ] = (u8)( n >= 8 ? w.hi >> ( ( n - 8 ) * 8 ) : w.lo >> ( n * 8 ) );
	}
}

static	w128_t	w_neg( w128_t w )
{
	w.lo = ~w.lo + 1;
	w.hi = ~w.hi + ( w.lo == 0 );
	return( w );
}

/* w = w / d, returns the remainder (d < 2^32) */

static	u32	w_div( w128_t * w, u32 d )
{
	u32	part[ 4 ];
	u64	r = 0;
	int	n;

	part[ 0 ] = (u32)( w->hi >> 32 );
	part[ 1 ] = (u32)w->hi;
	part[ 2 ] = (u32)( w->lo >> 32 );
	part[ 3 ] = (u32)w->lo;
	for( n = 0; n < 4; n++ ) {
		r = ( r << 32 ) | part[ n ];
		part[ n ] = (u32)( r / d );
		r %= d;
	}
	w->hi = ( (u64)part[ 0 ] << 32 ) | part[ 1 ];
	w->lo = ( (u64)part[ 2 ] << 32 ) | part[ 3 ];
	return( (u32)r );
}

/* w = w * m + a; returns non zero on overflow past 128 bits */

static	int	w_mul_add( w128_t * w, u32 m, u32 a )
{
	u64	x;
	u64	c;
	u32	part[ 4 ];
	int	n;

	part[ 0 ] = (u32)w->lo;
	part[ 1 ] = (u32)( w->lo >> 32 );
	part[ 2 ] = (u32)w->hi;
	part[ 3 ] = (u32)( w->hi >> 32 );
	c = a;
	for( n = 0; n < 4; n++ ) {
		x = (u64)part[ n ] * m + c;
		part[ n ] = (u32)x;
		c = x >> 32;
	}
	w->lo = ( (u64)part[ 1 ] << 32 ) | part[ 0 ];
	w->hi = ( (u64)part[ 3 ] << 32 ) | part[ 2 ];
	return( c != 0 );
}

/*
 *	Mio_Int_Out: integer of size bytes (1, 2, 4, 8, 16) to text
 *		base 10: sgn != 0 prints a sign always ("+5", "-5"); base 16:
 *		"0x" and 2 upper case digits per byte.
 */

void	Mio_Int_Out( char * buf, void * data, int size, int sgn, int base )
{
	w128_t	w = w_load( data, size, sgn );
	char	tmp[ 64 ];
	char *	t = &tmp[ sizeof( tmp ) - 1 ];
	char *	b = buf;
	int	neg = 0;
	int	n;

	*t = 0;
	if( base == 16 ) {
		for( n = 0; n < size * 2; n++ ) {
			*--t = "0123456789ABCDEF"[ w_div( &w, 16 ) ];
		}
		sprintf( buf, "0x%s", t );
		return;
	}
	if( sgn && ( w.hi >> 63 ) ) {
		neg = 1;
		w = w_neg( w );
	}
	do {
		*--t = (char)( '0' + w_div( &w, 10 ) );
	} while( w.lo != 0 || w.hi != 0 );
	if( sgn ) *b++ = neg ? '-' : '+';
	strcpy( b, t );
}

/*
 *	Mio_Int_In: text to integer of size bytes. Leading blanks, a sign,
 *		then digits ("0x" allowed in base 16). Out of range values are
 *		limited to the type's minimum or maximum. Returns the number of
 *		characters used (0: no number).
 */

int	Mio_Int_In( void * data, int size, int sgn, int base, char * s )
{
	char *	p = s;
	w128_t	w;
	w128_t	max;
	int	neg = 0;
	int	over = 0;
	int	any = 0;
	int	d;

	w.lo = w.hi = 0;
	while( isspace( (u8)*p ) ) p++;
	if( *p == '+' || *p == '-' ) neg = ( *p++ == '-' );
	if( base == 16 && p[ 0 ] == '0' && ( p[ 1 ] == 'x' || p[ 1 ] == 'X' ) && isxdigit( (u8)p[ 2 ] ) ) p += 2;
	for( ; ; p++, any = 1 ) {
		if( isdigit( (u8)*p ) ) d = *p - '0';
		else if( base == 16 && isxdigit( (u8)*p ) ) d = ( *p | 0x20 ) - 'a' + 10;
		else break;
		over |= w_mul_add( &w, base, d );
	}
	if( !any ) {				/* no digits: 0, like strtoull */
		w_store( data, size, w );
		return( 0 );
	}
	/* limits: unsigned max = 2^bits - 1, signed max = 2^(bits-1) - 1 */
	max.lo = size >= 8 ? ~(u64)0 : ( (u64)1 << ( size * 8 ) ) - 1;
	max.hi = size == 16 ? ~(u64)0 : 0;
	if( sgn ) {
		if( size == 16 ) max.hi >>= 1;
		else max.lo >>= 1;
	}
	if( sgn && neg ) {			/* signed minimum is -( max + 1 ) */
		max.lo += 1;
		if( max.lo == 0 ) max.hi += 1;
	}
	if( over || w.hi > max.hi || ( w.hi == max.hi && w.lo > max.lo ) ) {
		w = max;			/* limit, like strtoull/strtoll */
		if( !sgn ) neg = 0;
	}
	if( neg ) w = w_neg( w );		/* unsigned: wraps, like strtoull */
	w_store( data, size, w );
	return( (int)( p - s ) );
}

/*
 *	Big Unsigned Integers (for exact f128 conversion)
 */

#define	BN_WORDS	1300		/* > 38500 bits: 2^113 * 5^16494 */

typedef	struct	{
	int	n;			/* words used */
	u32	w[ BN_WORDS ];		/* little endian */
}	bn_t;

static	void	bn_set( bn_t * a, u32 v )
{
	a->n = v != 0;
	a->w[ 0 ] = v;
}

static	void	bn_trim( bn_t * a )
{
	while( a->n > 0 && a->w[ a->n - 1 ] == 0 ) a->n--;
}

static	int	bn_bits( bn_t * a )
{
	u32	t;
	int	b;

	if( a->n == 0 ) return( 0 );
	t = a->w[ a->n - 1 ];
	for( b = 0; t != 0; b++ ) t >>= 1;
	return( ( a->n - 1 ) * 32 + b );
}

static	void	bn_mul_add( bn_t * a, u32 m, u32 c0 )
{
	u64	c = c0;
	u64	x;
	int	n;

	for( n = 0; n < a->n; n++ ) {
		x = (u64)a->w[ n ] * m + c;
		a->w[ n ] = (u32)x;
		c = x >> 32;
	}
	if( c != 0 ) a->w[ a->n++ ] = (u32)c;
}

static	u32	bn_div_small( bn_t * a, u32 d )
{
	u64	r = 0;
	int	n;

	for( n = a->n - 1; n >= 0; n-- ) {
		r = ( r << 32 ) | a->w[ n ];
		a->w[ n ] = (u32)( r / d );
		r %= d;
	}
	bn_trim( a );
	return( (u32)r );
}

static	void	bn_shl( bn_t * a, int k )
{
	int	ws = k / 32;
	int	bs = k % 32;
	int	n;

	if( a->n == 0 || k == 0 ) return;
	a->w[ a->n ] = 0;
	for( n = a->n; n >= 0; n-- ) {
		u32	v = a->w[ n ] << bs;

		if( bs && n > 0 ) v |= a->w[ n - 1 ] >> ( 32 - bs );
		a->w[ n + ws ] = v;
	}
	for( n = 0; n < ws; n++ ) a->w[ n ] = 0;
	a->n += ws + 1;
	bn_trim( a );
}

static	void	bn_shr1( bn_t * a )
{
	int	n;

	for( n = 0; n < a->n; n++ ) {
		a->w[ n ] = ( a->w[ n ] >> 1 ) | ( n + 1 < a->n ? a->w[ n + 1 ] << 31 : 0 );
	}
	bn_trim( a );
}

static	int	bn_cmp( bn_t * a, bn_t * b )
{
	int	n;

	if( a->n != b->n ) return( a->n < b->n ? -1 : 1 );
	for( n = a->n - 1; n >= 0; n-- ) {
		if( a->w[ n ] != b->w[ n ] ) return( a->w[ n ] < b->w[ n ] ? -1 : 1 );
	}
	return( 0 );
}

static	void	bn_sub( bn_t * a, bn_t * b )	/* a -= b, a >= b */
{
	s64	c = 0;
	s64	x;
	int	n;

	for( n = 0; n < a->n; n++ ) {
		x = (s64)a->w[ n ] - ( n < b->n ? b->w[ n ] : 0 ) + c;
		a->w[ n ] = (u32)x;
		c = x < 0 ? -1 : 0;
	}
	bn_trim( a );
}

static	void	bn_pow( bn_t * a, u32 base, int e )	/* a *= base^e */
{
	u32	big = base == 10 ? 1000000000 : 1220703125;	/* 10^9, 5^13 */
	int	step = base == 10 ? 9 : 13;

	for( ; e >= step; e -= step ) bn_mul_add( a, big, 0 );
	for( ; e > 0; e-- ) bn_mul_add( a, base, 0 );
}

/*
 *	IEEE Binary Format Description
 */

typedef	struct	{
	int	p;		/* mantissa bits, implicit bit included */
	int	ebits;		/* exponent bits */
	int	emin;		/* minimum normal exponent */
	int	emax;
	int	explicit;	/* integer bit stored (x87 f80) */
}	fmt_t;

static	fmt_t	fmt_f80  = {  64, 15, -16382, 16383, 1 };
static	fmt_t	fmt_f128 = { 113, 15, -16382, 16383, 0 };

#define	MANT_FIELD(f)	( (f)->explicit ? (f)->p : (f)->p - 1 )	/* stored mantissa bits */
#define	FMT_BYTES(f)	( ( MANT_FIELD( f ) + (f)->ebits + 1 ) / 8 )

/* q (p + 3 bits at most) into the 4 words of m */

static	void	bn_to_words( bn_t * a, u32 m[ 5 ] )
{
	int	n;

	for( n = 0; n < 5; n++ ) m[ n ] = n < a->n ? a->w[ n ] : 0;
}

/*
 *	Decimal Text To Binary (correctly rounded, half even)
 *		Result: kind 0 zero, 1 finite, 2 inf, 3 nan; finite value is
 *		*mant * 2^*lsb with *mant < 2^p (5 words). Returns the
 *		number of characters used (0: no number).
 */

static	bn_t	bn_num;
static	bn_t	bn_den;

static	int	dec_to_bin( char * s, fmt_t * f, int * neg, int * kind, u32 mant[ 5 ], int * lsb )
{
	char *	p = s;
	int	any = 0;
	int	e10 = 0;
	int	ex = 0;
	int	exneg = 0;
	int	k;
	int	b;
	int	x;
	int	t;
	int	d;
	int	round;
	int	half;
	int	sticky;
	u32	q[ 5 ];
	bn_t *	num = &bn_num;
	bn_t *	den = &bn_den;

	*neg = 0;
	*kind = 0;
	while( isspace( (u8)*p ) ) p++;
	if( *p == '+' || *p == '-' ) *neg = ( *p++ == '-' );
	if( !strncasecmp( p, "inf", 3 ) ) {
		*kind = 2;
		p += strncasecmp( p, "infinity", 8 ) ? 3 : 8;
		return( (int)( p - s ) );
	}
	if( !strncasecmp( p, "nan", 3 ) ) {
		*kind = 3;
		p += 3;
		if( *p == '(' ) while( *p && *p++ != ')' );
		return( (int)( p - s ) );
	}
	bn_set( num, 0 );
	for( ; isdigit( (u8)*p ); p++, any = 1 ) bn_mul_add( num, 10, *p - '0' );
	if( *p == '.' ) {
		for( p++; isdigit( (u8)*p ); p++, any = 1, e10-- ) bn_mul_add( num, 10, *p - '0' );
	}
	if( !any ) return( 0 );
	if( ( *p == 'e' || *p == 'E' ) && ( isdigit( (u8)p[ 1 ] ) || ( ( p[ 1 ] == '+' || p[ 1 ] == '-' ) && isdigit( (u8)p[ 2 ] ) ) ) ) {
		p++;
		if( *p == '+' || *p == '-' ) exneg = ( *p++ == '-' );
		for( ; isdigit( (u8)*p ); p++ ) if( ex < 100000 ) ex = ex * 10 + ( *p - '0' );
		e10 += exneg ? -ex : ex;
	}
	bn_trim( num );
	if( num->n == 0 ) return( (int)( p - s ) );		/* zero */
	/* far out of range: decimal magnitude is about digits + e10 */
	d = (int)( bn_bits( num ) * 0.30103 ) + 1 + e10;
	if( d > f->emax * 0.30103 + 2 ) {
		*kind = 2;
		return( (int)( p - s ) );
	}
	if( d < ( f->emin - f->p ) * 0.30103 - 2 ) return( (int)( p - s ) );	/* zero */
	bn_set( den, 1 );
	if( e10 >= 0 ) bn_pow( num, 10, e10 );
	else bn_pow( den, 10, -e10 );
	/* q = num * 2^k / den with p + 2 bits: p, guard, round; sticky = remainder */
	k = f->p + 1 - ( bn_bits( num ) - bn_bits( den ) );
	for( ; ; ) {
		static	bn_t	a;
		static	bn_t	c;
		bn_t	qb;

		a = *num;
		c = *den;
		if( k >= 0 ) bn_shl( &a, k );
		else bn_shl( &c, -k );
		bn_set( &qb, 0 );
		bn_shl( &c, f->p + 3 );
		qb.n = 5;
		memset( qb.w, 0, 5 * sizeof( u32 ) );
		for( b = f->p + 3; b >= 0; b-- ) {
			if( bn_cmp( &a, &c ) >= 0 ) {
				bn_sub( &a, &c );
				qb.w[ b / 32 ] |= (u32)1 << ( b % 32 );
			}
			bn_shr1( &c );
		}
		bn_trim( &qb );
		x = bn_bits( &qb );
		if( x > f->p + 2 ) { k--; continue; }
		if( x < f->p + 2 ) { k++; continue; }
		sticky = a.n != 0;
		bn_to_words( &qb, q );
		break;
	}
	/* value = q * 2^-k, q has p + 2 bits: msb exponent X */
	x = f->p + 1 - k;
	t = x >= f->emin ? f->p : x - f->emin + f->p;	/* bits kept */
	if( t < 0 ) return( (int)( p - s ) );		/* below half the smallest subnormal */
	d = f->p + 2 - t;				/* bits dropped, 2 .. p + 2 */
	half = 0;
	round = 0;
	for( b = 0; b < d; b++ ) {			/* dropped bits: half = top bit, rest sticky */
		u32	bit = ( q[ b / 32 ] >> ( b % 32 ) ) & 1;

		if( b == d - 1 ) half = bit;
		else sticky |= bit;
	}
	for( b = 0; b < 5; b++ ) {			/* q >>= d */
		int	from = b * 32 + d;

		mant[ b ] = from / 32 < 5 ? q[ from / 32 ] >> ( from % 32 ) : 0;
		if( from % 32 && from / 32 + 1 < 5 ) mant[ b ] |= q[ from / 32 + 1 ] << ( 32 - from % 32 );
	}
	round = half && ( sticky || ( mant[ 0 ] & 1 ) );
	for( b = 0; round && b < 5; b++ ) round = ++mant[ b ] == 0;
	*lsb = x - t + 1;
	*kind = 1;
	return( (int)( p - s ) );
}

/* pack sign, mantissa * 2^lsb into IEEE bytes (little endian) */

static	void	bin_pack( void * data, fmt_t * f, int neg, int kind, u32 mant[ 5 ], int lsb )
{
	int	bytes = FMT_BYTES( f );
	int	fb = f->p - 1;				/* fraction bits */
	int	mw = MANT_FIELD( f );			/* exponent starts here */
	int	bias = f->emax;
	int	nb = 0;
	int	e = 0;
	int	b;
	u32	m[ 5 ];
	u8 *	o = (u8 *)data;

	memcpy( m, mant, sizeof( m ) );
	if( kind == 1 ) {
		for( b = 5 * 32 - 1; b >= 0; b-- ) if( ( m[ b / 32 ] >> ( b % 32 ) ) & 1 ) { nb = b + 1; break; }
		if( nb == 0 ) kind = 0;
	}
	if( kind == 1 ) {
		e = lsb + nb - 1;
		if( e > f->emax ) kind = 2;
		else if( nb > f->p ) {			/* rounding carried to 2^p: exact shift */
			for( b = 0; b < 5; b++ ) m[ b ] = ( m[ b ] >> 1 ) | ( b < 4 ? m[ b + 1 ] << 31 : 0 );
		}
	}
	memset( o, 0, bytes );
	if( kind == 1 && e >= f->emin ) {		/* normal: implicit bit dropped */
		if( !f->explicit ) m[ fb / 32 ] &= ~( (u32)1 << ( fb % 32 ) );
		for( b = 0; b < mw; b++ ) if( ( m[ b / 32 ] >> ( b % 32 ) ) & 1 ) o[ b / 8 ] |= 1 << ( b % 8 );
		e += bias;
	} else if( kind == 1 ) {			/* subnormal: lsb is emin - (p - 1) */
		for( b = 0; b < mw; b++ ) if( ( m[ b / 32 ] >> ( b % 32 ) ) & 1 ) o[ b / 8 ] |= 1 << ( b % 8 );
		e = 0;
	} else if( kind == 2 || kind == 3 ) {
		e = 2 * bias + 1;
		if( f->explicit ) o[ fb / 8 ] |= 1 << ( fb % 8 );		/* x87 integer bit */
		if( kind == 3 ) o[ ( fb - 1 ) / 8 ] |= 1 << ( ( fb - 1 ) % 8 );	/* quiet NaN */
	}
	for( b = 0; b < f->ebits; b++ ) if( ( e >> b ) & 1 ) o[ ( mw + b ) / 8 ] |= 1 << ( ( mw + b ) % 8 );
	if( neg ) o[ bytes - 1 ] |= 0x80;
}

/*
 *	Binary To Shortest Round-Trip Decimal (f128)
 */

static	char	dec_digits[ 12000 ];

static	void	g_format( char * buf, int neg, char * dig, int n, int x10 )
{
	char *	b = buf;
	int	k;

	while( n > 1 && dig[ n - 1 ] == '0' ) n--;	/* %g drops trailing zeros */
	if( neg ) *b++ = '-';
	if( x10 < -4 || x10 >= ( n > 6 ? n : 6 ) ) {	/* like %.{max(n,6)}g */
		*b++ = dig[ 0 ];
		if( n > 1 ) {
			*b++ = '.';
			for( k = 1; k < n; k++ ) *b++ = dig[ k ];
		}
		sprintf( b, "e%c%02d", x10 < 0 ? '-' : '+', x10 < 0 ? -x10 : x10 );
		return;
	}
	if( x10 < 0 ) {
		*b++ = '0';
		*b++ = '.';
		for( k = -1; k > x10; k-- ) *b++ = '0';
		for( k = 0; k < n; k++ ) *b++ = dig[ k ];
	} else {
		for( k = 0; k <= x10; k++ ) *b++ = k < n ? dig[ k ] : '0';
		if( n > x10 + 1 ) {
			*b++ = '.';
			for( ; k < n; k++ ) *b++ = dig[ k ];
		}
	}
	*b = 0;
}

/* dig = t (nd exact digits) correctly rounded (half even) to n digits; returns the decimal exponent */

static	int	dec_round( char * dig, char * t, int nd, int n, int x10 )
{
	int	b;
	int	k;
	int	up;
	int	zero;

	if( n >= nd ) {
		memcpy( dig, t, nd );
		for( k = nd; k < n; k++ ) dig[ k ] = '0';
		return( x10 );
	}
	memcpy( dig, t, n );
	zero = 1;
	for( b = n + 1; b < nd; b++ ) if( t[ b ] != '0' ) { zero = 0; break; }
	up = t[ n ] > '5' || ( t[ n ] == '5' && ( !zero || ( t[ n - 1 ] - '0' ) % 2 ) );
	for( b = n - 1; up && b >= 0; b-- ) {
		if( dig[ b ] == '9' ) dig[ b ] = '0';
		else { dig[ b ]++; up = 0; }
	}
	if( up ) {				/* 999 -> 1000 */
		dig[ 0 ] = '1';
		return( x10 + 1 );
	}
	return( x10 );
}

static	void	bin_to_dec( char * buf, void * data, fmt_t * f )
{
	u8 *	o = (u8 *)data;
	int	bytes = FMT_BYTES( f );
	int	fb = f->p - 1;
	int	mw = MANT_FIELD( f );
	int	neg = o[ bytes - 1 ] >> 7;
	int	lo;
	int	hi;
	int	e = 0;
	int	b;
	int	lsb;
	int	nd;
	int	n;
	int	x10;
	int	k;
	char	cand[ 64 ];
	char	dig[ 64 ];
	u8	back[ 16 ];
	int	bneg;
	int	bkind;
	int	blsb;
	u32	bm[ 5 ];
	bn_t *	a = &bn_num;
	char *	t;

	for( b = 0; b < f->ebits; b++ ) e |= ( ( o[ ( mw + b ) / 8 ] >> ( ( mw + b ) % 8 ) ) & 1 ) << b;
	bn_set( a, 0 );
	a->n = ( fb + 32 ) / 32;
	memset( a->w, 0, a->n * sizeof( u32 ) );
	for( b = 0; b < fb; b++ ) if( ( o[ b / 8 ] >> ( b % 8 ) ) & 1 ) a->w[ b / 32 ] |= (u32)1 << ( b % 32 );
	bn_trim( a );
	if( e == 2 * f->emax + 1 ) {
		strcpy( buf, a->n ? ( neg ? "-nan" : "nan" ) : ( neg ? "-inf" : "inf" ) );
		return;
	}
	if( e == 0 && a->n == 0 ) {
		strcpy( buf, neg ? "-0" : "0" );
		return;
	}
	if( f->explicit && e != 2 * f->emax + 1 ) {	/* x87: integer bit as stored */
		if( ( o[ fb / 8 ] >> ( fb % 8 ) ) & 1 ) {
			if( a->n <= fb / 32 ) a->n = fb / 32 + 1;
			a->w[ fb / 32 ] |= (u32)1 << ( fb % 32 );
		}
		if( a->n == 0 ) {
			strcpy( buf, neg ? "-0" : "0" );
			return;
		}
		lsb = ( e == 0 ? f->emin : e - f->emax ) - fb;
	} else if( e == 0 ) {
		lsb = f->emin - fb;
	} else {
		if( a->n <= fb / 32 ) a->n = fb / 32 + 1;
		a->w[ fb / 32 ] |= (u32)1 << ( fb % 32 );
		lsb = e - f->emax - fb;
	}
	/* exact value = a * 2^lsb = N * 10^min(lsb,0) */
	if( lsb >= 0 ) bn_shl( a, lsb );
	else bn_pow( a, 5, -lsb );
	t = &dec_digits[ sizeof( dec_digits ) - 1 ];
	*t = 0;
	while( a->n > 0 ) {
		u32	r = bn_div_small( a, 1000000000 );

		for( k = 0; k < 9; k++, r /= 10 ) *--t = (char)( '0' + r % 10 );
	}
	while( *t == '0' ) t++;
	nd = (int)strlen( t );
	x10 = nd - 1 + ( lsb < 0 ? lsb : 0 );
	lo = 1;					/* shortest that reads back: more */
	hi = 40;				/* digits never stop reading back, */
	while( lo < hi ) {			/* so binary search n */
		n = ( lo + hi ) / 2;
		k = dec_round( dig, t, nd, n, x10 );
		sprintf( cand, "%.*se%d", n, dig, k - ( n - 1 ) );
		dec_to_bin( cand, f, &bneg, &bkind, bm, &blsb );
		bin_pack( back, f, neg, bkind, bm, blsb );
		if( memcmp( back, o, bytes ) == 0 ) hi = n;
		else lo = n + 1;
	}
	n = lo;
	k = dec_round( dig, t, nd, n, x10 );
	g_format( buf, neg, dig, n, k );
}

/*
 *	Mio_Float_Out: float of kind 32, 64, 80 or 128 to the shortest
 *		text that reads back to the same value ("inf", "-0", "nan").
 */

void	Mio_Float_Out( char * buf, void * data, int kind )
{
	int	n;

	switch( kind ) {
	case 32:
		for( n = 1; n < 9; n++ ) {
			sprintf( buf, "%.*g", n, (f64)*(f32 *)data );
			if( strtof( buf, 0 ) == *(f32 *)data ) break;
		}
		sprintf( buf, "%.*g", n, (f64)*(f32 *)data );
		return;
	case 64:
		for( n = 1; n < 17; n++ ) {
			sprintf( buf, "%.*g", n, *(f64 *)data );
			if( strtod( buf, 0 ) == *(f64 *)data ) break;
		}
		sprintf( buf, "%.*g", n, *(f64 *)data );
		return;
#ifdef	F80_MANT_DIG
	case 80:			/* own engine: exact even where the */
		bin_to_dec( buf, data, &fmt_f80 );	/* compiler's long double */
		return;				/* math is not (c-saw) */
#endif
	case 128:
		bin_to_dec( buf, data, &fmt_f128 );
		return;
	}
	strcpy( buf, error_size );
}

/*
 *	Mio_Float_In: text to a float of kind 32, 64, 80 or 128, correctly
 *		rounded. Returns the number of characters used (0: none).
 */

int	Mio_Float_In( void * data, int kind, char * s )
{
	char *	e = s;
	int	neg;
	int	k;
	int	lsb;
	int	n;
	u32	m[ 5 ];

	switch( kind ) {
	case 32:	*(f32 *)data = strtof( s, &e );		return( (int)( e - s ) );
	case 64:	*(f64 *)data = strtod( s, &e );		return( (int)( e - s ) );
#ifdef	F80_MANT_DIG
	case 80:
		memset( m, 0, sizeof( m ) );
		lsb = 0;
		n = dec_to_bin( s, &fmt_f80, &neg, &k, m, &lsb );
		bin_pack( data, &fmt_f80, neg, k, m, lsb );
		return( n );
#endif
	case 128:
		memset( m, 0, sizeof( m ) );
		lsb = 0;
		n = dec_to_bin( s, &fmt_f128, &neg, &k, m, &lsb );
		bin_pack( data, &fmt_f128, neg, k, m, lsb );
		return( n );
	}
	return( 0 );
}

/* SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later */
/* Copyright (c) 2009-2026 John R. Smythe Jr. */
/*******************************************************************************
 * 
 *	Shared Memory Input Routines
 ******************************************************************************/
#include	<time.h>

#include	<now.h>
#include	"mio_local.h"

static	void	in_float( void * data, Mitem * i, char * string )
{
	switch( i->size ) {
	case sizeof( f32 ):	Mio_Float_In( data, 32, string );	return;
	case sizeof( f64 ):	Mio_Float_In( data, 64, string );	return;
	}
}

static	void	in_float80( void * data, Mitem * i, char * string )
{
#ifdef	F80_MANT_DIG
	if( i->size == sizeof( f80 ) ) Mio_Float_In( data, 80, string );
#endif
}

static	void	in_float128( void * data, Mitem * i, char * string )
{
	if( i->size == 16 ) Mio_Float_In( data, 128, string );
}

static	void	in_hexadecimal( void * data, Mitem * i, char * string )
{
	switch( i->size ) {
	case 1: case 2: case 4: case 8: case 16:
		Mio_Int_In( data, i->size, 0, 16, string );
	}
}

static	void	in_ip_addr( void * data, Mitem * i, char * string )
{
	u32	x[ 4 ];
	u32	ip;
	int	dots;
	char *	s;

	switch( i->size ) {
	default:		return;
	case sizeof( u32 ):	break;
	}
	dots = 0;
	ip = 0;
	x[ 0 ] = x[ 1 ] = x[ 2 ] = x[ 3 ] = 0;
	for( s = string; *s != 0; s++ ) {
		switch( *s ) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':	x[ dots ] *= 10;
				x[ dots ] += ( *s - '0' );
				continue;
		case '.':
				if( dots >= 3 ) break;
				dots += 1;
				continue;
		default:	break;
		}
		break;
	}
	switch( dots ) {
	case 0:
			ip  = ( ( x[ 0 ] & 0x000000FF ) << 24 );
			ip |= ( ( x[ 0 ] & 0x0000FF00 ) <<  8 );
			ip |= ( ( x[ 0 ] & 0x00FF0000 ) >>  8 );
			ip |= ( ( x[ 0 ] & 0xFF000000 ) >> 24 );
			break;
	case 1:
			ip  = ( ( x[ 1 ] & 0x000000FF ) << 24 );
			ip |= ( ( x[ 1 ] & 0x0000FF00 ) <<  8 );
			ip |= ( ( x[ 1 ] & 0x00FF0000 ) >>  8 );
			ip |= ( ( x[ 0 ] & 0x000000FF ) <<  0 );
			break;
	case 2:
			ip  = ( ( x[ 2 ] & 0x000000FF ) << 24 );
			ip |= ( ( x[ 2 ] & 0x0000FF00 ) <<  8 );
			ip |= ( ( x[ 1 ] & 0x000000FF ) <<  8 );
			ip |= ( ( x[ 0 ] & 0x000000FF ) <<  0 );
			break;
	case 3:
			ip  = ( ( x[ 3 ] & 0x000000FF ) << 24 );
			ip |= ( ( x[ 2 ] & 0x000000FF ) << 16 );
			ip |= ( ( x[ 1 ] & 0x000000FF ) <<  8 );
			ip |= ( ( x[ 0 ] & 0x000000FF ) <<  0 );
			break;
	}
	*(u32 *)data = ip; 
	return;
}

static	void	in_ip_port( void * data, Mitem * i, char * string )
{
	u64	x = strtoull( string, 0, 10 );

#if	__BYTE_ORDER == __LITTLE_ENDIAN
	switch( i->size ) {
	case sizeof( u8  ):	*(u8 *)data = x;
				return;
	case sizeof( u16 ):	( (u8 *)data )[ 0 ] = ( (u8 *)&x )[ 1 ];
				( (u8 *)data )[ 1 ] = ( (u8 *)&x )[ 0 ];
				return;

	case sizeof( u32 ):	( (u8 *)data )[ 0 ] = ( (u8 *)&x )[ 3 ];
				( (u8 *)data )[ 1 ] = ( (u8 *)&x )[ 2 ];
				( (u8 *)data )[ 2 ] = ( (u8 *)&x )[ 1 ];
				( (u8 *)data )[ 3 ] = ( (u8 *)&x )[ 0 ];
				return;
	case sizeof( u64 ):	( (u8 *)data )[ 0 ] = ( (u8 *)&x )[ 7 ];
				( (u8 *)data )[ 1 ] = ( (u8 *)&x )[ 6 ];
				( (u8 *)data )[ 2 ] = ( (u8 *)&x )[ 5 ];
				( (u8 *)data )[ 3 ] = ( (u8 *)&x )[ 4 ];
				( (u8 *)data )[ 4 ] = ( (u8 *)&x )[ 3 ];
				( (u8 *)data )[ 5 ] = ( (u8 *)&x )[ 2 ];
				( (u8 *)data )[ 6 ] = ( (u8 *)&x )[ 1 ];
				( (u8 *)data )[ 7 ] = ( (u8 *)&x )[ 0 ];
				return;
	}
#else
	switch( i->size ) {
	case sizeof( u8  ):	*(u8  *)data = x;	return;
	case sizeof( u16 ):	*(u16 *)data = x;	return;
	case sizeof( u32 ):	*(u32 *)data = x;	return;
	case sizeof( u64 ):	*(u64 *)data = x;	return;
	}
#endif
}


static	void	in_now( void * data, Mitem * i, char * string )
{
	switch( i->size ) {
	default:		return;
	case sizeof( u64 ):	break;
	}
	*(u64 *)data = Micro_From_String( string, NULL );
	return;
}

static	void	in_now_seconds( void * data, Mitem * i, char * string )
{
	int	j;
	tv_t	t;

	switch( i->size ) {
	default:		return;
	case sizeof( s64 ):	break;
	}
	t.tv_sec = strtoll( string, &string, 10 );
	t.tv_usec = 0;
	if( *string != 0 ) {
		string++;	// Skip '.'
		for( j = 0; j < 6; j++ ) {
			if( *string == 0 || !isdigit( *string ) ) {
				if( j == 0 ) break;
				for( ; j < 6; j++ ) t.tv_usec *= 10;
				break;
			}
			t.tv_usec *= 10;
			t.tv_usec += ( *string++ - '0' );
		}
	}
	*(s64 *)data = Micro_From_Timeval( t );
	return;
}

static	void	in_select( void * data, Mitem * i, char * string )
{
	u64	n;
	size_t	len;
	char **	l;

	l = (char **)i->list;
	if( l == 0 ) return;
	len = strlen( string );
	if( len == 0 ) return;
	for( n = 0; l[ n ] != 0; n ++ ) {
		if( strncasecmp( string, l[ n ], len ) != 0 ) continue;
		switch( i->size ) {
		case sizeof( s8  ):	*(s8  *)data = n; return;
		case sizeof( s16 ):	*(s16 *)data = n; return;
		case sizeof( s32 ):	*(s32 *)data = n; return;
		case sizeof( s64 ):	*(s64 *)data = n; return;
		}
		return;
	}
	if( isdigit( *string ) ) {
		u64	n = strtoull( string, 0, 10 );
		switch( i->size ) {
		case sizeof( u8  ):	*(u8  *)data = n; return;
		case sizeof( u16 ):	*(u16 *)data = n; return;
		case sizeof( u32 ):	*(u32 *)data = n; return;
		case sizeof( u64 ):	*(u64 *)data = n; return;
		}
	}
	if( *string == '+' || *string == '-' ) {
		s64	n = strtoll( string, 0, 10 );
		switch( i->size ) {
		case sizeof( u8  ):	*(u8  *)data = n; return;
		case sizeof( u16 ):	*(u16 *)data = n; return;
		case sizeof( u32 ):	*(u32 *)data = n; return;
		case sizeof( u64 ):	*(u64 *)data = n; return;
		}
	}
}

static	void	in_signed( void * data, Mitem * i, char * string )
{
	s64	x;

	switch( i->size ) {	/* up to 64 bits as before: strtoll, then truncate */
	case sizeof( s8  ):	x = strtoll( string, 0, 10 ); *(s8  *)data = x; return;
	case sizeof( s16 ):	x = strtoll( string, 0, 10 ); *(s16 *)data = x; return;
	case sizeof( s32 ):	x = strtoll( string, 0, 10 ); *(s32 *)data = x; return;
	case sizeof( s64 ):	x = strtoll( string, 0, 10 ); *(s64 *)data = x; return;
	case 16:		Mio_Int_In( data, 16, 1, 10, string ); return;
	}
}

static	void	in_stamp( void * data, Mitem * i, char * string )
{
	char	*	b;
	time_t		now;
	char		buf[ 6 ];
	struct	tm	tdat;
	struct	tm *	tm;

	now = time( 0 );
	tm = localtime_r( &now, &tdat );	/* Initializes tm_isdst */

	b = (char *)memset( buf, 0, sizeof( buf ) );
	switch( strlen( string ) ) {
	/*
	 *		1 to 4 Digit Year
	 */
	case	14:	if( !isdigit( ( *b++ = *string++ ) ) ) return;
	case	13:	if( !isdigit( ( *b++ = *string++ ) ) ) return;
	case	12:	if( !isdigit( ( *b++ = *string++ ) ) ) return;
	case	11:	if( !isdigit( ( *b++ = *string++ ) ) ) return;
			tm->tm_year = strtol( buf, 0, 10 );
			if( tm->tm_year < 0 ) tm->tm_year = 0;
			tm->tm_year %= 1900;
			b = (char *)memset( buf, 0, sizeof( buf ) );
	/*
	 *		1 to 2 Digit Month
	 */
	case	10:	if( !isdigit( ( *b++ = *string++ ) ) ) return;
	case	 9:	if( !isdigit( ( *b++ = *string++ ) ) ) return;
			tm->tm_mon = strtol( buf, 0, 10 ) - 1;
			if( tm->tm_mon < 0 ) tm->tm_mon = 0;
			tm->tm_mon %= 12;
			b = (char *)memset( buf, 0, sizeof( buf ) );
	/*
	 *		1 to 2 Digit Day Of Month
	 */
	case	 8:	if( !isdigit( ( *b++ = *string++ ) ) ) return;
	case	 7:	if( !isdigit( ( *b++ = *string++ ) ) ) return;
			tm->tm_mday = strtol( buf, 0, 10 );
			tm->tm_mday %= 32;
			if( tm->tm_mday < 1 ) tm->tm_mday = 1;
			b = (char *)memset( buf, 0, sizeof( buf ) );
	/*
	 *		1 to 2 Digit Hour
	 */
	case	 6:	if( !isdigit( ( *b++ = *string++ ) ) ) return;
	case	 5:	if( !isdigit( ( *b++ = *string++ ) ) ) return;
			tm->tm_hour = strtol( buf, 0, 10 );
			if( tm->tm_hour < 0 ) tm->tm_hour = 0;
			tm->tm_hour %= 24;
			b = (char *)memset( buf, 0, sizeof( buf ) );
	/*
	 *		1 to 2 Digit Minutes
	 */
	case	 4:	if( !isdigit( ( *b++ = *string++ ) ) ) return;
	case	 3:	if( !isdigit( ( *b++ = *string++ ) ) ) return;
			tm->tm_min = strtol( buf, 0, 10 );
			if( tm->tm_min < 0 ) tm->tm_min = 0;
			tm->tm_min %= 60;
			b = (char *)memset( buf, 0, sizeof( buf ) );
	/*
	 *		1 to 2 Digit Seconds
	 */
	case	 2:	if( !isdigit( ( *b++ = *string++ ) ) ) return;
	case	 1:	if( !isdigit( ( *b++ = *string++ ) ) ) return;
			tm->tm_sec = strtol( buf, 0, 10 );
			if( tm->tm_sec < 0 ) tm->tm_sec = 0;
			tm->tm_sec %= 60;
			b = (char *)memset( buf, 0, sizeof( buf ) );
			break;
	case	 0:	return;
	}
	if( sizeof( time_t ) == 8 && i->size == 4 ) {
		*(u32 *)data = (u32)mktime( tm );
	} else {
		*(time_t *)data = mktime( tm );
	}
}

static	void	in_string( void * data, Mitem * i, char * string )
{
	memset(  data, 0,      i->size );
	strncpy( data, string, i->size );
}

static	void	in_unsigned( void * data, Mitem * i, char * string )
{
	u64	x;

	switch( i->size ) {	/* up to 64 bits as before: strtoull, then truncate */
	case sizeof( u8  ):	x = strtoull( string, 0, 10 ); *(u8  *)data = x; return;
	case sizeof( u16 ):	x = strtoull( string, 0, 10 ); *(u16 *)data = x; return;
	case sizeof( u32 ):	x = strtoull( string, 0, 10 ); *(u32 *)data = x; return;
	case sizeof( u64 ):	x = strtoull( string, 0, 10 ); *(u64 *)data = x; return;
	case 16:		Mio_Int_In( data, 16, 0, 10, string ); return;
	}
}

MioItemHandler MioValueIn[ MAX_MTYPE + 1 ] = {
	0,			/* END_OF_LIST	*/
	0,			/* STRUCTURE	*/
	in_string,		/* STRING	*/
	in_unsigned,		/* UNSIGNED	*/
	in_signed,		/* SIGNED	*/
	in_float,		/* FLOAT	*/
	in_hexadecimal,		/* HEXADECIMAL	*/
	in_stamp,		/* STAMP	*/
	in_select,		/* SELECT	*/
	0,		/* MSGADDR: registered by MioMsgInit() */
	in_ip_addr,		/* IP_ADDR	*/
	in_ip_port,		/* IP_PORT	*/
	in_now,			/* NOW		*/
	in_now_seconds,		/* NOWSECS	*/
	in_float80,		/* FLOAT80	*/
	in_float128,		/* FLOAT128	*/
	0
};

static	void	Input_Item( void * m, Mitem * l, char * value )
{
	if( m == 0 ) return;
	if( l == 0 || l->type == END_OF_LIST ) return;
	if( l->type >= MAX_MTYPE || MioValueIn[ l->type ] == NULL ) {
		errno = ENOTSUP;
		fprintf( stderr, "mio: unsupported input field type %u: %s\n", l->type, error_func );
		strcpy( value, error_func );
		return;
	}
	if( getenv( "MEMORY_ALL" ) == NULL && ( l->flags & DISPLAY_ONLY ) != 0 ) return;
	MioValueIn[ l->type ]( m, l, value );
	return;
}

void	Input_Memory_Item( void * m, Mitem * l, char * n, char * v )
{
	memset(  Item_Name,  0, sizeof( Item_Name  ) );
	memset(  Item_Value, 0, sizeof( Item_Value ) );
	strncpy( Item_Name,  n, sizeof( Item_Name  ) );
	strncpy( Item_Value, v, sizeof( Item_Value ) );
	List_Of_Items( m, l, Item_Name, Input_Item );
}

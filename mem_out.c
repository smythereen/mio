/* SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later */
/* Copyright (c) 2009-2026 John R. Smythe Jr. */
/*******************************************************************************
 * 
 *	Shared Memory Print Routines
 ******************************************************************************/
#include	"mio_local.h"
#include	<time.h>
#include	<now.h>
#include	<inttypes.h>

/*
 *	Floats: shortest text that reads back to the same bits (mem_num.c).
 *	An item format still applies to f32/f64 (passed as f64) and f80.
 */

static	void	out_float( void * data, Mitem * i, char * buf )
{
	switch( i->size ) {
	default:		strcpy( buf, error_size );
				return;
	case sizeof( f32 ):	if( i->format != (char *)0 ) {
					sprintf( buf, i->format, (f64)*(f32 *)data );
					return;
				}
				Mio_Float_Out( buf, data, 32 );
				return;
	case sizeof( f64 ):	if( i->format != (char *)0 ) {
					sprintf( buf, i->format, *(f64 *)data );
					return;
				}
				Mio_Float_Out( buf, data, 64 );
				return;
	}
}

static	void	out_float80( void * data, Mitem * i, char * buf )
{
#ifdef	F80_MANT_DIG
	if( i->size == sizeof( f80 ) ) {
		if( i->format != (char *)0 ) {
			sprintf( buf, i->format, *(f80 *)data );
			return;
		}
		Mio_Float_Out( buf, data, 80 );
		return;
	}
#endif
	strcpy( buf, error_size );
}

static	void	out_float128( void * data, Mitem * i, char * buf )
{
	if( i->size == 16 ) {
		Mio_Float_Out( buf, data, 128 );
		return;
	}
	strcpy( buf, error_size );
}

static	void	out_hexadecimal( void * data, Mitem * i, char * buf )
{
	u64	x;

	switch( i->size ) {
	default:		strcpy( buf, error_size );
				return;
	case 16:		Mio_Int_Out( buf, data, 16, 0, 16 );
				return;
	case sizeof( u64 ):	x = *(u64 *)data;	break;
	case sizeof( u32 ):	x = *(u32 *)data;	break;
	case sizeof( u16 ):	x = *(u16 *)data;	break;
	case sizeof( u8  ):	x = *(u8  *)data;	break;
	}
	if( i->format != (char *)0 ) {
		sprintf( buf, i->format, x );
		return;
	}
	Mio_Int_Out( buf, data, i->size, 0, 16 );
}

static	void	out_ip_addr( void * data, Mitem * i, char * buf )
{
	uchar *	x;

	switch( i->size ) {
	default:		strcpy( buf, error_size );
				return;
	case sizeof( u32 ):	x = (uchar *)data;
				break;
	}
	sprintf( buf, ( i->format != (char *)0 ? i->format : "%u.%u.%u.%u" ), x[0], x[1], x[2], x[3] );
}

static	void	out_ip_port( void * data, Mitem * i, char * buf )
{
	u64	x = 0;

#if	__BYTE_ORDER == __LITTLE_ENDIAN
	switch( i->size ) {
	default:		strcpy( buf, error_size );	return;
	case sizeof( u64 ):	((u8 *)&x)[ 0 ] = ((u8 *)data)[ 7 ];
				((u8 *)&x)[ 1 ] = ((u8 *)data)[ 6 ];
				((u8 *)&x)[ 2 ] = ((u8 *)data)[ 5 ];
				((u8 *)&x)[ 3 ] = ((u8 *)data)[ 4 ];
				((u8 *)&x)[ 4 ] = ((u8 *)data)[ 3 ];
				((u8 *)&x)[ 5 ] = ((u8 *)data)[ 2 ];
				((u8 *)&x)[ 6 ] = ((u8 *)data)[ 1 ];
				((u8 *)&x)[ 7 ] = ((u8 *)data)[ 0 ];
				break;
	case sizeof( u32 ):	((u8 *)&x)[ 0 ] = ((u8 *)data)[ 3 ];
				((u8 *)&x)[ 1 ] = ((u8 *)data)[ 2 ];
				((u8 *)&x)[ 2 ] = ((u8 *)data)[ 1 ];
				((u8 *)&x)[ 3 ] = ((u8 *)data)[ 0 ];
				break;
	case sizeof( u16 ):	((u8 *)&x)[ 0 ] = ((u8 *)data)[ 1 ];
				((u8 *)&x)[ 1 ] = ((u8 *)data)[ 0 ];
				break;
	case sizeof( u8  ):	((u8 *)&x)[ 0 ] = ((u8 *)data)[ 0 ];
				break;
	}
#else
	switch( i->size ) {
	default:		strcpy( buf, error_size );	return;
	case sizeof( u64 ):	x = *(u64 *)data;		break;
	case sizeof( u32 ):	x = *(u32 *)data;		break;
	case sizeof( u16 ):	x = *(u16 *)data;		break;
	case sizeof( u8  ):	x = *(u8  *)data;		break;
	}
#endif
	sprintf( buf, ( i->format != (char *)0 ? i->format : "%" PRIu64 ), x );
}

static	void	out_now( void * data, Mitem * i, char * buf )
{
	u64	x;

	switch( i->size ) {
	default:		strcpy( buf, error_size );
				return;
	case sizeof( u64 ):	x = *(u64 *)data;
				break;
	}
	Micro_To_String_Local( buf, sizeof( Item_Value ), "%Y-%m-%d-%H-%M-%S", x );
}

static	void	out_now_seconds( void * data, Mitem * i, char * buf )
{
	s64	x;
	tv_t	t;
	char *	p;

	switch( i->size ) {
	default:		strcpy( buf, error_size );
				return;
	case sizeof( s64 ):	x = *(s64 *)data;
				break;
	}
	if( x < 0 ) {
		p = "-";
		x *= -1;
	} else {
		p = "+";
	}
	t.tv_sec = x / 1000000;
	t.tv_usec = x % 1000000;
	sprintf( buf, "%s%" PRIu64 ".%06u", p, (u64)t.tv_sec, (u32)t.tv_usec );
}

static	void	out_select( void * data, Mitem * i, char * buf )
{
	u64	x;
	u64	n;
	char **	l;

	switch( i->size ) {
	default:		strcpy( buf, error_size );	return;
	case sizeof( u64 ):	x = *(u64 *)data;		break;
	case sizeof( u32 ):	x = *(u32 *)data;		break;
	case sizeof( u16 ):	x = *(u16 *)data;		break;
	case sizeof( u8  ):	x = *(u8  *)data;		break;
	}
	for( l = (char **)i->list, n = 0; n <= x; n++ ) {
		if( l[ n ] == 0 ) {
			sprintf( buf, ( i->format != (char *)0 ? i->format : "%" PRIu64 ), x );
			return;
		}
	}
	if( i->format != (char *)0 && i->size >= sizeof( Item_Value ) ) {
		strcpy( buf, error_size );
		return;
	}
	if( Item_Prefix[ 0 ] == 0 ) {
		sprintf( buf, ( i->format != 0 ? i->format : "%14.14s" ), l[ x ] );
	} else {
		sprintf( buf, "\"%.*s\"", (int)strlen( l[ x ] ), l[ x ] );
	}
}

static	void	out_signed( void * data, Mitem * i, char * buf )
{
	s64	x;

	switch( i->size ) {
	default:		strcpy( buf, error_size );	return;
	case 16:		Mio_Int_Out( buf, data, 16, 1, 10 );	return;
	case sizeof( s64 ):	x = *(s64 *)data;		break;
	case sizeof( s32 ):	x = *(s32 *)data;		break;
	case sizeof( s16 ):	x = *(s16 *)data;		break;
	case sizeof( s8  ):	x = *(s8  *)data;		break;
	}
	if( i->format != (char *)0 ) {
		sprintf( buf, i->format, x );
		return;
	}
	Mio_Int_Out( buf, data, i->size, 1, 10 );
}

static	void	out_stamp( void * data, Mitem * i, char * buf )
{
	time_t	d;
	struct	tm	tdat;

	switch( i->size ) {
	case 4:	d = (time_t)*(u32 *)data;
		break;
	case 8:	d = (time_t)*(u64 *)data;
		break;
	default:
		strcpy( buf, error_size );
		return;
	}
	if( i->format   ==   (char *)0  ) {
		strcpy( buf, error_frmt );
		return;
	}
#ifdef	CFTIME
	cftime( buf, i->format, &d );
#else
	strftime(	buf,
			sizeof( Item_Value ),
			i->format,
			localtime_r( &d, &tdat )
	);
#endif
}

static	void	out_string( void * data, Mitem * i, char * buf )
{

	if( i->format != (char *)0 && i->size >= sizeof( Item_Value ) ) {
		strcpy( buf, error_size );
		return;
	}
	if( Item_Prefix[ 0 ] == 0 ) {
		sprintf( buf, ( i->format != 0 ? i->format : "%12.12s" ), (char *)data );
	} else {
		sprintf( buf, "\"%.*s\"", (int)i->size, (char *)data );
	}
}

static	void	out_unsigned( void * data, Mitem * i, char * buf )
{
	u64	x;

	switch( i->size ) {
	default:		strcpy( buf, error_size );	return;
	case 16:		Mio_Int_Out( buf, data, 16, 0, 10 );	return;
	case sizeof( u64 ):	x = *(u64 *)data;		break;
	case sizeof( u32 ):	x = *(u32 *)data;		break;
	case sizeof( u16 ):	x = *(u16 *)data;		break;
	case sizeof( u8  ):	x = *(u8  *)data;		break;
	}
	if( i->format != (char *)0 ) {
		sprintf( buf, i->format, x );
		return;
	}
	Mio_Int_Out( buf, data, i->size, 0, 10 );
}

MioItemHandler MioValueOut[ MAX_MTYPE + 1 ] = {
	0,			/* END_OF_LIST	*/
	0,			/* STRUCTURE	*/
	out_string,		/* STRING	*/
	out_unsigned,		/* UNSIGNED	*/
	out_signed,		/* SIGNED	*/
	out_float,		/* FLOAT	*/
	out_hexadecimal,	/* HEXADECIMAL	*/
	out_stamp,		/* STAMP	*/
	out_select,		/* SELECT	*/
	0,		/* MSGADDR: registered by MioMsgInit() */
	out_ip_addr,		/* IP_ADDR	*/
	out_ip_port,		/* IP_PORT	*/
	out_now,		/* NOW		*/
	out_now_seconds,	/* NOWSECS	*/
	out_float80,		/* FLOAT80	*/
	out_float128,		/* FLOAT128	*/
	0
};

static	void    Output_Item( void * m, Mitem * l, char * value )
{
	if( m == 0 ) return;
	if( l == 0 || l->type == END_OF_LIST ) return;
	if( l->type >= MAX_MTYPE || MioValueOut[ l->type ] == NULL ) {
		errno = ENOTSUP;
		strcpy( value, error_func );
	} else {
		MioValueOut[ l->type ]( m, l, value );
	}
	if( getenv( "MEMORY_ALL" ) == NULL && ( l->flags & INPUT_ONLY ) != 0 ) return;
	if( Item_Prefix[ 0 ] == 0 ) {
		fprintf( stdout, f_display, l->description, value, Item_Name );
	} else {
		fprintf( stdout, f_prefixed, Item_Prefix, Item_Name, value );
	}
	fflush( stdout );
	return;
}

void	Output_Memory_Item( void * m, Mitem * l, char * n, char * p )
{
	memset(  Item_Prefix, 0, sizeof( Item_Prefix ) );
	memset(  Item_Name,   0, sizeof( Item_Name   ) );
	memset(  Item_Value,  0, sizeof( Item_Value  ) );
	if( n != 0 ) strncpy( Item_Name,   n, sizeof( Item_Name   ) );
	if( p != 0 ) strncpy( Item_Prefix, p, sizeof( Item_Prefix ) );
	List_Of_Items( m, l, Item_Name, Output_Item );
}

static	int	newline = 1;

static	void    Header_CSV( void * m, Mitem * l, char * value )
{
	if( m == 0 ) return;
	if( l == 0 || l->type == END_OF_LIST ) return;
	if( l->type >= MAX_MTYPE || MioValueOut[ l->type ] == NULL ) {
		errno = ENOTSUP;
		strcpy( value, error_func );
	} else {
		MioValueOut[ l->type ]( m, l, value );
	}
	if( getenv( "MEMORY_ALL" ) == NULL && ( l->flags & INPUT_ONLY ) != 0 ) return;
	if( newline ) {
		fprintf( stdout, "%s,\"%s\"", Item_Prefix, Item_Name );
		newline = 0;
	} else {
		fprintf( stdout, ",\"%s\"", Item_Name );
	}
	fflush( stdout );
	return;
}

static	void    Output_CSV( void * m, Mitem * l, char * value )
{
	if( m == 0 ) return;
	if( l == 0 || l->type == END_OF_LIST ) return;
	if( l->type >= MAX_MTYPE || MioValueOut[ l->type ] == NULL ) {
		errno = ENOTSUP;
		strcpy( value, error_func );
	} else {
		MioValueOut[ l->type ]( m, l, value );
	}
	if( getenv( "MEMORY_ALL" ) == NULL && ( l->flags & INPUT_ONLY ) != 0 ) return;
	if( newline ) {
		fprintf( stdout, "%s,%s", Item_Prefix, value );
		newline = 0;
	} else {
		fprintf( stdout, ",%s", value );
	}
	fflush( stdout );
	return;
}

	int	CSV_Header = 1;

void	Output_Memory_CSV( void * m, Mitem * l, char * n, char * p )
{
	if( CSV_Header ) {
		memset(  Item_Prefix, 0, sizeof( Item_Prefix ) );
		memset(  Item_Name,   0, sizeof( Item_Name   ) );
		memset(  Item_Value,  0, sizeof( Item_Value  ) );
		if( n != 0 ) strncpy( Item_Name,   n, sizeof( Item_Name   ) );
		if( p != 0 ) strncpy( Item_Prefix, "\"\"", sizeof( Item_Prefix ) );
		List_Of_Items( m, l, Item_Name, Header_CSV );
		fprintf( stdout, "\n" );
		fflush( stdout );
		CSV_Header = 0;
		newline = 1;
	}
	memset(  Item_Prefix, 0, sizeof( Item_Prefix ) );
	memset(  Item_Name,   0, sizeof( Item_Name   ) );
	memset(  Item_Value,  0, sizeof( Item_Value  ) );
	if( n != 0 ) strncpy( Item_Name,   n, sizeof( Item_Name   ) );
	if( p != 0 ) strncpy( Item_Prefix, p, sizeof( Item_Prefix ) );
	List_Of_Items( m, l, Item_Name, Output_CSV );
	fprintf( stdout, "\n" );
	fflush( stdout );
	newline = 1;
}


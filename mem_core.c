/* SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later */
/* Copyright (c) 2007-2026 John R. Smythe Jr. */
/*******************************************************************************
 * 
 *	Shared Memory Item Processing Core Routines
 ******************************************************************************/
#include	"mio_local.h"

	char	Item_Prefix[  512 ];
	char	Item_Name[  512 ];
	char	Item_Value[ 512 ];

	char	error_frmt[ ] = "Format ERR";
	char	error_func[ ] = "Func.ERROR";
	char	error_list[ ] = "List ERROR";
	char	error_size[ ] = "Size ERROR";
	char	error_type[ ] = "Type ERROR";

	char	f_date[ ] = "%D";
	char	f_stmp[ ] = "%Y%m%d%H%M%S";
	char	f_time[ ] = "%T";

	char	f_display[ ] = "%-40.40s | %12.12s | %s\n";
	char	f_prefixed[  ] = "%s %-40s %s\n";

static	char *	terminator( char * name )
{
	if( name == 0 ) return( name );
	for( ; ; name += 1 ) {
		switch( *name ) {
		case 0:
		case '.':
		case ']':
		case '[': return( name );
		}
	}
	return( name );
}

void	Array_Of_Items(	void  * m,
			Mitem *	l,
			char  *	name,
			void (* func)( void * m, Mitem * l, char * n )
	)
{
	u32	i;
	char *	n;

	if( m       == (void  *)0  ) return;
	if( l       == (Mitem *)0  ) return;
	if( l->type == END_OF_LIST ) return;
	n = name;
	if( *n != 0 ) {
		if( *n == '[' ) {
			n += 1;
			i = strtoul( n, 0, 10 );
			if( i >= l->array ) return;
			if( i > 0 ) m = (void *)( (u_t)m + l->size * i );
			switch( *( n = terminator( n ) ) ) {
			default:	return;
			case ']':	n++;
			case  0 :
			case '.':	break;
			}
		}
		if( l->type == STRUCTURE ) {
			if( *n ==  0  ) {
				if( Item_Value[ 0 ] != 0 ) return;
				*n++ = '.';
			}
			if( *n == '.' )  n++;
			if( *n ==  0  ) {
				if( Item_Value[ 0 ] != 0 ) return;
			}
			List_Of_Items( m, (Mitem *)l->list, n, func );
			return;
		}
		*n = 0;
		func( m, l, Item_Value );
		return;
	}
	for( i = 0; i < l->array; i++, m = (void *)( (u_t)m + l->size ) ) {
		if( getenv( "MEMORY_ALL" ) == NULL &&
		    ( l->flags & NO_ARRAY_DISPLAY ) != 0 && i > 0 ) continue;
		n = name;
		if( l->name != (char *)0 && *l->name != 0 && l->array > 1 ) {
			n = &n[ sprintf( n, "[%u]", i ) ];
		}
		if( l->type == STRUCTURE ) {
			*n++ = '.';
			*n = 0;
			List_Of_Items( m, (Mitem *)l->list, n, func );
			continue;
		}
		func( m, l, Item_Value );
	}
}

void	List_Of_Items(	void  *	m,
			Mitem *	l,
			char  *	name,
			void (* func)( void * m, Mitem * l, char * n )
	)
{
	char *	s;	/* Source Pointer */
	char *	t;	/* Target Pointer */
	char *	save_ptr;
	char	save_char;

	if( m == (void  *)0 ) return;
	if( l == (Mitem *)0 ) return;
	if( *name == 0 ) {
		for( ; l->type != END_OF_LIST; l += 1 ) {
			for( t = name, s = l->name; *s != 0; *t++ = *s++ );
			*t = 0;
			Array_Of_Items(
				(void *)( (u_t)m + l->displacement ),
				l,
				t,
				func
			);
		}
		*name = 0;
		return;
	}
	save_char = *( save_ptr = terminator( name ) );
	*save_ptr = 0;
	for( ; l->type != END_OF_LIST; l += 1 ) {
		if( strcmp( name, l->name ) != 0 ) continue;
		*save_ptr = save_char;
		Array_Of_Items(
			(void *)( (u_t)m + l->displacement ),
			l,
			save_ptr,
			func
		);
		return;
	}
	*save_ptr = save_char;
}

/* Register as a pair so input and output agree on the representation. */
int MioRegisterHandler( Mtyp_t type, MioItemHandler input, MioItemHandler output )
{
	if( type <= STRUCTURE || type >= MAX_MTYPE || input == NULL || output == NULL ) {
		errno = EINVAL;
		return( -1 );
	}
	MioValueIn[ type ] = input;
	MioValueOut[ type ] = output;
	return( 0 );
}

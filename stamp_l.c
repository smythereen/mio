/* SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later */
/* Copyright (c) 2007-2026 John R. Smythe Jr. */
/*******************************************************************************
 *
 *	stamp_t / timeval Memory Format Data
 *
 *	Shared mio field list for struct timeval (typedef stamp_t in common.h).
 *	Used by any unit that displays or edits timeval/stamp_t fields through mio.
 *	Not specific to TPH/TP counters.
 ******************************************************************************/
#include	<common.h>
#include	<mem_io.h>

/*
 *	stamp_l — timeval fields (tv_sec as Stamp, tv_usec as Ulong)
 */

#define	STRUCT	stamp_t
	Mitem		stamp_l[ ] =	{
	Stamp(		tv_sec,		"Date & Time"	),
	Ulong(		tv_usec,	"Microseconds"	),
	{ END_OF_LIST }
};
#undef	STRUCT

/* SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later */
/* Copyright (c) 2009-2026 John R. Smythe Jr. */
/*******************************************************************************
 * 
 *	Shared Memory Print Routine Definitions & Prototypes
 ******************************************************************************/
#ifndef	MEM_IO_H
#define	MEM_IO_H

#if	defined( __cplusplus )
extern	"C"	{
#endif

#include	<common.h>

typedef	struct	mask_s	{
	char	*	name;
	u32		value;
}	mask_t;

typedef	enum	Mtyp_e	{
	END_OF_LIST	= 0x0,
	STRUCTURE,
	STRING,
	UNSIGNED,
	SIGNED,
	FLOAT,
	HEXADECIMAL,
	STAMP,
	SELECT,
	MSGADDR,
	IP_ADDR,
	IP_PORT,
	NOW,
	NOWSECS,
	FLOAT80,		/* f80: x87 extended, when F80_MANT_DIG */
	FLOAT128,		/* f128: IEEE quad, when F128_MANT_DIG */
	MAX_MTYPE
}	Mtyp_t;

typedef	enum	Mflg_e	{
	NORMAL_OP		= 0x00,
	INPUT_ONLY		= 0x01,
	DISPLAY_ONLY		= 0x02,
	NO_ARRAY_DISPLAY	= 0x04, /* Displays First Element Of Array Only */
	BIT_BASED		= 0x08
}	Mflg_t;

typedef	struct	{
	u8		type;
	u8		flags;
	u_t		displacement;
	u_t		size;
	u_t		array;
	char	*	name;
	char	*	description;
	char	*	format;
	void	*	list;
}	Mitem;

#define	F_DATE		"%D"
#define	F_STMP		"%Y%m%d%H%M%S"
#define	F_TIME		"%T"

#define	BITS(t,u,x,d,o) {	(u8)t,\
				(u8)0x08,\
				(u_t)o,\
				(u_t)sizeof(u),\
				(u_t)1,\
				#x,\
				(char *)d,\
				(char *)0,\
				(void *)0 }

#define	BitChar(x,o,d)		BITS( HEXADECIMAL, uchar,	x, d,	o	)

#define	ITEM(t,u,x,d,f,l) {	(u8)t,\
				(u8)0,\
				(u_t)&((STRUCT *)0)->x,\
				(u_t)sizeof(u),\
				(u_t)(sizeof(((STRUCT *)0)->x)/sizeof(u)),\
				#x,\
				(char *)d,\
				(char *)f,\
				(void *)l }

#define	IteM(o,t,u,x,d,f,l) {	(u8)t,\
				(u8)( o ),\
				(u_t)&((STRUCT *)0)->x,\
				(u_t)sizeof(u),\
				(u_t)(sizeof(((STRUCT *)0)->x)/sizeof(u)),\
				#x,\
				(char *)d,\
				(char *)f,\
				(void *)l }

#define	Char(x,d)		ITEM(	SIGNED,		s8,	x, d,	0,	0 )
#define	Char_(o,x,d)		IteM( o,SIGNED,		s8,	x, d,	0,	0 )

#define	Date(x,d)		ITEM(	STAMP,		time_t,	x, d,	F_DATE,	0 )
#define	Date_(o,x,d)		IteM( o,STAMP,		time_t,	x, d,	F_DATE,	0 )

#define	Date32(x,d)		ITEM(	STAMP,		u32,	x, d,	F_DATE,	0 )
#define	Date32_(o,x,d)		IteM( o,STAMP,		u32,	x, d,	F_DATE,	0 )

#define	Date64(x,d)		ITEM(	STAMP,		u64,	x, d,	F_DATE,	0 )
#define	Date64_(o,x,d)		IteM( o,STAMP,		u64,	x, d,	F_DATE,	0 )

#define	Double(x,d)		ITEM(	FLOAT,		f64,	x, d,	0,	0 )
#define	Double_(o,x,d)		IteM( o,FLOAT,		f64,	x, d,	0,	0 )

/*
 *	Larger floats exist only when common.h defines them (F80_MANT_DIG,
 *	F128_MANT_DIG); F_t / D_t follow f_t (F_T_MANT_DIG).
 */

#ifdef	F80_MANT_DIG
#define	D80(x,d)		ITEM(	FLOAT80,	f80,	x, d,	0,	0 )
#define	D80_(o,x,d)		IteM( o,FLOAT80,	f80,	x, d,	0,	0 )
#define	F80(x,d)		ITEM(	FLOAT80,	f80,	x, d,	0,	0 )
#define	F80_(o,x,d)		IteM( o,FLOAT80,	f80,	x, d,	0,	0 )
#endif

#ifdef	F128_MANT_DIG
#define	D128(x,d)		ITEM(	FLOAT128,	f128,	x, d,	0,	0 )
#define	D128_(o,x,d)		IteM( o,FLOAT128,	f128,	x, d,	0,	0 )
#define	F128(x,d)		ITEM(	FLOAT128,	f128,	x, d,	0,	0 )
#define	F128_(o,x,d)		IteM( o,FLOAT128,	f128,	x, d,	0,	0 )
#endif

#if	defined( F128_MANT_DIG ) && F_T_MANT_DIG == F128_MANT_DIG
#define	MIO_F_T		FLOAT128
#elif	defined( F80_MANT_DIG ) && F_T_MANT_DIG == F80_MANT_DIG
#define	MIO_F_T		FLOAT80
#else
#define	MIO_F_T		FLOAT
#endif
#define	D_t(x,d)		ITEM(	MIO_F_T,	f_t,	x, d,	0,	0 )
#define	D_t_(o,x,d)		IteM( o,MIO_F_T,	f_t,	x, d,	0,	0 )
#define	F_t(x,d)		ITEM(	MIO_F_T,	f_t,	x, d,	0,	0 )
#define	F_t_(o,x,d)		IteM( o,MIO_F_T,	f_t,	x, d,	0,	0 )

#define	D64(x,d)		ITEM(	FLOAT,		f64,	x, d,	0,	0 )
#define	D64_(o,x,d)		IteM( o,FLOAT,		f64,	x, d,	0,	0 )

#define	D32(x,d)		ITEM(	FLOAT,		f32,	x, d,	0,	0 )
#define	D32_(o,x,d)		IteM( o,FLOAT,		f32,	x, d,	0,	0 )

#define	Float(x,d)		ITEM(	FLOAT,		f32,	x, d,	0,	0 )
#define	Float_(o,x,d)		IteM( o,FLOAT,		f32,	x, d,	0,	0 )

#define	F64(x,d)		ITEM(	FLOAT,		f64,	x, d,	0,	0 )
#define	F64_(o,x,d)		IteM( o,FLOAT,		f64,	x, d,	0,	0 )

#define	F32(x,d)		ITEM(	FLOAT,		f32,	x, d,	0,	0 )
#define	F32_(o,x,d)		IteM( o,FLOAT,		f32,	x, d,	0,	0 )

#define	Huge(x,d)		ITEM(	SIGNED,		s64,	x, d,	0,	0 )
#define	Huge_(o,x,d)		IteM( o,SIGNED,		s64,	x, d,	0,	0 )

#define	IP_Addr(x,d)		ITEM(	IP_ADDR,	u32,	x, d,	0,	0 )
#define	IP_Addr_(o,x,d)		IteM( o,IP_ADDR,	u32,	x, d,	0,	0 )

#define	IP_Port(x,d)		ITEM(	IP_PORT,	u16,	x, d,	0,	0 )
#define	IP_Port_(o,x,d)		IteM( o,IP_PORT,	u16,	x, d,	0,	0 )

#define	Long(x,d)		ITEM(	SIGNED,		u32,	x, d,	0,	0 )
#define	Long_(o,x,d)		IteM( o,SIGNED,		u32,	x, d,	0,	0 )

#define	Mchar(x,l,d)		ITEM(	HEXADECIMAL,	u8,	x, d,	0,	l )
#define	Mchar_(o,x,l,d)		IteM( o,HEXADECIMAL,	u8,	x, d,	0,	l )

#define	Mhuge(x,l,d)		ITEM(	HEXADECIMAL,	u64,	x, d,	0,	l )
#define	Mhuge_(o,x,l,d)		IteM( o,HEXADECIMAL,	u64,	x, d,	0,	l )

#define	Mlong(x,l,d)		ITEM(	HEXADECIMAL,	u32,	x, d,	0,	l )
#define	Mlong_(o,x,l,d)		IteM( o,HEXADECIMAL,	u32,	x, d,	0,	l )

#define	Mshort(x,l,d)		ITEM(	HEXADECIMAL,	u16,	x, d,	0,	l )
#define	Mshort_(o,x,l,d)	IteM( o,HEXADECIMAL,	u16,	x, d,	0,	l )

#ifdef	MSG_H

#ifdef MSG_HEAD_SIZE
#define	MsgAddr(x,d)		ITEM(	MSGADDR,	MsgAddr_t,	x, d,	0,	0 )
#define	MsgAddr_(o,x,d)		IteM( o,MSGADDR,	MsgAddr_t,	x, d,	0,	0 )

#else
#define	MsgAddr(x,d)		ITEM(	MSGADDR,	u32,	x, d,	0,	0 )
#define	MsgAddr_(o,x,d)		IteM( o,MSGADDR,	u32,	x, d,	0,	0 )

#endif

#endif

#ifdef	NOW_H

#define	Now(x,d)		ITEM(	NOW,		u64,	x, d,	0,	0 )
#define	Now_(o,x,d)		IteM( o,NOW,		u64,	x, d,	0,	0 )

#define	NowSecs(x,d)		ITEM(	NOWSECS,	s64,	x, d,	0,	0 )
#define	NowSecs_(o,x,d)		IteM( o,NOWSECS,	s64,	x, d,	0,	0 )

#endif

#define	S8(x,d)			ITEM(	SIGNED,		s8,	x, d,	0,	0 )
#define	S8_(o,x,d)		IteM( o,SIGNED,		s8,	x, d,	0,	0 )

#define	S16(x,d)		ITEM(	SIGNED,		s16,	x, d,	0,	0 )
#define	S16_(o,x,d)		IteM( o,SIGNED,		s16,	x, d,	0,	0 )

#define	S32(x,d)		ITEM(	SIGNED,		s32,	x, d,	0,	0 )
#define	S32_(o,x,d)		IteM( o,SIGNED,		s32,	x, d,	0,	0 )

#define	S64(x,d)		ITEM(	SIGNED,		s64,	x, d,	0,	0 )
#define	S64_(o,x,d)		IteM( o,SIGNED,		s64,	x, d,	0,	0 )

#define	S128(x,d)		ITEM(	SIGNED,		s128,	x, d,	0,	0 )
#define	S128_(o,x,d)		IteM( o,SIGNED,		s128,	x, d,	0,	0 )

#define	S_t(x,d)		ITEM(	SIGNED,		s_t,	x, d,	0,	0 )
#define	S_t_(o,x,d)		IteM( o,SIGNED,		s_t,	x, d,	0,	0 )

#define	Select(x,l,d)		ITEM(	SELECT, ((STRUCT *)0)->x, x, d,	0,	l )
#define	Select_(o,x,l,d)	IteM( o,SELECT, ((STRUCT *)0)->x, x, d,	0,	l )

#define	SelChar(x,l,d)		ITEM(	SELECT,		u8,	x, d,	0,	l )
#define	SelChar_(o,x,l,d)	IteM( o,SELECT,		u8,	x, d,	0,	l )

#define	SelLong(x,l,d)		ITEM(	SELECT,		u32,	x, d,	0,	l )
#define	SelLong_(o,x,l,d)	IteM( o,SELECT,		u32,	x, d,	0,	l )

#define	SelShort(x,l,d)		ITEM(	SELECT,		u16,	x, d,	0,	l )
#define	SelShort_(o,x,l,d)	IteM( o,SELECT,		u16,	x, d,	0,	l )

#define	Short(x,d)		ITEM(	SIGNED,		s16,	x, d,	0,	0 )
#define	Short_(o,x,d)		IteM( o,SIGNED,		s16,	x, d,	0,	0 )

#define	Signed(x,d)		ITEM(	SIGNED,		s_t,	x, d,	0,	0 )
#define	Signed_(o,x,d)		IteM( o,SIGNED,		s_t,	x, d,	0,	0 )

#define	Stamp(x,d)		ITEM(	STAMP,		time_t,	x, d,	F_STMP,	0 )
#define	Stamp_(o,x,d)		IteM( o,STAMP,		time_t,	x, d,	F_STMP,	0 )

#define	Stamp32(x,d)		ITEM(	STAMP,		u32,	x, d,	F_STMP,	0 )
#define	Stamp32_(o,x,d)		IteM( o,STAMP,		u32,	x, d,	F_STMP,	0 )

#define	String(x,d)		ITEM(	STRING,	((STRUCT *)0)->x, x, d,	0,	0 )
#define	String_(o,x,d)		IteM( o,STRING,	((STRUCT *)0)->x, x, d,	0,	0 )

#define	Struct(t,x,l) 		ITEM(	STRUCTURE,	t,	x, #t,	0,	l )
#define	Struct_(o,t,x,l) 	IteM( o,STRUCTURE,	t,	x, #t,	0,	l )

#define	Time(x,d)		ITEM(	STAMP,		time_t,	x, d,	F_TIME,	0 )
#define	Time_(o,x,d)		IteM( o,STAMP,		time_t,	x, d,	F_TIME,	0 )

#define	Time32(x,d)		ITEM(	STAMP,		u32,	x, d,	F_TIME,	0 )
#define	Time32_(o,x,d)		IteM( o,STAMP,		u32,	x, d,	F_TIME,	0 )

#define	Time64(x,d)		ITEM(	STAMP,		u64,	x, d,	F_TIME,	0 )
#define	Time64_(o,x,d)		IteM( o,STAMP,		u64,	x, d,	F_TIME,	0 )

#define	Timer(x,d)		ITEM(	SIGNED,		time_t,	x, d,	0,	0 )
#define	Timer_(o,x,d)		IteM( o,SIGNED,		time_t,	x, d,	0,	0 )

#define	U8(x,d)			ITEM(	UNSIGNED,	u8,	x, d,	0,	0 )
#define	U8_(o,x,d)		IteM( o,UNSIGNED,	u8,	x, d,	0,	0 )

#define	U16(x,d)		ITEM(	UNSIGNED,	u16,	x, d,	0,	0 )
#define	U16_(o,x,d)		IteM( o,UNSIGNED,	u16,	x, d,	0,	0 )

#define	U32(x,d)		ITEM(	UNSIGNED,	u32,	x, d,	0,	0 )
#define	U32_(o,x,d)		IteM( o,UNSIGNED,	u32,	x, d,	0,	0 )

#define	U64(x,d)		ITEM(	UNSIGNED,	u64,	x, d,	0,	0 )
#define	U64_(o,x,d)		IteM( o,UNSIGNED,	u64,	x, d,	0,	0 )

#define	U128(x,d)		ITEM(	UNSIGNED,	u128,	x, d,	0,	0 )
#define	U128_(o,x,d)		IteM( o,UNSIGNED,	u128,	x, d,	0,	0 )

#define	U_t(x,d)		ITEM(	UNSIGNED,	u_t,	x, d,	0,	0 )
#define	U_t_(o,x,d)		IteM( o,UNSIGNED,	u_t,	x, d,	0,	0 )

#define	Uchar(x,d)		ITEM(	UNSIGNED,	u8,	x, d,	0,	0 )
#define	Uchar_(o,x,d)		IteM( o,UNSIGNED,	u8,	x, d,	0,	0 )

#define	Uhuge(x,d)		ITEM(	UNSIGNED,	u64,	x, d,	0,	0 )
#define	Uhuge_(o,x,d)		IteM( o,UNSIGNED,	u64,	x, d,	0,	0 )

#define	Ulong(x,d)		ITEM(	UNSIGNED,	u32,	x, d,	0,	0 )
#define	Ulong_(o,x,d)		IteM( o,UNSIGNED,	u32,	x, d,	0,	0 )

#define	Union(t,x,l) 		ITEM(	STRUCTURE,	t,	x, #t,	0,	l )
#define	Union_(o,t,x,l)		IteM( o,STRUCTURE,	t,	x, #t,	0,	l )

#define	Unsigned(x,d)		ITEM(	UNSIGNED,	u_t,	x, d,	0,	0 )
#define	Unsigned_(o,x,d)	IteM( o,UNSIGNED,	u_t,	x, d,	0,	0 )

#define	Ushort(x,d)		ITEM(	UNSIGNED,	u16,	x, d,	0,	0 )
#define	Ushort_(o,x,d)		IteM( o,UNSIGNED,	u16,	x, d,	0,	0 )

#define	X8(x,d)			ITEM(	HEXADECIMAL,	u8,	x, d,	0,	0 )
#define	X8_(o,x,d)		IteM( o,HEXADECIMAL,	u8,	x, d,	0,	0 )

#define	X16(x,d)		ITEM(	HEXADECIMAL,	u16,	x, d,	0,	0 )
#define	X16_(o,x,d)		IteM( o,HEXADECIMAL,	u16,	x, d,	0,	0 )

#define	X32(x,d)		ITEM(	HEXADECIMAL,	u32,	x, d,	0,	0 )
#define	X32_(o,x,d)		IteM( o,HEXADECIMAL,	u32,	x, d,	0,	0 )

#define	X64(x,d)		ITEM(	HEXADECIMAL,	u64,	x, d,	0,	0 )
#define	X64_(o,x,d)		IteM( o,HEXADECIMAL,	u64,	x, d,	0,	0 )

#define	X128(x,d)		ITEM(	HEXADECIMAL,	u128,	x, d,	0,	0 )
#define	X128_(o,x,d)		IteM( o,HEXADECIMAL,	u128,	x, d,	0,	0 )

#define	X_t(x,d)		ITEM(	HEXADECIMAL,	u_t,	x, d,	0,	0 )
#define	X_t_(o,x,d)		IteM( o,HEXADECIMAL,	u_t,	x, d,	0,	0 )

#define	Xchar(x,d)		ITEM(	HEXADECIMAL,	u8,	x, d,	0,	0 )
#define	Xchar_(o,x,d)		IteM( o,HEXADECIMAL,	u8,	x, d,	0,	0 )

#define	Xhuge(x,d)		ITEM(	HEXADECIMAL,	u64,	x, d,	0,	0 )
#define	Xhuge_(o,x,d)		IteM( o,HEXADECIMAL,	u64,	x, d,	0,	0 )

#define	Xlong(x,d)		ITEM(	HEXADECIMAL,	u32,	x, d,	0,	0 )
#define	Xlong_(o,x,d)		IteM( o,HEXADECIMAL,	u32,	x, d,	0,	0 )

#define	Xshort(x,d)		ITEM(	HEXADECIMAL,	u16,	x, d,	0,	0 )
#define	Xshort_(o,x,d)		IteM( o,HEXADECIMAL,	u16,	x, d,	0,	0 )

/*
 *	mem_core.c	Function & Data Prototypes
 */

extern	char	Item_Prefix[ 512 ];
extern	char	Item_Name[ 512 ];
extern	char	Item_Value[ 512 ];

extern	char	error_frmt[ ];
extern	char	error_func[ ];
extern	char	error_list[ ];
extern	char	error_size[ ];
extern	char	error_type[ ];
extern	char	f_display[ ];
extern	char	f_prefixed[ ];

extern	void	Array_Of_Items(	void *	shared_memory,
				Mitem *	memory_item_list,
				char *	c_variable_name,
				void	(* callback_function)( void *, Mitem *, char * )
);
extern	void	List_Of_Items(	void *	shared_memory,
				Mitem *	memory_item_list,
				char *	c_variable_name,
				void	(* callback_function)( void *, Mitem *, char * )
);

/*
 * Install a field-type extension before using mio (not during concurrent I/O).
 * Input and output have identical, typed callback ABIs; output writes its text to buffer.
 * Both callbacks are required.  Returns 0, or -1/errno=EINVAL for an invalid type/pair.
 * Re-registering the same pair is safe.  Existing built-ins may also be replaced explicitly.
 * MSGADDR has no built-in handler: link miomsg and call MioMsgInit() for message fields.
 * Missing handlers: input leaves data unchanged and reports Func.ERROR on stderr/Item_Value;
 * output displays Func.ERROR; both set errno=ENOTSUP. Existing void APIs remain unchanged.
 */
typedef void (* MioItemHandler)( void * data, Mitem * item, char * text );
extern int MioRegisterHandler( Mtyp_t type, MioItemHandler input, MioItemHandler output );

/*
 *	mem_num.c	Number Conversions (every common.h bit-size type)
 */

extern	void	Mio_Int_Out(	char *	buffer,
				void *	data,
				int	size,		/* 1, 2, 4, 8, 16 bytes */
				int	sgn,		/* signed: always prints + or - */
				int	base		/* 10 or 16 */
);
extern	int	Mio_Int_In(	void *	data,
				int	size,
				int	sgn,
				int	base,
				char *	text
);
extern	void	Mio_Float_Out(	char *	buffer,		/* shortest text that reads back exactly */
				void *	data,
				int	kind		/* 32, 64, 80, 128 */
);
extern	int	Mio_Float_In(	void *	data,
				int	kind,
				char *	text
);

/*
 *	mem_in.c	Function Prototypes
 */

extern	void	Input_Memory_Item(	void *	shared_memory,
					Mitem *	memory_item_list,
					char *	c_variable_name,
					char *	input_value
);

/*
 *	mem_out.c	Function & Data Prototypes
 */

extern	int	CSV_Header;

extern	void	Output_Memory_CSV(	void *	shared_memory,
					Mitem *	memory_item_list,
					char *	c_variable_name,
					char *	output_prefix
);

extern	void	Output_Memory_Item(	void *	shared_memory,
					Mitem *	memory_item_list,
					char *	c_variable_name,
					char *	output_prefix
);

#if	defined( __cplusplus )
}
#endif

#endif

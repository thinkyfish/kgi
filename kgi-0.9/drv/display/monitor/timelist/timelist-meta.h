/* ---------------------------------------------------------------------------
**	timelist monitor driver meta language definition
** ---------------------------------------------------------------------------
**	Copyright (C)	2001	Brian S. Julin
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ---------------------------------------------------------------------------
**	MAINTIANER	Brian_S_Julin
*/
#ifndef	_monitor_timelist_timelist_meta_h
#define	_monitor_timelist_timelist_meta_h

/*
**	timing definitions
*/
#define	TIMELIST_HPOS	0x1000
#define	TIMELIST_HNEG	0x0000
#define	TIMELIST_VPOS	0x2000
#define	TIMELIST_VNEG	0x0000
#define	TIMELIST_POLARITY_MASK	0x3000

#define TIMELIST_HNVN	(TIMELIST_HNEG | TIMELIST_VNEG)
#define TIMELIST_HNVP	(TIMELIST_HNEG | TIMELIST_VPOS)
#define TIMELIST_HPVN	(TIMELIST_HPOS | TIMELIST_VNEG)
#define TIMELIST_HPVP	(TIMELIST_HPOS | TIMELIST_VPOS)

typedef struct
{
	kgi_ascii_t			name[8];

	kgi_u_t				htimings;
	const struct {
		kgi_u_t 		dclk;
		kgim_monitor_timing_t	tm;
	}				*htiming;
	kgi_u_t				vtimings;
	const kgim_monitor_timing_t	*vtiming;

} timelist_timing_t;

typedef struct
{
	kgim_monitor_mode_t	kgim;
	const timelist_timing_t	*timing;

} timelist_monitor_mode_t;

typedef struct
{
} timelist_monitor_io_t;

typedef struct
{
	kgim_monitor_t		monitor;
	const timelist_timing_t	*timing;

} timelist_monitor_t;

KGIM_META_INIT_FN(timelist_monitor)
KGIM_META_MODE_CHECK_FN(timelist_monitor)

#endif	/* #ifndef _monitor_timelist_timelist_meta_h */

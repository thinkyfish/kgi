/* -----------------------------------------------------------------------------
**	PERMEDIA2v PLL meta language definition
** -----------------------------------------------------------------------------
**	Copyright	1999-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**	MAINTAINER	Steffen_Seeger
**
**	$Log: PERMEDIA2v-meta.h,v $
*/
#ifndef _clock_3Dlabs_PERMEDIA2v_meta_h
#define	_clock_3Dlabs_PERMEDIA2v_meta_h

#include "clock/pll-meta.h"

typedef pll_clock_io_t pgc2v_clock_io_t;

#define	PGC2v_DAC_OUT8(ctx, val, reg)	PLL_DAC_OUT8(ctx, val, reg)
#define	PGC2v_DAC_IN8(ctx, reg)		PLL_DAC_IN8(ctx, reg)
#define	PGC2v_CLK_OUT8(ctx, val, reg)	PLL_CLK_OUT8(ctx, val, reg)
#define	PGC2v_CLK_IN8(ctx, val, reg)	PLL_CLK_IN8(ctx, reg)

typedef struct
{
	pll_clock_mode_t	pll;

} pgc2v_clock_mode_t;

typedef struct
{
	pll_clock_t	pll;

	kgi_u8_t	DClk3PreScale;
	kgi_u8_t	DClk3FeedbackScale;
	kgi_u8_t	DClk3PostScale;

} pgc2v_clock_t;

KGIM_META_INIT_FN(pgc2v_clock)
KGIM_META_DONE_FN(pgc2v_clock)
KGIM_META_MODE_ENTER_FN(pgc2v_clock)

#endif	/* #ifndef _clock_3Dlabs_PERMEDIA2v_meta_h	*/

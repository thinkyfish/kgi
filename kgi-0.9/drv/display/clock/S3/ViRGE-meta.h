/* -----------------------------------------------------------------------------
**	S3 ViRGE PLL meta language definition
** -----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Jon Taylor
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**	MAINTAINER	Jon_Taylor
**
**	$Log: ViRGE-meta.h,v $
*/
#ifndef _clock_S3_ViRGE_meta_h
#define	_clock_S3_ViRGE_meta_h

#include "clock/pll-meta.h"

typedef pll_clock_io_t virge_clock_io_t;

#define	VIRGE_DAC_OUT8(ctx, val, reg)	PLL_DAC_OUT8(ctx, val, reg)
#define	VIRGE_DAC_IN8(ctx, reg)		PLL_DAC_IN8(ctx, reg)
#define	VIRGE_CLK_OUT8(ctx, val, reg)	PLL_CLK_OUT8(ctx, val, reg)
#define	VIRGE_CLK_IN8(ctx, val, reg)	PLL_CLK_IN8(ctx, reg)

typedef struct
{
	pll_clock_mode_t	pll_mode;

} virge_clock_mode_t;

typedef struct
{
	pll_clock_t	pll;

/*	kgi_u8_t	PixelClockC1;
**	kgi_u8_t	PixelClockC2;
**	kgi_u8_t	PixelClockC3;
*/
} virge_clock_t;

KGIM_META_INIT_FN(virge_clock)
KGIM_META_DONE_FN(virge_clock)
KGIM_META_MODE_CHECK_FN(virge_clock)
KGIM_META_MODE_RESOURCE_FN(virge_clock)
KGIM_META_MODE_PREPARE_FN(virge_clock)
KGIM_META_MODE_ENTER_FN(virge_clock)
KGIM_META_MODE_LEAVE_FN(virge_clock)

#endif	/* #ifndef _clock_S3_ViRGE_meta_h */

/* ----------------------------------------------------------------------------
**	##VENDOR## ##META## PLL meta definition
** ----------------------------------------------------------------------------
**	Copyright (C)	2000	##AUTHOR_NAME##
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	##AUTHOR##
**
**	$Log: pll-meta.h,v $
**	Revision 1.1.1.1  2000/04/18 08:51:11  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef _clock_##VENDOR##_##META##_meta_h
#define	_clock_##VENDOR##_##META##_meta_h

#include "clock/pll-meta.h"

typedef pll_clock_io_t ##meta##_clock_io_t;

#define	##META##_DAC_OUT8(ctx, val, reg)	PLL_DAC_OUT8(ctx, val, reg)
#define	##META##_DAC_IN8(ctx, reg)		PLL_DAC_IN8(ctx, reg)
#define	##META##_CLK_OUT8(ctx, val, reg)	PLL_CLK_OUT8(ctx, val, reg)
#define	##META##_CLK_IN8(ctx, reg)	PLL_CLK_IN8(ctx, reg)

typedef struct
{
	pll_clock_mode_t	pll;

} ##meta##_clock_mode_t;

typedef struct
{
	pll_clock_t	pll;

#warning add initial (saved) PLL state here.

} ##meta##_clock_t;

KGIM_META_INIT_FN(##meta##_clock)
KGIM_META_DONE_FN(##meta##_clock)
KGIM_META_MODE_ENTER_FN(##meta##_clock)

#endif	/* #ifndef _clock_##VENDOR##_##META##_meta_h	*/

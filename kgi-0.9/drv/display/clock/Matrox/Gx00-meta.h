/* ----------------------------------------------------------------------------
**	Matrox Gx00 clock driver meta
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2001	Johan Karlberg
**					Rodolphe Ortalo
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Rodolphe_Ortalo
**
**	$Log: Gx00-meta.h,v $
**	Revision 1.2  2001/08/31 23:59:14  ortalo
**	Driver nearly operational (without accel) on G400 and Mystique boards.
**	
**	Revision 1.1.1.1  2000/04/18 08:51:14  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/

#ifndef _clock_Matrox_Gx00_meta_h
#define	_clock_Matrox_Gx00_meta_h

#include "clock/pll-meta.h"

#warning your meta language prefix is GX00_ and gx00_.

typedef pll_clock_io_t mgag_clock_io_t;

#define MGAG_PCIDEV(ctx)		KGIM_PCIDEV(ctx)

#define MGAG_DAC_OUT8(ctx, val, reg)	PLL_DAC_OUT8(ctx, val, reg)
#define MGAG_DAC_IN8(ctx, reg)		PLL_DAC_IN8(ctx, reg)
#define MGAG_CLK_OUT8(ctx, val, reg)	PLL_CLK_OUT8(ctx, val, reg)
#define MGAG_CLK_IN8(ctx, val, reg)	PLL_CLK_IN8(ctx, reg)

typedef struct {

	pll_clock_mode_t	pll;

	kgi_u_t			s;	/* Loop filter value */

} mgag_clock_mode_t;

typedef enum {

	/* chipsets */

	MGAG_CF_1x64	= (0x01 << 1),
	MGAG_CF_G200	= (0x01 << 2),
	MGAG_CF_G400	= (0x01 << 3),
	MGAG_CF_G450	= (0x01 << 5),

	/* capabilities */

	MGAG_CF_SGRAM	= (0x01 << 4)

} mgag_clock_flags_t;

typedef struct {

	pll_clock_t		pll;

	mgag_clock_flags_t	flags;

	struct {

		kgi_u8_t	pixclkctrl,
				pixcm,
				pixcn,
				pixcp;
	} saved;

} mgag_clock_t;

KGIM_META_INIT_FN(mgag_clock)
KGIM_META_DONE_FN(mgag_clock)
KGIM_META_MODE_PREPARE_FN(mgag_clock)
KGIM_META_MODE_ENTER_FN(mgag_clock)

#endif	/* #ifndef _clock_Matrox_Gx00_meta_h	*/

/* ----------------------------------------------------------------------------
**	Matrox Gx00 chipset driver meta
** ----------------------------------------------------------------------------
**	Copyright (C) 1999-2000		Johan Karlberg
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: Gx00-meta.c,v $
*/

#include <kgi/maintainers.h>
#define	MAINTAINER		Johan_Karlberg
#define	KGIM_CLOCK_DRIVER	"$Revision: 1.2 $"

#define DEBUG_LEVEL 255

#include <kgi/module.h>

#include "chipset/Matrox/Gx00.h"
#include "clock/Matrox/Gx00-meta.h"

/*
**	IO helpers
*/

static inline void MGAG_EDAC_OUT8(mgag_clock_io_t *mgag_io, kgi_u8_t val,
	kgi_u8_t reg)
{
	MGAG_DAC_OUT8(mgag_io, reg, PALWTADD);
	MGAG_DAC_OUT8(mgag_io, val, X_DATAREG);
}

static inline kgi_u8_t MGAG_EDAC_IN8(mgag_clock_io_t *mgag_io, kgi_u8_t reg)
{
	MGAG_DAC_OUT8(mgag_io, reg, PALWTADD);
	return MGAG_DAC_IN8(mgag_io, X_DATAREG);
}

/*	KGI interface
*/

kgi_error_t mgag_clock_init(mgag_clock_t *mgag,	mgag_clock_io_t *mgag_io,
	const kgim_options_t *options) 
{
	KRN_DEBUG(2, "clock_init() initiated");

	mgag->saved.pixclkctrl	= MGAG_EDAC_IN8(mgag_io, XPIXCLKCTRL);
	mgag->saved.pixcm	= MGAG_EDAC_IN8(mgag_io, XPIXPLLCM);
	mgag->saved.pixcn	= MGAG_EDAC_IN8(mgag_io, XPIXPLLCN);
	mgag->saved.pixcp	= MGAG_EDAC_IN8(mgag_io, XPIXPLLCP);

	KRN_DEBUG(2, "clock_init() completed");

	return KGI_EOK;
}

void mgag_clock_done(mgag_clock_t *mgag, mgag_clock_io_t *mgag_io, 
	const kgim_options_t *options) 
{
	kgi_u_t	cnt = PLL_DELAY;

	KRN_DEBUG(2, "clock_done() initiated");

	MGAG_EDAC_OUT8(mgag_io, (XPIXCLKCTRL_PIXCLKSEL_PIXPLL | 
		XPIXCLKCTRL_PIXCLKDIS | XPIXCLKCTRL_PIXPLLPDN), XPIXCLKCTRL);

	MGAG_EDAC_OUT8(mgag_io, mgag->saved.pixcm, XPIXPLLCM);
	MGAG_EDAC_OUT8(mgag_io, mgag->saved.pixcn, XPIXPLLCN);
	MGAG_EDAC_OUT8(mgag_io,	mgag->saved.pixcp, XPIXPLLCP);

	while (cnt-- &&	
		(MGAG_EDAC_IN8(mgag_io, XPIXPLLSTAT) & XPIXPLLSTAT_PIXLOCK));

	KRN_ASSERT(cnt);

	MGAG_EDAC_OUT8(mgag_io, mgag->saved.pixclkctrl, XPIXCLKCTRL);

	KRN_DEBUG(2, "clock_done() completed");
}


void mgag_clock_mode_prepare(mgag_clock_t *mgag, mgag_clock_io_t *mgag_io,
	mgag_clock_mode_t *mgag_mode, kgi_image_mode_t *img, kgi_u_t images) 
{
	KRN_DEBUG(2, "clock_mode_prepare() initiated");

	/*	Get loop filter value (s) - 1x64 and G400 uses the same values
	*/

	if (mgag->flags & MGAG_CF_G400) {

		if ((mgag_mode->pll.fvco > 50 MHZ) && 
			(mgag_mode->pll.fvco < 110 MHZ)) {

			mgag_mode->s = 0;

		} else if (mgag_mode->pll.fvco < 170 MHZ) {

			mgag_mode->s = 1;

		} else if (mgag_mode->pll.fvco < 240 MHZ) {

			mgag_mode->s = 2;

		} else if (mgag_mode->pll.fvco < mgag->pll.fvco.max) {

			mgag_mode->s = 3;

		} else {

			KRN_INTERNAL_ERROR;
		}

	} else {

		if ((mgag_mode->pll.fvco > 50 MHZ) && 
			(mgag_mode->pll.fvco < 100 MHZ)) {

			mgag_mode->s = 0;

		} else if (mgag_mode->pll.fvco < 140 MHZ) {

			mgag_mode->s = 1;

		} else if (mgag_mode->pll.fvco < 180 MHZ) {

			mgag_mode->s = 2;

		} else if (mgag_mode->pll.fvco < mgag->pll.fvco.max) {

			mgag_mode->s = 3;

		} else {

			KRN_INTERNAL_ERROR;
		}
	}

	KRN_DEBUG(2, "clock_mode_prepare() completed");
}

void mgag_clock_mode_enter(mgag_clock_t *mgag, mgag_clock_io_t *mgag_io, 
	mgag_clock_mode_t *mgag_mode, kgi_image_mode_t *img, kgi_u_t images) 
{
	kgi_u_t	cnt = PLL_DELAY;

	KRN_DEBUG(2, "clock_mode_enter() initiated");

	/*	display must be disabled by chipset driver
	*/
	MGAG_EDAC_OUT8(mgag_io, (XPIXCLKCTRL_PIXCLKSEL_PIXPLL | 
		XPIXCLKCTRL_PIXCLKDIS | XPIXCLKCTRL_PIXPLLPDN), XPIXCLKCTRL);

	/*	setting pll values
	*/
	MGAG_EDAC_OUT8(mgag_io,  (mgag_mode->pll.mul & XPIXPLLM_MASK), XPIXPLLCM);
	MGAG_EDAC_OUT8(mgag_io,  (mgag_mode->pll.div & XPIXPLLN_MASK), XPIXPLLCN);
	MGAG_EDAC_OUT8(mgag_io,	((mgag_mode->pll.p   & XPIXPLLP_PIXPLLP_MASK) | (mgag_mode->s << XPIXPLLP_PIXPLLS_SHIFT)) & XPIXPLLP_MASK, XPIXPLLCP);

	while (cnt-- &&
		(MGAG_EDAC_IN8(mgag_io, XPIXPLLSTAT) & XPIXPLLSTAT_PIXLOCK));

	KRN_ASSERT(cnt);

	MGAG_EDAC_OUT8(mgag_io, (XPIXCLKCTRL_PIXCLKSEL_PIXPLL | 
		XPIXCLKCTRL_PIXPLLPDN), XPIXCLKCTRL);

	/*	display must be reenabled by chipset driver
	*/
	KRN_DEBUG(2, "clock_mode_enter() completed");
}

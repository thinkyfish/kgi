/* ----------------------------------------------------------------------------
**	Generic fixed clock routines
** ----------------------------------------------------------------------------
**	Copyright (C)	1995-2000	Steffen Seeger
**	Copyright (C)	2000		Jon Taylor
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: fixed-meta.c,v $
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Jon_Taylor
#define	KGIM_CLOCK_DRIVER	"$Revision: 1.2 $"

#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	1
#endif

#include "clock/fixed/fixed-meta.h"

kgi_error_t fixed_clock_init(fixed_clock_t *fixed, fixed_clock_io_t *fixed_io,
	const kgim_options_t *options)
{
	fixed->clk_index = FIXED_CLK_IN8(fixed_io, 0);
	KRN_DEBUG(2, "saved initial clock index %i", fixed->clk_index);
}

kgi_error_t fixed_clock_done(fixed_clock_t *fixed, fixed_clock_io_t *fixed_io,
	const kgim_options_t *options)
{
	KRN_DEBUG(2, "restore initial clock index %i", fixed->clk_index);
	FIXED_CLK_OUT8(fixed_io, fixed->clk_index, 0);
}

#define FIXED_INVALID_CLK -1

kgi_error_t fixed_clock_mode_check(fixed_clock_t *fixed, 
	fixed_clock_io_t *fixed_io, fixed_clock_mode_t *fixed_mode,
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_dot_port_mode_t *dpm = img->out;
	kgi_s_t newfreq;
	kgi_s_t best = 0;
	kgi_s_t delta = fixed->clock.dclk.freq[best] - newfreq;
	int i;

	KRN_DEBUG(2, "fixed_clock_mode_check()");
	
	if (cmd == KGI_TC_PROPOSE) {

		/* Propose the highest possible DCLK */
		newfreq = 0x7fffffff;

	} else {

		KRN_ASSERT(dpm->rclk.mul && dpm->rclk.div);

		newfreq = dpm->dclk * dpm->rclk.mul / dpm->rclk.div;
	}

	KRN_DEBUG(2, "newfreq = %i", newfreq);
	KRN_DEBUG(2, "dpm->dclk = %i", dpm->dclk);
	KRN_DEBUG(2, "dpm->lclk = {%i,%i}", dpm->lclk.mul, dpm->lclk.div);
	KRN_DEBUG(2, "dpm->rclk = {%i,%i}", dpm->rclk.mul, dpm->rclk.div);
	KRN_DEBUG(2, "delta = %i", delta);
	KRN_DEBUG(2, "fixed->clock.dclk.freq[0] = "
		"fixed->clock.dclk.freq[best] = %i", 
		fixed->clock.dclk.freq[best]);
	
	KRN_ASSERT(images == 1);
	
	for (i = 0; (i < __KGIM_MAX_NR_FIXED_CLOCKS) &&
		fixed->clock.dclk.freq[i]; i++)  {

		KRN_DEBUG(2, "Checking clock %i, freq[i] = %i",
			i, fixed->clock.dclk.freq[i])
		  
		if (KGI_ABS(fixed->clock.dclk.freq[i] - newfreq) <
			KGI_ABS(delta)) {

			best  = i;
			delta = fixed->clock.dclk.freq[best] - newfreq;
		}
		
		KRN_DEBUG(2, "best = %i, delta = %i", best, delta);
	}

	KRN_DEBUG(2, "best == %i, %i Hz, delta = %i", best, 
		fixed->clock.dclk.freq[best], delta);

	switch (cmd) {

	case KGI_TC_PROPOSE:

		KRN_DEBUG(2, "KGI_TC_PROPOSE:");

		dpm->dclk = fixed->clock.dclk.freq[best] * dpm->rclk.div /
				dpm->rclk.mul;

		return KGI_EOK;

	case KGI_TC_LOWER:

		KRN_DEBUG(2, "KGI_TC_LOWER:");

		if ((delta > 0) && (KGI_ABS(delta) >= (2 * newfreq / 100))) {

			best++;

			if ((best == __KGIM_MAX_NR_FIXED_CLOCKS) ||
				!fixed->clock.dclk.freq[best]) {

				KRN_DEBUG(2, "Unable to lower clock");
				return -E(CLOCK, UNKNOWN);
			}
		}

		dpm->dclk = fixed->clock.dclk.freq[best] * dpm->rclk.div /
				dpm->rclk.mul;
		KRN_DEBUG(2, "Request for rclk %i Hz, did %i Hz", newfreq, 
			fixed->clock.dclk.freq[best]);
		return KGI_EOK;

	case KGI_TC_RAISE:

		KRN_DEBUG(2, "KGI_TC_RAISE:");

		if ((delta < 0) && (KGI_ABS(delta) >= (2 * newfreq / 100))) {

			best--;
			
#warning	FIXME
			if (best < 0) {

				KRN_DEBUG(1, "Could not find a new best freq");
				return -E(CLOCK, UNKNOWN);
			}
		}

		KRN_DEBUG(2, "best freq = %i", best);

		dpm->dclk = fixed->clock.dclk.freq[best] * dpm->rclk.div /
				dpm->rclk.mul;
		KRN_DEBUG(2, "Request for rclk %i Hz, did %i Hz", 
			newfreq, fixed->clock.dclk.freq[best]);
		return KGI_EOK;

	case KGI_TC_CHECK:

		KRN_DEBUG(2, "KGI_TC_CHECK:");

		if ((KGI_ABS(delta) < (newfreq / 100)) &&
			fixed->clock.dclk.freq[best]) {

			KRN_DEBUG(2, "passed, index %i, rclk %i Hz, reg0 %i", 
				best, fixed->clock.dclk.freq[best],
				fixed->reg0[best]);

			fixed_mode->reg0 = fixed->reg0[best];
			dpm->dclk = fixed->clock.dclk.freq[best] * 
					dpm->rclk.div / dpm->rclk.mul;

			return KGI_EOK;

		} else {

			KRN_DEBUG(1, "Invalid clock request");
			
			fixed_mode->reg0 = 0;
			dpm->dclk = FIXED_INVALID_CLK;
			
			return -E(CLOCK, UNKNOWN);
		}

	default:
		/*	This should not happen ...
		*/
		KRN_INTERNAL_ERROR;
		return -E(CLOCK, UNKNOWN);
	}
}

void fixed_clock_mode_enter(fixed_clock_t *fixed, fixed_clock_io_t *fixed_io,
	fixed_clock_mode_t *fixed_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	KRN_DEBUG(2, "setting reg0 = 0x%.2x", fixed_mode->reg0);
	FIXED_CLK_OUT8(fixed_io, fixed_mode->reg0, 0);
}

/* ----------------------------------------------------------------------------
**	generic monosync monitor meta language implementation
** ----------------------------------------------------------------------------
**	Copyright (C)	1995-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: monosync-meta.c,v $
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		Steffen_Seeger
#define	KGIM_MONITOR_DRIVER	"$Revision: 1.5 $"

#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	1
#endif

#include "monitor/monosync/monosync-meta.h"

kgi_error_t monosync_monitor_init(monosync_monitor_t *monosync, 
	monosync_monitor_io_t *monosync_io, const kgim_options_t *options)
{
	KRN_NOTICE("%s %s [monosync %s] driver " KGIM_MONITOR_DRIVER,
		monosync->monitor.vendor, monosync->monitor.model,
		monosync->timing->name);

	return KGI_EOK;
}

static kgi_s_t monosync_monitor_err(kgi_s_t pixels, kgi_s_t ns, kgi_u_t dclk)
{
	kgi_s_t width = (ns * (dclk / 10000)) / 100000;
	kgi_s_t delta = (100*(pixels - width)) / width;
	kgi_s_t ret = (delta < -2) || (2 < delta);

	if (ret) {

		KRN_DEBUG(0, "Timing difference %i > 2% (%i pixel %i ns %i dclk)",
			delta, pixels, ns, dclk);
	}

	return ret;
}

static inline kgi_error_t monosync_monitor_fcheck(monosync_monitor_t *monosync, 
	kgi_u_t dclk, kgi_u_t xtotal, kgi_u_t ytotal)
{
	kgi_u_t flags = 0, i;
	kgi_u_t hfreq = dclk / xtotal;
	kgi_u_t vfreq = dclk / (xtotal * ytotal);

	i = KGIM_MONITOR_MAX_HFREQ;
	while (i--) {

		if ((monosync->monitor.hfreq[i].min <= hfreq) &&
			(hfreq <= monosync->monitor.hfreq[i].max)) {

			flags |= 1;
		}
	}

	i = KGIM_MONITOR_MAX_VFREQ;
	while (i--) {

		if ((monosync->monitor.vfreq[i].min <= vfreq) &&
			(vfreq <= monosync->monitor.vfreq[i].max)) {

			flags |= 2;
		}
	}
	return (flags == 3) ? KGI_EOK : -E(MONITOR,INVAL);
}

kgi_error_t monosync_monitor_mode_check(monosync_monitor_t *monosync,
	monosync_monitor_io_t *monosync_io,
	monosync_monitor_mode_t *monosync_mode,
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	const monosync_timing_t *t = monosync->timing;
	kgi_u_t v = 0, h = 0;
	kgi_u_t dclk, ddclk;
	kgi_u_t dotsx = monosync_mode->kgim.in.dots.x;
	kgi_u_t dotsy = monosync_mode->kgim.in.dots.y;

	if ((dotsx > monosync->monitor.maxdots.x) || 
		(dotsy > monosync->monitor.maxdots.y)) {

		KRN_DEBUG(2, "Resolution %ix%i higher than limit %ix%i",
			dotsx, dotsy, monosync->monitor.maxdots.x, 
			monosync->monitor.maxdots.y);
		return -E(MONITOR, UNKNOWN);
	}

	while ((v < t->vtimings) && (t->vtiming[v].width != dotsy)) {

		v++;
	}
	if ((v == t->vtimings) || (t->vtiming[v].width != dotsy)) {

		KRN_DEBUG(2, "No vtiming for %i lines in timing set %s.",
			dotsy, t->name);
		return -E(MONITOR, UNKNOWN);
	}

	h = t->vtiming[v].polarity & ~MONOSYNC_POLARITY_MASK;
	dclk = ((dotsx * 100000) / t->htiming[h].width) * 10000;
	ddclk = (monosync_mode->kgim.in.dclk > dclk)
		? (monosync_mode->kgim.in.dclk - dclk)
		: (dclk - monosync_mode->kgim.in.dclk);

	switch (cmd) {

	case KGI_TC_PROPOSE:
		monosync_mode->kgim.in.dclk	= dclk;
		monosync_mode->kgim.y		= t->vtiming[v];
		monosync_mode->kgim.y.polarity	= 
			(t->vtiming[v].polarity & MONOSYNC_VPOS) ? 1 : 0;

#		define POS(X)  ((dotsx * t->htiming[h].X) / t->htiming[h].width)
		monosync_mode->kgim.x.width      = POS(width);
		monosync_mode->kgim.x.blankstart = POS(blankstart);
		monosync_mode->kgim.x.syncstart  = POS(syncstart);
		monosync_mode->kgim.x.syncend    = POS(syncend);
		monosync_mode->kgim.x.blankend   = POS(blankend);
		monosync_mode->kgim.x.total      = POS(total);
		monosync_mode->kgim.x.polarity   = 
			(t->vtiming[v].polarity & MONOSYNC_HPOS) ? 1 : 0;
#		undef POS

		monosync_mode->kgim.sync = monosync->monitor.sync;
		  
		if (monosync->monitor.flags & KGIM_MF_PROPSIZE) {

			monosync_mode->kgim.size.x = dotsx *
				monosync->monitor.size.x /
				monosync->monitor.maxdots.x;
			monosync_mode->kgim.size.y = dotsy *
				monosync->monitor.size.y /
				monosync->monitor.maxdots.y;
		} else {

			monosync_mode->kgim.size = monosync->monitor.size;
		}

		KRN_DEBUG(2,
			"propose dclk = %i Hz, hfreq = %i Hz, vfreq = %i Hz",
			monosync_mode->kgim.in.dclk,
			monosync_mode->kgim.in.dclk /
				monosync_mode->kgim.x.total,
			monosync_mode->kgim.in.dclk /
				(monosync_mode->kgim.x.total * 
					monosync_mode->kgim.y.total));

		return KGI_TC_LOWER;

	case KGI_TC_LOWER:
		if (monosync_mode->kgim.in.dclk < monosync->monitor.dclk.min) {

			return -E(MONITOR, UNKNOWN);
		}

		if ((100 * ddclk)  >  (2 * dclk)) {

			KRN_DEBUG(2, "dclk is %i (should be %li) Hz",
				 monosync_mode->kgim.in.dclk, dclk);
			return -E(MONITOR, UNKNOWN);
		}

		return KGI_TC_CHECK;

	case KGI_TC_RAISE:
		if (monosync_mode->kgim.in.dclk > monosync->monitor.dclk.max) {

			return -E(MONITOR, UNKNOWN);
		}

		if ((100 * ddclk)  >  (2 * dclk)) {

			KRN_DEBUG(2, "dclk is %i (should be %li) Hz",
				monosync_mode->kgim.in.dclk, dclk);
			return -E(MONITOR, UNKNOWN);
		}

		return KGI_TC_CHECK;

	case KGI_TC_CHECK:
		if ((monosync_mode->kgim.in.dclk < monosync->monitor.dclk.min) || 
			(monosync_mode->kgim.in.dclk > monosync->monitor.dclk.max)) {

			KRN_DEBUG(2, "DCLK of %i Hz is out of bounds.",
				monosync_mode->kgim.in.dclk);
			return -E(MONITOR, UNKNOWN);
		}

		if ((monosync->monitor.maxdots.x < dotsx) ||
			(monosync->monitor.maxdots.y < dotsy)) {

			KRN_DEBUG(2, "resolution too high (%ix%i).",
				dotsx, dotsy);
			return -E(MONITOR, UNKNOWN);
		}

		if (((100 * ddclk) > (2 * dclk)) || 
			monosync_monitor_fcheck(monosync,
				monosync_mode->kgim.in.dclk,
				monosync_mode->kgim.x.total,
				monosync_mode->kgim.y.total)) {

			KRN_DEBUG(2, "frequency limits violated.");
			return -E(MONITOR, UNKNOWN);
		}

		KRN_DEBUG(2, "%i\t%i %i %i %i %i %i\t%i %i %i %i %i %i",
			monosync_mode->kgim.in.dclk,
			monosync_mode->kgim.x.width,
			monosync_mode->kgim.x.blankstart,
			monosync_mode->kgim.x.syncstart,
			monosync_mode->kgim.x.syncend,
			monosync_mode->kgim.x.blankend,
			monosync_mode->kgim.x.total,
			monosync_mode->kgim.y.width,
			monosync_mode->kgim.y.blankstart,
			monosync_mode->kgim.y.syncstart,
			monosync_mode->kgim.y.syncend,
			monosync_mode->kgim.y.blankend,
			monosync_mode->kgim.y.total);

#		define ERR(X) ((monosync_mode->kgim.y.X != t->vtiming[v].X) || \
			monosync_monitor_err(monosync_mode->kgim.x.X,	\
			t->htiming[h].X, monosync_mode->kgim.in.dclk))

			if (ERR(width) || ERR(blankstart) || ERR(syncstart) ||
				ERR(syncend) || ERR(blankend) || ERR(total)) {

				KRN_DEBUG(2, "Error in timings.");
				return -E(MONITOR, UNKNOWN);
			}

#		undef	ERR

		/*	now that everything is checked, initialize the
		**	driver dependent mode.
		*/
		monosync_mode->timing = t;

		return KGI_TC_READY;

	default:
		KRN_INTERNAL_ERROR;
		return -E(MONITOR, UNKNOWN);
	}
}

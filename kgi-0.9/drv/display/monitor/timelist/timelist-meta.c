/* ----------------------------------------------------------------------------
**	generic timelist monitor meta language implementation
** ----------------------------------------------------------------------------
**	Copyright (C)	2001		Brian S. Julin
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		Brian_S_Julin
#define	KGIM_MONITOR_DRIVER	"$Revision: 1.2 $"

#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	1
#endif

#include "monitor/timelist/timelist-meta.h"

kgi_error_t timelist_monitor_init(timelist_monitor_t *timelist, 
	timelist_monitor_io_t *timelist_io, const kgim_options_t *options)
{
	KRN_NOTICE("%s %s [timelist %s] driver " KGIM_MONITOR_DRIVER,
		timelist->monitor.vendor, timelist->monitor.model,
		timelist->timing->name);

	return KGI_EOK;
}

static kgi_s_t timelist_monitor_err(kgi_s_t pixels, kgi_s_t width, kgi_u_t dclk)
{
	kgi_s_t delta = (100*(pixels - width)) / width;
	kgi_s_t ret = (delta < -2) || (2 < delta);

	if (ret) {

		KRN_DEBUG(0, "Timing difference %i > 2% (%i pixel %i ns %i dclk)",
			delta, pixels, ns, dclk);
	}

	return ret;
}

static inline kgi_error_t timelist_monitor_fcheck(timelist_monitor_t *timelist, 
	kgi_u_t dclk, kgi_u_t xtotal, kgi_u_t ytotal)
{
	kgi_u_t flags = 0, i;
	kgi_u_t hfreq = dclk / xtotal;
	kgi_u_t vfreq = dclk / (xtotal * ytotal);

	i = KGIM_MONITOR_MAX_HFREQ;
	while (i--) {

		if ((timelist->monitor.hfreq[i].min <= hfreq) &&
			(hfreq <= timelist->monitor.hfreq[i].max)) {

			flags |= 1;
		}
	}

	i = KGIM_MONITOR_MAX_VFREQ;
	while (i--) {

		if ((timelist->monitor.vfreq[i].min <= vfreq) &&
			(vfreq <= timelist->monitor.vfreq[i].max)) {

			flags |= 2;
		}
	}
	return (flags == 3) ? KGI_EOK : -KGI_ERRNO(MONITOR,INVAL);
}


kgi_error_t timelist_monitor_mode_check(timelist_monitor_t *timelist,
	timelist_monitor_io_t *timelist_io,
	timelist_monitor_mode_t *timelist_mode,
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	const timelist_timing_t *t = timelist->timing;
	kgi_u_t v = 0, h = 0;
	kgi_u_t dclk, ddclk;
	kgi_u_t dotsx = timelist_mode->kgim.in.dots.x;
	kgi_u_t dotsy = timelist_mode->kgim.in.dots.y;

	if ((dotsx > timelist->monitor.maxdots.x) || 
		(dotsy > timelist->monitor.maxdots.y)) {

		KRN_DEBUG(2, "Resolution %ix%i higher than limit %ix%i",
			dotsx, dotsy, timelist->monitor.maxdots.x, 
			timelist->monitor.maxdots.y);
		return -KGI_ERRNO(MONITOR, UNKNOWN);
	}

	while ((v < t->vtimings) && (t->vtiming[v].width != dotsy)) {

		v++;
	}
	if ((v == t->vtimings) || (t->vtiming[v].width != dotsy)) {

		KRN_DEBUG(2, "No vtiming for %i lines in timelist %s.",
			dotsy, t->name);
		return -KGI_ERRNO(MONITOR, UNKNOWN);
	}

	h = t->vtiming[v].polarity & ~TIMELIST_POLARITY_MASK;
	dclk = t->htiming[h].dclk;
	ddclk = (timelist_mode->kgim.in.dclk > dclk)
		? (timelist_mode->kgim.in.dclk - dclk)
		: (dclk - timelist_mode->kgim.in.dclk);

	switch (cmd) {

	case KGI_TC_PROPOSE:
		timelist_mode->kgim.in.dclk	= dclk;
		timelist_mode->kgim.y		= t->vtiming[v];
		timelist_mode->kgim.y.polarity	= 
			(t->vtiming[v].polarity & TIMELIST_VPOS) ? 1 : 0;

#		define POS(X)  (t->htiming[h].tm.X)
		timelist_mode->kgim.x.width      = POS(width);
		timelist_mode->kgim.x.blankstart = POS(blankstart);
		timelist_mode->kgim.x.syncstart  = POS(syncstart);
		timelist_mode->kgim.x.syncend    = POS(syncend);
		timelist_mode->kgim.x.blankend   = POS(blankend);
		timelist_mode->kgim.x.total      = POS(total);
		timelist_mode->kgim.x.polarity   = 
			(t->vtiming[v].polarity & TIMELIST_HPOS) ? 1 : 0;
#		undef POS

		timelist_mode->kgim.sync = timelist->monitor.sync;
		  
		if (timelist->monitor.flags & KGIM_MF_PROPSIZE) {

			timelist_mode->kgim.size.x = dotsx *
				timelist->monitor.size.x /
				timelist->monitor.maxdots.x;
			timelist_mode->kgim.size.y = dotsy *
				timelist->monitor.size.y /
				timelist->monitor.maxdots.y;
		} else {

			timelist_mode->kgim.size = timelist->monitor.size;
		}

		KRN_DEBUG(2,
			"propose dclk = %i Hz, hfreq = %i Hz, vfreq = %i Hz",
			timelist_mode->kgim.in.dclk,
			timelist_mode->kgim.in.dclk /
				timelist_mode->kgim.x.total,
			timelist_mode->kgim.in.dclk /
				(timelist_mode->kgim.x.total * 
					timelist_mode->kgim.y.total));

		return KGI_TC_LOWER;

	case KGI_TC_LOWER:
		if (timelist_mode->kgim.in.dclk < timelist->monitor.dclk.min) {

			return -KGI_ERRNO(MONITOR, UNKNOWN);
		}

		if ((100 * ddclk)  >  (2 * dclk)) {

			KRN_DEBUG(2, "dclk is %i (should be %li) Hz",
				 timelist_mode->kgim.in.dclk, dclk);
			return -KGI_ERRNO(MONITOR, UNKNOWN);
		}

		return KGI_TC_CHECK;

	case KGI_TC_RAISE:
		if (timelist_mode->kgim.in.dclk > timelist->monitor.dclk.max) {

			return -KGI_ERRNO(MONITOR, UNKNOWN);
		}

		if ((100 * ddclk)  >  (2 * dclk)) {

			KRN_DEBUG(2, "dclk is %i (should be %li) Hz",
				timelist_mode->kgim.in.dclk, dclk);
			return -KGI_ERRNO(MONITOR, UNKNOWN);
		}

		return KGI_TC_CHECK;

	case KGI_TC_CHECK:
		if ((timelist_mode->kgim.in.dclk < timelist->monitor.dclk.min) || 
			(timelist_mode->kgim.in.dclk > timelist->monitor.dclk.max)) {

			KRN_DEBUG(2, "DCLK of %i Hz is out of bounds.",
				timelist_mode->kgim.in.dclk);
			return -KGI_ERRNO(MONITOR, UNKNOWN);
		}

		if ((timelist->monitor.maxdots.x < dotsx) ||
			(timelist->monitor.maxdots.y < dotsy)) {

			KRN_DEBUG(2, "resolution too high (%ix%i).",
				dotsx, dotsy);
			return -KGI_ERRNO(MONITOR, UNKNOWN);
		}

		if (((100 * ddclk) > (2 * dclk)) || 
			timelist_monitor_fcheck(timelist,
				timelist_mode->kgim.in.dclk,
				timelist_mode->kgim.x.total,
				timelist_mode->kgim.y.total)) {

			KRN_DEBUG(2, "frequency limits violated.");
			return -KGI_ERRNO(MONITOR, UNKNOWN);
		}

		KRN_DEBUG(2, "%i\t%i %i %i %i %i %i\t%i %i %i %i %i %i",
			timelist_mode->kgim.in.dclk,
			timelist_mode->kgim.x.width,
			timelist_mode->kgim.x.blankstart,
			timelist_mode->kgim.x.syncstart,
			timelist_mode->kgim.x.syncend,
			timelist_mode->kgim.x.blankend,
			timelist_mode->kgim.x.total,
			timelist_mode->kgim.y.width,
			timelist_mode->kgim.y.blankstart,
			timelist_mode->kgim.y.syncstart,
			timelist_mode->kgim.y.syncend,
			timelist_mode->kgim.y.blankend,
			timelist_mode->kgim.y.total);

#		define ERR(X) ((timelist_mode->kgim.y.X != t->vtiming[v].X) || \
			timelist_monitor_err(timelist_mode->kgim.x.X,	\
			t->htiming[h].tm.X, timelist_mode->kgim.in.dclk))

			if (ERR(width) || ERR(blankstart) || ERR(syncstart) ||
				ERR(syncend) || ERR(blankend) || ERR(total)) {

				KRN_DEBUG(2, "Error in timings.");
				return -KGI_ERRNO(MONITOR, UNKNOWN);
			}

#		undef	ERR

		/*	now that everything is checked, initialize the
		**	driver dependent mode.
		*/
		timelist_mode->timing = t;

		return KGI_TC_READY;

	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(MONITOR, UNKNOWN);
	}
}


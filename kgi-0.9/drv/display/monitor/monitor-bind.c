/* ----------------------------------------------------------------------------
**	monitor meta language binding
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: monitor-bind.c,v $
*/
#include <kgi/module.h>

/*
**	monitor spec conversion macros - may be considered "advanced Voodoo"
*/
#define	Data	1
#define	Monitor(vendor,model,meta)					\
kgi_error_t monitor_init_module_##meta(kgim_monitor_t *monitor,		\
	const kgim_options_t *options)					\
{									\
	KRN_DEBUG(2, "monitor_init_module_##meta()");
#define	Begin		monitor->revision = KGIM_MONITOR_REVISION;
#define	Contributor(c)
#define	Vendor(v)	kgim_strcpy(monitor->vendor, v);
#define	Model(m)	kgim_strcpy(monitor->model, m);
#define	Flags(f)	monitor->flags = f;
#define	MaxRes(x,y)	monitor->maxdots = (kgi_ucoord_t) { x, y };
#define	Size(x,y)	monitor->size = (kgi_ucoord_t) { x, y };
#define	Type(t)		monitor->type = t;
#define	Sync(s)		monitor->sync = s;
#define	Bandwidth(l,h)	monitor->dclk = (kgi_urange_t) { l, h };
#define	hFreq(n,l,h)	monitor->hfreq[n] = (kgi_urange_t) { l, h };
#define	vFreq(n,l,h)	monitor->vfreq[n] = (kgi_urange_t) { l, h };
#define	End		return KGI_EOK; \
}

#ifndef MONITOR_SPEC
#	define	MONITOR_SPEC	"monitor-bind.spec"
#endif
#include MONITOR_SPEC

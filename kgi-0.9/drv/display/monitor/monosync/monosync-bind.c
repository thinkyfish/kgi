/* ----------------------------------------------------------------------------
**	monosync monitor meta language binding
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: monosync-bind.c,v $
*/
#include <kgi/module.h>
#include "monitor/monosync/monosync-meta.h"

extern kgi_error_t monitor_init_module_Standard_VGA(kgim_monitor_t *monitor,
	const kgim_options_t *options);

extern const monosync_timing_t monosync_monitor_timing;
 
kgi_error_t monosync_monitor_init_module(monosync_monitor_t *monosync,
	const kgim_options_t *options)
{
	KRN_DEBUG(2, "monosync_monitor_init_module()");
	
	monosync->timing = &monosync_monitor_timing;

	return monitor_init_module_Standard_VGA(&monosync->monitor, options);
}

const kgim_meta_t monosync_monitor_meta =
{
	(kgim_meta_init_module_fn *)	monosync_monitor_init_module,
	(kgim_meta_done_module_fn *)	NULL,
	(kgim_meta_init_fn *)		monosync_monitor_init,
	(kgim_meta_done_fn *)		NULL,
	(kgim_meta_mode_check_fn *)	monosync_monitor_mode_check,
	(kgim_meta_mode_resource_fn *)	NULL,
	(kgim_meta_mode_prepare_fn *)	NULL,
	(kgim_meta_mode_enter_fn *)	NULL,
	(kgim_meta_mode_leave_fn *)	NULL,

	sizeof(monosync_monitor_t),
	0,
	sizeof(monosync_monitor_mode_t)
};

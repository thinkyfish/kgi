/* ----------------------------------------------------------------------------
**	timelist monitor meta language binding
** ----------------------------------------------------------------------------
**	Copyright (C)	2001	Brian S. Julin
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	
*/
#include <kgi/module.h>
#include "monitor/timelist/timelist-meta.h"

extern kgi_error_t monitor_init_module_Standard_SVGA(kgim_monitor_t *monitor,
	const kgim_options_t *options);

extern const timelist_timing_t timelist_monitor_timing;
 
kgi_error_t timelist_monitor_init_module(timelist_monitor_t *timelist,
	const kgim_options_t *options)
{
	KRN_DEBUG(2, "timelist_monitor_init_module()");
	
	timelist->timing = &timelist_monitor_timing;

	return monitor_init_module_Standard_SVGA(&timelist->monitor, options);
}

const kgim_meta_t timelist_monitor_meta =
{
	(kgim_meta_init_module_fn *)	timelist_monitor_init_module,
	(kgim_meta_done_module_fn *)	NULL,
	(kgim_meta_init_fn *)		timelist_monitor_init,
	(kgim_meta_done_fn *)		NULL,
	(kgim_meta_mode_check_fn *)	timelist_monitor_mode_check,
	(kgim_meta_mode_resource_fn *)	NULL,
	(kgim_meta_mode_prepare_fn *)	NULL,
	(kgim_meta_mode_enter_fn *)	NULL,
	(kgim_meta_mode_leave_fn *)	NULL,
	(kgim_meta_image_resource_fn *)	NULL,

	sizeof(timelist_monitor_t),
	0,
	sizeof(timelist_monitor_mode_t)
};

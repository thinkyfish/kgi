/* ----------------------------------------------------------------------------
**	VGA integrated DAC meta language binding
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Jon Taylor
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: VGA-bind.c,v $
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		Jon_Taylor
#define	KGIM_RAMDAC_DRIVER	"$Revision: 1.2 $"

#define	DEBUG_LEVEL	2

#include <kgi/module.h>
#define	__IBM_VGA
#include "chipset/IBM/VGA.h"
#include "ramdac/IBM/VGA.h"
#include "ramdac/IBM/VGA-meta.h"

kgi_error_t vga_ramdac_init_module(vga_ramdac_t *vga, vga_ramdac_io_t *vga_io,
	const kgim_options_t *options)
{
	kgi_u16_t vendor_id, device_id;

	KRN_DEBUG(2, "vga_ramdac_init_module()");
	
	KRN_ASSERT(vga);
	KRN_ASSERT(vga_io);
	KRN_ASSERT(options);

	vga->ramdac.revision = KGIM_RAMDAC_REVISION;
	
	kgim_strcpy(vga->ramdac.vendor, "IBM");
	kgim_strcpy(vga->ramdac.model, "VGA DAC");
	
/*	vga->ramdac.flags	*/
	vga->ramdac.mode_size	= sizeof(vga_ramdac_mode_t);
/*	vga->ramdac.Command	*/

	vga->ramdac.maxdots.x	= 0x03FF;
	vga->ramdac.maxdots.y	= 0x03FF;
	
	vga->ramdac.lclk.min = options->ramdac->lclk_min 
		? options->ramdac->lclk_min : 12000000;
	vga->ramdac.lclk.max = options->ramdac->lclk_max 
		? options->ramdac->lclk_max : 32000000;
	vga->ramdac.dclk.min = options->ramdac->dclk_min 
		? options->ramdac->dclk_min : 25000000;
	vga->ramdac.dclk.max = options->ramdac->dclk_max 
		? options->ramdac->dclk_max : 32000000;

	KRN_NOTICE("%s %s driver " KGIM_RAMDAC_DRIVER, 
		vga->ramdac.vendor, vga->ramdac.model);

	return KGI_EOK;
}

const kgim_meta_t vga_ramdac_meta =
{
	(kgim_meta_init_module_fn *)	vga_ramdac_init_module,
	(kgim_meta_done_module_fn *)	NULL,
	(kgim_meta_init_fn *)		vga_ramdac_init,
	(kgim_meta_done_fn *)		vga_ramdac_done,
	(kgim_meta_mode_check_fn *)	vga_ramdac_mode_check,
	(kgim_meta_mode_resource_fn *)	NULL,
	(kgim_meta_mode_prepare_fn *)	NULL,
	(kgim_meta_mode_enter_fn *)	vga_ramdac_mode_enter,
	(kgim_meta_mode_leave_fn *)	NULL,

	sizeof(vga_ramdac_t),
	0,
	sizeof(vga_ramdac_mode_t)

};

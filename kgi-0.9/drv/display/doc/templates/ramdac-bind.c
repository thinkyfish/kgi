/* -----------------------------------------------------------------------------
**	##VENDOR## ##META## ramdac binding
** -----------------------------------------------------------------------------
**	Copyright (C)	2000	##AUTHOR_NAME##
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**
**	$Log: ramdac-bind.c,v $
**	Revision 1.1.1.1  2000/04/18 08:51:12  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	##AUTHOR##
#define	KGIM_RAMDAC_DRIVER	"$Revision: 1.1.1.1 $"

#include <kgi/module.h>

#ifndef DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif

#include "ramdac/##VENDOR##/##META##.h"
#include "ramdac/##VENDOR##/##META##-bind.h"

kgi_error_t ##meta##_ramdac_init_module(##meta##_ramdac_t *##meta##,
	##meta##_ramdac_io_t *##meta##_io, const kgim_options_t *options)
{
	kgi_u16_t vendor_id, device_id;

	KRN_ASSERT(##meta##);
	KRN_ASSERT(##meta##_io);
	KRN_ASSERT(options);

#warning for integrated DACs, verify the chipset via PCICFG if you can.
	vendor_id = pcicfg_in16(##META##_PCIDEV(##meta##_io) + PCI_VENDOR_ID);
	device_id = pcicfg_in16(##META##_PCIDEV(##meta##_io) + PCI_DEVICE_ID);

	switch (PCICFG_SIGNATURE(vendor_id, device_id)) {

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_##VENDOR##, PCI_DEVICE_ID_##MODEL##):
		break;

	default:
		KRN_NOTICE("no ##VENDOR## ##MODEL## DAC detected");
		return -KGI_ERRNO(RAMDAC, NODEV);
	}

#warning detect or verify the dac if possible (e.g. verify ID registers)

	##meta##->ramdac.revision	= KGIM_RAMDAC_REVISION;
	kgim_strcpy(##meta##->ramdac.vendor, "##VENDOR##");
	kgim_strcpy(##meta##->ramdac.model, "##MODEL## DAC");
/*	##meta##->ramdac.flags	*/
	##meta##->ramdac.mode_size	= sizeof(##meta##_ramdac_mode_t);
/*	##meta##->ramdac.Command	*/

	##meta##->ramdac.maxdots.x	= 0x07FF;
	##meta##->ramdac.maxdots.y	= 0x07FF;
	##meta##->ramdac.lclk.min =
			KGIM_DEFAULT(options->ramdac->lclk_min, 0);
	##meta##->ramdac.lclk.max =
			KGIM_DEFAULT(options->ramdac->lclk_max, 135000000);
	##meta##->ramdac.dclk.min =
			KGIM_DEFAULT(options->ramdac->dclk_min, 0);
	##meta##->ramdac.dclk.max =
			KGIM_DEFAULT(options->ramdac->dclk_max, 135000000);

	KRN_NOTICE("%s %s driver " KGIM_RAMDAC_DRIVER,
		##meta##->ramdac.vendor, ##meta##->ramdac.model);

	return KGI_EOK;
}

const kgim_meta_t ##meta##_ramdac_meta =
{
	(kgim_meta_init_module_fn *)	##meta##_ramdac_init_module,
	(kgim_meta_done_module_fn *)	NULL,
	(kgim_meta_init_fn *)		##meta##_ramdac_init,
	(kgim_meta_done_fn *)		##meta##_ramdac_done,
	(kgim_meta_mode_check_fn *)	##meta##_ramdac_mode_check,
	(kgim_meta_mode_resource_fn *)	NULL,
	(kgim_meta_mode_prepare_fn *)	NULL,
	(kgim_meta_mode_enter_fn *)	##meta##_ramdac_mode_enter,
	(kgim_meta_mode_leave_fn *)	NULL,

	sizeof(##meta##_ramdac_t),
	0,
	sizeof(##meta##_ramdac_mode_t)

};

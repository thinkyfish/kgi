/* ----------------------------------------------------------------------------
**	Matrox Gx00 ramdac meta binding
** ----------------------------------------------------------------------------
**	Copyright (C) 1999-2000		Johan Karlberg
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: Gx00-bind.c,v $
*/

#include <kgi/maintainers.h>
#define	MAINTAINER		Johan_Karlberg
#define	KGIM_RAMDAC_DRIVER	"$Revision: 1.2 $"

#define DEBUG_LEVEL 255

#include <kgi/module.h>
#include "chipset/Matrox/Gx00.h"
#include "ramdac/Matrox/Gx00-meta.h"

/*
**	KGI interface
*/

kgi_error_t mgag_ramdac_init_module(mgag_ramdac_t *mgag, 
	mgag_ramdac_io_t *mgag_io, const kgim_options_t *options) 
{
	KRN_DEBUG(2, "ramdac_init_module() initiated");

	KRN_ASSERT(mgag);
	KRN_ASSERT(mgag_io);
	KRN_ASSERT(options);

	kgim_memset(mgag, 0, sizeof(*mgag));

	kgim_strcpy(mgag->ramdac.vendor, "Matrox Graphics Inc");
	kgim_strcpy(mgag->ramdac.model, "1x64/Gx00 integrated RAMDAC");

	mgag->ramdac.revision	= KGIM_RAMDAC_REVISION;
	mgag->ramdac.mode_size	= sizeof(mgag_ramdac_mode_t);

	mgag->ramdac.maxdots.x	=
	mgag->ramdac.maxdots.y	= 0x7FFF;

	mgag->ramdac.dclk.min = options->ramdac->dclk_min ? options->ramdac->dclk_min : 0;

#warning Do not use function calls in macros!
	switch (PCICFG_SIGNATURE(pcicfg_in16(MGAG_PCIDEV(mgag_io) + PCI_VENDOR_ID), pcicfg_in16(MGAG_PCIDEV(mgag_io) + PCI_DEVICE_ID))) {

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_MYS):

		switch (pcicfg_in8 (MGAG_PCIDEV(mgag_io) + PCI_REVISION_ID)) {

		case 1:
			mgag->ramdac.dclk.max = options->ramdac->dclk_max
				? options->ramdac->dclk_max : 135 MHZ;
			break;
		case 2:
			mgag->ramdac.dclk.max = options->ramdac->dclk_max
				? options->ramdac->dclk_max : 175  MHZ;
			break;
		case 3:
			mgag->ramdac.dclk.max = options->ramdac->dclk_max
				? options->ramdac->dclk_max : 220 MHZ;
			break;
		default:
			KRN_INTERNAL_ERROR;
		}
		break;

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX,
		PCI_DEVICE_ID_MATROX_G200_PCI):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, 
		PCI_DEVICE_ID_MATROX_G200_AGP):

		mgag->ramdac.dclk.max = options->ramdac->dclk_max
			? options->ramdac->dclk_max
			: ((mgag->flags & MGAG_CF_SGRAM) ? 250 MHZ : 230 MHZ);
		break;

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_G400):

		mgag->ramdac.dclk.max = options->ramdac->dclk_max 
			? options->ramdac->dclk_max : 300 MHZ;
		break;

	default:
		KRN_NOTICE("no Matrox Integrated RAMDAC detected.");
		return -E(RAMDAC, NODEV);
	}

	KRN_NOTICE("%s %s driver " KGIM_RAMDAC_DRIVER, 
		mgag->ramdac.vendor, mgag->ramdac.model);

	KRN_DEBUG(2, "%s ramdac_init_module() completed");

	return KGI_EOK;
}

const kgim_meta_t mgag_ramdac_meta =
{
	(kgim_meta_init_module_fn *)	mgag_ramdac_init_module,
	(kgim_meta_done_module_fn *)	NULL,
	(kgim_meta_init_fn *)		mgag_ramdac_init,
	(kgim_meta_done_fn *)		mgag_ramdac_done,
	(kgim_meta_mode_check_fn *)	mgag_ramdac_mode_check,
	(kgim_meta_mode_resource_fn *)	NULL,
	(kgim_meta_mode_prepare_fn *)	NULL,
	(kgim_meta_mode_enter_fn *)	mgag_ramdac_mode_enter,
	(kgim_meta_mode_leave_fn *)	NULL,

	sizeof(mgag_ramdac_t),
	0,
	sizeof(mgag_ramdac_mode_t)

};

/* ----------------------------------------------------------------------------
**	##VENDOR## ##META## chipset implementation
** ----------------------------------------------------------------------------
**	Copyright (C)	2000	##AUTHOR_NAME##
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: chipset-meta.c,v $
**	Revision 1.1.1.1  2000/04/18 08:51:11  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	##AUTHOR##
#define	KGIM_CHIPSET_DRIVER	"$Revision: 1.1.1.1 $"

#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif

#include "chipset/##VENDOR##/##META##.h"
#include "chipset/##VENDOR##/##META##-meta.h"

/*
**	Helper functions
*/

static inline void ##meta##_chipset_sync(##meta##_chipset_io_t *##meta##_io)
{
#warning	add code to synchronize with chipset here.
}

static inline kgi_u_t ##meta##_next_supported_width(kgi_u_t width)
{
#warning	add supported image buffer widths here. 
	return (width + 7) & ~7;
}

kgi_error_t ##meta##_chipset_irq_handler(##meta##_chipset_t *##meta##, 
	##meta##_chipset_io_t *##meta##_io, irq_system_t *system)
{
	kgi_u32_t handled = 0;
#warning read IRQ status flags here.
	kgi_u32_t flags = 0;

	KRN_ASSERT(##meta##);
	KRN_ASSERT(##meta##_io);

	if (flags & ~handled) {

		KRN_ERROR("unhandled interrupt flag(s) %.8x (pcidev %.8x)",
			flags & ~handled, ##META##_PCIDEV(##meta##_io));
	}

	if (! flags) {

		KRN_ERROR("interrupt but no reason indicated.");
	}

#warning clean handled interrupts here.

	return KGI_EOK;
}


kgi_error_t ##meta##_chipset_init(##meta##_chipset_t *##meta##,
	##meta##_chipset_io_t *##meta##_io, const kgim_options_t *options)
{
	pcicfg_vaddr_t pcidev = ##META##_PCIDEV(##meta##_io);

	KRN_ASSERT(##meta##);
	KRN_ASSERT(##meta##_io);
	KRN_ASSERT(options);

	KRN_DEBUG(2, "initializing %s %s",
		##meta##->chipset.vendor, ##meta##->chipset.model);

#warning	You might have to set PCICFG base registers here...

	PCICFG_SET_BASE32(##meta##_io->io.base_io,
		pcidev + PCI_BASE_ADDRESS_0);
	PCICFG_SET_BASE32(##meta##_io->mem.base_io,
		pcidev + PCI_BASE_ADDRESS_1);
	PCICFG_SET_BASE32(##meta##_io->rom.base_io | PCI_ROM_ADDRESS_ENABLE,
		pcidev + PCI_ROM_ADDRESS);

	KRN_DEBUG(2, "PCI (re-)configuration done");

#warning	enable chipset here
#warning	read & save initial configuration

	KRN_DEBUG(2, "chipset enabled");

#warning	examine chipset, e.g. detect memory sizes, capabilities etc.

	##meta##->chipset.memory = options->chipset->memory
		? options->chipset->memory
#warning 	(auto-) detect installed memory here.
		: 0 KB;

	if (##meta##->flags & ##META##_CF_IRQ_CLAIMED) {

#warning	enable interrupts on device here.
	}

	return KGI_EOK;
}

void ##meta##_chipset_done(##meta##_chipset_t *##meta##, 
	##meta##_chipset_io_t *##meta##_io,
	const kgim_options_t *options)
{
	pcicfg_vaddr_t pcidev = ##META##_PCIDEV(##meta##_io);

	if (##meta##->flags & ##META##_CF_IRQ_CLAIMED) {

#warning	disable interrupts on device here.
	}

#warning	restore initial chipset state here.

#warning	restore initial pcicfg base addresses here.
	PCICFG_SET_BASE32(##meta##->pci.BaseAddr0,
		pcidev + PCI_BASE_ADDRESS_0);
	PCICFG_SET_BASE32(##meta##->pci.BaseAddr1,
		pcidev + PCI_BASE_ADDRESS_1);
	PCICFG_SET_BASE32(##meta##->pci.RomAddr,
		pcidev + PCI_ROM_ADDRESS);
}

kgi_error_t ##meta##_chipset_mode_check(##meta##_chipset_t *##meta##,
	##meta##_chipset_io_t *##meta##_io,
	##meta##_chipset_mode_t *##meta##_mode, 
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_dot_port_mode_t *dpm = img->out;
	const kgim_monitor_mode_t *crt_mode = ##meta##_mode->##meta##.kgim.crt;
	kgi_u_t bpf, bpc, bpp, width, lclk;
	kgi_mmio_region_t *r;
	kgi_accel_t *a;
	kgi_u_t mul, div, bpd;

	if (images != 1) {

		KRN_DEBUG(1, "%i images not supported.", images);
		return -KGI_ERRNO(CHIPSET, NOSUP);
	}

	/*	for text16 support we fall back to VGA mode
	**	for unsupported image flags, bail out.
	*/
	if ((img[0].flags & KGI_IF_TEXT16) || 
		(img[0].fam & KGI_AM_TEXTURE_INDEX)) {

		return vga_chipset_mode_check(&##meta##->vga, &##meta##_io->vga,
			&##meta##_mode->vga, cmd, img, images);
	}
	if (img[0].flags & (KGI_IF_TILE_X | KGI_IF_TILE_Y | KGI_IF_VIRTUAL)) {

		KRN_DEBUG(1, "image flags %.8x not supported", img[0].flags);
		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	/*	check if common attributes are supported.
	*/
	switch (img[0].cam) {

	case 0:
		break;

	case KGI_AM_STENCIL | KGI_AM_Z:
		if ((1 != img[0].bpca[0]) || (15 != img[0].bpca[1])) {

			KRN_DEBUG(1, "S%iZ%i local buffer not supported",
				img[0].bpca[0], img[0].bpca[1]);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
		break;

	case KGI_AM_Z:
		if (16 != img[0].bpca[1]) {

			KRN_DEBUG(1,"Z%i local buffer not supported",
				img[0].bpca[0]);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
	default:
		KRN_DEBUG(1, "common attributes %.8x not supported",
			img[0].cam);
		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	/*	total bits per dot
	*/
	bpf = kgim_attr_bits(img[0].bpfa);
	bpc = kgim_attr_bits(img[0].bpca);
	bpd = kgim_attr_bits(dpm->bpda);
	bpp = (img[0].flags & KGI_IF_STEREO)
		? (bpc + bpf*img[0].frames*2)
		: (bpc + bpf*img[0].frames);

	lclk = (cmd == KGI_TC_PROPOSE)
		? 0 : dpm->dclk * dpm->lclk.mul / dpm->lclk.div;

	switch (cmd) {

	case KGI_TC_PROPOSE:

		KRN_ASSERT(img[0].frames);
		KRN_ASSERT(bpp);

		/*	if size.x or size.y are zero, default to 640x400
		*/
		if ((0 == img[0].size.x) || (0 == img[0].size.y)) {

			img[0].size.x = 640;
			img[0].size.y = 400;
		}

		/*	if virt.x and virt.y are zero, default to size;
		**	if either virt.x xor virt.y is zero, maximize the other
		*/
		if ((0 == img[0].virt.x) && (0 == img[0].virt.y)) {

			img[0].virt.x = img[0].size.x;
			img[0].virt.y = img[0].size.y;
		}

		if (0 == img[0].virt.x) {

			img[0].virt.x = (8 * ##meta##->chipset.memory) / 
				(img[0].virt.y * bpp);

			if (img[0].virt.x > ##meta##->chipset.maxdots.x) {

				img[0].virt.x = ##meta##->chipset.maxdots.x;
			}
		}
#warning	restrict img[0].virt.x to supported width here.

		if (0 == img[0].virt.y) {

			img[0].virt.y = (8 * ##meta##->chipset.memory) /
				(img[0].virt.x * bpp);
		}

		/*	Are we beyond the limits of the H/W?
		*/
		if ((img[0].size.x >= ##meta##->chipset.maxdots.x) || 
			(img[0].virt.x >= ##meta##->chipset.maxdots.x)) {

			KRN_DEBUG(1, "%i (%i) horizontal pixels are too many",
				img[0].size.x, img[0].virt.x);
			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}

		if ((img[0].size.y >= ##meta##->chipset.maxdots.y) ||
			(img[0].virt.y >= ##meta##->chipset.maxdots.y)) {

			KRN_DEBUG(1, "%i (%i) vertical pixels are too many",
				img[0].size.y, img[0].virt.y);
			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}

		if ((img[0].virt.x * img[0].virt.y * bpp) > 
			(8 * ##meta##->chipset.memory)) {

			KRN_DEBUG(1, "not enough memory for (%ipf*%if + %ipc)@"
				"%ix%i", bpf, img[0].frames, bpc,
				img[0].virt.x, img[0].virt.y);
			return -KGI_ERRNO(CHIPSET,NOMEM);
		}

#warning	Take screen visible width up to next supported width

		/*	set CRT visible fields
		*/
		dpm->dots.x = img[0].size.x;
		dpm->dots.y = img[0].size.y;

#warning	check for line and pixel doubling support for low-res modes
		if (img[0].size.y < 400) {

			dpm->dots.y += img[0].size.y;
		}
		return KGI_EOK;

	case KGI_TC_LOWER:
	case KGI_TC_RAISE:

		/*	adjust lclk and rclk
		*/
#warning	dot port load clock and reference clock rates here!
		dpm->lclk.mul = 1;
		dpm->lclk.div = 1;

		dpm->rclk.mul = 1;
		dpm->rclk.div = 1;

		if (cmd == KGI_TC_LOWER) {

			if (dpm->dclk < ##meta##->chipset.dclk.min) {

				KRN_DEBUG(1, "DCLK = %i Hz is too low",
					dpm->dclk);
				return -KGI_ERRNO(CHIPSET, UNKNOWN);
			}
#warning		check/set LCLK maximum here
			if (lclk > 50000000) {

				dpm->dclk = 50000000 * dpm->lclk.div /
					dpm->lclk.mul;
			}

		} else {

#warning		check/set LCLK maximum here
			if (lclk > 50000000) {

				KRN_DEBUG(1, "LCLK = %i Hz is too high", lclk);
				return -KGI_ERRNO(CHIPSET, UNKNOWN);
			}
		}
		return KGI_EOK;


	case KGI_TC_CHECK:

#warning Do proper checking of hardware limits etc. here.

		width = ##meta##_next_supported_width(img[0].virt.x);

		if (width != img[0].virt.x) {

			return -KGI_ERRNO(CHIPSET, INVAL);
		}
		if ((img[0].size.x >= ##meta##->chipset.maxdots.x) ||
			(img[0].size.y >= ##meta##->chipset.maxdots.y) || 
			(img[0].virt.x >= ##meta##->chipset.maxdots.x) ||
			((img[0].virt.y * img[0].virt.x * bpp) >
				(8 * ##meta##->chipset.memory))) {

			KRN_DEBUG(1, "resolution too high: %ix%i (%ix%i)",
				img[0].size.x, img[0].size.y,
				img[0].virt.x, img[0].virt.y);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}

#warning	check LCLK and RCLK rations here!
		if (((dpm->lclk.mul != 1) && (dpm->lclk.div != 1)) ||
			((dpm->rclk.mul != dpm->lclk.mul) &&
			(dpm->rclk.div != dpm->lclk.div))) {

			KRN_DEBUG(1, "invalid LCLK (%i:%i) or CLK (%i:%i)", 
				dpm->lclk.mul, dpm->lclk.div,
				dpm->rclk.mul, dpm->rclk.div);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}

#warning	check maximum LCLK rate here.
		if (lclk > 50000000) {

			KRN_DEBUG(1, "LCLK = %i Hz is too high\n", lclk);
			return -KGI_ERRNO(CHIPSET, CLK_LIMIT);
		}
		break;

	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(CHIPSET, UNKNOWN);
	}


	/*	Now everything is checked and should be sane.
	**	proceed to setup device dependent mode.
	*/
#warning initialize ##meta##_mode here.

	/*	initialize exported resources
	*/
	r = &##meta##_mode->##meta##.video_memory_aperture;
	r->meta = ##meta##;
	r->meta_io = ##meta##_io;
	r->type = KGI_RT_MMIO_FRAME_BUFFER;
	r->prot = KGI_PF_APP | KGI_PF_LIB | KGI_PF_DRV;
	r->name = "##MODEL## video memory aperture";
	r->access = 64 + 32 + 16 + 8;
	r->align  = 64 + 32 + 16 + 8;
	r->size   = r->win.size = ##meta##->chipset.memory;
	r->win.bus  = ##meta##_io->mem.base_bus;
	r->win.phys = ##meta##_io->mem.base_phys;
	r->win.virt = ##meta##_io->mem.base_virt;

	return KGI_EOK;
}

kgi_resource_t *##meta##_chipset_mode_resource(##meta##_chipset_t *##meta##, 
	##meta##_chipset_mode_t *##meta##_mode, 
	kgi_image_mode_t *img, kgi_u_t images, kgi_u_t index)
{
	if (img->fam & KGI_AM_TEXTURE_INDEX) {

		return vga_chipset_mode_resource(&##meta##->vga, 
			&##meta##_mode->vga, img, images, index);
	}

	switch (index) {

	case 0:	return	(kgi_resource_t *) &##meta##_mode->##meta##.video_memory_aperture;
	}
	return NULL;
}

void ##meta##_chipset_mode_prepare(##meta##_chipset_t *##meta##, ##meta##_chipset_io_t *##meta##_io,
	##meta##_chipset_mode_t *##meta##_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	##meta##_chipset_sync(##meta##_io);

	if (img->fam & KGI_AM_TEXTURE_INDEX) {

#warning 	prepare for VGA mode here.

		vga_chipset_mode_prepare(&##meta##->vga, &##meta##_io->vga,
			&##meta##_mode->vga, img, images);

		KRN_DEBUG(2, "prepared for VGA-mode");
		return;
	}

#warning	prepare for native mode here.

	KRN_DEBUG(2, "prepared for ##MODEL## mode");
}

void ##meta##_chipset_mode_enter(##meta##_chipset_t *##meta##, ##meta##_chipset_io_t *##meta##_io,
	##meta##_chipset_mode_t *##meta##_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	if (img->fam & KGI_AM_TEXTURE_INDEX) {

		vga_chipset_mode_enter(&##meta##->vga, &##meta##_io->vga,
			&##meta##_mode->vga, img, images);
		return;
	}

#warning	set native mode here.

	##meta##->mode = ##meta##_mode;
}

void ##meta##_chipset_mode_leave(##meta##_chipset_t *##meta##, ##meta##_chipset_io_t *##meta##_io,
	##meta##_chipset_mode_t *##meta##_mode, kgi_image_mode_t *img, kgi_u_t images)
{
#warning	sync with chipset here.

	##meta##->mode = NULL;
}

/* ----------------------------------------------------------------------------
**	##VENDOR## ##META## chipset binding
** ----------------------------------------------------------------------------
**	Copyright (C)	2000	##AUTHOR_NAME##
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: chipset-bind.c,v $
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
#include "chipset/##VENDOR##/##META##-bind.h"

/*
**	##MODEL## specific I/O functions
*/
#warning implement chipset specific I/O functions here.
#define	##META##_CONTROL_BASE	##meta##_io->mem.base_virt

static inline void ##meta##_chipset_ctrl_out32(##meta##_chipset_io_t *##meta##_io,
	kgi_u32_t val, kgi_u32_t reg)
{
	KRN_DEBUG(3, "Ctrl%.3x <= %.8x", reg, val);
	mem_out32(val, ##META##_CONTROL_BASE + reg);
}

static inline kgi_u32_t ##meta##_chipset_ctrl_in32(##meta##_chipset_io_t *##meta##_io,
	kgi_u32_t reg)
{
	return mem_in32(##META##_CONTROL_BASE + reg);
}

/*
**	VGA subsystem I/O functions
*/
#warning provide VGA subsystem I/O functions here.
static inline void ##meta##_chipset_vga_seq_out8(##meta##_chipset_io_t *##meta##_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_SEQ%.2x <= %.2x", reg, val);
}

static inline kgi_u8_t ##meta##_chipset_vga_seq_in8(##meta##_chipset_io_t *##meta##_io,
	kgi_u_t reg)
{
	register kgi_u8_t val;
	val = 0xab;
	KRN_DEBUG(3, "VGA_SEQ%.2x => %.2x", reg, val);
	return val;
}

static inline void ##meta##_chipset_vga_crt_out8(##meta##_chipset_io_t *##meta##_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_CRT%.2x <= %.2x", reg, val);
}

static inline kgi_u8_t ##meta##_chipset_vga_crt_in8(##meta##_chipset_io_t *##meta##_io,
	kgi_u_t reg)
{
	register kgi_u8_t val;
	val = 0xab;
	KRN_DEBUG(3, "VGA_CRT%.2x => %.2x", reg, val);
	return val;
}

static inline void ##meta##_chipset_vga_grc_out8(##meta##_chipset_io_t *##meta##_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_GRC%.2x <= %.2x", reg, val);
}

static inline kgi_u8_t ##meta##_chipset_vga_grc_in8(##meta##_chipset_io_t *##meta##_io,
	kgi_u_t reg)
{
	register kgi_u8_t val;
	val = 0xab;
	KRN_DEBUG(3, "VGA_CRT%.2x => %.2x", reg, val);
	return val;
}

static inline void ##meta##_chipset_vga_atc_out8(##meta##_chipset_io_t *##meta##_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_ATC%.2x <= %.2x", reg, val);
	/* read ATC register here, dont't forget to clear the AFF before!
	*/
}

static inline kgi_u8_t ##meta##_chipset_vga_atc_in8(##meta##_chipset_io_t *##meta##_io,
	kgi_u_t reg)
{
	register kgi_u8_t val;
	val = 0xab;
	KRN_DEBUG(3, "VGA_ATC%.2x => %.2x", reg, val);
	return val;
}

static inline void ##meta##_chipset_vga_misc_out8(##meta##_chipset_io_t *##meta##_io,
	kgi_u8_t val)
{
	KRN_DEBUG(3, "VGA_MISC <= %.2x", val);
}

static inline kgi_u8_t ##meta##_chipset_vga_misc_in8(##meta##_chipset_io_t *##meta##_io)
{
	register kgi_u8_t val;
	val = 0xab;
	KRN_DEBUG(3, "VGA_MISC => %.2x", val);
	return val;
}

static inline void ##meta##_chipset_vga_fctrl_out8(##meta##_chipset_io_t *##meta##_io,
	kgi_u8_t val)
{
	KRN_DEBUG(3, "VGA_FCTRL <= %.2x", val);
}

static inline kgi_u8_t ##meta##_chipset_vga_fctrl_in8(##meta##_chipset_io_t *##meta##_io)
{
	register kgi_u8_t val;
	val = 0xab;
	KRN_DEBUG(3, "VGA_FCTL => %.2x", val);
	return val;
}

/*
**	DAC subsystem I/O
*/
#warning	provide DAC I/O functions here.

static inline void ##meta##_chipset_dac_out8(##meta##_chipset_io_t *##meta##_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_ASSERT(reg < ##META##_MAX_DacRegisters);

	KRN_DEBUG(3, "DAC%.2x <= %.2x", reg, val);
}

static inline kgi_u8_t ##meta##_chipset_dac_in8(##meta##_chipset_io_t *##meta##_io,
	kgi_u_t reg)
{
	register kgi_u8_t val;
	KRN_ASSERT(reg < ##META##_MAX_DacRegisters);

	val = 0xab;
	KRN_DEBUG(3, "DAC%.2x => %.2x", reg, val);
	return val;
}

static inline void ##meta##_chipset_dac_outs8(##meta##_chipset_io_t *##meta##_io,
	kgi_u_t reg, void *buf, kgi_u_t cnt)
{
	KRN_ASSERT(reg < ##META##_MAX_DacRegisters);

	KRN_DEBUG(3, "DAC%.2x <= (%i bytes)", reg, cnt);
}

static inline void ##meta##_chipset_dac_ins8(##meta##_chipset_io_t *##meta##_io,
	kgi_u_t reg, void *buf, kgi_u_t cnt)
{
	KRN_ASSERT(reg < ##META##_MAX_DacRegisters);

	KRN_DEBUG(3, "DAC%.2x => (%i bytes)", reg, cnt);
}

/*
**	Clock Control
*/
#warning provide clock I/O functions here.
static inline void ##meta##_chipset_clk_out8(##meta##_chipset_io_t *##meta##_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "CLK%.2x <= %.2x", reg, val);
}

static inline kgi_u8_t ##meta##_chipset_clk_in8(##meta##_chipset_io_t *##meta##_io,
	kgi_u_t reg)
{
	register kgi_u8_t val;

	val = 0xab;
	KRN_DEBUG(3, "CLK%.2x => %.2x", reg, val);
	return val;
}

/*
**	chipset_init_module()
*/

kgi_error_t ##meta##_chipset_init_module(##meta##_chipset_t *##meta##, 
	##meta##_chipset_io_t *##meta##_io, const kgim_options_t *options)
{
	static const kgi_u32_t ##meta##_chipset_pcicfg[] =
	{
		PCICFG_SIGNATURE(PCI_VENDOR_ID_##VENDOR##, PCI_DEVICE_ID_##VENDOR##_##MODEL##),
		PCICFG_SIGNATURE(0,0)
	};

	pcicfg_vaddr_t pcidev = options->pci->dev;
	kgi_u16_t subvendor, subdevice;

	KRN_ASSERT(##meta##);
	KRN_ASSERT(##meta##_io);
	KRN_ASSERT(options);


	/*	auto-detect/verify the chipset
	*/
	if (pcidev == PCICFG_NULL) {

		if (pcicfg_find_device(&pcidev, ##meta##_chipset_pcicfg)) {

			KRN_ERROR("No supported device found!");
			return -KGI_ERRNO(CHIPSET,INVAL);
		}
	}

	/* 	##meta##->chipset.memory is initialized after we have access 
	** 	to the chipset.
	*/
	kgim_memset(##meta##, 0, sizeof(*##meta##));
	##meta##->chipset.revision	= KGIM_CHIPSET_REVISION;
	##meta##->chipset.mode_size	= sizeof(##meta##_chipset_mode_t);
	##meta##->chipset.dclk.max	= 200000000 /* Hz */;
	##meta##->chipset.vendor_id	= pcicfg_in16(pcidev + PCI_VENDOR_ID);
	##meta##->chipset.device_id	= pcicfg_in16(pcidev + PCI_DEVICE_ID);

	switch (PCICFG_SIGNATURE(##meta##->chipset.vendor_id,
		##meta##->chipset.device_id)) {

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_##VENDOR##, PCI_DEVICE_ID_##VENDOR##_##MODEL##):
		kgim_strcpy(##meta##->chipset.vendor, "##VENDOR##");
		kgim_strcpy(##meta##->chipset.model,  "##MODEL##");
		##meta##->chipset.maxdots.x = 1536;
		##meta##->chipset.maxdots.y = 2048;
		break;

	default:
		KRN_ERROR("Device not yet supported (vendor %.4x device %.4x).",
			##meta##->chipset.vendor_id, ##meta##->chipset.device_id);
		return -KGI_ERRNO(CHIPSET, NOSUP);
	}

	subvendor = pcicfg_in16(pcidev + PCI_SUBSYSTEM_VENDOR_ID);
	subdevice = pcicfg_in16(pcidev + PCI_SUBSYSTEM_ID);
	KRN_DEBUG(1, "  subvendor %.4x, subdevice %4x", subvendor, subdevice);

	/*	save initial PCICFG state
	*/
	##meta##->pci.Command	= pcicfg_in32(pcidev + PCI_COMMAND);
	##meta##->pci.LatTimer	= pcicfg_in32(pcidev + PCI_LATENCY_TIMER);
	##meta##->pci.IntLine	= pcicfg_in32(pcidev + PCI_INTERRUPT_LINE);
	##meta##->pci.BaseAddr0	= pcicfg_in32(pcidev + PCI_BASE_ADDRESS_0);
	##meta##->pci.BaseAddr1	= pcicfg_in32(pcidev + PCI_BASE_ADDRESS_1);
	##meta##->pci.RomAddr	= pcicfg_in32(pcidev + PCI_ROM_ADDRESS);
#warning save additional PCICFG state here.

	/*	Initialize driver claimed regions and I/O binding.
	*/
#warning initialize additional I/O regions here.
	##meta##_io->vga.kgim.pcidev = pcidev;

	##meta##_io->io.name      = "##MODEL## io";
	##meta##_io->io.device    = pcidev;
	##meta##_io->io.base_virt = IO_NULL;
	##meta##_io->io.base_io	  = ##meta##->pci.BaseAddr0 & ~(##META##_Base0_Size - 1);
	##meta##_io->io.size	  = ##META##_Base0_Size;
	##meta##_io->io.decode	  = IO_DECODE_ALL;


	##meta##_io->mem.name      = "##MODEL## mmio aperture";
	##meta##_io->mem.device    = pcidev;
	##meta##_io->mem.base_virt = MEM_NULL;
	##meta##_io->mem.base_io   = ##meta##->pci.BaseAddr1 & ~(##META##_Base1_Size - 1);
	##meta##_io->mem.size	   = ##META##_Base1_Size;
	##meta##_io->mem.decode	   = MEM_DECODE_ALL;

	##meta##_io->rom.name      = "##MODEL## mmio aperture";
	##meta##_io->rom.device    = pcidev;
	##meta##_io->rom.base_virt = MEM_NULL;
	##meta##_io->rom.base_io   = ##meta##->pci.RomAddr & ~(##META##_ROM_Size - 1);
	##meta##_io->rom.size	   = ##META##_ROM_Size;
	##meta##_io->rom.decode	   = MEM_DECODE_ALL;

	/*	make sure no other driver is serving the chipset
	*/
	if (##meta##->pci.Command & PCI_COMMAND_IO) {

		if (io_check_region(&##meta##_io->vga.ports)) {

			KRN_ERROR("%s region served (maybe another driver?).",
				##meta##_io->vga.ports);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
	}
	if (##meta##->pci.Command & PCI_COMMAND_MEMORY) {

		if (mem_check_region(&##meta##_io->mem)) {

			KRN_ERROR("%s region already served!",
				##meta##_io->mem.name);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
	}

	/*	If root specified new base addresses, he knows the
	**	consequences. If not, it's not our fault...
	*/
#	define	SET_BASE(region, addr, size)			\
		if (addr) {					\
			region.base_io = addr & ~(size - 1);	\
		}

	SET_BASE(##meta##_io->io,  options->pci->base0, ##META##_Base0_Size);
	SET_BASE(##meta##_io->mem, options->pci->base1, ##META##_Base1_Size);
	SET_BASE(##meta##_io->rom, options->pci->baseR, ##META##_ROM_Size);

#	undef SET_BASE

	/*	make sure the memory regions are free.
	*/
	if (io_check_region(&##meta##_io->io) ||
		mem_check_region(&##meta##_io->mem) ||
		mem_check_region(&##meta##_io->rom)) {

		KRN_ERROR("check of ##MODEL## io and memory regions failed!");
		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	/*	claim the regions & the IRQ line
	*/
	 io_claim_region(&##meta##_io->io);
	mem_claim_region(&##meta##_io->mem);
	mem_claim_region(&##meta##_io->rom);

	if (KGI_EOK == irq_claim_line(&##meta##_io->irq)) {

		KRN_DEBUG(1, "interrupt line claimed successfully");
		##meta##->flags |= ##META##_CF_IRQ_CLAIMED;
	}

	/*	mem aperture serves as vga aperture
	*/
#warning adjust VGA aperture initialization to particular chipset.
	##meta##_io->vga.aperture = ##meta##_io->mem;

	##meta##_io->vga.kgim.DacOut8  = (void *) ##meta##_chipset_dac_out8;
	##meta##_io->vga.kgim.DacIn8   = (void *) ##meta##_chipset_dac_in8;
	##meta##_io->vga.kgim.DacOuts8 = (void *) ##meta##_chipset_dac_outs8;
	##meta##_io->vga.kgim.DacIns8  = (void *) ##meta##_chipset_dac_ins8;

	##meta##_io->vga.kgim.ClkOut8  = (void *) ##meta##_chipset_clk_out8;
	##meta##_io->vga.kgim.ClkIn8   = (void *) ##meta##_chipset_clk_in8;
	
	##meta##_io->vga.SeqOut8   = (void *) ##meta##_chipset_vga_seq_out8;
	##meta##_io->vga.SeqIn8    = (void *) ##meta##_chipset_vga_seq_in8;
	##meta##_io->vga.CrtOut8   = (void *) ##meta##_chipset_vga_crt_out8;
	##meta##_io->vga.CrtIn8    = (void *) ##meta##_chipset_vga_crt_in8;
	##meta##_io->vga.GrcOut8   = (void *) ##meta##_chipset_vga_grc_out8;
	##meta##_io->vga.GrcIn8    = (void *) ##meta##_chipset_vga_grc_in8;
	##meta##_io->vga.AtcOut8   = (void *) ##meta##_chipset_vga_atc_out8;
	##meta##_io->vga.AtcIn8    = (void *) ##meta##_chipset_vga_atc_in8;
	##meta##_io->vga.MiscOut8  = (void *) ##meta##_chipset_vga_misc_out8;
	##meta##_io->vga.MiscIn8   = (void *) ##meta##_chipset_vga_misc_in8;
	##meta##_io->vga.FctrlOut8 = (void *) ##meta##_chipset_vga_fctrl_out8;
	##meta##_io->vga.FctrlIn8  = (void *) ##meta##_chipset_vga_fctrl_in8;

	##meta##_io->CtrlOut32	= (void *) ##meta##_chipset_ctrl_out32;
	##meta##_io->CtrlIn32	= (void *) ##meta##_chipset_ctrl_in32;

	KRN_NOTICE("%s %s driver " KGIM_CHIPSET_DRIVER,
		##meta##->chipset.vendor, ##meta##->chipset.model);
	return KGI_EOK;
}

void ##meta##_chipset_done_module(##meta##_chipset_t *##meta##, 
	##meta##_chipset_io_t *##meta##_io, const kgim_options_t *options)
{
	if (##meta##->flags & ##META##_CF_IRQ_CLAIMED) {

		irq_free_line(&##meta##_io->irq);
	}

	mem_free_region(&##meta##_io->rom);
	mem_free_region(&##meta##_io->mem);
	 io_free_region(&##meta##_io->io);
}

const kgim_meta_t ##meta##_chipset_meta =
{
	(kgim_meta_init_module_fn *)	##meta##_chipset_init_module,
	(kgim_meta_done_module_fn *)	##meta##_chipset_done_module,
	(kgim_meta_init_fn *)		##meta##_chipset_init,
	(kgim_meta_done_fn *)		##meta##_chipset_done,
	(kgim_meta_mode_check_fn *)	##meta##_chipset_mode_check,
	(kgim_meta_mode_resource_fn *)	##meta##_chipset_mode_resource,
	(kgim_meta_mode_prepare_fn *)	##meta##_chipset_mode_prepare,
	(kgim_meta_mode_enter_fn *)	##meta##_chipset_mode_enter,
	(kgim_meta_mode_leave_fn *)	##meta##_chipset_mode_leave,

	sizeof(##meta##_chipset_t),
	sizeof(##meta##_chipset_io_t),
	sizeof(##meta##_chipset_mode_t)
};

/* ----------------------------------------------------------------------------
**	Matrox Gx00 chipset binding
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
**	Revision 1.1.1.1  2000/04/18 08:51:23  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/

#include <kgi/maintainers.h>
#define	MAINTAINER		Johan_Karlberg
#define	KGIM_CHIPSET_DRIVER	"$Revision: 1.1.1.1 $"

#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	255
#endif


#define	__Matrox_Gx00
#include "chipset/Matrox/Gx00.h"
#include "chipset/Matrox/Gx00-meta.h"
#include "chipset/IBM/VGA.h"

/*	Low level native IO helpers
*/
static inline void mgag_gc_out8(mgag_chipset_io_t *mgag_io, kgi_u8_t val,
	kgi_u32_t reg) 
{
	KRN_DEBUG(3, "GC8: %.2x <- %.2x", reg, val); 
	mem_out8(val, mgag_io->control.base_virt + reg);
}

static inline kgi_u8_t mgag_gc_in8(mgag_chipset_io_t *mgag_io, kgi_u32_t reg) 
{
	kgi_u8_t val = mem_in8(mgag_io->control.base_virt + reg);
	KRN_DEBUG(3, "GC8: %.2x -> %.2x", reg, val);
	return val;
}

static inline void mgag_gc_out32(mgag_chipset_io_t *mgag_io, kgi_u32_t val,
	kgi_u32_t reg)
{
	KRN_DEBUG(3, "GC32: %.8x <- %.8x", reg, val);
	mem_out32(val, mgag_io->control.base_virt + reg);
}

static inline kgi_u32_t mgag_gc_in32(mgag_chipset_io_t *mgag_io, kgi_u32_t reg)
{
	kgi_u32_t val = mem_in32(mgag_io->control.base_virt + reg);
	KRN_DEBUG(3, "GC32: %.8x -> %.8x", reg, val);
	return val;
}

static inline void mgag_dac_out8(mgag_chipset_io_t *mgag_io, kgi_u8_t val,
	kgi_u32_t reg)
{
	KRN_DEBUG(3, "DAC8: %.2x <- %.2x", reg, val);
	mem_out8(val, mgag_io->control.base_virt + MGAG_RAMDAC + reg);
}

static inline kgi_u8_t mgag_dac_in8(mgag_chipset_io_t *mgag_io, kgi_u32_t reg)
{
	kgi_u32_t val = mem_in8(mgag_io->control.base_virt + MGAG_RAMDAC + reg);
	KRN_DEBUG(3, "DAC8: %.2x -> %.2x", reg, val);
	return val;
}

static inline void mgag_dac_outs8(mgag_chipset_io_t *mgag_io, kgi_u_t reg,
	void *buf, kgi_u_t cnt) 
{
	mem_outs8(mgag_io->control.base_virt + MGAG_RAMDAC + reg, buf, cnt);
}

static inline void mgag_dac_ins8(mgag_chipset_io_t *mgag_io, kgi_u_t reg,
	void *buf, kgi_u_t cnt) 
{
	mem_ins8(mgag_io->control.base_virt + MGAG_RAMDAC + reg, buf, cnt);
}

static inline void mgag_clk_out8(mgag_chipset_io_t *mgag_io, kgi_u8_t val,
	kgi_u_t reg)
{
	mem_out8((mem_in8(mgag_io->control.base_virt + MMIO_MISCr) & 
		~VGA_MISC_CLOCK_MASK) | ((val & 3) << 2),
		mgag_io->control.base_virt + MMIO_MISCw);
}

static inline kgi_u8_t mgag_clk_in8(mgag_chipset_io_t *mgag_io, kgi_u_t reg) 
{
	return (mem_in8(mgag_io->control.base_virt + MMIO_MISCr) & 
		VGA_MISC_CLOCK_MASK) >> 2;
}

/*	KGI interface
*/

kgi_error_t mgag_chipset_init_module(mgag_chipset_t *mgag, 
	mgag_chipset_io_t *mgag_io, const kgim_options_t *options)
{
	kgi_u_t		mgabase2_size = 0;

	kgi_u32_t	pci_mgabase1 = 0,
			pci_mgabase2 = 0;

	static const kgi_u32_t mgag_chipset_pcicfg[] = {

		PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_MYS),		PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_G200_PCI),
		PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_G200_PCI),
		PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_G200_AGP),
		PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_G400),
		PCICFG_SIGNATURE(0,0)
	};

	pcicfg_vaddr_t pcidev = options->pci->dev;

	KRN_DEBUG(2, "chipset_init_module() initialized");

	KRN_ASSERT(mgag);
	KRN_ASSERT(mgag_io);
	KRN_ASSERT(options);

	/* auto-detect/verify the chipset */

	if (pcidev == PCICFG_NULL) {

		if (pcicfg_find_device(&pcidev, mgag_chipset_pcicfg)) {

			KRN_ERROR("No supported device found.");

			return -KGI_ERRNO(CHIPSET, INVAL);
		}
	}

	kgim_memset(mgag, 0, sizeof(*mgag));

	kgim_strcpy(mgag->chipset.vendor, "Matrox Graphics Inc.");
	kgim_strcpy(mgag->chipset.model,  "1x64/Gx00 Chipset");

	mgag->chipset.revision  = KGIM_CHIPSET_REVISION;
	mgag->chipset.mode_size = sizeof(mgag_chipset_mode_t);

	mgag->chipset.vendor_id = pcicfg_in16(pcidev + PCI_VENDOR_ID);
	mgag->chipset.device_id = pcicfg_in16(pcidev + PCI_DEVICE_ID);

	mgag->pci.Command	= pcicfg_in32(pcidev + PCI_COMMAND);
	mgag->pci.LatTimer	= pcicfg_in32(pcidev + PCI_LATENCY_TIMER);
	mgag->pci.IntLine	= pcicfg_in32(pcidev + PCI_INTERRUPT_LINE);
	mgag->pci.BaseAddr0	= pcicfg_in32(pcidev + PCI_BASE_ADDRESS_0);
	mgag->pci.BaseAddr1	= pcicfg_in32(pcidev + PCI_BASE_ADDRESS_1);
	mgag->pci.BaseAddr2	= pcicfg_in32(pcidev + PCI_BASE_ADDRESS_2);

	switch PCICFG_SIGNATURE(mgag->chipset.vendor_id,
		mgag->chipset.device_id) {

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_MYS):

		mgag->chipset.maxdots.x = 2048;
		mgag->chipset.maxdots.y = 2048;

		mgabase2_size = M1x64_MGABASE2_SIZE;

		mgag->flags |= MGAG_CF_1x64 | MGAG_CF_SGRAM;

		switch(pcicfg_in8(pcidev + PCI_REVISION_ID)) {

		case 1:
			mgag->flags |= MGAG_CF_OLD;

			mgag->chipset.dclk.max = 135 MHZ;

			pci_mgabase1 = mgag->pci.BaseAddr0;
			pci_mgabase2 = mgag->pci.BaseAddr1;

			break;
		case 2:
			mgag->chipset.dclk.max = 175 MHZ;

			pci_mgabase1 = mgag->pci.BaseAddr1;
			pci_mgabase2 = mgag->pci.BaseAddr0;

			break;
		case 3:
			mgag->chipset.dclk.max = 220 MHZ;

			pci_mgabase1 = mgag->pci.BaseAddr1;
			pci_mgabase2 = mgag->pci.BaseAddr0;

			break;
		default:
			KRN_INTERNAL_ERROR;
			return -KGI_ERRNO(CHIPSET, NOSUP);
		}

		break;

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, 
		PCI_DEVICE_ID_MATROX_G200_PCI):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX,
		PCI_DEVICE_ID_MATROX_G200_AGP):

		mgag->chipset.maxdots.x = 4096;
		mgag->chipset.maxdots.y = 4096;

		mgag->flags |= MGAG_CF_G200;

		mgabase2_size = G200_MGABASE2_SIZE;

		pci_mgabase1 = mgag->pci.BaseAddr1;
		pci_mgabase2 = mgag->pci.BaseAddr0;

		mgag->chipset.dclk.max = (mgag->flags & MGAG_CF_SGRAM) 
			? 250 MHZ : 230 MHZ;
		break;

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_G400):

		mgag->chipset.maxdots.x = 4096;
		mgag->chipset.maxdots.y = 4096;

		mgag->flags |= MGAG_CF_G400;

		mgabase2_size = G400_MGABASE2_SIZE;

		pci_mgabase1 = mgag->pci.BaseAddr1;
		pci_mgabase2 = mgag->pci.BaseAddr0;

		mgag->chipset.dclk.max = 300 MHZ;

		break;

	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(CHIPSET, NOSUP);
	}

	/*	if MEMORY set, the board was initialized by the BIOS and 
	**	we needn't do memory init 
	*/
	if (mgag->pci.Command & PCI_COMMAND_MEMORY) {

		mgag->flags |= MGAG_CF_PRIMARY;
	}

	/* Initialize driver claimed regions */

	mgag_io->kgim.pcidev		= pcidev;

	mgag_io->control.name		= "1x64/Gx00 Control Aperture";
	mgag_io->control.device		= pcidev;
	mgag_io->control.base_virt	= MEM_NULL;
	mgag_io->control.base_io	= pci_mgabase1 & ~(MGAG_MGABASE1_SIZE - 1);
	mgag_io->control.size		= MGAG_MGABASE1_SIZE;
	mgag_io->control.decode		= MEM_DECODE_ALL;

	mgag_io->fb.name		= "1x64/Gx00 Framebuffer Aperture";
	mgag_io->fb.device		= pcidev;
	mgag_io->fb.base_virt		= MEM_NULL;
	mgag_io->fb.base_io		= pci_mgabase2 & ~(mgabase2_size - 1);
	mgag_io->fb.size		= mgabase2_size;
	mgag_io->fb.decode		= MEM_DECODE_ALL;

	mgag_io->iload.name		= "1x64/Gx00 ILOAD Aperture";
	mgag_io->iload.device		= pcidev;
	mgag_io->iload.base_virt	= MEM_NULL;
	mgag_io->iload.base_io		= mgag->pci.BaseAddr2 & ~(MGAG_MGABASE3_SIZE - 1);
	mgag_io->iload.size		= MGAG_MGABASE3_SIZE;
	mgag_io->iload.decode		= MEM_DECODE_ALL;

	mgag_io->irq.flags		= IF_SHARED_IRQ;
	mgag_io->irq.name		= "1x64/Gx00 interrupt line";
	mgag_io->irq.line		= mgag->pci.IntLine & 0xFF;
	mgag_io->irq.meta		= mgag;
	mgag_io->irq.meta_io		= mgag_io;
	mgag_io->irq.High		= (irq_handler_fn *) mgag_chipset_irq_handler;

	/*	dissallow option override of PCI bases, since they are 
	**	chipset/revision dependant 
	*/

	/*	Make sure the memory regions are free.
	*/
	if (	mem_check_region(&mgag_io->control)	||
		mem_check_region(&mgag_io->fb)		||
		mem_check_region(&mgag_io->iload)) {

		KRN_ERROR("check of 1x64/Gx00 memory regions failed.");

		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	/* claim the regions */
	
	mem_claim_region(&mgag_io->control);
	mem_claim_region(&mgag_io->fb);
	mem_claim_region(&mgag_io->iload);

	/* initializing the io helper struct. */

	mgag_io->kgim.DacOut8	= (void *) mgag_dac_out8;
	mgag_io->kgim.DacIn8	= (void *) mgag_dac_in8;
	mgag_io->kgim.DacOuts8	= (void *) mgag_dac_outs8;
	mgag_io->kgim.DacIns8	= (void *) mgag_dac_ins8;

	mgag_io->kgim.ClkOut8	= (void *) mgag_clk_out8;
	mgag_io->kgim.ClkIn8	= (void *) mgag_clk_in8;

	mgag_io->GCIn8		= (void *) mgag_gc_in8;
	mgag_io->GCOut8		= (void *) mgag_gc_out8;
	mgag_io->GCIn32		= (void *) mgag_gc_in32;
	mgag_io->GCOut32	= (void *) mgag_gc_out32;

 	KRN_NOTICE("%s %s driver " KGIM_CHIPSET_DRIVER, mgag->chipset.vendor, 
		mgag->chipset.model);

	KRN_DEBUG(2, "completed");

	return KGI_EOK;
}

void mgag_chipset_done_module(mgag_chipset_t *mgag, mgag_chipset_io_t *mgag_io,
	const kgim_options_t *options) 
{
	KRN_DEBUG(2, "entered");

	if (mgag->flags & MGAG_CF_IRQ_CLAIMED) {

		irq_free_line(&mgag_io->irq);
	}

	mem_free_region(&mgag_io->iload);
	mem_free_region(&mgag_io->fb);
	mem_free_region(&mgag_io->control);

	KRN_NOTICE("%s %s driver removed.", 
		mgag->chipset.vendor, mgag->chipset.model);

	KRN_DEBUG(2, "completed");
}

const kgim_meta_t mgag_chipset_meta = {

	(kgim_meta_init_module_fn *)	mgag_chipset_init_module,
	(kgim_meta_done_module_fn *)	mgag_chipset_done_module,
	(kgim_meta_init_fn *)		mgag_chipset_init,
	(kgim_meta_done_fn *)		mgag_chipset_done,
	(kgim_meta_mode_check_fn *)	mgag_chipset_mode_check,
	(kgim_meta_mode_resource_fn *)	mgag_chipset_mode_resource,
	(kgim_meta_mode_prepare_fn *)	NULL,
	(kgim_meta_mode_enter_fn *)	mgag_chipset_mode_enter,
	(kgim_meta_mode_leave_fn *)	mgag_chipset_mode_leave,

	sizeof(mgag_chipset_t),
	sizeof(mgag_chipset_io_t),
	sizeof(mgag_chipset_mode_t)
};

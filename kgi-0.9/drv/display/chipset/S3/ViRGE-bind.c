/* ----------------------------------------------------------------------------
**	S3 ViRGE chipset meta language binding
** ----------------------------------------------------------------------------
**	Copyright (C)	1999		Jon Taylor
**	Copyright (C)	1999-2000	Jos Hulzink
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: ViRGE-bind.c,v $
*/

#include <kgi/maintainers.h>
#define	DEBUG_LEVEL		2
#define	MAINTAINER		Jos_Hulzink
#define	KGIM_CHIPSET_DRIVER	"$Revision: 1.1 $"

#include <kgi/module.h>

#define	__S3_ViRGE
#include "chipset/S3/ViRGE.h"
#include "chipset/S3/ViRGE-meta.h"

#define VIRGE_MMIO_BASE		(virge_io->aperture.base_virt)+0x1000000
/*	#define	VIRGE_CONTROL_BASE	VIRGE_MMIO_BASE
*/
#define VIRGE_VGA_IO_BASE      	VGA_IO_BASE
#define VIRGE_VGA_MEM_BASE	(VIRGE_MMIO_BASE + 0x8000)
#define	VIRGE_VGA_DAC		(virge_io->flags & VIRGE_IF_VGA_DAC)

#define ViRGE_IO_CF_MMIO_ONLY 	0 /*FIXME*/

/*
**	ViRGE specific I/O functions
*/

static inline void virge_chipset_mmio_out16(virge_chipset_io_t *virge_io, 
	kgi_u16_t val, kgi_u32_t reg)
{
	KRN_DEBUG(3, "MMIO%.4x <- %.4x", reg, val);
	
	mem_out16(val, (VIRGE_MMIO_BASE + reg));
}

static inline kgi_u16_t virge_chipset_mmio_in16(virge_chipset_io_t *virge_io,
	kgi_u32_t reg)
{
	kgi_u16_t val;
	
	val = mem_in16(VIRGE_MMIO_BASE + reg);
	
	KRN_DEBUG(3, "MMIO%.4x -> %.4x", reg, val);
	
	return val;
}

static inline void virge_chipset_mmio_out32(virge_chipset_io_t *virge_io,
	kgi_u32_t val, kgi_u32_t reg)
{
	KRN_DEBUG(3, "MMIO%.4x <- %.8x", reg, val);
	
	mem_out32(val, (VIRGE_MMIO_BASE + reg));
}

static inline kgi_u32_t virge_chipset_mmio_in32(virge_chipset_io_t *virge_io,
	kgi_u32_t reg)
{
	kgi_u32_t val;
	
	val = mem_in32(VIRGE_MMIO_BASE + reg);
	
	KRN_DEBUG(3, "MMIO%.4x -> %.8x", reg, val);
	
	return val;
}

/*
**	VGA subsystem I/O functions
*/

static inline void virge_chipset_vga_seq_out8(virge_chipset_io_t *virge_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_SEQ%.2x <- %.2x", reg, val);
 	
	if (virge_io->flags & ViRGE_IO_CF_MMIO_ONLY) {

		mem_out8(reg, VIRGE_VGA_MEM_BASE + VGA_SEQ_INDEX);
		mem_out8(val, VIRGE_VGA_MEM_BASE + VGA_SEQ_DATA);

	} else {

		io_out8(reg, VIRGE_VGA_IO_BASE + VGA_SEQ_INDEX);
		io_out8(val, VIRGE_VGA_IO_BASE + VGA_SEQ_DATA);
	}
}

static inline kgi_u8_t virge_chipset_vga_seq_in8(virge_chipset_io_t *virge_io, 
	kgi_u_t reg)
{
	kgi_u8_t val;
	
/*	mem_out8(reg, VIRGE_VGA_MEM_BASE + VGA_SEQ_INDEX);	
**	return mem_in8(VIRGE_VGA_MEM_BASE + VGA_SEQ_DATA);
*/
	io_out8(reg, VIRGE_VGA_IO_BASE + VGA_SEQ_INDEX);
	val = io_in8(VIRGE_VGA_IO_BASE + VGA_SEQ_DATA);
	
	KRN_DEBUG(3, "VGA_SEQ%.2x -> %.2x", reg, val);
	
	return val;
}

static inline void virge_chipset_vga_crt_out8(virge_chipset_io_t *virge_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_CRT%.2x <- %.2x", reg, val);
	
/*	mem_out8(reg, VIRGE_VGA_MEM_BASE + VGA_CRT_INDEX);
**	mem_out8(val, VIRGE_VGA_MEM_BASE + VGA_CRT_DATA);
*/
	io_out8(reg, VIRGE_VGA_IO_BASE + VGA_CRT_INDEX);
	io_out8(val, VIRGE_VGA_IO_BASE + VGA_CRT_DATA);
}

static inline kgi_u8_t virge_chipset_vga_crt_in8(virge_chipset_io_t *virge_io,
	kgi_u_t reg)
{
	kgi_u8_t val;
	
/*	mem_out8(reg, VIRGE_VGA_BASE + VGA_CRT_INDEX);
**	return mem_in8(VIRGE_VGA_BASE + VGA_CRT_DATA);
*/
	io_out8(reg, VIRGE_VGA_IO_BASE + VGA_CRT_INDEX);
	val = io_in8(VIRGE_VGA_IO_BASE + VGA_CRT_DATA);
	
	KRN_DEBUG(3, "VGA_CRT%.2x -> %.2x", reg, val);
	
	return val;
}

static inline void virge_chipset_vga_grc_out8(virge_chipset_io_t *virge_io, 
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_GRC%.2x <- %.2x", reg, val);
	
/*	mem_out8(reg, VIRGE_VGA_BASE + VGA_GRC_INDEX);
**	mem_out8(val, VIRGE_VGA_BASE + VGA_GRC_DATA);
*/
	io_out8(reg, VIRGE_VGA_IO_BASE + VGA_GRC_INDEX);
	io_out8(val, VIRGE_VGA_IO_BASE + VGA_GRC_DATA);
}

static inline kgi_u8_t virge_chipset_vga_grc_in8(virge_chipset_io_t *virge_io, kgi_u_t reg)
{
	kgi_u8_t val;
	
/*	mem_out8(reg, VIRGE_VGA_BASE + VGA_GRC_INDEX);	
*/	return mem_in8(VIRGE_VGA_BASE + VGA_GRC_DATA);

	io_out8(reg, VIRGE_VGA_IO_BASE + VGA_GRC_INDEX);
	val = io_in8(VIRGE_VGA_IO_BASE + VGA_GRC_DATA);
	
	KRN_DEBUG(3, "VGA_GRC%.2x -> %.2x", reg, val);
	
	return val;
}

static inline void virge_chipset_vga_atc_out8(virge_chipset_io_t *virge_io, 
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_ATC%.2x <- %.2x", reg, val);
#if 0	
	mem_in8(VIRGE_VGA_BASE + VGA_ATC_AFF);
	mem_out8(reg, VIRGE_VGA_BASE + VGA_ATC_INDEX);
	mem_out8(val, VIRGE_VGA_BASE + VGA_ATC_DATAw);
	mem_out8(VGA_ATCI_ENABLE_DISPLAY, VIRGE_VGA_BASE + VGA_ATC_INDEX);

	io_out8(reg, VIRGE_VGA_IO_BASE + VGA_ATC_INDEX);
	io_out8(val, VIRGE_VGA_IO_BASE + VGA_ATC_DATAw);
#endif
	
	io_in8(VIRGE_VGA_IO_BASE + VGA_ATC_AFF);
	io_out8(reg, VIRGE_VGA_IO_BASE + VGA_ATC_INDEX);
	io_out8(val, VIRGE_VGA_IO_BASE + VGA_ATC_DATAw);
	io_out8(VGA_ATCI_ENABLE_DISPLAY, VIRGE_VGA_IO_BASE + VGA_ATC_INDEX);
}

static inline kgi_u8_t virge_chipset_vga_atc_in8(virge_chipset_io_t *virge_io, 
	kgi_u_t reg)
{
	register kgi_u8_t val;
#if 0	
	mem_in8(VIRGE_VGA_BASE + VGA_ATC_AFF);
	mem_out8(reg, VIRGE_VGA_BASE + VGA_ATC_INDEX);
	val = mem_in8(VIRGE_VGA_BASE + VGA_ATC_DATAr);
	mem_out8(VGA_ATCI_ENABLE_DISPLAY, VIRGE_VGA_BASE + VGA_ATC_INDEX);
#endif
	io_in8(VIRGE_VGA_IO_BASE + VGA_ATC_AFF);
	io_out8(reg, VIRGE_VGA_IO_BASE + VGA_ATC_INDEX);
	val = io_in8(VIRGE_VGA_IO_BASE + VGA_ATC_DATAr);
	io_out8(VGA_ATCI_ENABLE_DISPLAY, VIRGE_VGA_IO_BASE + VGA_ATC_INDEX);

	KRN_DEBUG(3, "VGA_ATC%.2x -> %.2x", reg, val);
	
	return val;
}

static inline void virge_chipset_vga_misc_out8(virge_chipset_io_t *virge_io, 
	kgi_u8_t val)
{
	KRN_DEBUG(3, "VGA_MISC <- %.2x", val);
	
/*	mem_out8(val, VIRGE_VGA_BASE + VGA_MISCw);
*/
	io_out8(val, VIRGE_VGA_IO_BASE + VGA_MISCw);
}

static inline kgi_u8_t virge_chipset_vga_misc_in8(virge_chipset_io_t *virge_io)
{
	kgi_u8_t val;
	
/*	return mem_in8(VIRGE_VGA_BASE + VGA_MISCr);
*/
	val = io_in8(VIRGE_VGA_IO_BASE + VGA_MISCr);
	KRN_DEBUG(3, "VGA_MISC -> %.2x", val);
	
	return val;
}

static inline void virge_chipset_vga_fctrl_out8(virge_chipset_io_t *virge_io, 
	kgi_u8_t val)
{
	KRN_DEBUG(3, "VGA_FCTRL <- %.2x", val);
	
/*	mem_out8(val, VIRGE_VGA_BASE + VGA_FCTRLw);
*/
	io_out8(val, VIRGE_VGA_IO_BASE + VGA_FCTRLw);
}

static inline kgi_u8_t virge_chipset_vga_fctrl_in8(virge_chipset_io_t *virge_io)
{
	kgi_u8_t val;
	
/*	return mem_in8(VIRGE_VGA_BASE + VGA_FCTRLr);
*/
	val = io_in8(VIRGE_VGA_IO_BASE + VGA_FCTRLr);
	
	KRN_DEBUG(3, "VGA_FCTRL -> %.2x", val);
	
	return val;
}

#if 1
/* DAC subsystem I/O */
static const kgi_u_t virge_vga_dac_register[4] = { 0x08, 0x09, 0x06, 0x07 };

#define	VIRG_SET_DAC_ADDR23					  	\
	mem_out8(0x05, VIRGE_VGA_BASE + VGA_SEQ_INDEX);		  	\
	mem_out8((mem_in8(VIRGE_VGA_BASE + VGA_SEQ_DATA) &	 	\
		~VIRGE_SR05_DACAddrMask) | ((reg << 2) & VIRGE_SR05_DACAddrMask),	\
		VIRGE_VGA_BASE + VGA_SEQ_DATA)

static inline void virge_chipset_dac_out8(virge_chipset_io_t *virge_io, kgi_u8_t val, kgi_u_t reg)
{
/*	KRN_ASSERT(reg < VIRGE_MAX_DAC_REGISTERS);
*/
	if (VIRGE_VGA_DAC) {

		KRN_DEBUG(3, "DAC_IO %.2x <- %.2x", reg, val);
		
/*		virge_SET_DAC_ADDR23;
**		mem_out8(val, VIRGE_VGA_BASE + VIRGE_vga_dac_register[reg & 3]);
*/		io_out8(val, VIRGE_VGA_IO_BASE + virge_vga_dac_register[reg & 3]);
	} else {

		KRN_DEBUG(3, "DAC_MEM %.2x <- %.2x", reg, val);
/*		mem_out8(val, VIRGE_VGA_MMIO_BASE + VIRGE_DAC_BASE + (reg << 3));
*/
	}
}

static inline kgi_u8_t virge_chipset_dac_in8(virge_chipset_io_t *virge_io, 
	kgi_u_t reg)
{
/*	KRN_ASSERT(reg < VIRGE_MAX_DAC_REGISTERS);
*/
	if (VIRGE_VGA_DAC) {

		register kgi_u8_t val;

/*		VIRGE_SET_DAC_ADDR23;
*/
		
/*		val = mem_in8(VIRGE_VGA_BASE + virge_vga_dac_register[reg & 3]);
*/		val = io_in8(VIRGE_VGA_IO_BASE + virge_vga_dac_register[reg & 3]);
		
		KRN_DEBUG(3, "DAC_IO %.2x -> %.2x", reg, val);
		
		return val;

	} else {

		register kgi_u8_t val = 0xff;

/*		val = mem_in8(VIRGE_VGA_MMIO_BASE + VIRGE_DAC_BASE + (reg << 3));
*/		
		KRN_DEBUG(3, "DAC_MEM %.2x -> %.2x", reg, val);
		
		return val;
	}
}

static inline void virge_chipset_dac_outs8(virge_chipset_io_t *virge_io, 
	kgi_u_t reg, void *buf, kgi_u_t cnt)
{
#warning fix this
#if 0
	KRN_ASSERT(reg < VIRGE_MAX_DAC_REGISTERS);

	if (VIRGE_VGA_DAC) {

		KRN_DEBUG(3, "DAC_VGA %.2x <- (%i bytes)", reg, cnt);
		
		VIRGE_SET_DAC_ADDR23;
		
		mem_outs8(VIRGE_VGA_BASE + virge_vga_dac_register[reg & 3], buf, cnt);
	} else {

		KRN_DEBUG(3, "dac_mem %.2x <- (%i bytes)", reg, cnt);
		
		mem_outs8(VIRGE_MMIO_BASE + VIRGE_DAC_BASE + (reg << 3), 
			buf, cnt);
	}
#endif
}

static inline void virge_chipset_dac_ins8(virge_chipset_io_t *virge_io, 
	kgi_u_t reg, void *buf, kgi_u_t cnt)
{
#warning fix this!
#if 0
	KRN_ASSERT(reg < VIRGE_MAX_DAC_REGISTERS);

	if (VIRGE_VGA_DAC) {

		KRN_DEBUG(3, "DAC_VGA %.2x -> (%i bytes)", reg, cnt);
		
		VIRGE_SET_DAC_ADDR23;
		
		mem_ins8(VIRGE_VGA_BASE + VIRGE_vga_dac_register[reg & 3], buf, cnt);
		io_ins8(VIRGE_VGA_IO_BASE + virge_vga_dac_register[reg & 3], buf, cnt);
	} else {

		KRN_DEBUG(3, "DAC_MEM %.2x -> (%i bytes)", reg, cnt);
		
		mem_ins8(VIRGE_MMIO_BASE + VIRGE_DAC_BASE + (reg << 3), buf, cnt);
	}
#endif
}

/*	Clock control
*/
static inline void virge_chipset_clk_out8(virge_chipset_io_t *virge_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_ASSERT(reg == 0);

	if (VIRGE_VGA_DAC) {

		KRN_DEBUG(3, "VGA_CLK%.2x <= %.2x", reg, val);
		
		io_out8((io_in8(VIRGE_VGA_IO_BASE + VGA_MISCr) & 
			~VGA_MISC_CLOCK_MASK) | ((val & 3) << 2), 
			VIRGE_VGA_IO_BASE + VGA_MISCw);
	} else {
#warning fix this!
#if 0
		register kgi_u32_t CS040 = mem_in32(VIRGE_CONTROL_BASE + 
			virge_ControlStatusBase + 0x040);

		mem_out32((CS040 & ~VIRGE_CS040_VClkMask) | 
			(val & VIRGE_CS040_VClkMask), 
			VIRGE_CONTROL_BASE + VIRGE_ControlStatusBase + 0x040);
#endif
	}
}

static inline kgi_u8_t virge_chipset_clk_in8(virge_chipset_io_t *virge_io, 
	kgi_u_t reg)
{
	KRN_ASSERT(reg == 0);

	if (VIRGE_VGA_DAC) {

		return (io_in8(VIRGE_VGA_IO_BASE + VGA_MISCr) & 
			VGA_MISC_CLOCK_MASK) >> 2;
	} else {

/*		return mem_in32(VIRGE_CONTROL_BASE + VIRGE_ControlStatusBase + 0x040) & VIRGE_CS040_VClkMask;
*/
	}
	
	return 0;
}
#endif

kgi_error_t virge_chipset_init_module(virge_chipset_t *virge, 
	virge_chipset_io_t *virge_io, const kgim_options_t *options)
{
	static const kgi_u32_t virge_chipset_pcicfg[] =
        {
#warning enable here what you truely intend to support.
                PCICFG_SIGNATURE(PCI_VENDOR_ID_S3, PCI_DEVICE_ID_S3_ViRGE),
                PCICFG_SIGNATURE(PCI_VENDOR_ID_S3, PCI_DEVICE_ID_S3_ViRGE_VX),
                PCICFG_SIGNATURE(PCI_VENDOR_ID_S3, PCI_DEVICE_ID_S3_ViRGE_DXGX),
                PCICFG_SIGNATURE(PCI_VENDOR_ID_S3, PCI_DEVICE_ID_S3_ViRGE_GX2),
                PCICFG_SIGNATURE(PCI_VENDOR_ID_S3, PCI_DEVICE_ID_S3_ViRGE_MX),
                PCICFG_SIGNATURE(PCI_VENDOR_ID_S3, PCI_DEVICE_ID_S3_ViRGE_MXP),
		PCICFG_SIGNATURE(PCI_VENDOR_ID_S3, PCI_DEVICE_ID_S3_ViRGE_MXPMV),
                PCICFG_SIGNATURE(0,0)
        };
	pcicfg_vaddr_t pcidev = options->pci->dev;
	kgi_u16_t subvendor, subdevice;
	
	KRN_DEBUG(2, "virge_chipset_init_module()");

	KRN_ASSERT(virge);
	KRN_ASSERT(virge_io);
	KRN_ASSERT(options);

	/*	Auto-detect/verify the chipset
	*/
	if (pcidev == PCICFG_NULL) {		

		if (pcicfg_find_device(&pcidev, virge_chipset_pcicfg)) {

			KRN_DEBUG(2, "No supported device found!");
			
			return -E(CHIPSET,INVAL);
		}
	}
	
	kgim_memset(virge, 0, sizeof(*virge));
	
	virge->chipset.revision		= KGIM_CHIPSET_REVISION;
	virge->chipset.mode_size	= sizeof(virge_chipset_mode_t);
	virge->chipset.vendor_id	= pcicfg_in16(pcidev + PCI_VENDOR_ID);
	virge->chipset.device_id	= pcicfg_in16(pcidev + PCI_DEVICE_ID);

	switch (PCICFG_SIGNATURE(virge->chipset.vendor_id,
		virge->chipset.device_id)) {

        case PCICFG_SIGNATURE(PCI_VENDOR_ID_S3, PCI_DEVICE_ID_S3_ViRGE):
		kgim_strcpy(virge->chipset.vendor, "S3");
		kgim_strcpy(virge->chipset.model,  "ViRGE");
		virge->chipset.maxdots.x = 2048;
		virge->chipset.maxdots.y = 2048;
                virge->chipset.dclk.max = 135000000;
/*		virge->chipset.mclk.max = 50000000;
*/		break;

        case PCICFG_SIGNATURE(PCI_VENDOR_ID_S3, PCI_DEVICE_ID_S3_ViRGE_VX):
                kgim_strcpy(virge->chipset.vendor, "S3");
                kgim_strcpy(virge->chipset.model,  "ViRGE VX");
                virge->chipset.maxdots.x = 2048;
                virge->chipset.maxdots.y = 2048;
                virge->chipset.dclk.max = 135000000;
/*		virge->chipset.mclk.max = 50000000;
*/		break;

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_S3, PCI_DEVICE_ID_S3_ViRGE_DXGX):
                kgim_strcpy(virge->chipset.vendor, "S3");
                kgim_strcpy(virge->chipset.model,  "ViRGE GX / DX");
                virge->chipset.maxdots.x = 2048;
                virge->chipset.maxdots.y = 2048;
                virge->chipset.dclk.max = 170000000;
/*		virge->chipset.mclk.max = 66000000;
*/		break;

        case PCICFG_SIGNATURE(PCI_VENDOR_ID_S3, PCI_DEVICE_ID_S3_ViRGE_GX2):
                kgim_strcpy(virge->chipset.vendor, "S3");
                kgim_strcpy(virge->chipset.model,  "ViRGE GX2");
                virge->chipset.maxdots.x = 2048;
                virge->chipset.maxdots.y = 2048;
                virge->chipset.dclk.max = 135000000;
/*              virge->chipset.mclk.max = 66000000;
*/		break;

        case PCICFG_SIGNATURE(PCI_VENDOR_ID_S3, PCI_DEVICE_ID_S3_ViRGE_MX):
                kgim_strcpy(virge->chipset.vendor, "S3");
                kgim_strcpy(virge->chipset.model,  "ViRGE MX");
                virge->chipset.maxdots.x = 2048;
                virge->chipset.maxdots.y = 2048;
                virge->chipset.dclk.max = 135000000;
/*		virge->chipset.mclk.max = 50000000;
*/		break;

        case PCICFG_SIGNATURE(PCI_VENDOR_ID_S3, PCI_DEVICE_ID_S3_ViRGE_MXP):
                kgim_strcpy(virge->chipset.vendor, "S3");
                kgim_strcpy(virge->chipset.model,  "ViRGE MXP");
                virge->chipset.maxdots.x = 2048;
                virge->chipset.maxdots.y = 2048;
                virge->chipset.dclk.max = 135000000;
/*		virge->chipset.mclk.max = 50000000;
*/		break;
 
        case PCICFG_SIGNATURE(PCI_VENDOR_ID_S3, PCI_DEVICE_ID_S3_ViRGE_MXPMV):
                kgim_strcpy(virge->chipset.vendor, "S3");
                kgim_strcpy(virge->chipset.model,  "ViRGE MXPMV");
                virge->chipset.maxdots.x = 2048;
                virge->chipset.maxdots.y = 2048;
                virge->chipset.dclk.max = 135000000;
/*		virge->chipset.mclk.max = 50000000;
*/		break;

        default:
                KRN_ERROR("Device not yet supported (vendor %.4x device %.4x).",
                        virge->chipset.vendor_id, virge->chipset.device_id);
                return -E(CHIPSET, NOSUP);
        }

	subvendor = pcicfg_in16(pcidev + PCI_SUBSYSTEM_VENDOR_ID);
	subdevice = pcicfg_in16(pcidev + PCI_SUBSYSTEM_ID);
	KRN_DEBUG(2, "subvendor %.4x, subdevice %.4x", subvendor, subdevice);

        /*	save initial PCICFG state
	*/
        virge->pci.Command      = pcicfg_in16(pcidev + PCI_COMMAND);
        virge->pci.LatTimer     = pcicfg_in8(pcidev + PCI_LATENCY_TIMER);
        virge->pci.IntLine      = pcicfg_in8(pcidev + PCI_INTERRUPT_LINE);
        virge->pci.BaseAddr0    = pcicfg_in32(pcidev + PCI_BASE_ADDRESS_0);
        virge->pci.BaseAddr1    = pcicfg_in32(pcidev + PCI_BASE_ADDRESS_1);
        virge->pci.BaseAddr2    = pcicfg_in32(pcidev + PCI_BASE_ADDRESS_2);
        virge->pci.BaseAddr3    = pcicfg_in32(pcidev + PCI_BASE_ADDRESS_3);
        virge->pci.BaseAddr4    = pcicfg_in32(pcidev + PCI_BASE_ADDRESS_4);
        virge->pci.RomAddr      = pcicfg_in32(pcidev + PCI_ROM_ADDRESS);

	/*	Initialize driver claimed regions and I/O binding
	*/
	virge_io->vga.kgim.pcidev = pcidev;

	virge_io->aperture.name		= "ViRGE aperture";
	virge_io->aperture.device	= pcidev;
	virge_io->aperture.base_virt	= MEM_NULL;
	virge_io->aperture.base_io	= virge->pci.BaseAddr0 & ~(VIRGE_Base0_Size - 1);
	virge_io->aperture.size		= VIRGE_Base0_Size;
	virge_io->aperture.decode	= MEM_DECODE_ALL;

	virge_io->irq.flags = IF_SHARED_IRQ;
	virge_io->irq.name = "ViRGE interrupt line";
	virge_io->irq.line = pcicfg_in8(pcidev + PCI_INTERRUPT_LINE);
	virge_io->irq.meta = virge;
	virge_io->irq.meta_io = virge_io;
	virge_io->irq.High = (irq_handler_fn *) virge_chipset_irq_handler;

	virge_io->vga.ports.name       	= "ViRGE VGA IO";
	virge_io->vga.ports.device	= pcidev;
	virge_io->vga.ports.base_virt	= IO_NULL;
	virge_io->vga.ports.base_io	= VGA_IO_BASE;
	virge_io->vga.ports.size       	= VGA_IO_SIZE;
	
	virge_io->vga.aperture.name      = "ViRGE VGA text aperture";
/*	virge_io->vga.aperture.device    = NULL;
*/	virge_io->vga.aperture.base_virt = MEM_NULL;
	virge_io->vga.aperture.base_io   = VGA_TEXT_MEM_BASE;
	virge_io->vga.aperture.size      = VGA_TEXT_MEM_SIZE;
	virge_io->vga.aperture.decode    = MEM_DECODE_ALL;
	
	/*	Make sure no other driver is serving the chipset
	*/
	if (virge->pci.Command & PCI_COMMAND_IO) {

		if (io_check_region(&virge_io->vga.ports)) {

			KRN_ERROR("%s region served (maybe another driver?).", 
				virge_io->vga.ports);
			return -E(CHIPSET, INVAL);
		}
	}

	if (mem_check_region(&virge_io->vga.aperture)) {

		KRN_ERROR("%s region already served!", 
			virge_io->vga.aperture.name);
		return -E(CHIPSET, INVAL);
	}
	
	if (virge->pci.Command & PCI_COMMAND_MEMORY) {

		if (mem_check_region(&virge_io->aperture)) {

			KRN_ERROR("%s region already served!", 
				virge_io->aperture.name);
			return -E(CHIPSET, INVAL);
		}
		
	}

	/*	If root specified new base addresses, he knows the
	**	consequences. If not, it's not our fault...
	*/
#define	SET_BASE(region, addr, size)					\
	if (addr) {							\
		region.base_io = addr & ~(size - 1);			\
		KRN_DEBUG(1, "region.base_io = %.8x", region.base_io);	\
	}

	SET_BASE(virge_io->aperture, options->pci->base0, VIRGE_Base0_Size);
/*	SET_BASE(virge_io->framebuffer, options->pci->base1, 2 * 1024 * 1024);
*/
#undef SET_BASE
	
	/*	Make sure the memory regions are free
	*/
	if (mem_check_region(&virge_io->aperture)) {

		KRN_ERROR("Check of ViRGE PCI LAW region failed!");
		return -E(CHIPSET, INVAL);
	}

	if (mem_check_region(&virge_io->vga.aperture)) {

		KRN_ERROR("Check of ViRGE VGA memory region failed!");
		return -E(CHIPSET, INVAL);
	}
	
	/*	Claim the regions 
	*/
	io_claim_region(&virge_io->vga.ports);
	mem_claim_region(&virge_io->vga.aperture);
	mem_claim_region(&virge_io->aperture);

	if (KGI_EOK == irq_claim_line(&virge_io->irq)) {

		KRN_DEBUG(2, "Interrupt line claimed successfully");
		virge->flags |= VIRGE_CF_IRQ_CLAIMED;
	}

	virge_io->vga.kgim.DacOut8  = (void *) virge_chipset_dac_out8;
	virge_io->vga.kgim.DacIn8   = (void *) virge_chipset_dac_in8;
	virge_io->vga.kgim.DacOuts8 = (void *) virge_chipset_dac_outs8;
	virge_io->vga.kgim.DacIns8  = (void *) virge_chipset_dac_ins8;

	virge_io->vga.kgim.ClkOut8  = (void *) virge_chipset_clk_out8;
	virge_io->vga.kgim.ClkIn8   = (void *) virge_chipset_clk_in8;
	
	virge_io->vga.SeqOut8   = (void *) virge_chipset_vga_seq_out8;
	virge_io->vga.SeqIn8    = (void *) virge_chipset_vga_seq_in8;
	virge_io->vga.CrtOut8   = (void *) virge_chipset_vga_crt_out8;
	virge_io->vga.CrtIn8    = (void *) virge_chipset_vga_crt_in8;
	virge_io->vga.GrcOut8   = (void *) virge_chipset_vga_grc_out8;
	virge_io->vga.GrcIn8    = (void *) virge_chipset_vga_grc_in8;
	virge_io->vga.AtcOut8   = (void *) virge_chipset_vga_atc_out8;
	virge_io->vga.AtcIn8    = (void *) virge_chipset_vga_atc_in8;
	virge_io->vga.MiscOut8  = (void *) virge_chipset_vga_misc_out8;
	virge_io->vga.MiscIn8   = (void *) virge_chipset_vga_misc_in8;
	virge_io->vga.FctrlOut8 = (void *) virge_chipset_vga_fctrl_out8;
	virge_io->vga.FctrlIn8  = (void *) virge_chipset_vga_fctrl_in8;

	virge_io->mmio_out16	= (void *) virge_chipset_mmio_out16;
	virge_io->mmio_in16	= (void *) virge_chipset_mmio_in16;
	
	virge_io->mmio_out32	= (void *) virge_chipset_mmio_out32;
	virge_io->mmio_in32	= (void *) virge_chipset_mmio_in32;
	
	KRN_NOTICE("%s %s driver " KGIM_CHIPSET_DRIVER, 
		virge->chipset.vendor, virge->chipset.model);
	
	return KGI_EOK;
}

void virge_chipset_done_module(virge_chipset_t *virge, 
	virge_chipset_io_t *virge_io, const kgim_options_t *options)
{
	KRN_DEBUG(2, "virge_chipset_done_module()");
	
	if (virge->flags & VIRGE_CF_IRQ_CLAIMED) {

		irq_free_line(&virge_io->irq);
	}

	mem_free_region(&virge_io->aperture);
}

const kgim_meta_t virge_chipset_meta =
{
	(kgim_meta_init_module_fn *)	virge_chipset_init_module,
	(kgim_meta_done_module_fn *)	virge_chipset_done_module,
	(kgim_meta_init_fn *)		virge_chipset_init,
	(kgim_meta_done_fn *)		virge_chipset_done,
	(kgim_meta_mode_check_fn *)	virge_chipset_mode_check,
	(kgim_meta_mode_resource_fn *)	virge_chipset_mode_resource,
	(kgim_meta_mode_prepare_fn *)	virge_chipset_mode_prepare,
	(kgim_meta_mode_enter_fn *)	virge_chipset_mode_enter,
	(kgim_meta_mode_leave_fn *)	virge_chipset_mode_leave,

	sizeof(virge_chipset_t),
	sizeof(virge_chipset_io_t),
	sizeof(virge_chipset_mode_t)
};

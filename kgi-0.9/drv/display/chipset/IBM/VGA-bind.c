/* ----------------------------------------------------------------------------
**	IBM VGA chipset meta language binding
** ----------------------------------------------------------------------------
**	Copyright	1999-2000	Jon Taylor
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
#define	KGIM_CHIPSET_DRIVER	"$Revision: 1.2 $"

#define	DEBUG_LEVEL	2

#include <kgi/module.h>

#define	__IBM_VGA
#include "chipset/IBM/VGA.h"
#include "chipset/IBM/VGA-meta.h"

/*	VGA subsystem I/O functions
*/
static inline void vga_chipset_seq_out8(vga_chipset_io_t *vga_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_SEQ%.2x <- %.2x", reg, val);
	
	io_out8(reg, VGA_IO_BASE + VGA_SEQ_INDEX);
	io_out8(val, VGA_IO_BASE + VGA_SEQ_DATA);
}

static inline kgi_u8_t vga_chipset_seq_in8(vga_chipset_io_t *vga_io,
	kgi_u_t reg)
{
	kgi_u8_t val;
	
	io_out8(reg, VGA_IO_BASE + VGA_SEQ_INDEX);
	val = io_in8(VGA_IO_BASE + VGA_SEQ_DATA);
	
	KRN_DEBUG(3, "VGA_SEQ%.2x -> %.2x", reg, val);
	
	return val;
}

static inline void vga_chipset_crt_out8(vga_chipset_io_t *vga_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_CRT%.2x <- %.2x", reg, val);
	
	io_out8(reg, VGA_IO_BASE + VGA_CRT_INDEX);
	io_out8(val, VGA_IO_BASE + VGA_CRT_DATA);
}

static inline kgi_u8_t vga_chipset_crt_in8(vga_chipset_io_t *vga_io,
	kgi_u_t reg)
{
	kgi_u8_t val;
	
	io_out8(reg, VGA_IO_BASE + VGA_CRT_INDEX);
	val = io_in8(VGA_IO_BASE + VGA_CRT_DATA);
	
	KRN_DEBUG(3, "VGA_CRT%.2x -> %.2x", reg, val);
	
	return val;
}

static inline void vga_chipset_grc_out8(vga_chipset_io_t *vga_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_GRC%.2x <- %.2x", reg, val);
	
	io_out8(reg, VGA_IO_BASE + VGA_GRC_INDEX);
	io_out8(val, VGA_IO_BASE + VGA_GRC_DATA);
}

static inline kgi_u8_t vga_chipset_grc_in8(vga_chipset_io_t *vga_io,
	kgi_u_t reg)
{
	kgi_u8_t val;
	
	io_out8(reg, VGA_IO_BASE + VGA_GRC_INDEX);
	val = io_in8(VGA_IO_BASE + VGA_GRC_DATA);
	
	KRN_DEBUG(3, "VGA_GRC%.2x -> %.2x", reg, val);
	
	return val;
}

static inline void vga_chipset_atc_out8(vga_chipset_io_t *vga_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_ATC%.2x <- %.2x", reg, val);
	
	io_in8(VGA_IO_BASE + VGA_ATC_AFF);
	io_out8(reg, VGA_IO_BASE + VGA_ATC_INDEX);
	io_out8(val, VGA_IO_BASE + VGA_ATC_DATAw);
	io_in8(VGA_IO_BASE + VGA_ATC_AFF);
	io_out8(VGA_ATCI_ENABLE_DISPLAY, VGA_IO_BASE + VGA_ATC_INDEX);
}

static inline kgi_u8_t vga_chipset_atc_in8(vga_chipset_io_t *vga_io,
	kgi_u_t reg)
{
	register kgi_u8_t val;
	
	io_in8(VGA_IO_BASE + VGA_ATC_AFF);
	io_out8(reg, VGA_IO_BASE + VGA_ATC_INDEX);
	val = io_in8(VGA_IO_BASE + VGA_ATC_DATAr);
	io_in8(VGA_IO_BASE + VGA_ATC_AFF);
	io_out8(VGA_ATCI_ENABLE_DISPLAY, VGA_IO_BASE + VGA_ATC_INDEX);

	KRN_DEBUG(3, "VGA_ATC%.2x -> %.2x", reg, val);
	
	return val;
}

static inline void vga_chipset_misc_out8(vga_chipset_io_t *vga_io, kgi_u8_t val)
{
	KRN_DEBUG(3, "VGA_MISC <- %.2x", val);
	
	io_out8(val, VGA_IO_BASE + VGA_MISCw);
}

static inline kgi_u8_t vga_chipset_misc_in8(vga_chipset_io_t *vga_io)
{
	kgi_u8_t val;
	
	val = io_in8(VGA_IO_BASE + VGA_MISCr);
	
	KRN_DEBUG(3, "VGA_MISC -> %.2x", val);
	
	return val;
}

static inline void vga_chipset_fctrl_out8(vga_chipset_io_t *vga_io,
	kgi_u8_t val)
{
	KRN_DEBUG(3, "VGA_FCTRL <- %.2x", val);
	
	io_out8(val, VGA_IO_BASE + VGA_FCTRLw);
}

static inline kgi_u8_t vga_chipset_fctrl_in8(vga_chipset_io_t *vga_io)
{
	kgi_u8_t val;
	
	val = io_in8(VGA_IO_BASE + VGA_FCTRLr);
	
	KRN_DEBUG(3, "VGA_FCTRL -> %.2x", val);
	
	return val;
}

/*	DAC subsystem I/O
*/
static const kgi_u_t vga_dac_register[4] = { 0x08, 0x09, 0x06, 0x07 };

static inline void vga_chipset_dac_out8(vga_chipset_io_t *vga_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_DAC %.2x <- %.2x", reg, val);
		
	io_out8(val, VGA_IO_BASE + vga_dac_register[reg & 3]);
}

static inline kgi_u8_t vga_chipset_dac_in8(vga_chipset_io_t *vga_io,
	kgi_u_t reg)
{
	kgi_u8_t val;

	val = io_in8(VGA_IO_BASE + vga_dac_register[reg & 3]);
		
	KRN_DEBUG(3, "VGA_DAC %.2x -> %.2x", reg, val);
		
	return val;
}

static inline void vga_chipset_dac_outs8(vga_chipset_io_t *vga_io,
	kgi_u_t reg, void *buf, kgi_u_t cnt)
{
	KRN_DEBUG(3, "VGA_DAC %.2x <- (%i bytes)", reg, cnt);
	
	io_outs8(VGA_IO_BASE + vga_dac_register[reg & 3], buf, cnt);
}

static inline void vga_chipset_dac_ins8(vga_chipset_io_t *vga_io,
	kgi_u_t reg, void *buf, kgi_u_t cnt)
{
	KRN_DEBUG(3, "VGA_DAC %.2x -> (%i bytes)", reg, cnt);
	
	io_ins8(VGA_IO_BASE + vga_dac_register[reg & 3], buf, cnt);
}

/*	Clock Control
*/

static inline void vga_chipset_clk_out8(vga_chipset_io_t *vga_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_CLK%.2x <- %.2x", reg, val);
	
	io_out8((io_in8(VGA_IO_BASE + VGA_MISCr) & ~VGA_MISC_CLOCK_MASK) |
		((val & 3) << 2), VGA_IO_BASE + VGA_MISCw);
}

static inline kgi_u8_t vga_chipset_clk_in8(vga_chipset_io_t *vga_io,
	kgi_u_t reg)
{
	kgi_u8_t val;
	
	val = (io_in8(VGA_IO_BASE + VGA_MISCr) & VGA_MISC_CLOCK_MASK) >> 2;
	
	KRN_DEBUG(3, "VGA_CLK%.2x -> %.2x", val);
	
	return val;
}


kgi_error_t vga_chipset_init_module(vga_chipset_t *vga,
	vga_chipset_io_t *vga_io, const kgim_options_t *options)
{
	KRN_DEBUG(2, "vga_chipset_init_module()");

	KRN_ASSERT(vga);
	KRN_ASSERT(vga_io);
	KRN_ASSERT(options);

	vga_io->ports.name     	= "VGA IO";
/*	vga_io->ports.device	= pcidev;
**	vga_io->ports.device	= NULL;
*/	vga_io->ports.base_virt	= IO_NULL;
	vga_io->ports.base_io	= VGA_IO_BASE;
	vga_io->ports.size     	= VGA_IO_SIZE;
	
	vga_io->text16fb.name      = "VGA text16 aperture";
/*	vga_io->text16fb.device    = NULL;
*/	vga_io->text16fb.base_virt = MEM_NULL;
	vga_io->text16fb.base_io   = VGA_TEXT_MEM_BASE;
	vga_io->text16fb.size      = VGA_TEXT_MEM_SIZE;
	vga_io->text16fb.decode    = MEM_DECODE_ALL;
	
	vga_io->pixelfb.name      = "VGA pixelfb aperture";
/*	vga_io->pixelfb.device    = NULL;
*/	vga_io->pixelfb.base_virt = MEM_NULL;
	vga_io->pixelfb.base_io   = VGA_MEM_BASE;
	vga_io->pixelfb.size      = VGA_MEM_SIZE;
	vga_io->pixelfb.decode    = MEM_DECODE_ALL;
	
	/*	Make sure no other driver is serving the chipset 
	*/
	if (io_check_region(&vga_io->ports)) {

		KRN_ERROR("%s region served (maybe another driver?).",
			vga_io->ports);
		return -E(CHIPSET, INVAL);
	}

	if (mem_check_region(&vga_io->text16fb)) {

		KRN_ERROR("%s region already served!",
			vga_io->text16fb.name);
		return -E(CHIPSET, INVAL);
	}
	
	if (mem_check_region(&vga_io->pixelfb)) {

		KRN_ERROR("%s region already served!", vga_io->pixelfb.name);
		return -E(CHIPSET, INVAL);
	}
	
	/*	Claim the regions
	*/
	io_claim_region(&vga_io->ports);
	mem_claim_region(&vga_io->text16fb);
	mem_claim_region(&vga_io->pixelfb);

	vga_io->kgim.DacOut8  = (void *) vga_chipset_dac_out8;
	vga_io->kgim.DacIn8   = (void *) vga_chipset_dac_in8;
	vga_io->kgim.DacOuts8 = (void *) vga_chipset_dac_outs8;
	vga_io->kgim.DacIns8  = (void *) vga_chipset_dac_ins8;
	vga_io->kgim.ClkOut8  = (void *) vga_chipset_clk_out8;
	vga_io->kgim.ClkIn8   = (void *) vga_chipset_clk_in8;
	vga_io->SeqOut8   = (void *) vga_chipset_seq_out8;
	vga_io->SeqIn8    = (void *) vga_chipset_seq_in8;
	vga_io->CrtOut8   = (void *) vga_chipset_crt_out8;
	vga_io->CrtIn8    = (void *) vga_chipset_crt_in8;
	vga_io->GrcOut8   = (void *) vga_chipset_grc_out8;
	vga_io->GrcIn8    = (void *) vga_chipset_grc_in8;
	vga_io->AtcOut8   = (void *) vga_chipset_atc_out8;
	vga_io->AtcIn8    = (void *) vga_chipset_atc_in8;
	vga_io->MiscOut8  = (void *) vga_chipset_misc_out8;
	vga_io->MiscIn8   = (void *) vga_chipset_misc_in8;
	vga_io->FctrlOut8 = (void *) vga_chipset_fctrl_out8;
	vga_io->FctrlIn8  = (void *) vga_chipset_fctrl_in8;

	KRN_NOTICE("%s %s driver " KGIM_CHIPSET_DRIVER,
		vga->chipset.vendor, vga->chipset.model);
	
	return KGI_EOK;
}

void vga_chipset_done_module(vga_chipset_t *vga, vga_chipset_io_t *vga_io,
	const kgim_options_t *options)
{
	KRN_DEBUG(2, "vga_chipset_done_module()");
	
/*	mem_free_region(&vga_io->text16fb);
*/	mem_free_region(&vga_io->pixelfb);
}

const kgim_meta_t vga_chipset_meta =
{
	(kgim_meta_init_module_fn *)	vga_chipset_init_module,
	(kgim_meta_done_module_fn *)	vga_chipset_done_module,
	(kgim_meta_init_fn *)		vga_chipset_init,
	(kgim_meta_done_fn *)		vga_chipset_done,
	(kgim_meta_mode_check_fn *)	vga_chipset_mode_check,
	(kgim_meta_mode_resource_fn *)	vga_chipset_mode_resource,
	(kgim_meta_mode_prepare_fn *)	vga_chipset_mode_prepare,
	(kgim_meta_mode_enter_fn *)	vga_chipset_mode_enter,
	(kgim_meta_mode_leave_fn *)	NULL,

	sizeof(vga_chipset_t),
	sizeof(vga_chipset_io_t),
	sizeof(vga_chipset_mode_t)
};

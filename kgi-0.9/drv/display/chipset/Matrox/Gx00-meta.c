/* ----------------------------------------------------------------------------
**	Gx00 chipset driver meta
** ----------------------------------------------------------------------------
**	Copyright (C) 1999-2000         Johan Karlberg
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: Gx00-meta.c,v $
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		Johan_Karlberg
#define	KGIM_CHIPSET_DRIVER	"$Revision: 1.3 $"

#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	255
#endif

#define	__Matrox_Gx00
#include "chipset/Matrox/Gx00.h"
#include "chipset/Matrox/Gx00-meta.h"
#include "chipset/IBM/VGA.h"

#warning	You must implement these in the binding driver. 
#warning	All uppercase is for macros only.
#warning 	A meta language prefix of Gx00, not mgag seems more suitable.

/* Legacy emulation */

static inline void MGAG_MISC_OUT8(mgag_chipset_io_t *mgag_io, kgi_u8_t val)
{
	KRN_DEBUG(3, "Misc out %.2x", val);

	MGAG_GC_OUT8(mgag_io, val, MMIO_MISCw);
}

static inline kgi_u8_t MGAG_MISC_IN8(mgag_chipset_io_t *mgag_io) 
{
	KRN_DEBUG(3, "Misc in");

	return MGAG_GC_IN8(mgag_io, MMIO_MISCr);
}

static inline void MGAG_SEQ_OUT8(mgag_chipset_io_t *mgag_io, kgi_u8_t val,
	kgi_u8_t reg)
{
	KRN_DEBUG(3, "SEQ out %.2x to %.2x", val, reg);

	MGAG_GC_OUT8(mgag_io, reg, MMIO_SEQi);
	MGAG_GC_OUT8(mgag_io, val, MMIO_SEQd);
}

static inline kgi_u8_t MGAG_SEQ_IN8(mgag_chipset_io_t *mgag_io, kgi_u8_t reg) 
{
	KRN_DEBUG(3, "SEQ in from %.2x", reg);

	MGAG_GC_OUT8(mgag_io, reg, MMIO_SEQi);
	return MGAG_GC_IN8(mgag_io, MMIO_SEQd);
}

static inline void MGAG_EDAC_OUT8(mgag_chipset_io_t *mgag_io, kgi_u8_t val,
	kgi_u8_t reg)
{
	KRN_DEBUG(3, "EDAC out %.2x to %.2x", val, reg);

	MGAG_GC_OUT8(mgag_io, reg, MGAG_RAMDAC + PALWTADD);
	MGAG_GC_OUT8(mgag_io, val, MGAG_RAMDAC + X_DATAREG);
}

static inline kgi_u8_t MGAG_EDAC_IN8(mgag_chipset_io_t *mgag_io, kgi_u8_t reg)
{
	KRN_DEBUG(3, "EDAC in from %.2x", reg);

	MGAG_GC_OUT8(mgag_io, reg,  MGAG_RAMDAC + PALWTADD);
	return MGAG_GC_IN8(mgag_io, MGAG_RAMDAC + X_DATAREG);
}

static inline void MGAG_CRT_OUT8(mgag_chipset_io_t *mgag_io, kgi_u8_t val,
	kgi_u8_t reg)
{
	KRN_DEBUG(3, "CRT out %.2x to %.2x", val, reg);

	MGAG_GC_OUT8(mgag_io, reg, MMIO_CRTCi);
	MGAG_GC_OUT8(mgag_io, val, MMIO_CRTCd);
}

static inline kgi_u8_t MGAG_CRT_IN8(mgag_chipset_io_t *mgag_io, kgi_u8_t reg)
{
	KRN_DEBUG(3, "CRT in from %.2x", reg);

	MGAG_GC_OUT8(mgag_io, reg, MMIO_CRTCi);
	return MGAG_GC_IN8(mgag_io, MMIO_CRTCd);
}

static inline void MGAG_ECRT_OUT8(mgag_chipset_io_t *mgag_io, kgi_u8_t val,
	kgi_u8_t reg)
{
	KRN_DEBUG(3, "ECRT out %.2x to %.2x", val, reg);

	MGAG_GC_OUT8(mgag_io, reg, MMIO_CRTCEXTi);
	MGAG_GC_OUT8(mgag_io, val, MMIO_CRTCEXTd);
}

static inline kgi_u8_t MGAG_ECRT_IN8(mgag_chipset_io_t *mgag_io, kgi_u8_t reg)
{
	KRN_DEBUG(3, "ECRT in from %.2x", reg);

	MGAG_GC_OUT8(mgag_io, reg, MMIO_CRTCEXTi);
	return MGAG_GC_IN8(mgag_io, MMIO_CRTCEXTd);
}

inline kgi_error_t mgag_chipset_irq_handler(mgag_chipset_t *mgag, 
	mgag_chipset_io_t *mgag_io, irq_system_t *system)
{
	pcicfg_vaddr_t pcidev = MGAG_PCIDEV(mgag_io);

	kgi_u32_t iclear = 0,
		  flags  = MGAG_GC_IN32(mgag_io, STATUS);

	KRN_ASSERT(mgag);
	KRN_ASSERT(mgag_io);

	KRN_DEBUG(4, "chipset IRQ handler initiated.");

	if (flags & STATUS_PICKPEN) {

		iclear |= ICLEAR_PICKPICLR;
		KRN_TRACE(0, mgag->interrupt.pick++);
		KRN_DEBUG(1, "Pick interrupt (pcidev %.8x)", pcidev);
	}

	if (flags & STATUS_VSYNCPEN) {

		MGAG_CRT_OUT8(mgag_io, MGAG_CRT_IN8(mgag_io, VGA_CRT_VSYNCEND) & ~VGA_CR11_CLEAR_VSYNC_IRQ, VGA_CRT_VSYNCEND);

		KRN_TRACE(0, mgag->interrupt.vsync++);
		KRN_DEBUG(1, "Vertical Sync interrupt (pcidev %.8x)", pcidev);
	}

	if (flags & STATUS_VLINEPEN) {

		iclear |= ICLEAR_VLINEICLR;
		KRN_TRACE(0, mgag->interrupt.vline++);
		KRN_DEBUG(1, "1st CRTC Vertical line interrupt (pcidev %.8x)", pcidev);
	}

	if (flags & STATUS_EXTPEN) {

		/* Uncleared, because I don't know how, probably device dependent */

		KRN_TRACE(0, mgag->interrupt.ext++);
		KRN_DEBUG(1, "External interrupt (pcidev %.8x)", pcidev);
	}

	if ((mgag->flags & MGAG_CF_G200) || (mgag->flags & MGAG_CF_G400)) {

		if (flags & STATUS_SOFTRAPEN) {

			iclear |= ICLEAR_SOFTRAPICLR;
			KRN_TRACE(0, mgag->interrupt.softtrap++);
			KRN_DEBUG(1, "soft trap interrupt (pcidev %.8x)", pcidev);
		}

		if (flags & STATUS_WPEN) {

			iclear |= ICLEAR_WICLR;
			KRN_TRACE(0, mgag->interrupt.warp++);
			KRN_DEBUG(1, "WARP pipe 0 interrupt (pcidev %.8x)", pcidev);
		}

		if (flags & STATUS_WCPEN) {

			iclear |= ICLEAR_WCICLR;
			KRN_TRACE(0, mgag->interrupt.warpcache++);
			KRN_DEBUG(1, "WARP pipe 0 cache interrupt (pcidev %.8x)", pcidev);
		}

		if (mgag->flags & MGAG_CF_G400) {

			if (flags & STATUS_C2VLINEPEN) {

				iclear |= ICLEAR_C2VLINEICLR;
				KRN_TRACE(0, mgag->interrupt.c2vline++);
				KRN_DEBUG(1, "2nd CRTC Vertical line interrupt (pcidev %.8x)", pcidev);
			}

			if (flags & STATUS_WPEN1) {

				iclear |= ICLEAR_WICLR1;
				KRN_TRACE(0, mgag->interrupt.warp1++);
				KRN_DEBUG(1, "WARP pipe 1 interrupt (pcidev %.8x)", pcidev);
			}

			if (flags & STATUS_WCPEN1) {

				iclear |= ICLEAR_WCICLR1;
				KRN_TRACE(0, mgag->interrupt.warpcache1++);
				KRN_DEBUG(1, "WARP pipe 1 cache interrupt (pcidev %.8x)", pcidev);
			}
		}
	}

	MGAG_GC_OUT32(mgag_io, iclear, ICLEAR);

	KRN_DEBUG(5, "IRQ recieved/handled differential: %.8x", (flags - iclear) - STATUS_VSYNCPEN);

	KRN_DEBUG(4, "chipset IRQ handler completed.");

	return KGI_EOK;
}

static inline kgi_u_t mgag_chipset_memory_count(mgag_chipset_t *mgag,
	mgag_chipset_io_t *mgag_io)
{
	kgi_u_t  ret   = 0;
	kgi_u8_t ecrt3 = MGAG_ECRT_IN8(mgag_io, ECRT3);

	MGAG_ECRT_OUT8(mgag_io, ecrt3 | ECRT3_MGAMODE, ECRT3);

	if (mgag->flags & MGAG_CF_G400) {

		mem_out8(0x99, mgag_io->fb.base_virt + 31 MB);
		ret = mem_in8( mgag_io->fb.base_virt + 31 MB) == 0x99 ? 32 : 16;

	} else if (mgag->flags & MGAG_CF_G200) {

		mem_out8(0x99, mgag_io->fb.base_virt + 15 MB);
		ret = mem_in8( mgag_io->fb.base_virt + 15 MB) == 0x99 ? 16 : 8;

	} else if (mgag->flags & MGAG_CF_1x64) {

		mem_out8(0x99, mgag_io->fb.base_virt + 5 MB);
		mem_out8(0x88, mgag_io->fb.base_virt + 3 MB);
		ret = mem_in8( mgag_io->fb.base_virt + 5 MB) == 0x99 ? 6 :
		      mem_in8( mgag_io->fb.base_virt + 3 MB) == 0x88 ? 4 : 2;
	} else {

		KRN_INTERNAL_ERROR;
	}

	MGAG_ECRT_OUT8(mgag_io, ecrt3, ECRT3);

	return ret MB;
}

static inline void mgag_chipset_clocks_init(mgag_chipset_t *mgag,
	mgag_chipset_io_t *mgag_io)
{
	pcicfg_vaddr_t pcidev = MGAG_PCIDEV(mgag_io);

	static chipset_config_s chipset_config[3] = {

		{	/* 1x64 */
			0x04, 0x44, 0x18,	/* M, N, P */

			(9 <<	M1x64_O1_RFHCNT_SHIFT)		|
			(7 <<	M1x64_O1_FBMASKN_SHIFT)		|
				M1x64_O1_SYSCLKSEL_SYS		|
				MGAG_O1_HARDPWMSK		|
				MGAG_O1_SYSCLKDIS,

			0,

			0
		},
		{	/* G200 */
			0x04, 0x2d, 0x19,	/* M, N, P */

			(19 <<	G200_O1_RFHCNT_SHIFT)		|
			(3  <<	G200_O1_MEMCONFIG_SHIFT)	|
				G200_O1_SYSCLKSEL_SYS		|
				G200_O1_ENHMEMACC		|
				MGAG_O1_SYSCLKDIS,

			G200_O2_NOMCLKDIV,

			0
		},
		{	/* G400 */
			0x09, 0x3c, 0x10,	/* M, N, P */

			(8 <<	G400_O1_RFHCNT_SHIFT)		|
				G400_O1_NOHIREQ			|
				MGAG_O1_SYSCLKDIS,

			G400_O2_CODCLKSEL_SYS,

			(3 <<	G400_O3_GCLKDIV_SHIFT)		|
				G400_O3_GCLKSEL_SYS		|
			(5 <<	G400_O3_MCLKDIV_SHIFT)		|
				G400_O3_MCLKSEL_SYS		|
			(3 <<	G400_O3_WCLKDIV_SHIFT)		|
				G400_O3_WCLKSEL_SYS
		}
	};

	chipset_config_s *config =	(mgag->flags & MGAG_CF_1x64) ? &chipset_config[0] :
					(mgag->flags & MGAG_CF_G200) ? &chipset_config[1] :
					(mgag->flags & MGAG_CF_G400) ? &chipset_config[2] : 0;

	kgi_u32_t	t_option1 = 0;

	kgi_u_t		cnt = PLL_DELAY;

	KRN_ASSERT(mgag);
	KRN_ASSERT(mgag_io);
	KRN_ASSERT(config);

	KRN_DEBUG(2, "chipset_clocks_init() initiated");

	t_option1 = pcicfg_in32(pcidev + MGAG_PCI_OPTION1);

	pcicfg_out32(t_option1 | MGAG_O1_SYSCLKDIS, pcidev + MGAG_PCI_OPTION1);

	if (mgag->flags & MGAG_CF_1x64) {

		t_option1 &= ~M1x64_O1_SYSCLKSEL_MASK;
		t_option1 |=  M1x64_O1_SYSCLKSEL_PCI;
		                                
	} else if (mgag->flags & MGAG_CF_G200) {

		t_option1 &= ~G200_O1_SYSCLKSEL_MASK;
		t_option1 |=  G200_O1_SYSCLKSEL_PCI;

	} else if (mgag->flags & MGAG_CF_G400) {

		pcicfg_out32(	G400_O3_GCLKSEL_PCI |
				G400_O3_MCLKSEL_PCI |
				G400_O3_WCLKSEL_PCI, pcidev + MGAG_PCI_OPTION3);
	}

	pcicfg_out32(t_option1 & ~MGAG_O1_SYSCLKDIS, pcidev + MGAG_PCI_OPTION1);

	MGAG_EDAC_OUT8(mgag_io, config->sysm, XSYSPLLM);
	MGAG_EDAC_OUT8(mgag_io, config->sysn, XSYSPLLN);
	MGAG_EDAC_OUT8(mgag_io, config->sysp, XSYSPLLP);

	while (cnt-- && !(MGAG_EDAC_IN8(mgag_io, XSYSPLLSTAT) & XSYSPLLSTAT_SYSLOCK));

	KRN_ASSERT(cnt);

	pcicfg_out32(t_option1 | MGAG_O1_SYSCLKDIS, pcidev + MGAG_PCI_OPTION1);

	if ((mgag->flags & MGAG_CF_G400) || (mgag->flags & MGAG_CF_G200)) {

		pcicfg_out32(config->option2, pcidev + MGAG_PCI_OPTION2);

		if (mgag->flags & MGAG_CF_G400) {

			pcicfg_out32(config->option3, pcidev + MGAG_PCI_OPTION3);
		}
	}

	pcicfg_out32(config->option1 & ~MGAG_O1_SYSCLKDIS, pcidev + MGAG_PCI_OPTION1);

	KRN_DEBUG(2, "chipset_clocks_init completed");
}

static inline void mgag_chipset_memory_init(mgag_chipset_t *mgag, 
	mgag_chipset_io_t *mgag_io, const kgim_options_t *options)
{
	pcicfg_vaddr_t pcidev = MGAG_PCIDEV(mgag_io);

	kgi_u32_t	mctlwtst  = 0,
			memconfig = 0,
			memreset  = 0,
			rfhcnt    = 0;

	KRN_DEBUG(2, "chipset system memory initialization initiated.");

	/*
		NOTE1:	System clocks need to be running here already.
		NOTE2:	All delays are set to maximum values for now.
			we need to dig up better values later
	*/

	return;

	switch(PCICFG_SIGNATURE(mgag->chipset.vendor_id, mgag->chipset.device_id)) {

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_MYS):

		mctlwtst  = 	M1x64_MCTLWTST_CASLTCNY		|
				M1x64_MCTLWTST_RCDELAY		|
				M1x64_MCTLWTST_RASMIN_MASK;

		memconfig =	0;
		memreset  =	MACCESS_MEMRESET | MACCESS_JEDECRST;
		rfhcnt    =	9 << M1x64_O1_RFHCNT_SHIFT;

		break;
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_G200_PCI):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_G200_AGP):

		mctlwtst =	Gx00_MCTLWTST_CASLTCNY_MASK	|
				Gx00_MCTLWTST_RRDDELAY_MASK	|
				Gx00_MCTLWTST_RCDDELAY_MASK	|
				Gx00_MCTLWTST_RASMIN_MASK	|
				Gx00_MCTLWTST_RPDELAY_MASK	|
				Gx00_MCTLWTST_WRDELAY_MASK	|
				Gx00_MCTLWTST_RDDELAY		|
				Gx00_MCTLWTST_SMRDELAY_MASK	|
				Gx00_MCTLWTST_BWCDELAY_MASK	|
				Gx00_MCTLWTST_BPLDELAY_MASK;

		memconfig =	 3 << G200_O1_MEMCONFIG_SHIFT;
		memreset  =	MACCESS_MEMRESET;
		rfhcnt    =	19 << G200_O1_RFHCNT_SHIFT;

		break;
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_G400):

		mctlwtst =	Gx00_MCTLWTST_CASLTCNY_MASK	|
				Gx00_MCTLWTST_RRDDELAY_MASK	|
				Gx00_MCTLWTST_RCDDELAY_MASK	|
				Gx00_MCTLWTST_RASMIN_MASK	|
				Gx00_MCTLWTST_RPDELAY_MASK	|
				Gx00_MCTLWTST_WRDELAY_MASK	|
				Gx00_MCTLWTST_RDDELAY		|
				Gx00_MCTLWTST_SMRDELAY_MASK	|
				Gx00_MCTLWTST_BWCDELAY_MASK	|
				Gx00_MCTLWTST_BPLDELAY_MASK;

		memconfig =	0;
		memreset  =	MACCESS_MEMRESET;
		rfhcnt    =	8 << G400_O1_RFHCNT_SHIFT;

		break;
	}

	MGAG_GC_OUT32(mgag_io, mctlwtst, MCTLWTST);

	pcicfg_out32(pcicfg_in32(pcidev + MGAG_PCI_OPTION1) | memconfig, pcidev + MGAG_PCI_OPTION1);

	switch(PCICFG_SIGNATURE(mgag->chipset.vendor_id, mgag->chipset.device_id)) {

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_MYS):

/*		udelay(200);*/

		MGAG_GC_OUT32(mgag_io, MACCESS_MEMRESET, MACCESS);

		break;
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_G200_PCI):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_G200_AGP):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_G400):

		MGAG_GC_OUT32(mgag_io, 0, MEMRDBK);

		break;
	}

/*	udelay(200);*/

	MGAG_GC_OUT32(mgag_io, memreset, MACCESS);

	pcicfg_out32(pcicfg_in32(pcidev + MGAG_PCI_OPTION1) | rfhcnt, pcidev + MGAG_PCI_OPTION1);

	KRN_DEBUG(2, "chipset system memory initialization completed.");
}

kgi_error_t mgag_chipset_init(mgag_chipset_t *mgag, mgag_chipset_io_t *mgag_io,
	const kgim_options_t *options)
{
	pcicfg_vaddr_t pcidev = MGAG_PCIDEV(mgag_io);

	KRN_ASSERT(mgag);
	KRN_ASSERT(mgag_io);
	KRN_ASSERT(options);

 	KRN_DEBUG(2, "chipset_init() initiated.");
 	KRN_DEBUG(2, "initializing %s %s", mgag->chipset.vendor,
		mgag->chipset.model);

#define	PCICFG_SET_BASE(value, reg)		\
		pcicfg_out32(0xFFFFFFFF, reg);	\
		pcicfg_in32(reg);		\
		pcicfg_out32((value), reg)

	if (mgag->flags & MGAG_CF_OLD) {

		PCICFG_SET_BASE(mgag_io->control.base_io,
			pcidev + PCI_BASE_ADDRESS_0);
		PCICFG_SET_BASE(mgag_io->fb.base_io,
			pcidev + PCI_BASE_ADDRESS_1);
	} else {

		PCICFG_SET_BASE(mgag_io->control.base_io,
			pcidev + PCI_BASE_ADDRESS_1);
		PCICFG_SET_BASE(mgag_io->fb.base_io,
			pcidev + PCI_BASE_ADDRESS_0);
	}

	PCICFG_SET_BASE(mgag_io->iload.base_io,	pcidev + PCI_BASE_ADDRESS_2);

#undef	PCICFG_SET_BASE

	/* initialize PCI command register */

	pcicfg_out16(PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER, pcidev + PCI_COMMAND);

	/* only change clocks and memory if we are secondary card */	
	/* FIXME: need to set clocks/options on _all_ cards.*/

/*	mgag_chipset_clocks_init(mgag, mgag_io);

	if (!(mgag->flags & MGAG_CF_PRIMARY)) {

		mgag_chipset_memory_init(mgag, mgag_io, options);
	}
*/
	mgag->chipset.memory = mgag_chipset_memory_count(mgag, mgag_io);

	KRN_DEBUG(1, "%i bytes framebuffer detected.", mgag->chipset.memory);

	if (mgag->flags & MGAG_CF_IRQ_CLAIMED) {

		KRN_DEBUG(2, "IRQ initialization skipped");
	}

	KRN_DEBUG(2, "chipset enabled");

	return KGI_EOK;
}

void mgag_chipset_done(mgag_chipset_t *mgag, mgag_chipset_io_t *mgag_io,
	const kgim_options_t *options)
{
	pcicfg_vaddr_t pcidev = MGAG_PCIDEV(mgag_io);

	if (mgag->flags & MGAG_CF_IRQ_CLAIMED) {

		MGAG_GC_OUT32(mgag_io, 0, IEN);
	}
}

kgi_error_t mgag_chipset_mode_check(mgag_chipset_t *mgag,
	mgag_chipset_io_t *mgag_io, mgag_chipset_mode_t *mgag_mode,
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	/*	most of the first part of this functiion is a big mess, 
	**	and completely incorrect, but somehow it compiles..
	*/

	kgi_dot_port_mode_t *dpm = img->out;
	const kgim_monitor_mode_t *crt_mode = mgag_mode->kgim.crt;

	kgi_u_t	shift	= 0,
		bpf	= 0,
		bpc	= 0,
		bpp	= 0,
		pgm	= 0,
		width	= 0,
		lclk	= 0,
		pp[3]	= { 0, 0, 0 };

	kgi_mmio_region_t *r;
	kgi_u_t mul, div, bpd;

	if (images != 1) {

		KRN_DEBUG(1, "%i images not supported.", images);
		return -E(CHIPSET, NOSUP);
	}

	/* for unsupported image flags, bail out. */

	if (img[0].flags & (KGI_IF_TILE_X | KGI_IF_TILE_Y | KGI_IF_VIRTUAL))
	{
		KRN_DEBUG(1, "image flags %.8x not supported", img[0].flags);
		return -E(CHIPSET, INVAL);
	}

	/* check if common attributes are supported. */

	switch (img[0].cam) {

	case 0:
		break;

	default:
		KRN_DEBUG(1, "common attributes %.8x not supported", img[0].cam);
		return -E(CHIPSET, INVAL);
	}

	/* total bits per dot */

	bpf = kgim_attr_bits(img[0].bpfa);
	bpc = kgim_attr_bits(img[0].bpca);
	bpd = kgim_attr_bits(dpm->bpda);
	bpp = (img[0].flags & KGI_IF_STEREO) ?
		(bpc + bpf*img[0].frames*2) :
		(bpc + bpf*img[0].frames);

	shift = 0;

	switch (bpd) {

	case  1:	shift++;	/* fall through	*/
	case  2:	shift++;	/* fall through */
	case  4:	shift++;	/* fall through	*/
	case  8:	shift++;	/* fall through	*/
	case 16:	shift++;	/* fall through	*/
	case 32:	shift++;
			pgm = (pgm << shift) - 1;
			break;

	default:	KRN_DEBUG(0, "%i bpd not supported", bpd);
			return -E(CHIPSET, FAILED);
	}

	lclk = (cmd == KGI_TC_PROPOSE) ? 0 : dpm->dclk * dpm->lclk.mul / dpm->lclk.div;

	switch (cmd) {

	case KGI_TC_PROPOSE:

		KRN_ASSERT(img[0].frames);
		KRN_ASSERT(bpp);

		/* if size.x or size.y are zero, default to 640x400 */

		if ((0 == img[0].size.x) || (0 == img[0].size.y)) {

			img[0].size.x = 640;
			img[0].size.y = 400;
		}

		/* if virt.x and virt.y are zero, default to size. if either virt.x xor virt.y is zero, maximize the other */

		if ((0 == img[0].virt.x) && (0 == img[0].virt.y)) {

			img[0].virt.x = img[0].size.x;
			img[0].virt.y = img[0].size.y;
		}

		if (0 == img[0].virt.x)	{

			img[0].virt.x = (8 * mgag->chipset.memory) / (img[0].virt.y * bpp);

			if (img[0].virt.x > mgag->chipset.maxdots.x) {

				img[0].virt.x = mgag->chipset.maxdots.x;
			}
		}

		if (0 == img[0].virt.y)	{

			img[0].virt.y = (8 * mgag->chipset.memory) / (img[0].virt.x * bpp);
		}

		/* Are we beyond the limits of the H/W?	*/

		if ((img[0].size.x >= mgag->chipset.maxdots.x) || (img[0].virt.x >= mgag->chipset.maxdots.x)) {

			KRN_DEBUG(1, "%i (%i) x pixels are too many", img[0].size.x, img[0].virt.x);

			return -E(CHIPSET, UNKNOWN);
		}

		if ((img[0].size.y >= mgag->chipset.maxdots.y) || (img[0].virt.y >= mgag->chipset.maxdots.y)) {

			KRN_DEBUG(1, "%i (%i) y pixels are too many", img[0].size.y, img[0].virt.y);

			return -E(CHIPSET, UNKNOWN);
		}

		if ((img[0].virt.x * img[0].virt.y * bpp) > (8 * mgag->chipset.memory)) {

			KRN_DEBUG(1, "not enough memory (%ipf*%if + %ipc)@%ix%i", bpf, img[0].frames, bpc, img[0].virt.x, img[0].virt.y);

			return -E(CHIPSET,NOMEM);
		}

		/* set CRT visible fields */

		dpm->dots.x = img[0].size.x;
		dpm->dots.y = img[0].size.y;

		if (img[0].size.y < 400) {

			dpm->dots.y += img[0].size.y;
		}

		return KGI_EOK;

	case KGI_TC_LOWER:
	case KGI_TC_RAISE:

		if (cmd == KGI_TC_LOWER) {

			if (dpm->dclk < mgag->chipset.dclk.min) {

				KRN_DEBUG(1, "DCLK = %i Hz is too low", dpm->dclk);
				return -E(CHIPSET, UNKNOWN);
			}
		}
		return KGI_EOK;

	case KGI_TC_CHECK:

		KRN_ASSERT(pp[0] < 8);
		KRN_ASSERT(pp[1] < 8);
		KRN_ASSERT(pp[2] < 8);

		if (width != img[0].virt.x) {

			return -E(CHIPSET, INVAL);
		}

		if ((img[0].size.x >= mgag->chipset.maxdots.x) ||
		    (img[0].size.y >= mgag->chipset.maxdots.y) ||
		    (img[0].virt.x >= mgag->chipset.maxdots.x) ||
		   ((img[0].virt.y * img[0].virt.x * bpp) > (8 * mgag->chipset.memory))) {

			KRN_DEBUG(1, "resolution too high: %ix%i (%ix%i)",
					img[0].size.x,
					img[0].size.y,
					img[0].virt.x,
					img[0].virt.y);
			return -E(CHIPSET, INVAL);
		}

		break;

	default:
		KRN_INTERNAL_ERROR;
		return -E(CHIPSET, UNKNOWN);
	}

	/* Now everything is checked and should be sane. proceed to setup device dependent mode. */

	bpd = kgim_attr_bits(dpm->bpda);

	mgag_mode->Misc	   = ((crt_mode->x.polarity > 0) ? 0 : VGA_MISC_NEG_HSYNC |
			      (crt_mode->y.polarity > 0) ? 0 : VGA_MISC_NEG_VSYNC | MISC_CLOCK_1X | VGA_MISC_COLOR_IO);

	mgag_mode->Offset  = ((img[0].virt.x * bpp) / 128);

	mgag_mode->HTotal  = (crt_mode->x.total     / 8) - 5;
	mgag_mode->HdEnd   = (crt_mode->x.width     / 8) - 1;
	mgag_mode->HsStart = (crt_mode->x.syncstart / 8);
	mgag_mode->HsEnd   = (crt_mode->x.syncend   / 8);
	mgag_mode->HbStart = (crt_mode->x.width     / 8) - 1;
	mgag_mode->HbEnd   = (crt_mode->x.total     / 8) - 4;

	mgag_mode->VTotal  = crt_mode->y.total - 2;
	mgag_mode->VdEnd   = crt_mode->y.width - 1;
	mgag_mode->VsStart = crt_mode->y.syncstart;
	mgag_mode->VsEnd   = crt_mode->y.syncend;
	mgag_mode->VbStart = crt_mode->y.width - 1;
	mgag_mode->VbEnd   = crt_mode->y.total - 1;

	mgag_mode->CRTC[0x00] =   mgag_mode->HTotal  & 0xFF;
	mgag_mode->CRTC[0x01] =   mgag_mode->HdEnd;
	mgag_mode->CRTC[0x02] =   mgag_mode->HbStart & 0xFF;
	mgag_mode->CRTC[0x03] =  (mgag_mode->HbEnd   & 0x1F) | VGA_CR03_IS_VGA;
	mgag_mode->CRTC[0x04] =   mgag_mode->HsStart & 0xFF;
	mgag_mode->CRTC[0x05] =  (mgag_mode->HsEnd   & 0x1F) |
				((mgag_mode->HbEnd   & 0x20) << 2);
	mgag_mode->CRTC[0x06] =   mgag_mode->VTotal  & 0xFF;
	mgag_mode->CRTC[0x07] = ((mgag_mode->VTotal  & 0x100) >> 8) |
				((mgag_mode->VdEnd   & 0x100) >> 7) |
				((mgag_mode->VsStart & 0x100) >> 6) |
				((mgag_mode->VbStart & 0x100) >> 5) |
				((mgag_mode->LnComp  & 0x100) >> 4) |
				((mgag_mode->VTotal  & 0x200) >> 4) |
				((mgag_mode->VdEnd   & 0x200) >> 3) |
				((mgag_mode->VsStart & 0x200) >> 2);

	mgag_mode->CRTC[0x08] = 0;
	mgag_mode->CRTC[0x09] = ((mgag_mode->LnComp  & 0x200) >> 3) |
				((mgag_mode->VbStart & 0x200) >> 4);

	mgag_mode->CRTC[0x10] =   mgag_mode->VsStart & 0xFF;
	mgag_mode->CRTC[0x11] =  (mgag_mode->VsEnd   & 0x0F) | VGA_CR11_DISABLE_VSYNC_IRQ;
	mgag_mode->CRTC[0x12] =   mgag_mode->VdEnd   & 0xFF;
	mgag_mode->CRTC[0x13] =   mgag_mode->Offset  & 0xFF;
	mgag_mode->CRTC[0x14] =   0;
	mgag_mode->CRTC[0x15] =   mgag_mode->VbStart & 0xFF;
	mgag_mode->CRTC[0x16] =   mgag_mode->VbEnd;
	mgag_mode->CRTC[0x17] =   0x80;
	mgag_mode->CRTC[0x18] =   mgag_mode->LnComp & 0xFF;

	mgag_mode->ECRTC[0]   =  (mgag_mode->Offset  & 0x300) >> 4;
	mgag_mode->ECRTC[1]   = ((mgag_mode->HTotal  & 0x100) >> 8) |
				((mgag_mode->HbStart & 0x100) >> 7) |
				((mgag_mode->HsStart & 0x100) >> 6) |
				 (mgag_mode->HbEnd   & 0x40);
	mgag_mode->ECRTC[2]   = ((mgag_mode->VTotal  & 0xC00) >> 10) |
				((mgag_mode->VdEnd   & 0x400) >> 8)  |
				((mgag_mode->VbStart & 0xC00) >> 7)  |
				((mgag_mode->VsStart & 0xC00) >> 5)  |
				((mgag_mode->LnComp  & 0x400) >> 3); 

	mgag_mode->ECRTC[3] = 0;
	mgag_mode->ECRTC[4] = 0;
	mgag_mode->ECRTC[5] = 0;

	/* I have no clue as to handle 6 and 7, so I don't. */

	/* initialize exported resources */

	r		= &mgag_mode->fb;
	r->meta		= mgag;
	r->meta_io	= mgag_io;
	r->type		= KGI_RT_MMIO_FRAME_BUFFER;
	r->prot		= KGI_PF_APP | KGI_PF_LIB | KGI_PF_DRV;
	r->name		= "MGA 1x64/Gx00 Framebuffer";
	r->access	= 64 + 32 + 16 + 8;
	r->align	= 64 + 32 + 16 + 8;
	r->size		= r->win.size = mgag->chipset.memory;
	r->win.bus	= mgag_io->fb.base_bus;
	r->win.phys	= mgag_io->fb.base_phys;
	r->win.virt	= mgag_io->fb.base_virt;

	r		= &mgag_mode->iload;
	r->meta		= mgag;
	r->meta_io	= mgag_io;
	r->type		= KGI_RT_MMIO_FRAME_BUFFER;
	r->prot		= KGI_PF_APP | KGI_PF_LIB | KGI_PF_DRV;
	r->name		= "MGA 1x64/Gx00 Framebuffer ILOAD aperture";
	r->access	= 64 + 32 + 16 + 8;
	r->align	= 64 + 32 + 16 + 8;
	r->size		= r->win.size = mgag->chipset.memory;
	r->win.bus	= mgag_io->iload.base_bus;
	r->win.phys	= mgag_io->iload.base_phys;
	r->win.virt	= mgag_io->iload.base_virt;

	r		= &mgag_mode->control;
	r->meta		= mgag;
	r->meta_io	= mgag_io;
	r->type		= KGI_RT_MMIO_PRIVATE;
	r->prot		= KGI_PF_LIB | KGI_PF_DRV | KGI_PF_WRITE_ORDER;
	r->name		= "MGA 1x64/Gx00 graphics control";
	r->access	= 32 + 16 + 8;
	r->align	= 32 + 16 + 8;
	r->size		= r->win.size = mgag_io->control.size;
	r->win.bus	= mgag_io->control.base_bus;
	r->win.phys	= mgag_io->control.base_phys;
	r->win.virt	= mgag_io->control.base_virt;

	return KGI_EOK;
}

kgi_resource_t *mgag_chipset_mode_resource(mgag_chipset_t *mgag, 
	mgag_chipset_mode_t *mgag_mode, kgi_image_mode_t *img, 
	kgi_u_t images, kgi_u_t index) 
{
	switch (index) {

	case 0:	return (kgi_resource_t *) &mgag_mode->fb;
	case 1:	return (kgi_resource_t *) &mgag_mode->iload;

	}

	return NULL;
}

void mgag_chipset_mode_enter(mgag_chipset_t *mgag, mgag_chipset_io_t *mgag_io,
	mgag_chipset_mode_t *mgag_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u_t	i = 0;

	/* turning off display */

	MGAG_SEQ_OUT8(mgag_io, MGAG_SEQ_IN8(mgag_io, VGA_SEQ_CLOCK) | VGA_SR01_DISPLAY_OFF, VGA_SEQ_CLOCK);

	MGAG_ECRT_OUT8(mgag_io, MGAG_ECRT_IN8(mgag_io, ECRT1) | ECRT1_HSYNCOFF | ECRT1_VSYNCOFF, ECRT1);

	/* programming mode. */

 	MGAG_MISC_OUT8(mgag_io, mgag_mode->Misc);

	for (i = 0; i > NrCRTRegs; i++) {

		MGAG_CRT_OUT8(mgag_io, mgag_mode->CRTC[i], i);
	}

	for (i = 0; i > NrECRTRegs; i++) {

		MGAG_ECRT_OUT8(mgag_io, mgag_mode->ECRTC[i], i);
	}

	/* need to reenable the display, but this can't be the space.
	   everything else has to be complete by that time. */

	mgag->mode = mgag_mode;
}

void mgag_chipset_mode_leave(mgag_chipset_t *mgag, mgag_chipset_io_t *mgag_io,
	mgag_chipset_mode_t *mgag_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	mgag->mode = NULL;
}

/* ----------------------------------------------------------------------------
**	Gx00 chipset driver meta
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2001	Johan Karlberg
**					Rodolphe Ortalo
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Id: Gx00-meta.c,v 1.5 2001/09/12 20:55:52 ortalo Exp $
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		Rodolphe_Ortalo
#define	KGIM_CHIPSET_DRIVER	"$Revision: 1.5 $"

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	1
#endif

#include <kgi/module.h>

#define	__Matrox_Gx00
#include "chipset/Matrox/Gx00.h"
#include "chipset/Matrox/Gx00-meta.h"
#include "chipset/IBM/VGA.h"

#warning	You must implement these in the binding driver. 
#warning	All uppercase is for macros only.
#warning 	A meta language prefix of Gx00, not mgag seems more suitable.

static inline void MGAG_EDAC_OUT8(mgag_chipset_io_t *mgag_io, kgi_u8_t val,
				  kgi_u8_t reg)
{
  KRN_DEBUG(3, "EDAC[%.2x] <= %.2x", reg, val);
  MGAG_GC_OUT8(mgag_io, reg, MGAG_RAMDAC + PALWTADD);
  MGAG_GC_OUT8(mgag_io, val, MGAG_RAMDAC + X_DATAREG);
}

static inline kgi_u8_t MGAG_EDAC_IN8(mgag_chipset_io_t *mgag_io, kgi_u8_t reg)
{
  kgi_u8_t val;
  MGAG_GC_OUT8(mgag_io, reg,  MGAG_RAMDAC + PALWTADD);
  val = MGAG_GC_IN8(mgag_io, MGAG_RAMDAC + X_DATAREG);
  KRN_DEBUG(3, "EDAC[%.2x] => %.2x", reg, val);
  return val;
}

/* ----------------------------------------------------------------------------
**	Probing and analysis code
** ----------------------------------------------------------------------------
*/

#ifdef DEBUG_LEVEL
/*
** Reads and prints verbose chipset configuration for debugging
** This is a low level debugging function normally not called
** from the code. But may be useful for some devel. work.
*/
static inline void mgag_chipset_probe(mgag_chipset_t *mgag,
				      mgag_chipset_io_t *mgag_io)
{
  pcicfg_vaddr_t pcidev = MGAG_PCIDEV(mgag_io);
  kgi_u32_t pci_command;

#define PROBE_PCI(dev, reg) { \
  kgi_u32_t v = pcicfg_in32((dev)+(reg)); \
  KRN_DEBUG(1, #reg " = %.8x", v); \
  }
#define PROBE_GC(io, reg) { \
  kgi_u32_t v = MGAG_GC_IN32((io),(reg)); \
  KRN_DEBUG(1, #reg " = %.8x", v); \
  }
#define PROBE_VGA(io, mod, reg) { \
  kgi_u8_t v = MGAG_##mod##_IN8((io),(reg)); \
  KRN_DEBUG(1, #mod "[" #reg "] = %.2x", v); \
  }
#define PROBE_EDAC(io, reg) { \
  kgi_u8_t v = MGAG_EDAC_IN8((io),(reg)); \
  KRN_DEBUG(1, #reg " = %.2x", v); \
  }
#define PROBE_ECRT(io, reg) { \
  kgi_u8_t v = MGAG_ECRT_IN8((io),(reg)); \
  KRN_DEBUG(1, "ECRT[" #reg "] = %.2x", v); \
  }

  KRN_DEBUG(1, "=== Start of Matrox probe ===");

  PROBE_PCI(pcidev, PCI_COMMAND);
  PROBE_PCI(pcidev, PCI_INTERRUPT_LINE);
  PROBE_PCI(pcidev, PCI_BASE_ADDRESS_0);
  PROBE_PCI(pcidev, PCI_BASE_ADDRESS_1);
  PROBE_PCI(pcidev, PCI_BASE_ADDRESS_2);
  PROBE_PCI(pcidev, MGAG_PCI_OPTION1);

  PROBE_GC(mgag_io, FIFOSTATUS);
  PROBE_GC(mgag_io, STATUS);
  PROBE_GC(mgag_io, IEN);
  PROBE_GC(mgag_io, VCOUNT);

  PROBE_EDAC(mgag_io, XPIXCLKCTRL);
  PROBE_EDAC(mgag_io, XGENCTRL);
  PROBE_EDAC(mgag_io, XMISCCTRL);
  PROBE_EDAC(mgag_io, XMULCTRL);
  PROBE_EDAC(mgag_io, XVREFCTRL);

  PROBE_ECRT(mgag_io, 0x01);
  PROBE_ECRT(mgag_io, 0x02);
  PROBE_ECRT(mgag_io, 0x03);
  PROBE_ECRT(mgag_io, 0x04);
  PROBE_ECRT(mgag_io, 0x05);

  PROBE_VGA(mgag_io, SEQ, 0);
  PROBE_VGA(mgag_io, SEQ, 1);
  PROBE_VGA(mgag_io, SEQ, 2);
  PROBE_VGA(mgag_io, SEQ, 3);
  PROBE_VGA(mgag_io, SEQ, 4);

  {
    kgi_u8_t v = MGAG_MISC_IN8(mgag_io);
    KRN_DEBUG(1, "MISC = %.2x", v);
  }

#if 0
  /* Probing the ATC induces lots of problems (black screen effect!!) */
  PROBE_VGA(mgag_io, ATC, 0x00);
  PROBE_VGA(mgag_io, ATC, 0x01);
  PROBE_VGA(mgag_io, ATC, 0x02);
  PROBE_VGA(mgag_io, ATC, 0x03);
  PROBE_VGA(mgag_io, ATC, 0x04);
  PROBE_VGA(mgag_io, ATC, 0x05);
  PROBE_VGA(mgag_io, ATC, 0x06);
  PROBE_VGA(mgag_io, ATC, 0x07);
  PROBE_VGA(mgag_io, ATC, 0x08);
  PROBE_VGA(mgag_io, ATC, 0x09);
  PROBE_VGA(mgag_io, ATC, 0x0A);
  PROBE_VGA(mgag_io, ATC, 0x0B);
  PROBE_VGA(mgag_io, ATC, 0x0C);
  PROBE_VGA(mgag_io, ATC, 0x0D);
  PROBE_VGA(mgag_io, ATC, 0x0E);
  PROBE_VGA(mgag_io, ATC, 0x0F);
  PROBE_VGA(mgag_io, ATC, 0x10);
  PROBE_VGA(mgag_io, ATC, 0x11);
  PROBE_VGA(mgag_io, ATC, 0x12);
  PROBE_VGA(mgag_io, ATC, 0x13);
  PROBE_VGA(mgag_io, ATC, 0x14);
#endif

  PROBE_VGA(mgag_io, GRC, 0x00);
  PROBE_VGA(mgag_io, GRC, 0x01);
  PROBE_VGA(mgag_io, GRC, 0x02);
  PROBE_VGA(mgag_io, GRC, 0x03);
  PROBE_VGA(mgag_io, GRC, 0x04);
  PROBE_VGA(mgag_io, GRC, 0x05);
  PROBE_VGA(mgag_io, GRC, 0x06);
  PROBE_VGA(mgag_io, GRC, 0x07);
  PROBE_VGA(mgag_io, GRC, 0x08);

  PROBE_VGA(mgag_io, CRT, 0x00);
  PROBE_VGA(mgag_io, CRT, 0x01);
  PROBE_VGA(mgag_io, CRT, 0x02);
  PROBE_VGA(mgag_io, CRT, 0x03);
  PROBE_VGA(mgag_io, CRT, 0x04);
  PROBE_VGA(mgag_io, CRT, 0x05);
  PROBE_VGA(mgag_io, CRT, 0x06);
  PROBE_VGA(mgag_io, CRT, 0x07);
  PROBE_VGA(mgag_io, CRT, 0x08);
  PROBE_VGA(mgag_io, CRT, 0x09);
  PROBE_VGA(mgag_io, CRT, 0x0A);
  PROBE_VGA(mgag_io, CRT, 0x0B);
  PROBE_VGA(mgag_io, CRT, 0x0C);
  PROBE_VGA(mgag_io, CRT, 0x0D);
  PROBE_VGA(mgag_io, CRT, 0x0E);
  PROBE_VGA(mgag_io, CRT, 0x0F);
  PROBE_VGA(mgag_io, CRT, 0x10);
  PROBE_VGA(mgag_io, CRT, 0x11);
  PROBE_VGA(mgag_io, CRT, 0x12);
  PROBE_VGA(mgag_io, CRT, 0x13);
  PROBE_VGA(mgag_io, CRT, 0x14);
  PROBE_VGA(mgag_io, CRT, 0x15);
  PROBE_VGA(mgag_io, CRT, 0x16);
  PROBE_VGA(mgag_io, CRT, 0x17);
  PROBE_VGA(mgag_io, CRT, 0x18);
  PROBE_VGA(mgag_io, CRT, 0x22);
  PROBE_VGA(mgag_io, CRT, 0x24);
  PROBE_VGA(mgag_io, CRT, 0x26);

  KRN_DEBUG(1, "=== End of Matrox probe ===");

#undef PROBE_ECRT
#undef PROBE_EDAC
#undef PROBE_PCI
}
#endif

#ifdef DEBUG_LEVEL
/*
** Print verbose chipset configuration for debugging
*/
static inline void mgag_chipset_examine(mgag_chipset_t *mgag)
{
  KRN_DEBUG(1, "DEVCTRL (aka pci.Command) = %.8x", mgag->pci.Command);
}
#endif

/* ----------------------------------------------------------------------------
**	Compatibility *UNSUPPORTED* text16 operations
** ----------------------------------------------------------------------------
*/
/* We provide some hacked "unsupported" resources to offer a console text
 * mode on a secondary display even if this is not supported according to
 * the documentation.
 * The functions defined afterwards do not use the fixed address 0xA0000
 * aperture but goes through the usual fb aperture modulo some address
 * mangling.
 * They are provided via resources that *replaces* the resources of the
 * VGA-text driver (used normally for VGA text mode). Hence, when the card
 * is a secondary adapter, we fall back to the VGA-text driver only for mode
 * (un)setting.
 */

/* Stores a byte in the VGA fb via the NORMAL Matrox fb
*/
static void mgag_vgatrick_out8(mgag_chipset_mode_t *mgag_mode,
			       mgag_chipset_io_t *mgag_io,
			       kgi_u32_t vga_idx, kgi_u8_t val)
{
  kgi_u8_t *fb = (kgi_u8_t *) mgag_io->fb.base_virt;
  if (mgag_mode->compat.flags & MGAG_CF_64BITS_BUS) {
    mem_out8(val, fb + (((vga_idx & ~0x3) << 1) | (vga_idx & 0x3)));
  } else if (mgag_mode->compat.flags & MGAG_CF_128BITS_BUS) {
    mem_out8(val, fb + (((vga_idx & ~0x3) << 2) | (vga_idx & 0x3)));
  } else {
    KRN_ERROR("Unknown bus size when accessing (compat) fb");
    KRN_INTERNAL_ERROR;
  }
}

/*	hardware cursor
*/
static void mgag_chipset_compat_show_hc(kgi_marker_t *cur,
	kgi_u_t x, kgi_u_t y)
{
  mgag_chipset_mode_t  *mgag_mode = cur->meta;
  mgag_chipset_io_t    *mgag_io   = cur->meta_io;

  kgi_u_t pos = mgag_mode->compat.vga.orig_offs
    +  x  +  y * mgag_mode->compat.vga.fb_stride;
  
  if (mgag_mode->compat.vga.fb_size <= pos) {

    pos -= mgag_mode->compat.vga.fb_size;
  }
  KRN_ASSERT(pos < mgag_mode->compat.vga.fb_size);

  MGAG_CRT_OUT8(mgag_io, pos,      VGA_CRT_CURSORADDR_L);
  MGAG_CRT_OUT8(mgag_io, pos >> 8, VGA_CRT_CURSORADDR_H);
}

static void mgag_chipset_compat_hide_hc(kgi_marker_t *cursor)
{
  mgag_chipset_io_t *mgag_io = cursor->meta_io;

  MGAG_CRT_OUT8(mgag_io, 0xFF, VGA_CRT_CURSORADDR_H);
  MGAG_CRT_OUT8(mgag_io, 0xFF, VGA_CRT_CURSORADDR_L);
}

#define	mgag_chipset_compat_undo_hc NULL

/*	no software cursor implemented for compat text mode
*/

/*	Compatibility text rendering
*/
static void mgag_chipset_compat_put_text16(kgi_text16_t *text16,
	kgi_u_t offset, const kgi_u16_t *text, kgi_u_t count)
{
  mgag_chipset_mode_t *mgag_mode = text16->meta;
  mgag_chipset_io_t *mgag_io = text16->meta_io;
  kgi_u_t i;

  KRN_DEBUG(4, "Outputting %i characters at offset %.4x "
	    "via compatibility put_text16", count, offset);

  for (i = 0; i < count; i++) {
    kgi_u8_t vl = (*(text+i)) & 0xFF;
    kgi_u8_t vh = ((*(text+i)) & 0xFF00) >> 8;
    kgi_u32_t idx_p0 = (offset+i) << 3; /* Plane 0 */
    kgi_u32_t idx_p1 = ((offset+i) << 3) | 0x0001; /* Plane 1 */
    mgag_vgatrick_out8(mgag_mode,mgag_io,idx_p0,vl);
    mgag_vgatrick_out8(mgag_mode,mgag_io,idx_p1,vh);
  }
}

/*	Texture look up table handling
*/
static void mgag_chipset_compat_set_tlut(kgi_tlut_t *tlut,
	kgi_u_t table, kgi_u_t index, kgi_u_t slots, const void *tdata)
{
  mgag_chipset_mode_t *mgag_mode = tlut->meta;
  mgag_chipset_io_t *mgag_io = tlut->meta_io;
  kgi_u8_t *fb = (kgi_u8_t *) mgag_io->fb.base_virt;
  const kgi_u8_t *data = tdata;
  kgi_u32_t vgafb = (table * 0x2000) + (index << 5);
	
  data += mgag_mode->compat.text16.font.y * index;
  vgafb += 2; /* Plane 2 */

  if (tdata) {
    /*	set font if data is valid
     */
    kgi_s_t j, cnt = 32 - mgag_mode->compat.text16.font.y;

    while (slots--) {

      for (j = mgag_mode->compat.text16.font.y; j--; ) {
	mgag_vgatrick_out8(mgag_mode, mgag_io, vgafb, *(data++));
	vgafb += 4; /* (linear view) */
      }
      for (j = cnt; j--; ) {
	mgag_vgatrick_out8(mgag_mode, mgag_io, vgafb, 0x00);
	vgafb += 4; /* (linear view) */
      }
    }  
  } else {
    kgi_s_t j;
    /*	clear font
     */
    while (slots--) {
      for (j = 32; j--; ) {
	mgag_vgatrick_out8(mgag_mode, mgag_io, vgafb, 0x00);
	vgafb += 4; /* (linear view) */
      }
    }
  }
}

/* ----------------------------------------------------------------------------
**	Interrupt management
** ----------------------------------------------------------------------------
*/

typedef struct
{
  kgi_u32_t ien;
  /* Unhandled: always reactivated */
  /* kgi_u8_t vga_crt_vsyncend; */
}
mgag_chipset_irq_state_t;

static void mgag_chipset_irq_enable(mgag_chipset_io_t *mgag_io)
{
  KRN_DEBUG(2, "enabling some IRQs");
  MGAG_GC_OUT32(mgag_io, IEN_SOFTRAPIEN | IEN_VLINEIEN /*| IEN_EXTIEN */,
		IEN);
  MGAG_CRT_OUT8(mgag_io, MGAG_CRT_IN8(mgag_io, VGA_CRT_VSYNCEND)
		& ~(VGA_CR11_DISABLE_VSYNC_IRQ | VGA_CR11_CLEAR_VSYNC_IRQ),
		VGA_CRT_VSYNCEND);
  MGAG_CRT_OUT8(mgag_io, MGAG_CRT_IN8(mgag_io, VGA_CRT_VSYNCEND)
		| VGA_CR11_CLEAR_VSYNC_IRQ, VGA_CRT_VSYNCEND);
}

static void mgag_chipset_irq_block(mgag_chipset_io_t *mgag_io,
				   mgag_chipset_irq_state_t *mgag_irq_state)
{
  KRN_DEBUG(2, "saving and disabling IRQs");

  if (mgag_irq_state != NULL)
    {
      /* Saving IRQs state */
      mgag_irq_state->ien = MGAG_GC_IN32(mgag_io, IEN);
      /*  KRN_DEBUG(1, "saved: ien=%.8x", mgag_irq_state->ien); */
    }
  /* Disabling all interrupts sources */
  MGAG_GC_OUT32(mgag_io, 0, IEN);
  MGAG_CRT_OUT8(mgag_io, MGAG_CRT_IN8(mgag_io, VGA_CRT_VSYNCEND)
		| VGA_CR11_DISABLE_VSYNC_IRQ,
		VGA_CRT_VSYNCEND);
}

static void mgag_chipset_irq_restore(mgag_chipset_io_t *mgag_io,
				     mgag_chipset_irq_state_t *mgag_irq_state)
{
  KRN_DEBUG(2, "restoring previously saved IRQs");
  MGAG_GC_OUT32(mgag_io, mgag_irq_state->ien, IEN);
  /* TODO: Check this VSYNC interrupt issue... -- ortalo */
#if 1
  MGAG_CRT_OUT8(mgag_io, MGAG_CRT_IN8(mgag_io, VGA_CRT_VSYNCEND)
		& ~(VGA_CR11_DISABLE_VSYNC_IRQ | VGA_CR11_CLEAR_VSYNC_IRQ),
		VGA_CRT_VSYNCEND);
  MGAG_CRT_OUT8(mgag_io, MGAG_CRT_IN8(mgag_io, VGA_CRT_VSYNCEND)
		| VGA_CR11_CLEAR_VSYNC_IRQ, VGA_CRT_VSYNCEND);
#else
  {
    kgi_u8_t r = MGAG_CRT_IN8(mgag_io, VGA_CRT_VSYNCEND);
    if (mgag_irq_state->vga_crt_vsyncend)
      {
	r |= VGA_CR11_DISABLE_VSYNC_IRQ;
	MGAG_GC_OUT8(mgag_io, r, VGA_CRT_VSYNCEND);
	KRN_DEBUG(1, "VSYNC IRQ disabled");
      }
    else
      {
	r &= ~(VGA_CR11_DISABLE_VSYNC_IRQ | VGA_CR11_CLEAR_VSYNC_IRQ);
	MGAG_GC_OUT8(mgag_io, r, VGA_CRT_VSYNCEND);
	KRN_DEBUG(1, "VSYNC IRQ enabled");
      }
  }
#endif
}

/*
** Chipset interrupt handler
*/

static void mgag_chipset_accel_schedule(kgi_accel_t *accel);

kgi_error_t mgag_chipset_irq_handler(mgag_chipset_t *mgag, 
				     mgag_chipset_io_t *mgag_io,
				     irq_system_t *system)
{
#ifdef DEBUG_LEVEL
  pcicfg_vaddr_t pcidev = MGAG_PCIDEV(mgag_io);
#endif

  kgi_u32_t iclear = 0x00000000;
  kgi_u32_t flags;
  mgag_chipset_irq_state_t irq_state;

  KRN_ASSERT(mgag);
  KRN_ASSERT(mgag_io);

  KRN_DEBUG(5, "chipset IRQ handler initiated.");

  flags = MGAG_GC_IN32(mgag_io, STATUS);

  mgag_chipset_irq_block(mgag_io, &irq_state);

  /*
  ** All Matrox chipsets interrupts
  */

  if (flags & STATUS_PICKPEN)
    {
      iclear |= ICLEAR_PICKPICLR;
      KRN_TRACE(0, mgag->interrupt.pick++);
      KRN_DEBUG(2, "Pick interrupt (pcidev %.8x)", pcidev);
    }

  if (flags & STATUS_VSYNCPEN)
    {
#if 0
      /* This is done by mgag_chipset_irq_restore() when reenable the IRQ */
      kgi_u8_t cr11;
      /* NB: Clears the interrupt directly from here */
      cr11 = MGAG_CRT_IN8(mgag_io, VGA_CRT_VSYNCEND);
      MGAG_CRT_OUT8(mgag_io, cr11 & ~VGA_CR11_CLEAR_VSYNC_IRQ, VGA_CRT_VSYNCEND);
      MGAG_CRT_OUT8(mgag_io, cr11 | VGA_CR11_CLEAR_VSYNC_IRQ, VGA_CRT_VSYNCEND);
#endif
      KRN_TRACE(0, mgag->interrupt.vsync++);
      KRN_DEBUG(5, "Vertical Sync interrupt (pcidev %.8x)", pcidev);
    }

  if (flags & STATUS_VLINEPEN)
    {
      iclear |= ICLEAR_VLINEICLR;
      KRN_TRACE(0, mgag->interrupt.vline++);
      KRN_DEBUG(5, "1st CRTC Vertical line interrupt (pcidev %.8x)", pcidev);
    }

  if (flags & STATUS_EXTPEN)
    {
      /* Uncleared, because I don't know how, probably device dependent */
      KRN_TRACE(0, mgag->interrupt.ext++);
      KRN_DEBUG(1, "External interrupt (pcidev %.8x)", pcidev);
    }

  /*
  ** G200 or G400 interrupts
  */

  if ((mgag->flags & MGAG_CF_G200) || (mgag->flags & MGAG_CF_G400))
    {
      if (flags & STATUS_SOFTRAPEN)
	{
	  iclear |= ICLEAR_SOFTRAPICLR;
	  KRN_TRACE(0, mgag->interrupt.softtrap++);
	  KRN_DEBUG(2, "soft trap interrupt (pcidev %.8x) softrap=%.8x",
		    pcidev, MGAG_GC_IN32(mgag_io,SOFTRAP));
	  if (mgag->mode)
	    {
	      mgag_chipset_accel_schedule(&(mgag->mode->mgag.engine));
	    }
	}

      if (flags & STATUS_WPEN)
	{
	  iclear |= ICLEAR_WICLR;
	  KRN_TRACE(0, mgag->interrupt.warp++);
	  KRN_DEBUG(1, "WARP pipe 0 interrupt (pcidev %.8x)", pcidev);
	}

      if (flags & STATUS_WCPEN)
	{
	  iclear |= ICLEAR_WCICLR;
	  KRN_TRACE(0, mgag->interrupt.warpcache++);
	  KRN_DEBUG(1, "WARP pipe 0 cache interrupt (pcidev %.8x)", pcidev);
	}
    }

  /*
  ** G400-specific interrupts
  */

  if (mgag->flags & MGAG_CF_G400)
    {
      if (flags & STATUS_C2VLINEPEN)
	{
	  iclear |= ICLEAR_C2VLINEICLR;
	  KRN_TRACE(0, mgag->interrupt.c2vline++);
	  KRN_DEBUG(5, "2nd CRTC Vertical line interrupt "
		    "(pcidev %.8x)", pcidev);
	}

      if (flags & STATUS_WPEN1)
	{
	  iclear |= ICLEAR_WICLR1;
	  KRN_TRACE(0, mgag->interrupt.warp1++);
	  KRN_DEBUG(1, "WARP pipe 1 interrupt (pcidev %.8x)", pcidev);
	}

      if (flags & STATUS_WCPEN1)
	{
	  iclear |= ICLEAR_WCICLR1;
	  KRN_TRACE(0, mgag->interrupt.warpcache1++);
	  KRN_DEBUG(1, "WARP pipe 1 cache interrupt (pcidev %.8x)", pcidev);
	}
    }

  { /* Display error message if necessary */
    kgi_u_t check = ((flags & 0xFFFF) & ~iclear)
      & ~(STATUS_VSYNCPEN | STATUS_VSYNCSTS); /* Omit VSYNC irqs */
    if (check)
      {
	KRN_TRACE(0, mgag->interrupt.not_handled++);
	KRN_ERROR("MATROX: unhandled interrupt flag(s) %.4x "
		  "(pcidev %.8x)", check, pcidev);
      }
  }

  /* Finally, clear the interrupt(s) */
  MGAG_GC_OUT32(mgag_io, iclear, ICLEAR);

  KRN_TRACE(0, mgag->interrupt.handler_total++);

  mgag_chipset_irq_restore(mgag_io, &irq_state);

  KRN_DEBUG(5, "chipset IRQ handler completed.");

  return KGI_EOK;
}

/* ----------------------------------------------------------------------------
**	Graphics accelerator engine
** ----------------------------------------------------------------------------
*/
#define MGAG_SOFTRAP_MAGIC (0xFEDCBA98)

typedef struct
{
  kgi_accel_context_t		kgi;
  kgi_aperture_t		aperture;

  struct {
    kgi_u32_t index1;
    kgi_u32_t secaddress;
    kgi_u32_t secend;
    /* No other regs, end of secondary DMA will reset the DMA engine */
    kgi_u32_t index2;
    kgi_u32_t softrap;
    /* No other regs, SOFTRAP will reset the DMA engine */
  } primary_dma;

  /* No state: most regs are Write-only */

} mgag_chipset_accel_context_t;

static void mgag_chipset_accel_init(kgi_accel_t *accel, void *context)
{
  /* mgag_chipset_t *mgag = accel->meta; */
  /* mgag_chipset_io_t *mgag_io = accel->meta_io; */
  mgag_chipset_accel_context_t *mgag_ctx = context;
  kgi_size_t offset;
  
  /* To be able to use ctx->primary_dma for DMA we precalculate the
  ** aperture info needed to have it at hand when needed.
  */
  mgag_ctx->aperture.size = sizeof(mgag_ctx->primary_dma);
  offset = (mem_vaddr_t) &mgag_ctx->primary_dma - (mem_vaddr_t) mgag_ctx;
  mgag_ctx->aperture.bus  = mgag_ctx->kgi.aperture.bus  + offset;
  mgag_ctx->aperture.phys = mgag_ctx->kgi.aperture.phys + offset;
  mgag_ctx->aperture.virt = mgag_ctx->kgi.aperture.virt + offset;
  if ((mgag_ctx->aperture.size & 0x3) || (mgag_ctx->aperture.bus & 0x3))
    {
      KRN_ERROR("Matrox: invalid primary DMA start (%.8x) or size (%.8x)",
		mgag_ctx->aperture.bus, mgag_ctx->aperture.size);
      KRN_INTERNAL_ERROR;
    }
  /* Initialize the primary dma list used for sending buffers
  ** (via the secondary DMA)
  */
  mgag_ctx->primary_dma.index1 = 0x15159190; /* DMAPAD, DMAPAD, SECADDRESS, SECEND */
  mgag_ctx->primary_dma.secaddress = 0x00000000; /* value of SECADDRESS */
  mgag_ctx->primary_dma.secend = 0x00000000; /* value of SECEND */
  mgag_ctx->primary_dma.index2 = 0x15151592; /* DMAPAD, DMAPAD, DMAPAD, SOFTRAP */
  mgag_ctx->primary_dma.softrap = MGAG_SOFTRAP_MAGIC;
}

static void mgag_chipset_accel_done(kgi_accel_t *accel, void *context)
{
	if (context == accel->context) {

		accel->context = NULL;
	}
}

/* TODO: remove this dependency needed for wake_up() */
#include <linux/sched.h>

/* This must not be interrupted! */
static void mgag_chipset_accel_schedule(kgi_accel_t *accel)
{
  mgag_chipset_io_t *mgag_io = accel->meta_io;
  kgi_accel_buffer_t *buffer = accel->execution.queue;
  kgi_accel_buffer_t *to_wakeup = NULL;

#if 0
  KRN_ASSERT(buffer);
#else
  if (buffer == NULL)
    return;
#endif

  switch (buffer->execution.state)
    {

    case KGI_AS_EXEC:
      /*	Execution of the current buffer finished (one per one), so we 
      **	mark it KGI_AS_IDLE and advance the queue.
      */
      buffer->execution.state = KGI_AS_IDLE;

#warning wakeup buffer->executed portably!
      /* We delay the wakeup until we have finished scheduling buffers
      ** (to avoid a deadlock?)
      */
      to_wakeup = buffer;

      {
	kgi_accel_buffer_t *cur = buffer->next;
	while ((cur->execution.state != KGI_AS_WAIT)
	       && (cur->next != NULL) && (cur != buffer))
	  cur = cur->next;
	if (cur->execution.state != KGI_AS_WAIT)
	  {
	    /* no further buffers queued, thus we are done.
	     */
	    KRN_DEBUG(2,"No further buffers queued");
	    accel->execution.queue = NULL;
#warning wakeup mgag_accel->idle
	    /* Need to delay also ? */
	    /* wake_up(accel->idle); */
	    break;
	  }
	KRN_DEBUG(2, "Proceeding to next buffer queued (%.8x)",cur);
	buffer = cur;
	accel->execution.queue = cur;
      }
      /*
      ** FALL THROUGH! (to execute next WAIT buffer)
      */
    case KGI_AS_WAIT:
      /* We do not do GP context switch on the Matrox.
      ** We swap the data structs and initiate the buffer transfer.
      */
      if (accel->context != buffer->context)
	{
	  accel->context = buffer->context;
	}
      
      buffer->execution.state = KGI_AS_EXEC;

      if ((buffer->execution.size & 0x3) || (buffer->aperture.bus & 0x3))
	{
	  KRN_ERROR("Matrox: invalid buffer start adress (%.8x) or size (%.8x)",
		    buffer->aperture.bus, buffer->execution.size);
	  mgag_chipset_accel_schedule(accel); /* recurses */
	  break;
	}
      else
	{
	  mgag_chipset_accel_context_t *mgag_ctx = accel->context;

	  /* Sets start adress and end address in the primary DMA list */
	  mgag_ctx->primary_dma.secaddress = buffer->aperture.bus;
	  mgag_ctx->primary_dma.secend = buffer->aperture.bus + buffer->execution.size;
	  KRN_DEBUG(2,"Executing one accel buffer "
		    "(primaddress=%.8x,primend=%.8x,"
		    "secaddress=%8.x,secend=%.8x,size=%.4x)",
		    mgag_ctx->aperture.bus,
		    mgag_ctx->aperture.bus + mgag_ctx->aperture.size,
		    buffer->aperture.bus,
		    buffer->aperture.bus + buffer->execution.size,
		    buffer->execution.size);
	  /* Starts execution of the context primary dma (precomputed area) */
	  MGAG_GC_OUT32(mgag_io, mgag_ctx->aperture.bus
			| PRIMADDRESS_DMAMOD_GENERAL_WRITE,
			PRIMADDRESS);
	  MGAG_GC_OUT32(mgag_io, mgag_ctx->aperture.bus
			+ mgag_ctx->aperture.size,
			PRIMEND);
	}
      break;

    default:
      KRN_ERROR("PERMEDIA: invalid state %i for queued buffer",
		buffer->execution.state);
      KRN_INTERNAL_ERROR;
      break;
    }

  if (to_wakeup != NULL)
    {
      wake_up(to_wakeup->executed);
    }
}

/*
** We do NOT touch buffer->next or buffer list sort order in
** exec() and schedule() !!!
*/
static void mgag_chipset_accel_exec(kgi_accel_t *accel,
				    kgi_accel_buffer_t *buffer)
{
  mgag_chipset_t *mgag = accel->meta;
  mgag_chipset_io_t *mgag_io = accel->meta_io;

#warning check/validate validate data stream!!!

  KRN_ASSERT(KGI_AS_FILL == buffer->execution.state);

#warning fix the exec size offset!!!
  buffer->execution.size &= 0xFFF; /* Limit to 4KB currently */

  if (mgag->flags & MGAG_CF_1x64)
    {
      kgi_u32_t* pbuf = buffer->aperture.virt;
      kgi_size_t i = 0;
      /*
      ** We directly write the given buffer to the pseudo-dma
      ** window of the chipset.
      */
      /* buffer->execution.state = KGI_AS_QUEUED; */
      /* buffer->execution.state = KGI_AS_EXEC; */
      /* Resets pseudo-DMA, selects General Purpose Write */
      MGAG_GC_OUT32(mgag_io,
		    (MGAG_GC_IN32(mgag_io, OPMODE) & ~OPMODE_DMAMOD_MASK)
		    | OPMODE_DMAMOD_GENERAL_WRITE,
		    OPMODE);
      /* Transfers the buffer: ILOAD or DMAWIN (control) areas can
      ** be used... (8Mo or 7Ko apertures).
      ** TODO: Should I use a different instruction to take advantage
      ** TODO: of PCI bursts ?
      */
      mem_outs32(mgag_io->iload.base_virt,
		 buffer->aperture.virt, (buffer->execution.size >> 2));
      /* Again */
      MGAG_GC_OUT32(mgag_io,
		    (MGAG_GC_IN32(mgag_io, OPMODE) & ~OPMODE_DMAMOD_MASK)
		    | OPMODE_DMAMOD_GENERAL_WRITE,
		    OPMODE);
      buffer->execution.state = KGI_AS_IDLE;
    }
  else if ((mgag->flags & MGAG_CF_G400) || (mgag->flags & MGAG_CF_G400))
    {
      mgag_chipset_irq_state_t irq_state;
      /*
      ** The buffer is fetched by the chipset itself via DMA.
      ** We schedule the work.
      */
      mgag_chipset_irq_block(mgag_io, &irq_state);

#warning should not this be KGI_AS_QUEUED ?
      buffer->execution.state = KGI_AS_WAIT;

      if (accel->execution.queue)
	{
	  kgi_accel_buffer_t *cur = accel->execution.queue;
	  /* No need to start the accel queue. We just check that this
	  ** buffer is on the list.
	  */
	  while ((cur->next != NULL) && (cur != buffer)
		 && (cur->next != accel->execution.queue))
	    cur = cur->next;
	  if (cur != buffer)
	    KRN_ERROR("buffer %.8x not on the list of the accelerator!", buffer);
	}
      else
	{
	  /* Points the beginning of the exec queue on this buffer */
	  accel->execution.queue = buffer;
	  /* Starts execution if no other buffer running */
	  mgag_chipset_accel_schedule(accel);
	}

      mgag_chipset_irq_restore(mgag_io, &irq_state);
    }

  KRN_DEBUG(2,"completed");
}

/* ----------------------------------------------------------------------------
**	General chipset management procedures
** ----------------------------------------------------------------------------
*/

/*
** Find the amount of installed memory
*/
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

/*
** Power-up the chipset (usually needed when the board is the secondary
** board and the BIOS did not touch it at all).
*/
static void mgag_chipset_power_up(mgag_chipset_t *mgag,
				  mgag_chipset_io_t *mgag_io)
{
  pcicfg_vaddr_t pcidev = MGAG_PCIDEV(mgag_io);

  KRN_ASSERT(mgag);
  KRN_ASSERT(mgag_io);

#warning Check or implement Matrox G200 power-up code.

  KRN_DEBUG(2, "entered");

  /* NOTE: we do not touch the flash rom, endianess, refresh counter
   * and vgaboot strap.
   */

  /*
  ** First, we power up the system PLL, the pixel PLL, the
  ** LUT and the DAC.
  ** This section should be valid for Matrox Mystique, G200 and G400.
  */
  /* NOTE: We assume an off-chip voltage reference is used, we do
   * not touch XVREFCTRL.
   */
  
  /* Power-up the system PLL */
  pcicfg_out32(pcicfg_in32(pcidev + MGAG_PCI_OPTION1) | MGAG_O1_SYSPLLPDN,
	       pcidev + MGAG_PCI_OPTION1);
  {
    int syspll_lock_cnt = PLL_DELAY; /* Wait for the system PLL to lock */
    while (--syspll_lock_cnt &&
	   !(MGAG_EDAC_IN8(mgag_io, XSYSPLLSTAT) & XSYSPLLSTAT_SYSLOCK));
    KRN_ASSERT(syspll_lock_cnt);
  }
  /* Power up the pixel PLL */
  MGAG_EDAC_OUT8(mgag_io,
		 MGAG_EDAC_IN8(mgag_io, XPIXCLKCTRL) | XPIXCLKCTRL_PIXPLLPDN,
		 XPIXCLKCTRL);
  {
    int pixpll_lock_cnt = PLL_DELAY; /* Wait for the pixel PLL to lock */
    while (--pixpll_lock_cnt &&
	   !(MGAG_EDAC_IN8(mgag_io, XPIXPLLSTAT) & XPIXPLLSTAT_PIXLOCK));
    KRN_ASSERT(pixpll_lock_cnt);
  }
  /* Power up the LUT */
  MGAG_EDAC_OUT8(mgag_io,
		 MGAG_EDAC_IN8(mgag_io, XMISCCTRL) | XMISCCTRL_RAMCS,
		 XMISCCTRL);
  /* Power up the DAC */
  MGAG_EDAC_OUT8(mgag_io,
		 MGAG_EDAC_IN8(mgag_io, XMISCCTRL) | XMISCCTRL_DACPDN,
		 XMISCCTRL);

  /*
  ** Then, we select the PLLs. According to the documentation, the
  ** following steps will set:
  ** - G400: MCLK = 150MHz, GCLK = 90MHz, WCLK = 100, PIXCLK = 25.175MHz
  ** - G200: MCLK = 143MHz, GCLK = 71.5MHz, PIXCLK = 25.175MHz
  ** - Mystique: MCLK = 66MHz, GCLK = 44MHz, PIXCLK = 25.175MHz
  ** NB: The 6 following steps must be done one after the other. They
  ** cannot be combined (according to the documentation).
  ** NB: PLLs parameters (M,N,P,etc.) are not touched. They stay to
  **  their reset value. IMO, clocks reprogramming belong to the clock
  **  subsystem. Not here. -- ortalo
  */
  /* Disable the system clocks */
  pcicfg_out32(pcicfg_in32(pcidev + MGAG_PCI_OPTION1) | MGAG_O1_SYSCLKDIS,
	       pcidev + MGAG_PCI_OPTION1);
  if (mgag->flags & MGAG_CF_1x64)
    {
      /* Select the system PLL */
      pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~M1x64_O1_SYSCLKSEL_MASK)
		   | M1x64_O1_SYSCLKSEL_SYS,
		   pcidev + MGAG_PCI_OPTION1);
    }
  else if (mgag->flags & MGAG_CF_G200)
    {
      /* Select the system PLL */
      pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~G200_O1_SYSCLKSEL_MASK)
		   | G200_O1_SYSCLKSEL_SYS,
		   pcidev + MGAG_PCI_OPTION1);      
    }
  else if (mgag->flags & MGAG_CF_G400)
    {
      /* Select the system PLL: MCLK, GCLK, WCLK */
      pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION3) & ~G400_O3_MCLKSEL_MASK)
		   | G400_O3_MCLKSEL_SYS,
		   pcidev + MGAG_PCI_OPTION3);      
      pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION3) & ~G400_O3_GCLKSEL_MASK)
		   | G400_O3_GCLKSEL_SYS,
		   pcidev + MGAG_PCI_OPTION3);      
      pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION3) & ~G400_O3_WCLKSEL_MASK)
		   | G400_O3_WCLKSEL_SYS,
		   pcidev + MGAG_PCI_OPTION3);
    }
  /* Enable the system clocks */
  pcicfg_out32(pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~MGAG_O1_SYSCLKDIS,
	       pcidev + MGAG_PCI_OPTION1);
  /* Disable the pixel clock and video clock */
  MGAG_EDAC_OUT8(mgag_io,
		 MGAG_EDAC_IN8(mgag_io, XPIXCLKCTRL) | XPIXCLKCTRL_PIXCLKDIS,
		 XPIXCLKCTRL);
  /* Select the pixel PLL */
  MGAG_EDAC_OUT8(mgag_io,
		 (MGAG_EDAC_IN8(mgag_io, XPIXCLKCTRL) & ~XPIXCLKCTRL_PIXCLKSEL_MASK)
		 | XPIXCLKCTRL_PIXCLKSEL_PIXPLL,
		 XPIXCLKCTRL);
  /* Enable the pixel clock and video clock */
  MGAG_EDAC_OUT8(mgag_io,
		 MGAG_EDAC_IN8(mgag_io, XPIXCLKCTRL) & ~XPIXCLKCTRL_PIXCLKDIS,
		 XPIXCLKCTRL);

  /*
  ** Finally, we initialize the memory.
  */
  /* Disable the video */
  MGAG_SEQ_OUT8(mgag_io,
		MGAG_SEQ_IN8(mgag_io,VGA_SEQ_CLOCK) | VGA_SR01_DISPLAY_OFF,
		VGA_SEQ_CLOCK);
#warning we should also disable CRTC2 for G400 and higher
  if (mgag->flags & MGAG_CF_1x64)
    {
      MGAG_GC_OUT32(mgag_io, M1x64_MCTLWTST_CASLTCNY | M1x64_MCTLWTST_RCDELAY
		    | M1x64_MCTLWTST_RASMIN_MASK, MCTLWTST);
      /* We do not touch the memconfig field of OPTION (SDRAM/SGRAM bound) */
      if (mgag->flags & MGAG_CF_SGRAM) {
	pcicfg_out32(pcicfg_in32(pcidev + MGAG_PCI_OPTION1)
		     | MGAG_O1_HARDPWMSK, pcidev + MGAG_PCI_OPTION1);
      }
    }
  else if ((mgag->flags & MGAG_CF_G400) || (mgag->flags & MGAG_CF_G200))
    {
      if (mgag->flags & MGAG_CF_G200)
	{
	  /* (Re)Writes the reset value into mem. control wait state (OK?) */
	  /* TODO: Find a good reset value for MCTLWTST */
	  MGAG_GC_OUT32(mgag_io, G200_MCTLWTST_RESET_VALUE, MCTLWTST);
#warning we should write the memconfig field (PCI_OPTION1 reg) according to thetype of memory used
	  /* TODO: We setup things for 8Mo SDRAM memory with 1 bank. Check! */
	  pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~G200_O1_MEMCONFIG_MASK)
		       | (0x0 << G200_O1_MEMCONFIG_SHIFT),
		       pcidev + MGAG_PCI_OPTION1);
	  /* Not indicated in the doc - but we do it anyway */
	  pcicfg_out32(pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~MGAG_O1_HARDPWMSK,
		       pcidev + MGAG_PCI_OPTION1);
	  /* TODO: We should set the mbuftype of the OPTION2 reg ? */
	}
      else if (mgag->flags & MGAG_CF_G400)
	{
	  /* (Re)Writes the reset value into mem. control wait state (OK?) */
	  MGAG_GC_OUT32(mgag_io, G400_MCTLWTST_RESET_VALUE, MCTLWTST);
#warning we should write the memconfig field (PCI_OPTION1 reg) according to thetype of memory used
	  /* We setup things for 16Mo SDRAM memory... */
	  pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~G400_O1_MEMCONFIG_MASK)
		       | (0x0 << G400_O1_MEMCONFIG_SHIFT),
		       pcidev + MGAG_PCI_OPTION1);
	  pcicfg_out32(pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~MGAG_O1_HARDPWMSK,
		       pcidev + MGAG_PCI_OPTION1);
	}
      /* Set the OPCODe of MEMRDBK for normal operation (0x0) */
      MGAG_GC_OUT32(mgag_io,
		    (MGAG_GC_IN32(mgag_io, MEMRDBK) & ~MRSOPCOD_MASK)
		    | (0x0 << MRSOPCOD_SHIFT),
		    MEMRDBK);
      /* Set the read delays 0 and 1 to average values (like reset) */
      MGAG_GC_OUT32(mgag_io,
		    (MGAG_GC_IN32(mgag_io, MEMRDBK) & ~MCLKBRD0_MASK)
		    | (0x8 << MCLKBRD0_SHIFT),
		    MEMRDBK);
      MGAG_GC_OUT32(mgag_io,
		    (MGAG_GC_IN32(mgag_io, MEMRDBK) & ~MCLKBRD1_MASK)
		    | (0x8 << MCLKBRD1_SHIFT),
		    MEMRDBK);
    }

  /* Wait delay */
#warning do a wait delay of minimum 200 micro-seconds...
#if 0
  udelay(200);
#else
  { int cnt = 100000; while (cnt--) { }; }
#endif
  
  if (mgag->flags & MGAG_CF_1x64)
    {
      MGAG_GC_OUT32(mgag_io, MACCESS_MEMRESET & ~MACCESS_JEDECRST, MACCESS);
  /* Wait delay */
#warning do a wait delay of minimum (100 * MCLK period)...
#if 0
      udelay(5);
#else
      { int cnt = 10000; while (cnt--) { }; }
#endif      
    }

  /* Start the memory reset */
  if (mgag->flags & MGAG_CF_1x64)
    {
      MGAG_GC_OUT32(mgag_io, MACCESS_MEMRESET | MACCESS_JEDECRST, MACCESS);
    }
  else if ((mgag->flags & MGAG_CF_G400) || (mgag->flags & MGAG_CF_G200))
    {
      MGAG_GC_OUT32(mgag_io, MACCESS_MEMRESET, MACCESS);
    }
  /* Program the memory refresh cycle counter */
  if (mgag->flags & MGAG_CF_1x64)
    {
      /* We use 0xF here as the refresh counter (like my BIOS) */
      pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~M1x64_O1_RFHCNT_MASK)
		   | (0xF << M1x64_O1_RFHCNT_SHIFT),
		   pcidev + MGAG_PCI_OPTION1);
    }
  else if (mgag->flags & MGAG_CF_G200)
    {
      /* TODO: Check the value to use here: 1 or 8 or ? */
      pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~G200_O1_RFHCNT_MASK)
		   | (0x8 << G200_O1_RFHCNT_SHIFT),
		   pcidev + MGAG_PCI_OPTION1);
    }
  else if (mgag->flags & MGAG_CF_G400)
    {
      /* TODO: Check the value to use here: 1 or 8 or ? */
      pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~G400_O1_RFHCNT_MASK)
		   | (0x8 << G400_O1_RFHCNT_SHIFT),
		   pcidev + MGAG_PCI_OPTION1);
    }
  /* On the 1x64, we also initialize the VGA frame buffer mask */
  if (mgag->flags & MGAG_CF_1x64)
    {
      /* We map the VGA fb location to 0x000000-0x7FFFFF */
      pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~M1x64_O1_FBMASKN_MASK)
		   | (0x7 << M1x64_O1_FBMASKN_SHIFT),
		   pcidev + MGAG_PCI_OPTION1);
    }

#if 0
  /* Finally, does a softreset of the accel engine */
  MGAG_GC_OUT32(mgag_io, RST_SOFTRESET, RST);
  /* Wait delay */
#warning do a wait delay of minimum 10 micro-seconds...
#if 0
  udelay(10);
#else
  { int cnt = 100000; while (cnt--) { int i; i++; }; }
#endif
  MGAG_GC_OUT32(mgag_io, 0, RST);
#endif

  /* TODO: No need to re-enable the video in theory, remove? */
  MGAG_SEQ_OUT8(mgag_io,
	       MGAG_SEQ_IN8(mgag_io,VGA_SEQ_CLOCK) & ~VGA_SR01_DISPLAY_OFF,
	       VGA_SEQ_CLOCK);

  KRN_DEBUG(2, "completed.");
}

/* ----------------------------------------------------------------------------
**	Main functions
** ----------------------------------------------------------------------------
*/

kgi_error_t mgag_chipset_init(mgag_chipset_t *mgag, mgag_chipset_io_t *mgag_io,
			      const kgim_options_t *options)
{
  pcicfg_vaddr_t pcidev = MGAG_PCIDEV(mgag_io);

  KRN_ASSERT(mgag);
  KRN_ASSERT(mgag_io);
  KRN_ASSERT(options);

  KRN_DEBUG(2, "initializing %s %s", mgag->chipset.vendor,
	    mgag->chipset.model);

  if (mgag->flags & MGAG_CF_OLD)
    {
      PCICFG_SET_BASE32(mgag_io->control.base_io,
			pcidev + PCI_BASE_ADDRESS_0);
      PCICFG_SET_BASE32(mgag_io->fb.base_io,
			pcidev + PCI_BASE_ADDRESS_1);
    }
  else
    {
      PCICFG_SET_BASE32(mgag_io->control.base_io,
			pcidev + PCI_BASE_ADDRESS_1);
      PCICFG_SET_BASE32(mgag_io->fb.base_io,
			pcidev + PCI_BASE_ADDRESS_0);
    }

  PCICFG_SET_BASE32(mgag_io->iload.base_io, pcidev + PCI_BASE_ADDRESS_2);

  /* initialize PCI command register */

  pcicfg_out16(PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER, pcidev + PCI_COMMAND);

  if (mgag->flags & MGAG_CF_PRIMARY)
    {
      KRN_DEBUG(2, "chipset already initialized.");	  
    }
  else
    {
      KRN_DEBUG(2, "chipset not initialized: doing reset/power-up");
      /* Power up the chipset */
      mgag_chipset_power_up(mgag, mgag_io);
    }

  /* Reading configuration and saving it
  ** (NB: Most of the VGA part is done by vga_text_chipset_init())
  */
  KRN_DEBUG(2, "chipset initialized, saving configuration...");
  {
    kgi_u_t i;

    for (i = 0; i < MGAG_ECRT_REGS; ++i)
      mgag->ECRT[i] = MGAG_ECRT_IN8(mgag_io, i);
  }

  /* Now proceeds with initialisation
   */
  mgag->chipset.memory = mgag_chipset_memory_count(mgag, mgag_io);

  KRN_NOTICE("%i bytes framebuffer detected.", mgag->chipset.memory);

  /*
  ** Initialize pointer shape access related fields
  */
  mgag_io->ptr_fboffset = mgag->chipset.memory - POINTER_FB_AREA_SIZE;
  mgag_io->flags |= MGAG_POINTER_FB_READY;
  MGAG_EDAC_OUT8(mgag_io, ((mgag_io->ptr_fboffset >> 10) >> 8) & 0x3F,
		 XCURADDH);
  MGAG_EDAC_OUT8(mgag_io, (mgag_io->ptr_fboffset >> 10) & 0xFF, XCURADDL);

  KRN_DEBUG(1, "pointer shape fb offset = %.8x", mgag_io->ptr_fboffset);

  KRN_TRACE(2, mgag_chipset_examine(mgag));

  KRN_TRACE(1, mgag_chipset_probe(mgag,mgag_io));

  /* Calls the VGA-text driver initialization procedure
   */
  vga_text_chipset_init(&(mgag->vga),&(mgag_io->vga),options);

  if (mgag->flags & MGAG_CF_IRQ_CLAIMED)
    {
      mgag_chipset_irq_enable(mgag_io);
    }

  KRN_DEBUG(2, "chipset enabled");

  return KGI_EOK;
}

void mgag_chipset_done(mgag_chipset_t *mgag, mgag_chipset_io_t *mgag_io,
		       const kgim_options_t *options)
{
  pcicfg_vaddr_t pcidev = MGAG_PCIDEV(mgag_io);
  kgi_u_t i;

  if (mgag->flags & MGAG_CF_IRQ_CLAIMED)
    {
      KRN_DEBUG(2, "Disabling all interrupts");
      /* Disable all interrupts */
      mgag_chipset_irq_block(mgag_io, NULL);

      /* Output IRQ stats */
      KRN_DEBUG(1, "IRQ stats: handler_total=%i, not_handled=%i",
		mgag->interrupt.handler_total,
		mgag->interrupt.not_handled);
      KRN_DEBUG(1, "IRQ stats: pick=%i, vsync=%i, vline=%i, ext=%i",
		mgag->interrupt.pick,
		mgag->interrupt.vsync,
		mgag->interrupt.vline,
		mgag->interrupt.ext);
      KRN_DEBUG(1, "IRQ stats (G200+): softrap=%i, warp=%i, warpcache=%i",
		mgag->interrupt.softtrap,
		mgag->interrupt.warp,
		mgag->interrupt.warpcache);
      KRN_DEBUG(1, "IRQ stats (G400+): c2vline=%i, warp1=%i, warpcache1=%i",
		mgag->interrupt.c2vline,
		mgag->interrupt.warp1,
		mgag->interrupt.warpcache1);
    }

  /* Restoring chipset state
   */
  KRN_DEBUG(2, "restoring initial chipset state");

  for (i = 0; i < MGAG_ECRT_REGS; ++i)
    mgag->ECRT[i] = MGAG_ECRT_IN8(mgag_io, i);

  /* Calling the VGA-text driver restore procedure */
  vga_text_chipset_done(&(mgag->vga), &(mgag_io->vga), options);

  /* Finally restoring PCI config regs (saved by the binding driver) */
  pcicfg_out32(mgag->pci.Command, pcidev + PCI_COMMAND);
  pcicfg_out32(mgag->pci.LatTimer, pcidev + PCI_LATENCY_TIMER);
  pcicfg_out32(mgag->pci.IntLine, pcidev + PCI_INTERRUPT_LINE);

  PCICFG_SET_BASE32(mgag->pci.BaseAddr0, pcidev + PCI_BASE_ADDRESS_0);
  PCICFG_SET_BASE32(mgag->pci.BaseAddr1, pcidev + PCI_BASE_ADDRESS_1);
  PCICFG_SET_BASE32(mgag->pci.BaseAddr2, pcidev + PCI_BASE_ADDRESS_2);
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
	kgi_accel_t *a;
	kgi_u_t mul, div, bpd;

	KRN_DEBUG(1, "mgag_chipset_mode_check initiated");

	if (images != 1) {

		KRN_DEBUG(1, "%i images not supported.", images);
		return -KGI_ERRNO(CHIPSET, NOSUP);
	}

	/*	for text16 support we fall back to VGA mode
	**	for unsupported image flags, bail out.
	*/
	if ((img[0].flags & KGI_IF_TEXT16) || 
	    (img[0].fam & KGI_AM_TEXTURE_INDEX)) {
	  KRN_DEBUG(2, "doing VGA-text mode_check");
	  if (mgag->flags & MGAG_CF_PRIMARY) {
	    return vga_text_chipset_mode_check(&mgag->vga, &mgag_io->vga,
					    &mgag_mode->vga, cmd, img,images);
	  } else {
	    kgi_error_t err;
	    err = vga_text_chipset_mode_check(&mgag->vga, &mgag_io->vga,
					      &(mgag_mode->compat.vga),
					      cmd, img,images);
	    /* Provide some hacked "unsupported" resources to offer
	     * a text mode on a secondary display even if theoretically
	     * unsupported.
	     * These do not use the fixed address 0xA0000 aperture but
	     * goes through the usual fb aperture modulo some address
	     * mangling.
	     */
	    KRN_DEBUG(1, "providing unofficial text mode "
		      "on a secondary Gx00/1x64 chipset");
	    /* Init flags */
	    mgag_mode->compat.flags = 0;
	    /* Stores bus sizes */
	    if ((mgag->flags & MGAG_CF_1x64) || (mgag->flags & MGAG_CF_G200))
	      {
		mgag_mode->compat.flags |= MGAG_CF_64BITS_BUS;
	      }
	    else if (mgag->flags & MGAG_CF_G400)
	      {
		mgag_mode->compat.flags |= MGAG_CF_128BITS_BUS;
	      }
	    else
	      {
		KRN_ERROR("Unknown chipset bus size!");
		return -KGI_ERRNO(CHIPSET, NOSUP);
	      }

	    /*	(compat) text16 handling
	     */
	    mgag_mode->compat.text16.meta	= mgag_mode;
	    mgag_mode->compat.text16.meta_io	= mgag_io;
	    mgag_mode->compat.text16.type	= KGI_RT_TEXT16_CONTROL;
	    mgag_mode->compat.text16.prot	= KGI_PF_DRV_RWS;
	    mgag_mode->compat.text16.name	=
	      "Gx00/1x64 *UNSUPPORTED* text16 control";
	    mgag_mode->compat.text16.size.x	= img[0].size.x;
	    mgag_mode->compat.text16.size.y	= img[0].size.y;
	    mgag_mode->compat.text16.virt.x	= img[0].virt.x;
	    mgag_mode->compat.text16.virt.y	= img[0].virt.y;
	    mgag_mode->compat.text16.cell.x	= 9;
	    mgag_mode->compat.text16.cell.y	= 16;
	    mgag_mode->compat.text16.font.x	= 8;
	    mgag_mode->compat.text16.font.y	= 16;
	    mgag_mode->compat.text16.PutText16	=
	      mgag_chipset_compat_put_text16;

	    /*	(compat) texture look up table control
	     */
	    mgag_mode->compat.tlut_ctrl.meta	= mgag_mode;
	    mgag_mode->compat.tlut_ctrl.meta_io	= mgag_io;
	    mgag_mode->compat.tlut_ctrl.type	= KGI_RT_TLUT_CONTROL;
	    mgag_mode->compat.tlut_ctrl.prot	= KGI_PF_DRV_RWS;
	    mgag_mode->compat.tlut_ctrl.name	=
	      "Gx00/1x64 *UNSUPPORTED* tlut control";
	    mgag_mode->compat.tlut_ctrl.Set	= mgag_chipset_compat_set_tlut;

	    /*	(compat) cursor setup
	     */
	    mgag_mode->compat.cursor_ctrl.meta = mgag_mode;
	    mgag_mode->compat.cursor_ctrl.meta_io = mgag_io;
	    mgag_mode->compat.cursor_ctrl.type = KGI_RT_CURSOR_CONTROL;
	    mgag_mode->compat.cursor_ctrl.prot = KGI_PF_DRV_RWS;
	    mgag_mode->compat.cursor_ctrl.name	=
	      "Gx00/1x64 *UNSUPPORTED* text cursor control";
	    mgag_mode->compat.cursor_ctrl.size.x = 1;
	    mgag_mode->compat.cursor_ctrl.size.y = 1;
	    mgag_mode->compat.cursor_ctrl.Show = mgag_chipset_compat_show_hc;
	    mgag_mode->compat.cursor_ctrl.Hide = mgag_chipset_compat_hide_hc;
	    mgag_mode->compat.cursor_ctrl.Undo = mgag_chipset_compat_undo_hc;

	    return err;
	  }
	}

	if (img[0].flags & (KGI_IF_TILE_X | KGI_IF_TILE_Y | KGI_IF_VIRTUAL))
	{
		KRN_DEBUG(1, "image flags %.8x not supported", img[0].flags);
		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	/* check if common attributes are supported.
	 */
	switch (img[0].cam) {

	case 0:
		break;

	default:
		KRN_DEBUG(1, "common attributes %.8x not supported", img[0].cam);
		return -KGI_ERRNO(CHIPSET, INVAL);
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
			return -KGI_ERRNO(CHIPSET, FAILED);
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

			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}

		if ((img[0].size.y >= mgag->chipset.maxdots.y) || (img[0].virt.y >= mgag->chipset.maxdots.y)) {

			KRN_DEBUG(1, "%i (%i) y pixels are too many", img[0].size.y, img[0].virt.y);

			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}

		if ((img[0].virt.x * img[0].virt.y * bpp) > (8 * mgag->chipset.memory)) {

			KRN_DEBUG(1, "not enough memory (%ipf*%if + %ipc)@%ix%i", bpf, img[0].frames, bpc, img[0].virt.x, img[0].virt.y);

			return -KGI_ERRNO(CHIPSET,NOMEM);
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
				return -KGI_ERRNO(CHIPSET, UNKNOWN);
			}
		}
		return KGI_EOK;

	case KGI_TC_CHECK:

		KRN_ASSERT(pp[0] < 8);
		KRN_ASSERT(pp[1] < 8);
		KRN_ASSERT(pp[2] < 8);

#if 0
		if (width != img[0].virt.x) {

KRN_DEBUG(1, "width == %i different from img[0].virt.x == %i",
          width, img[0].virt.x);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
#endif

		if ((img[0].size.x >= mgag->chipset.maxdots.x) ||
		    (img[0].size.y >= mgag->chipset.maxdots.y) ||
		    (img[0].virt.x >= mgag->chipset.maxdots.x) ||
		   ((img[0].virt.y * img[0].virt.x * bpp) > (8 * mgag->chipset.memory))) {

			KRN_DEBUG(1, "resolution too high: %ix%i (%ix%i)",
					img[0].size.x,
					img[0].size.y,
					img[0].virt.x,
					img[0].virt.y);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}

		break;

	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(CHIPSET, UNKNOWN);
	}

	/* Now everything is checked and should be sane. proceed to setup device dependent mode. */

	bpd = kgim_attr_bits(dpm->bpda);

	mgag_mode->mgag.Misc	   = ((crt_mode->x.polarity > 0) ? 0 : VGA_MISC_NEG_HSYNC |
			      (crt_mode->y.polarity > 0) ? 0 : VGA_MISC_NEG_VSYNC | MISC_CLOCK_1X | VGA_MISC_COLOR_IO);
	mgag_mode->mgag.Misc |= VGA_MISC_ENB_RAM;

	mgag_mode->mgag.Offset  = ((img[0].virt.x * bpp) / 128);
#warning check constraints on offset/pitch (see p4-74)

	/* Clearly, the character clocks for VGA mode corresponds here
	 * to 8 pixels in Power Graphic (aka SVGA) Mode.
	 * But this is also linked to the video clock divider (CRTCEXT3).
	 */
	mgag_mode->mgag.HTotal  = (crt_mode->x.total     / 8) - 5;
	mgag_mode->mgag.HdEnd   = (crt_mode->x.width     / 8) - 1;
	mgag_mode->mgag.HsStart = (crt_mode->x.syncstart / 8);
	mgag_mode->mgag.HsEnd   = (crt_mode->x.syncend   / 8);
	mgag_mode->mgag.HbStart = (crt_mode->x.width     / 8) - 1;
	mgag_mode->mgag.HbEnd   = (crt_mode->x.total     / 8) - 1;

	mgag_mode->mgag.VTotal  = crt_mode->y.total - 2;
	mgag_mode->mgag.VdEnd   = crt_mode->y.width - 1;
	mgag_mode->mgag.VsStart = crt_mode->y.syncstart;
	mgag_mode->mgag.VsEnd   = crt_mode->y.syncend;
	mgag_mode->mgag.VbStart = crt_mode->y.width - 1;
	mgag_mode->mgag.VbEnd   = crt_mode->y.total - 1;

	mgag_mode->mgag.CRTC[0x00] =   mgag_mode->mgag.HTotal  & 0xFF;
	mgag_mode->mgag.CRTC[0x01] =   mgag_mode->mgag.HdEnd   & 0xFF;
	mgag_mode->mgag.CRTC[0x02] =   mgag_mode->mgag.HbStart & 0xFF;
	/* NB: Do not touch VGA_CR03_IS_VGA in CRTC[0x03], it freezes
	   the Matrox Mystique (at least a 1064 rev.2) */
	mgag_mode->mgag.CRTC[0x03] =   mgag_mode->mgag.HbEnd   & 0x1F;
	mgag_mode->mgag.CRTC[0x04] =   mgag_mode->mgag.HsStart & 0xFF;
	mgag_mode->mgag.CRTC[0x05] =  (mgag_mode->mgag.HsEnd   & 0x1F) |
				((mgag_mode->mgag.HbEnd   & 0x20) << 2);
	mgag_mode->mgag.CRTC[0x06] =   mgag_mode->mgag.VTotal  & 0xFF;
	mgag_mode->mgag.CRTC[0x07] = ((mgag_mode->mgag.VTotal  & 0x100) >> 8) |
				((mgag_mode->mgag.VdEnd   & 0x100) >> 7) |
				((mgag_mode->mgag.VsStart & 0x100) >> 6) |
				((mgag_mode->mgag.VbStart & 0x100) >> 5) |
				((mgag_mode->mgag.LnComp  & 0x100) >> 4) |
				((mgag_mode->mgag.VTotal  & 0x200) >> 4) |
				((mgag_mode->mgag.VdEnd   & 0x200) >> 3) |
				((mgag_mode->mgag.VsStart & 0x200) >> 2);

	mgag_mode->mgag.CRTC[0x08] = 0x00;
	mgag_mode->mgag.CRTC[0x09] = ((mgag_mode->mgag.LnComp  & 0x200) >> 3) |
				((mgag_mode->mgag.VbStart & 0x200) >> 4);
	mgag_mode->mgag.CRTC[0x0A] = 0x00; /* No text cursor */
	mgag_mode->mgag.CRTC[0x0B] = 0x00; /* No text cursor */
#warning do we need to set the Start Address ?
	mgag_mode->mgag.CRTC[0x0C] = 0x00;
	mgag_mode->mgag.CRTC[0x0D] = 0x00;
	mgag_mode->mgag.CRTC[0x0E] = 0x00; /* No text cursor */
	mgag_mode->mgag.CRTC[0x0F] = 0x00; /* No text cursor */
	mgag_mode->mgag.CRTC[0x10] =   mgag_mode->mgag.VsStart & 0xFF;
	mgag_mode->mgag.CRTC[0x11] =  (mgag_mode->mgag.VsEnd   & 0x0F)
	  | ((mgag->flags & MGAG_CF_IRQ_CLAIMED) ? VGA_CR11_CLEAR_VSYNC_IRQ : VGA_CR11_DISABLE_VSYNC_IRQ);
	mgag_mode->mgag.CRTC[0x12] =   mgag_mode->mgag.VdEnd   & 0xFF;
	mgag_mode->mgag.CRTC[0x13] =   mgag_mode->mgag.Offset  & 0xFF;
	mgag_mode->mgag.CRTC[0x14] =   0x00;
	mgag_mode->mgag.CRTC[0x15] =   mgag_mode->mgag.VbStart & 0xFF;
	mgag_mode->mgag.CRTC[0x16] =   mgag_mode->mgag.VbEnd   & 0xFF;
	mgag_mode->mgag.CRTC[0x17] =   VGA_CR17_CGA_BANKING | VGA_CR17_HGC_BANKING
	  | VGA_CR17_BYTE_MODE | VGA_CR17_ENABLE_SYNC;
	mgag_mode->mgag.CRTC[0x18] =   mgag_mode->mgag.LnComp & 0xFF;
	/* CRTC22, CRTC24, CRTC26 not needed/handled */

#warning do we need to set the Start Address ?
	mgag_mode->mgag.ECRTC[0]   =  (mgag_mode->mgag.Offset  & 0x300) >> 4;
	mgag_mode->mgag.ECRTC[1]   = ((mgag_mode->mgag.HTotal  & 0x100) >> 8) |
				((mgag_mode->mgag.HbStart & 0x100) >> 7) |
				((mgag_mode->mgag.HsStart & 0x100) >> 6) |
				 (mgag_mode->mgag.HbEnd   & 0x40);
	mgag_mode->mgag.ECRTC[2]   = ((mgag_mode->mgag.VTotal  & 0xC00) >> 10) |
				((mgag_mode->mgag.VdEnd   & 0x400) >> 8)  |
				((mgag_mode->mgag.VbStart & 0xC00) >> 7)  |
				((mgag_mode->mgag.VsStart & 0xC00) >> 5)  |
				((mgag_mode->mgag.LnComp  & 0x400) >> 3);
	{
	  int vcsf = 0; /* Video Clocking Scale Factor */
	  switch (bpp) { /* TODO: Do something better than a switch ? */
	  case 8:
	    vcsf = 0; break;
	  case 16:
	    vcsf = 1; break;
	  case 24:
	    vcsf = 2; break;
	  case 32:
	    vcsf = 3; break;
	  default:
	    KRN_DEBUG(0, "%i bpp not supported", bpp);
	    return -KGI_ERRNO(CHIPSET, FAILED);
	  }
	  mgag_mode->mgag.ECRTC[3] = (vcsf & ECRT3_SCALE_MASK)
	    | ECRT3_MGAMODE;
	}
	mgag_mode->mgag.ECRTC[4] = 0x00;
	mgag_mode->mgag.ECRTC[5] = 0x00; /* Not used in non-interlaced mode */
	/* Registers 6 to 8 are left at their reset values */
	mgag_mode->mgag.ECRTC[6] = 0x00;
	mgag_mode->mgag.ECRTC[7] = 0x00;
	mgag_mode->mgag.ECRTC[8] = 0x00;

	/* initialize exported resources */

	r		= &mgag_mode->mgag.fb;
	r->meta		= mgag;
	r->meta_io	= mgag_io;
	r->type		= KGI_RT_MMIO_FRAME_BUFFER;
	r->prot		= KGI_PF_APP | KGI_PF_LIB | KGI_PF_DRV;
	r->name		= "MGA 1x64/Gx00 Framebuffer";
	r->access	= 64 + 32 + 16 + 8;
	r->align	= 64 + 32 + 16 + 8;
	r->size		= r->win.size
	  = mgag->chipset.memory /* - POINTER_FB_AREA_SIZE */;
	r->win.bus	= mgag_io->fb.base_bus;
	r->win.phys	= mgag_io->fb.base_phys;
	r->win.virt	= mgag_io->fb.base_virt;

	r		= &mgag_mode->mgag.iload;
	r->meta		= mgag;
	r->meta_io	= mgag_io;
	r->type		= KGI_RT_MMIO_PRIVATE;
	r->prot		= KGI_PF_LIB | KGI_PF_DRV | KGI_PF_WRITE_ORDER;
	r->name		= "MGA 1x64/Gx00 Pseudo-DMA ILOAD aperture";
	/* TODO: I wonder if accesses should not be on dwords... */
	r->access	= 64 + 32 + 16 + 8;
	r->align	= 64 + 32 + 16 + 8;
	/*	r->size		= r->win.size = MGAG_MGABASE3_SIZE;*/
	r->size		= r->win.size = mgag_io->iload.size;
	r->win.bus	= mgag_io->iload.base_bus;
	r->win.phys	= mgag_io->iload.base_phys;
	r->win.virt	= mgag_io->iload.base_virt;

	r		= &mgag_mode->mgag.control;
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

	a = &mgag_mode->mgag.engine;
	a->meta = mgag;
	a->meta_io = mgag_io;
	a->type = KGI_RT_ACCELERATOR;
	a->prot = KGI_PF_LIB | KGI_PF_DRV; 
	a->name = "Matrox Gx00 graphics engine";
	a->flags |= KGI_AF_DMA_BUFFERS;
	a->buffers = 3;
	a->buffer_size = 8 KB; /* TODO: Should be 7 KB ? */
	a->context = NULL;
	a->context_size = sizeof(mgag_chipset_accel_context_t);
	a->execution.queue = NULL;
#warning initialize a->idle!!!
	a->Init = mgag_chipset_accel_init;
	a->Done = mgag_chipset_accel_done;
	a->Exec = mgag_chipset_accel_exec;

	return KGI_EOK;
}

kgi_resource_t *mgag_chipset_mode_resource(mgag_chipset_t *mgag, 
	mgag_chipset_mode_t *mgag_mode, kgi_image_mode_t *img, 
	kgi_u_t images, kgi_u_t index) 
{
  /* Handles (VGA) text mode specific issues first */
  if (img->fam & KGI_AM_TEXTURE_INDEX) {
    if (mgag->flags & MGAG_CF_PRIMARY) {
      /* Calls the VGA-text driver procedure for a VGA text mode */
      return vga_text_chipset_mode_resource(&(mgag->vga), &(mgag_mode->vga),
					    img, images, index);
    } else {
      /* If we are a secondary card, we do not provide any mode
       * resource, especially, no MMIO fb.
       */
      return NULL;
    }
  }

  /* Then normal graphic mode resources */
  switch (index) {

  case 0:	return (kgi_resource_t *) &mgag_mode->mgag.fb;
  case 1:	return (kgi_resource_t *) &mgag_mode->mgag.iload;
  case 2:	return (kgi_resource_t *) &mgag_mode->mgag.engine;

  }

  return NULL;
}

kgi_resource_t *mgag_chipset_image_resource(mgag_chipset_t *mgag,
	mgag_chipset_mode_t *mgag_mode, kgi_image_mode_t *img,
	kgi_u_t image, kgi_u_t index)
{
  KRN_ASSERT(image == 0);

  /* Handles (VGA) text mode specific issues first */
  if (img[0].fam & KGI_AM_TEXTURE_INDEX) {
    if (mgag->flags & MGAG_CF_PRIMARY) {
      /* We fall back to the VGA-text driver */
      return vga_text_chipset_image_resource(&(mgag->vga),
			&(mgag_mode->vga), img, image, index);
    } else {
      /* We provide our own "unsupported officially" resources. */
      switch (index) {
      case 0: return (kgi_resource_t *) &(mgag_mode->compat.cursor_ctrl);
      case 1: return (kgi_resource_t *) &(mgag_mode->compat.tlut_ctrl);
      case 2: return (kgi_resource_t *) &(mgag_mode->compat.text16);
      }
      return NULL;
    }
  }

  /* Then normal graphic image resources */

  /* NONE YET */

  return NULL;
}

void mgag_chipset_mode_prepare(mgag_chipset_t *mgag,
			       mgag_chipset_io_t *mgag_io,
			       mgag_chipset_mode_t *mgag_mode,
			       kgi_image_mode_t *img,
			       kgi_u_t images)
{
  /* Handle text mode first */
  if (img->fam & KGI_AM_TEXTURE_INDEX)
    {
      kgi_u_t i;
      
      /* VGA mode preparation */
      mgag_io->flags |= MGAG_IF_VGA_MODE;
      
      /* Restores all CRTCEXT registers to their reset value
      ** (very probably all 0x00)
      */
      for (i = 0; i < MGAG_ECRT_REGS; ++i)
	{
	  MGAG_ECRT_OUT8(mgag_io, mgag->ECRT[i], i);
	}
    
      /* Prepare specifically for a VGA mode (probably redundant) */
      MGAG_ECRT_OUT8(mgag_io,
		     MGAG_ECRT_IN8(mgag_io, ECRT3) & ~ECRT3_MGAMODE,
		     ECRT3);

      /* Now calls the VGA-text driver function */
      if (mgag->flags & MGAG_CF_PRIMARY)
	{
	  vga_text_chipset_mode_prepare(&mgag->vga, &mgag_io->vga,
					&mgag_mode->vga, img, images);
	}
      else
	{
	  vga_text_chipset_mode_prepare(&mgag->vga, &mgag_io->vga,
					&(mgag_mode->compat.vga), img, images);
	}

      KRN_DEBUG(2, "prepared for VGA mode");
    
      return; /* Early return */
    }

  /* Normal mode preparation */
  mgag_io->flags &= ~MGAG_IF_VGA_MODE;

  /* Prepare for a SVGA mode */
  MGAG_ECRT_OUT8(mgag_io, MGAG_ECRT_IN8(mgag_io, ECRT3) | ECRT3_MGAMODE,
		 ECRT3);
  
  KRN_DEBUG(2, "prepared for MGA/Gx00 mode");
}

void mgag_chipset_mode_enter(mgag_chipset_t *mgag, mgag_chipset_io_t *mgag_io,
	mgag_chipset_mode_t *mgag_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u_t	i = 0;

	/* Manages a VGA text mode
	 */
	if (img->fam & KGI_AM_TEXTURE_INDEX) {

	  /* Calls the VGA-text driver function */
	  if (mgag->flags & MGAG_CF_PRIMARY) {
	    vga_text_chipset_mode_enter(&mgag->vga, &mgag_io->vga,
					&mgag_mode->vga, img, images);
	  } else {
	    vga_text_chipset_mode_enter(&mgag->vga, &mgag_io->vga,
					&(mgag_mode->compat.vga), img, images);
	  }
#if 0
	  KRN_TRACE(2, mgag_chipset_probe(mgag,mgag_io));
#endif

	  return; /* Early return */
	}

#if 0
	/* turning off display */

	MGAG_SEQ_OUT8(mgag_io, MGAG_SEQ_IN8(mgag_io, VGA_SEQ_CLOCK) | VGA_SR01_DISPLAY_OFF, VGA_SEQ_CLOCK);

	MGAG_ECRT_OUT8(mgag_io, MGAG_ECRT_IN8(mgag_io, ECRT1) | ECRT1_HSYNCOFF | ECRT1_VSYNCOFF, ECRT1);
#endif
	/* programming mode. */

	/* Programming the Misc - preserve the clock selection bits */
	MGAG_MISC_OUT8(mgag_io,
		      (MGAG_MISC_IN8(mgag_io) & VGA_MISC_CLOCK_MASK)
		      | (mgag_mode->mgag.Misc & ~VGA_MISC_CLOCK_MASK));

	for (i = 0; i < NrCRTRegs; i++) {

		MGAG_CRT_OUT8(mgag_io, mgag_mode->mgag.CRTC[i], i);
	}

	for (i = 0; i < NrECRTRegs; i++) {

		MGAG_ECRT_OUT8(mgag_io, mgag_mode->mgag.ECRTC[i], i);
	}

	/* need to reenable the display, but this can't be the space.
	   everything else has to be complete by that time. */

#if 0
	/* turning ON display */

	MGAG_SEQ_OUT8(mgag_io, MGAG_SEQ_IN8(mgag_io, VGA_SEQ_CLOCK) & ~VGA_SR01_DISPLAY_OFF, VGA_SEQ_CLOCK);

	MGAG_ECRT_OUT8(mgag_io, MGAG_ECRT_IN8(mgag_io, ECRT1) & ~ECRT1_HSYNCOFF & ~ECRT1_VSYNCOFF, ECRT1);
#endif

	/* Double-checking alpha control initial value */
	MGAG_GC_OUT32(mgag_io, 0x00000001, ALPHACTRL);

	/* TODO: Should set an initial value for PITCH, MACCESS and DSTORG */

	mgag->mode = mgag_mode;

#if 0
	KRN_TRACE(2, mgag_chipset_probe(mgag,mgag_io));
#endif
}

void mgag_chipset_mode_leave(mgag_chipset_t *mgag, mgag_chipset_io_t *mgag_io,
	mgag_chipset_mode_t *mgag_mode, kgi_image_mode_t *img, kgi_u_t images)
{
    mgag->mode = NULL;
}

/* ----------------------------------------------------------------------------
**	Matrox Gx00 chipset driver meta language definition
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2001	Johan Karlberg
**					Rodolphe Ortalo
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Rodolphe_Ortalo
**
**	$Log: Gx00-meta.h,v $
**	Revision 1.1.1.1  2000/04/18 08:51:24  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef	_chipset_Matrox_Gx00_meta_h
#define	_chipset_Matrox_Gx00_meta_h

#include "chipset/IBM/VGA-text-meta.h"

#define MGAG_ECRT_REGS 0x09

typedef enum
{
  MGAG_IF_VGA_MODE  = (0x1 << 0),
  MGAG_POINTER_FB_READY = (0x1 << 1)

} mgag_chipset_io_flags_t;

typedef	struct {

  vga_text_chipset_io_t vga;
  
  mgag_chipset_io_flags_t flags;
  
  mem_region_t control;
  mem_region_t fb;
  mem_region_t iload;
  mem_region_t text16fb;
  irq_line_t irq;

  kgi_u32_t ptr_fboffset;

  kgim_io_out32_fn *GCOut8;
  kgim_io_in32_fn  *GCIn8;
  kgim_io_out32_fn *GCOut32;
  kgim_io_in32_fn  *GCIn32;

  kgim_io_out8_fn *ECRTOut8;
  kgim_io_in8_fn *ECRTIn8;

} mgag_chipset_io_t;

/*
** Meta-language macros
*/
#define MGAG_PCIDEV(ctx)		((ctx)->vga.kgim.pcidev)
/*
 * for the MGA control region direct access.
 */
#define	MGAG_GC_OUT8(ctx, val, reg)	(ctx)->GCOut8((ctx), (val), (reg))
#define	MGAG_GC_IN8(ctx, reg)		(ctx)->GCIn8((ctx), (reg))
#define	MGAG_GC_OUT32(ctx, val, reg)	(ctx)->GCOut32((ctx), (val), (reg))
#define	MGAG_GC_IN32(ctx, reg)		(ctx)->GCIn32((ctx), (reg))
/*
 * MGA extended-CRT registers
 */
#define MGAG_ECRT_OUT8(ctx, val, reg)   (ctx)->ECRTOut8((ctx), (val), (reg))
#define MGAG_ECRT_IN8(ctx, reg)         (ctx)->ECRTIn8((ctx), (reg))
/*
 * Access to the VGA IO
 */
#define MGAG_SEQ_OUT8(ctx, val, reg)    VGA_SEQ_OUT8(&(ctx->vga),val,reg)
#define MGAG_SEQ_IN8(ctx, reg)          VGA_SEQ_IN8(&(ctx->vga),reg)
#define MGAG_CRT_OUT8(ctx, val, reg)    VGA_CRT_OUT8(&(ctx->vga),val,reg)
#define MGAG_CRT_IN8(ctx, reg)          VGA_CRT_IN8(&(ctx->vga),reg)
#define MGAG_ATC_OUT8(ctx, val, reg)    VGA_ATC_OUT8(&(ctx->vga),val,reg)
#define MGAG_ATC_IN8(ctx, reg)          VGA_ATC_IN8(&(ctx->vga),reg)
#define MGAG_GRC_OUT8(ctx, val, reg)    VGA_GRC_OUT8(&(ctx->vga),val,reg)
#define MGAG_GRC_IN8(ctx, reg)          VGA_GRC_IN8(&(ctx->vga),reg)
#define MGAG_MISC_OUT8(ctx, val)        VGA_MISC_OUT8(&((ctx)->vga),(val))
#define MGAG_MISC_IN8(ctx)              VGA_MISC_IN8(&((ctx)->vga))

typedef enum
{

  MGAG_CF_64BITS_BUS = (0x01 << 0),
  MGAG_CF_128BITS_BUS = (0x01 << 1)

} mgag_chipset_compat_mode_flags_t;

typedef union
{
  kgim_chipset_mode_t kgim;
  vga_text_chipset_mode_t vga;

  struct {

    vga_text_chipset_mode_t vga;

    mgag_chipset_compat_mode_flags_t flags;

    kgi_text16_t text16;
    kgi_tlut_t tlut_ctrl;
    kgi_marker_t cursor_ctrl;

  } compat;    

  struct {

    kgim_chipset_mode_t     kgim;

    kgi_mmio_region_t	control, fb, iload;

    kgi_u32_t		VideoControl, Offset, LnComp,
      HTotal, HdEnd, HsStart, HsEnd, HbStart, HbEnd,
      VTotal, VdEnd, VsStart, VsEnd, VbStart, VbEnd;

    kgi_u8_t		Misc, CRTC[NrCRTRegs], ECRTC[NrECRTRegs];

  } mgag;

} mgag_chipset_mode_t;

typedef enum {

  /* chipsets */
  MGAG_CF_1x64    = (0x01 << 1),
  MGAG_CF_G200    = (0x01 << 2),
  MGAG_CF_G400    = (0x01 << 3),

  /* capabilities */
  MGAG_CF_OLD     = (0x01 << 4),
  MGAG_CF_SGRAM	  = (0x01 << 5),

  /* driver status */
  MGAG_CF_PRIMARY = (0x01 << 6),
  MGAG_CF_IRQ_CLAIMED = (0x01 << 7)

} mgag_chipset_flags_t;

typedef struct
{
  kgim_chipset_t chipset;

  /* VGA text mode information */
  vga_text_chipset_t vga;

  mgag_chipset_io_t     *io;
  mgag_chipset_mode_t   *mode;
  mgag_chipset_flags_t  flags;

  /* Initial config (read from board) */
  struct { /* PCI registers */

    kgi_u32_t	Command, LatTimer, IntLine,
      BaseAddr0, BaseAddr1, BaseAddr2,
      BaseAddr3, BaseAddr4, RomAddr;

  } pci;

  kgi_u8_t ECRT[MGAG_ECRT_REGS];

  struct { /* Control registers */
    /* None yet */
  } ctrl;

  struct {

    kgi_u_t c2vline, warp1, warpcache1, /* G400+ */
      softtrap, warp, warpcache,        /* G200+ */
      pick, vsync, vline, ext,          /* 1x64+ */
      handler_total, not_handled;
    
  } interrupt;

} mgag_chipset_t;

KGIM_META_IRQ_HANDLER_FN(mgag_chipset)
KGIM_META_INIT_FN(mgag_chipset)
KGIM_META_DONE_FN(mgag_chipset)
KGIM_META_MODE_CHECK_FN(mgag_chipset)
KGIM_META_MODE_RESOURCE_FN(mgag_chipset)
KGIM_META_MODE_PREPARE_FN(mgag_chipset)
KGIM_META_MODE_ENTER_FN(mgag_chipset)
KGIM_META_MODE_LEAVE_FN(mgag_chipset)
KGIM_META_IMAGE_RESOURCE_FN(mgag_chipset)

#endif	/* #ifdef _chipset_Matrox_Gx00_meta_h */

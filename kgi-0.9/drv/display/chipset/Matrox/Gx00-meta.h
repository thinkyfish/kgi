/* ----------------------------------------------------------------------------
**	Matrox Gx00 chipset driver meta language definition
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Johan Karlberg
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Johan_Karlberg
**
**	$Log: Gx00-meta.h,v $
*/
#ifndef	_chipset_Matrox_Gx00_meta_h
#define	_chipset_Matrox_Gx00_meta_h

#warning Your meta language prefix is mgag_.
/*	every struct, define, function etc. has to have that prefix.
*/

typedef struct {

	kgi_u_t		sysm, sysn, sysp;

	kgi_u32_t	option1, option2, option3;

} chipset_config_s;

typedef	struct {

	kgim_chipset_io_t	kgim;

	kgim_io_out32_fn	*GCOut8;
	kgim_io_in32_fn		*GCIn8;
	kgim_io_out32_fn	*GCOut32;
	kgim_io_in32_fn		*GCIn32;

	mem_region_t		control,
				fb,
				iload;

	irq_line_t		irq;

} mgag_chipset_io_t;

/* for the MGA control region direct access. */

#define MGAG_PCIDEV(ctx)		((ctx)->kgim.pcidev)

#define	MGAG_GC_OUT8(ctx, val, reg)	(ctx)->GCOut8((ctx), (val), (reg))
#define	MGAG_GC_IN8(ctx, reg)		(ctx)->GCIn8((ctx), (reg))
#define	MGAG_GC_OUT16(ctx, val, reg)	(ctx)->GCOut16((ctx), (val), (reg))
#define	MGAG_GC_IN16(ctx, reg)		(ctx)->GCIn16((ctx), (reg))
#define	MGAG_GC_OUT32(ctx, val, reg)	(ctx)->GCOut32((ctx), (val), (reg))
#define	MGAG_GC_IN32(ctx, reg)		(ctx)->GCIn32((ctx), (reg))

#warning for VGA text support, fall back to the vga-text meta language!

typedef struct {

	kgim_chipset_mode_t     kgim;

	kgi_mmio_region_t	control, fb, iload;

	kgi_u32_t		VideoControl, Offset, LnComp,
				HTotal, HdEnd, HsStart, HsEnd, HbStart, HbEnd,
				VTotal, VdEnd, VsStart, VsEnd, VbStart, VbEnd;

	kgi_u8_t		Misc, CRTC[NrCRTRegs], ECRTC[NrECRTRegs];

} mgag_chipset_mode_t;

typedef enum {

	/* chipsets */

	MGAG_CF_1x64		= (0x01 << 1),
	MGAG_CF_G200		= (0x01 << 2),
	MGAG_CF_G400		= (0x01 << 3),

	/* capabilities */

	MGAG_CF_OLD		= (0x01 << 4),
	MGAG_CF_SGRAM		= (0x01 << 5),

	/* driver status */

	MGAG_CF_PRIMARY		= (0x01 << 6),
	MGAG_CF_IRQ_CLAIMED	= (0x01 << 7)

} mgag_chipset_flags_t;

typedef struct {

	struct {

		kgi_u32_t	Command, LatTimer, IntLine,
				BaseAddr0, BaseAddr1, BaseAddr2,
				BaseAddr3, BaseAddr4, RomAddr;

	} pci;

	struct {

		kgi_u_t		c2vline, warp1, warpcache1,	/* G400+ */
				softtrap, warp, warpcache,	/* G200+ */
				pick, vsync, vline, ext;	/* 1x64+ */

	} interrupt;

	kgim_chipset_t		chipset;

	mgag_chipset_io_t	*io;
	mgag_chipset_mode_t	*mode;
	mgag_chipset_flags_t	flags;

} mgag_chipset_t;

KGIM_META_IRQ_HANDLER_FN(mgag_chipset)
KGIM_META_INIT_FN(mgag_chipset)
KGIM_META_DONE_FN(mgag_chipset)
KGIM_META_MODE_CHECK_FN(mgag_chipset)
KGIM_META_MODE_RESOURCE_FN(mgag_chipset)
KGIM_META_MODE_ENTER_FN(mgag_chipset)
KGIM_META_MODE_LEAVE_FN(mgag_chipset)

#endif	/* #ifdef _chipset_Matrox_Gx00_meta_h */

/* ----------------------------------------------------------------------------
**	##VENDOR## ##META## chipset definitions
** ----------------------------------------------------------------------------
**	Copyright (C)	2000	##AUTHOR_NAME##
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: ##META##-meta.h,v $
*/
#ifndef	_chipset_##VENDOR##_##META##_meta_h
#define	_chipset_##VENDOR##_##META##_meta_h

/**	If you need VGA text mode support, you must use the VGA-text-meta
***	language. Please delete this comment then or both this comment and
***	the #include directive below if you don't need VGA text mode support.
**/
#include "chipset/IBM/VGA-text-meta.h"

typedef enum
{
	##META##_IF_VGA_DAC	= 0x00000001	/* use VGA or extended DAC */

} ##meta##_chipset_io_flags_t;

/**	The structs below are intended as a sample framework and should
***	guide you what to put where. Please add the appropriate items
***	and delete the guiding comments around.
**/
typedef	struct
{
	vga_chipset_io_t	vga;

	##meta##_chipset_io_flags_t	flags;

#warning	Add io, mem, or irq line resources as required
	io_region_t		io;		/* I/O region		*/
	mem_region_t		mem;		/* control MMIO aperture*/
	mem_region_t		rom;		/* onboard ROM aperture	*/
	irq_line_t		irq;		/* IRQ line		*/

#warning	Add meta language (chipset) specific I/O functions as required
	kgim_io_out32_fn	*CtrlOut32;
	kgim_io_in32_fn		*CtrlIn32;

} ##meta##_chipset_io_t;


#warning	Add meta language specific I/O function macros as required

#define	##META##_CTRL_OUT32(ctx, val, reg)	(ctx)->CtrlOut32((ctx), (val), (reg))
#define	##META##_CTRL_IN32(ctx, reg)		(ctx)->CtrlIn32((ctx), (reg))
#define	##META##_PCIDEV(ctx)			((ctx)->vga.kgim.pcidev)

typedef union
{
	kgim_chipset_mode_t	kgim;
	vga_chipset_mode_t	vga;

	struct {

		kgim_chipset_mode_t	kgim;

		/**
		***	All chipset state info needed to setup a mode.
		***	The fields below may serve you as a guide.
		**/
		kgi_u32_t		VideoControl, ScreenStride;
		kgi_u32_t		HTotal, HgEnd, HsStart, HsEnd, HbEnd;
		kgi_u32_t		VTotal, VsStart, VsEnd, VbEnd;
		kgi_u32_t		ScreenBase, ScreenBaseRight;

		/**
		***	resources exported in any mode (not all need to be
		***	available in every mode)
		**/
		kgi_mmio_region_t	video_memory_aperture;
		kgi_accel_t		accelerator;

	} ##meta##;

} ##meta##_chipset_mode_t;


/*	driver global state
*/
typedef enum
{
	##META##_CF_RESTORE_INITIAL	= 0x00000001,	/* restore init. state	*/
	##META##_CF_IRQ_CLAIMED	= 0x00000002	/* IRQ line claimed	*/

} ##meta##_chipset_flags_t;

typedef struct
{
	kgim_chipset_t		chipset;

	vga_chipset_t		vga;

	/**
	***	inital state of VGA-extensions
	**/
	kgi_u16_t		vga_SEQ_ControlReg;
	kgi_u16_t		vga_GRC_Mode640Reg;

	##meta##_chipset_flags_t	flags;	/* driver global flags	*/
	##meta##_chipset_mode_t	*mode;	/* current mode (if set)	*/

#if (DEBUG_LEVEL >= 0)
	/**
	***	additional debugging state, e.g. IRQ counts, etc.
	**/
#endif

	/**
	***	non-VGA initial state
	**/
	struct { kgi_u32_t

		ScreenBase, ScreenStride, 
		HTotal, HgEnd, HbEnd, HsStart, HsEnd,
		VTotal, VbEnd, VsStart, VsEnd, VideoControl,
		InterruptLine;

	} Ctrl;

	struct { kgi_u32_t

		Command, LatTimer, IntLine, BaseAddr0, BaseAddr1, RomAddr;
	} pci;

} ##meta##_chipset_t;

KGIM_META_IRQ_HANDLER_FN(##meta##_chipset)
KGIM_META_INIT_FN(##meta##_chipset)
KGIM_META_DONE_FN(##meta##_chipset)
KGIM_META_MODE_CHECK_FN(##meta##_chipset)
KGIM_META_MODE_RESOURCE_FN(##meta##_chipset)
KGIM_META_MODE_PREPARE_FN(##meta##_chipset)
KGIM_META_MODE_ENTER_FN(##meta##_chipset)
KGIM_META_MODE_LEAVE_FN(##meta##_chipset)

#endif	/* #ifdef _chipset_##VENDOR##_##meta##_meta_h */

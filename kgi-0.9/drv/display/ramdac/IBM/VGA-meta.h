/* ----------------------------------------------------------------------------
**	VGA ramdac meta language definition
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Jon Taylor
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Jon_Taylor
**
**	$Log: VGA-meta.h,v $
*/
#ifndef	_ramdac_IBM_VGA_meta_h
#define	_ramdac_IBM_VGA_meta_h

typedef kgim_chipset_io_t vga_ramdac_io_t;

#define	VGA_DAC_OUTS8(ctx, r, b, c)	KGIM_DAC_OUTS8(ctx, r, b, c)
#define	VGA_DAC_INS8(ctx, r, b, c)	KGIM_DAC_INS8(ctx, r, b, c)
#define	VGA_DAC_OUT8(ctx, val, reg)	KGIM_DAC_OUT8(ctx, val, reg)
#define	VGA_DAC_IN8(ctx, reg)		KGIM_DAC_IN8(ctx, reg)

typedef struct vga_ramdac_mode_record_s vga_ramdac_mode_record_t;

typedef struct
{
	kgim_ramdac_mode_t kgim;
	const vga_ramdac_mode_record_t *rec;
	
	struct 
	{
		struct 
		{ 
			kgi_u_t x,y; 
		} pos, hot, shift;
	} ptr;

	kgi_u8_t clut[3 * 256];

} vga_ramdac_mode_t;

typedef struct
{
	kgim_ramdac_t ramdac;
	vga_ramdac_io_t *io;

	kgi_u_t	rev;
	
	/*	Initial state
	*/

	kgi_u8_t clut[3 * 256];
/*	kgi_u8_t cursor_clut[3 * 4];
	kgi_u8_t cursor_data[];
*/

} vga_ramdac_t;

KGIM_META_INIT_FN(vga_ramdac)
KGIM_META_DONE_FN(vga_ramdac)
KGIM_META_MODE_CHECK_FN(vga_ramdac)
KGIM_META_MODE_ENTER_FN(vga_ramdac)

#endif	/* #ifndef _ramdac_IBM_VGA_meta_h */

/* ----------------------------------------------------------------------------
**	Matrox Gx00 ramdac meta definitions
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

#ifndef	_ramdac_Matrox_Gx00_meta_h
#define	_ramdac_Matrox_Gx00_meta_h

#warning your meta language prefix is Gx00 or gx00_, not MGAG/mgag.

typedef kgim_chipset_io_t mgag_ramdac_io_t;

#define MGAG_PCIDEV(ctx)		KGIM_PCIDEV(ctx)

#define MGAG_DAC_OUTS8(ctx, r, b, c)	KGIM_DAC_OUTS8((ctx), (r), (b), (c))
#define MGAG_DAC_INS8(ctx, r, b, c)	KGIM_DAC_INS8((ctx), (r), (b), (c))
#define MGAG_DAC_OUT8(ctx, val, reg)	KGIM_DAC_OUT8((ctx), (val), (reg))
#define MGAG_DAC_IN8(ctx, reg)		KGIM_DAC_IN8((ctx), (reg))

typedef struct {

	kgi_u8_t			depth;

	kgi_u_t				ilut;
	
	kgi_u_t				pal_size;

	kgi_u_t				r_shift;
	kgi_u_t				g_shift;
	kgi_u_t				b_shift;

	kgi_attribute_mask_t		dam;

	const kgi_u8_t			*bpda;

} mgag_ramdac_mode_record_t;

typedef struct {

	kgi_u8_t			xzoom,
					xgenctrl,
					miscctrl,
					composite;

	kgim_ramdac_mode_t		kgim;

	const mgag_ramdac_mode_record_t	*rec;

} mgag_ramdac_mode_t;

typedef enum {

	MGAG_CF_SGRAM	= (0x01 << 1)

} mgag_ramdac_flags_t;

typedef struct {

	kgim_ramdac_t		ramdac;
	mgag_ramdac_io_t	*io;

	mgag_ramdac_flags_t	flags;

	struct {

		kgi_u8_t	xzoom,
				xgenctrl,
				xmulctrl,
				pixrdmsk;
		
		kgi_u8_t	clut[256*3];

	} saved;

} mgag_ramdac_t;

KGIM_META_INIT_FN(mgag_ramdac)
KGIM_META_DONE_FN(mgag_ramdac)
KGIM_META_MODE_CHECK_FN(mgag_ramdac)
KGIM_META_MODE_PREPARE_FN(mgag_ramdac)
KGIM_META_MODE_ENTER_FN(mgag_ramdac)

#endif	/* #ifndef _ramdac_Matrox_Gx00_meta_h	*/

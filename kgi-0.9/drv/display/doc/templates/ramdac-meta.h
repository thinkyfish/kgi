/* ----------------------------------------------------------------------------
**	##VENDOR## ##META## ramdac meta definition
** ----------------------------------------------------------------------------
**	Copyright (C)	2000	##AUTHOR_NAME##
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	##AUTHOR##
**
**	$Log: ##META##-meta.h,v $
*/
#ifndef	_ramdac_##VENDOR##_##META##_meta_h
#define	_ramdac_##VENDOR##_##META##_meta_h

typedef kgim_chipset_io_t ##meta##_ramdac_io_t;
#define	##META##_PCIDEV(ctx)			KGIM_PCIDEV(ctx)
#define	##META##_DAC_OUTS8(ctx, r, b, c)	KGIM_DAC_OUTS8(ctx, r, b, c)
#define	##META##_DAC_INS8(ctx, r, b, c)	KGIM_DAC_INS8(ctx, r, b, c)
#define	##META##_DAC_OUT8(ctx, val, reg)	KGIM_DAC_OUT8(ctx, val, reg)
#define	##META##_DAC_IN8(ctx, reg)		KGIM_DAC_IN8(ctx, reg)

typedef struct ##meta##_ramdac_mode_record_s ##meta##_ramdac_mode_record_t;

typedef struct
{
	kgim_ramdac_mode_t		kgim;

	const ##meta##_ramdac_mode_record_t	*rec;
	struct {
		struct { kgi_u_t x,y; }
		pos, hot, shift;
	}				ptr;

#	warning add mode dependent state/registers here.

	kgi_u8_t			clut[3*256];

} ##meta##_ramdac_mode_t;

typedef struct
{
	kgim_ramdac_t		ramdac;

	kgi_u_t	rev;

	/*	initial state
	*/
#warning	add initial state/registers to save here.
	kgi_u8_t	clut[3*256];
	kgi_u8_t	cursor_clut[3*4];
	kgi_u8_t	cursor_data[];

} ##meta##_ramdac_t;

KGIM_META_INIT_FN(##meta##_ramdac)
KGIM_META_DONE_FN(##meta##_ramdac)
KGIM_META_MODE_CHECK_FN(##meta##_ramdac)
KGIM_META_MODE_ENTER_FN(##meta##_ramdac)

#endif	/* #ifndef _ramdac_##VENDOR##_##META##_meta_h	*/

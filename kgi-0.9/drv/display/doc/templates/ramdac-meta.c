/* -----------------------------------------------------------------------------
**	##VENDOR## ##META## ramdac implementation
** -----------------------------------------------------------------------------
**	Copyright (C)	2000	##AUTHOR_NAME##
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**
**	$Log: ##META##-meta.c,v $
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	##AUTHOR##
#define	KGIM_RAMDAC_DRIVER	"$Revision: 1.1 $"

#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif

#warning include register definitions here.
#include "ramdac/##VENDOR##/##META##.h"
#include "ramdac/##VENDOR##/##META##-meta.h"

/*
**	I/O helper functions
*/
#warning implement I/O helper functions to access extended registers.
static inline void ##META##_EDAC_OUT8(##meta##_ramdac_io_t *##meta##_io, 
	kgi_u8_t val, kgi_u8_t reg)
{
	KRN_DEBUG(3, "%.2x -> EDAC%.2X", val, reg);
}

static inline kgi_u8_t ##META##_EDAC_IN8(##meta##_ramdac_io_t *##meta##_io,
	kgi_u8_t reg)
{
	kgi_u8_t val;

	val = 0xFF;
	KRN_DEBUG(3, "EDAC%.2X -> %.2x", reg, val);
	return val;
}

#warning define	magic and shift for palette entry conversions.
/***	##META##_MAGIC * device_value == 16bit_palette_value
****	16bit_value >> ##META##_SHIFT == device_value
****
****			MAGIC		SHIFT
****	8bit device:	0x0101		8
****	6bit device:	0x0410		10
***/
#define	##META##_MAGIC	0x0101
#define	##META##_SHIFT	8

struct ##meta##_ramdac_mode_record_s
{
#warning add mode specific state/operation mode info here.

	kgi_u8_t		ilut;
	kgi_u8_t		alut;

	kgi_ratio_t		best_lclk;

	kgi_attribute_mask_t	dam;
	const kgi_u8_t		*bpda;

	kgi_u8_t		frames;
};

#warning add all modes and their state info here, e.g.
static const ##meta##_ramdac_mode_record_t ##meta##_ramdac_mode[] =
{
	{ 1, 0, {1,1},	KGI_AM_TEXT, KGI_AS_448,	1 },

	{ 1, 0, {1,4},	KGI_AM_I,    KGI_AS_8,		1 },
	{ 0, 1, {1,4},	KGI_AM_RGB,  KGI_AS_332,	1 }
};
#define	##META##_RAMDAC_NR_MODES (sizeof(##meta##_ramdac_mode)/sizeof(##meta##_ramdac_mode[0]))

static inline const ##meta##_ramdac_mode_record_t *##meta##_ramdac_mode_record(
	kgi_attribute_mask_t fam, const kgi_u8_t *bpfa)
{
	kgi_u_t i;

	for (i = 0; i < ##META##_RAMDAC_NR_MODES; i++) {

		if (##meta##_ramdac_mode[i].dam != fam) {

			continue;
		}
		if (kgim_strcmp(##meta##_ramdac_mode[i].bpda, bpfa) != 0) {

			continue;
		}
		break;
	}
	return (i == ##META##_RAMDAC_NR_MODES) 
		? NULL : ##meta##_ramdac_mode + i;
}




#define	LCLK(m,d)	((dpm->lclk.mul == m) && (dpm->lclk.div == d))
#define	RCLK(m,d)	((dpm->rclk.mul == m) && (dpm->rclk.div == d))

/*
** ----	text16 context functions ----------------------------------------------
*/
#define	##META##_RAMDAC_IO(ctx)	\
	KGIM_SUBSYSTEM_IO((kgim_display_t *) ctx->meta_object, ramdac)

#define	##META##_RAMDAC_MODE(ctx)	\
	KGIM_SUBSYSTEM_MODE((kgim_display_mode_t *) ctx->meta_mode, ramdac)
#if 0
static void ##meta##_text16_set_ilut(kgic_mode_text16context_t *ctx,
	kgic_ilut_entries_t *ilut)
{
	##meta##_ramdac_io_t   *##meta##_io   = ##META##_RAMDAC_IO(ctx);
	##meta##_ramdac_mode_t *##meta##_mode = ##META##_RAMDAC_MODE(ctx);
	kgi_u_t	cnt, src, dst;

	KRN_ASSERT(ilut->img == 0);
	KRN_ASSERT(ilut->lut == 0);
	KRN_ASSERT(ilut->cnt > 0);

	cnt = ilut->cnt;
	dst = ilut->cnt * 3;
	src = 0;

	while (cnt--) {

		if (ilut->am & KGI_AM_COLOR1) {

			##meta##_mode->clut[dst+0] = 
				ilut->data[src++] >> ##META##_SHIFT;
		}
		if (ilut->am & KGI_AM_COLOR2) {

			##meta##_mode->clut[dst+1] =
				ilut->data[src++] >> ##META##_SHIFT;
		}
		if (ilut->am & KGI_AM_COLOR3) {

			##meta##_mode->clut[dst+2] =
				ilut->data[src++] >> ##META##_SHIFT;
		}
		dst += 3;
	}

	src = 3 * ilut->idx;
	cnt = 3 * ilut->cnt;

#warning load palette data into device.
}
#endif
#if 0
void ##meta##_text16_hp_set_shape(kgic_mode_text16context_t *ctx,
	kgi_pointer64x64 *ptr)
{
	##meta##_ramdac_io_t   *##meta##_io   = ##META##_RAMDAC_IO(ctx);
	##meta##_ramdac_mode_t *##meta##_mode = ##META##_RAMDAC_MODE(ctx);
	kgi_u_t i;

	##meta##_mode->ptr.hot.x = ptr->hotx;
	##meta##_mode->ptr.hot.y = ptr->hoty;

	/*	update private cursor mode state
	*/
	switch (ptr->mode) {

	case KGI_PM_WINDOWS:
		break;

	case KGI_PM_X11:
		break;

	case KGI_PM_THREE_COLOR:
		break;

	default:
		KRN_INTERNAL_ERROR;
	}

#warning set cursor pattern in device.

	/*	set colors
	*/
#warning set cursor colors.
	for (i = 0; i < 3; i++) {

		/* set ptr->col[i].r	*/
		/* set ptr->col[i].g	*/
		/* set ptr->col[i].b	*/
	}
}
#endif

static void ##meta##_text16_hp_show(kgic_mode_text16context_t *ctx, 
	kgi_u_t x, kgi_u_t y)
{
	##meta##_ramdac_io_t   *##meta##_io   = ##META##_RAMDAC_IO(ctx);
	##meta##_ramdac_mode_t *##meta##_mode = ##META##_RAMDAC_MODE(ctx);

	if ((##meta##_mode->ptr.pos.x != x) || (##meta##_mode->ptr.pos.y != y)) {

		##meta##_mode->ptr.pos.x = x;
		##meta##_mode->ptr.pos.y = y;

		x += ##meta##_mode->ptr.shift.x - ##meta##_mode->ptr.hot.x;
		y += ##meta##_mode->ptr.shift.y - ##meta##_mode->ptr.hot.y;

#warning set hardware cursor position.
	}
}

static void ##meta##_text16_hp_hide(kgic_mode_text16context_t *ctx)
{
	##meta##_ramdac_io_t   *##meta##_io   = ##META##_RAMDAC_IO(ctx);
	##meta##_ramdac_mode_t *##meta##_mode = ##META##_RAMDAC_MODE(ctx);

	##meta##_mode->ptr.pos.x = ##meta##_mode->ptr.pos.y = 0x7FF;

#warning hide hardware cursor (e.g. set postion outside visible range)
}

#undef	##META##_RAMDAC_MODE
#undef	##META##_RAMDAC_IO
/*
** ----	end of text16 context functions ---------------------------------------
*/

kgi_error_t ##meta##_ramdac_init(##meta##_ramdac_t *##meta##,
	##meta##_ramdac_io_t *##meta##_io, const kgim_options_t *options)
{
#warning save initial ramdac state here.

	return KGI_EOK;
}

void ##meta##_ramdac_done(##meta##_ramdac_t *##meta##,
	##meta##_ramdac_io_t *##meta##_io, const kgim_options_t *options)
{
#warning restore initial state here.
}

kgi_error_t ##meta##_ramdac_mode_check(##meta##_ramdac_t *##meta##,
	##meta##_ramdac_io_t *##meta##_io,
	##meta##_ramdac_mode_t *##meta##_mode,
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u_t lclk;
	kgi_dot_port_mode_t *dpm;
	const ##meta##_ramdac_mode_record_t *rec;

	KRN_ASSERT((1 == images) && (NULL != img));
	KRN_ASSERT(NULL != img->out);

	dpm = img->out;
	rec = ##meta##_ramdac_mode_record(img[0].fam, img[0].bpfa);
	if (rec == NULL) {

		KRN_DEBUG(1, "could not handle dot attributes (dam %.8x)",
			dpm->dam);
		return -E(RAMDAC, UNKNOWN);
	}

	switch (cmd) {

	case KGI_TC_PROPOSE:
		dpm->flags &= ~KGI_DPF_TP_2XCLOCK;
		dpm->flags |= rec->alut ? KGI_DPF_CH_ALUT : 0;
		dpm->flags |= rec->ilut ? KGI_DPF_CH_ILUT : 0;

		dpm->lclk = rec->best_lclk;
		dpm->rclk.mul = dpm->rclk.div = 1;
		dpm->dam  = rec->dam;
		dpm->bpda = rec->bpda;

		if (0 == img[0].frames) {

			img[0].frames = rec->frames;
		}

		KRN_DEBUG(2, " lclk %i:%i, dclk %i:%i", dpm->lclk.mul,
			dpm->lclk.div, dpm->rclk.mul, dpm->rclk.div);

 		return KGI_EOK;

	case KGI_TC_LOWER:

		lclk = dpm->dclk * dpm->lclk.mul / dpm->lclk.div;

		if ((lclk < ##meta##->ramdac.lclk.min) ||
			(dpm->dclk < ##meta##->ramdac.dclk.min)) {

			KRN_DEBUG(1, "%i Hz DCLK is too low", dpm->dclk);
			return -E(RAMDAC, INVAL);
		}
		if ((lclk > ##meta##->ramdac.lclk.max) || 
			(dpm->dclk > ##meta##->ramdac.dclk.max)) {

			register kgi_u_t dclk = ##meta##->ramdac.lclk.max *
				dpm->lclk.div / dpm->lclk.mul;

			dpm->dclk = (dclk < ##meta##->ramdac.dclk.max)
				? dclk : ##meta##->ramdac.dclk.max;
		}
		return KGI_EOK;

	case KGI_TC_RAISE:

		lclk = dpm->dclk * dpm->lclk.mul / dpm->lclk.div;

		if ((lclk > ##meta##->ramdac.lclk.max) || 
			(dpm->dclk > ##meta##->ramdac.dclk.max)) {

			KRN_DEBUG(1, "%i Hz DCLK is too high", dpm->dclk);
			return -E(RAMDAC, INVAL);
		}
		if ((lclk < ##meta##->ramdac.lclk.min) ||
			(dpm->dclk < ##meta##->ramdac.dclk.min)) {

			register kgi_u_t dclk = ##meta##->ramdac.dclk.min *
				dpm->lclk.div / dpm->lclk.mul;

			dpm->dclk = (dclk > ##meta##->ramdac.dclk.min)
				? dclk : ##meta##->ramdac.dclk.min;
		}
		return KGI_EOK;

     case KGI_TC_CHECK:

		if (! ((dpm->lclk.mul == rec->best_lclk.mul) &&
			(dpm->lclk.div == rec->best_lclk.div))) {

			return -E(RAMDAC, NOSUP);
		}

		lclk = dpm->dclk * dpm->lclk.mul / dpm->lclk.div;
		if ((dpm->dclk < ##meta##->ramdac.dclk.min) ||
			(##meta##->ramdac.dclk.max < dpm->dclk) ||
			(lclk < ##meta##->ramdac.lclk.min) ||
			(##meta##->ramdac.lclk.max < lclk)) {

			KRN_DEBUG(1, "%i Hz DCLK (%i Hz LCLK) is out of bounds",
				dpm->dclk, lclk);
			return -E(RAMDAC, INVAL);
		}
#warning check if required sync protocol is supported.
                if (##meta##_mode->kgim.crt->sync & (KGIM_ST_SYNC_ON_RED |
			KGIM_ST_SYNC_ON_BLUE)) {

			KRN_DEBUG(1, "can't do red or blue composite sync");
			return -E(RAMDAC, NOSUP);
                }
		break;

	default:
		KRN_INTERNAL_ERROR;
		return -E(RAMDAC, UNKNOWN);
	}

	/*	Now everything is checked and should be sane.
	**	Proceed to setup device dependent mode.
	*/
	##meta##_mode->rec = rec;

#warning set mode dependent state.
	/***	This includes any registers not covered by the mode_rec,
	****	setting the right sync protocol etc.
	***/
	return KGI_EOK;
}

void ##meta##_ramdac_mode_enter(##meta##_ramdac_t *##meta##,
	##meta##_ramdac_io_t *##meta##_io,
	##meta##_ramdac_mode_t *##meta##_mode,
	kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u_t x, y;

#warning set DAC mode.
}

/* -----------------------------------------------------------------------------
**	PERMEDIA2 integrated DAC meta language implementation
** -----------------------------------------------------------------------------
**	Copyright (C)	1998-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**
**	$Log: PERMEDIA2-meta.c,v $
**	Revision 1.1.1.1  2000/04/18 08:51:03  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger
#define	KGIM_RAMDAC_DRIVER	"$Revision: 1.1.1.1 $"

#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif

#include "ramdac/3Dlabs/PERMEDIA2.h"
#include "ramdac/3Dlabs/PERMEDIA2-meta.h"

/*
**	I/O helper functions
*/
static inline void PGC_EDAC_OUT8(pgc_ramdac_io_t *pgc_io, 
	kgi_u8_t val, kgi_u8_t reg)
{
	PGC_DAC_OUT8(pgc_io, reg, PGC_DAC_EXT_ADDR);
	PGC_DAC_OUT8(pgc_io, val, PGC_DAC_EXT_DATA);
}

static inline kgi_u8_t PGC_EDAC_IN8(pgc_ramdac_io_t *pgc_io, kgi_u8_t reg)
{
	PGC_DAC_OUT8(pgc_io, reg, PGC_DAC_EXT_ADDR);
	return PGC_DAC_IN8(pgc_io, PGC_DAC_EXT_DATA);
}


#define	PGC_MAGIC	0x0101
#define	PGC_SHIFT	8

struct pgc_ramdac_mode_record_s
{
	kgi_u8_t		ext40;
	kgi_u8_t		ext18;

	kgi_u8_t		ilut;
	kgi_u8_t		alut;

	kgi_ratio_t		best_lclk;

	kgi_attribute_mask_t	dam;
	const kgi_u8_t		*bpda;

	kgi_u8_t		frames;
};

static const pgc_ramdac_mode_record_t pgc_ramdac_mode[] =
{
	{ 0x00, 0x00, 1, 0, {1,1},	KGI_AM_TEXT, KGI_AS_448,	1 },

	{ 0x00, 0x10, 1, 0, {1,4},	KGI_AM_I,    KGI_AS_8,		1 },
	{ 0x10, 0xB1, 0, 1, {1,4},	KGI_AM_RGB,  KGI_AS_332,	1 },
	{ 0x10, 0xB3, 0, 1, {1,4},	KGI_AM_RGBA, KGI_AS_2321,	1 },
	{ 0x10, 0xB3, 0, 1, {1,4},	KGI_AM_RGBX, KGI_AS_2321,	1 },

	{ 0x10, 0xB4, 0, 1, {1,4},	KGI_AM_RGBA, KGI_AS_5551,	1 },
	{ 0x10, 0xB4, 0, 1, {1,4},	KGI_AM_RGBX, KGI_AS_5551,	1 },
	{ 0x10, 0xB5, 0, 1, {1,4},	KGI_AM_RGBA, KGI_AS_4444,	1 },
	{ 0x10, 0xB5, 0, 1, {1,4},	KGI_AM_RGBX, KGI_AS_4444,	1 },
	{ 0x10, 0xB6, 0, 1, {1,4},	KGI_AM_RGB,  KGI_AS_565,	1 },

	{ 0x10, 0xB8, 0, 1, {1,2},	KGI_AM_RGBA, KGI_AS_8888,	1 },
	{ 0x10, 0xB8, 0, 1, {1,2},	KGI_AM_RGBX, KGI_AS_8888,	1 },
	{ 0x10, 0xB9, 0, 1, {3,8},	KGI_AM_RGB,  KGI_AS_888,	1 }
};
#define	PGC_RAMDAC_NR_MODES (sizeof(pgc_ramdac_mode)/sizeof(pgc_ramdac_mode[0]))

static const pgc_ramdac_mode_record_t pgc_ramdac_mode_RGBP5551 =
{
	0x10, 0xB4, 0, 1, {1,2}, KGI_AM_RGBP, KGI_AS_AAA2, 2
};

static inline const pgc_ramdac_mode_record_t *pgc_ramdac_mode_record(
	kgi_attribute_mask_t fam, const kgi_u8_t *bpfa)
{
	kgi_u_t i;

	if (((KGI_AM_PRIVATE | KGI_AM_RGB) == fam) &&
		(kgim_strcmp(KGI_AS_5551, bpfa) == 0)) {

		return &pgc_ramdac_mode_RGBP5551;
	}

	/* !!!	handle BGR modes */
	for (i = 0; i < PGC_RAMDAC_NR_MODES; i++) {

		if (pgc_ramdac_mode[i].dam != fam) {

			continue;
		}
		if (kgim_strcmp(pgc_ramdac_mode[i].bpda, bpfa) != 0) {

			continue;
		}
		break;
	}
	return (i == PGC_RAMDAC_NR_MODES) ? NULL : pgc_ramdac_mode + i;
}




#define	LCLK(m,d)	((dpm->lclk.mul == m) && (dpm->lclk.div == d))
#define	RCLK(m,d)	((dpm->rclk.mul == m) && (dpm->rclk.div == d))

/* ----	text16 context functions ----------------------------------------------
**
**	
*/
#define	PGC_RAMDAC_IO(ctx)	\
	KGIM_SUBSYSTEM_IO((kgim_display_t *) ctx->meta_object, ramdac)

#define	PGC_RAMDAC_MODE(ctx)	\
	KGIM_SUBSYSTEM_MODE((kgim_display_mode_t *) ctx->meta_mode, ramdac)
#if 0
static void pgc_text16_set_ilut(kgic_mode_text16context_t *ctx,
	kgic_ilut_entries_t *ilut)
{
	pgc_ramdac_io_t   *pgc_io   = PGC_RAMDAC_IO(ctx);
	pgc_ramdac_mode_t *pgc_mode = PGC_RAMDAC_MODE(ctx);
	kgi_u_t	cnt, src, dst;

	KRN_ASSERT(ilut->img == 0);
	KRN_ASSERT(ilut->lut == 0);
	KRN_ASSERT(ilut->cnt > 0);

	cnt = ilut->cnt;
	dst = ilut->cnt * 3;
	src = 0;

	while (cnt--) {

		if (ilut->am & KGI_AM_COLOR1) {
			pgc_mode->clut[dst+0] = ilut->data[src++] >> PGC_SHIFT;
		}
		if (ilut->am & KGI_AM_COLOR2) {
			pgc_mode->clut[dst+1] = ilut->data[src++] >> PGC_SHIFT;
		}
		if (ilut->am & KGI_AM_COLOR3) {
			pgc_mode->clut[dst+2] = ilut->data[src++] >> PGC_SHIFT;
		}
		dst += 3;
	}

	src = 3 * ilut->idx;
	cnt = 3 * ilut->cnt;
	PGC_DAC_OUT8(pgc_io, ilut->idx, PGC_DAC_PW_INDEX);
	PGC_DAC_OUTS8(pgc_io, PGC_DAC_P_DATA, pgc_mode->clut + src, cnt);
}
#endif
#if 0
void pgc_text16_hp_set_shape(kgic_mode_text16context_t *ctx,
	kgi_pointer64x64 *ptr)
{
	pgc_ramdac_io_t   *pgc_io   = PGC_RAMDAC_IO(ctx);
	pgc_ramdac_mode_t *pgc_mode = PGC_RAMDAC_MODE(ctx);
	kgi_u_t i;

	pgc_mode->ptr.hot.x = ptr->hotx;
	pgc_mode->ptr.hot.y = ptr->hoty;

	/*	update private state
	*/
	pgc_mode->ext06 = PGC_EDAC06_Cursor64x64;
	switch (ptr->mode) {

	case KGI_PM_WINDOWS:
		pgc_mode->ext06 |= PGC_EDAC06_CursorXGA;
		break;

	case KGI_PM_X11:
		pgc_mode->ext06 |= PGC_EDAC06_CursorX;
		break;

	case KGI_PM_THREE_COLOR:
		pgc_mode->ext06 |= PGC_EDAC06_CursorThreeColor;
		break;

	default:
		KRN_INTERNAL_ERROR;
	}
	PGC_EDAC_OUT8(pgc_io, pgc_mode->ext06, 0x06);
        PGC_DAC_OUT8(pgc_io, 0x00, 0x00); /* Reset A0 - A7 */

	/*	set pattern
	*/
        for (i = 0; i < 512; i++) {

        	PGC_DAC_OUT8(pgc_io, ptr->xor_mask[i], PGC_DAC_CURSOR_RAM_DATA);
        }
        for (i = 0; i < 512; i++) {

        	PGC_DAC_OUT8(pgc_io, ptr->and_mask[i], PGC_DAC_CURSOR_RAM_DATA);
        }

	/*	set colors
	*/
	PGC_DAC_OUT8(pgc_io, 1, PGC_DAC_CW_INDEX);
	for (i = 0; i < 3; i++) {

		PGC_DAC_OUT8(pgc_io, ptr->col[i].r >> SHIFT, PGC_DAC_C_DATA);
		PGC_DAC_OUT8(pgc_io, ptr->col[i].g >> SHIFT, PGC_DAC_C_DATA);
		PGC_DAC_OUT8(pgc_io, ptr->col[i].b >> SHIFT, PGC_DAC_C_DATA);
	}
}
#endif

static void pgc_text16_hp_show(kgic_mode_text16context_t *ctx, 
	kgi_u_t x, kgi_u_t y)
{
	pgc_ramdac_io_t   *pgc_io   = PGC_RAMDAC_IO(ctx);
	pgc_ramdac_mode_t *pgc_mode = PGC_RAMDAC_MODE(ctx);

	if ((pgc_mode->ptr.pos.x != x) || (pgc_mode->ptr.pos.y != y)) {

		pgc_mode->ptr.pos.x = x;
		pgc_mode->ptr.pos.y = y;

		x += pgc_mode->ptr.shift.x - pgc_mode->ptr.hot.x;
		y += pgc_mode->ptr.shift.y - pgc_mode->ptr.hot.y;

		PGC_DAC_OUT8(pgc_io, x,      PGC_DAC_CURSOR_XL);
		PGC_DAC_OUT8(pgc_io, x >> 8, PGC_DAC_CURSOR_XH);
		PGC_DAC_OUT8(pgc_io, y,      PGC_DAC_CURSOR_YL);
		PGC_DAC_OUT8(pgc_io, y >> 8, PGC_DAC_CURSOR_YH);
	}
}

static void pgc_text16_hp_hide(kgic_mode_text16context_t *ctx)
{
	pgc_ramdac_io_t   *pgc_io   = PGC_RAMDAC_IO(ctx);
	pgc_ramdac_mode_t *pgc_mode = PGC_RAMDAC_MODE(ctx);

	pgc_mode->ptr.pos.x = pgc_mode->ptr.pos.y = 0x7FF;

	PGC_DAC_OUT8(pgc_io, 0xFF, PGC_DAC_CURSOR_XL);
	PGC_DAC_OUT8(pgc_io, 0x07, PGC_DAC_CURSOR_XH);
	PGC_DAC_OUT8(pgc_io, 0xFF, PGC_DAC_CURSOR_YL);
	PGC_DAC_OUT8(pgc_io, 0x07, PGC_DAC_CURSOR_YH);
}

#undef	PGC_RAMDAC_MODE
#undef	PGC_RAMDAC_IO
/*
** ----	end of text16 context functions ---------------------------------------
*/

kgi_error_t pgc_ramdac_init(pgc_ramdac_t *pgc, pgc_ramdac_io_t *pgc_io,
	const kgim_options_t *options)
{
	pgc->EXT.CursorControl = PGC_EDAC_IN8(pgc_io, PGC_EDAC_CursorControl);
	pgc->EXT.ColorMode     = PGC_EDAC_IN8(pgc_io, PGC_EDAC_ColorMode);
	pgc->EXT.ModeControl   = PGC_EDAC_IN8(pgc_io, PGC_EDAC_ModeControl);
	pgc->EXT.PalettePage   = PGC_EDAC_IN8(pgc_io, PGC_EDAC_PalettePage);
	pgc->EXT.MiscControl   = PGC_EDAC_IN8(pgc_io, PGC_EDAC_MiscControl);
	pgc->EXT.ColorKeyControl=PGC_EDAC_IN8(pgc_io, PGC_EDAC_ColorKeyControl);
	pgc->EXT.OverlayKey    = PGC_EDAC_IN8(pgc_io, PGC_EDAC_OverlayKey);
	pgc->EXT.RedKey        = PGC_EDAC_IN8(pgc_io, PGC_EDAC_RedKey);
	pgc->EXT.GreenKey      = PGC_EDAC_IN8(pgc_io, PGC_EDAC_GreenKey);
	pgc->EXT.BlueKey       = PGC_EDAC_IN8(pgc_io, PGC_EDAC_BlueKey);

	PGC_DAC_OUT8(pgc_io, 0, PGC_DAC_PR_INDEX);
	PGC_DAC_INS8(pgc_io, PGC_DAC_P_DATA, pgc->clut, sizeof(pgc->clut));

/*	kgi_u8_t cursor_palette[3*4];
**	kgi_u8_t cursor_data[];
*/
	pgc->CursorXLow	 = PGC_DAC_IN8(pgc_io, PGC_DAC_CURSOR_XL);
	pgc->CursorXHigh = PGC_DAC_IN8(pgc_io, PGC_DAC_CURSOR_XH);
	pgc->CursorYLow	 = PGC_DAC_IN8(pgc_io, PGC_DAC_CURSOR_YL);
	pgc->CursorYHigh = PGC_DAC_IN8(pgc_io, PGC_DAC_CURSOR_YH);

	return KGI_EOK;
}

void pgc_ramdac_done(pgc_ramdac_t *pgc, pgc_ramdac_io_t *pgc_io, 
	const kgim_options_t *options)
{
	PGC_EDAC_OUT8(pgc_io, pgc->EXT.CursorControl,  PGC_EDAC_CursorControl);
	PGC_EDAC_OUT8(pgc_io, pgc->EXT.ColorMode,      PGC_EDAC_ColorMode);
	PGC_EDAC_OUT8(pgc_io, pgc->EXT.ModeControl,    PGC_EDAC_ModeControl);
	PGC_EDAC_OUT8(pgc_io, pgc->EXT.PalettePage,    PGC_EDAC_PalettePage);
	PGC_EDAC_OUT8(pgc_io, pgc->EXT.MiscControl,    PGC_EDAC_MiscControl);
	PGC_EDAC_OUT8(pgc_io, pgc->EXT.OverlayKey,     PGC_EDAC_OverlayKey);
	PGC_EDAC_OUT8(pgc_io, pgc->EXT.RedKey,         PGC_EDAC_RedKey);
	PGC_EDAC_OUT8(pgc_io, pgc->EXT.GreenKey,       PGC_EDAC_GreenKey);
	PGC_EDAC_OUT8(pgc_io, pgc->EXT.BlueKey,        PGC_EDAC_BlueKey);
	PGC_EDAC_OUT8(pgc_io, pgc->EXT.ColorKeyControl,
		PGC_EDAC_ColorKeyControl);

	PGC_DAC_OUT8(pgc_io, 0, PGC_DAC_PW_INDEX);
	PGC_DAC_OUTS8(pgc_io, PGC_DAC_P_DATA, pgc->clut, sizeof(pgc->clut));

/*
	cursor_palette[3*4];
	cursor_data[];
*/
	PGC_DAC_OUT8(pgc_io, pgc->CursorXLow,	PGC_DAC_CURSOR_XL);
	PGC_DAC_OUT8(pgc_io, pgc->CursorXHigh,	PGC_DAC_CURSOR_XH);
	PGC_DAC_OUT8(pgc_io, pgc->CursorYLow,	PGC_DAC_CURSOR_YL);
	PGC_DAC_OUT8(pgc_io, pgc->CursorYHigh,	PGC_DAC_CURSOR_YH);
}

kgi_error_t pgc_ramdac_mode_check(
	pgc_ramdac_t *pgc, pgc_ramdac_io_t *pgc_io, pgc_ramdac_mode_t *pgc_mode,
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u_t lclk;
	kgi_dot_port_mode_t *dpm;
	const pgc_ramdac_mode_record_t *rec;

	KRN_ASSERT((1 == images) && (NULL != img));
	KRN_ASSERT(NULL != img->out);

	dpm = img->out;
	rec = pgc_ramdac_mode_record(img[0].fam, img[0].bpfa);
	if (rec == NULL) {

		KRN_DEBUG(1, "could not handle dot attributes (dam %.8x)",
			dpm->dam);
		return -KGI_ERRNO(RAMDAC, UNKNOWN);
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

		if ((lclk < pgc->ramdac.lclk.min) ||
			(dpm->dclk < pgc->ramdac.dclk.min)) {

			KRN_DEBUG(1, "%i Hz DCLK is too low", dpm->dclk);
			return -KGI_ERRNO(RAMDAC, INVAL);
		}
		if ((lclk > pgc->ramdac.lclk.max) || 
			(dpm->dclk > pgc->ramdac.dclk.max)) {

			register kgi_u_t dclk = pgc->ramdac.lclk.max *
				dpm->lclk.div / dpm->lclk.mul;

			dpm->dclk = (dclk < pgc->ramdac.dclk.max)
				? dclk : pgc->ramdac.dclk.max;
		}
		return KGI_EOK;

	case KGI_TC_RAISE:

		lclk = dpm->dclk * dpm->lclk.mul / dpm->lclk.div;

		if ((lclk > pgc->ramdac.lclk.max) || 
			(dpm->dclk > pgc->ramdac.dclk.max)) {

			KRN_DEBUG(1, "%i Hz DCLK is too high", dpm->dclk);
			return -KGI_ERRNO(RAMDAC, INVAL);
		}
		if ((lclk < pgc->ramdac.lclk.min) ||
			(dpm->dclk < pgc->ramdac.dclk.min)) {

			register kgi_u_t dclk = pgc->ramdac.dclk.min *
				dpm->lclk.div / dpm->lclk.mul;

			dpm->dclk = (dclk > pgc->ramdac.dclk.min)
				? dclk : pgc->ramdac.dclk.min;
		}
		return KGI_EOK;

     case KGI_TC_CHECK:

		if (! ((dpm->lclk.mul == rec->best_lclk.mul) &&
			(dpm->lclk.div == rec->best_lclk.div))) {

			return -KGI_ERRNO(RAMDAC, NOSUP);
		}

		lclk = dpm->dclk * dpm->lclk.mul / dpm->lclk.div;
		if ((dpm->dclk < pgc->ramdac.dclk.min) ||
			(pgc->ramdac.dclk.max < dpm->dclk) ||
			(lclk < pgc->ramdac.lclk.min) ||
			(pgc->ramdac.lclk.max < lclk)) {

			KRN_DEBUG(1, "%i Hz DCLK (%i Hz LCLK) is out of bounds",
				dpm->dclk, lclk);
			return -KGI_ERRNO(RAMDAC, INVAL);
		}

                if (pgc_mode->kgim.crt->sync & (KGIM_ST_SYNC_ON_RED |
			KGIM_ST_SYNC_ON_BLUE)) {

			KRN_DEBUG(1, "can't do red or blue composite sync");
			return -KGI_ERRNO(RAMDAC, NOSUP);
                }

		/*	Now everything is checked and should be sane.
		**	Proceed to setup device dependent mode.
		*/
		pgc_mode->rec = rec;

		pgc_mode->ext1E = PGC_EDAC1E_8BitEnable;
		if (pgc_mode->kgim.crt->sync & KGIM_ST_SYNC_ON_GREEN) {

			pgc_mode->ext1E |= PGC_EDAC1E_SyncOnGreen;
		}
		if (pgc_mode->kgim.crt->sync & KGIM_ST_SYNC_PEDESTRAL) {

			pgc_mode->ext1E |= PGC_EDAC1E_BlankPedestral;
		}

		pgc_mode->ext06 = PGC_EDAC06_Cursor64x64 | PGC_EDAC06_CursorOff;
		pgc_mode->ptr.pos.x = pgc_mode->ptr.pos.y = 0x7FF;
		pgc_mode->ptr.hot.x = pgc_mode->ptr.hot.y = 0;
		pgc_mode->ptr.shift.x = 64 +
			pgc_mode->kgim.crt->x.total -
			pgc_mode->kgim.crt->x.blankend;
		pgc_mode->ptr.shift.y = 64 +
			pgc_mode->kgim.crt->y.total -
			pgc_mode->kgim.crt->y.blankend;

		return KGI_EOK;

	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(RAMDAC, UNKNOWN);
	}
}

void pgc_ramdac_mode_enter(pgc_ramdac_t *pgc, pgc_ramdac_io_t *pgc_io,
	pgc_ramdac_mode_t *pgc_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u_t x, y;

	PGC_EDAC_OUT8(pgc_io, pgc_mode->rec->ext40, 0x40);
	PGC_EDAC_OUT8(pgc_io, 0, PGC_EDAC_OverlayKey);
	PGC_EDAC_OUT8(pgc_io, 0, PGC_EDAC_RedKey);
	PGC_EDAC_OUT8(pgc_io, 0, PGC_EDAC_GreenKey);
	PGC_EDAC_OUT8(pgc_io, 0, PGC_EDAC_BlueKey);
	PGC_EDAC_OUT8(pgc_io, 0, PGC_EDAC_PalettePage);

	x = pgc_mode->ptr.pos.x + pgc_mode->ptr.shift.x - pgc_mode->ptr.hot.x;
	y = pgc_mode->ptr.pos.y + pgc_mode->ptr.shift.y - pgc_mode->ptr.hot.y;

/*
	clut
	cursor clut
	cursor shape
*/
	PGC_EDAC_OUT8(pgc_io, pgc_mode->ext06, 0x06);
	PGC_DAC_OUT8(pgc_io, x,      PGC_DAC_CURSOR_XL);
	PGC_DAC_OUT8(pgc_io, x >> 8, PGC_DAC_CURSOR_XH);
	PGC_DAC_OUT8(pgc_io, y,      PGC_DAC_CURSOR_YL);
	PGC_DAC_OUT8(pgc_io, y >> 8, PGC_DAC_CURSOR_YH);

	PGC_EDAC_OUT8(pgc_io, pgc_mode->rec->ext18, 0x18);
	PGC_EDAC_OUT8(pgc_io, PGC_EDAC19_FrontBuffer, 0x19);
	PGC_EDAC_OUT8(pgc_io, pgc_mode->ext1E, 0x1E);
}

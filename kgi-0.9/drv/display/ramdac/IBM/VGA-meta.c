/* ----------------------------------------------------------------------------
**	VGA DAC meta language implementation
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Jon Taylor
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: VGA-meta.c,v $
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		Jon_Taylor
#define	KGIM_RAMDAC_DRIVER	"$Revision: 1.2 $"

#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif

#include "ramdac/IBM/VGA.h"
#include "ramdac/IBM/VGA-meta.h"

struct vga_ramdac_mode_record_s
{
	kgi_u8_t		ilut;
	kgi_u8_t		alut;

	kgi_ratio_t		best_lclk;

	kgi_attribute_mask_t	dam;
	const kgi_u8_t		*bpda;

	kgi_u8_t		frames;
};

static const vga_ramdac_mode_record_t vga_ramdac_mode[] =
{
	{ 1, 0, {1, 1},	KGI_AM_TEXT, KGI_AS_448,	1 },

/*	{ 1, 0, {1, 1},	KGI_AM_I,    KGI_AS_1,		1 },
**	{ 1, 0, {1, 1},	KGI_AM_I,    KGI_AS_4,		1 },
*/	{ 1, 0, {1, 1},	KGI_AM_I,    KGI_AS_8,		1 }	
};

#define	VGA_RAMDAC_NR_MODES (sizeof(vga_ramdac_mode) / sizeof(vga_ramdac_mode[0]))

static inline const vga_ramdac_mode_record_t *vga_ramdac_mode_record(
	kgi_attribute_mask_t fam, const kgi_u8_t *bpfa)
{
	kgi_u_t i;

	KRN_DEBUG(2, "vga_ramdac_mode_record()");
	
	for (i = 0; i < VGA_RAMDAC_NR_MODES; i++) {

		if (vga_ramdac_mode[i].dam != fam) {

			continue;
		}
		
		if (kgim_strcmp(vga_ramdac_mode[i].bpda, bpfa) != 0) {

			continue;
		}
		
		break;
	}
	
	if (i == VGA_RAMDAC_NR_MODES) {

		KRN_DEBUG(1, "Couldn't find a ramdac mode");

	} else {

		KRN_DEBUG(2, "Found ramdac mode %i", i);
	}
	
	return (i == VGA_RAMDAC_NR_MODES) ? NULL : vga_ramdac_mode + i;
}

#define	LCLK(m,d)	((dpm->lclk.mul == m) && (dpm->lclk.div == d))
#define	RCLK(m,d)	((dpm->rclk.mul == m) && (dpm->rclk.div == d))

/* ----	text16 context functions --------------------------------------------*/
#define	VGA_RAMDAC_IO(ctx)	\
	KGIM_SUBSYSTEM_IO((kgim_display_t *) ctx->meta_object, ramdac)

#define	VGA_RAMDAC_MODE(ctx)	\
	KGIM_SUBSYSTEM_MODE((kgim_display_mode_t *) ctx->meta_mode, ramdac)


static void vga_text16_set_ilut(void *ctx, void *ilut)
{
/*	vga_ramdac_io_t   *vga_io   = VGA_RAMDAC_IO(ctx);
**	vga_ramdac_mode_t *vga_mode = VGA_RAMDAC_MODE(ctx);
*/	kgi_u_t	cnt, src, dst;
	
	KRN_DEBUG(2, "vga_text16_set_ilut()");
	
#if 0

	KRN_ASSERT(ilut->img == 0);
	KRN_ASSERT(ilut->lut == 0);
	KRN_ASSERT(ilut->cnt > 0);

	cnt = ilut->cnt;
	cnt = 0xff;
	dst = cnt * 3;
	src = 0;

	while (cnt--) {

		if (ilut->am & KGI_AM_COLOR1) {

			vga_mode->clut[dst + 0] = 
				ilut->data[src++] >> vga_SHIFT;
		}
		
		if (ilut->am & KGI_AM_COLOR2) {

			vga_mode->clut[dst+1] = 
				ilut->data[src++] >> vga_SHIFT;
		}
		
		if (ilut->am & KGI_AM_COLOR3) {

			vga_mode->clut[dst+2] =
				ilut->data[src++] >> vga_SHIFT;
		}
		
		dst += 3;
	}

	src = 3 * ilut->idx;
	cnt = 3 * ilut->cnt;
	VGA_DAC_OUT8(vga_io, ilut->idx, VGA_DAC_PW_INDEX);
	VGA_DAC_OUTS8(vga_io, VGA_DAC_P_DATA, vga_mode->clut + src, cnt);
#endif
}

#if 0
void vga_text16_hp_set_shape(kgic_mode_text16context_t *ctx,
	kgi_pointer64x64_t *ptr)
{
/*	vga_ramdac_io_t   *vga_io   = VGA_RAMDAC_IO(ctx);
**	vga_ramdac_mode_t *vga_mode = VGA_RAMDAC_MODE(ctx);
*/	kgi_u_t i;
	
	KRN_DEBUG(2, "vga_text16_hp_set_shape()");

/*	vga_mode->ptr.hot.x = ptr->hotx;
**	vga_mode->ptr.hot.y = ptr->hoty;
*/	
#if 0

	/* Update private state */
        VGA_DAC_OUT8(vga_io, 0x00, 0x00); /* Reset A0 - A7 */

	/*	Set pattern 
	*/
        for (i = 0; i < 512; i++) {

        	vga_DAC_OUT8(vga_io, ptr->xor_mask[i], vga_DAC_CURSOR_RAM_DATA);
        }
	
        for (i = 0; i < 512; i++) {

        	vga_DAC_OUT8(vga_io, ptr->and_mask[i], vga_DAC_CURSOR_RAM_DATA);
        }

	/*	Set colors
	*/
	vga_DAC_OUT8(vga_io, 1, vga_DAC_CW_INDEX);
	
	for (i = 0; i < 3; i++) {

		vga_DAC_OUT8(vga_io, ptr->col[i].r >> SHIFT, vga_DAC_C_DATA);
		vga_DAC_OUT8(vga_io, ptr->col[i].g >> SHIFT, vga_DAC_C_DATA);
		vga_DAC_OUT8(vga_io, ptr->col[i].b >> SHIFT, vga_DAC_C_DATA);
	}
#endif
}
#endif

#if 0
static void vga_text16_hp_show(kgic_mode_text16context_t *ctx, 
	kgi_u_t x, kgi_u_t y)
{
	vga_ramdac_io_t   *vga_io   = VGA_RAMDAC_IO(ctx);
	vga_ramdac_mode_t *vga_mode = VGA_RAMDAC_MODE(ctx);

	KRN_DEBUG(2, "vga_text16_hp_show()");
	
	if ((vga_mode->ptr.pos.x != x) || (vga_mode->ptr.pos.y != y)) {

		vga_mode->ptr.pos.x = x;
		vga_mode->ptr.pos.y = y;

		x += vga_mode->ptr.shift.x - vga_mode->ptr.hot.x;
		y += vga_mode->ptr.shift.y - vga_mode->ptr.hot.y;
	}
}
#endif

static void vga_text16_hp_hide(kgic_mode_text16context_t *ctx)
{
	vga_ramdac_io_t   *vga_io   = VGA_RAMDAC_IO(ctx);
	vga_ramdac_mode_t *vga_mode = VGA_RAMDAC_MODE(ctx);

	KRN_DEBUG(2, "vga_text16_hp_hide()");
	
	vga_mode->ptr.pos.x = vga_mode->ptr.pos.y = 0x7FF;
}

#undef	VGA_RAMDAC_MODE
#undef	VGA_RAMDAC_IO
/* ---- end of text16 context functions ----------------------------------- */

kgi_error_t vga_ramdac_init(vga_ramdac_t *vga, vga_ramdac_io_t *vga_io,
	const kgim_options_t *options)
{
	KRN_DEBUG(2, "vga_ramdac_init()");

#if 1
	VGA_DAC_OUT8(vga_io, 0, VGA_DAC_PR_INDEX);
	VGA_DAC_INS8(vga_io, VGA_DAC_P_DATA, vga->clut, sizeof(vga->clut));

/*	kgi_u8_t cursor_palette[3*4];
**	kgi_u8_t cursor_data[];
*/
#endif
	
	return KGI_EOK;
}

void vga_ramdac_done(vga_ramdac_t *vga, vga_ramdac_io_t *vga_io,
	const kgim_options_t *options)
{
	KRN_DEBUG(1, "vga_ramdac_done()");
	
#if 1
	VGA_DAC_OUT8(vga_io, 0, VGA_DAC_PW_INDEX);
	VGA_DAC_OUTS8(vga_io, VGA_DAC_P_DATA, vga->clut, sizeof(vga->clut));

/*
	cursor_palette[3*4];
	cursor_data[];
*/
#endif
}

kgi_error_t vga_ramdac_mode_check(vga_ramdac_t *vga, vga_ramdac_io_t *vga_io,
	vga_ramdac_mode_t *vga_mode,kgi_timing_command_t cmd, 
	kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u_t lclk;
	kgi_dot_port_mode_t *dpm;
	const vga_ramdac_mode_record_t *rec;

	KRN_DEBUG(2, "vga_ramdac_mode_check()");
	
	KRN_ASSERT((1 == images) && (NULL != img));
	KRN_ASSERT(NULL != img->out);

	dpm = img->out;
	rec = vga_ramdac_mode_record(img[0].fam, img[0].bpfa);
	
	if (rec == NULL) {

		KRN_DEBUG(2, "Couldn't handle dot attributes (dam %.8x)",
			dpm->dam);
		return -E(RAMDAC, UNKNOWN);
	}

	switch (cmd) {

	case KGI_TC_PROPOSE:
		KRN_DEBUG(3, "KGI_TC_PROPOSE:");
		
		dpm->flags &= ~KGI_DPF_TP_2XCLOCK;
		dpm->flags |= rec->alut ? KGI_DPF_CH_ALUT : 0;
		dpm->flags |= rec->ilut ? KGI_DPF_CH_ILUT : 0;
		
		dpm->lclk.mul = dpm->lclk.mul ? dpm->lclk.mul : 1;
		dpm->lclk.div = dpm->lclk.div ? dpm->lclk.div : 1;
		dpm->rclk.mul = dpm->rclk.mul ? dpm->rclk.mul : 1;
		dpm->rclk.div = dpm->rclk.div ? dpm->rclk.div : 1;

		dpm->dclk = vga->ramdac.lclk.max * dpm->lclk.mul /
			dpm->lclk.div;
		
		dpm->dam  = rec->dam;
		dpm->bpda = rec->bpda;

		if (0 == img[0].frames) {

			KRN_DEBUG(2, "frames == 0, setting to %i", rec->frames);
			img[0].frames = rec->frames;
		}

		KRN_DEBUG(3, "lclk %i:%i, dclk %i:%i", dpm->lclk.mul,
			dpm->lclk.div, dpm->rclk.mul, dpm->rclk.div);

 		return KGI_EOK;

	case KGI_TC_LOWER:
		KRN_DEBUG(3, "KGI_TC_LOWER:");

		if ((dpm->lclk.mul > 0) && (dpm->lclk.div > 0)) {

			lclk = vga->ramdac.lclk.max * dpm->lclk.mul /
				dpm->lclk.div;
		} else {

			KRN_DEBUG(1, "Invalid LCLK mul or div (%i,%i)",
				dpm->lclk.mul, dpm->lclk.div);
			return -E(RAMDAC, UNKNOWN);
		}


		if ((lclk < vga->ramdac.lclk.min) ||
			(dpm->dclk < vga->ramdac.dclk.min)) {

			KRN_DEBUG(2, "%i Hz DCLK is too low", dpm->dclk);
			return -E(RAMDAC, INVAL);
		}
		
		if ((lclk > vga->ramdac.lclk.max) ||
			(dpm->dclk > vga->ramdac.dclk.max)) {

			register kgi_u_t dclk = vga->ramdac.lclk.max *
				dpm->lclk.div / dpm->lclk.mul;

			dpm->dclk = (dclk < vga->ramdac.dclk.max)
				? dclk : vga->ramdac.dclk.max;
			
			KRN_DEBUG(2, "dpm->dclk = %i", dpm->dclk);
		}
		
		return KGI_EOK;

	case KGI_TC_RAISE:
		KRN_DEBUG(3, "KGI_TC_RAISE:");

		if ((dpm->lclk.mul > 0) && (dpm->lclk.div > 0)) {

			lclk = vga->ramdac.lclk.max * dpm->lclk.mul /
				dpm->lclk.div;
		} else {

			KRN_DEBUG(1, "Invalid LCLK mul or div (%i,%i)", 
				dpm->lclk.mul, dpm->lclk.div);
			return -E(RAMDAC, UNKNOWN);
		}


		if ((lclk > vga->ramdac.lclk.max) ||
			(dpm->dclk > vga->ramdac.dclk.max)) {

			KRN_DEBUG(2, "%i Hz DCLK is too high", dpm->dclk);
			return -E(RAMDAC, INVAL);
		}
		
		if ((lclk < vga->ramdac.lclk.min) ||
			(dpm->dclk < vga->ramdac.dclk.min)) {

			register kgi_u_t dclk = vga->ramdac.dclk.min * 
				dpm->lclk.div / dpm->lclk.mul;

			dpm->dclk = (dclk > vga->ramdac.dclk.min) 
				? dclk : vga->ramdac.dclk.min;
			
			KRN_DEBUG(2, "dpm->dclk = %i", dpm->dclk);
		}
		
		return KGI_EOK;

     case KGI_TC_CHECK:
		KRN_DEBUG(3, "KGI_TC_CHECK:");
		
		KRN_DEBUG(2, "dpm->lclk.[mul,div] = [%i,%i]", 
			dpm->lclk.mul, dpm->lclk.div);
		KRN_DEBUG(2, "rec->best_lclk.[mul,div] = [%i,%i]",
			rec->best_lclk.mul, rec->best_lclk.div);

		if (! ((dpm->lclk.mul == rec->best_lclk.mul) &&
			(dpm->lclk.div == rec->best_lclk.div))) {

			KRN_DEBUG(2, "Unsupported lclk mul (%i) or div (%i)",
				dpm->lclk.mul, dpm->lclk.div);
			return -E(RAMDAC, NOSUP);
		}

		lclk = dpm->dclk * dpm->lclk.mul / dpm->lclk.div;
		
		if ((dpm->dclk < vga->ramdac.dclk.min) || 
			(vga->ramdac.dclk.max < dpm->dclk) ||
			(lclk < vga->ramdac.lclk.min) || 
			(vga->ramdac.lclk.max < lclk)) {

			KRN_DEBUG(2, "%i Hz DCLK (%i Hz LCLK) is out of bounds",
				dpm->dclk, lclk);
			return -E(RAMDAC, INVAL);
		}

                if (vga_mode->kgim.crt->sync & 
			(KGIM_ST_SYNC_ON_RED | KGIM_ST_SYNC_ON_BLUE)) {

			KRN_DEBUG(2, "Can't do red or blue composite sync");
			return -E(RAMDAC, NOSUP);
                }

		/*	Now everything is checked and should be sane.
		**	Proceed to setup device dependent mode.
		*/
		vga_mode->rec = rec;

		vga_mode->ptr.pos.x = vga_mode->ptr.pos.y = 0x3FF;
		vga_mode->ptr.hot.x = vga_mode->ptr.hot.y = 0;
		vga_mode->ptr.shift.x = 64 + vga_mode->kgim.crt->x.total - 
			vga_mode->kgim.crt->x.blankend;
		vga_mode->ptr.shift.y = 64 + vga_mode->kgim.crt->y.total - 
			vga_mode->kgim.crt->y.blankend;

		return KGI_EOK;

	default:
		KRN_INTERNAL_ERROR;
		return -E(RAMDAC, UNKNOWN);
	}
}

void vga_ramdac_mode_enter(vga_ramdac_t *vga, vga_ramdac_io_t *vga_io, 
	vga_ramdac_mode_t *vga_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u_t x, y;

	KRN_DEBUG(1, "vga_ramdac_mode_enter()");
	
	VGA_DAC_OUT8(vga_io, 0xff, VGA_DAC_PIXEL_MASK);
	
	x = vga_mode->ptr.pos.x + vga_mode->ptr.shift.x - vga_mode->ptr.hot.x;
	y = vga_mode->ptr.pos.y + vga_mode->ptr.shift.y - vga_mode->ptr.hot.y;	
}

/* ----------------------------------------------------------------------------
**	Matrox Gx00 ramdac meta implementation
** ----------------------------------------------------------------------------
**	Copyright (C) 1999-2000		Johan Karlberg
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: Gx00-meta.c,v $
*/

#include <kgi/maintainers.h>
#define	MAINTAINER		Johan_Karlberg
#define	KGIM_RAMDAC_DRIVER	"$Revision: 1.2 $"

#define DEBUG_LEVEL 255

#include <kgi/module.h>
#include "chipset/Matrox/Gx00.h"
#include "ramdac/Matrox/Gx00-meta.h"

/*
**	IO helpers
*/

static inline void MGAG_EDAC_OUT8(mgag_ramdac_io_t *mgag_io,
	kgi_u8_t val, kgi_u8_t reg)
{
	MGAG_DAC_OUT8(mgag_io, reg, PALWTADD);
	MGAG_DAC_OUT8(mgag_io, val, X_DATAREG);
}

static inline kgi_u8_t MGAG_EDAC_IN8(mgag_ramdac_io_t *mgag_io, kgi_u8_t reg)
{
	MGAG_DAC_OUT8(mgag_io, reg, PALWTADD);
	return MGAG_DAC_IN8(mgag_io, X_DATAREG);
}

#define	MGAG_MAGIC	0x0101
#define	MGAG_SHIFT	8

static const mgag_ramdac_mode_record_t mgag_ramdac_mode[] =
{
	{ MUL_8_PAL,	1,	0,	0,	0,	0,	KGI_AM_I,	KGI_AS_8	},
	{ MUL_15_PAL,	1,	32,	3,	3,	3,	KGI_AM_RGBA,	KGI_AS_5551	},
	{ MUL_16_PAL,	1,	64,	3,	2,	3,	KGI_AM_RGB,	KGI_AS_565	},
	{ MUL_24_PAL,	1,	256,	0,	0,	0,	KGI_AM_RGB,	KGI_AS_888	},
	{ MUL_32_PAL,	1,	256,	0,	0,	0,	KGI_AM_RGBX,	KGI_AS_8888	},
	{ MUL_32_DIR,	0,	0,	0,	0,	0,	KGI_AM_RGBA,	KGI_AS_8888	}
};

#define	MGAG_RAMDAC_NR_MODES (sizeof(mgag_ramdac_mode) / sizeof(mgag_ramdac_mode[0]))

static inline const mgag_ramdac_mode_record_t *mgag_ramdac_mode_record(
	kgi_attribute_mask_t fam, const kgi_u8_t *bpfa)
{
	kgi_u_t i;

	KRN_DEBUG(2, "ramdac_mode_record() initiated");

	for (i = 0; i < MGAG_RAMDAC_NR_MODES; i++) {

		if (mgag_ramdac_mode[i].dam != fam) {

			continue;
		}

		if (kgim_strcmp(mgag_ramdac_mode[i].bpda, bpfa) != 0) {

			continue;
		}

		break;
	}

	if (i == MGAG_RAMDAC_NR_MODES) {

		KRN_DEBUG(1, "Couldn't find a ramdac mode");
	}

	KRN_DEBUG(2, "ramdac_mode_record() completed");

	return (i == MGAG_RAMDAC_NR_MODES) ? NULL : mgag_ramdac_mode + i;
}

kgi_error_t mgag_ramdac_init(mgag_ramdac_t *mgag, mgag_ramdac_io_t *mgag_io,
	const kgim_options_t *options)
{
	kgi_u_t i = 0;

	KRN_DEBUG(2, "ramdac_init() initiated");

	mgag->saved.xzoom    = MGAG_EDAC_IN8(mgag_io, XZOOMCTRL);
	mgag->saved.xgenctrl = MGAG_EDAC_IN8(mgag_io, XGENCTRL);
	mgag->saved.xmulctrl = MGAG_EDAC_IN8(mgag_io, XMULCTRL);
	mgag->saved.pixrdmsk = MGAG_EDAC_IN8(mgag_io, XMULCTRL);

	MGAG_DAC_OUT8(mgag_io, 0, PALWTADD);
	MGAG_DAC_INS8(mgag_io, PALDATA, mgag->saved.clut, sizeof(mgag->saved.clut));

	KRN_DEBUG(2, "ramdac_init() completed");

	return KGI_EOK;
}

void mgag_ramdac_done(mgag_ramdac_t *mgag, mgag_ramdac_io_t *mgag_io, 
	const kgim_options_t *options)
{
	KRN_DEBUG(2, "ramdac_done() initiated");

	MGAG_EDAC_OUT8(mgag_io, mgag->saved.xzoom,    XZOOMCTRL);
	MGAG_EDAC_OUT8(mgag_io, mgag->saved.xgenctrl, XGENCTRL);
	MGAG_EDAC_OUT8(mgag_io, mgag->saved.xmulctrl, XMULCTRL);
	MGAG_DAC_OUT8 (mgag_io, mgag->saved.pixrdmsk, PIXRDMSK);

	MGAG_DAC_OUT8(mgag_io, 0, PALWTADD);
	MGAG_DAC_OUTS8(mgag_io, PALWTADD, mgag->saved.clut, sizeof(mgag->saved.clut));

	KRN_DEBUG(2, "ramdac_done() completed");
}

kgi_error_t mgag_ramdac_mode_check( mgag_ramdac_t *mgag, 
	mgag_ramdac_io_t *mgag_io, mgag_ramdac_mode_t *mgag_mode, 
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_dot_port_mode_t *dpm;
	const mgag_ramdac_mode_record_t *rec;

	KRN_DEBUG(2, "ramdac_mode_check() initiated");

	KRN_ASSERT((1 == images) && (NULL != img));
	KRN_ASSERT(NULL != img->out);

	dpm = img->out;
	rec = mgag_ramdac_mode_record(img[0].fam, img[0].bpfa);

	if (rec == NULL) {

		KRN_DEBUG(1, "could not handle dot attributes (dam %.8x)",
			dpm->dam);
		return -E(RAMDAC, UNKNOWN);
	}

	switch (cmd) {

	case KGI_TC_PROPOSE:

		KRN_DEBUG(3, "KGI_TC_PROPOSE:");

		/*	setting dot port mode. dclk and dots, untouched
		*/
		dpm->flags &= ~KGI_DPF_TP_2XCLOCK;
		dpm->flags |= rec->ilut ? KGI_DPF_CH_ILUT : 0;
 
		dpm->lclk.mul = dpm->lclk.div =
		dpm->rclk.mul = dpm->rclk.div = 1;

		dpm->dam  = rec->dam;
		dpm->bpda = rec->bpda;

		KRN_DEBUG(2, "lclk %i:%i, dclk %i:%i", 
			dpm->lclk.mul, dpm->lclk.div, 
			dpm->rclk.mul, dpm->rclk.div);
 
 		return KGI_EOK;

	case KGI_TC_LOWER:

		KRN_DEBUG(3, "KGI_TC_LOWER:");

		if (dpm->dclk < mgag->ramdac.dclk.min) {

			KRN_DEBUG(1, "%i Hz DCLK is too low",
				dpm->dclk);
			return -E(RAMDAC, INVAL);
		}

		if (dpm->dclk > mgag->ramdac.dclk.max) {

			dpm->dclk = mgag->ramdac.dclk.max;
		}

		return KGI_EOK;
	case KGI_TC_RAISE:

		KRN_DEBUG(3, "KGI_TC_RAISE:");

		if (dpm->dclk > mgag->ramdac.dclk.max) {

			KRN_DEBUG(1, "%i Hz DCLK is too high", dpm->dclk);
			return -E(RAMDAC, INVAL);
		}

		if (dpm->dclk < mgag->ramdac.dclk.min) {

			dpm->dclk = mgag->ramdac.dclk.min;
		}

		return KGI_EOK;

	case KGI_TC_CHECK:

		KRN_DEBUG(3, "KGI_TC_CHECK:");

#warning sure you did not confuse "==" with "=" ?
		if (! (dpm->lclk.mul = dpm->lclk.div = dpm->rclk.mul = dpm->rclk.div = 1)) {

			KRN_DEBUG(1, "lclk/rclk div/mul not supported");
			return -E(RAMDAC, NOSUP);
		}

		if ((dpm->dclk < mgag->ramdac.dclk.min) || 
			(mgag->ramdac.dclk.max < dpm->dclk)) {

			KRN_DEBUG(1, "%i Hz DCLK is out of bounds", 
				dpm->dclk);
			return -E(RAMDAC, INVAL);
		}

		return KGI_EOK;

	default:
		KRN_INTERNAL_ERROR;
		return -E(RAMDAC, UNKNOWN);
	}

	KRN_DEBUG(2, "ramdac_mode_check() completed");
}

void mgag_ramdac_mode_prepare(mgag_ramdac_t *mgag, mgag_ramdac_io_t *mgag_io, 
	mgag_ramdac_mode_t *mgag_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	KRN_DEBUG(2, "ramdac_mode_prepare() initiated");

	if (mgag_mode->kgim.crt->sync & KGIM_ST_SYNC_PEDESTRAL) {

		mgag_mode->xgenctrl |= XGENCTRL_PEDON;
	}

	if (mgag_mode->kgim.crt->sync & KGIM_ST_SYNC_ON_GREEN) {

		mgag_mode->xgenctrl &= ~XGENCTRL_IOGSYNCDIS;
	}

	mgag_mode->miscctrl = (XMISCCTRL_VGA8DAC | XMISCCTRL_MCFSEL_DIS);

	KRN_DEBUG(2, "ramdac_mode_prepare() completed");
}

void mgag_ramdac_mode_enter(mgag_ramdac_t *mgag, mgag_ramdac_io_t *mgag_io,
	mgag_ramdac_mode_t *mgag_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u_t	i = 0;

	KRN_DEBUG(2, "ramdac_mode_enter() initiated");

	MGAG_EDAC_OUT8(mgag_io, mgag_mode->xzoom,	XZOOMCTRL);
	MGAG_EDAC_OUT8(mgag_io, mgag_mode->xgenctrl,	XGENCTRL);
	MGAG_EDAC_OUT8(mgag_io, mgag_mode->rec->depth,	XMULCTRL);

	MGAG_DAC_OUT8(mgag_io, 0xFF, PIXRDMSK);

	if (mgag_mode->rec->ilut) {

		MGAG_DAC_OUT8(mgag_io, 0, PALWTADD);

		for (i=0;i<mgag_mode->rec->pal_size;i++) {

			MGAG_DAC_OUT8(mgag_io, i << mgag_mode->rec->r_shift, 
				PALDATA);
			MGAG_DAC_OUT8(mgag_io, i << mgag_mode->rec->g_shift, 
				PALDATA);
			MGAG_DAC_OUT8(mgag_io, i << mgag_mode->rec->b_shift, 
				PALDATA);
		}
	}

	KRN_DEBUG(2, "ramdac_mode_enter() completed");
}

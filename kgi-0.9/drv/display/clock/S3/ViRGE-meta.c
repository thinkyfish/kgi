/* -----------------------------------------------------------------------------
**	S3 ViRGE PLL meta language definition
** -----------------------------------------------------------------------------
**	Copyright (C)	1997-2000	Jon Taylor
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**
**	$Log: ViRGE-meta.c,v $
**	Revision 1.1.1.1  2000/04/18 08:51:14  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Jon_Taylor
#define	KGIM_CLOCK_DRIVER	"$Revision: 0.1 $"
#define DEBUG_LEVEL 255

#include <kgi/module.h>
#include "ramdac/S3/ViRGE.h"
#include "clock/S3/ViRGE-meta.h"
#include "clock/S3/ViRGE-bind.h"

#if 0
static inline void VIRGE_EDAC_OUT8(virge_clock_io_t *virge_io, kgi_u8_t val, kgi_u8_t reg)
{
	VIRGE_DAC_OUT8(virge_io, reg, VIRGE_DAC_EXT_ADDR);
	VIRGE_DAC_OUT8(virge_io, val, VIRGE_DAC_EXT_DATA);
}

static inline kgi_u8_t VIRGE_EDAC_IN8(virge_clock_io_t *virge_io, kgi_u8_t reg)
{
	VIRGE_DAC_OUT8(virge_io, reg, VIRGE_DAC_EXT_ADDR);
	return VIRGE_DAC_IN8(virge_io, VIRGE_DAC_EXT_DATA);
}

#endif

kgi_error_t virge_clock_init(virge_clock_t *virge,
			     virge_clock_io_t *virge_io, 
			     const kgim_options_t *options)
{
	KRN_DEBUG(2, "virge_clock_init()");
	
	return KGI_EOK;
}

void virge_clock_done(virge_clock_t *virge, virge_clock_io_t *virge_io, const kgim_options_t *options)
{
	KRN_DEBUG(2, "virge_clock_done()");
	
}

kgi_error_t virge_clock_mode_check(virge_clock_t *virge,
				   virge_clock_io_t *virge_io,
				   virge_clock_mode_t *virge_mode,
				   kgi_timing_command_t cmd,
				   kgi_image_mode_t *img,
				   kgi_u_t images)
{
	kgi_error_t retval;
	
	KRN_DEBUG(2, "virge_clock_mode_check()");
	
	retval = pll_clock_mode_check(&virge->pll,
				      NULL, /* pll_io doesn't exist */
				      &virge_mode->pll_mode,
				      cmd, img, images);
	
	KRN_DEBUG(2, "pll_clock_mode_check() returned %i", retval);
	
	return retval;
}

kgi_resource_t *virge_clock_mode_resource(virge_clock_t *virge,
					  virge_clock_mode_t *virge_mode,
					  kgi_image_mode_t *img,
					  kgi_u_t images,
					  kgi_u_t index)
{
	KRN_DEBUG(2, "virge_clock_mode_resource()");

#if 0
	if (img->fam & KGI_AM_TEXTURE_INDEX)
	{
		return vga_clock_mode_resource(&virge->vga,
					       &virge_mode->vga,
					       img, images, index);
	}
#endif
	
	return NULL; /* We don't handle any clock resources yet */
}

void virge_clock_mode_prepare(virge_clock_t *virge,
			      virge_clock_io_t *virge_io,
			      virge_clock_mode_t *virge_mode,
			      kgi_image_mode_t *img,
			      kgi_u_t images)
{
	KRN_DEBUG(2, "virge_clock_mode_prepare()");
}

void virge_clock_mode_enter(virge_clock_t *virge, 
			    virge_clock_io_t *virge_io, 
			    virge_clock_mode_t *virge_mode, 
			    kgi_image_mode_t *img, 
			    kgi_u_t images)
{
	kgi_u_t cnt;

	KRN_DEBUG(2, "virge_clock_mode_enter()");
	
	/*KRN_ASSERT(virge_mode->pll_mode.mul < 256); // FIXME: Use #defines here
	KRN_ASSERT(virge_mode->pll_mode.div < 256);
	KRN_ASSERT(virge_mode->pll_mode.p   < 8);
	
	KRN_DEBUG(2, "mul = %d, div = %d, p = %d", mult, div, p);
*/
#if 0
	/* We use clock C, as this one is 'not valid' at boot up */
	VIRGE_EDAC_OUT8(virge_io, (virge_mode->pll.mul) & 0xFF, VIRGE_EDAC_PClkC_M);
	VIRGE_EDAC_OUT8(virge_io, (virge_mode->pll.div) & 0xFF, VIRGE_EDAC_PClkC_N); 
	VIRGE_EDAC_OUT8(virge_io, ((virge_mode->pll.p) & 0x07) | VIRGE_EDAC_PClkP_Enable, VIRGE_EDAC_PClkC_P);

	VIRGE_CLK_OUT8(virge_io, 3, 0);

	VIRGE_DAC_OUT8(virge_io, 0x29, VIRGE_DAC_EXT_ADDR);
	cnt = 10000;
	while (cnt-- && (VIRGE_DAC_IN8(virge_io, VIRGE_DAC_EXT_DATA) & virge_EDAC29_PLLlocked));

	KRN_ASSERT(cnt);
#endif
}

void virge_clock_mode_leave(virge_clock_t *virge,
			    virge_clock_io_t *virge_io,
			    virge_clock_mode_t *virge_mode,
			    kgi_image_mode_t *img,
			    kgi_u_t images)
{
	KRN_DEBUG(2, "virge_clock_mode_leave()");
}

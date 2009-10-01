/*-
 * Copyright (c) 2002 Nicholas Souchu
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
 * copies of the Software, and permit to persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,EXPRESSED OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHOR(S) OR COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
 * KGI vidsw interface display driver
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

#ifndef KGI_DBG_LEVEL
#define KGI_DBG_LEVEL 3
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/fbio.h>

#include <dev/fb/fbreg.h>
#include <dev/fb/vgareg.h>

#include <dev/kgi/system.h>

#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>

#include <dev/kgy/kgy.h>

/* 
 * This is the mode_info used for the console,
 * it is statically allocated to avoid malloc at init.
 */
static video_info_t console_info;

#define	print_img_mode(lvl,img)	KRN_DEBUG(lvl,				\
	"%ix%i (%ix%i), %i frames, fam=%.8x, bpam (IFTB) = %i%i%i%i",	\
	img[0].size.x, img[0].size.y, img[0].virt.x, img[0].virt.y,	\
	img[0].frames, img[0].fam, img[0].bpfa[0], img[0].bpfa[1],	\
	img[0].bpfa[2], img[0].bpfa[3])

void dpysw_set_mode(kgi_display_t *dpy, kgi_image_mode_t *img, kgi_u_t images,
	  void *dev_mode)
{
	dpysw_display_t *sc = (dpysw_display_t *)dpy;
	video_info_t *next = &((vidsw_mode_t *)dev_mode)->mode_info;
	video_info_t *previous = &((vidsw_mode_t *)dev_mode)->oldmode_info;
	kgi_mmio_region_t *fb;
	kgi_text16_t *text16;
	int error = 0;

	/* Set current adapter mode info as previous mode info. */
	*previous = sc->adp->va_info;

	/* Set video mode from previously checked mode. */
	error = (*vidsw[sc->adp->va_index]->set_mode)(sc->adp, next->vi_mode);
	
	if (error != 0) {
		KRN_DEBUG(1, "Could not set mode (mode=%d, err=%d).",
			  next->vi_mode, error);
		goto error;
	}

	/* 
	 * Set the framebuffer characteristics accordingly to the new mode set.
	 */
	fb = &sc->fb;
	text16 = &sc->text16;

	if (next->vi_flags & V_INFO_LINEAR) {
		fb->win.size = (kgi_size_t)next->vi_buffer_size;
		fb->win.virt = (kgi_virt_addr_t)sc->adp->va_buffer;
		fb->win.bus = (kgi_bus_addr_t)0;	/* XXX */
		fb->win.phys = (kgi_phys_addr_t)next->vi_buffer;
		fb->size = (kgi_size_t)next->vi_buffer_size;

		text16->meta_io = (void *)sc->adp->va_buffer;
	} else {
		fb->win.size = (kgi_size_t)next->vi_window_size;
		fb->win.virt = (kgi_virt_addr_t)sc->adp->va_window;
		fb->win.bus = (kgi_bus_addr_t)0;	/* XXX */
		fb->win.phys = (kgi_phys_addr_t)next->vi_window;
		fb->size = (kgi_size_t)next->vi_buffer_size/next->vi_planes;

		text16->meta_io = (void *)sc->adp->va_window;
	}
	fb->offset = 0;

 error:
	return;
}

void dpysw_unset_mode(kgi_display_t *dpy, kgi_image_mode_t *img, kgi_u_t images,
	  void *dev_mode)
{
	dpysw_display_t *sc = (dpysw_display_t *)dpy;
	video_info_t *next = &((vidsw_mode_t *)dev_mode)->oldmode_info;
	kgi_mmio_region_t *fb;
	kgi_text16_t *text16;
	int error = 0;

	/* Restore previous mode. */
	error = (*vidsw[sc->adp->va_index]->set_mode)(sc->adp,
						      next->vi_mode);
	
	if (error != 0) {
		KRN_DEBUG(1, "Could not restore mode (mode=%d,err=%d).",
			   next->vi_mode, error);
		goto error;
	}

	/* 
	 * Set the framebuffer characteristics accordingly to the new mode set.
	 */
	fb = &sc->fb;
	text16 = &sc->text16;

	if (next->vi_flags & V_INFO_LINEAR) {
		fb->win.size = (kgi_size_t)next->vi_buffer_size;
		fb->win.virt = (kgi_virt_addr_t)sc->adp->va_buffer;
		fb->win.bus = (kgi_bus_addr_t)0;	/* XXX */
		fb->win.phys = (kgi_phys_addr_t)next->vi_buffer;
		fb->size = (kgi_size_t)next->vi_buffer_size;

		text16->meta_io = (void *)sc->adp->va_buffer;
	} else {
		fb->win.size = (kgi_size_t)next->vi_window_size;
		fb->win.virt = (kgi_virt_addr_t)sc->adp->va_window;
		fb->win.bus = (kgi_bus_addr_t)0;	/* XXX */
		fb->win.phys = (kgi_phys_addr_t)next->vi_window;
		fb->size = (kgi_size_t)next->vi_buffer_size/next->vi_planes;

		text16->meta_io = (void *)sc->adp->va_window;
	}
	fb->offset = 0;

 error:
	return;
}

static int 
dpysw_try_mode(dpysw_display_t *sc, video_info_t *mode_info)
{
	int error, flags;

	error = (*vidsw[sc->adp->va_index]->query_mode)(sc->adp, mode_info);

	/* If VESA fails, try standard VGA. */
	if (error) {
		/* Save flags. */
		flags = mode_info->vi_flags;

		/* Remove VESA flags. */
		mode_info->vi_flags &= ~(V_INFO_VESA | V_INFO_LINEAR);

		error = (*vidsw[sc->adp->va_index]->query_mode)(sc->adp, mode_info);
		
		if (error) {
			/* Restore flags. */
			mode_info->vi_flags = flags;

			return (KGI_ERRNO(CHIPSET, INVAL));
		}
	}

	return (KGI_EOK);
}

#define	print_img_mode(lvl,img)	KRN_DEBUG(lvl,				\
	"%ix%i (%ix%i), %i frames, fam=%.8x, bpam (IFTB) = %i%i%i%i",	\
	img[0].size.x, img[0].size.y, img[0].virt.x, img[0].virt.y,	\
	img[0].frames, img[0].fam, img[0].bpfa[0], img[0].bpfa[1],	\
	img[0].bpfa[2], img[0].bpfa[3])

int
dpysw_check_mode(kgi_display_t *dpy, kgi_timing_command_t cmd,
	kgi_image_mode_t *img, kgi_u_t images, void *dev_mode, 
	const kgi_resource_t **r, kgi_u_t rsize)
{
#define	DEFAULT(x)	if (! (x)) { x = sc->mode.x; }
#define	MATCH(x)	((x) == sc->mode.x)
	dpysw_display_t *sc = (dpysw_display_t *)dpy;
	video_info_t *mode_info;

	kgi_text16_t *text16;
	kgi_u_t bpp, bpf, bpc;

	mode_info = (!dev_mode ? &console_info :
		     &((vidsw_mode_t *)dev_mode)->mode_info);

	bzero(mode_info, sizeof(*mode_info));

	if (images != 1) {
		KRN_DEBUG(2, "%i image layers are not supported.", images);
		return (KGI_EINVAL);
	}

	/*	For unsupported image flags, bail out. */
	if (img[0].flags & (KGI_IF_VIRTUAL | KGI_IF_VISIBLE | 
		KGI_IF_TILE_X | KGI_IF_STEREO)) {
		KRN_DEBUG(1, "Image flags %.8x not supported", img[0].flags);
		return (KGI_ERRNO(CHIPSET, INVAL));
	}

	switch (cmd) {
	case KGI_TC_PROPOSE:
		KRN_DEBUG(3, "dpysw: proposing original mode for:");
		print_img_mode(3, img);
		DEFAULT(img[0].virt.x);
		DEFAULT(img[0].virt.y);
		DEFAULT(img[0].size.x);
		DEFAULT(img[0].size.y);
		DEFAULT(img[0].frames);
		DEFAULT(img[0].tluts);
		DEFAULT(img[0].iluts);
		DEFAULT(img[0].aluts);
		if (! img[0].fam) {
			img[0].fam  = sc->mode.img[0].fam;
			img[0].cam  = sc->mode.img[0].cam;
			memcpy(img[0].bpfa, sc->mode.img[0].bpfa,
			       sizeof(img[0].bpfa));
			memcpy(img[0].bpca, sc->mode.img[0].bpca,
			       sizeof(img[0].bpca));
		}
		/*	Fall through. */
	case KGI_TC_CHECK:
		KRN_DEBUG(3, "dpysw: checking mode:");
		print_img_mode(3, img);

		if (img[0].fam == KGI_AM_TEXT ||
		    img[0].flags & KGI_IF_TEXT16) {
			KRN_DEBUG(2, "Checking a textmode...");

			/* Common attributes are not possible. */
			if (img[0].cam) {				
				KRN_DEBUG(1, "Common attributes %.8x not supported",
					  img[0].cam);
				return (KGI_ERRNO(CHIPSET, INVAL));
			}
			
			bpp = 0;
		
			/* If any of the sizes is null, set default image sizes. */
			if ((0 == img[0].size.x) || (0 == img[0].size.y)) {
				img[0].size.x = 80;
				img[0].size.y = 25;
			}
			mode_info->vi_cwidth = 8;
			mode_info->vi_cheight = 16;

			text16 = &sc->text16;

			text16->size.x		= img[0].size.x;
			text16->size.y		= img[0].size.y;
			text16->virt.x		= img[0].size.x;
			text16->virt.y		= img[0].size.y;
			text16->cell.x		= mode_info->vi_cwidth;
			text16->cell.y		= mode_info->vi_cheight;
			text16->font.x		= mode_info->vi_cwidth;
			text16->font.y		= mode_info->vi_cheight;

			KRN_NOTICE("text16 set up: size %ix%i, virt %ix%i",
				   text16->size.x, text16->size.y,
				   text16->virt.x, text16->virt.y);
	
			/* Per image resource. */
			img[0].resource[0] = (kgi_resource_t *) &sc->text16;

		} else {
			KRN_DEBUG(2, "Checking a graphic mode...");
			
			/* Not a textmode. */
			bpf = kgi_attr_bits(img[0].bpfa);
			bpc = kgi_attr_bits(img[0].bpca);
			bpp = (bpc + bpf * img[0].frames);
			
			KRN_DEBUG(2, "bpf = %i, bpc = %i, bpp = %i",
				  bpf, bpc, bpp);
			
			/* 
			 * Try VESA linear framebuffered mode first, then VGA standard
			 * modes.
			 */
			mode_info->vi_flags |= V_INFO_GRAPHICS | V_INFO_VESA |
				V_INFO_LINEAR;
			
			/*	Check if common attributes are supported. */
			if (img[0].cam) {
				KRN_DEBUG(1, "Common attributes %.8x not supported", 
					  img[0].cam);
				return (KGI_ERRNO(CHIPSET, INVAL));
			}

			img[0].resource[0] = (kgi_resource_t *) &sc->ilut;
			img[0].resource[1] = (kgi_resource_t *) &sc->ptr;
		}
		
		if (img[0].fam & (KGI_AM_COLORS | KGI_AM_COLOR_INDEX))
			mode_info->vi_flags |= V_INFO_COLOR;

		KRN_DEBUG(2, "dpysw: querying for cwidth = %i, cheight = %i, flags = 0x%x, ",
			  mode_info->vi_cwidth, mode_info->vi_cheight,
			  mode_info->vi_flags);

		mode_info->vi_depth = bpp;
		
		/* First, try to set the visible size. */
		mode_info->vi_width = img[0].size.x;
		mode_info->vi_height = img[0].size.y;

		KRN_DEBUG(2, "dpysw: Visible: width = %i, height = %i, depth = %i ",
			  mode_info->vi_width, mode_info->vi_height,
			  mode_info->vi_depth);

		if (dpysw_try_mode(sc, mode_info)) {		
			/* Try to set the virtual size otherwise. */
			mode_info->vi_width = img[0].virt.x;
			mode_info->vi_height = img[0].virt.y;

			KRN_DEBUG(2, "dpysw: Virtual: width = %i, height = %i, depth = %i ",
				  mode_info->vi_width, mode_info->vi_height,
				  mode_info->vi_depth);
			
			if (dpysw_try_mode(sc, mode_info)) {
				return (KGI_ERRNO(CHIPSET, INVAL));
			}
		}

		KRN_DEBUG(1, "VESA/VGA mode found %dx%dx%d",
			  mode_info->vi_width, mode_info->vi_height,
			  mode_info->vi_depth);

		/* Common resource to all kind of modes. */
		if ((0 < rsize) && r) {
			r[0] = (kgi_resource_t *) &sc->fb;
		}
		break;		
	default:
		KRN_INTERNAL_ERROR;
		return (KGI_ERRNO(CHIPSET, UNKNOWN));
	}

	return (KGI_EOK);
}

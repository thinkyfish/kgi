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
 * KGI vidsw interface display driver.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

#ifndef KGI_DBG_LEVEL
#define KGI_DBG_LEVEL 2
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/fbio.h>

#include <dev/fb/fbreg.h>
#include <dev/fb/vgareg.h>

#define	KGI_SYS_NEED_IO
#include <dev/kgi/system.h>

#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>

#include <dev/kgy/kgy.h>

static int registered = 0;

dpysw_display_t dpysw_sc;

/* XXX FIXME */
extern int vesa_load_ioctl(void);
extern int vesa_unload_ioctl(void);

/* Stub the ioctl install hook for vesa.c */
int
vesa_load_ioctl(void)
{

	return (0);
}

int
vesa_unload_ioctl(void)
{

	return (0);
}

static kgi_error_t
dpysw_display_command(kgi_display_t *dpy, kgi_u_t cmd, void *data)
{

	return (EINVAL);
}

static void
dpysw_inc_refcount(kgi_display_t *dpy)
{

	KGI_ASSERT(dpy);
	KGI_DEBUG(2, "dpysw display refcount increment");

	return;
}

static void
dpysw_dec_refcount(kgi_display_t *dpy)
{

	KGI_ASSERT(dpy);
	KGI_DEBUG(2, "dpysw display refcount decrement");

	return;
}

#if 0
static void dpysw_ptr_set_shape(kgi_marker_t *m, kgi_u_t x, kgi_u_t y)
{
	dpysw_display_t *sc = (dpysw_display_t *)m->meta;
	int error;

	error = (*vidsw[sc->adp->va_index]->set_hw_cursor_shape)
		(sc->adp, (int)x, (int)y);

	return;
}
#endif

static void
dpysw_ptr_show(kgi_marker_t *m, kgi_u_t x, kgi_u_t y)
{
	dpysw_display_t *sc;
	int error;

	sc = (dpysw_display_t *)m->meta;
	error = (*vidsw[sc->adp->va_index]->set_hw_cursor)
		(sc->adp, (int)x, (int)y);

	return;
}

static void
dpysw_ptr_hide(kgi_marker_t *m)
{
	dpysw_display_t *sc;
	int error;

	sc = (dpysw_display_t *)m->meta;
	error = (*vidsw[sc->adp->va_index]->set_hw_cursor)
		(sc->adp, -1, -1);

	return;
}

static void
dpysw_ptr_read(kgi_marker_t *m, kgi_u_t *x, kgi_u_t *y)
{
	dpysw_display_t *sc;
	int error;

	sc = (dpysw_display_t *)m->meta;
	error = (*vidsw[sc->adp->va_index]->read_hw_cursor)
		(sc->adp, (int *)x, (int *)y);

	return;
}

static void
dpysw_set_offset(kgi_mmio_region_t *r, kgi_size_t offset)
{
	dpysw_display_t *sc;
	int error;

	sc = (dpysw_display_t *)r->meta;
	error = (*vidsw[sc->adp->va_index]->set_win_org)
		(sc->adp, (off_t) offset);

	if (!error) {
		r->win.phys = sc->adp->va_info.vi_window;
		r->offset = offset;
	}

	return;
}

static void
dpysw_set_ilut(kgi_clut_t *r, kgi_u_t table, kgi_u_t index, kgi_u_t count,
		kgi_attribute_mask_t am, const kgi_u16_t *data)
{
	int error, i;
	dpysw_display_t *sc;
	kgi_u8_t clut[256 * 3];

	KGI_ASSERT(index == 0);
	KGI_ASSERT(table == 0);
	KGI_ASSERT(count == 256);

	/* All colors must change at once. */
	if ((am & KGI_AM_COLOR1) == 0 ||
	    (am & KGI_AM_COLOR2) == 0 ||
	    (am & KGI_AM_COLOR3) == 0 ||
	    count != 256 || index != 0)
		return;

	/* XXX stupid convertion. */
	for (i = 0; i < 256 * 3; i += 3) {
		clut[i] = data[i] >> 8;
		clut[i + 1] = data[i + 1] >> 8;
		clut[i + 2] = data[i + 2] >> 8;
	}

	sc = (dpysw_display_t *)r->meta;
	error = (*vidsw[sc->adp->va_index]->load_palette)
		(sc->adp, clut);

	return;
}

static void
dpysw_get_ilut(kgi_clut_t *r, kgi_u_t table, kgi_u_t index, kgi_u_t
		count, kgi_attribute_mask_t am, const kgi_u16_t *data)
{
	int error;
	dpysw_display_t *sc;

	KGI_ASSERT(table == 0);
	KGI_ASSERT(count == 256);
	KGI_ASSERT(index == 0);

	/* All colors must change at once. */
	if ((am & KGI_AM_COLOR1) == 0 ||
	    (am & KGI_AM_COLOR2) == 0 ||
	    (am & KGI_AM_COLOR3) == 0 ||
	    count != 256 || index != 0)
		return;


	sc = (dpysw_display_t *)r->meta;
	error = (*vidsw[sc->adp->va_index]->save_palette)
		(sc->adp, (u_char *)data);

	return;
}

static int
dpysw_configure(int flags)
{
	int i;
	video_adapter_t *adp, *vgadp = NULL;
	video_info_t *vi;
	kgi_display_t *dpy;
	kgi_mode_t *mode;
	kgi_mmio_region_t *fb;
	kgi_clut_t *ilut;
	kgi_marker_t *ptr;
	kgi_text16_t *text16;
	dpysw_display_t *sc = &dpysw_sc;

	/*
	 * XXX find a VGA or VESA adapter. Should work with any other
	 * kind of adapter.
	 */
	for (i = 0; (adp = vid_get_adapter(i)) != NULL; ++i) {
		/* Remember if we find VGA, then continue for VESA. */
		if (adp->va_type == KD_VGA)
			vgadp = adp;
		if (adp->va_flags & V_ADP_VESA) {
			KGI_DEBUG(1, "dpysw: Found VESA adapter!");
			break;
		}
	}

	if (adp == NULL && vgadp)
		adp = vgadp;

	if (adp == NULL) {
		KGI_ERROR("dpysw: no VGA adapter found!");
		return (ENXIO);
	}

	memset(sc, 0, sizeof(dpysw_display_t));

	sc->adp = adp;
	dpy = &sc->dpy;

	dpy->revision = KGI_DISPLAY_REVISION;
	snprintf(dpy->vendor, KGI_MAX_VENDOR_STRING, "KGI FreeBSD");
	snprintf(dpy->model, KGI_MAX_VENDOR_STRING, "dpysw");

	dpy->flags = 0;
	dpy->mode_size = sizeof(vidsw_mode_t);

	mode = &sc->mode;

	dpy->mode = mode;
	dpy->id = -1;
	dpy->graphic = 0;
	dpy->IncRefcount = dpysw_inc_refcount;
	dpy->DecRefcount = dpysw_dec_refcount;

	dpy->CheckMode = dpysw_check_mode;
	dpy->SetMode = dpysw_set_mode;
	dpy->UnsetMode = dpysw_unset_mode;
	dpy->Command = dpysw_display_command;

	dpy->focus = NULL;

	/* Initialize mode struct from current mode info. */
	mode->revision 		= KGI_MODE_REVISION;
	mode->dev_mode 		= NULL;
	mode->images 		= 1;
	mode->img[0].out 	= NULL;
	mode->img[0].flags 	= KGI_IF_TEXT16;
	mode->img[0].virt.x 	= adp->va_info.vi_width;
	mode->img[0].virt.y 	= adp->va_info.vi_height;
	mode->img[0].size.x 	= adp->va_info.vi_width;
	mode->img[0].size.y 	= adp->va_info.vi_height;
	mode->img[0].frames 	= 1;
	mode->img[0].tluts 	= 0;
	mode->img[0].aluts 	= 0;
	mode->img[0].ilutm 	= 0;
	mode->img[0].alutm 	= 0;
	mode->img[0].fam 	= 0;
	mode->img[0].cam 	= 0;

	/* Initialize mmio struct common resource. */
	fb = &sc->fb;

	fb->meta 		= dpy;
	fb->type 		= KGI_RT_MMIO_FRAME_BUFFER;
	fb->prot 		= KGI_PF_APP_RWS | KGI_PF_LIB_RWS
				  | KGI_PF_DRV_RWS;
	fb->name 		= "Frame buffer";
	fb->access 		= 8 + 16 + 32 + 64;
	fb->align 		= 8 + 16;
	fb->win.size 		= (kgi_size_t)adp->va_window_size;
	fb->win.virt 		= (kgi_virt_addr_t)adp->va_window; /* XXX */
	fb->win.bus 		= (kgi_bus_addr_t)0;		   /* XXX */
	fb->win.phys 		= (kgi_phys_addr_t)adp->va_mem_base;
	fb->size 		= (kgi_size_t)adp->va_mem_size;
	fb->offset 		= 0;
	fb->SetOffset 		= dpysw_set_offset;

	/* Initialize ilut struct. */
	ilut = &sc->ilut;

	ilut->meta 		= dpy;
	ilut->type 		= KGI_RT_ILUT_CONTROL;
	ilut->prot 		= KGI_PF_DRV_RWS;
	ilut->name 		= "ILUT control";
	ilut->Set 		= dpysw_set_ilut;
	ilut->Get 		= dpysw_get_ilut;
	ilut->entries		= 256;
	ilut->tables		= 1;

	/* Initialize marker struct. */
	ptr = &sc->ptr;

	ptr->meta 		= dpy;
	ptr->type 		= KGI_RT_POINTER_CONTROL;
	ptr->prot 		= KGI_PF_DRV_RWS;
	ptr->name 		= "Pointer control";
	ptr->modes 		= KGI_MM_TEXT16;
#if 0
	ptr->SetShape 		= dpysw_ptr_set_shape;
#endif
	ptr->Show 		= dpysw_ptr_show;
	ptr->Hide 		= dpysw_ptr_hide;
	ptr->Read 		= dpysw_ptr_read;

	text16 = &sc->text16;

	/* text16 control. */
	text16->meta		= dpy;
	text16->meta_io		= NULL;
	text16->type		= KGI_RT_TEXT16_CONTROL;
	text16->prot		= KGI_PF_DRV_RWS;
	text16->name		= "Text16 control";
	text16->PutText16	= dpysw_put_text16;

	/* Initialize KGI mode from current mode. */
	vi = &sc->adp->va_info;
	if (vi->vi_flags & V_INFO_GRAPHICS) {
		switch (vi->vi_mem_model) {
		case V_INFO_MM_PACKED:
			mode->img[0].fam = KGI_AM_I;
			mode->img[0].cam = KGI_AM_COLOR_INDEX;
			mode->img[0].bpfa[0] = vi->vi_depth;
			mode->img[0].bpfa[1] = 0;
			break;
		case V_INFO_MM_DIRECT:
			mode->img[0].fam = KGI_AM_RGB;
			mode->img[0].cam = KGI_AM_COLORS;
			mode->img[0].bpfa[0] = vi->vi_pixel_fsizes[0];
			mode->img[0].bpfa[1] = vi->vi_pixel_fsizes[1];

			mode->img[0].bpfa[2] = vi->vi_pixel_fsizes[2];
			mode->img[0].bpfa[3] = 0;
			break;
		case V_INFO_MM_PLANAR:		/* XXX reject planar modes. */
		default:
			return (ENODEV);
			/* NOT REACHED. */
		}

		ptr->shapes 	= 1;
		ptr->size.x 	= 64;
		ptr->size.y 	= 64;

		/* Per image resource. */
		mode->resource[0] = (kgi_resource_t *) &sc->fb;
		mode->resource[1] = (kgi_resource_t *) &sc->ilut;
		mode->resource[2] = (kgi_resource_t *) &sc->ptr;

	} else {
		if (vi->vi_mem_model != V_INFO_MM_TEXT)
			return (ENODEV);

		if (vi->vi_flags & V_INFO_COLOR) {
			mode->img[0].fam = KGI_AM_TEXT;

			mode->img[0].bpfa[0] = 4;	/* BG color. */
			mode->img[0].bpfa[1] = 4;	/* FG color. */
			mode->img[0].bpfa[2] = 8;	/* Texture.  */
			mode->img[0].bpfa[3] = 0;

			/* XXX change mode. */
		} else {
			mode->img[0].fam = KGI_AM_TEXT | KGI_AM_BLINK;

			mode->img[0].bpfa[0] = 3;	/* Index.    */
			mode->img[0].bpfa[1] = 4;	/* FG color. */
			mode->img[0].bpfa[2] = 8;	/* Texture.  */
			mode->img[0].bpfa[3] = 1;	/* Blink.    */
			mode->img[0].bpfa[4] = 0;

			/* XXX change mode. */
		}
		text16->meta_io		= (void *)adp->va_window;
		text16->size.x		= adp->va_info.vi_width;
		text16->size.y		= adp->va_info.vi_height;
		text16->virt.x		= adp->va_info.vi_width;
		text16->virt.y		= adp->va_info.vi_height;
		text16->cell.x		= vi->vi_cwidth;
		text16->cell.y		= vi->vi_cheight;
		text16->font.x		= vi->vi_cwidth;
		text16->font.y		= vi->vi_cheight;

		/* Per image resource. */
		mode->resource[0] = (kgi_resource_t *) &sc->fb;
		mode->resource[1] = (kgi_resource_t *) &sc->text16;
	}

	if (kgi_register_display(dpy, 0)) {
		KGI_NOTICE("Could not register vidsw display.\n");
		return (ENXIO);
	}

	registered = 1;

	kgy_splash(adp);

	return (0);
}

static int
dpysw_load(void)
{
	int error = 0;
	int s;

	if (registered)
		return (0);

	s = spltty();
	error = dpysw_configure(0);
	splx(s);

	return (error);
}

static int
dpysw_unload(void)
{
	int s;

	if (registered == 0)
		return (0);

	s = spltty();
	kgi_unregister_display(&dpysw_sc.dpy);
	splx(s);

	return (0);
}

static int
dpysw_mod_event(module_t mod, int type, void *data)
{

	switch (type) {
	case MOD_LOAD:
		return (dpysw_load());
	case MOD_UNLOAD:
		return (dpysw_unload());
	default:
		break;
	}
	return (0);
}

static moduledata_t dpysw_mod = {
	"dpysw",
	dpysw_mod_event,
	NULL,
};

/* Add 1 to MIDDLE to take place after VESA module */
DECLARE_MODULE(dpysw, dpysw_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE + 1);
MODULE_VERSION(dpysw, 1);

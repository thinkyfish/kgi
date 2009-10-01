/*-
 * Copyright (c) 1999-2000 Steffen Seeger
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
 * NULL display code
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

#ifndef KGI_DBG_LEVEL
#define	KGI_DBG_LEVEL	2
#endif

#define	KGI_SYS_NEED_IO
#define	KGI_SYS_NEED_MALLOC
#include <dev/kgi/system.h>

#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>

/*
 * kernel interface
 */

typedef struct {
	kgi_dot_port_mode_t	dpm;

	kgi_accel_t		blubber_accel;
	kgi_marker_t		cur;
	kgi_marker_t		ptr;
	kgi_text16_t		text16;

} dpy_null_mode_t;

typedef struct {
	kgi_display_t		dpy;
	kgi_mode_t		mode;
	kgi_ucoord_t		size;

} dpy_null_display_t;

static dpy_null_display_t dpy_data[CONFIG_KGI_DISPLAYS];
static int dpy_index = 0;

/* text16 operations */
static void 
dpy_null_show(kgi_marker_t *marker, kgi_u_t x, kgi_u_t y)
{

	KRN_ASSERT(marker);
}

static void 
dpy_null_hide(kgi_marker_t *marker)
{

	KRN_ASSERT(marker);
}

#define	dpy_null_undo	dpy_null_hide


static void dpy_null_put_text16(kgi_text16_t *text16,
	kgi_u_t offset, const kgi_u16_t *text, kgi_u_t count)
{
}

static void 
dpy_null_inc_refcount(kgi_display_t *dpy)
{

	KRN_ASSERT(dpy);
	KRN_DEBUG(2, "dpy_null display refcount increment");
}

static void 
dpy_null_dec_refcount(kgi_display_t *dpy)
{

	KRN_ASSERT(dpy);
	KRN_DEBUG(2, "dpy_null display refcount decrement");
}

static int dpy_null_check_mode(kgi_display_t *dpy, kgi_timing_command_t cmd,
	kgi_image_mode_t *img, kgi_u_t images, void *dev_mode, 
	const kgi_resource_t **r, kgi_u_t rsize)
{
#define	DEFAULT(x)	if (! (x)) { x = dpy_null->mode.x; }
#define	MATCH(x)	((x) == dpy_null->mode.x)
	dpy_null_display_t *dpy_null = (dpy_null_display_t *) dpy;
	dpy_null_mode_t *devmode = (dpy_null_mode_t *) dev_mode;

	if (images != 1) {
		KRN_DEBUG(2, "%i image layers are not supported.", images);
		return (-EINVAL);
	}

	switch (cmd) {
	case KGI_TC_PROPOSE:
		DEFAULT(img[0].virt.x);
		DEFAULT(img[0].virt.y);
		DEFAULT(img[0].size.x);
		DEFAULT(img[0].size.y);
		DEFAULT(img[0].frames);
		DEFAULT(img[0].tluts);
		DEFAULT(img[0].iluts);
		DEFAULT(img[0].aluts);
		if (!img[0].fam) {
			img[0].fam  = dpy_null->mode.img[0].fam;
			img[0].cam  = dpy_null->mode.img[0].cam;
			memcpy(img[0].bpfa, dpy_null->mode.img[0].bpfa,
				sizeof(img[0].bpfa));
			memcpy(img[0].bpca, dpy_null->mode.img[0].bpca,
				sizeof(img[0].bpca));
		}
		/* fall through */
	case KGI_TC_CHECK:
		if (!(MATCH(img[0].virt.x) && MATCH(img[0].virt.y) &&
			MATCH(img[0].size.x) && MATCH(img[0].size.y) &&
			MATCH(img[0].frames) && MATCH(img[0].tluts) &&
			MATCH(img[0].iluts) && MATCH(img[0].aluts))) {
			KRN_DEBUG(2, "image mode does not match boot mode");
			return (-EINVAL);
		}
		if (!MATCH(img[0].fam) || !MATCH(img[0].cam) ||
			strncmp(img[0].bpfa, dpy_null->mode.img[0].bpfa,
				__KGI_MAX_NR_ATTRIBUTES) ||
			strncmp(img[0].bpca, dpy_null->mode.img[0].bpca,
				__KGI_MAX_NR_ATTRIBUTES)) {
			KRN_DEBUG(2, "attributes do not match boot mode");
			return (-EINVAL);
		}

		devmode->dpm.dots.x = 8*img[0].size.x;
		devmode->dpm.dots.y = 12*img[0].size.y;
		devmode->dpm.dam    = img[0].fam;
		devmode->dpm.bpda   = img[0].bpfa;
		img[0].out = &devmode->dpm;
		img[0].flags = KGI_IF_TEXT16;

		devmode->cur.meta	= dpy_null;
		devmode->cur.meta_io	= NULL;
		devmode->cur.type	= KGI_RT_CURSOR_CONTROL;
		devmode->cur.prot	= KGI_PF_DRV_RWS;
		devmode->cur.name	= "cursor control";
		devmode->cur.size.x	= 1;
		devmode->cur.size.y	= 1;
		devmode->cur.Show	= dpy_null_show;
		devmode->cur.Hide	= dpy_null_hide;
		devmode->cur.Undo	= dpy_null_undo;

		devmode->ptr.meta	= dpy_null;
		devmode->ptr.meta_io	= NULL;
		devmode->ptr.type	= KGI_RT_POINTER_CONTROL;
		devmode->ptr.prot	= KGI_PF_DRV_RWS;
		devmode->ptr.name	= "pointer control";
		devmode->ptr.size.x	= 1;
		devmode->ptr.size.y	= 1;
		devmode->ptr.Show	= dpy_null_show;
		devmode->ptr.Hide	= dpy_null_hide;
		devmode->ptr.Undo	= dpy_null_undo;

		devmode->text16.meta		= dpy_null;
		devmode->text16.meta_io		= NULL;
		devmode->text16.type		= KGI_RT_TEXT16_CONTROL;
		devmode->text16.prot		= KGI_PF_DRV_RWS;
		devmode->text16.name		= "text16 control";
		devmode->text16.size.x		= img[0].size.x;
		devmode->text16.size.y		= img[0].size.y;
		devmode->text16.virt.x		= img[0].virt.x;
		devmode->text16.virt.y		= img[0].virt.y;
		devmode->text16.cell.x		= 8;
		devmode->text16.cell.y		= 12;
		devmode->text16.font.x		= 8;
		devmode->text16.font.y		= 12;
		devmode->text16.PutText16	= dpy_null_put_text16;

		img[0].resource[0] = (kgi_resource_t *) &(devmode->text16);
		img[0].resource[1] = (kgi_resource_t *) &(devmode->cur);
		img[0].resource[2] = (kgi_resource_t *) &(devmode->ptr);

		return (KGI_EOK);
	default:
		KRN_INTERNAL_ERROR;
		return (-EINVAL);
	}
#undef	MATCH
#undef	DEFAULT
}

static void 
dpy_null_set_mode(kgi_display_t *dpy, kgi_image_mode_t *img,
	kgi_u_t images, void *dev_mode)
{

	KRN_ASSERT(dpy);
	KRN_ASSERT(img);
	KRN_ASSERT(images == 1);
	KRN_ASSERT(dev_mode);
}

static inline kgi_display_t *
dpy_null_alloc(kgi_u_t size_x, kgi_u_t size_y)
{
	dpy_null_display_t *dpy_null;

	KRN_ASSERT(size_x && size_y);

	if (dpy_index >= CONFIG_KGI_DISPLAYS)
		return (NULL);

	/* Allocate null displays statically for early initialization */
	dpy_null = &dpy_data[dpy_index];
	dpy_index ++;

	memset(dpy_null, 0, sizeof(*dpy_null));

	/* initialize display struct */
	dpy_null->dpy.revision = KGI_DISPLAY_REVISION;
	sprintf(dpy_null->dpy.vendor, "KGI null display");
	sprintf(dpy_null->dpy.model, "%p", dpy_null);
	dpy_null->dpy.flags = 0;
	dpy_null->dpy.mode_size = sizeof(dpy_null_mode_t);
	dpy_null->dpy.Command = NULL;

	/* dpy_null->dpy.priv.priv_u64 = 0; */
	dpy_null->dpy.mode = &dpy_null->mode;
	dpy_null->dpy.id = KGI_INVALID_DISPLAY;
	dpy_null->dpy.graphic = 0;
	dpy_null->dpy.IncRefcount = dpy_null_inc_refcount;
	dpy_null->dpy.DecRefcount = dpy_null_dec_refcount;

	dpy_null->dpy.CheckMode	 = dpy_null_check_mode;
	dpy_null->dpy.SetMode	 = dpy_null_set_mode;

	dpy_null->dpy.focus = NULL;

	/* initialize mode struct */
	dpy_null->mode.revision	 = KGI_MODE_REVISION;
	dpy_null->mode.images     = 1;
	dpy_null->mode.img[0].out = NULL;
	dpy_null->mode.img[0].flags = 0;
	dpy_null->mode.img[0].virt.x = size_x;
	dpy_null->mode.img[0].virt.y = size_y;
	dpy_null->mode.img[0].size.x = size_x;
	dpy_null->mode.img[0].size.y = size_y;
	dpy_null->mode.img[0].frames = 1;
	dpy_null->mode.img[0].fam = KGI_AM_COLOR_INDEX | KGI_AM_TEXT |
		KGI_AM_FOREGROUND_INDEX | KGI_AM_TEXTURE_INDEX;
	dpy_null->mode.img[0].bpfa[0] = 4;	/* bg color */
	dpy_null->mode.img[0].bpfa[1] = 4;	/* fg color */
	dpy_null->mode.img[0].bpfa[2] = 8;	/* texture  */

	return ((kgi_display_t *) dpy_null);
}

int 
dpy_null_init(int display, int max_display)
{
	/*
	 * We fill up with null-displays. This allows for true multihead
	 * with X.
	 */
	while (display < max_display) {
		kgi_display_t *dpy = dpy_null_alloc(80, 25);
		if (!dpy)
			return (display);
		if (kgi_register_display(dpy, display)) {
			dpy_index--;
			return (display);
		}
		display++;
	}
	return (display);
}

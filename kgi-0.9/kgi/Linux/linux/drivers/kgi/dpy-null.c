/* -----------------------------------------------------------------------------
**	NULL display code
** -----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**	Alternatively you may distribute this file under the terms and
**	conditions of the GNU General Public License. Please see the file 
**	COPYRIGHT.GPL included with this software for details of these terms
**	and conditions.
** -----------------------------------------------------------------------------
**
**	$Log: dpy-null.c,v $
*/

#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger

#include <linux/config.h>
#include <linux/version.h>
#include <asm/io.h>
#include <linux/major.h>
#include <linux/mm.h>
#include <linux/malloc.h>
#include <linux/string.h>
#include <linux/ascii.h>
#include <linux/console.h>
#include <linux/kdev_t.h>

#define	KGI_SYS_NEED_IO
#include <kgi/kgi.h>
#include <linux/kgii.h>

/* ----------------------------------------------------------------------------
**	kernel interface
** ----------------------------------------------------------------------------
*/

typedef struct
{
	kgi_dot_port_mode_t	dpm;

	kgi_accel_t		blubber_accel;

} dpy_null_dev_mode_t;

typedef struct
{
	kgi_display_t		dpy;
	kgi_mode_t		mode;
	kgi_ucoord_t		size;
	dpy_null_dev_mode_t	dev_mode;

} dpy_null_display_t;

/*	text16 operations
*/
static void dpy_null_show(kgic_mode_text16context_t *ctx, kgi_u_t x, 
	kgi_u_t y)
{
}

static void dpy_null_hide(kgic_mode_text16context_t *ctx)
{
}

#define	dpy_null_undo	dpy_null_hide

static void dpy_null_put_text16(kgic_mode_text16context_t *ctx, kgi_u_t offset,
	const kgi_u16_t *text, kgi_u_t count)
{
}

/*	text16 display operations
*/
static kgi_error_t dpy_null_mode_command(void *context, kgi_u_t cmd, 
	void *in_buffer, void **out_buffer, kgi_size_t *out_size)
{
	kgic_mode_context_t *ctx = (kgic_mode_context_t *) context;
	dpy_null_display_t *dpy_null = (dpy_null_display_t *) ctx->dpy;

	switch (cmd) {

	case KGIC_MODE_TEXT16CONTEXT:
		{
			kgic_mode_text16context_t *out = *out_buffer;
			kgic_mode_text16context_request_t *in = in_buffer;

			if (in->image) {

				KRN_DEBUG(1, "invalid image %i", in->image);
				return -EINVAL;
			}

			out->revision = KGIC_MODE_TEXT16CONTEXT_REVISION;
			out->meta_object = (kgi_display_t *) dpy_null;
			out->size.x = ctx->img->size.x;
			out->size.y = ctx->img->size.y;
			out->virt.x = ctx->img->virt.x;
			out->virt.y = ctx->img->virt.y;
			out->cell.x = 8;  /* Good enough for boot display */
			out->cell.y = 12; /* (gives approximate mouse feel.) */
			out->font.x = 8;
			out->font.y = 12;
			out->CursorShow = dpy_null_show;
			out->CursorUndo = dpy_null_undo;
			out->CursorHide = dpy_null_hide;
			out->PointerShow = dpy_null_show;
			out->PointerUndo = dpy_null_undo;
			out->PointerHide = dpy_null_hide;
			out->PutText16 = dpy_null_put_text16;
			return KGI_EOK;
		}
	default:
		KRN_DEBUG(1, "unknown/unsupported image command %.8x", cmd);
		return -EINVAL;
	}
}

static kgi_error_t dpy_null_display_command(void *ctx, unsigned int cmd, 
	void *in, void **out_buffer, unsigned long *out_size)
{
	*out_buffer = NULL;
	*out_size = 0;
	return -EINVAL;
}

static void dpy_null_inc_refcount(kgi_display_t *dpy)
{
	KRN_ASSERT(dpy);
	KRN_DEBUG(2, "dpy_null display refcount increment");
}

static void dpy_null_dec_refcount(kgi_display_t *dpy)
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
	dpy_null_dev_mode_t *devmode = (dpy_null_dev_mode_t *) dev_mode;

	if (images != 1) {

		KRN_DEBUG(2, "%i image layers are not supported.", images);
		return -EINVAL;
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
		if (! img[0].fam) {

			img[0].fam  = dpy_null->mode.img[0].fam;
			img[0].cam  = dpy_null->mode.img[0].cam;
			memmove(img[0].bpfa, dpy_null->mode.img[0].bpfa,
				sizeof(img[0].bpfa));
			memmove(img[0].bpca, dpy_null->mode.img[0].bpca,
				sizeof(img[0].bpca));
		}
		/*	fall through
		*/
	case KGI_TC_CHECK:
		if (! (MATCH(img[0].virt.x) && MATCH(img[0].virt.y) &&
			MATCH(img[0].size.x) && MATCH(img[0].size.y) &&
			MATCH(img[0].frames) && MATCH(img[0].tluts) &&
			MATCH(img[0].iluts) && MATCH(img[0].aluts))) {

			KRN_DEBUG(2, "image mode does not match boot mode");
			return -EINVAL;
		}
		if (!MATCH(img[0].fam) || !MATCH(img[0].cam) ||
			strncmp(img[0].bpfa, dpy_null->mode.img[0].bpfa,
				__KGI_MAX_NR_ATTRIBUTES) ||
			strncmp(img[0].bpca, dpy_null->mode.img[0].bpca,
				__KGI_MAX_NR_ATTRIBUTES)) {

			KRN_DEBUG(2, "attributes do not match boot mode");
			return -EINVAL;
		}

		devmode->dpm.dots.x = 8*img[0].size.x;
		devmode->dpm.dots.y = 12*img[0].size.y;
		devmode->dpm.dam    = img[0].fam;
		devmode->dpm.bpda   = img[0].bpfa;
		img[0].out = &devmode->dpm;
		img[0].flags = KGI_IF_TEXT16;

		return KGI_EOK;

	default:
		KRN_INTERNAL_ERROR;
		return -EINVAL;
	}
#undef	MATCH
#undef	DEFAULT
}

static void dpy_null_set_mode(kgi_display_t *dpy, kgi_image_mode_t *img,
	kgi_u_t images, void *dev_mode)
{
	KRN_ASSERT(dpy);
	KRN_ASSERT(img);
	KRN_ASSERT(images == 1);
	KRN_ASSERT(dev_mode);
}

static inline kgi_display_t *dpy_null_malloc(kgi_u_t size_x, kgi_u_t size_y)
{
	dpy_null_display_t *dpy_null = kmalloc(sizeof(*dpy_null), GFP_KERNEL);

	KRN_ASSERT(size_x && size_y);

	if (dpy_null == NULL) {

		return NULL;
	}
	memset(dpy_null, 0, sizeof(*dpy_null));

	/*	initialize display struct
	*/
	dpy_null->dpy.revision = KGI_DISPLAY_REVISION;
	sprintf(dpy_null->dpy.vendor, "KGI null display");
	sprintf(dpy_null->dpy.model, "%p", dpy_null);
	dpy_null->dpy.flags = 0;
	dpy_null->dpy.mode_size = sizeof(dpy_null_dev_mode_t);
	dpy_null->dpy.Command = dpy_null_display_command;

/*	dpy_null->dpy.priv.priv_u64 = 0; */
	dpy_null->dpy.mode = &dpy_null->mode;
	dpy_null->dpy.id = -1;
	dpy_null->dpy.graphic = 0;
	dpy_null->dpy.IncRefcount = dpy_null_inc_refcount;
	dpy_null->dpy.DecRefcount = dpy_null_dec_refcount;

	dpy_null->dpy.CheckMode	 = dpy_null_check_mode;
	dpy_null->dpy.SetMode	 = dpy_null_set_mode;
	dpy_null->dpy.ModeCommand = dpy_null_mode_command;

	dpy_null->dpy.focus = NULL;

	/*	initialize mode struct 
	*/
	dpy_null->mode.revision	 = KGI_MODE_REVISION;
	dpy_null->mode.dev_mode   = &dpy_null->dev_mode;
	dpy_null->mode.images     = 1;
	dpy_null->mode.img[0].out = NULL;
	dpy_null->mode.img[0].flags = 0;
	dpy_null->mode.img[0].virt.x = size_x;
	dpy_null->mode.img[0].virt.y = size_y;
	dpy_null->mode.img[0].size.x = size_x;
	dpy_null->mode.img[0].size.y = size_y;
	dpy_null->mode.img[0].frames = 1;
/*	dpy_null->mode.img[0].tluts = 0;
**	dpy_null->mode.img[0].aluts = 0;
**	dpy_null->mode.img[0].ilutm = 0;
**	dpy_null->mode.img[0].alutm = 0;
**	dpy_null->mode.img[0].cam = 0;
**	dpy_null->mode.img[0].bpca = { 0, ... };
*/	dpy_null->mode.img[0].fam = KGI_AM_COLOR_INDEX |
		KGI_AM_FOREGROUND_INDEX | KGI_AM_TEXTURE_INDEX;
	dpy_null->mode.img[0].bpfa[0] = 4;	/* bg color */
	dpy_null->mode.img[0].bpfa[1] = 4;	/* fg color */
	dpy_null->mode.img[0].bpfa[2] = 8;	/* texture  */

	return (kgi_display_t *) dpy_null;
}

static void dpy_null_free(kgi_display_t *dpy)
{
	kfree(dpy);
}

int dpy_null_init(int display, int max_display)
{
	/*	We fill up with null-displays. This allows for true multihead
	**	with XFree86.
	*/
	while (display < max_display) {

		kgi_display_t *dpy = dpy_null_malloc(80, 25);
		if (!dpy || kgi_register_display(dpy, display)) {

			dpy_null_free(dpy);
			return display;
		}
		display++;
	}
	return display;
}

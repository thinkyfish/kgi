/*-
 * Copyright (c) 2003 Nicholas Souchu
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
 * KGC (BSD) General FB renderer
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <dev/kgi/maintainers.h>
#define	MAINTAINER	Nicholas_Souchu

#include "opt_kgi.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/linker.h>
#include <sys/linker.h>

#ifndef KGI_DBG_LEVEL
#define	KGI_DBG_LEVEL	2
#endif

#define KGI_SYS_NEED_MALLOC
#include <dev/kgi/system.h>

#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>
#include <dev/kii/kii.h>

#include <dev/kgc/kgc_console.h>
#include <dev/kgc/kgc_textbuf.h>
#include <dev/kgc/kgc_render.h>
#include <dev/kgc/kgc_kgirndr.h>
#include <dev/kgc/kgc_gfbrndr.h>
#include <dev/kgc/kgc_backgnd.h>

#include "render_if.h"

/* For now don't use background in high or true colour modes. */
#ifdef KGC_RENDER_16BITS
#undef KGC_RENDER_BACKGROUND
#endif

typedef struct {
	/*
	 * Must be there if you use KGI resources interface
	 */
	__KGC_KGIRNDR_DATA;

	kgi_u_t		width;
	kgi_u_t		height;
	kgi_u_t		depth;

	/* XXX Maximum of 2 pixel lines of 10 pixels with bpp=16 */
	kgi_u8_t	pointed[10 * 2 * 2];
	kgi_ucoord_t	cursor;

	kgc_gfb_font_t	*font;
	unsigned char	*bgnd;

	kgi_u16_t	palette[256 * 3];
} gfbrndr_meta;

static int
gfbrndr_putp(kgi_virt_addr_t fb, kgi_u32_t off, kgi_u32_t p, kgi_u32_t a,
	    int size, int bpp, int bit_ltor, int byte_ltor, unsigned char *bgnd)
{
	int i, j, k, num_shifts;
	kgi_u32_t _p, val, mask[32];

	if (bpp < 1)
		return (-1);

	  /*
	   * If we don't display bits right-to-left (little-bitian?),
	   * then perform a bit-swap on p...
	   */
	if (bit_ltor) {
		num_shifts = 8 * size;
		for (i = 0, _p = 0; i < num_shifts; i++, p >>= 1) {
			_p <<= 1;
			_p |= (p & 0x00000001);
		}
	} else
		_p = p;

	val = 0;

	switch (bpp) {
#if 0
	/* Accelerate the simplest cases... */
	case 1:
		if((a & 0x00000001) == 0)
			val[0] = 0;
		else if(size <= 0)
			val[0] = 0;
		else if(size == 1)
			val[0] = _p & 0x000000ff;
		else if(size == 2)
			val[0] = _p & 0x0000ffff;
		else if(size == 3)
			val[0] = _p & 0x00ffffff;
		else if(size == 4)
			val[0] = _p & 0xffffffff;
		break;
#endif
	case EIGHTBIT_COLOUR:
		if (size > 0) {
			a &= 0x000000ff;
			val = a | (a << 8) | (a << 16) | (a << 24);
			mask[0] = 0;
			if (_p & 0x00000001) mask[0] |= (0xff);
			if (_p & 0x00000002) mask[0] |= (0xff << 8);
			if (_p & 0x00000004) mask[0] |= (0xff << 16);
			if (_p & 0x00000008) mask[0] |= (0xff << 24);
			mask[1] = 0;
			if (_p & 0x00000010) mask[1] |= (0xff);
			if (_p & 0x00000020) mask[1] |= (0xff << 8);
			if (_p & 0x00000040) mask[1] |= (0xff << 16);
			if (_p & 0x00000080) mask[1] |= (0xff << 24);
		}
		if (size > 1) {
			mask[2] = 0;
			if (_p & 0x00000100) mask[2] |= (0xff);
			if (_p & 0x00000200) mask[2] |= (0xff << 8);
			if (_p & 0x00000400) mask[2] |= (0xff << 16);
			if (_p & 0x00000800) mask[2] |= (0xff << 24);
			mask[3] = 0;
			if (_p & 0x00001000) mask[3] |= (0xff);
			if (_p & 0x00002000) mask[3] |= (0xff << 8);
			if (_p & 0x00004000) mask[3] |= (0xff << 16);
			if (_p & 0x00008000) mask[3] |= (0xff << 24);
		}
		if (size > 2) {
			mask[4] = 0;
			if (_p & 0x00010000) mask[4] |= (0xff);
			if (_p & 0x00020000) mask[4] |= (0xff << 8);
			if (_p & 0x00040000) mask[4] |= (0xff << 16);
			if (_p & 0x00080000) mask[4] |= (0xff << 24);
			mask[5] = 0;
			if (_p & 0x00100000) mask[5] |= (0xff);
			if (_p & 0x00200000) mask[5] |= (0xff << 8);
			if (_p & 0x00400000) mask[5] |= (0xff << 16);
			if (_p & 0x00800000) mask[5] |= (0xff << 24);
		}
		if (size > 3) {
			mask[6] = 0;
			if (_p & 0x01000000) mask[6] |= (0xff);
			if (_p & 0x02000000) mask[6] |= (0xff << 8);
			if (_p & 0x04000000) mask[6] |= (0xff << 16);
			if (_p & 0x08000000) mask[6] |= (0xff << 24);
			mask[7] = 0;
			if (_p & 0x10000000) mask[7] |= (0xff);
			if (_p & 0x20000000) mask[7] |= (0xff << 8);
			if (_p & 0x40000000) mask[7] |= (0xff << 16);
			if (_p & 0x80000000) mask[7] |= (0xff << 24);
		}
		break;
	case HIGHCOLOUR_16BIT:
		if (size > 0) {
			a &= 0x0000ffff;
			val = 0xffffffff;
			mask[0] = 0;
			if (_p & 0x00000001) mask[0] |= (0xffff);
			if (_p & 0x00000002) mask[0] |= (0xffff << 16);
			mask[1] = 0;
			if (_p & 0x00000004) mask[1] |= (0xffff);
			if (_p & 0x00000008) mask[1] |= (0xffff << 16);
			mask[2] = 0;
			if (_p & 0x00000010) mask[2] |= (0xffff);
			if (_p & 0x00000020) mask[2] |= (0xffff << 16);
			mask[3] = 0;
			if (_p & 0x00000040) mask[3] |= (0xffff);
			if (_p & 0x00000080) mask[3] |= (0xffff << 16);
		}
		if (size > 1) {
			mask[4] = 0;
			if (_p & 0x00000100) mask[4] |= (0xffff);
			if (_p & 0x00000200) mask[4] |= (0xffff << 16);
			mask[5] = 0;
			if (_p & 0x00000400) mask[5] |= (0xffff);
			if (_p & 0x00000800) mask[5] |= (0xffff << 16);
			mask[6] = 0;
			if (_p & 0x00001000) mask[6] |= (0xffff);
			if (_p & 0x00002000) mask[6] |= (0xffff << 16);
			mask[7] = 0;
			if (_p & 0x00004000) mask[7] |= (0xffff);
			if (_p & 0x00008000) mask[7] |= (0xffff << 16);
		}
		if (size > 2) {
			mask[8] = 0;
			if (_p & 0x00010000) mask[8] |= (0xffff);
			if (_p & 0x00020000) mask[8] |= (0xffff << 16);
			mask[9] = 0;
			if (_p & 0x00040000) mask[9] |= (0xffff);
			if (_p & 0x00080000) mask[9] |= (0xffff << 16);
			mask[10] = 0;
			if (_p & 0x00100000) mask[10] |= (0xffff);
			if (_p & 0x00200000) mask[10] |= (0xffff << 16);
			mask[11] = 0;
			if (_p & 0x00400000) mask[11] |= (0xffff);
			if (_p & 0x00800000) mask[11] |= (0xffff << 16);
		}
		if (size > 3) {
			mask[12] = 0;
			if (_p & 0x01000000) mask[12] |= (0xffff);
			if (_p & 0x02000000) mask[12] |= (0xffff << 16);
			mask[13] = 0;
			if (_p & 0x04000000) mask[13] |= (0xffff);
			if (_p & 0x08000000) mask[13] |= (0xffff << 16);
			mask[14] = 0;
			if (_p & 0x10000000) mask[14] |= (0xffff);
			if (_p & 0x20000000) mask[14] |= (0xffff << 16);
			mask[15] = 0;
			if (_p & 0x40000000) mask[15] |= (0xffff);
			if (_p & 0x80000000) mask[15] |= (0xffff << 16);
		}
		break;
	default:
		break;
	}
	j = (bpp == 1) ? 1 : bpp * size / sizeof(kgi_u32_t);

	/*
	 * If we don't display bytes right-to-left (little-endian),
	 * then perform a byte-swap on p (we don't have to swap if
	 * bpp == 1 and val[0] == 0)...
	 */
	if ((byte_ltor) && (j > 1) && (mask[j] != 0)) {
		for (i = 0; i < (j - i); i++) {
			_p = mask[j - i];
			mask[j - i] = mask[i];
			mask[i] = _p;
		}
		for (i = 0; i < j; i++) {
			_p = mask[i];
			for (k = 0, mask[i] = 0; k < sizeof(kgi_u32_t); 
				k++, _p >>= 8) {
				mask[i] <<= 8;
				mask[i] |= (_p & 0xff);
			}
		}
	}

	for (i = 0; i < j; i++) {
		/* Write the pixel-row... */
		*((kgi_u32_t *)(fb) + off + i) = (val & mask[i]) | ((bgnd) ?
			(*((kgi_u32_t *)(bgnd) + off + i) & ~mask[i]) : 0);
	}
	return (0);
}

#define PIX2POS(pixel) (pixel & 0xFF)
#define PIX2ATTR(pixel) ((pixel & 0xFF00) >> 8)

static int
gfbrndr_putcs(render_t r, kgc_textbuf_t *tb, kgi_u_t start,
		kgi_u_t offset, kgi_u_t count)
{
	gfbrndr_meta *render;
	kgi_u32_t poff;
	kgi_u_t row, col, n;
	kgi_u8_t *pixel;
	kgi_u16_t *text;
	int i;

	render = kgc_render_meta(r);
	text = tb->buf + start;

	/* Iterate on the string */
	for (n = 0; n < count; n++) {
		/*
		 * Get the start of the array of pixels rows for this
		 * character...
		 */
		pixel =
		&render->font->data[PIX2POS(text[n]) * render->font->height];

		/* Calculate the new cursor position... */
		row = (offset + n) / render->width;
		col = (offset + n) % render->width;

		/* Iterate over all the pixel rows for this character... */
		for (i = 0; i < render->font->height; i++) {
			/* Get the address of the character's pixel-row... */
			poff = (((((row * render->font->height) + i) *
				  render->mode.img[0].size.x) +
				  (col * render->font->width)) *
				  (render->depth / 8) / sizeof(kgi_u32_t));

			/* Now display the current pixel row... */
			gfbrndr_putp(render->fb->win.virt, poff, pixel[i],
					PIX2ATTR(text[n]), sizeof(kgi_u8_t),
					render->depth, 1, 0, render->bgnd);
		}
	}
	return (0);
}

static void 
gfbrndr_get(render_t r, kgi_ucoord_t *size, kgi_ucoord_t *virt,
	    kgi_u_t *flags)
{
	gfbrndr_meta *render;

	render = kgc_render_meta(r);

	/* No hardscroll, virt == real */
	if (size && virt) {
		size->x = render->width;
		size->y = render->height;
		virt->x = render->width;
		virt->y = render->height;
	}

	if (flags) {
		*flags = render->flags;

		/* Hard code no hardscroll */
		*flags |= KGI_RF_NO_HARDSCROLL;

		*flags |= (render->mode.img[0].flags & KGI_IF_TILE_Y) ?
			KGI_RF_TILE_Y : 0;
	}
}

static void
gfbrndr_set(render_t r, kgi_ucoord_t *size, kgi_ucoord_t *virt)
{
	gfbrndr_meta *render;

	render = kgc_render_meta(r);
	render->width = virt->x;
	render->height = virt->y;
}

static kgi_u_t
gfbrndr_ctop(render_t r, kgi_isochar_t sym)
{

	/* Use direct mapping between the char and the glyph */
	return ((kgi_u_t)sym);
}

#ifdef KGC_RENDER_BACKGROUND
static void
gfbrndr_bgnd_draw(render_t r)
{
	gfbrndr_meta *render;

	render = kgc_render_meta(r);

	/* If focused draw/redraw the background */
	if (render->kgi.flags & KGI_DF_FOCUSED) {
		/*
		 * Only update the screen if focused. Avoid palette
		 * setting but get it in the render.
		 */
		backgnd_draw(r->devid, render->bgnd, render->palette);

		/*
		 * XXX
		 * Force 7 as light gray, consequently your
		 * image shall not contain any 7 or it will be
		 * light gray!!
		 */
		render->palette[7 * 3] = render->palette[7 * 3 + 1] =
			render->palette[7 * 3 + 2] = 66 * 0xFF / 100 << 8;

		/* Apply the modified palette */
		if (render->ilut->Set)
			render->ilut->Set(render->ilut, 0, 0, 256,
					  KGI_AM_COLORS, render->palette);

	}
}

static int
gfbrndr_callback(render_t r, int action, void *arg)
{
	gfbrndr_meta *render;
	kgi_u_t size;

	render = kgc_render_meta(r);
	size = 0;

	switch (action) {
	case BACKGND_INIT:
		/* If no background is allocated for this render, do it. */
		if (render->bgnd == NULL) {
			size =
			render->mode.img[0].size.x * render->mode.img[0].size.y
			* ((kgi_attr_bits(render->mode.img[0].bpfa) + 1) / 8);

			if ((render->bgnd = kgi_kmalloc(size)) == NULL)
				return (ENOMEM);

			bzero(render->bgnd, size);
		}

		/* Draw in the allocated background buffer */
		gfbrndr_bgnd_draw(r);

		/*
		 * XXX FIXME
		 * memcpy(render->fb->win.virt, render->bgnd, size);
		 */
		break;
	case BACKGND_TERM:
		/* XXX Blank the screen? */

		/* Release the background buffer */
		if (render->bgnd) {
			kgi_kfree(render->bgnd);
			render->bgnd = NULL;
		}
		break;
	default:
		break;
	}

	return (0);
}
#endif

static void
gfbrndr_unmap(render_t r)
{
	kgirndr_meta *render;

	render = kgc_render_meta(r);
	render->flags &= ~KGI_RF_NEEDS_UPDATE;
}

static void
gfbrndr_map(render_t r)
{
	gfbrndr_meta *render;

	render = kgc_render_meta(r);
	/* Draw/redraw the background */
#ifdef KGC_RENDER_BACKGROUND
	gfbrndr_bgnd_draw(r);
#endif

	/* Tell the scroller to update the screen */
	render->flags |= KGI_RF_NEEDS_UPDATE;
}

static void
gfbrndr_hide_gadgets(render_t r)
{
	gfbrndr_meta *render;
	kgi_u32_t poff;
	kgi_u8_t *addr, *saved;
	int i, j, k;

	render = kgc_render_meta(r);

	if (!(r->cons->flags & KGI_CF_CURSOR_SHOWN) ||
	    !(render->kgi.flags & KGI_DF_FOCUSED))
		return;

	/* Restore the 2 lines overwritten by the cursor */
	saved = &render->pointed[0];
	for (i = render->font->height-2; i < render->font->height; i++) {
		/* Get the address of the character's pixel-row... */
		poff = (((((render->cursor.y * render->font->height) + i) *
			render->mode.img[0].size.x) +
			(render->cursor.x * render->font->width)) *
			(render->depth / 8));

		/* Iterate over the columns of this row */
		for (j = 0; j < render->font->width; j += render->depth / 8) {
			addr = (kgi_u8_t *)(render->fb->win.virt) + poff + j;
			for (k = 0; k < render->depth / 8; k++)
				addr[k] = *saved++;
		}
	}

	r->cons->flags &= ~KGI_CF_CURSOR_SHOWN;
}

static void
gfbrndr_show_gadgets(render_t r, kgi_u_t x, kgi_u_t y, kgi_u_t offset)
{
	gfbrndr_meta *render;
	kgi_u32_t poff;
	kgi_u8_t *addr, *save;
	int i, j, k;

	render = kgc_render_meta(r);

	if (!CONSOLE_MODE(r->cons, KGI_CM_SHOW_CURSOR) ||
	    (r->cons->flags & KGI_CF_CURSOR_SHOWN) ||
	    !(render->kgi.flags & KGI_DF_FOCUSED))
		return;

	gfbrndr_hide_gadgets(r);

	render->cursor.x = x;
	render->cursor.y = y;

	/* Overwrite 2 lines to print the cursor */
	save = &render->pointed[0];
	for(i = render->font->height - 2; i < render->font->height; i++) {
		/* Get the address of the character's pixel-row... */
		poff = (((((render->cursor.y * render->font->height) + i) *
			render->mode.img[0].size.x) +
			(render->cursor.x * render->font->width)) *
			(render->depth / 8));

		/* Iterate over the columns of this row */
		for (j = 0; j < render->font->width; j+=render->depth / 8) {
			addr = (kgi_u8_t *)(render->fb->win.virt) + poff + j;
			for (k = 0; k < render->depth / 8; k++) {
				*save++ = addr[k];
				addr[k] = 7;
			}
		}
	}

	r->cons->flags |= KGI_CF_CURSOR_SHOWN;
}

/*
 * Some common screen display resolutions.
 */
const kgi_u16_t modes[][2] =
{

#ifdef KGC_RENDER_1280x1024
	{ 1280, 1024 },
#endif
#ifdef KGC_RENDER_1024x768
	{ 1024, 768 },
#endif
#ifdef KGC_RENDER_800x600
	{ 800, 600 },
#endif
#ifdef KGC_RENDER_640x480
	{ 640, 480 },
#endif
#ifdef KGC_RENDER_640x400
	{ 640, 400 },
#endif
#ifdef KGC_RENDER_320x240
	{ 320, 240 },
#endif
#ifdef KGC_RENDER_320x200
	{ 320, 200 },
#endif
	{ 0, 0 }
};

static void
gfbrndr_handle_event(kgi_device_t *device, kgi_event_t *event)
{
	kgi_console_t *cons;
	render_t r;

	cons = (kgi_console_t *)device->priv.priv_ptr;
	r = cons->render;

	switch (event->notice.command) {
	case KGI_EVENT_NOTICE_NEW_DISPLAY:
		/*
		 * Redo common initialization, especially concerning KGI
		 * resources
		 */
		kgirndr_init(r);
		break;
	default:
		break;
	}

	return;
}

static kgi_s_t
gfbrndr_init(render_t r, kgi_u_t devid)
{
	gfbrndr_meta *render;
	kgi_s_t i, error = KGI_ENODEV;

	render = kgc_render_meta(r);

	bzero(render, sizeof(*render));

	render->mode.images = 1;
	/* render->mode.flags |= KGI_MF_BOOT; XXX avoids mode alloc */
#ifdef KGC_RENDER_16BITS
	render->mode.img[0].fam = KGI_AM_RGB;
	render->mode.img[0].bpfa[0] = 5;
	render->mode.img[0].bpfa[1] = 6;
	render->mode.img[0].bpfa[2] = 5;
	render->mode.img[0].bpfa[3] = 0;
#else
	render->mode.img[0].fam = KGI_AM_COLOR_INDEX;
	render->mode.img[0].bpfa[0] = 8;
	render->mode.img[0].bpfa[1] = 0;
#endif

	render->kgi.mode = &(render->mode);
	render->kgi.flags |= KGI_DF_CONSOLE;
	render->kgi.priv.priv_ptr = kgc_render_cons(r);

	render->kgi.MapDevice = kgc_map_kgi;
	render->kgi.UnmapDevice = kgc_unmap_kgi;
	render->kgi.HandleEvent = gfbrndr_handle_event;

	for (i = 0; modes[i][0] && modes[i][1]; i++) {
		render->mode.img[0].size.x =
			render->mode.img[0].virt.x = modes[i][0];
		render->mode.img[0].size.y =
			render->mode.img[0].virt.y = modes[i][1];

		error = kgi_register_device(&(render->kgi), r->devid);
		if (error == KGI_EOK)
			goto found;
	}
	return (error);

 found:
	/* Common initialization, especially concerning KGI resources */
	kgirndr_init(r);

#ifndef KGC_DEFAULT_FONT
#define KGC_DEFAULT_FONT kgc_bold8x16
#endif

	render->font = &(KGC_DEFAULT_FONT);
	render->width = render->mode.img[0].size.x / render->font->width;
	render->height = render->mode.img[0].size.y / render->font->height;
#ifdef KGC_RENDER_16BITS
	render->depth = HIGHCOLOUR_16BIT;
#else
	render->depth = EIGHTBIT_COLOUR;
#endif

	if (kgi_current_focus(render->kgi.dpy_id) == NULL)
		kgi_map_device(render->kgi.id);

	/* Initialize the background */
#ifdef KGC_RENDER_BACKGROUND
	backgnd_init(r->devid, gfbrndr_callback, NULL);
#endif

	return (KGI_EOK);
}

static void
gfbrndr_done(render_t r)
{
	gfbrndr_meta *render;

	render = kgc_render_meta(r);
	/* Terminate the background management */
#ifdef KGC_RENDER_BACKGROUND
	backgnd_term(r->devid);
#endif

	/* Release KGI resources */
	if (render->kgi.flags & KGI_DF_FOCUSED)
		kgi_unmap_device(render->kgi.id);

	kgi_unregister_device(&render->kgi);
}

static render_method_t gfbrndr_methods[] = {
	/* Specific methods */
	RENDERMETHOD(render_init,		gfbrndr_init),
	RENDERMETHOD(render_done,		gfbrndr_done),
	RENDERMETHOD(render_put_text,		gfbrndr_putcs),
	RENDERMETHOD(render_ctop,		gfbrndr_ctop),
	RENDERMETHOD(render_get,		gfbrndr_get),
	RENDERMETHOD(render_set,		gfbrndr_set),
	RENDERMETHOD(render_map,		gfbrndr_map),
	RENDERMETHOD(render_unmap,		gfbrndr_unmap),
	RENDERMETHOD(render_show_gadgets,	gfbrndr_show_gadgets),
	RENDERMETHOD(render_hide_gadgets,	gfbrndr_hide_gadgets),
	RENDERMETHOD(render_undo_gadgets,	gfbrndr_hide_gadgets),
	/* Methods using KGI generic interface */
	RENDERMETHOD(render_ptoa,		kgirndr_ptoa_color),
	RENDERMETHOD(render_atop,		kgirndr_atop_color),
	RENDERMETHOD_END
};

render_driver_t gfbrndr_driver = {
	"gfbrndr",
	gfbrndr_methods,
	sizeof(gfbrndr_meta)
};

DECLARE_RENDER(gfbrndr, gfbrndr_driver, gfbrndr_meta);
